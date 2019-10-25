#ifndef _HALMAC_2_PLATFORM_H_
#define _HALMAC_2_PLATFORM_H_

/*
* Search The Keyword [Driver]
* Search The Keyword [Driver]
* Search The Keyword [Driver]
*/

/*[Driver] always set BUILD_TEST =0*/
#define BUILD_TEST	0

#if BUILD_TEST
#include "../Platform/App/Test/halmac_2_platformapi.h"
#else
/*[Driver] use thier own header files*/
#include "../Platform/App/VS2010MFC/AutoTest/stdafx.h"
#include "../Header/GeneralDef.h"
#include "halmac_hw_cfg.h"
#endif

/*[Driver] provide the define of _TRUE, _FALSE, NULL, u8, u16, u32*/

#ifndef _TRUE
	#define _TRUE	1
#endif

#ifndef _FALSE
	#define _FALSE	(!_TRUE)
#endif

#ifndef NULL
	#define NULL	((void *)0)
#endif

#define HALMAC_INLINE __inline

typedef unsigned char	u8, *pu8;
typedef unsigned short	u16, *pu16;
typedef unsigned long	u32, *pu32;
typedef char	s8, *ps8;
typedef short	s16, *ps16;
typedef long	s32, *ps32;


#define HALMAC_PLATFORM_LITTLE_ENDIAN	1
#define HALMAC_PLATFORM_BIG_ENDIAN		0

/* Note : Named HALMAC_PLATFORM_LITTLE_ENDIAN / HALMAC_PLATFORM_BIG_ENDIAN
 * is not mandatory. But Little endian must be '1'. Big endian must be '0'
 */
/*[Driver] config the system endian*/
#define HALMAC_SYSTEM_ENDIAN	HALMAC_PLATFORM_LITTLE_ENDIAN

/*[Driver] config if the operating platform*/
#define HALMAC_PLATFORM_WINDOWS	0
#define HALMAC_PLATFORM_LINUX	1
#define HALMAC_PLATFORM_AP		0
/*[Driver] must set HALMAC_PLATFORM_TESTPROGRAM = 0*/
#define HALMAC_PLATFORM_TESTPROGRAM	1

/*[Driver] config if enable the dbg msg or notl*/
#define HALMAC_DBG_MSG_ENABLE	1

/*[Driver] define the Platform SDIO Bus CLK */
#define PLATFORM_SD_CLK	50000000 /*50MHz*/


/*[Driver] provide the type mutex*/
/* Mutex type */
typedef	CRITICAL_SECTION	HALMAC_MUTEX;

#endif/* _HALMAC_2_PLATFORM_H_ */



