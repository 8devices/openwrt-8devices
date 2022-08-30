/*
 * Copyright (c) 2022, Qualcomm Innovation Center, Inc. All rights reserved.
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

#ifndef _DP_LITE_MON_PUB_
#define _DP_LITE_MON_PUB_

#ifdef QCA_SUPPORT_LITE_MONITOR

#define LITE_MON_FILTER_TYPE_SHIFT  16
#define LITE_MON_FILTER_TYPE_MASK   0xf0000
#define LITE_MON_FILTER_VALUE_SHIFT 0
#define LITE_MON_FILTER_VALUE_MASK  0xFFFF

#define LITE_MON_GET_FILTER_TYPE(x) \
	(((x) & LITE_MON_FILTER_TYPE_MASK) >> LITE_MON_FILTER_TYPE_SHIFT)
#define LITE_MON_GET_FILTER_VALUE(x) \
	(((x) & LITE_MON_FILTER_VALUE_MASK) >> LITE_MON_FILTER_VALUE_SHIFT)

#define LITE_MON_MODE_FILTER_FP 0
#define LITE_MON_MODE_FILTER_MD 1
#define LITE_MON_MODE_FILTER_MO 2
#define LITE_MON_MODE_MAX  3

#define LITE_MON_TYPE_MGMT 0
#define LITE_MON_TYPE_CTRL 1
#define LITE_MON_TYPE_DATA 2
#define LITE_MON_TYPE_MAX  3
#define LITE_MON_TYPE_ALL  0xF

#define LITE_MON_SET_FILTER 0
#define LITE_MON_SET_PEER   1
#define LITE_MON_GET_FILTER 2
#define LITE_MON_GET_PEER   3
#define LITE_MON_FILTER_MAX 4

#define LITE_MON_SET_FILTER_MASK 1
#define LITE_MON_SET_PEER_MASK   2
#define LITE_MON_GET_FILTER_MASK 4
#define LITE_MON_GET_PEER_MASK   8
#define LITE_MON_SANITY_NOT_NEEDED   16

#define LITE_MON_PEER_ADD     0
#define LITE_MON_PEER_REMOVE  1
#define LITE_MON_PEER_LIST    2
#define LITE_MON_PEER_CMD_MAX 3

#define LITE_MON_DIRECTION_RX 1
#define LITE_MON_DIRECTION_TX 2

#define LITE_MON_PEER_ASSOCIATED 1
#define LITE_MON_PEER_NON_ASSOCIATED 2

#define LITE_MON_MAX_PEER_COUNT 16

/**
 * struct lite_mon_filter_config - Lite monitor filter configuration
 * @level: Monitor filter level
 * @disable: Disable monitor filtering
 * @show_filter: Show the existing filter
 * @mgmt_filter: Management filter values
 * @ctrl_filter: Control filter values
 * @data_filter: Data filter values
 * @len: Length of packet to be filtered in
 * @metadata: Metadata needs to be added
 * @interface_name: interface to output
 */
struct lite_mon_filter_config {
	uint8_t level;
	uint8_t disable;
	uint8_t show_filter;
	uint16_t mgmt_filter[LITE_MON_MODE_MAX];
	uint16_t ctrl_filter[LITE_MON_MODE_MAX];
	uint16_t data_filter[LITE_MON_MODE_MAX];
	uint16_t len[LITE_MON_TYPE_MAX];
	uint8_t metadata;
	char interface_name[IFNAMSIZ];
};

/**
 * struct lite_mon_peer_config - Lite monitor peer filter configuration
 * @action: action whether to add/remove
 * @type: type of peer
 * @count: Total count of mac addresses added
 * @mac_addr: Mac address of peer
 * @interface_name: Interface name to send command
 */
struct lite_mon_peer_config {
	uint8_t action;
	uint8_t type;
	uint8_t count;
	uint8_t mac_addr[LITE_MON_MAX_PEER_COUNT][IEEE80211_ADDR_LEN];
	char interface_name[IFNAMSIZ];
};


/**
 * struct lite_mon_config - Overall lite mon config
 * @cmdtype: Type of command
 * @direction: Command direction tx/rx
 * @debug: debug option value
 * @filter_config: Filter config structure
 * @peer_config: Peer config structure
 */
struct lite_mon_config {
	uint8_t cmdtype;
	uint8_t direction;
	uint8_t debug;
	union {
		struct lite_mon_filter_config filter_config;
		struct lite_mon_peer_config peer_config;
	} data;
};

#endif /* QCA_SUPPORT_LITE_MONITOR */
#endif /* _DP_LITE_MON_PUB_ */
