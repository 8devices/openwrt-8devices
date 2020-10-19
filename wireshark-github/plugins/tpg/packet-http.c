/* packet-http.c
 *
 * Wireshark - Network traffic analyzer
 * By Gerald Combs <gerald@wireshark.org>
 * Copyright 1998 Gerald Combs
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include "config.h"

#include <epan/wmem/wmem.h>

#include "http-parser.h"
#include <gmodule.h>

static const value_string http_response_codes[] = {
    { 200, "OK" },
    { 302, "Later" },
    { 0,   NULL }
};

static gint ett_http = -1;
static int proto_http = -1;

static int hf_http_is_response = -1;
static int hf_http_request_method = -1;
static int hf_http_response_code = -1;
static int hf_http_transfer_encoding = -1;
static int hf_http_content_length = -1;
static int hf_http_media = -1;
static int hf_http_host = -1;
static int hf_http_request_uri = -1;

static dissector_handle_t http_handle;

static int dissect_http(tvbuff_t* tvb, packet_info* pinfo _U_, proto_tree* tree, void* data _U_) {
    http_info_value_t* msgdata = wmem_alloc0(wmem_packet_scope(), sizeof(http_info_value_t));
    tvbparse_elem_t* reqresp;
    tpg_parser_data_t* tpg;
    proto_item* pi = proto_tree_add_item(tree,proto_http,tvb,0,-1,ENC_NA);
    proto_tree* pt = proto_item_add_subtree(pi,ett_http);

    tpg = tpg_start(pt,tvb,0,-1,http_tpg_data.wanted_http_sp, msgdata);

    if (( reqresp = TPG_GET(tpg,http_tpg_data.wanted_http_req_resp) )) {
        tvbparse_elem_t* hdr;

        while(( hdr = TPG_GET(tpg,http_tpg_data.wanted_http_header) ))
            ;

        if ( TPG_GET(tpg,http_tpg_data.wanted_http_crlf) ) {
            pi = proto_tree_add_boolean(pt,hf_http_is_response,tvb,0,0,msgdata->is_response);
            pt = proto_item_add_subtree(pi,ett_http);


            if (msgdata->is_response) {
                proto_tree_add_uint(pt,hf_http_response_code,tvb,0,0,msgdata->response_code);
                proto_tree_add_uint(pt,hf_http_content_length,tvb,0,0,msgdata->content_length);
                if (msgdata->transfer_encoding) proto_tree_add_string(pt,hf_http_transfer_encoding,tvb,0,0,msgdata->transfer_encoding);
                if (msgdata->media) proto_tree_add_string(pt,hf_http_media,tvb,0,0,msgdata->media);
            } else {
                if (msgdata->request_method) proto_tree_add_string(pt,hf_http_request_method,tvb,0,0,msgdata->request_method);
                if (msgdata->http_host) proto_tree_add_string(pt,hf_http_host,tvb,0,0,msgdata->http_host);
                if (msgdata->request_uri) proto_tree_add_string(pt,hf_http_request_uri,tvb,0,0,msgdata->request_uri);
            }

    } else {
            /* header fragment */
        }
    } else {
        /* no header  */
    }
    return tvb_captured_length(tvb);
}

static void proto_register_http(void) {
    static hf_register_info hf[] = {
        HF_HTTP_PARSER,
        { &hf_http_is_response, { "=Is Response", "hyttp.info.is_response", FT_BOOLEAN, BASE_NONE, NULL, 0x0, "", HFILL }},
        { &hf_http_request_method, { "=Method", "hyttp.info.method", FT_STRING, BASE_NONE, NULL, 0x0, "", HFILL }},
        { &hf_http_response_code, { "=Response Code", "hyttp.info.response.code", FT_UINT32, BASE_DEC, VALS( http_response_codes ), 0x0, "", HFILL }},
        { &hf_http_transfer_encoding, { "=Transfer-Encoding", "hyttp.info.transfer_encoding", FT_STRING, BASE_NONE, NULL, 0x0, "", HFILL }},
        { &hf_http_content_length, { "=Content-Length", "hyttp.info.content_length", FT_UINT32, BASE_DEC, NULL, 0x0, "", HFILL }},
        { &hf_http_request_uri, { "=Request URI", "hyttp.info.uri", FT_STRING, BASE_NONE, NULL, 0x0, "", HFILL }},
        { &hf_http_media, { "=Media", "hyttp.info.media", FT_STRING, BASE_NONE, NULL, 0x0, "", HFILL }},
        { &hf_http_host, { "=Host", "hyttp.info.host", FT_STRING, BASE_NONE, NULL, 0x0, "", HFILL }}
    };

    gint *ett[] = {
        ETT_HTTP_PARSER,
        &ett_http
    };

    tpg_http_init();

    proto_http = proto_register_protocol("HyTTP", "HyTTP", "hyttp");
    proto_register_field_array(proto_http, hf, array_length(hf));
    proto_register_subtree_array(ett, array_length(ett));
}


static void proto_reg_handoff_http(void) {
    http_handle = create_dissector_handle(dissect_http, proto_http);

    dissector_add_uint_with_preference("tcp.port", 80, http_handle);

}

#ifndef ENABLE_STATIC

WS_DLL_PUBLIC_DEF const gchar version[] = "0.0.0";

WS_DLL_PUBLIC_DEF void
plugin_register(void)
{
    /* register the new protocol, protocol fields, and subtrees */
    if (proto_http == -1) { /* execute protocol initialization only once */
        proto_register_http();
    }
}

WS_DLL_PUBLIC_DEF void
plugin_reg_handoff(void){
    proto_reg_handoff_http();
}

#endif


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
