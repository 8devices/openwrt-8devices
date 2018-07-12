/*++
Copyright (c) Realtek Semiconductor Corp. All rights reserved.

Module Name:
	Hal8881AGen.c
	
Abstract:
	Defined RTL8881A HAL Function
	    
Major Change History:
	When       Who               What
	---------- ---------------   -------------------------------
	2012-03-23 Filen            Create.	
--*/

#if !defined(__ECOS) && !defined(CPTCFG_CFG80211_MODULE)
#include "HalPrecomp.h"
#else
#include "../../HalPrecomp.h"
#endif

#define VAR_MAPPING(dst,src) \
	u1Byte *data_##dst##_start = &data_##src[0]; \
	u1Byte *data_##dst##_end   = &data_##src[sizeof(data_##src)];

//Test chip
#include "../../../data_AGC_TAB_8881A.c"
#include "../../../data_RadioA_8881A.c"
#include "../../../data_MAC_REG_8881A.c"
#include "../../../data_PHY_REG_8881A.c"
#include "../../../data_PHY_REG_PG_8881A.c"

VAR_MAPPING(AGC_TAB_8881A, AGC_TAB_8881A);
VAR_MAPPING(MAC_REG_8881A, MAC_REG_8881A);
VAR_MAPPING(PHY_REG_8881A, PHY_REG_8881A);
VAR_MAPPING(RadioA_8881A, RadioA_8881A);
VAR_MAPPING(PHY_REG_PG_8881A, PHY_REG_PG_8881A);
/////////////////////////////////////////////////

#include "../../../data_AGC_TAB_8881Am.c"
#include "../../../data_AGC_TAB_8881ABP.c"
#include "../../../data_AGC_TAB_8881ABP_intpa.c"
#include "../../../data_AGC_TAB_8881AMP.c"
#ifdef CONFIG_8881A_2LAYER
#include "../../../data_AGC_TAB_8881AMP_2layer.c"
#endif
#include "../../../data_AGC_TAB_8881AMP_intpa.c"
#include "../../../data_AGC_TAB_8881AN.c"
#include "../../../data_AGC_TAB_8881AN_extpa.c"

VAR_MAPPING(AGC_TAB_8881Am, AGC_TAB_8881Am);
VAR_MAPPING(AGC_TAB_8881ABP, AGC_TAB_8881ABP);
VAR_MAPPING(AGC_TAB_8881ABP_intpa, AGC_TAB_8881ABP_intpa);
VAR_MAPPING(AGC_TAB_8881AMP, AGC_TAB_8881AMP);
#ifdef CONFIG_8881A_2LAYER
VAR_MAPPING(AGC_TAB_8881AMP_2layer, AGC_TAB_8881AMP_2layer);
#endif
VAR_MAPPING(AGC_TAB_8881AMP_intpa, AGC_TAB_8881AMP_intpa);
VAR_MAPPING(AGC_TAB_8881AN, AGC_TAB_8881AN);
VAR_MAPPING(AGC_TAB_8881AN_extpa, AGC_TAB_8881AN_extpa);

#include "../../../data_MAC_REG_8881Am.c"

VAR_MAPPING(MAC_REG_8881Am, MAC_REG_8881Am);

#include "../../../data_PHY_REG_8881Am.c"
#include "../../../data_PHY_REG_8881AMP.c"

VAR_MAPPING(PHY_REG_8881Am, PHY_REG_8881Am);
VAR_MAPPING(PHY_REG_8881AMP, PHY_REG_8881AMP);

#include "../../../data_RadioA_8881Am.c"
#include "../../../data_RadioA_8881ABP.c"
#include "../../../data_RadioA_8881ABP_intpa.c"
#include "../../../data_RadioA_8881AMP.c"
#ifdef CONFIG_8881A_2LAYER
#include "../../../data_RadioA_8881AMP_2layer.c"
#endif
#include "../../../data_RadioA_8881AMP_intpa.c"
#include "../../../data_RadioA_8881AN.c"
#include "../../../data_RadioA_8881AN_extpa.c"

VAR_MAPPING(RadioA_8881Am, RadioA_8881Am);
VAR_MAPPING(RadioA_8881ABP, RadioA_8881ABP);
VAR_MAPPING(RadioA_8881ABP_intpa, RadioA_8881ABP_intpa);
VAR_MAPPING(RadioA_8881AMP, RadioA_8881AMP);
#ifdef CONFIG_8881A_2LAYER
VAR_MAPPING(RadioA_8881AMP_2layer, RadioA_8881AMP_2layer);
#endif
VAR_MAPPING(RadioA_8881AMP_intpa, RadioA_8881AMP_intpa);
VAR_MAPPING(RadioA_8881AN, RadioA_8881AN);
VAR_MAPPING(RadioA_8881AN_extpa, RadioA_8881AN_extpa);

#include "../../../data_PHY_REG_PG_8881Am.c"
#include "../../../data_PHY_REG_PG_8881A_new.c"
#include "../../../data_PHY_REG_PG_8881AMP.c"
#include "../../../data_PHY_REG_PG_8881AMP_intpa.c"
#include "../../../data_PHY_REG_PG_8881ABP.c"
#include "../../../data_PHY_REG_PG_8881ABP_intpa.c"
#include "../../../data_PHY_REG_PG_8881AN.c"
#include "../../../data_PHY_REG_PG_8881AN_extpa.c"

VAR_MAPPING(PHY_REG_PG_8881Am, PHY_REG_PG_8881Am);
VAR_MAPPING(PHY_REG_PG_8881A_new, PHY_REG_PG_8881A_new);
VAR_MAPPING(PHY_REG_PG_8881AMP, PHY_REG_PG_8881AMP);
VAR_MAPPING(PHY_REG_PG_8881AMP_intpa, PHY_REG_PG_8881AMP_intpa);
VAR_MAPPING(PHY_REG_PG_8881ABP, PHY_REG_PG_8881ABP);
VAR_MAPPING(PHY_REG_PG_8881ABP_intpa, PHY_REG_PG_8881ABP_intpa);
VAR_MAPPING(PHY_REG_PG_8881AN, PHY_REG_PG_8881AN);
VAR_MAPPING(PHY_REG_PG_8881AN_extpa, PHY_REG_PG_8881AN_extpa);

#include "../../../data_TxPowerTrack_AP_8881A.c"
#include "../../../data_TxPowerTrack_AP_8881AMP.c"
#include "../../../data_TxPowerTrack_AP_8881AMP_intpa.c"
#include "../../../data_TxPowerTrack_AP_8881ABP.c"
#include "../../../data_TxPowerTrack_AP_8881ABP_intpa.c"
#include "../../../data_TxPowerTrack_AP_8881AN.c"
#include "../../../data_TxPowerTrack_AP_8881AN_extpa.c"

VAR_MAPPING(TxPowerTrack_AP_8881A, TxPowerTrack_AP_8881A);
VAR_MAPPING(TxPowerTrack_AP_8881AMP, TxPowerTrack_AP_8881AMP);
VAR_MAPPING(TxPowerTrack_AP_8881AMP_intpa, TxPowerTrack_AP_8881AMP_intpa);
VAR_MAPPING(TxPowerTrack_AP_8881ABP, TxPowerTrack_AP_8881ABP);
VAR_MAPPING(TxPowerTrack_AP_8881ABP_intpa, TxPowerTrack_AP_8881ABP_intpa);
VAR_MAPPING(TxPowerTrack_AP_8881AN, TxPowerTrack_AP_8881AN);
VAR_MAPPING(TxPowerTrack_AP_8881AN_extpa, TxPowerTrack_AP_8881AN_extpa);
#ifdef TXPWR_LMT_8881A
#include "../../../data_TXPWR_LMT_8881A_new.c"
#include "../../../data_TXPWR_LMT_8881AMP.c"
#include "../../../data_TXPWR_LMT_8881AMP_intpa.c"
#include "../../../data_TXPWR_LMT_8881ABP.c"
#include "../../../data_TXPWR_LMT_8881ABP_intpa.c"
#include "../../../data_TXPWR_LMT_8881AN.c"
#include "../../../data_TXPWR_LMT_8881AN_extpa.c"
VAR_MAPPING(TXPWR_LMT_8881A_new,TXPWR_LMT_8881A_new);
VAR_MAPPING(TXPWR_LMT_8881AMP,TXPWR_LMT_8881AMP);
VAR_MAPPING(TXPWR_LMT_8881AMP_intpa,TXPWR_LMT_8881AMP_intpa);
VAR_MAPPING(TXPWR_LMT_8881ABP,TXPWR_LMT_8881ABP);
VAR_MAPPING(TXPWR_LMT_8881ABP_intpa,TXPWR_LMT_8881ABP_intpa);
VAR_MAPPING(TXPWR_LMT_8881AN,TXPWR_LMT_8881AN);
VAR_MAPPING(TXPWR_LMT_8881AN_extpa,TXPWR_LMT_8881AN_extpa);
#endif

#if 1   //Filen, file below should be updated
#include "../../../data_PHY_REG_MP_8881A.c"
#include "../../../data_RTL8881FW_Test_T.c"
#include "../../../data_RTL8881TXBUF_Test_T.c"
#include "../../../data_RTL8881FW_A_CUT_T.c"
#include "../../../data_RTL8881TXBUF_A_CUT_T.c"
#endif

//3 MACDM
//default
#include "../../../data_MACDM_def_high_8881A.c"
#include "../../../data_MACDM_def_low_8881A.c"
#include "../../../data_MACDM_def_normal_8881A.c"
//general
#include "../../../data_MACDM_gen_high_8881A.c"
#include "../../../data_MACDM_gen_low_8881A.c"
#include "../../../data_MACDM_gen_normal_8881A.c"
//txop
#include "../../../data_MACDM_txop_high_8881A.c"
#include "../../../data_MACDM_txop_low_8881A.c"
#include "../../../data_MACDM_txop_normal_8881A.c"
//criteria
#include "../../../data_MACDM_state_criteria_8881A.c"

#if 1   //Filen, file below should be updated
VAR_MAPPING(PHY_REG_MP_8881A, PHY_REG_MP_8881A);
VAR_MAPPING(RTL8881FW_Test_T, RTL8881FW_Test_T);
VAR_MAPPING(RTL8881TXBUF_Test_T,RTL8881TXBUF_Test_T);
VAR_MAPPING(RTL8881FW_A_CUT_T,RTL8881FW_A_CUT_T);
VAR_MAPPING(RTL8881TXBUF_A_CUT_T,RTL8881TXBUF_A_CUT_T);
#endif

//3 MACDM
VAR_MAPPING(MACDM_def_high_8881A, MACDM_def_high_8881A);
VAR_MAPPING(MACDM_def_low_8881A, MACDM_def_low_8881A);
VAR_MAPPING(MACDM_def_normal_8881A, MACDM_def_normal_8881A);

VAR_MAPPING(MACDM_gen_high_8881A, MACDM_gen_high_8881A);
VAR_MAPPING(MACDM_gen_low_8881A, MACDM_gen_low_8881A);
VAR_MAPPING(MACDM_gen_normal_8881A, MACDM_gen_normal_8881A);

VAR_MAPPING(MACDM_txop_high_8881A, MACDM_txop_high_8881A);
VAR_MAPPING(MACDM_txop_low_8881A, MACDM_txop_low_8881A);
VAR_MAPPING(MACDM_txop_normal_8881A, MACDM_txop_normal_8881A);

VAR_MAPPING(MACDM_state_criteria_8881A, MACDM_state_criteria_8881A);



RT_STATUS
StopHW8881A(
    IN  HAL_PADAPTER Adapter
)
{

    // TODO: 
    return  RT_STATUS_SUCCESS;
}


RT_STATUS
InitPON8881A(
    IN  HAL_PADAPTER Adapter,     
    IN  u4Byte     	ClkSel    
)
{
    RT_TRACE_F( COMP_INIT, DBG_LOUD, ("\n") );

    HAL_RTL_W8(REG_RSV_CTRL, 0x00);

	if(ClkSel == XTAL_CLK_SEL_25M) {
		HAL_RTL_W8(REG_AFE_CTRL2, 5);
	} else if (ClkSel == XTAL_CLK_SEL_40M){
		HAL_RTL_W8(REG_AFE_CTRL2, 1);
	}	

	if (!HalPwrSeqCmdParsing88XX(Adapter, PWR_CUT_ALL_MSK, PWR_FAB_ALL_MSK,
			PWR_INTF_PCI_MSK, rtl8881A_card_enable_flow))
    {
        RT_TRACE( COMP_INIT, DBG_SERIOUS, ("%s %d, HalPwrSeqCmdParsing init fail!!!\n", __FUNCTION__, __LINE__));
        return RT_STATUS_FAILURE;
    }

    return RT_STATUS_SUCCESS;    
}


RT_STATUS	
hal_Associate_8881A(
    HAL_PADAPTER            Adapter,
    BOOLEAN			        IsDefaultAdapter
)
{
    PHAL_INTERFACE              pHalFunc = GET_HAL_INTERFACE(Adapter);
    PHAL_DATA_TYPE              pHalData = _GET_HAL_DATA(Adapter);
    

    //
    //Initialization Related
    //
    pHalData->AccessSwapCtrl        = HAL_ACCESS_SWAP_MEM;
    
    pHalFunc->InitPONHandler        = InitPON8881A;
    pHalFunc->InitMACHandler        = InitMAC88XX;
    pHalFunc->InitFirmwareHandler   = InitFirmware88XX;
    pHalFunc->InitHCIDMAMemHandler  = InitHCIDMAMem88XX;
    pHalFunc->InitHCIDMARegHandler  = InitHCIDMAReg88XX;    
#if CFG_HAL_SUPPORT_MBSSID        
    pHalFunc->InitMBSSIDHandler     = InitMBSSID88XX;
    pHalFunc->InitMBIDCAMHandler    = InitMBIDCAM88XX;
#endif  //CFG_HAL_SUPPORT_MBSSID
    pHalFunc->InitVAPIMRHandler     = InitVAPIMR88XX;
    pHalFunc->InitLLT_TableHandler  = InitLLT_Table88XX;
#ifdef MULTI_MAC_CLONE
	pHalFunc->McloneSetMBSSIDHandler 	= McloneSetMBSSID88XX;
	pHalFunc->McloneStopMBSSIDHandler   = McloneStopMBSSID88XX;
#endif
    pHalFunc->SetMBIDCAMHandler     = SetMBIDCAM88XX;


    //
    //Stop Related
    //
#if CFG_HAL_SUPPORT_MBSSID            
    pHalFunc->StopMBSSIDHandler     = StopMBSSID88XX;
#endif  //CFG_HAL_SUPPORT_MBSSID
    pHalFunc->StopHWHandler         = StopHW88XX;
    pHalFunc->StopSWHandler         = StopSW88XX;
    pHalFunc->DisableVXDAPHandler   = DisableVXDAP88XX;
    pHalFunc->StopMBIDCAMHandler    = StopMBIDCAM88XX;


    //
    //ISR Related
    //
    pHalFunc->InitIMRHandler                    = InitIMR88XX;
    pHalFunc->EnableIMRHandler                  = EnableIMR88XX;
    pHalFunc->InterruptRecognizedHandler        = InterruptRecognized88XX;
    pHalFunc->GetInterruptHandler               = GetInterrupt88XX;
    pHalFunc->AddInterruptMaskHandler           = AddInterruptMask88XX;
    pHalFunc->RemoveInterruptMaskHandler        = RemoveInterruptMask88XX;
    pHalFunc->DisableRxRelatedInterruptHandler  = DisableRxRelatedInterrupt88XX;
    pHalFunc->EnableRxRelatedInterruptHandler   = EnableRxRelatedInterrupt88XX;    


    //
    //Tx Related
    //
    pHalFunc->PrepareTXBDHandler            = PrepareTXBD88XX;    
    pHalFunc->FillTxHwCtrlHandler           = FillTxHwCtrl88XX;
    pHalFunc->SyncSWTXBDHostIdxToHWHandler  = SyncSWTXBDHostIdxToHW88XX;
    pHalFunc->TxPollingHandler              = TxPolling88XX;
    pHalFunc->SigninBeaconTXBDHandler       = SigninBeaconTXBD88XX;
    pHalFunc->SetBeaconDownloadHandler      = SetBeaconDownload88XX;
    pHalFunc->FillBeaconDescHandler         = FillBeaconDesc88XX;    
    pHalFunc->GetTxQueueHWIdxHandler        = GetTxQueueHWIdx88XX;
    pHalFunc->MappingTxQueueHandler         = MappingTxQueue88XX;
    pHalFunc->QueryTxConditionMatchHandler  = QueryTxConditionMatch88XX;
	pHalFunc->FillTxDescHandler             = FillTxDesc88XX;
    pHalFunc->FillRsrvPageDescHandler		= FillRsrvPageDesc88XX;

#if CFG_HAL_TX_SHORTCUT
//    pHalFunc->GetShortCutTxDescHandler      = GetShortCutTxDesc88XX;
//    pHalFunc->ReleaseShortCutTxDescHandler  = ReleaseShortCutTxDesc88XX;
    pHalFunc->GetShortCutTxBuffSizeHandler  = GetShortCutTxBuffSize88XX;
    pHalFunc->SetShortCutTxBuffSizeHandler  = SetShortCutTxBuffSize88XX;    
    pHalFunc->CopyShortCutTxDescHandler     = CopyShortCutTxDesc88XX;
    pHalFunc->FillShortCutTxHwCtrlHandler   = FillShortCutTxHwCtrl88XX;    
    pHalFunc->FillShortCutTxDescHandler     = FillShortCutTxDesc88XX;	
#endif // CFG_HAL_TX_SHORTCUT


    //
    //Rx Related
    //
    pHalFunc->PrepareRXBDHandler            = PrepareRXBD88XX;
    pHalFunc->QueryRxDescHandler            = QueryRxDesc88XX;
    pHalFunc->UpdateRXBDInfoHandler         = UpdateRXBDInfo88XX;
    pHalFunc->UpdateRXBDHWIdxHandler        = UpdateRXBDHWIdx88XX;
    pHalFunc->UpdateRXBDHostIdxHandler      = UpdateRXBDHostIdx88XX;      

    //
    // General operation
    //
    pHalFunc->GetChipIDMIMOHandler          =   GetChipIDMIMO88XX;
    pHalFunc->SetHwRegHandler               =   SetHwReg88XX;
    pHalFunc->GetHwRegHandler               =   GetHwReg88XX;
    pHalFunc->SetMACIDSleepHandler          =   SetMACIDSleep88XX;
    pHalFunc->GetMACIDQueueInTXPKTBUFHandler=   GetMACIDQueueInTXPKTBUF88XX;
    pHalFunc->CheckHangHandler              =   CheckHang88XX;


    //
    // Timer Related
    //
    pHalFunc->Timer1SecHandler              =   Timer1Sec88XX;
#if CFG_HAL_MACDM
    InitMACDM88XX(Adapter);
    pHalFunc->Timer1SecDMHandler            =   Timer1SecDM88XX;
#endif //CFG_HAL_MACDM


    //
    // Security Related		
    //
    pHalFunc->CAMReadMACConfigHandler       =   CAMReadMACConfig88XX;
    pHalFunc->CAMEmptyEntryHandler          =   CAMEmptyEntry88XX;
    pHalFunc->CAMFindUsableHandler          =   CAMFindUsable88XX;
    pHalFunc->CAMProgramEntryHandler        =   CAMProgramEntry88XX;


    //
    // PHY/RF Related
    //
    pHalFunc->PHYSetCCKTxPowerHandler       = PHYSetCCKTxPower88XX_AC;
    pHalFunc->PHYSetOFDMTxPowerHandler      = PHYSetOFDMTxPower88XX_AC;
    pHalFunc->PHYUpdateBBRFValHandler       = UpdateBBRFVal88XX_AC;
    pHalFunc->PHYSwBWModeHandler            = SwBWMode88XX_AC;
    // TODO: 8881A Power Tracking should be done
    pHalFunc->TXPowerTrackingHandler        = TXPowerTracking_ThermalMeter_Tmp8881A;
    pHalFunc->PHYSSetRFRegHandler           = PHY_SetRFReg_88XX_AC;    
    pHalFunc->PHYQueryRFRegHandler          = PHY_QueryRFReg_88XX_AC;
    pHalFunc->IsBBRegRangeHandler           = IsBBRegRange88XX;
    

    //
    // Firmware CMD IO related
    //
    pHalData->H2CBufPtr88XX     = 0;
    pHalData->bFWReady          = _FALSE;
    pHalFunc->FillH2CCmdHandler             = FillH2CCmd88XX;
    pHalFunc->UpdateHalRAMaskHandler        = UpdateHalRAMask88XX;
    pHalFunc->UpdateHalMSRRPTHandler        = UpdateHalMSRRPT88XX;    
    pHalFunc->SetAPOffloadHandler           = SetAPOffload88XX;
    pHalFunc->SetRsvdPageHandler	        = SetRsvdPage88XX;
    pHalFunc->GetRsvdPageLocHandler	        = GetRsvdPageLoc88XX;
    pHalFunc->DownloadRsvdPageHandler	    = DownloadRsvdPage88XX;
    pHalFunc->C2HHandler                    = C2HHandler88XX;
    pHalFunc->C2HPacketHandler              = C2HPacket88XX;        
    pHalFunc->GetTxRPTHandler               = GetTxRPTBuf88XX;
    pHalFunc->SetTxRPTHandler               = SetTxRPTBuf88XX;    

    pHalFunc->DumpRxBDescTestHandler = DumpRxBDesc88XX;
    pHalFunc->DumpTxBDescTestHandler = DumpTxBDesc88XX;
    
    return  RT_STATUS_SUCCESS;    
}


