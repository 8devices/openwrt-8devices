/**
 *  SPI Flash probe code.
 *  (C) 2006 Atmark Techno, Inc.
 *
 *  Copyright (c) 2011 Realtek Semiconductor Corp.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 */

#include <linux/init.h>
#include <linux/kernel.h>
//#include <linux/config.h>
#include <linux/module.h>
#include <linux/types.h>

#include <linux/mtd/map.h>
#include <linux/mtd/gen_probe.h>
#include "spi_flash.h"
#ifndef CONFIG_WRT_BARRIER_BREAKER
#include <linux/autoconf.h>
#else
#include <generated/autoconf.h>
#include <linux/slab.h>
#endif
//#include "spi_common.h"

#include <linux/mtd/mtd.h>
//#define MTD_SPI_DEBUG
#define REG32(reg)   (*(volatile unsigned int *)((unsigned int)reg))

#if defined(MTD_SPI_DEBUG)
#define KDEBUG(args...) printk(args)
#else
#define KDEBUG(args...)
#endif

typedef struct spi_chip_info *(spi_probe_func)(struct map_info *, struct chip_probe *);

//extern spi_probe_func spi_probe_suzaku;
struct spi_chip_info *spi_probe_flash_chip(struct map_info *map, struct chip_probe *cp);
static spi_probe_func *probe_func[] = {
//#if defined(CONFIG_MTD_SPI_SUZAKU) || defined(CONFIG_MTD_SPI_SUZAKU_MODULE)
	spi_probe_flash_chip,
//#endif
	NULL
};

extern int mtd_spi_erase(struct mtd_info *mtd, struct erase_info *instr);
extern int mtd_spi_read(struct mtd_info *mtd, loff_t from, size_t len, size_t *retlen, u_char *buf);
extern int mtd_spi_write(struct mtd_info *mtd, loff_t to, size_t len, size_t *retlen, const u_char *buf);
extern void mtd_spi_sync(struct mtd_info *mtd);
extern int mtd_spi_lock(struct mtd_info *mtd, loff_t ofs, size_t len);
extern int mtd_spi_unlock(struct mtd_info *mtd, loff_t ofs, size_t len);
extern int mtd_spi_suspend(struct mtd_info *mtd);
extern void mtd_spi_resume(struct mtd_info *mtd);

static struct mtd_info *spi_chip_setup(struct map_info *map, struct spi_chip_info *chip_info);
static struct mtd_info *spi_probe_chip(struct map_info *map, struct chip_probe *cp);

struct mtd_info *spi_probe1(struct map_info *map);
static void spi_destroy1(struct mtd_info *mtd);
#ifdef CONFIG_RTL_TWO_SPI_FLASH_ENABLE
struct mtd_info *spi_probe2(struct map_info *map);
static void spi_destroy2(struct mtd_info *mtd);
#endif




static struct mtd_chip_driver spi_chipdrv1 = {
	probe:   spi_probe1,
	destroy: spi_destroy1,
	name:    "flash_bank_1",
	module:  THIS_MODULE,
};
#ifdef CONFIG_RTL_TWO_SPI_FLASH_ENABLE
static struct mtd_chip_driver spi_chipdrv2 = {
	probe:   spi_probe2,
	destroy: spi_destroy2,
	name:    "flash_bank_2",
	module:  THIS_MODULE,
};
#endif
static struct mtd_info *spi_chip_setup(struct map_info *map, struct spi_chip_info *chip_info)
{
	struct mtd_info *mtd;

	mtd = kmalloc(sizeof(*mtd), GFP_KERNEL);
	if (!mtd) {
		printk(KERN_WARNING "Failed to allocate memory for MTD device\n");
		return NULL;
	}

	memset(mtd, 0, sizeof(struct mtd_info));

	mtd->type                = MTD_NORFLASH;//MTD_OTHER;
	mtd->flags               = MTD_CAP_NORFLASH;

	mtd->name                = map->name;

	mtd->size                = chip_info->flash->deviceSize;
	mtd->erasesize           = chip_info->flash->sectorSize;
#ifndef CONFIG_WRT_BARRIER_BREAKER
	mtd->erase               = mtd_spi_erase;
	mtd->read                = mtd_spi_read;
	mtd->write               = mtd_spi_write;
	mtd->sync                = mtd_spi_sync;
	mtd->lock                = mtd_spi_lock;
	mtd->unlock              = mtd_spi_unlock;
	mtd->suspend             = mtd_spi_suspend;
	mtd->resume              = mtd_spi_resume;
#else
      mtd->_erase               = mtd_spi_erase;
	mtd->_read                = mtd_spi_read;
	mtd->_write               = mtd_spi_write;
	mtd->_sync                = mtd_spi_sync;
	mtd->_lock                = mtd_spi_lock;
	mtd->_unlock              = mtd_spi_unlock;
	mtd->_suspend             = mtd_spi_suspend;
	mtd->_resume              = mtd_spi_resume;
#endif
	mtd->priv                = (void *)map;
#ifdef CONFIG_RTL_TWO_SPI_FLASH_ENABLE
	if (0 == chip_info->chip_select) map->fldrv = &spi_chipdrv1;
	else map->fldrv          = &spi_chipdrv2;
#else
	map->fldrv = &spi_chipdrv1;
#endif

	map->fldrv_priv          = chip_info;
	mtd->writesize 			 = 1;

	printk(KERN_INFO "SPI flash(%s) was found at CS%d, size 0x%x\n",
		chip_info->flash->name, chip_info->chip_select, chip_info->flash->deviceSize );

	return mtd;
}

static struct mtd_info *spi_probe_chip(struct map_info *map, struct chip_probe *cp)
{
	struct mtd_info *mtd = NULL;
	struct spi_chip_info *chip_info = NULL;
	int i;

	for (i = 0; probe_func[i]; i++) {
		chip_info = probe_func[i](map, cp);
		if (!chip_info) continue;

		mtd = spi_chip_setup(map, chip_info);

		if (!mtd) {
			kfree(chip_info);
			continue;
		}

		return mtd;
	}
	return NULL;
}

static struct chip_probe spi_chip_probe1 = {
	name: "SPI1",
};
#ifdef CONFIG_RTL_TWO_SPI_FLASH_ENABLE
static struct chip_probe spi_chip_probe2 = {
	name: "SPI2",
};
#endif
struct mtd_info *spi_probe1(struct map_info *map)
{
	KDEBUG("SPI flash 1\n");
	return spi_probe_chip(map, &spi_chip_probe1);
}
#ifdef CONFIG_RTL_TWO_SPI_FLASH_ENABLE
struct mtd_info *spi_probe2(struct map_info *map)
{
	KDEBUG("SPI flash 2\n");
	return spi_probe_chip(map, &spi_chip_probe2);
}
#endif
static void spi_destroy1(struct mtd_info *mtd)
{
	KDEBUG("SPI flash 1\n");
	struct map_info *map = (struct map_info *)mtd->priv;
	struct spi_chip_info *chip_info = (struct spi_chip_info *)map->fldrv_priv;

	if (chip_info->destroy) {
		chip_info->destroy(chip_info);
	}
}
#ifdef CONFIG_RTL_TWO_SPI_FLASH_ENABLE
static void spi_destroy2(struct mtd_info *mtd)
{
	KDEBUG("SPI flash 2\n");
	struct map_info *map = (struct map_info *)mtd->priv;
	struct spi_chip_info *chip_info = (struct spi_chip_info *)map->fldrv_priv;

	if (chip_info->destroy) {
		chip_info->destroy(chip_info);
	}
}
#endif
int __init spi_probe_init(void)
{
	printk("SPI INIT\n");
	register_mtd_chip_driver(&spi_chipdrv1);
#ifdef CONFIG_RTL_TWO_SPI_FLASH_ENABLE
	register_mtd_chip_driver(&spi_chipdrv2);
#endif
	return 0;
}

void __exit spi_probe_exit(void)
{
	unregister_mtd_chip_driver(&spi_chipdrv1);
#ifdef CONFIG_RTL_TWO_SPI_FLASH_ENABLE
	unregister_mtd_chip_driver(&spi_chipdrv2);
#endif
}

module_init(spi_probe_init);
module_exit(spi_probe_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Masahiro Nakai <nakai@atmark-techno.com> et al.");
MODULE_DESCRIPTION("Probe code for SPI flash chips");
