#ifndef __HAL8197FPHYCFG_H__
#define __HAL8197FPHYCFG_H__

/*++
Copyright (c) Realtek Semiconductor Corp. All rights reserved.

Module Name:
	Hal8197FPhyCfg.h
	
Abstract:
	Defined HAL 88XX PHY BB setting functions
	    
Major Change History:
	When       Who               What
	---------- ---------------   -------------------------------
	2012-11-14 Eric              Create.	
--*/



// TODO: this function should be modified
void 
TXPowerTracking_ThermalMeter_Tmp8197F(
    IN  HAL_PADAPTER    Adapter
);

u4Byte
PHY_QueryRFReg_8197F(
    IN  HAL_PADAPTER                Adapter,
    IN  u4Byte                      eRFPath,
    IN  u4Byte                      RegAddr,
    IN  u4Byte                      BitMask
);

void 
PHY_SetRFReg_8197F
(
    IN  HAL_PADAPTER                Adapter, 
    IN  u4Byte                      eRFPath,
    IN  u4Byte                      RegAddr,    
    IN  u4Byte                      BitMask,
    IN  u4Byte                      Data
);

void
SwBWMode88XX_97F(
	IN  HAL_PADAPTER    Adapter,
	IN  u4Byte          bandwidth,
	IN  s4Byte          offset
);




#endif // #ifndef __HAL8197FPHYCFG_H__

