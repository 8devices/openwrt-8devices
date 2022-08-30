/*
 * Copyright (c) 2021, The Linux Foundation. All rights reserved.
 *
 * Copyright (c) 2022 Qualcomm Innovation Center, Inc. All rights reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER
 * RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT
 * NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE
 * USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <asm/cacheflush.h>
#include <linux/version.h>
#include <linux/netdevice.h>
#include "edma.h"
#include "edma_debug.h"
#include "edma_regs.h"
#include "nss_dp_dev.h"

/*
 * edma_rx_alloc_buffer()
 *	Alloc Rx buffers for one RxFill ring
 */
int edma_rx_alloc_buffer(struct edma_rxfill_ring *rxfill_ring, int alloc_count)
{
	struct edma_rxfill_desc *rxfill_desc;
	struct edma_rx_fill_stats *rxfill_stats = &rxfill_ring->rx_fill_stats;
	uint16_t prod_idx, start_idx;
	uint16_t num_alloc = 0;
	uint32_t rx_alloc_size = rxfill_ring->alloc_size;
	uint32_t buf_len = rxfill_ring->buf_len;
	bool page_mode = rxfill_ring->page_mode;

	/*
	 * Get RXFILL ring producer index
	 */
	prod_idx = rxfill_ring->prod_idx;
	start_idx = prod_idx;

	while (likely(alloc_count--)) {
		void *page_addr = NULL;
		struct page *pg;
		struct sk_buff *skb;
		dma_addr_t buff_addr;

		/*
		 * Get RXFILL descriptor
		 */
		rxfill_desc = EDMA_RXFILL_DESC(rxfill_ring, prod_idx);

		/*
		 * Prefetch the current rxfill descriptor.
		 */
		prefetch(rxfill_desc);

		/*
		 * Allocate buffer
		 */
		skb = dev_alloc_skb(rx_alloc_size);
		if (unlikely(!skb)) {
			u64_stats_update_begin(&rxfill_stats->syncp);
			++rxfill_stats->alloc_failed;
			u64_stats_update_end(&rxfill_stats->syncp);
			break;
		}

		/*
		 * Reserve headroom
		 */
		skb_reserve(skb, EDMA_RX_SKB_HEADROOM + NET_IP_ALIGN);

		/*
		 * Map Rx buffer for DMA
		 */
		if (likely(!page_mode)) {
			buff_addr = (dma_addr_t)virt_to_phys(skb->data);
		} else {
			pg = alloc_page(GFP_ATOMIC);
			if (unlikely(!pg)) {
				u64_stats_update_begin(&rxfill_stats->syncp);
				++rxfill_stats->page_alloc_failed;
				u64_stats_update_end(&rxfill_stats->syncp);
				dev_kfree_skb_any(skb);
				edma_debug("edma_gbl_ctx:%px Unable to allocate page", &edma_gbl_ctx);
				break;
			}

			/*
			 * Get virtual address of allocated page
			 */
			page_addr = page_address(pg);
			buff_addr = (dma_addr_t)virt_to_phys(page_addr);
			skb_fill_page_desc(skb, 0, pg, 0, PAGE_SIZE);
			dmac_inv_range_no_dsb(page_addr, (page_addr + PAGE_SIZE));
		}

		EDMA_RXFILL_BUFFER_ADDR_SET(rxfill_desc, buff_addr);

		/*
		 * Store skb in opaque
		 */
		EDMA_RXFILL_OPAQUE_LO_SET(rxfill_desc, skb);
#ifdef __LP64__
		EDMA_RXFILL_OPAQUE_HI_SET(rxfill_desc, skb);
#endif

		/*
		 * Save buffer size in RXFILL descriptor
		 */
		EDMA_RXFILL_PACKET_LEN_SET(
			rxfill_desc,
			cpu_to_le32((uint32_t)
			(buf_len)
			& EDMA_RXFILL_BUF_SIZE_MASK));

		/*
		 * Invalidate skb->data
		 */
		dmac_inv_range_no_dsb((void *)skb->data,
				(void *)(skb->data + rx_alloc_size -
					EDMA_RX_SKB_HEADROOM -
					NET_IP_ALIGN));
		prod_idx = (prod_idx + 1) & EDMA_RX_RING_SIZE_MASK;
		num_alloc++;
	}

	if (likely(num_alloc)) {
		uint16_t end_idx =
			(start_idx + num_alloc) & EDMA_RX_RING_SIZE_MASK;

		rxfill_desc = EDMA_RXFILL_DESC(rxfill_ring, start_idx);

		/*
		 * Write-back all the cached descriptors
		 * that are processed.
		 */
		if (end_idx > start_idx) {
			dmac_clean_range_no_dsb((void *)rxfill_desc,
					(void *)(rxfill_desc + num_alloc));
		} else {
			dmac_clean_range_no_dsb((void *)rxfill_ring->desc,
					(void *)(rxfill_ring->desc + end_idx));
			dmac_clean_range_no_dsb((void *)rxfill_desc,
					(void *)(rxfill_ring->desc +
							EDMA_RX_RING_SIZE));
		}

		/*
		 * Make sure the information written to the descriptors
		 * is updated before writing to the hardware.
		 */
		dsb(st);

		edma_reg_write(EDMA_REG_RXFILL_PROD_IDX(rxfill_ring->ring_id),
								prod_idx);
		rxfill_ring->prod_idx = prod_idx;
	}

	return num_alloc;
}

/*
 * edma_rx_checksum_verify()
 *	Update hw checksum status into skb
 */
static inline void edma_rx_checksum_verify(struct edma_rxdesc_desc *rxdesc_desc,
							struct sk_buff* skb)
{
	uint8_t pid = EDMA_RXDESC_PID_GET(rxdesc_desc);

	skb_checksum_none_assert(skb);

	if (likely(EDMA_RX_PID_IS_IPV4(pid))) {
		if (likely(EDMA_RXDESC_L3CSUM_STATUS_GET(rxdesc_desc))
			&& likely(EDMA_RXDESC_L4CSUM_STATUS_GET(rxdesc_desc))) {
			skb->ip_summed = CHECKSUM_UNNECESSARY;
		}
	} else if (likely(EDMA_RX_PID_IS_IPV6(pid))) {
		if (likely(EDMA_RXDESC_L4CSUM_STATUS_GET(rxdesc_desc))) {
			skb->ip_summed = CHECKSUM_UNNECESSARY;
		}
	}
}

/*
 * edma_rx_handle_scatter_frames()
 *	Handle scattered packets in Rx direction
 *
 * This function should free the SKB in case of failure.
 */
static void edma_rx_handle_scatter_frames(struct edma_gbl_ctx *egc,
		struct edma_rxdesc_ring *rxdesc_ring,
		struct edma_rxdesc_desc *rxdesc_desc,
		struct sk_buff *skb)
{
	struct nss_dp_dev *dp_dev;
	struct edma_pcpu_stats *pcpu_stats;
	struct edma_rx_stats *rx_stats;
	struct sk_buff *rxdesc_ring_head;
	struct net_device *dev;
	uint32_t pkt_length;
	skb_frag_t *frag = NULL;
	bool page_mode = rxdesc_ring->rxfill->page_mode;

	/*
	 * Get packet length
	 */
	pkt_length = EDMA_RXDESC_PACKET_LEN_GET(rxdesc_desc);
	edma_debug("edma_gbl_ctx:%px skb:%px fragment pkt_length:%u\n", egc, skb, pkt_length);

	/*
	 * For fraglist case
	 */
	if (likely(!page_mode)) {

		/*
		 * Invalidate the buffer received from the HW
		 */
		dmac_inv_range((void *)skb->data,
				(void *)(skb->data + pkt_length));

		if (!(rxdesc_ring->head)) {
			skb_put(skb, pkt_length);
			rxdesc_ring->head = skb;
			rxdesc_ring->last = NULL;
			return;
		}

		/*
		 * If head is present and got next desc.
		 * Append it to the fraglist of head if this is second frame
		 * If not second frame append to tail
		 */
		skb_put(skb, pkt_length);
		if (!skb_has_frag_list(rxdesc_ring->head)) {
			skb_shinfo(rxdesc_ring->head)->frag_list = skb;
		} else {
			rxdesc_ring->last->next = skb;
		}

		rxdesc_ring->last = skb;
		rxdesc_ring->last->next = NULL;
		rxdesc_ring->head->len += pkt_length;
		rxdesc_ring->head->data_len += pkt_length;
		rxdesc_ring->head->truesize += skb->truesize;

		goto process_last_scatter;
	}

	/*
	 * Manage fragments for page mode
	 */
	frag = &skb_shinfo(skb)->frags[0];
	dmac_inv_range((void *)skb_frag_page(frag), (void *)(skb_frag_page(frag) + pkt_length));

	if (!(rxdesc_ring->head)) {
		skb->len = pkt_length;
		skb->data_len = pkt_length;
		skb->truesize = SKB_TRUESIZE(PAGE_SIZE);
		rxdesc_ring->head = skb;
		rxdesc_ring->last = NULL;
		return;
	}

	/*
	 * Append current frag at correct index as nr_frag of parent
	 */
	skb_add_rx_frag(rxdesc_ring->head, skb_shinfo(rxdesc_ring->head)->nr_frags,
			skb_frag_page(frag), 0, pkt_length, PAGE_SIZE);
	skb_shinfo(skb)->nr_frags = 0;

	/*
	 * Free the SKB after we have appended its frag page to the head skb
	 */
	dev_kfree_skb_any(skb);

process_last_scatter:

	/*
	 * If there are more segments for this packet,
	 * then we have nothing to do. Otherwise process
	 * last segment and send packet to stack
	 */
	rxdesc_ring_head = rxdesc_ring->head;
	dev = rxdesc_ring_head->dev;

	if (EDMA_RXDESC_MORE_BIT_GET(rxdesc_desc)) {
		return;
	}

	/*
	 * Check Rx checksum offload status.
	 */
	if (likely(dev->features & NETIF_F_RXCSUM)) {
		edma_rx_checksum_verify(rxdesc_desc, rxdesc_ring_head);
	}

	/*
	 * Get stats for the netdevice
	 */
	dp_dev = netdev_priv(dev);
	pcpu_stats = &dp_dev->dp_info.pcpu_stats;
	rx_stats = this_cpu_ptr(pcpu_stats->rx_stats);

	if (unlikely(page_mode)) {
		if (unlikely(!pskb_may_pull(rxdesc_ring_head, ETH_HLEN))) {
			/*
			 * Discard the SKB that we have been building,
			 * in addition to the SKB linked to current descriptor.
			 */
			dev_kfree_skb_any(rxdesc_ring_head);
			rxdesc_ring->head = NULL;
			rxdesc_ring->last = NULL;

			u64_stats_update_begin(&rx_stats->syncp);
			rx_stats->rx_nr_frag_headroom_err++;
			u64_stats_update_end(&rx_stats->syncp);

			return;
		}
	}

	/*
	 * TODO: Do a batched update of the stats per netdevice.
	 */
	u64_stats_update_begin(&rx_stats->syncp);
	rx_stats->rx_pkts++;
	rx_stats->rx_bytes += rxdesc_ring_head->len;
	rx_stats->rx_nr_frag_pkts += (uint64_t)page_mode;
	rx_stats->rx_fraglist_pkts += (uint64_t)(!page_mode);
	u64_stats_update_end(&rx_stats->syncp);

	rxdesc_ring_head->protocol = eth_type_trans(rxdesc_ring_head, dev);

	edma_debug("edma_gbl_ctx:%px skb:%px Jumbo pkt_length:%u\n", egc, rxdesc_ring_head, rxdesc_ring_head->len);

	/*
	 * Send packet up the stack
	 */
#if defined(NSS_DP_ENABLE_NAPI_GRO)
	napi_gro_receive(&rxdesc_ring->napi, rxdesc_ring_head);
#else
	netif_receive_skb(rxdesc_ring_head);
#endif

	rxdesc_ring->head = NULL;
	rxdesc_ring->last = NULL;
}

/*
 * edma_rx_handle_linear_packets()
 *	Handle linear packets
 */
static inline bool edma_rx_handle_linear_packets(struct edma_gbl_ctx *egc,
		struct edma_rxdesc_ring *rxdesc_ring,
		struct edma_rxdesc_desc *rxdesc_desc,
		struct sk_buff *skb)
{
	struct nss_dp_dev *dp_dev;
	struct edma_pcpu_stats *pcpu_stats;
	struct edma_rx_stats *rx_stats;
	uint32_t pkt_length;
	skb_frag_t *frag = NULL;
	bool page_mode = rxdesc_ring->rxfill->page_mode;

	/*
	 * Get stats for the netdevice
	 */
	dp_dev = netdev_priv(skb->dev);
	pcpu_stats = &dp_dev->dp_info.pcpu_stats;
	rx_stats = this_cpu_ptr(pcpu_stats->rx_stats);

	/*
	 * Get packet length
	 */
	pkt_length = EDMA_RXDESC_PACKET_LEN_GET(rxdesc_desc);

	if (likely(!page_mode)) {

		/*
		 * Invalidate the buffer received from the HW
		 */
		dmac_inv_range((void *)skb->data,
				(void *)(skb->data + pkt_length));
		prefetch(skb->data);
		skb_put(skb, pkt_length);
		goto send_to_stack;
	}

	/*
	 * Handle linear packet in page mode
	 */
	frag = &skb_shinfo(skb)->frags[0];
	dmac_inv_range((void *)skb_frag_page(frag),
			(void *)(skb_frag_page(frag) + pkt_length));
	skb_add_rx_frag(skb, 0, skb_frag_page(frag), 0, pkt_length, PAGE_SIZE);

	/*
	 * Pull ethernet header into SKB data area for header processing
	 */
	if (unlikely(!pskb_may_pull(skb, ETH_HLEN))) {
		u64_stats_update_begin(&rx_stats->syncp);
		rx_stats->rx_nr_frag_headroom_err++;
		u64_stats_update_end(&rx_stats->syncp);
		return false;
	}

send_to_stack:
	/*
	 * Check Rx checksum offload status.
	 */
	if (likely(skb->dev->features & NETIF_F_RXCSUM)) {
		edma_rx_checksum_verify(rxdesc_desc, skb);
	}

	/*
	 * TODO: Do a batched update of the stats per netdevice.
	 */
	u64_stats_update_begin(&rx_stats->syncp);
	rx_stats->rx_pkts++;
	rx_stats->rx_bytes += pkt_length;
	rx_stats->rx_nr_frag_pkts += (uint64_t)page_mode;
	u64_stats_update_end(&rx_stats->syncp);

	edma_debug("edma_gbl_ctx:%px, skb:%px pkt_length:%u\n",
			egc, skb, skb->len);

	skb->protocol = eth_type_trans(skb, skb->dev);

	/*
	 * Send packet upto network stack
	 */
#if defined(NSS_DP_ENABLE_NAPI_GRO)
	napi_gro_receive(&rxdesc_ring->napi, skb);
#else
	netif_receive_skb(skb);
#endif

	return true;
}

/*
 * edma_rx_reap()
 *	Reap Rx descriptors
 */
static uint32_t edma_rx_reap(struct edma_gbl_ctx *egc, int budget,
				struct edma_rxdesc_ring *rxdesc_ring)
{
	struct edma_rxdesc_desc *next_rxdesc_desc;
	struct edma_rx_desc_stats *rxdesc_stats = &rxdesc_ring->rx_desc_stats;
	uint32_t work_to_do, work_done = 0;
	uint32_t work_leftover;
	uint16_t prod_idx, cons_idx, end_idx;
	struct sk_buff *next_skb;

	/*
	 * Get Rx ring producer and consumer indices
	 */
	cons_idx = rxdesc_ring->cons_idx;

	if (likely(rxdesc_ring->work_leftover > EDMA_RX_MAX_PROCESS)) {
		work_to_do = rxdesc_ring->work_leftover;
	} else {
		prod_idx =
			edma_reg_read(EDMA_REG_RXDESC_PROD_IDX(rxdesc_ring->ring_id)) &
			EDMA_RXDESC_PROD_IDX_MASK;
		work_to_do = EDMA_DESC_AVAIL_COUNT(prod_idx,
				cons_idx, EDMA_RX_RING_SIZE);
		rxdesc_ring->work_leftover = work_to_do;
	}

	if (work_to_do > budget) {
		work_to_do = budget;
	}
	rxdesc_ring->work_leftover -= work_to_do;

	end_idx = (cons_idx + work_to_do) & EDMA_RX_RING_SIZE_MASK;

	next_rxdesc_desc = EDMA_RXDESC_PRI_DESC(rxdesc_ring, cons_idx);

	/*
	 * Invalidate all the cached descriptors
	 * that'll be processed.
	 */
	if (end_idx > cons_idx) {
		dmac_inv_range_no_dsb((void *)next_rxdesc_desc,
			(void *)(next_rxdesc_desc + work_to_do));
	} else {
		dmac_inv_range_no_dsb((void *)rxdesc_ring->pdesc,
			(void *)(rxdesc_ring->pdesc + end_idx));
		dmac_inv_range_no_dsb((void *)next_rxdesc_desc,
			(void *)(rxdesc_ring->pdesc + EDMA_RX_RING_SIZE));
	}

	dsb(st);

	/*
	 * Get opaque from RXDESC
	 */
	next_skb = (struct sk_buff *)EDMA_RXDESC_OPAQUE_GET(next_rxdesc_desc);

	work_leftover = work_to_do & (EDMA_RX_MAX_PROCESS - 1);
	while (likely(work_to_do--)) {
		struct edma_rxdesc_desc *rxdesc_desc;
		struct net_device *ndev;
		struct sk_buff *skb;
		uint32_t src_port_num;

		skb = next_skb;
		rxdesc_desc = next_rxdesc_desc;

		/*
		 * Update consumer index
		 */
		cons_idx = (cons_idx + 1) & EDMA_RX_RING_SIZE_MASK;

		/*
		 * Prefetch the next Rx descriptor.
		 */
		next_rxdesc_desc = EDMA_RXDESC_PRI_DESC(rxdesc_ring, cons_idx);
		prefetch(next_rxdesc_desc);

		/*
		 * Handle linear packets or initial segments first
		 */
		if (likely(!(rxdesc_ring->head))) {
			/*
			 * Check src_info
			 */
			src_port_num = EDMA_RXDESC_SRC_INFO_GET(rxdesc_desc);
			if (likely((src_port_num & EDMA_RXDESC_SRCINFO_TYPE_MASK)
					== EDMA_RXDESC_SRCINFO_TYPE_PORTID)) {
				src_port_num &= EDMA_RXDESC_PORTNUM_BITS;
			} else {
				edma_warn("Src_info_type:0x%x. Drop skb:%px\n",
						(src_port_num &
						EDMA_RXDESC_SRCINFO_TYPE_MASK),
						skb);
				u64_stats_update_begin(&rxdesc_stats->syncp);
				++rxdesc_stats->src_port_inval_type;
				u64_stats_update_end(&rxdesc_stats->syncp);
				dev_kfree_skb_any(skb);
				goto next_rx_desc;
			}

			if (unlikely((src_port_num < NSS_DP_START_IFNUM) ||
					(src_port_num > NSS_DP_HAL_MAX_PORTS))) {
				edma_warn("Port number error :%d. \
						Drop skb:%px\n",
						src_port_num, skb);
				u64_stats_update_begin(&rxdesc_stats->syncp);
				++rxdesc_stats->src_port_inval;
				u64_stats_update_end(&rxdesc_stats->syncp);
				dev_kfree_skb_any(skb);
				goto next_rx_desc;
			}

			/*
			 * Get netdev for this port using the source port
			 * number as index into the netdev array. We need to
			 * subtract one since the indices start form '0' and
			 * port numbers start from '1'.
			 */
			ndev = egc->netdev_arr[src_port_num - 1];
			if (unlikely(!ndev)) {
				edma_warn("Netdev Null src_info_type:0x%x. \
						Drop skb:%px\n",
						src_port_num, skb);
				u64_stats_update_begin(&rxdesc_stats->syncp);
				++rxdesc_stats->src_port_inval_netdev;
				u64_stats_update_end(&rxdesc_stats->syncp);
				dev_kfree_skb_any(skb);
				goto next_rx_desc;
			}

			/*
			 * Update skb fields for head skb
			 */
			skb->dev = ndev;
			skb->skb_iif = ndev->ifindex;

#ifdef CONFIG_NET_SWITCHDEV
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 5, 0))
			skb->offload_fwd_mark = ndev->offload_fwd_mark;
#else
			/*
			 * TODO: Implement ndo_get_devlink_port()
			 */
			skb->offload_fwd_mark = 0;
#endif
			edma_debug("edma_gbl_ctx:%px skb:%px ring_idx:%u proto:0x%x mark:%u\n",
					egc, skb, cons_idx, skb->protocol,
					skb->offload_fwd_mark);
#else
			edma_debug("edma_gbl_ctx:%px skb:%px ring_idx:%u proto:0x%x\n",
					egc, skb, cons_idx,  skb->protocol);
#endif

			/*
			 * Handle linear packets
			 */
			if (likely(!EDMA_RXDESC_MORE_BIT_GET(rxdesc_desc))) {

				/*
				 * Prefetch the next skb.
				 */
				next_skb = (struct sk_buff *)EDMA_RXDESC_OPAQUE_GET(next_rxdesc_desc);
				prefetch(next_skb);

				if (unlikely(!edma_rx_handle_linear_packets(egc, rxdesc_ring, rxdesc_desc, skb))) {
					dev_kfree_skb_any(skb);
				}

				goto next_rx_desc;
			}
		}

		/*
		 * Prefetch the next skb.
		 */
		next_skb = (struct sk_buff *)EDMA_RXDESC_OPAQUE_GET(next_rxdesc_desc);
		prefetch(next_skb);

		/*
		 * Handle scatter frame processing for first/middle/last segments
		 */
		edma_rx_handle_scatter_frames(egc, rxdesc_ring, rxdesc_desc, skb);

next_rx_desc:
		/*
		 * Update work done
		 */
		work_done++;

		/*
		 * Check if we can refill EDMA_RX_MAX_PROCESS worth buffers,
		 * if yes, refill and update index before continuing.
		 */
		if (unlikely(!(work_done & (EDMA_RX_MAX_PROCESS - 1)))) {
			edma_reg_write(EDMA_REG_RXDESC_CONS_IDX(rxdesc_ring->ring_id),
					cons_idx);
			rxdesc_ring->cons_idx = cons_idx;
			edma_rx_alloc_buffer(rxdesc_ring->rxfill,
					EDMA_RX_MAX_PROCESS);
		}
	}

	/*
	 * Check if we need to refill and update
	 * index for any buffers before exit.
	 */
	if (unlikely(work_leftover)) {
		edma_reg_write(EDMA_REG_RXDESC_CONS_IDX(rxdesc_ring->ring_id),
				cons_idx);
		rxdesc_ring->cons_idx = cons_idx;
		edma_rx_alloc_buffer(rxdesc_ring->rxfill, work_leftover);
	}

	return work_done;
}

/*
 * edma_rx_napi_poll()
 *	EDMA RX NAPI handler
 */
int edma_rx_napi_poll(struct napi_struct *napi, int budget)
{
	struct edma_rxdesc_ring *rxdesc_ring = (struct edma_rxdesc_ring *)napi;
	struct edma_gbl_ctx *egc = &edma_gbl_ctx;
	int32_t work_done = 0;
	uint32_t status;

	do {
		work_done += edma_rx_reap(egc, budget - work_done, rxdesc_ring);
		if (likely(work_done >= budget)) {
			return work_done;
		}

		/*
		 * Check if there are more packets to process
		 */
		status = EDMA_RXDESC_RING_INT_STATUS_MASK &
			edma_reg_read(
				EDMA_REG_RXDESC_INT_STAT(rxdesc_ring->ring_id));
	} while (likely(status));

	/*
	 * No more packets to process. Finish NAPI processing.
	 */
	napi_complete(napi);

	/*
	 * Set RXDESC ring interrupt mask
	 */
	edma_reg_write(EDMA_REG_RXDESC_INT_MASK(rxdesc_ring->ring_id),
						egc->rxdesc_intr_mask);

	return work_done;
}

/*
 * edma_rx_handle_irq()
 *	Process RX IRQ and schedule napi
 */
irqreturn_t edma_rx_handle_irq(int irq, void *ctx)
{
	struct edma_rxdesc_ring *rxdesc_ring = (struct edma_rxdesc_ring *)ctx;

	edma_debug("irq: irq=%d rxdesc_ring_id=%u\n", irq, rxdesc_ring->ring_id);

	if (likely(napi_schedule_prep(&rxdesc_ring->napi))) {

		/*
		 * Disable RxDesc interrupt
		 */
		edma_reg_write(EDMA_REG_RXDESC_INT_MASK(rxdesc_ring->ring_id),
							EDMA_MASK_INT_DISABLE);
		__napi_schedule(&rxdesc_ring->napi);
	}

	return IRQ_HANDLED;
}
