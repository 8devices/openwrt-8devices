#ifndef _RTL8221_PHY_H_
#define _RTL8221_PHY_H_

#ifdef __cplusplus
extern "C"
{
#endif				/* __cplusplus */


#define MII_ADDR_C45  (1<<30)
#define REG_C45_ADDRESS(dev_type, reg_num) (MII_ADDR_C45 | \
			((dev_type & 0x1f) << 16) | (reg_num & 0xffff))

  /* PHY Registers */
#define PHY_STD_CTRL                      0

  /* Auto Neg Enable */
#define CTRL_AUTONEGOTIATION_ENABLE      0x1000


a_uint16_t
rtl8221_phy_reg_read(a_uint32_t dev_id, a_uint32_t phy_id, a_uint32_t reg_id);

sw_error_t
rtl8221_phy_reg_write(a_uint32_t dev_id, a_uint32_t phy_id,
		a_uint32_t reg_id, a_uint16_t reg_val);

sw_error_t
rtl8221_phy_mmd_write(a_uint32_t dev_id, a_uint32_t phy_id,
		a_uint16_t mmd_num, a_uint16_t reg_id,
		a_uint16_t reg_val);

a_uint16_t
rtl8221_phy_mmd_read(a_uint32_t dev_id, a_uint32_t phy_id,
		a_uint16_t mmd_num, a_uint16_t reg_id);

sw_error_t
rtl8221_phy_get_duplex (a_uint32_t dev_id, a_uint32_t phy_id,
		   fal_port_duplex_t * duplex);

sw_error_t
rtl8221_phy_get_speed (a_uint32_t dev_id, a_uint32_t phy_id,
		  fal_port_speed_t * speed);


a_bool_t
rtl8221_phy_get_link_status (a_uint32_t dev_id, a_uint32_t phy_id);


sw_error_t
rtl8221_phy_get_autoneg_adv (a_uint32_t dev_id, a_uint32_t phy_id,
			a_uint32_t * autoneg);

a_bool_t rtl8221_phy_autoneg_status (a_uint32_t dev_id, a_uint32_t phy_id);


sw_error_t
rtl8221_phy_get_status(a_uint32_t dev_id, a_uint32_t phy_id,
		struct port_phy_status *phy_status);

sw_error_t
rtl8221_phy_interface_get_mode_status(a_uint32_t dev_id, a_uint32_t phy_id,
		fal_port_interface_mode_t *interface_mode_status);

int rtl8221_phy_init(a_uint32_t dev_id, a_uint32_t port_bmp);

#ifdef __cplusplus
}
#endif				/* __cplusplus */
#endif				/* _RTL8221_PHY_H_ */
