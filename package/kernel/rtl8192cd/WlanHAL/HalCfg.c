/*++
Copyright (c) Realtek Semiconductor Corp. All rights reserved.

Module Name:
	HalCfg.c
	
Abstract:
	Defined Mapping Function for each Driver 
	    
Major Change History:
	When       Who               What
	---------- ---------------   -------------------------------
	2012-04-11 Filen            Create.	
--*/

#include "HalPrecomp.h"

#if     IS_RTL88XX_GENERATION
void
MappingVariable88XX(
    IN  HAL_PADAPTER    Adapter
)
{
    //PHAL_DATA_TYPE      pHalData = _GET_HAL_DATA(Adapter);

    //pHalData->
    

}

HAL_IMEM
u4Byte  //HCI_TX_DMA_QUEUE_88XX
MappingTxQueue88XX(
    IN  HAL_PADAPTER    Adapter,
    IN  u4Byte          TxQNum      //enum _TX_QUEUE_
)
{
    int mapping[] = {
        HCI_TX_DMA_QUEUE_MGT,
        HCI_TX_DMA_QUEUE_BK,
        HCI_TX_DMA_QUEUE_BE,
        HCI_TX_DMA_QUEUE_VI,
        HCI_TX_DMA_QUEUE_VO,       
        HCI_TX_DMA_QUEUE_HI0,
        HCI_TX_DMA_QUEUE_HI1,
        HCI_TX_DMA_QUEUE_HI2,
        HCI_TX_DMA_QUEUE_HI3,
        HCI_TX_DMA_QUEUE_HI4,
        HCI_TX_DMA_QUEUE_HI5,
        HCI_TX_DMA_QUEUE_HI6,
        HCI_TX_DMA_QUEUE_HI7,
        HCI_TX_DMA_QUEUE_CMD,        
        HCI_TX_DMA_QUEUE_BCN};
      

    if ( HCI_TX_DMA_QUEUE_MAX_NUM <= TxQNum ) {
        RT_TRACE_F(COMP_SEND, DBG_WARNING, ("Unknown Queue Mapping\n"));        
        return HCI_TX_DMA_QUEUE_BE;
    }
    else {
        return mapping[TxQNum];
    }
}











#endif  //IS_RTL88XX_GENERATION

