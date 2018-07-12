/*
 *  WPA PSK handling routines
 *
 *  $Id: 8192cd_psk.c,v 1.6 2010/03/10 03:24:40 keith_huang Exp $
 *
 *  Copyright (c) 2009 Realtek Semiconductor Corp.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 */

#define _8192CD_PSK_C_

#ifdef __KERNEL__
#include <linux/module.h>
#include <linux/netdevice.h>
#include <linux/timer.h>
#include <linux/random.h>
#endif

#include "./8192cd_cfg.h"

#if defined(WIFI_HAPD) && !defined(HAPD_DRV_PSK_WPS) || defined(RTK_NL80211)

#include "./8192cd.h"
#include "./wifi.h"
#include "./8192cd_util.h"
#include "./8192cd_headers.h"
#include "./8192cd_security.h"
#include "./ieee802_mib.h"
#include "./8192cd_debug.h"
#include "./8192cd_psk.h"
#include "./1x_rc4.h"

#ifndef __KERNEL__
#include "./sys-support.h"
#endif

//#define DEBUG_PSK

#define ETHER_ADDRLEN					6
#define PMK_EXPANSION_CONST 	        "Pairwise key expansion"
#define PMK_EXPANSION_CONST_SIZE		22
#ifdef RTL_WPA2
#define PMKID_NAME_CONST 	        	"PMK Name"
#define PMKID_NAME_CONST_SIZE			8
#endif /* RTL_WPA2 */
#define GMK_EXPANSION_CONST				"Group key expansion"
#define GMK_EXPANSION_CONST_SIZE		19
#define RANDOM_EXPANSION_CONST			"Init Counter"
#define RANDOM_EXPANSION_CONST_SIZE	12
#define PTK_LEN_CCMP            		48

/*
	2008-12-16, For Corega CG-WLCB54GL 54Mbps NIC interoperability issue.
	The behavior of this NIC when it connect to the other AP with WPA/TKIP is:
		AP	<----------------------> 	STA
			....................
			------------> Assoc Rsp (ok)
			------------> EAPOL-key (4-way msg 1)
			<------------ unknown TKIP encryption data
			------------> EAPOL-key (4-way msg 1)
			<------------ unknown TKIP encryption data
			.....................
			<------------ disassoc (code=8, STA is leaving) when the 5 seconds timer timeout counting from Assoc_Rsp is got.
			....................
			------------> Assoc Rsp (ok)
			<-----------> EAPOL-key (4-way handshake success)

	If MAX_RESEND_NUM=3, our AP will send disassoc (code=15, 4-way timeout) to STA before STA sending disassoc to AP.
	And this NIC will always can not connect to our AP.
	set MAX_RESEND_NUM=5 can fix this issue.
 */
//#define MAX_RESEND_NUM					3
#define MAX_RESEND_NUM					5

#define RESEND_TIME						RTL_SECONDS_TO_JIFFIES(1)// in 10ms

#define LargeIntegerOverflow(x) (x.field.HighPart == 0xffffffff) && \
								(x.field.LowPart == 0xffffffff)
#define LargeIntegerZero(x) memset(&x.charData, 0, 8);

#define Octet16IntegerOverflow(x) LargeIntegerOverflow(x.field.HighPart) && \
								  LargeIntegerOverflow(x.field.LowPart)
#define Octet16IntegerZero(x) memset(&x.charData, 0, 16);

extern void hmac_sha(
	unsigned char*	k,     /* secret key */
	int				lk,    /* length of the key in bytes */
	unsigned char*	d,     /* data */
	int				ld,    /* length of data in bytes */
	unsigned char*	out,   /* output buffer, at least "t" bytes */
	int				t
	);

extern void hmac_sha1(unsigned char *text, int text_len, unsigned char *key,
		 int key_len, unsigned char *digest);

extern void hmac_md5(unsigned char *text, int text_len, unsigned char *key,
		 int key_len, void * digest);

#ifdef RTL_WPA2
extern void AES_WRAP(unsigned char *plain, int plain_len,
		unsigned char *iv,	int iv_len,
		unsigned char *kek,	int kek_len,
		unsigned char *cipher, unsigned short *cipher_len);
#endif


#ifdef DEBUG_PSK
static char *ID2STR(int id)
{
	switch(id) {
		case DOT11_EVENT_ASSOCIATION_IND:
			return("DOT11_EVENT_ASSOCIATION_IND");
		case DOT11_EVENT_REASSOCIATION_IND:
			return ("DOT11_EVENT_REASSOCIATION_IND");

		case DOT11_EVENT_DISASSOCIATION_IND:
			return ("DOT11_EVENT_DISASSOCIATION_IND");

		case DOT11_EVENT_EAP_PACKET:
			return ("DOT11_EVENT_EAP_PACKET");

		case DOT11_EVENT_MIC_FAILURE:
			return ("DOT11_EVENT_MIC_FAILURE");
		default:
			return ("Not support event");

	}
}

#endif // DEBUG_PSK

static OCTET_STRING SubStr(OCTET_STRING f, unsigned short s, unsigned short l)
{
	OCTET_STRING res;

	res.Length = l;
	res.Octet = f.Octet+s;
	return res;
}

static void i_P_SHA1(
	unsigned char*  key,                // pointer to authentication key
	int             key_len,            // length of authentication key
	unsigned char*  text,               // pointer to data stream
	int             text_len,           // length of data stream
	unsigned char*  digest,             // caller digest to be filled in
	int				digest_len			// in byte
	)
{
	int i;
	int offset=0;
	int step=20;
	int IterationNum=(digest_len+step-1)/step;

	for(i=0;i<IterationNum;i++)
	{
		text[text_len]=(unsigned char)i;
		hmac_sha(key,key_len,text,text_len+1,digest+offset,step);
		offset+=step;
	}
}

static void i_PRF(
	unsigned char*	secret,
	int				secret_len,
	unsigned char*	prefix,
	int				prefix_len,
	unsigned char*	random,
	int				random_len,
	unsigned char*  digest,             // caller digest to be filled in
	int				digest_len			// in byte
	)
{
	unsigned char data[1000];
	memcpy(data,prefix,prefix_len);
	data[prefix_len++]=0;
	memcpy(data+prefix_len,random,random_len);
	i_P_SHA1(secret,secret_len,data,prefix_len+random_len,digest,digest_len);
}


/*
 * F(P, S, c, i) = U1 xor U2 xor ... Uc
 * U1 = PRF(P, S || Int(i))
 * U2 = PRF(P, U1)
 * Uc = PRF(P, Uc-1)
 */

static void F(
	char *password,
	int passwordlength,
	unsigned char *ssid,
	int ssidlength,
	int iterations,
	int count,
	unsigned char *output)
{
	unsigned char digest[36], digest1[A_SHA_DIGEST_LEN];
	int i, j;

	/* U1 = PRF(P, S || int(i)) */
	memcpy(digest, ssid, ssidlength);
	digest[ssidlength] = (unsigned char)((count>>24) & 0xff);
	digest[ssidlength+1] = (unsigned char)((count>>16) & 0xff);
	digest[ssidlength+2] = (unsigned char)((count>>8) & 0xff);
	digest[ssidlength+3] = (unsigned char)(count & 0xff);
	hmac_sha1(digest, ssidlength + 4,
		(unsigned char*) password, (int)strlen(password),
           	digest1);

	/*
	hmac_sha1((unsigned char*) password, passwordlength,
           digest, ssidlength+4, digest1);
	*/

	/* output = U1 */
	memcpy(output, digest1, A_SHA_DIGEST_LEN);

	for (i = 1; i < iterations; i++) {
		/* Un = PRF(P, Un-1) */
		hmac_sha1(digest1, A_SHA_DIGEST_LEN, (unsigned char*) password,
				(int)strlen(password), digest);
		//hmac_sha1((unsigned char*) password, passwordlength,digest1, A_SHA_DIGEST_LEN, digest);
		memcpy(digest1, digest, A_SHA_DIGEST_LEN);

		/* output = output xor Un */
		for (j = 0; j < A_SHA_DIGEST_LEN; j++) {
			output[j] ^= digest[j];
		}
	}
}

/*
 * password - ascii string up to 63 characters in length
 * ssid - octet string up to 32 octets
 * ssidlength - length of ssid in octets
 * output must be 40 octets in length and outputs 256 bits of key
 */
static int PasswordHash (
	char *password,
	unsigned char *ssid,
	short ssidlength,
	unsigned char *output)
{
	int passwordlength = strlen(password);
//	int ssidlength = strlen(ssid);

	if ((passwordlength > 63) || (ssidlength > 32))
		return 0;

	F(password, passwordlength, ssid, ssidlength, 4096, 1, output);
	F(password, passwordlength, ssid, ssidlength, 4096, 2, &output[A_SHA_DIGEST_LEN]);
	return 1;
}

void ConstructIE(struct rtl8192cd_priv *priv, unsigned char *pucOut, int *usOutLen)
{
	DOT11_RSN_IE_HEADER dot11RSNIEHeader = { 0 };
	DOT11_RSN_IE_SUITE dot11RSNGroupSuite;
	DOT11_RSN_IE_COUNT_SUITE *pDot11RSNPairwiseSuite = NULL;
	DOT11_RSN_IE_COUNT_SUITE *pDot11RSNAuthSuite = NULL;
	unsigned short usSuitCount;
	unsigned long ulIELength = 0;
	unsigned long ulIndex = 0;
	unsigned long ulPairwiseLength = 0;
	unsigned long ulAuthLength = 0;
	unsigned char *pucBlob;
	DOT11_RSN_IE_COUNT_SUITE countSuite, authCountSuite;
#ifdef RTL_WPA2
	DOT11_RSN_CAPABILITY dot11RSNCapability = { 0 };
	unsigned long uCipherAlgo = 0;
	int bCipherAlgoEnabled = FALSE;
	unsigned long uAuthAlgo = 0;
	int bAuthAlgoEnabled = FALSE;
	unsigned long ulRSNCapabilityLength = 0;
#endif

	*usOutLen = 0;
	if ( priv->pmib->dot1180211AuthEntry.dot11EnablePSK & PSK_WPA) {
		//
		// Construct Information Header
		//
		dot11RSNIEHeader.ElementID = RSN_ELEMENT_ID;
		dot11RSNIEHeader.OUI[0] = 0x00;
		dot11RSNIEHeader.OUI[1] = 0x50;
		dot11RSNIEHeader.OUI[2] = 0xf2;
		dot11RSNIEHeader.OUI[3] = 0x01;
		dot11RSNIEHeader.Version = cpu_to_le16(RSN_VER1);
		ulIELength += sizeof(DOT11_RSN_IE_HEADER);

		// Construct Cipher Suite:
		// - Multicast Suite:
		memset(&dot11RSNGroupSuite, 0, sizeof dot11RSNGroupSuite);
		dot11RSNGroupSuite.OUI[0] = 0x00;
		dot11RSNGroupSuite.OUI[1] = 0x50;
		dot11RSNGroupSuite.OUI[2] = 0xF2;
		dot11RSNGroupSuite.Type = priv->wpa_global_info->MulticastCipher;
		ulIELength += sizeof(DOT11_RSN_IE_SUITE);

    	// - UnicastSuite
        pDot11RSNPairwiseSuite = &countSuite;
        memset(pDot11RSNPairwiseSuite, 0, sizeof(DOT11_RSN_IE_COUNT_SUITE));
		usSuitCount = 0;

#ifdef RTK_NL80211
        for (ulIndex=0; ulIndex<priv->wpa_global_info->NumOfUnicastCipher; ulIndex++)
        {
        	int i = ulIndex<priv->wpa_global_info->NumOfUnicastCipher - ulIndex - 1;
			pDot11RSNPairwiseSuite->dot11RSNIESuite[usSuitCount].OUI[0] = 0x00;
			pDot11RSNPairwiseSuite->dot11RSNIESuite[usSuitCount].OUI[1] = 0x50;
			pDot11RSNPairwiseSuite->dot11RSNIESuite[usSuitCount].OUI[2] = 0xF2;
			pDot11RSNPairwiseSuite->dot11RSNIESuite[usSuitCount].Type = priv->wpa_global_info->UnicastCipher[i];
			usSuitCount++;
        }
#else
        for (ulIndex=0; ulIndex<priv->wpa_global_info->NumOfUnicastCipher; ulIndex++)
        {
			pDot11RSNPairwiseSuite->dot11RSNIESuite[usSuitCount].OUI[0] = 0x00;
			pDot11RSNPairwiseSuite->dot11RSNIESuite[usSuitCount].OUI[1] = 0x50;
			pDot11RSNPairwiseSuite->dot11RSNIESuite[usSuitCount].OUI[2] = 0xF2;
			pDot11RSNPairwiseSuite->dot11RSNIESuite[usSuitCount].Type = priv->wpa_global_info->UnicastCipher[ulIndex];
			usSuitCount++;
        }
#endif
		pDot11RSNPairwiseSuite->SuiteCount = cpu_to_le16(usSuitCount);
        ulPairwiseLength = sizeof(pDot11RSNPairwiseSuite->SuiteCount) + usSuitCount*sizeof(DOT11_RSN_IE_SUITE);
        ulIELength += ulPairwiseLength;

		//
		// Construction of Auth Algo List
		//
        pDot11RSNAuthSuite = &authCountSuite;
        memset(pDot11RSNAuthSuite, 0, sizeof(DOT11_RSN_IE_COUNT_SUITE));
		usSuitCount = 0;
		pDot11RSNAuthSuite->dot11RSNIESuite[usSuitCount].OUI[0] = 0x00;
		pDot11RSNAuthSuite->dot11RSNIESuite[usSuitCount].OUI[1] = 0x50;
		pDot11RSNAuthSuite->dot11RSNIESuite[usSuitCount].OUI[2] = 0xF2;
		pDot11RSNAuthSuite->dot11RSNIESuite[usSuitCount].Type = DOT11_AuthKeyType_PSK;
	    usSuitCount++;

		pDot11RSNAuthSuite->SuiteCount = cpu_to_le16(usSuitCount);
        ulAuthLength = sizeof(pDot11RSNAuthSuite->SuiteCount) + usSuitCount*sizeof(DOT11_RSN_IE_SUITE);
        ulIELength += ulAuthLength;

	//---------------------------------------------------------------------------------------------
	// Do not encapsulate capability field to solve TI WPA issue
	//---------------------------------------------------------------------------------------------
	/*
        dot11RSNCapability.field.PreAuthentication = 0;//auth->RSNVariable.isSupportPreAuthentication
        dot11RSNCapability.field.PairwiseAsDefaultKey = auth->RSNVariable.isSupportPairwiseAsDefaultKey;
        switch(auth->RSNVariable.NumOfRxTSC)
        {
        case 1:
	        dot11RSNCapability.field.NumOfReplayCounter = 0;
        	break;
	case 2:
		dot11RSNCapability.field.NumOfReplayCounter = 1;
		break;
	case 4:
		dot11RSNCapability.field.NumOfReplayCounter = 2;
		break;
	case 16:
		dot11RSNCapability.field.NumOfReplayCounter = 3;
        	break;
        default:
		dot11RSNCapability.field.NumOfReplayCounter = 0;
        }

        ulRSNCapabilityLength = sizeof(DOT11_RSN_CAPABILITY);
        ulIELength += ulRSNCapabilityLength;
	*/

		pucBlob = pucOut;
		pucBlob += sizeof(DOT11_RSN_IE_HEADER);
		memcpy(pucBlob, &dot11RSNGroupSuite, sizeof(DOT11_RSN_IE_SUITE));
		pucBlob += sizeof(DOT11_RSN_IE_SUITE);
		memcpy(pucBlob, pDot11RSNPairwiseSuite, ulPairwiseLength);
		pucBlob += ulPairwiseLength;
		memcpy(pucBlob, pDot11RSNAuthSuite, ulAuthLength);
		pucBlob += ulAuthLength;

		*usOutLen = (int)ulIELength;
		pucBlob = pucOut;
		dot11RSNIEHeader.Length = (unsigned char)ulIELength - 2; //This -2 is to minus elementID and Length in OUI header
		memcpy(pucBlob, &dot11RSNIEHeader, sizeof(DOT11_RSN_IE_HEADER));
	}

#ifdef RTL_WPA2
	if ( priv->pmib->dot1180211AuthEntry.dot11EnablePSK & PSK_WPA2 ) {
       	DOT11_WPA2_IE_HEADER dot11WPA2IEHeader = { 0 };
		ulIELength = 0;
		ulIndex = 0;
		ulPairwiseLength = 0;
		uCipherAlgo = 0;
		bCipherAlgoEnabled = FALSE;
		ulAuthLength = 0;
		uAuthAlgo = 0;
		bAuthAlgoEnabled = FALSE;
		ulRSNCapabilityLength = 0;

		//
		// Construct Information Header
		//
		dot11WPA2IEHeader.ElementID = WPA2_ELEMENT_ID;
		dot11WPA2IEHeader.Version = cpu_to_le16(RSN_VER1);
		ulIELength += sizeof(DOT11_WPA2_IE_HEADER);

		// Construct Cipher Suite:
		//      - Multicast Suite:
		//
		memset(&dot11RSNGroupSuite, 0, sizeof(dot11RSNGroupSuite));
		dot11RSNGroupSuite.OUI[0] = 0x00;
		dot11RSNGroupSuite.OUI[1] = 0x0F;
		dot11RSNGroupSuite.OUI[2] = 0xAC;
		dot11RSNGroupSuite.Type = priv->wpa_global_info->MulticastCipher;;
		ulIELength += sizeof(DOT11_RSN_IE_SUITE);

		//      - UnicastSuite
        pDot11RSNPairwiseSuite = &countSuite;
        memset(pDot11RSNPairwiseSuite, 0, sizeof(DOT11_RSN_IE_COUNT_SUITE));
		usSuitCount = 0;
#ifdef RTK_NL80211
		for (ulIndex=0; ulIndex<priv->wpa_global_info->NumOfUnicastCipherWPA2; ulIndex++)
		{
			int i = priv->wpa_global_info->NumOfUnicastCipherWPA2 - ulIndex - 1; 
			pDot11RSNPairwiseSuite->dot11RSNIESuite[usSuitCount].OUI[0] = 0x00;
			pDot11RSNPairwiseSuite->dot11RSNIESuite[usSuitCount].OUI[1] = 0x0F;
			pDot11RSNPairwiseSuite->dot11RSNIESuite[usSuitCount].OUI[2] = 0xAC;
			pDot11RSNPairwiseSuite->dot11RSNIESuite[usSuitCount].Type = priv->wpa_global_info->UnicastCipherWPA2[i];
			usSuitCount++;
		}
#else
		for (ulIndex=0; ulIndex<priv->wpa_global_info->NumOfUnicastCipherWPA2; ulIndex++)
        {
			pDot11RSNPairwiseSuite->dot11RSNIESuite[usSuitCount].OUI[0] = 0x00;
			pDot11RSNPairwiseSuite->dot11RSNIESuite[usSuitCount].OUI[1] = 0x0F;
			pDot11RSNPairwiseSuite->dot11RSNIESuite[usSuitCount].OUI[2] = 0xAC;
			pDot11RSNPairwiseSuite->dot11RSNIESuite[usSuitCount].Type = priv->wpa_global_info->UnicastCipherWPA2[ulIndex];
			usSuitCount++;
        }
#endif
		pDot11RSNPairwiseSuite->SuiteCount = cpu_to_le16(usSuitCount);
        ulPairwiseLength = sizeof(pDot11RSNPairwiseSuite->SuiteCount) + usSuitCount*sizeof(DOT11_RSN_IE_SUITE);
        ulIELength += ulPairwiseLength;

		//
		// Construction of Auth Algo List
		//
        pDot11RSNAuthSuite = &authCountSuite;
        memset(pDot11RSNAuthSuite, 0, sizeof(DOT11_RSN_IE_COUNT_SUITE));
		usSuitCount = 0;
		pDot11RSNAuthSuite->dot11RSNIESuite[usSuitCount].OUI[0] = 0x00;
		pDot11RSNAuthSuite->dot11RSNIESuite[usSuitCount].OUI[1] = 0x0F;
		pDot11RSNAuthSuite->dot11RSNIESuite[usSuitCount].OUI[2] = 0xAC;
		pDot11RSNAuthSuite->dot11RSNIESuite[usSuitCount].Type = DOT11_AuthKeyType_PSK;
	    usSuitCount++;

		pDot11RSNAuthSuite->SuiteCount = cpu_to_le16(usSuitCount);
        ulAuthLength = sizeof(pDot11RSNAuthSuite->SuiteCount) + usSuitCount*sizeof(DOT11_RSN_IE_SUITE);
        ulIELength += ulAuthLength;

		//---------------------------------------------------------------------------------------------
		// Do not encapsulate capability field to solve TI WPA issue
		//---------------------------------------------------------------------------------------------

//#ifdef RTL_WPA2
#if 1
		dot11RSNCapability.field.PreAuthentication = 0;
#else
		dot11RSNCapability.field.PairwiseAsDefaultKey = auth->RSNVariable.isSupportPairwiseAsDefaultKey;
		switch(auth->RSNVariable.NumOfRxTSC)
		{
		case 1:
			dot11RSNCapability.field.NumOfReplayCounter = 0;
			break;
		case 2:
			dot11RSNCapability.field.NumOfReplayCounter = 1;
			break;
		case 4:
			dot11RSNCapability.field.NumOfReplayCounter = 2;
			break;
		case 16:
			dot11RSNCapability.field.NumOfReplayCounter = 3;
			break;
		default:
			dot11RSNCapability.field.NumOfReplayCounter = 0;
		}
#endif

#if (defined(WIFI_HAPD) & defined(WIFI_WMM))  || (defined(RTK_NL80211) & defined(WIFI_WMM)) //eric-wds
		if(QOS_ENABLE){
			/* 4 PTKSA replay counters when using WMM consistent with hostapd code*/
			dot11RSNCapability.field.PtksaReplayCounter = 3;
		}
#endif

		ulRSNCapabilityLength = sizeof(DOT11_RSN_CAPABILITY);
		ulIELength += ulRSNCapabilityLength;

		pucBlob = pucOut + *usOutLen;
		pucBlob += sizeof(DOT11_WPA2_IE_HEADER);
		memcpy(pucBlob, &dot11RSNGroupSuite, sizeof(DOT11_RSN_IE_SUITE));
		pucBlob += sizeof(DOT11_RSN_IE_SUITE);
		memcpy(pucBlob, pDot11RSNPairwiseSuite, ulPairwiseLength);
		pucBlob += ulPairwiseLength;
		memcpy(pucBlob, pDot11RSNAuthSuite, ulAuthLength);
		pucBlob += ulAuthLength;
		memcpy(pucBlob, &dot11RSNCapability, ulRSNCapabilityLength);

		pucBlob = pucOut + *usOutLen;
		dot11WPA2IEHeader.Length = (unsigned char)ulIELength - 2; //This -2 is to minus elementID and Length in OUI header
		memcpy(pucBlob, &dot11WPA2IEHeader, sizeof(DOT11_WPA2_IE_HEADER));
		*usOutLen = *usOutLen + (int)ulIELength;
   	}
#endif // RTL_WPA2

}

static int MIN(unsigned char *ucStr1, unsigned char *ucStr2, unsigned int ulLen)
{
	int i;
	for (i=0 ; i<ulLen ; i++) {
		if ((unsigned char)ucStr1[i] < (unsigned char)ucStr2[i])
				return -1;
		else if((unsigned char)ucStr1[i] > (unsigned char)ucStr2[i])
			return 1;
		else if(i == ulLen - 1)
  			return 0;
		else
			continue;
	}
	return 0;
}

static int parseIE(struct rtl8192cd_priv *priv, WPA_STA_INFO *pInfo,
						unsigned char *pucIE, unsigned int ulIELength)
{
	unsigned short usSuitCount;
	DOT11_RSN_IE_HEADER *pDot11RSNIEHeader;
	DOT11_RSN_IE_SUITE *pDot11RSNIESuite;
	DOT11_RSN_IE_COUNT_SUITE *pDot11RSNIECountSuite;

	DEBUG_TRACE;

	if (ulIELength < sizeof(DOT11_RSN_IE_HEADER)) {
		DEBUG_WARN("parseIE err 1!\n");
		return ERROR_INVALID_RSNIE;
	}

	pDot11RSNIEHeader = (DOT11_RSN_IE_HEADER *)pucIE;
	if (le16_to_cpu(pDot11RSNIEHeader->Version) != RSN_VER1) {
		DEBUG_WARN("parseIE err 2!\n");
		return ERROR_UNSUPPORTED_RSNEVERSION;
	}

	if (pDot11RSNIEHeader->ElementID != RSN_ELEMENT_ID ||
		pDot11RSNIEHeader->Length != ulIELength -2 ||
		pDot11RSNIEHeader->OUI[0] != 0x00 || pDot11RSNIEHeader->OUI[1] != 0x50 ||
		pDot11RSNIEHeader->OUI[2] != 0xf2 || pDot11RSNIEHeader->OUI[3] != 0x01 ) {
		DEBUG_WARN("parseIE err 3!\n");
		return ERROR_INVALID_RSNIE;
	}

	pInfo->RSNEnabled= PSK_WPA;	// wpa
	ulIELength -= sizeof(DOT11_RSN_IE_HEADER);
	pucIE += sizeof(DOT11_RSN_IE_HEADER);

	//----------------------------------------------------------------------------------
 	// Multicast Cipher Suite processing
	//----------------------------------------------------------------------------------
	if (ulIELength < sizeof(DOT11_RSN_IE_SUITE))
		return 0;

	pDot11RSNIESuite = (DOT11_RSN_IE_SUITE *)pucIE;
	if (pDot11RSNIESuite->OUI[0] != 0x00 ||
		pDot11RSNIESuite->OUI[1] != 0x50 ||
		pDot11RSNIESuite->OUI[2] != 0xF2) {
		DEBUG_WARN("parseIE err 4!\n");
		return ERROR_INVALID_RSNIE;
	}

	if (pDot11RSNIESuite->Type > DOT11_ENC_WEP104) {
		DEBUG_WARN("parseIE err 5!\n");
		return ERROR_INVALID_MULTICASTCIPHER;
	}

	if (pDot11RSNIESuite->Type != priv->wpa_global_info->MulticastCipher) {
		DEBUG_WARN("parseIE err 6!\n");
		return ERROR_INVALID_MULTICASTCIPHER;
	}

	ulIELength -= sizeof(DOT11_RSN_IE_SUITE);
	pucIE += sizeof(DOT11_RSN_IE_SUITE);

	//----------------------------------------------------------------------------------
	// Pairwise Cipher Suite processing
	//----------------------------------------------------------------------------------
	if (ulIELength < 2 + sizeof(DOT11_RSN_IE_SUITE))
		return 0;

	pDot11RSNIECountSuite = (PDOT11_RSN_IE_COUNT_SUITE)pucIE;
	pDot11RSNIESuite = pDot11RSNIECountSuite->dot11RSNIESuite;
	usSuitCount = le16_to_cpu(pDot11RSNIECountSuite->SuiteCount);

	if (usSuitCount != 1 ||
		pDot11RSNIESuite->OUI[0] != 0x00 ||
		pDot11RSNIESuite->OUI[1] != 0x50 ||
		pDot11RSNIESuite->OUI[2] != 0xF2) {
		DEBUG_WARN("parseIE err 7!\n");
		return ERROR_INVALID_RSNIE;
	}

	if (pDot11RSNIESuite->Type > DOT11_ENC_WEP104) {
		DEBUG_WARN("parseIE err 8!\n");
		return ERROR_INVALID_UNICASTCIPHER;
	}

	if ((pDot11RSNIESuite->Type < DOT11_ENC_WEP40)
		|| (!(BIT(pDot11RSNIESuite->Type - 1) & priv->pmib->dot1180211AuthEntry.dot11WPACipher))) {
		DEBUG_WARN("parseIE err 9!\n");
		return ERROR_INVALID_UNICASTCIPHER;
	}

	pInfo->UnicastCipher = pDot11RSNIESuite->Type;

#ifdef DEBUG_PSK
	printk("PSK: ParseIE -> WPA UnicastCipher=%x\n", pInfo->UnicastCipher);
#endif

	ulIELength -= sizeof(pDot11RSNIECountSuite->SuiteCount) + sizeof(DOT11_RSN_IE_SUITE);
	pucIE += sizeof(pDot11RSNIECountSuite->SuiteCount) + sizeof(DOT11_RSN_IE_SUITE);

	//----------------------------------------------------------------------------------
	// Authentication suite
	//----------------------------------------------------------------------------------
	if (ulIELength < 2 + sizeof(DOT11_RSN_IE_SUITE))
		return 0;

	pDot11RSNIECountSuite = (PDOT11_RSN_IE_COUNT_SUITE)pucIE;
	pDot11RSNIESuite = pDot11RSNIECountSuite->dot11RSNIESuite;
	usSuitCount = le16_to_cpu(pDot11RSNIECountSuite->SuiteCount);

	if (usSuitCount != 1 ||
		pDot11RSNIESuite->OUI[0] != 0x00 ||
		pDot11RSNIESuite->OUI[1] != 0x50 ||
		pDot11RSNIESuite->OUI[2] != 0xF2 ) {
		DEBUG_WARN("parseIE err 10!\n");
		return ERROR_INVALID_RSNIE;
	}

	if( pDot11RSNIESuite->Type < DOT11_AuthKeyType_RSN ||
		pDot11RSNIESuite->Type > DOT11_AuthKeyType_PSK) {
		DEBUG_WARN("parseIE err 11!\n");
		return ERROR_INVALID_AUTHKEYMANAGE;
	}

	if(pDot11RSNIESuite->Type != DOT11_AuthKeyType_PSK) {
		DEBUG_WARN("parseIE err 12!\n");
		return ERROR_INVALID_AUTHKEYMANAGE;
	}

	//pInfo->AuthKeyMethod = pDot11RSNIESuite->Type;
	ulIELength -= sizeof(pDot11RSNIECountSuite->SuiteCount) + sizeof(DOT11_RSN_IE_SUITE);
	pucIE += sizeof(pDot11RSNIECountSuite->SuiteCount) + sizeof(DOT11_RSN_IE_SUITE);

	// RSN Capability
	if (ulIELength < sizeof(DOT11_RSN_CAPABILITY))
		return 0;

//#ifndef RTL_WPA2
#if 0
	//----------------------------------------------------------------------------------
    // Capability field
	//----------------------------------------------------------------------------------
	pDot11RSNCapability = (DOT11_RSN_CAPABILITY * )pucIE;
	pInfo->isSuppSupportPreAuthentication = pDot11RSNCapability->field.PreAuthentication;
	pInfo->isSuppSupportPairwiseAsDefaultKey = pDot11RSNCapability->field.PairwiseAsDefaultKey;

	switch (pDot11RSNCapability->field.NumOfReplayCounter) {
	case 0:
		pInfo->NumOfRxTSC = 1;
		break;
	case 1:
		pInfo->NumOfRxTSC = 2;
		break;
	case 2:
		pInfo->NumOfRxTSC = 4;
		break;
	case 3:
		pInfo->NumOfRxTSC = 16;
		break;
	default:
		pInfo->NumOfRxTSC = 1;
	}
#endif /* RTL_WPA2 */

	return 0;
}

#ifdef RTL_WPA2
static int parseIEWPA2(struct rtl8192cd_priv *priv, WPA_STA_INFO *pInfo,
						unsigned char *pucIE, unsigned int ulIELength)
{
	unsigned short usSuitCount;
	DOT11_WPA2_IE_HEADER *pDot11WPA2IEHeader = NULL;
	DOT11_RSN_IE_SUITE  *pDot11RSNIESuite = NULL;
	DOT11_RSN_IE_COUNT_SUITE *pDot11RSNIECountSuite = NULL;
	DOT11_RSN_CAPABILITY *pDot11RSNCapability = NULL;

	DEBUG_TRACE;

	if (ulIELength < sizeof(DOT11_WPA2_IE_HEADER)) {
		DEBUG_WARN("ERROR_INVALID_RSNIE, err 1!\n");
		return ERROR_INVALID_RSNIE;
	}

	pDot11WPA2IEHeader = (DOT11_WPA2_IE_HEADER *)pucIE;
	if (le16_to_cpu(pDot11WPA2IEHeader->Version) != RSN_VER1) {
		DEBUG_WARN("ERROR_UNSUPPORTED_RSNEVERSION, err 2!\n");
		return ERROR_UNSUPPORTED_RSNEVERSION;
	}

	if (pDot11WPA2IEHeader->ElementID != WPA2_ELEMENT_ID ||
		pDot11WPA2IEHeader->Length != ulIELength -2 ) {
		DEBUG_WARN("ERROR_INVALID_RSNIE, err 3!\n");
		return ERROR_INVALID_RSNIE;
	}

	pInfo->RSNEnabled= PSK_WPA2;
	//pInfo->PMKCached= FALSE;

	ulIELength -= sizeof(DOT11_WPA2_IE_HEADER);
	pucIE += sizeof(DOT11_WPA2_IE_HEADER);

	//----------------------------------------------------------------------------------
 	// Multicast Cipher Suite processing
	//----------------------------------------------------------------------------------
	if (ulIELength < sizeof(DOT11_RSN_IE_SUITE))
		return 0;

	pDot11RSNIESuite = (DOT11_RSN_IE_SUITE *)pucIE;
	if (pDot11RSNIESuite->OUI[0] != 0x00 ||
			pDot11RSNIESuite->OUI[1] != 0x0F ||
				pDot11RSNIESuite->OUI[2] != 0xAC) {
		DEBUG_WARN("ERROR_INVALID_RSNIE, err 4!\n");
		return ERROR_INVALID_RSNIE;
	}

	if (pDot11RSNIESuite->Type > DOT11_ENC_WEP104)	{
		DEBUG_WARN("ERROR_INVALID_MULTICASTCIPHER, err 5!\n");
		return ERROR_INVALID_MULTICASTCIPHER;
	}

	if (pDot11RSNIESuite->Type != priv->wpa_global_info->MulticastCipher) {
		DEBUG_WARN("ERROR_INVALID_MULTICASTCIPHER, err 6!\n");
		return ERROR_INVALID_MULTICASTCIPHER;
	}

	ulIELength -= sizeof(DOT11_RSN_IE_SUITE);
	pucIE += sizeof(DOT11_RSN_IE_SUITE);

	//----------------------------------------------------------------------------------
	// Pairwise Cipher Suite processing
	//----------------------------------------------------------------------------------
	if (ulIELength < 2 + sizeof(DOT11_RSN_IE_SUITE))
		return 0;

	pDot11RSNIECountSuite = (PDOT11_RSN_IE_COUNT_SUITE)pucIE;
	pDot11RSNIESuite = pDot11RSNIECountSuite->dot11RSNIESuite;
	usSuitCount = le16_to_cpu(pDot11RSNIECountSuite->SuiteCount);

	if (usSuitCount != 1 ||
		pDot11RSNIESuite->OUI[0] != 0x00 ||
			pDot11RSNIESuite->OUI[1] != 0x0F ||
				pDot11RSNIESuite->OUI[2] != 0xAC) {
		DEBUG_WARN("ERROR_INVALID_RSNIE, err 7!\n");
		return ERROR_INVALID_RSNIE;
	}

	if (pDot11RSNIESuite->Type > DOT11_ENC_WEP104) {
		DEBUG_WARN("ERROR_INVALID_UNICASTCIPHER, err 8!\n");
		return ERROR_INVALID_UNICASTCIPHER;
	}

	if ((pDot11RSNIESuite->Type < DOT11_ENC_WEP40)
		|| (!(BIT(pDot11RSNIESuite->Type - 1) & priv->pmib->dot1180211AuthEntry.dot11WPA2Cipher))) {
		DEBUG_WARN("ERROR_INVALID_UNICASTCIPHER, err 9!\n");
		return ERROR_INVALID_UNICASTCIPHER;
	}

	pInfo->UnicastCipher = pDot11RSNIESuite->Type;
	ulIELength -= sizeof(pDot11RSNIECountSuite->SuiteCount) + sizeof(DOT11_RSN_IE_SUITE);
	pucIE += sizeof(pDot11RSNIECountSuite->SuiteCount) + sizeof(DOT11_RSN_IE_SUITE);

#ifdef DEBUG_PSK
	//printk("PSK: ParseIE -> WPA2 UnicastCipher=%x\n", pInfo->UnicastCipher);
#endif

	//----------------------------------------------------------------------------------
	// Authentication suite
	//----------------------------------------------------------------------------------
	if (ulIELength < 2 + sizeof(DOT11_RSN_IE_SUITE))
		return 0;

	pDot11RSNIECountSuite = (PDOT11_RSN_IE_COUNT_SUITE)pucIE;
	pDot11RSNIESuite = pDot11RSNIECountSuite->dot11RSNIESuite;
	usSuitCount = le16_to_cpu(pDot11RSNIECountSuite->SuiteCount);

	if (usSuitCount != 1 ||
			pDot11RSNIESuite->OUI[0] != 0x00 ||
				pDot11RSNIESuite->OUI[1] != 0x0F ||
					pDot11RSNIESuite->OUI[2] != 0xAC ) {
		DEBUG_WARN("ERROR_INVALID_RSNIE, err 10!\n");
		return ERROR_INVALID_RSNIE;
	}

	if (pDot11RSNIESuite->Type < DOT11_AuthKeyType_RSN ||
		  pDot11RSNIESuite->Type > DOT11_AuthKeyType_PSK) {
		DEBUG_WARN("ERROR_INVALID_AUTHKEYMANAGE, err 11!\n");
		return ERROR_INVALID_AUTHKEYMANAGE;
	}

	if(pDot11RSNIESuite->Type != DOT11_AuthKeyType_PSK) {
		DEBUG_WARN("ERROR_INVALID_AUTHKEYMANAGE, err 12!\n");
		return ERROR_INVALID_AUTHKEYMANAGE;
	}

	//pInfo->AuthKeyMethod = pDot11RSNIESuite->Type;
	ulIELength -= sizeof(pDot11RSNIECountSuite->SuiteCount) + sizeof(DOT11_RSN_IE_SUITE);
	pucIE += sizeof(pDot11RSNIECountSuite->SuiteCount) + sizeof(DOT11_RSN_IE_SUITE);

	// RSN Capability
	if (ulIELength < sizeof(DOT11_RSN_CAPABILITY)) {
		//pInfo->NumOfRxTSC = 2;
		return 0;
	}

	//----------------------------------------------------------------------------------
	// Capability field
	//----------------------------------------------------------------------------------
	pDot11RSNCapability = (DOT11_RSN_CAPABILITY * )pucIE;
#if 0
	global->RSNVariable.isSuppSupportPreAuthentication = pDot11RSNCapability->field.PreAuthentication;
//#ifdef RTL_WPA2_PREAUTH  // kenny temp
	//wpa2_hexdump("WPA2 IE Capability", pucIE, 2);
	//global->RSNVariable.isSuppSupportPreAuthentication = (pDot11RSNCapability->charData[0] & 0x01)?TRUE:FALSE;
#endif

#if 1
	//pInfo->NumOfRxTSC = 1;
#else
	global->RSNVariable.isSuppSupportPairwiseAsDefaultKey = pDot11RSNCapability->field.PairwiseAsDefaultKey;
	switch (pDot11RSNCapability->field.NumOfReplayCounter) {
	case 0:
		global->RSNVariable.NumOfRxTSC = 1;
		break;
	case 1:
		global->RSNVariable.NumOfRxTSC = 2;
		break;
	case 2:
		global->RSNVariable.NumOfRxTSC = 4;
		break;
	case 3:
		global->RSNVariable.NumOfRxTSC = 16;
		break;
	default:
		global->RSNVariable.NumOfRxTSC = 1;
	}
#endif
	pucIE += 2;

	// PMKID
	if ((ulIELength < 2 + PMKID_LEN))
		return 0;

	//----------------------------------------------------------------------------------
	// PMKID Count field
	//----------------------------------------------------------------------------------
	usSuitCount = le16_to_cpu(*((unsigned short *)pucIE));

	//printf("PMKID Count = %d\n",usSuitCount);
	pucIE += 2;
/*
	if ( usSuitCount > 0) {
		struct _WPA2_PMKSA_Node* pmksa_node;
		int i;
		for (i=0; i < usSuitCount; i++) {
			pmksa_node = find_pmksa(pucIE+(PMKID_LEN*i));
			if ( pmksa_node != NULL) {
				//wpa2_hexdump("Cached PMKID found", pmksa_node->pmksa.pmkid, PMKID_LEN);
				global->RSNVariable.PMKCached = TRUE;
				global->RSNVariable.cached_pmk_node = pmksa_node;
				break;
			}
		}

	}
*/
	return 0;
}
#endif // RTL_WPA2

static void ToDrv_RspAssoc(struct rtl8192cd_priv *priv, int id, unsigned char *mac, int status)
{
	DOT11_ASSOCIATIIN_RSP 	Association_Rsp;
	struct iw_point wrq;

	DEBUG_TRACE;

#ifdef DEBUG_PSK
	printk("PSK: Issue assoc-rsp [%x], mac:%02x:%02x:%02x:%02x:%02x:%02x\n",
		status, mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
#endif

	wrq.pointer = (caddr_t)&Association_Rsp;
	wrq.length = sizeof(DOT11_ASSOCIATIIN_RSP);

	if (id == DOT11_EVENT_ASSOCIATION_IND)
		Association_Rsp.EventId = DOT11_EVENT_ASSOCIATION_RSP;
	else
		Association_Rsp.EventId = DOT11_EVENT_REASSOCIATION_RSP;

	Association_Rsp.IsMoreEvent = FALSE;
	Association_Rsp.Status = status;
	memcpy(Association_Rsp.MACAddr, mac, 6);

	rtl8192cd_ioctl_priv_daemonreq(priv->dev, &wrq);
}


static void ToDrv_RemovePTK(struct rtl8192cd_priv *priv, unsigned char *mac, int type)
{
	struct iw_point wrq;
	DOT11_DELETE_KEY Delete_Key;

#ifdef DEBUG_PSK
	printk("PSK: Remove PTK, mac: %02x:%02x:%02x:%02x:%02x:%02x\n",
		mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
#endif

	wrq.pointer = (caddr_t)&Delete_Key;
	wrq.length = sizeof(DOT11_DELETE_KEY);

	Delete_Key.EventId = DOT11_EVENT_DELETE_KEY;
	Delete_Key.IsMoreEvent = FALSE;
	Delete_Key.KeyType = type;
	memcpy(&Delete_Key.MACAddr, mac, 6);

	rtl8192cd_ioctl_priv_daemonreq(priv->dev, &wrq);
}

static void ToDrv_SetPort(struct rtl8192cd_priv *priv, struct stat_info *pstat, int status)
{
	struct iw_point wrq;
	DOT11_SET_PORT	Set_Port;

#ifdef DEBUG_PSK
	printk("PSK: Set PORT [%x], mac: %02x:%02x:%02x:%02x:%02x:%02x\n",
		status, pstat->hwaddr[0], pstat->hwaddr[1], pstat->hwaddr[2],
		pstat->hwaddr[3], pstat->hwaddr[4], pstat->hwaddr[5]);
#endif

	wrq.pointer = (caddr_t)&Set_Port;
	wrq.length = sizeof(DOT11_SET_PORT);
	Set_Port.EventId = DOT11_EVENT_SET_PORT;
	Set_Port.PortStatus = status;
	memcpy(&Set_Port.MACAddr, pstat->hwaddr, 6);
	rtl8192cd_ioctl_priv_daemonreq(priv->dev, &wrq);
}

static void ToDrv_SetIE(struct rtl8192cd_priv *priv)
{
	struct iw_point wrq;
	DOT11_SET_RSNIE Set_Rsnie;

#ifdef DEBUG_PSK
	debug_out("PSK: Set RSNIE", priv->wpa_global_info->AuthInfoElement.Octet,
								priv->wpa_global_info->AuthInfoElement.Length);
#endif

	wrq.pointer = (caddr_t)&Set_Rsnie;
	wrq.length = sizeof(DOT11_SET_RSNIE);
	Set_Rsnie.EventId = DOT11_EVENT_SET_RSNIE;
	Set_Rsnie.IsMoreEvent = FALSE;
	Set_Rsnie.Flag = DOT11_Ioctl_Set;
	Set_Rsnie.RSNIELen = priv->wpa_global_info->AuthInfoElement.Length;
	memcpy(&Set_Rsnie.RSNIE,
			priv->wpa_global_info->AuthInfoElement.Octet,
			priv->wpa_global_info->AuthInfoElement.Length);

	rtl8192cd_ioctl_priv_daemonreq(priv->dev, &wrq);
}

static void reset_sta_info(struct rtl8192cd_priv *priv, struct stat_info *pstat)
{
	WPA_STA_INFO *pInfo = pstat->wpa_sta_info;
#ifndef SMP_SYNC
	unsigned long flags;
#endif

	SAVE_INT_AND_CLI(flags);

	if (OPMODE & WIFI_AP_STATE)
	{
		ToDrv_RemovePTK(priv, pstat->hwaddr, DOT11_KeyType_Pairwise);
		ToDrv_SetPort(priv, pstat, DOT11_PortStatus_Unauthorized);
	}

	memset((char *)pInfo, '\0', sizeof(WPA_STA_INFO));

	pInfo->priv = priv;

	if (OPMODE & WIFI_AP_STATE)
	{
		pInfo->state = PSK_STATE_IDLE;
	}

	RESTORE_INT(flags);
}

void psk_init(struct rtl8192cd_priv *priv)
{
	WPA_GLOBAL_INFO *pGblInfo=priv->wpa_global_info;
	int i, j, low_cipher=0;

	DEBUG_TRACE;

	memset((char *)pGblInfo, '\0', sizeof(WPA_GLOBAL_INFO));

	if ((priv->pmib->dot1180211AuthEntry.dot11EnablePSK & PSK_WPA) &&
			!priv->pmib->dot1180211AuthEntry.dot11WPACipher) {
		DEBUG_ERR("psk_init failed, WPA cipher did not set!\n");
		return;
	}

#ifdef RTL_WPA2
	if ((priv->pmib->dot1180211AuthEntry.dot11EnablePSK & PSK_WPA2) &&
			!priv->pmib->dot1180211AuthEntry.dot11WPA2Cipher) {
		DEBUG_ERR("psk_init failed, WPA2 cipher did not set!\n");
		return;
	}
#endif

	if ((priv->pmib->dot1180211AuthEntry.dot11EnablePSK & PSK_WPA2) &&
			!(priv->pmib->dot1180211AuthEntry.dot11EnablePSK & PSK_WPA)) {
		if (priv->pmib->dot1180211AuthEntry.dot11WPACipher)
			priv->pmib->dot1180211AuthEntry.dot11WPACipher = 0;
	}
	if ((priv->pmib->dot1180211AuthEntry.dot11EnablePSK & PSK_WPA) &&
			!(priv->pmib->dot1180211AuthEntry.dot11EnablePSK & PSK_WPA2)) {
		if (priv->pmib->dot1180211AuthEntry.dot11WPA2Cipher)
			priv->pmib->dot1180211AuthEntry.dot11WPA2Cipher = 0;
	}

	if (priv->pmib->dot1180211AuthEntry.dot11WPACipher) {
		for (i=0, j=0; i<_WEP_104_PRIVACY_; i++) {
			if (priv->pmib->dot1180211AuthEntry.dot11WPACipher & (1<<i)) {
				pGblInfo->UnicastCipher[j] = i+1;
				if (low_cipher == 0)
					low_cipher = pGblInfo->UnicastCipher[j];
				else {
					if (low_cipher == _WEP_104_PRIVACY_ &&
							pGblInfo->UnicastCipher[j] == _WEP_40_PRIVACY_)
						low_cipher = pGblInfo->UnicastCipher[j];
					else if (low_cipher == _TKIP_PRIVACY_ &&
							(pGblInfo->UnicastCipher[j] == _WEP_40_PRIVACY_ ||
								pGblInfo->UnicastCipher[j] == _WEP_104_PRIVACY_))
							low_cipher = pGblInfo->UnicastCipher[j];
					else if (low_cipher == _CCMP_PRIVACY_)
							low_cipher = pGblInfo->UnicastCipher[j];
				}
				if (++j >= MAX_UNICAST_CIPHER)
					break;
			}
		}
		pGblInfo->NumOfUnicastCipher = j;
	}

#ifdef RTL_WPA2
	if (priv->pmib->dot1180211AuthEntry.dot11WPA2Cipher) {
		for (i=0, j=0; i<_WEP_104_PRIVACY_; i++) {
			if (priv->pmib->dot1180211AuthEntry.dot11WPA2Cipher & (1<<i)) {
				pGblInfo->UnicastCipherWPA2[j] = i+1;
				if (low_cipher == 0)
					low_cipher = pGblInfo->UnicastCipherWPA2[j];
				else {
					if (low_cipher == _WEP_104_PRIVACY_ &&
							pGblInfo->UnicastCipherWPA2[j] == _WEP_40_PRIVACY_)
						low_cipher = pGblInfo->UnicastCipherWPA2[j];
					else if (low_cipher == _TKIP_PRIVACY_ &&
							(pGblInfo->UnicastCipherWPA2[j] == _WEP_40_PRIVACY_ ||
								pGblInfo->UnicastCipherWPA2[j] == _WEP_104_PRIVACY_))
							low_cipher = pGblInfo->UnicastCipherWPA2[j];
					else if (low_cipher == _CCMP_PRIVACY_)
							low_cipher = pGblInfo->UnicastCipherWPA2[j];
				}
				if (++j >= MAX_UNICAST_CIPHER)
					break;
			}
		}
		pGblInfo->NumOfUnicastCipherWPA2= j;
	}
#endif

	pGblInfo->MulticastCipher = low_cipher;

#ifdef DEBUG_PSK
	printk("PSK: WPA unicast cipher= ");
	for (i=0; i<pGblInfo->NumOfUnicastCipher; i++)
		printk("%x ", pGblInfo->UnicastCipher[i]);
	printk("\n");

#ifdef RTL_WPA2
	printk("PSK: WPA2 unicast cipher= ");
	for (i=0; i<pGblInfo->NumOfUnicastCipherWPA2; i++)
		printk("%x ", pGblInfo->UnicastCipherWPA2[i]);
	printk("\n");
#endif

	printk("PSK: multicast cipher= %x\n", pGblInfo->MulticastCipher);
#endif

	pGblInfo->AuthInfoElement.Octet = pGblInfo->AuthInfoBuf;

	ConstructIE(priv, pGblInfo->AuthInfoElement.Octet,
					 &pGblInfo->AuthInfoElement.Length);

	ToDrv_SetIE(priv);
}

#if	(defined(WIFI_HAPD) || defined(RTK_NL80211)) && defined(WDS)
void wds_psk_set(struct rtl8192cd_priv *priv, int idx, unsigned char *key)
{
#if !defined(WIFI_HAPD) // && !defined(RTK_NL80211)
	unsigned char pchar[40];

	if (key == NULL) {
		if (strlen(priv->pmib->dot11WdsInfo.wdsPskPassPhrase) == 64) // hex
			get_array_val(priv->pmib->dot11WdsInfo.wdsMapingKey[idx], priv->pmib->dot11WdsInfo.wdsPskPassPhrase, 64);	
		else {		
			memset(pchar, 0, sizeof(unsigned char)*40);
				PasswordHash(priv->pmib->dot11WdsInfo.wdsPskPassPhrase,"REALTEK", strlen("REALTEK"), pchar);
			memcpy(priv->pmib->dot11WdsInfo.wdsMapingKey[idx], pchar, sizeof(unsigned char)*32);			
			}
		}
	else
		memcpy(priv->pmib->dot11WdsInfo.wdsMapingKey[idx], key, sizeof(unsigned char)*32);
	
	priv->pmib->dot11WdsInfo.wdsMappingKeyLen[idx] = 32;
	priv->pmib->dot11WdsInfo.wdsMappingKeyLen[idx] |= 0x80000000;  //set bit to protect the key	
#endif
}

void wds_psk_init(struct rtl8192cd_priv *priv)
{
	unsigned char *key;
	int i;

	if ( !(OPMODE & WIFI_AP_STATE))
		return;

	for (i = 0; i < priv->pmib->dot11WdsInfo.wdsNum; i++) {
		if (i==0)
			key = NULL;
		else
			key = priv->pmib->dot11WdsInfo.wdsMapingKey[0];
			
		wds_psk_set(priv, i, key);
	}
}
void hapd_set_wdskey(struct net_device *dev, char *wdsPskPassPhrase, char *ssid, int wds_num)
{
	struct rtl8192cd_priv *priv = GET_DEV_PRIV(dev);
	int idx = 0;
	unsigned char pchar[40];

	memcpy(priv->pmib->dot11WdsInfo.wdsPskPassPhrase, wdsPskPassPhrase, strlen(wdsPskPassPhrase));	
	priv->pmib->dot11WdsInfo.wdsPskPassPhrase[strlen(wdsPskPassPhrase)] = '\0';

	memset(pchar, 0, sizeof(unsigned char)*40);
	PasswordHash(wdsPskPassPhrase, ssid, strlen(ssid), pchar);

	for(idx =0; idx<wds_num; idx++)
	{
		memcpy(priv->pmib->dot11WdsInfo.wdsMapingKey[idx], pchar, sizeof(unsigned char)*32);		
		priv->pmib->dot11WdsInfo.wdsMappingKeyLen[idx] = 32;
		priv->pmib->dot11WdsInfo.wdsMappingKeyLen[idx] |= 0x80000000;  //set bit to protect the key	
	}

}
#endif

int psk_indicate_evt(struct rtl8192cd_priv *priv, int id, unsigned char *mac, unsigned char *msg, int len)
{
	struct stat_info *pstat;
	unsigned char tmpbuf[1024];
	int ret;
#ifdef RTL_WPA2
	int isWPA2=0;
#endif

	if (!priv->pmib->dot1180211AuthEntry.dot11EnablePSK ||
		!((priv->pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm == _TKIP_PRIVACY_) ||
		  (priv->pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm == _CCMP_PRIVACY_)))
			return -1;

#ifdef DEBUG_PSK
	printk("PSK: Got evt:%s[%x], sta: %02x:%02x:%02x:%02x:%02x:%02x, msg_len=%x\n",
			ID2STR(id), id,
			mac[0], mac[1], mac[2], mac[3], mac[4], mac[5], len);
#endif

	pstat = get_stainfo(priv, mac);
		// button 2009.05.21
#if 0
		if (pstat == NULL) {
#else
	if (pstat == NULL && id!=DOT11_EVENT_WPA_MULTICAST_CIPHER && id!=DOT11_EVENT_WPA2_MULTICAST_CIPHER) {
#endif
		DEBUG_ERR("Invalid mac address: %02x:%02x:%02x:%02x:%02x:%02x\n",
			mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
				return -1;
	}

	switch (id) {
	case DOT11_EVENT_ASSOCIATION_IND:
	case DOT11_EVENT_REASSOCIATION_IND:

		reset_sta_info(priv, pstat);

		if (OPMODE & WIFI_AP_STATE) {
			// check RSNIE
			if (len > 2 && msg != NULL) {
#ifdef DEBUG_PSK
				debug_out("PSK: Rx Assoc-ind, RSNIE", msg, len);
#endif

#ifdef RTL_WPA2
				memcpy(tmpbuf, msg, len);
				len -= 2;
#else
				tmpbuf[0] = RSN_ELEMENT_ID;
				tmpbuf[1] = len;
				memcpy(tmpbuf+2, msg, len);
#endif

#ifdef RTL_WPA2
				isWPA2 = (tmpbuf[0] == WPA2_ELEMENT_ID) ? 1 : 0;
				if (isWPA2)
					ret = parseIEWPA2(priv, pstat->wpa_sta_info, tmpbuf, len+2);
				else
#endif
					ret = parseIE(priv, pstat->wpa_sta_info, tmpbuf, len+2);
				if (ret != 0) {
					DEBUG_ERR("parse IE error [%x]!\n", ret);
				}

				// issue assoc-rsp successfully
				ToDrv_RspAssoc(priv, id, mac, -ret);

				if (ret == 0) {
#ifdef EVENT_LOG
					char *pmsg;
					switch (pstat->wpa_sta_info->UnicastCipher) {
						case DOT11_ENC_NONE:	pmsg = "none"; 	break;
						case DOT11_ENC_WEP40:	pmsg = "WEP40"; 	break;
						case DOT11_ENC_TKIP:	pmsg = "TKIP"; 	break;
						case DOT11_ENC_WRAP:	pmsg = "AES";	break;
						case DOT11_ENC_CCMP:	pmsg = "AES";	break;
						case DOT11_ENC_WEP104:	pmsg = "WEP104";	break;
						default: pmsg = "invalid algorithm";			break;
					}
#ifdef RTL_WPA2
					LOG_MSG("%s-%s PSK authentication in progress...\n", (isWPA2 ? "WPA2" : "WPA"), pmsg);
#else
					LOG_MSG("%s-WPA PSK authentication in progress...\n",  pmsg);
#endif
#endif

#if defined(CONFIG_RTL8196B_TR) || defined(CONFIG_RTL865X_SC) || defined(CONFIG_RTL865X_AC) || defined(CONFIG_RTL865X_KLD) || defined(CONFIG_RTL8196B_KLD) || defined(CONFIG_RTL8196C_KLD) || defined(CONFIG_RTL8196C_EC)
					LOG_MSG_NOTICE("Authenticating......;note:%02x-%02x-%02x-%02x-%02x-%02x;\n",
						pstat->hwaddr[0],
						pstat->hwaddr[1],
						pstat->hwaddr[2],
						pstat->hwaddr[3],
						pstat->hwaddr[4],
						pstat->hwaddr[5]);
#endif
				}
			}
			else { // RNSIE is null
				if (priv->pmib->dot1180211AuthEntry.dot11EnablePSK)
					ToDrv_RspAssoc(priv, id, mac, -ERROR_INVALID_RSNIE);
			}
		}
		break;

	case DOT11_EVENT_DISASSOCIATION_IND:
		reset_sta_info(priv, pstat);
		break;

	case DOT11_EVENT_MIC_FAILURE:
		// do nothing
		break;
		
	}

			return 0;
}

#endif


