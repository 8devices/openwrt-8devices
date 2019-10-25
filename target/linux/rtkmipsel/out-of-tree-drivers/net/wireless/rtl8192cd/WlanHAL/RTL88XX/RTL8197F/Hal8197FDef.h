#ifndef __HAL8197F_DEF_H__
#define __HAL8197F_DEF_H__

/*++
Copyright (c) Realtek Semiconductor Corp. All rights reserved.

Module Name:
	Hal8197FDef.h
	
Abstract:
	Defined HAL 8197F data structure & Define
	    
Major Change History:
	When       Who               What
	---------- ---------------   -------------------------------
	2012-03-23 Filen            Create.	
--*/


extern u1Byte *data_AGC_TAB_8197F_start,    *data_AGC_TAB_8197F_end;
extern u1Byte *data_MAC_REG_8197F_start,    *data_MAC_REG_8197F_end;
extern u1Byte *data_PHY_REG_8197F_start,    *data_PHY_REG_8197F_end;
extern u1Byte *data_RadioA_8197F_start,     *data_RadioA_8197F_end;
extern u1Byte *data_RadioB_8197F_start,     *data_RadioB_8197F_end;

///////////////////////////////////////////////////////////

extern u1Byte *data_MAC_REG_8197Fm_start,    *data_MAC_REG_8197Fm_end;
extern u1Byte *data_PHY_REG_8197Fm_start,    *data_PHY_REG_8197Fm_end;
extern u1Byte *data_PHY_REG_8197FMP_start,    *data_PHY_REG_8197FMP_end;

extern u1Byte *data_AGC_TAB_8197Fm_start,    *data_AGC_TAB_8197Fm_end;
extern u1Byte *data_AGC_TAB_8197FBP_start,    *data_AGC_TAB_8197FBP_end;
#ifdef CONFIG_8197F_2LAYER
extern u1Byte *data_AGC_TAB_8197FMP_2layer_start,    *data_AGC_TAB_8197FMP_2layer_end;
#endif
extern u1Byte *data_AGC_TAB_8197FMP_start,    *data_AGC_TAB_8197FMP_end;
extern u1Byte *data_AGC_TAB_8197FN_start, *data_AGC_TAB_8197FN_end;
extern u1Byte *data_AGC_TAB_8197FBP_intpa_start,    *data_AGC_TAB_8197FBP_intpa_end;
extern u1Byte *data_AGC_TAB_8197FMP_intpa_start,    *data_AGC_TAB_8197FMP_intpa_end;
extern u1Byte *data_AGC_TAB_8197FN_extpa_start, *data_AGC_TAB_8197FN_extpa_end;

extern u1Byte *data_RadioA_8197Fm_start,     *data_RadioA_8197Fm_end;
extern u1Byte *data_RadioA_8197FBP_start,     *data_RadioA_8197FBP_end;
extern u1Byte *data_RadioA_8197FMP_start,     *data_RadioA_8197FMP_end;
#ifdef CONFIG_8197F_2LAYER
extern u1Byte *data_RadioA_8197FMP_2layer_start,    *data_RadioA_8197FMP_2layer_end;
#endif
extern u1Byte *data_RadioA_8197FN_start,     *data_RadioA_8197FN_end;
extern u1Byte *data_RadioA_8197FBP_intpa_start,     *data_RadioA_8197FBP_intpa_end;
extern u1Byte *data_RadioA_8197FMP_intpa_start,     *data_RadioA_8197FMP_intpa_end;
extern u1Byte *data_RadioA_8197FN_extpa_start,     *data_RadioA_8197FN_extpa_end;

extern u1Byte *data_TxPowerTrack_AP_8197F_start, *data_TxPowerTrack_AP_8197F_end;
extern u1Byte *data_TxPowerTrack_AP_8197FMP_start, *data_TxPowerTrack_AP_8197FMP_end;
extern u1Byte *data_TxPowerTrack_AP_8197FBP_start, *data_TxPowerTrack_AP_8197FBP_end;
extern u1Byte *data_TxPowerTrack_AP_8197FN_start, *data_TxPowerTrack_AP_8197FN_end;
extern u1Byte *data_TxPowerTrack_AP_8197FMP_intpa_start, *data_TxPowerTrack_AP_8197FMP_intpa_end;
extern u1Byte *data_TxPowerTrack_AP_8197FBP_intpa_start, *data_TxPowerTrack_AP_8197FBP_intpa_end;
extern u1Byte *data_TxPowerTrack_AP_8197FN_extpa_start, *data_TxPowerTrack_AP_8197FN_extpa_end;

extern u1Byte *data_PHY_REG_PG_8197Fmp_start, *data_PHY_REG_PG_8197Fmp_end;
extern u1Byte *data_PHY_REG_PG_8197Fmp_Type0_start, *data_PHY_REG_PG_8197Fmp_Type0_end;
extern u1Byte *data_PHY_REG_PG_8197Fmp_Type2_start, *data_PHY_REG_PG_8197Fmp_Type2_end;
extern u1Byte *data_PHY_REG_PG_8197Fmp_Type3_start, *data_PHY_REG_PG_8197Fmp_Type3_end;
extern u1Byte *data_PHY_REG_PG_8197Fmp_Type4_start, *data_PHY_REG_PG_8197Fmp_Type4_end;
extern u1Byte *data_PHY_REG_PG_8197Fmp_Type6_start, *data_PHY_REG_PG_8197Fmp_Type6_end;
#if CFG_HAL_HIGH_POWER_EXT_PA
extern u1Byte *data_PHY_REG_PG_8197Fmp_Type1_start, *data_PHY_REG_PG_8197Fmp_Type1_end;
extern u1Byte *data_PHY_REG_PG_8197Fmp_Type4_start, *data_PHY_REG_PG_8197Fmp_Type4_end;
extern u1Byte *data_PHY_REG_PG_8197Fmp_Type5_start, *data_PHY_REG_PG_8197Fmp_Type5_end;
#endif

#ifdef TXPWR_LMT_8197F
extern unsigned char *data_TXPWR_LMT_8197Fmp_start, *data_TXPWR_LMT_8197Fmp_end;
extern unsigned char *data_TXPWR_LMT_8197Fmp_Type0_start, *data_TXPWR_LMT_8197Fmp_Type0_end;
extern unsigned char *data_TXPWR_LMT_8197Fmp_Type2_start, *data_TXPWR_LMT_8197Fmp_Type2_end;
extern unsigned char *data_TXPWR_LMT_8197Fmp_Type3_start, *data_TXPWR_LMT_8197Fmp_Type3_end;
extern unsigned char *data_TXPWR_LMT_8197Fmp_Type4_start, *data_TXPWR_LMT_8197Fmp_Type4_end;
extern unsigned char *data_TXPWR_LMT_8197Fmp_Type6_start, *data_TXPWR_LMT_8197Fmp_Type6_end;
#if CFG_HAL_HIGH_POWER_EXT_PA
extern unsigned char *data_TXPWR_LMT_8197Fmp_Type1_start, *data_TXPWR_LMT_8197Fmp_Type1_end;
extern unsigned char *data_TXPWR_LMT_8197Fmp_Type4_start, *data_TXPWR_LMT_8197Fmp_Type4_end;
extern unsigned char *data_TXPWR_LMT_8197Fmp_Type5_start, *data_TXPWR_LMT_8197Fmp_Type5_end;
#endif
#ifdef BEAMFORMING_AUTO
extern unsigned char *data_TXPWR_LMT_8197Fmp_TXBF_start, *data_TXPWR_LMT_8197Fmp_TXBF_end;
#endif
#endif

#if 1   //Filen, file below should be updated
extern u1Byte *data_PHY_REG_MP_8197F_start, *data_PHY_REG_MP_8197F_end;
extern u1Byte *data_rtl8197Ffw_start, *data_rtl8197Ffw_end;
extern u1Byte *data_RTL8197FFW_Test_T_start, *data_RTL8197FFW_Test_T_end;
extern u1Byte *data_RTL8197FTXBUF_Test_T_start, *data_RTL8197FTXBUF_Test_T_end;
extern u1Byte *data_RTL8197FFW_A_CUT_T_start, *data_RTL8197FFW_A_CUT_T_end;
extern u1Byte *data_RTL8197FTXBUF_A_CUT_T_start, *data_RTL8197FTXBUF_A_CUT_T_end;
#endif

//3 MACDM
//default
extern u1Byte *data_MACDM_def_high_8197F_start, *data_MACDM_def_high_8197F_end;
extern u1Byte *data_MACDM_def_low_8197F_start, *data_MACDM_def_low_8197F_end;
extern u1Byte *data_MACDM_def_normal_8197F_start, *data_MACDM_def_normal_8197F_end;

//general
extern u1Byte *data_MACDM_gen_high_8197F_start, *data_MACDM_gen_high_8197F_end;
extern u1Byte *data_MACDM_gen_low_8197F_start, *data_MACDM_gen_low_8197F_end;
extern u1Byte *data_MACDM_gen_normal_8197F_start, *data_MACDM_gen_normal_8197F_end;

//txop
extern u1Byte *data_MACDM_txop_high_8197F_start, *data_MACDM_txop_high_8197F_end;
extern u1Byte *data_MACDM_txop_low_8197F_start, *data_MACDM_txop_low_8197F_end;
extern u1Byte *data_MACDM_txop_normal_8197F_start, *data_MACDM_txop_normal_8197F_end;

//criteria
extern u1Byte *data_MACDM_state_criteria_8197F_start, *data_MACDM_state_criteria_8197F_end;


RT_STATUS
StopHW8197F(
    IN  HAL_PADAPTER Adapter
);


RT_STATUS
hal_Associate_8197F(
    HAL_PADAPTER            Adapter,
    BOOLEAN			    IsDefaultAdapter
);


RT_STATUS
InitPON8197F(
    IN  HAL_PADAPTER Adapter,
    IN  u4Byte     	ClkSel        
);












#endif  //__HAL8197F_DEF_H__
