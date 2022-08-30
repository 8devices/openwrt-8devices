/*
 * Copyright (c) 2021 The Linux Foundation. All rights reserved.
 * Copyright (c) 2021-2022 Qualcomm Innovation Center, Inc. All rights reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for
 * any purpose with or without fee is hereby granted, provided that the
 * above copyright notice and this permission notice appear in all
 * copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL
 * WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE
 * AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL
 * DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR
 * PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */

#ifndef _STATS_LIB_H_
#define _STATS_LIB_H_

/* Network Interface name length */
#define IFNAME_LEN                   IFNAMSIZ
#define MAX_RADIO_NUM                3
#define MAX_VAP_NUM                  (17 * MAX_RADIO_NUM)
/* This path is for network interfaces */
#define PATH_SYSNET_DEV              "/sys/class/net/"
#define STATS_IF_MCS_VALID           1
#define STATS_IF_MCS_INVALID         0
#define STATS_IF_MAX_MCS_STRING_LEN  34

#ifdef WLAN_FEATURE_11BE
enum stats_if_ru_index {
	STATS_IF_RU_26_INDEX = 0,
	STATS_IF_RU_52_INDEX,
	STATS_IF_RU_52_26_INDEX,
	STATS_IF_RU_106_INDEX,
	STATS_IF_RU_106_26_INDEX,
	STATS_IF_RU_242_INDEX,
	STATS_IF_RU_484_INDEX,
	STATS_IF_RU_484_242_INDEX,
	STATS_IF_RU_996_INDEX,
	STATS_IF_RU_996_484_INDEX,
	STATS_IF_RU_996_484_242_INDEX,
	STATS_IF_RU_2X996_INDEX,
	STATS_IF_RU_2X996_484_INDEX,
	STATS_IF_RU_3X996_INDEX,
	STATS_IF_RU_3X996_484_INDEX,
	STATS_IF_RU_4X996_INDEX,
	STATS_IF_RU_INDEX_MAX,
};
#else
enum stats_if_ru_index {
	STATS_IF_RU_26_INDEX = 0,
	STATS_IF_RU_52_INDEX,
	STATS_IF_RU_106_INDEX,
	STATS_IF_RU_242_INDEX,
	STATS_IF_RU_484_INDEX,
	STATS_IF_RU_996_INDEX,
	STATS_IF_RU_INDEX_MAX,
};
#endif

enum stats_if_proto_subtype {
	STATS_IF_PROTO_INVALID,
	STATS_IF_PROTO_EAPOL_M1,
	STATS_IF_PROTO_EAPOL_M2,
	STATS_IF_PROTO_EAPOL_M3,
	STATS_IF_PROTO_EAPOL_M4,
	STATS_IF_PROTO_DHCP_DISCOVER,
	STATS_IF_PROTO_DHCP_REQUEST,
	STATS_IF_PROTO_DHCP_OFFER,
	STATS_IF_PROTO_DHCP_ACK,
	STATS_IF_PROTO_DHCP_NACK,
	STATS_IF_PROTO_DHCP_RELEASE,
	STATS_IF_PROTO_DHCP_INFORM,
	STATS_IF_PROTO_DHCP_DECLINE,
	STATS_IF_PROTO_ARP_REQ,
	STATS_IF_PROTO_ARP_RES,
	STATS_IF_PROTO_ICMP_REQ,
	STATS_IF_PROTO_ICMP_RES,
	STATS_IF_PROTO_ICMPV6_REQ,
	STATS_IF_PROTO_ICMPV6_RES,
	STATS_IF_PROTO_ICMPV6_RS,
	STATS_IF_PROTO_ICMPV6_RA,
	STATS_IF_PROTO_ICMPV6_NS,
	STATS_IF_PROTO_ICMPV6_NA,
	STATS_IF_PROTO_IPV4_UDP,
	STATS_IF_PROTO_IPV4_TCP,
	STATS_IF_PROTO_IPV6_UDP,
	STATS_IF_PROTO_IPV6_TCP,
	STATS_IF_PROTO_MGMT_ASSOC,
	STATS_IF_PROTO_MGMT_DISASSOC,
	STATS_IF_PROTO_MGMT_AUTH,
	STATS_IF_PROTO_MGMT_DEAUTH,
	STATS_IF_ROAM_SYNCH,
	STATS_IF_ROAM_COMPLETE,
	STATS_IF_ROAM_EVENTID,
	STATS_IF_PROTO_DNS_QUERY,
	STATS_IF_PROTO_DNS_RES,
	STATS_IF_PROTO_SUBTYPE_MAX
};

struct stats_if_ru_debug {
	char *ru_type;
};

enum stats_if_delay_bucket_index {
	STATS_IF_DELAY_BUCKET_0,
	STATS_IF_DELAY_BUCKET_1,
	STATS_IF_DELAY_BUCKET_2,
	STATS_IF_DELAY_BUCKET_3,
	STATS_IF_DELAY_BUCKET_4,
	STATS_IF_DELAY_BUCKET_5,
	STATS_IF_DELAY_BUCKET_6,
	STATS_IF_DELAY_BUCKET_7,
	STATS_IF_DELAY_BUCKET_8,
	STATS_IF_DELAY_BUCKET_9,
	STATS_IF_DELAY_BUCKET_10,
	STATS_IF_DELAY_BUCKET_11,
	STATS_IF_DELAY_BUCKET_12,
	STATS_IF_DELAY_BUCKET_MAX,
};

struct stats_if_rate_debug {
	char mcs_type[STATS_IF_MAX_MCS_STRING_LEN];
	uint8_t valid;
};

/**
 * struct interface_list: Structure to hold interfaces for Driver Communication
 * @r_count:  Number of Radio Interfaces
 * @v_count:  Number of Vap Interfaces
 * @r_names:  All radio interface names
 * @v_names:  All vap interface names
 */
struct interface_list {
	u_int8_t r_count;
	u_int8_t v_count;
	char *r_names[MAX_RADIO_NUM];
	char *v_names[MAX_VAP_NUM];
};

/* Basic peer data stats holder */
struct basic_peer_data {
	struct basic_peer_data_tx *tx;
	struct basic_peer_data_rx *rx;
	struct basic_peer_data_link *link;
	struct basic_peer_data_rate *rate;
};

/* Basic peer control stats holder */
struct basic_peer_ctrl {
	struct basic_peer_ctrl_tx *tx;
	struct basic_peer_ctrl_rx *rx;
	struct basic_peer_ctrl_link *link;
	struct basic_peer_ctrl_rate *rate;
};

/* Basic vdev data stats holder */
struct basic_vdev_data {
	struct basic_vdev_data_tx *tx;
	struct basic_vdev_data_rx *rx;
};

/* Basic vdev control stats holder */
struct basic_vdev_ctrl {
	struct basic_vdev_ctrl_tx *tx;
	struct basic_vdev_ctrl_rx *rx;
};

/* Basic pdev data stats holder */
struct basic_pdev_data {
	struct basic_pdev_data_tx *tx;
	struct basic_pdev_data_rx *rx;
};

/* Basic pdev control stats holder */
struct basic_pdev_ctrl {
	struct basic_pdev_ctrl_tx *tx;
	struct basic_pdev_ctrl_rx *rx;
	struct basic_pdev_ctrl_link *link;
};

/* Basic psoc data stats holder */
struct basic_psoc_data {
	struct basic_psoc_data_tx *tx;
	struct basic_psoc_data_rx *rx;
};

#if WLAN_ADVANCE_TELEMETRY
/* Advance peer data stats holder */
struct advance_peer_data {
	struct advance_peer_data_tx *tx;
	struct advance_peer_data_rx *rx;
	struct advance_peer_data_fwd *fwd;
	struct advance_peer_data_raw *raw;
	struct advance_peer_data_twt *twt;
	struct advance_peer_data_link *link;
	struct advance_peer_data_rate *rate;
	struct advance_peer_data_nawds *nawds;
	struct advance_peer_data_delay *delay;
	struct advance_peer_data_jitter *jitter;
	struct advance_peer_data_sawfdelay *sawfdelay;
	struct advance_peer_data_sawftx *sawftx;
};

/* Advance peer control stats holder */
struct advance_peer_ctrl {
	struct advance_peer_ctrl_tx *tx;
	struct advance_peer_ctrl_rx *rx;
	struct advance_peer_ctrl_twt *twt;
	struct advance_peer_ctrl_link *link;
	struct advance_peer_ctrl_rate *rate;
};

/* Advance vdev data stats holder */
struct advance_vdev_data {
	struct advance_vdev_data_me *me;
	struct advance_vdev_data_tx *tx;
	struct advance_vdev_data_rx *rx;
	struct advance_vdev_data_raw *raw;
	struct advance_vdev_data_tso *tso;
	struct advance_vdev_data_igmp *igmp;
	struct advance_vdev_data_mesh *mesh;
	struct advance_vdev_data_nawds *nawds;
};

/* Advance vdev control stats holder */
struct advance_vdev_ctrl {
	struct advance_vdev_ctrl_tx *tx;
	struct advance_vdev_ctrl_rx *rx;
};

/* Advance pdev data stats holder */
struct advance_pdev_data {
	struct advance_pdev_data_me *me;
	struct advance_pdev_data_tx *tx;
	struct advance_pdev_data_rx *rx;
	struct advance_pdev_data_raw *raw;
	struct advance_pdev_data_tso *tso;
	struct advance_pdev_data_igmp *igmp;
	struct advance_pdev_data_mesh *mesh;
	struct advance_pdev_data_nawds *nawds;
};

/* Advance pdev control stats holder */
struct advance_pdev_ctrl {
	struct advance_pdev_ctrl_tx *tx;
	struct advance_pdev_ctrl_rx *rx;
	struct advance_pdev_ctrl_link *link;
};

/* Advance psoc data stats holder */
struct advance_psoc_data {
	struct advance_psoc_data_tx *tx;
	struct advance_psoc_data_rx *rx;
};
#endif /* WLAN_ADVANCE_TELEMETRY */

#if WLAN_DEBUG_TELEMETRY
/* Debug peer data stats holder */
struct debug_peer_data {
	struct debug_peer_data_tx *tx;
	struct debug_peer_data_rx *rx;
	struct debug_peer_data_link *link;
	struct debug_peer_data_rate *rate;
	struct debug_peer_data_txcap *txcap;
};

/* Debug peer control stats holder */
struct debug_peer_ctrl {
	struct debug_peer_ctrl_tx *tx;
	struct debug_peer_ctrl_rx *rx;
	struct debug_peer_ctrl_link *link;
	struct debug_peer_ctrl_rate *rate;
};

/* Debug vdev data stats holder */
struct debug_vdev_data {
	struct debug_vdev_data_tx *tx;
	struct debug_vdev_data_rx *rx;
	struct debug_vdev_data_me *me;
	struct debug_vdev_data_raw *raw;
	struct debug_vdev_data_tso *tso;
};

/* Debug vdev control stats holder */
struct debug_vdev_ctrl {
	struct debug_vdev_ctrl_tx *tx;
	struct debug_vdev_ctrl_rx *rx;
	struct debug_vdev_ctrl_wmi *wmi;
};

/* Debug pdev data stats holder */
struct debug_pdev_data {
	struct debug_pdev_data_tx *tx;
	struct debug_pdev_data_rx *rx;
	struct debug_pdev_data_me *me;
	struct debug_pdev_data_raw *raw;
	struct debug_pdev_data_tso *tso;
	struct debug_pdev_data_cfr *cfr;
	struct debug_pdev_data_htt *htt;
	struct debug_pdev_data_wdi *wdi;
	struct debug_pdev_data_mesh *mesh;
	struct debug_pdev_data_txcap *txcap;
	struct debug_pdev_data_monitor *monitor;
};

/* Debug pdev control stats holder */
struct debug_pdev_ctrl {
	struct debug_pdev_ctrl_tx *tx;
	struct debug_pdev_ctrl_rx *rx;
	struct debug_pdev_ctrl_wmi *wmi;
	struct debug_pdev_ctrl_link *link;
};

/* Debug psoc data stats holder */
struct debug_psoc_data {
	struct debug_psoc_data_tx *tx;
	struct debug_psoc_data_rx *rx;
	struct debug_psoc_data_ast *ast;
};
#endif /* WLAN_DEBUG_TELEMETRY */

/**
 * struct stats_obj: Declares structure to hold Stats
 * @lvl: Stats level Basic/Advance/Debug
 * @obj_type: Stats object STA/VAP/RADIO/AP
 * @type: Stats type data or control
 * @pif_name: Parent interface name
 * @serviceid: serviceid for level of sawf stats
 * @u_id.mac_addr: MAC address for STA object
 * @u_id.if_name: Interface name for VAP/RADIO/AP objects
 * @stats: Stats based on above meta information
 * @next: Next stats_obj
 */
struct stats_obj {
	enum stats_level_e lvl;
	enum stats_object_e obj_type;
	enum stats_type_e type;
	char pif_name[IFNAME_LEN];
	uint8_t serviceid;
	union {
		u_int8_t mac_addr[ETH_ALEN];
		char if_name[IFNAME_LEN];
	} u_id;
	void *stats;
	struct stats_obj *next;
};

/**
 * struct reply_buffer: Defines structure to hold the driver reply
 * @obj_head:  Head pointer of stats_obj list
 * @obj_last:  Last pointer of stats_obj list
 */
struct reply_buffer {
	struct stats_obj *obj_head;
	struct stats_obj *obj_last;
};

/**
 * struct stats_command: Defines interface level command structure
 * @lvl:       Stats level
 * @obj:       Stats object
 * @type:      Stats traffic type
 * @recursive: Stats recursiveness
 * @serviceid: Stats serviceid
 * @feat_flag: Stats requested for combination of Features
 * @sta_mac:   Station MAC address if Stats requested for STA object
 * @if_name:   Interface name on which Stats is requested
 * @reply:     Pointer to reply buffer provided by user
 */
struct stats_command {
	enum stats_level_e lvl;
	enum stats_object_e obj;
	enum stats_type_e type;
	bool recursive;
	uint8_t serviceid;
	char if_name[IFNAME_LEN];
	u_int64_t feat_flag;
	struct ether_addr sta_mac;
	struct reply_buffer *reply;
};

/**
 * libstats_get_feature_flag(): Function to parse Feature flags and return value
 * @feat_flags: String holding feature flag names separted by dilimeter '|'
 *
 * Return: Combination of requested feature flag value or 0
 */
u_int64_t libstats_get_feature_flag(char *feat_flags);

/**
 * libstats_request_handle(): Function to send stats request to driver
 * @cmd: Filled command structure by Application
 *
 * Return: 0 on Success, -1 on Failure
 */
int32_t libstats_request_handle(struct stats_command *cmd);

/**
 * libstats_free_reply_buffer(): Function to free all reply objects
 * @cmd: Pointer to command structure
 *
 * Return: None
 */
void libstats_free_reply_buffer(struct stats_command *cmd);
#endif /* _STATS_LIB_H_ */
