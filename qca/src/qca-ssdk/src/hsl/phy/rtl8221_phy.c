#include "sw.h"
#include "fal_port_ctrl.h"
#include "hsl_api.h"
#include "hsl.h"
#include "hsl_phy.h"
#include "ssdk_plat.h"
#include "rtl8221_phy.h"
#include "rtl8221.h"

#define PHY_INVALID_DATA 0xffff
#define PHY_RTN_ON_READ_ERROR(phy_data) \
    do { if (phy_data == PHY_INVALID_DATA) return(SW_READ_ERROR); } while(0);

#define PHY_RTN_ON_ERROR(rv) \
    do { if (rv != SW_OK) return(rv); } while(0);

static a_bool_t phy_ops_flag = A_FALSE;

/******************************************************************************
*
* rtl8221_phy_mii_read - mii register read
*
* mii register read
*/
a_uint16_t
rtl8221_phy_reg_read(a_uint32_t dev_id, a_uint32_t phy_id, a_uint32_t reg_id)
{
	sw_error_t rv = SW_OK;
	a_uint16_t phy_data = 0;
	{
		HSL_PHY_GET(rv, dev_id, phy_id & 0x1f, reg_id, &phy_data);
	}

	if (rv != SW_OK) {
		return PHY_INVALID_DATA;
	}

	return phy_data;
}

/******************************************************************************
*
* rtl8221_phy_mii_write - mii register write
*
* mii register write
*/
sw_error_t
rtl8221_phy_reg_write(a_uint32_t dev_id, a_uint32_t phy_id, a_uint32_t reg_id,
		       a_uint16_t reg_val)
{
	sw_error_t rv;
	{
		HSL_PHY_SET(rv, dev_id, phy_id & 0x1f, reg_id, reg_val);
	}

	return rv;

}

/******************************************************************************
*
* rtl8221_phy_mmd_write - PHY MMD register write
*
* PHY MMD register write
*/
sw_error_t
rtl8221_phy_mmd_write(a_uint32_t dev_id, a_uint32_t phy_id,
		     a_uint16_t mmd_num, a_uint16_t reg_id, a_uint16_t reg_val)
{
	sw_error_t rv;
	a_uint32_t reg_id_c45 = REG_C45_ADDRESS(mmd_num, reg_id);
	{
		HSL_PHY_SET(rv, dev_id, phy_id, reg_id_c45, reg_val);
	}

	return rv;
}

/******************************************************************************
*
* rtl8221_phy_mmd_read -  PHY MMD register read
*
* PHY MMD register read
*/
a_uint16_t
rtl8221_phy_mmd_read(a_uint32_t dev_id, a_uint32_t phy_id,
		    a_uint16_t mmd_num, a_uint16_t reg_id)
{
	sw_error_t rv = SW_OK;
	a_uint16_t phy_data = 0;
	a_uint32_t reg_id_c45 = REG_C45_ADDRESS(mmd_num, reg_id);
	{
		HSL_PHY_GET(rv, dev_id, phy_id, reg_id_c45, &phy_data);
	}

	if (rv != SW_OK) {
		return PHY_INVALID_DATA;
	}

	return phy_data;
}

/******************************************************************************
*
* rtl8221_phy_get status
*
* get phy status
*/
sw_error_t
rtl8221_phy_get_status(a_uint32_t dev_id, a_uint32_t phy_id,
		struct port_phy_status *phy_status)
{
	a_uint16_t phy_data;
	uint32_t  spd_grp, spd;

	phy_data = rtl8221_phy_mmd_read(dev_id, phy_id, 31, 0xA434);
	PHY_RTN_ON_READ_ERROR(phy_data);

	/*get phy link status*/
	if (phy_data & BIT(2)) {
		phy_status->link_status = A_TRUE;
	}
	else {
		phy_status->link_status = A_FALSE;
		return SW_OK;
	}

	/*get phy speed*/
	spd_grp = ( phy_data & ( 0x0600 )) >> 9;
	spd = ( phy_data & ( 0x0030 )) >> 4;
	if (spd_grp) {
		if (spd == 1) {
			phy_status->speed = FAL_SPEED_2500;
		} else if(spd == 3) {
			phy_status->speed = FAL_SPEED_1000;
		} else {
			return SW_READ_ERROR;
		}
	}
	else {
		if(spd == 0) {
			phy_status->speed = FAL_SPEED_10;
		} else if(spd == 1) {
			phy_status->speed = FAL_SPEED_100;
		} else if(spd == 2) {
			phy_status->speed = FAL_SPEED_1000;
		}
	}

	/*get phy duplex*/
	if (phy_data & BIT(3)) {
		phy_status->duplex = FAL_FULL_DUPLEX;
	} else {
		phy_status->duplex = FAL_HALF_DUPLEX;
	}

	/* get phy flowctrl resolution status */
	if (phy_data & BIT(7)) {
		phy_status->rx_flowctrl = A_TRUE;
	} else {
		phy_status->rx_flowctrl = A_FALSE;
	}

	if (phy_data & BIT(6)) {
		phy_status->tx_flowctrl = A_TRUE;
	} else {
		phy_status->tx_flowctrl = A_FALSE;
	}

	return SW_OK;
}

/******************************************************************************
*
* rtl8221_phy_get_speed - Determines the speed of phy ports associated with the
* specified device.
*/

sw_error_t
rtl8221_phy_get_speed(a_uint32_t dev_id, a_uint32_t phy_id,
		     fal_port_speed_t * speed)
{
	sw_error_t rv = SW_OK;
	struct port_phy_status phy_status = {0};

	rv = rtl8221_phy_get_status(dev_id, phy_id, &phy_status);
	PHY_RTN_ON_ERROR(rv);

	if (phy_status.link_status == A_TRUE) {
		*speed = phy_status.speed;
	} else {
		*speed = FAL_SPEED_10;
	}

	return rv;
}

/******************************************************************************
*
* rtl8221_phy_get_duplex - Determines the duplex of phy ports associated with the
* specified device.
*/
sw_error_t
rtl8221_phy_get_duplex(a_uint32_t dev_id, a_uint32_t phy_id,
		      fal_port_duplex_t * duplex)
{
	sw_error_t rv = SW_OK;
	struct port_phy_status phy_status = {0};

	rv = rtl8221_phy_get_status(dev_id, phy_id, &phy_status);
	PHY_RTN_ON_ERROR(rv);

	if (phy_status.link_status == A_TRUE) {
		*duplex = phy_status.duplex;
	} else {
		*duplex = FAL_HALF_DUPLEX;
	}

	return rv;
}

/******************************************************************************
*
* rtl8221_phy_status - test to see if the specified phy link is alive
*
* RETURNS:
*    A_TRUE  --> link is alive
*    A_FALSE --> link is down
*/
a_bool_t rtl8221_phy_get_link_status(a_uint32_t dev_id, a_uint32_t phy_id)
{
	struct port_phy_status phy_status = {0};
	sw_error_t rv = SW_OK;

	rv = rtl8221_phy_get_status(dev_id, phy_id, &phy_status);
	if (rv == SW_OK) {
		return phy_status.link_status;
	} else {
		return A_FALSE;
	}
}

/******************************************************************************
*
* rtl8221_phy_autoneg_status
*
* Power off the phy
*/
a_bool_t rtl8221_phy_autoneg_status(a_uint32_t dev_id, a_uint32_t phy_id)
{
	a_uint16_t phy_data;

	phy_data = rtl8221_phy_reg_read(dev_id, phy_id, PHY_STD_CTRL);

	if (phy_data & CTRL_AUTONEGOTIATION_ENABLE) {
		return A_TRUE;
	}

	return A_FALSE;
}

/******************************************************************************
*
* rtl8221_phy_interface mode get
*
* get rtl8221 phy interface mode
*/
sw_error_t
rtl8221_phy_interface_get_mode(a_uint32_t dev_id, a_uint32_t phy_id,
		fal_port_interface_mode_t *interface_mode)
{
	sw_error_t rv = SW_OK;
	struct port_phy_status phy_status = {0};

	rv = rtl8221_phy_get_status(dev_id, phy_id, &phy_status);
	PHY_RTN_ON_ERROR(rv);

	if (phy_status.speed == FAL_SPEED_2500)
		*interface_mode = PORT_SGMII_PLUS;
	else
		*interface_mode = PHY_SGMII_BASET;

	SSDK_DEBUG("ret if_mode 0x%x\n", *interface_mode);
	return SW_OK;
}

/******************************************************************************
*
* rtl8221_phy_interface mode status get
*
* get rtl8221 phy interface mode status
*/
sw_error_t
rtl8221_phy_interface_get_mode_status(a_uint32_t dev_id, a_uint32_t phy_id,
		fal_port_interface_mode_t *interface_mode_status)
{
	sw_error_t rv = SW_OK;
	struct port_phy_status phy_status = {0};

	rv = rtl8221_phy_get_status(dev_id, phy_id, &phy_status);
	PHY_RTN_ON_ERROR(rv);

	if (phy_status.speed == FAL_SPEED_2500)
		*interface_mode_status = PORT_SGMII_PLUS;
	else
		*interface_mode_status = PHY_SGMII_BASET;

	SSDK_DEBUG("ret if_mode_status 0x%x\n", *interface_mode_status);
	return SW_OK;
}

static sw_error_t
rtl8221_phy_hw_init(a_uint32_t dev_id,  a_uint32_t port_bmp)
{
	a_uint16_t phy_data = 0;
	a_uint32_t port_id = 0, phy_addr = 0;
	sw_error_t rv = SW_OK;
	int i;

	for (port_id = SSDK_PHYSICAL_PORT0; port_id < SW_MAX_NR_PORT; port_id ++)
	{
		if (port_bmp & (0x1 << port_id))
		{
			phy_addr = qca_ssdk_port_to_phy_addr(dev_id, port_id);

			// wait for PHY
			for (i = 0; i < 100; i++) {
				phy_data = rtl8221_phy_mmd_read(dev_id, phy_addr, 31, 0xA420);
				if ((phy_data != 0xffff) && ((phy_data & 0x3) == 0x3)) {
					break;
				}
				aos_mdelay(1);
			}
			SSDK_INFO("PHY wait done! (%d/100) result: %x\n", i, phy_data);

			phy_data = rtl8221_phy_mmd_read(dev_id, phy_addr, 30, 0x75F3);
			PHY_RTN_ON_READ_ERROR(phy_data);
			phy_data &= 0xfffe;
			rv = rtl8221_phy_mmd_write(dev_id, phy_addr, 30, 0x75F3, phy_data);

			// Combo mode (2500Base-X + SGMII)
			phy_data = rtl8221_phy_mmd_read(dev_id, phy_addr, 30, 0x697A);
			PHY_RTN_ON_READ_ERROR(phy_data);
			phy_data = (phy_data & 0xffc0) | 0;
			rv = rtl8221_phy_mmd_write(dev_id, phy_addr, 30, 0x697A, phy_data);
		}
	}
	return rv;
}

static sw_error_t rtl8221_phy_api_ops_init(void)
{
	sw_error_t  ret = SW_OK;
	hsl_phy_ops_t *rtl8221_phy_api_ops = NULL;

	rtl8221_phy_api_ops = kzalloc(sizeof(hsl_phy_ops_t), GFP_KERNEL);
	if (rtl8221_phy_api_ops == NULL) {
		SSDK_ERROR("rtl8221 phy ops kzalloc failed!\n");
		return -ENOMEM;
	}

	phy_api_ops_init(RTL8221B_PHY_CHIP);

	rtl8221_phy_api_ops->phy_reg_write = rtl8221_phy_reg_write;
	rtl8221_phy_api_ops->phy_reg_read = rtl8221_phy_reg_read;
	rtl8221_phy_api_ops->phy_mmd_write = rtl8221_phy_mmd_write;
	rtl8221_phy_api_ops->phy_mmd_read = rtl8221_phy_mmd_read;
	rtl8221_phy_api_ops->phy_get_status = rtl8221_phy_get_status;
	rtl8221_phy_api_ops->phy_speed_get = rtl8221_phy_get_speed;
	rtl8221_phy_api_ops->phy_duplex_get = rtl8221_phy_get_duplex;
	rtl8221_phy_api_ops->phy_autoneg_status_get = rtl8221_phy_autoneg_status;
	rtl8221_phy_api_ops->phy_link_status_get = rtl8221_phy_get_link_status;
	rtl8221_phy_api_ops->phy_interface_mode_get = rtl8221_phy_interface_get_mode;
	rtl8221_phy_api_ops->phy_interface_mode_status_get = rtl8221_phy_interface_get_mode_status;

	ret = hsl_phy_api_ops_register(RTL8221B_PHY_CHIP, rtl8221_phy_api_ops);

	if (ret == SW_OK) {
		SSDK_INFO("qca probe rtl8221 phy driver succeeded!\n");
	} else {
		SSDK_ERROR("qca probe rtl8221 phy driver failed! (code: %d)\n", ret);
	}

	return ret;
}

/******************************************************************************
*
* rtl8221_phy_init -
*
*/
int rtl8221_phy_init(a_uint32_t dev_id, a_uint32_t port_bmp)
{
	a_uint32_t port_id = 0;
	a_int32_t ret = 0;

	if(phy_ops_flag == A_FALSE &&
			rtl8221_phy_api_ops_init() == SW_OK) {
		phy_ops_flag = A_TRUE;
	}
	rtl8221_phy_hw_init(dev_id, port_bmp);

	for (port_id = SSDK_PHYSICAL_PORT0; port_id < SW_MAX_NR_PORT; port_id ++)
	{
		if (port_bmp & (0x1 << port_id))
		{
			rtl8221_phydev_init(dev_id, port_id);
		}
	}
	ret = rtl8221_phy_driver_register();

	return ret;
}

