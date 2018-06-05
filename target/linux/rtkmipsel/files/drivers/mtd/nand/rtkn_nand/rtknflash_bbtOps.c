#include "rtknflash.h"
#include <linux/kernel.h>
#ifndef __UBOOT__
#include <linux/slab.h>
#endif

static spinlock_t lock_nand;

#if 0
static void check_ready()
{
	while(1) {
		if(( rtk_readl(NACR) & 0x80000000) == 0x80000000)
			break;
	}
}
#endif

static int rtkn_ecc_read_page0(struct mtd_info *mtd, struct nand_chip *chip,
				uint8_t *buf,int oob_required, int page)
{
	struct rtknflash *rtkn = (struct rtknflash *)chip->priv;
	//uint8_t* oobBuf = chip->oob_poi;
	unsigned long flags_nand = 0;
	unsigned int chunk_size = mtd->writesize;
	int realpage = page;
	int spare_step = chunk_size/512;
	uint8_t* oobBuf = (uint8_t *)(buf+chunk_size);
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
#if 0//def CONFIG_RTK_REMAP_BBT
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
		memcpy(oobBuf,rtkn->buf+chunk_size,spare_step*6);
	}
#else
	winbond_dma_read_data(buf,chunk_size, realpage);
#endif
#elif defined(CONFIG_PARALLEL_NAND_FLASH)
	if(parallel_nand_read_page(rtkn,rtkn->buf,rtkn->buf+chunk_size,realpage) < 0)
		goto Error;
	memcpy(buf,rtkn->buf,chunk_size);
	if(oob_required){
		/* spare area 8 byte aligned */
		for(i = 0;i < spare_step;i++)
			memcpy(oobBuf+6*i,rtkn->buf+chunk_size+8*i,6);
	}
#endif

	spin_unlock_irqrestore(&lock_nand, flags_nand);
	return 0;

Error:
/* read function donot need do bbt */
	spin_unlock_irqrestore(&lock_nand, flags_nand);
	printk("rtk_check_pageData return fail...\n");
	return -1;
}


static int rtkn_ecc_write_page0(struct mtd_info *mtd, struct nand_chip *chip,
				const uint8_t *buf, uint8_t *oobBuf,int oob_required)
{
	struct rtknflash *rtkn = (struct rtknflash *)chip->priv;
	//unsigned char* oobBuf = chip->oob_poi;
	int page = rtkn->curr_page_addr;
	unsigned int chunk_size = mtd->writesize;
	unsigned long flags_nand = 0;
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
#if 0//def CONFIG_RTK_REMAP_BBT
    page = rtkn_bbt_get_realpage(mtd,page);
#endif

#if defined(CONFIG_SPI_NAND_FLASH)
#ifdef RTK_BUILT_IN_ECC

	memset(rtkn->buf,0xff,MAX_RTKN_BUF_SIZE);
	memcpy(rtkn->buf,buf,chunk_size);
	if(oob_required){
		memcpy(rtkn->buf+chunk_size,oobBuf,spare_step*6);
	}
	winbond_2KB_chunk_write_with_ecc_encode(rtkn->buf,page,tmpBuf);

	/* read to check if this chunk write failed */
	if(winbond_2KB_chunk_read_with_ecc_decode(rtkn->buf,page,tmpBuf) < 0){
		goto Error;
	}
#else
	winbond_dma_write_data(buf, chunk_size, page);
#endif
#elif defined(CONFIG_PARALLEL_NAND_FLASH)
	memset(rtkn->buf,0xff,chunk_size+mtd->oobsize);
	memcpy(rtkn->buf,buf,chunk_size);
	if(oob_required){
		/* spare area 8 byte aligned */
		for(i = 0;i < spare_step;i++)
			memcpy(rtkn->buf+chunk_size+8*i,oobBuf+6*i,6);
	}

	if(parallel_nand_write_page(rtkn,rtkn->buf,rtkn->buf+chunk_size,page) < 0)
		goto Error;
#endif

	spin_unlock_irqrestore(&lock_nand, flags_nand);
	return 0;

Error:
	spin_unlock_irqrestore(&lock_nand, flags_nand);
	printk("rtk_check_pageData return fail...\n");
	return -1;


}

/* scan erase bbt */
static int rtknflash_erase1_cmd0(struct mtd_info* mtd,struct rtknflash *rtkn)
{
	unsigned long flags_nand = 0;
	unsigned int page_addr = rtkn->curr_page_addr;
	spin_lock_irqsave(&lock_nand, flags_nand);

#if 0//def CONFIG_RTK_REMAP_BBT
	page_addr = rtkn_bbt_get_realpage(mtd,page_addr);
#endif

#if defined(CONFIG_SPI_NAND_FLASH)
	if(winbond_block_erase(page_addr) < 0)
		goto Error;
#elif defined(CONFIG_PARALLEL_NAND_FLASH)
	if(paralledl_nand_erase_cmd(page_addr) < 0)
		goto Error;
#endif

	spin_unlock_irqrestore(&lock_nand, flags_nand);
	return 0;

Error:
	spin_unlock_irqrestore(&lock_nand, flags_nand);
	//printk("[%s] erasure is not completed at block %d\n", __FUNCTION__, page/ppb);
	printk("erase error\n");
	return -1;
}

/*write data and oob */
 int rtk_scan_write_bbt(struct mtd_info *mtd, int page, size_t len,
			  uint8_t *buf,uint8_t *oob)
{
	struct nand_chip* this=(struct nand_chip*)mtd->priv;
	struct rtknflash* rtkn = (struct rtknflash*)this->priv;
	rtkn->curr_page_addr = page;

	//memcpy(this->oob_poi,oob,oobsize);

	return rtkn_ecc_write_page0(mtd,this,buf,oob,1);
}

/* read data and oob */
 int rtk_scan_read_oob(struct mtd_info *mtd, uint8_t *buf, int  page,
			 size_t len)
{
	int res;
	struct nand_chip* this = (struct nand_chip*)mtd->priv;

	res = rtkn_ecc_read_page0(mtd,this,buf,1,page);
	//memcpy(buf+page_size,this->oob_poi,oobsize);

	return res;
}

 int rtk_scan_erase_bbt(struct mtd_info *mtd, int  page)
{
	struct nand_chip* this = (struct nand_chip*)mtd->priv;
	struct rtknflash* rtkn = (struct rtknflash*)this->priv;
	rtkn->curr_page_addr = page;

	return rtknflash_erase1_cmd0(mtd,rtkn);
}