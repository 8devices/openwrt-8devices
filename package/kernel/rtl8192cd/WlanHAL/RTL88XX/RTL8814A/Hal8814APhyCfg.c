/*++
Copyright (c) Realtek Semiconductor Corp. All rights reserved.

Module Name:
	Hal8814APhyCfg.c
	
Abstract:
	Defined HAL 8814A PHY BB setting functions
	    
Major Change History:
	When       Who               What
	---------- ---------------   -------------------------------
	2013-05-28 Filen              Create.	
--*/
#if !defined(__ECOS) && !defined(CPTCFG_CFG80211_MODULE)
#include "HalPrecomp.h"
#else
#include "../../HalPrecomp.h"
#endif

// TODO: this function should be modified
void 
TXPowerTracking_ThermalMeter_Tmp8814A(
    IN  HAL_PADAPTER    Adapter
)
{
     // Do Nothing now
}

u4Byte
PHY_QueryRFReg_8814(
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

	u4Byte	DataAndAddr = 0;
	u4Byte	Direct_Addr;	
	
	RegAddr &= 0xff;
	switch(eRFPath){
		case RF88XX_PATH_A:
			Direct_Addr = 0x2800+RegAddr*4;
		break;
		case RF88XX_PATH_B:
			Direct_Addr = 0x2c00+RegAddr*4;
		break;
		case RF88XX_PATH_C:
			Direct_Addr = 0x3800+RegAddr*4;
		break;
		case RF88XX_PATH_D:
			Direct_Addr = 0x3c00+RegAddr*4;
		break;
	}
	

	BitMask &= bRFRegOffsetMask;
	
	Readback_Value = PHY_QueryBBReg(Adapter, Direct_Addr, BitMask);		

	HAL_RESTORE_INT(flags);

	return (Readback_Value);
}


/* Setting CCA TH 2nd CCA parameter by Rx Antenna */
void
PHY_Set_SecCCATH_by_RXANT_8814A(
    IN  HAL_PADAPTER                Adapter,
    IN  u4Byte                      ulAntennaRx
)
{
	HAL_PADAPTER	priv	 = Adapter;

	u1Byte Current_band;
    
	if (HAL_VAR_DOT11CHANNEL <= CHANNEL_MAX_NUMBER_2G)
		Current_band = RF88XX_BAND_ON_2_4G;
	else
		Current_band = RF88XX_BAND_ON_5G;
	
	
	if(priv->pshare->CurrentChannelBW == HT_CHANNEL_WIDTH_80){
		PHY_SetBBReg(priv, REG_BB_PWED_TH_AC, 0x7C0000, 0x8);           // 830[22:18] reset default
		PHY_SetBBReg(priv, REG_BB_CCAONSEC_AC, 0x0000FF00, 0x76);       // 838[15:8] reset default
		if(priv->pshare->rf_ft_var.mp_specific == 1)
			PHY_SetBBReg(priv, 0x8ac, 0x3c, 0x1);
		switch(ulAntennaRx){
		case ANTENNA_A:  // xT1R
		case ANTENNA_B:
		case ANTENNA_C:
		case ANTENNA_D:
	        PHY_SetBBReg(priv, REG_BB_CCAONSEC_AC, 0x00000001, 0x1);      // Enable 2ndCCA
	        PHY_SetBBReg(priv, REG_BB_AGC_TABLE_AC, 0xFFFF0000, 0x7599);    // 0x82C[23:20] = 9, PWDB_TH_QB, 0x82C[19:16] = 9, PWDB_TH_HB
	        PHY_SetBBReg(priv, REG_BB_CCAONSEC_AC, 0x0FFF0000, 0x887);    // 838[27:24]=8, RF80_secondary40, 838[23:20]=8, RF80_secondary20, 838[19:16]=7, RF80_primary
	        PHY_SetBBReg(priv, REG_BB_L1_Weight_Jaguar, 0x0000F000, 0x7); //840[15:12]=7, L1_square_Pk_weight_80M
	        PHY_SetBBReg(priv, 0x844, 0x0F000000, 0xb);                   //0x844[27:24]=0xb
	        PHY_SetBBReg(priv, REG_BB_PWED_TH_AC, 0xFFFFFFFF, 0x79A0EA0A);
		break;
		case ANTENNA_AB: // xT2R
		case ANTENNA_AC:
		case ANTENNA_BC:
		case ANTENNA_BD:
		case ANTENNA_CD:
	        PHY_SetBBReg(priv, REG_BB_CCAONSEC_AC, 0x00000001, 0x1);      // Enable 2ndCCA
	        PHY_SetBBReg(priv, REG_BB_AGC_TABLE_AC, 0xFFFF0000, 0x7598);    // 0x82C[23:20] = 7, PWDB_TH_QB, 0x82C[19:16] = 8, PWDB_TH_HB
	        PHY_SetBBReg(priv, REG_BB_CCAONSEC_AC, 0x0FFF0000, 0x444);    // 838[27:24]=4, RF80_secondary40, 838[23:20]=4, RF80_secondary20, 838[19:16]=4, RF80_primary
	        PHY_SetBBReg(priv, REG_BB_L1_Weight_Jaguar, 0x0000F000, 0x6); //840[15:12]=6, L1_square_Pk_weight_80M
	        PHY_SetBBReg(priv, 0x844, 0x0F000000, 0xd);                   //0x844[27:24]=0xd
	        PHY_SetBBReg(priv, REG_BB_PWED_TH_AC, 0xFFFFFFFF, 0x79A0EA0A);
		break;
		case ANTENNA_ABC: // xT3R
		case ANTENNA_BCD:
	        PHY_SetBBReg(priv, REG_BB_CCAONSEC_AC, 0x00000001, 0x1);       // Enable 2ndCCA
	        PHY_SetBBReg(priv, REG_BB_AGC_TABLE_AC, 0xFFFF0000, 0x7398);     // 0x82C[23:20] = 9, PWDB_TH_QB, 0x82C[19:16] = 8, PWDB_TH_HB
	        PHY_SetBBReg(priv, REG_BB_CCAONSEC_AC, 0x0FFF0000, 0x666);     // 838[27:24]=6, RF80_secondary40, 838[23:20]=6, RF80_secondary20, 838[19:16]=6, RF80_primary
	        PHY_SetBBReg(priv, REG_BB_L1_Weight_Jaguar, 0x0000F000, 0x6);  //840[15:12]=6, L1_square_Pk_weight_80M
	        PHY_SetBBReg(priv, 0x844, 0x0F000000, 0xd);                    //0x844[27:24]=0xd
	        PHY_SetBBReg(priv, REG_BB_PWED_TH_AC, 0xFFFFFFFF, 0x79A0EA08);
	        break;
		case ANTENNA_ABCD:  // xT4R
	        PHY_SetBBReg(priv, REG_BB_CCAONSEC_AC, 0x00000001, 0x1);       // Enable 2ndCCA
	        PHY_SetBBReg(priv, REG_BB_AGC_TABLE_AC, 0xFFFF0000, 0x7366);     // 0x82C[23:20] = 9, PWDB_TH_QB, 0x82C[19:16] = 8, PWDB_TH_HB
			PHY_SetBBReg(priv, REG_BB_CCAONSEC_AC, 0x0FFF0000, 0x666);     // 838[27:24]=6, RF80_secondary40, 838[23:20]=6, RF80_secondary20, 838[19:16]=6, RF80_primary
	        PHY_SetBBReg(priv, REG_BB_L1_Weight_Jaguar, 0x0000F000, 0x7);  // 840[15:12]=7, L1_square_Pk_weight_80M
	        PHY_SetBBReg(priv, 0x844, 0x0F000000, 0xd);                    //0x844[27:24]=0xd
	        PHY_SetBBReg(priv, REG_BB_PWED_TH_AC, 0xFFFFFFFF, 0x79A0EA08);
	        break;
		default:
		    printk("Unknown Rx antenna.\n");
		    break;
		}
	}else if(priv->pshare->CurrentChannelBW == HT_CHANNEL_WIDTH_20_40){	
		PHY_SetBBReg(priv, REG_BB_CCAONSEC_AC, 0x0FFF0000, 0x666);         // 838[27:16] reset default
		PHY_SetBBReg(priv, REG_BB_L1_Weight_Jaguar, 0x0000F000, 0x7);      // 840[15:12] reset default
		PHY_SetBBReg(priv, REG_BB_CCAONSEC_AC, 0x1, 0x1);                  // 838[0] =0x1
		PHY_SetBBReg(priv, REG_BB_CCAONSEC_AC, 0xFF00, 0x43);              // 838[15:8] =0x52
		if(priv->pshare->rf_ft_var.mp_specific == 1)
			PHY_SetBBReg(priv, 0x8ac, 0x3c, 0x1);
		switch(ulAntennaRx){
		case ANTENNA_A:  // xT1R
		case ANTENNA_B:
		case ANTENNA_C:
		case ANTENNA_D:
			PHY_SetBBReg(priv, REG_BB_AGC_TABLE_AC, 0xFFFF0000, 0x7598);         // 82C[23:16]  reset default
	        PHY_SetBBReg(priv, 0x844, 0x0F000000, 0xb);                     //0x844[27:24]=0xb
	        PHY_SetBBReg(priv, REG_BB_PWED_TH_AC, 0xFFFFFFFF, 0x79B8EA0A);
			break;
		case ANTENNA_AB: // xT2R
		case ANTENNA_AC:
		case ANTENNA_BC:
		case ANTENNA_BD:
		case ANTENNA_CD:
			PHY_SetBBReg(priv, REG_BB_AGC_TABLE_AC, 0xFFFF0000, 0x7598);         // 82C[23:16]  reset default
	        PHY_SetBBReg(priv, 0x844, 0x0F000000, 0xd);                     //0x844[27:24]=0xd
	        PHY_SetBBReg(priv, REG_BB_PWED_TH_AC, 0xFFFFFFFF, 0x79A0EA0A);
	        break;
		
		default:
			PHY_SetBBReg(priv, REG_BB_PWED_TH_AC, 0x7C0000, 0xa);           // 830[22:18] = 5b'01010
	        PHY_SetBBReg(priv, 0x844, 0x0F000000, 0xd);                     //0x844[27:24]=0xd
	        PHY_SetBBReg(priv, REG_BB_CCAONSEC_AC, 0x0000FF00, 0x22);      // 0x839
	        if(Current_band == RF88XX_BAND_ON_2_4G) {
				PHY_SetBBReg(priv, REG_BB_AGC_TABLE_AC, 0xFFFF0000, 0x7596);								
				PHY_SetBBReg(priv, REG_BB_PWED_TH_AC, 0xFFFFFFFF, 0x79A8EA0B);
	        }else{
	        	if (ulAntennaRx==ANTENNA_ABCD)
					PHY_SetBBReg(priv, REG_BB_AGC_TABLE_AC, 0xFFFF0000, 0x7555);
				else
					PHY_SetBBReg(priv, REG_BB_AGC_TABLE_AC, 0xFFFF0000, 0x7598);
				
				//PHY_SetBBReg(priv, REG_BB_AGC_TABLE_AC, 0xFFFF0000, 0x7398);		 // 82C[23:16]	reset default
				PHY_SetBBReg(priv, REG_BB_PWED_TH_AC, 0xFFFFFFFF, 0x79A8EA08);     // 0x830
			}
			break;
		}
	}else{
		PHY_SetBBReg(priv, REG_BB_CCAONSEC_AC, 0x0000FF00, 0x76);       // 838[15:8] reset default
        PHY_SetBBReg(priv, REG_BB_CCAONSEC_AC, 0x0000000F, 0x1);        // Enable 2ndCCA for 20M suggested by Casper and BB
        PHY_SetBBReg(priv, REG_BB_AGC_TABLE_AC, 0xFFFF0000, 0x7543);      // 0x82C[23:20] = 9, PWDB_TH_QB, 0x82C[19:16] = 8, PWDB_TH_HB
		PHY_SetBBReg(priv, REG_BB_AGC_TABLE_AC, 0x0000F000, 0x7);
		PHY_SetBBReg(priv, REG_BB_CCAONSEC_AC, 0x0FFF0000, 0x7aa);      // 0x838[27:24]=6, RF80_secondary40, 838[23:20]=6, RF80_secondary20, 838[19:16]=6, RF80_primary
        PHY_SetBBReg(priv, REG_BB_L1_Weight_Jaguar, 0x0000F000, 0x7);   // 0x840[15:12]=7, L1_square_Pk_weight_80M
		PHY_SetBBReg(priv, 0x844, 0x0F000000, 0xb);	                    // 0x844[27:24]=0xb
		if(priv->pshare->rf_ft_var.mp_specific == 1)
			PHY_SetBBReg(priv, 0x8ac, 0x3c, 0x2);
		switch(ulAntennaRx){
		case ANTENNA_A:  // xT1R
		case ANTENNA_B:
		case ANTENNA_C:
		case ANTENNA_D:
		case ANTENNA_AB: // xT2R
		case ANTENNA_AC:
		case ANTENNA_BC:
		case ANTENNA_BD:
		case ANTENNA_CD:
			if(Current_band == RF88XX_BAND_ON_2_4G){
				//PHY_SetBBReg(priv, REG_BB_AGC_TABLE_AC, 0xFFFF0000, 0x7343);      // 0x82f, 0x82e	
				PHY_SetBBReg(priv, REG_BB_PWED_TH_AC, 0xFFFFFFFF, 0x79A0EA08);   // 0x830
			}else{
				//PHY_SetBBReg(priv, REG_BB_AGC_TABLE_AC, 0xFFFF0000, 0x7543);      // 0x82f, 0x82e	
				PHY_SetBBReg(priv, REG_BB_PWED_TH_AC, 0xFFFFFFFF, 0x79A0EA0A);      // 0x830
			}
			break;		
		default:
			PHY_SetBBReg(priv, REG_BB_PWED_TH_AC, 0x000000FF, 0x08);      // 0x830
			if (ulAntennaRx==ANTENNA_ABCD){
				if(Current_band == RF88XX_BAND_ON_2_4G) {
					PHY_SetBBReg(priv, REG_BB_AGC_TABLE_AC, 0x0000F000, 0x2);      // 0x82c					
					PHY_SetBBReg(priv, REG_BB_PWED_TH_AC, 0xFFFFFFFF, 0x79A0EA08);      // 0x830
		        }else{
		        	PHY_SetBBReg(priv, REG_BB_PWED_TH_AC, 0xFFFFFFFF, 0x79a14a0a);
					PHY_SetBBReg(priv, REG_BB_AGC_TABLE_AC, 0x0000F000, 0x3);		 // 82C[23:16]	reset default
				}
			}else
				PHY_SetBBReg(priv, REG_BB_PWED_TH_AC, 0xFFFFFFFF, 0x79A0EA08);
			//PHY_SetBBReg(priv, REG_BB_AGC_TABLE_AC, 0xFFFF0000, 0x7343);      // 0x82f, 0x82e							
			break;
		}
    }

}


// <20141230, James> A workaround to eliminate the 5280MHz & 5600MHz & 5760MHz spur of 8814A. (Asked by BBSD Neil.)
void
phy_SpurCalibration_8814A(
    IN  HAL_PADAPTER                Adapter
)
{
	BOOLEAN         Reset_NBI_CSI = TRUE;
	BOOLEAN         Temp_Flag = TRUE;
	
	if(HAL_VAR_CURRENTCHANNELBW == HT_CHANNEL_WIDTH_20 && HAL_VAR_WORKING_CHANNEL == 153){
		PHY_SetBBReg(Adapter, rNBI_Setting_Jaguar, 0x000fe000, 0xf);
		PHY_SetBBReg(Adapter, rCSI_Mask_Setting1_Jaguar, BIT(0), 1); // CSI Mask by reg
		PHY_SetBBReg(Adapter, rCSI_Fix_Mask0_Jaguar, BIT_MASK_SET_MASKDWORD_COMMON, 0);
		PHY_SetBBReg(Adapter, rCSI_Fix_Mask1_Jaguar, BIT_MASK_SET_MASKDWORD_COMMON, 0);
		PHY_SetBBReg(Adapter, rCSI_Fix_Mask6_Jaguar, BIT_MASK_SET_MASKDWORD_COMMON, 0);
		PHY_SetBBReg(Adapter, rCSI_Fix_Mask7_Jaguar, BIT(16), 1);
		
			if (((Adapter->pshare->rf_ft_var.mp_specific==1)&&((Adapter->pshare->mp_antenna_rx==ANTENNA_A)||(Adapter->pshare->mp_antenna_rx==ANTENNA_B)||(Adapter->pshare->mp_antenna_rx==ANTENNA_C)||(Adapter->pshare->mp_antenna_rx==ANTENNA_D)))
				||	((Adapter->pshare->rf_ft_var.mp_specific==0)&&(get_rf_mimo_mode(Adapter)==MIMO_1T1R))){
				PHY_SetBBReg(Adapter, REG_BB_AGC_TABLE_AC, 0x0000F000, 0x3);
				Temp_Flag=FALSE;
			}
		
		Reset_NBI_CSI = FALSE;
	}else if(HAL_VAR_CURRENTCHANNELBW == HT_CHANNEL_WIDTH_20_40 && HAL_VAR_WORKING_CHANNEL == 151){
		PHY_SetBBReg(Adapter, rNBI_Setting_Jaguar, 0x000fe000, 0xf);
		PHY_SetBBReg(Adapter, rCSI_Mask_Setting1_Jaguar, BIT(0), 1); // CSI Mask by reg
		PHY_SetBBReg(Adapter, rCSI_Fix_Mask0_Jaguar, BIT(16), 1);
		PHY_SetBBReg(Adapter, rCSI_Fix_Mask1_Jaguar, BIT_MASK_SET_MASKDWORD_COMMON, 0);
		PHY_SetBBReg(Adapter, rCSI_Fix_Mask6_Jaguar, BIT_MASK_SET_MASKDWORD_COMMON, 0);
		PHY_SetBBReg(Adapter, rCSI_Fix_Mask7_Jaguar, BIT_MASK_SET_MASKDWORD_COMMON, 0);
		if ((HAL_RFE_TYPE == 0)||(HAL_RFE_TYPE == 7)||(HAL_RFE_TYPE == 9)){ //for ifem
			if (((Adapter->pshare->rf_ft_var.mp_specific==1)&&((Adapter->pshare->mp_antenna_rx==ANTENNA_A)||(Adapter->pshare->mp_antenna_rx==ANTENNA_B)||(Adapter->pshare->mp_antenna_rx==ANTENNA_C)||(Adapter->pshare->mp_antenna_rx==ANTENNA_D)))
				||	((Adapter->pshare->rf_ft_var.mp_specific==0)&&(get_rf_mimo_mode(Adapter)==MIMO_1T1R))){
				PHY_SetBBReg(Adapter, REG_BB_AGC_TABLE_AC, 0x000F0000, 0x6);
				Temp_Flag=FALSE;
			}
		}
		Reset_NBI_CSI = FALSE;
	}else if(HAL_VAR_CURRENTCHANNELBW == HT_CHANNEL_WIDTH_80 && HAL_VAR_WORKING_CHANNEL == 155){ // center ch 155
		PHY_SetBBReg(Adapter, rNBI_Setting_Jaguar, 0x000fe000, 0x2d);
		PHY_SetBBReg(Adapter, rCSI_Mask_Setting1_Jaguar, BIT(0), 1); // CSI Mask by reg
		PHY_SetBBReg(Adapter, rCSI_Fix_Mask0_Jaguar, BIT_MASK_SET_MASKDWORD_COMMON, 0);
		PHY_SetBBReg(Adapter, rCSI_Fix_Mask1_Jaguar, BIT_MASK_SET_MASKDWORD_COMMON, 0);
		PHY_SetBBReg(Adapter, rCSI_Fix_Mask6_Jaguar, BIT(16), 1);
		PHY_SetBBReg(Adapter, rCSI_Fix_Mask7_Jaguar, BIT_MASK_SET_MASKDWORD_COMMON, 0);
	
		Reset_NBI_CSI = FALSE;
	}

	if((HAL_RFE_TYPE == 0)||(HAL_RFE_TYPE == 7)||(HAL_RFE_TYPE == 9)){ //for ifem
		switch(HAL_VAR_CURRENTCHANNELBW){
		case HT_CHANNEL_WIDTH_20_40:
			if(HAL_VAR_WORKING_CHANNEL == 54 || HAL_VAR_WORKING_CHANNEL == 118){
				PHY_SetBBReg(Adapter, rNBI_Setting_Jaguar, 0x000fe000, 0x1f);
				PHY_SetBBReg(Adapter, rCSI_Mask_Setting1_Jaguar, BIT(0), 1); // CSI Mask by reg
				PHY_SetBBReg(Adapter, rCSI_Fix_Mask0_Jaguar, BIT_MASK_SET_MASKDWORD_COMMON, 0);
				PHY_SetBBReg(Adapter, rCSI_Fix_Mask1_Jaguar, BIT(0), 1);
				PHY_SetBBReg(Adapter, rCSI_Fix_Mask6_Jaguar, BIT_MASK_SET_MASKDWORD_COMMON, 0);
				PHY_SetBBReg(Adapter, rCSI_Fix_Mask7_Jaguar, BIT_MASK_SET_MASKDWORD_COMMON, 0);
				if (((Adapter->pshare->rf_ft_var.mp_specific==1)&&((Adapter->pshare->mp_antenna_rx==ANTENNA_A)||(Adapter->pshare->mp_antenna_rx==ANTENNA_B)||(Adapter->pshare->mp_antenna_rx==ANTENNA_C)||(Adapter->pshare->mp_antenna_rx==ANTENNA_D)))
					||	((Adapter->pshare->rf_ft_var.mp_specific==0)&&(Adapter->pmib->dot11RFEntry.MIMO_TR_mode==MIMO_1T1R))){
					PHY_SetBBReg(Adapter, REG_BB_AGC_TABLE_AC, 0x000F0000, 0x6);
					Temp_Flag=FALSE;
				}
				
				Reset_NBI_CSI = FALSE;
			}
			break;
		case HT_CHANNEL_WIDTH_80: // center ch 58, 122
			if(HAL_VAR_WORKING_CHANNEL == 58 || HAL_VAR_WORKING_CHANNEL == 122){
				PHY_SetBBReg(Adapter, rNBI_Setting_Jaguar, 0x000fe000, 0x1d);
				PHY_SetBBReg(Adapter, rCSI_Mask_Setting1_Jaguar, BIT(0), 1); // CSI Mask by reg
				PHY_SetBBReg(Adapter, rCSI_Fix_Mask0_Jaguar, BIT_MASK_SET_MASKDWORD_COMMON, 0);
				PHY_SetBBReg(Adapter, rCSI_Fix_Mask1_Jaguar, BIT_MASK_SET_MASKDWORD_COMMON, 0);
				PHY_SetBBReg(Adapter, rCSI_Fix_Mask6_Jaguar, BIT_MASK_SET_MASKDWORD_COMMON, 0);
				PHY_SetBBReg(Adapter, rCSI_Fix_Mask7_Jaguar, BIT(0), 1);
				Reset_NBI_CSI = FALSE;
			}
			break;
		default:
			break;
		}
	}
	if (Reset_NBI_CSI){
		PHY_SetBBReg(Adapter, rNBI_Setting_Jaguar, 0x000fe000, 0x7e);
		PHY_SetBBReg(Adapter, rCSI_Mask_Setting1_Jaguar, BIT(0), 0);
		PHY_SetBBReg(Adapter, rCSI_Fix_Mask0_Jaguar, BIT_MASK_SET_MASKDWORD_COMMON, 0);
		PHY_SetBBReg(Adapter, rCSI_Fix_Mask1_Jaguar, BIT_MASK_SET_MASKDWORD_COMMON, 0);
		PHY_SetBBReg(Adapter, rCSI_Fix_Mask6_Jaguar, BIT_MASK_SET_MASKDWORD_COMMON, 0);
		PHY_SetBBReg(Adapter, rCSI_Fix_Mask7_Jaguar, BIT_MASK_SET_MASKDWORD_COMMON, 0);
		
	}
	if(Temp_Flag){
		if(HAL_VAR_CURRENTCHANNELBW==HT_CHANNEL_WIDTH_20){
			if (((Adapter->pshare->rf_ft_var.mp_specific==1)&&(Adapter->pshare->mp_antenna_rx==ANTENNA_ABCD))
				||	((Adapter->pshare->rf_ft_var.mp_specific==0)&&(get_rf_mimo_mode(Adapter)==MIMO_3T3R))){
				PHY_SetBBReg(Adapter, REG_BB_AGC_TABLE_AC, 0x0000F000, 0x3);
			}else{
				PHY_SetBBReg(Adapter, REG_BB_AGC_TABLE_AC, 0x0000F000, 0x7);	
			}
		}else if(HAL_VAR_CURRENTCHANNELBW==HT_CHANNEL_WIDTH_20_40)
			if (((Adapter->pshare->rf_ft_var.mp_specific==1)&&(Adapter->pshare->mp_antenna_rx==ANTENNA_ABCD))
				||	((Adapter->pshare->rf_ft_var.mp_specific==0)&&(get_rf_mimo_mode(Adapter)==MIMO_3T3R))){
				PHY_SetBBReg(Adapter, REG_BB_AGC_TABLE_AC, 0x000F0000, 0x5);
			}else{
				PHY_SetBBReg(Adapter, REG_BB_AGC_TABLE_AC, 0x000F0000, 0x8);	
			}
		 else if(HAL_VAR_CURRENTCHANNELBW==HT_CHANNEL_WIDTH_80)
			if (((Adapter->pshare->rf_ft_var.mp_specific==1)&&((Adapter->pshare->mp_antenna_rx==ANTENNA_A)||(Adapter->pshare->mp_antenna_rx==ANTENNA_B)||(Adapter->pshare->mp_antenna_rx==ANTENNA_C)||(Adapter->pshare->mp_antenna_rx==ANTENNA_D)))
				||	((Adapter->pshare->rf_ft_var.mp_specific==0)&&(get_rf_mimo_mode(Adapter)==MIMO_1T1R))){
				PHY_SetBBReg(Adapter, REG_BB_AGC_TABLE_AC, 0x00FF0000, 0x99);
			} else if (((Adapter->pshare->rf_ft_var.mp_specific==1)&&((Adapter->pshare->mp_antenna_rx==ANTENNA_AC)||(Adapter->pshare->mp_antenna_rx==ANTENNA_AB)||(Adapter->pshare->mp_antenna_rx==ANTENNA_BC)||(Adapter->pshare->mp_antenna_rx==ANTENNA_BD)||(Adapter->pshare->mp_antenna_rx==ANTENNA_CD)))
				||	((Adapter->pshare->rf_ft_var.mp_specific==0)&&(get_rf_mimo_mode(Adapter)==MIMO_2T2R))){
				PHY_SetBBReg(Adapter, REG_BB_AGC_TABLE_AC, 0x00FF0000, 0x98);
			} else if (((Adapter->pshare->rf_ft_var.mp_specific==1)&&(Adapter->pshare->mp_antenna_rx==ANTENNA_ABCD))
				||	((Adapter->pshare->rf_ft_var.mp_specific==0)&&(get_rf_mimo_mode(Adapter)==MIMO_3T3R))){
				PHY_SetBBReg(Adapter, REG_BB_AGC_TABLE_AC, 0x00FF0000, 0x66);
			} else { 
				PHY_SetBBReg(Adapter, REG_BB_AGC_TABLE_AC, 0x00FF0000, 0x98);
			}
	}
}

void
phy_ConfigBB_8814A(
    IN  HAL_PADAPTER                Adapter
)
{
	printk("[%s] start\n",__FUNCTION__);
	PHY_SetBBReg(Adapter, rOFDMCCKEN_Jaguar, bOFDMEN_Jaguar|bCCKEN_Jaguar, 0x3);
	PHY_SetBBReg(Adapter, 0x820, BIT_MASK_SET_MASKDWORD_COMMON, 0x0003BD20);
	PHY_SetBBReg(Adapter, 0x828, BIT_MASK_SET_MASKDWORD_COMMON, 0x0003BD20);
	PHY_SetBBReg(Adapter, rADC_Buf_160_Clk_Jaguar, BIT31, 0x1);
}


