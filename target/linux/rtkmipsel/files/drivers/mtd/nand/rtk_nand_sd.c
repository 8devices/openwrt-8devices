/******************************************************************************
 * $Id: rtk_nand_sd5.c,v 1.4 2012/04/02 06:39:30 ccwei0908 Exp $
 * drivers/mtd/nand/rtk_nand.c
 * Overview: Realtek NAND Flash Controller Driver
 * Copyright (c) 2008 Realtek Semiconductor Corp. All Rights Reserved.
 * Modification History:
 *    #000 2008-05-30 CMYu   create file
 *
 *******************************************************************************/
#include <linux/mtd/mtd.h>
#include <linux/mtd/partitions.h>
#include <linux/slab.h>
#include <linux/sysctl.h>
#include <linux/sched.h>
#include <linux/list.h>
#include <linux/pm.h>
#include <asm/io.h>

/* Ken-Yu */
//#include <linux/mtd/rtk_nand_reg.h>  //czyao
#include "bspchip.h"  //czyao
#include "./rtk_nand.h" //czyao
//#include <linux/mtd/rtk_nand.h> //czyao
#include <linux/bitops.h>
#include <mtd/mtd-abi.h>
//#include <asm/r4kcache.h>
#include <linux/jiffies.h>
#include <linux/delay.h>
#include <linux/time.h>
#include <linux/proc_fs.h>
#include <linux/string.h>
#include <asm/cacheflush.h>
#include <linux/version.h>

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,0))
#include <linux/seq_file.h>
#endif



//#include <asm/mach-venus/platform.h> //czyao
//CMYu, 20090720, for CP
//#include <asm/mach-venus/mcp.h> //czyao
#define REG32(reg)   (*(volatile unsigned int *)((unsigned int)reg))
#define BANNER  "Realtek NAND Flash Driver"
#define VERSION  "$Id: rtk_nand_sd5.c,v 1.4 2012/04/02 06:39:30 ccwei0908 Exp $"

#define MTDSIZE	(sizeof (struct mtd_info) + sizeof (struct nand_chip))
#define MAX_PARTITIONS	16
//#define BOOTCODE	8*1024*1024	//16MB
//extern int OOB_DATA;
int OOB_DATA=1;

extern int isLastPage;

/* nand driver low-level functions */
static void rtk_nand_read_id(struct mtd_info *mtd, unsigned char id[5], int chip_sel);
static int rtk_read_oob(struct mtd_info *mtd, u16 chipnr, int page, int len, u_char *buf);
static int rtk_read_ecc_page(struct mtd_info *mtd, u16 chipnr, unsigned int page,
			u_char *data, u_char *oob_buf);
static int rtk_write_oob(struct mtd_info *mtd, u16 chipnr, int page, int len, const u_char *buf);
static int rtk_write_ecc_page(struct mtd_info *mtd, u16 chipnr, unsigned int page,
			const u_char *data, const u_char *oob_buf, int isBBT);
static int rtk_erase_block(struct mtd_info *mtd, u16 chipnr, int page);
//static void rtk_nand_suspend (struct mtd_info *mtd);
//static void rtk_nand_resume (struct mtd_info *mtd);
/* Global Variables */
struct mtd_info *rtk_mtd;

#include <linux/spinlock.h>
spinlock_t lock_nand;

static int page_size, oob_size, ppb;
#ifdef CONFIG_RTK_REMAP_BBT
static int RBA_PERCENT = 5;
#endif
#ifdef CONFIG_RTK_NORMAL_BBT
static int BBT_PERCENT = 5;
#endif



//static unsigned int u32AddressBefore = 0;  //czyao
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,0)
static int rtk_read_proc_nandinfo(struct seq_file *s, void *v);

int rtk_nand_read_open(struct inode *inode, struct file *file)
{
	return(single_open(file, rtk_read_proc_nandinfo, NULL));
}


static const struct file_operations rtk_nand_flash_ops = {
	.owner = THIS_MODULE,
	.open			= rtk_nand_read_open,
	.read           = seq_read,
    .llseek         = seq_lseek,
    .release        = single_release,
};
#endif


#ifdef CONFIG_WRT_BARRIER_BREAKER
unsigned int HW_SETTING_OFFSET = 0x0;

static struct mtd_partition rtl8196_parts1[] = {
      {name: "boot", offset: 0, size:0x500000,},
      {name: "setting", offset: 0x500000, size:0x300000,},
      //{name: "ubifs", offset: 0x100000,	   size:0x200000,},
      {name: "linux", offset: 0x800000,    size:0x600000,}, //0x130000+ rootfs
      {name: "rootfs", offset: 0xe00000, size:0x2800000,},
      {name: "rootfs2", offset: 0x3600000,	   size:0x2800000,},
#ifdef CONFIG_RTK_BOOTINFO_DUALIMAGE
      //{name: "boot_backup", offset: 0x400000, size:0x30000,},
      {name: "linux_backup", offset: (0x400000+0x30000),    size:0x130000,}, //0x130000+ rootfs
#endif
};

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

#define RTK_USERDEFINE_FLASH_BANK_SIZE 0
static unsigned int  RTK_LINUX_PART_OFFSET=0x100000;
static unsigned int  RTK_FLASH_BANK_SIZE=RTK_USERDEFINE_FLASH_BANK_SIZE;
#define SQUASHFS_MAGIC_LE 0x68737173

void detect_nand_flash_map(struct mtd_info *mtd, struct mtd_partition *rtl819x_parts)
{
   int offset =RTK_LINUX_PART_OFFSET;
   struct squashfs_super_block *sb;
   uint32_t mtd_size;
   uint32_t active_bank_offset=0;

#ifdef CONFIG_RTK_BOOTINFO_DUALIMAGE
    int bootbank=0;
    uint32_t back_bank_offset = 0;
    BOOTINFO_P bootinfo_p;
    bootinfo_p = 	&bootinfo_ram;
#endif

	struct nand_chip *this = (struct nand_chip *) mtd->priv;
	int oob_size,page_size,ppb;
	unsigned int page, block;

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

	page_size = mtd->writesize;
	oob_size = mtd->oobsize;
	ppb = mtd->erasesize/mtd->writesize;

	unsigned char buf[oob_size] __attribute__((__aligned__(64)));
	unsigned char data_buf[page_size]; //fixme!

	rtk_init_flash_proc();
	while ((offset + page_size) < mtd_size) {
		page = ((int) offset) >> this->page_shift;
		block = page/ppb;

		if ( this->read_ecc_page (mtd, 0, page, data_buf, buf) ){
			printk ("%s: read_oob page=%d failed\n", __FUNCTION__, page);
			return 1;
		}

		if (*((__u32 *)data_buf) == SQUASHFS_MAGIC_LE) {

			sb = (struct squashfs_super_block *)data_buf;
			rtl819x_parts[3].size =  rtl819x_parts[3].size + rtl819x_parts[3].offset - offset;
		    rtl819x_parts[3].offset = offset;
			break;
		}
		offset += page_size;
	}
#ifdef CONFIG_RTK_BOOTINFO_DUALIMAGE
	//linux partioin
	rtl819x_parts[3].offset = back_bank_offset + RTK_LINUX_PART_OFFSET;
    rtl819x_parts[3].size =  (mtd_size+back_bank_offset) - rtl819x_parts[3].offset;

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

/*
 * RTK NAND suspend:
 */
 #if 0
static void rtk_nand_suspend (struct mtd_info *mtd)
{
	if ( rtk_readl(REG_DMA_CTL3) & 0x02 ){
		while ( rtk_readl(REG_DMA_CTL3) & 0x01 );
		while ( rtk_readl(REG_AUTO_TRIG) & 0x80 );
		udelay(20+60);
		while ( rtk_readl(REG_DMA_CTL3) & 0x01 );
		while ( rtk_readl(REG_AUTO_TRIG) & 0x80 );
	}else{
		while ( rtk_readl(REG_DMA_CTL3) & 0x01 );
		while ( rtk_readl(REG_AUTO_TRIG) & 0x80 );
	}
}


static void resume_to_reset_reg(void)
{
	rtk_writel( 0x103, 0xb800036c );
	rtk_writel(rtk_readl(0xb800000c)& (~0x00800000), 0xb800000c);
	rtk_writel( 0x02, 0xb8000034 );
	rtk_writel(rtk_readl(0xb800000c)| (0x00800000), 0xb800000c);

	rtk_writel( ~(0x1 << 0), REG_CHIP_EN );
	rtk_writel( CMD_RESET, REG_CMD1 );
	rtk_writel( (0x80 | 0x00), REG_CTL );
	while( rtk_readl(REG_CTL) & 0x80 );
	while((rtk_readl(REG_CTL)&0x40)==0);//add by alexchang 0317-2010

	rtk_writel(0x00, REG_MULTICHNL_MODE);

	rtk_writel( 0x00, REG_T1 );
	rtk_writel( 0x00, REG_T2 );
	rtk_writel( 0x00, REG_T3 );
	rtk_writel( 0x09, REG_DELAY_CNT );

	rtk_writel( 0x00, REG_TABLE_CTL);

}


static void rtk_nand_resume (struct mtd_info *mtd)
{
	resume_to_reset_reg();
	if ( rtk_readl(REG_DMA_CTL3) & 0x02 ){
		while ( rtk_readl(REG_DMA_CTL3) & 0x01 );
		while ( rtk_readl(REG_AUTO_TRIG) & 0x80 );
		udelay(20+60);
		while ( rtk_readl(REG_DMA_CTL3) & 0x01 );
		while ( rtk_readl(REG_AUTO_TRIG) & 0x80 );
	}else{
		while ( rtk_readl(REG_DMA_CTL3) & 0x01 );
		while ( rtk_readl(REG_AUTO_TRIG) & 0x80 );
	}
}


static void read_oob_from_PP(struct mtd_info *mtd, __u8 *r_oobbuf, int section) //czyao moves it out

static void write_oob_to_TableSram(__u8 *w_oobbuf, __u8 w_oob_1stB, int shift)
{
	unsigned int reg_oob, reg_num;
	int i;

	if (shift){
		reg_num = REG_BASE_ADDR;
		reg_oob = w_oob_1stB | (w_oobbuf[0] << 8) | (w_oobbuf[1] << 16) | (w_oobbuf[2] << 24);
		rtk_writel(reg_oob, reg_num);

		reg_num = REG_BASE_ADDR + 1*4;
		reg_oob = w_oobbuf[3];
		rtk_writel(reg_oob, reg_num);

		reg_num = REG_BASE_ADDR + 4*4;
		reg_oob = w_oobbuf[4] | (w_oobbuf[5] << 8) | (w_oobbuf[6] << 16) | (w_oobbuf[7] << 24);
		rtk_writel(reg_oob, reg_num);

		reg_num = REG_BASE_ADDR + 8*4;
		reg_oob = w_oobbuf[8] | (w_oobbuf[9] << 8) | (w_oobbuf[10] << 16) | (w_oobbuf[11] << 24);
		rtk_writel(reg_oob, reg_num);

		reg_num = REG_BASE_ADDR + 12*4;
		reg_oob = w_oobbuf[12] | (w_oobbuf[13] << 8) | (w_oobbuf[14] << 16) | (w_oobbuf[15] << 24);
		rtk_writel(reg_oob, reg_num);
	}else{
		for ( i=0; i < (oob_size/4); i++){
			reg_num = REG_BASE_ADDR + i*4;
			reg_oob = w_oobbuf[i*4+0] | (w_oobbuf[i*4+1] << 8) | (w_oobbuf[i*4+2] << 16) | (w_oobbuf[i*4+3] << 24);
			rtk_writel(reg_oob, reg_num);
		}
	}
}


static void rtk_nand_read_id(struct mtd_info *mtd, u_char id[5])  //czyao move it

static int rtk_erase_block(struct mtd_info *mtd, u16 chipnr, int page) //czyao move it

static int rtk_read_oob (struct mtd_info *mtd, u16 chipnr, int page, int len, u_char *oob_buf) //czyao move it

static int rtk_check_pageData(struct mtd_info *mtd, u16 chipnr, unsigned int page, int isLastSector )

static int rtk_read_ecc_page (struct mtd_info *mtd, u16 chipnr, unsigned int page,
			u_char *data_buf, u_char *oob_buf)  //czyao move it


static int rtk_write_oob(struct mtd_info *mtd, u16 chipnr, int page, int len, const u_char *oob_buf)  //czyao moves it

int rtk_write_ecc_page (struct mtd_info *mtd, u16 chipnr, unsigned int page,
			const u_char *data_buf, const u_char *oob_buf, int isBBT)	//czyao moves it

#endif

static void check_ready()
{
	while(1) {
		if(  ( rtk_readl(NACR) & 0x80000000) == 0x80000000 )
			break;
	}
}


//czyao changs it
static void rtk_nand_read_id(struct mtd_info *mtd, u_char id[5], int chip_sel)
{
	int id_chain;

	unsigned int flags_nand = 0;
	spin_lock_irqsave(&lock_nand, flags_nand);
	if(chip_sel>1)  //SD5 only supports chip0, chip1
	{
		spin_unlock_irqrestore(&lock_nand, flags_nand);
		return;
	}
#ifdef CONFIG_RTL_8325D_SUPPORT
	#define PINMUX_REG_8196D_97D   0xb8000040
	REG32(PINMUX_REG_8196D_97D) = (REG32(PINMUX_REG_8196D_97D) & ~((7<<0) | (3<<3) | (3<<8) | (3<<12)))
			| ((0<<0) | (1<<3) | (1<<8) | (1<<12));
#elif !defined(CONFIG_RTL_8198C)
	#define PINMUX_REG_8196D_97D   0xb8000040
	REG32 (PINMUX_REG_8196D_97D) &= 0xFFFFCCE9;
	REG32 (PINMUX_REG_8196D_97D) = 0x1508;
#endif
	check_ready();
	rtk_writel( (rtk_readl(NACR) |ECC_enable|RBO|WBO), NACR);

	//rtk_writel(0x0, NACMR);
	rtk_writel( (CECS0|CMD_READ_ID) , NACMR);          //read ID command
	check_ready();

	rtk_writel( (0x0 |AD2EN|AD1EN|AD0EN) , NAADR);  //dummy address cycle
	check_ready();

	id_chain = rtk_readl(NADR);
	id[0] = id_chain & 0xff;
	id[1] = (id_chain >> 8) & 0xff;
	id[2] = (id_chain >> 16) & 0xff;
	id[3] = (id_chain >> 24) & 0xff;

	id_chain = rtk_readl(NADR);
	id[4] = id_chain & 0xff;

	rtk_writel( 0x0, NACMR);
	rtk_writel( 0x0, NAADR);
	//rtk_writel( (ECC_enable & (~RBO) & (~WBO)), NACR );

	spin_unlock_irqrestore(&lock_nand, flags_nand);

}


#if 0

static void read_oob_867x (struct mtd_info *mtd, __u8 *r_oobbuf, int section, int page) {

	unsigned int reg_oob;
	unsigned oobsize512 = 16;   //512 byte data + 16 bytes OOB
	int count, cycle[5];
	unsigned long flash_addr1, flash_addr2;

	cycle[0] = 0;
	cycle[1] = (0x200) + (0x210)*section;
	cycle[2] = page & 0xff;
	cycle[3] = (page>>8) & 0xff;
	cycle[4] = (page>>16) & 0xff;

	flash_addr1 = (cycle[2] <<16) |cycle[1] ;
	flash_addr2 = (cycle[4] <<4) |cycle[3] ;

	//if(OOB_DATA)
		//printk("page=%d,addr1=%08X, addr2=%08X\n",page,flash_addr1,flash_addr2);

	check_ready();
	rtk_writel( (rtk_readl(NACR) |ECC_enable|RBO|WBO), NACR);

	rtk_writel(0x0, NACMR);
	rtk_writel( (CECS0|CMD_PG_READ_C1) , NACMR);          //read C1 command
	check_ready();

	rtk_writel( 0x0, NAADR);
	rtk_writel( (enNextAD|AD2EN|AD1EN|AD0EN|flash_addr1) , NAADR);  //dummy address cycle
	rtk_writel( ((~enNextAD)& AD1EN|AD0EN|flash_addr2) , NAADR);
	check_ready();

	rtk_writel( (CECS0|CMD_PG_READ_C2) , NACMR);          //read C2 command
	check_ready();

	for(count=0; count<oobsize512;){

		reg_oob = rtk_readl(NADR);

		//if(OOB_DATA)
			//printk("reg_oob=%x\n",reg_oob);

		r_oobbuf[16*section+count+0] = reg_oob & 0xff;
		r_oobbuf[16*section+count+1] = (reg_oob>>8) & 0xff;
		r_oobbuf[16*section+count+2] = (reg_oob>>16) & 0xff;
		r_oobbuf[16*section+count+3] = (reg_oob>>24) & 0xff;

		count +=4;
	}



}


static void write_oob_867x(__u8 *w_oobbuf, __u8 w_oob_1stB,int section,int page)
{
	unsigned int reg_oob;
	unsigned oobsize512 = 16;   //512 byte data + 16 bytes OOB
	int count, cycle[5];
	unsigned long flash_addr1, flash_addr2;

	cycle[0] = 0;
	cycle[1] = (0x200) + (0x210)*section;
	cycle[2] = page & 0xff;
	cycle[3] = (page>>8) & 0xff;
	cycle[4] = (page>>16) & 0xff;

	flash_addr1 = (cycle[2] <<16) |cycle[1] ;
	flash_addr2 = (cycle[4] <<4) |cycle[3] ;

	check_ready();
	rtk_writel( (rtk_readl(NACR) |ECC_enable|RBO|WBO), NACR);

	rtk_writel(0x0, NACMR);
	rtk_writel( (CECS0|CMD_PG_WRITE_C1) , NACMR);          //write C1 command
	check_ready();

	for(count=0;count<oobsize512){



	}

}
#endif


static int rtk_erase_block(struct mtd_info *mtd, u16 chipnr, int page)
{
	page_size = mtd->writesize;
	oob_size = mtd->oobsize;
	ppb = mtd->erasesize/mtd->writesize;
	int addr_cycle[5], page_shift;

	unsigned int flags = 0;
	spin_lock_irqsave(&lock_nand, flags);
	struct nand_chip *this = (struct nand_chip *) mtd->priv;

	if ( page & (ppb-1) ){
		printk("%s: page %d is not block alignment !!\n", __FUNCTION__, page);
		spin_unlock_irqrestore(&lock_nand, flags);
		return -1;
	}

	//debug cl
    if((page < 128)||page > (this->chipsize/page_size - 1))
	{
		printk("[%s:] %d,page num = %d fatal error, try to wirte boot page\n",__func__,__LINE__,page);
		spin_unlock_irqrestore(&lock_nand, flags);
		//while(1);
		return -ERESTARTSYS;
	}

	//printk("Start erase page num:%d\n",page);
	check_ready();

	rtk_writel( (rtk_readl(NACR) |ECC_enable), NACR);
	rtk_writel((NWER|NRER|NDRS|NDWS), NASR);
	rtk_writel(0x0, NACMR);

	rtk_writel((CECS0|CMD_BLK_ERASE_C1),NACMR);
	check_ready();

	addr_cycle[0] = addr_cycle[1] =0;
	for(page_shift=0; page_shift<3; page_shift++){
		addr_cycle[page_shift+2] = (page>>(8*page_shift)) & 0xff;
	}

	//rtk_writel( (enNextAD|AD2EN|AD1EN|AD0EN|(addr_cycle[2]<<CE_ADDR2)),NAADR);
	rtk_writel( ((~enNextAD) & AD2EN|AD1EN|AD0EN|
			(addr_cycle[2]<<CE_ADDR0) |(addr_cycle[3]<<CE_ADDR1)|(addr_cycle[4]<<CE_ADDR2)),NAADR);
	check_ready();

	rtk_writel((CECS0|CMD_BLK_ERASE_C2),NACMR);
	check_ready();

	rtk_writel((CECS0|CMD_BLK_ERASE_C3),NACMR);
	check_ready();

	if ( rtk_readl(NADR) & 0x01 ){
		spin_unlock_irqrestore(&lock_nand, flags);
		printk("[%s] erasure is not completed at block %d\n", __FUNCTION__, page/ppb);
/*
#if defined(CONFIG_RTK_REMAP_BBT)
		if ( chipnr == 0 && page >= 0 && page < ((CONFIG_BOOT_SIZE - (2*BACKUP_BBT*this->block_size))/page_size) )
#else
		if ( chipnr == 0 && page >= 0 && page < (CONFIG_BOOT_SIZE/page_size))
#endif
			return 0;
		else
*/
			return -1;
	}
//	printk("E:%u\n",page/ppb);

	unsigned int chip_section = (chipnr * this->page_num) >> 5;
	unsigned int section = page >> 5;
	//printk("%s:chip_section %d section %d page/ppb:%u this->page_num %d\n",__FUNCTION__,chip_section,section,page/ppb, this->page_num);

	memset ( (__u32 *)(this->erase_page_flag)+ chip_section + section, 0xff, ppb>>3);
	spin_unlock_irqrestore(&lock_nand, flags);

	return 0;
}
//   120301

int rtk_PIO_read_basic(int page, int offset)
{
	int i;
	unsigned int flash_addr1=0;
	unsigned int data_out;
	int rlen, real_page;
	unsigned int cmd;
	real_page = page;

/*
	if(offset==0 || offset==256){
		rlen = page_size/2;

		if(offset==0)
			cmd = CMD_PG_READ_A;
		else
			cmd = CMD_PG_READ_B;

	}else{
		rlen = oob_size;
		cmd = CMD_PG_READ_C;
	}
*/
	rlen = page_size + oob_size;
	cmd = CMD_PG_READ_A;

	rtk_writel(0xc0077777, NACR);

	/* Command cycle 1*/
	rtk_writel((CECS0|cmd), NACMR);

	check_ready();

	flash_addr1 |= ((real_page & 0xffffff) << 8) ;

	/* Give address */
	rtk_writel( (AD2EN|AD1EN|AD0EN|flash_addr1), NAADR);

	check_ready();

	for(i=0; i<(rlen/4); i++){
		data_out = rtk_readl(NADR);
		//printf("[%3d] 0x%08X \n",i, data_out);
		if(data_out!=0xffffffff){
			printk("[%3d] 0x%08X \n",i, data_out);
			printk("%s: page %d offset %d i %d rlen %d\n",__FUNCTION__, page, offset, i, rlen);
			return -1;
		}
	}

	check_ready();
	rtk_writel(0, NACMR);

	return 0;

}

int rtk_check_allone_512(int page)
{
	int rc=0;

	//printf("[%s] page = %d\n",__func__,page);
	rc = rtk_PIO_read_basic(page,0);
	if(rc < 0)
		goto read_finish;
/*
	rc = rtk_PIO_read_basic(page,256);
	if(rc < 0)
		goto read_finish;

	rc = rtk_PIO_read_basic(page,512);
*/
read_finish:
	return rc;
}

int rtk_check_allone(int page, int offset)
{
	unsigned int flash_addr1, flash_addr2;
	unsigned int data_out;
	int real_page, i, rlen;;

	real_page = page;

	/* rlen is equal to (512 + 16) */
	rlen = 528;

	rtk_writel(0xc00fffff, NACR);

	/* Command cycle 1*/
	rtk_writel((CECS0|CMD_PG_READ_C1), NACMR);

	check_ready();

	flash_addr1 =  ((real_page & 0xff) << 16) | offset;
	flash_addr2 = (real_page >> 8) & 0xffffff;

	/* Give address */
	rtk_writel( (enNextAD|AD2EN|AD1EN|AD0EN|flash_addr1), NAADR);
	rtk_writel( (AD1EN|AD0EN|flash_addr2), NAADR);

	/* Command cycle 2*/
	rtk_writel((CECS0|CMD_PG_READ_C2), NACMR);

	check_ready();

	for(i=0; i<(rlen/4); i++){
		data_out = rtk_readl(NADR);
		if( data_out != 0xffffffff)
			return -1;
	}

	check_ready();
	rtk_writel(0, NACMR);

	return 0;

}

static int rtk_check_pageData(struct mtd_info *mtd, u16 chipnr,int page, int offset, int isLastSector)
{
	struct nand_chip *this = (struct nand_chip *) mtd->priv;
	unsigned int chip_section = (chipnr * this->page_num) >> 5;
	unsigned int section = page >> 5;
	unsigned int index = page & (32-1);

	int rc = 0;
	int error_count,status;

	status = rtk_readl(NASR);
//printk("[%s:%d] status=%x page %x offset %x idx %x\n",__FUNCTION__,__LINE__,status, page,offset, index);

	if( (status & NDRS)== NDRS){

		if (this->erase_page_flag[chip_section+section] & (1<< index) ){
			//printk("[%s:%d]RRR page=%u status=%x\n",__FUNCTION__,__LINE__,page, status);
			;
		}else if( status & NRER) {
			error_count = (status & 0xf0) >> 4;
			if(error_count <=4 && error_count > 0 ) {
				printk("[%s] R: Correctable HW ECC Error at page=%u, status=0x%08X\n\r", __FUNCTION__, page,status);
				status &= 0x0f; //clear NECN
				rtk_writel(status, NASR);
				return 0;
			}else{

			    if(!NAND_ADDR_CYCLE){
			        if( rtk_check_allone(page,offset) == 0 ){
				        //printf("[%s] Page %d is all one page, bypass it !!\n\r",__func__,page);
						status &= 0x0f; //clear NECN
						rtk_writel(status, NASR);
				        return 0;
			        }
			    }else{
				    if( rtk_check_allone_512(page) == 0 ){
					    //printk("[%s] Page %d is all one page, bypass it !!\n\r",__func__,page);
						status &= 0x0f; //clear NECN
					    rtk_writel(status, NASR);
					    return 0;
				    }
			    }
				printk("[%s] R: Un-Correctable HW ECC Error at page=%u, status=0x%08X\n\r", __FUNCTION__, page,status);
				status &= 0x0f; //clear NECN
				rtk_writel(status, NASR);
				return -1;
			}
		}

	}
	else if( (status & NDWS)== NDWS){

			if (this->erase_page_flag[chip_section+section] & (1<< index) ){
			    //printk("[%s] AAAA NAND Flash write at page=%u, status=0x%08X\n\r", __FUNCTION__, page,status);
			    //printk("[%s] this->erase_page_flag[%d]:0x%x, %x:%x\n\r", __FUNCTION__,chip_section+section,this->erase_page_flag[chip_section+section],index, 1 << index);
			;
			}else if( status & NWER) {
			    printk("[%s] A NAND Flash write failed at page=%u, status=0x%08X\n\r", __FUNCTION__, page,status);
			    rtk_writel(status, NASR);
			    return -1;
		    }
			//printk("[%s] B NAND Flash write at page=%u, status=0x%08X\n\r", __FUNCTION__, page,status);

	}

	rtk_writel(status, NASR);
//printk("[%s:%d] status=%x page %u offset %u idx %d\n",__FUNCTION__,__LINE__,status, page,offset, index);

	return rc;
}

//end 120301
#if 0
static int rtk_check_pageData(struct mtd_info *mtd, u16 chipnr, unsigned int page, int isLastSector )
{
	struct nand_chip *this = (struct nand_chip *) mtd->priv;
	unsigned int chip_section = (chipnr * this->page_num) >> 5;
	unsigned int section = page >> 5;
	unsigned int index = page & (32-1);
	int blank_all_one = 0;
	int rc = 0;
	int error_count,status;

#if 0
	blank_all_one = (rtk_readl(REG_BLANK_CHECK)>>1) & 0x01;
	if (blank_all_one)
	{
		if(isLastSector)
			this->erase_page_flag[chip_section+section] =  (1<< index);
		else
			return 0;
	}

	if (rtk_readl(NASR) & 0x0C){
		if (this->erase_page_flag[chip_section+section] & (1<< index) ){
			;
		}else{
			if (rtk_readl(REG_ECC_STATE) & 0x08){
				if ( chipnr == 0 && page >= 0 && page < CONFIG_BOOT_SIZE/page_size )
				{
					return 0;
				}
				else
				{
					printk("[%s] Un-Correctable HW ECC Error at page=%u\n", __FUNCTION__, page);
					return -1;
				}
			}
			//if (rtk_readl(REG_ECC_STATE) & 0x04)		//mark for mlc, alexchang 0412-2010
			//	printk("[%s] Correctable HW ECC Error at page=%u\n", __FUNCTION__, page);
		}
	}
#endif

	status = rtk_readl(NASR);
printk("[%s:%d] status=%x page %u idx %d\n",__FUNCTION__,__LINE__,status, page, index);

	if( (status & NDRS)== NDRS){

		if (this->erase_page_flag[chip_section+section] & (1<< index) ){
			printk("[%s] kkkk: Correctable HW ECC Error at page=%u\n",__FUNCTION__,page);
			;
		}else if( status & NRER) {
			error_count = (status & 0xf0) >> 4;

			if(error_count >4 ) {
				printk("[%s:%d] R: Un-Correctable HW ECC Error at page=%u, status=0x%08X\n", __FUNCTION__,__LINE__, page,status);
                //   120229
                status &= 0x0f; //clear NECN
				rtk_writel(status, NASR);
				//rtk_writel(0, NASR);
				return -1;
			}else{
				printk("[%s:%d] R: Correctable HW ECC Error at page=%u error_count %d\n",__FUNCTION__,__LINE__,page, error_count);
                //   120229
                status &= 0x0f;//clear NECN
				rtk_writel(status, NASR);
				//rtk_writel(0, NASR);
				return 0;
			}
		}

	}else if( (status & NDWS)== NDWS){

		if (this->erase_page_flag[chip_section+section] & (1<< index) ){
			;
//		}else if( status & NRER) {
			}else if( status & NWER) {
/*
			error_count = (status & 0xf0) >> 4;

			if(error_count >4) {
				printk("[%s] W: Un-Correctable HW ECC Error at page=%u, status=0x%08X\n", __FUNCTION__, page,status);
				rtk_writel(status, NASR);
				rtk_writel(0, NASR);
				return -1;
			}else{
*/
				printk("[%s:%d] W: Correctable HW ECC Error at page=%u\n",__FUNCTION__, __LINE__,page);
                status &= 0x0f;//clear NECN
				rtk_writel(status, NASR);
				//rtk_writel(0, NASR);
				return -1;
//			}

		}

	}

	rtk_writel(status, NASR);
	status = rtk_readl(NASR);
	printk("[%s:%d] end status=%x page %u\n",__FUNCTION__,__LINE__,status, page);


	return rc;
}
#endif
unsigned char buf_temp[64*4] __attribute__((__aligned__(64)));
static int rtk_read_oob (struct mtd_info *mtd, u16 chipnr, int page, int len, u_char *oob_buf)
{
	struct nand_chip *this = (struct nand_chip *) mtd->priv;
	int rc = 0;
	int dram_sa, dma_len, oob_sa;
	int blank_all_one=0;
	int dma_counter = page_size >> 9;  //PP=512Bytes
	int dma_number = 4;
	int buf_pos=0;
	int page_num[3], page_shift=0;
	unsigned long flash_addr_t=0;
	int i, index;
	unsigned char *dram_addr;
	//unsigned char buf_temp[oob_size] __attribute__((__aligned__(64)));
	int oob_unit = oob_size/4;
//	printk("%s\r\n",__FUNCTION__);
	unsigned int flags = 0;
	spin_lock_irqsave(&lock_nand, flags);

	if ( len>oob_size || !oob_buf ){
		printk("[%s] error: len>oob_size OR oob_buf is NULL\n", __FUNCTION__);
		spin_unlock_irqrestore(&lock_nand, flags);
		return -1;
	}
	memcpy(buf_temp,oob_buf,oob_size);
	dram_addr = (unsigned char*)kzalloc(sizeof(unsigned char)*(page_size)/4, GFP_KERNEL);
	if( !dram_addr) {
		printk("!!!!!!allocate memory fail!!!!!!!\n");
		spin_unlock_irqrestore(&lock_nand, flags);
		return -1;
	}
	dma_cache_wback_inv((unsigned long)dram_addr,(page_size)/4);
#if 0
	if( oob_buf ) {
		//dma_cache_wback_inv((unsigned long)oob_buf,(oob_size/4));
		dma_cache_wback_inv((unsigned long)oob_buf,(oob_size));
		oob_sa =  (uint32_t)oob_buf & (~M_mask);
	}else {
		//dma_cache_wback_inv((unsigned long)this->g_oobbuf,(oob_size/4));
		dma_cache_wback_inv((unsigned long)this->g_oobbuf,(oob_size));
		oob_sa =  (uint32_t)(this->g_oobbuf) & (~M_mask);
	}
#else
	if( oob_buf ) {
		//dma_cache_wback_inv((unsigned long)oob_buf,(oob_size/4));
		//dma_cache_wback_inv((unsigned long)buf_temp,(oob_size));
		oob_sa =  (uint32_t)buf_temp & (~M_mask);
	}else {
		//dma_cache_wback_inv((unsigned long)this->g_oobbuf,(oob_size/4));
		dma_cache_wback_inv((unsigned long)this->g_oobbuf,(oob_size));
		oob_sa =  (uint32_t)(this->g_oobbuf) & (~M_mask);
	}
#endif

	check_ready();
	wmb();
	 __flush_cache_all();
	rtk_writel( (rtk_readl(NACR) | ECC_enable & (~RBO) & (~WBO)), NACR);

	//set DMA RAM Data start address
	dram_sa = (uint32_t)dram_addr & (~M_mask);
	//printk("[%s, line %d] dram_sa = 0x%08X\n",__FUNCTION__,__LINE__,dram_sa);
	rtk_writel( dram_sa, NADRSAR);

	//set DMA RAM OOB start address
	//printk("[%s, line %d] oob_sa = 0x%08X\n",__FUNCTION__,__LINE__,oob_sa);
	rtk_writel(oob_sa, NADTSAR);

	//set DMA flash start address
	for(page_shift=0;page_shift<3; page_shift++) {
             page_num[page_shift] = ((page>>(8*page_shift)) & 0xff);

	     if(!NAND_ADDR_CYCLE){
		flash_addr_t |= (page_num[page_shift] << (12+8*page_shift));
	     }else{
		flash_addr_t |= (page_num[page_shift] << (9+8*page_shift));
	     }
	}
	//printk("[%s, line %d] flash_addr_t = 0x%08X, page = %d\n",__FUNCTION__,__LINE__,flash_addr_t,page);
	rtk_writel( flash_addr_t, NADFSAR);

#if 0
	for(index=0; index<16;index++){
		printk("%d 0x%08X ", 4*index, rtk_readl(NAND_CTRL_BASE+index*4));
		if((index+1)%4 == 0) printk("\n");
	}
#endif

	//DMA read
	//rtk_writel( (DESC0|DMARE|LBC_128),NADCRR);
	wmb();
	rtk_writel( (DESC0|DMARE|LBC_128& (~TAG_DIS)),NADCRR);

	check_ready();


#if 0 //czyao debug
	if( OOB_DATA ){
		printk("OOB data in line %d: page:%x\n",__LINE__,page);
		for(i=0; i<(oob_size); i++){
			printk("0x%02X ",*(buf_temp+i));
			if((i+1)%16==0) printk("\n");
		}
	}
	printk("\n");
#endif

	if(rtk_check_pageData(mtd,chipnr,page,0,0)==-1)
	{
		spin_unlock_irqrestore(&lock_nand, flags);
		return -1;
	}

	dma_counter--;
	buf_pos++;

	/*=========================Move 512Bytes once============================*/
	while(dma_counter>0){
		//dma_cache_wback_inv((unsigned long)dram_addr,(page_size)/4);
#if 0
		if( oob_buf) {
			//dma_cache_wback_inv((unsigned long)(oob_buf+buf_pos*oob_unit),oob_unit);
			oob_sa = ((uint32_t)oob_buf+buf_pos*oob_unit) & (~M_mask);
		}else {
			//dma_cache_wback_inv((unsigned long)(this->g_oobbuf+buf_pos*oob_unit),oob_unit);
			oob_sa = ((uint32_t)(this->g_oobbuf)+buf_pos*oob_unit) & (~M_mask);
		}
#else
		if( oob_buf) {
			memcpy(buf_temp,((uint32_t)oob_buf+buf_pos*oob_unit),(oob_unit));
		//	printk("t:%x\r\n",buf_temp);
			//dma_cache_wback_inv((unsigned long)buf_temp,oob_unit);
			oob_sa = ((uint32_t)(buf_temp)) & (~M_mask);
		}else {
			//dma_cache_wback_inv((unsigned long)(this->g_oobbuf+buf_pos*oob_unit),oob_unit);
			oob_sa = ((uint32_t)(this->g_oobbuf)+buf_pos*oob_unit) & (~M_mask);
		}

#endif

		int lastSec = dma_counter-1;

		check_ready();
		wmb();
		 __flush_cache_all();
		//set DMA RAM data start address
		//printk("[%s, line %d] dram_sa = 0x%08X\n",__FUNCTION__,__LINE__,dram_sa);
		rtk_writel(dram_sa, NADRSAR);

		//set DMA RAM oob start address
		//printk("[%s, line %d] oob_sa = 0x%08X\n",__FUNCTION__,__LINE__,oob_sa);
		rtk_writel(oob_sa, NADTSAR);

		//set DMA flash start address,add (512+6+10)bytes
		flash_addr_t += 528;
		//printk("[%s, line %d] flash_addr_t = 0x%08X\n",__FUNCTION__,__LINE__,flash_addr_t);
		rtk_writel( flash_addr_t, NADFSAR);

		//DMA read
		//rtk_writel( (DESC0|DMARE|LBC_128),NADCRR);
		wmb();
		 __flush_cache_all();
		rtk_writel( (DESC0|DMARE|LBC_128 & (~TAG_DIS)),NADCRR);
		//printk("[%s, line %d]\n",__FUNCTION__,__LINE__);
		check_ready();

		/*if( oob_buf) {
			memcpy(oob_buf+buf_pos*16,dram_addr+512,8);
		}else {
			memcpy(this->g_oobbuf+buf_pos*16,dram_addr+512,8);
		}*/
		memcpy(((uint32_t)oob_buf+buf_pos*oob_unit),buf_temp,oob_unit);
#if 0//czyao debug
		if( OOB_DATA ){
			printk("OOB data in line %d:\n",__LINE__);



			for(i=0; i<(oob_size/4); i++){
				printk("0x%02X ",*(oob_buf+buf_pos*oob_unit+i));
				if((i+1)%16==0) printk("\n");
			}
		}
#endif
		if(lastSec<=0)
			lastSec =1;
		else
			lastSec =0;

		if(rtk_check_pageData(mtd,chipnr,page,buf_pos*(512+16),lastSec)==-1)
		{
		    spin_unlock_irqrestore(&lock_nand, flags);
			printk("%s: rtk_check_pageData return fail...\n",__FUNCTION__);
			return -1;
		}

		dma_counter--;
		buf_pos++;
	}
 __flush_cache_all();
	kfree(dram_addr);

#if 0
	unsigned int chip_section = (chipnr * this->page_num) >> 5;
	unsigned int section = page >> 5;
	unsigned int index = page & (32-1);
	blank_all_one = (rtk_readl(REG_BLANK_CHECK)>>1) & 0x01;
	if (blank_all_one)
		this->erase_page_flag[chip_section+section] =  (1<< index);

	if (rtk_readl(REG_ECC_STATE) & 0x0C){
		if (this->erase_page_flag[chip_section+section] & (1<< index) ){
			;
		}else{
			if (rtk_readl(REG_ECC_STATE) & 0x08){
				up (&sem);
				if ( chipnr == 0 && page >= 0 && page < CONFIG_BOOT_SIZE/page_size )
					return 0;
				else
					return -1;
			}
			//if (rtk_readl(REG_ECC_STATE) & 0x04)	//mark for mlc, alexchang 0412-2010
			//	printk("[%s] Correctable HW ECC Error at page=%d\n", __FUNCTION__, page);
		}
	}
#endif
	spin_unlock_irqrestore(&lock_nand, flags);

	return rc;
}


#if 0
static int rtk_read_ecc_page (struct mtd_info *mtd, u16 chipnr, unsigned int page,
			u_char *data_buf, u_char *oob_buf)
{

	page_size = mtd->writesize;
	oob_size = mtd->oobsize;
	ppb = mtd->erasesize/mtd->writesize;

	struct nand_chip *this = (struct nand_chip *) mtd->priv;
	int rc = 0;
	int i;
	int dram_sa, oob_sa;
	int blank_all_one = 0;

	int dma_counter = page_size >> 9;	//Move unit=512Byte
	int buf_pos=0;
	int page_len;
	int page_num[3], page_shift=0;
	unsigned long flash_addr_t=0;
	int page_unit = page_size/4;
	int oob_unit = oob_size/4;

	if (down_interruptible (&sem)) {
		printk("%s : user breaking\n",__FUNCTION__);
		return -ERESTARTSYS;
	}
	if ( !oob_buf )
		memset(this->g_oobbuf, 0xff, oob_size);


	/*winfred_wang*/
	dma_cache_wback_inv(((unsigned long) data_buf),page_size);   //czyao

	if ( oob_buf ) {
		/*winfred_wang*/
		dma_cache_wback_inv(((unsigned long) oob_buf),oob_size);   //czyao
		oob_sa = ((uint32_t)oob_buf) & (~M_mask);
	}else {
		dma_cache_wback_inv((unsigned long) this->g_oobbuf,oob_size);   //czyao
		oob_sa = ((uint32_t)this->g_oobbuf) & (~M_mask);
	}

	//---------------------DMA the 1st page ----------------------//
	check_ready();
	wmb();
	//rtk_writel( (rtk_readl(NACR) |ECC_enable|RBO|WBO), NACR);
	rtk_writel( (rtk_readl(NACR) |ECC_enable & (~RBO) & (~WBO)), NACR);

	//set DMA RAM data start address
	dram_sa = ((uint32_t)data_buf) & (~M_mask);
	rtk_writel( dram_sa, NADRSAR);

	//set DMA RAM oob start address
	rtk_writel( oob_sa, NADTSAR);

	//set DMA flash start address
	for(page_shift=0;page_shift<3; page_shift++) {
             page_num[page_shift] = ((page>>(8*page_shift)) & 0xff);

	     if(!NAND_ADDR_CYCLE){
		flash_addr_t |= (page_num[page_shift] << (12+8*page_shift));
	     }else{
		flash_addr_t |= (page_num[page_shift] << (9+8*page_shift));
	     }
	}
	//printk("flash_addr_t = 0x%08X, page = %d\n",flash_addr_t, page);
	rtk_writel( flash_addr_t, NADFSAR);

	//DMA read
	wmb();
	rtk_writel( ((~TAG_DIS)&(DESC0|DMARE|LBC_128)),NADCRR);
	//rtk_writel( (TAG_DIS |DESC0|DMARE|LBC_128),NADCRR);
	//printk("[%s, line %d] dram_sa = 0x%08X \n",__FUNCTION__,__LINE__,dram_sa);
	check_ready();
	//printk("[%s, line %d] \n",__FUNCTION__,__LINE__);

	//-----------------------Read OOB data---------------------//
	// HW DMA OOB data to specific address
	/*if ( oob_buf ){
		memcpy(oob_buf, data_buf+512,8);
	}else{
		memcpy(this->g_oobbuf, data_buf+512,8);
	}*/

#if 0 //czyao debug
	if( OOB_DATA ){
		printk("Page data in line %d:\n",__LINE__);
		for(i=0; i<((page_size+oob_size)/4); i++){
			printk("%02X ",*(data_buf+i));
			if((i+1)%16==0) printk("\n");
		}
	}
#endif


	/*
	rtk_writel(0x00, REG_SRAM_CTL);
	*/

	if(rtk_check_pageData(mtd,chipnr,page,0,0)==-1)
	{
		spin_unlock_irqrestore(&lock_nand, flags);
		return -1;
	}

	dma_counter--;
	buf_pos++;

	/*========================Move 512Bytes once==========================*/
	while(dma_counter>0){

		//dma_cache_wback_inv((unsigned long)(data_buf+buf_pos*page_unit),page_unit);
		if( oob_buf) {
			//dma_cache_wback_inv((unsigned long)(oob_buf+buf_pos*oob_unit),oob_unit);
			oob_sa = ((uint32_t)oob_buf+buf_pos*oob_unit) & (~M_mask);
		}else{
			//dma_cache_wback_inv((unsigned long)(this->g_oobbuf+buf_pos*oob_unit),oob_unit);
			oob_sa = ((uint32_t)this->g_oobbuf+buf_pos*oob_unit) & (~M_mask);
		}

		int lastSec = dma_counter-1;

		//printk("[%s, line %d] \n",__FUNCTION__,__LINE__);
		check_ready();
		wmb();
		//printk("[%s, line %d] \n",__FUNCTION__,__LINE__);
		//rtk_writel( (rtk_readl(NACR) |ECC_enable|RBO|WBO), NACR);
		rtk_writel( (rtk_readl(NACR) |ECC_enable & (~RBO) & (~WBO)), NACR);

		//set DMA RAM start address, add 512 bytes
		dram_sa = ( (uint32_t)(data_buf+buf_pos*page_unit) & (~M_mask));
		rtk_writel(dram_sa, NADRSAR);

		//set DMA RAM oob start address
		rtk_writel( oob_sa, NADTSAR);

		//set DMA flash start address,add (512+6+10)bytes
		flash_addr_t += 528;
		rtk_writel( flash_addr_t, NADFSAR);

		//DMA read
		wmb();
		rtk_writel( ((~TAG_DIS)&(DESC0|DMARE|LBC_128)),NADCRR);
		//rtk_writel( (TAG_DIS|DESC0|DMARE|LBC_128),NADCRR);
		//printk("[%s, line %d] dram_sa = 0x%08X \n",__FUNCTION__,__LINE__,dram_sa);
		check_ready();

		//---------------------Read OOB data--------------------//
		// HW DMA OOB to specific address
		/*if ( oob_buf ){
			memcpy((oob_buf+buf_pos*16),data_buf+(buf_pos+1)*512,8);
		}else{
			memcpy((this->g_oobbuf+buf_pos*16),data_buf+(buf_pos+1)*512,8);
		}*/

		#if 0 //czyao debug
			if( OOB_DATA ){
				printk("Page data in line %d:\n",__LINE__);
				for(i=0; i<((page_size+oob_size)/4); i++){
					printk("%02X ",*(data_buf+512*buf_pos+i));
					if((i+1)%16==0) printk("\n");
				}
			}
		#endif


		/*
		rtk_writel(0x00, REG_SRAM_CTL); */
		if(lastSec<=0)
			lastSec =1;
		else
			lastSec =0;

		if(rtk_check_pageData(mtd,chipnr,page,buf_pos*(512+16),lastSec)==-1)
		{
		    spin_unlock_irqrestore(&lock_nand, flags);
			printk("rtk_check_pageData return fail...\n");
			return -1;
		}

		dma_counter--;
		buf_pos++;
	}

	/*winfred_wang*/
	dma_cache_wback_inv(((unsigned long) data_buf),page_size);   //czyao

	if ( oob_buf ) {
		/*winfred_wang*/
		dma_cache_wback_inv(((unsigned long) oob_buf),oob_size);   //czyao
	}else {
		dma_cache_wback_inv((unsigned long) this->g_oobbuf,oob_size);   //czyao
	}

#ifdef CONFIG_REALTEK_MCP
	if ( this->mcp==MCP_AES_ECB ){
		MCP_AES_ECB_Decryption(NULL, data_buf, data_buf, page_size);
	}else{
		;
	}
#endif

	spin_unlock_irqrestore(&lock_nand, flags);

//	printk("\t R %u:%u\n",page/ppb, page%ppb);

	return rc;
}
#endif

int rtk_read_ecc_page(struct mtd_info *mtd, u16 chipnr, unsigned int page,
			u_char *data_buf, u_char *oob_buf)
{
	//int real_page = chunk_id*nandflash_info.page_per_chunk; //get real_page number
	struct nand_chip *this = (struct nand_chip *) mtd->priv;
	int real_page = page;

	int dma_counter = 4; //always 512byte * 4
	int dram_sa, oob_sa;
	int page_shift;
	int page_num[3];
	unsigned long flash_addr_t=0;
	unsigned long flash_addr2_t=0;
	int buf_pos=0;
	int return_value=0;
	int ecc_count;
	int i;
	//int block = real_page/nandflash_info.num_page_per_block;

	//printk("real_page = %d,ppb=%d\n",real_page,ppb);
	ppb = mtd->erasesize/mtd->writesize;
	int block = real_page/ppb;
	uint8_t tempvalue;
	uint8_t *oob_area;
	uint8_t temp_buf;
	uint8_t *data_area=NULL;
	uint8_t data_area0[512+16+CACHELINE_SIZE+CACHELINE_SIZE-4]; //data,oob point must 32 cache aligment
	unsigned long flags;

	unsigned int flags_nand = 0;
	spin_lock_irqsave(&lock_nand, flags_nand);

	if((data_buf == NULL)||(oob_buf == NULL)){
		printk("data_buf/oob_buf point is null\n");
		spin_unlock_irqrestore(&lock_nand, flags_nand);
		return -1;
	}

	memset(data_area0, 0xff, 512+16+CACHELINE_SIZE+CACHELINE_SIZE-4);
	data_area = (uint8_t*) ((uint32_t)(data_area0 + CACHELINE_SIZE-4) & 0xFFFFFFF0);
	oob_area=(uint8_t*) data_area+512;

	oob_sa = ((uint32_t)oob_area) & (~M_mask);
	dram_sa = ((uint32_t)data_area) & (~M_mask);

	rtk_writel(0xC00FFFFF, NACR);     //Enable ECC
	rtk_writel(0x0000000F, NASR);	  //clear NAND flash status register


	for(page_shift=0;page_shift<3; page_shift++) {
		page_num[page_shift] = ((real_page>>(8*page_shift)) & 0xff);
		{
			flash_addr_t |= (page_num[page_shift] << (12+8*page_shift));
		}
	}


#if 0
	for(page_shift=0;page_shift<3; page_shift++) {
		page_num[page_shift] = ((real_page>>(8*page_shift)) & 0xff);
		if(nandflash_info.page_size==2048){
			flash_addr_t |= (page_num[page_shift] << (12+8*page_shift));
	  }else if(nandflash_info.page_size==4096){
		  flash_addr_t |= (page_num[page_shift] << (13+8*page_shift));
	  }else if(nandflash_info.page_size==512){ //512byte
			flash_addr_t |= (page_num[page_shift] << (9+8*page_shift));
		}
	}
#endif

  // If the nand flash size is larger than 2G bytes, fill the other address for DMA
  flash_addr2_t= (real_page >> 20);

  /* DMA start read */

	while(dma_counter>0){

		int lastSec = dma_counter-1;

		//set DMA RAM DATA start address
		rtk_writel(dram_sa, NADRSAR);
		//set DMA RAM oob start address , always use oob_sa buffer
		rtk_writel(oob_sa, NADTSAR);

		//set DMA flash start address,
		rtk_writel( flash_addr_t, NADFSAR); //write flash address

		/*winfred_wang*/
#if 0 //BCH
		rtk_writel( flash_addr2_t, NADFSAR2);
#endif

		dma_cache_wback_inv((uint32_t *)data_area,528);
		//local_irq_save(flags);
		//DMA read command
	    rtk_writel( ((~TAG_DIS)&(DESC0|DMARE|LBC_64)),NADCRR);
		check_ready();
		//check status register
		//local_irq_restore(flags);

		if(lastSec<=0)
			lastSec =1;
		else
			lastSec =0;

		if(rtk_check_pageData(mtd,chipnr,page,buf_pos*(512+16),lastSec)==-1)
		{
		    spin_unlock_irqrestore(&lock_nand, flags_nand);
			printk("rtk_check_pageData return fail...\n");
			return -1;
		}

		#if 0
		ecc_count=nand_check_eccStatus();
		if((return_value!=-1) && (ecc_count != -1))
		{
			return_value= (ecc_count>return_value)?ecc_count:return_value;
		}else{
			return_value=-1;
		}
		#endif


		//copy data
		memcpy(data_buf+(buf_pos*512), data_area, 512);
		memcpy(oob_buf+(buf_pos*6), oob_area, 6);
		//copy oob

		flash_addr_t += 528;//move next flash addr add (512+6+10)bytes
		dma_counter--;
		buf_pos++;

	}


	#ifdef SWAP_2K_DATA
	unsigned int read_bbi;
	unsigned char switch_bbi = 0;
	if(block > 0){
		if(isLastPage){
			read_bbi = page & (ppb-1);
			if((read_bbi == (ppb-2)) || (read_bbi ==(ppb-1))){
				switch_bbi = 1;
			}
		}else{
			read_bbi = page & (ppb-1);
			if((read_bbi == 0) || (read_bbi ==1)){
				switch_bbi = 1;
			}
		}

		/*test code*/
		/*because image convert app cannot decise which is the first page of block,so need do switch in every page*/
		switch_bbi = 1;

		if(!NAND_ADDR_CYCLE)
		{
			/*switch bad block info*/
			unsigned char temp_val=0;
			if(switch_bbi){
				temp_val = data_buf[DATA_BBI_OFF];
				#if 0
				if(oob_buf){
					data_buf[DATA_BBI_OFF] = oob_buf[OOB_BBI_OFF];
					oob_buf[OOB_BBI_OFF] = temp_val;
				}
				else
				#endif
				{
					data_buf[DATA_BBI_OFF] = oob_buf[OOB_BBI_OFF];
					oob_buf[OOB_BBI_OFF] = temp_val;
				}
			}
		}
	}
	#endif


	/*winfred_wang need*/
	#if 0
	if(block>0){
		//change bbi,data
		temp_buf=chunk_buf[nandflash_info.bbi_dma_offset];
		chunk_buf[nandflash_info.bbi_dma_offset] = spare->u8_oob[nandflash_info.bbi_swap_offset];
		spare->u8_oob[nandflash_info.bbi_swap_offset]=temp_buf;
	}
	#endif
//eric mark, not need flush cache
//	dma_cache_wback_inv((u32_t *)chunk_buf,2048);
//	dma_cache_wback_inv((u32_t *)spare->u8_oob,64);


#if 0	 //only for test chip
	  if(return_value==-1){ //ecc fail ,test chip must check data is all 0xff
			return_value=0;
			for(i=0; i<(24); i++){
				if( spare->u8_oob[i] != 0xff){
					printk("ecc fail in _nand_read_chunk\n");
		//		  	if(data_area0!=NULL)
			//			kfree(data_area0);
					return_value= -1;
				  }
			}
	  }
#endif

//	if(data_area0!=NULL)
	//	kfree(data_area0);


    spin_unlock_irqrestore(&lock_nand, flags_nand);
	return return_value;

}



static int rtk_write_oob(struct mtd_info *mtd, u16 chipnr, int page, int len, const u_char *oob_buf)
{
	struct nand_chip *this = (struct nand_chip *) mtd->priv;
	unsigned int page_len, dram_sa, dma_len, oob_sa;
	unsigned char oob_1stB;
	int rc = 0;
	int page_num[3], page_shift=0;
	unsigned long flash_addr_t=0;
	unsigned int dma_counter = page_size>>9, buf_pos=0;
	int oob_unit = oob_size/4;

	printk("%s : page %x len %x dma_counter %x\n",__FUNCTION__, page, len, dma_counter);

	unsigned int flags = 0;
	spin_lock_irqsave(&lock_nand, flags);

	//debug cl
	//may not need
    if((page < 128)||page > (this->chipsize/page_size - 1))
	{
		printk("[%s:] %d,page num = %d fatal error, try to wirte boot page\n",__func__,__LINE__,page);
		spin_unlock_irqrestore(&lock_nand, flags);
		//while(1);
		return -ERESTARTSYS;
	}
/*
	//memset(this->g_databuf, 0xff, page_size);
#ifdef CONFIG_RTK_REMAP_BBT
	if ( chipnr == 0 && page >= 0 && page < ((CONFIG_BOOT_SIZE - (2*BACKUP_BBT*this->block_size))/page_size) ){
#else
    if ( chipnr == 0 && page >= 0 && page < (CONFIG_BOOT_SIZE/page_size) ){
#endif
		printk("[%s] You have no permission to write this page %d\n", __FUNCTION__, page);
		up (&sem);
		return -2;
	}
*/

	if ( len>oob_size || !oob_buf ){
		printk("[%s] error: len>oob_size OR oob_buf is NULL\n", __FUNCTION__);
		spin_unlock_irqrestore(&lock_nand, flags);
		return -1;
	}
	 __flush_cache_all();

	dma_cache_inv((unsigned long) this->g_databuf,page_size);   //czyao
	if ( oob_buf )  {
		dma_cache_inv((unsigned long) oob_buf,oob_size);   //czyao
		oob_sa = ( (uint32_t) oob_buf) & (~M_mask);
	}

	//memcpy(this->g_databuf+512, oob_buf, 8);

	check_ready();
	wmb();
	 __flush_cache_all();
	rtk_writel( (rtk_readl(NACR) | ECC_enable & (~RBO) & (~WBO)), NACR);

	//set DMA RAM start address
	dram_sa = ( (uint32_t) this->g_databuf & (~M_mask));
	rtk_writel( dram_sa, NADRSAR);

	//set DMA RAM oob start address
	rtk_writel( oob_sa, NADTSAR);

	//set DMA flash start address
	for(page_shift=0;page_shift<3; page_shift++) {
             page_num[page_shift] = ((page>>(8*page_shift)) & 0xff);

	     if(!NAND_ADDR_CYCLE){
		flash_addr_t |= (page_num[page_shift] << (12+8*page_shift));
	     }else{
		flash_addr_t |= (page_num[page_shift] << (9+8*page_shift));
	     }
	}
	rtk_writel( flash_addr_t, NADFSAR);

	//DMA write
	wmb();
	rtk_writel( (DESC0|DMAWE|LBC_128 & (~TAG_DIS)),NADCRR);
	check_ready();

	if(rtk_check_pageData(mtd,chipnr,page,0,0)==-1)
	{
		spin_unlock_irqrestore(&lock_nand, flags);
		printk("%s: rtk_check_pageData return fail... page %d\n",__FUNCTION__, page);
		return -1;
	}

	dma_counter--;
	buf_pos++;

	while(dma_counter>0){
		int lastSec = dma_counter-1;
 __flush_cache_all();
		if ( oob_buf )  {
			//dma_cache_wback((unsigned long) oob_buf+buf_pos*oob_unit,oob_unit);   //czyao
			oob_sa = ( (uint32_t) oob_buf+buf_pos*oob_unit) & (~M_mask);
		}

		check_ready();
		wmb();
		//set DMA RAM start address
		rtk_writel( dram_sa, NADRSAR);

		//set DMA RAM oob start address
		rtk_writel( oob_sa, NADTSAR);

		//set DMA flash start address
		for(page_shift=0;page_shift<3; page_shift++) {
	             page_num[page_shift] = ((page>>(8*page_shift)) & 0xff);
	             flash_addr_t |= (page_num[page_shift] << (12+8*page_shift));
		}
		rtk_writel( flash_addr_t, NADFSAR);

		//DMA write
		wmb();
		rtk_writel( (DESC0|DMAWE|LBC_128 & (~TAG_DIS)),NADCRR);
		check_ready();

		if(lastSec<=0)
			lastSec =1;
		else
			lastSec =0;

		if(rtk_check_pageData(mtd,chipnr,page,buf_pos*(512+16),lastSec)==-1)
		{
		    spin_unlock_irqrestore(&lock_nand, flags);
			printk("%s: rtk_check_pageData return fail...page %d, buf_pos %d\n",__FUNCTION__,page,buf_pos);
			return -1;
		}

		dma_counter--;
		buf_pos++;

	}

	unsigned int chip_section = (chipnr * this->page_num) >> 5;
	unsigned int section = page >> 5;
	unsigned int index = page & (32-1);
	this->erase_page_flag[chip_section+section] &= ~(1 << index);
    spin_unlock_irqrestore(&lock_nand, flags);

	return rc;
}

#if 0
int rtk_write_ecc_page (struct mtd_info *mtd, u16 chipnr, unsigned int page,
			const u_char *data_buf, const u_char *oob_buf, int isBBT)
{
	struct nand_chip *this = (struct nand_chip *) mtd->priv;
	int rc = 0;
	//uint8_t	auto_trigger_mode = 1;
	//uint8_t	addr_mode = 1;
	unsigned int page_len, dram_sa, dma_len, oob_sa;
	unsigned char oob_1stB;
	int page_num[3], page_shift=0;
	unsigned long flash_addr_t=0;
	unsigned int dma_counter = page_size>>9, buf_pos=0;
	int page_unit = page_size/4;
	int oob_unit = oob_size/4;
	//int oob_area_size = 8; //oob_area_size means the file system tag size for a page_unit (6bytes + 2bytes dummy)
	//int test=0;
	int lastSec=0;
	//printk("[%s] page %d\n",__FUNCTION__,page);
	//printk("[%s] data_buf 0x%x [0x%x][0x%x][0x%x][0x%x][0x%x]\n",__FUNCTION__,data_buf,data_buf[0],data_buf[1],data_buf[2],data_buf[3],data_buf[4]);

	if (down_interruptible (&sem)) {
			printk("%s : user breaking\n",__FUNCTION__);
			return -ERESTARTSYS;
		}
#if 0 //jasonwang0815
//ccwei 111116

    if(!isBBT){
	    if ( !oob_buf ){
			dma_cache_inv((unsigned long) this->g_oobbuf,oob_size);   //czyao
		    memset(this->g_oobbuf, 0xff, oob_size);
	    }else{
	    #if 0
	    dma_cache_inv((unsigned long) oob_buf,page_size);   //czyao
		    memset(oob_buf, 0xff, oob_size);
		#endif
	    }
    }
#endif
#ifdef CONFIG_REALTEK_MCP
	if ( this->mcp==MCP_AES_ECB ){
		MCP_AES_ECB_Encryption(NULL, data_buf, data_buf, page_size);
	}else{
	}
#endif

	/*winfred_wang*/
	dma_cache_inv(()((unsigned long) data_buf),page_size);   //czyao

	if ( oob_buf ){
		/*winfred_wangh*/
		dma_cache_inv(((unsigned long) oob_buf),page_size);   //czyao
	}else{
		dma_cache_inv((unsigned long) this->g_oobbuf,oob_size);   //czyao
	}
#ifdef CONFIG_RTK_NAND_BBT
	if ( chipnr == 0 && page >= 0 && page < ((CONFIG_BOOT_SIZE - (2*BACKUP_BBT*this->block_size))/page_size) ){
#else
	if ( chipnr == 0 && page >= 0 && page < (CONFIG_BOOT_SIZE/page_size) ){
#endif
		if ( isBBT && page<3*ppb ){
			printk("[%s] Updating BBT %d page=%d\n", __FUNCTION__, page/ppb, page%ppb);
		}else{
			printk("[%s] You have no permission to write page %d\n", __FUNCTION__, page);
			up (&sem);
			return -2;
		}
	}

	/*
	if ( page == 0 || page == 1 || page == ppb || page == ppb+1 )
		oob_1stB = BBT_TAG;
	else
		oob_1stB = 0xFF;
	*/

	//---------------------DMA write the 1st 528 bytes unit------------------//
	if( !oob_buf  && this->g_oobbuf ){
		oob_sa = ( (uint32_t)this->g_oobbuf & (~M_mask));
	}else{
		oob_sa = ( (uint32_t)oob_buf & (~M_mask));
	}
	//dma_cache_inv((unsigned long) oob_sa,page_size);   //czyao
	 __flush_cache_all();

	check_ready();
	wmb();
	rtk_writel( rtk_readl(NACR) & (~RBO) & (~WBO) , NACR);

	//set DMA RAM data start address
	dram_sa = ( (uint32_t)data_buf & (~M_mask));
	rtk_writel( dram_sa, NADRSAR);

	//set DMA RAM oob start address
	rtk_writel( oob_sa, NADTSAR);

	//set DMA flash start address
	for(page_shift=0;page_shift<3; page_shift++) {
             page_num[page_shift] = ((page>>(8*page_shift)) & 0xff);

	     if(!NAND_ADDR_CYCLE){
		 flash_addr_t |= (page_num[page_shift] << (12+8*page_shift));
	     }else{
		flash_addr_t |= (page_num[page_shift] << (9+8*page_shift));
	     }
	}
	rtk_writel( flash_addr_t, NADFSAR);

	//DMA write
	wmb();
	rtk_writel( (DESC0|DMAWE|LBC_128 & (~TAG_DIS)),NADCRR);
	//printk("[%s, line %d] dram_sa = 0x%08X, oob_sa = 0x%08X \n",__FUNCTION__,__LINE__,dram_sa,oob_sa);

	/*for(test=1;test<=oob_area_size;test++){
		printk("0x%02X ", *(oob_buf+test-1));
	}
	printk("\n");*/

	check_ready();

	if(rtk_check_pageData(mtd,chipnr,page,0,0)==-1)
	{
		up (&sem);
		printk("%s:rtk_check_pageData return fail...page %d\n",__FUNCTION__,page);
		return -1;
	}

	dma_counter--;
	buf_pos++;
	//--------------------DMA the other 3*528 bytes-----------------//
	while(dma_counter>0){
		//dma_cache_wback_inv(data_buf+buf_pos*page_unit,page_unit);
		 __flush_cache_all();
		lastSec = dma_counter-1;
		if ( !oob_buf  && this->g_oobbuf ) {
			//dma_cache_wback_inv((unsigned long)this->g_oobbuf+buf_pos*oob_unit,page_unit);
			oob_sa =  ( (uint32_t)(this->g_oobbuf + buf_pos*oob_unit) & (~M_mask));
		}else{
			//dma_cache_wback_inv((unsigned long)oob_buf+buf_pos*oob_unit,page_unit);
			oob_sa =  ( (uint32_t)(oob_buf + buf_pos*oob_unit) & (~M_mask));
		}

		check_ready();
		wmb();
		rtk_writel( rtk_readl(NACR) & (~RBO) & (~WBO) , NACR);

		//set DMA RAM start address, add 512 bytes
		dram_sa = ( (uint32_t)(data_buf+buf_pos*page_unit) & (~M_mask));
		rtk_writel(dram_sa, NADRSAR);

		//set DMA RAM oob address
		rtk_writel( oob_sa, NADTSAR);

		//set DMA flash start address,add (512+6+10)bytes
		flash_addr_t += (page_unit+oob_unit);
		rtk_writel( flash_addr_t, NADFSAR);

		//DMA write
		//rtk_writel( (DESC0|DMAWE|LBC_128),NADCRR);
		wmb();
		rtk_writel( (DESC0|DMAWE|LBC_128 & (~TAG_DIS)),NADCRR);
		//printk("[%s, line %d] dram_sa = 0x%08X, oob_sa = 0x%08X \n",__FUNCTION__,__LINE__,dram_sa,oob_sa);

		/*for(test=1;test<=oob_area_size;test++){
			printk("0x%02X ", *(oob_buf+buf_pos*oob_area_size+test-1));
		}
		printk("\n");*/

		check_ready();

		if(lastSec<=0)
			lastSec =1;
		else
			lastSec =0;

		if(rtk_check_pageData(mtd,chipnr,page,buf_pos*(512+16),lastSec)==-1)
		{
			up (&sem);
			printk("%s:rtk_check_pageData return fail...page %d buf_pos %d\n",__FUNCTION__, page,buf_pos );
			return -1;
		}

		dma_counter--;
		buf_pos++;

	}

	unsigned int chip_section = (chipnr * this->page_num) >> 5;
	unsigned int section = page >> 5;
	unsigned int index = page & (32-1);
	this->erase_page_flag[chip_section+section] &= ~(1 << index);
	up (&sem);

	return rc;
}
#endif



/*************************************************************************
**  rtk_write_ecc_page()
**	descriptions: write data to nand flash
**	parameters: chunk id  (must alignment of real_page/block)
**	return: 0:succes,-1 is fail
**  note: this function not check bad block table.

*************************************************************************/


//int _nand_write_chunk(u8_t *chunk_buf, spare_u *spare, u32_t chunk_id)
int rtk_write_ecc_page (struct mtd_info *mtd, u16 chipnr, unsigned int page,
			const u_char *data_buf, const u_char *oob_buf, int isBBT)
{
	//int real_page = chunk_id*nandflash_info.page_per_chunk; //get real_page number
	struct nand_chip *this = (struct nand_chip *) mtd->priv;
	int real_page = page;
	//debug cl

	//bootcode in block 0 and block 1,nend check
    if((page < 128)||page > (this->chipsize/page_size - 1))
	{
		printk("[%s:] %d,page num = %d fatal error, try to wirte boot page\n",__func__,__LINE__,page);
		//while(1);
		return -ERESTARTSYS;
	}
	int dma_counter = 4; //always 512byte * 4
	int dram_sa, oob_sa;
	int page_shift;
	int page_num[3];
	unsigned long flash_addr_t=0;
	unsigned long flash_addr2_t=0;
	int buf_pos=0;
	int return_value=0;
	int ecc_count=0;
	int i;
	unsigned long flags;
	ppb = mtd->erasesize/mtd->writesize;

	int block = real_page/ppb;

	#ifdef SWAP_2K_DATA
	unsigned char *data_ptr, *oob_ptr;
	unsigned int write_bbi;
	unsigned char switch_bbi;
	#endif

	uint8_t *oob_area;
	uint8_t *data_area, data_area0[512+16+CACHELINE_SIZE+CACHELINE_SIZE-4]; //data,oob point must 32 cache aligment

	memset(data_area0, 0xff, 512+16+CACHELINE_SIZE+CACHELINE_SIZE-4);

	data_area = (uint8_t*) ((uint32_t)(data_area0 + 12) & 0xFFFFFFF0);
	oob_area=(uint8_t*) data_area+512;

	//printk("_nand_write_chunk : spare->u8_oob 0x%p\n",spare->u8_oob );

	unsigned int flags_nand = 0;
	spin_lock_irqsave(&lock_nand, flags_nand);

	/*winfred_wang*/
	#if 0
	if(block>0){
		//swap bbi and data
		spare->u8_oob[nandflash_info.bbi_swap_offset]=chunk_buf[nandflash_info.bbi_dma_offset];
		chunk_buf[nandflash_info.bbi_dma_offset]=0xff;
	}
	#endif

	#ifdef SWAP_2K_DATA
	if(block>0){
		//unsigned int write_bbi;
		//unsigned char switch_bbi;

		if(isLastPage){
			/*bad block indicator is located in page (ppb-2) and page (ppb-1)*/
			write_bbi = page & (ppb-1);
			if((write_bbi == (ppb-2)) || (write_bbi ==(ppb-1))){
				switch_bbi = 1;
			}
		}else{
			/*bad block indicator is located in page 0 and page 1*/
			write_bbi = page & (ppb-1);
			if((write_bbi == 0) || (write_bbi ==1)){
				switch_bbi = 1;
			}
		}

		/*test code need check*/
		/*because image convert app cannot decise which is the first page of block,so need do switch in every page*/
		switch_bbi = 1;

		if(!NAND_ADDR_CYCLE)
		{
			unsigned char temp_val;
			if(switch_bbi){
				//memcpy(this->g_databuf,data_buf,page_size);
				temp_val = data_buf[DATA_BBI_OFF];
				data_ptr = data_buf;
				oob_ptr = oob_buf;
				#if 0
				if(oob_buf){
					this->g_databuf[DATA_BBI_OFF] = oob_buf[OOB_BBI_OFF];
					oob_ptr = oob_buf;
					oob_ptr[OOB_BBI_OFF] = temp_val;
				}
				else
				#endif
				{
					data_ptr[DATA_BBI_OFF] = oob_buf[OOB_BBI_OFF];
					oob_ptr[OOB_BBI_OFF] = temp_val;
				}
			}
		}
	}

	#endif


	//dma_cache_wback_inv((u32_t *)chunk_buf,2048);
	//set DMA flash start address

	for(page_shift=0;page_shift<3; page_shift++) {
	    page_num[page_shift] = ((real_page>>(8*page_shift)) & 0xff);
		//if(nandflash_info.page_size==2048)
		{
			flash_addr_t |= (page_num[page_shift] << (12+8*page_shift));
		}
	}

	#if 0
	for(page_shift=0;page_shift<3; page_shift++) {
	    page_num[page_shift] = ((real_page>>(8*page_shift)) & 0xff);
		if(nandflash_info.page_size==2048){
			flash_addr_t |= (page_num[page_shift] << (12+8*page_shift));
		}else if(nandflash_info.page_size==4096){
			flash_addr_t |= (page_num[page_shift] << (13+8*page_shift));
		}else if(nandflash_info.page_size==512){/* 512 */
			flash_addr_t |= (page_num[page_shift] << (9+8*page_shift));
		}
	}
	#endif

#if 1 //BCH
		// If the nand flash size is larger than 2G bytes, fill the other address for DMA
		flash_addr2_t= (real_page >> 20);
#endif

	rtk_writel(0xC00FFFFF, NACR);     //Enable ECC
	rtk_writel(0x0000000F, NASR);	  //clear NAND flash status register

	oob_sa =  ( (uint32_t)(oob_area ) & (~M_mask));
	dram_sa = ((uint32_t)data_area) & (~M_mask);

	/* dma move  4*528byte */

	while(dma_counter>0){

		int lastSec = dma_counter - 1;

		//copy oob to buffer
		//if(oob_buf)
		memcpy(oob_area, oob_buf+(buf_pos*6), 6);
		memcpy(data_area, data_buf+(buf_pos*512), 512);

		dma_cache_wback_inv((uint32_t *)data_area,528);//512+16
		rtk_writel( rtk_readl(NACR) & (~RBO) & (~WBO) , NACR);
		//write data/oob address
		rtk_writel(dram_sa, NADRSAR);
		rtk_writel( oob_sa, NADTSAR);
		rtk_writel( flash_addr_t, NADFSAR);

	/*winfred_wang*/
#if 0//BCH
		rtk_writel( flash_addr2_t, NADFSAR2);
#endif
		//dma write cmd
		//local_irq_save(flags);
		rtk_writel( (DESC0|DMAWE|LBC_64 & (~TAG_DIS)),NADCRR);

		check_ready();
		//check status register
		//local_irq_restore(flags);

		if(lastSec<=0)
			lastSec =1;
		else
			lastSec =0;

		if(rtk_check_pageData(mtd,chipnr,page,buf_pos*(512+16),lastSec)==-1)
		{
		    spin_unlock_irqrestore(&lock_nand, flags_nand);
			printk("rtk_check_pageData return fail...\n");
			return -1;
		}

#if 0 //eric not need check. for write always ok...
		ecc_count=nand_check_eccStatus();
	  //Record ecc counter ,return the max number
		if((return_value!=-1) && (ecc_count != -1))
		{
			return_value= (ecc_count>return_value)?ecc_count:return_value;
		}else{
			return_value=-1;
			//printk("\r\n _nand_write_chunk return -1\n");
		}
#endif
		rtk_writel(0xF, NASR);


		flash_addr_t += (528); //512+16 one unit

		dma_counter--;
		buf_pos++;
	}

	/*winfred_wang*/
	#ifdef SWAP_2K_DATA
	//swap back

	if(block>0){


		if(isLastPage){
			/*bad block indicator is located in page (ppb-2) and page (ppb-1)*/
			write_bbi = page & (ppb-1);
			if((write_bbi == (ppb-2)) || (write_bbi ==(ppb-1))){
				switch_bbi = 1;
			}
		}else{
			/*bad block indicator is located in page 0 and page 1*/
			write_bbi = page & (ppb-1);
			if((write_bbi == 0) || (write_bbi ==1)){
				switch_bbi = 1;
			}
		}

		/*test code*/
		/*because image convert app cannot decise which is the first page of block,so need do switch in every page*/
		switch_bbi = 1;

		if(!NAND_ADDR_CYCLE)
		{
			unsigned char temp_val;
			if(switch_bbi){
				//memcpy(this->g_databuf,data_buf,page_size);
				temp_val = data_buf[DATA_BBI_OFF];
				data_ptr = data_buf;
				oob_ptr = oob_buf;
				#if 0
				if(oob_buf){
					this->g_databuf[DATA_BBI_OFF] = oob_buf[OOB_BBI_OFF];
					oob_ptr = oob_buf;
					oob_ptr[OOB_BBI_OFF] = temp_val;
				}
				else
				#endif
				{
					data_ptr[DATA_BBI_OFF] = oob_buf[OOB_BBI_OFF];
					oob_ptr[OOB_BBI_OFF] = temp_val;
				}
			}
		}
	}
	#endif

	#if 0
	if(block>0){
		//swap bbi and data again. (recovery data)
		chunk_buf[nandflash_info.bbi_dma_offset]= spare->u8_oob[nandflash_info.bbi_swap_offset];
		//eric mark not need
	//	dma_cache_wback_inv((u32_t *)chunk_buf,2048);
	}
	#endif
    spin_unlock_irqrestore(&lock_nand, flags_nand);

	return return_value;
}

const char *ptypes[] = {"cmdlinepart", NULL};

static int rtk_nand_profile (void)
{
	int maxchips = 4;
	int err;
	struct nand_chip *this = (struct nand_chip *)rtk_mtd->priv;
    //  120203
#ifdef CONFIG_RTK_REMAP_BBT
	this->RBA_PERCENT = RBA_PERCENT;
#endif
#ifdef CONFIG_RTK_NORMAL_BBT
	this->BBT_PERCENT = BBT_PERCENT;
#endif
	//printk("[%s, line %d] maxchips = %d\n",__FUNCTION__,__LINE__,maxchips);
	if (rtk_nand_scan (rtk_mtd, maxchips) < 0 || rtk_mtd->size == 0){
		printk("%s: Error, cannot do nand_scan(on-board)\n", __FUNCTION__);
		return -ENODEV;
	}


	if ( !(rtk_mtd->writesize&(0x200-1)) )
		;//rtk_writel( rtk_mtd->oobblock >> 9, REG_PAGE_LEN);
	else{
		printk("Error: pagesize is not 512B Multiple");
		return -1;
	}

	char *ptype;
	int pnum = 0;
	struct mtd_partition *mtd_parts;

#ifdef CONFIG_WRT_BARRIER_BREAKER
	detect_nand_flash_map(rtk_mtd,rtl8196_parts1);
	if(mtd_device_parse_register(rtk_mtd,NULL,NULL,rtl8196_parts1,ARRAY_SIZE(rtl8196_parts1))){
		printk("%s: Error, cannot add %s device\n",
					__FUNCTION__, rtk_mtd->name);
		rtk_mtd->size = 0;
		return -EAGAIN;
	}
	return 0;
#endif

#ifdef CONFIG_MTD_CMDLINE_PARTS
#if LINUX_VERSION_CODE < KERNEL_VERSION(3,10,0)
	ptype = (char *)ptypes[0];
	pnum = parse_mtd_partitions (rtk_mtd, ptypes, &mtd_parts, 0);

	if (pnum <= 0) {
		printk(KERN_NOTICE "RTK: using the whole nand as a partitoin\n");
		if(add_mtd_device(rtk_mtd)) {
			printk(KERN_WARNING "RTK: Failed to register new nand device\n");
			return -EAGAIN;
		}
	}else{
		printk(KERN_NOTICE "RTK: using dynamic nand partition\n");
		if (add_mtd_partitions (rtk_mtd, mtd_parts, pnum)) {
			printk("%s: Error, cannot add %s device\n",
					__FUNCTION__, rtk_mtd->name);
			rtk_mtd->size = 0;
			return -EAGAIN;
		}
	}
#else
	ptype = (char *)ptypes[0];
	if(mtd_device_parse_register(rtk_mtd,ptypes,NULL,NULL,NULL)){
		printk("%s: Error, cannot add %s device\n",
					__FUNCTION__, rtk_mtd->name);
		rtk_mtd->size = 0;
		return -EAGAIN;
	}
#endif
#endif
	return 0;
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,0)
static int rtk_read_proc_nandinfo(struct seq_file *s, void *v)
{
	struct nand_chip *this = (struct nand_chip *) rtk_mtd->priv;
	int wlen = 0;

	seq_printf(s,"nand_size:%u\n", this->device_size);
	seq_printf(s,"chip_size:%u\n", this->chipsize);
	seq_printf(s,"block_size:%u\n", rtk_mtd->erasesize);
	seq_printf(s,"page_size:%u\n", rtk_mtd->writesize);
	seq_printf(s,"oob_size:%u\n", rtk_mtd->oobsize);
	seq_printf(s,"ppb:%u\n", rtk_mtd->erasesize/rtk_mtd->writesize);
#ifdef CONFIG_RTK_REMAP_BBT
	seq_printf(s,"RBA:%u\n", this->RBA);
#endif
#ifdef CONFIG_RTK_NORMAL_BBT
	seq_printf(s,"bbt_num:%u\n", this->bbt_num);
#endif
	seq_printf(s,"BBs:%u\n", this->BBs);

	return 0;

}
#else
int rtk_read_proc_nandinfo(char *buf, char **start, off_t offset, int len, int *eof, void *data)
{
	struct nand_chip *this = (struct nand_chip *) rtk_mtd->priv;
	int wlen = 0;

	//wlen += sprintf(buf+wlen,"nand_PartNum:%s\n", rtk_mtd->PartNum); //czyao
	wlen += sprintf(buf+wlen,"nand_size:%u\n", this->device_size);
	wlen += sprintf(buf+wlen,"chip_size:%u\n", this->chipsize);
	wlen += sprintf(buf+wlen,"block_size:%u\n", rtk_mtd->erasesize);
	wlen += sprintf(buf+wlen,"page_size:%u\n", rtk_mtd->writesize);
	wlen += sprintf(buf+wlen,"oob_size:%u\n", rtk_mtd->oobsize);
	wlen += sprintf(buf+wlen,"ppb:%u\n", rtk_mtd->erasesize/rtk_mtd->writesize);
#ifdef CONFIG_RTK_REMAP_BBT
	wlen += sprintf(buf+wlen,"RBA:%u\n", this->RBA);
#endif
#ifdef CONFIG_RTK_NORMAL_BBT
	wlen += sprintf(buf+wlen,"bbt_num:%u\n", this->bbt_num);
#endif
	wlen += sprintf(buf+wlen,"BBs:%u\n", this->BBs);
	return wlen;
}
#endif


static void display_version (void)
{
	const __u8 *revision;
	const __u8 *date;
	char *running = (__u8 *)VERSION;
	strsep(&running, " ");
	strsep(&running, " ");
	revision = strsep(&running, " ");
	date = strsep(&running, " ");
	printk(BANNER " Rev:%s (%s)\n", revision, date);
}

static int __init rtk_nand_init (void)
{
	#if 0  //czyao
	if ( is_venus_cpu() || is_neptune_cpu() )
		return -1;

	if ( rtk_readl(0xb800000c) & 0x00800000 ){
		display_version();
	}else{
		printk(KERN_ERR "Nand Flash Clock is NOT Open. Installing fails.\n");
		return -1;
	}

	rtk_writel( 0x103, 0xb800036c );

	rtk_writel(rtk_readl(0xb800000c)& (~0x00800000), 0xb800000c);
	rtk_writel( 0x02, 0xb8000034 );
	rtk_writel(rtk_readl(0xb800000c)| (0x00800000), 0xb800000c);
	#endif  //end of czyao

	display_version();

	spin_lock_init(&lock_nand);
	struct nand_chip *this;
	int rc = 0;

	rtk_mtd = kmalloc (MTDSIZE, GFP_KERNEL);
	if ( !rtk_mtd ){
		printk("%s: Error, no enough memory for rtk_mtd\n",__FUNCTION__);
		rc = -ENOMEM;
		goto EXIT;
	}
	memset ( (char *)rtk_mtd, 0, MTDSIZE);
	rtk_mtd->priv = this = (struct nand_chip *)(rtk_mtd+1);

	//rtk_writel(0xC0000000, NACFR);   //Set 2 cycle command, 5 address cycle
	rtk_writel(0xC00FFFFF, NACR);     //Enable ECC
	rtk_writel(0x0000000F, NASR);     //clear NAND flash status register


	this->read_id			= rtk_nand_read_id;
	this->read_ecc_page 	= rtk_read_ecc_page;
	this->read_oob 		= rtk_read_oob;
	this->write_ecc_page	= rtk_write_ecc_page;
	this->write_oob		= rtk_write_oob;
	this->erase_block 		= rtk_erase_block;
	//this->suspend			= rtk_nand_suspend;
	//this->resume			= rtk_nand_resume;
	this->sync			= NULL;

	if( rtk_nand_profile() < 0 ){
		rc = -1;
		goto EXIT;
	}

	page_size = rtk_mtd->writesize;
	oob_size = rtk_mtd->oobsize;
	ppb = (rtk_mtd->erasesize)/(rtk_mtd->writesize);

#if LINUX_VERSION_CODE < KERNEL_VERSION(3,10,0)
	create_proc_read_entry("nandinfo", 0, NULL, rtk_read_proc_nandinfo, NULL);
#else
	proc_create("nandinfo", 0, NULL, &rtk_nand_flash_ops);
#endif

EXIT:
	if (rc < 0) {
		if (rtk_mtd){
#if LINUX_VERSION_CODE < KERNEL_VERSION(3,10,0)
			del_mtd_partitions (rtk_mtd);
#else
			mtd_device_unregister(rtk_mtd);
#endif
			if (this->g_databuf)
				kfree(this->g_databuf);
			if(this->g_oobbuf)
				kfree(this->g_oobbuf);
			if (this->erase_page_flag){
				unsigned int flag_size =  (this->numchips * this->page_num) >> 3;
				unsigned int mempage_order = get_order(flag_size);
				free_pages((unsigned long)this->erase_page_flag, mempage_order);
			}
			kfree(rtk_mtd);
		}
		remove_proc_entry("nandinfo", NULL);
	}else
		printk(KERN_INFO "Realtek Nand Flash Driver is successfully installing.\n");

	return rc;
}


void __exit rtk_nand_exit (void)
{
	if (rtk_mtd){
#if LINUX_VERSION_CODE < KERNEL_VERSION(3,10,0)
		del_mtd_partitions (rtk_mtd);
#else
		mtd_device_unregister(rtk_mtd);
#endif
		struct nand_chip *this = (struct nand_chip *)rtk_mtd->priv;
		if (this->g_databuf)
			kfree(this->g_databuf);
		if(this->g_oobbuf)
			kfree(this->g_oobbuf);
		if (this->erase_page_flag){
			unsigned int flag_size =  (this->numchips * this->page_num) >> 3;
			unsigned int mempage_order = get_order(flag_size);
			free_pages((unsigned long)this->erase_page_flag, mempage_order);
		}
		kfree(rtk_mtd);
	}
	remove_proc_entry("nandinfo", NULL);
}

module_init(rtk_nand_init);
module_exit(rtk_nand_exit);
//MODULE_AUTHOR("Ken Yu<Ken.Yu@realtek.com>");
//MODULE_DESCRIPTION("Realtek NAND Flash Controller Driver");
