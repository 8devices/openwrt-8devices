#ifndef __HALLIB_H__
#define __HALLIB_H__
/*++
Copyright (c) Realtek Semiconductor Corp. All rights reserved.

Module Name:
	HalLib.h
	
Abstract:
	API Interface that export Function Prototype & Variable & Strcture & Enum & Marco 
	for Driver
	    
Major Change History:
	When       Who               What
	---------- ---------------   -------------------------------
	2012-03-23 Filen            Create.	
--*/

#ifndef WLAN_HAL_INTERNAL_USED
#if !defined(__ECOS) && !defined(CPTCFG_CFG80211_MODULE)
//Code below only are used by "Not WlanHAL"
//1 HAL Library API

//4 Common
#include "PlatformDef.h"
#include "StatusCode.h"
#include "GeneralDef.h"

//4 HAL Common
#include "HalCfg.h"
#include "HalCommon.h"
#include "HalMacFunc.h"

#ifdef CONFIG_WLAN_MACHAL_API
#include "HalMacAPI.h"
#endif

//4 Chip Dependent
#if IS_RTL88XX_GENERATION
#include "Hal88XXDef.h"
#include "Hal88XXDesc.h"
#include "Hal88XXTxDesc.h"
#include "Hal88XXRxDesc.h"
#include "Hal88XXFirmware.h"
#include "Hal88XXDebug.h"
#include "Hal88XXPhyCfg.h"
#include "Hal88XXDM.h"

#if IS_EXIST_RTL8192EE
#endif

#if IS_EXIST_RTL8881AEM
#endif

#endif  //IS_RTL88XX_GENERATION



//Final Result
#include "HalDef.h"

#else //if ecos , use relative dir 

//Code below only are used by "Not WlanHAL"
//1 HAL Library API

//4 Common
#include "../Include/PlatformDef.h"
#include "../Include/StatusCode.h"
#include "../Include/GeneralDef.h"

//4 HAL Common
#include "../HalCfg.h"
#include "../HalCommon.h"
#ifdef CONFIG_WLAN_MACHAL_API 
#include "../HalMacAPI.h"
#endif


//4 Chip Dependent
#if IS_RTL88XX_GENERATION
#include "../RTL88XX/Hal88XXDef.h"
#include "../RTL88XX/Hal88XXDesc.h"
#include "../RTL88XX/Hal88XXTxDesc.h"
#include "../RTL88XX/Hal88XXRxDesc.h"
#include "../RTL88XX/Hal88XXFirmware.h"
#include "../RTL88XX/Hal88XXDebug.h"
#include "../RTL88XX/Hal88XXPhyCfg.h"
#include "../RTL88XX/Hal88XXDM.h"

#if IS_EXIST_RTL8192EE
#endif

#if IS_EXIST_RTL8881AEM
#endif

#endif  //IS_RTL88XX_GENERATION



//Final Result
#include "../HalDef.h"

#endif // ECOS
#endif  //WLAN_HAL_INTERNAL_USED












#endif  //#ifndef __HAL8881A_LIB_H__


