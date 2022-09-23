/*
 * Copyright (c) 2014,2016-2019, The Linux Foundation. All rights reserved.
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


#include "sw.h"
#include "sw_ioctl.h"
#include "fal_port_ctrl.h"
#include "fal_uk_if.h"

sw_error_t
fal_port_duplex_set(a_uint32_t dev_id, fal_port_t port_id,
                    fal_port_duplex_t duplex)
{
    sw_error_t rv;

    rv = sw_uk_exec(SW_API_PT_DUPLEX_SET, dev_id, port_id,
                   duplex);
    return rv;
}

sw_error_t
fal_port_duplex_get(a_uint32_t dev_id, fal_port_t port_id,
                    fal_port_duplex_t * pduplex)
{
    sw_error_t rv;

    rv = sw_uk_exec(SW_API_PT_DUPLEX_GET, dev_id, port_id, pduplex);
    return rv;
}

sw_error_t
fal_port_speed_set(a_uint32_t dev_id, fal_port_t port_id,
                   fal_port_speed_t speed)
{
    sw_error_t rv;

    rv = sw_uk_exec(SW_API_PT_SPEED_SET, dev_id, port_id,
                   speed);
    return rv;
}

sw_error_t
fal_port_speed_get(a_uint32_t dev_id, fal_port_t port_id,
                   fal_port_speed_t * pspeed)
{
    sw_error_t rv;

    rv = sw_uk_exec(SW_API_PT_SPEED_GET, dev_id, port_id, pspeed);
    return rv;
}

sw_error_t
fal_port_autoneg_status_get(a_uint32_t dev_id, fal_port_t port_id,
                            a_bool_t * status)
{
    sw_error_t rv;

    rv = sw_uk_exec(SW_API_PT_AN_GET, dev_id, port_id, status);
    return rv;
}

sw_error_t
fal_port_autoneg_enable(a_uint32_t dev_id, fal_port_t port_id)
{
    sw_error_t rv;

    rv = sw_uk_exec(SW_API_PT_AN_ENABLE, dev_id, port_id);
    return rv;
}

sw_error_t
fal_port_autoneg_restart(a_uint32_t dev_id, fal_port_t port_id)
{
    sw_error_t rv;

    rv = sw_uk_exec(SW_API_PT_AN_RESTART, dev_id, port_id);
    return rv;
}

sw_error_t
fal_port_autoneg_adv_set(a_uint32_t dev_id, fal_port_t port_id,
                         a_uint32_t autoadv)
{
    sw_error_t rv;

    rv = sw_uk_exec(SW_API_PT_AN_ADV_SET, dev_id, port_id, autoadv);
    return rv;
}

sw_error_t
fal_port_autoneg_adv_get(a_uint32_t dev_id, fal_port_t port_id,
                         a_uint32_t * autoadv)
{
    sw_error_t rv;

    rv = sw_uk_exec(SW_API_PT_AN_ADV_GET, dev_id, port_id, autoadv);
    return rv;
}
sw_error_t
fal_port_hibernate_set(a_uint32_t dev_id, fal_port_t port_id,
                       a_bool_t enable)
{
    sw_error_t rv;

    rv = sw_uk_exec(SW_API_PT_HIBERNATE_SET, dev_id, port_id,
                    enable);
    return rv;
}

sw_error_t
fal_port_hibernate_get(a_uint32_t dev_id, fal_port_t port_id,
                       a_bool_t * enable)
{
    sw_error_t rv;

    rv = sw_uk_exec(SW_API_PT_HIBERNATE_GET, dev_id, port_id, enable);
    return rv;
}

sw_error_t
fal_port_cdt(a_uint32_t dev_id, fal_port_t port_id, a_uint32_t mdi_pair,
             a_uint32_t *cable_status, a_uint32_t *cable_len)
{
    sw_error_t rv;

    rv = sw_uk_exec(SW_API_PT_CDT, dev_id, port_id, mdi_pair,
                    cable_status, cable_len);
    return rv;
}
sw_error_t
fal_port_link_status_get(a_uint32_t dev_id, fal_port_t port_id, a_bool_t * status)
{
    sw_error_t rv;

    rv = sw_uk_exec(SW_API_PT_LINK_STATUS_GET, dev_id, port_id, status);
    return rv;
}
sw_error_t
fal_port_8023az_set(a_uint32_t dev_id, fal_port_t port_id,
                       a_bool_t enable)
{
    sw_error_t rv;

    rv = sw_uk_exec(SW_API_PT_8023AZ_SET, dev_id, port_id,
                    enable);
    return rv;
}

sw_error_t
fal_port_8023az_get(a_uint32_t dev_id, fal_port_t port_id,
                       a_bool_t * enable)
{
    sw_error_t rv;

    rv = sw_uk_exec(SW_API_PT_8023AZ_GET, dev_id, port_id, enable);
    return rv;
}

sw_error_t
fal_port_mdix_set(a_uint32_t dev_id, fal_port_t port_id,
                        fal_port_mdix_mode_t mode)
{
    sw_error_t rv;

    rv = sw_uk_exec(SW_API_PT_MDIX_SET, dev_id, port_id, mode);
    return rv;
}

sw_error_t
fal_port_mdix_get(a_uint32_t dev_id, fal_port_t port_id,
                        fal_port_mdix_mode_t * mode)
{
    sw_error_t rv;

    rv = sw_uk_exec(SW_API_PT_MDIX_GET, dev_id, port_id, mode);
    return rv;
}

sw_error_t
fal_port_mdix_status_get(a_uint32_t dev_id, fal_port_t port_id,
                        fal_port_mdix_status_t * mode)
{
    sw_error_t rv;

    rv = sw_uk_exec(SW_API_PT_MDIX_STATUS_GET, dev_id, port_id, mode);
    return rv;
}
sw_error_t
fal_port_local_loopback_set(a_uint32_t dev_id, fal_port_t port_id,
                       a_bool_t enable)
{
    sw_error_t rv;

    rv = sw_uk_exec(SW_API_PT_LOCAL_LOOPBACK_SET, dev_id, port_id,
                     enable);
    return rv;
}

sw_error_t
fal_port_local_loopback_get(a_uint32_t dev_id, fal_port_t port_id,
                       a_bool_t * enable)
{
    sw_error_t rv;

    rv = sw_uk_exec(SW_API_PT_LOCAL_LOOPBACK_GET, dev_id, port_id, enable);
    return rv;
}

sw_error_t
fal_port_remote_loopback_set(a_uint32_t dev_id, fal_port_t port_id,
                       a_bool_t enable)
{
    sw_error_t rv;

    rv = sw_uk_exec(SW_API_PT_REMOTE_LOOPBACK_SET, dev_id, port_id,
                    enable);
    return rv;
}

sw_error_t
fal_port_remote_loopback_get(a_uint32_t dev_id, fal_port_t port_id,
                       a_bool_t * enable)
{
    sw_error_t rv;

    rv = sw_uk_exec(SW_API_PT_REMOTE_LOOPBACK_GET, dev_id, port_id, enable);
    return rv;
}

sw_error_t
fal_port_reset(a_uint32_t dev_id, fal_port_t port_id)
{
    sw_error_t rv;

    rv = sw_uk_exec(SW_API_PT_RESET, dev_id, port_id);
    return rv;
}

sw_error_t
fal_port_power_off(a_uint32_t dev_id, fal_port_t port_id)
{
    sw_error_t rv;

    rv = sw_uk_exec(SW_API_PT_POWER_OFF, dev_id, port_id);
    return rv;
}

sw_error_t
fal_port_power_on(a_uint32_t dev_id, fal_port_t port_id)
{
    sw_error_t rv;

    rv = sw_uk_exec(SW_API_PT_POWER_ON, dev_id, port_id);
    return rv;
}

    sw_error_t
    fal_port_magic_frame_mac_set (a_uint32_t dev_id, fal_port_t port_id,
				   fal_mac_addr_t * mac)
{
    sw_error_t rv;

    rv = sw_uk_exec(SW_API_PT_MAGIC_FRAME_MAC_SET, dev_id, port_id, mac);
    return rv;

}

   sw_error_t
   fal_port_magic_frame_mac_get (a_uint32_t dev_id, fal_port_t port_id,
				   fal_mac_addr_t * mac)
{

    sw_error_t rv;

    rv = sw_uk_exec(SW_API_PT_MAGIC_FRAME_MAC_GET, dev_id, port_id, mac);
    return rv;


}
 sw_error_t
    fal_port_phy_id_get (a_uint32_t dev_id, fal_port_t port_id,
		      a_uint16_t * org_id, a_uint16_t * rev_id)
 {
             sw_error_t rv;

    rv = sw_uk_exec(SW_API_PT_PHY_ID_GET, dev_id, port_id, org_id, rev_id);
    return rv;
 }
 sw_error_t
    fal_port_wol_status_set (a_uint32_t dev_id, fal_port_t port_id,
			      a_bool_t enable)
{
        sw_error_t rv;

    rv = sw_uk_exec(SW_API_PT_WOL_STATUS_SET, dev_id, port_id, enable);
    return rv;

 }
 sw_error_t
    fal_port_wol_status_get (a_uint32_t dev_id, fal_port_t port_id,
			      a_bool_t * enable)

 {
           sw_error_t rv;

    rv = sw_uk_exec(SW_API_PT_WOL_STATUS_GET, dev_id, port_id,enable);
    return rv;
 }
sw_error_t
fal_port_interface_mode_status_get (a_uint32_t dev_id, fal_port_t port_id,
			fal_port_interface_mode_t * mode)
{
  sw_error_t rv;

  rv = sw_uk_exec(SW_API_PT_INTERFACE_MODE_STATUS_GET, dev_id, port_id, mode);
  return rv;
}

sw_error_t
fal_debug_phycounter_set(a_uint32_t dev_id, fal_port_t port_id,
                       a_bool_t enable)
{
    sw_error_t rv;

    rv = sw_uk_exec(SW_API_DEBUG_PHYCOUNTER_SET, dev_id, port_id,
                     enable);
    return rv;
}

sw_error_t
fal_debug_phycounter_get(a_uint32_t dev_id, fal_port_t port_id,
                       a_bool_t * enable)
{
    sw_error_t rv;

    rv = sw_uk_exec(SW_API_DEBUG_PHYCOUNTER_GET, dev_id, port_id, enable);
    return rv;
}

sw_error_t
fal_debug_phycounter_show(a_uint32_t dev_id, fal_port_t port_id,
                       fal_port_counter_info_t * port_counter_info)
{
    sw_error_t rv;

    rv = sw_uk_exec(SW_API_DEBUG_PHYCOUNTER_SHOW, dev_id, port_id, port_counter_info);
    return rv;
}
