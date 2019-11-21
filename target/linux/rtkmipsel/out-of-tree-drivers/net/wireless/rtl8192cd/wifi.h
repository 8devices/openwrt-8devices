/*
 *  Header file defines some WiFi (802.11) standard related
 *
 *  $Id: wifi.h,v 1.2 2010/01/20 11:10:10 pluswang Exp $
 *
 *  Copyright (c) 2009 Realtek Semiconductor Corp.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 */

#ifndef _WIFI_H_
#define _WIFI_H_

#ifdef BIT
//#error	"BIT define occurred earlier elsewhere!\n"
#undef BIT
#endif
#define BIT(x)	(1 << (x))


#define WLAN_ETHHDR_LEN		14
#define WLAN_ETHADDR_LEN	6
#define WLAN_IEEE_OUI_LEN	3
#define WLAN_ADDR_LEN		6
#define WLAN_CRC_LEN		4
#define WLAN_BSSID_LEN		6
#define WLAN_BSS_TS_LEN		8
#define WLAN_HDR_PSPOLL		16
#define WLAN_HDR_A3_LEN		24
#define WLAN_HDR_A3_HT_LEN	28
#define WLAN_HDR_A4_LEN		30
#define WLAN_HDR_A3_QOS_LEN	26
#define WLAN_HDR_A3_QOS_HT_LEN	30
#define WLAN_HDR_A4_QOS_LEN	32
#define WLAN_HDR_A4_QOS_HT_LEN	36

#define WLAN_SSID_MAXLEN	32
#define WLAN_DATA_MAXLEN	2312

#define WLAN_A3_PN_OFFSET	24
#define WLAN_A4_PN_OFFSET	30

#define WLAN_LLC_HEADER_SIZE	6


#define WLAN_MIN_ETHFRM_LEN	60
#define WLAN_MAX_ETHFRM_LEN	1514
#define WLAN_ETHHDR_LEN		14

#define P80211CAPTURE_VERSION	0x80211001

#if defined(GREEN_HILL) || defined(PACK_STRUCTURE) || defined(__ECOS)
#pragma pack(1)
#endif

__PACK struct wlan_ethhdr_t
{
	UINT8	daddr[WLAN_ETHADDR_LEN]		;
	UINT8	saddr[WLAN_ETHADDR_LEN]		;
	UINT16	type						__WLAN_ATTRIB_PACK__;
} __WLAN_ATTRIB_PACK__;

__PACK struct wlan_llc_t
{
	UINT8	dsap						;
	UINT8	ssap						;
	UINT8	ctl							;
} __WLAN_ATTRIB_PACK__;

__PACK struct wlan_snap_t
{
	UINT8	oui[WLAN_IEEE_OUI_LEN]		;
	UINT16	type						__WLAN_ATTRIB_PACK__;
} __WLAN_ATTRIB_PACK__;

__PACK struct llc_snap {
	struct	wlan_llc_t	llc_hdr			;
	struct	wlan_snap_t	snap_hdr		;
} __WLAN_ATTRIB_PACK__;

#ifdef SUPPORT_TX_AMSDU_SHORTCUT
__PACK struct wlan_amsdullcsnaphdr_t
{
	UINT8	daddr[WLAN_ETHADDR_LEN]		;
	UINT8	saddr[WLAN_ETHADDR_LEN]		;
	UINT16	amsdulen						__WLAN_ATTRIB_PACK__;
	struct llc_snap	llc_snap_hdr			;
} __WLAN_ATTRIB_PACK__;
#endif
__PACK struct ht_cap_elmt
{
	UINT16	ht_cap_info					__WLAN_ATTRIB_PACK__;
	UINT8	ampdu_para					;
	UINT8	support_mcs[16]				;
	UINT16	ht_ext_cap					__WLAN_ATTRIB_PACK__;
	UINT32	txbf_cap					__WLAN_ATTRIB_PACK__;
	UINT8	asel_cap					;
} __WLAN_ATTRIB_PACK__;

__PACK struct ht_info_elmt
{
	UINT8	primary_ch					;
	UINT8	info0						;
	UINT16	info1						__WLAN_ATTRIB_PACK__;
	UINT16	info2						__WLAN_ATTRIB_PACK__;
	UINT8	basic_mcs[16]				;
} __WLAN_ATTRIB_PACK__;

__PACK struct vht_cap_elmt
{
	UINT32	vht_cap_info				__WLAN_ATTRIB_PACK__;
	UINT32	vht_support_mcs[2]			__WLAN_ATTRIB_PACK__;
} __WLAN_ATTRIB_PACK__;

__PACK struct vht_oper_elmt
{
	UINT8	vht_oper_info[3]			;
	UINT16	vht_basic_msc				__WLAN_ATTRIB_PACK__;
} __WLAN_ATTRIB_PACK__;


#ifdef WIFI_11N_2040_COEXIST
__PACK struct obss_scan_para_elmt
{
	UINT16	word0					__WLAN_ATTRIB_PACK__;
	UINT16	word1					__WLAN_ATTRIB_PACK__;
	UINT16	word2					__WLAN_ATTRIB_PACK__;
	UINT16	word3					__WLAN_ATTRIB_PACK__;
	UINT16	word4					__WLAN_ATTRIB_PACK__;
	UINT16	word5					__WLAN_ATTRIB_PACK__;
	UINT16	word6					__WLAN_ATTRIB_PACK__;
} __WLAN_ATTRIB_PACK__;
#endif


#if defined(GREEN_HILL) || defined(PACK_STRUCTURE) || defined(__ECOS)
#pragma pack()
#endif

/**
 *	@brief Frame type value
 *  See 802.11 Table.1 , type value define by bit2 bit3
 */
enum WIFI_FRAME_TYPE {
	WIFI_MGT_TYPE  =	(0),
	WIFI_CTRL_TYPE =	(BIT(2)),
	WIFI_DATA_TYPE =	(BIT(3)),

};

/**
 *	@brief WIFI_FRAME_SUBTYPE
 *  See 802.11 Table.1 valid type and subtype combinations
 *
 */
enum WIFI_FRAME_SUBTYPE {

	// below is for mgt frame
	WIFI_ASSOCREQ       = (0 | WIFI_MGT_TYPE),
    WIFI_ASSOCRSP       = (BIT(4) | WIFI_MGT_TYPE),
    WIFI_REASSOCREQ     = (BIT(5) | WIFI_MGT_TYPE),
    WIFI_REASSOCRSP     = (BIT(5) | BIT(4) | WIFI_MGT_TYPE),
    WIFI_PROBEREQ       = (BIT(6) | WIFI_MGT_TYPE),
    WIFI_PROBERSP       = (BIT(6) | BIT(4) | WIFI_MGT_TYPE),
    WIFI_BEACON         = (BIT(7) | WIFI_MGT_TYPE),
    WIFI_ATIM           = (BIT(7) | BIT(4) | WIFI_MGT_TYPE),
    WIFI_DISASSOC       = (BIT(7) | BIT(5) | WIFI_MGT_TYPE),
    WIFI_AUTH           = (BIT(7) | BIT(5) | BIT(4) | WIFI_MGT_TYPE),
    WIFI_DEAUTH         = (BIT(7) | BIT(6) | WIFI_MGT_TYPE),
    WIFI_WMM_ACTION		= (BIT(7) | BIT(6) | BIT(4) | WIFI_MGT_TYPE),

    // below is for control frame
    WIFI_BLOCKACK_REQ	= (BIT(7) | WIFI_CTRL_TYPE),
    WIFI_BLOCKACK		= (BIT(7) | BIT(4) | WIFI_CTRL_TYPE),
    WIFI_PSPOLL         = (BIT(7) | BIT(5) | WIFI_CTRL_TYPE),
    WIFI_RTS            = (BIT(7) | BIT(5) | BIT(4) | WIFI_CTRL_TYPE),
    WIFI_CTS            = (BIT(7) | BIT(6) | WIFI_CTRL_TYPE),
    WIFI_ACK            = (BIT(7) | BIT(6) | BIT(4) | WIFI_CTRL_TYPE),
    WIFI_CFEND          = (BIT(7) | BIT(6) | BIT(5) | WIFI_CTRL_TYPE),
    WIFI_CFEND_CFACK    = (BIT(7) | BIT(6) | BIT(5) | BIT(4) | WIFI_CTRL_TYPE),

    // below is for data frame
    WIFI_DATA           = (0 | WIFI_DATA_TYPE),
    WIFI_QOS_DATA       = (BIT(7) | WIFI_DATA_TYPE),
    WIFI_DATA_CFACK     = (BIT(4) | WIFI_DATA_TYPE),
    WIFI_DATA_CFPOLL    = (BIT(5) | WIFI_DATA_TYPE),
    WIFI_DATA_CFACKPOLL = (BIT(5) | BIT(4) | WIFI_DATA_TYPE),
    WIFI_DATA_NULL      = (BIT(6) | WIFI_DATA_TYPE),
    WIFI_CF_ACK         = (BIT(6) | BIT(4) | WIFI_DATA_TYPE),
    WIFI_CF_POLL        = (BIT(6) | BIT(5) | WIFI_DATA_TYPE),
    WIFI_CF_ACKPOLL     = (BIT(6) | BIT(5) | BIT(4) | WIFI_DATA_TYPE),


};


/* cfg p2p*/
#define _VENDOR_SPECIFIC_IE_	221

/* cfg p2p*/
#define MAX_IE_SZ 255 // no allow fragment now

enum WIFI_AUTH_ALGM {
	_AUTH_ALGM_OPEN_					= 0,
	_AUTH_ALGM_SHARED_					= 1,
	_AUTH_ALGM_FT_						= 2
};

/**
 *	@brief	REASON CODE
 *	16 bit field, See textbook Table.4-5.
 */
enum WIFI_REASON_CODE	{
	_RSON_RESERVED_					= 0,	// Reserved.
	_RSON_UNSPECIFIED_				= 1,	// Unspecified.
	_RSON_AUTH_NO_LONGER_VALID_		= 2,	// Auth invalid.
	_RSON_DEAUTH_STA_LEAVING_		= 3,	// STA leave BSS or ESS, DeAuth.
	_RSON_INACTIVITY_				= 4,	// Exceed idle timer, Disconnect.
	_RSON_UNABLE_HANDLE_			= 5,	// BS resoruce insufficient.
	_RSON_CLS2_						= 6,	// UnAuth STA frame or SubType incorrect.
	_RSON_CLS3_						= 7, 	// UnAuth STA frame or SubType incorrect.
	_RSON_DISAOC_STA_LEAVING_		= 8,	// STA leave BSS or ESS, DeAssoc.
	_RSON_ASOC_NOT_AUTH_			= 9,	// Request assiciate or reassociate, before authenticate
	// 10,11,12 for 802.11h
	// WPA reason
	_RSON_DISASSOC_DUE_BSS_TRANSITION = 12,	//Disassociated due to BSS Transition Management
	_RSON_INVALID_IE_				= 13,
	_RSON_MIC_FAILURE_				= 14,
	_RSON_4WAY_HNDSHK_TIMEOUT_		= 15,
	_RSON_GROUP_KEY_UPDATE_TIMEOUT_	= 16,
	_RSON_DIFF_IE_					= 17,
	_RSON_MLTCST_CIPHER_NOT_VALID_	= 18,
	_RSON_UNICST_CIPHER_NOT_VALID_	= 19,
	_RSON_AKMP_NOT_VALID_			= 20,
	_RSON_UNSUPPORT_RSNE_VER_		= 21,
	_RSON_INVALID_RSNE_CAP_			= 22,
	_RSON_IEEE_802DOT1X_AUTH_FAIL_	= 23,

	//belowing are Realtek definition
	_RSON_PMK_NOT_AVAILABLE_		= 24,
/*#if defined(CONFIG_RTL_WAPI_SUPPORT)*/
	_RSON_USK_HANDSHAKE_TIMEOUT_	= 25,	/* handshake timeout for unicast session key*/
	_RSON_MSK_HANDSHAKE_TIMEOUT_	= 26,	/* handshake timeout for multicast session key*/
	_RSON_IE_NOT_CONSISTENT_			= 27,	/* IE was different between USK handshake & assocReq/probeReq/Beacon */
	_RSON_INVALID_USK_				= 28,	/* Invalid unicast key set */
	_RSON_INVALID_MSK_				= 29,	/* Invalid multicast key set */
	_RSON_INVALID_WAPI_VERSION_		= 30,	/* Invalid wapi version */
	_RSON_INVALID_WAPI_CAPABILITY_	= 31,	/* Wapi capability not support */
/*#endif*/

    //below are mesh definition
    _RSON_MESH_CHANNEL_SWITCH_REGULATORY_REQUIREMENTS_ = 65,
    _RSON_MESH_CHANNEL_SWITCH_UNSPECIFIED_ = 66,
};

enum WIFI_STATUS_CODE {
	_STATS_SUCCESSFUL_				= 0,	// Success.
	_STATS_FAILURE_					= 1,	// Failure.
	_STATS_CAP_FAIL_				= 10,	// Capability too wide, can't support
	_STATS_NO_ASOC_					= 11,	// Denial reassociate
	_STATS_OTHER_					= 12,	// Denial connect, not 802.11 standard.
	_STATS_NO_SUPP_ALG_				= 13,	// Authenticate algorithm not support .
	_STATS_OUT_OF_AUTH_SEQ_			= 14,	// Out of authenticate sequence number.
	_STATS_CHALLENGE_FAIL_			= 15,	// Denial authenticate, Response message fail.
	_STATS_AUTH_TIMEOUT_			= 16,	// Denial authenticate, timeout.
	_STATS_UNABLE_HANDLE_STA_		= 17,	// Denial authenticate, BS resoruce insufficient.
	_STATS_RATE_FAIL_				= 18,	// Denial authenticate, STA not support BSS request datarate.
	_STATS_ASSOC_REJ_TEMP_			= 30,	// Association request rejected temporarily; try again later
	_STATS_REQ_DECLINED_		= 37,
/*#if defined(CONFIG_RTL_WAPI_SUPPORT)*/
	__STATS_INVALID_IE_ = 40,
	__STATS_INVALID_AKMP_ = 43,
	__STATS_CIPER_REJECT_ = 46,
	__STATS_INVALID_USK_ = 47,
	__STATS_INVALID_MSK_ = 48,
	__STATS_INVALID_WAPI_VERSION_ = 49,
	__STATS_INVALID_WAPI_CAPABILITY_ = 50,
/*#endif*/

#ifdef CONFIG_IEEE80211V
	_STATS_REJ_BSS_TRANSITION		= 82	,	//Rejected with Suggested BSS Transition
#endif
};

enum WIFI_REG_DOMAIN {
	DOMAIN_FCC		= 1,
	DOMAIN_IC		= 2,
	DOMAIN_ETSI		= 3,
	DOMAIN_SPAIN	= 4,
	DOMAIN_FRANCE	= 5,
	DOMAIN_MKK		= 6,
	DOMAIN_ISRAEL	= 7,
	DOMAIN_MKK1		= 8,
	DOMAIN_MKK2		= 9,
	DOMAIN_MKK3		= 10,
	DOMAIN_NCC		= 11,
	DOMAIN_RUSSIAN	= 12,
	DOMAIN_CN		= 13,
	DOMAIN_GLOBAL	= 14,
	DOMAIN_WORLD_WIDE = 15,
	DOMAIN_TEST		= 16,
	DOMAIN_5M10M	= 17,
	DOMAIN_SG		= 18,
	DOMAIN_KR		= 19,
	DOMAIN_MAX
};


#define _TO_DS_		BIT(8)
#define _FROM_DS_	BIT(9)
#define _MORE_FRAG_	BIT(10)
#define _RETRY_		BIT(11)
#define _PWRMGT_	BIT(12)
#define _MORE_DATA_	BIT(13)
#define _PRIVACY_	BIT(14)
#define _ORDER_	    BIT(15)

#define SetToDs(pbuf)	\
	do	{	\
		*(unsigned short *)(pbuf) |= cpu_to_le16(_TO_DS_); \
	} while(0)

#define GetToDs(pbuf)	(((*(unsigned short *)(pbuf)) & le16_to_cpu(_TO_DS_)) != 0)

#define ClearToDs(pbuf)	\
	do	{	\
		*(unsigned short *)(pbuf) &= (~cpu_to_le16(_TO_DS_)); \
	} while(0)

#define SetFrDs(pbuf)	\
	do	{	\
		*(unsigned short *)(pbuf) |= cpu_to_le16(_FROM_DS_); \
	} while(0)

#define GetFrDs(pbuf)	(((*(unsigned short *)(pbuf)) & le16_to_cpu(_FROM_DS_)) != 0)

#define ClearFrDs(pbuf)	\
	do	{	\
		*(unsigned short *)(pbuf) &= (~cpu_to_le16(_FROM_DS_)); \
	} while(0)

#define SetMFrag(pbuf)	\
	do	{	\
		*(unsigned short *)(pbuf) |= cpu_to_le16(_MORE_FRAG_); \
	} while(0)

#define GetMFrag(pbuf)	(((*(unsigned short *)(pbuf)) & le16_to_cpu(_MORE_FRAG_)) != 0)

#define ClearMFrag(pbuf)	\
	do	{	\
		*(unsigned short *)(pbuf) &= (~cpu_to_le16(_MORE_FRAG_)); \
	} while(0)

#define SetRetry(pbuf)	\
	do	{	\
		*(unsigned short *)(pbuf) |= cpu_to_le16(_RETRY_); \
	} while(0)

#define GetRetry(pbuf)	(((*(unsigned short *)(pbuf)) & le16_to_cpu(_RETRY_)) != 0)

#define ClearRetry(pbuf)	\
	do	{	\
		*(unsigned short *)(pbuf) &= (~cpu_to_le16(_RETRY_)); \
	} while(0)

#define SetPwrMgt(pbuf)	\
	do	{	\
		*(unsigned short *)(pbuf) |= cpu_to_le16(_PWRMGT_); \
	} while(0)

#define GetPwrMgt(pbuf)	(((*(unsigned short *)(pbuf)) & le16_to_cpu(_PWRMGT_)) != 0)

#define ClearPwrMgt(pbuf)	\
	do	{	\
		*(unsigned short *)(pbuf) &= (~cpu_to_le16(_PWRMGT_)); \
	} while(0)

#define SetMData(pbuf)	\
	do	{	\
		*(unsigned short *)(pbuf) |= cpu_to_le16(_MORE_DATA_); \
	} while(0)

#define GetMData(pbuf)	(((*(unsigned short *)(pbuf)) & le16_to_cpu(_MORE_DATA_)) != 0)

#define ClearMData(pbuf)	\
	do	{	\
		*(unsigned short *)(pbuf) &= (~cpu_to_le16(_MORE_DATA_)); \
	} while(0)

#define SetPrivacy(pbuf)	\
	do	{	\
		*(unsigned short *)(pbuf) |= cpu_to_le16(_PRIVACY_); \
	} while(0)

#define GetPrivacy(pbuf)	(((*(unsigned short *)(pbuf)) & le16_to_cpu(_PRIVACY_)) != 0)

#define ClearPrivacy(pbuf)	\
	do	{	\
		*(unsigned short *)(pbuf) &= (~cpu_to_le16(_PRIVACY_)); \
	} while(0)


#define GetOrder(pbuf)	(((*(unsigned short *)(pbuf)) & le16_to_cpu(_ORDER_)) != 0)


#define GetFrameType(pbuf)	(le16_to_cpu(*(unsigned short *)(pbuf)) & (BIT(3) | BIT(2)))

#define SetFrameType(pbuf,type)	\
	do { 	\
		*(unsigned short *)(pbuf) &= __constant_cpu_to_le16(~(BIT(3) | BIT(2))); \
		*(unsigned short *)(pbuf) |= __constant_cpu_to_le16(type); \
	} while(0)

#define GetFrameSubType(pbuf)	(cpu_to_le16(*(unsigned short *)(pbuf)) & (BIT(7) | BIT(6) | BIT(5) | BIT(4) | BIT(3) | BIT(2)))

#define SetFrameSubType(pbuf,type) \
	do {    \
		*(unsigned short *)(pbuf) &= cpu_to_le16(~(BIT(7) | BIT(6) | BIT(5) | BIT(4) | BIT(3) | BIT(2))); \
		*(unsigned short *)(pbuf) |= cpu_to_le16(type); \
	} while(0)

#define GetSequence(pbuf)	(cpu_to_le16(*(unsigned short *)((unsigned long)(pbuf) + 22)) >> 4)

#define GetFragNum(pbuf)	(cpu_to_le16(*(unsigned short *)((unsigned long)(pbuf) + 22)) & 0x0f)

#define GetTupleCache(pbuf)	(cpu_to_le16(*(unsigned short *)((unsigned long)(pbuf) + 22)))

#define SetFragNum(pbuf, num) \
	do {    \
		*(unsigned short *)((unsigned long)(pbuf) + 22) = \
			((*(unsigned short *)((unsigned long)(pbuf) + 22)) & le16_to_cpu(~(0x000f))) | \
			cpu_to_le16(0x0f & (num));     \
	} while(0)

#define SetSeqNum(pbuf, num) \
	do {    \
		*(unsigned short *)((unsigned long)(pbuf) + 22) = \
			((*(unsigned short *)((unsigned long)(pbuf) + 22)) & le16_to_cpu((unsigned short)~0xfff0)) | \
			le16_to_cpu((unsigned short)(0xfff0 & (num << 4))); \
	} while(0)

#define SetDuration(pbuf, dur) \
	do {    \
		*(unsigned short *)((unsigned long)(pbuf) + 2) |= cpu_to_le16(0xffff & (dur)); \
	} while(0)

#define GetAid(pbuf)	(cpu_to_le16(*(unsigned short *)((unsigned long)(pbuf) + 2)) & 0x3fff)

#define GetTid(pbuf)	(cpu_to_le16(*(unsigned short *)((unsigned long)(pbuf) + (((GetToDs(pbuf)<<1)|GetFrDs(pbuf))==3?30:24))) & 0x000f)

#define SetPsPollAid(pbuf, aid)  \
	do {    \
		*(unsigned short *)((unsigned long)(pbuf) + 2) |= cpu_to_le16(0xffff & (aid|0xc000)); \
	} while(0)

//WIFI_WMM
#define GetQOSackPolicy(pbuf)	((cpu_to_le16(*(unsigned short *)((unsigned long)(pbuf) + (((GetToDs(pbuf)<<1)|GetFrDs(pbuf))==3?30:24))) & 0x0060)>>5)

#define GetAddr1Ptr(pbuf)	((unsigned char *)((unsigned long)(pbuf) + 4))

#define GetAddr2Ptr(pbuf)	((unsigned char *)((unsigned long)(pbuf) + 10))

#define GetAddr3Ptr(pbuf)	((unsigned char *)((unsigned long)(pbuf) + 16))

#define GetAddr4Ptr(pbuf)	((unsigned char *)((unsigned long)(pbuf) + 24))

//WIFI_WMM
#define GetQosControl(pbuf) (unsigned char *)((unsigned long)(pbuf) + (((GetToDs(pbuf)<<1)|GetFrDs(pbuf))==3?30:24))



/*-----------------------------------------------------------------------------
			Below is for the security related definition
------------------------------------------------------------------------------*/
#define _RESERVED_FRAME_TYPE_	0
#define _SKB_FRAME_TYPE_		2
#define _PRE_ALLOCMEM_			1
#define _PRE_ALLOCHDR_			3
#define _PRE_ALLOCLLCHDR_		4
#define _PRE_ALLOCICVHDR_		5
#define _PRE_ALLOCMICHDR_		6

#define _SIFSTIME_				((priv->pmib->dot11BssType.net_work_type&WIRELESS_11A)?16:10)
#define _ACKCTSLNG_				14	//14 bytes long, including crclng
#define _CRCLNG_				4

#define _ASOCREQ_IE_OFFSET_		4	// excluding wlan_hdr
#define	_ASOCRSP_IE_OFFSET_		6
#define _REASOCREQ_IE_OFFSET_	10
#define _REASOCRSP_IE_OFFSET_	6
#define _PROBEREQ_IE_OFFSET_	0
#define	_PROBERSP_IE_OFFSET_	12
#define _AUTH_IE_OFFSET_		6
#define _DEAUTH_IE_OFFSET_		0
#define _BEACON_IE_OFFSET_		12
#define _BEACON_CAP_OFFSET_	34


/* information element ID ,See textbook Table 4.7 */
#define _SSID_IE_				0
#define _SUPPORTEDRATES_IE_		1
#define _DSSET_IE_				3
#define _TIM_IE_				5
#define _IBSS_PARA_IE_			6
#define _COUNTRY_IE_			7	// for 802.11d
#define _REQUEST_IE_		    10
#define _BSS_LOAD_IE_           11
#define _CHLGETXT_IE_			16

#define	_PWR_CONSTRAINT_IE_	        32
#define	_PWR_CAPABILITY_IE_	        33
#define	_TPC_REQUEST_IE_            34
#define	_TPC_REPORT_IE_	            35
#define	_SUPPORTED_CHANNEL_IE_      36
#define _CSA_IE_                    37
#define _MEASUREMENT_REQUEST_IE_    38
#define _MEASUREMENT_REPORT_IE_     39
#define _SECONDARY_CHANNEL_OFFSET_IE_					62

#define _RSN_IE_2_                  48
#define _RSN_IE_1_                  221	// Error, Shall be Wi-Fi protected access (802.11b)
#define _ERPINFO_IE_                42	// [802.11g 7.3.2] ERP Information
#define _EXT_SUPPORTEDRATES_IE_     50	// [802.11g 7.3.2] Extended supported rates
#define _AP_CHANNEL_REPORT_IE_      51
#define _NEIGHBOR_REPORT_IE_        52
#define _MOBILITY_DOMAIN_IE_		54
#define _RM_ENABLE_CAP_IE_          70
#define _WPS_IE_                    221
#define _VENDOR_SPEC_IE_            221
// CONFIG_IEEE80211W_CLI
#define _MMIC_IE_				0x4c
#define _MMIC_LEN_				16
#define MIC_LEN					8
#define PMF_REQ					0x600
#define PMF_CAP					0x400
#define PMF_NONE				0x200

#define BIP_HEADER_LEN			26
#define MMIC_TAG_IE				1
#define MMIC_TAG_LEN			1
#define MMIC_CRC_LEN 			4
#define MMIC_ILLEGAL_LEN		30
#define MGNT_LEGAL				1
#define MGNT_ERR				0
#define MGNT_PRIVACY_ERR		-1
#define MGNT_BCAST_PRIVACY_ERR	-2



/*-----------------------------------------------------------------------------
            Below is for mesh related definition
------------------------------------------------------------------------------*/
#define _MESH_CATEGORY_ID_             13
#define _MULTIHOP_CATEGORY_ID_         14
#define _SELF_PROTECTED_CATEGORY_ID_   15

/*used in Mesh category */
#define _MESH_LINK_REPORT_ACTION_ID_   0
#define _HWMP_MESH_PATHSEL_ACTION_ID_  1
#define _GANN_ACTION_ID_               2

/*used in Multihop category */
#define _PROXY_UPDATES_ACTION_ID_         0
#define _PROXY_UPDATES_CONFIRM_ACTION_ID_ 1


#define _WLAN_MESH_CAP_IE_			   113	// WLAN mesh configuration IE
#define _MESH_ID_IE_				   114	// MESH ID infoemation element
#define _MESH_LINK_METRIC_REPORT_IE_   115	// Local link state announcement IE
#define _MESH_CHANNEL_SWITCH_IE_       118
#define _MESH_GANN_IE_	               125
#define _MESH_RANN_IE_	               126
#define _MESH_PREQ_IE_	               130
#define _MESH_PREP_IE_	               131
#define _MESH_PERR_IE_	               132
#define _PROXY_UPDATE_IE_			   137
#define _PROXY_UPDATE_CONFIRM_IE_	   138

// CATUTION: below ALL undefine !!
#define _OFDM_PARAMETER_SET_IE_		   200
#define _NEIGHBOR_LIST_IE_			   205	// neighbor list IE
#define _UCG_SWITCH_ANNOU_IE_		   210
#define _MDAOP_ADVERTISMENTS_IE_	   211
#define _MOAOP_SET_TEARDOWN_IE_		   212
#define _PEER_LINK_OPEN_IE_			   223
#define _PEER_LINK_CONFIRM_IE_		   224
#define _PEER_LINK_CLOSE_IE_		   225

/* ---------------------------------------------------------------------------
					Below is the fixed elements...
-----------------------------------------------------------------------------*/
#define _AUTH_ALGM_NUM_			2
#define _AUTH_SEQ_NUM_			2
#define _BEACON_ITERVAL_		2
#define _CAPABILITY_			2
#define _CURRENT_APADDR_		6
#define _LISTEN_INTERVAL_		2
#define _RSON_CODE_				2
#define _ASOC_ID_				2
#define _STATUS_CODE_			2
#define _TIMESTAMP_				8

#define AUTH_ODD_TO				0
#define AUTH_EVEN_TO			1

#define WLAN_ETHCONV_ENCAP		1
#define WLAN_ETHCONV_RFC1042	2
#define WLAN_ETHCONV_8021h		3
#define BEACON_MACHDR_LEN		24

/*-----------------------------------------------------------------------------
				Below is the definition for 802.11i / 802.1x
------------------------------------------------------------------------------*/
#define _IEEE8021X_MGT_			1		// WPA
#define _IEEE8021X_PSK_			2		// WPA with pre-shared key

#define _NO_PRIVACY_			0
#define _WEP_40_PRIVACY_		1
#define _TKIP_PRIVACY_			2
#define _WRAP_PRIVACY_			3
#define _CCMP_PRIVACY_			4
#define _WEP_104_PRIVACY_		5
#define _WEP_WPA_MIXED_PRIVACY_ 6	// WEP + WPA
#define _WAPI_SMS4_				7


/*-----------------------------------------------------------------------------
			Below is for QoS related definition
------------------------------------------------------------------------------*/
#define _WMM_IE_Length_				7
#define _WMM_Para_Element_Length_	24
#define _ADDBA_Req_Frame_Length_	9
#define _ADDBA_Rsp_Frame_Length_	9
#define _DELBA_Frame_Length 		6
#define _ADDBA_Maximum_Buffer_Size_	64

#define _BLOCK_ACK_CATEGORY_ID_		3
#define _ADDBA_Req_ACTION_ID_		0
#define _ADDBA_Rsp_ACTION_ID_		1
#define _DELBA_ACTION_ID_			2
#define _VENDOR_ACTION_ID_			0x7f	// add for P2P_SUPPORT

/*-----------------------------------------------------------------------------
				Below is for PMF related definition
------------------------------------------------------------------------------*/
#define _SA_QUERY_CATEGORY_ID_		8
#define _SA_QUERY_REQ_ACTION_ID_	0
#define _SA_QUERY_RSP_ACTION_ID_	1
#define EID_TIMEOUT_INTERVAL		56
#define ASSOC_COMEBACK_TIME			3


/*-----------------------------------------------------------------------------
				Below is for Fast BSS Transition related definition
------------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
				Below is for 11v BSS Transition related definition
------------------------------------------------------------------------------*/
#ifdef CONFIG_IEEE80211V
#ifdef CONFIG_IEEE80211V_CLI
#define	BSS_TRANS_SUPPORT				BIT(29)		//if add ext_cap in struct bss_desc, it will "caught SIGSEGV, dumping core in /tmp".
#define	BSS_TERMINATION_DURATION_LEN	12
#endif

#define _DEFAULT_STATUS_CODE_					-1	// for initializing status code
#define _TIMEOUT_STATUS_CODE_					99	// for no response after time out

#define _WNM_PREFERED_BSS_TRANS_LIST_INCLUDED_	19

#define _WNM_BSS_TRANS_SUPPORT_			BIT(3)

#define _WNM_PREFERRED_CANDIDATE_LIST_ 	BIT(0)		// 1: process
#define _WNM_ABRIDGED_					BIT(1)		// 0: no recommandation or against
#define _WNM_DIASSOC_IMMINENT_			BIT(2)		// 1: diassoc from current AP
#define _WNM_BSS_TERMINATION_INCLUDED_	BIT(3)		// 1: bss is shutting down
#define _WNM_ESS_DIASSOC_IMMINENT_		BIT(4)

#define MAX_LIST_LEN						2304

enum bss_trans_mgmt_status_code {
	WNM_BSS_TM_ACCEPT 									= 0,
	WNM_BSS_TM_REJECT_UNSPECIFIED 						= 1,
	WNM_BSS_TM_REJECT_INSUFFICIENT_BEACON			 	= 2,
	WNM_BSS_TM_REJECT_INSUFFICIENT_CAPABITY 			= 3,
	WNM_BSS_TM_REJECT_UNDESIRED 						= 4,
	WNM_BSS_TM_REJECT_DELAY_REQUEST 					= 5,
	WNM_BSS_TM_REJECT_STA_CANDIDATE_LIST_PROVIDED 	= 6,
	WNM_BSS_TM_REJECT_NO_SUITABLE_CANDIDATES 			= 7,
	WNM_BSS_TM_REJECT_LEAVING_ESS 						= 8
};
#endif

/*-----------------------------------------------------------------------------
			Below is for HT related definition
------------------------------------------------------------------------------*/
#define _HT_CAP_					45
#define _HT_IE_						61

#define _HT_MIMO_PS_STATIC_			BIT(0)
#define _HT_MIMO_PS_DYNAMIC_		BIT(1)

#define _HT_MIMO_PS_Frame_Length_	3

#ifdef WIFI_11N_2040_COEXIST
#define _PUBLIC_CATEGORY_ID_		4
#define _2040_COEXIST_ACTION_ID_	0

#define _2040_BSS_COEXIST_IE_		72
#define _40M_INTOLERANT_			BIT(1)
#define _20M_BSS_WIDTH_REQ_		BIT(2)

#define _2040_Intolerant_ChRpt_IE_	73
#define _OBSS_SCAN_PARA_IE_		74

//#define _EXTENDED_CAP_IE_			127
#define _2040_COEXIST_SUPPORT_	BIT(0)
#endif
#define _HT_CATEGORY_ID_			7
#define _HT_MIMO_PS_ACTION_ID_		1

#define _EXTENDED_CAP_IE_			127


/*-----------------------------------------------------------------------------
			Below is for VHT related definition
------------------------------------------------------------------------------*/
#define EID_VHTCapability			191	// Based on 802.11ac D2.0
#define EID_VHTOperation			192	// Based on 802.11ac D2.0
#define EID_VHTOperatingMode		199
#define EID_VHTTxPwrEnvelope		195
#define EID_WIDEBW_CH_SW			194
#define EID_CH_SW_WRAPPER			196



#define _VHT_ACTION_CATEGORY_ID_	21
#define _VHT_ACTION_OPMNOTIF_ID_	2
#define	_OPMNOTIF_Frame_Length_		3
#if 1 //eric-8822
#define _VHT_ACTION_GROUPID_ID_		1
#define	_GROUPID_Frame_Length_		(1+1+8+16)
#endif

#define _VHTCAP_RX_STBC_CAP_			(BIT(8) | BIT(9)| BIT(10))
#define _VHTCAP_RX_LDPC_CAP_			(BIT(4))

/*-----------------------------------------------------------------------------
			Below is the bit definition for HT Capabilities element
------------------------------------------------------------------------------*/
#define _HTCAP_SUPPORT_RX_LDPC_		BIT(0)

#define _HTCAP_SUPPORT_CH_WDTH_		BIT(1)
#define _HTCAP_SMPWR_STATIC_		0
#define _HTCAP_SMPWR_DYNAMIC_		BIT(2)
#define _HTCAP_STATIC_SMPWR_		0
#define _HTCAP_SMPWR_DISABLE_		(BIT(2) | BIT(3))
#define _HTCAP_SHORTGI_20M_			BIT(5)
#define _HTCAP_SHORTGI_40M_			BIT(6)
#define _HTCAP_TX_STBC_				BIT(7)
#define _HTCAP_RX_STBC_1S_			BIT(8)
#define _HTCAP_RX_STBC_2S_			BIT(9)
#define _HTCAP_RX_STBC_3S_			(BIT(8) | BIT(9))
#define _HTCAP_RX_STBC_CAP_			(BIT(8) | BIT(9))
#define _HTCAP_AMSDU_LEN_8K_		BIT(11)
#define _HTCAP_CCK_IN_40M_			BIT(12)
#ifdef WIFI_11N_2040_COEXIST
#define _HTCAP_40M_INTOLERANT_		BIT(14)
#endif

#if (BEAMFORMING_SUPPORT == 1)
#define _HTCAP_RECEIVED_NDP            BIT(3)
#define _HTCAP_TRANSMIT_NDP           BIT(4)
#endif

#define _HTCAP_AMPDU_FAC_8K_		0
#define _HTCAP_AMPDU_FAC_16K_		BIT(0)
#define _HTCAP_AMPDU_FAC_32K_		BIT(1)
#define _HTCAP_AMPDU_FAC_64K_		(BIT(0) | BIT(1))
#define _HTCAP_AMPDU_SPC_SHIFT_		2
#define _HTCAP_AMPDU_SPC_MASK_		0x1c
#define _HTCAP_AMPDU_SPC_NORES_		0
#define _HTCAP_AMPDU_SPC_QUAR_US_	1
#define _HTCAP_AMPDU_SPC_HALF_US_	2
#define _HTCAP_AMPDU_SPC_1_US_		3
#define _HTCAP_AMPDU_SPC_2_US_		4
#define _HTCAP_AMPDU_SPC_4_US_		5
#define _HTCAP_AMPDU_SPC_8_US_		6
#define _HTCAP_AMPDU_SPC_16_US_		7


/*-----------------------------------------------------------------------------
			Below is the bit definition for HT Information element
------------------------------------------------------------------------------*/
#define _HTIE_2NDCH_OFFSET_NO_		0
#define _HTIE_2NDCH_OFFSET_AB_		BIT(0)
#define _HTIE_2NDCH_OFFSET_BL_		(BIT(0) | BIT(1))
#define	_HTIE_STA_CH_WDTH_			BIT(2)
#define _HTIE_OP_MODE0_				0
#define _HTIE_OP_MODE1_				BIT(0)
#define _HTIE_OP_MODE2_				BIT(1)
#define _HTIE_OP_MODE3_				(BIT(0) | BIT(1))
#define _HTIE_NGF_STA_				BIT(2)
#define	_HTIE_TXBURST_LIMIT_		BIT(3)
#define _HTIE_OBSS_NHT_STA_			BIT(4)

/*-----------------------------------------------------------------------------
            Below is for DLS related definition
 ------------------------------------------------------------------------------*/
#define _DLS_CATEGORY_ID_			2
#define _DLS_REQ_ACTION_ID_			0
#define _DLS_RSP_ACTION_ID_			1
#define _TDLS_CATEGORY_ID_ 			12

#define	_TDLS_PROHIBITED_			BIT(6)
#define	_TDLS_CS_PROHIBITED_		BIT(7)
/*-----------------------------------------------------------------------------
            Below is for HS2.0 related definition
------------------------------------------------------------------------------*/
#define _GAS_INIT_REQ_ACTION_ID_	10
#define _GAS_INIT_RSP_ACTION_ID_	11
#define _GAS_COMBACK_REQ_ACTION_ID_	12
#define _GAS_COMBACK_RSP_ACTION_ID_	13
#define _BSS_LOAD_IE_				11
#define _MUL_BSSID_IE_				71
#define	_TIMEADVT_IE_				69
#define	_TIMEZONE_IE_				98
#define _INTERWORKING_IE_			107
#define _ADVT_PROTO_IE_				108
// HS2_SUPPORT
#define _QOS_MAP_SET_IE_			110
#define _ROAM_IE_					111
#define _HS2_IE_					221
/*20141204 modify */
//#define _PROXY_ARP_				BIT(12)
//#define _BSS_TRANSITION_            	BIT(19)
//#define _UTC_TSF_OFFSET_			BIT(27)
//#define _INTERWORKING_SUPPORT_	BIT(31)
#define _INTERWORKING_SUPPORT_BY_DW_	BIT(31)



#define _PROXY_ARP_					BIT(4)  /*byte1*/
#define _BSS_TRANSITION_            BIT(3)	/*byte2*/
#define _UTC_TSF_OFFSET_			BIT(3)  /*byte3*/
#define _INTERWORKING_SUPPORT_		BIT(7)	/*byte3*/
#define _QOS_MAP_ 					BIT(0)	/*byte4*/
#define _WNM_NOTIFY_ 				BIT(6)	/*byte5*/


// HS2_SUPPORT
#define _QOS_CATEGORY_ID_			1
#define _WNM_CATEGORY_ID_			10
#define _WNM_TSMQUERY_ACTION_ID_	6
#define _WNM_NOTIFICATION_ID_		26
// HS2_SUPPORT
#define _QOS_MAP_CONFIGURE_ID_		4
#define _BSS_TSMREQ_ACTION_ID_		7
#define _BSS_TSMRSP_ACTION_ID_		8
#define _VENDOR_SPECIFIC_IE_	221
#define MAX_IE_SZ 255 // no allow fragment now

enum mgmt_type {
	MGMT_BEACON = 0,
	MGMT_PROBERSP = 1,
	MGMT_ASSOCRSP = 2,
	MGMT_ASSOCREQ = 3,
	MGMT_PROBEREQ = 4,
};
/*-----------------------------------------------------------------------------
            Below is for Spectrum Management  related definition
 ------------------------------------------------------------------------------*/
#define _SPECTRUM_MANAGEMENT_CATEGORY_ID_			0
#define _MEASUREMENT_REQEST_ACTION_ID_	0
#define _MEASUREMENT_REPORT_ACTION_ID_	1
#define _TPC_REQEST_ACTION_ID_          2
#define _TPC_REPORT_ACTION_ID_	        3

/*-----------------------------------------------------------------------------
            Below is for Radio Measurement (802.11K)  related definition
 ------------------------------------------------------------------------------*/
#define _RADIO_MEASUREMENT_CATEGORY_ID_			    5
#define _RADIO_MEASUREMENT_REQEST_ACTION_ID_	    0
#define _RADIO_MEASUREMENT_REPORT_ACTION_ID_	    1
#define _LINK_MEASUREMENT_REQEST_ACTION_ID_	        2
#define _LINK_MEASUREMENT_REPORT_ACTION_ID_	        3
#define _NEIGHBOR_REPORT_REQEST_ACTION_ID_	    4
#define _NEIGHBOR_REPORT_RESPONSE_ACTION_ID_	5

#define _FRAME_BODY_SUBIE_		    1
#define _REPORT_DETAIL_SUBIE_		2
#define MAX_BEACON_SUBLEMENT_LEN    226

#ifdef SUPPORT_MONITOR
#define PTK_LEN_UPPER_BOUND 256
#define PTK_FILTER_LEN 		252
#define WLAN_802_2_LEN		8
#define WLAN_IP_HDR_LEN	20
#define WLAN_UDP_LEN		8
#endif

#endif // _WIFI_H_

