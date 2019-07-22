/*
 * SHEIPA SPI controller driver
 *
 * Author: Realtek PSP Group
 *
 * Copyright 2015, Realtek Semiconductor Corp.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/mtd/mtd.h>
#include <linux/mtd/nand.h>
#include <linux/mtd/partitions.h>
#include <linux/platform_device.h>
#include <linux/spi/spi.h>
#include <linux/spi/flash.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>

#include "bspchip.h"

static struct flash_platform_data sheipa_mtd_data = {
		.name		= "m25p80",
		.type		= "m25p80",
};

static struct spi_board_info sheipa_spi_devs[] __initdata = {
	{
		.modalias	= "m25p80",
		.max_speed_hz	= 40000000,
		.bus_num	= 0,
		.chip_select	= 0,
		.mode		= SPI_CPHA | SPI_CPOL,
		.platform_data	= &sheipa_mtd_data,
	},
};

static struct resource sheipa_spi_resources[] = {
	[0] = {
		.start	= BSP_SPIC_BASE,
		.end	= BSP_SPIC_BASE + BSP_SPIC_SIZE - 1,
		.flags	= IORESOURCE_MEM,
	},
	[1] = {
		.start	= BSP_SPIC_AUTO_BASE,
		.end	= BSP_SPIC_AUTO_BASE + BSP_SPIC_AUTO_SIZE - 1,
		.flags	= IORESOURCE_MEM,
	}
};

static struct platform_device sheipa_spi_device = {
	.name		= "spi-sheipa",
	.id		= 0,
	.resource	= sheipa_spi_resources,
	.num_resources	= ARRAY_SIZE(sheipa_spi_resources),
};

#define FLASH_MEM_MAP_ADDR 0xb0000000 //general read

unsigned int  RTK_LINUX_PART_OFFSET=0x30000; //defaut
unsigned int HW_SETTING_OFFSET = 0x20000;

#define BOOTOFFSET_NAME "bootoffset"
static struct proc_dir_entry *bootoffset_proc_entry;

static unsigned int FLASH_BOOTINFO_OFFSET=0x30000;

//#ifdef CONFIG_RTK_BOOTINFO_DUALIMAGE
#define TAG_LEN					4
#define RTK_BOOTINFO_SIGN "hwbt"
#define RTK_MAX_VALID_BOOTCNT 16

typedef struct bootinfo_header {
	unsigned char tag[TAG_LEN] ;  // Tag + version
	unsigned int len ;
} BOOTINFO_HEADER_T, *BOOTINFO_HEADER_Tp;

typedef union bootinfo_data{
		unsigned int 	val;
        struct {
			unsigned char	bootbank;
			unsigned char	bootmaxcnt;
			unsigned char	bootcnt;
			unsigned char	bootmode;
        } field;
} BOOTINFO_DATA_T, *BOOTINFO_DATA_P;

typedef struct bootinfo {
	BOOTINFO_HEADER_T header;
	BOOTINFO_DATA_T data;
}BOOTINFO_T, *BOOTINFO_P;

BOOTINFO_T bootinfo_ram;

extern void bsp_reboot(void);

static inline void rtk_read_bootinfo_from_flash(unsigned int bootinfo_addr,BOOTINFO_P bootinfo_ram_p)
{

	memset((char *)bootinfo_ram_p,0,sizeof(BOOTINFO_T));

	//flash(spi...etc)  can be read directly
	memcpy((char *)bootinfo_ram_p,(char *)bootinfo_addr,sizeof(BOOTINFO_T));
}

static void  rtk_init_bootinfo(BOOTINFO_P boot)
{
	rtk_read_bootinfo_from_flash(FLASH_MEM_MAP_ADDR+FLASH_BOOTINFO_OFFSET,boot);

	 // if not valid bootinfo header use default setting
	if(!memcmp(boot->header.tag, RTK_BOOTINFO_SIGN, TAG_LEN)) //ok
		return ;

	printk("Error!!Please update ur bootcode to support dualimage function !!\n");

	return ; //fail
}

static int __init get_hw_part(char *str)
{
	unsigned int  hw_part=0;

	sscanf(str, "0x%x", &hw_part);
	HW_SETTING_OFFSET = hw_part;
	//printk("get_hw_part = %x \n",HW_SETTING_OFFSET);
        return 1;
}
__setup("hwpart=", get_hw_part);

EXPORT_SYMBOL(HW_SETTING_OFFSET);

static struct proc_dir_entry *flash_proc_dir;
static struct proc_dir_entry *hwpart_proc_entry;

#define FLASH_PROC_DIR_NAME "flash"
#define HWPART_NAME "hwpart"

#define RTK_ADD_PROC(name, show_fct) \
	static int name##_open(struct inode *inode, struct file *file) \
	{ return single_open(file, show_fct, inode->i_private); }      \
	static const struct file_operations name##_fops = { \
		.owner = THIS_MODULE, \
		.open = name##_open, \
		.llseek = seq_lseek, \
		.read = seq_read, \
		.write = name##_write, \
		.release = single_release \
	};


static int hwpart_show(struct seq_file *s, void *p)
{
	int pos = 0;

	pos += seq_printf(s, "%x", HW_SETTING_OFFSET);
	return pos;
}

static ssize_t hwpart_write(struct file * file, const char __user * userbuf,
                     size_t count, loff_t * off)
{
        return 0;
}
RTK_ADD_PROC(hwpart,hwpart_show);


static int bootoffset_show(struct seq_file *s, void *p)
{
	int pos = 0;

        pos += seq_printf(s, "%x", FLASH_BOOTINFO_OFFSET);
        return pos;
}

static ssize_t bootoffset_write(struct file * file, const char __user * userbuf,
                     size_t count, loff_t * off)
{
        return 0;
}
RTK_ADD_PROC(bootoffset,bootoffset_show);

static void  rtk_init_flash_proc(void)
{
	flash_proc_dir = proc_mkdir(FLASH_PROC_DIR_NAME,NULL);

	if(!flash_proc_dir)
		return;

	//init hwpart proc
	hwpart_proc_entry = proc_create(HWPART_NAME,0,flash_proc_dir,&hwpart_fops);

	bootoffset_proc_entry = proc_create(BOOTOFFSET_NAME,0,flash_proc_dir,&bootoffset_fops);
	rtk_init_bootinfo(&bootinfo_ram);

}

int __init plat_spi_init(void)
{
	printk("INFO: registering sheipa spi device\n");

	rtk_init_flash_proc();
	spi_register_board_info(sheipa_spi_devs, ARRAY_SIZE(sheipa_spi_devs));
	platform_device_register(&sheipa_spi_device);

	return 0;
}
subsys_initcall(plat_spi_init);
