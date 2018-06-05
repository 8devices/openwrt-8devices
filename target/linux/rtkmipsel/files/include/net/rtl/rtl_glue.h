/*
* --------------------------------------------------------------------
* Copyright c                  Realtek Semiconductor Corporation, 2002  
* All rights reserved.
* 
* Program : rtl_glue.h
* Abstract :Header of porting layer
* Author	: Edward Jin-Ru Chen
*
* $Id: rtl_glue.h,v 1.3 2008/04/11 10:12:38 bo_zhao Exp $
* $Log: rtl_glue.h,v $
* Revision 1.3  2008/04/11 10:12:38  bo_zhao
* *: swap nic drive to 8186 style
*
* Revision 1.2  2008/01/18 02:50:16  alva_zhang
* *: refine function define to pass the gcc-3.4 compiling
*
* Revision 1.1  2007/12/21 10:29:52  davidhsu
* +: support nic driver
*
* Revision 1.35  2007/07/03 05:53:51  qy_wang
* *:add rtl8316s model core
*
* Revision 1.34  2007/06/07 05:35:30  chenyl
* +: add new function "rtlglue_clearDCache" to clear D-cache without
*    writing back to external memory. ( Only for Processor with this function )
*
* Revision 1.33  2007/06/01 09:20:52  michaelhuang
* -: removed the rtl867x compiler flag.
*
* Revision 1.32  2007/04/09 14:14:01  michaelhuang
* +: added rtlglue_read() function
*
* Revision 1.31  2007/03/02 13:31:59  chenyl
* *: enable range based D-cache flush function.
*
* Revision 1.30  2007/01/09 02:36:08  michaelhuang
* *: Prepare implement test module for RTL8672
*
* Revision 1.29  2006/12/21 06:57:00  yjlou
* -: remove the following usages:
*    printk
*    printfByPolling
*
* Revision 1.28  2006/11/28 02:15:14  hyking_liu
* *: Mofify function related with usrDefineTunnel
*
* Revision 1.27  2006/11/21 12:28:35  chenyl
* *: rearrange some code
* *: bug fix - Src VLAN decision
* *: bug fix - D-cache invalidation for FAST extension device forwarding
* *: bug fix - m_unused1 and ph_reserved are needed for FAST extension device forwarding
*
* Revision 1.26  2006/10/26 02:15:32  qy_wang
* *:remove the inculed file net_device.h in rtl8651_layer2fwd.c
*
* Revision 1.25  2006/10/25 07:10:15  qy_wang
* +:add a function to block extension device when link up
*
* Revision 1.24  2006/08/31 15:44:48  chenyl
* *: add some declarations of external APIs
*
* Revision 1.23  2006/07/13 15:55:49  chenyl
* *: modify code for RTL865XC driver compilation.
* *: dos2unix process
*
* Revision 1.22  2006/06/20 05:53:22  brian_bin
* +: brian_bin move open, write, close declaration into __RTL_GLUE__ define brace
*
* Revision 1.21  2006/01/12 08:32:37  brian_bin
* +: brian_bin add kernel system call of file operations
*
* Revision 1.20  2006/01/02 02:52:14  chenyl
* +: VLAN-tag support extension port forwarding and software Rome Driver forwarding.
*
* Revision 1.19  2005/09/16 11:34:48  tony
* *: bug fixed: support kernel space swNic model code for RTL865XC.
*
* Revision 1.18  2005/09/06 02:36:38  yjlou
* +: add random and time glue functions.
*
* Revision 1.17  2005/07/20 15:29:38  yjlou
* +: porting Model Code to Linux Kernel: check RTL865X_MODEL_KERNEL.
*
* Revision 1.16  2005/07/01 09:34:41  yjlou
* *: porting swNic2.c into model code.
*
* Revision 1.15  2005/06/19 05:41:19  yjlou
* *: merge ASIC driver into model code.
*
* Revision 1.14  2005/06/07 15:00:57  chenyl
* *: modify mutex Lock / Unlock code to check the balance of Lock/Unlock function call
*
* Revision 1.13  2004/11/29 07:35:44  chenyl
* *: fix bug : multiple-PPPoE MTU setting bug
* *: fix bug : Never add second session's ip into ASIC in multiple-PPPoE WAN type.
* *: fix bug : Tunekey abnormal behavior when multiple-PPPoE Dial-on-demand Dynamically turn ON/OFF
* *: in forwarding engine Init function, the parameter == 0xffffffff means user want to set as default value
* *: add Mutex-Lock/Unlock checking in testing code
*
* Revision 1.12  2004/11/05 09:54:08  yjlou
* +: support more precise timeUpdate() function.
*   +: add EVENT_TRIGGER_TIMEUPDATE==2
*   *: modify #ifdef EVENT_TRIGGER_TIMEUPDATE
*
* Revision 1.11  2004/07/27 10:47:08  cfliu
* -: Remove RTL865x external loopback port related code for PPTP/L2TP/WLAN
*
* Revision 1.10  2004/06/23 08:11:55  yjlou
* *: change the declaration of rtlglue_getmstime()
*
* Revision 1.9  2004/05/03 14:56:18  cfliu
* Add 8650B extension port support.
* Revise all WLAN/extport related code.
*
* Revision 1.8  2004/05/03 02:53:03  chenyl
* *: swNic_getRingSize -> rtlglue_getRingSize and move it from swNic2.c to rtl_glue.c
* *: set default upstream ip to 0 if we config interface to dhcp
*
* Revision 1.7  2004/04/30 08:58:08  chenyl
* +: ip multicast/igmp proxy
*
* Revision 1.6  2004/04/08 12:11:20  cfliu
* Change extension port API.....define rtlglue_extDeviceSend()
*
* Revision 1.5  2004/03/31 09:37:01  cfliu
* Add WDS support
*
* Revision 1.4  2004/03/19 13:13:35  cfliu
* Reorganize ROME driver local header files. Put all private data structures into different .h file corrsponding to its layering
* Rename printf, printk, malloc, free with rtlglue_XXX prefix
*
* Revision 1.1  2004/03/10 11:42:36  cfliu
* Move rtl_glue.* from rtl865x/
*
* Revision 1.2  2004/03/03 10:40:38  yjlou
* *: commit for mergence the difference in rtl86xx_tbl/ since 2004/02/26.
*
* Revision 1.1  2004/02/25 14:26:33  chhuang
* *** empty log message ***
*
* Revision 1.2  2004/02/25 14:24:52  chhuang
* *** empty log message ***
*
* Revision 1.7  2004/02/24 04:15:43  cfliu
* add API for WLAN acceleration
*
* Revision 1.6  2004/02/18 13:54:37  chhuang
* *** empty log message ***
*
* Revision 1.5  2003/12/26 09:27:50  orlando
* add rtl_glue.h
*
* Revision 1.3  2003/09/30 06:07:50  orlando
* check in RTL8651BLDRV_V20_20030930
*
* Revision 1.4  2003/06/10 05:33:30  cfliu
* Remove rtl8651_tblDrvFwdSend registration.
* Add rtlglue_drvSend to replace rtl8651_tblDrvFwdSend function pointer
*
* Revision 1.3  2003/06/09 04:48:43  cfliu
* add 3 OS dependent glue functions to support mbuf external cluster allocation.
* Rewrite all help messages using Autoduck's syntax so documentation could generated automatically.
*
* Revision 1.2  2003/05/23 07:47:06  cfliu
* Extract porting function and fix compile warning.
*
* Revision 1.1  2003/05/23 04:55:25  jzchen
* Add rtl glue to solve porting issue
*
*/

#ifndef RTL_GLUE_H
#define RTL_GLUE_H

/*	@doc RTLGLUE_API

	@module rtl_glue.h - Glue interface for Realtek 8651 Home gateway controller driver	|
	This guide documents the glue interface for porting 8651 driver to targeted operating system
	@normal Chun-Feng Liu (cfliu@realtek.com.tw) <date>

	Copyright <cp>2003 Realtek<tm> Semiconductor Cooperation, All Rights Reserved.

 	@head3 List of Symbols |
 	Here is the list of all functions and variables in this module.

 	@index | RTLGLUE_API
*/
#include <linux/semaphore.h>
#include <linux/spinlock.h>
#include <linux/slab.h>

#define RTL_DECLARE_MUTEX(name) 		DECLARE_MUTEX(name)

//int rtl_down_interruptible(struct semaphore * sem);
//void rtl_up(struct semaphore * sem);
//
#ifndef RTL865X_DEBUG
#define assert(expr) do {} while (0)
#else
#define assert(expr) \
        if(!(expr)) {                                   \
        printk( "\033[33;41m%s:%d: assert(%s)\033[m\n", \
        __FILE__,__LINE__,#expr);               \
        }
#endif

#define RTL_BUG(cause) \
        do { printk(" [= !! BUG !! =] at %s line %d\n\t=> Cause: \
        %s\n\t=>-- system Halt\n", __FUNCTION__, __LINE__, cause); while(1);} while (0)

#define TBL_MEM_ALLOC(tbl, type, size)   \
        { \
        (tbl) = (type *)kmalloc((size) * sizeof(type),GFP_ATOMIC); \
                if(!(tbl)){\
                        printk("MEM alloc failed at line %d\n", __LINE__);\
                        while(1);\
                        return FAILED;\
                }\
        }

#ifndef bzero
	#define bzero( p, s ) memset( p, 0, s )
#endif

#define	RTL_NIC_LOCK_INIT(__rtl_lock__)		spin_lock_init(&__rtl_lock__)
#define	RTL_NIC_LOCK(__rtl_lock__)			spin_lock(&__rtl_lock__)
#define	RTL_NIC_UNLOCK(__rtl_lock__)		spin_unlock(&__rtl_lock__)

static inline int rtl_down_interruptible(struct semaphore * sem)
{
        return down_interruptible(sem);
}

static inline void rtl_up(struct semaphore * sem)
{
        up(sem);
}

static inline void *rtl_malloc(size_t NBYTES)
{
	if(NBYTES==0) return NULL;
		return (void *)kmalloc(NBYTES,GFP_ATOMIC);
}

static inline void rtl_free(void *APTR)
{
	kfree(APTR);
}

void memDump (void *start, uint32 size, int8 * strHeader);

#endif
