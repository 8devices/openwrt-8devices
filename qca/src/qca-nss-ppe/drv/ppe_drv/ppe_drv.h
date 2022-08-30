/*
 * Copyright (c) 2022 Qualcomm Innovation Center, Inc. All rights reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <linux/module.h>
#include <ppe_drv_public.h>
#include "ppe_drv_exception.h"
#include "ppe_drv_flow.h"
#include "ppe_drv_host.h"
#include "ppe_drv_iface.h"
#include "ppe_drv_l3_if.h"
#include "ppe_drv_nexthop.h"
#include "ppe_drv_port.h"
#include "ppe_drv_pppoe.h"
#include "ppe_drv_pub_ip.h"
#include "ppe_drv_sc.h"
#include "ppe_drv_stats.h"
#include "ppe_drv_vsi.h"
#include "ppe_drv_v4.h"
#include "ppe_drv_v6.h"

/*
 * PPE debug macros
 */
#if (PPE_DRV_DEBUG_LEVEL == 3)
#define ppe_drv_assert(c, s, ...)
#else
#define ppe_drv_assert(c, s, ...) if (!(c)) { printk(KERN_CRIT "%s[%d]:" s, __FUNCTION__, __LINE__, ##__VA_ARGS__); BUG_ON(!(c)); }
#endif

#if defined(CONFIG_DYNAMIC_DEBUG)
/*
 * If dynamic debug is enabled, use pr_debug.
 */
#define ppe_drv_warn(s, ...) pr_debug("%s[%d]:" s, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#define ppe_drv_info(s, ...) pr_debug("%s[%d]:" s, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#define ppe_drv_trace(s, ...) pr_debug("%s[%d]:" s, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#else

/*
 * Statically compile messages at different levels, when dynamic debug is disabled.
 */
#if (PPE_DRV_DEBUG_LEVEL < 2)
#define ppe_drv_warn(s, ...)
#else
#define ppe_drv_warn(s, ...) pr_warn("%s[%d]:" s, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#endif

#if (PPE_DRV_DEBUG_LEVEL < 3)
#define ppe_drv_info(s, ...)
#else
#define ppe_drv_info(s, ...) pr_notice("%s[%d]:" s, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#endif

#if (PPE_DRV_DEBUG_LEVEL < 4)
#define ppe_drv_trace(s, ...)
#else
#define ppe_drv_trace(s, ...) pr_info("%s[%d]:" s, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#endif
#endif

/*
 * Default switch ID
 */
#define PPE_DRV_SWITCH_ID		0

/*
 * PPE Hash seed and mask
 *
 * Note: we don't initialize the seed value with a random value
 * to keep the hash calculation persistent across reboots.
 */
#define PPE_DRV_HASH_SEED_DEFAULT	0xabbcdefa
#define PPE_DRV_HASH_MASK		0xfff
#define PPE_DRV_HASH_MIX_V4_SIP		0x13
#define PPE_DRV_HASH_MIX_V4_DIP		0xb
#define PPE_DRV_HASH_MIX_V4_PROTO	0x13
#define PPE_DRV_HASH_MIX_V4_DPORT	0xb
#define PPE_DRV_HASH_MIX_V4_SPORT	0x13
#define PPE_DRV_HASH_FIN_MASK		0x1f
#define PPE_DRV_HASH_FIN_INNER_OUTER_0		0x205
#define PPE_DRV_HASH_FIN_INNER_OUTER_1		0x264
#define PPE_DRV_HASH_FIN_INNER_OUTER_2		0x227
#define PPE_DRV_HASH_FIN_INNER_OUTER_3		0x245
#define PPE_DRV_HASH_FIN_INNER_OUTER_4		0x201

#define PPE_DRV_HASH_SIPV6_MIX_0		0x13
#define PPE_DRV_HASH_SIPV6_MIX_1		0xb
#define PPE_DRV_HASH_SIPV6_MIX_2		0x13
#define PPE_DRV_HASH_SIPV6_MIX_3		0xb
#define PPE_DRV_HASH_DIPV6_MIX_0		0x13
#define PPE_DRV_HASH_DIPV6_MIX_1		0xb
#define PPE_DRV_HASH_DIPV6_MIX_2		0x13
#define PPE_DRV_HASH_DIPV6_MIX_3		0xb

#define PPE_DRV_IFACE_MAX 256
#define PPE_DRV_JUMBO_MAX 9216

/*
 * VLAN macros
 */
#define PPE_DRV_VLAN_NOT_CONFIGURED	0xFFF
#define PPE_DRV_VLAN_ID_MASK		0xFFF
#define PPE_DRV_VLAN_TPID_MASK		0xFFFF0000
#define PPE_DRV_VLAN_TCI_MASK		0xFFFF
#define PPE_DRV_VLAN_PRIORITY_MASK	0xE000
#define PPE_DRV_VLAN_PRIORITY_SHIFT	13

/*
 * ppe_drv_entry_valid
 *	PPE entry validity
 */
enum ppe_drv_entry_valid {
	PPE_DRV_ENTRY_INVALID,	/* Entry invalid. */
	PPE_DRV_ENTRY_VALID,	/* Entry valid. */
};

/*
 * ppe_drv
 *	PPE DRV base structure
 */
struct ppe_drv {
	spinlock_t lock;				/* PPE lock */
	spinlock_t stats_lock;				/* PPE statistics lock */

	uint32_t iface_num;				/* Number of PPE interface */
	uint32_t l3_if_num;				/* Number of entries in PPE L3_IF table */
	uint32_t port_num;				/* Number of entries in PPE Port table */
	uint32_t vsi_num;				/* Number of entries in PPE VSI table */
	uint32_t pub_ip_num;				/* Number of entries in PPE Public IP table */
	uint32_t host_num;				/* Number of entries in PPE Host table */
	uint32_t flow_num;				/* Number of entries in PPE Flow table */
	uint32_t pppoe_session_max;			/* Number of entries in PPE PPPoe Session table */
	uint32_t nexthop_num;				/* Number of entries in PPE Nexthop table */
	uint32_t sc_num;				/* Number of entries in PPE Service Code table */
	uint32_t queue_num;				/* Number of entries in PPE Service Code table */

	/*
	 * Timer
	 */
	struct timer_list hw_flow_stats_timer;      	/* Timer used to poll for stats from PPE_HW */
	struct work_struct sw_v4_stats;			/* Workqueue used to sync v4 stats from PPE driver to ECM */
	struct work_struct sw_v6_stats;       		/* Workqueue used to sync v6 stats from PPE driver to ECM */
	struct ppe_drv_stats stats;			/* PPE statistics */

	/*
	 * Pointer to memory pool for different PPE tables
	 */
	struct ppe_drv_iface *iface;			/* Memory for PPE interface shadow table */
	struct ppe_drv_flow *flow;			/* Memory for PPE Flow table */
	struct ppe_drv_host *host;			/* Memory for PPE Host table */
	struct ppe_drv_nexthop *nexthop;		/* Memory for PPE nexthop table */
	struct ppe_drv_pub_ip *pub_ip;			/* Memory for PPE Public IP table */
	struct ppe_drv_vsi *vsi;			/* Memory for PPE VSI shadow table */
	struct ppe_drv_port *port;			/* Memory for PPE Port table */
	struct ppe_drv_l3_if *l3_if;			/* Memory for PPE L3_IF shadow table */
	struct ppe_drv_pppoe *pppoe;			/* Memory for PPE PPPoe table */
	struct ppe_drv_queue *queue;			/* Memory for PPE queue table */
	struct ppe_drv_sc *sc;				/* Memory for PPE Service Code table */

	struct dentry *dentry;				/* Debugfs entry */

	struct list_head nh_active;			/* List of active nexthops */
	struct list_head nh_free;			/* List of free nexthops */
	struct kref ref;				/* Reference count */

	/*
	 * v4 and v6 connection list
	 */
	struct list_head conn_v4;               /* List of v4 connection in PPE */
	struct list_head conn_v6;               /* List of v6 connection in PPE */
	bool toggled;				/* Toggled bit for sync during a particular iteration */
};

extern struct ppe_drv ppe_drv_gbl;
