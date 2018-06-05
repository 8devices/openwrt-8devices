/*
 *
 *  Copyright (c) 2011 Realtek Semiconductor Corp.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 */
#ifndef CONFIG_WRT_BARRIER_BREAKER
#include <linux/config.h>
#else
#include <generated/autoconf.h>
#include <linux/slab.h>
#endif
#include <linux/mtd/map.h>
#include <linux/mtd/gen_probe.h>
#include <linux/spinlock.h>

#include "spi_flash.h"
#include "spi_common.h"

#define MTD_SPI_DEBUG 0
#if (MTD_SPI_DEBUG)
#define KDEBUG(args...) printk(args)
#else
#define KDEBUG(args...)
#endif

struct spi_chip_mtd spi_probe_mtd;
extern struct spi_flash_type spi_flash_info[2];

// uiAddr = from; pucBuffer = to; uiLen = size
static unsigned int do_spi_read(unsigned int from, unsigned int to, unsigned int size, unsigned int uiChip)
{
	unsigned int uiRet;
	uiRet = spi_flash_info[uiChip].pfRead(uiChip, from, size, (unsigned char*)to);
	KDEBUG("do_spi_read: from=%x; to=%x; size=%x; uiRet=%x\n", from, to, size, uiRet);
	return 0;
}
/*
// uiAddr = to; pucBuffer = from; uiLen = size
static unsigned int do_spi_write(unsigned int  from, unsigned int  to, unsigned int  size)
{
	unsigned int uiRet;
	uiRet = spi_flash_info[0].pfWrite(0, to, size, (unsigned char*)from);
	KDEBUG("do_spi_write: from=%x; to=%x; size=%x; uiRet=%x\n", from, to, size, uiRet);
	return 0;
}
*/
// uiAddr = to; pucBuffer = from; uiLen = size (stupid!!!)
static unsigned int do_spi_write(unsigned int from, unsigned int to, unsigned int size, unsigned int uiChip)
{
	unsigned int uiStartAddr, uiStartLen, uiPageAddr, uiPageCount, uiEndAddr, uiEndLen, i, uiRet;
	unsigned char* puc = (unsigned char*)from;
	KDEBUG("do_spi_write:from=%x; to=%x; size=%x;\n", from, to, size);
	calAddr(to, size, spi_flash_info[uiChip].page_size, &uiStartAddr, &uiStartLen, &uiPageAddr, &uiPageCount, &uiEndAddr, &uiEndLen);
	if((uiPageCount == 0x00) && (uiEndLen == 0x00))	// all data in the same page
	{
		uiRet = spi_flash_info[uiChip].pfPageWrite(uiChip, uiStartAddr, uiStartLen, puc);
	}
	else
	{
		if(uiStartLen > 0)
		{
			uiRet = spi_flash_info[uiChip].pfPageWrite(uiChip, uiStartAddr, uiStartLen, puc);
			puc += uiStartLen;
		}
		for(i = 0; i < uiPageCount; i++)
		{
			uiRet = spi_flash_info[uiChip].pfPageWrite(uiChip, uiPageAddr, spi_flash_info[uiChip].page_size, puc);
			puc += spi_flash_info[uiChip].page_size;
			uiPageAddr += spi_flash_info[uiChip].page_size;
		}
		if(uiEndLen > 0)
		{
			uiRet = spi_flash_info[uiChip].pfPageWrite(uiChip, uiEndAddr, uiEndLen, puc);
		}
	}
	//ComSrlCmd_WRDI(0);
	return 0;
}
// uiAddr = addr
static int do_spi_erase(unsigned int  addr, unsigned int uiChip)
{
	unsigned int uiRet;
	uiRet = spi_flash_info[uiChip].pfErase(uiChip, addr);
	KDEBUG("do_spi_erase: addr=%x;\n", addr);
	return 0;
}
// SPI flash destroy
static void spi_suzaku_destroy(struct spi_chip_info *chip_info)
{
	KDEBUG("spi_suzaku_destroy:\n");
	return;
}

// SPI flash probe
int spi_cp_probe(unsigned int uiChip)
{
	spi_regist(uiChip);

	spi_probe_mtd.chip_id = spi_flash_info[uiChip].chip_id;
	spi_probe_mtd.extra_id = spi_flash_info[uiChip].dev_id;
	spi_probe_mtd.sectorSize = spi_flash_info[uiChip].sector_size;
	spi_probe_mtd.deviceSize = spi_flash_info[uiChip].chip_size;
	spi_probe_mtd.uiClkMhz = 0;
	spi_probe_mtd.name = spi_flash_info[uiChip].chip_name;

	KDEBUG("spi_cp_probe:uiChip=%x\n", uiChip);
	return 0;
}

// malloc mtd
static struct spi_chip_info *spi_suzaku_setup(struct map_info *map)
{
	struct spi_chip_info *chip_info;
	chip_info = kmalloc(sizeof(*chip_info), GFP_KERNEL);
	if (!chip_info)
	{
		printk(KERN_WARNING "Failed to allocate memory for MTD device\n");
		return NULL;
	}
	memset(chip_info, 0, sizeof(struct spi_chip_info));
	KDEBUG("spi_suzaku_setup\n");
	return chip_info;
}


struct spi_chip_info *spi_probe_flash_chip(struct map_info *map, struct chip_probe *cp)
{
	struct spi_chip_info *chip_info = NULL;
	unsigned int chip_select=0; // 0 or 1

	if (!strcmp(cp->name,"SPI2"))
		chip_select=1;

	spi_cp_probe(chip_select);
	chip_info = spi_suzaku_setup(map);
	KDEBUG("spi_probe_flash_chip\n");
	if (chip_info)
	{
		chip_info->name = cp->name;
		chip_info->chip_select = chip_select;
		chip_info->flash	= &spi_probe_mtd;
		chip_info->destroy	= spi_suzaku_destroy;
		chip_info->read		= do_spi_read;
		chip_info->write	= do_spi_write;
		chip_info->erase	= do_spi_erase;
#ifdef RTK_FLASH_MUTEX
		mutex_init(&chip_info->lock);
#endif
		return chip_info;
	}
	else
	{
		return NULL;
	}
}

EXPORT_SYMBOL(spi_probe_flash_chip);
