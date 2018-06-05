#ifndef RTL865X_MULTICAST_H
#define RTL865X_MULTICAST_H

#define RTL865X_NETWORK_INTERFACE_DATA_STRUCTURE

#define RTL865X_MULTICAST_TABLE_AGE				35
#define RTL865X_MULTICAST_TABLE_ASIC_FULL_AGE	35
#define RTL865X_MULTICAST_TABLE_ASIC_AGE			10	/* we would always write this value into ASIC */
#define RTL865X_MULTICAST_SWAP_THRESHOLD		400
#define SINGLE_BITMASK_ISSET(x) ((x & (x - 1)) == 0)	/* chenyl: only 1 bit is set in vlanMask */

#define RTL865X_HW_MCAST_SWAP_GAP  300

/*================================================
  * Multicast Data Structure
  *================================================*/
#define TBLDRV_MULTICAST_DATA_STRUCTURE

/* mcast entry flag */
#define 	RTL865X_MULTICAST_PPPOEPATCH_CPUBIT		0x01
#define 	RTL865X_MULTICAST_EXTIP_SET				0x02
#define 	RTL865X_MULTICAST_UPLOADONLY				0x04

//#define 	MAX_MCAST_FWD_DESCRIPTOR_CNT			256
#define 	MAX_MCAST_FWD_DESCRIPTOR_CNT			(RTL8651_MULTICASTTBL_SIZE<<2)
//#define	    MAX_MCAST_TABLE_ENTRY_CNT				128
#define	    MAX_MCAST_TABLE_ENTRY_CNT			(RTL8651_MULTICASTTBL_SIZE<<1)
#if defined(CONFIG_RTL_8198C) || defined(CONFIG_RTL_8197F)
//#define 	MAX_MCASTV6_FWD_DESCRIPTOR_CNT			256
//#define     MAX_MCASTV6_TABLE_ENTRY_CNT				256
#define 	MAX_MCASTV6_FWD_DESCRIPTOR_CNT			(RTL8651_MULTICASTTBL_SIZE<<2)
#define     MAX_MCASTV6_TABLE_ENTRY_CNT				(RTL8651_MULTICASTTBL_SIZE<<1)
#endif

/*
 * List definitions.
 */
#define MC_LIST_HEAD(name, type)						\
struct name {								\
	struct type *lh_first;	/* first element */			\
}

#define MC_LIST_HEAD_INITIALIZER(head)					\
	{ NULL }

#define MC_LIST_ENTRY(type)						\
struct {								\
	struct type *le_next;	/* next element */			\
	struct type **le_prev;	/* address of previous next element */	\
}

/*
 * List functions.
 */

#define MC_LIST_EMPTY(head) ((head)->lh_first == NULL)

#define MC_LIST_FIRST(head)	((head)->lh_first)

#define MC_LIST_FOREACH(var, head, field)					\
	for((var) = (head)->lh_first; (var); (var) = (var)->field.le_next)

#define MC_LIST_INIT(head) do {						\
	(head)->lh_first = NULL;					\
} while (0)

#define MC_LIST_INSERT_AFTER(listelm, elm, field) do {			\
	if (((elm)->field.le_next = (listelm)->field.le_next) != NULL)	\
		(listelm)->field.le_next->field.le_prev =		\
		    &(elm)->field.le_next;				\
	(listelm)->field.le_next = (elm);				\
	(elm)->field.le_prev = &(listelm)->field.le_next;		\
} while (0)

#define MC_LIST_INSERT_BEFORE(listelm, elm, field) do {			\
	(elm)->field.le_prev = (listelm)->field.le_prev;		\
	(elm)->field.le_next = (listelm);				\
	*(listelm)->field.le_prev = (elm);				\
	(listelm)->field.le_prev = &(elm)->field.le_next;		\
} while (0)

#define MC_LIST_INSERT_HEAD(head, elm, field) do {				\
	if (((elm)->field.le_next = (head)->lh_first) != NULL)		\
		(head)->lh_first->field.le_prev = &(elm)->field.le_next;\
	(head)->lh_first = (elm);					\
	(elm)->field.le_prev = &(head)->lh_first;			\
} while (0)

#define MC_LIST_NEXT(elm, field)	((elm)->field.le_next)

#define MC_LIST_REMOVE(elm, field) do {					\
	if ((elm)->field.le_next != NULL)				\
		(elm)->field.le_next->field.le_prev = 			\
		    (elm)->field.le_prev;				\
	if ((elm)->field.le_prev != NULL)	\
		*(elm)->field.le_prev = (elm)->field.le_next;			\
} while (0)




/*
 * Counting Tail queue definitions.
 */
#define CTAILQ_HEAD(name, type)						\
struct name {\
	struct type *tqh_first;	/* first element */			\
	struct type **tqh_last;	/* addr of last next element */		\
	int tqh_count;\
}

#define CTAILQ_HEAD_INITIALIZER(head)					\
	{ 0, NULL, &(head).tqh_first }

#define CTAILQ_ENTRY(type)						\
struct {								\
	struct type *tqe_next;	/* next element */			\
	struct type **tqe_prev;	/* address of previous next element */	\
}

/*
 * Counting Tail queue functions.
 */
#define	CTAILQ_EMPTY(head) ((head)->tqh_first == NULL)

#define CTAILQ_FOREACH(var, head, field)					\
	for (var = TAILQ_FIRST(head); var; var = TAILQ_NEXT(var, field))

#define CTAILQ_FOREACH_REVERSE(var, head, headname, field)		\
	for ((var) = TAILQ_LAST((head), headname);			\
	     (var);							\
	     (var) = TAILQ_PREV((var), headname, field))

#define	CTAILQ_FIRST(head) ((head)->tqh_first)
#define	CTAILQ_TOTAL(head) ((head)->tqh_count)
#define	CTAILQ_LAST(head, headname) \
	(*(((struct headname *)((head)->tqh_last))->tqh_last))

#define	CTAILQ_NEXT(elm, field) ((elm)->field.tqe_next)

#define CTAILQ_PREV(elm, headname, field) \
	(*(((struct headname *)((elm)->field.tqe_prev))->tqh_last))

#define	CTAILQ_INIT(head) do {						\
	(head)->tqh_count=0;\
	(head)->tqh_first = NULL;					\
	(head)->tqh_last = &(head)->tqh_first;				\
} while (0)

#define CTAILQ_INSERT_HEAD(head, elm, field) do {			\
	if (((elm)->field.tqe_next = (head)->tqh_first) != NULL)	\
		(head)->tqh_first->field.tqe_prev =			\
		    &(elm)->field.tqe_next;				\
	else								\
		(head)->tqh_last = &(elm)->field.tqe_next;		\
	(head)->tqh_first = (elm);					\
	(elm)->field.tqe_prev = &(head)->tqh_first;			\
	(head)->tqh_count++;\
} while (0)

#define CTAILQ_INSERT_TAIL(head, elm, field) do {			\
	(elm)->field.tqe_next = NULL;					\
	(elm)->field.tqe_prev = (head)->tqh_last;			\
	*(head)->tqh_last = (elm);					\
	(head)->tqh_last = &(elm)->field.tqe_next;			\
	(head)->tqh_count++;\
} while (0)

#define CTAILQ_INSERT_AFTER(head, listelm, elm, field) do {		\
	if (((elm)->field.tqe_next = (listelm)->field.tqe_next) != NULL)\
		(elm)->field.tqe_next->field.tqe_prev = 		\
		    &(elm)->field.tqe_next;				\
	else								\
		(head)->tqh_last = &(elm)->field.tqe_next;		\
	(listelm)->field.tqe_next = (elm);				\
	(elm)->field.tqe_prev = &(listelm)->field.tqe_next;		\
	(head)->tqh_count++;	\
} while (0)

#define CTAILQ_INSERT_BEFORE(head, listelm, elm, field) do {			\
	(elm)->field.tqe_prev = (listelm)->field.tqe_prev;		\
	(elm)->field.tqe_next = (listelm);				\
	*(listelm)->field.tqe_prev = (elm);				\
	(listelm)->field.tqe_prev = &(elm)->field.tqe_next;		\
	(head)->tqh_count++;\
} while (0)

#define CTAILQ_REMOVE(head, elm, field) do {				\
	if (((elm)->field.tqe_next) != NULL)				\
		(elm)->field.tqe_next->field.tqe_prev = 		\
		    (elm)->field.tqe_prev;				\
	else								\
		(head)->tqh_last = (elm)->field.tqe_prev;		\
	*(elm)->field.tqe_prev = (elm)->field.tqe_next;			\
	(head)->tqh_count--;\
} while (0)

#define ETHERNET_DEVICE_TYPE 0
#define WIRELESS_DEVICE_TYPE 1
#define VIRTUAL_DEVICE_TYPE  2
typedef struct rtl865x_mcast_fwd_descriptor_s{
	char netifName[16];
	unsigned int descPortMask;
	char toCpu;
	unsigned int vid;
	unsigned int fwdPortMask;
#if defined(CONFIG_BRIDGE_IGMP_SNOOPING)
	unsigned int dip;
#endif
	MC_LIST_ENTRY(rtl865x_mcast_fwd_descriptor_s)	next;
	
}rtl865x_mcast_fwd_descriptor_t;

typedef MC_LIST_HEAD(mcast_fwd_descriptor_head_s, rtl865x_mcast_fwd_descriptor_s)  mcast_fwd_descriptor_head_t;

/*
 * Tail queue definitions.
 */
#define TAILQ_HEAD(name, type)						\
struct name {								\
	struct type *tqh_first;	/* first element */			\
	struct type **tqh_last;	/* addr of last next element */		\
}

#define TAILQ_HEAD_INITIALIZER(head)					\
	{ NULL, &(head).tqh_first }

#define TAILQ_ENTRY(type)						\
struct {								\
	struct type *tqe_next;	/* next element */			\
	struct type **tqe_prev;	/* address of previous next element */	\
}

/*
 * Tail queue functions.
 */
#define	TAILQ_EMPTY(head) ((head)->tqh_first == NULL)

#define TAILQ_FOREACH(var, head, field)					\
	for (var = TAILQ_FIRST(head); var; var = TAILQ_NEXT(var, field))

#define TAILQ_FOREACH_REVERSE(var, head, headname, field)		\
	for ((var) = TAILQ_LAST((head), headname);			\
	     (var);							\
	     (var) = TAILQ_PREV((var), headname, field))

#define	TAILQ_FIRST(head) ((head)->tqh_first)

#define	TAILQ_LAST(head, headname) \
	(*(((struct headname *)((head)->tqh_last))->tqh_last))

#define	TAILQ_NEXT(elm, field) ((elm)->field.tqe_next)

#define TAILQ_PREV(elm, headname, field) \
	(*(((struct headname *)((elm)->field.tqe_prev))->tqh_last))

#define	TAILQ_INIT(head) do {						\
	(head)->tqh_first = NULL;					\
	(head)->tqh_last = &(head)->tqh_first;				\
} while (0)

#define TAILQ_INSERT_HEAD(head, elm, field) do {			\
	if (((elm)->field.tqe_next = (head)->tqh_first) != NULL)	\
		(head)->tqh_first->field.tqe_prev =			\
		    &(elm)->field.tqe_next;				\
	else								\
		(head)->tqh_last = &(elm)->field.tqe_next;		\
	(head)->tqh_first = (elm);					\
	(elm)->field.tqe_prev = &(head)->tqh_first;			\
} while (0)

#define TAILQ_INSERT_TAIL(head, elm, field) do {			\
	(elm)->field.tqe_next = NULL;					\
	(elm)->field.tqe_prev = (head)->tqh_last;			\
	*(head)->tqh_last = (elm);					\
	(head)->tqh_last = &(elm)->field.tqe_next;			\
} while (0)

#define TAILQ_INSERT_AFTER(head, listelm, elm, field) do {		\
	if (((elm)->field.tqe_next = (listelm)->field.tqe_next) != NULL)\
		(elm)->field.tqe_next->field.tqe_prev = 		\
		    &(elm)->field.tqe_next;				\
	else								\
		(head)->tqh_last = &(elm)->field.tqe_next;		\
	(listelm)->field.tqe_next = (elm);				\
	(elm)->field.tqe_prev = &(listelm)->field.tqe_next;		\
} while (0)

#define TAILQ_INSERT_BEFORE(listelm, elm, field) do {			\
	(elm)->field.tqe_prev = (listelm)->field.tqe_prev;		\
	(elm)->field.tqe_next = (listelm);				\
	*(listelm)->field.tqe_prev = (elm);				\
	(listelm)->field.tqe_prev = &(elm)->field.tqe_next;		\
} while (0)

#define TAILQ_REMOVE(head, elm, field) do {				\
	if (((elm)->field.tqe_next) != NULL)				\
		(elm)->field.tqe_next->field.tqe_prev = 		\
		    (elm)->field.tqe_prev;				\
	else								\
		(head)->tqh_last = (elm)->field.tqe_prev;		\
	*(elm)->field.tqe_prev = (elm)->field.tqe_next;			\
} while (0)
#ifdef CONFIG_RTL_IGMP_PROXY_MULTIWAN
#define ALL_LAN_PORT 0xFFFFFFFF
#endif
typedef struct rtl865x_tblDrv_mCast_s {
	unsigned int	sip;
	unsigned int	dip;
	unsigned short	svid;
	unsigned short 	port;
	unsigned int	mbr;
	unsigned short	age;
	unsigned short	cpu;
	unsigned int extIp;
	/*above field is for asic table usage*/
	mcast_fwd_descriptor_head_t fwdDescChain;
	unsigned int	count;
	//unsigned int	maxPPS;
	unsigned char	cpuHold;
	unsigned char	flag;
	unsigned char	inAsic;	
	unsigned char	unKnownMCast;
#if  defined(CONFIG_RTL8196C_REVISION_B) || defined (CONFIG_RTL8198_REVISION_B) || defined(CONFIG_RTL_819XD) || defined(CONFIG_RTL_8196E)
	unsigned short liveTime;
#endif
//#if defined(CONFIG_RTL_819XD) || defined(CONFIG_RTL_8196E) 
	unsigned int hashIndex;
//#endif
#if defined(CONFIG_RTL_MULTICAST_PORT_MAPPING)
	unsigned int mapPortMbr;
#endif
	TAILQ_ENTRY(rtl865x_tblDrv_mCast_s) nextMCast;
	
} rtl865x_tblDrv_mCast_t;
TAILQ_HEAD(MCast_hash_head, rtl865x_tblDrv_mCast_s);
struct rtl865x_multicastTable{
	
	struct freeList_s {
		
		TAILQ_HEAD( _FreeMultiCastEntry, rtl865x_tblDrv_mCast_s) freeMultiCast;
	} freeList;

	struct inuseList_s {
		TAILQ_HEAD( _InuseMCast, rtl865x_tblDrv_mCast_s) *mCastTbl;
	}inuseList;

};

typedef struct rtl865x_mCastConfig_s{
	unsigned int externalPortMask;
	
}rtl865x_mCastConfig_t;

int rtl865x_initMulticast(rtl865x_mCastConfig_t * mCastConfig);
int rtl865x_reinitMulticast(void);	

int rtl865x_addMulticastExternalPort(unsigned int extPort);
int rtl865x_delMulticastExternalPort(unsigned int extPort);

int rtl865x_setMulticastExternalPortMask(unsigned int extPortMask);
int rtl865x_getMulticastExternalPortMask(void);

int rtl865x_addMulticastExternalPortMask(unsigned int extPortMask);
int rtl865x_delMulticastExternalPortMask(unsigned int extPortMask);

rtl865x_tblDrv_mCast_t *rtl865x_findMCastEntry(unsigned int mAddr, unsigned int sip, unsigned short svid, unsigned short sport);
#if defined CONFIG_RTL_MULTICAST_PORT_MAPPING
int rtl865x_addMulticastEntry(unsigned int mAddr, unsigned int sip, unsigned short svid, unsigned short sport, 
									rtl865x_mcast_fwd_descriptor_t * newFwdDescChain, 
									int flushOldChain, unsigned int extIp, char cpuHold, unsigned char flag, unsigned int mapPortMask);
#else
int rtl865x_addMulticastEntry(unsigned int mAddr, unsigned int sip, unsigned short svid, unsigned short sport, 
									rtl865x_mcast_fwd_descriptor_t * newFwdDescChain, 
									int flushOldChain, unsigned int extIp, char cpuHold, unsigned char flag);
#endif
int rtl865x_delMulticastEntry(unsigned int mcast_addr);

int rtl865x_genVirtualMCastFwdDescriptor(unsigned int forceToCpu, unsigned int  fwdPortMask, rtl865x_mcast_fwd_descriptor_t *fwdDescriptor);
#if defined CONFIG_RTL_MULTICAST_PORT_MAPPING
int rtl865x_blockMulticastFlow(unsigned int srcVlanId, unsigned int srcPort,unsigned int srcIpAddr, unsigned int destIpAddr, unsigned int mapPortMask);
#else
int rtl865x_blockMulticastFlow(unsigned int srcVlanId, unsigned int srcPort,unsigned int srcIpAddr, unsigned int destIpAddr);
#endif
int32 rtl865x_flushHWMulticastEntry(void);
int rtl865x_getMCastHashMethod(unsigned int *hashMethod);
int rtl865x_setMCastHashMethod(unsigned int hashMethod);

#if defined(CONFIG_RTL_8198C) || defined(CONFIG_RTL_8197F)
typedef MC_LIST_HEAD(mcast_fwd_descriptor_head6_s, rtl8198c_mcast_fwd_descriptor6_s)  mcast_fwd_descriptor_head6_t;
typedef struct rtl8198c_mcast_fwd_descriptor6_s{
	char netifName[16];
	unsigned int descPortMask;
	char toCpu;
	unsigned int vid;
	unsigned int fwdPortMask;
#if defined(CONFIG_BRIDGE_IGMP_SNOOPING)
	unsigned int swPortMask;
	unsigned int dip;
#endif
	MC_LIST_ENTRY(rtl8198c_mcast_fwd_descriptor6_s)	next;
	
}rtl8198c_mcast_fwd_descriptor6_t;
typedef struct rtl8198c_tblDrv_mCastv6_s {
	inv6_addr_t sip;
	inv6_addr_t dip;
	uint16 	port;
	uint32	mbr;
	uint16  age;
	uint16  cpu;
	uint16	six_rd_eg;
	uint16	six_rd_idx;
	/*above field is for asic table usage*/
	unsigned short	svid;
	mcast_fwd_descriptor_head6_t fwdDescChain;
	unsigned int	count;
	unsigned char	cpuHold;
	unsigned char	flag;
	unsigned char	inAsic;	
	unsigned char	unKnownMCast;
#if  defined(CONFIG_RTL8196C_REVISION_B) || defined (CONFIG_RTL8198_REVISION_B) || defined(CONFIG_RTL_819XD) || defined(CONFIG_RTL_8196E)
	unsigned short liveTime;
#endif 

//#if defined(CONFIG_RTL_819XD) || defined(CONFIG_RTL_8196E)
	unsigned int hashIndex;
//#endif

#if defined CONFIG_RTL_MULTICAST_PORT_MAPPING
	unsigned int mapPortMbr;
#endif

	TAILQ_ENTRY(rtl8198c_tblDrv_mCastv6_s) nextMCast;	
} rtl8198c_tblDrv_mCastv6_t;
TAILQ_HEAD( MCast6_hash_head, rtl8198c_tblDrv_mCastv6_s);
struct rtl8198c_multicastv6Table{
	struct freeList6_s {
		
		TAILQ_HEAD( _FreeMultiCastEntry6, rtl8198c_tblDrv_mCastv6_s) freeMultiCast;
	} freeList;

	struct inuseList6_s {
		TAILQ_HEAD( _InuseMCast6, rtl8198c_tblDrv_mCastv6_s) *mCastTbl;
	}inuseList;
};

typedef struct rtl8198c_mCastv6Config_s{
	unsigned int externalPortMask;	
}rtl8198c_mCastv6Config_t;

int rtl8198C_initMulticastv6(void);//rtl8198c_mCastv6Config_t * mCastConfig6);
int rtl8198C_reinitMulticastv6(void);	
int rtl8198C_addMulticastv6ExternalPort(unsigned int extPort);
int rtl8198C_delMulticastv6ExternalPort(unsigned int extPort);
int rtl8198C_setMulticastv6ExternalPortMask(unsigned int extPortMask);
int rtl8198C_getMulticastv6ExternalPortMask(void);
int rtl8198C_addMulticastv6ExternalPortMask(unsigned int extPortMask);
int rtl8198C_delMulticastv6ExternalPortMask(unsigned int extPortMask);
rtl8198c_tblDrv_mCastv6_t *rtl8198C_findMCastv6Entry(inv6_addr_t dip,inv6_addr_t sip, unsigned short svid, unsigned short sport);
#if defined CONFIG_RTL_MULTICAST_PORT_MAPPING
int rtl8198C_addMulticastv6Entry(inv6_addr_t dip,inv6_addr_t sip, unsigned short svid, unsigned short sport, 
									rtl8198c_mcast_fwd_descriptor6_t * newFwdDescChain, 
									int flushOldChain, unsigned int extIp, char cpuHold, unsigned char flag, unsigned int mapPortMask);
#else
int rtl8198C_addMulticastv6Entry(inv6_addr_t dip,inv6_addr_t sip, unsigned short svid, unsigned short sport, 
									rtl8198c_mcast_fwd_descriptor6_t * newFwdDescChain, 
									int flushOldChain, unsigned int extIp, char cpuHold, unsigned char flag);
#endif
int rtl8198C_delMulticastv6Entry(inv6_addr_t groupAddr);
int rtl8198C_genVirtualMCastv6FwdDescriptor(unsigned int forceToCpu, unsigned int  fwdPortMask, rtl8198c_mcast_fwd_descriptor6_t *fwdDescriptor);
#if defined CONFIG_RTL_MULTICAST_PORT_MAPPING
int rtl8198C_blockMulticastv6Flow(unsigned int srcVlanId, unsigned int srcPort,inv6_addr_t srcIpAddr,inv6_addr_t destIpAddr, unsigned int mapPortMask);
#else
int rtl8198C_blockMulticastv6Flow(unsigned int srcVlanId, unsigned int srcPort,inv6_addr_t srcIpAddr,inv6_addr_t destIpAddr);
#endif
int32 rtl8198C_flushHWMulticastv6Entry(void);
int rtl8198C_getMCastv6HashMethod(unsigned int *hashMethod);
int rtl8198C_setMCastv6HashMethod(unsigned int hashMethod);
#endif
#if defined (CONFIG_RTL_HW_MCAST_PATCH_FOR_MAC)
#define MACCLONE_MODE_HW_REPLACE 1
#define	MACCLONE_MODE_SW_REPLACE 2
#define MACCLONE_MODE_HW_DISBLE 3
#define	MACCLONE_MODE_ORI 0
#endif

#endif

