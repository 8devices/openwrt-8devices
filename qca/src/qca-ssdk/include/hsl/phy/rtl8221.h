#include <linux/kconfig.h>
#include <linux/version.h>
#include <linux/phy.h>
#include <linux/kthread.h>

#include "hsl.h"
#include "ssdk_plat.h"
#include "hsl_phy.h"
#include "rtl8221_phy.h"

struct rtl8221_phy_info {
	struct list_head list;
	a_uint32_t dev_id;
	/* phy real address,it is the mdio addr or the i2c slave addr */
	a_uint32_t phy_addr;
	/* the address of phy device, it is a fake addr for the i2c accessed phy */
	a_uint32_t phydev_addr;
};

typedef struct {
	struct phy_device *phydev;
	struct rtl8221_phy_info *phy_info;
} rtl8221_priv;

void rtl8221_phydev_init(a_uint32_t dev_id, a_uint32_t port_id);
void rtl8221_phydev_deinit(a_uint32_t dev_id, a_uint32_t port_id);
a_int32_t rtl8221_phy_driver_register(void);
void rtl8221_phy_driver_unregister(void);

