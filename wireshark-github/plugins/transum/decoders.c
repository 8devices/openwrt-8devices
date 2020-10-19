/* decoders.c
* Routines for the TRANSUM response time analyzer post-dissector
* By Paul Offord <paul.offord@advance7.com>
* Copyright 2016 Advance Seven Limited
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

#include <stdio.h>
#include <epan/packet.h>
#include <epan/prefs.h>
#include <epan/dissectors/packet-tcp.h>
#include "packet-transum.h"
#include "preferences.h"
#include "extractors.h"
#include "decoders.h"

extern TSUM_PREFERENCES preferences;


/* Returns the number of sub-packets of interest */
int decode_syn(packet_info *pinfo _U_, proto_tree *tree _U_, PKT_INFO* pkt_info)
{
    if (pkt_info->tcp_flags_ack)
        pkt_info->rrpd.c2s = FALSE;
    else
    {
        pkt_info->rrpd.c2s = TRUE;
        pkt_info->rrpd.state = RRPD_STATE_4;
        add_detected_tcp_svc(pkt_info->dstport);
    }

    pkt_info->rrpd.session_id = 1;
    pkt_info->rrpd.msg_id = 1;
    pkt_info->rrpd.suffix = 1;
    pkt_info->rrpd.decode_based = TRUE;
    pkt_info->rrpd.calculation = RTE_CALC_SYN;
    pkt_info->pkt_of_interest = TRUE;

    return 1;
}

/*
    This function sets basic information in the sub_packet entry.
    Because we don't expect multiple DCE-RPC messages in a single packet
    we only use single PKT_INFO

    Returns the number of sub-packets of interest, which in this case is always 1.
 */
int decode_dcerpc(packet_info *pinfo _U_, proto_tree *tree, PKT_INFO* pkt_info)
{
    guint32 field_uint[MAX_RETURNED_ELEMENTS];  /* An extracted field array for unsigned integers */
    size_t field_value_count;  /* How many entries are there in the extracted field array */
    guint32 dcerpc_cn_ctx_id = 0;

    if (!extract_uint(tree, hf_of_interest[HF_INTEREST_DCERPC_VER].hf, field_uint, &field_value_count))
    {
        if (field_value_count)
            pkt_info->dcerpc_ver = field_uint[0];
    }

    if (!extract_uint(tree, hf_of_interest[HF_INTEREST_DCERPC_PKT_TYPE].hf, field_uint, &field_value_count))
    {
        if (field_value_count)
            pkt_info->dcerpc_pkt_type = field_uint[0];
    }

    if (field_value_count)
    {
        if (!extract_uint(tree, hf_of_interest[HF_INTEREST_DCERPC_CN_CTX_ID].hf, field_uint, &field_value_count))
        {
            if (field_value_count)
                dcerpc_cn_ctx_id = field_uint[0];
        }

        if (is_dcerpc_context_zero(pkt_info->dcerpc_pkt_type))
        { /* This is needed to overcome an apparent Wireshark bug
             found in the LUA code - is this still true in C? */
            pkt_info->rrpd.session_id = 1;
        }
        else
        {
            if (dcerpc_cn_ctx_id)
                pkt_info->rrpd.session_id = dcerpc_cn_ctx_id;
            else
                pkt_info->rrpd.session_id = 1;
        }
        if (!extract_uint(tree, hf_of_interest[HF_INTEREST_DCERPC_CN_CALL_ID].hf, field_uint, &field_value_count))
        {
            if (field_value_count)
                pkt_info->rrpd.msg_id = field_uint[0];
        }
    }
    else
    {
        /*
            we don't have header information and so by setting the session_id and msg_id to zero
            the rrpd functions will either create a new rrpd_list (or temp_rsp_rrpd_list) entry
            or update the last entry for this ip_proto:stream_no.
         */
        pkt_info->rrpd.session_id = 0;
        pkt_info->rrpd.msg_id = 0;
    }


    if (is_dcerpc_req_pkt_type(pkt_info->dcerpc_pkt_type))
    {
        pkt_info->rrpd.c2s = TRUE;
        wmem_map_insert(preferences.tcp_svc_ports, GUINT_TO_POINTER(pkt_info->dstport), GUINT_TO_POINTER(RTE_CALC_DCERPC)); /* make sure we have this DCE-RPC service port set */
    }
    else
    {
        pkt_info->rrpd.c2s = FALSE;
        wmem_map_insert(preferences.tcp_svc_ports, GUINT_TO_POINTER(pkt_info->srcport), GUINT_TO_POINTER(RTE_CALC_DCERPC)); /* make sure we have this DCE-RPC service port set */
    }

    pkt_info->rrpd.suffix = 1;
    pkt_info->rrpd.decode_based = TRUE;
    pkt_info->rrpd.calculation = RTE_CALC_DCERPC;
    pkt_info->pkt_of_interest = TRUE;

    return 1;
}

/* Returns the number of sub-packets of interest */
int decode_smb(packet_info *pinfo _U_, proto_tree *tree, PKT_INFO* pkt_info, PKT_INFO* subpackets)
{
    guint32 field_uint[MAX_RETURNED_ELEMENTS];  /* An extracted field array for unsigned integers */
    size_t field_value_count;  /* How many entries are there in the extracted field array */

    guint64 ses_id[MAX_RETURNED_ELEMENTS];
    size_t ses_id_count;
    guint64 msg_id[MAX_RETURNED_ELEMENTS];
    size_t msg_id_count;

    /* set the direction information */
    if (pkt_info->dstport == 445)
        pkt_info->rrpd.c2s = TRUE;
    else
        pkt_info->rrpd.c2s = FALSE;

    if (!extract_uint(tree, hf_of_interest[HF_INTEREST_SMB_MID].hf, field_uint, &field_value_count))
    {
        if (field_value_count)
        {
            pkt_info->rrpd.calculation = RTE_CALC_SMB1;
            pkt_info->pkt_of_interest = FALSE; /* can't process SMB1 at the moment */
            return 0;
        }
    }
    /* Default in case we don't have header information */
    pkt_info->rrpd.session_id = 0;
    pkt_info->rrpd.msg_id = 0;
    pkt_info->rrpd.suffix = 1;
    pkt_info->rrpd.decode_based = TRUE;
    pkt_info->rrpd.calculation = RTE_CALC_SMB2;
    pkt_info->pkt_of_interest = TRUE;

    extract_ui64(tree, hf_of_interest[HF_INTEREST_SMB2_MSG_ID].hf, msg_id, &msg_id_count);
    if (msg_id_count)  /* test for header information */
    {
        extract_ui64(tree, hf_of_interest[HF_INTEREST_SMB2_SES_ID].hf, ses_id, &ses_id_count);

        for (size_t i = 0; (i < msg_id_count) && (i < MAX_SUBPKTS_PER_PACKET); i++)
        {
            subpackets[i].rrpd.c2s = pkt_info->rrpd.c2s;
            subpackets[i].rrpd.ip_proto = pkt_info->rrpd.ip_proto;
            subpackets[i].rrpd.stream_no = pkt_info->rrpd.stream_no;

            subpackets[i].rrpd.session_id = ses_id[i];
            subpackets[i].rrpd.msg_id = msg_id[i];
            subpackets[i].rrpd.suffix = 1;

            subpackets[i].rrpd.decode_based = TRUE;
            subpackets[i].rrpd.calculation = RTE_CALC_SMB2;
            subpackets[i].pkt_of_interest = TRUE;
        }
        return (int)msg_id_count;
    }

    return 1;
}

/* Returns the number of sub-packets of interest */
int decode_gtcp(packet_info *pinfo, proto_tree *tree, PKT_INFO* pkt_info)
{
    guint32 field_uint[MAX_RETURNED_ELEMENTS];  /* An extracted field array for unsigned integers */
    gboolean field_bool[MAX_RETURNED_ELEMENTS];  /* An extracted field array for unsigned integers */
    size_t field_value_count;  /* How many entries are there in the extracted field array */

    if (!extract_uint(tree, hf_of_interest[HF_INTEREST_TCP_STREAM].hf, field_uint, &field_value_count)) {
        if (field_value_count)
            pkt_info->rrpd.stream_no = field_uint[0];
    }

    pkt_info->srcport = pinfo->srcport;
    pkt_info->dstport = pinfo->destport;

    if (!extract_uint(tree, hf_of_interest[HF_INTEREST_TCP_LEN].hf, field_uint, &field_value_count)) {
        if (field_value_count)
            pkt_info->len = field_uint[0];
    }

    if (!extract_bool(tree, hf_of_interest[HF_INTEREST_TCP_FLAGS_SYN].hf, field_bool, &field_value_count)) {
        if (field_value_count)
            pkt_info->tcp_flags_syn = field_bool[0];
    }

    if (!extract_bool(tree, hf_of_interest[HF_INTEREST_TCP_FLAGS_ACK].hf, field_bool, &field_value_count)) {
        if (field_value_count)
            pkt_info->tcp_flags_ack = field_bool[0];
    }

    if (!extract_bool(tree, hf_of_interest[HF_INTEREST_TCP_FLAGS_RESET].hf, field_bool, &field_value_count)) {
        if (field_value_count)
            pkt_info->tcp_flags_reset = field_bool[0];
    }

    if (!extract_bool(tree, hf_of_interest[HF_INTEREST_TCP_RETRAN].hf, field_bool, &field_value_count)) {
        if (field_value_count)
            pkt_info->tcp_retran = field_bool[0];
    }

    if (!extract_bool(tree, hf_of_interest[HF_INTEREST_TCP_KEEP_ALIVE].hf, field_bool, &field_value_count)) {
        if (field_value_count)
            pkt_info->tcp_keep_alive = field_bool[0];
    }

    if (((wmem_map_lookup(preferences.tcp_svc_ports, GUINT_TO_POINTER(pkt_info->dstport)) != NULL) ||
         (wmem_map_lookup(preferences.tcp_svc_ports, GUINT_TO_POINTER(pkt_info->srcport)) != NULL)) &&
        (pkt_info->len > 0))
    {
        if (wmem_map_lookup(preferences.tcp_svc_ports, GUINT_TO_POINTER(pkt_info->dstport)) != NULL)
            pkt_info->rrpd.c2s = TRUE;

        pkt_info->rrpd.session_id = 1;
        pkt_info->rrpd.msg_id = 1;
        pkt_info->rrpd.calculation = RTE_CALC_GTCP;
        pkt_info->rrpd.decode_based = FALSE;
        pkt_info->pkt_of_interest = TRUE;

        return 1;
    }

    return 0;
}

/* Returns the number of sub-packets of interest */
int decode_dns(packet_info *pinfo _U_, proto_tree *tree, PKT_INFO* pkt_info)
{
    guint32 field_uint[MAX_RETURNED_ELEMENTS];  /* An extracted field array for unsigned integers */
    size_t field_value_count;  /* How many entries are there in the extracted field array */

    if (!extract_uint(tree, hf_of_interest[HF_INTEREST_DNS_ID].hf, field_uint, &field_value_count)) {
        if (field_value_count)
            pkt_info->rrpd.msg_id = field_uint[0];
    }

    pkt_info->rrpd.session_id = 1;
    pkt_info->rrpd.suffix = 1;  /* need to do something tricky here as dns.id gets reused */
    pkt_info->rrpd.decode_based = TRUE;
    pkt_info->rrpd.calculation = RTE_CALC_DNS;
    pkt_info->pkt_of_interest = TRUE;

    return 1;
}

/* Returns the number of sub-packets of interest */
int decode_gudp(packet_info *pinfo, proto_tree *tree, PKT_INFO* pkt_info)
{
    guint32 field_uint[MAX_RETURNED_ELEMENTS];  /* An extracted field array for unsigned integers */
    size_t field_value_count;  /* How many entries are there in the extracted field array */

    pkt_info->srcport = pinfo->srcport;
    pkt_info->dstport = pinfo->destport;

    if (!extract_uint(tree, hf_of_interest[HF_INTEREST_UDP_STREAM].hf, field_uint, &field_value_count)) {
        if (field_value_count)
            pkt_info->rrpd.stream_no = field_uint[0];
    }

    if (!extract_uint(tree, hf_of_interest[HF_INTEREST_UDP_LENGTH].hf, field_uint, &field_value_count)) {
        if (field_value_count)
            pkt_info->len = field_uint[0];
    }

    if ((wmem_map_lookup(preferences.udp_svc_ports, GUINT_TO_POINTER(pkt_info->dstport)) != NULL) ||
        (wmem_map_lookup(preferences.udp_svc_ports, GUINT_TO_POINTER(pkt_info->srcport)) != NULL))
    {
        if (wmem_map_lookup(preferences.udp_svc_ports, GUINT_TO_POINTER(pkt_info->dstport)) != NULL)
            pkt_info->rrpd.c2s = TRUE;

        pkt_info->rrpd.session_id = 1;
        pkt_info->rrpd.msg_id = 1;
        pkt_info->rrpd.suffix = 1;
        pkt_info->rrpd.decode_based = FALSE;
        pkt_info->rrpd.calculation = RTE_CALC_GUDP;
        pkt_info->pkt_of_interest = TRUE;
    }

    return 1;
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
