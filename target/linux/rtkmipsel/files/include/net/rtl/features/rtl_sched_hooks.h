#ifndef	RTL_SCHED_HOOKS_H
#define	RTL_SCHED_HOOKS_H		1
#include <net/pkt_sched.h>
#include <net/sch_generic.h>
#include <net/act_api.h>
#include <net/pkt_cls.h>
#include <linux/netfilter/x_tables.h>

#if	defined(CONFIG_RTL_HW_QOS_SUPPORT)
#if 0
enum nf_inet_hooks {
	NF_INET_PRE_ROUTING,
	NF_INET_LOCAL_IN,
	NF_INET_FORWARD,
	NF_INET_LOCAL_OUT,
	NF_INET_POST_ROUTING,
	NF_INET_NUMHOOKS
};
#endif
/* IP Hooks */
/* After promisc drops, checksum checks. */
#define NF_IP_PRE_ROUTING	0
/* If the packet is destined for this box. */
#define NF_IP_LOCAL_IN		1
/* If the packet is destined for another interface. */
#define NF_IP_FORWARD		2
/* Packets coming from a local process. */
#define NF_IP_LOCAL_OUT		3
/* Packets about to hit the wire. */
#define NF_IP_POST_ROUTING	4
#define NF_IP_NUMHOOKS		5
#endif

#define HTSIZE (PAGE_SIZE/sizeof(struct fw_filter *))
struct fw_filter {
	struct fw_filter	*next;
	u32			id;
	struct tcf_result	res;
#ifdef CONFIG_NET_CLS_IND
	char			indev[IFNAMSIZ];
#endif /* CONFIG_NET_CLS_IND */
	struct tcf_exts		exts;
};

struct fw_head {
	struct fw_filter *ht[HTSIZE];
	u32 mask;
};


 
 struct rtl_class_para
 {
	 struct Qdisc_class_common common;
	 unsigned int max_bw;
	 unsigned int min_bw;
	 struct psched_ratecfg rate;
	 struct psched_ratecfg ceil;
	 int prio;
	 int level;
	 int maxlevel;
	 void * parent;
	 void * cl;
	 int parentid;
	 unsigned long	cl_flags;
	 unsigned int type;
	 unsigned int qid;
 };
 /*for htb*/
 /* used internaly to keep status of single class */
 enum htb_cmode {
	 HTB_CANT_SEND, 	 /* class can't send and can't borrow */
	 HTB_MAY_BORROW,	 /* class can't send but may borrow */
	 HTB_CAN_SEND		 /* class can send */
 };
 
 struct htb_class {
	struct Qdisc_class_common common;
	/* general class parameters */
	struct gnet_stats_basic_packed bstats;
	struct gnet_stats_queue qstats;
	struct gnet_stats_rate_est rate_est;
	struct tc_htb_xstats xstats;	/* our special stats */
	int refcnt;		/* usage count of this class */

	/* topology */
	int level;		/* our level (see above) */
	unsigned int children;
	struct htb_class *parent;	/* parent class */

	u32 prio;		/* these two are used only by leaves... */
	int quantum;		/* but stored for parent-to-leaf return */

	union {
		struct htb_class_leaf {
			struct Qdisc *q;
			int deficit[TC_HTB_MAXDEPTH];
			struct list_head drop_list;
		} leaf;
		struct htb_class_inner {
			struct rb_root feed[TC_HTB_NUMPRIO];	/* feed trees */
			struct rb_node *ptr[TC_HTB_NUMPRIO];	/* current class ptr */
			/* When class changes from state 1->2 and disconnects from
			 * parent's feed then we lost ptr value and start from the
			 * first child again. Here we store classid of the
			 * last valid ptr (used when ptr is NULL).
			 */
			u32 last_ptr_id[TC_HTB_NUMPRIO];
		} inner;
	} un;
	struct rb_node node[TC_HTB_NUMPRIO];	/* node for self or feed tree */
	struct rb_node pq_node;	/* node for event queue */
	s64	pq_key;

	int prio_activity;	/* for which prios are we active */
	enum htb_cmode cmode;	/* current mode of the class */

	/* class attached filters */
	struct tcf_proto *filter_list;
	int filter_cnt;

	/* token bucket parameters */
	struct psched_ratecfg rate;
	struct psched_ratecfg ceil;
	s64	buffer, cbuffer;	/* token bucket depth/rate */
	s64	mbuffer;		/* max wait time */
	s64	tokens, ctokens;	/* current number of tokens */
	s64	t_c;			/* checkpoint time */
};
 
 struct htb_sched {
	struct Qdisc_class_hash clhash;
	struct list_head drops[TC_HTB_NUMPRIO];/* active leaves (for drops) */

	/* self list - roots of self generating tree */
	struct rb_root row[TC_HTB_MAXDEPTH][TC_HTB_NUMPRIO];
	int row_mask[TC_HTB_MAXDEPTH];
	struct rb_node *ptr[TC_HTB_MAXDEPTH][TC_HTB_NUMPRIO];
	u32 last_ptr_id[TC_HTB_MAXDEPTH][TC_HTB_NUMPRIO];

	/* self wait list - roots of wait PQs per row */
	struct rb_root wait_pq[TC_HTB_MAXDEPTH];

	/* time of nearest event per level (row) */
	psched_time_t near_ev_cache[TC_HTB_MAXDEPTH];

	int defcls;		/* class where unclassified flows go to */

	/* filters for qdisc itself */
	struct tcf_proto *filter_list;

	int rate2quantum;	/* quant = rate / rate2quantum */
	psched_time_t now;	/* cached dequeue time */
	struct qdisc_watchdog watchdog;

	/* non shaped skbs; let them go directly thru */
	struct sk_buff_head direct_queue;
	int direct_qlen;	/* max qlen of above */

	long direct_pkts;

#define HTB_WARN_TOOMANYEVENTS	0x1
	unsigned int warned;	/* only one warning */
	struct work_struct work;
};
 #if 1

#define	SM_SHIFT	(30 - PSCHED_SHIFT)
#define	ISM_SHIFT	(8 + PSCHED_SHIFT)
 
#define	SM_MASK		((1ULL << SM_SHIFT) - 1)
#define	ISM_MASK	((1ULL << ISM_SHIFT) - 1)

struct internal_sc {
	u64	sm1;	/* scaled slope of the 1st segment */
	u64	ism1;	/* scaled inverse-slope of the 1st segment */
	u64	dx;	/* the x-projection of the 1st segment */
	u64	dy;	/* the y-projection of the 1st segment */
	u64	sm2;	/* scaled slope of the 2nd segment */
	u64	ism2;	/* scaled inverse-slope of the 2nd segment */
};

/* runtime service curve */
struct runtime_sc {
	u64	x;	/* current starting position on x-axis */
	u64	y;	/* current starting position on y-axis */
	u64	sm1;	/* scaled slope of the 1st segment */
	u64	ism1;	/* scaled inverse-slope of the 1st segment */
	u64	dx;	/* the x-projection of the 1st segment */
	u64	dy;	/* the y-projection of the 1st segment */
	u64	sm2;	/* scaled slope of the 2nd segment */
	u64	ism2;	/* scaled inverse-slope of the 2nd segment */
};

enum hfsc_class_flags {
	HFSC_RSC = 0x1,
	HFSC_FSC = 0x2,
	HFSC_USC = 0x4
};

struct hfsc_class {
	struct Qdisc_class_common cl_common;
	unsigned int	refcnt;		/* usage count */

	struct gnet_stats_basic_packed bstats;
	struct gnet_stats_queue qstats;
	struct gnet_stats_rate_est rate_est;
	unsigned int	level;		/* class level in hierarchy */
	struct tcf_proto *filter_list;	/* filter list */
	unsigned int	filter_cnt;	/* filter count */

	struct hfsc_sched *sched;	/* scheduler data */
	struct hfsc_class *cl_parent;	/* parent class */
	struct list_head siblings;	/* sibling classes */
	struct list_head children;	/* child classes */
	struct Qdisc	*qdisc;		/* leaf qdisc */

	struct rb_node el_node;		/* qdisc's eligible tree member */
	struct rb_root vt_tree;		/* active children sorted by cl_vt */
	struct rb_node vt_node;		/* parent's vt_tree member */
	struct rb_root cf_tree;		/* active children sorted by cl_f */
	struct rb_node cf_node;		/* parent's cf_heap member */
	struct list_head dlist;		/* drop list member */

	u64	cl_total;		/* total work in bytes */
	u64	cl_cumul;		/* cumulative work in bytes done by
					   real-time criteria */

	u64	cl_d;			/* deadline*/
	u64	cl_e;			/* eligible time */
	u64	cl_vt;			/* virtual time */
	u64	cl_f;			/* time when this class will fit for
					   link-sharing, max(myf, cfmin) */
	u64	cl_myf;			/* my fit-time (calculated from this
					   class's own upperlimit curve) */
	u64	cl_myfadj;		/* my fit-time adjustment (to cancel
					   history dependence) */
	u64	cl_cfmin;		/* earliest children's fit-time (used
					   with cl_myf to obtain cl_f) */
	u64	cl_cvtmin;		/* minimal virtual time among the
					   children fit for link-sharing
					   (monotonic within a period) */
	u64	cl_vtadj;		/* intra-period cumulative vt
					   adjustment */
	u64	cl_vtoff;		/* inter-period cumulative vt offset */
	u64	cl_cvtmax;		/* max child's vt in the last period */
	u64	cl_cvtoff;		/* cumulative cvtmax of all periods */
	u64	cl_pcvtoff;		/* parent's cvtoff at initialization
					   time */

	struct internal_sc cl_rsc;	/* internal real-time service curve */
	struct internal_sc cl_fsc;	/* internal fair service curve */
	struct internal_sc cl_usc;	/* internal upperlimit service curve */
	struct runtime_sc cl_deadline;	/* deadline curve */
	struct runtime_sc cl_eligible;	/* eligible curve */
	struct runtime_sc cl_virtual;	/* virtual curve */
	struct runtime_sc cl_ulimit;	/* upperlimit curve */

	unsigned long	cl_flags;	/* which curves are valid */
	unsigned long	cl_vtperiod;	/* vt period sequence number */
	unsigned long	cl_parentperiod;/* parent's vt period sequence number*/
	unsigned long	cl_nactive;	/* number of active children */
};

struct hfsc_sched {
	u16	defcls;				/* default class id */
	struct hfsc_class root;			/* root class */
	struct Qdisc_class_hash clhash;		/* class hash */
	struct rb_root eligible;		/* eligible tree */
	struct list_head droplist;		/* active leaf class list (for
						   dropping) */
	struct qdisc_watchdog watchdog;		/* watchdog timer */
};

#define	HT_INFINITY	0xffffffffffffffffULL	/* infinite time value */

#endif

#if	defined(CONFIG_RTL_HW_QOS_SUPPORT)

#if defined(__linux__) && defined(__KERNEL__)
static struct timer_list rtlQosTimer;	/* timer for qos sync */
#endif
struct iptables_update_hook
{
	unsigned long last_hook_time;
	struct xt_table * table;
};
static struct iptables_update_hook iptables_hook_info;
unsigned long last_hook_time=0;
#define RTL_DELAY_PROCESS_TIME	2
#endif

#define RTL_HTB_TYPE	0x1 
#define RTL_HFSC_TYPE	0x2 
#define RTL_CBQ_TYPE	0x3
 
#define RTL_TBF_TYPE	0x4 
#define RTL_SFQ_TYPE	0x5
#define RTL_DRR_TYPE	0x6 
#define RTL_DSMARK_TYPE 0x7 


#define	BANDWIDTH_GAP_FOR_PORT		10000
#if defined(CONFIG_RTL_8196C) ||defined(CONFIG_RTL_819XD) ||defined(CONFIG_RTL_8196E) || defined(CONFIG_RTL_8197F)
#if defined CONFIG_RTL_8367_QOS_SUPPORT
#define	FULL_SPEED	1000000000
#else
#define	FULL_SPEED	100000000
#endif
#elif defined(CONFIG_RTL_8198)||defined(CONFIG_RTL_8198C)
#define	FULL_SPEED	1000000000
#else
#error "Please select the correct chip model."
#endif
#if defined(CONFIG_RTL_8198C)
#define RTL_MAX_HWQUEUE_NUM		6
#else
#define RTL_MAX_HWQUEUE_NUM		4
#endif

#if defined (CONFIG_RTL_QOS_SYNC_SUPPORT)
 struct qos_conf
 {
	 struct net_device *dev ;
	 int enable;
	 int valid;
	 unsigned long last_hook_time;
 };
 //to-do: multi wan
#define 	RTL_MAX_QOSCONFIG_DEV_NUM 2
 struct qos_conf qosConf[RTL_MAX_QOSCONFIG_DEV_NUM];
 

#endif

 #endif



