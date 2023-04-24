#ifndef __INC_PRECOMP_H
#define __INC_PRECOMP_H

/*++
Copyright (c) Realtek Semiconductor Corp. All rights reserved.

Module Name:
	HalPrecomp.h
	
Abstract:
	Defined HAL Configure
	    
Major Change History:
	When       Who               What
	---------- ---------------   -------------------------------
	2012-03-23 Filen            Create.	
--*/

//Avoid Recusive include, confused with Driver
// Only WlanHAL files  define
#define WLAN_HAL_INTERNAL_USED
#if !defined(__ECOS) && !defined(CPTCFG_CFG80211_MODULE)
#include "PlatformDef.h"
#include "GeneralDef.h"

//Driver Configuration & Input
#include "HalCfg.h"
#include "HalPrecompInc.h"
#else // use relative

#include "Include/PlatformDef.h"
#include "Include/GeneralDef.h"

//Driver Configuration & Input
#include "HalCfg.h"
#include "HalPrecompInc.h"

#endif
#endif  //#ifndef __INC_PRECOMP_H
