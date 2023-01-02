/*
 ******************************************************************************
 * Copyright (c) 2017, The Linux Foundation. All rights reserved.
 * Permission to use, copy, modify, and/or distribute this software for
 * any purpose with or without fee is hereby granted, provided that the
 * above copyright notice and this permission notice appear in all copies.
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT
 * OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 * ****************************************************************************
 */

#ifndef __NSS_WIFI_STATS_H
#define __NSS_WIFI_STATS_H

/*
 * wifi statistics
 */
enum nss_wifi_stats_types {
	NSS_WIFI_STATS_RX_PKTS,
	NSS_WIFI_STATS_RX_QUEUE_0_DROPPED,
	NSS_WIFI_STATS_RX_QUEUE_1_DROPPED,
	NSS_WIFI_STATS_RX_QUEUE_2_DROPPED,
	NSS_WIFI_STATS_RX_QUEUE_3_DROPPED,
	NSS_WIFI_STATS_TX_PKTS,
	NSS_WIFI_STATS_TX_DROPPED,
	NSS_WIFI_STATS_TX_COMPLETED,
	NSS_WIFI_STATS_MGMT_RCV_CNT,
	NSS_WIFI_STATS_MGMT_TX_PKTS,
	NSS_WIFI_STATS_MGMT_TX_DROPPED,
	NSS_WIFI_STATS_MGMT_TX_COMPLETIONS,
	NSS_WIFI_STATS_TX_INV_PEER_ENQUEUE_CNT,
	NSS_WIFI_STATS_RX_INV_PEER_RCV_CNT,
	NSS_WIFI_STATS_RX_PN_CHECK_FAILED,
	NSS_WIFI_STATS_RX_DELIVERED,
	NSS_WIFI_STATS_RX_BYTES_DELIVERED,
	NSS_WIFI_STATS_TX_BYTES_COMPLETED,
	NSS_WIFI_STATS_RX_DELIVER_UNALIGNED_DROP_CNT,
	NSS_WIFI_STATS_TIDQ_ENQUEUE_CNT,
	NSS_WIFI_STATS_TIDQ_DEQUEUE_CNT = NSS_WIFI_STATS_TIDQ_ENQUEUE_CNT + 8,
	NSS_WIFI_STATS_TIDQ_ENQUEUE_FAIL_CNT = NSS_WIFI_STATS_TIDQ_DEQUEUE_CNT + 8,
	NSS_WIFI_STATS_TIDQ_TTL_EXPIRE_CNT = NSS_WIFI_STATS_TIDQ_ENQUEUE_FAIL_CNT + 8,
	NSS_WIFI_STATS_TIDQ_DEQUEUE_REQ_CNT = NSS_WIFI_STATS_TIDQ_TTL_EXPIRE_CNT + 8,
	NSS_WIFI_STATS_TOTAL_TIDQ_DEPTH = NSS_WIFI_STATS_TIDQ_DEQUEUE_REQ_CNT + 8,
	NSS_WIFI_STATS_RX_HTT_FETCH_CNT,
	NSS_WIFI_STATS_TOTAL_TIDQ_BYPASS_CNT,
	NSS_WIFI_STATS_GLOBAL_Q_FULL_CNT,
	NSS_WIFI_STATS_TIDQ_FULL_CNT,
	NSS_WIFI_STATS_UNATH_RX_PKT_DROP,
	NSS_WIFI_STATS_MAX,
};

/*
 * wifi statistics APIs
 */
extern void nss_wifi_stats_sync(struct nss_ctx_instance *nss_ctx, struct nss_wifi_stats_sync_msg *stats, uint16_t interface);
extern void nss_wifi_stats_dentry_create(void);

#endif /* __NSS_WIFI_STATS_H */
