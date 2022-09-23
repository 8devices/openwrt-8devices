/*
 * Copyright (c) 2014-2019, The Linux Foundation. All rights reserved.
 * Permission to use, copy, modify, and/or distribute this software for
 * any purpose with or without fee is hereby granted, provided that the
 * above copyright notice and this permission notice appear in all copies.
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT
 * OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */


#include "shell_config.h"
#include "shell_sw.h"


/*cmdline tree descript*/
struct cmd_des_t gcmd_des[] =
{
    /*port ctrl*/
    {
        "port", "config port control",
        {
            {"duplex", "get", "get duplex mode of a port", "<port_id>" , SW_API_PT_DUPLEX_GET,
                NULL},
            {"duplex", "set", "set duplex mode of a port", "<port_id> <half|full>",
                SW_API_PT_DUPLEX_SET, NULL},
            {"speed", "get", "get speed mode of a port", "<port_id>",  SW_API_PT_SPEED_GET,
                NULL},
            {"speed", "set", "set speed mode of a port",
                "<port_id> <10|100|1000|2500|5000|10000>", SW_API_PT_SPEED_SET, NULL},
            {"autoAdv", "get", "get auto-negotiation advertisement of a port", "<port_id>",
                SW_API_PT_AN_ADV_GET, NULL},
            {"autoAdv", "set", "set auto-negotiation advertisement of a port",
                "<port_id> <cap_bitmap>", SW_API_PT_AN_ADV_SET, NULL},
            {"autoNeg", "get", "get auto-negotiation status of a port", "<port_id>",
                SW_API_PT_AN_GET, NULL},
            {"autoNeg", "enable", "enable auto-negotiation of a port", "<port_id>",
                SW_API_PT_AN_ENABLE, NULL},
            {"autoNeg", "restart", "restart auto-negotiation process of a port", "<port_id>",
                SW_API_PT_AN_RESTART, NULL},
            {"hibernate", "set", "set hibernate status of a port",
                "<port_id> <enable|disable>", SW_API_PT_HIBERNATE_SET, NULL},
            {"hibernate", "get", "get hibernate status of a port", "<port_id>",
                SW_API_PT_HIBERNATE_GET, NULL},
            {"cdt", "run", "run cable diagnostic test of a port",
                "<port_id> <mdi_pair>", SW_API_PT_CDT, NULL},
            {"linkstatus", "get", "get link status of a port", "<port_id>",
                SW_API_PT_LINK_STATUS_GET, NULL},
            {"Ieee8023az", "set", "set 8023az status of a port", "<port_id> <enable|disable>",
                SW_API_PT_8023AZ_SET, NULL},
            {"Ieee8023az", "get", "get 8023az status of a port", "<port_id>",
                SW_API_PT_8023AZ_GET, NULL},
            {"crossover", "set", "set crossover mode of a port", "<port_id> <auto|mdi|mdix>",
                SW_API_PT_MDIX_SET, NULL},
            {"crossover", "get", "get crossover mode of a port", "<port_id>",
                SW_API_PT_MDIX_GET, NULL},
            {"crossover", "status", "get current crossover status of a port", "<port_id>",
                SW_API_PT_MDIX_STATUS_GET, NULL},
            {"localLoopback", "set", "set local loopback of a port",
                "<port_id> <enable|disable>", SW_API_PT_LOCAL_LOOPBACK_SET, NULL},
            {"localLoopback", "get", "get local loopback of a port", "<port_id>",
                SW_API_PT_LOCAL_LOOPBACK_GET, NULL},
            {"remoteLoopback", "set", "set remote loopback of a port",
                "<port_id> <enable|disable>", SW_API_PT_REMOTE_LOOPBACK_SET, NULL},
            {"remoteLoopback", "get", "get remote loopback of a port", "<port_id>",
                SW_API_PT_REMOTE_LOOPBACK_GET, NULL},
            {"reset", "set", "reset phy of a port", "<port_id>", SW_API_PT_RESET, NULL},
            {"poweroff", "set", "power off phy of a port", "<port_id>",
                SW_API_PT_POWER_OFF, NULL},
            {"poweron", "set", "power on phy of a port", "<port_id>", SW_API_PT_POWER_ON, NULL},
            {"magicFrameMac", "set", "set magic frame mac address  of a port",
                "<port_id> <mac_address>", SW_API_PT_MAGIC_FRAME_MAC_SET, NULL},
            {"magicFrameMac", "get", "get magic frame mac address  of a port", "<port_id>",
                SW_API_PT_MAGIC_FRAME_MAC_GET, NULL},
            {"phyId", "get", "get phy id of a port", "<port_id>", SW_API_PT_PHY_ID_GET, NULL},
            {"wolstatus", "set", "set wol status of a port", "<port_id> <enable|disable>",
                SW_API_PT_WOL_STATUS_SET, NULL},
            {"wolstatus", "get", "get wol status of a port", "<port_id>",
                SW_API_PT_WOL_STATUS_GET, NULL},
            {"interfaceMode", "status", "get current interface mode of phy", "<port_id>",
                SW_API_PT_INTERFACE_MODE_STATUS_GET, NULL},
            {NULL, NULL, NULL, NULL, SW_API_INVALID, NULL},/*end of desc*/
        },
    },
    /* debug */
    {
        "debug", "read/write register",
        {
            {"phy", "get", "read phy register", "<ph_id> <reg_addr>", SW_API_PHY_GET, NULL},
            {"phy", "set", "write phy register", "<ph_id> <reg_addr> <value>", SW_API_PHY_SET, NULL},
            {"phycounter",  "set", "set counter status of a port",   "<port_id> <enable|disable>", SW_API_DEBUG_PHYCOUNTER_SET, NULL},
            {"phycounter",  "get", "get counter status of a port",   "<port_id>", SW_API_DEBUG_PHYCOUNTER_GET, NULL},
            {"phycounter",  "show", "show counter of a port",   "<port_id>", SW_API_DEBUG_PHYCOUNTER_SHOW, NULL},
            {NULL, NULL, NULL, NULL, SW_API_INVALID, NULL}/*end of desc*/
        },
    },
    {"help", "type ? get help", {{NULL, NULL, NULL, NULL, SW_API_INVALID, NULL}/*end of desc*/}},

    {"quit", "type quit/q quit shell", {{NULL, NULL, NULL, NULL, SW_API_INVALID, NULL}/*end of desc*/}},

    {NULL, NULL, {{NULL, NULL, NULL, NULL, SW_API_INVALID, NULL}}} /*end of desc*/
};
