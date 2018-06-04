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

#include "bspchip.h"

#define ERASE_SIZE	0x1000		//tmp code,modify if flash driver modified
#ifdef CONFIG_RTL_802_1X_CLIENT_SUPPORT
#define RTL_802_1X_CLIENT_SIZE 0x10000
#else
#define RTL_802_1X_CLIENT_SIZE ERASE_SIZE
#endif

#ifdef CONFIG_RTL_WAPI_SUPPORT
#define RTL_WAPI_SIZE 0x10000
#else
#define RTL_WAPI_SIZE ERASE_SIZE
#endif

#ifdef CONFIG_APPLE_HOMEKIT_BLOCK_SUPPORT
#define RTL_HOMEKIT_SIZE 0x10000
#else
#define RTL_HOMEKIT_SIZE ERASE_SIZE
#endif

#ifdef CONFIG_RTL_FLASH_MAPPING_ENABLE
#if defined(CONFIG_ROOTFS_JFFS2)
static struct mtd_partition sheipa_mtd_parts[] = {
        {
                name:           "boot+cfg",
                size:           (CONFIG_RTL_LINUX_IMAGE_OFFSET-0),
                offset:         0x00000000,
        },
        {
                name:           "jffs2(linux+root fs)",
#ifdef CONFIG_RTL_TWO_SPI_FLASH_ENABLE
#ifdef CONFIG_MTD_CONCAT
                size:        (CONFIG_RTL_SPI_FLASH1_SIZE + \
					  CONFIG_RTL_SPI_FLASH2_SIZE - \
					  CONFIG_RTL_ROOT_IMAGE_OFFSET - \
					  RTL_802_1X_CLIENT - RTL_WAPI_SIZE - RTL_HOMEKIT_SIZE),
#else
                size:        (CONFIG_RTL_SPI_FLASH1_SIZE - \
					  CONFIG_RTL_ROOT_IMAGE_OFFSET - \
					  RTL_802_1X_CLIENT - RTL_WAPI_SIZE - RTL_HOMEKIT_SIZE),
#endif
#else
                size:        (WINDOW_SIZE - CONFIG_RTL_ROOT_IMAGE_OFFSET - \
					  RTL_802_1X_CLIENT - RTL_WAPI_SIZE - RTL_HOMEKIT_SIZE),
#endif
                offset:      (CONFIG_RTL_ROOT_IMAGE_OFFSET),
        }
		,
		{
			name:	"1x",
			size:	(RTL_802_1X_CLIENT_SIZE-0),
#ifdef CONFIG_RTL_TWO_SPI_FLASH_ENABLE
#ifdef CONFIG_MTD_CONCAT
			offset:	(CONFIG_RTL_SPI_FLASH1_SIZE + \
			CONFIG_RTL_SPI_FLASH2_SIZE - \
					 RTL_802_1X_CLIENT_SIZE - RTL_WAPI_SIZE - RTL_HOMEKIT_SIZE),
#else
			offset:	(CONFIG_RTL_SPI_FLASH1_SIZE - \
			RTL_802_1X_CLIENT_SIZE - RTL_WAPI_SIZE - RTL_HOMEKIT_SIZE),
#endif
#else
			offset:	(WINDOW_SIZE - RTL_802_1X_CLIENT_SIZE - \
			RTL_WAPI_SIZE - RTL_HOMEKIT_SIZE),
#endif
		}
		,
		{
			name:	"wapi",
			size:	(RTL_WAPI_SIZE-0),
#ifdef CONFIG_RTL_TWO_SPI_FLASH_ENABLE
#ifdef CONFIG_MTD_CONCAT
			offset:	(CONFIG_RTL_SPI_FLASH1_SIZE + \
					CONFIG_RTL_SPI_FLASH2_SIZE - \
					RTL_WAPI_SIZE - RTL_HOMEKIT_SIZE ),
#else
			offset:	(CONFIG_RTL_SPI_FLASH1_SIZE - \
					RTL_WAPI_SIZE - RTL_HOMEKIT_SIZE  ),
#endif
#else
			offset:	(WINDOW_SIZE - RTL_WAPI_SIZE - RTL_HOMEKIT_SIZE ),
#endif
		}
		,
		{
			name:	"homekit",
			size:	(RTL_HOMEKIT_SIZE-0),
#ifdef CONFIG_RTL_TWO_SPI_FLASH_ENABLE
#ifdef CONFIG_MTD_CONCAT
			offset:	(CONFIG_RTL_SPI_FLASH1_SIZE + \
				CONFIG_RTL_SPI_FLASH2_SIZE - \
				RTL_HOMEKIT_SIZE ),
#else
			offset:	(CONFIG_RTL_SPI_FLASH1_SIZE - \
				RTL_HOMEKIT_SIZE ),
#endif
#else
			offset:	(WINDOW_SIZE - RTL_HOMEKIT_SIZE),
#endif
		}


};
#elif defined(CONFIG_ROOTFS_RAMFS)
static struct mtd_partition sheipa_mtd_parts[] = {
        {
                name:        "boot+cfg+linux+rootfs",
                size:        (CONFIG_RTL_FLASH_SIZE-0),
                offset:      0x00000000,
        },
};

#elif defined(CONFIG_ROOTFS_SQUASH)
#ifndef CONFIG_RTL_FLASH_DUAL_IMAGE_ENABLE
static struct mtd_partition sheipa_mtd_parts[] = {
        {
                name: "boot+cfg+linux",
                size:           (CONFIG_RTL_ROOT_IMAGE_OFFSET-0),
                offset:         0x00000000,
        },
        {
                name:           "root fs",
#ifdef CONFIG_RTL_TWO_SPI_FLASH_ENABLE
#ifdef CONFIG_MTD_CONCAT
                size:        (CONFIG_RTL_SPI_FLASH1_SIZE + \
					  CONFIG_RTL_SPI_FLASH2_SIZE - \
					  CONFIG_RTL_ROOT_IMAGE_OFFSET - \
					  RTL_802_1X_CLIENT_SIZE - RTL_WAPI_SIZE - RTL_HOMEKIT_SIZE),
#else
				size:        (CONFIG_RTL_SPI_FLASH1_SIZE - \
							  CONFIG_RTL_ROOT_IMAGE_OFFSET - \
							  RTL_802_1X_CLIENT_SIZE - RTL_WAPI_SIZE - RTL_HOMEKIT_SIZE),
#endif
#else
#if !defined(CONFIG_MTD_CHAR)
                size:        (CONFIG_RTL_FLASH_SIZE - \
					  CONFIG_RTL_ROOT_IMAGE_OFFSET - \
					  RTL_802_1X_CLIENT_SIZE - RTL_WAPI_SIZE - RTL_HOMEKIT_SIZE),
#else
				size:        (CONFIG_RTL_FLATFS_IMAGE_OFFSET - \
							  CONFIG_RTL_ROOT_IMAGE_OFFSET - \
							  RTL_802_1X_CLIENT_SIZE - RTL_WAPI_SIZE - RTL_HOMEKIT_SIZE),
#endif
#endif
                offset:         (CONFIG_RTL_ROOT_IMAGE_OFFSET),
        }
		,
		{
			name:	"1x",
			size:	(RTL_802_1X_CLIENT_SIZE-0),
#ifdef CONFIG_RTL_TWO_SPI_FLASH_ENABLE
#ifdef CONFIG_MTD_CONCAT
			offset: (CONFIG_RTL_SPI_FLASH1_SIZE + \
					CONFIG_RTL_SPI_FLASH2_SIZE - \
					RTL_802_1X_CLIENT_SIZE - RTL_WAPI_SIZE - RTL_HOMEKIT_SIZE),
#else
			offset: (CONFIG_RTL_SPI_FLASH1_SIZE - \
			RTL_802_1X_CLIENT_SIZE - RTL_WAPI_SIZE - RTL_HOMEKIT_SIZE),
#endif
#else
#ifdef CONFIG_MTD_CHAR
			offset: (CONFIG_RTL_FLATFS_IMAGE_OFFSET - \
					RTL_802_1X_CLIENT_SIZE - RTL_WAPI_SIZE - RTL_HOMEKIT_SIZE),
#else
			offset: (CONFIG_RTL_FLASH_SIZE - \
					RTL_802_1X_CLIENT_SIZE - RTL_WAPI_SIZE - RTL_HOMEKIT_SIZE),
#endif
#endif
		}
		,
		{
			name:	"wapi",
			size:	(RTL_WAPI_SIZE-0),
#ifdef CONFIG_RTL_TWO_SPI_FLASH_ENABLE
#ifdef CONFIG_MTD_CONCAT
			offset: (CONFIG_RTL_SPI_FLASH1_SIZE + \
					CONFIG_RTL_SPI_FLASH2_SIZE - \
					RTL_WAPI_SIZE - RTL_HOMEKIT_SIZE),
#else
			offset: (CONFIG_RTL_SPI_FLASH1_SIZE - \
					RTL_WAPI_SIZE - RTL_HOMEKIT_SIZE),
#endif
#else
#ifdef CONFIG_MTD_CHAR
			offset: (CONFIG_RTL_FLATFS_IMAGE_OFFSET - \
					RTL_WAPI_SIZE - RTL_HOMEKIT_SIZE),
#else
			offset: (CONFIG_RTL_FLASH_SIZE - \
					RTL_WAPI_SIZE - RTL_HOMEKIT_SIZE),
#endif
#endif
		}
		,
		{
			name:	"homekit",
			size:	(RTL_HOMEKIT_SIZE-0),
#ifdef CONFIG_RTL_TWO_SPI_FLASH_ENABLE
#ifdef CONFIG_MTD_CONCAT
			offset: (CONFIG_RTL_SPI_FLASH1_SIZE + \
					CONFIG_RTL_SPI_FLASH2_SIZE - \
					RTL_HOMEKIT_SIZE),
#else
			offset: (CONFIG_RTL_SPI_FLASH1_SIZE - \
					RTL_HOMEKIT_SIZE),
#endif
#else
#ifdef CONFIG_MTD_CHAR
			offset: (CONFIG_RTL_FLATFS_IMAGE_OFFSET - \
					RTL_HOMEKIT_SIZE),
#else
			offset: (CONFIG_RTL_FLASH_SIZE - \
					RTL_HOMEKIT_SIZE),
#endif
#endif
		}

#if defined(CONFIG_MTD_CHAR)
	,
          {
                name:           "flatfs",

                size:        (CONFIG_RTL_FLASH_SIZE-CONFIG_RTL_FLATFS_IMAGE_OFFSET),
                offset:         (CONFIG_RTL_FLATFS_IMAGE_OFFSET),
        }
#endif

};

#else //!CONFIG_RTL_FLASH_DUAL_IMAGE_ENABLE
static struct mtd_partition sheipa_mtd_parts[] = {
        {
                name: "boot+cfg+linux(bank1)",
                size:           (CONFIG_RTL_ROOT_IMAGE_OFFSET-0),
                offset:         0x00000000,
        },
        {
                name:           "root fs(bank1)",
                size:        	(CONFIG_RTL_FLASH_SIZE - \
						 CONFIG_RTL_ROOT_IMAGE_OFFSET - \
						 RTL_802_1X_CLIENT_SIZE - RTL_WAPI_SIZE - RTL_HOMEKIT_SIZE),
                offset:         (CONFIG_RTL_ROOT_IMAGE_OFFSET),
        },
		{
			name:	"1x",
			size:	(RTL_802_1X_CLIENT_SIZE - 0),
			offset:	(CONFIG_RTL_FLASH_SIZE - \
					 RTL_802_1X_CLIENT_SIZE - RTL_WAPI_SIZE - RTL_HOMEKIT_SIZE),
		},
		{
			name:	"wapi",
			size:	(RTL_WAPI_SIZE - 0),
			offset: (CONFIG_RTL_FLASH_SIZE - \
					 RTL_WAPI_SIZE - RTL_HOMEKIT_SIZE),
		},
		{
			name:	"homekit",
			size:	(RTL_WAPI_SIZE - 0),
			offset: (CONFIG_RTL_FLASH_SIZE - \
					 RTL_HOMEKIT_SIZE),
		},

        {
                name: "linux(bank2)",
                size:           (CONFIG_RTL_ROOT_IMAGE_OFFSET-0),
                offset:         CONFIG_RTL_FLASH_DUAL_IMAGE_OFFSET,
        },
        {
                name:           "root fs(bank2)",
                size:        (CONFIG_RTL_FLASH_SIZE-CONFIG_RTL_ROOT_IMAGE_OFFSET),
                offset:         CONFIG_RTL_FLASH_DUAL_IMAGE_OFFSET+(CONFIG_RTL_ROOT_IMAGE_OFFSET),
        }

};
#endif //CONFIG_RTL_FLASH_DUAL_IMAGE_ENABLE

#else
#error "unknow flash filesystem type"
#endif

#else // !CONFIG_RTL_FLASH_MAPPING_ENABLE
#ifndef CONFIG_OPENWRT_SDK
static struct mtd_partition sheipa_mtd_parts[] = {
	[0] = {
		.name	= "boot+cfg+linux",
		.offset	= 0,
		.size	= 0x001f0000,
		.mask_flags = 0
	},
	[1] = {
		.name	= "rootfs",
		.offset	= MTDPART_OFS_APPEND,
		.size	= MTDPART_SIZ_FULL,
		.mask_flags = 0
	},
};
#else
// size and offset will be decide using detect function , they are meanless here
static struct mtd_partition sheipa_mtd_parts[] = {
      {name: "boot", offset: 0, size:0x30000,},
      {name: "linux", offset: 0x30000,    size:0x130000,}, //0x130000+ rootfs
      {name: "rootfs", offset: 0x130000, size:0x2d0000,},
#ifdef CONFIG_RTK_BOOTINFO_DUALIMAGE
      //{name: "boot_backup", offset: 0x400000, size:0x30000,},
      {name: "linux_backup", offset: (0x400000+0x30000),    size:0x130000,}, //0x130000+ rootfs
      //{name: "roots_backup", offset: (0x400000+0x130000), size:0x2d0000,},
    //  {name: "prev_dmesg", offset: 0x7f0000,    size:0x10000,},
    //  {name: "priv_data1", offset: 0xfe0000,    size:0x10000,}, //0x130000+ rootfs
    //  {name: "priv_data2", offset: 0xff0000,    size:0x10000,}, //0x130000+ rootfs
      //20170925: free usable space
      // because of alignment: 0x80000-0x85000
      // also priv_data2
#endif
	};
#endif
#endif
static struct flash_platform_data sheipa_mtd_data = {
		.name		= "m25p80",
		.type		= "m25p80",
		.parts		= sheipa_mtd_parts,
		.nr_parts	= ARRAY_SIZE(sheipa_mtd_parts),
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

#define RTK_DETECT_FLASH_LAYOUT 1
#define FLASH_MEM_MAP_ADDR 0xb0000000 //general read
#define SQUASHFS_MAGIC_LE 0x68737173
//#define RTK_FLASH_BANK_SIZE 0x800000
#define RTK_FLASH_BANK_SIZE  CONFIG_RTL_FLASH_SIZE //mark_bb

#ifndef CONFIG_OPENWRT_SDK
void detect_flash_map(struct mtd_partition *rtl819x_parts)
{
   int offset =0;
   struct squashfs_super_block *sb;
   char *buf;
   uint32_t mtd_size;


   if(RTK_FLASH_BANK_SIZE != 0)
		mtd_size = RTK_FLASH_BANK_SIZE;

   buf = (char *)FLASH_MEM_MAP_ADDR;

   while (offset < mtd_size) {
        if (*((unsigned int *)buf) == ntohl(SQUASHFS_MAGIC_LE)) {
		printk("mtd_size=%8x found squashfs at %8x\n",mtd_size ,offset);
		sb = (struct squashfs_super_block *)buf;
		rtl819x_parts[0].size =  offset;
                break;
            }
        offset += 4096;
        buf += 4096;
    }
}
#else //OpenWRT below
unsigned int  RTK_LINUX_PART_OFFSET=0x30000; //defaut
unsigned int HW_SETTING_OFFSET = 0x20000;

#ifdef CONFIG_RTK_BOOTINFO_DUALIMAGE
#define BOOTOFFSET_NAME "bootoffset"
static struct proc_dir_entry *bootoffset_proc_entry;

#ifdef CONFIG_RTL_8198C
static unsigned int FLASH_BOOTINFO_OFFSET=0x2a000;
#else
#ifdef CONFIG_RTL_8197F //97F , is possible to use 64k erase flash , hence don't reuse 2a000 as bootinfo offset , use 30000 as new one
static unsigned int FLASH_BOOTINFO_OFFSET=0x30000;
#else //9XD/8881A/96E
static unsigned int FLASH_BOOTINFO_OFFSET=0xc000;
#endif
#endif

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
	 //if not valid , printk to error and reboot
	 bsp_reboot() ; //call from arch/mish/realtek/setup.c

	return ; //fail
}
//#endif
#endif

void detect_flash_map(struct mtd_partition *rtl819x_parts)
{
   int offset =0;
   struct squashfs_super_block *sb;
   char *buf;
   uint32_t mtd_size;
   uint32_t active_bank_offset=0;

#ifdef CONFIG_RTK_BOOTINFO_DUALIMAGE
    int bootbank=0;
    uint32_t back_bank_offset = 0;
    BOOTINFO_P bootinfo_p;
    bootinfo_p = 	&bootinfo_ram;
#endif

   if(RTK_FLASH_BANK_SIZE != 0)
	mtd_size = RTK_FLASH_BANK_SIZE;
   //else
	//mtd_size = (uint32_t )mtd->size;

#ifdef CONFIG_RTK_BOOTINFO_DUALIMAGE
   //mtd_size = mtd_size / 2;  // split to half size for dual image
   mtd_size = CONFIG_RTK_DUALIMAGE_FLASH_OFFSET;

   back_bank_offset = mtd_size;

   bootbank = bootinfo_p->data.field.bootbank;

   if(bootbank == 1 )
    {
	active_bank_offset = mtd_size;
	back_bank_offset = 0;
   }
#endif

   buf =FLASH_MEM_MAP_ADDR + active_bank_offset ;
   //boot partioin
   //rtl819x_parts[0].offset = active_bank_offset;
   rtl819x_parts[0].offset = 0; //boot+hw always from 0.
   rtl819x_parts[0].size =  RTK_LINUX_PART_OFFSET;

   // find active rootfs and linux
   //while ((offset + mtd->erasesize) < mtd_size) {
   while ((offset ) < mtd_size) {
	//printk(KERN_EMERG "[0x%08X] = [0x%08X]!=[0x%08X]\n",offset,*((unsigned int *) buf),SQUASHFS_MAGIC_LE);
		//if (*((__u32 *)buf) == SQUASHFS_MAGIC_LE) {
		if (*((unsigned int *)buf) == ntohl(SQUASHFS_MAGIC_LE)) {
			//printk("mtd_size=%8x found squashfs at %8x\n",flash_size ,offset);
			sb = (struct squashfs_super_block *)buf;
			       rtl819x_parts[2].offset = offset+ active_bank_offset;
				rtl819x_parts[2].size =  (mtd_size+active_bank_offset) - rtl819x_parts[2].offset;

				//rtl819x_parts[1].offset = rtl819x_parts[0].offset +rtl819x_parts[0].size;
				rtl819x_parts[1].offset = active_bank_offset + RTK_LINUX_PART_OFFSET;
				rtl819x_parts[1].size =  (mtd_size+active_bank_offset) - rtl819x_parts[1].offset;
				break;
			}
		offset += 4096;
		buf += 4096;
	}
#ifdef CONFIG_RTK_BOOTINFO_DUALIMAGE
       //fiil backup patiotn info
	//linux partioin
	rtl819x_parts[3].offset = back_bank_offset + RTK_LINUX_PART_OFFSET;
       rtl819x_parts[3].size =  (mtd_size+back_bank_offset) - rtl819x_parts[3].offset;

	//rtl819x_parts[3].offset = back_bank_offset;
       //rtl819x_parts[3].size =  RTK_LINUX_PART_OFFSET;

	 //rtl819x_parts[4].offset =rtl819x_parts[3].offset +rtl819x_parts[3].size;
	 //rtl819x_parts[4].size =  (mtd_size+back_bank_offset) - rtl819x_parts[4].offset;

	//rtl819x_parts[5].offset = rtl819x_parts[4].offset + 0x100000 ; //it's not important here
	//rtl819x_parts[5].size =  (mtd_size+back_bank_offset) - rtl819x_parts[5].offset;
 #endif
}

//read the kerenl command to get the linux part info
static int __init get_linux_part(char *str)
{
	unsigned int  linux_part=0;
	sscanf(str, "0x%x", &linux_part);
	RTK_LINUX_PART_OFFSET = linux_part;
        return 1;
}
__setup("linuxpart=", get_linux_part);

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
#endif

#ifdef CONFIG_OPENWRT_SDK
#include <linux/proc_fs.h>
#include <linux/seq_file.h>

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

//mark_hw
RTK_ADD_PROC(hwpart,hwpart_show);

#ifdef CONFIG_RTK_BOOTINFO_DUALIMAGE

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
#endif

static void  rtk_init_flash_proc(void)
{
	flash_proc_dir = proc_mkdir(FLASH_PROC_DIR_NAME,NULL);

	if(!flash_proc_dir)
		return;

	//init hwpart proc
	hwpart_proc_entry = proc_create(HWPART_NAME,0,flash_proc_dir,&hwpart_fops);

#ifdef CONFIG_RTK_BOOTINFO_DUALIMAGE
	bootoffset_proc_entry = proc_create(BOOTOFFSET_NAME,0,flash_proc_dir,&bootoffset_fops);
	rtk_init_bootinfo(&bootinfo_ram);
#endif

}
#endif
int __init plat_spi_init(void)
{
	printk("INFO: registering sheipa spi device\n");

#ifdef CONFIG_OPENWRT_SDK
	rtk_init_flash_proc();
#endif
#ifdef RTK_DETECT_FLASH_LAYOUT
	detect_flash_map(&sheipa_mtd_parts);
#endif
	spi_register_board_info(sheipa_spi_devs, ARRAY_SIZE(sheipa_spi_devs));
	platform_device_register(&sheipa_spi_device);
	return 0;
}
subsys_initcall(plat_spi_init);
