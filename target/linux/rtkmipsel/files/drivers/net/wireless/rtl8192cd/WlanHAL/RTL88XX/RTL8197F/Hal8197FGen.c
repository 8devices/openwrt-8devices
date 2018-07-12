/*++
Copyright (c) Realtek Semiconductor Corp. All rights reserved.

Module Name:
	Hal8197FGen.c
	
Abstract:
	Defined RTL8197F HAL Function
	    
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
#include "../../../data_AGC_TAB_8197F.c"
#include "../../../data_RadioA_8197F.c"
#include "../../../data_RadioB_8197F.c"
#include "../../../data_MAC_REG_8197F.c"
#include "../../../data_PHY_REG_8197F.c"


VAR_MAPPING(AGC_TAB_8197F, AGC_TAB_8197F);
VAR_MAPPING(MAC_REG_8197F, MAC_REG_8197F);
VAR_MAPPING(PHY_REG_8197F, PHY_REG_8197F);
VAR_MAPPING(RadioA_8197F, RadioA_8197F);
VAR_MAPPING(RadioB_8197F, RadioB_8197F);

/////////////////////////////////////////////////


#ifdef CONFIG_8197F_2LAYER
#include "../../../data_AGC_TAB_8197FMP_2layer.c"
#endif
#ifdef CONFIG_8197F_2LAYER
VAR_MAPPING(AGC_TAB_8197FMP_2layer, AGC_TAB_8197FMP_2layer);
#endif

#ifdef CONFIG_8197F_2LAYER
#include "../../../data_RadioA_8197FMP_2layer.c"
#endif
#ifdef CONFIG_8197F_2LAYER
VAR_MAPPING(RadioA_8197FMP_2layer, RadioA_8197FMP_2layer);
#endif

#include "../../../data_PHY_REG_PG_8197Fmp.c"
#include "../../../data_PHY_REG_PG_8197Fmp_Type0.c"
#include "../../../data_PHY_REG_PG_8197Fmp_Type2.c"
#include "../../../data_PHY_REG_PG_8197Fmp_Type3.c"
#include "../../../data_PHY_REG_PG_8197Fmp_Type4.c"
#include "../../../data_PHY_REG_PG_8197Fmp_Type6.c"
#if CFG_HAL_HIGH_POWER_EXT_PA
#include "../../../data_PHY_REG_PG_8197Fmp_Type1.c"
#include "../../../data_PHY_REG_PG_8197Fmp_Type5.c"
#endif

VAR_MAPPING(PHY_REG_PG_8197Fmp, PHY_REG_PG_8197Fmp);
VAR_MAPPING(PHY_REG_PG_8197Fmp_Type0, PHY_REG_PG_8197Fmp_Type0);
VAR_MAPPING(PHY_REG_PG_8197Fmp_Type2, PHY_REG_PG_8197Fmp_Type2);
VAR_MAPPING(PHY_REG_PG_8197Fmp_Type3, PHY_REG_PG_8197Fmp_Type3);
VAR_MAPPING(PHY_REG_PG_8197Fmp_Type4, PHY_REG_PG_8197Fmp_Type4);
VAR_MAPPING(PHY_REG_PG_8197Fmp_Type6, PHY_REG_PG_8197Fmp_Type6);

#if CFG_HAL_HIGH_POWER_EXT_PA
VAR_MAPPING(PHY_REG_PG_8197Fmp_Type1, PHY_REG_PG_8197Fmp_Type1);
VAR_MAPPING(PHY_REG_PG_8197Fmp_Type5, PHY_REG_PG_8197Fmp_Type5);
#endif

#ifdef TXPWR_LMT_8197F
#include "../../../data_TXPWR_LMT_8197Fmp.c"
#include "../../../data_TXPWR_LMT_8197Fmp_Type0.c"
#include "../../../data_TXPWR_LMT_8197Fmp_Type2.c"
#include "../../../data_TXPWR_LMT_8197Fmp_Type3.c"
#include "../../../data_TXPWR_LMT_8197Fmp_Type4.c"
#include "../../../data_TXPWR_LMT_8197Fmp_Type6.c"
VAR_MAPPING(TXPWR_LMT_8197Fmp,TXPWR_LMT_8197Fmp);
VAR_MAPPING(TXPWR_LMT_8197Fmp_Type0,TXPWR_LMT_8197Fmp_Type0);
VAR_MAPPING(TXPWR_LMT_8197Fmp_Type2,TXPWR_LMT_8197Fmp_Type2);
VAR_MAPPING(TXPWR_LMT_8197Fmp_Type3,TXPWR_LMT_8197Fmp_Type3);
VAR_MAPPING(TXPWR_LMT_8197Fmp_Type4,TXPWR_LMT_8197Fmp_Type4);
VAR_MAPPING(TXPWR_LMT_8197Fmp_Type6,TXPWR_LMT_8197Fmp_Type6);
#if CFG_HAL_HIGH_POWER_EXT_PA
#include "../../../data_TXPWR_LMT_8197Fmp_Type1.c"
#include "../../../data_TXPWR_LMT_8197Fmp_Type5.c"
VAR_MAPPING(TXPWR_LMT_8197Fmp_Type1,TXPWR_LMT_8197Fmp_Type1);
VAR_MAPPING(TXPWR_LMT_8197Fmp_Type5,TXPWR_LMT_8197Fmp_Type5);
#endif
#endif

#ifdef BEAMFORMING_AUTO
#include "../../../data_TXPWR_LMT_8197Fmp_TXBF.c"
VAR_MAPPING(TXPWR_LMT_8197Fmp_TXBF,TXPWR_LMT_8197Fmp_TXBF);
#endif

#if 1   //Filen, file below should be updated
#include "../../../data_PHY_REG_MP_8197F.c"
#include "../../../data_rtl8197Ffw.c"

#endif

//3 MACDM
//default
#include "../../../data_MACDM_def_high_8197F.c"
#include "../../../data_MACDM_def_low_8197F.c"
#include "../../../data_MACDM_def_normal_8197F.c"
//general
#include "../../../data_MACDM_gen_high_8197F.c"
#include "../../../data_MACDM_gen_low_8197F.c"
#include "../../../data_MACDM_gen_normal_8197F.c"
//txop
#include "../../../data_MACDM_txop_high_8197F.c"
#include "../../../data_MACDM_txop_low_8197F.c"
#include "../../../data_MACDM_txop_normal_8197F.c"
//criteria
#include "../../../data_MACDM_state_criteria_8197F.c"

#if 1   //Filen, file below should be updated
VAR_MAPPING(PHY_REG_MP_8197F, PHY_REG_MP_8197F);
VAR_MAPPING(rtl8197Ffw, rtl8197Ffw);

#endif

//3 MACDM
VAR_MAPPING(MACDM_def_high_8197F, MACDM_def_high_8197F);
VAR_MAPPING(MACDM_def_low_8197F, MACDM_def_low_8197F);
VAR_MAPPING(MACDM_def_normal_8197F, MACDM_def_normal_8197F);

VAR_MAPPING(MACDM_gen_high_8197F, MACDM_gen_high_8197F);
VAR_MAPPING(MACDM_gen_low_8197F, MACDM_gen_low_8197F);
VAR_MAPPING(MACDM_gen_normal_8197F, MACDM_gen_normal_8197F);

VAR_MAPPING(MACDM_txop_high_8197F, MACDM_txop_high_8197F);
VAR_MAPPING(MACDM_txop_low_8197F, MACDM_txop_low_8197F);
VAR_MAPPING(MACDM_txop_normal_8197F, MACDM_txop_normal_8197F);

VAR_MAPPING(MACDM_state_criteria_8197F, MACDM_state_criteria_8197F);



RT_STATUS
StopHW8197F(
    IN  HAL_PADAPTER Adapter
)
{

    // TODO: 
    return  RT_STATUS_SUCCESS;
}


RT_STATUS
InitPON8197F(
    IN  HAL_PADAPTER Adapter,     
    IN  u4Byte     	ClkSel    
)
{
    RT_TRACE_F( COMP_INIT, DBG_LOUD, ("\n") );

    HAL_RTL_W8(REG_RSV_CTRL, 0x00);

    if(ClkSel == XTAL_CLK_SEL_25M) {
        HAL_RTL_W32(REG_AFE_CTRL5, (HAL_RTL_R32(REG_AFE_CTRL5) & ~(BIT_MASK_REF_SEL<<BIT_SHIFT_REF_SEL)) | BIT_REF_SEL(1));
    } else if (ClkSel == XTAL_CLK_SEL_40M){
        HAL_RTL_W32(REG_AFE_CTRL5, (HAL_RTL_R32(REG_AFE_CTRL5) & ~(BIT_MASK_REF_SEL<<BIT_SHIFT_REF_SEL)) | BIT_REF_SEL(0));
    }	

    if (!HalPwrSeqCmdParsing88XX(Adapter, PWR_CUT_ALL_MSK, PWR_FAB_ALL_MSK,
        PWR_INTF_PCI_MSK, rtl8197F_card_enable_flow))
    {
        RT_TRACE( COMP_INIT, DBG_SERIOUS, ("%s %d, HalPwrSeqCmdParsing init fail!!!\n", __FUNCTION__, __LINE__));
        return RT_STATUS_FAILURE;
    }

    return RT_STATUS_SUCCESS;    
}


RT_STATUS	
hal_Associate_8197F(
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
    
    pHalFunc->InitPONHandler        = InitPON8197F;
    pHalFunc->InitMACHandler        = InitMAC88XX;
    pHalFunc->InitFirmwareHandler   = InitMIPSFirmware88XX;
    pHalFunc->InitHCIDMAMemHandler  = InitHCIDMAMem88XX;
    pHalFunc->InitHCIDMARegHandler  = InitHCIDMAReg88XX;    
#if CFG_HAL_SUPPORT_MBSSID        
    pHalFunc->InitMBSSIDHandler     = InitMBSSID88XX;
    pHalFunc->InitMBIDCAMHandler    = InitMBIDCAM88XX;
#endif  //CFG_HAL_SUPPORT_MBSSID
    pHalFunc->InitVAPIMRHandler     = InitVAPIMR88XX;
    pHalFunc->InitLLT_TableHandler  = InitLLT_Table88XX_V1;
    pHalFunc->SetMBIDCAMHandler     = SetMBIDCAM88XX;
#if CFG_HAL_HW_FILL_MACID
    pHalFunc->InitMACIDSearchHandler    = InitMACIDSearch88XX;            
    pHalFunc->CheckHWMACIDResultHandler = CheckHWMACIDResult88XX;            
#endif //CFG_HAL_HW_FILL_MACID

#ifdef MULTI_MAC_CLONE
    pHalFunc->McloneSetMBSSIDHandler    = McloneSetMBSSID88XX;
    pHalFunc->McloneStopMBSSIDHandler   = McloneStopMBSSID88XX;
#endif //CFG_HAL_HW_FILL_MACID


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
    pHalFunc->FillBeaconDescHandler         = FillBeaconDesc88XX_V1;    
    pHalFunc->GetTxQueueHWIdxHandler        = GetTxQueueHWIdx88XX;
    pHalFunc->MappingTxQueueHandler         = MappingTxQueue88XX;
    pHalFunc->QueryTxConditionMatchHandler  = QueryTxConditionMatch88XX;
    pHalFunc->FillTxDescHandler             = FillTxDesc88XX_V1;
    pHalFunc->FillShortCutTxDescHandler     = FillShortCutTxDesc88XX_V1;
    pHalFunc->FillRsrvPageDescHandler		= FillRsrvPageDesc88XX_V1; 

#if CFG_HAL_TX_SHORTCUT
//    pHalFunc->GetShortCutTxDescHandler      = GetShortCutTxDesc88XX;
//    pHalFunc->ReleaseShortCutTxDescHandler  = ReleaseShortCutTxDesc88XX;
    pHalFunc->GetShortCutTxBuffSizeHandler  = GetShortCutTxBuffSize88XX_V1;
    pHalFunc->SetShortCutTxBuffSizeHandler  = SetShortCutTxBuffSize88XX_V1;    
    pHalFunc->CopyShortCutTxDescHandler     = CopyShortCutTxDesc88XX;
    pHalFunc->FillShortCutTxHwCtrlHandler   = FillShortCutTxHwCtrl88XX;    
#if CFG_HAL_HW_TX_SHORTCUT_REUSE_TXDESC        
    pHalFunc->FillHwShortCutTxDescHandler   = FillHwShortCutTxDesc88XX_V1;        
#endif
#endif // CFG_HAL_TX_SHORTCUT
    pHalFunc->ReleaseOnePacketHandler       = ReleaseOnePacket88XX;                  

    //
    //Rx Related
    //
    pHalFunc->PrepareRXBDHandler            = PrepareRXBD88XX;
    pHalFunc->QueryRxDescHandler            = QueryRxDesc88XX_V1;
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
    pHalFunc->PHYSetCCKTxPowerHandler       = PHYSetCCKTxPower88XX_N;
    pHalFunc->PHYSetOFDMTxPowerHandler      = PHYSetOFDMTxPower88XX_N;
    pHalFunc->PHYUpdateBBRFValHandler       = HalGeneralDummy;
    pHalFunc->PHYSwBWModeHandler            = SwBWMode88XX_97F;
    // TODO: 8197F Power Tracking should be done
    pHalFunc->TXPowerTrackingHandler        = TXPowerTracking_ThermalMeter_Tmp8197F;
    pHalFunc->PHYSSetRFRegHandler           = PHY_SetRFReg_8197F;    
    pHalFunc->PHYQueryRFRegHandler          = PHY_QueryRFReg_8197F;
    pHalFunc->IsBBRegRangeHandler           = IsBBRegRange88XX_V1;
    

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
    pHalFunc->InitDDMAHandler               = InitDDMA88XX;
    pHalFunc->SetStatistic_ReportHandler    = SetStatistic_Report88XX;
#if CFG_HAL_HW_FILL_MACID
    pHalFunc->SetCRC5ToRPTBufferHandler     = SetCRC5ToRPTBuffer88XX;        
#endif //#if CFG_HAL_HW_FILL_MACID

    pHalFunc->DumpRxBDescTestHandler = DumpRxBDesc88XX;
    pHalFunc->DumpTxBDescTestHandler = DumpTxBDesc88XX;
    
    return  RT_STATUS_SUCCESS;    
}


