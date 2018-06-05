#ifndef	RTL_NIC_H
#define	RTL_NIC_H

#ifdef CONFIG_RTL_STP
#include <net/rtl/rtk_stp.h>
#endif

#include <linux/version.h>

#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,2,0)
#include <linux/interrupt.h>
#endif
#include "rtl865x_netif.h"
/************************************
*	feature enable/disable
*************************************/
#define RX_TASKLET	1
#define TX_TASKLET	1
#define LINK_TASKLET 1
#define RTL819X_PRIV_IOCTL_ENABLE 1 	/* mark_add */
#define CONFIG_RTL_PHY_PATCH		1
#define RTK_QUE			1
#if defined(CONFIG_NET_WIRELESS_AGN) || defined (CONFIG_RTL8192SE) || defined(CONFIG_RTL8192CD) || defined(CONFIG_RTL8192CD_MODULE)

#if !defined(CONFIG_RTL_NO_BR_SHORTCUT)
#if !defined(CONFIG_RTL_FASTBRIDGE)//&&!defined(CONFIG_RPS)
#define BR_SHORTCUT         1
#endif
#endif

#endif
/*
*#define	CONFIG_RTL_MULTI_LAN_DEV	1
*/

//#define CONFIG_RTL_ETH_NAPI_SUPPORT 1

#if defined(CONFIG_POCKET_AP_SUPPORT)
#define	CONFIG_POCKET_ROUTER_SUPPORT
#endif

#if defined(CONFIG_RTL_ETH_PRIV_SKB)
	#if defined(CONFIG_RTL_819X)
		#define MAX_ETH_SKB_NUM	(\
			NUM_RX_PKTHDR_DESC \
			+ NUM_RX_PKTHDR_DESC1 \
			+ NUM_RX_PKTHDR_DESC2 \
			+ NUM_RX_PKTHDR_DESC3 \
			+ NUM_RX_PKTHDR_DESC4 \
			+ NUM_RX_PKTHDR_DESC5 \
			+ MAX_PRE_ALLOC_RX_SKB + 128)
	#else
		#define MAX_ETH_SKB_NUM	(\
			NUM_RX_PKTHDR_DESC \
			+ NUM_RX_PKTHDR_DESC1 \
			+ NUM_RX_PKTHDR_DESC2 \
			+ NUM_RX_PKTHDR_DESC3 \
			+ NUM_RX_PKTHDR_DESC4 \
			+ NUM_RX_PKTHDR_DESC5 \
			+ MAX_PRE_ALLOC_RX_SKB + 256)
	#endif

#include <linux/version.h>
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,36)
#define ETH_SKB_BUF_SIZE	(SKB_DATA_ALIGN(CROSS_LAN_MBUF_LEN+sizeof(struct skb_shared_info)+160+RTL_NET_SKB_PAD))
#else	
#define ETH_SKB_BUF_SIZE	(SKB_DATA_ALIGN(CROSS_LAN_MBUF_LEN+sizeof(struct skb_shared_info)+160+NET_SKB_PAD))
#endif
#define ETH_MAGIC_CODE		"819X"
#define ETH_MAGIC_LEN		4
#endif

struct re865x_priv
{
	u16			ready;
	u16			addIF;
	u16			devnum;
	u32			sec_count;
	u32			sec;
#if defined(CONFIG_RTK_VLAN_SUPPORT) || defined (CONFIG_RTL_MULTI_LAN_DEV)
	struct	net_device	*dev[ETH_INTF_NUM];
#else
	struct	net_device	*dev[ETH_INTF_NUM];
#endif
#if defined(CONFIG_RTL_STP) && !defined(CONFIG_RTL_MULTI_LAN_DEV)
	struct	net_device	*stp_port[MAX_RE865X_STP_PORT];
#endif
#if defined(CONFIG_RTL_CUSTOM_PASSTHRU)
	struct	net_device	*pdev;
#endif
	//spinlock_t		lock;
	void			*regs;
	struct tasklet_struct	rx_tasklet;
	struct timer_list timer;	/* Media monitoring timer. */
	unsigned long		linkchg;	
};

struct dev_priv {
	u32			id;            /* VLAN id, not vlan index */
	u32			portmask;     /* member port mask */
	u32			portnum;     	/* number of member ports */
	u32			netinit;
	struct net_device	*dev;
	struct net_device   *dev_prev;
	struct net_device   *dev_next;
#ifdef RX_TASKLET
	struct tasklet_struct   rx_dsr_tasklet;
#endif
#if defined(CONFIG_RTL_ETH_NAPI_SUPPORT)
	struct napi_struct napi;
#endif

#ifdef TX_TASKLET
	struct tasklet_struct   tx_dsr_tasklet;
#if defined(CONFIG_RTL_NIC_QUEUE)
	struct tasklet_struct   tx_queue_tasklet;
#endif
#endif

	struct tasklet_struct   link_dsr_tasklet;

#if defined(CONFIG_RTK_VLAN_SUPPORT)
	struct vlan_info	vlan_setting;
#endif

#ifdef CP_VLAN_TAG_USED
	struct vlan_group	*vlgrp;
#endif
	spinlock_t			lock;
	u32			msg_enable;
	u32 			opened;
	u32			irq_owner; //record which dev request IRQ
	struct net_device_stats net_stats;
#if defined(DYNAMIC_ADJUST_TASKLET) || defined(BR_SHORTCUT) || defined(CONFIG_RTL8196C_REVISION_B) || defined(CONFIG_RTL_8198) || defined(CONFIG_RTL_819XD) || defined(CONFIG_RTL_8196E) || defined(CONFIG_RTL_8198C) || defined(CONFIG_RTL_8197F)
    struct timer_list expire_timer; 
#endif

#ifdef CONFIG_RTL8196C_GREEN_ETHERNET
    struct timer_list expire_timer2; 
#endif
};

typedef struct __rtlInterruptRxData
{
	#if defined (CONFIG_RTK_VOIP_QOS) && !defined (CONFIG_RTK_VOIP_ETHERNET_DSP_IS_HOST)
	unsigned long	voip_rx_start_time;
  	int				voip_rx_cnt;
  	#endif
}	rtlInterruptRxData;

/*	define return value		*/
#define	RTL_RX_PROCESS_RETURN_SUCCESS		0
#define	RTL_RX_PROCESS_RETURN_CONTINUE		-1
#define	RTL_RX_PROCESS_RETURN_BREAK		-2


#if defined (CONFIG_RTL_UNKOWN_UNICAST_CONTROL)
#define	RTL_MAC_RECORD_NUM		16	/* Must be 2's orders */
#define	RTL_MAC_THRESHOLD		200	/* threshold for block unkown unicast */
#define	RTL_MAC_TIMEOUT			10*HZ	/* unkown unicast restriction time */
#define	RTL_MAC_REFILL_TOKEN	10	/* per second refill token number */
typedef	struct __rtlMacRecord
{
	uint8		enable;
	uint8		cnt;
	uint8		mac[ETHER_ADDR_LEN];
	struct timer_list	timer;
	uint8		acl_config;
}	rtlMacRecord;
#endif

#if defined(RTL819X_PRIV_IOCTL_ENABLE)
#define RTL819X_IOCTL_READ_PORT_STATUS			(SIOCDEVPRIVATE + 0x01)	
#define RTL819X_IOCTL_READ_PORT_STATS	              (SIOCDEVPRIVATE + 0x02)	

struct lan_port_status {
    unsigned char link;
    unsigned char speed;
    unsigned char duplex;
    unsigned char nway;    	
}; 

struct port_statistics  {
	unsigned int  rx_bytes;		
 	unsigned int  rx_unipkts;		
       unsigned int  rx_mulpkts;			
	unsigned int  rx_bropkts;		
 	unsigned int  rx_discard;		
       unsigned int  rx_error;			
	unsigned int  tx_bytes;		
 	unsigned int  tx_unipkts;		
       unsigned int  tx_mulpkts;			
	unsigned int  tx_bropkts;		
 	unsigned int  tx_discard;		
       unsigned int  tx_error;			   
};
#endif

#if defined (CONFIG_RTL_INBAND_CTL_API)
struct port_stats
{
	unsigned long rx_bytes_last;
	unsigned long tx_bytes_last;
	unsigned long rx_bytes_current;
	unsigned long tx_bytes_current;
	unsigned long rx_rate;
	unsigned long tx_rate;
};
#endif

#if defined(CONFIG_RTL_ISP_MULTI_WAN_SUPPORT)
struct ifvlan
{
	int cmd;
	char enable;
	short vlanIdx;
	short vid;
	char	disable_priority;
	int member;
	int port;
	char txtag;
};
struct lan_dev_bind_mask_drv {
	char 	ifname[IFNAMSIZ];
	int 		bind_mask;
};
#endif
#if defined(CONFIG_RTK_VLAN_SUPPORT) || defined (CONFIG_RTL_MULTI_LAN_DEV)
#if defined(CONFIG_RTL_ISP_MULTI_WAN_SUPPORT)
#define	RTL_LANVLANID_1		RTL_LANVLANID
#define	RTL_LANVLANID_2		RTL_LANVLANID
#define	RTL_LANVLANID_3		RTL_LANVLANID
#define	RTL_LANVLANID_4		RTL_LANVLANID
#define 	RTL_LANVLANID_5		RTL_LANVLANID
#else
#define	RTL_LANVLANID_1		9
#define	RTL_LANVLANID_2		10
#define	RTL_LANVLANID_3		11
#define	RTL_LANVLANID_4		12
#define RTL_LANVLANID_5		13
#endif
#endif
#define	RTL_PPTPL2TP_VLANID	999

//flowing name in protocol stack DO NOT duplicate
#define RTL_PS_BR0_DEV_NAME RTL_BR_NAME
#define RTL_PS_ETH_NAME	"eth"
#define RTL_PS_WLAN_NAME	RTL_WLAN_NAME
#define RTL_PS_PPP_NAME	"ppp"
#define RTL_PS_LAN_P0_DEV_NAME RTL_DEV_NAME_NUM(RTL_PS_ETH_NAME,0)
#define RTL_PS_WAN0_DEV_NAME RTL_DEV_NAME_NUM(RTL_PS_ETH_NAME,1)
#define RTL_PS_PPP0_DEV_NAME RTL_DEV_NAME_NUM(RTL_PS_PPP_NAME,0)
#define RTL_PS_PPP1_DEV_NAME RTL_DEV_NAME_NUM(RTL_PS_PPP_NAME,1)
#define RTL_PS_WLAN0_DEV_NAME RTL_DEV_NAME_NUM(RTL_PS_WLAN_NAME,0)
#define RTL_PS_WLAN1_DEV_NAME RTL_DEV_NAME_NUM(RTL_PS_WLAN_NAME,1)
#if defined(CONFIG_RTL_MESH_SINGLE_IFACE)
#define RTL_PS_MESH_DEV_NAME "wlan-msh"
#else	//!CONFIG_RTL_MESH_SINGLE_IFACE
#define RTL_PS_MESH1_DEV_NAME "wlan0-msh0"
#define RTL_PS_MESH2_DEV_NAME "wlan1-msh0"
#endif	//CONFIG_RTL_MESH_SINGLE_IFACE

#if defined(CONFIG_RTL_MULTIPLE_WAN)
#define RTL_MULTIWAN_ADD 1
#define RTL_MULTIWAN_DEL 2
//#define RTL_PS_WAN1_DEV_NAME RTL_DEV_NAME_NUM(RTL_PS_ETH_NAME,2)
#endif

#if defined(CONFIG_RTL_PUBLIC_SSID)
#define RTL_LAN_DEVICE_NAME "br0"
#define RTL_GW_WAN_DEVICE_NAME "br1"
#define RTL_WISP_WAN_DEVICE_NAME "wlan0"
#define RTL_BR_WAN_DEVICE_NAME "br0"
#endif

//Used by fastpath mac-based qos under IMPROVE_QOS 
#define QOS_LAN_DEV_NAME RTL_PS_BR0_DEV_NAME

#ifdef CONFIG_RTL_LAYERED_DRIVER
struct rtl865x_vlanConfig {
	uint8 			ifname[IFNAMSIZ];
	uint8			isWan;
	uint16			if_type;
	uint16			vid;
	uint16			fid;

	/* 
		When CONFIG_RTL_ISP_MULTI_WAN_SUPPORT defined: 
		For WAN port (isWan=1): 	"memPort" is NOT the vlan's mbr , it means the port-binding members of this WAN
		For LAN port (isWan=0): 	"memPort" means the switch phyport mapping
	*/
	uint32			memPort;
	uint32			untagSet;
	uint32			mtu;
	ether_addr_t		mac;
	uint8			is_slave;
	#if defined(CONFIG_RTL_ISP_MULTI_WAN_SUPPORT)
	uint8			protocol; /* 0: pppoe, 1: ipoe, 2: bridge */
	#endif
};
#if defined(CONFIG_RTL_ISP_MULTI_WAN_SUPPORT)
#define RTL865X_CONFIG_END					{ "", 0, 0, 0, 0, 0, 0, 0, {{0}}, 0, 0 }
#else
#define RTL865X_CONFIG_END					{ "", 0, 0, 0, 0, 0, 0, 0, {{0}}, 0 }
#endif
#define GATEWAY_MODE				0
#define BRIDGE_MODE					1
#define WISP_MODE					2
//#define MULTIPLE_VLAN_BRIDGE_MODE 3
//#define MULTIPLE_VLAN_WISP_MODE 4
#define CONFIG_CHECK(expr) do {\
	if(((int32)expr)!=SUCCESS){\
		rtlglue_printf("Error >>> %s:%d failed !\n", __FUNCTION__,__LINE__);\
			return FAILED;\
	}\
}while(0)

#define INIT_CHECK(expr) do {\
	if(((int32)expr)!=SUCCESS){\
		rtlglue_printf("Error >>> %s:%d failed !\n", __FUNCTION__,__LINE__);\
			return FAILED;\
	}\
}while(0)
#endif

#ifdef CONFIG_RTL8686_GMAC
#define GATEWAY_MODE				0
#define BRIDGE_MODE					1
#define WISP_MODE					2
#endif

typedef struct _ps_drv_netif_mapping_s
{
	uint32 valid:1, //entry enable?
		flags;	//reserverd
	struct net_device *ps_netif; //linux ps network interface
	char drvName[MAX_IFNAMESIZE];//netif name in driver
	
}ps_drv_netif_mapping_t;

int32 rtl865x_changeOpMode(int mode);

#if defined(CONFIG_RTL_ETH_PRIV_SKB)
__MIPS16 __IRAM_FWD  int is_rtl865x_eth_priv_buf(unsigned char *head);
void free_rtl865x_eth_priv_buf(unsigned char *head);
#endif

ps_drv_netif_mapping_t* rtl_get_ps_drv_netif_mapping_by_psdev(struct net_device *dev);
int rtl_add_ps_drv_netif_mapping(struct net_device *dev, const char *name);
int rtl_del_ps_drv_netif_mapping(struct net_device *dev);

#if defined(CONFIG_RTK_VLAN_SUPPORT) && defined(CONFIG_RTK_VLAN_FOR_CABLE_MODEM)
extern struct net_device* get_dev_by_vid(int vid);
#endif
extern __MIPS16  struct net_device *get_shortcut_dev(unsigned char *da);
#define CONFIG_RTL_NIC_HWSTATS

int32 rtl865x_changeOpMode(int mode);
int  rtl865x_reChangeOpMode (void);
extern int update_vlanconfig(uint32 vid, uint32 portMask, uint32 untagPortMask);
extern void htonl_array(u32 *org_buf, u32 *dst_buf, unsigned int words);
extern void ntohl_array(u32 *org_buf, u32 *dst_buf, unsigned int words);

#if defined(CONFIG_RTL_HW_VLAN_SUPPORT)
extern int rtl_hw_vlan_ignore_tagged_mc;
#define CONFIG_RTL_HW_VLAN_SUPPORT_HW_NAT 
#endif
#ifndef CONFIG_RTL_CUSTOM_PASSTHRU
#ifndef CONFIG_OPENWRT_SDK
#define CONFIG_RTL_CUSTOM_PASSTHRU
#endif
//#define CONFIG_RTL_CUSTOM_PASSTHRU_PPPOE
#ifdef CONFIG_RTL_CUSTOM_PASSTHRU
#define PASSTHRU_VLAN_ID 100

#define IP6_PASSTHRU_RULEID 5
#define PPPOE_PASSTHRU_RULEID1 3
#define PPPOE_PASSTHRU_RULEID2 4
#define IP6_PASSTHRU_MASK 0x1
#define PPPOE_PASSTHRU_MASK 0x1<<1
#endif
#endif

//#define RTL_TRAP_BR_REAMIN_IP_TO_CPU 		1

#if defined(RTL_TRAP_BR_REAMIN_IP_TO_CPU)
#define RTL_MAX_BR_IP_NUM					10
#define RTL865X_ACL_BR_REMAIN_IP_USED0		-20000
#else
#define RTL_DROP_OUT_OF_BR_SUBNET_FRAME 	1
#endif

#if defined(CONFIG_RTL_PROCESS_PPPOE_IGMP_FOR_BRIDGE_FORWARD)
struct iphdr * rtl_get_pppoe_ipv4_hdr(unsigned char *data, int check_from_mac, unsigned short protocol);
#if defined(CONFIG_RTL_MLD_SNOOPING)
struct ipv6hdr * rtl_get_pppoe_ipv6_hdr(unsigned char *data, int check_from_mac, unsigned short protocol);
#endif
int rtl_is_pppoe_igmp_bridge_frame(struct sk_buff *skb, int check_from_mac, unsigned char *dest);
#define PPPOE_IGMP  1
#define PPPOE_UDP   2
#define PPPOE_TCP   3
#if defined(CONFIG_RTL_MLD_SNOOPING)
#define PPPOE_ICMPV6  4
#define PPPOE_UDPV6  5
#define PPPOE_TCPV6  6
#endif
#endif
#endif
