/*
 * Realtek NAND flash driver
 *
 * Copyright (C) 2012 winfred_wang <winfred_wang@realsil.com.cn>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */

#include "rtknflash.h"
#include <linux/kernel.h>
#ifndef __UBOOT__
#include <linux/module.h>
#include <linux/slab.h>
#endif

MODULE_DESCRIPTION("NAND flash driver for Realtek bus");
MODULE_LICENSE("GPL");
MODULE_AUTHOR("winfred_wang");

struct rtknflash *rtkn;
#ifndef __UBOOT__
static int __init rtknflash_init(void)
#else
int  rtknflash_init(void)
#endif
{
#ifndef __UBOOT__
	return board_nand_init();
#else
	board_nand_init();
#endif
}

#ifndef __UBOOT__
static void __exit rtknflash_exit(void)
#else
void  rtknflash_exit(void)
#endif
{
#ifdef CONFIG_RTK_REMAP_BBT
	if(rtkn->bbt)
		kfree(rtkn->bbt);
	if(rtkn->bbt_v2r)
		kfree(rtkn->bbt_v2r);
#endif
#ifdef CONFIG_RTK_NORMAL_BBT
	if(rtkn->bbt_nor)
		kfree(rtkn->bbt_nor);
#endif

	kfree(rtkn);

	return 0;
}

#ifndef __UBOOT__
module_init(rtknflash_init);
module_exit(rtknflash_exit);
#endif
