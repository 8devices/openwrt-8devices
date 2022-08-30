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

#include "lite_mon.h"

/*
 * usage: Function to display usage of lite_mon app
 * return void
 */
static void usage(void)
{
	printf("nandhaki %s[%d]\n",  __func__, __LINE__);
	lite_mon_printf(LITE_MON_TRACE_INFO,
			"lite_mon wifiX <option> <arguments>");
	lite_mon_printf(LITE_MON_TRACE_INFO,
			"--direction: Specify the direction <rx/tx>");
	lite_mon_printf(LITE_MON_TRACE_INFO,
			"--level: Specify the level <MSDU/MPDU/PPDU>");
	lite_mon_printf(LITE_MON_TRACE_INFO,
			"--disable: Disable lite mon filter");
	lite_mon_printf(LITE_MON_TRACE_INFO,
			"--filter_fp: set filter for filter pass");
	lite_mon_printf(LITE_MON_TRACE_INFO,
			"--filter_mo: set filter for monitor other");
	lite_mon_printf(LITE_MON_TRACE_INFO,
			"--filter_md: set filter for monitor direct");
	lite_mon_printf(LITE_MON_TRACE_INFO,
			"\tfilter format: 0xTSSSS, T:type, SSSS:Subtype mask");
	lite_mon_printf(LITE_MON_TRACE_INFO,
			"--mgmt_len: Length of management frame to be filtered");
	lite_mon_printf(LITE_MON_TRACE_INFO,
			"--ctrl_len: Length of control frame to be filtered");
	lite_mon_printf(LITE_MON_TRACE_INFO,
			"--data_len: Length of data frame to be filtered");
	lite_mon_printf(LITE_MON_TRACE_INFO,
			"\tlength options: 0x40, 0x80, 0x100");
	lite_mon_printf(LITE_MON_TRACE_INFO,
			"--metadata: Enable metadata <0x1/0x2>");
	lite_mon_printf(LITE_MON_TRACE_INFO,
			"--output: output vap to deliver <athX>");
	lite_mon_printf(LITE_MON_TRACE_INFO,
			"--show_filter: Show the provided filter option");
	lite_mon_printf(LITE_MON_TRACE_INFO,
			"--peer_add: Add peer into peer filter list <mac_addr1,mac_addr2,mac_addr3,....>");
	lite_mon_printf(LITE_MON_TRACE_INFO,
			"--peer_remove: remove peer frm peer filer list <mac_addr1,mac_addr2,mac_addr3,....>");
	lite_mon_printf(LITE_MON_TRACE_INFO,
			"--peer_list: List the peers added into peer filtering list");
	lite_mon_printf(LITE_MON_TRACE_INFO,
			"--type: type of peer to be added/removed/displayed <assoc/non-assoc>");
	lite_mon_printf(LITE_MON_TRACE_INFO,
			"--vap: ifname to which the peer is to be added");
	lite_mon_printf(LITE_MON_TRACE_INFO,
			"--debug: Debug option for lite_mon <0/1/2/3/4>");
	lite_mon_printf(LITE_MON_TRACE_INFO,
			"--help: Display help menu");

	exit(0);
}

/*
 * lite_mon_fill_fil_value: Fill user given value into filter structure
 * @filter_config: filter config structre to be filled
 * @input_filter: filter that was given as input by user
 * @filter_mode: filter mode, FP, MO or MD
 * return void
 */
static void lite_mon_fill_fil_value(struct lite_mon_filter_config *filter_config,
				    uint32_t input_filter,
				    int filter_mode)
{
	int type = LITE_MON_GET_FILTER_TYPE(input_filter);
	uint16_t filter = LITE_MON_GET_FILTER_VALUE(input_filter);

	lite_mon_printf(LITE_MON_TRACE_DEBUG, "Filter type %d filter %02X",
			type, filter);
	switch (type) {
	case LITE_MON_TYPE_MGMT:
		filter_config->mgmt_filter[filter_mode] = filter;
		break;
	case LITE_MON_TYPE_CTRL:
		filter_config->ctrl_filter[filter_mode] = filter;
		break;
	case LITE_MON_TYPE_DATA:
		filter_config->data_filter[filter_mode] = filter;
		break;
	case LITE_MON_TYPE_ALL:
		if (filter != LITE_MON_FILTER_ALL) {
			lite_mon_printf(LITE_MON_TRACE_ERROR,
					"Wrong filter type %d filter %02X",
					type, filter);
			exit(0);
		}
		filter_config->mgmt_filter[filter_mode] = LITE_MON_FILTER_ALL;
		filter_config->ctrl_filter[filter_mode] = LITE_MON_FILTER_ALL;
		filter_config->data_filter[filter_mode] = LITE_MON_FILTER_ALL;
		break;
	default:
		lite_mon_printf(LITE_MON_TRACE_ERROR,
				"Wrong filter type %d filter %02X",
				type, filter);
		exit(0);
	}
}

/*
 * lite_mon_set_cmdtype: type of the command passed to driver from application
 * @mon_config: mon_config structre to be filled
 * @cmd_bitmask: cmd_bitmask that is given by user to be passed to the driver.
 * return void
 */
static void lite_mon_set_cmdtype(struct lite_mon_config *mon_config,
				 int cmd_bitmask)
{
	int count = 0;
	int i;

	for (i = 0; i < LITE_MON_FILTER_MAX; i++) {
		if ((cmd_bitmask >> i) & 1)
			count++;
	}

	lite_mon_printf(LITE_MON_TRACE_DEBUG, "cmd_bitmask %02X count %d",
			cmd_bitmask, count);
	if (count == 1) {
		if (cmd_bitmask & LITE_MON_SET_FILTER_MASK) {
			mon_config->cmdtype = LITE_MON_SET_FILTER;
		} else if (cmd_bitmask & LITE_MON_SET_PEER_MASK) {
			mon_config->cmdtype = LITE_MON_SET_PEER;
		} else if (cmd_bitmask & LITE_MON_GET_FILTER_MASK) {
			mon_config->cmdtype = LITE_MON_GET_FILTER;
		} else if (cmd_bitmask & LITE_MON_GET_PEER_MASK) {
			mon_config->cmdtype = LITE_MON_GET_PEER;
		} else {
			lite_mon_printf(LITE_MON_TRACE_ERROR,
					"Wrong command given. No function can be executed");
			usage();
		}
	} else if (count >= 1) {
		lite_mon_printf(LITE_MON_TRACE_ERROR,
				"Options corresponding to more than one command given");
		usage();
	} else {
		lite_mon_printf(LITE_MON_TRACE_ERROR,
				"The options given correspond to no valid command");
		usage();
	}
}

/*
 * lite_mon_set_default_len: set default len in mon_config structre
 * @mon_config: mon_config structre filled by user
 * return void
 */
static void lite_mon_set_default_len(struct lite_mon_config *mon_config)
{
	int i;
	bool mgmt_filter_set = false;
	bool ctrl_filter_set = false;
	bool data_filter_set = false;

	for (i = 0; i < LITE_MON_MODE_MAX; i++)	{
		if (mon_config->data.filter_config.mgmt_filter[i])
			mgmt_filter_set = true;
		if (mon_config->data.filter_config.ctrl_filter[i])
			ctrl_filter_set = true;
		if (mon_config->data.filter_config.data_filter[i])
			data_filter_set = true;
	}

	if (mgmt_filter_set &&
	    !mon_config->data.filter_config.len[LITE_MON_TYPE_MGMT])
		mon_config->data.filter_config.len[LITE_MON_TYPE_MGMT] =
							LITE_MON_LEN_ALL;

	if (ctrl_filter_set &&
	    !mon_config->data.filter_config.len[LITE_MON_TYPE_CTRL])
		mon_config->data.filter_config.len[LITE_MON_TYPE_CTRL] =
							LITE_MON_LEN_ALL;

	if (data_filter_set &&
	    !mon_config->data.filter_config.len[LITE_MON_TYPE_DATA])
		mon_config->data.filter_config.len[LITE_MON_TYPE_DATA] =
							LITE_MON_LEN_ALL;
}

/*
 * lite_mon_set_default_filter: set default filter in mon_config structre
 * @mon_config: mon_config structre filled by user
 * return void
 */
static void lite_mon_set_default_filter(struct lite_mon_config *mon_config)
{
	int i;
	bool mgmt_filter_set = false;
	bool ctrl_filter_set = false;
	bool data_filter_set = false;
	struct lite_mon_filter_config *filter_config =
					&mon_config->data.filter_config;

	for (i = 0; i < LITE_MON_MODE_MAX; i++)	{
		if (mon_config->data.filter_config.mgmt_filter[i])
			mgmt_filter_set = true;
		if (mon_config->data.filter_config.ctrl_filter[i])
			ctrl_filter_set = true;
		if (mon_config->data.filter_config.data_filter[i])
			data_filter_set = true;
	}

	if (mgmt_filter_set || ctrl_filter_set || data_filter_set)
		return;

	switch (mon_config->direction) {
	case LITE_MON_DIRECTION_RX:
		lite_mon_printf(LITE_MON_TRACE_DEBUG,
				"Setting default filters for Rx");
		filter_config->mgmt_filter[LITE_MON_MODE_FILTER_FP] =
							LITE_MON_FILTER_ALL;
		filter_config->data_filter[LITE_MON_MODE_FILTER_FP] =
							LITE_MON_FILTER_ALL;
		filter_config->ctrl_filter[LITE_MON_MODE_FILTER_FP] =
							LITE_MON_FILTER_ALL;
		filter_config->mgmt_filter[LITE_MON_MODE_FILTER_MO] =
							LITE_MON_FILTER_ALL;
		filter_config->data_filter[LITE_MON_MODE_FILTER_MO] =
							LITE_MON_FILTER_ALL;
		filter_config->ctrl_filter[LITE_MON_MODE_FILTER_MO] =
							LITE_MON_FILTER_ALL;
		break;
	case LITE_MON_DIRECTION_TX:
		lite_mon_printf(LITE_MON_TRACE_DEBUG,
				"Setting default filters for Tx");
		filter_config->mgmt_filter[LITE_MON_MODE_FILTER_FP] =
							LITE_MON_FILTER_ALL;
		filter_config->data_filter[LITE_MON_MODE_FILTER_FP] =
							LITE_MON_FILTER_ALL;
		filter_config->ctrl_filter[LITE_MON_MODE_FILTER_FP] =
							LITE_MON_FILTER_ALL;
		break;
	default:
		lite_mon_printf(LITE_MON_TRACE_ERROR, "Invalid Direction %d",
				mon_config->direction);
		break;
	}
}

/*
 * lite_mon_set_defaults: set default value of mon_config structre
 * @mon_config: mon_config structre to be filled
 * return void
 */
static void lite_mon_set_defaults(struct lite_mon_config *mon_config)
{
	lite_mon_set_default_filter(mon_config);
	lite_mon_set_default_len(mon_config);
}

/*
 * lite_mon_sanitize_level: sanitize the level given by user.
 * @level: filter level given by user.
 * return void
 */
static void lite_mon_sanitize_level(uint8_t level)
{
	lite_mon_printf(LITE_MON_TRACE_DEBUG, "Level %d", level);
	switch (level) {
	case LITE_MON_LEVEL_MSDU:
	case LITE_MON_LEVEL_MPDU:
	case LITE_MON_LEVEL_PPDU:
		break;
	default:
		lite_mon_printf(LITE_MON_TRACE_ERROR, "Invalid level option %d",
				level);
		exit(0);
	}
}

/*
 * lite_mon_sanitize_filter: sanitize the filter given by user
 * @filter: filter config given by user
 * @direction: direction value given by user
 * return void
 */
static void lite_mon_sanitize_filter(uint16_t *filter, uint8_t direction,
				     uint32_t filter_invalid)
{
	int i;

	lite_mon_printf(LITE_MON_TRACE_DEBUG, "filter FP %02X MD %02X MO %02X",
			filter[0], filter[1], filter[2]);
	for (i = 0; i < LITE_MON_MODE_MAX; i++) {
		if (filter[i] == LITE_MON_FILTER_ALL)
			continue;
		if (filter[i] & filter_invalid) {
			lite_mon_printf(LITE_MON_TRACE_ERROR,
					"filter invalid i %d filter %02X",
					i, filter[i]);
			exit(0);
		}
		if (direction == LITE_MON_DIRECTION_TX &&
		    i != LITE_MON_MODE_FILTER_FP && filter[i]) {
			lite_mon_printf(LITE_MON_TRACE_ERROR,
					"Only FP filter is valid for Tx i %d filter %02X",
					i, filter[i]);
			exit(0);
		}
	}
}

/*
 * lite_mon_sanitize_len: sanitize the length given by user
 * @len: filter length given by user
 * return void
 */
static void lite_mon_sanitize_len(uint16_t *len)
{
	int i;

	lite_mon_printf(LITE_MON_TRACE_DEBUG,
			"mgmt_len %02X ctrl_len %02X data_len %02X",
			len[0], len[1], len[2]);
	for (i = 0; i < LITE_MON_TYPE_MAX; i++) {
		switch (len[i]) {
		case LITE_MON_LEN_0:
		case LITE_MON_LEN_1:
		case LITE_MON_LEN_2:
		case LITE_MON_LEN_3:
		case LITE_MON_LEN_ALL:
			break;
		default:
			lite_mon_printf(LITE_MON_TRACE_ERROR,
					"Invalid length 0x%X",
					len[i]);
			exit(0);
		}
	}
}

/*
 * lite_mon_sanitize_len: sanitize the metadata given by user
 * @len: metadata given by user
 * return void
 */
static void lite_mon_sanitize_metadata(uint16_t metadata)
{
	lite_mon_printf(LITE_MON_TRACE_DEBUG, "metadata %02X", metadata);
	if (metadata & LITE_MON_METADATA_INVALID) {
		lite_mon_printf(LITE_MON_TRACE_ERROR,
				"Invalid metadata option %02X",
				metadata);
		exit(0);
	}
}

/*
 * lite_mon_sanitize_direction: sanitize the direction given by user.
 * @direction: filter direction given by user.
 * return void
 */
static void lite_mon_sanitize_direction(uint8_t direction)
{
	lite_mon_printf(LITE_MON_TRACE_DEBUG, "Direction %d", direction);
	switch (direction) {
	case LITE_MON_DIRECTION_RX:
	case LITE_MON_DIRECTION_TX:
		break;
	default:
		lite_mon_printf(LITE_MON_TRACE_ERROR,
				"Invalid direction option %d",
				direction);
		exit(0);
	}
}

/*
 * lite_mon_sanitize_debug: sanitize the debug given by user.
 * @debug: filter debug given by user.
 * return void
 */
static void lite_mon_sanitize_debug(uint8_t debug)
{
	lite_mon_printf(LITE_MON_TRACE_DEBUG, "Debug %d", debug);
	switch (debug) {
	case LITE_MON_TRACE_NONE:
	case LITE_MON_TRACE_ERROR:
	case LITE_MON_TRACE_INFO:
	case LITE_MON_TRACE_DEBUG:
	case LITE_MON_TRACE_MAX:
		break;
	default:
		lite_mon_printf(LITE_MON_TRACE_ERROR, "Invalid debug option %d",
				debug);
		exit(0);
	}
}

/*
 * lite_mon_sanitize_type: sanitize the type given by user.
 * @type: filter type given by user.
 * return void
 */
static void lite_mon_sanitize_type(uint8_t type)
{
	lite_mon_printf(LITE_MON_TRACE_DEBUG, "type %d", type);
	switch (type) {
	case LITE_MON_PEER_ASSOCIATED:
	case LITE_MON_PEER_NON_ASSOCIATED:
		break;
	default:
		lite_mon_printf(LITE_MON_TRACE_ERROR, "Invalid type option %d",
				type);
		exit(0);
	}
}

/*
 * lite_mon_sanitize_count: sanitize the count given by user.
 * @count: filter count given by user.
 * return void
 */
static void lite_mon_sanitize_count(uint8_t count)
{
	lite_mon_printf(LITE_MON_TRACE_DEBUG, "count %d", count);
	if (count == 0) {
		lite_mon_printf(LITE_MON_TRACE_ERROR, "Invalid count option %d",
				count);
		exit(0);
	}
}

/*
 * lite_mon_sanitize_interface_name: sanitize the vap name given by user.
 * @ifname: Interface name given by user.
 * return void
 */
static void lite_mon_sanitize_interface_name(char *ifname)
{
	lite_mon_printf(LITE_MON_TRACE_DEBUG, "interface name %s", ifname);
	if (strncmp(ifname, "ath", VAP_IFACE_PREFIX_LEN)) {
		lite_mon_printf(LITE_MON_TRACE_ERROR,
				"Vap name missing or wrong");
		exit(0);
	}
}

/*
 * lite_mon_set_filter_sanity: check sanity of all fields for set filter command
 * @mon_config: mon_config structre to be filled
 * return void
 */
static void lite_mon_set_filter_sanity(struct lite_mon_config *mon_config)
{
	lite_mon_sanitize_direction(mon_config->direction);
	lite_mon_sanitize_debug(mon_config->debug);
	lite_mon_sanitize_level(mon_config->data.filter_config.level);
	lite_mon_sanitize_filter(mon_config->data.filter_config.mgmt_filter,
				 mon_config->direction,
				 LITE_MON_MGMT_FILTER_INVALID);
	lite_mon_sanitize_filter(mon_config->data.filter_config.ctrl_filter,
				 mon_config->direction,
				 LITE_MON_CTRL_FILTER_INVALID);
	lite_mon_sanitize_filter(mon_config->data.filter_config.data_filter,
				 mon_config->direction,
				 LITE_MON_DATA_FILTER_INVALID);
	lite_mon_sanitize_len(mon_config->data.filter_config.len);
	lite_mon_sanitize_metadata(mon_config->data.filter_config.metadata);
}

/*
 * lite_mon_get_filter_sanity: check sanity of all fields for get filter command
 * @mon_config: mon_config structre to be filled
 * return void
 */
static void lite_mon_get_filter_sanity(struct lite_mon_config *mon_config)
{
	lite_mon_sanitize_direction(mon_config->direction);
	lite_mon_sanitize_debug(mon_config->debug);
}

/*
 * lite_mon_set_peer_sanity: check sanity of all fields for set peer command
 * @mon_config: mon_config structre to be filled
 * return void
 */
static void lite_mon_set_peer_sanity(struct lite_mon_config *mon_config)
{
	lite_mon_sanitize_direction(mon_config->direction);
	lite_mon_sanitize_debug(mon_config->debug);
	lite_mon_sanitize_type(mon_config->data.peer_config.type);
	lite_mon_sanitize_count(mon_config->data.peer_config.count);
	lite_mon_sanitize_interface_name(mon_config->data.peer_config.interface_name);
}

/*
 * lite_mon_get_peer_sanity: check sanity of all fields for get peer command
 * @mon_config: mon_config structre to be filled
 * return void
 */
static void lite_mon_get_peer_sanity(struct lite_mon_config *mon_config)
{
	lite_mon_sanitize_direction(mon_config->direction);
	lite_mon_sanitize_debug(mon_config->debug);
	lite_mon_sanitize_type(mon_config->data.peer_config.type);
}

/*
 * lite_mon_get_debug: Convert debug value into text string
 * @debug: debug value given by user
 * return void
 */
static char *lite_mon_get_debug(uint8_t debug)
{
	switch (debug) {
	case LITE_MON_TRACE_NONE:
		return "none";
	case LITE_MON_TRACE_ERROR:
		return "err";
	case LITE_MON_TRACE_INFO:
		return "info";
	case LITE_MON_TRACE_DEBUG:
		return "dbg";
	case LITE_MON_TRACE_MAX:
		return "max";
	default:
		return "unknown";
	}
}

/*
 * lite_mon_get_direction: Convert direction value into text string
 * @direction: direction value given by user
 * return void
 */
static char *lite_mon_get_direction(uint8_t direction)
{
	switch (direction) {
	case LITE_MON_DIRECTION_RX:
		return "rx";
	case LITE_MON_DIRECTION_TX:
		return "tx";
	default:
		return "Wrong Direction";
	}
}

/*
 * lite_mon_get_level: Convert level value into text string
 * @level: level value given by user
 * return void
 */
static char *lite_mon_get_level(uint8_t level)
{
	switch (level) {
	case LITE_MON_LEVEL_MSDU:
		return "MSDU";
	case LITE_MON_LEVEL_MPDU:
		return "MPDU";
	case LITE_MON_LEVEL_PPDU:
		return "PPDU";
	default:
		return "Wrong Level";
	}
}

/*
 * lite_mon_get_type: Convert type value into text string
 * @type: type value given by user
 * return void
 */
static char *lite_mon_get_type(uint8_t type)
{
	switch (type) {
	case LITE_MON_PEER_ASSOCIATED:
		return "ASSOCIATED";
	case LITE_MON_PEER_NON_ASSOCIATED:
		return "NON-ASSOCIATED";
	default:
		return "Wrong type";
	}
}

/*
 * lite_mon_display_filter: Display the filter settings given by user
 * @mon_config: mon_config structre given by user
 * return void
 */
static void lite_mon_display_filter(struct lite_mon_config *mon_config)
{
	struct lite_mon_filter_config *filter_config =
					&mon_config->data.filter_config;

	lite_mon_printf(LITE_MON_TRACE_INFO, "Direction: %s",
			lite_mon_get_direction(mon_config->direction));
	lite_mon_printf(LITE_MON_TRACE_INFO, "Debug: %02X",
			mon_config->debug);
	lite_mon_printf(LITE_MON_TRACE_INFO, "Level: %s",
			lite_mon_get_level(filter_config->level));
	lite_mon_printf(LITE_MON_TRACE_INFO, "Metadata Requested: %02X",
			filter_config->metadata);
	lite_mon_printf(LITE_MON_TRACE_INFO, "Interface Name: %s",
			filter_config->interface_name);
	lite_mon_printf(LITE_MON_TRACE_INFO, "MGMT Length: %02X ",
			filter_config->len[LITE_MON_TYPE_MGMT]);
	lite_mon_printf(LITE_MON_TRACE_INFO, "CTRL Length: %02X ",
			filter_config->len[LITE_MON_TYPE_CTRL]);
	lite_mon_printf(LITE_MON_TRACE_INFO, "DATA Length: %02X ",
			filter_config->len[LITE_MON_TYPE_DATA]);
	lite_mon_printf(LITE_MON_TRACE_INFO,
			"MGMT filter: FP %02X MD %02X MO %02X",
			filter_config->mgmt_filter[LITE_MON_MODE_FILTER_FP],
			filter_config->mgmt_filter[LITE_MON_MODE_FILTER_MD],
			filter_config->mgmt_filter[LITE_MON_MODE_FILTER_MO]);
	lite_mon_printf(LITE_MON_TRACE_INFO,
			"CTRL filter: FP %02X MD %02X MO %02X",
			filter_config->ctrl_filter[LITE_MON_MODE_FILTER_FP],
			filter_config->ctrl_filter[LITE_MON_MODE_FILTER_MD],
			filter_config->ctrl_filter[LITE_MON_MODE_FILTER_MO]);
	lite_mon_printf(LITE_MON_TRACE_INFO,
			"DATA filter: FP %02X MD %02X MO %02X",
			filter_config->data_filter[LITE_MON_MODE_FILTER_FP],
			filter_config->data_filter[LITE_MON_MODE_FILTER_MD],
			filter_config->data_filter[LITE_MON_MODE_FILTER_MO]);
}

/*
 * lite_mon_display_peer: Display the peer filter settings given by user
 * @mon_config: mon_config structre given by user
 * return void
 */
static void lite_mon_display_peer(struct lite_mon_config *mon_config)
{
	int i;

	lite_mon_printf(LITE_MON_TRACE_INFO, "Direction: %s",
			lite_mon_get_direction(mon_config->direction));
	lite_mon_printf(LITE_MON_TRACE_INFO, "Debug: %02X",
			mon_config->debug);
	lite_mon_printf(LITE_MON_TRACE_INFO, "Type: %s",
			lite_mon_get_type(mon_config->data.peer_config.type));
	lite_mon_printf(LITE_MON_TRACE_INFO, "Number of peers in list: %d",
			mon_config->data.peer_config.count);
	for (i = 0; i < mon_config->data.peer_config.count; i++) {
		lite_mon_printf(LITE_MON_TRACE_INFO,
				"mac %02X:%02X:%02X:%02X:%02X:%02X",
				mon_config->data.peer_config.mac_addr[i][0],
				mon_config->data.peer_config.mac_addr[i][1],
				mon_config->data.peer_config.mac_addr[i][2],
				mon_config->data.peer_config.mac_addr[i][3],
				mon_config->data.peer_config.mac_addr[i][4],
				mon_config->data.peer_config.mac_addr[i][5]);
	}
}

/*
 * lite_mon_handle_callback: Callback function from driver
 * @buffer: callback buffer that is given by the driver
 * return void
 */
static void lite_mon_handle_callback(struct cfg80211_data *buffer)
{
	uint8_t *buf = buffer->data;
	uint32_t len = buffer->length;
	struct lite_mon_config *mon_config;

	if (len < sizeof(struct lite_mon_config)) {
		lite_mon_printf(LITE_MON_TRACE_ERROR,
				"Got wrong length in callback message %d", len);
		return;
	}

	mon_config = (struct lite_mon_config *)buf;

	switch (mon_config->cmdtype) {
	case LITE_MON_GET_FILTER:
		lite_mon_display_filter(mon_config);
		break;
	case LITE_MON_GET_PEER:
		lite_mon_display_peer(mon_config);
		break;
	/* Fallthrough */
	case LITE_MON_SET_FILTER:
	case LITE_MON_SET_PEER:
	default:
		lite_mon_printf(LITE_MON_TRACE_ERROR,
				"Callback not expected for cmdtype %d",
				mon_config->cmdtype);
		exit(0);
		break;
	}
}

/*
 * lite_mon_dump_structure_content: Dump the mon structure given by user
 * @mon_config: mon_config structre given by user
 * return void
 */
static void lite_mon_dump_structure_content(struct lite_mon_config *mon_config)
{
	int i;
	struct lite_mon_filter_config *filter_config =
					&mon_config->data.filter_config;
	struct lite_mon_peer_config *peer_config =
					&mon_config->data.peer_config;

	lite_mon_printf(LITE_MON_TRACE_DEBUG,
			"Dumping Lite Mon structure contents: ");
	lite_mon_printf(LITE_MON_TRACE_DEBUG,
			"cmdtype %d direction %d debug %d",
			mon_config->cmdtype, mon_config->direction,
			mon_config->debug);
	if (mon_config->cmdtype == LITE_MON_SET_FILTER ||
	    mon_config->cmdtype == LITE_MON_GET_FILTER) {
		lite_mon_printf(LITE_MON_TRACE_DEBUG,
				"level %d show %d disable %d metadata %d interface_name %s",
				filter_config->level,
				filter_config->show_filter,
				filter_config->disable,
				filter_config->metadata,
				filter_config->interface_name);
		lite_mon_printf(LITE_MON_TRACE_DEBUG,
				"mgmt filter fp %02X md %02X mo %02X",
				filter_config->mgmt_filter[0],
				filter_config->mgmt_filter[1],
				filter_config->mgmt_filter[2]);
		lite_mon_printf(LITE_MON_TRACE_DEBUG,
				"ctrl filter fp %02X md %02X mo %02X",
				filter_config->ctrl_filter[0],
				filter_config->ctrl_filter[1],
				filter_config->ctrl_filter[2]);
		lite_mon_printf(LITE_MON_TRACE_DEBUG,
				"data filter fp %02X md %02X mo %02X",
				filter_config->data_filter[0],
				filter_config->data_filter[1],
				filter_config->data_filter[2]);
		lite_mon_printf(LITE_MON_TRACE_DEBUG,
				"len mgmt %02X ctrl %02X data %02X",
				filter_config->len[0],
				filter_config->len[1],
				filter_config->len[2]);
	} else {
		lite_mon_printf(LITE_MON_TRACE_DEBUG,
				"action %d type %d count %d vap %s",
				peer_config->action,
				peer_config->type,
				peer_config->count,
				peer_config->interface_name);
		for (i = 0; i < peer_config->count; i++)
			lite_mon_printf(LITE_MON_TRACE_DEBUG,
					"mac %02X:%02X:%02X:%02X:%02X:%02X",
					peer_config->mac_addr[i][0],
					peer_config->mac_addr[i][1],
					peer_config->mac_addr[i][2],
					peer_config->mac_addr[i][3],
					peer_config->mac_addr[i][4],
					peer_config->mac_addr[i][5]);
	}
}

static void lite_mon_validate_interface(const char *ifname)
{
	if (strncmp(ifname, "wifi", WIFI_IFACE_PREFIX_LEN)) {
		lite_mon_printf(LITE_MON_TRACE_ERROR,
				"Interface name wifiX missing/wrong");
		lite_mon_printf(LITE_MON_TRACE_ERROR,
				"Not able to send cfg command");
		exit(0);
	}
}

static void lite_mon_peer_operation(struct lite_mon_config *mon_config,
				    char *optarg)
{
	char *token;
	uint8_t mac_addr[IEEE80211_ADDR_LEN];
	int count = 0;

	while ((token = strtok_r(optarg, ",", &optarg))) {
		if (ether_string2mac(mac_addr, token) != 0) {
			lite_mon_printf(LITE_MON_TRACE_ERROR,
					"Invalid MAC address");
			exit(0);
		} else if (IEEE80211_IS_BROADCAST(mac_addr)) {
			lite_mon_printf(LITE_MON_TRACE_ERROR,
					"Broadcast MAC address is invalid");
			exit(0);
		} else if (IEEE80211_IS_MULTICAST(mac_addr)) {
			lite_mon_printf(LITE_MON_TRACE_ERROR,
					"Multicast MAC address is invalid");
			exit(0);
		} else {
			if (count == LITE_MON_MAX_PEER_COUNT) {
				lite_mon_printf(LITE_MON_TRACE_ERROR,
						"More than allowed mac addresses procided count %d",
						count);
				exit(0);
			}
			memcpy(mon_config->data.peer_config.mac_addr[count++],
			       &mac_addr, IEEE80211_ADDR_LEN);
		}
	}
	mon_config->data.peer_config.count = count;
}

int main(int argc, char *argv[])
{
	int c;
	int option_index = 0;
	int cmd_bitmask = 0;
	uint32_t filter;
	struct lite_mon_config mon_config = {0};
	struct socket_context sock_ctx = {0};
	const char *ifname = argv[1];

	/* Set default debug level to INFO */
	debug_level = LITE_MON_TRACE_INFO;
	mon_config.debug = LITE_MON_TRACE_INFO;

	sock_ctx.cfg80211 =  get_config_mode_type();
	init_socket_context(&sock_ctx, DEFAULT_NL80211_CMD_SOCK_ID,
			    DEFAULT_NL80211_EVENT_SOCK_ID);

	while (1) {
		c = getopt_long(argc, argv, "d:e:Dp:m:o:j:k:l:O:sa:r:Lx:M:t:",
				long_options, &option_index);
		if (c == -1)
			break;

		switch (c) {
		case 'd': /* direction */
			if (strncmp(optarg, "rx", strlen("rx")) == 0) {
				mon_config.direction = LITE_MON_DIRECTION_RX;
			} else if (strncmp(optarg, "tx", strlen("tx")) == 0) {
				mon_config.direction = LITE_MON_DIRECTION_TX;
			} else {
				lite_mon_printf(LITE_MON_TRACE_ERROR,
						"Wrong Direction option provided %s",
						optarg);
				exit(0);
			}
			break;
		case 'e': /* level */
			mon_config.data.filter_config.level = atoi(optarg);
			cmd_bitmask |= LITE_MON_SET_FILTER_MASK;
			break;
		case 'D': /* disable */
			mon_config.data.filter_config.disable = 1;
			cmd_bitmask |= LITE_MON_SET_FILTER_MASK;
			cmd_bitmask |= LITE_MON_SANITY_NOT_NEEDED;
			break;
		case 'p': /* filter_fp */
			cmd_bitmask |= LITE_MON_SET_FILTER_MASK;
			filter = strtoul(optarg, NULL, 0);
			lite_mon_fill_fil_value(&mon_config.data.filter_config,
						filter,
						LITE_MON_MODE_FILTER_FP);
			break;
		case 'm': /* filter_md */
			cmd_bitmask |= LITE_MON_SET_FILTER_MASK;
			filter = strtoul(optarg, NULL, 0);
			lite_mon_fill_fil_value(&mon_config.data.filter_config,
						filter,
						LITE_MON_MODE_FILTER_MD);
			break;
		case 'o': /* filter_mo */
			cmd_bitmask |= LITE_MON_SET_FILTER_MASK;
			filter = strtoul(optarg, NULL, 0);
			lite_mon_fill_fil_value(&mon_config.data.filter_config,
						filter,
						LITE_MON_MODE_FILTER_MO);
			break;
		case 'j': /* mgmt_len */
			cmd_bitmask |= LITE_MON_SET_FILTER_MASK;
			mon_config.data.filter_config.len[LITE_MON_TYPE_MGMT] =
				strtoul(optarg, NULL, 0);
			break;
		case 'k': /* ctrl_len */
			cmd_bitmask |= LITE_MON_SET_FILTER_MASK;
			mon_config.data.filter_config.len[LITE_MON_TYPE_CTRL] =
				strtoul(optarg, NULL, 0);
			break;
		case 'l': /* data_len */
			cmd_bitmask |= LITE_MON_SET_FILTER_MASK;
			mon_config.data.filter_config.len[LITE_MON_TYPE_DATA] =
				strtoul(optarg, NULL, 0);
			break;
		case 'M': /* metadata */
			cmd_bitmask |= LITE_MON_SET_FILTER_MASK;
			mon_config.data.filter_config.metadata =
				strtoul(optarg, NULL, 0);
			break;
		case 'O': /* output */
			cmd_bitmask |= LITE_MON_SET_FILTER_MASK;
			strlcpy(mon_config.data.filter_config.interface_name,
				optarg,
				sizeof(mon_config.data.filter_config.interface_name));
			break;
		case 's': /* show_filter */
			cmd_bitmask |= LITE_MON_GET_FILTER_MASK;
			mon_config.data.filter_config.show_filter = 1;
			break;
		case 'a': /* peer_add */
			cmd_bitmask |= LITE_MON_SET_PEER_MASK;
			mon_config.data.peer_config.action = LITE_MON_PEER_ADD;
			lite_mon_peer_operation(&mon_config, optarg);
			break;
		case 'r': /* peer_remove */
			cmd_bitmask |= LITE_MON_SET_PEER_MASK;
			mon_config.data.peer_config.action =
							LITE_MON_PEER_REMOVE;
			lite_mon_peer_operation(&mon_config, optarg);
			break;
		case 'L': /* peer_list */
			cmd_bitmask |= LITE_MON_GET_PEER_MASK;
			mon_config.data.peer_config.action = LITE_MON_PEER_LIST;
			break;
		case 't': /* type */
			if (strncmp(optarg, "assoc", strlen("assoc")) == 0) {
				mon_config.data.peer_config.type =
						LITE_MON_PEER_ASSOCIATED;
			} else if (strncmp(optarg, "non-assoc",
				   strlen("non-assoc")) == 0) {
				mon_config.data.peer_config.type =
						LITE_MON_PEER_NON_ASSOCIATED;
			} else {
				lite_mon_printf(LITE_MON_TRACE_ERROR,
						"Wrong option provided for type %s",
						optarg);
				exit(0);
			}
			break;
		case 'v': /* interface name */
			cmd_bitmask |= LITE_MON_SET_PEER_MASK;
			strlcpy(mon_config.data.peer_config.interface_name,
				optarg,
				sizeof(mon_config.data.peer_config.interface_name));
			break;
		case 'x': /* debug */
			mon_config.debug = atoi(optarg);
			debug_level = mon_config.debug;
			break;
		case 'h': /* help */
			usage();
			break;
		default: /* wrong option */
			lite_mon_printf(LITE_MON_TRACE_ERROR,
					"Cannot understand option %s",
					argv[option_index]);
			usage();
			break;
		}
	}
	lite_mon_set_cmdtype(&mon_config, cmd_bitmask);
	lite_mon_set_defaults(&mon_config);
	if (!(cmd_bitmask & LITE_MON_SANITY_NOT_NEEDED)) {
		switch (mon_config.cmdtype) {
		case LITE_MON_SET_FILTER:
			lite_mon_set_filter_sanity(&mon_config);
			break;
		case LITE_MON_SET_PEER:
			lite_mon_set_peer_sanity(&mon_config);
			break;
		case LITE_MON_GET_FILTER:
			lite_mon_get_filter_sanity(&mon_config);
			break;
		case LITE_MON_GET_PEER:
			lite_mon_get_peer_sanity(&mon_config);
			break;
		default:
			lite_mon_printf(LITE_MON_TRACE_ERROR,
					"Unknown cmdtype %d",
					mon_config.cmdtype);
			exit(0);
			break;
		}
	}

	lite_mon_dump_structure_content(&mon_config);
	lite_mon_printf(LITE_MON_TRACE_DEBUG, "Send VENDOR CMD cmd_type %d",
			mon_config.cmdtype);
	lite_mon_validate_interface(ifname);
	send_command(&sock_ctx, ifname, &mon_config,
		     sizeof(struct lite_mon_config),
		     lite_mon_handle_callback,
		     QCA_NL80211_VENDOR_SUBCMD_LITE_MONITOR,
		     IEEE80211_IOCTL_CONFIG_GENERIC);

	destroy_socket_context(&sock_ctx);

	return 0;
}
