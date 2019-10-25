#ifndef __HAL8822B_DEF_H__
#define __HAL8822B_DEF_H__

/*++
Copyright (c) Realtek Semiconductor Corp. All rights reserved.

Module Name:
	Hal8822BDef.h
	
Abstract:
	Defined HAL 8822B data structure & Define
	    
Major Change History:
	When       Who               What
	---------- ---------------   -------------------------------
	2015/06/25 Eric            Create.	
--*/

// Test Chip
extern u1Byte *data_AGC_TAB_8822B_start,    *data_AGC_TAB_8822B_end;
extern u1Byte *data_MAC_REG_8822B_start,    *data_MAC_REG_8822B_end;
extern u1Byte *data_PHY_REG_8822B_start,    *data_PHY_REG_8822B_end;
//extern u1Byte *data_PHY_REG_1T_8822B_start, *data_PHY_REG_1T_8822B_end;
extern u1Byte *data_PHY_REG_MP_8822B_start, *data_PHY_REG_MP_8822B_end;
extern u1Byte *data_PHY_REG_PG_8822B_start, *data_PHY_REG_PG_8822B_end;
extern u1Byte *data_RadioA_8822B_start,     *data_RadioA_8822B_end;
extern u1Byte *data_RadioB_8822B_start,     *data_RadioB_8822B_end;


//High Power
#if CFG_HAL_HIGH_POWER_EXT_PA
extern u1Byte *data_AGC_TAB_8822B_hp_start,    *data_AGC_TAB_8822B_hp_end;
extern u1Byte *data_PHY_REG_8822B_hp_start,    *data_PHY_REG_8822B_hp_end;
extern u1Byte *data_RadioA_8822B_hp_start,     *data_RadioA_8822B_hp_end;
extern u1Byte *data_RadioB_8822B_hp_start,     *data_RadioB_8822B_hp_end;
#endif

// FW
extern u1Byte *data_rtl8822Bfw_start,         *data_rtl8822Bfw_end;
extern u1Byte *data_rtl8822BfwMP_start,       *data_rtl8822BfwMP_end;

// Power Tracking
extern u1Byte *data_TxPowerTrack_AP_8822B_start,    *data_TxPowerTrack_AP_8822B_end;


//3 MACDM
//default
extern u1Byte *data_MACDM_def_high_8822B_start, *data_MACDM_def_high_8822B_end;
extern u1Byte *data_MACDM_def_low_8822B_start, *data_MACDM_def_low_8822B_end;
extern u1Byte *data_MACDM_def_normal_8822B_start, *data_MACDM_def_normal_8822B_end;

//general
extern u1Byte *data_MACDM_gen_high_8822B_start, *data_MACDM_gen_high_8822B_end;
extern u1Byte *data_MACDM_gen_low_8822B_start, *data_MACDM_gen_low_8822B_end;
extern u1Byte *data_MACDM_gen_normal_8822B_start, *data_MACDM_gen_normal_8822B_end;

//txop
extern u1Byte *data_MACDM_txop_high_8822B_start, *data_MACDM_txop_high_8822B_end;
extern u1Byte *data_MACDM_txop_low_8822B_start, *data_MACDM_txop_low_8822B_end;
extern u1Byte *data_MACDM_txop_normal_8822B_start, *data_MACDM_txop_normal_8822B_end;

//criteria
extern u1Byte *data_MACDM_state_criteria_8822B_start, *data_MACDM_state_criteria_8822B_end;


// MP chip 
extern u1Byte *data_AGC_TAB_8822Bmp_start,    *data_AGC_TAB_8822Bmp_end;
extern u1Byte *data_MAC_REG_8822Bmp_start,    *data_MAC_REG_8822Bmp_end;
extern u1Byte *data_PHY_REG_8822Bmp_start,    *data_PHY_REG_8822Bmp_end;
//extern u1Byte *data_PHY_REG_1T_8822B_start, *data_PHY_REG_1T_8822B_end;
extern u1Byte *data_PHY_REG_MP_8822Bmp_start, *data_PHY_REG_MP_8822Bmp_end;
extern u1Byte *data_PHY_REG_PG_8822Bmp_start, *data_PHY_REG_PG_8822Bmp_end;
extern u1Byte *data_PHY_REG_PG_8822Bmp_Type0_start, *data_PHY_REG_PG_8822Bmp_Type0_end;
extern u1Byte *data_PHY_REG_PG_8822Bmp_Type8_start, *data_PHY_REG_PG_8822Bmp_Type8_end;
extern u1Byte *data_PHY_REG_PG_8822Bmp_Type9_start, *data_PHY_REG_PG_8822Bmp_Type9_end;
extern u1Byte *data_PHY_REG_PG_8822Bmp_Type10_start, *data_PHY_REG_PG_8822Bmp_Type10_end;
extern u1Byte *data_PHY_REG_PG_8822Bmp_Type13_start, *data_PHY_REG_PG_8822Bmp_Type13_end;
extern u1Byte *data_PHY_REG_PG_8822Bmp_Type14_start, *data_PHY_REG_PG_8822Bmp_Type14_end;
extern u1Byte *data_RadioA_8822Bmp_start,     *data_RadioA_8822Bmp_end;
extern u1Byte *data_RadioB_8822Bmp_start,     *data_RadioB_8822Bmp_end;

//High Power
#if CFG_HAL_HIGH_POWER_EXT_PA
extern u1Byte *data_AGC_TAB_8822Bmp_hp_start,    *data_AGC_TAB_8822Bmp_hp_end;
extern u1Byte *data_PHY_REG_8822Bmp_hp_start,    *data_PHY_REG_8822Bmp_hp_end;
//extern u1Byte *data_PHY_REG_PG_8822Bmp_hp_start, *data_PHY_REG_PG_8822Bmp_hp_end;
extern u1Byte *data_PHY_REG_PG_8822Bmp_Type1_start, *data_PHY_REG_PG_8822Bmp_Type1_end;
extern u1Byte *data_PHY_REG_PG_8822Bmp_Type4_start, *data_PHY_REG_PG_8822Bmp_Type4_end;
extern u1Byte *data_PHY_REG_PG_8822Bmp_Type6_start, *data_PHY_REG_PG_8822Bmp_Type6_end;
extern u1Byte *data_PHY_REG_PG_8822Bmp_Type7_start, *data_PHY_REG_PG_8822Bmp_Type7_end;
extern u1Byte *data_PHY_REG_PG_8822Bmp_Type11_start, *data_PHY_REG_PG_8822Bmp_Type11_end;
extern u1Byte *data_RadioA_8822Bmp_hp_start,     *data_RadioA_8822Bmp_hp_end;
extern u1Byte *data_RadioB_8822Bmp_hp_start,     *data_RadioB_8822Bmp_hp_end;
#endif

// FW
extern u1Byte *data_rtl8822Bfw_start,         *data_rtl8822Bfw_end;
extern u1Byte *data_rtl8822BfwMP_start,       *data_rtl8822BfwMP_end;

// Power Tracking
extern u1Byte *data_TxPowerTrack_AP_8822Bmp_start,    *data_TxPowerTrack_AP_8822Bmp_end;

#ifdef TXPWR_LMT_8822B
extern unsigned char *data_TXPWR_LMT_8822Bmp_start, *data_TXPWR_LMT_8822Bmp_end;
extern unsigned char *data_TXPWR_LMT_8822Bmp_Type0_start, *data_TXPWR_LMT_8822Bmp_Type0_end;
extern unsigned char *data_TXPWR_LMT_8822Bmp_Type8_start, *data_TXPWR_LMT_8822Bmp_Type8_end;
extern unsigned char *data_TXPWR_LMT_8822Bmp_Type9_start, *data_TXPWR_LMT_8822Bmp_Type9_end;
extern unsigned char *data_TXPWR_LMT_8822Bmp_Type10_start, *data_TXPWR_LMT_8822Bmp_Type10_end;
extern unsigned char *data_TXPWR_LMT_8822Bmp_Type13_start, *data_TXPWR_LMT_8822Bmp_Type13_end;
extern unsigned char *data_TXPWR_LMT_8822Bmp_Type14_start, *data_TXPWR_LMT_8822Bmp_Type14_end;

#if CFG_HAL_HIGH_POWER_EXT_PA
extern unsigned char *data_TXPWR_LMT_8822Bmp_Type1_start, *data_TXPWR_LMT_8822Bmp_Type1_end;
extern unsigned char *data_TXPWR_LMT_8822Bmp_Type4_start, *data_TXPWR_LMT_8822Bmp_Type4_end;
extern unsigned char *data_TXPWR_LMT_8822Bmp_Type6_start, *data_TXPWR_LMT_8822Bmp_Type6_end;
extern unsigned char *data_TXPWR_LMT_8822Bmp_Type7_start, *data_TXPWR_LMT_8822Bmp_Type7_end;
extern unsigned char *data_TXPWR_LMT_8822Bmp_Type11_start, *data_TXPWR_LMT_8822Bmp_Type11_end;
#endif
#ifdef BEAMFORMING_AUTO
extern unsigned char *data_TXPWR_LMT_8822Bmp_TXBF_start, *data_TXPWR_LMT_8822Bmp_TXBF_end;
#endif
#endif



//3 MACDM
//default
extern u1Byte *data_MACDM_def_high_8822Bmp_start, *data_MACDM_def_high_8822Bmp_end;
extern u1Byte *data_MACDM_def_low_8822Bmp_start, *data_MACDM_def_low_8822Bmp_end;
extern u1Byte *data_MACDM_def_normal_8822Bmp_start, *data_MACDM_def_normal_8822Bmp_end;

//general
extern u1Byte *data_MACDM_gen_high_8822Bmp_start, *data_MACDM_gen_high_8822Bmp_end;
extern u1Byte *data_MACDM_gen_low_8822Bmp_start, *data_MACDM_gen_low_8822Bmp_end;
extern u1Byte *data_MACDM_gen_normal_8822Bmp_start, *data_MACDM_gen_normal_8822Bmp_end;

//txop
extern u1Byte *data_MACDM_txop_high_8822Bmp_start, *data_MACDM_txop_high_8822Bmp_end;
extern u1Byte *data_MACDM_txop_low_8822Bmp_start, *data_MACDM_txop_low_8822Bmp_end;
extern u1Byte *data_MACDM_txop_normal_8822Bmp_start, *data_MACDM_txop_normal_8822Bmp_end;

//criteria
extern u1Byte *data_MACDM_state_criteria_8822Bmp_start, *data_MACDM_state_criteria_8822Bmp_end;

#endif  //__HAL8822B_DEF_H__


