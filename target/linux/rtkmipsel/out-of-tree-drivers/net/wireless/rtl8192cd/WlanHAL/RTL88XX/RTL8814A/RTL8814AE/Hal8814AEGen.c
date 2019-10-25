/*++
Copyright (c) Realtek Semiconductor Corp. All rights reserved.

Module Name:
	Hal8814AEGen.c
	
Abstract:
	Defined RTL8814AE HAL Function
	    
Major Change History:
	When       Who               What
	---------- ---------------   -------------------------------
	2013-05-28 Filen            Create.	
--*/

#if !defined(__ECOS) && !defined(CPTCFG_CFG80211_MODULE)
#include "HalPrecomp.h"
#else
#include "../../../HalPrecomp.h"
#endif

RT_STATUS
InitPON8814AE(
    IN  HAL_PADAPTER Adapter,
    IN  u4Byte     	ClkSel        
)
{
	HAL_PADAPTER priv = Adapter;
    u32     bytetmp;
    u32     retry;
    u1Byte	u1btmp;
	
    RT_TRACE_F( COMP_INIT, DBG_LOUD, ("\n"));

    // TODO: Filen, first write IO will fail, don't know the root cause
	DEBUG_INFO("0: Reg0x0: 0x%x, Reg0x4: 0x%x, Reg0x1C: 0x%x\n", HAL_RTL_R32(0x0), HAL_RTL_R32(0x4), HAL_RTL_R32(0x1C));
	HAL_RTL_W8(REG_RSV_CTRL, 0x00);
    DEBUG_INFO("1: Reg0x0: 0x%x, Reg0x4: 0x%x, Reg0x1C: 0x%x\n", HAL_RTL_R32(0x0), HAL_RTL_R32(0x4), HAL_RTL_R32(0x1C));
	HAL_RTL_W8(REG_RSV_CTRL, 0x00);
    DEBUG_INFO("2: Reg0x0: 0x%x, Reg0x4: 0x%x, Reg0x1C: 0x%x\n", HAL_RTL_R32(0x0), HAL_RTL_R32(0x4), HAL_RTL_R32(0x1C));

    // TODO: Filen, check 8814A setting
	if(ClkSel == XTAL_CLK_SEL_25M) {
	} else if (ClkSel == XTAL_CLK_SEL_40M){
	}	

	// YX sugguested 2014.06.03
	u1btmp = PlatformEFIORead1Byte(Adapter, 0x10C2);
	PlatformEFIOWrite1Byte(Adapter, 0x10C2, (u1btmp | BIT1));
	
	if (!HalPwrSeqCmdParsing88XX(Adapter, PWR_CUT_ALL_MSK, PWR_FAB_ALL_MSK,
			PWR_INTF_PCI_MSK, rtl8814A_card_enable_flow))
    {
        RT_TRACE( COMP_INIT, DBG_SERIOUS, ("%s %d, HalPwrSeqCmdParsing init fail!!!\n", __FUNCTION__, __LINE__));
        return RT_STATUS_FAILURE;
    }

	DEBUG_INFO("3: Reg0x0: 0x%x, Reg0x4: 0x%x, Reg0x1C: 0x%x\n", HAL_RTL_R32(0x0), HAL_RTL_R32(0x4), HAL_RTL_R32(0x1C));

#ifdef RTL_8814A_MP_TEMP

    HAL_RTL_W32(REG_BD_RWPTR_CLR,0xffffffff);

    HAL_RTL_W32(0x1000, HAL_RTL_R32(0x1000)|BIT16|BIT17);
    DEBUG_INFO("%s(%d): 0x1000:0x%x \n", __FUNCTION__, __LINE__, HAL_RTL_R32(0x1000));
#endif

    return  RT_STATUS_SUCCESS;
}


RT_STATUS
StopHW8814AE(
    IN  HAL_PADAPTER Adapter
)
{
    // TODO:

    return RT_STATUS_SUCCESS;
}


RT_STATUS
ResetHWForSurprise8814AE(
    IN  HAL_PADAPTER Adapter
)
{
    // TODO: Filen, necessary to be added code here

    return RT_STATUS_SUCCESS;
}


RT_STATUS	
hal_Associate_8814AE(
	 HAL_PADAPTER        Adapter,
    BOOLEAN             IsDefaultAdapter
)
{
    PHAL_INTERFACE              pHalFunc = GET_HAL_INTERFACE(Adapter);
    PHAL_DATA_TYPE              pHalData = _GET_HAL_DATA(Adapter);

    //
    //Initialization Related
    //
    pHalData->AccessSwapCtrl        = HAL_ACCESS_SWAP_MEM;
    pHalFunc->InitPONHandler        = InitPON8814AE;
    pHalFunc->InitMACHandler        = InitMAC88XX;
    pHalFunc->InitFirmwareHandler   = InitMIPSFirmware88XX;
    pHalFunc->InitHCIDMAMemHandler  = InitHCIDMAMem88XX;
    pHalFunc->InitHCIDMARegHandler  = InitHCIDMAReg88XX;    
#if CFG_HAL_SUPPORT_MBSSID    
    pHalFunc->InitMBSSIDHandler     = InitMBSSID88XX;
	pHalFunc->InitMBIDCAMHandler	= InitMBIDCAM88XX;
#endif  //CFG_HAL_SUPPORT_MBSSID
    pHalFunc->InitVAPIMRHandler     = InitVAPIMR88XX;
    pHalFunc->InitLLT_TableHandler  = InitLLT_Table88XX_V1;
#if CFG_HAL_HW_FILL_MACID
    pHalFunc->InitMACIDSearchHandler    = InitMACIDSearch88XX;            
    pHalFunc->CheckHWMACIDResultHandler = CheckHWMACIDResult88XX;            
#endif //CFG_HAL_HW_FILL_MACID
#ifdef MULTI_MAC_CLONE
	pHalFunc->McloneSetMBSSIDHandler	= McloneSetMBSSID88XX;
	pHalFunc->McloneStopMBSSIDHandler	= McloneStopMBSSID88XX;
#endif //CFG_HAL_HW_FILL_MACID
    pHalFunc->SetMBIDCAMHandler     = SetMBIDCAM88XX;
    pHalFunc->InitVAPIMRHandler     = InitVAPIMR88XX;

    //
    //Stop Related
    //
#if CFG_HAL_SUPPORT_MBSSID        
    pHalFunc->StopMBSSIDHandler     = StopMBSSID88XX;
#endif  //CFG_HAL_SUPPORT_MBSSID
    pHalFunc->StopHWHandler         = StopHW88XX;
    pHalFunc->StopSWHandler         = StopSW88XX;
    pHalFunc->DisableVXDAPHandler   = DisableVXDAP88XX;
    pHalFunc->ResetHWForSurpriseHandler     = ResetHWForSurprise8814AE;


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
#if CFG_HAL_TX_SHORTCUT //eric-8822
    pHalFunc->FillShortCutTxDescHandler     = FillShortCutTxDesc88XX;
//    pHalFunc->GetShortCutTxDescHandler      = GetShortCutTxDesc88XX;
//    pHalFunc->ReleaseShortCutTxDescHandler  = ReleaseShortCutTxDesc88XX;
    pHalFunc->GetShortCutTxBuffSizeHandler  = GetShortCutTxBuffSize88XX;
    pHalFunc->SetShortCutTxBuffSizeHandler  = SetShortCutTxBuffSize88XX;
    pHalFunc->CopyShortCutTxDescHandler     = CopyShortCutTxDesc88XX;
    pHalFunc->FillShortCutTxHwCtrlHandler   = FillShortCutTxHwCtrl88XX;    
#if CFG_HAL_HW_TX_SHORTCUT_REUSE_TXDESC    
    pHalFunc->FillHwShortCutTxDescHandler   = FillHwShortCutTxDesc88XX;        
#endif
#endif // CFG_HAL_TX_SHORTCUT
    pHalFunc->ReleaseOnePacketHandler       = ReleaseOnePacket88XX;                  

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
	pHalFunc->CheckHangHandler              =   CheckHang88XX;
    pHalFunc->GetMACIDQueueInTXPKTBUFHandler=   GetMACIDQueueInTXPKTBUF88XX;

    //
    // Timer Related
    //
    pHalFunc->Timer1SecHandler              =   Timer1Sec88XX;


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
    pHalFunc->PHYSwBWModeHandler            = SwBWMode88XX_AC;
    pHalFunc->PHYUpdateBBRFValHandler       = UpdateBBRFVal88XX_AC;
    // TODO: 8814A Power Tracking should be done
    pHalFunc->TXPowerTrackingHandler        = TXPowerTracking_ThermalMeter_Tmp8814A;
    pHalFunc->PHYSSetRFRegHandler           = PHY_SetRFReg_88XX_AC;
    pHalFunc->PHYQueryRFRegHandler          = PHY_QueryRFReg_8814;
    pHalFunc->IsBBRegRangeHandler           = IsBBRegRange88XX_V1;
    pHalFunc->PHYSetSecCCATHbyRXANT         = PHY_Set_SecCCATH_by_RXANT_8814A;
    pHalFunc->PHYSpurCalibration            = phy_SpurCalibration_8814A;
    pHalFunc->PHYConfigBB                   = phy_ConfigBB_8814A;


    //
    // Firmware CMD IO related
    //
    pHalData->H2CBufPtr88XX     = 0;
    pHalData->bFWReady          = _FALSE;
    // TODO: code below should be sync with new 3081 FW
    pHalFunc->FillH2CCmdHandler             = FillH2CCmd88XX;
    pHalFunc->UpdateHalRAMaskHandler        = UpdateHalRAMask8814A;
    pHalFunc->UpdateHalMSRRPTHandler        = UpdateHalMSRRPT88XX;
    pHalFunc->SetAPOffloadHandler           = HalGeneralDummy;
#ifdef AP_PS_Offlaod
    pHalFunc->SetAPPSOffloadHandler         = SetAPPSOffload88XX;
    pHalFunc->APPSOffloadMACIDPauseHandler  = APPSOffloadMacidPauseCtrl88XX;
#endif    
    pHalFunc->SetRsvdPageHandler	        = HalGeneralDummy;
    pHalFunc->GetRsvdPageLocHandler	        = HalGeneralDummy;
    pHalFunc->DownloadRsvdPageHandler	    = HalGeneralDummy;
    pHalFunc->C2HHandler                    = HalGeneralDummy;
    pHalFunc->C2HPacketHandler              = C2HPacketHandler_8814A;//C2HPacket88XX;    
    pHalFunc->GetTxRPTHandler               = GetTxRPTBuf88XX;
    pHalFunc->SetTxRPTHandler               = SetTxRPTBuf88XX;    
#if CFG_HAL_HW_FILL_MACID
    pHalFunc->SetCRC5ToRPTBufferHandler     = SetCRC5ToRPTBuffer88XX;        
#endif //#if CFG_HAL_HW_FILL_MACID
    pHalFunc->DumpRxBDescTestHandler        = DumpRxBDesc88XX;
    pHalFunc->DumpTxBDescTestHandler        = DumpTxBDesc88XX;
    
    return  RT_STATUS_SUCCESS;    
}


void 
InitMAC8814AE(
    IN  HAL_PADAPTER Adapter
)
{


    
}


VOID
_C2HContentParsing8814A(
	struct rtl8192cd_priv *priv,
		u1Byte			c2hCmdId, 
		u1Byte			c2hCmdLen,
		pu1Byte 			tmpBuf
)

{
	static u1Byte CmdBuffer = 0x1;
	u1Byte i;
	u1Byte	Extend_c2hSubID = 0;
		
	switch(c2hCmdId)
	{
	
	case C2H_88XX_LB:
		// get c2h loopback
		printk("c2h content c2hCmdId = 0x%x c2hCmdLen= 0x%x \n", c2hCmdId, c2hCmdLen);

		for(i=0;i<c2hCmdLen;i++)
		{
			printk("%x ",*(tmpBuf+i));    
		}

		printk("\n");

		CmdBuffer++;

		delay_ms(100);
		// issue h2c 0xE0 back
		printk("GEN h2c cmd CmdBuffer = %x \n",CmdBuffer);        
		FillH2CCmd88XX(priv,0xE0,1,&CmdBuffer);        
		//  FillH2CCmd88XX(Adapter,0xE0,1,&CmdBuffer);                
		break;

	case C2H_88XX_TX_RATE:
		#ifdef TXREPORT	
		APReqTXRptHandler(priv,tmpBuf);
		#ifdef TXRETRY_CNT
			requestTxRetry88XX(priv);
		#else
			requestTxReport88XX(priv);
		#endif
		#endif			
		break;
#ifdef TXRETRY_CNT
		case C2H_88XX_TX_RETRY:
			C2HTxTxRetryHandler(priv, tmpBuf);
			requestTxReport88XX(priv);	
			break;
#endif

	case C2H_88XX_TXBF:
		#if (BEAMFORMING_SUPPORT == 1)
		C2HTxBeamformingHandler88XX(priv, tmpBuf, c2hCmdLen);
		#endif
		break;

	case C2H_88XX_RA_PARA_RPT:
		ODM_C2HRaParaReportHandler(&(priv->pshare->_dmODM), tmpBuf, c2hCmdLen);
		break;
		
	case C2H_88XX_RA_DYNAMIC_TX_PATH_RPT:
		phydm_c2h_dtp_handler(&(priv->pshare->_dmODM), tmpBuf, c2hCmdLen);
		//DbgPrint("[C2H] C2H_8192E_RA_PARA_RPT \n");
		break;
		
	case C2H_88XX_EXTEND_IND:	
		Extend_c2hSubID= tmpBuf[0];
		if(Extend_c2hSubID == EXTEND_C2H_88XX_DBG_PRINT)
		{
			phydm_fw_trace_handler_8051(&(priv->pshare->_dmODM), tmpBuf, c2hCmdLen);
		}
		break;

	default:
		if(!(phydm_c2H_content_parsing(&(priv->pshare->_dmODM), c2hCmdId, c2hCmdLen, tmpBuf))) {
			DbgPrint("[C2H], Unkown packet!! CmdId(%#X)!\n", c2hCmdId);
		}
		break;
	}
}

VOID
C2HPacketHandler_8814A(
	struct rtl8192cd_priv *priv,
		pu1Byte 		Buffer,
		u1Byte			Length
)
{
	u1Byte	c2hCmdId=0, c2hCmdSeq=0, c2hCmdLen=0;
	pu1Byte tmpBuf=NULL;
	c2hCmdId = *(Buffer);
	c2hCmdSeq = *(Buffer+1);

    //RT_PRINT_DATA(COMP_SEND, DBG_TRACE, "C2H packet :\n", pBuf, length); 
#if 0 //remove by eric, for avoiding system hang
#if IS_SUPPORT_MACHAL_API
    if(IS_HARDWARE_MACHAL_SUPPORT(Adapter))
    {
        GET_MACHALAPI_INTERFACE(Adapter)->halmac_get_c2h_info(Adapter->pHalmac_adapter,pBuf,(u4Byte)length);
    }
#endif // IS_SUPPORT_MACHAL_API
#if CFG_HAL_WLAN_SUPPORT_H2C_PACKET
#if IS_SUPPORT_MACHAL_API
        else
#endif        
            ParseC2HPacket88XX(Adapter, pBuf, length);   //for new format c2h, including ack...etc
#endif    
#endif

    //printk("get c2h packet ,c2hCmdId =%x , length =%x \n",c2hCmdId,length);
	/*
	if(c2hCmdId==C2H_88XX_EXTEND_IND)
	{
		c2hCmdLen = length;
		tmpBuf = pBuf;
		C2HExtEventHandler88XX(Adapter, c2hCmdId, c2hCmdLen, tmpBuf);
	}
	else
	*/
	{
		c2hCmdLen = Length -2;
		tmpBuf = Buffer+2;
		_C2HContentParsing8814A(priv, c2hCmdId, c2hCmdLen, tmpBuf);
		//C2HEventHandler88XX(priv, c2hCmdId, c2hCmdLen, tmpBuf);
	}
}



#if (BEAMFORMING_SUPPORT == 1)

BOOLEAN
beamforming_setiqgen_8814a (
	struct rtl8192cd_priv *priv
	)
{
	u1Byte i = 0;
	u2Byte counter = 0;
	u4Byte rf_mode[4];

	for (i= ODM_RF_PATH_A; i < MAX_RF_PATH; i++)
		ODM_SetRFReg(ODMPTR, i, RF_WE_LUT, 0x80000, 0x1); // RF Mode table write enable
	
	while(1) {
		counter++;
		for (i= ODM_RF_PATH_A; i < MAX_RF_PATH; i++)
			ODM_SetRFReg(ODMPTR, i, RF_RCK_OS, 0xfffff, 0x18000); // Select Rx mode

		ODM_delay_us(2);
		
		for (i= ODM_RF_PATH_A; i < MAX_RF_PATH; i++)
			rf_mode[i] = ODM_GetRFReg(ODMPTR, i, RF_RCK_OS, 0xfffff); // Select Rx mode
		
		if ((rf_mode[0] == 0x18000) && (rf_mode[1] == 0x18000) && (rf_mode[2] == 0x18000) && (rf_mode[3] == 0x18000))
			break;
		else if (counter == 100) {
			ODM_RT_TRACE(ODMPTR, ODM_PHY_CONFIG, ODM_DBG_TRACE, ("iqgen setting fail:8814A \n"));
			for (i= ODM_RF_PATH_A; i < MAX_RF_PATH; i++)
				ODM_SetRFReg(ODMPTR, i, RF_WE_LUT, 0x80000, 0x0);	/*RF Mode table write disable*/
			return FALSE;
		}
	}

	for (i= ODM_RF_PATH_A; i < MAX_RF_PATH; i++) {
		ODM_SetRFReg(ODMPTR, i, RF_TXPA_G1, 0xfffff, 0xBE77F); // Set Table data
		ODM_SetRFReg(ODMPTR, i, RF_TXPA_G2, 0xfffff, 0x226BF); // Enable TXIQGEN in Rx mode
	}
	ODM_SetRFReg(ODMPTR, ODM_RF_PATH_A, RF_TXPA_G2, 0xfffff, 0xE26BF); // Enable TXIQGEN in Rx mode
	

	while(1) {
		counter++;
		for (i= ODM_RF_PATH_A; i < MAX_RF_PATH; i++)
			ODM_SetRFReg(ODMPTR, i, RF_RCK_OS, 0xfffff,0x8000); // Select Standby mode

		ODM_delay_us(2);
		
		for (i= ODM_RF_PATH_A; i < MAX_RF_PATH; i++)
			rf_mode[i] = ODM_GetRFReg(ODMPTR, i, RF_RCK_OS, 0xfffff); // Select Standby mode
		
		if ((rf_mode[0] == 0x8000) && (rf_mode[1] == 0x8000) && (rf_mode[2] == 0x8000) && (rf_mode[3] == 0x8000))
			break;
		else if (counter == 100) {
			ODM_RT_TRACE(ODMPTR, ODM_PHY_CONFIG, ODM_DBG_TRACE, ("iqgen setting fail:8814A \n"));
			for (i= ODM_RF_PATH_A; i < MAX_RF_PATH; i++)
				ODM_SetRFReg(ODMPTR, i, RF_WE_LUT, 0x80000, 0x0);	/*RF Mode table write disable*/
			return FALSE;
		}
	}

	for (i= ODM_RF_PATH_A; i < MAX_RF_PATH; i++) {
		ODM_SetRFReg(ODMPTR, i, RF_TXPA_G1, 0xfffff,0xFE441);/*Set Table data*/
		ODM_SetRFReg(ODMPTR, i, RF_TXPA_G2, 0xfffff,0xE20BF);/*Enable TXIQGEN in Standby mode*/
	}

	for (i= ODM_RF_PATH_A; i < MAX_RF_PATH; i++)
		ODM_SetRFReg(ODMPTR, i, RF_WE_LUT, 0x80000, 0x0);	/*RF Mode table write disable*/

	return TRUE;

}

VOID
halTxbf8814A_GetBeamformCap(
	struct rtl8192cd_priv *priv
	)
{
	PRT_BEAMFORMING_INFO 	pBeamformingInfo = &(priv->pshare->BeamformingInfo);
	BEAMFORMING_CAP		BeamformCap = BEAMFORMING_CAP_NONE;

	BeamformCap = Beamforming_GetBeamCap(priv, pBeamformingInfo);

	pBeamformingInfo->BeamformCap = BeamformCap;
	
}

u1Byte
halTxbf8814A_GetNtx(
	struct rtl8192cd_priv *priv
	)
{
	u1Byte			Ntx = 0;

	if(get_rf_mimo_mode(priv) == MIMO_4T4R)
		Ntx = 3;
	else if(get_rf_mimo_mode(priv) == MIMO_3T3R)
		Ntx = 2;
	else if(get_rf_mimo_mode(priv) == MIMO_2T4R)
		Ntx = 1;
	else if(get_rf_mimo_mode(priv) == MIMO_2T2R)
		Ntx = 1;
	else
		Ntx = 1;

	return Ntx;

}

u1Byte
halTxbf8814A_GetNrx(
	struct rtl8192cd_priv *priv
	)
{
	u1Byte			Nrx = 0;

	if(get_rf_mimo_mode(priv) == MIMO_4T4R)
		Nrx = 3;
	else if(get_rf_mimo_mode(priv) == MIMO_3T3R)
		Nrx = 2;
	else if(get_rf_mimo_mode(priv) == MIMO_2T4R)
		Nrx = 3;
	else if(get_rf_mimo_mode(priv) == MIMO_2T2R)
		Nrx = 1;
	else if(get_rf_mimo_mode(priv) == MIMO_1T2R)
		Nrx = 1;
	else
		Nrx = 0;

	return Nrx;
	
}


VOID
SetBeamformRfMode8814A(
	struct rtl8192cd_priv *priv,
	PRT_BEAMFORMING_INFO 	pBeamformingInfo,
	u1Byte					idx
	)
{
	u1Byte					i, Nr_index = 0;
	BOOLEAN					bSelfBeamformer = FALSE;
	RT_BEAMFORMING_ENTRY	BeamformeeEntry;
	BEAMFORMING_CAP		BeamformCap = BEAMFORMING_CAP_NONE;

	if (idx < BEAMFORMEE_ENTRY_NUM)
		BeamformeeEntry = pBeamformingInfo->BeamformeeEntry[idx];
	else
		return;
	
	BeamformCap = Beamforming_GetBeamCap(priv, pBeamformingInfo);

	Nr_index = TxBF_Nr(halTxbf8814A_GetNtx(priv), BeamformeeEntry.CompSteeringNumofBFer);
	
	pBeamformingInfo->BeamformCap = BeamformCap;

	if (get_rf_mimo_mode(priv) == MIMO_1T1R)
		return;

	bSelfBeamformer = BeamformeeEntry.BeamformEntryCap & (BEAMFORMER_CAP_HT_EXPLICIT|BEAMFORMER_CAP_VHT_SU);
	
	if (bSelfBeamformer) {		/*for 8814 19ac(idx 1), 19b4(idx 0), different Tx ant setting*/
		PHY_SetBBReg(priv, REG_BB_TXBF_ANT_SET_BF1, BIT28|BIT29, 0x2);			/*enable BB TxBF ant mapping register*/
		PHY_SetBBReg(priv, REG_BB_TXBF_ANT_SET_BF1, BIT30, 0x1);			/*if Nsts > Nc, Don't apply V matrix*/
		PHY_SetBBReg(priv, REG_BB_TX_PATH_SEL_1, BIT23|BIT22|BIT21|BIT20, 0xe);	/*set TxPath selection for 8814a BFer bug refine*/
		PHY_SetBBReg(priv, REG_BB_TX_PATH_SEL_1, 0xff000000, 0x93);				/*if Bfer enable, always use 3Tx for all Spatial stream*/
		PHY_SetBBReg(priv, REG_BB_TX_PATH_SEL_2, 0xffffffff, 0x93e93e0);

		if (idx == 0) {
			switch(Nr_index) {			
				case 0:	
				break;

				case 1:		// Nsts = 2	BC
				PHY_SetBBReg(priv, REG_BB_TXBF_ANT_SET_BF0, BIT3|BIT2|BIT1|BIT0, 0x6);	//1ss
				PHY_SetBBReg(priv, REG_BB_TXBF_ANT_SET_BF0, BIT7|BIT6|BIT5|BIT4, 0x6);	//2ss
				PHY_SetBBReg(priv, REG_BB_TXBF_ANT_SET_BF0, 0x0000ff00, 0x10);			//BC
				break;

				case 2:		//Nsts = 3	BCD
				PHY_SetBBReg(priv, REG_BB_TXBF_ANT_SET_BF0, BIT3|BIT2|BIT1|BIT0, 0xe);	//1ss
				PHY_SetBBReg(priv, REG_BB_TXBF_ANT_SET_BF0, BIT7|BIT6|BIT5|BIT4, 0xe);	//2ss
				PHY_SetBBReg(priv, REG_BB_TXBF_ANT_SET_BF0, 0x0000ff00, 0x90);			//BCD
				PHY_SetBBReg(priv, REG_BB_TXBF_ANT_SET_BF0, BIT19|BIT18|BIT17|BIT16, 0xe);	//3ss
				PHY_SetBBReg(priv, REG_BB_TXBF_ANT_SET_BF0, 0xff00000, 0x90);			//bcd

				break;
			
				default:		//Nr>3, same as Case 3
				PHY_SetBBReg(priv, REG_BB_TXBF_ANT_SET_BF0, BIT3|BIT2|BIT1|BIT0, 0xf);	//1ss
				PHY_SetBBReg(priv, REG_BB_TXBF_ANT_SET_BF0, BIT7|BIT6|BIT5|BIT4, 0xf);	//2ss
				PHY_SetBBReg(priv, REG_BB_TXBF_ANT_SET_BF0, 0x0000ff00, 0x93);			//aBCd
				PHY_SetBBReg(priv, REG_BB_TXBF_ANT_SET_BF0, BIT19|BIT18|BIT17|BIT16, 0xf);	//4ss
				PHY_SetBBReg(priv, REG_BB_TXBF_ANT_SET_BF0, 0xff00000, 0x93);			//abcd
				PHY_SetBBReg(priv, REG_BB_TX_PATH_SEL_1, BIT23|BIT22|BIT21|BIT20, 0xf);	/*set TxPath selection for 8814a BFer bug refine*/
				PHY_SetBBReg(priv, REG_BB_TX_PATH_SEL_1, 0xff000000, 0x93);				/*if Bfer enable, always use 3Tx for all Spatial stream*/
				PHY_SetBBReg(priv, REG_BB_TX_PATH_SEL_2, 0xffffffff, 0x93f93f0);
				break;
			}
		} else {		// IDX =1
			switch(Nr_index) {
				case 0:	
				break;

				case 1:		// Nsts = 2	BC
				PHY_SetBBReg(priv, REG_BB_TXBF_ANT_SET_BF1, BIT3|BIT2|BIT1|BIT0, 0x6);	//1ss
				PHY_SetBBReg(priv, REG_BB_TXBF_ANT_SET_BF1, BIT7|BIT6|BIT5|BIT4, 0x6);	//2ss
				PHY_SetBBReg(priv, REG_BB_TXBF_ANT_SET_BF1, 0x0000ff00, 0x10);			//BC
				break;

				case 2:		//Nsts = 3	BCD
				PHY_SetBBReg(priv, REG_BB_TXBF_ANT_SET_BF1, BIT3|BIT2|BIT1|BIT0, 0xe);	//1ss
				PHY_SetBBReg(priv, REG_BB_TXBF_ANT_SET_BF1, BIT7|BIT6|BIT5|BIT4, 0xe);	//2ss
				PHY_SetBBReg(priv, REG_BB_TXBF_ANT_SET_BF1, 0x0000ff00, 0x90);			//BC
				PHY_SetBBReg(priv, REG_BB_TXBF_ANT_SET_BF1, BIT19|BIT18|BIT17|BIT16, 0xe);	//3ss
				PHY_SetBBReg(priv, REG_BB_TXBF_ANT_SET_BF1, 0xff00000, 0x90);			//bcd
				break;
			
				default:		//Nr>3, same as Case 3
				PHY_SetBBReg(priv, REG_BB_TXBF_ANT_SET_BF1, BIT3|BIT2|BIT1|BIT0, 0xf);	//1ss
				PHY_SetBBReg(priv, REG_BB_TXBF_ANT_SET_BF1, BIT7|BIT6|BIT5|BIT4, 0xf);	//2ss
				PHY_SetBBReg(priv, REG_BB_TXBF_ANT_SET_BF1, 0x0000ff00, 0x93);			//BC
				PHY_SetBBReg(priv, REG_BB_TXBF_ANT_SET_BF1, BIT19|BIT18|BIT17|BIT16, 0xf);	//3ss
				PHY_SetBBReg(priv, REG_BB_TXBF_ANT_SET_BF1, 0xff00000, 0x93);			//bcd
				PHY_SetBBReg(priv, REG_BB_TX_PATH_SEL_1, BIT23|BIT22|BIT21|BIT20, 0xf);	/*set TxPath selection for 8814a BFer bug refine*/
				PHY_SetBBReg(priv, REG_BB_TX_PATH_SEL_1, 0xff000000, 0x93);				/*if Bfer enable, always use 3Tx for all Spatial stream*/
				PHY_SetBBReg(priv, REG_BB_TX_PATH_SEL_2, 0xffffffff, 0x93f93f0);
				break;
			}
		}
		
	}


}
VOID
SetBeamformEnter8814A(
	struct rtl8192cd_priv *priv,
	u1Byte				BFerBFeeIdx
	)
{
	u1Byte					i = 0;
	u1Byte					BFerIdx = (BFerBFeeIdx & 0xF0)>>4;
	u1Byte					BFeeIdx = (BFerBFeeIdx & 0xF);
	u2Byte					CSI_Param = 0;
	PRT_BEAMFORMING_INFO 	pBeamformingInfo = &(priv->pshare->BeamformingInfo);
	RT_BEAMFORMING_ENTRY	BeamformeeEntry;
	RT_BEAMFORMER_ENTRY	BeamformerEntry;
	u2Byte					STAid = 0;
	u1Byte					Nc_index = 0,Nr_index = 0, grouping = 0, codebookinfo = 0, coefficientsize = 0;

	halTxbf8814A_GetBeamformCap(priv);

	/*RTL_W16( REG_SND_PTCL_CTRL+1, 0x0202);*/
	RTL_W8( REG_SND_PTCL_CTRL+1, 0x02);
	RTL_W8( REG_SND_PTCL_CTRL+2, 0x02);

	//I am Bfee
	if ((pBeamformingInfo->BeamformCap & BEAMFORMEE_CAP) && (BFerIdx < BEAMFORMER_ENTRY_NUM)) {
		BeamformerEntry = pBeamformingInfo->BeamformerEntry[BFerIdx];
	
		RTL_W8(REG_SND_PTCL_CTRL, 0xDB);	

		// MAC addresss/Partial AID of Beamformer
		if (BFerIdx == 0) {
			for (i = 0; i < 6 ; i++)
				RTL_W8( (REG_ASSOCIATED_BFMER0_INFO+i), BeamformerEntry.MacAddr[i]);
		} else {
			for (i = 0; i < 6 ; i++)
				RTL_W8( (REG_ASSOCIATED_BFMER1_INFO+i), BeamformerEntry.MacAddr[i]);
		}

		// CSI report parameters of Beamformer

		Nc_index = halTxbf8814A_GetNrx(priv);	// for 8814A Nrx = 3(4 Ant), min=0(1 Ant)
		Nr_index = BeamformerEntry.NumofSoundingDim;	//0x718[7] = 1 use Nsts, 0x718[7] = 0 use reg setting. as Bfee, we use Nsts, so Nr_index don't care
		grouping = 0;

		//// for ac = 1, for n = 3
		if(BeamformerEntry.BeamformEntryCap & BEAMFORMEE_CAP_VHT_SU)
			codebookinfo = 1;	
		else if(BeamformerEntry.BeamformEntryCap & BEAMFORMEE_CAP_HT_EXPLICIT)
			codebookinfo = 3;	

		coefficientsize = 3;

		CSI_Param =(u2Byte)((coefficientsize<<10)|(codebookinfo<<8)|(grouping<<6)|(Nr_index<<3)|(Nc_index));

		if (BFerIdx == 0) {
			RTL_W16(REG_TX_CSI_RPT_PARAM_BW20, CSI_Param);
		} else {
			RTL_W16(REG_TX_CSI_RPT_PARAM_BW20+2, CSI_Param);
		}

		// Timeout value for MAC to leave NDP_RX_standby_state 60 us
		RTL_W8( REG_SND_PTCL_CTRL+3, 0x70);				//ndp_rx_standby_timer, 8814 need > 0x56, suggest from Dvaid
	}

	//I am Bfer
	if((pBeamformingInfo->BeamformCap & BEAMFORMER_CAP) && (BFeeIdx < BEAMFORMEE_ENTRY_NUM)) {	

		BeamformeeEntry = pBeamformingInfo->BeamformeeEntry[BFeeIdx];
		SetBeamformRfMode8814A(priv, pBeamformingInfo, BFeeIdx);	

		if(OPMODE & WIFI_ADHOC_STATE)
			STAid = BeamformeeEntry.AID;
		else 
			STAid = BeamformeeEntry.P_AID;

		// P_AID of Beamformee & enable NDPA transmission
		if(BFeeIdx == 0) {	
			RTL_W16( REG_TXBF_CTRL, STAid);	
			RTL_W8( REG_TXBF_CTRL+3, RTL_R8( REG_TXBF_CTRL+3)|BIT6|BIT7|BIT4);
		} else {
			RTL_W16( REG_TXBF_CTRL+2, STAid |BIT14| BIT15|BIT12);
		}	

		// CSI report parameters of Beamformee
		if(BFeeIdx == 0) {
			// Get BIT24 & BIT25
			u1Byte	tmp = RTL_R8( REG_ASSOCIATED_BFMEE_SEL+3) & 0x3;	
			RTL_W8( REG_ASSOCIATED_BFMEE_SEL+3, tmp | 0x60);
			RTL_W16( REG_ASSOCIATED_BFMEE_SEL, STAid | BIT9);
		} else {
			// Set BIT25
			RTL_W16( REG_ASSOCIATED_BFMEE_SEL+2, STAid | 0xE200);
		}

		RTL_W16( REG_TXBF_CTRL, RTL_R16(REG_TXBF_CTRL)|BIT15);	// disable NDP/NDPA packet use beamforming 

		Beamforming_Notify(priv);
	}
	
}


VOID
SetBeamformLeave8814A(
	struct rtl8192cd_priv *priv,
	u1Byte				Idx
	)
{
	PRT_BEAMFORMING_INFO 	pBeamformingInfo = &(priv->pshare->BeamformingInfo);
	RT_BEAMFORMING_ENTRY	BeamformeeEntry;
	RT_BEAMFORMER_ENTRY	BeamformerEntry;

	if (Idx < BEAMFORMER_ENTRY_NUM) {
		BeamformeeEntry = pBeamformingInfo->BeamformeeEntry[Idx];
		BeamformerEntry = pBeamformingInfo->BeamformerEntry[Idx];
	} else
		return;
	
	/*	Clear P_AID of Beamformee
	* 	Clear MAC addresss of Beamformer
	*	Clear Associated Bfmee Sel
	*/
	if (BeamformeeEntry.BeamformEntryCap == BEAMFORMING_CAP_NONE) {
		if(Idx == 0) {
			RTL_W16( REG_TXBF_CTRL, 0|BIT15);
			RTL_W16( REG_ASSOCIATED_BFMEE_SEL, 0);
		} else {
			RTL_W16(REG_TXBF_CTRL+2, RTL_R16( REG_TXBF_CTRL+2) & 0xF000);
			RTL_W16(REG_ASSOCIATED_BFMEE_SEL+2,	RTL_R16( REG_ASSOCIATED_BFMEE_SEL+2) & 0x60);
		}	
	}
	
	if (BeamformerEntry.BeamformEntryCap == BEAMFORMING_CAP_NONE) {
		if(Idx == 0) {
			RTL_W32( REG_ASSOCIATED_BFMER0_INFO, 0);
			RTL_W16( REG_ASSOCIATED_BFMER0_INFO+4, 0);
			RTL_W16( REG_TX_CSI_RPT_PARAM_BW20, 0);
		} else {
			RTL_W32( REG_ASSOCIATED_BFMER1_INFO, 0);
			RTL_W16( REG_ASSOCIATED_BFMER1_INFO+4, 0);
			RTL_W16( REG_TX_CSI_RPT_PARAM_BW20+2, 0);
		}	
	}

	if (((pBeamformingInfo->BeamformerEntry[0]).BeamformEntryCap == BEAMFORMING_CAP_NONE)
		&& ((pBeamformingInfo->BeamformerEntry[1]).BeamformEntryCap == BEAMFORMING_CAP_NONE))
			RTL_W8( REG_SND_PTCL_CTRL, 0xD8);

}
VOID
SetBeamformStatus8814A(
	struct rtl8192cd_priv *priv,
	 u1Byte				Idx
	)
{
	u2Byte					BeamCtrlVal;
	u4Byte					BeamCtrlReg;
	PRT_BEAMFORMING_INFO pBeamformingInfo = &(priv->pshare->BeamformingInfo);
	RT_BEAMFORMING_ENTRY	BeamformEntry;

	if (Idx < BEAMFORMEE_ENTRY_NUM)
		BeamformEntry = pBeamformingInfo->BeamformeeEntry[Idx];
	else
		return;
	
	if (OPMODE & WIFI_ADHOC_STATE)
		BeamCtrlVal = BeamformEntry.MacId;
	else 
		BeamCtrlVal = BeamformEntry.P_AID;
	if (Idx == 0)
		BeamCtrlReg = REG_TXBF_CTRL;
	else {
		BeamCtrlReg = REG_TXBF_CTRL+2;
		BeamCtrlVal |= BIT12|BIT14|BIT15;
	}

	if(BeamformEntry.pSTA->error_csi == 1 && BeamformEntry.pSTA->csi_nc == 0 && BeamformEntry.pSTA->current_tx_rate >= _NSS2_MCS0_RATE_ && (priv->pshare->rf_ft_var.applyVmatrix)) {			
		BeamCtrlVal &= ~(BIT9|BIT10|BIT11);
	}
	else if(BeamformEntry.pSTA->error_csi == 1 && BeamformEntry.pSTA->csi_nc == 1 && BeamformEntry.pSTA->current_tx_rate >= _NSS3_MCS0_RATE_ && (priv->pshare->rf_ft_var.applyVmatrix)) {			
		BeamCtrlVal &= ~(BIT9|BIT10|BIT11);
	} else if((BeamformEntry.BeamformEntryState == BEAMFORMING_ENTRY_STATE_PROGRESSED) && (priv->pshare->rf_ft_var.applyVmatrix)) {
		if (BeamformEntry.BW == HT_CHANNEL_WIDTH_20)
			BeamCtrlVal |= BIT9;
		else if (BeamformEntry.BW == HT_CHANNEL_WIDTH_20_40)
			BeamCtrlVal |= (BIT9 | BIT10);
		else if (BeamformEntry.BW == HT_CHANNEL_WIDTH_80)
			BeamCtrlVal |= (BIT9 | BIT10 | BIT11);
	} else {
		BeamCtrlVal &= ~(BIT9 | BIT10 | BIT11);
	}
	RTL_W16(BeamCtrlReg, BeamCtrlVal);
	RTL_W16( REG_TXBF_CTRL, RTL_R16(REG_TXBF_CTRL)|BIT15);	// disable NDP/NDPA packet use beamforming 

}
//2REG_C2HEVT_CLEAR
#define		C2H_EVT_HOST_CLOSE			0x00	// Set by driver and notify FW that the driver has read the C2H command message
#define		C2H_EVT_FW_CLOSE			0xFF	// Set by FW indicating that FW had set the C2H command message and it's not yet read by driver.



VOID Beamforming_NDPARate_8814A(
	struct rtl8192cd_priv *priv,
	BOOLEAN		Mode,
	u1Byte		BW,
	u1Byte		Rate)
{

	u2Byte	NDPARate = Rate;

	if(NDPARate == 0) {
		if(priv->pshare->rssi_min > 30) // link RSSI > 30%
			NDPARate = 0x8;				//MGN_24M, TxDesc = 0x08
		else
			NDPARate = 0x4;				//MGN_6M, TxDesc = 0x4
	}

	if(NDPARate < ODM_MGN_MCS0)
		BW = HT_CHANNEL_WIDTH_20;	

	RTL_W8(REG_NDPA_OPT_CTRL, BW & 0x3);
	RTL_W8(REG_NDPA_RATE, NDPARate);

}

VOID
C2HTxBeamformingHandler_8814A(
	struct rtl8192cd_priv *priv,
		pu1Byte			CmdBuf,
		u1Byte			CmdLen
)
{
	u1Byte 	status = CmdBuf[0] & BIT0;
	Beamforming_End(priv, status);
}

VOID HW_VAR_HW_REG_TIMER_START_8814A(struct rtl8192cd_priv *priv)
{
	RTL_W8(0x15F, 0x0);
	RTL_W8(0x15F, 0x5);

}

VOID HW_VAR_HW_REG_TIMER_INIT_8814A(struct rtl8192cd_priv *priv, int t)
{
	RTL_W8(0x164, 1);
	RTL_W16(0x15C, t);

}

VOID HW_VAR_HW_REG_TIMER_STOP_8814A(struct rtl8192cd_priv *priv)
{
	RTL_W8(0x15F, 0);
}

#endif 


