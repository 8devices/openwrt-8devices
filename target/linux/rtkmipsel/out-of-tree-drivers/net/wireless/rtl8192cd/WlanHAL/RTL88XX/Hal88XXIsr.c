/*++
Copyright (c) Realtek Semiconductor Corp. All rights reserved.

Module Name:
	Hal88XXIsr.c

Abstract:
	Defined RTL88XX HAL common Function

Major Change History:
	When       Who               What
	---------- ---------------   -------------------------------
	2012-04-30 Filen            Create.
--*/

#if !defined(__ECOS) && !defined(CPTCFG_CFG80211_MODULE)
#include "HalPrecomp.h"
#else
#include "../HalPrecomp.h"
#endif

VOID
EnableIMR88XX(
    IN  HAL_PADAPTER    Adapter
)
{
    PHAL_DATA_TYPE              pHalData = _GET_HAL_DATA(Adapter);
    
    RT_TRACE_F(COMP_INIT, DBG_LOUD, ("\n"));

    HAL_RTL_W32(REG_HIMR0, pHalData->IntMask[0]);
    HAL_RTL_W32(REG_HIMR1, pHalData->IntMask[1]);

#if IS_RTL88XX_MAC_V2
    if ( _GET_HAL_DATA(Adapter)->MacVersion.is_MAC_v2) {
        HAL_RTL_W32(REG_HIMR2, pHalData->IntMask[2]);    
        HAL_RTL_W32(REG_HIMR3, pHalData->IntMask[3]);
    }
#endif //IS_RTL88XX_MAC_V2

#if CFG_HAL_SUPPORT_AXI_BUS_EXCEPTION
    if(IS_SUPPORT_AXI_EXCEPTION(Adapter)) {
        // write AXI exception interrupt IMR
        HAL_RTL_W32(REG_HSIMR, pHalData->IntMask[4]);  
    }
#endif // CFG_HAL_SUPPORT_AXI_BUS_EXCEPTION

#if 0//CFG_HAL_TX_AMSDU
    if ( IS_SUPPORT_TX_AMSDU(Adapter) ) { //Do not enable FW IMR if FW does not use, REG_FTIMR do not trigger interrupt to host
        HAL_RTL_W32(REG_FTIMR, HAL_RTL_R32(REG_FTIMR) | pHalData->FtIntMask);
    }
#endif


}


//
// Description:
//	Recognize the interrupt content by reading the interrupt register or content and masking interrupt mask (IMR)
//	if it is our NIC's interrupt. After recognizing, we may clear the all interrupts (ISR).
// Arguments:
//	[in] Adapter -
//		The adapter context.
//	[in] pContent -
//		Under PCI interface, this field is ignord.
//		Under USB interface, the content is the interrupt content pointer.
//		Under SDIO interface, this is the interrupt type which is Local interrupt or system interrupt.
//	[in] ContentLen -
//		The length in byte of pContent.
// Return:
//	If any interrupt matches the mask (IMR), return TRUE, and return FALSE otherwise.
//

HAL_IMEM
BOOLEAN
InterruptRecognized88XX(
    IN  HAL_PADAPTER        Adapter,
	IN	PVOID				pContent,
	IN	u4Byte				ContentLen
)
{
    PHAL_DATA_TYPE              pHalData = _GET_HAL_DATA(Adapter);
    u1Byte                      result;

	pHalData->IntArray_bak[0] = pHalData->IntArray[0];
	pHalData->IntArray_bak[1] = pHalData->IntArray[1];

    pHalData->IntArray[0] = HAL_RTL_R32(REG_HISR0);
    pHalData->IntArray[0] &= pHalData->IntMask[0];
    HAL_RTL_W32(REG_HISR0, pHalData->IntArray[0]);

    pHalData->IntArray[1] = HAL_RTL_R32(REG_HISR1);
    pHalData->IntArray[1] &= pHalData->IntMask[1];
    HAL_RTL_W32(REG_HISR1, pHalData->IntArray[1]);

    result = (pHalData->IntArray[0]!=0 || pHalData->IntArray[1]!=0);

#if IS_RTL88XX_MAC_V2
    if ( _GET_HAL_DATA(Adapter)->MacVersion.is_MAC_v2) {

    pHalData->IntArray[2] = HAL_RTL_R32(REG_HISR2);
    pHalData->IntArray[2] &= pHalData->IntMask[2];
    HAL_RTL_W32(REG_HISR2, pHalData->IntArray[2]);

    pHalData->IntArray[3] = HAL_RTL_R32(REG_HISR3);
    pHalData->IntArray[3] &= pHalData->IntMask[3];
    HAL_RTL_W32(REG_HISR3, pHalData->IntArray[3]);
   
    result = (result || (pHalData->IntArray[2]!=0 || pHalData->IntArray[3]!=0 ));

    }
#endif // #if IS_RTL88XX_MAC_V2

#if CFG_HAL_SUPPORT_AXI_BUS_EXCEPTION
    if(IS_SUPPORT_AXI_EXCEPTION(Adapter)) {
        pHalData->IntArray[4] = HAL_RTL_R32(REG_HSISR);
        pHalData->IntArray[4] &= pHalData->IntMask[4];
        HAL_RTL_W32(REG_HSISR, pHalData->IntArray[4]);
    }
    
    result = (result || (pHalData->IntArray[4]!=0));    
    
#endif // CFG_HAL_SUPPORT_AXI_BUS_EXCEPTION

#if CFG_HAL_TX_AMSDU
    if ( IS_SUPPORT_TX_AMSDU(Adapter) ) {
        pHalData->FtIntArray = HAL_RTL_R32(REG_FTISR);
        pHalData->FtIntArray &= pHalData->FtIntMask;
        HAL_RTL_W32(REG_FTISR, pHalData->FtIntArray);

        result = (result || (pHalData->FtIntArray!=0));
    }
#endif

    return result;
}

//
// Description:
//	Check the interrupt content (read from previous process) in HAL.
// Arguments:
//	[in] pAdapter -
//		The adapter context pointer.
//	[in] intType -
//		The HAL interrupt type for querying.
// Return:
//	If the corresponding interrupt content (bit) is toggled, return TRUE.
//	If the input interrupt type isn't recognized or this corresponding
//	hal interupt isn't toggled, return FALSE.
// Note:
//	We don't perform I/O here to read interrupt such as ISR here, so the
//	interrupt content shall be read before this handler.
//
HAL_IMEM
BOOLEAN
GetInterrupt88XX(
	IN	HAL_PADAPTER	Adapter,
	IN	HAL_INT_TYPE	intType
	)
{
	HAL_DATA_TYPE	*pHalData   = _GET_HAL_DATA(Adapter);
	BOOLEAN			bResult     = FALSE;

	switch(intType)
	{
	default:
		// Unknown interrupt type, no need to alarm because this IC may not
		// support this interrupt.
		RT_TRACE_F(COMP_SYSTEM, DBG_WARNING, ("Unkown intType: %d!\n", intType));
		break;

	case HAL_INT_TYPE_ANY:
		bResult = (pHalData->IntArray[0] || pHalData->IntArray[1]) ? TRUE : FALSE;
		break;

	//4 // ========== DWORD 0 ==========
	case HAL_INT_TYPE_BCNDERR0:
		bResult = (pHalData->IntArray[0] & BIT_BCNDERR0) ? TRUE : FALSE;
		break;

	case HAL_INT_TYPE_TBDOK:
		bResult = (pHalData->IntArray[0] & BIT_TXBCN0OK) ? TRUE : FALSE;
		break;
		
	case HAL_INT_TYPE_TBDER:
		bResult = (pHalData->IntArray[0] & BIT_TXBCN0ERR) ? TRUE : FALSE;
		break;
		
	case HAL_INT_TYPE_BcnInt:
		bResult = (pHalData->IntArray[0] & BIT_BCNDMAINT0) ? TRUE : FALSE;
		break;
    case HAL_INT_TYPE_GTIMER4:
        bResult = (pHalData->IntArray[0] & BIT_GTINT4) ? TRUE : FALSE;
		break;
		
	case HAL_INT_TYPE_PSTIMEOUT:
		bResult = (pHalData->IntArray[0] & BIT_PSTIMEOUT) ? TRUE : FALSE;
		break;

	case HAL_INT_TYPE_PSTIMEOUT1:
		bResult = (pHalData->IntArray[0] & BIT_PSTIMEOUT1) ? TRUE : FALSE;
		break;

	case HAL_INT_TYPE_PSTIMEOUT2:
		bResult = (pHalData->IntArray[0] & BIT_PSTIMEOUT2) ? TRUE : FALSE;
		break; 
		
	case HAL_INT_TYPE_C2HCMD:
		bResult = (pHalData->IntArray[0] & BIT_C2HCMD) ? TRUE : FALSE;
		break;	
		
	case HAL_INT_TYPE_VIDOK:
		bResult = (pHalData->IntArray[0] & BIT_VIDOK) ? TRUE : FALSE;
		break;
		
	case HAL_INT_TYPE_VODOK:
		bResult = (pHalData->IntArray[0] & BIT_VODOK) ? TRUE : FALSE;
		break;
		
	case HAL_INT_TYPE_BEDOK:
		bResult = (pHalData->IntArray[0] & BIT_BEDOK) ? TRUE : FALSE;
		break;
		
	case HAL_INT_TYPE_BKDOK:
		bResult = (pHalData->IntArray[0] & BIT_BKDOK) ? TRUE : FALSE;
		break;

	case HAL_INT_TYPE_MGNTDOK:
		bResult = (pHalData->IntArray[0] & BIT_MGTDOK) ? TRUE : FALSE;
		break;

	case HAL_INT_TYPE_HIGHDOK:
		bResult = (pHalData->IntArray[0] & BIT_HIGHDOK) ? TRUE : FALSE;
		break;

    #if 0   //Filen: removed
	case HAL_INT_TYPE_BDOK:
		bResult = (pHalData->IntArray[0] & IMR_BCNDOK0_88E) ? TRUE : FALSE;
		break;
    #endif
		
	case HAL_INT_TYPE_CPWM:
		bResult = (pHalData->IntArray[0] & BIT_CPWM) ? TRUE : FALSE;
		break;

	case HAL_INT_TYPE_TSF_BIT32_TOGGLE:
		bResult = (pHalData->IntArray[0] & BIT_TSF_BIT32_TOGGLE) ? TRUE : FALSE;
		break;

    case HAL_INT_TYPE_RX_OK:
		bResult = (pHalData->IntArray[0] & BIT_RXOK) ? TRUE : FALSE;
        break;
        
    case HAL_INT_TYPE_RDU:
		bResult = (pHalData->IntArray[0] & BIT_RDU) ? TRUE : FALSE;
        break;        
    case HAL_INT_TYPE_CPWM2:
            bResult = (pHalData->IntArray[0] & BIT_CPWM2) ? TRUE : FALSE;
            break;

    case HAL_INT_TYPE_CTWEND:
#if IS_RTL88XX_MAC_V1
        if ( _GET_HAL_DATA(Adapter)->MacVersion.is_MAC_v1) { 
    		bResult = (pHalData->IntArray[0] & BIT_CTWEND) ? TRUE : FALSE;
        }
#endif //(IS_RTL8192E_SERIES || IS_RTL8881A_SERIES)
        
#if IS_RTL88XX_MAC_V2
        if ( _GET_HAL_DATA(Adapter)->MacVersion.is_MAC_v2)  
        {
            //this interrupt is removed at 8814A
            bResult = FALSE;
        }
#endif // IS_RTL8192E_SERIES
        break;

	//4 // ========== DWORD 1 ==========
	case HAL_INT_TYPE_RXFOVW:
		bResult = (pHalData->IntArray[1] & BIT_FOVW) ? TRUE : FALSE;
		break;

    case HAL_INT_TYPE_TXFOVW:
        bResult = (pHalData->IntArray[1] & BIT_TXFOVW) ? TRUE : FALSE;
		break;

    case HAL_INT_TYPE_RXERR:
        bResult = (pHalData->IntArray[1] & BIT_RXERR_INT) ? TRUE : FALSE;
		break;        
        
    case HAL_INT_TYPE_TXERR:
        bResult = (pHalData->IntArray[1] & BIT_TXERR_INT) ? TRUE : FALSE;
		break;        

    case HAL_INT_TYPE_BcnInt_MBSSID:
        bResult = ((pHalData->IntArray[1] & (BIT_BCNDMAINT1|BIT_BCNDMAINT2|BIT_BCNDMAINT3|BIT_BCNDMAINT4|
                                    BIT_BCNDMAINT5|BIT_BCNDMAINT6|BIT_BCNDMAINT7)) ||
                    (pHalData->IntArray[0] & BIT_BCNDMAINT0)
                    ) ? TRUE : FALSE;
        break;

    case HAL_INT_TYPE_BcnInt1:
		bResult = (pHalData->IntArray[1] & BIT_BCNDMAINT1) ? TRUE : FALSE;
        break;

    case HAL_INT_TYPE_BcnInt2:
		bResult = (pHalData->IntArray[1] & BIT_BCNDMAINT2) ? TRUE : FALSE;
        break;

    case HAL_INT_TYPE_BcnInt3:
		bResult = (pHalData->IntArray[1] & BIT_BCNDMAINT3) ? TRUE : FALSE;
        break;

    case HAL_INT_TYPE_BcnInt4:
		bResult = (pHalData->IntArray[1] & BIT_BCNDMAINT4) ? TRUE : FALSE;
        break;

    case HAL_INT_TYPE_BcnInt5:
		bResult = (pHalData->IntArray[1] & BIT_BCNDMAINT5) ? TRUE : FALSE;
        break;

    case HAL_INT_TYPE_BcnInt6:
		bResult = (pHalData->IntArray[1] & BIT_BCNDMAINT6) ? TRUE : FALSE;
        break;

    case HAL_INT_TYPE_BcnInt7:
		bResult = (pHalData->IntArray[1] & BIT_BCNDMAINT7) ? TRUE : FALSE;
        break;

#if CFG_HAL_SUPPORT_EACH_VAP_INT
    if ( IS_SUPPORT_EACH_VAP_INT(Adapter) ) {

    case HAL_INT_TYPE_TXBCNOK_MBSSID:
        bResult = ((pHalData->IntArray[2] & (BIT_TXBCN1OK|BIT_TXBCN2OK|BIT_TXBCN3OK|BIT_TXBCN4OK|
                                    BIT_TXBCN5OK|BIT_TXBCN6OK|BIT_TXBCN7OK)) ||
                    (pHalData->IntArray[0] & BIT_TXBCN0OK)
                    ) ? TRUE : FALSE;
        break;
    case HAL_INT_TYPE_TXBCN1OK:
        bResult = (pHalData->IntArray[2] & BIT_TXBCN1OK) ? TRUE : FALSE;
        break;
    case HAL_INT_TYPE_TXBCN2OK:
        bResult = (pHalData->IntArray[2] & BIT_TXBCN2OK) ? TRUE : FALSE;
        break;
    case HAL_INT_TYPE_TXBCN3OK:
        bResult = (pHalData->IntArray[2] & BIT_TXBCN3OK) ? TRUE : FALSE;
        break;
    case HAL_INT_TYPE_TXBCN4OK:
        bResult = (pHalData->IntArray[2] & BIT_TXBCN4OK) ? TRUE : FALSE;
        break;
    case HAL_INT_TYPE_TXBCN5OK:
        bResult = (pHalData->IntArray[2] & BIT_TXBCN5OK) ? TRUE : FALSE;
        break;    
    case HAL_INT_TYPE_TXBCN6OK:
        bResult = (pHalData->IntArray[2] & BIT_TXBCN6OK) ? TRUE : FALSE;
        break;    
    case HAL_INT_TYPE_TXBCN7OK:
        bResult = (pHalData->IntArray[2] & BIT_TXBCN7OK) ? TRUE : FALSE;
        break;    

    case HAL_INT_TYPE_TXBCNERR_MBSSID:
        bResult = ((pHalData->IntArray[2] & (BIT_TXBCN1ERR|BIT_TXBCN2ERR|BIT_TXBCN3ERR|BIT_TXBCN4ERR|
                                   BIT_TXBCN5ERR|BIT_TXBCN6ERR|BIT_TXBCN7ERR)) ||
                   (pHalData->IntArray[0] & BIT_TXBCN0ERR)
                   ) ? TRUE : FALSE;
        break;
    case HAL_INT_TYPE_TXBCN1ERR:
        bResult = (pHalData->IntArray[2] & BIT_TXBCN1ERR) ? TRUE : FALSE;
        break;
    case HAL_INT_TYPE_TXBCN2ERR:
        bResult = (pHalData->IntArray[2] & BIT_TXBCN2ERR) ? TRUE : FALSE;
        break;
    case HAL_INT_TYPE_TXBCN3ERR:
        bResult = (pHalData->IntArray[2] & BIT_TXBCN3ERR) ? TRUE : FALSE;
        break;
    case HAL_INT_TYPE_TXBCN4ERR:
        bResult = (pHalData->IntArray[2] & BIT_TXBCN4ERR) ? TRUE : FALSE;
        break;
    case HAL_INT_TYPE_TXBCN5ERR:
        bResult = (pHalData->IntArray[2] & BIT_TXBCN5ERR) ? TRUE : FALSE;
        break;    
    case HAL_INT_TYPE_TXBCN6ERR: 
        bResult = (pHalData->IntArray[2] & BIT_TXBCN6ERR) ? TRUE : FALSE;
        break;    
    case HAL_INT_TYPE_TXBCN7ERR:
        bResult = (pHalData->IntArray[2] & BIT_TXBCN7ERR) ? TRUE : FALSE;
        break;            
    }
#endif // #if CFG_HAL_SUPPORT_EACH_VAP_INT

#if CFG_HAL_HW_DETEC_POWER_STATE
    if ( IS_SUPPORT_HW_DETEC_POWER_STATE(Adapter)) {

    case HAL_INT_TYPE_PwrInt0:
        bResult = (pHalData->IntArray[3] & BIT_PWR_INT_31to0) ? TRUE : FALSE;
        break;
    case HAL_INT_TYPE_PwrInt1:
        bResult = (pHalData->IntArray[3] & BIT_PWR_INT_63to32) ? TRUE : FALSE;
        break;
    case HAL_INT_TYPE_PwrInt2:
        bResult = (pHalData->IntArray[3] & BIT_PWR_INT_95to64) ? TRUE : FALSE;
        break;
    case HAL_INT_TYPE_PwrInt3:
        bResult = (pHalData->IntArray[3] & BIT_PWR_INT_126to96) ? TRUE : FALSE;
        break;
    case HAL_INT_TYPE_PwrInt4:
        bResult = (pHalData->IntArray[3] & BIT_PWR_INT_127) ? TRUE : FALSE;
        break;    
    }
#endif //#if CFG_HAL_HW_DETEC_POWER_STATE


#if CFG_HAL_SUPPORT_AXI_BUS_EXCEPTION
    if(IS_SUPPORT_AXI_EXCEPTION(Adapter)) {
        case HAL_INT_TYPE_AXI_EXCEPTION:
        bResult = (pHalData->IntArray[4] & BIT_AXI_EXCEPT_HINT) ? TRUE : FALSE;
        break;
    }
#endif // CFG_HAL_SUPPORT_AXI_BUS_EXCEPTION

#if CFG_HAL_TX_AMSDU
    if ( IS_SUPPORT_TX_AMSDU(Adapter) ) {

    case HAL_INT_TYPE_FS_TIMEOUT0:
        bResult = (pHalData->FtIntArray & BIT_FS_PS_TIMEOUT0_EN) ? TRUE : FALSE;
        break;
    }
#endif

	}

	return bResult;
}


// TODO: Pedro, we can set several IMR combination for different scenario. Ex: 1) AP, 2) Client, 3) ....
// TODO: this can avoid to check non-necessary interrupt in __wlan_interrupt(..)...
VOID
AddInterruptMask88XX(
	IN	HAL_PADAPTER	Adapter,
	IN	HAL_INT_TYPE	intType
	)
{
	HAL_DATA_TYPE	*pHalData   = _GET_HAL_DATA(Adapter);

	switch(intType)
	{
    	default:
    		// Unknown interrupt type, no need to alarm because this IC may not
    		// support this interrupt.
    		RT_TRACE_F(COMP_SYSTEM, DBG_WARNING, ("Unkown intType: %d!\n", intType));
    		break;

    	case HAL_INT_TYPE_ANY:
            pHalData->IntMask[0] = 0xFFFFFFFF;
            pHalData->IntMask[1] = 0xFFFFFFFF;
    		break;

    	//4 // ========== DWORD 0 ==========
    	case HAL_INT_TYPE_BCNDERR0:
            pHalData->IntMask[0] |= BIT_BCNDERR0;
    		break;
            
    	case HAL_INT_TYPE_TBDOK:
    		pHalData->IntMask[0] |= BIT_TXBCN0OK;
    		break;
    		
    	case HAL_INT_TYPE_TBDER:
            pHalData->IntMask[0] |= BIT_TXBCN0ERR;
    		break;
    		
        case HAL_INT_TYPE_GTIMER4:
            pHalData->IntMask[0] |= BIT_GTINT4;
            break;
    	case HAL_INT_TYPE_BcnInt:
    		pHalData->IntMask[0] |= BIT_BCNDMAINT0;
    		break;
    		
    	case HAL_INT_TYPE_PSTIMEOUT:
            pHalData->IntMask[0] |= BIT_PSTIMEOUT;
    		break;

    	case HAL_INT_TYPE_PSTIMEOUT1:
    		pHalData->IntMask[0] |= BIT_PSTIMEOUT1;
    		break;

    	case HAL_INT_TYPE_PSTIMEOUT2:
    		pHalData->IntMask[0] |= BIT_PSTIMEOUT2;
    		break; 
    		
    	case HAL_INT_TYPE_C2HCMD:
    		pHalData->IntMask[0] |= BIT_C2HCMD;
    		break;	
    		
    	case HAL_INT_TYPE_VIDOK:
    		pHalData->IntMask[0] |= BIT_VIDOK;
    		break;
    		
    	case HAL_INT_TYPE_VODOK:
    		pHalData->IntMask[0] |= BIT_VODOK;
    		break;
    		
    	case HAL_INT_TYPE_BEDOK:
    		pHalData->IntMask[0] |= BIT_BEDOK;
    		break;
    		
    	case HAL_INT_TYPE_BKDOK:
    		pHalData->IntMask[0] |= BIT_BKDOK;
    		break;

    	case HAL_INT_TYPE_MGNTDOK:
    		pHalData->IntMask[0] |= BIT_MGTDOK;
    		break;

    	case HAL_INT_TYPE_HIGHDOK:
    		pHalData->IntMask[0] |= BIT_HIGHDOK;
    		break;

        #if 0   //Filen: removed
    	case HAL_INT_TYPE_BDOK:
            pHalData->IntMask[0] |= IMR_BCNDOK0_88E;
    		break;
        #endif
    		
    	case HAL_INT_TYPE_CPWM:
    		pHalData->IntMask[0] |= BIT_CPWM;
    		break;

    	case HAL_INT_TYPE_TSF_BIT32_TOGGLE:
    		pHalData->IntMask[0] |= BIT_TSF_BIT32_TOGGLE;
    		break;

        case HAL_INT_TYPE_RX_OK:
    		pHalData->IntMask[0] |= BIT_RXOK;
            break;

        case HAL_INT_TYPE_RDU:
    		pHalData->IntMask[0] |= BIT_RDU;
            break;
        case HAL_INT_TYPE_CPWM2:
            pHalData->IntMask[0] |= BIT_CPWM2;
            break;

    	//4 // ========== DWORD 1 ==========
    	case HAL_INT_TYPE_RXFOVW:
    		pHalData->IntMask[1] |= BIT_FOVW;
    		break;

        case HAL_INT_TYPE_TXFOVW:
            pHalData->IntMask[1] |= BIT_TXFOVW;
            break;

        case HAL_INT_TYPE_RXERR:
            pHalData->IntMask[1] |= BIT_RXERR_INT;
            break;
            
        case HAL_INT_TYPE_TXERR:
            pHalData->IntMask[1] |= BIT_TXERR_INT;
            break;

#if CFG_HAL_HW_DETEC_POWER_STATE
    if ( IS_SUPPORT_HW_DETEC_POWER_STATE(Adapter)) {
    
        case HAL_INT_TYPE_PwrInt0:
            pHalData->IntMask[3] |= BIT_PWR_INT_31to0;
            break;
        case HAL_INT_TYPE_PwrInt1:
            pHalData->IntMask[3] |= BIT_PWR_INT_63to32;
            break;
        case HAL_INT_TYPE_PwrInt2:
            pHalData->IntMask[3] |= BIT_PWR_INT_95to64;
            break;
        case HAL_INT_TYPE_PwrInt3:
            pHalData->IntMask[3] |= BIT_PWR_INT_126to96;            
            break;
        case HAL_INT_TYPE_PwrInt4:
            pHalData->IntMask[3] |= BIT_PWR_INT_127;            
            break;    
        }
#endif // #if CFG_HAL_HW_DETEC_POWER_STATE

#if CFG_HAL_SUPPORT_AXI_BUS_EXCEPTION
    if(IS_SUPPORT_AXI_EXCEPTION(Adapter)) {
        case HAL_INT_TYPE_AXI_EXCEPTION:
            pHalData->IntMask[4] |= BIT_AXI_EXCEPT_HINT;
        break;
    }
#endif // CFG_HAL_SUPPORT_AXI_BUS_EXCEPTION

#if CFG_HAL_TX_AMSDU
        if ( IS_SUPPORT_TX_AMSDU(Adapter) ) {

        case HAL_INT_TYPE_FS_TIMEOUT0:
            pHalData->FtIntArray |= BIT_FS_PS_TIMEOUT0_EN;
            break;
        }
#endif
	}
}


VOID
RemoveInterruptMask88XX(
	IN	HAL_PADAPTER	Adapter,
	IN	HAL_INT_TYPE	intType
	)
{
	HAL_DATA_TYPE	*pHalData   = _GET_HAL_DATA(Adapter);

	switch(intType)
	{
    	default:
    		// Unknown interrupt type, no need to alarm because this IC may not
    		// support this interrupt.
    		RT_TRACE_F(COMP_SYSTEM, DBG_WARNING, ("Unkown intType: %d!\n", intType));
    		break;

    	case HAL_INT_TYPE_ANY:
            pHalData->IntMask[0] &= ~0xFFFFFFFF;
            pHalData->IntMask[1] &= ~0xFFFFFFFF;
    		break;

    	//4 // ========== DWORD 0 ==========
    	case HAL_INT_TYPE_BCNDERR0:
            pHalData->IntMask[0] &= ~BIT_BCNDERR0;
    		break;
            
    	case HAL_INT_TYPE_TBDOK:
    		pHalData->IntMask[0] &= ~BIT_TXBCN0OK;
    		break;
    		
    	case HAL_INT_TYPE_TBDER:
            pHalData->IntMask[0] &= ~BIT_TXBCN0ERR;
    		break;
    		
        case HAL_INT_TYPE_GTIMER4:
            pHalData->IntMask[0] &= ~BIT_GTINT4;
            break;
    	case HAL_INT_TYPE_BcnInt:
    		pHalData->IntMask[0] &= ~BIT_BCNDMAINT0;
    		break;
    		
    	case HAL_INT_TYPE_PSTIMEOUT:
            pHalData->IntMask[0] &= ~BIT_PSTIMEOUT;
    		break;

    	case HAL_INT_TYPE_PSTIMEOUT1:
    		pHalData->IntMask[0] &= ~BIT_PSTIMEOUT1;
    		break;

    	case HAL_INT_TYPE_PSTIMEOUT2:
    		pHalData->IntMask[0] &= ~BIT_PSTIMEOUT2;
    		break; 
    		
    	case HAL_INT_TYPE_C2HCMD:
    		pHalData->IntMask[0] &= ~BIT_C2HCMD;
    		break;	
    		
    	case HAL_INT_TYPE_VIDOK:
    		pHalData->IntMask[0] &= ~BIT_VIDOK;
    		break;
    		
    	case HAL_INT_TYPE_VODOK:
    		pHalData->IntMask[0] &= ~BIT_VODOK;
    		break;
    		
    	case HAL_INT_TYPE_BEDOK:
    		pHalData->IntMask[0] &= ~BIT_BEDOK;
    		break;
    		
    	case HAL_INT_TYPE_BKDOK:
    		pHalData->IntMask[0] &= ~BIT_BKDOK;
    		break;

    	case HAL_INT_TYPE_MGNTDOK:
    		pHalData->IntMask[0] &= ~BIT_MGTDOK;
    		break;

    	case HAL_INT_TYPE_HIGHDOK:
    		pHalData->IntMask[0] &= ~BIT_HIGHDOK;
    		break;

        #if 0   //Filen: removed
    	case HAL_INT_TYPE_BDOK:
            pHalData->IntMask[0] &= ~IMR_BCNDOK0_88E;
    		break;
        #endif
    		
    	case HAL_INT_TYPE_CPWM:
    		pHalData->IntMask[0] &= ~BIT_CPWM;
    		break;

    	case HAL_INT_TYPE_TSF_BIT32_TOGGLE:
    		pHalData->IntMask[0] &= ~BIT_TSF_BIT32_TOGGLE;
    		break;

        case HAL_INT_TYPE_RX_OK:
    		pHalData->IntMask[0] &= ~BIT_RXOK;
            break;

        case HAL_INT_TYPE_RDU:
    		pHalData->IntMask[0] &= ~BIT_RDU;
            break;            
        case HAL_INT_TYPE_CPWM2:
            pHalData->IntMask[0] &= ~BIT_CPWM2;
            break;

    	//4 // ========== DWORD 1 ==========
    	case HAL_INT_TYPE_RXFOVW:
    		pHalData->IntMask[1] &= ~BIT_FOVW;
    		break;

        case HAL_INT_TYPE_TXFOVW:
            pHalData->IntMask[1] &= ~BIT_TXFOVW;
            break;

        case HAL_INT_TYPE_RXERR:
            pHalData->IntMask[1] &= ~BIT_RXERR_INT;
            break;
            
        case HAL_INT_TYPE_TXERR:
            pHalData->IntMask[1] &= ~BIT_TXERR_INT;
            break;

#if CFG_HAL_HW_DETEC_POWER_STATE
    if ( IS_SUPPORT_HW_DETEC_POWER_STATE(Adapter)) {
    
        case HAL_INT_TYPE_PwrInt0:
            pHalData->IntMask[3] &= ~BIT_PWR_INT_31to0;
            break;
        case HAL_INT_TYPE_PwrInt1:
            pHalData->IntMask[3] &= ~BIT_PWR_INT_63to32;
            break;
        case HAL_INT_TYPE_PwrInt2:
            pHalData->IntMask[3] &= ~BIT_PWR_INT_95to64;
            break;
        case HAL_INT_TYPE_PwrInt3:
            pHalData->IntMask[3] &= ~BIT_PWR_INT_126to96;            
            break;
        case HAL_INT_TYPE_PwrInt4:
            pHalData->IntMask[3] &= ~BIT_PWR_INT_127;            
            break;    
        }
#endif // #if CFG_HAL_HW_DETEC_POWER_STATE

#if CFG_HAL_SUPPORT_AXI_BUS_EXCEPTION
    if(IS_SUPPORT_AXI_EXCEPTION(Adapter)) {
        case HAL_INT_TYPE_AXI_EXCEPTION:
            pHalData->IntMask[4] &= ~BIT_AXI_EXCEPT_HINT;
        break;
    }
#endif // CFG_HAL_SUPPORT_AXI_BUS_EXCEPTION

#if CFG_HAL_TX_AMSDU
        if ( IS_SUPPORT_TX_AMSDU(Adapter) ) {


        case HAL_INT_TYPE_FS_TIMEOUT0:
            pHalData->FtIntArray &= ~BIT_FS_PS_TIMEOUT0_EN;
            break;
        }
#endif

	}
}


HAL_IMEM
VOID
DisableRxRelatedInterrupt88XX(
	IN	HAL_PADAPTER	Adapter
    )
{
    PHAL_DATA_TYPE      pHalData = _GET_HAL_DATA(Adapter);
    HAL_PADAPTER        priv     = Adapter;
    ULONG               flags;

#if 0
    HAL_SAVE_INT_AND_CLI(flags);

    pHalData->IntMask_RxINTBackup[0] = pHalData->IntMask[0];
    pHalData->IntMask_RxINTBackup[1] = pHalData->IntMask[1];

    pHalData->IntMask[0] &= ~BIT_RXOK;
    pHalData->IntMask[1] &= ~BIT_FOVW;

    HAL_RESTORE_INT(flags);

    HAL_RTL_W32(REG_HIMR0, pHalData->IntMask[0]);
    HAL_RTL_W32(REG_HIMR1, pHalData->IntMask[1]);
#else
    HAL_RTL_W32(REG_HIMR0, pHalData->IntMask[0] & ~ (BIT_RXOK | BIT_RDU));
    HAL_RTL_W32(REG_HIMR1, pHalData->IntMask[1] & ~BIT_FOVW);
#endif

}

HAL_IMEM
VOID
EnableRxRelatedInterrupt88XX(
	IN	HAL_PADAPTER	Adapter
    )
{
    PHAL_DATA_TYPE      pHalData    = _GET_HAL_DATA(Adapter);
    HAL_PADAPTER        priv        = Adapter;
    ULONG               flags;

#if 0
    HAL_SAVE_INT_AND_CLI(flags);

    pHalData->IntMask[0] = pHalData->IntMask_RxINTBackup[0];
    pHalData->IntMask[1] = pHalData->IntMask_RxINTBackup[1];

    HAL_RESTORE_INT(flags);
#endif
    HAL_RTL_W32(REG_HIMR0, pHalData->IntMask[0]);
    HAL_RTL_W32(REG_HIMR1, pHalData->IntMask[1]);

}


