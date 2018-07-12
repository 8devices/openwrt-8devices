#ifndef __HALDEF_H__
#define __HALDEF_H__
/*++
Copyright (c) Realtek Semiconductor Corp. All rights reserved.

Module Name:
	HalComDef.h
	
Abstract:
	Defined HAL common data structure & Define
	    
Major Change History:
	When       Who               What
	---------- ---------------   -------------------------------
	2012-03-23 Filen            Create.	
--*/
#if defined(__ECOS) || defined(CPTCFG_CFG80211_MODULE)
#include "HalMacFunc.h"
#endif

typedef enum _HW_VARIABLES{
        HW_VAR_ETHER_ADDR,
        HW_VAR_MULTICAST_REG,
        HW_VAR_BSSID,
        HW_VAR_MAC_IO_ENABLE,       //Set Only
        HW_VAR_MACREGFILE_START,    //Get Only
        HW_VAR_MACREGFILE_SIZE,     //Get Only
        HW_VAR_PHYREGFILE_START,    //Get Only
        HW_VAR_PHYREGFILE_SIZE,     //Get Only        
        HW_VAR_PHYREGFILE_HP_START,    //Get Only
        HW_VAR_PHYREGFILE_HP_SIZE,     //Get Only  
        HW_VAR_PHYREGFILE_EXTPA_START,    //Get Only
        HW_VAR_PHYREGFILE_EXTPA_SIZE,     //Get Only            
        HW_VAR_PHYREGFILE_EXTLNA_START,    //Get Only
        HW_VAR_PHYREGFILE_EXTLNA_SIZE,     //Get Only    
        HW_VAR_PHYREGFILE_1T_START,    //Get Only
        HW_VAR_PHYREGFILE_1T_SIZE,     //Get Only         
        HW_VAR_PHYREGFILE_MP_START, //Get Only
        HW_VAR_PHYREGFILE_MP_SIZE,  //Get Only 
        HW_VAR_PHYREGFILE_PG_START, //Get Only
        HW_VAR_PHYREGFILE_PG_SIZE,  //Get Only   
#ifdef PWR_BY_RATE_92E_HP			
    	HW_VAR_PHYREGFILE_PG_HP_START, //Get Only
        HW_VAR_PHYREGFILE_PG_HP_SIZE,  //Get Only
#endif      
        HW_VAR_PHYREGFILE_AGC_START, //Get Only
        HW_VAR_PHYREGFILE_AGC_SIZE,  //Get Only        
        HW_VAR_PHYREGFILE_AGC_HP_START, //Get Only
        HW_VAR_PHYREGFILE_AGC_HP_SIZE,  //Get Only   
        HW_VAR_PHYREGFILE_AGC_EXTPA_START, //Get Only
        HW_VAR_PHYREGFILE_AGC_EXTPA_SIZE,  //Get Only  
        HW_VAR_PHYREGFILE_AGC_EXTLNA_START, //Get Only
        HW_VAR_PHYREGFILE_AGC_EXTLNA_SIZE,  //Get Only  
        HW_VAR_RFREGFILE_RADIO_A_START, //Get Only
        HW_VAR_RFREGFILE_RADIO_A_SIZE,  //Get Only             
        HW_VAR_RFREGFILE_RADIO_A_HP_START, //Get Only
        HW_VAR_RFREGFILE_RADIO_A_HP_SIZE,  //Get Only       
        HW_VAR_RFREGFILE_RADIO_A_EXTPA_START, //Get Only
        HW_VAR_RFREGFILE_RADIO_A_EXTPA_SIZE,  //Get Only   
        HW_VAR_RFREGFILE_RADIO_A_EXTLNA_START, //Get Only
        HW_VAR_RFREGFILE_RADIO_A_EXTLNA_SIZE,  //Get Only  
        HW_VAR_RFREGFILE_RADIO_B_START, //Get Only
        HW_VAR_RFREGFILE_RADIO_B_SIZE,  //Get Only        
        HW_VAR_RFREGFILE_RADIO_B_HP_START, //Get Only
        HW_VAR_RFREGFILE_RADIO_B_HP_SIZE,  //Get Only   
	  	HW_VAR_RFREGFILE_RADIO_B_EXTPA_START, //Get Only
		HW_VAR_RFREGFILE_RADIO_B_EXTPA_SIZE,  //Get Only
 		HW_VAR_RFREGFILE_RADIO_B_EXTLNA_START, //Get Only
		HW_VAR_RFREGFILE_RADIO_B_EXTLNA_SIZE,  //Get Only
#if defined(CONFIG_WLAN_HAL_8814AE)		
		HW_VAR_RFREGFILE_RADIO_C_START, //Get Only
		HW_VAR_RFREGFILE_RADIO_C_SIZE,	//Get Only	  
		HW_VAR_RFREGFILE_RADIO_C_HP_START, //Get Only
		HW_VAR_RFREGFILE_RADIO_C_HP_SIZE,  //Get Only	
		HW_VAR_RFREGFILE_RADIO_C_EXTPA_SIZE,
		HW_VAR_RFREGFILE_RADIO_C_EXTPA_START,
		HW_VAR_RFREGFILE_RADIO_C_EXTLNA_SIZE,
		HW_VAR_RFREGFILE_RADIO_C_EXTLNA_START,
		HW_VAR_RFREGFILE_RADIO_D_START, //Get Only
		HW_VAR_RFREGFILE_RADIO_D_SIZE,	//Get Only	  
		HW_VAR_RFREGFILE_RADIO_D_HP_START, //Get Only
		HW_VAR_RFREGFILE_RADIO_D_HP_SIZE,  //Get Only	
		HW_VAR_RFREGFILE_RADIO_D_EXTPA_SIZE,
		HW_VAR_RFREGFILE_RADIO_D_EXTPA_START,
		HW_VAR_RFREGFILE_RADIO_D_EXTLNA_SIZE,
		HW_VAR_RFREGFILE_RADIO_D_EXTLNA_START,		
#endif		
        HW_VAR_FWFILE_START,        //Get Only
        HW_VAR_FWFILE_SIZE,         //Get Only        
        HW_VAR_TXPKTFWFILE_START,        //Get Only
        HW_VAR_TXPKTFWFILE_SIZE,         //Get Only
        HW_VAR_POWERTRACKINGFILE_START,  //Get Only
        HW_VAR_POWERTRACKINGFILE_SIZE,   //Get Only        
        HW_VAR_POWERLIMITFILE_START,  //Get Only
        HW_VAR_POWERLIMITFILE_SIZE,   //Get Only    
#ifdef BEAMFORMING_AUTO		
        HW_VAR_POWERLIMITFILE_TXBF_START,  //Get Only
        HW_VAR_POWERLIMITFILE_TXBF_SIZE,   //Get Only    
#endif		
#ifdef PWR_BY_RATE_92E_HP
        HW_VAR_POWERLIMITFILE_HP_START,  //Get Only
        HW_VAR_POWERLIMITFILE_HP_SIZE,   //Get Only    
#endif        
        HW_VAR_MEDIA_STATUS,
        HW_VAR_MAC_LOOPBACK_ENABLE, //Set Only
        HW_VAR_MAC_CONFIG,          //Set Only
        HW_VAR_EDCA,                //Set Only
        HW_VAR_CAM_RESET_ALL_ENTRY, //Set Only
        HW_VAR_SECURITY_CONFIG,
        HW_VAR_BEACON_INTERVAL,
        HW_VAR_ENABLE_BEACON_DMA,
        HW_VAR_TXPAUSE,
        HW_VAR_HIQ_NO_LMT_EN,
        HW_VAR_DRV_DBG,
        HW_VAR_NUM_TOTAL_RF_PATH,    //Get Only
        HW_VAR_NUM_RXDMA_STATUS,
        HW_VAR_NUM_TXDMA_STATUS,
        HW_VAR_BEACON_ENABLE_DOWNLOAD,
        HW_VAR_BEACON_DISABLE_DOWNLOAD,
        HW_VAR_TSF_TIMER,           //Get Only
        HW_VAR_PS_TIMER,
#if CFG_HAL_MACDM
//3 MACDM
        //Default
        HW_VAR_MACDM_DEF_LOW_START,    //Get Only
        HW_VAR_MACDM_DEF_LOW_SIZE,     //Get Only
        HW_VAR_MACDM_DEF_NORMAL_START,    //Get Only
        HW_VAR_MACDM_DEF_NORMAL_SIZE,     //Get Only
        HW_VAR_MACDM_DEF_HIGH_START,    //Get Only
        HW_VAR_MACDM_DEF_HIGH_SIZE,     //Get Only

        //General
        HW_VAR_MACDM_GEN_LOW_START,    //Get Only
        HW_VAR_MACDM_GEN_LOW_SIZE,     //Get Only
        HW_VAR_MACDM_GEN_NORMAL_START,    //Get Only
        HW_VAR_MACDM_GEN_NORMAL_SIZE,     //Get Only
        HW_VAR_MACDM_GEN_HIGH_START,    //Get Only
        HW_VAR_MACDM_GEN_HIGH_SIZE,     //Get Only

        //Txop
        HW_VAR_MACDM_TXOP_LOW_START,    //Get Only
        HW_VAR_MACDM_TXOP_LOW_SIZE,     //Get Only
        HW_VAR_MACDM_TXOP_NORMAL_START,    //Get Only
        HW_VAR_MACDM_TXOP_NORMAL_SIZE,     //Get Only
        HW_VAR_MACDM_TXOP_HIGH_START,    //Get Only
        HW_VAR_MACDM_TXOP_HIGH_SIZE,     //Get Only

        //Criteria
        HW_VAR_MACDM_CRITERIA_START,    //Get Only
        HW_VAR_MACDM_CRITERIA_SIZE,      //Get Only
#endif
#if CFG_HAL_HW_DETEC_POWER_STATE        
        HW_VAR_HW_PS_STATE0,
        HW_VAR_HW_PS_STATE1,        
        HW_VAR_HW_PS_STATE2,        
        HW_VAR_HW_PS_STATE3,        
#endif //#if CFG_HAL_HW_DETEC_POWER_STATE    
#if CFG_HAL_AP_PS_OFFLOAD
        HW_VAR_MACID_SLEEP0,
        HW_VAR_MACID_SLEEP1,        
        HW_VAR_MACID_SLEEP2,        
        HW_VAR_MACID_SLEEP3,        
#endif //#if CFG_HAL_AP_PS_OFFLOAD    
#if CFG_HAL_MULTICAST_BMC_ENHANCE
        HW_VAR_BMC_RTS0_INVALID,
        HW_VAR_BMC_RTS0_ADDR,        
        HW_VAR_BMC_RTS1_INVALID,
        HW_VAR_BMC_RTS1_ADDR,                
#endif //#if CFG_HAL_MULTICAST_BMC_ENHANCE
		HW_VAR_REG_CCK_CHECK,
#if (IS_RTL8192E_SERIES | IS_RTL8881A_SERIES)
		HW_VAR_HWSEQ_CTRL,
#endif //#if (IS_RTL8192E_SERIES | IS_RTL8881A_SERIES)
		HW_VAR_REG_CR,
        HW_VAR_BCN_HEAD,
        HW_VAR_RXPKT_NUM,

}HW_VARIABLES;


// The type used to query whether the interrupt in HAL is toggled.
typedef enum _HAL_INT_TYPE
{
	HAL_INT_TYPE_ANY,				// Any interrupt
	HAL_INT_TYPE_TBDOK,				// Tx Beacon OK
	HAL_INT_TYPE_TBDER,				// Tx Beacon error
	HAL_INT_TYPE_BcnInt,			// For 92C or later, it should be early beacon interrupt.
	HAL_INT_TYPE_PSTIMEOUT,			// PS timer interrupt by TSF
	HAL_INT_TYPE_PSTIMEOUT1,		// PS timer 1 interrupt by TSF	
	HAL_INT_TYPE_PSTIMEOUT2,		// PS timer 2 interrupt by TSF	
	HAL_INT_TYPE_C2HCMD,			// CPU to Host Command INT Status interrupt
	HAL_INT_TYPE_RXFOVW,			// Rx FIFO over flow
	HAL_INT_TYPE_VIDOK,				// VI queue DMA OK
	HAL_INT_TYPE_VODOK,				// VO queue DMA OK
	HAL_INT_TYPE_BEDOK,				// BE queue DMA OK
	HAL_INT_TYPE_BKDOK,				// BK queue DMA OK
	HAL_INT_TYPE_MGNTDOK,			// Mgnt queue DMA OK
	HAL_INT_TYPE_HIGHDOK,			// High queue DMA OK
	HAL_INT_TYPE_BDOK,				// Beacon queue DMA OK  , Note: Filen, this interrupt has removed in 8812 & later chip
	HAL_INT_TYPE_CPWM,				// CPU power Mode exchange INT Status
	HAL_INT_TYPE_TSF_BIT32_TOGGLE,	// TSF Timer BIT32 toggle indication interrupt
	HAL_INT_TYPE_RX_OK,				// Receive DMA OK
	HAL_INT_TYPE_RDU,
	HAL_INT_TYPE_BcnInt_MBSSID,		// For 92C or later, it should be early beacon interrupt.	
	HAL_INT_TYPE_BcnInt1,
	HAL_INT_TYPE_BcnInt2,
	HAL_INT_TYPE_BcnInt3,
	HAL_INT_TYPE_BcnInt4,
	HAL_INT_TYPE_BcnInt5,
	HAL_INT_TYPE_BcnInt6,
	HAL_INT_TYPE_BcnInt7,
	HAL_INT_TYPE_CTWEND,
	HAL_INT_TYPE_BCNDERR0,
	HAL_INT_TYPE_TXFOVW,            // Transmit packet buffer Overflow.
	HAL_INT_TYPE_RXERR,             // Rx Error Flag INT Status
	HAL_INT_TYPE_TXERR,             // Tx Error Flag INT Status    
	HAL_INT_TYPE_GTIMER4,           // Gtimer 4.
	HAL_INT_TYPE_AXI_EXCEPTION,     // Add AXI exception count.
    HAL_INT_TYPE_CPWM2,

#if CFG_HAL_TX_AMSDU == 1 || defined(P2P_SUPPORT)
	HAL_INT_TYPE_FS_TIMEOUT0,
#endif	
	#if 0   //Filen: Not used to AP Platform
	//==== SDIO Specified Interrupt=====//
	HAL_INT_TYPE_SDIO_ISR_IND,
	HAL_INT_TYPE_SDIO_GPIO12_0_INT,
	HAL_INT_TYPE_SDIO_SPS_OCP_INT,
	HAL_INT_TYPE_SDIO_RON_INT_EN,
	HAL_INT_TYPE_SDIO_PDNINT,
	HAL_INT_TYPE_SDIO_GPIO9_INT,
	#endif
#if CFG_HAL_SUPPORT_EACH_VAP_INT
    HAL_INT_TYPE_TXBCNOK_MBSSID,
    HAL_INT_TYPE_TXBCN1OK,
    HAL_INT_TYPE_TXBCN2OK,
    HAL_INT_TYPE_TXBCN3OK,
    HAL_INT_TYPE_TXBCN4OK,    
    HAL_INT_TYPE_TXBCN5OK,        
    HAL_INT_TYPE_TXBCN6OK,    
    HAL_INT_TYPE_TXBCN7OK,    
    HAL_INT_TYPE_TXBCNERR_MBSSID,    
    HAL_INT_TYPE_TXBCN1ERR,    
    HAL_INT_TYPE_TXBCN2ERR,    
    HAL_INT_TYPE_TXBCN3ERR,    
    HAL_INT_TYPE_TXBCN4ERR,        
    HAL_INT_TYPE_TXBCN5ERR,        
    HAL_INT_TYPE_TXBCN6ERR,        
    HAL_INT_TYPE_TXBCN7ERR,              
    HAL_INT_TYPE_PwrInt0,
	HAL_INT_TYPE_PwrInt1,    
	HAL_INT_TYPE_PwrInt2,	
	HAL_INT_TYPE_PwrInt3,	
	HAL_INT_TYPE_PwrInt4,	
#endif //#if CFG_HAL_SUPPORT_EACH_VAP_INT

}HAL_INT_TYPE, *PHAL_INT_TYPE;

enum _XTAL_CLK_SEL_ {
	XTAL_CLK_SEL_40M = 0,
	XTAL_CLK_SEL_25M = 1
};

typedef struct _MACCONFIG_PARA_ {
    u4Byte     AckTO;
    u4Byte     vap_enable;
    u4Byte     OP_Mode;
    u2Byte     dot11DTIMPeriod;
	u1Byte     WdsPure;
    // TODO:
} MACCONFIG_PARA, *PMACCONFIG_PARA;

typedef struct _EDCA_PARA_ {
    u4Byte              slot_time;
    u4Byte              sifs_time;
    struct ParaRecord   Para[AC_PARAM_SIZE];
}EDCA_PARA, *PEDCA_PARA;

typedef struct _CAM_ENTRY_CFG_ {
    BOOLEAN             bValid;
    u1Byte              KeyID;
    DOT11_ENC_ALGO      EncAlgo;
}CAM_ENTRY_CFG, *PCAM_ENTRY_CFG;

typedef enum _TXRPT_VARIABLES {
    TXRPT_VAR_DATA_RATE,            // offset 0
    TXRPT_VAR_PKT_DROP,             // offset 1
    TXRPT_VAR_DATA_RTY_LOW_RATE,    // offset 3,shift 0
    TXRPT_VAR_RTY_LOW_RATE_EN,      // offset 3,shift 7
    TXRPT_VAR_RTS_RTY_LOW_RATE,     // offset 4,shift 0
    TXRPT_VAR_RTY_LMT_EN,           // offset 4,shift 7
    TXRPT_VAR_DATA_RT_LMT,          // offset 5,shift 0
    TXRPT_VAR_PKT_TX_ONE_SEL,       // offset 5,shift 6    
    TXRPT_VAR_MAC_ADDRESS,          // offset 16,shift 0,mask 
    TXRPT_VAR_SPECIALQ_PKT_NUM1,	//offset 14, shift 0 
    TXRPT_VAR_SPECIALQ_PKT_NUM2,	//offset15, shift 0, mask 0xF
    TXRPT_VAR_ALL,        
}TXRPT_VARIABLES;

typedef enum _HAL_HANG_TYPE {
    HANG_VAR_NORMAL,
    HANG_VAR_TX_DESC_LEN_ERROR,
    HANG_VAR_TX_STUCK,
    HANG_VAR_RX_STUCK,
} HAL_HANG_TYPE;


//-----------------------------------------------------------
//
//	Queue mapping
//
//-----------------------------------------------------------
//1.) used to TXPOLL
#define TXPOLL_BK_QUEUE						0
#define TXPOLL_BE_QUEUE						1
#define TXPOLL_VI_QUEUE						2
#define TXPOLL_VO_QUEUE						3
#define TXPOLL_BEACON_QUEUE					4
//#define TXPOLL_TXCMD_QUEUE				5
#define TXPOLL_MGNT_QUEUE					6
#define TXPOLL_HIGH_QUEUE					7
#define TXPOLL_HCCA_QUEUE					8

//2.) used to TXPAUSE
#define TXPAUSE_BK_QUEUE_BIT                    BIT0
#define TXPAUSE_BE_QUEUE_BIT                    BIT1
#define TXPAUSE_VI_QUEUE_BIT                    BIT2
#define TXPAUSE_VO_QUEUE_BIT                    BIT3
#define TXPAUSE_MGNT_QUEUE_BIT                  BIT4
#define TXPAUSE_HIGH_QUEUE_BIT                  BIT5
#define TXPAUSE_BCN_QUEUE_BIT                   BIT6
#define TXPAUSE_BCN_HI_MGNT_QUEUE_BIT           BIT7

#define TXPAUSE_ALL_QUEUE_BIT                   0xFF


//3 Initialization Related
typedef RT_STATUS
(*NicInitPONHandler)(
    INPUT	HAL_PADAPTER		Adapter,
    INPUT   u4Byte          	ClkSel    
    );

typedef RT_STATUS
(*NicInitMACHandler)(
    INPUT	HAL_PADAPTER		Adapter
    );

typedef VOID
(*NicInitIMRHandler)(
    INPUT	HAL_PADAPTER		Adapter,
    INPUT   RT_OP_MODE          OPMode
    );

typedef RT_STATUS
(*NicInitFirmwareHandler)(
    INPUT	HAL_PADAPTER		Adapter
    );

typedef RT_STATUS
(*NicInitHCIDMAMemHandler)(
    INPUT	HAL_PADAPTER		Adapter
    );

typedef RT_STATUS
(*NicInitHCIDMARegHandler)(
    INPUT	HAL_PADAPTER		Adapter
    );

//MULTI_MAC_CLONE
typedef VOID
(*NicMcloneSetMBSSIDHandler)(
    INPUT	HAL_PADAPTER		Adapter,
    IN		pu1Byte 	 		macAddr,
    IN		int					entIdx
    );

//MULTI_MAC_CLONE
typedef VOID
(*NicMcloneStopMBSSIDHandler)(
    INPUT	HAL_PADAPTER		Adapter,
    IN		int					entIdx
    );

typedef VOID
(*NicInitMBSSIDHandler)(
    INPUT	HAL_PADAPTER		Adapter
    );

typedef VOID
(*NicInitMBIDCAMHandler)(
    INPUT	HAL_PADAPTER		Adapter
    );

typedef VOID
(*NicStopMBSSIDHandler)(
    INPUT	HAL_PADAPTER		Adapter
    );
typedef RT_STATUS 
(*NicSetMBIDCAMHandler)(
    IN  HAL_PADAPTER Adapter,
    IN  u1Byte       MBID_Addr,    
    IN  u1Byte       IsRoot
);

typedef RT_STATUS 
(*NicInitMACIDSearchHandler)(
    INPUT  HAL_PADAPTER Adapter    
);



typedef RT_STATUS 
(*NicStopMBIDCAMHandler)(
    IN  HAL_PADAPTER Adapter,
    IN  u1Byte       MBID_Addr
);

typedef RT_STATUS 
(*NicCheckHWMACIDResultHandler)(
    INPUT  HAL_PADAPTER        Adapter,    
    INPUT  u4Byte              MacID,
    OUTPUT pu1Byte             result
);

typedef VOID
(*NicInitVAPIMRHandler)(
    INPUT  HAL_PADAPTER    Adapter,
    INPUT  u4Byte          VapSeq
);

typedef RT_STATUS
(*NicInitLLT_TableHandler)(
    INPUT   HAL_PADAPTER        Adapter
);


//3 Stop Related
typedef VOID
(*NicDisableVXDAPHandler)(
    INPUT	HAL_PADAPTER		Adapter
);


//3 ISR Related 
typedef VOID
(*NicEnableIMRHandler)(
    INPUT	HAL_PADAPTER		Adapter
);

typedef BOOLEAN
(*NicInterruptRecognizedHandler)(
    INPUT   HAL_PADAPTER    Adapter,
    INPUT   PVOID           pContent,
    INPUT   u4Byte          ContentLen
	);

typedef BOOLEAN
(*NicGetInterruptHandler)(
    IN  HAL_PADAPTER    Adapter,
    IN  HAL_INT_TYPE	intType
	);

typedef VOID
(*NicAddInterruptMaskHandler)(
    IN  HAL_PADAPTER    Adapter,
    IN  HAL_INT_TYPE	intType
	);

typedef VOID
(*NicRemoveInterruptMaskHandler)(
    IN  HAL_PADAPTER    Adapter,
    IN  HAL_INT_TYPE	intType
	);

typedef VOID
(*NicDisableRxRelatedInterruptHandler)(
    IN  HAL_PADAPTER    Adapter
	);

typedef VOID
(*NicEnableRxRelatedInterruptHandler)(
    IN  HAL_PADAPTER    Adapter
	);


//3 Tx Related
typedef RT_STATUS
(*NicPrepareTxBufferDescriptorHandler)(
    INPUT	HAL_PADAPTER		Adapter
    );

typedef VOID
(*NicTxPollingHandler)(
	INPUT	HAL_PADAPTER		Adapter,
	INPUT	u1Byte  			QueueIndex
	);

typedef VOID
(*NicFillBeaconDescHandler)
(
    INPUT  HAL_PADAPTER        Adapter,
    INPUT  PVOID               pdesc,
    INPUT  PVOID               dat_content,
    INPUT  u2Byte              txLength,
    INPUT  BOOLEAN             bForceUpdate
);

typedef VOID
(*NicSigninBeaconTXBDHandler)(
    INPUT	HAL_PADAPTER        Adapter,
    INPUT   pu4Byte             beaconbuf,
    INPUT   u2Byte              frlen
	);

typedef VOID
(*NicSetBeaconDownloadHandler) (
    INPUT	HAL_PADAPTER        Adapter,
    INPUT   u4Byte              Value
);

typedef u2Byte
(*NicGetTxQueueHWIdxHandler)(
    IN	HAL_PADAPTER        Adapter,
    IN  u4Byte              q_num       //enum _TX_QUEUE_
);

typedef u4Byte
(*NicMappingTxQueueHandler)(
    IN  HAL_PADAPTER    Adapter,
    IN  u4Byte          TxQNum      //enum _TX_QUEUE_
	);

typedef BOOLEAN
(*NicQueryTxConditionMatchHandler)(
    IN	HAL_PADAPTER    Adapter
    );

typedef BOOLEAN
(*NicFillTxHwCtrlHandler)(
    IN  HAL_PADAPTER    Adapter,
    IN  u4Byte          queueIndex,  //HCI_TX_DMA_QUEUE_88XX
    IN  PVOID           pDescData
    );

typedef RT_STATUS
(*NicSyncSWTXBDHostIdxToHWHandler) (
    IN  HAL_PADAPTER    Adapter,
    IN  u4Byte          queueIndex  //HCI_TX_DMA_QUEUE_88XX
    );

typedef PVOID
(*NicGetShortCutTxDescriptorHandler) (
    IN  HAL_PADAPTER    Adapter
    );

typedef VOID
(*NicReleaseShortCutTxDescriptorHandler)(
    IN  HAL_PADAPTER    Adapter,
    IN  PVOID           pTxDesc
    );

typedef VOID
(*NicSetShortCutTxBuffSizeHandler) (
    IN  HAL_PADAPTER    Adapter,
    IN  PVOID           pTxDesc,
    IN  u2Byte          txPktSize
    );

typedef u2Byte
(*NicGetShortCutTxBuffSizeHandler)(
    IN  HAL_PADAPTER    Adapter,
    IN  PVOID           pTxDesc
    );

typedef PVOID
(*NicCopyShortCutTxDescriptorHandler)(
    IN  HAL_PADAPTER    Adapter,
    IN  u4Byte          queueIndex, //HCI_TX_DMA_QUEUE_88XX
    IN  PVOID           pTxDesc,
    IN  u4Byte          direction    
    );

typedef BOOLEAN
(*NicFillShortCutTxHwCtrlHandler)(
    IN  HAL_PADAPTER    Adapter,
    IN  u4Byte          queueIndex,  //HCI_TX_DMA_QUEUE_88XX
    IN  PVOID           pDescData,
    IN  PVOID           pTxDesc,
    IN  u4Byte          direction,
    IN  u4Byte          index
    );

typedef RT_STATUS
(*NicReleaseOnePacketHandler) (
    IN  HAL_PADAPTER        Adapter,
    IN  u1Byte              macID
);

typedef RT_STATUS
(*NicGetTxRPTHandler)(
    IN	HAL_PADAPTER        Adapter,
    IN	u4Byte              macID,
    IN  u1Byte              variable, 
    IN	u1Byte				offset,
    OUT pu1Byte             val    
);

typedef RT_STATUS
(*NicSetTxRPTHandler)(
    IN	HAL_PADAPTER        Adapter,
    IN	u4Byte              macID,
    IN  u1Byte              variable,        
    IN  pu1Byte             val      
);


typedef VOID
(*NicSetCRC5ToRPTBufferHandler)(
    IN  HAL_PADAPTER        Adapter,
    IN  u1Byte              val,
    IN  u4Byte              macID,
    IN  u1Byte              bValid        
);

typedef VOID
(*NicFillTxDescHandler)(
    IN  HAL_PADAPTER    Adapter,
    IN  u4Byte          queueIndex,  //HCI_TX_DMA_QUEUE_88XX
    IN  PVOID           pDescData
);

typedef VOID
(*NicFillShortCutTxDescHandler)(
    IN      HAL_PADAPTER    Adapter,
    IN      u4Byte          queueIndex,  //HCI_TX_DMA_QUEUE_88XX
    IN      PVOID           pDescData,
    IN      PVOID           pTxDesc
);

typedef VOID
(*NicFillHwShortCutTxDescHandler)(
    IN  HAL_PADAPTER    Adapter,
    IN  u4Byte          queueIndex,  //HCI_TX_DMA_QUEUE_88XX
    IN  PVOID           pDescData
);

typedef VOID
(*NicFillRsrvPageDescHandler)
(
    INPUT  HAL_PADAPTER        Adapter,
    INPUT  PVOID               pdesc,
    INPUT  PVOID               dat_content,
    INPUT  u2Byte              txLength
);

typedef RT_STATUS
(*NicInitDDMAHandler)(
    IN  HAL_PADAPTER    Adapter,
    IN  u4Byte	source,
    IN  u4Byte	dest,
    IN  u4Byte 	length
);



//3 Rx Related
typedef RT_STATUS
(*NicPrepareRXBDHandler)(
    INPUT	HAL_PADAPTER		Adapter,
    INPUT   u2Byte              bufferLen,
    INPUT   PVOID               Callback
    );

typedef RT_STATUS
(*NicQueryRxDescHandler) (
    INPUT   HAL_PADAPTER    Adapter,
    INPUT   u4Byte          queueIndex,
    INPUT   pu1Byte         pBufAddr,
    OUTPUT  PVOID           pRxDescStatus    
    );

typedef RT_STATUS 
(*NicUpdateRXBDInfoHandler)(
    IN      HAL_PADAPTER    Adapter,
    IN      u4Byte          queueIndex,  //HCI_RX_DMA_QUEUE_88XX
    IN      u2Byte          rxbd_idx,
    IN      pu1Byte         pBuf,
    IN      PVOID           Callback,    // callback function    
    IN      BOOLEAN         bInit
);

typedef u4Byte	
(*NicReadableRxBufferDescCountHandler)(
    INPUT   HAL_PADAPTER	    Adapter,
	INPUT   u4Byte		        queueIndex
    );

typedef VOID
(*NicUpdateRXBDHostIdxHandler)(
    IN      HAL_PADAPTER    Adapter,
    IN      u4Byte          queueIndex,  //HCI_TX_DMA_QUEUE_88XX
    IN      u4Byte          Count
    );

typedef u2Byte	
(*NicUpdateRXBDHWIdxHandler)(
    IN  HAL_PADAPTER    Adapter,
	IN  u4Byte		    queueIndex  //HCI_TX_DMA_QUEUE_88XX
    );

//3 General operation
typedef MIMO_TR_STATUS
(*NicGetChipIDMIMOHandler)(
	INPUT	HAL_PADAPTER		Adapter
	);

typedef VOID
(*NicSetHwRegHandler)(
	INPUT	HAL_PADAPTER		Adapter,
	INPUT	u1Byte				RegName,
	INPUT	pu1Byte				val
	);

typedef VOID
(*NicGetHwRegHandler)(
	INPUT	HAL_PADAPTER		Adapter,
	INPUT	u1Byte				RegName,
	OUTPUT	pu1Byte				val
	);

typedef RT_STATUS
(*NicGetMACIDQueueInTXPKTBUFHandler)(
    INPUT  HAL_PADAPTER Adapter,
    OUTPUT  pu1Byte     MACIDList  
    );


typedef RT_STATUS
(*NicSetMACIDSleepHandler)(
    INPUT  HAL_PADAPTER Adapter,
    INPUT  BOOLEAN      bSleep,   
    INPUT  u4Byte       aid
    );

typedef VOID
(*NicCAMReadMACConfigHandler)(
	INPUT	HAL_PADAPTER		Adapter,
    INPUT   u1Byte              index, 
    OUTPUT  pu1Byte             pMacad,
    OUTPUT  PCAM_ENTRY_CFG      pCfg
	);


//3 Timer Related
typedef VOID
(*NicTimer1SecHandler)(
    IN  HAL_PADAPTER    Adapter
);


typedef VOID
(*NicTimer1SecDMHandler)(
    IN  HAL_PADAPTER    Adapter
);


//3 Security Related    
typedef VOID
(*NicCAMEmptyEntryHandler)(
	INPUT	HAL_PADAPTER		Adapter,
    INPUT   u1Byte              index
	);

typedef u4Byte
(*NicCAMFindUsableHandler)(
	INPUT	HAL_PADAPTER		Adapter,
    INPUT   u4Byte              for_begin
	);

typedef VOID
(*NicCAMProgramEntryHandler)(
	INPUT	HAL_PADAPTER		Adapter,
    INPUT   u1Byte              index,
    INPUT   pu1Byte             macad,
    INPUT   pu1Byte             key128,
    INPUT   u2Byte              config
	);

typedef RT_STATUS
(*NicStopHWHandler)(
	INPUT	HAL_PADAPTER		Adapter
	);

typedef RT_STATUS
(*NicStopSWHandler)(
	INPUT	HAL_PADAPTER		Adapter
	);

typedef RT_STATUS
(*NicResetHWForSurpriseHandler)(
	INPUT	HAL_PADAPTER		Adapter
	);



//3 Firmware CMD IO related
typedef RT_STATUS
(*NicFillH2CCmdHandler)(
	IN  HAL_PADAPTER    Adapter,
	IN	u1Byte 		    ElementID,
	IN	u4Byte 		    CmdLen,
	IN	pu1Byte		    pCmdBuffer
    );

typedef VOID
(*NicUpdateHalRAMaskHandler)(
	IN HAL_PADAPTER         Adapter,	
	HAL_PSTAINFO            pEntry,
	u1Byte				    rssi_level
    );

typedef VOID
(*NicUpdateHalMSRRPTHandler)(
	IN HAL_PADAPTER     Adapter,
	HAL_PSTAINFO        pEntry,
	u1Byte              opmode
    );

typedef VOID
(*NicSetAPOffloadHandler)(
    IN HAL_PADAPTER     Adapter,
    u1Byte              bEn,
    u1Byte              numOfAP,
    u1Byte              bHidden,    
    u1Byte              bDenyAny,
    pu1Byte             loc_bcn,
    pu1Byte             loc_probe
    );
#ifdef AP_PS_Offlaod	
typedef VOID
(*NicSetAPPSOffloadHandler)(    
    IN HAL_PADAPTER     Adapter,    
    u1Byte              _PS_offload_En,    
    u1Byte              _phase,    
    u1Byte              _pause_Qnum_limit,        
    u1Byte              _timeout_time    
    );
typedef VOID
(*NicAPPSOffloadMACIDPauseHandler)(
    IN HAL_PADAPTER     Adapter,      
    u1Byte              macid,    
    u1Byte              setPause    
    );
#endif

typedef VOID
(*NicSetSAPPsHandler)(
    IN HAL_PADAPTER     Adapter,
    u1Byte              bEn,
    u1Byte              duration
    );

typedef VOID
(*NicSetRsvdPageHandler) ( 
	IN  IN HAL_PADAPTER     Adapter,
    IN  pu1Byte             prsp,
    IN  pu1Byte             beaconbuf,    
    IN  u4Byte              pktLen,  
    IN  u4Byte              bigPktLen,
    IN  u4Byte              bcnLen
    );

typedef u4Byte
(*NicGetRsvdPageLocHandler)(
	IN  IN HAL_PADAPTER     Adapter,
    IN  u4Byte              frlen,
    OUT pu1Byte             loc_page
    );

typedef BOOLEAN
(*NicDownloadRsvdPageHandler)(
	IN HAL_PADAPTER     Adapter,
    IN  pu1Byte         beaconbuf,    
    IN  u4Byte          beaconPktLen,
    IN  u1Byte          bReDownload    
    );

typedef void 
(*NicC2HHandler)(
    IN HAL_PADAPTER     Adapter
);

typedef void 
(*NicC2HPacketHandler)(
	IN  HAL_PADAPTER    Adapter,
	IN  pu1Byte			pBuf,
	IN	u1Byte			length
);

typedef VOID
(*DumpRxBDescHandler)(
	IN HAL_PADAPTER     Adapter,
#ifdef CONFIG_RTL_PROC_NEW
	IN struct seq_file *s,
#endif
	u4Byte              q_num
    );

typedef VOID
(*DumpTxBDescHandler)(
	IN HAL_PADAPTER     Adapter,
#ifdef CONFIG_RTL_PROC_NEW
	IN struct seq_file *s,
#endif
	u4Byte              q_num
    );

typedef u4Byte
(*NicCheckHangHandler)(
    IN	HAL_PADAPTER    Adapter
);

typedef void
(*NicSetStatistic_ReportHandler)(
    IN HAL_PADAPTER  Adapter,
    u1Byte report_en,
	u1Byte report_type,
	u1Byte report_interval
);
// 4. RF setting related

typedef RT_STATUS
(*NicPHYSetCCKTxPowerHandler)(
    IN  HAL_PADAPTER    Adapter, 
    IN  u1Byte          channel
    );

typedef RT_STATUS
(*NicPHYSetOFDMTxPowerHandler)(
    IN  HAL_PADAPTER    Adapter, 
    IN  u1Byte          channel
    );

typedef VOID
(*NicPHYUpdateBBRFValHandler)(
    IN  HAL_PADAPTER    Adapter, 
    IN  u1Byte          channel,
    IN  s4Byte          offset  
    );

typedef VOID
(*NicPHYSwBWModeHandler)(
    IN  HAL_PADAPTER    Adapter, 
    IN  u4Byte          bandwidth,
    IN  s4Byte          offset
    );

typedef VOID
(*NicTXPowerTrackingHandler)(
    IN  HAL_PADAPTER    Adapter 
    );

typedef void 
(*NicPHYSSetRFRegHandler)(
    IN  HAL_PADAPTER                Adapter, 
    IN  u4Byte                      eRFPath,
    IN  u4Byte                      RegAddr,    
    IN  u4Byte                      BitMask,
    IN  u4Byte                      Data
);

typedef u4Byte
(*NicPHYQueryRFRegHandler)(
    IN  HAL_PADAPTER                Adapter, 
    IN  u4Byte                      eRFPath,
    IN  u4Byte                      RegAddr,    
    IN  u4Byte                      BitMask
);

typedef BOOLEAN
(*NicIsBBRegRangeHandler)(
    IN  HAL_PADAPTER                Adapter, 
    IN  u4Byte                      RegAddr
);

typedef VOID
(*NicPHYSetSecCCATHbyRXANT)(
    IN  HAL_PADAPTER                Adapter,
    IN  u4Byte                      ulAntennaRx
);

typedef VOID
(*NicPHYSpurCalibration)(
    IN  HAL_PADAPTER                Adapter
);

typedef VOID
(*NicPHYConfigBB)(
    IN  HAL_PADAPTER                Adapter
);

typedef struct _HAL_INTERFACE_COMMON_{


    //
    // WLAN Device operations. 
    //

    //3 Initialization Related
    NicInitPONHandler               InitPONHandler;
    NicInitMACHandler               InitMACHandler;
    NicInitIMRHandler               InitIMRHandler;
    NicInitFirmwareHandler          InitFirmwareHandler;
    NicInitHCIDMAMemHandler         InitHCIDMAMemHandler;
    NicInitHCIDMARegHandler         InitHCIDMARegHandler;    
    NicInitMBSSIDHandler            InitMBSSIDHandler;
    NicInitMBIDCAMHandler           InitMBIDCAMHandler;
    NicMcloneSetMBSSIDHandler       McloneSetMBSSIDHandler;//MULTI_MAC_CLONE
    NicMcloneStopMBSSIDHandler	    McloneStopMBSSIDHandler;//MULTI_MAC_CLONE
    NicInitLLT_TableHandler         InitLLT_TableHandler;
    NicInitMACIDSearchHandler       InitMACIDSearchHandler;    
    NicSetMBIDCAMHandler            SetMBIDCAMHandler;
	NicCheckHWMACIDResultHandler    CheckHWMACIDResultHandler;     
    NicInitVAPIMRHandler            InitVAPIMRHandler;

    //3 Stop Related
    NicStopMBSSIDHandler            StopMBSSIDHandler;
    NicStopHWHandler                StopHWHandler;
    NicStopSWHandler                StopSWHandler;
    NicDisableVXDAPHandler          DisableVXDAPHandler;
    NicStopMBIDCAMHandler           StopMBIDCAMHandler;	
    NicResetHWForSurpriseHandler    ResetHWForSurpriseHandler;

    //3 ISR Related 
    NicEnableIMRHandler                     EnableIMRHandler;
#if IS_EXIST_PCI || IS_EXIST_EMBEDDED
    NicInterruptRecognizedHandler           InterruptRecognizedHandler;
    NicGetInterruptHandler                  GetInterruptHandler;
    NicAddInterruptMaskHandler              AddInterruptMaskHandler;
    NicRemoveInterruptMaskHandler           RemoveInterruptMaskHandler;
    NicDisableRxRelatedInterruptHandler     DisableRxRelatedInterruptHandler;
    NicEnableRxRelatedInterruptHandler      EnableRxRelatedInterruptHandler;
#endif

    //3 General operation
    NicGetChipIDMIMOHandler     GetChipIDMIMOHandler;
    NicSetHwRegHandler          SetHwRegHandler;
    NicGetHwRegHandler          GetHwRegHandler;
    NicSetMACIDSleepHandler     SetMACIDSleepHandler;
    NicGetMACIDQueueInTXPKTBUFHandler   GetMACIDQueueInTXPKTBUFHandler;

    //3 Timer Related
    NicTimer1SecHandler                 Timer1SecHandler;
    NicTimer1SecDMHandler               Timer1SecDMHandler;

    //3 Security Related	
    //CAM
    NicCAMReadMACConfigHandler  CAMReadMACConfigHandler;
    NicCAMEmptyEntryHandler     CAMEmptyEntryHandler;
    NicCAMFindUsableHandler     CAMFindUsableHandler;
    NicCAMProgramEntryHandler   CAMProgramEntryHandler;      

    //3 PHY/RF Related
    NicPHYSetCCKTxPowerHandler              PHYSetCCKTxPowerHandler;
    NicPHYSetOFDMTxPowerHandler             PHYSetOFDMTxPowerHandler;
    NicPHYUpdateBBRFValHandler              PHYUpdateBBRFValHandler;
    NicPHYSwBWModeHandler                   PHYSwBWModeHandler;
    NicTXPowerTrackingHandler               TXPowerTrackingHandler;
    NicPHYSSetRFRegHandler                  PHYSSetRFRegHandler;    
    NicPHYQueryRFRegHandler                 PHYQueryRFRegHandler;
    NicIsBBRegRangeHandler                  IsBBRegRangeHandler;
    NicPHYSetSecCCATHbyRXANT                PHYSetSecCCATHbyRXANT;
    NicPHYSpurCalibration                   PHYSpurCalibration;
    NicPHYConfigBB                          PHYConfigBB;

    //3 Firmware CMD IO related

    //
    //  Special Operation for each Chip type
    //
#if IS_RTL88XX_GENERATION

#if (HAL_DEV_BUS_TYPE & (HAL_RT_EMBEDDED_INTERFACE | HAL_RT_PCI_INTERFACE))
    //3 Tx Related
    NicTxPollingHandler                     TxPollingHandler;
    NicSigninBeaconTXBDHandler              SigninBeaconTXBDHandler;
    NicSetBeaconDownloadHandler             SetBeaconDownloadHandler;
    NicFillBeaconDescHandler                FillBeaconDescHandler;
    NicGetTxQueueHWIdxHandler               GetTxQueueHWIdxHandler;
    NicMappingTxQueueHandler                MappingTxQueueHandler;
    NicQueryTxConditionMatchHandler         QueryTxConditionMatchHandler;
    NicPrepareTxBufferDescriptorHandler     PrepareTXBDHandler;
    NicFillTxHwCtrlHandler                  FillTxHwCtrlHandler;
    NicSyncSWTXBDHostIdxToHWHandler         SyncSWTXBDHostIdxToHWHandler;
//    NicGetShortCutTxDescriptorHandler       GetShortCutTxDescHandler;
    NicReleaseShortCutTxDescriptorHandler   ReleaseShortCutTxDescHandler;
    NicGetShortCutTxBuffSizeHandler         GetShortCutTxBuffSizeHandler;
    NicSetShortCutTxBuffSizeHandler         SetShortCutTxBuffSizeHandler;
    NicCopyShortCutTxDescriptorHandler      CopyShortCutTxDescHandler;
    NicFillShortCutTxHwCtrlHandler          FillShortCutTxHwCtrlHandler;
    NicReleaseOnePacketHandler              ReleaseOnePacketHandler;
    NicGetTxRPTHandler                      GetTxRPTHandler;
    NicSetTxRPTHandler                      SetTxRPTHandler;
    NicSetCRC5ToRPTBufferHandler            SetCRC5ToRPTBufferHandler;    
    NicFillTxDescHandler                    FillTxDescHandler;
    NicFillShortCutTxDescHandler            FillShortCutTxDescHandler;
    NicFillHwShortCutTxDescHandler          FillHwShortCutTxDescHandler;    
    NicFillRsrvPageDescHandler              FillRsrvPageDescHandler;
    NicSetStatistic_ReportHandler           SetStatistic_ReportHandler;
    DumpTxBDescHandler	DumpTxBDescTestHandler;
    

    //3 Rx Related
    NicPrepareRXBDHandler                   PrepareRXBDHandler;
    NicQueryRxDescHandler                   QueryRxDescHandler;
    NicUpdateRXBDInfoHandler                UpdateRXBDInfoHandler;
    NicReadableRxBufferDescCountHandler     ReadableRxBufferDescCountHandler;
    NicUpdateRXBDHWIdxHandler               UpdateRXBDHWIdxHandler;
    NicUpdateRXBDHostIdxHandler             UpdateRXBDHostIdxHandler;
    DumpRxBDescHandler	DumpRxBDescTestHandler;
	
#endif // (HAL_DEV_BUS_TYPE & (HAL_RT_EMBEDDED_INTERFACE | HAL_RT_PCI_INTERFACE))

    //3 Firmware CMD IO related
    NicFillH2CCmdHandler                    FillH2CCmdHandler;
    NicUpdateHalRAMaskHandler               UpdateHalRAMaskHandler;
    NicUpdateHalMSRRPTHandler               UpdateHalMSRRPTHandler;
    NicSetAPOffloadHandler                  SetAPOffloadHandler;
    NicSetSAPPsHandler                      SetSAPPsHandler;
#ifdef AP_PS_Offlaod	    
    NicSetAPPSOffloadHandler                SetAPPSOffloadHandler;    
    NicAPPSOffloadMACIDPauseHandler         APPSOffloadMACIDPauseHandler;
#endif    
   	NicSetRsvdPageHandler			        SetRsvdPageHandler;
	NicGetRsvdPageLocHandler		        GetRsvdPageLocHandler;
	NicDownloadRsvdPageHandler		        DownloadRsvdPageHandler;
    NicC2HHandler                           C2HHandler;
    NicC2HPacketHandler                     C2HPacketHandler;    
    NicCheckHangHandler                     CheckHangHandler;
    NicInitDDMAHandler                      InitDDMAHandler;

#if     IS_RTL8192E_SERIES
    PVOID               PHalFunc8192E;
#endif

#if     IS_RTL8881A_SERIES
    PVOID               PHalFunc8881A;
#endif

#endif  //IS_RTL88XX_GENERATION    
} HAL_INTERFACE_COMMON, *PHAL_INTERFACE_COMMON;

#define HAL_INTERFACE           HAL_INTERFACE_COMMON
#define PHAL_INTERFACE          PHAL_INTERFACE_COMMON
#define GET_HAL_INTERFACE(__pAdapter)	((HAL_INTERFACE_COMMON *)((__pAdapter)->HalFunc))
#define GET_MACHALAPI_INTERFACE(__pAdapter)	(__pAdapter->pHalmac_api)

typedef struct _HAL_DATA_MV_
{
    u1Byte  test;
    
}HAL_DATA_MV, *PHAL_DATA_PMV;


typedef u4Byte RT_INT_REG, *PRT_INT_REG;


// Variable: AccessSwapCtrl
#define HAL_ACCESS_SWAP_IO      BIT0	/* Do bye-swap in access IO register */
#define HAL_ACCESS_SWAP_MEM     BIT1    /* Do byte-swap in access memory space */

#define MAX_MACDM_REG_NUM       30
//#define MAX_MACDM_THRS_NUM      13   // 3 state, 4 criteria * 3 Rssi = 12, add one EOF=>13
#define HALDEF_MACDM_TP_THRS_MAX_NUM    4 //MACDM_TP_THRS_MAX_NUM
#define HALDEF_RSSI_LVL_MAX_NUM         3 //RSSI_LVL_MAX_NUM
#define HALDEF_MACDM_TP_STATE_MAX_NUM   3 //MACDM_TP_STATE_MAX_NUM


// IO Format
typedef struct _IOREG_FORMAT_
{
    u4Byte  offset;
    u4Byte  value;
}IOREG_FORMAT, *PIOREG_FORMAT;

typedef struct _HAL_DATA_COMMON_
{
    u2Byte              HardwareType;
    BOOLEAN             bTestChip;      // 1: TestChip, 0:MP
	u1Byte				cutVersion;

    u1Byte              devIdx;

    //IO/MEM Swap
    u4Byte              AccessSwapCtrl;

    //Mapping driver variable
    HAL_DATA_MV         MappingVariable;

    //ISR
    //u4Byte              InterruptMask;
    //u4Byte              InterruptMaskExt;
    RT_INT_REG          IntArray[5];
    RT_INT_REG          IntArray_bak[2];
    RT_INT_REG			IntMask[5];
    RT_INT_REG			IntMask_RxINTBackup[2]; //Backup for Rx IMR Control

#if IS_RTL88XX_GENERATION

    //Firmware
    u1Byte              H2CBufPtr88XX;      //88XX range: 0~3
    BOOLEAN             bFWReady;
    PVOID               PFWHeader;

    //TRX DESC
    PVOID               PRxDescData88XX;
    PVOID               PRxDescStatus88XX;

    //DM
    u4Byte              MACDM_Mode_Sel;
    u4Byte              MACDM_State;
    u4Byte              MACDM_preRssiLvl;
    u4Byte              MACDM_stateThrs[HALDEF_MACDM_TP_THRS_MAX_NUM][HALDEF_RSSI_LVL_MAX_NUM];
    IOREG_FORMAT        MACDM_Table[HALDEF_MACDM_TP_STATE_MAX_NUM][HALDEF_RSSI_LVL_MAX_NUM][MAX_MACDM_REG_NUM];

#if (HAL_DEV_BUS_TYPE & (HAL_RT_EMBEDDED_INTERFACE | HAL_RT_PCI_INTERFACE))
    PVOID               PTxDMA88XX;
    PVOID               PRxDMA88XX;

#if CFG_HAL_TX_AMSDU
    PVOID               PTxDMAAMSDU88XX;
#endif

#if defined(WLAN_HAL_TXDESC_CHECK_ADDR_LEN)
#if IS_EXIST_RTL8881AEM
    u4Byte              cur_txbd;
#if 0
    u4Byte              cur_tx_desc_phy_addr;
    u4Byte              cur_tx_desc_len;
#endif
    u4Byte              cur_tx_psb_len;
#endif //IS_EXIST_RTL8881AEM
#endif // WLAN_HAL_TXDESC_CHECK_ADDR_LEN
    pu1Byte             desc_dma_buf;       //desc memory from common driver
    u4Byte              desc_dma_buf_len;   //desc memory length from common driver
#if CFG_HAL_WLAN_SUPPORT_H2C_PACKET
    pu1Byte             h2c_buf;
    u4Byte              h2c_buf_len;   
#endif //CFG_HAL_WLAN_SUPPORT_H2C_PACKET
#if CFG_HAL_TX_AMSDU
    RT_INT_REG          FtIntArray;
    RT_INT_REG			FtIntMask;
    pu1Byte             desc_dma_buf_amsdu;
    u4Byte              desc_dma_buf_len_amsdu;
#endif
    dma_addr_t          ring_dma_addr;	//rx_dma_addr_start.
    dma_addr_t          ring_buf_len;
    dma_addr_t          ring_virt_addr;
    dma_addr_t          alloc_dma_buf;
    dma_addr_t          txBD_dma_ring_addr[14]; //there are 14 queues in system, including BCN queue
    dma_addr_t          txDesc_dma_ring_addr[14]; //there are 14 queues in system, including BCN queue
#endif

#if IS_RTL8881A_SERIES
    PVOID               PHalData8881A;
#endif  //IS_RTL8881A_SERIES

#if IS_RTL8192E_SERIES
    PVOID               PHalData8192E;
#endif  //IS_RTL8192E_SERIES

#if IS_RTL8814A_SERIES
    u1Byte              crc5Valid[128];
    u1Byte              crc5groupValid[12];
#endif //IS_RTL8814A_SERIES
    MAC_VERSION         MacVersion;

#endif  //IS_RTL88XX_GENERATION

}HAL_DATA_COMMON, *PHAL_DATA_COMMON;

#define HAL_DATA_TYPE                   HAL_DATA_COMMON
#define PHAL_DATA_TYPE                  PHAL_DATA_COMMON
#define _GET_HAL_DATA(__pAdapter)	    ((HAL_DATA_TYPE *)((__pAdapter)->HalData))


#define HAL_HW_TYPE_ID_8723A				0x01
#define HAL_HW_TYPE_ID_8188E				0x02
#define HAL_HW_TYPE_ID_8881A				0x03
#define HAL_HW_TYPE_ID_8812A				0x04
#define HAL_HW_TYPE_ID_8723B				0x05
#define HAL_HW_TYPE_ID_8821A				0x06
#define HAL_HW_TYPE_ID_8192E				0x07
#define HAL_HW_TYPE_ID_8814A				0x08

#define HAL_HW_TYPE_ID_8821B				0x09
#define HAL_HW_TYPE_ID_8822B				0x0A
#define HAL_HW_TYPE_ID_8703B				0x0B

#define HAL_HW_TYPE_ID_8197F				0x0E



typedef enum _HARDWARE_TYPE{
	HARDWARE_TYPE_RTL8192SE,
	HARDWARE_TYPE_RTL8192SU,
	HARDWARE_TYPE_RTL8192CE,
	HARDWARE_TYPE_RTL8192CU,
	HARDWARE_TYPE_RTL8192DE,
	HARDWARE_TYPE_RTL8192DU,
	HARDWARE_TYPE_RTL8723AE,
	HARDWARE_TYPE_RTL8723AU,
	HARDWARE_TYPE_RTL8723AS,
	HARDWARE_TYPE_RTL8188EE,
	HARDWARE_TYPE_RTL8188EU,
	HARDWARE_TYPE_RTL8188ES,
	HARDWARE_TYPE_RTL8812E,
	HARDWARE_TYPE_RTL8821E,
	HARDWARE_TYPE_RTL8812AU,
	HARDWARE_TYPE_RTL8821U,
	HARDWARE_TYPE_RTL8881AEM,
	HARDWARE_TYPE_RTL8192EE,
    HARDWARE_TYPE_RTL8192EU,
    HARDWARE_TYPE_RTL8192ES,
    HARDWARE_TYPE_RTL8814AE,
    HARDWARE_TYPE_RTL8814AU,
    HARDWARE_TYPE_RTL8814AS,
    HARDWARE_TYPE_RTL8197FEM,
    HARDWARE_TYPE_RTL8822BE,
    HARDWARE_TYPE_RTL8822BU,
    HARDWARE_TYPE_RTL8822BS,    
	HARDWARE_TYPE_MAX,
}HARDWARE_TYPE;

#ifdef TESTCHIP_SUPPORT
#define IS_HAL_TEST_CHIP(_Adapter)              (_GET_HAL_DATA(_Adapter)->bTestChip==_TRUE)
#else
#define IS_HAL_TEST_CHIP(_Adapter)				0
#endif
#define IS_HAL_A_CUT(_Adapter)                  (GET_BIT_CHIP_VER(HAL_RTL_R32(REG_SYS_CFG1)) == 0x0)

//
// RTL MAC Version 
//
#define IS_HARDWARE_TYPE_MAC_V1(_Adapter)	    (IS_HARDWARE_TYPE_8881A(_Adapter)|| IS_HARDWARE_TYPE_8192E(_Adapter))
#define IS_HARDWARE_TYPE_MAC_V2(_Adapter)	    (IS_HARDWARE_TYPE_8814A(_Adapter)|| IS_HARDWARE_TYPE_8197F(_Adapter)|| IS_HARDWARE_TYPE_8822B(_Adapter))
#define IS_HARDWARE_MACHAL_SUPPORT(_Adapter)	(IS_HARDWARE_TYPE_8822B(_Adapter))


//
// RTL8192E Series
//
#if defined(CONFIG_WLAN_HAL_8192EE) && defined(CONFIG_SDIO_HCI)
#define IS_HARDWARE_TYPE_8192ES(_Adapter)	1
#define IS_HARDWARE_TYPE_8192EE(_Adapter)	0
#define IS_HARDWARE_TYPE_8192EU(_Adapter)	0
#else
#define IS_HARDWARE_TYPE_8192ES(_Adapter)	(_GET_HAL_DATA(_Adapter)->HardwareType==HARDWARE_TYPE_RTL8192ES)
#define IS_HARDWARE_TYPE_8192EE(_Adapter)	(_GET_HAL_DATA(_Adapter)->HardwareType==HARDWARE_TYPE_RTL8192EE)
#define IS_HARDWARE_TYPE_8192EU(_Adapter)	(_GET_HAL_DATA(_Adapter)->HardwareType==HARDWARE_TYPE_RTL8192EU)
#endif
#define IS_HARDWARE_TYPE_8192E(_Adapter)			\
    (IS_HARDWARE_TYPE_8192EE(_Adapter) || IS_HARDWARE_TYPE_8192EU(_Adapter) || IS_HARDWARE_TYPE_8192ES(_Adapter))

//
// RTL8881A Series
//
#define IS_HARDWARE_TYPE_8881A(_Adapter)   	(_GET_HAL_DATA(_Adapter)->HardwareType==HARDWARE_TYPE_RTL8881AEM)

//
// RTL8814A Series
//
#define IS_HARDWARE_TYPE_8814AS(_Adapter)	(_GET_HAL_DATA(_Adapter)->HardwareType==HARDWARE_TYPE_RTL8814AS)
#define IS_HARDWARE_TYPE_8814AE(_Adapter)	(_GET_HAL_DATA(_Adapter)->HardwareType==HARDWARE_TYPE_RTL8814AE)
#define IS_HARDWARE_TYPE_8814AU(_Adapter)	(_GET_HAL_DATA(_Adapter)->HardwareType==HARDWARE_TYPE_RTL8814AU)
#define IS_HARDWARE_TYPE_8814A(_Adapter)			\
    (IS_HARDWARE_TYPE_8814AE(_Adapter) || IS_HARDWARE_TYPE_8814AU(_Adapter) || IS_HARDWARE_TYPE_8814AS(_Adapter))

//
// RTL8197F Series
//    
#define IS_HARDWARE_TYPE_8197F(_Adapter)	(_GET_HAL_DATA(_Adapter)->HardwareType==HARDWARE_TYPE_RTL8197FEM)		

//
// RTL8822B Series
//    
#define IS_HARDWARE_TYPE_8822BE(_Adapter)	(_GET_HAL_DATA(_Adapter)->HardwareType==HARDWARE_TYPE_RTL8822BE)
#define IS_HARDWARE_TYPE_8822BU(_Adapter)	(_GET_HAL_DATA(_Adapter)->HardwareType==HARDWARE_TYPE_RTL8822BU)
#define IS_HARDWARE_TYPE_8822BS(_Adapter)	(_GET_HAL_DATA(_Adapter)->HardwareType==HARDWARE_TYPE_RTL8822BS)
#define IS_HARDWARE_TYPE_8822B(_Adapter)			\
    (IS_HARDWARE_TYPE_8822BE(_Adapter) || IS_HARDWARE_TYPE_8822BU(_Adapter) || IS_HARDWARE_TYPE_8822BS(_Adapter))	


#endif  //__HALDEF_H__
