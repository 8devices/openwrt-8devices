/*
 * Copyright (c) 2022 Qualcomm Innovation Center, Inc. All rights reserved.
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

#ifndef _DP_SAWF_H_
#define _DP_SAWF_H_

#include <qdf_lock.h>
#include <dp_types.h>
#include "dp_internal.h"
#include "dp_types.h"
#include "cdp_txrx_cmn_struct.h"
#include "cdp_txrx_hist_struct.h"
#include "cdp_txrx_extd_struct.h"

#define MSDU_QUEUE_LATENCY_WIN_MIN_SAMPLES 20
#define WLAN_TX_DELAY_UNITS_US 10
#define WLAN_TX_DELAY_MASK 0x1FFFFFFF
#define DP_SAWF_DEFINED_Q_PTID_MAX 2
#define DP_SAWF_DEFAULT_Q_PTID_MAX 2
#define DP_SAWF_TID_MAX 8
#define DP_SAWF_Q_MAX (DP_SAWF_DEFINED_Q_PTID_MAX * DP_SAWF_TID_MAX)
#define DP_SAWF_DEFAULT_Q_MAX (DP_SAWF_DEFAULT_Q_PTID_MAX * DP_SAWF_TID_MAX)
#define dp_sawf(peer, msduq_num, field) ((peer)->sawf->msduq[msduq_num].field)
#define DP_SAWF_DEFAULT_Q_INVALID 0xff
#define DP_SAWF_PEER_Q_INVALID 0xffff
#define DP_SAWF_INVALID_AST_IDX 0xffff

struct sawf_stats {
	struct sawf_delay_stats delay[DP_SAWF_MAX_TIDS][DP_SAWF_MAX_QUEUES];
	struct sawf_tx_stats tx_stats[DP_SAWF_MAX_TIDS][DP_SAWF_MAX_QUEUES];
	struct qdf_spinlock lock;
};

struct dp_peer_sawf_stats {
	struct sawf_stats stats;
};

struct sawf_def_queue_report {
	uint8_t svc_class_id;
};

/**
 * dp_sawf_def_queues_unmap_req - unmap peer to service class ID mapping
 * @soc: soc handle
 * @mac_addr: mac address
 * @svc_id: service class ID
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
dp_sawf_def_queues_unmap_req(struct cdp_soc_t *soc_hdl,
			     uint8_t *mac_addr,
			     uint8_t svc_id);

/**
 * dp_sawf_def_queues_get_map_report - get peer to sevice class ID mappings
 * @soc: soc handle
 * @mac_addr: mac address
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
dp_sawf_def_queues_get_map_report(struct cdp_soc_t *soc_hdl,
				  uint8_t *mac_addr);

/**
 * dp_sawf_def_queues_map_req - map peer to service class ID
 * @soc: soc handle
 * @mac_addr: mac address
 * @svc_clss_id: service class ID
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
dp_sawf_def_queues_map_req(struct cdp_soc_t *soc_hdl,
			   uint8_t *mac_addr, uint8_t svc_class_id);

/**
 * dp_peer_sawf_ctx_alloc - allocate SAWF ctx
 * @soc: soc handle
 * @peer: dp peer
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
dp_peer_sawf_ctx_alloc(struct dp_soc *soc,
		       struct dp_peer *peer);

/**
 * dp_peer_sawf_ctx_free - free SAWF ctx
 * @soc: soc handle
 * @peer: dp peer
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
dp_peer_sawf_ctx_free(struct dp_soc *soc,
		      struct dp_peer *peer);

/**
 * dp_peer_sawf_ctx_get - get SAWF ctx
 * @peer: dp peer
 *
 * Return: SAWF ctx on success; NULL otherwise
 */
struct dp_peer_sawf *dp_peer_sawf_ctx_get(struct dp_peer *peer);

/**
 * dp_peer_sawf_stats_ctx_alloc - allocate SAWF stats ctx
 * @soc: soc handle
 * @txrx_peer: DP txrx peer
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
dp_peer_sawf_stats_ctx_alloc(struct dp_soc *soc,
			     struct dp_txrx_peer *txrx_peer);

/**
 * dp_peer_sawf_stats_ctx_free - free SAWF stats ctx
 * @soc: soc handle
 * @txrx_peer: DP txrx peer
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
dp_peer_sawf_stats_ctx_free(struct dp_soc *soc,
			    struct dp_txrx_peer *txrx_peer);

/**
 * dp_peer_sawf_stats_ctx_free - free SAWF stats ctx
 * @txrx_peer: DP txrx peer
 *
 * Return: SAWF stas ctx on success, NULL otherwise
 */
struct dp_peer_sawf_stats *
dp_peer_sawf_stats_ctx_get(struct dp_txrx_peer *txrx_peer);

/**
 * dp_sawf_tx_compl_update_peer_stats - update SAWF stats in Tx completion
 * @soc: soc handle
 * @vdev: DP vdev context
 * @txrx_peer: DP txrx peer
 * @tx_desc: Tx descriptor
 * @ts: Tx completion status
 * @tid: TID
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
dp_sawf_tx_compl_update_peer_stats(struct dp_soc *soc,
				   struct dp_vdev *vdev,
				   struct dp_txrx_peer *txrx_peer,
				   struct dp_tx_desc_s *tx_desc,
				   struct hal_tx_completion_status *ts,
				   uint8_t tid);

/**
 * dp_sawf_tx_enqueue_fail_peer_stats - update SAWF stats in Tx enqueue failure
 * @soc: soc handle
 * @tx_desc: Tx descriptor
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
dp_sawf_tx_enqueue_fail_peer_stats(struct dp_soc *soc,
				   struct dp_tx_desc_s *tx_desc);

/**
 * dp_sawf_tx_enqueue_peer_stats - update SAWF stats in Tx enqueue
 * @soc: soc handle
 * @tx_desc: Tx descriptor
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
dp_sawf_tx_enqueue_peer_stats(struct dp_soc *soc,
			      struct dp_tx_desc_s *tx_desc);

#define DP_SAWF_STATS_SVC_CLASS_ID_ALL	0

/**
 * dp_sawf_dump_peer_stats - print peer stats
 * @txrx_peer: DP txrx peer
 *
 * Return: Success
 */
QDF_STATUS
dp_sawf_dump_peer_stats(struct dp_txrx_peer *txrx_peer);

/**
 * dp_sawf_get_peer_delay_stats - get delay stats
 * @soc: soc handle
 * @svc_id: service class ID
 * @mac: mac address
 * @data: data to be filled
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
dp_sawf_get_peer_delay_stats(struct cdp_soc_t *soc,
			     uint32_t svc_id, uint8_t *mac, void *data);

/**
 * dp_sawf_get_peer_tx_stats - get Tx stats
 * @soc: soc handle
 * @svc_id: service class ID
 * @mac: mac address
 * @data: data to be filled
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
dp_sawf_get_peer_tx_stats(struct cdp_soc_t *soc,
			  uint32_t svc_id, uint8_t *mac, void *data);

struct dp_sawf_msduq {
	uint8_t ref_count;
	uint8_t htt_msduq;
	uint8_t remapped_tid;
	bool is_used;
	bool del_in_progress;
	uint32_t tx_flow_number;
	uint32_t svc_id;
};

struct dp_sawf_msduq_tid_map {
	uint8_t host_queue_id;
};

struct dp_peer_sawf {
	/* qdf_bitmap queue_usage; */
	struct dp_sawf_msduq msduq[DP_SAWF_Q_MAX];
	struct dp_sawf_msduq_tid_map
	       msduq_map[DP_SAWF_TID_MAX][DP_SAWF_DEFINED_Q_PTID_MAX];
	struct sawf_def_queue_report tid_reports[DP_SAWF_TID_MAX];
};

uint16_t dp_sawf_get_msduq(struct net_device *netdev, uint8_t *peer_mac,
			   uint32_t service_id);
uint32_t dp_sawf_get_search_index(struct dp_soc *soc, qdf_nbuf_t nbuf,
				  uint8_t vdev_id, uint16_t queue_id);
uint32_t dp_sawf_queue_id_get(qdf_nbuf_t nbuf);
void dp_sawf_tcl_cmd(uint16_t *htt_tcl_metadata, qdf_nbuf_t nbuf);
bool dp_sawf_tag_valid_get(qdf_nbuf_t nbuf);

#endif /* DP_SAWF_H*/
