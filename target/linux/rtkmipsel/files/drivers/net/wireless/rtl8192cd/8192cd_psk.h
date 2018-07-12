/*
 *  Header files of WPA PSK module
 *
 *  $Id: 8192cd_psk.h,v 1.1 2009/11/06 12:26:48 victoryman Exp $
 *
 *  Copyright (c) 2009 Realtek Semiconductor Corp.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 */

#ifndef _8192CD_PSK_H_
#define _8192CD_PSK_H_

#define GMK_LEN						32
#define GTK_LEN						32
#define PMK_LEN						32
#define PTK_LEN						32
#ifdef CONFIG_IEEE80211W
#define IGTK_LEN 					16
#endif /* CONFIG_IEEE80211W */
#define KEY_NONCE_LEN				32
#define NumGroupKey					4
#define KEY_RC_LEN					8
#define KEY_IV_LEN					16
#define KEY_RSC_LEN					8
#define KEY_ID_LEN					8
#define KEY_MIC_LEN					16
#define KEY_MATERIAL_LEN			2
#define PTK_LEN_EAPOLMIC        	16
#define PTK_LEN_EAPOLENC        	16
#define PTK_LEN_TKIP           		64
#define PMKID_LEN					16
#define LIB1X_ETHER_EAPOL_TYPE		0x888E

#define DescTypePos					0
#define KeyInfoPos					1
#define KeyLenPos					3
#define ReplayCounterPos			5
#define KeyNoncePos					13
#define KeyIVPos					45
#define KeyRSCPos					61
#define KeyIDPos					69
#define KeyMICPos					77
#define KeyDataLenPos				93
#define KeyDataPos					95
#define LIB1X_EAPOL_VER				1		//00000001B
#define LIB1X_EAPOL_LOGOFF			2       //0000 0010B
#define LIB1X_EAPOL_EAPPKT			0       //0000 0000B
#define LIB1X_EAPOL_START			1		//0000 0001B
#define LIB1X_EAPOL_KEY				3		//0000 0011B
#define LIB1X_EAPOL_ENCASFALERT		4		//0000 0100B
#ifdef CONFIG_IEEE80211W_CLI
#define KeyAKMPos					16	
#define KEY_AKM_LEN					4
#endif

#define RANDOM_EXPANSION_CONST		"Init Counter"
#define RANDOM_EXPANSION_CONST_SIZE			12
#define A_SHA_DIGEST_LEN			20
#define ETHER_HDRLEN				14
#define LIB1X_EAPOL_HDRLEN			4
#ifdef CONFIG_IEEE80211R
#define INFO_ELEMENT_SIZE       	384
#else
#define INFO_ELEMENT_SIZE       	128
#endif
#define MAX_EAPOLMSG_LEN        	512
#define MAX_EAPOLKEYMSG_LEN	(MAX_EAPOLMSG_LEN-(ETHER_HDRLEN+LIB1X_EAPOL_HDRLEN))
#define EAPOLMSG_HDRLEN				95      //EAPOL-key payload length without KeyData
#define MAX_UNICAST_CIPHER			2
#define WPA_ELEMENT_ID				0xDD
#define WPA2_ELEMENT_ID				0x30



typedef enum	{ desc_type_WPA2 = 2, desc_type_RSN = 254 } DescTypeRSN;
typedef enum	{ type_Group = 0, type_Pairwise = 1 } KeyType;
typedef enum	{ key_desc_ver1 = 1, key_desc_ver2 = 2, key_desc_ver3 = 3 } KeyDescVer;
enum { PSK_WPA=1, PSK_WPA2=2};

#ifdef TLN_STATS
enum { STATS_AUTH_OPEN=0, STATS_AUTH_SHARE=1,
		STATS_PSK_WPA=2, STATS_PSK_WPA2=3,
		STATS_ETP_WPA=4, STATS_ETP_WPA2=5
	};
#endif

enum {
	PSK_STATE_IDLE,
	PSK_STATE_PTKSTART,
	PSK_STATE_PTKINITNEGOTIATING,
	PSK_STATE_PTKINITDONE,
};

enum {
	PSK_GSTATE_REKEYNEGOTIATING,
	PSK_GSTATE_REKEYESTABLISHED,
	PSK_GSTATE_KEYERROR,
};

#ifdef CONFIG_IEEE80211W
enum mfp_options {
	NO_MGMT_FRAME_PROTECTION = 0,	
	MGMT_FRAME_PROTECTION_OPTIONAL = 1,	
	MGMT_FRAME_PROTECTION_REQUIRED = 2
};
#endif

/*
 * Reason code for Disconnect
 */
typedef enum _ReasonCode{
	unspec_reason					= 0x01,
	auth_not_valid  				= 0x02,
	deauth_lv_ss    				= 0x03,
	inactivity						= 0x04,
	ap_overload						= 0x05,
	class2_err						= 0x06,
	class3_err						= 0x07,
	disas_lv_ss						= 0x08,
	asoc_not_auth					= 0x09,
	RSN_invalid_info_element		= 13,
	RSN_MIC_failure					= 14,
	RSN_4_way_handshake_timeout	= 15,
	RSN_diff_info_element			= 17,
	RSN_multicast_cipher_not_valid	= 18,
	RSN_unicast_cipher_not_valid	= 19,
	RSN_AKMP_not_valid				= 20,
	RSN_unsupported_RSNE_version	= 21,
	RSN_invalid_RSNE_capabilities	= 22,
	RSN_ieee_802dot1x_failed		= 23,
	//belowing are Realtek definition
	RSN_PMK_not_avaliable			= 24,
	expire							= 30,
	session_timeout					= 31,
	acct_idle_timeout				= 32,
	acct_user_request				= 33
}ReasonCode;


typedef	struct _OCTET_STRING {
	unsigned char	*Octet;
	int				Length;
} OCTET_STRING, *POCTET_STRING;

typedef union _LARGE_INTEGER {
		unsigned char 	charData[8];
        struct {
			unsigned int 	HighPart;
			unsigned int 	LowPart;
        } field;
} LARGE_INTEGER, *PLARGE_INTEGER;

typedef union _OCTET16_INTEGER {
		unsigned char 	charData[16];
        struct {
			LARGE_INTEGER	HighPart;
			LARGE_INTEGER	LowPart;
        } field;
} OCTET16_INTEGER;

typedef union  _OCTET32_INTEGER {
		unsigned char	charData[32];
        struct {
			OCTET16_INTEGER	HighPart;
			OCTET16_INTEGER	LowPart;
        } field;
} OCTET32_INTEGER;

typedef struct _DOT11_WPA2_IE_HEADER {
        unsigned char ElementID;
        unsigned char Length;
        unsigned short Version;
} DOT11_WPA2_IE_HEADER;

#if defined(WIFI_HAPD) && !defined(HAPD_DRV_PSK_WPS) || defined(RTK_NL80211)
// group key info
typedef struct _wpa_global_info {
	OCTET_STRING		AuthInfoElement;
	unsigned char		AuthInfoBuf[INFO_ELEMENT_SIZE];
	unsigned char		MulticastCipher;
	int					NumOfUnicastCipher;
	unsigned char		UnicastCipher[MAX_UNICAST_CIPHER];
#ifdef RTL_WPA2
	int					NumOfUnicastCipherWPA2;
	unsigned char		UnicastCipherWPA2[MAX_UNICAST_CIPHER];
#endif
} WPA_GLOBAL_INFO;

#else

// group key info
typedef struct _wpa_global_info {
	OCTET32_INTEGER		Counter;
	unsigned char		PSK[A_SHA_DIGEST_LEN*2];
	unsigned char		PSKGuest[A_SHA_DIGEST_LEN*2];
	int					GTKAuthenticator;
	int					GKeyDoneStations;
	int					GInitAKeys;
	int					GUpdateStationKeys;
	int					GkeyReady;
	OCTET_STRING		AuthInfoElement;
	unsigned char		AuthInfoBuf[INFO_ELEMENT_SIZE];
	unsigned char		MulticastCipher;
	int					NumOfUnicastCipher;
	unsigned char		UnicastCipher[MAX_UNICAST_CIPHER];
#ifdef RTL_WPA2
	int					NumOfUnicastCipherWPA2;
	unsigned char		UnicastCipherWPA2[MAX_UNICAST_CIPHER];
#endif
	OCTET_STRING		GNonce;
	unsigned char		GNonceBuf[KEY_NONCE_LEN];
	unsigned char		GTK[NumGroupKey][GTK_LEN];
	unsigned char		GMK[GMK_LEN];
	int					GN;
	int					GM;
#ifdef CONFIG_IEEE80211W	
#ifdef CONFIG_IEEE80211W_CLI		
	unsigned short		rsnie_cap;	
#endif	
	unsigned char		IGTK[2][IGTK_LEN];	
	int					GN_igtk;	
	int 				GM_igtk;	
	union PN48 IGTK_PN;
#endif
	int					GRekeyCounts;
	int					GResetCounter;

	int					IntegrityFailed;
	int					GTKRekey;
	int					GKeyFailure;
	struct timer_list	GKRekeyTimer;
} WPA_GLOBAL_INFO;
#endif

#if defined(WIFI_HAPD) && !defined(HAPD_DRV_PSK_WPS) || defined(RTK_NL80211)
// wpa sta info
typedef struct _wpa_sta_info {
	int 				state;
	int					RSNEnabled;		// bit0-WPA, bit1-WPA2
	unsigned char		UnicastCipher;
	struct rtl8192cd_priv	*priv;
} WPA_STA_INFO;

#else

// wpa sta info
typedef struct _wpa_sta_info {
	int					state;
	int					gstate;
	int					RSNEnabled;		// bit0-WPA, bit1-WPA2
	int					PMKCached;
	int					PInitAKeys;
	unsigned char		UnicastCipher;
 	unsigned char		NumOfRxTSC;
 	unsigned char		AuthKeyMethod;
#ifdef CONFIG_IEEE80211W	
	enum mfp_options ieee80211w;	/* dot11AssociationSAQueryMaximumTimeout (in TUs) */
	unsigned int assoc_sa_query_max_timeout;	/* dot11AssociationSAQueryRetryTimeout (in TUs) */	
	int assoc_sa_query_retry_timeout;
#endif /* CONFIG_IEEE80211W */	
 	int					isSuppSupportPreAuthentication;
	int					isSuppSupportPairwiseAsDefaultKey;
	LARGE_INTEGER		CurrentReplayCounter;
	LARGE_INTEGER		LatestGKReplayCounter;
	LARGE_INTEGER		ReplayCounterStarted; // david+1-12-2007
	OCTET_STRING		ANonce;
	OCTET_STRING		SNonce;
	unsigned char		AnonceBuf[KEY_NONCE_LEN];
	unsigned char		SnonceBuf[KEY_NONCE_LEN];
 	unsigned char		PMK[PMK_LEN];
 	unsigned char		PTK[PTK_LEN_TKIP];
	OCTET_STRING		EAPOLMsgRecvd;
	OCTET_STRING		EAPOLMsgSend;
	OCTET_STRING		EapolKeyMsgRecvd;
	OCTET_STRING		EapolKeyMsgSend;
	unsigned char		eapSendBuf[MAX_EAPOLMSG_LEN];
	unsigned char		eapRecvdBuf[MAX_EAPOLMSG_LEN];
	struct timer_list	resendTimer;
	struct rtl8192cd_priv	*priv;
	int					resendCnt;
	int					isGuest;
	int					clientHndshkProcessing;
	int					clientHndshkDone;
	int 				clientGkeyUpdate;
	LARGE_INTEGER		clientMICReportReplayCounter;
#ifdef CONFIG_IEEE80211W	
	BOOLEAN 			mgmt_frame_prot;
#endif
#ifdef CONFIG_IEEE80211R
	unsigned char		isFT;
	unsigned char		cache_pmk_r0_id[PMKID_LEN];
	struct r1_key_holder *r1kh;
	unsigned char		over_ds;
	unsigned char		current_ap[MACADDRLEN];
	unsigned char		cache_r0kh_id[MAX_R0KHID_LEN];
	unsigned int		cache_r0kh_id_len;
	unsigned char		UnicastCipher_1x;
	unsigned char		MulticastCipher_1x;
#endif
} WPA_STA_INFO;
#endif

#if defined(PACK_STRUCTURE) || defined(__ECOS)
#pragma pack(1)
#endif

__PACK typedef struct _LIB1X_EAPOL_KEY
{
 	unsigned char		key_desc_ver;
 	unsigned char		key_info[2];
 	unsigned char		key_len[2];
 	unsigned char		key_replay_counter[KEY_RC_LEN];
 	unsigned char		key_nounce[KEY_NONCE_LEN];
 	unsigned char		key_iv[KEY_IV_LEN];
 	unsigned char		key_rsc[KEY_RSC_LEN];
 	unsigned char		key_id[KEY_ID_LEN];
 	unsigned char		key_mic[KEY_MIC_LEN];
 	unsigned char		key_data_len[KEY_MATERIAL_LEN];
 	unsigned char		*key_data;
}__WLAN_ATTRIB_PACK__ lib1x_eapol_key;


__PACK struct lib1x_eapol
{
 	unsigned char		protocol_version;
 	unsigned char		packet_type;			// This makes it odd in number !
 	unsigned short		packet_body_length;
}__WLAN_ATTRIB_PACK__;

#if defined(PACK_STRUCTURE) || defined(__ECOS)
#pragma pack()
#endif

#define SetSubStr(f,a,l)				memcpy(f.Octet+l,a.Octet,a.Length)
#define GetKeyInfo0(f, mask) 			((f.Octet[KeyInfoPos + 1] & mask) ? 1 :0)
#define SetKeyInfo0(f,mask,b)			(f.Octet[KeyInfoPos + 1] = (f.Octet[KeyInfoPos + 1] & ~mask) | ( b?mask:0x0) )
#define GetKeyInfo1(f, mask)			((f.Octet[KeyInfoPos] & mask) ? 1 :0)
#define SetKeyInfo1(f,mask,b)			(f.Octet[KeyInfoPos] = (f.Octet[KeyInfoPos] & ~mask) | ( b?mask:0x0) )

// EAPOLKey
#define Message_DescType(f)				(f.Octet[DescTypePos])
#define Message_setDescType(f, type)	(f.Octet[DescTypePos] = type)
// Key Information Filed
#define Message_KeyDescVer(f)			(f.Octet[KeyInfoPos+1] & 0x07)//(f.Octet[KeyInfoPos+1] & 0x01) | (f.Octet[KeyInfoPos+1] & 0x02) <<1 | (f.Octet[KeyInfoPos+1] & 0x04) <<2
#define Message_setKeyDescVer(f, v)		(f.Octet[KeyInfoPos+1] &= 0xf8) , f.Octet[KeyInfoPos+1] |= (v & 0x07)//(f.Octet[KeyInfoPos+1] |= ((v&0x01)<<7 | (v&0x02)<<6 | (v&0x04)<<5) )
#define Message_KeyType(f)				GetKeyInfo0(f,0x08)
#define Message_setKeyType(f, b)		SetKeyInfo0(f,0x08,b)
#define Message_KeyIndex(f)				((f.Octet[KeyInfoPos+1] & 0x30) >> 4) & 0x03//(f.Octet[KeyInfoPos+1] & 0x20) | (f.Octet[KeyInfoPos+1] & 0x10) <<1
#define Message_setKeyIndex(f, v)		(f.Octet[KeyInfoPos+1] &= 0xcf), f.Octet[KeyInfoPos+1] |= ((v<<4) & 0x07)//(f.Octet[KeyInfoPos+1] |= ( (v&0x01)<<5 | (v&0x02)<<4)  )
#define Message_Install(f)				GetKeyInfo0(f,0x40)
#define Message_setInstall(f, b)		SetKeyInfo0(f,0x40,b)
#define Message_KeyAck(f)				GetKeyInfo0(f,0x80)
#define Message_setKeyAck(f, b)			SetKeyInfo0(f,0x80,b)

#define Message_KeyMIC(f)				GetKeyInfo1(f,0x01)
#define Message_setKeyMIC(f, b)			SetKeyInfo1(f,0x01,b)
#define Message_Secure(f)				GetKeyInfo1(f,0x02)
#define Message_setSecure(f, b)			SetKeyInfo1(f,0x02,b)
#define Message_Error(f)				GetKeyInfo1(f,0x04)
#define Message_setError(f, b)			SetKeyInfo1(f,0x04,b)
#define Message_Request(f)				GetKeyInfo1(f,0x08)
#define Message_setRequest(f, b)		SetKeyInfo1(f,0x08,b)
#define Message_Reserved(f)				(f.Octet[KeyInfoPos] & 0xf0)
#define Message_setReserved(f, v)		(f.Octet[KeyInfoPos] |= (v<<4&0xff))
#define Message_KeyLength(f)			((unsigned short)(f.Octet[KeyLenPos] <<8) + (unsigned short)(f.Octet[KeyLenPos+1]))
#define Message_setKeyLength(f, v)		(f.Octet[KeyLenPos] = (v&0xff00) >>8 ,  f.Octet[KeyLenPos+1] = (v&0x00ff))

#define Message_KeyNonce(f)					SubStr(f,KeyNoncePos,KEY_NONCE_LEN)
#define Message_setKeyNonce(f, v)			SetSubStr(f, v, KeyNoncePos)
#define Message_EqualKeyNonce(f1, f2)		(memcmp(f1.Octet + KeyNoncePos, f2.Octet, KEY_NONCE_LEN)? 0:1)
#define Message_KeyIV(f)					Substr(f, KeyIVPos, KEY_IV_LEN)
#define Message_setKeyIV(f, v)				SetSubStr(f, v, KeyIVPos)
#define Message_KeyRSC(f)					Substr(f, KeyRSCPos, KEY_RSC_LEN)
#define Message_setKeyRSC(f, v)				SetSubStr(f, v, KeyRSCPos)
#define Message_KeyID(f)					Substr(f, KeyIDPos, KEY_ID_LEN)
#define Message_setKeyID(f, v)				SetSubStr(f, v, KeyIDPos)
#define Message_MIC(f)						SubStr(f, KeyMICPos, KEY_MIC_LEN)
#define Message_setMIC(f, v)				SetSubStr(f, v, KeyMICPos)
#define Message_clearMIC(f)					memset(f.Octet+KeyMICPos, 0, KEY_MIC_LEN)
#define Message_KeyDataLength(f)			((unsigned short)(f.Octet[KeyDataLenPos] <<8) + (unsigned short)(f.Octet[KeyDataLenPos+1]))
#define Message_setKeyDataLength(f, v)		(f.Octet[KeyDataLenPos] = (v&0xff00) >>8 ,  f.Octet[KeyDataLenPos+1] = (v&0x00ff))
#define Message_KeyData(f, l)				SubStr(f, KeyDataPos, l)
#define Message_setKeyData(f, v)			SetSubStr(f, v, KeyDataPos);
#define Message_EqualRSNIE(f1 , f2, l)		(memcmp(f1.Octet, f2.Octet, l) ? 0:1)
#define Message_ReturnKeyDataLength(f)		f.Length - (ETHER_HDRLEN + LIB1X_EAPOL_HDRLEN + EAPOLMSG_HDRLEN)

#define Message_CopyReplayCounter(f1, f2)	memcpy(f1.Octet + ReplayCounterPos, f2.Octet + ReplayCounterPos, KEY_RC_LEN)
#define Message_DefaultReplayCounter(li)	((li.field.HighPart == 0xffffffff) && (li.field.LowPart == 0xffffffff) ) ?1:0
#ifdef CONFIG_IEEE80211W_CLI
#define Message_setSha256AKM(f, v)			SetSubStr(f, v, KeyAKMPos)
#endif

#endif // _8192CD_PSK_H_

