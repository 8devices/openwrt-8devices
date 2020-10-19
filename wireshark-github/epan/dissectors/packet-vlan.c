/* packet-vlan.c
 * Routines for VLAN 802.1Q ethernet header disassembly
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

#define NEW_PROTO_TREE_API

#include "config.h"

#include <epan/packet.h>
#include <epan/capture_dissectors.h>
#include <wsutil/pint.h>
#include <epan/expert.h>
#include "packet-ieee8023.h"
#include "packet-ipx.h"
#include "packet-llc.h"
#include <epan/etypes.h>
#include <epan/prefs.h>
#include <epan/to_str.h>
#include <epan/addr_resolv.h>

void proto_register_vlan(void);
void proto_reg_handoff_vlan(void);

static unsigned int q_in_q_ethertype = ETHERTYPE_QINQ_OLD;

static gboolean vlan_summary_in_tree = TRUE;

enum version_value {
  IEEE_8021Q_1998,
  IEEE_8021Q_2005,
  IEEE_8021Q_2011
};

static gint vlan_version = (gint)IEEE_8021Q_2011;

enum priority_drop_value {
  Priority_Drop_8P0D,
  Priority_Drop_7P1D,
  Priority_Drop_6P2D,
  Priority_Drop_5P3D,
};

static gint vlan_priority_drop = (gint)Priority_Drop_8P0D;

static dissector_handle_t vlan_handle;
static dissector_handle_t ethertype_handle;

static capture_dissector_handle_t llc_cap_handle;
static capture_dissector_handle_t ipx_cap_handle;

static header_field_info *hfi_vlan = NULL;

#define VLAN_HFI_INIT HFI_INIT(proto_vlan)

/* From Table G-2 of IEEE standard 802.1D-2004 */
/* Note that 0 is the default priority, but is above 1 and 2.
 * Priority order from lowest to highest is 1,2,0,3,4,5,6,7 */
static const value_string pri_vals_old[] = {
  { 0, "Best Effort (default)"             },
  { 1, "Background"                        },
  { 2, "Spare"                             },
  { 3, "Excellent Effort"                  },
  { 4, "Controlled Load"                   },
  { 5, "Video, < 100ms latency and jitter" },
  { 6, "Voice, < 10ms latency and jitter"  },
  { 7, "Network Control"                   },
  { 0, NULL                                }
};

static header_field_info hfi_vlan_priority_old VLAN_HFI_INIT = {
        "Priority", "vlan.priority", FT_UINT16, BASE_DEC,
        VALS(pri_vals_old), 0xE000, "Descriptions are recommendations from IEEE standard 802.1D-2004", HFILL };

/* From Table G-2 of IEEE standard 802.1Q-2005 (and I-2 of 2011 and 2014 revisions) */
/* Note that 0 is still the default, but priority 2 was moved from below 0 to
 * above it. The new order from lowest to highest is 1,0,2,3,4,5,6,7 */
static const value_string pri_vals[] = {
  { 0, "Best Effort (default)"             },
  { 1, "Background"                        },
  { 2, "Excellent Effort"                  },
  { 3, "Critical Applications"             },
  { 4, "Video, < 100ms latency and jitter" },
  { 5, "Voice, < 10ms latency and jitter"  },
  { 6, "Internetwork Control"              },
  { 7, "Network Control"                   },
  { 0, NULL                                }
};

static header_field_info hfi_vlan_priority VLAN_HFI_INIT = {
        "Priority", "vlan.priority", FT_UINT16, BASE_DEC,
        VALS(pri_vals), 0xE000, "Descriptions are recommendations from IEEE standard 802.1Q-2014", HFILL };

/* From Tables G-2,3 of IEEE standard 802.1Q-2005 (and I-2,3,7 of 2011 and 2014 revisions) */
static const value_string pri_vals_7[] = {
  { 0, "Best Effort (default)"                           },
  { 1, "Background"                                      },
  { 2, "Excellent Effort"                                },
  { 3, "Critical Applications"                           },
  { 4, "Voice, < 10ms latency and jitter, Drop Eligible" },
  { 5, "Voice, < 10ms latency and jitter"                },
  { 6, "Internetwork Control"                            },
  { 7, "Network Control"                                 },
  { 0, NULL                                              }
};

static header_field_info hfi_vlan_priority_7 VLAN_HFI_INIT = {
        "Priority", "vlan.priority", FT_UINT16, BASE_DEC,
        VALS(pri_vals_7), 0xE000, "Descriptions are recommendations from IEEE standard 802.1Q-2014", HFILL };

/* From Tables G-2,3 of IEEE standard 802.1Q-2005 (and I-2,3,7 of 2011 and 2015 revisions) */
static const value_string pri_vals_6[] = {
  { 0, "Best Effort (default)"                            },
  { 1, "Background"                                       },
  { 2, "Critical Applications, Drop Eligible"             },
  { 3, "Critical Applications"                            },
  { 4, "Voice, < 10ms latency and jitter, Drop Eligible"  },
  { 5, "Voice, < 10ms latency and jitter"                 },
  { 6, "Internetwork Control"                             },
  { 7, "Network Control"                                  },
  { 0, NULL                                               }
};

static header_field_info hfi_vlan_priority_6 VLAN_HFI_INIT = {
        "Priority", "vlan.priority", FT_UINT16, BASE_DEC,
        VALS(pri_vals_6), 0xE000, "Descriptions are recommendations from IEEE standard 802.1Q-2014", HFILL };

/* From Tables G-2,3 of IEEE standard 802.1Q-2005 (and I-2,3,7 of 2011 and 2015 revisions) */
static const value_string pri_vals_5[] = {
  { 0, "Best Effort (default), Drop Eligible"            },
  { 1, "Best Effort (default)"                           },
  { 2, "Critical Applications, Drop Eligible"            },
  { 3, "Critical Applications"                           },
  { 4, "Voice, < 10ms latency and jitter, Drop Eligible" },
  { 5, "Voice, < 10ms latency and jitter"                },
  { 6, "Internetwork Control"                            },
  { 7, "Network Control"                                 },
  { 0, NULL                                              }
};

static header_field_info hfi_vlan_priority_5 VLAN_HFI_INIT = {
        "Priority", "vlan.priority", FT_UINT16, BASE_DEC,
        VALS(pri_vals_5), 0xE000, "Descriptions are recommendations from IEEE standard 802.1Q-2014", HFILL };

/* True is non-canonical (i.e., bit-reversed MACs like Token Ring) since usually 0 and canonical. */
static const true_false_string tfs_noncanonical_canonical = { "Non-canonical", "Canonical" };

static header_field_info hfi_vlan_cfi VLAN_HFI_INIT = {
        "CFI", "vlan.cfi", FT_BOOLEAN, 16,
        TFS(&tfs_noncanonical_canonical), 0x1000, "Canonical Format Identifier", HFILL };

static const true_false_string tfs_eligible_ineligible = { "Eligible", "Ineligible" };

static header_field_info hfi_vlan_dei VLAN_HFI_INIT = {
        "DEI", "vlan.dei", FT_BOOLEAN, 16,
        TFS(&tfs_eligible_ineligible), 0x1000, "Drop Eligible Indicator", HFILL };

static header_field_info hfi_vlan_id VLAN_HFI_INIT = {
        "ID", "vlan.id", FT_UINT16, BASE_DEC,
        NULL, 0x0FFF, "VLAN ID", HFILL };

static header_field_info hfi_vlan_id_name VLAN_HFI_INIT = {
        "Name", "vlan.id_name", FT_STRING, STR_UNICODE,
        NULL, 0x0, "VLAN ID Name", HFILL };

static header_field_info hfi_vlan_etype VLAN_HFI_INIT = {
        "Type", "vlan.etype", FT_UINT16, BASE_HEX,
        VALS(etype_vals), 0x0, "Ethertype", HFILL };

static header_field_info hfi_vlan_len VLAN_HFI_INIT = {
        "Length", "vlan.len", FT_UINT16, BASE_DEC,
        NULL, 0x0, NULL, HFILL };

static header_field_info hfi_vlan_trailer VLAN_HFI_INIT = {
        "Trailer", "vlan.trailer", FT_BYTES, BASE_NONE,
        NULL, 0x0, "VLAN Trailer", HFILL };


static gint ett_vlan = -1;

static expert_field ei_vlan_len = EI_INIT;

static gboolean
capture_vlan(const guchar *pd, int offset, int len, capture_packet_info_t *cpinfo, const union wtap_pseudo_header *pseudo_header _U_ ) {
  guint16 encap_proto;
  if ( !BYTES_ARE_IN_FRAME(offset,len,5) )
    return FALSE;

  encap_proto = pntoh16( &pd[offset+2] );
  if ( encap_proto <= IEEE_802_3_MAX_LEN) {
    if ( pd[offset+4] == 0xff && pd[offset+5] == 0xff ) {
      return call_capture_dissector(ipx_cap_handle, pd,offset+4,len, cpinfo, pseudo_header);
    } else {
      return call_capture_dissector(llc_cap_handle, pd,offset+4,len, cpinfo, pseudo_header);
    }
  }

  return try_capture_dissector("ethertype", encap_proto, pd, offset+4, len, cpinfo, pseudo_header);
}

static void
columns_set_vlan(column_info *cinfo, guint16 tci)
{
  char id_str[16];

  guint32_to_str_buf(tci & 0xFFF, id_str, sizeof(id_str));

  if (vlan_version < IEEE_8021Q_2011) {
    col_add_fstr(cinfo, COL_INFO,
                 "PRI: %d  CFI: %d  ID: %s",
                 (tci >> 13), ((tci >> 12) & 1), id_str);
  } else {
    col_add_fstr(cinfo, COL_INFO,
                 "PRI: %d  DEI: %d  ID: %s",
                 (tci >> 13), ((tci >> 12) & 1), id_str);
  }
  col_add_str(cinfo, COL_8021Q_VLAN_ID, id_str);
}

static int
dissect_vlan(tvbuff_t *tvb, packet_info *pinfo, proto_tree *tree, void* data _U_)
{
  proto_item *ti;
  guint16 tci, vlan_id;
  guint16 encap_proto;
  gboolean is_802_2;
  proto_tree *vlan_tree;
  proto_item *item;

  col_set_str(pinfo->cinfo, COL_PROTOCOL, "VLAN");
  col_clear(pinfo->cinfo, COL_INFO);

  tci = tvb_get_ntohs( tvb, 0 );
  vlan_id = tci & 0xFFF;
  /* Add the VLAN Id if it's the first one*/
  if (pinfo->vlan_id == 0) {
      pinfo->vlan_id = vlan_id;
  }

  columns_set_vlan(pinfo->cinfo, tci);

  vlan_tree = NULL;

  if (tree) {
    ti = proto_tree_add_item(tree, hfi_vlan, tvb, 0, 4, ENC_NA);

    if (vlan_summary_in_tree) {
      if (vlan_version < IEEE_8021Q_2011) {
        proto_item_append_text(ti, ", PRI: %u, CFI: %u, ID: %u",
                (tci >> 13), ((tci >> 12) & 1), vlan_id);
      } else {
        proto_item_append_text(ti, ", PRI: %u, DEI: %u, ID: %u",
                (tci >> 13), ((tci >> 12) & 1), vlan_id);
      }
    }

    vlan_tree = proto_item_add_subtree(ti, ett_vlan);

    if (vlan_version == IEEE_8021Q_1998) {
        proto_tree_add_item(vlan_tree, &hfi_vlan_priority_old, tvb, 0, 2, ENC_BIG_ENDIAN);
        proto_tree_add_item(vlan_tree, &hfi_vlan_cfi, tvb, 0, 2, ENC_BIG_ENDIAN);
    } else {
      switch (vlan_priority_drop) {

        case Priority_Drop_8P0D:
          proto_tree_add_item(vlan_tree, &hfi_vlan_priority, tvb, 0, 2, ENC_BIG_ENDIAN);
          break;

        case Priority_Drop_7P1D:
          proto_tree_add_item(vlan_tree, &hfi_vlan_priority_7, tvb, 0, 2, ENC_BIG_ENDIAN);
          break;

        case Priority_Drop_6P2D:
          proto_tree_add_item(vlan_tree, &hfi_vlan_priority_6, tvb, 0, 2, ENC_BIG_ENDIAN);
          break;

        case Priority_Drop_5P3D:
          proto_tree_add_item(vlan_tree, &hfi_vlan_priority_5, tvb, 0, 2, ENC_BIG_ENDIAN);
          break;
      }
      if (vlan_version == IEEE_8021Q_2005) {
        proto_tree_add_item(vlan_tree, &hfi_vlan_cfi, tvb, 0, 2, ENC_BIG_ENDIAN);
      } else {
        proto_tree_add_item(vlan_tree, &hfi_vlan_dei, tvb, 0, 2, ENC_BIG_ENDIAN);
      }
    }
    proto_tree_add_item(vlan_tree, &hfi_vlan_id, tvb, 0, 2, ENC_BIG_ENDIAN);
    if (gbl_resolv_flags.vlan_name) {
      item = proto_tree_add_string(vlan_tree, &hfi_vlan_id_name, tvb, 0, 2,
                                   get_vlan_name(wmem_packet_scope(), vlan_id));
      PROTO_ITEM_SET_GENERATED(item);

    }

    /* TODO: If the CFI is set on Ethernet (or FDDI MAC and not source routed,
     * i.e. the RII bit in the source MAC address is 0, then a E-RIF follows.
     * Only true before version 2011 since the CFI was replaced with DEI
     * (Since who needs VLANs that bridge Token Ring and FDDI these days?)  */
  }

  encap_proto = tvb_get_ntohs(tvb, 2);
  if (encap_proto <= IEEE_802_3_MAX_LEN) {
    /* Is there an 802.2 layer? I can tell by looking at the first 2
       bytes after the VLAN header. If they are 0xffff, then what
       follows the VLAN header is an IPX payload, meaning no 802.2.
       (IPX/SPX is they only thing that can be contained inside a
       straight 802.3 packet, so presumably the same applies for
       Ethernet VLAN packets). A non-0xffff value means that there's an
       802.2 layer inside the VLAN layer */
    is_802_2 = TRUE;

    /* Don't throw an exception for this check (even a BoundsError) */
    if (tvb_captured_length_remaining(tvb, 4) >= 2) {
      if (tvb_get_ntohs(tvb, 4) == 0xffff) {
        is_802_2 = FALSE;
      }
    }

    dissect_802_3(encap_proto, is_802_2, tvb, 4, pinfo, tree, vlan_tree,
                  hfi_vlan_len.id, hfi_vlan_trailer.id, &ei_vlan_len, 0);
  } else {
    ethertype_data_t ethertype_data;

    ethertype_data.etype = encap_proto;
    ethertype_data.offset_after_ethertype = 4;
    ethertype_data.fh_tree = vlan_tree;
    ethertype_data.etype_id = hfi_vlan_etype.id;
    ethertype_data.trailer_id = hfi_vlan_trailer.id;
    ethertype_data.fcs_len = 0;

    call_dissector_with_data(ethertype_handle, tvb, pinfo, tree, &ethertype_data);
  }
  return tvb_captured_length(tvb);
}

void
proto_register_vlan(void)
{
#ifndef HAVE_HFI_SECTION_INIT
  static header_field_info *hfi[] = {
    &hfi_vlan_priority_old,
    &hfi_vlan_priority,
    &hfi_vlan_priority_7,
    &hfi_vlan_priority_6,
    &hfi_vlan_priority_5,
    &hfi_vlan_cfi,
    &hfi_vlan_dei,
    &hfi_vlan_id,
    &hfi_vlan_id_name,
    &hfi_vlan_etype,
    &hfi_vlan_len,
    &hfi_vlan_trailer,
  };
#endif /* HAVE_HFI_SECTION_INIT */

  static gint *ett[] = {
    &ett_vlan
  };

  static ei_register_info ei[] = {
     { &ei_vlan_len, { "vlan.len.past_end", PI_MALFORMED, PI_ERROR, "Length field value goes past the end of the payload", EXPFILL }},
  };

  static const enum_val_t version_vals[] = {
    {"1998", "IEEE 802.1Q-1998", IEEE_8021Q_1998},
    {"2005", "IEEE 802.1Q-2005", IEEE_8021Q_2005},
    {"2011", "IEEE 802.1Q-2011", IEEE_8021Q_2011},
    {NULL, NULL, -1}
  };

  static const enum_val_t priority_drop_vals[] = {
    {"8p0d", "8 Priorities, 0 Drop Eligible", Priority_Drop_8P0D},
    {"7p1d", "7 Priorities, 1 Drop Eligible", Priority_Drop_7P1D},
    {"6p2d", "6 Priorities, 2 Drop Eligible", Priority_Drop_6P2D},
    {"5p3d", "5 Priorities, 3 Drop Eligible", Priority_Drop_5P3D},
    {NULL, NULL, -1}
  };

  module_t *vlan_module;
  expert_module_t* expert_vlan;
  int proto_vlan;

  proto_vlan = proto_register_protocol("802.1Q Virtual LAN", "VLAN", "vlan");
  hfi_vlan = proto_registrar_get_nth(proto_vlan);

  proto_register_fields(proto_vlan, hfi, array_length(hfi));
  proto_register_subtree_array(ett, array_length(ett));
  expert_vlan = expert_register_protocol(proto_vlan);
  expert_register_field_array(expert_vlan, ei, array_length(ei));

  vlan_module = prefs_register_protocol(proto_vlan, proto_reg_handoff_vlan);
  prefs_register_bool_preference(vlan_module, "summary_in_tree",
        "Show vlan summary in protocol tree",
        "Whether the vlan summary line should be shown in the protocol tree",
        &vlan_summary_in_tree);
  prefs_register_uint_preference(vlan_module, "qinq_ethertype",
        "802.1QinQ Ethertype (in hex)",
        "The (hexadecimal) Ethertype used to indicate 802.1QinQ VLAN in VLAN tunneling.",
        16, &q_in_q_ethertype);
  prefs_register_enum_preference(vlan_module, "version",
        "IEEE 802.1Q version",
        "IEEE 802.1Q specification version used (802.1Q-1998 uses 802.1D-2004 for PRI values)",
        &vlan_version, version_vals, TRUE);
  prefs_register_enum_preference(vlan_module, "priority_drop",
        "Priorities and drop eligibility",
        "Number of priorities supported, and number of those drop eligible (not used for 802.1Q-1998)",
        &vlan_priority_drop, priority_drop_vals, FALSE);
  vlan_handle = create_dissector_handle(dissect_vlan, proto_vlan);
}

void
proto_reg_handoff_vlan(void)
{
  static gboolean prefs_initialized = FALSE;
  static unsigned int old_q_in_q_ethertype;
  capture_dissector_handle_t vlan_cap_handle;

  if (!prefs_initialized)
  {
    dissector_add_uint("ethertype", ETHERTYPE_VLAN, vlan_handle);
    vlan_cap_handle = create_capture_dissector_handle(capture_vlan, hfi_vlan->id);
    capture_dissector_add_uint("ethertype", ETHERTYPE_VLAN, vlan_cap_handle);

    prefs_initialized = TRUE;
  }
  else
  {
    dissector_delete_uint("ethertype", old_q_in_q_ethertype, vlan_handle);
  }

  old_q_in_q_ethertype = q_in_q_ethertype;
  ethertype_handle = find_dissector_add_dependency("ethertype", hfi_vlan->id);

  dissector_add_uint("ethertype", q_in_q_ethertype, vlan_handle);

  llc_cap_handle = find_capture_dissector("llc");
  ipx_cap_handle = find_capture_dissector("ipx");
}

/*
 * Editor modelines  -  http://www.wireshark.org/tools/modelines.html
 *
 * Local Variables:
 * c-basic-offset: 2
 * tab-width: 8
 * indent-tabs-mode: nil
 * End:
 *
 * ex: set shiftwidth=2 tabstop=8 expandtab:
 * :indentSize=2:tabSize=8:noTabs=true:
 */
