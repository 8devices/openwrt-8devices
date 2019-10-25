
/*++
Copyright (c) Realtek Semiconductor Corp. All rights reserved.

Module Name:
	HalMacAPI.c
	
Abstract:
	Define MAC function support 
	for Driver
	    
Major Change History:
	When       Who               What
	---------- ---------------   -------------------------------
	2015-07-07 Eric            Create.	
--*/

#include "HalPrecomp.h"

#ifdef CONFIG_WLAN_MACHAL_API
void associate_halMac_API(struct rtl8192cd_priv *priv)
{

    priv->pshare->use_macHalAPI = 1;
    priv->pHalmac_platform_api = (PHALMAC_PLATFORM_API)kmalloc(sizeof(HALMAC_PLATFORM_API), GFP_ATOMIC);
    //priv->pHalmac_adapter      = (PHALMAC_ADAPTER)kmalloc(sizeof(HALMAC_ADAPTER), GFP_ATOMIC);
    //priv->pHalmac_api          = (PHALMAC_API)kmalloc(sizeof(HALMAC_API), GFP_ATOMIC);   

    PHALMAC_PLATFORM_API              pMacHalFunc = priv->pHalmac_platform_api;


    //
    //Initialization Related
    //
    pMacHalFunc->SDIO_CMD52_READ            = MacHalGeneralDummy;
    pMacHalFunc->SDIO_CMD53_READ_8          = MacHalGeneralDummy;
    pMacHalFunc->SDIO_CMD53_READ_16         = MacHalGeneralDummy;
    pMacHalFunc->SDIO_CMD53_READ_32         = MacHalGeneralDummy;
    pMacHalFunc->SDIO_CMD52_WRITE           = MacHalGeneralDummy;
    pMacHalFunc->SDIO_CMD53_WRITE_8         = MacHalGeneralDummy;
    pMacHalFunc->SDIO_CMD53_WRITE_16        = MacHalGeneralDummy;
    pMacHalFunc->SDIO_CMD53_WRITE_32        = MacHalGeneralDummy;
    pMacHalFunc->REG_READ_8                 = HALAPI_R8;
    pMacHalFunc->REG_READ_16                = HALAPI_R16;
    pMacHalFunc->REG_READ_32                = HALAPI_R32;
    pMacHalFunc->REG_WRITE_8                = HALAPI_W8;
    pMacHalFunc->REG_WRITE_16               = HALAPI_W16;
    pMacHalFunc->REG_WRITE_32               = HALAPI_W32;
    pMacHalFunc->SEND_RSVD_PAGE             = HAL_SEND_RSVD_PAGE;
#ifdef WLAN_SUPPORT_H2C_PACKET    
    pMacHalFunc->SEND_H2C_PKT               = HAL_SEND_H2C_PKT;
#endif
    pMacHalFunc->RTL_MALLOC                 = HALAPI_MALLOC;
    pMacHalFunc->RTL_FREE                   = HALAPI_FREE;
    pMacHalFunc->RTL_MEMCPY                 = HALAPI_MEMCPY;
    pMacHalFunc->RTL_MEMSET                 = HALAPI_MEMSET;
    pMacHalFunc->RTL_DELAY_US               = HALAPI_DELAY_US;
//    pMacHalFunc->SIGNAL_INIT                = MacHalGeneralDummy;
//    pMacHalFunc->SIGNAL_DEINIT              = MacHalGeneralDummy;
//    pMacHalFunc->SIGNAL_WAIT                = MacHalGeneralDummy;
//    pMacHalFunc->SIGNAL_SET                 = MacHalGeneralDummy;
    pMacHalFunc->MSG_PRINT                  = HALAPI_PRINT;
    pMacHalFunc->MUTEX_INIT                 = HALAPI_MUTEX_INIT;
    pMacHalFunc->MUTEX_DEINIT               = HALAPI_MUTEX_DEINIT;
    pMacHalFunc->MUTEX_LOCK                 = HALAPI_MUTEX_LOCK;
    pMacHalFunc->MUTEX_UNLOCK               = HALAPI_MUTEX_UNLOCK;
    pMacHalFunc->EVENT_INDICATION           = HALAPI_EVENT_INDICATION;
}

#ifdef WLAN_SUPPORT_H2C_PACKET    
u8 HAL_SEND_H2C_PKT(
    IN VOID *pDriver_adapter, 
    IN u8 *pBuf, 
    IN u32 size
)
{
#if 1
    //TX_DESC_DATA_88XX               desc_data;
    PHCI_TX_DMA_QUEUE_STRUCT_88XX   cur_q;
	PHCI_TX_DMA_MANAGER_88XX        ptx_dma;
    PTX_BUFFER_DESCRIPTOR           cur_txbd;    
    HAL_PADAPTER    pAdapter =      (HAL_PADAPTER)pDriver_adapter;
    pu1Byte                         cur_h2c_buffer;
    u2Byte                          host_wrtie,hw_read;
    PH2C_PAYLOAD_88XX   h2c_buf_start = (PH2C_PAYLOAD_88XX)_GET_HAL_DATA(pAdapter)->h2c_buf;
    ptx_dma     = (PHCI_TX_DMA_MANAGER_88XX)(_GET_HAL_DATA(pAdapter)->PTxDMA88XX);
    cur_q       = &(ptx_dma->tx_queue[HCI_TX_DMA_QUEUE_CMD]);
    cur_txbd    = cur_q->pTXBD_head + cur_q->host_idx;
    
    cur_h2c_buffer = &(h2c_buf_start[cur_q->host_idx]);

    //memset(&desc_data, 0, sizeof(TX_DESC_DATA_88XX));
    hw_read = GetTxQueueHWIdx88XX(pDriver_adapter,HCI_TX_DMA_QUEUE_CMD);
    host_wrtie = cur_q->host_idx;

    //printk("h2c send packet \n");
    if(CIRC_SPACE_RTK(host_wrtie,hw_read,H2C_ENTERY_MAX_NUM) < 1)
    {
        printk("h2c page full \n");
        return FALSE;
    }

    memcpy(cur_h2c_buffer,pBuf,size);
    // 1. Fill desc_data
    //desc_data.hdrLen = size;
    //desc_data.pHdr = pBuf;
    // 2. FillTXDESC
    //FillH2CTxDesc88XX(pDriver_adapter, HCI_TX_DMA_QUEUE_CMD);

    // 3. SetTXBufferDesc
    //SetTxBufferDesc88XX(pDriver_adapter, HCI_TX_DMA_QUEUE_CMD, (PTX_DESC_DATA_88XX)&desc_data);
    
    if (size) {
        _dma_cache_wback(((GET_DESC_FIELD(cur_txbd->TXBD_ELE[1].Dword1, TXBD_DW1_PHYADDR_LOW_MSK, TXBD_DW1_PHYADDR_LOW_SH)|0x80000000) - CONFIG_LUNA_SLAVE_PHYMEM_OFFSET_HAL),
            (u4Byte)size);
    }


    //printk("cur_h2c_buffer adr = %x \n",cur_h2c_buffer);
    //RT_PRINT_DATA(COMP_SEND, DBG_TRACE, "H2C Payload:\n", cur_h2c_buffer, 32);    

    
    UpdateSWTXBDHostIdx88XX(pDriver_adapter, cur_q);
    SyncSWTXBDHostIdxToHW88XX(pDriver_adapter, HCI_TX_DMA_QUEUE_CMD);
    return TRUE;
    
#endif
}
#endif 

u8 HAL_SEND_RSVD_PAGE(
    IN VOID *pDriver_adapter, 
    IN u8 *pBuf, 
    IN u32 size
)
{
    SigninBeaconTXBD88XX(pDriver_adapter,(pu4Byte)pBuf, size);    // fill TXDESC & TXBD

    //RT_PRINT_DATA(COMP_INIT, DBG_LOUD, "DLtoTXBUFandDDMA88XX\n", pbuf, len);

    if(_FALSE == DownloadRsvdPage88XX(pDriver_adapter,NULL,0,0)) {
        RT_TRACE_F(COMP_INIT, DBG_WARNING,("Download to TXpktbuf fail ! \n"));
        return FALSE;
    }else{
        return TRUE;
    }    
}

u8
HALAPI_R8(
    IN VOID *pDriver_adapter,
    IN u32 offset
)
{
    return RTL_R8_F(pDriver_adapter,offset);
}

u16
HALAPI_R16(
    IN VOID *pDriver_adapter,
    IN u32 offset
)
{
    return RTL_R16_F(pDriver_adapter,offset);  
}

u32
HALAPI_R32(
    IN VOID *pDriver_adapter,
    IN u32 offset
)
{
    return RTL_R32_F(pDriver_adapter,offset);   
}


VOID
HALAPI_W8(
    IN VOID *pDriver_adapter,
    IN u32 offset,
    IN u8 value
)
{
    RTL_W8_F(pDriver_adapter,offset,value);   
}

VOID
HALAPI_W16(
    IN VOID *pDriver_adapter,
    IN u32 offset,
    IN u16 value
)
{
    RTL_W16_F(pDriver_adapter,offset,value);   
}

VOID
HALAPI_W32(
    IN VOID *pDriver_adapter,
    IN u32 offset,
    IN u32 value
)
{
    RTL_W32_F(pDriver_adapter,offset,value);   
}

VOID
MacHalGeneralDummy(struct rtl8192cd_priv *priv)
{
}

u8 
HALAPI_PRINT(
    IN VOID *pDriver_adapter, 
    IN u32 msg_type, 
    IN u8 msg_level, 
    IN s8* lpszFormat,...
)
{
//#if HALMAC_DEBUG_MESSAGE
    printk(lpszFormat);
//#endif
 return _TRUE;
}

VOID
HALAPI_MALLOC(
    IN VOID *pDriver_adapter, 
    IN u32 size
)
{
    kmalloc(size, GFP_ATOMIC);    
}


VOID 
HALAPI_FREE(
 IN VOID *pDriver_adapter, 
 IN VOID *pBuf, 
 IN u32 size
)
{
    kfree(pBuf);
}


VOID
HALAPI_MEMCPY(
    IN VOID *pDriver_adapter, 
    IN VOID *dest, 
    IN VOID *src, 
    IN u32 size
)
{
    memcpy(dest,src,size);    
}

VOID
HALAPI_MEMSET(
    IN VOID *pDriver_adapter, 
    IN VOID *pAddress, 
    IN u8 value, 
    IN u32 size
)
{
    memset(pAddress,value,size);    
}
VOID
HALAPI_DELAY_US(
    IN VOID *pDriver_adapter, 
    IN u32 us
)
{
    delay_us(us);  
}
#if 0
VOID
HALAPI_MSLEEP(
    IN VOID *pDriver_adapter, 
    IN u32 ms
)
{
    delay_ms(ms);  
}
#endif
VOID
HALAPI_MUTEX_INIT(
 IN VOID *pDriver_adapter, 
 IN HALMAC_MUTEX *pMutex
)
{
    return;
}    

VOID
HALAPI_MUTEX_DEINIT(
 IN VOID *pDriver_adapter, 
 IN HALMAC_MUTEX *pMutex
)
{
    return;
}   

VOID
HALAPI_MUTEX_LOCK(
 IN VOID *pDriver_adapter, 
 IN HALMAC_MUTEX *pMutex
)
{
    u32 flag;
    spin_lock_irqsave(pMutex, flag);
}    

 

VOID
HALAPI_MUTEX_UNLOCK(
 IN VOID *pDriver_adapter, 
 IN HALMAC_MUTEX *pMutex
)
{
    u32 flag;
    spin_unlock_irqrestore(pMutex, flag);
}    

VOID
HALAPI_EVENT_INDICATION(
 IN VOID *pDriver_adapter, 
 IN HALMAC_FEATURE_ID feature_id, 
 IN HALMAC_CMD_PROCESS_STATUS process_status, 
 IN u8* buf, u32 size
)
{
    return;
}

#endif 



