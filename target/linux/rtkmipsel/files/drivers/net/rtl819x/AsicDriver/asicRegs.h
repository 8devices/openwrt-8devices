/*
* ----------------------------------------------------------------
* Copyright c                  Realtek Semiconductor Corporation, 2002  
* All rights reserved.
* 
* $Header: /home1/sub1/tmp/cvs2svn/skylark/skylark/skylark/linux-2.6.x/drivers/net/re865x/AsicDriver/asicRegs.h,v 1.1 2007-12-21 10:29:24 davidhsu Exp $
*
* Abstract: ASIC specific definitions -- common part.
*
* $Author: davidhsu $
*
* -----------------------------------------------------------------
*	The following definitions are shared by 865xB/865xC series IC.
* -----------------------------------------------------------------
*/

//#include <rtl_types.h>

/*********************************************************************
 **                                                                 **
 **    Common Parts -- Add Common Definitions Here !                **
 **                                                                 **
 *********************************************************************/
#define UNCACHED_ADDRESS(x) ((void *)(0x20000000 | (uint32)x ))
#define CACHED_ADDRESS(x) ((void*)(~0x20000000 & (uint32)x ))
#define PHYSICAL_ADDRESS(x) (((uint32)x) & 0x1fffffff)
#define KSEG0_ADDRESS(x) ((void*)(PHYSICAL_ADDRESS(x) | 0x80000000))
#define KSEG1_ADDRESS(x) ((void*)(PHYSICAL_ADDRESS(x) | 0xA0000000))


/*********************************************************************
 **                                                                 **
 ** IC-Dependent Part --Add in the Specific Definitions in its file **
 **                                                                 **
 *********************************************************************/
#include "rtl865xc_asicregs.h"
