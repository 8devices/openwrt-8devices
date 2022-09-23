/*
 * Copyright (c) 2012, 2015-2019, The Linux Foundation. All rights reserved.
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


#ifndef _FAL_API_H_
#define _FAL_API_H_

#ifdef __cplusplus
extern "C" {
#endif                          /* __cplusplus */
#define PORTCONTROL_API \
    SW_API_DEF(SW_API_PT_DUPLEX_GET, fal_port_duplex_get), \
    SW_API_DEF(SW_API_PT_DUPLEX_SET, fal_port_duplex_set), \
    SW_API_DEF(SW_API_PT_SPEED_GET, fal_port_speed_get), \
    SW_API_DEF(SW_API_PT_SPEED_SET, fal_port_speed_set), \
    SW_API_DEF(SW_API_PT_AN_GET, fal_port_autoneg_status_get), \
    SW_API_DEF(SW_API_PT_AN_ENABLE, fal_port_autoneg_enable), \
    SW_API_DEF(SW_API_PT_AN_RESTART, fal_port_autoneg_restart), \
    SW_API_DEF(SW_API_PT_AN_ADV_GET, fal_port_autoneg_adv_get), \
    SW_API_DEF(SW_API_PT_AN_ADV_SET, fal_port_autoneg_adv_set), \
    SW_API_DEF(SW_API_PT_HIBERNATE_SET, fal_port_hibernate_set), \
    SW_API_DEF(SW_API_PT_HIBERNATE_GET, fal_port_hibernate_get), \
    SW_API_DEF(SW_API_PT_CDT, fal_port_cdt),  \
    SW_API_DEF(SW_API_PT_LINK_STATUS_GET, fal_port_link_status_get), \
    SW_API_DEF(SW_API_PT_8023AZ_SET, fal_port_8023az_set), \
    SW_API_DEF(SW_API_PT_8023AZ_GET, fal_port_8023az_get), \
    SW_API_DEF(SW_API_PT_MDIX_SET, fal_port_mdix_set), \
    SW_API_DEF(SW_API_PT_MDIX_GET, fal_port_mdix_get), \
    SW_API_DEF(SW_API_PT_MDIX_STATUS_GET, fal_port_mdix_status_get), \
    SW_API_DEF(SW_API_PT_LOCAL_LOOPBACK_SET, fal_port_local_loopback_set), \
    SW_API_DEF(SW_API_PT_LOCAL_LOOPBACK_GET, fal_port_local_loopback_get), \
    SW_API_DEF(SW_API_PT_REMOTE_LOOPBACK_SET, fal_port_remote_loopback_set), \
    SW_API_DEF(SW_API_PT_REMOTE_LOOPBACK_GET, fal_port_remote_loopback_get), \
    SW_API_DEF(SW_API_PT_RESET, fal_port_reset), \
    SW_API_DEF(SW_API_PT_POWER_OFF, fal_port_power_off), \
    SW_API_DEF(SW_API_PT_POWER_ON, fal_port_power_on), \
    SW_API_DEF(SW_API_PT_MAGIC_FRAME_MAC_SET, fal_port_magic_frame_mac_set), \
    SW_API_DEF(SW_API_PT_MAGIC_FRAME_MAC_GET, fal_port_magic_frame_mac_get), \
    SW_API_DEF(SW_API_PT_PHY_ID_GET, fal_port_phy_id_get), \
    SW_API_DEF(SW_API_PT_WOL_STATUS_SET, fal_port_wol_status_set), \
    SW_API_DEF(SW_API_PT_WOL_STATUS_GET, fal_port_wol_status_get), \
    SW_API_DEF(SW_API_PT_INTERFACE_MODE_STATUS_GET, fal_port_interface_mode_status_get), \
    SW_API_DEF(SW_API_DEBUG_PHYCOUNTER_SET, fal_debug_phycounter_set), \
    SW_API_DEF(SW_API_DEBUG_PHYCOUNTER_GET, fal_debug_phycounter_get), \
    SW_API_DEF(SW_API_DEBUG_PHYCOUNTER_SHOW, fal_debug_phycounter_show),\
/*end of PORTCONTROL_API*/
#define PORTCONTROL_API_PARAM \
    SW_API_DESC(SW_API_PT_DUPLEX_GET) \
    SW_API_DESC(SW_API_PT_DUPLEX_SET) \
    SW_API_DESC(SW_API_PT_SPEED_GET)  \
    SW_API_DESC(SW_API_PT_SPEED_SET)  \
    SW_API_DESC(SW_API_PT_AN_GET) \
    SW_API_DESC(SW_API_PT_AN_ENABLE) \
    SW_API_DESC(SW_API_PT_AN_RESTART) \
    SW_API_DESC(SW_API_PT_AN_ADV_GET) \
    SW_API_DESC(SW_API_PT_AN_ADV_SET) \
    SW_API_DESC(SW_API_PT_HIBERNATE_SET) \
    SW_API_DESC(SW_API_PT_HIBERNATE_GET) \
    SW_API_DESC(SW_API_PT_CDT) \
    SW_API_DESC(SW_API_PT_LINK_STATUS_GET) \
    SW_API_DESC(SW_API_PT_8023AZ_SET) \
    SW_API_DESC(SW_API_PT_8023AZ_GET) \
    SW_API_DESC(SW_API_PT_MDIX_SET) \
    SW_API_DESC(SW_API_PT_MDIX_GET) \
    SW_API_DESC(SW_API_PT_MDIX_STATUS_GET) \
    SW_API_DESC(SW_API_PT_LOCAL_LOOPBACK_SET) \
    SW_API_DESC(SW_API_PT_LOCAL_LOOPBACK_GET) \
    SW_API_DESC(SW_API_PT_REMOTE_LOOPBACK_SET) \
    SW_API_DESC(SW_API_PT_REMOTE_LOOPBACK_GET) \
    SW_API_DESC(SW_API_PT_RESET) \
    SW_API_DESC(SW_API_PT_POWER_OFF) \
    SW_API_DESC(SW_API_PT_POWER_ON) \
    SW_API_DESC(SW_API_PT_MAGIC_FRAME_MAC_SET) \
    SW_API_DESC(SW_API_PT_MAGIC_FRAME_MAC_GET) \
    SW_API_DESC(SW_API_PT_PHY_ID_GET) \
    SW_API_DESC(SW_API_PT_WOL_STATUS_SET) \
    SW_API_DESC(SW_API_PT_WOL_STATUS_GET) \
    SW_API_DESC(SW_API_PT_INTERFACE_MODE_STATUS_GET) \
    SW_API_DESC(SW_API_DEBUG_PHYCOUNTER_SET) \
    SW_API_DESC(SW_API_DEBUG_PHYCOUNTER_GET) \
    SW_API_DESC(SW_API_DEBUG_PHYCOUNTER_SHOW) \
/*end of PORTCONTROL_API_PARAM*/
#define REG_API \
    SW_API_DEF(SW_API_PHY_GET, fal_phy_get), \
	SW_API_DEF(SW_API_PHY_SET, fal_phy_set), \
/*end of REG_API*/
#define REG_API_PARAM \
    SW_API_DESC(SW_API_PHY_GET) \
    SW_API_DESC(SW_API_PHY_SET) \
/*end of REG_API_PARAM*/
#define SSDK_API \
    PORTCONTROL_API \
    REG_API \
    SW_API_DEF(SW_API_MAX, NULL),


#define SSDK_PARAM  \
    PORTCONTROL_API_PARAM \
    REG_API_PARAM \
    SW_PARAM_DEF(SW_API_MAX, SW_UINT32, 4, SW_PARAM_IN, "Dev ID"),


#ifdef __cplusplus
}
#endif                          /* __cplusplus */

#endif                          /* _FAL_API_H_ */
