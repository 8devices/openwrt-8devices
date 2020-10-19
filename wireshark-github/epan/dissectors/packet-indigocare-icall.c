/* packet-indigocare-icall.c
 * Dissector routines for the IndigoCare iCall protocol
 * By Erik de Jong <erikdejong@gmail.com>
 * Copyright 2016 Erik de Jong
 *
 * Wireshark - Network traffic analyzer
 * By Gerald Combs <gerald@wireshark.org>
 * Copyright 1998 Gerald Combs
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include "config.h"

#include <range.h>
#include <wiretap/wtap.h>
#include <epan/packet.h>
#include <epan/expert.h>
#include <wsutil/strtoi.h>

#define INDIGOCARE_ICALL_SOH			0x01
#define INDIGOCARE_ICALL_STX			0x02
#define INDIGOCARE_ICALL_ETX			0x03
#define INDIGOCARE_ICALL_EOT			0x04
#define INDIGOCARE_ICALL_ACK			0x06
#define INDIGOCARE_ICALL_US			0x1F
#define INDIGOCARE_ICALL_RS			0x1E

#define INDIGOCARE_ICALL_CALL			0x0A

#define INDIGOCARE_ICALL_CALL_ROOM		0x01
#define INDIGOCARE_ICALL_CALL_TYPE		0x02
#define INDIGOCARE_ICALL_CALL_ADDITION		0x03
#define INDIGOCARE_ICALL_CALL_ID		0x04
#define INDIGOCARE_ICALL_CALL_TASK		0x05
#define INDIGOCARE_ICALL_CALL_LOCATION		0x06
#define INDIGOCARE_ICALL_CALL_NAME1		0x07
#define INDIGOCARE_ICALL_CALL_NAME2		0x08
#define INDIGOCARE_ICALL_CALL_TYPE_NUMERICAL	0x09
#define INDIGOCARE_ICALL_CALL_NURSE		0x0A

void proto_reg_handoff_icall(void);
void proto_register_icall(void);

static expert_field ei_icall_unexpected_header = EI_INIT;
static expert_field ei_icall_unexpected_record = EI_INIT;

static int proto_icall = -1;
static int hf_icall_header_type = -1;

static int hf_icall_call_room_type = -1;
static int hf_icall_call_type_type = -1;
static int hf_icall_call_addition_type = -1;
static int hf_icall_call_id_type = -1;
static int hf_icall_call_task_type = -1;
static int hf_icall_call_location_type = -1;
static int hf_icall_call_name1_type = -1;
static int hf_icall_call_name2_type = -1;
static int hf_icall_call_numerical_type = -1;
static int hf_icall_call_nurse_type = -1;

static gint ett_icall = -1;
static gint ett_icall_call = -1;
static gint ett_icall_unknown = -1;

static const value_string icall_headertypenames[] = {
	{ INDIGOCARE_ICALL_CALL,		"Call Info" },
	{ 0, NULL }
};

static int
dissect_icall(tvbuff_t *tvb, packet_info *pinfo, proto_tree *tree _U_, void *data _U_)
{
	proto_item *ti;
	proto_item *header_item;
	proto_tree *icall_tree;
	proto_tree *icall_header_tree;
	gint32 current_offset = 0, header_offset, identifier_start, identifier_offset, data_start, data_offset, ett;
	gint32 header;
	gint32 record_identifier;
	const guint8 * record_data;

	/* Starts with SOH */
	if ( tvb_get_guint8(tvb, 0) != INDIGOCARE_ICALL_SOH )
		return 0;
	/* Ends with EOT */
	if ((pinfo->phdr->caplen == pinfo->phdr->len) && ( tvb_get_guint8(tvb, tvb_reported_length(tvb) - 1) != INDIGOCARE_ICALL_EOT ))
		return 0;
	/* It is a iCall Communication Protocol packet */
	col_set_str(pinfo->cinfo, COL_PROTOCOL, "iCall");
	col_clear(pinfo->cinfo,COL_INFO);
	ti = proto_tree_add_item(tree, proto_icall, tvb, 0, -1, ENC_NA);
	icall_tree = proto_item_add_subtree(ti, ett_icall);
	current_offset++;

	/* Read header */
	header_offset = tvb_find_guint8(tvb, current_offset, -1, INDIGOCARE_ICALL_STX);
	ws_strtoi32(tvb_get_string_enc(wmem_packet_scope(), tvb, current_offset, header_offset - current_offset, ENC_NA), NULL, &header);
	col_add_fstr(pinfo->cinfo, COL_INFO, "%s:", val_to_str(header, icall_headertypenames, "Unknown (%d)"));
	switch(header) {
		case INDIGOCARE_ICALL_CALL:
			ett = ett_icall_call;
		break;
		default:
			proto_tree_add_expert_format(icall_tree, pinfo, &ei_icall_unexpected_header, tvb, current_offset, header_offset -  current_offset, "Unexpected header %d", header);
			return 0;
		break;
	}
	header_item = proto_tree_add_uint_format(icall_tree, hf_icall_header_type, tvb, current_offset, header_offset - current_offset, header, "%s", val_to_str(header, icall_headertypenames, "Unknown (%d)"));
	icall_header_tree = proto_item_add_subtree(header_item, ett);
	current_offset = header_offset + 1;

	/* Read records */
	while (tvb_get_guint8(tvb, current_offset) != INDIGOCARE_ICALL_ETX) {
		identifier_start = current_offset;
		identifier_offset = tvb_find_guint8(tvb, current_offset, -1, INDIGOCARE_ICALL_US);
		ws_strtoi32(tvb_get_string_enc(wmem_packet_scope(), tvb, current_offset, identifier_offset - current_offset, ENC_NA), NULL, &record_identifier);
		current_offset = identifier_offset + 1;

		data_start = current_offset;
		data_offset = tvb_find_guint8(tvb, data_start, -1, INDIGOCARE_ICALL_RS);
		record_data = tvb_get_string_enc(wmem_packet_scope(), tvb, current_offset, data_offset - data_start, ENC_NA);

		current_offset = data_offset + 1;

		switch (header) {
			case INDIGOCARE_ICALL_CALL:
				switch (record_identifier) {
					case INDIGOCARE_ICALL_CALL_ROOM:
						proto_tree_add_item_ret_string(icall_header_tree, hf_icall_call_room_type, tvb, data_start, data_offset - data_start, ENC_ASCII|ENC_NA, wmem_packet_scope(), &record_data);
						col_append_fstr(pinfo->cinfo, COL_INFO, " Room=%s", record_data);
					break;
					case INDIGOCARE_ICALL_CALL_TYPE:
						proto_tree_add_item_ret_string(icall_header_tree, hf_icall_call_type_type, tvb, data_start, data_offset - data_start, ENC_ASCII|ENC_NA, wmem_packet_scope(), &record_data);
						col_append_fstr(pinfo->cinfo, COL_INFO, " Type=%s", record_data);
					break;
					case INDIGOCARE_ICALL_CALL_ADDITION:
						proto_tree_add_item(icall_header_tree, hf_icall_call_addition_type, tvb, data_start, data_offset - data_start, ENC_ASCII|ENC_NA);
					break;
					case INDIGOCARE_ICALL_CALL_ID:
						proto_tree_add_item(icall_header_tree, hf_icall_call_id_type, tvb, data_start, data_offset - data_start, ENC_ASCII|ENC_NA);
					break;
					case INDIGOCARE_ICALL_CALL_TASK:
						proto_tree_add_item(icall_header_tree, hf_icall_call_task_type, tvb, data_start, data_offset - data_start, ENC_ASCII|ENC_NA);
					break;
					case INDIGOCARE_ICALL_CALL_LOCATION:
						proto_tree_add_item_ret_string(icall_header_tree, hf_icall_call_location_type, tvb, data_start, data_offset - data_start, ENC_ASCII|ENC_NA, wmem_packet_scope(), &record_data);
						col_append_fstr(pinfo->cinfo, COL_INFO, " Location=%s", record_data);
					break;
					case INDIGOCARE_ICALL_CALL_NAME1:
						proto_tree_add_item_ret_string(icall_header_tree, hf_icall_call_name1_type, tvb, data_start, data_offset - data_start, ENC_ASCII|ENC_NA, wmem_packet_scope(), &record_data);
						col_append_fstr(pinfo->cinfo, COL_INFO, " Name 1=%s", record_data);
					break;
					case INDIGOCARE_ICALL_CALL_NAME2:
						proto_tree_add_item_ret_string(icall_header_tree, hf_icall_call_name2_type, tvb, data_start, data_offset - data_start, ENC_ASCII|ENC_NA, wmem_packet_scope(), &record_data);
						col_append_fstr(pinfo->cinfo, COL_INFO, " Name 2=%s", record_data);
					break;
					case INDIGOCARE_ICALL_CALL_TYPE_NUMERICAL:
						proto_tree_add_item(icall_header_tree, hf_icall_call_numerical_type, tvb, data_start, data_offset - data_start, ENC_ASCII|ENC_NA);
					break;
					case INDIGOCARE_ICALL_CALL_NURSE:
						proto_tree_add_item(icall_header_tree, hf_icall_call_nurse_type, tvb, data_start, data_offset - data_start, ENC_ASCII|ENC_NA);
					break;
					default:
						proto_tree_add_expert_format(icall_header_tree, pinfo, &ei_icall_unexpected_record, tvb, identifier_start, data_offset - identifier_start, "Unexpected record %d with value %s", record_identifier, record_data);
					break;
				}
			break;
		}
	}
	return tvb_captured_length(tvb);
}

void
proto_reg_handoff_icall(void)
{
	dissector_handle_t icall_handle;

	icall_handle = create_dissector_handle(dissect_icall, proto_icall);
	dissector_add_for_decode_as("udp.port", icall_handle);
	dissector_add_for_decode_as("tcp.port", icall_handle);
}

void
proto_register_icall(void)
{
	static hf_register_info hf[] = {
	{ &hf_icall_header_type,
		{ "Header Type", "icall.header",
		FT_UINT32, BASE_DEC,
		VALS(icall_headertypenames), 0x0,
		NULL, HFILL }
	},
	{ &hf_icall_call_room_type,
		{ "Room", "icall.call.room",
		FT_STRING, BASE_NONE,
		NULL, 0x0,
		NULL, HFILL }
	},
	{ &hf_icall_call_type_type,
		{ "Type", "icall.call.type",
		FT_STRING, BASE_NONE,
		NULL, 0x0,
		NULL, HFILL }
	},
	{ &hf_icall_call_addition_type,
		{ "Addition", "icall.call.addition",
		FT_STRING, BASE_NONE,
		NULL, 0x0,
		NULL, HFILL }
	},
	{ &hf_icall_call_id_type,
		{ "ID", "icall.call.id",
		FT_STRING, BASE_NONE,
		NULL, 0x0,
		NULL, HFILL }
	},
	{ &hf_icall_call_task_type,
		{ "Task", "icall.call.task",
		FT_STRING, BASE_NONE,
		NULL, 0x0,
		NULL, HFILL }
	},
	{ &hf_icall_call_location_type,
		{ "Location", "icall.call.location",
		FT_STRING, BASE_NONE,
		NULL, 0x0,
		NULL, HFILL }
	},
	{ &hf_icall_call_name1_type,
		{ "Name 1", "icall.call.name1",
		FT_STRING, BASE_NONE,
		NULL, 0x0,
		NULL, HFILL }
	},
	{ &hf_icall_call_name2_type,
		{ "Name 2", "icall.call.name2",
		FT_STRING, BASE_NONE,
		NULL, 0x0,
		NULL, HFILL }
	},
	{ &hf_icall_call_numerical_type,
		{ "Type Numerical", "icall.call.type_numerical",
		FT_STRING, BASE_NONE,
		NULL, 0x0,
		NULL, HFILL }
	},
	{ &hf_icall_call_nurse_type,
		{ "Nurse", "icall.call.nurse",
		FT_STRING, BASE_NONE,
		NULL, 0x0,
		NULL, HFILL }
	}
	};

	static ei_register_info ei[] = {
		{ &ei_icall_unexpected_header, { "icall.unexpected.header", PI_MALFORMED, PI_WARN, "Unexpected header", EXPFILL }},
		{ &ei_icall_unexpected_record, { "icall.unexpected.record", PI_MALFORMED, PI_WARN, "Unexpected record", EXPFILL }}
	};

	expert_module_t* expert_icall;

	/* Setup protocol subtree array */
	static gint *ett[] = {
		&ett_icall,
		&ett_icall_call,
		&ett_icall_unknown
	};

	proto_icall = proto_register_protocol (
		"iCall Communication Protocol",	/* name */
		"iCall",			/* short name */
		"icall"				/* abbrev */
	);

	proto_register_field_array(proto_icall, hf, array_length(hf));
	proto_register_subtree_array(ett, array_length(ett));

	expert_icall = expert_register_protocol(proto_icall);
	expert_register_field_array(expert_icall, ei, array_length(ei));
}

/*
 * Editor modelines  -  http://www.wireshark.org/tools/modelines.html
 *
 * Local variables:
 * c-basic-offset: 8
 * tab-width: 8
 * indent-tabs-mode: t
 * End:
 *
 * vi: set shiftwidth=8 tabstop=8 noexpandtab:
 * :indentSize=8:tabSize=8:noTabs=false:
 */
