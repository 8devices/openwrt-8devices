/*
* Program : Implementation of Glue Functions of Model Code
* Abstract : 
* Author : Louis Yung-Chieh Lo (yjlou@realtek.com.tw)               
* $Id: rtl_glue.c,v 1.5 2008/04/11 10:12:38 bo_zhao Exp $

*  Copyright (c) 2011 Realtek Semiconductor Corp.
*
*  This program is free software; you can redistribute it and/or modify
*  it under the terms of the GNU General Public License version 2 as
*  published by the Free Software Foundation.
*/

#include <net/rtl/rtl_types.h>
#include <net/rtl/rtl_glue.h>

#if 0
int rtl_down_interruptible(struct semaphore * sem)
{
	//return 0;
	return down_interruptible(sem);
}

void rtl_up(struct semaphore * sem)
{
	up(sem);
}

int32 rtlglue_drvMutexLock(void)
{	
#if 0
	spinlock_t lockForCC = SPIN_LOCK_UNLOCKED;
	int flagsForCC;
	int needLock;

	spin_lock_irqsave(&lockForCC,flagsForCC);

	needLock = (int_cc >= 1)?FALSE:TRUE;
	int_cc ++;

	if ( needLock == TRUE )
	{
		uint32 dummy;
 		saveGimr = READ_MEM32( GIMR );
		
#if defined (CONFIG_RTK_VOIP_DRIVERS_PCM865xC)	
		WRITE_MEM32( GIMR, 0x00080000 );		/* Note: DO NOT disable PCM interrupt because users might be already on the phone call at this time
												==> Keep PCM interrupt enable for RTK VoIP applications
													==> RTL865xC, bit19: PCM_IE 
											*/		
#elif defined (CONFIG_RTK_VOIP_DRIVERS_PCM8651)
		WRITE_MEM32( GIMR, 0x00040000 );		/* RTL865xB, bit18: PCM_IE */
#else
		WRITE_MEM32( GIMR, 0 );
#endif
		/*
			For RLX5181, LW/SW for external IPs (ex. GIMR) would be slow because one
			CMD queue is implement.

			So, we would always read GIMR back before calling <spin_unlock_irqrestore()> to
			make sure GIMR has be set before spin-unlock.
		*/
		dummy = READ_MEM32( GIMR );

 	}
 	
	spin_unlock_irqrestore(&lockForCC, flagsForCC);
#endif
	return 0;
}

int32 rtlglue_drvMutexUnlock(void)
{
#if 0
	spinlock_t lockForCC = SPIN_LOCK_UNLOCKED;
	int flagsForCC;
	int needUnlock;
	
	spin_lock_irqsave(&lockForCC,flagsForCC);
	
	int_cc--;
	needUnlock = ( int_cc==0 )?TRUE:FALSE;

	if ( needUnlock == TRUE )
	{
 		WRITE_MEM32( GIMR, saveGimr );
		/*
			For RLX5181, LW/SW for external IPs (ex. GIMR) would be slow because one
			CMD queue is implement.

			So, we would always read GIMR back before calling <spin_unlock_irqrestore()> to
			make sure GIMR has be set before spin-unlock.
		*/
		saveGimr = READ_MEM32( GIMR );
	}

	spin_unlock_irqrestore(&lockForCC, flagsForCC);
#endif
	return 0;
}
#endif

