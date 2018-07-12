#ifndef _HALMAC_2_PLATFORM_H_
#define _HALMAC_2_PLATFORM_H_

/*==========================================================
					Search The Keyword [Driver]
					Search The Keyword [Driver] 					
					Search The Keyword [Driver] 
===========================================================*/


#define BUILD_TEST 0/*[Driver] always set BUILD_TEST =0*/

#if BUILD_TEST
//#include "../Platform/App/HalTestBuild/halmac_2_platformapi.h"
#include "../Platform/App/Test/halmac_2_platformapi.h"
#else
/*[Driver] use thier own header files*/
//#include "../Platform/App/VS2010MFC/AutoTest/stdafx.h"
//#include "../Header/GeneralDef.h"
#include "../../8192cd_cfg.h" //eric-8812b
#include "halmac_hw_cfg.h"
#ifndef __ECOS
#include <asm/io.h>
#include <linux/spinlock.h>
#else
#include <cyg/io/eth/rltk/819x/wrapper/sys_support.h>
#endif
#include "Hal88XXDesc.h"

#endif

/****************************************************/
/*Define the Platform related Macro*/
/***************************************************/

/*[Driver] provide the define of _TRUE, _FALSE, NULL, u8, u16, u32*/

#ifndef _TRUE
	#define _TRUE		1
#endif

#ifndef _FALSE
	#define _FALSE		(!_TRUE)
#endif

#ifndef NULL
	#define NULL		((void *)0)
#endif

#if 0
typedef unsigned char           	u8;
typedef unsigned short          	u16;
typedef unsigned long           	u32;
typedef char				s8;
typedef short				s16;
typedef long				s32;
#endif 

typedef u8 *pu8;
typedef u16 *pu16;
typedef u32 *pu32;
typedef s8 *ps8;
typedef s16 *ps16;
typedef s32 *ps32;

#define HALMAC_INLINE __inline

//eric-8822 ??
#define HALMAC_PLATFORM_LITTLE_ENDIAN                  1
#define HALMAC_PLATFORM_BIG_ENDIAN                     0

/* Note : Named HALMAC_PLATFORM_LITTLE_ENDIAN / HALMAC_PLATFORM_BIG_ENDIAN is not mandatory. 
		  But Little endian must be '1'. Big endian must be '0'. */
/*[Driver] config the system endian*/

#ifdef _LITTLE_ENDIAN_
#undef HALMAC_SYSTEM_ENDIAN
#define HALMAC_SYSTEM_ENDIAN                           HALMAC_PLATFORM_LITTLE_ENDIAN
#endif

#ifdef _BIG_ENDIAN_
#undef HALMAC_SYSTEM_ENDIAN
#define HALMAC_SYSTEM_ENDIAN                           HALMAC_PLATFORM_BIG_ENDIAN
#endif


/*[Driver] config if the operating platform*/
#define HALMAC_PLATFORM_WINDOWS   0
#define HALMAC_PLATFORM_LINUX		  0
#define HALMAC_PLATFORM_AP		  1
#define HALMAC_PLATFORM_TESTPROGRAM       0 /*[Driver] must set HALMAC_PLATFORM_TESTPROGRAM =0*/

/*[Driver] config if enable the dbg msg or notl*/
#define HALMAC_DBG_MSG_ENABLE		0//1

/*[Driver] config if it support signal*/
#define PLATFORM_SUPPORT_SIGNAL	0

/*[Driver] provide the type of signal and mutex*/
#if PLATFORM_SUPPORT_SIGNAL
/* Signal type */
typedef	CEvent									    				HALMAC_SIGNAL;
#endif
/* Mutex type */
//typedef	CRITICAL_SECTION											HALMAC_MUTEX;
#ifndef __ECOS
typedef	spinlock_t											HALMAC_MUTEX;
#else
typedef	int													HALMAC_MUTEX;
#endif

#define HALMAC_DBG_MSG_LEVEL	HALMAC_DBG_ERR

/*[Driver] define the Platform SDIO Bus CLK */
#define PLATFORM_SD_CLK	50000000 /*50MHz*/

#endif//_HALMAC_2_PLATFORM_H_



