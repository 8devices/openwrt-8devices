#include "rtknflash.h"
#include "rtkn_parallel_regs.h"
#include "rtkn_chip_param.h"

int cp_mode = CP_NF_NONE;
int read_retry_toshiba_cnt[]={0x00, 0x00, 0x04, 0x7c, 0x78, 0x74, 0x08};

#define VIR_TO_PHY(x) (x & (~0xa0000000))

//need check
static unsigned int monflg = 0x0;

/*---------------------------------------------------------------------------------------------------------------------------------*/
void WAIT_DONE(unsigned int addr, unsigned int mask, unsigned int value)
{
	while ( (REG_READ_U32(addr) & mask) != value )
	{
		asm("nop");
		smp_wmb();
	}
}

int is_jupiter_cpu(void)
{
	return 0;
}
//----------------------------------------------------------------------------
int is_saturn_cpu(void)
{
	return 0;
}

//----------------------------------------------------------------------------
int is_darwin_cpu(void)
{
	return 0;
}

//----------------------------------------------------------------------------

int is_macarthur_cpu(void)
{
	return 0;
}

//----------------------------------------------------------------------------
int is_nike_cpu(void)
{
	return 0;
}

//----------------------------------------------------------------------------
int is_venus_cpu(void)
{
	return 0;
}

//----------------------------------------------------------------------------
int is_neptune_cpu(void)
{
	return 0;
}

//----------------------------------------------------------------------------
int is_mars_cpu(void)
{
	return 0;
}

//----------------------------------------------------------------------------
int is_macarthur2_cpu(void)
{
	return 1;
}

void rtk_set_feature_micron(int P1){

	printk("[%s] enter \n", __FUNCTION__);

	REG_WRITE_U32(REG_DATA_TL0, 0x4);	//Set data transfer count[7:0]
	REG_WRITE_U32(REG_DATA_TL1, NF_DATA_TL1_access_mode(0x1) | NF_DATA_TL1_length1(0x0));	//Set data transfer count[13:8], SRAM path and access mode

	//Set PP
	REG_WRITE_U32(REG_READ_BY_PP, NF_READ_BY_PP_read_by_pp(0)); //Data read to 	DRAM from NAND through PP
	REG_WRITE_U32(REG_PP_CTL0, NF_PP_CTL0_pp_start_addr(0x0) | NF_PP_CTL0_pp_reset(0x0) | NF_PP_CTL0_pp_enable(0x1));	//Set PP starting address[9:8], PP reset and PP enable
	REG_WRITE_U32(REG_PP_CTL1, 0x0);	//Set PP starting assdress[7:0]

	//Set command
	REG_WRITE_U32(REG_ND_CMD, NF_ND_CMD_cmd(0xef));	//Set CMD1
	REG_WRITE_U32(REG_ND_CTL, NF_ND_CTL_xfer(1) | NF_ND_CTL_ecc_enable(0x0) | NF_ND_CTL_tran_mode(0x0));	//Enable Xfer, eanble ECC and set transfer mode
	WAIT_DONE(REG_ND_CTL, 0x80, 0x00);	//Wait xfer done

	//Set address,
	REG_WRITE_U32(REG_ND_PA0, 0x89);	//Set PA[7:0]
	REG_WRITE_U32(REG_ND_PA2, NF_ND_PA2_addr_mode(0x7));	//Set address mode & PA[20:16]
	REG_WRITE_U32(REG_ND_CTL, NF_ND_CTL_xfer(1) | NF_ND_CTL_ecc_enable(0x0) | NF_ND_CTL_tran_mode(0x1));	//Enable Xfer, eanble ECC and set transfer mode
	WAIT_DONE(REG_ND_CTL, 0x80, 0x00);	//Wait xfer done

	//Reset PP
	REG_WRITE_U32(REG_READ_BY_PP, NF_READ_BY_PP_read_by_pp(0)); //Data read to 	DRAM from NAND through PP
	REG_WRITE_U32(REG_PP_CTL0, NF_PP_CTL0_pp_start_addr(0x0) | NF_PP_CTL0_pp_reset(0x1) | NF_PP_CTL0_pp_enable(0x0));	//Set PP starting address[9:8], PP reset and PP enable

	//Set data
	REG_WRITE_U32(REG_SRAM_CTL,NF_SRAM_CTL_map_sel(1)|NF_SRAM_CTL_access_en(0x1) | NF_SRAM_CTL_mem_region(0x0));	//Enable direct access SRAM
	REG_WRITE_U32(REG_ND_PA0, P1);	//Set data
	REG_WRITE_U32(REG_SRAM_CTL, NF_SRAM_CTL_access_en(0x0));	//Disable direct access SRAM
	//Set PP
	REG_WRITE_U32(REG_PP_CTL0, NF_PP_CTL0_pp_start_addr(0x0) | NF_PP_CTL0_pp_reset(0) | NF_PP_CTL0_pp_enable(1));	//Set PP starting address[9:8], PP reset and PP enable
	REG_WRITE_U32(REG_PP_CTL0, NF_PP_CTL0_pp_start_addr(0x0));				//Set PP starting address[9:8]
	REG_WRITE_U32(REG_ND_CTL, NF_ND_CTL_xfer(1) | NF_ND_CTL_ecc_enable(0) | NF_ND_CTL_tran_mode(0x3));	//Enable Xfer, eanble ECC and set transfer mode
	WAIT_DONE(REG_ND_CTL, 0xc0, 0x40);	//Wait xfer done
}

/*---------------------------------------------------------------------------------------------------------------------------------*/

/* select chip cmd */
int rtkn_select_chip_cmd(int chipid){
	switch(chipid) {
		case -1:
			REG_WRITE_U32(REG_PD,0xff);
			break;
		case 1:
		case 2:
		case 3:
		case 4:
			REG_WRITE_U32(REG_PD, ~(1 << (chipid-1)));
			break;
		default:
			REG_WRITE_U32(REG_PD, ~(1 << 0));
	}

	return 0;
}

static void parallel_nand_set_chip_param(unsigned int id_chain)
{
	int i;

	for(i = 0;i < sizeof(nand_chip_id)/sizeof(nand_chip_param_T);i++)
	{
		if(id_chain == nand_chip_id[i].id){
			REG_WRITE_U32( REG_TIME_PARA1,nand_chip_id[i].T1);
			REG_WRITE_U32( REG_TIME_PARA2,nand_chip_id[i].T2);
			REG_WRITE_U32( REG_TIME_PARA3,nand_chip_id[i].T3);

			if(id_chain != HY27UT084G2M){
				REG_WRITE_U32( REG_MULTI_CHNL_MODE,0x20);
			}

			if(nand_chip_id[i].sycnmode == 0)
				REG_WRITE_U32(REG_NF_MODE, 0x0);
			else if(nand_chip_id[i].sycnmode == 1){
				// sync mode
				REG_WRITE_U32(REG_NF_MODE, 0x2);
				if(id_chain == MT29F64G08CBABB){
					REG_WRITE_U32(REG_TIME_PARA4, 0x2);
					REG_WRITE_U32(REG_MULTI_CHNL_MODE,NF_MULTI_CHNL_MODE_edo(0x00));
				}
			}

			rtkn->chip_param.id = nand_chip_id[i].id;
			rtkn->chip_param.isLastPage = nand_chip_id[i].isLastPage;
			rtkn->chip_param.ecc_num = nand_chip_id[i].ecc_num;
			rtkn->chip_param.T1 = nand_chip_id[i].T1;
			rtkn->chip_param.T2 = nand_chip_id[i].T2;
			rtkn->chip_param.T3 = nand_chip_id[i].T3;
			rtkn->chip_param.ecc_select = nand_chip_id[i].ecc_select;
			rtkn->chip_param.sycnmode = nand_chip_id[i].sycnmode;
			rtkn->chip_param.g_enReadRetrial = nand_chip_id[i].g_enReadRetrial;
			rtkn->chip_param.read_retry_cnt = 0;
			rtkn->chip_param.max_read_retry_cnt = nand_chip_id[i].max_read_retry_cnt;

			/* set feature */
			switch(id_chain){
			case MT29F64G08CBABA:
			case MT29F32G08CBADA:
				rtkn->chip_param.set_chip_pre_param	= NULL;
				rtkn->chip_param.set_chip_param = &rtk_set_feature_micron;
				rtkn->chip_param.set_chip_terminate_param = &rtk_set_feature_micron;
				break;
#if 0
			case TC58TEG6DCJT:
			case TC58TEG5DCJT:
			//case TC58TEG6DDK:
				rtkn->chip_param.set_chip_pre_param = &rtk_set_pre_condition_toshiba;
				rtkn->chip_param.set_chip_param = &rtk_set_pre_condition_toshiba;
				rtkn->chip_param.set_chip_terminate_param = &rtk_set_pre_condition_toshiba;
				break;
#endif
			default:
				rtkn->chip_param.set_chip_pre_param	= NULL;
				rtkn->chip_param.set_chip_param = NULL;
				rtkn->chip_param.set_chip_terminate_param = NULL;
			}

			break;
		}
	}

	if(i == sizeof(nand_chip_id)/sizeof(nand_chip_param_T)){
		printk("%s:%d,cannot fine id=%x\n",__func__,__LINE__,id_chain);
	}
}

/* read id cmd */
int parallel_nand_read_id_cmd(unsigned char* id_data)
{
	unsigned int id_chain1,id_chain2;

	REG_WRITE_U32(REG_DATA_TL0,NF_DATA_TL0_length0(0x06));
	REG_WRITE_U32(REG_DATA_TL1,NF_DATA_TL1_access_mode(0x01));

	//Set PP
	REG_WRITE_U32(REG_READ_BY_PP,NF_READ_BY_PP_read_by_pp(0x0));

	REG_WRITE_U32(REG_PP_CTL0,NF_PP_CTL0_pp_enable(0x01));
	REG_WRITE_U32(REG_PP_CTL1,NF_PP_CTL1_pp_start_addr(0));

	//Set command
	REG_WRITE_U32(REG_ND_CMD,NF_ND_CMD_cmd(CMD_READ_ID));
	REG_WRITE_U32(REG_ND_CTL,NF_ND_CTL_xfer(0x01));
	WAIT_DONE(REG_ND_CTL,0x80,0);

	//Set address
	REG_WRITE_U32(REG_ND_PA0, 0);
	REG_WRITE_U32(REG_ND_PA1, 0);
	REG_WRITE_U32(REG_ND_PA2,NF_ND_PA2_addr_mode(0x07));

	REG_WRITE_U32(REG_ND_CTL,NF_ND_CTL_xfer(1)|NF_ND_CTL_tran_mode(1));
	WAIT_DONE(REG_ND_CTL,0x80,0);
	//Enable XFER mode
	REG_WRITE_U32(REG_ND_CTL,NF_ND_CTL_xfer(1)|NF_ND_CTL_tran_mode(4));
	WAIT_DONE(REG_ND_CTL,0x80,0);

	//Reset PP
	REG_WRITE_U32(REG_PP_CTL0,NF_PP_CTL0_pp_reset(1));

	//Move data to DRAM from SRAM
	REG_WRITE_U32(REG_SRAM_CTL,NF_SRAM_CTL_map_sel(1)|NF_SRAM_CTL_access_en(1)|NF_SRAM_CTL_mem_region(0));

	id_chain1 = REG_READ_U32(REG_ND_PA0);

	//printk("id_chain 1 = 0x%x \n", id_chain);
	id_data[0] = id_chain1 & 0xff;
	id_data[1] = (id_chain1 >> 8) & 0xff;
	id_data[2] = (id_chain1 >> 16) & 0xff;
	id_data[3] = (id_chain1 >> 24) & 0xff;

	printf("id_chain1=%x\n",id_chain1);

	id_chain2 = REG_READ_U32(REG_ND_PA1);
	//printk("id_chain 2 = 0x%x \n", id_chain);
	id_data[4] = id_chain2 & 0xff;
	id_data[5] = (id_chain2 >> 8) & 0xff;
	id_data[6] = (id_chain2 >> 16) & 0xff;
	id_data[7] = (id_chain2 >> 24) & 0xff;
	REG_WRITE_U32(REG_SRAM_CTL,0x0);

	printf("id_chain2=%x\n",id_chain2);

	/* 97F little endian */
	id_chain1 = (id_data[0] << 24) | (id_data[1] <<16) | (id_data[2] <<8) | id_data[3];
	parallel_nand_set_chip_param(id_chain1);

	return 0;
}



/* reset cmd */
void parallel_nand_reset_cmd()
{
	/* need check */
	REG_WRITE_U32( REG_ND_CMD,0xFF);
	REG_WRITE_U32( REG_ND_CTL,(NF_ND_CTL_xfer(0x01) | NF_ND_CTL_tran_mode(0)) );
	WAIT_DONE(REG_ND_CTL,0x80,0);
}

/* read status cmd */
void parallel_nand_read_status()
{
	int status;
	REG_WRITE_U32(REG_DATA_TL0,NF_DATA_TL0_length0(0x04));
	REG_WRITE_U32(REG_DATA_TL1,NF_DATA_TL1_access_mode(0x01));

	//Set PP
	REG_WRITE_U32(REG_READ_BY_PP,NF_READ_BY_PP_read_by_pp(0x0));

	REG_WRITE_U32(REG_PP_CTL0,NF_PP_CTL0_pp_enable(0x01));
	REG_WRITE_U32(REG_PP_CTL1,NF_PP_CTL1_pp_start_addr(0));

	//Set command
	REG_WRITE_U32(REG_ND_CMD,NF_ND_CMD_cmd(CMD_READ_STATUS));
	REG_WRITE_U32(REG_ND_CTL,NF_ND_CTL_xfer(0x01));
	WAIT_DONE(REG_ND_CTL,0x80,0);

	//Set address
	REG_WRITE_U32(REG_ND_PA0, 0);
	REG_WRITE_U32(REG_ND_PA1, 0);
	REG_WRITE_U32(REG_ND_PA2,NF_ND_PA2_addr_mode(0x07));

	REG_WRITE_U32(REG_ND_CTL,NF_ND_CTL_xfer(1)|NF_ND_CTL_tran_mode(1));
	WAIT_DONE(REG_ND_CTL,0x80,0);
	//Enable XFER mode
	REG_WRITE_U32(REG_ND_CTL,NF_ND_CTL_xfer(1)|NF_ND_CTL_tran_mode(4));
	WAIT_DONE(REG_ND_CTL,0x80,0);

	//Reset PP
	REG_WRITE_U32(REG_PP_CTL0,NF_PP_CTL0_pp_reset(1));

	//Move data to DRAM from SRAM
	REG_WRITE_U32(REG_SRAM_CTL,NF_SRAM_CTL_map_sel(1)|NF_SRAM_CTL_access_en(1)|NF_SRAM_CTL_mem_region(0));

	status = REG_READ_U32(REG_ND_PA0);
	rtkn->status = status  & 0xff;

	REG_WRITE_U32(REG_SRAM_CTL,0x0);

	return;
}

/* erase cmd */
int paralledl_nand_erase_cmd(int page_addr)
{

	REG_WRITE_U32( REG_MULTI_CHNL_MODE,NF_MULTI_CHNL_MODE_no_wait_busy(1)|NF_MULTI_CHNL_MODE_edo(1));

	REG_WRITE_U32( REG_ND_CMD,NF_ND_CMD_cmd(CMD_BLK_ERASE_C1) );
	REG_WRITE_U32( REG_CMD2,NF_CMD2_cmd2(CMD_BLK_ERASE_C2) );
	REG_WRITE_U32( REG_CMD3,NF_CMD3_cmd3(CMD_BLK_ERASE_C3) );

	REG_WRITE_U32( REG_ND_PA0,NF_ND_PA0_page_addr0(page_addr) );
	REG_WRITE_U32( REG_ND_PA1,NF_ND_PA1_page_addr1(page_addr>>8) );
	REG_WRITE_U32( REG_ND_PA2,NF_ND_PA2_addr_mode(0x04)|NF_ND_PA2_page_addr2(page_addr>>16) );
	REG_WRITE_U32( REG_ND_PA3,NF_ND_PA3_page_addr3((page_addr>> 21)&0x7) );

	REG_WRITE_U32( REG_AUTO_TRIG,NF_AUTO_TRIG_auto_trig(1)|NF_AUTO_TRIG_spec_auto_case(1)|NF_AUTO_TRIG_auto_case(2) );
	WAIT_DONE(REG_AUTO_TRIG,0x80,0);

	REG_WRITE_U32(REG_POLL_FSTS,NF_POLL_FSTS_bit_sel(6)|NF_POLL_FSTS_trig_poll(1) );
	WAIT_DONE(REG_POLL_FSTS,0x01,0x0);
	WAIT_DONE(REG_ND_CTL,0x40,0x40);//add by alexchang 0416-2010

	if ( REG_READ_U32(REG_ND_DAT) & 0x01 ){
		printk("[%s] erase is not completed at page %d\n", __FUNCTION__, page_addr);
		return -1;
	}

	return 0;
}

/* read page */
int parallel_nand_read_page(struct rtknflash *rtkn,unsigned char* buf,unsigned char* oobBuf,int page)
{
	struct mtd_info* mtd = rtkn->mtd;
	int page_len,page_size,oob_size,dma_len;
	uint8_t	auto_trigger_mode = 2;
	uint8_t	bChkAllOne = 0;
	int dram_sa,oob_sa;
	uint8_t addr_mode = 1;
	unsigned int is_sync_mode;
	page_size = mtd->writesize,oob_size = mtd->oobsize;
	rtkn->chip_param.read_retry_cnt = 0;


	if (((uint32_t)buf&0x7)!=0) {
		printk("[%s]data_buf must 8 byte alignmemt!!\n",__FUNCTION__);
		BUG();
	}

	while (1)
	{
		REG_WRITE_U32(REG_DATA_TL0,NF_DATA_TL0_length0(0));
		if(rtkn->chip_param.ecc_select >= 0x18)
		{
			if (bChkAllOne) {
				// disable randomizer
				REG_WRITE_U32(REG_RMZ_CTRL, 0);
				REG_WRITE_U32(REG_BLANK_CHK,NF_BLANK_CHK_blank_ena(1)|NF_BLANK_CHK_read_ecc_xnor_ena(1));
				//printk("[DBG]ecc error, set xnor and blank to 1, REG_BLANK_CHK reg: 0x%x\n", REG_READ_U32(REG_BLANK_CHK));
			}
			else {
				// enable randomizer
				REG_WRITE_U32(REG_RMZ_CTRL, 1);
				REG_WRITE_U32(REG_BLANK_CHK,NF_BLANK_CHK_blank_ena(1)|NF_BLANK_CHK_read_ecc_xnor_ena(0));
				//printk("[DBG]set xnor to 0, REG_BLANK_CHK reg: 0x%x\n", REG_READ_U32(REG_BLANK_CHK));
			}

			page_len = page_size >> 10;
			REG_WRITE_U32(REG_DATA_TL1,NF_DATA_TL1_access_mode(1)|NF_DATA_TL1_length1(4));
		}
		else
		{
			// set random read

			REG_WRITE_U32(REG_RND_EN, 1);
			REG_WRITE_U32(REG_RND_CMD1, 0x5);
			REG_WRITE_U32(REG_RND_CMD2, 0xe0);
			REG_WRITE_U32(REG_RND_DATA_STR_COL_H, 0);	// data start address MSB (always 0)
			REG_WRITE_U32(REG_RND_SPR_STR_COL_H, page_size >> 8);	// spare start address MSB
			REG_WRITE_U32(REG_RND_SPR_STR_COL_L, page_size & 0xff); // spare start address LSB
			REG_WRITE_U32(REG_RMZ_CTRL, 0);

			page_len = page_size >> 9;
			REG_WRITE_U32(REG_DATA_TL1,NF_DATA_TL1_access_mode(1)|NF_DATA_TL1_length1(2));
		}

		REG_WRITE_U32(REG_PAGE_LEN,NF_PAGE_LEN_page_len(page_len));

		//Set PP
		REG_WRITE_U32(REG_READ_BY_PP,NF_READ_BY_PP_read_by_pp(1));
		REG_WRITE_U32(REG_PP_CTL1,NF_PP_CTL1_pp_start_addr(0));
		REG_WRITE_U32(REG_PP_CTL0,0);

		// enable blank check
		//REG_WRITE_U32(REG_BLANK_CHK,NF_BLANK_CHK_blank_ena(1) );		//need check

		//Set command
		REG_WRITE_U32(REG_ND_CMD,NF_ND_CMD_cmd(CMD_PG_READ_C1));
		REG_WRITE_U32(REG_CMD2,NF_CMD2_cmd2(CMD_PG_READ_C2));
		REG_WRITE_U32(REG_CMD3,NF_CMD3_cmd3(CMD_PG_READ_C3));

		//Set address
		REG_WRITE_U32( REG_ND_PA0,NF_ND_PA0_page_addr0( 0xff&page ));
		REG_WRITE_U32( REG_ND_PA1,NF_ND_PA1_page_addr1( 0xff&(page>>8) ));
		REG_WRITE_U32( REG_ND_PA2,NF_ND_PA2_addr_mode(addr_mode)|NF_ND_PA2_page_addr2( 0x1f&(page>>16) ));
		REG_WRITE_U32( REG_ND_PA3,NF_ND_PA3_page_addr3( 0x7&(page>>21) ));

		REG_WRITE_U32(REG_ND_CA0, 0);
		REG_WRITE_U32(REG_ND_CA1, 0);

		is_sync_mode = (REG_READ_U32(REG_NF_MODE) & 0x2);
		//Set ECC
		if(monflg == 5566)
		{
			printk("set ECC NO CHECK read \n");
			REG_WRITE_U32( REG_MULTI_CHNL_MODE,NF_MULTI_CHNL_MODE_edo(1)|NF_MULTI_CHNL_MODE_ecc_no_check(1) | NF_MULTI_CHNL_MODE_ecc_pass(1));//add by alexchang0205-2010.
			//REG_WRITE_U32(REG_RMZ_CTRL, 1);
		}
		REG_WRITE_U32( REG_MULTI_CHNL_MODE,NF_MULTI_CHNL_MODE_edo(1));//add by alexchang0205-2010.
		if(is_sync_mode == 0x2)
			REG_WRITE_U32(REG_MULTI_CHNL_MODE,REG_READ_U32(REG_MULTI_CHNL_MODE) & (~(1<<5)));//sync mode doesn't support edo

		switch(rtkn->chip_param.ecc_select) {
			case 6:
				REG_WRITE_U32(REG_ECC_SEL, 0x0);
				break;
			case 12:
				REG_WRITE_U32(REG_ECC_SEL, 0x1);
				break;
			case 16:
				REG_WRITE_U32(REG_ECC_SEL, 0xe);
				break;
			case 24:
				REG_WRITE_U32(REG_ECC_SEL, 0xa);
				break;
			case 40:
				REG_WRITE_U32(REG_ECC_SEL, 0x2);
				break;
			case 43:
				REG_WRITE_U32(REG_ECC_SEL, 0x4);
				break;
			case 65:
				REG_WRITE_U32(REG_ECC_SEL, 0x6);
				break;
			case 72:
				REG_WRITE_U32(REG_ECC_SEL, 0x8);
				break;
			default:
				REG_WRITE_U32(REG_ECC_SEL, 0x0);
				break;
		}

		/* set oobbuf */
		oob_sa = ( VIR_TO_PHY((uint32_t)oobBuf) >> 3);
		if(monflg == 5566)
			REG_WRITE_U32( REG_SPR_DDR_CTL,NF_SPR_DDR_CTL_spare_ddr_ena(0)|NF_SPR_DDR_CTL_per_2k_spr_ena(0)|NF_SPR_DDR_CTL_spare_dram_sa(oob_sa));
		else
			REG_WRITE_U32( REG_SPR_DDR_CTL,NF_SPR_DDR_CTL_spare_ddr_ena(1)|NF_SPR_DDR_CTL_per_2k_spr_ena(0)|NF_SPR_DDR_CTL_spare_dram_sa(oob_sa));

		/* set buf */
		dram_sa = ( VIR_TO_PHY((uint32_t)buf) >> 3);
		REG_WRITE_U32(REG_DMA_CTL1,NF_DMA_CTL1_dram_sa(dram_sa));
		dma_len = page_size >> 9;
		REG_WRITE_U32(REG_DMA_CTL2,NF_DMA_CTL2_dma_len(dma_len));
		REG_WRITE_U32(REG_DMA_CTL3,NF_DMA_CTL3_ddr_wr(1)|NF_DMA_CTL3_dma_xfer(1));

		// flush cache.
		//dma_cache_inv(buf,page_size);
		//dma_cache_inv(oobBuf,oob_size);
		dma_cache_wback_inv((unsigned long)buf,(unsigned long)page_size);
		dma_cache_wback_inv((unsigned long)oobBuf,(unsigned long)oob_size);

		//Enable Auto mode
		REG_WRITE_U32( REG_AUTO_TRIG,NF_AUTO_TRIG_auto_trig(1)|NF_AUTO_TRIG_spec_auto_case(0)| NF_AUTO_TRIG_auto_case(auto_trigger_mode));
		WAIT_DONE(REG_AUTO_TRIG,0x80,0);
		WAIT_DONE(REG_DMA_CTL3,0x01,0);

		if (REG_READ_U32(REG_BLANK_CHK) & 0x2){
			/* data all 0xff */
			//printk("data all one \n");
			REG_WRITE_U32(REG_BLANK_CHK,NF_BLANK_CHK_blank_ena(1)|NF_BLANK_CHK_read_ecc_xnor_ena(0));
			return 0;
		}
		else if (REG_READ_U32(REG_ND_ECC) & 0x8) {
			if (!bChkAllOne && rtkn->chip_param.ecc_select>=0x18) {
				bChkAllOne = 1;
				continue;
			}
			if (rtkn->chip_param.g_enReadRetrial) {
				bChkAllOne = 0;
				if(rtkn->chip_param.read_retry_cnt == 0){	//set pre condition
					if(rtkn->chip_param.set_chip_pre_param != NULL)
						rtkn->chip_param.set_chip_pre_param(0);
				}
				rtkn->chip_param.read_retry_cnt++;
				if (rtkn->chip_param.read_retry_cnt <= rtkn->chip_param.max_read_retry_cnt) {
					if(rtkn->chip_param.set_chip_param != NULL)
						rtkn->chip_param.set_chip_param(rtkn->chip_param.read_retry_cnt);
					continue;
				}
			}
			printk("[DBG]ecc error... page=0x%x, REG_BLANK_CHK reg: 0x%x \n", page, (unsigned int)REG_READ_U32(REG_BLANK_CHK));
			if (rtkn->chip_param.g_enReadRetrial) {
				if (rtkn->chip_param.read_retry_cnt !=0) {
					if(rtkn->chip_param.set_chip_terminate_param != NULL)
						rtkn->chip_param.set_chip_terminate_param(0);

				}
			}
			REG_WRITE_U32(REG_BLANK_CHK,NF_BLANK_CHK_blank_ena(1)|NF_BLANK_CHK_read_ecc_xnor_ena(0));
			return -1;	// ECC not correctable
		}
		else {
			if (rtkn->chip_param.g_enReadRetrial) {
				if (rtkn->chip_param.read_retry_cnt !=0) {
					if(rtkn->chip_param.set_chip_terminate_param != NULL)
						rtkn->chip_param.set_chip_terminate_param(0);
				}
			}
			REG_WRITE_U32(REG_BLANK_CHK,NF_BLANK_CHK_blank_ena(1)|NF_BLANK_CHK_read_ecc_xnor_ena(0));
			return 0;
		}
	}

	return 0;
}

int parallel_nand_write_page(struct rtknflash *rtkn,unsigned char* buf,unsigned char* oobBuf,int page)
{
	struct mtd_info *mtd = (struct mtd_info*)rtkn->mtd;
	unsigned int page_size = mtd->writesize;
	unsigned int oob_size = mtd->oobsize;
	//struct nand_chip *this = (struct nand_chip *) mtd->priv;
	//int rc = 0, i;
	uint8_t auto_trigger_mode = 1;
	uint8_t addr_mode = 1;

	unsigned int page_len, dram_sa, dma_len, spare_dram_sa;
	//unsigned char oob_1stB;

	//unsigned char nf_oob_buf[oob_size];
	//unsigned int chip_section = 0;
	//unsigned int section = 0;
	//unsigned int index = 0;
	unsigned int is_sync_mode;
	//printk("[%s] entry... page : %d\n", __FUNCTION__, page);

	if(((uint32_t)buf&0x7)!=0) {
		printk("[%s]data_buf must 8 byte alignmemt!!\n",__FUNCTION__);
		BUG();
	}
	if(((uint32_t)oobBuf&0x7)!=0) {
		printk("[%s]oob_buf must 8 byte alignmemt!!\n",__FUNCTION__);
		BUG();
	}

	REG_WRITE_U32(REG_DATA_TL0,NF_DATA_TL0_length0(0));
	if(rtkn->chip_param.ecc_select >= 0x18)
	{
		// enable randomizer
		REG_WRITE_U32(REG_RMZ_CTRL, 1);

		page_len = page_size >> 10;
		REG_WRITE_U32(REG_DATA_TL1,NF_DATA_TL1_length1(4));
	}
	else
	{
		// set random write
		REG_WRITE_U32(REG_RND_EN, 1);
		REG_WRITE_U32(REG_RND_CMD1, 0x85);
		REG_WRITE_U32(REG_RND_DATA_STR_COL_H, 0);	// data start address MSB (always 0)
		REG_WRITE_U32(REG_RND_SPR_STR_COL_H, page_size >> 8);	// spare start address MSB
		REG_WRITE_U32(REG_RND_SPR_STR_COL_L, page_size & 0xff); // spare start address LSB

		page_len = page_size >> 9;
		REG_WRITE_U32(REG_DATA_TL1,NF_DATA_TL1_length1(2));
	}

	REG_WRITE_U32( REG_PAGE_LEN,NF_PAGE_LEN_page_len(page_len));

	//Set PP
	REG_WRITE_U32(REG_READ_BY_PP,NF_READ_BY_PP_read_by_pp(0));//add by alexchang 0208-2010
	REG_WRITE_U32(REG_PP_CTL1,NF_PP_CTL1_pp_start_addr(0));//add by alexchang 0208-2010
	REG_WRITE_U32(REG_PP_CTL0,0);//add by alexchang 0208-2010

	//Set command
	REG_WRITE_U32(REG_ND_CMD,NF_ND_CMD_cmd(CMD_PG_WRITE_C1));
	REG_WRITE_U32(REG_CMD2,NF_CMD2_cmd2(CMD_PG_WRITE_C2));
	REG_WRITE_U32(REG_CMD3,NF_CMD3_cmd3(CMD_PG_WRITE_C3));

	//Set address
	REG_WRITE_U32( REG_ND_PA0,NF_ND_PA0_page_addr0(page));
	REG_WRITE_U32( REG_ND_PA1,NF_ND_PA1_page_addr1(page>>8));
	REG_WRITE_U32( REG_ND_PA2,NF_ND_PA2_addr_mode(addr_mode)|NF_ND_PA2_page_addr2(page>>16));
	REG_WRITE_U32( REG_ND_PA3,NF_ND_PA3_page_addr3((page>>21)&0x7));
	REG_WRITE_U32(REG_ND_CA0, 0);
	REG_WRITE_U32(REG_ND_CA1, 0);

	//Set ECC

	// no ECC NF_MULTI_CHNL_MODE_ecc_pass(1)
	is_sync_mode = (REG_READ_U32(REG_NF_MODE) & 0x2);
	if(monflg == 5566)
	{
		printk("Set write ECC PASS \n");
		REG_WRITE_U32(REG_MULTI_CHNL_MODE,NF_MULTI_CHNL_MODE_edo(1)|NF_MULTI_CHNL_MODE_ecc_pass(1) | NF_MULTI_CHNL_MODE_ecc_no_check(1));
		//REG_WRITE_U32(REG_RMZ_CTRL, 1);
	}
	else
		REG_WRITE_U32(REG_MULTI_CHNL_MODE,NF_MULTI_CHNL_MODE_edo(1));//add by alexchang0205-2010

	if(is_sync_mode == 0x2)
		REG_WRITE_U32(REG_MULTI_CHNL_MODE,REG_READ_U32(REG_MULTI_CHNL_MODE) & (~(1<<5)));//sync mode doesn't support edo

	REG_WRITE_U32( REG_ECC_STOP,NF_ECC_STOP_ecc_n_stop(0x01));

	switch(rtkn->chip_param.ecc_select) {
		case 6:
			REG_WRITE_U32(REG_ECC_SEL, 0x0);
			break;
		case 12:
			REG_WRITE_U32(REG_ECC_SEL, 0x1);
			break;
		case 16:
			REG_WRITE_U32(REG_ECC_SEL, 0xe);
			break;
		case 24:
			REG_WRITE_U32(REG_ECC_SEL, 0xa);
			break;
		case 40:
			REG_WRITE_U32(REG_ECC_SEL, 0x2);
			break;
		case 43:
			REG_WRITE_U32(REG_ECC_SEL, 0x4);
			break;
		case 65:
			REG_WRITE_U32(REG_ECC_SEL, 0x6);
			break;
		case 72:
			REG_WRITE_U32(REG_ECC_SEL, 0x8);
			break;
		default:
			REG_WRITE_U32(REG_ECC_SEL, 0x0);
			break;
	}

	// flush cache.
	dma_cache_wback_inv((unsigned long)buf, (unsigned long)page_size);
	dma_cache_wback_inv((unsigned long)oobBuf, (unsigned long)oob_size);

	dram_sa = ( VIR_TO_PHY((uint32_t)buf) >> 3);
	REG_WRITE_U32(REG_DMA_CTL1,NF_DMA_CTL1_dram_sa(dram_sa));
	dma_len = page_size >> 9;
	REG_WRITE_U32(REG_DMA_CTL2,NF_DMA_CTL2_dma_len(dma_len));

	spare_dram_sa = ( VIR_TO_PHY((uint32_t)oobBuf) >> 3);
	if(monflg == 5566)
		REG_WRITE_U32(REG_SPR_DDR_CTL,NF_SPR_DDR_CTL_spare_ddr_ena(0)|NF_SPR_DDR_CTL_per_2k_spr_ena(0)|NF_SPR_DDR_CTL_spare_dram_sa(spare_dram_sa));
	else
		REG_WRITE_U32(REG_SPR_DDR_CTL,NF_SPR_DDR_CTL_spare_ddr_ena(1)|NF_SPR_DDR_CTL_per_2k_spr_ena(1)|NF_SPR_DDR_CTL_spare_dram_sa(spare_dram_sa));

	REG_WRITE_U32(REG_DMA_CTL3,NF_DMA_CTL3_ddr_wr(0)|NF_DMA_CTL3_dma_xfer(1));
	REG_WRITE_U32( REG_AUTO_TRIG,NF_AUTO_TRIG_auto_trig(1)|NF_AUTO_TRIG_spec_auto_case(0)| NF_AUTO_TRIG_auto_case(auto_trigger_mode));

	WAIT_DONE(REG_AUTO_TRIG,0x80,0);
	WAIT_DONE(REG_DMA_CTL3,0x01,0);
	REG_WRITE_U32(REG_POLL_FSTS,NF_POLL_FSTS_bit_sel(6)|NF_POLL_FSTS_trig_poll(1) );
	WAIT_DONE(REG_POLL_FSTS,0x01,0x0);
	WAIT_DONE(REG_ND_CTL,0x40,0x40);//add by alexchang 0416-2010

	return 0;
}

/*---------------------------------------------------------------------------------------------------------------------------------*/
