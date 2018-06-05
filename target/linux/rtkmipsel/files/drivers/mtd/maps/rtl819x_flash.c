
/*
 * Flash mapping for rtl8196 board
 *
 * Copyright (C) 2008 Realtek Corporation
 *
 */

#include <linux/module.h>
#include <linux/types.h>
#include <linux/kernel.h>
#include <asm/io.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/map.h>
#include <linux/root_dev.h>
#include <linux/mtd/partitions.h>
//#include <linux/config.h>
#include <linux/delay.h>
#ifndef CONFIG_WRT_BARRIER_BREAKER
#include <linux/autoconf.h>
#else
#include <generated/autoconf.h>
#endif

#ifdef CONFIG_MTD_CONCAT
#include <linux/mtd/concat.h>
#endif

#define WINDOW_ADDR 0xbfe00000
#ifdef CONFIG_SPANSION_16M_FLASH
#define WINDOW_SIZE 0x1000000
#define FLASH_BANK_SIZE 0x400000
#else
#define WINDOW_SIZE 0x400000
#endif
#define BUSWIDTH 2

#ifdef CONFIG_RTL_TWO_SPI_FLASH_ENABLE
#define MAX_SPI_CS 2		/* Number of CS we are going to test */
#else
#define MAX_SPI_CS 1
#endif

#ifdef CONFIG_RTK_BOOTINFO_DUALIMAGE //mark_boot
#define CONFIG_RTK_BOOTINFO_SUPPORT 1
#endif

#define RTK_DETECT_FLASH_LAYOUT 1
#define FLASH_MEM_MAP_ADDR 0xbd000000 //general read

unsigned int FLASH_MEM_MAP_ADDR_W=FLASH_MEM_MAP_ADDR;

#define RTK_USERDEFINE_FLASH_BANK_SIZE 0 //You can define your flash bank size here , for example "0x400000" to 4M bank

unsigned int  RTK_LINUX_PART_OFFSET=0x30000; //defaut

unsigned int  RTK_FLASH_BANK_SIZE=RTK_USERDEFINE_FLASH_BANK_SIZE; //defaut

#ifndef CONFIG_RTL_8198C //mark_hw , default hw setting
unsigned int HW_SETTING_OFFSET = 0x6000;
#else
unsigned int HW_SETTING_OFFSET = 0x20000;
#endif

#ifdef CONFIG_RTK_BOOTINFO_SUPPORT
#include "rtk_bootinfo.c"

#ifndef CONFIG_RTL_8198C
#define FLASH_BOOTINFO_OFFSET 0xC000
#else //98C
#define FLASH_BOOTINFO_OFFSET 0x2a000
#endif

BOOTINFO_T bootinfo_ram;
#endif


static struct mtd_info *rtl8196_mtd;

__u8 rtl8196_map_read8(struct map_info *map, unsigned long ofs)
{
	//printk("enter %s %d\n",__FILE__,__LINE__);
	return __raw_readb(map->map_priv_1 + ofs);
}

__u16 rtl8196_map_read16(struct map_info *map, unsigned long ofs)
{
	//printk("enter %s %d\n",__FILE__,__LINE__);
	return __raw_readw(map->map_priv_1 + ofs);
}

__u32 rtl8196_map_read32(struct map_info *map, unsigned long ofs)
{
	//printk("enter %s %d\n",__FILE__,__LINE__);
	return __raw_readl(map->map_priv_1 + ofs);
}

void rtl8196_map_copy_from(struct map_info *map, void *to, unsigned long from, ssize_t len)
{
	//printk("enter to %x from  %x len %d\n",to, map->map_priv_1+from , len);
	//11/15/05' hrchen, change the size to fit file systems block size if use different fs
	//4096 for cramfs, 1024 for squashfs
	if (from>0x10000)
	    memcpy(to, map->map_priv_1 + from, (len<=1024)?len:1024);//len);
	else
	    memcpy(to, map->map_priv_1 + from, (len<=4096)?len:4096);//len);
	//printk("enter %s %d\n", __FILE__,__LINE__);

}

void rtl8196_map_write8(struct map_info *map, __u8 d, unsigned long adr)
{
	__raw_writeb(d, map->map_priv_1 + adr);
	mb();
}

void rtl8196_map_write16(struct map_info *map, __u16 d, unsigned long adr)
{
	__raw_writew(d, map->map_priv_1 + adr);
	mb();
}

void rtl8196_map_write32(struct map_info *map, __u32 d, unsigned long adr)
{
	__raw_writel(d, map->map_priv_1 + adr);
	mb();
}

void rtl8196_map_copy_to(struct map_info *map, unsigned long to, const void *from, ssize_t len)
{
	//printk("enter %s %d\n",__FILE__,__LINE__);
	memcpy_toio(map->map_priv_1 + to, from, len);
}

static struct map_info spi_map[MAX_SPI_CS] = {
        {
			name: 		"flash_bank_1",
#ifdef CONFIG_RTL_TWO_SPI_FLASH_ENABLE
			size: 		CONFIG_RTL_SPI_FLASH1_SIZE,
#else
			size:			WINDOW_SIZE,
#endif
			phys: 		0xbd000000,
			virt: 			0xbd000000,
			bankwidth: 	BUSWIDTH

        },
#ifdef CONFIG_RTL_TWO_SPI_FLASH_ENABLE
        {
			name: 		"flash_bank_2",
			size: 		CONFIG_RTL_SPI_FLASH2_SIZE,
			phys: 		0xbe000000,
			virt: 			0xbe000000,
			bankwidth: 	BUSWIDTH
        }
#endif
};

static struct mtd_info *my_sub_mtd[MAX_SPI_CS] = {
	NULL,
#ifdef CONFIG_RTL_TWO_SPI_FLASH_ENABLE
	NULL
#endif
};

static struct mtd_info *mymtd;



/////////////////////////////////////////////////////////////////////////////

#ifdef CONFIG_RTL_FLASH_MAPPING_ENABLE
#if defined( CONFIG_ROOTFS_JFFS2 )
static struct mtd_partition rtl8196_parts1[] = {
        {
                name:           "boot+cfg",
                size:           (CONFIG_RTL_LINUX_IMAGE_OFFSET-0),
                offset:         0x00000000,
        },
        {
                name:           "jffs2(linux+root fs)",
#ifdef CONFIG_RTL_TWO_SPI_FLASH_ENABLE
#ifdef CONFIG_MTD_CONCAT
                size:        (CONFIG_RTL_SPI_FLASH1_SIZE+CONFIG_RTL_SPI_FLASH2_SIZE-CONFIG_RTL_ROOT_IMAGE_OFFSET),
#else
                size:        (CONFIG_RTL_SPI_FLASH1_SIZE-CONFIG_RTL_ROOT_IMAGE_OFFSET),
#endif
#else
                size:        (WINDOW_SIZE - CONFIG_RTL_ROOT_IMAGE_OFFSET),
#endif
                offset:      (CONFIG_RTL_ROOT_IMAGE_OFFSET),
        }
};
#elif defined( CONFIG_ROOTFS_RAMFS )
static struct mtd_partition rtl8196_parts1[] = {
        {
                name:        "boot+cfg+linux+rootfs",
                size:        (CONFIG_RTL_FLASH_SIZE-0),
                offset:      0x00000000,
        },
};

#elif defined( CONFIG_ROOTFS_SQUASH )
#ifndef CONFIG_RTL_FLASH_DUAL_IMAGE_ENABLE
static struct mtd_partition rtl8196_parts1[] = {
        {
                name: "boot+cfg+linux",
                size:           (CONFIG_RTL_ROOT_IMAGE_OFFSET-0),
                offset:         0x00000000,
        },
        {
                name:           "root fs",
#ifdef CONFIG_RTL_TWO_SPI_FLASH_ENABLE
#ifdef CONFIG_MTD_CONCAT
                size:        (CONFIG_RTL_SPI_FLASH1_SIZE+CONFIG_RTL_SPI_FLASH2_SIZE-CONFIG_RTL_ROOT_IMAGE_OFFSET),
#else
		  size:        (CONFIG_RTL_SPI_FLASH1_SIZE-CONFIG_RTL_ROOT_IMAGE_OFFSET),
#endif
#else
#if !defined(CONFIG_MTD_CHAR)
                size:        (CONFIG_RTL_FLASH_SIZE-CONFIG_RTL_ROOT_IMAGE_OFFSET),
#else
		size:        (CONFIG_RTL_FLATFS_IMAGE_OFFSET-CONFIG_RTL_ROOT_IMAGE_OFFSET),
#endif
#endif
                offset:         (CONFIG_RTL_ROOT_IMAGE_OFFSET),
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
static struct mtd_partition rtl8196_parts1[] = {
        {
                name: "boot+cfg+linux(bank1)",
                size:           (CONFIG_RTL_ROOT_IMAGE_OFFSET-0),
                offset:         0x00000000,
        },
        {
                name:           "root fs(bank1)",
                size:        (CONFIG_RTL_FLASH_SIZE-CONFIG_RTL_ROOT_IMAGE_OFFSET),
                offset:         (CONFIG_RTL_ROOT_IMAGE_OFFSET),
        },
        {
                name: "inux(bank2)",
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

#ifdef RTK_DETECT_FLASH_LAYOUT
// size and offset will be decide using detect function , they are meanless here
static struct mtd_partition rtl8196_parts1[] = {
      {name: "boot", offset: 0, size:0x30000,},
      {name: "linux", offset: 0x30000,    size:0x130000,}, //0x130000+ rootfs
      {name: "rootfs", offset: 0x130000, size:0x2d0000,},
#ifdef CONFIG_RTK_BOOTINFO_DUALIMAGE
      //{name: "boot_backup", offset: 0x400000, size:0x30000,},
      {name: "linux_backup", offset: (0x400000+0x30000),    size:0x130000,}, //0x130000+ rootfs
      //{name: "roots_backup", offset: (0x400000+0x130000), size:0x2d0000,},
#ifndef CONFIG_RTL_8198C
      {name: "priv_data1", offset: (0x400000+0x30000),    size:0x130000,}, //0x130000+ rootfs
      {name: "priv_data2", offset: (0x400000+0x30000),    size:0x130000,}, //0x130000+ rootfs
#else
      {name: "priv_data1", offset: 0x130000, size:0x2d0000,},
      {name: "priv_data2", offset: 0x130000, size:0x2d0000,},
#endif
#endif
};
#else
static struct mtd_partition rtl8196_parts1[] = {
        {
                name: "boot+cfg+linux",
                size:  0x00130000,
                offset:0x00000000,
        },
        {
                name:           "rootfs",
				size:        	0x002D0000,
                offset:         0x00130000,
        }
};
#endif

#endif


#ifdef RTK_DETECT_FLASH_LAYOUT
//#include <linux/magic.h>
#define SQUASHFS_MAGIC_LE 0x68737173

void detect_flash_map(struct mtd_info *mtd, struct mtd_partition *rtl819x_parts)
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
   else
	mtd_size = (uint32_t )mtd->size;

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
   while ((offset + mtd->erasesize) < mtd_size) {
	//printk(KERN_EMERG "[0x%08X] = [0x%08X]!=[0x%08X]\n",offset,*((unsigned int *) buf),SQUASHFS_MAGIC_LE);
		if (*((__u32 *)buf) == SQUASHFS_MAGIC_LE) {
			//printk("mtd_size=%8x found squashfs at %8x\n",flash_size ,offset);
			sb = (struct squashfs_super_block *)buf;
#ifdef CONFIG_RTK_BOOTINFO_DUALIMAGE //eric test back offset= flash-size/2 -0x10000
			       rtl819x_parts[2].offset = offset+ active_bank_offset;
#ifndef CONFIG_RTL_8198C
				rtl819x_parts[2].size =  (mtd_size+active_bank_offset) - rtl819x_parts[2].offset;
#else //8198C
#ifdef CONFIG_RTL_8198C_VDSL2 //32MB support
				rtl819x_parts[2].size =  (mtd_size+active_bank_offset) - rtl819x_parts[2].offset;
#else // 16MB support
				rtl819x_parts[2].size =  (mtd_size+active_bank_offset) - rtl819x_parts[2].offset-0x10000;
#endif //16MB
#endif //8198C

				//rtl819x_parts[1].offset = rtl819x_parts[0].offset +rtl819x_parts[0].size;
				rtl819x_parts[1].offset = active_bank_offset + RTK_LINUX_PART_OFFSET;
				rtl819x_parts[1].size =  (mtd_size+active_bank_offset) - rtl819x_parts[1].offset;
#endif //CONFIG_RTK_BOOTINFO_DUALIMAGE
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

	rtl819x_parts[4].offset = mtd_size+active_bank_offset+back_bank_offset;
	rtl819x_parts[4].size = 0x10000;
	rtl819x_parts[5].offset = mtd_size+active_bank_offset+back_bank_offset+0x10000;
	rtl819x_parts[5].size = 0x10000;


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

#ifdef CONFIG_RTL_TWO_SPI_FLASH_ENABLE
static struct mtd_partition rtl8196_parts2[] = {
        {
                name: 		"data",
                size:  		CONFIG_RTL_SPI_FLASH2_SIZE,
                offset:		0x00000000,
        }
};
#endif

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

#ifdef CONFIG_RTK_BOOTINFO_SUPPORT

extern unsigned int ComSrlCmd_ComWriteData(unsigned char ucChip, unsigned int uiAddr, unsigned int uiLen, unsigned char* pucBuffer)	;
void rtk_flash_write_data(unsigned int flash_addr,unsigned int len,unsigned char* data)
{
	ComSrlCmd_ComWriteData(0,flash_addr,len,data);
}

static void  rtk_init_bootinfo(BOOTINFO_P boot)
{
	rtk_read_bootinfo_from_flash(FLASH_MEM_MAP_ADDR+FLASH_BOOTINFO_OFFSET,boot);

	 if(!rtk_check_bootinfo(boot)) // if not valid bootinfo header use default setting
		rtk_reset_bootinfo(boot);
}

#define BOOTBANK_NAME "bootbank"
#define BOOTMAXCNT_NAME "bootmaxcnt"
#define BOOTCNT_NAME "bootcnt"
#define BOOTMODE_NAME "bootmode"
#define BOOTOFFSET_NAME "bootoffset"

static struct proc_dir_entry *bootbank_proc_entry;
static struct proc_dir_entry *bootcnt_proc_entry;
static struct proc_dir_entry *bootcntmax_proc_entry;
static struct proc_dir_entry *bootmode_proc_entry;
static struct proc_dir_entry *bootoffset_proc_entry;

static int bootbank_proc_write(struct file *file, const char *buffer,
				unsigned long count, void *data)
{
	char tmp[32];
	unsigned int bootbank;
	unsigned int num;
	BOOTINFO_P bootinfo_p;

	if (buffer && !copy_from_user(tmp, buffer, 32)) {
		num = sscanf(tmp, "%d", &bootbank);

		if ((num != 1) || ( bootbank > 1) ) {
			printk("Invalid bootbank!\n");
			return count;
		}
		bootinfo_p = 	&bootinfo_ram;
		bootinfo_p->data.field.bootbank = bootbank ;
		bootinfo_p->data.field.bootcnt = 0; //also reset cnt if switch bank

		rtk_write_bootinfo_to_flash(FLASH_MEM_MAP_ADDR_W+FLASH_BOOTINFO_OFFSET,&bootinfo_ram,rtk_flash_write_data);
	}

	return count;

}

static int bootcnt_proc_write(struct file *file, const char *buffer,
				unsigned long count, void *data)
{
	char tmp[32];
	unsigned int bootcnt;
	unsigned int num;
	BOOTINFO_P bootinfo_p;

	if (buffer && !copy_from_user(tmp, buffer, 32)) {
		num = sscanf(tmp, "%d", &bootcnt);

		if ((num != 1) || ( bootcnt > RTK_MAX_VALID_BOOTCNT) ) {
			printk("Invalid bootcnt!\n");
			return count;
		}
		bootinfo_p = 	&bootinfo_ram;
		bootinfo_p->data.field.bootcnt = bootcnt ;

		rtk_write_bootinfo_to_flash(FLASH_MEM_MAP_ADDR_W+FLASH_BOOTINFO_OFFSET,&bootinfo_ram,rtk_flash_write_data);
	}

	return count;

}

static int bootcntmax_proc_write(struct file *file, const char *buffer,
				unsigned long count, void *data)
{
	char tmp[32];
	unsigned int bootmaxcnt;
	unsigned int num;
	BOOTINFO_P bootinfo_p;

	if (buffer && !copy_from_user(tmp, buffer, 32)) {
		num = sscanf(tmp, "%d", &bootmaxcnt);

		if ((num != 1) || ( bootmaxcnt > RTK_MAX_VALID_BOOTCNT) ) {
			printk("Invalid bootmaxcnt!\n");
			return count;
		}
		bootinfo_p = 	&bootinfo_ram;
		bootinfo_p->data.field.bootmaxcnt = bootmaxcnt ;

		rtk_write_bootinfo_to_flash(FLASH_MEM_MAP_ADDR_W+FLASH_BOOTINFO_OFFSET,&bootinfo_ram,rtk_flash_write_data);
	}

	return count;

}

static int bootmode_proc_write(struct file *file, const char *buffer,
				unsigned long count, void *data)
{
	char tmp[32];
	unsigned int bootmode;
	unsigned int num;
	BOOTINFO_P bootinfo_p;

	if (buffer && !copy_from_user(tmp, buffer, 32)) {
		num = sscanf(tmp, "%d", &bootmode);

		if ((num != 1)) {
			printk("Invalid bootmode!\n");
			return count;
		}
		bootinfo_p = 	&bootinfo_ram;
		bootinfo_p->data.field.bootmode = bootmode ;

		rtk_write_bootinfo_to_flash(FLASH_MEM_MAP_ADDR_W+FLASH_BOOTINFO_OFFSET,&bootinfo_ram,rtk_flash_write_data);
	}

	return count;

}

#ifdef CONFIG_WRT_BARRIER_BREAKER

static int bootbank_show(struct seq_file *s, void *p)
{
	int pos = 0;
	BOOTINFO_P bootinfo_p;
	bootinfo_p = 	&bootinfo_ram;

	pos += seq_printf(s, "%d", bootinfo_p->data.field.bootbank);
	return pos;
}

static ssize_t bootbank_write(struct file * file, const char __user * userbuf,
                     size_t count, loff_t * off)
{
        return bootbank_proc_write(file, userbuf,count, off);
}

static int bootcnt_show(struct seq_file *s, void *p)
{
	int pos = 0;
	BOOTINFO_P bootinfo_p;
	bootinfo_p = 	&bootinfo_ram;

	pos += seq_printf(s, "%d", bootinfo_p->data.field.bootcnt);
	return pos;
}

static ssize_t bootcnt_write(struct file * file, const char __user * userbuf,
                     size_t count, loff_t * off)
{
        return bootcnt_proc_write(file, userbuf,count, off);
}

static int bootcntmax_show(struct seq_file *s, void *p)
{
	int pos = 0;
	BOOTINFO_P bootinfo_p;
	bootinfo_p = 	&bootinfo_ram;

	pos += seq_printf(s, "%d", bootinfo_p->data.field.bootmaxcnt);
	return pos;
}

static ssize_t bootcntmax_write(struct file * file, const char __user * userbuf,
                     size_t count, loff_t * off)
{
        return bootcntmax_proc_write(file, userbuf,count, off);
}

static int bootmode_show(struct seq_file *s, void *p)
{
	int pos = 0;
	BOOTINFO_P bootinfo_p;
	bootinfo_p = 	&bootinfo_ram;

	pos += seq_printf(s, "%d", bootinfo_p->data.field.bootmode);
	return pos;
}

static ssize_t bootmode_write(struct file * file, const char __user * userbuf,
                     size_t count, loff_t * off)
{
        return bootmode_proc_write(file, userbuf,count, off);
}

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

//add related proc fops here
RTK_ADD_PROC(bootbank,bootbank_show);
RTK_ADD_PROC(bootcnt,bootcnt_show);
RTK_ADD_PROC(bootcntmax,bootcntmax_show);
RTK_ADD_PROC(bootmode,bootmode_show);
RTK_ADD_PROC(bootoffset,bootoffset_show);

#else //old proc files
static int bootbank_proc_read( char *page, char **start, off_t off, int count, int *eof, void *data )
{
		int 	len;
		BOOTINFO_P bootinfo_p;
		bootinfo_p = 	&bootinfo_ram;

		len = sprintf(page, "%d",bootinfo_p->data.field.bootbank);
		return len;

}
static int bootcnt_proc_read( char *page, char **start, off_t off, int count, int *eof, void *data )
{
		int 	len;
		BOOTINFO_P bootinfo_p;
		bootinfo_p = 	&bootinfo_ram;

		len = sprintf(page, "%d",bootinfo_p->data.field.bootcnt);
		return len;

}

static int bootcntmax_proc_read( char *page, char **start, off_t off, int count, int *eof, void *data )
{
		int 	len;
		BOOTINFO_P bootinfo_p;
		bootinfo_p = 	&bootinfo_ram;

		len = sprintf(page, "%d",bootinfo_p->data.field.bootmaxcnt);
		return len;

}



static int bootmode_proc_read( char *page, char **start, off_t off, int count, int *eof, void *data )
{
		int 	len;
		BOOTINFO_P bootinfo_p;
		bootinfo_p = 	&bootinfo_ram;

		len = sprintf(page, "%d",bootinfo_p->data.field.bootmode);
		return len;

}
#endif
static void  rtk_init_bootinfo_proc(struct proc_dir_entry *flash_proc_dir)
{
#ifdef CONFIG_RTL_8198C //eric for realtek pathch_dualimage_0805
        //modify flash real address for write bootinfo correctly in 32M flash case
#define REG32(reg)      (*(volatile unsigned int *)(reg))
	if(REG32(0xb8000008)&0x1) //32M case
	    FLASH_MEM_MAP_ADDR_W = 0xbe000000;
#endif

	rtk_init_bootinfo(&bootinfo_ram);

#ifdef CONFIG_WRT_BARRIER_BREAKER
	bootbank_proc_entry = proc_create(BOOTBANK_NAME,0,flash_proc_dir,&bootbank_fops);
	bootoffset_proc_entry = proc_create(BOOTOFFSET_NAME,0,flash_proc_dir,&bootoffset_fops);
#else
	bootbank_proc_entry = create_proc_entry(BOOTBANK_NAME,0,flash_proc_dir);
	bootbank_proc_entry->read_proc = bootbank_proc_read;
	bootbank_proc_entry->write_proc= bootbank_proc_write;
#endif

	if(!bootbank_proc_entry)
		return;

#ifdef CONFIG_WRT_BARRIER_BREAKER
	bootcnt_proc_entry = proc_create(BOOTCNT_NAME,0,flash_proc_dir,&bootcnt_fops);
#else
	bootcnt_proc_entry = create_proc_entry(BOOTCNT_NAME,0,flash_proc_dir);
	bootcnt_proc_entry->read_proc = bootcnt_proc_read;
	bootcnt_proc_entry->write_proc= bootcnt_proc_write;
#endif
	if(!bootcnt_proc_entry)
		return;

#ifdef CONFIG_WRT_BARRIER_BREAKER
	bootcntmax_proc_entry = proc_create(BOOTMAXCNT_NAME,0,flash_proc_dir,&bootcntmax_fops);
#else
	bootcntmax_proc_entry = create_proc_entry(BOOTMAXCNT_NAME,0,flash_proc_dir);
	bootcntmax_proc_entry->read_proc = bootcntmax_proc_read;
	bootcntmax_proc_entry->write_proc= bootcntmax_proc_write;
#endif
	if(!bootcntmax_proc_entry)
		return;
#ifdef CONFIG_WRT_BARRIER_BREAKER
	bootmode_proc_entry = proc_create(BOOTMODE_NAME,0,flash_proc_dir,&bootmode_fops);
#else
	bootmode_proc_entry = create_proc_entry(BOOTMODE_NAME,0,flash_proc_dir);
	bootmode_proc_entry->read_proc = bootmode_proc_read;
	bootmode_proc_entry->write_proc=  bootmode_proc_write;
#endif
	if(!bootmode_proc_entry)
	return;
}
#endif

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

static void  rtk_init_flash_proc(void)
{
	flash_proc_dir = proc_mkdir(FLASH_PROC_DIR_NAME,NULL);

	if(!flash_proc_dir)
		return;

	//init hwpart proc
	hwpart_proc_entry = proc_create(HWPART_NAME,0,flash_proc_dir,&hwpart_fops);

#ifdef CONFIG_RTK_BOOTINFO_SUPPORT
	rtk_init_bootinfo_proc(flash_proc_dir);
#endif

}

#if LINUX_VERSION_CODE < 0x20212 && defined(MODULE)
#define init_rtl8196_map init_module
#define cleanup_rtl8196_map cleanup_module
#endif

#define mod_init_t  static int __init
#define mod_exit_t  static void __exit

mod_init_t init_rtl8196_map(void)
{
        int i,chips;
#ifdef SIZE_REMAINING
	struct mtd_partition *last_partition;
#endif
	rtk_init_flash_proc();

	chips = 0;
	for (i=0;i<MAX_SPI_CS;i++) {
		simple_map_init(&spi_map[i]);
		my_sub_mtd[i] = do_map_probe(spi_map[i].name, &spi_map[i]);

		if (my_sub_mtd[i]) {
			my_sub_mtd[i]->owner = THIS_MODULE;
			chips++;
			//printk("%s, %d, i=%d, chips=%d\n", __FUNCTION__, __LINE__, i, chips);
		}
	}

	#ifdef CONFIG_MTD_CONCAT
	if (chips == 1)
		mymtd = my_sub_mtd[0];
	else
		{
			//printk("%s, %d\n, size=0x%x\n", __FUNCTION__, __LINE__, my_sub_mtd[0]->size);
			mymtd = mtd_concat_create(&my_sub_mtd[0], chips,"flash_concat");
			//printk("%s, %d, size=0x%x\n", __FUNCTION__, __LINE__, (mymtd->size));
		}

	if (!mymtd) {
		printk("Cannot create flash concat device\n");
		return -ENXIO;
	}
	#endif

#ifdef SIZE_REMAINING
#ifdef CONFIG_MTD_CONCAT
		last_partition = &rtl8196_parts1[ ARRAY_SIZE(rtl8196_parts1) - 1 ];
		if( last_partition->size == SIZE_REMAINING ) {
			if( last_partition->offset > mymtd->size ) {
				printk( "Warning: partition offset larger than mtd size\n" );
			}else{
				last_partition->size = mymtd->size - last_partition->offset;
			}
#ifdef DEBUG_MAP
			printk(KERN_NOTICE "last_partition size: 0x%x\n",last_partition->size );
#endif
		}
#else
		//for (i=0;i<chips;i++)
		{
				last_partition = &rtl8196_parts1[ ARRAY_SIZE(rtl8196_parts1) - 1 ];
				if( last_partition->size == SIZE_REMAINING ) {
					if( last_partition->offset > my_sub_mtd[0]->size ) {
						printk( "Warning: partition offset larger than mtd size\n" );
					}else{
						last_partition->size = my_sub_mtd[0]->size - last_partition->offset;
					}
#ifdef DEBUG_MAP
					printk(KERN_NOTICE "last_partition size: 0x%x\n",last_partition->size );
#endif
				}
				last_partition = &rtl8196_parts2[ ARRAY_SIZE(rtl8196_parts2) - 1 ];
				if( last_partition->size == SIZE_REMAINING ) {
					if( last_partition->offset > my_sub_mtd[1]->size ) {
						printk( "Warning: partition offset larger than mtd size\n" );
					}else{
						last_partition->size = my_sub_mtd[1]->size - last_partition->offset;
					}
		#ifdef DEBUG_MAP
					printk(KERN_NOTICE "last_partition size: 0x%x\n",last_partition->size );
		#endif
				}
		}
#endif
#endif


	#ifdef CONFIG_MTD_CONCAT
	add_mtd_partitions(mymtd, rtl8196_parts1, ARRAY_SIZE(rtl8196_parts1));
	#ifdef DEBUG_MAP
	printk(KERN_NOTICE "name=%s, size=0x%x\n", mymtd->name, mymtd->size);
	#endif
	#else
	if (my_sub_mtd[0]) {
		#ifdef RTK_DETECT_FLASH_LAYOUT
		detect_flash_map(my_sub_mtd[0],rtl8196_parts1);//mark_flash
		#endif
#ifndef CONFIG_WRT_BARRIER_BREAKER
		add_mtd_partitions(my_sub_mtd[0], rtl8196_parts1,
					ARRAY_SIZE(rtl8196_parts1));
#else
                mtd_device_parse_register(my_sub_mtd[0],NULL,NULL,rtl8196_parts1,ARRAY_SIZE(rtl8196_parts1));
#endif
	#ifdef DEBUG_MAP
	printk(KERN_NOTICE "name=%s, size=0x%x\n", my_sub_mtd[0]->name,
					 my_sub_mtd[0]->size);
	#endif
	}
#ifdef CONFIG_RTL_TWO_SPI_FLASH_ENABLE
	if (my_sub_mtd[1]) {
		add_mtd_partitions(my_sub_mtd[1], rtl8196_parts2,
					ARRAY_SIZE(rtl8196_parts2));
	#ifdef DEBUG_MAP
	printk(KERN_NOTICE "name=%s, size=0x%x\n", my_sub_mtd[1]->name,
					 my_sub_mtd[1]->size);
	#endif
	}
#endif
	#endif
#ifndef CONFIG_MTD_ROOTFS_ROOT_DEV//mark_boot
	ROOT_DEV = MKDEV(MTD_BLOCK_MAJOR, 0);
#endif
	return 0;
}

mod_exit_t cleanup_rtl8196_map(void)
{
	int i;

	if (mymtd) {
#ifndef CONFIG_WRT_BARRIER_BREAKER
		del_mtd_partitions(mymtd);
#else
                mtd_device_unregister(mymtd);
#endif
		map_destroy(mymtd);
	}

	for(i=0; i<MAX_SPI_CS; i++)
	{
		if (my_sub_mtd[i])
		{
#ifndef CONFIG_WRT_BARRIER_BREAKER
			del_mtd_partitions(my_sub_mtd[i]);
#else
                        mtd_device_unregister(my_sub_mtd[i]);
#endif
			map_destroy(my_sub_mtd[i]);
		}
	}
}

MODULE_LICENSE("GPL");
module_init(init_rtl8196_map);
module_exit(cleanup_rtl8196_map);
