/*
* Copyright c                  Realtek Semiconductor Corporation, 2002  
* All rights reserved.
* 
* Program : Header File for RTL utilities
* Abstract : 
* Author : 
* $Id: rtl_utils.h,v 1.1 2007-12-21 10:29:52 davidhsu Exp $
*/

#ifndef RTL8651_UTILITY_H
#define RTL8651_UTILITY_H

void __strlowerncpy(char *dst, const char *src, int32 len);
int8 *_strncpy(int8 *dst0, const int8 *src0, int32 count);
int _strncasecmp(const char *s1, const char *s2, unsigned int n);
int32 _strncmp(const int8 *s1, const int8 *s2, int32 n);
void rtl8651_memcpy(void *dst,void*src,int32 len);
int32 isPowerOf2(uint32 number, uint32 *exponent);
int32 IpStrToAscii(const int8 *cp, uint32 *addr);
void IntToAscii(int32 n, int8 s[]);
int AsciiToInt( char **s);
uint8 charToInt(char c);
int32 ether_aton_r(int8 *a, ether_addr_t *eth);
int32 ether_ntoa_r(ether_addr_t *n, uint8 *a);
int8 *inet_ntoa_r(ipaddr_t ipaddr, 	int8 *p);
int32 memComp( void* _p1, void *_p2, int32 len, uint8* strHeader );
uint8* strtomac(ether_addr_t *mac, int8 *str);
void memDump (void *start, uint32 size, int8 * strHeader);

#if 0
void *rtl_malloc(uint32);
void rtl_free(void *APTR);
#endif

#ifdef CONFIG_RTL865X_ROMEREAL
	/*Turn on In-memory "ethereal" like pkt sniffing code.*/
	#define START_SNIFFING rtl8651_romerealRecord
#else
	#define START_SNIFFING(x,y) do{}while(0)	
#endif

#ifdef CONFIG_RTL865X_ROMEPERF
	/*Turn on CPU profiling code.*/
	#include "../romeperf.h"
	#define PROFILING_START rtl8651_romeperfEnterPoint
	#define PROFILING_END	rtl8651_romeperfExitPoint
#else
	#define PROFILING_START(x)  do{}while(0)	
	#define PROFILING_END(x) do{}while(0)	
#endif

/*===================================================================
 *  ONE's COMPLEMENT OPERATION
 *  All the operands and return value are 'host-order'.
 *  Please notice the endian problem.
 *===================================================================*/
/* One's Complement ADD */
inline static uint16 OCADD( uint16 a, uint16 b )
{
	uint32 t; /* temp */
	t = ((a)&0xffff)+((b)&0xffff);
	t = (t&0xffff)+(t>>16);
	return (uint16)t;
}

/* One's Complement NEGtive */
inline static uint16 OCNEG( uint16 a )
{
	return (~a)&0xffff;
}

/* One's Complement SUBtract */
inline static uint16 OCSUB( uint16 a, uint16 b )
{
	return OCADD( a, OCNEG(b) );
}

ipaddr_t convPrefix(int prefixLen);

#if 0
#define TBL_MEM_ALLOC(tbl, type, size)	 \
	{ \
	(tbl) = (type *)rtl_malloc((size) * sizeof(type)); \
		if(!(tbl)){\
			rtlglue_printf("MEM alloc failed at line %d\n", __LINE__);\
			while(1);\
			return FAILED;\
		}\
	}

#if defined(RTL865X_MODEL_USER)
	/* User mode, compiler use STDINC, said that, bezro is defined. */
#else
	/* Not user mode, we need define. */
	#ifndef bzero
		#define bzero( p, s ) memset( p, 0, s )
	#endif
#endif
#endif

#endif /* RTL8651_UTILITY_H */



