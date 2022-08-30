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

#ifndef LITE_MONITOR_H
#define LITE_MONITOR_H

#include <netlink/genl/genl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <qcatools_lib.h>
#include <ieee80211_external.h>
#include <dp_lite_mon_pub.h>

uint8_t debug_level;
static char *lite_mon_get_debug(uint8_t debug);

#define MACSTR_LEN 18
#define WIFI_IFACE_PREFIX_LEN 4
#define VAP_IFACE_PREFIX_LEN 3

#define LITE_MON_LEVEL_MSDU 1
#define LITE_MON_LEVEL_MPDU 2
#define LITE_MON_LEVEL_PPDU 3

#define LITE_MON_METADATA_INVALID    0xFFFC

#define LITE_MON_LEN_0  0
#define LITE_MON_LEN_1  0x40
#define LITE_MON_LEN_2  0x80
#define LITE_MON_LEN_3  0x100
#define LITE_MON_LEN_ALL  0xFFFF

#define LITE_MON_MGMT_FILTER_INVALID 0x8080
#define LITE_MON_CTRL_FILTER_INVALID 0x000B
#define LITE_MON_DATA_FILTER_INVALID 0x3FF6
#define LITE_MON_FILTER_ALL          0xFFFF

#define LITE_MON_TRACE_NONE   0
#define LITE_MON_TRACE_ERROR  1
#define LITE_MON_TRACE_INFO   2
#define LITE_MON_TRACE_DEBUG  3
#define LITE_MON_TRACE_MAX    4

static struct option long_options[] = {
	/* Direction of command Tx/RX */
	{"direction", 1, 0, 'd'},
	/* filtering level MSDU/MPDU/PPDU */
	{"level", 1, 0, 'e'},
	/* disable lite monitor */
	{"disable", 0, 0, 'D'},
	/* filter value for filter pass */
	{"filter_fp", 1, 0, 'p'},
	/* filter value for monitor direct */
	{"filter_md", 1, 0, 'm'},
	/* filter value for monitor other*/
	{"filter_mo", 1, 0, 'o'},
	/* Length of mgmt packet */
	{"mgmt_len", 1, 0, 'j'},
	/* Length of ctrl packet */
	{"ctrl_len", 1, 0, 'k'},
	/* Length of data packet */
	{"data_len", 1, 0, 'l'},
	/* Meta data is also needed*/
	{"metadata", 1, 0, 'M'},
	/* Interface to output the frames */
	{"output", 1, 0, 'O'},
	/* Show the existing filter */
	{"show_filter", 0, 0, 's'},
	/* Add mac address for peer filtering */
	{"peer_add", 1, 0, 'a'},
	/* Remove mac address from peer filtering */
	{"peer_remove", 1, 0, 'r'},
	/* List mac address for peer filtering */
	{"peer_list", 0, 0, 'L'},
	/* Specify type of peer (assoc/non-assoc)*/
	{"type", 1, 0, 't'},
	/* Vap for issuing the mac address command */
	{"vap", 1, 0, 'v'},
	/* Provide debug value */
	{"debug", 1, 0, 'x'},
	/* Display help menu*/
	{"help", 0, 0, 'h'},
	{0, 0, 0, 0}
};

#define lite_mon_printf(level, fmt, args...) do {                              \
	if ((level) <= debug_level && (level) != LITE_MON_TRACE_INFO) {        \
		printf("lite_mon_%s: %s: %d: " fmt "\n",                       \
		       lite_mon_get_debug(level), __func__, __LINE__, ## args);\
	} else if ((level) <= debug_level) {                                   \
		printf(fmt "\n", ## args);                                     \
	}                                                                      \
} while (0)

#endif /* LITE_MONITOR_H */
