/*++
Copyright (c) Realtek Semiconductor Corp. All rights reserved.

Module Name:
	Hal8188EPwrSeq.c
	
Abstract:
	This file includes all kinds of Power Action event for RTL8188E and corresponding hardware configurtions which are released from HW SD.
	    
Major Change History:
	When       Who               What
	---------- ---------------   -------------------------------
	2011-07-07 Roger            Create.
	
--*/
#ifdef __ECOS
#include <cyg/io/eth/rltk/819x/wrapper/sys_support.h>
#endif
#include "8192cd.h"

#ifdef CONFIG_RTL_88E_SUPPORT

#include "HalPwrSeqCmd.h"
#include "Hal8188EPwrSeq.h"


/*
 *	drivers should parse below arrays and do the corresponding actions
 */

/*	Power on  Array	*/
WLAN_PWR_CFG rtl8188E_power_on_flow[RTL8188E_TRANS_CARDEMU_TO_ACT_STEPS+RTL8188E_TRANS_END_STEPS]=
{
	RTL8188E_TRANS_CARDEMU_TO_ACT
	RTL8188E_TRANS_END
};

/*	Radio off Array	*/
WLAN_PWR_CFG rtl8188E_radio_off_flow[RTL8188E_TRANS_ACT_TO_CARDEMU_STEPS+RTL8188E_TRANS_END_STEPS]=
{
	RTL8188E_TRANS_ACT_TO_CARDEMU
	RTL8188E_TRANS_END
};

/*	Card Disable Array	*/
WLAN_PWR_CFG rtl8188E_card_disable_flow[RTL8188E_TRANS_ACT_TO_CARDEMU_STEPS+RTL8188E_TRANS_CARDEMU_TO_PDN_STEPS+RTL8188E_TRANS_END_STEPS]=
{
	RTL8188E_TRANS_ACT_TO_CARDEMU
	RTL8188E_TRANS_CARDEMU_TO_CARDDIS
	RTL8188E_TRANS_END
};

/*	Card Enable Array	*/
WLAN_PWR_CFG rtl8188E_card_enable_flow[RTL8188E_TRANS_ACT_TO_CARDEMU_STEPS+RTL8188E_TRANS_CARDEMU_TO_PDN_STEPS+RTL8188E_TRANS_END_STEPS]=
{
	RTL8188E_TRANS_CARDDIS_TO_CARDEMU
	RTL8188E_TRANS_CARDEMU_TO_ACT
	RTL8188E_TRANS_END
};

/*	Suspend Array	*/
WLAN_PWR_CFG rtl8188E_suspend_flow[RTL8188E_TRANS_ACT_TO_CARDEMU_STEPS+RTL8188E_TRANS_CARDEMU_TO_SUS_STEPS+RTL8188E_TRANS_END_STEPS]=
{
	RTL8188E_TRANS_ACT_TO_CARDEMU
	RTL8188E_TRANS_CARDEMU_TO_SUS
	RTL8188E_TRANS_END
};

/*	Resume Array		*/
WLAN_PWR_CFG rtl8188E_resume_flow[RTL8188E_TRANS_ACT_TO_CARDEMU_STEPS+RTL8188E_TRANS_CARDEMU_TO_SUS_STEPS+RTL8188E_TRANS_END_STEPS]=
{
	RTL8188E_TRANS_SUS_TO_CARDEMU
	RTL8188E_TRANS_CARDEMU_TO_ACT
	RTL8188E_TRANS_END
};



/*	HWPDN Array		*/
WLAN_PWR_CFG rtl8188E_hwpdn_flow[RTL8188E_TRANS_ACT_TO_CARDEMU_STEPS+RTL8188E_TRANS_CARDEMU_TO_PDN_STEPS+RTL8188E_TRANS_END_STEPS]=
{
	RTL8188E_TRANS_ACT_TO_CARDEMU
	RTL8188E_TRANS_CARDEMU_TO_PDN	
	RTL8188E_TRANS_END
};

/*	Enter LPS 	*/
WLAN_PWR_CFG rtl8188E_enter_lps_flow[RTL8188E_TRANS_ACT_TO_LPS_STEPS+RTL8188E_TRANS_END_STEPS]=
{
	//FW behavior
	RTL8188E_TRANS_ACT_TO_LPS	
	RTL8188E_TRANS_END
};

/*	Leave LPS 	*/
WLAN_PWR_CFG rtl8188E_leave_lps_flow[RTL8188E_TRANS_LPS_TO_ACT_STEPS+RTL8188E_TRANS_END_STEPS]=
{
	//FW behavior
	RTL8188E_TRANS_LPS_TO_ACT
	RTL8188E_TRANS_END
};
#endif

