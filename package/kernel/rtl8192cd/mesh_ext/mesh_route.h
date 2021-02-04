#ifndef PATHSELECTION_H
#define PATHSELECTION_H

#include "./mesh.h"

#ifndef CONFIG_RTK_MESH	//if"N"def For 8190n.h wlan_device[]
#define MESH_SHIFT			0
#define MESH_NUM_CFG		0
#else
#define MESH_SHIFT			8 // ACCESS_MASK uses 2 bits, WDS_MASK use 4 bits
#define MESH_MASK			0x3
#define MESH_NUM_CFG		NUM_MESH

#define HWMP_MAX_PREQ_RETRIES 		2

#define PROXY_TBL_AGING_LIMIT		300	//proxy table expire time
#define HWMP_ACTIVE_PATH_TIMEOUT	300	//pathsel table expire time, chuangch
#define HWMP_PREQ_REFRESH_PERIOD	60	//route maintenance, chuangch 10.19
#define HWMP_PREQ_REFRESH_PERIOD2	200	//route maintenance, chuangch 10.19


#if defined(RTK_MESH_AODV_STANDALONE_TIMER)
#define HWMP_NETDIAMETER_TRAVERSAL_TIME MESH_AODV_EXPIRE_TO*3 //miliseconds
#else
#define HWMP_NETDIAMETER_TRAVERSAL_TIME HZ
#endif

#ifdef PU_STANDARD	// For 802.11 Mesh proxy update
enum PU_flag
{
    PU_add,
    PU_delete
};
#endif

__PACK struct mesh_rreq_retry_entry {
    unsigned char		MACAddr[MACADDRLEN];
    long int			TimeStamp;
    long int			createTime;
    unsigned char		Retries; // add by chuangch, the num of rreq retries
    struct pkt_queue    pktqueue;
    struct mesh_rreq_retry_entry *rreq_next;
    struct mesh_rreq_retry_entry *rreq_prev;    
}__WLAN_ATTRIB_PACK__;


__PACK struct lls_mesh_header {
    unsigned char 	mesh_flag;
    INT8 			TTL;
    UINT16 			segNum;
    unsigned char 	DestMACAddr[MACADDRLEN]; // modify for 6 address
    unsigned char 	SrcMACAddr[MACADDRLEN];
} __WLAN_ATTRIB_PACK__;


__PACK struct proxy_table_entry	 {
    unsigned char 	sta[MACADDRLEN];
    unsigned char 	owner[MACADDRLEN];
    // unsigned char	type; // 0: others, 1: my_sta, 2: my bridge
    short aging_time;
};


__PACK struct proxyupdate_table_entry	 {
    UINT8	PUflag, isMultihop, PUSN, retry;
    unsigned short	STAcount;
    unsigned char	nexthopmac[MACADDRLEN];
    unsigned char	destproxymac[MACADDRLEN];
    unsigned char 	proxymac[MACADDRLEN];
    unsigned char   proxiedmac[MACADDRLEN];
    DRV_PRIV *priv;    
    unsigned long update_time;
};

#ifdef SUPPORT_TX_MCAST2UNI
__PACK struct mesh_ip_mcast_info {
	int					used;
	unsigned char		mcmac[MACADDRLEN];

    /*src record*/
    unsigned char srcCount;
    unsigned char srcValid[MAX_IP_SRC_ENTRY];
    unsigned char srcMac[MAX_IP_SRC_ENTRY][MACADDRLEN];	
};
#endif

__PACK struct path_sel_entry {
    unsigned char destMAC[MACADDRLEN];
    unsigned char nexthopMAC[MACADDRLEN];
    unsigned int dsn;
    unsigned int metric;
    unsigned char hopcount;
    // the following entries are NOT shared by daemon and driver
    //	unsigned char isvalid;
    //	unsigned char modify_time[8];
    unsigned int start, end;
    unsigned short RecentSeq[SZ_HASH_IDX2];
    DRV_PRIV *priv;
    unsigned long update_time;
    unsigned long routeMaintain;
    #ifdef  SUPPORT_TX_MCAST2UNI
	int	ipmc_num;
	struct mesh_ip_mcast_info	ipmc[MAX_IP_MC_ENTRY];
    #endif
} __WLAN_ATTRIB_PACK__;


struct pann_mpp_tb_entry {
    unsigned char mac[MACADDRLEN];
    unsigned int timeout;
    unsigned int seqNum;
    short int flag;
    int beRoot;
};


struct mpp_tb {
    struct pann_mpp_tb_entry pann_mpp_pool[MAX_MPP_NUM];
    unsigned int 		 pool_count ;
};



// _DOT11s_XXX serires struct size need less than MAXDATALEN(1560)	, which defined in iwreq.c
typedef __PACK struct _DOT11s_GEN_RREQ_PACKET{
    unsigned char	EventId;
    unsigned char   IsMoreEvent;
    unsigned char	MyMACAddr[MACADDRLEN];
    unsigned char	destMACAddr[MACADDRLEN];
    unsigned char   TTL;
    unsigned short  Seq_num;
}__WLAN_ATTRIB_PACK__ DOT11s_GEN_RREQ_PACKET;

typedef __PACK struct _DOT11s_GEN_RERR_PACKET{
    unsigned char	EventId;
    unsigned char   IsMoreEvent;
    unsigned char	MyMACAddr[MACADDRLEN];
    unsigned char	SorNMACAddr[MACADDRLEN];	//this address is the neighbor that hello message finds it is unreachabel.
    											// if flag == 1, this field means disconnected neighbor MAC addr
    											// if flag == 2, this field means source MAC addr of the data frame
    unsigned char	DataDestMAC[MACADDRLEN];	// this field is valid if and only if flag == 2
    unsigned char	PrehopMAC[MACADDRLEN];		// this field is valid if and only if flag == 2
    unsigned char   TTL;
    unsigned short  Seq_num;
    unsigned char   Flag;						// flag = 1 means the link to neighbor has broken
												// flag = 2 means this MP doesn't have the nexthop information for the destination in pathseleciton table
}__WLAN_ATTRIB_PACK__ DOT11s_GEN_RERR_PACKET;

typedef __PACK struct _DOT11s_RECV_PACKET{
    unsigned char	EventId;
    unsigned char   IsMoreEvent;
    unsigned char	MyMACAddr[MACADDRLEN];
    unsigned char	PreHopMACAddr[MACADDRLEN];
    unsigned char	DesMACAddr[MACADDRLEN];
    unsigned int  Pre_Hop_Metric;
    unsigned char ReceiveData[128];
    unsigned char   TTL;
    unsigned short  Seq_num;
    unsigned char Is6AddrFormat;
    unsigned char	MACAddr5[MACADDRLEN];
    unsigned char	MACAddr6[MACADDRLEN];
#ifdef RTK_MESH_METRIC_REFINE
    unsigned char	InterfaceMACAddr[MACADDRLEN];
    unsigned int	RSSI_Metric;
    unsigned int	Cu_Metric;
    unsigned int	Noise_Metric;
#endif
}__WLAN_ATTRIB_PACK__ DOT11s_RECV_PACKET;

#endif // end of not define CONFIG_RTK_MESH

#endif//PATHSELECTION_H
