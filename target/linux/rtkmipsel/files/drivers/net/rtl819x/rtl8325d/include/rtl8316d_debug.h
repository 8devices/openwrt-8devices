/*
* Copyright (C) 2009 Realtek Semiconductor Corp.
* All Rights Reserved.
*
* This program is the proprietary software of Realtek Semiconductor
* Corporation and/or its licensors, and only be used, duplicated,
* modified or distributed under the authorized license from Realtek.
*
* ANY USE OF THE SOFTWARE OTEHR THAN AS AUTHORIZED UNDER 
* THIS LICENSE OR COPYRIGHT LAW IS PROHIBITED.
* 
* $Revision: 8334 $
* $Date: 2010-02-09 13:22:42 +0800 (Tue, 09 Feb 2010) $
*
* Purpose : Utility for debugging.
*
*  Feature :  This file consists of following modules:
*             1) 
*
*/


#ifndef RTL8316D_DEBUG_H
#define RTL8316D_DEBUG_H

#define RTL8316D_DEBUG_MSG       /* DEFINED: enable debugging message */
//#undef RTL8316D_DEBUG_MSG       /*to close debugging message*/

#ifdef RTL8316D_DEBUG_MSG

/*=========== global debug message ==========*/
/* global debug level */
#define RTL8316D_DEBUG_MASK_NULL            0
#define RTL8316D_DEBUG_VERIF                     (1 << 0)
#define RTL8316D_DEBUG_INFO                      (1 << 1)
#define RTL8316D_DEBUG_WARN                     (1 << 2)
#define RTL8316D_DEBUG_ERR                        (1 << 3)

#define RTL8316D_DEBUG_MEMDUMP          (1 << 4)

//#define RTL8316D_DEBUG_MASK               RTL8316D_DEBUG_MASK_NULL
#define RTL8316D_DEBUG_MASK               RTL8316D_DEBUG_ERR|RTL8316D_DEBUG_INFO

/* Individual module debug mask */
#define RTL8316D_DEBUG_NULL                 0
#define RTL8316D_DEBUG_VLAN                (1 << 0)          /* vlan debug mask */
#define RTL8316D_DEBUG_PRI                   (1 << 1)          /* priority debug mask */
#define RTL8316D_DEBUG_802DOT1X         (1 << 2)         /*802.1X network access control*/
#define RTL8316D_DEBUG_RANGECHECK     (1 << 3)        /*TCP/UDP port range checking*/
#define RTL8316D_DEBUG_PKTTRANSLATOR (1 << 4)       /*packet translator debug*/
#define RTL8316D_DEBUG_TRAFFIC             (1 << 5)      /*traffic isolation filter debug*/
#define RTL8316D_DEBUG_PIE                     (1 << 6)     /*PIE look up*/
#define RTL8316D_DEBUG_FLOWCLF           (1 << 7)      /*flow classification*/
#define RTL8316D_DEBUG_ACL                   (1 << 8)      /*ACL debug*/
#define RTL8316D_DEBUG_PARSER              (1 << 9)     /*packet parser debug*/
#define RTL8316D_DEBUG_LEARNING           (1 << 10)   /*source mac learning debug*/
#define RTL8316D_DEBUG_ADDRLOOKUP      (1 << 11)   /*L2 table and foward table lookup*/
#define RTL8316D_DEBUG_RMA                   (1 << 12)   /*RMA*/
#define RTL8316D_DEBUG_OAM                  (1 << 13)    /*OAM Parser decision& oam multiplexer*/
#define RTL8316D_DEBUG_LINKAGGR         (1 << 14)                  /*Link Aggregation*/
#define RTL8316D_DEBUG_LOOPDETECTION    (1 << 15)              /*Loop detection*/
#define RTL8316D_DEBUG_PKTGEN             (1 << 16)        /*packet generator*/
#define RTL8316D_DEBUG_TEMPLGEN         (1 << 17)
#define RTL8316D_DEBUG_MIRROR             (1 << 18)           /*Mirror, sFlow and RSPAN*/
#define RTL8316D_DEBUG_ATTACKPREV       (1 << 19)        /*Attack prevention*/
#define RTL8316D_DEBUG_ICMODEL             (1 << 20)        /*IC Model*/
#define RTL8316D_DEBUG_SPT                  (1 << 21)        /*Spanning tree protocol*/
#define RTL8316D_DEBUG_TESTCASE         (1 << 22)           /*for testing cases*/
/*to add your module debug mask here.....*/


#define RTL8316D_DEBUG_REGDRVAPI        (1 << 30)
#define RTL8316D_DEBUG_MISC                 (1 << 31)         /*for all trivial things*/

//#define RTL8316D_DEBUG_MODULE_MASK  RTL8316D_DEBUG_NULL | RTL8316D_DEBUG_ADDRLOOKUP |RTL8316D_DEBUG_TESTCASE|RTL8316D_DEBUG_VLAN

#define RTL8316D_DEBUG_MODULE_MASK RTL8316D_DEBUG_PKTTRANSLATOR 

#endif /*RTL8316D_DEBUG_MSG*/

#ifndef rtlglue_printf
    #ifdef __KERNEL__ 
		#ifdef CONFIG_PRINTK
			#define rtlglue_printf	printk
		#else
			#define rtlglue_printf	panic_printk
		#endif		
    #else
        #define rtlglue_printf printf 
    #endif
#endif


    /* global debug message define */
#if ((RTL8316D_DEBUG_MASK) & RTL8316D_DEBUG_VERIF)
#define DEBUG_VERIF(type,fmt, args...) \
    do {if(type & (RTL8316D_DEBUG_MODULE_MASK)) rtlglue_printf("[%s-%d]-verif-: " fmt "\n", __FUNCTION__, __LINE__, ## args);} while (0)
#else
#define DEBUG_VERIF(type, fmt, args...) do {} while(0)
#endif

#if ((RTL8316D_DEBUG_MASK) & RTL8316D_DEBUG_INFO)
#define DEBUG_INFO(type,fmt, args...) \
    do {if(type & (RTL8316D_DEBUG_MODULE_MASK)) rtlglue_printf("[%s-%d]-info-: " fmt "\n", __FUNCTION__, __LINE__, ## args);} while (0)
#else
#define DEBUG_INFO(type,fmt, args...) do {} while(0)
#endif

#if ((RTL8316D_DEBUG_MASK) & RTL8316D_DEBUG_WARN)
#define DEBUG_WARN(type,fmt, args...) \
    do {if(type & (RTL8316D_DEBUG_MODULE_MASK)) rtlglue_printf("[%s-%d]-warn-: " fmt "\n", __FUNCTION__, __LINE__, ## args);} while (0)
#else
#define DEBUG_WARN(type,fmt, args...) do {} while(0)
#endif

#if ((RTL8316D_DEBUG_MASK) & RTL8316D_DEBUG_ERR)
#define DEBUG_ERR(type,fmt, args...) \
    do {if(type & (RTL8316D_DEBUG_MODULE_MASK)) rtlglue_printf("[%s-%d]-error-: " fmt "\n", __FUNCTION__, __LINE__, ## args);} while (0)
#else
#define DEBUG_ERR(type,fmt, args...) do {} while(0)
#endif

#if ((RTL8316D_DEBUG_MASK) & RTL8316D_DEBUG_MEMDUMP)
#define DEBUG_MEMDUMP(type, start, size, strHeader) \
    do {if(type & (RTL8316D_DEBUG_MODULE_MASK))   \
        rtlglue_printf("[%s-%d]-memdump-:\n", __FUNCTION__, __LINE__);   \
        memDump(start,size,strHeader);} while (0)
#else
#define DEBUG_MEMDUMP(type, start, size, strHeader) do {} while(0)
#endif

#ifndef RTL8316D_DEBUG
#define ASSERT(expr) do {if (expr); } while (0)
#else
#define ASSERT(expr) \
        if((expr) == 0) { \
            rtlglue_printf( "\033[33;41m%s:%d: assert(%s)\033[m\n", \
                                __FILE__,__LINE__,#expr); \
        }
#endif
#ifndef RTL8316D_DEBUG
#define PRINT_ERROR(fmt, args...) do {} while (0)
#else
#define PRINT_ERROR(fmt, args...) \
    do {rtlglue_printf("[%s-%d]-ERROR-: " fmt "\n", __FUNCTION__, __LINE__, ## args);} while (0)
#endif    


#endif /*header file*/

