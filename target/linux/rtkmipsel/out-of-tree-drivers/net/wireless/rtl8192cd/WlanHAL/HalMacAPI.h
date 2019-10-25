
#ifndef __HALMACAPI_H__
#define __HALMACAPI_H__
/*++
Copyright (c) Realtek Semiconductor Corp. All rights reserved.

Module Name:
	HalMacFunc.h
	
Abstract:
	Define MAC function support 
	for Driver
	    
Major Change History:
	When       Who               What
	---------- ---------------   -------------------------------
	2015-07-07 Eric            Create.	
--*/



#include "./HalMac88XX/halmac_2_platform.h"



void associate_halMac_API(struct rtl8192cd_priv *priv);
VOID MacHalGeneralDummy(struct rtl8192cd_priv *priv);
u8 HALAPI_R8(VOID *pDriver_adapter,u32 offset);
u8 HAL_SEND_RSVD_PAGE(VOID *pDriver_adapter,u8 *pBuf,u32 size);
u8 HAL_SEND_H2C_PKT(VOID *pDriver_adapter,u8 *pBuf,u32 size);
u16 HALAPI_R16(VOID *pDriver_adapter,u32 offset);
u32 HALAPI_R32(VOID *pDriver_adapter,u32 offset);
VOID HALAPI_W8(VOID *pDriver_adapter,u32 offset,u8 value);
VOID HALAPI_W16(VOID *pDriver_adapter,u32 offset,u16 value);
VOID HALAPI_W32(VOID *pDriver_adapter,u32 offset,u32 value);
u8 HALAPI_PRINT(VOID *pDriver_adapter,u32 msg_type,u8 msg_level,s8* lpszFormat,...);
VOID HALAPI_MALLOC(VOID *pDriver_adapter,u32 ms);
VOID HALAPI_FREE(VOID *pDriver_adapter,VOID *pBuf,u32 size);
VOID HALAPI_MEMCPY(VOID *pDriver_adapter,VOID *dest,VOID *src,u32 size);
VOID HALAPI_MEMSET(VOID *pDriver_adapter,VOID *pAddress,u8 value,u32 size);
VOID HALAPI_DELAY_US(VOID *pDriver_adapter,u32 us);
VOID HALAPI_MSLEEP(VOID *pDriver_adapter,u32 ms);
VOID HALAPI_MUTEX_INIT(VOID *pDriver_adapter,HALMAC_MUTEX *pMutex);   
VOID HALAPI_MUTEX_DEINIT(VOID *pDriver_adapter,HALMAC_MUTEX *pMutex);
VOID HALAPI_MUTEX_LOCK(VOID *pDriver_adapter,HALMAC_MUTEX *pMutex); 
VOID HALAPI_MUTEX_UNLOCK(VOID *pDriver_adapter,HALMAC_MUTEX *pMutex);   
VOID HALAPI_EVENT_INDICATION(VOID *pDriver_adapter,HALMAC_FEATURE_ID feature_id,HALMAC_CMD_PROCESS_STATUS process_status,u8* buf, u32 size);




#endif //__HALMACAPI_H__


