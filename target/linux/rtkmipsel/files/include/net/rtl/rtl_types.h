/*
* Copyright c                  Realtek Semiconductor Corporation, 2002  
* All rights reserved.                                                    
* 
* Program : The header file of realtek type definition
* Abstract :                                                           
* Author :              
* $Id: rtl_types.h,v 1.2 2008-01-18 07:53:11 hyking_liu Exp $
* $Log: not supported by cvs2svn $
* Revision 1.1  2007/12/21 10:29:52  davidhsu
* +: support nic driver
*
* Revision 1.27  2007/07/27 09:31:23  chenyl
* +: Add macro for MIPS16-enabled function declaration.
*
* Revision 1.26  2007/07/24 08:37:02  chenyl
* *: For MIPS16 related macros
*
* Revision 1.25  2007/06/25 13:08:43  alva_zhang
* -: remove suitable header file rtl_config.h
*
* Revision 1.24  2007/06/25 05:42:30  alva_zhang
* *: reopen including rtl_config.h
*
* Revision 1.23  2007/06/25 02:29:27  alva_zhang
* -: diable include rtl_config.h temporarily
*
* Revision 1.22  2007/06/23 09:11:00  alva_zhang
* +: add rtl_config.h which includes the MAcro indicate the usage of SDK
*
* Revision 1.21  2007/05/15 03:36:56  michaelhuang
* *: fixed compatibility for Linux Kernel 2.6
*
* Revision 1.20  2007/04/04 15:48:54  chenyl
* +: cleshell commend for asic register configuration : mmtu ( multicast mtu )
*
* Revision 1.19  2006/08/29 13:00:00  chenyl
* *: New SWNIC driver for RTL865xC
* *: some rearrange in re_core.c for Bootstrap sequence being more readable.
*
* Revision 1.18  2006/02/27 07:47:06  ympan
* +: No change by ympan
*
* Revision 1.17  2005/09/27 05:59:44  chenyl
* *: modify IRAM / DRAM layout:
* 	IRAM/DRAM-FWD		: external used, swNic ... blahblah
* 	IRAM/DRAM-L2-FWD	: fwdengine internal used, L2 and below (ex. preprocess, postprocess)
* 	IRAM/DRAM-L34-FWD	: fwdengine internal used, L3/L4 process only (ex. Routing, decision table...etc).
*
* 	=> If L34 is used, we strongly suggest L2 must be used, too.
*
* Revision 1.16  2005/08/23 14:38:26  chenyl
* +: apply prioirty IRAM/DRAM usage
*
* Revision 1.15  2005/08/22 07:33:55  chenyl
* *: don't set DRAM/IRAM for other OSs yet.
*
* Revision 1.14  2005/08/18 09:14:08  chenyl
* *: add code to porting to other OSs
*
* Revision 1.13  2005/08/18 06:29:29  chenyl
* +: always define the rtlglue_printf in rtl_types.h
*
* Revision 1.12  2005/07/01 09:34:41  yjlou
* *: porting swNic2.c into model code.
*
* Revision 1.11  2005/06/19 05:29:37  yjlou
* *: use 'unsigned int' to replace 'size_t'
* *: define spinlock_t when RTL865X_MODEL_USER defined.
*
* Revision 1.10  2005/06/10 05:32:22  yjlou
* +: Porting CLE Shell to Linux user space for model test.
*    See RTL865X_MODEL_USER compile flags.
*
* Revision 1.9  2005/01/10 03:21:43  yjlou
* *: always define __IRAM and __DRAM
*
* Revision 1.8  2004/07/23 13:42:45  tony
* *: remove all warning messages
*
* Revision 1.7  2004/07/05 08:25:32  chenyl
* +: define __IRAM, __DRAM for module test
*
* Revision 1.6  2004/07/04 15:04:55  cfliu
* +: add IRAM and DRAM
*
* Revision 1.5  2004/04/20 03:44:03  tony
* if disable define "RTL865X_OVER_KERNEL" and "RTL865X_OVER_LINUX", __KERNEL__ and __linux__ will be undefined.
*
* Revision 1.4  2004/03/19 13:13:35  cfliu
* Reorganize ROME driver local header files. Put all private data structures into different .h file corrsponding to its layering
* Rename printf, printk, malloc, free with rtlglue_XXX prefix
*
* Revision 1.3  2004/03/05 07:44:27  cfliu
* fix header file problem for ctype.h
*
* Revision 1.2  2004/03/03 10:40:38  yjlou
* *: commit for mergence the difference in rtl86xx_tbl/ since 2004/02/26.
*
* Revision 1.1  2004/02/25 14:26:33  chhuang
* *** empty log message ***
*
* Revision 1.3  2004/02/25 14:24:52  chhuang
* *** empty log message ***
*
* Revision 1.8  2003/12/10 06:30:12  tony
* add linux/config.h, disable define CONFIG_RTL865X_NICDRV2 in mbuf.c by default
*
* Revision 1.7  2003/12/03 14:25:43  cfliu
* change SIZE_T to _SIZE_T. Linux kernel seems to recognize _SIZE_T
*
* Revision 1.6  2003/10/01 12:29:02  tony
* #define DEBUG_P(args...) while(0);
*
* Revision 1.5  2003/10/01 10:31:47  tony
* solve all the compiler warnning messages in the board.c
*
* Revision 1.4  2003/09/30 06:07:50  orlando
* check in RTL8651BLDRV_V20_20030930
*
* Revision 1.30  2003/07/21 06:27:49  cfliu
* no message
*
* Revision 1.29  2003/04/30 15:32:30  cfliu
* move macros to types.h
*
* Revision 1.28  2003/03/13 10:29:22  cfliu
* Remove unused symbols
*
* Revision 1.27  2003/03/06 05:00:04  cfliu
* Move '#pragma ghs inlineprologue' to rtl_depend.h since it is compiler dependent
*
* Revision 1.26  2003/03/06 03:41:46  danwu
* Prevent compiler from generating internal sub-routine call code at the
*  function prologue and epilogue automatically
*
* Revision 1.25  2003/03/03 09:16:35  hiwu
* remove ip4a
*
* Revision 1.24  2003/02/18 10:04:06  jzchen
* Add ether_addr_t to compatable with protocol stack's ether_addr
*
* Revision 1.23  2003/01/21 05:59:51  cfliu
* add min, max, SETBITS, CLEARBITS, etc.
*
* Revision 1.22  2002/11/25 07:31:30  cfliu
* Remove _POSIX_SOURCE since it is cygwin specific
*
* Revision 1.21  2002/09/30 11:51:49  jzchen
* Add ASSERT_ISR for not print inside ISR
*
* Revision 1.20  2002/09/18 01:43:24  jzchen
* Add type limit definition
*
* Revision 1.19  2002/09/16 00:14:34  elvis
* remove struct posix_handle_t (change the handle type from
*  structure to uint32)
*
* Revision 1.18  2002/08/20 01:40:40  danwu
* Add definitions of ipaddr_t & macaddr_t.
*
* Revision 1.17  2002/07/30 04:36:30  danwu
* Add ASSERT_CSP.
*
* Revision 1.16  2002/07/19 06:47:30  cfliu
* Add _POSIX_SOURCE symbol
*
* Revision 1.15  2002/07/05 02:10:39  elvis
* Add new types for OSK
*
* Revision 1.14  2002/07/03 12:36:21  orlando
* <rtl_depend.h> will use type definitions. Has to be moved to
* be after the type declaration lines.
*
* Revision 1.13  2002/07/03 09:19:00  cfliu
* Removed all standard header files from source code. They would be included by <core/types.h>-><rtl_depend.h>
*
* Revision 1.12  2002/07/03 09:16:48  cfliu
* Removed all standard header files from source code. They would be included by <core/types.h>-><rtl_depend.h>
*
* Revision 1.11  2002/07/03 07:14:47  orlando
* Add "struct posix_handle_t_", used by POSIX module.
*
* Revision 1.9  2002/06/21 03:15:36  cfliu
* Add time.h for struct timeval
*
* Revision 1.8  2002/06/14 01:58:03  cfliu
* Move sa_family_t to socket
*
* Revision 1.7  2002/06/13 09:37:42  cfliu
* Move byte order conversion routines to socket
*
* Revision 1.6  2002/05/23 04:24:37  hiwu
* change memaddr_t to calladdr_t
*
* Revision 1.5  2002/05/13 10:15:16  hiwu
* add new type definition
*
* Revision 1.4  2002/05/09 05:21:51  cfliu
* Add parenthesis around swaps16, swapl32
*
* Revision 1.3  2002/04/30 03:07:34  orlando
* Remove UIxx_T definitions to conform with new
* naming conventions.
*
* Revision 1.2  2002/04/29 10:10:32  hiwu
* add NTOHS macro
*
* Revision 1.1.1.1  2002/04/26 08:53:53  orlando
* Initial source tree creation.
*
* Revision 1.9  2002/04/25 03:59:05  cfliu
* no message
*
* Revision 1.8  2002/04/08 08:08:04  hiwu
* initial version
*
*/


#ifndef _RTL_TYPES_H
#define _RTL_TYPES_H

#define RTL_LAYERED_DRIVER_DEBUG 0

#if 0
#ifndef RTL865X_OVER_KERNEL
	#undef __KERNEL__
#endif

#ifndef RTL865X_OVER_LINUX
	#undef __linux__
#endif
#endif

/*
 * Internal names for basic integral types.  Omit the typedef if
 * not possible for a machine/compiler combination.
 */
#ifdef __linux__
#ifdef __KERNEL__
#include <linux/version.h>

#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,2,0)
#include <linux/kconfig.h>
#elif (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,0))
#include <linux/config.h>
#endif
//#include <linux/ctype.h>
#include <linux/module.h>
//#include <linux/string.h>

#endif /*__KERNEL__*/
#endif /*__linux__*/

/* ===============================================================================
		IRAM / DRAM definition
    =============================================================================== */
#undef __DRAM_GEN
#undef __DRAM_FWD
#undef __DRAM_L2_FWD
#undef __DRAM_L34_FWD
#undef __DRAM_EXTDEV
#undef __DRAM_AIRGO
#undef __DRAM_RTKWLAN
#undef __DRAM_CRYPTO
#undef __DRAM_VOIP
#undef __DRAM_TX
#undef __DRAM

#undef __IRAM_GEN
#undef __IRAM_FWD
#undef __IRAM_L2_FWD
#undef __IRAM_L34_FWD
#undef __IRAM_EXTDEV
#undef __IRAM_AIRGO
#undef __IRAM_RTKWLAN
#undef __IRAM_CRYPTO
#undef __IRAM_VOIP
#undef __IRAM_TX
#undef __IRAM

#if defined(__linux__)&&defined(__KERNEL__)&& defined(CONFIG_RTL_819X) && !defined(CONFIG_RTL_8198C) && !defined(CONFIG_RTL_8197F)
	#define __DRAM_GEN			__attribute__  ((section(".dram-gen")))
	#define __DRAM_FWD			__attribute__  ((section(".dram-fwd")))
	#define __DRAM_L2_FWD		__attribute__  ((section(".dram-l2-fwd")))
	#define __DRAM_L34_FWD	__attribute__  ((section(".dram-l34-fwd")))
	#define __DRAM_EXTDEV		__attribute__  ((section(".dram-extdev")))
	#define __DRAM_AIRGO		__attribute__  ((section(".dram-airgo")))
	#define __DRAM_RTKWLAN	__attribute__  ((section(".dram-rtkwlan")))
	#define __DRAM_CRYPTO		__attribute__  ((section(".dram-crypto")))
	#define __DRAM_VOIP			__attribute__  ((section(".dram-voip")))
	#define __DRAM_TX			__attribute__  ((section(".dram-tx")))
	#define __DRAM				__attribute__  ((section(".dram")))

	#define __IRAM_GEN			__attribute__  ((section(".iram-gen")))
	#define __IRAM_FWD			__attribute__  ((section(".iram-fwd")))
	#define __IRAM_L2_FWD		__attribute__  ((section(".iram-l2-fwd")))
	#define __IRAM_L34_FWD		__attribute__  ((section(".iram-l34-fwd")))
	#define __IRAM_EXTDEV		__attribute__  ((section(".iram-extdev")))
	#define __IRAM_AIRGO		__attribute__  ((section(".iram-airgo")))
	#define __IRAM_RTKWLAN		__attribute__  ((section(".iram-rtkwlan")))
	#define __IRAM_CRYPTO		__attribute__  ((section(".iram-crypto")))
	#define __IRAM_VOIP			__attribute__  ((section(".iram-voip")))
	#define __IRAM_TX			__attribute__  ((section(".iram-tx")))
	#define __IRAM				__attribute__  ((section(".iram")))
#else
	#define __DRAM_GEN
	#define __DRAM_FWD
	#define __DRAM_L2_FWD
	#define __DRAM_L34_FWD
	#define __DRAM_EXTDEV
	#define __DRAM_AIRGO
	#define __DRAM_RTKWLAN
	#define __DRAM_CRYPTO
	#define __DRAM_VOIP
	#define __DRAM_TX
	#define __DRAM

	#define __IRAM_GEN
	#define __IRAM_FWD
	#define __IRAM_L2_FWD
	#define __IRAM_L34_FWD
	#define __IRAM_EXTDEV
	#define __IRAM_AIRGO
	#define __IRAM_RTKWLAN
	#define __IRAM_CRYPTO
	#define __IRAM_VOIP
	#define __IRAM_TX
	#define __IRAM
#endif

#if defined(CONFIG_RTL_DYNAMIC_IRAM_MAPPING_FOR_WAPI)
	#define __IRAM_GEN_WAPI			__attribute__  ((section(".iram-gen-wapi")))
	#define __IRAM_FWD_WAPI			__attribute__  ((section(".iram-fwd-wapi")))
	#define __IRAM_L2_FWD_WAPI		__attribute__  ((section(".iram-l2-fwd-wapi")))
	#define __IRAM_L34_FWD_WAPI		__attribute__  ((section(".iram-l34-fwd-wapi")))
	#define __IRAM_RTKWLAN_WAPI		__attribute__  ((section(".iram-rtkwlan-wapi")))
	#define __IRAM_TX_WAPI			__attribute__  ((section(".iram-tx-wapi")))

	#define FUNCTION_CHECK(x)	do \
							{ \
								if((x)==NULL) \
									printk("---%s %s(%d) function is NULL!!\n",__FILE__,__FUNCTION__,__LINE__); \
							} while(0)
#endif


/* ===============================================================================
		Additional GCC attribute
    =============================================================================== */

#undef __NOMIPS16
#undef __MIPS16

#if defined(__linux__)&&defined(__KERNEL__)&&defined(CONFIG_RTL_819X) && !defined(CONFIG_RTL_8196C) && !defined(CONFIG_RTL_8198C) && !defined(CONFIG_RTL_8197F) && !defined(CONFIG_RTL_DISABLE_ETH_MIPS16)
	#define __NOMIPS16			__attribute__((nomips16))	/* Inidcate to prevent from MIPS16 */
	#define __MIPS16			__attribute__((mips16))		/* Inidcate to use MIPS16 */
#else
	#define __NOMIPS16
	#define __MIPS16
#endif

/* ===============================================================================
		print macro
    =============================================================================== */
#if	defined(__linux__)&&defined(__KERNEL__)

// to_be_checked !!!, since kernel does not has panic_printk
//#ifdef CONFIG_RTL_8198C // modified by lynn_pu, 2014-10-21
#if defined(CONFIG_RTL_8198C) || defined(CONFIG_RTL_8197F) || (defined(CONFIG_RLX)&&(LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,0))) || defined(CONFIG_OPENWRT_SDK)
	#define  rtlglue_printf		printk
	#define  panic_printk		printk
#else
	#define rtlglue_printf	panic_printk
#endif

#else	/* defined(__linux__)&&defined(__KERNEL__) */

#ifdef	RTL865X_TEST
	#include <ctype.h>
#endif	/* RTL865X_TEST */

#define rtlglue_printf	printf

#endif	/* defined(__linux__)&&defined(__KERNEL__) */

/* ===============================================================================
		Type definition
    =============================================================================== */
#if 1
typedef unsigned long long	uint64;
typedef long long		int64;
typedef unsigned int	uint32;

#ifndef int32
//#undef int32
#if defined(CONFIG_OPENWRT_SDK)
#ifndef _SLHC_H
typedef int			int32;
#endif
#else
typedef int			int32;
#endif
#endif

typedef unsigned short	uint16;
typedef short			int16;
typedef unsigned char	uint8;
typedef char			int8;

#else
typedef __u64 uint64;
typedef __s64	int64;
typedef __u32	uint32;
#ifndef int32
typedef __s32			int32;
#endif
typedef __u16	uint16;
typedef __s16	int16;
typedef __u8		uint8;
typedef __s8			int8;
#endif

typedef uint32		memaddr;	
typedef uint32          ipaddr_t;


#if defined(CONFIG_RTL_8198C) || defined(CONFIG_RTL_8197F)
/*copy from in6.h*/
typedef struct inv6_addr_s
{
	union 
	{
		__u8		u6_addr8[16];
		__be16		u6_addr16[8];
		__be32		u6_addr32[4];
	} inv6_u;
#define v6_addr			inv6_u.u6_addr8
#define v6_addr16		inv6_u.u6_addr16
#define v6_addr32		inv6_u.u6_addr32
}inv6_addr_t;
#endif

typedef struct {
    uint16      mac47_32;
    uint16      mac31_16;
    uint16      mac15_0;
} macaddr_t;

#define ETHER_ADDR_LEN				6
typedef struct ether_addr_s {
	uint8 octet[ETHER_ADDR_LEN];
} ether_addr_t;

#if defined(CONFIG_RTL_ULINKER_BRSC)
#define RX_OFFSET	4
#else
#define RX_OFFSET	2
#endif
#define MBUF_LEN	1700
#define CROSS_LAN_MBUF_LEN		(MBUF_LEN+RX_OFFSET+10)

#if defined(CONFIG_RTL_819X)
	#if defined(CONFIG_RTL_ETH_PRIV_SKB) && !defined(CONFIG_RTL_8198C) && !defined(CONFIG_RTL_8197F)
	#define DELAY_REFILL_ETH_RX_BUF	1
	#endif

	#if defined(CONFIG_RTL_ETH_PRIV_SKB)
	#define PRIV_BUF_CAN_USE_KERNEL_BUF		1
	#define INIT_RX_RING_ERR_HANDLE			1
	//#define ALLOW_RX_RING_PARTIAL_EMPTY		1
	#endif	
#endif

#ifndef NULL
#define NULL 0
#endif
#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif

#ifndef SUCCESS
#define SUCCESS 	0
#endif
#ifndef FAILED
#define FAILED -1
#endif



#define DEBUG_P(args...) while(0);
#ifndef OK
#define OK		0
#endif
#ifndef NOT_OK
#define NOT_OK  1
#endif

#ifndef CLEARBITS
#define CLEARBITS(a,b)	((a) &= ~(b))
#endif

#ifndef SETBITS
#define SETBITS(a,b)		((a) |= (b))
#endif

#ifndef ISSET
#define ISSET(a,b)		(((a) & (b))!=0)
#endif

#ifndef ISCLEARED
#define ISCLEARED(a,b)	(((a) & (b))==0)
#endif

#ifndef max
#define max(a,b)  (((a) > (b)) ? (a) : (b))
#endif			   /* max */

#ifndef min
#define min(a,b)  (((a) < (b)) ? (a) : (b))
#endif			   /* min */

//round down x to multiple of y.  Ex: ROUNDDOWN(20, 7)=14
#ifndef ROUNDDOWN
#define	ROUNDDOWN(x, y)	(((x)/(y))*(y))
#endif

//round up x to multiple of y. Ex: ROUNDUP(11, 7) = 14
#ifndef ROUNDUP
#define	ROUNDUP(x, y)	((((x)+((y)-1))/(y))*(y))  /* to any y */
#endif

#ifndef ROUNDUP2
#define	ROUNDUP2(x, y)	(((x)+((y)-1))&(~((y)-1))) /* if y is powers of two */
#endif

#ifndef ROUNDUP4
#define	ROUNDUP4(x)		((1+(((x)-1)>>2))<<2)
#endif

#ifndef IS4BYTEALIGNED
#define IS4BYTEALIGNED(x)	 ((((x) & 0x3)==0)? 1 : 0)
#endif

#ifndef __offsetof
#define __offsetof(type, field) ((unsigned long)(&((type *)0)->field))
#endif

#ifndef offsetof
#define offsetof(type, field) __offsetof(type, field)
#endif

#ifndef RTL_PROC_CHECK
#define RTL_PROC_CHECK(expr, success) \
	do {\
			int __retval; \
			if ((__retval = (expr)) != (success))\
			{\
				rtlglue_printf("ERROR >>> [%s]:[%d] failed -- return value: %d\n", __FUNCTION__,__LINE__, __retval);\
				return __retval; \
			}\
		}while(0)
#endif

#ifndef RTL_STREAM_SAME
#define RTL_STREAM_SAME(s1, s2) \
	((strlen(s1) == strlen(s2)) && (strcmp(s1, s2) == 0))
#endif

#define ASSERT_ISR(x) if(!(x)) {while(1);}
#define RTL_STATIC_INLINE   static __inline__

#define ASSERT_CSP(x) if (!(x)) {rtlglue_printf("\nAssert Fail: %s %d", __FILE__, __LINE__); while(1);}
 

#if defined(RTL865X_TEST)||defined(RTL865X_MODEL_USER)
#define UNCACHE_MASK		0
#define UNCACHE(addr)		(addr)
#define CACHED(addr)			((uint32)(addr))
#else
#define UNCACHE_MASK		0x20000000
#define UNCACHE(addr)		((UNCACHE_MASK)|(uint32)(addr))
#define CACHED(addr)			((uint32)(addr) & ~(UNCACHE_MASK))
#endif

/*	asic configuration	*/
#if defined(CONFIG_RTL_8198C) || defined(CONFIG_RTL_8197F)
#define RTL8651_OUTPUTQUEUE_SIZE		8
#else
#define RTL8651_OUTPUTQUEUE_SIZE		6
#endif
#define TOTAL_VLAN_PRIORITY_NUM	8
#define RTL8651_RATELIMITTBL_SIZE			32

#define VLAN_ETH_ALEN	6		/* Octets in one ethernet addr	 */

#define NIPQUAD(addr) \
	((unsigned char *)&addr)[0], \
	((unsigned char *)&addr)[1], \
	((unsigned char *)&addr)[2], \
	((unsigned char *)&addr)[3]
	
#if defined(CONFIG_RTL_8196C)
#define CONFIG_RTL8196C_ETH_IOT         1
#ifdef CONFIG_MP_PSD_SUPPORT
#undef CONFIG_RTL8196C_GREEN_ETHERNET
#else
//#define CONFIG_RTL_8196C_ESD            1 
#endif
#endif

#if defined(CONFIG_RTL_8198) && !defined(CONFIG_RTL_819XD)
#define CONFIG_RTL_8198_ESD        1
#endif 

#if defined(CONFIG_RTL_8198)
#define RTL8198_EEE_MAC 	1
#endif

#if defined(CONFIG_RTL_819XD) || defined(CONFIG_RTL_8196E)
#define CONFIG_RTL_8197D_DYN_THR		1
#endif

#define DYN_THR_LINK_UP_PORTS			3

#ifdef CONFIG_RTL_8881A
/* IC default value */
#define DYN_THR_DEF_fcON				0x1ac
#define DYN_THR_DEF_fcOFF				0x1a0
#define DYN_THR_DEF_sharedON			0x162
#define DYN_THR_DEF_sharedOFF			0x14a

/* aggressive value */
#define DYN_THR_AGG_fcON				0x1d2
#define DYN_THR_AGG_fcOFF				0x1ba
#define DYN_THR_AGG_sharedON			0x190
#define DYN_THR_AGG_sharedOFF			0x178
#else
/* IC default value */
#define DYN_THR_DEF_fcON				0xac
#define DYN_THR_DEF_fcOFF				0xa0
#define DYN_THR_DEF_sharedON			0x62
#define DYN_THR_DEF_sharedOFF			0x4a

/* aggressive value */
#define DYN_THR_AGG_fcON				0xd0
#define DYN_THR_AGG_fcOFF				0xa0 // 0xc0
#if defined(CONFIG_RTL_819XDT)
#define DYN_THR_AGG_sharedON			0xa0
#define DYN_THR_AGG_sharedOFF			0x88
#else
#define DYN_THR_AGG_sharedON			0x88 // 0xc0
#define DYN_THR_AGG_sharedOFF			0x70 // 0xa8
#endif
#endif
#if defined(CONFIG_RTL_LOG_DEBUG)
extern int scrlog_printk(const char * fmt, ...);

extern struct RTL_LOG_PRINT_MASK
{
	uint32 ERROR:1;
	uint32 WARN:1;
	uint32 INFO:1;
}RTL_LogTypeMask;

extern struct RTL_LOG_ERROR_MASK
{
	uint32 MEM:1;
	uint32 SKB:1;
}RTL_LogErrorMask;
extern uint32 RTL_LogRatelimit;

extern struct RTL_LOG_MODULE_MASK
{
	uint8 NIC:1;
	uint8 WIRELESS:1;
	uint8 PROSTACK:1;
}RTL_LogModuleMask;


#define LOG_LIMIT (!RTL_LogRatelimit||net_ratelimit())


#define LOG_ERROR(fmt, args...) do{ \
	if(RTL_LogTypeMask.ERROR&&LOG_LIMIT)scrlog_printk("ERROR:"fmt, ## args); \
		}while(0)
		
#define LOG_MEM_ERROR(fmt, args...) do{ \
	if(RTL_LogTypeMask.ERROR&&RTL_LogErrorMask.MEM&&LOG_LIMIT)scrlog_printk("ERROR:"fmt, ## args); \
		}while(0)
		
#define LOG_SKB_ERROR(fmt, args...) do{ \
		if(RTL_LogTypeMask.ERROR&&RTL_LogErrorMask.SKB&&LOG_LIMIT)scrlog_printk("ERROR:"fmt, ## args); \
			}while(0)
			
#define LOG_WARN(fmt, args...) do{ \
		if(RTL_LogTypeMask.WARN&&LOG_LIMIT)scrlog_printk("WARN:"fmt, ## args); \
			}while(0)
			
#define LOG_INFO(fmt, args...) do{ \
		if(RTL_LogTypeMask.INFO&&LOG_LIMIT)scrlog_printk("INFO:"fmt, ## args); \
			}while(0)

#else

#define LOG_ERROR(fmt, args...) 
#define LOG_MEM_ERROR(fmt, args...) 
#define LOG_SKB_ERROR(fmt, args...)
#define LOG_WARN(fmt, args...)
#define LOG_INFO(fmt, args...)

#endif 

#ifdef CONFIG_SMP
#include <linux/spinlock.h>

extern int lock_owner;
extern int lock_owner_rx;
extern int lock_owner_tx;
extern int lock_owner_buf;
extern int lock_owner_hw;
extern int lock_owner_smi;
extern spinlock_t lock_eth_other;
extern spinlock_t lock_eth_rx;
extern spinlock_t lock_eth_tx;
extern spinlock_t lock_eth_buf;
extern spinlock_t lock_eth_hw;
extern spinlock_t lock_eth_smi;

#if 0
/* ============ release version ============ */
#define SMP_LOCK_ETH(__x__)				spin_lock_irqsave(&lock_eth_other, (__x__))	
#define SMP_UNLOCK_ETH(__x__) 			spin_unlock_irqrestore(&lock_eth_other, (__x__))

#define SMP_LOCK_ETH_XMIT(__x__)			spin_lock_irqsave(&lock_eth_tx, (__x__))
#define SMP_UNLOCK_ETH_XMIT(__x__) 		spin_unlock_irqrestore(&lock_eth_tx, (__x__))

#define SMP_LOCK_ETH_RECV(__x__)		spin_lock_irqsave(&lock_eth_rx, (__x__))
#define SMP_UNLOCK_ETH_RECV(__x__) 		spin_unlock_irqrestore(&lock_eth_rx, (__x__))

#define SMP_LOCK_ETH_BUF(__x__)			spin_lock_irqsave(&lock_eth_buf, (__x__))	
#define SMP_UNLOCK_ETH_BUF(__x__) 		spin_unlock_irqrestore(&lock_eth_buf, (__x__))

#define SMP_LOCK_ETH_HW(__x__)			spin_lock_irqsave(&lock_eth_hw, (__x__))
#define SMP_UNLOCK_ETH_HW(__x__) 		spin_unlock_irqrestore(&lock_eth_hw, (__x__))
#else

/* ============ debug version ============ */
#define SMP_LOCK_ETH(__x__)	\
	do { \
		if(lock_owner!=smp_processor_id()) \
			spin_lock_irqsave(&lock_eth_other, (__x__)); \
		else \
			rtlglue_printf("[%s %s %d] recursion detection in ETH\n",__FILE__, __FUNCTION__,__LINE__); \
		lock_owner=smp_processor_id();\
	} while(0)
	
#define SMP_UNLOCK_ETH(__x__) \
	do { \
		lock_owner=-1; \
		spin_unlock_irqrestore(&lock_eth_other, (__x__)); \
	} while(0)

#define SMP_LOCK_ETH_XMIT(__x__)	\
	do { \
		if(lock_owner_tx!=smp_processor_id()) \
			spin_lock_irqsave(&lock_eth_tx, (__x__)); \
		else \
			rtlglue_printf("[%s %d] recursion detection in XMIT\n",__FUNCTION__,__LINE__); \
		lock_owner_tx=smp_processor_id();\
	} while(0)
	
#define SMP_UNLOCK_ETH_XMIT(__x__) \
	do { \
		lock_owner_tx=-1; \
		spin_unlock_irqrestore(&lock_eth_tx, (__x__)); \
	} while(0)

#define SMP_LOCK_ETH_RECV(__x__)	\
	do { \
		if(lock_owner_rx!=smp_processor_id()) \
			spin_lock_irqsave(&lock_eth_rx, (__x__)); \
		else \
			rtlglue_printf("[%s %d] recursion detection in RECV\n",__FUNCTION__,__LINE__); \
		lock_owner_rx=smp_processor_id();\
	} while(0)
	
#define SMP_UNLOCK_ETH_RECV(__x__) \
	do { \
		lock_owner_rx=-1; \
		spin_unlock_irqrestore(&lock_eth_rx, (__x__)); \
	} while(0)

#define SMP_LOCK_ETH_BUF(__x__)	\
	do { \
		if(lock_owner_buf!=smp_processor_id()) {\
			spin_lock_irqsave(&lock_eth_buf, (__x__)); \
		} else \
			rtlglue_printf("[%s %d] recursion detection in BUF\n",__FUNCTION__,__LINE__); \
		lock_owner_buf=smp_processor_id();\
	} while(0)
	
#define SMP_UNLOCK_ETH_BUF(__x__) \
	do { \
		lock_owner_buf=-1; \
		spin_unlock_irqrestore(&lock_eth_buf, (__x__)); \
	} while(0)

#define SMP_LOCK_ETH_HW(__x__)	\
	do { \
		if(lock_owner_hw!=smp_processor_id()) \
			spin_lock_irqsave(&lock_eth_hw, (__x__)); \
		else \
			rtlglue_printf("[%s %d] recursion detection in HW\n",__FUNCTION__,__LINE__); \
		lock_owner_hw=smp_processor_id();\
	} while(0)
	
#define SMP_UNLOCK_ETH_HW(__x__) \
	do { \
		lock_owner_hw=-1; \
		spin_unlock_irqrestore(&lock_eth_hw, (__x__)); \
	} while(0)

#define SMP_LOCK_ETH_SMI(__x__)	\
	do { \
		if(lock_owner_smi!=smp_processor_id()) \
			spin_lock_irqsave(&lock_eth_smi, (__x__)); \
		else \
			rtlglue_printf("[%s %d] recursion detection in SMI\n",__FUNCTION__,__LINE__); \
		lock_owner_smi=smp_processor_id();\
	} while(0)
	
#define SMP_UNLOCK_ETH_SMI(__x__) \
	do { \
		lock_owner_smi=-1; \
		spin_unlock_irqrestore(&lock_eth_smi, (__x__)); \
	} while(0)	
#endif

#if defined(CONFIG_RTL819X_SPI_FLASH)
extern spinlock_t lock_spi;

#define SMP_LOCK_ETH_CACHE(__x__)		(__x__++) //	spin_lock_irqsave(&lock_spi, (__x__))
#define SMP_UNLOCK_ETH_CACHE(__x__) 		(__x__++) //spin_unlock_irqrestore(&lock_spi, (__x__))
#else
#define SMP_LOCK_ETH_CACHE(x)			(x++)
#define SMP_UNLOCK_ETH_CACHE(x) 		(x++)
#endif

#define SMP_LOCK_ETH2(x)		(x++)
#define SMP_UNLOCK_ETH2(x)		(x++)

#else
#define SMP_LOCK_ETH(x)				local_irq_save(x)
#define SMP_UNLOCK_ETH(x)			local_irq_restore(x)

#define SMP_LOCK_ETH_XMIT(x)		local_irq_save(x)
#define SMP_UNLOCK_ETH_XMIT(x)		local_irq_restore(x)

#define SMP_LOCK_ETH_RECV(x)		local_irq_save(x)
#define SMP_UNLOCK_ETH_RECV(x)		local_irq_restore(x)

#define SMP_LOCK_ETH_BUF(x)		local_irq_save(x)
#define SMP_UNLOCK_ETH_BUF(x)		local_irq_restore(x)

#define SMP_LOCK_ETH_HW(x)			local_irq_save(x)
#define SMP_UNLOCK_ETH_HW(x)		local_irq_restore(x)

#define SMP_LOCK_ETH_SMI(x)			local_irq_save(x)
#define SMP_UNLOCK_ETH_SMI(x)		local_irq_restore(x)

#define SMP_LOCK_ETH2(x)			local_irq_save(x)
#define SMP_UNLOCK_ETH2(x)			local_irq_restore(x)

#endif 

#ifdef __KERNEL__
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,2,0)
#define CONFIG_RTL_PROC_NEW		1
#endif
#endif

#ifdef CONFIG_CPU_LITTLE_ENDIAN
#define _LITTLE_ENDIAN			1
#endif

#ifdef CONFIG_RTL_8197F
#define CONFIG_RTL_SWITCH_NEW_DESCRIPTOR	1
#endif
// move this two definitions from linux_menuconfig to here due to they are too obviously.
#if !defined(CONFIG_RTL_AP_PACKAGE) && !defined(CONFIG_OPENWRT_SDK)
#define REINIT_SWITCH_CORE	1
//#define CONFIG_RTL_CHECK_SWITCH_TX_HANGUP	1

#endif

#ifdef CONFIG_RTL_SWITCH_NEW_DESCRIPTOR
#define RTL_swNic_init(a,b,c,d)		New_swNic_init(a,c,d)
#define RTL_swNic_send(a,b,c,d)		New_swNic_send(a,b,c,d)
#define RTL_swNic_receive(a,b)		New_swNic_receive(a,b)
#define RTL_swNic_txDone(a)			New_swNic_txDone(a)
#define RTL_swNic_freeRxBuf()		New_swNic_freeRxBuf()
#define RTL_dumpRxRing(a)			New_dumpRxRing(a)
#define RTL_dumpTxRing(a)			New_dumpTxRing(a)
#define RTL_dumpMbufRing(a)
#define RTL_swNic_reInit()			New_swNic_reInit()
#define RTL_swNic_freeTxRing()		New_swNic_freeTxRing()
#define RTL_check_tx_own(a)			New_check_tx_done_desc_swCore_own(a)
#else
#define RTL_swNic_init(a,b,c,d)		swNic_init(a,b,c,d)
#define RTL_swNic_send(a,b,c,d)		swNic_send(a,b,c,d)
#define RTL_swNic_receive(a,b)		swNic_receive(a,b)
#define RTL_swNic_txDone(a)			swNic_txDone(a)
#define RTL_swNic_freeRxBuf()		swNic_freeRxBuf()
#define RTL_dumpRxRing(a)			rtl_dumpRxRing(a)
#define RTL_dumpTxRing(a)			rtl_dumpTxRing(a)
#define RTL_dumpMbufRing(a)			rtl_dumpMbufRing(a)
#define RTL_swNic_reInit()			swNic_reInit()
#define RTL_swNic_freeTxRing()		swNic_freeTxRing()
#define RTL_check_tx_own(a)			rtl_check_tx_done_desc_swCore_own(a)
#endif

#if defined(CONFIG_RTL_8198C) || defined(CONFIG_RTL_8197F)
#define RTL_CACHE_WBACK(x,y)		_dma_cache_wback(x,y)
#else
#define RTL_CACHE_WBACK(x,y)		_dma_cache_wback_inv(x,y)
#endif

#if defined(__linux__) && defined(__KERNEL__)

#if LINUX_VERSION_CODE < KERNEL_VERSION(3,0,0)
	#undef CONFIG_RTL_GSO
#endif

#endif

#if defined(CONFIG_RTL_GSO)

		// these features are needed for GSO
		#if defined(CONFIG_RTL_SWITCH_NEW_DESCRIPTOR)
		#define CONFIG_RTL_97F_HW_TX_CSUM            1
		#else
			#if (defined(CONFIG_RTL_819XD) || defined(CONFIG_RTL_8881A)) && defined(CONFIG_IPV6)
			// for 97D/8881A and older IC, they do not support HW L4 csum offload for IPv6+TCP/UDP packet.
			#else
			#define CONFIG_RTL_HW_TX_CSUM 		1
			#endif
		#define CONFIG_RTL_ETH_TX_SG 		1
		#endif

#endif

#if defined(CONFIG_RTL_8197F) && defined(CONFIG_RTL_TSO) && !defined(CONFIG_RTL_8367R_SUPPORT)
	//#define CONFIG_RTL_ADAPTABLE_TSO 
#endif

#if defined(CONFIG_RTL_WTDOG)
#define WATCHDOG_NUM_OF_TIMES 8 
void rtl_periodic_watchdog_kick(unsigned int count, unsigned int times);
#endif

#if !defined(__KERNEL__) 
#define GET_BR_PORT(netdev)	(netdev)->br_port
#else
#if LINUX_VERSION_CODE <= KERNEL_VERSION(2,6,35)
#define GET_BR_PORT(netdev)	(netdev)->br_port
#else
#define GET_BR_PORT(netdev)	br_port_get_rcu(netdev)
#endif
#endif

#if defined(CONFIG_RTL_8197F) && defined(CONFIG_RTL_8211F_SUPPORT)
extern int gpio_simulate_mdc_mdio;
#endif

#endif

