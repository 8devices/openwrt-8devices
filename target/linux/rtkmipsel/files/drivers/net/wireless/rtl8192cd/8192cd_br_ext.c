/*
 *  bridge extention functions
 *
 *  $Id: 8192cd_br_ext.c,v 1.1.4.3 2010/12/22 09:08:59 pluswang Exp $
 *
 *  Copyright (c) 2009 Realtek Semiconductor Corp.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 */

#define _8192CD_BR_EXT_C_

#ifdef __KERNEL__
#include <linux/if_arp.h>
#include <net/ip.h>
#include <net/ipx.h>
#include <linux/atalk.h>
#include <linux/udp.h>
#include <linux/if_pppox.h>
typedef struct udphdr udphdr;
#elif defined(__ECOS)
//#include <cyg/io/eth/rltk/819x/wrapper/sys_support.h>
#include <cyg/io/eth/rltk/819x/wrapper/skbuff.h>
#include <cyg/io/eth/rltk/819x/wrapper/timer.h>
#include <cyg/io/eth/rltk/819x/wrapper/wrapper.h>
typedef struct udp_hdr udphdr;
#elif defined(__OSK__)
typedef struct udphdr udphdr;
#endif

#include "./8192cd_cfg.h"

#ifndef __KERNEL__
#ifdef __ECOS
#include <cyg/io/eth/rltk/819x/wrapper/sys_support.h>
#else
#include "./sys-support.h"
#endif
#endif

#include "./8192cd.h"
#include "./8192cd_headers.h"
#include "./8192cd_br_ext.h"
#include "./8192cd_debug.h"

#if defined(__OSK__)
#include "p2port.h"
#include "ipport.h"
#endif

#ifdef CL_IPV6_PASS
#ifdef __KERNEL__
#include <linux/ipv6.h>
#include <linux/icmpv6.h>
#include <net/ndisc.h>
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 24))
#include <net/ip6_checksum.h>
#else
#include <net/checksum.h>
#endif
#endif
#endif

#ifdef RTK_BR_EXT

//#define BR_EXT_DEBUG

#define NAT25_IPV4		01
#define NAT25_IPV6		02
#define NAT25_IPX		03
#define NAT25_APPLE		04
#define NAT25_PPPOE		05

#define RTL_RELAY_TAG_LEN (ETH_ALEN)
#define TAG_HDR_LEN		4

#define MAGIC_CODE		0x8186
#define MAGIC_CODE_LEN	2
#define WAIT_TIME_PPPOE	5	// waiting time for pppoe server in sec

#ifdef __ECOS
#define ETH_P_PPP_MP    0x0008
#endif

static const unsigned short nat25_filter_ethtype[] = {
    0x0800, /*IPv4*/
    0x0806, /*ARP*/
    0x8137, /*IPX*/
    0x86DD, /*IPv6*/
    0x8863, /*PPPoE discovery stage*/
    0x8864, /*PPPoE session stage*/
};

static const unsigned char nat25_filter_ipproto[] = {
    0x01, /*ICMP*/
    0x02, /*IGMP*/
    0x04, /*IP*/
    0x06, /*TCP*/ 
    0x11, /*UDP (17)*/
    0x29, /*IPv6 (41)*/
    0x2C, /*IPv6-Frag (44)*/
    0x2F, /*GRE (47)*/
    0x32, /*ESP (50)*/
    0x3A, /*IPv6-ICMP (58)*/ 
    0x73, /* L2TP (115)*/ 
    
};

#if defined(__ECOS) || defined(__OSK__)
#ifdef CL_IPV6_PASS
#define NDISC_ROUTER_SOLICITATION       133
#define NDISC_ROUTER_ADVERTISEMENT      134
#define NDISC_NEIGHBOUR_SOLICITATION    135
#define NDISC_NEIGHBOUR_ADVERTISEMENT   136
#define NDISC_REDIRECT                  137
#endif
#endif

#ifdef __ECOS
#ifdef CL_IPV6_PASS

#include <netinet/in.h>

#define ETH_P_IPV6      0x86DD          /* IPv6 over bluebook           */
#define IPPROTO_ICMPV6	58		/* ICMPv6						*/

struct ipv6hdr {
#if defined(__LITTLE_ENDIAN_BITFIELD)
        __u8                    priority:4,
                                version:4;
#elif defined(__BIG_ENDIAN_BITFIELD)
        __u8                    version:4,
                                priority:4;
#else
#error  "Please fix <asm/byteorder.h>"
#endif
        __u8                    flow_lbl[3];

        __u16                   payload_len;
        __u8                    nexthdr;
        __u8                    hop_limit;

        struct  in6_addr        saddr;
        struct  in6_addr        daddr;
};
#endif /* CL_IPV6_PASS */

#define MAX_DB_NUM		32
#define __constant_htonl(x)	htonl(x)

#define ADJUST_CHKSUM(port_mod, port_org, chksum) \
	do { \
		s32 accumulate = 0; \
		accumulate += (port_org); \
		accumulate -= (port_mod); \
		accumulate += ntohs(chksum); \
		if (accumulate < 0) { \
			accumulate = -accumulate; \
			accumulate = (accumulate >> 16) + (accumulate & 0xffff); \
			accumulate += accumulate >> 16; \
			chksum = htons((u16) ~accumulate); \
		} else { \
			accumulate = (accumulate >> 16) + (accumulate & 0xffff); \
			accumulate += accumulate >> 16; \
			chksum = htons((u16) accumulate); \
		} \
	}while(0)	/* Checksum adjustment */

static struct nat25_network_db_entry sta_db[MAX_DB_NUM];


void nat25_db_init(void)
{
	memset(sta_db, '\0', sizeof(sta_db));
}


static struct nat25_network_db_entry *nat25_db_get(void)
{
	int i;

	for (i=0; i<MAX_DB_NUM; i++) {
		if (!sta_db[i].used)
			break;
	}	
	if (i == MAX_DB_NUM)
		return NULL;

	sta_db[i].used = 1;
	return &sta_db[i];
	
}


static void nat25_db_free(struct nat25_network_db_entry *entry)
{
	entry->used = 0;
}
#endif /* __ECOS */


/*--------------------------------------------------------------------------------------------
  How database records network address:
           0    1    2    3    4    5    6    7    8    9   10   11   12   13   14   15   16
        |----|----|----|----|----|----|----|----|----|----|----|----|----|----|----|----|----|
  IPv4  |type|                             |      IP addr      |
  IPv6  |type|                             IPv6 addr                                         |
  IPX   |type|      Net addr     |          Node addr          |
  IPX   |type|      Net addr     |Sckt addr|
  Apple |type| Network |node|
  PPPoE |type|   SID   |           AC MAC            |
--------------------------------------------------------------------------------------------*/


//Find a tag in pppoe frame and return the pointer
static __inline__ unsigned char *__nat25_find_pppoe_tag(struct pppoe_hdr *ph, unsigned short type)
{
	unsigned char *cur_ptr, *start_ptr;
	unsigned short tagLen, tagType;

	start_ptr = cur_ptr = (unsigned char *)ph->tag;
	while((cur_ptr - start_ptr) < ntohs(ph->length)) {
		// prevent un-alignment access
		tagType = (unsigned short)((cur_ptr[0] << 8) + cur_ptr[1]);
		tagLen  = (unsigned short)((cur_ptr[2] << 8) + cur_ptr[3]);
		if(tagType == type)
			return cur_ptr;
		cur_ptr = cur_ptr + TAG_HDR_LEN + tagLen;
	}
	return 0;
}


static __inline__ int __nat25_add_pppoe_tag(struct sk_buff *skb, struct pppoe_tag *tag)
{
	struct pppoe_hdr *ph = (struct pppoe_hdr *)(skb->data + ETH_HLEN);
	int data_len;

	data_len = ntohs(tag->tag_len) + TAG_HDR_LEN;
	if (skb_tailroom(skb) < data_len) {
		_DEBUG_ERR("skb_tailroom() failed in add SID tag!\n");
		return -1;
	}

	skb_put(skb, data_len);
	// have a room for new tag
	memmove(((unsigned char *)ph->tag + data_len), (unsigned char *)ph->tag, ntohs(ph->length));
	ph->length = htons(ntohs(ph->length) + data_len);
	memcpy((unsigned char *)ph->tag, tag, data_len);
	return data_len;
}


static int skb_pull_and_merge(struct sk_buff *skb, unsigned char *src, int len)
{
	int tail_len;
	unsigned long end, tail;

	if ((src+len) > skb->tail || skb->len < len)
		return -1;

	tail = (unsigned long)skb->tail;
	end = (unsigned long)src+len;
	if (tail < end)
		return -1;

	tail_len = (int)(tail-end);
	if (tail_len > 0)
		memmove(src, src+len, tail_len);

	skb_trim(skb, skb->len-len);
	return 0;
}


static __inline__ unsigned long __nat25_timeout(struct rtl8192cd_priv *priv)
{
	unsigned long timeout;

	timeout = jiffies - NAT25_AGEING_TIME*HZ;

	return timeout;
}


static __inline__ int  __nat25_has_expired(struct rtl8192cd_priv *priv,
		struct nat25_network_db_entry *fdb)
{
	if(time_before_eq(fdb->ageing_timer, __nat25_timeout(priv)))
		return 1;

	return 0;
}


static __inline__ void __nat25_generate_ipv4_network_addr(unsigned char *networkAddr,
#ifdef __ECOS
		unsigned char *ipAddr
#else
		unsigned int *ipAddr
#endif
		)
{
	memset(networkAddr, 0, MAX_NETWORK_ADDR_LEN);

	networkAddr[0] = NAT25_IPV4;
	memcpy(networkAddr+7, (unsigned char *)ipAddr, 4);
}


static __inline__ void __nat25_generate_ipx_network_addr_with_node(unsigned char *networkAddr,
#ifdef __ECOS
		unsigned char *ipxNetAddr, unsigned char *ipxNodeAddr
#else
		unsigned int *ipxNetAddr, unsigned char *ipxNodeAddr
#endif
		)
{
	memset(networkAddr, 0, MAX_NETWORK_ADDR_LEN);

	networkAddr[0] = NAT25_IPX;
	memcpy(networkAddr+1, (unsigned char *)ipxNetAddr, 4);
	memcpy(networkAddr+5, ipxNodeAddr, 6);
}


static __inline__ void __nat25_generate_ipx_network_addr_with_socket(unsigned char *networkAddr,
#ifdef __ECOS
		unsigned char *ipxNetAddr, unsigned char *ipxSocketAddr
#else
		unsigned int *ipxNetAddr, unsigned short *ipxSocketAddr
#endif
		)
{
	memset(networkAddr, 0, MAX_NETWORK_ADDR_LEN);

	networkAddr[0] = NAT25_IPX;
	memcpy(networkAddr+1, (unsigned char *)ipxNetAddr, 4);
	memcpy(networkAddr+5, (unsigned char *)ipxSocketAddr, 2);
}


static __inline__ void __nat25_generate_apple_network_addr(unsigned char *networkAddr,
#ifdef __ECOS
		unsigned char *network, unsigned char *node
#else
		unsigned short *network, unsigned char *node
#endif
		)
{
	memset(networkAddr, 0, MAX_NETWORK_ADDR_LEN);

	networkAddr[0] = NAT25_APPLE;
	memcpy(networkAddr+1, (unsigned char *)network, 2);
	networkAddr[3] = *node;
}


static __inline__ void __nat25_generate_pppoe_network_addr(unsigned char *networkAddr,
		unsigned char *ac_mac, unsigned short *sid)
{
	memset(networkAddr, 0, MAX_NETWORK_ADDR_LEN);

	networkAddr[0] = NAT25_PPPOE;
	memcpy(networkAddr+1, (unsigned char *)sid, 2);
	memcpy(networkAddr+3, (unsigned char *)ac_mac, 6);
}


#ifdef CL_IPV6_PASS
static  void __nat25_generate_ipv6_network_addr(unsigned char *networkAddr,
#ifdef __ECOS
		unsigned char *ipAddr
#else
		unsigned int *ipAddr
#endif
		)
{
	memset(networkAddr, 0, MAX_NETWORK_ADDR_LEN);

	networkAddr[0] = NAT25_IPV6;
	memcpy(networkAddr+1, (unsigned char *)ipAddr, 16);
}


static unsigned char *scan_tlv(unsigned char *data, int len, unsigned char tag, unsigned char len8b)
{
	while (len > 0) {
		if (*data == tag && *(data+1) == len8b && len >= len8b*8)
			return data+2;

		len -= (*(data+1))*8;
		data += (*(data+1))*8;
	}
	return NULL;
}


#ifdef __ECOS
static int update_nd_link_layer_addr(unsigned char *data, int len, unsigned char *replace_mac, unsigned char *org_mac)
#else
static int update_nd_link_layer_addr(unsigned char *data, int len, unsigned char *replace_mac)
#endif
{
	struct icmp6hdr *icmphdr = (struct icmp6hdr *)data;
	unsigned char *mac;

	if (icmphdr->icmp6_type == NDISC_ROUTER_SOLICITATION) {
		if (len >= 8) {
			mac = scan_tlv(&data[8], len-8, 1, 1);
			if (mac) {
				_DEBUG_INFO("Router Solicitation, replace MAC From: %02x:%02x:%02x:%02x:%02x:%02x, To: %02x:%02x:%02x:%02x:%02x:%02x\n",
						mac[0],mac[1],mac[2],mac[3],mac[4],mac[5],
						replace_mac[0],replace_mac[1],replace_mac[2],replace_mac[3],replace_mac[4],replace_mac[5]);
#ifdef __ECOS
				memcpy(org_mac, mac, 6);
#endif
				memcpy(mac, replace_mac, 6);
				return 1;
			}
		}
	}
	else if (icmphdr->icmp6_type == NDISC_ROUTER_ADVERTISEMENT) {
		if (len >= 16) {
			mac = scan_tlv(&data[16], len-16, 1, 1);
			if (mac) {
				_DEBUG_INFO("Router Advertisement, replace MAC From: %02x:%02x:%02x:%02x:%02x:%02x, To: %02x:%02x:%02x:%02x:%02x:%02x\n",
						mac[0],mac[1],mac[2],mac[3],mac[4],mac[5],
						replace_mac[0],replace_mac[1],replace_mac[2],replace_mac[3],replace_mac[4],replace_mac[5]);
#ifdef __ECOS
				memcpy(org_mac, mac, 6);
#endif
				memcpy(mac, replace_mac, 6);
				return 1;
			}
		}
	}
	else if (icmphdr->icmp6_type == NDISC_NEIGHBOUR_SOLICITATION) {
		if (len >= 24) {
			mac = scan_tlv(&data[24], len-24, 1, 1);
			if (mac) {
				_DEBUG_INFO("Neighbor Solicitation, replace MAC From: %02x:%02x:%02x:%02x:%02x:%02x, To: %02x:%02x:%02x:%02x:%02x:%02x\n",
						mac[0],mac[1],mac[2],mac[3],mac[4],mac[5],
						replace_mac[0],replace_mac[1],replace_mac[2],replace_mac[3],replace_mac[4],replace_mac[5]);
#ifdef __ECOS
				memcpy(org_mac, mac, 6);
#endif
				memcpy(mac, replace_mac, 6);
				return 1;
			}
		}
	}
	else if (icmphdr->icmp6_type == NDISC_NEIGHBOUR_ADVERTISEMENT) {
		if (len >= 24) {
			mac = scan_tlv(&data[24], len-24, 2, 1);
			if (mac) {
				_DEBUG_INFO("Neighbor Advertisement, replace MAC From: %02x:%02x:%02x:%02x:%02x:%02x, To: %02x:%02x:%02x:%02x:%02x:%02x\n",
						mac[0],mac[1],mac[2],mac[3],mac[4],mac[5],
						replace_mac[0],replace_mac[1],replace_mac[2],replace_mac[3],replace_mac[4],replace_mac[5]);
#ifdef __ECOS
				memcpy(org_mac, mac, 6);
#endif
				memcpy(mac, replace_mac, 6);
				return 1;
			}
		}
	}
	else if (icmphdr->icmp6_type == NDISC_REDIRECT) {
		if (len >= 40) {
			mac = scan_tlv(&data[40], len-40, 2, 1);
			if (mac) {
				_DEBUG_INFO("Redirect,  replace MAC From: %02x:%02x:%02x:%02x:%02x:%02x, To: %02x:%02x:%02x:%02x:%02x:%02x\n",
						mac[0],mac[1],mac[2],mac[3],mac[4],mac[5],
						replace_mac[0],replace_mac[1],replace_mac[2],replace_mac[3],replace_mac[4],replace_mac[5]);
#ifdef __ECOS
				memcpy(org_mac, mac, 6);
#endif
				memcpy(mac, replace_mac, 6);
				return 1;
			}
		}
	}
	return 0;
}


#ifdef SUPPORT_RX_UNI2MCAST
static void convert_ipv6_mac_to_mc(struct sk_buff *skb)
{
	struct ipv6hdr *iph = (struct ipv6hdr *)(skb->data + ETH_HLEN);
	unsigned char *dst_mac = skb->data;

	//dst_mac[0] = 0xff;
	//dst_mac[1] = 0xff;
	/*modified by qinjunjie,ipv6 multicast address ix 0x33-33-xx-xx-xx-xx*/
	dst_mac[0] = 0x33;
	dst_mac[1] = 0x33;
	memcpy(&dst_mac[2], &iph->daddr.s6_addr32[3], 4);
#if defined(__LINUX_2_6__)
	/*modified by qinjunjie,warning:should not remove next line*/
	skb->pkt_type = PACKET_MULTICAST;
#endif
}
#endif
#endif /* CL_IPV6_PASS */


static __inline__ int __nat25_network_hash(unsigned char *networkAddr)
{
	if(networkAddr[0] == NAT25_IPV4)
	{
		unsigned long x;

		x = networkAddr[7] ^ networkAddr[8] ^ networkAddr[9] ^ networkAddr[10];

		return x & (NAT25_HASH_SIZE - 1);
	}
	else if(networkAddr[0] == NAT25_IPX)
	{
		unsigned long x;

		x = networkAddr[1] ^ networkAddr[2] ^ networkAddr[3] ^ networkAddr[4] ^ networkAddr[5] ^
			networkAddr[6] ^ networkAddr[7] ^ networkAddr[8] ^ networkAddr[9] ^ networkAddr[10];

		return x & (NAT25_HASH_SIZE - 1);
	}
	else if(networkAddr[0] == NAT25_APPLE)
	{
		unsigned long x;

		x = networkAddr[1] ^ networkAddr[2] ^ networkAddr[3];

		return x & (NAT25_HASH_SIZE - 1);
	}
	else if(networkAddr[0] == NAT25_PPPOE)
	{
		unsigned long x;

		x = networkAddr[0] ^ networkAddr[1] ^ networkAddr[2] ^ networkAddr[3] ^ networkAddr[4] ^ networkAddr[5] ^ networkAddr[6] ^ networkAddr[7] ^ networkAddr[8];

		return x & (NAT25_HASH_SIZE - 1);
	}
#ifdef CL_IPV6_PASS
	else if(networkAddr[0] == NAT25_IPV6)
	{
		unsigned long x;

		x = networkAddr[1] ^ networkAddr[2] ^ networkAddr[3] ^ networkAddr[4] ^ networkAddr[5] ^
			networkAddr[6] ^ networkAddr[7] ^ networkAddr[8] ^ networkAddr[9] ^ networkAddr[10] ^
			networkAddr[11] ^ networkAddr[12] ^ networkAddr[13] ^ networkAddr[14] ^ networkAddr[15] ^
			networkAddr[16];

		return x & (NAT25_HASH_SIZE - 1);
	}
#endif
	else
	{
		unsigned long x = 0;
		int i;

		for (i=0; i<MAX_NETWORK_ADDR_LEN; i++)
			x ^= networkAddr[i];

		return x & (NAT25_HASH_SIZE - 1);
	}
}


static __inline__ void __network_hash_link(struct rtl8192cd_priv *priv,
		struct nat25_network_db_entry *ent, int hash)
{
	ent->next_hash = priv->nethash[hash];
	if(ent->next_hash != NULL)
		ent->next_hash->pprev_hash = &ent->next_hash;
	priv->nethash[hash] = ent;
	ent->pprev_hash = &priv->nethash[hash];
}


static __inline__ void __network_hash_unlink(struct nat25_network_db_entry *ent)
{
	*(ent->pprev_hash) = ent->next_hash;
	if(ent->next_hash != NULL)
		ent->next_hash->pprev_hash = ent->pprev_hash;
	ent->next_hash = NULL;
	ent->pprev_hash = NULL;
}


static int __nat25_db_network_lookup_and_replace(struct rtl8192cd_priv *priv,
		struct sk_buff *skb, unsigned char *networkAddr)
{
	struct nat25_network_db_entry *db;

	db = priv->nethash[__nat25_network_hash(networkAddr)];
	while (db != NULL)
	{
		if(!memcmp(db->networkAddr, networkAddr, MAX_NETWORK_ADDR_LEN))
		{
			if(!__nat25_has_expired(priv, db))
			{
				// replace the destination mac address
				memcpy(skb->data, db->macAddr, ETH_ALEN);
				atomic_inc(&db->use_count);

#ifdef CL_IPV6_PASS
				DEBUG_INFO("NAT25: Lookup M:%02x%02x%02x%02x%02x%02x N:%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x"
						"%02x%02x%02x%02x%02x%02x\n",
						db->macAddr[0],
						db->macAddr[1],
						db->macAddr[2],
						db->macAddr[3],
						db->macAddr[4],
						db->macAddr[5],
						db->networkAddr[0],
						db->networkAddr[1],
						db->networkAddr[2],
						db->networkAddr[3],
						db->networkAddr[4],
						db->networkAddr[5],
						db->networkAddr[6],
						db->networkAddr[7],
						db->networkAddr[8],
						db->networkAddr[9],
						db->networkAddr[10],
						db->networkAddr[11],
						db->networkAddr[12],
						db->networkAddr[13],
						db->networkAddr[14],
						db->networkAddr[15],
						db->networkAddr[16]);
#else
				DEBUG_INFO("NAT25: Lookup M:%02x%02x%02x%02x%02x%02x N:%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x\n",
						db->macAddr[0],
						db->macAddr[1],
						db->macAddr[2],
						db->macAddr[3],
						db->macAddr[4],
						db->macAddr[5],
						db->networkAddr[0],
						db->networkAddr[1],
						db->networkAddr[2],
						db->networkAddr[3],
						db->networkAddr[4],
						db->networkAddr[5],
						db->networkAddr[6],
						db->networkAddr[7],
						db->networkAddr[8],
						db->networkAddr[9],
						db->networkAddr[10]);
#endif
			}
			return 1;
		}

		db = db->next_hash;
	}

	return 0;
}


static void __nat25_db_network_insert(struct rtl8192cd_priv *priv,
		unsigned char *macAddr, unsigned char *networkAddr)
{
	struct nat25_network_db_entry *db;
	int hash;

	hash = __nat25_network_hash(networkAddr);
	db = priv->nethash[hash];
	while (db != NULL)
	{
		if(!memcmp(db->networkAddr, networkAddr, MAX_NETWORK_ADDR_LEN))
		{
			memcpy(db->macAddr, macAddr, ETH_ALEN);
			db->ageing_timer = jiffies;
			return;
		}

		db = db->next_hash;
	}

#ifdef __ECOS
	db = nat25_db_get();
#else
	db = kmalloc(sizeof(*db), GFP_ATOMIC);
#endif
	if(db == NULL) {
#ifdef __ECOS
		diag_printf("run out  nat25  table!\n");
#endif
		return;
	}

	memcpy(db->networkAddr, networkAddr, MAX_NETWORK_ADDR_LEN);
	memcpy(db->macAddr, macAddr, ETH_ALEN);
	atomic_set(&db->use_count, 1);
	db->ageing_timer = jiffies;

	__network_hash_link(priv, db, hash);
}


#ifdef  CONFIG_RTL_ULINKER	
int __nat25_db_query(struct rtl8192cd_priv *priv , unsigned char* MacAddr)
{

	static int counter = 0;
	int i, j , retVal;
	struct nat25_network_db_entry *db;
	retVal=0;

	counter++;
	if((counter <16))
		return 0;


	for(i=0, j=0; i<NAT25_HASH_SIZE; i++)
	{
		db = priv->nethash[i];
		while (db != NULL)
		{
			if(!memcmp(db->macAddr, MacAddr, MACADDRLEN)){
				return 1;
			}
			j++;
			db = db->next_hash;
		}
	}

	return 0;
}
#endif

static void __nat25_db_print(struct rtl8192cd_priv *priv)
{
#ifdef BR_EXT_DEBUG
	static int counter = 0;
	int i, j;
	struct nat25_network_db_entry *db;

	counter++;
	if((counter % 16) != 0)
		return;

	for(i=0, j=0; i<NAT25_HASH_SIZE; i++)
	{
		db = priv->nethash[i];

		while (db != NULL)
		{
#ifdef CL_IPV6_PASS
			panic_printk("NAT25: DB(%d) H(%02d) C(%d) M:%02x%02x%02x%02x%02x%02x N:%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x"
					"%02x%02x%02x%02x%02x%02x\n",
					j,
					i,
					atomic_read(&db->use_count),
					db->macAddr[0],
					db->macAddr[1],
					db->macAddr[2],
					db->macAddr[3],
					db->macAddr[4],
					db->macAddr[5],
					db->networkAddr[0],
					db->networkAddr[1],
					db->networkAddr[2],
					db->networkAddr[3],
					db->networkAddr[4],
					db->networkAddr[5],
					db->networkAddr[6],
					db->networkAddr[7],
					db->networkAddr[8],
					db->networkAddr[9],
					db->networkAddr[10],
					db->networkAddr[11],
					db->networkAddr[12],
					db->networkAddr[13],
					db->networkAddr[14],
					db->networkAddr[15],
					db->networkAddr[16]);
#else
			panic_printk("NAT25: DB(%d) H(%02d) C(%d) M:%02x%02x%02x%02x%02x%02x N:%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x\n",
					j,
					i,
					atomic_read(&db->use_count),
					db->macAddr[0],
					db->macAddr[1],
					db->macAddr[2],
					db->macAddr[3],
					db->macAddr[4],
					db->macAddr[5],
					db->networkAddr[0],
					db->networkAddr[1],
					db->networkAddr[2],
					db->networkAddr[3],
					db->networkAddr[4],
					db->networkAddr[5],
					db->networkAddr[6],
					db->networkAddr[7],
					db->networkAddr[8],
					db->networkAddr[9],
					db->networkAddr[10]);
#endif
			j++;

			db = db->next_hash;
		}
	}
#endif
}




/*
 *	NAT2.5 interface
 */

void nat25_db_cleanup(struct rtl8192cd_priv *priv)
{
	int i;

	for(i=0; i<NAT25_HASH_SIZE; i++)
	{
		struct nat25_network_db_entry *f;
		f = priv->nethash[i];
		while (f != NULL) {
			struct nat25_network_db_entry *g;

			g = f->next_hash;
			__network_hash_unlink(f);
#ifdef __ECOS
			nat25_db_free(f);
#else
			kfree(f);
#endif
			f = g;
		}
	}
}


void nat25_db_expire(struct rtl8192cd_priv *priv)
{
	int i;

	if(!priv->pmib->ethBrExtInfo.nat25_disable)
	{
		for (i=0; i<NAT25_HASH_SIZE; i++)
		{
			struct nat25_network_db_entry *f;
			f = priv->nethash[i];

			while (f != NULL)
			{
				struct nat25_network_db_entry *g;
				g = f->next_hash;

				if(__nat25_has_expired(priv, f))
				{
					if(atomic_dec_and_test(&f->use_count))
					{
#ifdef BR_EXT_DEBUG
#ifdef CL_IPV6_PASS
						panic_printk("NAT25 Expire H(%02d) M:%02x%02x%02x%02x%02x%02x N:%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x"
								"%02x%02x%02x%02x%02x%02x\n",
								i,
								f->macAddr[0],
								f->macAddr[1],
								f->macAddr[2],
								f->macAddr[3],
								f->macAddr[4],
								f->macAddr[5],
								f->networkAddr[0],
								f->networkAddr[1],
								f->networkAddr[2],
								f->networkAddr[3],
								f->networkAddr[4],
								f->networkAddr[5],
								f->networkAddr[6],
								f->networkAddr[7],
								f->networkAddr[8],
								f->networkAddr[9],
								f->networkAddr[10],
								f->networkAddr[11],
								f->networkAddr[12],
								f->networkAddr[13],
								f->networkAddr[14],
								f->networkAddr[15],
								f->networkAddr[16]);
#else

						panic_printk("NAT25 Expire H(%02d) M:%02x%02x%02x%02x%02x%02x N:%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x\n",
								i,
								f->macAddr[0],
								f->macAddr[1],
								f->macAddr[2],
								f->macAddr[3],
								f->macAddr[4],
								f->macAddr[5],
								f->networkAddr[0],
								f->networkAddr[1],
								f->networkAddr[2],
								f->networkAddr[3],
								f->networkAddr[4],
								f->networkAddr[5],
								f->networkAddr[6],
								f->networkAddr[7],
								f->networkAddr[8],
								f->networkAddr[9],
								f->networkAddr[10]);
#endif
#endif
						if (!memcmp(priv->scdb_mac, f->macAddr, ETH_ALEN)) {
							memset(priv->scdb_mac, 0, ETH_ALEN);
							memset(priv->scdb_ip, 0, 4);
						}
						__network_hash_unlink(f);
#ifdef __ECOS
						nat25_db_free(f);
#else
						kfree(f);
#endif
					}
				}

				f = g;
			}
		}
	}
}


#ifdef SUPPORT_TX_MCAST2UNI
#ifdef __ECOS
static int checkIPMcAndReplace(struct rtl8192cd_priv *priv, struct sk_buff *skb, unsigned char *dst_ip)
#else
static int checkIPMcAndReplace(struct rtl8192cd_priv *priv, struct sk_buff *skb, unsigned int *dst_ip)
#endif
{
	struct stat_info	*pstat;
	struct list_head	*phead, *plist;
	int i, result = 0;
#ifdef SMP_SYNC
	unsigned long flags = 0;
#endif

	phead = &priv->asoc_list;

	SMP_LOCK_ASOC_LIST(flags);

	plist = phead->next;
	while (plist != phead) {
		pstat = list_entry(plist, struct stat_info, asoc_list);
		plist = plist->next;

		if (pstat->ipmc_num == 0)
			continue;

		for (i=0; i<MAX_IP_MC_ENTRY; i++) {
			if (pstat->ipmc[i].used && !memcmp(&pstat->ipmc[i].mcmac[3], ((unsigned char *)dst_ip)+1, 3)) {
				memcpy(skb->data, pstat->ipmc[i].mcmac, ETH_ALEN);
				result = 1;
				goto exit;
			}
		}
	}

exit:
	SMP_UNLOCK_ASOC_LIST(flags);

	return result;
}
#endif

#ifdef MULTI_MAC_CLONE
// return 0: not replaced
//        1: dst mac replaced
int mclone_checkMacAndReplace(struct rtl8192cd_priv *priv, struct sk_buff *skb)
{
	unsigned char *da=skb->data;
	int id = mclone_find_address(priv, da, NULL, MAC_CLONE_NOCARE_FIND);

	if (id > 0) {
		memcpy(skb->data, priv->pshare->mclone_sta[id-1].sa_addr, ETH_ALEN);
		return 1;
	}

	return 0;
}
#endif

int nat25_db_handle(struct rtl8192cd_priv *priv, struct sk_buff *skb, int method)
{
	unsigned short protocol;
	unsigned char networkAddr[MAX_NETWORK_ADDR_LEN];
	int macclone_da=0;

	if(skb == NULL)
		return -1;

	if((method <= NAT25_MIN) || (method >= NAT25_MAX))
		return -1;

	if(priv->pmib->ethBrExtInfo.macclone_enable && priv->macclone_completed){
			//WSC_DEBUG("NAT25:no handle macclone packet\n");
			macclone_da=1;
	}
	protocol = *((unsigned short *)(skb->data + 2 * ETH_ALEN));

	/*---------------------------------------------------*/
	/*                 Handle IP frame                   */
	/*---------------------------------------------------*/
	if(protocol == __constant_htons(ETH_P_IP))
	{
		struct iphdr* iph = (struct iphdr *)(skb->data + ETH_HLEN);

		if(((unsigned char*)(iph) + (iph->ihl<<2)) >= (skb->data + ETH_HLEN + skb->len))
		{
			DEBUG_WARN("NAT25: malformed IP packet !\n");
			return -1;
		}

		switch(method)
		{
			case NAT25_CHECK:
				if (IS_MCAST(skb->data))
					return 0;
				else
					return -1;

			case NAT25_INSERT:
				{
					DEBUG_INFO("NAT25: Insert IP, SA=%08x, DA=%08x\n", iph->saddr, iph->daddr);
#ifdef __ECOS
					__nat25_generate_ipv4_network_addr(networkAddr, (unsigned char *)&iph->saddr);
#else
					__nat25_generate_ipv4_network_addr(networkAddr, &iph->saddr);
#endif
					__nat25_db_network_insert(priv, skb->data+ETH_ALEN, networkAddr);

					__nat25_db_print(priv);
				}
				return 0;

			case NAT25_LOOKUP:
				{
					DEBUG_INFO("NAT25: Lookup IP, SA=%08x, DA=%08x\n", iph->saddr, iph->daddr);
#ifdef SUPPORT_TX_MCAST2UNI
#ifdef MULTI_MAC_CLONE
					if (mclone_checkMacAndReplace(priv, skb))
						;
					else
#endif
					if (priv->pshare->rf_ft_var.mc2u_disable ||
							((((OPMODE & (WIFI_STATION_STATE|WIFI_ASOC_STATE))
							   == (WIFI_STATION_STATE|WIFI_ASOC_STATE)) &&
							  !checkIPMcAndReplace(priv, skb, &iph->daddr)) ||
							 (OPMODE & WIFI_ADHOC_STATE)))
#endif
					{
#ifdef __ECOS
						__nat25_generate_ipv4_network_addr(networkAddr, (unsigned char *)&iph->daddr);
#else
						__nat25_generate_ipv4_network_addr(networkAddr, &iph->daddr);
#endif
						if (!__nat25_db_network_lookup_and_replace(priv, skb, networkAddr)) {
							if (*((unsigned char *)&iph->daddr + 3) == 0xff) {
								// L2 is unicast but L3 is broadcast, make L2 bacome broadcast
								DEBUG_INFO("NAT25: Set DA as boardcast\n");
								memset(skb->data, 0xff, ETH_ALEN);
							}
							else {

								if(macclone_da){
									/*RX path ,the maccloned packet don't Replace it's DA with BR's MAC*/
								}else
								{	// NAT25 only (maclone is disabled)
									// forward unknow IP packet to upper TCP/IP
									DEBUG_INFO("NAT25: Replace DA with BR's MAC\n");
									memcpy(skb->data, priv->br_mac, ETH_ALEN);
								}

							}
						}
					}
#ifdef MULTI_MAC_CLONE
					mclone_dhcp_caddr(priv, skb);
#endif
					dhcp_dst_bcast(priv, skb);
				}
				return 0;

			default:
				return -1;
		}
	}

	/*---------------------------------------------------*/
	/*                 Handle ARP frame                  */
	/*---------------------------------------------------*/
	else if(protocol == __constant_htons(ETH_P_ARP))
	{
		struct arphdr *arp = (struct arphdr *)(skb->data + ETH_HLEN);
		unsigned char *arp_ptr = (unsigned char *)(arp + 1);
		unsigned int *sender, *target;

		if(arp->ar_pro != __constant_htons(ETH_P_IP))
		{
			DEBUG_WARN("NAT25: arp protocol unknown (%4x)!\n", htons(arp->ar_pro));
			return -1;
		}

		switch(method)
		{
			case NAT25_CHECK:
				return 0;	// skb_copy for all ARP frame

			case NAT25_INSERT:
				{
					DEBUG_INFO("NAT25: Insert ARP, MAC=%02x%02x%02x%02x%02x%02x\n", arp_ptr[0],
							arp_ptr[1], arp_ptr[2], arp_ptr[3], arp_ptr[4], arp_ptr[5]);

					// change to ARP sender mac address to wlan STA address
					memcpy(arp_ptr, GET_MY_HWADDR, ETH_ALEN);

					arp_ptr += arp->ar_hln;
					sender = (unsigned int *)arp_ptr;
#ifdef __ECOS
					__nat25_generate_ipv4_network_addr(networkAddr, (unsigned char *)sender);
#else
					__nat25_generate_ipv4_network_addr(networkAddr, sender);
#endif
					__nat25_db_network_insert(priv, skb->data+ETH_ALEN, networkAddr);

					__nat25_db_print(priv);
				}
				return 0;

			case NAT25_LOOKUP:
				{
					DEBUG_INFO("NAT25: Lookup ARP\n");

					arp_ptr += arp->ar_hln;
					sender = (unsigned int *)arp_ptr;
					arp_ptr += (arp->ar_hln + arp->ar_pln);
					target = (unsigned int *)arp_ptr;
#ifdef __ECOS
					__nat25_generate_ipv4_network_addr(networkAddr, (unsigned char *)target);
#else
					__nat25_generate_ipv4_network_addr(networkAddr, target);
#endif
					__nat25_db_network_lookup_and_replace(priv, skb, networkAddr);

					// change to ARP target mac address to Lookup result
					arp_ptr = (unsigned char *)(arp + 1);
					arp_ptr += (arp->ar_hln + arp->ar_pln);
					memcpy(arp_ptr, skb->data, ETH_ALEN);
				}
				return 0;

			default:
				return -1;
		}
	}

	/*---------------------------------------------------*/
	/*         Handle IPX and Apple Talk frame           */
	/*---------------------------------------------------*/
	else if((protocol == __constant_htons(ETH_P_IPX)) ||
			(ntohs(protocol) <= ETH_FRAME_LEN))
	{
		unsigned char ipx_header[2] = {0xFF, 0xFF};
		struct ipxhdr	*ipx = NULL;
		struct elapaarp	*ea = NULL;
		struct ddpehdr	*ddp = NULL;
		unsigned char *framePtr = skb->data + ETH_HLEN;

		if(protocol == __constant_htons(ETH_P_IPX))
		{
			DEBUG_INFO("NAT25: Protocol=IPX (Ethernet II)\n");
			ipx = (struct ipxhdr *)framePtr;
		}
		else if(ntohs(protocol) <= ETH_FRAME_LEN)
		{
			if(!memcmp(ipx_header, framePtr, 2))
			{
				DEBUG_INFO("NAT25: Protocol=IPX (Ethernet 802.3)\n");
				ipx = (struct ipxhdr *)framePtr;
			}
			else
			{
				unsigned char ipx_8022_type =  0xE0;
				unsigned char snap_8022_type = 0xAA;

				if(*framePtr == snap_8022_type)
				{
					unsigned char ipx_snap_id[5] = {0x0, 0x0, 0x0, 0x81, 0x37};		// IPX SNAP ID
					unsigned char aarp_snap_id[5] = {0x00, 0x00, 0x00, 0x80, 0xF3};	// Apple Talk AARP SNAP ID
					unsigned char ddp_snap_id[5] = {0x08, 0x00, 0x07, 0x80, 0x9B};	// Apple Talk DDP SNAP ID

					framePtr += 3;	// eliminate the 802.2 header

					if(!memcmp(ipx_snap_id, framePtr, 5))
					{
						framePtr += 5;	// eliminate the SNAP header

						DEBUG_INFO("NAT25: Protocol=IPX (Ethernet SNAP)\n");
						ipx = (struct ipxhdr *)framePtr;
					}
					else if(!memcmp(aarp_snap_id, framePtr, 5))
					{
						framePtr += 5;	// eliminate the SNAP header

						ea = (struct elapaarp *)framePtr;
					}
					else if(!memcmp(ddp_snap_id, framePtr, 5))
					{
						framePtr += 5;	// eliminate the SNAP header

						ddp = (struct ddpehdr *)framePtr;
					}
					else
					{
						DEBUG_WARN("NAT25: Protocol=Ethernet SNAP %02x%02x%02x%02x%02x\n", framePtr[0],
								framePtr[1], framePtr[2], framePtr[3], framePtr[4]);
						return -1;
					}
				}
				else if(*framePtr == ipx_8022_type)
				{
					framePtr += 3;	// eliminate the 802.2 header

					if(!memcmp(ipx_header, framePtr, 2))
					{
						DEBUG_INFO("NAT25: Protocol=IPX (Ethernet 802.2)\n");
						ipx = (struct ipxhdr *)framePtr;
					}
					else
						return -1;
				}
				else
					return -1;
			}
		}
		else
			return -1;

		/*   IPX   */
		if(ipx != NULL)
		{
			switch(method)
			{
				case NAT25_CHECK:
					if(!memcmp(skb->data+ETH_ALEN, ipx->ipx_source.node, ETH_ALEN))
					{
						DEBUG_INFO("NAT25: Check IPX skb_copy\n");
						return 0;
					}
					return -1;

				case NAT25_INSERT:
					{
						DEBUG_INFO("NAT25: Insert IPX, Dest=%08x,%02x%02x%02x%02x%02x%02x,%04x Source=%08x,%02x%02x%02x%02x%02x%02x,%04x\n",
								ipx->ipx_dest.net,
								ipx->ipx_dest.node[0],
								ipx->ipx_dest.node[1],
								ipx->ipx_dest.node[2],
								ipx->ipx_dest.node[3],
								ipx->ipx_dest.node[4],
								ipx->ipx_dest.node[5],
								ipx->ipx_dest.sock,
								ipx->ipx_source.net,
								ipx->ipx_source.node[0],
								ipx->ipx_source.node[1],
								ipx->ipx_source.node[2],
								ipx->ipx_source.node[3],
								ipx->ipx_source.node[4],
								ipx->ipx_source.node[5],
								ipx->ipx_source.sock);

						if(!memcmp(skb->data+ETH_ALEN, ipx->ipx_source.node, ETH_ALEN))
						{
							DEBUG_INFO("NAT25: Use IPX Net, and Socket as network addr\n");
#ifdef __ECOS
							__nat25_generate_ipx_network_addr_with_socket(networkAddr, (unsigned char *)&ipx->ipx_source.net, (unsigned char *)&ipx->ipx_source.sock);
#else
							__nat25_generate_ipx_network_addr_with_socket(networkAddr, &ipx->ipx_source.net, &ipx->ipx_source.sock);
#endif
							// change IPX source node addr to wlan STA address
							memcpy(ipx->ipx_source.node, GET_MY_HWADDR, ETH_ALEN);
						}
						else
						{
#ifdef __ECOS
							__nat25_generate_ipx_network_addr_with_node(networkAddr, (unsigned char *)&ipx->ipx_source.net, ipx->ipx_source.node);
#else
							__nat25_generate_ipx_network_addr_with_node(networkAddr, &ipx->ipx_source.net, ipx->ipx_source.node);
#endif
						}

						__nat25_db_network_insert(priv, skb->data+ETH_ALEN, networkAddr);

						__nat25_db_print(priv);
					}
					return 0;

				case NAT25_LOOKUP:
					{
						if(!memcmp(GET_MY_HWADDR, ipx->ipx_dest.node, ETH_ALEN))
						{
							DEBUG_INFO("NAT25: Lookup IPX, Modify Destination IPX Node addr\n");
#ifdef __ECOS
							__nat25_generate_ipx_network_addr_with_socket(networkAddr, (unsigned char *)&ipx->ipx_dest.net, (unsigned char *)&ipx->ipx_dest.sock);
#else
							__nat25_generate_ipx_network_addr_with_socket(networkAddr, &ipx->ipx_dest.net, &ipx->ipx_dest.sock);
#endif
							__nat25_db_network_lookup_and_replace(priv, skb, networkAddr);

							// replace IPX destination node addr with Lookup destination MAC addr
							memcpy(ipx->ipx_dest.node, skb->data, ETH_ALEN);
						}
						else
						{
#ifdef __ECOS
							__nat25_generate_ipx_network_addr_with_node(networkAddr, (unsigned char *)&ipx->ipx_dest.net, ipx->ipx_dest.node);
#else
							__nat25_generate_ipx_network_addr_with_node(networkAddr, &ipx->ipx_dest.net, ipx->ipx_dest.node);
#endif
							__nat25_db_network_lookup_and_replace(priv, skb, networkAddr);
						}
					}
					return 0;

				default:
					return -1;
			}
		}

		/*   AARP   */
		else if(ea != NULL)
		{
			/* Sanity check fields. */
			if(ea->hw_len != ETH_ALEN || ea->pa_len != AARP_PA_ALEN)
			{
				DEBUG_WARN("NAT25: Appletalk AARP Sanity check fail!\n");
				return -1;
			}

			switch(method)
			{
				case NAT25_CHECK:
					return 0;

				case NAT25_INSERT:
					{
						// change to AARP source mac address to wlan STA address
						memcpy(ea->hw_src, GET_MY_HWADDR, ETH_ALEN);

						DEBUG_INFO("NAT25: Insert AARP, Source=%d,%d Destination=%d,%d\n",
								ea->pa_src_net,
								ea->pa_src_node,
								ea->pa_dst_net,
								ea->pa_dst_node);
#ifdef __ECOS
						__nat25_generate_apple_network_addr(networkAddr, (unsigned char *)&ea->pa_src_net, &ea->pa_src_node);
#else
						__nat25_generate_apple_network_addr(networkAddr, &ea->pa_src_net, &ea->pa_src_node);
#endif
						__nat25_db_network_insert(priv, skb->data+ETH_ALEN, networkAddr);

						__nat25_db_print(priv);
					}
					return 0;

				case NAT25_LOOKUP:
					{
						DEBUG_INFO("NAT25: Lookup AARP, Source=%d,%d Destination=%d,%d\n",
								ea->pa_src_net,
								ea->pa_src_node,
								ea->pa_dst_net,
								ea->pa_dst_node);
#ifdef __ECOS
						__nat25_generate_apple_network_addr(networkAddr, (unsigned char *)&ea->pa_dst_net, &ea->pa_dst_node);
#else
						__nat25_generate_apple_network_addr(networkAddr, &ea->pa_dst_net, &ea->pa_dst_node);
#endif
						__nat25_db_network_lookup_and_replace(priv, skb, networkAddr);

						// change to AARP destination mac address to Lookup result
						memcpy(ea->hw_dst, skb->data, ETH_ALEN);
					}
					return 0;

				default:
					return -1;
			}
		}

		/*   DDP   */
		else if(ddp != NULL)
		{
			switch(method)
			{
				case NAT25_CHECK:
					return -1;

				case NAT25_INSERT:
					{
						DEBUG_INFO("NAT25: Insert DDP, Source=%d,%d Destination=%d,%d\n",
								ddp->deh_snet,
								ddp->deh_snode,
								ddp->deh_dnet,
								ddp->deh_dnode);
#ifdef __ECOS
						__nat25_generate_apple_network_addr(networkAddr, (unsigned char *)&ddp->deh_snet, &ddp->deh_snode);
#else
						__nat25_generate_apple_network_addr(networkAddr, &ddp->deh_snet, &ddp->deh_snode);
#endif
						__nat25_db_network_insert(priv, skb->data+ETH_ALEN, networkAddr);

						__nat25_db_print(priv);
					}
					return 0;

				case NAT25_LOOKUP:
					{
						DEBUG_INFO("NAT25: Lookup DDP, Source=%d,%d Destination=%d,%d\n",
								ddp->deh_snet,
								ddp->deh_snode,
								ddp->deh_dnet,
								ddp->deh_dnode);
#ifdef __ECOS
						__nat25_generate_apple_network_addr(networkAddr, (unsigned char *)&ddp->deh_dnet, &ddp->deh_dnode);
#else
						__nat25_generate_apple_network_addr(networkAddr, &ddp->deh_dnet, &ddp->deh_dnode);
#endif
						__nat25_db_network_lookup_and_replace(priv, skb, networkAddr);
					}
					return 0;

				default:
					return -1;
			}
		}

		return -1;
	}

	/*---------------------------------------------------*/
	/*                Handle PPPoE frame                 */
	/*---------------------------------------------------*/
	else if((protocol == __constant_htons(ETH_P_PPP_DISC)) ||
			(protocol == __constant_htons(ETH_P_PPP_SES)))
	{
		struct pppoe_hdr *ph = (struct pppoe_hdr *)(skb->data + ETH_HLEN);
		unsigned short *pMagic;
#if defined(__ECOS) || defined(__OSK__)
		unsigned short num;
		char *tmp_magic;
#endif

		switch(method)
		{
			case NAT25_CHECK:
				if (ph->sid == 0)
					return 0;
				return 1;

			case NAT25_INSERT:
				if(ph->sid == 0)	// Discovery phase according to tag
				{
					if(ph->code == PADI_CODE || ph->code == PADR_CODE)
					{
						if (priv->pmib->ethBrExtInfo.addPPPoETag) {
							struct pppoe_tag *tag, *pOldTag;
							unsigned char tag_buf[40];
							int old_tag_len=0;

							tag = (struct pppoe_tag *)tag_buf;
							pOldTag = (struct pppoe_tag *)__nat25_find_pppoe_tag(ph, ntohs(PTT_RELAY_SID));
							if (pOldTag) { // if SID existed, copy old value and delete it
								old_tag_len = ntohs(pOldTag->tag_len);
								if (old_tag_len+TAG_HDR_LEN+MAGIC_CODE_LEN+RTL_RELAY_TAG_LEN > sizeof(tag_buf)) {
									DEBUG_ERR("SID tag length too long!\n");
									return -1;
								}

								memcpy(tag->tag_data+MAGIC_CODE_LEN+RTL_RELAY_TAG_LEN,
										pOldTag->tag_data, old_tag_len);

								if (skb_pull_and_merge(skb, (unsigned char *)pOldTag, TAG_HDR_LEN+old_tag_len) < 0) {
									DEBUG_ERR("call skb_pull_and_merge() failed in PADI/R packet!\n");
									return -1;
								}
								ph->length = htons(ntohs(ph->length)-TAG_HDR_LEN-old_tag_len);
							}

							tag->tag_type = PTT_RELAY_SID;
							tag->tag_len = htons(MAGIC_CODE_LEN+RTL_RELAY_TAG_LEN+old_tag_len);

							// insert the magic_code+client mac in relay tag
#if defined(__ECOS) || defined(__OSK__)
							num = htons(MAGIC_CODE);
							tmp_magic = (char *)&num;
							memcpy(tag->tag_data, tmp_magic, 2);
#else
							pMagic = (unsigned short *)tag->tag_data;
							*pMagic = htons(MAGIC_CODE);
#endif
							memcpy(tag->tag_data+MAGIC_CODE_LEN, skb->data+ETH_ALEN, ETH_ALEN);

							//Add relay tag
							if(__nat25_add_pppoe_tag(skb, tag) < 0)
								return -1;

							DEBUG_INFO("NAT25: Insert PPPoE, forward %s packet\n",
									(ph->code == PADI_CODE ? "PADI" : "PADR"));
						}
						else { // not add relay tag
							if (priv->pppoe_connection_in_progress &&
									memcmp(skb->data+ETH_ALEN, priv->pppoe_addr, ETH_ALEN))	 {
								DEBUG_ERR("Discard PPPoE packet due to another PPPoE connection is in progress!\n");
								return -2;
							}

							if (priv->pppoe_connection_in_progress == 0)
								memcpy(priv->pppoe_addr, skb->data+ETH_ALEN, ETH_ALEN);

							priv->pppoe_connection_in_progress = WAIT_TIME_PPPOE;
						}
					}
					else
						return -1;
				}
				else	// session phase
				{
					DEBUG_INFO("NAT25: Insert PPPoE, insert session packet to %s\n", skb->dev->name);

					__nat25_generate_pppoe_network_addr(networkAddr, skb->data, &(ph->sid));

					__nat25_db_network_insert(priv, skb->data+ETH_ALEN, networkAddr);

					__nat25_db_print(priv);

					if (!priv->pmib->ethBrExtInfo.addPPPoETag &&
							priv->pppoe_connection_in_progress &&
							!memcmp(skb->data+ETH_ALEN, priv->pppoe_addr, ETH_ALEN))
						priv->pppoe_connection_in_progress = 0;
				}
				return 0;

			case NAT25_LOOKUP:
				if(ph->code == PADO_CODE || ph->code == PADS_CODE)
				{
					if (priv->pmib->ethBrExtInfo.addPPPoETag) {
						struct pppoe_tag *tag;
						unsigned char *ptr;
						unsigned short tagType, tagLen;
						int offset=0;

						if((ptr = __nat25_find_pppoe_tag(ph, ntohs(PTT_RELAY_SID))) == 0) {
							DEBUG_ERR("Fail to find PTT_RELAY_SID in FADO!\n");
							return -1;
						}

						tag = (struct pppoe_tag *)ptr;
						tagType = (unsigned short)((ptr[0] << 8) + ptr[1]);
						tagLen = (unsigned short)((ptr[2] << 8) + ptr[3]);

						if((tagType != ntohs(PTT_RELAY_SID)) || (tagLen < (MAGIC_CODE_LEN+RTL_RELAY_TAG_LEN))) {
							DEBUG_ERR("Invalid PTT_RELAY_SID tag length [%d]!\n", tagLen);
							return -1;
						}

#if defined(__ECOS) || defined(__OSK__)
						memcpy(&num, tag->tag_data, 2);
						pMagic = &num;
#else
						pMagic = (unsigned short *)tag->tag_data;
#endif
						if (*pMagic != htons(MAGIC_CODE)) {
							DEBUG_ERR("Can't find MAGIC_CODE in %s packet!\n",
									(ph->code == PADO_CODE ? "PADO" : "PADS"));
							return -1;
						}

						memcpy(skb->data, tag->tag_data+MAGIC_CODE_LEN, ETH_ALEN);

						if (tagLen > MAGIC_CODE_LEN+RTL_RELAY_TAG_LEN)
							offset = TAG_HDR_LEN;

						if (skb_pull_and_merge(skb, ptr+offset, TAG_HDR_LEN+MAGIC_CODE_LEN+RTL_RELAY_TAG_LEN-offset) < 0) {
							DEBUG_ERR("call skb_pull_and_merge() failed in PADO packet!\n");
							return -1;
						}
						ph->length = htons(ntohs(ph->length)-(TAG_HDR_LEN+MAGIC_CODE_LEN+RTL_RELAY_TAG_LEN-offset));
						if (offset > 0)
							tag->tag_len = htons(tagLen-MAGIC_CODE_LEN-RTL_RELAY_TAG_LEN);

						DEBUG_INFO("NAT25: Lookup PPPoE, forward %s Packet from %s\n",
								(ph->code == PADO_CODE ? "PADO" : "PADS"),	skb->dev->name);
					}
					else { // not add relay tag
						if (!priv->pppoe_connection_in_progress) {
							DEBUG_ERR("Discard PPPoE packet due to no connection in progresss!\n");
							return -1;
						}
						memcpy(skb->data, priv->pppoe_addr, ETH_ALEN);
						priv->pppoe_connection_in_progress = WAIT_TIME_PPPOE;
					}
				}
				else {
					if(ph->sid != 0)
					{
						DEBUG_INFO("NAT25: Lookup PPPoE, lookup session packet from %s\n", skb->dev->name);
						__nat25_generate_pppoe_network_addr(networkAddr, skb->data+ETH_ALEN, &(ph->sid));

						__nat25_db_network_lookup_and_replace(priv, skb, networkAddr);

						__nat25_db_print(priv);
					}
					else
						return -1;

				}
				return 0;

			default:
				return -1;
		}
	}

	/*---------------------------------------------------*/
	/*                 Handle EAP frame                  */
	/*---------------------------------------------------*/
	else if(protocol == __constant_htons(0x888e))
	{
		switch(method)
		{
			case NAT25_CHECK:
				return -1;

			case NAT25_INSERT:
				return 0;

			case NAT25_LOOKUP:
				return 0;

			default:
				return -1;
		}
	}

	/*---------------------------------------------------*/
	/*                 Handle LLTD frame                 */
	/*---------------------------------------------------*/
	else if(protocol == __constant_htons(0x88d9))
	{
		switch(method)
		{
			case NAT25_CHECK:
				return -1;

			case NAT25_INSERT:
				return 0;

			case NAT25_LOOKUP:
				return 0;

			default:
				return -1;
		}
	}

	/*---------------------------------------------------*/
	/*         Handle C-Media proprietary frame          */
	/*---------------------------------------------------*/
	else if((protocol == __constant_htons(0xe2ae)) ||
			(protocol == __constant_htons(0xe2af)))
	{
		switch(method)
		{
			case NAT25_CHECK:
				return -1;

			case NAT25_INSERT:
				return 0;

			case NAT25_LOOKUP:
				return 0;

			default:
				return -1;
		}
	}

	/*---------------------------------------------------*/
	/*                 Handle IPV6 frame                 */
	/*---------------------------------------------------*/
#ifdef CL_IPV6_PASS
	else if(protocol == __constant_htons(ETH_P_IPV6))
	{
		struct ipv6hdr *iph = (struct ipv6hdr *)(skb->data + ETH_HLEN);

		if (sizeof(*iph) >= (skb->len - ETH_HLEN))
		{
			DEBUG_WARN("NAT25: malformed IPv6 packet !\n");
			return -1;
		}

		switch(method)
		{
			case NAT25_CHECK:
				if (IS_MCAST(skb->data))
					return 0;
				else
					return -1;

			case NAT25_INSERT:
				{
					DEBUG_INFO("NAT25: Insert IP, SA=%4x:%4x:%4x:%4x:%4x:%4x:%4x:%4x,"
							" DA=%4x:%4x:%4x:%4x:%4x:%4x:%4x:%4x\n",
							iph->saddr.s6_addr16[0],iph->saddr.s6_addr16[1],iph->saddr.s6_addr16[2],iph->saddr.s6_addr16[3],
							iph->saddr.s6_addr16[4],iph->saddr.s6_addr16[5],iph->saddr.s6_addr16[6],iph->saddr.s6_addr16[7],
							iph->daddr.s6_addr16[0],iph->daddr.s6_addr16[1],iph->daddr.s6_addr16[2],iph->daddr.s6_addr16[3],
							iph->daddr.s6_addr16[4],iph->daddr.s6_addr16[5],iph->daddr.s6_addr16[6],iph->daddr.s6_addr16[7]);

					if (memcmp(&iph->saddr, "\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0", 16)) {
#ifdef __ECOS
						__nat25_generate_ipv6_network_addr(networkAddr, (unsigned char *)&iph->saddr);
#else
						__nat25_generate_ipv6_network_addr(networkAddr, (unsigned int *)&iph->saddr);
#endif
						__nat25_db_network_insert(priv, skb->data+ETH_ALEN, networkAddr);
						__nat25_db_print(priv);

						if (iph->nexthdr == IPPROTO_ICMPV6 &&
								skb->len > (ETH_HLEN +  sizeof(*iph) + 4)) {
#ifdef __ECOS
							unsigned char org_mac[6];
							if (update_nd_link_layer_addr(skb->data + ETH_HLEN + sizeof(*iph), 
									skb->len - ETH_HLEN - sizeof(*iph), GET_MY_HWADDR, org_mac)) {
								u16 *p1 = (u16 *)GET_MY_HWADDR;
								u16 *p2 = (u16 *)org_mac;
								struct icmp6hdr  *hdr = (struct icmp6hdr *)(skb->data + ETH_HLEN + sizeof(*iph));
								ADJUST_CHKSUM(*p1++, *p2++, hdr->icmp6_cksum);
								ADJUST_CHKSUM(*p1++, *p2++, hdr->icmp6_cksum);
								ADJUST_CHKSUM(*p1, *p2, hdr->icmp6_cksum);
							}
#else
							if (update_nd_link_layer_addr(skb->data + ETH_HLEN + sizeof(*iph),
										skb->len - ETH_HLEN - sizeof(*iph), GET_MY_HWADDR)) {
								struct icmp6hdr  *hdr = (struct icmp6hdr *)(skb->data + ETH_HLEN + sizeof(*iph));
#ifdef __OSK__                  //add cksum in here, fixme
								int len = natohs(iph->payload_len);
								hdr->icmp6_cksum = 0;
#else
								int len = ntohs(iph->payload_len);
								hdr->icmp6_cksum = 0;
								hdr->icmp6_cksum = csum_ipv6_magic(&iph->saddr, &iph->daddr,
										len,
										IPPROTO_ICMPV6,
										csum_partial((__u8 *)hdr, len, 0));
#endif
							}
#endif
						}
					}
				}
				return 0;

			case NAT25_LOOKUP:
				DEBUG_INFO("NAT25: Lookup IP, SA=%4x:%4x:%4x:%4x:%4x:%4x:%4x:%4x,"
						" DA=%4x:%4x:%4x:%4x:%4x:%4x:%4x:%4x\n",
						iph->saddr.s6_addr16[0],iph->saddr.s6_addr16[1],iph->saddr.s6_addr16[2],iph->saddr.s6_addr16[3],
						iph->saddr.s6_addr16[4],iph->saddr.s6_addr16[5],iph->saddr.s6_addr16[6],iph->saddr.s6_addr16[7],
						iph->daddr.s6_addr16[0],iph->daddr.s6_addr16[1],iph->daddr.s6_addr16[2],iph->daddr.s6_addr16[3],
						iph->daddr.s6_addr16[4],iph->daddr.s6_addr16[5],iph->daddr.s6_addr16[6],iph->daddr.s6_addr16[7]);

#ifdef __ECOS
				__nat25_generate_ipv6_network_addr(networkAddr, (unsigned char *)&iph->daddr);
#else
				__nat25_generate_ipv6_network_addr(networkAddr, (unsigned int *)&iph->daddr);
#endif
				if (!__nat25_db_network_lookup_and_replace(priv, skb, networkAddr)) {
#ifdef SUPPORT_RX_UNI2MCAST
					if (iph->daddr.s6_addr[0] == 0xff)
						convert_ipv6_mac_to_mc(skb);
#endif
				}
				return 0;

			default:
				return -1;
		}
	}
#endif

	/*---------------------------------------------------*/
	/*         Handle all other unknown format           */
	/*---------------------------------------------------*/
	else
	{
		DEBUG_INFO("NAT25: Unknown protocol: 0x%04x\n", ntohs(protocol));
		switch(method)
		{
			case NAT25_CHECK:
				if (IS_MCAST(skb->data))
					return 0;
				else
					return -1;

			case NAT25_INSERT:
				if (memcmp(skb->data+ETH_ALEN, GET_MY_HWADDR, ETH_ALEN) &&
						memcmp(skb->data+ETH_ALEN, priv->br_mac, ETH_ALEN))
				{
					if (memcmp(skb->data+ETH_ALEN, priv->ukpro_mac, ETH_ALEN)) {
						memcpy(priv->ukpro_mac, skb->data+ETH_ALEN, ETH_ALEN);
						priv->ukpro_mac_valid = 1;
						DEBUG_INFO("NAT25: Insert unknown protocol, MAC=%02x%02x%02x%02x%02x%02x\n",
								priv->ukpro_mac[0], priv->ukpro_mac[1], priv->ukpro_mac[2],
								priv->ukpro_mac[3], priv->ukpro_mac[4], priv->ukpro_mac[5]);
					}
				}
				return 0;

			case NAT25_LOOKUP:
				// replace the destination mac address
				if (priv->ukpro_mac_valid) {
					memcpy(skb->data, priv->ukpro_mac, ETH_ALEN);
					DEBUG_INFO("NAT25: Lookup unknown protocol, MAC=%02x%02x%02x%02x%02x%02x\n",
							priv->ukpro_mac[0], priv->ukpro_mac[1], priv->ukpro_mac[2],
							priv->ukpro_mac[3], priv->ukpro_mac[4], priv->ukpro_mac[5]);
				}
				return 0;

			default:
				return -1;
		}
	}
}


int nat25_handle_frame(struct rtl8192cd_priv *priv, struct sk_buff *skb)
{
#ifdef NOT_RTK_BSP
	/* not check EAPOL key because it is not inserted into DB */
	if ( (*((unsigned short *)(skb->data+ETH_ALEN*2)) == __constant_htons(0x888e)) )
		return 0;
#endif
#ifdef BR_EXT_DEBUG
	if((!priv->pmib->ethBrExtInfo.nat25_disable) && (!(skb->data[0] & 1)))
	{
		panic_printk("NAT25: Input Frame: DA=%02x%02x%02x%02x%02x%02x SA=%02x%02x%02x%02x%02x%02x\n",
				skb->data[0],
				skb->data[1],
				skb->data[2],
				skb->data[3],
				skb->data[4],
				skb->data[5],
				skb->data[6],
				skb->data[7],
				skb->data[8],
				skb->data[9],
				skb->data[10],
				skb->data[11]);
	}
#endif
		
	if(!(skb->data[0] & 1))
	{
		int is_vlan_tag=0, i, retval=0;
		unsigned short vlan_hdr=0;

		if (*((unsigned short *)(skb->data+ETH_ALEN*2)) == __constant_htons(ETH_P_8021Q)) {
			is_vlan_tag = 1;
			vlan_hdr = *((unsigned short *)(skb->data+ETH_ALEN*2+2));
			for (i=0; i<6; i++)
				*((unsigned short *)(skb->data+ETH_ALEN*2+2-i*2)) = *((unsigned short *)(skb->data+ETH_ALEN*2-2-i*2));
			skb_pull(skb, 4);
		}

		if (!priv->pmib->ethBrExtInfo.nat25_disable
#ifdef MULTI_MAC_CLONE
			&& ((ACTIVE_ID == 0) || (ACTIVE_ID > 0 && priv->pshare->mclone_sta[ACTIVE_ID-1].usedStaAddrId != 0xff))
#endif
			)
		{
			/*
			 *	This function look up the destination network address from
			 *	the NAT2.5 database. Return value = -1 means that the
			 *	corresponding network protocol is NOT support.
			 */
			if (!priv->pmib->ethBrExtInfo.nat25sc_disable &&
					(*((unsigned short *)(skb->data+ETH_ALEN*2)) == __constant_htons(ETH_P_IP)) &&
					!memcmp(priv->scdb_ip, skb->data+ETH_HLEN+16, 4)) {
				memcpy(skb->data, priv->scdb_mac, ETH_ALEN);
#ifdef MULTI_MAC_CLONE
				mclone_dhcp_caddr(priv, skb);
#endif
			}
			else
				retval = nat25_db_handle(priv, skb, NAT25_LOOKUP);
		}
		else {
			if (((*((unsigned short *)(skb->data+ETH_ALEN*2)) == __constant_htons(ETH_P_IP)) &&
						!memcmp(priv->br_ip, skb->data+ETH_HLEN+16, 4)) ||
					((*((unsigned short *)(skb->data+ETH_ALEN*2)) == __constant_htons(ETH_P_ARP)) &&
					 !memcmp(priv->br_ip, skb->data+ETH_HLEN+24, 4))) {
				// for traffic to upper TCP/IP
				retval = nat25_db_handle(priv, skb, NAT25_LOOKUP);
			}
		}

		if (is_vlan_tag) {
			skb_push(skb, 4);
			for (i=0; i<6; i++)
				*((unsigned short *)(skb->data+i*2)) = *((unsigned short *)(skb->data+4+i*2));
			*((unsigned short *)(skb->data+ETH_ALEN*2)) = __constant_htons(ETH_P_8021Q);
			*((unsigned short *)(skb->data+ETH_ALEN*2+2)) = vlan_hdr;
		}

		if(retval == -1) {
			DEBUG_ERR("NAT25: Lookup fail!\n");
			return -1;
		}
	}
#ifdef MULTI_MAC_CLONE
	else if ((skb!=NULL) && (skb->data[0]==0xff) 
		&& (*((unsigned short *)(skb->data+ETH_ALEN*2))==__constant_htons(ETH_P_IP)))
			mclone_dhcp_caddr(priv, skb);
#endif

	return 0;
}


void mac_clone(struct rtl8192cd_priv *priv, unsigned char *addr)
{
#ifdef __KERNEL__
	struct sockaddr sa;
	memcpy(sa.sa_data, addr, ETH_ALEN);
#endif

	DEBUG_INFO("MAC Clone: Addr=%02x%02x%02x%02x%02x%02x\n",
			addr[0], addr[1], addr[2], addr[3], addr[4], addr[5]);
#ifdef __KERNEL__
	rtl8192cd_set_hwaddr(priv->dev, &sa);
#else
	rtl8192cd_set_hwaddr(priv->dev, (void *)addr);
#endif
}

#ifdef MULTI_MAC_CLONE
unsigned char mclone_find_staFixedAddr(struct rtl8192cd_priv *priv)
{
	unsigned char i;

	for (i=0; i<MCLONE_NUM; i++) {
		if (priv->pshare->mclone_sta_fixed_addr[i].used == 0) 
			return i;						
	}
	return 0xff;
}

unsigned char FindWiFiSta(struct rtl8192cd_priv *inPriv, unsigned char *addr)
{
    struct rtl8192cd_priv *priv;
    struct list_head	*phead, *plist;
    struct stat_info	*pstat;
    unsigned char idx=0;
    
    if (inPriv ==NULL){
        return 0;   
    }
    
    priv = GET_ROOT(inPriv);
    
    phead = &priv->asoc_list;
	plist = phead->next;
	
	for (idx=0; idx<=RTL8192CD_NUM_VWLAN; idx++) {
	    while( ( (((GET_MIB(priv))->dot11OperationEntry.opmode) & WIFI_AP_STATE) == WIFI_AP_STATE ) && (plist != phead))
	    {
		    pstat = list_entry(plist, struct stat_info, asoc_list);
		    plist = plist->next;
            if (pstat && (!memcmp(addr, pstat->hwaddr, ETH_ALEN)) ){
                return 1;
            }
        }
#ifdef MBSSID
        priv = GET_VAP_PRIV(GET_ROOT(priv), idx);
        phead = &priv->asoc_list;
	    plist = phead->next;
#else
        break;
#endif
	}
	
    return 0;    
}

int mclone_find_unused_id(struct rtl8192cd_priv *priv)
{
    int i=0;
    for (i=0; i<priv->pshare->mclone_num_max; i++) {
		if (priv->pshare->mclone_sta[i].priv == NULL) 
			return i+1;
	}
	return -1;
}

//direction-- SA:1, DA:2, donotcare:0
int mclone_find_address(struct rtl8192cd_priv *priv, unsigned char *addr, struct sk_buff *pskb, unsigned char direction)
{
	int i;

	if (direction == MAC_CLONE_SA_FIND){
	    if ((pskb == NULL) || 
            #ifdef __ECOS
            (pskb && *(unsigned int *)&(pskb->cb[8]) == 0x86518190)
            #else
            (pskb && *(unsigned int *)&(pskb->cb[40]) == 0x86518192)
            #endif
            ) {
			// from wifi, src MAC A1 -> A1'
		    // compare with mclone_sta[].sa_addr
			for (i=0; i<priv->pshare->mclone_num_max; i++) {
				if ((priv->pshare->mclone_sta[i].priv) && priv->pshare->mclone_sta[i].usedStaAddrId!=0xff
				        && (!memcmp(addr, priv->pshare->mclone_sta[i].sa_addr, ETH_ALEN))) {
					return i+1;						
				}
			}
		}else{
			// from ethernet, src MAC A1 -> A1
			// compare with mclone_sta[].hwaddr
			// printk("it does not come from wifi%02x%02x%02x%02x%02x%02x\n", addr[0], addr[1], addr[2], addr[3], addr[4], addr[5]);
		}
	}else if (direction == MAC_CLONE_MSA_FIND && pskb){
		for (i=0; i<priv->pshare->mclone_num_max; i++) {
			if (priv->pshare->mclone_sta[i].usedStaAddrId!=0xff && 
				!memcmp(addr, priv->pshare->mclone_sta[i].hwaddr, ETH_ALEN)) 
				return i+1;						
		}
	}else if (direction == MAC_CLONE_DA_FIND && pskb){
	}

	if (!memcmp(addr, priv->pmib->dot11OperationEntry.hwaddr, ETH_ALEN)) 
		return 0;

	for (i=0; i<priv->pshare->mclone_num_max; i++) {
		if ( (priv->pshare->mclone_sta[i].priv) && (!memcmp(addr, priv->pshare->mclone_sta[i].hwaddr, ETH_ALEN)) ) 
			return i+1;						
	}
	
	return -1;
}
#endif

void indicate_wsc_mac_addr_has_changed(struct rtl8192cd_priv *priv){
		/*2015 1008 add for macclone issue*/

	DOT11_WSC_PIN_IND wsc_ind;

	wsc_ind.EventId = DOT11_EVENT_WSC_CHANGE_MAC_IND;
	wsc_ind.IsMoreEvent = 0;
	memcpy((char *)wsc_ind.code, GET_MY_HWADDR,MACADDRLEN);
	
	DOT11_EnQueue((unsigned long)priv, priv->pevent_queue, (unsigned char*)&wsc_ind, sizeof(DOT11_WSC_PIN_IND));
	event_indicate(priv, NULL, -1);

	//panic_printk("[%s %d] ",__func__,__LINE__);
	//printMac(GET_MY_HWADDR);

}
int mac_clone_handle_frame(struct rtl8192cd_priv *priv, struct sk_buff *skb)
{
	struct stat_info	*pstat=NULL;	
	if(priv->pmib->ethBrExtInfo.macclone_enable && !priv->macclone_completed)
	{
		if(!(skb->data[ETH_ALEN] & 1))	//// check any other particular MAC add
		{
#ifdef MULTI_MAC_CLONE
        #ifdef __ECOS
            struct net_device* eth0_dev=rtl_getDevByName("eth0");
            memcpy(priv->br_mac,eth0_dev->dev_addr,6); 
            STADEBUG("macclone_completed =1 \n\n");
            priv->macclone_completed = 1;
	    #else
#if defined( __KERNEL__) || defined(__OSK__)
			struct net_bridge_port *br_port = GET_BR_PORT(priv->dev);

			if (br_port) {
				memcpy(priv->br_mac, br_port->br->dev->dev_addr, MACADDRLEN);
				STADEBUG("br mac=[%02X%02X%02X:%02X%02X%02X]\n",
				priv->br_mac[0],priv->br_mac[1],priv->br_mac[2],priv->br_mac[3],priv->br_mac[4],priv->br_mac[5]);
			}
			STADEBUG("macclone_completed =1 \n\n");
			priv->macclone_completed = 1;
	      #endif
       #endif
#else // !MULTI_MAC_CLONE		
			pstat = get_stainfo(priv, skb->data+ETH_ALEN);
			if(memcmp(skb->data+ETH_ALEN, GET_MY_HWADDR, ETH_ALEN) && (
#if defined( __KERNEL__) || defined(__OSK__)
				GET_BR_PORT(priv->dev) &&
                #endif
				 memcmp(skb->data+ETH_ALEN, priv->br_mac, ETH_ALEN) && 
				 (pstat==NULL)))
			{
#if defined( __KERNEL__) || defined(__OSK__)
                #ifdef  CONFIG_RTL_ULINKER
				if(__nat25_db_query(priv , priv->br_mac))
                #endif                    
                #endif
				{
#if defined(CONFIG_PCI_HCI)
					//found nat25 entry of br0
					mac_clone(priv, skb->data+ETH_ALEN);
					priv->macclone_completed = 1;
					indicate_wsc_mac_addr_has_changed(priv);
#elif defined(CONFIG_USB_HCI) || defined(CONFIG_SDIO_HCI)
					notify_mac_clone(priv, skb->data+ETH_ALEN);
#endif
				}
			}
#endif // MULTI_MAC_CLONE
		}
	}

#ifdef MULTI_MAC_CLONE
	if (ACTIVE_ID == 0 && priv->pmib->ethBrExtInfo.macclone_enable) 
	{
		int id;
		struct net_bridge_port *br_port;

		priv = GET_DEV_PRIV(skb->dev);

		if (!memcmp(&skb->data[ETH_ALEN], GET_MY_HWADDR, ETH_ALEN))
			return 0;			

		if ((skb->data[ETH_ALEN] & 1) || !memcmp(&skb->data[ETH_ALEN], NULL_MAC_ADDR, ETH_ALEN))
			return 0;
		
		#ifndef __ECOS
		br_port = GET_BR_PORT(priv->dev);
		#endif
		//from br but not br mac
		if (
            #ifdef __ECOS
             (((Rltk819x_t *)(skb->dev->info))->sc->sc_arpcom.ac_if.if_bridge) && 
            #else
             br_port && 
            #endif
            memcmp(skb->data+ETH_ALEN, priv->br_mac, ETH_ALEN))  
		{
			if (MCLONE_NUM >= priv->pshare->mclone_num_max)
				return 1;

			if ((*(unsigned short *)(skb->data+ETH_ALEN*2)) ==  __constant_htons(ETH_P_PPP_MP))
				return 1; // skip this packet (protocol=0x0008)

			id = mclone_find_unused_id(priv);
			
			if (id == -1)
			    return 1;
			
			if (((skb->data+ETH_ALEN)[0] & BIT1) && (priv->pmib->ethBrExtInfo.macclone_method == 1)){
				return 1;
			}
			MCLONE_NUM++;

            if (
                #ifdef __ECOS
                *(unsigned int *)&(skb->cb[8]) == 0x86518190 
                #else
                *(unsigned int *)&(skb->cb[40]) == 0x86518192
                #endif
                )
            { //from wifi AP
			    unsigned char usedID=0xff;
			    
				memcpy(priv->pshare->mclone_sta[id-1].sa_addr, skb->data+ETH_ALEN, ETH_ALEN);//wifi sta addr

				usedID = mclone_find_staFixedAddr(priv);//should always have free entry
				if (usedID == 0xff){
				    panic_printk("clone mac -- should always have value for wifi sta addr, err!!!!\n");
				} else {
				    panic_printk("clone mac -- wifi sta addr useID: %d \n", usedID);
				    priv->pshare->mclone_sta_fixed_addr[usedID].used = 1;
				}
				if (priv->pmib->ethBrExtInfo.macclone_method == 1){
					memcpy(priv->pshare->mclone_sta_fixed_addr[usedID].clone_addr,skb->data+ETH_ALEN, ETH_ALEN);//wifi sta addr
					priv->pshare->mclone_sta_fixed_addr[usedID].clone_addr[0] = priv->pshare->mclone_sta_fixed_addr[usedID].clone_addr[0] | BIT1;					
					memcpy(priv->pshare->mclone_sta[id-1].hwaddr, priv->pshare->mclone_sta_fixed_addr[usedID].clone_addr, ETH_ALEN);//wifi sta addr
				}else
				memcpy(priv->pshare->mclone_sta[id-1].hwaddr, priv->pshare->mclone_sta_fixed_addr[usedID].clone_addr, ETH_ALEN);//wifi sta addr
				priv->pshare->mclone_sta[id-1].usedStaAddrId = usedID;
				//memcpy(skb->data+ETH_ALEN, priv->pshare->mclone_sta[id-1].hwaddr, MACADDRLEN);//replace the src mac
			} else {
				memcpy(priv->pshare->mclone_sta[id-1].hwaddr, skb->data+ETH_ALEN, ETH_ALEN);
			}

			priv->pshare->mclone_sta[id-1].priv = priv;
			
			#ifdef  CONFIG_WLAN_HAL
				if (IS_HAL_CHIP(priv)){
            		GET_HAL_INTERFACE(priv)->McloneSetMBSSIDHandler(priv, priv->pshare->mclone_sta[id-1].hwaddr, (id-1));
				}else
			#endif
				mclone_set_mbssid(priv, priv->pshare->mclone_sta[id-1].hwaddr);
			
			ACTIVE_ID = id;
	
			panic_printk("clone mac - %02x:%02x:%02x:%02x:%02x:%02x\n", 
					*GET_MY_HWADDR, *(GET_MY_HWADDR+1), *(GET_MY_HWADDR+2),
				 	*(GET_MY_HWADDR+3), *(GET_MY_HWADDR+4), *(GET_MY_HWADDR+5));
			
			start_clnt_join(priv);
		}
	}
#endif
	
	return 0;
}


#define SERVER_PORT			67
#define CLIENT_PORT			68
#define DHCP_MAGIC			0x63825363
#define BROADCAST_FLAG		0x8000

#define OPT_CODE 	0
#define OPT_LEN 	1
#define OPT_DATA 	2
#define OPTION_FIELD	0
#define FILE_FIELD	1
#define SNAME_FIELD	2

/* DHCP option codes (partial list) */
#define DHCP_PADDING		0x00
#define DHCP_REQUESTED_IP	0x32
#define DHCP_OPTION_OVER	0x34
#define DHCP_CLIENT_ID		0x3d
#define DHCP_END			0xFF

struct dhcpMessage {
	u_int8_t op;
	u_int8_t htype;
	u_int8_t hlen;
	u_int8_t hops;
	u_int32_t xid;
	u_int16_t secs;
	u_int16_t flags;
	u_int32_t ciaddr;
	u_int32_t yiaddr;
	u_int32_t siaddr;
	u_int32_t giaddr;
	u_int8_t chaddr[16];
	u_int8_t sname[64];
	u_int8_t file[128];
	u_int32_t cookie;
	u_int8_t options[308]; /* 312 - cookie */
};


static int end_option(unsigned char *optionptr)
{
	int i = 0;

	while (optionptr[i] != DHCP_END) {
		if (optionptr[i] == DHCP_PADDING) i++;
		else i += optionptr[i + OPT_LEN] + 2;
	}
	return i;
}


unsigned char *get_dhcp_option(struct dhcpMessage *packet, int code)
{
	int i, length;
	unsigned char *optionptr=NULL;
	int over = 0, done = 0, curr = OPTION_FIELD;

	optionptr = packet->options;
	i = 0;
	length = 308;
	while (!done) {
		if (i >= length) {
			return NULL;
		}

		if (optionptr[i + OPT_CODE] == code) {
			if (i + 1 + optionptr[i + OPT_LEN] >= length) {
				return NULL;
			}
			return optionptr + i + 2;
		}

		switch (optionptr[i + OPT_CODE]) {
		case DHCP_PADDING:
			i++;
			break;
		case DHCP_OPTION_OVER:
			if (i + 1 + optionptr[i + OPT_LEN] >= length) {
				return NULL;
			}
			over = optionptr[i + 3];
			i += optionptr[OPT_LEN] + 2;
			break;
		case DHCP_END:
			if (curr == OPTION_FIELD && over & FILE_FIELD) {
				optionptr = packet->file;
				i = 0;
				length = 128;
				curr = FILE_FIELD;
			} else if (curr == FILE_FIELD && over & SNAME_FIELD) {
				optionptr = packet->sname;
				i = 0;
				length = 64;
				curr = SNAME_FIELD;
			} else done = 1;
			break;
		default:
			i += optionptr[OPT_LEN + i] + 2;
		}
	}
	return NULL;
}


void dhcp_add_reqip_option(struct rtl8192cd_priv *priv, struct sk_buff *skb)
{
	struct iphdr* iph = (struct iphdr *)(skb->data + ETH_HLEN);
	udphdr *udph = (udphdr *)((unsigned long)iph + (iph->ihl << 2));
	struct dhcpMessage *dhcph = (struct dhcpMessage *)((unsigned long)udph + sizeof(udphdr));
	unsigned char opt[6] = {0};
	unsigned char *opt_end;
	unsigned int end_offset;

	DEBUG_INFO("=======> %s - chaddr : %02X:%02X:%02X:%02X:%02X:%02X\n", __FUNCTION__, dhcph->chaddr[0], dhcph->chaddr[1],
			dhcph->chaddr[2], dhcph->chaddr[3], dhcph->chaddr[4], dhcph->chaddr[5]);

	opt[0] = (unsigned char)DHCP_REQUESTED_IP;
	opt[1] = (unsigned char)4;
#ifdef __ECOS
	rtk_put_unaligned_u32(rtk_get_unaligned_u32((char *)&dhcph->ciaddr), (char *)&opt[2]);
	rtk_put_unaligned_u32(0, (char *)&dhcph->ciaddr);
#else
	memcpy(&opt[2], &(dhcph->ciaddr), 4);
#if defined(__OSK__)
	WTOPDU(&dhcph->ciaddr, 0);
#else
	dhcph->ciaddr = 0;
#endif
#endif

	skb_put(skb, 6);

	end_offset = end_option(dhcph->options);
	if (end_offset + dhcph->options[OPT_LEN] + 2 + 1 >= 308) {
		DEBUG_ERR("*** %s add option error!!! ***\n", __FUNCTION__);
		return;
	}

	opt_end = (unsigned char *)&(dhcph->options) + end_offset;

	memcpy(opt_end, opt, 6);
	dhcph->options[end_offset+6] = DHCP_END;
	udph->len = htons(ntohs(udph->len) + 6);
	iph->tot_len = htons(ntohs(iph->tot_len) + 6);
}


void dhcp_flag_bcast(struct rtl8192cd_priv *priv, struct sk_buff *skb)
{
	unsigned int ip_check_recalc = 0;
	unsigned int udp_check_recalc = 0;
	struct iphdr* iph=NULL;
	udphdr *udph=NULL;
	struct dhcpMessage *dhcph=NULL;

	if(skb == NULL)
		return;

	if(!priv->pmib->ethBrExtInfo.dhcp_bcst_disable) {
		unsigned short protocol = *((unsigned short *)(skb->data + 2 * ETH_ALEN));

		if(protocol == __constant_htons(ETH_P_IP)) {
			iph = (struct iphdr *)(skb->data + ETH_HLEN);

			if(iph->protocol == IPPROTO_UDP) {
				udph = (udphdr *)((unsigned long)iph + (iph->ihl << 2));

				if((udph->source == __constant_htons(CLIENT_PORT)) && (udph->dest == __constant_htons(SERVER_PORT))) {// DHCP request
					dhcph = (struct dhcpMessage *)((unsigned long)udph + sizeof(udphdr));

#ifdef __ECOS
					if(rtk_get_unaligned_u32((char *)&dhcph->cookie) == __constant_htonl(DHCP_MAGIC))
#elif defined(__OSK__)
					if(DWFROMPDU(&dhcph->cookie) == __constant_htonl(DHCP_MAGIC))
#else
					if(dhcph->cookie == __constant_htonl(DHCP_MAGIC))
#endif
					{
						if(!(dhcph->flags & htons(BROADCAST_FLAG))) {
							DEBUG_INFO("DHCP: change flag of DHCP request to broadcast.\n");
							// or BROADCAST flag
							dhcph->flags |= htons(BROADCAST_FLAG);
							udp_check_recalc++;
						}

#ifdef __ECOS
						if (rtk_get_unaligned_u32((char *)&dhcph->ciaddr) && memcmp(dhcph->chaddr, GET_MY_HWADDR, MACADDRLEN))
#elif defined(__OSK__)
						if (XIPA(&dhcph->ciaddr) && memcmp(dhcph->chaddr, GET_MY_HWADDR, MACADDRLEN))
#else
						if (dhcph->ciaddr && memcmp(dhcph->chaddr, GET_MY_HWADDR, MACADDRLEN))
#endif
						{
							if (!get_dhcp_option(dhcph, DHCP_REQUESTED_IP)) {
								dhcp_add_reqip_option(priv, skb);
								udp_check_recalc++;
								ip_check_recalc++;
							} else {
#ifdef __ECOS
								rtk_put_unaligned_u32(0, (char *)&dhcph->ciaddr);
#elif defined(__OSK__)
								WIPA(&dhcph->ciaddr, 0);
#else
								dhcph->ciaddr = 0;
#endif
								udp_check_recalc++;
							}
						}
					}
				}

				if (udph->check && udp_check_recalc) {
#ifdef __ECOS
					ADJUST_CHKSUM(dhcph->flags, (dhcph->flags&(~htons(BROADCAST_FLAG))), udph->check);
#elif defined(__OSK__)				
					//jim 20130106 fix dhcp transform  error.
					uint8   phdr[12];	/* pesudo header */
					uint16  res;
					memcpy(phdr, &iph->saddr, 8);	/* saddr, daddr */
					phdr[8] = 0;			/* zero */
					phdr[9] = IPPROTO_UDP;	/* proto */
					//phdr[10] = htons(udph->len);	/*len*/
					htonas(&phdr[10], udph->len);
					res = ipcsum((uint16 *)phdr, 12, 0);
					udph->check = 0;
					res = ipcsum((uint16 *)udph, udph->len, res);
					//udph->check = htons(~res);
					htonas(&(udph->check) , ~res);
#else
					udph->check = 0;
					udph->check = csum_tcpudp_magic(iph->saddr, iph->daddr,
						cpu_to_be16(udph->len), IPPROTO_UDP,
						csum_partial((void *)udph, cpu_to_be16(udph->len), 0));
#endif
				}
			}

			if (ip_check_recalc) {
#ifdef __ECOS
				udph->check = 0;
				ADJUST_CHKSUM(iph->tot_len, (iph->tot_len-6), iph->check);
#elif defined(__OSK__)
				uint16 res = 0;
				iph->check = 0;
				res = ipcsum((uint16 *)iph, iph->ihl << 2, 0);
				iph->check = htons(~res);
#else
				ip_send_check(iph);
#endif
			}
		}
	}
}


void dhcp_dst_bcast(struct rtl8192cd_priv * priv,struct sk_buff * skb)
{
	unsigned int ip_check_recalc = 0;
	unsigned int udp_check_recalc = 0;
	struct iphdr* iph=NULL;
	udphdr *udph=NULL;
	struct dhcpMessage *dhcph=NULL;

	if(skb == NULL)
		return;

	if(!priv->pmib->ethBrExtInfo.dhcp_bcst_disable) {
		iph = (struct iphdr *)(skb->data + ETH_HLEN);

		if(iph->protocol == IPPROTO_UDP) {
			udph = (udphdr *)((unsigned long)iph + (iph->ihl << 2));

			if((udph->source == __constant_htons(SERVER_PORT)) && (udph->dest == __constant_htons(CLIENT_PORT))) {// DHCP request
				dhcph = (struct dhcpMessage *)((unsigned long)udph + sizeof(udphdr));

#ifdef __ECOS
				if(rtk_get_unaligned_u32((char *)&dhcph->cookie) == __constant_htonl(DHCP_MAGIC))
#elif defined(__OSK__)
				if(DWFROMPDU(&dhcph->cookie) == __constant_htonl(DHCP_MAGIC))
#else
				if(dhcph->cookie == __constant_htonl(DHCP_MAGIC))
#endif
				{

					unsigned char *daddr = (unsigned char *)&(iph->daddr);
					if((daddr[0] != 0xff))
					{
						//we set bootp flags to broadcast, but the DHCP server reply a unicast packets, so we reset the dst to broadcast
						daddr[0] = 0xff;
						daddr[1] = 0xff;
						daddr[2] = 0xff;
						daddr[3] = 0xff;
						if(skb->data[0] != 0xff)
						{
							skb->data[0] = 0xff;
							skb->data[1] = 0xff;
							skb->data[2] = 0xff;
							skb->data[3] = 0xff;
							skb->data[4] = 0xff;
							skb->data[5] = 0xff;
							memcpy(&skb->cb[10], skb->data, 6);
						}
						ip_check_recalc++;
						udp_check_recalc++;
					}
					
				}
			}

			if (udph->check && udp_check_recalc) {
#ifdef __ECOS
				ADJUST_CHKSUM(dhcph->flags, (dhcph->flags&(~htons(BROADCAST_FLAG))), udph->check);
#elif defined(__OSK__)
				uint8   phdr[12];	/* pesudo header */
				uint16  res;
				memcpy(phdr, &iph->saddr, 8);	/* saddr, daddr */
				phdr[8] = 0;			/* zero */
				phdr[9] = IPPROTO_UDP;	/* proto */
				phdr[10] = htons(udph->len);	/*len*/
				res = ipcsum((uint16 *)phdr, 12, 0);
				res = ipcsum((uint16 *)udph, udph->len, res);
				udph->check = htons(~res);
#else
				udph->check = 0;
				udph->check = csum_tcpudp_magic(iph->saddr, iph->daddr,
					cpu_to_be16(udph->len), IPPROTO_UDP,
					csum_partial((void *)udph, cpu_to_be16(udph->len), 0));
#endif
			}

			if (ip_check_recalc) {
#ifdef __ECOS
				udph->check = 0;
				ADJUST_CHKSUM(iph->tot_len, (iph->tot_len-6), iph->check);
#elif defined(__OSK__)
				uint16 res = 0;
				iph->check = 0;
				res = ipcsum((uint16 *)iph, iph->ihl << 2, 0);
				iph->check = htons(~res);
#else
				ip_send_check(iph);
#endif
			}
		}
	}
}


#ifdef MULTI_MAC_CLONE
int mclone_dhcp_caddr(struct rtl8192cd_priv *priv, struct sk_buff *skb)
{
    unsigned short protocol;
    unsigned char *dhcpopt=NULL;
    struct iphdr *iph;
    udphdr *udph;
    struct dhcpMessage *dhcph;
    int active_id;
    unsigned int udp_check_recalc = 0;
    unsigned int opt61changed =0,posOdd=0;
    unsigned char *caddr, *opt61;
    unsigned char oldchaddr[ETH_ALEN],oldopt61[ETH_ALEN+2];

	if(skb == NULL)
		return 0;

	iph = (struct iphdr *)(skb->data + ETH_HLEN);

	if(iph->protocol == IPPROTO_UDP)
	{
		udph = (udphdr *)((unsigned int)iph + (iph->ihl << 2));
		if((udph->source == __constant_htons(CLIENT_PORT))
			&& (udph->dest == __constant_htons(SERVER_PORT))) // BootP request
		{
			dhcph = (struct dhcpMessage *)((unsigned int)udph + sizeof(udphdr));
			if(dhcph->cookie == __constant_htonl(DHCP_MAGIC))
			{
				active_id = mclone_find_address(priv, dhcph->chaddr, skb, MAC_CLONE_SA_FIND);
				if ((active_id>0) && (priv->pshare->mclone_sta[active_id-1].usedStaAddrId!=0xff)) {
					caddr = priv->pshare->mclone_sta[active_id-1].hwaddr;
					udp_check_recalc++;
					DEBUG_INFO("BootP Request: (active_id %d) %02x:%02x:%02x:%02x:%02x:%02x => %02x:%02x:%02x:%02x:%02x:%02x\n",
						active_id, dhcph->chaddr[0], dhcph->chaddr[1], dhcph->chaddr[2], dhcph->chaddr[3], dhcph->chaddr[4], dhcph->chaddr[5],
						caddr[0], caddr[1], caddr[2], caddr[3], caddr[4], caddr[5]);
				}
				else if (!memcmp(dhcph->chaddr, priv->br_mac, ETH_ALEN)) {
					caddr = GET_MY_HWADDR;
					udp_check_recalc++;
					DEBUG_INFO("BootP Request: (br_mac)%02x:%02x:%02x:%02x:%02x:%02x => %02x:%02x:%02x:%02x:%02x:%02x\n",
						dhcph->chaddr[0], dhcph->chaddr[1], dhcph->chaddr[2], dhcph->chaddr[3], dhcph->chaddr[4], dhcph->chaddr[5],
						caddr[0], caddr[1], caddr[2], caddr[3], caddr[4], caddr[5]);
				}
			}
		}
		else if((udph->source == __constant_htons(SERVER_PORT))
			&& (udph->dest == __constant_htons(CLIENT_PORT))) // BootP reply
		{
			dhcph = (struct dhcpMessage *)((unsigned int)udph + sizeof(udphdr));
			if(dhcph->cookie == __constant_htonl(DHCP_MAGIC))
			{
				active_id = mclone_find_address(priv, dhcph->chaddr, skb, MAC_CLONE_SA_FIND);
				if ((active_id>0) && (priv->pshare->mclone_sta[active_id-1].usedStaAddrId!=0xff)) {
					caddr = priv->pshare->mclone_sta[active_id-1].sa_addr;
					udp_check_recalc++;
					DEBUG_INFO("BootP Response: %02x:%02x:%02x:%02x:%02x:%02x => (active_id %d)%02x:%02x:%02x:%02x:%02x:%02x\n",
						dhcph->chaddr[0], dhcph->chaddr[1], dhcph->chaddr[2], dhcph->chaddr[3], dhcph->chaddr[4], dhcph->chaddr[5],
						active_id, caddr[0], caddr[1], caddr[2], caddr[3], caddr[4], caddr[5]);
				}
				else if (!memcmp(dhcph->chaddr, GET_MY_HWADDR, ETH_ALEN) && memcmp(priv->br_mac, NULL_MAC_ADDR, ETH_ALEN)) {
					caddr = priv->br_mac;
					udp_check_recalc++;
					DEBUG_INFO("BootP Response: %02x:%02x:%02x:%02x:%02x:%02x => (br_mac)%02x:%02x:%02x:%02x:%02x:%02x\n",
						dhcph->chaddr[0], dhcph->chaddr[1], dhcph->chaddr[2], dhcph->chaddr[3], dhcph->chaddr[4], dhcph->chaddr[5],
						caddr[0], caddr[1], caddr[2], caddr[3], caddr[4], caddr[5]);
				}
			}
		}

		if (udp_check_recalc) {
                memcpy(oldchaddr,dhcph->chaddr,ETH_ALEN);
                memcpy(dhcph->chaddr, caddr, ETH_ALEN);
                if (((opt61=get_dhcp_option(dhcph, DHCP_CLIENT_ID))!=NULL)
                	&& (*(unsigned char *)(opt61-1)==7) && (opt61[0]==1)) {
                	DEBUG_INFO("DHCP opt61: %d %02x:%02x:%02x:%02x:%02x:%02x\n", opt61[0], opt61[1], opt61[2], opt61[3], opt61[4], opt61[5], opt61[6]);

                	if(0 != ((opt61+1-dhcph->options)%2))
                		 posOdd=1;
                	if(posOdd)
                		memcpy(oldopt61, opt61, ETH_ALEN+2);
                	else
                		memcpy(oldopt61, opt61+1, ETH_ALEN);
                	
                	memcpy(opt61+1, caddr, ETH_ALEN);
                	opt61changed =1;
			}
			if (udph->check) {
          #ifdef __ECOS
                u16 *p1 = (u16 *)dhcph->chaddr;
                u16 *p2 = (u16 *)oldchaddr;
                ADJUST_CHKSUM(*p1++, *p2++, udph->check);
                ADJUST_CHKSUM(*p1++, *p2++, udph->check);
                ADJUST_CHKSUM(*p1, *p2, udph->check);

                if(1==opt61changed)  {
                	if(posOdd)
                		p1 = (u16 *) opt61;
                	else
                		p1 = (u16 *) (opt61+1);
                	p2 = (u16 *)oldopt61;
                	ADJUST_CHKSUM(*p1++, *p2++, udph->check);
                	ADJUST_CHKSUM(*p1++, *p2++, udph->check);
                	if(posOdd) ADJUST_CHKSUM(*p1++, *p2++, udph->check);
                	ADJUST_CHKSUM(*p1, *p2, udph->check);
			    }
#elif defined(__OSK__)
				//jim 20130106 fix dhcp transform  error.
				uint8   phdr[12];	/* pesudo header */
				uint16  res;
				memcpy(phdr, &iph->saddr, 8);	/* saddr, daddr */
				phdr[8] = 0;			/* zero */
				phdr[9] = IPPROTO_UDP;	/* proto */
				//phdr[10] = htons(udph->len);	/*len*/
				htonas(&phdr[10], udph->len);
				res = ipcsum((uint16 *)phdr, 12, 0);
				udph->check = 0;
				res = ipcsum((uint16 *)udph, udph->len, res);
				//udph->check = htons(~res);
				htonas(&(udph->check) , ~res);
          #else
				udph->check = 0;
				udph->check = csum_tcpudp_magic(iph->saddr, iph->daddr, htons(udph->len), IPPROTO_UDP,
					csum_partial((char *)udph, htons(udph->len), 0));
          #endif
			}
		}
	}

	return 1;
}
#endif

void *scdb_findEntry(struct rtl8192cd_priv *priv, unsigned char *macAddr,
		unsigned char *ipAddr)
{
	unsigned char networkAddr[MAX_NETWORK_ADDR_LEN];
	struct nat25_network_db_entry *db;
	int hash;

#ifdef __ECOS
	__nat25_generate_ipv4_network_addr(networkAddr, (unsigned char *)ipAddr);
#else
	__nat25_generate_ipv4_network_addr(networkAddr, (unsigned int *)ipAddr);
#endif
	hash = __nat25_network_hash(networkAddr);
	db = priv->nethash[hash];
	while (db != NULL)
	{
		if(!memcmp(db->networkAddr, networkAddr, MAX_NETWORK_ADDR_LEN))
			return (void *)db;

		db = db->next_hash;
	}
	return NULL;
}


void nat25_filter_default(struct rtl8192cd_priv *priv) {
    int i;
    int num;
    priv->nat25_filter = 0;

    memset(priv->nat25_filter_ethlist, 0xFF, sizeof(priv->nat25_filter_ethlist));
    memset(priv->nat25_filter_ipprotolist, 0xFF, sizeof(priv->nat25_filter_ipprotolist));
    
    num = sizeof(nat25_filter_ethtype) / sizeof(unsigned short);
    for(i = 0; i < NAT25_FILTER_ETH_NUM && i < num; i++) {
        priv->nat25_filter_ethlist[i] = nat25_filter_ethtype[i];
    }

    num = sizeof(nat25_filter_ipproto);
    for(i = 0; i < NAT25_FILTER_IPPROTO_NUM && i < num; i++) {
        priv->nat25_filter_ipprotolist[i] = nat25_filter_ipproto[i];
    }

}


/*return value: 1: filter out, 0: continue to process*/
unsigned char nat25_filter(struct rtl8192cd_priv *priv, struct sk_buff *skb) {
    int i;
    int found = 0;
    unsigned char ipproto = 0xFF;
    unsigned short ethtype = *((unsigned short *)(skb->data+MACADDRLEN*2));
    unsigned char * sa = skb->data+MACADDRLEN;
    if(!memcmp(sa, GET_MY_HWADDR, MACADDRLEN) || !memcmp(sa, priv->br_mac, MACADDRLEN)) {
        return 0;
    }
        
    
    for(i = 0; i < NAT25_FILTER_ETH_NUM; i++) {
        if(priv->nat25_filter_ethlist[i] == 0xFFFF)
            break;
        if(priv->nat25_filter_ethlist[i] == ethtype) {
            found = 1;
            break;
        }
    }

    if(found) {
        if(ethtype == __constant_htons(ETH_P_IP)) {
            ipproto = *((unsigned char *)skb->data+WLAN_ETHHDR_LEN+9);
        }
        else if(ethtype == __constant_htons(ETH_P_IPV6)) {
            ipproto = *((unsigned char *)skb->data+WLAN_ETHHDR_LEN+6);
        }

        if(ipproto != 0xFF) {            
            found = 0;
            for(i = 0; i < NAT25_FILTER_IPPROTO_NUM; i++) {
                if(priv->nat25_filter_ipprotolist[i] == 0xFF)
                    break;
                if(priv->nat25_filter_ipprotolist[i] == ipproto) {
                    found = 1;
                    break;
                }
            }
        }
    }

    if(found && priv->nat25_filter == 2) { /* match deny list*/
        return 1;
    }
    else if(found == 0 && priv->nat25_filter == 1) { /*do not match accept list*/
        return 1;
    }
    return 0;

}

#endif // RTK_BR_EXT

