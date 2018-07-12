#ifndef __HAL88XXDEBUG_H__
#define __HAL88XXDEBUG_H__

/*++
Copyright (c) Realtek Semiconductor Corp. All rights reserved.

Module Name:
	Hal88XXDebug.h
	
Abstract:
	Defined HAL 88XX debug reigster
	    
Major Change History:
	When       Who               What
	---------- ---------------   -------------------------------
	2012-09-07 Eric              Create.	
--*/


//
//**Note**: 
//      If one of these register is used, we should register MACRO at RTL88XXX_debug.h.
//
// Range List:
//      1.) 0x5FE ~ 0x5FF : there are no control circuit. But these register can be R/W.

//1 Debug Register Location (from different IC)
// 4 Bytes
#define REG_DRV_DBG				REG_PAGE5_DUMMY

#define REG_DBG_DWORD_0         REG_DRV_DBG



//3 Register Debug register for our purpose
// 1.) Driver Component Error
#define REGDUMP_DRV_ERR0         REG_DBG_DWORD_0


//REGDUMP_DRV_ERR0
typedef enum _DRV_ERR0_STATUS_
{
    DRV_ER_INIT_PON             = BIT0,
    DRV_ER_INIT_MAC             = BIT1,
    DRV_ER_INIT_HCIDMA          = BIT2,
    DRV_ER_INIT_MACPHYREGFILE   = BIT3,
    DRV_ER_INIT_BBEGFILE        = BIT4,
    DRV_ER_INIT_PHYRF           = BIT5,
    DRV_ER_INIT_DLFW            = BIT6,
    DRV_ER_INIT_RSVD_0          = BIT7,
    DRV_ER_INIT_RSVD_1          = BIT8,
    DRV_ER_INIT_RSVD_2          = BIT9,
    DRV_ER_CLOSE_STOP_HW        = BIT10,
    DRV_ER_RSVD_0               = BIT11,
    DRV_ER_RSVD_1               = BIT12,
    DRV_ER_RSVD_2               = BIT13,
    DRV_ER_RSVD_3               = BIT14,
    DRV_ER_RSVD_4               = BIT15,
    DRV_ER_RSVD_5               = BIT16,    
    
}DRV_ERR0_STATUS, *PDRV_ERR0_STATUS;


#endif //  __HAL88XXDEBUG_H__

