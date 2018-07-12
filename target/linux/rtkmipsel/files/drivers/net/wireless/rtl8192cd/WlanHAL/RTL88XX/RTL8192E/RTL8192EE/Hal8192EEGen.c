/*++
Copyright (c) Realtek Semiconductor Corp. All rights reserved.

Module Name:
	Hal88XXGen.c
	
Abstract:
	Defined RTL8192EE HAL Function
	    
Major Change History:
	When       Who               What
	---------- ---------------   -------------------------------
	2012-03-23 Filen            Create.	
--*/

#if !defined(__ECOS) && !defined(CPTCFG_CFG80211_MODULE)
#include "HalPrecomp.h"
#else
#include "../../../HalPrecomp.h"
#endif
#include "../../../../8192cd.h"

typedef enum _RTL8192E_C2H_EVT
{
	C2H_8192E_DBG = 0,
	C2H_8192E_LB = 1,
	C2H_8192E_TXBF = 2,
	C2H_8192E_TX_REPORT = 3,
	C2H_8192E_TX_RATE =4,
	C2H_8192E_BT_INFO = 9,
	C2H_8192E_BT_MP = 11,
	C2H_8192E_RA_RPT = 12,
#ifdef TXRETRY_CNT
	C2H_8192E_TX_RETRY = 13, //0x0D
#endif
	C2H_8192E_RA_PARA_RPT=14,
	C2H_8192E_EXTEND_IND = 0xFF,
	MAX_8192E_C2HEVENT
}RTL8192E_C2H_EVT;

typedef enum _RTL8192E_EXTEND_C2H_EVT
{
	EXTEND_C2H_8192E_DBG_PRINT = 0

}RTL8192E_EXTEND_C2H_EVT;

RT_STATUS
InitPON8192EE(
    IN  HAL_PADAPTER Adapter,
    IN  u4Byte     	ClkSel        
)
{
    u32     bytetmp;
    u32     retry;
    u32     round = 0;
    
    RT_TRACE_F( COMP_INIT, DBG_LOUD, ("\n"));

	HAL_RTL_W8(REG_RSV_CTRL, 0x00);

    // Add by Eric 2013/01/24
    // For 92E MP chip, power on sometimes crystal clk setting error
    // clk set 25M, value 0x00
	if(ClkSel == XTAL_CLK_SEL_25M) {
		HAL_RTL_W16(REG_AFE_CTRL4, 0x002a);
		HAL_RTL_W8(REG_AFE_CTRL2, 5);
	} else if (ClkSel == XTAL_CLK_SEL_40M){
		HAL_RTL_W16(REG_AFE_CTRL4, 0x002a);
		HAL_RTL_W8(REG_AFE_CTRL2, 1);
	}
	
	//pattern Patching EQC fail IC(AFE issue)
	//Setting is suggested by SD1-Pisa
	//Set 0x78[21] = 0, 0x28[6] = 0
	//0x28[6] = 0 is done by XTAL seletion 0x28[6] = 0

	HAL_RTL_W32(REG_AFE_CTRL4, HAL_RTL_R32(REG_AFE_CTRL4) & ~BIT21);
	if (!HalPwrSeqCmdParsing88XX(Adapter, PWR_CUT_ALL_MSK, PWR_FAB_ALL_MSK,
			PWR_INTF_PCI_MSK, rtl8192E_card_enable_flow))
    {
        RT_TRACE( COMP_INIT, DBG_SERIOUS, ("%s %d, HalPwrSeqCmdParsing init fail!!!\n", __FUNCTION__, __LINE__));
        return RT_STATUS_FAILURE;
    }

	// Power on when re-enter from IPS/Radio off/card disable
    HAL_RTL_W8(REG_AFE_CTRL1, HAL_RTL_R8(REG_AFE_CTRL1) | BIT0);

    bytetmp = HAL_RTL_R16(REG_SYS_PW_CTRL);
    bytetmp &= 0xE7ff;
    bytetmp |= 0x0800;
    HAL_RTL_W16(REG_SYS_PW_CTRL, bytetmp);

    while (!((bytetmp = HAL_RTL_R32(REG_SYS_PW_CTRL)) & 0x00020000)) {
		if (++round > 10000) {
			panic_printk("%s[%d] while (1) goes too many\n", __FUNCTION__, __LINE__);
			break;
		}
    }

    bytetmp = HAL_RTL_R16(REG_SYS_PW_CTRL);
    bytetmp &= 0x7FFF;
    HAL_RTL_W16(REG_SYS_PW_CTRL, bytetmp);

    bytetmp = HAL_RTL_R16(REG_SYS_PW_CTRL);
    bytetmp &= 0xE7ff;
    bytetmp |= 0x0000;
    HAL_RTL_W16(REG_SYS_PW_CTRL, bytetmp);

    HAL_delay_ms(1);

	// auto enable WLAN
	// Power On Reset for MAC Block
	bytetmp = HAL_RTL_R8(REG_SYS_PW_CTRL+1) | BIT(0);
	HAL_delay_us(2);
	HAL_RTL_W8(REG_SYS_PW_CTRL+1, bytetmp);
	HAL_delay_us(2);

	bytetmp = HAL_RTL_R8(REG_SYS_PW_CTRL+1);
	HAL_delay_us(2);
	retry = 0;
	while((bytetmp & BIT(0)) && retry < 1000){
		retry++;
		HAL_delay_us(50);
		bytetmp = HAL_RTL_R8(REG_SYS_PW_CTRL+1);
		HAL_delay_us(50);
	}
	
    RT_TRACE(COMP_INIT, DBG_WARNING, ("%s: RTL_R8(APS_FSMCO+1) retry times=%d\n", (char *)__FUNCTION__, retry) );

	{
		int val;
		val = HAL_RTL_R32(REG_SYS_PW_CTRL);
        RT_TRACE(COMP_INIT, DBG_TRACE, ("FSMCO11=0x%x\n", val) );
	}
	
	if (bytetmp & BIT(0)) {
        RT_TRACE(COMP_INIT, DBG_SERIOUS, ("%s ERROR: auto enable WLAN failed!!(0x%02X)\n", __FUNCTION__, bytetmp) );
	}	

    HAL_RTL_W16(REG_SYS_FUNC_EN, HAL_RTL_R16(REG_SYS_FUNC_EN) & ~BIT_FEN_CPUEN);
    
    HAL_delay_us(2);

    // check LDO mode 
    if(HAL_RTL_R32(REG_SYS_CFG1)&BIT24) {
        // LDO mode set 0x7C
        HAL_RTL_W8(REG_LDO_SWR_CTRL,0xc3);
    } else {
        // SPS mode
        HAL_RTL_W8(REG_LDO_SWR_CTRL,0x83);
    }

    return  RT_STATUS_SUCCESS;    
}


RT_STATUS
StopHW8192EE(
    IN  HAL_PADAPTER Adapter
)
{
    // TODO:

    return RT_STATUS_SUCCESS;
}


RT_STATUS	
hal_Associate_8192EE(
    HAL_PADAPTER        Adapter,
    BOOLEAN                 IsDefaultAdapter
)
{
    PHAL_INTERFACE              pHalFunc = GET_HAL_INTERFACE(Adapter);
    PHAL_DATA_TYPE              pHalData = _GET_HAL_DATA(Adapter);

    //
    //Initialization Related
    //
    pHalData->AccessSwapCtrl        = HAL_ACCESS_SWAP_MEM;
    pHalFunc->InitPONHandler        = InitPON8192EE;
    pHalFunc->InitMACHandler        = InitMAC88XX;
    pHalFunc->InitFirmwareHandler   = InitFirmware88XX;
    pHalFunc->InitHCIDMAMemHandler  = InitHCIDMAMem88XX;
    pHalFunc->InitHCIDMARegHandler  = InitHCIDMAReg88XX;    
#if CFG_HAL_SUPPORT_MBSSID    
    pHalFunc->InitMBSSIDHandler     = InitMBSSID88XX;
    pHalFunc->InitMBIDCAMHandler    = InitMBIDCAM88XX;
#endif  //CFG_HAL_SUPPORT_MBSSID
    pHalFunc->InitLLT_TableHandler  = InitLLT_Table88XX;
#ifdef MULTI_MAC_CLONE
	pHalFunc->McloneSetMBSSIDHandler 	= McloneSetMBSSID88XX;
	pHalFunc->McloneStopMBSSIDHandler   = McloneStopMBSSID88XX;
#endif
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
    pHalFunc->StopMBIDCAMHandler    = StopMBIDCAM88XX;
	pHalFunc->ResetHWForSurpriseHandler     = ResetHWForSurprise88XX;

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
    pHalFunc->FillTxDescHandler             =  FillTxDesc88XX;

#if CFG_HAL_TX_SHORTCUT
//    pHalFunc->GetShortCutTxDescHandler      = GetShortCutTxDesc88XX;
//    pHalFunc->ReleaseShortCutTxDescHandler  = ReleaseShortCutTxDesc88XX;
    pHalFunc->GetShortCutTxBuffSizeHandler  = GetShortCutTxBuffSize88XX;
    pHalFunc->SetShortCutTxBuffSizeHandler  = SetShortCutTxBuffSize88XX;
    pHalFunc->CopyShortCutTxDescHandler     = CopyShortCutTxDesc88XX;
    pHalFunc->FillShortCutTxHwCtrlHandler   = FillShortCutTxHwCtrl88XX;    
    pHalFunc->FillShortCutTxDescHandler     = FillShortCutTxDesc88XX;
    pHalFunc->FillRsrvPageDescHandler		= FillRsrvPageDesc88XX; 
#if 0	//CFG_HAL_HW_TX_SHORTCUT_REUSE_TXDESC    
    pHalFunc->FillHwShortCutTxDescHandler   =  FillHwShortCutTxDesc88XX;    
#endif
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
    pHalFunc->CheckHangHandler              =   CheckHang88XX;
    pHalFunc->GetMACIDQueueInTXPKTBUFHandler=   GetMACIDQueueInTXPKTBUF88XX;

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
    pHalFunc->PHYSwBWModeHandler            = SwBWMode88XX_N;
    //pHalFunc->TXPowerTrackingHandler        = TXPowerTracking_ThermalMeter_88XX;    
    pHalFunc->PHYSSetRFRegHandler           = PHY_SetRFReg_88XX_N;
    pHalFunc->PHYQueryRFRegHandler          = PHY_QueryRFReg_88XX_N;
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
	pHalFunc->C2HPacketHandler              = C2HPacketHandler_92E;    
    pHalFunc->DumpRxBDescTestHandler = DumpRxBDesc88XX;
    pHalFunc->DumpTxBDescTestHandler = DumpTxBDesc88XX;
    pHalFunc->GetTxRPTHandler               = GetTxRPTBuf88XX;
    pHalFunc->SetTxRPTHandler               = SetTxRPTBuf88XX;    

    
    return  RT_STATUS_SUCCESS;    
}


void 
InitMAC8192EE(
    IN  HAL_PADAPTER Adapter
)
{

    
}


#ifdef BT_COEXIST		
VOID
C2HBTInfoHandler_92E(
	struct rtl8192cd_priv *priv,
		pu1Byte			CmdBuf,
		u1Byte			CmdLen
)
{
	u1Byte src, byte4, H2CCommand[1];
	int j=0;
	extern c2h_bt_cnt;
	if(priv->pshare->rf_ft_var.bt_dump){
		panic_printk("\n");
		panic_printk("C2H BT Info [");
		for(j=0; j<=6; j++) {
			panic_printk(" %x ",CmdBuf[j]);
		}
		panic_printk("]\n");
	}
	src = CmdBuf[0];
	byte4 = CmdBuf[4];
	delay_ms(10);
	//panic_printk("source= %u\n",src);
	if(src == 1 || src == 2){ /* src BT */
		c2h_bt_cnt++;
		if(priv->pshare->rf_ft_var.bt_dump)
			panic_printk("source BT\n");

	}
	if(byte4 & BIT3){
		if(priv->pshare->rf_ft_var.bt_dump)
			panic_printk("Byte4[3] = 1\n");
		if(priv->pshare->rf_ft_var.btc == 1){
			H2CCommand[0]=0x0;
			FillH2CCmd88XX(priv, H2C_88XX_BT_63, 1, H2CCommand);
		}
	}
}
#endif


VOID
C2HTxTxReportHandler_92E(
	struct rtl8192cd_priv *priv,
		pu1Byte			CmdBuf,
		u1Byte			CmdLen
)
{
    #ifdef TXREPORT	
	struct tx_rpt rpt1;
	int k=0, j=0;
	for(j=0; j<2; j++) {
		rpt1.macid= CmdBuf[k];
		rpt1.txok = CmdBuf[k+1] | ((short)CmdBuf[k+2]<<8);
		rpt1.txfail = CmdBuf[k+3] | ((short)CmdBuf[k+4]<<8);
		rpt1.initil_tx_rate = CmdBuf[k+5];
		if(rpt1.macid != 0xff)
			txrpt_handler(priv, &rpt1);
		k+=6;
	}
	#endif
}

VOID
_C2HContentParsing92E(
	struct rtl8192cd_priv *priv,
		u1Byte			c2hCmdId, 
		u1Byte			c2hCmdLen,
		pu1Byte 			tmpBuf
)
{	
	switch(c2hCmdId)
	{
		case C2H_8192E_TXBF:
#if (BEAMFORMING_SUPPORT == 1) 	
			C2HTxBeamformingHandler_92E(priv, tmpBuf, c2hCmdLen);
#endif	
			break;
		case C2H_8192E_TX_RATE:
#ifdef TXREPORT	
			C2HTxTxReportHandler_92E(priv, tmpBuf, c2hCmdLen);

	#ifdef TXRETRY_CNT
			requestTxRetry88XX(priv);
	#else
			requestTxReport88XX(priv);
	#endif			
#endif		
			break;
		
#ifdef TXRETRY_CNT
		case C2H_8192E_TX_RETRY:
			C2HTxTxRetryHandler(priv, tmpBuf);
			requestTxReport88XX(priv);	
			break;
#endif
#ifdef BT_COEXIST		
		case C2H_8192E_BT_INFO:
			C2HBTInfoHandler_92E(priv, tmpBuf, c2hCmdLen);
			break;		
#endif
		default:
			if(!(phydm_c2H_content_parsing(ODMPTR, c2hCmdId, c2hCmdLen, tmpBuf))) {
				printk("[C2H], Unkown packet!! CmdId(%#X)!\n", c2hCmdId);
			}
			break;
	}
}

VOID
C2HPacketHandler_92E(
	struct rtl8192cd_priv *priv,
		pu1Byte			Buffer,
		u1Byte			Length
	)
{
	u1Byte	c2hCmdId=0, c2hCmdSeq=0, c2hCmdLen=0;
	pu1Byte tmpBuf=NULL;
	c2hCmdId = Buffer[0];
	c2hCmdSeq = Buffer[1];
/*
	if(c2hCmdId==C2H_88XX_EXTEND_IND)
	{
		c2hCmdLen = Length;
		tmpBuf = Buffer;
		C2HExtEventHandler88XX(NULL, c2hCmdId, c2hCmdLen, tmpBuf);
	}
	else
*/
	{
		c2hCmdLen = Length -2;
		tmpBuf = Buffer+2;
		
		_C2HContentParsing92E(priv, c2hCmdId, c2hCmdLen, tmpBuf);		
	}
}


#if (BEAMFORMING_SUPPORT == 1)

VOID
SetBeamformRfMode92E(
	struct rtl8192cd_priv *priv,
	PRT_BEAMFORMING_INFO 	pBeamformingInfo
	)
{
	u1Byte					i;
	u4Byte 				bSelfBeamformer = FALSE;
	u4Byte 				bSelfBeamformee = FALSE;
	RT_BEAMFORMING_ENTRY	BeamformEntry;
	BEAMFORMING_CAP 	BeamformCap = BEAMFORMING_CAP_NONE;

	BeamformCap = Beamforming_GetBeamCap(priv, pBeamformingInfo);
	
	if(BeamformCap == pBeamformingInfo->BeamformCap)
		return;
	else 
		pBeamformingInfo->BeamformCap = BeamformCap;
	
	if(get_rf_mimo_mode(priv) == MIMO_1T1R)
		return;

	PHY_SetRFReg(priv, ODM_RF_PATH_A, RF_WE_LUT, 0x80000,0x1); // RF Mode table write enable
	PHY_SetRFReg(priv, ODM_RF_PATH_B, RF_WE_LUT, 0x80000,0x1); // RF Mode table write enable

	bSelfBeamformer = BeamformCap & BEAMFORMER_CAP;
	bSelfBeamformee = BeamformCap & BEAMFORMEE_CAP;

	if(bSelfBeamformer)
	{ 
		// Path_A
		PHY_SetRFReg(priv, ODM_RF_PATH_A, RF_RCK_OS, 0xfffff,0x18000); // Select RX mode	0x30=0x18000
		PHY_SetRFReg(priv, ODM_RF_PATH_A, RF_TXPA_G1, 0xfffff,0x0000f); // Set Table data
		PHY_SetRFReg(priv, ODM_RF_PATH_A, RF_TXPA_G2, 0xfffff,0x77fc2); // Enable TXIQGEN in RX mode
		// Path_B
		PHY_SetRFReg(priv, ODM_RF_PATH_B, RF_RCK_OS, 0xfffff,0x18000); // Select RX mode
		PHY_SetRFReg(priv, ODM_RF_PATH_B, RF_TXPA_G1, 0xfffff,0x0000f); // Set Table data
		PHY_SetRFReg(priv, ODM_RF_PATH_B, RF_TXPA_G2, 0xfffff,0x77fc2); // Enable TXIQGEN in RX mode
	}
	else
	{
		// Paath_A
		PHY_SetRFReg(priv, ODM_RF_PATH_A, RF_RCK_OS, 0xfffff,0x18000); // Select RX mode
		PHY_SetRFReg(priv, ODM_RF_PATH_A, RF_TXPA_G1, 0xfffff,0x0000f); // Set Table data
		PHY_SetRFReg(priv, ODM_RF_PATH_A, RF_TXPA_G2, 0xfffff,0x77f82); // Disable TXIQGEN in RX mode
		// Path_B
		PHY_SetRFReg(priv, ODM_RF_PATH_B, RF_RCK_OS, 0xfffff,0x18000); // Select RX mode
		PHY_SetRFReg(priv, ODM_RF_PATH_B, RF_TXPA_G1, 0xfffff,0x0000f); // Set Table data
		PHY_SetRFReg(priv, ODM_RF_PATH_B, RF_TXPA_G2, 0xfffff,0x77f82); // Disable TXIQGEN in RX mode
	}

	PHY_SetRFReg(priv, ODM_RF_PATH_A, RF_WE_LUT, 0x80000,0x0); // RF Mode table write disable
	PHY_SetRFReg(priv, ODM_RF_PATH_B, RF_WE_LUT, 0x80000,0x0); // RF Mode table write disable

	if(bSelfBeamformer){
		PHY_SetBBReg(priv, 0x90c, 0xffffffff, 0x83321333);
		PHY_SetBBReg(priv, 0xa04, BIT31|BIT30, 0x3);
#ifdef RF_MIMO_SWITCH
		priv->pshare->rf_phy_bb_backup[19] = 0x83321333;
		priv->pshare->rf_phy_bb_backup[22] |=  BIT31|BIT30;
#endif
			
	}
	else {
		PHY_SetBBReg(priv, 0x90c, 0xffffffff, 0x81121313);
#ifdef RF_MIMO_SWITCH
		priv->pshare->rf_phy_bb_backup[19] = 0x81121313;
#endif

	}
}



VOID
SetBeamformEnter92E(
	struct rtl8192cd_priv *priv,
	u1Byte				BFerBFeeIdx
	)
{
	u1Byte					i = 0;
	u1Byte					BFerIdx = (BFerBFeeIdx & 0xF0)>>4;
	u1Byte					BFeeIdx = (BFerBFeeIdx & 0xF);
	u4Byte					CSI_Param;	
	PRT_BEAMFORMING_INFO 	pBeamformingInfo = &(priv->pshare->BeamformingInfo);
	RT_BEAMFORMING_ENTRY	BeamformeeEntry;
	RT_BEAMFORMER_ENTRY	BeamformerEntry;
	u2Byte					STAid = 0;

	SetBeamformRfMode92E(priv, pBeamformingInfo);

	if(get_rf_mimo_mode(priv) == MIMO_2T2R)
		RTL_W32(0xD80, 0x01081008);

	if((pBeamformingInfo->BeamformCap & BEAMFORMEE_CAP) && (BFerIdx < BEAMFORMER_ENTRY_NUM))
	{
		BeamformerEntry = pBeamformingInfo->BeamformerEntry[BFerIdx];
		//eric-8813
		if (IS_TEST_CHIP(priv))
			RTL_W8( REG_SND_PTCL_CTRL, 0x1B); 		// Disable SIG-B CRC8 check
		else
			RTL_W8( REG_SND_PTCL_CTRL, 0xCB);	

		// MAC addresss/Partial AID of Beamformer
		if(BFerIdx == 0)
		{
			for(i = 0; i < 6 ; i++)
				RTL_W8( (REG_ASSOCIATED_BFMER0_INFO+i), BeamformerEntry.MacAddr[i]);
			
			RTL_W16( REG_ASSOCIATED_BFMER0_INFO+6, BeamformerEntry.P_AID);
		}
		else
		{
			for(i = 0; i < 6 ; i++)
				RTL_W8( (REG_ASSOCIATED_BFMER1_INFO+i), BeamformerEntry.MacAddr[i]);

			RTL_W16( REG_ASSOCIATED_BFMER1_INFO+6, BeamformerEntry.P_AID);
		}

		// CSI report parameters of Beamformer
		CSI_Param = 0x03090309;//Nc =2, V matrix
		RTL_W32( REG_TX_CSI_RPT_PARAM_BW20, CSI_Param);
		RTL_W32( REG_TX_CSI_RPT_PARAM_BW40, CSI_Param);

		// Timeout value for MAC to leave NDP_RX_standby_state 60 us
		//	RTL_W8( REG_SND_PTCL_CTRL_8812+3, 0x3C);
		RTL_W8( REG_SND_PTCL_CTRL+3, 0x50);				// // ndp_rx_standby_timer
	}

	if((pBeamformingInfo->BeamformCap & BEAMFORMER_CAP) && (BFeeIdx < BEAMFORMEE_ENTRY_NUM))
	{
		BeamformeeEntry = pBeamformingInfo->BeamformeeEntry[BFeeIdx];
		
		if(OPMODE & WIFI_ADHOC_STATE)
			STAid = BeamformeeEntry.AID;
		else 
			STAid = BeamformeeEntry.P_AID;

		// P_AID of Beamformee & enable NDPA transmission
		if(BFeeIdx == 0)
		{	
			RTL_W16( REG_TXBF_CTRL, STAid);	
			RTL_W8( REG_TXBF_CTRL+3, RTL_R8( REG_TXBF_CTRL+3)|BIT6|BIT7|BIT4);
		}	
		else
		{
			RTL_W16( REG_TXBF_CTRL+2, STAid |BIT14| BIT15|BIT12);
		}	

		// CSI report parameters of Beamformee
		if(BFeeIdx == 0)	
		{
			// Get BIT24 & BIT25
			u1Byte	tmp = RTL_R8( REG_ASSOCIATED_BFMEE_SEL+3) & 0x3;	
			RTL_W8( REG_ASSOCIATED_BFMEE_SEL+3, tmp | 0x60);
			RTL_W16( REG_ASSOCIATED_BFMEE_SEL, STAid | BIT9);
		}	
		else
		{
			// Set BIT25
			RTL_W16( REG_ASSOCIATED_BFMEE_SEL+2, STAid | 0xE200);
		}

	//	if(pHalData->bIsMPChip == FALSE) 
		if (IS_TEST_CHIP(priv))		
		{
			// VHT category value 
			RTL_W8( REG_SND_PTCL_CTRL+1, ACT_CAT_VHT);
			// NDPA subtype
			RTL_W8( REG_SND_PTCL_CTRL+2, Type_NDPA >> 4);
		}	

		Beamforming_Notify(priv);
	}

}


VOID
SetBeamformLeave92E(
	struct rtl8192cd_priv *priv,
	u1Byte				Idx
	)
{
    PRT_BEAMFORMING_INFO    pBeamformingInfo = &(priv->pshare->BeamformingInfo);
    RT_BEAMFORMING_ENTRY    BeamformeeEntry = pBeamformingInfo->BeamformeeEntry[Idx];
    RT_BEAMFORMER_ENTRY	    BeamformerEntry = pBeamformingInfo->BeamformerEntry[Idx];

	/*	Clear P_AID of Beamformee
	* 	Clear MAC addresss of Beamformer
	*	Clear Associated Bfmee Sel
	*/	

    if (BeamformeeEntry.BeamformEntryCap == BEAMFORMING_CAP_NONE)
    {
	if(Idx == 0)
	{	
		RTL_W16( REG_TXBF_CTRL, 0);	
		RTL_W16( REG_ASSOCIATED_BFMEE_SEL, 0);
	}	
	else
	{
		RTL_W16( REG_TXBF_CTRL+2, RTL_R16( REG_TXBF_CTRL+2) & 0xF000);
            RTL_W16( REG_ASSOCIATED_BFMEE_SEL+2, RTL_R16( REG_ASSOCIATED_BFMEE_SEL+2) & 0x60);
        }
    }

    if (BeamformerEntry.BeamformEntryCap == BEAMFORMING_CAP_NONE)
    {
        if(Idx == 0)
        {
            RTL_W32( REG_ASSOCIATED_BFMER0_INFO, 0);
            RTL_W16( REG_ASSOCIATED_BFMER0_INFO+4, 0);
        }
        else
        {
		RTL_W32( REG_ASSOCIATED_BFMER1_INFO, 0);
		RTL_W16( REG_ASSOCIATED_BFMER1_INFO+4, 0);
	}	
}

    if(((pBeamformingInfo->BeamformerEntry[0]).BeamformEntryCap == BEAMFORMING_CAP_NONE)
        && ((pBeamformingInfo->BeamformerEntry[1]).BeamformEntryCap == BEAMFORMING_CAP_NONE)){
        ODM_RT_TRACE(ODMPTR, PHYDM_COMP_TXBF, ODM_DBG_LOUD, ("[Beamforming]@%s, All BeamformerEntryCap == NONE, STOP feedback CSI\n", __FUNCTION__, Idx));
        RTL_W8( REG_SND_PTCL_CTRL, 0xC8);
    }


}



VOID
SetBeamformStatus92E(
	struct rtl8192cd_priv *priv,
	 u1Byte				Idx
	)
{
	u2Byte					BeamCtrlVal;
	u4Byte					BeamCtrlReg;
//	PRT_BEAMFORMING_INFO 	pBeamformingInfo = GET_BEAMFORM_INFO(&(Adapter->MgntInfo));
	PRT_BEAMFORMING_INFO pBeamformingInfo = &(priv->pshare->BeamformingInfo);
	RT_BEAMFORMING_ENTRY	BeamformEntry = pBeamformingInfo->BeamformeeEntry[Idx];

//	if(ACTING_AS_IBSS(Adapter))
	if(OPMODE & WIFI_ADHOC_STATE)
		BeamCtrlVal = BeamformEntry.MacId;
	else 
		BeamCtrlVal = BeamformEntry.P_AID;

	if(Idx == 0)
		BeamCtrlReg = REG_TXBF_CTRL;
	else
	{
		BeamCtrlReg = REG_TXBF_CTRL+2;
		BeamCtrlVal |= BIT12 | BIT14|BIT15;
	}
// debug
#if 1
	if((BeamformEntry.BeamformEntryState == BEAMFORMING_ENTRY_STATE_PROGRESSED)  && (priv->pshare->rf_ft_var.applyVmatrix))
	{
	
//	panic_printk("%d%s BW = %d\n",__LINE__,__FUNCTION__,BeamformEntry.BW);
		if(BeamformEntry.BW == HT_CHANNEL_WIDTH_20)
			BeamCtrlVal |= BIT9;
		else if(BeamformEntry.BW == HT_CHANNEL_WIDTH_20_40)
			BeamCtrlVal |= (BIT9 | BIT10);
		else if(BeamformEntry.BW == HT_CHANNEL_WIDTH_80)
			BeamCtrlVal |= (BIT9 | BIT10 | BIT11);
	}
#endif

	else
	{
		BeamCtrlVal &= ~(BIT9|BIT10|BIT11);
	}
	

//	PlatformEFIOWrite2Byte(Adapter, BeamCtrlReg, BeamCtrlVal);
	RTL_W16(BeamCtrlReg, BeamCtrlVal);

//	panic_printk("%s Idx %d BeamCtrlReg %x BeamCtrlVal %x, bw=%d\n", __FUNCTION__, Idx, BeamCtrlReg, BeamCtrlVal, BeamformEntry.BW);
}


//2REG_C2HEVT_CLEAR
#define		C2H_EVT_HOST_CLOSE			0x00	// Set by driver and notify FW that the driver has read the C2H command message
#define		C2H_EVT_FW_CLOSE			0xFF	// Set by FW indicating that FW had set the C2H command message and it's not yet read by driver.



VOID Beamforming_NDPARate_92E(
	struct rtl8192cd_priv *priv,
	BOOLEAN		Mode,
	u1Byte		BW,
	u1Byte		Rate)
{

#if 1
	u2Byte	NDPARate = Rate;

	if(NDPARate == 0)
	{
// ???	
#if 0
		if(Mode == 1 || Mode == 4)
			NDPARate = 0x0c;		//MGN_MCS0;
		else
			NDPARate = 0x2c;		//MGN_VHT1SS_MCS0;
#else
		if(priv->pshare->rssi_min > 30) // link RSSI > 30%
			NDPARate = 8;				//MGN_24M
		else
			NDPARate = 4;				//MGN_6M

#endif
	}

	if(NDPARate < 0x0c)
		BW = HT_CHANNEL_WIDTH_20;	

	RTL_W8(REG_NDPA_OPT_CTRL, (NDPARate<<2) |  (BW & 0x03));

#else
	{
		if(mode ==0) {
#if 1
			rate = 11;		// 54M
			BW = 0;
#else
			rate = 0x0c;		//_MCS0_RATE_;
			if(BW ==3)
				BW= 2;
#endif				
		}
		else {
			rate = 0x2c;		//_NSS1_MCS0_RATE_;
			
		}
	}
	RTL_W8(0x45f, (rate<<2) |  (BW & 0x03));
#endif	
}

// V1
#if 0
void TXBF_timer_callback(unsigned long task_priv)
{
	struct rtl8192cd_priv *priv = (struct rtl8192cd_priv *)task_priv;
	struct stat_info	*pstat;
	struct list_head	*phead, *plist;
	unsigned long flags;

	if(!priv->pmib->dot11RFEntry.txbf)
		return;

	SAVE_INT_AND_CLI(flags);

	phead = &priv->asoc_list;
	plist = phead->next;

//	panic_printk(".");

// VAP ?	
	
	while(plist != phead)
	{
		pstat = list_entry(plist, struct stat_info, asoc_list);

//		if(priv->pmib->dot11RFEntry.txbf == 1)
		if((pstat->ht_cap_len && (pstat->ht_cap_buf.txbf_cap)) ||
		(pstat->vht_cap_len && (cpu_to_le32(pstat->vht_cap_buf.vht_cap_info) & BIT(SU_BFEE_S))))

		{
//				BeamformingInit(priv, pstat->hwaddr, pstat->aid);
			if(pstat->vht_cap_len) //_eric_txbf
				BeamformingControl(priv, pstat->hwaddr, pstat->aid, 2, pstat->tx_bw);
			else
				BeamformingControl(priv, pstat->hwaddr, pstat->aid, 3, pstat->tx_bw);
		}


		if (plist == plist->next)
			break;
		plist = plist->next;

	};

	RESTORE_INT(flags);

	mod_timer(&priv->txbf_timer, jiffies + priv->pshare->rf_ft_var.soundingPeriod);

}
#endif

VOID
C2HTxBeamformingHandler_92E(
	struct rtl8192cd_priv *priv,
		pu1Byte			CmdBuf,
		u1Byte			CmdLen
)
{
	u1Byte 	status = (CmdBuf[0] & BIT0);
	
//	panic_printk("%d%s status = %d \n",__LINE__,__FUNCTION__,status);
	Beamforming_End(priv, status);
}

VOID HW_VAR_HW_REG_TIMER_START_92E(struct rtl8192cd_priv *priv)
{
	panic_printk("[%d][%s]\n",__LINE__,__FUNCTION__);
	RTL_W8(0x15F, 0x0);
	RTL_W8(0x15F, 0x5);

}

VOID HW_VAR_HW_REG_TIMER_INIT_92E(struct rtl8192cd_priv *priv, u2Byte t)
{
	panic_printk("[%d][%s]\n",__LINE__,__FUNCTION__);
	RTL_W8(0x164, 1);
	RTL_W16(0x15C, t);

}

VOID HW_VAR_HW_REG_TIMER_STOP_92E(struct rtl8192cd_priv *priv)
{
	panic_printk("[%d][%s]\n",__LINE__,__FUNCTION__);
	RTL_W8(0x15F, 0);
}
#endif


