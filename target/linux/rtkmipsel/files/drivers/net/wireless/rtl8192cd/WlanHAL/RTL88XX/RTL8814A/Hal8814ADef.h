#ifndef __HAL8814A_DEF_H__
#define __HAL8814A_DEF_H__

/*++
Copyright (c) Realtek Semiconductor Corp. All rights reserved.

Module Name:
	Hal8814ADef.h
	
Abstract:
	Defined HAL 8814A data structure & Define
	    
Major Change History:
	When       Who               What
	---------- ---------------   -------------------------------
	2013-05-28 Filen            Create.	
--*/

// Test Chip
extern u1Byte *data_AGC_TAB_8814A_start,    *data_AGC_TAB_8814A_end;
extern u1Byte *data_MAC_REG_8814A_start,    *data_MAC_REG_8814A_end;
extern u1Byte *data_PHY_REG_8814A_start,    *data_PHY_REG_8814A_end;
//extern u1Byte *data_PHY_REG_1T_8814A_start, *data_PHY_REG_1T_8814A_end;
extern u1Byte *data_PHY_REG_MP_8814A_start, *data_PHY_REG_MP_8814A_end;
extern u1Byte *data_PHY_REG_PG_8814A_start, *data_PHY_REG_PG_8814A_end;
extern u1Byte *data_RadioA_8814A_start,     *data_RadioA_8814A_end;
extern u1Byte *data_RadioB_8814A_start,     *data_RadioB_8814A_end;
extern u1Byte *data_RadioC_8814A_start,     *data_RadioC_8814A_end;
extern u1Byte *data_RadioD_8814A_start,     *data_RadioD_8814A_end;


//High Power
#if CFG_HAL_HIGH_POWER_EXT_PA
extern u1Byte *data_AGC_TAB_8814A_hp_start,    *data_AGC_TAB_8814A_hp_end;
extern u1Byte *data_PHY_REG_8814A_hp_start,    *data_PHY_REG_8814A_hp_end;
extern u1Byte *data_RadioA_8814A_hp_start,     *data_RadioA_8814A_hp_end;
extern u1Byte *data_RadioB_8814A_hp_start,     *data_RadioB_8814A_hp_end;
extern u1Byte *data_RadioC_8814A_hp_start,     *data_RadioC_8814A_hp_end;
extern u1Byte *data_RadioD_8814A_hp_start,     *data_RadioD_8814A_hp_end;
#endif

// FW
extern u1Byte *data_rtl8814Afw_start,         *data_rtl8814Afw_end;
extern u1Byte *data_rtl8814AfwMP_start,       *data_rtl8814AfwMP_end;

// Power Tracking
extern u1Byte *data_TxPowerTrack_AP_8814A_start,    *data_TxPowerTrack_AP_8814A_end;


//3 MACDM
//default
extern u1Byte *data_MACDM_def_high_8814A_start, *data_MACDM_def_high_8814A_end;
extern u1Byte *data_MACDM_def_low_8814A_start, *data_MACDM_def_low_8814A_end;
extern u1Byte *data_MACDM_def_normal_8814A_start, *data_MACDM_def_normal_8814A_end;

//general
extern u1Byte *data_MACDM_gen_high_8814A_start, *data_MACDM_gen_high_8814A_end;
extern u1Byte *data_MACDM_gen_low_8814A_start, *data_MACDM_gen_low_8814A_end;
extern u1Byte *data_MACDM_gen_normal_8814A_start, *data_MACDM_gen_normal_8814A_end;

//txop
extern u1Byte *data_MACDM_txop_high_8814A_start, *data_MACDM_txop_high_8814A_end;
extern u1Byte *data_MACDM_txop_low_8814A_start, *data_MACDM_txop_low_8814A_end;
extern u1Byte *data_MACDM_txop_normal_8814A_start, *data_MACDM_txop_normal_8814A_end;

//criteria
extern u1Byte *data_MACDM_state_criteria_8814A_start, *data_MACDM_state_criteria_8814A_end;


// MP chip 
extern u1Byte *data_AGC_TAB_8814Amp_start,    *data_AGC_TAB_8814Amp_end;
extern u1Byte *data_MAC_REG_8814Amp_start,    *data_MAC_REG_8814Amp_end;
extern u1Byte *data_PHY_REG_8814Amp_start,    *data_PHY_REG_8814Amp_end;
//extern u1Byte *data_PHY_REG_1T_8814A_start, *data_PHY_REG_1T_8814A_end;
extern u1Byte *data_PHY_REG_MP_8814Amp_start, *data_PHY_REG_MP_8814Amp_end;
extern u1Byte *data_PHY_REG_PG_8814Amp_start, *data_PHY_REG_PG_8814Amp_end;
extern u1Byte *data_PHY_REG_PG_8814Amp_Type0_start, *data_PHY_REG_PG_8814Amp_Type0_end;
extern u1Byte *data_PHY_REG_PG_8814Amp_Type7_start, *data_PHY_REG_PG_8814Amp_Type7_end;
extern u1Byte *data_PHY_REG_PG_8814Amp_Type9_start, *data_PHY_REG_PG_8814Amp_Type9_end;
extern u1Byte *data_RadioA_8814Amp_start,     *data_RadioA_8814Amp_end;
extern u1Byte *data_RadioB_8814Amp_start,     *data_RadioB_8814Amp_end;
extern u1Byte *data_RadioC_8814Amp_start,     *data_RadioC_8814Amp_end;
extern u1Byte *data_RadioD_8814Amp_start,     *data_RadioD_8814Amp_end;


//High Power
#if CFG_HAL_HIGH_POWER_EXT_PA
extern u1Byte *data_AGC_TAB_8814Amp_hp_start,    *data_AGC_TAB_8814Amp_hp_end;
extern u1Byte *data_PHY_REG_8814Amp_hp_start,    *data_PHY_REG_8814Amp_hp_end;
extern u1Byte *data_PHY_REG_PG_8814Amp_Type2_start, *data_PHY_REG_PG_8814Amp_Type2_end;
extern u1Byte *data_PHY_REG_PG_8814Amp_Type3_start, *data_PHY_REG_PG_8814Amp_Type3_end;
extern u1Byte *data_PHY_REG_PG_8814Amp_Type4_start, *data_PHY_REG_PG_8814Amp_Type4_end;
extern u1Byte *data_PHY_REG_PG_8814Amp_Type5_start, *data_PHY_REG_PG_8814Amp_Type5_end;
extern u1Byte *data_PHY_REG_PG_8814Amp_Type8_start, *data_PHY_REG_PG_8814Amp_Type8_end;
extern u1Byte *data_RadioA_8814Amp_hp_start,     *data_RadioA_8814Amp_hp_end;
extern u1Byte *data_RadioB_8814Amp_hp_start,     *data_RadioB_8814Amp_hp_end;
extern u1Byte *data_RadioC_8814Amp_hp_start,     *data_RadioC_8814Amp_hp_end;
extern u1Byte *data_RadioD_8814Amp_hp_start,     *data_RadioD_8814Amp_hp_end;
#endif

// FW
extern u1Byte *data_rtl8814Afw_start,         *data_rtl8814Afw_end;
extern u1Byte *data_rtl8814AfwMP_start,       *data_rtl8814AfwMP_end;

// Power Tracking
extern u1Byte *data_TxPowerTrack_AP_8814Amp_start,    *data_TxPowerTrack_AP_8814Amp_end;

#ifdef TXPWR_LMT_8814A
extern unsigned char *data_TXPWR_LMT_8814Amp_start, *data_TXPWR_LMT_8814Amp_end;
extern unsigned char *data_TXPWR_LMT_8814Amp_Type0_start, *data_TXPWR_LMT_8814Amp_Type0_end;
extern unsigned char *data_TXPWR_LMT_8814Amp_Type7_start, *data_TXPWR_LMT_8814Amp_Type7_end;
extern unsigned char *data_TXPWR_LMT_8814Amp_Type9_start, *data_TXPWR_LMT_8814Amp_Type9_end;
#if CFG_HAL_HIGH_POWER_EXT_PA
extern unsigned char *data_TXPWR_LMT_8814Amp_Type2_start, *data_TXPWR_LMT_8814Amp_Type2_end;
extern unsigned char *data_TXPWR_LMT_8814Amp_Type3_start, *data_TXPWR_LMT_8814Amp_Type3_end;
extern unsigned char *data_TXPWR_LMT_8814Amp_Type4_start, *data_TXPWR_LMT_8814Amp_Type4_end;
extern unsigned char *data_TXPWR_LMT_8814Amp_Type5_start, *data_TXPWR_LMT_8814Amp_Type5_end;
extern unsigned char *data_TXPWR_LMT_8814Amp_Type8_start, *data_TXPWR_LMT_8814Amp_Type8_end;
#endif
#ifdef BEAMFORMING_AUTO
extern unsigned char *data_TXPWR_LMT_8814Amp_TXBF_start, *data_TXPWR_LMT_8814Amp_TXBF_end;
#endif
#endif


//3 MACDM
//default
extern u1Byte *data_MACDM_def_high_8814Amp_start, *data_MACDM_def_high_8814Amp_end;
extern u1Byte *data_MACDM_def_low_8814Amp_start, *data_MACDM_def_low_8814Amp_end;
extern u1Byte *data_MACDM_def_normal_8814Amp_start, *data_MACDM_def_normal_8814Amp_end;

//general
extern u1Byte *data_MACDM_gen_high_8814Amp_start, *data_MACDM_gen_high_8814Amp_end;
extern u1Byte *data_MACDM_gen_low_8814Amp_start, *data_MACDM_gen_low_8814Amp_end;
extern u1Byte *data_MACDM_gen_normal_8814Amp_start, *data_MACDM_gen_normal_8814Amp_end;

//txop
extern u1Byte *data_MACDM_txop_high_8814Amp_start, *data_MACDM_txop_high_8814Amp_end;
extern u1Byte *data_MACDM_txop_low_8814Amp_start, *data_MACDM_txop_low_8814Amp_end;
extern u1Byte *data_MACDM_txop_normal_8814Amp_start, *data_MACDM_txop_normal_8814Amp_end;

//criteria
extern u1Byte *data_MACDM_state_criteria_8814Amp_start, *data_MACDM_state_criteria_8814Amp_end;

#endif  //__HAL8814A_DEF_H__


