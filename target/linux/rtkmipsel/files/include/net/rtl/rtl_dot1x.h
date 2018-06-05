#ifndef	RTL_DOT1X_H
#define	RTL_DOT1X_H

//#include "rtl865x_netif.h"
//#define DOT1X_DEBUG

#define DOT1X_AUTH_TYPE_PORT_BASED      1
#define DOT1X_AUTH_TYPE_MAC_BASED       2

#define DOT1X_AUTH_MODE_SNOOPING        1
#define DOT1X_AUTH_MODE_PROXY       	2
#define DOT1X_AUTH_MODE_CLIENT       	3

#define DOT1X_EAPOL_TYPE_EAP_PACKET         0x00
#define DOT1X_EAPOL_TYPE_START              0x01
#define DOT1X_EAPOL_TYPE_LOGOFF             0x02
#define DOT1X_EAPOL_TYPE_EAP_REQUEST        0x03
#define DOT1X_EAPOL_TYPE_EAP_RESPONSE       0x04
#define DOT1X_EAPOL_TYPE_EAP_SUCCESS        0x05
#define DOT1X_EAPOL_TYPE_EAP_FAILURE        0x06

#define DOT1X_EAPOL_PACKET_CODE_REQUEST      0x1
#define DOT1X_EAPOL_PACKET_CODE_RESPONSE     0x2
#define DOT1X_EAPOL_PACKET_CODE_SUCCESS      0x3
#define DOT1X_EAPOL_PACKET_CODE_FAILURE      0x4

#define DOT1X_EAPOL_PACKET_TYPE_OFFSET		 3
#define DOT1X_EAPOL_PACKET_CODE_OFFSET		 6
#define DOT1X_EAPOL_PACKET_ID_OFFSET		 7

#define DOT1X_AUTH_STATE_UNAUTH              0x00
#define DOT1X_AUTH_STATE_START               0x01
#define DOT1X_AUTH_STATE_PROCESSING          0x02
#define DOT1X_AUTH_STATE_SUCCESS             0x03

#define DOT1X_AUTH_TIME_OUT                  (5*60)

#define DOT1X_CONSUME_PKT                    1
#define DOT1X_DROP_PKT                       2

#define	DOT1X_TABLE_LIST_MAX	256
#define	DOT1X_TABLE_ENTRY_MAX	256

#define	DOT1X_TABLE_ENTRY_VALID	  0xff
#define	DOT1X_TABLE_ENTRY_INVALID 0x00

#define DOT1X_EVENT_EAP_PACKET	  0x01
#define DOT1X_EVENT_PORT_DOWN	  0x02
#define DOT1X_EVENT_PORT_UP	  0x03

#define DOT1X_MAX_QUEUE_SIZE	8
#define DOT1X_MAX_DATA_LEN		1560
#define DOT1X_MAX_EAP_PACKET_LEN 1550
#define DOT1X_ERROR_2LARGE	-1
#define DOT1X_ERROR_QFULL	-2
#define DOT1X_ERROR_QEMPTY	-3
#define DOT1X_ERROR_ERROR	-4

#define DOT1X_SPECIAL_IDENTITY 0xaabb


typedef struct __rtl802Dot1xPortMode
{
		uint8           port_enable;
		/* auth_dir:
		0= BOTH direction. (control for the packets that 
		"incoming from" or "expect outgoing from" this port .) 
		1= IN direction. (control for the packets that "incoming from" this port .) 			
		*/
		uint8 			auth_dir;
		uint8		    auth_mode; 
}rtl802Dot1xPortMode;

typedef	struct __rtl802Dot1xConfig
{
	int8		server_port;
	uint8		enable;
	uint8		type; /* 1:port based/2:mac based */
	/* only valid in proxy mode  or snooping mode???
	  * 0:authenticator response EAP packet's destination MAC address as multicast
	  * 1:authenticator response EAP packet's destination MAC address as unicast
	  */
	uint8       enable_unicastresp; 
    /*mode
    	  *1:snooping mode-->transparent forward eapol packets
    	  *2:repeater mode -->normal 1x auth flow client<->DUT(auth deamon) <->server 
    	  *3:client mode-->act as supplicant
    	  */
	rtl802Dot1xPortMode	mode[RTL8651_PORT_NUMBER]; 	
}rtl802Dot1xConfig;
#if 0
typedef struct __rtl802Dot1xPortBasedInfo
{
    uint8 portAuthEnable;
    int8  portNum;
    uint8 authState;
    uint8 authOpdir;
    uint8 macaddr[ETH_ALEN];
    struct timer_list expire_timer; 
}rtl802Dot1xPortBasedInfo;
#endif

/* ---start---used for snooping mode ---start---*/
typedef struct __rtl802Dot1xCacheEntry
{
	uint32 id;
	uint8  rx_port_num;
    uint8 auth_state;
    uint8 mac_addr[ETH_ALEN];
	uint8 valid; 
	CTAILQ_ENTRY(__rtl802Dot1xCacheEntry) cache_link;
	CTAILQ_ENTRY(__rtl802Dot1xCacheEntry) tqe_link;
}rtl802Dot1xCacheEntry;
typedef struct __rtl802Dot1xCacheTable
{
	CTAILQ_HEAD(__rtl802Dot1xCacheListEntryHead, __rtl802Dot1xCacheEntry) *list;
}rtl802Dot1xCacheTable;

/* ---end---used for snooping mode ---end---*/

/* ---start---used for repeater mode/client mode ---start---*/
typedef struct __rtl802Dot1xPortStateInfo
{
		uint8	event_id;
		int8	flag; /* more packets flag */
		uint32	port_mask;/* Bit0 = port0, Bit0=1 means down and so on */
}rtl802Dot1xPortStateInfo;

typedef struct __rtl802Dot1xEapPkt
{
		uint8   event_id;
		int8	flag; /* more packets flag */
		uint8	rx_port_num;
        int16	item_size;
        uint8	item[DOT1X_MAX_EAP_PACKET_LEN];
}rtl802Dot1xEapPkt;

typedef struct __rtl802Dot1xQueueNode
{
        int16	item_size;
        uint8	item[DOT1X_MAX_DATA_LEN];
}rtl802Dot1xQueueNode;

typedef struct __rtl802Dot1xQueue
{
        int32     head;
        int32     tail;
        int32     item_num;
        int32     max_item;
    	rtl802Dot1xQueueNode item_array[DOT1X_MAX_QUEUE_SIZE];
}rtl802Dot1xQueue;

typedef struct __rtl802Dot1xAuthResult
{
		uint8     type; /* 1:port base result/2:mac base result */
        uint8     port_num;
        int8      auth_state;
		uint8     mac_addr[ETH_ALEN];
}rtl802Dot1xAuthResult;
/* ---end---used for repeater mode/client mode ---end---*/
#endif
