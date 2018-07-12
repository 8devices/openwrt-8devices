/*++
Copyright (c) Realtek Semiconductor Corp. All rights reserved.

Module Name:
	Hal88XXPhyCfg.c

Abstract:
	Defined HAL 88XX PHY BB setting functions

Major Change History:
	When       Who               What
	---------- ---------------   -------------------------------
	2012-11-14 Eric              Create.
--*/

#if !defined(__ECOS) && !defined(CPTCFG_CFG80211_MODULE)
#include "HalPrecomp.h"
#else
#include "../HalPrecomp.h"
#endif


#ifdef TXPWR_LMT

void HAL_get_tx_pwr(unsigned int tmp_dw, unsigned char *tmp_b1, unsigned char *tmp_b2,
					unsigned char *tmp_b3, unsigned char *tmp_b4)
{
	*tmp_b1 = (tmp_dw & (0xff));
	*tmp_b2 = ((tmp_dw & (0xff00)) >> 8);
	*tmp_b3 = ((tmp_dw & (0xff0000)) >> 16);
	*tmp_b4 = ((tmp_dw & (0xff000000)) >> 24);
}


unsigned char HAL_get_byte_from_dw(unsigned int tmp_dw, unsigned char byte_num)
{

	if (byte_num == 0)
		return (tmp_dw & (0xff));
	else if (byte_num == 1)
		return ((tmp_dw & (0xff00)) >> 8);
	else if (byte_num == 2)
		return ((tmp_dw & (0xff0000)) >> 16);
	else if (byte_num == 3)
		return ((tmp_dw & (0xff000000)) >> 24);
	else
		return 0;
}

unsigned int HAL_assign_lmt_reg_value(IN  HAL_PADAPTER	Adapter, unsigned int reg_offset, unsigned char max_idx, unsigned char max_idx2)
{

	unsigned int tmp_dw;
	unsigned char reg_val[4];
	unsigned char i = 0;

	tmp_dw = HAL_RTL_R32(reg_offset);

	//printk("[0x%03x] 0x%08x >> ", reg_offset, tmp_dw);

	HAL_get_tx_pwr(tmp_dw, (unsigned char *)&reg_val[0], (unsigned char *)&reg_val[1],
				   (unsigned char *)&reg_val[2], (unsigned char *)&reg_val[3]);

#if IS_RTL88XX_N
	if (reg_offset == REG_BB_TXAGC_A_CCK1_MCS32_N) {
		reg_val[1] = POWER_RANGE_CHECK(POWER_MIN_CHECK(reg_val[1], max_idx));
	} else if (reg_offset == REG_BB_TXAGC_B_CCK5_1_MCS32_N) {
		reg_val[1] = POWER_RANGE_CHECK(POWER_MIN_CHECK(reg_val[1], max_idx));
		reg_val[2] = POWER_RANGE_CHECK(POWER_MIN_CHECK(reg_val[2], max_idx));
		reg_val[3] = POWER_RANGE_CHECK(POWER_MIN_CHECK(reg_val[3], max_idx));
	} else if (reg_offset == REG_BB_TXAGC_A_CCK11_2_B_CCK11_N) {
		//if (GetChipIDMIMO88XX(Adapter) == MIMO_2T2R)
		reg_val[0] = POWER_RANGE_CHECK(POWER_MIN_CHECK(reg_val[0], max_idx2));
		reg_val[1] = POWER_RANGE_CHECK(POWER_MIN_CHECK(reg_val[1], max_idx));
		reg_val[2] = POWER_RANGE_CHECK(POWER_MIN_CHECK(reg_val[2], max_idx));
		reg_val[3] = POWER_RANGE_CHECK(POWER_MIN_CHECK(reg_val[3], max_idx));
	} else
#endif
#if IS_RTL88XX_AC
		if ((reg_offset == REG_BB_TXAGC_A_NSS2INDEX1_NSS1INDEX8_AC) ||
				(reg_offset == REG_BB_TXAGC_B_NSS2INDEX1_NSS1INDEX8_AC)) {
			reg_val[0] = POWER_RANGE_CHECK(POWER_MIN_CHECK(reg_val[0], max_idx));
			reg_val[1] = POWER_RANGE_CHECK(POWER_MIN_CHECK(reg_val[1], max_idx));
			reg_val[2] = POWER_RANGE_CHECK(POWER_MIN_CHECK(reg_val[2], max_idx2));
			reg_val[3] = POWER_RANGE_CHECK(POWER_MIN_CHECK(reg_val[3], max_idx2));
		} else
#endif
		{
			for (i = 0; i < 4; i++) {
				reg_val[i] = POWER_RANGE_CHECK(POWER_MIN_CHECK(reg_val[i], max_idx));
			}
		}

	tmp_dw = (reg_val[3] << 24) | (reg_val[2] << 16) | (reg_val[1] << 8) | reg_val[0];

	//printk("0x%08x\n", tmp_dw);

	HAL_RTL_W32(reg_offset, tmp_dw);

}

unsigned char HAL_check_lmt_valid(IN  HAL_PADAPTER	Adapter, unsigned char phy_band)
{
	unsigned char lmt_valid = 1;

	if (phy_band == PHY_BAND_2G)
		if (Adapter->pshare->txpwr_lmt_CCK == 0)
			lmt_valid = 0;

	if (Adapter->pshare->txpwr_lmt_OFDM == 0)
		lmt_valid = 0;

	if (Adapter->pshare->txpwr_lmt_HT1S == 0)
		lmt_valid = 0;

	if (Adapter->pshare->txpwr_lmt_HT2S == 0)
		lmt_valid = 0;

#if IS_RTL8814A_SERIES
    if (GET_CHIP_VER(Adapter) == VERSION_8814A) {
        if(Adapter->pshare->txpwr_lmt_HT3S == 0)
            lmt_valid = 0;
    }
#endif

	
	if (phy_band == PHY_BAND_5G) {
		if (Adapter->pshare->txpwr_lmt_VHT1S == 0)
			lmt_valid = 0;

		if (Adapter->pshare->txpwr_lmt_VHT2S == 0)
			lmt_valid = 0;
#if IS_RTL8814A_SERIES
        if (GET_CHIP_VER(Adapter) == VERSION_8814A) {
            if(Adapter->pshare->txpwr_lmt_VHT3S == 0)
                lmt_valid = 0;
        }
#endif
	}
#if 1

	if ((Adapter->pmib->dot11StationConfigEntry.dot11RegDomain != DOMAIN_TEST) 
	&& lmt_valid == 0) {
		panic_printk("NO Limit Value, NO limit TX POWER !!!\n limit for cck=%d, ofdm=%d, ht1s=%d, ht2s=%d, vht1s=%d, vht2s=%d\n",
			   Adapter->pshare->txpwr_lmt_CCK,
			   Adapter->pshare->txpwr_lmt_OFDM,
			   Adapter->pshare->txpwr_lmt_HT1S,
			   Adapter->pshare->txpwr_lmt_HT2S,
			   Adapter->pshare->txpwr_lmt_VHT1S,
			   Adapter->pshare->txpwr_lmt_VHT2S
			  );
	}
	
#endif
	return lmt_valid;

}


#if IS_RTL88XX_AC
void TxLMT_OFDM_Jaguar_PathX_AC(
    IN  HAL_PADAPTER	Adapter,
    IN  u1Byte          bPathXEnIdx
    )
{

	unsigned int  tmp_dw = 0;
 
	unsigned char	idx_odfm_54m = 0;
	unsigned char	idx_ht_mcs7 = 0;
	unsigned char	idx_ht_mcs15 = 0;
	unsigned char	idx_vht_1ss_mcs7 =0;
	unsigned char	idx_vht_2ss_mcs7 =0;

	char	lmt_pg_idx_ofdm = 0;
	char	lmt_pg_idx_ht1s, lmt_pg_idx_ht2s = 0;
	char	lmt_pg_idx_vht1s, lmt_pg_idx_vht2s = 0;

	char	max_lmt_idx_ofdm = 0;
	char	max_lmt_idx_ht1s, max_lmt_idx_ht2s = 0;
	char	max_lmt_idx_vht1s, max_lmt_idx_vht2s = 0;

    // for MIMO 3T3R, 4T4R
	unsigned char	idx_vht_3ss_mcs7 =0;    
	unsigned char	idx_vht_4ss_mcs7 =0;  
	char	        lmt_pg_idx_ht3s, lmt_pg_idx_ht4s = 0;
	char	        lmt_pg_idx_vht3s, lmt_pg_idx_vht4s = 0;
	char	max_lmt_idx_ht3s, max_lmt_idx_ht4s = 0;
	char	max_lmt_idx_vht3s, max_lmt_idx_vht4s = 0;

    u4Byte REG_BB_TXAGC_X_OFDM18_OFDM6_AC[4] =  {REG_BB_TXAGC_A_OFDM18_OFDM6_AC, REG_BB_TXAGC_B_OFDM18_OFDM6_AC, REG_BB_TXAGC_C_OFDM18_OFDM6_AC, REG_BB_TXAGC_D_OFDM18_OFDM6_AC};
    u4Byte REG_BB_TXAGC_X_OFDM54_OFDM24_AC[4] = {REG_BB_TXAGC_A_OFDM54_OFDM24_AC, REG_BB_TXAGC_B_OFDM54_OFDM24_AC, REG_BB_TXAGC_C_OFDM54_OFDM24_AC, REG_BB_TXAGC_D_OFDM54_OFDM24_AC};

    u4Byte REG_BB_TXAGC_X_MCS3_MCS0_AC[4] =     {REG_BB_TXAGC_A_MCS3_MCS0_AC, REG_BB_TXAGC_B_MCS3_MCS0_AC, REG_BB_TXAGC_C_MCS3_MCS0_AC, REG_BB_TXAGC_D_MCS3_MCS0_AC};
    u4Byte REG_BB_TXAGC_X_MCS7_MCS4_AC[4] =     {REG_BB_TXAGC_A_MCS7_MCS4_AC, REG_BB_TXAGC_B_MCS7_MCS4_AC, REG_BB_TXAGC_C_MCS7_MCS4_AC, REG_BB_TXAGC_D_MCS7_MCS4_AC};
    u4Byte REG_BB_TXAGC_X_MCS11_MCS8_AC[4] =    {REG_BB_TXAGC_A_MCS11_MCS8_AC, REG_BB_TXAGC_B_MCS11_MCS8_AC, REG_BB_TXAGC_C_MCS11_MCS8_AC, REG_BB_TXAGC_D_MCS11_MCS8_AC};
    u4Byte REG_BB_TXAGC_X_MCS15_MCS12_AC[4] =   {REG_BB_TXAGC_A_MCS15_MCS12_AC, REG_BB_TXAGC_B_MCS15_MCS12_AC, REG_BB_TXAGC_C_MCS15_MCS12_AC, REG_BB_TXAGC_D_MCS15_MCS12_AC};

    u4Byte REG_BB_TXAGC_X_NSS1INDEX3_NSS1INDEX0_AC[4] = {REG_BB_TXAGC_A_NSS1INDEX3_NSS1INDEX0_AC, REG_BB_TXAGC_B_NSS1INDEX3_NSS1INDEX0_AC, REG_BB_TXAGC_C_NSS1INDEX3_NSS1INDEX0_AC, REG_BB_TXAGC_D_NSS1INDEX3_NSS1INDEX0_AC};
    u4Byte REG_BB_TXAGC_X_NSS1INDEX7_NSS1INDEX4_AC[4] = {REG_BB_TXAGC_A_NSS1INDEX7_NSS1INDEX4_AC, REG_BB_TXAGC_B_NSS1INDEX7_NSS1INDEX4_AC, REG_BB_TXAGC_C_NSS1INDEX7_NSS1INDEX4_AC, REG_BB_TXAGC_D_NSS1INDEX7_NSS1INDEX4_AC};
    u4Byte REG_BB_TXAGC_X_NSS2INDEX1_NSS1INDEX8_AC[4] = {REG_BB_TXAGC_A_NSS2INDEX1_NSS1INDEX8_AC, REG_BB_TXAGC_B_NSS2INDEX1_NSS1INDEX8_AC, REG_BB_TXAGC_C_NSS2INDEX1_NSS1INDEX8_AC, REG_BB_TXAGC_D_NSS2INDEX1_NSS1INDEX8_AC};
    u4Byte REG_BB_TXAGC_X_NSS2INDEX5_NSS2INDEX2_AC[4] = {REG_BB_TXAGC_A_NSS2INDEX5_NSS2INDEX2_AC, REG_BB_TXAGC_B_NSS2INDEX5_NSS2INDEX2_AC, REG_BB_TXAGC_C_NSS2INDEX5_NSS2INDEX2_AC, REG_BB_TXAGC_D_NSS2INDEX5_NSS2INDEX2_AC};
    u4Byte REG_BB_TXAGC_X_NSS2INDEX9_NSS2INDEX6_AC[4] = {REG_BB_TXAGC_A_NSS2INDEX9_NSS2INDEX6_AC, REG_BB_TXAGC_B_NSS2INDEX9_NSS2INDEX6_AC, REG_BB_TXAGC_C_NSS2INDEX9_NSS2INDEX6_AC, REG_BB_TXAGC_D_NSS2INDEX9_NSS2INDEX6_AC};

	lmt_pg_idx_ofdm = Adapter->pshare->txpwr_lmt_OFDM - Adapter->pshare->tgpwr_OFDM_new[0];
	lmt_pg_idx_ht1s = Adapter->pshare->txpwr_lmt_HT1S - Adapter->pshare->tgpwr_HT1S_new[0];
	lmt_pg_idx_ht2s = Adapter->pshare->txpwr_lmt_HT2S - Adapter->pshare->tgpwr_HT2S_new[0];
	lmt_pg_idx_vht1s = Adapter->pshare->txpwr_lmt_VHT1S - Adapter->pshare->tgpwr_VHT1S_new[0];
	lmt_pg_idx_vht2s = Adapter->pshare->txpwr_lmt_VHT2S - Adapter->pshare->tgpwr_VHT2S_new[0];

 	//Cal Max tx pwr idx for OFDM

	tmp_dw = HAL_RTL_R32(REG_BB_TXAGC_X_OFDM54_OFDM24_AC[bPathXEnIdx]);
	idx_odfm_54m = HAL_get_byte_from_dw(tmp_dw, 3);
	max_lmt_idx_ofdm = idx_odfm_54m + lmt_pg_idx_ofdm;
	max_lmt_idx_ofdm = POWER_MIN_INDEX(max_lmt_idx_ofdm);

	//Cal Max tx pwr idx for HT1S
	tmp_dw = HAL_RTL_R32(REG_BB_TXAGC_X_MCS7_MCS4_AC[bPathXEnIdx]);
	idx_ht_mcs7 = HAL_get_byte_from_dw(tmp_dw, 3);
	max_lmt_idx_ht1s = idx_ht_mcs7 + lmt_pg_idx_ht1s;
	max_lmt_idx_ht1s = POWER_MIN_INDEX(max_lmt_idx_ht1s);

	//Cal Max tx pwr idx for HT2S
	tmp_dw = HAL_RTL_R32(REG_BB_TXAGC_X_MCS15_MCS12_AC[bPathXEnIdx]);
	idx_ht_mcs15 = HAL_get_byte_from_dw(tmp_dw, 3);
	max_lmt_idx_ht2s = idx_ht_mcs15 + lmt_pg_idx_ht2s;
	max_lmt_idx_ht2s = POWER_MIN_INDEX(max_lmt_idx_ht2s);

	//Cal Max tx pwr idx for VHT1S
	tmp_dw = HAL_RTL_R32(REG_BB_TXAGC_X_NSS1INDEX7_NSS1INDEX4_AC[bPathXEnIdx]);
	idx_vht_1ss_mcs7 = HAL_get_byte_from_dw(tmp_dw, 3);
	max_lmt_idx_vht1s = idx_vht_1ss_mcs7 + lmt_pg_idx_vht1s;
	max_lmt_idx_vht1s = POWER_MIN_INDEX(max_lmt_idx_vht1s);

	//Cal Max tx pwr idx for VHT2S
	tmp_dw = HAL_RTL_R32(REG_BB_TXAGC_X_NSS2INDEX9_NSS2INDEX6_AC[bPathXEnIdx]);
	idx_vht_2ss_mcs7 = HAL_get_byte_from_dw(tmp_dw, 1);
	max_lmt_idx_vht2s = idx_vht_2ss_mcs7 + lmt_pg_idx_vht2s; 
	max_lmt_idx_vht2s = POWER_MIN_INDEX(max_lmt_idx_vht2s);

	HAL_assign_lmt_reg_value(Adapter, REG_BB_TXAGC_X_OFDM18_OFDM6_AC[bPathXEnIdx], max_lmt_idx_ofdm, 0);
	HAL_assign_lmt_reg_value(Adapter, REG_BB_TXAGC_X_OFDM54_OFDM24_AC[bPathXEnIdx], max_lmt_idx_ofdm, 0);
	
	HAL_assign_lmt_reg_value(Adapter, REG_BB_TXAGC_X_MCS3_MCS0_AC[bPathXEnIdx], max_lmt_idx_ht1s, 0);
	HAL_assign_lmt_reg_value(Adapter, REG_BB_TXAGC_X_MCS7_MCS4_AC[bPathXEnIdx], max_lmt_idx_ht1s, 0);
	HAL_assign_lmt_reg_value(Adapter, REG_BB_TXAGC_X_MCS11_MCS8_AC[bPathXEnIdx], max_lmt_idx_ht2s, 0);
	HAL_assign_lmt_reg_value(Adapter, REG_BB_TXAGC_X_MCS15_MCS12_AC[bPathXEnIdx], max_lmt_idx_ht2s, 0);

	HAL_assign_lmt_reg_value(Adapter, REG_BB_TXAGC_X_NSS1INDEX3_NSS1INDEX0_AC[bPathXEnIdx], max_lmt_idx_vht1s, 0);
	HAL_assign_lmt_reg_value(Adapter, REG_BB_TXAGC_X_NSS1INDEX7_NSS1INDEX4_AC[bPathXEnIdx], max_lmt_idx_vht1s, 0);
	HAL_assign_lmt_reg_value(Adapter, REG_BB_TXAGC_X_NSS2INDEX1_NSS1INDEX8_AC[bPathXEnIdx], max_lmt_idx_vht1s, max_lmt_idx_vht2s);
	HAL_assign_lmt_reg_value(Adapter, REG_BB_TXAGC_X_NSS2INDEX5_NSS2INDEX2_AC[bPathXEnIdx], max_lmt_idx_vht2s, 0);
	HAL_assign_lmt_reg_value(Adapter, REG_BB_TXAGC_X_NSS2INDEX9_NSS2INDEX6_AC[bPathXEnIdx], max_lmt_idx_vht2s, 0);

}


#if IS_RTL8814A_SERIES || IS_RTL8822B_SERIES	
void TxLMT_OFDM_Jaguar_AC_8814(
    IN  HAL_PADAPTER	Adapter
    )
{

	unsigned int  	tmp_dw = 0;
	unsigned char	bPathXEnIdx, rate, start, end;
 
	unsigned char	idx_odfm_54m = 0;
	unsigned char	idx_ht_mcs7 = 0;
	unsigned char	idx_ht_mcs15 = 0;
	unsigned char	idx_vht_1ss_mcs7 =0;
	unsigned char	idx_vht_2ss_mcs7 =0;
	unsigned char	idx_vht_3ss_mcs7 =0;    
	unsigned char	idx_vht_4ss_mcs7 =0; 

	char			lmt_pg_idx_ofdm = 0;
	char			lmt_pg_idx_ht1s, lmt_pg_idx_ht2s = 0;
	char			lmt_pg_idx_vht1s, lmt_pg_idx_vht2s = 0;
	char	        lmt_pg_idx_ht3s, lmt_pg_idx_ht4s = 0;
	char	        lmt_pg_idx_vht3s, lmt_pg_idx_vht4s = 0;

	char	max_lmt_idx_ofdm = 0;
	char	max_lmt_idx_ht1s, max_lmt_idx_ht2s = 0;
	char	max_lmt_idx_vht1s, max_lmt_idx_vht2s = 0;
	char	max_lmt_idx_ht3s, max_lmt_idx_ht4s = 0;
	char	max_lmt_idx_vht3s, max_lmt_idx_vht4s = 0;

#if defined(CONFIG_SLOT_0_8814_2T2R_SUPPORT) || defined(CONFIG_SLOT_1_8814_2T2R_SUPPORT)
	if(Adapter->pmib->dot11RFEntry.phyBandSelect ==  PHY_BAND_5G) {
		start=1;
		end=2;
	} else
#endif		
#if defined(CONFIG_SLOT_0_8194_2T2R_SUPPORT) || defined(CONFIG_SLOT_1_8194_2T2R_SUPPORT)
	if(Adapter->pmib->dot11RFEntry.phyBandSelect ==  PHY_BAND_2G) {
		start=1;
		end=2;			
	} else
#endif
	{
		start=0;
		end=3;
	}
	if(GET_CHIP_VER(Adapter) == VERSION_8822B){
		start=0;
		end=1;
	}


 	/* Cal Max tx pwr idx for each path */
	for(bPathXEnIdx = start; bPathXEnIdx <= end ; bPathXEnIdx++ ){
		lmt_pg_idx_ofdm = Adapter->pshare->txpwr_lmt_OFDM - Adapter->pshare->tgpwr_OFDM_new[bPathXEnIdx];
		lmt_pg_idx_ht1s = Adapter->pshare->txpwr_lmt_HT1S - Adapter->pshare->tgpwr_HT1S_new[bPathXEnIdx];
		lmt_pg_idx_ht2s = Adapter->pshare->txpwr_lmt_HT2S - Adapter->pshare->tgpwr_HT2S_new[bPathXEnIdx];
#ifdef CONFIG_WLAN_HAL_8814AE
		lmt_pg_idx_ht3s = Adapter->pshare->txpwr_lmt_HT3S - Adapter->pshare->tgpwr_HT3S_new[bPathXEnIdx];
	    lmt_pg_idx_ht4s = Adapter->pshare->txpwr_lmt_HT4S - Adapter->pshare->tgpwr_HT4S_new[bPathXEnIdx];
#endif
		lmt_pg_idx_vht1s = Adapter->pshare->txpwr_lmt_VHT1S - Adapter->pshare->tgpwr_VHT1S_new[bPathXEnIdx];
		lmt_pg_idx_vht2s = Adapter->pshare->txpwr_lmt_VHT2S - Adapter->pshare->tgpwr_VHT2S_new[bPathXEnIdx];
#ifdef CONFIG_WLAN_HAL_8814AE
	    lmt_pg_idx_vht3s = Adapter->pshare->txpwr_lmt_VHT3S - Adapter->pshare->tgpwr_VHT3S_new[bPathXEnIdx];
	    lmt_pg_idx_vht4s = Adapter->pshare->txpwr_lmt_VHT4S - Adapter->pshare->tgpwr_VHT4S_new[bPathXEnIdx];
#endif
#if 0	// Debug message
		printk("lmt_pg_idx_ofdm=%d =(txpwr_lmt_OFDM=%d - tgpwr_OFDM_new[%u]=%d)\n",lmt_pg_idx_ofdm, Adapter->pshare->txpwr_lmt_OFDM, bPathXEnIdx, Adapter->pshare->tgpwr_OFDM_new[bPathXEnIdx]);
		printk("lmt_pg_idx_ht1s=%d =(txpwr_lmt_HT1S=%d - tgpwr_HT1S_new[%u]=%d)\n",lmt_pg_idx_ht1s, Adapter->pshare->txpwr_lmt_HT1S, bPathXEnIdx, Adapter->pshare->tgpwr_HT1S_new[bPathXEnIdx]);
		printk("lmt_pg_idx_ht2s=%d =(txpwr_lmt_HT2S=%d - tgpwr_HT2S_new[%u]=%d)\n",lmt_pg_idx_ht2s, Adapter->pshare->txpwr_lmt_HT2S, bPathXEnIdx, Adapter->pshare->tgpwr_HT2S_new[bPathXEnIdx]);
		printk("lmt_pg_idx_ht3s=%d =(txpwr_lmt_HT3S=%d - tgpwr_HT3S_new[%u]=%d)\n",lmt_pg_idx_ht3s, Adapter->pshare->txpwr_lmt_HT3S, bPathXEnIdx, Adapter->pshare->tgpwr_HT3S_new[bPathXEnIdx]);
		printk("lmt_pg_idx_ht4s=%d =(txpwr_lmt_HT4S=%d - tgpwr_HT4S_new[%u]=%d)\n",lmt_pg_idx_ht4s, Adapter->pshare->txpwr_lmt_HT4S, bPathXEnIdx, Adapter->pshare->tgpwr_HT4S_new[bPathXEnIdx]);
		printk("lmt_pg_idx_vht1s=%d =(txpwr_lmt_VHT1S=%d - tgpwr_VHT1S_new[%u]=%d)\n",lmt_pg_idx_vht1s, Adapter->pshare->txpwr_lmt_VHT1S, bPathXEnIdx, Adapter->pshare->tgpwr_VHT1S_new[bPathXEnIdx]);
		printk("lmt_pg_idx_vht2s=%d =(txpwr_lmt_VHT2S=%d - tgpwr_VHT2S_new[%u]=%d)\n",lmt_pg_idx_vht2s, Adapter->pshare->txpwr_lmt_VHT2S, bPathXEnIdx, Adapter->pshare->tgpwr_VHT2S_new[bPathXEnIdx]);
		printk("lmt_pg_idx_vht3s=%d =(txpwr_lmt_VHT3S=%d - tgpwr_VHT3S_new[%u]=%d)\n",lmt_pg_idx_vht3s, Adapter->pshare->txpwr_lmt_VHT3S, bPathXEnIdx, Adapter->pshare->tgpwr_VHT3S_new[bPathXEnIdx]);
		printk("lmt_pg_idx_vht4s=%d =(txpwr_lmt_VHT4S=%d - tgpwr_VHT4S_new[%u]=%d)\n",lmt_pg_idx_vht4s, Adapter->pshare->txpwr_lmt_VHT4S, bPathXEnIdx, Adapter->pshare->tgpwr_VHT4S_new[bPathXEnIdx]);
#endif
		max_lmt_idx_ofdm = lmt_pg_idx_ofdm + HAL_VAR_CurrentTxAgcOFDM(bPathXEnIdx, TX_AGC_OFDM_54M-TX_AGC_OFDM_6M);
		max_lmt_idx_ofdm = POWER_MIN_INDEX(max_lmt_idx_ofdm);
		max_lmt_idx_ht1s = lmt_pg_idx_ht1s + HAL_VAR_CurrentTxAgcMCS(bPathXEnIdx, TX_AGC_HT_NSS1_MCS7-TX_AGC_HT_NSS1_MCS0);
		max_lmt_idx_ht1s = POWER_MIN_INDEX(max_lmt_idx_ht1s);
		max_lmt_idx_ht2s = lmt_pg_idx_ht2s + HAL_VAR_CurrentTxAgcMCS(bPathXEnIdx, TX_AGC_HT_NSS2_MCS15-TX_AGC_HT_NSS1_MCS0);
		max_lmt_idx_ht2s = POWER_MIN_INDEX(max_lmt_idx_ht2s);
#ifdef CONFIG_WLAN_HAL_8814AE
		max_lmt_idx_ht3s = lmt_pg_idx_ht3s + HAL_VAR_CurrentTxAgcMCS(bPathXEnIdx, TX_AGC_HT_NSS3_MCS23-TX_AGC_HT_NSS1_MCS0);
		max_lmt_idx_ht3s = POWER_MIN_INDEX(max_lmt_idx_ht3s);
#endif
		max_lmt_idx_vht1s = lmt_pg_idx_vht1s + HAL_VAR_CurrentTxAgcVHT(bPathXEnIdx, TX_AGC_VHT_NSS1_MCS7-TX_AGC_VHT_NSS1_MCS0);
		max_lmt_idx_vht1s = POWER_MIN_INDEX(max_lmt_idx_vht1s);
		max_lmt_idx_vht2s = lmt_pg_idx_vht2s + HAL_VAR_CurrentTxAgcVHT(bPathXEnIdx, TX_AGC_VHT_NSS2_MCS7-TX_AGC_VHT_NSS1_MCS0);
		max_lmt_idx_vht2s = POWER_MIN_INDEX(max_lmt_idx_vht2s);
#ifdef CONFIG_WLAN_HAL_8814AE
		max_lmt_idx_vht3s = lmt_pg_idx_vht3s + HAL_VAR_CurrentTxAgcVHT(bPathXEnIdx, TX_AGC_VHT_NSS3_MCS7-TX_AGC_VHT_NSS1_MCS0);
		max_lmt_idx_vht3s = POWER_MIN_INDEX(max_lmt_idx_vht3s);
#endif
#if 0	// Debug message
		printk("max_lmt_idx_ofdm=%d =(lmt_pg_idx_ofdm=%d + HAL_VAR_CurrentTxAgcOFDM(%d, TX_AGC_OFDM_54M-TX_AGC_OFDM_6M)=%d)\n",max_lmt_idx_ofdm, lmt_pg_idx_ofdm, bPathXEnIdx, HAL_VAR_CurrentTxAgcOFDM(bPathXEnIdx, TX_AGC_OFDM_54M-TX_AGC_OFDM_6M));
		printk("max_lmt_idx_ht1s=%d =(lmt_pg_idx_ht1s=%d + HAL_VAR_CurrentTxAgcMCS(%d, TX_AGC_HT_NSS1_MCS7-TX_AGC_HT_NSS1_MCS0)=%d)\n",max_lmt_idx_ht1s, lmt_pg_idx_ht1s, bPathXEnIdx, HAL_VAR_CurrentTxAgcMCS(bPathXEnIdx, TX_AGC_HT_NSS1_MCS7-TX_AGC_HT_NSS1_MCS0));
		printk("max_lmt_idx_ht2s=%d =(lmt_pg_idx_ht2s=%d + HAL_VAR_CurrentTxAgcMCS(%d, TX_AGC_HT_NSS2_MCS15-TX_AGC_HT_NSS1_MCS0)=%d)\n",max_lmt_idx_ht2s, lmt_pg_idx_ht2s, bPathXEnIdx, HAL_VAR_CurrentTxAgcMCS(bPathXEnIdx, TX_AGC_HT_NSS2_MCS15-TX_AGC_HT_NSS1_MCS0));
		printk("max_lmt_idx_ht3s=%d =(lmt_pg_idx_ht3s=%d + HAL_VAR_CurrentTxAgcMCS(%d, TX_AGC_HT_NSS3_MCS23-TX_AGC_HT_NSS1_MCS0)=%d)\n",max_lmt_idx_ht3s, lmt_pg_idx_ht3s, bPathXEnIdx, HAL_VAR_CurrentTxAgcMCS(bPathXEnIdx, TX_AGC_HT_NSS3_MCS23-TX_AGC_HT_NSS1_MCS0));
		printk("max_lmt_idx_vht1s=%d =(lmt_pg_idx_vht1s=%d + HAL_VAR_CurrentTxAgcVHT(%d, TX_AGC_VHT_NSS1_MCS7-TX_AGC_VHT_NSS1_MCS0)=%d)\n",max_lmt_idx_vht1s, lmt_pg_idx_vht1s, bPathXEnIdx, HAL_VAR_CurrentTxAgcVHT(bPathXEnIdx, TX_AGC_VHT_NSS1_MCS7-TX_AGC_VHT_NSS1_MCS0));
		printk("max_lmt_idx_vht2s=%d =(lmt_pg_idx_vht2s=%d + HAL_VAR_CurrentTxAgcVHT(%d, TX_AGC_VHT_NSS2_MCS7-TX_AGC_VHT_NSS1_MCS0)=%d)\n",max_lmt_idx_vht2s, lmt_pg_idx_vht2s, bPathXEnIdx, HAL_VAR_CurrentTxAgcVHT(bPathXEnIdx, TX_AGC_VHT_NSS2_MCS7-TX_AGC_VHT_NSS1_MCS0));
		printk("max_lmt_idx_vht3s=%d =(lmt_pg_idx_vht3s=%d + HAL_VAR_CurrentTxAgcVHT(%d, TX_AGC_VHT_NSS3_MCS7-TX_AGC_VHT_NSS1_MCS0)=%d)\n",max_lmt_idx_vht3s, lmt_pg_idx_vht3s, bPathXEnIdx, HAL_VAR_CurrentTxAgcVHT(bPathXEnIdx, TX_AGC_VHT_NSS3_MCS7-TX_AGC_VHT_NSS1_MCS0));
#endif
		/* set OFDM power index */
		for(rate = 0; rate <= TX_AGC_OFDM_54M - TX_AGC_OFDM_6M; rate++)
			PHYSetTxPower88XX(Adapter, rate+TX_AGC_OFDM_6M, rate+TX_AGC_OFDM_6M, bPathXEnIdx, POWER_RANGE_CHECK(POWER_MIN_CHECK(HAL_VAR_CurrentTxAgcOFDM(bPathXEnIdx, rate), max_lmt_idx_ofdm)));
		/* set MCS 1/2/3 SS power index */
		for(rate = 0; rate <= TX_AGC_HT_NSS1_MCS7-TX_AGC_HT_NSS1_MCS0; rate++)
			PHYSetTxPower88XX(Adapter, rate+TX_AGC_HT_NSS1_MCS0, rate+TX_AGC_HT_NSS1_MCS0, bPathXEnIdx, POWER_RANGE_CHECK(POWER_MIN_CHECK(HAL_VAR_CurrentTxAgcMCS(bPathXEnIdx, rate), max_lmt_idx_ht1s)));
		for(rate = TX_AGC_HT_NSS2_MCS8-TX_AGC_HT_NSS1_MCS0; rate <= TX_AGC_HT_NSS2_MCS15-TX_AGC_HT_NSS1_MCS0; rate++)
			PHYSetTxPower88XX(Adapter, rate+TX_AGC_HT_NSS1_MCS0, rate+TX_AGC_HT_NSS1_MCS0, bPathXEnIdx, POWER_RANGE_CHECK(POWER_MIN_CHECK(HAL_VAR_CurrentTxAgcMCS(bPathXEnIdx, rate), max_lmt_idx_ht2s)));
#ifdef CONFIG_WLAN_HAL_8814AE
		for(rate = TX_AGC_HT_NSS3_MCS16-TX_AGC_HT_NSS1_MCS0; rate <= TX_AGC_HT_NSS3_MCS23-TX_AGC_HT_NSS1_MCS0; rate++)
			PHYSetTxPower88XX(Adapter, rate+TX_AGC_HT_NSS1_MCS0, rate+TX_AGC_HT_NSS1_MCS0, bPathXEnIdx, POWER_RANGE_CHECK(POWER_MIN_CHECK(HAL_VAR_CurrentTxAgcMCS(bPathXEnIdx, rate), max_lmt_idx_ht3s)));
#endif
		/* set VHT MCS 1/2/3 SS power index */		
		for(rate = 0; rate <= TX_AGC_VHT_NSS1_MCS9-TX_AGC_VHT_NSS1_MCS0; rate++)
			PHYSetTxPower88XX(Adapter, rate+TX_AGC_VHT_NSS1_MCS0, rate+TX_AGC_VHT_NSS1_MCS0, bPathXEnIdx, POWER_RANGE_CHECK(POWER_MIN_CHECK(HAL_VAR_CurrentTxAgcVHT(bPathXEnIdx, rate), max_lmt_idx_vht1s)));
		for(rate = TX_AGC_VHT_NSS2_MCS0-TX_AGC_VHT_NSS1_MCS0; rate <= TX_AGC_VHT_NSS2_MCS9-TX_AGC_VHT_NSS1_MCS0; rate++)
			PHYSetTxPower88XX(Adapter, rate+TX_AGC_VHT_NSS1_MCS0, rate+TX_AGC_VHT_NSS1_MCS0, bPathXEnIdx, POWER_RANGE_CHECK(POWER_MIN_CHECK(HAL_VAR_CurrentTxAgcVHT(bPathXEnIdx, rate), max_lmt_idx_vht2s)));
#ifdef CONFIG_WLAN_HAL_8814AE
		for(rate = TX_AGC_VHT_NSS3_MCS0-TX_AGC_VHT_NSS1_MCS0; rate <= TX_AGC_VHT_NSS3_MCS9-TX_AGC_VHT_NSS1_MCS0; rate++)
			PHYSetTxPower88XX(Adapter, rate+TX_AGC_VHT_NSS1_MCS0, rate+TX_AGC_VHT_NSS1_MCS0, bPathXEnIdx, POWER_RANGE_CHECK(POWER_MIN_CHECK(HAL_VAR_CurrentTxAgcVHT(bPathXEnIdx, rate), max_lmt_idx_vht3s)));
#endif
	}
}


void TxLMT_CCK_Jaguar_AC_8814(IN  HAL_PADAPTER	Adapter)
{

	unsigned int	tmp_dw = 0;
	unsigned char	idx_cck_11m = 0;
	unsigned char	bPathXEnIdx, rate;
	char			max_lmt_idx_cck, lmt_pg_idx_cck;
	int i, start, end;
	
#if defined(CONFIG_SLOT_0_8814_2T2R_SUPPORT) || defined(CONFIG_SLOT_1_8814_2T2R_SUPPORT)
	if(Adapter->pmib->dot11RFEntry.phyBandSelect ==  PHY_BAND_5G) {
		start=1;
		end=2;
	} else
#endif		
#if defined(CONFIG_SLOT_0_8194_2T2R_SUPPORT) || defined(CONFIG_SLOT_1_8194_2T2R_SUPPORT)
	if(Adapter->pmib->dot11RFEntry.phyBandSelect ==  PHY_BAND_2G) {
		start=1;
		end=2;			
	} else
#endif
	{
		start=0;
		end=3;
	}


	//Cal Max tx pwr idx for CCK
	for(bPathXEnIdx = start; bPathXEnIdx <= end ; bPathXEnIdx++ ){
		lmt_pg_idx_cck = Adapter->pshare->txpwr_lmt_CCK - Adapter->pshare->tgpwr_CCK_new[bPathXEnIdx];
		//printk("lmt_pg_idx_cck=%d =(txpwr_lmt_CCK=%d - tgpwr_CCK_new[%u]=%d)\n",lmt_pg_idx_cck, Adapter->pshare->txpwr_lmt_CCK, bPathXEnIdx, Adapter->pshare->tgpwr_CCK_new[bPathXEnIdx]);
		
		max_lmt_idx_cck = lmt_pg_idx_cck + HAL_VAR_CurrentTxAgcCCK(bPathXEnIdx, TX_AGC_CCK_11M);
		max_lmt_idx_cck = POWER_MIN_INDEX(max_lmt_idx_cck);
		//printk("max_lmt_idx_cck=%d =(lmt_pg_idx_cck=%d + HAL_VAR_CurrentTxAgcCCK(%d, TX_AGC_CCK_11M)=%d)\n",max_lmt_idx_cck, lmt_pg_idx_cck, bPathXEnIdx, HAL_VAR_CurrentTxAgcCCK(bPathXEnIdx, TX_AGC_CCK_11M));
		/* set CCK power index */
		for(rate = 0; rate <= TX_AGC_CCK_11M; rate++)
			PHYSetTxPower88XX(Adapter, rate, rate, bPathXEnIdx, POWER_RANGE_CHECK(POWER_MIN_CHECK(HAL_VAR_CurrentTxAgcCCK(bPathXEnIdx, rate), max_lmt_idx_cck)));
	}
}
#endif


void TxLMT_OFDM_Jaguar_AC(IN  HAL_PADAPTER	Adapter)
{
    if (GetChipIDMIMO88XX(Adapter) == MIMO_1T1R) {
        TxLMT_OFDM_Jaguar_PathX_AC(Adapter, 0);
	} else if (GetChipIDMIMO88XX(Adapter) == MIMO_2T2R) {
        TxLMT_OFDM_Jaguar_PathX_AC(Adapter, 0);
        TxLMT_OFDM_Jaguar_PathX_AC(Adapter, 1);
    } else if (GetChipIDMIMO88XX(Adapter) == MIMO_3T3R) {
        TxLMT_OFDM_Jaguar_PathX_AC(Adapter, 0);
        TxLMT_OFDM_Jaguar_PathX_AC(Adapter, 1);
        TxLMT_OFDM_Jaguar_PathX_AC(Adapter, 2);
    } else if (GetChipIDMIMO88XX(Adapter) == MIMO_4T4R) {
        TxLMT_OFDM_Jaguar_PathX_AC(Adapter, 0);
        TxLMT_OFDM_Jaguar_PathX_AC(Adapter, 1);
        TxLMT_OFDM_Jaguar_PathX_AC(Adapter, 2);
        TxLMT_OFDM_Jaguar_PathX_AC(Adapter, 3);
	}
}

void TxLMT_CCK_Jaguar_AC(IN  HAL_PADAPTER	Adapter, IN pu1Byte bPathXEn)
{

	unsigned int  tmp_dw = 0;

	unsigned char	idx_cck_11m = 0;

	char	lmt_pg_idx_cck;

	char	max_lmt_idx_cck;
    int i;

    unsigned int REG_BB_TXAGC_X_CCK11_CCK1_AC[4] = {REG_BB_TXAGC_A_CCK11_CCK1_AC, REG_BB_TXAGC_B_CCK11_CCK1_AC, REG_BB_TXAGC_C_CCK11_CCK1_AC, REG_BB_TXAGC_D_CCK11_CCK1_AC};

	lmt_pg_idx_cck = Adapter->pshare->txpwr_lmt_CCK - Adapter->pshare->tgpwr_CCK_new[0];

	//Cal Max tx pwr idx for CCK
    for(i = 0; i < 4; i++) {
        if (bPathXEn[i]) {
            tmp_dw = HAL_RTL_R32(REG_BB_TXAGC_X_CCK11_CCK1_AC[i]);
            idx_cck_11m = HAL_get_byte_from_dw(tmp_dw, 3);
            max_lmt_idx_cck = idx_cck_11m + lmt_pg_idx_cck;
			max_lmt_idx_cck = POWER_MIN_INDEX(max_lmt_idx_cck);
            HAL_assign_lmt_reg_value(Adapter, REG_BB_TXAGC_X_CCK11_CCK1_AC[i], max_lmt_idx_cck, 0);
        }
	}


}


#endif
#if IS_RTL88XX_N
void TxLMT_OFDM_Jaguar_N(IN  HAL_PADAPTER	Adapter)
{

	unsigned int  tmp_dw = 0;
 
	unsigned char	idx_ht_mcs7 = 0;
	unsigned char	idx_ht_mcs15 = 0;
	unsigned char   idx_rate54 = 0;

	char	lmt_pg_idx_ht1s, lmt_pg_idx_ht2s = 0;
	char	lmt_pg_idx_rate;

	char	max_lmt_idx_ht1s, max_lmt_idx_ht2s = 0;
	char    max_lmt_rate54 = 0;
 	
	lmt_pg_idx_ht1s = Adapter->pshare->txpwr_lmt_HT1S - Adapter->pshare->tgpwr_HT1S_new[0];
	lmt_pg_idx_ht2s = Adapter->pshare->txpwr_lmt_HT2S - Adapter->pshare->tgpwr_HT2S_new[0];
	lmt_pg_idx_rate = Adapter->pshare->txpwr_lmt_OFDM - Adapter->pshare->tgpwr_OFDM_new[0];

	tmp_dw = HAL_RTL_R32(REG_BB_TXAGC_A_RATE54_24_N);
	idx_rate54 = HAL_get_byte_from_dw(tmp_dw, 3);
	max_lmt_rate54 = idx_rate54 + lmt_pg_idx_rate;
	max_lmt_rate54 = POWER_MIN_INDEX(max_lmt_rate54);

	//Cal Max tx pwr idx for HT1S
	tmp_dw = HAL_RTL_R32(REG_BB_TXAGC_A_MCS07_MCS04_N);
	idx_ht_mcs7 = HAL_get_byte_from_dw(tmp_dw, 3);
	max_lmt_idx_ht1s = idx_ht_mcs7 + lmt_pg_idx_ht1s;
	max_lmt_idx_ht1s = POWER_MIN_INDEX(max_lmt_idx_ht1s);

	//Cal Max tx pwr idx for HT2S
	tmp_dw = HAL_RTL_R32(REG_BB_TXAGC_A_MCS15_MCS12_N);
	idx_ht_mcs15 = HAL_get_byte_from_dw(tmp_dw, 3);
	max_lmt_idx_ht2s = idx_ht_mcs15 + lmt_pg_idx_ht2s;
	max_lmt_idx_ht2s = POWER_MIN_INDEX(max_lmt_idx_ht2s);

	RT_TRACE(COMP_RF, DBG_TRACE, ("TxLMT_OFDM_Jaguar_N, idx_rate54=%x, idx_ht_mcs7=%x, idx_ht_mcs15=%x\n", idx_rate54, idx_ht_mcs7, idx_ht_mcs15));
	RT_TRACE(COMP_RF, DBG_TRACE, ("TxLMT_OFDM_Jaguar_N, OFDM Limit=%x, HT1S Limit=%x, HT2S Limit=%x\n", max_lmt_rate54, max_lmt_idx_ht1s, max_lmt_idx_ht2s));
	HAL_assign_lmt_reg_value(Adapter, REG_BB_TXAGC_A_RATE18_06_N, max_lmt_rate54, 0);
	HAL_assign_lmt_reg_value(Adapter, REG_BB_TXAGC_A_RATE54_24_N, max_lmt_rate54, 0);

	HAL_assign_lmt_reg_value(Adapter, REG_BB_TXAGC_A_MCS03_MCS00_N, max_lmt_idx_ht1s, 0);
	HAL_assign_lmt_reg_value(Adapter, REG_BB_TXAGC_A_MCS07_MCS04_N, max_lmt_idx_ht1s, 0);

	HAL_assign_lmt_reg_value(Adapter, REG_BB_TXAGC_A_MCS11_MCS08_N, max_lmt_idx_ht2s, 0);
	HAL_assign_lmt_reg_value(Adapter, REG_BB_TXAGC_A_MCS15_MCS12_N, max_lmt_idx_ht2s, 0);


	if (GetChipIDMIMO88XX(Adapter) == MIMO_2T2R) {

		lmt_pg_idx_ht1s = Adapter->pshare->txpwr_lmt_HT1S - Adapter->pshare->tgpwr_HT1S_new[1];
		lmt_pg_idx_ht2s = Adapter->pshare->txpwr_lmt_HT2S - Adapter->pshare->tgpwr_HT2S_new[1];
		lmt_pg_idx_rate = Adapter->pshare->txpwr_lmt_OFDM - Adapter->pshare->tgpwr_OFDM_new[1];

		tmp_dw = HAL_RTL_R32(REG_BB_TXAGC_B_RATE54_24_N);
		idx_rate54 = HAL_get_byte_from_dw(tmp_dw, 3);
		max_lmt_rate54 = idx_rate54 + lmt_pg_idx_rate;
		max_lmt_rate54 = POWER_MIN_INDEX(max_lmt_rate54);
		//Cal Max tx pwr idx for HT1S
		tmp_dw = HAL_RTL_R32(REG_BB_TXAGC_B_MCS07_MCS04_N);
		idx_ht_mcs7 = HAL_get_byte_from_dw(tmp_dw, 3);
		max_lmt_idx_ht1s = idx_ht_mcs7 + lmt_pg_idx_ht1s;
		max_lmt_idx_ht1s = POWER_MIN_INDEX(max_lmt_idx_ht1s);

		//Cal Max tx pwr idx for HT2S
		tmp_dw = HAL_RTL_R32(REG_BB_TXAGC_B_MCS15_MCS12_N);
		idx_ht_mcs15 = HAL_get_byte_from_dw(tmp_dw, 3);
		max_lmt_idx_ht2s = idx_ht_mcs15 + lmt_pg_idx_ht2s;
		max_lmt_idx_ht2s = POWER_MIN_INDEX(max_lmt_idx_ht2s);

		HAL_assign_lmt_reg_value(Adapter, REG_BB_TXAGC_B_RATE18_06_N, max_lmt_rate54, 0);
		HAL_assign_lmt_reg_value(Adapter, REG_BB_TXAGC_B_RATE54_24_N, max_lmt_rate54, 0);

		HAL_assign_lmt_reg_value(Adapter, REG_BB_TXAGC_B_MCS03_MCS00_N, max_lmt_idx_ht1s, 0);
		HAL_assign_lmt_reg_value(Adapter, REG_BB_TXAGC_B_MCS07_MCS04_N, max_lmt_idx_ht1s, 0);

		HAL_assign_lmt_reg_value(Adapter, REG_BB_TXAGC_B_MCS11_MCS08_N, max_lmt_idx_ht2s, 0);
		HAL_assign_lmt_reg_value(Adapter, REG_BB_TXAGC_B_MCS15_MCS12_N, max_lmt_idx_ht2s, 0);
	}
}

void TxLMT_CCK_Jaguar_N(IN  HAL_PADAPTER	Adapter)
{

	unsigned int  tmp_dw = 0;

	unsigned char	idx_cck_11m_A = 0, idx_cck_11m_B = 0;

	char	lmt_pg_idx_cck_A,lmt_pg_idx_cck_B;

	char	max_lmt_idx_cck_A;
	char	max_lmt_idx_cck_B = 0;

	lmt_pg_idx_cck_A = Adapter->pshare->txpwr_lmt_CCK - Adapter->pshare->tgpwr_CCK_new[0];

	if (GetChipIDMIMO88XX(Adapter) == MIMO_2T2R)
		lmt_pg_idx_cck_B = Adapter->pshare->txpwr_lmt_CCK - Adapter->pshare->tgpwr_CCK_new[1];

	//printk("%d %d %d %d %d %d\n", lmt_pg_idx_cck, lmt_pg_idx_ofdm,
	//lmt_pg_idx_ht1s, lmt_pg_idx_ht2s, lmt_pg_idx_vht1s, lmt_pg_idx_vht2s);


	//Cal Max tx pwr idx for CCK
	tmp_dw = HAL_RTL_R32(REG_BB_TXAGC_A_CCK11_2_B_CCK11_N);
	idx_cck_11m_A = HAL_get_byte_from_dw(tmp_dw, 3);
	max_lmt_idx_cck_A = idx_cck_11m_A + lmt_pg_idx_cck_A;
	max_lmt_idx_cck_A = POWER_MIN_INDEX(max_lmt_idx_cck_A);

	tmp_dw = HAL_RTL_R32(REG_BB_TXAGC_A_CCK11_2_B_CCK11_N);
	idx_cck_11m_B = HAL_get_byte_from_dw(tmp_dw, 0);
	max_lmt_idx_cck_B = idx_cck_11m_B + lmt_pg_idx_cck_B;
	max_lmt_idx_cck_B = POWER_MIN_INDEX(max_lmt_idx_cck_B);

	RT_TRACE(COMP_RF, DBG_TRACE, ("TxLMT_CCK_Jaguar_N, idx_cck_11m_A=%x, idx_cck_11m_B=%x\n", idx_cck_11m_A, idx_cck_11m_B));
	RT_TRACE(COMP_RF, DBG_TRACE, ("TxLMT_CCK_Jaguar_N, CCK Limit A=%x, CCK Limit B=%x\n", max_lmt_idx_cck_A, max_lmt_idx_cck_B));

	HAL_assign_lmt_reg_value(Adapter, REG_BB_TXAGC_A_CCK1_MCS32_N, max_lmt_idx_cck_A, 0);
	HAL_assign_lmt_reg_value(Adapter, REG_BB_TXAGC_A_CCK11_2_B_CCK11_N, max_lmt_idx_cck_A, max_lmt_idx_cck_B);
	HAL_assign_lmt_reg_value(Adapter, REG_BB_TXAGC_B_CCK5_1_MCS32_N, max_lmt_idx_cck_B, 0);
}
#endif

#endif //TXPWR_LMT


#if IS_RTL88XX_AC

void
phy_RFSerialWrite_88XX_AC
(
	IN  HAL_PADAPTER                Adapter,
	IN  RFRF88XX_RADIO_PATH_E       eRFPath,
	IN  u4Byte                      RegAddr,
	IN  u4Byte                      Data
)
{
	u4Byte dwTmp = 0;
	u4Byte value = ((RegAddr << 20) | Data); // addr BIT[27:20], data BIT[19:0]
	u4Byte temp_CCA = 0;


    temp_CCA = HAL_RTL_R32(REG_BB_CCAONSEC_AC);
	PHY_SetBBReg_88XX(Adapter, REG_BB_CCAONSEC_AC, BIT_MASK_BB_CCA_AC, 0xc); // CCA off

	if (eRFPath == RF88XX_PATH_A) {
		dwTmp = HAL_RTL_R32(REG_BB_A_LSSIWRITE_AC);
		dwTmp &= BIT_MASK_BB_LSSIWRITE_AC;
		dwTmp |= value;

		HAL_RTL_W32(REG_BB_A_LSSIWRITE_AC, dwTmp);
	} else if (eRFPath == RF88XX_PATH_B) {
		dwTmp = HAL_RTL_R32(REG_BB_B_LSSIWRITE_AC);
		dwTmp &= BIT_MASK_BB_LSSIWRITE_AC;
		dwTmp |= value;

		HAL_RTL_W32(REG_BB_B_LSSIWRITE_AC, dwTmp);
	}
#if IS_RTL8814A_SERIES
	else if(eRFPath == RF88XX_PATH_C)
	{
		dwTmp = HAL_RTL_R32(REG_BB_C_LSSIWRITE_AC);
		dwTmp &= BIT_MASK_BB_LSSIWRITE_AC;
		dwTmp |= value;
	
		HAL_RTL_W32(REG_BB_C_LSSIWRITE_AC, dwTmp);
	}
	else if(eRFPath == RF88XX_PATH_D)
	{
		dwTmp = HAL_RTL_R32(REG_BB_D_LSSIWRITE_AC);
		dwTmp &= BIT_MASK_BB_LSSIWRITE_AC;
		dwTmp |= value;
	
		HAL_RTL_W32(REG_BB_D_LSSIWRITE_AC, dwTmp);
	}
#endif

	PHY_SetBBReg_88XX(Adapter, REG_BB_CCAONSEC_AC, BIT_MASK_BB_CCA_AC, temp_CCA); // CCA off
}

u4Byte
PHY_QueryRFReg_88XX_AC(
	IN  HAL_PADAPTER                Adapter,
	IN  u4Byte                      eRFPath,
	IN  u4Byte                      RegAddr,
	IN  u4Byte                      BitMask
)
{
	// The phy Query function have bug in 8812 & 8881, must follow the rules as below
	// 1. No matter path A or B, we have to assign the addr on 0x8b0[7:0]
	// 2. Before Query RF reg, we must turn off CCA
	// 3. delay 10us after set addr
	// 4. If SI mode, read value from 0xd08[19:0] on PathA, 0xd48[19:0] on PathB
	// 5. If PI mode, read value from 0xd08[19:0] on PathA, 0xd48[19:0] on PathB

	HAL_PADAPTER    priv     = Adapter;
	unsigned long   flags;
	u4Byte          Original_Value, Readback_Value, BitShift;
    u4Byte          temp_CCA;

#if CFG_HAL_DISABLE_BB_RF
	return 0;
#endif //CFG_HAL_DISABLE_BB_RF

	HAL_SAVE_INT_AND_CLI(flags);

    temp_CCA = HAL_RTL_R32(REG_BB_CCAONSEC_AC);
	PHY_SetBBReg_88XX(Adapter, REG_BB_CCAONSEC_AC, BIT_MASK_BB_CCA_AC, 0xc); // CCA off

	HAL_RTL_W8(REG_BB_HSSIREAD_AC, RegAddr);

	delay_us(30);

	// normally, we use PI mode in normal chip
	if (eRFPath == RF88XX_PATH_A) {
		Original_Value = HAL_RTL_R32(REG_BB_A_PIREAD_AC);
	} else if (eRFPath == RF88XX_PATH_B) {
		Original_Value = HAL_RTL_R32(REG_BB_B_PIREAD_AC);
	}
#if IS_RTL8814A_SERIES
	else if(eRFPath == RF88XX_PATH_C) {
		Original_Value = HAL_RTL_R32(REG_BB_C_PIREAD_AC);
	}
	else if(eRFPath == RF88XX_PATH_D) {
		Original_Value = HAL_RTL_R32(REG_BB_D_PIREAD_AC);
	}
#endif
    else {
		RT_TRACE_F(COMP_RF, DBG_SERIOUS, ("RF PATH no existence\n"));
		HAL_RESTORE_INT(flags);
		return 0xFFFFFFFF;
	}

	Original_Value &= 0xfffff;
	
	BitShift =	phy_CalculateBitShift_88XX(BitMask);
	Readback_Value = (Original_Value & BitMask) >> BitShift;

	PHY_SetBBReg_88XX(Adapter, REG_BB_CCAONSEC_AC, BIT_MASK_BB_CCA_AC, temp_CCA); //	CCA on

	HAL_RESTORE_INT(flags);

	return (Readback_Value);
}

void
PHY_SetRFReg_88XX_AC
(
	IN  HAL_PADAPTER                Adapter,
	IN  u4Byte                      eRFPath,
	IN  u4Byte                      RegAddr,
	IN  u4Byte                      BitMask,
	IN  u4Byte                      Data
)
{
	u4Byte          Original_Value, BitShift, New_Value;
	HAL_PADAPTER    priv     = Adapter;
	unsigned long   flags;

#if CFG_HAL_DISABLE_BB_RF
	return;
#endif //CFG_HAL_DISABLE_BB_RF

#if (RTL8822B_SUPPORT == 1)  
	if(IS_HARDWARE_TYPE_8822BE(Adapter)) {
		HAL_SAVE_INT_AND_CLI(flags);
		config_phydm_write_rf_reg_8822b((&(Adapter->pshare->_dmODM)), eRFPath, RegAddr, BitMask, Data);
		HAL_RESTORE_INT(flags);
		delay_us(4);
		return;
	}
#endif


	HAL_SAVE_INT_AND_CLI(flags);
	if (BitMask != BIT_MASK_SET_MASK20BITS_COMMON) {
		Original_Value = GET_HAL_INTERFACE(priv)->PHYQueryRFRegHandler(Adapter, eRFPath, RegAddr, BIT_MASK_SET_MASK20BITS_COMMON); 
		//Original_Value = PHY_QueryRFReg_88XX_AC(Adapter, eRFPath, RegAddr, BIT_MASK_SET_MASK20BITS_COMMON);
		BitShift = phy_CalculateBitShift_88XX(BitMask);
		New_Value = ((Original_Value & (~BitMask)) | (Data << BitShift));
		phy_RFSerialWrite_88XX_AC(Adapter, eRFPath, RegAddr, New_Value);
	} else {
		phy_RFSerialWrite_88XX_AC(Adapter, eRFPath, RegAddr, Data);
	}

	HAL_RESTORE_INT(flags);
	delay_us(4);    // At least 500ns delay to avoid RF write fail.
}


#if 1//def TX_PG_Jaguar

void get_tx_pwr_88XX(unsigned int tmp_dw, unsigned char *tmp_b1, unsigned char *tmp_b2,
					 unsigned char *tmp_b3, unsigned char *tmp_b4)
{
	*tmp_b1 = (tmp_dw & (0xff));
	*tmp_b2 = ((tmp_dw & (0xff00)) >> 8);
	*tmp_b3 = ((tmp_dw & (0xff0000)) >> 16);
	*tmp_b4 = ((tmp_dw & (0xff000000)) >> 24);
}

void TxPG_CCK_Jaguar
(
	IN  HAL_PADAPTER    Adapter,
	IN  pu1Byte         bPathXEn
)
{
	unsigned char b1, b2, b3, b4;
	unsigned char* tmp_b1 = &b1;
	unsigned char* tmp_b2 = &b2;
	unsigned char* tmp_b3 = &b3;
	unsigned char* tmp_b4 = &b4;
	unsigned int  tmp_dw = 0;
	unsigned int  writeVal = 0;

    if (bPathXEn[0]) {
    	tmp_dw = HAL_RTL_R32(REG_BB_TXAGC_A_CCK11_CCK1_AC);
    	get_tx_pwr_88XX(tmp_dw, tmp_b1, tmp_b2, tmp_b3, tmp_b4);
    	*tmp_b1 = HAL_POWER_RANGE_CHECK(*tmp_b1 + Adapter->pshare->phw->CCKTxAgc_A[3]);
    	*tmp_b2 = HAL_POWER_RANGE_CHECK(*tmp_b2 + Adapter->pshare->phw->CCKTxAgc_A[2]);
    	*tmp_b3 = HAL_POWER_RANGE_CHECK(*tmp_b3 + Adapter->pshare->phw->CCKTxAgc_A[1]);
    	*tmp_b4 = HAL_POWER_RANGE_CHECK(*tmp_b4 + Adapter->pshare->phw->CCKTxAgc_A[0]);
    	writeVal = (*tmp_b4 << 24) | (*tmp_b3 << 16) | (*tmp_b2 << 8) | *tmp_b1;
    	HAL_RTL_W32(REG_BB_TXAGC_A_CCK11_CCK1_AC, writeVal);
    } 
    // TODO: 8814AE CCKTxAgc_B[4,5,6,7] ???
    if (bPathXEn[1]) {
    	tmp_dw = HAL_RTL_R32(REG_BB_TXAGC_B_CCK11_CCK1_AC);
    	get_tx_pwr_88XX(tmp_dw, tmp_b1, tmp_b2, tmp_b3, tmp_b4);
    	*tmp_b1 = HAL_POWER_RANGE_CHECK(*tmp_b1 + Adapter->pshare->phw->CCKTxAgc_B[3]);
    	*tmp_b2 = HAL_POWER_RANGE_CHECK(*tmp_b2 + Adapter->pshare->phw->CCKTxAgc_B[2]);
    	*tmp_b3 = HAL_POWER_RANGE_CHECK(*tmp_b3 + Adapter->pshare->phw->CCKTxAgc_B[1]);
    	*tmp_b4 = HAL_POWER_RANGE_CHECK(*tmp_b4 + Adapter->pshare->phw->CCKTxAgc_B[0]);
    	writeVal = (*tmp_b4 << 24) | (*tmp_b3 << 16) | (*tmp_b2 << 8) | *tmp_b1;
    	HAL_RTL_W32(REG_BB_TXAGC_B_CCK11_CCK1_AC, writeVal);
    }
#if IS_RTL8814A_SERIES
    if (bPathXEn[2]) {
        tmp_dw = HAL_RTL_R32(REG_BB_TXAGC_C_CCK11_CCK1_AC);
        get_tx_pwr_88XX(tmp_dw, tmp_b1, tmp_b2, tmp_b3, tmp_b4);
        *tmp_b1 = HAL_POWER_RANGE_CHECK(*tmp_b1 + Adapter->pshare->phw->CCKTxAgc_C[3]);
        *tmp_b2 = HAL_POWER_RANGE_CHECK(*tmp_b2 + Adapter->pshare->phw->CCKTxAgc_C[2]);
        *tmp_b3 = HAL_POWER_RANGE_CHECK(*tmp_b3 + Adapter->pshare->phw->CCKTxAgc_C[1]);
        *tmp_b4 = HAL_POWER_RANGE_CHECK(*tmp_b4 + Adapter->pshare->phw->CCKTxAgc_C[0]);
        writeVal = (*tmp_b4 << 24) | (*tmp_b3 << 16) | (*tmp_b2 << 8) | *tmp_b1;
        HAL_RTL_W32(REG_BB_TXAGC_C_CCK11_CCK1_AC, writeVal);
    }
    if (bPathXEn[3]) {
        tmp_dw = HAL_RTL_R32(REG_BB_TXAGC_D_CCK11_CCK1_AC);
        get_tx_pwr_88XX(tmp_dw, tmp_b1, tmp_b2, tmp_b3, tmp_b4);
        *tmp_b1 = HAL_POWER_RANGE_CHECK(*tmp_b1 + Adapter->pshare->phw->CCKTxAgc_D[3]);
        *tmp_b2 = HAL_POWER_RANGE_CHECK(*tmp_b2 + Adapter->pshare->phw->CCKTxAgc_D[2]);
        *tmp_b3 = HAL_POWER_RANGE_CHECK(*tmp_b3 + Adapter->pshare->phw->CCKTxAgc_D[1]);
        *tmp_b4 = HAL_POWER_RANGE_CHECK(*tmp_b4 + Adapter->pshare->phw->CCKTxAgc_D[0]);
        writeVal = (*tmp_b4 << 24) | (*tmp_b3 << 16) | (*tmp_b2 << 8) | *tmp_b1;
        HAL_RTL_W32(REG_BB_TXAGC_D_CCK11_CCK1_AC, writeVal);
    }
#endif
}


#if (IS_RTL8814A_SERIES || IS_RTL8822B_SERIES)
void TxPG_CCK_8814
(
	IN  HAL_PADAPTER                Adapter
)
{
	int path, rate, start, end;
	char *pTxAgcOffset;
	
#if defined(CONFIG_SLOT_0_8814_2T2R_SUPPORT) || defined(CONFIG_SLOT_1_8814_2T2R_SUPPORT)
	if(Adapter->pmib->dot11RFEntry.phyBandSelect ==  PHY_BAND_5G) {
		start=1;
		end=2;
	} else
#endif		
#if defined(CONFIG_SLOT_0_8194_2T2R_SUPPORT) || defined(CONFIG_SLOT_1_8194_2T2R_SUPPORT)
	if(Adapter->pmib->dot11RFEntry.phyBandSelect ==  PHY_BAND_2G) {
		start=1;
		end=2;			
	} else
#endif
	{
		start=0;
		end=3;
	}


	for(path=start;path<=end;path++){
		switch(path){
		case 0:	pTxAgcOffset = Adapter->pshare->phw->CCKTxAgc_A;	break;
		case 1:	pTxAgcOffset = Adapter->pshare->phw->CCKTxAgc_B;	break;
		case 2:	pTxAgcOffset = Adapter->pshare->phw->CCKTxAgc_C;	break;
		case 3:	pTxAgcOffset = Adapter->pshare->phw->CCKTxAgc_D;	break;
		}
		for(rate=TX_AGC_CCK_1M;rate<=TX_AGC_CCK_11M;rate++){
			//printk("CurrentTxAgcCCK[%d][%d]=%x + %x = ",path,rate,Adapter->pshare->phw->CurrentTxAgcCCK[path][rate],(unsigned char)*(pTxAgcOffset+rate));
			Adapter->pshare->phw->CurrentTxAgcCCK[path][rate] += (unsigned char)*(pTxAgcOffset+rate);
			Adapter->pshare->phw->CurrentTxAgcCCK[path][rate] = HAL_POWER_RANGE_CHECK(Adapter->pshare->phw->CurrentTxAgcCCK[path][rate]);
            PHYSetTxPower88XX(Adapter, rate, rate, path, Adapter->pshare->phw->CurrentTxAgcCCK[path][rate]);
			//printk("CurrentTxAgcCCK[%d][%d]=%x\n",path,rate,Adapter->pshare->phw->CurrentTxAgcCCK[path][rate]);
		}
	}
}


void TxPG_OFDM_8814
(
	IN  HAL_PADAPTER                Adapter
)
{
	int path, rate, start, end;
	char *pTxAgcOffset;

#if defined(CONFIG_SLOT_0_8814_2T2R_SUPPORT) || defined(CONFIG_SLOT_1_8814_2T2R_SUPPORT)
	if(Adapter->pmib->dot11RFEntry.phyBandSelect ==  PHY_BAND_5G) {
		start=1;
		end=2;
	} else
#endif		
#if defined(CONFIG_SLOT_0_8194_2T2R_SUPPORT) || defined(CONFIG_SLOT_1_8194_2T2R_SUPPORT)
	if(Adapter->pmib->dot11RFEntry.phyBandSelect ==  PHY_BAND_2G) {
		start=1;
		end=2;			
	} else
#endif
	{
		start=0;
		end=3;
	}

	/* OFDM */
	for(path=start;path<=end;path++){
		switch(path){
		case 0:	pTxAgcOffset = Adapter->pshare->phw->OFDMTxAgcOffset_A;	break;
		case 1:	pTxAgcOffset = Adapter->pshare->phw->OFDMTxAgcOffset_B;	break;
		case 2:	pTxAgcOffset = Adapter->pshare->phw->OFDMTxAgcOffset_C;	break;
		case 3:	pTxAgcOffset = Adapter->pshare->phw->OFDMTxAgcOffset_D;	break;
		}
		for(rate=TX_AGC_OFDM_6M;rate<=TX_AGC_OFDM_54M;rate++){
			//printk("CurrentTxAgcOFDM[%d][%d]=%x + %x = ",path,rate-TX_AGC_OFDM_6M,Adapter->pshare->phw->CurrentTxAgcOFDM[path][rate-TX_AGC_OFDM_6M],(unsigned char)*(pTxAgcOffset+rate-TX_AGC_OFDM_6M));
			Adapter->pshare->phw->CurrentTxAgcOFDM[path][rate-TX_AGC_OFDM_6M] += (unsigned char)*(pTxAgcOffset+rate-TX_AGC_OFDM_6M);
			Adapter->pshare->phw->CurrentTxAgcOFDM[path][rate-TX_AGC_OFDM_6M] = HAL_POWER_RANGE_CHECK(Adapter->pshare->phw->CurrentTxAgcOFDM[path][rate-TX_AGC_OFDM_6M]);
            PHYSetTxPower88XX(Adapter, rate, rate, path, Adapter->pshare->phw->CurrentTxAgcOFDM[path][rate-TX_AGC_OFDM_6M]);
			//printk("CurrentTxAgcOFDM[%d][%d]=%x\n",path,rate-TX_AGC_OFDM_6M,Adapter->pshare->phw->CurrentTxAgcOFDM[path][rate-TX_AGC_OFDM_6M]);
		}
	}
	/* HT MCS */
	for(path=start;path<=end;path++){
		switch(path){
		case 0:	pTxAgcOffset = Adapter->pshare->phw->MCSTxAgcOffset_A;	break;
		case 1:	pTxAgcOffset = Adapter->pshare->phw->MCSTxAgcOffset_B;	break;
		case 2:	pTxAgcOffset = Adapter->pshare->phw->MCSTxAgcOffset_C;	break;
		case 3:	pTxAgcOffset = Adapter->pshare->phw->MCSTxAgcOffset_D;	break;
		}
		for(rate=TX_AGC_HT_NSS1_MCS0;rate<=TX_AGC_HT_NSS3_MCS23;rate++){
			//printk("CurrentTxAgcMCS[%d][%d]=%x + %x = ",path,rate-TX_AGC_HT_NSS1_MCS0,Adapter->pshare->phw->CurrentTxAgcMCS[path][rate-TX_AGC_HT_NSS1_MCS0],(unsigned char)*(pTxAgcOffset+rate-TX_AGC_HT_NSS1_MCS0));
			Adapter->pshare->phw->CurrentTxAgcMCS[path][rate-TX_AGC_HT_NSS1_MCS0] += (unsigned char)*(pTxAgcOffset+rate-TX_AGC_HT_NSS1_MCS0);
			Adapter->pshare->phw->CurrentTxAgcMCS[path][rate-TX_AGC_HT_NSS1_MCS0] = HAL_POWER_RANGE_CHECK(Adapter->pshare->phw->CurrentTxAgcMCS[path][rate-TX_AGC_HT_NSS1_MCS0]);
			PHYSetTxPower88XX(Adapter, rate, rate, path, Adapter->pshare->phw->CurrentTxAgcMCS[path][rate-TX_AGC_HT_NSS1_MCS0]);			
			//printk("CurrentTxAgcMCS[%d][%d]=%x\n",path,rate-TX_AGC_HT_NSS1_MCS0,Adapter->pshare->phw->CurrentTxAgcMCS[path][rate-TX_AGC_HT_NSS1_MCS0]);
		}
	}
	/* VHT MCS */
	for(path=start;path<=end;path++){
		switch(path){
		case 0:	pTxAgcOffset = Adapter->pshare->phw->VHTTxAgcOffset_A;	break;
		case 1:	pTxAgcOffset = Adapter->pshare->phw->VHTTxAgcOffset_B;	break;
		case 2:	pTxAgcOffset = Adapter->pshare->phw->VHTTxAgcOffset_C;	break;
		case 3:	pTxAgcOffset = Adapter->pshare->phw->VHTTxAgcOffset_D;	break;
		}
		for(rate=TX_AGC_VHT_NSS1_MCS0;rate<=TX_AGC_VHT_NSS3_MCS9;rate++){
			//printk("CurrentTxAgcVHT[%d][%d]=%x + %x = ",path,rate-TX_AGC_VHT_NSS1_MCS0,Adapter->pshare->phw->CurrentTxAgcVHT[path][rate-TX_AGC_VHT_NSS1_MCS0],(unsigned char)*(pTxAgcOffset+rate-TX_AGC_VHT_NSS1_MCS0));
			if( rate == TX_AGC_VHT_NSS1_MCS8 || rate == TX_AGC_VHT_NSS1_MCS9 \
				|| rate == TX_AGC_VHT_NSS2_MCS8|| rate == TX_AGC_VHT_NSS2_MCS9 \
				|| rate == TX_AGC_VHT_NSS3_MCS8 || rate == TX_AGC_VHT_NSS3_MCS9 )
				Adapter->pshare->phw->CurrentTxAgcVHT[path][rate-TX_AGC_VHT_NSS1_MCS0] -= (unsigned char)*(pTxAgcOffset+rate-TX_AGC_VHT_NSS1_MCS0);
			else
				Adapter->pshare->phw->CurrentTxAgcVHT[path][rate-TX_AGC_VHT_NSS1_MCS0] += (unsigned char)*(pTxAgcOffset+rate-TX_AGC_VHT_NSS1_MCS0);
			Adapter->pshare->phw->CurrentTxAgcVHT[path][rate-TX_AGC_VHT_NSS1_MCS0] = HAL_POWER_RANGE_CHECK(Adapter->pshare->phw->CurrentTxAgcVHT[path][rate-TX_AGC_VHT_NSS1_MCS0]);
            PHYSetTxPower88XX(Adapter, rate, rate, path, Adapter->pshare->phw->CurrentTxAgcVHT[path][rate-TX_AGC_VHT_NSS1_MCS0]);			
			//printk("CurrentTxAgcVHT[%d][%d]=%x\n",path,rate-TX_AGC_VHT_NSS1_MCS0,Adapter->pshare->phw->CurrentTxAgcVHT[path][rate-TX_AGC_VHT_NSS1_MCS0]);
		}
	}		
}
#endif
#endif
	
void TxPG_OFDM_Jaguar_A
(
	IN  HAL_PADAPTER                Adapter
)
{
	unsigned char b1, b2, b3, b4, b5;
	unsigned char* tmp_b1 = &b1;
	unsigned char* tmp_b2 = &b2;
	unsigned char* tmp_b3 = &b3;
	unsigned char* tmp_b4 = &b4;
	unsigned char* tmp_b5 = &b5;
	unsigned int  tmp_dw = 0;
	unsigned int  tmp_dw1 = 0;
	unsigned int  writeVal = 0;
	//printk("TxPG_OFDM_8812_A!!\n");
	tmp_dw = HAL_RTL_R32(REG_BB_TXAGC_A_OFDM18_OFDM6_AC);
	get_tx_pwr_88XX(tmp_dw, tmp_b1, tmp_b2, tmp_b3, tmp_b4);
	*tmp_b1 = HAL_POWER_RANGE_CHECK(*tmp_b1 + Adapter->pshare->phw->OFDMTxAgcOffset_A[3]);
	*tmp_b2 = HAL_POWER_RANGE_CHECK(*tmp_b2 + Adapter->pshare->phw->OFDMTxAgcOffset_A[2]);
	*tmp_b3 = HAL_POWER_RANGE_CHECK(*tmp_b3 + Adapter->pshare->phw->OFDMTxAgcOffset_A[1]);
	*tmp_b4 = HAL_POWER_RANGE_CHECK(*tmp_b4 + Adapter->pshare->phw->OFDMTxAgcOffset_A[0]);
	writeVal = (*tmp_b4 << 24) | (*tmp_b3 << 16) | (*tmp_b2 << 8) | *tmp_b1;
	HAL_RTL_W32(REG_BB_TXAGC_A_OFDM18_OFDM6_AC, writeVal);
	tmp_dw = HAL_RTL_R32(REG_BB_TXAGC_A_OFDM54_OFDM24_AC);
	get_tx_pwr_88XX(tmp_dw, tmp_b1, tmp_b2, tmp_b3, tmp_b4);
	*tmp_b1 = HAL_POWER_RANGE_CHECK(*tmp_b1 + Adapter->pshare->phw->OFDMTxAgcOffset_A[7]);
	*tmp_b2 = HAL_POWER_RANGE_CHECK(*tmp_b2 + Adapter->pshare->phw->OFDMTxAgcOffset_A[6]);
	*tmp_b3 = HAL_POWER_RANGE_CHECK(*tmp_b3 + Adapter->pshare->phw->OFDMTxAgcOffset_A[5]);
	*tmp_b4 = HAL_POWER_RANGE_CHECK(*tmp_b4 + Adapter->pshare->phw->OFDMTxAgcOffset_A[4]);
	writeVal = (*tmp_b4 << 24) | (*tmp_b3 << 16) | (*tmp_b2 << 8) | *tmp_b1;
	HAL_RTL_W32(REG_BB_TXAGC_A_OFDM54_OFDM24_AC, writeVal);
	tmp_dw = HAL_RTL_R32(REG_BB_TXAGC_A_MCS3_MCS0_AC);
	get_tx_pwr_88XX(tmp_dw, tmp_b1, tmp_b2, tmp_b3, tmp_b4);
	*tmp_b1 = HAL_POWER_RANGE_CHECK(*tmp_b1 + Adapter->pshare->phw->MCSTxAgcOffset_A[3]);
	*tmp_b2 = HAL_POWER_RANGE_CHECK(*tmp_b2 + Adapter->pshare->phw->MCSTxAgcOffset_A[2]);
	*tmp_b3 = HAL_POWER_RANGE_CHECK(*tmp_b3 + Adapter->pshare->phw->MCSTxAgcOffset_A[1]);
	*tmp_b4 = HAL_POWER_RANGE_CHECK(*tmp_b4 + Adapter->pshare->phw->MCSTxAgcOffset_A[0]);
	writeVal = (*tmp_b4 << 24) | (*tmp_b3 << 16) | (*tmp_b2 << 8) | *tmp_b1;
	HAL_RTL_W32(REG_BB_TXAGC_A_MCS3_MCS0_AC, writeVal);
	tmp_dw = HAL_RTL_R32(REG_BB_TXAGC_A_MCS7_MCS4_AC);
	get_tx_pwr_88XX(tmp_dw, tmp_b1, tmp_b2, tmp_b3, tmp_b4);
	*tmp_b1 = HAL_POWER_RANGE_CHECK(*tmp_b1 + Adapter->pshare->phw->MCSTxAgcOffset_A[7]);
	*tmp_b2 = HAL_POWER_RANGE_CHECK(*tmp_b2 + Adapter->pshare->phw->MCSTxAgcOffset_A[6]);
	*tmp_b3 = HAL_POWER_RANGE_CHECK(*tmp_b3 + Adapter->pshare->phw->MCSTxAgcOffset_A[5]);
	*tmp_b4 = HAL_POWER_RANGE_CHECK(*tmp_b4 + Adapter->pshare->phw->MCSTxAgcOffset_A[4]);
	writeVal = (*tmp_b4 << 24) | (*tmp_b3 << 16) | (*tmp_b2 << 8) | *tmp_b1;
	HAL_RTL_W32(REG_BB_TXAGC_A_MCS7_MCS4_AC, writeVal);
	if (GetChipIDMIMO88XX(Adapter) == MIMO_2T2R) {
		tmp_dw = HAL_RTL_R32(REG_BB_TXAGC_A_MCS11_MCS8_AC);
		get_tx_pwr_88XX(tmp_dw, tmp_b1, tmp_b2, tmp_b3, tmp_b4);
		*tmp_b1 = HAL_POWER_RANGE_CHECK(*tmp_b1 + Adapter->pshare->phw->MCSTxAgcOffset_A[11]);
		*tmp_b2 = HAL_POWER_RANGE_CHECK(*tmp_b2 + Adapter->pshare->phw->MCSTxAgcOffset_A[10]);
		*tmp_b3 = HAL_POWER_RANGE_CHECK(*tmp_b3 + Adapter->pshare->phw->MCSTxAgcOffset_A[9]);
		*tmp_b4 = HAL_POWER_RANGE_CHECK(*tmp_b4 + Adapter->pshare->phw->MCSTxAgcOffset_A[8]);
		writeVal = (*tmp_b4 << 24) | (*tmp_b3 << 16) | (*tmp_b2 << 8) | *tmp_b1;
		HAL_RTL_W32(REG_BB_TXAGC_A_MCS11_MCS8_AC, writeVal);
		tmp_dw = HAL_RTL_R32(REG_BB_TXAGC_A_MCS15_MCS12_AC);
		get_tx_pwr_88XX(tmp_dw, tmp_b1, tmp_b2, tmp_b3, tmp_b4);
		*tmp_b1 = HAL_POWER_RANGE_CHECK(*tmp_b1 + Adapter->pshare->phw->MCSTxAgcOffset_A[15]);
		*tmp_b2 = HAL_POWER_RANGE_CHECK(*tmp_b2 + Adapter->pshare->phw->MCSTxAgcOffset_A[14]);
		*tmp_b3 = HAL_POWER_RANGE_CHECK(*tmp_b3 + Adapter->pshare->phw->MCSTxAgcOffset_A[13]);
		*tmp_b4 = HAL_POWER_RANGE_CHECK(*tmp_b4 + Adapter->pshare->phw->MCSTxAgcOffset_A[12]);
		writeVal = (*tmp_b4 << 24) | (*tmp_b3 << 16) | (*tmp_b2 << 8) | *tmp_b1;
		HAL_RTL_W32(REG_BB_TXAGC_A_MCS15_MCS12_AC, writeVal);
	}
	if (HAL_VAR_PHYBANDSELECT == PHY_BAND_5G) {
		tmp_dw = HAL_RTL_R32(REG_BB_TXAGC_A_NSS1INDEX3_NSS1INDEX0_AC);
		get_tx_pwr_88XX(tmp_dw, tmp_b1, tmp_b2, tmp_b3, tmp_b4);
		*tmp_b1 = HAL_POWER_RANGE_CHECK(*tmp_b1 + Adapter->pshare->phw->VHTTxAgcOffset_A[3]);
		*tmp_b2 = HAL_POWER_RANGE_CHECK(*tmp_b2 + Adapter->pshare->phw->VHTTxAgcOffset_A[2]);
		*tmp_b3 = HAL_POWER_RANGE_CHECK(*tmp_b3 + Adapter->pshare->phw->VHTTxAgcOffset_A[1]);
		*tmp_b4 = HAL_POWER_RANGE_CHECK(*tmp_b4 + Adapter->pshare->phw->VHTTxAgcOffset_A[0]);
		writeVal = (*tmp_b4 << 24) | (*tmp_b3 << 16) | (*tmp_b2 << 8) | *tmp_b1;
		HAL_RTL_W32(REG_BB_TXAGC_A_NSS1INDEX3_NSS1INDEX0_AC, writeVal);
		tmp_dw = HAL_RTL_R32(REG_BB_TXAGC_A_NSS1INDEX7_NSS1INDEX4_AC);
		get_tx_pwr_88XX(tmp_dw, tmp_b1, tmp_b2, tmp_b3, tmp_b4);
		*tmp_b1 = HAL_POWER_RANGE_CHECK(*tmp_b1 + Adapter->pshare->phw->VHTTxAgcOffset_A[7]);
		*tmp_b2 = HAL_POWER_RANGE_CHECK(*tmp_b2 + Adapter->pshare->phw->VHTTxAgcOffset_A[6]);
		*tmp_b3 = HAL_POWER_RANGE_CHECK(*tmp_b3 + Adapter->pshare->phw->VHTTxAgcOffset_A[5]);
		*tmp_b4 = HAL_POWER_RANGE_CHECK(*tmp_b4 + Adapter->pshare->phw->VHTTxAgcOffset_A[4]);
		writeVal = (*tmp_b4 << 24) | (*tmp_b3 << 16) | (*tmp_b2 << 8) | *tmp_b1;
		HAL_RTL_W32(REG_BB_TXAGC_A_NSS1INDEX7_NSS1INDEX4_AC, writeVal);

		tmp_dw = HAL_RTL_R32(REG_BB_TXAGC_A_NSS2INDEX1_NSS1INDEX8_AC);
		tmp_dw1 = HAL_RTL_R32(REG_BB_TXAGC_A_NSS1INDEX7_NSS1INDEX4_AC);
		get_tx_pwr_88XX(tmp_dw1, tmp_b1, tmp_b2, tmp_b3, tmp_b5);
		get_tx_pwr_88XX(tmp_dw, tmp_b1, tmp_b2, tmp_b3, tmp_b4);
		*tmp_b1 = HAL_POWER_RANGE_CHECK(*tmp_b5 - Adapter->pshare->phw->VHTTxAgcOffset_A[11]);
		*tmp_b2 = HAL_POWER_RANGE_CHECK(*tmp_b5 - Adapter->pshare->phw->VHTTxAgcOffset_A[10]);
		*tmp_b3 = HAL_POWER_RANGE_CHECK(*tmp_b3 + Adapter->pshare->phw->VHTTxAgcOffset_A[9]);
		*tmp_b4 = HAL_POWER_RANGE_CHECK(*tmp_b4 + Adapter->pshare->phw->VHTTxAgcOffset_A[8]);
		writeVal = (*tmp_b4 << 24) | (*tmp_b3 << 16) | (*tmp_b2 << 8) | *tmp_b1;
		HAL_RTL_W32(REG_BB_TXAGC_A_NSS2INDEX1_NSS1INDEX8_AC, writeVal);

		tmp_dw = HAL_RTL_R32(REG_BB_TXAGC_A_NSS2INDEX5_NSS2INDEX2_AC);
		get_tx_pwr_88XX(tmp_dw, tmp_b1, tmp_b2, tmp_b3, tmp_b4);
		*tmp_b1 = HAL_POWER_RANGE_CHECK(*tmp_b1 + Adapter->pshare->phw->VHTTxAgcOffset_A[15]);
		*tmp_b2 = HAL_POWER_RANGE_CHECK(*tmp_b2 + Adapter->pshare->phw->VHTTxAgcOffset_A[14]);
		*tmp_b3 = HAL_POWER_RANGE_CHECK(*tmp_b3 + Adapter->pshare->phw->VHTTxAgcOffset_A[13]);
		*tmp_b4 = HAL_POWER_RANGE_CHECK(*tmp_b4 + Adapter->pshare->phw->VHTTxAgcOffset_A[12]);
		writeVal = (*tmp_b4 << 24) | (*tmp_b3 << 16) | (*tmp_b2 << 8) | *tmp_b1;
		HAL_RTL_W32(REG_BB_TXAGC_A_NSS2INDEX5_NSS2INDEX2_AC, writeVal);

		tmp_dw = HAL_RTL_R32(REG_BB_TXAGC_A_NSS2INDEX9_NSS2INDEX6_AC);
		get_tx_pwr_88XX(tmp_dw, tmp_b1, tmp_b2, tmp_b3, tmp_b4);
		*tmp_b1 = HAL_POWER_RANGE_CHECK(*tmp_b1 + Adapter->pshare->phw->VHTTxAgcOffset_A[19]);
		*tmp_b2 = HAL_POWER_RANGE_CHECK(*tmp_b2 + Adapter->pshare->phw->VHTTxAgcOffset_A[18]);
		*tmp_b3 = HAL_POWER_RANGE_CHECK(*tmp_b3 - Adapter->pshare->phw->VHTTxAgcOffset_A[17]);
		*tmp_b4 = HAL_POWER_RANGE_CHECK(*tmp_b4 - Adapter->pshare->phw->VHTTxAgcOffset_A[16]);
		writeVal = (*tmp_b4 << 24) | (*tmp_b3 << 16) | (*tmp_b2 << 8) | *tmp_b1;
		HAL_RTL_W32(REG_BB_TXAGC_A_NSS2INDEX9_NSS2INDEX6_AC, writeVal);
	}
}


void TxPG_OFDM_Jaguar_B
(
	IN  HAL_PADAPTER                Adapter
)
{
	unsigned char b1, b2, b3, b4, b5;
	unsigned char* tmp_b1 = &b1;
	unsigned char* tmp_b2 = &b2;
	unsigned char* tmp_b3 = &b3;
	unsigned char* tmp_b4 = &b4;
	unsigned char* tmp_b5 = &b5;
	unsigned int  tmp_dw = 0;
	unsigned int  tmp_dw1 = 0;
	unsigned int  writeVal = 0;

	//printk("TxPG_OFDM_8812_B!!\n");
	tmp_dw = HAL_RTL_R32(REG_BB_TXAGC_B_OFDM18_OFDM6_AC);
	get_tx_pwr_88XX(tmp_dw, tmp_b1, tmp_b2, tmp_b3, tmp_b4);
	*tmp_b1 = HAL_POWER_RANGE_CHECK(*tmp_b1 + Adapter->pshare->phw->OFDMTxAgcOffset_B[3]);
	*tmp_b2 = HAL_POWER_RANGE_CHECK(*tmp_b2 + Adapter->pshare->phw->OFDMTxAgcOffset_B[2]);
	*tmp_b3 = HAL_POWER_RANGE_CHECK(*tmp_b3 + Adapter->pshare->phw->OFDMTxAgcOffset_B[1]);
	*tmp_b4 = HAL_POWER_RANGE_CHECK(*tmp_b4 + Adapter->pshare->phw->OFDMTxAgcOffset_B[0]);
	writeVal = (*tmp_b4 << 24) | (*tmp_b3 << 16) | (*tmp_b2 << 8) | *tmp_b1;
	HAL_RTL_W32(REG_BB_TXAGC_B_OFDM18_OFDM6_AC, writeVal);
	tmp_dw = HAL_RTL_R32(REG_BB_TXAGC_B_OFDM54_OFDM24_AC);
	get_tx_pwr_88XX(tmp_dw, tmp_b1, tmp_b2, tmp_b3, tmp_b4);
	*tmp_b1 = HAL_POWER_RANGE_CHECK(*tmp_b1 + Adapter->pshare->phw->OFDMTxAgcOffset_B[7]);
	*tmp_b2 = HAL_POWER_RANGE_CHECK(*tmp_b2 + Adapter->pshare->phw->OFDMTxAgcOffset_B[6]);
	*tmp_b3 = HAL_POWER_RANGE_CHECK(*tmp_b3 + Adapter->pshare->phw->OFDMTxAgcOffset_B[5]);
	*tmp_b4 = HAL_POWER_RANGE_CHECK(*tmp_b4 + Adapter->pshare->phw->OFDMTxAgcOffset_B[4]);
	writeVal = (*tmp_b4 << 24) | (*tmp_b3 << 16) | (*tmp_b2 << 8) | *tmp_b1;
	HAL_RTL_W32(REG_BB_TXAGC_B_OFDM54_OFDM24_AC, writeVal);
	tmp_dw = HAL_RTL_R32(REG_BB_TXAGC_B_MCS3_MCS0_AC);
	get_tx_pwr_88XX(tmp_dw, tmp_b1, tmp_b2, tmp_b3, tmp_b4);
	*tmp_b1 = HAL_POWER_RANGE_CHECK(*tmp_b1 + Adapter->pshare->phw->MCSTxAgcOffset_B[3]);
	*tmp_b2 = HAL_POWER_RANGE_CHECK(*tmp_b2 + Adapter->pshare->phw->MCSTxAgcOffset_B[2]);
	*tmp_b3 = HAL_POWER_RANGE_CHECK(*tmp_b3 + Adapter->pshare->phw->MCSTxAgcOffset_B[1]);
	*tmp_b4 = HAL_POWER_RANGE_CHECK(*tmp_b4 + Adapter->pshare->phw->MCSTxAgcOffset_B[0]);
	writeVal = (*tmp_b4 << 24) | (*tmp_b3 << 16) | (*tmp_b2 << 8) | *tmp_b1;
	HAL_RTL_W32(REG_BB_TXAGC_B_MCS3_MCS0_AC, writeVal);
	tmp_dw = HAL_RTL_R32(REG_BB_TXAGC_B_MCS7_MCS4_AC);
	get_tx_pwr_88XX(tmp_dw, tmp_b1, tmp_b2, tmp_b3, tmp_b4);
	*tmp_b1 = HAL_POWER_RANGE_CHECK(*tmp_b1 + Adapter->pshare->phw->MCSTxAgcOffset_B[7]);
	*tmp_b2 = HAL_POWER_RANGE_CHECK(*tmp_b2 + Adapter->pshare->phw->MCSTxAgcOffset_B[6]);
	*tmp_b3 = HAL_POWER_RANGE_CHECK(*tmp_b3 + Adapter->pshare->phw->MCSTxAgcOffset_B[5]);
	*tmp_b4 = HAL_POWER_RANGE_CHECK(*tmp_b4 + Adapter->pshare->phw->MCSTxAgcOffset_B[4]);
	writeVal = (*tmp_b4 << 24) | (*tmp_b3 << 16) | (*tmp_b2 << 8) | *tmp_b1;
	HAL_RTL_W32(REG_BB_TXAGC_B_MCS7_MCS4_AC, writeVal);
	tmp_dw = HAL_RTL_R32(REG_BB_TXAGC_B_MCS11_MCS8_AC);
	get_tx_pwr_88XX(tmp_dw, tmp_b1, tmp_b2, tmp_b3, tmp_b4);
	*tmp_b1 = HAL_POWER_RANGE_CHECK(*tmp_b1 + Adapter->pshare->phw->MCSTxAgcOffset_B[11]);
	*tmp_b2 = HAL_POWER_RANGE_CHECK(*tmp_b2 + Adapter->pshare->phw->MCSTxAgcOffset_B[10]);
	*tmp_b3 = HAL_POWER_RANGE_CHECK(*tmp_b3 + Adapter->pshare->phw->MCSTxAgcOffset_B[9]);
	*tmp_b4 = HAL_POWER_RANGE_CHECK(*tmp_b4 + Adapter->pshare->phw->MCSTxAgcOffset_B[8]);
	writeVal = (*tmp_b4 << 24) | (*tmp_b3 << 16) | (*tmp_b2 << 8) | *tmp_b1;
	HAL_RTL_W32(REG_BB_TXAGC_B_MCS11_MCS8_AC, writeVal);
	tmp_dw = HAL_RTL_R32(REG_BB_TXAGC_B_MCS15_MCS12_AC);
	get_tx_pwr_88XX(tmp_dw, tmp_b1, tmp_b2, tmp_b3, tmp_b4);
	*tmp_b1 = HAL_POWER_RANGE_CHECK(*tmp_b1 + Adapter->pshare->phw->MCSTxAgcOffset_B[15]);
	*tmp_b2 = HAL_POWER_RANGE_CHECK(*tmp_b2 + Adapter->pshare->phw->MCSTxAgcOffset_B[14]);
	*tmp_b3 = HAL_POWER_RANGE_CHECK(*tmp_b3 + Adapter->pshare->phw->MCSTxAgcOffset_B[13]);
	*tmp_b4 = HAL_POWER_RANGE_CHECK(*tmp_b4 + Adapter->pshare->phw->MCSTxAgcOffset_B[12]);
	writeVal = (*tmp_b4 << 24) | (*tmp_b3 << 16) | (*tmp_b2 << 8) | *tmp_b1;
	HAL_RTL_W32(REG_BB_TXAGC_B_MCS15_MCS12_AC, writeVal);
	tmp_dw = HAL_RTL_R32(REG_BB_TXAGC_B_NSS1INDEX3_NSS1INDEX0_AC);
	get_tx_pwr_88XX(tmp_dw, tmp_b1, tmp_b2, tmp_b3, tmp_b4);
	*tmp_b1 = HAL_POWER_RANGE_CHECK(*tmp_b1 + Adapter->pshare->phw->VHTTxAgcOffset_B[3]);
	*tmp_b2 = HAL_POWER_RANGE_CHECK(*tmp_b2 + Adapter->pshare->phw->VHTTxAgcOffset_B[2]);
	*tmp_b3 = HAL_POWER_RANGE_CHECK(*tmp_b3 + Adapter->pshare->phw->VHTTxAgcOffset_B[1]);
	*tmp_b4 = HAL_POWER_RANGE_CHECK(*tmp_b4 + Adapter->pshare->phw->VHTTxAgcOffset_B[0]);
	writeVal = (*tmp_b4 << 24) | (*tmp_b3 << 16) | (*tmp_b2 << 8) | *tmp_b1;
	HAL_RTL_W32(REG_BB_TXAGC_B_NSS1INDEX3_NSS1INDEX0_AC, writeVal);
	tmp_dw = HAL_RTL_R32(REG_BB_TXAGC_B_NSS1INDEX7_NSS1INDEX4_AC);
	get_tx_pwr_88XX(tmp_dw, tmp_b1, tmp_b2, tmp_b3, tmp_b4);
	*tmp_b1 = HAL_POWER_RANGE_CHECK(*tmp_b1 + Adapter->pshare->phw->VHTTxAgcOffset_B[7]);
	*tmp_b2 = HAL_POWER_RANGE_CHECK(*tmp_b2 + Adapter->pshare->phw->VHTTxAgcOffset_B[6]);
	*tmp_b3 = HAL_POWER_RANGE_CHECK(*tmp_b3 + Adapter->pshare->phw->VHTTxAgcOffset_B[5]);
	*tmp_b4 = HAL_POWER_RANGE_CHECK(*tmp_b4 + Adapter->pshare->phw->VHTTxAgcOffset_B[4]);
	writeVal = (*tmp_b4 << 24) | (*tmp_b3 << 16) | (*tmp_b2 << 8) | *tmp_b1;
	HAL_RTL_W32(REG_BB_TXAGC_B_NSS1INDEX7_NSS1INDEX4_AC, writeVal);
	tmp_dw = HAL_RTL_R32(REG_BB_TXAGC_B_NSS2INDEX1_NSS1INDEX8_AC);
	tmp_dw1= HAL_RTL_R32(REG_BB_TXAGC_B_NSS1INDEX7_NSS1INDEX4_AC);
	get_tx_pwr_88XX(tmp_dw1, tmp_b1, tmp_b2, tmp_b3, tmp_b5);
	get_tx_pwr_88XX(tmp_dw, tmp_b1, tmp_b2, tmp_b3, tmp_b4);
	*tmp_b1 = HAL_POWER_RANGE_CHECK(*tmp_b5 - Adapter->pshare->phw->VHTTxAgcOffset_B[11]);
	*tmp_b2 = HAL_POWER_RANGE_CHECK(*tmp_b5 - Adapter->pshare->phw->VHTTxAgcOffset_B[10]);
	*tmp_b3 = HAL_POWER_RANGE_CHECK(*tmp_b3 + Adapter->pshare->phw->VHTTxAgcOffset_B[9]);
	*tmp_b4 = HAL_POWER_RANGE_CHECK(*tmp_b4 + Adapter->pshare->phw->VHTTxAgcOffset_B[8]);
	writeVal = (*tmp_b4 << 24) | (*tmp_b3 << 16) | (*tmp_b2 << 8) | *tmp_b1;
	HAL_RTL_W32(REG_BB_TXAGC_B_NSS2INDEX1_NSS1INDEX8_AC, writeVal);
	tmp_dw = HAL_RTL_R32(REG_BB_TXAGC_B_NSS2INDEX5_NSS2INDEX2_AC);
	get_tx_pwr_88XX(tmp_dw, tmp_b1, tmp_b2, tmp_b3, tmp_b4);
	*tmp_b1 = HAL_POWER_RANGE_CHECK(*tmp_b1 + Adapter->pshare->phw->VHTTxAgcOffset_B[15]);
	*tmp_b2 = HAL_POWER_RANGE_CHECK(*tmp_b2 + Adapter->pshare->phw->VHTTxAgcOffset_B[14]);
	*tmp_b3 = HAL_POWER_RANGE_CHECK(*tmp_b3 + Adapter->pshare->phw->VHTTxAgcOffset_B[13]);
	*tmp_b4 = HAL_POWER_RANGE_CHECK(*tmp_b4 + Adapter->pshare->phw->VHTTxAgcOffset_B[12]);
	writeVal = (*tmp_b4 << 24) | (*tmp_b3 << 16) | (*tmp_b2 << 8) | *tmp_b1;
	HAL_RTL_W32(REG_BB_TXAGC_B_NSS2INDEX5_NSS2INDEX2_AC, writeVal);
	tmp_dw = HAL_RTL_R32(REG_BB_TXAGC_B_NSS2INDEX9_NSS2INDEX6_AC);
	get_tx_pwr_88XX(tmp_dw, tmp_b1, tmp_b2, tmp_b3, tmp_b4);
	*tmp_b1 = HAL_POWER_RANGE_CHECK(*tmp_b1 + Adapter->pshare->phw->VHTTxAgcOffset_B[19]);
	*tmp_b2 = HAL_POWER_RANGE_CHECK(*tmp_b2 + Adapter->pshare->phw->VHTTxAgcOffset_B[18]);
	*tmp_b3 = HAL_POWER_RANGE_CHECK(*tmp_b3 - Adapter->pshare->phw->VHTTxAgcOffset_B[17]);
	*tmp_b4 = HAL_POWER_RANGE_CHECK(*tmp_b4 - Adapter->pshare->phw->VHTTxAgcOffset_B[16]);
	writeVal = (*tmp_b4 << 24) | (*tmp_b3 << 16) | (*tmp_b2 << 8) | *tmp_b1;
	HAL_RTL_W32(REG_BB_TXAGC_B_NSS2INDEX9_NSS2INDEX6_AC, writeVal);
}

#if IS_RTL8814A_SERIES
void TxPG_OFDM_Jaguar_C
(    
	IN  HAL_PADAPTER                Adapter
)	
{
	unsigned char b1, b2, b3, b4, b5;
	unsigned char* tmp_b1 = &b1;
	unsigned char* tmp_b2 = &b2;
	unsigned char* tmp_b3 = &b3;
	unsigned char* tmp_b4 = &b4;
	unsigned char* tmp_b5 = &b5;
	unsigned int  tmp_dw = 0;
	unsigned int  tmp_dw1 =0;
	unsigned int  writeVal = 0;

	//printk("TxPG_OFDM_8812_B!!\n");
	tmp_dw = HAL_RTL_R32(REG_BB_TXAGC_C_OFDM18_OFDM6_AC);
	get_tx_pwr_88XX(tmp_dw, tmp_b1, tmp_b2, tmp_b3, tmp_b4);
	*tmp_b1 = HAL_POWER_RANGE_CHECK(*tmp_b1 + Adapter->pshare->phw->OFDMTxAgcOffset_C[3]);
	*tmp_b2 = HAL_POWER_RANGE_CHECK(*tmp_b2 + Adapter->pshare->phw->OFDMTxAgcOffset_C[2]);
	*tmp_b3 = HAL_POWER_RANGE_CHECK(*tmp_b3 + Adapter->pshare->phw->OFDMTxAgcOffset_C[1]);
	*tmp_b4 = HAL_POWER_RANGE_CHECK(*tmp_b4 + Adapter->pshare->phw->OFDMTxAgcOffset_C[0]);
	writeVal = (*tmp_b4 << 24) | (*tmp_b3 << 16) | (*tmp_b2 << 8) | *tmp_b1;
	HAL_RTL_W32(REG_BB_TXAGC_C_OFDM18_OFDM6_AC, writeVal);
	tmp_dw = HAL_RTL_R32(REG_BB_TXAGC_C_OFDM54_OFDM24_AC);
	get_tx_pwr_88XX(tmp_dw, tmp_b1, tmp_b2, tmp_b3, tmp_b4);
	*tmp_b1 = HAL_POWER_RANGE_CHECK(*tmp_b1 + Adapter->pshare->phw->OFDMTxAgcOffset_C[7]);
	*tmp_b2 = HAL_POWER_RANGE_CHECK(*tmp_b2 + Adapter->pshare->phw->OFDMTxAgcOffset_C[6]);
	*tmp_b3 = HAL_POWER_RANGE_CHECK(*tmp_b3 + Adapter->pshare->phw->OFDMTxAgcOffset_C[5]);
	*tmp_b4 = HAL_POWER_RANGE_CHECK(*tmp_b4 + Adapter->pshare->phw->OFDMTxAgcOffset_C[4]);
	writeVal = (*tmp_b4 << 24) | (*tmp_b3 << 16) | (*tmp_b2 << 8) | *tmp_b1;
	HAL_RTL_W32(REG_BB_TXAGC_C_OFDM54_OFDM24_AC, writeVal);
	tmp_dw = HAL_RTL_R32(REG_BB_TXAGC_C_MCS3_MCS0_AC);
	get_tx_pwr_88XX(tmp_dw, tmp_b1, tmp_b2, tmp_b3, tmp_b4);
	*tmp_b1 = HAL_POWER_RANGE_CHECK(*tmp_b1 + Adapter->pshare->phw->MCSTxAgcOffset_C[3]);
	*tmp_b2 = HAL_POWER_RANGE_CHECK(*tmp_b2 + Adapter->pshare->phw->MCSTxAgcOffset_C[2]);
	*tmp_b3 = HAL_POWER_RANGE_CHECK(*tmp_b3 + Adapter->pshare->phw->MCSTxAgcOffset_C[1]);
	*tmp_b4 = HAL_POWER_RANGE_CHECK(*tmp_b4 + Adapter->pshare->phw->MCSTxAgcOffset_C[0]);
	writeVal = (*tmp_b4 << 24) | (*tmp_b3 << 16) | (*tmp_b2 << 8) | *tmp_b1;
	HAL_RTL_W32(REG_BB_TXAGC_C_MCS3_MCS0_AC, writeVal);
	tmp_dw = HAL_RTL_R32(REG_BB_TXAGC_C_MCS7_MCS4_AC);
	get_tx_pwr_88XX(tmp_dw, tmp_b1, tmp_b2, tmp_b3, tmp_b4);
	*tmp_b1 = HAL_POWER_RANGE_CHECK(*tmp_b1 + Adapter->pshare->phw->MCSTxAgcOffset_C[7]);
	*tmp_b2 = HAL_POWER_RANGE_CHECK(*tmp_b2 + Adapter->pshare->phw->MCSTxAgcOffset_C[6]);
	*tmp_b3 = HAL_POWER_RANGE_CHECK(*tmp_b3 + Adapter->pshare->phw->MCSTxAgcOffset_C[5]);
	*tmp_b4 = HAL_POWER_RANGE_CHECK(*tmp_b4 + Adapter->pshare->phw->MCSTxAgcOffset_C[4]);
	writeVal = (*tmp_b4 << 24) | (*tmp_b3 << 16) | (*tmp_b2 << 8) | *tmp_b1;
	HAL_RTL_W32(REG_BB_TXAGC_C_MCS7_MCS4_AC, writeVal);
	tmp_dw = HAL_RTL_R32(REG_BB_TXAGC_C_MCS11_MCS8_AC);
	get_tx_pwr_88XX(tmp_dw, tmp_b1, tmp_b2, tmp_b3, tmp_b4);
	*tmp_b1 = HAL_POWER_RANGE_CHECK(*tmp_b1 + Adapter->pshare->phw->MCSTxAgcOffset_C[11]);
	*tmp_b2 = HAL_POWER_RANGE_CHECK(*tmp_b2 + Adapter->pshare->phw->MCSTxAgcOffset_C[10]);
	*tmp_b3 = HAL_POWER_RANGE_CHECK(*tmp_b3 + Adapter->pshare->phw->MCSTxAgcOffset_C[9]);
	*tmp_b4 = HAL_POWER_RANGE_CHECK(*tmp_b4 + Adapter->pshare->phw->MCSTxAgcOffset_C[8]);
	writeVal = (*tmp_b4 << 24) | (*tmp_b3 << 16) | (*tmp_b2 << 8) | *tmp_b1;
	HAL_RTL_W32(REG_BB_TXAGC_C_MCS11_MCS8_AC, writeVal);
	tmp_dw = HAL_RTL_R32(REG_BB_TXAGC_C_MCS15_MCS12_AC);
	get_tx_pwr_88XX(tmp_dw, tmp_b1, tmp_b2, tmp_b3, tmp_b4);
	*tmp_b1 = HAL_POWER_RANGE_CHECK(*tmp_b1 + Adapter->pshare->phw->MCSTxAgcOffset_C[15]);
	*tmp_b2 = HAL_POWER_RANGE_CHECK(*tmp_b2 + Adapter->pshare->phw->MCSTxAgcOffset_C[14]);
	*tmp_b3 = HAL_POWER_RANGE_CHECK(*tmp_b3 + Adapter->pshare->phw->MCSTxAgcOffset_C[13]);
	*tmp_b4 = HAL_POWER_RANGE_CHECK(*tmp_b4 + Adapter->pshare->phw->MCSTxAgcOffset_C[12]);
	writeVal = (*tmp_b4 << 24) | (*tmp_b3 << 16) | (*tmp_b2 << 8) | *tmp_b1;
	HAL_RTL_W32(REG_BB_TXAGC_C_MCS15_MCS12_AC, writeVal);
	tmp_dw = HAL_RTL_R32(REG_BB_TXAGC_C_NSS1INDEX3_NSS1INDEX0_AC);
	get_tx_pwr_88XX(tmp_dw, tmp_b1, tmp_b2, tmp_b3, tmp_b4);
	*tmp_b1 = HAL_POWER_RANGE_CHECK(*tmp_b1 + Adapter->pshare->phw->VHTTxAgcOffset_C[3]);
	*tmp_b2 = HAL_POWER_RANGE_CHECK(*tmp_b2 + Adapter->pshare->phw->VHTTxAgcOffset_C[2]);
	*tmp_b3 = HAL_POWER_RANGE_CHECK(*tmp_b3 + Adapter->pshare->phw->VHTTxAgcOffset_C[1]);
	*tmp_b4 = HAL_POWER_RANGE_CHECK(*tmp_b4 + Adapter->pshare->phw->VHTTxAgcOffset_C[0]);
	writeVal = (*tmp_b4 << 24) | (*tmp_b3 << 16) | (*tmp_b2 << 8) | *tmp_b1;
	HAL_RTL_W32(REG_BB_TXAGC_C_NSS1INDEX3_NSS1INDEX0_AC, writeVal);
	tmp_dw = HAL_RTL_R32(REG_BB_TXAGC_C_NSS1INDEX7_NSS1INDEX4_AC);
	get_tx_pwr_88XX(tmp_dw, tmp_b1, tmp_b2, tmp_b3, tmp_b4);
	*tmp_b1 = HAL_POWER_RANGE_CHECK(*tmp_b1 + Adapter->pshare->phw->VHTTxAgcOffset_C[7]);
	*tmp_b2 = HAL_POWER_RANGE_CHECK(*tmp_b2 + Adapter->pshare->phw->VHTTxAgcOffset_C[6]);
	*tmp_b3 = HAL_POWER_RANGE_CHECK(*tmp_b3 + Adapter->pshare->phw->VHTTxAgcOffset_C[5]);
	*tmp_b4 = HAL_POWER_RANGE_CHECK(*tmp_b4 + Adapter->pshare->phw->VHTTxAgcOffset_C[4]);
	writeVal = (*tmp_b4 << 24) | (*tmp_b3 << 16) | (*tmp_b2 << 8) | *tmp_b1;
	HAL_RTL_W32(REG_BB_TXAGC_C_NSS1INDEX7_NSS1INDEX4_AC, writeVal);
	tmp_dw = HAL_RTL_R32(REG_BB_TXAGC_C_NSS2INDEX1_NSS1INDEX8_AC);
	tmp_dw1= HAL_RTL_R32(REG_BB_TXAGC_C_NSS1INDEX7_NSS1INDEX4_AC);
	get_tx_pwr_88XX(tmp_dw1, tmp_b1, tmp_b2, tmp_b3, tmp_b5);
	get_tx_pwr_88XX(tmp_dw, tmp_b1, tmp_b2, tmp_b3, tmp_b4);
	*tmp_b1 = HAL_POWER_RANGE_CHECK(*tmp_b5 - Adapter->pshare->phw->VHTTxAgcOffset_C[11]);
	*tmp_b2 = HAL_POWER_RANGE_CHECK(*tmp_b5 - Adapter->pshare->phw->VHTTxAgcOffset_C[10]);
	*tmp_b3 = HAL_POWER_RANGE_CHECK(*tmp_b3 + Adapter->pshare->phw->VHTTxAgcOffset_C[9]);
	*tmp_b4 = HAL_POWER_RANGE_CHECK(*tmp_b4 + Adapter->pshare->phw->VHTTxAgcOffset_C[8]);
	writeVal = (*tmp_b4 << 24) | (*tmp_b3 << 16) | (*tmp_b2 << 8) | *tmp_b1;
	HAL_RTL_W32(REG_BB_TXAGC_C_NSS2INDEX1_NSS1INDEX8_AC, writeVal);
	tmp_dw = HAL_RTL_R32(REG_BB_TXAGC_C_NSS2INDEX5_NSS2INDEX2_AC);
	get_tx_pwr_88XX(tmp_dw, tmp_b1, tmp_b2, tmp_b3, tmp_b4);
	*tmp_b1 = HAL_POWER_RANGE_CHECK(*tmp_b1 + Adapter->pshare->phw->VHTTxAgcOffset_C[15]);
	*tmp_b2 = HAL_POWER_RANGE_CHECK(*tmp_b2 + Adapter->pshare->phw->VHTTxAgcOffset_C[14]);
	*tmp_b3 = HAL_POWER_RANGE_CHECK(*tmp_b3 + Adapter->pshare->phw->VHTTxAgcOffset_C[13]);
	*tmp_b4 = HAL_POWER_RANGE_CHECK(*tmp_b4 + Adapter->pshare->phw->VHTTxAgcOffset_C[12]);
	writeVal = (*tmp_b4 << 24) | (*tmp_b3 << 16) | (*tmp_b2 << 8) | *tmp_b1;
	HAL_RTL_W32(REG_BB_TXAGC_C_NSS2INDEX5_NSS2INDEX2_AC, writeVal);
	tmp_dw = HAL_RTL_R32(REG_BB_TXAGC_C_NSS2INDEX9_NSS2INDEX6_AC);
	get_tx_pwr_88XX(tmp_dw, tmp_b1, tmp_b2, tmp_b3, tmp_b4);
	*tmp_b1 = HAL_POWER_RANGE_CHECK(*tmp_b1 + Adapter->pshare->phw->VHTTxAgcOffset_C[19]);
	*tmp_b2 = HAL_POWER_RANGE_CHECK(*tmp_b2 + Adapter->pshare->phw->VHTTxAgcOffset_C[18]);
	*tmp_b3 = HAL_POWER_RANGE_CHECK(*tmp_b3 - Adapter->pshare->phw->VHTTxAgcOffset_C[17]);
	*tmp_b4 = HAL_POWER_RANGE_CHECK(*tmp_b4 - Adapter->pshare->phw->VHTTxAgcOffset_C[16]);
	writeVal = (*tmp_b4 << 24) | (*tmp_b3 << 16) | (*tmp_b2 << 8) | *tmp_b1;
	HAL_RTL_W32(REG_BB_TXAGC_C_NSS2INDEX9_NSS2INDEX6_AC, writeVal);
}

void TxPG_OFDM_Jaguar_D
(    
	IN  HAL_PADAPTER                Adapter
)	
{
	unsigned char b1, b2, b3, b4, b5;
	unsigned char* tmp_b1 = &b1;
	unsigned char* tmp_b2 = &b2;
	unsigned char* tmp_b3 = &b3;
	unsigned char* tmp_b4 = &b4;
	unsigned char* tmp_b5 = &b5;
	unsigned int  tmp_dw = 0;
	unsigned int  tmp_dw1 =0;
	unsigned int  writeVal = 0;

	//printk("TxPG_OFDM_8812_B!!\n");
	tmp_dw = HAL_RTL_R32(REG_BB_TXAGC_D_OFDM18_OFDM6_AC);
	get_tx_pwr_88XX(tmp_dw, tmp_b1, tmp_b2, tmp_b3, tmp_b4);
	*tmp_b1 = HAL_POWER_RANGE_CHECK(*tmp_b1 + Adapter->pshare->phw->OFDMTxAgcOffset_D[3]);
	*tmp_b2 = HAL_POWER_RANGE_CHECK(*tmp_b2 + Adapter->pshare->phw->OFDMTxAgcOffset_D[2]);
	*tmp_b3 = HAL_POWER_RANGE_CHECK(*tmp_b3 + Adapter->pshare->phw->OFDMTxAgcOffset_D[1]);
	*tmp_b4 = HAL_POWER_RANGE_CHECK(*tmp_b4 + Adapter->pshare->phw->OFDMTxAgcOffset_D[0]);
	writeVal = (*tmp_b4 << 24) | (*tmp_b3 << 16) | (*tmp_b2 << 8) | *tmp_b1;
	HAL_RTL_W32(REG_BB_TXAGC_D_OFDM18_OFDM6_AC, writeVal);
	tmp_dw = HAL_RTL_R32(REG_BB_TXAGC_D_OFDM54_OFDM24_AC);
	get_tx_pwr_88XX(tmp_dw, tmp_b1, tmp_b2, tmp_b3, tmp_b4);
	*tmp_b1 = HAL_POWER_RANGE_CHECK(*tmp_b1 + Adapter->pshare->phw->OFDMTxAgcOffset_D[7]);
	*tmp_b2 = HAL_POWER_RANGE_CHECK(*tmp_b2 + Adapter->pshare->phw->OFDMTxAgcOffset_D[6]);
	*tmp_b3 = HAL_POWER_RANGE_CHECK(*tmp_b3 + Adapter->pshare->phw->OFDMTxAgcOffset_D[5]);
	*tmp_b4 = HAL_POWER_RANGE_CHECK(*tmp_b4 + Adapter->pshare->phw->OFDMTxAgcOffset_D[4]);
	writeVal = (*tmp_b4 << 24) | (*tmp_b3 << 16) | (*tmp_b2 << 8) | *tmp_b1;
	HAL_RTL_W32(REG_BB_TXAGC_D_OFDM54_OFDM24_AC, writeVal);
	tmp_dw = HAL_RTL_R32(REG_BB_TXAGC_D_MCS3_MCS0_AC);
	get_tx_pwr_88XX(tmp_dw, tmp_b1, tmp_b2, tmp_b3, tmp_b4);
	*tmp_b1 = HAL_POWER_RANGE_CHECK(*tmp_b1 + Adapter->pshare->phw->MCSTxAgcOffset_D[3]);
	*tmp_b2 = HAL_POWER_RANGE_CHECK(*tmp_b2 + Adapter->pshare->phw->MCSTxAgcOffset_D[2]);
	*tmp_b3 = HAL_POWER_RANGE_CHECK(*tmp_b3 + Adapter->pshare->phw->MCSTxAgcOffset_D[1]);
	*tmp_b4 = HAL_POWER_RANGE_CHECK(*tmp_b4 + Adapter->pshare->phw->MCSTxAgcOffset_D[0]);
	writeVal = (*tmp_b4 << 24) | (*tmp_b3 << 16) | (*tmp_b2 << 8) | *tmp_b1;
	HAL_RTL_W32(REG_BB_TXAGC_D_MCS3_MCS0_AC, writeVal);
	tmp_dw = HAL_RTL_R32(REG_BB_TXAGC_D_MCS7_MCS4_AC);
	get_tx_pwr_88XX(tmp_dw, tmp_b1, tmp_b2, tmp_b3, tmp_b4);
	*tmp_b1 = HAL_POWER_RANGE_CHECK(*tmp_b1 + Adapter->pshare->phw->MCSTxAgcOffset_D[7]);
	*tmp_b2 = HAL_POWER_RANGE_CHECK(*tmp_b2 + Adapter->pshare->phw->MCSTxAgcOffset_D[6]);
	*tmp_b3 = HAL_POWER_RANGE_CHECK(*tmp_b3 + Adapter->pshare->phw->MCSTxAgcOffset_D[5]);
	*tmp_b4 = HAL_POWER_RANGE_CHECK(*tmp_b4 + Adapter->pshare->phw->MCSTxAgcOffset_D[4]);
	writeVal = (*tmp_b4 << 24) | (*tmp_b3 << 16) | (*tmp_b2 << 8) | *tmp_b1;
	HAL_RTL_W32(REG_BB_TXAGC_D_MCS7_MCS4_AC, writeVal);
	tmp_dw = HAL_RTL_R32(REG_BB_TXAGC_D_MCS11_MCS8_AC);
	get_tx_pwr_88XX(tmp_dw, tmp_b1, tmp_b2, tmp_b3, tmp_b4);
	*tmp_b1 = HAL_POWER_RANGE_CHECK(*tmp_b1 + Adapter->pshare->phw->MCSTxAgcOffset_D[11]);
	*tmp_b2 = HAL_POWER_RANGE_CHECK(*tmp_b2 + Adapter->pshare->phw->MCSTxAgcOffset_D[10]);
	*tmp_b3 = HAL_POWER_RANGE_CHECK(*tmp_b3 + Adapter->pshare->phw->MCSTxAgcOffset_D[9]);
	*tmp_b4 = HAL_POWER_RANGE_CHECK(*tmp_b4 + Adapter->pshare->phw->MCSTxAgcOffset_D[8]);
	writeVal = (*tmp_b4 << 24) | (*tmp_b3 << 16) | (*tmp_b2 << 8) | *tmp_b1;
	HAL_RTL_W32(REG_BB_TXAGC_D_MCS11_MCS8_AC, writeVal);
	tmp_dw = HAL_RTL_R32(REG_BB_TXAGC_D_MCS15_MCS12_AC);
	get_tx_pwr_88XX(tmp_dw, tmp_b1, tmp_b2, tmp_b3, tmp_b4);
	*tmp_b1 = HAL_POWER_RANGE_CHECK(*tmp_b1 + Adapter->pshare->phw->MCSTxAgcOffset_D[15]);
	*tmp_b2 = HAL_POWER_RANGE_CHECK(*tmp_b2 + Adapter->pshare->phw->MCSTxAgcOffset_D[14]);
	*tmp_b3 = HAL_POWER_RANGE_CHECK(*tmp_b3 + Adapter->pshare->phw->MCSTxAgcOffset_D[13]);
	*tmp_b4 = HAL_POWER_RANGE_CHECK(*tmp_b4 + Adapter->pshare->phw->MCSTxAgcOffset_D[12]);
	writeVal = (*tmp_b4 << 24) | (*tmp_b3 << 16) | (*tmp_b2 << 8) | *tmp_b1;
	HAL_RTL_W32(REG_BB_TXAGC_D_MCS15_MCS12_AC, writeVal);
	tmp_dw = HAL_RTL_R32(REG_BB_TXAGC_D_NSS1INDEX3_NSS1INDEX0_AC);
	get_tx_pwr_88XX(tmp_dw, tmp_b1, tmp_b2, tmp_b3, tmp_b4);
	*tmp_b1 = HAL_POWER_RANGE_CHECK(*tmp_b1 + Adapter->pshare->phw->VHTTxAgcOffset_D[3]);
	*tmp_b2 = HAL_POWER_RANGE_CHECK(*tmp_b2 + Adapter->pshare->phw->VHTTxAgcOffset_D[2]);
	*tmp_b3 = HAL_POWER_RANGE_CHECK(*tmp_b3 + Adapter->pshare->phw->VHTTxAgcOffset_D[1]);
	*tmp_b4 = HAL_POWER_RANGE_CHECK(*tmp_b4 + Adapter->pshare->phw->VHTTxAgcOffset_D[0]);
	writeVal = (*tmp_b4 << 24) | (*tmp_b3 << 16) | (*tmp_b2 << 8) | *tmp_b1;
	HAL_RTL_W32(REG_BB_TXAGC_D_NSS1INDEX3_NSS1INDEX0_AC, writeVal);
	tmp_dw = HAL_RTL_R32(REG_BB_TXAGC_D_NSS1INDEX7_NSS1INDEX4_AC);
	get_tx_pwr_88XX(tmp_dw, tmp_b1, tmp_b2, tmp_b3, tmp_b4);
	*tmp_b1 = HAL_POWER_RANGE_CHECK(*tmp_b1 + Adapter->pshare->phw->VHTTxAgcOffset_D[7]);
	*tmp_b2 = HAL_POWER_RANGE_CHECK(*tmp_b2 + Adapter->pshare->phw->VHTTxAgcOffset_D[6]);
	*tmp_b3 = HAL_POWER_RANGE_CHECK(*tmp_b3 + Adapter->pshare->phw->VHTTxAgcOffset_D[5]);
	*tmp_b4 = HAL_POWER_RANGE_CHECK(*tmp_b4 + Adapter->pshare->phw->VHTTxAgcOffset_D[4]);
	writeVal = (*tmp_b4 << 24) | (*tmp_b3 << 16) | (*tmp_b2 << 8) | *tmp_b1;
	HAL_RTL_W32(REG_BB_TXAGC_D_NSS1INDEX7_NSS1INDEX4_AC, writeVal);
	tmp_dw = HAL_RTL_R32(REG_BB_TXAGC_D_NSS2INDEX1_NSS1INDEX8_AC);
	tmp_dw1= HAL_RTL_R32(REG_BB_TXAGC_D_NSS1INDEX7_NSS1INDEX4_AC);
	get_tx_pwr_88XX(tmp_dw1, tmp_b1, tmp_b2, tmp_b3, tmp_b5);
	get_tx_pwr_88XX(tmp_dw, tmp_b1, tmp_b2, tmp_b3, tmp_b4);
	*tmp_b1 = HAL_POWER_RANGE_CHECK(*tmp_b5 - Adapter->pshare->phw->VHTTxAgcOffset_D[11]);
	*tmp_b2 = HAL_POWER_RANGE_CHECK(*tmp_b5 - Adapter->pshare->phw->VHTTxAgcOffset_D[10]);
	*tmp_b3 = HAL_POWER_RANGE_CHECK(*tmp_b3 + Adapter->pshare->phw->VHTTxAgcOffset_D[9]);
	*tmp_b4 = HAL_POWER_RANGE_CHECK(*tmp_b4 + Adapter->pshare->phw->VHTTxAgcOffset_D[8]);
	writeVal = (*tmp_b4 << 24) | (*tmp_b3 << 16) | (*tmp_b2 << 8) | *tmp_b1;
	HAL_RTL_W32(REG_BB_TXAGC_D_NSS2INDEX1_NSS1INDEX8_AC, writeVal);
	tmp_dw = HAL_RTL_R32(REG_BB_TXAGC_D_NSS2INDEX5_NSS2INDEX2_AC);
	get_tx_pwr_88XX(tmp_dw, tmp_b1, tmp_b2, tmp_b3, tmp_b4);
	*tmp_b1 = HAL_POWER_RANGE_CHECK(*tmp_b1 + Adapter->pshare->phw->VHTTxAgcOffset_D[15]);
	*tmp_b2 = HAL_POWER_RANGE_CHECK(*tmp_b2 + Adapter->pshare->phw->VHTTxAgcOffset_D[14]);
	*tmp_b3 = HAL_POWER_RANGE_CHECK(*tmp_b3 + Adapter->pshare->phw->VHTTxAgcOffset_D[13]);
	*tmp_b4 = HAL_POWER_RANGE_CHECK(*tmp_b4 + Adapter->pshare->phw->VHTTxAgcOffset_D[12]);
	writeVal = (*tmp_b4 << 24) | (*tmp_b3 << 16) | (*tmp_b2 << 8) | *tmp_b1;
	HAL_RTL_W32(REG_BB_TXAGC_D_NSS2INDEX5_NSS2INDEX2_AC, writeVal);
	tmp_dw = HAL_RTL_R32(REG_BB_TXAGC_D_NSS2INDEX9_NSS2INDEX6_AC);
	get_tx_pwr_88XX(tmp_dw, tmp_b1, tmp_b2, tmp_b3, tmp_b4);
	*tmp_b1 = HAL_POWER_RANGE_CHECK(*tmp_b1 + Adapter->pshare->phw->VHTTxAgcOffset_D[19]);
	*tmp_b2 = HAL_POWER_RANGE_CHECK(*tmp_b2 + Adapter->pshare->phw->VHTTxAgcOffset_D[18]);
	*tmp_b3 = HAL_POWER_RANGE_CHECK(*tmp_b3 - Adapter->pshare->phw->VHTTxAgcOffset_D[17]);
	*tmp_b4 = HAL_POWER_RANGE_CHECK(*tmp_b4 - Adapter->pshare->phw->VHTTxAgcOffset_D[16]);
	writeVal = (*tmp_b4 << 24) | (*tmp_b3 << 16) | (*tmp_b2 << 8) | *tmp_b1;
	HAL_RTL_W32(REG_BB_TXAGC_D_NSS2INDEX9_NSS2INDEX6_AC, writeVal);
}
#endif


void
checkSwitchChannelPara88XX_AC(
	IN  HAL_PADAPTER    Adapter,
	IN  u4Byte          channel
)
{
	RT_TRACE_F(COMP_RF, DBG_TRACE, ("Switch to channel [%x] \n", channel));

	// BW
	RT_TRACE_F(COMP_RF, DBG_TRACE, ("BW parameter \n"));
	RT_TRACE_F(COMP_RF, DBG_TRACE, ("CCK en 0x808[28][29]=%x \n", HAL_RTL_R32(0x808)));
	RT_TRACE_F(COMP_RF, DBG_TRACE, ("rf_mode BB 0x8ac[21,20,9:6,1,0]=%x \n", HAL_RTL_R32(0x8ac)));
	RT_TRACE_F(COMP_RF, DBG_TRACE, ("rf_mode MAC 0x8[7:6]=%x \n", HAL_RTL_R8(0x8)));
	RT_TRACE_F(COMP_RF, DBG_TRACE, ("RF TRX_BW RF 0x18=%x \n", PHY_QueryRFReg(Adapter, 0, 0x18, 0xffffffff, 1)));


	// 2.4G <=> 5G
	RT_TRACE_F(COMP_RF, DBG_TRACE, ("2.4G 5G switch \n"));
	RT_TRACE_F(COMP_RF, DBG_TRACE, ("CCK_CHECK_en 0x454[7]=%x \n", HAL_RTL_R8(0x454)));
	RT_TRACE_F(COMP_RF, DBG_TRACE, ("AGC table select 0x82c[1:0]=%x \n", HAL_RTL_R32(0x82c)));
	RT_TRACE_F(COMP_RF, DBG_TRACE, ("RFE module 0xcb0[7:4]= %x \n", HAL_RTL_R32(0xcb0)));
	RT_TRACE_F(COMP_RF, DBG_TRACE, ("RFE module 0xcb0[7:4]= %x \n", HAL_RTL_R32(0xeb0)));
	RT_TRACE_F(COMP_RF, DBG_TRACE, ("TRSW module 0xcb0[23:16]= %x \n", HAL_RTL_R32(0xcb0)));
	RT_TRACE_F(COMP_RF, DBG_TRACE, ("TRSW module 0xcb0[23:16]= %x \n", HAL_RTL_R32(0xeb0)));
	RT_TRACE_F(COMP_RF, DBG_TRACE, ("fc_area 0x860[28:17]= %x \n", HAL_RTL_R32(0x860)));
	RT_TRACE_F(COMP_RF, DBG_TRACE, ("RF MOD AG reg18[18,17,16,9,8]=%x\n", PHY_QueryRFReg(Adapter, 0, 0x18, 0xc00, 1)));
}

#if 0
void checkSwitchChannelPara88XX(IN  HAL_PADAPTER    Adapter, unsigned char channel)
{
	printk("Switch to channel [%x] \n", channel);

	// BW
	printk("BW parameter \n");
	printk("CCK en 0x808[28][29]=%x \n", HAL_RTL_R32(0x808));
	printk("rf_mode BB 0x8ac[21,20,9:6,1,0]=%x \n", HAL_RTL_R32(0x8ac));
	printk("rf_mode MAC 0x8[7:6]=%x \n", HAL_RTL_R8(0x8));
	printk("RF TRX_BW RF 0x18=%x \n", PHY_QueryRFReg_88XX_AC(Adapter, 0, 0x18, 0xffffffff));


	// 2.4G <=> 5G
	printk("2.4G 5G switch \n");
	printk("CCK_CHECK_en 0x454[7]=%x \n", HAL_RTL_R8(0x454));
	printk("AGC table select 0x82c[1:0]=%x \n", HAL_RTL_R32(0x82c));
	printk("RFE module 0xcb0[7:4]= %x \n", HAL_RTL_R32(0xcb0));
	printk("RFE module 0xcb0[7:4]= %x \n", HAL_RTL_R32(0xeb0));
	printk("TRSW module 0xcb0[23:16]= %x \n", HAL_RTL_R32(0xcb0));
	printk("TRSW module 0xcb0[23:16]= %x \n", HAL_RTL_R32(0xeb0));
	printk("fc_area 0x860[28:17]= %x \n", HAL_RTL_R32(0x860));
	printk("RF MOD AG reg18[18,17,16,9,8]=%x\n", PHY_QueryRFReg_88XX_AC(Adapter, 0, 0x18, 0xc00));

	printk("0x8ac = %x\n", PHY_QueryBBReg_88XX(Adapter, 0x8ac, BIT_MASK_SET_MASKDWORD_COMMON));
	printk("0x8ac = %x\n", HAL_RTL_R32(0x8ac));
	printk("0x668 = %x\n", PHY_QueryBBReg_88XX(Adapter, 0x668, BIT_MASK_SET_MASKDWORD_COMMON));
	printk("0x668  = %x\n", HAL_RTL_R32(0x668));
	printk("0x8c4 = %x\n", PHY_QueryBBReg_88XX(Adapter, 0x8c4, BIT_MASK_SET_MASKDWORD_COMMON));
	printk("0x8c4 = %x\n", HAL_RTL_R32(0x8c4));
	printk("0x24 = %x\n", PHY_QueryBBReg_88XX(Adapter, 0x24, BIT_MASK_SET_MASKDWORD_COMMON));
	printk("0x24 = %x\n", HAL_RTL_R32(0x24));
	printk("0xa00 = %x\n", PHY_QueryBBReg_88XX(Adapter, 0xa00, BIT_MASK_SET_MASKDWORD_COMMON));
	printk("0xa00 = %x\n", HAL_RTL_R32(0xa00));
	printk("0x483 = %x\n", PHY_QueryBBReg_88XX(Adapter, 0x483, BIT_MASK_SET_MASKDWORD_COMMON));
	printk("0x483 = %x\n", HAL_RTL_R32(0x483));
	printk("0x838 = %x\n", PHY_QueryBBReg_88XX(Adapter, 0x838, BIT_MASK_SET_MASKDWORD_COMMON));
	printk("0x838 = %x\n", HAL_RTL_R32(0x838));
	printk("0x440 = %x\n", PHY_QueryBBReg_88XX(Adapter, 0x440, BIT_MASK_SET_MASKDWORD_COMMON));
	printk("0x440 = %x\n", HAL_RTL_R32(0x440));
	printk("0x848 = %x\n", PHY_QueryBBReg_88XX(Adapter, 0x848, BIT_MASK_SET_MASKDWORD_COMMON));
	printk("0x848 = %x\n", HAL_RTL_R32(0x848));
}
#endif


#if 1 //eric-8822
int conver_bw_hal_2_odm(int bandwidth)
{
	int bandwidth_odm = ODM_BW20M;

	switch (bandwidth) {
		case HT_CHANNEL_WIDTH_AC_5:
			bandwidth_odm = ODM_BW5M;
			break;
		case HT_CHANNEL_WIDTH_AC_10:
			bandwidth_odm = ODM_BW10M;
			break;
		case HT_CHANNEL_WIDTH_AC_20:
			bandwidth_odm = ODM_BW20M;
			break;
		case HT_CHANNEL_WIDTH_AC_40:
			bandwidth_odm = ODM_BW40M;
			break;
		case HT_CHANNEL_WIDTH_AC_80:
			bandwidth_odm = ODM_BW80M;
			break;
		case HT_CHANNEL_WIDTH_AC_160:
			bandwidth_odm = ODM_BW160M;
			break;
	}

	return bandwidth_odm;
}
#endif

void
SwBWMode88XX_AC(
	IN  HAL_PADAPTER    Adapter,
	IN  u4Byte          bandwidth,
	IN  s4Byte          offset
)
{

	u4Byte rfBand = 0, clkSel = 0 , rrSR = 0, tmp_rf = 0;
	u1Byte dataSC = 0;
	u1Byte SCSettingOf40 = 0;
	u1Byte SCSettingOf20 = 0;
    u1Byte Current_band;
    
	if (HAL_VAR_DOT11CHANNEL <= CHANNEL_MAX_NUMBER_2G)
		Current_band = RF88XX_BAND_ON_2_4G;
	else
		Current_band = RF88XX_BAND_ON_5G;
	SCSettingOf20 = HAL_VAR_TXSC_20;
	SCSettingOf40 = HAL_VAR_TXSC_40;

#if (RTL8822B_SUPPORT == 1)  
	if(IS_HARDWARE_TYPE_8822BE(Adapter)) {
			
		int bandwidth_odm = conver_bw_hal_2_odm(bandwidth);

		rfBand = HAL_RTL_R32(REG_WMAC_TRXPTCL_CTL);
		rfBand &= ~(BIT(7) | BIT(8));
		
		dataSC = ( SCSettingOf20 | (SCSettingOf40 << 4)); 
		
		switch (bandwidth) {

			case HT_CHANNEL_WIDTH_AC_40:

				rfBand |= BIT(7);
				break;
		
			case HT_CHANNEL_WIDTH_AC_80:

				rfBand |= BIT(8);
				break;
		}

		HAL_RTL_W32(REG_WMAC_TRXPTCL_CTL, rfBand);
		HAL_RTL_W8(REG_DATA_SC, dataSC);

		if(Adapter->pshare->rf_ft_var.mp_specific)
			config_phydm_trx_mode_8822b((&(Adapter->pshare->_dmODM)), Adapter->pshare->mp_antenna_tx, Adapter->pshare->mp_antenna_rx, 0);
		//else
			//config_phydm_trx_mode_8822b((&(Adapter->pshare->_dmODM)), (ODM_RF_A|ODM_RF_B), (ODM_RF_A|ODM_RF_B), Adapter->pmib->dot11RFEntry.tx2path);
			
		config_phydm_switch_bandwidth_8822b((&(Adapter->pshare->_dmODM)), SCSettingOf20, bandwidth_odm);
		
		return;
	}
#endif

	//3 ========== <1> Set rf_mode 0x8ac / 0x24 (REG_AFE_CTRL1, BIT_MAC_CLK_SEL)/ 0x668

	rfBand = HAL_RTL_R32(REG_WMAC_TRXPTCL_CTL);
	rfBand &= ~(BIT(7) | BIT(8));

#if IS_RTL8814A_SERIES
    if (IS_HARDWARE_TYPE_8814A(Adapter)) {
        clkSel = HAL_RTL_R32(REG_SYS_CLK_CTRL);
        clkSel &= ~(BIT(6)|BIT(7));
    } else
#endif
    {
        clkSel = HAL_RTL_R32(REG_AFE_CTRL1);
        clkSel &= ~(BIT(20)|BIT(21));
    }
	
	switch (bandwidth) {
	case HT_CHANNEL_WIDTH_AC_5:
#if IS_RTL8814A_SERIES
        if (IS_HARDWARE_TYPE_8814A(Adapter)) {
            if(IS_HAL_TEST_CHIP(Adapter)) {
                PHY_SetBBReg_88XX(Adapter, REG_BB_RFMOD_AC, 0x103103C3, 0x200040); // 5M : 0x8ac[28,21,20,16,9,8,7,6,1,0]=10'b0100000100
            } else {
                PHY_SetBBReg_88XX(Adapter, REG_BB_RFMOD_AC, 0x103103C3, 0x200140); // 5M : 0x8ac[28,21,20,16,9,8,7,6,1,0]=10'b0100010100
            }
            clkSel |= BIT(7);
    		HAL_RTL_W32(REG_SYS_CLK_CTRL, clkSel);
            
        } else
#endif
        {
            PHY_SetBBReg_88XX(Adapter, REG_BB_RFMOD_AC, 0x003003C3, 0x100040); // 5M : 0x8ac[21,20,9:6,1,0]=8'b01000100
            clkSel |= BIT(21);
    		HAL_RTL_W32(REG_AFE_CTRL1, clkSel);
        }
		HAL_RTL_W32(REG_WMAC_TRXPTCL_CTL, rfBand);
		break;
	case HT_CHANNEL_WIDTH_AC_10:
#if IS_RTL8814A_SERIES
        if (IS_HARDWARE_TYPE_8814A(Adapter)) {
            if(IS_HAL_TEST_CHIP(Adapter)) {
                PHY_SetBBReg_88XX(Adapter, REG_BB_RFMOD_AC, 0x103103C3, 0x300180); // 10M :0x8ac[28,21,20,16,9,8,7,6,1,0]=10'b0110011000
            } else {
                PHY_SetBBReg_88XX(Adapter, REG_BB_RFMOD_AC, 0x103103C3, 0x300280); // 10M :0x8ac[28,21,20,16,9,8,7,6,1,0]=10'b0110101000
            }
            clkSel |= BIT(6);
            HAL_RTL_W32(REG_SYS_CLK_CTRL, clkSel);
        } else
#endif
        {
            PHY_SetBBReg_88XX(Adapter, REG_BB_RFMOD_AC, 0x003003C3, 0x200180); // 10M : 0x8ac[21,20,9:6,1,0]=8'b10011000
    		clkSel |= BIT(20);
    		HAL_RTL_W32(REG_AFE_CTRL1, clkSel);
        }
		HAL_RTL_W32(REG_WMAC_TRXPTCL_CTL, rfBand);
		break;
	case HT_CHANNEL_WIDTH_AC_20:
#if IS_RTL8814A_SERIES
        if (IS_HARDWARE_TYPE_8814A(Adapter)) {
            if(IS_HAL_TEST_CHIP(Adapter)) {            
                if (Current_band == RF88XX_BAND_ON_5G) { // 5G
                    PHY_SetBBReg_88XX(Adapter, REG_BB_RFMOD_AC, 0x103103C3, 0x10000200); // 20M : 0x8ac[28,21,20,16,9,8,7,6,1,0]=10'b1000100000
                } else { // 2.4G
                    PHY_SetBBReg_88XX(Adapter, REG_BB_RFMOD_AC, 0x103103C3, 0x10010000); // 20M : 0x8ac[28,21,20,16,9,8,7,6,1,0]=10'b1001000000
                }
            } else {
                if (Current_band == RF88XX_BAND_ON_5G) { // 5G
                    if (IS_HAL_A_CUT(Adapter)) {
                        // MP chip - A-cut
                    PHY_SetBBReg_88XX(Adapter, REG_BB_RFMOD_AC, 0x103103C3, 0x10000300); // 20M : 0x8ac[28,21,20,16,9,8,7,6,1,0]=10'b1000110000
                    } else {
                        // MP chip - B-cut
                        PHY_SetBBReg_88XX(Adapter, REG_BB_RFMOD_AC, 0x103103C3, 0x10010100); // 20M : 0x8ac[28,21,20,16,9,8,7,6,1,0]=10'b1001010000
                    }
                } else { // 2.4G
                    PHY_SetBBReg_88XX(Adapter, REG_BB_RFMOD_AC, 0x103103C3, 0x10010100); // 20M : 0x8ac[28,21,20,16,9,8,7,6,1,0]=10'b1001010000
                }            
            }
            HAL_RTL_W32(REG_SYS_CLK_CTRL, clkSel);
        } else
#endif
        {
            PHY_SetBBReg_88XX(Adapter, REG_BB_RFMOD_AC, 0x003003C3, 0x300200); // 20M : 0x8ac[21,20,9:6,1,0]=8'b11100000        
		    HAL_RTL_W32(REG_AFE_CTRL1, clkSel);
        }
		HAL_RTL_W32(REG_WMAC_TRXPTCL_CTL, rfBand);
		break;
	case HT_CHANNEL_WIDTH_AC_40:
#if IS_RTL8814A_SERIES
        if (IS_HARDWARE_TYPE_8814A(Adapter)) {
            if(IS_HAL_TEST_CHIP(Adapter)) { 
                if (Current_band == RF88XX_BAND_ON_5G) { // 5G
                    PHY_SetBBReg_88XX(Adapter, REG_BB_RFMOD_AC, 0x20CC3, 0xC01); // 40M : 0x8ac[17,11,10,7,6,1,0]=7'b0110001
                } else { // 2.4G
                    PHY_SetBBReg_88XX(Adapter, REG_BB_RFMOD_AC, 0x20CC3, 0x20001); // 40M : 0x8ac[17,11,10,7,6,1,0]=7'b1000001
                }
            } else {
                if (Current_band == RF88XX_BAND_ON_5G) { // 5G
                    if (IS_HAL_A_CUT(Adapter)) {
                        // MP chip - A-cut
                    PHY_SetBBReg_88XX(Adapter, REG_BB_RFMOD_AC, 0x20CC3, 0x20001); // 40M : 0x8ac[17,11,10,7,6,1,0]=7'b1000001
                    } else {
                        // MP chip - B-cut
                        PHY_SetBBReg_88XX(Adapter, REG_BB_RFMOD_AC, 0x20C20CC3, 0x20020401); // 40M : 0x8ac[29,23,22,17,11,10,7,6,1,0]=10'b1001010001
                    }
                } else { // 2.4G
                    PHY_SetBBReg_88XX(Adapter, REG_BB_RFMOD_AC, 0x20CC3, 0x20401); // 40M : 0x8ac[17,11,10,7,6,1,0]=7'b1010001
                }
            }
            HAL_RTL_W32(REG_SYS_CLK_CTRL, clkSel);
        } else
#endif
        {
            PHY_SetBBReg_88XX(Adapter, REG_BB_RFMOD_AC, 0x003003C3, 0x300201); // 40M : 0x8ac[21,20,9:6,1,0]=8'b11100001
       		HAL_RTL_W32(REG_AFE_CTRL1, clkSel);
        }
		rfBand |= BIT(7);
		HAL_RTL_W32(REG_WMAC_TRXPTCL_CTL, rfBand);
		break;
	case HT_CHANNEL_WIDTH_AC_80:
#if IS_RTL8814A_SERIES
        if (IS_HARDWARE_TYPE_8814A(Adapter)) {
            if(IS_HAL_TEST_CHIP(Adapter)) {
                PHY_SetBBReg_88XX(Adapter, REG_BB_RFMOD_AC, 0xC3, 0x2); // 80M : 0x8ac[7,6,1,0]=4'b0010
            } else {
                if (IS_HAL_A_CUT(Adapter)) {
                    // MP chip - A-cut
            PHY_SetBBReg_88XX(Adapter, REG_BB_RFMOD_AC, 0xC3, 0x2); // 80M : 0x8ac[7,6,1,0]=4'b0010
                } else {
                    // MP chip - B-cut
                    PHY_SetBBReg_88XX(Adapter, REG_BB_RFMOD_AC, 0x430430C3, 0x40041002); // 80M : 0x8ac[30,25,24,18,13,12,7,6,1,0]=10'b1001010010
                }
            }
            HAL_RTL_W32(REG_SYS_CLK_CTRL, clkSel);
        } else
#endif
        {
            PHY_SetBBReg_88XX(Adapter, REG_BB_RFMOD_AC, 0x003003C3, 0x300202); // 80M : 0x8ac[21,20,9:6,1,0]=8'b11100010 
       		HAL_RTL_W32(REG_AFE_CTRL1, clkSel);
        }
		rfBand |= BIT(8);
		HAL_RTL_W32(REG_WMAC_TRXPTCL_CTL, rfBand);
		break;
	}	

	//3 ========== <2> Set adc buff clk 0x8c4

#if IS_RTL8814A_SERIES
    if (IS_HARDWARE_TYPE_8814A(Adapter)) {
        if(IS_HAL_TEST_CHIP(Adapter) || IS_HAL_A_CUT(Adapter)) {    
            switch (bandwidth)
            {
            case HT_CHANNEL_WIDTH_AC_5:
            case HT_CHANNEL_WIDTH_AC_10:
            case HT_CHANNEL_WIDTH_AC_20:
                if (Current_band == RF88XX_BAND_ON_5G) { // 5G
                    PHY_SetBBReg_88XX(Adapter, REG_BB_ADC_BUF_CLK_AC, BIT(30), 0);
                    PHY_SetBBReg_88XX(Adapter, REG_BB_ADC_BUF_CLK_2_AC, BIT(31), 1);
                } else {
                    PHY_SetBBReg_88XX(Adapter, REG_BB_ADC_BUF_CLK_AC, BIT(30), 1);
                }
                break;
            case HT_CHANNEL_WIDTH_AC_40:
                if (Current_band == RF88XX_BAND_ON_5G) { // 5G
                    PHY_SetBBReg_88XX(Adapter, REG_BB_ADC_BUF_CLK_AC, BIT(30), 0);
                    PHY_SetBBReg_88XX(Adapter, REG_BB_ADC_BUF_CLK_2_AC, BIT(31), 0);
                } else {
                    PHY_SetBBReg_88XX(Adapter, REG_BB_ADC_BUF_CLK_AC, BIT(30), 1);
                }
                break;
            case HT_CHANNEL_WIDTH_AC_80:
                PHY_SetBBReg_88XX(Adapter, REG_BB_ADC_BUF_CLK_AC, BIT(30), 1);
                break;
            }
        } else {
            switch (bandwidth)
            {
            case HT_CHANNEL_WIDTH_AC_5:
            case HT_CHANNEL_WIDTH_AC_10:
                PHY_SetBBReg_88XX(Adapter, REG_BB_ADC_BUF_CLK_AC, BIT(30), 0);
                PHY_SetBBReg_88XX(Adapter, REG_BB_ADC_BUF_CLK_2_AC, BIT(31), 1);
                break;
            case HT_CHANNEL_WIDTH_AC_20:
            case HT_CHANNEL_WIDTH_AC_40:
            case HT_CHANNEL_WIDTH_AC_80:
                PHY_SetBBReg_88XX(Adapter, REG_BB_ADC_BUF_CLK_AC, BIT(30), 1);
                break;
            }
        }
        
    } else 
#endif
    {
    	switch (bandwidth)
    	{
    	case HT_CHANNEL_WIDTH_AC_5:
    		PHY_SetBBReg_88XX(Adapter, REG_BB_ADC_BUF_CLK_AC, BIT(30), 0);
    		break;
    	case HT_CHANNEL_WIDTH_AC_10:
    		PHY_SetBBReg_88XX(Adapter, REG_BB_ADC_BUF_CLK_AC, BIT(30), 0);
    		break;
    	case HT_CHANNEL_WIDTH_AC_20:
    		PHY_SetBBReg_88XX(Adapter, REG_BB_ADC_BUF_CLK_AC, BIT(30), 0);
    		break;
    	case HT_CHANNEL_WIDTH_AC_40:
    		PHY_SetBBReg_88XX(Adapter, REG_BB_ADC_BUF_CLK_AC, BIT(30), 0);
    		break;
    	case HT_CHANNEL_WIDTH_AC_80:
    		PHY_SetBBReg_88XX(Adapter, REG_BB_ADC_BUF_CLK_AC, BIT(30), 1);
    		break;
    	}
    }

	//3 ========== <3> Set primary channel 0x8ac & 0xa00, txsc 0x483

	dataSC = HAL_RTL_R8(REG_DATA_SC);

	switch (bandwidth) {
	case HT_CHANNEL_WIDTH_AC_5:
	case HT_CHANNEL_WIDTH_AC_10:
	case HT_CHANNEL_WIDTH_AC_20:
		break;

	case HT_CHANNEL_WIDTH_AC_40:
		PHY_SetBBReg_88XX(Adapter, REG_BB_RFMOD_AC, 0x3C, SCSettingOf20); //0x8ac[5:2]=1/2
#if IS_RTL8814A_SERIES
        if (IS_HARDWARE_TYPE_8814A(Adapter)) {
            // Do nothing for 8814A
        } else 
#endif
        {
            PHY_SetBBReg_88XX(Adapter, REG_BB_CCAONSEC_AC, 0xF0000000, SCSettingOf20); //0x838[31:28]=1/2
        }

		if (SCSettingOf20 == 1) {
			// BIT(4) = 1, upper sideBand
			PHY_SetBBReg_88XX(Adapter, REG_BB_CCK_SYSTEM_AC, 0x10, 0x1); //0xa00 BIT(4)=1
		} else {
			PHY_SetBBReg_88XX(Adapter, REG_BB_CCK_SYSTEM_AC, 0x10, 0x0); //0xa00 BIT(4)=0
		}

		dataSC &= 0xf0;
		dataSC |= SCSettingOf20;
		HAL_RTL_W8(REG_DATA_SC, dataSC); //0x483[3:0]=1/2
		break;

	case HT_CHANNEL_WIDTH_AC_80:
		PHY_SetBBReg_88XX(Adapter, REG_BB_RFMOD_AC, 0x3C, SCSettingOf20); //0x8ac[5:2]=1/2/3/4 offset 9&10 seems few to use
#if IS_RTL8814A_SERIES
        if (IS_HARDWARE_TYPE_8814A(Adapter)) {
            // Do nothing for 8814A
        } else 
#endif
        {
            PHY_SetBBReg_88XX(Adapter, REG_BB_CCAONSEC_AC, 0xF0000000, SCSettingOf20); //0x838[31:28]=1/2/3/4
        }

		dataSC = ( SCSettingOf20 | (SCSettingOf40 << 4)); //0x483[3:0]=1/2/3/4
		HAL_RTL_W8(REG_DATA_SC, dataSC);

		break;
	}

	//3 ========== <4> Set Equzlier step size , reg0x864[20:18]

#if IS_RTL8814A_SERIES
    if (IS_HARDWARE_TYPE_8814A(Adapter)) {
        // Do nothing for 8814A
    } else 
#endif
    {
		switch (bandwidth) {
		case HT_CHANNEL_WIDTH_AC_5:
			PHY_SetBBReg_88XX(Adapter, REG_BB_FPGA0_XB_RFINTERFACEOE_AC, (BIT20 | BIT19 | BIT18), 4);
			break;
		case HT_CHANNEL_WIDTH_AC_10:
			PHY_SetBBReg_88XX(Adapter, REG_BB_FPGA0_XB_RFINTERFACEOE_AC, (BIT20 | BIT19 | BIT18), 4);
			break;
		case HT_CHANNEL_WIDTH_AC_20:
			PHY_SetBBReg_88XX(Adapter, REG_BB_FPGA0_XB_RFINTERFACEOE_AC, (BIT20 | BIT19 | BIT18), 4);
			break;
		case HT_CHANNEL_WIDTH_AC_40:
			PHY_SetBBReg_88XX(Adapter, REG_BB_FPGA0_XB_RFINTERFACEOE_AC, (BIT20 | BIT19 | BIT18), 2);
			break;
		case HT_CHANNEL_WIDTH_AC_80:
			PHY_SetBBReg_88XX(Adapter, REG_BB_FPGA0_XB_RFINTERFACEOE_AC, (BIT20 | BIT19 | BIT18), 2);
			break;
		}
    }


    //3 ========== <> PWED_TH_FB, 0x82C[15:12]
    //3 ========== <> r_CCA_primary, 0x838[0]
#if IS_RTL8814A_SERIES
    if (IS_HARDWARE_TYPE_8814A(Adapter)) {
    	switch (bandwidth)
    	{
    	case HT_CHANNEL_WIDTH_AC_5:
    	case HT_CHANNEL_WIDTH_AC_10:
    	case HT_CHANNEL_WIDTH_AC_20:
            if(IS_HAL_TEST_CHIP(Adapter)) {
                PHY_SetBBReg_88XX(Adapter, REG_BB_AGC_TABLE_AC, (BIT15|BIT14|BIT13|BIT12), 0x9);
                PHY_SetBBReg_88XX(Adapter, REG_BB_CCAONSEC_AC, BIT0, 0x1);
            } else {
            	if (Current_band == RF88XX_BAND_ON_5G)
                	PHY_SetBBReg_88XX(Adapter, REG_BB_AGC_TABLE_AC, (BIT15|BIT14|BIT13|BIT12), 0x7);
				else		
					PHY_SetBBReg_88XX(Adapter, REG_BB_AGC_TABLE_AC, (BIT15|BIT14|BIT13|BIT12), 0x7);
            }
    		break;
    	case HT_CHANNEL_WIDTH_AC_40:            
            if(IS_HAL_TEST_CHIP(Adapter)) {
				PHY_SetBBReg_88XX(Adapter, REG_BB_AGC_TABLE_AC, (BIT15|BIT14|BIT13|BIT12), 0x5);
                PHY_SetBBReg_88XX(Adapter, REG_BB_CCAONSEC_AC, BIT0, 0x0);
            } else {
	            if (Current_band == RF88XX_BAND_ON_5G)
    	          	PHY_SetBBReg_88XX(Adapter, REG_BB_AGC_TABLE_AC, (BIT15|BIT14|BIT13|BIT12), 0x8);
				else
					PHY_SetBBReg_88XX(Adapter, REG_BB_AGC_TABLE_AC, (BIT15|BIT14|BIT13|BIT12), 0x7);
            }
    		break;
    	case HT_CHANNEL_WIDTH_AC_80:
            if(IS_HAL_TEST_CHIP(Adapter)) {
				PHY_SetBBReg_88XX(Adapter, REG_BB_AGC_TABLE_AC, (BIT15|BIT14|BIT13|BIT12), 0x7);
                PHY_SetBBReg_88XX(Adapter, REG_BB_CCAONSEC_AC, BIT0, 0x0);
            } else {
                PHY_SetBBReg_88XX(Adapter, REG_BB_AGC_TABLE_AC, (BIT15|BIT14|BIT13|BIT12), 0x3);
            }
    		break;
    	}
    }
#endif


	//3 ========== <5> 0821, L1_peak_th 0x848

#if IS_RTL8814A_SERIES
    if (IS_HARDWARE_TYPE_8814A(Adapter)) {
        // Do nothing here..
    } else 
#endif
    {
    	switch (bandwidth)
    	{
    	case HT_CHANNEL_WIDTH_AC_5:
    	case HT_CHANNEL_WIDTH_AC_10:
    	case HT_CHANNEL_WIDTH_AC_20:
        #if 0
            if(GetChipIDMIMO88XX(Adapter) == MIMO_2T2R)
    			PHY_SetBBReg_88XX(Adapter, REG_BB_L1PEAKTH_AC, 0x03C00000, 7);		// 2R 0x848[25:22] = 0x7
    		else	
    			PHY_SetBBReg_88XX(Adapter, REG_BB_L1PEAKTH_AC, 0x03C00000, 8);		// 1R 0x848[25:22] = 0x8
        #endif
            if (IS_HARDWARE_TYPE_8881A(Adapter)) {
                PHY_SetBBReg_88XX(Adapter, REG_BB_L1PEAKTH_AC, 0x03C00000, 8);
            }
    		break;
    	case HT_CHANNEL_WIDTH_AC_40:
            if (IS_HARDWARE_TYPE_8881A(Adapter)) {
    		    PHY_SetBBReg_88XX(Adapter, REG_BB_L1PEAKTH_AC, 0x03C00000, 8);
            }
    		break;
    	case HT_CHANNEL_WIDTH_AC_80:
            if (IS_HARDWARE_TYPE_8881A(Adapter)) {
    		    PHY_SetBBReg_88XX(Adapter, REG_BB_L1PEAKTH_AC, 0x03C00000, 7);
            }
    		break;
    	}	
    }

	//3 ========== <6> Set RRSR_RSC 0x440[22:21]

	rrSR = HAL_RTL_R32(REG_RRSR);
	rrSR &= ~(BIT(21) | BIT(22));

	switch (bandwidth) {
	case HT_CHANNEL_WIDTH_AC_5:
	case HT_CHANNEL_WIDTH_AC_10:
	case HT_CHANNEL_WIDTH_AC_20:
		break;
	case HT_CHANNEL_WIDTH_AC_40:
		HAL_RTL_W32(REG_RRSR, rrSR);
		break;
	case HT_CHANNEL_WIDTH_AC_80:
		HAL_RTL_W32(REG_RRSR, rrSR);
		break;
	}

	//3 ========== <7> Set RF TRX_BW rf_0x18[11:10]
	switch (bandwidth) {
	case HT_CHANNEL_WIDTH_AC_5:
	case HT_CHANNEL_WIDTH_AC_10:
	case HT_CHANNEL_WIDTH_AC_20:
		tmp_rf = 0x03;
		break;
	case HT_CHANNEL_WIDTH_AC_40:
		tmp_rf = 0x01;
		break;
	case HT_CHANNEL_WIDTH_AC_80:
		tmp_rf = 0x00;
		break;
	}

    if(GetChipIDMIMO88XX(Adapter) == MIMO_1T1R)
    {
		PHY_SetRFReg_88XX_AC(Adapter, RF88XX_PATH_A, REG_RF_CHNLBW_AC, (BIT11|BIT10),tmp_rf);
#if IS_RTL8814A_SERIES
		/* 2nd CCA enable/disable settings*/
		if(IS_HARDWARE_TYPE_8814A(Adapter)) {
			if (!Adapter->pshare->rf_ft_var.disable_2ndcca)
			GET_HAL_INTERFACE(Adapter)->PHYSetSecCCATHbyRXANT(Adapter, ANTENNA_A);
		}
#endif	
    }
    else if(GetChipIDMIMO88XX(Adapter) == MIMO_2T2R)
    {
		PHY_SetRFReg_88XX_AC(Adapter, RF88XX_PATH_A, REG_RF_CHNLBW_AC, (BIT11|BIT10),tmp_rf);
		PHY_SetRFReg_88XX_AC(Adapter, RF88XX_PATH_B, REG_RF_CHNLBW_AC, (BIT11|BIT10),tmp_rf);
#if IS_RTL8814A_SERIES
		/* 2nd CCA enable/disable settings*/
		if(IS_HARDWARE_TYPE_8814A(Adapter)) {
			if (!Adapter->pshare->rf_ft_var.disable_2ndcca)
			GET_HAL_INTERFACE(Adapter)->PHYSetSecCCATHbyRXANT(Adapter, ANTENNA_AB);
		}
#endif			
    }
#if IS_RTL8814A_SERIES
    else if(GetChipIDMIMO88XX(Adapter) == MIMO_3T3R)
    {
		PHY_SetRFReg_88XX_AC(Adapter, RF88XX_PATH_A, REG_RF_CHNLBW_AC, (BIT11|BIT10),tmp_rf);
		PHY_SetRFReg_88XX_AC(Adapter, RF88XX_PATH_B, REG_RF_CHNLBW_AC, (BIT11|BIT10),tmp_rf);
        PHY_SetRFReg_88XX_AC(Adapter, RF88XX_PATH_C, REG_RF_CHNLBW_AC, (BIT11|BIT10),tmp_rf);
		if(IS_HARDWARE_TYPE_8814A(Adapter))		
	        PHY_SetRFReg_88XX_AC(Adapter, RF88XX_PATH_D, REG_RF_CHNLBW_AC, (BIT11|BIT10),tmp_rf);
#if IS_RTL8814A_SERIES
		/* 2nd CCA enable/disable settings*/
		if(IS_HARDWARE_TYPE_8814A(Adapter)) {
			if (!Adapter->pshare->rf_ft_var.disable_2ndcca) {
#if defined(CONFIG_SLOT_0_8814_2T2R_SUPPORT) || defined(CONFIG_SLOT_1_8814_2T2R_SUPPORT)				
				if (Current_band == RF88XX_BAND_ON_5G) {
					GET_HAL_INTERFACE(Adapter)->PHYSetSecCCATHbyRXANT(Adapter, ANTENNA_AB);
				}
				else
#endif				
#if defined(CONFIG_SLOT_0_8194_2T2R_SUPPORT) || defined(CONFIG_SLOT_1_8194_2T2R_SUPPORT)
				if (Current_band == RF88XX_BAND_ON_2_4G) {
					GET_HAL_INTERFACE(Adapter)->PHYSetSecCCATHbyRXANT(Adapter, ANTENNA_AB);
				}
				else
#endif
				{
					GET_HAL_INTERFACE(Adapter)->PHYSetSecCCATHbyRXANT(Adapter, ANTENNA_ABCD);
				}
			}
		}
#endif	
    }
    else if(GetChipIDMIMO88XX(Adapter) == MIMO_4T4R)
    {
		PHY_SetRFReg_88XX_AC(Adapter, RF88XX_PATH_A, REG_RF_CHNLBW_AC, (BIT11|BIT10),tmp_rf);
		PHY_SetRFReg_88XX_AC(Adapter, RF88XX_PATH_B, REG_RF_CHNLBW_AC, (BIT11|BIT10),tmp_rf);
        PHY_SetRFReg_88XX_AC(Adapter, RF88XX_PATH_C, REG_RF_CHNLBW_AC, (BIT11|BIT10),tmp_rf);
        PHY_SetRFReg_88XX_AC(Adapter, RF88XX_PATH_D, REG_RF_CHNLBW_AC, (BIT11|BIT10),tmp_rf);
#if IS_RTL8814A_SERIES
		/* 2nd CCA enable/disable settings*/
		if(IS_HARDWARE_TYPE_8814A(Adapter)) {
			if (!Adapter->pshare->rf_ft_var.disable_2ndcca) {
#if defined(CONFIG_SLOT_0_8814_2T2R_SUPPORT) || defined(CONFIG_SLOT_1_8814_2T2R_SUPPORT)				
				if (Current_band == RF88XX_BAND_ON_5G) {
					GET_HAL_INTERFACE(Adapter)->PHYSetSecCCATHbyRXANT(Adapter, ANTENNA_AB);
				}
				else
#endif				
#if defined(CONFIG_SLOT_0_8194_2T2R_SUPPORT) || defined(CONFIG_SLOT_1_8194_2T2R_SUPPORT)
				if (Current_band == RF88XX_BAND_ON_2_4G) {
					GET_HAL_INTERFACE(Adapter)->PHYSetSecCCATHbyRXANT(Adapter, ANTENNA_AB);
				}
				else
#endif
				{
					GET_HAL_INTERFACE(Adapter)->PHYSetSecCCATHbyRXANT(Adapter, ANTENNA_ABCD);
				}
			}
		}
#endif			
    }
#endif
#if IS_RTL8814A_SERIES
    // for MP chip A-cut
    if(IS_HARDWARE_TYPE_8814A(Adapter)) {
	    if (IS_HAL_A_CUT(Adapter)) {
	        phy_ADC_CLK_8814A(Adapter);
	    }
    }
#endif
#if IS_RTL8814A_SERIES
    /* RCK for pathB/C/D. Fix 8814 MP-chip RF reg setting bug  */
    if (IS_HARDWARE_TYPE_8814A(Adapter) && !IS_HAL_TEST_CHIP(Adapter)){
        unsigned int rfPathA = PHY_QueryRFReg(Adapter, RF88XX_PATH_A, 0x1c, 0xffffffff, 1);
        PHY_SetRFReg_88XX_AC(Adapter, RF88XX_PATH_B, 0x1c, 0xffffffff, rfPathA);
        PHY_SetRFReg_88XX_AC(Adapter, RF88XX_PATH_C, 0x1c, 0xffffffff, rfPathA);
        PHY_SetRFReg_88XX_AC(Adapter, RF88XX_PATH_D, 0x1c, 0xffffffff, rfPathA);
    }
#endif

}

void
SetChannelPara88XX_AC(
	IN  HAL_PADAPTER    Adapter,
	IN  u4Byte          channel,
	IN  s4Byte          offset
)
{
	u4Byte val = channel;
	u4Byte eRFPath, curMaxRFPath;

	//AGC_Table
	// TODO: 8813AE BB/RF
	if ((GET_CHIP_VER(Adapter) == VERSION_8881A) && IS_HAL_TEST_CHIP(Adapter)) {

		if (channel <= CHANNEL_MAX_NUMBER_2G) {
			// 2G
			PHY_SetBBReg_88XX(Adapter, REG_BB_AGC_TABLE_AC, BIT_MASK_BB_BAGC_TABLE_AC, 0);
		} else {
			//5G
			PHY_SetBBReg_88XX(Adapter, REG_BB_AGC_TABLE_AC, BIT_MASK_BB_BAGC_TABLE_AC, 1);
		}
    }
#if IS_RTL8814A_SERIES
    else if (GET_CHIP_VER(Adapter) == VERSION_8814A) {
        if (channel <= CHANNEL_MAX_NUMBER_2G) {
            // 2G
            PHY_SetBBReg_88XX(Adapter, REG_BB_AGC_TABLE_AC_V2, BIT_MASK_BB_BAGC_TABLE_AC_V2, 0);
        }
        else {
            //5G
            if ( IS_HARDWARE_TYPE_8814A(Adapter) && (IS_HAL_TEST_CHIP(Adapter)==FALSE)) {
                if (36 <= channel && channel <= 64) {
                    PHY_SetBBReg_88XX(Adapter, REG_BB_AGC_TABLE_AC_V2, BIT_MASK_BB_BAGC_TABLE_AC_V2, 1);
                } else if (100 <= channel && channel <= 144) {
                    PHY_SetBBReg_88XX(Adapter, REG_BB_AGC_TABLE_AC_V2, BIT_MASK_BB_BAGC_TABLE_AC_V2, 2);
                } else if (149 <= channel) {
                    PHY_SetBBReg_88XX(Adapter, REG_BB_AGC_TABLE_AC_V2, BIT_MASK_BB_BAGC_TABLE_AC_V2, 3);
                } else {
                    printk("%s(%d): REG_BB_AGC_TABLE setting error !!! \n", __FUNCTION__, __LINE__);
                }
            } else {
                PHY_SetBBReg_88XX(Adapter, REG_BB_AGC_TABLE_AC_V2, BIT_MASK_BB_BAGC_TABLE_AC_V2, 1);
            }
        }
    }
#endif
    else {
		if (channel <= CHANNEL_MAX_NUMBER_2G) {
			// 2G
			PHY_SetBBReg_88XX(Adapter, REG_BB_AGC_TABLE_AC_V1, BIT_MASK_BB_BAGC_TABLE_AC_V1, 0);
		} else {
			//5G
			if ((GET_CHIP_VER(Adapter) == VERSION_8881A)) {
				if (channel <= 64) {
					PHY_SetBBReg_88XX(Adapter, REG_BB_AGC_TABLE_AC_V1, BIT_MASK_BB_BAGC_TABLE_AC_V1, 1);
				} else if (channel >= 100 && channel <= 140) {
					PHY_SetBBReg_88XX(Adapter, REG_BB_AGC_TABLE_AC_V1, BIT_MASK_BB_BAGC_TABLE_AC_V1, 2);
				} else {
					PHY_SetBBReg_88XX(Adapter, REG_BB_AGC_TABLE_AC_V1, BIT_MASK_BB_BAGC_TABLE_AC_V1, 3);
				}
			} else {
				if (channel <= 64) {
					PHY_SetBBReg_88XX(Adapter, REG_BB_AGC_TABLE_AC_V1, BIT_MASK_BB_BAGC_TABLE_AC_V1, 1);
				} else {
					PHY_SetBBReg_88XX(Adapter, REG_BB_AGC_TABLE_AC_V1, BIT_MASK_BB_BAGC_TABLE_AC_V1, 2);
				}
			}
		}
	}

	// fc_area
	if (36 <= channel && channel <= 48) {
		PHY_SetBBReg_88XX(Adapter, REG_BB_FC_AREA_AC, 0x1ffe0000, 0x494); //0x860[28:17]=0x494
	} else if (50 <= channel && channel <= 64) {
		PHY_SetBBReg_88XX(Adapter, REG_BB_FC_AREA_AC, 0x1ffe0000, 0x453); //0x860[28:17]=0x453
	} else if (100 <= channel && channel <= 116) {
		PHY_SetBBReg_88XX(Adapter, REG_BB_FC_AREA_AC, 0x1ffe0000, 0x452); //0x860[28:17]=0x452
	} else if (118 <= channel && channel <= 165) {
		PHY_SetBBReg_88XX(Adapter, REG_BB_FC_AREA_AC, 0x1ffe0000, 0x412); //0x860[28:17]=0x412
	} else {
		PHY_SetBBReg_88XX(Adapter, REG_BB_FC_AREA_AC, 0x1ffe0000, 0x96a); //0x860[28:17]=0x96a
	}

	// RF_MOD_AG
	if (channel <= 14) {
		PHY_SetRFReg_88XX_AC(Adapter, RF88XX_PATH_A, REG_RF_CHNLBW_AC, 0x70300, 0x0); //RF_A reg18[18,17,16,9,8]=4'b00101
		PHY_SetRFReg_88XX_AC(Adapter, RF88XX_PATH_B, REG_RF_CHNLBW_AC, 0x70300, 0x0); //RF_B reg18[18,17,16,9,8]=4'b00101
#if IS_RTL8814A_SERIES
       if (GET_CHIP_VER(Adapter) == VERSION_8814A) {
           PHY_SetRFReg_88XX_AC(Adapter,RF88XX_PATH_C,REG_RF_CHNLBW_AC,0x70300,0x0); //RF_C reg18[18,17,16,9,8]=5'b00000
           PHY_SetRFReg_88XX_AC(Adapter,RF88XX_PATH_D,REG_RF_CHNLBW_AC,0x70300,0x0); //RF_D reg18[18,17,16,9,8]=5'b00000
       }
#endif
	} else if (36 <= channel && channel <= 64) {
		PHY_SetRFReg_88XX_AC(Adapter, RF88XX_PATH_A, REG_RF_CHNLBW_AC, 0x70300, 0x101); //RF_A reg18[18,17,16,9,8]=4'b00101
		PHY_SetRFReg_88XX_AC(Adapter, RF88XX_PATH_B, REG_RF_CHNLBW_AC, 0x70300, 0x101); //RF_B reg18[18,17,16,9,8]=4'b00101
#if IS_RTL8814A_SERIES
       if (GET_CHIP_VER(Adapter) == VERSION_8814A) {
           PHY_SetRFReg_88XX_AC(Adapter,RF88XX_PATH_C,REG_RF_CHNLBW_AC,0x70300,0x101); //RF_A reg18[18,17,16,9,8]=5'b00101
           PHY_SetRFReg_88XX_AC(Adapter,RF88XX_PATH_D,REG_RF_CHNLBW_AC,0x70300,0x101); //RF_B reg18[18,17,16,9,8]=5'b00101 
       }
#endif
	} else if (100 <= channel && channel <= 140) {
		PHY_SetRFReg_88XX_AC(Adapter, RF88XX_PATH_A, REG_RF_CHNLBW_AC, 0x70300, 0x301); //RF_A reg18[18,17,16,9,8]=4'b01101
		PHY_SetRFReg_88XX_AC(Adapter, RF88XX_PATH_B, REG_RF_CHNLBW_AC, 0x70300, 0x301); //RF_B reg18[18,17,16,9,8]=4'b01101
#if IS_RTL8814A_SERIES
       if (GET_CHIP_VER(Adapter) == VERSION_8814A) {
           PHY_SetRFReg_88XX_AC(Adapter,RF88XX_PATH_C,REG_RF_CHNLBW_AC,0x70300,0x301); //RF_A reg18[18,17,16,9,8]=5'b01101
           PHY_SetRFReg_88XX_AC(Adapter,RF88XX_PATH_D,REG_RF_CHNLBW_AC,0x70300,0x301); //RF_B reg18[18,17,16,9,8]=5'b01101
       }
#endif
	} else if (140 < channel) {
		PHY_SetRFReg_88XX_AC(Adapter, RF88XX_PATH_A, REG_RF_CHNLBW_AC, 0x70300, 0x501); //RF_A reg18[18,17,16,9,8]=4'b10101
		PHY_SetRFReg_88XX_AC(Adapter, RF88XX_PATH_B, REG_RF_CHNLBW_AC, 0x70300, 0x501); //RF_B reg18[18,17,16,9,8]=4'b10101
#if IS_RTL8814A_SERIES
       if (GET_CHIP_VER(Adapter) == VERSION_8814A) {
           PHY_SetRFReg_88XX_AC(Adapter,RF88XX_PATH_C,REG_RF_CHNLBW_AC,0x70300,0x501); //RF_A reg18[18,17,16,9,8]=5'b10101
           PHY_SetRFReg_88XX_AC(Adapter,RF88XX_PATH_D,REG_RF_CHNLBW_AC,0x70300,0x501); //RF_B reg18[18,17,16,9,8]=5'b10101
       }
#endif
	}

	// set channel number
	curMaxRFPath = RF88XX_PATH_MAX;

	if (channel > 14)
		HAL_VAR_CURR_BAND = RF88XX_BAND_ON_5G;
	else
		HAL_VAR_CURR_BAND = RF88XX_BAND_ON_2_4G;

	if (HAL_VAR_CURRENTCHANNELBW == RF88XX_HT_CHANNEL_WIDTH_80) {
		if (channel <= 48)
			val = 42;
		else if (channel <= 64)
			val = 58;
		else if (channel <= 112)
			val = 106;
		else if (channel <= 128)
			val = 122;
		else if (channel <= 144)
			val = 138;
		else if (channel <= 161)
			val = 155;
		else if (channel <= 177)
			val = 171;
	} else if (HAL_VAR_CURRENTCHANNELBW == RF88XX_HT_CHANNEL_WIDTH_20_40) {
		if (offset == 1)
			val -= 2;
		else
			val += 2;
	}

	if (HAL_VAR_USE_FRQ_2_3G) {
		val += 14;
	}

	for (eRFPath = RF88XX_PATH_A; eRFPath < curMaxRFPath; eRFPath++)	{
		PHY_SetRFReg_88XX_AC(Adapter, eRFPath, REG_RF_CHNLBW_AC, 0xff, val);
	}
}


void
CheckBand88XX_AC(
	IN  HAL_PADAPTER    Adapter,
	IN  u4Byte          RequestChannel
)
{

	u1Byte Current_band;
	//check CCK_CHECK_en BIT7
	if ( HAL_RTL_R8(REG_CCK_CHECK) & BIT(7) )
		Current_band = RF88XX_BAND_ON_5G;
	else
		Current_band = RF88XX_BAND_ON_2_4G;

	if (HAL_VAR_pre_channel == 0) { // fisrt lauch, no need to check band
		if (RequestChannel <= CHANNEL_MAX_NUMBER_2G) {
			SwitchWirelessBand88XX_AC(Adapter, RequestChannel, RF88XX_BAND_ON_2_4G);
		} else {
			SwitchWirelessBand88XX_AC(Adapter, RequestChannel, RF88XX_BAND_ON_5G);
		}
	} else {
		if (RequestChannel <= CHANNEL_MAX_NUMBER_2G) {
			// Request for 2.4G Band
			if (Current_band == RF88XX_BAND_ON_2_4G) {
				// In 2.4G ,Donothing
			} else if (Current_band == RF88XX_BAND_ON_5G) {
				// In 5G ,switch band to 2.4G
				SwitchWirelessBand88XX_AC(Adapter, RequestChannel, RF88XX_BAND_ON_2_4G);
			}
		} else if (RequestChannel > CHANNEL_MAX_NUMBER_2G) {
			// Request for 5G Band
			if (Current_band == RF88XX_BAND_ON_2_4G) {
				// In 2.4G ,switch band to 5G
				SwitchWirelessBand88XX_AC(Adapter, RequestChannel, RF88XX_BAND_ON_5G);
			} else if (Current_band == RF88XX_BAND_ON_5G) {
				// In 5G ,Donothing
			}
		}
	}
}

#if IS_RTL8814A_SERIES
void
phy_ADC_CLK_8814A(
	IN  HAL_PADAPTER    Adapter
	)
{
	printk("[%s]\n",__FUNCTION__);

	u4Byte     MAC_REG_520, BB_REG_8FC, BB_REG_808, RXIQC[4], Search_index = 0, MAC_Active = 1;
	u4Byte     RXIQC_REG[2][4] = {{0xc10, 0xe10, 0x1810, 0x1a10}, {0xc14, 0xe14, 0x1814, 0x1a14}} ;

	//1 Step1. MAC TX pause

	MAC_REG_520 = PHY_QueryBBReg( Adapter, 0x520, BIT_MASK_SET_MASKDWORD_COMMON);
	BB_REG_8FC = PHY_QueryBBReg( Adapter, 0x8fc, BIT_MASK_SET_MASKDWORD_COMMON);
	BB_REG_808 = PHY_QueryBBReg( Adapter, 0x808, BIT_MASK_SET_MASKDWORD_COMMON);
	PHY_SetBBReg(Adapter, 0x520, BIT_MASK_SET_MASKBYTE2_COMMON, 0x3f);

	//1 Step 2. Backup RXIQC & RXIQC = 0
	
	for(Search_index = 0; Search_index<4; Search_index++){
	    RXIQC[Search_index] = PHY_QueryBBReg( Adapter, RXIQC_REG[0][Search_index], BIT_MASK_SET_MASKDWORD_COMMON);
        PHY_SetBBReg(Adapter, RXIQC_REG[0][Search_index], BIT_MASK_SET_MASKDWORD_COMMON, 0x0);
        PHY_SetBBReg(Adapter, RXIQC_REG[1][Search_index], BIT_MASK_SET_MASKDWORD_COMMON, 0x0);
	}
	PHY_SetBBReg(Adapter, 0xa14, 0x00000300, 0x3);
	Search_index = 0;


	//1 Step 3. Monitor MAC IDLE
	PHY_SetBBReg(Adapter, 0x8fc, BIT_MASK_SET_MASKDWORD_COMMON, 0x0);
	while(MAC_Active){
		MAC_Active = PHY_QueryBBReg( Adapter, 0xfa0, BIT_MASK_SET_MASKDWORD_COMMON) & (0x803e0008);
		Search_index++;
		if(Search_index>1000){
			break;
		}
	}
	//1 Step 4. ADC clk flow
	
	PHY_SetBBReg(Adapter, 0x808, BIT_MASK_SET_MASKBYTE0_COMMON, 0x11);
	PHY_SetBBReg(Adapter, 0x90c, BIT(13), 0x1);
	PHY_SetBBReg(Adapter, 0x764, BIT(10)|BIT(9), 0x3);
	
	PHY_SetBBReg(Adapter, 0x804, BIT(2), 0x1);
	// 0xc1c/0xe1c/0x181c/0x1a1c[4] must=1 to ensure table can be written when bbrstb=0         
	// 0xc60/0xe60/0x1860/0x1a60[15] always = 1 after this line              
	// 0xc60/0xe60/0x1860/0x1a60[14] always = 0 bcz its error in A-cut          
	// power_off/clk_off @ anapar_state=idle mode
	PHY_SetBBReg(Adapter, 0xc60, BIT_MASK_SET_MASKDWORD_COMMON, 0x15800002);      //0xc60       0x15808002    
	PHY_SetBBReg(Adapter, 0xc60, BIT_MASK_SET_MASKDWORD_COMMON, 0x01808003);      //0xc60       0x01808003
	PHY_SetBBReg(Adapter, 0xe60, BIT_MASK_SET_MASKDWORD_COMMON, 0x15800002);      //0xe60      0x15808002    
	PHY_SetBBReg(Adapter, 0xe60, BIT_MASK_SET_MASKDWORD_COMMON, 0x01808003);      //0xe60      0x01808003    
	PHY_SetBBReg(Adapter, 0x1860, BIT_MASK_SET_MASKDWORD_COMMON, 0x15800002);    //0x1860    0x15808002    
	PHY_SetBBReg(Adapter, 0x1860, BIT_MASK_SET_MASKDWORD_COMMON, 0x01808003);    //0x1860    0x01808003    
	PHY_SetBBReg(Adapter, 0x1a60, BIT_MASK_SET_MASKDWORD_COMMON, 0x15800002);    //0x1a60    0x15808002        
	PHY_SetBBReg(Adapter, 0x1a60, BIT_MASK_SET_MASKDWORD_COMMON, 0x01808003);    //0x1a60    0x01808003    
	PHY_SetBBReg(Adapter, 0x764, BIT(10), 0x0);
	PHY_SetBBReg(Adapter, 0x804, BIT(2), 0x0);
	PHY_SetBBReg(Adapter, 0xc5c, BIT_MASK_SET_MASKDWORD_COMMON,  0x0D080058);     //0xc5c       0x00080058  // [19] =1 to turn off ADC  
	PHY_SetBBReg(Adapter, 0xe5c, BIT_MASK_SET_MASKDWORD_COMMON,  0x0D080058);    //0xe5c       0x00080058  // [19] =1 to turn off ADC     
	PHY_SetBBReg(Adapter, 0x185c, BIT_MASK_SET_MASKDWORD_COMMON,  0x0D080058);  //0x185c     0x00080058 // [19] =1 to turn off ADC    
	PHY_SetBBReg(Adapter, 0x1a5c, BIT_MASK_SET_MASKDWORD_COMMON,  0x0D080058);  //0x1a5c     0x00080058 // [19] =1 to turn off ADC       
	// power_on/clk_off 

	//PHY_SetBBReg(Adapter, 0x764, BIT(10), 0x1);

	PHY_SetBBReg(Adapter, 0xc5c, BIT_MASK_SET_MASKDWORD_COMMON,  0x0D000058);     //0xc5c       0x0D000058   // [19] =0 to turn on ADC    
	PHY_SetBBReg(Adapter, 0xe5c, BIT_MASK_SET_MASKDWORD_COMMON,  0x0D000058);    //0xe5c       0x0D000058  // [19] =0 to turn on ADC     
	PHY_SetBBReg(Adapter, 0x185c, BIT_MASK_SET_MASKDWORD_COMMON,  0x0D000058);  //0x185c     0x0D000058  // [19] =0 to turn on ADC     
	PHY_SetBBReg(Adapter, 0x1a5c, BIT_MASK_SET_MASKDWORD_COMMON,  0x0D000058);  //0x1a5c     0x0D000058 // [19] =0 to turn on ADC       

	// power_on/clk_on @ anapar_state=BT mode

	PHY_SetBBReg(Adapter, 0xc60, BIT_MASK_SET_MASKDWORD_COMMON, 0x05808032);      //0xc60 0x05808002 
	PHY_SetBBReg(Adapter, 0xe60, BIT_MASK_SET_MASKDWORD_COMMON, 0x05808032);      //0xe60 0x05808002           
	PHY_SetBBReg(Adapter, 0x1860, BIT_MASK_SET_MASKDWORD_COMMON, 0x05808032);    //0x1860 0x05808002    
	PHY_SetBBReg(Adapter, 0x1a60, BIT_MASK_SET_MASKDWORD_COMMON, 0x05808032);    //0x1a60 0x05808002            
	PHY_SetBBReg(Adapter, 0x764, BIT(10), 0x1);
	PHY_SetBBReg(Adapter, 0x804, BIT(2), 0x1);
	// recover original setting @ anapar_state=BT mode                              
	PHY_SetBBReg(Adapter, 0xc60, BIT_MASK_SET_MASKDWORD_COMMON, 0x05808032);      //0xc60       0x05808036    
	PHY_SetBBReg(Adapter, 0xe60, BIT_MASK_SET_MASKDWORD_COMMON, 0x05808032);      //0xe60      0x05808036    
	PHY_SetBBReg(Adapter, 0x1860, BIT_MASK_SET_MASKDWORD_COMMON, 0x05808032);    //0x1860    0x05808036    
	PHY_SetBBReg(Adapter, 0x1a60, BIT_MASK_SET_MASKDWORD_COMMON, 0x05808032);    //0x1a60    0x05808036        
	PHY_SetBBReg(Adapter, 0xc60, BIT_MASK_SET_MASKDWORD_COMMON, 0x05800002);      //0xc60       0x05800002    
	PHY_SetBBReg(Adapter, 0xc60, BIT_MASK_SET_MASKDWORD_COMMON, 0x07808003);      //0xc60       0x07808003
	PHY_SetBBReg(Adapter, 0xe60, BIT_MASK_SET_MASKDWORD_COMMON, 0x05800002);      //0xe60      0x05800002    
	PHY_SetBBReg(Adapter, 0xe60, BIT_MASK_SET_MASKDWORD_COMMON, 0x07808003);      //0xe60      0x07808003    
	PHY_SetBBReg(Adapter, 0x1860, BIT_MASK_SET_MASKDWORD_COMMON, 0x05800002);    //0x1860    0x05800002    
	PHY_SetBBReg(Adapter, 0x1860, BIT_MASK_SET_MASKDWORD_COMMON, 0x07808003);    //0x1860    0x07808003    
	PHY_SetBBReg(Adapter, 0x1a60, BIT_MASK_SET_MASKDWORD_COMMON, 0x05800002);    //0x1a60    0x05800002        
	PHY_SetBBReg(Adapter, 0x1a60, BIT_MASK_SET_MASKDWORD_COMMON, 0x07808003);    //0x1a60    0x07808003     
	PHY_SetBBReg(Adapter, 0x764, BIT(10)|BIT(9), 0x0);	
	PHY_SetBBReg(Adapter, 0x804, BIT(2), 0x0);
	PHY_SetBBReg(Adapter, 0x90c, BIT(13), 0x0);

	//1 Step 5. Recover MAC TX & IQC

	PHY_SetBBReg(Adapter, 0x520, BIT_MASK_SET_MASKDWORD_COMMON, MAC_REG_520);
	PHY_SetBBReg(Adapter, 0x8fc, BIT_MASK_SET_MASKDWORD_COMMON, BB_REG_8FC);
	PHY_SetBBReg(Adapter, 0x808, BIT_MASK_SET_MASKDWORD_COMMON, BB_REG_808);
	for(Search_index = 0; Search_index<4; Search_index++){
        PHY_SetBBReg(Adapter, RXIQC_REG[0][Search_index], BIT_MASK_SET_MASKDWORD_COMMON, RXIQC[Search_index]);
        PHY_SetBBReg(Adapter, RXIQC_REG[1][Search_index], BIT_MASK_SET_MASKDWORD_COMMON, 0x01000000);
	}
	PHY_SetBBReg(Adapter, 0xa14, 0x00000300, 0x0);
}
#endif

void
SwitchWirelessBand88XX_AC(
	IN  HAL_PADAPTER    Adapter,
	IN  u4Byte          RequestChannel,
	IN  u1Byte          Band
)
{
	if (Band == RF88XX_BAND_ON_2_4G) { // 2.4G band
		RT_TRACE(COMP_RF, DBG_LOUD, ("==>SwitchWirelessBand88XX_AC() BAND_ON_2_4G settings\n"));

		// STOP Tx/Rx
		PHY_SetBBReg_88XX(Adapter, REG_BB_CCKENABLE_AC, BIT_MASK_BB_BCCKEN_AC | BIT_MASK_BB_BOFDMEN_AC , 0x00);

		// PWED_TH
#if IS_RTL8881A_SERIES
		if (IS_HARDWARE_TYPE_8881A(Adapter)) {
			if ((GET_CHIP_VER(Adapter) == VERSION_8881A) && IS_HAL_TEST_CHIP(Adapter)) {
			} else {
				PHY_SetBBReg_88XX(Adapter, REG_BB_PWED_TH_AC, BIT3 | BIT2 | BIT1, 0x3);
			}
		}
#endif

		//PinMux
		if (GET_CHIP_VER(Adapter) == VERSION_8881A) {
			if (IS_HAL_TEST_CHIP(Adapter)) {
				PHY_SetBBReg_88XX(Adapter, REG_BB_A_RFE_PINMUX_AC, 0xff, 0x00);
				PHY_SetBBReg_88XX(Adapter, REG_BB_A_RFE_PINMUX_AC, 0xf000, 0x0);
			} else {
				PHY_SetBBReg_88XX(Adapter, REG_BB_A_RFE_PINMUX_AC, 0xf0, 0x7);
				PHY_SetBBReg_88XX(Adapter, REG_BB_A_RFE_PINMUX_AC, 0xf000, 0x7);
			}
		} else if (GET_CHIP_VER(Adapter) == VERSION_8812E) {
			if (IS_HAL_TEST_CHIP(Adapter)) {
				// r_select_5G for path_A/B
				PHY_SetBBReg_88XX(Adapter, REG_BB_A_RFE_AC, BIT12, 0x0);
				PHY_SetBBReg_88XX(Adapter, REG_BB_B_RFE_AC, BIT12, 0x0);

				// LANON (5G uses external LNA)
				PHY_SetBBReg_88XX(Adapter, REG_BB_A_RFE_AC, BIT15, 0x1);
				PHY_SetBBReg_88XX(Adapter, REG_BB_B_RFE_AC, BIT15, 0x1);

			} else {
				// TODO: Eric check PA type fill reg[REG_BB_RA_RFE_PINMUX]
				PHY_SetBBReg_88XX(Adapter, REG_BB_A_RFE_PINMUX_AC, BIT_MASK_SET_MASKBYTE2_COMMON, 0x77);
				PHY_SetBBReg_88XX(Adapter, REG_BB_B_RFE_PINMUX_AC, BIT_MASK_SET_MASKBYTE2_COMMON, 0x77);
			}
        } else if (GET_CHIP_VER(Adapter) == VERSION_8814A) {
            panic_printk("2G RFE TYPE\n");
#ifdef HIGH_POWER_EXT_PA
#ifdef HIGH_POWER_EXT_LNA
			//3 RFE Type 2
			if (Adapter->pshare->rf_ft_var.use_ext_pa && Adapter->pshare->rf_ft_var.use_ext_lna) {
				switch(Adapter->pmib->dot11RFEntry.rfe_type){
				case 3:
				case 5:
					panic_printk("TRSWITCH: RFE TYPE 3 or 5\n");
					PHY_SetBBReg_88XX(Adapter, 0x1994, 0xf, 0xf); // (ANTSW/ANTSWB controlled by S3)
					PHY_SetBBReg_88XX(Adapter, REG_GPIO_MUXCFG, 0x00f00000, 0xf); // (S3 RFE output enable) , 0x40[23:20]
					PHY_SetBBReg_88XX(Adapter, REG_BB_A_RFE_PINMUX_AC, BIT_MASK_SET_MASKDWORD_COMMON, 0x45704570);
					PHY_SetBBReg_88XX(Adapter, REG_BB_B_RFE_PINMUX_AC, BIT_MASK_SET_MASKDWORD_COMMON, 0x45704570);
					PHY_SetBBReg_88XX(Adapter, REG_BB_C_RFE_PINMUX_AC, BIT_MASK_SET_MASKDWORD_COMMON, 0x45704570);
					PHY_SetBBReg_88XX(Adapter, REG_BB_D_RFE_PINMUX_AC, BIT_MASK_SET_MASKDWORD_COMMON, 0x77707770);
					PHY_SetBBReg_88XX(Adapter, 0x1ABC, 0x0FF00000, 0x45); // (D路, use ANTSW/ANTSWB pins as LNAON), 0x1ABC[27:20]
					break;
				case 2:
				case 4:
					panic_printk("TRSWITCH: RFE TYPE 2 or 4\n");
					PHY_SetBBReg_88XX(Adapter, 0x1994, 0xf, 0xf); // (ANTSW/ANTSWB controlled by S3)
					PHY_SetBBReg_88XX(Adapter, REG_GPIO_MUXCFG, 0x00f00000, 0xf); // (S3 RFE output enable) , 0x40[23:20]
					PHY_SetBBReg_88XX(Adapter, REG_BB_A_RFE_PINMUX_AC, BIT_MASK_SET_MASKDWORD_COMMON, 0x72707270);
					PHY_SetBBReg_88XX(Adapter, REG_BB_B_RFE_PINMUX_AC, BIT_MASK_SET_MASKDWORD_COMMON, 0x72707270);
					PHY_SetBBReg_88XX(Adapter, REG_BB_C_RFE_PINMUX_AC, BIT_MASK_SET_MASKDWORD_COMMON, 0x72707270);
					PHY_SetBBReg_88XX(Adapter, REG_BB_D_RFE_PINMUX_AC, BIT_MASK_SET_MASKDWORD_COMMON, 0x77707770);
					PHY_SetBBReg_88XX(Adapter, 0x1ABC, 0x0FF00000, 0x72); // (D路, use ANTSW/ANTSWB pins as LNAON), 0x1ABC[27:20]
					break;
				default:	
					GDEBUG("TRSWITCH: Unknown RFE type (%d)!\n", Adapter->pmib->dot11RFEntry.rfe_type);
					break;
						// TODO: high power setting
					
				}
			} else
#endif
#endif		
			//3 RFE Type 7 or 9
			{
				panic_printk("TRSWITCH: RFE TYPE 7 or 9\n");
	           
	            PHY_SetBBReg_88XX(Adapter, 0x1994, 0xf, 0xf); // (ANTSW/ANTSWB controlled by S3)
	            PHY_SetBBReg_88XX(Adapter, REG_GPIO_MUXCFG, 0xC00000, 0x2); // (S3 RFE output enable) , 0x40[23:22]

	            PHY_SetBBReg_88XX(Adapter, REG_BB_A_RFE_PINMUX_AC, BIT_MASK_SET_MASKDWORD_COMMON, 0x77777777);
	            PHY_SetBBReg_88XX(Adapter, REG_BB_B_RFE_PINMUX_AC, BIT_MASK_SET_MASKDWORD_COMMON, 0x77777777);
	            PHY_SetBBReg_88XX(Adapter, REG_BB_C_RFE_PINMUX_AC, BIT_MASK_SET_MASKDWORD_COMMON, 0x77777777);
	            PHY_SetBBReg_88XX(Adapter, REG_BB_D_RFE_PINMUX_AC, BIT_MASK_SET_MASKDWORD_COMMON, 0x77777777);
	            PHY_SetBBReg_88XX(Adapter, 0x1abc, 0xFF00000, 0x77);  //  0x1ABC[27:20]
            }
		}
		// cck_enable
		PHY_SetBBReg_88XX(Adapter, REG_BB_CCKENABLE_AC, BIT_MASK_BB_BCCKEN_AC | BIT_MASK_BB_BOFDMEN_AC, 0x3);

		if (GET_CHIP_VER(Adapter) != VERSION_8814A) // 8814 RX CCK is from PHY parameters and no 2G/5G selectable
		PHY_SetBBReg_88XX(Adapter, REG_BB_CCK_RX_AC, 0x0f000000, 0x1);

		// CCK_CHECK_en
		HAL_RTL_W8(REG_CCK_CHECK, (HAL_RTL_R8(REG_CCK_CHECK) & ~(BIT(7)))); // 5G to 2.4G last lauch
		
	} else if (Band == RF88XX_BAND_ON_5G) { //5G band
		RT_TRACE(COMP_RF, DBG_LOUD, ("==>PHY_SwitchWirelessBand8812() BAND_ON_5G settings\n"));

		// CCK_CHECK_en
		HAL_RTL_W8(REG_CCK_CHECK, (HAL_RTL_R8(REG_CCK_CHECK) | BIT(7))); // 2.4G to 5G first lauch

		// STOP CCK,OFDM
		PHY_SetBBReg_88XX(Adapter, REG_BB_CCKENABLE_AC, BIT_MASK_BB_BCCKEN_AC | BIT_MASK_BB_BOFDMEN_AC , 0x00);

		// PWED_TH
#if IS_RTL8881A_SERIES
		if (IS_HARDWARE_TYPE_8881A(Adapter)) {
			if ((GET_CHIP_VER(Adapter) == VERSION_8881A) && IS_HAL_TEST_CHIP(Adapter)) {
			} else {
				PHY_SetBBReg_88XX(Adapter, REG_BB_PWED_TH_AC, BIT3 | BIT2 | BIT1, 0x2);
			}
		}
#endif

		//PinMux
		if (GET_CHIP_VER(Adapter) == VERSION_8881A) {
#if 1
#ifdef CONFIG_WLAN_HAL_8881A
			if ( (HAL_VAR_INTERLPA_8881A == 1 && (get_bonding_type_8881A() != BOND_8881AB))
#ifdef CONFIG_8881A_HP				
				|| HAL_VAR_HP_8881A == 1
#endif
				) {
				PHY_SetBBReg_88XX(Adapter, REG_BB_A_RFE_PINMUX_AC, 0xff, 0x54);
				panic_printk("8881A internal PA PIN control or 8881A high power\n");
			} else 
#endif
			{
				PHY_SetBBReg_88XX(Adapter, REG_BB_A_RFE_PINMUX_AC, 0xff, 0x55);
				panic_printk("8881A external PA PIN control :87503 or RTC 5634\n");
			}

			PHY_SetBBReg_88XX(Adapter, REG_BB_A_RFE_PINMUX_AC, 0xf000, 0x1);
#else
			PHY_SetBBReg_88XX(Adapter, REG_BB_A_RFE_PINMUX_AC, 0xf0, 0x4);
			PHY_SetBBReg_88XX(Adapter, REG_BB_A_RFE_PINMUX_AC, 0xf000, 0x5);
#endif
		} else if (GET_CHIP_VER(Adapter) == VERSION_8812E) {
			if (IS_HAL_TEST_CHIP(Adapter)) {
				// r_select_5G for path_A/B
				PHY_SetBBReg_88XX(Adapter, REG_BB_A_RFE_AC, BIT12, 0x1);
				PHY_SetBBReg_88XX(Adapter, REG_BB_B_RFE_AC, BIT12, 0x1);

				// LANON (5G uses external LNA)
				PHY_SetBBReg_88XX(Adapter, REG_BB_A_RFE_AC, BIT15, 0x0);
				PHY_SetBBReg_88XX(Adapter, REG_BB_B_RFE_AC, BIT15, 0x0);
			} else {
				// TODO: Eric check PA type fill reg[REG_BB_RA_RFE_PINMUX]
				if (HAL_VAR_PA_TYPE == PA_RFMD_4501 || HAL_VAR_PA_TYPE == PA_SKYWORKS_85712_HP) {
					PHY_SetBBReg_88XX(Adapter, REG_BB_A_RFE_PINMUX_AC, BIT_MASK_SET_MASKBYTE2_COMMON, 0x53);
					PHY_SetBBReg_88XX(Adapter, REG_BB_B_RFE_PINMUX_AC, BIT_MASK_SET_MASKBYTE2_COMMON, 0x53);
				} else {
					PHY_SetBBReg_88XX(Adapter, REG_BB_A_RFE_PINMUX_AC, BIT_MASK_SET_MASKBYTE2_COMMON, 0x54);
					PHY_SetBBReg_88XX(Adapter, REG_BB_B_RFE_PINMUX_AC, BIT_MASK_SET_MASKBYTE2_COMMON, 0x54);
				}
			}
        }  else if (GET_CHIP_VER(Adapter) == VERSION_8814A) {
			 panic_printk("5G RFE TYPE\n");
#ifdef HIGH_POWER_EXT_PA
#ifdef HIGH_POWER_EXT_LNA
			//3 RFE Type
			if (Adapter->pshare->rf_ft_var.use_ext_pa && Adapter->pshare->rf_ft_var.use_ext_lna) {
				switch(Adapter->pmib->dot11RFEntry.rfe_type){
				case 3:
				case 5:
					panic_printk("TRSWITCH: RFE TYPE 3 or 5\n");
					PHY_SetBBReg_88XX(Adapter, 0x1994, 0xf, 0xf); // (ANTSW/ANTSWB controlled by S3)
					PHY_SetBBReg_88XX(Adapter, REG_GPIO_MUXCFG, 0x00f00000, 0xf); // (S3 RFE output enable) , 0x40[23:20]
					PHY_SetBBReg_88XX(Adapter, REG_BB_A_RFE_PINMUX_AC, BIT_MASK_SET_MASKDWORD_COMMON, 0x54175417);
					PHY_SetBBReg_88XX(Adapter, REG_BB_B_RFE_PINMUX_AC, BIT_MASK_SET_MASKDWORD_COMMON, 0x54175417);
					PHY_SetBBReg_88XX(Adapter, REG_BB_C_RFE_PINMUX_AC, BIT_MASK_SET_MASKDWORD_COMMON, 0x54175417);
					PHY_SetBBReg_88XX(Adapter, REG_BB_D_RFE_PINMUX_AC, BIT_MASK_SET_MASKDWORD_COMMON, 0x77177717);
					PHY_SetBBReg_88XX(Adapter, 0x1ABC, 0x0FF00000, 0x54); // (D路, use ANTSW/ANTSWB pins as LNAON), 0x1ABC[27:20]
					break;
				case 2:				
				case 4:
				case 8:	
					panic_printk("TRSWITCH: RFE TYPE 2 or 4 or 8\n");
					PHY_SetBBReg_88XX(Adapter, 0x1994, 0xf, 0xf); // (ANTSW/ANTSWB controlled by S3)
					PHY_SetBBReg_88XX(Adapter, REG_GPIO_MUXCFG, 0x00f00000, 0xf); // (S3 RFE output enable) , 0x40[23:20]
					PHY_SetBBReg_88XX(Adapter, REG_BB_A_RFE_PINMUX_AC, BIT_MASK_SET_MASKDWORD_COMMON, 0x37173717);
					PHY_SetBBReg_88XX(Adapter, REG_BB_B_RFE_PINMUX_AC, BIT_MASK_SET_MASKDWORD_COMMON, 0x37173717);
					PHY_SetBBReg_88XX(Adapter, REG_BB_C_RFE_PINMUX_AC, BIT_MASK_SET_MASKDWORD_COMMON, 0x37173717);
					PHY_SetBBReg_88XX(Adapter, REG_BB_D_RFE_PINMUX_AC, BIT_MASK_SET_MASKDWORD_COMMON, 0x77177717);
					PHY_SetBBReg_88XX(Adapter, 0x1ABC, 0x0FF00000, 0x37); // (D路, use ANTSW/ANTSWB pins as LNAON), 0x1ABC[27:20]
					break;
				default:	
					GDEBUG("TRSWITCH: Unknown RFE type (%d)!\n", Adapter->pmib->dot11RFEntry.rfe_type);
					break;	
				}
				  // TODO: high power setting
			} else
#endif
#endif
			//3 RFE Type 7 or 9
			{
				panic_printk("TRSWITCH: RFE TYPE 7 or 9\n");
				PHY_SetBBReg_88XX(Adapter, 0x1994, 0xf, 0xf); // (ANTSW/ANTSWB controlled by S3)
				PHY_SetBBReg_88XX(Adapter, REG_GPIO_MUXCFG, 0x00f00000, 0xf); // (S3 RFE output enable) , 0x40[23:20]
	            // RFE type 0
	            PHY_SetBBReg_88XX(Adapter, REG_BB_A_RFE_PINMUX_AC, BIT_MASK_SET_MASKDWORD_COMMON, 0x54775477);
	            PHY_SetBBReg_88XX(Adapter, REG_BB_B_RFE_PINMUX_AC, BIT_MASK_SET_MASKDWORD_COMMON, 0x54775477);
	            PHY_SetBBReg_88XX(Adapter, REG_BB_C_RFE_PINMUX_AC, BIT_MASK_SET_MASKDWORD_COMMON, 0x54775477);
	            PHY_SetBBReg_88XX(Adapter, REG_BB_D_RFE_PINMUX_AC, BIT_MASK_SET_MASKDWORD_COMMON, 0x54775477);
				PHY_SetBBReg_88XX(Adapter, 0x1abc, 0xFF00000, 0x54); // 0x1ABC[27:20]
	        }
			//else {
	        //    RT_TRACE(COMP_RF, DBG_WARNING,("Error: No set BB register\n"));
	        //}       

			PHY_SetBBReg_88XX(Adapter, REG_BB_CCKENABLE_AC, BIT_MASK_BB_BCCKEN_AC|BIT_MASK_BB_BOFDMEN_AC , 0x2);
	        PHY_SetBBReg_88XX(Adapter, REG_BB_FC_ENABL_VHT_AC, BIT17 ,0x0);
	   }
   	}
}



void UpdateBBRFVal88XX_AC(
	IN  HAL_PADAPTER    Adapter,
	IN  u1Byte          channel,
	IN  s4Byte          offset
)
{

#if (RTL8822B_SUPPORT == 1)  
	if(IS_HARDWARE_TYPE_8822BE(Adapter)) {

		int val = channel;
	 
		if (channel > 14)
			HAL_VAR_CURR_BAND = RF88XX_BAND_ON_5G;
		else
			HAL_VAR_CURR_BAND = RF88XX_BAND_ON_2_4G;
		if (!Adapter->pshare->rf_ft_var.mp_specific) 
		{
		if (HAL_VAR_CURRENTCHANNELBW == RF88XX_HT_CHANNEL_WIDTH_80) {
			if (channel <= 48)
				val = 42;
			else if (channel <= 64)
				val = 58;
			else if (channel <= 112)
				val = 106;
			else if (channel <= 128)
				val = 122;
			else if (channel <= 144)
				val = 138;
			else if (channel <= 161)
				val = 155;
			else if (channel <= 177)
				val = 171;
		} else if (HAL_VAR_CURRENTCHANNELBW == RF88XX_HT_CHANNEL_WIDTH_20_40) {
			if (offset == 1)
				val -= 2;
			else
				val += 2;
		}

		if (HAL_VAR_USE_FRQ_2_3G) {
			val += 14;
		}		
		}
		//eric-8822 ?? what shall MAC do during swchnl??
		config_phydm_switch_band_8822b((&(Adapter->pshare->_dmODM)), val);
		config_phydm_switch_channel_8822b((&(Adapter->pshare->_dmODM)), val);
		return;
	}
#endif

	CheckBand88XX_AC(Adapter, channel);

	SetChannelPara88XX_AC(Adapter, channel, offset);

	HAL_VAR_pre_channel = channel;

}

#if (IS_RTL8814A_SERIES || IS_RTL8822B_SERIES)
void
SetCurrentTxAGC(
	IN	HAL_PADAPTER	Adapter,
	IN  u1Byte 		 txPathIdx,	
	IN  u1Byte 		 txRate,
	IN  u1Byte 		 power
)
{
	if(txRate <= TX_AGC_CCK_11M){
		Adapter->pshare->phw->CurrentTxAgcCCK[txPathIdx][txRate] = power;
	}else if(txRate <= TX_AGC_OFDM_54M){
		Adapter->pshare->phw->CurrentTxAgcOFDM[txPathIdx][txRate - TX_AGC_OFDM_6M] = power;	
	}else if(txRate <= TX_AGC_HT_NSS4_MCS31 ){
		Adapter->pshare->phw->CurrentTxAgcMCS[txPathIdx][txRate - TX_AGC_HT_NSS1_MCS0] = power;	
	}else if(txRate <= TX_AGC_VHT_NSS4_MCS9){
		Adapter->pshare->phw->CurrentTxAgcVHT[txPathIdx][txRate - TX_AGC_VHT_NSS1_MCS0] = power;	
	}
}


void
PHYSetTxPower88XX(
    IN  HAL_PADAPTER    Adapter, 
    IN  u1Byte          txRateStart,
    IN  u1Byte          txRateEnd,
    IN  u1Byte          txPathIdx,
    IN  u1Byte          defPower
)
{
    u1Byte i            = 0;
    u1Byte tx_path_idx  = txPathIdx;  // select path a/b/c/d
    u1Byte tx_none      = 0x1;
    u1Byte tx_write     = 0x1;
    u1Byte def_power    = defPower;
    u4Byte value32      = 0;

#if 0
	printk("[%s] Path %u, rate from 0x%x to 0x%x: 0x%x\n", __FUNCTION__, tx_path_idx+1, txRateStart, txRateEnd, def_power);
#endif


#if (RTL8822B_SUPPORT == 1)  
	if(IS_HARDWARE_TYPE_8822BE(Adapter)) {
		for (i = txRateStart; i <= txRateEnd; i++) {
			phydm_write_txagc_1byte_8822b((&(Adapter->pshare->_dmODM)), defPower, txPathIdx, i);
			SetCurrentTxAGC(Adapter,tx_path_idx,i,def_power);
		}
		return;
	}
#endif

    // before write data, we must add extra one write commnad to "turn on" the AGC table
    // TODO: this can be moved to other place ?? ex: after init done ?
    HAL_RTL_W32(REG_BB_TXAGC_V2_BASE_ADDR, FILL_REG_BB_TXAGC_V2(txRateStart, tx_path_idx, tx_none, tx_write, def_power));

    for (i = txRateStart; i <= txRateEnd; i++) {
        HAL_RTL_W32(REG_BB_TXAGC_V2_BASE_ADDR, FILL_REG_BB_TXAGC_V2(i, tx_path_idx, tx_none, tx_write, def_power));
		SetCurrentTxAGC(Adapter,tx_path_idx,i,def_power); /* Path, Rate, Power */
    }
}
#endif


RT_STATUS
PHYSetCCKTxPower88XX_AC(
	IN  HAL_PADAPTER    Adapter,
	IN  u1Byte          channel
)
{
	u4Byte def_power    = 0x12121212;//0x20202020;
	u4Byte writeVal     = 0;
	u1Byte ch_idx       = 0;
	u1Byte tmp_TPI      = 0;
	u1Byte phy_band     = 0;
    u1Byte bPathXEn[4]   = {0};
    u1Byte i;

    u1Byte def_power_value = 0x12;

    u4Byte REG_BB_TXAGC_X_CCK11_CCK1_AC[4] = {REG_BB_TXAGC_A_CCK11_CCK1_AC, REG_BB_TXAGC_B_CCK11_CCK1_AC,
		REG_BB_TXAGC_C_CCK11_CCK1_AC, REG_BB_TXAGC_D_CCK11_CCK1_AC
		};
    u1Byte HAL_VAR_pwrlevelCCK_X[4] = {HAL_VAR_pwrlevelCCK_A(channel - 1), HAL_VAR_pwrlevelCCK_B(channel - 1), 			
		HAL_VAR_pwrlevelCCK_C(channel - 1), HAL_VAR_pwrlevelCCK_D(channel - 1)
		};

	if (channel > 0)
		ch_idx = (channel - 1);
	else {
		RT_TRACE_F(COMP_RF, DBG_WARNING, ("Error Channel [%x]!\n", channel));
		return RT_STATUS_FAILURE;
	}

	if (channel > CHANNEL_MAX_NUMBER_2G)
		phy_band = PHY_BAND_5G;
	else
		phy_band = PHY_BAND_2G;

#if 0
	printk("\n-----------------------   CCK Efuse Power  ------------------------\n");
	printk("[%s]HAL_VAR_pwrlevelCCK_A(%d)=%x\n",__FUNCTION__,ch_idx,HAL_VAR_pwrlevelCCK_A(ch_idx));
	printk("[%s]HAL_VAR_pwrlevelCCK_B(%d)=%x\n",__FUNCTION__,ch_idx,HAL_VAR_pwrlevelCCK_B(ch_idx)); 
	printk("[%s]HAL_VAR_pwrlevelCCK_C(%d)=%x\n",__FUNCTION__,ch_idx,HAL_VAR_pwrlevelCCK_C(ch_idx));
	printk("[%s]HAL_VAR_pwrlevelCCK_D(%d)=%x\n",__FUNCTION__,ch_idx,HAL_VAR_pwrlevelCCK_D(ch_idx)); 
#endif
    if (GetChipIDMIMO88XX(Adapter) == MIMO_1T1R) {
        bPathXEn[0] = 1;
        bPathXEn[1] = 0;
        bPathXEn[2] = 0;
        bPathXEn[3] = 0;
        if(HAL_VAR_pwrlevelCCK_A(ch_idx) == 0) {
            RT_TRACE_F(COMP_RF, DBG_WARNING,("1T1R NO Calibration data, use default CCK power = 0x%x\n", (unsigned int)def_power));

#if IS_RTL8814A_SERIES
            if ( IS_HARDWARE_TYPE_8814A(Adapter) && (IS_HAL_TEST_CHIP(Adapter)==FALSE)) {
                PHYSetTxPower88XX(Adapter, TX_AGC_CCK_1M, TX_AGC_CCK_11M, 0, def_power_value);
            } else 
#endif
            {
                HAL_RTL_W32(REG_BB_TXAGC_X_CCK11_CCK1_AC[0], def_power);
            }
           	return RT_STATUS_SUCCESS;
        }
    }
    else if (GetChipIDMIMO88XX(Adapter) == MIMO_2T2R) {
        bPathXEn[0] = 1;
        bPathXEn[1] = 1;
        bPathXEn[2] = 0;
        bPathXEn[3] = 0;
        if ((HAL_VAR_pwrlevelCCK_A(ch_idx) == 0) || (HAL_VAR_pwrlevelCCK_B(ch_idx) == 0)) {
		    RT_TRACE_F(COMP_RF, DBG_WARNING,("2T2R NO Calibration data, use default CCK power = 0x%x\n", (unsigned int)def_power));

#if IS_RTL8814A_SERIES            
            if ( IS_HARDWARE_TYPE_8814A(Adapter) && (IS_HAL_TEST_CHIP(Adapter)==FALSE)) {
                PHYSetTxPower88XX(Adapter, TX_AGC_CCK_1M, TX_AGC_CCK_11M, 0, def_power_value);
                PHYSetTxPower88XX(Adapter, TX_AGC_CCK_1M, TX_AGC_CCK_11M, 1, def_power_value);
            } else 
#endif
            {
                HAL_RTL_W32(REG_BB_TXAGC_X_CCK11_CCK1_AC[0], def_power);
                HAL_RTL_W32(REG_BB_TXAGC_X_CCK11_CCK1_AC[1], def_power);
            }
            return RT_STATUS_SUCCESS;
        }
	}
    else if (GetChipIDMIMO88XX(Adapter) == MIMO_3T3R) {
			if(IS_HARDWARE_TYPE_8814A(Adapter)){
#if defined(CONFIG_SLOT_0_8194_2T2R_SUPPORT) || defined(CONFIG_SLOT_1_8194_2T2R_SUPPORT)
				bPathXEn[0] = 0;
				bPathXEn[1] = 1;
				bPathXEn[2] = 1;
				bPathXEn[3] = 0;
				if ((HAL_VAR_pwrlevelCCK_B(ch_idx)== 0) || (HAL_VAR_pwrlevelCCK_C(ch_idx) == 0)){
				    RT_TRACE_F(COMP_RF, DBG_WARNING,("3T3R NO Calibration data, use default CCK power = 0x%x\n", (unsigned int)def_power));
#if IS_RTL8814A_SERIES
		            if ( IS_HARDWARE_TYPE_8814A(Adapter) && (IS_HAL_TEST_CHIP(Adapter)==FALSE)) {
		                PHYSetTxPower88XX(Adapter, TX_AGC_CCK_1M, TX_AGC_CCK_11M, 1, def_power_value);
		                PHYSetTxPower88XX(Adapter, TX_AGC_CCK_1M, TX_AGC_CCK_11M, 2, def_power_value);
		            } else 
#endif
		            {
		        		HAL_RTL_W32(REG_BB_TXAGC_X_CCK11_CCK1_AC[1], def_power);
		                HAL_RTL_W32(REG_BB_TXAGC_X_CCK11_CCK1_AC[2], def_power);
		            }
		            return RT_STATUS_SUCCESS;
				}
#else
				bPathXEn[0] = 1;
				bPathXEn[1] = 1;
				bPathXEn[2] = 1;
				bPathXEn[3] = 1;
				if ((HAL_VAR_pwrlevelCCK_B(ch_idx)== 0) || (HAL_VAR_pwrlevelCCK_C(ch_idx) == 0)
					||(HAL_VAR_pwrlevelCCK_D(ch_idx)== 0)
				){
				    RT_TRACE_F(COMP_RF, DBG_WARNING,("3T3R NO Calibration data, use default CCK power = 0x%x\n", (unsigned int)def_power));
#if IS_RTL8814A_SERIES
		            if ( IS_HARDWARE_TYPE_8814A(Adapter) && (IS_HAL_TEST_CHIP(Adapter)==FALSE)) {
		                PHYSetTxPower88XX(Adapter, TX_AGC_CCK_1M, TX_AGC_CCK_11M, 0, def_power_value);
		                PHYSetTxPower88XX(Adapter, TX_AGC_CCK_1M, TX_AGC_CCK_11M, 1, def_power_value);
		                PHYSetTxPower88XX(Adapter, TX_AGC_CCK_1M, TX_AGC_CCK_11M, 2, def_power_value);
		                PHYSetTxPower88XX(Adapter, TX_AGC_CCK_1M, TX_AGC_CCK_11M, 3, def_power_value);
		            } else 
#endif
		            {
		                HAL_RTL_W32(REG_BB_TXAGC_X_CCK11_CCK1_AC[0], def_power);
		                HAL_RTL_W32(REG_BB_TXAGC_X_CCK11_CCK1_AC[1], def_power);
		                HAL_RTL_W32(REG_BB_TXAGC_X_CCK11_CCK1_AC[2], def_power);
		        		HAL_RTL_W32(REG_BB_TXAGC_X_CCK11_CCK1_AC[3], def_power);
		            }
		            return RT_STATUS_SUCCESS;
				}
#endif
		}else{
	        bPathXEn[0] = 1;
	        bPathXEn[1] = 1;
	        bPathXEn[2] = 1;
			bPathXEn[3] = 0;
	        if ((HAL_VAR_pwrlevelCCK_A(ch_idx) == 0)||(HAL_VAR_pwrlevelCCK_B(ch_idx) == 0)
				||(HAL_VAR_pwrlevelCCK_C(ch_idx) == 0)	
				) {
			    RT_TRACE_F(COMP_RF, DBG_WARNING,("3T3R NO Calibration data, use default CCK power = 0x%x\n", (unsigned int)def_power));
#if IS_RTL8814A_SERIES
	            if ( IS_HARDWARE_TYPE_8814A(Adapter) && (IS_HAL_TEST_CHIP(Adapter)==FALSE)) {
	                PHYSetTxPower88XX(Adapter, TX_AGC_CCK_1M, TX_AGC_CCK_11M, 0, def_power_value);
	                PHYSetTxPower88XX(Adapter, TX_AGC_CCK_1M, TX_AGC_CCK_11M, 1, def_power_value);
	                PHYSetTxPower88XX(Adapter, TX_AGC_CCK_1M, TX_AGC_CCK_11M, 2, def_power_value);
	            } else 
#endif
	            {
	        		HAL_RTL_W32(REG_BB_TXAGC_X_CCK11_CCK1_AC[0], def_power);
	        		HAL_RTL_W32(REG_BB_TXAGC_X_CCK11_CCK1_AC[1], def_power);
	                HAL_RTL_W32(REG_BB_TXAGC_X_CCK11_CCK1_AC[2], def_power);
	            }
	            return RT_STATUS_SUCCESS;
	        }
    	}
   	}
    else if (GetChipIDMIMO88XX(Adapter) == MIMO_4T4R) {
        bPathXEn[0] = 1;
        bPathXEn[1] = 1;
        bPathXEn[2] = 1;
        bPathXEn[3] = 1;
        if ((HAL_VAR_pwrlevelCCK_A(ch_idx) == 0)||(HAL_VAR_pwrlevelCCK_B(ch_idx) == 0)		
			||(HAL_VAR_pwrlevelCCK_C(ch_idx) == 0)||(HAL_VAR_pwrlevelCCK_D(ch_idx) == 0)
			) {
		    RT_TRACE_F(COMP_RF, DBG_WARNING,("4T4R NO Calibration data, use default CCK power = 0x%x\n", (unsigned int)def_power));

#if IS_RTL8814A_SERIES
            if ( IS_HARDWARE_TYPE_8814A(Adapter) && (IS_HAL_TEST_CHIP(Adapter)==FALSE)) {
                PHYSetTxPower88XX(Adapter, TX_AGC_CCK_1M, TX_AGC_CCK_11M, 0, def_power_value);
                PHYSetTxPower88XX(Adapter, TX_AGC_CCK_1M, TX_AGC_CCK_11M, 1, def_power_value);
                PHYSetTxPower88XX(Adapter, TX_AGC_CCK_1M, TX_AGC_CCK_11M, 2, def_power_value);
                PHYSetTxPower88XX(Adapter, TX_AGC_CCK_1M, TX_AGC_CCK_11M, 3, def_power_value);
            } else 
#endif
            {
        		HAL_RTL_W32(REG_BB_TXAGC_X_CCK11_CCK1_AC[0], def_power);
        		HAL_RTL_W32(REG_BB_TXAGC_X_CCK11_CCK1_AC[1], def_power);
                HAL_RTL_W32(REG_BB_TXAGC_X_CCK11_CCK1_AC[2], def_power);
                HAL_RTL_W32(REG_BB_TXAGC_X_CCK11_CCK1_AC[3], def_power);
            }
            return RT_STATUS_SUCCESS;
        }
    }


	if (phy_band == PHY_BAND_2G) {

        for(i = 0; i < 4; i++) {
            //PATH X
            if(bPathXEn[i]) {
#if IS_RTL8814A_SERIES
                if ( IS_HARDWARE_TYPE_8814A(Adapter) && (IS_HAL_TEST_CHIP(Adapter)==FALSE)) {
                    PHYSetTxPower88XX(Adapter, TX_AGC_CCK_1M, TX_AGC_CCK_11M, i, HAL_VAR_pwrlevelCCK_X[i]?HAL_VAR_pwrlevelCCK_X[i]:0x12);
                } else 
#endif
                {                
                    tmp_TPI = HAL_VAR_pwrlevelCCK_X[i];
                    writeVal = (tmp_TPI << 24) | (tmp_TPI << 16) | (tmp_TPI << 8) | tmp_TPI;
                    HAL_RTL_W32(REG_BB_TXAGC_X_CCK11_CCK1_AC[i], writeVal);
                }
            }
        }  
	}

#if IS_RTL8814A_SERIES	
		if(IS_HARDWARE_TYPE_8814A(Adapter)){
			TxPG_CCK_8814(Adapter);
		}else
#endif
			TxPG_CCK_Jaguar(Adapter, bPathXEn);
#if 0//def CONFIG_WLAN_HAL_8814AE
		printk("CCK After TxPG_CCK_8814\n");
		for(path=0;path<4;path++){
			printk("\n	Path%d\n	CCK:		",path);
			for(rate=TX_AGC_CCK_1M;rate<=TX_AGC_CCK_11M;rate++){
				printk("%x,",Adapter->pshare->phw->CurrentTxAgcCCK[path][rate]);
			}
		}
#endif //#ifdef CONFIG_WLAN_HAL_8814AE

#ifdef TXPWR_LMT	
	if (!Adapter->pshare->rf_ft_var.disable_txpwrlmt) {
		if (HAL_check_lmt_valid(Adapter, phy_band)) {
#if IS_RTL8814A_SERIES		
			if(IS_HARDWARE_TYPE_8814A(Adapter))
				TxLMT_CCK_Jaguar_AC_8814(Adapter);
			else
#endif						
				TxLMT_CCK_Jaguar_AC(Adapter, bPathXEn);
		}
	}
#endif


	return RT_STATUS_SUCCESS;
}

RT_STATUS
PHYSetOFDMTxPower88XX_AC(
	IN  HAL_PADAPTER    Adapter,
	IN  u1Byte          channel
)
{
	u1Byte ch_idx = 0, no_cali_data = 0;
	u1Byte phy_band = 0;

	if (channel > 0)
		ch_idx = (channel - 1);
	else {
		RT_TRACE_F(COMP_RF, DBG_WARNING, ("Error Channel !!\n"));
		return RT_STATUS_FAILURE;
	}

	if (channel > CHANNEL_MAX_NUMBER_2G)
		phy_band = PHY_BAND_5G;
	else
		phy_band = PHY_BAND_2G;
#if 0	
	printk("\n-----------------------	5G Efuse Power	 ------------------------\n");

	printk("HAL_VAR_pwrlevel5GHT40_1S_A(%d)=%x\n",ch_idx,HAL_VAR_pwrlevel5GHT40_1S_A(ch_idx));
	printk("HAL_VAR_pwrlevel5GHT40_1S_B(%d)=%x\n",ch_idx,HAL_VAR_pwrlevel5GHT40_1S_B(ch_idx));	
	printk("HAL_VAR_pwrlevel5GHT40_1S_C(%d)=%x\n",ch_idx,HAL_VAR_pwrlevel5GHT40_1S_C(ch_idx));
	printk("HAL_VAR_pwrlevel5GHT40_1S_D(%d)=%x\n",ch_idx,HAL_VAR_pwrlevel5GHT40_1S_D(ch_idx)); 
	
	printk("HAL_VAR_pwrdiff_5G_20BW1S_OFDM1T_A(%d)=%x\n",ch_idx,HAL_VAR_pwrdiff_5G_20BW1S_OFDM1T_A(ch_idx)); 
	printk("HAL_VAR_pwrdiff_5G_40BW2S_20BW2S_A(%d)=%x\n",ch_idx,HAL_VAR_pwrdiff_5G_40BW2S_20BW2S_A(ch_idx)); 
	printk("HAL_VAR_pwrdiff_5G_40BW3S_20BW3S_A(%d)=%x\n",ch_idx,HAL_VAR_pwrdiff_5G_40BW3S_20BW3S_A(ch_idx)); 
	printk("HAL_VAR_pwrdiff_5G_80BW1S_160BW1S_A(%d)=%x\n",ch_idx,HAL_VAR_pwrdiff_5G_80BW1S_160BW1S_A(ch_idx)); 
	printk("HAL_VAR_pwrdiff_5G_80BW2S_160BW2S_A(%d)=%x\n",ch_idx,HAL_VAR_pwrdiff_5G_80BW2S_160BW2S_A(ch_idx)); 
	printk("HAL_VAR_pwrdiff_5G_80BW3S_160BW3S_A(%d)=%x\n",ch_idx,HAL_VAR_pwrdiff_5G_80BW3S_160BW3S_A(ch_idx)); 

	printk("HAL_VAR_pwrdiff_5G_20BW1S_OFDM1T_B(%d)=%x\n",ch_idx,HAL_VAR_pwrdiff_5G_20BW1S_OFDM1T_B(ch_idx)); 
	printk("HAL_VAR_pwrdiff_5G_40BW2S_20BW2S_B(%d)=%x\n",ch_idx,HAL_VAR_pwrdiff_5G_40BW2S_20BW2S_B(ch_idx)); 
	printk("HAL_VAR_pwrdiff_5G_40BW3S_20BW3S_B(%d)=%x\n",ch_idx,HAL_VAR_pwrdiff_5G_40BW3S_20BW3S_B(ch_idx)); 
	printk("HAL_VAR_pwrdiff_5G_80BW1S_160BW1S_B(%d)=%x\n",ch_idx,HAL_VAR_pwrdiff_5G_80BW1S_160BW1S_B(ch_idx)); 
	printk("HAL_VAR_pwrdiff_5G_80BW2S_160BW2S_B(%d)=%x\n",ch_idx,HAL_VAR_pwrdiff_5G_80BW2S_160BW2S_B(ch_idx)); 
	printk("HAL_VAR_pwrdiff_5G_80BW3S_160BW3S_B(%d)=%x\n",ch_idx,HAL_VAR_pwrdiff_5G_80BW3S_160BW3S_B(ch_idx)); 

	printk("HAL_VAR_pwrdiff_5G_20BW1S_OFDM1T_C(%d)=%x\n",ch_idx,HAL_VAR_pwrdiff_5G_20BW1S_OFDM1T_C(ch_idx)); 
	printk("HAL_VAR_pwrdiff_5G_40BW2S_20BW2S_C(%d)=%x\n",ch_idx,HAL_VAR_pwrdiff_5G_40BW2S_20BW2S_C(ch_idx)); 
	printk("HAL_VAR_pwrdiff_5G_40BW3S_20BW3S_C(%d)=%x\n",ch_idx,HAL_VAR_pwrdiff_5G_40BW3S_20BW3S_C(ch_idx)); 
	printk("HAL_VAR_pwrdiff_5G_80BW1S_160BW1S_C(%d)=%x\n",ch_idx,HAL_VAR_pwrdiff_5G_80BW1S_160BW1S_C(ch_idx)); 
	printk("HAL_VAR_pwrdiff_5G_80BW2S_160BW2S_C(%d)=%x\n",ch_idx,HAL_VAR_pwrdiff_5G_80BW2S_160BW2S_C(ch_idx)); 
	printk("HAL_VAR_pwrdiff_5G_80BW3S_160BW3S_C(%d)=%x\n",ch_idx,HAL_VAR_pwrdiff_5G_80BW3S_160BW3S_C(ch_idx)); 

	printk("HAL_VAR_pwrdiff_5G_20BW1S_OFDM1T_D(%d)=%x\n",ch_idx,HAL_VAR_pwrdiff_5G_20BW1S_OFDM1T_D(ch_idx)); 
	printk("HAL_VAR_pwrdiff_5G_40BW2S_20BW2S_D(%d)=%x\n",ch_idx,HAL_VAR_pwrdiff_5G_40BW2S_20BW2S_D(ch_idx)); 
	printk("HAL_VAR_pwrdiff_5G_40BW3S_20BW3S_D(%d)=%x\n",ch_idx,HAL_VAR_pwrdiff_5G_40BW3S_20BW3S_D(ch_idx)); 
	printk("HAL_VAR_pwrdiff_5G_80BW1S_160BW1S_D(%d)=%x\n",ch_idx,HAL_VAR_pwrdiff_5G_80BW1S_160BW1S_D(ch_idx)); 
	printk("HAL_VAR_pwrdiff_5G_80BW2S_160BW2S_D(%d)=%x\n",ch_idx,HAL_VAR_pwrdiff_5G_80BW2S_160BW2S_D(ch_idx)); 
	printk("HAL_VAR_pwrdiff_5G_80BW3S_160BW3S_D(%d)=%x\n",ch_idx,HAL_VAR_pwrdiff_5G_80BW3S_160BW3S_D(ch_idx)); 


	printk("\n-----------------------   2G Efuse Power  ------------------------\n");
	
	printk("HAL_VAR_pwrlevelHT40_1S_A(%d)=%x\n",ch_idx,HAL_VAR_pwrlevelHT40_1S_A(ch_idx));
	printk("HAL_VAR_pwrlevelHT40_1S_B(%d)=%x\n",ch_idx,HAL_VAR_pwrlevelHT40_1S_B(ch_idx));	
	printk("HAL_VAR_pwrlevelHT40_1S_C(%d)=%x\n",ch_idx,HAL_VAR_pwrlevelHT40_1S_C(ch_idx));
	printk("HAL_VAR_pwrlevelHT40_1S_D(%d)=%x\n",ch_idx,HAL_VAR_pwrlevelHT40_1S_D(ch_idx));	

	printk("HAL_VAR_pwrdiff_20BW1S_OFDM1T_A(%d)=%x\n",ch_idx,HAL_VAR_pwrdiff_20BW1S_OFDM1T_A(ch_idx)); 
	printk("HAL_VAR_pwrdiff_40BW2S_20BW2S_A(%d)=%x\n",ch_idx,HAL_VAR_pwrdiff_40BW2S_20BW2S_A(ch_idx)); 
	printk("HAL_VAR_pwrdiff_40BW3S_20BW3S_A(%d)=%x\n",ch_idx,HAL_VAR_pwrdiff_40BW3S_20BW3S_A(ch_idx)); 
	printk("HAL_VAR_pwrdiff_20BW1S_OFDM1T_B(%d)=%x\n",ch_idx,HAL_VAR_pwrdiff_20BW1S_OFDM1T_B(ch_idx)); 
	printk("HAL_VAR_pwrdiff_40BW2S_20BW2S_B(%d)=%x\n",ch_idx,HAL_VAR_pwrdiff_40BW2S_20BW2S_B(ch_idx)); 
	printk("HAL_VAR_pwrdiff_40BW3S_20BW3S_B(%d)=%x\n",ch_idx,HAL_VAR_pwrdiff_40BW3S_20BW3S_B(ch_idx)); 
	printk("HAL_VAR_pwrdiff_20BW1S_OFDM1T_C(%d)=%x\n",ch_idx,HAL_VAR_pwrdiff_20BW1S_OFDM1T_C(ch_idx)); 
	printk("HAL_VAR_pwrdiff_40BW2S_20BW2S_C(%d)=%x\n",ch_idx,HAL_VAR_pwrdiff_40BW2S_20BW2S_C(ch_idx)); 
	printk("HAL_VAR_pwrdiff_40BW3S_20BW3S_C(%d)=%x\n",ch_idx,HAL_VAR_pwrdiff_40BW3S_20BW3S_C(ch_idx)); 
	printk("HAL_VAR_pwrdiff_20BW1S_OFDM1T_D(%d)=%x\n",ch_idx,HAL_VAR_pwrdiff_20BW1S_OFDM1T_D(ch_idx)); 
	printk("HAL_VAR_pwrdiff_40BW2S_20BW2S_D(%d)=%x\n",ch_idx,HAL_VAR_pwrdiff_40BW2S_20BW2S_D(ch_idx)); 
	printk("HAL_VAR_pwrdiff_40BW3S_20BW3S_D(%d)=%x\n\n",ch_idx,HAL_VAR_pwrdiff_40BW3S_20BW3S_D(ch_idx)); 
#endif		

	if (phy_band == PHY_BAND_5G) {
	    if (GetChipIDMIMO88XX(Adapter) == MIMO_4T4R) {
	        if ((HAL_VAR_pwrlevel5GHT40_1S_A(ch_idx)== 0)||(HAL_VAR_pwrlevel5GHT40_1S_B(ch_idx) == 0)
				||(HAL_VAR_pwrlevel5GHT40_1S_C(ch_idx)== 0)||(HAL_VAR_pwrlevel5GHT40_1S_D(ch_idx) == 0)		
				) {
				no_cali_data = 1;	
		    }            
	    } else if (GetChipIDMIMO88XX(Adapter) == MIMO_3T3R) {
			if(IS_HARDWARE_TYPE_8814A(Adapter)){
#if defined(CONFIG_SLOT_0_8814_2T2R_SUPPORT) || defined(CONFIG_SLOT_1_8814_2T2R_SUPPORT)
				if ((HAL_VAR_pwrlevel5GHT40_1S_B(ch_idx) == 0) || (HAL_VAR_pwrlevel5GHT40_1S_C(ch_idx) == 0)) {
					no_cali_data = 1;
				}
#else
				if ((HAL_VAR_pwrlevel5GHT40_1S_B(ch_idx) == 0) || (HAL_VAR_pwrlevel5GHT40_1S_C(ch_idx)== 0)
					||(HAL_VAR_pwrlevel5GHT40_1S_D(ch_idx) == 0)
					) {
					no_cali_data = 1;
			    }
#endif
			}else{
		        if ((HAL_VAR_pwrlevel5GHT40_1S_A(ch_idx)== 0)||(HAL_VAR_pwrlevel5GHT40_1S_B(ch_idx) == 0)			
					||(HAL_VAR_pwrlevel5GHT40_1S_C(ch_idx)== 0)
					) {
					no_cali_data = 1;
			    } 
			}
	    } else if (GetChipIDMIMO88XX(Adapter) == MIMO_2T2R) {

			if ((HAL_VAR_pwrlevel5GHT40_1S_A(ch_idx) == 0) || (HAL_VAR_pwrlevel5GHT40_1S_B(ch_idx) == 0)) {
				no_cali_data = 1;

			}
		} else if (GetChipIDMIMO88XX(Adapter) == MIMO_1T1R) {
			if ((HAL_VAR_pwrlevel5GHT40_1S_A(ch_idx) == 0)) {
				no_cali_data = 1;
			}
		}
	} else {
		if (GetChipIDMIMO88XX(Adapter) == MIMO_4T4R) {
	        if ((HAL_VAR_pwrlevelHT40_1S_A(ch_idx)== 0)||(HAL_VAR_pwrlevelHT40_1S_B(ch_idx) == 0)
				||(HAL_VAR_pwrlevelHT40_1S_C(ch_idx)== 0)||(HAL_VAR_pwrlevelHT40_1S_D(ch_idx) == 0)			
				) {
				no_cali_data = 1;	
		    }            
	    } else if (GetChipIDMIMO88XX(Adapter) == MIMO_3T3R) {
			if(IS_HARDWARE_TYPE_8814A(Adapter)){
#if defined(CONFIG_SLOT_0_8194_2T2R_SUPPORT) || defined(CONFIG_SLOT_1_8194_2T2R_SUPPORT)
				if ((HAL_VAR_pwrlevelHT40_1S_B(ch_idx) == 0) || (HAL_VAR_pwrlevelHT40_1S_C(ch_idx) == 0)) {
					no_cali_data = 1;
				}
#else
				if ((HAL_VAR_pwrlevelHT40_1S_B(ch_idx) == 0) || (HAL_VAR_pwrlevelHT40_1S_C(ch_idx)== 0)
					||(HAL_VAR_pwrlevelHT40_1S_D(ch_idx) == 0)
					) {
					no_cali_data = 1;
			    }
#endif
			}else{
		        if ((HAL_VAR_pwrlevelHT40_1S_A(ch_idx)== 0)||(HAL_VAR_pwrlevelHT40_1S_B(ch_idx) == 0)			
					||(HAL_VAR_pwrlevelHT40_1S_C(ch_idx)== 0)
					) {
					no_cali_data = 1;
			    } 
			}
	    } else if (GetChipIDMIMO88XX(Adapter) == MIMO_2T2R) {
			if ((HAL_VAR_pwrlevelHT40_1S_A(ch_idx) == 0) || (HAL_VAR_pwrlevelHT40_1S_B(ch_idx) == 0)) {
				no_cali_data = 1;
			}
		} else if (GetChipIDMIMO88XX(Adapter) == MIMO_1T1R) {
			if ((HAL_VAR_pwrlevelHT40_1S_A(ch_idx) == 0)) {
				no_cali_data = 1;
			}
		}
	}
	
	if (phy_band == PHY_BAND_5G) {
		CalOFDMTxPower5G_88XX_AC(Adapter, ch_idx);
	} else if (phy_band == PHY_BAND_2G) {
		CalOFDMTxPower2G_88XX_AC(Adapter, ch_idx);
	}

#if 0 //IS_RTL8822B_SERIES	//eric-8822 show tx agc
	if(IS_HARDWARE_TYPE_8822BE(Adapter)) {
		int i = 0, ii=0;
		
		panic_printk("\n PATH A Power:\n");
		for (i = TX_AGC_OFDM_6M; i <= TX_AGC_VHT_NSS2_MCS9; i++) {
			panic_printk("%d ", config_phydm_read_txagc_8822b((&(Adapter->pshare->_dmODM)), 0, i));
			ii++; 
			if(ii==4){
				ii=0; 
				panic_printk("\n");
			}
		}

		ii = 0;
		
		panic_printk("\n PATH B Power:\n");
		for (i = TX_AGC_OFDM_6M; i <= TX_AGC_VHT_NSS2_MCS9; i++) {
			panic_printk("%d ", config_phydm_read_txagc_8822b((&(Adapter->pshare->_dmODM)), 1, i));
			ii++; 
			if(ii==4){
				ii=0; 
				panic_printk("\n");
			}
		}
		panic_printk("\n");
	}
#endif


#if 0//def CONFIG_WLAN_HAL_8814AE
	int path, rate;
	printk("After CalOFDMTxPower5G_88XX_AC()\n");
	for(path=0;path<4;path++){
		printk("\n	Path%d\nCCK:		",path);
		for(rate=TX_AGC_CCK_1M;rate<=TX_AGC_CCK_11M;rate++){
			printk("%x,",Adapter->pshare->phw->CurrentTxAgcCCK[path][rate]);
		}
		printk("\n	OFDM: 		");
		for(rate=TX_AGC_OFDM_6M;rate<=TX_AGC_OFDM_54M;rate++){
			printk("%x,",Adapter->pshare->phw->CurrentTxAgcOFDM[path][rate-TX_AGC_OFDM_6M]);
		}
		printk("\n	HT MCS 1SS:	");
		for(rate=TX_AGC_HT_NSS1_MCS0;rate<=TX_AGC_HT_NSS1_MCS7;rate++){
			printk("%x,",Adapter->pshare->phw->CurrentTxAgcMCS[path][rate-TX_AGC_HT_NSS1_MCS0]);
		}		
		printk("\n	HT MCS 2SS:	");
		for(rate=TX_AGC_HT_NSS2_MCS8;rate<=TX_AGC_HT_NSS2_MCS15;rate++){
			printk("%x,",Adapter->pshare->phw->CurrentTxAgcMCS[path][rate-TX_AGC_HT_NSS1_MCS0]);
		}
		printk("\n	HT MCS 3SS:	");
		for(rate=TX_AGC_HT_NSS3_MCS16;rate<=TX_AGC_HT_NSS3_MCS23;rate++){
			printk("%x,",Adapter->pshare->phw->CurrentTxAgcMCS[path][rate-TX_AGC_HT_NSS1_MCS0]);
		}
		printk("\n	VHT MCS 1SS:	");
		for(rate=TX_AGC_VHT_NSS1_MCS0;rate<=TX_AGC_VHT_NSS1_MCS9;rate++){
			printk("%x,",Adapter->pshare->phw->CurrentTxAgcVHT[path][rate-TX_AGC_VHT_NSS1_MCS0]);
		}			
		printk("\n	VHT MCS 2SS:	");
		for(rate=TX_AGC_VHT_NSS2_MCS0;rate<=TX_AGC_VHT_NSS2_MCS9;rate++){
			printk("%x,",Adapter->pshare->phw->CurrentTxAgcVHT[path][rate-TX_AGC_VHT_NSS1_MCS0]);
		}
		printk("\n	VHT MCS 3SS:	");
		for(rate=TX_AGC_VHT_NSS3_MCS0;rate<=TX_AGC_VHT_NSS3_MCS9;rate++){
			printk("%x,",Adapter->pshare->phw->CurrentTxAgcVHT[path][rate-TX_AGC_VHT_NSS1_MCS0]);
		}
		printk("\n");
	}
#endif //#ifdef CONFIG_WLAN_HAL_8814AE




	if (!no_cali_data) {

#if IS_RTL8814A_SERIES || IS_RTL8822B_SERIES
		if((IS_HARDWARE_TYPE_8814A(Adapter))||(IS_HARDWARE_TYPE_8822BE(Adapter))){
			TxPG_OFDM_8814(Adapter);
			
#if 0//def CONFIG_WLAN_HAL_8814AE
		int path, rate;

		printk("After TxPG_OFDM_8814()\n");
		for(path=0;path<4;path++){
			printk("\n	Path%d\n	CCK:		",path);
			for(rate=TX_AGC_CCK_1M;rate<=TX_AGC_CCK_11M;rate++){
				printk("%x,",Adapter->pshare->phw->CurrentTxAgcCCK[path][rate]);
			}
			printk("\n	OFDM:		");
			for(rate=TX_AGC_OFDM_6M;rate<=TX_AGC_OFDM_54M;rate++){
				printk("%x,",Adapter->pshare->phw->CurrentTxAgcOFDM[path][rate-TX_AGC_OFDM_6M]);
			}
			printk("\n	HT MCS 1SS: ");
			for(rate=TX_AGC_HT_NSS1_MCS0;rate<=TX_AGC_HT_NSS1_MCS7;rate++){
				printk("%x,",Adapter->pshare->phw->CurrentTxAgcMCS[path][rate-TX_AGC_HT_NSS1_MCS0]);
			}		
			printk("\n	HT MCS 2SS: ");
			for(rate=TX_AGC_HT_NSS2_MCS8;rate<=TX_AGC_HT_NSS2_MCS15;rate++){
				printk("%x,",Adapter->pshare->phw->CurrentTxAgcMCS[path][rate-TX_AGC_HT_NSS1_MCS0]);
			}
			printk("\n	HT MCS 3SS: ");
			for(rate=TX_AGC_HT_NSS3_MCS16;rate<=TX_AGC_HT_NSS3_MCS23;rate++){
				printk("%x,",Adapter->pshare->phw->CurrentTxAgcMCS[path][rate-TX_AGC_HT_NSS1_MCS0]);
			}
			printk("\n	VHT MCS 1SS:	");
			for(rate=TX_AGC_VHT_NSS1_MCS0;rate<=TX_AGC_VHT_NSS1_MCS9;rate++){
				printk("%x,",Adapter->pshare->phw->CurrentTxAgcVHT[path][rate-TX_AGC_VHT_NSS1_MCS0]);
			}			
			printk("\n	VHT MCS 2SS:	");
			for(rate=TX_AGC_VHT_NSS2_MCS0;rate<=TX_AGC_VHT_NSS2_MCS9;rate++){
				printk("%x,",Adapter->pshare->phw->CurrentTxAgcVHT[path][rate-TX_AGC_VHT_NSS1_MCS0]);
			}
			printk("\n	VHT MCS 3SS:	");
			for(rate=TX_AGC_VHT_NSS3_MCS0;rate<=TX_AGC_VHT_NSS3_MCS9;rate++){
				printk("%x,",Adapter->pshare->phw->CurrentTxAgcVHT[path][rate-TX_AGC_VHT_NSS1_MCS0]);
			}
			printk("\n");
		}
#endif //#ifdef CONFIG_WLAN_HAL_8814AE
		}else
#endif
		{
		    // TODO: try to rewrite and combine the functions: TxPG_OFDM_Jaguar_A, B, C, D
		    if (GetChipIDMIMO88XX(Adapter) == MIMO_1T1R) {
			    TxPG_OFDM_Jaguar_A(Adapter);
	        } else if (GetChipIDMIMO88XX(Adapter) == MIMO_2T2R) {
	            TxPG_OFDM_Jaguar_A(Adapter);
				TxPG_OFDM_Jaguar_B(Adapter);
	        }
#if IS_RTL8814A_SERIES
			else if (GetChipIDMIMO88XX(Adapter) == MIMO_3T3R) {
	            TxPG_OFDM_Jaguar_A(Adapter);
	            TxPG_OFDM_Jaguar_B(Adapter);
		        TxPG_OFDM_Jaguar_C(Adapter);
	        } else if (GetChipIDMIMO88XX(Adapter) == MIMO_4T4R) {
	            TxPG_OFDM_Jaguar_A(Adapter);
	            TxPG_OFDM_Jaguar_B(Adapter);			
	            TxPG_OFDM_Jaguar_C(Adapter);
	            TxPG_OFDM_Jaguar_D(Adapter);		
	        }
#endif
		}

#ifdef TXPWR_LMT
	if (!Adapter->pshare->rf_ft_var.disable_txpwrlmt) {
		if (HAL_check_lmt_valid(Adapter, phy_band)) {
#if IS_RTL8814A_SERIES || IS_RTL8822B_SERIES 	
			if((IS_HARDWARE_TYPE_8814A(Adapter))||(IS_HARDWARE_TYPE_8822B(Adapter)))
				TxLMT_OFDM_Jaguar_AC_8814(Adapter);
			else
#endif			
				TxLMT_OFDM_Jaguar_AC(Adapter);
			
		}
	}
#endif
	}
	return RT_STATUS_SUCCESS;
}

#if RTL8881A_SUPPORT
RT_STATUS
AddTxPower88XX_AC(
	IN  HAL_PADAPTER    Adapter,
	IN  s1Byte          index
)
{

	char add_index;
	unsigned char b1, b2, b3, b4, b5;
	unsigned char* tmp_b1 = &b1;
	unsigned char* tmp_b2 = &b2;
	unsigned char* tmp_b3 = &b3;
	unsigned char* tmp_b4 = &b4;
	unsigned char* tmp_b5 = &b5;
	unsigned int  tmp_dw = 0;
	unsigned int  tmp_dw1 = 0;
	unsigned int  writeVal = 0;

	add_index = index - Adapter->pshare->rf_ft_var.pwrtrk_TxAGC;
	Adapter->pshare->rf_ft_var.pwrtrk_TxAGC = index;


	//printk("TxPG_OFDM_8812_A!!\n");
	tmp_dw = HAL_RTL_R32(REG_BB_TXAGC_A_OFDM18_OFDM6_AC);
	get_tx_pwr_88XX(tmp_dw, tmp_b1, tmp_b2, tmp_b3, tmp_b4);
	*tmp_b1 = HAL_POWER_RANGE_CHECK(*tmp_b1 + add_index);
	*tmp_b2 = HAL_POWER_RANGE_CHECK(*tmp_b2 + add_index);
	*tmp_b3 = HAL_POWER_RANGE_CHECK(*tmp_b3 + add_index);
	*tmp_b4 = HAL_POWER_RANGE_CHECK(*tmp_b4 + add_index);
	writeVal = (*tmp_b4 << 24) | (*tmp_b3 << 16) | (*tmp_b2 << 8) | *tmp_b1;
	HAL_RTL_W32(REG_BB_TXAGC_A_OFDM18_OFDM6_AC, writeVal);
	tmp_dw = HAL_RTL_R32(REG_BB_TXAGC_A_OFDM54_OFDM24_AC);
	get_tx_pwr_88XX(tmp_dw, tmp_b1, tmp_b2, tmp_b3, tmp_b4);
	*tmp_b1 = HAL_POWER_RANGE_CHECK(*tmp_b1 + add_index);
	*tmp_b2 = HAL_POWER_RANGE_CHECK(*tmp_b2 + add_index);
	*tmp_b3 = HAL_POWER_RANGE_CHECK(*tmp_b3 + add_index);
	*tmp_b4 = HAL_POWER_RANGE_CHECK(*tmp_b4 + add_index);
	writeVal = (*tmp_b4 << 24) | (*tmp_b3 << 16) | (*tmp_b2 << 8) | *tmp_b1;
	HAL_RTL_W32(REG_BB_TXAGC_A_OFDM54_OFDM24_AC, writeVal);
	tmp_dw = HAL_RTL_R32(REG_BB_TXAGC_A_MCS3_MCS0_AC);
	get_tx_pwr_88XX(tmp_dw, tmp_b1, tmp_b2, tmp_b3, tmp_b4);
	*tmp_b1 = HAL_POWER_RANGE_CHECK(*tmp_b1 + add_index);
	*tmp_b2 = HAL_POWER_RANGE_CHECK(*tmp_b2 + add_index);
	*tmp_b3 = HAL_POWER_RANGE_CHECK(*tmp_b3 + add_index);
	*tmp_b4 = HAL_POWER_RANGE_CHECK(*tmp_b4 + add_index);
	writeVal = (*tmp_b4 << 24) | (*tmp_b3 << 16) | (*tmp_b2 << 8) | *tmp_b1;
	HAL_RTL_W32(REG_BB_TXAGC_A_MCS3_MCS0_AC, writeVal);
	tmp_dw = HAL_RTL_R32(REG_BB_TXAGC_A_MCS7_MCS4_AC);
	get_tx_pwr_88XX(tmp_dw, tmp_b1, tmp_b2, tmp_b3, tmp_b4);
	*tmp_b1 = HAL_POWER_RANGE_CHECK(*tmp_b1 + add_index);
	*tmp_b2 = HAL_POWER_RANGE_CHECK(*tmp_b2 + add_index);
	*tmp_b3 = HAL_POWER_RANGE_CHECK(*tmp_b3 + add_index);
	*tmp_b4 = HAL_POWER_RANGE_CHECK(*tmp_b4 + add_index);
	writeVal = (*tmp_b4 << 24) | (*tmp_b3 << 16) | (*tmp_b2 << 8) | *tmp_b1;
	HAL_RTL_W32(REG_BB_TXAGC_A_MCS7_MCS4_AC, writeVal);
	tmp_dw = HAL_RTL_R32(REG_BB_TXAGC_A_MCS11_MCS8_AC);
	get_tx_pwr_88XX(tmp_dw, tmp_b1, tmp_b2, tmp_b3, tmp_b4);
	*tmp_b1 = HAL_POWER_RANGE_CHECK(*tmp_b1 + add_index);
	*tmp_b2 = HAL_POWER_RANGE_CHECK(*tmp_b2 + add_index);
	*tmp_b3 = HAL_POWER_RANGE_CHECK(*tmp_b3 + add_index);
	*tmp_b4 = HAL_POWER_RANGE_CHECK(*tmp_b4 + add_index);
	writeVal = (*tmp_b4 << 24) | (*tmp_b3 << 16) | (*tmp_b2 << 8) | *tmp_b1;
	HAL_RTL_W32(REG_BB_TXAGC_A_MCS11_MCS8_AC, writeVal);
	tmp_dw = HAL_RTL_R32(REG_BB_TXAGC_A_MCS15_MCS12_AC);
	get_tx_pwr_88XX(tmp_dw, tmp_b1, tmp_b2, tmp_b3, tmp_b4);
	*tmp_b1 = HAL_POWER_RANGE_CHECK(*tmp_b1 + add_index);
	*tmp_b2 = HAL_POWER_RANGE_CHECK(*tmp_b2 + add_index);
	*tmp_b3 = HAL_POWER_RANGE_CHECK(*tmp_b3 + add_index);
	*tmp_b4 = HAL_POWER_RANGE_CHECK(*tmp_b4 + add_index);
	writeVal = (*tmp_b4 << 24) | (*tmp_b3 << 16) | (*tmp_b2 << 8) | *tmp_b1;
	HAL_RTL_W32(REG_BB_TXAGC_A_MCS15_MCS12_AC, writeVal);
	tmp_dw = HAL_RTL_R32(REG_BB_TXAGC_A_NSS1INDEX3_NSS1INDEX0_AC);
	get_tx_pwr_88XX(tmp_dw, tmp_b1, tmp_b2, tmp_b3, tmp_b4);
	*tmp_b1 = HAL_POWER_RANGE_CHECK(*tmp_b1 + add_index);
	*tmp_b2 = HAL_POWER_RANGE_CHECK(*tmp_b2 + add_index);
	*tmp_b3 = HAL_POWER_RANGE_CHECK(*tmp_b3 + add_index);
	*tmp_b4 = HAL_POWER_RANGE_CHECK(*tmp_b4 + add_index);
	writeVal = (*tmp_b4 << 24) | (*tmp_b3 << 16) | (*tmp_b2 << 8) | *tmp_b1;
	HAL_RTL_W32(REG_BB_TXAGC_A_NSS1INDEX3_NSS1INDEX0_AC, writeVal);
	tmp_dw = HAL_RTL_R32(REG_BB_TXAGC_A_NSS1INDEX7_NSS1INDEX4_AC);
	get_tx_pwr_88XX(tmp_dw, tmp_b1, tmp_b2, tmp_b3, tmp_b4);
	*tmp_b1 = HAL_POWER_RANGE_CHECK(*tmp_b1 + add_index);
	*tmp_b2 = HAL_POWER_RANGE_CHECK(*tmp_b2 + add_index);
	*tmp_b3 = HAL_POWER_RANGE_CHECK(*tmp_b3 + add_index);
	*tmp_b4 = HAL_POWER_RANGE_CHECK(*tmp_b4 + add_index);
	writeVal = (*tmp_b4 << 24) | (*tmp_b3 << 16) | (*tmp_b2 << 8) | *tmp_b1;
	HAL_RTL_W32(REG_BB_TXAGC_A_NSS1INDEX7_NSS1INDEX4_AC, writeVal);

	tmp_dw = HAL_RTL_R32(REG_BB_TXAGC_A_NSS2INDEX1_NSS1INDEX8_AC);
	get_tx_pwr_88XX(tmp_dw, tmp_b1, tmp_b2, tmp_b3, tmp_b4);
	*tmp_b1 = HAL_POWER_RANGE_CHECK(*tmp_b1 + add_index);
	*tmp_b2 = HAL_POWER_RANGE_CHECK(*tmp_b2 + add_index);
	*tmp_b3 = HAL_POWER_RANGE_CHECK(*tmp_b3 + add_index);
	*tmp_b4 = HAL_POWER_RANGE_CHECK(*tmp_b4 + add_index);
	writeVal = (*tmp_b4 << 24) | (*tmp_b3 << 16) | (*tmp_b2 << 8) | *tmp_b1;
	HAL_RTL_W32(REG_BB_TXAGC_A_NSS2INDEX1_NSS1INDEX8_AC, writeVal);

	tmp_dw = HAL_RTL_R32(REG_BB_TXAGC_A_NSS2INDEX5_NSS2INDEX2_AC);
	get_tx_pwr_88XX(tmp_dw, tmp_b1, tmp_b2, tmp_b3, tmp_b4);
	*tmp_b1 = HAL_POWER_RANGE_CHECK(*tmp_b1 + add_index);
	*tmp_b2 = HAL_POWER_RANGE_CHECK(*tmp_b2 + add_index);
	*tmp_b3 = HAL_POWER_RANGE_CHECK(*tmp_b3 + add_index);
	*tmp_b4 = HAL_POWER_RANGE_CHECK(*tmp_b4 + add_index);
	writeVal = (*tmp_b4 << 24) | (*tmp_b3 << 16) | (*tmp_b2 << 8) | *tmp_b1;
	HAL_RTL_W32(REG_BB_TXAGC_A_NSS2INDEX5_NSS2INDEX2_AC, writeVal);

	tmp_dw = HAL_RTL_R32(REG_BB_TXAGC_A_NSS2INDEX9_NSS2INDEX6_AC);
	get_tx_pwr_88XX(tmp_dw, tmp_b1, tmp_b2, tmp_b3, tmp_b4);
	*tmp_b1 = HAL_POWER_RANGE_CHECK(*tmp_b1 + add_index);
	*tmp_b2 = HAL_POWER_RANGE_CHECK(*tmp_b2 + add_index);
	*tmp_b3 = HAL_POWER_RANGE_CHECK(*tmp_b3 + add_index);
	*tmp_b4 = HAL_POWER_RANGE_CHECK(*tmp_b4 + add_index);
	writeVal = (*tmp_b4 << 24) | (*tmp_b3 << 16) | (*tmp_b2 << 8) | *tmp_b1;
	HAL_RTL_W32(REG_BB_TXAGC_A_NSS2INDEX9_NSS2INDEX6_AC, writeVal);



	return RT_STATUS_SUCCESS;

}
#endif

void
CalOFDMTxPower5G_88XX_AC(
	IN  HAL_PADAPTER    Adapter,
	IN  u1Byte          ch_idx
)
{
	u1Byte tmp_TPI = 0;
	u1Byte pwr_40_1s = 0, pwr_80_1s = 0, ch_idx_ht40 = 0, ch_idx_vht80 = 0;
	s1Byte diff_ofdm_1t = 0;
	s1Byte diff_bw40_2s = 0;
	s1Byte diff_bw40_3s = 0;	
	s1Byte diff_bw20_1s = 0;
	s1Byte diff_bw20_2s = 0;
	s1Byte diff_bw20_3s = 0;	
	s1Byte diff_bw80_1s = 0;
	s1Byte diff_bw80_2s = 0;
	s1Byte diff_bw80_3s = 0;
	u4Byte writeVal     = 0;
    u1Byte bPathXEn[4]  = {0};
	ch_idx_ht40 = ((HAL_VAR_OFFSET_2ND_CHANNEL==1) ? (HAL_VAR_DOT11CHANNEL-2) : (HAL_VAR_DOT11CHANNEL+2)) -1;
	ch_idx_vht80 = get_center_channel(Adapter, HAL_VAR_DOT11CHANNEL, HAL_VAR_OFFSET_2ND_CHANNEL, 1) -1;
	
    u1Byte i;
    u1Byte HAL_VAR_pwrlevel5GHT40_1S_X[4] = {HAL_VAR_pwrlevel5GHT40_1S_A(ch_idx), HAL_VAR_pwrlevel5GHT40_1S_B(ch_idx),		
		HAL_VAR_pwrlevel5GHT40_1S_C(ch_idx), HAL_VAR_pwrlevel5GHT40_1S_D(ch_idx)	
		};
	/* for 40M*/
	u1Byte HAL_VAR_pwrlevel5GHT40_1S_X_40M[4] = {HAL_VAR_pwrlevel5GHT40_1S_A(ch_idx_ht40), HAL_VAR_pwrlevel5GHT40_1S_B(ch_idx_ht40),		
		HAL_VAR_pwrlevel5GHT40_1S_C(ch_idx_ht40), HAL_VAR_pwrlevel5GHT40_1S_D(ch_idx_ht40)
		};
	/* for 80M*/
    u1Byte HAL_VAR_pwrlevel5GHT40_1S_X_A[4] = {HAL_VAR_pwrlevel5GHT40_1S_A(ch_idx_vht80-4), HAL_VAR_pwrlevel5GHT40_1S_B(ch_idx_vht80-4), 
		HAL_VAR_pwrlevel5GHT40_1S_C(ch_idx_vht80-4), HAL_VAR_pwrlevel5GHT40_1S_D(ch_idx_vht80-4)
		};
    u1Byte HAL_VAR_pwrlevel5GHT40_1S_X_B[4] = {HAL_VAR_pwrlevel5GHT40_1S_A(ch_idx_vht80+4), HAL_VAR_pwrlevel5GHT40_1S_B(ch_idx_vht80+4),		
		HAL_VAR_pwrlevel5GHT40_1S_C(ch_idx_vht80+4), HAL_VAR_pwrlevel5GHT40_1S_D(ch_idx_vht80+4)
		};
    s1Byte HAL_VAR_pwrdiff_5G_20BW1S_OFDM1T_X[4] = {HAL_VAR_pwrdiff_5G_20BW1S_OFDM1T_A(ch_idx), HAL_VAR_pwrdiff_5G_20BW1S_OFDM1T_B(ch_idx),
		HAL_VAR_pwrdiff_5G_20BW1S_OFDM1T_C(ch_idx), HAL_VAR_pwrdiff_5G_20BW1S_OFDM1T_D(ch_idx)
		};
    s1Byte HAL_VAR_pwrdiff_5G_40BW2S_20BW2S_X[4] = {HAL_VAR_pwrdiff_5G_40BW2S_20BW2S_A(ch_idx), HAL_VAR_pwrdiff_5G_40BW2S_20BW2S_B(ch_idx), 
		HAL_VAR_pwrdiff_5G_40BW2S_20BW2S_C(ch_idx), HAL_VAR_pwrdiff_5G_40BW2S_20BW2S_D(ch_idx)
		};
    s1Byte HAL_VAR_pwrdiff_5G_80BW1S_160BW1S_X[4] = {HAL_VAR_pwrdiff_5G_80BW1S_160BW1S_A(ch_idx), HAL_VAR_pwrdiff_5G_80BW1S_160BW1S_B(ch_idx),
		HAL_VAR_pwrdiff_5G_80BW1S_160BW1S_C(ch_idx), HAL_VAR_pwrdiff_5G_80BW1S_160BW1S_D(ch_idx)
		};
    s1Byte HAL_VAR_pwrdiff_5G_80BW2S_160BW2S_X[4] = {HAL_VAR_pwrdiff_5G_80BW2S_160BW2S_A(ch_idx), HAL_VAR_pwrdiff_5G_80BW2S_160BW2S_B(ch_idx), 
		HAL_VAR_pwrdiff_5G_80BW2S_160BW2S_C(ch_idx), HAL_VAR_pwrdiff_5G_80BW2S_160BW2S_D(ch_idx)
		};	
    s1Byte HAL_VAR_pwrdiff_5G_40BW3S_20BW3S_X[4] = {HAL_VAR_pwrdiff_5G_40BW3S_20BW3S_A(ch_idx), HAL_VAR_pwrdiff_5G_40BW3S_20BW3S_B(ch_idx), HAL_VAR_pwrdiff_5G_40BW3S_20BW3S_C(ch_idx), HAL_VAR_pwrdiff_5G_40BW3S_20BW3S_D(ch_idx)};		
	s1Byte HAL_VAR_pwrdiff_5G_80BW3S_160BW3S_X[4] = {HAL_VAR_pwrdiff_5G_80BW3S_160BW3S_A(ch_idx), HAL_VAR_pwrdiff_5G_80BW3S_160BW3S_B(ch_idx), HAL_VAR_pwrdiff_5G_80BW3S_160BW3S_C(ch_idx), HAL_VAR_pwrdiff_5G_80BW3S_160BW3S_D(ch_idx)};		


    if (GetChipIDMIMO88XX(Adapter) == MIMO_4T4R) {
        bPathXEn[0] = 1;
        bPathXEn[1] = 1;
        bPathXEn[2] = 1;
        bPathXEn[3] = 1;
        if ((HAL_VAR_pwrlevel5GHT40_1S_A(ch_idx)== 0)||(HAL_VAR_pwrlevel5GHT40_1S_B(ch_idx) == 0)
			||(HAL_VAR_pwrlevel5GHT40_1S_C(ch_idx)== 0)||(HAL_VAR_pwrlevel5GHT40_1S_D(ch_idx)== 0)	
			) {
            use_DefaultOFDMTxPowerPathX88XX_AC(Adapter, bPathXEn);
            return;
        }
    }
    else if (GetChipIDMIMO88XX(Adapter) == MIMO_3T3R) {
		if(IS_HARDWARE_TYPE_8814A(Adapter)){
#if defined(CONFIG_SLOT_0_8814_2T2R_SUPPORT) || defined(CONFIG_SLOT_1_8814_2T2R_SUPPORT)
			bPathXEn[0] = 0;
			bPathXEn[1] = 1;
			bPathXEn[2] = 1;
			bPathXEn[3] = 0;
			if ((HAL_VAR_pwrlevel5GHT40_1S_B(ch_idx)== 0) || (HAL_VAR_pwrlevel5GHT40_1S_C(ch_idx) == 0)){
				use_DefaultOFDMTxPowerPathX88XX_AC(Adapter, bPathXEn);
				return;
			}
#else
			bPathXEn[0] = 1;
			bPathXEn[1] = 1;
			bPathXEn[2] = 1;
			bPathXEn[3] = 1;
			if ((HAL_VAR_pwrlevel5GHT40_1S_B(ch_idx)== 0) || (HAL_VAR_pwrlevel5GHT40_1S_C(ch_idx) == 0)
				||(HAL_VAR_pwrlevel5GHT40_1S_D(ch_idx)== 0)
			){
				use_DefaultOFDMTxPowerPathX88XX_AC(Adapter, bPathXEn);
				return;
			}
#endif
		}else

		{
			bPathXEn[0] = 1;
			bPathXEn[1] = 1;
			bPathXEn[2] = 1;
			bPathXEn[3] = 0;
	        if ((HAL_VAR_pwrlevel5GHT40_1S_A(ch_idx)== 0)||(HAL_VAR_pwrlevel5GHT40_1S_B(ch_idx) == 0)
				||(HAL_VAR_pwrlevel5GHT40_1S_C(ch_idx)== 0)	
				) {
	            use_DefaultOFDMTxPowerPathX88XX_AC(Adapter, bPathXEn);
	            return;
	        }
		}
    }
    else if (GetChipIDMIMO88XX(Adapter) == MIMO_2T2R) {
        bPathXEn[0] = 1;
        bPathXEn[1] = 1;
        bPathXEn[2] = 0;
        bPathXEn[3] = 0;
	    if ((HAL_VAR_pwrlevel5GHT40_1S_A(ch_idx)== 0)||(HAL_VAR_pwrlevel5GHT40_1S_B(ch_idx) == 0)) {
            use_DefaultOFDMTxPowerPathX88XX_AC(Adapter, bPathXEn);
       		return;
	    }
	}
    else if (GetChipIDMIMO88XX(Adapter) == MIMO_1T1R) {
        bPathXEn[0] = 1;
        bPathXEn[1] = 0;
        bPathXEn[2] = 0;
        bPathXEn[3] = 0;
        if ((HAL_VAR_pwrlevel5GHT40_1S_A(ch_idx)== 0)) {
            use_DefaultOFDMTxPowerPathX88XX_AC(Adapter, bPathXEn);
    		return;
        }
	}

    for (i = 0; i < 4; i++) {
        // PATH i, OFDM
        if(bPathXEn[i]) {
            pwr_40_1s =  HAL_VAR_pwrlevel5GHT40_1S_X[i];
            if(pwr_40_1s == 0)
                pwr_40_1s = 0x12;
            diff_ofdm_1t = (HAL_VAR_pwrdiff_5G_20BW1S_OFDM1T_X[i] & 0x0f);
            diff_ofdm_1t = convert_diff_88XX_AC(diff_ofdm_1t);
            tmp_TPI = pwr_40_1s + diff_ofdm_1t;
			//tmp_TPI  = HAL_POWER_RANGE_CHECK(tmp_TPI );
#if (IS_RTL8814A_SERIES||IS_RTL8822B_SERIES)
            if (( IS_HARDWARE_TYPE_8814A(Adapter) && (IS_HAL_TEST_CHIP(Adapter)==FALSE))||
				( IS_HARDWARE_TYPE_8822B(Adapter) && (IS_HAL_TEST_CHIP(Adapter)==FALSE))){
                PHYSetTxPower88XX(Adapter, TX_AGC_OFDM_6M, TX_AGC_OFDM_54M, i, tmp_TPI);
            } else
#endif
            {
                writeVal = (tmp_TPI << 24) | (tmp_TPI << 16) | (tmp_TPI << 8) | tmp_TPI;
                Write_OFDM_X_88XX_AC(Adapter, writeVal, i);
            }
        }
    }
   
    //3 20M
	if (HAL_VAR_CurrentChannelBW == 0) {

        for (i = 0; i < 4; i++) {
            if(bPathXEn[i]) {
                //PATH i, BW20-1S
                
                pwr_40_1s = HAL_VAR_pwrlevel5GHT40_1S_X[i];
                if(pwr_40_1s == 0)
                    pwr_40_1s = 0x12;
				//printk("[%s]pwr_40_1s=%x\n",__FUNCTION__,pwr_40_1s);
                diff_bw20_1s = ((HAL_VAR_pwrdiff_5G_20BW1S_OFDM1T_X[i] & 0xf0) >> 4);				
				//printk("[%s]diff_bw20_1s=%x\n",__FUNCTION__,diff_bw20_1s);
                diff_bw20_1s = convert_diff_88XX_AC(diff_bw20_1s);
                tmp_TPI = pwr_40_1s + diff_bw20_1s;
                writeVal = (tmp_TPI << 24) | (tmp_TPI << 16) | (tmp_TPI << 8) | tmp_TPI;		
				//printk("[%s]writeVal=%x\n",__FUNCTION__,writeVal);
#if (IS_RTL8814A_SERIES||IS_RTL8822B_SERIES)
                if (( IS_HARDWARE_TYPE_8814A(Adapter) && (IS_HAL_TEST_CHIP(Adapter)==FALSE))||
					( IS_HARDWARE_TYPE_8822B(Adapter) && (IS_HAL_TEST_CHIP(Adapter)==FALSE))){
                    PHYSetTxPower88XX(Adapter, TX_AGC_HT_NSS1_MCS0, TX_AGC_HT_NSS1_MCS7, i, tmp_TPI);
                    PHYSetTxPower88XX(Adapter, TX_AGC_VHT_NSS1_MCS0, TX_AGC_VHT_NSS1_MCS9, i, tmp_TPI);
                } else
#endif
                {
                    Write_1S_X_88XX_AC(Adapter, writeVal, i);
                }
                       
                //PATH i, BW20-2S
                diff_bw20_2s = (HAL_VAR_pwrdiff_5G_40BW2S_20BW2S_X[i] & 0x0f);
				//printk("[%s]diff_bw20_2s=%x\n",__FUNCTION__,diff_bw20_2s);
				
                diff_bw20_2s = convert_diff_88XX_AC(diff_bw20_2s);
                tmp_TPI = pwr_40_1s + diff_bw20_1s + diff_bw20_2s;
                writeVal = (tmp_TPI << 24) | (tmp_TPI << 16) | (tmp_TPI << 8) | tmp_TPI;
#if (IS_RTL8814A_SERIES||IS_RTL8822B_SERIES)
                if (( IS_HARDWARE_TYPE_8814A(Adapter) && (IS_HAL_TEST_CHIP(Adapter)==FALSE))||
					( IS_HARDWARE_TYPE_8822B(Adapter) && (IS_HAL_TEST_CHIP(Adapter)==FALSE))){
                    PHYSetTxPower88XX(Adapter, TX_AGC_HT_NSS2_MCS8, TX_AGC_HT_NSS2_MCS15, i, tmp_TPI);
                    PHYSetTxPower88XX(Adapter, TX_AGC_VHT_NSS2_MCS0, TX_AGC_VHT_NSS2_MCS9, i, tmp_TPI);
                } else
#endif
                {
                    Write_2S_X_88XX_AC(Adapter, writeVal, i);
                }

#if IS_RTL8814A_SERIES
				//PATH i, BW20-3S
                diff_bw20_3s = (HAL_VAR_pwrdiff_5G_40BW3S_20BW3S_X[i] & 0x0f);
                diff_bw20_3s = convert_diff_88XX_AC(diff_bw20_3s);
                tmp_TPI = pwr_40_1s + diff_bw20_1s + diff_bw20_2s + diff_bw20_3s;
                writeVal = (tmp_TPI << 24) | (tmp_TPI << 16) | (tmp_TPI << 8) | tmp_TPI;
                if ( IS_HARDWARE_TYPE_8814A(Adapter) && (IS_HAL_TEST_CHIP(Adapter)==FALSE)) {
                    PHYSetTxPower88XX(Adapter, TX_AGC_HT_NSS3_MCS16, TX_AGC_HT_NSS3_MCS23, i, tmp_TPI);
                    PHYSetTxPower88XX(Adapter, TX_AGC_VHT_NSS3_MCS0, TX_AGC_VHT_NSS3_MCS9, i, tmp_TPI);
                } else
                {
                    Write_3S_X_88XX_AC(Adapter, writeVal, i);
                }
#endif
            }
        }
	//3 40M
	} else if (HAL_VAR_CurrentChannelBW == 1) {
        for (i = 0; i < 4; i++) {
            if(bPathXEn[i]) {
                //PATH i, BW40-1S
                pwr_40_1s = HAL_VAR_pwrlevel5GHT40_1S_X_40M[i];
                if(pwr_40_1s == 0)
                    pwr_40_1s = 0x12;
                tmp_TPI = pwr_40_1s ;
                writeVal = (tmp_TPI << 24) | (tmp_TPI << 16) | (tmp_TPI << 8) | tmp_TPI;
#if (IS_RTL8814A_SERIES||IS_RTL8822B_SERIES)
                if (( IS_HARDWARE_TYPE_8814A(Adapter) && (IS_HAL_TEST_CHIP(Adapter)==FALSE))||
					( IS_HARDWARE_TYPE_8822B(Adapter) && (IS_HAL_TEST_CHIP(Adapter)==FALSE))){
                    PHYSetTxPower88XX(Adapter, TX_AGC_HT_NSS1_MCS0, TX_AGC_HT_NSS1_MCS7, i, tmp_TPI);
                    PHYSetTxPower88XX(Adapter, TX_AGC_VHT_NSS1_MCS0, TX_AGC_VHT_NSS1_MCS9, i, tmp_TPI);
                } else
#endif
                {
                    Write_1S_X_88XX_AC(Adapter, writeVal, i);
                }

                //PATH i, BW40-2S
                diff_bw40_2s = ((HAL_VAR_pwrdiff_5G_40BW2S_20BW2S_X[i] & 0xf0) >> 4);
                diff_bw40_2s = convert_diff_88XX_AC(diff_bw40_2s);
                tmp_TPI = pwr_40_1s + diff_bw40_2s;
                writeVal = (tmp_TPI << 24) | (tmp_TPI << 16) | (tmp_TPI << 8) | tmp_TPI;
#if (IS_RTL8814A_SERIES||IS_RTL8822B_SERIES)
                if (( IS_HARDWARE_TYPE_8814A(Adapter) && (IS_HAL_TEST_CHIP(Adapter)==FALSE))||
					( IS_HARDWARE_TYPE_8822B(Adapter) && (IS_HAL_TEST_CHIP(Adapter)==FALSE))){
                    PHYSetTxPower88XX(Adapter, TX_AGC_HT_NSS2_MCS8, TX_AGC_HT_NSS2_MCS15, i, tmp_TPI);
                    PHYSetTxPower88XX(Adapter, TX_AGC_VHT_NSS2_MCS0, TX_AGC_VHT_NSS2_MCS9, i, tmp_TPI);
                } else
#endif
                {
                    Write_2S_X_88XX_AC(Adapter, writeVal, i);
                }

#if defined(CONFIG_WLAN_HAL_8814AE)
				//PATH i, BW40-3S
				diff_bw40_3s = ((HAL_VAR_pwrdiff_5G_40BW3S_20BW3S_X[i] & 0xf0) >> 4);
                diff_bw40_3s = convert_diff_88XX_AC(diff_bw40_3s);
                tmp_TPI = pwr_40_1s + diff_bw40_2s + diff_bw40_3s;
                writeVal = (tmp_TPI << 24) | (tmp_TPI << 16) | (tmp_TPI << 8) | tmp_TPI;
#if IS_RTL8814A_SERIES
                if ( IS_HARDWARE_TYPE_8814A(Adapter) && (IS_HAL_TEST_CHIP(Adapter)==FALSE)) {
                    PHYSetTxPower88XX(Adapter, TX_AGC_HT_NSS3_MCS16, TX_AGC_HT_NSS3_MCS23, i, tmp_TPI);
                    PHYSetTxPower88XX(Adapter, TX_AGC_VHT_NSS3_MCS0, TX_AGC_VHT_NSS3_MCS9, i, tmp_TPI);
                } else
#endif
                {
                    Write_3S_X_88XX_AC(Adapter, writeVal, i);
                }
#endif
            }
        }
	}
	//3 80M
	else if (HAL_VAR_CurrentChannelBW == 2) {


        for (i = 0; i < 4; i++) {
            if(bPathXEn[i]) {
                //PATH X, BW80-1S
                pwr_40_1s = (HAL_VAR_pwrlevel5GHT40_1S_X_A[i] +
                             HAL_VAR_pwrlevel5GHT40_1S_X_B[i]) / 2  ;
                if(pwr_40_1s == 0)
                    pwr_40_1s = 0x12;
                diff_bw80_1s = ((HAL_VAR_pwrdiff_5G_80BW1S_160BW1S_X[i] & 0xf0) >> 4);
                diff_bw80_1s = convert_diff_88XX_AC(diff_bw80_1s);
                tmp_TPI = pwr_40_1s + diff_bw80_1s;
                writeVal = (tmp_TPI << 24) | (tmp_TPI << 16) | (tmp_TPI << 8) | tmp_TPI;
#if (IS_RTL8814A_SERIES||IS_RTL8822B_SERIES)
                if (( IS_HARDWARE_TYPE_8814A(Adapter) && (IS_HAL_TEST_CHIP(Adapter)==FALSE))||
					( IS_HARDWARE_TYPE_8822B(Adapter) && (IS_HAL_TEST_CHIP(Adapter)==FALSE))){
                    PHYSetTxPower88XX(Adapter, TX_AGC_HT_NSS1_MCS0, TX_AGC_HT_NSS1_MCS7, i, tmp_TPI);
                    PHYSetTxPower88XX(Adapter, TX_AGC_VHT_NSS1_MCS0, TX_AGC_VHT_NSS1_MCS9, i, tmp_TPI);
                } else
#endif
                {
                    Write_1S_X_88XX_AC(Adapter, writeVal, i);
                }

                //PATH X, BW80-2S
                diff_bw80_2s = ((HAL_VAR_pwrdiff_5G_80BW2S_160BW2S_X[i] & 0xf0) >> 4);
                diff_bw80_2s = convert_diff_88XX_AC(diff_bw80_2s);
                tmp_TPI = pwr_40_1s + diff_bw80_1s + diff_bw80_2s;
                writeVal = (tmp_TPI << 24) | (tmp_TPI << 16) | (tmp_TPI << 8) | tmp_TPI;
#if (IS_RTL8814A_SERIES||IS_RTL8822B_SERIES)
                if (( IS_HARDWARE_TYPE_8814A(Adapter) && (IS_HAL_TEST_CHIP(Adapter)==FALSE))||
					( IS_HARDWARE_TYPE_8822B(Adapter) && (IS_HAL_TEST_CHIP(Adapter)==FALSE))){
                    PHYSetTxPower88XX(Adapter, TX_AGC_HT_NSS2_MCS8, TX_AGC_HT_NSS2_MCS15, i, tmp_TPI);
                    PHYSetTxPower88XX(Adapter, TX_AGC_VHT_NSS2_MCS0, TX_AGC_VHT_NSS2_MCS9, i, tmp_TPI);
                } else
#endif
                {
                    Write_2S_X_88XX_AC(Adapter, writeVal, i);
                }

#if defined(CONFIG_WLAN_HAL_8814AE)
				//PATH X, BW80-2S
			   diff_bw80_3s = ((HAL_VAR_pwrdiff_5G_80BW3S_160BW3S_X[i] & 0xf0) >> 4);
			   diff_bw80_3s = convert_diff_88XX_AC(diff_bw80_3s);
			   //panic_printk("diff_bw80_3s=%c, %d\n",diff_bw80_3s,diff_bw80_3s);
			   tmp_TPI = pwr_40_1s + diff_bw80_1s + diff_bw80_2s +diff_bw80_3s;
			   writeVal = (tmp_TPI << 24) | (tmp_TPI << 16) | (tmp_TPI << 8) | tmp_TPI;
#if IS_RTL8814A_SERIES
               if ( IS_HARDWARE_TYPE_8814A(Adapter) && (IS_HAL_TEST_CHIP(Adapter)==FALSE)) {
                   PHYSetTxPower88XX(Adapter, TX_AGC_HT_NSS3_MCS16, TX_AGC_HT_NSS3_MCS23, i, tmp_TPI);
                   PHYSetTxPower88XX(Adapter, TX_AGC_VHT_NSS3_MCS0, TX_AGC_VHT_NSS3_MCS9, i, tmp_TPI);
               } else
#endif
               {
                   Write_3S_X_88XX_AC(Adapter, writeVal, i);
               }

#endif
            }
        }
	}
}

void
CalOFDMTxPower2G_88XX_AC(
	IN  HAL_PADAPTER    Adapter,
	IN  u1Byte          ch_idx
)
{
	u1Byte tmp_TPI = 0;
	u1Byte pwr_40_1s = 0, ch_idx_ht40 = 0;
	s1Byte diff_ofdm_1t = 0;
	s1Byte diff_bw40_2s = 0;
	s1Byte diff_bw40_3s = 0;	
	s1Byte diff_bw20_1s = 0;
	s1Byte diff_bw20_2s = 0;	
	s1Byte diff_bw20_3s = 0;
	u4Byte writeVal = 0;
    u1Byte bPathXEn[4] = {0};
    u1Byte i;
	
	ch_idx_ht40 = ((HAL_VAR_OFFSET_2ND_CHANNEL==1) ? (HAL_VAR_DOT11CHANNEL-2) : (HAL_VAR_DOT11CHANNEL+2)) -1;

    u1Byte HAL_VAR_pwrlevelHT40_1S_X[4] = {HAL_VAR_pwrlevelHT40_1S_A(ch_idx), HAL_VAR_pwrlevelHT40_1S_B(ch_idx), 
		HAL_VAR_pwrlevelHT40_1S_C(ch_idx), HAL_VAR_pwrlevelHT40_1S_D(ch_idx)
		};
	/* for 40M*/
	u1Byte HAL_VAR_pwrlevelHT40_1S_X_40M[4] = {HAL_VAR_pwrlevelHT40_1S_A(ch_idx_ht40), HAL_VAR_pwrlevelHT40_1S_B(ch_idx_ht40),
		HAL_VAR_pwrlevelHT40_1S_C(ch_idx_ht40), HAL_VAR_pwrlevelHT40_1S_D(ch_idx_ht40)
		};
    s1Byte HAL_VAR_pwrdiff_20BW1S_OFDM1T_X[4] = {HAL_VAR_pwrdiff_20BW1S_OFDM1T_A(ch_idx), HAL_VAR_pwrdiff_20BW1S_OFDM1T_B(ch_idx),
		HAL_VAR_pwrdiff_20BW1S_OFDM1T_C(ch_idx), HAL_VAR_pwrdiff_20BW1S_OFDM1T_D(ch_idx)
		};
    s1Byte HAL_VAR_pwrdiff_40BW2S_20BW2S_X[4] = {HAL_VAR_pwrdiff_40BW2S_20BW2S_A(ch_idx), HAL_VAR_pwrdiff_40BW2S_20BW2S_B(ch_idx),
		HAL_VAR_pwrdiff_40BW2S_20BW2S_C(ch_idx), HAL_VAR_pwrdiff_40BW2S_20BW2S_D(ch_idx)
		};
	s1Byte HAL_VAR_pwrdiff_40BW3S_20BW3S_X[4] = {HAL_VAR_pwrdiff_40BW3S_20BW3S_A(ch_idx), HAL_VAR_pwrdiff_40BW3S_20BW3S_B(ch_idx), 
		HAL_VAR_pwrdiff_40BW3S_20BW3S_C(ch_idx), HAL_VAR_pwrdiff_40BW3S_20BW3S_D(ch_idx)
		};		
       
    if (GetChipIDMIMO88XX(Adapter) == MIMO_4T4R) {
        bPathXEn[0] = 1;
        bPathXEn[1] = 1;
        bPathXEn[2] = 1;
        bPathXEn[3] = 1;
        if ((HAL_VAR_pwrlevelHT40_1S_A(ch_idx)== 0)||(HAL_VAR_pwrlevelHT40_1S_B(ch_idx) == 0)	
			||(HAL_VAR_pwrlevelHT40_1S_C(ch_idx) == 0)||(HAL_VAR_pwrlevelHT40_1S_D(ch_idx) == 0)
			) {
            use_DefaultOFDMTxPowerPathX88XX_AC(Adapter, bPathXEn);
            return;
        }
    }
    else if (GetChipIDMIMO88XX(Adapter) == MIMO_3T3R) {
		if(IS_HARDWARE_TYPE_8814A(Adapter)){
#if defined(CONFIG_SLOT_0_8194_2T2R_SUPPORT) || defined(CONFIG_SLOT_1_8194_2T2R_SUPPORT)
			bPathXEn[0] = 0;
			bPathXEn[1] = 1;
			bPathXEn[2] = 1;
			bPathXEn[3] = 0;
			if ((HAL_VAR_pwrlevelHT40_1S_B(ch_idx)== 0) || (HAL_VAR_pwrlevelHT40_1S_C(ch_idx) == 0)){
				use_DefaultOFDMTxPowerPathX88XX_AC(Adapter, bPathXEn);
				return;
			}
#else
			bPathXEn[0] = 1;
			bPathXEn[1] = 1;
			bPathXEn[2] = 1;
			bPathXEn[3] = 1;
			if ((HAL_VAR_pwrlevelHT40_1S_B(ch_idx)== 0) || (HAL_VAR_pwrlevelHT40_1S_C(ch_idx) == 0)
				||(HAL_VAR_pwrlevelHT40_1S_D(ch_idx)== 0)
			){
				use_DefaultOFDMTxPowerPathX88XX_AC(Adapter, bPathXEn);
				return;
			}
#endif
		}else{
			bPathXEn[0] = 1;
			bPathXEn[1] = 1;
			bPathXEn[2] = 1;
			bPathXEn[3] = 0;
			if ((HAL_VAR_pwrlevelHT40_1S_A(ch_idx)== 0)||(HAL_VAR_pwrlevelHT40_1S_B(ch_idx) == 0)
				||(HAL_VAR_pwrlevelHT40_1S_C(ch_idx) == 0)
				) {
				use_DefaultOFDMTxPowerPathX88XX_AC(Adapter, bPathXEn);
				return;
			}
		}
    }
    else if (GetChipIDMIMO88XX(Adapter) == MIMO_2T2R) {
        bPathXEn[0] = 1;
        bPathXEn[1] = 1;
        bPathXEn[2] = 0;
        bPathXEn[3] = 0;
        if ((HAL_VAR_pwrlevelHT40_1S_A(ch_idx)== 0)||(HAL_VAR_pwrlevelHT40_1S_B(ch_idx) == 0)) {
            use_DefaultOFDMTxPowerPathX88XX_AC(Adapter, bPathXEn);
            return;
        }
    }
    else if (GetChipIDMIMO88XX(Adapter) == MIMO_1T1R) {
        bPathXEn[0] = 1;
        bPathXEn[1] = 0;
        bPathXEn[2] = 0;
        bPathXEn[3] = 0;
        if ((HAL_VAR_pwrlevelHT40_1S_A(ch_idx)== 0)) {
            use_DefaultOFDMTxPowerPathX88XX_AC(Adapter, bPathXEn);
            return;
        }                   
    }


    for(i = 0; i < 4; i++) {
        // PATH X, OFDM
        if(bPathXEn[i]) {
            pwr_40_1s = HAL_VAR_pwrlevelHT40_1S_X[i];
            if(pwr_40_1s == 0)
                pwr_40_1s = 0x12;
            diff_ofdm_1t = (HAL_VAR_pwrdiff_20BW1S_OFDM1T_X[i] & 0x0f);
            diff_ofdm_1t = convert_diff_88XX_AC(diff_ofdm_1t);
            tmp_TPI = pwr_40_1s + diff_ofdm_1t;
       		//tmp_TPI  = HAL_POWER_RANGE_CHECK(tmp_TPI );
#if (IS_RTL8814A_SERIES||IS_RTL8822B_SERIES)
            if (( IS_HARDWARE_TYPE_8814A(Adapter) && (IS_HAL_TEST_CHIP(Adapter)==FALSE))||
				( IS_HARDWARE_TYPE_8822B(Adapter) && (IS_HAL_TEST_CHIP(Adapter)==FALSE))){
                PHYSetTxPower88XX(Adapter, TX_AGC_OFDM_6M, TX_AGC_OFDM_54M, i, tmp_TPI);
            } else
#endif
            {
                writeVal = (tmp_TPI << 24) | (tmp_TPI << 16) | (tmp_TPI << 8) | tmp_TPI;
                Write_OFDM_X_88XX_AC(Adapter, writeVal, i);
            }
        }
    }
	//3 20M
	if (HAL_VAR_dot11nUse40M == 0) {

        for (i = 0; i < 4; i++) {
            if(bPathXEn[i]) {   
                //PATH A, BW20-1S
                pwr_40_1s = HAL_VAR_pwrlevelHT40_1S_X[i];
                if(pwr_40_1s == 0)
                    pwr_40_1s = 0x12;
                diff_bw20_1s = ((HAL_VAR_pwrdiff_20BW1S_OFDM1T_X[i] & 0xf0) >> 4);
                diff_bw20_1s = convert_diff_88XX_AC(diff_bw20_1s);
                tmp_TPI = pwr_40_1s + diff_bw20_1s;
                writeVal = (tmp_TPI << 24) | (tmp_TPI << 16) | (tmp_TPI << 8) | tmp_TPI;
#if (IS_RTL8814A_SERIES||IS_RTL8822B_SERIES)
                if (( IS_HARDWARE_TYPE_8814A(Adapter) && (IS_HAL_TEST_CHIP(Adapter)==FALSE))|| 
					( IS_HARDWARE_TYPE_8822B(Adapter) && (IS_HAL_TEST_CHIP(Adapter)==FALSE))){
                    PHYSetTxPower88XX(Adapter, TX_AGC_HT_NSS1_MCS0, TX_AGC_HT_NSS1_MCS7, i, tmp_TPI);
                    PHYSetTxPower88XX(Adapter, TX_AGC_VHT_NSS1_MCS0, TX_AGC_VHT_NSS1_MCS9, i, tmp_TPI);
                } else
#endif
                {
                    Write_1S_X_88XX_AC(Adapter, writeVal, i);
                }

                //PATH A, BW20-2S
                diff_bw20_2s = (HAL_VAR_pwrdiff_40BW2S_20BW2S_X[i] & 0x0f);
                diff_bw20_2s = convert_diff_88XX_AC(diff_bw20_2s);
                tmp_TPI = pwr_40_1s + diff_bw20_1s + diff_bw20_2s;
                writeVal = (tmp_TPI << 24) | (tmp_TPI << 16) | (tmp_TPI << 8) | tmp_TPI;
#if (IS_RTL8814A_SERIES||IS_RTL8822B_SERIES)
                if (( IS_HARDWARE_TYPE_8814A(Adapter) && (IS_HAL_TEST_CHIP(Adapter)==FALSE))||
					( IS_HARDWARE_TYPE_8822B(Adapter) && (IS_HAL_TEST_CHIP(Adapter)==FALSE))){
                    PHYSetTxPower88XX(Adapter, TX_AGC_HT_NSS2_MCS8, TX_AGC_HT_NSS2_MCS15, i, tmp_TPI);
                    PHYSetTxPower88XX(Adapter, TX_AGC_VHT_NSS2_MCS0, TX_AGC_VHT_NSS2_MCS9, i, tmp_TPI);
                } else
#endif
                {
                    Write_2S_X_88XX_AC(Adapter, writeVal, i);
                }

#if defined(CONFIG_WLAN_HAL_8814AE)
				//PATH i, BW20-3S
				diff_bw20_3s = (HAL_VAR_pwrdiff_40BW3S_20BW3S_X[i] & 0x0f);
				diff_bw20_3s = convert_diff_88XX_AC(diff_bw20_3s);
				tmp_TPI = pwr_40_1s + diff_bw20_1s + diff_bw20_2s + diff_bw20_3s;
				writeVal = (tmp_TPI << 24) | (tmp_TPI << 16) | (tmp_TPI << 8) | tmp_TPI;
#if IS_RTL8814A_SERIES
               if ( IS_HARDWARE_TYPE_8814A(Adapter) && (IS_HAL_TEST_CHIP(Adapter)==FALSE)) {
                   PHYSetTxPower88XX(Adapter, TX_AGC_HT_NSS3_MCS16, TX_AGC_HT_NSS3_MCS23, i, tmp_TPI);
                   PHYSetTxPower88XX(Adapter, TX_AGC_VHT_NSS3_MCS0, TX_AGC_VHT_NSS3_MCS9, i, tmp_TPI);
               } else
#endif
                {
                	Write_3S_X_88XX_AC(Adapter, writeVal, i);
                }
#endif
            }
        }
	//3 40M
	} else if (HAL_VAR_dot11nUse40M == 1) {
        for (i = 0; i < 4; i++) {
            if(bPathXEn[i]) {
                //PATH A, BW40-1S
                pwr_40_1s = HAL_VAR_pwrlevelHT40_1S_X_40M[i];
                if(pwr_40_1s == 0)
                    pwr_40_1s = 0x12;
                tmp_TPI = pwr_40_1s ;
                writeVal = (tmp_TPI << 24) | (tmp_TPI << 16) | (tmp_TPI << 8) | tmp_TPI;
#if (IS_RTL8814A_SERIES||IS_RTL8822B_SERIES)
                if (( IS_HARDWARE_TYPE_8814A(Adapter) && (IS_HAL_TEST_CHIP(Adapter)==FALSE))||
					( IS_HARDWARE_TYPE_8822B(Adapter) && (IS_HAL_TEST_CHIP(Adapter)==FALSE))){
                    PHYSetTxPower88XX(Adapter, TX_AGC_HT_NSS1_MCS0, TX_AGC_HT_NSS1_MCS7, i, tmp_TPI);
                    PHYSetTxPower88XX(Adapter, TX_AGC_VHT_NSS1_MCS0, TX_AGC_VHT_NSS1_MCS9, i, tmp_TPI);
                } else
#endif
                {
                    Write_1S_X_88XX_AC(Adapter, writeVal, i);
                }

                //PATH A, BW40-2S
                diff_bw40_2s = ((HAL_VAR_pwrdiff_40BW2S_20BW2S_X[i] & 0xf0) >> 4);
                diff_bw40_2s = convert_diff_88XX_AC(diff_bw40_2s);
                tmp_TPI = pwr_40_1s + diff_bw40_2s;
                writeVal = (tmp_TPI << 24) | (tmp_TPI << 16) | (tmp_TPI << 8) | tmp_TPI;
#if (IS_RTL8814A_SERIES||IS_RTL8822B_SERIES)
                if (( IS_HARDWARE_TYPE_8814A(Adapter) && (IS_HAL_TEST_CHIP(Adapter)==FALSE))||
					( IS_HARDWARE_TYPE_8822B(Adapter) && (IS_HAL_TEST_CHIP(Adapter)==FALSE))){
                    PHYSetTxPower88XX(Adapter, TX_AGC_HT_NSS2_MCS8, TX_AGC_HT_NSS2_MCS15, i, tmp_TPI);
                    PHYSetTxPower88XX(Adapter, TX_AGC_VHT_NSS2_MCS0, TX_AGC_VHT_NSS2_MCS9, i, tmp_TPI);
                } else
#endif
                {
                    Write_2S_X_88XX_AC(Adapter, writeVal, i);
                }

#if defined(CONFIG_WLAN_HAL_8814AE)
				//PATH i, BW40-3S
				diff_bw40_3s = ((HAL_VAR_pwrdiff_40BW3S_20BW3S_X[i] & 0xf0) >> 4);
				diff_bw40_3s = convert_diff_88XX_AC(diff_bw40_3s);
				tmp_TPI = pwr_40_1s + diff_bw40_2s + diff_bw40_3s;
				writeVal = (tmp_TPI << 24) | (tmp_TPI << 16) | (tmp_TPI << 8) | tmp_TPI;
#if IS_RTL8814A_SERIES
                if ( IS_HARDWARE_TYPE_8814A(Adapter) && (IS_HAL_TEST_CHIP(Adapter)==FALSE)) {
                    PHYSetTxPower88XX(Adapter, TX_AGC_HT_NSS3_MCS16, TX_AGC_HT_NSS3_MCS23, i, tmp_TPI);
                    PHYSetTxPower88XX(Adapter, TX_AGC_VHT_NSS3_MCS0, TX_AGC_VHT_NSS3_MCS9, i, tmp_TPI);
                } else
#endif
                {
                	Write_3S_X_88XX_AC(Adapter, writeVal, i);
                }
#endif
            }
        }

	}
}



s1Byte
convert_diff_88XX_AC(
	IN s1Byte value
)
{
	// range from -8 ~ 7
	if (value <= 7)
		return value;
	else
		return (value - 16);
}

void 
Write_OFDM_X_88XX_AC(
    IN  HAL_PADAPTER    Adapter, 
    IN  u4Byte          writeVal,
    IN  u1Byte          bPathXEnIdx
)
{
    u4Byte REG_BB_TXAGC_X_OFDM18_OFDM6_AC[4] = {REG_BB_TXAGC_A_OFDM18_OFDM6_AC, REG_BB_TXAGC_B_OFDM18_OFDM6_AC, REG_BB_TXAGC_C_OFDM18_OFDM6_AC, REG_BB_TXAGC_D_OFDM18_OFDM6_AC};
    u4Byte REG_BB_TXAGC_X_OFDM54_OFDM24_AC[4] = {REG_BB_TXAGC_A_OFDM54_OFDM24_AC, REG_BB_TXAGC_B_OFDM54_OFDM24_AC, REG_BB_TXAGC_C_OFDM54_OFDM24_AC, REG_BB_TXAGC_D_OFDM54_OFDM24_AC};

	HAL_RTL_W32(REG_BB_TXAGC_X_OFDM18_OFDM6_AC[bPathXEnIdx], writeVal);
	HAL_RTL_W32(REG_BB_TXAGC_X_OFDM54_OFDM24_AC[bPathXEnIdx], writeVal);
}

void 
Write_1S_X_88XX_AC(
    IN  HAL_PADAPTER    Adapter, 
    IN  u4Byte          writeVal,
    IN  u1Byte          bPathXEnIdx
)
{
    u4Byte REG_BB_TXAGC_X_MCS3_MCS0_AC[4] = {REG_BB_TXAGC_A_MCS3_MCS0_AC, REG_BB_TXAGC_B_MCS3_MCS0_AC, REG_BB_TXAGC_C_MCS3_MCS0_AC, REG_BB_TXAGC_D_MCS3_MCS0_AC};
    u4Byte REG_BB_TXAGC_X_MCS7_MCS4_AC[4] = {REG_BB_TXAGC_A_MCS7_MCS4_AC, REG_BB_TXAGC_B_MCS7_MCS4_AC, REG_BB_TXAGC_C_MCS7_MCS4_AC, REG_BB_TXAGC_D_MCS7_MCS4_AC};
    u4Byte REG_BB_TXAGC_X_NSS1INDEX3_NSS1INDEX0_AC[4] = {REG_BB_TXAGC_A_NSS1INDEX3_NSS1INDEX0_AC, REG_BB_TXAGC_B_NSS1INDEX3_NSS1INDEX0_AC, REG_BB_TXAGC_C_NSS1INDEX3_NSS1INDEX0_AC, REG_BB_TXAGC_D_NSS1INDEX3_NSS1INDEX0_AC};
    u4Byte REG_BB_TXAGC_X_NSS1INDEX7_NSS1INDEX4_AC[4] = {REG_BB_TXAGC_A_NSS1INDEX7_NSS1INDEX4_AC, REG_BB_TXAGC_B_NSS1INDEX7_NSS1INDEX4_AC, REG_BB_TXAGC_C_NSS1INDEX7_NSS1INDEX4_AC, REG_BB_TXAGC_D_NSS1INDEX7_NSS1INDEX4_AC};
    u4Byte REG_BB_TXAGC_X_NSS2INDEX1_NSS1INDEX8_AC[4] = {REG_BB_TXAGC_A_NSS2INDEX1_NSS1INDEX8_AC, REG_BB_TXAGC_B_NSS2INDEX1_NSS1INDEX8_AC, REG_BB_TXAGC_C_NSS2INDEX1_NSS1INDEX8_AC, REG_BB_TXAGC_D_NSS2INDEX1_NSS1INDEX8_AC};
	HAL_RTL_W32(REG_BB_TXAGC_X_MCS3_MCS0_AC[bPathXEnIdx], writeVal);
	HAL_RTL_W32(REG_BB_TXAGC_X_MCS7_MCS4_AC[bPathXEnIdx], writeVal);
	HAL_RTL_W32(REG_BB_TXAGC_X_NSS1INDEX3_NSS1INDEX0_AC[bPathXEnIdx], writeVal);
	HAL_RTL_W32(REG_BB_TXAGC_X_NSS1INDEX7_NSS1INDEX4_AC[bPathXEnIdx], writeVal);
    // TODO: compare with Write_2S_X_88XX_AC, why don't we write only NSS1 (reference to Write_2S_X_88XX_AC)
	HAL_RTL_W32(REG_BB_TXAGC_X_NSS2INDEX1_NSS1INDEX8_AC[bPathXEnIdx], writeVal);
}


void 
Write_2S_X_88XX_AC(
    IN  HAL_PADAPTER    Adapter, 
    IN  u4Byte          writeVal,
    IN  u1Byte          bPathXEnIdx
)
{
    u4Byte REG_BB_TXAGC_X_MCS11_MCS8_AC[4] = {REG_BB_TXAGC_A_MCS11_MCS8_AC, REG_BB_TXAGC_B_MCS11_MCS8_AC, REG_BB_TXAGC_C_MCS11_MCS8_AC, REG_BB_TXAGC_D_MCS11_MCS8_AC};
    u4Byte REG_BB_TXAGC_X_MCS15_MCS12_AC[4] = {REG_BB_TXAGC_A_MCS15_MCS12_AC, REG_BB_TXAGC_B_MCS15_MCS12_AC, REG_BB_TXAGC_C_MCS15_MCS12_AC, REG_BB_TXAGC_D_MCS15_MCS12_AC};
    u4Byte REG_BB_TXAGC_X_NSS2INDEX5_NSS2INDEX2_AC[4] = {REG_BB_TXAGC_A_NSS2INDEX5_NSS2INDEX2_AC, REG_BB_TXAGC_B_NSS2INDEX5_NSS2INDEX2_AC, REG_BB_TXAGC_C_NSS2INDEX5_NSS2INDEX2_AC, REG_BB_TXAGC_D_NSS2INDEX5_NSS2INDEX2_AC};
    u4Byte REG_BB_TXAGC_X_NSS2INDEX9_NSS2INDEX6_AC[4] = {REG_BB_TXAGC_A_NSS2INDEX9_NSS2INDEX6_AC, REG_BB_TXAGC_B_NSS2INDEX9_NSS2INDEX6_AC, REG_BB_TXAGC_C_NSS2INDEX9_NSS2INDEX6_AC, REG_BB_TXAGC_D_NSS2INDEX9_NSS2INDEX6_AC};
    u4Byte REG_BB_TXAGC_X_NSS2INDEX1_NSS1INDEX8_AC[4] = {REG_BB_TXAGC_A_NSS2INDEX1_NSS1INDEX8_AC, REG_BB_TXAGC_B_NSS2INDEX1_NSS1INDEX8_AC, REG_BB_TXAGC_C_NSS2INDEX1_NSS1INDEX8_AC, REG_BB_TXAGC_D_NSS2INDEX1_NSS1INDEX8_AC};

	HAL_RTL_W32(REG_BB_TXAGC_X_MCS11_MCS8_AC[bPathXEnIdx], writeVal);
	HAL_RTL_W32(REG_BB_TXAGC_X_MCS15_MCS12_AC[bPathXEnIdx], writeVal);
	HAL_RTL_W32(REG_BB_TXAGC_X_NSS2INDEX5_NSS2INDEX2_AC[bPathXEnIdx], writeVal);
	HAL_RTL_W32(REG_BB_TXAGC_X_NSS2INDEX9_NSS2INDEX6_AC[bPathXEnIdx], writeVal);
	writeVal = (writeVal & 0xffff0000) | (HAL_RTL_R32(REG_BB_TXAGC_X_NSS2INDEX1_NSS1INDEX8_AC[bPathXEnIdx]) & 0xffff);
	HAL_RTL_W32(REG_BB_TXAGC_X_NSS2INDEX1_NSS1INDEX8_AC[bPathXEnIdx], writeVal);
}

void 
Write_3S_X_88XX_AC(
    IN  HAL_PADAPTER    Adapter, 
    IN  u4Byte          writeVal,
    IN  u1Byte          bPathXEnIdx
)
{
    u4Byte REG_BB_TXAGC_X_MCS19_MCS16_AC[4] = {REG_BB_TXAGC_A_MCS19_MCS16_AC, REG_BB_TXAGC_B_MCS19_MCS16_AC, REG_BB_TXAGC_C_MCS19_MCS16_AC, REG_BB_TXAGC_D_MCS19_MCS16_AC};
    u4Byte REG_BB_TXAGC_X_MCS23_MCS20_AC[4] = {REG_BB_TXAGC_A_MCS23_MCS20_AC, REG_BB_TXAGC_B_MCS23_MCS20_AC, REG_BB_TXAGC_C_MCS23_MCS20_AC, REG_BB_TXAGC_D_MCS23_MCS20_AC};
    u4Byte REG_BB_TXAGC_X_NSS3INDEX3_NSS3INDEX0_AC[4] = {REG_BB_TXAGC_A_NSS3INDEX3_NSS3INDEX0_AC, REG_BB_TXAGC_B_NSS3INDEX3_NSS3INDEX0_AC, REG_BB_TXAGC_C_NSS3INDEX3_NSS3INDEX0_AC, REG_BB_TXAGC_D_NSS3INDEX3_NSS3INDEX0_AC};
    u4Byte REG_BB_TXAGC_X_NSS3INDEX7_NSS3INDEX4_AC[4] = {REG_BB_TXAGC_A_NSS3INDEX7_NSS3INDEX4_AC, REG_BB_TXAGC_B_NSS3INDEX7_NSS3INDEX4_AC, REG_BB_TXAGC_C_NSS3INDEX7_NSS3INDEX4_AC, REG_BB_TXAGC_D_NSS3INDEX7_NSS3INDEX4_AC};
    u4Byte REG_BB_TXAGC_X_NSS3INDEX9_NSS3INDEX8_AC[4] = {REG_BB_TXAGC_A_NSS3INDEX9_NSS3INDEX8_AC, REG_BB_TXAGC_B_NSS3INDEX9_NSS3INDEX8_AC, REG_BB_TXAGC_C_NSS3INDEX9_NSS3INDEX8_AC, REG_BB_TXAGC_D_NSS3INDEX9_NSS3INDEX8_AC};

	HAL_RTL_W32(REG_BB_TXAGC_X_MCS19_MCS16_AC[bPathXEnIdx], writeVal);
	HAL_RTL_W32(REG_BB_TXAGC_X_MCS23_MCS20_AC[bPathXEnIdx], writeVal);
	HAL_RTL_W32(REG_BB_TXAGC_X_NSS3INDEX3_NSS3INDEX0_AC[bPathXEnIdx], writeVal);
	HAL_RTL_W32(REG_BB_TXAGC_X_NSS3INDEX7_NSS3INDEX4_AC[bPathXEnIdx], writeVal);
	writeVal = (writeVal & 0xffff) | (HAL_RTL_R32(REG_BB_TXAGC_X_NSS3INDEX9_NSS3INDEX8_AC[bPathXEnIdx]) & 0xffff0000);
	HAL_RTL_W32(REG_BB_TXAGC_X_NSS3INDEX9_NSS3INDEX8_AC[bPathXEnIdx], writeVal);
}

void 
use_DefaultOFDMTxPowerPathX88XX_AC(
    IN  HAL_PADAPTER    Adapter,
    IN  pu1Byte         bPathXEn
)
{
    u4Byte def_power = 0x20202020; //eric-8822 ??  //0x12121212;   //0x20202020; 
    u4Byte REG_BB_TXAGC_X_OFDM18_OFDM6_AC[4] = {REG_BB_TXAGC_A_OFDM18_OFDM6_AC, REG_BB_TXAGC_B_OFDM18_OFDM6_AC, REG_BB_TXAGC_C_OFDM18_OFDM6_AC, REG_BB_TXAGC_D_OFDM18_OFDM6_AC};
    u4Byte REG_BB_TXAGC_X_OFDM54_OFDM24_AC[4] = {REG_BB_TXAGC_A_OFDM54_OFDM24_AC, REG_BB_TXAGC_B_OFDM54_OFDM24_AC, REG_BB_TXAGC_C_OFDM54_OFDM24_AC, REG_BB_TXAGC_D_OFDM54_OFDM24_AC};
    u4Byte REG_BB_TXAGC_X_MCS3_MCS0_AC[4] = {REG_BB_TXAGC_A_MCS3_MCS0_AC, REG_BB_TXAGC_B_MCS3_MCS0_AC, REG_BB_TXAGC_C_MCS3_MCS0_AC, REG_BB_TXAGC_D_MCS3_MCS0_AC};
    u4Byte REG_BB_TXAGC_X_MCS7_MCS4_AC[4] = {REG_BB_TXAGC_A_MCS7_MCS4_AC, REG_BB_TXAGC_B_MCS7_MCS4_AC, REG_BB_TXAGC_C_MCS7_MCS4_AC, REG_BB_TXAGC_D_MCS7_MCS4_AC};
    u4Byte REG_BB_TXAGC_X_MCS11_MCS8_AC[4] = {REG_BB_TXAGC_A_MCS11_MCS8_AC, REG_BB_TXAGC_B_MCS11_MCS8_AC, REG_BB_TXAGC_C_MCS11_MCS8_AC, REG_BB_TXAGC_D_MCS11_MCS8_AC};
    u4Byte REG_BB_TXAGC_X_MCS15_MCS12_AC[4] = {REG_BB_TXAGC_A_MCS15_MCS12_AC, REG_BB_TXAGC_B_MCS15_MCS12_AC, REG_BB_TXAGC_C_MCS15_MCS12_AC, REG_BB_TXAGC_C_MCS15_MCS12_AC};
    u4Byte REG_BB_TXAGC_X_NSS1INDEX3_NSS1INDEX0_AC[4] = {REG_BB_TXAGC_A_NSS1INDEX3_NSS1INDEX0_AC, REG_BB_TXAGC_B_NSS1INDEX3_NSS1INDEX0_AC, REG_BB_TXAGC_C_NSS1INDEX3_NSS1INDEX0_AC, REG_BB_TXAGC_D_NSS1INDEX3_NSS1INDEX0_AC};
    u4Byte REG_BB_TXAGC_X_NSS1INDEX7_NSS1INDEX4_AC[4] = {REG_BB_TXAGC_A_NSS1INDEX7_NSS1INDEX4_AC, REG_BB_TXAGC_B_NSS1INDEX7_NSS1INDEX4_AC, REG_BB_TXAGC_C_NSS1INDEX7_NSS1INDEX4_AC, REG_BB_TXAGC_D_NSS1INDEX7_NSS1INDEX4_AC};
    u4Byte REG_BB_TXAGC_X_NSS2INDEX1_NSS1INDEX8_AC[4] = {REG_BB_TXAGC_A_NSS2INDEX1_NSS1INDEX8_AC, REG_BB_TXAGC_B_NSS2INDEX1_NSS1INDEX8_AC, REG_BB_TXAGC_C_NSS2INDEX1_NSS1INDEX8_AC, REG_BB_TXAGC_D_NSS2INDEX1_NSS1INDEX8_AC};
    u4Byte REG_BB_TXAGC_X_NSS2INDEX5_NSS2INDEX2_AC[4] = {REG_BB_TXAGC_A_NSS2INDEX5_NSS2INDEX2_AC, REG_BB_TXAGC_B_NSS2INDEX5_NSS2INDEX2_AC, REG_BB_TXAGC_C_NSS2INDEX5_NSS2INDEX2_AC, REG_BB_TXAGC_D_NSS2INDEX5_NSS2INDEX2_AC};
    u4Byte REG_BB_TXAGC_X_NSS2INDEX9_NSS2INDEX6_AC[4] = {REG_BB_TXAGC_A_NSS2INDEX9_NSS2INDEX6_AC, REG_BB_TXAGC_B_NSS2INDEX9_NSS2INDEX6_AC, REG_BB_TXAGC_C_NSS2INDEX9_NSS2INDEX6_AC, REG_BB_TXAGC_D_NSS2INDEX9_NSS2INDEX6_AC};
    u1Byte i;

    u1Byte def_power_value = 0x16;

    for (i = 0; i < 4; i++) {
        if (bPathXEn[i]) {
#if IS_RTL8814A_SERIES
            if ( IS_HARDWARE_TYPE_8814A(Adapter) && (IS_HAL_TEST_CHIP(Adapter)==FALSE)) {
                PHYSetTxPower88XX(Adapter, TX_AGC_OFDM_6M, TX_AGC_VHT_NSS4_MCS9, i, def_power_value);
            } else 
#endif
#if IS_RTL8822B_SERIES //eric-8822
			if ( IS_HARDWARE_TYPE_8822BE(Adapter)) {
#if 1
				if(i == 0 && Adapter->pshare->rf_ft_var.txa)
					def_power_value = Adapter->pshare->rf_ft_var.txa;

				if(i == 1 && Adapter->pshare->rf_ft_var.txb)
					def_power_value = Adapter->pshare->rf_ft_var.txb;

#endif
				PHYSetTxPower88XX(Adapter, TX_AGC_OFDM_6M, TX_AGC_VHT_NSS2_MCS9, i, def_power_value);
			} else 
#endif

            {
            	HAL_RTL_W32(REG_BB_TXAGC_X_OFDM18_OFDM6_AC[i], def_power);
            	HAL_RTL_W32(REG_BB_TXAGC_X_OFDM54_OFDM24_AC[i], def_power);
            	HAL_RTL_W32(REG_BB_TXAGC_X_MCS3_MCS0_AC[i], def_power);
            	HAL_RTL_W32(REG_BB_TXAGC_X_MCS7_MCS4_AC[i], def_power);
            	HAL_RTL_W32(REG_BB_TXAGC_X_MCS11_MCS8_AC[i], def_power);
            	HAL_RTL_W32(REG_BB_TXAGC_X_MCS15_MCS12_AC[i], def_power);
            	HAL_RTL_W32(REG_BB_TXAGC_X_NSS1INDEX3_NSS1INDEX0_AC[i], def_power);
            	HAL_RTL_W32(REG_BB_TXAGC_X_NSS1INDEX7_NSS1INDEX4_AC[i], def_power);
            	HAL_RTL_W32(REG_BB_TXAGC_X_NSS2INDEX1_NSS1INDEX8_AC[i], def_power);
            	HAL_RTL_W32(REG_BB_TXAGC_X_NSS2INDEX5_NSS2INDEX2_AC[i], def_power);
            	HAL_RTL_W32(REG_BB_TXAGC_X_NSS2INDEX9_NSS2INDEX6_AC[i], def_power);
            }
        }
    }
}

#endif //#if IS_RTL88XX_AC


#if IS_RTL88XX_N

void
phy_RFSerialWrite_88XX_N
(
	IN  HAL_PADAPTER                Adapter,
	IN  RFRF88XX_RADIO_PATH_E       eRFPath,
	IN  u4Byte                      Offset,
	IN  u4Byte                      Data
)
{
	u4Byte				DataAndAddr = 0;
	u4Byte				NewOffset;

	Offset &= 0xff;

	//
	// Switch page for 8256 RF IC
	//
	NewOffset = Offset;

	//
	// Put write addr in [5:0]  and write data in [31:16]
	//
	//DataAndAddr = (Data<<16) | (NewOffset&0x3f);
	DataAndAddr = ((NewOffset << 20) | (Data & 0x000fffff)) & 0x0fffffff;	// T65 RF

	//
	// Write Operation
	//
	if (eRFPath == RF88XX_PATH_A) {
		PHY_SetBBReg_88XX(Adapter, REG_BB_FPGA0_XA_LSSIPARAMETER_N, BIT_MASK_SET_MASKDWORD_COMMON, DataAndAddr);
	} else {
		PHY_SetBBReg_88XX(Adapter, REG_BB_FPGA0_XB_LSSIPARAMETER_N, BIT_MASK_SET_MASKDWORD_COMMON, DataAndAddr);
	}
}

u4Byte
phy_RFSerialRead_88XX_N
(
	IN  HAL_PADAPTER                Adapter,
	IN  RFRF88XX_RADIO_PATH_E       eRFPath,
	IN  u4Byte                      Offset
)
{
	u4Byte  tmplong, tmplong2;
	u4Byte  retValue = 0;
	u4Byte  NewOffset;

	//
	// Make sure RF register offset is correct
	//

	Offset &= 0xff;


	//
	// Switch page for 8256 RF IC
	//
	NewOffset = Offset;

	// For 92S LSSI Read RFLSSIRead
	// For RF A/B write 0x824/82c(does not work in the future)
	// We must use 0x824 for RF A and B to execute read trigger
	// 1. PathA : addr write 0x824, Path B : addr write 0x82c

	if (eRFPath == RF88XX_PATH_A) {
		tmplong2 = PHY_QueryBBReg_88XX(Adapter, REG_BB_FPGA0_XA_HSSIPARAMETER2_N, BIT_MASK_SET_MASKDWORD_COMMON);;
		tmplong2 = (tmplong2 & (~BIT_MASK_BB_LSSIREADADDRESS_N)) | (NewOffset << 23) | BIT_MASK_BB_LSSIREADEDGE_N;	//T65 RF
		PHY_SetBBReg_88XX(Adapter, REG_BB_FPGA0_XA_HSSIPARAMETER2_N, BIT_MASK_SET_MASKDWORD_COMMON, tmplong2 & (~BIT_MASK_BB_LSSIREADEDGE_N));
	} else {
		tmplong2 = PHY_QueryBBReg_88XX(Adapter, REG_BB_FPGA0_XB_HSSIPARAMETER2_N, BIT_MASK_SET_MASKDWORD_COMMON);
		tmplong2 = (tmplong2 & (~BIT_MASK_BB_LSSIREADADDRESS_N)) | (NewOffset << 23) | BIT_MASK_BB_LSSIREADEDGE_N;	//T65 RF
		PHY_SetBBReg_88XX(Adapter, REG_BB_FPGA0_XB_HSSIPARAMETER2_N, BIT_MASK_SET_MASKDWORD_COMMON, tmplong2 & (~BIT_MASK_BB_LSSIREADEDGE_N));
	}

	tmplong2 = PHY_QueryBBReg_88XX(Adapter, REG_BB_FPGA0_XA_HSSIPARAMETER2_N, BIT_MASK_SET_MASKDWORD_COMMON);
	PHY_SetBBReg_88XX(Adapter, REG_BB_FPGA0_XA_HSSIPARAMETER2_N, BIT_MASK_SET_MASKDWORD_COMMON, tmplong2 & (~BIT_MASK_BB_LSSIREADEDGE_N));
	PHY_SetBBReg_88XX(Adapter, REG_BB_FPGA0_XA_HSSIPARAMETER2_N, BIT_MASK_SET_MASKDWORD_COMMON, tmplong2 | BIT_MASK_BB_LSSIREADEDGE_N);

	delay_us(20);

	// 1. Path A, PI mode read data from 0x8b8
	// 2. Path A, SI mode read data from 0x8a0
	// 3. Path B, PI mode read data from 0x8bc
	// 4. Path B, SI mode read data from 0x8a4

	if (eRFPath == RF88XX_PATH_A) {
		if ((HAL_RTL_R32(REG_BB_FPGA0_XA_HSSIPARAMETER1_N)&BIT(8))) {
			retValue = PHY_QueryBBReg_88XX(Adapter, REG_BB_TRANSCEIVERA_HSPI_READBACK_N, BIT_MASK_BB_LSSIREADBACKDATA_N);
		} else {
			retValue = PHY_QueryBBReg_88XX(Adapter, REG_BB_FPGA0_XA_LSSIREADBACK_N, BIT_MASK_BB_LSSIREADBACKDATA_N);
		}
	} else if (eRFPath == RF88XX_PATH_B) {
		if ((HAL_RTL_R32(REG_BB_FPGA0_XB_HSSIPARAMETER1_N)&BIT(8))) {
			retValue = PHY_QueryBBReg_88XX(Adapter, REG_BB_TRANSCEIVERB_HSPI_READBACK_N, BIT_MASK_BB_LSSIREADBACKDATA_N);
		} else {
			retValue = PHY_QueryBBReg_88XX(Adapter, REG_BB_FPGA0_XB_LSSIREADBACK_N, BIT_MASK_BB_LSSIREADBACKDATA_N);
		}
	}


	return retValue;
}

void PHY_SetRFReg_88XX_N
(
	IN  HAL_PADAPTER                Adapter,
	IN  u4Byte                      eRFPath,
	IN  u4Byte                      RegAddr,
	IN  u4Byte                      BitMask,
	IN  u4Byte                      Data
)
{
	u4Byte          Original_Value, BitShift, New_Value;
	HAL_PADAPTER    priv     = Adapter;
	u4Byte          flags;

#if CFG_HAL_DISABLE_BB_RF
	return;
#endif //CFG_HAL_DISABLE_BB_RF

	HAL_SAVE_INT_AND_CLI(flags);

	if (BitMask != BIT_MASK_SET_MASK20BITS_COMMON) {
		Original_Value = phy_RFSerialRead_88XX_N(Adapter, eRFPath, RegAddr);
		BitShift = phy_CalculateBitShift_88XX(BitMask);
		New_Value = ((Original_Value & (~BitMask)) | (Data << BitShift));

		phy_RFSerialWrite_88XX_N(Adapter, eRFPath, RegAddr, New_Value);
	} else {
		phy_RFSerialWrite_88XX_N(Adapter, eRFPath, RegAddr, Data);
	}

	HAL_RESTORE_INT(flags);
	delay_us(4);    // At least 500ns delay to avoid RF write fail.
}

u4Byte
PHY_QueryRFReg_88XX_N
(
	IN  HAL_PADAPTER                Adapter,
	IN  u4Byte                      eRFPath,
	IN  u4Byte                      RegAddr,
	IN  u4Byte                      BitMask
)
{
	HAL_PADAPTER    priv     = Adapter;
	u4Byte          flags;
	u4Byte          Original_Value, Readback_Value, BitShift;

#if CFG_HAL_DISABLE_BB_RF
	return 0;
#endif //CFG_HAL_DISABLE_BB_RF

	HAL_SAVE_INT_AND_CLI(flags);
	Original_Value = phy_RFSerialRead_88XX_N(Adapter, eRFPath, RegAddr);
	BitShift =  phy_CalculateBitShift_88XX(BitMask);
	Readback_Value = (Original_Value & BitMask) >> BitShift;
	HAL_RESTORE_INT(flags);

	return (Readback_Value);
}

RT_STATUS
PHYSetOFDMTxPower88XX_N(
	IN  HAL_PADAPTER    Adapter,
	IN  u1Byte          channel
)
{
	u4Byte writeVal, defValue = 0x28, pwrdiff = 0x06060606;
	u1Byte offset, ch_idx=0, ch_idx_ht40=0;
	u1Byte pwrlevelHT40_1S_A = 0;
	u1Byte pwrlevelHT40_1S_B = 0;
	u1Byte pwrdiffHT40_2S = 0;
	u1Byte pwrdiffHT20 = HAL_VAR_pwrdiffHT20(channel - 1);
	u1Byte pwrdiffOFDM = HAL_VAR_pwrdiffOFDM(channel - 1);
	u1Byte base, byte0, byte1, byte2, byte3;
	u1Byte phy_band = 0, is_coexist = 0;	
	PRTL8192CD_PRIV		priv = Adapter;
#if CFG_HAL_POWER_PERCENT_ADJUSTMENT
	s1Byte pwrdiff_percent = HAL_PwrPercent2PwrLevel(HAL_VAR_power_percent);
#endif

	if (channel > 0) {
		ch_idx = (channel - 1);
		ch_idx_ht40 = ((HAL_VAR_OFFSET_2ND_CHANNEL==1) ? (channel-2) : (channel+2)) -1;
	} else {
		RT_TRACE_F(COMP_RF, DBG_WARNING, ("Error Channel !!\n"));
		return RT_STATUS_FAILURE;
	}

	if (channel > CHANNEL_MAX_NUMBER_2G)
		phy_band = PHY_BAND_5G;
	else
		phy_band = PHY_BAND_2G;


#ifdef WIFI_11N_2040_COEXIST
		if(COEXIST_ENABLE && (((HAL_OPMODE & WIFI_AP_STATE) && 
				 (Adapter->bg_ap_timeout || orForce20_Switch20Map(Adapter)
				 ))
		 ))
			is_coexist = 1;	 
#endif

	if ((HAL_VAR_CurrentChannelBW == HT_CHANNEL_WIDTH_20) || is_coexist) {
		pwrlevelHT40_1S_A = HAL_VAR_pwrlevelHT40_1S_A(ch_idx);
		pwrlevelHT40_1S_B = HAL_VAR_pwrlevelHT40_1S_B(ch_idx);
		pwrdiffHT40_2S = HAL_VAR_pwrdiffHT40_2S(ch_idx);
	} else if (HAL_VAR_CurrentChannelBW == HT_CHANNEL_WIDTH_20_40) {
		pwrlevelHT40_1S_A = HAL_VAR_pwrlevelHT40_1S_A(ch_idx_ht40);
		pwrlevelHT40_1S_B = HAL_VAR_pwrlevelHT40_1S_B(ch_idx_ht40);
		pwrdiffHT40_2S = HAL_VAR_pwrdiffHT40_2S(ch_idx_ht40);
	}

	if ((pwrlevelHT40_1S_A == 0) )

	{
		// use default value

#if CFG_HAL_HIGH_POWER_EXT_PA
		if (HAL_VAR_use_ext_pa)
			defValue = HP_OFDM_POWER_DEFAULT ;
#endif
#if  (!CFG_HAL_ADD_TX_POWER_BY_CMD)
		writeVal = (defValue << 24) | (defValue << 16) | (defValue << 8) | (defValue);
		HAL_RTL_W32(REG_BB_TXAGC_A_RATE18_06_N, writeVal);
		HAL_RTL_W32(REG_BB_TXAGC_A_RATE54_24_N, writeVal);
		HAL_RTL_W32(REG_BB_TXAGC_A_MCS03_MCS00_N, writeVal);
		HAL_RTL_W32(REG_BB_TXAGC_A_MCS07_MCS04_N, writeVal);
		HAL_RTL_W32(REG_BB_TXAGC_B_RATE18_06_N, writeVal);
		HAL_RTL_W32(REG_BB_TXAGC_B_RATE54_24_N, writeVal);
		HAL_RTL_W32(REG_BB_TXAGC_B_MCS03_MCS00_N, writeVal);
		HAL_RTL_W32(REG_BB_TXAGC_B_MCS07_MCS04_N, writeVal);
		HAL_RTL_W32(REG_BB_TXAGC_A_MCS11_MCS08_N, writeVal);
		HAL_RTL_W32(REG_BB_TXAGC_A_MCS15_MCS12_N, writeVal);
		HAL_RTL_W32(REG_BB_TXAGC_B_MCS11_MCS08_N, writeVal);
		HAL_RTL_W32(REG_BB_TXAGC_B_MCS15_MCS12_N, writeVal);
#else

		base = defValue;
		byte0 = byte1 = byte2 = byte3 = 0;
		HAL_ASSIGN_TX_POWER_OFFSET(byte0, HAL_VAR_txPowerPlus_ofdm_18);
		HAL_ASSIGN_TX_POWER_OFFSET(byte1, HAL_VAR_txPowerPlus_ofdm_12);
		HAL_ASSIGN_TX_POWER_OFFSET(byte2, HAL_VAR_txPowerPlus_ofdm_9);
		HAL_ASSIGN_TX_POWER_OFFSET(byte3, HAL_VAR_txPowerPlus_ofdm_6);

		byte0 = HAL_POWER_RANGE_CHECK(base + byte0);
		byte1 = HAL_POWER_RANGE_CHECK(base + byte1);
		byte2 = HAL_POWER_RANGE_CHECK(base + byte2);
		byte3 = HAL_POWER_RANGE_CHECK(base + byte3);
		writeVal = (byte0 << 24) | (byte1 << 16) | (byte2 << 8) | byte3;
		HAL_RTL_W32(REG_BB_TXAGC_A_RATE18_06_N, writeVal);

		if (GetChipIDMIMO88XX(Adapter) == MIMO_2T2R)
			HAL_RTL_W32(REG_BB_TXAGC_B_RATE18_06_N, writeVal);

		byte0 = byte1 = byte2 = byte3 = 0;
		HAL_ASSIGN_TX_POWER_OFFSET(byte0, HAL_VAR_txPowerPlus_ofdm_54);
		HAL_ASSIGN_TX_POWER_OFFSET(byte1, HAL_VAR_txPowerPlus_ofdm_48);
		HAL_ASSIGN_TX_POWER_OFFSET(byte2, HAL_VAR_txPowerPlus_ofdm_36);
		HAL_ASSIGN_TX_POWER_OFFSET(byte3, HAL_VAR_txPowerPlus_ofdm_24);
		byte0 = HAL_POWER_RANGE_CHECK(base + byte0);
		byte1 = HAL_POWER_RANGE_CHECK(base + byte1);
		byte2 = HAL_POWER_RANGE_CHECK(base + byte2);
		byte3 = HAL_POWER_RANGE_CHECK(base + byte3);
		writeVal = (byte0 << 24) | (byte1 << 16) | (byte2 << 8) | byte3;
		HAL_RTL_W32(REG_BB_TXAGC_A_RATE54_24_N, writeVal);
		if (GetChipIDMIMO88XX(Adapter) == MIMO_2T2R)
			HAL_RTL_W32(REG_BB_TXAGC_B_RATE54_24_N, writeVal);

		byte0 = byte1 = byte2 = byte3 = 0;
		HAL_ASSIGN_TX_POWER_OFFSET(byte0, HAL_VAR_txPowerPlus_mcs_3);
		HAL_ASSIGN_TX_POWER_OFFSET(byte1, HAL_VAR_txPowerPlus_mcs_2);
		HAL_ASSIGN_TX_POWER_OFFSET(byte2, HAL_VAR_txPowerPlus_mcs_1);
		HAL_ASSIGN_TX_POWER_OFFSET(byte3, HAL_VAR_txPowerPlus_mcs_0);
		byte0 = HAL_POWER_RANGE_CHECK(base + byte0);
		byte1 = HAL_POWER_RANGE_CHECK(base + byte1);
		byte2 = HAL_POWER_RANGE_CHECK(base + byte2);
		byte3 = HAL_POWER_RANGE_CHECK(base + byte3);
		writeVal = (byte0 << 24) | (byte1 << 16) | (byte2 << 8) | byte3;
		HAL_RTL_W32(REG_BB_TXAGC_A_MCS03_MCS00_N, writeVal);
		if (GetChipIDMIMO88XX(Adapter) == MIMO_2T2R)
			HAL_RTL_W32(REG_BB_TXAGC_B_MCS03_MCS00_N, writeVal);

		byte0 = byte1 = byte2 = byte3 = 0;
		HAL_ASSIGN_TX_POWER_OFFSET(byte0, HAL_VAR_txPowerPlus_mcs_7);
		HAL_ASSIGN_TX_POWER_OFFSET(byte1, HAL_VAR_txPowerPlus_mcs_6);
		HAL_ASSIGN_TX_POWER_OFFSET(byte2, HAL_VAR_txPowerPlus_mcs_5);
		HAL_ASSIGN_TX_POWER_OFFSET(byte3, HAL_VAR_txPowerPlus_mcs_4);
		byte0 = HAL_POWER_RANGE_CHECK(base + byte0);
		byte1 = HAL_POWER_RANGE_CHECK(base + byte1);
		byte2 = HAL_POWER_RANGE_CHECK(base + byte2);
		byte3 = HAL_POWER_RANGE_CHECK(base + byte3);
		writeVal = (byte0 << 24) | (byte1 << 16) | (byte2 << 8) | byte3;
		HAL_RTL_W32(REG_BB_TXAGC_A_MCS07_MCS04_N, writeVal);
		if (GetChipIDMIMO88XX(Adapter) == MIMO_2T2R)
			HAL_RTL_W32(REG_BB_TXAGC_B_MCS07_MCS04_N, writeVal);

		byte0 = byte1 = byte2 = byte3 = 0;
		byte0 = HAL_POWER_RANGE_CHECK(base + byte0);
		byte1 = HAL_POWER_RANGE_CHECK(base + byte1);
		byte2 = HAL_POWER_RANGE_CHECK(base + byte2);
		byte3 = HAL_POWER_RANGE_CHECK(base + byte3);
		writeVal = (byte0 << 24) | (byte1 << 16) | (byte2 << 8) | byte3;
		HAL_RTL_W32(REG_BB_TXAGC_A_MCS11_MCS08_N, writeVal);
		if (GetChipIDMIMO88XX(Adapter) == MIMO_2T2R)
			HAL_RTL_W32(REG_BB_TXAGC_B_MCS11_MCS08_N, writeVal);

		byte0 = byte1 = byte2 = byte3 = 0;
		byte0 = HAL_POWER_RANGE_CHECK(base + byte0);
		byte1 = HAL_POWER_RANGE_CHECK(base + byte1);
		byte2 = HAL_POWER_RANGE_CHECK(base + byte2);
		byte3 = HAL_POWER_RANGE_CHECK(base + byte3);
		writeVal = (byte0 << 24) | (byte1 << 16) | (byte2 << 8) | byte3;
		HAL_RTL_W32(REG_BB_TXAGC_A_MCS15_MCS12_N, writeVal);
		if (GetChipIDMIMO88XX(Adapter) == MIMO_2T2R)
			HAL_RTL_W32(REG_BB_TXAGC_B_MCS15_MCS12_N, writeVal);

#ifdef TXPWR_LMT
		if (!Adapter->pshare->rf_ft_var.disable_txpwrlmt) {
			if (HAL_check_lmt_valid(Adapter, phy_band)) {
				TxLMT_OFDM_Jaguar_N(Adapter);
			}
		}
#endif // TXPWR_LMT

#endif // ADD_TX_POWER_BY_CMD
		return RT_STATUS_SUCCESS; // use default
	}

	/******************************  PATH A  ******************************/
	base = pwrlevelHT40_1S_A;
	offset = (pwrdiffOFDM & 0x0f);
	base = HAL_COUNT_SIGN_OFFSET(base, offset);
#if CFG_HAL_POWER_PERCENT_ADJUSTMENT
	base = HAL_POWER_RANGE_CHECK(base + pwrdiff_percent);
#endif

	// fill 6Mbps ~ 18Mbps
	byte0 = HAL_POWER_RANGE_CHECK(base + HAL_VAR_OFDMTxAgcOffset_A(0));
	byte1 = HAL_POWER_RANGE_CHECK(base + HAL_VAR_OFDMTxAgcOffset_A(1));
	byte2 = HAL_POWER_RANGE_CHECK(base + HAL_VAR_OFDMTxAgcOffset_A(2));
	byte3 = HAL_POWER_RANGE_CHECK(base + HAL_VAR_OFDMTxAgcOffset_A(3));
	writeVal = (byte0 << 24) | (byte1 << 16) | (byte2 << 8) | byte3;
	HAL_RTL_W32(REG_BB_TXAGC_A_RATE18_06_N, writeVal);

	// fill 24Mbps ~ 54Mbps
	byte0 = HAL_POWER_RANGE_CHECK(base + HAL_VAR_OFDMTxAgcOffset_A(4));
	byte1 = HAL_POWER_RANGE_CHECK(base + HAL_VAR_OFDMTxAgcOffset_A(5));
	byte2 = HAL_POWER_RANGE_CHECK(base + HAL_VAR_OFDMTxAgcOffset_A(6));
	byte3 = HAL_POWER_RANGE_CHECK(base + HAL_VAR_OFDMTxAgcOffset_A(7));
	writeVal = (byte0 << 24) | (byte1 << 16) | (byte2 << 8) | byte3;
	HAL_RTL_W32(REG_BB_TXAGC_A_RATE54_24_N, writeVal);

    // cal HT rate diff
	base = pwrlevelHT40_1S_A;
	if (HAL_VAR_CurrentChannelBW == HT_CHANNEL_WIDTH_20) {
		offset = (pwrdiffHT20 & 0x0f);
		base = HAL_COUNT_SIGN_OFFSET(base, offset);
	}
#if CFG_HAL_POWER_PERCENT_ADJUSTMENT
	base = HAL_POWER_RANGE_CHECK(base + pwrdiff_percent);
#endif

	// fill MCS0~MCS3
	byte0 = HAL_POWER_RANGE_CHECK(base + HAL_VAR_MCSTxAgcOffset_A(0));
	byte1 = HAL_POWER_RANGE_CHECK(base + HAL_VAR_MCSTxAgcOffset_A(1));
	byte2 = HAL_POWER_RANGE_CHECK(base + HAL_VAR_MCSTxAgcOffset_A(2));
	byte3 = HAL_POWER_RANGE_CHECK(base + HAL_VAR_MCSTxAgcOffset_A(3));
	writeVal = (byte0 << 24) | (byte1 << 16) | (byte2 << 8) | byte3;
	HAL_RTL_W32(REG_BB_TXAGC_A_MCS03_MCS00_N, writeVal);

	// fill MCS4~MCS7
	byte0 = HAL_POWER_RANGE_CHECK(base + HAL_VAR_MCSTxAgcOffset_A(4));
	byte1 = HAL_POWER_RANGE_CHECK(base + HAL_VAR_MCSTxAgcOffset_A(5));
	byte2 = HAL_POWER_RANGE_CHECK(base + HAL_VAR_MCSTxAgcOffset_A(6));
	byte3 = HAL_POWER_RANGE_CHECK(base + HAL_VAR_MCSTxAgcOffset_A(7));
	writeVal = (byte0 << 24) | (byte1 << 16) | (byte2 << 8) | byte3;
	HAL_RTL_W32(REG_BB_TXAGC_A_MCS07_MCS04_N, writeVal);

	offset = (pwrdiffHT40_2S & 0x0f);
	base = HAL_COUNT_SIGN_OFFSET(base, offset);


	//_TXPWR_REDEFINE ?? MCS 8 - 11, shall NOT add power by rate even NOT USB power ??
	byte0 = HAL_POWER_RANGE_CHECK(base + HAL_VAR_MCSTxAgcOffset_A(8));
	byte1 = HAL_POWER_RANGE_CHECK(base + HAL_VAR_MCSTxAgcOffset_A(9));
	byte2 = HAL_POWER_RANGE_CHECK(base + HAL_VAR_MCSTxAgcOffset_A(10));
	byte3 = HAL_POWER_RANGE_CHECK(base + HAL_VAR_MCSTxAgcOffset_A(11));

	writeVal = (byte0 << 24) | (byte1 << 16) | (byte2 << 8) | byte3;

	//DEBUG_INFO("debug e18:%x,%x,[%x,%x,%x,%x],%x\n", offset, base, byte0, byte1, byte2, byte3, writeVal);
	HAL_RTL_W32(REG_BB_TXAGC_A_MCS11_MCS08_N, writeVal);

	byte0 = HAL_POWER_RANGE_CHECK(base + HAL_VAR_MCSTxAgcOffset_A(12));
	byte1 = HAL_POWER_RANGE_CHECK(base + HAL_VAR_MCSTxAgcOffset_A(13));
	byte2 = HAL_POWER_RANGE_CHECK(base + HAL_VAR_MCSTxAgcOffset_A(14));
	byte3 = HAL_POWER_RANGE_CHECK(base + HAL_VAR_MCSTxAgcOffset_A(15));

	writeVal = (byte0<<24) | (byte1<<16) |(byte2<<8) | byte3;
	HAL_RTL_W32(REG_BB_TXAGC_A_MCS15_MCS12_N, writeVal);

	/******************************  PATH B  ******************************/

	if (GetChipIDMIMO88XX(Adapter) == MIMO_2T2R) {

		base = pwrlevelHT40_1S_B;
	offset = ((pwrdiffOFDM & 0xf0) >> 4);
		base = HAL_COUNT_SIGN_OFFSET(base, offset);
#if CFG_HAL_POWER_PERCENT_ADJUSTMENT
		base = HAL_POWER_RANGE_CHECK(base + pwrdiff_percent);
#endif

		byte0 = HAL_POWER_RANGE_CHECK(base + HAL_VAR_OFDMTxAgcOffset_B(0));
		byte1 = HAL_POWER_RANGE_CHECK(base + HAL_VAR_OFDMTxAgcOffset_B(1));
		byte2 = HAL_POWER_RANGE_CHECK(base + HAL_VAR_OFDMTxAgcOffset_B(2));
		byte3 = HAL_POWER_RANGE_CHECK(base + HAL_VAR_OFDMTxAgcOffset_B(3));
		writeVal = (byte0 << 24) | (byte1 << 16) | (byte2 << 8) | byte3;
		HAL_RTL_W32(REG_BB_TXAGC_B_RATE18_06_N, writeVal);

		byte0 = HAL_POWER_RANGE_CHECK(base + HAL_VAR_OFDMTxAgcOffset_B(4));
		byte1 = HAL_POWER_RANGE_CHECK(base + HAL_VAR_OFDMTxAgcOffset_B(5));
		byte2 = HAL_POWER_RANGE_CHECK(base + HAL_VAR_OFDMTxAgcOffset_B(6));
		byte3 = HAL_POWER_RANGE_CHECK(base + HAL_VAR_OFDMTxAgcOffset_B(7));
		writeVal = (byte0 << 24) | (byte1 << 16) | (byte2 << 8) | byte3;
		HAL_RTL_W32(REG_BB_TXAGC_B_RATE54_24_N, writeVal);

		base = pwrlevelHT40_1S_B;
	if (HAL_VAR_CurrentChannelBW == HT_CHANNEL_WIDTH_20) {
			offset = ((pwrdiffHT20 & 0xf0) >> 4);
			base = HAL_COUNT_SIGN_OFFSET(base, offset);
		}
#if CFG_HAL_POWER_PERCENT_ADJUSTMENT
		base = HAL_POWER_RANGE_CHECK(base + pwrdiff_percent);
#endif

		byte0 = HAL_POWER_RANGE_CHECK(base + HAL_VAR_MCSTxAgcOffset_B(0));
		byte1 = HAL_POWER_RANGE_CHECK(base + HAL_VAR_MCSTxAgcOffset_B(1));
		byte2 = HAL_POWER_RANGE_CHECK(base + HAL_VAR_MCSTxAgcOffset_B(2));
		byte3 = HAL_POWER_RANGE_CHECK(base + HAL_VAR_MCSTxAgcOffset_B(3));
		writeVal = (byte0 << 24) | (byte1 << 16) | (byte2 << 8) | byte3;
		HAL_RTL_W32(REG_BB_TXAGC_B_MCS03_MCS00_N, writeVal);

		byte0 = HAL_POWER_RANGE_CHECK(base + HAL_VAR_MCSTxAgcOffset_B(4));
		byte1 = HAL_POWER_RANGE_CHECK(base + HAL_VAR_MCSTxAgcOffset_B(5));
		byte2 = HAL_POWER_RANGE_CHECK(base + HAL_VAR_MCSTxAgcOffset_B(6));
		byte3 = HAL_POWER_RANGE_CHECK(base + HAL_VAR_MCSTxAgcOffset_B(7));
		writeVal = (byte0 << 24) | (byte1 << 16) | (byte2 << 8) | byte3;
		HAL_RTL_W32(REG_BB_TXAGC_B_MCS07_MCS04_N, writeVal);

		offset = ((pwrdiffHT40_2S & 0xf0) >> 4);
		base = HAL_COUNT_SIGN_OFFSET(base, offset);

		byte0 = HAL_POWER_RANGE_CHECK(base + HAL_VAR_MCSTxAgcOffset_B(8));
		byte1 = HAL_POWER_RANGE_CHECK(base + HAL_VAR_MCSTxAgcOffset_B(9));
		byte2 = HAL_POWER_RANGE_CHECK(base + HAL_VAR_MCSTxAgcOffset_B(10));
		byte3 = HAL_POWER_RANGE_CHECK(base + HAL_VAR_MCSTxAgcOffset_B(11));

		writeVal = (byte0 << 24) | (byte1 << 16) | (byte2 << 8) | byte3;
		HAL_RTL_W32(REG_BB_TXAGC_B_MCS11_MCS08_N, writeVal);

		byte0 = HAL_POWER_RANGE_CHECK(base + HAL_VAR_MCSTxAgcOffset_B(12));
		byte1 = HAL_POWER_RANGE_CHECK(base + HAL_VAR_MCSTxAgcOffset_B(13));
		byte2 = HAL_POWER_RANGE_CHECK(base + HAL_VAR_MCSTxAgcOffset_B(14));
		byte3 = HAL_POWER_RANGE_CHECK(base + HAL_VAR_MCSTxAgcOffset_B(15));

		writeVal = (byte0 << 24) | (byte1 << 16) | (byte2 << 8) | byte3;
		HAL_RTL_W32(REG_BB_TXAGC_B_MCS15_MCS12_N, writeVal);
	}
#ifdef TXPWR_LMT
	if (!Adapter->pshare->rf_ft_var.disable_txpwrlmt) {
		if (HAL_check_lmt_valid(Adapter, phy_band)) {
			TxLMT_OFDM_Jaguar_N(Adapter);
		}
	}
#endif // TXPWR_LMT

	return RT_STATUS_SUCCESS;
}

RT_STATUS
PHYSetCCKTxPower88XX_N(
	IN  HAL_PADAPTER    Adapter,
	IN  u1Byte          channel
)
{
	unsigned int writeVal = 0, pwrdiff = 0x06060606;
	u1Byte byte, byte1, byte2;
	u1Byte pwrlevelCCK_A = HAL_VAR_pwrlevelCCK_A(channel-1);
	u1Byte pwrlevelCCK_B = HAL_VAR_pwrlevelCCK_B(channel-1);
#if CFG_HAL_POWER_PERCENT_ADJUSTMENT
	s1Byte pwrdiff_percent = HAL_PwrPercent2PwrLevel(HAL_VAR_power_percent);
#endif
	u1Byte phy_band = 0;

	if (channel > CHANNEL_MAX_NUMBER_2G)
		phy_band = PHY_BAND_5G;
	else
		phy_band = PHY_BAND_2G;	

	if (HAL_cck_pwr_max) {
		byte = HAL_POWER_RANGE_CHECK((char)HAL_cck_pwr_max);
		writeVal = byte;
		Adapter->pshare->rf_ft_var.bcn_pwr_idex=writeVal;
		PHY_SetBBReg_88XX(Adapter, REG_BB_TXAGC_A_CCK1_MCS32_N, 0x0000ff00, writeVal);
		writeVal = (byte << 16) | (byte << 8) | byte;
		PHY_SetBBReg_88XX(Adapter, REG_BB_TXAGC_B_CCK5_1_MCS32_N, 0xffffff00, writeVal);
		writeVal = (byte << 24) | (byte << 16) | (byte << 8) | byte;
		PHY_SetBBReg_88XX(Adapter, REG_BB_TXAGC_A_CCK11_2_B_CCK11_N, 0xffffffff, writeVal);
		return RT_STATUS_SUCCESS;
	}

	if ((pwrlevelCCK_A == 0 && pwrlevelCCK_B == 0)) {
#if CFG_HAL_HIGH_POWER_EXT_PA
		if (HAL_VAR_use_ext_pa)
			byte = HP_CCK_POWER_DEFAULT;
		else
#endif
			byte = 0x24;


#if (!CFG_HAL_ADD_TX_POWER_BY_CMD)
		// use default value
		writeVal = byte;
		PHY_SetBBReg_88XX(Adapter, REG_BB_TXAGC_A_CCK1_MCS32_N, 0x0000ff00, writeVal);
		writeVal = (byte << 16) | (byte << 8) | byte;
		PHY_SetBBReg_88XX(Adapter, REG_BB_TXAGC_B_CCK5_1_MCS32_N, 0xffffff00, writeVal);
		writeVal = (byte << 24) | (byte << 16) | (byte << 8) | byte;
		PHY_SetBBReg_88XX(Adapter, REG_BB_TXAGC_A_CCK11_2_B_CCK11_N, 0xffffffff, writeVal);
#else
		// use cmd value
		pwrlevelCCK_A = pwrlevelCCK_B = byte;
		byte = 0;
		HAL_ASSIGN_TX_POWER_OFFSET(byte, HAL_txPowerPlus_cck_1);

		writeVal = HAL_POWER_RANGE_CHECK(pwrlevelCCK_A + byte);
		PHY_SetBBReg_88XX(Adapter, REG_BB_TXAGC_A_CCK1_MCS32_N, 0x0000ff00, writeVal);

		byte = byte1 = byte2 = 0;
		HAL_ASSIGN_TX_POWER_OFFSET(byte, HAL_txPowerPlus_cck_1);
		HAL_ASSIGN_TX_POWER_OFFSET(byte1, HAL_txPowerPlus_cck_2);
		HAL_ASSIGN_TX_POWER_OFFSET(byte2, HAL_txPowerPlus_cck_5);
		byte  = HAL_POWER_RANGE_CHECK(pwrlevelCCK_B + byte);
		byte1 = HAL_POWER_RANGE_CHECK(pwrlevelCCK_B + byte1);
		byte2 = HAL_POWER_RANGE_CHECK(pwrlevelCCK_B + byte2);

		writeVal = ((byte2 << 16) | (byte1 << 8) | byte);
		PHY_SetBBReg_88XX(Adapter, REG_BB_TXAGC_B_CCK5_1_MCS32_N, 0xffffff00, writeVal);

		byte = byte1 = byte2 = 0;
		HAL_ASSIGN_TX_POWER_OFFSET(byte, HAL_txPowerPlus_cck_2);
		HAL_ASSIGN_TX_POWER_OFFSET(byte1, HAL_txPowerPlus_cck_5);
		HAL_ASSIGN_TX_POWER_OFFSET(byte2, HAL_txPowerPlus_cck_11);
		byte  = HAL_POWER_RANGE_CHECK(pwrlevelCCK_A + byte);
		byte1 = HAL_POWER_RANGE_CHECK(pwrlevelCCK_A + byte1);
		byte2 = HAL_POWER_RANGE_CHECK(pwrlevelCCK_A + byte2);
		writeVal = ((byte2 << 24) | (byte1 << 16) | (byte << 8) | byte2);
		PHY_SetBBReg_88XX(Adapter, REG_BB_TXAGC_A_CCK11_2_B_CCK11_N, 0xffffffff, writeVal);

#ifdef TXPWR_LMT
		if (!Adapter->pshare->rf_ft_var.disable_txpwrlmt) {
			if (HAL_check_lmt_valid(Adapter, phy_band)) {
				TxLMT_CCK_Jaguar_N(Adapter);
			}
		}
#endif // TXPWR_LMT

#endif
		{
			int cck1a = PHY_QueryBBReg_88XX(Adapter, REG_BB_TXAGC_A_CCK1_MCS32_N, 0x0000ff00);
			int cck1b = PHY_QueryBBReg_88XX(Adapter, REG_BB_TXAGC_B_CCK5_1_MCS32_N, 0x0000ff00);
			if(cck1a>Adapter->pshare->rf_ft_var.bcn_pwr_max) {
				cck1a = Adapter->pshare->rf_ft_var.bcn_pwr_max;
				PHY_SetBBReg_88XX(Adapter, REG_BB_TXAGC_A_CCK1_MCS32_N, 0x0000ff00, cck1a);
			}
			if(cck1b>Adapter->pshare->rf_ft_var.bcn_pwr_max) {
				cck1b = Adapter->pshare->rf_ft_var.bcn_pwr_max;
				PHY_SetBBReg_88XX(Adapter, REG_BB_TXAGC_B_CCK5_1_MCS32_N, 0x0000ff00, cck1b);
			}				
			Adapter->pshare->rf_ft_var.bcn_pwr_idex= max(cck1a,cck1b);
		}
		return RT_STATUS_SUCCESS; // use default
	}
#if CFG_HAL_POWER_PERCENT_ADJUSTMENT
	pwrlevelCCK_A = HAL_POWER_RANGE_CHECK(pwrlevelCCK_A + pwrdiff_percent);
	pwrlevelCCK_B = HAL_POWER_RANGE_CHECK(pwrlevelCCK_B + pwrdiff_percent);
#endif

	// use flash value to fill

	writeVal = HAL_POWER_RANGE_CHECK(pwrlevelCCK_A + HAL_CCKTxAgc_A(3) + Adapter->pmib->dot11RFEntry.add_cck1M_pwr);
	PHY_SetBBReg_88XX(Adapter, REG_BB_TXAGC_A_CCK1_MCS32_N, 0x0000ff00, writeVal);
	writeVal = (HAL_POWER_RANGE_CHECK(pwrlevelCCK_B + HAL_CCKTxAgc_B(1)) << 16) |
			   (HAL_POWER_RANGE_CHECK(pwrlevelCCK_B + HAL_CCKTxAgc_B(2)) << 8)  |
			   HAL_POWER_RANGE_CHECK(pwrlevelCCK_B + HAL_CCKTxAgc_B(3) + Adapter->pmib->dot11RFEntry.add_cck1M_pwr);
	PHY_SetBBReg_88XX(Adapter, REG_BB_TXAGC_B_CCK5_1_MCS32_N, 0xffffff00, writeVal);
	writeVal = (HAL_POWER_RANGE_CHECK(pwrlevelCCK_A + HAL_CCKTxAgc_A(0)) << 24) |
			   (HAL_POWER_RANGE_CHECK(pwrlevelCCK_A + HAL_CCKTxAgc_A(1)) << 16) |
			   (HAL_POWER_RANGE_CHECK(pwrlevelCCK_A + HAL_CCKTxAgc_A(2)) << 8)  |
			   HAL_POWER_RANGE_CHECK(pwrlevelCCK_B + HAL_CCKTxAgc_B(0));
	PHY_SetBBReg_88XX(Adapter, REG_BB_TXAGC_A_CCK11_2_B_CCK11_N, 0xffffffff, writeVal);

#ifdef TXPWR_LMT
	if (!Adapter->pshare->rf_ft_var.disable_txpwrlmt) {
		if (HAL_check_lmt_valid(Adapter, phy_band)) {
			TxLMT_CCK_Jaguar_N(Adapter);
		}
	}
#endif // TXPWR_LMT

	{
		int cck1a = PHY_QueryBBReg_88XX(Adapter, REG_BB_TXAGC_A_CCK1_MCS32_N, 0x0000ff00);
		int cck1b = PHY_QueryBBReg_88XX(Adapter, REG_BB_TXAGC_B_CCK5_1_MCS32_N, 0x0000ff00);
		if(cck1a>Adapter->pshare->rf_ft_var.bcn_pwr_max) {
			cck1a = Adapter->pshare->rf_ft_var.bcn_pwr_max;
			PHY_SetBBReg_88XX(Adapter, REG_BB_TXAGC_A_CCK1_MCS32_N, 0x0000ff00, cck1a);
		}
		if(cck1b>Adapter->pshare->rf_ft_var.bcn_pwr_max) {
			cck1b = Adapter->pshare->rf_ft_var.bcn_pwr_max;
			PHY_SetBBReg_88XX(Adapter, REG_BB_TXAGC_B_CCK5_1_MCS32_N, 0x0000ff00, cck1b);
		}				
		Adapter->pshare->rf_ft_var.bcn_pwr_idex= max(cck1a,cck1b);
	}

	return RT_STATUS_SUCCESS;
}

#if 0
void
printk92E(IN  HAL_PADAPTER    Adapter)
{
	printk("0x440 = %x\n", HAL_RTL_R32(0x440));

	printk("0x800  = %x\n", HAL_RTL_R32(0x800));
	printk("0xa00  = %x\n", HAL_RTL_R32(0xa00));
	printk("0xd00  = %x\n", HAL_RTL_R32(0xd00));
	printk("0x818  = %x\n", HAL_RTL_R32(0x818));
	printk("0x800  = %x\n", HAL_RTL_R32(0x800));

	printk("RF TRX_BW RF PathA 0x18=%x \n", PHY_QueryRFReg_88XX_N(Adapter, 0, 0x18, 0xffffffff));

	printk("RF TRX_BW RF PathB 0x18=%x \n", PHY_QueryRFReg_88XX_N(Adapter, 1, 0x18, 0xffffffff));

	printk("RF TRX_BW RF PathA 0x18=%x \n", PHY_QueryRFReg(Adapter, 0, 0x18, 0xffffffff, 1));

	printk("RF TRX_BW RF PathB 0x18=%x \n", PHY_QueryRFReg(Adapter, 1, 0x18, 0xffffffff, 1));

}
#endif

void
SwBWMode88XX_N(
	IN  HAL_PADAPTER    Adapter,
	IN  u4Byte          bandwidth,
	IN  s4Byte          offset
)
{
	u4Byte eRFPath, curMaxRFPath, val, rrSR = 0;
	u1Byte nCur40MhzPrimeSC, regDataSC;
	unsigned int dwTmp1 = 0, dwTmp2 = 0, dwTmp3 = 0;

	RT_TRACE(COMP_RF, DBG_TRACE, ("SwBWMode(): Switch to %s bandwidth\n", bandwidth ? "40MHz" : "20MHz"));

	curMaxRFPath = RF88XX_PATH_MAX;

	if (offset == 1)
		nCur40MhzPrimeSC = 2;
	else
		nCur40MhzPrimeSC = 1;

	//3 <1> Set MAC register
	rrSR = HAL_RTL_R32(REG_RRSR);
	rrSR &= ~(BIT(21) | BIT(22));

	regDataSC = HAL_RTL_R8(0x483);

	if (IS_C_CUT_8192E(Adapter) || IS_D_CUT_8192E(Adapter)) {
		dwTmp1 = HAL_RTL_R32(0x800);
		dwTmp2 = HAL_RTL_R32(0xce4);
		dwTmp3 = HAL_RTL_R32(0xc30);
		dwTmp1 &= ~(BIT(10) | BIT(9) | BIT(8) | BIT(13) | BIT(12) | BIT(14));
		dwTmp2 &= ~(BIT(31) | BIT(30));
		dwTmp3 &= ~(BIT(10) | BIT(9) | BIT(8));
	}
	switch (bandwidth) {
	case HT_CHANNEL_WIDTH_20:
		if (IS_C_CUT_8192E(Adapter) || IS_D_CUT_8192E(Adapter)) {
			dwTmp3 |= (BIT(10));
			HAL_RTL_W32(0xc30, dwTmp3);
		}
		break;
	case HT_CHANNEL_WIDTH_20_40:
		HAL_RTL_W32(REG_RRSR, rrSR);

		regDataSC &= 0xf0;
		regDataSC |= Adapter->pshare->txsc_20_92e;
		HAL_RTL_W8(0x483, regDataSC);

		// Let 812cd_rx, re-assign value
		if (HAL_VAR_IS_40M_BW) {
			HAL_VAR_REG_RRSR_2 = 0;
			HAL_VAR_REG_81B = 0;
		}
		if (IS_C_CUT_8192E(Adapter) || IS_D_CUT_8192E(Adapter)) {
			dwTmp3 |= (BIT(10));
			HAL_RTL_W32(0xc30, dwTmp3);
		}
		break;
		if (IS_C_CUT_8192E(Adapter)) {
		case HT_CHANNEL_WIDTH_10:
			dwTmp1 |= (BIT(10) | BIT(8) | BIT(12) | BIT(14));
			dwTmp2 |= (BIT(31));
			dwTmp3 |= (BIT(10) | BIT(9));
			HAL_RTL_W32(0x800, dwTmp1);
			HAL_RTL_W32(0xce4, dwTmp2);
			HAL_RTL_W32(0xc30, dwTmp3);
			break;
		case HT_CHANNEL_WIDTH_5:
			dwTmp1 |= (BIT(10) | BIT(8) | BIT(14));
			dwTmp2 |= (BIT(30));
			dwTmp3 |= (BIT(10) | BIT(9) | BIT(8));
			HAL_RTL_W32(0x800, dwTmp1);
			HAL_RTL_W32(0xce4, dwTmp2);
			HAL_RTL_W32(0xc30, dwTmp3);
			break;
		}
	default:
		RT_TRACE(COMP_RF, DBG_LOUD, ("SwBWMode(): bandwidth mode error!\n"));
		return;
		break;
	}

	//3 <2> Set PHY related register
	switch (bandwidth) {
	case HT_CHANNEL_WIDTH_20:
		PHY_SetBBReg_88XX(Adapter, REG_BB_FPGA0_RFMOD_N, BIT_MASK_BB_RFMOD_N_N, 0x0);
		PHY_SetBBReg_88XX(Adapter, REG_BB_FPGA1_RFMOD_N, BIT_MASK_BB_RFMOD_N_N, 0x0);
		val = 3;
		break;
	case HT_CHANNEL_WIDTH_20_40:
		PHY_SetBBReg_88XX(Adapter, REG_BB_FPGA0_RFMOD_N, BIT_MASK_BB_RFMOD_N_N, 0x1);
		PHY_SetBBReg_88XX(Adapter, REG_BB_FPGA1_RFMOD_N, BIT_MASK_BB_RFMOD_N_N, 0x1);
		// Set Control channel to upper or lower. These settings are required only for 40MHz
		PHY_SetBBReg_88XX(Adapter, REG_BB_CCK0_SYSTEM_N, BIT_MASK_BB_CCKSIDEBAND_N, (nCur40MhzPrimeSC >> 1));
		PHY_SetBBReg_88XX(Adapter, REG_BB_OFDM1_LSTF_N, 0xC00, nCur40MhzPrimeSC);
		val = 1;

		PHY_SetBBReg_88XX(Adapter, REG_BB_PRIMESC_N, (BIT(26) | BIT(27)), (nCur40MhzPrimeSC == 2) ? 1 : 2);
		break;
	default:
		RT_TRACE(COMP_RF, DBG_LOUD, ("SwBWMode(): bandwidth mode error! %d\n", __LINE__));
		return;
		break;
	}

	for (eRFPath = RF88XX_PATH_A; eRFPath < curMaxRFPath; eRFPath++)
		PHY_SetRFReg_88XX_N(Adapter, eRFPath, REG_RF_RFCHANNEL_N, (BIT(11) | BIT(10)), val);
}




/* ----------------------------------
    Power Tracking Realted functions
--------------------------------------*/
#if 0
void
TXPowerTracking_ThermalMeter_88XX
(
	IN  HAL_PADAPTER    Adapter
)
{
	u1Byte          ThermalValue = 0, delta, channel, is_decrease, rf_mimo_mode;
	u1Byte			ThermalValue_AVG_count = 0;
	u1Byte          OFDM_min_index = 10; //OFDM BB Swing should be less than +2.5dB, which is required by Arthur
	s1Byte			OFDM_index[2], index ;
	u4Byte			ThermalValue_AVG = 0;
	u4Byte			i = 0, j = 0, rf;
	s4Byte	value32, CCK_index, ele_A, ele_D, ele_C, X, Y;
	PRTL8192CD_PRIV priv = Adapter;

	rf_mimo_mode = GetChipIDMIMO88XX(Adapter);

#ifdef CFG_HAL_MP_TEST
	if ((HAL_OPMODE & WIFI_MP_STATE) || HAL_VAR_MP_SPECIFIC) {
		channel = HAL_VAR_WORKING_CHANNEL;
		//printk("MP start , channel = %x \n",channel);
		if (HAL_VAR_MP_TXPWR_TRACKING == FALSE)
			return;
	} else
#endif
	{
		channel = HAL_VAR_DOT11CHANNEL;
	}

	if (HAL_VAR_POWER_TRACKING_ON_88XX == 0) {
		HAL_VAR_POWER_TRACKING_ON_88XX = 1;
		RT_TRACE(COMP_POWER_TRACKING, DBG_LOUD, ("TXPowerTracking_ThermalMeter_88XX: First Timer set 0x42\n"));
		//printk("First Timer set 0x42\n");
		PHY_SetRFReg_88XX_N(Adapter, RF88XX_PATH_A, REG_RF_CSI_MASK_N, (BIT(17) | BIT(16)), 0x03);
		return;
	}
	//else
	{
		//	HAL_VAR_POWER_TRACKING_ON_88XX = 0;
		ThermalValue = (unsigned char)PHY_QueryRFReg_88XX_N(Adapter, RF88XX_PATH_A, REG_RF_CSI_MASK_N, 0xfc00);	//0x42: RF Reg[15:10] 88E
		RT_TRACE(COMP_POWER_TRACKING, DBG_LOUD, ("\nReadback Thermal Meter = 0x%x pre thermal meter 0x%x EEPROMthermalmeter 0x%x\n", ThermalValue, HAL_VAR_THERMALVALUE, HAL_VAR_THER));
		//printk("\nReadback Thermal Meter = %x \n",ThermalValue);
	}

	switch (rf_mimo_mode) {
	case MIMO_1T1R:
		rf = 1;
		//printk("%s:%d get_rf_mimo_mode =1 !\n", __FUNCTION__, __LINE__);
		break;
	case MIMO_2T2R:
		rf = 2;
		//printk("%s:%d get_rf_mimo_mode =2 !\n", __FUNCTION__, __LINE__);
		break;
	default:
		//printk("%s:%d get_rf_mimo_mode error!\n", __FUNCTION__, __LINE__);
		break;
	}

	//Query OFDM path A default setting 	Bit[31:21]
	ele_D = PHY_QueryBBReg_88XX(Adapter, REG_BB_OFDM0_XATXIQIMBALANC_N, BIT_MASK_SET_MASKOFDM_D_COMMON);
	//printk("0xc80 ele_D = %x \n",ele_D);
	// TODO : need to check mask
	for (i = 0; i < OFDM_TABLE_SIZE_92E; i++) {
		if (ele_D == (OFDMSwingTable_92E[i] >> 22)) {
			OFDM_index[0] = (unsigned char)i;
			RT_TRACE(COMP_POWER_TRACKING, DBG_LOUD, ("PathA 0xC80[31:22] = 0x%x, OFDM_index=%d\n", ele_D, OFDM_index[0]));
			break;
		}
	}

	//Query OFDM path B default setting
	if (rf_mimo_mode == MIMO_2T2R) {
		ele_D = PHY_QueryBBReg_88XX(Adapter, REG_BB_OFDM0_XBTXIQIMBALANCE_N, BIT_MASK_SET_MASKOFDM_D_COMMON);
		for (i = 0; i < OFDM_TABLE_SIZE_92E; i++) {
			if (ele_D == (OFDMSwingTable_92E[i] >> 22)) {
				OFDM_index[1] = (unsigned char)i;
				RT_TRACE(COMP_POWER_TRACKING, DBG_LOUD, ("PathB 0xC88[31:22] = 0x%x, OFDM_index=%d\n", ele_D, OFDM_index[1]));
				//printk("PathB 0xe1c[31:21] = 0x%x, OFDM_index=%d\n", ele_D, OFDM_index[1]);
				break;
			}
		}
	}

	/* Initialize */
	// if first time use flash default PG value
	if (!HAL_VAR_THERMALVALUE) {
		HAL_VAR_THERMALVALUE = HAL_VAR_THER;
		//printk("HAL_VAR_THERMALVALUE = %x \n",HAL_VAR_THERMALVALUE);
	}

	/* calculate average thermal meter */
	{
		HAL_VAR_THERMALVALUE_AVG_88XX(HAL_VAR_THERMALVALUE_AVG_INDEX_88XX) = ThermalValue;
		HAL_VAR_THERMALVALUE_AVG_INDEX_88XX++;
		if (HAL_VAR_THERMALVALUE_AVG_INDEX_88XX == AVG_THERMAL_NUM_88XX)
			HAL_VAR_THERMALVALUE_AVG_INDEX_88XX = 0;

		for (i = 0; i < AVG_THERMAL_NUM_88XX; i++) {
			if (HAL_VAR_THERMALVALUE_AVG_88XX(i)) {
				ThermalValue_AVG += HAL_VAR_THERMALVALUE_AVG_88XX(i);
				ThermalValue_AVG_count++;
			}
		}

		if (ThermalValue_AVG_count) {
			ThermalValue = (unsigned char)(ThermalValue_AVG / ThermalValue_AVG_count);
			//printk("AVG Thermal Meter = 0x%x \n", ThermalValue);
		}
	}

	if (ThermalValue != HAL_VAR_THERMALVALUE) {
		RT_TRACE(COMP_POWER_TRACKING, DBG_LOUD, ("\n******** START:%s() ********\n", __FUNCTION__));
		RT_TRACE(COMP_POWER_TRACKING, DBG_LOUD, ("\nReadback Thermal Meter = 0x%x pre thermal meter 0x%x EEPROMthermalmeter 0x%x\n", ThermalValue, HAL_VAR_THERMALVALUE, HAL_VAR_THER));

		delta = HAL_RTL_ABS(ThermalValue, HAL_VAR_THER);
		is_decrease = ((ThermalValue < HAL_VAR_THER) ? 1 : 0);

		//printk("delta = %x \n",delta);
		//printk("is_decrease = %x \n",is_decrease);

		if (HAL_VAR_PHYBANDSELECT == PHY_BAND_2G) {
#ifdef  CFG_TRACKING_TABLE_FILE
			if (HAL_VAR_PWR_TRACK_FILE) {

				if (is_decrease) {
					for (i = 0; i < rf; i++) {
						OFDM_index[i] = HAL_VAR_OFDM_BASE_INDEX(i) + get_tx_tracking_index_88XX(Adapter, channel, i, delta, is_decrease, 0);
						OFDM_index[i] = ((OFDM_index[i] > (OFDM_TABLE_SIZE_92E - 1)) ? (OFDM_TABLE_SIZE_92E - 1) : OFDM_index[i]);
						RT_TRACE(COMP_POWER_TRACKING, DBG_LOUD, (">>> Decrese power ---> new OFDM_INDEX:%d (%d + %d)\n", OFDM_index[i], HAL_VAR_OFDM_BASE_INDEX(i), get_tx_tracking_index_88XX(Adapter, channel, i, delta, is_decrease, 0)));
						//printk(">>> decrese power ---> new OFDM_INDEX:%d (%d + %d)\n", OFDM_index[i], HAL_VAR_OFDM_BASE_INDEX(i), get_tx_tracking_index_88XX(Adapter, channel, i, delta, is_decrease, 0));
						CCK_index = HAL_VAR_CCK_BASE_INDEX + get_tx_tracking_index_88XX(Adapter, channel, i, delta, is_decrease, 1);
						RT_TRACE(COMP_POWER_TRACKING, DBG_LOUD, (">>> Decrese power ---> new CCK_INDEX:%d (%d + %d)\n",  CCK_index, HAL_VAR_CCK_BASE_INDEX, get_tx_tracking_index_88XX(Adapter, channel, i, delta, is_decrease, 1)));
						CCK_index = ((CCK_index > (CCK_TABLE_SIZE_92E - 1)) ? (CCK_TABLE_SIZE_92E - 1) : CCK_index);
					}
				} else {
					for (i = 0; i < rf; i++) {
						OFDM_index[i] = HAL_VAR_OFDM_BASE_INDEX(i) - get_tx_tracking_index_88XX(Adapter, channel, i, delta, is_decrease, 0);
						//printk("HAL_VAR_OFDM_BASE_INDEX(i) = %x OFDM_index[i] =%x \n",HAL_VAR_OFDM_BASE_INDEX(i),OFDM_index[i]);
						OFDM_index[i] = ((OFDM_index[i] < OFDM_min_index) ?  OFDM_min_index : OFDM_index[i]);
						RT_TRACE(COMP_POWER_TRACKING, DBG_LOUD, (">>> Increse power ---> new OFDM_INDEX:%d (%d - %d)\n", OFDM_index[i], HAL_VAR_OFDM_BASE_INDEX(i), get_tx_tracking_index_88XX(Adapter, channel, i, delta, is_decrease, 0)));
						//printk(">>> increse power ---> new OFDM_INDEX:%d (%d - %d)\n", OFDM_index[i], HAL_VAR_OFDM_BASE_INDEX(i), get_tx_tracking_index_88XX(Adapter, channel, i, delta, is_decrease, 0));
						CCK_index = HAL_VAR_CCK_BASE_INDEX - get_tx_tracking_index_88XX(Adapter, channel, i, delta, is_decrease, 1);
						RT_TRACE(COMP_POWER_TRACKING, DBG_LOUD, (">>> Increse power ---> new CCK_INDEX:%d (%d - %d)\n", CCK_index, HAL_VAR_CCK_BASE_INDEX, get_tx_tracking_index_88XX(Adapter, channel, i, delta, is_decrease, 1)));
						//printk("HAL_VAR_CCK_BASE_INDEX = %x CCK_index =%x \n",HAL_VAR_CCK_BASE_INDEX,CCK_index);
						CCK_index = ((CCK_index < 0 ) ? 0 : CCK_index);
						//printk("HAL_VAR_CCK_BASE_INDEX = %x CCK_index =%x \n",HAL_VAR_CCK_BASE_INDEX,CCK_index);
					}
				}
			}
#endif //CFG_TRACKING_TABLE_FILE
			RT_TRACE(COMP_POWER_TRACKING, DBG_LOUD, ("OFDMSwingTable_92E[(unsigned int)OFDM_index[0]] = %x \n", OFDMSwingTable_92E[(unsigned int)OFDM_index[0]]));
			RT_TRACE(COMP_POWER_TRACKING, DBG_LOUD, ("OFDMSwingTable_92E[(unsigned int)OFDM_index[1]] = %x \n", OFDMSwingTable_92E[(unsigned int)OFDM_index[1]]));

			//Adujst OFDM Ant_A according to IQK result
			ele_D = (OFDMSwingTable_92E[(unsigned int)OFDM_index[0]] & 0xFFC00000) >> 22;
			X = Adapter->pshare->RegE94;
			Y = Adapter->pshare->RegE9C;

			if (X != 0) {
				if ((X & 0x00000200) != 0)
					X = X | 0xFFFFFC00;
				ele_A = ((X * ele_D) >> 8) & 0x000003FF;

				//new element C = element D x Y
				if ((Y & 0x00000200) != 0)
					Y = Y | 0xFFFFFC00;
				ele_C = ((Y * ele_D) >> 8) & 0x000003FF;

				//wirte new elements A, C, D to regC80 and regC94, element B is always 0
				value32 = (ele_D << 22) | ((ele_C & 0x3F) << 16) | ele_A;
				PHY_SetBBReg(Adapter, REG_BB_OFDM0_XATXIQIMBALANC_N, BIT_MASK_SET_MASKDWORD_COMMON, value32);

				value32 = (ele_C & 0x000003C0) >> 6;
				PHY_SetBBReg(Adapter, REG_BB_OFDM0_XCTXAFE_N, BIT_MASK_SET_MASKH4BITS_COMMON, value32);

				value32 = ((X * ele_D) >> 7) & 0x01;
				PHY_SetBBReg(Adapter, REG_BB_OFDM0_ECCATHRESHOLD_N, BIT(24), value32);

			} else {
				PHY_SetBBReg(Adapter, REG_BB_OFDM0_XATXIQIMBALANC_N, BIT_MASK_SET_MASKDWORD_COMMON, OFDMSwingTable_92E[(unsigned int)OFDM_index[0]]);
				PHY_SetBBReg(Adapter, REG_BB_OFDM0_XCTXAFE_N, BIT_MASK_SET_MASKH4BITS_COMMON, 0x00);
				PHY_SetBBReg(Adapter, REG_BB_OFDM0_ECCATHRESHOLD_N, BIT(24), 0x00);
			}

			set_CCK_swing_index_88XX(Adapter, CCK_index);

			if (rf == 2) {
				ele_D = (OFDMSwingTable_92E[(unsigned int)OFDM_index[1]] & 0xFFC00000) >> 22;
				X = Adapter->pshare->RegEB4;
				Y = Adapter->pshare->RegEBC;

				if (X != 0) {
					if ((X & 0x00000200) != 0)	//consider minus
						X = X | 0xFFFFFC00;
					ele_A = ((X * ele_D) >> 8) & 0x000003FF;

					//new element C = element D x Y
					if ((Y & 0x00000200) != 0)
						Y = Y | 0xFFFFFC00;
					ele_C = ((Y * ele_D) >> 8) & 0x00003FF;

					//wirte new elements A, C, D to regC88 and regC9C, element B is always 0
					value32 = (ele_D << 22) | ((ele_C & 0x3F) << 16) | ele_A;
					PHY_SetBBReg(Adapter, REG_BB_OFDM0_XBTXIQIMBALANCE_N, BIT_MASK_SET_MASKDWORD_COMMON, value32);

					value32 = (ele_C & 0x000003C0) >> 6;
					PHY_SetBBReg(Adapter, REG_BB_OFDM0_XDTXAFE_N, BIT_MASK_SET_MASKH4BITS_COMMON, value32);

					value32 = ((X * ele_D) >> 7) & 0x01;
					PHY_SetBBReg(Adapter, REG_BB_OFDM0_ECCATHRESHOLD_N, BIT(28), value32);

				} else {
					PHY_SetBBReg(Adapter, REG_BB_OFDM0_XBTXIQIMBALANCE_N, BIT_MASK_SET_MASKDWORD_COMMON, OFDMSwingTable_92E[(unsigned int)OFDM_index[1]]);
					PHY_SetBBReg(Adapter, REG_BB_OFDM0_XDTXAFE_N, BIT_MASK_SET_MASKH4BITS_COMMON, 0x00);
					PHY_SetBBReg(Adapter, REG_BB_OFDM0_ECCATHRESHOLD_N, BIT(28), 0x00);
				}

			}

			RT_TRACE(COMP_POWER_TRACKING, DBG_LOUD, ("0xc80 = 0x%x \n", PHY_QueryBBReg_88XX(Adapter, REG_BB_OFDM0_XATXIQIMBALANC_N, BIT_MASK_SET_MASKDWORD_COMMON)));
			RT_TRACE(COMP_POWER_TRACKING, DBG_LOUD, ("0xc88 = 0x%x \n", PHY_QueryBBReg_88XX(Adapter, REG_BB_OFDM0_XBTXIQIMBALANCE_N, BIT_MASK_SET_MASKDWORD_COMMON)));
			set_CCK_swing_index_88XX(Adapter, CCK_index);

			if (delta > 3) {
#ifdef MP_TEST
				if (priv->pshare->rf_ft_var.mp_specific) {
					if ((OPMODE & WIFI_MP_CTX_BACKGROUND) && !(OPMODE & WIFI_MP_CTX_PACKET))
						printk("NOT do IQK during ctx !!!! \n");
					else
						PHY_IQCalibrate_8192E(&(priv->pshare->_dmODM), false);
				} else
#endif
					PHY_IQCalibrate_8192E(&(priv->pshare->_dmODM), false);
			}
		} else {
			// TODO 5G setting
		}

		//update thermal meter value
		HAL_VAR_THERMALVALUE = ThermalValue;
		for (i = 0 ; i < rf ; i++)
			HAL_VAR_OFDM_INDEX(i) = OFDM_index[i];
		HAL_VAR_CCK_INDEX = CCK_index;

		RT_TRACE(COMP_POWER_TRACKING, DBG_LOUD, ("\n******** END:%s() ********\n", __FUNCTION__));
	}
}


void
set_CCK_swing_index_88XX(
	IN  HAL_PADAPTER    Adapter,
	IN  s2Byte          CCK_index
)
{
	u2Byte channel;

#if CFG_HAL_DISABLE_BB_RF
	return;
#endif //CFG_HAL_DISABLE_BB_RF

#ifdef CFG_HAL_MP_TEST
	if ((HAL_OPMODE & WIFI_MP_STATE) || HAL_VAR_MP_SPECIFIC)
		channel = HAL_VAR_WORKING_CHANNEL;
	else
#endif
		channel = HAL_VAR_DOT11CHANNEL;

	if (channel != 14) {
		HAL_RTL_W8( 0xa22, CCKSwingTable_Ch1_Ch13_92E[CCK_index][0]);
		HAL_RTL_W8( 0xa23, CCKSwingTable_Ch1_Ch13_92E[CCK_index][1]);
		HAL_RTL_W8( 0xa24, CCKSwingTable_Ch1_Ch13_92E[CCK_index][2]);
		HAL_RTL_W8( 0xa25, CCKSwingTable_Ch1_Ch13_92E[CCK_index][3]);
		HAL_RTL_W8( 0xa26, CCKSwingTable_Ch1_Ch13_92E[CCK_index][4]);
		HAL_RTL_W8( 0xa27, CCKSwingTable_Ch1_Ch13_92E[CCK_index][5]);
		HAL_RTL_W8( 0xa28, CCKSwingTable_Ch1_Ch13_92E[CCK_index][6]);
		HAL_RTL_W8( 0xa29, CCKSwingTable_Ch1_Ch13_92E[CCK_index][7]);
	} else {
		HAL_RTL_W8( 0xa22, CCKSwingTable_Ch14_92E[CCK_index][0]);
		HAL_RTL_W8( 0xa23, CCKSwingTable_Ch14_92E[CCK_index][1]);
		HAL_RTL_W8( 0xa24, CCKSwingTable_Ch14_92E[CCK_index][2]);
		HAL_RTL_W8( 0xa25, CCKSwingTable_Ch14_92E[CCK_index][3]);
		HAL_RTL_W8( 0xa26, CCKSwingTable_Ch14_92E[CCK_index][4]);
		HAL_RTL_W8( 0xa27, CCKSwingTable_Ch14_92E[CCK_index][5]);
		HAL_RTL_W8( 0xa28, CCKSwingTable_Ch14_92E[CCK_index][6]);
		HAL_RTL_W8( 0xa29, CCKSwingTable_Ch14_92E[CCK_index][7]);
	}

	RT_TRACE(COMP_POWER_TRACKING, DBG_LOUD,
			 ("0xa22 ~ 0xa29 = {0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x,}\n",
			  HAL_RTL_R8(0xa22),
			  HAL_RTL_R8(0xa23),
			  HAL_RTL_R8(0xa24),
			  HAL_RTL_R8(0xa25),
			  HAL_RTL_R8(0xa26),
			  HAL_RTL_R8(0xa27),
			  HAL_RTL_R8(0xa28),
			  HAL_RTL_R8(0xa29)
			 ));
}


s4Byte
get_tx_tracking_index_88XX(
	IN  HAL_PADAPTER    Adapter,
	IN  s4Byte  channel,
	IN  s4Byte  i,
	IN  s4Byte  delta,
	IN  s4Byte  is_decrease,
	IN  s4Byte  is_CCK
)
{
	s4Byte index = 0;

	if (delta == 0)
		return 0;

	if (delta > index_mapping_NUM_MAX)
		delta = index_mapping_NUM_MAX;

	//printk("\n_eric_tracking +++ channel = %d, i = %d, delta = %d, is_decrease = %d, is_CCK = %d\n", channel, i, delta, is_decrease, is_CCK);

	// note, we transfer the table
	//
	delta = delta - 1;

	if (channel > 14) {
		if (channel <= 99) {
			index = HAL_VAR_TXPWR_TRACKING_5GL(((i * 2) + is_decrease), delta);
		} else if (channel <= 140) {
			index = HAL_VAR_TXPWR_TRACKING_5GM(((i * 2) + is_decrease), delta);
		} else {
			index = HAL_VAR_TXPWR_TRACKING_5GH(((i * 2) + is_decrease), delta);
		}
	} else {
		if (is_CCK) {
			index = HAL_VAR_TXPWR_TRACKING_2G_CCK(((i * 2) + is_decrease), delta);
		} else {
			index = HAL_VAR_TXPWR_TRACKING_2G_OFDM(((i * 2) + is_decrease), delta);
		}
	}

	//printk("check Table index  = %d\n", index);

	return index;

}
#endif
#endif //#if IS_RTL88XX_N


u4Byte
phy_CalculateBitShift_88XX(
	IN u4Byte BitMask)
{
	u4Byte i;

	for (i = 0; i <= 31; i++) {
		if (((BitMask >> i) & 0x1) == 1)
			break;
	}

	return (i);
}



u4Byte
PHY_QueryBBReg_88XX(
	IN  HAL_PADAPTER    Adapter,
	IN  u4Byte          RegAddr,
	IN  u4Byte          BitMask
)
{
	u4Byte ReturnValue = 0;
	u4Byte OriginalValue;
	u4Byte BitShift;

#if CFG_HAL_DISABLE_BB_RF
	return 0;
#endif //CFG_HAL_DISABLE_BB_RF

	OriginalValue = HAL_RTL_R32(RegAddr);
	BitShift = phy_CalculateBitShift_88XX(BitMask);
	ReturnValue = (OriginalValue & BitMask) >> BitShift;

	return (ReturnValue);
}


void
PHY_SetBBReg_88XX(
	IN  HAL_PADAPTER    Adapter,
	IN  u4Byte          RegAddr,
	IN  u4Byte          BitMask,
	IN  u4Byte          Data
)
{
	u4Byte OriginalValue, BitShift, NewValue;

#if CFG_HAL_DISABLE_BB_RF
	return;
#endif //CFG_HAL_DISABLE_BB_RF

	if (BitMask != BIT_MASK_SET_MASKDWORD_COMMON) {
		//if not "double word" write

		OriginalValue = HAL_RTL_R32(RegAddr);
		BitShift = phy_CalculateBitShift_88XX(BitMask);
		NewValue = ((OriginalValue & (~BitMask)) | (Data << BitShift));
		HAL_RTL_W32(RegAddr, NewValue);
	} else
		HAL_RTL_W32(RegAddr, Data);

}


#if (IS_RTL8192E_SERIES || IS_RTL8881A_SERIES)
BOOLEAN
IsBBRegRange88XX
(
	IN  HAL_PADAPTER                Adapter,
	IN  u4Byte                      RegAddr
)
{
	if ((RegAddr >= 0x800) && (RegAddr <= 0xFFF)) {
		return _TRUE;
	} else {
		return _FALSE;
	}
}
#endif //(IS_RTL8192E_SERIES || IS_RTL8881A_SERIES)

#if (IS_RTL8814A_SERIES || IS_RTL8197F_SERIES || IS_RTL8822B_SERIES)
BOOLEAN
IsBBRegRange88XX_V1
(
	IN  HAL_PADAPTER                Adapter,
	IN  u4Byte                      RegAddr
)
{
	if (((RegAddr >= 0x800) && (RegAddr <= 0xFFF)) ||
			((RegAddr >= 0x1800) && (RegAddr <= 0x1FFF)) ) {
		return _TRUE;
	} else {
		return _FALSE;
	}
}
#endif //(IS_RTL8814A_SERIES || IS_RTL8197F_SERIES || IS_RTL8822B_SERIES)








