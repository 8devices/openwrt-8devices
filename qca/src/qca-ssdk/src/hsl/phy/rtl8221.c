#include "hsl_phy.h"
#include "rtl8221.h"
#include "rtl8221_phy.h"

LIST_HEAD(g_rtl8221_phy_list);

struct rtl8221_phy_info* rtl8221_phy_info_get(a_uint32_t phy_addr)
{
	struct rtl8221_phy_info *pdata = NULL;
	list_for_each_entry(pdata, &g_rtl8221_phy_list, list) {
		if (pdata->phydev_addr == phy_addr) {
			return pdata;
		}
	}

	SSDK_ERROR("%s can't get the data for phy addr: %d\n", __func__, phy_addr);
	return NULL;
}

static int rtl8221_config_init(struct phy_device *phydev)
{
	rtl8221_priv *priv = phydev->priv;
	struct rtl8221_phy_info *pdata = priv->phy_info;
	__ETHTOOL_DECLARE_LINK_MODE_MASK(mask) = { 0, };

	if (!pdata) {
		return SW_NOT_FOUND;
	}

	genphy_read_abilities(phydev);

	linkmode_copy(mask, phydev->supported);
	linkmode_copy(mask, phydev->advertising);

	linkmode_set_bit(ETHTOOL_LINK_MODE_2500baseT_Full_BIT, mask);

	linkmode_copy(phydev->supported, mask);
	linkmode_copy(phydev->advertising, mask);

	return SW_OK;
}

static int rtl8221_read_status(struct phy_device *phydev)
{
	struct port_phy_status phy_status;
	a_uint32_t dev_id = 0, phy_id = 0;
	rtl8221_priv *priv = phydev->priv;
	const struct rtl8221_phy_info *pdata = priv->phy_info;

	if (!pdata) {
		return SW_FAIL;
	}

	dev_id = pdata->dev_id;
	phy_id = pdata->phy_addr;

	rtl8221_phy_get_status(dev_id, phy_id, &phy_status);

	if (phy_status.link_status) {
		phydev->link = 1;
	} else {
		phydev->link = 0;
	}

	switch (phy_status.speed) {
		case FAL_SPEED_2500:
			phydev->speed = SPEED_2500;
			break;
		case FAL_SPEED_1000:
			phydev->speed = SPEED_1000;
			break;
		case FAL_SPEED_100:
			phydev->speed = SPEED_100;
			break;
		default:
			phydev->speed = SPEED_10;
			break;
	}

	if (phy_status.duplex == FAL_FULL_DUPLEX) {
		phydev->duplex = DUPLEX_FULL;
	} else {
		phydev->duplex = DUPLEX_HALF;
	}

	return 0;
}

static int rtl8221_phy_probe(struct phy_device *phydev)
{
	rtl8221_priv *priv;
	int err = 0;

	priv = kzalloc(sizeof(rtl8221_priv), GFP_KERNEL);
	if (!priv) {
		return -ENOMEM;
	}

	priv->phydev = phydev;
	priv->phy_info = rtl8221_phy_info_get(phydev->mdio.addr);
	phydev->priv = priv;

	return err;
}

static void rtl8221_phy_remove(struct phy_device *phydev)
{
	rtl8221_priv *priv = phydev->priv;
	kfree(priv);
}

struct phy_driver rtl8221_phy_driver = {
	.phy_id		= RTL8221B_VB,
	.phy_id_mask    = 0xffffffff,
	.name		= "Realtek RTL8221B-VB ethernet",
	.features	= PHY_GBIT_FEATURES,
	.probe		= rtl8221_phy_probe,
	.remove		= rtl8221_phy_remove,
	.config_init	= rtl8221_config_init,
	.read_status	= rtl8221_read_status,
	.mdiodrv.driver		= { .owner = THIS_MODULE },
};

a_int32_t rtl8221_phy_driver_register(void)
{
	a_int32_t ret;
	ret = phy_driver_register(&rtl8221_phy_driver, THIS_MODULE);
	return ret;
}

void rtl8221_phy_driver_unregister(void)
{
	phy_driver_unregister(&rtl8221_phy_driver);
}

void rtl8221_phydev_init(a_uint32_t dev_id, a_uint32_t port_id)
{
	struct rtl8221_phy_info *pdata;
	pdata = kzalloc(sizeof(struct rtl8221_phy_info), GFP_KERNEL);

	if (!pdata) {
		return;
	}
	list_add_tail(&pdata->list, &g_rtl8221_phy_list);
	pdata->dev_id = dev_id;
	pdata->phy_addr = qca_ssdk_port_to_phy_addr(dev_id, port_id);
	pdata->phydev_addr = pdata->phy_addr;
}

void rtl8221_phydev_deinit(a_uint32_t dev_id, a_uint32_t port_id)
{
	struct rtl8221_phy_info *pdata, *pnext;

	list_for_each_entry_safe(pdata, pnext, &g_rtl8221_phy_list, list) {
		list_del(&pdata->list);
		kfree(pdata);
	}
}

