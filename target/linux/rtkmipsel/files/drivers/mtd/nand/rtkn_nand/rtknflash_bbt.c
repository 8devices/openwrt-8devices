#include "rtknflash.h"
#include <linux/kernel.h>
#ifndef __UBOOT__
#include <linux/slab.h>
#endif

/* realtek bbt */
/*	1. first remap v2r bbt: 	block ----> v2r_block
	2. second create rba bbt:	v2r_block ----> rba_block
	3. mark block bad function.
		a.mark v2r_block bad (oob write)
		b.update rba bbt
	4. recreate bbt function
		a. v2r bbt and rba bbt
			only based on  oob flag
	5. mtd check if block is bad
		a. always return 0(not bad) because of remap bbt
	6. for pagesize > 512 nand flash, badblock pos is 5,others is 0
*/

#ifdef CONFIG_RTK_REMAP_BBT
static int RBA;
#if 0
static int read_has_check_bbt = 0;
static unsigned int read_block = 0XFFFFFFFF;
static unsigned int read_remap_block = 0XFFFFFFFF;
static int write_has_check_bbt = 0;
static unsigned int write_block = 0XFFFFFFFF;
static unsigned int write_remap_block = 0XFFFFFFFF;
#endif
#endif
#ifdef CONFIG_RTK_NORMAL_BBT
static int bbt_num;
#endif
/*
	 * Bad block marker is stored in the last page of each block on Samsung
	 * and Hynix MLC devices; stored in first two pages of each block on
	 * Micron devices with 2KiB pages and on SLC Samsung, Hynix, Toshiba,
	 * AMD/Spansion, and Macronix.  All others scan only the first page.
	default store in first and second page and read in first page
*/

/* copy from nand_bbt.c */
/* scan read bbt */

/*  buf = data + oob */
static int rtk_block_isbad(struct mtd_info *mtd, u16 chipnr, loff_t ofs)
{
	struct nand_chip *this = (struct nand_chip *)mtd->priv;
	struct rtknflash *rtkn = (struct rtknflash *)this->priv;
	unsigned int page, block, page_offset;
	unsigned char block_status_p1;
	int isLastPage = rtkn->chip_param.isLastPage;

	unsigned int page_size = mtd->writesize;
	//unsigned int oob_size = mtd->oobsize;
	unsigned int ppb = mtd->erasesize/mtd->writesize;

	page = ((int) ofs) >> this->page_shift;
	page_offset = page & (ppb-1);
	block = page/ppb;

	if ( isLastPage ){
		page = block*ppb + (ppb-1);
		if(rtk_scan_read_oob(mtd,rtkn->tmpBuf,page,page_size)){
			printk ("%s: read_oob page=%d failed\n", __FUNCTION__, page);
			return 1;
		}
		#ifdef SWAP_2K_DATA
			block_status_p1 = rtkn->tmpBuf[page_size+OOB_BBI_OFF];
		#else
			block_status_p1 = rtkn->tmpBuf[page_size];
		#endif

	}else{
		if(rtk_scan_read_oob(mtd,rtkn->tmpBuf,page,page_size)){
			printk ("%s: read_oob page=%d failed\n", __FUNCTION__, page);
			return 1;
		}

		#ifdef SWAP_2K_DATA
			block_status_p1 = rtkn->tmpBuf[page_size+OOB_BBI_OFF];
		#else
			block_status_p1 = rtkn->tmpBuf[page_size];
		#endif

	}
#if defined(CONFIG_RTK_REMAP_BBT) || defined(CONFIG_RTK_NORMAL_BBT)
    if ( block_status_p1 == BBT_TAG){
        printk ("INFO: Stored BBT in Die %d: block=%d , block_status_p1=0x%x\n", chipnr, block, block_status_p1);
    }else
#endif
    if ( block_status_p1 != 0xff){
		printk ("WARNING: Die %d: block=%d is bad, block_status_p1=0x%x\n", chipnr, block, block_status_p1);
		return -1;
	}
	return 0;
}

/***********************************REMAP BBT********************************/
#ifdef CONFIG_RTK_REMAP_BBT

int rtkn_bbt_get_realpage(struct mtd_info *mtd,unsigned int page)
{
	struct nand_chip* this = (struct nand_chip*)mtd->priv;
	struct rtknflash *rtkn = (struct rtknflash *)this->priv;

	int i;
	unsigned int ppb = mtd->erasesize/mtd->writesize;
	unsigned int block,realpage,realblock,page_offset;

	page_offset =page % ppb;
	block = page/ppb;
	realblock = rtkn->bbt_v2r[block].block_r;

	for ( i=0; i<RBA; i++){
		if ( rtkn->bbt[i].bad_block != BB_INIT ){
			if(realblock == rtkn->bbt[i].bad_block ){
				//read_remap_block = realblock = rtkn->bbt[i].remap_block;
				realblock = rtkn->bbt[i].remap_block;
			}
		}else
			break;
	}

	realpage = realblock*ppb + page_offset;
	//page = block*ppb + page_offset;

	return realpage;

}

static void dump_BBT(struct mtd_info *mtd)
{
	struct nand_chip *this = (struct nand_chip *) mtd->priv;
	struct rtknflash *rtkn = (struct rtknflash *) this->priv;
	int i;
	int BBs=0;
	printk("[%s] Nand BBT Content\n", __FUNCTION__);
	for ( i=0; i<RBA; i++){
		if ( i==0 &&  rtkn ->bbt[i].bad_block == BB_INIT ){
			printk("Congratulation!! No BBs in this Nand.\n");
			break;
		}

		if ( rtkn ->bbt[i].bad_block != BB_INIT ){
			printk("[%d] (%x, %u, %x, %u)\n", i, rtkn ->bbt[i].BB_die, rtkn ->bbt[i].bad_block,
				rtkn ->bbt[i].RB_die, rtkn ->bbt[i].remap_block);
			BBs++;
		}
#if 0
		else {
			printk("[%d] (%d, %u, %d, %u)\n", i, this->bbt[i].BB_die, this->bbt[i].bad_block,
				this->bbt[i].RB_die, this->bbt[i].remap_block);
		}
#endif
	}
	/* how much BBs */
	rtkn->BBs = BBs;
}

static int check_BBT(struct mtd_info *mtd, unsigned int blk)
{
	int i;
	struct nand_chip *this = (struct nand_chip *) mtd->priv;
	struct rtknflash *rtkn = (struct rtknflash *) this->priv;
	printk("[%s] blk %x\n", __FUNCTION__, blk);

	for ( i=0; i<RBA; i++)
	{
	    if ( rtkn->bbt[i].bad_block == blk )
        {
            printk("blk 0x%x already exist\n",blk);
		    return -1;
	    }
	}
	return 0;
}

static int scan_last_die_BB(struct mtd_info *mtd)
{
	struct nand_chip *this = (struct nand_chip *)mtd->priv;
	struct rtknflash *rtkn = (struct rtknflash *)this->priv;
	//__u32 start_page;
	__u32 addr;
	int block_num = (1<<this->chip_shift)/(1<<this->phys_erase_shift);
	int block_size = 1 << this->phys_erase_shift;
	int table_index=0;
	int remap_block[RBA];
	int remap_count = 0;
	int i, j;
	int numchips = this->numchips;
	int chip_size = this->chipsize;
	int rc = 0;

	//this->active_chip = numchips-1;
	//this->select_chip(mtd, numchips-1);

	__u8 *block_status = kmalloc( block_num, GFP_KERNEL );
	if ( !block_status ){
		printk("%s: Error, no enough memory for block_status\n",__FUNCTION__);
		rc = -ENOMEM;
		goto EXIT;
	}
	memset ( (__u32 *)block_status, 0, block_num );

	//printk("[%s, line %d] this->numchips = %d\n",__FUNCTION__,__LINE__,this->numchips);
	for( addr=0; addr<chip_size; addr+=block_size ){
		if ( rtk_block_isbad(mtd, numchips-1, addr) ){
			int bb = addr >> this->phys_erase_shift;
			block_status[bb] = 0xff;
		}
	}

	//printk("[%s, line %d] this->numchips = %d\n",__FUNCTION__,__LINE__,this->numchips);
	for ( i=0; i<RBA; i++){
		if ( block_status[(block_num-1)-i] == 0x00){
			remap_block[remap_count] = (block_num-1)-i;
			remap_count++;
		}
	}

	/* if remap block is RB_INIT,remap block is bad block */
	if (remap_count<RBA+1){
		for (j=remap_count+1; j<RBA+1; j++){
			//printk("[j=%d]\n",j);
			remap_block[j-1] = RB_INIT;
		}
	}
	//printk("[%s, line %d] this->numchips = %d\n",__FUNCTION__,__LINE__,this->numchips);
	for( i=table_index; table_index<RBA; table_index++){
		rtkn->bbt[table_index].bad_block = BB_INIT;
		/* only support one chip now */
		rtkn->bbt[table_index].BB_die = numchips-1;
		#if 0
		rtkn->bbt[table_index].BB_die = BB_DIE_INIT;
		#endif
		rtkn->bbt[table_index].remap_block = remap_block[table_index];
		rtkn->bbt[table_index].RB_die = numchips-1;
	}



	//printk("[%s, line %d] this->numchips = %d\n",__FUNCTION__,__LINE__,this->numchips);
	kfree(block_status);

	//printk("[%s, line %d] this->numchips = %d\n",__FUNCTION__,__LINE__,this->numchips);
	dma_cache_wback((unsigned long) rtkn->bbt,sizeof(BB_t)*RBA);   //czyao

EXIT:
	if (rc){
		if (block_status)
			kfree(block_status);
	}

	return 0;
}

static int rtk_create_bbt(struct mtd_info *mtd, int page)
{
	//printk("[%s] nand driver creates B%d !!\n", __FUNCTION__, page ==0?0:1);
	struct nand_chip *this = (struct nand_chip *)mtd->priv;
	struct rtknflash *rtkn = (struct rtknflash *)this->priv;
	int isLastPage = rtkn->chip_param.isLastPage;

	unsigned int page_size = mtd->writesize;
	int rc = 0;
	u8 *temp_BBT = 0;
	u8 mem_page_num, page_counter=0;

	unsigned int ppb = (1<<this->phys_erase_shift)/(1<<this->page_shift);
	#if 1
	//printk("[%s, line %d] this->numchips = %d\n",__FUNCTION__,__LINE__,this->numchips);
	if ( scan_last_die_BB(mtd) ){
		printk("[%s] scan_last_die_BB() error !!\n", __FUNCTION__);
		return -1;
	}
	#endif
	//printk("[%s, line %d]\n",__FUNCTION__,__LINE__);
	mem_page_num = (sizeof(BB_t)*RBA + page_size-1 )/page_size;
	temp_BBT = kmalloc( mem_page_num*page_size, GFP_KERNEL );
	if ( !temp_BBT ){
		printk("%s: Error, no enough memory for temp_BBT\n",__FUNCTION__);
		return -ENOMEM;
	}
	//printk("[%s, line %d]\n",__FUNCTION__,__LINE__);
	memset( temp_BBT, 0xff, mem_page_num*page_size);

	//this->select_chip(mtd, 0);

	if(rtk_scan_erase_bbt(mtd,page)){
		//printk("[%s]erase block %d failure !!\n", __FUNCTION__, page/ppb);
		rc =  -1;

		#ifdef SWAP_2K_DATA
			memset(rtkn->tmpBuf,0xff,MAX_RTKN_BUF_SIZE);
			rtkn->tmpBuf[page_size+OOB_BBI_OFF] = 0x00;
		#else
			memset(rtkn->tmpBuf,0xff,MAX_RTKN_BUF_SIZE);
			rtkn->tmpBuf[page_size] = 0x00;
		#endif

		//mark as bad block;
		if (isLastPage){
			rtk_scan_write_bbt(mtd,(page+ppb-1),page_size,temp_BBT,&rtkn->tmpBuf[page_size]);
			rtk_scan_write_bbt(mtd,(page+ppb-2),page_size,temp_BBT,&rtkn->tmpBuf[page_size]);
		}else{
			rtk_scan_write_bbt(mtd,(page),page_size,temp_BBT,&rtkn->tmpBuf[page_size]);
			rtk_scan_write_bbt(mtd,(page+1),page_size,temp_BBT,&rtkn->tmpBuf[page_size]);
		}
		goto EXIT;
	}

	#ifdef SWAP_2K_DATA
		memset(rtkn->tmpBuf,0xff,MAX_RTKN_BUF_SIZE);
		rtkn->tmpBuf[page_size+OOB_BBI_OFF] = BBT_TAG;
	#else
		memset(rtkn->tmpBuf,0xff,MAX_RTKN_BUF_SIZE);
		rtkn->tmpBuf[page_size] = BBT_TAG;
	#endif
	memcpy( temp_BBT, rtkn->bbt, sizeof(BB_t)*RBA );

	while( mem_page_num>0 ){
		if(rtk_scan_write_bbt(mtd,(page+page_counter),page_size,temp_BBT+page_counter*page_size,&rtkn->tmpBuf[page_size])){
			printk("[%s] write BBT B%d page %d failure!!\n", __FUNCTION__, page ==0?0:1, page+page_counter);
			rc =  -1;
			rtk_scan_erase_bbt(mtd,page);

			#ifdef SWAP_2K_DATA
				memset(rtkn->tmpBuf,0xff,MAX_RTKN_BUF_SIZE);
				rtkn->tmpBuf[page_size+OOB_BBI_OFF] = 0x00;
			#else
				memset(rtkn->tmpBuf,0xff,MAX_RTKN_BUF_SIZE);
				rtkn->tmpBuf[page_size] = 0x00;
			#endif
			//mark as bad block;
			if ( isLastPage){
				rtk_scan_write_bbt(mtd,(page+ppb-1),page_size,temp_BBT,&rtkn->tmpBuf[page_size]);
				rtk_scan_write_bbt(mtd,(page+ppb-2),page_size,temp_BBT,&rtkn->tmpBuf[page_size]);
			}else{
				rtk_scan_write_bbt(mtd,(page),page_size,temp_BBT,&rtkn->tmpBuf[page_size]);
				rtk_scan_write_bbt(mtd,(page+1),page_size,temp_BBT,&rtkn->tmpBuf[page_size]);
			}
			goto EXIT;
		}
		page_counter++;
		mem_page_num--;
	}

EXIT:
	if (temp_BBT)
		kfree(temp_BBT);

	return rc;
}

static int rtk_nand_scan_bbt(struct mtd_info *mtd)
{
	struct nand_chip *this = (struct nand_chip *)mtd->priv;
	struct rtknflash *rtkn = (struct rtknflash *)this->priv;
	int isLastPage = rtkn->chip_param.isLastPage;
	int rc = 0, i;
	__u8 isbbt;
	u8 *temp_BBT=0;
	u8 mem_page_num, page_counter=0, mem_page_num_tmp=0;
	u8 load_bbt_error=0,is_first_boot=1, error_count=0;
	int numchips = this->numchips;
	unsigned int bbt_page;
	unsigned int ppb = (1<<this->phys_erase_shift)/(1<<this->page_shift);
	unsigned int page_size = (1<<this->page_shift);
	unsigned int block_size = (1<<this->phys_erase_shift);

	nand_printf("[%s]:%d\n",__func__,__LINE__);

	//__u8 check0, check1, check2, check3;
	dma_cache_wback((unsigned long) rtkn->bbt,sizeof(BB_t)*RBA);   //czyao

	//bbt_page = ((BOOT_SIZE/block_size)-BACKUP_BBT)*ppb;
	bbt_page = ((REMAP_BBT_POS/block_size)+BACKUP_BBT)*ppb;
	mem_page_num = (sizeof(BB_t)*RBA + page_size-1 )/page_size;
	/* check */
	if(mem_page_num > ppb)
	{
		printk("remap bbt data large than one block,block=%d\n",mem_page_num);
		return -1;
	}

	printk("[%s, line %d] mem_page_num=%d bbt_page %d\n",__FUNCTION__,__LINE__,mem_page_num, bbt_page);

	temp_BBT = kmalloc( mem_page_num*page_size, GFP_KERNEL );
	if ( !temp_BBT ){
		printk("%s: Error, no enough memory for temp_BBT\n",__FUNCTION__);
		return -ENOMEM;
	}

	memset( temp_BBT, 0xff, mem_page_num*page_size);
	//NEW method!
	for(i=0;i<BACKUP_BBT;i++){
		rc = rtk_scan_read_oob(mtd,rtkn->tmpBuf,(bbt_page+(i*ppb)),page_size);
		#ifdef SWAP_2K_DATA
			isbbt = rtkn->tmpBuf[page_size+OOB_BBI_OFF];
		#else
			isbbt = rtkn->tmpBuf[page_size];
		#endif

		if(!rc){
		    if(isbbt==BBT_TAG)//bbt has already created
				is_first_boot = 0;
		}
	}
	//printk("%s: is_first_boot:%d\n\r",__FUNCTION__, is_first_boot);
	//is_first_boot = 1;

#if 1
	for(i=0;i<BACKUP_BBT;i++){
		mem_page_num_tmp = mem_page_num;
		page_counter=0;load_bbt_error=0;
		rc = rtk_block_isbad(mtd,numchips-1,(bbt_page+(i*ppb))*page_size);
		if(!rc){
			printk("load bbt table:%d page:%d\n\r",i, (bbt_page+(i*ppb)));
		    //rc = rtk_read_ecc_page(bbt_page+(i*ppb), &NfDataBuf, &NfSpareBuf,page_size);
			rc = rtk_scan_read_oob(mtd,rtkn->tmpBuf,(bbt_page+(i*ppb)),page_size);

			#ifdef SWAP_2K_DATA
				isbbt = rtkn->tmpBuf[page_size+OOB_BBI_OFF];
			#else
				isbbt = rtkn->tmpBuf[page_size];
			#endif

	        if(!rc){
			#if 1
			    if(isbbt == BBT_TAG){
			    //if(0){
			        printk("[%s] have created bbt table:%d on block %d, just loads it !!\n\r", __FUNCTION__,i,(bbt_page/ppb)+i);
			        //memcpy( temp_BBT, &NfDataBuf, page_size );
					memcpy( temp_BBT, rtkn->tmpBuf, page_size );
			        page_counter++;
			        mem_page_num_tmp--;
			        while( mem_page_num_tmp>0 ){
					#if 1
				        if(rtk_scan_read_oob(mtd,rtkn->tmpBuf,(bbt_page+(i*ppb)+page_counter),page_size)){
					        printk("[%s] load bbt table%d error!!\n\r", __FUNCTION__,i);
					        rtk_scan_erase_bbt(mtd,(bbt_page+(ppb*i)));
							#ifdef SWAP_2K_DATA
								memset(rtkn->tmpBuf,0xff,MAX_RTKN_BUF_SIZE);
								rtkn->tmpBuf[page_size+OOB_BBI_OFF] = 0x00;
							#else
								memset(rtkn->tmpBuf,0xff,MAX_RTKN_BUF_SIZE);
								rtkn->tmpBuf[page_size] = 0x00;
							#endif

							if ( isLastPage){
								rtk_scan_write_bbt(mtd,(bbt_page+(ppb*i)+(ppb-1)),page_size,temp_BBT,&rtkn->tmpBuf[page_size]);
								rtk_scan_write_bbt(mtd,(bbt_page+(ppb*i)+(ppb-2)),page_size,temp_BBT,&rtkn->tmpBuf[page_size]);
							}else{
								rtk_scan_write_bbt(mtd,(bbt_page+(ppb*i)),page_size,temp_BBT,&rtkn->tmpBuf[page_size]);
								rtk_scan_write_bbt(mtd,(bbt_page+(ppb*i)+1),page_size,temp_BBT,&rtkn->tmpBuf[page_size]);
							}
                            load_bbt_error=1;
			                error_count++;
					        break;
				        }
				       #endif
						#if 1
					#ifdef SWAP_2K_DATA
							isbbt = rtkn->tmpBuf[page_size+OOB_BBI_OFF];
						#else
							isbbt = rtkn->tmpBuf[page_size];
						#endif
						if(isbbt == BBT_TAG){//check bb tag in each page!
							memcpy( temp_BBT+page_counter*page_size, rtkn->tmpBuf, page_size );
						page_counter++;
						mem_page_num_tmp--;
						}else{
                            load_bbt_error=1;
			                error_count++;
					        printk("[%s] check bbt table%d tag:0x%x fail!!\n\r", __FUNCTION__,i,isbbt);
							break;
						}
						#endif
			        }
					if(!load_bbt_error){
					    memcpy( rtkn->bbt, temp_BBT, sizeof(BB_t)*RBA );
					    printk("check bbt table:%d OK\n\r",i);
					    goto CHECK_BBT_OK;
					}
			    }else{
					if(is_first_boot){
					    printk("Create bbt table:%d is_first_boot:%d\n\r",i, is_first_boot);
				        rtk_create_bbt(mtd, bbt_page+(i*ppb));
					}
			    }
			    #endif
		    }else{
				printk("read bbt table:%d page:%d\n\r",i, (bbt_page+(i*ppb)));
				rtk_scan_erase_bbt(mtd,(bbt_page+(ppb*i)));

				#ifdef SWAP_2K_DATA
					memset(rtkn->tmpBuf,0xff,MAX_RTKN_BUF_SIZE);
					rtkn->tmpBuf[page_size+OOB_BBI_OFF] = 0x00;
				#else
					memset(rtkn->tmpBuf,0xff,MAX_RTKN_BUF_SIZE);
					rtkn->tmpBuf[page_size] = 0x00;
				#endif

				if ( isLastPage){
					rtk_scan_write_bbt(mtd,(bbt_page+(ppb*i)+(ppb-1)),page_size,temp_BBT,&rtkn->tmpBuf[page_size]);
					rtk_scan_write_bbt(mtd,(bbt_page+(ppb*i)+(ppb-2)),page_size,temp_BBT,&rtkn->tmpBuf[page_size]);
				}else{
					rtk_scan_write_bbt(mtd,(bbt_page+(ppb*i)),page_size,temp_BBT,&rtkn->tmpBuf[page_size]);
					rtk_scan_write_bbt(mtd,(bbt_page+(ppb*i)+1),page_size,temp_BBT,&rtkn->tmpBuf[page_size]);
				}
		    }
		}else{
            printk("bbt table:%d block:%d page:%d is bad\n\r",i,(bbt_page/ppb)+i,bbt_page+(i*ppb));
			error_count++;
		}
	}
#endif
CHECK_BBT_OK:
	dump_BBT(mtd);

	if (temp_BBT)
		kfree(temp_BBT);
    if(error_count >= BACKUP_BBT){
        rc = -1;
		printk("%d bbt table are all bad!(T______T)\n\r", BACKUP_BBT);
	}

	nand_printf("[%s]:%d\n",__func__,__LINE__);

	return rc;
}


static int create_v2r_remapping(struct mtd_info *mtd, unsigned int page, unsigned int block_v2r_num)
{
	struct nand_chip *this = (struct nand_chip *)mtd->priv;
	struct rtknflash *rtkn = (struct rtknflash *)this->priv;
	int isLastPage = rtkn->chip_param.isLastPage;
	unsigned int offs=0, offs_real=0;
	unsigned char mem_page_num, page_counter=0;
	unsigned char *temp_BBT = 0;
	unsigned int ppb;
    int rc=0;
	int numchips = this->numchips;
	unsigned int search_region=0, count=0;
	unsigned int block_size = (1 << this->phys_erase_shift);
	unsigned int page_size = (1 << this->page_shift);
	count = 0;
	search_region = (block_v2r_num << this->phys_erase_shift);
	ppb = (1 << this->phys_erase_shift)/(1 << this->page_shift);

	/* need modify */
    while(offs_real < search_region){
		if ( rtk_block_isbad(mtd,numchips-1,offs_real) ){
			offs_real += block_size;
		}else{
		//this->bbt_v2r[count].block_v = (offs >> this->phys_erase_shift);
		rtkn->bbt_v2r[count].block_r = (offs_real >> this->phys_erase_shift);
			offs+=block_size;
			offs_real += block_size;
			//printk("bbt_v2r[%d].block_v %d,  bbt_v2r[%d].block_r %d\n",count,bbt_v2r[count].block_v,count,bbt_v2r[count].block_r);
			count++;
		}
	}
	//printk("[%s, line %d] block_v2r_num %d\n\r",__FUNCTION__,__LINE__, block_v2r_num);

	mem_page_num = ((sizeof(BB_v2r)*block_v2r_num) + page_size-1 )/page_size;
	//printk("[%s, line %d] mem_page_num = %d\n\r",__FUNCTION__,__LINE__,mem_page_num);

	//temp_BBT = (unsigned char *) malloc( mem_page_num*page_size);
	temp_BBT = (unsigned char *)kmalloc( mem_page_num*page_size, GFP_KERNEL );
	if ( !temp_BBT ){
		printk("%s: Error, no enough memory for temp_BBT v2r\n\r",__FUNCTION__);
		rc = -1;
		goto EXIT_V2R;
	}
	memset( temp_BBT, 0xff, mem_page_num*page_size);


	if(rtk_scan_erase_bbt(mtd,page)){
		//printk("[%s]erase block %d failure !!\n\r", __FUNCTION__, page/this->ppb);
		rc =  -1;
		#ifdef SWAP_2K_DATA
			memset(rtkn->tmpBuf,0xff,MAX_RTKN_BUF_SIZE);
			rtkn->tmpBuf[page_size+OOB_BBI_OFF] = 0x00;
		#else
			memset(rtkn->tmpBuf,0xff,MAX_RTKN_BUF_SIZE);
			rtkn->tmpBuf[page_size] = 0x00;
		#endif


		//mark as bad block;
		if ( isLastPage){
			rtk_scan_write_bbt(mtd,(page+ppb-1),page_size,rtkn->tmpBuf,&rtkn->tmpBuf[page_size]);
			rtk_scan_write_bbt(mtd,(page+ppb-2),page_size,rtkn->tmpBuf,&rtkn->tmpBuf[page_size]);
		}else{
			rtk_scan_write_bbt(mtd,(page),page_size,rtkn->tmpBuf,&rtkn->tmpBuf[page_size]);
			rtk_scan_write_bbt(mtd,(page+1),page_size,rtkn->tmpBuf,&rtkn->tmpBuf[page_size]);
		}
		goto EXIT_V2R;
	}

	#ifdef SWAP_2K_DATA
		memset(rtkn->tmpBuf,0xff,MAX_RTKN_BUF_SIZE);
		rtkn->tmpBuf[page_size+OOB_BBI_OFF] = BBT_TAG;
	#else
		memset(rtkn->tmpBuf,0xff,MAX_RTKN_BUF_SIZE);
		rtkn->tmpBuf[page_size] = BBT_TAG;
	#endif


	memcpy( temp_BBT, rtkn->bbt_v2r, sizeof(BB_v2r)*block_v2r_num );
	//dump_mem((unsigned int)temp_BBT,512);
	while( mem_page_num>0 ){
		if(rtk_scan_write_bbt(mtd,(page+page_counter),page_size,temp_BBT+page_counter*page_size,&rtkn->tmpBuf[page_size])){
			printk("[%s] write BBT page %d failure!!\n\r", __FUNCTION__, page+page_counter);
			rc =  -1;
			rtk_scan_erase_bbt(mtd,page);

			#ifdef SWAP_2K_DATA
				memset(rtkn->tmpBuf,0xff,MAX_RTKN_BUF_SIZE);
				rtkn->tmpBuf[page_size+OOB_BBI_OFF] = 0x00;
			#else
				memset(rtkn->tmpBuf,0xff,MAX_RTKN_BUF_SIZE);
				rtkn->tmpBuf[page_size] = 0x00;
			#endif

			//mark as bad block;
			if ( isLastPage){
				rtk_scan_write_bbt(mtd,(page+ppb-1),page_size,temp_BBT,&rtkn->tmpBuf[page_size]);
				rtk_scan_write_bbt(mtd,(page+ppb-2),page_size,temp_BBT,&rtkn->tmpBuf[page_size]);
			}else{
				rtk_scan_write_bbt(mtd,(page),page_size,temp_BBT,&rtkn->tmpBuf[page_size]);
				rtk_scan_write_bbt(mtd,(page+1),page_size,temp_BBT,&rtkn->tmpBuf[page_size]);
			}
			goto EXIT_V2R;
		}
		//printk("[%s, line %d] mem_page_num = %d page_counter %d\n\r",__FUNCTION__,__LINE__,mem_page_num, page_counter);
		page_counter++;
		mem_page_num--;
	}
EXIT_V2R:
	if(temp_BBT)
		kfree(temp_BBT);
	return rc;
}

static int rtk_scan_v2r_bbt(struct mtd_info *mtd)
{
	struct nand_chip *this = (struct nand_chip *)mtd->priv;
	struct rtknflash *rtkn = (struct rtknflash *)this->priv;
	int isLastPage = rtkn->chip_param.isLastPage;
	unsigned int bbt_v2r_page;
    int rc=0, i=0, error_count=0;
	unsigned char isbbt=0;
	unsigned char mem_page_num=0, page_counter=0, mem_page_num_tmp=0;
	unsigned char *temp_BBT=NULL;
	unsigned int block_v2r_num=0;
	unsigned int block_size = (1 << this->phys_erase_shift);
	unsigned int page_size = (1 << this->page_shift);
	//unsigned int oob_size = mtd->oobsize;
	unsigned char load_bbt_error = 0, is_first_boot=1;
	int numchips = this->numchips;
	unsigned int ppb = (1 << this->phys_erase_shift)/(1 << this->page_shift);

    //bbt_v2r_page = ((BOOT_SIZE/block_size)-(2*BACKUP_BBT))*ppb;
    bbt_v2r_page = (REMAP_BBT_POS/block_size)*ppb;
	block_v2r_num = (1<<this->chip_shift)/block_size - RBA;

	printk("[%s]:%d,RBA=%x,2=%x,\n",__func__,__LINE__,RBA,(1<<this->chip_shift)/block_size);

	#if 0
	block_v2r_num = (unsigned int)(this->chip_size)/block_size-((unsigned int)(this->chipsize)/block_size)*RBA_PERCENT/100;
	#endif

	printk("[%s]:%d,block_v2r_num=%x\n",__func__,__LINE__,block_v2r_num);
	//create virtual block to real good block remapping!!!
	dma_cache_wback((unsigned long)rtkn->bbt_v2r,(sizeof(BB_v2r)*block_v2r_num));   //czyao

	mem_page_num = ((sizeof(BB_v2r)*block_v2r_num) + page_size-1 )/page_size;
	/* check */
	if(mem_page_num > ppb)
	{
		printk("v2r remap bbt size large than one block,block=%d\n",mem_page_num);
		return -1;
	}

	temp_BBT = (unsigned char *)kmalloc( mem_page_num*page_size, GFP_KERNEL );
	if(!temp_BBT){
		printk("%s: Error, no enough memory for temp_BBT_v2r\n",__FUNCTION__);
		return -1;
	}
	nand_printf("[%s]:%d\n",__func__,__LINE__);

	//test NEW method!
	for(i=0;i<BACKUP_BBT;i++){
		nand_printf("[%s]:%d\n",__func__,__LINE__);
		rc = rtk_scan_read_oob(mtd,rtkn->tmpBuf,(bbt_v2r_page+(i*ppb)),page_size);
		nand_printf("[%s]:%d\n",__func__,__LINE__);
#ifdef SWAP_2K_DATA
		isbbt = rtkn->tmpBuf[page_size+OOB_BBI_OFF];
#else
		isbbt = rtkn->tmpBuf[page_size];
#endif

		if(!rc){
		    if(isbbt==BBT_TAG)//bbt has already created
				is_first_boot = 0;
		}
	}
	nand_printf("[%s]:%d\n",__func__,__LINE__);
	//is_first_boot = 1;
	//printk("%s: is_first_boot:%d\n\r",__FUNCTION__, is_first_boot);

	for(i=0;i<BACKUP_BBT;i++){
		mem_page_num_tmp = mem_page_num;
		page_counter=0;load_bbt_error=0;
		rc = rtk_block_isbad(mtd,numchips-1,(bbt_v2r_page+(i*ppb))*page_size);
		if(!rc){
			printk("load bbt v2r table:%d page:%d\n\r",i, (bbt_v2r_page+(i*ppb)));
			rc = rtk_scan_read_oob(mtd,rtkn->tmpBuf,(bbt_v2r_page+(i*ppb)),page_size);
#ifdef SWAP_2K_DATA
			isbbt = rtkn->tmpBuf[page_size+OOB_BBI_OFF];
#else
			isbbt = rtkn->tmpBuf[page_size];
#endif
			nand_printf("[%s]:%d\n",__func__,__LINE__);
	        if(!rc){
			    if(isbbt == BBT_TAG){
			    //if(0){
			        printk("[%s] have created v2r bbt table:%d on block %d, just loads it !!\n\r", __FUNCTION__,i,(bbt_v2r_page/ppb)+i);
					memcpy( temp_BBT, rtkn->tmpBuf, page_size );
			        page_counter++;
			        mem_page_num_tmp--;
			        nand_printf("[%s]:%d\n",__func__,__LINE__);
			        while( mem_page_num_tmp>0 ){
				        //if( rtk_read_ecc_page((bbt0_block_num+(i*ppb)+page_counter), &NfDataBuf, &NfSpareBuf, page_size)){
				        if( rtk_scan_read_oob(mtd, rtkn->tmpBuf, (bbt_v2r_page+(i*ppb)+page_counter),page_size) ){
					        printk("[%s] load v2r bbt table%d error!!\n\r", __FUNCTION__,i);
					        rtk_scan_erase_bbt(mtd,(bbt_v2r_page+(ppb*i)));
#ifdef SWAP_2K_DATA
							memset(rtkn->tmpBuf,0xff,MAX_RTKN_BUF_SIZE);
							rtkn->tmpBuf[page_size+OOB_BBI_OFF] = 0x00;
#else
							memset(rtkn->tmpBuf,0xff,MAX_RTKN_BUF_SIZE);
							rtkn->tmpBuf[page_size] = 0x00;
#endif

							if ( isLastPage){
								rtk_scan_write_bbt(mtd,(bbt_v2r_page+(ppb*i)+(ppb-1)),page_size,temp_BBT,&rtkn->tmpBuf[page_size]);
								rtk_scan_write_bbt(mtd,(bbt_v2r_page+(ppb*i)+(ppb-2)),page_size,temp_BBT,&rtkn->tmpBuf[page_size]);
							}else{
								rtk_scan_write_bbt(mtd,(bbt_v2r_page+(ppb*i)),page_size,temp_BBT,&rtkn->tmpBuf[page_size]);
								rtk_scan_write_bbt(mtd,(bbt_v2r_page+(ppb*i)+1),page_size,temp_BBT,&rtkn->tmpBuf[page_size]);
							}
                            load_bbt_error=1;
			                error_count++;
					        break;
				        }
				        nand_printf("[%s]:%d\n",__func__,__LINE__);
#ifdef SWAP_2K_DATA
						isbbt = rtkn->tmpBuf[page_size+OOB_BBI_OFF];
#else
						isbbt = rtkn->tmpBuf[page_size];
#endif
						if(isbbt == BBT_TAG){//check bb tag in each page!
							nand_printf("[%s]:%d\n",__func__,__LINE__);
							memcpy(temp_BBT+page_counter*page_size, rtkn->tmpBuf, page_size );
							nand_printf("[%s]:%d\n",__func__,__LINE__);
						page_counter++;
						mem_page_num_tmp--;
						}else{
                            load_bbt_error=1;
			                error_count++;
					        printk("[%s] check v2r bbt table%d tag:0x%x fail!!\n\r", __FUNCTION__,i,isbbt);
							break;
						}
			        }
					if(!load_bbt_error){

						nand_printf("[%s]:%d\n",__func__,__LINE__);
						memcpy( rtkn->bbt_v2r, temp_BBT, sizeof(BB_v2r)*(block_v2r_num));
					    printk("check v2r bbt table:%d OK\n\r",i);
					    goto CHECK_BBT_OK;
					}
			    }else{
					if(is_first_boot){
						printk("Create v2r bbt table:%d is_first_boot:%d\n\r",i, is_first_boot);
						create_v2r_remapping(mtd, bbt_v2r_page+(i*ppb), block_v2r_num);
					}
			    }
		    }else{
				printk("read v2r bbt table:%d page:%d\n\r",i, (bbt_v2r_page+(i*ppb)));
				rtk_scan_erase_bbt(mtd,(bbt_v2r_page+(ppb*i)));
#ifdef SWAP_2K_DATA
				memset(rtkn->tmpBuf,0xff,MAX_RTKN_BUF_SIZE);
				rtkn->tmpBuf[page_size+OOB_BBI_OFF] = 0x00;
#else
				memset(rtkn->tmpBuf,0xff,MAX_RTKN_BUF_SIZE);
				rtkn->tmpBuf[page_size] = 0x00;
#endif
				if ( isLastPage){
					rtk_scan_write_bbt(mtd,(bbt_v2r_page+(ppb*i)+(ppb-1)),page_size,temp_BBT,&rtkn->tmpBuf[page_size]);
					rtk_scan_write_bbt(mtd,(bbt_v2r_page+(ppb*i)+(ppb-2)),page_size,temp_BBT,&rtkn->tmpBuf[page_size]);

				}else{
					rtk_scan_write_bbt(mtd,(bbt_v2r_page+(ppb*i)),page_size,temp_BBT,&rtkn->tmpBuf[page_size]);
					rtk_scan_write_bbt(mtd,(bbt_v2r_page+(ppb*i)+1),page_size,temp_BBT,&rtkn->tmpBuf[page_size]);
			    }
		    }
		}else{
            printk("v2r bbt table:%d block:%d page:%d is bad\n\r",i,(bbt_v2r_page/ppb)+i,bbt_v2r_page+(i*ppb));
			error_count++;
		}
	}
CHECK_BBT_OK:
	/* test */

	if (temp_BBT)
		kfree(temp_BBT);
    if(error_count >= BACKUP_BBT){
        rc = -1;
		printk("%d v2r table are all bad!(T______T)\n\r", BACKUP_BBT);
	}

    return rc;
}

static int rtk_update_bbt (struct mtd_info *mtd, BB_t *bbt)
{
	int rc = 0,i=0, error_count=0;
	struct nand_chip *this = mtd->priv;
	struct rtknflash *rtkn = (struct rtknflash *)this->priv;
	int isLastPage = rtkn->chip_param.isLastPage;
	//unsigned char active_chip = this->active_chip;
	unsigned int ppb = (1<<this->phys_erase_shift)/(1<<this->page_shift);
	unsigned int page_size = (1<<this->page_shift);
	unsigned int bbt_page;
	unsigned char mem_page_num, page_counter=0, mem_page_num_tmp=0;
	//int numchips = this->numchips;

	u8 *temp_BBT = 0;

	//bbt_page = ((BOOT_SIZE >> this->phys_erase_shift)-BACKUP_BBT)*ppb;
	bbt_page = ((REMAP_BBT_POS >> this->phys_erase_shift)+BACKUP_BBT)*ppb;
	mem_page_num = (sizeof(BB_t)*rtkn->RBA + page_size-1 )/page_size;
	printk("[%s] mem_page_num %d bbt_page %d\n\r", __FUNCTION__, mem_page_num, bbt_page);

	temp_BBT = kmalloc( mem_page_num*page_size, GFP_KERNEL );
	if ( !(temp_BBT) ){
		printk("%s: Error, no enough memory for temp_BBT\n",__FUNCTION__);
		return -1;
	}

	memset(temp_BBT, 0xff, mem_page_num*page_size);
	memcpy(temp_BBT, bbt, sizeof(BB_t)*rtkn->RBA );

#ifdef SWAP_2K_DATA
	rtkn->tmpBuf[page_size+OOB_BBI_OFF] = BBT_TAG;
#else
	rtkn->tmpBuf[page_size] = BBT_TAG;
#endif

	//this->select_chip(mtd, numchips-1);

    for(i=0;i<BACKUP_BBT;i++){
		mem_page_num_tmp = mem_page_num;
		page_counter=0;

#ifdef SWAP_2K_DATA
		rtkn->tmpBuf[page_size+OOB_BBI_OFF] = BBT_TAG;
#else
		rtkn->tmpBuf[page_size] = BBT_TAG;
#endif

	    //this->select_chip(mtd, numchips-1);
		if(rtk_scan_erase_bbt(mtd,(bbt_page+(ppb*i)))){
			printk("[%s]error: erase BBT%d page %d failure\n\r", __FUNCTION__,i, bbt_page+(ppb*i));
			/*erase fail: mean this block is bad, so do not write data!!!*/
#ifdef SWAP_2K_DATA
			rtkn->tmpBuf[page_size+OOB_BBI_OFF] = BBT_TAG;
#else
			rtkn->tmpBuf[page_size] = BBT_TAG;
#endif
			//mark as bad block;
			if ( isLastPage){
				rtk_scan_write_bbt(mtd,(bbt_page+(ppb*i)+ppb-1),page_size,temp_BBT,&rtkn->tmpBuf[page_size]);
				rtk_scan_write_bbt(mtd,(bbt_page+(ppb*i)+ppb-2),page_size,temp_BBT,&rtkn->tmpBuf[page_size]);
			}else{
				rtk_scan_write_bbt(mtd,(bbt_page+(ppb*i)),page_size,temp_BBT,&rtkn->tmpBuf[page_size]);
				rtk_scan_write_bbt(mtd,(bbt_page+(ppb*i)+1),page_size,temp_BBT,&rtkn->tmpBuf[page_size]);
			}
			mem_page_num_tmp = 0;
			error_count++;
		}
		while( mem_page_num_tmp>0 ){
			if(rtk_scan_write_bbt(mtd,(bbt_page+(ppb*i)+page_counter),page_size,temp_BBT+page_counter*page_size,&rtkn->tmpBuf[page_size])){
				rtk_scan_erase_bbt(mtd,(bbt_page+(ppb*i)));
				//this->erase_block(mtd, numchips-1, bbt_page+(ppb*i));
#ifdef SWAP_2K_DATA
				memset(rtkn->tmpBuf,0xff,MAX_RTKN_BUF_SIZE);
				rtkn->tmpBuf[page_size+OOB_BBI_OFF] = 0x00;
#else
				memset(rtkn->tmpBuf,0xff,MAX_RTKN_BUF_SIZE);
				rtkn->tmpBuf[page_size] = 0x00;
#endif
				//mark as bad block;
				if (isLastPage){
					rtk_scan_write_bbt(mtd,(bbt_page+(ppb*i)+ppb-1),page_size,temp_BBT,&rtkn->tmpBuf[page_size]);
					rtk_scan_write_bbt(mtd,(bbt_page+(ppb*i)+ppb-2),page_size,temp_BBT,&rtkn->tmpBuf[page_size]);
				}else{
					rtk_scan_write_bbt(mtd,(bbt_page+(ppb*i)),page_size,temp_BBT,&rtkn->tmpBuf[page_size]);
					rtk_scan_write_bbt(mtd,(bbt_page+(ppb*i)+1),page_size,temp_BBT,&rtkn->tmpBuf[page_size]);
				}
				error_count++;
				break;
			}
			page_counter++;
			mem_page_num_tmp--;
		}

	}
	//this->select_chip(mtd, active_chip);
//EXIT:
	if (temp_BBT)
		kfree(temp_BBT);
    if(error_count >= BACKUP_BBT){
		rc = -1;
		printk("%d bbt table are all bad!(T______T)\n\r", BACKUP_BBT);
	}
	return rc;
}

#if 0
/* erase|write|read function for ops */
static rtk_remapBBT_erase(struct mtd_info* mtd,unsigned int page_addr)
{
	int addr_cycle[5], page_shift,res;
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
		spin_unlock_irqrestore(&lock_nand, flags_nand);
		return -1
	}

	spin_unlock_irqrestore(&lock_nand, flags_nand);
	return 0;
}

static rtk_remapBBT_read(struct mtd_info* mtd,uint8_t *buf, int oob_required, int page)
{
	nand_printf("[%s]:%d\n",__func__,__LINE__);
	struct nand_chip *chip = (struct nand_chip *)mtd->priv;
	struct rtknflash *rtkn = (struct rtknflash *)chip->priv;
	int dma_counter = 4,page_shift,page_num[3],buf_pos=0;
	int dram_sa, oob_sa;
	unsigned long flash_addr_t=0;
	unsigned int flags_nand = 0;
	uint8_t* oobBuf = chip->oob_poi;
	int orig_block = page/(mtd->erasesize/mtd->writesize);

	int i;
	//printk("[%s]:%d,page=%x\n",__func__,__LINE__,page);
	spin_lock_irqsave(&lock_nand, flags_nand);

	/* get the real page */

	//__flush_cache_all();
	/* addr */
	uint8_t *oob_area,*data_area,data_area0[512+16+CACHELINE_SIZE+CACHELINE_SIZE-4]; //data,oob point must 32 cache aligment
	memset(data_area0, 0xff, 512+16+CACHELINE_SIZE+CACHELINE_SIZE-4);
	data_area = data_area0;
	oob_area=(uint8_t*) data_area+512;
	oob_sa =  ( (uint32_t)(oob_area ) & (~M_mask));
	dram_sa = ((uint32_t)data_area) & (~M_mask);

	/* flash addr */
	for(page_shift=0;page_shift<3; page_shift++) {
	    page_num[page_shift] = ((page>>(8*page_shift)) & 0xff);
		flash_addr_t |= (page_num[page_shift] << (12+8*page_shift));
	}

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


}

static rtk_remapBBT_write(struct mtd_info *mtd,const uint8_t *buf,int page,int oob_required)
{
	int i;
	struct rtknflash *rtkn = (struct rtknflash *)chip->priv;
	int page = rtkn->curr_page_addr,dma_counter = 4,page_shift,page_num[3],buf_pos=0;
	int dram_sa, oob_sa;
	unsigned long flash_addr_t=0;
	unsigned int flags_nand = 0;
	unsigned int ppb = mtd->erasesize/mtd->writesize;
	uint8_t* dataBuf = buf,*oobBuf = chip->oob_poi;
	int orig_block = page/ppb,res;

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
	for(page_shift=0;page_shift<3; page_shift++) {
	    page_num[page_shift] = ((page>>(8*page_shift)) & 0xff);
		flash_addr_t |= (page_num[page_shift] << (12+8*page_shift));
	}
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
}

#endif

int rtk_remapBBT_write_fail(struct mtd_info* mtd,unsigned int page,const uint8_t *buf,int oob_required)
{
	struct nand_chip* this = (struct nand_chip*)mtd->priv;
	struct rtknflash* rtkn = (struct rtknflash*)this->priv;
	int backup_offset;
	int isLastPage = rtkn->chip_param.isLastPage;

	int block_remap = 0x12345678,i,ppb=(mtd->erasesize)/(mtd->writesize),block;
	unsigned int page_size = 1<<this->page_shift,oobsize=mtd->oobsize;


	if(check_BBT(mtd,page/ppb)==0)
	{
	    for( i=0; i<RBA; i++){
		    if ( rtkn->bbt[i].bad_block == BB_INIT && rtkn->bbt[i].remap_block != RB_INIT){
			/* do have one chip */
			#if 0
				if ( chipnr != chipnr_remap)
					this->bbt[i].BB_die = chipnr_remap;
				else
					this->bbt[i].BB_die = chipnr;
				#endif
			    rtkn->bbt[i].bad_block = page/ppb;
			    block_remap = rtkn->bbt[i].remap_block;
			    break;
		    }
	    }

	    if ( block_remap == 0x12345678 ){
		    printk("[%s] RBA do not have free remap block\n", __FUNCTION__);
		    return -1;
	    }

	    dump_BBT(mtd);

	    if ( rtk_update_bbt (mtd,rtkn->bbt) ){
		    printk("[%s] rtk_update_bbt() fails\n", __FUNCTION__);
		    return -1;
	    }
	}

	block = page/ppb;
	backup_offset = page&(ppb-1);

	memset(rtkn->tmpBuf,0xff,MAX_RTKN_BUF_SIZE);
#ifdef CONFIG_JFFS2_FS
	/* need read cleanmarker */
	if(backup_offset == 0)
		rtk_scan_read_oob(mtd,rtkn->tmpBuf,page,page_size);
#endif

	rtk_scan_erase_bbt(mtd,(block_remap*ppb));
	printk("[%s] Start to Backup old_page from %d to %d\n", __FUNCTION__, block*ppb, block*ppb+backup_offset-1);
	//memset(rtkn->buf,0xff,MAX_RTKN_BUF_SIZE);
	for ( i=0; i<backup_offset; i++){
		rtk_scan_read_oob(mtd,rtkn->tmpBuf,(block*ppb+i),page_size);
		rtk_scan_write_bbt(mtd,(block_remap*ppb+i),page_size,rtkn->tmpBuf,&rtkn->tmpBuf[page_size]);
	}

	/* check */
	if(oob_required){
		memcpy(&rtkn->tmpBuf[page_size],this->oob_poi,oobsize);
	}

	rtk_scan_write_bbt(mtd,(block_remap*ppb+backup_offset),page_size,(unsigned char*)buf,&rtkn->tmpBuf[page_size]);
	printk("[%s] write failure page = %d to %d\n", __FUNCTION__, page, block_remap*ppb+backup_offset);
	memset(rtkn->tmpBuf,0xff,MAX_RTKN_BUF_SIZE);
#ifdef SWAP_2K_DATA
	rtkn->tmpBuf[page_size+OOB_BBI_OFF] = 0x00;
#else
	rtkn->tmpBuf[page_size] = 0x00;
#endif

	block = page/ppb;
	memset(rtkn->tmpBuf,0xff,MAX_RTKN_BUF_SIZE);
	if ( isLastPage ){
		rtk_scan_erase_bbt(mtd,(block*ppb));
		rtk_scan_write_bbt(mtd,(block*ppb+ppb-1),page_size,rtkn->tmpBuf,&rtkn->tmpBuf[page_size]);
		rtk_scan_write_bbt(mtd,(block*ppb+ppb-2),page_size,rtkn->tmpBuf,&rtkn->tmpBuf[page_size]);
	}else{
		rtk_scan_erase_bbt(mtd,(block*ppb));
		rtk_scan_write_bbt(mtd,(block*ppb),page_size,rtkn->tmpBuf,&rtkn->tmpBuf[page_size]);
		rtk_scan_write_bbt(mtd,(block*ppb+1),page_size,rtkn->tmpBuf,&rtkn->tmpBuf[page_size]);
	}

	return 0;
}

int rtk_remapBBT_read_fail(struct mtd_info* mtd,unsigned int page)
{
	struct nand_chip* this = (struct nand_chip*)mtd->priv;
	struct rtknflash* rtkn = (struct rtknflash*)this->priv;
	int isLastPage = rtkn->chip_param.isLastPage;
	unsigned int ppb = (mtd->erasesize)/(mtd->writesize),block,block_remap = 0x12345678,page_size = (1<<this->page_shift);
	int i;


	if(check_BBT(mtd,page/ppb)==0)
	{
	    for( i=0; i<RBA; i++){
		    if ( rtkn->bbt[i].bad_block == BB_INIT && rtkn->bbt[i].remap_block != RB_INIT){
			/* do have one chip */
			#if 0
				if ( chipnr != chipnr_remap)
					this->bbt[i].BB_die = chipnr_remap;
				else
					this->bbt[i].BB_die = chipnr;
				#endif
			    rtkn->bbt[i].bad_block = page/ppb;
			    block_remap = rtkn->bbt[i].remap_block;
			    break;
		    }
	    }

	    if ( block_remap == 0x12345678 ){
		    printk("[%s] RBA do not have free remap block\n", __FUNCTION__);
		    return -1;
	    }

	    dump_BBT(mtd);

	    if ( rtk_update_bbt (mtd,rtkn->bbt) ){
		    printk("[%s] rtk_update_bbt() fails\n", __FUNCTION__);
		    return -1;
	    }
	}

	block = page/ppb;
	memset(rtkn->tmpBuf,0xff,MAX_RTKN_BUF_SIZE);
#ifdef SWAP_2K_DATA
	rtkn->tmpBuf[page_size+OOB_BBI_OFF] = 0x00;
#else
	rtkn->tmpBuf[page_size] = 0x00;
#endif
	if ( isLastPage ){
		rtk_scan_erase_bbt(mtd,(block*ppb));
		rtk_scan_write_bbt(mtd,(block*ppb+ppb-1),page_size,rtkn->tmpBuf,&rtkn->tmpBuf[page_size]);
		rtk_scan_write_bbt(mtd,(block*ppb+ppb-2),page_size,rtkn->tmpBuf,&rtkn->tmpBuf[page_size]);
	}else{
		rtk_scan_erase_bbt(mtd,(block*ppb));
		rtk_scan_write_bbt(mtd,(block*ppb),page_size,rtkn->tmpBuf,&rtkn->tmpBuf[page_size]);
		rtk_scan_write_bbt(mtd,(block*ppb+1),page_size,rtkn->tmpBuf,&rtkn->tmpBuf[page_size]);
	}


	return 0;

}

int rtk_remapBBT_erase_fail(struct mtd_info* mtd,unsigned int page)
{
	struct nand_chip* this = (struct nand_chip*)mtd->priv;
	struct rtknflash* rtkn = (struct rtknflash*)this->priv;
	int isLastPage = rtkn->chip_param.isLastPage;
	unsigned int ppb = (mtd->erasesize)/(mtd->writesize),block,block_remap=0x12345678,page_size=(1<<this->page_shift);
	int i;

	if(check_BBT(mtd,page/ppb)==0)
	{
	    for( i=0; i<RBA; i++){
		    if ( rtkn->bbt[i].bad_block == BB_INIT && rtkn->bbt[i].remap_block != RB_INIT){
			/* do have one chip */
			#if 0
				if ( chipnr != chipnr_remap)
					this->bbt[i].BB_die = chipnr_remap;
				else
					this->bbt[i].BB_die = chipnr;
				#endif
			    rtkn->bbt[i].bad_block = page/ppb;
			    block_remap = rtkn->bbt[i].remap_block;
			    break;
		    }
	    }

	    if ( block_remap == 0x12345678 ){
		    printk("[%s] RBA do not have free remap block\n", __FUNCTION__);
		    return -1;
	    }

	    dump_BBT(mtd);

	    if ( rtk_update_bbt (mtd,rtkn->bbt) ){
		    printk("[%s] rtk_update_bbt() fails\n", __FUNCTION__);
		    return -1;
	    }
	}

	block = page/ppb;
	memset(rtkn->tmpBuf,0xff,MAX_RTKN_BUF_SIZE);
#ifdef SWAP_2K_DATA
	rtkn->tmpBuf[page_size+OOB_BBI_OFF] = 0x00;
#else
	rtkn->tmpBuf[page_size] = 0x00;
#endif
	if ( isLastPage ){
		rtk_scan_erase_bbt(mtd,(block*ppb));
		rtk_scan_write_bbt(mtd,(block*ppb+ppb-1),page_size,rtkn->tmpBuf,&rtkn->tmpBuf[page_size]);
		rtk_scan_write_bbt(mtd,(block*ppb+ppb-2),page_size,rtkn->tmpBuf,&rtkn->tmpBuf[page_size]);
	}else{
		rtk_scan_erase_bbt(mtd,(block*ppb));
		rtk_scan_write_bbt(mtd,(block*ppb),page_size,rtkn->tmpBuf,&rtkn->tmpBuf[page_size]);
		rtk_scan_write_bbt(mtd,(block*ppb+1),page_size,rtkn->tmpBuf,&rtkn->tmpBuf[page_size]);
	}

	/* erase remap block*/
	rtk_scan_erase_bbt(mtd,block_remap*ppb);

	return 0;
}

#endif


/*********************************************NORMAL BBT***************************************************/
#ifdef CONFIG_RTK_NORMAL_BBT
static int scan_normal_BB(struct mtd_info *mtd)
{
	struct nand_chip *this = (struct nand_chip *)mtd->priv;
	struct rtknflash *rtkn = (struct rtknflash *)this->priv;
	__u32 addr;
	int block_size = 1 << this->phys_erase_shift;
	int table_index=0;
	int i;
	int numchips = this->numchips;
	int chip_size = this->chipsize;
//	this->active_chip = numchips-1;
//	this->select_chip(mtd, numchips-1);
	for( addr=0; addr<chip_size; addr+=block_size ){
		int block_index = addr >> this->phys_erase_shift;
		if ( rtk_block_isbad(mtd, numchips-1, addr) ){
			printk("block[%d] is bad\n",block_index);
			rtkn->bbt_nor[table_index].bad_block = block_index;
			rtkn->bbt_nor[table_index].block_info = 0x00;
			rtkn->bbt_nor[table_index].BB_die = numchips-1;
			rtkn->bbt_nor[table_index].RB_die = numchips-1;
			table_index++;
		}
		if(table_index >= bbt_num){
			printk("bad block number %d exceed bbt_num %d\n",table_index,bbt_num);
			return -1;
		}
	}
	for( i=table_index; table_index<bbt_num; table_index++){
		rtkn->bbt_nor[table_index].bad_block = BB_INIT;
		rtkn->bbt_nor[table_index].BB_die = BB_DIE_INIT;
		rtkn->bbt_nor[table_index].block_info = 0xff;
		rtkn->bbt_nor[table_index].RB_die = BB_DIE_INIT;
	}
	return 0;
}
static int rtk_create_normal_bbt(struct mtd_info *mtd, int page)
{
	printk("[%s] nand driver creates normal B%d !!\n", __FUNCTION__, page ==0?0:1);
	struct nand_chip *this = (struct nand_chip *)mtd->priv;
	struct rtknflash *rtkn = (struct rtknflash*)this->priv;
	int isLastPage = rtkn->chip_param.isLastPage;
	unsigned int page_size = (1<<this->page_shift),ppb=(mtd->erasesize)/(mtd->writesize);
	int rc = 0;
	u8 *temp_BBT = 0;
	u8 mem_page_num, page_counter=0;
	if ( scan_normal_BB(mtd) ){
		printk("[%s] scan_normal_BB() error !!\n", __FUNCTION__);
		return -1;
	}
#if 0
	if ( this->numchips >1 ){
		printk("[%s, line %d]\n",__FUNCTION__,__LINE__);
		if ( scan_other_normal_BB(mtd) ){
			printk("[%s] scan_last_die() error !!\n", __FUNCTION__);
			return -1;
		}
	}
#endif
	mem_page_num = (sizeof(BBT_normal)*bbt_num + page_size-1 )/page_size;
	temp_BBT = kmalloc( mem_page_num*page_size, GFP_KERNEL );
	if ( !temp_BBT ){
		printk("%s: Error, no enough memory for temp_BBT\n",__FUNCTION__);
		return -ENOMEM;
	}
	memset( temp_BBT, 0xff, mem_page_num*page_size);
//	this->select_chip(mtd, 0);
	if (rtk_scan_erase_bbt(mtd,page) ){
		printk("[%s]erase block %d failure !!\n", __FUNCTION__, page/ppb);
#ifdef SWAP_2K_DATA
		memset(rtkn->tmpBuf,0xff,MAX_RTKN_BUF_SIZE);
		rtkn->tmpBuf[page_size+OOB_BBI_OFF] = 0x00;
#else
		memset(rtkn->tmpBuf,0xff,MAX_RTKN_BUF_SIZE);
		rtkn->tmpBuf[page_size] = 0x00;
#endif
		if ( isLastPage){
			rtk_scan_write_bbt(mtd,(page+ppb-1),page_size,rtkn->tmpBuf,&rtkn->tmpBuf[page_size]);
			rtk_scan_write_bbt(mtd,(page+ppb-2),page_size,rtkn->tmpBuf,&rtkn->tmpBuf[page_size]);
		}else{
			rtk_scan_write_bbt(mtd,(page),page_size,rtkn->tmpBuf,&rtkn->tmpBuf[page_size]);
			rtk_scan_write_bbt(mtd,(page+1),page_size,rtkn->tmpBuf,&rtkn->tmpBuf[page_size]);
		}
		rc =  -1;
		goto EXIT;
	}
#ifdef SWAP_2K_DATA
		memset(rtkn->tmpBuf,0xff,MAX_RTKN_BUF_SIZE);
		rtkn->tmpBuf[page_size+OOB_BBI_OFF] = BBT_TAG;
#else
		memset(rtkn->tmpBuf,0xff,MAX_RTKN_BUF_SIZE);
		rtkn->tmpBuf[page_size] = BBT_TAG;
#endif

	memcpy( temp_BBT, rtkn->bbt_nor, sizeof(BBT_normal)*bbt_num );
	while( mem_page_num>0 ){
#if 0
		if ( this->write_ecc_page(mtd, 0, page+page_counter, temp_BBT+page_counter*page_size,
			this->g_oobbuf, 1) ){
#endif
		if(rtk_scan_write_bbt(mtd,(page+page_counter),page_size,temp_BBT+page_counter*page_size,&rtkn->tmpBuf[page_size])){
			printk("[%s] write page %d failure!!\n", __FUNCTION__, page+page_counter);
			rc =  -1;
			rtk_scan_erase_bbt(mtd,page);
#ifdef SWAP_2K_DATA
			memset(rtkn->tmpBuf,0xff,MAX_RTKN_BUF_SIZE);
			rtkn->tmpBuf[page_size+OOB_BBI_OFF] = 0x00;
#else
			memset(rtkn->tmpBuf,0xff,MAX_RTKN_BUF_SIZE);
			rtkn->tmpBuf[page_size] = 0x00;
#endif
			if ( isLastPage){
				rtk_scan_write_bbt(mtd,(page+ppb-1),page_size,rtkn->tmpBuf,&rtkn->tmpBuf[page_size]);
				rtk_scan_write_bbt(mtd,(page+ppb-2),page_size,rtkn->tmpBuf,&rtkn->tmpBuf[page_size]);
			}else{
				rtk_scan_write_bbt(mtd,(page),page_size,rtkn->tmpBuf,&rtkn->tmpBuf[page_size]);
				rtk_scan_write_bbt(mtd,(page+1),page_size,rtkn->tmpBuf,&rtkn->tmpBuf[page_size]);
			}
			goto EXIT;
		}
		page_counter++;
		mem_page_num--;
	}
EXIT:
	if (temp_BBT)
		kfree(temp_BBT);
	return rc;
}
int rtk_update_normal_bbt (struct mtd_info *mtd, BBT_normal *bbt_nor)
{
	int rc = 0,i=0, error_count=0;
	struct nand_chip *this = mtd->priv;
	struct rtknflash *rtkn = (struct rtknflash*)this->priv;
	int isLastPage = rtkn->chip_param.isLastPage;
	//unsigned char active_chip = this->active_chip;
	unsigned int bbt_page;
	unsigned char mem_page_num, page_counter=0, mem_page_num_tmp=0;
	int numchips = this->numchips;
	u8 *temp_BBT = 0;
	unsigned int ppb = (mtd->erasesize)/(mtd->writesize),page_size=(1<<this->page_shift);
	//bbt_page = ((NORMAL_BBT_POSITION >> this->phys_erase_shift)-BACKUP_BBT)*ppb;
	bbt_page = (NORMAL_BBT_POSITION >> this->phys_erase_shift)*ppb;
	mem_page_num = (sizeof(BBT_normal)*rtkn->bbt_num + page_size-1 )/page_size;
	printk("[%s] mem_page_num %d bbt_page %d\n\r", __FUNCTION__, mem_page_num, bbt_page);
	temp_BBT = kmalloc( mem_page_num*page_size, GFP_KERNEL );
	if ( !(temp_BBT) ){
		printk("%s: Error, no enough memory for temp_BBT\n",__FUNCTION__);
		return -1;
	}
	memset(temp_BBT, 0xff, mem_page_num*page_size);
	memcpy(temp_BBT, bbt_nor, sizeof(BBT_normal)*rtkn->bbt_num);
#ifdef SWAP_2K_DATA
	memset(rtkn->tmpBuf,0xff,MAX_RTKN_BUF_SIZE);
	rtkn->tmpBuf[page_size+OOB_BBI_OFF] = BBT_TAG;
#else
	memset(rtkn->tmpBuf,0xff,MAX_RTKN_BUF_SIZE);
	rtkn->tmpBuf[page_size] = BBT_TAG;
#endif
	//this->select_chip(mtd, numchips-1);
    for(i=0;i<BACKUP_BBT;i++){
		mem_page_num_tmp = mem_page_num;
		page_counter=0;
#ifdef SWAP_2K_DATA
		rtkn->tmpBuf[page_size+OOB_BBI_OFF] = BBT_TAG;
#else
		rtkn->tmpBuf[page_size] = BBT_TAG;
#endif
	    //this->select_chip(mtd, numchips-1);

		if (rtk_scan_erase_bbt(mtd,(bbt_page+(ppb*i))) ){
			printk("[%s]error: erase normal BBT%d page %d failure\n\r", __FUNCTION__,i, bbt_page+(ppb*i));
#ifdef SWAP_2K_DATA
			memset(rtkn->tmpBuf,0xff,MAX_RTKN_BUF_SIZE);
			rtkn->tmpBuf[page_size+OOB_BBI_OFF] = 0x00;
#else
			memset(rtkn->tmpBuf,0xff,MAX_RTKN_BUF_SIZE);
			rtkn->tmpBuf[page_size] = 0x00;
#endif
			if ( isLastPage){
				rtk_scan_write_bbt(mtd,(bbt_page+(ppb*i)+(ppb-1)),page_size,rtkn->tmpBuf,&rtkn->tmpBuf[page_size]);
				rtk_scan_write_bbt(mtd,(bbt_page+(ppb*i)+(ppb-2)),page_size,rtkn->tmpBuf,&rtkn->tmpBuf[page_size]);
			}else{
				rtk_scan_write_bbt(mtd,(bbt_page+(ppb*i)),page_size,rtkn->tmpBuf,&rtkn->tmpBuf[page_size]);
				rtk_scan_write_bbt(mtd,(bbt_page+(ppb*i)+1),page_size,rtkn->tmpBuf,&rtkn->tmpBuf[page_size]);
			}
			mem_page_num_tmp = 0;
			error_count++;
		}
		while( mem_page_num_tmp>0 ){
		    if (rtk_scan_write_bbt(mtd,(bbt_page+(ppb*i)+page_counter),page_size,temp_BBT+page_counter*page_size,&rtkn->tmpBuf[page_size])){
					printk("[%s] write normal BBT%d page %d failure!!\n\r", __FUNCTION__,i, bbt_page+(ppb*i)+page_counter);
					rtk_scan_erase_bbt(mtd,(bbt_page+(ppb*i)));
#ifdef SWAP_2K_DATA
					memset(rtkn->tmpBuf,0xff,MAX_RTKN_BUF_SIZE);
					rtkn->tmpBuf[page_size+OOB_BBI_OFF] = 0x00;
#else
					memset(rtkn->tmpBuf,0xff,MAX_RTKN_BUF_SIZE);
					rtkn->tmpBuf[page_size] = 0x00;
#endif
					if ( isLastPage){
						rtk_scan_write_bbt(mtd,(bbt_page+(ppb*i)+(ppb-1)),page_size,rtkn->tmpBuf,&rtkn->tmpBuf[page_size]);
						rtk_scan_write_bbt(mtd,(bbt_page+(ppb*i)+(ppb-2)),page_size,rtkn->tmpBuf,&rtkn->tmpBuf[page_size]);
					}else{
						rtk_scan_write_bbt(mtd,(bbt_page+(ppb*i)),page_size,rtkn->tmpBuf,&rtkn->tmpBuf[page_size]);
						rtk_scan_write_bbt(mtd,(bbt_page+(ppb*i)+1),page_size,rtkn->tmpBuf,&rtkn->tmpBuf[page_size]);
					}
					error_count++;
					break;
			}
			page_counter++;
			mem_page_num_tmp--;
		}
	}
	//this->select_chip(mtd, active_chip);
EXIT:
	if (temp_BBT)
		kfree(temp_BBT);
    if(error_count >= BACKUP_BBT){
		rc = -1;
		printk("normal %d bbt table are all bad!(T______T)\n\r", BACKUP_BBT);
	}
	return rc;
}
static void dump_normal_BBT(struct mtd_info *mtd)
{
	printk("[%s] Nand normal BBT Content\n", __FUNCTION__);
	struct nand_chip *this = (struct nand_chip *) mtd->priv;
	struct rtknflash *rtkn = (struct rtknflash *)this->priv;
	int i;
	int BBs=0;
	for ( i=0; i<bbt_num; i++){
		if( i == 0 && rtkn->bbt_nor[i].bad_block == BB_INIT){
			printk("Congratulation!! No BBs in this Nand.\n");
			break;
		}
		if ( rtkn->bbt_nor[i].bad_block != BB_INIT ){
			printk("[%d] (%d, %d, %d, %x)\n", i, rtkn->bbt_nor[i].BB_die, rtkn->bbt_nor[i].bad_block,
				rtkn->bbt_nor[i].RB_die, rtkn->bbt_nor[i].block_info);
			BBs++;
		}
	}
	rtkn->BBs = BBs;
}
static int nand_scan_normal_bbt(struct mtd_info *mtd)
{
	struct nand_chip *this = (struct nand_chip *)mtd->priv;
	struct rtknflash *rtkn = (struct rtknflash*)this->priv;
	int isLastPage = rtkn->chip_param.isLastPage;
	int rc = 0, i;
	__u8 isbbt;
	u8 *temp_BBT=0;
	u8 mem_page_num, page_counter=0, mem_page_num_tmp=0;
	u8 load_bbt_error=0,is_first_boot=1, error_count=0;
	unsigned int ppb=(mtd->erasesize)/(mtd->writesize);
	int numchips = this->numchips;
	unsigned int bbt_page,page_size = (1<<this->page_shift);
	dma_cache_wback((unsigned long) rtkn->bbt_nor,sizeof(BBT_normal)*bbt_num);   //czyao
	//bbt_page = ((NORMAL_BBT_POSITION/mtd->erasesize)-BACKUP_BBT)*ppb;
	bbt_page = (NORMAL_BBT_POSITION/mtd->erasesize)*ppb;
	mem_page_num = (sizeof(BBT_normal)*bbt_num + page_size-1 )/page_size;

	/* check */
	if(mem_page_num > ppb){
		printk("normal bbt data large than one block,block=%d\n",mem_page_num);
		return -1;
	}
	printk("[%s, line %d] mem_page_num=%d bbt_page %d\n",__FUNCTION__,__LINE__,mem_page_num, bbt_page);
	temp_BBT = kmalloc( mem_page_num*page_size, GFP_KERNEL );
	if ( !temp_BBT ){
		printk("%s: Error, no enough memory for temp_BBT\n",__FUNCTION__);
		return -ENOMEM;
	}
	memset( temp_BBT, 0xff, mem_page_num*page_size);
	for(i=0;i<BACKUP_BBT;i++){
		rc = rtk_block_isbad(mtd,numchips-1,(bbt_page+(i*ppb))*page_size);
		if(!rc){
			//rc = this->read_ecc_page(mtd, numchips-1, bbt_page+(i*ppb), this->g_databuf, this->g_oobbuf);
			rc = rtk_scan_read_oob(mtd,rtkn->tmpBuf,(bbt_page+(i*ppb)),page_size);
#ifdef SWAP_2K_DATA
			isbbt = rtkn->tmpBuf[page_size+OOB_BBI_OFF];
#else
			isbbt = rtkn->tmpBuf[page_size];
#endif
			if(!rc){
			if(isbbt==BBT_TAG)//bbt has already created
					is_first_boot = 0;
			}
		}
	}
	printk("%s: is_first_boot:%d\n\r",__FUNCTION__, is_first_boot);

	for(i=0;i<BACKUP_BBT;i++){
		mem_page_num_tmp = mem_page_num;
		page_counter=0;load_bbt_error=0;
		rc = rtk_block_isbad(mtd,numchips-1,(bbt_page+(i*ppb))*page_size);
		if(!rc){
			printk("load normal bbt table:%d page:%d\n\r",i, (bbt_page+(i*ppb)));
			rc = rtk_scan_read_oob(mtd,rtkn->tmpBuf,(bbt_page+(i*ppb)),page_size);
			//rc = this->read_ecc_page(mtd, numchips-1, bbt_page+(i*ppb), this->g_databuf, this->g_oobbuf);
#ifdef SWAP_2K_DATA
			isbbt = rtkn->tmpBuf[page_size+OOB_BBI_OFF];
#else
			isbbt = rtkn->tmpBuf[page_size];
#endif
	        if(!rc){
			    if(isbbt == BBT_TAG){
			        printk("[%s] have created normal bbt table:%d on block %d, just loads it !!\n\r", __FUNCTION__,i,(bbt_page/ppb)+i);
					memcpy( temp_BBT, rtkn->tmpBuf, page_size );
			        page_counter++;
			        mem_page_num_tmp--;
			        while( mem_page_num_tmp>0 ){
				        if( rtk_scan_read_oob(mtd, rtkn->tmpBuf, (bbt_page+(i*ppb)+page_counter), page_size) ){
					        printk("[%s] load normal bbt table%d error!! page:%d\n\r", __FUNCTION__,i, bbt_page+(i*ppb)+page_counter);
							//this->erase_block(mtd, numchips-1, bbt_page+(ppb*i));
							rtk_scan_erase_bbt(mtd,(bbt_page+(ppb*i)));
#ifdef SWAP_2K_DATA
							memset(rtkn->tmpBuf,0xff,MAX_RTKN_BUF_SIZE);
							rtkn->tmpBuf[page_size+OOB_BBI_OFF] = 0x00;
#else
							memset(rtkn->tmpBuf,0xff,MAX_RTKN_BUF_SIZE);
							rtkn->tmpBuf[page_size] = 0x00;
#endif
							if ( isLastPage){
								rtk_scan_write_bbt(mtd,(bbt_page+(ppb*i)+(ppb-1)),page_size,rtkn->tmpBuf,&rtkn->tmpBuf[page_size]);
								rtk_scan_write_bbt(mtd,(bbt_page+(ppb*i)+(ppb-2)),page_size,rtkn->tmpBuf,&rtkn->tmpBuf[page_size]);
							}else{
								rtk_scan_write_bbt(mtd,(bbt_page+(ppb*i)),page_size,rtkn->tmpBuf,&rtkn->tmpBuf[page_size]);
								rtk_scan_write_bbt(mtd,(bbt_page+(ppb*i)+1),page_size,rtkn->tmpBuf,&rtkn->tmpBuf[page_size]);
							}
                            load_bbt_error=1;
			                error_count++;
					        break;
				        }
#ifdef SWAP_2K_DATA
						isbbt = rtkn->tmpBuf[page_size+OOB_BBI_OFF];
#else
						isbbt = rtkn->tmpBuf[page_size];
#endif
						if(isbbt == BBT_TAG){//check bb tag in each page!
							memcpy( temp_BBT+page_counter*page_size, rtkn->tmpBuf, page_size );
							page_counter++;
							mem_page_num_tmp--;
						}else{
                            load_bbt_error=1;
			                error_count++;
					        printk("[%s] check normal bbt table%d tag:0x%x fail!!\n\r", __FUNCTION__,i,isbbt);
							break;
						}
			        }
					if(!load_bbt_error){
					    memcpy( rtkn->bbt_nor, temp_BBT, sizeof(BBT_normal)*bbt_num );
					    printk("check normal bbt table:%d OK\n\r",i);
					    goto CHECK_BBT_OK;
					}
			    }else{
					if(is_first_boot){
					    printk("Create normal bbt table:%d is_first_boot:%d\n\r",i, is_first_boot);
				        rtk_create_normal_bbt(mtd, bbt_page+(i*ppb));
					}
			    }
		    }else{
				printk("read normal bbt table:%d page:%d fail!\n\r",i, (bbt_page+(i*ppb)));
				rtk_scan_erase_bbt(mtd,(bbt_page+(ppb*i)));
				//this->erase_block(mtd, numchips-1, bbt_page+(ppb*i));
#ifdef SWAP_2K_DATA
				memset(rtkn->tmpBuf,0xff,MAX_RTKN_BUF_SIZE);
				rtkn->tmpBuf[page_size+OOB_BBI_OFF] = 0x00;
#else
				memset(rtkn->tmpBuf,0xff,MAX_RTKN_BUF_SIZE);
				rtkn->tmpBuf[page_size] = 0x00;
#endif
				if ( isLastPage){
					rtk_scan_write_bbt(mtd,(bbt_page+(ppb*i)+(ppb-1)),page_size,rtkn->tmpBuf,&rtkn->tmpBuf[page_size]);
					rtk_scan_write_bbt(mtd,(bbt_page+(ppb*i)+(ppb-2)),page_size,rtkn->tmpBuf,&rtkn->tmpBuf[page_size]);
				}else{
					rtk_scan_write_bbt(mtd,(bbt_page+(ppb*i)),page_size,rtkn->tmpBuf,&rtkn->tmpBuf[page_size]);
					rtk_scan_write_bbt(mtd,(bbt_page+(ppb*i)+1),page_size,rtkn->tmpBuf,&rtkn->tmpBuf[page_size]);
				}
			}
		}else{
            printk("normal bbt table:%d block:%d page:%d is bad\n\r",i,(bbt_page/ppb)+i,bbt_page+(i*ppb));
			error_count++;
		}
	}
CHECK_BBT_OK:
	dump_normal_BBT(mtd);
	if (temp_BBT)
		kfree(temp_BBT);
    if(error_count >= BACKUP_BBT){
        rc = -1;
		printk("%d normal bbt table are all bad!(T______T)\n\r", BACKUP_BBT);
	}
	return rc;
}
#endif //CONFIG_RTK_NORMAL_BBT

int rtkn_scan_bbt(struct mtd_info *mtd)
{
	struct nand_chip *this = (struct nand_chip *)mtd->priv;
	struct rtknflash *rtkn = this->priv;
	unsigned int block_v2r_num;
	//int ret;
	nand_printf("[%s]:%d\n",__func__,__LINE__);

	/* default seletc chip 0 */
	rtknflash_ops_select_chip(rtkn->mtd,1);

#ifdef CONFIG_RTK_REMAP_BBT
	/* default */
	rtkn->RBA_PERCENT = 5;

	//prom_printf("chipsize=%d,phys_erase_shift=%d\n",this->chipsize,this->phys_erase_shift);

	block_v2r_num = (this->chipsize >> this->phys_erase_shift);

	RBA = rtkn->RBA = block_v2r_num * rtkn->RBA_PERCENT/100;
	printk("[%s, line %d], RBA=%d, this->RBA_PERCENT = %d,block_v2r_num=%d\n",__func__,__LINE__,RBA,rtkn->RBA_PERCENT,block_v2r_num);

	rtkn->bbt = kmalloc( sizeof(BB_t)*RBA, GFP_KERNEL );
	if ( !rtkn->bbt ){
		printk("%s: Error, no enough memory for BBT\n",__FUNCTION__);
		return -1;
	}
	//printk("%s, %s, line %d, REG(NACR) = 0x%08x\n",__FILE__,__func__,__LINE__,rtk_readl(NACR));
	memset(rtkn->bbt, 0,  sizeof(BB_t)*RBA);

	printk("[%s, line %d] block_v2r_num %x\n\r",__FUNCTION__,__LINE__, block_v2r_num);

	rtkn->bbt_v2r = kmalloc(sizeof(BB_v2r)*(block_v2r_num), GFP_KERNEL);
	if ( !rtkn->bbt_v2r ){
		if(rtkn->bbt){
			kfree(rtkn->bbt);
		}
		printk("%s: Error, no enough memory for bbt_v2r\n",__FUNCTION__);
		return -1;
	}
	//printk("%s, %s, line %d, REG(NACR) = 0x%08x\n",__FILE__,__func__,__LINE__,rtk_readl(NACR));
	memset(rtkn->bbt_v2r, 0,  sizeof(BB_v2r)*block_v2r_num);
#endif
#ifdef CONFIG_RTK_NORMAL_BBT
	rtkn->BBT_PERCENT = 5;
	bbt_num = rtkn->bbt_num = ((u32)mtd->size/mtd->erasesize) * rtkn->BBT_PERCENT/100;
	printk("[%s, line %d], bbt_num=%d, this->BBT_PERCENT = %d\n",__func__,__LINE__,bbt_num,rtkn->BBT_PERCENT);
	rtkn->bbt_nor = kmalloc( sizeof(BBT_normal)*bbt_num, GFP_KERNEL );
	if ( !rtkn->bbt_nor ){
		printk("%s: Error, no enough memory for BBT Normal\n",__FUNCTION__);
		return -1;
	}
	memset(rtkn->bbt_nor, 0,  sizeof(BBT_normal)*bbt_num);
#endif


#if defined(CONFIG_RTK_REMAP_BBT)
	if(rtk_scan_v2r_bbt(mtd) < 0
		|| rtk_nand_scan_bbt(mtd) < 0)
	{
		printk("Error: realtek bbt scan fail\n");
		goto Error;
	}

	return 0;
#elif defined(CONFIG_RTK_NORMAL_BBT)
	if(nand_scan_normal_bbt(mtd) <0){
		printk("Error: realtek normal bbt scan fail\n");
		goto Error;
	}

	return 0;
#else
	return -1;
#endif

Error:
#ifdef CONFIG_RTK_REMAP_BBT
	if(rtkn->bbt_v2r){
		kfree(rtkn->bbt_v2r);
	}
	if(rtkn->bbt){
		kfree(rtkn->bbt);
	}
#endif
#ifdef CONFIG_RTK_NORMAL_BBT
	if(rtkn->bbt_nor){
		kfree(rtkn->bbt_nor);
	}
#endif
	return -1;
}


int rtkn_block_markbad(struct mtd_info *mtd, loff_t ofs)
{
	struct nand_chip *this = (struct nand_chip *)mtd->priv;
	struct rtknflash *rtkn = (struct rtknflash *)this->priv;
	int isLastPage = rtkn->chip_param.isLastPage;
	unsigned int page, block, page_offset;
	int i;
	//int rc = 0;
	//unsigned int oob_size = mtd->oobsize;
	unsigned int page_size = (1<<this->page_shift);
	unsigned int ppb = (1<<this->phys_erase_shift)/(1<<this->page_shift);

	//unsigned char buf[oob_size] __attribute__((__aligned__(4)));
	int chipnr, chipnr_remap;
	//nand_get_device(mtd, FL_WRITING);
#ifdef CONFIG_RTK_REMAP_BBT
	{
		i = (ofs >> this->phys_erase_shift);//virtual block index
		ofs = (rtkn->bbt_v2r[i].block_r << this->phys_erase_shift);//real block index, addr.
		printk("%s: blockv:%d blockr:%d ofs:%lld\n\r",__FUNCTION__,i,rtkn->bbt_v2r[i].block_r, ofs);
	}
#endif
	page = ((int) ofs) >> this->page_shift;
	chipnr = chipnr_remap = (int)(ofs >> this->chip_shift);
	page_offset = page & (ppb-1);
	block = page/ppb;

	//printk("%s: block:%d offs:%lld\n\r",__FUNCTION__,block, ofs);
#ifdef CONFIG_RTK_NORMAL_BBT
		for ( i=0; i<bbt_num; i++){
			if ( rtkn->bbt_nor[i].bad_block != BB_INIT ){
				if ( block == rtkn->bbt_nor[i].bad_block ){
					printk("%s: block:%d is already mark as bad!\n",__FUNCTION__,block);
					//nand_release_device(mtd);
					return 0;
				}
			}else
				break;
		}
#endif
	//this->select_chip(mtd, chipnr);
#ifdef CONFIG_RTK_NORMAL_BBT
	for( i=0; i<bbt_num; i++){
		if ( rtkn->bbt_nor[i].bad_block == BB_INIT){
			rtkn->bbt_nor[i].BB_die = chipnr;
			rtkn->bbt_nor[i].RB_die = chipnr;
			rtkn->bbt_nor[i].bad_block = page/ppb;
			rtkn->bbt_nor[i].block_info = 0xba;
			break;
		}
	}
	dump_normal_BBT(mtd);
	if ( rtk_update_normal_bbt (mtd, rtkn->bbt_nor) ){
		printk("[%s] rtk_update_normal_bbt() fail\n", __FUNCTION__);
		//nand_release_device(mtd);
		return -1;
	}
#endif

	/* do update bbt here,not used maybe*/
#if 0
#ifdef CONFIG_RTK_REMAP_BBT
		int block_remap = 0x12345678;
		if(check_BBT(mtd,page/ppb)==0)
		{

		    for( i=0; i<RBA; i++){
			    if ( rtkn->bbt[i].bad_block == BB_INIT && rtkn->bbt[i].remap_block != RB_INIT){
				/* do have one chip */
				#if 0
					if ( chipnr != chipnr_remap)
						this->bbt[i].BB_die = chipnr_remap;
					else
						this->bbt[i].BB_die = chipnr;
					#endif
				    rtkn->bbt[i].bad_block = page/ppb;
				    block_remap = rtkn->bbt[i].remap_block;
				    break;
			    }
		    }

		    if ( block_remap == 0x12345678 ){
			    printk("[%s] RBA do not have free remap block\n", __FUNCTION__);
			    return -1;
		    }

		    dump_BBT(mtd);

		    if ( rtk_update_bbt (mtd,rtkn->bbt) ){
			    printk("[%s] rtk_update_bbt() fails\n", __FUNCTION__);
			    return -1;
		    }
		}
#endif
#endif

	memset(rtkn->tmpBuf,0xff,MAX_RTKN_BUF_SIZE);
#ifdef SWAP_2K_DATA
	rtkn->tmpBuf[page_size+OOB_BBI_OFF] = 0x00;
#else
	rtkn->tmpBuf[page_size] = 0x00;
#endif

	if(isLastPage){
		rtk_scan_erase_bbt(mtd,block*ppb);
		rtk_scan_write_bbt(mtd,(block*ppb+ppb-1),page_size,rtkn->tmpBuf,&rtkn->tmpBuf[page_size]);
		rtk_scan_write_bbt(mtd,(block*ppb+ppb-2),page_size,rtkn->tmpBuf,&rtkn->tmpBuf[page_size]);
	}else{
		rtk_scan_erase_bbt(mtd,block*ppb);
		rtk_scan_write_bbt(mtd,(block*ppb),page_size,rtkn->tmpBuf,&rtkn->tmpBuf[page_size]);
		rtk_scan_write_bbt(mtd,((block*ppb)+1),page_size,rtkn->tmpBuf,&rtkn->tmpBuf[page_size]);
	}

	//nand_release_device(mtd);
	return 0;
}

int rtkn_block_bad(struct mtd_info *mtd, loff_t ofs, int getchip)
{
	//int chipnr=0, val=0;
	int val = 0;
#ifndef CONFIG_RTK_REMAP_BBT
	unsigned int page, block, page_offset,i;
	struct nand_chip *this = (struct nand_chip *)mtd->priv;
	struct rtknflash *rtkn = (struct rtknflash *)this->priv;
	unsigned int ppb = (mtd->erasesize)/(mtd->writesize);
#endif
	//nand_get_device(mtd, FL_READING);

#ifndef CONFIG_RTK_REMAP_BBT
	page = ((int) ofs) >> this->page_shift;
	page_offset = page & (ppb-1);
	block = page/ppb;
	for( i=0; i<bbt_num; i++){
		if ( rtkn->bbt_nor[i].bad_block != BB_INIT ){
			if ( block == rtkn->bbt_nor[i].bad_block ){
				printk("%s: block:%d is bad!\n",__FUNCTION__,block);
				//nand_release_device(mtd);
				return 1;
		}
		}else
			break;
	}
#endif
	//nand_release_device(mtd);
	return val;
}
//#endif
