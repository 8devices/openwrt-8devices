/*
 * BCM47XX NAND flash driver
 *
 * Copyright (C) 2012 Rafał Miłecki <zajec5@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */

#include "rtknflash.h"
#include <linux/kernel.h>
#ifdef __RTK_BOOT__
#include <linux/slab.h>
#include <linux/spinlock.h>
#else
#ifdef __UBOOT__
#include <nand.h>
#else
#include <linux/mtd/mtd.h>
#include <linux/mtd/partitions.h>
#include <linux/version.h>
#include <linux/proc_fs.h>
#include <linux/slab.h>
#include <linux/spinlock.h>
#include <asm/uaccess.h>
#endif
#endif

#ifdef CONFIG_PARALLEL_NAND_FLASH
#include "rtkn_parallel_regs.h"
#endif

static spinlock_t lock_nand;
extern struct rtknflash *rtkn;
#if 0
#if !(defined(__UBOOT__) || defined(__RTK_BOOT__))
static const char *probes[] = { "cmdlinepart", NULL };
#endif
#endif

#ifdef RTKN_FLASH_TEST
static int rtkn_test_num = 0;
#endif
#ifdef RTKN_FLASH_TEST_WRITE
static int rtkn_test_write = 0;
#endif

#ifdef __UBOOT__
#ifdef CONFIG_RTK_REMAP_BBT
unsigned int uboot_scrub = 0;
#endif
#endif
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,0)
#ifndef __RTK_BOOT__
#include <linux/mtd/partitions.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#endif
#endif

#include "rtknflash_openwrt.c"
#if !(defined(__UBOOT__) || defined(__RTK_BOOT__))
#if !defined(CONFIG_MTD_RTKXX_PARTS)
#include "rtknflash_partition.c"
#elif defined(CONFIG_MTD_RTKXX_PARTS) && defined(CONFIG_RTK_NAND_FLASH_STORAGE)
#include "rtknflash_partition.c"
#endif
#endif

#if defined(CONFIG_MTD_RTKXX_PARTS)
/* Define realtek mtd partion hook function */
static const char * const probes[] = { "rtkxxpart", NULL };
#endif

/* other param */
#if defined(__UBOOT__) || defined(__RTK_BOOT__)
unsigned int uboot_scrub = 0;
#endif
/**************************************************
 * Various helpers
 **************************************************/
/* realtek */
#if !defined(__RTK_BOOT__)
#if CONFIG_MTD_NAND_RTK_PAGE_SIZE==2048
static struct nand_ecclayout nand_bch_oob_64 = {
	.eccbytes = 40,
	.eccpos = {
			24,25, 26, 27, 28, 29, 30, 31,
			32,33, 34, 35, 36, 37, 38, 39,
			40,41, 42, 43, 44, 45, 46, 47,
			48,49, 50, 51, 52, 53, 54, 55,
			56,57, 58, 59, 60, 61, 62, 63},
	/* offset 0: bbi */
	.oobfree = {
		{.offset = 2,
		 .length = 24}
		 }
};

#endif

#if  CONFIG_MTD_NAND_RTK_PAGE_SIZE==4096
static struct nand_ecclayout nand_bch_oob_128 = {
	.eccbytes = 80,
	.eccpos = {
			48,  49,  50,  51, 52, 53, 54, 55, 56,
			57, 58, 59, 60, 61, 62, 63, 64, 65, 66,
		    67, 68, 69, 70, 71, 72, 73, 74, 75, 76,
		    77, 78, 79, 80, 81, 82, 83, 84, 85, 86,
		    87, 88, 89, 90, 91, 92, 93, 94, 95, 96,
		    97, 98, 99, 100, 101, 102, 103, 104, 105, 106,
		    107,108,109,110,111,112,113,114,115,116,
		    117, 118,119,120,121,122,123,124,125,126,127},
	/* offset 0: bbi */
	.oobfree = {
		{.offset = 2,
		 .length = 48}
		 }
};
#endif

#if  CONFIG_MTD_NAND_RTK_PAGE_SIZE==8192
static struct nand_ecclayout nand_bch_oob_256 = {
	.eccbytes = 160,
	.eccpos = {
			96,  97,  98,  99, 100, 101, 102, 103, 104,
			105, 106, 107, 108, 109, 110, 111, 112, 113, 114,
		    115, 116, 117, 118, 119, 120, 121, 122, 123, 124,
		    125, 126, 127, 128, 129, 130, 131, 132, 133, 134,
		    135, 136, 137, 138, 139, 140, 141, 142, 143, 144,
		    145, 146, 147, 148, 149, 150, 151, 152, 153, 154,
		    155,156,157,158,159,160,161,162,163,164,
		    165,166,167,168,169,170,171,172,173,174,
		    175,176,177,178,179,180,181,182,183,184,
		    185,186,187,188,189,190,191,192,193,194,
		    195,196,197,198,199,200,201,202,203,204,
		    205,206,207,208,209,210,211,212,213,214,
		    215,216,217,218,219,220,221,222,223,224,
		    225,226,227,228,229,230,231,232,233,234,
		    235,236,237,238,239,240,241,242,243,244,
		    245,236,247,248,249,250,251,252,253,254,255
		    },
	/* offset 0: bbi */
	.oobfree = {
		{.offset = 2,
		 .length = 96}
};
#endif
#endif



/**************************************************
 * NAND chip ops
 **************************************************/
#if 1//def __RTK_BOOT__
void rtknflash_ops_select_chip(struct mtd_info *mtd,
						  int chip)
#else
static void rtknflash_ops_select_chip(struct mtd_info *mtd,
						  int chip)
#endif
{
	unsigned long flags_nand = 0;
	spin_lock_irqsave(&lock_nand, flags_nand);
#if defined(CONFIG_PARALLEL_NAND_FLASH)
	rtkn_select_chip_cmd(chip);
#endif
    spin_unlock_irqrestore(&lock_nand, flags_nand);
}

/*
 * Default nand_command and nand_command_lp don't match BCM4706 hardware layout.
 * For example, reading chip id is performed in a non-standard way.
 * Setting column and page is also handled differently, we use a special
 * registers of ChipCommon core. Hacking cmd_ctrl to understand and convert
 * standard commands would be much more complicated.
 */

/* need modify to support full id */
#ifdef __RTK_BOOT__
void rtknflash_read_id_cmd(struct rtknflash *rtkn)
#else
static void rtknflash_read_id_cmd(struct rtknflash *rtkn)
#endif
{
	unsigned long flags_nand = 0;
#if defined(CONFIG_SPI_NAND_FLASH)
	int id_chain;
#endif
	spin_lock_irqsave(&lock_nand, flags_nand);
#if defined(CONFIG_SPI_NAND_FLASH)
    id_chain = winbond_read_id();
    memset(rtkn->id_data,0,8);

	rtkn->id_data[0] = (id_chain >> 16)& 0xff;
        rtkn->id_data[1] = (id_chain >> 8) & 0xff;
        rtkn->id_data[2] = (id_chain) & 0xff;
	printk("id_chain value=%x\n",id_chain);

	/* set islargepage ,default 0 */
	rtkn->chip_param.id = id_chain;
	rtkn->chip_param.isLastPage = 0;

#elif defined(CONFIG_PARALLEL_NAND_FLASH)
	parallel_nand_read_id_cmd(rtkn->id_data);
#endif
	spin_unlock_irqrestore(&lock_nand, flags_nand);
}

/* reset cmd */
static void rtknflash_reset_cmd(struct rtknflash *rtkn)
{
	unsigned long flags_nand = 0;
	spin_lock_irqsave(&lock_nand, flags_nand);
#if defined(CONFIG_SPI_NAND_FLASH)
	winbond_reset();
#elif defined(CONFIG_PARALLEL_NAND_FLASH)
	parallel_nand_reset_cmd();
#endif
	spin_unlock_irqrestore(&lock_nand, flags_nand);
}

/* read_status cmd */
static void rtknflash_read_status_cmd(struct rtknflash *rtkn)
{
	unsigned long flags_nand = 0;
	spin_lock_irqsave(&lock_nand, flags_nand);
#if defined(CONFIG_SPI_NAND_FLASH)
	/* temp code */
	#define NAND_STATUS_TRUE_READY	0x20
	#define NAND_STATUS_READY	0x40
	#define NAND_STATUS_WP		0x80

	rtkn->status = NAND_STATUS_TRUE_READY|NAND_STATUS_READY|NAND_STATUS_WP;
#elif defined(CONFIG_PARALLEL_NAND_FLASH)
	parallel_nand_read_status();
#endif

	spin_unlock_irqrestore(&lock_nand, flags_nand);
}



/* erase cmd */
#ifdef __RTK_BOOT__
int rtknflash_erase1_cmd(struct mtd_info* mtd,struct rtknflash *rtkn)
#else
static int rtknflash_erase1_cmd(struct mtd_info* mtd,struct rtknflash *rtkn)
#endif
{
	unsigned long flags_nand = 0;
	unsigned int page_addr = rtkn->curr_page_addr;
	int res;
	spin_lock_irqsave(&lock_nand, flags_nand);


#ifdef CONFIG_RTK_REMAP_BBT
#if defined(__UBOOT__) || defined(__RTK_BOOT__)
	if(uboot_scrub == 0)
#endif
		page_addr = rtkn_bbt_get_realpage(mtd,page_addr);
#endif

#if defined(CONFIG_SPI_NAND_FLASH)
	if(winbond_block_erase(page_addr) < 0){
		res = -1;
		goto Error;
	}
#elif defined(CONFIG_PARALLEL_NAND_FLASH)
	if(paralledl_nand_erase_cmd(page_addr) < 0){
		res = -1;
		goto Error;
	}
#endif

	spin_unlock_irqrestore(&lock_nand, flags_nand);
    return 0;

Error:
#ifdef CONFIG_RTK_REMAP_BBT
    printk("[%s]:%d,write fail,page=%x\n",__func__,__LINE__,page_addr);
    spin_unlock_irqrestore(&lock_nand, flags_nand);

#if defined(__UBOOT__) || defined(__RTK_BOOT__)
    if(uboot_scrub == 0)
#endif
	res = rtk_remapBBT_erase_fail(mtd,page_addr);
    return res;
#else
    spin_unlock_irqrestore(&lock_nand, flags_nand);
    //printk("[%s] erasure is not completed at block %d\n", __FUNCTION__, page/ppb);
    printk("erase error\n");
    return -1;
#endif
}

#if LINUX_VERSION_CODE < KERNEL_VERSION(3,10,0)
static int rtkn_ecc_read_page(struct mtd_info *mtd, struct nand_chip *chip,
				uint8_t *buf)
#else
#ifdef __RTK_BOOT__
int rtkn_ecc_read_page(struct mtd_info *mtd, struct nand_chip *chip,
			uint8_t *buf, int oob_required, int page)
#else
static int rtkn_ecc_read_page(struct mtd_info *mtd, struct nand_chip *chip,
			uint8_t *buf, int oob_required, int page)
#endif
#endif
{
	struct rtknflash *rtkn = (struct rtknflash *)chip->priv;
	uint8_t* oobBuf = chip->oob_poi;
	unsigned long flags_nand = 0;
	unsigned int chunk_size = mtd->writesize;
	int realpage = page;
	int spare_step = chunk_size/512;
#if defined(CONFIG_SPI_NAND_FLASH)
#ifdef RTK_BUILT_IN_ECC
	unsigned char tmpBuf[64]; //temp code
	int ret = 0;
#endif
#endif
#if defined(CONFIG_PARALLEL_NAND_FLASH)
	int i;
#endif

	spin_lock_irqsave(&lock_nand, flags_nand);
#ifdef CONFIG_RTK_REMAP_BBT
     realpage = rtkn_bbt_get_realpage(mtd,realpage);
#endif

#if defined(CONFIG_SPI_NAND_FLASH)
#ifdef RTK_BUILT_IN_ECC

	ret = winbond_2KB_chunk_read_with_ecc_decode(rtkn->buf,realpage,tmpBuf);
	if(ret < 0){
		goto Error;
	}

	memcpy(buf,rtkn->buf,chunk_size);
	if(oob_required){
		/* for spi nand 4+2 layout */
		memcpy(oobBuf,rtkn->buf+chunk_size,6*spare_step);
	}
#else
	winbond_dma_read_data(rtkn->buf,chunk_size+mtd->oobsize, realpage);
	memcpy(buf,rtkn->buf,chunk_size);
	if(oob_required){
		memcpy(oobBuf,rtkn->buf+chunk_size,mtd->oobsize);
	}
#endif
#elif defined(CONFIG_PARALLEL_NAND_FLASH)
	if(parallel_nand_read_page(rtkn,rtkn->buf,rtkn->buf+chunk_size,realpage) < 0)
		goto Error;
	memcpy(buf,rtkn->buf,chunk_size);
	if(oob_required){
		/* spare ram 8 byte aligned*/
		for(i = 0;i < spare_step;i++)
			memcpy(oobBuf+6*i,rtkn->buf+chunk_size+8*i,6);
	}

#endif

	spin_unlock_irqrestore(&lock_nand, flags_nand);
	return 0;

Error:
/* read function donot need do bbt */
	/* read may ecc error; */
#ifdef CONFIG_RTK_REMAP_BBT
	printk("[%s]:%d,read fail\n",__func__,__LINE__);
	spin_unlock_irqrestore(&lock_nand, flags_nand);
	rtk_remapBBT_read_fail(mtd,page);
	return 0;
#else
	spin_unlock_irqrestore(&lock_nand, flags_nand);
	printk("rtk_check_pageData return fail...\n");
	return -1;
#endif
}


#ifdef CONFIG_JFFS2_FS
/* jffs2 clearmark at first page of block */
/* if write first page of block and find oob_area is not 0xff,need reerase this block */
#define  	JFFS2_NEED_REERAE 			0x1
#define 	JFFS2_NO_NEED_REERAE		0x2

static int rtkn_jffs2_write_patch_check(struct mtd_info *mtd, struct nand_chip *chip,
				const uint8_t *buf, int oob_required,int page)
{
	int i,res;
	struct rtknflash* rtkn = (struct rtknflash*)chip->priv;
	unsigned int oobsize=mtd->oobsize,pagesize=mtd->writesize;
	unsigned int ppb = (mtd->erasesize)/(mtd->writesize);

	if(page % ppb == 0){
		memset(rtkn->tmpBuf,0xff,MAX_RTKN_BUF_SIZE);
		res = rtk_scan_read_oob(mtd,rtkn->tmpBuf,page,pagesize);
		if(res){
			printk("[%s]:%d:read ecc page fail\n",__func__,__LINE__);
			return res;
		}
		for(i = 0;i< oobsize;i++){
			if(rtkn->tmpBuf[pagesize+i] != 0xff)
				return JFFS2_NEED_REERAE;
		}

	}

	return JFFS2_NO_NEED_REERAE;
}

static int rtkn_jffs2_write_patch(struct mtd_info *mtd, struct nand_chip *chip,
				const uint8_t *buf, int oob_required,int page)
{
	int res = 0,i;
	struct rtknflash* rtkn = (struct rtknflash*)chip->priv;
	unsigned int oobsize=mtd->oobsize,pagesize=mtd->writesize;
	unsigned char* blocktmp=NULL;
	unsigned int ppb = (mtd->erasesize)/(mtd->writesize);

	{
		/* this page have write;need reerase the whole block */

		blocktmp = (unsigned char*)kmalloc((pagesize+oobsize)*ppb,GFP_KERNEL);
		if(!blocktmp){
			return -1;
		}
		memset(blocktmp,0xff,(pagesize+oobsize)*ppb);

		memcpy(blocktmp,buf,pagesize);
		memcpy(blocktmp+pagesize,rtkn->tmpBuf+pagesize,oobsize);

		if(oob_required)
			memcpy(blocktmp+pagesize,chip->oob_poi,oobsize);

		/* read */
		for(i = 1;i<ppb;i++){
			res = rtk_scan_read_oob(mtd,blocktmp+(pagesize+oobsize)*i,page+i,pagesize);
			if(res < 0)
				goto Error;
		}

		/* erase */
		if(rtk_scan_erase_bbt(mtd,page) < 0)
			goto Error;

		/*write*/
		for(i = 0;i< ppb;i++){
			memset(rtkn->tmpBuf,0xff,MAX_RTKN_BUF_SIZE);
			if(memcmp(blocktmp + i*(pagesize+oobsize),rtkn->tmpBuf,pagesize+oobsize) == 0)
				continue;

			res = rtk_scan_write_bbt(mtd,page+i,pagesize,blocktmp+i*(pagesize+oobsize),blocktmp+i*(pagesize+oobsize)+pagesize);
			if(res < 0){
				goto Error;
			}
		}
		goto FINISH;
	}

Error:
	if(blocktmp)
		kfree(blocktmp);
	return -1;
FINISH:
	if(blocktmp)
		kfree(blocktmp);
	return 0;
}
#endif

#ifdef __RTK_BOOT__
int rtkn_ecc_write_page(struct mtd_info *mtd, struct nand_chip *chip,
				const uint8_t *buf, int oob_required)
#else
static int rtkn_ecc_write_page(struct mtd_info *mtd, struct nand_chip *chip,
				const uint8_t *buf, int oob_required)
#endif
{
	struct rtknflash *rtkn = (struct rtknflash *)chip->priv;
	unsigned char* oobBuf = chip->oob_poi;
	int page = rtkn->curr_page_addr;
	unsigned int chunk_size = mtd->writesize;
	unsigned long flags_nand = 0;
	int res = 0;
	int spare_step = chunk_size/512;
#if defined(CONFIG_SPI_NAND_FLASH)
#ifdef RTK_BUILT_IN_ECC
	unsigned char tmpBuf[64]; //temp code
#endif
#endif
#if defined(CONFIG_PARALLEL_NAND_FLASH)
	int i;
#endif

	spin_lock_irqsave(&lock_nand, flags_nand);

#ifdef CONFIG_RTK_REMAP_BBT
	page = rtkn_bbt_get_realpage(mtd,page);
#endif


#ifdef CONFIG_JFFS2_FS
	spin_unlock_irqrestore(&lock_nand, flags_nand);
	res = rtkn_jffs2_write_patch_check(mtd,chip,buf,oob_required,page);
	if(res < 0){
		printk("donot write page\n");
		return res;
	}
	else if(res == JFFS2_NEED_REERAE){
		//printk("need reerase block,page=%x\n",page);
		res = rtkn_jffs2_write_patch(mtd,chip,buf,oob_required,page);
		return res;
	}

	spin_lock_irqsave(&lock_nand, flags_nand);
#endif

#if defined(CONFIG_SPI_NAND_FLASH)
#ifdef RTK_BUILT_IN_ECC

	memset(rtkn->buf,0xff,MAX_RTKN_BUF_SIZE);
	memcpy(rtkn->buf,buf,chunk_size);
	if(oob_required){
		/* for spi nand, 4+2 alyout */
		memcpy(rtkn->buf+chunk_size,oobBuf,spare_step*6);
	}
	winbond_2KB_chunk_write_with_ecc_encode(rtkn->buf,page,tmpBuf);

	/* read to check if this chunk write failed */
	if(winbond_2KB_chunk_read_with_ecc_decode(rtkn->buf,page,tmpBuf) < 0){
		goto Error;
	}
#else
	memset(rtkn->buf,0xff,chunk_size+mtd->oobsize);
	memcpy(rtkn->buf,buf,chunk_size);
	if(oob_required)
		memcpy(rtkn->buf+chunk_size,oobBuf,mtd->oobsize);				//need check
	winbond_dma_write_data(rtkn->buf, chunk_size+mtd->oobsize, page);
#endif
#elif defined(CONFIG_PARALLEL_NAND_FLASH)
	memset(rtkn->buf,0xff,chunk_size+mtd->oobsize);
	memcpy(rtkn->buf,buf,chunk_size);
	if(oob_required){
		/* because spare area is 8 byte aligned */
		for(i = 0;i < spare_step;i++)
			memcpy(rtkn->buf+chunk_size+8*i,oobBuf+6*i,6);
	}

	if(parallel_nand_write_page(rtkn,rtkn->buf,rtkn->buf+chunk_size,page) < 0)
		goto Error;
#endif
	spin_unlock_irqrestore(&lock_nand, flags_nand);
	return 0;

Error:
#ifdef CONFIG_RTK_REMAP_BBT
    printk("[%s]:%d,write fail,page=%x\n",__func__,__LINE__,page);
    spin_unlock_irqrestore(&lock_nand, flags_nand);
    res = rtk_remapBBT_write_fail(mtd,page,buf,oob_required);
    return res;
#else
    spin_unlock_irqrestore(&lock_nand, flags_nand);
    printk("rtk_check_pageData return fail...\n");
    return -1;
#endif
}

static int rtkn_ecc_read_oob(struct mtd_info *mtd, struct nand_chip *chip,
				int page)
{
	struct rtknflash *rtkn = (struct rtknflash *)chip->priv;
	//pr_debug("page number is %d\n", page);
	/* clear the OOB buffer */
	memset(chip->oob_poi, ~0, mtd->oobsize);
	memset(rtkn->tmpBuf,~0,MAX_RTKN_BUF_SIZE);


#if LINUX_VERSION_CODE < KERNEL_VERSION(3,10,0)
	/* for 2.6.30 kernel */
	rtkn->curr_page_addr = page;
	rtkn_ecc_read_page(mtd,chip,rtkn->tmpBuf);
#else
	rtkn_ecc_read_page(mtd,chip,rtkn->tmpBuf,1,page);
#endif
	return 0;
}


static int rtkn_ecc_write_oob(struct mtd_info *mtd, struct nand_chip *chip, int page)
{
	struct rtknflash *rtkn = (struct rtknflash *)chip->priv;

	/*check if page is have written */
	memset(rtkn->tmpBuf,~0,MAX_RTKN_BUF_SIZE);
	rtkn->curr_page_addr = page;

	rtkn_ecc_write_page(mtd,chip,rtkn->tmpBuf,1);

	return 0;
}

static int rtknflash_wait(struct mtd_info *mtd, struct nand_chip *chip)
{
	struct rtknflash *rtkn = (struct rtknflash *)chip->priv;
	return rtkn->cmd_status;
}


#if !defined(__RTK_BOOT__)
static void rtknflash_ops_cmdfunc(struct mtd_info *mtd,
					      unsigned command, int column,
					      int page_addr)
{
	struct nand_chip *nand_chip = (struct nand_chip *)mtd->priv;
	struct rtknflash *rtkn = (struct rtknflash *)nand_chip->priv;
	/* donot need */
	//struct bcma_drv_cc *cc = rtkn->cc;
	//u32 ctlcode;
	//int i;

	if (column != -1)
		rtkn->curr_column = column;
	if (page_addr != -1)
		rtkn->curr_page_addr = page_addr;

	/* default val */
	rtkn->cmd_status = 0;

	switch (command) {
	case NAND_CMD_RESET:
		/* not support now */
		rtknflash_reset_cmd(rtkn);
		break;
	case NAND_CMD_READID:
		/* read nand chip id; store in id_data */
		rtknflash_read_id_cmd(rtkn);
		break;
	case NAND_CMD_STATUS:
		/* not support now */
		rtknflash_read_status_cmd(rtkn);
		break;
	#if 1
	case NAND_CMD_READ0:
		/* not support now */
		break;
	case NAND_CMD_READOOB:
		/* not support now */
		break;
	#endif
	case NAND_CMD_ERASE1:
		/* erase1 */
		if(rtknflash_erase1_cmd(mtd,rtkn) < 0)
			rtkn->cmd_status = NAND_STATUS_FAIL;
		break;
	case NAND_CMD_ERASE2:
		/* erase2 */
		break;
	#if 1
	case NAND_CMD_SEQIN:
		/* not support now*/
		break;
	case NAND_CMD_PAGEPROG:
		/* not support now */
		break;
	#endif
	default:
		pr_err("Command 0x%X unsupported\n", command);
		break;
	}
	rtkn->curr_command = command;
}


static u8 rtknflash_ops_read_byte(struct mtd_info *mtd)
{
	struct nand_chip *nand_chip = (struct nand_chip *)mtd->priv;
	struct rtknflash *rtkn = (struct rtknflash *)nand_chip->priv;
	//u32 tmp = 0;

	switch (rtkn->curr_command) {
	case NAND_CMD_READID:
		if (rtkn->curr_column >= ARRAY_SIZE(rtkn->id_data)) {
			pr_err("Requested invalid id_data: %d\n",
			       rtkn->curr_column);
			return 0;
		}
		return rtkn->id_data[rtkn->curr_column++];
	case NAND_CMD_STATUS:
		return rtkn->status;
#if 0
	case NAND_CMD_READOOB:
		rtknflash_ops_read(mtd, (u8 *)&tmp, 4);
		return tmp & 0xFF;
#endif
	}


	pr_err("Invalid command for byte read: 0x%X\n", rtkn->curr_command);
	return 0;
}
#endif

/**************************************************
 * Init
 **************************************************/
#if defined(CONFIG_SPI_NAND_FLASH)
#define RTK_BIT(x)				(1<<x)
#define REG_CLK_MANAGE2		0xB8000014
#define REG_PINMUX_06		0xB8000818
#define REG_PINMUX_15		0xB800081C

static int rtk_nand_register_init(void)
{
	/* set spi nand pin mux */
	REG32(REG_PINMUX_06) = 	(REG32(REG_PINMUX_06) & (~((15<<28) | (15 << 24)))) | ((3<<28) | (3 << 24));
	REG32(REG_PINMUX_15) = (REG32(REG_PINMUX_15) & (~((15<<28) | (15 << 24) | (15 << 16)))) | ((3<<28) | (3 << 24) | (3<<16));

	/*enable spi nand clock */
	if((REG32(REG_CLK_MANAGE2) & (RTK_BIT(19) | RTK_BIT(20))) != (RTK_BIT(19) | RTK_BIT(20))){
		printk("SPI NAND clock not enable\n");
		REG32(REG_CLK_MANAGE2) = REG32(REG_CLK_MANAGE2) | RTK_BIT(19) | RTK_BIT(20);
	}

	return 0;
}
#elif defined(CONFIG_PARALLEL_NAND_FLASH)

#define GPIO_PABCD_CNR 0xb8003500
#define GPIO_PABCD_DIR 0xb8003508
#define GPIO_PABCD_DAT 0xb800350c

static void WP_HIGH(void)
{
	REG_WRITE_U32(GPIO_PABCD_CNR, (REG_READ_U32(GPIO_PABCD_CNR) & 0xFFFFBFFF) | (0<<14)); //set GPIOB[6] as GPIO pin
	REG_WRITE_U32(GPIO_PABCD_DIR, (REG_READ_U32(GPIO_PABCD_DIR) & 0xFFFFBFFF) | (1<<14)); //set as output pin
	REG_WRITE_U32(GPIO_PABCD_DAT, (REG_READ_U32(GPIO_PABCD_DAT) & 0xFFFFBFFF) | (1<<14)); //set high
}

#if 0
static void WP_LOW(void)
{
	REG_WRITE_U32(GPIO_PABCD_CNR, (REG_READ_U32(GPIO_PABCD_CNR) & 0xFFFFBFFF) | (0<<14)); //set GPIOB[6] as GPIO pin
	REG_WRITE_U32(GPIO_PABCD_DIR, (REG_READ_U32(GPIO_PABCD_DIR) & 0xFFFFBFFF) | (1<<14)); //set as output pin
	REG_WRITE_U32(GPIO_PABCD_DAT, (REG_READ_U32(GPIO_PABCD_DAT) & 0xFFFFBFFF) | (0<<14)); //set low
}
#endif

static int rtk_nand_register_init(void)
{
	/* enable parallel nand clock */
	REG_WRITE_U32(REG_CLK_MANAGE2,((REG_READ_U32(REG_CLK_MANAGE2) & 0xFFBFFFFF) | (1<<21)));
	/* set pin mux */
	REG_WRITE_U32(REG_PIN_MUX00,REG_READ_U32(REG_PIN_MUX00) & (~0xffff0000));
	REG_WRITE_U32(REG_PIN_MUX01,REG_READ_U32(REG_PIN_MUX01) & (~0xffff0000));
	REG_WRITE_U32(REG_PIN_MUX02,REG_READ_U32(REG_PIN_MUX02) & (~0xffff000));
	REG_WRITE_U32(REG_PINMUX_15,REG_READ_U32(REG_PINMUX_15) & (~0xffff0000));

	REG_WRITE_U32(REG_PIN_MUX00,REG_READ_U32(REG_PIN_MUX00) | 0x21110000);
	REG_WRITE_U32(REG_PIN_MUX01,REG_READ_U32(REG_PIN_MUX01) | 0x12220000);
	REG_WRITE_U32(REG_PIN_MUX02,REG_READ_U32(REG_PIN_MUX02) | 0x1121000);
	REG_WRITE_U32(REG_PINMUX_15,REG_READ_U32(REG_PINMUX_15) | 0x44440000);

	REG_WRITE_U32(REG_SPR_DDR_CTL, NF_SPR_DDR_CTL_spare_ddr_ena(1) | NF_SPR_DDR_CTL_per_2k_spr_ena(1)| NF_SPR_DDR_CTL_spare_dram_sa(0));
	REG_WRITE_U32( REG_PD,~(0x1 << 0) );
	REG_WRITE_U32( REG_ND_CMD,0xFF);
	REG_WRITE_U32( REG_ND_CTL,(NF_ND_CTL_xfer(0x01) | NF_ND_CTL_tran_mode(0)) );

	WAIT_DONE(REG_ND_CTL,0x80,0);
	WAIT_DONE(REG_ND_CTL,0x40,0x40);

	REG_WRITE_U32( REG_MULTI_CHNL_MODE,0x00);
	REG_WRITE_U32( REG_TIME_PARA1,NF_TIME_PARA1_T1(0x01));
	REG_WRITE_U32( REG_TIME_PARA2,NF_TIME_PARA2_T2(0x01));
	REG_WRITE_U32( REG_TIME_PARA3,NF_TIME_PARA3_T3(0x01));

	REG_WRITE_U32( REG_DELAY_CTL,NF_DELAY_CTL_T_WHR_ADL(0x09) );

	REG_WRITE_U32( REG_MULTI_CHNL_MODE,NF_MULTI_CHNL_MODE_edo(0x00));
	REG_WRITE_U32( REG_ECC_STOP,NF_ECC_STOP_ecc_n_stop(0x01));

	REG_WRITE_U32( REG_ND_CA0,0);
	REG_WRITE_U32( REG_ND_CA1,0);

	/* enable wp */
	WP_HIGH();

	return 0;
}
#endif

int rtknflash_ops_init(struct rtknflash *rtkn)
{
	int err;

#if !defined(__RTK_BOOT__)
	/* basic */
	rtkn->nand_chip->select_chip 		= rtknflash_ops_select_chip;
	rtkn->nand_chip->cmdfunc 			= rtknflash_ops_cmdfunc;
	rtkn->nand_chip->waitfunc			= rtknflash_wait;

	rtkn->nand_chip->read_byte 			= rtknflash_ops_read_byte;
#if 0
	rtkn->nand_chip.read_buf 			= rtknflash_ops_read_buf;
	rtkn->nand_chip.write_buf 			= rtknflash_ops_write_buf;
#endif
	/* bbt */
#if defined(CONFIG_RTK_REMAP_BBT) || defined(CONFIG_RTK_NORMAL_BBT)
	rtkn->nand_chip->scan_bbt			= rtkn_scan_bbt;
	rtkn->nand_chip->block_bad			= rtkn_block_bad;
	rtkn->nand_chip->block_markbad		= rtkn_block_markbad;
#else
	/* use default bbt: not support */
	rtkn->nand_chip->bbt_options 		= NAND_BBT_USE_FLASH | NAND_BBT_NO_OOB;
#endif
	/* ecc */
	rtkn->nand_chip->ecc.read_page		= rtkn_ecc_read_page;
	rtkn->nand_chip->ecc.write_page		= rtkn_ecc_write_page;
	rtkn->nand_chip->ecc.read_oob		= rtkn_ecc_read_oob;
	rtkn->nand_chip->ecc.write_oob		= rtkn_ecc_write_oob;
	/* raw function */
	rtkn->nand_chip->ecc.read_page_raw	= rtkn_ecc_read_page;
	rtkn->nand_chip->ecc.write_page_raw	= rtkn_ecc_write_page;

	/* ecc mode */
	rtkn->nand_chip->ecc.mode			= NAND_ECC_HW;

	rtkn->nand_chip->ecc.size			= 1;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,0)
	rtkn->nand_chip->ecc.strength		= 8;
#endif

	/* 8k page */
#if CONFIG_MTD_NAND_RTK_PAGE_SIZE==8192
	rtkn->nand_chip->ecc.layout 		= &nand_bch_oob_256;
#endif
	/* 4k page */
#if CONFIG_MTD_NAND_RTK_PAGE_SIZE==4096
	rtkn->nand_chip->ecc.layout 		= &nand_bch_oob_128;
#endif
	/* 2k page */
#if CONFIG_MTD_NAND_RTK_PAGE_SIZE==2048
	rtkn->nand_chip->ecc.layout 		= &nand_bch_oob_64;
#endif

	/* chip option */
	rtkn->nand_chip->options			|= NAND_NO_SUBPAGE_WRITE;
	/* for 2.6.30 kernel used */
#if LINUX_VERSION_CODE < KERNEL_VERSION(3,10,0)
	rtkn->nand_chip->options			|= NAND_NO_AUTOINCR;
#endif
#endif

	/* Enable NAND flash access */
	err = rtk_nand_register_init();
	if(err){
		pr_err("Error; nand flash register init fail\n");
		goto exit;
	}

	/* Scan NAND */
	err = nand_scan(rtkn->mtd,1);
	if (err) {
		pr_err("Could not scan NAND flash: %d\n", err);
		goto exit;
	}

exit:
	return err;
}


/* nand info proc */
#if !(defined(__UBOOT__) || defined(__RTK_BOOT__))
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,0)
static int rtk_read_proc_nandinfo(struct seq_file *s, void *v)
{
	struct nand_chip	*this = (struct nand_chip *) rtkn->nand_chip;
	struct mtd_info		*mtd = (struct mtd_info *)rtkn->mtd;
	//int wlen = 0;

	//seq_printf(s,"nand_size:%u\n", this->device_size);
	seq_printf(s,"chip_size:%lld\n", this->chipsize);
	seq_printf(s,"block_size:%u\n", mtd->erasesize);
	seq_printf(s,"page_size:%u\n", mtd->writesize);
	seq_printf(s,"oob_size:%u\n", mtd->oobsize);
	seq_printf(s,"ppb:%u\n", mtd->erasesize/mtd->writesize);
#ifdef CONFIG_RTK_REMAP_BBT
	seq_printf(s,"RBA:%u\n", rtkn->RBA);
#endif
#ifdef CONFIG_RTK_NORMAL_BBT
	seq_printf(s,"bbt_num:%u\n", rtkn->bbt_num);
#endif
	seq_printf(s,"BBs:%u\n", rtkn->BBs);

	return 0;

}
#else
int rtk_read_proc_nandinfo(char *buf, char **start, off_t offset, int len, int *eof, void *data)
{
	struct nand_chip	*this = (struct nand_chip *) rtkn->nand_chip;
	struct mtd_info		*mtd = (struct mtd_info *)rtkn->mtd;
	int wlen = 0;

	//wlen += sprintf(buf+wlen,"nand_PartNum:%s\n", rtk_mtd->PartNum); //czyao
	//wlen += sprintf(buf+wlen,"nand_size:%u\n", this->device_size);
	wlen += sprintf(buf+wlen,"chip_size:%lld\n", this->chipsize);
	wlen += sprintf(buf+wlen,"block_size:%u\n", mtd->erasesize);
	wlen += sprintf(buf+wlen,"page_size:%u\n", mtd->writesize);
	wlen += sprintf(buf+wlen,"oob_size:%u\n", mtd->oobsize);
	wlen += sprintf(buf+wlen,"ppb:%u\n", mtd->erasesize/mtd->writesize);
#ifdef CONFIG_RTK_REMAP_BBT
	wlen += sprintf(buf+wlen,"RBA:%u\n", rtkn->RBA);
#endif
#ifdef CONFIG_RTK_NORMAL_BBT
	wlen += sprintf(buf+wlen,"bbt_num:%u\n", rtkn->bbt_num);
#endif
	wlen += sprintf(buf+wlen,"BBs:%u\n", rtkn->BBs);
	return wlen;
}
#endif

#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,0)
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
#endif


int  rtknflash_lowinit(struct mtd_info *mtd,struct nand_chip* nand)
{
	unsigned char* origbuf = NULL;
	int err = 0;

	/* nand */
	rtkn = kmalloc(sizeof(struct rtknflash),GFP_KERNEL);
	if(!rtkn)
	{
		err = -ENOMEM;
		goto  out1;
	}

	rtkn->buf = kmalloc(MAX_RTKN_BUF_SIZE+8,GFP_KERNEL);
	if(rtkn->buf == NULL){
		err = -ENOMEM;
		goto  out2;
	}

	origbuf = rtkn->buf;
	rtkn->buf = (unsigned char*)(((unsigned int)rtkn->buf+7) & ~7);		//8 size aligned


	/* nand */
	rtkn->nand_chip = nand;
	rtkn->mtd = mtd;
	nand->priv = rtkn;

#ifndef __RTK_BOOT__
	mtd->owner = THIS_MODULE;
#endif
	mtd->priv = nand;
	mtd->name = "rtk_nand";

	err = rtknflash_ops_init(rtkn);
	if (err) {
		printk("Initialization failed: %d\n", err);
		goto err_init;
	}

	/* create partition and nandinfo proc */
#if !(defined(__UBOOT__) || defined(__RTK_BOOT__))
#ifndef CONFIG_WRT_BARRIER_BREAKER
#if 0//def CONFIG_MTD_CMDLINE_PARTS
#if LINUX_VERSION_CODE < KERNEL_VERSION(3,10,0)
	int pnum = 0;
        struct mtd_partition *mtd_parts;

        pnum = parse_mtd_partitions (mtd, probes, &mtd_parts, 0);
        if (pnum <= 0) {
                printk(KERN_NOTICE "RTK: using the whole nand as a partitoin\n");
                if(add_mtd_device(mtd)) {
                        printk(KERN_WARNING "RTK: Failed to register new nand device\n");
                        err = EAGAIN;
			goto err_init;
                }
        }else{
                printk(KERN_NOTICE "RTK: using dynamic nand partition\n");
                if (add_mtd_partitions (mtd, mtd_parts, pnum)) {
                        printk("%s: Error, cannot add %s device\n",
                                        __FUNCTION__, mtd->name);
                        mtd->size = 0;
                        err = EAGAIN;
			goto err_init;
                }
        }
#else
	err = mtd_device_parse_register(mtd, probes, NULL, NULL, 0);
	if (err) {
		pr_err("Failed to register MTD device: %d\n", err);
		goto err_init;
	}
#endif
#else
	#if !defined(CONFIG_RTK_NAND_FLASH_STORAGE)
	#if !defined(CONFIG_MTD_RTKXX_PARTS)
	/* if need ???? */
	if(detect_rootfs_flash_map() < 0){
		printk("dynamic alloc partition fail\n");
		mtd->size = 0;
        err = EAGAIN;
		goto err_init;
	}
#if LINUX_VERSION_CODE < KERNEL_VERSION(3,10,0)
	if (add_mtd_partitions (mtd, rtl8196_parts1, ARRAY_SIZE(rtl8196_parts1))) {
        printk("%s: Error, cannot add %s device\n",
                        __FUNCTION__, mtd->name);
        mtd->size = 0;
        err = EAGAIN;
		goto err_init;
	}
#else
	if(mtd_device_parse_register(mtd,NULL,NULL,rtl8196_parts1,ARRAY_SIZE(rtl8196_parts1))){
		pr_err("%s: Error, cannot add %s device\n",
					__FUNCTION__, mtd->name);
		goto err_init;
	}
#endif
	#else
	// defined CONFIG_MTD_RTKXX_PARTS
	if(mtd_device_parse_register(mtd, probes, NULL, NULL, 0)){
		pr_err("%s: Error, cannot add %s device\n",
					__FUNCTION__, mtd->name);
		goto err_init;
	}
	#endif
	#else
	// defined(CONFIG_RTK_NAND_FLASH_STORAGE)
	#if LINUX_VERSION_CODE < KERNEL_VERSION(3,10,0)
	if (add_mtd_partitions (mtd, rtl8196_parts1, ARRAY_SIZE(rtl8196_parts1))) {
        printk("%s: Error, cannot add %s device\n",
                        __FUNCTION__, mtd->name);
        mtd->size = 0;
        err = EAGAIN;
		goto err_init;
	}
	#else
	if(mtd_device_parse_register(mtd,NULL,NULL,rtl8196_parts1,ARRAY_SIZE(rtl8196_parts1))){
		pr_err("%s: Error, cannot add %s device\n",
					__FUNCTION__, mtd->name);
		goto err_init;
	}
	#endif
	#endif
#endif
#else
	// openwrt use
	rtk_init_flash_proc();
	detect_nand_flash_map(mtd,rtl8196_parts1);
	if(mtd_device_parse_register(mtd,NULL,NULL,rtl8196_parts1,ARRAY_SIZE(rtl8196_parts1))){
		pr_err("%s: Error, cannot add %s device\n",
					__FUNCTION__, mtd->name);
		goto err_init;
	}
#endif
#endif


	/* nand info proc */
#if !(defined(__UBOOT__) || defined(__RTK_BOOT__))
#if LINUX_VERSION_CODE < KERNEL_VERSION(3,10,0)
	create_proc_read_entry("nandinfo", 0, NULL, rtk_read_proc_nandinfo, NULL);
#else
	proc_create("nandinfo", 0, NULL, &rtk_nand_flash_ops);
#endif
#endif

	return 0;

err_init:
	if(origbuf){
		kfree(origbuf);
		origbuf = NULL;
	}
out2:
	if(rtkn){
		kfree(rtkn);
		rtkn = NULL;
	}
out1:
	return err;
}


/* init function */
static struct nand_chip rtkn_nand_chip;
#ifdef __UBOOT__
void  board_nand_init(void)
{
	struct mtd_info *mtd = &nand_info[0];
	struct nand_chip *nand = &rtkn_nand_chip;

	if(rtknflash_lowinit(mtd,nand))
		hang();

	nand_register(0);
}
#else
struct mtd_info rtkn_mtd_info;
int board_nand_init(void)
{
	struct mtd_info *mtd = &rtkn_mtd_info;
	struct nand_chip *nand = &rtkn_nand_chip;

	if(rtknflash_lowinit(mtd,nand))
		return -1;
	return 0;
}
#endif
