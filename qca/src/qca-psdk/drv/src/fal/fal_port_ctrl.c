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


/**
 * @defgroup fal_port_ctrl FAL_PORT_CONTROL
 * @{
 */
#include "sw.h"
#include "fal_port_ctrl.h"
#include "hsl_api.h"
static sw_error_t
_fal_port_duplex_set (a_uint32_t dev_id, fal_port_t port_id,
		      fal_port_duplex_t duplex)
{
  sw_error_t rv;
  hsl_api_t *p_api;
  SW_RTN_ON_NULL (p_api = hsl_api_ptr_get (dev_id));

  if (NULL == p_api->port_duplex_set)
    return SW_NOT_SUPPORTED;

  rv = p_api->port_duplex_set (dev_id, port_id, duplex);
  return rv;
}

static sw_error_t
_fal_port_speed_set (a_uint32_t dev_id, fal_port_t port_id,
		     fal_port_speed_t speed)
{
  sw_error_t rv;
  hsl_api_t *p_api;
  SW_RTN_ON_NULL (p_api = hsl_api_ptr_get (dev_id));

  if (NULL == p_api->port_speed_set)
    return SW_NOT_SUPPORTED;

  rv = p_api->port_speed_set (dev_id, port_id, speed);
  return rv;
}
static sw_error_t
_fal_port_speed_get (a_uint32_t dev_id, fal_port_t port_id,
		     fal_port_speed_t * pspeed)
{
  sw_error_t rv;
  hsl_api_t *p_api;
  SW_RTN_ON_NULL (p_api = hsl_api_ptr_get (dev_id));

  if (NULL == p_api->port_speed_get)
    return SW_NOT_SUPPORTED;

  rv = p_api->port_speed_get (dev_id, port_id, pspeed);
  return rv;
}

static sw_error_t
_fal_port_duplex_get (a_uint32_t dev_id, fal_port_t port_id,
		      fal_port_duplex_t * pduplex)
{
  sw_error_t rv;
  hsl_api_t *p_api;
  SW_RTN_ON_NULL (p_api = hsl_api_ptr_get (dev_id));

  if (NULL == p_api->port_duplex_get)
    return SW_NOT_SUPPORTED;

  rv = p_api->port_duplex_get (dev_id, port_id, pduplex);
  return rv;
}

static sw_error_t
_fal_port_autoneg_enable (a_uint32_t dev_id, fal_port_t port_id)
{
  sw_error_t rv;
  hsl_api_t *p_api;
  SW_RTN_ON_NULL (p_api = hsl_api_ptr_get (dev_id));

  if (NULL == p_api->port_autoneg_enable)
    return SW_NOT_SUPPORTED;

  rv = p_api->port_autoneg_enable (dev_id, port_id);
  return rv;
}

static sw_error_t
_fal_port_autoneg_restart (a_uint32_t dev_id, fal_port_t port_id)
{
  sw_error_t rv;
  hsl_api_t *p_api;
  SW_RTN_ON_NULL (p_api = hsl_api_ptr_get (dev_id));

  if (NULL == p_api->port_autoneg_restart)
    return SW_NOT_SUPPORTED;

  rv = p_api->port_autoneg_restart (dev_id, port_id);
  return rv;
}


static sw_error_t
_fal_port_autoneg_adv_set (a_uint32_t dev_id, fal_port_t port_id,
			   a_uint32_t autoadv)
{
  sw_error_t rv;
  hsl_api_t *p_api;
  SW_RTN_ON_NULL (p_api = hsl_api_ptr_get (dev_id));

  if (NULL == p_api->port_autoneg_adv_set)
    return SW_NOT_SUPPORTED;

  rv = p_api->port_autoneg_adv_set (dev_id, port_id, autoadv);
  return rv;
}

static sw_error_t
_fal_port_autoneg_status_get (a_uint32_t dev_id, fal_port_t port_id,
			      a_bool_t * status)
{
  sw_error_t rv;
  hsl_api_t *p_api;
  SW_RTN_ON_NULL (p_api = hsl_api_ptr_get (dev_id));

  if (NULL == p_api->port_autoneg_status_get)
    return SW_NOT_SUPPORTED;

  rv = p_api->port_autoneg_status_get (dev_id, port_id, status);
  return rv;
}

static sw_error_t
_fal_port_autoneg_adv_get (a_uint32_t dev_id, fal_port_t port_id,
			   a_uint32_t * autoadv)
{
  sw_error_t rv;
  hsl_api_t *p_api;
  SW_RTN_ON_NULL (p_api = hsl_api_ptr_get (dev_id));

  if (NULL == p_api->port_autoneg_adv_get)
    return SW_NOT_SUPPORTED;

  rv = p_api->port_autoneg_adv_get (dev_id, port_id, autoadv);
  return rv;
}
static sw_error_t
_fal_port_hibernate_set (a_uint32_t dev_id, fal_port_t port_id,
			 a_bool_t enable)
{
  sw_error_t rv;
  hsl_api_t *p_api;
  SW_RTN_ON_NULL (p_api = hsl_api_ptr_get (dev_id));

  if (NULL == p_api->port_hibernate_set)
    return SW_NOT_SUPPORTED;

  rv = p_api->port_hibernate_set (dev_id, port_id, enable);
  return rv;
}


static sw_error_t
_fal_port_hibernate_get (a_uint32_t dev_id, fal_port_t port_id,
			 a_bool_t * enable)
{
  sw_error_t rv;
  hsl_api_t *p_api;
  SW_RTN_ON_NULL (p_api = hsl_api_ptr_get (dev_id));

  if (NULL == p_api->port_hibernate_get)
    return SW_NOT_SUPPORTED;

  rv = p_api->port_hibernate_get (dev_id, port_id, enable);
  return rv;
}

static sw_error_t
_fal_port_cdt (a_uint32_t dev_id, fal_port_t port_id, a_uint32_t mdi_pair,
	       a_uint32_t * cable_status, a_uint32_t * cable_len)
{
  sw_error_t rv;
  hsl_api_t *p_api;
  SW_RTN_ON_NULL (p_api = hsl_api_ptr_get (dev_id));

  if (NULL == p_api->port_cdt)
    return SW_NOT_SUPPORTED;

  rv = p_api->port_cdt (dev_id, port_id, mdi_pair, cable_status, cable_len);
  return rv;
}
static sw_error_t
_fal_port_link_status_get (a_uint32_t dev_id, fal_port_t port_id,
			   a_bool_t * status)
{
  sw_error_t rv;
  hsl_api_t *p_api;
  SW_RTN_ON_NULL (p_api = hsl_api_ptr_get (dev_id));

  if (NULL == p_api->port_link_status_get)
    return SW_NOT_SUPPORTED;

  rv = p_api->port_link_status_get (dev_id, port_id, status);
  return rv;
}

static sw_error_t
_fal_port_power_off (a_uint32_t dev_id, fal_port_t port_id)
{
  sw_error_t rv;
  hsl_api_t *p_api;
  SW_RTN_ON_NULL (p_api = hsl_api_ptr_get (dev_id));

  if (NULL == p_api->port_power_off)
    return SW_NOT_SUPPORTED;

  rv = p_api->port_power_off (dev_id, port_id);
  return rv;
}

static sw_error_t
_fal_port_power_on (a_uint32_t dev_id, fal_port_t port_id)
{
  sw_error_t rv;
  hsl_api_t *p_api;
  SW_RTN_ON_NULL (p_api = hsl_api_ptr_get (dev_id));

  if (NULL == p_api->port_power_on)
    return SW_NOT_SUPPORTED;

  rv = p_api->port_power_on (dev_id, port_id);
  return rv;
}
static sw_error_t
_fal_port_8023az_set (a_uint32_t dev_id, fal_port_t port_id, a_bool_t enable)
{
  sw_error_t rv;
  hsl_api_t *p_api;
  SW_RTN_ON_NULL (p_api = hsl_api_ptr_get (dev_id));

  if (NULL == p_api->port_8023az_set)
    return SW_NOT_SUPPORTED;

  rv = p_api->port_8023az_set (dev_id, port_id, enable);
  return rv;
}


static sw_error_t
_fal_port_8023az_get (a_uint32_t dev_id, fal_port_t port_id,
		      a_bool_t * enable)
{
  sw_error_t rv;
  hsl_api_t *p_api;
  SW_RTN_ON_NULL (p_api = hsl_api_ptr_get (dev_id));

  if (NULL == p_api->port_8023az_get)
    return SW_NOT_SUPPORTED;

  rv = p_api->port_8023az_get (dev_id, port_id, enable);
  return rv;
}

static sw_error_t
_fal_port_mdix_set (a_uint32_t dev_id, fal_port_t port_id,
		    fal_port_mdix_mode_t mode)
{
  sw_error_t rv;
  hsl_api_t *p_api;
  SW_RTN_ON_NULL (p_api = hsl_api_ptr_get (dev_id));

  if (NULL == p_api->port_mdix_set)
    return SW_NOT_SUPPORTED;

  rv = p_api->port_mdix_set (dev_id, port_id, mode);
  return rv;
}

static sw_error_t
_fal_port_mdix_get (a_uint32_t dev_id, fal_port_t port_id,
		    fal_port_mdix_mode_t * mode)
{
  sw_error_t rv;
  hsl_api_t *p_api;
  SW_RTN_ON_NULL (p_api = hsl_api_ptr_get (dev_id));

  if (NULL == p_api->port_mdix_get)
    return SW_NOT_SUPPORTED;

  rv = p_api->port_mdix_get (dev_id, port_id, mode);
  return rv;
}

static sw_error_t
_fal_port_mdix_status_get (a_uint32_t dev_id, fal_port_t port_id,
			   fal_port_mdix_status_t * mode)
{
  sw_error_t rv;
  hsl_api_t *p_api;
  SW_RTN_ON_NULL (p_api = hsl_api_ptr_get (dev_id));

  if (NULL == p_api->port_mdix_status_get)
    return SW_NOT_SUPPORTED;

  rv = p_api->port_mdix_status_get (dev_id, port_id, mode);
  return rv;
}
static sw_error_t
_fal_port_local_loopback_set (a_uint32_t dev_id, fal_port_t port_id,
			      a_bool_t enable)
{
  sw_error_t rv;
  hsl_api_t *p_api;
  SW_RTN_ON_NULL (p_api = hsl_api_ptr_get (dev_id));

  if (NULL == p_api->port_local_loopback_set)
    return SW_NOT_SUPPORTED;

  rv = p_api->port_local_loopback_set (dev_id, port_id, enable);
  return rv;
}


static sw_error_t
_fal_port_local_loopback_get (a_uint32_t dev_id, fal_port_t port_id,
			      a_bool_t * enable)
{
  sw_error_t rv;
  hsl_api_t *p_api;
  SW_RTN_ON_NULL (p_api = hsl_api_ptr_get (dev_id));

  if (NULL == p_api->port_local_loopback_get)
    return SW_NOT_SUPPORTED;

  rv = p_api->port_local_loopback_get (dev_id, port_id, enable);
  return rv;
}

static sw_error_t
_fal_port_remote_loopback_set (a_uint32_t dev_id, fal_port_t port_id,
			       a_bool_t enable)
{
  sw_error_t rv;
  hsl_api_t *p_api;
  SW_RTN_ON_NULL (p_api = hsl_api_ptr_get (dev_id));

  if (NULL == p_api->port_remote_loopback_set)
    return SW_NOT_SUPPORTED;

  rv = p_api->port_remote_loopback_set (dev_id, port_id, enable);
  return rv;
}


static sw_error_t
_fal_port_remote_loopback_get (a_uint32_t dev_id, fal_port_t port_id,
			       a_bool_t * enable)
{
  sw_error_t rv;
  hsl_api_t *p_api;
  SW_RTN_ON_NULL (p_api = hsl_api_ptr_get (dev_id));

  if (NULL == p_api->port_remote_loopback_get)
    return SW_NOT_SUPPORTED;

  rv = p_api->port_remote_loopback_get (dev_id, port_id, enable);
  return rv;
}

static sw_error_t
_fal_port_reset (a_uint32_t dev_id, fal_port_t port_id)
{
  sw_error_t rv;
  hsl_api_t *p_api;
  SW_RTN_ON_NULL (p_api = hsl_api_ptr_get (dev_id));

  if (NULL == p_api->port_reset)
    return SW_NOT_SUPPORTED;

  rv = p_api->port_reset (dev_id, port_id);
  return rv;
}


static sw_error_t
_fal_port_phy_id_get (a_uint32_t dev_id, fal_port_t port_id,a_uint16_t * org_id, a_uint16_t * rev_id)
{
  sw_error_t rv;
  hsl_api_t *p_api;
  SW_RTN_ON_NULL (p_api = hsl_api_ptr_get (dev_id));

  if (NULL == p_api->port_phy_id_get)
    return SW_NOT_SUPPORTED;

  rv = p_api->port_phy_id_get (dev_id, port_id,org_id,rev_id);
  return rv;
}

static sw_error_t
_fal_port_wol_status_set (a_uint32_t dev_id, fal_port_t port_id, a_bool_t enable)
{
  sw_error_t rv;
  hsl_api_t *p_api;
  SW_RTN_ON_NULL (p_api = hsl_api_ptr_get (dev_id));

  if (NULL == p_api->port_wol_status_set)
    return SW_NOT_SUPPORTED;

  rv = p_api->port_wol_status_set (dev_id, port_id,enable);
  return rv;
}

static sw_error_t
_fal_port_wol_status_get (a_uint32_t dev_id, fal_port_t port_id, a_bool_t *enable)
{
  sw_error_t rv;
  hsl_api_t *p_api;
  SW_RTN_ON_NULL (p_api = hsl_api_ptr_get (dev_id));

  if (NULL == p_api->port_wol_status_get)
    return SW_NOT_SUPPORTED;

  rv = p_api->port_wol_status_get (dev_id, port_id,enable);
  return rv;
}

static sw_error_t
_fal_port_magic_frame_mac_set (a_uint32_t dev_id, fal_port_t port_id, fal_mac_addr_t * mac)
{
  sw_error_t rv;
  hsl_api_t *p_api;
  SW_RTN_ON_NULL (p_api = hsl_api_ptr_get (dev_id));

  if (NULL == p_api->port_magic_frame_mac_set)
    return SW_NOT_SUPPORTED;

  rv = p_api->port_magic_frame_mac_set (dev_id, port_id, mac);
  return rv;
}

static sw_error_t
_fal_port_magic_frame_mac_get (a_uint32_t dev_id, fal_port_t port_id, fal_mac_addr_t * mac)
{
  sw_error_t rv;
  hsl_api_t *p_api;
  SW_RTN_ON_NULL (p_api = hsl_api_ptr_get (dev_id));

  if (NULL == p_api->port_magic_frame_mac_get)
    return SW_NOT_SUPPORTED;

  rv = p_api->port_magic_frame_mac_get (dev_id, port_id, mac);
  return rv;
}
static sw_error_t
_fal_port_interface_mode_status_get (a_uint32_t dev_id, fal_port_t port_id, fal_port_interface_mode_t * mode)
{
  sw_error_t rv;
  hsl_api_t *p_api;
  SW_RTN_ON_NULL (p_api = hsl_api_ptr_get (dev_id));

  if (NULL == p_api->port_interface_mode_status_get)
    return SW_NOT_SUPPORTED;

  rv = p_api->port_interface_mode_status_get (dev_id, port_id, mode);
  return rv;
}

static sw_error_t
_fal_port_counter_set (a_uint32_t dev_id, fal_port_t port_id, a_bool_t enable)
{
  sw_error_t rv;
  hsl_api_t *p_api;
  SW_RTN_ON_NULL (p_api = hsl_api_ptr_get (dev_id));

  if (NULL == p_api->port_counter_set)
    return SW_NOT_SUPPORTED;

  rv = p_api->port_counter_set (dev_id, port_id, enable);
  return rv;
}


static sw_error_t
_fal_port_counter_get (a_uint32_t dev_id, fal_port_t port_id,
		      a_bool_t * enable)
{
  sw_error_t rv;
  hsl_api_t *p_api;
  SW_RTN_ON_NULL (p_api = hsl_api_ptr_get (dev_id));

  if (NULL == p_api->port_counter_get)
    return SW_NOT_SUPPORTED;

  rv = p_api->port_counter_get (dev_id, port_id, enable);
  return rv;
}

static sw_error_t
_fal_port_counter_show (a_uint32_t dev_id, fal_port_t port_id, fal_port_counter_info_t * counter_info)
{
  sw_error_t rv;
  hsl_api_t *p_api;
  SW_RTN_ON_NULL (p_api = hsl_api_ptr_get (dev_id));

  if (NULL == p_api->port_counter_show)
    return SW_NOT_SUPPORTED;

  rv = p_api->port_counter_show (dev_id, port_id, counter_info);
  return rv;
}
/*insert flag for inner fal, don't remove it*/
/**
 * @brief Set duplex mode on a particular port.
 * @param[in] dev_id device id
 * @param[in] port_id port id
 * @param[in] duplex duplex mode
 * @return SW_OK or error code
 */
sw_error_t
fal_port_duplex_set (a_uint32_t dev_id, fal_port_t port_id,
		     fal_port_duplex_t duplex)
{
  sw_error_t rv;

  FAL_API_LOCK;
  rv = _fal_port_duplex_set (dev_id, port_id, duplex);
  FAL_API_UNLOCK;
  return rv;
}



/**
 * @brief Set speed on a particular port.
 * @param[in] dev_id device id
 * @param[in] port_id port id
 * @param[in] speed port speed
 * @return SW_OK or error code
 */
sw_error_t
fal_port_speed_set (a_uint32_t dev_id, fal_port_t port_id,
		    fal_port_speed_t speed)
{
  sw_error_t rv;

  FAL_API_LOCK;
  rv = _fal_port_speed_set (dev_id, port_id, speed);
  FAL_API_UNLOCK;
  return rv;
}
/**
 * @brief Get duplex mode on a particular port.
 * @param[in] dev_id device id
 * @param[in] port_id port id
 * @param[out] duplex duplex mode
 * @return SW_OK or error code
 */
sw_error_t
fal_port_duplex_get (a_uint32_t dev_id, fal_port_t port_id,
		     fal_port_duplex_t * pduplex)
{
  sw_error_t rv;

  FAL_API_LOCK;
  rv = _fal_port_duplex_get (dev_id, port_id, pduplex);
  FAL_API_UNLOCK;
  return rv;
}

/**
 * @brief Get speed on a particular port.
 * @param[in] dev_id device id
 * @param[in] port_id port id
 * @param[out] speed port speed
 * @return SW_OK or error code
 */
sw_error_t
fal_port_speed_get (a_uint32_t dev_id, fal_port_t port_id,
		    fal_port_speed_t * pspeed)
{
  sw_error_t rv;

  FAL_API_LOCK;
  rv = _fal_port_speed_get (dev_id, port_id, pspeed);
  FAL_API_UNLOCK;
  return rv;
}

/**
 * @brief Enable auto negotiation status on a particular port.
 * @param[in] dev_id device id
 * @param[in] port_id port id
 * @return SW_OK or error code
 */
sw_error_t
fal_port_autoneg_enable (a_uint32_t dev_id, fal_port_t port_id)
{
  sw_error_t rv;

  FAL_API_LOCK;
  rv = _fal_port_autoneg_enable (dev_id, port_id);
  FAL_API_UNLOCK;
  return rv;
}

/**
 * @brief Restart auto negotiation procedule on a particular port.
 * @param[in] dev_id device id
 * @param[in] port_id port id
 * @return SW_OK or error code
 */
sw_error_t
fal_port_autoneg_restart (a_uint32_t dev_id, fal_port_t port_id)
{
  sw_error_t rv;

  FAL_API_LOCK;
  rv = _fal_port_autoneg_restart (dev_id, port_id);
  FAL_API_UNLOCK;
  return rv;
}

/**
 * @brief Set auto negotiation advtisement ability on a particular port.
 *   @details  Comments:
 *   auto negotiation advtisement ability is defined by macro such as
 *   FAL_PHY_ADV_10T_HD, FAL_PHY_ADV_PAUSE...
 * @param[in] dev_id device id
 * @param[in] port_id port id
 * @param[in] autoadv auto negotiation advtisement ability bit map
 * @return SW_OK or error code
 */
sw_error_t
fal_port_autoneg_adv_set (a_uint32_t dev_id, fal_port_t port_id,
			  a_uint32_t autoadv)
{
  sw_error_t rv;

  FAL_API_LOCK;
  rv = _fal_port_autoneg_adv_set (dev_id, port_id, autoadv);
  FAL_API_UNLOCK;
  return rv;
}

/**
 * @brief Get auto negotiation status on a particular port.
 * @param[in] dev_id device id
 * @param[in] port_id port id
 * @param[out] status A_TRUE or A_FALSE
 * @return SW_OK or error code
 */
sw_error_t
fal_port_autoneg_status_get (a_uint32_t dev_id, fal_port_t port_id,
			     a_bool_t * status)
{
  sw_error_t rv;

  FAL_API_LOCK;
  rv = _fal_port_autoneg_status_get (dev_id, port_id, status);
  FAL_API_UNLOCK;
  return rv;
}

/**
 * @brief Get auto negotiation advtisement ability on a particular port.
 * @param[in] dev_id device id
 * @param[in] port_id port id
 * @param[out] autoadv auto negotiation advtisement ability bit map
 * @return SW_OK or error code
 */
sw_error_t
fal_port_autoneg_adv_get (a_uint32_t dev_id, fal_port_t port_id,
			  a_uint32_t * autoadv)
{
  sw_error_t rv;

  FAL_API_LOCK;
  rv = _fal_port_autoneg_adv_get (dev_id, port_id, autoadv);
  FAL_API_UNLOCK;
  return rv;
}
/**
 * @brief Set hibernate status on a particular port.
 * @param[in] dev_id device id
 * @param[in] port_id port id
 * @param[in] enable A_TRUE or A_FALSE
 * @return SW_OK or error code
 */
sw_error_t
fal_port_hibernate_set (a_uint32_t dev_id, fal_port_t port_id,
			a_bool_t enable)
{
  sw_error_t rv;

  FAL_API_LOCK;
  rv = _fal_port_hibernate_set (dev_id, port_id, enable);
  FAL_API_UNLOCK;
  return rv;
}

/**
 * @brief Get hibernate status on a particular port.
 * @param[in] dev_id device id
 * @param[in] port_id port id
 * @param[out] enable A_TRUE or A_FALSE
 * @return SW_OK or error code
 */
sw_error_t
fal_port_hibernate_get (a_uint32_t dev_id, fal_port_t port_id,
			a_bool_t * enable)
{
  sw_error_t rv;

  FAL_API_LOCK;
  rv = _fal_port_hibernate_get (dev_id, port_id, enable);
  FAL_API_UNLOCK;
  return rv;
}

/**
 * @brief cable diagnostic test.
 * @param[in] dev_id device id
 * @param[in] port_id port id
 * @param[in] mdi_pair mdi pair id
 * @param[out] cable_status cable status
 * @param[out] cable_len cable len
 * @return SW_OK or error code
 */
sw_error_t
fal_port_cdt (a_uint32_t dev_id, fal_port_t port_id, a_uint32_t mdi_pair,
	      a_uint32_t * cable_status, a_uint32_t * cable_len)
{
  sw_error_t rv;

  FAL_API_LOCK;
  rv = _fal_port_cdt (dev_id, port_id, mdi_pair, cable_status, cable_len);
  FAL_API_UNLOCK;
  return rv;
}
/**
 * @brief Get link status on particular port.
 * @param[in] dev_id device id
 * @param[in] port_id port id
 * @param[out] status link status up (A_TRUE) or down (A_FALSE)
 * @return SW_OK or error code
 */
sw_error_t
fal_port_link_status_get (a_uint32_t dev_id, fal_port_t port_id,
			  a_bool_t * status)
{
  sw_error_t rv;

  FAL_API_LOCK;
  rv = _fal_port_link_status_get (dev_id, port_id, status);
  FAL_API_UNLOCK;
  return rv;
}

/**
 * @brief power off on a particular port.
 * @param[in] dev_id device id
 * @param[in] port_id port id
 * @param[out]
 * @return SW_OK or error code
 */
sw_error_t
fal_port_power_off (a_uint32_t dev_id, fal_port_t port_id)
{
  sw_error_t rv;

  FAL_API_LOCK;
  rv = _fal_port_power_off (dev_id, port_id);
  FAL_API_UNLOCK;
  return rv;
}

/**
 * @brief power on on a particular port.
 * @param[in] dev_id device id
 * @param[in] port_id port id
 * @param[out]
 * @return SW_OK or error code
 */
sw_error_t
fal_port_power_on (a_uint32_t dev_id, fal_port_t port_id)
{
  sw_error_t rv;

  FAL_API_LOCK;
  rv = _fal_port_power_on (dev_id, port_id);
  FAL_API_UNLOCK;
  return rv;
}
/**
 * @brief Set 8023az status on a particular port.
 * @param[in] dev_id device id
 * @param[in] port_id port id
 * @param[in] enable A_TRUE or A_FALSE
 * @return SW_OK or error code
 */
sw_error_t
fal_port_8023az_set (a_uint32_t dev_id, fal_port_t port_id, a_bool_t enable)
{
  sw_error_t rv;

  FAL_API_LOCK;
  rv = _fal_port_8023az_set (dev_id, port_id, enable);
  FAL_API_UNLOCK;
  return rv;
}

/**
 * @brief Get 8023az status on a particular port.
 * @param[in] dev_id device id
 * @param[in] port_id port id
 * @param[out] enable A_TRUE or A_FALSE
 * @return SW_OK or error code
 */
sw_error_t
fal_port_8023az_get (a_uint32_t dev_id, fal_port_t port_id, a_bool_t * enable)
{
  sw_error_t rv;

  FAL_API_LOCK;
  rv = _fal_port_8023az_get (dev_id, port_id, enable);
  FAL_API_UNLOCK;
  return rv;
}

/**
 * @brief Set mdix mode on a particular port.
 * @param[in] dev_id device id
 * @param[in] port_id port id
 * @param[in] set mdix mode [mdx , mdix or auto]
 * @return SW_OK or error code
 */
sw_error_t
fal_port_mdix_set (a_uint32_t dev_id, fal_port_t port_id,
		   fal_port_mdix_mode_t mode)
{
  sw_error_t rv;

  FAL_API_LOCK;
  rv = _fal_port_mdix_set (dev_id, port_id, mode);
  FAL_API_UNLOCK;
  return rv;
}

/**
 * @brief Get mdix on a particular port.
 * @param[in] dev_id device id
 * @param[in] port_id port id
 * @param[out] set mdx ,mdix or auto 
 * @return SW_OK or error code
 */
sw_error_t
fal_port_mdix_get (a_uint32_t dev_id, fal_port_t port_id,
		   fal_port_mdix_mode_t * mode)
{
  sw_error_t rv;

  FAL_API_LOCK;
  rv = _fal_port_mdix_get (dev_id, port_id, mode);
  FAL_API_UNLOCK;
  return rv;
}

/**
 * @brief Get mdix status on a particular port.
 * @param[in] dev_id device id
 * @param[in] port_id port id
 * @param[out] set mdx ,mdix 
 * @return SW_OK or error code
 */
sw_error_t
fal_port_mdix_status_get (a_uint32_t dev_id, fal_port_t port_id,
			  fal_port_mdix_status_t * mode)
{
  sw_error_t rv;

  FAL_API_LOCK;
  rv = _fal_port_mdix_status_get (dev_id, port_id, mode);
  FAL_API_UNLOCK;
  return rv;
}
/**
 * @brief Set local loopback  on a particular port.
 * @param[in] dev_id device id
 * @param[in] port_id port id
 * @param[in] enable A_TRUE or A_FALSE
 * @return SW_OK or error code
 */
sw_error_t
fal_port_local_loopback_set (a_uint32_t dev_id, fal_port_t port_id,
			     a_bool_t enable)
{
  sw_error_t rv;

  FAL_API_LOCK;
  rv = _fal_port_local_loopback_set (dev_id, port_id, enable);
  FAL_API_UNLOCK;
  return rv;
}

/**
 * @brief Get local loopback status on a particular port.
 * @param[in] dev_id device id
 * @param[in] port_id port id
 * @param[out] enable A_TRUE or A_FALSE
 * @return SW_OK or error code
 */
sw_error_t
fal_port_local_loopback_get (a_uint32_t dev_id, fal_port_t port_id,
			     a_bool_t * enable)
{
  sw_error_t rv;

  FAL_API_LOCK;
  rv = _fal_port_local_loopback_get (dev_id, port_id, enable);
  FAL_API_UNLOCK;
  return rv;
}

/**
 * @brief Set remote loopback  on a particular port.
 * @param[in] dev_id device id
 * @param[in] port_id port id
 * @param[in] enable A_TRUE or A_FALSE
 * @return SW_OK or error code
 */
sw_error_t
fal_port_remote_loopback_set (a_uint32_t dev_id, fal_port_t port_id,
			      a_bool_t enable)
{
  sw_error_t rv;

  FAL_API_LOCK;
  rv = _fal_port_remote_loopback_set (dev_id, port_id, enable);
  FAL_API_UNLOCK;
  return rv;
}

/**
 * @brief Get remote loopback status on a particular port.
 * @param[in] dev_id device id
 * @param[in] port_id port id
 * @param[out] enable A_TRUE or A_FALSE
 * @return SW_OK or error code
 */
sw_error_t
fal_port_remote_loopback_get (a_uint32_t dev_id, fal_port_t port_id,
			      a_bool_t * enable)
{
  sw_error_t rv;

  FAL_API_LOCK;
  rv = _fal_port_remote_loopback_get (dev_id, port_id, enable);
  FAL_API_UNLOCK;
  return rv;
}

/**
 * @brief software reset on a particular port.
 * @param[in] dev_id device id
 * @param[in] port_id port id
 * @param[out]
 * @return SW_OK or error code
 */
sw_error_t
fal_port_reset (a_uint32_t dev_id, fal_port_t port_id)
{
  sw_error_t rv;

  FAL_API_LOCK;
  rv = _fal_port_reset (dev_id, port_id);
  FAL_API_UNLOCK;
  return rv;
}

/**
 * @brief phy id on a particular port.
 * @param[in] dev_id device id
 * @param[in] port_id port id
 * @param[out]
 * @return SW_OK or error code
 */
sw_error_t
fal_port_phy_id_get (a_uint32_t dev_id, fal_port_t port_id, a_uint16_t * org_id, a_uint16_t * rev_id)
{
  sw_error_t rv;

  FAL_API_LOCK;
  rv = _fal_port_phy_id_get (dev_id, port_id,org_id,rev_id);
  FAL_API_UNLOCK;
  return rv;
}

/**
 * @brief wol status on a particular port.
 * @param[in] dev_id device id
 * @param[in] port_id port id
 * @param[out]
 * @return SW_OK or error code
 */
sw_error_t
fal_port_wol_status_set (a_uint32_t dev_id, fal_port_t port_id, a_bool_t enable)
{
  sw_error_t rv;

  FAL_API_LOCK;
  rv = _fal_port_wol_status_set (dev_id, port_id,enable);
  FAL_API_UNLOCK;
  return rv;
}

/**
 * @brief wol status on a particular port.
 * @param[in] dev_id device id
 * @param[in] port_id port id
 * @param[out]
 * @return SW_OK or error code
 */
sw_error_t
fal_port_wol_status_get (a_uint32_t dev_id, fal_port_t port_id, a_bool_t *enable)
{
  sw_error_t rv;

  FAL_API_LOCK;
  rv = _fal_port_wol_status_get (dev_id, port_id,enable);
  FAL_API_UNLOCK;
  return rv;
}

/**
 * @brief magic frame mac on a particular port.
 * @param[in] dev_id device id
 * @param[in] port_id port id
 * @param[out]
 * @return SW_OK or error code
 */
sw_error_t
fal_port_magic_frame_mac_set (a_uint32_t dev_id, fal_port_t port_id, fal_mac_addr_t * mac)
{
  sw_error_t rv;

  FAL_API_LOCK;
  rv = _fal_port_magic_frame_mac_set (dev_id, port_id,mac);
  FAL_API_UNLOCK;
  return rv;
}

/**
 * @brief magic frame mac  on a particular port.
 * @param[in] dev_id device id
 * @param[in] port_id port id
 * @param[out]
 * @return SW_OK or error code
 */
sw_error_t
fal_port_magic_frame_mac_get (a_uint32_t dev_id, fal_port_t port_id, fal_mac_addr_t * mac)
{
  sw_error_t rv;

  FAL_API_LOCK;
  rv = _fal_port_magic_frame_mac_get (dev_id, port_id,mac);
  FAL_API_UNLOCK;
  return rv;
}
/**
 * @brief interface mode status on a particular port.
 * @param[in] dev_id device id
 * @param[in] port_id port id
 * @param[out]
 * @return SW_OK or error code
 */
sw_error_t
fal_port_interface_mode_status_get (a_uint32_t dev_id, fal_port_t port_id, fal_port_interface_mode_t * mode)
{
  sw_error_t rv;

  FAL_API_LOCK;
  rv = _fal_port_interface_mode_status_get (dev_id, port_id,mode);
  FAL_API_UNLOCK;
  return rv;
}

/**
 * @brief Set counter status on a particular port.
 * @param[in] dev_id device id
 * @param[in] port_id port id
 * @param[in] enable A_TRUE or A_FALSE
 * @return SW_OK or error code
 */
sw_error_t
fal_debug_phycounter_set (a_uint32_t dev_id, fal_port_t port_id, a_bool_t enable)
{
  sw_error_t rv;

  FAL_API_LOCK;
  rv = _fal_port_counter_set (dev_id, port_id, enable);
  FAL_API_UNLOCK;
  return rv;
}

/**
 * @brief Get counter status on a particular port.
 * @param[in] dev_id device id
 * @param[in] port_id port id
 * @param[out] enable A_TRUE or A_FALSE
 * @return SW_OK or error code
 */
sw_error_t
fal_debug_phycounter_get (a_uint32_t dev_id, fal_port_t port_id, a_bool_t * enable)
{
  sw_error_t rv;

  FAL_API_LOCK;
  rv = _fal_port_counter_get (dev_id, port_id, enable);
  FAL_API_UNLOCK;
  return rv;
}

/**
 * @brief Get counter statistics on a particular port.
 * @param[in] dev_id device id
 * @param[in] port_id port id
 * @param[out] counter frame number
 * @return SW_OK or error code
 */
sw_error_t
fal_debug_phycounter_show (a_uint32_t dev_id, fal_port_t port_id, fal_port_counter_info_t* port_counter_info)
{
  sw_error_t rv;

  FAL_API_LOCK;
  rv = _fal_port_counter_show (dev_id, port_id, port_counter_info);
  FAL_API_UNLOCK;
  return rv;
}
