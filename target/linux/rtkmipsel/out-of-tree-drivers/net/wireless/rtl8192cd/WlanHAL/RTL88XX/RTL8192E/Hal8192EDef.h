#ifndef __HAL8192E_DEF_H__
#define __HAL8192E_DEF_H__

/*++
Copyright (c) Realtek Semiconductor Corp. All rights reserved.

Module Name:
	Hal8192EDef.h
	
Abstract:
	Defined HAL 8192E data structure & Define
	    
Major Change History:
	When       Who               What
	---------- ---------------   -------------------------------
	2012-04-16 Filen            Create.	
--*/

extern u1Byte *data_AGC_TAB_8192E_start,    *data_AGC_TAB_8192E_end;
extern u1Byte *data_MAC_REG_8192E_start,    *data_MAC_REG_8192E_end;
extern u1Byte *data_PHY_REG_8192E_start,    *data_PHY_REG_8192E_end;
//extern u1Byte *data_PHY_REG_1T_8192E_start, *data_PHY_REG_1T_8192E_end;
extern u1Byte *data_PHY_REG_MP_8192E_start, *data_PHY_REG_MP_8192E_end;
#ifdef TXPWR_LMT_92EE
extern u1Byte *data_PHY_REG_PG_8192E_new_start, *data_PHY_REG_PG_8192E_new_end;
#endif
extern u1Byte *data_PHY_REG_PG_8192E_start, *data_PHY_REG_PG_8192E_end;


extern u1Byte *data_RadioA_8192E_start,     *data_RadioA_8192E_end;
extern u1Byte *data_RadioB_8192E_start,     *data_RadioB_8192E_end;

//High Power

#if CFG_HAL_HIGH_POWER_EXT_PA
#ifdef PWR_BY_RATE_92E_HP			
extern u1Byte *data_PHY_REG_PG_8192Emp_hp_start, *data_PHY_REG_PG_8192Emp_hp_end;
#endif
#if CFG_HAL_HIGH_POWER_EXT_LNA
extern u1Byte *data_AGC_TAB_8192E_hp_start,    *data_AGC_TAB_8192E_hp_end;
extern u1Byte *data_PHY_REG_8192E_hp_start,    *data_PHY_REG_8192E_hp_end;
extern u1Byte *data_RadioA_8192E_hp_start,     *data_RadioA_8192E_hp_end;
extern u1Byte *data_RadioB_8192E_hp_start,     *data_RadioB_8192E_hp_end;
#endif
extern u1Byte *data_AGC_TAB_8192E_extpa_start,    *data_AGC_TAB_8192E_extpa_end;
extern u1Byte *data_PHY_REG_8192E_extpa_start,    *data_PHY_REG_8192E_extpa_end;
extern u1Byte *data_RadioA_8192E_extpa_start,     *data_RadioA_8192E_extpa_end;
extern u1Byte *data_RadioB_8192E_extpa_start,     *data_RadioB_8192E_extpa_end;
#endif

#if CFG_HAL_HIGH_POWER_EXT_LNA
#if IS_EXIST_PCI
extern u1Byte *data_AGC_TAB_8192E_extlna_start,    *data_AGC_TAB_8192E_extlna_end;
extern u1Byte *data_AGC_TAB_8192E_extlna_type1_start,   *data_AGC_TAB_8192E_extlna_type1_end;
extern u1Byte *data_AGC_TAB_8192E_extlna_type2_start,   *data_AGC_TAB_8192E_extlna_type2_end;
extern u1Byte *data_AGC_TAB_8192E_extlna_type3_start,   *data_AGC_TAB_8192E_extlna_type3_end;	
extern u1Byte *data_PHY_REG_8192E_extlna_start,    *data_PHY_REG_8192E_extlna_end;
extern u1Byte *data_RadioA_8192E_extlna_start,     *data_RadioA_8192E_extlna_end;
extern u1Byte *data_RadioB_8192E_extlna_start,     *data_RadioB_8192E_extlna_end;
#endif
#if IS_EXIST_SDIO
extern u1Byte *data_AGC_TAB_8192ES_extlna_start,    *data_AGC_TAB_8192ES_extlna_end;
extern u1Byte *data_PHY_REG_8192ES_extlna_start,    *data_PHY_REG_8192ES_extlna_end;
extern u1Byte *data_RadioA_8192ES_extlna_start,     *data_RadioA_8192ES_extlna_end;
extern u1Byte *data_RadioB_8192ES_extlna_start,     *data_RadioB_8192ES_extlna_end;
#endif
#endif

// B-cut support
extern u1Byte *data_MAC_REG_8192Eb_start,    *data_MAC_REG_8192Eb_end;
extern u1Byte *data_PHY_REG_8192Eb_start,    *data_PHY_REG_8192Eb_end;
#if IS_EXIST_PCI
extern u1Byte *data_RadioA_8192Eb_start,     *data_RadioA_8192Eb_end;
extern u1Byte *data_RadioB_8192Eb_start,     *data_RadioB_8192Eb_end;
#endif

// MP chip 
#if IS_EXIST_PCI
extern u1Byte *data_AGC_TAB_8192Emp_start,    *data_AGC_TAB_8192Emp_end;
extern u1Byte *data_RadioA_8192Emp_start,     *data_RadioA_8192Emp_end;
extern u1Byte *data_RadioB_8192Emp_start,     *data_RadioB_8192Emp_end;
extern u1Byte *data_RadioA_8192EmpA_start,     *data_RadioA_8192EmpA_end;
extern u1Byte *data_RadioB_8192EmpA_start,     *data_RadioB_8192EmpA_end;
#endif
#if IS_EXIST_SDIO
extern u1Byte *data_AGC_TAB_8192ES_start,    *data_AGC_TAB_8192ES_end;
extern u1Byte *data_RadioA_8192ES_start,     *data_RadioA_8192ES_end;
extern u1Byte *data_RadioB_8192ES_start,     *data_RadioB_8192ES_end;
#endif
extern u1Byte *data_PHY_REG_MP_8192Emp_start, *data_PHY_REG_MP_8192Emp_end;
extern u1Byte *data_PHY_REG_PG_8192Emp_start, *data_PHY_REG_PG_8192Emp_end;
extern u1Byte *data_MAC_REG_8192Emp_start,    *data_MAC_REG_8192Emp_end;
extern u1Byte *data_PHY_REG_8192Emp_start,    *data_PHY_REG_8192Emp_end;

// FW
#if IS_EXIST_PCI
extern u1Byte *data_rtl8192Efw_start,         *data_rtl8192Efw_end;
extern u1Byte *data_rtl8192EfwMP_start,       *data_rtl8192EfwMP_end;
#endif
#if IS_EXIST_SDIO
extern u1Byte *data_rtl8192ESfwMP_start,       *data_rtl8192ESfwMP_end;
#endif

// Power Tracking
extern u1Byte *data_TxPowerTrack_AP_start,    *data_TxPowerTrack_AP_end;

#ifdef TXPWR_LMT_92EE
extern unsigned char *data_TXPWR_LMT_92EE_new_start, *data_TXPWR_LMT_92EE_new_end;
#ifdef PWR_BY_RATE_92E_HP
#if CFG_HAL_HIGH_POWER_EXT_PA
extern unsigned char *data_TXPWR_LMT_92EE_hp_start, *data_TXPWR_LMT_92EE_hp_end;
#endif
#endif
#endif

//3 MACDM
//default
extern u1Byte *data_MACDM_def_high_8192E_start, *data_MACDM_def_high_8192E_end;
extern u1Byte *data_MACDM_def_low_8192E_start, *data_MACDM_def_low_8192E_end;
extern u1Byte *data_MACDM_def_normal_8192E_start, *data_MACDM_def_normal_8192E_end;

//general
extern u1Byte *data_MACDM_gen_high_8192E_start, *data_MACDM_gen_high_8192E_end;
extern u1Byte *data_MACDM_gen_low_8192E_start, *data_MACDM_gen_low_8192E_end;
extern u1Byte *data_MACDM_gen_normal_8192E_start, *data_MACDM_gen_normal_8192E_end;

//txop
extern u1Byte *data_MACDM_txop_high_8192E_start, *data_MACDM_txop_high_8192E_end;
extern u1Byte *data_MACDM_txop_low_8192E_start, *data_MACDM_txop_low_8192E_end;
extern u1Byte *data_MACDM_txop_normal_8192E_start, *data_MACDM_txop_normal_8192E_end;

//criteria
extern u1Byte *data_MACDM_state_criteria_8192E_start, *data_MACDM_state_criteria_8192E_end;



#endif  //__HAL8192E_DEF_H__


