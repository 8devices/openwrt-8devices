/*
 *  Header file define some p2p struct define inline functions
 *
 *  $Id: 8192cd_p2p.h,v 1.2 2010/12/21
 *
 *  Copyright (c) 2010 Realtek Semiconductor Corp.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 */

#ifndef _8192CD_P2P_H_
#define _8192CD_P2P_H_

#include "./8192cd_cfg.h"
#include "./8192cd.h"
#if defined(P2P_SUPPORT) && defined(CONFIG_OPENWRT_SDK)
#include <net/cfg80211.h>
#endif

//#define P2PMODE				((GET_MIB(priv))->p2p_mib.p2p_role)
//#define P2P_STATE				((GET_MIB(priv))->p2p_mib.p2p_state)
#define P2P_EVENT_INDICATE		(priv->p2pPtr->p2p_event_indiate)
#define P2P_PRE_MODE			(priv->p2pPtr->p2p_pre_role)
#define P2P_DISCOVERY		    (priv->p2pPtr->p2p_on_discovery)
//#include "./wps/wsc.h"
/*============== will redeclare  with wsc.h ==================*/ 


#define P2P_WILDCARD_SSID_LEN 7
#define P2P_IE_ID					221

#define MAC_LEN 					6
#define UUID_LEN					16
#define MAX_MANUFACT_LEN			64
#define MAX_MODEL_NAME_LEN			32
#define MAX_MODEL_NUM_LEN			32
#define MAX_SERIAL_NUM_LEN			32
#define MAX_DEVICE_NAME_LEN			32

#define WSC_IE_ID					221
#define WSC_VER						0x10
#define TAG_SIMPLE_CONFIG_STATE		0x1044
#define TAG_RESPONSE_TYPE2			0x103B
#define TAG_UUID_E					0x1047
#define TAG_MANUFACTURER			0x1021
#define TAG_MODEL_NAME				0x1023
#define TAG_MODEL_NUMBER			0x1024
#define TAG_SERIAL_NUM				0x1042
#define TAG_PRIMARY_DEVICE_TYPE		0x1054
#define TAG_DEVICE_NAME				0x1011
#define TAG_CONFIG_METHODS 			0x1008
#define TAG_VERSION					0x104A
#define TAG_DEVICE_PASSWORD_ID		0x1012
#define TAG_SEC_DEVICE_TYPE_LIST	0x1055

#define WFA_OUI_LEN	3
#define WFA_OUI_PLUS_TYPE_LEN	4
#define WFD_OUI_PLUS_TYPE_LEN   4

#define MAX_NOA_DESC_MUN 6
#define MAX_P2P_CLIENT_MUN 5
#define CLIENT_MODE_WAIT_TIME	30
#define WSC_MODE_WAIT_TIME	30

#define P2P_CLIENT_ASSOC_EXPIRE 120

#define P2P_device_category_id_AP  6
#define P2P_device_category_id_STA  1
#define P2P_device_sub_category_id  1



/*for P2P IE attribute ID0 use*/
enum p2p_status_code {
	P2P_SC_SUCCESS = 0,
	P2P_SC_FAIL_INFO_CURRENTLY_UNAVAILABLE = 1,
	P2P_SC_FAIL_INCOMPATIBLE_PARAMS = 2,
	P2P_SC_FAIL_LIMIT_REACHED = 3,
	P2P_SC_FAIL_INVALID_PARAMS = 4,
	P2P_SC_FAIL_UNABLE_TO_ACCOMMODATE = 5,
	P2P_SC_FAIL_PREV_PROTOCOL_ERROR = 6,
	P2P_SC_FAIL_NO_COMMON_CHANNELS = 7,
	P2P_SC_FAIL_UNKNOWN_GROUP = 8,
	P2P_SC_FAIL_BOTH_GO_INTENT_15 = 9,
	P2P_SC_FAIL_INCOMPATIBLE_PROV_METHOD = 10,
	P2P_SC_FAIL_REJECTED_BY_USER = 11,
};

/*for P2P IE attribute ID1 use*/
enum p2p_minor_reason{
	minor_case1 =1,
	minor_case2 =2,
	minor_case3 =3,
	minor_case4 =4
};


/* P2P public action frames ;
   these type frames no need assoc we must process*/
enum p2p_public_action_frame {
	P2P_GO_NEG_REQ = 0,
	P2P_GO_NEG_RESP = 1,
	P2P_GO_NEG_CONF = 2,
	P2P_INVITATION_REQ = 3,
	P2P_INVITATION_RESP = 4,
	P2P_DEV_DISC_REQ = 5,
	P2P_DEV_DISC_RESP = 6,
	P2P_PROV_DISC_REQ = 7,
	P2P_PROV_DISC_RSP = 8
};

/* P2P action frames ,we process these type frames after assoc*/
enum p2p_action_frame{
	P2P_NOA = 0,
	P2P_PRESENCE_REQ = 1,
	P2P_PRESENCE_RSP = 2,
	P2P_GO_DISCOVERY = 3
	/*4-255 reserved*/
};

enum { 
	RSP_TYPE_ENR,
	RSP_TYPE_ENR_1X, 
	RSP_TYPE_REG, 
	RSP_TYPE_AP 
};

// wsc passwd ID
enum { 
	PASS_ID_DEFAULT, // 0
	PASS_ID_USER,    // 1 
	PASS_ID_MACHINE, // 2
	PASS_ID_REKEY,	 // 3
	PASS_ID_PB, 	 // 4
	PASS_ID_REG,     // 5
	PASS_ID_RESERVED // 6
};

 // need sync with wscd
enum { 
	GO_WPS_SUCCESS = 1, 
	GO_WPS_FAIL = 2 
};



enum { 
	CONFIG_METHOD_ETH=0x2, 
	CONFIG_METHOD_PIN=0x4, 		// label(PIN)
	CONFIG_METHOD_DISPLAY=0x8  ,// (PISPLAY)		
	CONFIG_METHOD_PBC=0x80, 
	CONFIG_METHOD_KEYPAD=0x100,
	/*add for wps2.x*/		
	CONFIG_METHOD_VIRTUAL_PBC=0x280	,
	CONFIG_METHOD_PHYSICAL_PBC=0x480,
	CONFIG_METHOD_VIRTUAL_PIN=0x2008,
	CONFIG_METHOD_PHYSICAL_PIN=0x4008
};

enum p2p_role_s {
	R_P2P_GO =1	,
	R_P2P_DEVICE = 2,
	R_P2P_CLIENT =3  
};


/*note , 20140325 , cfg p2p

under PROPERTY_P2P  case, P2P_PRE_CLIENT,P2P_PRE_GO ,used for change the mode if FORMATION FAILURE  

under CFG80211_P2P case, seem don't care  P2P_PRE_CLIENT,P2P_PRE_GO 


*/
enum p2p_role_more{ 
	P2P_DEVICE=1, 
	P2P_PRE_CLIENT=2,
	P2P_CLIENT=3,
	P2P_PRE_GO=4,	 // after GO nego , we are GO and proceed WSC exchange
	P2P_TMP_GO=5	 // after GO nego , we are GO and proceed WSC exchange is done
};

// need sync with web server utility.h
enum {
	P2P_S_IDLE = 			0,			/* between state and state */	

	P2P_S_LISTEN ,			/*1 listen state */
	
	P2P_S_SCAN ,			/*2 Scan state */
	
	P2P_S_SEARCH ,			/*3 Search state*/

	// 4~14 ; show status 4 in web page
	P2P_S_PROVI_TX_REQ ,	/*4 send provision req*/	
	P2P_S_PROVI_WAIT_RSP ,	/*5 wait provision rsp*/	
	P2P_S_PROVI_RX_RSP	,	/*6 rx provision rsp*/	

	P2P_S_PROVI_RX_REQ ,	/*7 received provision req*/	
	P2P_S_PROVI_TX_RSP ,	/*8 send provision rsp*/	

	
	P2P_S_NEGO_TX_REQ ,		/*9 send NEGO req*/	
	P2P_S_NEGO_WAIT_RSP ,	/*10 waiting for NEGO rsp*/		
	P2P_S_NEGO_TX_CONF ,	/*11 send NEGO confirm*/			


	P2P_S_NEGO_RX_REQ ,		/*12 rx NEGO req */	
	P2P_S_NEGO_TX_RSP ,		/*13 send NEGO rsp */	
	P2P_S_NEGO_WAIT_CONF ,	/*14 wait NEGO conf */	

	// 15~16 ; show status 5 in web page	
	P2P_S_CLIENT_CONNECTED_DHCPC ,		/*15 p2p client Rdy connected */		
	P2P_S_CLIENT_CONNECTED_DHCPC_done, 	/*16 p2p client Rdy connected */			

	// 17~18 ; show status 6 in web page	
	P2P_S_preGO2GO_DHCPD ,			/*17 GO not start dhcpd yet */		
	P2P_S_preGO2GO_DHCPD_done, 		/*18 GO rdy start dhcpd*/

	P2P_S_back2dev 		/*exceed 20 seconds p2p client can't connected*/
};



enum {
	P2P_EVENT_RX_PROVI_REQ = 1			/* received provision req*/			
};


enum { 	
	WPS_MODE_NO_CHANGE=0,
	MODE_AP_UNCONFIG=1, 				// AP unconfigured (enrollee)
	MODE_CLIENT_UNCONFIG=2, 			// client unconfigured (enrollee) 
	MODE_CLIENT_CONFIG=3,				// client configured (registrar) 
	MODE_AP_PROXY=4, 					// AP configured (proxy)
	MODE_AP_PROXY_REGISTRAR=5,			// AP configured (proxy and registrar)
	MODE_CLIENT_UNCONFIG_REGISTRAR=6	// client unconfigured (registrar)
};

enum { 
	P2P_PIN_METHOD = 1, 
	P2P_PBC_METHOD = 2
};

enum { 
	USE_TARGET_PIN = 1, 
	USE_MY_PIN = 2 
};

enum {
	P2P_GO_PS_NONE,
	P2P_GO_PS_OPPPS,
	P2P_GO_PS_NP_NOA,
	P2P_GO_PS_CONT_NOA,
	P2P_GO_PS_NOA
};


/*==================================================================*/

/*=====================commu with web UI   start===========================*/

/* Any changed here MUST sync with web server utility.h */
struct p2p_state_event{
	unsigned char  p2p_status;
	unsigned char  p2p_event;	
	unsigned short p2p_wsc_method;		
	unsigned char  p2p_role;	
};

struct __p2p_wsc_confirm
{
	unsigned char dev_address[MAC_LEN];	
	unsigned short wsc_config_method;	
	unsigned char pincode[9];  	

};
/*=====================commu with web UI   end===========================*/




/*reg_class - Regulatory class (IEEE 802.11-2007, Annex J)
  P2P_MAX_REG_CLASSES - Maximum number of regulatory classes */

#define P2P_MAX_REG_CLASSES 10

/*P2P_MAX_REG_CLASS_CHANNELS - 
 Maximum number of channels per regulatory class */
#define P2P_MAX_REG_CLASS_CHANNELS 20


struct p2p_reg_class {

	/*reg_class - Regulatory class (IEEE 802.11-2007, Annex J)*/
	unsigned char  reg_class;

	/* channels - Number of channel entries in use */
	unsigned char channel_mun;

	/* channel - Supported channels */
	unsigned char  channel[P2P_MAX_REG_CLASS_CHANNELS];

};

/* struct p2p_channels - List of supported channels */
struct p2p_channels {
	unsigned short Id11_len;
	char country[3];
	
	/* reg_classes - Number of reg_class entries in use 
 	   corresponding to table 25(page 89) number of channels*/ 
	unsigned char  reg_class_mun; 
		
	/* struct p2p_reg_class - Supported regulatory class*/
	struct p2p_reg_class reg_class[P2P_MAX_REG_CLASSES];

};

/*============== will redeclare  with wsc.h ==================*/ 




/*P2P TAG*/
#define TAG_STATUE 				0
#define TAG_MINOR_RES_CODE 		1
#define TAG_P2P_CAPABILITY 		2
#define TAG_DEVICE_ID 			3
#define TAG_GROUP_OWNER_INTENT 	4
#define TAG_CONFIG_TIMEOUT 		5
#define TAG_LISTEN_CHANNEL 		6
#define TAG_P2P_GROUP_BSSID 	7
#define TAG_EXT_LISTEN_TIMING 	8
#define TAG_INTEN_P2P_INTERFACE_ADDR 9
#define TAG_P2P_MANAGEABILITY 	10
#define TAG_CHANNEL_LIST 		11
#define TAG_NOTICE_OF_ABSENCE 	12
#define TAG_P2P_DEVICE_INFO		 13
#define TAG_P2P_GROUP_INFO		14
#define TAG_P2P_GROUP_ID 		15
#define TAG_P2P_INTERFACE	 	16
#define TAG_OPERATION_CHANNEL	 17
#define TAG_INVITATION_FLAGS	 18




/* for P2P Capability attr*/
/*dev cap*/
#define SUPPORT_P2P_INVITATION		(BIT(5))
#define CLIENT_DISCOVERY		(BIT(1))

/*group cap*/
#define GCAP_GO (BIT(0))
#define GCAP_PRESISTENT_GO (BIT(1))
#define GCAP_GROUP_LIMIT (BIT(2))
#define GCAP_IBSS_DIST (BIT(3))
#define GCAP_GO_FORMATION (BIT(6))

struct provision_comm
{
	unsigned char dev_address[MAC_LEN];
	unsigned short wsc_config_method;	
	unsigned char channel;  
	
};


#define MAX_SEC_DEV_TYPE 3
struct device_info_s
{
	unsigned char  dev_address[6] ;	// 
	unsigned short config_method;		/*which wsc config method can provided*/	
	unsigned char pri_dev_type[8];			/*primary device type*/
	unsigned char sdv_mun;
	unsigned char sec_dev_type[MAX_SEC_DEV_TYPE][8];		/*we keep 3 sets (second device type)*/
	char devname[33];
	
};


/*	need sync with wsc.h 	*/
typedef struct _DOT11_P2P_INDICATE_WSC{
        unsigned char   EventId;
        unsigned char   IsMoreEvent;

		unsigned char 	modeSwitch ;
		char 	network_key[65] ;	
		char 	gossid[33] ;		
		unsigned char 	trigger_method ;		
		
		unsigned char 	whosPINuse ;		
		char 	PINCode[9] ;		
		unsigned char 	requestor;
		char 	interfacename[16];		                
}DOT11_P2P_INDICATE_WSC;



typedef struct p2p_device_peer {
	unsigned char inuse;
	unsigned char dev_addr[6];	

	unsigned char  dialog_token;			

	int role; 	/* p2p is which role */
	
	/*attribute ID 0 Status*/
	int status; /* enum p2p_status_code */
	
	/*attribute ID 2*/
	unsigned char dev_capab;	/* device capability*/	
	unsigned char group_capab;  /* group capability*/

	/*attribute ID3 device ID*/
	//unsigned char device_address_to_find[6] ;

	/*attribute ID4 GO intent value*/
	unsigned char intent_value;	
	unsigned char TieBreak;	

	/*attribute ID5 configure timeout*/
	unsigned char go_config_timeout;	
	unsigned char client_config_timeout;	

	/*attribute ID6 configure timeout*/
	int operating_class; // frequency band
	int listen_channle;  // 
	
	/*attribute ID7 group BSSID*/
	unsigned char go_bssid[6];

	/*attribute ID8 extended listen timing*/
	unsigned short avail_period;
	unsigned short avail_interval;

	/*attribute ID9 intented p2p interface address*/
	unsigned char p2p_interface_address[6] ;

	/*attribute ID10 manageability*/
	unsigned char manageability ; // bit(0)|bit(1)|bit(2)|

	/*attribute ID11 channel list*/
	/* Country code to use in P2P operations */
		
	struct p2p_channels channels_list;

	
	/*attribute ID 12 Notice of Absence*/
	unsigned char noa_index ;	/* noa index */	
	unsigned char noa_ct_oops ;	/* noa ctwindows and oops */	
	unsigned char noa_desc_count ;	/* noa count of noa descriptor*/	
	unsigned char noa_desc[2][13] ;	/* noa descriptor ; a go max has two desc simultaneously*/	

	/*attribute ID 13 device info*/
	struct device_info_s peer_device_info;
	
	/*attribute ID 14 group info ; more Id14's attr can represent at ID13 */
	//unsigned short p2p_interface_address[6] ;	// represent at ID9
	
	/*attribute ID 15 group ID */
	unsigned char group_bssid[6];
	unsigned char group_ssid[33];
	unsigned char group_ssid_len;

	/*attribute ID 16 group ID */
	unsigned char interface_addr_num;
	unsigned char p2p_interface_add_list[2][6] ;

	/*attribute ID 17 operating channel */
	unsigned char op_country[4];
	unsigned char op_class;
	unsigned char op_channel;	

	/*attribute ID 18 invitation flags*/
	unsigned char invitation_flag;


	/*wsc IE */
	unsigned short wsc_config_method;	
	unsigned short device_pass_id;


}P2P_DEV_T, *P2P_DEV_Tp;


/*
	in discovery phase , we can just recored the necessity info 
*/ 
struct assoc_peer
{

	unsigned char inuse;
	// ID2
	unsigned char dev_cap;
	unsigned char group_cap;	

	// ID13
	struct device_info_s devInfo;	
	unsigned char if_addr[6];		
};

struct noa_desc
{
	unsigned char count;
	unsigned int duration;
	unsigned int interval;
	unsigned int starttime;
};

// ID12
struct noa_list
{
	unsigned char index;
	unsigned char CTWindow_OppPs;	/*bit7 OppPS bit(0-6) CTWindows*/
	struct noa_desc noa_descs;
	unsigned int go_ps_type;
	unsigned int noa_counter;
	unsigned int p2p_txpause_flag;
};

#define MAX_P2P_IE_LEN 128
struct p2p_context {
	struct timer_list		p2p_search_timer_t;
	struct timer_list		p2p_find_timer_t;

	 //unsigned int   p2p_type; rename to role
     //int p2p_role;	
	 int p2p_state;
	 int p2p_on_discovery;
     int p2p_event_indiate;
     u8  use_6m_rate;

     /*=========remain on channel  related========*/    
     int pre_p2p_role;        // keep pre p2p role    20140306 add
     int pre_p2p_state;        // keep pre p2p role    20140306 add    
  
#if defined(P2P_SUPPORT) && defined(CONFIG_OPENWRT_SDK)
      /*=========remain on channel  related========*/      
      struct timer_list           remain_on_ch_timer;     
      u8  restore_channel;    
      struct ieee80211_channel    remain_on_ch_channel;           
      u64 remain_on_ch_cookie;
  
      /*=========remain on channel  related========*/
      struct timer_list  scan_deny_timer; 
      u64 send_action_id;
      //struct cfg80211_wifidirect_info   cfg80211_wdinfo;
#endif
	//unsigned int  Status;	
	
	unsigned char wsc_ie_rsp[MAX_P2P_IE_LEN]; 	 // full size maybe 32 + 8 + 8
	unsigned char wsc_ie_rsp_mun; 	 // use for action frame (eg nego rsp)	

	unsigned char wsc_ie_req[MAX_P2P_IE_LEN]; 	 // full size maybe 32 + 8 + 8
	unsigned char wsc_ie_req_mun; 	 // use for action frame (eg nego rsp)	
	
	unsigned char p2p_probe_req_ie[MAX_P2P_IE_LEN];		
	unsigned char  p2p_probe_req_ie_len;		

	unsigned char p2p_probe_rsp_ie[MAX_P2P_IE_LEN];		
	unsigned char  p2p_probe_rsp_ie_len;		

	unsigned char p2p_beacon_ie[MAX_P2P_IE_LEN];		
	unsigned char  p2p_beacon_ie_len;		

	unsigned char p2p_assocReq_ie[MAX_P2P_IE_LEN];
	unsigned char  p2p_assocReq_ie_len;

	unsigned char p2p_assoc_RspIe[MAX_P2P_IE_LEN];
	unsigned char  p2p_assoc_RspIe_len;

	unsigned char p2p_disass_ie[MAX_P2P_IE_LEN];		
	unsigned char  p2p_disass_ie_len;		

	unsigned char wfd_probe_req_ie[MAX_P2P_IE_LEN];		
	unsigned char  wfd_probe_req_ie_len;		


	unsigned char wait2listenState;
	

	unsigned char  collect_type;		// 1:from beacon   ; 2:from probe_rsp

	/*when i am GO and some p2p client assoc to me ; recored here*/
	struct assoc_peer assocPeers[MAX_P2P_CLIENT_MUN];

	/*when i am GO and some p2p dev probe_req me */
	unsigned char probe_rps_to_p2p_dev;

	/*when i am client record noa from beacon */
	struct noa_list noa_list_t;
	

	int pre_client_timeout;
	int pre_go_timeout;	

	/*for handle client discovery req/rsp , GO discovery req related */
	unsigned char  dev_dis_req_dialog_token;	
	unsigned char  dev_dis_rsp_dialog_token;		
	//	unsigned char  provision_rx_dialog_token;		

	/*for handle provision discovery related process*/
	unsigned char  presence_tx_dialog_token;	
	unsigned char  presence_rx_dialog_token;		

	/*for handle provision discovery related process*/
	unsigned char  provision_tx_dialog_token;	
	unsigned char  provision_rx_dialog_token;		

	/*for handle active send provision req and Nego Req*/
	unsigned char  target_device_role;	
	unsigned char  target_device_ssid[33];		
	unsigned char  target_device_addr[MAC_LEN];	
	unsigned char  target_device_channel;	
	unsigned short wsc_method_to_target_dev;		
	unsigned short dev_passwd_to_tar_dev;			
	unsigned char  target_dev_pin_code[9];	
	unsigned char  wsc_method_match;
	/* when receive provision req , record target dev wsc method*/
	unsigned short wsc_method_from_target_dev;			
	unsigned char  passivemode_pbc_trigger_flag;

	/*for handle action frames*/ 	
	unsigned char  provision_req_timeout;	
	
	unsigned char  requestor; // i am requestor or not



	/*for handle go-nego related process*/
	unsigned char  go_nego_tx_dialog_token;	 // my req  token
	unsigned char  go_nego_rx_dialog_token;	 // target dev's req  token		
	
	unsigned char  go_nego_on_going;		 // for lock this GO nego process	
	unsigned char  go_nego_on_going_timeout; // for lock this GO nego process		

	/*for handle action packets ; when receive go-nego req*/
	int wait_nego_conf_timeout;

	/*my  GO Negotiation data */	
	unsigned char  my_GO_ssid[33];	 /*should be DRIECT-xy+(0~23 assic)*/
	unsigned char  my_GO_ssid_len;
	unsigned char ssid_random[3]; // DIRETC-xy ; the xy
	unsigned char go_PSK[65];	 

	struct p2p_channels my_channel_list;	

	/*my P2P capability*/

	/*device capa 0~5*/	
	unsigned char service_discover;
	unsigned char p2p_client_discoverability;
	unsigned char concurrent_operations;
	unsigned char p2p_infrastructure_managed;
	unsigned char p2p_device_limit;
	unsigned char p2p_invitation;	
	/*group capa 0~6*/
	unsigned char p2p_go_role;	
	unsigned char persistent_go;
	unsigned char p2p_go_limit;	
	unsigned char intra_bss_distribution;	
	unsigned char cross_conect;	
	unsigned char persistent_reconnect;
	unsigned char go_formation;	

	
	unsigned char wps_is_ongoing;
	
	/*p2p discovery phase need related data ; no use now*/
	unsigned char spec_find_dev_addr[6];		 /*the assigned p2p device address be find in probe_req */ 
	unsigned char spec_find_primary_dev_type;	 /*the assigned primary device type be find in probe_req*/  

	struct p2p_device_peer ongoing_nego_peer ; 
	
	/*when i am on going with someone , reject another one*/ 		
	struct p2p_device_peer others_nego_tar_device; 

	unsigned char clientmode_try_connect;
	unsigned char clientmode_connected;    
	int backup_orig_use40M;
	int backup_orig_2ndchoffset;    
	unsigned char change_role_is_ongoing; // to avoid deadlock at 8192cd_close()    
};

#if defined(CONFIG_OPENWRT_SDK)
typedef struct android_wifi_priv_cmd {
	char *buf;
	int used_len;
	int total_len;
} android_wifi_priv_cmd_t;
#endif

#define CFG80211_P2P 1
#define PROPERTY_P2P 2


struct report_ssid_psk{
    char ssid_report[33];
    char ssid_len;
    char psk_report[65];
    char psk_len;    
};


#endif

