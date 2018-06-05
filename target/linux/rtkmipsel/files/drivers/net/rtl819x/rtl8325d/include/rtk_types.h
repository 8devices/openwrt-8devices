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
* Purpose :This header defines VERY general macro, structure, enumeration and variable types that 
*              could be used both by applications, APIs and ASIC driver. 
*              Be ware that this file could be released to customer whenever you add new contents.
*
*  Feature : 
*
*/

#ifndef __RTK_TYPES_H__
#define __RTK_TYPES_H__

#ifndef __CONFIG_8051_MODE__
typedef unsigned long long  uint64;
typedef long long   int64;
typedef unsigned int    uint32;
typedef int int32;
typedef unsigned short  uint16;
typedef short  int16;
typedef unsigned char uint8;
typedef char int8;
#define xdata            
#else

typedef unsigned long uint32;
typedef long int32;
typedef unsigned int uint16;
typedef int  int16;
typedef unsigned char uint8;
typedef char int8;
#define xdata           xdata
#endif /* __8051_MODE__*/


typedef int32           rtk_api_ret_t;
typedef uint32          ipaddr_t;

#define ETHER_ADDR_LEN 6
typedef struct ether_addr_s {
    uint8 octet[ETHER_ADDR_LEN];
} ether_addr_t;



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
#define SUCCESS 0
#endif
#ifndef FAILED
#define FAILED -1
#endif

#endif /*__RTK_TYPES_H__*/
