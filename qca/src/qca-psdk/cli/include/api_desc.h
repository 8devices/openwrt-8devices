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


#ifndef _API_DESC_H_
#define _API_DESC_H_

#ifdef __cplusplus
extern "C" {
#endif                          /* __cplusplus */

#define SW_API_PT_DUPLEX_GET_DESC \
    SW_PARAM_DEF(SW_API_PT_DUPLEX_GET, SW_UINT32, 4, SW_PARAM_IN, "Dev ID"), \
    SW_PARAM_DEF(SW_API_PT_DUPLEX_GET, SW_UINT32, 4, SW_PARAM_IN, "Port No."), \
    SW_PARAM_DEF(SW_API_PT_DUPLEX_GET, SW_DUPLEX, sizeof(fal_port_duplex_t), SW_PARAM_PTR|SW_PARAM_OUT, \
              "duplex"),

#define SW_API_PT_DUPLEX_SET_DESC \
    SW_PARAM_DEF(SW_API_PT_DUPLEX_SET, SW_UINT32, 4, SW_PARAM_IN, "Dev ID"),  \
    SW_PARAM_DEF(SW_API_PT_DUPLEX_SET, SW_UINT32, 4, SW_PARAM_IN, "Port No."), \
    SW_PARAM_DEF(SW_API_PT_DUPLEX_SET, SW_DUPLEX, sizeof(fal_port_duplex_t), SW_PARAM_IN, \
              "duplex"),

#define SW_API_PT_SPEED_GET_DESC \
    SW_PARAM_DEF(SW_API_PT_SPEED_GET, SW_UINT32, 4, SW_PARAM_IN, "Dev ID"), \
    SW_PARAM_DEF(SW_API_PT_SPEED_GET, SW_UINT32, 4, SW_PARAM_IN, "Port No."), \
    SW_PARAM_DEF(SW_API_PT_SPEED_GET, SW_SPEED, sizeof(fal_port_speed_t), SW_PARAM_PTR|SW_PARAM_OUT, \
              "speed"),

#define SW_API_PT_SPEED_SET_DESC \
    SW_PARAM_DEF(SW_API_PT_SPEED_SET, SW_UINT32, 4, SW_PARAM_IN, "Dev ID"), \
    SW_PARAM_DEF(SW_API_PT_SPEED_SET, SW_UINT32, 4, SW_PARAM_IN, "Port No."), \
    SW_PARAM_DEF(SW_API_PT_SPEED_SET, SW_SPEED, sizeof(fal_port_speed_t), SW_PARAM_IN, \
              "speed"),

#define SW_API_PT_AN_GET_DESC \
    SW_PARAM_DEF(SW_API_PT_AN_GET, SW_UINT32, 4, SW_PARAM_IN, "Dev ID"), \
    SW_PARAM_DEF(SW_API_PT_AN_GET, SW_UINT32, 4, SW_PARAM_IN, "Port No."), \
    SW_PARAM_DEF(SW_API_PT_AN_GET, SW_ENABLE, 4, SW_PARAM_PTR|SW_PARAM_OUT, "autoneg"),


#define SW_API_PT_AN_ENABLE_DESC \
    SW_PARAM_DEF(SW_API_PT_AN_ENABLE, SW_UINT32, 4, SW_PARAM_IN, "Dev ID"), \
    SW_PARAM_DEF(SW_API_PT_AN_ENABLE, SW_UINT32, 4, SW_PARAM_IN, "Port No."),


#define SW_API_PT_AN_RESTART_DESC \
    SW_PARAM_DEF(SW_API_PT_AN_RESTART, SW_UINT32, 4, SW_PARAM_IN, "Dev ID"), \
    SW_PARAM_DEF(SW_API_PT_AN_RESTART, SW_UINT32, 4, SW_PARAM_IN, "Port No."),

#define SW_API_PT_AN_ADV_GET_DESC \
    SW_PARAM_DEF(SW_API_PT_AN_ADV_GET, SW_UINT32, 4, SW_PARAM_IN, "Dev ID"), \
    SW_PARAM_DEF(SW_API_PT_AN_ADV_GET, SW_UINT32, 4, SW_PARAM_IN, "Port No."), \
    SW_PARAM_DEF(SW_API_PT_AN_ADV_GET, SW_CAP, 4, SW_PARAM_PTR|SW_PARAM_OUT, "autoneg"),

#define SW_API_PT_AN_ADV_SET_DESC \
    SW_PARAM_DEF(SW_API_PT_AN_ADV_SET, SW_UINT32, 4, SW_PARAM_IN, "Dev ID"), \
    SW_PARAM_DEF(SW_API_PT_AN_ADV_SET, SW_UINT32, 4, SW_PARAM_IN, "Port No."), \
    SW_PARAM_DEF(SW_API_PT_AN_ADV_SET, SW_CAP, 4, SW_PARAM_IN, "autoneg"),
#define SW_API_PT_HIBERNATE_SET_DESC \
    SW_PARAM_DEF(SW_API_PT_HIBERNATE_SET, SW_UINT32, 4, SW_PARAM_IN, "Dev ID"), \
    SW_PARAM_DEF(SW_API_PT_HIBERNATE_SET, SW_UINT32, 4, SW_PARAM_IN, "Port ID"), \
    SW_PARAM_DEF(SW_API_PT_HIBERNATE_SET, SW_ENABLE, 4, SW_PARAM_IN, "Hibernate status"),

#define SW_API_PT_HIBERNATE_GET_DESC \
    SW_PARAM_DEF(SW_API_PT_HIBERNATE_GET, SW_UINT32, 4, SW_PARAM_IN, "Dev ID"), \
    SW_PARAM_DEF(SW_API_PT_HIBERNATE_GET, SW_UINT32, 4, SW_PARAM_IN, "Port ID"), \
    SW_PARAM_DEF(SW_API_PT_HIBERNATE_GET, SW_ENABLE, 4, SW_PARAM_PTR|SW_PARAM_OUT, "Hibernate Status"),

#define SW_API_PT_CDT_DESC \
    SW_PARAM_DEF(SW_API_PT_CDT, SW_UINT32, 4, SW_PARAM_IN, "Dev ID"), \
    SW_PARAM_DEF(SW_API_PT_CDT, SW_UINT32, 4, SW_PARAM_IN, "Port ID"), \
    SW_PARAM_DEF(SW_API_PT_CDT, SW_UINT32, 4, SW_PARAM_IN, "MDI Pair ID"), \
    SW_PARAM_DEF(SW_API_PT_CDT, SW_CABLESTATUS, 4, SW_PARAM_PTR|SW_PARAM_OUT, "cable status"), \
    SW_PARAM_DEF(SW_API_PT_CDT, SW_CABLELEN, 4, SW_PARAM_PTR|SW_PARAM_OUT, "cable len"),
#define SW_API_PT_LINK_STATUS_GET_DESC \
    SW_PARAM_DEF(SW_API_PT_LINK_STATUS_GET, SW_UINT32, 4, SW_PARAM_IN, "Dev ID"), \
    SW_PARAM_DEF(SW_API_PT_LINK_STATUS_GET, SW_UINT32, 4, SW_PARAM_IN, "Port ID"), \
    SW_PARAM_DEF(SW_API_PT_LINK_STATUS_GET, SW_ENABLE, 4, SW_PARAM_PTR|SW_PARAM_OUT, "Status"),
#define SW_API_PT_8023AZ_SET_DESC \
    SW_PARAM_DEF(SW_API_PT_8023AZ_SET, SW_UINT32, 4, SW_PARAM_IN, "Dev ID"), \
    SW_PARAM_DEF(SW_API_PT_8023AZ_SET, SW_UINT32, 4, SW_PARAM_IN, "Port ID"), \
    SW_PARAM_DEF(SW_API_PT_8023AZ_SET, SW_ENABLE, 4, SW_PARAM_IN, "8023az Status"),

#define SW_API_PT_8023AZ_GET_DESC \
    SW_PARAM_DEF(SW_API_PT_8023AZ_GET, SW_UINT32, 4, SW_PARAM_IN, "Dev ID"), \
    SW_PARAM_DEF(SW_API_PT_8023AZ_GET, SW_UINT32, 4, SW_PARAM_IN, "Port ID"), \
    SW_PARAM_DEF(SW_API_PT_8023AZ_GET, SW_ENABLE, 4, SW_PARAM_PTR|SW_PARAM_OUT, "8023az Status"),

#define SW_API_PT_MDIX_SET_DESC \
    SW_PARAM_DEF(SW_API_PT_MDIX_SET, SW_UINT32, 4, SW_PARAM_IN, "Dev ID"), \
    SW_PARAM_DEF(SW_API_PT_MDIX_SET, SW_UINT32, 4, SW_PARAM_IN, "Port ID"), \
    SW_PARAM_DEF(SW_API_PT_MDIX_SET, SW_CROSSOVER_MODE, sizeof(fal_port_mdix_mode_t), SW_PARAM_IN, "Crossover Mode"),


#define SW_API_PT_MDIX_GET_DESC \
    SW_PARAM_DEF(SW_API_PT_MDIX_GET, SW_UINT32, 4, SW_PARAM_IN, "Dev ID"), \
    SW_PARAM_DEF(SW_API_PT_MDIX_GET, SW_UINT32, 4, SW_PARAM_IN, "Port ID"), \
    SW_PARAM_DEF(SW_API_PT_MDIX_GET, SW_CROSSOVER_MODE, sizeof(fal_port_mdix_mode_t), SW_PARAM_PTR|SW_PARAM_OUT, \
              "Crossover Mode"),

#define SW_API_PT_MDIX_STATUS_GET_DESC \
    SW_PARAM_DEF(SW_API_PT_MDIX_STATUS_GET, SW_UINT32, 4, SW_PARAM_IN, "Dev ID"), \
    SW_PARAM_DEF(SW_API_PT_MDIX_STATUS_GET, SW_UINT32, 4, SW_PARAM_IN, "Port ID"), \
    SW_PARAM_DEF(SW_API_PT_MDIX_STATUS_GET, SW_CROSSOVER_STATUS, sizeof(fal_port_mdix_status_t), SW_PARAM_PTR|SW_PARAM_OUT, \
              "Crossover Status"),
#define SW_API_PT_LOCAL_LOOPBACK_SET_DESC \
    SW_PARAM_DEF(SW_API_PT_LOCAL_LOOPBACK_SET, SW_UINT32, 4, SW_PARAM_IN, "Dev ID"), \
    SW_PARAM_DEF(SW_API_PT_LOCAL_LOOPBACK_SET, SW_UINT32, 4, SW_PARAM_IN, "Port ID"), \
    SW_PARAM_DEF(SW_API_PT_LOCAL_LOOPBACK_SET, SW_ENABLE, 4, SW_PARAM_IN, "Local Loopback Status"),

#define SW_API_PT_LOCAL_LOOPBACK_GET_DESC \
    SW_PARAM_DEF(SW_API_PT_LOCAL_LOOPBACK_GET, SW_UINT32, 4, SW_PARAM_IN, "Dev ID"), \
    SW_PARAM_DEF(SW_API_PT_LOCAL_LOOPBACK_GET, SW_UINT32, 4, SW_PARAM_IN, "Port ID"), \
    SW_PARAM_DEF(SW_API_PT_LOCAL_LOOPBACK_GET, SW_ENABLE, 4, SW_PARAM_PTR|SW_PARAM_OUT, "Local Loopback Status"),

#define SW_API_PT_REMOTE_LOOPBACK_SET_DESC \
    SW_PARAM_DEF(SW_API_PT_REMOTE_LOOPBACK_SET, SW_UINT32, 4, SW_PARAM_IN, "Dev ID"), \
    SW_PARAM_DEF(SW_API_PT_REMOTE_LOOPBACK_SET, SW_UINT32, 4, SW_PARAM_IN, "Port ID"), \
    SW_PARAM_DEF(SW_API_PT_REMOTE_LOOPBACK_SET, SW_ENABLE, 4, SW_PARAM_IN, "Remote Loopback Status"),

#define SW_API_PT_REMOTE_LOOPBACK_GET_DESC \
    SW_PARAM_DEF(SW_API_PT_REMOTE_LOOPBACK_GET, SW_UINT32, 4, SW_PARAM_IN, "Dev ID"), \
    SW_PARAM_DEF(SW_API_PT_REMOTE_LOOPBACK_GET, SW_UINT32, 4, SW_PARAM_IN, "Port ID"), \
    SW_PARAM_DEF(SW_API_PT_REMOTE_LOOPBACK_GET, SW_ENABLE, 4, SW_PARAM_PTR|SW_PARAM_OUT, "Remote Loopback Status"),

#define SW_API_PT_RESET_DESC \
    SW_PARAM_DEF(SW_API_PT_RESET, SW_UINT32, 4, SW_PARAM_IN, "Dev ID"), \
    SW_PARAM_DEF(SW_API_PT_RESET, SW_UINT32, 4, SW_PARAM_IN, "Port ID"),

#define SW_API_PT_POWER_OFF_DESC \
    SW_PARAM_DEF(SW_API_PT_POWER_OFF, SW_UINT32, 4, SW_PARAM_IN, "Dev ID"), \
    SW_PARAM_DEF(SW_API_PT_POWER_OFF, SW_UINT32, 4, SW_PARAM_IN, "Port ID"),

#define SW_API_PT_POWER_ON_DESC \
    SW_PARAM_DEF(SW_API_PT_POWER_ON, SW_UINT32, 4, SW_PARAM_IN, "Dev ID"), \
    SW_PARAM_DEF(SW_API_PT_POWER_ON, SW_UINT32, 4, SW_PARAM_IN, "Port ID"),

#define SW_API_PT_MAGIC_FRAME_MAC_SET_DESC \
    SW_PARAM_DEF(SW_API_PT_MAGIC_FRAME_MAC_SET, SW_UINT32, 4, SW_PARAM_IN, "Dev ID"), \
    SW_PARAM_DEF(SW_API_PT_MAGIC_FRAME_MAC_SET, SW_UINT32, 4, SW_PARAM_IN, "Port ID"), \
    SW_PARAM_DEF(SW_API_PT_MAGIC_FRAME_MAC_SET, SW_MACADDR, sizeof(fal_mac_addr_t), SW_PARAM_PTR|SW_PARAM_IN, "[Magic mac]"),

#define SW_API_PT_MAGIC_FRAME_MAC_GET_DESC \
    SW_PARAM_DEF(SW_API_PT_MAGIC_FRAME_MAC_GET, SW_UINT32, 4, SW_PARAM_IN, "Dev ID"), \
    SW_PARAM_DEF(SW_API_PT_MAGIC_FRAME_MAC_GET, SW_UINT32, 4, SW_PARAM_IN, "Port ID"), \
    SW_PARAM_DEF(SW_API_PT_MAGIC_FRAME_MAC_GET, SW_MACADDR, sizeof(fal_mac_addr_t), SW_PARAM_PTR|SW_PARAM_OUT, "[Magic mac]"),

#define SW_API_PT_PHY_ID_GET_DESC \
    SW_PARAM_DEF(SW_API_PT_PHY_ID_GET, SW_UINT32, 4, SW_PARAM_IN, "Dev ID"), \
    SW_PARAM_DEF(SW_API_PT_PHY_ID_GET, SW_UINT32, 4, SW_PARAM_IN, "Port ID"), \
    SW_PARAM_DEF(SW_API_PT_PHY_ID_GET, SW_UINT16, 2, SW_PARAM_PTR|SW_PARAM_OUT, "Org ID"), \
    SW_PARAM_DEF(SW_API_PT_PHY_ID_GET, SW_UINT16, 2, SW_PARAM_PTR|SW_PARAM_OUT, "Rev ID"),

#define SW_API_PT_WOL_STATUS_SET_DESC \
    SW_PARAM_DEF(SW_API_PT_WOL_STATUS_SET, SW_UINT32, 4, SW_PARAM_IN, "Dev ID"), \
    SW_PARAM_DEF(SW_API_PT_WOL_STATUS_SET, SW_UINT32, 4, SW_PARAM_IN, "Port ID"), \
    SW_PARAM_DEF(SW_API_PT_WOL_STATUS_SET, SW_ENABLE, 4, SW_PARAM_IN, "Wol Status"),

#define SW_API_PT_WOL_STATUS_GET_DESC \
    SW_PARAM_DEF(SW_API_PT_WOL_STATUS_GET, SW_UINT32, 4, SW_PARAM_IN, "Dev ID"), \
    SW_PARAM_DEF(SW_API_PT_WOL_STATUS_GET, SW_UINT32, 4, SW_PARAM_IN, "Port ID"), \
    SW_PARAM_DEF(SW_API_PT_WOL_STATUS_GET, SW_ENABLE, 4, SW_PARAM_PTR|SW_PARAM_OUT, "Wol Status"),
#define SW_API_PT_INTERFACE_MODE_STATUS_GET_DESC \
    SW_PARAM_DEF(SW_API_PT_INTERFACE_MODE_STATUS_GET, SW_UINT32, 4, SW_PARAM_IN, "Dev ID"), \
    SW_PARAM_DEF(SW_API_PT_INTERFACE_MODE_STATUS_GET, SW_UINT32, 4, SW_PARAM_IN, "Port ID"), \
    SW_PARAM_DEF(SW_API_PT_INTERFACE_MODE_STATUS_GET, SW_INTERFACE_MODE, sizeof(fal_port_interface_mode_t), SW_PARAM_PTR|SW_PARAM_OUT, \
              "Interface Status"),

#define SW_API_DEBUG_PHYCOUNTER_SET_DESC \
			SW_PARAM_DEF(SW_API_DEBUG_PHYCOUNTER_SET, SW_UINT32, 4, SW_PARAM_IN, "Dev ID"), \
			SW_PARAM_DEF(SW_API_DEBUG_PHYCOUNTER_SET, SW_UINT32, 4, SW_PARAM_IN, "Port ID"), \
			SW_PARAM_DEF(SW_API_DEBUG_PHYCOUNTER_SET, SW_ENABLE, 4, SW_PARAM_IN, "Counter Status"),

#define SW_API_DEBUG_PHYCOUNTER_GET_DESC \
			SW_PARAM_DEF(SW_API_DEBUG_PHYCOUNTER_GET, SW_UINT32, 4, SW_PARAM_IN, "Dev ID"), \
			SW_PARAM_DEF(SW_API_DEBUG_PHYCOUNTER_GET, SW_UINT32, 4, SW_PARAM_IN, "Port ID"), \
			SW_PARAM_DEF(SW_API_DEBUG_PHYCOUNTER_GET, SW_ENABLE, 4, SW_PARAM_PTR|SW_PARAM_OUT, "Counter Status"),

#define SW_API_DEBUG_PHYCOUNTER_SHOW_DESC \
		SW_PARAM_DEF(SW_API_DEBUG_PHYCOUNTER_SHOW, SW_UINT32, 4, SW_PARAM_IN, "Dev ID"), \
		SW_PARAM_DEF(SW_API_DEBUG_PHYCOUNTER_SHOW, SW_UINT32, 4, SW_PARAM_IN, "Port ID"), \
		SW_PARAM_DEF(SW_API_DEBUG_PHYCOUNTER_SHOW, SW_COUNTER_INFO, sizeof(fal_port_counter_info_t), SW_PARAM_PTR|SW_PARAM_OUT, \
				  "Phy Counter Statistics On port"),
#define SW_API_PHY_GET_DESC \
    SW_PARAM_DEF(SW_API_PHY_GET, SW_UINT32, 4, SW_PARAM_IN, "Dev ID"),\
    SW_PARAM_DEF(SW_API_PHY_GET, SW_UINT32, 4, SW_PARAM_IN, "Phy ID"),\
    SW_PARAM_DEF(SW_API_PHY_GET, SW_UINT32, 4, SW_PARAM_IN, "Reg ID"),\
    SW_PARAM_DEF(SW_API_PHY_GET, SW_UINT16, 2, SW_PARAM_PTR|SW_PARAM_OUT, "Data"),


#define SW_API_PHY_SET_DESC \
    SW_PARAM_DEF(SW_API_PHY_SET, SW_UINT32, 4, SW_PARAM_IN, "Dev ID"),\
    SW_PARAM_DEF(SW_API_PHY_SET, SW_UINT32, 4, SW_PARAM_IN, "Phy ID"),\
    SW_PARAM_DEF(SW_API_PHY_SET, SW_UINT32, 4, SW_PARAM_IN, "Reg ID"),\
    SW_PARAM_DEF(SW_API_PHY_SET, SW_UINT16, 2, SW_PARAM_IN, "Data"),
#define SW_API_DESC(api_id) api_id##_DESC

#ifdef __cplusplus
}
#endif                          /* __cplusplus */

#endif                          /* _API_DESC_H_ */
