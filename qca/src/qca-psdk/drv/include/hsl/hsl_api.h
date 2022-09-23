/*
 * Copyright (c) 2012, 2015, 2017-2018, The Linux Foundation. All rights reserved.
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


#ifndef _HSL_API_H
#define _HSL_API_H

#ifdef __cplusplus
extern "C"
{
#endif				/* __cplusplus */

#include "fal.h"

  /* Port Control */
#define PORT_CONTROL_FUNC_PROTOTYPE_DEF
  typedef sw_error_t
    (*hsl_port_duplex_get) (a_uint32_t dev_id, fal_port_t port_id,
			    fal_port_duplex_t * pduplex);

  typedef sw_error_t
    (*hsl_port_duplex_set) (a_uint32_t dev_id, fal_port_t port_id,
			    fal_port_duplex_t duplex);

  typedef sw_error_t
    (*hsl_port_speed_get) (a_uint32_t dev_id, fal_port_t port_id,
			   fal_port_speed_t * pspeed);

  typedef sw_error_t
    (*hsl_port_autoneg_status_get) (a_uint32_t dev_id, fal_port_t port_id,
				    a_bool_t * status);

  typedef sw_error_t
    (*hsl_port_speed_set) (a_uint32_t dev_id, fal_port_t port_id,
			   fal_port_speed_t speed);

  typedef sw_error_t
    (*hsl_port_autoneg_enable) (a_uint32_t dev_id, fal_port_t port_id);

  typedef sw_error_t
    (*hsl_port_autoneg_restart) (a_uint32_t dev_id, fal_port_t port_id);

  typedef sw_error_t
    (*hsl_port_autoneg_adv_get) (a_uint32_t dev_id, fal_port_t port_id,
				 a_uint32_t * autoadv);

  typedef sw_error_t
    (*hsl_port_autoneg_adv_set) (a_uint32_t dev_id, fal_port_t port_id,
				 a_uint32_t autoadv);

  typedef sw_error_t
    (*hsl_port_hdr_status_set) (a_uint32_t dev_id, fal_port_t port_id,
				a_bool_t enable);

  typedef sw_error_t
    (*hsl_port_hdr_status_get) (a_uint32_t dev_id, fal_port_t port_id,
				a_bool_t * enable);

  typedef sw_error_t
    (*hsl_port_flowctrl_set) (a_uint32_t dev_id, fal_port_t port_id,
			      a_bool_t enable);

  typedef sw_error_t
    (*hsl_port_flowctrl_get) (a_uint32_t dev_id, fal_port_t port_id,
			      a_bool_t * enable);
typedef sw_error_t
    (*hsl_port_flowctrl_thresh_set) (a_uint32_t dev_id, fal_port_t port_id,
			      a_uint8_t  on, a_uint8_t  off);

  typedef sw_error_t
    (*hsl_port_flowctrl_forcemode_set) (a_uint32_t dev_id, fal_port_t port_id,
					a_bool_t enable);

  typedef sw_error_t
    (*hsl_port_flowctrl_forcemode_get) (a_uint32_t dev_id, fal_port_t port_id,
					a_bool_t * enable);

  typedef sw_error_t
    (*hsl_port_powersave_set) (a_uint32_t dev_id, fal_port_t port_id,
			       a_bool_t enable);


  typedef sw_error_t
    (*hsl_port_powersave_get) (a_uint32_t dev_id, fal_port_t port_id,
			       a_bool_t * enable);


  typedef sw_error_t
    (*hsl_port_hibernate_set) (a_uint32_t dev_id, fal_port_t port_id,
			       a_bool_t enable);


  typedef sw_error_t
    (*hsl_port_hibernate_get) (a_uint32_t dev_id, fal_port_t port_id,
			       a_bool_t * enable);


  typedef sw_error_t
    (*hsl_port_cdt) (a_uint32_t dev_id, fal_port_t port_id,
		     a_uint32_t mdi_pair, fal_cable_status_t * cable_status,
		     a_uint32_t * cable_len);

  typedef sw_error_t
    (*hsl_port_rxhdr_mode_set) (a_uint32_t dev_id, fal_port_t port_id,
				fal_port_header_mode_t mode);

  typedef sw_error_t
    (*hsl_port_rxhdr_mode_get) (a_uint32_t dev_id, fal_port_t port_id,
				fal_port_header_mode_t * mode);

  typedef sw_error_t
    (*hsl_port_txhdr_mode_set) (a_uint32_t dev_id, fal_port_t port_id,
				fal_port_header_mode_t mode);

  typedef sw_error_t
    (*hsl_port_txhdr_mode_get) (a_uint32_t dev_id, fal_port_t port_id,
				fal_port_header_mode_t * mode);

  typedef sw_error_t
    (*hsl_header_type_set) (a_uint32_t dev_id, a_bool_t enable,
			    a_uint32_t type);

  typedef sw_error_t
    (*hsl_header_type_get) (a_uint32_t dev_id, a_bool_t * enable,
			    a_uint32_t * type);

  typedef sw_error_t
    (*hsl_port_txmac_status_set) (a_uint32_t dev_id, fal_port_t port_id,
				  a_bool_t enable);

  typedef sw_error_t
    (*hsl_port_txmac_status_get) (a_uint32_t dev_id, fal_port_t port_id,
				  a_bool_t * enable);

  typedef sw_error_t
    (*hsl_port_rxmac_status_set) (a_uint32_t dev_id, fal_port_t port_id,
				  a_bool_t enable);

  typedef sw_error_t
    (*hsl_port_rxmac_status_get) (a_uint32_t dev_id, fal_port_t port_id,
				  a_bool_t * enable);

  typedef sw_error_t
    (*hsl_port_txfc_status_set) (a_uint32_t dev_id, fal_port_t port_id,
				 a_bool_t enable);

  typedef sw_error_t
    (*hsl_port_txfc_status_get) (a_uint32_t dev_id, fal_port_t port_id,
				 a_bool_t * enable);

  typedef sw_error_t
    (*hsl_port_rxfc_status_set) (a_uint32_t dev_id, fal_port_t port_id,
				 a_bool_t enable);

  typedef sw_error_t
    (*hsl_port_rxfc_status_get) (a_uint32_t dev_id, fal_port_t port_id,
				 a_bool_t * enable);

  typedef sw_error_t
    (*hsl_port_bp_status_set) (a_uint32_t dev_id, fal_port_t port_id,
			       a_bool_t enable);

  typedef sw_error_t
    (*hsl_port_bp_status_get) (a_uint32_t dev_id, fal_port_t port_id,
			       a_bool_t * enable);

  typedef sw_error_t
    (*hsl_port_link_forcemode_set) (a_uint32_t dev_id, fal_port_t port_id,
				    a_bool_t enable);

  typedef sw_error_t
    (*hsl_port_link_forcemode_get) (a_uint32_t dev_id, fal_port_t port_id,
				    a_bool_t * enable);

  typedef sw_error_t
    (*hsl_port_link_status_get) (a_uint32_t dev_id, fal_port_t port_id,
				 a_bool_t * status);

  typedef sw_error_t
    (*hsl_ports_link_status_get) (a_uint32_t dev_id, a_uint32_t * status);

  typedef sw_error_t
    (*hsl_port_mac_loopback_set) (a_uint32_t dev_id, fal_port_t port_id,
				  a_bool_t enable);

  typedef sw_error_t
    (*hsl_port_mac_loopback_get) (a_uint32_t dev_id, fal_port_t port_id,
				  a_bool_t * enable);

  typedef sw_error_t
    (*hsl_port_congestion_drop_set) (a_uint32_t dev_id, fal_port_t port_id,
				     a_uint32_t queue_id, a_bool_t enable);

  typedef sw_error_t
    (*hsl_port_congestion_drop_get) (a_uint32_t dev_id, fal_port_t port_id,
				     a_uint32_t queue_id, a_bool_t * enable);

  typedef sw_error_t
    (*hsl_ring_flow_ctrl_thres_set) (a_uint32_t dev_id, a_uint32_t ring_id,
				     a_uint8_t on_thres, a_uint8_t off_thres);

  typedef sw_error_t
    (*hsl_ring_flow_ctrl_thres_get) (a_uint32_t dev_id, a_uint32_t ring_id,
				     a_uint8_t * on_thres,
				     a_uint8_t * off_thres);

  typedef sw_error_t
    (*hsl_port_8023az_set) (a_uint32_t dev_id, fal_port_t port_id,
			    a_bool_t enable);


  typedef sw_error_t
    (*hsl_port_8023az_get) (a_uint32_t dev_id, fal_port_t port_id,
			    a_bool_t * enable);

  typedef sw_error_t
    (*hsl_port_mdix_set) (a_uint32_t dev_id, fal_port_t port_id,
			  fal_port_mdix_mode_t mode);

  typedef sw_error_t
    (*hsl_port_mdix_get) (a_uint32_t dev_id, fal_port_t port_id,
			  fal_port_mdix_mode_t * mode);

  typedef sw_error_t
    (*hsl_port_mdix_status_get) (a_uint32_t dev_id, fal_port_t port_id,
				 fal_port_mdix_status_t * mode);

  typedef sw_error_t
    (*hsl_port_combo_prefer_medium_set) (a_uint32_t dev_id,
					 fal_port_t port_id,
					 fal_port_medium_t medium);

  typedef sw_error_t
    (*hsl_port_combo_prefer_medium_get) (a_uint32_t dev_id,
					 fal_port_t port_id,
					 fal_port_medium_t * medium);

  typedef sw_error_t
    (*hsl_port_combo_medium_status_get) (a_uint32_t dev_id,
					 fal_port_t port_id,
					 fal_port_medium_t * medium);

  typedef sw_error_t
    (*hsl_port_combo_fiber_mode_set) (a_uint32_t dev_id, fal_port_t port_id,
				      fal_port_fiber_mode_t mode);

  typedef sw_error_t
    (*hsl_port_combo_fiber_mode_get) (a_uint32_t dev_id, fal_port_t port_id,
				      fal_port_fiber_mode_t * mode);

  typedef sw_error_t
    (*hsl_port_local_loopback_set) (a_uint32_t dev_id, fal_port_t port_id,
				    a_bool_t enable);

  typedef sw_error_t
    (*hsl_port_local_loopback_get) (a_uint32_t dev_id, fal_port_t port_id,
				    a_bool_t * enable);

  typedef sw_error_t
    (*hsl_port_remote_loopback_set) (a_uint32_t dev_id, fal_port_t port_id,
				     a_bool_t enable);

  typedef sw_error_t
    (*hsl_port_remote_loopback_get) (a_uint32_t dev_id, fal_port_t port_id,
				     a_bool_t * enable);

  typedef sw_error_t
    (*hsl_port_reset) (a_uint32_t dev_id, fal_port_t port_id);

  typedef sw_error_t
    (*hsl_port_power_off) (a_uint32_t dev_id, fal_port_t port_id);

  typedef sw_error_t
    (*hsl_port_power_on) (a_uint32_t dev_id, fal_port_t port_id);

  typedef sw_error_t
    (*hsl_port_phy_id_get) (a_uint32_t dev_id, fal_port_t port_id,a_uint16_t * org_id, a_uint16_t * rev_id);

  typedef sw_error_t
    (*hsl_port_wol_status_set) (a_uint32_t dev_id, fal_port_t port_id,a_bool_t enable);

  typedef sw_error_t
    (*hsl_port_wol_status_get) (a_uint32_t dev_id, fal_port_t port_id,a_bool_t *enable);

  typedef sw_error_t
    (*hsl_port_magic_frame_mac_set) (a_uint32_t dev_id, fal_port_t port_id,fal_mac_addr_t * mac);

  typedef sw_error_t
    (*hsl_port_magic_frame_mac_get) (a_uint32_t dev_id, fal_port_t port_id,fal_mac_addr_t * mac);

  typedef sw_error_t
    (*hsl_port_interface_mode_set) (a_uint32_t dev_id, fal_port_t port_id,
			      fal_port_interface_mode_t mode);

  typedef sw_error_t
    (*hsl_port_interface_mode_get) (a_uint32_t dev_id, fal_port_t port_id,
			      fal_port_interface_mode_t * mode);
    typedef sw_error_t
    (*hsl_port_interface_mode_apply) (a_uint32_t dev_id);

  typedef sw_error_t
    (*hsl_port_interface_mode_status_get) (a_uint32_t dev_id, fal_port_t port_id,
			      fal_port_interface_mode_t * mode);
   typedef sw_error_t
    (*hsl_port_counter_set) (a_uint32_t dev_id, fal_port_t port_id,
			    a_bool_t enable);
  typedef sw_error_t
    (*hsl_port_counter_get) (a_uint32_t dev_id, fal_port_t port_id,
			    a_bool_t * enable);
    typedef sw_error_t
    (*hsl_port_counter_show) (a_uint32_t dev_id, fal_port_t port_id,
			      fal_port_counter_info_t * counter_info);

  typedef sw_error_t (*hsl_dev_reset) (a_uint32_t dev_id);

  typedef sw_error_t (*hsl_dev_clean) (a_uint32_t dev_id);

  typedef sw_error_t
    (*hsl_dev_access_set) (a_uint32_t dev_id, hsl_access_mode mode);
#define REG_FUNC_PROTOTYPE_DEF
  typedef sw_error_t
    (*hsl_phy_get) (a_uint32_t dev_id, a_uint32_t phy_addr, a_uint32_t reg,
		    a_uint16_t * value);

  typedef sw_error_t
    (*hsl_phy_set) (a_uint32_t dev_id, a_uint32_t phy_addr, a_uint32_t reg,
		    a_uint16_t value);

  typedef struct
  {
    /* Port control */
    hsl_port_duplex_set port_duplex_set;
    hsl_port_duplex_get port_duplex_get;
    hsl_port_speed_set port_speed_set;
    hsl_port_speed_get port_speed_get;
    hsl_port_autoneg_status_get port_autoneg_status_get;
    hsl_port_autoneg_enable port_autoneg_enable;
    hsl_port_autoneg_restart port_autoneg_restart;
    hsl_port_autoneg_adv_get port_autoneg_adv_get;
    hsl_port_autoneg_adv_set port_autoneg_adv_set;
    hsl_port_flowctrl_set port_flowctrl_set;
    hsl_port_flowctrl_get port_flowctrl_get;
    hsl_port_powersave_set port_powersave_set;
    hsl_port_powersave_get port_powersave_get;
    hsl_port_hibernate_set port_hibernate_set;
    hsl_port_hibernate_get port_hibernate_get;
    hsl_port_cdt port_cdt;
    hsl_port_link_status_get port_link_status_get;
    hsl_ports_link_status_get ports_link_status_get;
    hsl_port_8023az_set port_8023az_set;
    hsl_port_8023az_get port_8023az_get;
    hsl_port_mdix_set port_mdix_set;
    hsl_port_mdix_get port_mdix_get;
    hsl_port_mdix_status_get port_mdix_status_get;
    hsl_port_local_loopback_set port_local_loopback_set;
    hsl_port_local_loopback_get port_local_loopback_get;
    hsl_port_remote_loopback_set port_remote_loopback_set;
    hsl_port_remote_loopback_get port_remote_loopback_get;
    hsl_port_reset port_reset;
    hsl_port_power_off port_power_off;
    hsl_port_power_on port_power_on;
    hsl_port_phy_id_get port_phy_id_get;
    hsl_port_wol_status_set port_wol_status_set;
    hsl_port_wol_status_get port_wol_status_get;
    hsl_port_magic_frame_mac_set port_magic_frame_mac_set;
    hsl_port_magic_frame_mac_get port_magic_frame_mac_get;
    hsl_port_interface_mode_status_get port_interface_mode_status_get;
    hsl_port_counter_set port_counter_set;
    hsl_port_counter_get port_counter_get;
    hsl_port_counter_show port_counter_show;
    /* REG Access */
    hsl_phy_get phy_get;
    hsl_phy_set phy_set;
      /*INIT*/ hsl_dev_reset dev_reset;
    hsl_dev_clean dev_clean;
    hsl_phy_get phy_i2c_get;
    hsl_phy_set phy_i2c_set;
  } hsl_api_t;

  hsl_api_t *hsl_api_ptr_get (a_uint32_t dev_id);

    sw_error_t hsl_api_init (a_uint32_t dev_id);

#if defined(SW_API_LOCK) && (!defined(HSL_STANDALONG))
  extern aos_lock_t sw_hsl_api_lock;
#define FAL_API_LOCK    aos_lock(&sw_hsl_api_lock)
#define FAL_API_UNLOCK  aos_unlock(&sw_hsl_api_lock)
#else
#define FAL_API_LOCK
#define FAL_API_UNLOCK
#endif

#ifdef __cplusplus
}
#endif				/* __cplusplus */
#endif				/* _SW_API_H */
