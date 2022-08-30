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

#include <qcatools_lib.h>
#include <wlan_stats_define.h>
#include <stats_lib.h>

#define FL "%s(): %d:"

#define STATS_ERR(fmt, args...) \
	fprintf(stderr, FL "stats: Error: "fmt, __func__, __LINE__, ## args)
#define STATS_WARN(fmt, args...) \
	fprintf(stdout, FL "stats: Warn: "fmt, __func__, __LINE__, ## args)
#define STATS_MSG(fmt, args...) \
	fprintf(stdout, FL "stats: "fmt, __func__, __LINE__, ## args)

#define STATS_PRINT(fmt, args...) \
	fprintf(stdout, fmt, ## args)

#define DESC_FIELD_PROVISIONING         31

#define STATS_64(fp, descr, x) \
	fprintf(fp, "\t%-*s = %ju\n", DESC_FIELD_PROVISIONING, (descr), (x))

#define STATS_32(fp, descr, x) \
	fprintf(fp, "\t%-*s = %u\n", DESC_FIELD_PROVISIONING, (descr), (x))

#define STATS_16(fp, descr, x) \
	fprintf(fp, "\t%-*s = %hu\n", DESC_FIELD_PROVISIONING, (descr), (x))

#define STATS_16_SIGNED(fp, descr, x) \
	fprintf(fp, "\t%-*s = %hi\n", DESC_FIELD_PROVISIONING, (descr), (x))

#define STATS_8(fp, descr, x) \
	fprintf(fp, "\t%-*s = %hhu\n", DESC_FIELD_PROVISIONING, (descr), (x))

#define STATS_FLT(fp, descr, x, precsn) \
	fprintf(fp, "\t%-*s = %.*f\n", DESC_FIELD_PROVISIONING, (descr), \
	(precsn), (x))

#define STATS_UNVLBL(fp, descr, msg) \
	fprintf(fp, "\t%-*s = %s\n", DESC_FIELD_PROVISIONING, (descr), (msg))

/* WLAN MAC Address length */
#define USER_MAC_ADDR_LEN     (ETH_ALEN * 3)

#define STATS_IF_NSS_LENGTH   (6 * STATS_IF_SS_COUNT)

#define MAX_STRING_LEN        500

#ifdef WLAN_FEATURE_11BE
static const struct stats_if_rate_debug rate_string[STATS_IF_DOT11_MAX]
						   [STATS_IF_MAX_MCS] = {
	{
		{"OFDM 48 Mbps", STATS_IF_MCS_VALID},
		{"OFDM 24 Mbps", STATS_IF_MCS_VALID},
		{"OFDM 12 Mbps", STATS_IF_MCS_VALID},
		{"OFDM 6 Mbps ", STATS_IF_MCS_VALID},
		{"OFDM 54 Mbps", STATS_IF_MCS_VALID},
		{"OFDM 36 Mbps", STATS_IF_MCS_VALID},
		{"OFDM 18 Mbps", STATS_IF_MCS_VALID},
		{"OFDM 9 Mbps ", STATS_IF_MCS_VALID},
		{"INVALID ", STATS_IF_MCS_INVALID},
		{"INVALID ", STATS_IF_MCS_INVALID},
		{"INVALID ", STATS_IF_MCS_INVALID},
		{"INVALID ", STATS_IF_MCS_INVALID},
		{"INVALID ", STATS_IF_MCS_INVALID},
		{"INVALID ", STATS_IF_MCS_INVALID},
		{"INVALID ", STATS_IF_MCS_INVALID},
		{"INVALID ", STATS_IF_MCS_INVALID},
		{"INVALID ", STATS_IF_MCS_INVALID},
	},
	{
		{"CCK 11 Mbps Long  ", STATS_IF_MCS_VALID},
		{"CCK 5.5 Mbps Long ", STATS_IF_MCS_VALID},
		{"CCK 2 Mbps Long   ", STATS_IF_MCS_VALID},
		{"CCK 1 Mbps Long   ", STATS_IF_MCS_VALID},
		{"CCK 11 Mbps Short ", STATS_IF_MCS_VALID},
		{"CCK 5.5 Mbps Short", STATS_IF_MCS_VALID},
		{"CCK 2 Mbps Short  ", STATS_IF_MCS_VALID},
		{"INVALID ", STATS_IF_MCS_INVALID},
		{"INVALID ", STATS_IF_MCS_INVALID},
		{"INVALID ", STATS_IF_MCS_INVALID},
		{"INVALID ", STATS_IF_MCS_INVALID},
		{"INVALID ", STATS_IF_MCS_INVALID},
		{"INVALID ", STATS_IF_MCS_INVALID},
		{"INVALID ", STATS_IF_MCS_INVALID},
		{"INVALID ", STATS_IF_MCS_INVALID},
		{"INVALID ", STATS_IF_MCS_INVALID},
		{"INVALID ", STATS_IF_MCS_INVALID},
	},
	{
		{"HT MCS 0 (BPSK 1/2)  ", STATS_IF_MCS_VALID},
		{"HT MCS 1 (QPSK 1/2)  ", STATS_IF_MCS_VALID},
		{"HT MCS 2 (QPSK 3/4)  ", STATS_IF_MCS_VALID},
		{"HT MCS 3 (16-QAM 1/2)", STATS_IF_MCS_VALID},
		{"HT MCS 4 (16-QAM 3/4)", STATS_IF_MCS_VALID},
		{"HT MCS 5 (64-QAM 2/3)", STATS_IF_MCS_VALID},
		{"HT MCS 6 (64-QAM 3/4)", STATS_IF_MCS_VALID},
		{"HT MCS 7 (64-QAM 5/6)", STATS_IF_MCS_VALID},
		{"INVALID ", STATS_IF_MCS_INVALID},
		{"INVALID ", STATS_IF_MCS_INVALID},
		{"INVALID ", STATS_IF_MCS_INVALID},
		{"INVALID ", STATS_IF_MCS_INVALID},
		{"INVALID ", STATS_IF_MCS_INVALID},
		{"INVALID ", STATS_IF_MCS_INVALID},
		{"INVALID ", STATS_IF_MCS_INVALID},
		{"INVALID ", STATS_IF_MCS_INVALID},
		{"INVALID ", STATS_IF_MCS_INVALID},
	},
	{
		{"VHT MCS 0 (BPSK 1/2)     ", STATS_IF_MCS_VALID},
		{"VHT MCS 1 (QPSK 1/2)     ", STATS_IF_MCS_VALID},
		{"VHT MCS 2 (QPSK 3/4)     ", STATS_IF_MCS_VALID},
		{"VHT MCS 3 (16-QAM 1/2)   ", STATS_IF_MCS_VALID},
		{"VHT MCS 4 (16-QAM 3/4)   ", STATS_IF_MCS_VALID},
		{"VHT MCS 5 (64-QAM 2/3)   ", STATS_IF_MCS_VALID},
		{"VHT MCS 6 (64-QAM 3/4)   ", STATS_IF_MCS_VALID},
		{"VHT MCS 7 (64-QAM 5/6)   ", STATS_IF_MCS_VALID},
		{"VHT MCS 8 (256-QAM 3/4)  ", STATS_IF_MCS_VALID},
		{"VHT MCS 9 (256-QAM 5/6)  ", STATS_IF_MCS_VALID},
		{"VHT MCS 10 (1024-QAM 3/4)", STATS_IF_MCS_VALID},
		{"VHT MCS 11 (1024-QAM 5/6)", STATS_IF_MCS_VALID},
		{"INVALID ", STATS_IF_MCS_INVALID},
		{"INVALID ", STATS_IF_MCS_INVALID},
		{"INVALID ", STATS_IF_MCS_INVALID},
		{"INVALID ", STATS_IF_MCS_INVALID},
	},
	{
		{"HE MCS 0 (BPSK 1/2)     ", STATS_IF_MCS_VALID},
		{"HE MCS 1 (QPSK 1/2)     ", STATS_IF_MCS_VALID},
		{"HE MCS 2 (QPSK 3/4)     ", STATS_IF_MCS_VALID},
		{"HE MCS 3 (16-QAM 1/2)   ", STATS_IF_MCS_VALID},
		{"HE MCS 4 (16-QAM 3/4)   ", STATS_IF_MCS_VALID},
		{"HE MCS 5 (64-QAM 2/3)   ", STATS_IF_MCS_VALID},
		{"HE MCS 6 (64-QAM 3/4)   ", STATS_IF_MCS_VALID},
		{"HE MCS 7 (64-QAM 5/6)   ", STATS_IF_MCS_VALID},
		{"HE MCS 8 (256-QAM 3/4)  ", STATS_IF_MCS_VALID},
		{"HE MCS 9 (256-QAM 5/6)  ", STATS_IF_MCS_VALID},
		{"HE MCS 10 (1024-QAM 3/4)", STATS_IF_MCS_VALID},
		{"HE MCS 11 (1024-QAM 5/6)", STATS_IF_MCS_VALID},
		{"HE MCS 12 (4096-QAM 3/4)", STATS_IF_MCS_VALID},
		{"HE MCS 13 (4096-QAM 5/6)", STATS_IF_MCS_VALID},
		{"INVALID ", STATS_IF_MCS_INVALID},
		{"INVALID ", STATS_IF_MCS_INVALID},
		{"INVALID ", STATS_IF_MCS_INVALID},
	},
	{
		{"EHT MCS 0 (BPSK 1/2)     ", STATS_IF_MCS_VALID},
		{"EHT MCS 1 (QPSK 1/2)     ", STATS_IF_MCS_VALID},
		{"EHT MCS 2 (QPSK 3/4)     ", STATS_IF_MCS_VALID},
		{"EHT MCS 3 (16-QAM 1/2)   ", STATS_IF_MCS_VALID},
		{"EHT MCS 4 (16-QAM 3/4)   ", STATS_IF_MCS_VALID},
		{"EHT MCS 5 (64-QAM 2/3)   ", STATS_IF_MCS_VALID},
		{"EHT MCS 6 (64-QAM 3/4)   ", STATS_IF_MCS_VALID},
		{"EHT MCS 7 (64-QAM 5/6)   ", STATS_IF_MCS_VALID},
		{"EHT MCS 8 (256-QAM 3/4)  ", STATS_IF_MCS_VALID},
		{"EHT MCS 9 (256-QAM 5/6)  ", STATS_IF_MCS_VALID},
		{"EHT MCS 10 (1024-QAM 3/4)", STATS_IF_MCS_VALID},
		{"EHT MCS 11 (1024-QAM 5/6)", STATS_IF_MCS_VALID},
		{"EHT MCS 12 (4096-QAM 3/4)", STATS_IF_MCS_VALID},
		{"EHT MCS 13 (4096-QAM 5/6)", STATS_IF_MCS_VALID},
		{"EHT MCS 14 (BPSK-DCM 1/2)", STATS_IF_MCS_VALID},
		{"EHT MCS 15 (BPSK-DCM 1/2)", STATS_IF_MCS_VALID},
		{"INVALID ", STATS_IF_MCS_INVALID},
	}
};
#else
static const struct stats_if_rate_debug rate_string[STATS_IF_DOT11_MAX]
						   [STATS_IF_MAX_MCS] = {
	{
		{"OFDM 48 Mbps", STATS_IF_MCS_VALID},
		{"OFDM 24 Mbps", STATS_IF_MCS_VALID},
		{"OFDM 12 Mbps", STATS_IF_MCS_VALID},
		{"OFDM 6 Mbps ", STATS_IF_MCS_VALID},
		{"OFDM 54 Mbps", STATS_IF_MCS_VALID},
		{"OFDM 36 Mbps", STATS_IF_MCS_VALID},
		{"OFDM 18 Mbps", STATS_IF_MCS_VALID},
		{"OFDM 9 Mbps ", STATS_IF_MCS_VALID},
		{"INVALID ", STATS_IF_MCS_INVALID},
		{"INVALID ", STATS_IF_MCS_INVALID},
		{"INVALID ", STATS_IF_MCS_INVALID},
		{"INVALID ", STATS_IF_MCS_INVALID},
		{"INVALID ", STATS_IF_MCS_INVALID},
	},
	{
		{"CCK 11 Mbps Long  ", STATS_IF_MCS_VALID},
		{"CCK 5.5 Mbps Long ", STATS_IF_MCS_VALID},
		{"CCK 2 Mbps Long   ", STATS_IF_MCS_VALID},
		{"CCK 1 Mbps Long   ", STATS_IF_MCS_VALID},
		{"CCK 11 Mbps Short ", STATS_IF_MCS_VALID},
		{"CCK 5.5 Mbps Short", STATS_IF_MCS_VALID},
		{"CCK 2 Mbps Short  ", STATS_IF_MCS_VALID},
		{"INVALID ", STATS_IF_MCS_INVALID},
		{"INVALID ", STATS_IF_MCS_INVALID},
		{"INVALID ", STATS_IF_MCS_INVALID},
		{"INVALID ", STATS_IF_MCS_INVALID},
		{"INVALID ", STATS_IF_MCS_INVALID},
		{"INVALID ", STATS_IF_MCS_INVALID},
	},
	{
		{"HT MCS 0 (BPSK 1/2)  ", STATS_IF_MCS_VALID},
		{"HT MCS 1 (QPSK 1/2)  ", STATS_IF_MCS_VALID},
		{"HT MCS 2 (QPSK 3/4)  ", STATS_IF_MCS_VALID},
		{"HT MCS 3 (16-QAM 1/2)", STATS_IF_MCS_VALID},
		{"HT MCS 4 (16-QAM 3/4)", STATS_IF_MCS_VALID},
		{"HT MCS 5 (64-QAM 2/3)", STATS_IF_MCS_VALID},
		{"HT MCS 6 (64-QAM 3/4)", STATS_IF_MCS_VALID},
		{"HT MCS 7 (64-QAM 5/6)", STATS_IF_MCS_VALID},
		{"INVALID ", STATS_IF_MCS_INVALID},
		{"INVALID ", STATS_IF_MCS_INVALID},
		{"INVALID ", STATS_IF_MCS_INVALID},
		{"INVALID ", STATS_IF_MCS_INVALID},
		{"INVALID ", STATS_IF_MCS_INVALID},
	},
	{
		{"VHT MCS 0 (BPSK 1/2)     ", STATS_IF_MCS_VALID},
		{"VHT MCS 1 (QPSK 1/2)     ", STATS_IF_MCS_VALID},
		{"VHT MCS 2 (QPSK 3/4)     ", STATS_IF_MCS_VALID},
		{"VHT MCS 3 (16-QAM 1/2)   ", STATS_IF_MCS_VALID},
		{"VHT MCS 4 (16-QAM 3/4)   ", STATS_IF_MCS_VALID},
		{"VHT MCS 5 (64-QAM 2/3)   ", STATS_IF_MCS_VALID},
		{"VHT MCS 6 (64-QAM 3/4)   ", STATS_IF_MCS_VALID},
		{"VHT MCS 7 (64-QAM 5/6)   ", STATS_IF_MCS_VALID},
		{"VHT MCS 8 (256-QAM 3/4)  ", STATS_IF_MCS_VALID},
		{"VHT MCS 9 (256-QAM 5/6)  ", STATS_IF_MCS_VALID},
		{"VHT MCS 10 (1024-QAM 3/4)", STATS_IF_MCS_VALID},
		{"VHT MCS 11 (1024-QAM 5/6)", STATS_IF_MCS_VALID},
		{"INVALID ", STATS_IF_MCS_INVALID},
	},
	{
		{"HE MCS 0 (BPSK 1/2)     ", STATS_IF_MCS_VALID},
		{"HE MCS 1 (QPSK 1/2)     ", STATS_IF_MCS_VALID},
		{"HE MCS 2 (QPSK 3/4)     ", STATS_IF_MCS_VALID},
		{"HE MCS 3 (16-QAM 1/2)   ", STATS_IF_MCS_VALID},
		{"HE MCS 4 (16-QAM 3/4)   ", STATS_IF_MCS_VALID},
		{"HE MCS 5 (64-QAM 2/3)   ", STATS_IF_MCS_VALID},
		{"HE MCS 6 (64-QAM 3/4)   ", STATS_IF_MCS_VALID},
		{"HE MCS 7 (64-QAM 5/6)   ", STATS_IF_MCS_VALID},
		{"HE MCS 8 (256-QAM 3/4)  ", STATS_IF_MCS_VALID},
		{"HE MCS 9 (256-QAM 5/6)  ", STATS_IF_MCS_VALID},
		{"HE MCS 10 (1024-QAM 3/4)", STATS_IF_MCS_VALID},
		{"HE MCS 11 (1024-QAM 5/6)", STATS_IF_MCS_VALID},
		{"HE MCS 12 (4096-QAM 3/4)", STATS_IF_MCS_VALID},
		{"HE MCS 13 (4096-QAM 5/6)", STATS_IF_MCS_VALID},
		{"INVALID ", STATS_IF_MCS_INVALID},
	}
};
#endif

#if WLAN_DEBUG_TELEMETRY
#ifdef WLAN_FEATURE_11BE
static const struct stats_if_ru_debug ru_string[STATS_IF_RU_INDEX_MAX] = {
	{ "RU_26" },
	{ "RU_52" },
	{ "RU_52_26" },
	{ "RU_106" },
	{ "RU_106_26" },
	{ "RU_242" },
	{ "RU_484" },
	{ "RU_484_242" },
	{ "RU_996" },
	{ "RU_996_484" },
	{ "RU_996_484_242" },
	{ "RU_2x996" },
	{ "RU_2x996_484" },
	{ "RU_3x996" },
	{ "RU_3x996_484" },
	{ "RU_4x996" },
};
#else
static const struct stats_if_ru_debug ru_string[STATS_IF_RU_INDEX_MAX] = {
	{ "RU_26" },
	{ "RU_52" },
	{ "RU_106" },
	{ "RU_242" },
	{ "RU_484" },
	{ "RU_996" }
};
#endif

static const char *proto_subtype_string[STATS_IF_PROTO_SUBTYPE_MAX] = {
	"INVALID",
	"EAPOL_M1",
	"EAPOL_M2",
	"EAPOL_M3",
	"EAPOL_M4",
	"DHCP_DISCOVER",
	"DHCP_REQUEST",
	"DHCP_OFFER",
	"DHCP_ACK",
	"DHCP_NACK",
	"DHCP_RELEASE",
	"DHCP_INFORM",
	"DHCP_DECLINE",
	"ARP_REQ",
	"ARP_RES",
	"ICMP_REQ",
	"ICMP_RES",
	"ICMPV6_REQ",
	"ICMPV6_RES",
	"ICMPV6_RS",
	"ICMPV6_RA",
	"ICMPV6_NS",
	"ICMPV6_NA",
	"IPV4_UDP",
	"IPV4_TCP",
	"IPV6_UDP",
	"IPV6_TCP",
	"MGMT_ASSOC",
	"MGMT_DISASSOC",
	"MGMT_AUTH",
	"MGMT_DEAUTH",
	"ROAM_SYNCH",
	"ROAM_COMPLETE",
	"ROAM_EVENTID",
	"DNS_QUERY",
	"DNS_RES",
};

const char *mu_reception_mode[STATS_IF_TXRX_TYPE_MU_MAX] = {
	"MU MIMO", "MU OFDMA"
};
#endif /* WLAN_DEBUG_TELEMETRY */

static const char *opt_string = "BADarvsdcf:i:m:t:Rh?";

static const struct option long_opts[] = {
	{ "basic", no_argument, NULL, 'B' },
	{ "advance", no_argument, NULL, 'A' },
	{ "debug", no_argument, NULL, 'D' },
	{ "ap", no_argument, NULL, 'a' },
	{ "radio", no_argument, NULL, 'r' },
	{ "vap", no_argument, NULL, 'v' },
	{ "sta", no_argument, NULL, 's' },
	{ "data", no_argument, NULL, 'd' },
	{ "ctrl", no_argument, NULL, 'c' },
	{ "feature", required_argument, NULL, 'f' },
	{ "ifname", required_argument, NULL, 'i' },
	{ "stamacaddr", required_argument, NULL, 'm' },
	{ "serviceid", no_argument, NULL, 't' },
	{ "recursive", no_argument, NULL, 'R' },
	{ "help", no_argument, NULL, 'h' },
	{ NULL, no_argument, NULL, 0 },
};

const char *stats_if_fw_to_hw_delay_bucket[STATS_IF_DELAY_BUCKET_MAX + 1] = {
	"0 to 10 ms", "11 to 20 ms",
	"21 to 30 ms", "31 to 40 ms",
	"41 to 50 ms", "51 to 60 ms",
	"61 to 70 ms", "71 to 80 ms",
	"81 to 90 ms", "91 to 100 ms",
	"101 to 250 ms", "251 to 500 ms", "500+ ms"
};

const char *stats_if_sw_enq_delay_bucket[STATS_IF_DELAY_BUCKET_MAX + 1] = {
	"0 to 1 ms", "1 to 2 ms",
	"2 to 3 ms", "3 to 4 ms",
	"4 to 5 ms", "5 to 6 ms",
	"6 to 7 ms", "7 to 8 ms",
	"8 to 9 ms", "9 to 10 ms",
	"10 to 11 ms", "11 to 12 ms", "12+ ms"
};

const char *stats_if_intfrm_delay_bucket[STATS_IF_DELAY_BUCKET_MAX + 1] = {
	"0 to 5 ms", "6 to 10 ms",
	"11 to 15 ms", "16 to 20 ms",
	"21 to 25 ms", "26 to 30 ms",
	"31 to 35 ms", "36 to 40 ms",
	"41 to 45 ms", "46 to 50 ms",
	"51 to 55 ms", "56 to 60 ms", "60+ ms"
};

static void display_help(void)
{
	STATS_PRINT("\nwifitelemetry : Displays Statistics of Access Point\n");
	STATS_PRINT("\nUsage:\n"
		    "wifitelemetry [Level] [Object] [StatsType] [FeatureFlag] [[-i interface_name] | [-m StationMACAddress]] [-R] [-h | ?]\n"
		    "\n"
		    "One of the 3 Levels can be selected. The Levels are as follows (catagorised based on purpose or\n"
		    "the level of details):\n"
		    "Basic, Advance and Debug. The default is Basic where it provides very minimal stats.\n"
		    "\n"
		    "One of the 4 Objects can be selected. The objects are as follows (from top to bottom of the hierarchy):\n"
		    "Entire Access Point, Radio, Vap and Station (STA). The default is Entire Access Point.\n"
		    "Stats for sub-levels below the selected Object can be displayed by choosing -R (recursive) option (not\n"
		    "applicable for the STA Object, where information for a single STA in displayed).\n"
		    "\n"
		    "StatsType specifies the stats category. Catagories are:\n"
		    "Control and Data. Default is Data category.\n"
		    "\n"
		    "Feature flag specifies various specific feature for which Stats is to be displayed. By default ALL will\n"
		    "be selected. List of feature flags are as follows:\n"
		    "ALL,RX,TX,AST,CFR,FWD,HTT,RAW,RDK,TSO,TWT,VOW,WDI,WMI,IGMP,LINK,MESH,RATE,DELAY,JITTER,ME,NAWDS,TXCAP,MONITOR,SAWFDELAY and SAWFTX.\n"
		    "\n"
		    "Levels:\n"
		    "\n"
		    "-B or --basic\n"
		    "    Only Basic level stats are displyed. This is the default level\n"
		    "-A or --advance\n"
		    "    Advance level stats are displayed.\n"
		    "-D or --debug\n"
		    "    Debug level stats are displayed.\n"
		    "\n"
		    "Objects:\n"
		    "\n"
		    "-a or --ap\n"
		    "    Entire Access Point stats are displayed. No need to specify interface and STA MAC.\n"
		    "-r or --radio\n"
		    "    Radio object stats are displayed. Radio interface Name needs to be provided.\n"
		    "-v or --vap\n"
		    "    Vap object stats are displayed. Vap interface Name need to be provided.\n"
		    "-s or --sta\n"
		    "    STA object stats are displayed. STA MAC Address need to be provided.\n"
		    "\n"
		    "Feature Flags:\n"
		    "\n"
		    "-f <flag1[,flag2,...,flagN]> or --feature=<flag1[,flag2,...,flagN]>\n"
		    "    Feature flag carries the flags for requested features. Multiple flags can be added with ',' separation.\n"
		    "\n"
		    "Types:\n"
		    "\n"
		    "-d or --data\n"
		    "    Stats for data is displayed\n"
		    "-c or --ctrl\n"
		    "    Stats for control is displayed\n"
		    "\n"
		    "Interface:\n"
		    "\n"
		    "If Radio object is selected:\n"
		    "-i wifiX or --ifname=wifiX\n"
		    "\n"
		    "If VAP object is selected:\n"
		    "-i <VAP_name> or --ifname=<VAP_name>\n"
		    "\n"
		    "STA MAC Address:\n"
		    "\n"
		    "Required if STA object is selected:\n"
		    "-m xx:xx:xx:xx:xx:xx or --stamacaddr xx:xx:xx:xx:xx:xx\n"
		    "\n"
		    "Can be set only in STA mode for SAWFDELAY and SWFATX features:\n"
		    "-t <serviceid>\n"
		    "\n"
		    "OTHER OPTIONS:\n"
		    "-R or --recursive\n"
		    "    Recursive display\n"
		    "-h or --help\n"
		    "    Usage display\n");
}

static char *macaddr_to_str(u_int8_t *addr)
{
	static char string[3 * ETHER_ADDR_LEN];

	memset(string, 0, sizeof(string));
	if (addr) {
		snprintf(string, sizeof(string),
			 "%02x:%02x:%02x:%02x:%02x:%02x",
			 addr[0], addr[1], addr[2],
			 addr[3], addr[4], addr[5]);
	}

	return string;
}

void print_basic_data_tx_stats(struct basic_data_tx_stats *tx)
{
	STATS_64(stdout, "Tx Success", tx->tx_success.num);
	STATS_64(stdout, "Tx Success Bytes", tx->tx_success.bytes);
	STATS_64(stdout, "Tx Complete", tx->comp_pkt.num);
	STATS_64(stdout, "Tx Complete Bytes", tx->comp_pkt.bytes);
	STATS_64(stdout, "Tx Failed", tx->tx_failed);
	STATS_64(stdout, "Tx Dropped Count", tx->dropped_count);
}

void print_basic_data_rx_stats(struct basic_data_rx_stats *rx)
{
	STATS_64(stdout, "Rx To Stack", rx->to_stack.num);
	STATS_64(stdout, "Rx To Stack Bytes", rx->to_stack.bytes);
	STATS_64(stdout, "Rx Total Packets", rx->total_rcvd.num);
	STATS_64(stdout, "Rx Total Bytes", rx->total_rcvd.bytes);
	STATS_64(stdout, "Rx Error Count", rx->rx_error_count);
}

void print_basic_sta_data_tx(struct basic_peer_data_tx *tx)
{
	print_basic_data_tx_stats(&tx->tx);
}

void print_basic_sta_data_rx(struct basic_peer_data_rx *rx)
{
	print_basic_data_rx_stats(&rx->rx);
}

void print_basic_sta_data_link(struct basic_peer_data_link *link)
{
	STATS_8(stdout, "SNR", link->snr);
	STATS_8(stdout, "Last SNR", link->last_snr);
	STATS_32(stdout, "Avrage SNR", link->avg_snr);
}

void print_basic_sta_data_rate(struct basic_peer_data_rate *rate)
{
	STATS_32(stdout, "Rx Rate", rate->rx_rate);
	STATS_32(stdout, "Last Rx Rate", rate->last_rx_rate);
	STATS_32(stdout, "Tx Rate", rate->tx_rate);
	STATS_32(stdout, "Last Tx Rate", rate->last_tx_rate);
}

void print_basic_sta_ctrl_tx(struct basic_peer_ctrl_tx *tx)
{
	STATS_32(stdout, "Tx Management", tx->cs_tx_mgmt);
	STATS_32(stdout, "Tx Not Ok", tx->cs_is_tx_not_ok);
}

void print_basic_sta_ctrl_rx(struct basic_peer_ctrl_rx *rx)
{
	STATS_32(stdout, "Rx Management", rx->cs_rx_mgmt);
	STATS_32(stdout, "Rx Decrypt Crc", rx->cs_rx_decryptcrc);
	STATS_32(stdout, "Rx Security Failure", rx->cs_rx_security_failure);
}

void print_basic_sta_ctrl_link(struct basic_peer_ctrl_link *link)
{
	STATS_8(stdout, "Rx Management SNR", link->cs_rx_mgmt_snr);
}

void print_basic_sta_ctrl_rate(struct basic_peer_ctrl_rate *rate)
{
	STATS_32(stdout, "Rx Management Rate", rate->cs_rx_mgmt_rate);
}

void print_basic_vap_data_tx(struct basic_vdev_data_tx *tx)
{
	struct pkt_info *pkt = NULL;

	print_basic_data_tx_stats(&tx->tx);
	pkt = &tx->ingress;
	STATS_64(stdout, "Tx Ingress Received", pkt->num);
	STATS_64(stdout, "Tx Ingress Received Bytes", pkt->bytes);
	pkt = &tx->processed;
	STATS_64(stdout, "Tx Ingress Processed", pkt->num);
	STATS_64(stdout, "Tx Ingress Processed Bytes", pkt->bytes);
	pkt = &tx->dropped;
	STATS_64(stdout, "Tx Ingress Dropped", pkt->num);
	STATS_64(stdout, "Tx Ingress Dropped Bytes", pkt->bytes);
}

void print_basic_vap_data_rx(struct basic_vdev_data_rx *rx)
{
	print_basic_data_rx_stats(&rx->rx);
}

void print_basic_vap_ctrl_tx(struct basic_vdev_ctrl_tx *tx)
{
	STATS_64(stdout, "Tx Management", tx->cs_tx_mgmt);
	STATS_64(stdout, "Tx Error Count", tx->cs_tx_error_counter);
	STATS_64(stdout, "Tx Discard", tx->cs_tx_discard);
}

void print_basic_vap_ctrl_rx(struct basic_vdev_ctrl_rx *rx)
{
	STATS_64(stdout, "Rx Management", rx->cs_rx_mgmt);
	STATS_64(stdout, "Rx Error Count", rx->cs_rx_error_counter);
	STATS_64(stdout, "Rx Management Discard", rx->cs_rx_mgmt_discard);
	STATS_64(stdout, "Rx Control", rx->cs_rx_ctl);
	STATS_64(stdout, "Rx Discard", rx->cs_rx_discard);
	STATS_64(stdout, "Rx Security Failure", rx->cs_rx_security_failure);
}

void print_basic_radio_data_tx(struct basic_pdev_data_tx *tx)
{
	struct pkt_info *pkt = NULL;

	print_basic_data_tx_stats(&tx->tx);
	pkt = &tx->ingress;
	STATS_64(stdout, "Tx Ingress Received", pkt->num);
	STATS_64(stdout, "Tx Ingress Received Bytes", pkt->bytes);
	pkt = &tx->processed;
	STATS_64(stdout, "Tx Ingress Processed", pkt->num);
	STATS_64(stdout, "Tx Ingress Processed Bytes", pkt->bytes);
	pkt = &tx->dropped;
	STATS_64(stdout, "Tx Ingress Dropped", pkt->num);
	STATS_64(stdout, "Tx Ingress Dropped Bytes", pkt->bytes);
}

void print_basic_radio_data_rx(struct basic_pdev_data_rx *rx)
{
	print_basic_data_rx_stats(&rx->rx);
	STATS_64(stdout, "Dropped Count", rx->dropped_count);
	STATS_64(stdout, "Error Count", rx->err_count);
}

void print_basic_radio_ctrl_tx(struct basic_pdev_ctrl_tx *tx)
{
	STATS_32(stdout, "Lithium_cycle_counts: Tx Frame Count",
		 tx->cs_tx_frame_count);
	STATS_32(stdout, "Tx Management", tx->cs_tx_mgmt);
}

void print_basic_radio_ctrl_rx(struct basic_pdev_ctrl_rx *rx)
{
	STATS_32(stdout, "Lithium_cycle_counts: Rx Frame Count",
		 rx->cs_rx_frame_count);
	STATS_32(stdout, "Rx Management", rx->cs_rx_mgmt);
	STATS_32(stdout, "Rx Number of Mnagement", rx->cs_rx_num_mgmt);
	STATS_32(stdout, "Rx Number of Control", rx->cs_rx_num_ctl);
	STATS_32(stdout, "Rx Error Sum", rx->cs_rx_error_sum);
}

void print_basic_radio_ctrl_link(struct basic_pdev_ctrl_link *link)
{
	STATS_32(stdout, "Channel Tx Power", link->cs_chan_tx_pwr);
	STATS_16_SIGNED(stdout, "Channel NF", link->cs_chan_nf);
	STATS_16_SIGNED(stdout, "Channel NF Sec80", link->cs_chan_nf_sec80);
	STATS_8(stdout, "DCS Total Util", link->dcs_total_util);
}

void print_basic_ap_data_tx(struct basic_psoc_data_tx *tx)
{
	STATS_64(stdout, "Tx Egress Pkts", tx->egress.num);
	STATS_64(stdout, "Tx Egress Bytes", tx->egress.bytes);
}

void print_basic_ap_data_rx(struct basic_psoc_data_rx *rx)
{
	STATS_64(stdout, "Rx Ingress Pkts", rx->ingress.num);
	STATS_64(stdout, "Rx Ingress Bytes", rx->ingress.bytes);
}

#if WLAN_ADVANCE_TELEMETRY
void print_advance_data_tx_stats(struct advance_data_tx_stats *tx)
{
	u_int8_t inx = 0;

	STATS_64(stdout, "Tx Unicast Packets", tx->ucast.num);
	STATS_64(stdout, "Tx Unicast Bytes", tx->ucast.bytes);
	STATS_64(stdout, "Tx Multicast Packets", tx->mcast.num);
	STATS_64(stdout, "Tx Multicast Bytes", tx->mcast.bytes);
	STATS_64(stdout, "Tx Broadcast Packets", tx->bcast.num);
	STATS_64(stdout, "Tx Broadcast Bytes", tx->bcast.bytes);
	STATS_PRINT("\tTx SGI = 0.8us %u, 0.4us %u, 1.6us %u, 3.2us %u\n",
		    tx->sgi_count[0], tx->sgi_count[1],
		    tx->sgi_count[2], tx->sgi_count[3]);
	STATS_PRINT("\tTx NSS (1-%d)\n\t", STATS_IF_SS_COUNT);
	for (inx = 0; inx < STATS_IF_SS_COUNT; inx++)
		STATS_PRINT(" %u = %u ", (inx + 1), tx->nss[inx]);
	STATS_PRINT("\n\tTx BW Counts = 20MHZ %u 40MHZ %u 80MHZ %u 160MHZ %u\n",
		    tx->bw[0], tx->bw[1], tx->bw[2], tx->bw[3]);
	STATS_32(stdout, "Tx Retries", tx->retries);
	STATS_PRINT("\tTx Aggregation:\n");
	STATS_32(stdout, "MSDU's Part of AMPDU", tx->ampdu_cnt);
	STATS_32(stdout, "MSDU's With No MPDU Level Aggregation",
		 tx->non_ampdu_cnt);
	STATS_32(stdout, "MSDU's Part of AMSDU", tx->amsdu_cnt);
	STATS_32(stdout, "MSDU's With No MSDU Level Aggregation",
		 tx->non_amsdu_cnt);
}

void print_advance_data_rx_stats(struct advance_data_rx_stats *rx)
{
	u_int8_t inx = 0;

	STATS_64(stdout, "Rx Unicast Packets", rx->unicast.num);
	STATS_64(stdout, "Rx Unicast Bytes", rx->unicast.bytes);
	STATS_64(stdout, "Rx Multicast Packets", rx->multicast.num);
	STATS_64(stdout, "Rx Multicast Bytes", rx->multicast.bytes);
	STATS_64(stdout, "Rx Broadcast Packets", rx->bcast.num);
	STATS_64(stdout, "Rx Broadcast Bytes", rx->bcast.bytes);
	STATS_32(stdout, "Rx Retries", rx->rx_retries);
	STATS_32(stdout, "Rx Multipass Packet Drop", rx->multipass_rx_pkt_drop);
	STATS_32(stdout, "Rx BAR Reaceive Count", rx->bar_recv_cnt);
	STATS_32(stdout, "Rx MPDU FCS Ok Count", rx->mpdu_cnt_fcs_ok);
	STATS_32(stdout, "Rx MPDU FCS Error Count", rx->mpdu_cnt_fcs_err);
	STATS_PRINT("\tRx PPDU Counts\n");
	for (inx = 0; inx < STATS_IF_MAX_MCS; inx++) {
		if (!rate_string[STATS_IF_DOT11_AX][inx].valid)
			continue;
		STATS_PRINT("\t\t%s = %u\n",
			    rate_string[STATS_IF_DOT11_AX][inx].mcs_type,
			    rx->su_ax_ppdu_cnt[inx]);
	}
	STATS_PRINT("\tRx SGI = 0.8us %u, 0.4us %u, 1.6us %u, 3.2us %u\n",
		    rx->sgi_count[0], rx->sgi_count[1],
		    rx->sgi_count[2], rx->sgi_count[3]);
	STATS_PRINT("\tRx MSDU Counts for NSS (1-%u)\n\t", STATS_IF_SS_COUNT);
	for (inx = 0; inx < STATS_IF_SS_COUNT; inx++)
		STATS_PRINT(" %u = %u ", (inx + 1), rx->nss[inx]);
	STATS_PRINT("\n\tRx PPDU Counts for NSS (1-%u) in SU mode\n\t",
		    STATS_IF_SS_COUNT);
	for (inx = 0; inx < STATS_IF_SS_COUNT; inx++)
		STATS_PRINT(" %u = %u ", (inx + 1), rx->ppdu_nss[inx]);
	STATS_PRINT("\n\tRx BW Counts = 20MHZ %u 40MHZ %u 80MHZ %u 160MHZ %u\n",
		    rx->bw[0], rx->bw[1], rx->bw[2], rx->bw[3]);
	STATS_PRINT("\tRx Data Packets per AC\n");
	STATS_32(stdout, "     Best effort",
		 rx->wme_ac_type[STATS_IF_WME_AC_BE]);
	STATS_32(stdout, "      Background",
		 rx->wme_ac_type[STATS_IF_WME_AC_BK]);
	STATS_32(stdout, "           Video",
		 rx->wme_ac_type[STATS_IF_WME_AC_VI]);
	STATS_32(stdout, "           Voice",
		 rx->wme_ac_type[STATS_IF_WME_AC_VO]);
	STATS_PRINT("\tRx Aggregation:\n");
	STATS_32(stdout, "MSDU's Part of AMPDU", rx->ampdu_cnt);
	STATS_32(stdout, "MSDU's With No MPDU Level Aggregation",
		 rx->non_ampdu_cnt);
	STATS_32(stdout, "MSDU's Part of AMSDU", rx->amsdu_cnt);
	STATS_32(stdout, "MSDU's With No MSDU Level Aggregation",
		 rx->non_amsdu_cnt);
}

void print_advance_sta_data_tx(struct advance_peer_data_tx *tx)
{
	print_basic_sta_data_tx(&tx->b_tx);
	print_advance_data_tx_stats(&tx->adv_tx);
}

void print_advance_sta_data_rx(struct advance_peer_data_rx *rx)
{
	print_basic_sta_data_rx(&rx->b_rx);
	print_advance_data_rx_stats(&rx->adv_rx);
}

void print_advance_sta_data_fwd(struct advance_peer_data_fwd *fwd)
{
	STATS_64(stdout, "Intra BSS Packets", fwd->pkts.num);
	STATS_64(stdout, "Intra BSS Bytes", fwd->pkts.bytes);
	STATS_64(stdout, "Intra BSS Fail Packets", fwd->fail.num);
	STATS_64(stdout, "Intra BSS Fail Bytes", fwd->fail.bytes);
	STATS_32(stdout, "Intra BSS MDNS No FWD", fwd->mdns_no_fwd);
}

void print_advance_sta_data_raw(struct advance_peer_data_raw *raw)
{
	STATS_64(stdout, "Raw Packets", raw->raw.num);
	STATS_64(stdout, "Raw Bytes", raw->raw.bytes);
}

void print_advance_sta_data_twt(struct advance_peer_data_twt *twt)
{
	STATS_64(stdout, "Rx TWT Packets", twt->to_stack_twt.num);
	STATS_64(stdout, "Rx TWT Bytes", twt->to_stack_twt.bytes);
	STATS_64(stdout, "Tx TWT Packets", twt->tx_success_twt.num);
	STATS_64(stdout, "Tx TWT Bytes", twt->tx_success_twt.bytes);
}

void print_advance_sta_data_link(struct advance_peer_data_link *link)
{
	print_basic_sta_data_link(&link->b_link);
	STATS_32(stdout, "Rx SNR Time", link->rx_snr_measured_time);
}

void print_advance_sta_data_rate(struct advance_peer_data_rate *rate)
{
	print_basic_sta_data_rate(&rate->b_rate);
	STATS_32(stdout, "Avg ppdu Rx rate(kbps)", rate->rnd_avg_rx_rate);
	STATS_32(stdout, "Avg ppdu Rx rate", rate->avg_rx_rate);
	STATS_32(stdout, "Avg ppdu Tx rate(kbps)", rate->rnd_avg_tx_rate);
	STATS_32(stdout, "Avg ppdu Tx rate", rate->avg_tx_rate);
}

void print_advance_sta_data_nawds(struct advance_peer_data_nawds *nawds)
{
	STATS_64(stdout, "Multicast Packets", nawds->nawds_mcast.num);
	STATS_64(stdout, "Multicast Bytes", nawds->nawds_mcast.bytes);
	STATS_32(stdout, "NAWDS Tx Drop Count", nawds->nawds_mcast_tx_drop);
	STATS_32(stdout, "NAWDS Rx Drop Count", nawds->nawds_mcast_rx_drop);
}

static void print_advance_hist_stats(struct stats_if_hist_stats *hstats,
				     enum stats_if_hist_types hist_type)
{
	uint8_t index = 0;
	uint64_t count = 0;

	for (index = 0; index < STATS_IF_HIST_BUCKET_MAX; index++) {
		count = hstats->hist.freq[index];
		if (!count)
			continue;
		if (index > STATS_IF_DELAY_BUCKET_MAX) {
			STATS_PRINT("%s: Packets = %ju ",
				    "Invalid index", count);
			continue;
		}
		switch (hist_type) {
		case STATS_IF_HIST_TYPE_SW_ENQEUE_DELAY:
			STATS_PRINT("%s: Packets = %ju ",
				    stats_if_sw_enq_delay_bucket[index],
				    count);
			break;
		case STATS_IF_HIST_TYPE_HW_COMP_DELAY:
			STATS_PRINT("%s: Packets = %ju ",
				    stats_if_fw_to_hw_delay_bucket[index],
				    count);
			break;
		case STATS_IF_HIST_TYPE_REAP_STACK:
			STATS_PRINT("%s: Packets = %ju ",
				    stats_if_intfrm_delay_bucket[index],
				    count);
			break;
		default:
			break;
		}
	}

	STATS_PRINT("Min = %d ", hstats->min);
	STATS_PRINT("Max = %d ", hstats->max);
	STATS_PRINT("Avg = %d\n", hstats->avg);
}

static void print_advance_sta_data_delay(struct advance_peer_data_delay *delay)
{
	uint8_t tid;

	STATS_PRINT("Tx Delay Stats:\n");
	for (tid = 0; tid < STATS_IF_MAX_DATA_TIDS; tid++) {
		STATS_PRINT("----TID: %d----", tid);
		STATS_PRINT(" Software Enqueue Delay: ");
		print_advance_hist_stats(&delay->delay_stats[tid].tx_delay.tx_swq_delay,
					 STATS_IF_HIST_TYPE_SW_ENQEUE_DELAY);
		STATS_PRINT("\t\tHardware Transmission Delay: ");
		print_advance_hist_stats(&delay->delay_stats[tid].tx_delay.hwtx_delay,
					 STATS_IF_HIST_TYPE_HW_COMP_DELAY);
	}
	STATS_PRINT("\nRx Delay Stats:\n");
	for (tid = 0; tid < STATS_IF_MAX_DATA_TIDS; tid++) {
		STATS_PRINT("----TID: %d---- ", tid);
		STATS_PRINT("Reap2stack Deliver Delay: ");
		print_advance_hist_stats(&delay->delay_stats[tid].rx_delay
					 .to_stack_delay,
					 STATS_IF_HIST_TYPE_REAP_STACK);
	}
}

static void
print_advance_sta_data_jitter(struct advance_peer_data_jitter *jitter)
{
	uint8_t tid;

	for (tid = 0; tid < STATS_IF_MAX_DATA_TIDS; tid++) {
		STATS_PRINT("----TID: %d---- ", tid);
		STATS_PRINT("avg_jitter = %u ",
			    jitter->jitter_stats[tid].tx_avg_jitter);
		STATS_PRINT("avg_delay  = %u ",
			    jitter->jitter_stats[tid].tx_avg_delay);
		STATS_PRINT("avg_err  = %ju ",
			    jitter->jitter_stats[tid].tx_avg_err);
		STATS_PRINT("total_success = %ju ",
			    jitter->jitter_stats[tid].tx_total_success);
		STATS_PRINT("drop  = %ju\n", jitter->jitter_stats[tid].tx_drop);
	}
}

static void
print_advance_sta_data_sawf_delay(struct advance_peer_data_sawfdelay *data,
				  uint8_t svc_id)
{
	uint8_t idx = 0;

	if (svc_id > 0) {
		STATS_PRINT("sliding_window_sum = %u   ",
			    data->delay[0][0].avg.sum);
		STATS_PRINT("sliding_window_count = %u   ",
			    data->delay[0][0].avg.count);
		STATS_PRINT("current_window_index = %u   ",
			    data->delay[0][0].cur_win);
		STATS_PRINT("data_for_each_window");
		for (idx = 0; idx < STATS_IF_NUM_AVG_WINDOWS; idx++) {
			STATS_PRINT("----WINDOW: %d----   ", idx);
			STATS_PRINT("sum = %u   ",
				    data->delay[0][0].win_avgs[idx].sum);
			STATS_PRINT("count = %u   ",
				    data->delay[0][0].win_avgs[idx].count);
		}
		print_advance_hist_stats(&data->delay[0][0].delay_hist,
					 STATS_IF_HIST_TYPE_HW_COMP_DELAY);
	} else {
		uint8_t tidx = 0, queues = 0;
		uint8_t max_queue = STATS_IF_MAX_SAWF_DATA_QUEUE;
		struct stats_if_sawf_delay_stats *dly = NULL;
		uint32_t hw_comp = STATS_IF_HIST_TYPE_HW_COMP_DELAY;

		for (tidx = 0; tidx < STATS_IF_MAX_SAWF_DATA_TIDS; tidx++) {
			for (queues = 0; queues < max_queue; queues++) {
				dly = &data->delay[tidx][queues];
				STATS_PRINT("----TID: %d----   ", tidx);
				STATS_PRINT("----QUEUES: %d----\n", queues);

				STATS_PRINT("sliding_window_sum = %u\n",
					    dly->avg.sum);
				STATS_PRINT("sliding_window_count = %u\n",
					    dly->avg.count);
				STATS_PRINT("current_window_index = %u\n",
					    dly->cur_win);
				STATS_PRINT("data_for_each_window\n");
				for (idx = 0; idx < STATS_IF_NUM_AVG_WINDOWS;
				     idx++) {
					STATS_PRINT("----WINDOW: %d---- ", idx);
					STATS_PRINT("sum = %u    ",
						    dly->win_avgs[idx].sum);
					STATS_PRINT("count = %u\n",
						    dly->win_avgs[idx].count);
				}
				print_advance_hist_stats(&dly->delay_hist,
							 hw_comp);
			}
		}
	}
}

static void
print_advance_sta_data_sawf_tx(struct advance_peer_data_sawftx *data,
			       uint8_t svc_id)
{
	if (svc_id > 0) {
		STATS_PRINT("Tx_info_success_num = %u   ",
			    data->tx[0][0].tx_success.num);
		STATS_PRINT("Tx_info_success_bytes = %ju   ",
			    data->tx[0][0].tx_success.bytes);
		STATS_PRINT("Tx_info_dropped_num = %u   ",
			    data->tx[0][0].dropped.fw_rem.num);


		STATS_PRINT("Tx_info_dropped_bytes = %ju   ",
			    data->tx[0][0].dropped.fw_rem.bytes);
		STATS_PRINT("Tx_info_dropped_Tx_fw_rem_notx = %u   ",
			    data->tx[0][0].dropped.fw_rem_notx);
		STATS_PRINT("Tx_info_dropped_Tx_fw_rem_tx = %u   ",
			    data->tx[0][0].dropped.fw_rem_tx);
		STATS_PRINT("Tx_info_dropped_Tx_age_out = %u   ",
			    data->tx[0][0].dropped.age_out);


		STATS_PRINT("Tx_info_dropped_Tx_fw_reason1 = %u   ",
			    data->tx[0][0].dropped.fw_reason1);
		STATS_PRINT("Tx_info_dropped_Tx_fw_reason2 = %u   ",
			    data->tx[0][0].dropped.fw_reason2);
		STATS_PRINT("Tx_info_dropped_Tx_fw_reason3 = %u   ",
			    data->tx[0][0].dropped.fw_reason3);
		STATS_PRINT("Tx_info_tx_failed = %u   ",
			    data->tx[0][0].tx_failed);
		STATS_PRINT("Tx_info_queue_depth = %u",
			    data->tx[0][0].queue_depth);
	} else {
		uint8_t tidx = 0, queues = 0;
		uint8_t max_queue = STATS_IF_MAX_SAWF_DATA_QUEUE;
		struct stats_if_sawf_tx_stats *sawftx;

		for (tidx = 0; tidx < STATS_IF_MAX_SAWF_DATA_TIDS; tidx++) {
			for (queues = 0; queues < max_queue; queues++) {
				sawftx = &data->tx[tidx][queues];
				STATS_PRINT("----TIDX: %d----   ", tidx);
				STATS_PRINT("----QUEUE: %d---- \n", queues);
				STATS_PRINT("Tx_info_success_num = %u\n",
					    sawftx->tx_success.num);
				STATS_PRINT("Tx_info_success_bytes = %ju\n",
					    sawftx->tx_success.bytes);


				STATS_PRINT("Tx_info_drop_num = %u\n",
					    sawftx->dropped.fw_rem.num);
				STATS_PRINT("Tx_info_drop_bytes = %ju\n",
					    sawftx->dropped.fw_rem.bytes);
				STATS_PRINT("Tx_info_drop_fw_rem_notx = %u\n",
					    sawftx->dropped.fw_rem_notx);
				STATS_PRINT("Tx_info_drop_Tx_fw_rem_tx= %u\n",
					    sawftx->dropped.fw_rem_tx);
				STATS_PRINT("Tx_info_drop_Tx_age_out = %u\n",
					    sawftx->dropped.age_out);


				STATS_PRINT("Tx_info_drop_fw_reason1 = %u \n",
					    sawftx->dropped.fw_reason1);
				STATS_PRINT("Tx_inf_drop_Tx_fw_reason2 = %u \n",
					    sawftx->dropped.fw_reason2);
				STATS_PRINT("Tx_info_drop_Tx_fw_reason3 = %u\n",
					    sawftx->dropped.fw_reason3);
				STATS_PRINT("Tx_info_tx_failed = %u\n",
					    sawftx->tx_failed);
				STATS_PRINT("Tx_info_queue_depth = %u \n",
					    sawftx->queue_depth);
			}
		}
	}
}
void print_advance_sta_ctrl_tx(struct advance_peer_ctrl_tx *tx)
{
	print_basic_sta_ctrl_tx(&tx->b_tx);
	STATS_32(stdout, "Assocition Tx Count", tx->cs_tx_assoc);
	STATS_32(stdout, "Assocition Tx Failed Count", tx->cs_tx_assoc_fail);
}

void print_advance_sta_ctrl_rx(struct advance_peer_ctrl_rx *rx)
{
	print_basic_sta_ctrl_rx(&rx->b_rx);
}

void print_advance_sta_ctrl_twt(struct advance_peer_ctrl_twt *twt)
{
	STATS_32(stdout, "TWT Event Type", twt->cs_twt_event_type);
	STATS_32(stdout, "TWT Flow ID", twt->cs_twt_flow_id);
	STATS_32(stdout, "Broadcast TWT", twt->cs_twt_bcast);
	STATS_32(stdout, "TWT Trigger", twt->cs_twt_trig);
	STATS_32(stdout, "TWT Announcement", twt->cs_twt_announ);
	STATS_32(stdout, "TWT Dialog ID", twt->cs_twt_dialog_id);
	STATS_32(stdout, "TWT Wake duration (us)", twt->cs_twt_wake_dura_us);
	STATS_32(stdout, "TWT Wake interval (us)", twt->cs_twt_wake_intvl_us);
	STATS_32(stdout, "TWT SP offset (us)", twt->cs_twt_sp_offset_us);
}

void print_advance_sta_ctrl_link(struct advance_peer_ctrl_link *link)
{
	print_basic_sta_ctrl_link(&link->b_link);
}

void print_advance_sta_ctrl_rate(struct advance_peer_ctrl_rate *rate)
{
	print_basic_sta_ctrl_rate(&rate->b_rate);
}

void print_advance_vap_data_me(struct advance_vdev_data_me *me)
{
	STATS_64(stdout, "Multicast Packets", me->mcast_pkt.num);
	STATS_64(stdout, "Multicast Bytes", me->mcast_pkt.bytes);
	STATS_32(stdout, "Unicast Packets", me->ucast);
}

void print_advance_vap_data_tx(struct advance_vdev_data_tx *tx)
{
	print_basic_vap_data_tx(&tx->b_tx);
	print_advance_data_tx_stats(&tx->adv_tx);
	STATS_64(stdout, "Tx Reinject Packets", tx->reinject_pkts.num);
	STATS_64(stdout, "Tx Reinject Bytes", tx->reinject_pkts.bytes);
	STATS_64(stdout, "Tx Inspect Packets", tx->inspect_pkts.num);
	STATS_64(stdout, "Tx Inspect Bytes", tx->inspect_pkts.bytes);
	STATS_32(stdout, "Tx CCE Classified", tx->cce_classified);
}

void print_advance_vap_data_rx(struct advance_vdev_data_rx *rx)
{
	print_basic_vap_data_rx(&rx->b_rx);
	print_advance_data_rx_stats(&rx->adv_rx);
}

void print_advance_vap_data_raw(struct advance_vdev_data_raw *raw)
{
	STATS_64(stdout, "RAW Rx Packets", raw->rx_raw.num);
	STATS_64(stdout, "RAW Rx Bytes", raw->rx_raw.bytes);
	STATS_64(stdout, "RAW Tx Packets", raw->tx_raw_pkt.num);
	STATS_64(stdout, "RAW Tx Bytes", raw->tx_raw_pkt.bytes);
	STATS_32(stdout, "RAW Tx Classified by CCE", raw->cce_classified_raw);
}

void print_advance_vap_data_tso(struct advance_vdev_data_tso *tso)
{
	STATS_64(stdout, "SG Packets", tso->sg_pkt.num);
	STATS_64(stdout, "SG Bytess", tso->sg_pkt.bytes);
	STATS_64(stdout, "Non-SG Packets", tso->non_sg_pkts.num);
	STATS_64(stdout, "Non-SG Bytess", tso->non_sg_pkts.bytes);
	STATS_64(stdout, "TSO Packets", tso->num_tso_pkts.num);
	STATS_64(stdout, "TSO Bytess", tso->num_tso_pkts.bytes);
}

void print_advance_vap_data_igmp(struct advance_vdev_data_igmp *igmp)
{
	STATS_32(stdout, "IGMP Received", igmp->igmp_rcvd);
	STATS_32(stdout, "IGMP Converted Unicast", igmp->igmp_ucast_converted);
}

void print_advance_vap_data_mesh(struct advance_vdev_data_mesh *mesh)
{
	STATS_32(stdout, "MESH FW Completion Count", mesh->completion_fw);
	STATS_32(stdout, "MESH FW Exception Count", mesh->exception_fw);
}

void print_advance_vap_data_nawds(struct advance_vdev_data_nawds *nawds)
{
	STATS_64(stdout, "Multicast Tx Packets", nawds->tx_nawds_mcast.num);
	STATS_64(stdout, "Multicast Tx Bytes", nawds->tx_nawds_mcast.bytes);
	STATS_32(stdout, "NAWDS Tx Drop Count", nawds->nawds_mcast_tx_drop);
	STATS_32(stdout, "NAWDS Rx Drop Count", nawds->nawds_mcast_rx_drop);
}

void print_advance_vap_ctrl_tx(struct advance_vdev_ctrl_tx *tx)
{
	print_basic_vap_ctrl_tx(&tx->b_tx);
	STATS_64(stdout, "Tx Off Channel Management Count",
		 tx->cs_tx_offchan_mgmt);
	STATS_64(stdout, "Tx Off Channel Data Count", tx->cs_tx_offchan_data);
	STATS_64(stdout, "Tx Off Channel Fail Count", tx->cs_tx_offchan_fail);
	STATS_64(stdout, "Tx Beacon Count", tx->cs_tx_bcn_success);
	STATS_64(stdout, "Tx Beacon Outage Count", tx->cs_tx_bcn_outage);
	STATS_64(stdout, "Tx FILS Frame Sent Count", tx->cs_fils_frames_sent);
	STATS_64(stdout, "Tx FILS Frame Sent Fail",
		 tx->cs_fils_frames_sent_fail);
	STATS_64(stdout, "Tx Offload Probe Response Success Count",
		 tx->cs_tx_offload_prb_resp_succ_cnt);
	STATS_64(stdout, "Tx Offload Probe Response Fail Count",
		 tx->cs_tx_offload_prb_resp_fail_cnt);
}

void print_advance_vap_ctrl_rx(struct advance_vdev_ctrl_rx *rx)
{
	print_basic_vap_ctrl_rx(&rx->b_rx);
	STATS_64(stdout, "Rx Action Frame Count", rx->cs_rx_action);
	STATS_64(stdout, "Rx MLME Auth Attempt", rx->cs_mlme_auth_attempt);
	STATS_64(stdout, "Rx MLME Auth Success", rx->cs_mlme_auth_success);
	STATS_64(stdout, "Authorization Attempt", rx->cs_authorize_attempt);
	STATS_64(stdout, "Authorization Success", rx->cs_authorize_success);
	STATS_64(stdout, "Probe Request Drops", rx->cs_prob_req_drops);
	STATS_64(stdout, "OOB Probe Requests", rx->cs_oob_probe_req_count);
	STATS_64(stdout, "Wildcard probe requests drops",
		 rx->cs_wc_probe_req_drops);
	STATS_64(stdout, "Connections refuse Radio limit",
		 rx->cs_sta_xceed_rlim);
	STATS_64(stdout, "Connections refuse Vap limit", rx->cs_sta_xceed_vlim);
}

void print_advance_radio_data_me(struct advance_pdev_data_me *me)
{
	STATS_64(stdout, "Multicast Packets", me->mcast_pkt.num);
	STATS_64(stdout, "Multicast Bytes", me->mcast_pkt.bytes);
	STATS_32(stdout, "Unicast Packets", me->ucast);
}

void print_histogram_stats(struct histogram_stats *hist)
{
	STATS_32(stdout, " Single Packets", hist->pkts_1);
	STATS_32(stdout, "   2-20 Packets", hist->pkts_2_20);
	STATS_32(stdout, "  21-40 Packets", hist->pkts_21_40);
	STATS_32(stdout, "  41-60 Packets", hist->pkts_41_60);
	STATS_32(stdout, "  61-80 Packets", hist->pkts_61_80);
	STATS_32(stdout, " 81-100 Packets", hist->pkts_81_100);
	STATS_32(stdout, "101-200 Packets", hist->pkts_101_200);
	STATS_32(stdout, "   200+ Packets", hist->pkts_201_plus);
}

void print_advance_radio_data_tx(struct advance_pdev_data_tx *tx)
{
	print_basic_radio_data_tx(&tx->b_tx);
	print_advance_data_tx_stats(&tx->adv_tx);
	STATS_64(stdout, "Tx Reinject Packets", tx->reinject_pkts.num);
	STATS_64(stdout, "Tx Reinject Bytes", tx->reinject_pkts.bytes);
	STATS_64(stdout, "Tx Inspect Packets", tx->inspect_pkts.num);
	STATS_64(stdout, "Tx Inspect Bytes", tx->inspect_pkts.bytes);
	STATS_32(stdout, "Tx CCE Classified", tx->cce_classified);
	STATS_PRINT("\tTx packets sent per interrupt\n");
	print_histogram_stats(&tx->tx_hist);
}

void print_advance_radio_data_rx(struct advance_pdev_data_rx *rx)
{
	print_basic_radio_data_rx(&rx->b_rx);
	print_advance_data_rx_stats(&rx->adv_rx);
	STATS_PRINT("\tRx packets sent per interrupt\n");
	print_histogram_stats(&rx->rx_hist);
}

void print_advance_radio_data_raw(struct advance_pdev_data_raw *raw)
{
	STATS_64(stdout, "RAW Rx Packets", raw->rx_raw.num);
	STATS_64(stdout, "RAW Rx Bytes", raw->rx_raw.bytes);
	STATS_64(stdout, "RAW Tx Packets", raw->tx_raw_pkt.num);
	STATS_64(stdout, "RAW Tx Bytes", raw->tx_raw_pkt.bytes);
	STATS_32(stdout, "RAW Tx Classified by CCE", raw->cce_classified_raw);
	STATS_32(stdout, "RAW Rx Packets", raw->rx_raw_pkts);
}

void print_advance_radio_data_tso(struct advance_pdev_data_tso *tso)
{
	STATS_64(stdout, "SG Packets", tso->sg_pkt.num);
	STATS_64(stdout, "SG Bytess", tso->sg_pkt.bytes);
	STATS_64(stdout, "Non-SG Packets", tso->non_sg_pkts.num);
	STATS_64(stdout, "Non-SG Bytess", tso->non_sg_pkts.bytes);
	STATS_64(stdout, "TSO Packets", tso->num_tso_pkts.num);
	STATS_64(stdout, "TSO Bytess", tso->num_tso_pkts.bytes);
	STATS_32(stdout, "TSO Complete", tso->tso_comp);
	STATS_PRINT("\tTSO Histogram\n");
	STATS_64(stdout, "    Single", tso->segs_1);
	STATS_64(stdout, "  2-5 segs", tso->segs_2_5);
	STATS_64(stdout, " 6-10 segs", tso->segs_6_10);
	STATS_64(stdout, "11-15 segs", tso->segs_11_15);
	STATS_64(stdout, "16-20 segs", tso->segs_16_20);
	STATS_64(stdout, "  20+ segs", tso->segs_20_plus);
}

void print_advance_radio_data_igmp(struct advance_pdev_data_igmp *igmp)
{
	STATS_32(stdout, "IGMP Received", igmp->igmp_rcvd);
	STATS_32(stdout, "IGMP Converted Unicast", igmp->igmp_ucast_converted);
}

void print_advance_radio_data_mesh(struct advance_pdev_data_mesh *mesh)
{
	STATS_32(stdout, "MESH FW Completion Count", mesh->completion_fw);
	STATS_32(stdout, "MESH FW Exception Count", mesh->exception_fw);
}

void print_advance_radio_data_nawds(struct advance_pdev_data_nawds *nawds)
{
	STATS_64(stdout, "Multicast Tx Packets", nawds->tx_nawds_mcast.num);
	STATS_64(stdout, "Multicast Tx Bytes", nawds->tx_nawds_mcast.bytes);
	STATS_32(stdout, "NAWDS Tx Drop Count", nawds->nawds_mcast_tx_drop);
	STATS_32(stdout, "NAWDS Rx Drop Count", nawds->nawds_mcast_rx_drop);
}

void print_advance_radio_ctrl_tx(struct advance_pdev_ctrl_tx *tx)
{
	print_basic_radio_ctrl_tx(&tx->b_tx);
	STATS_64(stdout, "Tx Beacon Count", tx->cs_tx_beacon);
}

void print_advance_radio_ctrl_rx(struct advance_pdev_ctrl_rx *rx)
{
	print_basic_radio_ctrl_rx(&rx->b_rx);
	STATS_32(stdout, "Rx Mgmt Frames dropped (RSSI too low)",
		 rx->cs_rx_mgmt_rssi_drop);
}

void print_advance_radio_ctrl_link(struct advance_pdev_ctrl_link *link)
{
	print_basic_radio_ctrl_link(&link->b_link);
	STATS_8(stdout, "DCS Tx Utilized", link->dcs_ap_tx_util);
	STATS_8(stdout, "DCS Rx Utilized", link->dcs_ap_rx_util);
	STATS_8(stdout, "DCS Self BSS Utilized", link->dcs_self_bss_util);
	STATS_8(stdout, "DCS OBSS Utilized", link->dcs_obss_util);
	STATS_8(stdout, "DCS OBSS Rx Utilised", link->dcs_obss_rx_util);
	STATS_8(stdout, "DCS Free Medium", link->dcs_free_medium);
	STATS_8(stdout, "DCS Non-WiFi Utilized", link->dcs_non_wifi_util);
	STATS_32(stdout, "DCS Under SS Utilized", link->dcs_ss_under_util);
	STATS_32(stdout, "DCS Secondary 20 Utilized", link->dcs_sec_20_util);
	STATS_32(stdout, "DCS Secondary 40 Utilized", link->dcs_sec_40_util);
	STATS_32(stdout, "DCS Secondary 80 Utilized", link->dcs_sec_80_util);
	STATS_8(stdout, "Rx RSSI (Chain 0) Pri 20", link->rx_rssi_chain0_pri20);
	STATS_8(stdout, "Rx RSSI (Chain 0) Sec 20", link->rx_rssi_chain0_sec20);
	STATS_8(stdout, "Rx RSSI (Chain 0) Sec 40", link->rx_rssi_chain0_sec40);
	STATS_8(stdout, "Rx RSSI (Chain 0) Sec 80", link->rx_rssi_chain0_sec80);
	STATS_8(stdout, "Rx RSSI (Chain 1) Pri 20", link->rx_rssi_chain1_pri20);
	STATS_8(stdout, "Rx RSSI (Chain 1) Sec 20", link->rx_rssi_chain1_sec20);
	STATS_8(stdout, "Rx RSSI (Chain 1) Sec 40", link->rx_rssi_chain1_sec40);
	STATS_8(stdout, "Rx RSSI (Chain 1) Sec 80", link->rx_rssi_chain1_sec80);
	STATS_8(stdout, "Rx RSSI (Chain 2) Pri 20", link->rx_rssi_chain2_pri20);
	STATS_8(stdout, "Rx RSSI (Chain 2) Sec 20", link->rx_rssi_chain2_sec20);
	STATS_8(stdout, "Rx RSSI (Chain 2) Sec 40", link->rx_rssi_chain2_sec40);
	STATS_8(stdout, "Rx RSSI (Chain 2) Sec 80", link->rx_rssi_chain2_sec80);
	STATS_8(stdout, "Rx RSSI (Chain 3) Pri 20", link->rx_rssi_chain3_pri20);
	STATS_8(stdout, "Rx RSSI (Chain 3) Sec 20", link->rx_rssi_chain3_sec20);
	STATS_8(stdout, "Rx RSSI (Chain 3) Sec 40", link->rx_rssi_chain3_sec40);
	STATS_8(stdout, "Rx RSSI (Chain 3) Sec 80", link->rx_rssi_chain3_sec80);
	STATS_32(stdout, "Tx RSSI", link->cs_tx_rssi);
}

void print_advance_ap_data_tx(struct advance_psoc_data_tx *tx)
{
	print_basic_ap_data_tx(&tx->b_tx);
}

void print_advance_ap_data_rx(struct advance_psoc_data_rx *rx)
{
	print_basic_ap_data_rx(&rx->b_rx);
	STATS_32(stdout, "Rx Ring Error Packets", rx->err_ring_pkts);
	STATS_32(stdout, "Rx Fragments", rx->rx_frags);
	STATS_32(stdout, "Rx HW Reinject", rx->rx_hw_reinject);
	STATS_32(stdout, "Rx BAR Frames", rx->bar_frame);
	STATS_32(stdout, "Rx Rejected", rx->rejected);
	STATS_32(stdout, "Rx RAW Frame Drop", rx->raw_frm_drop);
}
#endif /* WLAN_ADVANCE_TELEMETRY */

void print_basic_sta_data(struct stats_obj *sta)
{
	struct basic_peer_data *data = sta->stats;

	STATS_PRINT("Basic Data STATS For STA %s (Parent %s)\n",
		    macaddr_to_str(sta->u_id.mac_addr), sta->pif_name);
	if (data->tx) {
		STATS_PRINT("Tx Stats\n");
		print_basic_sta_data_tx(data->tx);
	}
	if (data->rx) {
		STATS_PRINT("Rx Stats\n");
		print_basic_sta_data_rx(data->rx);
	}
	if (data->link) {
		STATS_PRINT("Link Stats\n");
		print_basic_sta_data_link(data->link);
	}
	if (data->rate) {
		STATS_PRINT("Rate Stats\n");
		print_basic_sta_data_rate(data->rate);
	}
}

void print_basic_sta_ctrl(struct stats_obj *sta)
{
	struct basic_peer_ctrl *ctrl = sta->stats;

	STATS_PRINT("Basic Control STATS For STA %s (Parent %s)\n",
		    macaddr_to_str(sta->u_id.mac_addr), sta->pif_name);
	if (ctrl->tx) {
		STATS_PRINT("Tx Stats\n");
		print_basic_sta_ctrl_tx(ctrl->tx);
	}
	if (ctrl->rx) {
		STATS_PRINT("Rx Stats\n");
		print_basic_sta_ctrl_rx(ctrl->rx);
	}
	if (ctrl->link) {
		STATS_PRINT("Link Stats\n");
		print_basic_sta_ctrl_link(ctrl->link);
	}
	if (ctrl->rate) {
		STATS_PRINT("Rate Stats\n");
		print_basic_sta_ctrl_rate(ctrl->rate);
	}
}

void print_basic_vap_data(struct stats_obj *vap)
{
	struct basic_vdev_data *data = vap->stats;

	STATS_PRINT("Basic Data STATS For VAP %s (Parent %s)\n",
		    vap->u_id.if_name, vap->pif_name);
	if (data->tx) {
		STATS_PRINT("Tx Stats\n");
		print_basic_vap_data_tx(data->tx);
	}
	if (data->rx) {
		STATS_PRINT("Rx Stats\n");
		print_basic_vap_data_rx(data->rx);
	}
}

void print_basic_vap_ctrl(struct stats_obj *vap)
{
	struct basic_vdev_ctrl *ctrl = vap->stats;

	STATS_PRINT("Basic Control STATS For VAP %s (Parent %s)\n",
		    vap->u_id.if_name, vap->pif_name);
	if (ctrl->tx) {
		STATS_PRINT("Tx Stats\n");
		print_basic_vap_ctrl_tx(ctrl->tx);
	}
	if (ctrl->rx) {
		STATS_PRINT("Rx Stats\n");
		print_basic_vap_ctrl_rx(ctrl->rx);
	}
}

void print_basic_radio_data(struct stats_obj *radio)
{
	struct basic_pdev_data *data = radio->stats;

	STATS_PRINT("Basic Data STATS for Radio %s (Parent %s)\n",
		    radio->u_id.if_name, radio->pif_name);
	if (data->tx) {
		STATS_PRINT("Tx Stats\n");
		print_basic_radio_data_tx(data->tx);
	}
	if (data->rx) {
		STATS_PRINT("Rx Stats\n");
		print_basic_radio_data_rx(data->rx);
	}
}

void print_basic_radio_ctrl(struct stats_obj *radio)
{
	struct basic_pdev_ctrl *ctrl = radio->stats;

	STATS_PRINT("Basic Control STATS for Radio %s (Parent %s)\n",
		    radio->u_id.if_name, radio->pif_name);
	if (ctrl->tx) {
		STATS_PRINT("Tx Stats\n");
		print_basic_radio_ctrl_tx(ctrl->tx);
	}
	if (ctrl->rx) {
		STATS_PRINT("Rx Stats\n");
		print_basic_radio_ctrl_rx(ctrl->rx);
	}
	if (ctrl->link) {
		STATS_PRINT("Link Stats\n");
		print_basic_radio_ctrl_link(ctrl->link);
	}
}

void print_basic_ap_data(struct stats_obj *ap)
{
	struct basic_psoc_data *data = ap->stats;

	STATS_PRINT("Basic Data STATS for AP %s\n", ap->u_id.if_name);
	if (data->tx) {
		STATS_PRINT("Tx Stats\n");
		print_basic_ap_data_tx(data->tx);
	}
	if (data->rx) {
		STATS_PRINT("Rx Stats\n");
		print_basic_ap_data_rx(data->rx);
	}
}

void print_basic_stats(struct stats_obj *obj)
{
	switch (obj->obj_type) {
	case STATS_OBJ_STA:
		if (obj->type == STATS_TYPE_DATA)
			print_basic_sta_data(obj);
		else
			print_basic_sta_ctrl(obj);
		break;
	case STATS_OBJ_VAP:
		if (obj->type == STATS_TYPE_DATA)
			print_basic_vap_data(obj);
		else
			print_basic_vap_ctrl(obj);
		break;
	case STATS_OBJ_RADIO:
		if (obj->type == STATS_TYPE_DATA)
			print_basic_radio_data(obj);
		else
			print_basic_radio_ctrl(obj);
		break;
	case STATS_OBJ_AP:
		if (obj->type == STATS_TYPE_DATA)
			print_basic_ap_data(obj);
		break;
	default:
		STATS_ERR("Invalid object option\n");
	}
}

#if WLAN_ADVANCE_TELEMETRY
void print_advance_sta_data(struct stats_obj *sta)
{
	struct advance_peer_data *data = sta->stats;

	STATS_PRINT("Advance Data STATS For STA %s (Parent %s)\n",
		    macaddr_to_str(sta->u_id.mac_addr), sta->pif_name);
	if (data->tx) {
		STATS_PRINT("Tx Stats\n");
		print_advance_sta_data_tx(data->tx);
	}
	if (data->rx) {
		STATS_PRINT("Rx Stats\n");
		print_advance_sta_data_rx(data->rx);
	}
	if (data->fwd) {
		STATS_PRINT("Intra Bss (FWD) Stats\n");
		print_advance_sta_data_fwd(data->fwd);
	}
	if (data->raw) {
		STATS_PRINT("RAW Stats\n");
		print_advance_sta_data_raw(data->raw);
	}
	if (data->twt) {
		STATS_PRINT("TWT Stats\n");
		print_advance_sta_data_twt(data->twt);
	}
	if (data->link) {
		STATS_PRINT("Link Stats\n");
		print_advance_sta_data_link(data->link);
	}
	if (data->rate) {
		STATS_PRINT("Rate Stats\n");
		print_advance_sta_data_rate(data->rate);
	}
	if (data->nawds) {
		STATS_PRINT("NAWDS Stats\n");
		print_advance_sta_data_nawds(data->nawds);
	}
	if (data->delay) {
		STATS_PRINT("DELAY Stats\n");
		print_advance_sta_data_delay(data->delay);
	}
	if (data->jitter) {
		STATS_PRINT("JITTER Stats\n");
		print_advance_sta_data_jitter(data->jitter);
	}
	if (data->sawfdelay) {
		STATS_PRINT("SAWFDELAY Stats\n");
		print_advance_sta_data_sawf_delay(data->sawfdelay, sta->serviceid);
	}
	if (data->sawftx) {
		STATS_PRINT("SAWFTX Stats\n");
		print_advance_sta_data_sawf_tx(data->sawftx, sta->serviceid);
	}
}

void print_advance_sta_ctrl(struct stats_obj *sta)
{
	struct advance_peer_ctrl *ctrl = sta->stats;

	STATS_PRINT("Advance Control STATS For STA %s (Parent %s)\n",
		    macaddr_to_str(sta->u_id.mac_addr), sta->pif_name);
	if (ctrl->tx) {
		STATS_PRINT("Tx Stats\n");
		print_advance_sta_ctrl_tx(ctrl->tx);
	}
	if (ctrl->rx) {
		STATS_PRINT("Rx Stats\n");
		print_advance_sta_ctrl_rx(ctrl->rx);
	}
	if (ctrl->twt) {
		STATS_PRINT("TWT Stats\n");
		print_advance_sta_ctrl_twt(ctrl->twt);
	}
	if (ctrl->link) {
		STATS_PRINT("Link Stats\n");
		print_advance_sta_ctrl_link(ctrl->link);
	}
	if (ctrl->rate) {
		STATS_PRINT("Rate Stats\n");
		print_advance_sta_ctrl_rate(ctrl->rate);
	}
}

void print_advance_vap_data(struct stats_obj *vap)
{
	struct advance_vdev_data *data = vap->stats;

	STATS_PRINT("Advance Data STATS for Vap %s (Parent %s)\n",
		    vap->u_id.if_name, vap->pif_name);
	if (data->me) {
		STATS_PRINT("ME Stats\n");
		print_advance_vap_data_me(data->me);
	}
	if (data->tx) {
		STATS_PRINT("Tx Stats\n");
		print_advance_vap_data_tx(data->tx);
	}
	if (data->rx) {
		STATS_PRINT("Rx Stats\n");
		print_advance_vap_data_rx(data->rx);
	}
	if (data->raw) {
		STATS_PRINT("RAW Stats\n");
		print_advance_vap_data_raw(data->raw);
	}
	if (data->tso) {
		STATS_PRINT("TSO Stats\n");
		print_advance_vap_data_tso(data->tso);
	}
	if (data->igmp) {
		STATS_PRINT("IGMP Stats\n");
		print_advance_vap_data_igmp(data->igmp);
	}
	if (data->mesh) {
		STATS_PRINT("MESH Stats\n");
		print_advance_vap_data_mesh(data->mesh);
	}
	if (data->nawds) {
		STATS_PRINT("NAWDS Stats\n");
		print_advance_vap_data_nawds(data->nawds);
	}
}

void print_advance_vap_ctrl(struct stats_obj *vap)
{
	struct advance_vdev_ctrl *ctrl = vap->stats;

	STATS_PRINT("Advance Control STATS for Vap %s (Parent %s)\n",
		    vap->u_id.if_name, vap->pif_name);
	if (ctrl->tx) {
		STATS_PRINT("Tx Stats\n");
		print_advance_vap_ctrl_tx(ctrl->tx);
	}
	if (ctrl->rx) {
		STATS_PRINT("Rx Stats\n");
		print_advance_vap_ctrl_rx(ctrl->rx);
	}
}

void print_advance_radio_data(struct stats_obj *radio)
{
	struct advance_pdev_data *data = radio->stats;

	STATS_PRINT("Advance Data STATS for Radio %s (Parent %s)\n",
		    radio->u_id.if_name, radio->pif_name);
	if (data->me) {
		STATS_PRINT("ME Stats\n");
		print_advance_radio_data_me(data->me);
	}
	if (data->tx) {
		STATS_PRINT("Tx Stats\n");
		print_advance_radio_data_tx(data->tx);
	}
	if (data->rx) {
		STATS_PRINT("Rx Stats\n");
		print_advance_radio_data_rx(data->rx);
	}
	if (data->raw) {
		STATS_PRINT("RAW Stats\n");
		print_advance_radio_data_raw(data->raw);
	}
	if (data->tso) {
		STATS_PRINT("TSO Stats\n");
		print_advance_radio_data_tso(data->tso);
	}
	if (data->igmp) {
		STATS_PRINT("IGMP Stats\n");
		print_advance_radio_data_igmp(data->igmp);
	}
	if (data->mesh) {
		STATS_PRINT("MESH Stats\n");
		print_advance_radio_data_mesh(data->mesh);
	}
	if (data->nawds) {
		STATS_PRINT("NAWDS Stats\n");
		print_advance_radio_data_nawds(data->nawds);
	}
}

void print_advance_radio_ctrl(struct stats_obj *radio)
{
	struct advance_pdev_ctrl *ctrl = radio->stats;

	STATS_PRINT("Advance Control STATS for Radio %s (Parent %s)\n",
		    radio->u_id.if_name, radio->pif_name);
	if (ctrl->tx) {
		STATS_PRINT("Tx Stats\n");
		print_advance_radio_ctrl_tx(ctrl->tx);
	}
	if (ctrl->rx) {
		STATS_PRINT("Rx Stats\n");
		print_advance_radio_ctrl_rx(ctrl->rx);
	}
	if (ctrl->link) {
		STATS_PRINT("Link Stats\n");
		print_advance_radio_ctrl_link(ctrl->link);
	}
}

void print_advance_ap_data(struct stats_obj *ap)
{
	struct advance_psoc_data *data = ap->stats;

	STATS_PRINT("Advance Data STATS for AP %s\n", ap->u_id.if_name);
	if (data->tx) {
		STATS_PRINT("Tx Stats\n");
		print_advance_ap_data_tx(data->tx);
	}
	if (data->rx) {
		STATS_PRINT("Rx Stats\n");
		print_advance_ap_data_rx(data->rx);
	}
}

void print_advance_stats(struct stats_obj *obj)
{
	switch (obj->obj_type) {
	case STATS_OBJ_STA:
		if (obj->type == STATS_TYPE_DATA)
			print_advance_sta_data(obj);
		else
			print_advance_sta_ctrl(obj);
		break;
	case STATS_OBJ_VAP:
		if (obj->type == STATS_TYPE_DATA)
			print_advance_vap_data(obj);
		else
			print_advance_vap_ctrl(obj);
		break;
	case STATS_OBJ_RADIO:
		if (obj->type == STATS_TYPE_DATA)
			print_advance_radio_data(obj);
		else
			print_advance_radio_ctrl(obj);
		break;
	case STATS_OBJ_AP:
		if (obj->type == STATS_TYPE_DATA)
			print_advance_ap_data(obj);
		break;
	default:
		STATS_ERR("Invalid object option\n");
	}
}
#endif /* WLAN_ADVANCE_TELEMETRY */

#if WLAN_DEBUG_TELEMETRY
#ifdef VDEV_PEER_PROTOCOL_COUNT
static void stats_if_print_tx_proto_trace(struct debug_data_tx_stats *tx)
{
	STATS_PRINT("\tTx Data Packets PROTOCOL Based:\n");
	STATS_16(stdout, "ICMP tx ingress",
		 tx->protocol_trace_cnt[STATS_IF_TRACE_ICMP].ingress_cnt);
	STATS_16(stdout, "ICMP tx egress",
		 tx->protocol_trace_cnt[STATS_IF_TRACE_ICMP].egress_cnt);
	STATS_16(stdout, "ARP tx ingress",
		 tx->protocol_trace_cnt[STATS_IF_TRACE_ARP].ingress_cnt);
	STATS_16(stdout, "ARP tx egress",
		 tx->protocol_trace_cnt[STATS_IF_TRACE_ARP].egress_cnt);
	STATS_16(stdout, "EAP tx ingress",
		 tx->protocol_trace_cnt[STATS_IF_TRACE_EAP].ingress_cnt);
	STATS_16(stdout, "EAP tx egress",
		 tx->protocol_trace_cnt[STATS_IF_TRACE_EAP].egress_cnt);
}

static void stats_if_print_rx_proto_trace(struct debug_data_rx_stats *rx)
{
	STATS_PRINT("\tRx Data Packets PROTOCOL Based:\n");
	STATS_16(stdout, "ICMP rx ingress",
		 rx->protocol_trace_cnt[STATS_IF_TRACE_ICMP].ingress_cnt);
	STATS_16(stdout, "ICMP rx egress",
		 rx->protocol_trace_cnt[STATS_IF_TRACE_ICMP].egress_cnt);
	STATS_16(stdout, "ARP rx ingress",
		 rx->protocol_trace_cnt[STATS_IF_TRACE_ARP].ingress_cnt);
	STATS_16(stdout, "ARP rx egress",
		 rx->protocol_trace_cnt[STATS_IF_TRACE_ARP].egress_cnt);
	STATS_16(stdout, "EAP rx ingress",
		 rx->protocol_trace_cnt[STATS_IF_TRACE_EAP].ingress_cnt);
	STATS_16(stdout, "EAP rx egress",
		 rx->protocol_trace_cnt[STATS_IF_TRACE_EAP].egress_cnt);
}
#else
static void stats_if_print_tx_proto_trace(struct debug_data_tx_stats *tx)
{
}

static void stats_if_print_rx_proto_trace(struct debug_data_rx_stats *rx)
{
}
#endif /* VDEV_PEER_PROTOCOL_COUNT */

static void stats_if_print_ru_loc(struct debug_data_tx_stats *tx)
{
	uint8_t i;

	STATS_PRINT("\tRU Locations:\n");
	STATS_PRINT("\t\tMSDU:\n");
	for (i = 0; i < STATS_IF_RU_INDEX_MAX; i++) {
		STATS_PRINT("\t\t%s:  %u\n", ru_string[i].ru_type,
			    tx->ru_loc[i].num_msdu);
	}
	STATS_PRINT("\t\tMPDU:\n");
	for (i = 0; i < STATS_IF_RU_INDEX_MAX; i++) {
		STATS_PRINT("\t\t%s:  %u\n", ru_string[i].ru_type,
			    tx->ru_loc[i].num_mpdu);
	}
	STATS_PRINT("\t\tMPDU Tried:\n");
	for (i = 0; i < STATS_IF_RU_INDEX_MAX; i++) {
		STATS_PRINT("\t\t%s:  %u\n", ru_string[i].ru_type,
			    tx->ru_loc[i].mpdu_tried);
	}
}

void print_debug_data_tx_stats(struct debug_data_tx_stats *tx)
{
	char mu_group_id[STATS_IF_MU_GROUP_LENGTH] = {'\0'};
	uint8_t i, mcs, ptype;
	uint32_t j, index;

	STATS_32(stdout, "Inactive time in seconds", tx->inactive_time);
	STATS_32(stdout, "Total packets as Ofdma", tx->ofdma);
	STATS_32(stdout, "Total packets in STBC", tx->stbc);
	STATS_32(stdout, "Total packets in LDPC", tx->ldpc);
	STATS_32(stdout, "Number of PPDU's with Punctured Preamble",
		 tx->pream_punct_cnt);
	STATS_32(stdout, "Number of stand alone received",
		 tx->num_ppdu_cookie_valid);
	STATS_PRINT("\tPackets dropped on the Tx side:\n");
	STATS_64(stdout, "Firmware discarded packets", tx->fw_rem.num);
	STATS_64(stdout, "Firmware discarded bytes", tx->fw_rem.bytes);
	STATS_32(stdout, "Firmware discard untransmitted", tx->fw_rem_notx);
	STATS_32(stdout, "Firmware discard transmitted", tx->fw_rem_tx);
	STATS_32(stdout, "Aged out in mpdu/msdu queues", tx->age_out);
	STATS_32(stdout, "Firmware discard untransmitted fw_reason1",
		 tx->fw_reason1);
	STATS_32(stdout, "Firmware discard untransmitted fw_reason2",
		 tx->fw_reason2);
	STATS_32(stdout, "Firmware discard untransmitted fw_reason3",
		 tx->fw_reason3);
	STATS_PRINT("\tLast Packet RU index [%u], Size [%u]\n",
		    tx->ru_start, tx->ru_tones);
	STATS_PRINT("\tTx Data Packets per AC\n");
	STATS_32(stdout, "     Best effort",
		 tx->wme_ac_type[STATS_IF_WME_AC_BE]);
	STATS_32(stdout, "      Background",
		 tx->wme_ac_type[STATS_IF_WME_AC_BK]);
	STATS_32(stdout, "           Video",
		 tx->wme_ac_type[STATS_IF_WME_AC_VI]);
	STATS_32(stdout, "           Voice",
		 tx->wme_ac_type[STATS_IF_WME_AC_VO]);
	STATS_PRINT("\tExcess Retries per AC\n");
	STATS_32(stdout, "     Best effort",
		 tx->excess_retries_per_ac[STATS_IF_WME_AC_BE]);
	STATS_32(stdout, "      Background",
		 tx->excess_retries_per_ac[STATS_IF_WME_AC_BK]);
	STATS_32(stdout, "           Video",
		 tx->excess_retries_per_ac[STATS_IF_WME_AC_VI]);
	STATS_32(stdout, "           Voice",
		 tx->excess_retries_per_ac[STATS_IF_WME_AC_VO]);
	STATS_PRINT("\tTransmit Type:\n");
	STATS_PRINT("\tMSDU:SU %u, MU_MIMO %u, MU_OFDMA %u, MU_MIMO_OFDMA %u\n",
		    tx->transmit_type[STATS_IF_SU].num_msdu,
		    tx->transmit_type[STATS_IF_MU_MIMO].num_msdu,
		    tx->transmit_type[STATS_IF_MU_OFDMA].num_msdu,
		    tx->transmit_type[STATS_IF_MU_MIMO_OFDMA].num_msdu);
	STATS_PRINT("\tMPDU:SU %u, MU_MIMO %u, MU_OFDMA %u, MU_MIMO_OFDMA %u\n",
		    tx->transmit_type[STATS_IF_SU].num_mpdu,
		    tx->transmit_type[STATS_IF_MU_MIMO].num_mpdu,
		    tx->transmit_type[STATS_IF_MU_OFDMA].num_mpdu,
		    tx->transmit_type[STATS_IF_MU_MIMO_OFDMA].num_mpdu);
	STATS_PRINT("\tMPDU Tried:");
	STATS_PRINT("SU %u, MU_MIMO %u, MU_OFDMA %u, MU_MIMO_OFDMA %u\n",
		    tx->transmit_type[STATS_IF_SU].mpdu_tried,
		    tx->transmit_type[STATS_IF_MU_MIMO].mpdu_tried,
		    tx->transmit_type[STATS_IF_MU_OFDMA].mpdu_tried,
		    tx->transmit_type[STATS_IF_MU_MIMO_OFDMA].mpdu_tried);
	for (i = 0; i < STATS_IF_MAX_MU_GROUP_ID;) {
		index = 0;
		for (j = 0; j < STATS_IF_MU_GROUP_SHOW &&
		     i < STATS_IF_MAX_MU_GROUP_ID; j++) {
			index += snprintf(&mu_group_id[index],
					  STATS_IF_MU_GROUP_LENGTH - index,
					  " %d", tx->mu_group_id[i]);
			i++;
		}
		STATS_PRINT("\tUser position list for GID %02u->%u: [%s]\n",
			    i - STATS_IF_MU_GROUP_SHOW, i - 1, mu_group_id);
	}
	STATS_PRINT("\tTx MCS stats:\n");
	for (ptype = 0; ptype < STATS_IF_DOT11_MAX; ptype++) {
		for (mcs = 0; mcs < STATS_IF_MAX_MCS; mcs++) {
			if (rate_string[ptype][mcs].valid)
				STATS_PRINT("\t\t %s = %u\n",
					    rate_string[ptype][mcs].mcs_type,
					    tx->pkt_type[ptype].mcs_count[mcs]);
		}
	}

	stats_if_print_ru_loc(tx);
	stats_if_print_tx_proto_trace(tx);

	STATS_PRINT("\tTx Ack not recevied counter:\n");
	for (ptype = STATS_IF_PROTO_EAPOL_M1;
	     ptype < STATS_IF_PROTO_SUBTYPE_MAX; ptype++) {
		if (tx->no_ack_count[ptype])
			STATS_PRINT("\t\t%s = %u\n",
				    proto_subtype_string[ptype],
				    tx->no_ack_count[ptype]);
	}
}

void print_debug_data_rx_stats(struct debug_data_rx_stats *rx)
{
	enum stats_if_mu_packet_type rx_mu_type;
	struct rx_mu_info *rx_mu;
	uint32_t *pnss;
	char nss[STATS_IF_NSS_LENGTH];
	uint8_t i, mcs, ptype;
	uint32_t index;

	STATS_32(stdout, "Rx Dropped", rx->rx_discard);
	STATS_32(stdout, "Rx MIC Error", rx->mic_err);
	STATS_32(stdout, "Rx Decrypt Error", rx->decrypt_err);
	STATS_32(stdout, "Rx FCS Error", rx->fcserr);
	STATS_32(stdout, "Rx PN Error", rx->pn_err);
	STATS_32(stdout, "Rx Out of Order Error", rx->oor_err);
	STATS_64(stdout, "Rx MEC drop packets", rx->mec_drop.num);
	STATS_64(stdout, "Rx MEC drop bytes", rx->mec_drop.bytes);
	STATS_PRINT("\tMSDU Reception Type:\n");
	STATS_PRINT("\t\tSU %u MU_MIMO %u MU_OFDMA %u MU_OFDMA_MIMO %u\n",
		    rx->reception_type[STATS_IF_SU],
		    rx->reception_type[STATS_IF_MU_MIMO],
		    rx->reception_type[STATS_IF_MU_OFDMA],
		    rx->reception_type[STATS_IF_MU_MIMO_OFDMA]);
	STATS_PRINT("\tPPDU Reception Type:\n");
	STATS_PRINT("\t\tSU %u MU_MIMO %u MU_OFDMA %u MU_OFDMA_MIMO %u\n",
		    rx->ppdu_cnt[STATS_IF_SU],
		    rx->ppdu_cnt[STATS_IF_MU_MIMO],
		    rx->ppdu_cnt[STATS_IF_MU_OFDMA],
		    rx->ppdu_cnt[STATS_IF_MU_MIMO_OFDMA]);
	for (rx_mu_type = 0; rx_mu_type < STATS_IF_TXRX_TYPE_MU_MAX;
	     rx_mu_type++) {
		STATS_PRINT("\tReception mode %s\n",
			    mu_reception_mode[rx_mu_type]);
		rx_mu = &rx->rx_mu[rx_mu_type];
		pnss = &rx_mu->ppdu_nss[0];
		index = 0;
		for (i = 0; i < STATS_IF_SS_COUNT; i++) {
			index += snprintf(&nss[index],
					  STATS_IF_NSS_LENGTH - index,
					  " %u", *(pnss + i));
		}
		STATS_PRINT("\t\tPPDU Count\n");
		STATS_PRINT("\t\t\tNSS(1-8) = %s\n", nss);
		STATS_PRINT("\t\t\tMPDU OK = %u, MPDU Fail = %u\n",
			    rx_mu->mpdu_cnt_fcs_ok, rx_mu->mpdu_cnt_fcs_err);
	}
	STATS_PRINT("\tRx MCS stats:\n");
	for (ptype = 0; ptype < STATS_IF_DOT11_MAX; ptype++) {
		for (mcs = 0; mcs < STATS_IF_MAX_MCS; mcs++) {
			if (rate_string[ptype][mcs].valid)
				STATS_PRINT("\t\t %s = %u\n",
					    rate_string[ptype][mcs].mcs_type,
					    rx->pkt_type[ptype].mcs_count[mcs]);
		}
	}

	stats_if_print_rx_proto_trace(rx);

	for (i = 0; i <  STATS_IF_MAX_RX_RINGS; i++) {
		STATS_PRINT("\tRing Id = %u", i);
		STATS_64(stdout, "Packets Received", rx->rcvd_reo[i].num);
		STATS_64(stdout, "\t\tBytes Received", rx->rcvd_reo[i].bytes);
	}
}

void print_debug_sta_data_tx(struct debug_peer_data_tx *tx)
{
	print_basic_sta_data_tx(&tx->b_tx);
	STATS_PRINT("\tTx Stats in Last One Second:\n");
	STATS_32(stdout, "Tx success packets", tx->tx_data_success_last);
	STATS_32(stdout, "Tx success bytes", tx->tx_bytes_success_last);
	STATS_32(stdout, "Tx packets", tx->tx_data_rate);
	STATS_32(stdout, "Tx bytes", tx->tx_byte_rate);
	STATS_32(stdout, "Tx Packet Error Rate (PER)", tx->last_per);
	STATS_32(stdout, "Tx unicast data", tx->tx_data_ucast_last);
	STATS_32(stdout, "Tx unicast data rate", tx->tx_data_ucast_rate);
	STATS_PRINT("\n");
	print_debug_data_tx_stats(&tx->dbg_tx);
}

void print_debug_sta_data_rx(struct debug_peer_data_rx *rx)
{
	print_basic_sta_data_rx(&rx->b_rx);
	STATS_PRINT("\tRx Stats in Last One Second:\n");
	STATS_32(stdout, "Rx packets", rx->rx_data_success_last);
	STATS_32(stdout, "Rx bytes", rx->rx_bytes_success_last);
	STATS_32(stdout, "Rx packets", rx->rx_data_rate);
	STATS_32(stdout, "Rx bytes", rx->rx_byte_rate);
	STATS_PRINT("\n");
	print_debug_data_rx_stats(&rx->dbg_rx);
}

void print_debug_sta_data_link(struct debug_peer_data_link *link)
{
	print_basic_sta_data_link(&link->b_link);
	STATS_32(stdout, "Last ack rssi", link->last_ack_rssi);
}

void print_debug_sta_data_rate(struct debug_peer_data_rate *rate)
{
	print_basic_sta_data_rate(&rate->b_rate);
	STATS_32(stdout, "Last Tx rate for unicast Packets(mcs)",
		 rate->last_tx_rate_mcs);
	STATS_32(stdout, "Last Tx rate for multicast Packets",
		 rate->mcast_last_tx_rate);
	STATS_32(stdout, "Last Tx rate for multicast Packets(mcs)",
		 rate->mcast_last_tx_rate_mcs);
}

void print_debug_sta_data_txcap(struct debug_peer_data_txcap *txcap)
{
	uint8_t tid;

	for (tid = 0; tid < STATS_IF_MAX_TIDS; tid++) {
		STATS_PRINT("\tTID[%u]: defer_msdu_q[%ju] ", tid,
			    txcap->defer_msdu_len[tid]);
		STATS_PRINT("msdu_comp_q[%ju] pending_ppdu_q[%ju]\n",
			    txcap->tasklet_msdu_len[tid],
			    txcap->pending_q_len[tid]);
	}
	STATS_PRINT("\tMSDU SUCC:%u ENQ:%u DEQ:%u FLUSH:%u DROP:%u XRETRY:%u\n",
		    txcap->msdu[STATS_IF_MSDU_SUCC],
		    txcap->msdu[STATS_IF_MSDU_ENQ],
		    txcap->msdu[STATS_IF_MSDU_DEQ],
		    txcap->msdu[STATS_IF_MSDU_FLUSH],
		    txcap->msdu[STATS_IF_MSDU_DROP],
		    txcap->msdu[STATS_IF_MSDU_XRETRY]);
	STATS_PRINT("\tMPDU TRI:%u SUCC:%u RESTITCH:%u ARR:%u CLONE:%u",
		    txcap->mpdu[STATS_IF_MPDU_TRI],
		    txcap->mpdu[STATS_IF_MPDU_SUCC],
		    txcap->mpdu[STATS_IF_MPDU_RESTITCH],
		    txcap->mpdu[STATS_IF_MPDU_ARR],
		    txcap->mpdu[STATS_IF_MPDU_CLONE]);
	STATS_PRINT(" TO STACK:%u\n", txcap->mpdu[STATS_IF_MPDU_TO_STACK]);
}

void print_debug_sta_ctrl_tx(struct debug_peer_ctrl_tx *tx)
{
	print_basic_sta_ctrl_tx(&tx->b_tx);
	STATS_32(stdout, "Tx Packets Discard as Power save aged",
		 tx->cs_ps_discard);
	STATS_32(stdout, "Tx Packets dropped form PS queue", tx->cs_psq_drops);
}

void print_debug_sta_ctrl_rx(struct debug_peer_ctrl_rx *rx)
{
	print_basic_sta_ctrl_rx(&rx->b_rx);
}

void print_debug_sta_ctrl_link(struct debug_peer_ctrl_link *link)
{
	print_basic_sta_ctrl_link(&link->b_link);
}

void print_debug_sta_ctrl_rate(struct debug_peer_ctrl_rate *rate)
{
	print_basic_sta_ctrl_rate(&rate->b_rate);
}

void print_debug_sta_data(struct stats_obj *sta)
{
	struct debug_peer_data *data = sta->stats;

	STATS_PRINT("Debug Data STATS For STA %s (Parent %s)\n",
		    macaddr_to_str(sta->u_id.mac_addr), sta->pif_name);
	if (data->tx) {
		STATS_PRINT("Tx Stats\n");
		print_debug_sta_data_tx(data->tx);
	}
	if (data->rx) {
		STATS_PRINT("Rx Stats\n");
		print_debug_sta_data_rx(data->rx);
	}
	if (data->link) {
		STATS_PRINT("Link Stats\n");
		print_debug_sta_data_link(data->link);
	}
	if (data->rate) {
		STATS_PRINT("Rate Stats\n");
		print_debug_sta_data_rate(data->rate);
	}
	if (data->txcap) {
		STATS_PRINT("Tx Capture Stats\n");
		print_debug_sta_data_txcap(data->txcap);
	}
}

void print_debug_sta_ctrl(struct stats_obj *sta)
{
	struct debug_peer_ctrl *ctrl = sta->stats;

	STATS_PRINT("Debug Control STATS For STA %s (Parent %s)\n",
		    macaddr_to_str(sta->u_id.mac_addr), sta->pif_name);
	if (ctrl->tx) {
		STATS_PRINT("Tx Stats\n");
		print_debug_sta_ctrl_tx(ctrl->tx);
	}
	if (ctrl->rx) {
		STATS_PRINT("Rx Stats\n");
		print_debug_sta_ctrl_rx(ctrl->rx);
	}
	if (ctrl->link) {
		STATS_PRINT("Link Stats\n");
		print_debug_sta_ctrl_link(ctrl->link);
	}
	if (ctrl->rate) {
		STATS_PRINT("Rate Stats\n");
		print_debug_sta_ctrl_rate(ctrl->rate);
	}
}

void print_debug_vap_data_me(struct debug_vdev_data_me *me)
{
	STATS_32(stdout, "Packets dropped due to map error",
		 me->dropped_map_error);
	STATS_32(stdout, "Packets dropped due to self Mac address",
		 me->dropped_self_mac);
	STATS_32(stdout, "Packets dropped due to send fail",
		 me->dropped_send_fail);
	STATS_32(stdout, "Segment allocation failure", me->fail_seg_alloc);
	STATS_32(stdout, "NBUF clone failure", me->clone_fail);
}

void print_debug_vap_data_tx(struct debug_vdev_data_tx *tx)
{
	print_basic_vap_data_tx(&tx->b_tx);
	print_debug_data_tx_stats(&tx->dbg_tx);
	STATS_PRINT("\tPackets dropped on the Tx ingress side:\n");
	STATS_64(stdout, "Packets dropped Desc Not Available", tx->desc_na.num);
	STATS_64(stdout, "Bytes dropped Desc Not Available", tx->desc_na.bytes);
	STATS_64(stdout, "Packets dropped Descriptor alloc fail",
		 tx->desc_na_exc_alloc_fail.num);
	STATS_64(stdout, "Bytes dropped Descriptor alloc fail",
		 tx->desc_na_exc_alloc_fail.bytes);
	STATS_64(stdout, "Packets dropped Tx outstanding too many",
		 tx->desc_na_exc_outstand.num);
	STATS_64(stdout, "Bytes dropped Tx outstanding too many",
		 tx->desc_na_exc_outstand.bytes);
	STATS_64(stdout, "Packets dropped as Exception pkts more than limit",
		 tx->exc_desc_na.num);
	STATS_64(stdout, "Bytes dropped as Exception pkts more than limit",
		 tx->exc_desc_na.bytes);
	STATS_64(stdout, "Packets dropped due to sniffer received",
		 tx->sniffer_rcvd.num);
	STATS_64(stdout, "Bytes dropped due to sniffer received",
		 tx->sniffer_rcvd.bytes);
	STATS_32(stdout, "Packets dropped due to Ring full", tx->ring_full);
	STATS_32(stdout, "Packets dropped due to HW enqueue failed",
		 tx->enqueue_fail);
	STATS_32(stdout, "Packets dropped due to DMA error", tx->dma_error);
	STATS_32(stdout, "Packets dropped due to Resource Full", tx->res_full);
	STATS_32(stdout, "Packets dropped due to Insufficient Headroom",
		 tx->headroom_insufficient);
	STATS_32(stdout, "Packets dropped due to per Pkt vdev id check fail",
		 tx->fail_per_pkt_vdev_id_check);
}

void print_debug_vap_data_rx(struct debug_vdev_data_rx *rx)
{
	print_basic_vap_data_rx(&rx->b_rx);
	print_debug_data_rx_stats(&rx->dbg_rx);
}

void print_debug_vap_data_raw(struct debug_vdev_data_raw *raw)
{
	STATS_32(stdout, "DMA map error", raw->dma_map_error);
	STATS_32(stdout, "Packet type not data error",
		 raw->invalid_raw_pkt_datatype);
	STATS_32(stdout, "Frags count overflow error",
		 raw->num_frags_overflow_err);
}

void print_debug_vap_data_tso(struct debug_vdev_data_tso *tso)
{
	STATS_64(stdout, "Packets dropped by host", tso->dropped_host.num);
	STATS_64(stdout, "Bytes dropped by host", tso->dropped_host.bytes);
	STATS_32(stdout, "TSO DMA map error", tso->dma_map_error);
	STATS_32(stdout, "Packets dropped by target", tso->dropped_target);
}

void print_debug_vap_ctrl_tx(struct debug_vdev_ctrl_tx *tx)
{
	print_basic_vap_ctrl_tx(&tx->b_tx);
	STATS_64(stdout, "Tx SWBA Beacon interval Counter", tx->cs_tx_bcn_swba);
	STATS_64(stdout, "Tx failed due to no Node", tx->cs_tx_nonode);
	STATS_64(stdout, "Tx Not ok set in descriptor", tx->cs_tx_not_ok);
}

void print_debug_vap_ctrl_rx(struct debug_vdev_ctrl_rx *rx)
{
	print_basic_vap_ctrl_rx(&rx->b_rx);
	STATS_64(stdout, "Invalid mac address: node alloc failure",
		 rx->cs_invalid_macaddr_nodealloc_fail);
	STATS_64(stdout, "Packets due to wrong direction",
		 rx->cs_rx_wrongdir);
	STATS_64(stdout, "Packets dropped due to sta not associated",
		 rx->cs_rx_not_assoc);
	STATS_64(stdout, "Packets dropped due to rateset truncated",
		 rx->cs_rx_rs_too_big);
	STATS_64(stdout, "Packets dropped due to required element missing",
		 rx->cs_rx_elem_missing);
	STATS_64(stdout, "Packets dropped due to element too big",
		 rx->cs_rx_elem_too_big);
	STATS_64(stdout, "Packets with invalid channel dropped",
		 rx->cs_rx_chan_err);
	STATS_64(stdout, "Packets dropped due to node allocation failure",
		 rx->cs_rx_node_alloc);
	STATS_64(stdout, "Packets with unsupported Auth algo dropped",
		 rx->cs_rx_auth_unsupported);
	STATS_64(stdout, "STA Auth failure", rx->cs_rx_auth_fail);
	STATS_64(stdout, "Auth discard due to counter measure",
		 rx->cs_rx_auth_countermeasures);
	STATS_64(stdout, "Assoc from wrong BSS", rx->cs_rx_assoc_bss);
	STATS_64(stdout, "Auth without assoc", rx->cs_rx_assoc_notauth);
	STATS_64(stdout, "Assoc with Capabilities mismatch",
		 rx->cs_rx_assoc_cap_mismatch);
	STATS_64(stdout, "Assoc with no rate match", rx->cs_rx_assoc_norate);
	STATS_64(stdout, "Assoc with bad WPA IE", rx->cs_rx_assoc_wpaie_err);
	STATS_64(stdout, "Bad auth request", rx->cs_rx_auth_err);
	STATS_64(stdout, "Rx discarded due to ACL policy", rx->cs_rx_acl);
	STATS_64(stdout, "Rx 4-addressed pkts while WDS disable",
		 rx->cs_rx_nowds);
	STATS_64(stdout, "Rx from wrong BSSID", rx->cs_rx_wrongbss);
	STATS_64(stdout, "Rx packet length too short", rx->cs_rx_tooshort);
	STATS_64(stdout, "Rx SSID mismatch", rx->cs_rx_ssid_mismatch);
	STATS_64(stdout, "Rx Ucast Dycrypt Ok", rx->cs_rx_decryptok_u);
	STATS_64(stdout, "Rx Mcast Dycrypt Ok", rx->cs_rx_decryptok_m);
}

void print_debug_vap_ctrl_wmi(struct debug_vdev_ctrl_wmi *wmi)
{
	STATS_64(stdout, "Peer delete req", wmi->cs_peer_delete_req);
	STATS_64(stdout, "Peer delete resp", wmi->cs_peer_delete_resp);
	STATS_64(stdout, "Peer delete all req", wmi->cs_peer_delete_all_req);
	STATS_64(stdout, "Peer delete all resp", wmi->cs_peer_delete_all_resp);
}

void print_debug_vap_data(struct stats_obj *vap)
{
	struct debug_vdev_data *data = vap->stats;

	STATS_PRINT("Debug Data STATS for Vap %s (Parent %s)\n",
		    vap->u_id.if_name, vap->pif_name);
	if (data->tx) {
		STATS_PRINT("Tx Stats\n");
		print_debug_vap_data_tx(data->tx);
	}
	if (data->rx) {
		STATS_PRINT("Rx Stats\n");
		print_debug_vap_data_rx(data->rx);
	}
	if (data->me) {
		STATS_PRINT("ME Stats\n");
		print_debug_vap_data_me(data->me);
	}
	if (data->raw) {
		STATS_PRINT("RAW Stats\n");
		print_debug_vap_data_raw(data->raw);
	}
	if (data->tso) {
		STATS_PRINT("TSO Stats\n");
		print_debug_vap_data_tso(data->tso);
	}
}

void print_debug_vap_ctrl(struct stats_obj *vap)
{
	struct debug_vdev_ctrl *ctrl = vap->stats;

	STATS_PRINT("Debug Control STATS for Vap %s (Parent %s)\n",
		    vap->u_id.if_name, vap->pif_name);
	if (ctrl->tx) {
		STATS_PRINT("Tx Stats\n");
		print_debug_vap_ctrl_tx(ctrl->tx);
	}
	if (ctrl->rx) {
		STATS_PRINT("Rx Stats\n");
		print_debug_vap_ctrl_rx(ctrl->rx);
	}
	if (ctrl->wmi) {
		STATS_PRINT("WMI Stats\n");
		print_debug_vap_ctrl_wmi(ctrl->wmi);
	}
}

void print_debug_radio_data_me(struct debug_pdev_data_me *me)
{
	STATS_32(stdout, "Packets dropped due to map error",
		 me->dropped_map_error);
	STATS_32(stdout, "Packets dropped due to self Mac address",
		 me->dropped_self_mac);
	STATS_32(stdout, "Packets dropped due to send fail",
		 me->dropped_send_fail);
	STATS_32(stdout, "Segment allocation failure", me->fail_seg_alloc);
	STATS_32(stdout, "NBUF clone failure", me->clone_fail);
}

void print_debug_radio_data_tx(struct debug_pdev_data_tx *tx)
{
	print_basic_radio_data_tx(&tx->b_tx);
	print_debug_data_tx_stats(&tx->dbg_tx);
}

void print_debug_radio_data_rx(struct debug_pdev_data_rx *rx)
{
	print_basic_radio_data_rx(&rx->b_rx);
	print_debug_data_rx_stats(&rx->dbg_rx);
	STATS_PRINT("\tRx Packets dropped:\n");
	STATS_64(stdout, "Total Packets replenished", rx->replenished_pkts.num);
	STATS_64(stdout, "Total Bytes replenished", rx->replenished_pkts.bytes);
	STATS_32(stdout, "Rx DMA Error replenished", rx->rxdma_err);
	STATS_32(stdout, "NBUG allocation failed", rx->nbuf_alloc_fail);
	STATS_32(stdout, "Frag allocation failed", rx->frag_alloc_fail);
	STATS_32(stdout, "MAP error", rx->map_err);
	STATS_32(stdout, "x86 failure", rx->x86_fail);
	STATS_32(stdout, "Low threshold interrupts", rx->low_thresh_intrs);
	STATS_32(stdout, "Packets dropped because msdu_done bit not set",
		 rx->msdu_not_done);
	STATS_32(stdout, "Multicast Echo check", rx->mec);
	STATS_32(stdout, "Mesh Filtered packets", rx->mesh_filter);
	STATS_32(stdout, "Wifi Parse failed", rx->wifi_parse);
	STATS_32(stdout, "Mon Rx drop", rx->mon_rx_drop);
	STATS_32(stdout, "Mon Radio Tap error drop",
		 rx->mon_radiotap_update_err);
	STATS_32(stdout, "Descriptor alloc fail", rx->desc_alloc_fail);
	STATS_32(stdout, "IP checksum error", rx->ip_csum_err);
	STATS_32(stdout, "TCP/UDP checksum error", rx->tcp_udp_csum_err);
	STATS_32(stdout, "Rx HW error", rx->reo_error);
	STATS_PRINT("\n");
	STATS_32(stdout, "Buffer added back in freelist", rx->buf_freelist);
	STATS_PRINT("\tSent To Stack:\n");
	STATS_32(stdout, "Vlan tagged stp packets in wifi parse error",
		 rx->vlan_tag_stp_cnt);
}

void print_debug_radio_data_raw(struct debug_pdev_data_raw *raw)
{
	STATS_32(stdout, "DMA map error", raw->dma_map_error);
	STATS_32(stdout, "Packet type not data error",
		 raw->invalid_raw_pkt_datatype);
	STATS_32(stdout, "Frags count overflow error",
		 raw->num_frags_overflow_err);
}

void print_debug_radio_data_tso(struct debug_pdev_data_tso *tso)
{
	STATS_64(stdout, "Packets dropped by host", tso->dropped_host.num);
	STATS_64(stdout, "Bytes dropped by host", tso->dropped_host.bytes);
	STATS_64(stdout, "Packets dropped due to no memory",
		 tso->tso_no_mem_dropped.num);
	STATS_64(stdout, "Bytes dropped due to no memory",
		 tso->tso_no_mem_dropped.bytes);
	STATS_32(stdout, "Packets dropped by target", tso->dropped_target);
}

static const char *capture_status_to_str(enum stats_if_chan_capture_status type)
{
	switch (type) {
	case STATS_IF_CAPTURE_IDLE:
		return "CAPTURE_IDLE";
	case STATS_IF_CAPTURE_BUSY:
		return "CAPTURE_BUSY";
	case STATS_IF_CAPTURE_ACTIVE:
		return "CAPTURE_ACTIVE";
	case STATS_IF_CAPTURE_NO_BUFFER:
		return "CAPTURE_NO_BUFFER";
	default:
		return "INVALID";
	}
}

static const
char *freeze_reason_to_str(enum stats_if_freeze_capture_reason type)
{
	switch (type) {
	case STATS_IF_FREEZE_REASON_TM:
		return "FREEZE_REASON_TM";
	case STATS_IF_FREEZE_REASON_FTM:
		return "FREEZE_REASON_FTM";
	case STATS_IF_FREEZE_REASON_ACK_RESP_TO_TM_FTM:
		return "FREEZE_REASON_ACK_RESP_TO_TM_FTM";
	case STATS_IF_FREEZE_REASON_TA_RA_TYPE_FILTER:
		return "FREEZE_REASON_TA_RA_TYPE_FILTER";
	case STATS_IF_FREEZE_REASON_NDPA_NDP:
		return "FREEZE_REASON_NDPA_NDP";
	case STATS_IF_FREEZE_REASON_ALL_PACKET:
		return "FREEZE_REASON_ALL_PACKET";
	default:
		return "INVALID";
	}
}

void print_debug_radio_data_cfr(struct debug_pdev_data_cfr *cfr)
{
	uint8_t cnt;

	STATS_64(stdout, "bb_captured_channel_cnt",
		 cfr->bb_captured_channel_cnt);
	STATS_64(stdout, "bb_captured_timeout_cnt",
		 cfr->bb_captured_timeout_cnt);
	STATS_64(stdout, "rx_loc_info_valid_cnt",
		 cfr->rx_loc_info_valid_cnt);
	STATS_PRINT("\tChannel capture status:\n");
	for (cnt = 0; cnt < STATS_IF_CAPTURE_MAX; cnt++)
		STATS_PRINT("\t\t%s = %ju\n", capture_status_to_str(cnt),
			    cfr->chan_capture_status[cnt]);
	STATS_PRINT("\tFreeze reason:\n");
	for (cnt = 0; cnt < STATS_IF_FREEZE_REASON_MAX; cnt++)
		STATS_PRINT("\t\t%s = %ju\n", freeze_reason_to_str(cnt),
			    cfr->reason_cnt[cnt]);
}

void print_debug_radio_data_htt(struct debug_pdev_data_htt *htt)
{
}

void print_debug_radio_data_wdi(struct debug_pdev_data_wdi *wdi)
{
	uint8_t i;

	for (i = 0; i < STATS_IF_WDI_EVENT_LAST; i++) {
		if (wdi->wdi_event[i])
			STATS_PRINT("\tWdi msgs received from fw [%u]: [%u]\n",
				    i, wdi->wdi_event[i]);
	}
}

void print_debug_radio_data_mesh(struct debug_pdev_data_mesh *mesh)
{
	STATS_32(stdout, "Mesh Rx Stats Alloc fail", mesh->mesh_mem_alloc);
}

void print_debug_radio_data_txcap(struct debug_pdev_data_txcap *txcap)
{
	uint8_t i, j;

	STATS_32(stdout, "BA not received for delayed_ba",
		 txcap->delayed_ba_not_recev);
	STATS_32(stdout, "Last received ppdu stats", txcap->last_rcv_ppdu);
	STATS_32(stdout, "PPDU stats queue depth",
		 txcap->ppdu_stats_queue_depth);
	STATS_32(stdout, "PPDU stats defer queue depth",
		 txcap->ppdu_stats_defer_queue_depth);
	STATS_32(stdout, "PPDU dropped", txcap->ppdu_dropped);
	STATS_32(stdout, "Pending PPDU dropped", txcap->pend_ppdu_dropped);
	STATS_32(stdout, "PPDU peer flush counter", txcap->ppdu_flush_count);
	STATS_32(stdout, "PPDU MSDU threshold drop",
		 txcap->msdu_threshold_drop);
	STATS_64(stdout, "Peer mismatch", txcap->peer_mismatch);
	STATS_64(stdout, "defer_msdu_q length", txcap->defer_msdu_len);
	STATS_64(stdout, "msdu_comp_q length", txcap->tasklet_msdu_len);
	STATS_64(stdout, "pending_ppdu_q length", txcap->pending_q_len);
	STATS_64(stdout, "tx_ppdu_proc", txcap->tx_ppdu_proc);
	STATS_64(stdout, "ACK BA comes twice", txcap->ack_ba_comes_twice);
	STATS_64(stdout, "PPDU dropped because of incomplete tlv",
		 txcap->ppdu_drop);
	STATS_64(stdout, "PPDU dropped because of wrap around",
		 txcap->ppdu_wrap_drop);
	STATS_PRINT("\tMgmt control enqueue stats:\n");
	for (i = 0; i < STATS_IF_TXCAP_MAX_TYPE; i++) {
		for (j = 0; j < STATS_IF_TXCAP_MAX_SUBTYPE; j++) {
			if (!txcap->ctl_mgmt_q_len[i][j])
				continue;
			STATS_PRINT("\t\tctl_mgmt_q[%u][%u].len = %ju\n", i, j,
				    txcap->ctl_mgmt_q_len[i][j]);
		}
	}
	STATS_PRINT("\tMgmt control retry queue stats:\n");
	for (i = 0; i < STATS_IF_TXCAP_MAX_TYPE; i++) {
		for (j = 0; j < STATS_IF_TXCAP_MAX_SUBTYPE; j++) {
			if (!txcap->retries_ctl_mgmt_q_len[i][j])
				continue;
			STATS_PRINT("\t\tretries_ctl_mgmt_q[%u][%u].len = %ju",
				    i, j, txcap->retries_ctl_mgmt_q_len[i][j]);
			STATS_PRINT("\n");
		}
	}
	STATS_PRINT("\tHTT Frame Type Stats:\n");
	for (i = 0; i < STATS_IF_TX_CAP_HTT_MAX_FTYPE; i++) {
		if (txcap->htt_frame_type[i])
			STATS_PRINT("\t\tsgen htt frame type[%d] = %d\n",
				    i, txcap->htt_frame_type[i]);
	}
	STATS_PRINT("\tPPDU stats counter:\n");
	for (i = 0; i < STATS_IF_PPDU_STATS_MAX_TAG; i++)
		STATS_PRINT("\t\tTag[%u] = %ju\n",
			    i, txcap->ppdu_stats_counter[i]);
}

void print_debug_radio_data_monitor(struct debug_pdev_data_monitor *monitor)
{
	uint32_t i, idx;
	char *str_buf = malloc(MAX_STRING_LEN);

	if (!monitor->status_ppdu_state)
		STATS_UNVLBL(stdout, "PPDU State", "Start");
	else
		STATS_UNVLBL(stdout, "PPDU State", "End");
	STATS_32(stdout, "ul_ofdma_data_rx_ppdu", monitor->data_rx_ppdu);
	for (i = 0; i < STATS_IF_OFDMA_NUM_USERS; i++) {
		STATS_PRINT("\t\tul_ofdma data %u user = %u\n",
			    i, monitor->data_users[i]);
	}
	STATS_32(stdout, "status_ppdu_start_cnt", monitor->status_ppdu_start);
	STATS_32(stdout, "status_ppdu_end_cnt", monitor->status_ppdu_end);
	STATS_32(stdout, "status_ppdu_start_mis_cnt",
		 monitor->status_ppdu_compl);
	STATS_32(stdout, "status_ppdu_start_mis_cnt",
		 monitor->status_ppdu_start_mis);
	STATS_32(stdout, "status_ppdu_end_mis_cnt",
		 monitor->status_ppdu_end_mis);
	STATS_32(stdout, "status_ppdu_done_cnt", monitor->status_ppdu_done);
	STATS_32(stdout, "dest_ppdu_done_cnt", monitor->dest_ppdu_done);
	STATS_32(stdout, "dest_mpdu_done_cnt", monitor->dest_mpdu_done);
	STATS_32(stdout, "dest_mpdu_drop_cnt", monitor->dest_mpdu_drop);
	STATS_32(stdout, "dup_mon_linkdesc_cnt", monitor->dup_mon_linkdesc_cnt);
	STATS_32(stdout, "dup_mon_buf_cnt", monitor->dup_mon_buf_cnt);
	idx = monitor->ppdu_id_hist_idx;
	STATS_PRINT("\tPPDU Id history:\n");
	STATS_PRINT("\tstat_ring_ppdu_ids\t dest_ring_ppdu_ids\n");
	for (i = 0; i < STATS_IF_MAX_PPDU_ID_HIST; i++) {
		idx = (idx + 1) & (STATS_IF_MAX_PPDU_ID_HIST - 1);
		STATS_PRINT("\t\t%*u\t%*u\n", 16,
			    monitor->stat_ring_ppdu_id_hist[idx], 16,
			    monitor->dest_ring_ppdu_id_hist[idx]);
	}
	STATS_32(stdout, "mon_rx_dest_stuck", monitor->mon_rx_dest_stuck);
	STATS_32(stdout, "tlv_tag_status_err_cnt", monitor->tlv_tag_status_err);
	STATS_32(stdout, "mon status DMA not done WAR count",
		 monitor->status_buf_done_war);
	STATS_32(stdout, "mon_rx_buf_replenished",
		 monitor->mon_rx_bufs_replenished_dest);
	STATS_32(stdout, "mon_rx_buf_reaped", monitor->mon_rx_bufs_reaped_dest);
	STATS_32(stdout, "ppdu_id_mismatch", monitor->ppdu_id_mismatch);
	STATS_32(stdout, "mpdu_ppdu_id_match_cnt", monitor->ppdu_id_match);
	STATS_32(stdout, "ppdus dropped frm status ring",
		 monitor->status_ppdu_drop);
	STATS_32(stdout, "ppdus dropped frm dest ring",
		 monitor->dest_ppdu_drop);
	STATS_32(stdout, "mon_link_desc_invalid",
		 monitor->mon_link_desc_invalid);
	STATS_32(stdout, "mon_rx_desc_invalid", monitor->mon_rx_desc_invalid);
	STATS_32(stdout, "mon_nbuf_sanity_err", monitor->mon_nbuf_sanity_err);
	if (str_buf) {
		uint32_t index = 0;

		memset(str_buf, 0, MAX_STRING_LEN);
		for (i = 0; i < STATS_IF_OFDMA_NUM_RU_SIZE; i++) {
			index += snprintf(&str_buf[index],
					  STATS_IF_OFDMA_NUM_RU_SIZE - index,
					  " %u:%u,", i,
					  monitor->data_rx_ru_size[i]);
		}
		STATS_UNVLBL(stdout, "ul_ofdma_data_rx_ru_size", str_buf);
		index = 0;
		memset(str_buf, 0, MAX_STRING_LEN);
		for (i = 0; i < STATS_IF_OFDMA_NUM_RU_SIZE; i++) {
			index += snprintf(&str_buf[index],
					  STATS_IF_OFDMA_NUM_RU_SIZE - index,
					  " %u:%u,", i,
					  monitor->nondata_rx_ru_size[i]);
		}
		STATS_UNVLBL(stdout, "ul_ofdma_nodata_rx_ru_size", str_buf);
	}
}

void print_debug_radio_ctrl_tx(struct debug_pdev_ctrl_tx *tx)
{
	print_basic_radio_ctrl_tx(&tx->b_tx);
	STATS_32(stdout, "Tx no buffer", tx->cs_be_nobuf);
	STATS_32(stdout, "Tx buffer count", tx->cs_tx_buf_count);
	STATS_64(stdout, "Tx HW retries", tx->cs_tx_hw_retries);
	STATS_64(stdout, "Tx HW failures", tx->cs_tx_hw_failures);
	STATS_8(stdout, "AP stats tx cal enable status",
		tx->cs_ap_stats_tx_cal_enable);
}

void print_debug_radio_ctrl_rx(struct debug_pdev_ctrl_rx *rx)
{
	print_basic_radio_ctrl_rx(&rx->b_rx);
	STATS_32(stdout, "Rx RTS success count", rx->cs_rx_rts_success);
	STATS_32(stdout, "Rx clear count", rx->cs_rx_clear_count);
	STATS_32(stdout, "Rx overrun frames", rx->cs_rx_overrun);
	STATS_32(stdout, "Rx phy error", rx->cs_rx_phy_err);
	STATS_32(stdout, "Rx ack error", rx->cs_rx_ack_err);
	STATS_32(stdout, "Rx RTS error", rx->cs_rx_rts_err);
	STATS_32(stdout, "Rx No beacon count", rx->cs_no_beacons);
	STATS_32(stdout, "Rx phy error count", rx->cs_phy_err_count);
	STATS_32(stdout, "Rx FCS error count", rx->cs_fcsbad);
	STATS_32(stdout, "Rx loop limit Start", rx->cs_rx_looplimit_start);
	STATS_32(stdout, "Rx loop limit End", rx->cs_rx_looplimit_end);
}

void print_debug_radio_ctrl_wmi(struct debug_pdev_ctrl_wmi *wmi)
{
	STATS_64(stdout, "Tx Mgmt count", wmi->cs_wmi_tx_mgmt);
	STATS_64(stdout, "Tx Mgmt completions count",
		 wmi->cs_wmi_tx_mgmt_completions);
	STATS_32(stdout, "Tx Mgmt completions error",
		 wmi->cs_wmi_tx_mgmt_completion_err);
}

void print_debug_radio_ctrl_link(struct debug_pdev_ctrl_link *link)
{
	print_basic_radio_ctrl_link(&link->b_link);
}

void print_debug_radio_data(struct stats_obj *radio)
{
	struct debug_pdev_data *data = radio->stats;

	STATS_PRINT("Debug Data STATS for Radio %s (Parent %s)\n",
		    radio->u_id.if_name, radio->pif_name);
	if (data->tx) {
		STATS_PRINT("Tx Stats\n");
		print_debug_radio_data_tx(data->tx);
	}
	if (data->rx) {
		STATS_PRINT("Rx Stats\n");
		print_debug_radio_data_rx(data->rx);
	}
	if (data->me) {
		STATS_PRINT("ME Stats\n");
		print_debug_radio_data_me(data->me);
	}
	if (data->raw) {
		STATS_PRINT("RAW Stats\n");
		print_debug_radio_data_raw(data->raw);
	}
	if (data->tso) {
		STATS_PRINT("TSO Stats\n");
		print_debug_radio_data_tso(data->tso);
	}
	if (data->cfr) {
		STATS_PRINT("CFR Stats\n");
		print_debug_radio_data_cfr(data->cfr);
	}
	if (data->htt) {
		STATS_PRINT("HTT Stats\n");
		print_debug_radio_data_htt(data->htt);
	}
	if (data->wdi) {
		STATS_PRINT("WDI Stats\n");
		print_debug_radio_data_wdi(data->wdi);
	}
	if (data->mesh) {
		STATS_PRINT("MESH Stats\n");
		print_debug_radio_data_mesh(data->mesh);
	}
	if (data->txcap) {
		STATS_PRINT("Tx Capture Stats\n");
		print_debug_radio_data_txcap(data->txcap);
	}
	if (data->monitor) {
		STATS_PRINT("Monitor mode Stats\n");
		print_debug_radio_data_monitor(data->monitor);
	}
}

void print_debug_radio_ctrl(struct stats_obj *radio)
{
	struct debug_pdev_ctrl *ctrl = radio->stats;

	STATS_PRINT("Debug Control STATS for Radio %s (Parent %s)\n",
		    radio->u_id.if_name, radio->pif_name);
	if (ctrl->tx) {
		STATS_PRINT("Tx Stats\n");
		print_debug_radio_ctrl_tx(ctrl->tx);
	}
	if (ctrl->rx) {
		STATS_PRINT("Rx Stats\n");
		print_debug_radio_ctrl_rx(ctrl->rx);
	}
	if (ctrl->wmi) {
		STATS_PRINT("WMI Stats\n");
		print_debug_radio_ctrl_wmi(ctrl->wmi);
	}
	if (ctrl->link) {
		STATS_PRINT("Link Stats\n");
		print_debug_radio_ctrl_link(ctrl->link);
	}
}

void print_debug_ap_data_tx(struct debug_psoc_data_tx *tx)
{
	uint8_t i;

	print_basic_ap_data_tx(&tx->b_tx);
	STATS_64(stdout, "Tx Invalid peer Packets", tx->tx_invalid_peer.num);
	STATS_64(stdout, "Tx Invalid peer Bytes", tx->tx_invalid_peer.bytes);
	STATS_PRINT("\tPackets Queued in HW Ring:\n");
	for (i = 0; i < STATS_IF_MAX_TX_DATA_RINGS; i++)
		STATS_PRINT("\t\tRing ID %u = %u\n", i, tx->tx_hw_enq[i]);
	STATS_PRINT("\tPackets dropped due to HW Ring full = %u %u %u\n",
		    tx->tx_hw_ring_full[0], tx->tx_hw_ring_full[1],
		    tx->tx_hw_ring_full[2]);
	STATS_32(stdout, "Tx Descriptor in use", tx->desc_in_use);
	STATS_32(stdout, "Tx packets dropped due to FW removed",
		 tx->dropped_fw_removed);
	STATS_PRINT("Tx comp wifi internal error = %u : [%u %u %u %u]\n",
		    tx->wifi_internal_error[0], tx->wifi_internal_error[1],
		    tx->wifi_internal_error[2], tx->wifi_internal_error[3],
		    tx->wifi_internal_error[4]);
	STATS_32(stdout, "Tx invalid completion release",
		 tx->invalid_release_source);
	STATS_32(stdout, "Tx comp non wifi internal error",
		 tx->non_wifi_internal_err);
	STATS_32(stdout, "Tx comp loop pkt limit hit",
		 tx->tx_comp_loop_pkt_limit_hit);
	STATS_32(stdout, "Tx comp HP out of sync2", tx->hp_oos2);
	STATS_32(stdout, "Tx comp exception", tx->tx_comp_exception);
}

void print_debug_ap_data_rx(struct debug_psoc_data_rx *rx)
{
	uint8_t ring;
	uint16_t core;
	uint32_t err_code;
	uint64_t total_pkts;

	print_basic_ap_data_rx(&rx->b_rx);
	STATS_PRINT("\tPackets per Core per Ring:\n");
	for (ring = 0; ring < STATS_IF_MAX_RX_DEST_RINGS; ring++) {
		total_pkts = 0;
		STATS_PRINT("\t\tPackets on ring %u:\n", ring);
		for (core = 0; core < rx->rx_packets.num_cpus; core++) {
			if (rx->rx_packets.pkts[core][ring]) {
				STATS_PRINT("\t\t\tPackets on core%u: %ju\n",
					    core,
					    rx->rx_packets.pkts[core][ring]);
				total_pkts += rx->rx_packets.pkts[core][ring];
			}
		}
		STATS_PRINT("\t\t\tTotal Packets on Ring %u: %ju\n",
			    ring, total_pkts);
	}
	STATS_64(stdout, "Rx Invalid Peer Packets", rx->rx_invalid_peer.num);
	STATS_64(stdout, "Rx Invalid Peer Bytes", rx->rx_invalid_peer.bytes);
	STATS_64(stdout, "Rx Invalid SW Peer Id Packets",
		 rx->rx_invalid_peer_id.num);
	STATS_64(stdout, "Rx Invalid SW Peer Id Bytes",
		 rx->rx_invalid_peer_id.bytes);
	STATS_64(stdout, "Rx Invalid length Packets",
		 rx->rx_invalid_pkt_len.num);
	STATS_64(stdout, "Rx Invalid length Bytes",
		 rx->rx_invalid_pkt_len.bytes);
	STATS_32(stdout, "Rx Frag Length error", rx->rx_frag_err_len_error);
	STATS_32(stdout, "Rx Frag No Peer error", rx->rx_frag_err_no_peer);
	STATS_32(stdout, "Rx Frag Wait", rx->rx_frag_wait);
	STATS_32(stdout, "Rx Frag Error", rx->rx_frag_err);
	STATS_32(stdout, "Rx Frag Out of order", rx->rx_frag_oor);
	STATS_32(stdout, "Rx Reap Loop Pkt Limit Hit",
		 rx->reap_loop_pkt_limit_hit);
	STATS_32(stdout, "Rx HP out_of_sync", rx->hp_oos2);
	STATS_32(stdout, "Rx HW near full", rx->near_full);
	STATS_32(stdout, "Rx wait completed msdu break",
		 rx->msdu_scatter_wait_break);
	STATS_32(stdout, "Rx SW2rel route drop", rx->rx_sw_route_drop);
	STATS_32(stdout, "Rx HW2rel route drop", rx->rx_hw_route_drop);
	STATS_32(stdout, "Phy ring access fail: msdus",
		 rx->phy_ring_access_fail);
	STATS_32(stdout, "Phy ring access full fail: msdus",
		 rx->phy_ring_access_full_fail);
	STATS_32(stdout, "Phy Rx HW DUP DESC", rx->phy_rx_hw_dest_dup);
	STATS_32(stdout, "Phy Rx HW REL DUP DESC", rx->phy_wifi_rel_dup);
	STATS_32(stdout, "Phy Rx SW ERR DUP DESC", rx->phy_rx_sw_err_dup);
	STATS_32(stdout, "rbm error: msdus", rx->invalid_rbm);
	STATS_32(stdout, "Invalid vdev", rx->invalid_vdev);
	STATS_32(stdout, "Invalid pdev", rx->invalid_pdev);
	STATS_32(stdout, "pkts delivered no peer", rx->pkt_delivered_no_peer);
	STATS_32(stdout, "defrag peer uninit", rx->defrag_peer_uninit);
	STATS_32(stdout, "sa or da idx invalid", rx->invalid_sa_da_idx);
	STATS_32(stdout, "MSDU Done failures", rx->msdu_done_fail);
	STATS_32(stdout, "Rx DESC invalid magic", rx->rx_desc_invalid_magic);
	STATS_32(stdout, "Rx HW CMD SEND FAIL", rx->rx_hw_cmd_send_fail);
	STATS_32(stdout, "Rx HW CMD SEND Drain", rx->rx_hw_cmd_send_drain);
	STATS_32(stdout, "Rx scatter msdu", rx->scatter_msdu);
	STATS_32(stdout, "RX invalid cookie", rx->invalid_cookie);
	STATS_32(stdout, "RX stale cookie", rx->stale_cookie);
	STATS_32(stdout, "2k jump delba sent", rx->rx_2k_jump_delba_sent);
	STATS_32(stdout, "2k jump msdu to stack", rx->rx_2k_jump_to_stack);
	STATS_32(stdout, "2k jump msdu drop", rx->rx_2k_jump_drop);
	STATS_32(stdout, "Rx HW Error MSDU buffer recved",
		 rx->rx_hw_err_msdu_buf_rcved);
	STATS_32(stdout, "Rx HW Error MSDU buffer invalid cookie",
		 rx->rx_hw_err_msdu_buf_invalid_cookie);
	STATS_32(stdout, "Rx HW oor msdu drop", rx->rx_hw_err_oor_drop);
	STATS_32(stdout, "Rx HW oor msdu to stack", rx->rx_hw_err_oor_to_stack);
	STATS_32(stdout, "Rx HW Out of Order sg count",
		 rx->rx_hw_err_oor_sg_count);
	STATS_32(stdout, "Rx MSDU count mismatch", rx->msdu_count_mismatch);
	STATS_32(stdout, "Rx stale link desc cookie", rx->invalid_link_cookie);
	STATS_32(stdout, "Rx nbuf sanity fails", rx->nbuf_sanity_fail);
	STATS_32(stdout, "Rx refill duplicate link desc",
		 rx->dup_refill_link_desc);
	STATS_32(stdout, "Rx err msdu continuation err",
		 rx->msdu_continuation_err);
	STATS_32(stdout, "ssn update count", rx->ssn_update_count);
	STATS_32(stdout, "bar handle update fail count",
		 rx->bar_handle_fail_count);
	STATS_32(stdout, "Intra-bss EAPOL drops", rx->intrabss_eapol_drop);
	STATS_32(stdout, "PN-in-Dest error frame pn-check fail",
		 rx->pn_in_dest_check_fail);
	STATS_32(stdout, "MSDU Length Error", rx->msdu_len_err);
	STATS_32(stdout, "Rx Flush count", rx->rx_flush_count);
	for (err_code = 0; err_code < STATS_IF_WIFI_ERR_MAX; err_code++) {
		if (rx->rx_sw_error[err_code])
			STATS_PRINT("\tRx SW error number (%u): %u msdus\n",
				    err_code, rx->rx_sw_error[err_code]);
	}
	for (err_code = 0; err_code < STATS_IF_RX_ERR_MAX; err_code++) {
		if (rx->rx_hw_error[err_code])
			STATS_PRINT("\tRx HW error number (%u): %u msdus\n",
				    err_code, rx->rx_hw_error[err_code]);
	}
	for (err_code = 0; err_code < STATS_IF_MAX_RX_DEST_RINGS; err_code++) {
		if (rx->phy_rx_hw_error[err_code])
			STATS_PRINT("\tPhy Rx SW error number (%u): %u msdus\n",
				    err_code, rx->phy_rx_hw_error[err_code]);
	}
}

void print_debug_ap_data_ast(struct debug_psoc_data_ast *ast)
{
	STATS_32(stdout, "Entries Added", ast->ast_added);
	STATS_32(stdout, "Entries Deleted", ast->ast_deleted);
	STATS_32(stdout, "Entries Agedout", ast->ast_aged_out);
	STATS_32(stdout, "Entries MAP ERR", ast->ast_map_err);
	STATS_32(stdout, "Entries Mismatch ERR", ast->ast_mismatch);
	STATS_32(stdout, "MEC Added", ast->mec_added);
	STATS_32(stdout, "MEC Deleted", ast->mec_deleted);
}

void print_debug_ap_data(struct stats_obj *ap)
{
	struct debug_psoc_data *data = ap->stats;

	STATS_PRINT("Debug Data STATS for AP %s\n", ap->u_id.if_name);
	if (data->tx) {
		STATS_PRINT("Tx Stats\n");
		print_debug_ap_data_tx(data->tx);
	}
	if (data->rx) {
		STATS_PRINT("Rx Stats\n");
		print_debug_ap_data_rx(data->rx);
	}
	if (data->ast) {
		STATS_PRINT("AST Stats\n");
		print_debug_ap_data_ast(data->ast);
	}
}

void print_debug_stats(struct stats_obj *obj)
{
	switch (obj->obj_type) {
	case STATS_OBJ_STA:
		if (obj->type == STATS_TYPE_DATA)
			print_debug_sta_data(obj);
		else
			print_debug_sta_ctrl(obj);
		break;
	case STATS_OBJ_VAP:
		if (obj->type == STATS_TYPE_DATA)
			print_debug_vap_data(obj);
		else
			print_debug_vap_ctrl(obj);
		break;
	case STATS_OBJ_RADIO:
		if (obj->type == STATS_TYPE_DATA)
			print_debug_radio_data(obj);
		else
			print_debug_radio_ctrl(obj);
		break;
	case STATS_OBJ_AP:
		if (obj->type == STATS_TYPE_DATA)
			print_debug_ap_data(obj);
		break;
	default:
		STATS_ERR("Invalid object option\n");
	}
}
#endif /* WLAN_DEBUG_TELEMETRY */

void print_response(struct reply_buffer *reply)
{
	struct stats_obj *obj = reply->obj_head;

	while (obj) {
		switch (obj->lvl) {
		case STATS_LVL_BASIC:
			print_basic_stats(obj);
			break;
#if WLAN_ADVANCE_TELEMETRY
		case STATS_LVL_ADVANCE:
			print_advance_stats(obj);
			break;
#endif /* WLAN_ADVANCE_TELEMETRY */
#if WLAN_DEBUG_TELEMETRY
		case STATS_LVL_DEBUG:
			print_debug_stats(obj);
			break;
#endif /* WLAN_DEBUG_TELEMETRY */
		default:
			STATS_ERR("Invalid Level!\n");
		}
		obj = obj->next;
	}
}

static int is_radio_ifname_valid(const char *ifname)
{
	int i;

	assert(ifname);
	/*
	 * At this step, we only validate if the string makes sense.
	 * If the interface doesn't actually exist, we'll throw an
	 * error at the place where we make system calls to try and
	 * use the interface.
	 * Reduces the no. of ioctl calls.
	 */
	if (strncmp(ifname, "wifi", 4) != 0)
		return 0;
	if (!ifname[4] || !isdigit(ifname[4]))
		return 0;
	/*
	 * We don't make any assumptions on max no. of radio interfaces,
	 * at this step.
	 */
	for (i = 5; i < IFNAMSIZ; i++) {
		if (!ifname[i])
			break;
		if (!isdigit(ifname[i]))
			return 0;
	}

	return 1;
}

static int is_vap_ifname_valid(const char *ifname)
{
	char path[100];
	FILE *fp;
	ssize_t bufsize = sizeof(path);

	assert(ifname);

	if ((strlcpy(path, PATH_SYSNET_DEV, bufsize) >= bufsize) ||
	    (strlcat(path, ifname, bufsize) >= bufsize) ||
	    (strlcat(path, "/parent", bufsize) >= bufsize)) {
		STATS_ERR("Error creating pathname\n");
		return -EINVAL;
	}
	fp = fopen(path, "r");
	if (fp) {
		fclose(fp);
		return 1;
	}

	return 0;
}

static void free_interface_list(struct interface_list *if_list)
{
	u_int8_t inx = 0;

	for (inx = 0; inx < MAX_RADIO_NUM; inx++) {
		if (if_list->r_names[inx])
			free(if_list->r_names[inx]);
		if_list->r_names[inx] = NULL;
	}
	if_list->r_count = 0;
	for (inx = 0; inx < MAX_VAP_NUM; inx++) {
		if (if_list->v_names[inx])
			free(if_list->v_names[inx]);
		if_list->v_names[inx] = NULL;
	}
	if_list->v_count = 0;
}

static int fetch_all_interfaces(struct interface_list *if_list)
{
	char temp_name[IFNAME_LEN] = {'\0'};
	DIR *dir = NULL;
	u_int8_t rinx = 0;
	u_int8_t vinx = 0;

	dir = opendir(PATH_SYSNET_DEV);
	if (!dir) {
		perror(PATH_SYSNET_DEV);
		return -ENOENT;
	}
	while (1) {
		struct dirent *entry;
		char *d_name;

		entry = readdir(dir);
		if (!entry)
			break;
		d_name = entry->d_name;
		if (entry->d_type & (DT_DIR | DT_LNK)) {
			if (strlcpy(temp_name, d_name, IFNAME_LEN) >=
				    IFNAME_LEN) {
				STATS_ERR("Unable to fetch interface name\n");
				closedir(dir);
				return -EIO;
			}
		} else {
			continue;
		}
		if (is_radio_ifname_valid(temp_name)) {
			if (rinx >= MAX_RADIO_NUM) {
				STATS_WARN("Radio Interfaces exceeded limit\n");
				continue;
			}
			if_list->r_names[rinx] = (char *)malloc(IFNAME_LEN);
			if (!if_list->r_names[rinx]) {
				STATS_ERR("Unable to Allocate Memory!\n");
				closedir(dir);
				return -ENOMEM;
			}
			strlcpy(if_list->r_names[rinx], temp_name, IFNAME_LEN);
			rinx++;
		} else if (is_vap_ifname_valid(temp_name)) {
			if (vinx >= MAX_VAP_NUM) {
				STATS_WARN("Vap Interfaces exceeded limit\n");
				continue;
			}
			if_list->v_names[vinx] = (char *)malloc(IFNAME_LEN);
			if (!if_list->v_names[vinx]) {
				STATS_ERR("Unable to Allocate Memory!\n");
				closedir(dir);
				return -ENOMEM;
			}
			strlcpy(if_list->v_names[vinx], temp_name, IFNAME_LEN);
			vinx++;
		}
	}

	closedir(dir);
	if_list->r_count = rinx;
	if_list->v_count = vinx;

	return 0;
}

int main(int argc, char *argv[])
{
	struct stats_command cmd;
	struct reply_buffer *reply = NULL;
	enum stats_level_e level_temp = STATS_LVL_BASIC;
	enum stats_object_e obj_temp = STATS_OBJ_AP;
	enum stats_type_e type_temp = STATS_TYPE_DATA;
	u_int64_t feat_temp = STATS_FEAT_FLG_ALL;
	int ret = 0;
	int long_index = 0;
	u_int8_t is_obj_set = 0;
	u_int8_t is_type_set = 0;
	u_int8_t is_feat_set = 0;
	u_int8_t is_level_set = 0;
	u_int8_t is_ifname_set = 0;
	u_int8_t is_stamacaddr_set = 0;
	u_int8_t is_serviceid_set = 0;
	u_int8_t is_option_selected = 0;
	u_int8_t inx = 0;
	bool recursion_temp = false;
	char feat_flags[128] = {0};
	char ifname_temp[IFNAME_LEN] = {0};
	char stamacaddr_temp[USER_MAC_ADDR_LEN] = {0};
	struct ether_addr *ret_eth_addr = NULL;
	struct interface_list if_list;
	u_int8_t servid_temp = 0;

	memset(&cmd, 0, sizeof(struct stats_command));
	memset(&if_list, 0, sizeof(struct interface_list));

	ret = getopt_long(argc, argv, opt_string, long_opts, &long_index);
	while (ret != -1) {
		switch (ret) {
		case 'B':
			if (is_level_set) {
				STATS_ERR("Multiple Stats Level Arguments\n");
				display_help();
				return -EINVAL;
			}
			level_temp = STATS_LVL_BASIC;
			is_level_set = 1;
			is_option_selected = 1;
			break;
		case 'A':
			if (is_level_set) {
				STATS_ERR("Multiple Stats Level Arguments\n");
				display_help();
				return -EINVAL;
			}
			level_temp = STATS_LVL_ADVANCE;
			is_level_set = 1;
			is_option_selected = 1;
			break;
		case 'D':
			if (is_level_set) {
				STATS_ERR("Multiple Stats Level Arguments\n");
				display_help();
				return -EINVAL;
			}
			level_temp = STATS_LVL_DEBUG;
			is_level_set = 1;
			is_option_selected = 1;
			break;
		case 'i':
			if (is_ifname_set) {
				STATS_ERR("Multiple Stats ifname Arguments\n");
				display_help();
				return -EINVAL;
			}
			memset(ifname_temp, '\0', sizeof(ifname_temp));
			if (strlcpy(ifname_temp, optarg, sizeof(ifname_temp)) >=
			    sizeof(ifname_temp)) {
				STATS_ERR("Error creating ifname\n");
				return -EINVAL;
			}
			is_ifname_set  = 1;
			is_option_selected = 1;
			break;
		case 's':
			if (is_obj_set) {
				STATS_ERR("Multiple Stats Object Arguments\n");
				display_help();
				return -EINVAL;
			}
			obj_temp = STATS_OBJ_STA;
			is_obj_set = 1;
			is_option_selected = 1;
			break;
		case 'v':
			if (is_obj_set) {
				STATS_ERR("Multiple Stats Object Arguments\n");
				display_help();
				return -EINVAL;
			}
			obj_temp = STATS_OBJ_VAP;
			is_obj_set = 1;
			is_option_selected = 1;
			break;
		case 'r':
			if (is_obj_set) {
				STATS_ERR("Multiple Stats Object Arguments\n");
				display_help();
				return -EINVAL;
			}
			obj_temp = STATS_OBJ_RADIO;
			is_obj_set = 1;
			is_option_selected = 1;
			break;
		case 'a':
			if (is_obj_set) {
				STATS_ERR("Multiple Stats Object Arguments\n");
				display_help();
				return -EINVAL;
			}
			obj_temp = STATS_OBJ_AP;
			is_obj_set = 1;
			is_option_selected = 1;
			break;
		case 'm':
			if (is_stamacaddr_set) {
				STATS_ERR("Multiple STA MAC Arguments\n");
				display_help();
				return -EINVAL;
			}
			memset(stamacaddr_temp, '\0', sizeof(stamacaddr_temp));
			if (strlcpy(stamacaddr_temp, optarg,
				    sizeof(stamacaddr_temp)) >=
				    sizeof(stamacaddr_temp)) {
				STATS_ERR("Error copying macaddr\n");
				return -EINVAL;
			}
			is_stamacaddr_set  = 1;
			is_option_selected = 1;
			break;
		case 't':
			if (is_serviceid_set) {
				STATS_ERR("Multiple Serviceid Arguments\n");
				display_help();
				return -EINVAL;
			}
			servid_temp = atoi(optarg);
			is_serviceid_set = 1;
			is_option_selected = 1;
			break;
		case 'f':
			if (is_feat_set) {
				STATS_ERR("Multiple Feature flag Arguments\n");
				display_help();
				return -EINVAL;
			}
			memset(feat_flags, '\0', sizeof(feat_flags));
			if (strlcpy(feat_flags, optarg, sizeof(feat_flags)) >=
			    sizeof(feat_flags)) {
				STATS_ERR("Error copying feature flags\n");
				return -EINVAL;
			}
			is_feat_set = 1;
			is_option_selected = 1;
			break;
		case 'c':
			if (is_type_set) {
				STATS_ERR("Multiple Type Arguments\n");
				display_help();
				return -EINVAL;
			}
			type_temp = STATS_TYPE_CTRL;
			is_type_set = 1;
			is_option_selected = 1;
			break;
		case 'd':
			if (is_type_set) {
				STATS_ERR("Multiple Type Arguments\n");
				display_help();
				return -EINVAL;
			}
			type_temp = STATS_TYPE_DATA;
			is_type_set = 1;
			is_option_selected = 1;
			break;
		case 'h':
		case '?':
			display_help();
			is_option_selected = 1;
			return 0;
		case 'R':
			recursion_temp = true;
			break;
		default:
			STATS_ERR("Unrecognized option\n");
			display_help();
			is_option_selected = 1;
			return -EINVAL;
		}
		ret = getopt_long(argc, argv, opt_string,
				  long_opts, &long_index);
	}

	if (!is_option_selected)
		STATS_WARN("No valid option selected\n"
			   "Will display only default stats\n");

	if (feat_flags[0])
		feat_temp = libstats_get_feature_flag(feat_flags);
	if (!feat_temp)
		return -EINVAL;

	cmd.lvl = level_temp;
	cmd.obj = obj_temp;
	cmd.type = type_temp;
	cmd.feat_flag = feat_temp;
	cmd.recursive = recursion_temp;
	cmd.serviceid = servid_temp;

	if (ifname_temp[0])
		strlcpy(cmd.if_name, ifname_temp, IFNAME_LEN);
	ret = fetch_all_interfaces(&if_list);
	if (ret < 0) {
		STATS_ERR("Unbale to fetch Interfaces!\n");
		free_interface_list(&if_list);
		return ret;
	}

	if (stamacaddr_temp[0] != '\0') {
		ret_eth_addr = ether_aton_r(stamacaddr_temp, &cmd.sta_mac);
		if (!ret_eth_addr) {
			STATS_ERR("STA MAC address not valid.\n");
			return -EINVAL;
		}
	}

	reply = (struct reply_buffer *)malloc(sizeof(struct reply_buffer));
	if (!reply) {
		STATS_ERR("Failed to allocate memory\n");
		return -ENOMEM;
	}
	memset(reply, 0, sizeof(struct reply_buffer));
	cmd.reply = reply;

	if ((cmd.obj == STATS_OBJ_AP) && !cmd.if_name[0]) {
		for (inx = 0; inx < if_list.r_count; inx++) {
			strlcpy(cmd.if_name, if_list.r_names[inx], IFNAME_LEN);
			ret = libstats_request_handle(&cmd);
			if (ret < 0)
				break;
		}
	} else {
		ret = libstats_request_handle(&cmd);
	}

	/* Print Output */
	if (!ret)
		print_response(cmd.reply);

	/* Cleanup */
	free_interface_list(&if_list);
	libstats_free_reply_buffer(&cmd);

	return 0;
}
