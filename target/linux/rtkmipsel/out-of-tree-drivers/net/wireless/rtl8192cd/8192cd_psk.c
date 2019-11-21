/*
 *  WPA PSK handling routines
 *
 *  $Id: 8192cd_psk.c,v 1.6.2.3 2011/01/07 06:55:01 victoryman Exp $
 *
 *  Copyright (c) 2009 Realtek Semiconductor Corp.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 */

#define _8192CD_PSK_C_

#include <linux/module.h>
#include <linux/netdevice.h>
#include <linux/timer.h>
#include <linux/random.h>

#include "./8192cd_cfg.h"

#if (defined(INCLUDE_WPA_PSK) && !defined(WIFI_HAPD) && !defined(RTK_NL80211)) || defined(HAPD_DRV_PSK_WPS)

#include "./8192cd.h"
#include "./wifi.h"
#include "./8192cd_util.h"
#include "./8192cd_headers.h"
#include "./8192cd_security.h"
#include "./ieee802_mib.h"
#include "./8192cd_debug.h"
#include "./8192cd_psk.h"
#include "./1x_rc4.h"





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

#define IGMK_EXPANSION_CONST 	        "IGTK key expansion"
#define IGMK_EXPANSION_CONST_SIZE		18

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
#define RESEND_TIME						RTL_SECONDS_TO_JIFFIES(1)	// in 10ms

#ifdef CLIENT_MODE
#define WAIT_EAP_TIME				RTL_SECONDS_TO_JIFFIES(5)
#endif

#define LargeIntegerOverflow(x) (x.field.HighPart == 0xffffffff) && \
								(x.field.LowPart == 0xffffffff)
#define LargeIntegerZero(x) memset(&x.charData, 0, 8);

#define Octet16IntegerOverflow(x) LargeIntegerOverflow(x.field.HighPart) && \
								  LargeIntegerOverflow(x.field.LowPart)
#define Octet16IntegerZero(x) memset(&x.charData, 0, 16);

#define SetNonce(ocDst, oc32Counter) SetEAPOL_KEYIV(ocDst, oc32Counter)

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
#ifdef CLIENT_MODE
extern void AES_UnWRAP(unsigned char *cipher, int cipher_len, unsigned char *kek,
					   int kek_len, unsigned char *plain);
#endif
#endif

static void UpdateGK(struct rtl8192cd_priv *priv);
static void SendEAPOL(struct rtl8192cd_priv *priv, struct stat_info *pstat, int resend);
static void ResendTimeout(unsigned long task_psta);
static void reset_sta_info(struct rtl8192cd_priv *priv, struct stat_info *pstat);


#ifdef DEBUG_PSK
static char *ID2STR(int id)
{
	switch (id) {
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

static char *STATE2RXMSG(struct stat_info *pstat)
{
	if (Message_KeyType(pstat->wpa_sta_info->EapolKeyMsgRecvd) == type_Pairwise) {
		if (pstat->wpa_sta_info->state == PSK_STATE_PTKSTART)
			return ("4-2");
		else if (pstat->wpa_sta_info->state == PSK_STATE_PTKINITNEGOTIATING) {
			if (Message_KeyDataLength(pstat->wpa_sta_info->EapolKeyMsgRecvd) != 0)
				return ("4-2 (duplicated)");
			else
				return ("4-4");
		} else if (pstat->wpa_sta_info->state == PSK_STATE_PTKINITDONE)
			return ("4-4 (duplicated)");
		else
			return ("invalid state");

	} else
		return ("2-2");

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
	int offset = 0;
	int step = 20;
	int IterationNum = (digest_len + step - 1) / step;

	for (i = 0; i < IterationNum; i++) {
		text[text_len] = (unsigned char)i;
		hmac_sha(key, key_len, text, text_len + 1, digest + offset, step);
		offset += step;
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
	memcpy(data, prefix, prefix_len);
	data[prefix_len++] = 0;
	memcpy(data + prefix_len, random, random_len);
	i_P_SHA1(secret, secret_len, data, prefix_len + random_len, digest, digest_len);
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
	digest[ssidlength] = (unsigned char)((count >> 24) & 0xff);
	digest[ssidlength + 1] = (unsigned char)((count >> 16) & 0xff);
	digest[ssidlength + 2] = (unsigned char)((count >> 8) & 0xff);
	digest[ssidlength + 3] = (unsigned char)(count & 0xff);
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

static void Message_ReplayCounter_OC2LI(OCTET_STRING f, LARGE_INTEGER * li)
{
	li->field.HighPart = ((unsigned int)(*(f.Octet + ReplayCounterPos + 3)))
						 + ((unsigned int)(*(f.Octet + ReplayCounterPos + 2)) << 8 )
						 + ((unsigned int)(*(f.Octet + ReplayCounterPos + 1)) <<  16)
						 + ((unsigned int)(*(f.Octet + ReplayCounterPos + 0)) << 24);
	li->field.LowPart =  ((unsigned int)(*(f.Octet + ReplayCounterPos + 7)))
						 + ((unsigned int)(*(f.Octet + ReplayCounterPos + 6)) << 8 )
						 + ((unsigned int)(*(f.Octet + ReplayCounterPos + 5)) <<  16)
						 + ((unsigned int)(*(f.Octet + ReplayCounterPos + 4)) << 24);
}

#if 1
/*-----------------------------------------------------------------------------------------------
	f is EAPOL-KEY message
------------------------------------------------------------------------------------------------*/
static int Message_EqualReplayCounter(LARGE_INTEGER li1, OCTET_STRING f)
{
	LARGE_INTEGER li2;
	Message_ReplayCounter_OC2LI(f, &li2);
	if (li1.field.HighPart == li2.field.HighPart && li1.field.LowPart == li2.field.LowPart)
		return 1;
	else
		return 0;
}
#endif

#ifdef CLIENT_MODE
/*-------------------------------------------------------------------------------------------
	li1 is recorded replay counter on STA
	f is the replay counter from EAPOL-KEY message
---------------------------------------------------------------------------------------------*/
static int Message_SmallerEqualReplayCounter(LARGE_INTEGER li1, OCTET_STRING f)
{
	LARGE_INTEGER li2;
	Message_ReplayCounter_OC2LI(f, &li2);
	if (li2.field.HighPart > li1.field.HighPart)
		return 0;
	else if (li2.field.HighPart < li1.field.HighPart)
		return 1;
	else if (li2.field.LowPart > li1.field.LowPart)
		return 0;
	else if (li2.field.LowPart <= li1.field.LowPart)
		return 1;
	else
		return 0;
}
#endif

/*---------------------------------------------------------------------------------------------
	li1 is recorded replay counter on STA
	f is the replay counter from EAPOL-KEY message
-----------------------------------------------------------------------------------------------*/
static int Message_LargerReplayCounter(LARGE_INTEGER li1, OCTET_STRING f)
{
	LARGE_INTEGER li2;
	Message_ReplayCounter_OC2LI(f, &li2);

	if (li2.field.HighPart > li1.field.HighPart)
		return 1;
	else if (li2.field.LowPart > li1.field.LowPart)
		return 1;
	else
		return 0;
}

static void Message_setReplayCounter(OCTET_STRING f, unsigned int h, unsigned int l)
{
	LARGE_INTEGER *li = (LARGE_INTEGER *)(f.Octet + ReplayCounterPos);
	li->charData[0] = (unsigned char)(h >> 24) & 0xff;
	li->charData[1] = (unsigned char)(h >> 16) & 0xff;
	li->charData[2] = (unsigned char)(h >>  8) & 0xff;
	li->charData[3] = (unsigned char)(h >>  0) & 0xff;
	li->charData[4] = (unsigned char)(l >> 24) & 0xff;
	li->charData[5] = (unsigned char)(l >> 16) & 0xff;
	li->charData[6] = (unsigned char)(l >>  8) & 0xff;
	li->charData[7] = (unsigned char)(l >>  0) & 0xff;
}

void ConstructIE(struct rtl8192cd_priv *priv, unsigned char *pucOut, int *usOutLen)
{
	DOT11_RSN_IE_HEADER dot11RSNIEHeader = { 0 };
	DOT11_RSN_IE_SUITE dot11RSNGroupSuite;
#ifdef CONFIG_IEEE80211W
	DOT11_RSN_IE_SUITE dot11RSNIGTKSuite;
#endif
	DOT11_RSN_IE_COUNT_SUITE *pDot11RSNPairwiseSuite = NULL;
	DOT11_RSN_IE_COUNT_SUITE *pDot11RSNAuthSuite = NULL;
	unsigned short usSuitCount;
	unsigned int ulIELength = 0;
	unsigned int ulIndex = 0;
	unsigned int ulPairwiseLength = 0;
	unsigned int ulAuthLength = 0;
	unsigned char *pucBlob;
	DOT11_RSN_IE_COUNT_SUITE countSuite, authCountSuite;
#ifdef RTL_WPA2
	DOT11_RSN_CAPABILITY dot11RSNCapability = { 0 };
	unsigned int uCipherAlgo = 0;
	int bCipherAlgoEnabled = FALSE;
	unsigned int uAuthAlgo = 0;
	int bAuthAlgoEnabled = FALSE;
	unsigned int ulRSNCapabilityLength = 0;
#endif
#if 0
	unsigned char prefer_auth = 0;

	if (OPMODE & WIFI_STATION_STATE)
	{
		if ( priv->pmib->dot1180211AuthEntry.dot11EnablePSK == (PSK_WPA | PSK_WPA2) &&
			memcmp(priv->pmib->dot11Bss.bssid, NULL_MAC_ADDR, MACADDRLEN))
		{
			for (ulIndex=0; ulIndex<priv->wpa_global_info->NumOfUnicastCipherWPA2; ulIndex++)
			{
				if (priv->pmib->dot11Bss.t_stamp[0] & (BIT(priv->wpa_global_info->UnicastCipherWPA2[ulIndex]) << 22))
				{
					prefer_auth = PSK_WPA2;
					break;
				}
			}
			if (!prefer_auth)
			{
				for (ulIndex=0; ulIndex<priv->wpa_global_info->NumOfUnicastCipher; ulIndex++)
				{
					if (priv->pmib->dot11Bss.t_stamp[0] & (BIT(priv->wpa_global_info->UnicastCipher[ulIndex]) << 6))
					{
						prefer_auth = PSK_WPA;
						break;
					}
				}
			}
		}
		else
			prefer_auth = priv->pmib->dot1180211AuthEntry.dot11EnablePSK;
		priv->wpa_global_info->ClntAuthType = prefer_auth;
	}
#endif

	//	Stanley
	//	According to Bss_desc, dynamic adjust mib(psk_enable, wpa_cipher, wpa2_cipher)
	*usOutLen = 0;
	if ( priv->pmib->dot1180211AuthEntry.dot11EnablePSK & PSK_WPA
//#ifdef SUPPORT_CLIENT_MIXED_SECURITY
//		&& ((OPMODE & WIFI_STATION_STATE)==0 ||
//			((OPMODE & WIFI_STATION_STATE) && (prefer_auth & PSK_WPA)))
//#endif
	) {
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
                //printk("[WPA] MulticastSuite = %d, ", dot11RSNGroupSuite.Type);
		ulIELength += sizeof(DOT11_RSN_IE_SUITE);

		// - UnicastSuite
		pDot11RSNPairwiseSuite = &countSuite;
		memset(pDot11RSNPairwiseSuite, 0, sizeof(DOT11_RSN_IE_COUNT_SUITE));
		usSuitCount = 0;
		{
		for (ulIndex = 0; ulIndex < priv->wpa_global_info->NumOfUnicastCipher; ulIndex++)
                {
			pDot11RSNPairwiseSuite->dot11RSNIESuite[usSuitCount].OUI[0] = 0x00;
			pDot11RSNPairwiseSuite->dot11RSNIESuite[usSuitCount].OUI[1] = 0x50;
			pDot11RSNPairwiseSuite->dot11RSNIESuite[usSuitCount].OUI[2] = 0xF2;
			pDot11RSNPairwiseSuite->dot11RSNIESuite[usSuitCount].Type = priv->wpa_global_info->UnicastCipher[ulIndex];
			usSuitCount++;
		}
                }
		pDot11RSNPairwiseSuite->SuiteCount = cpu_to_le16(usSuitCount);
		ulPairwiseLength = sizeof(pDot11RSNPairwiseSuite->SuiteCount) + usSuitCount * sizeof(DOT11_RSN_IE_SUITE);
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
		ulAuthLength = sizeof(pDot11RSNAuthSuite->SuiteCount) + usSuitCount * sizeof(DOT11_RSN_IE_SUITE);
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
	if ( priv->pmib->dot1180211AuthEntry.dot11EnablePSK & PSK_WPA2
//#ifdef SUPPORT_CLIENT_MIXED_SECURITY
//		&& ((OPMODE & WIFI_STATION_STATE)==0 ||
//			((OPMODE & WIFI_STATION_STATE) && (prefer_auth & PSK_WPA2)))
//#endif
	) {
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
		dot11RSNGroupSuite.Type = priv->wpa_global_info->MulticastCipher;
                //printk("[WPA2] MulticastSuite = %d, ", dot11RSNGroupSuite.Type);
		ulIELength += sizeof(DOT11_RSN_IE_SUITE);

		//      - UnicastSuite
		pDot11RSNPairwiseSuite = &countSuite;
		memset(pDot11RSNPairwiseSuite, 0, sizeof(DOT11_RSN_IE_COUNT_SUITE));
		usSuitCount = 0;
		{
		for (ulIndex = 0; ulIndex < priv->wpa_global_info->NumOfUnicastCipherWPA2; ulIndex++) {
			pDot11RSNPairwiseSuite->dot11RSNIESuite[usSuitCount].OUI[0] = 0x00;
			pDot11RSNPairwiseSuite->dot11RSNIESuite[usSuitCount].OUI[1] = 0x0F;
			pDot11RSNPairwiseSuite->dot11RSNIESuite[usSuitCount].OUI[2] = 0xAC;
			pDot11RSNPairwiseSuite->dot11RSNIESuite[usSuitCount].Type = priv->wpa_global_info->UnicastCipherWPA2[ulIndex];
			usSuitCount++;
		}
                }
		pDot11RSNPairwiseSuite->SuiteCount = cpu_to_le16(usSuitCount);
		ulPairwiseLength = sizeof(pDot11RSNPairwiseSuite->SuiteCount) + usSuitCount * sizeof(DOT11_RSN_IE_SUITE);
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


#ifdef CONFIG_IEEE80211W_CLI
	if(OPMODE & WIFI_STATION_STATE) {

		if ((priv->pmib->dot1180211AuthEntry.dot11IEEE80211W == MGMT_FRAME_PROTECTION_REQUIRED)
			|| (priv->bss_support_pmf == MGMT_FRAME_PROTECTION_REQUIRED)) {
			pDot11RSNAuthSuite->dot11RSNIESuite[usSuitCount].Type = DOT11_AuthKeyType_PSK_SHA256;
			usSuitCount++;
		}
		else {

			if(priv->bss_support_akmp & BIT(DOT11_AuthKeyType_PSK_SHA256)){
				pDot11RSNAuthSuite->dot11RSNIESuite[usSuitCount].Type = DOT11_AuthKeyType_PSK_SHA256;
				priv->pmib->dot1180211AuthEntry.dot11EnableSHA256 = 1;
				usSuitCount++;
			}
			else { //if((priv->bss_support_akmp & BIT(DOT11_AuthKeyType_PSK)) || (priv->bss_support_akmp == 0)) {
				pDot11RSNAuthSuite->dot11RSNIESuite[usSuitCount].OUI[0] = 0x00;
				pDot11RSNAuthSuite->dot11RSNIESuite[usSuitCount].OUI[1] = 0x0F;
				pDot11RSNAuthSuite->dot11RSNIESuite[usSuitCount].OUI[2] = 0xAC;
				pDot11RSNAuthSuite->dot11RSNIESuite[usSuitCount].Type = DOT11_AuthKeyType_PSK;
				usSuitCount++;
			}
		}


	}
	else
#endif
	{
#ifdef CONFIG_IEEE80211W
		if (priv->pmib->dot1180211AuthEntry.dot11IEEE80211W == MGMT_FRAME_PROTECTION_REQUIRED)
			pDot11RSNAuthSuite->dot11RSNIESuite[usSuitCount].Type = DOT11_AuthKeyType_PSK_SHA256;
		else
#endif
			pDot11RSNAuthSuite->dot11RSNIESuite[usSuitCount].Type = DOT11_AuthKeyType_PSK;

		usSuitCount++;

#ifdef CONFIG_IEEE80211W
		if (priv->pmib->dot1180211AuthEntry.dot11IEEE80211W == MGMT_FRAME_PROTECTION_OPTIONAL
				&& priv->pmib->dot1180211AuthEntry.dot11EnableSHA256 == 1) {
			pDot11RSNAuthSuite->dot11RSNIESuite[usSuitCount].OUI[0] = 0x00;
			pDot11RSNAuthSuite->dot11RSNIESuite[usSuitCount].OUI[1] = 0x0F;
			pDot11RSNAuthSuite->dot11RSNIESuite[usSuitCount].OUI[2] = 0xAC;
			pDot11RSNAuthSuite->dot11RSNIESuite[usSuitCount].Type = DOT11_AuthKeyType_PSK_SHA256;
			usSuitCount++;
		}
#endif
	}



		pDot11RSNAuthSuite->SuiteCount = cpu_to_le16(usSuitCount);
		ulAuthLength = sizeof(pDot11RSNAuthSuite->SuiteCount) + usSuitCount * sizeof(DOT11_RSN_IE_SUITE);
		ulIELength += ulAuthLength;

		//---------------------------------------------------------------------------------------------
		// Do not encapsulate capability field to solve TI WPA issue
		//---------------------------------------------------------------------------------------------

//#ifdef RTL_WPA2
#if 1
		dot11RSNCapability.field.PreAuthentication = 0;
#else
		dot11RSNCapability.field.PairwiseAsDefaultKey = auth->RSNVariable.isSupportPairwiseAsDefaultKey;
		switch (auth->RSNVariable.NumOfRxTSC) {
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

#ifdef CONFIG_IEEE80211W
		//Protected Managemenet Protection Capability (PMF)
		//printk("ConstructIE, dot11IEEE80211W=%d\n", priv->pmib->dot1180211AuthEntry.dot11IEEE80211W);
		if (priv->pmib->dot1180211AuthEntry.dot11IEEE80211W == NO_MGMT_FRAME_PROTECTION) {
			dot11RSNCapability.field.MFPC = 0;
			dot11RSNCapability.field.MFPR = 0;
		} else if (priv->pmib->dot1180211AuthEntry.dot11IEEE80211W == MGMT_FRAME_PROTECTION_OPTIONAL)
			dot11RSNCapability.field.MFPC = 1; // MFPC
		else if (priv->pmib->dot1180211AuthEntry.dot11IEEE80211W == MGMT_FRAME_PROTECTION_REQUIRED) {
			dot11RSNCapability.field.MFPR = 1; // MFPR
			dot11RSNCapability.field.MFPC = 1; // MFPC
		}
#endif

		ulRSNCapabilityLength = sizeof(DOT11_RSN_CAPABILITY);
		ulIELength += ulRSNCapabilityLength;

#ifdef CONFIG_IEEE80211W
		// Construct Cipher Suite:
		//		- IGTK Suite:
		//

		if (OPMODE & WIFI_AP_STATE)
		if ((priv->pmib->dot1180211AuthEntry.dot11IEEE80211W == 1 && priv->pmib->dot1180211AuthEntry.dot11EnableSHA256 == TRUE)  ||
		(priv->pmib->dot1180211AuthEntry.dot11IEEE80211W == 2 && priv->pmib->dot1180211AuthEntry.dot11EnableSHA256 == TRUE)) {
			memset(&dot11RSNIGTKSuite, 0, sizeof(dot11RSNIGTKSuite));
			dot11RSNIGTKSuite.OUI[0] = 0x00;
			dot11RSNIGTKSuite.OUI[1] = 0x0F;
			dot11RSNIGTKSuite.OUI[2] = 0xAC;

			dot11RSNIGTKSuite.Type = DOT11_ENC_BIP;
			ulIELength += sizeof(dot11RSNIGTKSuite) + 2;
		}
#endif // CONFIG_IEEE80211W

		pucBlob = pucOut + *usOutLen;
		pucBlob += sizeof(DOT11_WPA2_IE_HEADER);
		memcpy(pucBlob, &dot11RSNGroupSuite, sizeof(DOT11_RSN_IE_SUITE));
		pucBlob += sizeof(DOT11_RSN_IE_SUITE);
		memcpy(pucBlob, pDot11RSNPairwiseSuite, ulPairwiseLength);
		pucBlob += ulPairwiseLength;
		memcpy(pucBlob, pDot11RSNAuthSuite, ulAuthLength);
		pucBlob += ulAuthLength;
		memcpy(pucBlob, &dot11RSNCapability, ulRSNCapabilityLength);

#ifdef CONFIG_IEEE80211W
		if (OPMODE & WIFI_AP_STATE)
		if (priv->pmib->dot1180211AuthEntry.dot11IEEE80211W != NO_MGMT_FRAME_PROTECTION) {
			pucBlob += ulRSNCapabilityLength + 2; // add PMDID Count (2bytes)
			memcpy(pucBlob, &dot11RSNIGTKSuite, sizeof(DOT11_RSN_IE_SUITE));
		}
#endif // CONFIG_IEEE80211W

		pucBlob = pucOut + *usOutLen;
		dot11WPA2IEHeader.Length = (unsigned char)ulIELength - 2; //This -2 is to minus elementID and Length in OUI header
		memcpy(pucBlob, &dot11WPA2IEHeader, sizeof(DOT11_WPA2_IE_HEADER));
		*usOutLen = *usOutLen + (int)ulIELength;
	}
#endif // RTL_WPA2

}

static void INCLargeInteger(LARGE_INTEGER * x)
{
	if (x->field.LowPart == 0xffffffff) {
		if (x->field.HighPart == 0xffffffff) {
			x->field.HighPart = 0;
			x->field.LowPart = 0;
		} else {
			x->field.HighPart++;
			x->field.LowPart = 0;
		}
	} else
		x->field.LowPart++;
}

static void INCOctet16_INTEGER(OCTET16_INTEGER * x)
{
	if (LargeIntegerOverflow(x->field.LowPart)) {
		if (LargeIntegerOverflow(x->field.HighPart)) {
			LargeIntegerZero(x->field.HighPart);
			LargeIntegerZero(x->field.LowPart);
		} else {
			INCLargeInteger(&x->field.HighPart);
			LargeIntegerZero(x->field.LowPart);
		}
	} else
		INCLargeInteger(&x->field.LowPart);

}

static OCTET32_INTEGER *INCOctet32_INTEGER(OCTET32_INTEGER * x)
{
	if (Octet16IntegerOverflow(x->field.LowPart)) {
		if (Octet16IntegerOverflow(x->field.HighPart)) {
			Octet16IntegerZero(x->field.HighPart);
			Octet16IntegerZero(x->field.LowPart);
		} else {
			INCOctet16_INTEGER(&x->field.HighPart);
			Octet16IntegerZero( x->field.LowPart);
		}
	} else
		INCOctet16_INTEGER(&x->field.LowPart);
	return x;
}

static void SetEAPOL_KEYIV(OCTET_STRING ocDst, OCTET32_INTEGER oc32Counter)
{
	unsigned char *ptr = ocDst.Octet;
	unsigned int ulTmp;

	ulTmp = cpu_to_le32(oc32Counter.field.HighPart.field.HighPart.field.HighPart);
	memcpy(ptr, (char *)&ulTmp, sizeof(ulTmp));
	ptr += 4;

	ulTmp = cpu_to_le32(oc32Counter.field.HighPart.field.HighPart.field.LowPart);
	memcpy(ptr, (char *)&ulTmp, sizeof(ulTmp));
	ptr += 4;

	ulTmp = cpu_to_le32(oc32Counter.field.HighPart.field.LowPart.field.HighPart);
	memcpy(ptr, (char *)&ulTmp, sizeof(ulTmp));
	ptr += 4;

	ulTmp = cpu_to_le32(oc32Counter.field.HighPart.field.LowPart.field.LowPart);
	memcpy(ptr, (char *)&ulTmp, sizeof(ulTmp));

	if (ocDst.Length == 16) // for AES
		return;

	ptr += 4;

	ulTmp = cpu_to_le32(oc32Counter.field.LowPart.field.HighPart.field.HighPart);
	memcpy(ptr, (char *)&ulTmp, sizeof(ulTmp));
	ptr += 4;

	ulTmp = cpu_to_le32(oc32Counter.field.LowPart.field.HighPart.field.LowPart);
	memcpy(ptr, (char *)&ulTmp, sizeof(ulTmp));
	ptr += 4;

	ulTmp = cpu_to_le32(oc32Counter.field.LowPart.field.LowPart.field.HighPart);
	memcpy(ptr, (char *)&ulTmp, sizeof(ulTmp));
	ptr += 4;

	ulTmp = cpu_to_le32(oc32Counter.field.LowPart.field.LowPart.field.LowPart);
	memcpy(ptr, (char *)&ulTmp, sizeof(ulTmp));
}

static void EncGTK(struct rtl8192cd_priv *priv, struct stat_info *pstat,
				   unsigned char *kek, int keklen, unsigned char *key,
				   int keylen, unsigned char *out, unsigned short *outlen)
{

	unsigned char tmp1[257], tmp2[257];
	RC4_KEY	 *rc4key;
#ifdef RTL_WPA2
	unsigned char default_key_iv[] = { 0xA6, 0xA6, 0xA6, 0xA6, 0xA6, 0xA6, 0xA6, 0xA6 };
#endif
	OCTET_STRING EAPOLMsgSend;
	lib1x_eapol_key *eapolkey;

	rc4key = (RC4_KEY *)kmalloc(sizeof(RC4_KEY), GFP_ATOMIC);
	if (rc4key == NULL)
		return;

	EAPOLMsgSend.Octet = pstat->wpa_sta_info->EAPOLMsgSend.Octet;
	eapolkey = (lib1x_eapol_key *)(EAPOLMsgSend.Octet + ETHER_HDRLEN + LIB1X_EAPOL_HDRLEN);

// should refer tx packet, david+2006-04-06
//	if (Message_KeyDescVer(pstat->wpa_sta_info->EapolKeyMsgRecvd) == key_desc_ver1) {
	if (Message_KeyDescVer(pstat->wpa_sta_info->EapolKeyMsgSend) == key_desc_ver1) {

		memcpy(tmp1, eapolkey->key_iv, KEY_IV_LEN);
		memcpy(tmp1 + KEY_IV_LEN, kek, keklen);

		RC4_set_key(rc4key, KEY_IV_LEN + keklen, tmp1);

		//first 256 bytes are discarded
		RC4(rc4key, 256, (unsigned char *)tmp1, (unsigned char *)tmp2);
		RC4(rc4key, keylen, (unsigned char *)key, out);
		*outlen = keylen;
	}
#ifdef RTL_WPA2
	else
		//according to p75 of 11i/D3.0, the IV should be put in the least significant octecs of
		//KeyIV field which shall be padded with 0, so eapolkey->key_iv + 8
		AES_WRAP(key, keylen, default_key_iv, 8, kek, keklen, out, outlen);
#endif
	kfree(rc4key);
}

static int CheckMIC(OCTET_STRING EAPOLMsgRecvd, unsigned char *key, int keylen)
{
	int retVal = 0;
	OCTET_STRING EapolKeyMsgRecvd;
	unsigned char ucAlgo;
	OCTET_STRING tmp; //copy of overall 802.1x message
	unsigned char tmpbuf[512];
	struct lib1x_eapol *tmpeapol;
	lib1x_eapol_key *tmpeapolkey;
	unsigned char sha1digest[20];

	EapolKeyMsgRecvd.Octet = EAPOLMsgRecvd.Octet +
							 ETHER_HDRLEN + LIB1X_EAPOL_HDRLEN;
	EapolKeyMsgRecvd.Length = EAPOLMsgRecvd.Length -
							  (ETHER_HDRLEN + LIB1X_EAPOL_HDRLEN);
	ucAlgo = Message_KeyDescVer(EapolKeyMsgRecvd);

	tmp.Length = EAPOLMsgRecvd.Length;
	tmp.Octet = tmpbuf;
	memcpy(tmp.Octet, EAPOLMsgRecvd.Octet, EAPOLMsgRecvd.Length);
	tmpeapol = (struct lib1x_eapol *)(tmp.Octet + ETHER_HDRLEN);
	tmpeapolkey = (lib1x_eapol_key *)(tmp.Octet + ETHER_HDRLEN + LIB1X_EAPOL_HDRLEN);
	memset(tmpeapolkey->key_mic, 0, KEY_MIC_LEN);

	if (ucAlgo == key_desc_ver1) {
		hmac_md5((unsigned char*)tmpeapol, LIB1X_EAPOL_HDRLEN + ntohs(tmpeapol->packet_body_length) ,
				 key, keylen, tmpeapolkey->key_mic);
#if 0
		lib1x_hexdump2(MESS_DBG_KEY_MANAGE, "CheckMIC", EapolKeyMsgRecvd.Octet +
					   KeyMICPos, KEY_MIC_LEN, "Original");
		lib1x_hexdump2(MESS_DBG_KEY_MANAGE, "CheckMIC", tmpeapolkey->key_mic,
					   KEY_MIC_LEN, "Calculated");
#endif
		if (!memcmp(tmpeapolkey->key_mic, EapolKeyMsgRecvd.Octet + KeyMICPos, KEY_MIC_LEN))
			retVal = 1;
	} else if (ucAlgo == key_desc_ver2) {
		hmac_sha1((unsigned char*)tmpeapol, LIB1X_EAPOL_HDRLEN + ntohs(tmpeapol->packet_body_length) ,
					key, keylen, sha1digest);
		if (!memcmp(sha1digest, EapolKeyMsgRecvd.Octet + KeyMICPos, KEY_MIC_LEN))
			retVal = 1;
	}
#if defined(CONFIG_IEEE80211W) || defined(CONFIG_IEEE80211R)
	else if (ucAlgo == key_desc_ver3
        #ifdef HS2_SUPPORT     // OSEN
        || ucAlgo == 0
        #endif
        )
    {

		omac1_aes_128(key, (unsigned char*)tmpeapol, LIB1X_EAPOL_HDRLEN + ntohs(tmpeapol->packet_body_length) , tmpeapolkey->key_mic);

		if (!memcmp(tmpeapolkey->key_mic, EapolKeyMsgRecvd.Octet + KeyMICPos, KEY_MIC_LEN))
			retVal = 1;
	}
#endif
	return retVal;
}

static void CalcMIC(OCTET_STRING EAPOLMsgSend, int algo, unsigned char *key, int keylen)
{
	struct lib1x_eapol *eapol = (struct lib1x_eapol *)(EAPOLMsgSend.Octet + ETHER_HDRLEN);
	lib1x_eapol_key *eapolkey = (lib1x_eapol_key *)(EAPOLMsgSend.Octet + ETHER_HDRLEN + LIB1X_EAPOL_HDRLEN);
	unsigned char sha1digest[20];

	memset(eapolkey->key_mic, 0, KEY_MIC_LEN);

	if (algo == key_desc_ver1)
		hmac_md5((unsigned char*)eapol, EAPOLMsgSend.Length - ETHER_HDRLEN ,
				 key, keylen, eapolkey->key_mic);
	else if (algo == key_desc_ver2) {
		hmac_sha1((unsigned char*)eapol, EAPOLMsgSend.Length - ETHER_HDRLEN ,
				  key, keylen, sha1digest);
		memcpy(eapolkey->key_mic, sha1digest, KEY_MIC_LEN);
	}
#if defined(CONFIG_IEEE80211W) || defined(CONFIG_IEEE80211R)
	else if (algo == key_desc_ver3) {
		omac1_aes_128(key, (unsigned char*)eapol, EAPOLMsgSend.Length - ETHER_HDRLEN, eapolkey->key_mic);
	}
#endif
}

/* GTK-PRF-X
   X = 256 in TKIP
   X = 128 in CCMP, WRAP, and WEP
*/
static void CalcGTK(unsigned char *addr, unsigned char *nonce,
					unsigned char *keyin, int keyinlen,
					unsigned char *keyout, int keyoutlen, unsigned char *label)
{
	unsigned char data[ETHER_ADDRLEN + KEY_NONCE_LEN], tmp[64];

	memcpy(data, addr, ETHER_ADDRLEN);
	memcpy(data + ETHER_ADDRLEN, nonce, KEY_NONCE_LEN);
#ifdef CONFIG_IEEE80211W
	sha256_prf(keyin, keyinlen, label, data, sizeof(data), tmp, keyoutlen);
#else
	i_PRF(keyin, keyinlen, label,
		  GMK_EXPANSION_CONST_SIZE, data, sizeof(data),
		  tmp, keyoutlen);
#endif
	memcpy(keyout, tmp, keyoutlen);
}

static int MIN(unsigned char *ucStr1, unsigned char *ucStr2, unsigned int ulLen)
{
	int i;
	for (i = 0 ; i < ulLen ; i++) {
		if ((unsigned char)ucStr1[i] < (unsigned char)ucStr2[i])
			return -1;
		else if ((unsigned char)ucStr1[i] > (unsigned char)ucStr2[i])
			return 1;
		else if (i == ulLen - 1)
			return 0;
		else
			continue;
	}
	return 0;
}

static void CalcPTK(unsigned char *addr1, unsigned char *addr2,
					unsigned char *nonce1, unsigned char *nonce2,
					unsigned char * keyin, int keyinlen,
					unsigned char *keyout, int keyoutlen
#ifdef CONFIG_IEEE80211W
					, int use_sha256
#endif
				   )
{
	unsigned char data[2 * ETHER_ADDRLEN + 2 * KEY_NONCE_LEN], tmpPTK[128];
#ifdef CONFIG_IEEE80211W
	unsigned char keydata[32]={0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0a,0x0b,0x0c,0x0d,0x0e,0x0f,0x10,
    				0x11,0x12,0x13,0x14,0x15,0x16,0x17,0x18,0x19,0x1a,0x1b,0x1c,0x1d,0x1e,0x1f,0x20 };
	unsigned char plainData[10]="abc";
	unsigned char **ptr;
	unsigned char encryptedData[32];
#endif

	if (MIN(addr1, addr2, ETHER_ADDRLEN) <= 0) {
		memcpy(data, addr1, ETHER_ADDRLEN);
		memcpy(data + ETHER_ADDRLEN, addr2, ETHER_ADDRLEN);
	} else {
		memcpy(data, addr2, ETHER_ADDRLEN);
		memcpy(data + ETHER_ADDRLEN, addr1, ETHER_ADDRLEN);
	}
	if (MIN(nonce1, nonce2, KEY_NONCE_LEN) <= 0) {
		memcpy(data + 2 * ETHER_ADDRLEN, nonce1, KEY_NONCE_LEN);
		memcpy(data + 2 * ETHER_ADDRLEN + KEY_NONCE_LEN, nonce2, KEY_NONCE_LEN);
	} else {
		memcpy(data + 2 * ETHER_ADDRLEN, nonce2, KEY_NONCE_LEN);
		memcpy(data + 2 * ETHER_ADDRLEN + KEY_NONCE_LEN, nonce1, KEY_NONCE_LEN);
	}
#ifdef CONFIG_IEEE80211W
	if (use_sha256) {
		//PMFDEBUG("sha256_prf\n");
		sha256_prf(keyin, keyinlen, (unsigned char*)PMK_EXPANSION_CONST, data, sizeof(data),
				   tmpPTK, keyoutlen);
	} else
#endif
	{

		i_PRF(keyin, keyinlen, (unsigned char*)PMK_EXPANSION_CONST,
			  PMK_EXPANSION_CONST_SIZE, data, sizeof(data),
			  tmpPTK, PTK_LEN_TKIP);
	}
	memcpy(keyout, tmpPTK, keyoutlen);
}


#ifdef CLIENT_MODE
/*
	decrypt WPA2 Message 3's Key Data
*/
// Use RC4 or AES to decode the keydata by checking desc-ver, david-2006-01-06
//int DecWPA2KeyData(u_char *key, int keylen, u_char *kek, int keklen, u_char *kout)
int DecWPA2KeyData(WPA_STA_INFO* pStaInfo, unsigned char *key, int keylen, unsigned char *kek, int keklen, unsigned char *kout)
{
	int	retVal = 0;
	unsigned char		default_key_iv[] = { 0xA6, 0xA6, 0xA6, 0xA6, 0xA6, 0xA6, 0xA6, 0xA6 };
	unsigned char		tmp2[257];

// Use RC4 or AES to decode the keydata by checking desc-ver, david-2006-01-06
	unsigned char 	tmp1[257];
	RC4_KEY			*rc4key;

	lib1x_eapol_key *eapolkey = (lib1x_eapol_key *)(pStaInfo->EAPOLMsgRecvd.Octet + ETHER_HDRLEN + LIB1X_EAPOL_HDRLEN);

	rc4key = (RC4_KEY *)kmalloc(sizeof(RC4_KEY), GFP_ATOMIC);
	if (rc4key == NULL)
		return 0;

	if (Message_KeyDescVer(pStaInfo->EapolKeyMsgRecvd) == key_desc_ver1) {
		memcpy(tmp1, eapolkey->key_iv, KEY_IV_LEN);
		memcpy(tmp1 + KEY_IV_LEN, kek, keklen);
		RC4_set_key(rc4key, KEY_IV_LEN + keklen, tmp1);

		//first 256 bits is discard
		RC4(rc4key, 256, (unsigned char*)tmp1, (unsigned char*)tmp2);

		//RC4(rc4key, keylen, eapol_key->key_data, global->skm_sm->GTK[Message_KeyIndex(global->EapolKeyMsgRecvd)]);
		RC4(rc4key, keylen, pStaInfo->EapolKeyMsgRecvd.Octet + KeyDataPos, (unsigned char*)tmp2);

		memcpy(kout, tmp2, keylen);
		//memcpy(&global->supp_kmsm->GTK[Message_KeyIndex(global->EapolKeyMsgRecvd)], tmp2, keylen);
		retVal = 1;
	} else {
//--------------------------------------------------------
		AES_UnWRAP(key, keylen, kek, keklen, tmp2);
		if (memcmp(tmp2, default_key_iv, 8))
			retVal = 0;
		else {
			memcpy(kout, tmp2 + 8, keylen);
			retVal = 1;
		}
	}
	kfree(rc4key);
	return retVal;
}

int DecGTK(OCTET_STRING EAPOLMsgRecvd, unsigned char *kek, int keklen, int keylen, unsigned char *kout)
{
	int		retVal = 0;
	unsigned char		tmp1[257], tmp2[257];
	RC4_KEY		*rc4key;
	lib1x_eapol_key * eapol_key = (lib1x_eapol_key *)(EAPOLMsgRecvd.Octet + ETHER_HDRLEN + LIB1X_EAPOL_HDRLEN);
	OCTET_STRING	EapolKeyMsgRecvd;

	rc4key = (RC4_KEY *)kmalloc(sizeof(RC4_KEY), GFP_ATOMIC);
	if (rc4key == NULL)
		return 0;

	EapolKeyMsgRecvd.Octet = EAPOLMsgRecvd.Octet + ETHER_HDRLEN + LIB1X_EAPOL_HDRLEN;
	EapolKeyMsgRecvd.Length = EAPOLMsgRecvd.Length - (ETHER_HDRLEN + LIB1X_EAPOL_HDRLEN);

	if (Message_KeyDescVer(EapolKeyMsgRecvd) == key_desc_ver1) {
		memcpy(tmp1, eapol_key->key_iv, KEY_IV_LEN);
		memcpy(tmp1 + KEY_IV_LEN, kek, keklen);
		RC4_set_key(rc4key, KEY_IV_LEN + keklen, tmp1);
		//first 256 bits is discard
		RC4(rc4key, 256, (unsigned char*)tmp1, (unsigned char*)tmp2);
		//RC4(rc4key, keylen, eapol_key->key_data, global->skm_sm->GTK[Message_KeyIndex(global->EapolKeyMsgRecvd)]);
		RC4(rc4key, keylen, EapolKeyMsgRecvd.Octet + KeyDataPos, (unsigned char*)tmp2);
		memcpy(kout, tmp2, keylen);
		//memcpy(&global->supp_kmsm->GTK[Message_KeyIndex(global->EapolKeyMsgRecvd)], tmp2, keylen);
		retVal = 1;
	} else if (Message_KeyDescVer(EapolKeyMsgRecvd) == key_desc_ver2) {
		// kenny: should use default IV 0xA6A6A6A6A6A6A6A6
		unsigned char	default_key_iv[] = { 0xA6, 0xA6, 0xA6, 0xA6, 0xA6, 0xA6, 0xA6, 0xA6 };
// david, get key len from eapol packet
//			AES_UnWRAP(EapolKeyMsgRecvd.Octet + KeyDataPos, keylen + 8, kek, keklen, tmp2);

		keylen = Message_KeyDataLength(EapolKeyMsgRecvd);
		AES_UnWRAP(EapolKeyMsgRecvd.Octet + KeyDataPos, keylen, kek, keklen, tmp2);
//------------------------- 2005-08-01

		//if(memcmp(tmp2, eapol_key->key_iv + 8, 8))
		if (memcmp(tmp2, default_key_iv, 8))
			retVal = 0;
		else {
			//memcpy(kout, tmp2, keylen);
			//memcpy(global->supp_kmsm->GTK[Message_KeyIndex(global->EapolKeyMsgRecvd)], tmp2 + 8, keylen - 8);
			memcpy(kout, tmp2 + 8, keylen);
			retVal = 1;
		}
	}
	kfree(rc4key);
	return retVal;
}
#endif /* CLIENT_MODE */

static int parseIE(struct rtl8192cd_priv *priv, WPA_STA_INFO *pInfo,
				   unsigned char *pucIE, unsigned int ulIELength)
{
	unsigned short usSuitCount;
	DOT11_RSN_IE_HEADER *pDot11RSNIEHeader;
	DOT11_RSN_IE_SUITE *pDot11RSNIESuite;
	DOT11_RSN_IE_COUNT_SUITE *pDot11RSNIECountSuite;

	DOT11_RSN_CAPABILITY * pDot11RSNCapability = NULL;
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
			pDot11RSNIEHeader->Length != ulIELength - 2 ||
			pDot11RSNIEHeader->OUI[0] != 0x00 || pDot11RSNIEHeader->OUI[1] != 0x50 ||
			pDot11RSNIEHeader->OUI[2] != 0xf2 || pDot11RSNIEHeader->OUI[3] != 0x01 ) {
		DEBUG_WARN("parseIE err 3!\n");
		return ERROR_INVALID_RSNIE;
	}

	pInfo->RSNEnabled = PSK_WPA;	// wpa
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

#ifdef CONFIG_IEEE80211W
	if (priv->pmib->dot1180211AuthEntry.dot11IEEE80211W == MGMT_FRAME_PROTECTION_REQUIRED) {
		if (pDot11RSNIESuite->Type != DOT11_ENC_CCMP) {
			PMFDEBUG("Invalid WPA group cipher %d\n", pDot11RSNIESuite->Type);
			return ERROR_MGMT_FRAME_PROTECTION_VIOLATION;
		}
	}
#endif


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

#ifdef CONFIG_IEEE80211W
	if (priv->pmib->dot1180211AuthEntry.dot11IEEE80211W == MGMT_FRAME_PROTECTION_REQUIRED) {
		if (pInfo->UnicastCipher == DOT11_ENC_TKIP) {
			PMFDEBUG("Management frame protection cannot use TKIP\n");
			return ERROR_MGMT_FRAME_PROTECTION_VIOLATION;
		}
	}
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

	if ( pDot11RSNIESuite->Type < DOT11_AuthKeyType_RSN ||
			pDot11RSNIESuite->Type > DOT11_AuthKeyType_PSK) {
		DEBUG_WARN("parseIE err 11!\n");
		return ERROR_INVALID_AUTHKEYMANAGE;
	}

	if (pDot11RSNIESuite->Type != DOT11_AuthKeyType_PSK) {
		DEBUG_WARN("parseIE err 12!\n");
		return ERROR_INVALID_AUTHKEYMANAGE;
	}

	pInfo->AuthKeyMethod = pDot11RSNIESuite->Type;
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

#ifdef CONFIG_IEEE80211W
	pDot11RSNCapability = (DOT11_RSN_CAPABILITY * )pucIE;
	if (priv->pmib->dot1180211AuthEntry.dot11IEEE80211W == MGMT_FRAME_PROTECTION_REQUIRED) {
		if (!pDot11RSNCapability->field.MFPC) {
			PMFDEBUG("Management frame protection Required, but client did not enable it\n");
			return ERROR_MGMT_FRAME_PROTECTION_VIOLATION;
		}
	}

	if (priv->pmib->dot1180211AuthEntry.dot11IEEE80211W == NO_MGMT_FRAME_PROTECTION ||
			!(pDot11RSNCapability->field.MFPC))
		pInfo->mgmt_frame_prot = 0;
	else
		pInfo->mgmt_frame_prot = 1;

	PMFDEBUG("mgmt_frame_prot=%d\n", pInfo->mgmt_frame_prot);

#endif // CONFIG_IEEE80211W


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
			pDot11WPA2IEHeader->Length != ulIELength - 2 ) {
		DEBUG_WARN("ERROR_INVALID_RSNIE, err 3!\n");
		return ERROR_INVALID_RSNIE;
	}

	pInfo->RSNEnabled = PSK_WPA2;
	pInfo->PMKCached = FALSE;

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

#ifdef CONFIG_IEEE80211W
	if (pDot11RSNIESuite->Type > DOT11_ENC_BIP) {
#else
	if (pDot11RSNIESuite->Type > DOT11_ENC_WEP104)	{
#endif
		DEBUG_WARN("ERROR_INVALID_MULTICASTCIPHER, err 5!\n");
		return ERROR_INVALID_MULTICASTCIPHER;
	}

	if (pDot11RSNIESuite->Type != priv->wpa_global_info->MulticastCipher) {
		DEBUG_WARN("ERROR_INVALID_MULTICASTCIPHER, err 6!\n");
		return ERROR_INVALID_MULTICASTCIPHER;
	}
#ifdef CONFIG_IEEE80211W
	if (priv->pmib->dot1180211AuthEntry.dot11IEEE80211W == MGMT_FRAME_PROTECTION_REQUIRED) {
		if (pDot11RSNIESuite->Type != DOT11_ENC_CCMP) {
			DEBUG_WARN("Invalid WPA group cipher %d\n", pDot11RSNIESuite->Type);
			return ERROR_MGMT_FRAME_PROTECTION_VIOLATION;
		}
	}
#endif


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

	if(usSuitCount != 1 ||
		pDot11RSNIESuite->OUI[0] != 0x00 ||
		pDot11RSNIESuite->OUI[1] != 0x0F ||
		pDot11RSNIESuite->OUI[2] != 0xAC ) {
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
#ifdef CONFIG_IEEE80211W
	if (priv->pmib->dot1180211AuthEntry.dot11IEEE80211W == MGMT_FRAME_PROTECTION_REQUIRED)	{
		if (pInfo->UnicastCipher == DOT11_ENC_TKIP) {
			DEBUG_WARN("Management frame protection cannot use TKIP\n");
			return ERROR_MGMT_FRAME_PROTECTION_VIOLATION;
		}
	}
#endif

	ulIELength -= sizeof(pDot11RSNIECountSuite->SuiteCount) + sizeof(DOT11_RSN_IE_SUITE);
	pucIE += sizeof(pDot11RSNIECountSuite->SuiteCount) + sizeof(DOT11_RSN_IE_SUITE);

#ifdef DEBUG_PSK
	printk("PSK: ParseIE -> WPA2 UnicastCipher=%x\n", pInfo->UnicastCipher);
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
#ifdef CONFIG_IEEE80211W
			pDot11RSNIESuite->Type > DOT11_AuthKeyType_PSK_SHA256
#else
			pDot11RSNIESuite->Type > DOT11_AuthKeyType_PSK
#endif
	) {
		DEBUG_WARN("ERROR_INVALID_AUTHKEYMANAGE, err 11!\n");
		return ERROR_INVALID_AUTHKEYMANAGE;
	}

	if (pDot11RSNIESuite->Type != DOT11_AuthKeyType_PSK
#ifdef CONFIG_IEEE80211W
			&& pDot11RSNIESuite->Type != DOT11_AuthKeyType_PSK_SHA256
#endif
	   ) {
		DEBUG_WARN("ERROR_INVALID_AUTHKEYMANAGE, err 12!\n");
		return ERROR_INVALID_AUTHKEYMANAGE;
	}

	pInfo->AuthKeyMethod = pDot11RSNIESuite->Type;
	ulIELength -= sizeof(pDot11RSNIECountSuite->SuiteCount) + sizeof(DOT11_RSN_IE_SUITE);
	pucIE += sizeof(pDot11RSNIECountSuite->SuiteCount) + sizeof(DOT11_RSN_IE_SUITE);

	// RSN Capability
	if (ulIELength < sizeof(DOT11_RSN_CAPABILITY)) {
		pInfo->NumOfRxTSC = 2;
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
	pInfo->NumOfRxTSC = 1;
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


#ifdef CONFIG_IEEE80211W
	pDot11RSNCapability = (DOT11_RSN_CAPABILITY * )pucIE;
	if (priv->pmib->dot1180211AuthEntry.dot11IEEE80211W == MGMT_FRAME_PROTECTION_REQUIRED) {
		if (!pDot11RSNCapability->field.MFPC) {
			printk("Management frame protection Required, but client did not enable it\n");
			return ERROR_MGMT_FRAME_PROTECTION_VIOLATION;
		}
	}

	if (priv->pmib->dot1180211AuthEntry.dot11IEEE80211W == NO_MGMT_FRAME_PROTECTION ||
			!(pDot11RSNCapability->field.MFPC))
		pInfo->mgmt_frame_prot = 0;
	else
		pInfo->mgmt_frame_prot = 1;

	PMFDEBUG("mgmt_frame_prot=%d\n", pInfo->mgmt_frame_prot);

#endif // CONFIG_IEEE80211W


	pucIE += 2;
	ulIELength -= 2;
	// PMKID
	if ((ulIELength < 2 + PMKID_LEN))
		return 0;

	//----------------------------------------------------------------------------------
	// PMKID Count field
	//----------------------------------------------------------------------------------
	usSuitCount = le16_to_cpu(*((unsigned short *)pucIE));

	//printf("PMKID Count = %d\n",usSuitCount);
	pucIE += 2;
	ulIELength -= 2;
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

#ifdef CONFIG_IEEE80211W

	pucIE += PMKID_LEN * usSuitCount;
	printk("usSuitCount=%d, ulIELength=%d\n", usSuitCount, ulIELength);
	ulIELength -= PMKID_LEN * usSuitCount;
	//----------------------------------------------------------------------------------
	// Group Management Cipher field (IGTK)
	//----------------------------------------------------------------------------------
	if ((ulIELength < sizeof(DOT11_RSN_IE_SUITE))) {
		return 0;
	}

	pDot11RSNIESuite = (DOT11_RSN_IE_SUITE*)pucIE;

	if (pDot11RSNIESuite->OUI[0] != 0x00 ||
			pDot11RSNIESuite->OUI[1] != 0x0F ||
			pDot11RSNIESuite->OUI[2] != 0xAC) {
		printk("RSNIE Suite OUI = %02x:%02x:%02x\n", pDot11RSNIESuite->OUI[0], pDot11RSNIESuite->OUI[1], pDot11RSNIESuite->OUI[2]);
		return ERROR_INVALID_RSNIE;
	}
	if (pDot11RSNIESuite->Type != DOT11_ENC_BIP) {
		return ERROR_MGMT_FRAME_PROTECTION_VIOLATION;
	}
#endif // CONFIG_IEEE80211W

	return 0;
}
#endif // RTL_WPA2

static void GenNonce(unsigned char *nonce, unsigned char *addr)
{
	unsigned char secret[256], random[256], result[256];

	get_random_bytes(random, 256);
	memset(secret, 0, sizeof(secret));

	i_PRF(secret, sizeof(secret), (unsigned char*)RANDOM_EXPANSION_CONST, RANDOM_EXPANSION_CONST_SIZE,
		  random, sizeof(random), result, KEY_NONCE_LEN);

	memcpy(nonce, result, KEY_NONCE_LEN);
}

static void IntegrityFailure(struct rtl8192cd_priv *priv)
{
	priv->wpa_global_info->IntegrityFailed = FALSE;

	if (priv->wpa_global_info->GKeyFailure) {
		priv->wpa_global_info->GTKRekey = TRUE;
		priv->wpa_global_info->GKeyFailure = FALSE;
	}

	//waitupto60;
	INCOctet32_INTEGER(&priv->wpa_global_info->Counter);
//	SetNonce(global->akm_sm->ANonce, global->auth->Counter);

	INCOctet32_INTEGER(&priv->wpa_global_info->Counter);
//	SetNonce(global->akm_sm->ANonce, global->auth->Counter);
}

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

#ifdef CONFIG_IEEE80211W
static void ToDrv_SetPMF(struct rtl8192cd_priv *priv, struct stat_info *pstat)
{
	int retVal = 0;
	struct iw_point wrq;
	DOT11_SET_11W_Flags flags;

	PMFDEBUG("Drv:setPMF=%d\n", pstat->wpa_sta_info->mgmt_frame_prot);
	flags.EventId = DOT11_EVENT_SET_PMF;
	flags.IsMoreEvent = FALSE;
	flags.isPMF = pstat->wpa_sta_info->mgmt_frame_prot;

	memcpy(flags.macAddr, pstat->hwaddr, 6);

	wrq.pointer = &flags;
	wrq.length = sizeof(flags);

	rtl8192cd_ioctl_priv_daemonreq(priv->dev, &wrq);
}

static void ToDrv_SetIGTK(struct rtl8192cd_priv *priv)
{
	int retVal = 0;
	u_long	ulKeyLength = 0;
	struct iw_point wrq;
	DOT11_SET_KEY  Set_Key;
	u_char	szBradcast[] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};

	/*
	u_char	szDefaultKey[16] = {0x11, 0x11,0x11, 0x11,0x11, 0x11,0x11, 0x11,
					0x11, 0x11,0x11, 0x11,0x11, 0x11,0x11, 0x11};
	*/
	PMFDEBUG("\n");
	wrq.pointer = &Set_Key;

	Set_Key.EventId = DOT11_EVENT_SET_KEY;
	Set_Key.IsMoreEvent = FALSE;
	Set_Key.KeyIndex = priv->wpa_global_info->GN_igtk;
	Set_Key.KeyType = DOT11_KeyType_IGTK;
	memcpy(&Set_Key.MACAddr, szBradcast, 6);

	Set_Key.EncType = DOT11_ENC_BIP;
	//sc_yang
	memset(Set_Key.KeyMaterial, 0, 64);

	// Set IGTK
	memcpy(Set_Key.KeyMaterial ,
		   priv->wpa_global_info->IGTK[priv->wpa_global_info->GN_igtk - 4] ,
		   16);


	wrq.length = sizeof(DOT11_SET_KEY) - 1 + 16;

	rtl8192cd_ioctl_priv_daemonreq(priv->dev, &wrq);

}
#endif // CONFIG_IEEE80211W


static void ToDrv_SetPTK(struct rtl8192cd_priv *priv, struct stat_info *pstat)
{
	unsigned long ulKeyLength = 0;
	unsigned char *pucKeyMaterial = 0;
	struct iw_point wrq;
	DOT11_SET_KEY Set_Key;

#ifdef DEBUG_PSK
	printk("PSK: Set PTK, mac: %02x:%02x:%02x:%02x:%02x:%02x\n",
		   pstat->hwaddr[0], pstat->hwaddr[1], pstat->hwaddr[2],
		   pstat->hwaddr[3], pstat->hwaddr[4], pstat->hwaddr[5]);
#endif

	wrq.pointer = (caddr_t)&Set_Key;
	Set_Key.EventId = DOT11_EVENT_SET_KEY;
	Set_Key.IsMoreEvent = FALSE;
	Set_Key.KeyIndex = 0;
	Set_Key.KeyType = DOT11_KeyType_Pairwise;

		memcpy(&Set_Key.MACAddr, pstat->hwaddr, 6);

	switch (pstat->wpa_sta_info->UnicastCipher) {
	case DOT11_ENC_TKIP:
		ulKeyLength =  PTK_LEN_TKIP - (PTK_LEN_EAPOLMIC + PTK_LEN_EAPOLENC);
		break;

		// Kenny
	case DOT11_ENC_CCMP:
		ulKeyLength =  PTK_LEN_CCMP - (PTK_LEN_EAPOLMIC + PTK_LEN_EAPOLENC);
		break;
	}
	pucKeyMaterial = pstat->wpa_sta_info->PTK + (PTK_LEN_EAPOLMIC + PTK_LEN_EAPOLENC);

	//sc_yang
	memset(Set_Key.KeyMaterial, 0, 64);
	memcpy(Set_Key.KeyMaterial, pucKeyMaterial, ulKeyLength);
	Set_Key.EncType = pstat->wpa_sta_info->UnicastCipher;
	Set_Key.KeyLen = ulKeyLength;
	wrq.length = sizeof(DOT11_SET_KEY) - 1 + ulKeyLength;

#ifdef DEBUG_PSK
	debug_out(NULL, Set_Key.KeyMaterial, ulKeyLength);
#endif

	rtl8192cd_ioctl_priv_daemonreq(priv->dev, &wrq);
}

void ToDrv_SetGTK(struct rtl8192cd_priv *priv)
{
	unsigned long ulKeyLength = 0;
	//unsigned long  ulKeyIndex = 0;
	struct iw_point wrq;
	DOT11_SET_KEY  Set_Key;
	unsigned char szBradcast[] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};

	memset(&Set_Key, 0, sizeof(DOT11_SET_KEY));

#ifdef DEBUG_PSK
	printk("PSK: Set GTK\n");
#endif

	wrq.pointer = (caddr_t)&Set_Key;
	wrq.length = sizeof(DOT11_SET_KEY);

	Set_Key.EventId = DOT11_EVENT_SET_KEY;
	Set_Key.IsMoreEvent = FALSE;

	Set_Key.KeyType = DOT11_KeyType_Group;
	memcpy(&Set_Key.MACAddr, szBradcast, 6);

	Set_Key.EncType = priv->wpa_global_info->MulticastCipher;
	//sc_yang
	memset(Set_Key.KeyMaterial, 0, 64);
	ulKeyLength = 32;
#ifdef CLIENT_MODE
	if (OPMODE & WIFI_ADHOC_STATE) {
		memcpy(Set_Key.KeyMaterial, priv->wpa_global_info->PSK, ulKeyLength);
		Set_Key.KeyIndex = 0;
	} else
#endif
	{
		//ulKeyIndex = priv->wpa_global_info->GN;
#ifdef CONFIG_IEEE80211W
		if (priv->wpa_global_info->MulticastCipher == DOT11_ENC_BIP) {
			memcpy(Set_Key.KeyMaterial,
				   priv->wpa_global_info->IGTK[priv->wpa_global_info->GN_igtk],
				   ulKeyLength);
			Set_Key.KeyIndex = priv->wpa_global_info->GN_igtk;

		} else
#endif
		{
			memcpy(Set_Key.KeyMaterial,
				   priv->wpa_global_info->GTK[priv->wpa_global_info->GN],
				   ulKeyLength);
			Set_Key.KeyIndex = priv->wpa_global_info->GN;
		}
	}

	wrq.length = sizeof(DOT11_SET_KEY) - 1 + ulKeyLength;
	wrq.length = sizeof(DOT11_SET_KEY) - 1 + ulKeyLength;

#ifdef DEBUG_PSK
	debug_out(NULL, Set_Key.KeyMaterial, ulKeyLength);
#endif

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

void ToDrv_SetIE(struct rtl8192cd_priv *priv)
{
	struct iw_point wrq;
	DOT11_SET_RSNIE Set_Rsnie;
#ifdef DEBUG_PSK
	debug_out("PSK: Set RSNIE", priv->wpa_global_info->AuthInfoElement.Octet,
			  priv->wpa_global_info->AuthInfoElement.Length);
#endif
    PMFDEBUG("dev[%s]\n",priv->dev->name);

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

static void ToDrv_DisconnectSTA(struct rtl8192cd_priv *priv, struct stat_info *pstat, int reason)
{
	struct iw_point wrq;
	DOT11_DISCONNECT_REQ	Disconnect_Req;

#ifdef DEBUG_PSK
	printk("PSK: disconnect sta, mac: %02x:%02x:%02x:%02x:%02x:%02x\n",
		   pstat->hwaddr[0], pstat->hwaddr[1], pstat->hwaddr[2],
		   pstat->hwaddr[3], pstat->hwaddr[4], pstat->hwaddr[5]);
#endif

	wrq.pointer = (caddr_t)&Disconnect_Req;
	wrq.length = sizeof(DOT11_DISCONNECT_REQ);

	Disconnect_Req.EventId = DOT11_EVENT_DISCONNECT_REQ;
	Disconnect_Req.IsMoreEvent = FALSE;
	Disconnect_Req.Reason = (unsigned short)reason;
	memcpy(Disconnect_Req.MACAddr, pstat->hwaddr, 6);

	rtl8192cd_ioctl_priv_daemonreq(priv->dev, &wrq);
};

static void ToDrv_IndicateMICFail(struct rtl8192cd_priv *priv, unsigned char *mac)
{
	struct iw_point wrq;
	DOT11_MIC_FAILURE MIC_Failure;

	wrq.pointer = (caddr_t)&MIC_Failure;
	wrq.length = sizeof(DOT11_INIT_QUEUE);

	MIC_Failure.EventId = DOT11_EVENT_MIC_FAILURE;
	memcpy(MIC_Failure.MACAddr, mac, 6);

	rtl8192cd_ioctl_priv_daemonreq(priv->dev, &wrq);
}

static void reset_sta_info(struct rtl8192cd_priv *priv, struct stat_info *pstat)
{
	WPA_STA_INFO *pInfo = pstat->wpa_sta_info;

	unsigned long flags;

	SAVE_INT_AND_CLI(flags);

	SMP_LOCK_PSK_RESEND(flags);
	if (timer_pending(&pInfo->resendTimer)) {
		del_timer(&pInfo->resendTimer);
	}
	SMP_UNLOCK_PSK_RESEND(flags);

	if (OPMODE & WIFI_AP_STATE) {
		ToDrv_RemovePTK(priv, pstat->hwaddr, DOT11_KeyType_Pairwise);
		ToDrv_SetPort(priv, pstat, DOT11_PortStatus_Unauthorized);
	}

	memset((char *)pInfo, '\0', sizeof(WPA_STA_INFO));

	pInfo->ANonce.Octet = pInfo->AnonceBuf;
	pInfo->ANonce.Length = KEY_NONCE_LEN;

	pInfo->SNonce.Octet = pInfo->SnonceBuf;
	pInfo->SNonce.Length = KEY_NONCE_LEN;


	pInfo->EAPOLMsgSend.Octet = pInfo->eapSendBuf;
	pInfo->EapolKeyMsgSend.Octet = pInfo->EAPOLMsgSend.Octet + ETHER_HDRLEN + LIB1X_EAPOL_HDRLEN;

	pInfo->EAPOLMsgRecvd.Octet = pInfo->eapRecvdBuf;
	pInfo->EapolKeyMsgRecvd.Octet = pInfo->EAPOLMsgRecvd.Octet + ETHER_HDRLEN + LIB1X_EAPOL_HDRLEN;

	init_timer(&pInfo->resendTimer);
	pInfo->resendTimer.data = (unsigned long)pstat;
	pInfo->resendTimer.function = ResendTimeout;

	pInfo->priv = priv;

	if (OPMODE & WIFI_AP_STATE) {
		pInfo->state = PSK_STATE_IDLE;
	}
#ifdef CLIENT_MODE
	else if (OPMODE & WIFI_STATION_STATE) {
		memcpy(pInfo->PMK, priv->wpa_global_info->PSK, PMK_LEN);

		pInfo->clientHndshkProcessing = FALSE;
		pInfo->clientHndshkDone = FALSE;

		pInfo->CurrentReplayCounter.field.HighPart = 0xffffffff;
		pInfo->CurrentReplayCounter.field.LowPart = 0xffffffff;
	}
#endif

	RESTORE_INT(flags);
}


static void ResendTimeout(unsigned long task_psta)
{
	struct stat_info *pstat = (struct stat_info *)task_psta;
	struct rtl8192cd_priv *priv = pstat->wpa_sta_info->priv;

	DEBUG_TRACE;

	if (!(priv->drv_state & DRV_STATE_OPEN))
		return;

	if (pstat == NULL)
		return;

	SMP_LOCK_PSK_RESEND(flags);
#ifdef CLIENT_MODE
	if ((OPMODE & WIFI_STATION_STATE) && !pstat->wpa_sta_info->clientHndshkProcessing) {
		DEBUG_ERR("Wait EAP timeout, disconnect  AP!\n");
		priv->dot114WayStatus = _RSON_4WAY_HNDSHK_TIMEOUT_;
		ToDrv_DisconnectSTA(priv, pstat, expire);
		SMP_UNLOCK_PSK_RESEND(flags);
		return;
	}
#endif

	if (++pstat->wpa_sta_info->resendCnt > MAX_RESEND_NUM)
	{

// When the case of group rekey timeout, update GTK to driver when it is
// the last one node
		if (OPMODE & WIFI_AP_STATE) {
			if (pstat->wpa_sta_info->state == PSK_STATE_PTKINITDONE &&
					pstat->wpa_sta_info->gstate == PSK_GSTATE_REKEYNEGOTIATING) {
				if (priv->wpa_global_info->GKeyDoneStations > 0)
					priv->wpa_global_info->GKeyDoneStations--;

				if (priv->wpa_global_info->GKeyDoneStations == 0 && !priv->wpa_global_info->GkeyReady) {
					ToDrv_SetGTK(priv);
#ifdef CONFIG_IEEE80211W
					if (priv->pmib->dot1180211AuthEntry.dot11IEEE80211W != NO_MGMT_FRAME_PROTECTION)
						ToDrv_SetIGTK(priv);
#endif
					priv->wpa_global_info->GkeyReady = TRUE;
					priv->wpa_global_info->GResetCounter = TRUE;

					// start groupkey rekey timer
					if (priv->pmib->dot1180211AuthEntry.dot11GKRekeyTime)
						mod_timer(&priv->wpa_global_info->GKRekeyTimer, jiffies + RTL_SECONDS_TO_JIFFIES(priv->pmib->dot1180211AuthEntry.dot11GKRekeyTime));

					// if only one sta and group key rekey fail, don't disconnect the sta
					if (get_assoc_sta_num(priv, 0) == 1) {
#ifdef DEBUG_PSK
						printk("!!!!DO NOT disconnect STA under 1 client case!!!\n");
#endif
						SMP_UNLOCK_PSK_RESEND(flags);
						return;
					}

				}
			}
		}
#ifdef CLIENT_MODE
		else if (OPMODE & WIFI_STATION_STATE) {
			priv->dot114WayStatus = _RSON_4WAY_HNDSHK_TIMEOUT_;
		}
#endif
//--------------------------------------------------------- david+2006-04-06

		priv->ext_stats.rx_cnt_psk_to++;
		ToDrv_DisconnectSTA(priv, pstat, RSN_4_way_handshake_timeout);
#if defined(CONFIG_AUTH_RESULT)
		//panic_printk("[%s:%d]pstat->rssi:%d\n",__FUNCTION__,__LINE__,pstat->rssi);
		//if(pstat->rssi<35)
			priv->authRes = RSN_4_way_handshake_timeout;
#endif
// need not reset because ToDrv_DisconnectSTA() will take care of it, david+2006-04-06
//		reset_sta_info(priv, pstat);

#ifdef DEBUG_PSK
		printk("PSK: Exceed max retry, disconnect sta\n");
#endif
	} else {
		if (OPMODE & WIFI_AP_STATE)
			SendEAPOL(priv, pstat, 1);
#ifdef CLIENT_MODE
		else if (OPMODE & WIFI_STATION_STATE)
			ClientSendEAPOL(priv, pstat, 0);
#endif
	}

	SMP_UNLOCK_PSK_RESEND(flags);
}


static void GKRekeyTimeout(unsigned long task_priv)
{
	struct rtl8192cd_priv *priv = (struct rtl8192cd_priv *)task_priv;
	unsigned long flags = 0;

	DEBUG_TRACE;

	if (!(priv->drv_state & DRV_STATE_OPEN))
		return;

	SAVE_INT_AND_CLI(flags);
//	SMP_LOCK(flags);

	SMP_LOCK_PSK_GKREKEY(flags2);
	priv->wpa_global_info->GTKRekey = TRUE;
	SMP_UNLOCK_PSK_GKREKEY(flags2);
	UpdateGK(priv);

	RESTORE_INT(flags);
//	SMP_UNLOCK(flags);
}

static void SendEAPOL(struct rtl8192cd_priv *priv, struct stat_info *pstat, int resend)
{
	OCTET_STRING	IV, RSC, KeyID, MIC, KeyData, EAPOLMsgSend, EapolKeyMsgSend;
	unsigned char	IV_buff[KEY_IV_LEN], RSC_buff[KEY_RSC_LEN];
	unsigned char	ID_buff[KEY_ID_LEN], MIC_buff[KEY_MIC_LEN], KEY_buff[INFO_ELEMENT_SIZE];
	unsigned short	tmpKeyData_Length;
	unsigned char	KeyDescriptorVer = key_desc_ver1;
	int 			IfCalcMIC = 0;
	struct wlan_ethhdr_t *eth_hdr;
	struct lib1x_eapol *eapol;
	struct sk_buff	*pskb;
	lib1x_eapol_key	*eapol_key;
	WPA_STA_INFO	*pStaInfo;
	WPA_GLOBAL_INFO *pGblInfo;

	DEBUG_TRACE;

	if (priv == NULL || pstat == NULL)
		return;

	pStaInfo = pstat->wpa_sta_info;
	pGblInfo = priv->wpa_global_info;

	if (pStaInfo->state == PSK_STATE_IDLE)
		return;

	memset(&EapolKeyMsgSend, 0, sizeof(EapolKeyMsgSend));
	EAPOLMsgSend.Octet = pStaInfo->EAPOLMsgSend.Octet;
	EapolKeyMsgSend.Octet = EAPOLMsgSend.Octet + ETHER_HDRLEN + LIB1X_EAPOL_HDRLEN;
	pStaInfo->EapolKeyMsgSend.Octet = EapolKeyMsgSend.Octet;
	eapol_key = (lib1x_eapol_key *)EapolKeyMsgSend.Octet;

#ifdef CONFIG_IEEE80211W
	if (pStaInfo->AuthKeyMethod == DOT11_AuthKeyType_PSK_SHA256)
		KeyDescriptorVer = key_desc_ver3;
	else
#endif
		if ((pGblInfo->MulticastCipher == _CCMP_PRIVACY_) || (pStaInfo->UnicastCipher == _CCMP_PRIVACY_))
			KeyDescriptorVer = key_desc_ver2;

	if (resend) {
		EAPOLMsgSend.Length = pStaInfo->EAPOLMsgSend.Length;
		EapolKeyMsgSend.Length = pStaInfo->EapolKeyMsgSend.Length;
		Message_setReplayCounter(EapolKeyMsgSend, pStaInfo->CurrentReplayCounter.field.HighPart, pStaInfo->CurrentReplayCounter.field.LowPart);
		INCLargeInteger(&pStaInfo->CurrentReplayCounter);
		IfCalcMIC = TRUE;
		goto send_packet;
	}

	IV.Octet = IV_buff;
	IV.Length = KEY_IV_LEN;
	RSC.Octet = RSC_buff;
	RSC.Length = KEY_RSC_LEN;
	KeyID.Octet = ID_buff;
	KeyID.Length = KEY_ID_LEN;
	MIC.Octet = MIC_buff;
	MIC.Length = KEY_MIC_LEN;
	KeyData.Octet = KEY_buff;
	KeyData.Length = 0;

	switch (pStaInfo->state) {
	case PSK_STATE_PTKSTART:
		//send 1st message of 4-way handshake
		DEBUG_INFO("4-1\n");
		PSKDEBUG("4-1\n");
		memset(EapolKeyMsgSend.Octet, 0, MAX_EAPOLKEYMSG_LEN);

#ifdef RTL_WPA2
		if (pStaInfo->RSNEnabled & PSK_WPA2)
			Message_setDescType(EapolKeyMsgSend, desc_type_WPA2);
		else
#endif
			Message_setDescType(EapolKeyMsgSend, desc_type_RSN);

		Message_setKeyDescVer(EapolKeyMsgSend, KeyDescriptorVer);
		Message_setKeyType(EapolKeyMsgSend, type_Pairwise);
		Message_setKeyIndex(EapolKeyMsgSend, 0);
		Message_setInstall(EapolKeyMsgSend, 0);
		Message_setKeyAck(EapolKeyMsgSend, 1);
		Message_setKeyMIC(EapolKeyMsgSend, 0);
		Message_setSecure(EapolKeyMsgSend, 0);
		Message_setError(EapolKeyMsgSend, 0);
		Message_setRequest(EapolKeyMsgSend, 0);
		Message_setReserved(EapolKeyMsgSend, 0);
		Message_setKeyLength(EapolKeyMsgSend, (pStaInfo->UnicastCipher  == DOT11_ENC_TKIP) ? 32 : 16);
//			Message_setKeyLength(EapolKeyMsgSend, 32);

		// make 4-1's ReplyCounter increased
		Message_setReplayCounter(EapolKeyMsgSend, pStaInfo->CurrentReplayCounter.field.HighPart, pStaInfo->CurrentReplayCounter.field.LowPart);
		memcpy(&pStaInfo->ReplayCounterStarted, &pStaInfo->CurrentReplayCounter, sizeof(LARGE_INTEGER)); // save started reply counter, david+1-12-2007

		INCLargeInteger(&pStaInfo->CurrentReplayCounter);

		INCOctet32_INTEGER(&pGblInfo->Counter);
//#ifndef RTL_WPA2_PREAUTH
		// ANonce is only updated in lib1x_init_authenticator()
		// or after 4-way handshake
		// To avoid different ANonce values among multiple issued 4-1 messages because of multiple association requests
		// Different ANonce values among multiple 4-1 messages induce 4-2 MIC failure.
		SetNonce(pStaInfo->ANonce, pGblInfo->Counter);
//#endif
		Message_setKeyNonce(EapolKeyMsgSend, pStaInfo->ANonce);

		memset(IV.Octet, 0, IV.Length);
		Message_setKeyIV(EapolKeyMsgSend, IV);
		memset(RSC.Octet, 0, RSC.Length);
		Message_setKeyRSC(EapolKeyMsgSend, RSC);
		memset(KeyID.Octet, 0, KeyID.Length);
		Message_setKeyID(EapolKeyMsgSend, KeyID);

// enable it to interoper with Intel 11n Centrino, david+2007-11-19
#if 1
		// otherwise PMK cache
		if ((pStaInfo->RSNEnabled & PSK_WPA2) && (pStaInfo->AuthKeyMethod == DOT11_AuthKeyType_PSK || pStaInfo->PMKCached) ) {
			static char PMKID_KDE_TYPE[] = { 0xDD, 0x14, 0x00, 0x0F, 0xAC, 0x04 };
			Message_setKeyDataLength(EapolKeyMsgSend, 22);
			memcpy(EapolKeyMsgSend.Octet + KeyDataPos,
				   PMKID_KDE_TYPE, sizeof(PMKID_KDE_TYPE));
//				memcpy(EapolKeyMsgSend.Octet+KeyDataPos+sizeof(PMKID_KDE_TYPE),
//					global->akm_sm->PMKID, PMKID_LEN);
		} else
#endif
			Message_setKeyDataLength(EapolKeyMsgSend, 0);

		memset(MIC.Octet, 0, MIC.Length);
		Message_setMIC(EapolKeyMsgSend, MIC);

#ifdef RTL_WPA2
		if (pStaInfo->RSNEnabled & PSK_WPA2)
			EapolKeyMsgSend.Length = EAPOLMSG_HDRLEN + Message_KeyDataLength(EapolKeyMsgSend);
		else
#endif
			EapolKeyMsgSend.Length = EAPOLMSG_HDRLEN;
		EAPOLMsgSend.Length = ETHER_HDRLEN + LIB1X_EAPOL_HDRLEN + EapolKeyMsgSend.Length;
		break;

	case PSK_STATE_PTKINITNEGOTIATING:

		//Construct Message3
		DEBUG_INFO("4-3\n");
		PSKDEBUG("4-3\n");
		memset(EapolKeyMsgSend.Octet, 0, MAX_EAPOLKEYMSG_LEN);

		pStaInfo->resendCnt = MAX_RESEND_NUM; // sent 4-3 only one time to avoid key re-installation attack
#ifdef RTL_WPA2
		if (pStaInfo->RSNEnabled & PSK_WPA2)
			Message_setDescType(EapolKeyMsgSend, desc_type_WPA2);
		else
#endif
			Message_setDescType(EapolKeyMsgSend, desc_type_RSN);

		Message_setKeyDescVer(EapolKeyMsgSend, Message_KeyDescVer(pStaInfo->EapolKeyMsgRecvd));
		Message_setKeyType(EapolKeyMsgSend, Message_KeyType(pStaInfo->EapolKeyMsgRecvd));
		Message_setKeyIndex(EapolKeyMsgSend, Message_KeyIndex(pStaInfo->EapolKeyMsgRecvd));

		//Message_setInstall(global->EapolKeyMsgSend, global->RSNVariable.isSuppSupportUnicastCipher ? 1:0);
		Message_setInstall(EapolKeyMsgSend, 1);
		Message_setKeyAck(EapolKeyMsgSend, 1);
		Message_setKeyMIC(EapolKeyMsgSend, 1);
		//??
		//	Message_setSecure(pStaInfo->EapolKeyMsgSend, pStaInfo->RSNVariable.isSuppSupportMulticastCipher ? 0:1);
		Message_setSecure(EapolKeyMsgSend, 0);
		//??
		Message_setError(EapolKeyMsgSend, 0);
		Message_setRequest(EapolKeyMsgSend, 0);
		Message_setReserved(EapolKeyMsgSend, 0);
		Message_setKeyLength(EapolKeyMsgSend, (pStaInfo->UnicastCipher  == DOT11_ENC_TKIP) ? 32 : 16);
		Message_setReplayCounter(EapolKeyMsgSend, pStaInfo->CurrentReplayCounter.field.HighPart, pStaInfo->CurrentReplayCounter.field.LowPart);
		Message_setKeyNonce(EapolKeyMsgSend, pStaInfo->ANonce);
		memset(IV.Octet, 0, IV.Length);
		Message_setKeyIV(EapolKeyMsgSend, IV);

#ifdef RTL_WPA2
		if (pStaInfo->RSNEnabled & PSK_WPA2) {
			unsigned char key_data[128];
			unsigned char *key_data_pos = key_data;
			int i;
			unsigned char GTK_KDE_TYPE[] = {0xDD, 0x16, 0x00, 0x0F, 0xAC, 0x01, 0x01, 0x00};
#ifdef CONFIG_IEEE80211W
			unsigned char IGTK_KDE_TYPE[] = {0xDD, 0x1C, 0x00, 0x0F, 0xAC, 0x09};
#endif


			EapolKeyMsgSend.Octet[1] = 0x13;
//???
			if (KeyDescriptorVer == key_desc_ver2
			#ifdef CONFIG_IEEE80211W
				|| KeyDescriptorVer == key_desc_ver3
			#endif
			) {
				INCOctet32_INTEGER(&pGblInfo->Counter);
				SetEAPOL_KEYIV(IV, pGblInfo->Counter);
				//memset(IV.Octet, 0x0, IV.Length);
				Message_setKeyIV(EapolKeyMsgSend, IV);
			}
			// RSN IE
			if (pGblInfo->AuthInfoElement.Octet[0] == WPA2_ELEMENT_ID) {
				int len = (unsigned char)pGblInfo->AuthInfoElement.Octet[1] + 2;
				if (len > 100) {
					DEBUG_ERR("invalid IE length!\n");
#if defined(CONFIG_WLAN_STATS_EXTENTION)
					priv->ext_stats.total_psk_fail++;
#endif
					return;
				}
				memcpy(key_data_pos, pGblInfo->AuthInfoElement.Octet, len);
				key_data_pos += len;
			} else {
				//find WPA2_ELEMENT_ID 0x30
				int len = (unsigned char)pGblInfo->AuthInfoElement.Octet[1] + 2;
				//printf("%s: global->auth->RSNVariable.AuthInfoElement.Octet[%d] = %02X\n", (char *)__FUNCTION__, len, global->auth->RSNVariable.AuthInfoElement.Octet[len]);
				if (pGblInfo->AuthInfoElement.Octet[len] == WPA2_ELEMENT_ID) {
					int len2 = (unsigned char)pGblInfo->AuthInfoElement.Octet[len + 1] + 2;
					memcpy(key_data_pos, pGblInfo->AuthInfoElement.Octet + len, len2);
					key_data_pos += len2;
				} else {
					DEBUG_ERR("%d ERROR!\n", __LINE__);
#if defined(CONFIG_WLAN_STATS_EXTENTION)
					priv->ext_stats.total_psk_fail++;
#endif
				}
			}


			memcpy(key_data_pos, GTK_KDE_TYPE, sizeof(GTK_KDE_TYPE));
			key_data_pos[1] = (unsigned char) 6 + ((pGblInfo->MulticastCipher == DOT11_ENC_TKIP) ? 32 : 16);
			key_data_pos += sizeof(GTK_KDE_TYPE);


			// FIX GROUPKEY ALL ZERO
// david+2006-04-04, fix the issue of re-generating group key
//				pGblInfo->GInitAKeys = TRUE;
			UpdateGK(priv);
			memcpy(key_data_pos, pGblInfo->GTK[pGblInfo->GN], (pGblInfo->MulticastCipher == DOT11_ENC_TKIP) ? 32 : 16);

			key_data_pos += (pGblInfo->MulticastCipher == DOT11_ENC_TKIP) ? 32 : 16;

			//=================================================
			// IGTK KDE
#ifdef CONFIG_IEEE80211W
			PMFDEBUG("mgmt_frame_prot=[%d]\n", pstat->wpa_sta_info->mgmt_frame_prot);
			if (pstat->wpa_sta_info->mgmt_frame_prot) {
				//PMFDEBUG("SendEAPOL,PATH5\n");
				memcpy(key_data_pos, IGTK_KDE_TYPE, sizeof(IGTK_KDE_TYPE));
				key_data_pos += sizeof(IGTK_KDE_TYPE);
				// Key ID
				*(key_data_pos) = (unsigned char)priv->wpa_global_info->GN_igtk;
				*(key_data_pos + 1) = 0;
				key_data_pos += 2;
				// IPN
				*(key_data_pos++) = priv->wpa_global_info->IGTK_PN._byte_.TSC0;
				*(key_data_pos++) = priv->wpa_global_info->IGTK_PN._byte_.TSC1;
				*(key_data_pos++) = priv->wpa_global_info->IGTK_PN._byte_.TSC2;
				*(key_data_pos++) = priv->wpa_global_info->IGTK_PN._byte_.TSC3;
				*(key_data_pos++) = priv->wpa_global_info->IGTK_PN._byte_.TSC4;
				*(key_data_pos++) = priv->wpa_global_info->IGTK_PN._byte_.TSC5;

				memcpy(key_data_pos, priv->wpa_global_info->IGTK[priv->wpa_global_info->GN_igtk - 4], 16);
                #if 0 // for debug
				PMFDEBUG("IGTK=");
				for (i = 0; i < 16; i++)
					panic_printk("%x", priv->wpa_global_info->IGTK[priv->wpa_global_info->GN_igtk - 4][i]);
				panic_printk("\n");
                #endif
				key_data_pos += 16;
			}
#endif

			i = (key_data_pos - key_data) % 8;
			if ( i != 0 ) {
				*key_data_pos = 0xdd;
				key_data_pos++;
				for (i = i + 1; i < 8; i++) {
					*key_data_pos = 0x0;
					key_data_pos++;
				}

			}
			EncGTK(priv, pstat,
				   pStaInfo->PTK + PTK_LEN_EAPOLMIC, PTK_LEN_EAPOLENC,
				   key_data,
				   (key_data_pos - key_data),
				   KeyData.Octet, &tmpKeyData_Length);
			KeyData.Length = (int)tmpKeyData_Length;
			Message_setKeyData(EapolKeyMsgSend, KeyData);
			Message_setKeyDataLength(EapolKeyMsgSend, KeyData.Length);

			EapolKeyMsgSend.Length = EAPOLMSG_HDRLEN +	KeyData.Length;
			RSC.Octet[0] = priv->pmib->dot11GroupKeysTable.dot11EncryptKey.dot11TXPN48._byte_.TSC0;
			RSC.Octet[1] = priv->pmib->dot11GroupKeysTable.dot11EncryptKey.dot11TXPN48._byte_.TSC1;
			RSC.Octet[2] = priv->pmib->dot11GroupKeysTable.dot11EncryptKey.dot11TXPN48._byte_.TSC2;
			RSC.Octet[3] = priv->pmib->dot11GroupKeysTable.dot11EncryptKey.dot11TXPN48._byte_.TSC3;
			RSC.Octet[4] = priv->pmib->dot11GroupKeysTable.dot11EncryptKey.dot11TXPN48._byte_.TSC4;
			RSC.Octet[5] = priv->pmib->dot11GroupKeysTable.dot11EncryptKey.dot11TXPN48._byte_.TSC5;
			RSC.Octet[6] = 0;
			RSC.Octet[7] = 0;
			Message_setKeyRSC(EapolKeyMsgSend, RSC);
		} else
#endif // RTL_WPA2
		{
			// WPA
			memset(RSC.Octet, 0, RSC.Length);
			Message_setKeyRSC(EapolKeyMsgSend, RSC);
			memset(KeyID.Octet, 0, KeyID.Length);
			Message_setKeyID(EapolKeyMsgSend, KeyID);
			//lib1x_hexdump2(MESS_DBG_KEY_MANAGE, "lib1x_akmsm_ProcessEAPOL_proc", global->auth->RSNVariable.AuthInfoElement.Octet, global->auth->RSNVariable.AuthInfoElement.Length,"Append Authenticator Information Element");

			{
				//WPA 0xDD
				//printf("%s: global->auth->RSNVariable.AuthInfoElement.Octet[0] = %02X\n", (char *)__FUNCTION__, global->auth->RSNVariable.AuthInfoElement.Octet[0]);

				int len = (unsigned char)pGblInfo->AuthInfoElement.Octet[1] + 2;

				if (pGblInfo->AuthInfoElement.Octet[0] == RSN_ELEMENT_ID) {
					memcpy(KeyData.Octet, pGblInfo->AuthInfoElement.Octet, len);
					KeyData.Length = len;
				} else {
					// impossible case??
					int len2 = (unsigned char)pGblInfo->AuthInfoElement.Octet[len + 1] + 2;
					memcpy(KeyData.Octet, pGblInfo->AuthInfoElement.Octet + len, len2);
					KeyData.Length = len2;
				}
			}
			Message_setKeyDataLength(EapolKeyMsgSend, KeyData.Length);
			Message_setKeyData(EapolKeyMsgSend, KeyData);
			EapolKeyMsgSend.Length = EAPOLMSG_HDRLEN + KeyData.Length;
		}

		INCLargeInteger(&pStaInfo->CurrentReplayCounter);

#if 0
		memset(RSC.Octet, 0, RSC.Length);
		Message_setKeyRSC(EapolKeyMsgSend, RSC);
		memset(KeyID.Octet, 0, KeyID.Length);
		Message_setKeyID(EapolKeyMsgSend, KeyID);
		//lib1x_hexdump2(MESS_DBG_KEY_MANAGE, "lib1x_akmsm_ProcessEAPOL_proc", global->auth->RSNVariable.AuthInfoElement.Octet, global->auth->RSNVariable.AuthInfoElement.Length,"Append Authenticator Information Element");
		Message_setKeyDataLength(EapolKeyMsgSend, pGblInfo->AuthInfoElement.Length);
		Message_setKeyData(EapolKeyMsgSend, pGblInfo->AuthInfoElement);
		//Message_setKeyDataLength(global->EapolKeyMsgSend, global->akm_sm->AuthInfoElement.Length);
		//Message_setKeyData(global->EapolKeyMsgSend, global->akm_sm->AuthInfoElement);
		EapolKeyMsgSend.Length = EAPOLMSG_HDRLEN + pGblInfo->AuthInfoElement.Length;
#endif /* RTL_WPA2 */

		EAPOLMsgSend.Length = ETHER_HDRLEN + LIB1X_EAPOL_HDRLEN + EapolKeyMsgSend.Length;
		IfCalcMIC = TRUE;
		break;

	case PSK_STATE_PTKINITDONE:
		//send 1st message of 2-way handshake
		DEBUG_INFO("2-1\n");
		PSKDEBUG("2-1\n");
		memset(EapolKeyMsgSend.Octet, 0, MAX_EAPOLKEYMSG_LEN);

#ifdef RTL_WPA2
		if (pStaInfo->RSNEnabled & PSK_WPA2)
			Message_setDescType(EapolKeyMsgSend, desc_type_WPA2);
		else
#endif
			Message_setDescType(EapolKeyMsgSend, desc_type_RSN);

		Message_setKeyDescVer(EapolKeyMsgSend, KeyDescriptorVer);
		Message_setKeyType(EapolKeyMsgSend, type_Group);
		Message_setKeyIndex(EapolKeyMsgSend, 1);
		Message_setInstall(EapolKeyMsgSend, 1);
		Message_setKeyAck(EapolKeyMsgSend, 1);
		Message_setKeyMIC(EapolKeyMsgSend, 1);
		Message_setSecure(EapolKeyMsgSend, 1);
		Message_setError(EapolKeyMsgSend, 0);
		Message_setRequest(EapolKeyMsgSend, 0);
		Message_setReserved(EapolKeyMsgSend, 0);

		EapolKeyMsgSend.Octet[1] = 0x03;
		if (KeyDescriptorVer == key_desc_ver1 )
			EapolKeyMsgSend.Octet[2] = 0x91;
		else
			EapolKeyMsgSend.Octet[2] = 0x92;

		Message_setKeyLength(EapolKeyMsgSend, (pGblInfo->MulticastCipher == DOT11_ENC_TKIP) ? 32 : 16);
		Message_setReplayCounter(EapolKeyMsgSend, pStaInfo->CurrentReplayCounter.field.HighPart, pStaInfo->CurrentReplayCounter.field.LowPart);
		memcpy(&pStaInfo->LatestGKReplayCounter,&pStaInfo->CurrentReplayCounter, sizeof(pStaInfo->CurrentReplayCounter));
		INCLargeInteger(&pStaInfo->CurrentReplayCounter);
		// kenny: n+2
		INCLargeInteger(&pStaInfo->CurrentReplayCounter);
		SetNonce(pGblInfo->GNonce, pGblInfo->Counter);
		Message_setKeyNonce(EapolKeyMsgSend, pGblInfo->GNonce);
		memset(IV.Octet, 0, IV.Length);
		Message_setKeyIV(EapolKeyMsgSend, IV);

		RSC.Octet[0] = priv->pmib->dot11GroupKeysTable.dot11EncryptKey.dot11TXPN48._byte_.TSC0;
		RSC.Octet[1] = priv->pmib->dot11GroupKeysTable.dot11EncryptKey.dot11TXPN48._byte_.TSC1;
		RSC.Octet[2] = priv->pmib->dot11GroupKeysTable.dot11EncryptKey.dot11TXPN48._byte_.TSC2;
		RSC.Octet[3] = priv->pmib->dot11GroupKeysTable.dot11EncryptKey.dot11TXPN48._byte_.TSC3;
		RSC.Octet[4] = priv->pmib->dot11GroupKeysTable.dot11EncryptKey.dot11TXPN48._byte_.TSC4;
		RSC.Octet[5] = priv->pmib->dot11GroupKeysTable.dot11EncryptKey.dot11TXPN48._byte_.TSC5;
		RSC.Octet[6] = 0;
		RSC.Octet[7] = 0;
		Message_setKeyRSC(EapolKeyMsgSend, RSC);

		memset(KeyID.Octet, 0, KeyID.Length);
		Message_setKeyID(EapolKeyMsgSend, KeyID);

#ifdef RTL_WPA2
		if (pStaInfo->RSNEnabled & PSK_WPA2) {
			char key_data[128];
			char *key_data_pos = key_data;
			static char GTK_KDE_TYPE[] = { 0xDD, 0x16, 0x00, 0x0F, 0xAC, 0x01, 0x01, 0x00 };
			memcpy(key_data_pos, GTK_KDE_TYPE, sizeof(GTK_KDE_TYPE));

//fix the bug of using default KDE length -----------
			key_data_pos[1] = (unsigned char)(6 + ((pGblInfo->MulticastCipher == DOT11_ENC_TKIP) ? 32 : 16));
//------------------------------ david+2006-04-04

			key_data_pos += sizeof(GTK_KDE_TYPE);

			EapolKeyMsgSend.Octet[1] = 0x13;
// fill key-data length after encrypt --------------------
#if 0
			if (KeyDescriptorVer == key_desc_ver1) {
// david+2006-01-06, fix the bug of using 0 as group key id
//					EapolKeyMsgSend.Octet[2] = 0x81;
				Message_setKeyDescVer(EapolKeyMsgSend, key_desc_ver1);
				Message_setKeyDataLength(EapolKeyMsgSend,
										 (sizeof(GTK_KDE_TYPE) + ((pGblInfo->MulticastCipher == DOT11_ENC_TKIP) ? 32 : 16)));
			} else if (KeyDescriptorVer == key_desc_ver2) {
// david+2006-01-06, fix the bug of using 0 as group key id
//					EapolKeyMsgSend.Octet[2] = 0x82;
				Message_setKeyDescVer(EapolKeyMsgSend, key_desc_ver2);
				Message_setKeyDataLength(EapolKeyMsgSend,
										 (sizeof(GTK_KDE_TYPE) + (8 + ((pGblInfo->MulticastCipher == DOT11_ENC_TKIP) ? 32 : 16))));
			}
#endif
//------------------------------------- david+2006-04-04

			memcpy(key_data_pos, pGblInfo->GTK[pGblInfo->GN], (pGblInfo->MulticastCipher == DOT11_ENC_TKIP) ? 32 : 16);
			EncGTK(priv, pstat, pStaInfo->PTK + PTK_LEN_EAPOLMIC, PTK_LEN_EAPOLENC,
				   (unsigned char *)key_data,
				   sizeof(GTK_KDE_TYPE) + ((pGblInfo->MulticastCipher == DOT11_ENC_TKIP) ? 32 : 16),
				   KeyData.Octet, &tmpKeyData_Length);
		} else
#endif // RTL_WPA2
		{
// fill key-data length after encrypt ---------------------
#if 0
			if (KeyDescriptorVer == key_desc_ver1)
				Message_setKeyDataLength(EapolKeyMsgSend,
										 ((pGblInfo->MulticastCipher == DOT11_ENC_TKIP) ? 32 : 16));
			else
				Message_setKeyDataLength(EapolKeyMsgSend,
										 (8 + ((pGblInfo->MulticastCipher == DOT11_ENC_TKIP) ? 32 : 16) ));
#endif
//------------------------------------- david+2006-04-04
			EncGTK(priv, pstat, pStaInfo->PTK + PTK_LEN_EAPOLMIC, PTK_LEN_EAPOLENC,
				   pGblInfo->GTK[pGblInfo->GN],
				   (pGblInfo->MulticastCipher == DOT11_ENC_TKIP) ? 32 : 16,
				   KeyData.Octet, &tmpKeyData_Length);
		}

		KeyData.Length = (int)tmpKeyData_Length;
		Message_setKeyData(EapolKeyMsgSend, KeyData);

//set keyData length after encrypt ------------------
		Message_setKeyDataLength(EapolKeyMsgSend, KeyData.Length);
//------------------------------- david+2006-04-04

		/* Kenny
					global->EapolKeyMsgSend.Length = EAPOLMSG_HDRLEN +
							((global->RSNVariable.MulticastCipher == DOT11_ENC_TKIP) ? 32:16);
		*/
		EapolKeyMsgSend.Length = EAPOLMSG_HDRLEN +	KeyData.Length;
		EAPOLMsgSend.Length = ETHER_HDRLEN + LIB1X_EAPOL_HDRLEN + EapolKeyMsgSend.Length;
		IfCalcMIC = TRUE;
		break;
	default:
		DEBUG_ERR("Invalid wpa state [%x]\n", pStaInfo->state);
#if defined(CONFIG_WLAN_STATS_EXTENTION)
		priv->ext_stats.total_psk_fail++;
#endif
		return;
	}//switch

	pStaInfo->EAPOLMsgSend.Length = EAPOLMsgSend.Length;
	pStaInfo->EapolKeyMsgSend.Length = EapolKeyMsgSend.Length;

send_packet:
	eth_hdr = (struct wlan_ethhdr_t *)pStaInfo->EAPOLMsgSend.Octet;
	memcpy(eth_hdr->daddr, pstat->hwaddr, 6);
	memcpy(eth_hdr->saddr, GET_MY_HWADDR, 6);
	eth_hdr->type = htons(LIB1X_ETHER_EAPOL_TYPE);

	eapol = (struct lib1x_eapol *)(EAPOLMsgSend.Octet + ETHER_HDRLEN);
	eapol->protocol_version = LIB1X_EAPOL_VER;
	eapol->packet_type =  LIB1X_EAPOL_KEY;
	eapol->packet_body_length = htons(EapolKeyMsgSend.Length);

	if (IfCalcMIC)
		CalcMIC(EAPOLMsgSend, KeyDescriptorVer, pStaInfo->PTK, PTK_LEN_EAPOLMIC);

	pskb = rtl_dev_alloc_skb(priv, MAX_EAPOLMSG_LEN, _SKB_TX_, 1);
	if (pskb == NULL) {
		DEBUG_ERR("Allocate EAP skb failed!\n");
#if defined(CONFIG_WLAN_STATS_EXTENTION)
		priv->ext_stats.total_psk_fail++;
#endif
		return;
	}
	memcpy(pskb->data, (char *)eth_hdr, EAPOLMsgSend.Length);
	skb_put(pskb, EAPOLMsgSend.Length);

#ifdef DEBUG_PSK
	{
		unsigned char *msg;
		if (pStaInfo->state == PSK_STATE_PTKSTART)
			msg = "4-1";
		else if (pStaInfo->state == PSK_STATE_PTKINITNEGOTIATING)
			msg = "4-3";
		else
			msg = "2-1";

		printk("PSK: Send a EAPOL %s, len=%x\n", msg, pskb->len);
		debug_out(NULL, pskb->data, pskb->len);
	}
#endif
#ifdef ENABLE_RTL_SKB_STATS
	rtl_atomic_dec(&priv->rtl_tx_skb_cnt);
#endif

//	pskb->cb[0] = 7;	// use highest priority to xmit
	if (rtl8192cd_start_xmit(pskb, priv->dev))
		rtl_kfree_skb(priv, pskb, _SKB_TX_);

	mod_timer(&pstat->wpa_sta_info->resendTimer, jiffies + RESEND_TIME);
}




#ifdef CLIENT_MODE
void ClientSendEAPOL(struct rtl8192cd_priv *priv, struct stat_info *pstat, int resend)
{
	OCTET_STRING	IV, RSC, KeyID, MIC, KeyData, EAPOLMsgSend, EapolKeyMsgSend, Nonce;
	unsigned char	IV_buff[KEY_IV_LEN], RSC_buff[KEY_RSC_LEN];
	unsigned char	ID_buff[KEY_ID_LEN], MIC_buff[KEY_MIC_LEN], KEY_buff[INFO_ELEMENT_SIZE], Nonce_buff[KEY_NONCE_LEN];
	struct wlan_ethhdr_t *eth_hdr;
	struct lib1x_eapol *eapol;
	struct sk_buff	*pskb;
	lib1x_eapol_key	*eapol_key;
	WPA_STA_INFO	*pStaInfo;
	WPA_GLOBAL_INFO *pGblInfo;
	unsigned char KeyDescriptorVer = key_desc_ver1;
	int send_report = 0;

	DEBUG_TRACE;

	if (priv == NULL || pstat == NULL)
		return;

	pStaInfo = pstat->wpa_sta_info;
	pGblInfo = priv->wpa_global_info;

	EAPOLMsgSend.Octet = pStaInfo->EAPOLMsgSend.Octet;
	EapolKeyMsgSend.Octet = EAPOLMsgSend.Octet + ETHER_HDRLEN + LIB1X_EAPOL_HDRLEN;
	pStaInfo->EapolKeyMsgSend.Octet = EapolKeyMsgSend.Octet;
	eapol_key = (lib1x_eapol_key *)EapolKeyMsgSend.Octet;

	if (resend) {
		EAPOLMsgSend.Length = pStaInfo->EAPOLMsgSend.Length;
		EapolKeyMsgSend.Length = pStaInfo->EapolKeyMsgSend.Length;
		if ( (pStaInfo->EAPOLMsgSend.Length == 0) || (pStaInfo->EapolKeyMsgSend.Length == 0) ) {
			DEBUG_ERR("(%s):The length of EAPOLMsg or EapolKeyMsg is error! EAPOLMsg:%d,  EapolKeyMsg:%d\n", __FUNCTION__, pStaInfo->EAPOLMsgSend.Length,  pStaInfo->EapolKeyMsgSend.Length);
			return;
		}
		//---goto send_packet
	} else {
		IV.Octet = IV_buff;
		IV.Length = KEY_IV_LEN;
		RSC.Octet = RSC_buff;
		RSC.Length = KEY_RSC_LEN;
		KeyID.Octet = ID_buff;
		KeyID.Length = KEY_ID_LEN;
		MIC.Octet = MIC_buff;
		MIC.Length = KEY_MIC_LEN;
		KeyData.Octet = KEY_buff;
		KeyData.Length = 0;

		Nonce.Octet = Nonce_buff;
		Nonce.Length = KEY_NONCE_LEN;

		if (!pStaInfo->clientHndshkDone) {
			if (pStaInfo->clientHndshkProcessing < fourWaystep3) {
				//send 2nd message of 4-way handshake
				DEBUG_INFO("client mode 4-2\n");
				PSKDEBUG("client mode 4-2\n");

				priv->is_4Way_finished= 0;
				pStaInfo->clientHndshkProcessing = fourWaystep2;

				memset(EapolKeyMsgSend.Octet, 0, MAX_EAPOLKEYMSG_LEN);

				Message_setDescType(EapolKeyMsgSend, Message_DescType(pStaInfo->EapolKeyMsgRecvd));
				Message_setKeyDescVer(EapolKeyMsgSend, Message_KeyDescVer(pStaInfo->EapolKeyMsgRecvd));
				Message_setKeyType(EapolKeyMsgSend, Message_KeyType(pStaInfo->EapolKeyMsgRecvd));
				Message_setKeyIndex(EapolKeyMsgSend, 0);
				Message_setInstall(EapolKeyMsgSend, Message_KeyIndex(pStaInfo->EapolKeyMsgRecvd));
				Message_setKeyAck(EapolKeyMsgSend, 0);
				Message_setKeyMIC(EapolKeyMsgSend, 1);
				Message_setSecure(EapolKeyMsgSend, Message_Secure(pStaInfo->EapolKeyMsgRecvd));
				Message_setError(EapolKeyMsgSend, Message_Error(pStaInfo->EapolKeyMsgRecvd));
				Message_setRequest(EapolKeyMsgSend, Message_Request(pStaInfo->EapolKeyMsgRecvd));
				Message_setReserved(EapolKeyMsgSend, 0);

				Message_setKeyLength(EapolKeyMsgSend, Message_KeyLength(pStaInfo->EapolKeyMsgRecvd));
				Message_CopyReplayCounter(EapolKeyMsgSend, pStaInfo->EapolKeyMsgRecvd);

				Message_setKeyNonce(EapolKeyMsgSend, pStaInfo->SNonce);
				memset(IV.Octet, 0, IV.Length);
				Message_setKeyIV(EapolKeyMsgSend, IV);
				memset(RSC.Octet, 0, RSC.Length);
				Message_setKeyRSC(EapolKeyMsgSend, RSC);
				memset(KeyID.Octet, 0, KeyID.Length);
				Message_setKeyID(EapolKeyMsgSend, KeyID);

				Message_setKeyDataLength(EapolKeyMsgSend, pGblInfo->AuthInfoElement.Length);
				Message_setKeyData(EapolKeyMsgSend, pGblInfo->AuthInfoElement);

				EapolKeyMsgSend.Length = EAPOLMSG_HDRLEN + pGblInfo->AuthInfoElement.Length;
				EAPOLMsgSend.Length = ETHER_HDRLEN + LIB1X_EAPOL_HDRLEN + EapolKeyMsgSend.Length;

			} else {
				//Construct Message4
				DEBUG_INFO("client mode 4-4\n");
				PSKDEBUG("client mode 4-4\n");

				pStaInfo->clientHndshkDone++;
				pStaInfo->clientHndshkProcessing=fourWaystep4;
				priv->dot114WayStatus = 0;
#if defined(CONFIG_AUTH_RESULT)
				priv->authRes = 0;
#endif

				memset(EapolKeyMsgSend.Octet, 0, MAX_EAPOLKEYMSG_LEN);

				Message_setDescType(EapolKeyMsgSend, Message_DescType(pStaInfo->EapolKeyMsgRecvd));
				Message_setKeyDescVer(EapolKeyMsgSend, Message_KeyDescVer(pStaInfo->EapolKeyMsgRecvd));
				Message_setKeyType(EapolKeyMsgSend, Message_KeyType(pStaInfo->EapolKeyMsgRecvd));
				Message_setKeyIndex(EapolKeyMsgSend, 0);
				Message_setInstall(EapolKeyMsgSend, 0);
				Message_setKeyAck(EapolKeyMsgSend, 0);
				Message_setKeyMIC(EapolKeyMsgSend, 1);
				Message_setSecure(EapolKeyMsgSend, Message_Secure(pStaInfo->EapolKeyMsgRecvd));
				Message_setError(EapolKeyMsgSend, 0);
				Message_setRequest(EapolKeyMsgSend, 0);
				Message_setReserved(EapolKeyMsgSend, 0);
				Message_setKeyLength(EapolKeyMsgSend, Message_KeyLength(pStaInfo->EapolKeyMsgRecvd));
				Message_CopyReplayCounter(EapolKeyMsgSend, pStaInfo->EapolKeyMsgRecvd);
				memset(IV.Octet, 0, IV.Length);
				Message_setKeyIV(EapolKeyMsgSend, IV);
				memset(RSC.Octet, 0, RSC.Length);
				Message_setKeyRSC(EapolKeyMsgSend, RSC);
				memset(KeyID.Octet, 0, KeyID.Length);
				Message_setKeyID(EapolKeyMsgSend, KeyID);
				Message_setKeyDataLength(EapolKeyMsgSend, 0);

				EapolKeyMsgSend.Length = EAPOLMSG_HDRLEN;
				EAPOLMsgSend.Length = ETHER_HDRLEN + LIB1X_EAPOL_HDRLEN + EapolKeyMsgSend.Length;

				LOG_MSG( "Open and authenticated\n");

				pStaInfo->clientMICReportReplayCounter.field.HighPart = 0;
				pStaInfo->clientMICReportReplayCounter.field.LowPart = 0;
//				printk("client mode 4-Way Message 4-4 done\n");
			}
		} else {
			if (pStaInfo->clientGkeyUpdate) {
				//send 2nd message of 2-way handshake
				DEBUG_INFO("client mode 2-2\n");
				PSKDEBUG("client mode 2-2\n");
				memset(EapolKeyMsgSend.Octet, 0, MAX_EAPOLKEYMSG_LEN);

				Message_setDescType(EapolKeyMsgSend, Message_DescType(pStaInfo->EapolKeyMsgRecvd));
				Message_setKeyDescVer(EapolKeyMsgSend, Message_KeyDescVer(pStaInfo->EapolKeyMsgRecvd));
				Message_setKeyType(EapolKeyMsgSend, Message_KeyType(pStaInfo->EapolKeyMsgRecvd));
				Message_setKeyIndex(EapolKeyMsgSend, pGblInfo->GN);
				Message_setInstall(EapolKeyMsgSend, 0);
				Message_setKeyAck(EapolKeyMsgSend, 0);
				Message_setKeyMIC(EapolKeyMsgSend, 1);
				Message_setSecure(EapolKeyMsgSend, 1);
				Message_setError(EapolKeyMsgSend, 0);
				Message_setRequest(EapolKeyMsgSend, 0);
				Message_setReserved(EapolKeyMsgSend, 0);

				Message_setKeyLength(EapolKeyMsgSend, Message_KeyLength(pStaInfo->EapolKeyMsgRecvd));
				Message_CopyReplayCounter(EapolKeyMsgSend, pStaInfo->EapolKeyMsgRecvd);
				memset(Nonce.Octet, 0, KEY_NONCE_LEN);
				Message_setKeyNonce(EapolKeyMsgSend, Nonce);
				memset(IV.Octet, 0, IV.Length);
				Message_setKeyIV(EapolKeyMsgSend, IV);
				memset(RSC.Octet, 0, RSC.Length);
				Message_setKeyRSC(EapolKeyMsgSend, RSC);
				memset(KeyID.Octet, 0, KeyID.Length);
				Message_setKeyID(EapolKeyMsgSend, KeyID);
				Message_setKeyDataLength(EapolKeyMsgSend, 0);

				EapolKeyMsgSend.Length = EAPOLMSG_HDRLEN;
				EAPOLMsgSend.Length = ETHER_HDRLEN + LIB1X_EAPOL_HDRLEN + EapolKeyMsgSend.Length;

				//			printk("client mode Group Message 2-2 done\n");
				pStaInfo->clientGkeyUpdate = 0;
			} else {
				DEBUG_INFO("Send MIC Failure Report\n");
				PSKDEBUG("Send MIC Failure Report\n");
				memset(EapolKeyMsgSend.Octet, 0, MAX_EAPOLKEYMSG_LEN);
#ifdef RTL_WPA2
				if (pStaInfo->RSNEnabled & PSK_WPA2)
					Message_setDescType(EapolKeyMsgSend, desc_type_WPA2);
				else
#endif
					Message_setDescType(EapolKeyMsgSend, desc_type_RSN);


		#ifdef CONFIG_IEEE80211W_CLI
				if (pStaInfo->AuthKeyMethod == DOT11_AuthKeyType_PSK_SHA256)
					KeyDescriptorVer = key_desc_ver3;
				else
		#endif
				if ((pGblInfo->MulticastCipher == _CCMP_PRIVACY_) || (pStaInfo->UnicastCipher == _CCMP_PRIVACY_))
					KeyDescriptorVer = key_desc_ver2;
				Message_setKeyDescVer(EapolKeyMsgSend, KeyDescriptorVer);
				Message_setKeyType(EapolKeyMsgSend, 0);
				Message_setKeyIndex(EapolKeyMsgSend, 0);
				Message_setInstall(EapolKeyMsgSend, 0);
				Message_setKeyAck(EapolKeyMsgSend, 0);
				Message_setKeyMIC(EapolKeyMsgSend, 1);
				Message_setSecure(EapolKeyMsgSend, 1);
				Message_setError(EapolKeyMsgSend, 1);
				Message_setRequest(EapolKeyMsgSend, 1);
				Message_setReserved(EapolKeyMsgSend, 0);
				Message_setKeyLength(EapolKeyMsgSend, 0);

				Message_setReplayCounter(EapolKeyMsgSend, pStaInfo->clientMICReportReplayCounter.field.HighPart, pStaInfo->clientMICReportReplayCounter.field.LowPart);
				INCLargeInteger(&pStaInfo->clientMICReportReplayCounter);

				memset(Nonce.Octet, 0, KEY_NONCE_LEN);
				Message_setKeyNonce(EapolKeyMsgSend, Nonce);
				memset(IV.Octet, 0, IV.Length);
				Message_setKeyIV(EapolKeyMsgSend, IV);
				memset(RSC.Octet, 0, RSC.Length);
				Message_setKeyRSC(EapolKeyMsgSend, RSC);
				memset(KeyID.Octet, 0, KeyID.Length);
				Message_setKeyID(EapolKeyMsgSend, KeyID);
				Message_setKeyDataLength(EapolKeyMsgSend, 0);

				EapolKeyMsgSend.Length = EAPOLMSG_HDRLEN;
				EAPOLMsgSend.Length = ETHER_HDRLEN + LIB1X_EAPOL_HDRLEN + EapolKeyMsgSend.Length;
				send_report = 1;
			}
		}
		pStaInfo->EAPOLMsgSend.Length = EAPOLMsgSend.Length;
		pStaInfo->EapolKeyMsgSend.Length = EapolKeyMsgSend.Length;
	}

	//for record EAPOLMSG length
	pStaInfo->EAPOLMsgSend.Length = EAPOLMsgSend.Length;
	pStaInfo->EapolKeyMsgSend.Length = EapolKeyMsgSend.Length;
	//send_packet--------------------------------------------------------------

	eth_hdr = (struct wlan_ethhdr_t *)EAPOLMsgSend.Octet;
	memcpy(eth_hdr->daddr, pstat->hwaddr, 6);
	memcpy(eth_hdr->saddr, GET_MY_HWADDR, 6);
	eth_hdr->type = htons(LIB1X_ETHER_EAPOL_TYPE);

	eapol = (struct lib1x_eapol *)(EAPOLMsgSend.Octet + ETHER_HDRLEN);
	eapol->protocol_version = LIB1X_EAPOL_VER;
	eapol->packet_type =  LIB1X_EAPOL_KEY;
	eapol->packet_body_length = htons(EapolKeyMsgSend.Length);

	if (!resend) {
		if (send_report)
			CalcMIC(EAPOLMsgSend, KeyDescriptorVer, pStaInfo->PTK, PTK_LEN_EAPOLMIC);
		else
			CalcMIC(EAPOLMsgSend, Message_KeyDescVer(pStaInfo->EapolKeyMsgRecvd), pStaInfo->PTK, PTK_LEN_EAPOLMIC);
	}

	pskb = rtl_dev_alloc_skb(priv, MAX_EAPOLMSG_LEN, _SKB_TX_, 1);

	if (pskb == NULL) {
		DEBUG_ERR("Allocate EAP skb failed!\n");
		printk("Allocate EAP skb failed!\n");
		return;
	}

	memcpy(pskb->data, (char *)eth_hdr, EAPOLMsgSend.Length);
	skb_put(pskb, EAPOLMsgSend.Length);

#ifdef ENABLE_RTL_SKB_STATS
	rtl_atomic_dec(&priv->rtl_tx_skb_cnt);
#endif

	if (rtl8192cd_start_xmit(pskb, priv->dev))
		rtl_kfree_skb(priv, pskb, _SKB_TX_);

	if (!pStaInfo->clientHndshkDone) // only 4-2 need to check the time
		mod_timer(&pStaInfo->resendTimer, jiffies + RESEND_TIME);

	//--------------------------------------------------------------send_packet
}
#endif // CLIENT_MODE

static void AuthenticationRequest(struct rtl8192cd_priv *priv, struct stat_info *pstat)
{
	static unsigned int RC_toggle = 0;

	DEBUG_TRACE;

	// For some STA that can only process if Replay Counter is not 0
	if ((++RC_toggle) % 2)
		INCLargeInteger(&pstat->wpa_sta_info->CurrentReplayCounter);

	INCOctet32_INTEGER(&priv->wpa_global_info->Counter);
	SetNonce(pstat->wpa_sta_info->ANonce, priv->wpa_global_info->Counter);

	memcpy(pstat->wpa_sta_info->PMK, priv->wpa_global_info->PSK, PMK_LEN);

//#ifdef RTL_WPA2
#if 0
	CalcPMKID(	akm_sm->PMKID,
				akm_sm->PMK, 	 // PMK
				global->theAuthenticator->global->TxRx->oursupp_addr,   // AA
				global->theAuthenticator->supp_addr); 			// SPA
#endif
	pstat->wpa_sta_info->state = PSK_STATE_PTKSTART;
	pstat->wpa_sta_info->resendCnt = 0;


	//send 1st message
	SendEAPOL(priv, pstat, 0);
}

//-------------------------------------------------------------
// Start 2-way handshake after receiving 4th message
//-------------------------------------------------------------
static void UpdateGK(struct rtl8192cd_priv *priv)
{
	WPA_GLOBAL_INFO *pGblInfo = priv->wpa_global_info;
	struct stat_info *pstat;
	int i;

	SMP_LOCK_PSK_GKREKEY(flags);
	//------------------------------------------------------------
	// Execute Global Group key state machine
	//------------------------------------------------------------
	if (pGblInfo->GTKAuthenticator && (pGblInfo->GTKRekey || pGblInfo->GInitAKeys)) {
		pGblInfo->GTKRekey = FALSE;
		pGblInfo->GInitAKeys = FALSE; // david+2006-04-04, fix the issue of re-generating group key

		INCOctet32_INTEGER(&pGblInfo->Counter);

		// kenny:??? GNonce should be a random number ???
		SetNonce(pGblInfo->GNonce , pGblInfo->Counter);
		CalcGTK(GET_MY_HWADDR, pGblInfo->GNonce.Octet,
				pGblInfo->GMK, GMK_LEN, pGblInfo->GTK[pGblInfo->GN], GTK_LEN, GMK_EXPANSION_CONST);

#ifdef DEBUG_PSK
		debug_out("PSK: Generated GTK=", pGblInfo->GTK[pGblInfo->GN], GTK_LEN);
#endif

		pGblInfo->GUpdateStationKeys = TRUE;
		pGblInfo->GkeyReady = FALSE;

		if (timer_pending(&pGblInfo->GKRekeyTimer))
			del_timer_sync(&pGblInfo->GKRekeyTimer);

		//---- In the case of updating GK to all STAs, only the STA that has finished
		//---- 4-way handshake is needed to be sent with 2-way handshake
		//gkm_sm->GKeyDoneStations = auth->NumOfSupplicant;
		pGblInfo->GKeyDoneStations = 0;

		for (i = 0; i < NUM_STAT; i++) {
			if (priv->pshare->aidarray[i] && (priv->pshare->aidarray[i]->used == TRUE)) {
				if (priv != priv->pshare->aidarray[i]->priv)
					continue;

				if ((priv->pshare->aidarray[i]->station.state & WIFI_ASOC_STATE) &&
						(priv->pshare->aidarray[i]->station.wpa_sta_info->state == PSK_STATE_PTKINITDONE))
					pGblInfo->GKeyDoneStations++;
			}
		}
	}

#ifdef CONFIG_IEEE80211W
	if (priv->pmib->dot1180211AuthEntry.dot11IEEE80211W != NO_MGMT_FRAME_PROTECTION) {
		//int ii;
		PMFDEBUG("Calculate IGTK\n");

		CalcGTK(GET_MY_HWADDR, pGblInfo->GNonce.Octet,
				pGblInfo->GMK, GMK_LEN, pGblInfo->IGTK[pGblInfo->GN_igtk - 4], IGTK_LEN, (u_char*)IGMK_EXPANSION_CONST);
        #ifdef CONFIG_IEEE80211W_AP_DEBUG
		panic_printk("IGTK[%d]\n", pGblInfo->GN_igtk - 4);
		for (ii = 0; ii < IGTK_LEN; ii++) {
			panic_printk("%02x ", pGblInfo->IGTK[pGblInfo->GN_igtk - 4][ii]);
		}
		panic_printk("\n");
        #endif
	}
#endif // CONFIG_IEEE80211W

	//------------------------------------------------------------
	// Execute Group key state machine of each STA
	//------------------------------------------------------------
	for (i = 0; i < NUM_STAT; i++) {
		if (priv->pshare->aidarray[i] == NULL)
			continue;
		pstat = &priv->pshare->aidarray[i]->station;
		if ((priv->pshare->aidarray[i]->used != TRUE) || !(pstat->state & WIFI_ASOC_STATE))
			continue;

		if (priv != priv->pshare->aidarray[i]->priv)
			continue;

		//---- Group key handshake to only one supplicant ----
		if (pstat->wpa_sta_info->state == PSK_STATE_PTKINITDONE &&
				(pGblInfo->GkeyReady && pstat->wpa_sta_info->PInitAKeys)) {
			pstat->wpa_sta_info->PInitAKeys = FALSE;
			pstat->wpa_sta_info->gstate = PSK_GSTATE_REKEYNEGOTIATING; // set proper gstat, david+2006-04-06
			pstat->wpa_sta_info->resendCnt = 0;
			SendEAPOL(priv, pstat, 0);
		}
		//---- Updata group key to all supplicant----
		else if (pstat->wpa_sta_info->state == PSK_STATE_PTKINITDONE &&  //Done 4-way handshake
				 (pGblInfo->GUpdateStationKeys ||                     //When new key is generated
				  pstat->wpa_sta_info->gstate == PSK_GSTATE_REKEYNEGOTIATING))  { //1st message is not yet sent
			pstat->wpa_sta_info->PInitAKeys = FALSE;
			pstat->wpa_sta_info->gstate = PSK_GSTATE_REKEYNEGOTIATING; // set proper gstat, david+2006-04-06
			pstat->wpa_sta_info->resendCnt = 0;
			SendEAPOL(priv, pstat, 0);
		}
	}
	pGblInfo->GUpdateStationKeys = FALSE;

	SMP_UNLOCK_PSK_GKREKEY(flags);
};

static void EAPOLKeyRecvd(struct rtl8192cd_priv *priv, struct stat_info *pstat)
{
	WPA_GLOBAL_INFO *pGblInfo = priv->wpa_global_info;
	WPA_STA_INFO *pStaInfo = pstat->wpa_sta_info;
	LARGE_INTEGER recievedRC;
	struct lib1x_eapol *eapol;
	OCTET_STRING tmpSNonce;

	DEBUG_TRACE;

	eapol = ( struct lib1x_eapol * ) ( pstat->wpa_sta_info->EAPOLMsgRecvd.Octet + ETHER_HDRLEN );
	if (eapol->packet_type != LIB1X_EAPOL_KEY) {
#ifdef DEBUG_PSK
		printk("Not Eapol-key pkt (type %d), drop\n", eapol->packet_type);
#endif
#if defined(CONFIG_WLAN_STATS_EXTENTION)
		priv->ext_stats.total_psk_fail++;
#endif
		return;
	}

	pStaInfo->EapolKeyMsgRecvd.Octet = pStaInfo->EAPOLMsgRecvd.Octet + ETHER_HDRLEN + LIB1X_EAPOL_HDRLEN;

#ifdef DEBUG_PSK
	printk("PSK: Rx a EAPOL %s, len=%x\n", STATE2RXMSG(pstat), pStaInfo->EAPOLMsgRecvd.Length);
	debug_out(NULL, pstat->wpa_sta_info->EAPOLMsgRecvd.Octet, pStaInfo->EAPOLMsgRecvd.Length);
#endif

	//----IEEE 802.11-03/156r2. MIC report : (1)MIC bit (2)error bit (3) request bit
	//----Check if it is MIC failure report. If it is, indicate to driver
	if (Message_KeyMIC(pStaInfo->EapolKeyMsgRecvd) && Message_Error(pStaInfo->EapolKeyMsgRecvd)
			&& Message_Request(pStaInfo->EapolKeyMsgRecvd)) {
#ifdef DEBUG_PSK
		printk("PSK: Rx MIC errir report from client\n");
#endif
		ToDrv_IndicateMICFail(priv, pstat->hwaddr);
#if defined(CONFIG_WLAN_STATS_EXTENTION)
		priv->ext_stats.total_psk_fail++;
		priv->ext_stats.total_mic_fail++;
#endif
		return;
	}

	if (Message_KeyType(pStaInfo->EapolKeyMsgRecvd) == type_Pairwise) {
		switch (pStaInfo->state) {
		case PSK_STATE_PTKSTART:
			//---- Receive 2nd message and send third
			DEBUG_INFO("4-2\n");
			PSKDEBUG("4-2\n");

			//check replay counter
			Message_ReplayCounter_OC2LI(pStaInfo->EapolKeyMsgRecvd, &recievedRC);
			INCLargeInteger(&recievedRC);
			if ( !(pStaInfo->CurrentReplayCounter.field.HighPart == recievedRC.field.HighPart
					&& pStaInfo->CurrentReplayCounter.field.LowPart == recievedRC.field.LowPart)) {
				DEBUG_ERR("4-2: ERROR_NONEEQUL_REPLAYCOUNTER\n");
#if defined(CONFIG_WLAN_STATS_EXTENTION)
				priv->ext_stats.total_psk_fail++;
#endif
				break;
			}

check_msg2:
			//pStaInfo->SNonce = Message_KeyNonce(pStaInfo->EapolKeyMsgRecvd);
			tmpSNonce = Message_KeyNonce(pStaInfo->EapolKeyMsgRecvd);
			memcpy(pStaInfo->SNonce.Octet, tmpSNonce.Octet, KEY_NONCE_LEN);
			pStaInfo->SNonce.Length = tmpSNonce.Length;
			CalcPTK(pStaInfo->EAPOLMsgRecvd.Octet, pStaInfo->EAPOLMsgRecvd.Octet + 6,
					pStaInfo->ANonce.Octet, pStaInfo->SNonce.Octet,
					pStaInfo->PMK, PMK_LEN,
					pStaInfo->PTK, (pStaInfo->AuthKeyMethod == DOT11_AuthKeyType_PSK_SHA256) ? 48 : PTK_LEN_TKIP
#ifdef CONFIG_IEEE80211W
					, (pStaInfo->AuthKeyMethod == DOT11_AuthKeyType_PSK_SHA256)
#endif
				   );

#ifdef DEBUG_PSK
			debug_out("PSK: Generated PTK=", pStaInfo->PTK, PTK_LEN_TKIP);
#endif


			if (!CheckMIC(pStaInfo->EAPOLMsgRecvd, pStaInfo->PTK, PTK_LEN_EAPOLMIC)) {
				if (priv->pmib->dot1180211AuthEntry.dot11PassPhraseGuest[0]) {
					CalcPTK(pStaInfo->EAPOLMsgRecvd.Octet, pStaInfo->EAPOLMsgRecvd.Octet + 6,
							pStaInfo->ANonce.Octet, pStaInfo->SNonce.Octet,
							pGblInfo->PSKGuest, PMK_LEN,
							pStaInfo->PTK, (pStaInfo->AuthKeyMethod == DOT11_AuthKeyType_PSK_SHA256) ? 48 : PTK_LEN_TKIP
#ifdef CONFIG_IEEE80211W
							, (pStaInfo->AuthKeyMethod == DOT11_AuthKeyType_PSK_SHA256)
#endif
						   );
					if (CheckMIC(pStaInfo->EAPOLMsgRecvd, pStaInfo->PTK, PTK_LEN_EAPOLMIC)) {
						pStaInfo->isGuest = 1;
						goto cont_msg;
					}
				}

				DEBUG_ERR("4-2: ERROR_MIC_FAIL\n");
#if defined(CONFIG_WLAN_STATS_EXTENTION)
				priv->ext_stats.total_mic_fail++;
				priv->ext_stats.total_psk_fail++;
#endif
#if 0
				if (global->RSNVariable.PMKCached ) {
					printf("\n%s:%d del_pmksa due to 4-2 ERROR_MIC_FAIL\n", (char *)__FUNCTION__, __LINE__);
					global->RSNVariable.PMKCached = FALSE;
					del_pmksa_by_spa(global->theAuthenticator->supp_addr);
				}
#endif

				LOG_MSG("Authentication failled! (4-2: MIC error)\n");

#if defined(CONFIG_RTL8196B_TR) || defined(CONFIG_RTL865X_SC) || defined(CONFIG_RTL865X_AC) || defined(CONFIG_RTL865X_KLD) || defined(CONFIG_RTL8196B_KLD) || defined(CONFIG_RTL8196C_KLD) || defined(CONFIG_RTL8196C_EC)
				LOG_MSG_NOTICE("Authentication failed;note:%02x-%02x-%02x-%02x-%02x-%02x;\n",
							   pstat->hwaddr[0],
							   pstat->hwaddr[1],
							   pstat->hwaddr[2],
							   pstat->hwaddr[3],
							   pstat->hwaddr[4],
							   pstat->hwaddr[5]);
#endif
#ifdef RTK_WLAN_EVENT_INDICATE
				rtk_wlan_event_indicate(priv->dev->name, WIFI_CONNECT_FAIL, pstat->hwaddr, RSN_MIC_failure);
#endif
#ifdef PROC_STA_CONN_FAIL_INFO
				{
					int i;
					for (i=0; i<64; i++) {
						if (!priv->sta_conn_fail[i].used) {
							priv->sta_conn_fail[i].used = TRUE;
							priv->sta_conn_fail[i].error_state = RSN_MIC_failure;
							memcpy(priv->sta_conn_fail[i].addr, pstat->hwaddr, MACADDRLEN);
							break;
						}
						else {
							if (!memcmp(priv->sta_conn_fail[i].addr, pstat->hwaddr, MACADDRLEN)) {
								priv->sta_conn_fail[i].error_state = RSN_MIC_failure;
								break;
							}
						}
					}
				}
#endif
#ifdef STA_ASSOC_STATISTIC
				add_sta_assoc_status(priv, pstat->hwaddr, pstat->rssi, RSN_MIC_failure);
#endif
#ifdef ERR_ACCESS_CNTR
				{
					int i = 0, found = 0;
					for (i=0; i<MAX_ERR_ACCESS_CNTR; i++) {
						if (priv->err_ac_list[i].used) {
							if (!memcmp(pstat->hwaddr, priv->err_ac_list[i].mac, MACADDRLEN)) {
								priv->err_ac_list[i].num++;
								found++;
								break;
							}
						}
						else
							break;
					}
					if (!found && (i != MAX_ERR_ACCESS_CNTR)) {
						priv->err_ac_list[i].used = TRUE;
						memcpy(priv->err_ac_list[i].mac, pstat->hwaddr, MACADDRLEN);
						priv->err_ac_list[i].num++;
					}
				}
#endif
				/* Fix, to prevent prompt "Connection Timeout" when MacBook connect with incorrect PSK
				ToDrv_DisconnectSTA(priv, pstat, RSN_MIC_failure);
				pStaInfo->state = PSK_STATE_IDLE;
				*/
				break;
			}
cont_msg:
			pStaInfo->resendCnt = 0;
			/* Fix, to prevent display connection timeout when MacBook connect with incorrect PSK.
			   Relocate cancel timer here to make 4-1 retried while 4-2 MIC error, then MacBook will prompt "Incorrect Password" */
			// delete resend timer
			SMP_LOCK_PSK_RESEND(flags);
			if (timer_pending(&pStaInfo->resendTimer)){
				del_timer(&pStaInfo->resendTimer);
			}
			SMP_UNLOCK_PSK_RESEND(flags);
			pStaInfo->state = PSK_STATE_PTKINITNEGOTIATING;
			SendEAPOL(priv, pstat, 0);	// send msg 3
			break;

		case PSK_STATE_PTKINITNEGOTIATING:
			//---- Receive 4th message ----
			DEBUG_INFO("4-4\n");
			PSKDEBUG("4-4\n");

			// test 2nd or 4th message
// check replay counter to determine if msg 2 or 4 received --------------
//			if ( Message_KeyDataLength(pStaInfo->EapolKeyMsgRecvd) != 0) {
			if (Message_EqualReplayCounter(pStaInfo->ReplayCounterStarted, pStaInfo->EapolKeyMsgRecvd)) {
//---------------------------------------------------- david+1-12-2007

				DEBUG_INFO("4-2 in akmsm_PTKINITNEGOTIATING: resend 4-3\n");
				PSKDEBUG("4-2 in akmsm_PTKINITNEGOTIATING: resend 4-3\n");

#if 0 // Don't check replay counter during dup 4-2
#ifdef RTL_WPA2
				Message_ReplayCounter_OC2LI(global->EapolKeyMsgRecvd, &recievedRC);
				INCLargeInteger(&recievedRC);
				if ( !(global->akm_sm->CurrentReplayCounter.field.HighPart == recievedRC.field.HighPart
						&& global->akm_sm->CurrentReplayCounter.field.LowPart == recievedRC.field.LowPart))
#else
				if (!Message_EqualReplayCounter(global->akm_sm->CurrentReplayCounter, global->EapolKeyMsgRecvd))
#endif
				{
#ifdef FOURWAY_DEBUG
					printf("4-2: ERROR_NONEEQUL_REPLAYCOUNTER\n");
					printf("global->akm_sm->CurrentReplayCounter.field.LowPart = %d\n", global->akm_sm->CurrentReplayCounter.field.LowPart);
					printf("recievedRC.field.LowPart = %d\n", recievedRC.field.LowPart);
#endif
					retVal = ERROR_NONEEQUL_REPLAYCOUNTER;
					k
				} else
#endif // Don't check replay counter during dup 4-2
//#ifndef RTL_WPA2
#if 0
					// kenny: already increase CurrentReplayCounter after 4-1. Do it at the end of 4-2
					INCLargeInteger(&global->akm_sm->CurrentReplayCounter);
#endif

				goto check_msg2;
			}

			if (!CheckMIC(pStaInfo->EAPOLMsgRecvd, pStaInfo->PTK, PTK_LEN_EAPOLMIC)) { // errror
				DEBUG_ERR("4-4: RSN_MIC_failure\n");
#if defined(CONFIG_WLAN_STATS_EXTENTION)
				priv->ext_stats.total_mic_fail++;
				priv->ext_stats.total_psk_fail++;
#endif
#if 0
				if (global->RSNVariable.PMKCached ) {
					printf("\n%s:%d del_pmksa due to 4-4 RSN_MIC_failure\n", (char *)__FUNCTION__, __LINE__);
					global->RSNVariable.PMKCached = FALSE;
					del_pmksa_by_spa(global->theAuthenticator->supp_addr);
				}
#endif
				LOG_MSG("Authentication failled! (4-4: MIC error)\n");

#if defined(CONFIG_RTL8196B_TR) || defined(CONFIG_RTL865X_SC) || defined(CONFIG_RTL865X_AC) || defined(CONFIG_RTL865X_KLD) || defined(CONFIG_RTL8196B_KLD) || defined(CONFIG_RTL8196C_KLD) || defined(CONFIG_RTL8196C_EC)
				LOG_MSG_NOTICE("Authentication failed;note:%02x-%02x-%02x-%02x-%02x-%02x;\n",
							   pstat->hwaddr[0],
							   pstat->hwaddr[1],
							   pstat->hwaddr[2],
							   pstat->hwaddr[3],
							   pstat->hwaddr[4],
							   pstat->hwaddr[5]);
#endif
#ifdef RTK_WLAN_EVENT_INDICATE
				rtk_wlan_event_indicate(priv->dev->name, WIFI_CONNECT_FAIL, pstat->hwaddr, RSN_MIC_failure);
#endif
#ifdef PROC_STA_CONN_FAIL_INFO
				{
					int i;
					for (i=0; i<64; i++) {
						if (!priv->sta_conn_fail[i].used) {
							priv->sta_conn_fail[i].used = TRUE;
							priv->sta_conn_fail[i].error_state = RSN_MIC_failure;
							memcpy(priv->sta_conn_fail[i].addr, pstat->hwaddr, MACADDRLEN);
							break;
						}
						else {
							if (!memcmp(priv->sta_conn_fail[i].addr, pstat->hwaddr, MACADDRLEN)) {
								priv->sta_conn_fail[i].error_state = RSN_MIC_failure;
								break;
							}
						}
					}
				}
#endif
#ifdef STA_ASSOC_STATISTIC
				add_sta_assoc_status(priv, pstat->hwaddr, pstat->rssi, RSN_MIC_failure);
#endif
#ifdef ERR_ACCESS_CNTR
				{
					int i = 0, found = 0;
					for (i=0; i<MAX_ERR_ACCESS_CNTR; i++) {
						if (priv->err_ac_list[i].used) {
							if (!memcmp(pstat->hwaddr, priv->err_ac_list[i].mac, MACADDRLEN)) {
								priv->err_ac_list[i].num++;
								found++;
								break;
							}
						}
						else
							break;
					}
					if (!found && (i != MAX_ERR_ACCESS_CNTR)) {
						priv->err_ac_list[i].used = TRUE;
						memcpy(priv->err_ac_list[i].mac, pstat->hwaddr, MACADDRLEN);
						priv->err_ac_list[i].num++;
					}
				}
#endif
				/* Refine, retry 4-3 instead dropping connect directly
				ToDrv_DisconnectSTA(priv, pstat, RSN_MIC_failure);
				pStaInfo->state = PSK_STATE_IDLE;
				*/
				break;
			}
			pStaInfo->resendCnt = 0;
			/* Refine, relocate cancel timer here to make 4-3 retried while 4-4 MIC error */
			// delete resend timer
			SMP_LOCK_PSK_RESEND(flags);
			if (timer_pending(&pStaInfo->resendTimer)){
				del_timer(&pStaInfo->resendTimer);
			}
			SMP_UNLOCK_PSK_RESEND(flags);
			LOG_MSG("Open and authenticated\n");

#if defined(CONFIG_RTL8196B_TR) || defined(CONFIG_RTL865X_SC) || defined(CONFIG_RTL865X_AC) || defined(CONFIG_RTL865X_KLD) || defined(CONFIG_RTL8196B_KLD) || defined(CONFIG_RTL8196C_KLD) || defined(CONFIG_RTL8196C_EC)
			LOG_MSG_NOTICE("Authentication Success;note:%02x-%02x-%02x-%02x-%02x-%02x;\n",
						   pstat->hwaddr[0],
						   pstat->hwaddr[1],
						   pstat->hwaddr[2],
						   pstat->hwaddr[3],
						   pstat->hwaddr[4],
						   pstat->hwaddr[5]);
#endif
#ifdef STA_ASSOC_STATISTIC
			add_sta_assoc_status(priv, pstat->hwaddr, pstat->rssi, 0);
#endif
#ifdef RTK_WLAN_EVENT_INDICATE
			rtk_wlan_event_indicate(priv->dev->name, WIFI_CONNECT_SUCCESS, pstat->hwaddr, 0);
#endif
//#ifdef RTL_WPA2_PREAUTH
#if 0
			// update ANonce for next 4-way handshake
			SetNonce(akm_sm->ANonce, global->auth->Counter);
#endif

			//MLME-SETKEYS.request
			INCLargeInteger(&pStaInfo->CurrentReplayCounter);
			// kenny: n+2
			INCLargeInteger(&pStaInfo->CurrentReplayCounter);

			ToDrv_SetPTK(priv, pstat);
			if (pStaInfo->isGuest)
				ToDrv_SetPort(priv, pstat, DOT11_PortStatus_Guest);
			else
				ToDrv_SetPort(priv, pstat, DOT11_PortStatus_Authorized);


// david+2006-04-04, fix the issue of re-generating group key, and need not
// update group key in WPA2
//			pGblInfo->GInitAKeys = TRUE;
#ifdef RTL_WPA2
			if (!(pStaInfo->RSNEnabled & PSK_WPA2))
#endif
				pStaInfo->PInitAKeys = TRUE;
			pStaInfo->state = PSK_STATE_PTKINITDONE;
			pStaInfo->gstate = PSK_GSTATE_REKEYNEGOTIATING;

			//lib1x_akmsm_UpdateGK_proc() calls lib1x_akmsm_SendEAPOL_proc for 2-way
			//if group key sent is needed, send msg 1 of 2-way handshake
#ifdef RTL_WPA2
			if (pStaInfo->RSNEnabled & PSK_WPA2) {
				//------------------------------------------------------
				// Only when the group state machine is in the state of
				// (1) The first STA Connected,
				// (2) UPDATE GK to all station
				// does the GKeyDoneStations needed to be decreased
				//------------------------------------------------------

				if (pGblInfo->GKeyDoneStations > 0)
					pGblInfo->GKeyDoneStations--;

				//Avaya akm_sm->TimeoutCtr = 0;
				//To Do : set port secure to driver
//				global->portSecure = TRUE;
				//akm_sm->state = akmsm_PTKINITDONE;

				pStaInfo->gstate = PSK_GSTATE_REKEYESTABLISHED;

				if (pGblInfo->GKeyDoneStations == 0 && !pGblInfo->GkeyReady) {
					ToDrv_SetGTK(priv);
#ifdef CONFIG_IEEE80211W
					if (priv->pmib->dot1180211AuthEntry.dot11IEEE80211W != NO_MGMT_FRAME_PROTECTION)
						ToDrv_SetIGTK(priv);
#endif
					pGblInfo->GkeyReady = TRUE;
					pGblInfo->GResetCounter = TRUE;

					// start groupkey rekey timer
					if (priv->pmib->dot1180211AuthEntry.dot11GKRekeyTime)
						mod_timer(&pGblInfo->GKRekeyTimer, jiffies + RTL_SECONDS_TO_JIFFIES(priv->pmib->dot1180211AuthEntry.dot11GKRekeyTime));
				}
#if 0
				if (global->RSNVariable.PMKCached) {
					global->RSNVariable.PMKCached = FALSE;  // reset
				}
#endif
#ifdef CONFIG_IEEE80211W
				ToDrv_SetPMF(priv, pstat);
#endif

				DEBUG_INFO("\nWPA2: 4-way handshake done\n");
				PSKDEBUG("WPA2: 4-way handshake done\n");

			}
#endif	// RTL_WPA2

			// start unicast key rekey timer
			if (priv->pmib->dot1180211AuthEntry.dot11UKRekeyTime)
				pstat->uk_timeout = priv->pmib->dot1180211AuthEntry.dot11UKRekeyTime;

			if (!Message_Secure(pStaInfo->EapolKeyMsgRecvd))
				UpdateGK(priv); // send 2-1
			break;

		case PSK_STATE_PTKINITDONE:
			// delete resend timer
			SMP_LOCK_PSK_RESEND(flags);
			if (timer_pending(&pStaInfo->resendTimer)) {
				del_timer(&pStaInfo->resendTimer);
			}
			SMP_UNLOCK_PSK_RESEND(flags);

#if 0
			//receive message [with request bit set]
			if (Message_Request(global->EapolKeyMsgRecvd))
				//supp request to initiate 4-way handshake
			{

			}
#endif

			//------------------------------------------------
			// Supplicant request to init 4 or 2 way handshake
			//------------------------------------------------
			if (Message_Request(pStaInfo->EapolKeyMsgRecvd)) {
				pStaInfo->state = PSK_STATE_PTKSTART;
				if (Message_KeyType(pStaInfo->EapolKeyMsgRecvd) == type_Pairwise) {
					if (Message_Error(pStaInfo->EapolKeyMsgRecvd))
						IntegrityFailure(priv);
					else {
						if (Message_KeyType(pStaInfo->EapolKeyMsgRecvd) == type_Group) {
							if (Message_Error(pStaInfo->EapolKeyMsgRecvd)) {
								//auth change group key, initilate 4-way handshake with supp and execute
								//the Group key handshake to all Supplicants
								pGblInfo->GKeyFailure = TRUE;
								IntegrityFailure(priv);
							}
						}
					}
					//---- Start 4-way handshake ----
					pStaInfo->resendCnt = 0;
					SendEAPOL(priv, pstat, 0);
				}
//#ifdef RTL_WPA2_PREAUTH
//				printf("kenny: %s() in akmsm_PTKINITDONE state. Call lib1x_akmsm_UpdateGK_proc()\n", (char *)__FUNCTION__);
//#endif
				//---- Execute Group Key state machine for each STA ----
				UpdateGK(priv);
			} else {
			}

			break;
		default:
			break;

		}//switch

	} else if (Message_KeyType(pStaInfo->EapolKeyMsgRecvd) == type_Group) {

		// delete resend timer
		SMP_LOCK_PSK_RESEND(flags);
		if (timer_pending(&pStaInfo->resendTimer)) {
			del_timer(&pStaInfo->resendTimer);
		}
		SMP_UNLOCK_PSK_RESEND(flags);

		//---- Receive 2nd message of 2-way handshake ----
		DEBUG_INFO("2-2\n");
		PSKDEBUG("2-2\n");
		if (!Message_Request(pStaInfo->EapolKeyMsgRecvd)) {//2nd message of 2-way handshake
			//verify that replay counter maches one it has used in the Group Key handshake
			if (!Message_EqualReplayCounter(pStaInfo->LatestGKReplayCounter, pStaInfo->EapolKeyMsgRecvd)) {
				DEBUG_ERR("ERROR_LARGER_REPLAYCOUNTER\n");
#if defined(CONFIG_WLAN_STATS_EXTENTION)
				priv->ext_stats.total_psk_fail++;
#endif
				return;
			}
			if (!CheckMIC(pStaInfo->EAPOLMsgRecvd, pStaInfo->PTK, PTK_LEN_EAPOLMIC)) {
				DEBUG_ERR("ERROR_MIC_FAIL\n");

				LOG_MSG("2-way handshake failled! (2-2: MIC error)\n");
#if defined(CONFIG_WLAN_STATS_EXTENTION)
				priv->ext_stats.total_mic_fail++;
				priv->ext_stats.total_psk_fail++;
#endif

#if defined(CONFIG_RTL8196B_TR) || defined(CONFIG_RTL865X_SC) || defined(CONFIG_RTL865X_AC) || defined(CONFIG_RTL865X_KLD) || defined(CONFIG_RTL8196B_KLD) || defined(CONFIG_RTL8196C_KLD) || defined(CONFIG_RTL8196C_EC)
				LOG_MSG_NOTICE("Authentication failed;note:%02x-%02x-%02x-%02x-%02x-%02x;\n",
							   pstat->hwaddr[0],
							   pstat->hwaddr[1],
							   pstat->hwaddr[2],
							   pstat->hwaddr[3],
							   pstat->hwaddr[4],
							   pstat->hwaddr[5]);
#endif
#ifdef RTK_WLAN_EVENT_INDICATE
				rtk_wlan_event_indicate(priv->dev->name, WIFI_CONNECT_FAIL, pstat->hwaddr, RSN_MIC_failure);
#endif
#ifdef PROC_STA_CONN_FAIL_INFO
				{
					int i;
					for (i=0; i<64; i++) {
						if (!priv->sta_conn_fail[i].used) {
							priv->sta_conn_fail[i].used = TRUE;
							priv->sta_conn_fail[i].error_state = RSN_MIC_failure;
							memcpy(priv->sta_conn_fail[i].addr, pstat->hwaddr, MACADDRLEN);
							break;
						}
						else {
							if (!memcmp(priv->sta_conn_fail[i].addr, pstat->hwaddr, MACADDRLEN)) {
								priv->sta_conn_fail[i].error_state = RSN_MIC_failure;
								break;
							}
						}
					}
				}
#endif
#ifdef STA_ASSOC_STATISTIC
				add_sta_assoc_status(priv, pstat->hwaddr, pstat->rssi, RSN_MIC_failure);
#endif
#ifdef ERR_ACCESS_CNTR
				{
					int i = 0, found = 0;
					for (i=0; i<MAX_ERR_ACCESS_CNTR; i++) {
						if (priv->err_ac_list[i].used) {
							if (!memcmp(pstat->hwaddr, priv->err_ac_list[i].mac, MACADDRLEN)) {
								priv->err_ac_list[i].num++;
								found++;
								break;
							}
						}
						else
							break;
					}
					if (!found && (i != MAX_ERR_ACCESS_CNTR)) {
						priv->err_ac_list[i].used = TRUE;
						memcpy(priv->err_ac_list[i].mac, pstat->hwaddr, MACADDRLEN);
						priv->err_ac_list[i].num++;
					}
				}
#endif

				ToDrv_DisconnectSTA(priv, pstat, RSN_MIC_failure);
				pStaInfo->state = PSK_STATE_IDLE;
				return;
			}
		} else //if(!Message_Request(global->EapolKeyMsgRecvd))
			//supp request to change group key
		{
		}

		//------------------------------------------------------
		// Only when the group state machine is in the state of
		// (1) The first STA Connected,
		// (2) UPDATE GK to all station
		// does the GKeyDoneStations needed to be decreased
		//------------------------------------------------------

		if (pGblInfo->GKeyDoneStations > 0)
			pGblInfo->GKeyDoneStations--;

		//Avaya akm_sm->TimeoutCtr = 0;
		//To Do : set port secure to driver
		pStaInfo->gstate = PSK_GSTATE_REKEYESTABLISHED;

		if (pGblInfo->GKeyDoneStations == 0 && !pGblInfo->GkeyReady) {
			ToDrv_SetGTK(priv);
			DEBUG_INFO("2-way Handshake is finished\n");
			PSKDEBUG("2-way Handshake is finished\n");
			pGblInfo->GkeyReady = TRUE;
			pGblInfo->GResetCounter = TRUE;

			// start groupkey rekey timer
			if (priv->pmib->dot1180211AuthEntry.dot11GKRekeyTime)
				mod_timer(&pGblInfo->GKRekeyTimer, jiffies + RTL_SECONDS_TO_JIFFIES(priv->pmib->dot1180211AuthEntry.dot11GKRekeyTime));
		} else {
			DEBUG_INFO(" Receive bad group key handshake");
			PSKDEBUG(" Receive bad group key handshake");
		}
	}
}

#ifdef CLIENT_MODE
static void ClientEAPOLKeyRecvd(struct rtl8192cd_priv *priv, struct stat_info *pstat)
{
	WPA_GLOBAL_INFO *pGblInfo = priv->wpa_global_info;
	WPA_STA_INFO *pStaInfo = pstat->wpa_sta_info;
	LARGE_INTEGER recievedRC;
	struct lib1x_eapol *eapol;
	int toSetKey = 0, error_code = 0;
	OCTET_STRING tmpANonce;

	eapol = ( struct lib1x_eapol * ) ( pstat->wpa_sta_info->EAPOLMsgRecvd.Octet + ETHER_HDRLEN );
	if (eapol->packet_type != LIB1X_EAPOL_KEY) {
#ifdef DEBUG_PSK
		printk("Not Eapol-key pkt (type %d), drop\n", eapol->packet_type);
#endif
		return;
	}

	pStaInfo->EapolKeyMsgRecvd.Octet = pStaInfo->EAPOLMsgRecvd.Octet + ETHER_HDRLEN + LIB1X_EAPOL_HDRLEN;

#ifdef DEBUG_PSK
	debug_out(NULL, pStaInfo->EAPOLMsgRecvd.Octet, pStaInfo->EAPOLMsgRecvd.Length);
#endif

	if (Message_KeyType(pStaInfo->EapolKeyMsgRecvd) == type_Pairwise) {

		if (Message_KeyMIC(pStaInfo->EapolKeyMsgRecvd) == FALSE) {
			//---- Receive 1st message and send 2nd
			DEBUG_INFO("client mode 4-1\n");
			PSKDEBUG("client mode 4-1\n");

			SMP_LOCK_PSK_RESEND(flags);
			if (timer_pending(&pStaInfo->resendTimer)) {
				del_timer(&pStaInfo->resendTimer);
			}
			SMP_UNLOCK_PSK_RESEND(flags);

			if (pStaInfo->clientHndshkDone && pStaInfo->clientHndshkProcessing == fourWaystep4) {
				DEBUG_INFO("AP trigger pairwise rekey\n");
				PSKDEBUG("AP trigger pairwise rekey\n");

				//reset the client info-------------------------------------------------------
				pStaInfo->CurrentReplayCounter.field.HighPart = 0xffffffff;
				pStaInfo->CurrentReplayCounter.field.LowPart = 0xffffffff;

				pStaInfo->clientHndshkProcessing = FALSE;
				pStaInfo->clientHndshkDone = FALSE;

				pGblInfo->GkeyReady = FALSE;

				//-------------------------------------------------------reset the client info
			}

			//check replay counter
			if (!Message_DefaultReplayCounter(pStaInfo->CurrentReplayCounter) &&
					Message_SmallerEqualReplayCounter(pStaInfo->CurrentReplayCounter, pStaInfo->EapolKeyMsgRecvd) ) {
				DEBUG_ERR("client mode 4-1: ERROR_NONEEQUL_REPLAYCOUNTER\n");
				error_code = _STATS_OUT_OF_AUTH_SEQ_;
				goto error_reconn;
			}


			//set wpa_sta_info parameter----------------------------------------------------
            pstat->wpa_sta_info->RSNEnabled = priv->pmib->dot1180211AuthEntry.dot11EnablePSK;
            if (pstat->wpa_sta_info->RSNEnabled == 1) {
				if (priv->pmib->dot1180211AuthEntry.dot11WPACipher == 2)
					pstat->wpa_sta_info->UnicastCipher = 0x2;
				else if (priv->pmib->dot1180211AuthEntry.dot11WPACipher == 8)
					pstat->wpa_sta_info->UnicastCipher = 0x4;
                else
                    printk("unicastcipher in wpa = nothing\n");
			} else if (pstat->wpa_sta_info->RSNEnabled == 2) {
				if (priv->pmib->dot1180211AuthEntry.dot11WPA2Cipher == 2)
					pstat->wpa_sta_info->UnicastCipher = 0x2;
				else if (priv->pmib->dot1180211AuthEntry.dot11WPA2Cipher == 8)
					pstat->wpa_sta_info->UnicastCipher = 0x4;
                else
                    printk("unicastcipher in wpa = nothing\n");
			}

	#ifdef CONFIG_IEEE80211W_CLI
			if(Message_KeyDescVer(pStaInfo->EapolKeyMsgRecvd) == key_desc_ver3)
				pStaInfo->AuthKeyMethod = DOT11_AuthKeyType_PSK_SHA256;
	#endif


			//----------------------------------------------------set wpa_sta_info parameter

			INCOctet32_INTEGER(&pGblInfo->Counter);
			SetNonce(pStaInfo->SNonce, pGblInfo->Counter);

			//pStaInfo->ANonce = Message_KeyNonce(pStaInfo->EapolKeyMsgRecvd);
			tmpANonce = Message_KeyNonce(pStaInfo->EapolKeyMsgRecvd);
			memcpy(pStaInfo->ANonce.Octet, tmpANonce.Octet, KEY_NONCE_LEN);
			pStaInfo->ANonce.Length = tmpANonce.Length;
			CalcPTK(pStaInfo->EAPOLMsgRecvd.Octet, pStaInfo->EAPOLMsgRecvd.Octet + 6,
					pStaInfo->ANonce.Octet, pStaInfo->SNonce.Octet,
					pStaInfo->PMK, PMK_LEN, pStaInfo->PTK
					, (pStaInfo->AuthKeyMethod == DOT11_AuthKeyType_PSK_SHA256) ? 48 : PTK_LEN_TKIP
#ifdef CONFIG_IEEE80211W
					, (pStaInfo->AuthKeyMethod == DOT11_AuthKeyType_PSK_SHA256)
#endif
				   );

#ifdef DEBUG_PSK
			debug_out("PSK: Generated PTK=", pStaInfo->PTK, PTK_LEN_TKIP);
#endif
			/* Refine, make 4-2 only be sent 5 times
			   to prevent MAYBE some AP keep sending out 4-1 because of incorrect PSK */
			pStaInfo->clientHndshkProcessing = fourWaystep1;
			ResendTimeout(pstat);
			//ClientSendEAPOL(priv, pstat, 0);	// send msg 2
		} else {
			//---- Receive 3rd message ----
			DEBUG_INFO("client mode 4-3\n");
			PSKDEBUG("client mode 4-3\n");

			if (!pStaInfo->clientHndshkProcessing) {
				DEBUG_ERR("client mode 4-3: ERROR_MSG_1_ABSENT\n");
				error_code = _STATS_OUT_OF_AUTH_SEQ_;
				goto error_reconn;
			}

			pStaInfo->resendCnt = 0;
			pStaInfo->clientHndshkProcessing=fourWaystep3;

			// delete resend timer
			SMP_LOCK_PSK_RESEND(flags);
			if (timer_pending(&pStaInfo->resendTimer)) {
				del_timer(&pStaInfo->resendTimer);
			}
			SMP_UNLOCK_PSK_RESEND(flags);

			Message_ReplayCounter_OC2LI(pStaInfo->EapolKeyMsgRecvd, &recievedRC);
			if (!Message_DefaultReplayCounter(pStaInfo->CurrentReplayCounter) &&
					Message_SmallerEqualReplayCounter(pStaInfo->CurrentReplayCounter, pStaInfo->EapolKeyMsgRecvd) ) {
				DEBUG_ERR("client mode 4-3: ERROR_NONEEQUL_REPLAYCOUNTER\n");
				error_code = _STATS_OUT_OF_AUTH_SEQ_;
				goto error_reconn;
			} else if (!Message_EqualKeyNonce(pStaInfo->EapolKeyMsgRecvd, pStaInfo->ANonce)) {
				DEBUG_ERR("client mode 4-3: ANonce not equal\n");
				error_code = _STATS_OUT_OF_AUTH_SEQ_;
				goto error_reconn;
			} else if (!CheckMIC(pStaInfo->EAPOLMsgRecvd, pStaInfo->PTK, PTK_LEN_EAPOLMIC)) {
				DEBUG_ERR("client mode 4-3: PTK MIC ERROR\n");
				LOG_MSG("Authentication failled! (4-3: MIC error)\n");
				error_code = _RSON_MIC_FAILURE_;
				goto error_reconn;
			}

			pStaInfo->CurrentReplayCounter.field.HighPart = recievedRC.field.HighPart;
			pStaInfo->CurrentReplayCounter.field.LowPart = recievedRC.field.LowPart;

			if ((pStaInfo->RSNEnabled & PSK_WPA2) && (Message_DescType(pStaInfo->EapolKeyMsgRecvd) == desc_type_WPA2)) {
				unsigned char decrypted_data[128];
				unsigned char GTK_KDE_OUI[] = { 0x00, 0x0F, 0xAC, 0x01 };
				unsigned char WPA_IE_OUI[] = { 0x00, 0x50, 0xF2, 0x01 };
				unsigned char *pGTK_KDE;
				unsigned char *pKeyData;
				unsigned short keyDataLength;

				keyDataLength = Message_KeyDataLength(pStaInfo->EapolKeyMsgRecvd);
				pKeyData = pStaInfo->EapolKeyMsgRecvd.Octet + KeyDataPos;

				if(!DecWPA2KeyData(pStaInfo, pKeyData, keyDataLength, pStaInfo->PTK + PTK_LEN_EAPOLMIC, PTK_LEN_EAPOLENC, decrypted_data))
				{
					DEBUG_ERR("client mode 4-3: ERROR_AESKEYWRAP_MIC_FAIL\n");
					LOG_MSG("Authentication failled! (4-3: ERROR_AESKEYWRAP_MIC_FAIL)\n");
					error_code = _RSON_MIC_FAILURE_;
					goto error_reconn;
				}

				//wpa2_hexdump("4-3 KeyData (Decrypted)",decrypted_data,keyDataLength);
				if ( decrypted_data[0] == WPA2_ELEMENT_ID) {
					pGTK_KDE = &decrypted_data[2] + (unsigned char)decrypted_data[1];
					if ( *pGTK_KDE == WPA2_ELEMENT_ID ) {
						// The second optional RSNIE is present
						DEBUG_ERR("client mode 4-3: The second optional RSNIE is present! Cannot handle it yet!\n");
						error_code=__STATS_INVALID_IE_;
						goto error_reconn;
					} else if ( *pGTK_KDE == WPA_ELEMENT_ID ) {
						// if contain RSN IE, skip it
						if (!memcmp((pGTK_KDE + 2), WPA_IE_OUI, sizeof(WPA_IE_OUI)))
							pGTK_KDE += (unsigned char) * (pGTK_KDE + 1) + 2;

						if (!memcmp((pGTK_KDE + 2), GTK_KDE_OUI, sizeof(GTK_KDE_OUI))) {
							// GTK Key Data Encapsulation Format
							unsigned char gtk_len = (unsigned char) * (pGTK_KDE + 1) - 6;
							unsigned char keyID = (unsigned char) * (pGTK_KDE + 6) & 0x03;
							pGblInfo->GN = keyID;
							memcpy(pGblInfo->GTK[keyID], (pGTK_KDE + 8), gtk_len);
							toSetKey = 1;
							pGblInfo->GkeyReady = TRUE;
						#ifdef CONFIG_IEEE80211W_CLI
						if(priv->bss_support_pmf){
							unsigned char IGTK_KDE_TYPE[] = {0xDD, 0x1C, 0x00, 0x0F, 0xAC, 0x09};

							if(!memcmp((pGTK_KDE+8+gtk_len), IGTK_KDE_TYPE, sizeof(IGTK_KDE_TYPE))){
								#define IGTK_HEADER_LEN 8
								PMFDEBUG("IGTK KDE Exists \n");
								memcpy(priv->pmib->dot11IGTKTable.dot11EncryptKey.dot11TTKey.skey,
										(pGTK_KDE+8+gtk_len+sizeof(IGTK_KDE_TYPE)+IGTK_HEADER_LEN), IGTK_LEN);
							#ifdef CONFIG_IEEE80211W_CLI_DEBUG
								panic_printk("IGTK=");
								int idx;
								for(idx=0;idx<IGTK_LEN;idx++)
									panic_printk("%02x",priv->pmib->dot11IGTKTable.dot11EncryptKey.dot11TTKey.skey[idx]);
								panic_printk("\n");
							#endif
							}else
								PMFDEBUG("IGTK KDE Not Exists \n");
						}
						#endif
						}
					}

					//check AP's RSNIE and set Group Key Chiper
					if (decrypted_data[7] == _TKIP_PRIVACY_)
						pGblInfo->MulticastCipher = _TKIP_PRIVACY_ ;
					else if (decrypted_data[7] == _CCMP_PRIVACY_)
						pGblInfo->MulticastCipher = _CCMP_PRIVACY_ ;
				}
			} else if ((pStaInfo->RSNEnabled & PSK_WPA) && (Message_DescType(pStaInfo->EapolKeyMsgRecvd) == desc_type_RSN)) {
				unsigned char WPAkeyData[255];
				unsigned short DataLength;
				memset(WPAkeyData, 0, 255);
				DataLength = Message_KeyDataLength(pStaInfo->EapolKeyMsgRecvd);
				memcpy(WPAkeyData, pStaInfo->EapolKeyMsgRecvd.Octet + KeyDataPos, 255);

				//check AP's RSNIE and set Group Key Chiper
				if (WPAkeyData[11] == _TKIP_PRIVACY_)
					pGblInfo->MulticastCipher = _TKIP_PRIVACY_ ;
				else if (WPAkeyData[11] == _CCMP_PRIVACY_)
					pGblInfo->MulticastCipher = _CCMP_PRIVACY_ ;
			}

#ifdef RTK_WLAN_EVENT_INDICATE
			if (pStaInfo->clientHndshkProcessing && !pStaInfo->clientHndshkDone){
				rtk_wlan_event_indicate(priv->dev->name, WIFI_CONNECT_SUCCESS, pstat->hwaddr, 0);
			}
#endif

			/* Refine, make 4-4 only be sent 5 times
			   to prevent MAYBE some AP keep sending out 4-3 because of incorrect PSK */
			ResendTimeout(pstat);
			//ClientSendEAPOL(priv, pstat, 0);	// send msg 4

			if (toSetKey) {
				ToDrv_SetGTK(priv);
				toSetKey = 0;
			}
			ToDrv_SetPTK(priv, pstat);
			ToDrv_SetPort(priv, pstat, DOT11_PortStatus_Authorized);
#ifdef CONFIG_IEEE80211W_CLI
			if(priv->bss_support_pmf)
				pstat->wpa_sta_info->mgmt_frame_prot = 1;
			else
				pstat->wpa_sta_info->mgmt_frame_prot = 0;

			ToDrv_SetPMF(priv,pstat);
#endif
		}
	} else if (Message_KeyType(pStaInfo->EapolKeyMsgRecvd) == type_Group) {

		unsigned char decrypted_data[128];
		unsigned char GTK_KDE_OUI[] = { 0x00, 0x0F, 0xAC, 0x01 };
		unsigned char *pGTK_KDE;
		unsigned char keyID;

		//---- Receive 1st message of 2-way handshake ----
		DEBUG_INFO("client mode receive 2-1\n");
		PSKDEBUG("client mode receive 2-1\n");

		pStaInfo->resendCnt = 0;

		Message_ReplayCounter_OC2LI(pStaInfo->EapolKeyMsgRecvd, &recievedRC);

		if (Message_SmallerEqualReplayCounter(pStaInfo->CurrentReplayCounter, pStaInfo->EapolKeyMsgRecvd) ) {
			DEBUG_ERR("client mode 2-1: ERROR_NONEEQUL_REPLAYCOUNTER\n");
			error_code = _STATS_OUT_OF_AUTH_SEQ_;
			goto error_reconn;
		} else if (!CheckMIC(pStaInfo->EAPOLMsgRecvd, pStaInfo->PTK, PTK_LEN_EAPOLMIC)) {
			DEBUG_ERR("client mode 2-1: ERROR_MIC_FAIL\n");
			LOG_MSG("Authentication failled! (4-2: MIC error)\n");
#if defined(CONFIG_RTL8186_KB_N)|| defined(CONFIG_AUTH_RESULT)
			priv->authRes = 1;//Auth failed
#endif
			error_code = _STATS_OUT_OF_AUTH_SEQ_;
			goto error_reconn;
		} else if (!DecGTK(pStaInfo->EAPOLMsgRecvd, pStaInfo->PTK + PTK_LEN_EAPOLMIC, PTK_LEN_EAPOLENC,
						   ((pGblInfo->MulticastCipher == DOT11_ENC_TKIP) ? 32 : 16),
						   pGblInfo->GTK[Message_KeyIndex(pStaInfo->EapolKeyMsgRecvd)])) {
			DEBUG_ERR("client mode 2-1: ERROR_AESKEYWRAP_MIC_FAIL\n");
			error_code = _RSON_MIC_FAILURE_;
			goto error_reconn;
		}

		keyID = Message_KeyIndex(pStaInfo->EapolKeyMsgRecvd);
		if ((pStaInfo->RSNEnabled & PSK_WPA2) && (Message_DescType(pStaInfo->EapolKeyMsgRecvd) == desc_type_WPA2)) {
			memcpy(decrypted_data, pGblInfo->GTK[keyID], Message_KeyDataLength(pStaInfo->EapolKeyMsgRecvd));
			pGTK_KDE = decrypted_data;
			if ( *pGTK_KDE == WPA_ELEMENT_ID && !memcmp((pGTK_KDE + 2), GTK_KDE_OUI, sizeof(GTK_KDE_OUI))) {
				// GTK Key Data Encapsulation Format
				unsigned char gtk_len = (unsigned char) * (pGTK_KDE + 1) - 6;
				keyID = (unsigned char) * (pGTK_KDE + 6) & 0x03;

				if(keyID == pGblInfo->GN && !memcmp(pGblInfo->GTK[keyID], (pGTK_KDE + 8), gtk_len)) {
					PSKDEBUG("the same keydata, don't reinstall again\n");
					pStaInfo->clientGkeyUpdate = 1;
					ClientSendEAPOL(priv, pstat, 0); // send msg 2-1
					return;
				}

				pGblInfo->GN = keyID;
				memcpy(pGblInfo->GTK[keyID], (pGTK_KDE + 8), gtk_len);
			}
		} else {
			pGblInfo->GN = keyID;
		}
		//MLME_SETKEYS.request() to set Group Key;
		pGblInfo->GkeyReady = TRUE;

		pStaInfo->CurrentReplayCounter.field.HighPart = recievedRC.field.HighPart;
		pStaInfo->CurrentReplayCounter.field.LowPart = recievedRC.field.LowPart;
		pStaInfo->clientGkeyUpdate = 1;

		ToDrv_SetGTK(priv);
		ClientSendEAPOL(priv, pstat, 0); // send msg 2-1
	} else {
		printk("Client EAPOL Key Receive ERROR!!\n");
		error_code = _RSON_UNSPECIFIED_;
	}

error_reconn:
	if (error_code) {
		unsigned int opmode = OPMODE;
		if (ACTIVE_ID == 0) {
			priv->dot114WayStatus = error_code;
		}
		issue_deauth(priv, BSSID, error_code);
		free_stainfo(priv, pstat);
		opmode &= ~(WIFI_AUTH_SUCCESS | WIFI_ASOC_STATE);
		OPMODE_VAL(opmode);
		JOIN_RES_VAL(STATE_Sta_No_Bss);
		start_clnt_lookup(priv, RESCAN);
	}
}
#endif // CLIENT_MODE


void derivePSK(struct rtl8192cd_priv *priv)
{
	WPA_GLOBAL_INFO *pGblInfo = priv->wpa_global_info;
	//unsigned long x;
	//SAVE_INT_AND_CLI(x);

#if 1//backup derivePSK
	if(!priv->is_PSK_backup){
		memset(priv->Backup_PSK, 0x0, sizeof(priv->Backup_PSK));
		memset(priv->Backup_PassPhrase, 0x0, sizeof(priv->Backup_PassPhrase));
		memset(priv->Backup_SSID, 0x0, sizeof(priv->Backup_SSID));
		priv->Backup_SSIDLen = 0;
	}else{
		if(!memcmp(SSID, priv->Backup_SSID, sizeof(priv->Backup_SSID)) &&
			SSID_LEN==priv->Backup_SSIDLen &&
			!memcmp(priv->Backup_PassPhrase, priv->pmib->dot1180211AuthEntry.dot11PassPhrase, sizeof(priv->Backup_PassPhrase))){
			//panic_printk(" ### hit PSK backup data!!!\n");
			memcpy(pGblInfo->PSK, priv->Backup_PSK, sizeof(priv->Backup_PSK));
			return;
		}else{
			//panic_printk(" ### update PSK backup data!!!\n");
			priv->is_PSK_backup = 0;
			memset(priv->Backup_PSK, 0x0, sizeof(priv->Backup_PSK));
			memset(priv->Backup_PassPhrase, 0x0, sizeof(priv->Backup_PassPhrase));
			memset(priv->Backup_SSID, 0x0, sizeof(priv->Backup_SSID));
			priv->Backup_SSIDLen = 0;
		}
	}
#endif

	if (strlen((char *)priv->pmib->dot1180211AuthEntry.dot11PassPhrase) == 64) // hex
		get_array_val(pGblInfo->PSK, (char *)priv->pmib->dot1180211AuthEntry.dot11PassPhrase, 64);
	else
		PasswordHash((char *)priv->pmib->dot1180211AuthEntry.dot11PassPhrase, SSID, SSID_LEN,
					 pGblInfo->PSK);

	if (priv->pmib->dot1180211AuthEntry.dot11PassPhraseGuest[0]) {
		if (strlen((char *)priv->pmib->dot1180211AuthEntry.dot11PassPhraseGuest) == 64)
			get_array_val(pGblInfo->PSKGuest, (char *)priv->pmib->dot1180211AuthEntry.dot11PassPhraseGuest, 64);
		else
			PasswordHash((char *)priv->pmib->dot1180211AuthEntry.dot11PassPhraseGuest, SSID, SSID_LEN,
						 pGblInfo->PSKGuest);
	}

#if 1//backup derivePSK
	//panic_printk(" ### backup PSK data\n");
	memcpy(priv->Backup_SSID, SSID, sizeof(priv->Backup_SSID));
	priv->Backup_SSIDLen = SSID_LEN;
	memcpy(priv->Backup_PassPhrase, priv->pmib->dot1180211AuthEntry.dot11PassPhrase, sizeof(priv->Backup_PassPhrase));
	memcpy(priv->Backup_PSK, pGblInfo->PSK, sizeof(priv->Backup_PSK));
	priv->is_PSK_backup = 1;
#endif

	//RESTORE_INT(x);
}

void psk_init(struct rtl8192cd_priv *priv)
{
	WPA_GLOBAL_INFO *pGblInfo = priv->wpa_global_info;
	int i, j, low_cipher = 0;

	DEBUG_TRACE;

	memset((char *)pGblInfo, '\0', sizeof(WPA_GLOBAL_INFO));

	//---- Counter is initialized whenever boot time ----
	GenNonce(pGblInfo->Counter.charData, (unsigned char*)"addr");

	if (OPMODE & WIFI_AP_STATE) {
		//---- Initialize Goup Key state machine ----
		pGblInfo->GNonce.Octet = pGblInfo->GNonceBuf;
		pGblInfo->GNonce.Length = KEY_NONCE_LEN;
		pGblInfo->GTKAuthenticator = TRUE;
		pGblInfo->GN = 1;
		pGblInfo->GM = 2;
#ifdef CONFIG_IEEE80211W
		pGblInfo->GN_igtk = 4;
		pGblInfo->GM_igtk = 5;
#endif /* CONFIG_IEEE80211W */
		pGblInfo->GInitAKeys = TRUE; // david+2006-04-04, fix the issue of re-generating group key

		init_timer(&pGblInfo->GKRekeyTimer);
		pGblInfo->GKRekeyTimer.data = (unsigned long)priv;
		pGblInfo->GKRekeyTimer.function = GKRekeyTimeout;
	}
#if 0
	if (strlen(priv->pmib->dot1180211AuthEntry.dot11PassPhrase) == 64) // hex
		get_array_val(pGblInfo->PSK, priv->pmib->dot1180211AuthEntry.dot11PassPhrase, 64);
	else
		PasswordHash(priv->pmib->dot1180211AuthEntry.dot11PassPhrase, priv->pmib->dot11StationConfigEntry.dot11DesiredSSID,
					 pGblInfo->PSK);

	if (priv->pmib->dot1180211AuthEntry.dot11PassPhraseGuest[0]) {
		if (strlen(priv->pmib->dot1180211AuthEntry.dot11PassPhraseGuest) == 64)
			get_array_val(pGblInfo->PSKGuest, priv->pmib->dot1180211AuthEntry.dot11PassPhraseGuest, 64);
		else
			PasswordHash(priv->pmib->dot1180211AuthEntry.dot11PassPhraseGuest, priv->pmib->dot11StationConfigEntry.dot11DesiredSSID,
						 pGblInfo->PSKGuest);
	}
#else
	derivePSK(priv);
#endif
#ifdef DEBUG_PSK
	debug_out("PSK: PMK=", pGblInfo->PSK, PMK_LEN);
	if (priv->pmib->dot1180211AuthEntry.dot11PassPhraseGuest[0])
		debug_out("PSK-Guest: PMK=", pGblInfo->PSKGuest, PMK_LEN);
#endif

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
		for (i = 0, j = 0; i < _WEP_104_PRIVACY_; i++) {
			if (priv->pmib->dot1180211AuthEntry.dot11WPACipher & (1 << i)) {
				pGblInfo->UnicastCipher[j] = i + 1;
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

#ifdef CLIENT_MODE
	if ((OPMODE & WIFI_ADHOC_STATE) && (priv->pmib->dot1180211AuthEntry.dot11EnablePSK & 2)) // if WPA2
	{
		//if(priv->pmib->dot1180211AuthEntry.dot11WPA2Cipher == 8)
			//low_cipher = 0;
	}
#endif

#ifdef RTL_WPA2
	if (priv->pmib->dot1180211AuthEntry.dot11WPA2Cipher) {
		for (i = 0, j = 0; i < _WEP_104_PRIVACY_; i++) {
			if (priv->pmib->dot1180211AuthEntry.dot11WPA2Cipher & (1 << i)) {
				pGblInfo->UnicastCipherWPA2[j] = i + 1;
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
		pGblInfo->NumOfUnicastCipherWPA2 = j;
	}
#endif

	pGblInfo->MulticastCipher = low_cipher;

#ifdef DEBUG_PSK
	printk("PSK: WPA unicast cipher= ");
	for (i = 0; i < pGblInfo->NumOfUnicastCipher; i++)
		printk("%x ", pGblInfo->UnicastCipher[i]);
	printk("\n");

#ifdef RTL_WPA2
	printk("PSK: WPA2 unicast cipher= ");
	for (i = 0; i < pGblInfo->NumOfUnicastCipherWPA2; i++)
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

#ifdef SUPPORT_CLIENT_MIXED_SECURITY
void client_psk_reConstructIE(struct rtl8192cd_priv *priv)
{
	WPA_GLOBAL_INFO *pGblInfo=priv->wpa_global_info;

	ConstructIE(priv, pGblInfo->AuthInfoElement.Octet,
					 &pGblInfo->AuthInfoElement.Length);

	ToDrv_SetIE(priv);
}
#endif


#ifdef CONFIG_IEEE80211W

void ToDrv_SA_QUERY_RSP(struct rtl8192cd_priv *priv, const u8 *sa, const u8 *trans_id)
{
	struct iw_point wrq;
	DOT11_SA_QUERY_RSP resp;


	wrq.pointer = &resp;
	wrq.length = sizeof(resp);

	resp.EventId = DOT11_EVENT_SA_QUERY_RSP;
	resp.IsMoreEvent = FALSE;
	memcpy(resp.MACAddr, sa, 6);
	memcpy(resp.trans_id, trans_id, 2);

	rtl8192cd_ioctl_priv_daemonreq(priv->dev, &wrq);
}

void ieee802_11_sa_query_action(struct rtl8192cd_priv *priv, unsigned char *sa, unsigned char *pframe, int len)
{
	unsigned char Category_field = pframe[0];
	unsigned char Action_field = pframe[1];
	unsigned char trans_id[2];
	struct stat_info *pstat;

	memcpy(trans_id, pframe + 2, 2);

	if (Action_field == _SA_QUERY_REQ_ACTION_ID_) {
		printk("IEEE 802.11: Received SA Query Request from %02x:%02x:%02x:%02x:%02x:%02x\n"
			   , sa[0], sa[1], sa[2], sa[3], sa[4], sa[5]);
		printk("IEEE 802.11: SA Query Transaction ID %d\n", trans_id);

		pstat = get_stainfo(priv, sa);
		if (pstat == NULL || pstat->wpa_sta_info->state != PSK_STATE_PTKINITDONE) {
			printk("IEEE 802.11: Ignore SA Query Request from unassociated STA %02x:%02x:%02x:%02x:%02x:%02x\n"
				   , sa[0], sa[1], sa[2], sa[3], sa[4], sa[5]);
			return;
		}

		printk("IEEE 802.11: Sending SA Query Response to %02x:%02x:%02x:%02x:%02x:%02x\n"
			   , sa[0], sa[1], sa[2], sa[3], sa[4], sa[5]);
		ToDrv_SA_QUERY_RSP(priv, sa, trans_id);
		return;
	}

	if (Action_field != _SA_QUERY_RSP_ACTION_ID_) {
		printk("IEEE 802.11: Unexpected SA Query Action %d\n", Action_field);
		return;
	}


}

#endif  // CONFIG_IEEE80211W

int psk_indicate_evt(struct rtl8192cd_priv *priv, int id, unsigned char *mac, unsigned char *msg, int len)
{
	struct stat_info *pstat;
	static unsigned char tmpbuf[1024];
	int ret;
#ifdef RTL_WPA2
	int isWPA2 = 0;
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
	if (pstat == NULL)
#else
	if (pstat == NULL && id != DOT11_EVENT_WPA_MULTICAST_CIPHER && id != DOT11_EVENT_WPA2_MULTICAST_CIPHER)
#endif
	{
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
				memcpy(tmpbuf + 2, msg, len);
#endif

#ifdef RTL_WPA2
				isWPA2 = (tmpbuf[0] == WPA2_ELEMENT_ID) ? 1 : 0;
				if (isWPA2)
					ret = parseIEWPA2(priv, pstat->wpa_sta_info, tmpbuf, len + 2);
				else
#endif
					ret = parseIE(priv, pstat->wpa_sta_info, tmpbuf, len + 2);
				if (ret != 0) {
					DEBUG_ERR("parse IE error [%x]!\n", ret);
				}

				// issue assoc-rsp successfully
				ToDrv_RspAssoc(priv, id, mac, -ret);

				if (ret == 0) {
#ifdef EVENT_LOG
					char *pmsg;
					switch (pstat->wpa_sta_info->UnicastCipher) {
					case DOT11_ENC_NONE:
						pmsg = "none";
						break;
					case DOT11_ENC_WEP40:
						pmsg = "WEP40";
						break;
					case DOT11_ENC_TKIP:
						pmsg = "TKIP";
						break;
					case DOT11_ENC_WRAP:
						pmsg = "AES";
						break;
					case DOT11_ENC_CCMP:
						pmsg = "AES";
						break;
					case DOT11_ENC_WEP104:
						pmsg = "WEP104";
						break;
					default:
						pmsg = "invalid algorithm";
						break;
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

					AuthenticationRequest(priv, pstat); // send 4-1
				}
			} else { // RNSIE is null
				if (priv->pmib->dot1180211AuthEntry.dot11EnablePSK)
					ToDrv_RspAssoc(priv, id, mac, -ERROR_INVALID_RSNIE);
			}
		}
#ifdef  CLIENT_MODE
		else
			mod_timer(&pstat->wpa_sta_info->resendTimer, jiffies + WAIT_EAP_TIME);
#endif
		break;

	case DOT11_EVENT_DISASSOCIATION_IND:
		reset_sta_info(priv, pstat);
		break;

#ifdef CONFIG_IEEE80211W
	case DOT11_EVENT_SA_QUERY:
		ieee802_11_sa_query_action(priv, mac, msg, len);
		break;
#endif


	case DOT11_EVENT_EAP_PACKET:
		if (OPMODE & WIFI_AP_STATE) {
			if (pstat->wpa_sta_info->state == PSK_STATE_IDLE) {
				DEBUG_ERR("Rx EAPOL packet but did not get Assoc-Ind yet!\n");
				break;
			}
		}

		if (len > MAX_EAPOLMSG_LEN) {
			DEBUG_ERR("Rx EAPOL packet which length is too long [%x]!\n", len);
#if defined(CONFIG_WLAN_STATS_EXTENTION)
			priv->ext_stats.total_psk_fail++;
#endif
			break;
		}

#ifdef CLIENT_MODE
		if ((OPMODE & WIFI_STATION_STATE) &&
				!(pstat->wpa_sta_info->clientHndshkProcessing || pstat->wpa_sta_info->clientHndshkDone)) {
#ifdef EVENT_LOG
			char *pmsg;
			switch (pstat->wpa_sta_info->UnicastCipher) {
			case DOT11_ENC_NONE:
				pmsg = "none";
				break;
			case DOT11_ENC_WEP40:
				pmsg = "WEP40";
				break;
			case DOT11_ENC_TKIP:
				pmsg = "TKIP";
				break;
			case DOT11_ENC_WRAP:
				pmsg = "AES";
				break;
			case DOT11_ENC_CCMP:
				pmsg = "AES";
				break;
			case DOT11_ENC_WEP104:
				pmsg = "WEP104";
				break;
			default:
				pmsg = "invalid algorithm";
				break;
			}
			LOG_MSG("%s-%s PSK authentication in progress...\n", (isWPA2 ? "WPA2" : "WPA"), pmsg);
#endif
			reset_sta_info(priv, pstat);
		}else if(OPMODE & WIFI_ADHOC_STATE){
			DEBUG_ERR("Rx EAPOL packet but not support 4 way handshark yet on Ad-hoc mode, block it!\n");
			break;
		}
#endif
		memcpy(pstat->wpa_sta_info->EAPOLMsgRecvd.Octet, msg, len);
		pstat->wpa_sta_info->EAPOLMsgRecvd.Length = len;
		if (OPMODE & WIFI_AP_STATE)
			EAPOLKeyRecvd(priv, pstat);
#ifdef CLIENT_MODE
		else if (OPMODE & WIFI_STATION_STATE)
			ClientEAPOLKeyRecvd(priv, pstat);
#endif
		break;

	case DOT11_EVENT_MIC_FAILURE:
#ifdef CLIENT_MODE
		if (OPMODE & WIFI_STATION_STATE)
			ClientSendEAPOL(priv, pstat, 0);
#endif
		break;

// button 2009.05.21
	case DOT11_EVENT_WPA_MULTICAST_CIPHER:
	case DOT11_EVENT_WPA2_MULTICAST_CIPHER:
#ifdef CLIENT_MODE
		if (OPMODE & WIFI_STATION_STATE) {
			priv->wpa_global_info->MulticastCipher = *msg;
			ConstructIE(priv, priv->wpa_global_info->AuthInfoElement.Octet,
						&priv->wpa_global_info->AuthInfoElement.Length);
			memcpy((void *)priv->pmib->dot11RsnIE.rsnie, priv->wpa_global_info->AuthInfoElement.Octet
				   , priv->wpa_global_info->AuthInfoElement.Length);
			DEBUG_WARN("####### MulticastCipher=%d\n", priv->wpa_global_info->MulticastCipher);
		}
#endif
		break;
	}

	return 0;
}

void UKRekeyTimeout(struct rtl8192cd_priv *priv, struct stat_info *pstat)
{
	AuthenticationRequest(priv, pstat);
}






#ifdef SUPPORT_CLIENT_MIXED_SECURITY
void choose_cipher(struct rtl8192cd_priv *priv, struct bss_desc *bss_target)
{
	unsigned int t_stamp;
	int len;
	int dot11EnablePSK = 0;
	int dot11WPACipher = 0;
	int dot11WPA2Cipher = 0;
	unsigned char dot11PrivacyAlgrthm = _NO_PRIVACY_;

	t_stamp = bss_target->t_stamp[0];

	if ( bss_target->capability & BIT4 ) { /*Privacy bit*/
		//  fetch WPA2
		if ( (t_stamp & BIT18) || (t_stamp & BIT20) ) {
			dot11EnablePSK |= PSK_WPA2;
			//  pairwise cipher
			if ( t_stamp & BIT24 )
				dot11WPA2Cipher |= BIT1;
			if ( t_stamp & BIT26 )
				dot11WPA2Cipher |= BIT3;
		}

		// fetch WPA
		if ( (t_stamp & BIT2) || (t_stamp & BIT4) ) {
			dot11EnablePSK |= PSK_WPA;
			//  unicast cipher
			if ( t_stamp & BIT8 )
				dot11WPACipher |= BIT1;
			if ( t_stamp & BIT10 )
				dot11WPACipher |= BIT3;
		}
	}

	DEBUG_INFO("\ndot11EnablePSK:  %d  %d\n", priv->pmib->dot1180211AuthEntry.dot11EnablePSK, dot11EnablePSK);
	DEBUG_INFO("dot11WPACipher:  %d  %d\n", priv->pmib->dot1180211AuthEntry.dot11WPACipher, dot11WPACipher);
	DEBUG_INFO("dot11WPA2Cipher:	%d	%d\n", priv->pmib->dot1180211AuthEntry.dot11WPA2Cipher, dot11WPA2Cipher);

	if (priv->pshare->rf_ft_var.auto_cipher) {
		//  privacy enable and not exist WPA & WPA2 ie -> WEP encryption
		if ( dot11EnablePSK == 0 ) {
			len = strlen(priv->pmib->dot11DefaultKeysTable.keytype[0].skey);
			if ( len == 5 )
				dot11PrivacyAlgrthm = _WEP_40_PRIVACY_;
			else if (len == 13 )
				dot11PrivacyAlgrthm = _WEP_104_PRIVACY_;
		} else { // WPA or WPA2 encryption
			// Select the most higher security (AES > WPA2 > OTHER)
			if ( (dot11WPACipher & BIT3) ||
					(dot11WPA2Cipher & BIT3) ) {
				if (dot11WPA2Cipher & BIT3 ) {
					dot11EnablePSK = PSK_WPA2;
					dot11WPACipher = 0;
					dot11WPA2Cipher = BIT3;

				} else {
					dot11EnablePSK = PSK_WPA;
					dot11WPACipher = BIT3;
					dot11WPA2Cipher = 0;
				}

				dot11PrivacyAlgrthm = _CCMP_PRIVACY_;
			} else {
				if ( dot11EnablePSK & PSK_WPA2 ) {
					dot11EnablePSK = PSK_WPA2;
					dot11WPACipher = 0;
					dot11WPA2Cipher = BIT1;
				} else {
					dot11EnablePSK = PSK_WPA;
					dot11WPACipher = BIT1;
					dot11WPA2Cipher = 0;
				}

				dot11PrivacyAlgrthm = _TKIP_PRIVACY_;
			}
		}
	}
	else {
		if ((priv->pmib->dot1180211AuthEntry.dot11EnablePSK & dot11EnablePSK)
		&& ((priv->pmib->dot1180211AuthEntry.dot11WPACipher & dot11WPACipher)
		|| (priv->pmib->dot1180211AuthEntry.dot11WPA2Cipher & dot11WPA2Cipher))
		) {

			if ((priv->pmib->dot1180211AuthEntry.dot11WPA2Cipher & dot11WPA2Cipher) & BIT3) {
				DEBUG_INFO("\nWPA2-AES\n");
				dot11EnablePSK = PSK_WPA2;
		       		dot11WPACipher = 0;
	        		dot11WPA2Cipher = BIT3;
				dot11PrivacyAlgrthm = _CCMP_PRIVACY_;
			} else if ((priv->pmib->dot1180211AuthEntry.dot11WPACipher & dot11WPACipher) & BIT3) {
				DEBUG_INFO("\nWPA-AES\n");
				dot11EnablePSK = PSK_WPA;
		       		dot11WPACipher = BIT3;
	        		dot11WPA2Cipher = 0;
				dot11PrivacyAlgrthm = _CCMP_PRIVACY_;
			} else if ((priv->pmib->dot1180211AuthEntry.dot11WPA2Cipher & dot11WPA2Cipher) & BIT1) {
				DEBUG_INFO("\nWPA2-TKIP\n");
				dot11EnablePSK = PSK_WPA2;
		       		dot11WPACipher = 0;
	        		dot11WPA2Cipher = BIT1;
				dot11PrivacyAlgrthm = _TKIP_PRIVACY_;
			} else if ((priv->pmib->dot1180211AuthEntry.dot11WPACipher & dot11WPACipher) & BIT1) {
				DEBUG_INFO("\nWPA-TKIP\n");
				dot11EnablePSK = PSK_WPA;
		       		dot11WPACipher = BIT1;
	        		dot11WPA2Cipher = 0;
				dot11PrivacyAlgrthm = _TKIP_PRIVACY_;
			} else {
				DEBUG_INFO("No match wpa cipher!!!\n");
				return;
			}
		} else {
			DEBUG_INFO("Not wpa enable!!!\n");
			return;
		}
	}

	priv->pmib->dot1180211AuthEntry.dot11EnablePSK = dot11EnablePSK;
	priv->pmib->dot1180211AuthEntry.dot11WPACipher = dot11WPACipher;
	priv->pmib->dot1180211AuthEntry.dot11WPA2Cipher = dot11WPA2Cipher;
	priv->pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm = dot11PrivacyAlgrthm;

	DEBUG_INFO("choose_cipher: psk_enable %d wpa_cipher %d wpa2_cipher %d\n\n",
			   priv->pmib->dot1180211AuthEntry.dot11EnablePSK,
			   priv->pmib->dot1180211AuthEntry.dot11WPACipher,
			   priv->pmib->dot1180211AuthEntry.dot11WPA2Cipher);

	if (priv->pmib->dot1180211AuthEntry.dot11EnablePSK) {
		psk_init(priv);
		priv->pmib->dot118021xAuthEntry.dot118021xAlgrthm = 1;
	} else {
		priv->pmib->dot11RsnIE.rsnielen = 0;
		priv->pmib->dot118021xAuthEntry.dot118021xAlgrthm = 0;
	}

	if (should_forbid_Nmode(priv)) {
		if (priv->pmib->dot11BssType.net_work_type & WIRELESS_11N) {
#ifdef SUPPORT_MULTI_PROFILE
			if (!priv->mask_n_band)
				priv->mask_n_band = (priv->pmib->dot11BssType.net_work_type & (WIRELESS_11N | WIRELESS_11AC));
#endif
			priv->pmib->dot11BssType.net_work_type &= ~(WIRELESS_11N | WIRELESS_11AC);
		}
	}
#ifdef SUPPORT_MULTI_PROFILE
	else {
		if (priv->mask_n_band) {
			priv->pmib->dot11BssType.net_work_type |= priv->mask_n_band;
			priv->mask_n_band = 0;
		}
	}
#endif
}
#endif


#endif // INCLUDE_WPA_PSK


