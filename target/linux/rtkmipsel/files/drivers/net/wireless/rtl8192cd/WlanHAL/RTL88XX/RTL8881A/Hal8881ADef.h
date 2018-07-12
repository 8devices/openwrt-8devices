#ifndef __HAL8881A_DEF_H__
#define __HAL8881A_DEF_H__

/*++
Copyright (c) Realtek Semiconductor Corp. All rights reserved.

Module Name:
	Hal8881ADef.h
	
Abstract:
	Defined HAL 8881A data structure & Define
	    
Major Change History:
	When       Who               What
	---------- ---------------   -------------------------------
	2012-03-23 Filen            Create.	
--*/


extern u1Byte *data_AGC_TAB_8881A_start,    *data_AGC_TAB_8881A_end;
extern u1Byte *data_MAC_REG_8881A_start,    *data_MAC_REG_8881A_end;
extern u1Byte *data_PHY_REG_8881A_start,    *data_PHY_REG_8881A_end;
extern u1Byte *data_RadioA_8881A_start,     *data_RadioA_8881A_end;
extern u1Byte *data_PHY_REG_PG_8881A_start, *data_PHY_REG_PG_8881A_end;

///////////////////////////////////////////////////////////

extern u1Byte *data_MAC_REG_8881Am_start,    *data_MAC_REG_8881Am_end;
extern u1Byte *data_PHY_REG_8881Am_start,    *data_PHY_REG_8881Am_end;
extern u1Byte *data_PHY_REG_8881AMP_start,    *data_PHY_REG_8881AMP_end;

extern u1Byte *data_AGC_TAB_8881Am_start,    *data_AGC_TAB_8881Am_end;
extern u1Byte *data_AGC_TAB_8881ABP_start,    *data_AGC_TAB_8881ABP_end;
#ifdef CONFIG_8881A_2LAYER
extern u1Byte *data_AGC_TAB_8881AMP_2layer_start,    *data_AGC_TAB_8881AMP_2layer_end;
#endif
extern u1Byte *data_AGC_TAB_8881AMP_start,    *data_AGC_TAB_8881AMP_end;
extern u1Byte *data_AGC_TAB_8881AN_start, *data_AGC_TAB_8881AN_end;
extern u1Byte *data_AGC_TAB_8881ABP_intpa_start,    *data_AGC_TAB_8881ABP_intpa_end;
extern u1Byte *data_AGC_TAB_8881AMP_intpa_start,    *data_AGC_TAB_8881AMP_intpa_end;
extern u1Byte *data_AGC_TAB_8881AN_extpa_start, *data_AGC_TAB_8881AN_extpa_end;

extern u1Byte *data_RadioA_8881Am_start,     *data_RadioA_8881Am_end;
extern u1Byte *data_RadioA_8881ABP_start,     *data_RadioA_8881ABP_end;
extern u1Byte *data_RadioA_8881AMP_start,     *data_RadioA_8881AMP_end;
#ifdef CONFIG_8881A_2LAYER
extern u1Byte *data_RadioA_8881AMP_2layer_start,    *data_RadioA_8881AMP_2layer_end;
#endif
extern u1Byte *data_RadioA_8881AN_start,     *data_RadioA_8881AN_end;
extern u1Byte *data_RadioA_8881ABP_intpa_start,     *data_RadioA_8881ABP_intpa_end;
extern u1Byte *data_RadioA_8881AMP_intpa_start,     *data_RadioA_8881AMP_intpa_end;
extern u1Byte *data_RadioA_8881AN_extpa_start,     *data_RadioA_8881AN_extpa_end;

extern u1Byte *data_TxPowerTrack_AP_8881A_start, *data_TxPowerTrack_AP_8881A_end;
extern u1Byte *data_TxPowerTrack_AP_8881AMP_start, *data_TxPowerTrack_AP_8881AMP_end;
extern u1Byte *data_TxPowerTrack_AP_8881ABP_start, *data_TxPowerTrack_AP_8881ABP_end;
extern u1Byte *data_TxPowerTrack_AP_8881AN_start, *data_TxPowerTrack_AP_8881AN_end;
extern u1Byte *data_TxPowerTrack_AP_8881AMP_intpa_start, *data_TxPowerTrack_AP_8881AMP_intpa_end;
extern u1Byte *data_TxPowerTrack_AP_8881ABP_intpa_start, *data_TxPowerTrack_AP_8881ABP_intpa_end;
extern u1Byte *data_TxPowerTrack_AP_8881AN_extpa_start, *data_TxPowerTrack_AP_8881AN_extpa_end;

extern u1Byte *data_PHY_REG_PG_8881Am_start, *data_PHY_REG_PG_8881Am_end;
extern u1Byte *data_PHY_REG_PG_8881A_new_start, *data_PHY_REG_PG_8881A_new_end;
extern u1Byte *data_PHY_REG_PG_8881AMP_start, *data_PHY_REG_PG_8881AMP_end;
extern u1Byte *data_PHY_REG_PG_8881ABP_start, *data_PHY_REG_PG_8881ABP_end;
extern u1Byte *data_PHY_REG_PG_8881AN_start, *data_PHY_REG_PG_8881AN_end;
extern u1Byte *data_PHY_REG_PG_8881AMP_intpa_start, *data_PHY_REG_PG_8881AMP_intpa_end;
extern u1Byte *data_PHY_REG_PG_8881ABP_intpa_start, *data_PHY_REG_PG_8881ABP_intpa_end;
extern u1Byte *data_PHY_REG_PG_8881AN_extpa_start, *data_PHY_REG_PG_8881AN_extpa_end;
#ifdef TXPWR_LMT_8881A
extern unsigned char *data_TXPWR_LMT_8881A_new_start, *data_TXPWR_LMT_8881A_new_end;
extern unsigned char *data_TXPWR_LMT_8881AMP_start, *data_TXPWR_LMT_8881AMP_end;
extern unsigned char *data_TXPWR_LMT_8881ABP_start, *data_TXPWR_LMT_8881ABP_end;
extern unsigned char *data_TXPWR_LMT_8881AN_start, *data_TXPWR_LMT_8881AN_end;
extern unsigned char *data_TXPWR_LMT_8881AMP_intpa_start, *data_TXPWR_LMT_8881AMP_intpa_end;
extern unsigned char *data_TXPWR_LMT_8881ABP_intpa_start, *data_TXPWR_LMT_8881ABP_intpa_end;
extern unsigned char *data_TXPWR_LMT_8881AN_extpa_start, *data_TXPWR_LMT_8881AN_extpa_end;
#endif

#if 1   //Filen, file below should be updated
extern u1Byte *data_PHY_REG_MP_8881A_start, *data_PHY_REG_MP_8881A_end;
extern u1Byte *data_RTL8881FW_Test_T_start, *data_RTL8881FW_Test_T_end;
extern u1Byte *data_RTL8881TXBUF_Test_T_start, *data_RTL8881TXBUF_Test_T_end;
extern u1Byte *data_RTL8881FW_A_CUT_T_start, *data_RTL8881FW_A_CUT_T_end;
extern u1Byte *data_RTL8881TXBUF_A_CUT_T_start, *data_RTL8881TXBUF_A_CUT_T_end;
#endif

//3 MACDM
//default
extern u1Byte *data_MACDM_def_high_8881A_start, *data_MACDM_def_high_8881A_end;
extern u1Byte *data_MACDM_def_low_8881A_start, *data_MACDM_def_low_8881A_end;
extern u1Byte *data_MACDM_def_normal_8881A_start, *data_MACDM_def_normal_8881A_end;

//general
extern u1Byte *data_MACDM_gen_high_8881A_start, *data_MACDM_gen_high_8881A_end;
extern u1Byte *data_MACDM_gen_low_8881A_start, *data_MACDM_gen_low_8881A_end;
extern u1Byte *data_MACDM_gen_normal_8881A_start, *data_MACDM_gen_normal_8881A_end;

//txop
extern u1Byte *data_MACDM_txop_high_8881A_start, *data_MACDM_txop_high_8881A_end;
extern u1Byte *data_MACDM_txop_low_8881A_start, *data_MACDM_txop_low_8881A_end;
extern u1Byte *data_MACDM_txop_normal_8881A_start, *data_MACDM_txop_normal_8881A_end;

//criteria
extern u1Byte *data_MACDM_state_criteria_8881A_start, *data_MACDM_state_criteria_8881A_end;


RT_STATUS
StopHW8881A(
    IN  HAL_PADAPTER Adapter
);


RT_STATUS
hal_Associate_8881A(
    HAL_PADAPTER            Adapter,
    BOOLEAN			    IsDefaultAdapter
);


RT_STATUS
InitPON8881A(
    IN  HAL_PADAPTER Adapter,
    IN  u4Byte     	ClkSel        
);












#endif  //__HAL8881A_DEF_H__
