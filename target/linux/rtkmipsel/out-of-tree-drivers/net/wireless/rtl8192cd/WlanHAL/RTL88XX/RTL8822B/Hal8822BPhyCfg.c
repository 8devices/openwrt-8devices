/*++
Copyright (c) Realtek Semiconductor Corp. All rights reserved.

Module Name:
	Hal8822BPhyCfg.c
	
Abstract:
	Defined HAL 8822B PHY BB setting functions
	    
Major Change History:
	When       Who               What
	---------- ---------------   -------------------------------
	2015-06-25 Eric               Create.	
--*/
#include "HalPrecomp.h"

// TODO: this function should be modified
void 
TXPowerTracking_ThermalMeter_Tmp8822B(
    IN  HAL_PADAPTER    Adapter
)
{
     // Do Nothing now
}

u4Byte
PHY_QueryRFReg_8822(
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

#if 1 //eric-8822
	if(IS_HARDWARE_TYPE_8822BE(Adapter)) {
		HAL_SAVE_INT_AND_CLI(flags);
		Readback_Value = config_phydm_read_rf_reg_8822b((&(Adapter->pshare->_dmODM)), eRFPath, RegAddr, BitMask);
		HAL_RESTORE_INT(flags);
		return Readback_Value;
	}
#endif

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
	}
	

	BitMask &= bRFRegOffsetMask;
	
	Readback_Value = PHY_QueryBBReg(Adapter, Direct_Addr, BitMask);		

	HAL_RESTORE_INT(flags);

	return (Readback_Value);
}


void
PHY_Set_SecCCATH_by_RXANT_8822B(
    IN  HAL_PADAPTER                Adapter,
    IN  u4Byte                      ulAntennaRx
)
{
	HAL_PADAPTER	priv	 = Adapter;
    //1 Setting CCA TH 2nd CCA parameter by Rx Antenna
	if(priv->pshare->CurrentChannelBW == HT_CHANNEL_WIDTH_80){
		switch(ulAntennaRx){
		case ANTENNA_A:  // xT1R
		case ANTENNA_B:
		case ANTENNA_C:
		case ANTENNA_D:
		        PHY_SetBBReg(priv, REG_BB_CCAONSEC_AC, 0x00000001, 0x1); // Enable 2ndCCA
		        PHY_SetBBReg(priv, REG_BB_AGC_TABLE_AC, 0x00FF0000, 0x89); // 0x82C[23:20] = 8, PWDB_TH_QB, 0x82C[19:16] = 9, PWDB_TH_HB
		        PHY_SetBBReg(priv, REG_BB_CCAONSEC_AC, 0x0FFF0000, 0x887); // 838[27:24]=8, RF80_secondary40, 838[23:20]=8, RF80_secondary20, 838[19:16]=7, RF80_primary
		        PHY_SetBBReg(priv, REG_BB_L1_Weight_Jaguar, 0x0000F000, 0x7); //840[15:12]=7, L1_square_Pk_weight_80M
		break;
		case ANTENNA_AB: // xT2R
		case ANTENNA_AC:
		//case ANTENNA_AD:        
		case ANTENNA_BC:
		case ANTENNA_BD:
		case ANTENNA_CD:
		        PHY_SetBBReg(priv, REG_BB_CCAONSEC_AC, 0x00000001, 0x1); // Enable 2ndCCA
		        PHY_SetBBReg(priv, REG_BB_AGC_TABLE_AC, 0x00FF0000, 0x78); // 0x82C[23:20] = 7, PWDB_TH_QB, 0x82C[19:16] = 8, PWDB_TH_HB
		        PHY_SetBBReg(priv, REG_BB_CCAONSEC_AC, 0x0FFF0000, 0x444); // 838[27:24]=4, RF80_secondary40, 838[23:20]=4, RF80_secondary20, 838[19:16]=4, RF80_primary
		        PHY_SetBBReg(priv, REG_BB_L1_Weight_Jaguar, 0x0000F000, 0x6); //840[15:12]=6, L1_square_Pk_weight_80M
		break;
		case ANTENNA_ABC: // xT3R
		//case ANTENNA_ABD:
		//case ANTENNA_ACD:
		case ANTENNA_BCD:
		        PHY_SetBBReg(priv, REG_BB_CCAONSEC_AC, 0x00000001, 0x1); // Enable 2ndCCA
		        PHY_SetBBReg(priv, REG_BB_AGC_TABLE_AC, 0x00FF0000, 0x98); // 0x82C[23:20] = 9, PWDB_TH_QB, 0x82C[19:16] = 8, PWDB_TH_HB
		        PHY_SetBBReg(priv, REG_BB_CCAONSEC_AC, 0x0FFF0000, 0x666); // 838[27:24]=6, RF80_secondary40, 838[23:20]=6, RF80_secondary20, 838[19:16]=6, RF80_primary
		        PHY_SetBBReg(priv, REG_BB_L1_Weight_Jaguar, 0x0000F000, 0x6); //840[15:12]=6, L1_square_Pk_weight_80M
		        break;
		case ANTENNA_ABCD:  // xT4R
		        PHY_SetBBReg(priv, REG_BB_CCAONSEC_AC, 0x00000001, 0x1); // Enable 2ndCCA
		        PHY_SetBBReg(priv, REG_BB_AGC_TABLE_AC, 0x00FF0000, 0x98); // 0x82C[23:20] = 9, PWDB_TH_QB, 0x82C[19:16] = 8, PWDB_TH_HB
		        PHY_SetBBReg(priv, REG_BB_CCAONSEC_AC, 0x0FFF0000, 0x666); // 838[27:24]=6, RF80_secondary40, 838[23:20]=6, RF80_secondary20, 838[19:16]=6, RF80_primary
		        PHY_SetBBReg(priv, REG_BB_L1_Weight_Jaguar, 0x0000F000, 0x7); //840[15:12]=7, L1_square_Pk_weight_80M
		        break;
		default:
		        panic_printk("Unknown Rx antenna.\n");
		        break;
		}
	}else{
            PHY_SetBBReg(priv, REG_BB_CCAONSEC_AC, 0x00000001, 0x0); // Enable 2ndCCA
            PHY_SetBBReg(priv, REG_BB_AGC_TABLE_AC, 0x00FF0000, 0x43); // 0x82C[23:20] = 9, PWDB_TH_QB, 0x82C[19:16] = 8, PWDB_TH_HB
            PHY_SetBBReg(priv, REG_BB_CCAONSEC_AC, 0x0FFF0000, 0x7aa); // 838[27:24]=6, RF80_secondary40, 838[23:20]=6, RF80_secondary20, 838[19:16]=6, RF80_primary
            PHY_SetBBReg(priv, REG_BB_L1_Weight_Jaguar, 0x0000F000, 0x7); //840[15:12]=7, L1_square_Pk_weight_80M
    }
}


// <20141230, James> A workaround to eliminate the 5280MHz & 5600MHz & 5760MHz spur of 8822B. (Asked by BBSD Neil.)
void
phy_SpurCalibration_8822B(
    IN  HAL_PADAPTER                Adapter
)
{
	BOOLEAN         Reset_NBI_CSI = TRUE;
	
	if(HAL_RFE_TYPE == 0 || HAL_RFE_TYPE == 1 || HAL_RFE_TYPE == 2 || HAL_RFE_TYPE == 3 || HAL_RFE_TYPE == 4){
		if(HAL_VAR_CURRENTCHANNELBW == HT_CHANNEL_WIDTH_20 && HAL_VAR_WORKING_CHANNEL == 153){
			PHY_SetBBReg(Adapter, rNBI_Setting_Jaguar, 0x000fe000, 0xf);
			PHY_SetBBReg(Adapter, rCSI_Mask_Setting1_Jaguar, BIT(0), 1); // CSI Mask by reg
			PHY_SetBBReg(Adapter, rCSI_Fix_Mask0_Jaguar, BIT_MASK_SET_MASKDWORD_COMMON, 0);
			PHY_SetBBReg(Adapter, rCSI_Fix_Mask1_Jaguar, BIT_MASK_SET_MASKDWORD_COMMON, 0);
			PHY_SetBBReg(Adapter, rCSI_Fix_Mask6_Jaguar, BIT_MASK_SET_MASKDWORD_COMMON, 0);
			PHY_SetBBReg(Adapter, rCSI_Fix_Mask7_Jaguar, BIT(16), 1);
			Reset_NBI_CSI = FALSE;
		}else if(HAL_VAR_CURRENTCHANNELBW == HT_CHANNEL_WIDTH_20_40 && HAL_VAR_WORKING_CHANNEL == 151){
			PHY_SetBBReg(Adapter, rNBI_Setting_Jaguar, 0x000fe000, 0xf);
			PHY_SetBBReg(Adapter, rCSI_Mask_Setting1_Jaguar, BIT(0), 1); // CSI Mask by reg
			PHY_SetBBReg(Adapter, rCSI_Fix_Mask0_Jaguar, BIT(16), 1);
			PHY_SetBBReg(Adapter, rCSI_Fix_Mask1_Jaguar, BIT_MASK_SET_MASKDWORD_COMMON, 0);
			PHY_SetBBReg(Adapter, rCSI_Fix_Mask6_Jaguar, BIT_MASK_SET_MASKDWORD_COMMON, 0);
			PHY_SetBBReg(Adapter, rCSI_Fix_Mask7_Jaguar, BIT_MASK_SET_MASKDWORD_COMMON, 0);
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
	}
	if(HAL_RFE_TYPE == 0){
		switch(HAL_VAR_CURRENTCHANNELBW){
		case HT_CHANNEL_WIDTH_20_40:
			if(HAL_VAR_WORKING_CHANNEL == 54 || HAL_VAR_WORKING_CHANNEL == 118){
				PHY_SetBBReg(Adapter, rNBI_Setting_Jaguar, 0x000fe000, 0x1f);
				PHY_SetBBReg(Adapter, rCSI_Mask_Setting1_Jaguar, BIT(0), 1); // CSI Mask by reg
				PHY_SetBBReg(Adapter, rCSI_Fix_Mask0_Jaguar, BIT_MASK_SET_MASKDWORD_COMMON, 0);
				PHY_SetBBReg(Adapter, rCSI_Fix_Mask1_Jaguar, BIT(0), 1);
				PHY_SetBBReg(Adapter, rCSI_Fix_Mask6_Jaguar, BIT_MASK_SET_MASKDWORD_COMMON, 0);
				PHY_SetBBReg(Adapter, rCSI_Fix_Mask7_Jaguar, BIT_MASK_SET_MASKDWORD_COMMON, 0);
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
}



