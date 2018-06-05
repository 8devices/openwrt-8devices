/*
 *
 *  Copyright (c) 2011 Realtek Semiconductor Corp.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 */


//#include "rtl_utils.h"
#include <net/rtl/rtl_types.h>
#include <net/rtl/rtl_glue.h>
#include "AsicDriver/asicRegs.h"
#ifdef CONFIG_RTL_LAYERED_ASIC_DRIVER
#include "AsicDriver/rtl865x_asicCom.h"
#include "AsicDriver/rtl865x_asicL2.h"
#else
#include <common/rtl8651_aclLocal.h>
#include <AsicDriver/rtl865xC_tblAsicDrv.h>
#endif

#include <net/rtl/rtk_stp.h>

int32 rtl865x_setSpanningEnable(int8 spanningTreeEnabled)
{
	return rtl8651_setAsicSpanningEnable(spanningTreeEnabled);
}

int32 rtl865x_setSpanningTreePortState(uint32 port, uint32 portState)
{
	return rtl865xC_setAsicSpanningTreePortState(port,  portState);
}

int32 rtl865x_setMulticastSpanningTreePortState(uint32 port, uint32 portState)
{
	//Note: 96C/98 have remove these bits!!!
	return SUCCESS;
}
