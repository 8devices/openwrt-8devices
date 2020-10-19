/* packet-btle.c
 * Routines for Bluetooth Low Energy Link Layer dissection
 * https://www.bluetooth.org/Technical/Specifications/adopted.htm
 *
 * Copyright 2013, Mike Ryan, mikeryan /at/ isecpartners /dot/ com
 * Copyright 2013, Michal Labedzki for Tieto Corporation
 * Copyright 2014, Christopher D. Kilgour, techie at whiterocker dot com
 * Copyright 2017, Stig Bjorlykke for Nordic Semiconductor
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

#include <epan/packet.h>
#include <epan/prefs.h>
#include <epan/expert.h>
#include <epan/proto_data.h>
#include <epan/reassemble.h>

#include <wiretap/wtap.h>

#include "packet-btle.h"
#include "packet-bthci_cmd.h"
#include "packet-bthci_acl.h"

static int proto_btle = -1;
static int proto_btle_rf = -1;
static int proto_nordic_ble = -1;

static int hf_access_address = -1;
static int hf_crc = -1;
static int hf_master_bd_addr = -1;
static int hf_slave_bd_addr = -1;
static int hf_length = -1;
static int hf_advertising_header = -1;
static int hf_advertising_header_pdu_type = -1;
static int hf_advertising_header_ch_sel = -1;
static int hf_advertising_header_rfu_1 = -1;
static int hf_advertising_header_randomized_tx = -1;
static int hf_advertising_header_randomized_rx = -1;
static int hf_advertising_header_reserved = -1;
static int hf_advertising_header_length = -1;
static int hf_advertising_address = -1;
static int hf_initiator_addresss = -1;
static int hf_scanning_address = -1;
static int hf_scan_response_data = -1;
static int hf_link_layer_data = -1;
static int hf_link_layer_data_access_address = -1;
static int hf_link_layer_data_crc_init = -1;
static int hf_link_layer_data_window_size = -1;
static int hf_link_layer_data_window_offset = -1;
static int hf_link_layer_data_interval = -1;
static int hf_link_layer_data_latency = -1;
static int hf_link_layer_data_timeout = -1;
static int hf_link_layer_data_channel_map = -1;
static int hf_link_layer_data_hop = -1;
static int hf_link_layer_data_sleep_clock_accuracy = -1;
static int hf_data_header = -1;
static int hf_data_header_length = -1;
static int hf_data_header_rfu = -1;
static int hf_data_header_llid = -1;
static int hf_data_header_more_data = -1;
static int hf_data_header_sequence_number = -1;
static int hf_data_header_next_expected_sequence_number = -1;
static int hf_control_opcode = -1;
static int hf_l2cap_index = -1;
static int hf_l2cap_fragment = -1;
static int hf_control_reject_opcode = -1;
static int hf_control_error_code = -1;
static int hf_control_unknown_type = -1;
static int hf_control_version_number = -1;
static int hf_control_company_id = -1;
static int hf_control_subversion_number = -1;
static int hf_control_feature_set = -1;
static int hf_control_feature_set_le_encryption = -1;
static int hf_control_feature_set_connection_parameters_request_procedure = -1;
static int hf_control_feature_set_extended_reject_indication = -1;
static int hf_control_feature_set_slave_initiated_features_exchange = -1;
static int hf_control_feature_set_le_ping = -1;
static int hf_control_feature_set_le_pkt_len_ext = -1;
static int hf_control_feature_set_ll_privacy = -1;
static int hf_control_feature_set_ext_scan_flt_pol = -1;
static int hf_control_feature_set_le_2m_phy = -1;
static int hf_control_feature_set_stable_modulation_index_transmitter = -1;
static int hf_control_feature_set_stable_modulation_index_receiver = -1;
static int hf_control_feature_set_le_coded_phy = -1;
static int hf_control_feature_set_le_extended_advertising = -1;
static int hf_control_feature_set_le_periodic_advertising = -1;
static int hf_control_feature_set_channel_selection_algorithm_2 = -1;
static int hf_control_feature_set_le_power_class_1 = -1;
static int hf_control_feature_set_minimum_number_of_used_channels_procedure = -1;
static int hf_control_feature_set_reserved_bits = -1;
static int hf_control_feature_set_reserved = -1;
static int hf_control_window_size = -1;
static int hf_control_window_offset = -1;
static int hf_control_interval = -1;
static int hf_control_latency = -1;
static int hf_control_timeout = -1;
static int hf_control_instant = -1;
static int hf_control_interval_min = -1;
static int hf_control_interval_max = -1;
static int hf_control_preffered_periodicity = -1;
static int hf_control_reference_connection_event_count = -1;
static int hf_control_offset_0 = -1;
static int hf_control_offset_1 = -1;
static int hf_control_offset_2 = -1;
static int hf_control_offset_3 = -1;
static int hf_control_offset_4 = -1;
static int hf_control_offset_5 = -1;
static int hf_control_channel_map = -1;
static int hf_control_random_number = -1;
static int hf_control_encrypted_diversifier = -1;
static int hf_control_master_session_key_diversifier = -1;
static int hf_control_master_session_initialization_vector = -1;
static int hf_control_slave_session_key_diversifier = -1;
static int hf_control_slave_session_initialization_vector = -1;
static int hf_control_max_rx_octets = -1;
static int hf_control_max_rx_time = -1;
static int hf_control_max_tx_octets = -1;
static int hf_control_max_tx_time = -1;
static int hf_control_phys_sender_le_1m_phy = -1;
static int hf_control_phys_sender_le_2m_phy = -1;
static int hf_control_phys_sender_le_coded_phy = -1;
static int hf_control_phys_update_le_1m_phy = -1;
static int hf_control_phys_update_le_2m_phy = -1;
static int hf_control_phys_update_le_coded_phy = -1;
static int hf_control_phys_reserved_bits = -1;
static int hf_control_tx_phys = -1;
static int hf_control_rx_phys = -1;
static int hf_control_m_to_s_phy = -1;
static int hf_control_s_to_m_phy = -1;
static int hf_control_phys = -1;
static int hf_control_phys_le_1m_phy = -1;
static int hf_control_phys_le_2m_phy = -1;
static int hf_control_phys_le_coded_phy = -1;
static int hf_control_min_used_channels = -1;
static int hf_btle_l2cap_msg_fragments = -1;
static int hf_btle_l2cap_msg_fragment = -1;
static int hf_btle_l2cap_msg_fragment_overlap = -1;
static int hf_btle_l2cap_msg_fragment_overlap_conflicts = -1;
static int hf_btle_l2cap_msg_fragment_multiple_tails = -1;
static int hf_btle_l2cap_msg_fragment_too_long_fragment = -1;
static int hf_btle_l2cap_msg_fragment_error = -1;
static int hf_btle_l2cap_msg_fragment_count = -1;
static int hf_btle_l2cap_msg_reassembled_in = -1;
static int hf_btle_l2cap_msg_reassembled_length = -1;


static gint ett_btle = -1;
static gint ett_advertising_header = -1;
static gint ett_link_layer_data = -1;
static gint ett_data_header = -1;
static gint ett_features = -1;
static gint ett_tx_phys = -1;
static gint ett_rx_phys = -1;
static gint ett_m_to_s_phy = -1;
static gint ett_s_to_m_phy = -1;
static gint ett_phys = -1;
static gint ett_channel_map = -1;
static gint ett_scan_response_data = -1;
static gint ett_btle_l2cap_msg_fragment = -1;
static gint ett_btle_l2cap_msg_fragments = -1;


static const int *hfx_control_feature_set_1[] = {
    &hf_control_feature_set_le_encryption,
    &hf_control_feature_set_connection_parameters_request_procedure,
    &hf_control_feature_set_extended_reject_indication,
    &hf_control_feature_set_slave_initiated_features_exchange,
    &hf_control_feature_set_le_ping,
    &hf_control_feature_set_le_pkt_len_ext,
    &hf_control_feature_set_ll_privacy,
    &hf_control_feature_set_ext_scan_flt_pol,
    NULL
};

static const int *hfx_control_feature_set_2[] = {
    &hf_control_feature_set_le_2m_phy,
    &hf_control_feature_set_stable_modulation_index_transmitter,
    &hf_control_feature_set_stable_modulation_index_receiver,
    &hf_control_feature_set_le_coded_phy,
    &hf_control_feature_set_le_extended_advertising,
    &hf_control_feature_set_le_periodic_advertising,
    &hf_control_feature_set_channel_selection_algorithm_2,
    &hf_control_feature_set_le_power_class_1,
    NULL
};

static const int *hfx_control_feature_set_3[] = {
    &hf_control_feature_set_minimum_number_of_used_channels_procedure,
    &hf_control_feature_set_reserved_bits,
    NULL
};

static const int *hfx_control_phys_sender[] = {
    &hf_control_phys_sender_le_1m_phy,
    &hf_control_phys_sender_le_2m_phy,
    &hf_control_phys_sender_le_coded_phy,
    &hf_control_phys_reserved_bits,
    NULL
};

static const int *hfx_control_phys_update[] = {
    &hf_control_phys_update_le_1m_phy,
    &hf_control_phys_update_le_2m_phy,
    &hf_control_phys_update_le_coded_phy,
    &hf_control_phys_reserved_bits,
    NULL
};

static const int *hfx_control_phys[] = {
    &hf_control_phys_le_1m_phy,
    &hf_control_phys_le_2m_phy,
    &hf_control_phys_le_coded_phy,
    &hf_control_phys_reserved_bits,
    NULL
};


static expert_field ei_unknown_data = EI_INIT;
static expert_field ei_access_address_matched = EI_INIT;
static expert_field ei_access_address_bit_errors = EI_INIT;
static expert_field ei_access_address_illegal = EI_INIT;
static expert_field ei_crc_cannot_be_determined = EI_INIT;
static expert_field ei_crc_incorrect = EI_INIT;
static expert_field ei_missing_fragment_start = EI_INIT;
static expert_field ei_retransmit = EI_INIT;

static dissector_handle_t btle_handle;
static dissector_handle_t btcommon_ad_handle;
static dissector_handle_t btcommon_le_channel_map_handle;
static dissector_handle_t btl2cap_handle;

static wmem_tree_t *connection_info_tree;
static guint32 l2cap_index;

/* Reassembly */
static reassembly_table btle_l2cap_msg_reassembly_table;

static const fragment_items btle_l2cap_msg_frag_items = {
    /* Fragment subtrees */
    &ett_btle_l2cap_msg_fragment,
    &ett_btle_l2cap_msg_fragments,
    /* Fragment fields */
    &hf_btle_l2cap_msg_fragments,
    &hf_btle_l2cap_msg_fragment,
    &hf_btle_l2cap_msg_fragment_overlap,
    &hf_btle_l2cap_msg_fragment_overlap_conflicts,
    &hf_btle_l2cap_msg_fragment_multiple_tails,
    &hf_btle_l2cap_msg_fragment_too_long_fragment,
    &hf_btle_l2cap_msg_fragment_error,
    &hf_btle_l2cap_msg_fragment_count,
    /* Reassembled in field */
    &hf_btle_l2cap_msg_reassembled_in,
    /* Reassembled length field */
    &hf_btle_l2cap_msg_reassembled_length,
    /* Reassembled data field */
    NULL,
    /* Tag */
    "BTLE L2CAP fragments"
};

/* Store information about a connection direction */
typedef struct _direction_info_t {
    guint    prev_seq_num : 1;          /* Previous sequence number for this direction */
    guint    segmentation_started : 1;  /* 0 = No, 1 = Yes */
    guint    segment_len_rem;           /* The remaining segment length, used to find last segment */
    guint32  l2cap_index;               /* Unique identifier for each L2CAP message */
} direction_info_t;

/* Store information about a connection */
typedef struct _connection_info_t {
    /* Address information */
    guint32  interface_id;
    guint32  adapter_id;
    guint32  access_address;

    guint8   master_bd_addr[6];
    guint8   slave_bd_addr[6];
    /* Connection information */
    /* Data used on the first pass to get info from previous frame, result will be in per_packet_data */
    guint    first_data_frame_seen : 1;
    direction_info_t direction_info[3];  /* UNKNOWN, MASTER_SLAVE and SLAVE_MASTER */
} connection_info_t;

/* */
typedef struct _btle_frame_info_t {
    guint    retransmit : 1;      /* 0 = No, 1 = Retransmitted frame */
    guint    more_fragments : 1;  /* 0 = Last fragment, 1 = More fragments */
    guint    missing_start : 1;   /* 0 = No, 1 = Missing fragment start */
    guint32  l2cap_index;         /* Unique identifier for each L2CAP message */
} btle_frame_info_t;

static const value_string pdu_type_vals[] = {
    { 0x00, "ADV_IND" },
    { 0x01, "ADV_DIRECT_IND" },
    { 0x02, "ADV_NONCONN_IND" },
    { 0x03, "SCAN_REQ" },
    { 0x04, "SCAN_RSP" },
    { 0x05, "CONNECT_REQ" },
    { 0x06, "ADV_SCAN_IND" },
    { 0, NULL }
};
static value_string_ext pdu_type_vals_ext = VALUE_STRING_EXT_INIT(pdu_type_vals);

static const value_string sleep_clock_accuracy_vals[] = {
    { 0x00, "251 ppm to 500 ppm" },
    { 0x01, "151 ppm to 250 ppm" },
    { 0x02, "101 ppm to 150 ppm" },
    { 0x03, "76 ppm to 100 ppm" },
    { 0x04, "51 ppm to 75 ppm" },
    { 0x05, "31 ppm to 50 ppm" },
    { 0x06, "21 ppm to 30 ppm" },
    { 0x07, "0 ppm to 20 ppm" },
    { 0, NULL }
};
static value_string_ext sleep_clock_accuracy_vals_ext = VALUE_STRING_EXT_INIT(sleep_clock_accuracy_vals);

static const value_string llid_codes_vals[] = {
    { 0x01, "Continuation fragment of an L2CAP message, or an Empty PDU" },
    { 0x02, "Start of an L2CAP message or a complete L2CAP message with no fragmentation" },
    { 0x03, "Control PDU" },
    { 0, NULL }
};
static value_string_ext llid_codes_vals_ext = VALUE_STRING_EXT_INIT(llid_codes_vals);

static const value_string control_opcode_vals[] = {
    { 0x00, "LL_CONNECTION_UPDATE_REQ" },
    { 0x01, "LL_CHANNEL_MAP_REQ" },
    { 0x02, "LL_TERMINATE_IND" },
    { 0x03, "LL_ENC_REQ" },
    { 0x04, "LL_ENC_RSP" },
    { 0x05, "LL_START_ENC_REQ" },
    { 0x06, "LL_START_ENC_RSP" },
    { 0x07, "LL_UNKNOWN_RSP" },
    { 0x08, "LL_FEATURE_REQ" },
    { 0x09, "LL_FEATURE_RSP" },
    { 0x0A, "LL_PAUSE_ENC_REQ" },
    { 0x0B, "LL_PAUSE_ENC_RSP" },
    { 0x0C, "LL_VERSION_IND" },
    { 0x0D, "LL_REJECT_IND" },
    { 0x0E, "LL_SLAVE_FEATURE_REQ" },
    { 0x0F, "LL_CONNECTION_PARAM_REQ" },
    { 0x10, "LL_CONNECTION_PARAM_RSP" },
    { 0x11, "LL_REJECT_IND_EXT" },
    { 0x12, "LL_PING_REQ" },
    { 0x13, "LL_PING_RSP" },
    { 0x14, "LL_LENGTH_REQ" },
    { 0x15, "LL_LENGTH_RSP" },
    { 0x16, "LL_PHY_REQ" },
    { 0x17, "LL_PHY_RSP" },
    { 0x18, "LL_PHY_UPDATE_IND" },
    { 0x19, "LL_MIN_USED_CHANNELS_IND" },
    { 0, NULL }
};
static value_string_ext control_opcode_vals_ext = VALUE_STRING_EXT_INIT(control_opcode_vals);

/* Taken from https://www.bluetooth.org/en-us/specification/assigned-numbers/link-layer */
static const value_string ll_version_number_vals[] = {
    { 0x06, "4.0"},
    { 0x07, "4.1" },
    { 0x08, "4.2" },
    { 0x09, "5.0" },
    { 0, NULL }
};
static value_string_ext ll_version_number_vals_ext = VALUE_STRING_EXT_INIT(ll_version_number_vals);

static const true_false_string tfs_ch_sel = {
    "#2",
    "#1"
};

static const true_false_string tfs_random_public = {
    "Random",
    "Public"
};

void proto_register_btle(void);
void proto_reg_handoff_btle(void);

static gboolean btle_detect_retransmit = TRUE;

static void
btle_init(void)
{
    l2cap_index = 0;
}

/*
 * Implements Bluetooth Vol 6, Part B, Section 3.1.1 (ref Figure 3.2)
 *
 * At entry: tvb is entire BTLE packet without preamble
 *           payload_len is the Length field from the BTLE PDU header
 *           crc_init as defined in the specifications
 *
 * This implementation operates on nibbles and is therefore
 * endian-neutral.
 */
static guint32
btle_crc(tvbuff_t *tvb, const guint8 payload_len, const guint32 crc_init)
{
    static const guint16 btle_crc_next_state_flips[256] = {
        0x0000, 0x32d8, 0x196c, 0x2bb4, 0x0cb6, 0x3e6e, 0x15da, 0x2702,
        0x065b, 0x3483, 0x1f37, 0x2def, 0x0aed, 0x3835, 0x1381, 0x2159,
        0x065b, 0x3483, 0x1f37, 0x2def, 0x0aed, 0x3835, 0x1381, 0x2159,
        0x0000, 0x32d8, 0x196c, 0x2bb4, 0x0cb6, 0x3e6e, 0x15da, 0x2702,
        0x0cb6, 0x3e6e, 0x15da, 0x2702, 0x0000, 0x32d8, 0x196c, 0x2bb4,
        0x0aed, 0x3835, 0x1381, 0x2159, 0x065b, 0x3483, 0x1f37, 0x2def,
        0x0aed, 0x3835, 0x1381, 0x2159, 0x065b, 0x3483, 0x1f37, 0x2def,
        0x0cb6, 0x3e6e, 0x15da, 0x2702, 0x0000, 0x32d8, 0x196c, 0x2bb4,
        0x196c, 0x2bb4, 0x0000, 0x32d8, 0x15da, 0x2702, 0x0cb6, 0x3e6e,
        0x1f37, 0x2def, 0x065b, 0x3483, 0x1381, 0x2159, 0x0aed, 0x3835,
        0x1f37, 0x2def, 0x065b, 0x3483, 0x1381, 0x2159, 0x0aed, 0x3835,
        0x196c, 0x2bb4, 0x0000, 0x32d8, 0x15da, 0x2702, 0x0cb6, 0x3e6e,
        0x15da, 0x2702, 0x0cb6, 0x3e6e, 0x196c, 0x2bb4, 0x0000, 0x32d8,
        0x1381, 0x2159, 0x0aed, 0x3835, 0x1f37, 0x2def, 0x065b, 0x3483,
        0x1381, 0x2159, 0x0aed, 0x3835, 0x1f37, 0x2def, 0x065b, 0x3483,
        0x15da, 0x2702, 0x0cb6, 0x3e6e, 0x196c, 0x2bb4, 0x0000, 0x32d8,
        0x32d8, 0x0000, 0x2bb4, 0x196c, 0x3e6e, 0x0cb6, 0x2702, 0x15da,
        0x3483, 0x065b, 0x2def, 0x1f37, 0x3835, 0x0aed, 0x2159, 0x1381,
        0x3483, 0x065b, 0x2def, 0x1f37, 0x3835, 0x0aed, 0x2159, 0x1381,
        0x32d8, 0x0000, 0x2bb4, 0x196c, 0x3e6e, 0x0cb6, 0x2702, 0x15da,
        0x3e6e, 0x0cb6, 0x2702, 0x15da, 0x32d8, 0x0000, 0x2bb4, 0x196c,
        0x3835, 0x0aed, 0x2159, 0x1381, 0x3483, 0x065b, 0x2def, 0x1f37,
        0x3835, 0x0aed, 0x2159, 0x1381, 0x3483, 0x065b, 0x2def, 0x1f37,
        0x3e6e, 0x0cb6, 0x2702, 0x15da, 0x32d8, 0x0000, 0x2bb4, 0x196c,
        0x2bb4, 0x196c, 0x32d8, 0x0000, 0x2702, 0x15da, 0x3e6e, 0x0cb6,
        0x2def, 0x1f37, 0x3483, 0x065b, 0x2159, 0x1381, 0x3835, 0x0aed,
        0x2def, 0x1f37, 0x3483, 0x065b, 0x2159, 0x1381, 0x3835, 0x0aed,
        0x2bb4, 0x196c, 0x32d8, 0x0000, 0x2702, 0x15da, 0x3e6e, 0x0cb6,
        0x2702, 0x15da, 0x3e6e, 0x0cb6, 0x2bb4, 0x196c, 0x32d8, 0x0000,
        0x2159, 0x1381, 0x3835, 0x0aed, 0x2def, 0x1f37, 0x3483, 0x065b,
        0x2159, 0x1381, 0x3835, 0x0aed, 0x2def, 0x1f37, 0x3483, 0x065b,
        0x2702, 0x15da, 0x3e6e, 0x0cb6, 0x2bb4, 0x196c, 0x32d8, 0x0000
    };
    gint    offset = 4; /* skip AA, CRC applies over PDU */
    guint32 state = crc_init;
    guint8  bytes_to_go = 2+payload_len; /* PDU includes header and payload */
    while( bytes_to_go-- ) {
        guint8 byte   = tvb_get_guint8(tvb, offset++);
        guint8 nibble = (byte & 0xf);
        guint8 byte_index  = ((state >> 16) & 0xf0) | nibble;
        state  = ((state << 4) ^ btle_crc_next_state_flips[byte_index]) & 0xffffff;
        nibble = ((byte >> 4) & 0xf);
        byte_index  = ((state >> 16) & 0xf0) | nibble;
        state  = ((state << 4) ^ btle_crc_next_state_flips[byte_index]) & 0xffffff;
    }
    return state;
}

/*
 * Reverses the bits in each byte of a 32-bit word.
 *
 * Needed because CRCs are transmitted in bit-reversed order compared
 * to the rest of the BTLE packet.  See BT spec, Vol 6, Part B,
 * Section 1.2.
 */
static guint32
reverse_bits_per_byte(const guint32 val)
{
    const guint8 nibble_rev[16] = {
        0x0, 0x8, 0x4, 0xc, 0x2, 0xa, 0x6, 0xe,
        0x1, 0x9, 0x5, 0xd, 0x3, 0xb, 0x7, 0xf
    };
    guint32 retval = 0;
    unsigned byte_index;
    for (byte_index=0; byte_index<4; byte_index++) {
        guint shiftA = byte_index*8;
        guint shiftB = shiftA+4;
        retval |= (nibble_rev[((val >> shiftA) & 0xf)] << shiftB);
        retval |= (nibble_rev[((val >> shiftB) & 0xf)] << shiftA);
    }
    return retval;
}

static gint
dissect_btle(tvbuff_t *tvb, packet_info *pinfo, proto_tree *tree, void *data)
{
    proto_item           *btle_item;
    proto_tree           *btle_tree;
    proto_item           *sub_item;
    proto_tree           *sub_tree;
    gint                  offset = 0;
    guint32               access_address, length;
    tvbuff_t              *next_tvb;
    guint8                *dst_bd_addr;
    guint8                *src_bd_addr;
    static const guint8    broadcast_addr[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    connection_info_t     *connection_info = NULL;
    wmem_tree_t           *wmem_tree;
    wmem_tree_key_t        key[5];
    guint32                interface_id;
    guint32                adapter_id;
    guint32                connection_access_address;
    guint32                frame_number;
    enum {CRC_INDETERMINATE,
          CRC_CAN_BE_CALCULATED,
          CRC_INCORRECT,
          CRC_CORRECT} crc_status = CRC_INDETERMINATE;
    guint32      crc_init = 0x555555; /* default to advertising channel's value */
    guint32      packet_crc;
    const btle_context_t  *btle_context   = NULL;
    bluetooth_data_t      *bluetooth_data = NULL;
    ubertooth_data_t      *ubertooth_data = NULL;
    gint                   previous_proto;
    wmem_list_frame_t     *list_data;
    proto_item            *item;
    guint                  window_size;
    guint                  window_offset;
    guint                  data_interval;
    guint                  data_timeout;

    list_data = wmem_list_frame_prev(wmem_list_tail(pinfo->layers));
    if (list_data) {
        previous_proto = GPOINTER_TO_INT(wmem_list_frame_data(list_data));

        if ((previous_proto == proto_btle_rf)||(previous_proto == proto_nordic_ble)) {
            btle_context = (const btle_context_t *) data;
            bluetooth_data = btle_context->previous_protocol_data.bluetooth_data;
        } else if (previous_proto == proto_bluetooth) {
            bluetooth_data = (bluetooth_data_t *) data;
        }

        if (bluetooth_data && bluetooth_data->previous_protocol_data_type == BT_PD_UBERTOOTH_DATA) {
            ubertooth_data = bluetooth_data->previous_protocol_data.ubertooth_data;
        }
    }

    src_bd_addr = (gchar *) wmem_alloc(pinfo->pool, 6);
    dst_bd_addr = (gchar *) wmem_alloc(pinfo->pool, 6);

    if (btle_context && btle_context->crc_checked_at_capture) {
        crc_status = btle_context->crc_valid_at_capture ? CRC_CORRECT : CRC_INCORRECT;
    }

    col_set_str(pinfo->cinfo, COL_PROTOCOL, "LE LL");

    btle_item = proto_tree_add_item(tree, proto_btle, tvb, offset, -1, ENC_NA);
    btle_tree = proto_item_add_subtree(btle_item, ett_btle);

    sub_item = proto_tree_add_item(btle_tree, hf_access_address, tvb, offset, 4, ENC_LITTLE_ENDIAN);
    access_address = tvb_get_letohl(tvb, offset);
    if (btle_context) {
        switch(btle_context->aa_category) {
        case E_AA_MATCHED:
            expert_add_info(pinfo, sub_item, &ei_access_address_matched);
            break;
        case E_AA_ILLEGAL:
            expert_add_info(pinfo, sub_item, &ei_access_address_illegal);
            break;
        case E_AA_BIT_ERRORS:
            expert_add_info(pinfo, sub_item, &ei_access_address_bit_errors);
            break;
        default:
            break;
        }
    }
    offset += 4;

    if (bluetooth_data)
        interface_id = bluetooth_data->interface_id;
    else if (pinfo->phdr->presence_flags & WTAP_HAS_INTERFACE_ID)
        interface_id = pinfo->phdr->interface_id;
    else
        interface_id = HCI_INTERFACE_DEFAULT;

    if (ubertooth_data)
        adapter_id = ubertooth_data->bus_id << 8 | ubertooth_data->device_address;
    else if (bluetooth_data)
        adapter_id = bluetooth_data->adapter_id;
    else
        adapter_id = HCI_ADAPTER_DEFAULT;

    frame_number = pinfo->num;

    if (access_address == ACCESS_ADDRESS_ADVERTISING) {
        proto_item  *advertising_header_item;
        proto_tree  *advertising_header_tree;
        proto_item  *link_layer_data_item;
        proto_tree  *link_layer_data_tree;
        guint8       pdu_type, ch_sel;

        if (crc_status == CRC_INDETERMINATE) {
            /* Advertising channel CRCs can aways be calculated, because CRCInit is always known. */
            crc_status = CRC_CAN_BE_CALCULATED;
        }

        advertising_header_item = proto_tree_add_item(btle_tree, hf_advertising_header, tvb, offset, 2, ENC_LITTLE_ENDIAN);
        advertising_header_tree = proto_item_add_subtree(advertising_header_item, ett_advertising_header);

        pdu_type = tvb_get_guint8(tvb, offset) & 0x0F;
        ch_sel = (tvb_get_guint8(tvb, offset) & 0x20) >> 5;
        proto_item_append_text(advertising_header_item, " (PDU Type: %s, ChSel: %s, TxAdd: %s",
                               val_to_str_ext_const(pdu_type, &pdu_type_vals_ext, "Unknown"),
                               (ch_sel & 0x01) ? tfs_ch_sel.true_string : tfs_ch_sel.false_string,
                               (tvb_get_guint8(tvb, offset) & 0x40) ? tfs_random_public.true_string : tfs_random_public.false_string);
        proto_tree_add_item(advertising_header_tree, hf_advertising_header_pdu_type, tvb, offset, 1, ENC_LITTLE_ENDIAN);
        proto_tree_add_item(advertising_header_tree, hf_advertising_header_rfu_1, tvb, offset, 1, ENC_LITTLE_ENDIAN);
        proto_tree_add_item(advertising_header_tree, hf_advertising_header_ch_sel, tvb, offset, 1, ENC_LITTLE_ENDIAN);
        proto_tree_add_item(advertising_header_tree, hf_advertising_header_randomized_tx, tvb, offset, 1, ENC_LITTLE_ENDIAN);
        switch (pdu_type) {
        case 0x00: /* ADV_IND */
        case 0x02: /* ADV_NONCONN_IND */
        case 0x04: /* SCAN_RSP */
        case 0x06: /* ADV_SCAN_IND */
            proto_tree_add_item(advertising_header_tree, hf_advertising_header_reserved, tvb, offset, 1, ENC_LITTLE_ENDIAN);
            break;
        default:
            proto_tree_add_item(advertising_header_tree, hf_advertising_header_randomized_rx, tvb, offset, 1, ENC_LITTLE_ENDIAN);
            proto_item_append_text(advertising_header_item, ", RxAdd: %s",
                                   (tvb_get_guint8(tvb, offset) & 0x80) ? tfs_random_public.true_string : tfs_random_public.false_string);
        }
        proto_item_append_text(advertising_header_item, ")");
        offset += 1;

        col_set_str(pinfo->cinfo, COL_INFO, val_to_str_ext_const(pdu_type, &pdu_type_vals_ext, "Unknown"));

        proto_tree_add_item(advertising_header_tree, hf_advertising_header_length, tvb, offset, 1, ENC_LITTLE_ENDIAN);
        item = proto_tree_add_item_ret_uint(btle_tree, hf_length, tvb, offset, 1, ENC_LITTLE_ENDIAN, &length);
        PROTO_ITEM_SET_HIDDEN(item);
        offset += 1;

        switch (pdu_type) {
        case 0x00: /* ADV_IND */
        case 0x02: /* ADV_NONCONN_IND */
        case 0x06: /* ADV_SCAN_IND */
            offset = dissect_bd_addr(hf_advertising_address, pinfo, btle_tree, tvb, offset, TRUE, interface_id, adapter_id, src_bd_addr);

            set_address(&pinfo->net_src, AT_ETHER, 6, src_bd_addr);
            copy_address_shallow(&pinfo->dl_src, &pinfo->net_src);
            copy_address_shallow(&pinfo->src, &pinfo->net_src);

            set_address(&pinfo->net_dst, AT_ETHER, 6, broadcast_addr);
            copy_address_shallow(&pinfo->dl_dst, &pinfo->net_dst);
            copy_address_shallow(&pinfo->dst, &pinfo->net_dst);

            if (!pinfo->fd->flags.visited) {
                address *addr;

                addr = (address *) wmem_memdup(wmem_file_scope(), &pinfo->dl_src, sizeof(address));
                addr->data =  wmem_memdup(wmem_file_scope(), pinfo->dl_src.data, pinfo->dl_src.len);
                p_add_proto_data(wmem_file_scope(), pinfo, proto_bluetooth, BLUETOOTH_DATA_SRC, addr);

                addr = (address *) wmem_memdup(wmem_file_scope(), &pinfo->dl_dst, sizeof(address));
                addr->data =  wmem_memdup(wmem_file_scope(), pinfo->dl_dst.data, pinfo->dl_dst.len);
                p_add_proto_data(wmem_file_scope(), pinfo, proto_bluetooth, BLUETOOTH_DATA_DST, addr);
            }

            if (tvb_reported_length_remaining(tvb, offset) > 3) {
                bluetooth_eir_ad_data_t *ad_data = wmem_new0(wmem_packet_scope(), bluetooth_eir_ad_data_t);
                ad_data->interface_id = interface_id;
                ad_data->adapter_id = adapter_id;
                next_tvb = tvb_new_subset_length(tvb, offset, tvb_reported_length_remaining(tvb, offset) - 3);
                call_dissector_with_data(btcommon_ad_handle, next_tvb, pinfo, btle_tree, ad_data);
            }

            offset += tvb_reported_length_remaining(tvb, offset) - 3;

            break;
        case 0x01: /* ADV_DIRECT_IND */
            offset = dissect_bd_addr(hf_advertising_address, pinfo, btle_tree, tvb, offset, TRUE, interface_id, adapter_id, src_bd_addr);
            offset = dissect_bd_addr(hf_initiator_addresss, pinfo, btle_tree, tvb, offset, FALSE, interface_id, adapter_id, dst_bd_addr);

            set_address(&pinfo->net_src, AT_ETHER, 6, src_bd_addr);
            copy_address_shallow(&pinfo->dl_src, &pinfo->net_src);
            copy_address_shallow(&pinfo->src, &pinfo->net_src);

            set_address(&pinfo->net_dst, AT_ETHER, 6, dst_bd_addr);
            copy_address_shallow(&pinfo->dl_dst, &pinfo->net_dst);
            copy_address_shallow(&pinfo->dst, &pinfo->net_dst);

            if (!pinfo->fd->flags.visited) {
                address *addr;

                addr = (address *) wmem_memdup(wmem_file_scope(), &pinfo->dl_src, sizeof(address));
                addr->data =  wmem_memdup(wmem_file_scope(), pinfo->dl_src.data, pinfo->dl_src.len);
                p_add_proto_data(wmem_file_scope(), pinfo, proto_bluetooth, BLUETOOTH_DATA_SRC, addr);

                addr = (address *) wmem_memdup(wmem_file_scope(), &pinfo->dl_dst, sizeof(address));
                addr->data =  wmem_memdup(wmem_file_scope(), pinfo->dl_dst.data, pinfo->dl_dst.len);
                p_add_proto_data(wmem_file_scope(), pinfo, proto_bluetooth, BLUETOOTH_DATA_DST, addr);
            }

            break;
        case 0x03: /* SCAN_REQ */
            offset = dissect_bd_addr(hf_scanning_address, pinfo, btle_tree, tvb, offset, TRUE, interface_id, adapter_id, src_bd_addr);
            offset = dissect_bd_addr(hf_advertising_address, pinfo, btle_tree, tvb, offset, FALSE, interface_id, adapter_id, dst_bd_addr);

            set_address(&pinfo->net_src, AT_ETHER, 6, src_bd_addr);
            copy_address_shallow(&pinfo->dl_src, &pinfo->net_src);
            copy_address_shallow(&pinfo->src, &pinfo->net_src);

            set_address(&pinfo->net_dst, AT_ETHER, 6, dst_bd_addr);
            copy_address_shallow(&pinfo->dl_dst, &pinfo->net_dst);
            copy_address_shallow(&pinfo->dst, &pinfo->net_dst);

            if (!pinfo->fd->flags.visited) {
                address *addr;

                addr = (address *) wmem_memdup(wmem_file_scope(), &pinfo->dl_src, sizeof(address));
                addr->data =  wmem_memdup(wmem_file_scope(), pinfo->dl_src.data, pinfo->dl_src.len);
                p_add_proto_data(wmem_file_scope(), pinfo, proto_bluetooth, BLUETOOTH_DATA_SRC, addr);

                addr = (address *) wmem_memdup(wmem_file_scope(), &pinfo->dl_dst, sizeof(address));
                addr->data =  wmem_memdup(wmem_file_scope(), pinfo->dl_dst.data, pinfo->dl_dst.len);
                p_add_proto_data(wmem_file_scope(), pinfo, proto_bluetooth, BLUETOOTH_DATA_DST, addr);
            }

            break;
        case 0x04: /* SCAN_RSP */
            offset = dissect_bd_addr(hf_advertising_address, pinfo, btle_tree, tvb, offset, TRUE, interface_id, adapter_id, src_bd_addr);

            set_address(&pinfo->net_src, AT_ETHER, 6, src_bd_addr);
            copy_address_shallow(&pinfo->dl_src, &pinfo->net_src);
            copy_address_shallow(&pinfo->src, &pinfo->net_src);

            set_address(&pinfo->net_dst, AT_ETHER, 6, broadcast_addr);
            copy_address_shallow(&pinfo->dl_dst, &pinfo->net_dst);
            copy_address_shallow(&pinfo->dst, &pinfo->net_dst);

            if (!pinfo->fd->flags.visited) {
                address *addr;

                addr = (address *) wmem_memdup(wmem_file_scope(), &pinfo->dl_src, sizeof(address));
                addr->data =  wmem_memdup(wmem_file_scope(), pinfo->dl_src.data, pinfo->dl_src.len);
                p_add_proto_data(wmem_file_scope(), pinfo, proto_bluetooth, BLUETOOTH_DATA_SRC, addr);

                addr = (address *) wmem_memdup(wmem_file_scope(), &pinfo->dl_dst, sizeof(address));
                addr->data =  wmem_memdup(wmem_file_scope(), pinfo->dl_dst.data, pinfo->dl_dst.len);
                p_add_proto_data(wmem_file_scope(), pinfo, proto_bluetooth, BLUETOOTH_DATA_DST, addr);
            }

            sub_item = proto_tree_add_item(btle_tree, hf_scan_response_data, tvb, offset, tvb_reported_length_remaining(tvb, offset) - 3, ENC_NA);
            sub_tree = proto_item_add_subtree(sub_item, ett_scan_response_data);

            if (tvb_reported_length_remaining(tvb, offset) > 3) {
                bluetooth_eir_ad_data_t *ad_data = wmem_new0(wmem_packet_scope(), bluetooth_eir_ad_data_t);
                ad_data->interface_id = interface_id;
                ad_data->adapter_id = adapter_id;
                next_tvb = tvb_new_subset_length(tvb, offset, tvb_reported_length_remaining(tvb, offset) - 3);
                call_dissector_with_data(btcommon_ad_handle, next_tvb, pinfo, sub_tree, ad_data);
            }

            offset += tvb_reported_length_remaining(tvb, offset) - 3;

            break;
        case 0x05: /* CONNECT_REQ */
            offset = dissect_bd_addr(hf_initiator_addresss, pinfo, btle_tree, tvb, offset, FALSE, interface_id, adapter_id, src_bd_addr);
            offset = dissect_bd_addr(hf_advertising_address, pinfo, btle_tree, tvb, offset, TRUE, interface_id, adapter_id, dst_bd_addr);

            set_address(&pinfo->net_src, AT_ETHER, 6, src_bd_addr);
            copy_address_shallow(&pinfo->dl_src, &pinfo->net_src);
            copy_address_shallow(&pinfo->src, &pinfo->net_src);

            set_address(&pinfo->net_dst, AT_ETHER, 6, dst_bd_addr);
            copy_address_shallow(&pinfo->dl_dst, &pinfo->net_dst);
            copy_address_shallow(&pinfo->dst, &pinfo->net_dst);

            if (!pinfo->fd->flags.visited) {
                address *addr;

                addr = (address *) wmem_memdup(wmem_file_scope(), &pinfo->dl_src, sizeof(address));
                addr->data =  wmem_memdup(wmem_file_scope(), pinfo->dl_src.data, pinfo->dl_src.len);
                p_add_proto_data(wmem_file_scope(), pinfo, proto_bluetooth, BLUETOOTH_DATA_SRC, addr);

                addr = (address *) wmem_memdup(wmem_file_scope(), &pinfo->dl_dst, sizeof(address));
                addr->data =  wmem_memdup(wmem_file_scope(), pinfo->dl_dst.data, pinfo->dl_dst.len);
                p_add_proto_data(wmem_file_scope(), pinfo, proto_bluetooth, BLUETOOTH_DATA_DST, addr);
            }

            link_layer_data_item = proto_tree_add_item(btle_tree, hf_link_layer_data, tvb, offset, 22, ENC_NA);
            link_layer_data_tree = proto_item_add_subtree(link_layer_data_item, ett_link_layer_data);

            proto_tree_add_item(link_layer_data_tree, hf_link_layer_data_access_address, tvb, offset, 4, ENC_LITTLE_ENDIAN);
            connection_access_address = tvb_get_letohl(tvb, offset);
            offset += 4;

            proto_tree_add_item(link_layer_data_tree, hf_link_layer_data_crc_init, tvb, offset, 3, ENC_LITTLE_ENDIAN);
            offset += 3;

            item = proto_tree_add_item_ret_uint(link_layer_data_tree, hf_link_layer_data_window_size, tvb, offset, 1, ENC_LITTLE_ENDIAN, &window_size);
            proto_item_append_text(item, " (%g msec)", window_size*1.25);
            offset += 1;

            item = proto_tree_add_item_ret_uint(link_layer_data_tree, hf_link_layer_data_window_offset, tvb, offset, 2, ENC_LITTLE_ENDIAN, &window_offset);
            proto_item_append_text(item, " (%g msec)", window_offset*1.25);
            offset += 2;

            item = proto_tree_add_item_ret_uint(link_layer_data_tree, hf_link_layer_data_interval, tvb, offset, 2, ENC_LITTLE_ENDIAN, &data_interval);
            proto_item_append_text(item, " (%g msec)", data_interval*1.25);
            offset += 2;

            proto_tree_add_item(link_layer_data_tree, hf_link_layer_data_latency, tvb, offset, 2, ENC_LITTLE_ENDIAN);
            offset += 2;

            item = proto_tree_add_item_ret_uint(link_layer_data_tree, hf_link_layer_data_timeout, tvb, offset, 2, ENC_LITTLE_ENDIAN, &data_timeout);
            proto_item_append_text(item, " (%g msec)", data_timeout*1.25);
            offset += 2;

            sub_item = proto_tree_add_item(link_layer_data_tree, hf_link_layer_data_channel_map, tvb, offset, 5, ENC_NA);
            sub_tree = proto_item_add_subtree(sub_item, ett_channel_map);

            call_dissector(btcommon_le_channel_map_handle, tvb_new_subset_length(tvb, offset, 5), pinfo, sub_tree);
            offset += 5;

            proto_tree_add_item(link_layer_data_tree, hf_link_layer_data_hop, tvb, offset, 1, ENC_LITTLE_ENDIAN);
            proto_tree_add_item(link_layer_data_tree, hf_link_layer_data_sleep_clock_accuracy, tvb, offset, 1, ENC_LITTLE_ENDIAN);
            offset += 1;

            if (!pinfo->fd->flags.visited) {
                key[0].length = 1;
                key[0].key = &interface_id;
                key[1].length = 1;
                key[1].key = &adapter_id;
                key[2].length = 1;
                key[2].key = &connection_access_address;
                key[3].length = 1;
                key[3].key = &frame_number;
                key[4].length = 0;
                key[4].key = NULL;

                connection_info = wmem_new0(wmem_file_scope(), connection_info_t);
                connection_info->interface_id   = interface_id;
                connection_info->adapter_id     = adapter_id;
                connection_info->access_address = connection_access_address;

                memcpy(connection_info->master_bd_addr, src_bd_addr, 6);
                memcpy(connection_info->slave_bd_addr,  dst_bd_addr, 6);

                wmem_tree_insert32_array(connection_info_tree, key, connection_info);
            }

            break;
        default:
            if (tvb_reported_length_remaining(tvb, offset) > 3) {
                proto_tree_add_expert(btle_tree, pinfo, &ei_unknown_data, tvb, offset, tvb_reported_length_remaining(tvb, offset) - 3);
                offset += tvb_reported_length_remaining(tvb, offset) - 3;
            }
        }
    } else { /* data PDU */
        proto_item  *data_header_item, *seq_item;
        proto_tree  *data_header_tree;
        guint8       oct;
        guint8       llid;
        guint8       control_opcode;
        guint32      direction = BTLE_DIR_UNKNOWN;
        gboolean     add_l2cap_index = FALSE;
        gboolean     retransmit = FALSE;

        if (btle_context) {
            direction = btle_context->direction;
        }

        btle_frame_info_t *btle_frame_info = NULL;
        fragment_head *frag_btl2cap_msg = NULL;
        btle_frame_info_t empty_btle_frame_info = {0, 0, 0, 0};

        key[0].length = 1;
        key[0].key = &interface_id;
        key[1].length = 1;
        key[1].key = &adapter_id;
        key[2].length = 1;
        key[2].key = &access_address;
        key[3].length = 0;
        key[3].key = NULL;

        oct = tvb_get_guint8(tvb, offset);
        wmem_tree = (wmem_tree_t *) wmem_tree_lookup32_array(connection_info_tree, key);
        if (wmem_tree) {
            connection_info = (connection_info_t *) wmem_tree_lookup32_le(wmem_tree, pinfo->num);
            if (connection_info) {
                gchar  *str_addr_src, *str_addr_dst;
                /* Holds "unknown" + access_address + NULL, which is the longest string */
                int     str_addr_len = 18 + 1;

                str_addr_src = (gchar *) wmem_alloc(pinfo->pool, str_addr_len);
                str_addr_dst = (gchar *) wmem_alloc(pinfo->pool, str_addr_len);

                sub_item = proto_tree_add_ether(btle_tree, hf_master_bd_addr, tvb, 0, 0, connection_info->master_bd_addr);
                PROTO_ITEM_SET_GENERATED(sub_item);

                sub_item = proto_tree_add_ether(btle_tree, hf_slave_bd_addr, tvb, 0, 0, connection_info->slave_bd_addr);
                PROTO_ITEM_SET_GENERATED(sub_item);

                switch (direction) {
                case BTLE_DIR_MASTER_SLAVE:
                    g_snprintf(str_addr_src, str_addr_len, "Master_0x%08x", connection_info->access_address);
                    g_snprintf(str_addr_dst, str_addr_len, "Slave_0x%08x", connection_info->access_address);
                    set_address(&pinfo->dl_src, AT_ETHER, sizeof(connection_info->master_bd_addr), connection_info->master_bd_addr);
                    set_address(&pinfo->dl_dst, AT_ETHER, sizeof(connection_info->slave_bd_addr), connection_info->slave_bd_addr);
                    break;
                case BTLE_DIR_SLAVE_MASTER:
                    g_snprintf(str_addr_src, str_addr_len, "Slave_0x%08x", connection_info->access_address);
                    g_snprintf(str_addr_dst, str_addr_len, "Master_0x%08x", connection_info->access_address);
                    set_address(&pinfo->dl_src, AT_ETHER, sizeof(connection_info->slave_bd_addr), connection_info->slave_bd_addr);
                    set_address(&pinfo->dl_dst, AT_ETHER, sizeof(connection_info->master_bd_addr), connection_info->master_bd_addr);
                    break;
                default:
                    /* BTLE_DIR_UNKNOWN */
                    g_snprintf(str_addr_src, str_addr_len, "Unknown_0x%08x", connection_info->access_address);
                    g_snprintf(str_addr_dst, str_addr_len, "Unknown_0x%08x", connection_info->access_address);
                    clear_address(&pinfo->dl_src);
                    clear_address(&pinfo->dl_dst);
                    break;
                }

                set_address(&pinfo->net_src, AT_STRINGZ, (int)strlen(str_addr_src)+1, str_addr_src);
                copy_address_shallow(&pinfo->src, &pinfo->net_src);

                set_address(&pinfo->net_dst, AT_STRINGZ, (int)strlen(str_addr_dst)+1, str_addr_dst);
                copy_address_shallow(&pinfo->dst, &pinfo->net_dst);

                if (!pinfo->fd->flags.visited) {
                    address *addr;

                    btle_frame_info = wmem_new0(wmem_file_scope(), btle_frame_info_t);
                    btle_frame_info->l2cap_index = connection_info->direction_info[direction].l2cap_index;

                    addr = (address *) wmem_memdup(wmem_file_scope(), &pinfo->dl_src, sizeof(address));
                    addr->data =  wmem_memdup(wmem_file_scope(), pinfo->dl_src.data, pinfo->dl_src.len);
                    p_add_proto_data(wmem_file_scope(), pinfo, proto_bluetooth, BLUETOOTH_DATA_SRC, addr);

                    addr = (address *) wmem_memdup(wmem_file_scope(), &pinfo->dl_dst, sizeof(address));
                    addr->data =  wmem_memdup(wmem_file_scope(), pinfo->dl_dst.data, pinfo->dl_dst.len);
                    p_add_proto_data(wmem_file_scope(), pinfo, proto_bluetooth, BLUETOOTH_DATA_DST, addr);

                    if (!connection_info->first_data_frame_seen) {
                        connection_info->first_data_frame_seen = 1;
                        btle_frame_info->retransmit = 0;
                        connection_info->direction_info[BTLE_DIR_MASTER_SLAVE].prev_seq_num = 0;
                        connection_info->direction_info[BTLE_DIR_SLAVE_MASTER].prev_seq_num = 1;
                    }
                    else {
                        guint8 seq_num = !!(oct & 0x8);

                        if (seq_num != connection_info->direction_info[direction].prev_seq_num) {
                            /* SN is not equal to previous packet (in same direction) SN */
                            btle_frame_info->retransmit = 0;
                        }
                        else {
                            btle_frame_info->retransmit = 1;
                        }
                        connection_info->direction_info[direction].prev_seq_num = seq_num;
                    }
                    p_add_proto_data(wmem_file_scope(), pinfo, proto_btle, pinfo->curr_layer_num, btle_frame_info);
                }
                else {
                    /* Not the first pass */
                    btle_frame_info = (btle_frame_info_t *)p_get_proto_data(wmem_file_scope(), pinfo, proto_btle, pinfo->curr_layer_num);
                }
            }
        }

        if (btle_frame_info == NULL) {
            btle_frame_info = &empty_btle_frame_info;
        }
        data_header_item = proto_tree_add_item(btle_tree, hf_data_header, tvb, offset, 2, ENC_LITTLE_ENDIAN);
        data_header_tree = proto_item_add_subtree(data_header_item, ett_data_header);

        proto_tree_add_item(data_header_tree, hf_data_header_llid, tvb, offset, 1, ENC_LITTLE_ENDIAN);
        proto_tree_add_item(data_header_tree, hf_data_header_next_expected_sequence_number, tvb, offset, 1, ENC_LITTLE_ENDIAN);
        seq_item = proto_tree_add_item(data_header_tree, hf_data_header_sequence_number, tvb, offset, 1, ENC_LITTLE_ENDIAN);

        if (direction != BTLE_DIR_UNKNOWN) {
            /* Unable to check valid SN or retransmission without direction */
            if (btle_frame_info->retransmit == 0) {
                proto_item_append_text(seq_item, " [OK]");
            }
            else {
                proto_item_append_text(seq_item, " [Wrong]");
                if (btle_detect_retransmit) {
                    expert_add_info(pinfo, seq_item, &ei_retransmit);
                    retransmit = TRUE;
                }
            }
        }

        proto_tree_add_item(data_header_tree, hf_data_header_more_data, tvb, offset, 1, ENC_LITTLE_ENDIAN);
        proto_tree_add_item(data_header_tree, hf_data_header_rfu, tvb, offset, 1, ENC_LITTLE_ENDIAN);
        llid = tvb_get_guint8(tvb, offset) & 0x03;
        offset += 1;

        proto_tree_add_item(data_header_tree, hf_data_header_length, tvb, offset, 1, ENC_LITTLE_ENDIAN);
        item = proto_tree_add_item_ret_uint(btle_tree, hf_length, tvb, offset, 1, ENC_LITTLE_ENDIAN, &length);
        PROTO_ITEM_SET_HIDDEN(item);
        offset += 1;

        switch (llid) {
        case 0x01: /* Continuation fragment of an L2CAP message, or an Empty PDU */
            if (length > 0) {
                tvbuff_t *new_tvb = NULL;

                pinfo->fragmented = TRUE;
                if (connection_info && !retransmit) {
                    if (!pinfo->fd->flags.visited) {
                        if (connection_info->direction_info[direction].segmentation_started == 1) {
                            if (connection_info->direction_info[direction].segment_len_rem >= length) {
                                connection_info->direction_info[direction].segment_len_rem = connection_info->direction_info[direction].segment_len_rem - length;
                            } else {
                                /*
                                 * Missing fragment for previous L2CAP and fragment start for this.
                                 * Increase l2cap_index.
                                 */
                                btle_frame_info->missing_start = 1;
                                btle_frame_info->l2cap_index = l2cap_index;
                                l2cap_index++;
                            }
                            if (connection_info->direction_info[direction].segment_len_rem > 0) {
                                btle_frame_info->more_fragments = 1;
                            }
                            else {
                                btle_frame_info->more_fragments = 0;
                                connection_info->direction_info[direction].segmentation_started = 0;
                                connection_info->direction_info[direction].segment_len_rem = 0;
                            }
                        } else {
                            /*
                             * Missing fragment start.
                             * Set more_fragments and increase l2cap_index to avoid reassembly.
                             */
                            btle_frame_info->more_fragments = 1;
                            btle_frame_info->missing_start = 1;
                            btle_frame_info->l2cap_index = l2cap_index;
                            l2cap_index++;
                        }
                    }

                    add_l2cap_index = TRUE;

                    frag_btl2cap_msg = fragment_add_seq_next(&btle_l2cap_msg_reassembly_table,
                        tvb, offset,
                        pinfo,
                        btle_frame_info->l2cap_index,      /* guint32 ID for fragments belonging together */
                        NULL,                              /* data* */
                        length,                            /* Fragment length */
                        btle_frame_info->more_fragments);  /* More fragments */

                    new_tvb = process_reassembled_data(tvb, offset, pinfo,
                        "Reassembled L2CAP",
                        frag_btl2cap_msg,
                        &btle_l2cap_msg_frag_items,
                        NULL,
                        btle_tree);
                }

                if (new_tvb) {
                    bthci_acl_data_t  *acl_data;

                    col_set_str(pinfo->cinfo, COL_INFO, "L2CAP Data");

                    acl_data = wmem_new(wmem_packet_scope(), bthci_acl_data_t);
                    acl_data->interface_id = interface_id;
                    acl_data->adapter_id = adapter_id;
                    acl_data->chandle = 0; /* No connection handle at this layer */
                    acl_data->remote_bd_addr_oui = 0;
                    acl_data->remote_bd_addr_id = 0;
                    acl_data->is_btle = TRUE;
                    acl_data->is_btle_retransmit = retransmit;

                    next_tvb = tvb_new_subset_length(tvb, offset, length);
                    if (next_tvb) {
                        call_dissector_with_data(btl2cap_handle, new_tvb, pinfo, tree, acl_data);
                    }
                    offset += length;
                }
                else {
                    col_set_str(pinfo->cinfo, COL_INFO, "L2CAP Fragment");
                    item = proto_tree_add_item(btle_tree, hf_l2cap_fragment, tvb, offset, length, ENC_NA);
                    if (btle_frame_info->missing_start) {
                        expert_add_info(pinfo, item, &ei_missing_fragment_start);
                    }
                    offset += length;
                }
            } else {
                col_set_str(pinfo->cinfo, COL_INFO, "Empty PDU");
            }

            break;
        case 0x02: /* Start of an L2CAP message or a complete L2CAP message with no fragmentation */
            if (length > 0) {
                guint le_frame_len = tvb_get_letohs(tvb, offset);
                if (le_frame_len > length) {
                    pinfo->fragmented = TRUE;
                    if (connection_info && !retransmit) {
                        if (!pinfo->fd->flags.visited) {
                            connection_info->direction_info[direction].segmentation_started = 1;
                            /* The first two octets in the L2CAP PDU contain the length of the entire
                             * L2CAP PDU in octets, excluding the Length and CID fields(4 octets).
                             */
                            connection_info->direction_info[direction].segment_len_rem = le_frame_len + 4 - length;
                            connection_info->direction_info[direction].l2cap_index = l2cap_index;
                            btle_frame_info->more_fragments = 1;
                            btle_frame_info->l2cap_index = l2cap_index;
                            l2cap_index++;
                        }

                        add_l2cap_index = TRUE;

                        frag_btl2cap_msg = fragment_add_seq_next(&btle_l2cap_msg_reassembly_table,
                            tvb, offset,
                            pinfo,
                            btle_frame_info->l2cap_index,      /* guint32 ID for fragments belonging together */
                            NULL,                              /* data* */
                            length,                            /* Fragment length */
                            btle_frame_info->more_fragments);  /* More fragments */

                        process_reassembled_data(tvb, offset, pinfo,
                            "Reassembled L2CAP",
                            frag_btl2cap_msg,
                            &btle_l2cap_msg_frag_items,
                            NULL,
                            btle_tree);
                    }

                    col_set_str(pinfo->cinfo, COL_INFO, "L2CAP Fragment Start");
                    proto_tree_add_item(btle_tree, hf_l2cap_fragment, tvb, offset, length, ENC_NA);
                    offset += length;
                } else {
                    bthci_acl_data_t  *acl_data;
                    if (connection_info) {
                        /* Add a L2CAP index for completeness */
                        if (!pinfo->fd->flags.visited) {
                            btle_frame_info->l2cap_index = l2cap_index;
                            l2cap_index++;
                        }

                        add_l2cap_index = TRUE;
                    }

                    col_set_str(pinfo->cinfo, COL_INFO, "L2CAP Data");

                    acl_data = wmem_new(wmem_packet_scope(), bthci_acl_data_t);
                    acl_data->interface_id = interface_id;
                    acl_data->adapter_id   = adapter_id;
                    acl_data->chandle      = 0; /* No connection handle at this layer */
                    acl_data->remote_bd_addr_oui = 0;
                    acl_data->remote_bd_addr_id  = 0;
                    acl_data->is_btle = TRUE;
                    acl_data->is_btle_retransmit = retransmit;

                    next_tvb = tvb_new_subset_length(tvb, offset, length);
                    call_dissector_with_data(btl2cap_handle, next_tvb, pinfo, tree, acl_data);
                    offset += length;
                }
            }
            break;
        case 0x03: /* Control PDU */
            proto_tree_add_item(btle_tree, hf_control_opcode, tvb, offset, 1, ENC_LITTLE_ENDIAN);
            control_opcode = tvb_get_guint8(tvb, offset);
            offset += 1;

            col_add_fstr(pinfo->cinfo, COL_INFO, "Control Opcode: %s",
                    val_to_str_ext_const(control_opcode, &control_opcode_vals_ext, "Unknown"));

            switch (control_opcode) {
            case 0x05: /* LL_START_ENC_REQ */
            case 0x06: /* LL_START_ENC_RSP */
            case 0x0A: /* LL_PAUSE_ENC_REQ */
            case 0x0B: /* LL_PAUSE_ENC_RSP */
            case 0x12: /* LL_PING_REQ */
            case 0x13: /* LL_PING_RSP */
                if (tvb_reported_length_remaining(tvb, offset) > 3) {
                    proto_tree_add_expert(btle_tree, pinfo, &ei_unknown_data, tvb, offset, tvb_reported_length_remaining(tvb, offset) - 3);
                    offset += tvb_reported_length_remaining(tvb, offset) - 3;
                }

                break;
            case 0x00: /* LL_CONNECTION_UPDATE_REQ */
                proto_tree_add_item(btle_tree, hf_control_window_size, tvb, offset, 1, ENC_LITTLE_ENDIAN);
                offset += 1;

                proto_tree_add_item(btle_tree, hf_control_window_offset, tvb, offset, 2, ENC_LITTLE_ENDIAN);
                offset += 2;

                proto_tree_add_item(btle_tree, hf_control_interval, tvb, offset, 2, ENC_LITTLE_ENDIAN);
                offset += 2;

                proto_tree_add_item(btle_tree, hf_control_latency, tvb, offset, 2, ENC_LITTLE_ENDIAN);
                offset += 2;

                proto_tree_add_item(btle_tree, hf_control_timeout, tvb, offset, 2, ENC_LITTLE_ENDIAN);
                offset += 2;

                proto_tree_add_item(btle_tree, hf_control_instant, tvb, offset, 2, ENC_LITTLE_ENDIAN);
                offset += 2;

                break;
            case 0x01: /* LL_CHANNEL_MAP_REQ */
                sub_item = proto_tree_add_item(btle_tree, hf_control_channel_map, tvb, offset, 5, ENC_NA);
                sub_tree = proto_item_add_subtree(sub_item, ett_channel_map);

                call_dissector(btcommon_le_channel_map_handle, tvb_new_subset_length(tvb, offset, 5), pinfo, sub_tree);
                offset += 5;

                proto_tree_add_item(btle_tree, hf_control_instant, tvb, offset, 2, ENC_LITTLE_ENDIAN);
                offset += 2;

                break;
            case 0x02: /* LL_TERMINATE_IND */
            case 0x0D: /* LL_REJECT_IND */
                proto_tree_add_item(btle_tree, hf_control_error_code, tvb, offset, 1, ENC_LITTLE_ENDIAN);
                offset += 1;

                break;
            case 0x03: /* LL_ENC_REQ */
                proto_tree_add_item(btle_tree, hf_control_random_number, tvb, offset, 8, ENC_LITTLE_ENDIAN);
                offset += 8;

                proto_tree_add_item(btle_tree, hf_control_encrypted_diversifier, tvb, offset, 2, ENC_LITTLE_ENDIAN);
                offset += 2;

                proto_tree_add_item(btle_tree, hf_control_master_session_key_diversifier, tvb, offset, 8, ENC_LITTLE_ENDIAN);
                offset += 8;

                proto_tree_add_item(btle_tree, hf_control_master_session_initialization_vector, tvb, offset, 4, ENC_LITTLE_ENDIAN);
                offset += 4;

                break;
            case 0x04: /* LL_ENC_RSP */
                proto_tree_add_item(btle_tree, hf_control_slave_session_key_diversifier, tvb, offset, 8, ENC_LITTLE_ENDIAN);
                offset += 8;

                proto_tree_add_item(btle_tree, hf_control_slave_session_initialization_vector, tvb, offset, 4, ENC_LITTLE_ENDIAN);
                offset += 4;

                break;
            case 0x07: /* LL_UNKNOWN_RSP */
                proto_tree_add_item(btle_tree, hf_control_unknown_type, tvb, offset, 1, ENC_LITTLE_ENDIAN);
                offset += 1;

                break;
            case 0x08: /* LL_FEATURE_REQ */
            case 0x09: /* LL_FEATURE_RSP */
            case 0x0E: /* LL_SLAVE_FEATURE_REQ */
                sub_item = proto_tree_add_item(btle_tree, hf_control_feature_set, tvb, offset, 8, ENC_LITTLE_ENDIAN);
                sub_tree = proto_item_add_subtree(sub_item, ett_features);

                proto_tree_add_bitmask_list(sub_tree, tvb, offset, 1, hfx_control_feature_set_1, ENC_NA);
                offset += 1;

                proto_tree_add_bitmask_list(sub_tree, tvb, offset, 1, hfx_control_feature_set_2, ENC_NA);
                offset += 1;

                proto_tree_add_bitmask_list(sub_tree, tvb, offset, 1, hfx_control_feature_set_3, ENC_NA);
                offset += 1;

                proto_tree_add_item(sub_tree, hf_control_feature_set_reserved, tvb, offset, 5, ENC_NA);
                offset += 5;

                break;
            case 0x0C: /* LL_VERSION_IND */
                proto_tree_add_item(btle_tree, hf_control_version_number, tvb, offset, 1, ENC_LITTLE_ENDIAN);
                offset += 1;

                proto_tree_add_item(btle_tree, hf_control_company_id, tvb, offset, 2, ENC_LITTLE_ENDIAN);
                offset += 2;

                proto_tree_add_item(btle_tree, hf_control_subversion_number, tvb, offset, 2, ENC_LITTLE_ENDIAN);
                offset += 2;

                break;
            case 0x0F: /* LL_CONNECTION_PARAM_REQ */
            case 0x10: /* LL_CONNECTION_PARAM_RSP */
                proto_tree_add_item(btle_tree, hf_control_interval_min, tvb, offset, 2, ENC_LITTLE_ENDIAN);
                offset += 2;

                proto_tree_add_item(btle_tree, hf_control_interval_max, tvb, offset, 2, ENC_LITTLE_ENDIAN);
                offset += 2;

                proto_tree_add_item(btle_tree, hf_control_latency, tvb, offset, 2, ENC_LITTLE_ENDIAN);
                offset += 2;

                proto_tree_add_item(btle_tree, hf_control_timeout, tvb, offset, 2, ENC_LITTLE_ENDIAN);
                offset += 2;

                proto_tree_add_item(btle_tree, hf_control_preffered_periodicity, tvb, offset, 1, ENC_LITTLE_ENDIAN);
                offset += 1;

                proto_tree_add_item(btle_tree, hf_control_reference_connection_event_count, tvb, offset, 2, ENC_LITTLE_ENDIAN);
                offset += 2;

                proto_tree_add_item(btle_tree, hf_control_offset_0, tvb, offset, 2, ENC_LITTLE_ENDIAN);
                offset += 2;

                proto_tree_add_item(btle_tree, hf_control_offset_1, tvb, offset, 2, ENC_LITTLE_ENDIAN);
                offset += 2;

                proto_tree_add_item(btle_tree, hf_control_offset_2, tvb, offset, 2, ENC_LITTLE_ENDIAN);
                offset += 2;

                proto_tree_add_item(btle_tree, hf_control_offset_3, tvb, offset, 2, ENC_LITTLE_ENDIAN);
                offset += 2;

                proto_tree_add_item(btle_tree, hf_control_offset_4, tvb, offset, 2, ENC_LITTLE_ENDIAN);
                offset += 2;

                proto_tree_add_item(btle_tree, hf_control_offset_5, tvb, offset, 2, ENC_LITTLE_ENDIAN);
                offset += 2;

                break;
            case 0x11: /* LL_REJECT_IND_EXT */
                proto_tree_add_item(btle_tree, hf_control_reject_opcode, tvb, offset, 1, ENC_LITTLE_ENDIAN);
                offset += 1;

                proto_tree_add_item(btle_tree, hf_control_error_code, tvb, offset, 1, ENC_LITTLE_ENDIAN);
                offset += 1;

                break;
            case 0x14: /* LL_LENGTH_REQ */
            case 0x15: /* LL_LENGTH_RSP */
                proto_tree_add_item(btle_tree, hf_control_max_rx_octets, tvb, offset, 2, ENC_LITTLE_ENDIAN);
                offset += 2;

                proto_tree_add_item(btle_tree, hf_control_max_rx_time, tvb, offset, 2, ENC_LITTLE_ENDIAN);
                offset += 2;

                proto_tree_add_item(btle_tree, hf_control_max_tx_octets, tvb, offset, 2, ENC_LITTLE_ENDIAN);
                offset += 2;

                proto_tree_add_item(btle_tree, hf_control_max_tx_time, tvb, offset, 2, ENC_LITTLE_ENDIAN);
                offset += 2;

                break;
            case 0x16: /* LL_PHY_REQ */
            case 0x17: /* LL_PYH_RSP */
                proto_tree_add_bitmask(btle_tree, tvb, offset, hf_control_tx_phys, ett_tx_phys, hfx_control_phys_sender, ENC_NA);
                offset += 1;

                proto_tree_add_bitmask(btle_tree, tvb, offset, hf_control_rx_phys, ett_rx_phys, hfx_control_phys_sender, ENC_NA);
                offset += 1;

                break;
            case 0x18: /* LL_PHY_UPDATE_IND */
                proto_tree_add_bitmask(btle_tree, tvb, offset, hf_control_m_to_s_phy, ett_m_to_s_phy, hfx_control_phys_update, ENC_NA);
                offset += 1;

                proto_tree_add_bitmask(btle_tree, tvb, offset, hf_control_s_to_m_phy, ett_s_to_m_phy, hfx_control_phys_update, ENC_NA);
                offset += 1;

                proto_tree_add_item(btle_tree, hf_control_instant, tvb, offset, 2, ENC_LITTLE_ENDIAN);
                offset += 2;

                break;
            case 0x19: /* LL_MIN_USED_CHANNELS_IND */
                proto_tree_add_bitmask(btle_tree, tvb, offset, hf_control_phys, ett_phys, hfx_control_phys, ENC_NA);
                offset += 1;

                proto_tree_add_item(btle_tree, hf_control_min_used_channels, tvb, offset, 1, ENC_LITTLE_ENDIAN);
                offset += 1;

                break;
            default:
                if (tvb_reported_length_remaining(tvb, offset) > 3) {
                    proto_tree_add_expert(btle_tree, pinfo, &ei_unknown_data, tvb, offset, tvb_reported_length_remaining(tvb, offset) - 3);
                    offset += tvb_reported_length_remaining(tvb, offset) - 3;
                }
            }

            break;
        default:
            if (tvb_reported_length_remaining(tvb, offset) > 3) {
                proto_tree_add_expert(btle_tree, pinfo, &ei_unknown_data, tvb, offset, tvb_reported_length_remaining(tvb, offset) - 3);
                offset += tvb_reported_length_remaining(tvb, offset) - 3;
            }
        }

        if (add_l2cap_index) {
            item = proto_tree_add_uint(btle_tree, hf_l2cap_index, tvb, 0, 0, btle_frame_info->l2cap_index);
            PROTO_ITEM_SET_GENERATED(item);
        }

        if ((crc_status == CRC_INDETERMINATE) &&
            btle_context && btle_context->connection_info_valid) {
            /* the surrounding context has provided CRCInit */
            crc_init = btle_context->connection_info.CRCInit;
            crc_status = CRC_CAN_BE_CALCULATED;
        }
    }

    /* BT spec Vol 6, Part B, Section 1.2: CRC is big endian and bits in byte are flipped */
    packet_crc = reverse_bits_per_byte(tvb_get_ntoh24(tvb, offset));
    sub_item = proto_tree_add_uint(btle_tree, hf_crc, tvb, offset, 3, packet_crc);
    offset += 3;
    if (crc_status == CRC_CAN_BE_CALCULATED) {
        guint32 crc = btle_crc(tvb, length, crc_init);
        crc_status = (packet_crc == crc) ? CRC_CORRECT : CRC_INCORRECT;
    }
    switch(crc_status) {
    case CRC_INDETERMINATE:
        expert_add_info(pinfo, sub_item, &ei_crc_cannot_be_determined);
        break;
    case CRC_INCORRECT:
        expert_add_info(pinfo, sub_item, &ei_crc_incorrect);
        break;
    case CRC_CORRECT:
    default:
        break;
    }

    return offset;
}

void
proto_register_btle(void)
{
    module_t         *module;
    expert_module_t  *expert_module;

    static hf_register_info hf[] = {
        { &hf_access_address,
            { "Access Address",                  "btle.access_address",
            FT_UINT32, BASE_HEX, NULL, 0x0,
            NULL, HFILL }
        },
        { &hf_master_bd_addr,
            { "Master Address",                  "btle.master_bd_addr",
            FT_ETHER, BASE_NONE, NULL, 0x0,
            NULL, HFILL }
        },
        { &hf_slave_bd_addr,
            { "Slave Address",                   "btle.slave_bd_addr",
            FT_ETHER, BASE_NONE, NULL, 0x0,
            NULL, HFILL }
        },
        { &hf_length,
            { "Length",                          "btle.length",
            FT_UINT8, BASE_DEC, NULL, 0x0,
            NULL, HFILL }
        },
        { &hf_advertising_header,
            { "Packet Header",                   "btle.advertising_header",
            FT_UINT16, BASE_HEX, NULL, 0x0,
            NULL, HFILL }
        },
        { &hf_advertising_header_pdu_type,
            { "PDU Type",                        "btle.advertising_header.pdu_type",
            FT_UINT8, BASE_HEX | BASE_EXT_STRING, &pdu_type_vals_ext, 0x0F,
            NULL, HFILL }
        },
        { &hf_advertising_header_rfu_1,
            { "RFU",                             "btle.advertising_header.rfu.1",
            FT_UINT8, BASE_DEC, NULL, 0x10,
            "Reserved for Future Use", HFILL }
        },
        { &hf_advertising_header_ch_sel,
            { "Channel Selection Algorithm",     "btle.advertising_header.ch_sel",
            FT_BOOLEAN, 8, TFS(&tfs_ch_sel), 0x20,
            NULL, HFILL }
        },
        { &hf_advertising_header_randomized_tx,
            { "Tx Address",                      "btle.advertising_header.randomized_tx",
            FT_BOOLEAN, 8, TFS(&tfs_random_public), 0x40,
            NULL, HFILL }
        },
        { &hf_advertising_header_randomized_rx,
            { "Rx Address",                      "btle.advertising_header.randomized_rx",
            FT_BOOLEAN, 8, TFS(&tfs_random_public), 0x80,
            NULL, HFILL }
        },
        { &hf_advertising_header_reserved,
            { "Reserved",                        "btle.advertising_header.reserved",
            FT_BOOLEAN, 8, NULL, 0x80,
            NULL, HFILL }
        },
        { &hf_advertising_header_length,
            { "Length",                          "btle.advertising_header.length",
            FT_UINT8, BASE_DEC, NULL, 0x0,
            NULL, HFILL }
        },
        { &hf_advertising_address,
            { "Advertising Address",             "btle.advertising_address",
            FT_ETHER, BASE_NONE, NULL, 0x0,
            NULL, HFILL }
        },
        { &hf_initiator_addresss,
            { "Initator Address",                "btle.initiator_address",
            FT_ETHER, BASE_NONE, NULL, 0x0,
            NULL, HFILL }
        },
        { &hf_scanning_address,
            { "Scanning Address",                "btle.scanning_address",
            FT_ETHER, BASE_NONE, NULL, 0x0,
            NULL, HFILL }
        },
        { &hf_scan_response_data,
            { "Scan Response Data",              "btle.scan_responce_data",
            FT_BYTES, BASE_NONE, NULL, 0x0,
            NULL, HFILL }
        },
        { &hf_link_layer_data,
            { "Link Layer Data",                 "btle.link_layer_data",
            FT_NONE, BASE_NONE, NULL, 0x0,
            NULL, HFILL }
        },
        { &hf_link_layer_data_access_address,
            { "Access Address",                  "btle.link_layer_data.access_address",
            FT_UINT32, BASE_HEX, NULL, 0x0,
            NULL, HFILL }
        },
        { &hf_link_layer_data_crc_init,
            { "CRC Init",                        "btle.link_layer_data.crc_init",
            FT_UINT24, BASE_HEX, NULL, 0x0,
            NULL, HFILL }
        },
        { &hf_link_layer_data_window_size,
            { "Window Size",                     "btle.link_layer_data.window_size",
            FT_UINT8, BASE_DEC, NULL, 0x0,
            NULL, HFILL }
        },
        { &hf_link_layer_data_window_offset,
            { "Window Offset",                   "btle.link_layer_data.window_offset",
            FT_UINT16, BASE_DEC, NULL, 0x0,
            NULL, HFILL }
        },
        { &hf_link_layer_data_interval,
            { "Interval",                        "btle.link_layer_data.interval",
            FT_UINT16, BASE_DEC, NULL, 0x0,
            NULL, HFILL }
        },
        { &hf_link_layer_data_latency,
            { "Latency",                         "btle.link_layer_data.latency",
            FT_UINT16, BASE_DEC, NULL, 0x0,
            NULL, HFILL }
        },
        { &hf_link_layer_data_timeout,
            { "Timeout",                         "btle.link_layer_data.timeout",
            FT_UINT16, BASE_DEC, NULL, 0x0,
            NULL, HFILL }
        },
        { &hf_link_layer_data_channel_map,
            { "Channel Map",                     "btle.link_layer_data.channel_map",
            FT_BYTES, BASE_NONE, NULL, 0x0,
            NULL, HFILL }
        },
        { &hf_link_layer_data_hop,
            { "Hop",                             "btle.link_layer_data.hop",
            FT_UINT8, BASE_DEC, NULL, 0x1f,
            NULL, HFILL }
        },
        { &hf_link_layer_data_sleep_clock_accuracy,
            { "Sleep Clock Accuracy",            "btle.link_layer_data.sleep_clock_accuracy",
            FT_UINT8, BASE_DEC | BASE_EXT_STRING, &sleep_clock_accuracy_vals_ext, 0xe0,
            NULL, HFILL }
        },
        { &hf_data_header,
            { "Data Header",                     "btle.data_header",
            FT_UINT16, BASE_HEX, NULL, 0x0,
            NULL, HFILL }
        },
        { &hf_data_header_llid,
            { "LLID",                            "btle.data_header.llid",
            FT_UINT8, BASE_HEX | BASE_EXT_STRING, &llid_codes_vals_ext, 0x03,
            "Logical Link Identifier", HFILL }
        },
        { &hf_data_header_next_expected_sequence_number,
            { "Next Expected Sequence Number",   "btle.data_header.next_expected_sequence_number",
            FT_UINT8, BASE_DEC, NULL, 0x04,
            NULL, HFILL }
        },
        { &hf_data_header_sequence_number,
            { "Sequence Number",                 "btle.data_header.sequence_number",
            FT_UINT8, BASE_DEC, NULL, 0x08,
            NULL, HFILL }
        },
        { &hf_data_header_more_data,
            { "More Data",                       "btle.data_header.more_data",
            FT_BOOLEAN, 8, NULL, 0x10,
            NULL, HFILL }
        },
        { &hf_data_header_length,
            { "Length",                          "btle.data_header.length",
            FT_UINT8, BASE_DEC, NULL, 0x0,
            NULL, HFILL }
        },
        { &hf_data_header_rfu,
            { "RFU",                             "btle.data_header.rfu",
            FT_UINT8, BASE_DEC, NULL, 0xE0,
            "Reserved for Future Use", HFILL }
        },
        { &hf_control_opcode,
            { "Control Opcode",                  "btle.control_opcode",
            FT_UINT8, BASE_HEX | BASE_EXT_STRING, &control_opcode_vals_ext, 0x0,
            NULL, HFILL }
        },
        { &hf_control_reject_opcode,
            { "Reject Opcode",                   "btle.control.reject_opcode",
            FT_UINT8, BASE_HEX | BASE_EXT_STRING, &control_opcode_vals_ext, 0x0,
            NULL, HFILL }
        },
        { &hf_control_unknown_type,
            { "Unknown Type",                    "btle.control.unknown_type",
            FT_UINT8, BASE_HEX | BASE_EXT_STRING, &control_opcode_vals_ext, 0x0,
            NULL, HFILL }
        },
        { &hf_control_error_code,
            { "Error Code",                      "btle.control.error_code",
            FT_UINT8, BASE_HEX | BASE_EXT_STRING, &bthci_cmd_status_vals_ext, 0x0,
            NULL, HFILL }
        },
        { &hf_control_version_number,
            { "Version Number",                  "btle.control.version_number",
            FT_UINT8, BASE_HEX | BASE_EXT_STRING, &ll_version_number_vals_ext, 0x0,
            NULL, HFILL }
        },
        { &hf_control_company_id,
            { "Company Id",                      "btle.control.company_id",
            FT_UINT8, BASE_HEX | BASE_EXT_STRING, &bluetooth_company_id_vals_ext, 0x0,
            NULL, HFILL }
        },
        { &hf_control_subversion_number,
            { "Subversion Number",               "btle.control.subversion_number",
            FT_UINT16, BASE_HEX, NULL, 0x0,
            NULL, HFILL }
        },
        { &hf_control_feature_set,
            { "Feature Set",                     "btle.control.feature_set",
            FT_UINT64, BASE_HEX, NULL, 0x0,
            NULL, HFILL }
        },
        { &hf_control_feature_set_le_encryption,
            { "LE Encryption",                   "btle.control.feature_set.le_encryption",
            FT_BOOLEAN, 8, NULL, 0x01,
            NULL, HFILL }
        },
        { &hf_control_feature_set_connection_parameters_request_procedure,
            { "Connection Parameters Request Procedure",   "btle.control.feature_set.connection_parameters_request_procedure",
            FT_BOOLEAN, 8, NULL, 0x02,
            NULL, HFILL }
        },
        { &hf_control_feature_set_extended_reject_indication,
            { "Extended Reject Indication",           "btle.control.feature_set.extended_reject_indication",
            FT_BOOLEAN, 8, NULL, 0x04,
            NULL, HFILL }
        },
        { &hf_control_feature_set_slave_initiated_features_exchange,
            { "Slave Initiated Features Exchange",    "btle.control.feature_set.slave_initiated_features_exchange",
            FT_BOOLEAN, 8, NULL, 0x08,
            NULL, HFILL }
        },
        { &hf_control_feature_set_le_ping,
            { "LE Ping",                         "btle.control.feature_set.le_ping",
            FT_BOOLEAN, 8, NULL, 0x10,
            NULL, HFILL }
        },
        { &hf_control_feature_set_le_pkt_len_ext,
        { "LE Data Packet Length Extension",          "btle.control.feature_set.le_pkt_len_ext",
            FT_BOOLEAN, 8, NULL, 0x20,
            NULL, HFILL }
        },
        { &hf_control_feature_set_ll_privacy,
        { "LL Privacy",          "btle.control.feature_set.le_privacy",
            FT_BOOLEAN, 8, NULL, 0x40,
            NULL, HFILL }
        },
        { &hf_control_feature_set_ext_scan_flt_pol,
        { "Extended Scanner Filter Policies",          "btle.control.feature_set.ext_scan_flt_pol",
            FT_BOOLEAN, 8, NULL, 0x80,
            NULL, HFILL }
        },
        { &hf_control_feature_set_le_2m_phy,
        { "LE 2M PHY", "btle.control.feature_set.le_2m_phy",
            FT_BOOLEAN, 8, NULL, 0x01,
            NULL, HFILL }
        },
        { &hf_control_feature_set_stable_modulation_index_transmitter,
        { "Stable Modulation Index - Transmitter", "btle.control.feature_set.st_mod_idx_tx",
            FT_BOOLEAN, 8, NULL, 0x02,
            NULL, HFILL }
        },
        { &hf_control_feature_set_stable_modulation_index_receiver,
        { "Stable Modulation Index - Receiver", "btle.control.feature_set.st_mod_idx_rx",
            FT_BOOLEAN, 8, NULL, 0x04,
            NULL, HFILL }
        },
        { &hf_control_feature_set_le_coded_phy,
        { "LE Coded PHY", "btle.control.feature_set.le_coded_phy",
            FT_BOOLEAN, 8, NULL, 0x08,
            NULL, HFILL }
        },
        { &hf_control_feature_set_le_extended_advertising,
        { "LE Extended Advertising", "btle.control.feature_set.le_extended_adv",
            FT_BOOLEAN, 8, NULL, 0x10,
            NULL, HFILL }
        },
        { &hf_control_feature_set_le_periodic_advertising,
        { "LE Periodic Advertising", "btle.control.feature_set.periodic_adv",
            FT_BOOLEAN, 8, NULL, 0x20,
            NULL, HFILL }
        },
        { &hf_control_feature_set_channel_selection_algorithm_2,
        { "Channel Selection Algorithm #2", "btle.control.feature_set.ch_sel_2",
            FT_BOOLEAN, 8, NULL, 0x40,
            NULL, HFILL }
        },
        { &hf_control_feature_set_le_power_class_1,
        { "LE Power Class 1", "btle.control.feature_set.le_power_class_1",
            FT_BOOLEAN, 8, NULL, 0x80,
            NULL, HFILL }
        },
        { &hf_control_feature_set_minimum_number_of_used_channels_procedure,
        { "Minimum Number of Used Channels Procedure", "btle.control.feature_set.min_num_used_ch_proc",
            FT_BOOLEAN, 8, NULL, 0x01,
            NULL, HFILL }
        },
        { &hf_control_feature_set_reserved_bits,
            { "Reserved", "btle.control.feature_set.reserved_bits",
            FT_UINT8, BASE_DEC, NULL, 0xFE,
            NULL, HFILL }
        },
        { &hf_control_feature_set_reserved,
            { "Reserved",                        "btle.control.feature_set.reserved",
            FT_BYTES, BASE_NONE, NULL, 0x0,
            NULL, HFILL }
        },
        { &hf_control_window_size,
            { "Window Size",                     "btle.control.window_size",
            FT_UINT8, BASE_DEC, NULL, 0x0,
            NULL, HFILL }
        },
        { &hf_control_window_offset,
            { "Window Offset",                   "btle.control.window_offset",
            FT_UINT16, BASE_DEC, NULL, 0x0,
            NULL, HFILL }
        },
        { &hf_control_interval,
            { "Interval",                        "btle.control.interval",
            FT_UINT16, BASE_DEC, NULL, 0x0,
            NULL, HFILL }
        },
        { &hf_control_latency,
            { "Latency",                         "btle.control.latency",
            FT_UINT16, BASE_DEC, NULL, 0x0,
            NULL, HFILL }
        },
        { &hf_control_timeout,
            { "Timeout",                         "btle.control.timeout",
            FT_UINT16, BASE_DEC, NULL, 0x0,
            NULL, HFILL }
        },
        { &hf_control_instant,
            { "Instant",                         "btle.control.instant",
            FT_UINT16, BASE_DEC, NULL, 0x0,
            NULL, HFILL }
        },
        { &hf_control_interval_min,
            { "Interval Min",                    "btle.control.interval.min",
            FT_UINT16, BASE_DEC, NULL, 0x0,
            NULL, HFILL }
        },
        { &hf_control_interval_max,
            { "Interval Max",                    "btle.control.interval.max",
            FT_UINT16, BASE_DEC, NULL, 0x0,
            NULL, HFILL }
        },
        { &hf_control_preffered_periodicity,
            { "Preffered Periodicity",           "btle.control.preffered_periodicity",
            FT_UINT8, BASE_DEC, NULL, 0x0,
            NULL, HFILL }
        },
        { &hf_control_reference_connection_event_count,
            { "Reference Connection Event Count","btle.control.reference_connection_event_count",
            FT_UINT16, BASE_DEC, NULL, 0x0,
            NULL, HFILL }
        },
        { &hf_control_offset_0,
            { "Offset 0",                        "btle.control.offset.0",
            FT_UINT16, BASE_DEC, NULL, 0x0,
            NULL, HFILL }
        },
        { &hf_control_offset_1,
            { "Offset 1",                        "btle.control.offset.1",
            FT_UINT16, BASE_DEC, NULL, 0x0,
            NULL, HFILL }
        },
        { &hf_control_offset_2,
            { "Offset 2",                        "btle.control.offset.2",
            FT_UINT16, BASE_DEC, NULL, 0x0,
            NULL, HFILL }
        },
        { &hf_control_offset_3,
            { "Offset 3",                        "btle.control.offset.3",
            FT_UINT16, BASE_DEC, NULL, 0x0,
            NULL, HFILL }
        },
        { &hf_control_offset_4,
            { "Offset 4",                        "btle.control.offset.4",
            FT_UINT16, BASE_DEC, NULL, 0x0,
            NULL, HFILL }
        },
        { &hf_control_offset_5,
            { "Offset 5",                        "btle.control.offset.5",
            FT_UINT16, BASE_DEC, NULL, 0x0,
            NULL, HFILL }
        },
        { &hf_control_channel_map,
            { "Channel Map",                     "btle.control.channel_map",
            FT_BYTES, BASE_NONE, NULL, 0x0,
            NULL, HFILL }
        },
        { &hf_control_random_number,
            { "Random Number",                   "btle.control.random_number",
            FT_UINT64, BASE_DEC_HEX, NULL, 0x0,
            NULL, HFILL }
        },
        { &hf_control_encrypted_diversifier,
            { "Encrypted Diversifier",           "btle.control.encrypted_diversifier",
            FT_UINT16, BASE_DEC_HEX, NULL, 0x0,
            NULL, HFILL }
        },
        { &hf_control_master_session_key_diversifier,
            { "Master Session Key Diversifier",  "btle.control.master_session_key_diversifier",
            FT_UINT64, BASE_DEC_HEX, NULL, 0x0,
            NULL, HFILL }
        },
        { &hf_control_slave_session_key_diversifier,
            { "Slave Session Key Diversifier",   "btle.control.slave_session_key_diversifier",
            FT_UINT64, BASE_DEC_HEX, NULL, 0x0,
            NULL, HFILL }
        },
        { &hf_control_master_session_initialization_vector,
            { "Master Session Initialization Vector",      "btle.control.master_session_initialization_vector",
            FT_UINT32, BASE_DEC_HEX, NULL, 0x0,
            NULL, HFILL }
        },
        { &hf_control_slave_session_initialization_vector,
            { "Slave Session Initialization Vector",       "btle.control.slave_session_initialization_vector",
            FT_UINT64, BASE_DEC_HEX, NULL, 0x0,
            NULL, HFILL }
        },
        { &hf_control_max_rx_octets,
            { "Max RX octets",   "btle.control.max_rx_octets",
            FT_UINT16, BASE_DEC, NULL, 0x0,
            NULL, HFILL }
        },
        { &hf_control_max_rx_time,
            { "Max RX time",     "btle.control.max_rx_time",
            FT_UINT16, BASE_DEC|BASE_UNIT_STRING, &units_microsecond_microseconds, 0x0,
            NULL, HFILL }
        },
        { &hf_control_max_tx_octets,
            { "Max TX octets",   "btle.control.max_tx_octets",
            FT_UINT16, BASE_DEC, NULL, 0x0,
            NULL, HFILL }
        },
        { &hf_control_max_tx_time,
            { "Max TX time",     "btle.control.max_tx_time",
            FT_UINT16, BASE_DEC|BASE_UNIT_STRING, &units_microsecond_microseconds, 0x0,
            NULL, HFILL }
        },
        { &hf_control_phys_sender_le_1m_phy,
            { "Sender prefers to use the LE 1M PHY", "btle.control.phys.le_1m_phy",
            FT_BOOLEAN, 8, NULL, 0x01,
            NULL, HFILL }
        },
        { &hf_control_phys_sender_le_2m_phy,
            { "Sender prefers to use the LE 2M PHY", "btle.control.phys.le_2m_phy",
            FT_BOOLEAN, 8, NULL, 0x02,
            NULL, HFILL }
        },
        { &hf_control_phys_sender_le_coded_phy,
            { "Sender prefers to use the LE Coded PHY", "btle.control.phys.le_coded_phy",
            FT_BOOLEAN, 8, NULL, 0x04,
            NULL, HFILL }
        },
        { &hf_control_phys_update_le_1m_phy,
            { "The LE 1M PHY shall be used", "btle.control.phys.le_1m_phy",
            FT_BOOLEAN, 8, NULL, 0x01,
            NULL, HFILL }
        },
        { &hf_control_phys_update_le_2m_phy,
            { "The LE 2M PHY shall be used", "btle.control.phys.le_2m_phy",
            FT_BOOLEAN, 8, NULL, 0x02,
            NULL, HFILL }
        },
        { &hf_control_phys_update_le_coded_phy,
            { "The LE Coded PHY shall be used", "btle.control.phys.le_coded_phy",
            FT_BOOLEAN, 8, NULL, 0x04,
            NULL, HFILL }
        },
        { &hf_control_phys_reserved_bits,
            { "Reserved for future use", "btle.control.phys.reserved",
            FT_UINT8, BASE_DEC, NULL, 0xF8,
            NULL, HFILL }
        },
        { &hf_control_tx_phys,
            { "TX PHYs", "btle.control.tx_phys",
            FT_UINT8, BASE_HEX, NULL, 0x0,
            NULL, HFILL }
        },
        { &hf_control_rx_phys,
            { "RX PHYs", "btle.control.rx_phys",
            FT_UINT8, BASE_HEX, NULL, 0x0,
            NULL, HFILL }
        },
        { &hf_control_m_to_s_phy,
            { "Master to Slave PHY", "btle.control.m_to_s_phy",
            FT_UINT8, BASE_HEX, NULL, 0x0,
            NULL, HFILL }
        },
        { &hf_control_s_to_m_phy,
            { "Slave to Master PHY", "btle.control.s_to_m_phy",
            FT_UINT8, BASE_HEX, NULL, 0x0,
            NULL, HFILL }
        },
        { &hf_control_phys,
            { "PHYs", "btle.control.phys",
            FT_UINT8, BASE_HEX, NULL, 0x0,
            NULL, HFILL }
        },
        { &hf_control_phys_le_1m_phy,
            { "LE 1M PHY", "btle.control.phys.le_1m_phy",
            FT_BOOLEAN, 8, NULL, 0x01,
            NULL, HFILL }
        },
        { &hf_control_phys_le_2m_phy,
            { "LE 2M PHY", "btle.control.phys.le_2m_phy",
            FT_BOOLEAN, 8, NULL, 0x02,
            NULL, HFILL }
        },
        { &hf_control_phys_le_coded_phy,
            { "LE Coded PHY", "btle.control.phys.le_coded_phy",
            FT_BOOLEAN, 8, NULL, 0x04,
            NULL, HFILL }
        },
        { &hf_control_min_used_channels,
            { "Minimum Used Channels", "btle.control.min_used_channels",
            FT_UINT8, BASE_DEC, NULL, 0x0,
            NULL, HFILL }
        },
        { &hf_l2cap_index,
            { "L2CAP Index",                     "btle.l2cap_index",
            FT_UINT32, BASE_DEC, NULL, 0x0,
            NULL, HFILL }
        },
        { &hf_l2cap_fragment,
            { "L2CAP Fragment",                  "btle.l2cap_data",
            FT_NONE, BASE_NONE, NULL, 0x0,
            NULL, HFILL }
        },
        { &hf_crc,
            { "CRC",                             "btle.crc",
            FT_UINT24, BASE_HEX, NULL, 0x0,
            NULL, HFILL }
        },
        { &hf_btle_l2cap_msg_fragments,
        { "L2CAP fragments", "btle.l2cap.fragments",
            FT_NONE, BASE_NONE, NULL, 0x00,
            NULL, HFILL }
        },
        { &hf_btle_l2cap_msg_fragment,
        { "L2CAP fragment", "btle.l2cap.fragment",
            FT_FRAMENUM, BASE_NONE, NULL, 0x00,
            NULL, HFILL }
        },
        { &hf_btle_l2cap_msg_fragment_overlap,
        { "L2CAP fragment overlap", "btle.l2cap.fragment.overlap",
            FT_BOOLEAN, BASE_NONE, NULL, 0x0,
            NULL, HFILL }
        },
        { &hf_btle_l2cap_msg_fragment_overlap_conflicts,
        { "L2CAP fragment overlapping with conflicting data", "btle.l2cap.fragment.overlap.conflicts",
            FT_BOOLEAN, BASE_NONE, NULL, 0x0,
            NULL, HFILL }
        },
        { &hf_btle_l2cap_msg_fragment_multiple_tails,
        { "L2CAP has multiple tail fragments", "btle.l2cap.fragment.multiple_tails",
            FT_BOOLEAN, BASE_NONE, NULL, 0x0,
            NULL, HFILL }
        },
        { &hf_btle_l2cap_msg_fragment_too_long_fragment,
        { "L2CAP fragment too long", "btle.l2cap.fragment.too_long_fragment",
            FT_BOOLEAN, BASE_NONE, NULL, 0x0,
            NULL, HFILL }
        },
        { &hf_btle_l2cap_msg_fragment_error,
        { "L2CAP defragmentation error", "btle.l2cap.fragment.error",
            FT_FRAMENUM, BASE_NONE, NULL, 0x00,
            NULL, HFILL }
        },
        { &hf_btle_l2cap_msg_fragment_count,
        { "L2CAP fragment count", "btle.l2cap.fragment.count",
            FT_UINT32, BASE_DEC, NULL, 0x00,
            NULL, HFILL }
        },
        { &hf_btle_l2cap_msg_reassembled_in,
        { "Reassembled in", "btle.l2cap.reassembled.in",
            FT_FRAMENUM, BASE_NONE, NULL, 0x00,
            NULL, HFILL }
        },
        { &hf_btle_l2cap_msg_reassembled_length,
        { "Reassembled L2CAP length", "btle.l2cap.reassembled.length",
            FT_UINT32, BASE_DEC, NULL, 0x00,
            NULL, HFILL }
        },

    };

    static ei_register_info ei[] = {
        { &ei_unknown_data,
            { "btle.unknown_data",              PI_PROTOCOL, PI_NOTE,  "Unknown data", EXPFILL }},
        { &ei_access_address_matched,
            { "btle.access_address.matched",    PI_PROTOCOL, PI_NOTE,  "AccessAddress matched at capture", EXPFILL }},
        { &ei_access_address_bit_errors,
            { "btle.access_address.bit_errors", PI_PROTOCOL, PI_WARN,  "AccessAddress has errors present at capture", EXPFILL }},
        { &ei_access_address_illegal,
            { "btle.access_address.illegal",    PI_PROTOCOL, PI_ERROR, "AccessAddress has illegal value", EXPFILL }},
        { &ei_crc_cannot_be_determined,
            { "btle.crc.indeterminate",         PI_CHECKSUM, PI_NOTE,  "CRC unchecked, not all data available", EXPFILL }},
        { &ei_crc_incorrect,
            { "btle.crc.incorrect",             PI_CHECKSUM, PI_WARN,  "Incorrect CRC", EXPFILL }},
        { &ei_missing_fragment_start,
            { "btle.missing_fragment_start",    PI_SEQUENCE, PI_WARN,  "Missing Fragment Start", EXPFILL }},
        { &ei_retransmit,
            { "btle.retransmit",                PI_SEQUENCE, PI_NOTE,  "Retransmission", EXPFILL }},
    };

    static gint *ett[] = {
        &ett_btle,
        &ett_advertising_header,
        &ett_link_layer_data,
        &ett_data_header,
        &ett_features,
        &ett_tx_phys,
        &ett_rx_phys,
        &ett_m_to_s_phy,
        &ett_s_to_m_phy,
        &ett_phys,
        &ett_channel_map,
        &ett_scan_response_data,
        &ett_btle_l2cap_msg_fragment,
        &ett_btle_l2cap_msg_fragments
    };

    connection_info_tree = wmem_tree_new_autoreset(wmem_epan_scope(), wmem_file_scope());

    proto_btle = proto_register_protocol("Bluetooth Low Energy Link Layer",
            "BT LE LL", "btle");
    btle_handle = register_dissector("btle", dissect_btle, proto_btle);

    proto_register_field_array(proto_btle, hf, array_length(hf));
    proto_register_subtree_array(ett, array_length(ett));

    expert_module = expert_register_protocol(proto_btle);
    expert_register_field_array(expert_module, ei, array_length(ei));

    module = prefs_register_protocol(proto_btle, NULL);
    prefs_register_static_text_preference(module, "version",
            "Bluetooth LE LL version: 5.0 (Core)",
            "Version of protocol supported by this dissector.");

    prefs_register_bool_preference(module, "detect_retransmit",
                                   "Detect retransmission",
                                   "Detect retransmission based on SN (Sequence Number)",
                                   &btle_detect_retransmit);

    reassembly_table_register(&btle_l2cap_msg_reassembly_table,
        &addresses_reassembly_table_functions);

    register_init_routine(btle_init);
}

void
proto_reg_handoff_btle(void)
{
    btcommon_ad_handle = find_dissector_add_dependency("btcommon.eir_ad.ad", proto_btle);
    btcommon_le_channel_map_handle = find_dissector_add_dependency("btcommon.le_channel_map", proto_btle);
    btl2cap_handle = find_dissector_add_dependency("btl2cap", proto_btle);

    proto_btle_rf = proto_get_id_by_filter_name("btle_rf");
    proto_nordic_ble = proto_get_id_by_filter_name("nordic_ble");

    dissector_add_uint("bluetooth.encap", WTAP_ENCAP_BLUETOOTH_LE_LL, btle_handle);
}

/*
 * Editor modelines  -  http://www.wireshark.org/tools/modelines.html
 *
 * Local variables:
 * c-basic-offset: 4
 * tab-width: 8
 * indent-tabs-mode: nil
 * End:
 *
 * vi: set shiftwidth=4 tabstop=8 expandtab:
 * :indentSize=4:tabSize=8:noTabs=true:
 */
