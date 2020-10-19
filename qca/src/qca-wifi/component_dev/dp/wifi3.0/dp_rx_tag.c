/*
 * Copyright (c) 2016-2020 The Linux Foundation. All rights reserved.
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

#include "hal_hw_headers.h"
#include "hal_api.h"
#include "hal_rx.h"
#include "qdf_trace.h"
#include "dp_tx.h"
#include "dp_peer.h"
#include "dp_internal.h"
#include "dp_rx_tag.h"

#if defined(WLAN_SUPPORT_RX_TAG_STATISTICS) && \
	defined(WLAN_SUPPORT_RX_PROTOCOL_TYPE_TAG)
/**
 * dp_rx_update_rx_protocol_tag_stats() - Increments the protocol tag stats
 *                                        for the given protocol type
 * @soc: core txrx main context
 * @pdev: TXRX pdev context for which stats should be incremented
 * @protocol_index: Protocol index for which the stats should be incremented
 * @ring_index: REO ring number from which this tag was received.
 *
 * Since HKv2 is a SMP, two or more cores may simultaneously receive packets
 * of same type, and hence attempt to increment counters for the same protocol
 * type at the same time. This creates the possibility of missing stats.
 *
 * For example,  when two or more CPUs have each read the old tag value, V,
 * for protocol type, P and each increment the value to V+1. Instead, the
 * operations should have been  sequenced to achieve a final value of V+2.
 *
 * In order to avoid this scenario,  we can either use locks or store stats
 * on a per-CPU basis. Since tagging happens in the core data path, locks
 * are not preferred. Instead, we use a per-ring counter, since each CPU
 * operates on a REO ring.
 *
 * Return: void
 */
void dp_rx_update_rx_protocol_tag_stats(struct dp_pdev *pdev,
					uint16_t protocol_index,
					uint16_t ring_index)
{
	if (ring_index >= MAX_REO_DEST_RINGS)
		return;

	pdev->reo_proto_tag_stats[ring_index][protocol_index].tag_ctr++;
}
#endif /* WLAN_SUPPORT_RX_TAG_STATISTICS */

#if defined(WLAN_SUPPORT_RX_TAG_STATISTICS) && \
	defined(WLAN_SUPPORT_RX_PROTOCOL_TYPE_TAG)
/**
 * dp_rx_update_rx_err_protocol_tag_stats() - Increments the protocol tag stats
 *                                        for the given protocol type
 *                                        received from exception ring
 * @soc: core txrx main context
 * @pdev: TXRX pdev context for which stats should be incremented
 * @protocol_index: Protocol index for which the stats should be incremented
 *
 * In HKv2, all exception packets are received on Ring-0 (along with normal
 * Rx). Hence tags are maintained separately for exception ring as well.
 *
 * Return: void
 */
void dp_rx_update_rx_err_protocol_tag_stats(struct dp_pdev *pdev,
					    uint16_t protocol_index)
{
	pdev->rx_err_proto_tag_stats[protocol_index].tag_ctr++;
}
#endif /* WLAN_SUPPORT_RX_TAG_STATISTICS */

/**
 * dp_rx_update_protocol_tag() - Reads CCE metadata from the RX MSDU end TLV
 *                              and set the corresponding tag in QDF packet
 * @soc: core txrx main context
 * @vdev: vdev on which the packet is received
 * @nbuf: QDF pkt buffer on which the protocol tag should be set
 * @rx_tlv_hdr: rBbase address where the RX TLVs starts
 * @ring_index: REO ring number, not used for error & monitor ring
 * @is_reo_exception: flag to indicate if rx from REO ring or exception ring
 * @is_update_stats: flag to indicate whether to update stats or not
 * Return: void
 */
#ifdef WLAN_SUPPORT_RX_PROTOCOL_TYPE_TAG

#ifdef DP_RX_MON_MEM_FRAG
/**
 * dp_rx_update_proto_tag() - Update protocol tag to nbuf cb or to headroom
 *
 * @nbuf: QDF pkt buffer on which the protocol tag should be set
 * @protocol_tag: Protocol tag
 */
static inline
void dp_rx_update_proto_tag(qdf_nbuf_t nbuf, uint16_t protocol_tag)
{
	uint8_t idx;
	uint8_t *nbuf_head = NULL;

	if (qdf_nbuf_get_nr_frags(nbuf)) {
		/* Get frag index, which was saved while restitch */
		idx = QDF_NBUF_CB_RX_CTX_ID(nbuf);
		nbuf_head = qdf_nbuf_head(nbuf);
		nbuf_head += (idx * DP_RX_MON_PF_TAG_SIZE);

		*((uint16_t *)nbuf_head) = protocol_tag;
	} else {
		qdf_nbuf_set_rx_protocol_tag(nbuf, protocol_tag);
	}
}
#else
static inline
void dp_rx_update_proto_tag(qdf_nbuf_t nbuf, uint16_t protocol_tag)
{
	qdf_nbuf_set_rx_protocol_tag(nbuf, protocol_tag);
}
#endif

void
dp_rx_update_protocol_tag(struct dp_soc *soc, struct dp_vdev *vdev,
			  qdf_nbuf_t nbuf, uint8_t *rx_tlv_hdr,
			  uint16_t ring_index,
			  bool is_reo_exception, bool is_update_stats)
{
	uint16_t cce_metadata = RX_PROTOCOL_TAG_START_OFFSET;
	bool     cce_match = false;
	struct   dp_pdev *pdev;
	uint16_t protocol_tag = 0;

	if (qdf_unlikely(!vdev))
		return;

	pdev = vdev->pdev;

	if (qdf_likely(!pdev->is_rx_protocol_tagging_enabled))
		return;

	/*
	 * In case of raw frames, rx_attention and rx_msdu_end tlv
	 * may be stale or invalid. Do not tag such frames.
	 * Default decap_type is set to ethernet for monitor vdev,
	 * therefore, cannot check decap_type for monitor mode.
	 * We will call this only for eth frames from dp_rx_mon_dest.c.
	 */
	if (qdf_likely(!(pdev->monitor_vdev && pdev->monitor_vdev == vdev) &&
		       (vdev->rx_decap_type !=  htt_cmn_pkt_type_ethernet)))
		return;

	/*
	 * Check whether HW has filled in the CCE metadata in
	 * this packet, if not filled, just return
	 */
	if (qdf_likely(!hal_rx_msdu_cce_match_get(rx_tlv_hdr)))
		return;

	cce_match = true;
	/* Get the cce_metadata from RX MSDU TLV */
	cce_metadata = hal_rx_msdu_cce_metadata_get(soc->hal_soc, rx_tlv_hdr);
	/*
	 * Received CCE metadata should be within the
	 * valid limits
	 */
	qdf_assert_always((cce_metadata >= RX_PROTOCOL_TAG_START_OFFSET) &&
			  (cce_metadata < (RX_PROTOCOL_TAG_START_OFFSET +
			   RX_PROTOCOL_TAG_MAX)));

	/*
	 * The CCE metadata received is just the
	 * packet_type + RX_PROTOCOL_TAG_START_OFFSET
	 */
	cce_metadata -= RX_PROTOCOL_TAG_START_OFFSET;

	/*
	 * Update the QDF packet with the user-specified
	 * tag/metadata by looking up tag value for
	 * received protocol type.
	 */
	protocol_tag = pdev->rx_proto_tag_map[cce_metadata].tag;

	dp_rx_update_proto_tag(nbuf, protocol_tag);
	QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_INFO_LOW,
		  "Seq:%u dcap:%u CCE Match:%u ProtoID:%u Tag:%u stats:%u",
		  hal_rx_get_rx_sequence(soc->hal_soc, rx_tlv_hdr),
		  vdev->rx_decap_type, cce_match, cce_metadata,
		  protocol_tag, is_update_stats);

	if (qdf_likely(!is_update_stats))
		return;

	if (qdf_unlikely(is_reo_exception)) {
		dp_rx_update_rx_err_protocol_tag_stats(pdev,
						       cce_metadata);
	} else {
		dp_rx_update_rx_protocol_tag_stats(pdev,
						   cce_metadata,
						   ring_index);
	}
}
#endif /* WLAN_SUPPORT_RX_PROTOCOL_TYPE_TAG */

/**
 * dp_rx_update_flow_tag() - Reads FSE metadata from the RX MSDU end TLV
 *                           and set the corresponding tag in QDF packet
 * @soc: core txrx main context
 * @vdev: vdev on which the packet is received
 * @nbuf: QDF pkt buffer on which the protocol tag should be set
 * @rx_tlv_hdr: base address where the RX TLVs starts
 * @is_update_stats: flag to indicate whether to update stats or not
 *
 * Return: void
 */
#ifdef WLAN_SUPPORT_RX_FLOW_TAG

#ifdef DP_RX_MON_MEM_FRAG
/**
 * dp_rx_update_flow_tags() - Update protocol tag to nbuf cb or to headroom
 *
 * @nbuf: QDF pkt buffer on which the protocol tag should be set
 * @flow_tag: Flow tag
 */
static inline
void dp_rx_update_flow_tags(qdf_nbuf_t nbuf, uint32_t flow_tag)
{
	uint8_t idx;
	uint8_t *nbuf_head = NULL;
	uint16_t updated_flow_tag = (uint16_t)(flow_tag & 0xFFFF);

	if (qdf_nbuf_get_nr_frags(nbuf)) {
		/* Get frag index, which was saved while restitch */
		idx = QDF_NBUF_CB_RX_CTX_ID(nbuf);
		nbuf_head = qdf_nbuf_head(nbuf);
		nbuf_head += ((idx * DP_RX_MON_PF_TAG_SIZE) +
				sizeof(uint16_t));

		*((uint16_t *)nbuf_head) = updated_flow_tag;
	} else {
		qdf_nbuf_set_rx_flow_tag(nbuf, flow_tag);
	}
}
#else
static inline
void dp_rx_update_flow_tags(qdf_nbuf_t nbuf, uint16_t flow_tag)
{
	qdf_nbuf_set_rx_flow_tag(nbuf, flow_tag);
}
#endif

void
dp_rx_update_flow_tag(struct dp_soc *soc, struct dp_vdev *vdev,
		      qdf_nbuf_t nbuf, uint8_t *rx_tlv_hdr, bool update_stats)
{
	bool flow_idx_invalid, flow_idx_timeout;
	uint32_t flow_idx, fse_metadata;
	struct dp_pdev *pdev;

	if (qdf_unlikely(!vdev))
		return;

	pdev = vdev->pdev;
	if (qdf_likely(!wlan_cfg_is_rx_flow_tag_enabled(soc->wlan_cfg_ctx)))
		return;

	QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_INFO_LOW,
		  "Seq:%u dcap:%u invalid:%u timeout:%u flow:%u tag:%u stat:%u",
		  hal_rx_get_rx_sequence(soc->hal_soc, rx_tlv_hdr),
		  vdev->rx_decap_type,
		  hal_rx_msdu_flow_idx_invalid(soc->hal_soc, rx_tlv_hdr),
		  hal_rx_msdu_flow_idx_timeout(soc->hal_soc, rx_tlv_hdr),
		  hal_rx_msdu_flow_idx_get(soc->hal_soc, rx_tlv_hdr),
		  hal_rx_msdu_fse_metadata_get(soc->hal_soc, rx_tlv_hdr),
		  update_stats);

	/**
	 * In case of raw frames, rx_msdu_end tlv may be stale or invalid.
	 * Do not tag such frames in normal REO path.
	 * Default decap_type is set to ethernet for monitor vdev currently,
	 * therefore, we will not check decap_type for monitor mode.
	 * We will call this only for eth frames from dp_rx_mon_dest.c.
	 */
	if (qdf_likely((vdev->rx_decap_type !=  htt_cmn_pkt_type_ethernet)))
		return;

	flow_idx_invalid = hal_rx_msdu_flow_idx_invalid(soc->hal_soc, rx_tlv_hdr);
	hal_rx_msdu_get_flow_params(soc->hal_soc, rx_tlv_hdr, &flow_idx_invalid,
				    &flow_idx_timeout, &flow_idx);
	if (qdf_unlikely(flow_idx_invalid))
		return;

	if (qdf_unlikely(flow_idx_timeout))
		return;

	/**
	 * Limit FSE metadata to 16 bit as we have allocated only
	 * 16 bits for flow_tag field in skb->cb
	 */
	fse_metadata = hal_rx_msdu_fse_metadata_get(soc->hal_soc, rx_tlv_hdr) & 0xFFFF;

	/* update the skb->cb with the user-specified tag/metadata */
	dp_rx_update_flow_tags(nbuf, fse_metadata);

	QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_INFO_LOW,
		  "Seq:%u dcap:%u invalid:%u timeout:%u flow:%u tag:%u stat:%u",
		  hal_rx_get_rx_sequence(soc->hal_soc, rx_tlv_hdr),
		  vdev->rx_decap_type, flow_idx_invalid, flow_idx_timeout,
		  flow_idx, fse_metadata, update_stats);

	if (qdf_likely(update_stats))
		dp_rx_update_rx_flow_tag_stats(pdev, flow_idx);
}
#endif /* WLAN_SUPPORT_RX_FLOW_TAG */

#if defined(WLAN_SUPPORT_RX_PROTOCOL_TYPE_TAG) ||\
	defined(WLAN_SUPPORT_RX_FLOW_TAG)
/**
 * dp_rx_mon_update_protocol_flow_tag() - Performs necessary checks for monitor
 *                                       mode and then tags appropriate packets
 * @soc: core txrx main context
 * @vdev: pdev on which packet is received
 * @msdu: QDF packet buffer on which the protocol tag should be set
 * @rx_desc: base address where the RX TLVs start
 * Return: void
 */
void dp_rx_mon_update_protocol_flow_tag(struct dp_soc *soc,
					struct dp_pdev *dp_pdev,
					qdf_nbuf_t msdu, void *rx_desc)
{
	uint32_t msdu_ppdu_id = 0;
	struct mon_rx_status *mon_recv_status;
	struct cdp_mon_status *rs;

	bool is_mon_protocol_flow_tag_enabled =
		wlan_cfg_is_rx_mon_protocol_flow_tag_enabled(soc->wlan_cfg_ctx);

	if (qdf_likely(!is_mon_protocol_flow_tag_enabled))
		return;

	if (qdf_likely(!dp_pdev->monitor_vdev))
		return;

	if (qdf_likely(1 != dp_pdev->ppdu_info.rx_status.rxpcu_filter_pass))
		return;

	rs = &dp_pdev->rx_mon_recv_status;
	if (rs->cdp_rs_rxdma_err)
		return;

	msdu_ppdu_id = hal_rx_get_ppdu_id(soc->hal_soc, rx_desc);

	if (msdu_ppdu_id != dp_pdev->ppdu_info.com_info.ppdu_id) {
		QDF_TRACE(QDF_MODULE_ID_DP,
			  QDF_TRACE_LEVEL_ERROR,
			  "msdu_ppdu_id=%x,com_info.ppdu_id=%x",
			  msdu_ppdu_id,
			  dp_pdev->ppdu_info.com_info.ppdu_id);
		return;
	}

	mon_recv_status = &dp_pdev->ppdu_info.rx_status;
	if (mon_recv_status->frame_control_info_valid &&
	    ((mon_recv_status->frame_control & IEEE80211_FC0_TYPE_MASK) ==
	      IEEE80211_FC0_TYPE_DATA)) {
		/*
		 * Update the protocol tag in SKB for packets received on BSS.
		 * Do not update tag stats since it would double actual
		 * received count.
		 */
		dp_rx_update_protocol_tag(soc,
					  dp_pdev->monitor_vdev,
					  msdu, rx_desc,
					  MAX_REO_DEST_RINGS,
					  false, false);
		/* Update the flow tag in SKB based on FSE metadata */
		dp_rx_update_flow_tag(soc, dp_pdev->monitor_vdev,
				      msdu, rx_desc, false);
	}
}
#endif /* WLAN_SUPPORT_RX_PROTOCOL_TYPE_TAG || WLAN_SUPPORT_RX_FLOW_TAG */

#ifdef WLAN_SUPPORT_RX_PROTOCOL_TYPE_TAG
#ifdef WLAN_SUPPORT_RX_TAG_STATISTICS
/**
 * dp_summarize_tag_stats - sums up the given protocol type's counters
 * across all the rings and dumps the same
 * @pdev: dp_pdev handle
 * @protocol_type: protocol type for which stats should be displayed
 *
 * Return: none
 */
uint64_t dp_summarize_tag_stats(struct dp_pdev *pdev,
				uint16_t protocol_type)
{
	uint8_t ring_idx;
	uint64_t total_tag_cnt = 0;

	for (ring_idx = 0; ring_idx < MAX_REO_DEST_RINGS; ring_idx++) {
		total_tag_cnt +=
		pdev->reo_proto_tag_stats[ring_idx][protocol_type].tag_ctr;
	}
	total_tag_cnt += pdev->rx_err_proto_tag_stats[protocol_type].tag_ctr;
	DP_PRINT_STATS("ProtoID: %d, Tag: %u Tagged MSDU cnt: %llu",
		       protocol_type,
		       pdev->rx_proto_tag_map[protocol_type].tag,
		       total_tag_cnt);
	return total_tag_cnt;
}

/**
 * dp_dump_pdev_rx_protocol_tag_stats - dump the number of packets tagged for
 * given protocol type (RX_PROTOCOL_TAG_ALL indicates for all protocol)
 * @soc: cdp_soc handle
 * @pdev_id: id of cdp_pdev handle
 * @protocol_type: protocol type for which stats should be displayed
 *
 * Return: none
 */
void
dp_dump_pdev_rx_protocol_tag_stats(struct cdp_soc_t  *soc, uint8_t pdev_id,
				   uint16_t protocol_type)
{
	uint16_t proto_idx;
	struct dp_pdev *pdev =
		dp_get_pdev_from_soc_pdev_id_wifi3((struct dp_soc *)soc,
						   pdev_id);

	if (!pdev)
		return;

	if (protocol_type != RX_PROTOCOL_TAG_ALL &&
	    protocol_type >= RX_PROTOCOL_TAG_MAX) {
		DP_PRINT_STATS("Invalid protocol type : %u", protocol_type);
		return;
	}

	/* protocol_type in [0 ... RX_PROTOCOL_TAG_MAX] */
	if (protocol_type != RX_PROTOCOL_TAG_ALL) {
		dp_summarize_tag_stats(pdev, protocol_type);
		return;
	}

	/* protocol_type == RX_PROTOCOL_TAG_ALL */
	for (proto_idx = 0; proto_idx < RX_PROTOCOL_TAG_MAX; proto_idx++)
		dp_summarize_tag_stats(pdev, proto_idx);
}
#endif /* WLAN_SUPPORT_RX_TAG_STATISTICS */

#ifdef WLAN_SUPPORT_RX_TAG_STATISTICS
static void
__dp_reset_pdev_rx_protocol_tag_stats(struct dp_pdev *pdev,
				      uint16_t protocol_type)
{
	uint8_t ring_idx;

	for (ring_idx = 0; ring_idx < MAX_REO_DEST_RINGS; ring_idx++)
		pdev->reo_proto_tag_stats[ring_idx][protocol_type].tag_ctr = 0;
	pdev->rx_err_proto_tag_stats[protocol_type].tag_ctr = 0;
}

/**
 * dp_reset_pdev_rx_protocol_tag_stats - resets the stats counters for
 * given protocol type
 * @soc: soc handle
 * @pdev_id: id of cdp_pdev handle
 * @protocol_type: protocol type for which stats should be reset
 *
 * Return: none
 */
static void
dp_reset_pdev_rx_protocol_tag_stats(struct cdp_soc_t  *soc, uint8_t pdev_id,
				    uint16_t protocol_type)
{
	struct dp_pdev *pdev =
		dp_get_pdev_from_soc_pdev_id_wifi3((struct dp_soc *)soc,
						   pdev_id);
	if (!pdev)
		return;

	__dp_reset_pdev_rx_protocol_tag_stats(pdev, protocol_type);
}
#endif /* WLAN_SUPPORT_RX_TAG_STATISTICS */

/**
 * dp_update_pdev_rx_protocol_tag - Add/remove a protocol tag that should be
 * applied to the desired protocol type packets
 * @soc: soc handle
 * @pdev_id: id of cdp_pdev handle
 * @enable_rx_protocol_tag - bitmask that indicates what protocol types
 * are enabled for tagging. zero indicates disable feature, non-zero indicates
 * enable feature
 * @protocol_type: new protocol type for which the tag is being added
 * @tag: user configured tag for the new protocol
 *
 * Return: Success
 */
QDF_STATUS
dp_update_pdev_rx_protocol_tag(struct cdp_soc_t  *soc, uint8_t pdev_id,
			       uint32_t enable_rx_protocol_tag,
			       uint16_t protocol_type,
			       uint16_t tag)
{
	struct dp_pdev *pdev =
		dp_get_pdev_from_soc_pdev_id_wifi3((struct dp_soc *)soc,
						   pdev_id);
	if (!pdev)
		return QDF_STATUS_E_FAILURE;

	/*
	 * dynamically enable/disable tagging based on enable_rx_protocol_tag
	 * flag.
	 */
	if (enable_rx_protocol_tag) {
		/* Tagging for one or more protocols has been set by user */
		pdev->is_rx_protocol_tagging_enabled = true;
	} else {
		/*
		 * No protocols being tagged, disable feature till next add
		 * operation
		 */
		pdev->is_rx_protocol_tagging_enabled = false;
	}

	/** Reset stats counter across all rings for given protocol */
	__dp_reset_pdev_rx_protocol_tag_stats(pdev, protocol_type);

	pdev->rx_proto_tag_map[protocol_type].tag = tag;

	return QDF_STATUS_SUCCESS;
}
#endif /* WLAN_SUPPORT_RX_PROTOCOL_TYPE_TAG */

#ifdef WLAN_SUPPORT_RX_FLOW_TAG
/**
 * dp_set_rx_flow_tag - add/delete a flow
 * @soc: soc handle
 * @pdev_id: id of cdp_pdev handle
 * @flow_info: flow tuple that is to be added to/deleted from flow search table
 *
 * Return: Success
 */
QDF_STATUS
dp_set_rx_flow_tag(struct cdp_soc_t *soc, uint8_t pdev_id,
		   struct cdp_rx_flow_info *flow_info)
{
	struct dp_pdev *pdev =
		dp_get_pdev_from_soc_pdev_id_wifi3((struct dp_soc *)soc,
						   pdev_id);
	struct wlan_cfg_dp_soc_ctxt *cfg;

	if (qdf_unlikely(!pdev))
		return QDF_STATUS_E_FAILURE;

	cfg = pdev->soc->wlan_cfg_ctx;

	if (qdf_unlikely(!wlan_cfg_is_rx_flow_tag_enabled(cfg))) {
		dp_err("RX Flow tag feature disabled");
		return QDF_STATUS_E_NOSUPPORT;
	}

	if (flow_info->op_code == CDP_FLOW_FST_ENTRY_ADD)
		return dp_rx_flow_add_entry(pdev, flow_info);
	if (flow_info->op_code == CDP_FLOW_FST_ENTRY_DEL)
		return dp_rx_flow_delete_entry(pdev, flow_info);

	return QDF_STATUS_E_INVAL;
}

/**
 * dp_dump_rx_flow_tag_stats - dump the number of packets tagged for
 * given flow 5-tuple
 * @cdp_soc: soc handle
 * @pdev_id: id of cdp_pdev handle
 * @flow_info: flow 5-tuple for which stats should be displayed
 *
 * Return: Success
 */
QDF_STATUS
dp_dump_rx_flow_tag_stats(struct cdp_soc_t *soc, uint8_t pdev_id,
			  struct cdp_rx_flow_info *flow_info)
{
	QDF_STATUS status;
	struct cdp_flow_stats stats;
	struct wlan_cfg_dp_soc_ctxt *cfg;
	struct dp_pdev *pdev =
		dp_get_pdev_from_soc_pdev_id_wifi3((struct dp_soc *)soc,
						   pdev_id);

	if (!pdev)
		return QDF_STATUS_E_FAILURE;

	cfg = pdev->soc->wlan_cfg_ctx;
	if (qdf_unlikely(!wlan_cfg_is_rx_flow_tag_enabled(cfg))) {
		dp_err("RX Flow tag feature disabled");
		return QDF_STATUS_E_NOSUPPORT;
	}

	status = dp_rx_flow_get_fse_stats(pdev, flow_info, &stats);

	if (status != QDF_STATUS_SUCCESS) {
		dp_err("Unable to get flow stats, error: %u", status);
		return status;
	}

	DP_PRINT_STATS("Dest IP address %x:%x:%x:%x",
		       flow_info->flow_tuple_info.dest_ip_127_96,
		       flow_info->flow_tuple_info.dest_ip_95_64,
		       flow_info->flow_tuple_info.dest_ip_63_32,
		       flow_info->flow_tuple_info.dest_ip_31_0);
	DP_PRINT_STATS("Source IP address %x:%x:%x:%x",
		       flow_info->flow_tuple_info.src_ip_127_96,
		       flow_info->flow_tuple_info.src_ip_95_64,
		       flow_info->flow_tuple_info.src_ip_63_32,
		       flow_info->flow_tuple_info.src_ip_31_0);
	DP_PRINT_STATS("Dest port %u, Src Port %u, Protocol %u",
		       flow_info->flow_tuple_info.dest_port,
		       flow_info->flow_tuple_info.src_port,
		       flow_info->flow_tuple_info.l4_protocol);
	DP_PRINT_STATS("MSDU Count: %u", stats.msdu_count);

	return status;
}
#endif /* WLAN_SUPPORT_RX_FLOW_TAG */
