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

#ifndef _WLAN_STATS_DEFINE_H_
#define _WLAN_STATS_DEFINE_H_

/* Flags for Feature specific stats */
#define STATS_FEAT_FLG_RX              0x00000001
#define STATS_FEAT_FLG_TX              0x00000002
#define STATS_FEAT_FLG_AST             0x00000004
#define STATS_FEAT_FLG_CFR             0x00000008
#define STATS_FEAT_FLG_FWD             0x00000010
#define STATS_FEAT_FLG_HTT             0x00000020
#define STATS_FEAT_FLG_RAW             0x00000040
#define STATS_FEAT_FLG_PEER            0x00000080
#define STATS_FEAT_FLG_TSO             0x00000100
#define STATS_FEAT_FLG_TWT             0x00000200
#define STATS_FEAT_FLG_VOW             0x00000400
#define STATS_FEAT_FLG_WDI             0x00000800
#define STATS_FEAT_FLG_WMI             0x00001000
#define STATS_FEAT_FLG_IGMP            0x00002000
#define STATS_FEAT_FLG_LINK            0x00004000
#define STATS_FEAT_FLG_MESH            0x00008000
#define STATS_FEAT_FLG_RATE            0x00010000
#define STATS_FEAT_FLG_DELAY           0x00020000
#define STATS_FEAT_FLG_ME              0x00040000
#define STATS_FEAT_FLG_NAWDS           0x00080000
#define STATS_FEAT_FLG_TXCAP           0x00100000
#define STATS_FEAT_FLG_MONITOR         0x00200000
#define STATS_FEAT_FLG_JITTER          0x00400000
#define STATS_FEAT_FLG_SAWFDELAY       0x00800000
#define STATS_FEAT_FLG_SAWFTX          0x01000000

/* Add new feature flag above and update STATS_FEAT_FLG_ALL */
#define STATS_FEAT_FLG_ALL             \
	(STATS_FEAT_FLG_RX | STATS_FEAT_FLG_TX | STATS_FEAT_FLG_AST | \
	 STATS_FEAT_FLG_CFR | STATS_FEAT_FLG_FWD | STATS_FEAT_FLG_HTT | \
	 STATS_FEAT_FLG_RAW | STATS_FEAT_FLG_PEER | STATS_FEAT_FLG_TSO | \
	 STATS_FEAT_FLG_TWT | STATS_FEAT_FLG_VOW | STATS_FEAT_FLG_WDI | \
	 STATS_FEAT_FLG_WMI | STATS_FEAT_FLG_IGMP | STATS_FEAT_FLG_LINK | \
	 STATS_FEAT_FLG_MESH | STATS_FEAT_FLG_RATE | STATS_FEAT_FLG_DELAY | \
	 STATS_FEAT_FLG_ME | STATS_FEAT_FLG_NAWDS | STATS_FEAT_FLG_TXCAP | \
	 STATS_FEAT_FLG_MONITOR | STATS_FEAT_FLG_JITTER | \
	 STATS_FEAT_FLG_SAWFDELAY | STATS_FEAT_FLG_SAWFTX)

#define STATS_BASIC_AP_CTRL_MASK       0
#define STATS_BASIC_AP_DATA_MASK       (STATS_FEAT_FLG_RX | STATS_FEAT_FLG_TX)
#define STATS_BASIC_RADIO_CTRL_MASK                    \
	(STATS_FEAT_FLG_RX | STATS_FEAT_FLG_TX |       \
	 STATS_FEAT_FLG_LINK)
#define STATS_BASIC_RADIO_DATA_MASK    (STATS_FEAT_FLG_RX | STATS_FEAT_FLG_TX)
#define STATS_BASIC_VAP_CTRL_MASK      (STATS_FEAT_FLG_RX | STATS_FEAT_FLG_TX)
#define STATS_BASIC_VAP_DATA_MASK      (STATS_FEAT_FLG_RX | STATS_FEAT_FLG_TX)
#define STATS_BASIC_STA_CTRL_MASK                      \
	(STATS_FEAT_FLG_RX | STATS_FEAT_FLG_TX |       \
	 STATS_FEAT_FLG_LINK | STATS_FEAT_FLG_RATE)
#define STATS_BASIC_STA_DATA_MASK                      \
	(STATS_FEAT_FLG_RX | STATS_FEAT_FLG_TX |       \
	 STATS_FEAT_FLG_LINK | STATS_FEAT_FLG_RATE)

#if WLAN_ADVANCE_TELEMETRY
#define STATS_ADVANCE_AP_CTRL_MASK     STATS_BASIC_AP_CTRL_MASK
#define STATS_ADVANCE_AP_DATA_MASK     STATS_BASIC_AP_DATA_MASK
#define STATS_ADVANCE_RADIO_CTRL_MASK  STATS_BASIC_RADIO_CTRL_MASK
#define STATS_ADVANCE_RADIO_DATA_MASK                  \
	(STATS_BASIC_RADIO_DATA_MASK |                 \
	 STATS_FEAT_FLG_ME | STATS_FEAT_FLG_RAW |      \
	 STATS_FEAT_FLG_TSO | STATS_FEAT_FLG_IGMP |    \
	 STATS_FEAT_FLG_MESH | STATS_FEAT_FLG_NAWDS)
#define STATS_ADVANCE_VAP_CTRL_MASK    STATS_BASIC_VAP_CTRL_MASK
#define STATS_ADVANCE_VAP_DATA_MASK                    \
	(STATS_BASIC_VAP_DATA_MASK |                   \
	 STATS_FEAT_FLG_ME | STATS_FEAT_FLG_RAW |      \
	 STATS_FEAT_FLG_TSO | STATS_FEAT_FLG_IGMP |    \
	 STATS_FEAT_FLG_MESH | STATS_FEAT_FLG_NAWDS)
#define STATS_ADVANCE_STA_CTRL_MASK                    \
	(STATS_BASIC_STA_CTRL_MASK | STATS_FEAT_FLG_TWT)
#define STATS_ADVANCE_STA_DATA_MASK                    \
	(STATS_BASIC_STA_DATA_MASK |                   \
	 STATS_FEAT_FLG_FWD | STATS_FEAT_FLG_TWT |     \
	 STATS_FEAT_FLG_RAW | STATS_FEAT_FLG_PEER |    \
	 STATS_FEAT_FLG_NAWDS | STATS_FEAT_FLG_DELAY | \
	 STATS_FEAT_FLG_JITTER | \
	 STATS_FEAT_FLG_SAWFDELAY | STATS_FEAT_FLG_SAWFTX)
#endif /* WLAN_ADVANCE_TELEMETRY */

#if WLAN_DEBUG_TELEMETRY
#define STATS_DEBUG_AP_CTRL_MASK      STATS_BASIC_AP_CTRL_MASK
#define STATS_DEBUG_AP_DATA_MASK                       \
	(STATS_BASIC_AP_DATA_MASK | STATS_FEAT_FLG_AST)
#define STATS_DEBUG_RADIO_CTRL_MASK                    \
	(STATS_BASIC_RADIO_CTRL_MASK | STATS_FEAT_FLG_WMI)
#define STATS_DEBUG_RADIO_DATA_MASK                    \
	(STATS_BASIC_RADIO_DATA_MASK |                 \
	 STATS_FEAT_FLG_ME | STATS_FEAT_FLG_RAW |      \
	 STATS_FEAT_FLG_TSO | STATS_FEAT_FLG_CFR |     \
	 STATS_FEAT_FLG_HTT | STATS_FEAT_FLG_TXCAP |   \
	 STATS_FEAT_FLG_WDI | STATS_FEAT_FLG_MONITOR | \
	 STATS_FEAT_FLG_MESH)
#define STATS_DEBUG_VAP_CTRL_MASK                      \
	(STATS_BASIC_VAP_CTRL_MASK | STATS_FEAT_FLG_WMI)
#define STATS_DEBUG_VAP_DATA_MASK                      \
	(STATS_BASIC_VAP_DATA_MASK |                   \
	 STATS_FEAT_FLG_ME | STATS_FEAT_FLG_RAW |      \
	 STATS_FEAT_FLG_TSO)
#define STATS_DEBUG_STA_CTRL_MASK     STATS_BASIC_STA_CTRL_MASK
#define STATS_DEBUG_STA_DATA_MASK                      \
	(STATS_BASIC_STA_DATA_MASK | STATS_FEAT_FLG_TXCAP)
#endif /* WLAN_DEBUG_TELEMETRY */

#ifdef WLAN_FEATURE_11BE
#define STATS_IF_MAX_MCS             (16 + 1)
#else
#define STATS_IF_MAX_MCS             (14 + 1)
#endif
#define STATS_IF_SS_COUNT            8

/**
 * enum stats_level_e: Defines detailing levels
 * @STATS_LVL_BASIC:    Very minimal stats data
 * @STATS_LVL_ADVANCE:  Mostly feature specific stats data
 * @STATS_LVL_DEBUG:    Stats data for debug purpose
 * @STATS_LVL_MAX:      Max supported Stats levels
 */
enum stats_level_e {
	STATS_LVL_BASIC,
	STATS_LVL_ADVANCE,
	STATS_LVL_DEBUG,
	STATS_LVL_MAX = STATS_LVL_DEBUG,
};

/**
 * enum stats_object_e: Defines the Stats specific to object
 * @STATS_OBJ_STA:   Stats for station/peer associated to AP
 * @STATS_OBJ_VAP:   Stats for VAP
 * @STATS_OBJ_RADIO: Stats for particular Radio
 * @STATS_OBJ_AP:    Stats for SoC
 * @STATS_OBJ_MAX:   Max supported objects
 */
enum stats_object_e {
	STATS_OBJ_STA,
	STATS_OBJ_VAP,
	STATS_OBJ_RADIO,
	STATS_OBJ_AP,
	STATS_OBJ_MAX = STATS_OBJ_AP,
};

/**
 * enum stats_type_e: Defines the Stats for specific category
 * @STATS_TYPE_DATA: Stats for Data frames
 * @STATS_TYPE_CTRL: Stats for Control/Management frames
 * @STATS_TYPE_MAX:  Max supported types
 */
enum stats_type_e {
	STATS_TYPE_DATA,
	STATS_TYPE_CTRL,
	STATS_TYPE_MAX = STATS_TYPE_CTRL,
};

enum stats_if_wme_ac {
	STATS_IF_WME_AC_BE = 0,
	STATS_IF_WME_AC_BK,
	STATS_IF_WME_AC_VI,
	STATS_IF_WME_AC_VO,
	STATS_IF_WME_AC_MAX,
};

enum stats_if_packet_type {
	STATS_IF_DOT11_A = 0,
	STATS_IF_DOT11_B = 1,
	STATS_IF_DOT11_N = 2,
	STATS_IF_DOT11_AC = 3,
	STATS_IF_DOT11_AX = 4,
#ifdef WLAN_FEATURE_11BE
	STATS_IF_DOT11_BE = 5,
#endif
	STATS_IF_DOT11_MAX,
};

/**
 * struct pkt_info: Structure to hold packet info
 * @num:  Number packets
 * @bytes:  NUmber of bytes
 */
struct pkt_info {
	u_int64_t num;
	u_int64_t bytes;
};

struct basic_data_tx_stats {
	struct pkt_info tx_success;
	struct pkt_info comp_pkt;
	u_int64_t dropped_count;
	u_int64_t tx_failed;
};

struct basic_data_rx_stats {
	struct pkt_info to_stack;
	struct pkt_info total_rcvd;
	u_int64_t rx_error_count;
};

/* Basic Peer Data */
struct basic_peer_data_tx {
	struct basic_data_tx_stats tx;
};

struct basic_peer_data_rx {
	struct basic_data_rx_stats rx;
};

struct basic_peer_data_rate {
	u_int32_t rx_rate;
	u_int32_t last_rx_rate;
	u_int32_t tx_rate;
	u_int32_t last_tx_rate;
};

struct basic_peer_data_link {
	u_int32_t avg_snr;
	u_int8_t snr;
	u_int8_t last_snr;
};

/* Basic Peer Ctrl */
struct basic_peer_ctrl_tx {
	u_int32_t cs_tx_mgmt;
	u_int32_t cs_is_tx_not_ok;
};

struct basic_peer_ctrl_rx {
	u_int32_t cs_rx_mgmt;
	u_int32_t cs_rx_decryptcrc;
	u_int32_t cs_rx_security_failure;
};

struct basic_peer_ctrl_rate {
	u_int32_t cs_rx_mgmt_rate;
};

struct basic_peer_ctrl_link {
	int8_t cs_rx_mgmt_snr;
};

/* Basic vdev Data */
struct basic_vdev_data_tx {
	struct basic_data_tx_stats tx;
	struct pkt_info ingress;
	struct pkt_info processed;
	struct pkt_info dropped;
};

struct basic_vdev_data_rx {
	struct basic_data_rx_stats rx;
};

/* Basic vdev Ctrl */
struct basic_vdev_ctrl_tx {
	u_int64_t cs_tx_mgmt;
	u_int64_t cs_tx_error_counter;
	u_int64_t cs_tx_discard;
};

struct basic_vdev_ctrl_rx {
	u_int64_t cs_rx_mgmt;
	u_int64_t cs_rx_error_counter;
	u_int64_t cs_rx_mgmt_discard;
	u_int64_t cs_rx_ctl;
	u_int64_t cs_rx_discard;
	u_int64_t cs_rx_security_failure;
};

/* Basic Pdev Data */
struct basic_pdev_data_tx {
	struct basic_data_tx_stats tx;
	struct pkt_info ingress;
	struct pkt_info processed;
	struct pkt_info dropped;
};

struct basic_pdev_data_rx {
	struct basic_data_rx_stats rx;
	u_int64_t dropped_count;
	u_int64_t err_count;
};

/* Basic pdev Ctrl */
struct basic_pdev_ctrl_tx {
	u_int32_t cs_tx_mgmt;
	u_int32_t cs_tx_frame_count;
};

struct basic_pdev_ctrl_rx {
	u_int32_t cs_rx_mgmt;
	u_int32_t cs_rx_num_mgmt;
	u_int32_t cs_rx_num_ctl;
	u_int32_t cs_rx_frame_count;
	u_int32_t cs_rx_error_sum;
};

struct basic_pdev_ctrl_link {
	u_int32_t cs_chan_tx_pwr;
	int16_t cs_chan_nf;
	int16_t cs_chan_nf_sec80;
	u_int8_t dcs_total_util;
};

/* Basic psoc Data */
struct basic_psoc_data_tx {
	struct pkt_info egress;
};

struct basic_psoc_data_rx {
	struct pkt_info ingress;
};

#if WLAN_ADVANCE_TELEMETRY
#define STATS_IF_MAX_GI              (4 + 1)
#define STATS_IF_MAX_BW              8
#define STATS_IF_MAX_DATA_TIDS       9
#define STATS_IF_MAX_RX_CTX          8
#define STATS_IF_MAX_SAWF_DATA_TIDS  8
#define STATS_IF_MAX_SAWF_DATA_QUEUE 2
#define STATS_IF_NUM_AVG_WINDOWS     5

enum stats_if_hist_bucket_index {
	STATS_IF_HIST_BUCKET_0,
	STATS_IF_HIST_BUCKET_1,
	STATS_IF_HIST_BUCKET_2,
	STATS_IF_HIST_BUCKET_3,
	STATS_IF_HIST_BUCKET_4,
	STATS_IF_HIST_BUCKET_5,
	STATS_IF_HIST_BUCKET_6,
	STATS_IF_HIST_BUCKET_7,
	STATS_IF_HIST_BUCKET_8,
	STATS_IF_HIST_BUCKET_9,
	STATS_IF_HIST_BUCKET_MAX,
};

enum stats_if_hist_types {
	STATS_IF_HIST_TYPE_SW_ENQEUE_DELAY,
	STATS_IF_HIST_TYPE_HW_COMP_DELAY,
	STATS_IF_HIST_TYPE_REAP_STACK,
	STATS_IF_HIST_TYPE_MAX,
};

struct stats_if_hist_bucket {
	enum stats_if_hist_types hist_type;
	uint64_t freq[STATS_IF_HIST_BUCKET_MAX];
};

struct stats_if_hist_stats {
	struct stats_if_hist_bucket hist;
	int32_t max;
	int32_t min;
	int32_t avg;
};

struct stats_if_delay_tx_stats {
	struct stats_if_hist_stats tx_swq_delay;
	struct stats_if_hist_stats hwtx_delay;
};

struct stats_if_delay_rx_stats {
	struct stats_if_hist_stats to_stack_delay;
};

struct stats_if_delay_tid_stats {
	struct stats_if_delay_tx_stats  tx_delay;
	struct stats_if_delay_rx_stats  rx_delay;
};

struct stats_if_jitter_tid_stats {
	u_int32_t tx_avg_jitter;
	u_int32_t tx_avg_delay;
	u_int64_t tx_avg_err;
	u_int64_t tx_total_success;
	u_int64_t tx_drop;
};

struct stats_if_sawf_delay_stats {
	struct stats_if_hist_stats delay_hist;
	struct {
		uint32_t sum;
		uint32_t count;
	} avg;
	struct {
		uint32_t sum;
		uint32_t count;
	} win_avgs[STATS_IF_NUM_AVG_WINDOWS];
	uint8_t cur_win;
};

struct stats_if_pkt_info {
	uint32_t num;
	uint64_t bytes;
};

struct stats_if_sawf_tx_stats {
	struct stats_if_pkt_info tx_success;
	struct {
		struct stats_if_pkt_info fw_rem;
		uint32_t fw_rem_notx;
		uint32_t fw_rem_tx;
		uint32_t age_out;
		uint32_t fw_reason1;
		uint32_t fw_reason2;
		uint32_t fw_reason3;
	} dropped;
	uint32_t tx_failed;
	uint32_t queue_depth;
};

struct advance_data_tx_stats {
	struct pkt_info ucast;
	struct pkt_info mcast;
	struct pkt_info bcast;
	u_int32_t nss[STATS_IF_SS_COUNT];
	u_int32_t sgi_count[STATS_IF_MAX_GI];
	u_int32_t bw[STATS_IF_MAX_BW];
	u_int32_t retries;
	u_int32_t non_amsdu_cnt;
	u_int32_t amsdu_cnt;
	u_int32_t ampdu_cnt;
	u_int32_t non_ampdu_cnt;
};

struct advance_data_rx_stats {
	struct pkt_info unicast;
	struct pkt_info multicast;
	struct pkt_info bcast;
	u_int32_t su_ax_ppdu_cnt[STATS_IF_MAX_MCS];
	u_int32_t rx_mpdu_cnt[STATS_IF_MAX_MCS];
	u_int32_t wme_ac_type[STATS_IF_WME_AC_MAX];
	u_int32_t sgi_count[STATS_IF_MAX_GI];
	u_int32_t nss[STATS_IF_SS_COUNT];
	u_int32_t ppdu_nss[STATS_IF_SS_COUNT];
	u_int32_t bw[STATS_IF_MAX_BW];
	u_int32_t mpdu_cnt_fcs_ok;
	u_int32_t mpdu_cnt_fcs_err;
	u_int32_t non_amsdu_cnt;
	u_int32_t non_ampdu_cnt;
	u_int32_t ampdu_cnt;
	u_int32_t amsdu_cnt;
	u_int32_t bar_recv_cnt;
	u_int32_t rx_retries;
	u_int32_t multipass_rx_pkt_drop;
};

/* Advance Peer Data */
struct advance_peer_data_tx {
	struct basic_peer_data_tx b_tx;
	struct advance_data_tx_stats adv_tx;
};

struct advance_peer_data_rx {
	struct basic_peer_data_rx b_rx;
	struct advance_data_rx_stats adv_rx;
};

struct advance_peer_data_raw {
	struct pkt_info raw;
};

struct advance_peer_data_fwd {
	struct pkt_info pkts;
	struct pkt_info fail;
	uint32_t mdns_no_fwd;
};

struct advance_peer_data_twt {
	struct pkt_info to_stack_twt;
	struct pkt_info tx_success_twt;
};

struct advance_peer_data_rate {
	struct basic_peer_data_rate b_rate;
	u_int32_t rnd_avg_rx_rate;
	u_int32_t avg_rx_rate;
	u_int32_t rnd_avg_tx_rate;
	u_int32_t avg_tx_rate;
};

struct advance_peer_data_link {
	struct basic_peer_data_link b_link;
	u_int32_t rx_snr_measured_time;
};

struct advance_peer_data_nawds {
	struct pkt_info nawds_mcast;
	u_int32_t nawds_mcast_tx_drop;
	u_int32_t nawds_mcast_rx_drop;
};

struct advance_peer_data_jitter {
	struct stats_if_jitter_tid_stats jitter_stats[STATS_IF_MAX_DATA_TIDS];
};

struct advance_peer_data_delay {
	struct stats_if_delay_tid_stats delay_stats[STATS_IF_MAX_DATA_TIDS];
};

struct advance_peer_data_sawfdelay {
	struct stats_if_sawf_delay_stats delay[STATS_IF_MAX_SAWF_DATA_TIDS][STATS_IF_MAX_SAWF_DATA_QUEUE];
};

struct advance_peer_data_sawftx {
	struct stats_if_sawf_tx_stats tx[STATS_IF_MAX_SAWF_DATA_TIDS][STATS_IF_MAX_SAWF_DATA_QUEUE];
};

/* Advance Peer Ctrl */
struct advance_peer_ctrl_tx {
	struct basic_peer_ctrl_tx b_tx;
	u_int32_t cs_tx_assoc;
	u_int32_t cs_tx_assoc_fail;
};

struct advance_peer_ctrl_rx {
	struct basic_peer_ctrl_rx b_rx;
};

struct advance_peer_ctrl_twt {
	u_int32_t cs_twt_event_type;
	u_int32_t cs_twt_dialog_id;
	u_int32_t cs_twt_wake_dura_us;
	u_int32_t cs_twt_wake_intvl_us;
	u_int32_t cs_twt_sp_offset_us;
	u_int32_t cs_twt_flow_id:16,
		  cs_twt_bcast:1,
		  cs_twt_trig:1,
		  cs_twt_announ:1;
};

struct advance_peer_ctrl_rate {
	struct basic_peer_ctrl_rate b_rate;
};

struct advance_peer_ctrl_link {
	struct basic_peer_ctrl_link b_link;
};

/* Advance Vdev Data */
struct advance_vdev_data_tx {
	struct basic_vdev_data_tx b_tx;
	struct advance_data_tx_stats adv_tx;
	struct pkt_info reinject_pkts;
	struct pkt_info inspect_pkts;
	u_int32_t cce_classified;
};

struct advance_vdev_data_rx {
	struct basic_vdev_data_rx b_rx;
	struct advance_data_rx_stats adv_rx;
};

struct advance_vdev_data_me {
	struct pkt_info mcast_pkt;
	u_int32_t ucast;
};

struct advance_vdev_data_raw {
	struct pkt_info rx_raw;
	struct pkt_info tx_raw_pkt;
	u_int32_t cce_classified_raw;
};

struct advance_vdev_data_tso {
	struct pkt_info sg_pkt;
	struct pkt_info non_sg_pkts;
	struct pkt_info num_tso_pkts;
};

struct advance_vdev_data_igmp {
	u_int32_t igmp_rcvd;
	u_int32_t igmp_ucast_converted;
};

struct advance_vdev_data_mesh {
	u_int32_t exception_fw;
	u_int32_t completion_fw;
};

struct advance_vdev_data_nawds {
	struct pkt_info tx_nawds_mcast;
	u_int32_t nawds_mcast_tx_drop;
	u_int32_t nawds_mcast_rx_drop;
};

/* Advance Vdev Ctrl */
struct advance_vdev_ctrl_tx {
	struct basic_vdev_ctrl_tx b_tx;
	u_int64_t cs_tx_offchan_mgmt;
	u_int64_t cs_tx_offchan_data;
	u_int64_t cs_tx_offchan_fail;
	u_int64_t cs_tx_bcn_success;
	u_int64_t cs_tx_bcn_outage;
	u_int64_t cs_fils_frames_sent;
	u_int64_t cs_fils_frames_sent_fail;
	u_int64_t cs_tx_offload_prb_resp_succ_cnt;
	u_int64_t cs_tx_offload_prb_resp_fail_cnt;
};

struct advance_vdev_ctrl_rx {
	struct basic_vdev_ctrl_rx b_rx;
	u_int64_t cs_rx_action;
	u_int64_t cs_mlme_auth_attempt;
	u_int64_t cs_mlme_auth_success;
	u_int64_t cs_authorize_attempt;
	u_int64_t cs_authorize_success;
	u_int64_t cs_prob_req_drops;
	u_int64_t cs_oob_probe_req_count;
	u_int64_t cs_wc_probe_req_drops;
	u_int64_t cs_sta_xceed_rlim;
	u_int64_t cs_sta_xceed_vlim;
};

/* Advance Pdev Data */
struct histogram_stats {
	u_int32_t pkts_1;
	u_int32_t pkts_2_20;
	u_int32_t pkts_21_40;
	u_int32_t pkts_41_60;
	u_int32_t pkts_61_80;
	u_int32_t pkts_81_100;
	u_int32_t pkts_101_200;
	u_int32_t pkts_201_plus;
};

struct advance_pdev_data_tx {
	struct basic_pdev_data_tx b_tx;
	struct advance_data_tx_stats adv_tx;
	struct pkt_info reinject_pkts;
	struct pkt_info inspect_pkts;
	struct histogram_stats tx_hist;
	u_int32_t cce_classified;
};

struct advance_pdev_data_rx {
	struct basic_pdev_data_rx b_rx;
	struct advance_data_rx_stats adv_rx;
	struct histogram_stats rx_hist;
};

struct advance_pdev_data_me {
	struct pkt_info mcast_pkt;
	u_int32_t ucast;
};

struct advance_pdev_data_raw {
	struct pkt_info rx_raw;
	struct pkt_info tx_raw_pkt;
	u_int32_t cce_classified_raw;
	u_int32_t rx_raw_pkts;
};

struct advance_pdev_data_tso {
	struct pkt_info sg_pkt;
	struct pkt_info non_sg_pkts;
	struct pkt_info num_tso_pkts;
	u_int64_t segs_1;
	u_int64_t segs_2_5;
	u_int64_t segs_6_10;
	u_int64_t segs_11_15;
	u_int64_t segs_16_20;
	u_int64_t segs_20_plus;
	u_int32_t tso_comp;
};

struct advance_pdev_data_igmp {
	u_int32_t igmp_rcvd;
	u_int32_t igmp_ucast_converted;
};

struct advance_pdev_data_mesh {
	u_int32_t exception_fw;
	u_int32_t completion_fw;
};

struct advance_pdev_data_nawds {
	struct pkt_info tx_nawds_mcast;
	u_int32_t nawds_mcast_tx_drop;
	u_int32_t nawds_mcast_rx_drop;
};

/* Advance Pdev Ctrl */
struct advance_pdev_ctrl_tx {
	struct basic_pdev_ctrl_tx b_tx;
	u_int64_t cs_tx_beacon;
};

struct advance_pdev_ctrl_rx {
	struct basic_pdev_ctrl_rx b_rx;
	u_int32_t cs_rx_mgmt_rssi_drop;
};

struct advance_pdev_ctrl_link {
	struct basic_pdev_ctrl_link b_link;
	u_int32_t dcs_ss_under_util;
	u_int32_t dcs_sec_20_util;
	u_int32_t dcs_sec_40_util;
	u_int32_t dcs_sec_80_util;
	u_int32_t cs_tx_rssi;
	u_int8_t dcs_ap_tx_util;
	u_int8_t dcs_ap_rx_util;
	u_int8_t dcs_self_bss_util;
	u_int8_t dcs_obss_util;
	u_int8_t dcs_obss_rx_util;
	u_int8_t dcs_free_medium;
	u_int8_t dcs_non_wifi_util;
	u_int8_t rx_rssi_chain0_pri20;
	u_int8_t rx_rssi_chain0_sec20;
	u_int8_t rx_rssi_chain0_sec40;
	u_int8_t rx_rssi_chain0_sec80;
	u_int8_t rx_rssi_chain1_pri20;
	u_int8_t rx_rssi_chain1_sec20;
	u_int8_t rx_rssi_chain1_sec40;
	u_int8_t rx_rssi_chain1_sec80;
	u_int8_t rx_rssi_chain2_pri20;
	u_int8_t rx_rssi_chain2_sec20;
	u_int8_t rx_rssi_chain2_sec40;
	u_int8_t rx_rssi_chain2_sec80;
	u_int8_t rx_rssi_chain3_pri20;
	u_int8_t rx_rssi_chain3_sec20;
	u_int8_t rx_rssi_chain3_sec40;
	u_int8_t rx_rssi_chain3_sec80;
};

/* Advance Psoc Data */
struct advance_psoc_data_tx {
	struct basic_psoc_data_tx b_tx;
};

struct advance_psoc_data_rx {
	struct basic_psoc_data_rx b_rx;
	u_int32_t err_ring_pkts;
	u_int32_t rx_frags;
	u_int32_t rx_hw_reinject;
	u_int32_t bar_frame;
	u_int32_t rejected;
	u_int32_t raw_frm_drop;
};
#endif /* WLAN_ADVANCE_TELEMETRY */

#if WLAN_DEBUG_TELEMETRY
#define STATS_IF_NR_CPUS                     8
#define STATS_IF_MAX_PPDU_ID_HIST            128
#define STATS_IF_MAX_MU_GROUP_ID             64
#define STATS_IF_MU_GROUP_SHOW               16
#define STATS_IF_MU_GROUP_LENGTH             (6 * STATS_IF_MU_GROUP_SHOW)
#ifdef WLAN_FEATURE_11BE
#define STATS_IF_MAX_RU_LOCATIONS            16
#define STATS_IF_OFDMA_NUM_RU_SIZE           16
#else
#define STATS_IF_MAX_RU_LOCATIONS            6
#define STATS_IF_OFDMA_NUM_RU_SIZE           7
#endif
#define STATS_IF_PPDU_STATS_MAX_TAG          14
#define STATS_IF_OFDMA_NUM_USERS             37
#define STATS_IF_MAX_TRANSMIT_TYPES          9
#define STATS_IF_MAX_TIDS                    17
#define STATS_IF_MAX_RECEPTION_TYPES         4
#define STATS_IF_MAX_RX_RINGS                8
#define STATS_IF_MAX_RX_DEST_RINGS           8
#define STATS_IF_MAX_TX_DATA_RINGS           5
#define STATS_IF_MAX_WIFI_INT_ERROR_REASONS  5
#define STATS_IF_TX_CAP_HTT_MAX_FTYPE        19
#define STATS_IF_FC0_TYPE_SHIFT              2
#define STATS_IF_FC0_SUBTYPE_SHIFT           4
#define STATS_IF_FC0_TYPE_DATA               0x08
#define STATS_IF_FC0_SUBTYPE_MASK            0xf0

#define STATS_IF_TXCAP_MAX_TYPE \
	((STATS_IF_FC0_TYPE_DATA >> STATS_IF_FC0_TYPE_SHIFT) + 1)
#define STATS_IF_TXCAP_MAX_SUBTYPE \
	((STATS_IF_FC0_SUBTYPE_MASK >> STATS_IF_FC0_SUBTYPE_SHIFT) + 1)

enum stats_if_protocol_trace {
	STATS_IF_TRACE_ICMP,
	STATS_IF_TRACE_EAP,
	STATS_IF_TRACE_ARP,
	STATS_IF_TRACE_MAX
};

enum stats_if_freeze_capture_reason {
	STATS_IF_FREEZE_REASON_TM = 0,
	STATS_IF_FREEZE_REASON_FTM,
	STATS_IF_FREEZE_REASON_ACK_RESP_TO_TM_FTM,
	STATS_IF_FREEZE_REASON_TA_RA_TYPE_FILTER,
	STATS_IF_FREEZE_REASON_NDPA_NDP,
	STATS_IF_FREEZE_REASON_ALL_PACKET,
	STATS_IF_FREEZE_REASON_MAX
};

enum stats_if_chan_capture_status {
	STATS_IF_CAPTURE_IDLE = 0,
	STATS_IF_CAPTURE_BUSY,
	STATS_IF_CAPTURE_ACTIVE,
	STATS_IF_CAPTURE_NO_BUFFER,
	STATS_IF_CAPTURE_MAX
};

enum stats_if_msdu_counter {
	STATS_IF_MSDU_SUCC,
	STATS_IF_MSDU_ENQ,
	STATS_IF_MSDU_DEQ,
	STATS_IF_MSDU_FLUSH,
	STATS_IF_MSDU_DROP,
	STATS_IF_MSDU_XRETRY,
	STATS_IF_MSDU_MAX,
};

enum stats_if_mpdu_counter {
	STATS_IF_MPDU_TRI,
	STATS_IF_MPDU_SUCC,
	STATS_IF_MPDU_RESTITCH,
	STATS_IF_MPDU_ARR,
	STATS_IF_MPDU_CLONE,
	STATS_IF_MPDU_TO_STACK,
	STATS_IF_MPDU_MAX,
};

enum stats_if_wdi_event {
	STATS_IF_WDI_EVENT_TX_STATUS = 0,
	STATS_IF_WDI_EVENT_OFFLOAD_ALL,
	STATS_IF_WDI_EVENT_RX_DESC_REMOTE,
	STATS_IF_WDI_EVENT_RX_PEER_INVALID,
	STATS_IF_WDI_EVENT_DBG_PRINT,
	STATS_IF_WDI_EVENT_RX_CBF_REMOTE,
	STATS_IF_WDI_EVENT_RATE_FIND,
	STATS_IF_WDI_EVENT_RATE_UPDATE,
	STATS_IF_WDI_EVENT_SW_EVENT,
	STATS_IF_WDI_EVENT_RX_DESC,
	STATS_IF_WDI_EVENT_LITE_T2H,
	STATS_IF_WDI_EVENT_LITE_RX,
	STATS_IF_WDI_EVENT_RX_PPDU_DESC,
	STATS_IF_WDI_EVENT_TX_PPDU_DESC,
	STATS_IF_WDI_EVENT_TX_MSDU_DESC,
	STATS_IF_WDI_EVENT_TX_DATA,
	STATS_IF_WDI_EVENT_RX_DATA,
	STATS_IF_WDI_EVENT_TX_MGMT_CTRL,
	STATS_IF_WDI_EVENT_TX_PKT_CAPTURE,
	STATS_IF_WDI_EVENT_HTT_STATS,
	STATS_IF_WDI_EVENT_TX_BEACON,
	STATS_IF_WDI_EVENT_PEER_STATS,
	STATS_IF_WDI_EVENT_TX_SOJOURN_STAT,
	STATS_IF_WDI_EVENT_UPDATE_DP_STATS,
	STATS_IF_WDI_EVENT_RX_MGMT_CTRL,
	STATS_IF_WDI_EVENT_PEER_CREATE,
	STATS_IF_WDI_EVENT_PEER_DESTROY,
	STATS_IF_WDI_EVENT_PEER_FLUSH_RATE_STATS,
	STATS_IF_WDI_EVENT_FLUSH_RATE_STATS_REQ,
	STATS_IF_WDI_EVENT_RX_MPDU,
	STATS_IF_WDI_EVENT_HMWDS_AST_ADD_STATUS,
	STATS_IF_WDI_EVENT_PEER_QOS_STATS,
	STATS_IF_WDI_EVENT_PKT_CAPTURE_TX_DATA,
	STATS_IF_WDI_EVENT_PKT_CAPTURE_RX_DATA,
	STATS_IF_WDI_EVENT_PKT_CAPTURE_RX_DATA_NO_PEER,
	STATS_IF_WDI_EVENT_PKT_CAPTURE_OFFLOAD_TX_DATA,
	STATS_IF_WDI_EVENT_RX_CBF,
	STATS_IF_WDI_EVENT_PKT_CAPTURE_PPDU_STATS,
	STATS_IF_WDI_EVENT_HOST_SW_EVENT,
#ifdef QCA_WIFI_QCN9224
	STATS_IF_WDI_EVENT_HYBRID_TX,
#ifdef WLAN_FEATURE_11BE_MLO
	STATS_IF_WDI_EVENT_MLO_TSTMP,
#endif
#endif
	/* End of new event items */
	STATS_IF_WDI_EVENT_LAST
};

enum stats_if_wifi_error_code {
	STATS_IF_WIFI_ERR_OVERFLOW = 0,
	STATS_IF_WIFI_ERR_MPDU_LENGTH,
	STATS_IF_WIFI_ERR_FCS,
	STATS_IF_WIFI_ERR_DECRYPT,
	STATS_IF_WIFI_ERR_TKIP_MIC,
	STATS_IF_WIFI_ERR_UNENCRYPTED,
	STATS_IF_WIFI_ERR_MSDU_LEN,
	STATS_IF_WIFI_ERR_MSDU_LIMIT,
	STATS_IF_WIFI_ERR_WIFI_PARSE,
	STATS_IF_WIFI_ERR_AMSDU_PARSE,
	STATS_IF_WIFI_ERR_SA_TIMEOUT,
	STATS_IF_WIFI_ERR_DA_TIMEOUT,
	STATS_IF_WIFI_ERR_FLOW_TIMEOUT,
	STATS_IF_WIFI_ERR_FLUSH_REQUEST,
	STATS_IF_WIFI_ERR_AMSDU_FRAGMENT,
	STATS_IF_WIFI_ERR_MULTICAST_ECHO,
	STATS_IF_WIFI_ERR_DUMMY = 31,
	STATS_IF_WIFI_ERR_MAX
};

enum stats_if_phy_rx_error_code {
	STATS_IF_RX_ERR_QUEUE_ADDR_0 = 0,
	STATS_IF_RX_ERR_QUEUE_INVALID,
	STATS_IF_RX_ERR_AMPDU_IN_NON_BA,
	STATS_IF_RX_ERR_NON_BA_DUPLICATE,
	STATS_IF_RX_ERR_BA_DUPLICATE,
	STATS_IF_RX_ERR_REGULAR_FRAME_2K_JUMP,
	STATS_IF_RX_ERR_BAR_FRAME_2K_JUMP,
	STATS_IF_RX_ERR_REGULAR_FRAME_OOR,
	STATS_IF_RX_ERR_BAR_FRAME_OOR,
	STATS_IF_RX_ERR_BAR_FRAME_NO_BA_SESSION,
	STATS_IF_RX_ERR_BAR_FRAME_SN_EQUALS_SSN,
	STATS_IF_RX_ERR_PN_CHECK_FAILED,
	STATS_IF_RX_ERR_2K_ERROR_HANDLING_FLAG_SET,
	STATS_IF_RX_ERR_PN_ERROR_HANDLING_FLAG_SET,
	STATS_IF_RX_ERR_QUEUE_BLOCKED_SET,
	STATS_IF_RX_ERR_MAX
};

enum stats_if_tx_transmit_type {
	STATS_IF_SU = 0,
	STATS_IF_MU_MIMO,
	STATS_IF_MU_OFDMA,
	STATS_IF_MU_MIMO_OFDMA,
};

enum stats_if_mu_packet_type {
	STATS_IF_TXRX_TYPE_MU_MIMO = 0,
	STATS_IF_TXRX_TYPE_MU_OFDMA = 1,
	STATS_IF_TXRX_TYPE_MU_MAX = 2,
};

struct tx_pkt_info {
	uint32_t num_msdu;
	uint32_t num_mpdu;
	uint32_t mpdu_tried;
};

struct proto_trace_count {
	uint16_t egress_cnt;
	uint16_t ingress_cnt;
};

struct pkt_type {
	uint32_t mcs_count[STATS_IF_MAX_MCS];
};

struct rx_mu_info {
	uint32_t ppdu_nss[STATS_IF_SS_COUNT];
	uint32_t mpdu_cnt_fcs_ok;
	uint32_t mpdu_cnt_fcs_err;
	struct pkt_type ppdu;
};

struct per_cpu_packets {
	uint8_t num_cpus;
	uint64_t pkts[STATS_IF_NR_CPUS][STATS_IF_MAX_RX_DEST_RINGS];
};

struct debug_data_tx_stats {
	uint32_t inactive_time;
	uint32_t ofdma;
	uint32_t stbc;
	uint32_t ldpc;
	uint32_t pream_punct_cnt;
	uint32_t num_ppdu_cookie_valid;
	uint32_t fw_rem_notx;
	uint32_t fw_rem_tx;
	uint32_t age_out;
	uint32_t fw_reason1;
	uint32_t fw_reason2;
	uint32_t fw_reason3;
	uint32_t ru_start;
	uint32_t ru_tones;
	uint32_t wme_ac_type[STATS_IF_WME_AC_MAX];
	uint32_t excess_retries_per_ac[STATS_IF_WME_AC_MAX];
	uint32_t mu_group_id[STATS_IF_MAX_MU_GROUP_ID];
	uint32_t no_ack_count[QDF_PROTO_SUBTYPE_MAX];
	struct pkt_info fw_rem;
	struct pkt_type pkt_type[STATS_IF_DOT11_MAX];
	struct tx_pkt_info transmit_type[STATS_IF_MAX_TRANSMIT_TYPES];
	struct tx_pkt_info ru_loc[STATS_IF_MAX_RU_LOCATIONS];
	struct proto_trace_count protocol_trace_cnt[STATS_IF_TRACE_MAX];
};

struct debug_data_rx_stats {
	uint32_t rx_discard;
	uint32_t mic_err;
	uint32_t decrypt_err;
	uint32_t fcserr;
	uint32_t pn_err;
	uint32_t oor_err;
	uint32_t ppdu_cnt[STATS_IF_MAX_RECEPTION_TYPES];
	uint32_t reception_type[STATS_IF_MAX_RECEPTION_TYPES];
	struct pkt_info mec_drop;
	struct pkt_type pkt_type[STATS_IF_DOT11_MAX];
	struct rx_mu_info rx_mu[STATS_IF_TXRX_TYPE_MU_MAX];
	struct proto_trace_count protocol_trace_cnt[STATS_IF_TRACE_MAX];
	struct pkt_info rcvd_reo[STATS_IF_MAX_RX_RINGS];
};

/* Debug Peer Data */
struct debug_peer_data_tx {
	struct basic_peer_data_tx b_tx;
	struct debug_data_tx_stats dbg_tx;
	uint32_t last_per;
	uint32_t tx_bytes_success_last;
	uint32_t tx_data_success_last;
	uint32_t tx_byte_rate;
	uint32_t tx_data_rate;
	uint32_t tx_data_ucast_last;
	uint32_t tx_data_ucast_rate;
};

struct debug_peer_data_rx {
	struct basic_peer_data_rx b_rx;
	struct debug_data_rx_stats dbg_rx;
	uint32_t rx_bytes_success_last;
	uint32_t rx_data_success_last;
	uint32_t rx_byte_rate;
	uint32_t rx_data_rate;
};

struct debug_peer_data_link {
	struct basic_peer_data_link b_link;
	uint32_t last_ack_rssi;
};

struct debug_peer_data_rate {
	struct basic_peer_data_rate b_rate;
	uint32_t last_tx_rate_mcs;
	uint32_t mcast_last_tx_rate;
	uint32_t mcast_last_tx_rate_mcs;
};

struct debug_peer_data_txcap {
	uint64_t defer_msdu_len[STATS_IF_MAX_TIDS];
	uint64_t tasklet_msdu_len[STATS_IF_MAX_TIDS];
	uint64_t pending_q_len[STATS_IF_MAX_TIDS];
	uint32_t mpdu[STATS_IF_MPDU_MAX];
	uint32_t msdu[STATS_IF_MSDU_MAX];
};

/* Debug Peer control */
struct debug_peer_ctrl_tx {
	struct basic_peer_ctrl_tx b_tx;
	uint32_t cs_ps_discard;
	uint32_t cs_psq_drops;
	uint32_t cs_tx_dropblock;
	uint32_t cs_is_tx_nobuf;
};

struct debug_peer_ctrl_rx {
	struct basic_peer_ctrl_rx b_rx;
	uint32_t cs_rx_noprivacy;
	uint32_t cs_rx_wepfail;
	uint32_t cs_rx_ccmpmic;
	uint32_t cs_rx_wpimic;
	uint32_t cs_rx_tkipicv;
};

struct debug_peer_ctrl_link {
	struct basic_peer_ctrl_link b_link;
};

struct debug_peer_ctrl_rate {
	struct basic_peer_ctrl_rate b_rate;
};

/* Debug vdev data */
struct debug_vdev_data_tx {
	struct basic_vdev_data_tx b_tx;
	struct debug_data_tx_stats dbg_tx;
	struct pkt_info desc_na;
	struct pkt_info desc_na_exc_alloc_fail;
	struct pkt_info desc_na_exc_outstand;
	struct pkt_info exc_desc_na;
	struct pkt_info sniffer_rcvd;
	uint32_t ring_full;
	uint32_t enqueue_fail;
	uint32_t dma_error;
	uint32_t res_full;
	uint32_t headroom_insufficient;
	uint32_t fail_per_pkt_vdev_id_check;
};

struct debug_vdev_data_rx {
	struct basic_vdev_data_rx b_rx;
	struct debug_data_rx_stats dbg_rx;
};

struct debug_vdev_data_me {
	uint32_t dropped_map_error;
	uint32_t dropped_self_mac;
	uint32_t dropped_send_fail;
	uint32_t fail_seg_alloc;
	uint32_t clone_fail;
};

struct debug_vdev_data_raw {
	uint32_t dma_map_error;
	uint32_t invalid_raw_pkt_datatype;
	uint32_t num_frags_overflow_err;
};

struct debug_vdev_data_tso {
	struct pkt_info dropped_host;
	uint32_t dma_map_error;
	uint32_t dropped_target;
};

/* Debug Peer control */
struct debug_vdev_ctrl_tx {
	struct basic_vdev_ctrl_tx b_tx;
	uint64_t cs_tx_bcn_swba;
	uint64_t cs_tx_nodefkey;
	uint64_t cs_tx_noheadroom;
	uint64_t cs_tx_nobuf;
	uint64_t cs_tx_nonode;
	uint64_t cs_tx_cipher_err;
	uint64_t cs_tx_not_ok;
};

struct debug_vdev_ctrl_rx {
	struct basic_vdev_ctrl_rx b_rx;
	uint64_t cs_invalid_macaddr_nodealloc_fail;
	uint64_t cs_rx_wrongdir;
	uint64_t cs_rx_not_assoc;
	uint64_t cs_rx_rs_too_big;
	uint64_t cs_rx_elem_missing;
	uint64_t cs_rx_elem_too_big;
	uint64_t cs_rx_chan_err;
	uint64_t cs_rx_node_alloc;
	uint64_t cs_rx_auth_unsupported;
	uint64_t cs_rx_auth_fail;
	uint64_t cs_rx_auth_countermeasures;
	uint64_t cs_rx_assoc_bss;
	uint64_t cs_rx_assoc_notauth;
	uint64_t cs_rx_assoc_cap_mismatch;
	uint64_t cs_rx_assoc_norate;
	uint64_t cs_rx_assoc_wpaie_err;
	uint64_t cs_rx_auth_err;
	uint64_t cs_rx_acl;
	uint64_t cs_rx_nowds;
	uint64_t cs_rx_wrongbss;
	uint64_t cs_rx_tooshort;
	uint64_t cs_rx_ssid_mismatch;
	uint64_t cs_rx_decryptok_u;
	uint64_t cs_rx_decryptok_m;
};

struct debug_vdev_ctrl_wmi {
	uint64_t cs_peer_delete_req;
	uint64_t cs_peer_delete_resp;
	uint64_t cs_peer_delete_all_req;
	uint64_t cs_peer_delete_all_resp;
};

/* Debug Radio data */
struct debug_pdev_data_tx {
	struct basic_pdev_data_tx b_tx;
	struct debug_data_tx_stats dbg_tx;
};

struct debug_pdev_data_rx {
	struct basic_pdev_data_rx b_rx;
	struct debug_data_rx_stats dbg_rx;
	struct pkt_info replenished_pkts;
	uint32_t rxdma_err;
	uint32_t nbuf_alloc_fail;
	uint32_t frag_alloc_fail;
	uint32_t map_err;
	uint32_t x86_fail;
	uint32_t low_thresh_intrs;
	uint32_t buf_freelist;
	uint32_t vlan_tag_stp_cnt;
	uint32_t msdu_not_done;
	uint32_t mec;
	uint32_t mesh_filter;
	uint32_t wifi_parse;
	uint32_t mon_rx_drop;
	uint32_t mon_radiotap_update_err;
	uint32_t desc_alloc_fail;
	uint32_t ip_csum_err;
	uint32_t tcp_udp_csum_err;
	uint32_t rxdma_error;
	uint32_t reo_error;
};

struct debug_pdev_data_me {
	uint32_t dropped_map_error;
	uint32_t dropped_self_mac;
	uint32_t dropped_send_fail;
	uint32_t fail_seg_alloc;
	uint32_t clone_fail;
};

struct debug_pdev_data_raw {
	uint32_t dma_map_error;
	uint32_t invalid_raw_pkt_datatype;
	uint32_t num_frags_overflow_err;
};

struct debug_pdev_data_tso {
	struct pkt_info dropped_host;
	struct pkt_info tso_no_mem_dropped;
	uint32_t dropped_target;
};

struct debug_pdev_data_cfr {
	uint64_t bb_captured_channel_cnt;
	uint64_t bb_captured_timeout_cnt;
	uint64_t rx_loc_info_valid_cnt;
	uint64_t chan_capture_status[STATS_IF_CAPTURE_MAX];
	uint64_t reason_cnt[STATS_IF_FREEZE_REASON_MAX];
};

struct debug_pdev_data_htt {
};

struct debug_pdev_data_wdi {
	uint32_t wdi_event[STATS_IF_WDI_EVENT_LAST];
};

struct debug_pdev_data_mesh {
	uint32_t mesh_mem_alloc;
};

struct debug_pdev_data_txcap {
	uint32_t delayed_ba_not_recev;
	uint32_t last_rcv_ppdu;
	uint32_t ppdu_stats_queue_depth;
	uint32_t ppdu_stats_defer_queue_depth;
	uint32_t ppdu_dropped;
	uint32_t pend_ppdu_dropped;
	uint32_t ppdu_flush_count;
	uint32_t msdu_threshold_drop;
	uint32_t htt_frame_type[STATS_IF_TX_CAP_HTT_MAX_FTYPE];
	uint64_t ctl_mgmt_q_len[STATS_IF_TXCAP_MAX_TYPE]
			       [STATS_IF_TXCAP_MAX_SUBTYPE];
	uint64_t retries_ctl_mgmt_q_len[STATS_IF_TXCAP_MAX_TYPE]
				       [STATS_IF_TXCAP_MAX_SUBTYPE];
	uint64_t peer_mismatch;
	uint64_t defer_msdu_len;
	uint64_t tasklet_msdu_len;
	uint64_t pending_q_len;
	uint64_t tx_ppdu_proc;
	uint64_t ack_ba_comes_twice;
	uint64_t ppdu_drop;
	uint64_t ppdu_wrap_drop;
	uint64_t ppdu_stats_counter[STATS_IF_PPDU_STATS_MAX_TAG];
};

struct debug_pdev_data_monitor {
	uint32_t data_rx_ru_size[STATS_IF_OFDMA_NUM_RU_SIZE];
	uint32_t nondata_rx_ru_size[STATS_IF_OFDMA_NUM_RU_SIZE];
	uint32_t data_rx_ppdu;
	uint32_t data_users[STATS_IF_OFDMA_NUM_USERS];
	uint32_t status_ppdu_state;
	uint32_t status_ppdu_start;
	uint32_t status_ppdu_end;
	uint32_t status_ppdu_compl;
	uint32_t status_ppdu_start_mis;
	uint32_t status_ppdu_end_mis;
	uint32_t status_ppdu_done;
	uint32_t dest_ppdu_done;
	uint32_t dest_mpdu_done;
	uint32_t dest_mpdu_drop;
	uint32_t dup_mon_linkdesc_cnt;
	uint32_t dup_mon_buf_cnt;
	uint32_t stat_ring_ppdu_id_hist[STATS_IF_MAX_PPDU_ID_HIST];
	uint32_t dest_ring_ppdu_id_hist[STATS_IF_MAX_PPDU_ID_HIST];
	uint32_t ppdu_id_hist_idx;
	uint32_t mon_rx_dest_stuck;
	uint32_t tlv_tag_status_err;
	uint32_t status_buf_done_war;
	uint32_t mon_rx_bufs_replenished_dest;
	uint32_t mon_rx_bufs_reaped_dest;
	uint32_t ppdu_id_mismatch;
	uint32_t ppdu_id_match;
	uint32_t status_ppdu_drop;
	uint32_t dest_ppdu_drop;
	uint32_t mon_link_desc_invalid;
	uint32_t mon_rx_desc_invalid;
	uint32_t mon_nbuf_sanity_err;
};

/* Debug pdev control */
struct debug_pdev_ctrl_tx {
	struct basic_pdev_ctrl_tx b_tx;
	uint32_t cs_be_nobuf;
	uint32_t cs_tx_buf_count;
	uint64_t cs_tx_hw_retries;
	uint64_t cs_tx_hw_failures;
	uint8_t  cs_ap_stats_tx_cal_enable;
};

struct debug_pdev_ctrl_rx {
	struct basic_pdev_ctrl_rx b_rx;
	uint32_t cs_rx_rts_success;
	uint32_t cs_rx_clear_count;
	uint32_t cs_rx_overrun;
	uint32_t cs_rx_phy_err;
	uint32_t cs_rx_ack_err;
	uint32_t cs_rx_rts_err;
	uint32_t cs_no_beacons;
	uint32_t cs_phy_err_count;
	uint32_t cs_fcsbad;
	uint32_t cs_rx_looplimit_start;
	uint32_t cs_rx_looplimit_end;
};

struct debug_pdev_ctrl_wmi {
	uint64_t cs_wmi_tx_mgmt;
	uint64_t cs_wmi_tx_mgmt_completions;
	uint32_t cs_wmi_tx_mgmt_completion_err;
};

struct debug_pdev_ctrl_link {
	struct basic_pdev_ctrl_link b_link;
};

/* Debug psoc data */
struct debug_psoc_data_tx {
	struct basic_psoc_data_tx b_tx;
	struct pkt_info tx_invalid_peer;
	uint32_t tx_hw_enq[STATS_IF_MAX_TX_DATA_RINGS];
	uint32_t tx_hw_ring_full[STATS_IF_MAX_TX_DATA_RINGS];
	uint32_t desc_in_use;
	uint32_t dropped_fw_removed;
	uint32_t invalid_release_source;
	uint32_t wifi_internal_error[STATS_IF_MAX_WIFI_INT_ERROR_REASONS];
	uint32_t non_wifi_internal_err;
	uint32_t tx_comp_loop_pkt_limit_hit;
	uint32_t hp_oos2;
	uint32_t tx_comp_exception;
};

struct debug_psoc_data_rx {
	struct basic_psoc_data_rx b_rx;
	struct per_cpu_packets rx_packets;
	struct pkt_info rx_invalid_peer;
	struct pkt_info rx_invalid_peer_id;
	struct pkt_info rx_invalid_pkt_len;
	uint32_t rx_frag_err_len_error;
	uint32_t rx_frag_err_no_peer;
	uint32_t rx_frag_wait;
	uint32_t rx_frag_err;
	uint32_t rx_frag_oor;
	uint32_t reap_loop_pkt_limit_hit;
	uint32_t hp_oos2;
	uint32_t near_full;
	uint32_t msdu_scatter_wait_break;
	uint32_t rx_sw_route_drop;
	uint32_t rx_hw_route_drop;
	uint32_t phy_ring_access_fail;
	uint32_t phy_ring_access_full_fail;
	uint32_t phy_rx_hw_error[STATS_IF_MAX_RX_DEST_RINGS];
	uint32_t phy_rx_hw_dest_dup;
	uint32_t phy_wifi_rel_dup;
	uint32_t phy_rx_sw_err_dup;
	uint32_t invalid_rbm;
	uint32_t invalid_vdev;
	uint32_t invalid_pdev;
	uint32_t pkt_delivered_no_peer;
	uint32_t defrag_peer_uninit;
	uint32_t invalid_sa_da_idx;
	uint32_t msdu_done_fail;
	uint32_t rx_sw_error[STATS_IF_WIFI_ERR_MAX];
	uint32_t rx_desc_invalid_magic;
	uint32_t rx_hw_error[STATS_IF_RX_ERR_MAX];
	uint32_t rx_hw_cmd_send_fail;
	uint32_t rx_hw_cmd_send_drain;
	uint32_t scatter_msdu;
	uint32_t invalid_cookie;
	uint32_t stale_cookie;
	uint32_t rx_2k_jump_delba_sent;
	uint32_t rx_2k_jump_to_stack;
	uint32_t rx_2k_jump_drop;
	uint32_t rx_hw_err_msdu_buf_rcved;
	uint32_t rx_hw_err_msdu_buf_invalid_cookie;
	uint32_t rx_hw_err_oor_drop;
	uint32_t rx_hw_err_oor_to_stack;
	uint32_t rx_hw_err_oor_sg_count;
	uint32_t msdu_count_mismatch;
	uint32_t invalid_link_cookie;
	uint32_t nbuf_sanity_fail;
	uint32_t dup_refill_link_desc;
	uint32_t msdu_continuation_err;
	uint32_t ssn_update_count;
	uint32_t bar_handle_fail_count;
	uint32_t intrabss_eapol_drop;
	uint32_t pn_in_dest_check_fail;
	uint32_t msdu_len_err;
	uint32_t rx_flush_count;
};

struct debug_psoc_data_ast {
	uint32_t ast_added;
	uint32_t ast_deleted;
	uint32_t ast_aged_out;
	uint32_t ast_map_err;
	uint32_t ast_mismatch;
	uint32_t mec_added;
	uint32_t mec_deleted;
};
#endif /* WLAN_DEBUG_TELEMETRY */
#endif /* _WLAN_STATS_DEFINE_H_ */
