#ifndef	__FASTPATH_CORE_H__
#define	__FASTPATH_CORE_H__

#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/in.h>
#include <linux/ip.h>
#include <linux/udp.h>
#include <linux/tcp.h>
#include <linux/netdevice.h>
#include <linux/netfilter.h>
#include <linux/netfilter_ipv4.h>
#include <net/dst.h>
#include <net/rtl/rtl_queue.h>

#if	defined(CONFIG_RTL_HW_QOS_SUPPORT)
#include <net/rtl/rtl865x_nat.h>
#endif

#include <linux/version.h>

#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,2,0)
#include <linux/seq_file.h>
extern struct proc_dir_entry proc_root;
#endif
#if defined(CONFIG_OPENWRT_SDK)
#include <uapi/linux/netfilter/nf_conntrack_tuple_common.h>
#endif

//#include <net/netfilter/nf_conntrack.h>
//#include <net/route.h>


/*
	Virtual Rome Driver API & System (Light Rome Driver Simulator)
*/

#if defined (__IRAM_GEN)
#undef	__IRAM_GEN
#define	__IRAM_GEN
#endif

//#define	ipaddr_t		__u32
//#define	uint8		__u8
//#define	uint16		__u16
//#define	uint32		__u32
//typedef unsigned long long	__uint64;

#if 0
#define DEBUGP_API printk
#else
#define DEBUGP_API(format, args...)
#endif

#if 0
#define DEBUGP_PKT printk
#else
#define DEBUGP_PKT(format, args...)
#endif

#if 0
#define DEBUGP_SYS printk
#else
#define DEBUGP_SYS(format, args...)
#endif

#define	DEBUG_PROCFILE	/* Create ProcFile for debug */

//define this macro to improve qos
#define IMPROVE_QOS		1

//#define       RTL_FP_CHECK_SPI_ENABLED        1
#if !defined(RTL_FP_CHECK_SPI_ENABLED)
#define CONFIG_UDP_FRAG_CACHE 1   //enable fragmentation cache ,mark_add
#endif

#if defined(CONFIG_UDP_FRAG_CACHE)
#define MAX_UDP_FRAG_ENTRY 64 //kernel => 64 frag queue!!
#define FRAG_CACHE_TIMEOUT (10 * HZ)
#endif
extern void rtl_readUdpFragEntry(void);
extern unsigned int statistic_udp_frag_rx;
extern unsigned int statistic_udp_frag_ps;
extern unsigned int statistic_udp_frag_tbl_full;
extern unsigned int statistic_udp_frag_add;
extern unsigned int statistic_udp_frag_free;
#if 1
/***********************************cary:refine filter.c**********************************/
#define FASTPATH_FILTER	1
/***********************************************************************************/
#define CUSTOM_RSP_PACKET
#define DOS_FILTER
#define URL_FILTER

//#define URL_CONTENT_AUTHENTICATION

#define ACTION_ALLOW 1
#define ACTION_DENY 0
#define WEEK_ALL_DAY 1<<7
#define TIME_ALL_TIME 1<<0
#define FAST_PPTP
#define FAST_L2TP
#endif
//#define DEL_NAPT_TBL
#define DEL_ROUTE_TBL	//sync from rtl865x --2010.02.09
#define NO_ARP_USED	// 2008.01.09, Forrest Lin. Use kernel route cache already.
#define INVALID_PATH_BY_FIN

#if defined(CONFIG_RTL_FAST_PPPOE)
#define MAX_PPPOE_ENTRY 16
struct pppoe_info
{	
	char            wan_dev[IFNAMSIZ];          /* Local device to use */ 
	char            ppp_dev[IFNAMSIZ];          /* Local device to use */ 
	unsigned char	our_mac[6];
	unsigned char	peer_mac[6];
	unsigned int 	our_ip;
	unsigned int	peer_ip;
	unsigned short  sid; 
	unsigned short  valid;
	unsigned long 	last_rx;
	unsigned long	last_tx;
	unsigned char   txHdrCache[22];	
#if defined (CONFIG_RTL_FAST_PPPOE_DEBUG)	
	unsigned int 	total_rx;
	unsigned int	total_tx;
#endif	
};


int  __init fast_pppoe_init(void);
int __exit fast_pppoe_exit(void);

int clear_pppoe_info(char *ppp_dev, char *wan_dev, unsigned short sid,
								unsigned int our_ip,unsigned int	peer_ip,
								unsigned char * our_mac, unsigned char *peer_mac);

int set_pppoe_info(char *ppp_dev, char *wan_dev, unsigned short sid,
							unsigned int our_ip,unsigned int	peer_ip,
							unsigned char * our_mac, unsigned char *peer_mac);

unsigned long get_pppoe_last_rx_tx(char *ppp_dev, char *wan_dev, unsigned short sid,
								unsigned int our_ip,unsigned int	peer_ip,
								unsigned char * our_mac, unsigned char *peer_mac,
								unsigned long *last_rx, unsigned long *last_tx);


int check_and_pull_pppoe_hdr(struct sk_buff *skb);

void check_and_restore_pppoe_hdr(struct sk_buff *skb);

int fast_pppoe_xmit(struct sk_buff *skb);

#endif

#if defined(CONFIG_OPENWRT_SDK)
#if defined(CONFIG_RTL_IPTABLES_FAST_PATH)
#define NET_RX_CN_LOW		2   /* storm alert, just in case */
#define NET_RX_CN_MOD		3   /* Storm on its way! */
#define NET_RX_CN_HIGH		4   /* The storm is here */
#define NET_RX_BAD		5  /* packet dropped due to kernel error */
#define NET_RX_PASSBY	6 /* packet pass by for next process */
#endif

#if defined(CONFIG_RTL_819X)
static inline struct dst_entry *rtl_skb_dst(const struct sk_buff *skb)
{
	return (struct dst_entry *)(skb->_skb_refdst);
}
#endif

#if defined 	(CONFIG_RTL865X_LANPORT_RESTRICTION)	
#define NET_RX_AUTH_BLOCK	6 
#endif
#endif

#ifdef CONFIG_RTL_FAST_IPV6
#define CONFIG_V6_UDP_FRAG_CACHE 1	
struct V6_Tuple
{
	/* hash 5 tuple */
    struct  in6_addr	saddr;
    struct  in6_addr	daddr;	
	uint32	protocol;
	uint32	sport;
	uint32	dport;	
};
int ipv6_fast_enter(struct sk_buff *pskb);
int rtk_addV6Connection(struct V6_Tuple *PathEntry, struct sk_buff *pskb ,void *ct);
int rtk_delV6Connection(struct V6_Tuple *PathEntry);
int rtl_V6_Cache_Timer_update(void *ct);

#endif
#ifdef URL_CONTENT_AUTHENTICATION
#define RTL_UNAUTH_BUFFER_SIZE 8
#define RTL_URL_CONTENT_READED 0x1

typedef struct _unAuth_skb_s
{
	struct list_head list;
	int id; /*skb->iphdr.id*/
	uint32 saddr;
	uint32 daddr;
	uint32 flag; /*whether the url content is readed by application....*/
	struct sk_buff *skb;
} unAuth_skb_t;

int rtl_urlContent_auth(struct sk_buff *skb);
#endif

#if defined(FAST_L2TP)
#if 0
	#define DEBUGP	printk
#else
	#define DEBUGP(fmt, args...) {}
#endif


#define control_message 0x8000
#define connect_control 0xc
#define stop_control 0x4
#define call_reply 0xb

struct l2tp_info
{
	void *wan_dev;
	void *ppp0_dev;
	unsigned long last_xmit;
	__u32 daddr;
	__u32 saddr;
	__u16 tid;                   /* Tunnel ID */
	__u16 cid;                   /* Caller ID */
        unsigned char mac_header[ETH_HLEN];
	__u16 valid;

	__u16 sport;
};

struct l2tp_ext_hdr
{
	__u16 source;
	__u16 dest;
	__u16 len;
	__u16 checksum;
	__u16 type;
	__u16 tid;
	__u16 cid;
	__u16 addr_control;
	__u16 protocol;
};

struct Rus_l2tp_ext_hdr
{
	//udp header
	__u16 source;
	__u16 dest;
	__u16 len;
	__u16 checksum;

	//l2tp header
	__u16 type;
	__u16 tid;
	__u16 cid;
	
	//ppp header
	__u16 addr_control;
	__u16 protocol;

	// Lcp reply header
	unsigned char code;
	unsigned char id;
	__u16 lcp_length;
	unsigned int imagicNumber;
	unsigned int message;
};

struct avp_info
{
	__u16 length;
        __u16 vendorid;
	__u16 attr;
	__u16 mss_type;
};

struct l2tp_header
{
	__u16 ver;                   /* Version and friends */
	__u16 length;                /* Optional Length */
	__u16 tid;                   /* Tunnel ID */
	__u16 cid;                   /* Caller ID */
	__u16 Ns;                    /* Optional next sent */
	__u16 Nr;                    /* Optional next received */
};
extern void (*l2tp_tx_id_hook)(void *skb,int tx,int rx);

#endif


#if defined(FAST_PPTP)

#if 0
	#define FAST_PPTP_PRINT	printk
#else
	#define FAST_PPTP_PRINT(fmt, args...) {}
#endif

/*
struct pptp_info {
	struct net_device *wan_dev;
	unsigned int tx_seqno;
	unsigned int rx_seqno;
	__u32 saddr;
	__u32 daddr;
	__u16 callID;
	__u16 peer_callID;
	__u16 tx_ipID;
	__u16 ipID;
	struct net_device *ppp0_dev;
	struct net_device *lan_dev;
	unsigned char mac_header[ETH_HLEN];
	unsigned int tx_seqno_daemon;
	unsigned int rx_seqno_daemon;
	int ppp_hdr_len;
	unsigned char ppp_hdr[4];
};
*/

struct pptp_acc_info {
	unsigned long fast_pptp_lastxmit;
	int valid;
	void *wanDev;
	unsigned char ourMac[ETHER_ADDR_LEN];
	__u16 ourCallID;
	__u32 ourIp;
	unsigned char peerMac[ETHER_ADDR_LEN];
	__u16 peerCallID;
	__u32 peerIp;
	unsigned int tx_seqno;
	unsigned int rx_seqno;
	__u16 tx_ipID;
	__u16 ipID;
	unsigned int tx_seqno_daemon;
	unsigned int rx_seqno_daemon;
	void *pppDev;
	struct ppp_channel *pppChan;
};

extern int fast_pptp_fw;


/* following define are imported from kerenl */
#define SC_COMP_RUN     0x00001000      /* compressor has been inited */
enum NPmode {
     NPMODE_PASS,                /* pass the packet through */
     NPMODE_DROP,                /* silently drop the packet */
     NPMODE_ERROR,               /* return an error */
     NPMODE_QUEUE                /* save it up for later. */
};

#define NUM_NP	6		/* Number of NPs. */
struct ppp_file {
	enum {
		INTERFACE=1, CHANNEL
	}		kind;
	struct sk_buff_head xq;		/* pppd transmit queue */
	struct sk_buff_head rq;		/* receive queue for pppd */
	wait_queue_head_t rwait;	/* for poll on reading /dev/ppp */
	atomic_t	refcnt;		/* # refs (incl /dev/ppp attached) */
	int		hdrlen;		/* space to leave for headers */
	int		index;		/* interface unit / channel number */
	int		dead;		/* unit/channel has been shut down */
};

//To sync with drivers/net/ppp_generic.c
struct ppp {
	struct ppp_file	file;		/* stuff for read/write/poll 0 */
	struct file	*owner;		/* file that owns this unit 48 */
	struct list_head channels;	/* list of attached channels 4c */
	int		n_channels;	/* how many channels are attached 54 */
	spinlock_t	rlock;		/* lock for receive side 58 */
	spinlock_t	wlock;		/* lock for transmit side 5c */
	int		mru;		/* max receive unit 60 */
#if defined(CONFIG_PPP_MPPE_MPPC)
	int		mru_alloc;	/* MAX(1500,MRU) for dev_alloc_skb() */
#endif
	unsigned int	flags;		/* control bits 64 */
	unsigned int	xstate;		/* transmit state bits 68 */
	unsigned int	rstate;		/* receive state bits 6c */
	int		debug;		/* debug flags 70 */
	struct slcompress *vj;		/* state for VJ header compression */
	enum NPmode	npmode[NUM_NP];	/* what to do with each net proto 78 */
	struct sk_buff	*xmit_pending;	/* a packet ready to go out 88 */
	struct compressor *xcomp;	/* transmit packet compressor 8c */
	void		*xc_state;	/* its internal state 90 */
	struct compressor *rcomp;	/* receive decompressor 94 */
	void		*rc_state;	/* its internal state 98 */
	unsigned long	last_xmit;	/* jiffies when last pkt sent 9c */
	unsigned long	last_recv;	/* jiffies when last pkt rcvd a0 */
	struct net_device *dev;		/* network interface device a4 */
	int		closing;	/* is device closing down? a8 */
#ifdef CONFIG_PPP_MULTILINK
	int		nxchan;		/* next channel to send something on */
	u32		nxseq;		/* next sequence number to send */
	int		mrru;		/* MP: max reconst. receive unit */
	u32		nextseq;	/* MP: seq no of next packet */
	u32		minseq;		/* MP: min of most recent seqnos */
	struct sk_buff_head mrq;	/* MP: receive reconstruction queue */
#endif /* CONFIG_PPP_MULTILINK */
	struct net_device_stats stats;	/* statistics */
#ifdef CONFIG_PPP_FILTER
	struct sock_filter *pass_filter;	/* filter for packets to pass */
	struct sock_filter *active_filter;/* filter for pkts to reset idle */
	unsigned pass_len, active_len;
#endif /* CONFIG_PPP_FILTER */
	struct net	*ppp_net;	/* the net we belong to */
};

#if defined(CONFIG_RTL_HW_NAPT_REFINE_KERNEL_HOOKS)
/*
 * Private data structure for each channel.
 * This includes the data structure used for multilink.
 */
struct channel {
	struct ppp_file	file;		/* stuff for read/write/poll */
	struct list_head list;		/* link in all/new_channels list */
	struct ppp_channel *chan;	/* public channel data structure */
	struct rw_semaphore chan_sem;	/* protects `chan' during chan ioctl */
	spinlock_t	downl;		/* protects `chan', file.xq dequeue */
	struct ppp	*ppp;		/* ppp unit we're connected to */
	struct net	*chan_net;	/* the net channel belongs to */
	struct list_head clist;		/* link in list of channels per unit */
	rwlock_t	upl;		/* protects `ppp' */
#ifdef CONFIG_PPP_MULTILINK
	u8		avail;		/* flag used in multilink stuff */
	u8		had_frag;	/* >= 1 fragments have been sent */
	u32		lastseq;	/* MP: last sequence # received */
	int		speed;		/* speed of the corresponding ppp channel*/
#endif /* CONFIG_PPP_MULTILINK */
#if defined(CONFIG_RTL_PPPOE_HWACC) || defined (CONFIG_RTL_FAST_PPPOE)
    u8      pppoe;
    u8      rsv1;
    u16     rsv2;
#endif
};
#endif

extern void (*sync_tx_pptp_gre_seqno_hook)(void *skb);
#endif
#if defined(CUSTOM_RSP_PACKET)
void register_customRspHook(int *_cusRsp401_func,int *_cusRspTCPFinAck_func,int *_cusRspTCPEndAck_func);
void unregister_customRspHook(void);
void register_customRspStr(char *_str);
void unregister_customRspStr(void);
 int  GenerateHTTP401(void *skb);
#endif
#ifdef DOS_FILTER
	extern int filter_enter(void *skb);
	extern int __init filter_init(void);
	extern void __exit filter_exit(void);
	extern void filter_addconnect(ipaddr_t ipaddr);
	extern void filter_delconnect(ipaddr_t ipaddr);
	extern int filter_checkConnect(__u32 ipaddr);
#endif

#ifdef FAST_PPTP
	extern void fast_pptp_filter(void *skb);
	extern void fast_pptp_sync_rx_seq(void *skb);
	extern int __init fast_pptp_init(void);
	extern void __exit fast_pptp_exit(void);
	extern int fast_pptp_to_lan(void **pskb);
	extern int Check_GRE_rx_net_device(void *skb);
	extern int pptp_tcp_finished;
#endif

#ifdef FAST_L2TP
	extern int __init fast_l2tp_init(void);
	extern void __exit fast_l2tp_exit(void);
	#if !defined(CONFIG_OPENWRT_SDK)
	extern int fast_l2tp_to_wan(void *skb);
	#endif
	extern int fast_l2tp_rx(void *skb);
	extern void l2tp_id(void *skb,int tx,int rx);
	extern int fast_l2tp_fw;
#endif


#ifdef CONFIG_RTL_FAST_IPV6
	extern int __init ipv6_fast_forward_init(void);
	extern void __exit ipv6_fast_forward_exit(void);
	extern int init_V6_table_path(int v6_path_tbl_list_max, int v6_path_tbl_entry_max);
	extern int V6_udp_fragCache_init(int V6_udp_frag_entry_max);
	extern int fast_ipv6_fw;
#endif

/* ---------------------------------------------------------------------------------------------------- */

#define	IFNAME_LEN_MAX		16
#define	MAC_ADDR_LEN_MAX		18
#if	!defined(NO_ARP_USED)
#define	ARP_TABLE_LIST_MAX		32
#define	ARP_TABLE_ENTRY_MAX	128
#endif

#define	ROUTE_TABLE_LIST_MAX	16
#define	ROUTE_TABLE_ENTRY_MAX	64

#if defined(CONFIG_RTL_8198) || defined(CONFIG_RTL_92D_SUPPORT) ||defined(CONFIG_RTL_819XD) ||(defined(CONFIG_RTL_8196E)&&defined(CONFIG_RTL_SDRAM_GE_32M)) || defined(CONFIG_RTL_8198C) || (defined(CONFIG_RTL_8197F) && !defined(CONFIG_RTL_SHRINK_MEMORY_SIZE))
#if defined(CONFIG_OPENWRT_SDK)
#define	NAPT_TABLE_LIST_MAX	8192
#define	NAPT_TABLE_ENTRY_MAX	8192
#define	PATH_TABLE_LIST_MAX	8192
#else
#define	NAPT_TABLE_LIST_MAX	4096
#define	NAPT_TABLE_ENTRY_MAX	4096
#define	PATH_TABLE_LIST_MAX	4096
#endif
#else
#define	NAPT_TABLE_LIST_MAX	768		
#define	NAPT_TABLE_ENTRY_MAX	768	
#define	PATH_TABLE_LIST_MAX		768
#endif

#define	PATH_TABLE_ENTRY_MAX	(NAPT_TABLE_ENTRY_MAX<<1)

#if	defined(CONFIG_RTL_FAST_FILTER)
#define NF_DROP 	0
#define NF_FASTPATH	1
#define NF_LINUX	2
#define NF_MARK 	3
#define NF_REPEAT	4
#define NF_OMIT 	5
#endif

#if 0
#define	ETHER_ADDR_LEN		6
typedef struct ether_addr_s {
        uint8 octet[ETHER_ADDR_LEN];
} ether_addr_t;
#endif

/* ########### API #################################################################################### */
enum LR_RESULT
{
	/* Common error code */
	LR_SUCCESS = 0,						/* Function Success */
	LR_FAILED = -1,						/* General Failure, not recommended to use */
	LR_ERROR_PARAMETER = -2,				/* The given parameter error */
	LR_EXIST = -3,							/* The entry you want to add has been existed, add failed */
	LR_NONEXIST = -4,						/* The specified entry is not found */

	LR_NOBUFFER = -1000,					/* Out of Entry Space */
	LR_INVAPARAM = -1001,					/* Invalid parameters */
	LR_NOTFOUND = -1002,					/* Entry not found */
	LR_DUPENTRY = -1003,					/* Duplicate entry found */
};

#if 0
enum IF_FLAGS
{
	IF_NONE,
	IF_INTERNAL = (0<<1),					/* This is an internal interface. */
	IF_EXTERNAL = (1<<1),					/* This is an external interface. */
};

enum FDB_FLAGS
{
	FDB_NONE = 0,
};
#endif

enum ARP_FLAGS
{
	ARP_NONE = 0,
};

enum RT_FLAGS
{
	RT_NONE = 0,
};

enum SE_TYPE
{
	SE_PPPOE = 1,
	SE_PPTP = 2,
	SE_L2TP = 3,
};
enum SE_FLAGS
{
	SE_NONE = 0,
};

enum NP_PROTOCOL
{
	NP_UDP = 1,
	NP_TCP = 2,
#if  defined(CONFIG_RTL_FAST_GRE)	
	NP_GRE = 3,
#endif
};
enum NP_FLAGS
{
	NP_NONE = 0,
};

typedef struct _rtl_fp_napt_entry
{
	enum NP_PROTOCOL protocol;
	ipaddr_t intIp;
	uint32 intPort;
	ipaddr_t extIp;
	uint32 extPort;
	ipaddr_t remIp;
	uint32 remPort;
}rtl_fp_napt_entry;

/* ---------------------------------------------------------------------------------------------------- */
#if 0
enum LR_RESULT rtk_addInterface( uint8* ifname, ipaddr_t ipAddr, ether_addr_t* gmac, uint32 mtu, enum IF_FLAGS flags );
enum LR_RESULT rtk_configInterface( uint8* ifname, uint32 vlanId, uint32 fid, uint32 mbr, uint32 untag, enum IF_FLAGS flags );
enum LR_RESULT rtk_delInterface( uint8* ifname );
enum LR_RESULT rtk_addFdbEntry( uint32 vid, uint32 fid, ether_addr_t* mac, uint32 portmask, enum FDB_FLAGS flags );
enum LR_RESULT rtk_delFdbEntry( uint32 vid, uint32 fid, ether_addr_t* mac );
#endif
enum LR_RESULT rtk_addArp( ipaddr_t ip, ether_addr_t* mac, enum ARP_FLAGS flags );
enum LR_RESULT rtk_modifyArp( ipaddr_t ip, ether_addr_t* mac, enum ARP_FLAGS flags );
enum LR_RESULT rtk_delArp( ipaddr_t ip );
enum LR_RESULT rtk_addRoute( ipaddr_t ip, ipaddr_t mask, ipaddr_t gateway, uint8* ifname, enum RT_FLAGS flags );
enum LR_RESULT rtk_modifyRoute( ipaddr_t ip, ipaddr_t mask, ipaddr_t gateway, uint8* ifname, enum RT_FLAGS flags );
enum LR_RESULT rtk_delRoute( ipaddr_t ip, ipaddr_t mask );
enum LR_RESULT rtk_addSession( uint8* ifname, enum SE_TYPE seType, uint32 sessionId, enum SE_FLAGS flags );
enum LR_RESULT rtk_delSession( uint8* ifname );

enum LR_RESULT rtk_addNaptConnection(rtl_fp_napt_entry *fpNaptEntry,
#if defined(IMPROVE_QOS)
									void *pskb, void *ct,
#endif
									enum NP_FLAGS flags);
enum LR_RESULT rtk_delNaptConnection( rtl_fp_napt_entry *fpNaptEntry);
#if 0//defined(IMPROVE_QOS)
enum LR_RESULT rtk_idleNaptConnection(rtl_fp_napt_entry *fpNaptEntry, uint32 interval);
#else
enum LR_RESULT rtk_idleNaptConnection(rtl_fp_napt_entry *fpNaptEntry, uint32 interval, unsigned long *lastUsed);
#endif
#if defined(CONFIG_BRIDGE)
int32 rtl_br_fdb_time_update(void *br, void *fdb, const unsigned char *addr);
#endif
int32 rtl_fp_dev_queue_xmit_check(struct sk_buff *skb, struct net_device *dev);
int32 rtl_fp_dev_hard_start_xmit_check(struct sk_buff *skb, struct net_device *dev, struct netdev_queue *txq);

#if defined(IMPROVE_QOS) || defined(CONFIG_RTL_HW_QOS_SUPPORT)
//To query hardware address based on IP through arp table of dev
int arp_req_get_ha(__be32 queryIP, struct net_device *dev, unsigned char * resHwAddr);
#endif

#if	defined(CONFIG_RTL_HW_QOS_SUPPORT)
int32 rtl_qosGetSkbMarkByNaptEntry(rtl865x_napt_entry *naptEntry, rtl865x_qos_mark *qosMark, struct sk_buff *pskb);
#endif
int fast_path_before_nat_check(struct sk_buff *skb, struct iphdr *iph, uint32 iphProtocol);

#if defined(CONFIG_PROC_FS)
int fastpath_dump_napt_entry_num(char *page, int len);
#endif

/* [MARCO FUNCTION] ========================================================================= */
#define	MAC2STR(addr) \
	((unsigned char *)&addr)[0], \
	((unsigned char *)&addr)[1], \
	((unsigned char *)&addr)[2], \
	((unsigned char *)&addr)[3], \
	((unsigned char *)&addr)[4], \
	((unsigned char *)&addr)[5]

#define	FASTPATH_MAC2STR(mac, hbuffer) \
	do { \
		int j,k; \
		const char hexbuf[] =  "0123456789ABCDEF"; \
		for (k=0,j=0;k<MAC_ADDR_LEN_MAX && j<6;j++) { \
			hbuffer[k++]=hexbuf[(mac->octet[j]>>4)&15 ]; \
			hbuffer[k++]=hexbuf[mac->octet[j]&15     ]; \
			hbuffer[k++]=':'; \
		} \
		hbuffer[--k]=0; \
	} while(0)	/* Mac Address to String */
#if 0
#define FASTPATH_ADJUST_CHKSUM_NAT(ip_mod, ip_org, chksum) \
	do { \
		s32 accumulate = 0; \
		if (((ip_mod) != 0) && ((ip_org) != 0)){ \
			accumulate = ((ip_org) & 0xffff); \
			accumulate += (( (ip_org) >> 16 ) & 0xffff); \
			accumulate -= ((ip_mod) & 0xffff); \
			accumulate -= (( (ip_mod) >> 16 ) & 0xffff); \
		} \
		accumulate += ntohs(chksum); \
		if (accumulate < 0) { \
			accumulate = -accumulate; \
			accumulate = (accumulate >> 16) + (accumulate & 0xffff); \
			accumulate += accumulate >> 16; \
			chksum = htons((uint16) ~accumulate); \
		} else { \
			accumulate = (accumulate >> 16) + (accumulate & 0xffff); \
			accumulate += accumulate >> 16; \
			chksum = htons((uint16) accumulate); \
		} \
	}while(0)	/* Checksum adjustment */

#define FASTPATH_ADJUST_CHKSUM_NPT(port_mod, port_org, chksum) \
	do { \
		s32 accumulate = 0; \
		if (((port_mod) != 0) && ((port_org) != 0)){ \
			accumulate += (port_org); \
			accumulate -= (port_mod); \
		} \
		accumulate += ntohs(chksum); \
		if (accumulate < 0) { \
			accumulate = -accumulate; \
			accumulate = (accumulate >> 16) + (accumulate & 0xffff); \
			accumulate += accumulate >> 16; \
			chksum = htons((uint16) ~accumulate); \
		} else { \
			accumulate = (accumulate >> 16) + (accumulate & 0xffff); \
			accumulate += accumulate >> 16; \
			chksum = htons((uint16) accumulate); \
		} \
	}while(0)	/* Checksum adjustment */


#define FASTPATH_ADJUST_CHKSUM_NAPT(ip_mod, ip_org, port_mod, port_org, chksum) \
	do { \
		s32 accumulate = 0; \
		if (((ip_mod) != 0) && ((ip_org) != 0)){ \
			accumulate = ((ip_org) & 0xffff); \
			accumulate += (( (ip_org) >> 16 ) & 0xffff); \
			accumulate -= ((ip_mod) & 0xffff); \
			accumulate -= (( (ip_mod) >> 16 ) & 0xffff); \
		} \
		if (((port_mod) != 0) && ((port_org) != 0)){ \
			accumulate += (port_org); \
			accumulate -= (port_mod); \
		} \
		accumulate += ntohs(chksum); \
		if (accumulate < 0) { \
			accumulate = -accumulate; \
			accumulate = (accumulate >> 16) + (accumulate & 0xffff); \
			accumulate += accumulate >> 16; \
			chksum = htons((uint16) ~accumulate); \
		} else { \
			accumulate = (accumulate >> 16) + (accumulate & 0xffff); \
			accumulate += accumulate >> 16; \
			chksum = htons((uint16) accumulate); \
		} \
	}while(0)	/* Checksum adjustment */
#else

#define FASTPATH_ADJUST_CHKSUM_NAT(ip_mod, ip_org, chksum) \
	do { \
		s32 accumulate = 0; \
		accumulate = ((ip_org) & 0xffff); \
		accumulate += (( (ip_org) >> 16 ) & 0xffff); \
		accumulate -= ((ip_mod) & 0xffff); \
		accumulate -= (( (ip_mod) >> 16 ) & 0xffff); \
		accumulate += ntohs(chksum); \
		if (accumulate < 0) { \
			accumulate = -accumulate; \
			accumulate = (accumulate >> 16) + (accumulate & 0xffff); \
			accumulate += accumulate >> 16; \
			chksum = htons((uint16) ~accumulate); \
		} else { \
			accumulate = (accumulate >> 16) + (accumulate & 0xffff); \
			accumulate += accumulate >> 16; \
			chksum = htons((uint16) accumulate); \
		} \
	}while(0)	/* Checksum adjustment */

#define FASTPATH_ADJUST_CHKSUM_NPT(port_mod, port_org, chksum) \
	do { \
		s32 accumulate = 0; \
		accumulate += (port_org); \
		accumulate -= (port_mod); \
		accumulate += ntohs(chksum); \
		if (accumulate < 0) { \
			accumulate = -accumulate; \
			accumulate = (accumulate >> 16) + (accumulate & 0xffff); \
			accumulate += accumulate >> 16; \
			chksum = htons((uint16) ~accumulate); \
		} else { \
			accumulate = (accumulate >> 16) + (accumulate & 0xffff); \
			accumulate += accumulate >> 16; \
			chksum = htons((uint16) accumulate); \
		} \
	}while(0)	/* Checksum adjustment */


#define FASTPATH_ADJUST_CHKSUM_NAPT(ip_mod, ip_org, port_mod, port_org, chksum) \
	do { \
		s32 accumulate = 0; \
		accumulate = ((ip_org) & 0xffff); \
		accumulate += (( (ip_org) >> 16 ) & 0xffff); \
		accumulate -= ((ip_mod) & 0xffff); \
		accumulate -= (( (ip_mod) >> 16 ) & 0xffff); \
		accumulate += (port_org); \
		accumulate -= (port_mod); \
		accumulate += ntohs(chksum); \
		if (accumulate < 0) { \
			accumulate = -accumulate; \
			accumulate = (accumulate >> 16) + (accumulate & 0xffff); \
			accumulate += accumulate >> 16; \
			chksum = htons((uint16) ~accumulate); \
		} else { \
			accumulate = (accumulate >> 16) + (accumulate & 0xffff); \
			accumulate += accumulate >> 16; \
			chksum = htons((uint16) accumulate); \
		} \
	}while(0)	/* Checksum adjustment */

#endif
/* ---------------------------------------------------------------------------------------------------- */
#ifdef	DEBUG_PROCFILE
void init_fastpath_debug_proc(void);
void remove_fastpath_debug_proc(void);
#endif
#ifndef NO_ARP_USED
int init_table_arp(int arp_tbl_list_max, int arp_tbl_entry_max);
#endif
#ifndef DEL_ROUTE_TBL
int init_table_route(int route_tbl_list_max, int route_tbl_entry_max);
#endif
#ifndef DEL_NAPT_TBL
int init_table_napt(int napt_tbl_list_max, int napt_tbl_entry_max);
#endif

#ifdef CONFIG_RTL_FAST_IPV6
u_int8_t rtl_get_ct_protonum_ipv6(void *ct_ptr, enum ip_conntrack_dir dir);
#endif

int init_table_path(int path_tbl_list_max, int path_tbl_entry_max);
int udp_fragCache_init(int udp_frag_entry_max);
int negative_fragCache_init(void);
void fastpath_set_qos_mark(struct sk_buff *skb, unsigned int preRouteMark, unsigned int postRouteMark);
int fast_path_pre_process_check(struct iphdr *iph, struct tcphdr *tcphupuh, struct sk_buff *skb);
int fast_path_post_process_xmit_check(struct iphdr *iph, struct tcphdr *tcphupuh, struct sk_buff *skb);
int fast_path_post_process_return_check(struct iphdr *iph, struct tcphdr *tcphupuh, struct sk_buff *skb);
int ip_finish_output3(struct sk_buff *skb);
#if defined (CONFIG_RTL_FAST_PPPOE)
int ip_finish_output4(struct sk_buff *skb);
#endif
__IRAM_GEN int enter_fast_path(void *skb);
uint8 *FastPath_Route(ipaddr_t dIp);
int FastPath_Enter(struct sk_buff **skb);
extern int Get_fast_pptp_fw(void);
#ifdef CONFIG_FAST_PATH_MODULE
extern int (*fast_path_hook)(struct sk_buff **pskb) ;
extern enum LR_RESULT (*FastPath_hook1)( ipaddr_t ip, ipaddr_t mask );
extern enum LR_RESULT (*FastPath_hook2)( ipaddr_t ip, ipaddr_t mask, ipaddr_t gateway, uint8* ifname, enum RT_FLAGS flags );
extern int (*fast_path_hook)(struct sk_buff **pskb) ;
extern enum LR_RESULT (*FastPath_hook3)( ipaddr_t ip, ipaddr_t mask, ipaddr_t gateway, uint8* ifname, enum RT_FLAGS flags );
extern  enum LR_RESULT (*FastPath_hook4)( rtl_fp_napt_entry *fpNaptEntry);
extern enum LR_RESULT (*FastPath_hook5)( ipaddr_t ip, ether_addr_t* mac, enum ARP_FLAGS flags );
#if defined(CONFIG_OPENWRT_SDK)
extern enum LR_RESULT (*FastPath_hook6)( rtl_fp_napt_entry *fpNaptEntry,
#if defined(IMPROVE_QOS)
									void *pskb, void *ct,
#endif
                                                               enum NP_FLAGS flags);
#else
enum LR_RESULT (*FastPath_hook6)( rtl_fp_napt_entry *fpNaptEntry,
#if defined(IMPROVE_QOS)
									struct sk_buff *pskb, struct nf_conn *ct,
#endif
                                                               enum NP_FLAGS flags);
#endif
extern enum LR_RESULT (*FastPath_hook7)( ipaddr_t ip );
extern enum LR_RESULT (*FastPath_hook8)( ipaddr_t ip, ether_addr_t* mac, enum ARP_FLAGS flags );
extern int (*FastPath_hook9)( void );
#if defined(CONFIG_OPENWRT_SDK)
extern int (*FastPath_hook10)(void *skb);
#else
extern int (*FastPath_hook10)(struct sk_buff *skb);
#endif
extern enum LR_RESULT (*FastPath_hook11)(rtl_fp_napt_entry *fpNaptEntry, uint32 interval);
#if defined(CONFIG_RTL_HW_QOS_SUPPORT) && defined(CONFIG_OPENWRT_SDK)
extern int (*qosGetSkbMarkByNaptEntryHook)(rtl865x_napt_entry *naptEntry, rtl865x_qos_mark *qosMark, struct sk_buff *pskb);
#endif
extern  int fast_pptp_to_wan(void *skb);
#endif

/* ---------------------------------------------------------------------------------------------------- */
#if defined(FASTPATH_FILTER)
#define RTL_FILTER_CONTENT_MAXNUM 40
#define RTL_TABLE_FILTER_ENTRY_COUNT 10
#define IP_RANGE_TABLE 	1
#define MAC_TABLE		2
#define URL_KEY_TABLE	3
#define SCHEDULT_TABLE	4
#define CONTENT_FILTER	5

typedef struct _rlt_filter_table_head
{
	struct list_head filter_table;
	struct list_head filter_items;
	uint32 flag;
}rlt_filter_table_head;

//ip range table
#define RTL_IP_RANGE_FILTER_ENTRY_COUNT 20
typedef struct _filter_ipRange_fastpath
{
	struct list_head list;
	uint32 addr_start; /*ipaddr start*/
	uint32 addr_end; /*address end*/
	uint32 flag; /*0 bit: default action[0:block,1:forward];1 bit: src ip or dest ip[0:src, 1:dest];2 bit: refer both direction*/
			    /*bit 9: valid 1; invalid 0*/
}rtl_ipRange_fastpath;

//url and keyword
#define RTL_URL_FILTER_CONTENT_MAXNUM_FASTPATH 40
typedef struct _url_table_head_entry_fastpath
{
	struct list_head list;
	uint32 flag;
	int (*func)(struct sk_buff *skb);
}url_table_head_entry_fastpath;

typedef struct _url_table_unit_entry_fastpath
{
	struct list_head list;
	char url_content[RTL_URL_FILTER_CONTENT_MAXNUM_FASTPATH];
}url_table_unit_entry_fastpath;

typedef struct _url_entry_fastpath
{
	struct list_head list;
	char url_content[RTL_URL_FILTER_CONTENT_MAXNUM_FASTPATH];
	uint32 flag;
}rtl_url_entry_fastpath;

typedef struct _rtl_mac_entry_fastpath
{
	struct list_head list;
	char mac[ETHER_ADDR_LEN];
	uint8 flag;
}rtl_mac_entry_fastpath;

typedef struct _rtl_sch_entry_fastpath
{
	struct list_head list;
	uint32 	weekMask; /*bit0: sunday, bit 1: monday, .... bit 6 saturday, bit7: (1: all days, monday~sunday)*/
	uint32 	startTime; /*minutes, ex. 5:21 = 5*60+21 minutes*/
	uint32 	endTime; /*minutes*/
	uint8 	allTimeFlag;/*if alltime(00:00~23:59:59), please set this flag...*/
	uint8 	flag; /* bit0( 0: deny, 1: allow), bit1( 1:block all http packet), bit2(0:default deny; 1: default allow)*/
}rtl_sch_entry_fastpath;

typedef struct _filter_table_info
{
	uint32 type;	//type
	int (*func)(struct sk_buff *skb, void *data);
}filter_table_info;

typedef struct _filter_table_list
{
	struct list_head table_list;
	struct list_head item_list;
	uint32 type;	//type
	uint32 flag;
	uint32 num;
	int (*func)(struct sk_buff *skb, void *data);
}filter_table_list;

typedef struct _filter_item_entry
{
	struct list_head item_list;
	struct list_head rule_list;
	uint32 relation_flag;	//bit0: is the first condition? 1;0
						//bit1: have next condition? 1:0 [next table condition]
						//bit2: have "and" logic condition?1:0
						//bit3: default action: 1 block;0 forward
						//bit4~7: the index of "and" logic rule
						//bit8: all match flag 1: all, 0: not all
						//bit9: NULL flag, 1:NULL, 0: not NULL
	uint32 index;
	uint32 flag;
#ifdef CONFIG_URL_FILTER_USER_MODE_SUPPORT
	char userMode;
	char macAddr[2*ETHER_ADDR_LEN+1];
	uint32 ipAddr;
#endif
	char data[RTL_FILTER_CONTENT_MAXNUM];
}filter_item_entry;

typedef struct _rtl_mac_info
{
	char mac[ETHER_ADDR_LEN];
}rtl_mac_info;

#if !defined(CONFIG_OPENWRT_SDK)
/* --- PATH Table Structures --- */
struct Path_List_Entry
{
	uint8			vaild;
	uint8			type;
	uint8			course;			/* 1:Out-Bonud 2:In-Bound */

	/*diff with sd2 port1 start*/
	enum NP_PROTOCOL	protocol;
	//uint32		protocol;
	uint16			in_dPort;
	uint16			in_sPort;
	ipaddr_t		in_sIp;
	ipaddr_t		in_dIp;
	ipaddr_t		out_sIp;
	uint16			out_sPort;
	ipaddr_t		out_dIp;
	uint16			out_dPort;
	/*diff with sd2 port1 end*/
	uint32			last_used;
	uint8			*out_ifname;
#ifndef NO_ARP_USED
	struct Arp_List_Entry	*arp_entry;		/* for Out-dMac */
#endif

#if defined(IMPROVE_QOS)
#if defined(CONFIG_NET_SCHED)
	uint32	PreMark;		//nfmark of uplink
	uint32	PostMark;		//nfmark of downlink
#endif
	void *ct;
	unsigned long		   timeout;
#endif
#if 1//defined(CONFIG_RTL_IP_POLICY_ROUTING_SUPPORT)
	uint32 	policy_route_mark;
#endif

//	struct dst_entry 	*dst;
	unsigned int packet_bypassed;
	unsigned int packet_bypass_checked;

#if 1//defined(CONFIG_RTL_DSCP_IPTABLE_CHECK) && defined(IMPROVE_QOS)
          uint8  dscp_in;//inbound dscp shoud changed to
          uint8  dscp_out;//outbound dscp shoud changed to
#endif

#if 1 //defined(CONFIG_RTL_VLANID_IPTABLE_CHECK) && defined(IMPROVE_QOS)
          uint16  vlanid_in;//inbound vlanid shoud changed to
          uint16  vlanid_out;//outbound vlanid shoud changed to
#endif

#if 1//defined(CONFIG_RTL_VLANPRI_IPTABLE_CHECK) && defined(IMPROVE_QOS)
	   	  uint16  vlanpri_out;
#endif

	CTAILQ_ENTRY(Path_List_Entry) path_link;
	CTAILQ_ENTRY(Path_List_Entry) tqe_link;
};
#endif
//============================================================================

extern filter_table_list table_list_head;

#define	RTL_FP_FLT_TBL_INIT_VALUE	1
#define	RTL_FP_FLT_TBL_EMPTY	(table_list_head.num==RTL_FP_FLT_TBL_INIT_VALUE)
#define	RTL_FP_FLT_TBL_NOT_INIT	(table_list_head.num<RTL_FP_FLT_TBL_INIT_VALUE)
#endif

#define RTL_FASTPATH_PPP0_DEV_NAME	"ppp0"

#if 1//defined(CONFIG_RTL_NF_CONNTRACK_GARBAGE_NEW)
void rtl_fp_mark_invalid(void *ct);
#endif
#if !defined(CONFIG_OPENWRT_SDK)
void mark_path_invalid(uint32 sIp, uint16 sPort, uint32 dIp, uint16 dPort, uint16 iphProtocol);
#endif

#define FASTPTH_INDEPENDENCE_KERNEL 1
#if defined(FASTPTH_INDEPENDENCE_KERNEL)
extern struct dst_entry *dst_tmp;
extern int ppp_start_xmit(struct sk_buff *skb, struct net_device *dev);
extern struct sk_buff *ppp_receive_nonmp_frame(struct ppp *ppp, struct sk_buff *skb, int is_fast_fw);

__be16 rtl_get_skb_protocol(struct sk_buff *skb);
void rtl_set_skb_protocol(struct sk_buff *skb,__be16 protocol);


unsigned char rtl_get_skb_type(struct sk_buff *skb);

__wsum rtl_get_skb_csum(struct sk_buff *skb);

#if !defined(CONFIG_OPENWRT_SDK)
void rtl_dev_put(struct net_device *dev);
#endif
unsigned int rtl_get_skb_len(struct sk_buff *skb);

unsigned char *rtl_get_skb_data(struct sk_buff* skb);
void rtl_set_skb_data(struct sk_buff *skb, int offset, int action);

unsigned char *rtl_skb_mac_header(struct sk_buff * skb);
void rtl_skb_set_mac_header(struct sk_buff *skb, int offset);
int rtl_skb_mac_header_was_set(struct sk_buff *skb);

void rtl_set_skb_dmac(struct sk_buff *skb, void *device);
void rtl_set_skb_smac(struct sk_buff *skb, void *device);


unsigned char *rtl_skb_network_header(struct sk_buff * skb);
void rtl_skb_set_network_header(struct sk_buff * skb,const int offset);
void rtl_skb_reset_network_header(struct sk_buff *skb);
void rtl_set_skb_network_header(struct sk_buff * skb, unsigned char *network_header);

unsigned char *rtl_skb_transport_header(struct sk_buff * skb);
void rtl_skb_set_transport_header(struct sk_buff * skb,const int offset);
void rtl_skb_reset_transport_header(struct sk_buff *skb);
void rtl_set_skb_transport_header(struct sk_buff * skb, unsigned char *transport_header);

unsigned int rtl_get_skb_pppoe_flag(struct sk_buff * skb);
void rtl_set_skb_pppoe_flag(struct sk_buff * skb,unsigned int pppoe_flag);

unsigned char *rtl_skb_pull(struct sk_buff *skb, unsigned int len);
unsigned char *rtl_skb_push(struct sk_buff *skb, unsigned int len);


int rtl_ppp_proto_check(struct sk_buff *skb, unsigned char* ppp_proto);
unsigned int rtl_ipt_do_table(struct sk_buff * skb, unsigned int hook, void *in, void *out);
#if defined(CONFIG_OPENWRT_SDK)
int rtl_ip_route_input(struct sk_buff  *skb, __be32 daddr, __be32 saddr, u8 tos);
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,0)
int rtl_skb_dst_check(struct sk_buff *skb, unsigned long dip,void *ct,uint8 type);
#else
int rtl_skb_dst_check(struct sk_buff *skb);
#endif
#else
int rtl_ip_route_input(struct sk_buff  *skb, __be32 daddr, void *ct);
unsigned int rtl_nf_rtcache_in(struct sk_buff *skb,const void *entry_ct);
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,0)
int rtl_skb_dst_check(struct sk_buff *skb, unsigned long dip, struct iphdr *iph);
#else
int rtl_skb_dst_check(struct sk_buff *skb, struct iphdr *iph);
#endif
void rtl_dst_release_ipv4(struct sk_buff *skb);
#endif
void rtl_set_skb_ip_summed(struct sk_buff *skb, int value);
void rtl_dst_release(struct sk_buff *skb);



__u32 rtl_get_skb_mark(struct sk_buff *skb);
void rtl_set_skb_mark(struct sk_buff *skb, unsigned int value);
void rtl_store_skb_dst(struct sk_buff *skb);
void rtl_set_skb_dst(struct sk_buff *skb);
int rtl_tcp_get_timeouts(void *ptr);
int rtl_arp_req_get_ha(__be32 queryIP, void *device, unsigned char * resHwAddr);



u_int8_t rtl_get_ct_protonum(void *ct_ptr, enum ip_conntrack_dir dir);
unsigned long rtl_get_ct_udp_status(void *ct_ptr);
u_int8_t rtl_get_ct_tcp_state(void *ct_ptr);
__be32 rtl_get_ct_ip_by_dir(void *ct_ptr, enum ip_conntrack_dir dir, int flag);
__be16 rtl_get_ct_port_by_dir(void *ct_ptr, enum ip_conntrack_dir dir, int flag);
void rtl_set_ct_timeout_expires(void *ct_ptr, unsigned long value);
unsigned long rtl_hold_time(void *br_ptr);
void rtl_set_fdb_aging(void *fdb_ptr, unsigned long value);
unsigned long rtl_get_fdb_aging(void *fdb_ptr);

struct ethhdr *rtl_eth_hdr(struct sk_buff *skb);
struct iphdr *rtl_ip_hdr(struct sk_buff *skb);


struct net_device * rtl_get_dev_by_name(char *name);

struct net_device *rtl_get_skb_dev(struct sk_buff* skb);
unsigned char *rtl_get_dev_addr(struct net_device *dev);

void rtl_set_skb_dev(struct sk_buff *skb, struct net_device *dev);

char *rtl_get_skb_dev_name(struct sk_buff *skb);

void rtl_set_skb_inDev(struct sk_buff *skb);

#if !defined(CONFIG_OPENWRT_SDK)
void rtl_skb_policy_route_mark_set(struct sk_buff *skb, unsigned int policy_route_mark, unsigned int *mark_bak);
void rtl_recheck_fp_policy_route_mark(struct sk_buff *skb, uint32 *fp_policy_route_mark);
#endif
struct net_device *rtl_get_skb_rx_dev(struct sk_buff* skb);
void rtl_set_skb_rx_dev(struct sk_buff* skb,struct net_device *dev);

char * rtl_get_ppp_dev_name(struct net_device *ppp_dev);
void * rtl_get_ppp_dev_priv(struct net_device *ppp_dev);

int rtl_call_skb_ndo_start_xmit(struct sk_buff *skb);

void rtl_inc_ppp_stats(struct ppp *ppp, int act, int len);

void rtl_set_skb_tail(struct sk_buff *skb, int offset, int action);
struct sk_buff *rtl_ppp_receive_nonmp_frame(struct ppp *ppp, struct sk_buff *skb, int is_fast_fw);
int rtl_ppp_start_xmit(struct sk_buff *skb, struct net_device *dev);
void rtl_set_skb_cb(struct sk_buff *skb, char *value, int len);
int rtl_ppp_vj_check(struct ppp* ppp);
void *rtl_get_ppp_xmit_pending(struct ppp* ppp);
void rtl_set_ppp_xmit_pending(struct ppp* ppp, struct sk_buff* skb);
void rtl_set_skb_nfct(struct sk_buff *skb, void *value);
struct neighbour *rtl_neigh_lookup(const void *pkey, struct net_device *dev);
struct hh_cache *rtl_get_hh_from_neigh(struct neighbour *neigh);
seqlock_t rtl_get_lock_from_hh(struct hh_cache * hh);
unsigned short rtl_get_len_from_hh(struct hh_cache * hh);
unsigned long *rtl_get_data_from_hh(struct hh_cache * hh);
unsigned int rtl_skb_headroom(struct sk_buff *skb);
int rtl_skb_cloned(struct sk_buff *skb);
int rtl_skb_shared(const struct sk_buff *skb);

#if 1//defined(CONFIG_RTL_DSCP_IPTABLE_CHECK) && defined(IMPROVE_QOS)
#if defined(CONFIG_OPENWRT_SDK)
#if defined(CONFIG_RTL_DSCP_IPTABLE_CHECK) && defined(IMPROVE_QOS)
__u8 rtl_get_skb_orig_dscp(struct sk_buff *skb);
#endif
#else
__u8 rtl_get_skb_orig_dscp(struct sk_buff *skb);
void rtl_set_iph_dscp(struct Path_List_Entry *entry_path,struct iphdr *iph);
void rtl_set_entry_dscp_out(struct Path_List_Entry *entry_path,struct sk_buff *pskb);
void rtl_set_entry_dscp_in(struct Path_List_Entry *entry_path,struct sk_buff *pskb);
#endif
#endif

#if !defined(CONFIG_OPENWRT_SDK)
#if 1//defined(CONFIG_RTL_VLANPRI_IPTABLE_CHECK) && defined(IMPROVE_QOS)
__u16 rtl_get_skb_orig_vlanpri(struct sk_buff *skb); 
uint16 rtl_get_skb_remark_vlanpri(struct sk_buff *skb);
uint16 rtl_restore_skb_vlanpri(struct sk_buff *skb, uint16 pri);
void rtl_set_skb_remark_vlanpri(struct Path_List_Entry *entry_path, struct sk_buff *skb);
void rtl_set_entry_vlanpri(struct Path_List_Entry *entry_path,struct sk_buff *pskb);
void rtl_set_skb_ori_vlanpri(struct sk_buff *skb);
#endif

#if 1 //defined(CONFIG_RTL_VLANID_IPTABLE_CHECK) && defined(IMPROVE_QOS)
uint16 rtl_get_skb_vlanId(struct sk_buff *skb);
void rtl_set_skb_vlanId(struct sk_buff *skb, uint16 vlanId);
void rtl_set_entry_vlanid_in(struct Path_List_Entry *entry_path, struct sk_buff *skb);
void rtl_set_entry_vlanid_out(struct Path_List_Entry *entry_path, struct sk_buff *skb);
#endif

__u8 rtl_get_skb_phy_port(struct sk_buff *skb);
void rtl_set_skb_phy_port(struct sk_buff *skb, uint8 phy_port);
void rtl_set_skb_src_phy_port(struct sk_buff *skb,unsigned char* resMac, int is_wan);
#endif

void rtl_conntrack_drop_check_hook(struct nf_conn *ct_tmp, uint16 ipprotocol, struct nf_conn *ct);
int  rtl_Add_Pattern_ACL_For_ContentFilter(void);
#ifdef CONFIG_RTL_FAST_IPV6
struct ipv6hdr *rtl_ipv6_hdr(const struct sk_buff *skb);
void rtl_store_skb_ipv6_dst(struct sk_buff *skb);
void rtl_set_skb_ipv6_dst(struct sk_buff *skb);
struct  dst_entry *rtl_get_skb_ipv6_dst(struct sk_buff *skb);
struct net_device *rtl_get_skb_ipv6_dst_dev(struct sk_buff *skb);
void rtl_get_ct_v6_ip_by_dir(void *ct_ptr, enum ip_conntrack_dir dir, int flag, struct in6_addr *addr);

int rtl_skb_ipv6_dst_check(struct sk_buff *skb);
int rtl_skb_is_gso(struct sk_buff *skb);
int rtl_skb_to_dut(struct sk_buff *skb);
u32 rtl_dst_allfrag(struct sk_buff *skb);
struct hh_cache *rtl_get_hh_from_skb(struct sk_buff *skb);
struct neighbour *rtl_get_neigh_from_skb(struct sk_buff *skb);
int rtl_check_skb_dst_output_exist(struct sk_buff *skb);
int rtl_neigh_hh_output(struct sk_buff *skb);
int rtl_neigh_output(struct sk_buff *skb);
int rtl_dst_output(struct sk_buff *skb);
int rtl_ip6_route_input(struct sk_buff *skb);
int rtl_ipt6_do_table(struct sk_buff * skb, unsigned int hook, void *in, void *out);
int rtl_check_dst_input(struct sk_buff * skb);
void rtl_skb_dst_set(struct sk_buff *skb, struct dst_entry *dst);
#endif
#endif

int rtl_is_wan_dev(struct sk_buff *skb);


#define RTL_FSTPATH_TTL_ADJUST 1
void get_fastpath_module_info(unsigned char *buf);

//#define RTL_FASTPATH_FRAGMENT_SUPPORT 1
#if defined(RTL_FASTPATH_FRAGMENT_SUPPORT)
int fast_pppoe_xmit_check(struct sk_buff *skb);
int fast_pppoe_xmit2(struct sk_buff *skb);
int fast_l2tp_to_wan_check(void *skb);
int fast_l2tp_to_wan2(void *skb);
#endif

#endif	/* __FASTPATH_CORE_H__ */

