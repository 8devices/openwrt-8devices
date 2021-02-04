/*++
Copyright (c) Realtek Semiconductor Corp. All rights reserved.

Module Name:
	Hal8822BGen.c
	
Abstract:
	Defined RTL8822B HAL Function
	    
Major Change History:
	When       Who               What
	---------- ---------------   -------------------------------
	2015-06-25 Eric             Create.	
--*/

#ifndef __ECOS
#include "HalPrecomp.h"
#else
#include "../../HalPrecomp.h"
#endif

// TestChip
#include "data_AGC_TAB_8822B.c"
#include "data_MAC_REG_8822B.c"
#include "data_PHY_REG_8822B.c"
#include "data_PHY_REG_MP_8822B.c"
#include "data_PHY_REG_PG_8822B.c"
#include "data_RadioA_8822B.c"
#include "data_RadioB_8822B.c"
#include "data_rtl8822Bfw.c"

// High Power
#if CFG_HAL_HIGH_POWER_EXT_PA
#include "data_AGC_TAB_8822B_hp.c"
#include "data_PHY_REG_8822B_hp.c"
#include "data_RadioA_8822B_hp.c"
#include "data_RadioB_8822B_hp.c"
#endif

// Power Tracking
#include "data_TxPowerTrack_AP_8822B.c"


//3 MACDM
//default
#include "data_MACDM_def_high_8822B.c"
#include "data_MACDM_def_low_8822B.c"
#include "data_MACDM_def_normal_8822B.c"
//general
#include "data_MACDM_gen_high_8822B.c"
#include "data_MACDM_gen_low_8822B.c"
#include "data_MACDM_gen_normal_8822B.c"
//txop
#include "data_MACDM_txop_high_8822B.c"
#include "data_MACDM_txop_low_8822B.c"
#include "data_MACDM_txop_normal_8822B.c"
//criteria
#include "data_MACDM_state_criteria_8822B.c"


#define VAR_MAPPING(dst,src) \
	u1Byte *data_##dst##_start = &data_##src[0]; \
	u1Byte *data_##dst##_end   = &data_##src[sizeof(data_##src)];

VAR_MAPPING(AGC_TAB_8822B, AGC_TAB_8822B);
VAR_MAPPING(MAC_REG_8822B, MAC_REG_8822B);
VAR_MAPPING(PHY_REG_8822B, PHY_REG_8822B);
//VAR_MAPPING(PHY_REG_1T_8822B, PHY_REG_1T_8822B);
VAR_MAPPING(PHY_REG_PG_8822B, PHY_REG_PG_8822B);
VAR_MAPPING(PHY_REG_MP_8822B, PHY_REG_MP_8822B);
VAR_MAPPING(RadioA_8822B, RadioA_8822B);
VAR_MAPPING(RadioB_8822B, RadioB_8822B);
VAR_MAPPING(rtl8822Bfw, rtl8822Bfw);

// High Power
#if CFG_HAL_HIGH_POWER_EXT_PA
VAR_MAPPING(AGC_TAB_8822B_hp, AGC_TAB_8822B_hp);
VAR_MAPPING(PHY_REG_8822B_hp, PHY_REG_8822B_hp);
VAR_MAPPING(RadioA_8822B_hp, RadioA_8822B_hp);
VAR_MAPPING(RadioB_8822B_hp, RadioB_8822B_hp);
#endif

// Power Tracking
VAR_MAPPING(TxPowerTrack_AP_8822B, TxPowerTrack_AP_8822B);

#ifdef TXPWR_LMT_8822B
#include "../../../data_TXPWR_LMT_8822Bmp.c"
VAR_MAPPING(TXPWR_LMT_8822Bmp,TXPWR_LMT_8822Bmp);
#include "../../../data_TXPWR_LMT_8822Bmp_Type0.c"
VAR_MAPPING(TXPWR_LMT_8822Bmp_Type0,TXPWR_LMT_8822Bmp_Type0);
#include "../../../data_TXPWR_LMT_8822Bmp_Type8.c"
VAR_MAPPING(TXPWR_LMT_8822Bmp_Type8,TXPWR_LMT_8822Bmp_Type8);
#include "../../../data_TXPWR_LMT_8822Bmp_Type9.c"
VAR_MAPPING(TXPWR_LMT_8822Bmp_Type9,TXPWR_LMT_8822Bmp_Type9);
#include "../../../data_TXPWR_LMT_8822Bmp_Type10.c"
VAR_MAPPING(TXPWR_LMT_8822Bmp_Type10,TXPWR_LMT_8822Bmp_Type10);
#include "../../../data_TXPWR_LMT_8822Bmp_Type13.c"
VAR_MAPPING(TXPWR_LMT_8822Bmp_Type13,TXPWR_LMT_8822Bmp_Type13);
#include "../../../data_TXPWR_LMT_8822Bmp_Type14.c"
VAR_MAPPING(TXPWR_LMT_8822Bmp_Type14,TXPWR_LMT_8822Bmp_Type14);



#if CFG_HAL_HIGH_POWER_EXT_PA
#include "../../../data_TXPWR_LMT_8822Bmp_Type1.c"
VAR_MAPPING(TXPWR_LMT_8822Bmp_Type1,TXPWR_LMT_8822Bmp_Type1);
#include "../../../data_TXPWR_LMT_8822Bmp_Type4.c"
VAR_MAPPING(TXPWR_LMT_8822Bmp_Type4,TXPWR_LMT_8822Bmp_Type4);
#include "../../../data_TXPWR_LMT_8822Bmp_Type6.c"
VAR_MAPPING(TXPWR_LMT_8822Bmp_Type6,TXPWR_LMT_8822Bmp_Type6);
#include "../../../data_TXPWR_LMT_8822Bmp_Type7.c"
VAR_MAPPING(TXPWR_LMT_8822Bmp_Type7,TXPWR_LMT_8822Bmp_Type7);
#include "../../../data_TXPWR_LMT_8822Bmp_Type11.c"
VAR_MAPPING(TXPWR_LMT_8822Bmp_Type11,TXPWR_LMT_8822Bmp_Type11);
#endif
#ifdef BEAMFORMING_AUTO
#include "../../../data_TXPWR_LMT_8822Bmp_TXBF.c"
VAR_MAPPING(TXPWR_LMT_8822Bmp_TXBF,TXPWR_LMT_8822Bmp_TXBF);
#endif
#endif


//3 MACDM
VAR_MAPPING(MACDM_def_high_8822B, MACDM_def_high_8822B);
VAR_MAPPING(MACDM_def_low_8822B, MACDM_def_low_8822B);
VAR_MAPPING(MACDM_def_normal_8822B, MACDM_def_normal_8822B);

VAR_MAPPING(MACDM_gen_high_8822B, MACDM_gen_high_8822B);
VAR_MAPPING(MACDM_gen_low_8822B, MACDM_gen_low_8822B);
VAR_MAPPING(MACDM_gen_normal_8822B, MACDM_gen_normal_8822B);

VAR_MAPPING(MACDM_txop_high_8822B, MACDM_txop_high_8822B);
VAR_MAPPING(MACDM_txop_low_8822B, MACDM_txop_low_8822B);
VAR_MAPPING(MACDM_txop_normal_8822B, MACDM_txop_normal_8822B);

VAR_MAPPING(MACDM_state_criteria_8822B, MACDM_state_criteria_8822B);


//MP Chip
#include "data_AGC_TAB_8822Bmp.c"
#include "data_MAC_REG_8822Bmp.c"
#include "data_PHY_REG_8822Bmp.c"
#include "data_PHY_REG_MP_8822Bmp.c"
#include "data_PHY_REG_PG_8822Bmp.c"
#include "data_PHY_REG_PG_8822Bmp_Type0.c"
#include "data_PHY_REG_PG_8822Bmp_Type8.c"
#include "data_PHY_REG_PG_8822Bmp_Type9.c"
#include "data_PHY_REG_PG_8822Bmp_Type10.c"
#include "data_PHY_REG_PG_8822Bmp_Type13.c"
#include "data_PHY_REG_PG_8822Bmp_Type14.c"
#include "data_RadioA_8822Bmp.c"
#include "data_RadioB_8822Bmp.c"
#include "data_rtl8822BfwMP.c"

// High Power
#if CFG_HAL_HIGH_POWER_EXT_PA
#include "data_AGC_TAB_8822Bmp_hp.c"
#include "data_PHY_REG_8822Bmp_hp.c"
#include "data_PHY_REG_PG_8822Bmp_Type1.c"
#include "data_PHY_REG_PG_8822Bmp_Type4.c"
#include "data_PHY_REG_PG_8822Bmp_Type6.c"
#include "data_PHY_REG_PG_8822Bmp_Type7.c"
#include "data_PHY_REG_PG_8822Bmp_Type11.c"
#include "data_RadioA_8822Bmp_hp.c"
#include "data_RadioB_8822Bmp_hp.c"
#endif

// Power Tracking
#include "data_TxPowerTrack_AP_8822Bmp.c"


VAR_MAPPING(AGC_TAB_8822Bmp, AGC_TAB_8822Bmp);
VAR_MAPPING(MAC_REG_8822Bmp, MAC_REG_8822Bmp);
VAR_MAPPING(PHY_REG_8822Bmp, PHY_REG_8822Bmp);
VAR_MAPPING(PHY_REG_PG_8822Bmp, PHY_REG_PG_8822Bmp);
VAR_MAPPING(PHY_REG_PG_8822Bmp_Type0, PHY_REG_PG_8822Bmp_Type0);
VAR_MAPPING(PHY_REG_PG_8822Bmp_Type8, PHY_REG_PG_8822Bmp_Type8);
VAR_MAPPING(PHY_REG_PG_8822Bmp_Type9, PHY_REG_PG_8822Bmp_Type9);
VAR_MAPPING(PHY_REG_PG_8822Bmp_Type10, PHY_REG_PG_8822Bmp_Type10);
VAR_MAPPING(PHY_REG_PG_8822Bmp_Type13, PHY_REG_PG_8822Bmp_Type13);
VAR_MAPPING(PHY_REG_PG_8822Bmp_Type14, PHY_REG_PG_8822Bmp_Type14);
VAR_MAPPING(PHY_REG_MP_8822Bmp, PHY_REG_MP_8822Bmp);
VAR_MAPPING(RadioA_8822Bmp, RadioA_8822Bmp);
VAR_MAPPING(RadioB_8822Bmp, RadioB_8822Bmp);
VAR_MAPPING(rtl8822BfwMP, rtl8822BfwMP);

// High Power
#if CFG_HAL_HIGH_POWER_EXT_PA
VAR_MAPPING(AGC_TAB_8822Bmp_hp, AGC_TAB_8822Bmp_hp);
VAR_MAPPING(PHY_REG_8822Bmp_hp, PHY_REG_8822Bmp_hp);
VAR_MAPPING(PHY_REG_PG_8822Bmp_Type1, PHY_REG_PG_8822Bmp_Type1);
VAR_MAPPING(PHY_REG_PG_8822Bmp_Type4, PHY_REG_PG_8822Bmp_Type4);
VAR_MAPPING(PHY_REG_PG_8822Bmp_Type6, PHY_REG_PG_8822Bmp_Type6);
VAR_MAPPING(PHY_REG_PG_8822Bmp_Type7, PHY_REG_PG_8822Bmp_Type7);
VAR_MAPPING(PHY_REG_PG_8822Bmp_Type11, PHY_REG_PG_8822Bmp_Type11);
VAR_MAPPING(RadioA_8822Bmp_hp, RadioA_8822Bmp_hp);
VAR_MAPPING(RadioB_8822Bmp_hp, RadioB_8822Bmp_hp);
#endif

// Power Tracking
VAR_MAPPING(TxPowerTrack_AP_8822Bmp, TxPowerTrack_AP_8822Bmp);


