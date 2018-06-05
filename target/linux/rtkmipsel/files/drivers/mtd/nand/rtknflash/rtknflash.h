#ifndef __BCM47XXNFLASH_H
#define __BCM47XXNFLASH_H

#include <linux/mtd/mtd.h>
#include <linux/mtd/nand.h>

/* only support 2048+64 page size 4 or 5cycle*/


/* default enable */
#define BOOT_BLOCK		2
#define SWAP_2K_DATA

#define rtk_readl(offset)         (*(volatile unsigned long *)(offset))
#define rtk_writel(val, offset)   (*(volatile unsigned long *)(offset) = val)

/********************************Realtek BBT**************************************/
#if defined(CONFIG_RTK_REMAP_BBT) || defined(CONFIG_RTK_NORMAL_BBT)
#ifdef CONFIG_RTK_REMAP_BBT
typedef struct  __attribute__ ((__packed__)){
    u16 BB_die;				//bad block chip
    u16 bad_block;			//bad block,from v2r bb
    u16 RB_die;				//remap block chip
    u16 remap_block;		//remap block, in RBA area
}BB_t;


typedef struct  __attribute__ ((__packed__)){
    u32 block_r;
}BB_v2r;
#endif

#ifdef CONFIG_RTK_NORMAL_BBT

typedef struct  __attribute__ ((__packed__)){
    u16 BB_die;
    u16 bad_block;
    u16 RB_die;
    u16 block_info;
}BBT_normal;
#endif


#define BACKUP_BBT 3
#define BOOT_SIZE	           		0x100000

#define	BB_INIT	0xFFFE				//bad_block init val
#define	RB_INIT	0xFFFD				//bad block flag in remap block //example 0~20 ok,21~30 bad
#define	BBT_TAG	0xBB
#define BB_DIE_INIT	0xEEEE			//bad block chip int val
#define RB_DIE_INIT	BB_DIE_INIT		//not used now

/* bbt location */
#if 0
#define REMAP_V2R_BBT_POS		0x100000	/*1M*/
#define REMAP_BBT_POS			0x200000	/*2M*/
#endif
#define REMAP_BBT_POS			0x100000	/*1M*/
#define NORMAL_BBT_POSITION     0x200000 	/*2M*/
#endif


/************************************************rtknflash*******************************************/
#define MAX_RTKN_BUF_SIZE		(8192+256)
struct rtknflash {
	struct nand_chip *nand_chip;
	struct mtd_info *mtd;

	unsigned curr_command;
	int curr_page_addr;
	int curr_column;

	/* read id cmd */
	u8 id_data[8];
	/* read cmd status */
	unsigned char	status;


#if defined(CONFIG_RTK_REMAP_BBT) || defined(CONFIG_RTK_NORMAL_BBT)
#ifdef CONFIG_RTK_REMAP_BBT
	BB_t 		*bbt;
	BB_v2r 		*bbt_v2r;
	unsigned int RBA;
	unsigned int RBA_PERCENT;
#endif
#ifdef CONFIG_RTK_NORMAL_BBT
	unsigned int bbt_num;
	unsigned int BBT_PERCENT;
	BBT_normal *bbt_nor;
#endif
	unsigned int BBs;
#endif
	unsigned char buf[MAX_RTKN_BUF_SIZE];

	int cmd_status;
};

//int rtknflash_ops_init(struct rtknflash *rtkn);

#ifdef __UBOOT__
void  board_nand_init(void);
#else
int  board_nand_init(void);
#endif
/* realtek bbt */
int rtkn_scan_bbt(struct mtd_info *mtd);
int rtkn_block_markbad(struct mtd_info *mtd, loff_t ofs);
int rtkn_block_bad(struct mtd_info *mtd, loff_t ofs, int getchip);
int rtkn_bbt_get_realpage(struct mtd_info *mtd,unsigned int page);

#ifdef CONFIG_RTK_REMAP_BBT
int rtk_remapBBT_read_fail(struct mtd_info* mtd,unsigned int page);
int rtk_remapBBT_erase_fail(struct mtd_info* mtd,unsigned int page);
int rtk_remapBBT_write_fail(struct mtd_info* mtd,unsigned int page,const uint8_t *buf,int oob_required);
#endif

/* ecc */
int rtk_check_pageData(struct mtd_info *mtd, u16 chipnr,int page, int offset, int isLastSector);

/* api */
int rtk_scan_write_bbt(struct mtd_info *mtd, int page, size_t len,
			  uint8_t *buf,uint8_t *oob);
int rtk_scan_read_oob(struct mtd_info *mtd, uint8_t *buf, int  page,
			 size_t len);
 int rtk_scan_erase_bbt(struct mtd_info *mtd, int  page);

/********************************************* register ************************************************/
#define NAND_CTRL_BASE  0xB801A000
/* NACFR */
#define NACFR  	(NAND_CTRL_BASE + 0x0)
/* NACR */
#define NACR    (NAND_CTRL_BASE + 0x04)
#define ECC_enable    	(1<<30)
#define RBO		     	(1<<29)
#define WBO		     	(1<<28)
/* NACMR */
#define NACMR   (NAND_CTRL_BASE + 0x08)
#define CECS1 			(1<<31)
#define CECS0			(1<<30)
/* NAADR */
#define NAADR   (NAND_CTRL_BASE + 0x0C)
#define enNextAD		(1<<27)
#define AD2EN			(1<<26)
#define AD1EN			(1<<25)
#define AD0EN			(1<<24)
#define CE_ADDR2		16
#define CE_ADDR1		8
#define CE_ADDR0		0
/* NADCRR */
#define NADCRR  (NAND_CTRL_BASE + 0x10)
#define TAG_SEL			7
#define TAG_DIS			(1<<6)
#define DESC1			(1<<5)
#define DESC0			(1<<4)
#define DMARE			(1<<3)
#define DMAWE			(1<<2)
#define LBC_128			3
#define LBC_64			2
#define LBC_32			1
#define LBC_16			0
/* NADR */
#define NADR	(NAND_CTRL_BASE + 0x14)
/* NADFSAR */
#define NADFSAR (NAND_CTRL_BASE + 0x18)
/* NADFSAR2 */
#define NADFSAR2 (NAND_CTRL_BASE + 0x1C)		//need check
/* NADRSAR && NASR && NADTSAR */
#ifndef CONFIG_RTL_8198C
#define NADRSAR 	(NAND_CTRL_BASE + 0x1C)
#define NASR        (NAND_CTRL_BASE + 0x20)
#define NADTSAR		(NAND_CTRL_BASE + 0x54)
#else
#define NADRSAR 	(NAND_CTRL_BASE + 0x20)
#define NASR        (NAND_CTRL_BASE + 0x28)
#define NADTSAR		(NAND_CTRL_BASE + 0x24)
#endif
#define NECN			(1<<4)
#define NRER			(1<<3)
#define NWER			(1<<2)
#define NDRS			(1<<1)
#define NDWS			(1<<0)


/*************************************************** NAND Flash Command Sets ***************************************/
#define CMD_READ_ID					0x90
#define CMD_READ_STATUS				0x70

#define CMD_PG_READ_C1				0x00
#define CMD_PG_READ_C2				0x30
#define CMD_PG_READ_C3				CMD_READ_STATUS

#define CMD_PG_READ_A				0x00
#define CMD_PG_READ_B				0x01
#define CMD_PG_READ_C				0x50

#define CMD_PG_WRITE_C1				0x80
#define CMD_PG_WRITE_C2				0x10
#define CMD_PG_WRITE_C3				CMD_READ_STATUS

#define CMD_BLK_ERASE_C1			0x60	//Auto Block Erase Setup command
#define CMD_BLK_ERASE_C2			0xd0	//CMD_ERASE_START
#define CMD_BLK_ERASE_C3			CMD_READ_STATUS	//CMD_STATUS_READ

#define CMD_RESET                 	0xff
#define CMD_RANDOM_DATA_INPUT     	0x85    /* RANDOM DATA write */

#define CMD_RANDOM_DATA_OUTPUT_C1 	0x05    /* RANDOM DATA read */
#define CMD_RANDOM_DATA_OUTPUT_C2 	0xe0

/* temp; may need modify:CACHELINE_SIZE */
#define CACHELINE_SIZE		32
#define M_mask				0xe0000000

/* cycle */
#define Boot_Select			0xB801a000
#define NAND_ADDR_MASK		(1<<31)
#ifndef CONFIG_RTL_8198C
#define NAND_ADDR_CYCLE	(((*(volatile unsigned int *)((Boot_Select)) & NAND_ADDR_MASK) == 0) ? 1:0)
#else
/* 98c is 4 or 5 cycle default */
#define NAND_ADDR_CYCLE	0
#endif

#ifdef SWAP_2K_DATA
#if  CONFIG_MTD_NAND_RTK_PAGE_SIZE==2048
#define DATA_BBI_OFF  ((512*4)- 48)	/*(0~1999)512+512+512+512-48*/
#define OOB_BBI_OFF   53			/* 16+16+16+5*/
#endif
#if CONFIG_MTD_NAND_RTK_PAGE_SIZE==4096
#define DATA_BBI_OFF	3984
#define OOB_BBI_OFF		((16*8) - 10 -1)
#endif
#endif

/* cache releate */
#ifdef __UBOOT__
#define dma_cache_inv(x,y)			invalidate_dcache_range(x,(x+y))
#define dma_cache_wback(x,y)		flush_dcache_range(x,(x+y))
#define dma_cache_wback_inv(x,y)	flush_dcache_range(x,(x+y))

#define spin_lock_irqsave(x,y)
#define spin_unlock_irqrestore(x,y)
#define printk(fmt,args...)	printf(fmt ,##args)

#endif

#define REG32(reg)   (*(volatile unsigned int *)((unsigned int)reg))

#ifdef NAND_PRINTF
#define nand_printf(fmt,args...)	printk(fmt ,##args)
#else
#define nand_printf(fmt,args...)
#endif

/* just for test */
//#define RTKN_FLASH_TEST
//#define RTKN_FLASH_TEST_WRITE
#if defined(RTKN_FLASH_TEST) || defined(RTKN_FLASH_TEST_WRITE)
#define RTKN_BLOCK_BAD	80
#endif

#endif /* BCM47XXNFLASH */
