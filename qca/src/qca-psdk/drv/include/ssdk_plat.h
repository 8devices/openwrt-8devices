/*
 * Copyright (c) 2012, 2014-2015, 2017-2020, The Linux Foundation. All rights reserved.
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


#ifndef __SSDK_PLAT_H
#define __SSDK_PLAT_H

#include "sw.h"
#include <linux/phy.h>

#ifndef BIT
#define BIT(_n)                                      (1UL << (_n))
#endif


#ifndef BITS
#define BITS(_s, _n)                                 (((1UL << (_n)) - 1) << _s)
#endif

/* Atheros specific MII registers */
#define QCA_MII_MMD_ADDR                             0x0d
#define QCA_MII_MMD_DATA                             0x0e
#define QCA_MII_DBG_ADDR                             0x1d
#define QCA_MII_DBG_DATA                             0x1e
#define SSDK_PHY_BCAST_ID                            0x1f
#define SSDK_PHY_MIN_ID                              0x0
#define SSDK_PORT_CPU                                0
enum {
	QCA_VER_AR8216 = 0x01,
	QCA_VER_AR8227 = 0x02,
	QCA_VER_AR8236 = 0x03,
	QCA_VER_AR8316 = 0x10,
	QCA_VER_AR8327 = 0x12,
	QCA_VER_AR8337 = 0x13,
	QCA_VER_DESS = 0x14,
	QCA_VER_HPPE = 0x15,
	QCA_VER_SCOMPHY = 0xEE
};
#define SSDK_PHYSICAL_PORT0             0
#define SSDK_PHYSICAL_PORT1             1
#define SSDK_PHYSICAL_PORT2             2
#define SSDK_PHYSICAL_PORT3             3
#define SSDK_PHYSICAL_PORT4             4
#define SSDK_PHYSICAL_PORT5             5
#define SSDK_PHYSICAL_PORT6             6
#define SSDK_PHYSICAL_PORT7             7
#define SSDK_LOG_LEVEL_ERROR    0
#define SSDK_LOG_LEVEL_WARN     1
#define SSDK_LOG_LEVEL_INFO     2
#define SSDK_LOG_LEVEL_DEBUG    3
#define SSDK_LOG_LEVEL_DEFAULT  SSDK_LOG_LEVEL_INFO

extern a_uint32_t ssdk_log_level;

#define __SSDK_LOG_FUN(lev, fmt, ...) \
	do { \
		if (SSDK_LOG_LEVEL_##lev <= ssdk_log_level) { \
			printk("%s[%u]:"#lev":"fmt, \
				 __FUNCTION__, __LINE__, ##__VA_ARGS__); \
		} \
	} while(0)
#define SSDK_DUMP_BUF(lev, buf, len) \
	do {\
		if (SSDK_LOG_LEVEL_##lev <= ssdk_log_level) {\
			a_uint32_t i_buf = 0;\
			for(i_buf=0; i_buf<(len); i_buf++) {\
				printk("%08lx ", *((buf)+i_buf));\
			}\
			printk("\n");\
		}\
	} while(0)

#define SSDK_ERROR(fmt, ...) __SSDK_LOG_FUN(ERROR, fmt, ##__VA_ARGS__)
#define SSDK_WARN(fmt, ...)  __SSDK_LOG_FUN(WARN, fmt, ##__VA_ARGS__)
#define SSDK_INFO(fmt, ...)  __SSDK_LOG_FUN(INFO, fmt, ##__VA_ARGS__)
#define SSDK_DEBUG(fmt, ...) __SSDK_LOG_FUN(DEBUG, fmt, ##__VA_ARGS__)

struct qca_phy_priv {
	struct phy_device *phy;
#if defined(IN_SWCONFIG)
	struct switch_dev sw_dev;
#endif
    a_uint8_t version;
	a_uint8_t revision;
	a_uint32_t (*mii_read)(a_uint32_t dev_id, a_uint32_t reg);
	void (*mii_write)(a_uint32_t dev_id, a_uint32_t reg, a_uint32_t val);
    void (*phy_dbg_write)(a_uint32_t dev_id, a_uint32_t phy_addr,
                        a_uint16_t dbg_addr, a_uint16_t dbg_data);
	void (*phy_dbg_read)(a_uint32_t dev_id, a_uint32_t phy_addr,
                        a_uint16_t dbg_addr, a_uint16_t *dbg_data);
    void (*phy_mmd_write)(a_uint32_t dev_id, a_uint32_t phy_addr,
                          a_uint16_t addr, a_uint16_t data);
    sw_error_t (*phy_write)(a_uint32_t dev_id, a_uint32_t phy_addr,
                            a_uint32_t reg, a_uint16_t data);
    sw_error_t (*phy_read)(a_uint32_t dev_id, a_uint32_t phy_addr,
                           a_uint32_t reg, a_uint16_t* data);

	bool init;
	a_uint8_t device_id;
	struct device_node *of_node;
	struct mii_bus *miibus;
};

struct ipq40xx_mdio_data {
        struct mii_bus          *mii_bus;
        void __iomem            *membase;
        int phy_irq[PHY_MAX_ADDR];
};

#if defined(IN_SWCONFIG)
#define qca_phy_priv_get(_dev) \
		container_of(_dev, struct qca_phy_priv, sw_dev)
#endif

sw_error_t
qca_ar8327_phy_read(a_uint32_t dev_id, a_uint32_t phy_addr,
			a_uint32_t reg, a_uint16_t* data);
sw_error_t
qca_ar8327_phy_write(a_uint32_t dev_id, a_uint32_t phy_addr,
                            a_uint32_t reg, a_uint16_t data);
void
qca_ar8327_mmd_write(a_uint32_t dev_id, a_uint32_t phy_addr,
                              a_uint16_t addr, a_uint16_t data);
void
qca_ar8327_phy_dbg_write(a_uint32_t dev_id, a_uint32_t phy_addr,
		                          a_uint16_t dbg_addr, a_uint16_t dbg_data);
void
qca_ar8327_phy_dbg_read(a_uint32_t dev_id, a_uint32_t phy_addr,
		                          a_uint16_t dbg_addr, a_uint16_t *dbg_data);

void
qca_phy_mmd_write(u32 dev_id, u32 phy_id,
                     u16 mmd_num, u16 reg_id, u16 reg_val);

u16
qca_phy_mmd_read(u32 dev_id, u32 phy_id,
		u16 mmd_num, u16 reg_id);
int ssdk_plat_init(ssdk_init_cfg *cfg, a_uint32_t dev_id);
void ssdk_plat_exit(a_uint32_t dev_id);

#endif
