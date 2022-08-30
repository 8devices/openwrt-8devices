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

#include <dp_types.h>
#include <dp_peer.h>
#include <dp_internal.h>
#include <dp_htt.h>
#include <dp_sawf_htt.h>
#include <dp_sawf.h>
#include <dp_hist.h>
#include <hal_tx.h>
#include "hal_hw_headers.h"
#include "hal_api.h"
#include "hal_rx.h"
#include "qdf_trace.h"
#include "dp_tx.h"
#include "dp_peer.h"
#include "dp_internal.h"
#include "osif_private.h"
#include <wlan_objmgr_vdev_obj.h>

/**
 ** SAWF_metadata related information.
 **/
#define SAWF_VALID_TAG 0xAA
#define SAWF_TAG_SHIFT 0x18
#define SAWF_SERVICE_CLASS_SHIFT 0x10
#define SAWF_SERVICE_CLASS_MASK 0xff
#define SAWF_PEER_ID_SHIFT 0x6
#define SAWF_PEER_ID_MASK 0x3ff
#define SAWF_MSDUQ_MASK 0x3f

/**
 ** SAWF_metadata extraction.
 **/
#define SAWF_TAG_GET(x) ((x) >> SAWF_TAG_SHIFT)
#define SAWF_SERVICE_CLASS_GET(x) (((x) >> SAWF_SERVICE_CLASS_SHIFT) \
	& SAWF_SERVICE_CLASS_MASK)
#define SAWF_PEER_ID_GET(x) (((x) >> SAWF_PEER_ID_SHIFT) \
	& SAWF_PEER_ID_MASK)
#define SAWF_MSDUQ_GET(x) ((x) & SAWF_MSDUQ_MASK)
#define SAWF_TAG_IS_VALID(x) \
	((SAWF_TAG_GET(x) == SAWF_VALID_TAG) ? true : false)

#define DP_TX_TCL_METADATA_TYPE_SET(_var, _val) \
	HTT_TX_TCL_METADATA_TYPE_V2_SET(_var, _val)
#define DP_TCL_METADATA_TYPE_SVC_ID_BASED \
	HTT_TCL_METADATA_V2_TYPE_SVC_ID_BASED

uint16_t dp_sawf_msduq_peer_id_set(uint16_t peer_id, uint8_t msduq)
{
	uint16_t peer_msduq = 0;

	peer_msduq |= (peer_id & SAWF_PEER_ID_MASK) << SAWF_PEER_ID_SHIFT;
	peer_msduq |= (msduq & SAWF_MSDUQ_MASK);
	return peer_msduq;
}

bool dp_sawf_tag_valid_get(qdf_nbuf_t nbuf)
{
	if (SAWF_TAG_IS_VALID(qdf_nbuf_get_mark(nbuf)))
		return true;

	return false;
}

uint32_t dp_sawf_queue_id_get(qdf_nbuf_t nbuf)
{
	uint32_t mark = qdf_nbuf_get_mark(nbuf);
	uint8_t msduq = 0;

	msduq = SAWF_MSDUQ_GET(mark);

	if (!SAWF_TAG_IS_VALID(mark) || msduq == SAWF_MSDUQ_MASK)
		return DP_SAWF_DEFAULT_Q_INVALID;

	return msduq;
}

void dp_sawf_tcl_cmd(uint16_t *htt_tcl_metadata, qdf_nbuf_t nbuf)
{
	uint32_t mark = qdf_nbuf_get_mark(nbuf);
	uint16_t service_id = SAWF_SERVICE_CLASS_GET(mark);

	if (!SAWF_TAG_IS_VALID(mark))
		return;

	*htt_tcl_metadata = 0;
	DP_TX_TCL_METADATA_TYPE_SET(*htt_tcl_metadata,
				    DP_TCL_METADATA_TYPE_SVC_ID_BASED);
	HTT_TX_FLOW_METADATA_TID_OVERRIDE_SET(*htt_tcl_metadata, 1);
	HTT_TX_TCL_METADATA_SVC_CLASS_ID_SET(*htt_tcl_metadata, service_id);
}

QDF_STATUS
dp_peer_sawf_ctx_alloc(struct dp_soc *soc,
		       struct dp_peer *peer)
{
	struct dp_peer_sawf *sawf_ctx;

	sawf_ctx = qdf_mem_malloc(sizeof(struct dp_peer_sawf));
	if (!sawf_ctx) {
		qdf_err("Failed to allocate peer SAWF ctx");
		return QDF_STATUS_E_FAILURE;
	}

	peer->sawf = sawf_ctx;
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
dp_peer_sawf_ctx_free(struct dp_soc *soc,
		      struct dp_peer *peer)
{
	if (!peer->sawf) {
		qdf_err("Failed to free peer SAWF ctx");
		return QDF_STATUS_E_FAILURE;
	}

	if (peer->sawf)
		qdf_mem_free(peer->sawf);

	return QDF_STATUS_SUCCESS;
}

struct dp_peer_sawf *dp_peer_sawf_ctx_get(struct dp_peer *peer)
{
	struct dp_peer_sawf *sawf_ctx;

	sawf_ctx = peer->sawf;
	if (!sawf_ctx) {
		qdf_err("Failed to allocate peer SAWF ctx");
		return NULL;
	}

	return sawf_ctx;
}

QDF_STATUS
dp_sawf_def_queues_get_map_report(struct cdp_soc_t *soc_hdl,
				  uint8_t *mac_addr)
{
	struct dp_soc *dp_soc;
	struct dp_peer *peer;
	uint8_t tid;
	struct dp_peer_sawf *sawf_ctx;

	dp_soc = cdp_soc_t_to_dp_soc(soc_hdl);

	if (!dp_soc) {
		qdf_err("Invalid soc");
		return QDF_STATUS_E_INVAL;
	}

	peer = dp_peer_find_hash_find(dp_soc, mac_addr, 0,
				      DP_VDEV_ALL, DP_MOD_ID_CDP);
	if (!peer) {
		qdf_err("Invalid peer");
		return QDF_STATUS_E_FAILURE;
	}

	sawf_ctx = dp_peer_sawf_ctx_get(peer);
	if (!sawf_ctx) {
		qdf_err("Invalid SAWF ctx");
		dp_peer_unref_delete(peer, DP_MOD_ID_CDP);
		return QDF_STATUS_E_FAILURE;
	}

	qdf_info("Peer ", QDF_MAC_ADDR_FMT, QDF_MAC_ADDR_REF(mac_addr));
	qdf_nofl_info("TID\tService Class ID");
	for (tid = 0; tid < DP_SAWF_TID_MAX; ++tid) {
		if (sawf_ctx->tid_reports[tid].svc_class_id ==
				HTT_SAWF_SVC_CLASS_INVALID_ID) {
			continue;
		}
		qdf_nofl_info("%u\t%u", tid,
			      sawf_ctx->tid_reports[tid].svc_class_id);
	}

	dp_peer_unref_delete(peer, DP_MOD_ID_CDP);

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
dp_sawf_def_queues_map_req(struct cdp_soc_t *soc_hdl,
			   uint8_t *mac_addr, uint8_t svc_class_id)
{
	struct dp_soc *dp_soc;
	struct dp_peer *peer;
	uint16_t peer_id;
	QDF_STATUS status;

	dp_soc = cdp_soc_t_to_dp_soc(soc_hdl);

	if (!dp_soc) {
		qdf_err("Invalid soc");
		return QDF_STATUS_E_INVAL;
	}

	peer = dp_peer_find_hash_find(dp_soc, mac_addr, 0,
				      DP_VDEV_ALL, DP_MOD_ID_CDP);
	if (!peer) {
		qdf_err("Invalid peer");
		return QDF_STATUS_E_FAILURE;
	}

	peer_id = peer->peer_id;

	dp_peer_unref_delete(peer, DP_MOD_ID_CDP);

	qdf_info("peer " QDF_MAC_ADDR_FMT "svc id %u peer id %u",
		 QDF_MAC_ADDR_REF(mac_addr), svc_class_id, peer_id);

	status = dp_htt_h2t_sawf_def_queues_map_req(dp_soc->htt_handle,
						    svc_class_id, peer_id);

	if (status != QDF_STATUS_SUCCESS)
		return status;

	/*
	 * Request map repot conf from FW for all TIDs
	 */
	return dp_htt_h2t_sawf_def_queues_map_report_req(dp_soc->htt_handle,
							 peer_id, 0xff);
}

QDF_STATUS
dp_sawf_def_queues_unmap_req(struct cdp_soc_t *soc_hdl,
			     uint8_t *mac_addr, uint8_t svc_id)
{
	struct dp_soc *dp_soc;
	struct dp_peer *peer;
	uint16_t peer_id;
	QDF_STATUS status;
	uint8_t wildcard_mac[QDF_MAC_ADDR_SIZE] = {0xff, 0xff, 0xff,
		0xff, 0xff, 0xff};

	dp_soc = cdp_soc_t_to_dp_soc(soc_hdl);

	if (!dp_soc) {
		qdf_err("Invalid soc");
		return QDF_STATUS_E_INVAL;
	}

	if (!qdf_mem_cmp(mac_addr, wildcard_mac, QDF_MAC_ADDR_SIZE)) {
		/* wildcard unmap */
		peer_id = HTT_H2T_SAWF_DEF_QUEUES_UNMAP_PEER_ID_WILDCARD;
	} else {
		peer = dp_peer_find_hash_find(dp_soc, mac_addr, 0,
					      DP_VDEV_ALL, DP_MOD_ID_CDP);
		if (!peer) {
			qdf_err("Invalid peer");
			return QDF_STATUS_E_FAILURE;
		}
		peer_id = peer->peer_id;
		dp_peer_unref_delete(peer, DP_MOD_ID_CDP);
	}

	qdf_info("peer " QDF_MAC_ADDR_FMT "svc id %u peer id %u",
		 QDF_MAC_ADDR_REF(mac_addr), svc_id, peer_id);

	status =  dp_htt_h2t_sawf_def_queues_unmap_req(dp_soc->htt_handle,
						       svc_id, peer_id);

	if (status != QDF_STATUS_SUCCESS)
		return status;

	/*
	 * Request map repot conf from FW for all TIDs
	 */
	return dp_htt_h2t_sawf_def_queues_map_report_req(dp_soc->htt_handle,
							 peer_id, 0xff);
}

QDF_STATUS
dp_peer_sawf_stats_ctx_alloc(struct dp_soc *soc,
			     struct dp_txrx_peer *txrx_peer)
{
	struct dp_peer_sawf_stats *ctx;
	struct sawf_stats *stats;
	uint8_t tid, q_idx;

	ctx = qdf_mem_malloc(sizeof(struct dp_peer_sawf_stats));
	if (!ctx) {
		qdf_err("Failed to allocate peer SAWF stats");
		return QDF_STATUS_E_FAILURE;
	}

	txrx_peer->sawf_stats = ctx;
	stats = &ctx->stats;

	/* Initialize delay stats hist */
	for (tid = 0; tid < DP_SAWF_MAX_TIDS; tid++) {
		for (q_idx = 0; q_idx < DP_SAWF_MAX_QUEUES; q_idx++) {
			dp_hist_init(&stats->delay[tid][q_idx].delay_hist,
				     CDP_HIST_TYPE_HW_COMP_DELAY);
		}
	}

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
dp_peer_sawf_stats_ctx_free(struct dp_soc *soc,
			    struct dp_txrx_peer *txrx_peer)
{
	if (!txrx_peer->sawf_stats) {
		qdf_err("Failed to free peer SAWF stats");
		return QDF_STATUS_E_FAILURE;
	}

	qdf_mem_free(txrx_peer->sawf_stats);
	txrx_peer->sawf_stats = NULL;

	return QDF_STATUS_SUCCESS;
}

struct dp_peer_sawf_stats *
dp_peer_sawf_stats_ctx_get(struct dp_txrx_peer *txrx_peer)
{
	struct dp_peer_sawf_stats *sawf_stats;

	sawf_stats = txrx_peer->sawf_stats;
	if (!sawf_stats) {
		qdf_err("Failed to get SAWF stats ctx");
		return NULL;
	}

	return sawf_stats;
}

static int
dp_sawf_msduq_delay_window_switch(struct sawf_delay_stats *tx_delay)
{
	uint8_t cur_win;

	cur_win = tx_delay->cur_win;

	if (cur_win < DP_SAWF_NUM_AVG_WINDOWS) {
		if (tx_delay->win_avgs[cur_win].count >=
			MSDU_QUEUE_LATENCY_WIN_MIN_SAMPLES) {
			cur_win++;
			if (cur_win == DP_SAWF_NUM_AVG_WINDOWS) {
				cur_win = 0;
			}
			tx_delay->cur_win = cur_win;
			tx_delay->win_avgs[cur_win].sum = 0;
			tx_delay->win_avgs[cur_win].count = 0;
		}
	}

	return cur_win;
}

static QDF_STATUS
dp_sawf_compute_tx_delay(struct dp_tx_desc_s *tx_desc,
			 uint32_t *delay)
{
	int64_t current_timestamp, timestamp_hw_enqueue;

	timestamp_hw_enqueue = tx_desc->timestamp;
	current_timestamp = qdf_ktime_to_ms(qdf_ktime_real_get());

	if (timestamp_hw_enqueue == 0)
		return QDF_STATUS_E_FAILURE;

	*delay = (uint32_t)(current_timestamp -
			    timestamp_hw_enqueue);

	return QDF_STATUS_SUCCESS;
}

static QDF_STATUS
dp_sawf_compute_tx_hw_delay_us(struct dp_soc *soc,
			       struct dp_vdev *vdev,
			       struct hal_tx_completion_status *ts,
			       uint32_t *delay_us)
{
	int32_t buffer_ts;
	int32_t delta_tsf;
	int32_t delay;

	/* Tx_rate_stats_info_valid is 0 and tsf is invalid then */
	if (!ts->valid) {
		qdf_info("Invalid Tx rate stats info");
		return QDF_STATUS_E_FAILURE;
	}

	if (!vdev) {
		qdf_err("vdev is null");
		return QDF_STATUS_E_INVAL;
	}

	delta_tsf = vdev->delta_tsf;

	/* buffer_timestamp is in units of 1024 us and is [31:13] of
	 * WBM_RELEASE_RING_4. After left shift 10 bits, it's
	 * valid up to 29 bits.
	 */
	buffer_ts = ts->buffer_timestamp << WLAN_TX_DELAY_UNITS_US;

	delay = ts->tsf - buffer_ts - delta_tsf;
	/* mask 29 BITS */
	delay &= WLAN_TX_DELAY_MASK;

	if (delay > 0x1000000) {
		return QDF_STATUS_E_FAILURE;
	}

	if (delay_us)
		*delay_us = delay;

	return QDF_STATUS_SUCCESS;
}

static QDF_STATUS
dp_sawf_update_tx_delay(struct dp_soc *soc,
			struct dp_vdev *vdev,
			struct hal_tx_completion_status *ts,
			struct dp_tx_desc_s *tx_desc,
			struct sawf_stats *stats,
			uint8_t tid,
			uint8_t msduq_idx)
{
	struct sawf_delay_stats *tx_delay;
	uint32_t hw_delay;
	uint8_t cur_win;

	if (QDF_IS_STATUS_ERROR(dp_sawf_compute_tx_delay(tx_desc, &hw_delay)))
		return QDF_STATUS_E_FAILURE;

	tx_delay = &stats->delay[tid][msduq_idx];

	/* Update hist */
	dp_hist_update_stats(&tx_delay->delay_hist, hw_delay);

	/* Update total average */
	tx_delay->avg.sum += hw_delay;
	tx_delay->avg.count++;

	/* Update window average. Switch window if needed */
	cur_win = dp_sawf_msduq_delay_window_switch(tx_delay);

	if (cur_win >= DP_SAWF_NUM_AVG_WINDOWS) {
		qdf_info("Invalid Current Window");
		return QDF_STATUS_E_FAILURE;
	}

	tx_delay->win_avgs[cur_win].sum += hw_delay;
	tx_delay->win_avgs[cur_win].count++;

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
dp_sawf_get_msduq_map_info(struct dp_soc *soc, uint16_t peer_id,
			   uint8_t host_q_idx,
			   uint8_t *remaped_tid, uint8_t *target_q_idx)
{
	struct dp_peer *peer;
	struct dp_peer_sawf *sawf_ctx;
	struct dp_sawf_msduq *msduq;
	uint8_t tid, q_idx;
	uint8_t mdsuq_index = 0;

	mdsuq_index = host_q_idx - DP_SAWF_DEFAULT_Q_MAX;

	if (mdsuq_index >= DP_SAWF_Q_MAX) {
		qdf_err("Invalid Host Queue Index");
		return QDF_STATUS_E_FAILURE;
	}

	peer = dp_peer_get_ref_by_id(soc, peer_id, DP_MOD_ID_CDP);
	if (!peer) {
		return QDF_STATUS_E_FAILURE;
	}

	sawf_ctx = dp_peer_sawf_ctx_get(peer);
	if (!sawf_ctx) {
		qdf_err("ctx doesn't exist");
		dp_peer_unref_delete(peer, DP_MOD_ID_CDP);
		return QDF_STATUS_E_FAILURE;
	}

	msduq = &sawf_ctx->msduq[mdsuq_index];
	/*
	 * Find tid and msdu queue idx from host msdu queue number
	 * host idx to be taken from the tx descriptor
	 */
	tid = msduq->remapped_tid;
	q_idx = msduq->htt_msduq;

	if (remaped_tid)
		*remaped_tid = tid;

	if (target_q_idx)
		*target_q_idx = q_idx;

	dp_peer_unref_delete(peer, DP_MOD_ID_CDP);

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
dp_sawf_tx_compl_update_peer_stats(struct dp_soc *soc,
				   struct dp_vdev *vdev,
				   struct dp_txrx_peer *txrx_peer,
				   struct dp_tx_desc_s *tx_desc,
				   struct hal_tx_completion_status *ts,
				   uint8_t host_tid)
{
	struct dp_peer_sawf_stats *sawf_ctx;
	struct sawf_tx_stats *tx_stats;
	uint8_t tid, msduq_idx, host_msduq_idx;
	uint16_t peer_id;
	qdf_size_t length;
	QDF_STATUS status;

	if (!dp_sawf_tag_valid_get(tx_desc->nbuf))
		return QDF_STATUS_E_INVAL;

	if (!ts || !ts->valid)
		return QDF_STATUS_E_INVAL;

	sawf_ctx = dp_peer_sawf_stats_ctx_get(txrx_peer);

	if (!sawf_ctx) {
		qdf_err("Invalid SAWF stats ctx");
		return QDF_STATUS_E_FAILURE;
	}

	peer_id = SAWF_PEER_ID_GET(qdf_nbuf_get_mark(tx_desc->nbuf));
	/*
	 * Find remaped tid and target msdu queue idx to fill the stats
	 */
	host_msduq_idx = dp_sawf_queue_id_get(tx_desc->nbuf);

	if (host_msduq_idx == DP_SAWF_DEFAULT_Q_INVALID)
		return QDF_STATUS_E_FAILURE;

	status = dp_sawf_get_msduq_map_info(soc, peer_id,
					    host_msduq_idx,
					    &tid, &msduq_idx);
	if (status != QDF_STATUS_SUCCESS) {
		qdf_err("unable to find tid and msduq idx");
		return status;
	}

	qdf_assert(tid < DP_SAWF_MAX_TIDS);
	qdf_assert(msduq_idx < DP_SAWF_MAX_QUEUES);

	status = dp_sawf_update_tx_delay(soc, vdev, ts, tx_desc,
					 &sawf_ctx->stats, tid, msduq_idx);

	if (QDF_IS_STATUS_ERROR(status))
		return QDF_STATUS_E_FAILURE;

	length = qdf_nbuf_len(tx_desc->nbuf);

	DP_STATS_INCC_PKT(sawf_ctx, tx_stats[tid][msduq_idx].tx_success, 1,
			  length, (ts->status == HAL_TX_TQM_RR_FRAME_ACKED));

	DP_STATS_INCC_PKT(sawf_ctx, tx_stats[tid][msduq_idx].dropped.fw_rem, 1,
			  length, (ts->status == HAL_TX_TQM_RR_REM_CMD_REM));

	DP_STATS_INCC(sawf_ctx, tx_stats[tid][msduq_idx].dropped.fw_rem_notx, 1,
		      (ts->status == HAL_TX_TQM_RR_REM_CMD_NOTX));

	DP_STATS_INCC(sawf_ctx, tx_stats[tid][msduq_idx].dropped.fw_rem_tx, 1,
		      (ts->status == HAL_TX_TQM_RR_REM_CMD_TX));

	DP_STATS_INCC(sawf_ctx, tx_stats[tid][msduq_idx].dropped.age_out, 1,
		      (ts->status == HAL_TX_TQM_RR_REM_CMD_AGED));

	DP_STATS_INCC(sawf_ctx, tx_stats[tid][msduq_idx].dropped.fw_reason1, 1,
		      (ts->status == HAL_TX_TQM_RR_FW_REASON1));

	DP_STATS_INCC(sawf_ctx, tx_stats[tid][msduq_idx].dropped.fw_reason2, 1,
		      (ts->status == HAL_TX_TQM_RR_FW_REASON2));

	DP_STATS_INCC(sawf_ctx, tx_stats[tid][msduq_idx].dropped.fw_reason3, 1,
		      (ts->status == HAL_TX_TQM_RR_FW_REASON3));

	tx_stats = &sawf_ctx->stats.tx_stats[tid][msduq_idx];

	tx_stats->tx_failed = tx_stats->dropped.fw_rem.num +
				tx_stats->dropped.fw_rem_notx +
				tx_stats->dropped.fw_rem_tx +
				tx_stats->dropped.age_out +
				tx_stats->dropped.fw_reason1 +
				tx_stats->dropped.fw_reason2 +
				tx_stats->dropped.fw_reason3;

	if (tx_stats->queue_depth > 0)
		tx_stats->queue_depth--;

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
dp_sawf_tx_enqueue_fail_peer_stats(struct dp_soc *soc,
				   struct dp_tx_desc_s *tx_desc)
{
	struct dp_peer_sawf_stats *sawf_ctx;
	struct sawf_tx_stats *tx_stats;
	struct dp_peer *peer;
	struct dp_txrx_peer *txrx_peer;
	uint8_t tid, msduq_idx, host_msduq_idx;
	uint16_t peer_id;
	QDF_STATUS status;

	peer_id = SAWF_PEER_ID_GET(qdf_nbuf_get_mark(tx_desc->nbuf));

	/*
	 * Find remaped tid and target msdu queue idx to fill the stats
	 */
	host_msduq_idx = dp_sawf_queue_id_get(tx_desc->nbuf);
	if (host_msduq_idx == DP_SAWF_DEFAULT_Q_INVALID)
		return QDF_STATUS_E_FAILURE;

	status = dp_sawf_get_msduq_map_info(soc, peer_id,
					    host_msduq_idx,
					    &tid, &msduq_idx);
	if (status != QDF_STATUS_SUCCESS) {
		qdf_err("unable to find tid and msduq idx");
		return status;
	}

	peer = dp_peer_get_ref_by_id(soc, peer_id, DP_MOD_ID_TX);
	if (!peer) {
		qdf_err("Invalid peer_id %u", peer_id);
		return QDF_STATUS_E_FAILURE;
	}

	txrx_peer = dp_get_txrx_peer(peer);
	if (!txrx_peer) {
		qdf_err("Invalid peer_id %u", peer_id);
		goto fail;
	}

	sawf_ctx = dp_peer_sawf_stats_ctx_get(txrx_peer);
	if (!sawf_ctx) {
		qdf_err("Invalid SAWF stats ctx");
		goto fail;
	}

	tx_stats = &sawf_ctx->stats.tx_stats[tid][msduq_idx];
	tx_stats->queue_depth--;

	dp_peer_unref_delete(peer, DP_MOD_ID_TX);

	return QDF_STATUS_SUCCESS;
fail:
	dp_peer_unref_delete(peer, DP_MOD_ID_TX);
	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS
dp_sawf_tx_enqueue_peer_stats(struct dp_soc *soc,
			      struct dp_tx_desc_s *tx_desc)
{
	struct dp_peer_sawf_stats *sawf_ctx;
	struct sawf_tx_stats *tx_stats;
	struct dp_peer *peer;
	struct dp_txrx_peer *txrx_peer;
	uint8_t tid, msduq_idx, host_msduq_idx;
	uint16_t peer_id;
	QDF_STATUS status;

	peer_id = SAWF_PEER_ID_GET(qdf_nbuf_get_mark(tx_desc->nbuf));

	/*
	 * Find remaped tid and target msdu queue idx to fill the stats
	 */
	host_msduq_idx = dp_sawf_queue_id_get(tx_desc->nbuf);
	if (host_msduq_idx == DP_SAWF_DEFAULT_Q_INVALID)
		return QDF_STATUS_E_FAILURE;

	status = dp_sawf_get_msduq_map_info(soc, peer_id,
					    host_msduq_idx,
					    &tid, &msduq_idx);
	if (status != QDF_STATUS_SUCCESS) {
		qdf_err("unable to find tid and msduq idx: host_queue %d",
			host_msduq_idx);
		return status;
	}

	peer = dp_peer_get_ref_by_id(soc, peer_id, DP_MOD_ID_TX);
	if (!peer) {
		qdf_err("Invalid peer_id %u", peer_id);
		return QDF_STATUS_E_FAILURE;
	}

	txrx_peer = dp_get_txrx_peer(peer);
	if (!txrx_peer) {
		qdf_err("Invalid peer_id %u", peer_id);
		goto fail;
	}

	sawf_ctx = dp_peer_sawf_stats_ctx_get(txrx_peer);
	if (!sawf_ctx) {
		qdf_err("Invalid SAWF stats ctx");
		goto fail;
	}

	tx_stats = &sawf_ctx->stats.tx_stats[tid][msduq_idx];
	tx_stats->queue_depth++;
	tx_desc->timestamp = qdf_ktime_to_ms(qdf_ktime_real_get());

	dp_peer_unref_delete(peer, DP_MOD_ID_TX);

	return QDF_STATUS_SUCCESS;
fail:
	dp_peer_unref_delete(peer, DP_MOD_ID_TX);
	return QDF_STATUS_E_FAILURE;
}

const char *hw_delay_bucket[CDP_DELAY_BUCKET_MAX + 1] = {
	"0 to 10 ms", "11 to 20 ms",
	"21 to 30 ms", "31 to 40 ms",
	"41 to 50 ms", "51 to 60 ms",
	"61 to 70 ms", "71 to 80 ms",
	"81 to 90 ms", "91 to 100 ms",
	"101 to 250 ms", "251 to 500 ms", "500+ ms"
};

static inline const char *dp_sawf_str_hw_delay(uint8_t index)
{
	if (index > CDP_DELAY_BUCKET_MAX)
		return "Invalid index";

	return hw_delay_bucket[index];
}

static void dp_sawf_dump_delay_stats(struct sawf_delay_stats *stats)
{
	uint8_t idx;

	/* Average */
	DP_PRINT_STATS("Sum = %u Samples = %u",
		       stats->avg.sum, stats->avg.count);

	/* Window average */
	for (idx = 0; idx < DP_SAWF_NUM_AVG_WINDOWS; idx++) {
		DP_PRINT_STATS("Window = %u Sum = %u Samples = %u",
			       idx,
			       stats->win_avgs[idx].sum,
			       stats->win_avgs[idx].count);
	}

	/* CDP hist min, max and weighted average */
	DP_PRINT_STATS("Min  = %d Max  = %d Avg  = %d",
		       stats->delay_hist.min,
		       stats->delay_hist.max,
		       stats->delay_hist.avg);

	/* CDP hist bucket frequency */
	for (idx = 0; idx < CDP_HIST_BUCKET_MAX; idx++) {
		DP_PRINT_STATS("%s:  Packets = %llu",
			       dp_sawf_str_hw_delay(idx),
			       stats->delay_hist.hist.freq[idx]);
	}
}

static void dp_sawf_dump_tx_stats(struct sawf_tx_stats *tx_stats)
{
	DP_PRINT_STATS("tx_success: num = %u bytes = %lu",
		       tx_stats->tx_success.num,
		       tx_stats->tx_success.bytes);
	DP_PRINT_STATS("dropped: fw_rem num = %u bytes = %lu",
		       tx_stats->dropped.fw_rem.num,
		       tx_stats->dropped.fw_rem.bytes);
	DP_PRINT_STATS("dropped: fw_rem_notx = %u",
		       tx_stats->dropped.fw_rem_notx);
	DP_PRINT_STATS("dropped: fw_rem_tx = %u",
		       tx_stats->dropped.age_out);
	DP_PRINT_STATS("dropped: fw_reason1 = %u",
		       tx_stats->dropped.fw_reason1);
	DP_PRINT_STATS("dropped: fw_reason2 = %u",
		       tx_stats->dropped.fw_reason2);
	DP_PRINT_STATS("dropped: fw_reason3 = %u",
		       tx_stats->dropped.fw_reason3);
	DP_PRINT_STATS("tx_failed = %u", tx_stats->tx_failed);
	DP_PRINT_STATS("queue_depth = %u", tx_stats->queue_depth);
}

QDF_STATUS
dp_sawf_dump_peer_stats(struct dp_txrx_peer *txrx_peer)
{
	struct dp_peer_sawf_stats *ctx;
	uint8_t tid, q_idx;
	struct sawf_tx_stats *tx_stats;

	ctx = dp_peer_sawf_stats_ctx_get(txrx_peer);
	if (!ctx) {
		qdf_err("Invalid SAWF stats ctx");
		return QDF_STATUS_E_FAILURE;
	}

	for (tid = 0; tid < DP_SAWF_MAX_TIDS; tid++) {
		for (q_idx = 0; q_idx < DP_SAWF_MAX_QUEUES; q_idx++) {
			tx_stats = &ctx->stats.tx_stats[tid][q_idx];

			if (!tx_stats->tx_success.num && !tx_stats->tx_failed)
				continue;

			DP_PRINT_STATS("----TID: %u MSDUQ: %u ----",
				       tid, q_idx);
			DP_PRINT_STATS("Delay Stats:");
			dp_sawf_dump_delay_stats(&ctx->stats.delay[tid][q_idx]);
			DP_PRINT_STATS("Tx Stats:");
			dp_sawf_dump_tx_stats(&ctx->stats.tx_stats[tid][q_idx]);
		}
	}

	return QDF_STATUS_SUCCESS;
}

static QDF_STATUS
dp_sawf_find_msdu_from_svc_id(struct dp_peer_sawf *ctx, uint8_t svc_id,
			      uint8_t *tid, uint8_t *q_idx)
{
	uint8_t index = 0;

	for (index = 0; index < DP_SAWF_Q_MAX; index++) {
		if (ctx->msduq[index].svc_id == svc_id) {
			*tid = ctx->msduq[index].remapped_tid;
			*q_idx = ctx->msduq[index].htt_msduq;
			return QDF_STATUS_SUCCESS;
		}
	}

	return QDF_STATUS_E_FAILURE;
}

static void
dp_sawf_copy_delay_stats(struct sawf_delay_stats *dst,
			 struct sawf_delay_stats *src)
{
	int win;

	dst->avg.sum = src->avg.sum;
	dst->avg.count = src->avg.count;

	for (win = 0; win < DP_SAWF_NUM_AVG_WINDOWS; win++) {
		dst->win_avgs[win].sum = src->win_avgs[win].sum;
		dst->win_avgs[win].count = src->win_avgs[win].count;
	}

	dp_copy_hist_stats(&src->delay_hist, &dst->delay_hist);
}

static void
dp_sawf_copy_tx_stats(struct sawf_tx_stats *dst, struct sawf_tx_stats *src)
{
	dst->tx_success.num = src->tx_success.num;
	dst->tx_success.bytes = src->tx_success.bytes;

	dst->dropped.fw_rem.num = src->dropped.fw_rem.num;
	dst->dropped.fw_rem.bytes = src->dropped.fw_rem.bytes;
	dst->dropped.fw_rem_notx = src->dropped.fw_rem_notx;
	dst->dropped.fw_rem_tx = src->dropped.fw_rem_tx;
	dst->dropped.age_out = src->dropped.age_out;
	dst->dropped.fw_reason1 = src->dropped.fw_reason1;
	dst->dropped.fw_reason2 = src->dropped.fw_reason2;
	dst->dropped.fw_reason3 = src->dropped.fw_reason3;

	dst->tx_failed = src->tx_failed;
	dst->queue_depth = src->queue_depth;
}

QDF_STATUS
dp_sawf_get_peer_delay_stats(struct cdp_soc_t *soc,
			     uint32_t svc_id, uint8_t *mac, void *data)
{
	struct dp_soc *dp_soc;
	struct dp_peer *peer;
	struct dp_txrx_peer *txrx_peer;
	struct dp_peer_sawf *sawf_ctx;
	struct dp_peer_sawf_stats *stats_ctx;
	struct sawf_delay_stats *stats, *dst, *src;
	uint8_t tid, q_idx;
	QDF_STATUS status;

	stats = (struct sawf_delay_stats *)data;
	if (!stats) {
		qdf_err("Invalid data to fill");
		return QDF_STATUS_E_FAILURE;
	}

	dp_soc = cdp_soc_t_to_dp_soc(soc);
	if (!dp_soc) {
		qdf_err("Invalid soc");
		return QDF_STATUS_E_FAILURE;
	}

	peer = dp_peer_find_hash_find(dp_soc, mac, 0,
				      DP_VDEV_ALL, DP_MOD_ID_CDP);
	if (!peer) {
		qdf_err("Invalid peer");
		return QDF_STATUS_E_INVAL;
	}

	txrx_peer = dp_get_txrx_peer(peer);
	if (!txrx_peer) {
		qdf_err("txrx peer is NULL");
		goto fail;
	}

	stats_ctx = dp_peer_sawf_stats_ctx_get(txrx_peer);
	if (!stats_ctx) {
		qdf_err("stats ctx doesn't exist");
		goto fail;
	}

	dst = stats;
	if (svc_id == DP_SAWF_STATS_SVC_CLASS_ID_ALL) {
		for (tid = 0; tid < DP_SAWF_MAX_TIDS; tid++) {
			for (q_idx = 0; q_idx < DP_SAWF_MAX_QUEUES; q_idx++) {
				src = &stats_ctx->stats.delay[tid][q_idx];
				dp_sawf_copy_delay_stats(dst, src);
				dst++;
			}
		}
	} else {
		sawf_ctx = dp_peer_sawf_ctx_get(peer);
		if (!sawf_ctx) {
			qdf_err("stats_ctx doesn't exist");
			goto fail;
		}

		/*
		 * Find msduqs of the peer from service class ID
		 */
		status = dp_sawf_find_msdu_from_svc_id(sawf_ctx, svc_id,
						       &tid, &q_idx);
		if (QDF_IS_STATUS_ERROR(status)) {
			qdf_err("No MSDU Queue found for svc id %u", svc_id);
			goto fail;
		}

		src = &stats_ctx->stats.delay[tid][q_idx];
		dp_sawf_copy_delay_stats(dst, src);
	}

	dp_sawf_dump_peer_stats(txrx_peer);

	dp_peer_unref_delete(peer, DP_MOD_ID_CDP);

	return QDF_STATUS_SUCCESS;
fail:
	dp_peer_unref_delete(peer, DP_MOD_ID_CDP);
	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS
dp_sawf_get_peer_tx_stats(struct cdp_soc_t *soc,
			  uint32_t svc_id, uint8_t *mac, void *data)
{
	struct dp_soc *dp_soc;
	struct dp_peer *peer;
	struct dp_txrx_peer *txrx_peer;
	struct dp_peer_sawf *sawf_ctx;
	struct dp_peer_sawf_stats *stats_ctx;
	struct sawf_tx_stats *stats, *dst, *src;
	uint8_t tid, q_idx;
	QDF_STATUS status;

	stats = (struct sawf_tx_stats *)data;
	if (!stats) {
		qdf_err("Invalid data to fill");
		return QDF_STATUS_E_FAILURE;
	}

	dp_soc = cdp_soc_t_to_dp_soc(soc);
	if (!dp_soc) {
		qdf_err("Invalid soc");
		return QDF_STATUS_E_FAILURE;
	}

	peer = dp_peer_find_hash_find(dp_soc, mac, 0,
				      DP_VDEV_ALL, DP_MOD_ID_CDP);
	if (!peer) {
		qdf_err("Invalid peer");
		return QDF_STATUS_E_INVAL;
	}

	txrx_peer = dp_get_txrx_peer(peer);
	if (!txrx_peer) {
		qdf_err("txrx peer is NULL");
		goto fail;
	}

	stats_ctx = dp_peer_sawf_stats_ctx_get(txrx_peer);
	if (!stats_ctx) {
		qdf_err("stats ctx doesn't exist");
		goto fail;
	}

	dst = stats;
	if (svc_id == DP_SAWF_STATS_SVC_CLASS_ID_ALL) {
		for (tid = 0; tid < DP_SAWF_MAX_TIDS; tid++) {
			for (q_idx = 0; q_idx < DP_SAWF_MAX_QUEUES; q_idx++) {
				src = &stats_ctx->stats.tx_stats[tid][q_idx];
				dp_sawf_copy_tx_stats(dst, src);
				dst++;
			}
		}
	} else {
		sawf_ctx = dp_peer_sawf_ctx_get(peer);
		if (!sawf_ctx) {
			qdf_err("stats_ctx doesn't exist");
			goto fail;
		}

		/*
		 * Find msduqs of the peer from service class ID
		 */
		status = dp_sawf_find_msdu_from_svc_id(sawf_ctx, svc_id,
						       &tid, &q_idx);
		if (QDF_IS_STATUS_ERROR(status)) {
			qdf_err("No MSDU Queue found for svc id %u", svc_id);
			goto fail;
		}

		src = &stats_ctx->stats.tx_stats[tid][q_idx];
		dp_sawf_copy_tx_stats(dst, src);
	}

	dp_sawf_dump_peer_stats(txrx_peer);

	dp_peer_unref_delete(peer, DP_MOD_ID_CDP);

	return QDF_STATUS_SUCCESS;
fail:
	dp_peer_unref_delete(peer, DP_MOD_ID_CDP);
	return QDF_STATUS_E_FAILURE;
}

uint16_t dp_sawf_get_peerid(struct dp_soc *soc, uint8_t *dest_mac,
			    uint8_t vdev_id)
{
	struct dp_ast_entry *ast_entry = NULL;
	uint16_t peer_id;

	qdf_spin_lock_bh(&soc->ast_lock);
	ast_entry = dp_peer_ast_hash_find_by_vdevid(soc, dest_mac, vdev_id);

	if (!ast_entry) {
		qdf_spin_unlock_bh(&soc->ast_lock);
		qdf_warn("%s NULL ast entry");
		return HTT_INVALID_PEER;
	}

	peer_id = ast_entry->peer_id;
	qdf_spin_unlock_bh(&soc->ast_lock);
	return peer_id;
}

uint32_t dp_sawf_get_search_index(struct dp_soc *soc, qdf_nbuf_t nbuf,
				  uint8_t vdev_id, uint16_t queue_id)
{
	struct dp_peer *peer = NULL;
	uint32_t search_index = DP_SAWF_INVALID_AST_IDX;
	uint16_t peer_id = SAWF_PEER_ID_GET(qdf_nbuf_get_mark(nbuf));
	uint8_t index = queue_id / DP_SAWF_TID_MAX;

	peer = dp_peer_get_ref_by_id(soc, peer_id, DP_MOD_ID_SAWF);

	if (!peer || index >= DP_PEER_AST_FLOWQ_MAX) {
		qdf_warn("peer:%p index:%d", peer, index);
		return DP_SAWF_INVALID_AST_IDX;
	}

	search_index = peer->peer_ast_flowq_idx[index].ast_idx;
	dp_peer_unref_delete(peer, DP_MOD_ID_SAWF);

	return search_index;
}

uint16_t dp_sawf_get_msduq(struct net_device *netdev, uint8_t *dest_mac,
			   uint32_t service_id)
{
	osif_dev  *osdev;
	wlan_if_t vap;
	struct wlan_objmgr_vdev *vdev;
	uint8_t vdev_id;
	uint8_t i = 1;
	struct dp_peer *peer = NULL;
	struct dp_soc *soc = NULL;
	uint16_t peer_id;
	uint8_t q_id;

	if (!netdev->ieee80211_ptr) {
		qdf_debug("non vap netdevice");
		return DP_SAWF_PEER_Q_INVALID;
	}

	osdev = ath_netdev_priv(netdev);
	vap = osdev->os_if;
	vdev = osdev->ctrl_vdev;
	soc = (struct dp_soc *)wlan_psoc_get_dp_handle
	      (wlan_pdev_get_psoc(wlan_vdev_get_pdev(vdev)));

	vdev_id = wlan_vdev_get_id(vdev);

	peer_id = dp_sawf_get_peerid(soc, dest_mac, vdev_id);
	if (peer_id == HTT_INVALID_PEER)
		return DP_SAWF_PEER_Q_INVALID;

	peer = dp_peer_get_ref_by_id(soc, peer_id,
				     DP_MOD_ID_SAWF);

	if (!peer) {
		qdf_warn("NULL peer");
		return DP_SAWF_PEER_Q_INVALID;
	}

	/*
	 * First loop to go through all msdu queues of peer which
	 * have been used. If flow has same service id as that of
	 * used msdu queues, return used msdu queue.
	 */
	for (i = 0; i < DP_SAWF_Q_MAX; i++) {
		if ((dp_sawf(peer, i, is_used) == 1) &&
		    dp_sawf(peer, i, svc_id) == service_id) {
			dp_sawf(peer, i, ref_count)++;
			dp_peer_unref_delete(peer, DP_MOD_ID_SAWF);
			q_id = i + DP_SAWF_DEFAULT_Q_MAX;
			return dp_sawf_msduq_peer_id_set(peer_id, q_id);
		}
	}

	/*
	 * Second loop to go through all unused msdu queues of peer.
	 * Allot new msdu queue for new service class.
	 */
	for (i = 0; i < DP_SAWF_Q_MAX; i++) {
		if (dp_sawf(peer, i, is_used) == 0) {
			dp_sawf(peer, i, is_used) = 1;
			dp_sawf(peer, i, svc_id) = service_id;
			dp_sawf(peer, i, ref_count)++;
			dp_peer_unref_delete(peer, DP_MOD_ID_SAWF);
			q_id = i + DP_SAWF_DEFAULT_Q_MAX;
			return dp_sawf_msduq_peer_id_set(peer_id, q_id);
		}
	}

	dp_peer_unref_delete(peer, DP_MOD_ID_SAWF);

	/* request for more msdu queues. Return error*/
	return DP_SAWF_PEER_Q_INVALID;
}

qdf_export_symbol(dp_sawf_get_msduq);
