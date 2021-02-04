/*
 *  Header file defines the interface with AUTH daemon (802.1x authenticator)
 *
 *  $Id: 8192cd_security.h,v 1.3.2.1 2010/12/01 13:38:00 button Exp $
 *
 *  Copyright (c) 2009 Realtek Semiconductor Corp.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 */

#ifndef	_8192CD_SECURITY_H_
#define _8192CD_SECURITY_H_

#include "./8192cd_cfg.h"

#ifdef INCLUDE_WPS
#include "./wps/wsc.h"
#endif

typedef struct _DOT11_QUEUE_NODE
{
        unsigned short  ItemSize;
        unsigned char   Item[MAXDATALEN];
}DOT11_QUEUE_NODE;

typedef struct _DOT11_QUEUE
{
        int     Head;
        int     Tail;
        int     NumItem;
        int     MaxItem;
        DOT11_QUEUE_NODE        ItemArray[MAXQUEUESIZE];
}DOT11_QUEUE;

typedef unsigned char DOT11_KEY_RSC[8];

typedef enum{
        DOT11_KeyType_Group = 0,
        DOT11_KeyType_Pairwise = 1,
        DOT11_KeyType_IGTK = 2
} DOT11_KEY_TYPE;

typedef enum{
	DOT11_EAPOL_GROUP_INDEX = 0,
	DOT11_EAPOL_PAIRWISE_INDEX = 3
} DOT11_EAPOL_KEY_INDEX;

typedef enum{
        DOT11_KeyUsage_ENC,
        DOT11_KeyUsage_MIC
} DOT11_KEY_USAGE;

typedef enum{
        DOT11_Role_Auth,
        DOT11_Role_Supp
} DOT11_ROLE;

typedef enum{
        DOT11_VARIABLE_MACEnable,
        DOT11_VARIABLE_SystemAuthControl,
        DOT11_VARIABLE_AuthControlledPortStatus,
        DOT11_VARIABLE_AuthControlledPortControl,
        DOT11_VARIABLE_AuthenticationType,
        DOT11_VARIABLE_KeyManagement,
        DOT11_VARIABLE_MulticastCipher,
        DOT11_VARIABLE_UnicastCipher
} DOT11_VARIABLE_TYPE;

typedef enum{
        DOT11_SysAuthControl_Disabled,
        DOT11_SysAuthControl_Enabled
} DOT11_SYSTEM_AUTHENTICATION_CONTROL;

typedef enum{
        DOT11_PortControl_ForceUnauthorized,
        DOT11_PortControl_ForceAuthorized,
        DOT11_PortControl_Auto
} DOT11_PORT_CONTROL;

typedef enum{
        DOT11_PortStatus_Unauthorized,
        DOT11_PortStatus_Authorized,
        DOT11_PortStatus_Guest
}DOT11_PORT_STATUS;

typedef enum{
        DOT11_Association_Fail,
        DOT11_Association_Success
}DOT11_ASSOCIATION_RESULT;

typedef enum{
        DOT11_AuthKeyType_RSN = 1,
        DOT11_AuthKeyType_PSK = 2,
#ifdef CONFIG_IEEE80211R
        DOT11_AuthKeyType_FT8021x = 3,
        DOT11_AuthKeyType_FTPSK = 4,
#endif
         DOT11_AuthKeyType_802_1X_SHA256 = 5,		//CONFIG_IEEE80211W_CLI
         DOT11_AuthKeyType_PSK_SHA256 = 6
} DOT11_AUTHKEY_TYPE;

typedef enum{
	DOT11_AuthKeyType_RSN_MAP = 1,
        DOT11_AuthKeyType_PSK_MAP = 2,
        DOT11_AuthKeyType_NonRSN802dot1x_MAP = 4
} DOT11_AUTHKEY_TYPE_MAP;

typedef enum{
	DOT11_Ioctl_Query = 0,
	DOT11_Ioctl_Set = 1
} DOT11_Ioctl_Flag;

typedef enum{
        DOT11_ENC_NONE  = 0,
        DOT11_ENC_WEP40 = 1,
        DOT11_ENC_TKIP  = 2,
        DOT11_ENC_WRAP  = 3,
        DOT11_ENC_CCMP  = 4,
        DOT11_ENC_WEP104= 5,
        DOT11_ENC_BIP=	  6,
        DOT11_ENC_WAPI= 6
} DOT11_ENC_ALGO;

typedef enum{
        DOT11_ENC_NONE_MAP  = 1,
        DOT11_ENC_WEP40_MAP = 2,
        DOT11_ENC_TKIP_MAP  = 4,
        DOT11_ENC_WRAP_MAP  = 8,
        DOT11_ENC_CCMP_MAP  = 16,
        DOT11_ENC_WEP104_MAP= 32
} DOT11_ENC_ALGO_MAP;


typedef enum{
        DOT11_EVENT_NO_EVENT = 1,
        DOT11_EVENT_REQUEST = 2,
        DOT11_EVENT_ASSOCIATION_IND = 3,
        DOT11_EVENT_ASSOCIATION_RSP = 4,
        DOT11_EVENT_AUTHENTICATION_IND = 5,
        DOT11_EVENT_REAUTHENTICATION_IND = 6,
        DOT11_EVENT_DEAUTHENTICATION_IND = 7,
        DOT11_EVENT_DISASSOCIATION_IND = 8,
        DOT11_EVENT_DISCONNECT_REQ = 9,
        DOT11_EVENT_SET_802DOT11 = 10,
        DOT11_EVENT_SET_KEY = 11,
        DOT11_EVENT_SET_PORT = 12,
        DOT11_EVENT_DELETE_KEY = 13,
        DOT11_EVENT_SET_RSNIE = 14,
        DOT11_EVENT_GKEY_TSC = 15,
        DOT11_EVENT_MIC_FAILURE = 16,
        DOT11_EVENT_ASSOCIATION_INFO = 17,
        DOT11_EVENT_INIT_QUEUE = 18,
        DOT11_EVENT_EAPOLSTART = 19,
#ifdef CONFIG_IEEE80211W
		DOT11_EVENT_SA_QUERY=20,
		DOT11_EVENT_SA_QUERY_RSP=21,
#endif
        DOT11_EVENT_ACC_SET_EXPIREDTIME = 31,
        DOT11_EVENT_ACC_QUERY_STATS = 32,
        DOT11_EVENT_ACC_QUERY_STATS_ALL = 33,
        DOT11_EVENT_REASSOCIATION_IND = 34,
        DOT11_EVENT_REASSOCIATION_RSP = 35,
        DOT11_EVENT_STA_QUERY_BSSID = 36,
        DOT11_EVENT_STA_QUERY_SSID = 37,
        DOT11_EVENT_EAP_PACKET = 41,

#ifdef RTL_WPA2_PREAUTH
        DOT11_EVENT_EAPOLSTART_PREAUTH = 45,
        DOT11_EVENT_EAP_PACKET_PREAUTH = 46,
#endif

        DOT11_EVENT_WPA2_MULTICAST_CIPHER = 47,
        DOT11_EVENT_WPA_MULTICAST_CIPHER = 48,

#ifdef WIFI_SIMPLE_CONFIG
		DOT11_EVENT_WSC_SET_IE = 55,
		DOT11_EVENT_WSC_PROBE_REQ_IND = 56,
		DOT11_EVENT_WSC_PIN_IND = 57,
		DOT11_EVENT_WSC_ASSOC_REQ_IE_IND = 58,


#ifdef INCLUDE_WPS

		DOT11_EVENT_WSC_SET_MIB=42,
		DOT11_EVENT_WSC_GET_MIB=43,
		DOT11_EVENT_REQUEST_F_INCLUDE_WPS=44,
		

		DOT11_EVENT_WSC_INIT_IND = 70,
		DOT11_EVENT_WSC_EXIT_IND = 71,
		DOT11_EVENT_WSC_TERM_IND = 72,
		DOT11_EVENT_WSC_GETCONF_IND = 73,
		DOT11_EVENT_WSC_PUTCONF_IND = 74,
		DOT11_EVENT_WSC_LEDCONTROL_IND = 75,
		DOT11_EVENT_WSC_SENDMSG_IND = 76,
		DOT11_EVENT_WSC_PUTCONF = 77,
		DOT11_EVENT_WSC_SOAP = 78,
		DOT11_EVENT_WSC_PIN = 79,
		DOT11_EVENT_WSC_PBC = 80,
		DOT11_EVENT_WSC_SYS = 81,
		DOT11_EVENT_WSC_PUTWLANREQUEST_IND = 82,
		DOT11_EVENT_WSC_PUTPKT = 83,
		DOT11_EVENT_WSC_GETDEVINFO = 84,
		DOT11_EVENT_WSC_M2M4M6M8 = 85,
		DOT11_EVENT_WSC_PUTWLANRESPONSE = 86,
		DOT11_EVENT_WSC_PUTMESSAGE = 87,
		DOT11_EVENT_WSC_PUTWLREQ_PROBEIND = 88,
		DOT11_EVENT_WSC_PUTWLREQ_STATUSIND = 89,
#endif

#ifdef P2P_SUPPORT
		DOT11_EVENT_WSC_SWITCH_MODE = 100,
		DOT11_EVENT_WSC_STOP = 101,				
		DOT11_EVENT_WSC_SWITCH_WLAN_MODE = 108,						
#endif
	/* support  Assigned MAC Addr,Assigned SSID,dymanic change STA's PIN code, 2011-0505 */	
		DOT11_EVENT_WSC_SET_MY_PIN = 102,
		DOT11_EVENT_WSC_SPEC_SSID = 103,
		DOT11_EVENT_WSC_SPEC_MAC_IND = 104,
	/* support  Assigned MAC Addr,Assigned SSID,dymanic change STA's PIN code, 2011-0505 */			
				
#endif
#ifdef	CONFIG_RTK_MESH
	DOT11_EVENT_PATHSEL_GEN_RREQ = 59,
	DOT11_EVENT_PATHSEL_GEN_RERR = 60,
	DOT11_EVENT_PATHSEL_RECV_RREQ = 61,
	DOT11_EVENT_PATHSEL_RECV_RREP = 62,
	DOT11_EVENT_PATHSEL_RECV_RERR = 63,
	DOT11_EVENT_PATHSEL_RECV_PANN = 65,
	DOT11_EVENT_PATHSEL_RECV_RANN = 66,
#endif // CONFIG_RTK_MESH
#ifdef CONFIG_RTL_WAPI_SUPPORT
	DOT11_EVENT_WAPI_INIT_QUEUE =67,
	DOT11_EVENT_WAPI_READ_QUEUE = 68,
	DOT11_EVENT_WAPI_WRITE_QUEUE  =69,
#endif
#if defined(CONFIG_RTL_COMAPI_CFGFILE) && defined(WIFI_SIMPLE_CONFIG)
#ifdef INCLUDE_WPS
	DOT11_EVENT_WSC_START_IND = 90,
	//EV_MODE, EV_STATUS, EV_MEHOD, EV_STEP, EV_OOB
	DOT11_EVENT_WSC_MODE_IND = 91,
	DOT11_EVENT_WSC_STATUS_IND = 92,
	DOT11_EVENT_WSC_METHOD_IND = 93,
	DOT11_EVENT_WSC_STEP_IND = 94,
	DOT11_EVENT_WSC_OOB_IND = 95,
#else
	DOT11_EVENT_WSC_START_IND = 70,
	//EV_MODE, EV_STATUS, EV_MEHOD, EV_STEP, EV_OOB
	DOT11_EVENT_WSC_MODE_IND = 71,
	DOT11_EVENT_WSC_STATUS_IND = 72,
	DOT11_EVENT_WSC_METHOD_IND = 73,
	DOT11_EVENT_WSC_STEP_IND = 74,
	DOT11_EVENT_WSC_OOB_IND = 75,
#endif
#endif
	DOT11_EVENT_WSC_RM_PBC_STA=106,
	DOT11_EVENT_WSC_CHANGE_MAC_IND = 107,		
#ifdef HS2_SUPPORT
	DOT11_EVENT_WNM_NOTIFY = 109,
	DOT11_EVENT_GAS_INIT_REQ = 110,
	DOT11_EVENT_GAS_COMEBACK_REQ = 111,
	DOT11_EVENT_HS2_SET_IE = 112,
	DOT11_EVENT_HS2_GAS_RSP = 113,
	DOT11_EVENT_HS2_GET_TSF = 114,
	DOT11_EVENT_HS2_TSM_REQ = 115,
	DOT11_EVENT_HS2_GET_RSN = 116,
	DOT11_EVENT_HS2_GET_MMPDULIMIT=117,
	DOT11_EVENT_WNM_DEAUTH_REQ = 118,
	DOT11_EVENT_QOS_MAP_CONF = 119,
#endif
#ifdef CONFIG_IEEE80211W
	DOT11_EVENT_SET_PMF = 120,
	DOT11_EVENT_GET_IGTK_PN = 121,
	DOT11_EVENT_INIT_PMF = 122,	// HS2 R2 logo test
#endif	
#ifdef RSSI_MONITOR_NCR
	DOT11_EVENT_RSSI_MONITOR_REPORT = 122,
	DOT11_EVENT_RSSI_MONITOR_SETTYPE = 123,	
#endif	
#ifdef CONFIG_IEEE80211R
	DOT11_EVENT_FT_GET_EVENT		= 125,
	DOT11_EVENT_FT_IMD_ASSOC_IND	= 126,
	DOT11_EVENT_FT_GET_KEY			= 127,
	DOT11_EVENT_FT_SET_KEY			= 128,
	DOT11_EVENT_FT_PULL_KEY_IND		= 129,
	DOT11_EVENT_FT_ASSOC_IND		= 130,
	DOT11_EVENT_FT_KEY_EXPIRE_IND	= 131,
	DOT11_EVENT_FT_ACTION_IND		= 132,
	DOT11_EVENT_FT_QUERY_INFO		= 133,
	DOT11_EVENT_FT_SET_INFO			= 134,
	DOT11_EVENT_FT_AUTH_INSERT_R0	= 135,
	DOT11_EVENT_FT_AUTH_INSERT_R1	= 136,
	DOT11_EVENT_FT_TRIGGER_EVENT	= 137,
#endif
#if defined(SUPPORT_UCFGING_LED) 
	DOT11_EVENT_UCFGING_LED	= 139,
#endif
#ifdef INDICATE_LINK_CHANGE
	DOT11_EVENT_LINK_CHANGE_IND = 140,
#endif
#ifdef USER_ADDIE
	DOT11_EVENT_USER_SETIE	= 141,
#endif
	DOT11_EVENT_UNKNOWN = 142
	
} DOT11_EVENT;

#ifdef WIFI_SIMPLE_CONFIG
enum {SET_IE_FLAG_BEACON=1, SET_IE_FLAG_PROBE_RSP=2, SET_IE_FLAG_PROBE_REQ=3,
		SET_IE_FLAG_ASSOC_RSP=4, SET_IE_FLAG_ASSOC_REQ=5};
#endif

#ifdef HS2_SUPPORT
/* Hotspot 2.0 Release 1 */
enum {SET_IE_FLAG_INTERWORKING=1, SET_IE_FLAG_ADVT_PROTO=2, SET_IE_FLAG_ROAMING=3,
		SET_IE_FLAG_HS2=4, SET_IE_FLAG_TIMEADVT=5, SET_IE_FLAG_TIMEZONE=6, SET_IE_FLAG_PROXYARP=7,
        SET_IE_FLAG_MBSSID=8, SET_IE_FLAG_REMEDSVR=9, SET_IE_FLAG_MMPDULIMIT=10, SET_IE_FLAG_ICMPv4ECHO=11,
        SET_IE_FLAG_SessionInfoURL=12, SET_IE_FLAG_QOSMAP=13};
#endif

#ifdef USER_ADDIE
enum {SET_IE_FLAG_INSERT=1, SET_IE_FLAG_DELETE=2};
#endif

typedef struct _DOT11_GENERAL{
        unsigned char   EventId;
        unsigned char   IsMoreEvent;
        unsigned char   *Data;
}DOT11_GENERAL;

typedef struct _DOT11_NOEVENT{
        unsigned char   EventId;
        unsigned char   IsMoreEvent;
}DOT11_NO_EVENT;

typedef struct _DOT11_REQUEST{
        unsigned char   EventId;
}DOT11_REQUEST;

typedef struct _DOT11_WPA2_MULTICAST_CIPHER{
        unsigned char   EventId;
        unsigned char   IsMoreEvent;
        unsigned char	MulticastCipher;
}DOT11_WPA2_MULTICAST_CIPHER;

typedef struct _DOT11_WPA_MULTICAST_CIPHER{
        unsigned char   EventId;
        unsigned char   IsMoreEvent;
        unsigned char	MulticastCipher;
}DOT11_WPA_MULTICAST_CIPHER;

typedef struct _DOT11_ASSOCIATION_IND{
        unsigned char   EventId;
        unsigned char   IsMoreEvent;
        unsigned char            MACAddr[MACADDRLEN];
        unsigned short  RSNIELen;
        unsigned char            RSNIE[MAXRSNIELEN]; // include ID and Length by kenny
}DOT11_ASSOCIATION_IND;

typedef struct _DOT11_ASSOCIATION_RSP{
        unsigned char   EventId;
        unsigned char   IsMoreEvent;
        char            MACAddr[MACADDRLEN];
        unsigned char   Status;
}DOT11_ASSOCIATIIN_RSP;

typedef struct _DOT11_REASSOCIATION_IND{
        unsigned char   EventId;
        unsigned char   IsMoreEvent;
        char            MACAddr[MACADDRLEN];
        unsigned short  RSNIELen;
        char            RSNIE[MAXRSNIELEN];
        char            OldAPaddr[MACADDRLEN];
}DOT11_REASSOCIATION_IND;

typedef struct _DOT11_REASSOCIATION_RSP{
        unsigned char   EventId;
        unsigned char   IsMoreEvent;
        char            MACAddr[MACADDRLEN];
        unsigned char   Status;
        char            CurrAPaddr[MACADDRLEN];
}DOT11_REASSOCIATIIN_RSP;

typedef struct _DOT11_AUTHENTICATION_IND{
	unsigned char   EventId;
        unsigned char   IsMoreEvent;
        char            MACAddr[MACADDRLEN];
}DOT11_AUTHENTICATION_IND;

typedef struct _DOT11_REAUTHENTICATION_IND{
        unsigned char   EventId;
        unsigned char   IsMoreEvent;
        char            MACAddr[MACADDRLEN];
}DOT11_REAUTHENTICATION_IND;

#ifdef WIFI_SIMPLE_CONFIG
typedef struct _DOT11_PROBE_REQUEST_IND{
        unsigned char   EventId;
        unsigned char   IsMoreEvent;
        char            MACAddr[MACADDRLEN];
        unsigned short  ProbeIELen;
        char            ProbeIE[PROBEIELEN];
}DOT11_PROBE_REQUEST_IND;

typedef struct _DOT11_WSC_ASSOC_IND{
        unsigned char   EventId;
        unsigned char   IsMoreEvent;
        char            MACAddr[MACADDRLEN];
        unsigned short  AssocIELen;
        char            AssocIE[PROBEIELEN];
	  unsigned char wscIE_included;
}DOT11_WSC_ASSOC_IND;

typedef struct _DOT11_GETSET_MIB {
	unsigned char EventId;
	unsigned char IsMoreEvent;
	unsigned char cmd[50];
}DOT11_GETSET_MIB;

#endif

typedef struct _DOT11_DEAUTHENTICATION_IND{
	unsigned char   EventId;
	unsigned char   IsMoreEvent;
	char            MACAddr[MACADDRLEN];
	unsigned long	tx_packets;       // == transmited packets
	unsigned long	rx_packets;       // == received packets
	unsigned long	tx_bytes;         // == transmited bytes
	unsigned long	rx_bytes;         // == received bytes
	unsigned long  	Reason;
}DOT11_DEAUTHENTICATION_IND;

typedef struct _DOT11_DISASSOCIATION_IND{
        unsigned char   EventId;
        unsigned char   IsMoreEvent;
        char            MACAddr[MACADDRLEN];
	unsigned long	tx_packets;       // == transmited packets
	unsigned long	rx_packets;       // == received packets
	unsigned long	tx_bytes;         // == transmited bytes
	unsigned long	rx_bytes;         // == received bytes
	unsigned long  	Reason;
}DOT11_DISASSOCIATION_IND;

typedef struct _DOT11_DISCONNECT_REQ{
        unsigned char   EventId;
        unsigned char   IsMoreEvent;
        unsigned short  Reason;
        char            MACAddr[MACADDRLEN];
}DOT11_DISCONNECT_REQ;

typedef struct _DOT11_SET_802DOT11{
        unsigned char   EventId;
        unsigned char   IsMoreEvent;
        unsigned char   VariableType;
        unsigned char   VariableValue;
	char            MACAddr[MACADDRLEN];
}DOT11_SET_802DOT11;

typedef struct _DOT11_SET_KEY{
        unsigned char   EventId;
        unsigned char   IsMoreEvent;
	unsigned long   KeyIndex;
	unsigned long   KeyLen;
	unsigned char   KeyType;
	unsigned char	EncType;
        unsigned char   MACAddr[MACADDRLEN];
	DOT11_KEY_RSC   KeyRSC;
	unsigned char   KeyMaterial[64];
}DOT11_SET_KEY;

typedef struct _DOT11_DELETE_KEY{
        unsigned char   EventId;
        unsigned char   IsMoreEvent;
        char            MACAddr[MACADDRLEN];
        unsigned char   KeyType;
}DOT11_DELETE_KEY;

typedef struct _DOT11_SET_RSNIE{
        unsigned char   EventId;
        unsigned char   IsMoreEvent;
	unsigned short	Flag;
        unsigned short  RSNIELen;
        char            RSNIE[MAXRSNIELEN];
	char            MACAddr[MACADDRLEN];
}DOT11_SET_RSNIE;

typedef struct _DOT11_SET_PORT{
        unsigned char EventId;
        unsigned char PortStatus;
        unsigned char PortType;
        unsigned char MACAddr[MACADDRLEN];
}DOT11_SET_PORT;

typedef struct _DOT11_GKEY_TSC{
        unsigned char   EventId;
        unsigned char   IsMoreEvent;
	unsigned char	KeyTSC[8];
}DOT11_GKEY_TSC;

typedef struct _DOT11_MIC_FAILURE{
        unsigned char   EventId;
        unsigned char   IsMoreEvent;
        char            MACAddr[MACADDRLEN];
}DOT11_MIC_FAILURE;

typedef struct _DOT11_STA_QUERY_BSSID{
        unsigned char   EventId;
        unsigned char   IsMoreEvent;
        unsigned long   IsValid;
        char            Bssid[MACADDRLEN];
}DOT11_STA_QUERY_BSSID;

typedef struct _DOT11_STA_QUERY_SSID{
        unsigned char   EventId;
        unsigned char   IsMoreEvent;
        unsigned long   IsValid;
        char            ssid[32];
        int             ssid_len;
}DOT11_STA_QUERY_SSID;

typedef struct _DOT11_EAPOL_START{
        unsigned char   EventId;
        unsigned char   IsMoreEvent;
        char            MACAddr[MACADDRLEN];
}DOT11_EAPOL_START;

typedef struct _DOT11_SET_EXPIREDTIME{
        unsigned char EventId;
        unsigned char IsMoreEvent;
        unsigned char MACAddr[MACADDRLEN];
		unsigned long ExpireTime;
}DOT11_SET_EXPIREDTIME;

typedef struct _DOT11_QUERY_STATS{
	unsigned char   EventId;
	unsigned char   IsMoreEvent;
	unsigned char	MACAddr[MACADDRLEN];
	unsigned long   IsSuccess;
	unsigned long	tx_packets;       // == transmited packets
	unsigned long	rx_packets;       // == received packets
	unsigned long	tx_bytes;         // == transmited bytes
	unsigned long	rx_bytes;         // == received bytes
}DOT11_QUERY_STATS;

typedef struct _DOT11_EAP_PACKET{
	unsigned char	EventId;
	unsigned char	IsMoreEvent;
	unsigned short  packet_len;
	unsigned char	packet[1550];
}DOT11_EAP_PACKET;

#ifdef INCLUDE_WPS
#ifndef CONFIG_MSC
typedef struct _DOT11_EVENT_PACKET{
	unsigned char	EventId;
	unsigned char	EventType;
	unsigned short  packet_len;
	unsigned char	packet[1550];
}DOT11_EVENT_PACKET;
#endif
#endif

typedef DOT11_ASSOCIATION_IND DOT11_AUTH_IND;

#ifdef WIFI_SIMPLE_CONFIG
typedef struct _DOT11_WSC_PIN_IND{
	unsigned char EventId;
	unsigned char IsMoreEvent;
	unsigned char code[256];
} DOT11_WSC_PIN_IND;

#ifdef CONFIG_RTL_COMAPI_CFGFILE
typedef struct _DOT11_WSC_IND{
	unsigned char EventId;
	unsigned char IsMoreEvent;
	unsigned int value;
} DOT11_WSC_IND;
#endif  //CONFIG_RTL_COMAPI_CFGFILE
#endif
#ifdef RSSI_MONITOR_NCR
typedef		struct _DOT11_RSSIM_SET_TYPE {
	unsigned char 		EventId;
	unsigned char		type;
	unsigned char		hwaddr[MACADDRLEN];
} DOT11_RSSIM_SET_TYPE;
#endif
#ifdef CONFIG_IEEE80211R
typedef struct _DOT11_FT_IMD_ASSOC_IND{
	unsigned char EventId;
	unsigned char IsMoreEvent;
	unsigned char MACAddr[MACADDRLEN];
} __WLAN_ATTRIB_PACK__ DOT11_FT_IMD_ASSOC_IND;

typedef struct _DOT11_FT_PULL_KEY_IND{
	unsigned char EventId;
	unsigned char IsMoreEvent;
	unsigned char Type;
	unsigned char r0kh_id[MAX_R0KHID_LEN];
	unsigned int Length;
	unsigned char nonce[FT_R0KH_R1KH_PULL_NONCE_LEN];
	unsigned char pmk_r0_name[FT_PMKID_LEN];
	unsigned char r1kh_id[MACADDRLEN];
	unsigned char s1kh_id[MACADDRLEN];
} __WLAN_ATTRIB_PACK__ DOT11_FT_PULL_KEY_IND;

enum _FTKEY_TYPE{
	FTKEY_TYPE_PUSH		= 1,
	FTKEY_TYPE_PULL		= 2,
};

typedef struct _DOT11_FT_GET_KEY_PROTO{
	unsigned char EventId;
	unsigned char IsMoreEvent;
	unsigned char Type;
} __WLAN_ATTRIB_PACK__ DOT11_FT_GET_KEY_PROTO;

typedef struct _DOT11_FT_GET_KEY{
	unsigned char EventId;
	unsigned char IsMoreEvent;
	unsigned char Type;
	unsigned int Length;
	unsigned char r1kh_id[MACADDRLEN];
	unsigned char s1kh_id[MACADDRLEN];
} __WLAN_ATTRIB_PACK__ DOT11_FT_GET_KEY;

typedef struct _DOT11_FT_GET_KEY_PUSH{
	unsigned char EventId;
	unsigned char IsMoreEvent;
	unsigned char Type;
	unsigned int Length;
	unsigned int timestamp;
	unsigned char r1kh_id[MACADDRLEN];
	unsigned char s1kh_id[MACADDRLEN];
	unsigned char pmk_r0_name[FT_PMKID_LEN];
	unsigned char pmk_r1[FT_PMK_LEN];
	unsigned char pmk_r1_name[FT_PMKID_LEN];
	unsigned short pairwise;
} __WLAN_ATTRIB_PACK__ DOT11_FT_GET_KEY_PUSH;

typedef struct _DOT11_FT_GET_KEY_PULL{
	unsigned char EventId;
	unsigned char IsMoreEvent;
	unsigned char Type;
	unsigned int Length;
	unsigned char nonce[FT_R0KH_R1KH_PULL_NONCE_LEN];
	unsigned char r1kh_id[MACADDRLEN];
	unsigned char s1kh_id[MACADDRLEN];
	unsigned char pmk_r1[FT_PMK_LEN];
	unsigned char pmk_r1_name[FT_PMKID_LEN];
	unsigned short pairwise;
} __WLAN_ATTRIB_PACK__ DOT11_FT_GET_KEY_PULL;

typedef struct _DOT11_FT_SET_KEY_PROTO{
	unsigned char EventId;
	unsigned char IsMoreEvent;
	unsigned char Type;
} __WLAN_ATTRIB_PACK__ DOT11_FT_SET_KEY_PROTO;

typedef struct _DOT11_FT_SET_KEY_PUSH{
	unsigned char EventId;
	unsigned char IsMoreEvent;
	unsigned char Type;
	unsigned int Length;
	unsigned int timestamp;
	unsigned char r1kh_id[MACADDRLEN];
	unsigned char s1kh_id[MACADDRLEN];
	unsigned char pmk_r0_name[FT_PMKID_LEN];
	unsigned char pmk_r1[FT_PMK_LEN];
	unsigned char pmk_r1_name[FT_PMKID_LEN];
	unsigned short pairwise;
} __WLAN_ATTRIB_PACK__ DOT11_FT_SET_KEY_PUSH;

typedef struct _DOT11_FT_SET_KEY_PULL{
	unsigned char EventId;
	unsigned char IsMoreEvent;
	unsigned char Type;
	unsigned int Length;
	unsigned char nonce[FT_R0KH_R1KH_PULL_NONCE_LEN];
	unsigned char r1kh_id[MACADDRLEN];
	unsigned char s1kh_id[MACADDRLEN];
	unsigned char pmk_r1[FT_PMK_LEN];
	unsigned char pmk_r1_name[FT_PMKID_LEN];
	unsigned short pairwise;
} __WLAN_ATTRIB_PACK__ DOT11_FT_SET_KEY_PULL;

typedef struct _DOT11_FT_ASSOC_IND{
	unsigned char EventId;
	unsigned char IsMoreEvent;
	unsigned char MACAddr[MACADDRLEN];
} __WLAN_ATTRIB_PACK__ DOT11_FT_ASSOC_IND;

typedef struct _DOT11_FT_KEY_EXPIRE_IND{
	unsigned char EventId;
	unsigned char IsMoreEvent;
	unsigned char MACAddr[MACADDRLEN];
} __WLAN_ATTRIB_PACK__ DOT11_FT_KEY_EXPIRE_IND;

enum _FT_ACTION_CODE{
	ACTION_CODE_REQUEST		= 0,
	ACTION_CODE_RESPONSE	= 1
};

typedef struct _DOT11_FT_ACTION{
	unsigned char EventId;
	unsigned char IsMoreEvent;
	unsigned char MACAddr[MACADDRLEN];
	unsigned char ActionCode;
	unsigned int packet_len;
	unsigned char packet[MAX_FTACTION_LEN];
} __WLAN_ATTRIB_PACK__ DOT11_FT_ACTION;

// Following are for Auth daemon
typedef struct _DOT11_QUERY_FT_INFORMATION
{
    unsigned char EventId;
    unsigned char IsMoreEvent;
	unsigned char sta_addr[MACADDRLEN];
	unsigned char ssid[32];
	unsigned int ssid_len;
	unsigned char mdid[2];
	unsigned char r0kh_id[MAX_R0KHID_LEN];
	unsigned int r0kh_id_len;
	unsigned char bssid[MACADDRLEN];
	unsigned char over_ds;
	unsigned char res_request;
} DOT11_QUERY_FT_INFORMATION, *PDOT11_QUERY_FT_INFORMATION;

typedef struct _DOT11_SET_FT_INFORMATION
{
    unsigned char EventId;
    unsigned char IsMoreEvent;
	unsigned char sta_addr[MACADDRLEN];
	unsigned char UnicastCipher;
	unsigned char MulticastCipher;
	unsigned char bInstallKey;
} DOT11_SET_FT_INFORMATION, *PDOT11_SET_FT_INFORMATION;

typedef struct _DOT11_AUTH_FT_INSERT_R0_KEY
{
    unsigned char EventId;
    unsigned char IsMoreEvent;
	unsigned char sta_addr[MACADDRLEN];
	unsigned char pmk_r0[FT_PMK_LEN];
	unsigned char pmk_r0_name[FT_PMKID_LEN];
} DOT11_AUTH_FT_INSERT_R0_KEY, *PDOT11_AUTH_FT_INSERT_R0_KEY;

typedef struct _DOT11_AUTH_FT_INSERT_R1_KEY
{
    unsigned char EventId;
    unsigned char IsMoreEvent;
	unsigned char sta_addr[MACADDRLEN];
	unsigned char bssid[MACADDRLEN];
	unsigned char r0kh_id[MAX_R0KHID_LEN];
	unsigned int r0kh_id_len;
	unsigned char pmk_r1[FT_PMK_LEN];
	unsigned char pmk_r1_name[FT_PMKID_LEN];
	unsigned char pmk_r0_name[FT_PMKID_LEN];
	unsigned int pairwise;
} DOT11_AUTH_FT_INSERT_R1_KEY, *PDOT11_AUTH_FT_INSERT_R1_KEY;

typedef struct _DOT11_AUTH_FT_TRIGGER_EVENT
{
    unsigned char EventId;
    unsigned char IsMoreEvent;
	unsigned char trigger_eventid;
	unsigned char sta_addr[MACADDRLEN];
} DOT11_AUTH_FT_TRIGGER_EVENT, *PDOT11_AUTH_FT_TRIGGER_EVENT;
#endif

#define DOT11_AI_REQFI_CAPABILITIES      1
#define DOT11_AI_REQFI_LISTENINTERVAL    2
#define DOT11_AI_REQFI_CURRENTAPADDRESS  4

#define DOT11_AI_RESFI_CAPABILITIES      1
#define DOT11_AI_RESFI_STATUSCODE        2
#define DOT11_AI_RESFI_ASSOCIATIONID     4

typedef struct _DOT11_ASSOCIATION_INFO
{
    unsigned char   EventId;
    unsigned char   IsMoreEvent;
    unsigned char   SupplicantAddress[MACADDRLEN];
    UINT32 Length;
    UINT16 AvailableRequestFixedIEs;
    struct _DOT11_AI_REQFI {
                UINT16 Capabilities;
                UINT16 ListenInterval;
        	char    CurrentAPAddress[MACADDRLEN];
    } RequestFixedIEs;
    UINT32 RequestIELength;
    UINT32 OffsetRequestIEs;
    UINT16 AvailableResponseFixedIEs;
    struct _DOT11_AI_RESFI {
                UINT16 Capabilities;
                UINT16 StatusCode;
                UINT16 AssociationId;
    } ResponseFixedIEs;
    UINT32 ResponseIELength;
    UINT32 OffsetResponseIEs;
} DOT11_ASSOCIATION_INFO, *PDOT11_ASSOCIATION_INFO;

typedef struct _DOT11_INIT_QUEUE
{
    unsigned char EventId;
    unsigned char IsMoreEvent;
} DOT11_INIT_QUEUE, *PDOT11_INIT_QUEUE;

#ifdef USER_ADDIE
typedef struct _DOT11_SET_USERIE{
    unsigned char   EventId;
    unsigned char   IsMoreEvent;
	unsigned short	Flag;
    unsigned short  USERIELen;
    char            USERIE[256];
}DOT11_SET_USERIE;
#endif
#if defined(SUPPORT_UCFGING_LED) 
typedef struct _DOT11_SET_UCFGING_LED {
	unsigned char	EventId;
	unsigned int	State;
}DOT11_SET_UCFGING_LED;
#endif

#ifdef INDICATE_LINK_CHANGE
typedef struct _DOT11_LINK_CHANGE_IND {
        unsigned char   EventId;
        unsigned char   IsMoreEvent;
        char            MACAddr[MACADDRLEN];
        unsigned char LinkStatus;
} DOT11_LINK_CHANGE_IND;
#endif


//*------The following are defined to handle the event Queue for security event--------*/
//    For Event Queue related function
void DOT11_InitQueue(DOT11_QUEUE *q);
#ifndef WITHOUT_ENQUEUE
int DOT11_EnQueue(unsigned long task_priv, DOT11_QUEUE *q, unsigned char *item, int itemsize);
int DOT11_DeQueue(unsigned long task_priv, DOT11_QUEUE *q, unsigned char *item, int *itemsize);
#endif
void DOT11_PrintQueue(DOT11_QUEUE *q);
char *DOT11_ErrMsgQueue(int err);
#define DOT11_IsEmptyQueue(q) (q->NumItem==0 ? 1:0)
#define DOT11_IsFullQueue(q) (q->NumItem==q->MaxItem? 1:0)
#define DOT11_NumItemQueue(q) q->NumItem


typedef enum{
	ERROR_BUFFER_TOO_SMALL = -1,
	ERROR_INVALID_PARA = -2,
	ERROR_INVALID_RSNIE = -13,
	ERROR_INVALID_MULTICASTCIPHER = -18,
	ERROR_INVALID_UNICASTCIPHER = -19,
	ERROR_INVALID_AUTHKEYMANAGE = -20,
	ERROR_UNSUPPORTED_RSNEVERSION = -21,
	ERROR_INVALID_CAPABILITIES = -22,
	ERROR_MGMT_FRAME_PROTECTION_VIOLATION = -31,
	ERROR_INVALID_AKMP = -43
} INFO_ERROR;

#define RSN_STRERROR_BUFFER_TOO_SMALL			"Input Buffer too small"
#define RSN_STRERROR_INVALID_PARAMETER			"Invalid RSNIE Parameter"
#define RSN_STRERROR_INVALID_RSNIE				"Invalid RSNIE"
#define RSN_STRERROR_INVALID_MULTICASTCIPHER	"Multicast Cipher is not valid"
#define RSN_STRERROR_INVALID_UNICASTCIPHER		"Unicast Cipher is not valid"
#define RSN_STRERROR_INVALID_AUTHKEYMANAGE		"Authentication Key Management Protocol is not valid"
#define RSN_STRERROR_UNSUPPORTED_RSNEVERSION	"Unsupported RSNE version"
#define RSN_STRERROR_INVALID_CAPABILITIES		"Invalid RSNE Capabilities"

#define DOT11_s2n(s,c)   	(*((c))=(unsigned char)(((s)>> 8)&0xff), \
                         	*((c)+1)=(unsigned char)(((s)    )&0xff))

#define DOT11_n2s(c,s)   	(s =((unsigned short)(*((c))))<< 8, \
                          	s|=((unsigned short)(*((c)+1))))

#define DOT11_lc2s(bc,s)   	(s = ((unsigned short)(*((bc)+1)))<< 8, \
                          	s |= ((unsigned short)(*((bc)))))


void DOT11_Dump(char *fun, UINT8 *buf, int size, char *comment);

typedef enum _COUNTERMEASURE_TEST
{
	TEST_TYPE_PAIRWISE_ERROR = 0,
	TEST_TYPE_GROUP_ERROR = 1,
	TEST_TYPE_SEND_BAD_UNICAST_PACKET = 2,
	TEST_TYPE_SEND_BAD_BROADCAST_PACKET = 3
} COUNTERMEASURE_TEST;

#define	MIC_TIMER_PERIOD	RTL_SECONDS_TO_JIFFIES(60)	//unit: 10 milli-seconds
#define REJECT_ASSOC_PERIOD	RTL_SECONDS_TO_JIFFIES(60)


//*---------- The followings are for processing of RSN Information Element------------*/
#define RSN_ELEMENT_ID					0xDD
#define RSN_VER1						0x01
#define DOT11_MAX_CIPHER_ALGORITHMS		0x0a
#define DOT11_GROUPFLAG					0x02
#define DOT11_REPLAYBITSSHIFT			2
#define	DOT11_REPLAYBITS				3
#define IsPairwiseUsingDefaultKey(Cap)	((Cap[0] & DOT11_GROUPFLAG)?TRUE:FALSE)
#define IsPreAuthentication(Cap)		((Cap[0] & 0x01)?TRUE:FALSE)
#define DOT11_GetNumOfRxTSC(Cap)		(2<<((Cap[0] >> DOT11_REPLAYBITSSHIFT) & DOT11_REPLAYBITS))

#if defined(PACK_STRUCTURE) || defined(__ECOS)
#pragma pack(1)
#endif

typedef struct _DOT11_RSN_IE_HEADER {
	UINT8	ElementID;
	UINT8	Length;
	UINT8   OUI[4];
	UINT16	Version;
}DOT11_RSN_IE_HEADER;


typedef struct _DOT11_RSN_IE_SUITE{
	UINT8	OUI[3];
	UINT8	Type;
}DOT11_RSN_IE_SUITE;


typedef struct _DOT11_RSN_IE_COUNT_SUITE{

	UINT16	SuiteCount;
	DOT11_RSN_IE_SUITE	dot11RSNIESuite[DOT11_MAX_CIPHER_ALGORITHMS];
} __WLAN_ATTRIB_PACK__ DOT11_RSN_IE_COUNT_SUITE, *PDOT11_RSN_IE_COUNT_SUITE;

typedef	union _DOT11_RSN_CAPABILITY{

	UINT16	shortData;
	UINT8	charData[2];

#ifdef RTL_WPA2
	struct
	{
#ifdef _BIG_ENDIAN_
#ifdef CONFIG_IEEE80211W
		unsigned short MFPC:1; // B7
		unsigned short MFPR:1; // B6
#else
		unsigned short Reserved1:2; // B7 B6
#endif		
		unsigned short GtksaReplayCounter:2; // B5 B4
		unsigned short PtksaReplayCounter:2; // B3 B2
		unsigned short NoPairwise:1; // B1
		unsigned short PreAuthentication:1; // B0
		unsigned short Reserved2:8;
#else
		unsigned short PreAuthentication:1; // B0
		unsigned short NoPairwise:1; // B1
		unsigned short PtksaReplayCounter:2; // B3 B2
		unsigned short GtksaReplayCounter:2; // B5 B4
#ifdef CONFIG_IEEE80211W
		unsigned short MFPR:1; // B6
		unsigned short MFPC:1; // B7
#else
		unsigned short Reserved1:2; // B7 B6
#endif
		unsigned short Reserved2:8;
#endif
	} __WLAN_ATTRIB_PACK__ field;
#else
	struct
	{
#ifdef _BIG_ENDIAN_
		unsigned short PreAuthentication:1;
		unsigned short PairwiseAsDefaultKey:1;
		unsigned short NumOfReplayCounter:2;
		unsigned short Reserved:12;
#else
		unsigned short Reserved1:4;
		unsigned short NumOfReplayCounter:2;
		unsigned short PairwiseAsDefaultKey:1;
		unsigned short PreAuthentication:1;
		unsigned short Reserved2:8;
#endif
	} __WLAN_ATTRIB_PACK__ field;
#endif

} __WLAN_ATTRIB_PACK__ DOT11_RSN_CAPABILITY;

#ifdef HS2_SUPPORT
/* Hotspot 2.0 Release 1 */
#define MAX_GAS_CONTENTS_LEN	PRE_ALLOCATED_BUFSIZE*4
typedef struct _DOT11_HS2_GAS_REQ{
	unsigned char   EventId;
	unsigned char   IsMoreEvent;
	unsigned char	Dialog_token;
	unsigned char   MACAddr[MACADDRLEN];
	unsigned char	Advt_proto;
	unsigned short	Reqlen;
	unsigned char   Req[MAX_GAS_CONTENTS_LEN];
}DOT11_HS2_GAS_REQ;
/*==========HS2_SUPPORT==========*/ 
typedef struct _DOT11_WNM_NOTIFY{
        unsigned char   EventId;
        unsigned char   IsMoreEvent;
		unsigned char   macAddr[6];
        unsigned char   remedSvrURL[2048];
#if 1		
		unsigned char   serverMethod;
#endif
}DOT11_WNM_NOTIFY;

typedef struct _DOT11_WNM_DEAUTH_REQ{
        unsigned char   EventId;
        unsigned char   IsMoreEvent;
		unsigned char   macAddr[6];
		unsigned char   reason;
		unsigned short  reAuthDelay;
        unsigned char   URL[2048];
}DOT11_WNM_DEAUTH_REQ;

typedef struct _DOT11_QoSMAPConf{
        unsigned char   EventId;
        unsigned char   IsMoreEvent;
		unsigned char   macAddr[MACADDRLEN];
		unsigned char   indexQoSMAP;
}DOT11_QoSMAPConf;


typedef struct _DOT11_BSS_SessInfo_URL{
        unsigned char   EventId;
        unsigned char   IsMoreEvent;
		unsigned char   macAddr[6];
		unsigned char   SWT;
        unsigned char   URL[2048];
}DOT11_BSS_SessInfo_URL;

/*==========HS2_SUPPORT==========*/ 

typedef struct _DOT11_HS2_GAS_RSP{
	unsigned char   EventId;
	unsigned char   IsMoreEvent;
	unsigned char	Dialog_token;
	unsigned char	Action;
	unsigned char   MACAddr[MACADDRLEN];
	unsigned short	StatusCode;
	unsigned short	Comeback_delay;
	unsigned char	Rsp_fragment_id;
	unsigned char	Advt_proto;
	unsigned short	Rsplen;
	unsigned char   Rsp[MAX_GAS_CONTENTS_LEN];
}DOT11_HS2_GAS_RSP;

typedef struct _DOT11_HS2_TSM_REQ{
	unsigned char   EventId;
	unsigned char   IsMoreEvent;
	unsigned char   Dialog_token;
	unsigned char   MACAddr[MACADDRLEN];
	unsigned char   Req_mode;
	unsigned char	Validity_intval;
	unsigned char   Disassoc_timer; 	/*HS2 R2 logo test*/ 
	unsigned char	term_len;
	unsigned char	url_len;
	unsigned char	list_len;
	unsigned char	terminal_dur[12];
		unsigned char	Session_url[256];	// HS2
	unsigned char   Candidate_list[100];
}DOT11_HS2_TSM_REQ;
#endif

#ifdef CONFIG_IEEE80211W
/*HS2 R2 logo test*/ 
typedef struct _DOT11_INIT_11W_Flags {
	unsigned char	EventId;
	unsigned char	IsMoreEvent;
	unsigned char   dot11IEEE80211W;
    unsigned char   dot11EnableSHA256;
}DOT11_INIT_11W_Flags;
typedef struct _DOT11_SET_11W_Flags {
	unsigned char	EventId;
	unsigned char	IsMoreEvent;
	unsigned char	macAddr[MACADDRLEN];
	unsigned char   isPMF;
}DOT11_SET_11W_Flags;

typedef struct _DOT11_SA_QUERY_RSP{
        unsigned char   EventId;
        unsigned char   IsMoreEvent;
        char            MACAddr[MACADDRLEN];
        unsigned char   trans_id[2];
}DOT11_SA_QUERY_RSP;
#endif // CONFIG_IEEE80211W

#if defined(PACK_STRUCTURE) || defined(__ECOS)
#pragma pack()
#endif

#endif // _8192CD_SECURITY_H_

