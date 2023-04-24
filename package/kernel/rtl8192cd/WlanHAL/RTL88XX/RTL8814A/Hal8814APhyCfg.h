#ifndef __HAL8814APHYCFG_H__
#define __HAL8814APHYCFG_H__

/*++
Copyright (c) Realtek Semiconductor Corp. All rights reserved.

Module Name:
	Hal8814APhyCfg.h
	
Abstract:
	Defined HAL 8814A PHY BB setting functions
	    
Major Change History:
	When       Who               What
	---------- ---------------   -------------------------------
	2013-05-28 Filen              Create.	
--*/

void 
TXPowerTracking_ThermalMeter_Tmp8814A(
    IN  HAL_PADAPTER    Adapter
);

u4Byte
PHY_QueryRFReg_8814(
	IN  HAL_PADAPTER                Adapter,
	IN  u4Byte                      eRFPath,
	IN  u4Byte                      RegAddr,
	IN  u4Byte                      BitMask
);

void
PHY_Set_SecCCATH_by_RXANT_8814A(
	IN  HAL_PADAPTER                Adapter,
	IN  u4Byte                      ulAntennaRx
);

void
phy_SpurCalibration_8814A(
    IN  HAL_PADAPTER                Adapter
);

void
phy_ConfigBB_8814A(
    IN  HAL_PADAPTER                Adapter
);
#endif //__HAL8814APHYCFG_H__

