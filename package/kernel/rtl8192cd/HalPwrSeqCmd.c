/*++
Copyright (c) Realtek Semiconductor Corp. All rights reserved.

Module Name:
	HalPwrSeqCmd.c

Abstract:
	Implement HW Power sequence configuration CMD handling routine for Realtek devices.

Major Change History:
	When       Who               What
	---------- ---------------   -------------------------------
	2011-07-07 Roger            Create.

--*/


/*
#if !defined(__ECOS) && !defined(CPTCFG_CFG80211_MODULE)
#include "mp_precomp.h"
#else
#include "../mp_precomp.h"
#endif
*/
#include <linux/kernel.h>
#include "8192cd.h"
#include "8192cd_debug.h"
#include "8192cd_headers.h"
#include "8192cd_util.h"


