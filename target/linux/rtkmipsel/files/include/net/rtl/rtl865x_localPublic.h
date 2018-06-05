#ifndef RTL865X_LOCAL_PUBLIC_H
#define RTL865X_LOCAL_PUBLIC_H

#if defined(CONFIG_RTL_HW_QOS_SUPPORT)
#include <common/rtl865x_netif.h>
#endif

#define FROM_LOCAL_PUBLIC	1
#define TO_LOCAL_PUBLIC		2	
#define RX_LAN_PACKET	1
#define RX_WAN_PACKET	2
#define TX_LAN_PACKET	3
#define TX_WAN_PACKET	4

#define MAX_HW_LOCAL_PUBLIC_NUM 2
#define DEF_LOCAL_PUBLIC_NUM	32
#define MAX_LOCAL_PUBLIC_NUM	32

#define LOCAL_PUBLIC_VLAN_START 	3198
#define LOCAL_PUBLIC_VLAN_END		4094
#define MCAST_NETIF_VLAN_ID		3197	

struct rtl865x_localPublic
{
	unsigned int ipAddr;
	unsigned int netMask;
	unsigned int defGateway;
	unsigned int hw;
	unsigned int inAsic;
	
	unsigned char port;
	unsigned char mac[6];
	unsigned char dev[16];
	
	unsigned char lpNetif[16];
	unsigned int lpVid;
	unsigned char defGwMac[6];
	
	//must at the tail...
	struct 
	{								
		struct rtl865x_localPublic *tqe_next;	
		struct rtl865x_localPublic**tqe_prev;
	}next;
	
};

#if defined(CONFIG_RTL_PUBLIC_SSID)
struct rtl865x_public_ssid_entry
{
	struct list_head list;
	unsigned int public_addr;
	unsigned int flags;
};
#endif
	
struct rtl865x_pktInfo
{
	/*input parameter*/
	unsigned char *data;
	unsigned char port;
	unsigned char action;
	unsigned char dev[16];
	
	/*output parameter*/
	unsigned char *arpHdr;
	unsigned char *ipHdr;
	unsigned char fromLocalPublic;
	unsigned char toLocalPublic;
	unsigned int srcIp;
	unsigned int dstIp;
};

struct rtl865x_localPublicPara
{
	unsigned int maxEntryNum;
};

struct rtl865x_interface_info
{
	unsigned char 			ifname[16];
	unsigned char 			isWan;
	unsigned short			if_type;
	unsigned short			vid;
	unsigned short			fid;
	unsigned int				memPort;
	unsigned int				untagSet;
	unsigned int				mtu;
};

int rtl865x_initLocalPublic(struct rtl865x_localPublicPara* para);

int rtl865x_reInitLocalPublic(void);

int rtl865x_addLocalPublic(struct rtl865x_localPublic* newEntry);

int rtl865x_delLocalPublic(struct rtl865x_localPublic* delEntry);

int rtl865x_checkLocalPublic(struct rtl865x_pktInfo *pktInfo);

int rtl865x_isLocalPublicIp(unsigned int ipAddr);

int rtl865x_getLocalPublicInfo(unsigned int ipAddr, struct rtl865x_localPublic  *localPublicInfo);

int rtl865x_localPublicEnabled(void);
int rtl865x_getLocalPublicMac(unsigned int ip, unsigned char mac[]);
int rtl865x_getAllLocalPublic(struct rtl865x_localPublic localPublicArray[], int arraySize);
#if defined(CONFIG_RTL_HW_QOS_SUPPORT)
int rtl_checkLocalPublicNetifIngressRule(rtl865x_AclRule_t *rule);
#endif
#if defined(CONFIG_RTL_PUBLIC_SSID)
int rtl865x_is_public_ssid_entry(unsigned int ipAddr);
int rtl865x_from_public_ssid_device(unsigned char *name);
#endif

int rtl865x_setMCastSrcMac(unsigned char *srcMac);
#endif
