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

/*
 * PPE flow flag definitions
 */
#define PPE_DRV_FLOW_VALID	0x0001
#define PPE_DRV_FLOW_SNAT	0x0002
#define PPE_DRV_FLOW_DNAT	0x0004
#define PPE_DRV_FLOW_BRIDGED	0x0008
#define PPE_DRV_FLOW_V4		0x0010
#define PPE_DRV_FLOW_V6		0x0020
#define PPE_DRV_FLOW_EIP	0x0040

struct ppe_drv_v4_conn_flow;
struct ppe_drv_v6_conn_flow;

/*
 * ppe_drv_flow_pmtu_type
 *	Types of PMTU checks handled
 */
enum ppe_drv_flow_pmtu_type {
        PPE_DRV_FLOW_PMTU_TYPE_L2,	/* L2 header is included in packet length for flow MTU */
        PPE_DRV_FLOW_PMTU_TYPE_L3	/* L3 packet length is used for flow MTU */
};

/*
 * ppe_drv_flow
 *	Flow information
 */
struct ppe_drv_flow {
	uint64_t bytes;			/* Byte counter from PPE */
	uint32_t pkts;			/* Packet counter from PPE */
	union {
		struct ppe_drv_v4_conn_flow *v4;
		struct ppe_drv_v6_conn_flow *v6;
	} pcf;				/* Corresponding connection flow entry */
	struct ppe_drv_nexthop *nh;	/* Index into the nexthop table */
	struct ppe_drv_host *host;	/* Pointer to associated host entry */
	enum ppe_drv_ip_type type;	/* Indicates if flow is of type v4 or v6 */
	struct ppe_drv_host *host_mc;	/* Pointer to associated host multicast entry */
	uint16_t index;			/* Index into the flow table */
	uint16_t flags;			/* Connection flags */
	uint8_t entry_type;		/* Flow type - 5 tuple or 3 tuple */
	uint8_t service_code;		/* Service code used by this entry */
};

bool ppe_drv_flow_valid_set(struct ppe_drv_flow *pf, bool enable);
void ppe_drv_flow_dump(struct ppe_drv_flow *pf);
void ppe_drv_flow_stats_clear(struct ppe_drv_flow *pf);

bool ppe_drv_flow_v4_qos_clear(struct ppe_drv_flow *pf);
bool ppe_drv_flow_v4_qos_set(struct ppe_drv_v4_conn_flow *pcf, struct ppe_drv_flow *pf);

bool ppe_drv_flow_v6_qos_clear(struct ppe_drv_flow *pf);
bool ppe_drv_flow_v6_qos_set(struct ppe_drv_v6_conn_flow *pcf, struct ppe_drv_flow *pf);

bool ppe_drv_flow_del(struct ppe_drv_flow *pf);
struct ppe_drv_flow *ppe_drv_flow_v4_get(struct ppe_drv_v4_5tuple *tuple);
struct ppe_drv_flow *ppe_drv_flow_v4_add(struct ppe_drv_v4_conn_flow *pcf,
		struct ppe_drv_nexthop *nh, struct ppe_drv_host *host, bool entry_valid);

struct ppe_drv_flow *ppe_drv_flow_v6_get(struct ppe_drv_v6_5tuple *tuple);
struct ppe_drv_flow *ppe_drv_flow_v6_add(struct ppe_drv_v6_conn_flow *pcf,
		struct ppe_drv_nexthop *nh, struct ppe_drv_host *host, bool entry_valid);

void ppe_drv_flow_entries_free(struct ppe_drv_flow *flow);
struct ppe_drv_flow *ppe_drv_flow_entries_alloc(void);
