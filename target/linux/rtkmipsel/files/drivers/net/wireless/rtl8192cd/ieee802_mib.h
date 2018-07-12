/*
 *  Header file defines IEEE802.11 MIB
 *
 *  $Id: ieee802_mib.h,v 1.12.2.5 2010/12/16 05:32:20 bruce Exp $
 *
 *  Copyright (c) 2009 Realtek Semiconductor Corp.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 */

/*
 *  Note: Should not use compiler flag in this header file in order for
 *        flash utility to get correct mib size.
 */

#ifndef _IEEE802_MIB_H_
#define _IEEE802_MIB_H_

#ifdef __ECOS
#include <pkgconf/system.h>
#include <pkgconf/devs_eth_rltk_819x_wlan.h>
#endif
#if defined(RTK_NL80211)
#include "wifi.h"
#endif
#define MIB_VERSION				32

#define MAX_2G_CHANNEL_NUM		14
#define MAX_5G_CHANNEL_NUM		196
#define MACADDRLEN				6

#if 1//def CONFIG_SDIO_HCI
#define SDIOTYPELEN             5
#endif

//-------------------------------------------------------------
// Support add or remove ACL list at run time
//-------------------------------------------------------------
#define D_ACL

#ifdef D_ACL
#define NUM_ACL					128
#else
#ifdef CONFIG_RTL8196B_TLD
#define NUM_ACL					64
#else
#define NUM_ACL					32
#endif
#endif

#define NUM_GBWC				64
#define NUM_SBWC				64

#ifdef __ECOS
#ifdef CONFIG_RTL_WDS_SUPPORT
#define NUM_WDS					RTLPKG_DEVS_ETH_RLTK_819X_WLAN_WDS_NUM
#else
#define NUM_WDS					0
#endif
#else
#if defined(CONFIG_RTL8196C_EC) //no WDS or only one WDS
#define NUM_WDS					1
#else
#if defined(CONFIG_RTL8196B_GW_8M) || defined(CONFIG_RTL8196C_AP_ROOT) || defined(CONFIG_RTL8198_AP_ROOT)
#define NUM_WDS					4
#else
#define NUM_WDS					8
#endif
#endif
#endif

#define MESH_ID_LEN				32

#if !defined(_LITTLE_ENDIAN_) && !defined(_BIG_ENDIAN_)
	#define _BIG_ENDIAN_
#ifdef	__MIPSEL__ //eric-8822 97f
	#undef _BIG_ENDIAN_
	#define _LITTLE_ENDIAN_
#endif
#endif

struct Dot11StationConfigEntry {
	unsigned char	dot11Bssid[MACADDRLEN];
	unsigned char	dot11DesiredSSID[32];
	unsigned int	dot11DesiredSSIDLen;
	unsigned char	dot11DefaultSSID[32];
	unsigned int	dot11DefaultSSIDLen;
	unsigned char	dot11SSIDtoScan[32];
	unsigned int	dot11SSIDtoScanLen;
	unsigned char	dot11DesiredBssid[6];
	unsigned char	dot11OperationalRateSet[32];
	unsigned int	dot11OperationalRateSetLen;

	unsigned int	dot11BeaconPeriod;
	unsigned int	dot11DTIMPeriod;
	unsigned int	dot11swcrypto;
	unsigned int	dot11AclMode;			// 1: positive check 2: negative check
	unsigned char	dot11AclAddr[NUM_ACL][MACADDRLEN];
	unsigned int	dot11AclNum;			// acl entry number, this field should be followed to dot11AclAddr
	unsigned int	dot11SupportedRates;	// bit mask value. bit0-bit11 as 1,2,5.5,11,6,9,12,18,24,36,48,54
	unsigned int	dot11BasicRates;		// bit mask value. bit0-bit11 as 1,2,5.5,11,6,9,12,18,24,36,48,54
	unsigned int	dot11RegDomain;			// reguration domain
	int				txpwr_lmt_index;	    // TX Power Limit Index
	unsigned int	autoRate;				// enable/disable auto rate
	unsigned int	fixedTxRate;			// fix tx rate
	int				swTkipMic;
	int				protectionDisabled;		// force disable protection
	int				olbcDetectDisabled;		// david, force disable olbc dection	
	int				nmlscDetectDisabled;		// hf, force disable no member legacy station condition detection
	int				legacySTADeny;			// deny association from legacy (11B) STA

	//unsigned int	w52_passive_scan;
	int				fastRoaming;			// 1: enable fast-roaming, 0: disable
	unsigned int	lowestMlcstRate;		// 1: use lowest basic rate to send multicast
	unsigned int	supportedStaNum;		// limit supported station number
	unsigned int	staAssociateRSSIThreshold;  
	unsigned int	staRoamingRSSIGap;
	unsigned int	staRoamingTimeGap;
	unsigned int	RmStaRSSIThreshold;			
	unsigned int	probe_info_enable;		// proc probe_info 
	unsigned int sc_enabled;	//0 is disable, 1 is enable
	int sc_duration_time;				//-1 is always parse, 0 stop parse, >0 parse all packets.
	int sc_get_sync_time;					//unit is second
	int sc_get_profile_time;				//unit is second
	int sc_vxd_rescan_time;				//unit is second
	int sc_connect_timeout;				//unit is second
	int sc_pin_enabled;					// 1, MUST have PIN for SIMPLE CONFIG
	int sc_status;							//0 is not running, -1 is timeout, 1<=x<10 runing, >10 finish
	int sc_debug;
	unsigned char sc_pin[65];
	unsigned char sc_default_pin[65];
	unsigned char sc_passwd[65];
	unsigned char sc_device_name[64];
	unsigned short sc_device_type;
	int sc_ack_round;
	int sc_check_link_time;
	int sc_sync_vxd_to_root;
	unsigned int sc_control_ip;
	int sc_check_level;					//default value is 2. 0, don't check packet length; 1, check the first profile packet length; 2, check all profile packet length
	int sc_ignore_overlap;					//0, Simple Config will fail when more than one Smart Phone send config packet. 1, ignore overlap device packet.
	int sc_reset_beacon_psk;				//0, close/open interface when receive profile and try to connect remote AP; 1. reset psk and beacon only when try to connect remote AP
	int sc_security_type;
	int sc_fix_channel;					//0, don't fix channel; others, the remote AP's channel
	int sc_fix_encrypt;					//0, not fix; >0, fix
	int sc_config_type;					
	
	int sc_fix_bw;
	int sc_fix_offset;
	
	unsigned int	limit_rxloop;
	unsigned char wnmtest;				    
	unsigned char	deauth_mac[MACADDRLEN]; //CONFIG_IEEE80211W
	unsigned char	sa_req_mac[MACADDRLEN]; //CONFIG_IEEE80211W_CLI
	unsigned char   pmf_cli_test;			// CONFIG_IEEE80211W_CLI
	unsigned char   pmftest;	// CONFIG_IEEE80211W
	unsigned int	bcastSSID_inherit;
	unsigned int	beacon_rate;
	unsigned int	prsp_rate;	
	unsigned int	disable_prsp;	
	//unsigned int    channel_utili_beaconIntval;

    /* below is for 802.11k radio measurement*/
    unsigned char   dot11RadioMeasurementActivated;
    unsigned char   dot11RMLinkMeasurementActivated;
    unsigned char   dot11RMNeighborReportActivated;
    unsigned char   dot11RMBeaconPassiveMeasurementActivated;
    unsigned char   dot11RMBeaconActiveMeasurementActivated;
    unsigned char   dot11RMBeaconTableMeasurementActivated;
    unsigned char   dot11RMAPChannelReportActivated;
    unsigned int    dot11RMNeighborReportExpireTime;

	/*below is for BT Config*/
	unsigned int   rtkBtconfig; 
	unsigned char           cu_enable; /*[CLM, channel loading measurement] channel utilization calculated enable/disable*/

};

/* add for 802.11d */
struct Dot1180211CountryCodeEntry {
	unsigned int	dot11CountryCodeSwitch;	// 1=enabled; 0=disabled
	unsigned char   dot11CountryString[3];
};

// Detect STA for disappearing; added by Annie, 2010-08-10.
struct StaDetectInfo {
	unsigned char		txRprDetectPeriod;	// period to send H2C command for Tx report infoormation; in second.
};

struct Dot1180211AuthEntry {
	unsigned int	dot11AuthAlgrthm;		// 802.11 auth, could be open, shared, auto
	unsigned char	dot11PrivacyAlgrthm;	// encryption algorithm, could be none, wep40, TKIP, CCMP, wep104
	unsigned int	dot11PrivacyKeyIndex;	// this is only valid for legendary wep, 0~3 for key id.
	unsigned int	dot11PrivacyKeyLen;		// this could be 40 or 104
	int				dot11EnablePSK;			// 0: disable, bit0: WPA, bit1: WPA2
	int				dot11WPACipher;			// bit0-wep64, bit1-tkip, bit2-wrap,bit3-ccmp, bit4-wep128
	int				dot11WPA2Cipher;			// bit0-wep64, bit1-tkip, bit2-wrap,bit3-ccmp, bit4-wep128
	unsigned char	dot11PassPhrase[65];	// passphrase
	unsigned char	dot11PassPhraseGuest[65];	// passphrase of guest
	unsigned long	dot11GKRekeyTime;		// group key rekey time, 0 - disable
	unsigned long	dot11UKRekeyTime;		// unicast key rekey time, 0 - disable
	unsigned char	dot11IEEE80211W; 		// 0: disabled, 1: capable, 2:required
	unsigned char   dot11EnableSHA256; 		// 0: disabled, 1: enabled
};

struct Dot118021xAuthEntry {
	unsigned int	dot118021xAlgrthm;		// could be null, 802.1x/PSK
	unsigned int	dot118021xDefaultPort;	// used as AP mode for default ieee8021x control port
	unsigned int	dot118021xcontrolport;
	unsigned int	acct_enabled;
	unsigned long	acct_timeout_period;
	unsigned int	acct_timeout_throughput;
};

union Keytype {
	unsigned char	skey[16];
	unsigned int	lkey[4];
};

struct Dot11DefaultKeysTable {
	union Keytype	keytype[4];
};

union TSC48 {
	unsigned long long val48;

#if defined _LITTLE_ENDIAN_
	struct {
		unsigned char TSC0;
		unsigned char TSC1;
		unsigned char TSC2;
		unsigned char TSC3;
		unsigned char TSC4;
		unsigned char TSC5;
		unsigned char TSC6;
		unsigned char TSC7;
	} _byte_;

#elif defined _BIG_ENDIAN_
	struct {
		unsigned char TSC7;
		unsigned char TSC6;
		unsigned char TSC5;
		unsigned char TSC4;
		unsigned char TSC3;
		unsigned char TSC2;
		unsigned char TSC1;
		unsigned char TSC0;
	} _byte_;

#endif
};

union PN48 {
	unsigned long long val48;

#if defined _LITTLE_ENDIAN_
	struct {
		unsigned char TSC0;
		unsigned char TSC1;
		unsigned char TSC2;
		unsigned char TSC3;
		unsigned char TSC4;
		unsigned char TSC5;
		unsigned char TSC6;
		unsigned char TSC7;
	} _byte_;

#elif defined _BIG_ENDIAN_
	struct {
		unsigned char TSC7;
		unsigned char TSC6;
		unsigned char TSC5;
		unsigned char TSC4;
		unsigned char TSC3;
		unsigned char TSC2;
		unsigned char TSC1;
		unsigned char TSC0;
	} _byte_;

#endif
};

struct Dot11EncryptKey
{
	unsigned int	dot11TTKeyLen;
	unsigned int	dot11TMicKeyLen;
	union Keytype	dot11TTKey;
	union Keytype	dot11TMicKey1;
	union Keytype	dot11TMicKey2;
	union PN48		dot11TXPN48;
	union PN48		dot11RXPN48;
};

struct Dot11KeyMappingsEntry {
	unsigned int	dot11Privacy;
	unsigned int	keyInCam;	// Is my key in CAM?
	unsigned int	keyid;
	struct Dot11EncryptKey	dot11EncryptKey;
	struct Dot11EncryptKey	dot11EncryptKey2;
};

struct Dot11RsnIE {
	unsigned char	rsnie[128];
	unsigned char	rsnielen;
};

struct Dot11OperationEntry {
	unsigned char	hwaddr[MACADDRLEN];
	unsigned int	opmode;
	unsigned int	hiddenAP;
	unsigned int	dot11RTSThreshold;
	unsigned int	dot11FragmentationThreshold;
	unsigned int	dot11ShortRetryLimit;
	unsigned int	dot11LongRetryLimit;
	unsigned int	expiretime;
	unsigned int	ledtype;
	unsigned int	ledroute;
	unsigned int	iapp_enable;
	unsigned int	block_relay;
	unsigned int	deny_any;
	unsigned int	crc_log;
	unsigned int	wifi_specific;
	unsigned int	disable_txsc;
	unsigned int	disable_amsdu_txsc;
	unsigned int	disable_rxsc;
	unsigned int	disable_brsc;
	int				keep_rsnie;
	int				guest_access;
	unsigned char 	tdls_prohibited;				
	unsigned char 	tdls_cs_prohibited; 
//#ifdef CONFIG_POWER_SAVE
	unsigned int	ps_level;
	unsigned int	ps_timeout;
//#endif
};

struct Dot11RFEntry {
	unsigned int	dot11RFType;
	unsigned char	dot11channel;
	unsigned char   band5GSelected; // bit0: Band1, bit1: Band2, bit2: Band3, bit3: Band4
	unsigned int	dot11ch_low;
	unsigned int	dot11ch_hi;
	unsigned char	pwrlevelCCK_A[MAX_2G_CHANNEL_NUM];
	unsigned char	pwrlevelCCK_B[MAX_2G_CHANNEL_NUM];
	unsigned char	pwrlevelCCK_C[MAX_2G_CHANNEL_NUM];
	unsigned char	pwrlevelCCK_D[MAX_2G_CHANNEL_NUM];
	unsigned char	pwrlevelHT40_1S_A[MAX_2G_CHANNEL_NUM];
	unsigned char	pwrlevelHT40_1S_B[MAX_2G_CHANNEL_NUM];
	unsigned char   pwrlevelHT40_1S_C[MAX_2G_CHANNEL_NUM];
	unsigned char   pwrlevelHT40_1S_D[MAX_2G_CHANNEL_NUM];
	unsigned char	pwrdiffHT40_2S[MAX_2G_CHANNEL_NUM];
	unsigned char	pwrdiffHT20[MAX_2G_CHANNEL_NUM];
	unsigned char	pwrdiffOFDM[MAX_2G_CHANNEL_NUM];
	unsigned char	pwrlevel5GHT40_1S_A[MAX_5G_CHANNEL_NUM];
	unsigned char	pwrlevel5GHT40_1S_B[MAX_5G_CHANNEL_NUM];
	unsigned char	pwrlevel5GHT40_1S_C[MAX_5G_CHANNEL_NUM];
	unsigned char	pwrlevel5GHT40_1S_D[MAX_5G_CHANNEL_NUM];
	unsigned char	pwrdiff5GHT40_2S[MAX_5G_CHANNEL_NUM];
	unsigned char	pwrdiff5GHT20[MAX_5G_CHANNEL_NUM];
	unsigned char	pwrdiff5GOFDM[MAX_5G_CHANNEL_NUM];
// RTK_AC_SUPPORT
	unsigned char	pwrdiff_20BW1S_OFDM1T_A[MAX_2G_CHANNEL_NUM];	
	unsigned char	pwrdiff_40BW2S_20BW2S_A[MAX_2G_CHANNEL_NUM];	
	unsigned char	pwrdiff_OFDM2T_CCK2T_A[MAX_2G_CHANNEL_NUM];
	unsigned char	pwrdiff_40BW3S_20BW3S_A[MAX_2G_CHANNEL_NUM];	
	unsigned char	pwrdiff_4OFDM3T_CCK3T_A[MAX_2G_CHANNEL_NUM];	
	unsigned char	pwrdiff_40BW4S_20BW4S_A[MAX_2G_CHANNEL_NUM];	
	unsigned char	pwrdiff_OFDM4T_CCK4T_A[MAX_2G_CHANNEL_NUM];

	unsigned char	pwrdiff_5G_20BW1S_OFDM1T_A[MAX_5G_CHANNEL_NUM];	
	unsigned char	pwrdiff_5G_40BW2S_20BW2S_A[MAX_5G_CHANNEL_NUM];	
	unsigned char	pwrdiff_5G_40BW3S_20BW3S_A[MAX_5G_CHANNEL_NUM];	
	unsigned char	pwrdiff_5G_40BW4S_20BW4S_A[MAX_5G_CHANNEL_NUM];	
	unsigned char	pwrdiff_5G_RSVD_OFDM4T_A[MAX_5G_CHANNEL_NUM];	
	unsigned char	pwrdiff_5G_80BW1S_160BW1S_A[MAX_5G_CHANNEL_NUM];	
	unsigned char	pwrdiff_5G_80BW2S_160BW2S_A[MAX_5G_CHANNEL_NUM];	
	unsigned char	pwrdiff_5G_80BW3S_160BW3S_A[MAX_5G_CHANNEL_NUM];	
	unsigned char	pwrdiff_5G_80BW4S_160BW4S_A[MAX_5G_CHANNEL_NUM];	

	unsigned char	pwrdiff_20BW1S_OFDM1T_B[MAX_2G_CHANNEL_NUM];	
	unsigned char	pwrdiff_40BW2S_20BW2S_B[MAX_2G_CHANNEL_NUM];	
	unsigned char	pwrdiff_OFDM2T_CCK2T_B[MAX_2G_CHANNEL_NUM];
	unsigned char	pwrdiff_40BW3S_20BW3S_B[MAX_2G_CHANNEL_NUM];	
	unsigned char	pwrdiff_OFDM3T_CCK3T_B[MAX_2G_CHANNEL_NUM];
	unsigned char	pwrdiff_40BW4S_20BW4S_B[MAX_2G_CHANNEL_NUM];	
	unsigned char	pwrdiff_OFDM4T_CCK4T_B[MAX_2G_CHANNEL_NUM];

	unsigned char	pwrdiff_5G_20BW1S_OFDM1T_B[MAX_5G_CHANNEL_NUM];	
	unsigned char	pwrdiff_5G_40BW2S_20BW2S_B[MAX_5G_CHANNEL_NUM];	
	unsigned char	pwrdiff_5G_40BW3S_20BW3S_B[MAX_5G_CHANNEL_NUM];	
	unsigned char	pwrdiff_5G_40BW4S_20BW4S_B[MAX_5G_CHANNEL_NUM];	
	unsigned char	pwrdiff_5G_RSVD_OFDM4T_B[MAX_5G_CHANNEL_NUM];	
	unsigned char	pwrdiff_5G_80BW1S_160BW1S_B[MAX_5G_CHANNEL_NUM];	
	unsigned char	pwrdiff_5G_80BW2S_160BW2S_B[MAX_5G_CHANNEL_NUM];	
	unsigned char	pwrdiff_5G_80BW3S_160BW3S_B[MAX_5G_CHANNEL_NUM];	
	unsigned char	pwrdiff_5G_80BW4S_160BW4S_B[MAX_5G_CHANNEL_NUM];	

	unsigned char	pwrdiff_20BW1S_OFDM1T_C[MAX_2G_CHANNEL_NUM];  
	unsigned char	pwrdiff_40BW2S_20BW2S_C[MAX_2G_CHANNEL_NUM];  
	unsigned char	pwrdiff_OFDM2T_CCK2T_C[MAX_2G_CHANNEL_NUM];
	unsigned char	pwrdiff_40BW3S_20BW3S_C[MAX_2G_CHANNEL_NUM];  
	unsigned char	pwrdiff_4OFDM3T_CCK3T_C[MAX_2G_CHANNEL_NUM];  
	unsigned char	pwrdiff_40BW4S_20BW4S_C[MAX_2G_CHANNEL_NUM];  
	unsigned char	pwrdiff_OFDM4T_CCK4T_C[MAX_2G_CHANNEL_NUM];

	unsigned char	pwrdiff_5G_20BW1S_OFDM1T_C[MAX_5G_CHANNEL_NUM];   
	unsigned char	pwrdiff_5G_40BW2S_20BW2S_C[MAX_5G_CHANNEL_NUM];   
	unsigned char	pwrdiff_5G_40BW3S_20BW3S_C[MAX_5G_CHANNEL_NUM];   
	unsigned char	pwrdiff_5G_40BW4S_20BW4S_C[MAX_5G_CHANNEL_NUM];   
	unsigned char	pwrdiff_5G_RSVD_OFDM4T_C[MAX_5G_CHANNEL_NUM]; 
	unsigned char	pwrdiff_5G_80BW1S_160BW1S_C[MAX_5G_CHANNEL_NUM];  
	unsigned char	pwrdiff_5G_80BW2S_160BW2S_C[MAX_5G_CHANNEL_NUM];  
	unsigned char	pwrdiff_5G_80BW3S_160BW3S_C[MAX_5G_CHANNEL_NUM];  
	unsigned char	pwrdiff_5G_80BW4S_160BW4S_C[MAX_5G_CHANNEL_NUM];

	unsigned char	pwrdiff_20BW1S_OFDM1T_D[MAX_2G_CHANNEL_NUM];  
	unsigned char	pwrdiff_40BW2S_20BW2S_D[MAX_2G_CHANNEL_NUM];  
	unsigned char	pwrdiff_OFDM2T_CCK2T_D[MAX_2G_CHANNEL_NUM];
	unsigned char	pwrdiff_40BW3S_20BW3S_D[MAX_2G_CHANNEL_NUM];  
	unsigned char	pwrdiff_4OFDM3T_CCK3T_D[MAX_2G_CHANNEL_NUM];  
	unsigned char	pwrdiff_40BW4S_20BW4S_D[MAX_2G_CHANNEL_NUM];  
	unsigned char	pwrdiff_OFDM4T_CCK4T_D[MAX_2G_CHANNEL_NUM];

	unsigned char	pwrdiff_5G_20BW1S_OFDM1T_D[MAX_5G_CHANNEL_NUM];   
	unsigned char	pwrdiff_5G_40BW2S_20BW2S_D[MAX_5G_CHANNEL_NUM];   
	unsigned char	pwrdiff_5G_40BW3S_20BW3S_D[MAX_5G_CHANNEL_NUM];   
	unsigned char	pwrdiff_5G_40BW4S_20BW4S_D[MAX_5G_CHANNEL_NUM];   
	unsigned char	pwrdiff_5G_RSVD_OFDM4T_D[MAX_5G_CHANNEL_NUM]; 
	unsigned char	pwrdiff_5G_80BW1S_160BW1S_D[MAX_5G_CHANNEL_NUM];  
	unsigned char	pwrdiff_5G_80BW2S_160BW2S_D[MAX_5G_CHANNEL_NUM];  
	unsigned char	pwrdiff_5G_80BW3S_160BW3S_D[MAX_5G_CHANNEL_NUM];  
	unsigned char	pwrdiff_5G_80BW4S_160BW4S_D[MAX_5G_CHANNEL_NUM];

	unsigned int	defaultAntennaB;
	unsigned int	dot11DiversitySupport;
	unsigned int	shortpreamble;
	unsigned int	trswitch;
	unsigned int	disable_ch14_ofdm;
	unsigned int	disable_scan_ch14;
	unsigned int	disable_ch1213;
	unsigned int	xcap;
	unsigned int    xcap2;
	unsigned int	share_xcap;
	unsigned int	tssi1;
	unsigned int	tssi2;
	unsigned int	ther;
	unsigned int    ther2;
	unsigned int	deltaIQK;
	unsigned int	deltaLCK;
	unsigned int	MIMO_TR_mode;
	unsigned char	phyBandSelect;
	unsigned char	macPhyMode;
	unsigned int	smcc;
	unsigned int	smcc_t;
	unsigned int	smcc_p;
	unsigned char	trsw_pape_C9;
	unsigned char	trsw_pape_CC;
	unsigned int	tx2path;
	unsigned int	tx3path;
	unsigned int	tx4path;
	unsigned int	txbf;
	unsigned int    txbfer;
	unsigned int	txbfee;
	unsigned int	txbf_mu;
	unsigned int	target_pwr;
	unsigned char	bcn2path;
	unsigned char	bcnagc;	
	unsigned char	add_cck1M_pwr;
	unsigned int	pa_type;
	unsigned char	txpwr_reduction;
	unsigned char	rfe_type;
	unsigned int	acs_type;
	unsigned int	acs2_dis_clean_channel;
	unsigned int	acs2_round; // 200ms per round <SS_AUTO_CHNL_ACS2_TO>
	unsigned int	acs2_clm_weighting_mode;
	unsigned int	acs2_cca_cap_db;	
	unsigned char	power_percent;
	unsigned char   kfree_enable;
	unsigned int	periodicAutochannel;
	unsigned int	dynamicACS_noise;
	unsigned int	dynamicACS_idle;
};

struct ibss_priv {
	unsigned short	atim_win;
};

#if defined(CONFIG_RTL_WAPI_SUPPORT)
#define	SECURITY_INFO_WAPI		0xa5a56789
#endif
struct bss_desc {
	unsigned char	bssid[MACADDRLEN];
	unsigned char	ssid[32];
	unsigned char	*ssidptr;			// unused, for backward compatible
	unsigned short	ssidlen;
	unsigned char	meshid[MESH_ID_LEN];
	unsigned char	*meshidptr;			// unused, for backward compatible
	unsigned short	meshidlen;
	unsigned int	bsstype;
	unsigned short	beacon_prd;
	unsigned char	dtim_prd;
	unsigned int	t_stamp[2];
	struct ibss_priv	ibss_par;
	unsigned short	capability;
	unsigned char	channel;
	unsigned int	basicrate;
	unsigned int	supportrate;
	unsigned char	bdsa[MACADDRLEN];
	unsigned char	rssi;
	unsigned char	sq;
	unsigned char	network;
	/*add for P2P_SUPPORT ; for sync; it exist no matter p2p enabled or not*/
	unsigned char	p2pdevname[33];
	unsigned char	p2prole;
	unsigned short	p2pwscconfig;
	unsigned char	p2paddress[MACADDRLEN];
	/* multi-stage 2.3.0 */
	unsigned char	stage;	
    /*cfg p2p cfg p2p*/	
    #if	defined(WIFI_WPAS) || defined(RTK_NL80211)
	#if 0
    unsigned char	    p2p_ie_len;
    unsigned char	    p2p_ie[256];    
	unsigned char	    wscie_len;
	unsigned char	    wscie[256];
	#endif
	struct ht_info_elmt ht_info;
	struct ht_cap_elmt ht_cap;
	#endif
	#if	defined(WIFI_WPAS) || defined(RTK_NL80211) || defined(CONFIG_RTL_COMAPI_WLTOOLS)
    unsigned char	    wpa_ie_len;
    unsigned char	    wpa_ie[256];
    unsigned char	    rsn_ie_len;    
    unsigned char	    rsn_ie[256];
    #endif    
    /*cfg p2p cfg p2p*/
};

struct bss_type
{
	unsigned char	net_work_type;
};

struct erp_mib {
	int	protection;			// protection mechanism flag
	int	nonErpStaNum;		// none ERP client assoication num
	int	olbcDetected;		// OLBC detected
	int	olbcExpired;		// expired time of OLBC state
	int	shortSlot;			// short slot time flag
	int	ctsToSelf;			// CTStoSelf flag
	int	longPreambleStaNum;	// number of assocated STA using long preamble
};

struct wdsEntry {
	unsigned char macAddr [MACADDRLEN];
	unsigned int txRate __attribute__ ((packed));
};

struct wds_info {
	int				wdsEnabled;
	int				wdsPure;		// act as WDS bridge only, no AP function
	int				wdsPriority;	// WDS packets have higer priority
	struct wdsEntry	entry[NUM_WDS];
	int				wdsNum;			// number of WDS used
	int				wdsPrivacy;
	unsigned char	wdsWepKey[32];
	unsigned char	wdsMapingKey[NUM_WDS][32];
	int				wdsMappingKeyLen[NUM_WDS];
	int				wdsKeyId;
	unsigned char	wdsPskPassPhrase[65];
};

struct br_ext_info {
	unsigned int	nat25_disable;
	unsigned int	macclone_enable;
	unsigned int	dhcp_bcst_disable;
	int				addPPPoETag;		// 1: Add PPPoE relay-SID, 0: disable
	unsigned char	nat25_dmzMac[MACADDRLEN];
	unsigned int	nat25sc_disable;
	unsigned int	macclone_method;
};

struct Dot11DFSEntry {
	unsigned int	disable_DFS;	// 1 or 0
	unsigned int	disable_tx;		// 1 or 0
	unsigned int	DFS_timeout;	// set to 10 ms
	unsigned int	DFS_detected;	// 1 or 0
	unsigned int	NOP_timeout;	// set to 30 mins
	unsigned int	DFS_TXPAUSE_timeout; 
	unsigned int	CAC_enable;		// 1 or 0
	unsigned int	CAC_ss_counter;
	unsigned int	reserved1;
	unsigned int	reserved2;
	unsigned int	reserved3;
	unsigned int	reserved4;
};

struct Dot11hTPCEntry {
	unsigned char   tpc_enable;	// 1 or 0
	unsigned char	tpc_tx_power;
	unsigned char	tpc_link_margin;
	unsigned char   min_tx_power;
	unsigned char   max_tx_power;
};


struct MiscEntry {
	unsigned int	show_hidden_bss;
	unsigned int	ack_timeout;
	unsigned int	reserved1;
	unsigned int	reserved2;
	unsigned char	private_ie[32];
	unsigned int	private_ie_len;
	unsigned int	set_vlanid;
	unsigned int	rxInt_thrd;
	unsigned int	drvmac_lb;
	unsigned char	lb_da[6];
	unsigned int	lb_tps;
	unsigned int	lb_mlmp;
	unsigned int	groupID;
	unsigned int	vap_enable;
	unsigned int	rsv_txdesc;				// 1: enable "reserve tx desc for each interface" function; 0: disable.
	unsigned int	use_txq;
	unsigned int	func_off;
	unsigned int	auto_rts_mode;
	unsigned int	raku_only;    
	unsigned int	tv_mode;               // 0: disable, 1: enable,  2: auto
	unsigned int	stage;              
	unsigned int	forward_streaming;     // 0: disable, 1: enable	
	unsigned int	passive_ss_int;
	unsigned int	ss_delay;
	unsigned int	ss_loop_delay;
	unsigned int    chan_switch_time;
	unsigned int 	chan_switch_disable;
	unsigned int 	pkt_filter_len;
	unsigned int	autoch_ss_to;//AUTOCH_SS_SPEEDUP
	unsigned int	autoch_ss_cnt;//AUTOCH_SS_SPEEDUP
	unsigned int	autoch_1611_enable;//AUTOCH_SS_SPEEDUP
	unsigned int	max_xmitbuf_agg;
	unsigned int	max_recvbuf_agg;
	unsigned int	max_handle_xmitbuf;
	unsigned int	max_handle_recvbuf;
	unsigned int	drop_rxpkt_en;
	unsigned int	drop_rxpkt_len;
	unsigned int	drop_rxpkt_rate;
	unsigned int	G5_drop_rxpkt_rate;
	unsigned int	G5G24_drop_rxpkt_rate;
	unsigned int 	client_host_sniffer_enable;
	unsigned int	manual_priority;
};

struct ParaRecord {
	unsigned int	ACM;
	unsigned int	AIFSN;
	unsigned int	ECWmin;
	unsigned int	ECWmax;
	unsigned int	TXOPlimit;
};

struct Dot11QosEntry {
	unsigned int	dot11QosEnable;			// 0=disable, 1=enable
	unsigned int	dot11QosAPSD;			// 0=disable, 1=enable
	unsigned int	EDCAparaUpdateCount;	// default=0, increases if any STA_AC_XX_paraRecord updated
	unsigned int	EDCA_STA_config;		// WMM STA, default=0, will be set when assoc AP's EDCA para have been set
	unsigned char	WMM_IE[7];				// WMM STA, WMM IE
	unsigned char	WMM_PARA_IE[24];		// WMM EDCA Parameter IE
	unsigned int	UAPSD_AC_BE;
	unsigned int	UAPSD_AC_BK;
	unsigned int	UAPSD_AC_VI;
	unsigned int	UAPSD_AC_VO;

	struct ParaRecord STA_AC_BE_paraRecord;
	struct ParaRecord STA_AC_BK_paraRecord;
	struct ParaRecord STA_AC_VI_paraRecord;
	struct ParaRecord STA_AC_VO_paraRecord;
//#ifdef RTL_MANUAL_EDCA
	unsigned int	 ManualEDCA;					 // 0=disable, 1=enable
	struct ParaRecord AP_manualEDCA[4];
	struct ParaRecord STA_manualEDCA[4];
	unsigned char	  TID_mapping[8];		// 1: BK, 2: BE, 3: VI, 4: VO
};

struct WifiSimpleConfigEntry {
	unsigned int	wsc_enable;
	unsigned char	beacon_ie[256];
	int				beacon_ielen;
	unsigned char	probe_rsp_ie[256];
	int				probe_rsp_ielen;
	unsigned char	probe_req_ie[256];
	int				probe_req_ielen;
	unsigned char	assoc_ie[256];
	int				assoc_ielen;
	unsigned int	both_band_multicredential;
};

struct GroupBandWidthControl {
	unsigned int	GBWCMode;		// 0: disable 1: inner limitation 2: outer limitation 3: tx interface 4: rx interface 5: tx+rx interface
	unsigned char	GBWCAddr[NUM_GBWC][MACADDRLEN];
	unsigned int	GBWCNum;		// GBWC entry number, this field should be followed to GBWCAddr
	unsigned int	GBWCThrd_tx;		// Tx Threshold, in unit of kbps
	unsigned int	GBWCThrd_rx;		// Rx Threshold, in unit of kbps	
};

struct SBWC_ENTRY {
	unsigned char mac[MACADDRLEN];
	unsigned int tx_lmt;
	unsigned int rx_lmt;
};

struct StaBandwidthControl {
	unsigned int count;
	struct SBWC_ENTRY entry[NUM_SBWC];
};

struct Dot11nConfigEntry {
	unsigned int	dot11nSupportedMCS;
	unsigned int	dot11nBasicMCS;
	unsigned int	dot11nUse40M;			// 0: 20M, 1: 40M
	unsigned int	dot11n2ndChOffset;		// 0: don't care, 1: below the primary, 2: above the primary
	unsigned int	dot11nShortGIfor20M;
	unsigned int	dot11nShortGIfor40M;
	unsigned int	dot11nShortGIfor80M;
	unsigned int	dot11nSTBC;
	unsigned int	dot11nLDPC;	
	unsigned int	dot11nAMPDU;
	unsigned int	dot11nAMSDU;
	unsigned int	dot11nAMPDUSendSz;		// 8: 8K, 16: 16K, 32: 32K, 64: 64K, other: auto
	unsigned int	dot11nAMPDURevSz;
	unsigned int	dot11nAMSDURecvMax;		// 0: 4K, 1: 8K
	unsigned int	dot11nAMSDUSendTimeout;	// timeout value to queue AMSDU packets
	unsigned int	dot11nAMSDUSendNum;		// max aggregation packet number
	unsigned int	dot11curAMSDUSendNum;		// current AMSDU packet number
	unsigned int	dot11nLgyEncRstrct;		// bit0: Wep, bit1: TKIP, bit2: restrict Realtek client, bit3: forbid  N mode for legacy enc
	unsigned int	dot11nCoexist;
	unsigned int	dot11nCoexist_ch_chk;	// coexist channel chaek
	unsigned int	dot11nCoexist_obss_scan;// 0: disable, 1: active scan, 2: passive scan
	unsigned int	dot11nBGAPRssiChkTh;
	unsigned int	dot11nTxNoAck;
	unsigned int	dot11nAddBAreject;		//add for support sigma test	
	unsigned int	dot11nDisable2RCCA;
};

struct Dot11acConfigEntry {
	unsigned int	dot11SupportedVHT;	// b[1:0]: NSS1, b[3:2]: NSS2
	unsigned int	dot11VHT_TxMap;		// b[19:10]: NSS2 MCS9~0, b[9:0]: 	NSS1 MCS9~0
};

struct ReorderControlEntry {
	unsigned int	ReorderCtrlEnable;
	unsigned int	reserved;
	unsigned int	ReorderCtrlWinSz;
	unsigned int	ReorderCtrlTimeout;
	unsigned int	ReorderCtrlTimeoutCli;
};

struct Dot1180211sInfo{
    unsigned char	meshSilence;			// active when pure Mesh Mode but not enabled
    unsigned char	mesh_enable;
    unsigned char	mesh_root_enable;
    unsigned char	mesh_ap_enable;
    unsigned char	mesh_portal_enable;
    unsigned char	mesh_id[32];
    unsigned short	mesh_max_neightbor;
    unsigned char	log_enabled;			// content from webpage setting MIB_LOG_ENABLED (bitmap)
    unsigned char	dot11PassPhrase[65];	// passphrase

    unsigned int	mesh_acl_mode;
    unsigned char	mesh_acl_addr[NUM_ACL][MACADDRLEN];
    unsigned int	mesh_acl_num;

    unsigned short	mesh_reserved1;
    unsigned short	mesh_reserved2;
    unsigned short	mesh_reserved3;
    unsigned short	mesh_reserved4;
    unsigned short	mesh_reserved5;
    unsigned short	mesh_reserved6;
    unsigned short	mesh_reserved7;
    unsigned short	mesh_reserved8;
    unsigned short	mesh_reserved9;
    unsigned short	mesh_reserveda;
    unsigned short	mesh_reservedb;
    unsigned short	mesh_reservedc;
    unsigned short	mesh_reservedd;
    unsigned short	mesh_reservede;
    unsigned short	mesh_reservedf;
    unsigned char	mesh_reservedstr1[16];
};

struct EfuseEntry {
	unsigned int	enable_efuse;
};

struct VlanConfig {
	int global_vlan;		// 0/1 - global vlan disable/enable
	int is_lan;				// 1: LAN port, 0: WAN port (per-port)
	int vlan_enable;		// 0/1: disable/enable vlan (per-port)
	int vlan_tag;			// 0/1: disable/enable tagging (per-port)
	int vlan_id;			// 1~4090: vlan id (per-port)
	int vlan_pri;			// 0~7: priority; (per-port)
	int vlan_cfi;			// 0/1: cfi (per-port)
	int forwarding_rule;			// 0: disable 1:L2 bridged 2:NAT
};

/*	type define must accordint to the wapi standard	*/
typedef	enum __wapiMibType {
	wapiDisable  = 0,
	wapiTypeCert = 1,
	wapiTypePSK  = 2
} wapiMibType;

#define	WAPI_PSK_LEN	32
typedef	struct __wapiMibPSK {
	unsigned char	octet[WAPI_PSK_LEN];
	unsigned int	len;
} wapiMibPSK;

typedef	enum __wapiMibTimeoutType {
	wapiTimeoutStart    = 0,
	wapiTimeoutTotalNum = 1
} wapiMibTimeoutType;

typedef	unsigned int	wapiMibTimeout;

/*	Currently only one encrypt algorithm was supported,
*	we need NOT to record it
*/
typedef	enum __wapiMibEncryptAlgorithm {
	wapi_SMS4 = 1
} wapiMibEncryptAlgorithm;

typedef	enum __wapiMibKeyUpdateType {
	wapi_disable_update = 1,
	wapi_time_update    = 2,
	wapi_pktnum_update  = 3,
	wapi_all_update     = 4
} wapiMibKeyUpdateType;

typedef struct __wapiMibInfo {
	wapiMibType				wapiType;
	wapiMibEncryptAlgorithm	wapiUCastEncodeType;
	wapiMibEncryptAlgorithm	wapiMCastEncodeType;
	wapiMibPSK				wapiPsk;
	wapiMibKeyUpdateType	wapiUpdateUCastKeyType;
	unsigned long			wapiUpdateUCastKeyTimeout;
	unsigned long			wapiUpdateUCastKeyPktNum;
	wapiMibKeyUpdateType	wapiUpdateMCastKeyType;
	unsigned long			wapiUpdateMCastKeyTimeout;
	unsigned long			wapiUpdateMCastKeyPktNum;
	wapiMibTimeout			wapiTimeout[wapiTimeoutTotalNum];
} wapiMibInfo;

/*for HS2_SUPPORT*/
#define MAX_DSCP_EXCEPT 4
#define MAX_QOS_PRIORITY 8
struct HotSpotConfigEntry {
	unsigned int	hs_enable;
	unsigned char   hs2_ie[256];
    int             hs2_ielen;
	unsigned char	interworking_ie[256];
	int				interworking_ielen;
	unsigned char	QoSMap_ie[2][256];
	unsigned char	QoSMap_ielen[2];
	unsigned char   QoSMAP_range[MAX_QOS_PRIORITY][2];
	unsigned char   QoSMAP_except[MAX_DSCP_EXCEPT][2];
	unsigned char   nQoSMap;
	unsigned char   curQoSMap;
	unsigned char	advt_proto_ie[256];
    int				advt_proto_ielen;
	unsigned char	roam_ie[256];
    int				roam_ielen;
	unsigned char	timeadvt_ie[20];
	int				timeadvt_ielen;
	unsigned char	timezone_ie[10];
	int				timezone_ielen;
	unsigned char	MBSSID_ie[256];
	int				MBSSID_ielen;
	unsigned char   remedSvrURL[256];

	unsigned char 	serverMethod;

	unsigned char   SessionInfoURL[256];
	unsigned char	bssload_ie[5];
	//	unsigned int    channel_utili_beaconIntval;
	unsigned int	timeadvt_DTIMIntval;
	unsigned int	reqmode;
	unsigned int	disassoc_timer;
	unsigned int	validity_intval;
	unsigned char	session_url[50];
	unsigned char   sta_mac[MACADDRLEN];
    unsigned char   redir_mac[MACADDRLEN];
	unsigned char	remed_mac[MACADDRLEN];
	unsigned int    roam_enable;
	unsigned int    mmpdu_limit;
	unsigned int	bssload;
	unsigned int	radioOff;
	unsigned int	ICMPv4ECHO;
};

/*for P2P_SUPPORT*/
struct P2P_Direct {
/*cfg p2p cfg p2p*/
	unsigned char  p2p_enabled;
	unsigned char  p2p_type;
	unsigned char  p2p_state;
	unsigned char  p2p_on_discovery;
	unsigned char  p2p_intent;
	unsigned char  p2p_listen_channel;
	unsigned char  p2p_op_channel;
/*cfg p2p cfg p2p*/
	unsigned char  p2p_event_indiate;

	unsigned char  p2p_device_name[33];
	unsigned char  p2p_wsc_pin_code[9];
	unsigned short p2p_wsc_config_method;

};


#define PROFILE_NUM		5 	// must reserve one for root profile. 

struct ap_profile {
	char ssid[33];		// desired SSID in string
	int	encryption;		// 0: open, 1:wep40, 2:wep128, 3:wpa, 4:wpa2
	int	auth_type;		// authentication type. 0: open, 1: shared, 2: auto
	int	wpa_cipher;		// bit0-wep64, bit1-tkip, bit2-wrap,bit3-ccmp, bit4-wep128
	unsigned char wpa_psk[65]; // wpa psk
	int	wep_default_key;	  // wep default tx key index, 0~3
	unsigned char wep_key1[13]; // WEP key1 in hex
	unsigned char wep_key2[13]; // WEP key2 in hex
	unsigned char wep_key3[13]; // WEP key3 in hex
	unsigned char wep_key4[13]; // WEP key4 in hex	
    int MulticastCipher;	    
	unsigned char bss_PMF;	
};

struct ap_conn_profile { // ap connection profile
	int	enable_profile;
	int	profile_num;		// profile number, except root profile.
    int sortbyprofile;	
	struct ap_profile profile[PROFILE_NUM];
};

struct StaControl {
    unsigned char stactrl_enable; //0:disable, 1:enable   
    unsigned char stactrl_groupID;    
    unsigned char stactrl_prefer_band;    //0: disable, 1: enable
    unsigned int  stactrl_param_1;    // rssi threshold   
    unsigned int  stactrl_param_2;    // rssi threshold tolerance
    unsigned int  stactrl_param_3;    // timer X    
    unsigned int  stactrl_param_4;    // retry Y         
    unsigned int  stactrl_param_5;    // timer Z    
    unsigned int  stactrl_param_6;    // rssi threshold for non-prefer band   
    unsigned int  stactrl_param_7;    // kickoff times //20170103
    unsigned int  stactrl_param_8;    // kickoff times //20170106
    unsigned int  stactrl_param_9;
};
    
struct SmartRoamingProfile {
	unsigned char	block_aging;
	unsigned int	tp_max;
	unsigned char	signal_max;
	unsigned char	signal_min;
	unsigned char	load_min;
	unsigned char	speed_weight;
	unsigned char	signal_weight;
	unsigned char	load_weight;
};

struct MeshPathsel{
    unsigned char	mesh_crossbandEnable;
	unsigned char	mesh_rssiThreshold;
	unsigned char	mesh_cuThreshold;
	unsigned char	mesh_noiseThreshold;
	unsigned char	mesh_rssiWeight;
	unsigned char	mesh_cuWeight;
	unsigned char	mesh_noiseWeight;
};

struct CrossBand{
	unsigned char crossband_enable; //0:disable, 1:enable
	unsigned char crossband_prefer; //1: preferred interface 0: not preferred interface
	unsigned char crossband_assoc; //1: Yes, 0: no, interface is associated to remote AP
	unsigned char crossband_pathReady; //1: Yes, 0: No, both interfaces are connected
	unsigned char crossband_cuThreshold; //channel utilization threshold
	unsigned char crossband_noiseThreshold; //noise threshold
	unsigned char crossband_rssiThreshold; //rssi threshold
	unsigned char crossband_cuWeight; //channel utilization weight
	unsigned char crossband_noiseWeight; //noise weight
	unsigned char crossband_rssiWeight; //rssi weight
};

struct Dot11FastBSSTransitionEntry {
	int dot11FastBSSTransitionEnabled;
	unsigned char dot11FTMobilityDomainID[2];
	int dot11FTOverDSEnabled;
	int dot11FTResourceRequestSupported;
	unsigned char dot11FTR0KeyHolderID[49];
	unsigned int dot11FTR0KeyHolderIDLen;
	int dot11FTReassociationDeadline;
	int dot11FTR0KeyLifetime;
	int dot11FTR1KeyPushEnable;
	int dot11FT4way;
};

//RTK_ATM
/* Airtime Management Config Entry*/
struct atm_stainfo {
	unsigned char	hwaddr[MACADDRLEN];
	unsigned char	ipaddr[4];
	int				atm_time;
};

struct ATMConfigEntry {
	int	atm_en;								//enable/disable
	int	atm_mode;							//0:auto 1:interface 2:client
	int	atm_iftime[1+4+1];					//Root+4*VAP+VXD, interface mode
	struct atm_stainfo atm_sta[64];			//64 client list, clietn dmoe
};

struct Dot11WNMEntry {
	unsigned char dot11vBssTransEnable; 		
	unsigned char dot11vReqMode;	   
	unsigned char dot11vDiassocImminent;   		
	unsigned char sta_mac[MACADDRLEN];
	unsigned short dot11vDiassocDeadline;
	unsigned char bssTransExpiredTime;
	unsigned char Is11kDaemonOn;
	unsigned char algoType;
	unsigned char defaultPrefVal;
};

struct HalQcDebug {
	unsigned int HalDbgCom; 		
	unsigned int HalDbgLev;	   
};

struct Dot11RFDPKEntry {
	/* DPK 2G parameters */
	unsigned char 	bDPPathAOK;
	unsigned char 	bDPPathBOK;
	unsigned char 	pwsf_2g_a[3];
	unsigned char 	pwsf_2g_b[3];
	unsigned int	lut_2g_even_a[3][64];
	unsigned int	lut_2g_odd_a[3][64];
	unsigned int	lut_2g_even_b[3][64];
	unsigned int	lut_2g_odd_b[3][64];

	/* DPK 5G parameters */
	unsigned char 	is_5g_pdk_patha_ok;
	unsigned char 	is_5g_pdk_pathb_ok;
	unsigned int 	pwsf_5g_a[9];
	unsigned int 	pwsf_5g_b[9];
	unsigned int	lut_5g_even_a[9][16];
	unsigned int	lut_5g_odd_a[9][16];
	unsigned int	lut_5g_even_b[9][16];
	unsigned int	lut_5g_odd_b[9][16];
};


// driver mib
struct wifi_mib {
	unsigned int					mib_version;
	struct Dot11StationConfigEntry	dot11StationConfigEntry;
	struct Dot1180211AuthEntry		dot1180211AuthEntry;
	struct Dot118021xAuthEntry		dot118021xAuthEntry;
	struct Dot11DefaultKeysTable	dot11DefaultKeysTable;
	struct Dot11KeyMappingsEntry	dot11GroupKeysTable;
	struct Dot11KeyMappingsEntry	dot11IGTKTable; 		//CONFIG_IEEE80211W
	struct Dot11RsnIE				dot11RsnIE;
	struct Dot11OperationEntry		dot11OperationEntry;
	struct Dot11RFEntry				dot11RFEntry;
	struct bss_desc					dot11Bss;
	struct bss_type					dot11BssType;
	struct erp_mib					dot11ErpInfo;
	struct wds_info					dot11WdsInfo;
	struct br_ext_info				ethBrExtInfo;
	struct Dot11DFSEntry			dot11DFSEntry;
	struct Dot11hTPCEntry           dot11hTPCEntry;
	struct MiscEntry				miscEntry;
	struct Dot11QosEntry			dot11QosEntry;
	struct WifiSimpleConfigEntry	wscEntry;
	struct GroupBandWidthControl	gbwcEntry;
	struct StaBandwidthControl 		sbwcEntry;
	struct Dot11nConfigEntry		dot11nConfigEntry;
	struct Dot11acConfigEntry		dot11acConfigEntry;	
	struct ReorderControlEntry		reorderCtrlEntry;
	struct VlanConfig 				vlan;
	struct Dot1180211sInfo			dot1180211sInfo;
	struct Dot11KeyMappingsEntry	dot11sKeysTable;
	struct __wapiMibInfo			wapiInfo;
	struct Dot1180211CountryCodeEntry	dot11dCountry;
	struct EfuseEntry				efuseEntry;
	struct StaDetectInfo			staDetectInfo;
	struct P2P_Direct				p2p_mib;	// add for P2P_SUPPORT
	struct ap_conn_profile			ap_profile;
    struct HotSpotConfigEntry		hs2Entry;	// add for HS2_SUPPORT, Hotspot 2.0 Release 1
    struct Dot11FastBSSTransitionEntry	dot11FTEntry;
	struct StaControl			    staControl;	// add for sta control
	struct SmartRoamingProfile		sr_profile;	//add for smart roaming
	struct MeshPathsel				meshPathsel;
	struct CrossBand				crossBand; //add for crossband - keith
	struct Dot11WNMEntry 		    wnmEntry;	
    struct HalQcDebug               qc_debug;   // add for qc debug
	struct Dot11RFDPKEntry				dot11RFDPKEntry;
};

#endif // _IEEE802_MIB_H_

