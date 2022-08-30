/*
 **************************************************************************
 * Copyright (c) 2022 Qualcomm Innovation Center, Inc. All rights reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for
 * any purpose with or without fee is hereby granted, provided that the
 * above copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT
 * OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 **************************************************************************
 */

#include <linux/list.h>
#include <linux/string.h>
#include <linux/hrtimer.h>
#include <net/act_api.h>
#include <net/netfilter/nf_conntrack_core.h>
#include <net/ip6_checksum.h>
#include "nss_udp_st_public.h"

int tx_timer_flag;
static ktime_t kt;
static struct hrtimer tx_hr_timer;
static enum hrtimer_restart tx_hr_restart = HRTIMER_NORESTART;

/*
 * nss_udp_st_generate_ipv4_hdr()
 *	generate ipv4 header
 */
static inline void nss_udp_st_generate_ipv4_hdr(struct iphdr *iph, uint16_t ip_len, struct nss_udp_st_rules *rules)
{
	iph->version = 4;
	iph->ihl = 5;
	iph->tos = nust.config.dscp;
	iph->tot_len = htons(ip_len);
	iph->id = 0;
	iph->frag_off = 0;
	iph->ttl = 64;
	iph->protocol = IPPROTO_UDP;
	iph->check = 0;
	iph->saddr = htonl(rules->sip.ip.ipv4);
	iph->daddr = htonl(rules->dip.ip.ipv4);
	iph->check = ip_fast_csum((unsigned char *)iph, iph->ihl);
}

/*
 * nss_udp_st_generate_ipv6_hdr()
 *	generate ipv6 header
 */
static inline void nss_udp_st_generate_ipv6_hdr(struct ipv6hdr *ipv6h, uint16_t ip_len, struct nss_udp_st_rules *rules)
{
	struct in6_addr addr;

	ipv6h->version = 6;
	memset(&ipv6h->flow_lbl, 0, sizeof(ipv6h->flow_lbl));
	ipv6h->nexthdr = IPPROTO_UDP;
	ipv6h->payload_len = htons(ip_len - sizeof(*ipv6h));
	ipv6h->hop_limit = 64;
	nss_udp_st_get_ipv6_addr_hton(rules->sip.ip.ipv6, addr.s6_addr32);
	memcpy(ipv6h->saddr.s6_addr32, addr.s6_addr32, sizeof(ipv6h->saddr.s6_addr32));
	nss_udp_st_get_ipv6_addr_hton(rules->dip.ip.ipv6, addr.s6_addr32);
	memcpy(ipv6h->daddr.s6_addr32, addr.s6_addr32, sizeof(ipv6h->daddr.s6_addr32));
}

/*
 * nss_udp_st_generate_udp_hdr()
 *	generate udp header
 */
static void nss_udp_st_generate_udp_hdr(struct udphdr *uh, uint16_t udp_len, struct nss_udp_st_rules *rules)
{

	uh->source = htons(rules->sport);
	uh->dest = htons(rules->dport);
	uh->len = htons(udp_len);

	if (rules->flags & NSS_UDP_ST_FLAG_IPV4) {
		uh->check = csum_tcpudp_magic(rules->sip.ip.ipv4, rules->dip.ip.ipv4, udp_len, IPPROTO_UDP,
		csum_partial(uh, udp_len, 0));
	} else if (rules->flags & NSS_UDP_ST_FLAG_IPV6) {
		struct in6_addr saddr;
		struct in6_addr daddr;
		nss_udp_st_get_ipv6_addr_hton(rules->sip.ip.ipv6, saddr.s6_addr32);
		nss_udp_st_get_ipv6_addr_hton(rules->dip.ip.ipv6, daddr.s6_addr32);
		uh->check = csum_ipv6_magic(&saddr, &daddr, udp_len, IPPROTO_UDP,
		csum_partial(uh, udp_len, 0));
	} else {
		atomic_long_inc(&nust.stats.errors[NSS_UDP_ST_ERROR_INCORRECT_IP_VERSION]);
		return;
	}

	if (uh->check == 0) {
		uh->check = CSUM_MANGLED_0;
	}
}

/*
 * nss_udp_st_generate_eth_hdr()
 *	generate L2 header
 */
static inline void nss_udp_st_generate_eth_hdr(struct ethhdr *eh, struct sk_buff *skb, struct nss_udp_st_rules *rules, struct net_device *ndev)
{
	if (rules->flags & NSS_UDP_ST_FLAG_IPV4) {
		eh->h_proto = htons(ETH_P_IP);
		skb->protocol = htons(ETH_P_IP);
	} else {
		eh->h_proto = htons(ETH_P_IPV6);
		skb->protocol = htons(ETH_P_IPV6);
	}

	memcpy(eh->h_source, ndev->dev_addr, ETH_ALEN);
	memcpy(eh->h_dest, rules->dst_mac, ETH_ALEN);
}

/*
 * nss_udp_st_tx_packets()
 *	allocate, populate and send tx packet
 */
static void nss_udp_st_tx_packets(struct net_device *ndev, struct nss_udp_st_rules *rules)
{
	struct sk_buff *skb;
	struct udphdr *uh;
	struct iphdr *iph;
	struct ipv6hdr *ipv6h;
	struct ethhdr *eh;
	size_t align_offset;
	size_t skb_sz;
	size_t pkt_sz;
	uint16_t ip_len;
	uint16_t udp_len;
	unsigned char *data;

	pkt_sz = nust.config.buffer_sz;
	ip_len = pkt_sz;

	if (rules->flags & NSS_UDP_ST_FLAG_IPV4) {
		udp_len = pkt_sz - sizeof(*iph);
	} else if (rules->flags & NSS_UDP_ST_FLAG_IPV6) {
		udp_len = pkt_sz - sizeof(*ipv6h);
	} else {
		atomic_long_inc(&nust.stats.errors[NSS_UDP_ST_ERROR_INCORRECT_IP_VERSION]);
		return;
	}

	skb_sz = NSS_UDP_ST_MIN_HEADROOM + pkt_sz + sizeof(struct ethhdr) + NSS_UDP_ST_MIN_TAILROOM + SMP_CACHE_BYTES;

	skb = dev_alloc_skb(skb_sz);
	if (!skb) {
		atomic_long_inc(&nust.stats.errors[NSS_UDP_ST_ERROR_MEMORY_FAILURE]);
		return;
	}

	align_offset = PTR_ALIGN(skb->data, SMP_CACHE_BYTES) - skb->data;
	skb_reserve(skb, NSS_UDP_ST_MAX_HEADROOM + align_offset + sizeof(uint16_t));

	/*
	 * populate udp header
	 */
	skb_push(skb, sizeof(*uh));
	skb_reset_transport_header(skb);
	uh = udp_hdr(skb);
	nss_udp_st_generate_udp_hdr(uh, udp_len, rules);

	/*
	 * populate ipv4 or ipv6  header
	 */
	if (rules->flags & NSS_UDP_ST_FLAG_IPV4) {
		skb_push(skb, sizeof(*iph));
		skb_reset_network_header(skb);
		iph = ip_hdr(skb);
		nss_udp_st_generate_ipv4_hdr(iph, ip_len, rules);
		data = skb_put(skb, pkt_sz - sizeof(*iph) - sizeof(*uh));
		memset(data, 0, pkt_sz - sizeof(*iph) - sizeof(*uh));
	} else if (rules->flags & NSS_UDP_ST_FLAG_IPV6) {
		skb_push(skb, sizeof(*ipv6h));
		skb_reset_network_header(skb);
		ipv6h = ipv6_hdr(skb);
		nss_udp_st_generate_ipv6_hdr(ipv6h, ip_len, rules);
		data = skb_put(skb, pkt_sz - sizeof(*ipv6h) - sizeof(*uh));
		memset(data, 0, pkt_sz - sizeof(*ipv6h) - sizeof(*uh));
	} else {
		atomic_long_inc(&nust.stats.errors[NSS_UDP_ST_ERROR_INCORRECT_IP_VERSION]);
		kfree_skb(skb);
		return;
	}

	/*
	 * populate ethernet header
	 */
	eh = (struct ethhdr *)skb_push(skb, ETH_HLEN);
	skb_reset_mac_header(skb);
	nss_udp_st_generate_eth_hdr(eh, skb, rules, ndev);

	/*
	 * tx packet
	 */
	skb->dev = ndev;
	if (ndev->netdev_ops->ndo_start_xmit(skb, ndev) != NETDEV_TX_OK) {
		kfree_skb(skb);
		atomic_long_inc(&nust.stats.errors[NSS_UDP_ST_ERROR_PACKET_DROP]);
		return;
	}
	nss_udp_st_update_stats(ip_len + sizeof(struct ethhdr));
}

/*
 * nss_udp_st_set_dev()
 *	get net_device
 */
static bool nss_udp_st_set_dev(void)
{
	nust_dev = dev_get_by_name(&init_net, nust.config.net_dev);
	if (!nust_dev) {
		pr_err("Cannot find the net device\n");
		return false;
	}
	return true;
}

/*
 * nss_udp_st_tx_valid()
 *	check if test time has elapsed
 */
bool nss_udp_st_tx_valid(void)
{
	long long elapsed = atomic_long_read(&nust.stats.timer_stats[NSS_UDP_ST_STATS_TIME_ELAPSED]);

	if (elapsed < (nust.time * 1000)) {
		return true;
	}
	nust.mode = NSS_UDP_ST_STOP;
	return false;
}

/*
 * nss_udp_st_tx_work_send_packets()
 *	generate and send packets per rule
 */
static void nss_udp_st_tx_work_send_packets(void)
{
	int i = 0;
	struct nss_udp_st_rules *pos = NULL;
	struct nss_udp_st_rules *n = NULL;

	if (!nss_udp_st_tx_valid()  || nust.mode == NSS_UDP_ST_STOP ) {
		dev_put(nust_dev);
		tx_hr_restart = HRTIMER_NORESTART;
		return;
	}

	list_for_each_entry_safe(pos, n, &nust.rules.list, list) {
		for (i = 0; i < nss_udp_st_tx_num_pkt; i++) {
			/*
			 * check if test time has elapsed or test has been stopped
			 */
			if (!nss_udp_st_tx_valid()  || nust.mode == NSS_UDP_ST_STOP ) {
				dev_put(nust_dev);
				tx_hr_restart = HRTIMER_NORESTART;
				return;
			}
			nss_udp_st_tx_packets(nust_dev, pos);
		}
	}
	tx_hr_restart = HRTIMER_RESTART;
}

/*
 * nss_udp_st_tx_init()
 *	initialize speedtest for tx
 */
static bool nss_udp_st_tx_init(void)
{
	uint64_t total_bps;

	if (nust.config.rate > NSS_UDP_ST_RATE_MAX) {
		atomic_long_inc(&nust.stats.errors[NSS_UDP_ST_ERROR_INCORRECT_RATE]);
		return false;
	}

	if (nust.config.buffer_sz > NSS_UDP_ST_BUFFER_SIZE_MAX) {
		atomic_long_inc(&nust.stats.errors[NSS_UDP_ST_ERROR_INCORRECT_BUFFER_SIZE]);
		return false;
	}
	total_bps = (uint64_t)nust.config.rate * 1024 * 1024;

	/*
	 * calculate number of pkts to send per rule per 10 ms
	 */
	nss_udp_st_tx_num_pkt = total_bps / (nust.rule_count * (nust.config.buffer_sz + sizeof(struct ethhdr)) * 8 * NSS_UDP_ST_TX_TIMER);
	nss_udp_st_tx_num_pkt ++;
	pr_debug("total number of packets to tx every 100ms %llu\n",nss_udp_st_tx_num_pkt);
	if(!nss_udp_st_set_dev()) {
		return false;
	}

	return true;
}

/*
 * nss_udp_st_hrtimer_cleanup()
 *	cancel hrtimer
 */
void nss_udp_st_hrtimer_cleanup(void)
{
	hrtimer_cancel(&tx_hr_timer);
	tx_hr_restart = HRTIMER_NORESTART;
}

/*
 * nss_udp_st_hrtimer_callback()
 *	hrtimer callback function
 */
static enum hrtimer_restart nss_udp_st_hrtimer_callback(struct hrtimer *timer)
{
	nss_udp_st_tx_work_send_packets();
	if(tx_hr_restart == HRTIMER_RESTART) {
		hrtimer_forward_now(timer, kt);
	}
	return tx_hr_restart;
}

/*
 * nss_udp_st_hrtimer_init()
 *	initialize hrtimer
 */
void nss_udp_st_hrtimer_init(void)
{
	tx_hr_restart = HRTIMER_RESTART;
	kt = ktime_set(0,10000000);
	hrtimer_init(&tx_hr_timer, CLOCK_REALTIME, HRTIMER_MODE_ABS_HARD);
	tx_hr_timer.function = &nss_udp_st_hrtimer_callback;
}

/*
 * nss_udp_st_tx()
 *	start speedtest for tx
 */
bool nss_udp_st_tx(void)
{
	if (!nss_udp_st_tx_init()) {
		return false;
	}

	if (!tx_timer_flag) {
		nss_udp_st_hrtimer_init();
		hrtimer_start(&tx_hr_timer, kt, HRTIMER_MODE_ABS_HARD);
		tx_timer_flag = 1;
	} else {
		hrtimer_restart(&tx_hr_timer);
	}

	return true;
}
