#include "rtknflash.h"
#include <linux/kernel.h>
#ifndef __UBOOT__
#include <linux/slab.h>
#endif

static spinlock_t lock_nand;

static void check_ready()
{
	while(1) {
		if(( rtk_readl(NACR) & 0x80000000) == 0x80000000)
			break;
	}
}

static int rtkn_ecc_read_page0(struct mtd_info *mtd, struct nand_chip *chip,
				uint8_t *buf, uint8_t* oobBuf,int oob_required, int page)
{
	nand_printf("[%s]:%d\n",__func__,__LINE__);
	struct rtknflash *rtkn = (struct rtknflash *)chip->priv;
	int dma_counter,page_shift,page_num[3],buf_pos=0;
	int dram_sa, oob_sa;
	unsigned long flash_addr_t=0,flash_addr2_t=0;
	unsigned int flags_nand = 0;
	//uint8_t* oobBuf = chip->oob_poi;
	//uint8_t* oobBuf = buf+mtd->writesize;
	int orig_block = page/(mtd->erasesize/mtd->writesize);

	int i;
	//printk("[%s]:%d,page=%x\n",__func__,__LINE__,page);
	spin_lock_irqsave(&lock_nand, flags_nand);


	//__flush_cache_all();
	/* addr */
	uint8_t *oob_area,*data_area,data_area0[512+16+CACHELINE_SIZE+CACHELINE_SIZE-4]; //data,oob point must 32 cache aligment
	memset(data_area0, 0xff, 512+16+CACHELINE_SIZE+CACHELINE_SIZE-4);
	data_area = (uint8_t*) ((uint32_t)(data_area0 + CACHELINE_SIZE-4) & 0xFFFFFFF0);
	oob_area=(uint8_t*) data_area+512;
	oob_sa =  ( (uint32_t)(oob_area ) & (~M_mask));
	dram_sa = ((uint32_t)data_area) & (~M_mask);

	/* flash addr */
	/* dma_counter and flash_addr_t */

	dma_counter = (mtd->writesize) >> 9;
	for(page_shift=0;page_shift<3; page_shift++) {
		page_num[page_shift] = ((page>>(8*page_shift)) & 0xff);
		if(mtd->writesize == 2048){
			flash_addr_t |= (page_num[page_shift] << (12+8*page_shift));
		}else if(mtd->writesize == 4096){
		  flash_addr_t |= (page_num[page_shift] << (13+8*page_shift));
		}else if(mtd->writesize == 512){
			flash_addr_t |= (page_num[page_shift] << (9+8*page_shift));
		}
	}

	#if 0
	for(page_shift=0;page_shift<3; page_shift++) {
	    page_num[page_shift] = ((page>>(8*page_shift)) & 0xff);
		flash_addr_t |= (page_num[page_shift] << (12+8*page_shift));
	}
	#endif
	flash_addr2_t= (page >> 20);


	rtk_writel(0xC00FFFFF, NACR);     //Enable ECC
	rtk_writel(0x0000000F, NASR);	  //clear NAND flash status register

	while(dma_counter>0){

		int lastSec = dma_counter-1;
		//set DMA RAM DATA start address
		rtk_writel(dram_sa, NADRSAR);
		//set DMA RAM oob start address , always use oob_sa buffer
		rtk_writel(oob_sa, NADTSAR);
		//set DMA flash start address,
		rtk_writel( flash_addr_t, NADFSAR);
		//set DMA flash start address2,
		//rtk_writel( flash_addr2_t, NADFSAR2);

		dma_cache_wback_inv((uint32_t *)data_area,528);
		//DMA read command
	    rtk_writel( ((~TAG_DIS)&(DESC0|DMARE|LBC_64)),NADCRR);
		check_ready();

		if(lastSec<=0)
			lastSec =1;
		else
			lastSec =0;

		if(rtk_check_pageData(mtd,0,page,buf_pos*(512+16),lastSec)==-1)
		{
		    goto Error;
		}

		//copy data
		memcpy(buf+(buf_pos*512), data_area, 512);
		//copy oob
		memcpy(oobBuf +(buf_pos*16), oob_area, 16);

		flash_addr_t += 528;
		dma_counter--;
		buf_pos++;
	}

	#ifdef SWAP_2K_DATA
	if(orig_block >= BOOT_BLOCK ){
		if(!NAND_ADDR_CYCLE)
		{
			/*switch bad block info*/
			unsigned char temp_val=0;
			{
				temp_val = buf[DATA_BBI_OFF];
				{
					buf[DATA_BBI_OFF] = oobBuf[OOB_BBI_OFF];
					oobBuf[OOB_BBI_OFF] = temp_val;
				}
			}
		}
	}
	#endif

	nand_printf("[%s]:%d\n",__func__,__LINE__);
	spin_unlock_irqrestore(&lock_nand, flags_nand);
	return 0;

Error:
/* read function donot need do bbt */
	spin_unlock_irqrestore(&lock_nand, flags_nand);
	printk("rtk_check_pageData return fail...\n");
	return -1;
}


static int rtkn_ecc_write_page0(struct mtd_info *mtd, struct nand_chip *chip,
				const uint8_t *buf, const uint8_t *oob,int oob_required)
{
	int i;
	struct rtknflash *rtkn = (struct rtknflash *)chip->priv;
	int page = rtkn->curr_page_addr,dma_counter,page_shift,page_num[3],buf_pos=0;
	int dram_sa, oob_sa;
	unsigned long flash_addr_t=0,flash_addr2_t=0;
	unsigned int flags_nand = 0;
	unsigned int ppb = mtd->erasesize/mtd->writesize;
	uint8_t* dataBuf = buf,*oobBuf = oob;
	int orig_block = page/ppb,res = 0;

	spin_lock_irqsave(&lock_nand, flags_nand);
	 //__flush_cache_all();

	/* addr */
	uint8_t *oob_area,*data_area,data_area0[512+16+CACHELINE_SIZE+CACHELINE_SIZE-4]; //data,oob point must 32 cache aligment
	memset(data_area0, 0xff, 512+16+CACHELINE_SIZE+CACHELINE_SIZE-4);
	data_area = (uint8_t*) ((uint32_t)(data_area0 + 12) & 0xFFFFFFF0);
	oob_area=(uint8_t*) data_area+512;
	oob_sa =  ( (uint32_t)(oob_area ) & (~M_mask));
	dram_sa = ((uint32_t)data_area) & (~M_mask);

	#ifdef SWAP_2K_DATA
	if(orig_block >= BOOT_BLOCK){
		if(!NAND_ADDR_CYCLE)
		{
			unsigned char temp_val;
			{
				temp_val = dataBuf[DATA_BBI_OFF];
				{
					dataBuf[DATA_BBI_OFF] = oobBuf[OOB_BBI_OFF];
					oobBuf[OOB_BBI_OFF] = temp_val;
				}
			}
		}
	}
	#endif

	/* flash addr */
	/* dma_counter and flash_addr_t */
	dma_counter = (mtd->writesize) >> 9;
	for(page_shift=0;page_shift<3; page_shift++) {
		page_num[page_shift] = ((page>>(8*page_shift)) & 0xff);
		if(mtd->writesize == 2048){
			flash_addr_t |= (page_num[page_shift] << (12+8*page_shift));
		}else if(mtd->writesize == 4096){
		  flash_addr_t |= (page_num[page_shift] << (13+8*page_shift));
		}else if(mtd->writesize == 512){
			flash_addr_t |= (page_num[page_shift] << (9+8*page_shift));
		}
	}

#if 0
	for(page_shift=0;page_shift<3; page_shift++) {
	    page_num[page_shift] = ((page>>(8*page_shift)) & 0xff);
		flash_addr_t |= (page_num[page_shift] << (12+8*page_shift));
	}
#endif

	/* if nand flash chip > 2G byte */
	flash_addr2_t= (page >> 20);

	rtk_writel(0xC00FFFFF, NACR);     //Enable ECC
	rtk_writel(0x0000000F, NASR);	  //clear NAND flash status register

	while(dma_counter>0){

		int lastSec = dma_counter - 1;
		memcpy(oob_area, oobBuf+(buf_pos*16), 16);
		memcpy(data_area, dataBuf+(buf_pos*512), 512);
		dma_cache_wback_inv((uint32_t *)data_area,528);//512+16
		rtk_writel( rtk_readl(NACR) & (~RBO) & (~WBO) , NACR);
		//write data/oob address
		rtk_writel(dram_sa, NADRSAR);
		rtk_writel(oob_sa, NADTSAR);
		rtk_writel(flash_addr_t, NADFSAR);
		//rtk_writel(flash_addr2_t, NADFSAR2);

		rtk_writel( (DESC0|DMAWE|LBC_64 & (~TAG_DIS)),NADCRR);
		check_ready();

		if(lastSec<=0)
			lastSec =1;
		else
			lastSec =0;

		if(rtk_check_pageData(mtd,0,page,buf_pos*(512+16),lastSec)==-1)
		{
			goto Error;

		}
		rtk_writel(0xF, NASR);

		flash_addr_t += (528); //512+16 one unit

		dma_counter--;
		buf_pos++;
	}
	#ifdef SWAP_2K_DATA
	if(orig_block >= BOOT_BLOCK){
		if(!NAND_ADDR_CYCLE)
		{
			unsigned char temp_val;
			{
				temp_val = dataBuf[DATA_BBI_OFF];
				{
					dataBuf[DATA_BBI_OFF] = oobBuf[OOB_BBI_OFF];
					oobBuf[OOB_BBI_OFF] = temp_val;
				}
			}
		}
	}
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
	int addr_cycle[5], page_shift,res = 0;
	int page_addr = rtkn->curr_page_addr;
	unsigned int flags_nand = 0;

	spin_lock_irqsave(&lock_nand, flags_nand);

	//printk("[%s]:%d,page=%x\n",__func__,__LINE__,page_addr);

	check_ready();
	rtk_writel( (rtk_readl(NACR) |ECC_enable), NACR);
	rtk_writel((NWER|NRER|NDRS|NDWS), NASR);
	rtk_writel(0x0, NACMR);
	rtk_writel((CECS0|CMD_BLK_ERASE_C1),NACMR);
	check_ready();
	addr_cycle[0] = addr_cycle[1] =0;
	for(page_shift=0; page_shift<3; page_shift++){
		addr_cycle[page_shift+2] = (page_addr>>(8*page_shift)) & 0xff;
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
		goto Error;
	}
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
	unsigned int oobsize = mtd->oobsize;
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
	unsigned int oobsize = mtd->oobsize,page_size = mtd->writesize;

	res = rtkn_ecc_read_page0(mtd,this,buf,buf+page_size,1,page);

	return res;
}

 int rtk_scan_erase_bbt(struct mtd_info *mtd, int  page)
{
	struct nand_chip* this = (struct nand_chip*)mtd->priv;
	struct rtknflash* rtkn = (struct rtknflash*)this->priv;
	rtkn->curr_page_addr = page;

	return rtknflash_erase1_cmd0(mtd,rtkn);
}