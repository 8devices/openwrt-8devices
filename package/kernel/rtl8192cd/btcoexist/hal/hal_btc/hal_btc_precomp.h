/******************************************************************************
 *
 * Copyright(c) 2013 Realtek Corporation. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110, USA
 *
 *
 ******************************************************************************/
#ifndef __HAL_BTC_PRECOMP_H__
#define __HAL_BTC_PRECOMP_H__

#include <8192cd.h>
#include "../hal_btcoexist.h"


#define BT_TMP_BUF_SIZE	100

#ifdef PLATFORM_LINUX
#define rsprintf snprintf
#elif defined(PLATFORM_WINDOWS)
#define rsprintf sprintf_s
#endif

#define DCMD_Printf			DBG_BT_INFO

#define delay_ms(ms)		rtw_mdelay_os(ms)

#ifdef bEnable
#undef bEnable
#endif

#define WPP_SOFTWARE_TRACE 0

typedef enum _BTC_MSG_COMP_TYPE{
	COMP_COEX		= 0,
	COMP_MAX
}BTC_MSG_COMP_TYPE;
extern u4Byte GLBtcDbgType[];

#define DBG_OFF			0
#define DBG_SEC			1
#define DBG_SERIOUS		2
#define DBG_WARNING		3
#define DBG_LOUD		4
#define DBG_TRACE		5


#ifdef CONFIG_BT_COEXIST
#define BT_SUPPORT		1
#define COEX_SUPPORT		1
#define HS_SUPPORT		1
#else
#define BT_SUPPORT		0
#define COEX_SUPPORT		0
#define HS_SUPPORT		0
#endif

#include "./hal_btc_outsrc.h"


#endif // __MP_PRECOMP_H__
