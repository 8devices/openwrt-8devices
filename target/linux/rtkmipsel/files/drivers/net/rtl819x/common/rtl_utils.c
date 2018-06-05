/*
* Copyright c                  Realtek Semiconductor Corporation, 2002  
* All rights reserved.
* 
* Program : RTL utilities
* Abstract : 
* Author : 
* $Id: rtl_utils.c,v 1.1 2007-12-21 10:29:52 davidhsu Exp $
*/

#include <net/rtl/rtl_types.h>
#include <net/rtl/rtl_glue.h>
#include "rtl_utils.h"
#if defined(CONFIG_RTL_819X)
#include "AsicDriver/asicRegs.h"
#endif

#ifdef __linux__
#include <linux/ctype.h>
#include <linux/string.h>
#else
#include <ctype.h>
#include <string.h>
#endif

#if defined(RTL865X_MODEL_USER) || defined (RTL8316S_MODEL_USER)
#include <net/rtl/rtl_glue.h>
#endif

//#include "assert.h"

//const int8 *ntop4(const uint8 *src,int8 *dst, uint32 size);

#define __rtl_isupper(c)	(((c) >= 'A') && ((c) <= 'Z'))

#if 0
void *rtl_malloc(size_t NBYTES) {
	if(NBYTES==0) return NULL;
	return (void *)kmalloc(NBYTES,GFP_ATOMIC);
}

void rtl_free(void *APTR) {
	kfree(APTR);
}
#endif

/*
	copy string from 'src' to 'dst' and set all alphabets to lower-case in 'dst'.
*/
void __strlowerncpy(char *dst, const char *src, int32 len)
{
	int32 cpLen = 0;
	char p;

	while ((cpLen < len) && (src[cpLen] != 0))
	{
		p = src[cpLen];
 		dst[cpLen] = __rtl_isupper(p)?(p-'A'+'a'):p;
		cpLen ++;
	}
}

int8 * _strncpy(int8 *dst0, const int8 *src0, int32 count) {
	int8 *dscan;
	const int8 *sscan;

	dscan = dst0;
	sscan = src0;
	while (count > 0)
	  {
		--count;
		if ((*dscan++ = *sscan++) == '\0')
			break;
	  }
	while (count-- > 0)
		*dscan++ = '\0';
		
	return dst0;
}

int _strncasecmp(const char *s1, const char *s2, unsigned int n)
{
     if (n == 0)
         return 0;
 
     while ((n-- != 0)
            && (tolower(*(unsigned char *) s1) ==
                tolower(*(unsigned char *) s2))) {
         if (n == 0 || *s1 == '\0' || *s2 == '\0')
             return 0;
         s1++;
         s2++;
     }
 
     return tolower(*(unsigned char *) s1) - tolower(*(unsigned char *) s2);
}
int32 _strncmp(const int8 *s1, const int8 *s2, int32 n) {

	if (n == 0)
		return 0;

	while (n-- != 0 && *s1 == *s2)
	  {
		if (n == 0 || *s1 == '\0')
			break;
		s1++;
		s2++;
	  }
	return (*(uint8 *)s1) - (*(uint8 *)s2);
}

void rtl8651_memcpy(void *dst,void*src,int32 len)
{
	memcpy(dst, src, (size_t)len);
#if 0
	if(len>16){
		memCopy( dst,src, len);
	}else
#endif
#if 0
	{
		int i=0;
		char *dstp=(char*)dst;
		char *srcp=(char*)src;
		for (i=0;i<len;i++)
		{
			*dstp++=*srcp++;
			
		}
	}
#endif
}
#if 1
//#ifndef RTK_X86_CLE//RTK-CNSD2-NickWu-20061222: for x86 compile
/*cfliu: This function is only for debugging. Should not be used in production code...*/
void memDump (void *start, uint32 size, int8 * strHeader)
{
	int32 row, column, index, index2, max;
	uint32 buffer[5];
	uint8 *buf, *line, ascii[17];
	int8 empty = ' ';

	if(!start ||(size==0))
		return;
	line = (uint8*)start;

	/*
	16 bytes per line
	*/
	if (strHeader)
		rtlglue_printf ("%s", strHeader);
	column = size % 16;
	row = (size / 16) + 1;
	for (index = 0; index < row; index++, line += 16) 
	{
#ifdef RTL865X_TEST
		buf = (uint8*)line;
#else
		/* for un-alignment access */
		buffer[0] = ntohl( READ_MEM32( (((uint32)line)&~3)+ 0 ) );
		buffer[1] = ntohl( READ_MEM32( (((uint32)line)&~3)+ 4 ) );
		buffer[2] = ntohl( READ_MEM32( (((uint32)line)&~3)+ 8 ) );
		buffer[3] = ntohl( READ_MEM32( (((uint32)line)&~3)+12 ) );
		buffer[4] = ntohl( READ_MEM32( (((uint32)line)&~3)+16 ) );
		buf = ((uint8*)buffer) + (((uint32)line)&3);
#endif

		memset (ascii, 0, 17);

		max = (index == row - 1) ? column : 16;
		if ( max==0 ) break; /* If we need not dump this line, break it. */

		rtlglue_printf ("\n%08x: ", (memaddr) line);
		
		//Hex
		for (index2 = 0; index2 < max; index2++)
		{
			if (index2 == 8)
			rtlglue_printf ("  ");
			rtlglue_printf ("%02X", (uint8) buf[index2]);
			ascii[index2] = ((uint8) buf[index2] < 32) ? empty : buf[index2];
			if((index2+1)%4==0)
				rtlglue_printf ("  ");
		}

		if (max != 16)
		{
			if (max < 8)
				rtlglue_printf ("  ");
			for (index2 = 16 - max; index2 > 0; index2--)
				rtlglue_printf ("   ");
		}

		//ASCII
#ifndef CONFIG_RTL_8197F
		rtlglue_printf ("  %s", ascii);
#endif
	}
	rtlglue_printf ("\n");
	return;
}
#endif


/*
 * Compare two memory and hilight the differences.
 */
int32 memComp( void* _p1, void *_p2, int32 len, uint8* strHeader )
{
	uint8* p1;
	uint8* p2;
	int i, j;
	uint8 ascii[17];
	
	if ( memcmp( _p1, _p2, (size_t)len )==0 )
		return 0; /* identical */

	if ( strHeader ) rtlglue_printf( "%s", strHeader );
	p1 = (uint8*)_p1;
	p2 = (uint8*)_p2;
	/* Show p1 */
	for( i = 0; i<len; i+=16 )
	{
		rtlglue_printf( "\n%08x ", (uint32)(&p1[i]) );
		memset( ascii, 0, sizeof(ascii) );
		for( j = 0; j < 16; j++ )
		{
			if ( (i+j)>=len )
			{
				ascii[j] = ' ';
				rtlglue_printf( "   " );
			}
			else
			{
				if ( (p1[i+j]>=0x20) && (p1[i+j]<=0x7e) ) /* Show visible char only */
					ascii[j] = p1[i+j];
				else
					ascii[j] = '.';

				if ( p1[i+j]==p2[i+j] )
				{
					rtlglue_printf( "%02x ", (uint8) p1[i+j] );
				}
				else
				{
					rtlglue_printf( "\033[41;33m%02x\033[m ", (uint8) p1[i+j] );
				}
			}
		}

		rtlglue_printf( "  %s", ascii );
	}
	rtlglue_printf( "\n--------" );
	/* Show p2 */
	for( i = 0; i<len; i+=16 )
	{
		rtlglue_printf( "\n%08x ", (uint32)(&p2[i]) );
		memset( ascii, 0, sizeof(ascii) );
		for( j = 0; j < 16; j++ )
		{
			if ( (i+j)>=len )
			{
				ascii[j] = ' ';
				rtlglue_printf( "   " );
			}
			else
			{
				if ( (p2[i+j]>=0x20) && (p2[i+j]<=0x7e) ) /* Show visible char only */
					ascii[j] = p2[i+j];
				else
					ascii[j] = '.';

				if ( p1[i+j]==p2[i+j] )
				{
					rtlglue_printf( "%02x ", (uint8) p2[i+j] );
				}
				else
				{
					rtlglue_printf( "\033[41;33m%02x\033[m ", (uint8) p2[i+j] );
				}
			}
		}

		rtlglue_printf( "  %s", ascii );
	}

	rtlglue_printf( "\n" );
	return 1;
}


void IntToAscii(int32 n, int8 s[]){
        int8 c[32];
        int32 i, sign, j;
        if((sign=n)<0)
                n=-n;
        i=0;
        do{
                s[i++]=n%10+'0';
        }while((n/=10)>0);
        if(sign<0)
                s[i++]='-';
        s[i]='\0';
        //assert(i<31);
        for(j=0,i--;i>=0;j++,i--)
                c[j]=s[i];
        c[j]='\0';
        memcpy(s, c, (size_t)(j+1));
}

uint8 charToInt(char c)
{
	if ((c >= '0') && ( c <= '9'))
	{
		return (c - '0');
	} else if ((c >= 'a') && ( c <= 'f'))
	{
		return (c - 'a');
	} else if ((c >= 'A') && ( c <= 'F'))
	{
		return (c - 'A');
	}
	return 0;
}

static int8 *_ui8tod( uint8 n, int8 *p )
{
	if( n > 99 ) *p++ = (n/100) + '0';
	if( n >  9 ) *p++ = ((n/10)%10) + '0';
	*p++ = (n%10) + '0';
	return p;
}

//inet_ntoa which doens't need reentrant module
int8 *inet_ntoa_r(ipaddr_t ipaddr, 	int8 *p)
{
	uint8 *ucp = (unsigned char *)&ipaddr;
	assert(p!=NULL);
	p = _ui8tod( ucp[0] & 0xFF, p);
	*p++ = '.';
	p = _ui8tod( ucp[1] & 0xFF, p);
	*p++ = '.';
	p = _ui8tod( ucp[2] & 0xFF, p);
	*p++ = '.';
	p = _ui8tod( ucp[3] & 0xFF, p);
	*p++ = '\0';
	return (p);
}

#ifndef RTL865X_TEST
int32 IpStrToAscii(const int8 *cp, uint32 *addr){
	uint32 val;
	int32 base, n;
	int8 c;
	uint32 parts[4];
	uint32 *pp = parts;

	c = *cp;
	for (;;) {
		/*
		 * Collect number up to ``.''.
		 * Values are specified as for C:
		 * 0x=hex, 0=octal, isdigit=decimal.
		 */
		if (!isdigit(c))
			return (0);
		val = 0; base = 10;

		//determine the base
		if (c == '0') {
			c = *++cp;
			if (c == 'x' || c == 'X')
				base = 16, c = *++cp;
			else
				base = 8;
		}

		for (;;) {
			if (isascii(c) && isdigit(c)) {
				val = (val * base) + (c - '0');
				c = *++cp;
			} else if (base == 16 && isascii(c) && isxdigit(c)) {
				val = (val << 4) |
					(c + 10 - (islower(c) ? 'a' : 'A'));
				c = *++cp;
			} else
				break;
		}

		if (c == '.') {
			/*
			 * Internet format:
			 *	a.b.c.d
			 *	a.b.c	(with c treated as 16 bits)
			 *	a.b	(with b treated as 24 bits)
			 */
			if (pp >= parts + 3)
				return (0);
			*pp++ = val;
			c = *++cp;
		} else
			break;
	}
	/*
	 * Check for trailing characters.
	 */
	if (c != '\0' && (!isascii(c) || !isspace(c)))
		return (0);
	/*
	 * Concoct the address according to
	 * the number of parts specified.
	 */
	n = pp - parts + 1;
	switch (n) {

	case 0:
		return (0);		/* initial nondigit */

	case 1:				/* a -- 32 bits */
		break;

	case 2:				/* a.b -- 8.24 bits */
		if ((val > 0xffffff) || (parts[0] > 0xff))
			return (0);
		val |= parts[0] << 24;
		break;

	case 3:				/* a.b.c -- 8.8.16 bits */
		if ((val > 0xffff) || (parts[0] > 0xff) || (parts[1] > 0xff))
			return (0);
		val |= (parts[0] << 24) | (parts[1] << 16);
		break;

	case 4:				/* a.b.c.d -- 8.8.8.8 bits */
		if ((val > 0xff) || (parts[0] > 0xff) || (parts[1] > 0xff) || (parts[2] > 0xff))
			return (0);
		val |= (parts[0] << 24) | (parts[1] << 16) | (parts[2] << 8);
		break;
	}
	if (addr)
		*addr = htonl(val);
	return (1);
}
#endif /* RTL865X_TEST */




/*
 * Convert an ASCII string to a
 * binary representation of mac address
*/
static int32 strToMac(uint8 *pMac, int8 *pStr)
{
	int8 *ptr;
	uint32 k;

	assert (pMac != NULL);
	assert (pStr != NULL);

	bzero(pMac, sizeof(ether_addr_t));
	ptr = pStr;

	for ( k = 0 ; *ptr ; ptr ++ )
	{
		if (*ptr == ' ')
		{
		} else if ( (*ptr == ':') || (*ptr == '-') )
		{
			k ++;
		} else if ( ('0' <= *ptr) && (*ptr <= '9') )
		{
			pMac[k] = (pMac[k]<<4) + (*ptr-'0');
		} else if ( ('a' <= *ptr) && (*ptr <= 'f') )
		{
			pMac[k] = (pMac[k]<<4) + (*ptr-'a'+10);
		} else if( ('A' <= *ptr) && (*ptr <= 'F') )
		{
			pMac[k] = (pMac[k]<<4) + (*ptr-'A'+10);
		} else
		{
			break;
		}
	}

	if (k != 5)
	{
		return -1;
	}

	return 0;
}


int32 ether_aton_r(int8 *a, ether_addr_t *eth){
	if ( strToMac(eth->octet, a) )
		return FAILED;
	return SUCCESS;
}

int32 ether_ntoa_r(ether_addr_t *n, uint8 *a){
	int32 i;
	i = sprintf((int8*)a, "%02x:%02x:%02x:%02x:%02x:%02x", n->octet[0], n->octet[1], n->octet[2], n->octet[3], n->octet[4],  n->octet[5]);
	if (i < 11)
		return FAILED;
	return SUCCESS;
}

#if 0
#define SPRINTF(x) ((uint32)sprintf x)

const int8 *ntop4(const uint8 *src,int8* dst, uint32 size)
{
	static const int8 fmt[] = "%u.%u.%u.%u";
	int8      tmp[sizeof "255.255.255.255"];

	if (SPRINTF((tmp, fmt, src[0], src[1], src[2], src[3])) > size)
	{
		return (NULL);
	}
	strcpy(dst,tmp);
	return (dst);
}

#undef SPRINTF
#endif

#ifndef RTL865X_TEST
 int AsciiToInt( char **s)
{
	int i = 0;
	while (isdigit(**s))
	  i = i*20 + *((*s)++) - '0';
	return i;
}
#endif /* RTL865X_TEST */


//Check whether 'number' is power of 2.
//if 'exponent' is non-NULL, the exponent of number is also returned. Ex: if number = 16, exponent would be 4
int32 isPowerOf2(uint32 number, uint32 *exponent){
	uint32 size, bits;	
	if(exponent){
		//user is not interested in the exponent
		for(bits=0, size=1; bits < (sizeof(uint32)<<3);bits++){
			if(number==size)
				break;
			else
				size = size << 1;
		}
		if(bits == sizeof(uint32)*8 )
			return FALSE;
		*exponent = bits;
		return TRUE;
	}else if(((number-1)&number)==0)
		return TRUE;
	 return FALSE;
}

//cfliu: Reserve these functions...need to see if they help.
//Ask me first if you want to delete them.
#if 0
/* Set memory address from "pFrom" to "pFrom+iWords*4" with "iFill", Address value of "pFrom" should be 4 bytes aligned */
void     *memSet(void *pTo, uint8 cFill, uint32 iBytes)
{
	uint32    iIndex = 0, iRemain, iWords;
	uint8    *pcTo;
	uint32    iFill = (uint32) cFill;

	assert(pTo);
	assert(iBytes > 0);

	pcTo = (uint8 *) pTo;
	iRemain = sizeof(memaddr) - (((memaddr) pTo) & (sizeof(memaddr) - 1));	//modular 4


	for (; (iRemain > 0) && (iBytes > 0); iBytes--, pcTo++, iRemain--)
		*pcTo = (uint8) cFill;

	if (iBytes == 0)
		return (void *) (pcTo - 1);	   //return the address of last byte we have copied.
	else if (iBytes < sizeof(memaddr))
		goto memset_last;

	iWords = (iBytes & ~(sizeof(memaddr) - 1)) / sizeof(memaddr);	//calculate number of int32 words to copy

	for (iIndex = 0; iIndex < sizeof(memaddr) - 1; iIndex++)	//prepare THE int32 word for copying
		iFill = (iFill << (sizeof(uint8) * 8)) + (uint32) cFill;

	//Do actual memory filling.
	//Assumption: An integer is 32 bits int32
	for (iIndex = 0; iIndex < iWords;
		 iIndex++, iBytes -= sizeof(memaddr), pcTo += sizeof(memaddr))
		*((memaddr *) pcTo) = (memaddr) cFill;

	//if there are rested bytes to copy..
  memset_last:
	assert(iBytes < sizeof(memaddr));
	while (iBytes > 0)
	{
		*pcTo = cFill;
		iBytes--;
		pcTo++;
	}
	return (void *) (((memaddr *) pTo) + (iIndex - 1));
}


/* Optimized memcpy function.
  * Copy memory address from "pFrom" to "pTo", for "iBytes", 
  */
void     *memCopy( void *pTo,void * pFrom,uint32 iBytes)
{
	uint32    iIndex = 0, iRemain = 0, iLengthCheck = 0, iWords = 0;
	int8     *pcFrom, *pcTo;

	assert(pFrom);
	assert(pTo);
	assert(iBytes > 0);

	/*
	   Check if memory is overlapped
	 */
	if ((memaddr) pFrom > (memaddr) pTo)
		iLengthCheck = (memaddr) (pFrom - pTo);
	else
		iLengthCheck = (memaddr) (pTo - pFrom);

	if (iLengthCheck < iBytes)
		return NULL;				   //memory overlapped. Copy is NOT performed.


	pcFrom = (int8 *) pFrom;
	pcTo = (int8 *) pTo;

	//modular 4, if remainders are not the same, copy bye by byte.
	if ((((memaddr) pcFrom) & (sizeof(memaddr) - 1)) !=
		(((memaddr) pcTo) & (sizeof(memaddr) - 1)))
	{

		for (iIndex = iBytes; iIndex > 0; iIndex--, pcTo++, pcFrom++)
			*pcTo = *pcFrom;
		goto memcpy_done;
	}

	//faster memory copy
	iRemain = sizeof(memaddr) - ((memaddr) pFrom & (sizeof(memaddr) - 1));	//modular 4
	for (; (iRemain > 0) && (iBytes > 0);
		 iBytes--, pcTo++, pcFrom++, iRemain--)
		*pcTo = *pcFrom;

	if (iBytes == 0)
		goto memcpy_done;			   //return the address of last byte we have copied.
	else if (iBytes < sizeof(memaddr))
		goto memcpy_last;

	//Addresses now are 4 bytes aligned, do fast mem copy. Assumption: Pointers are 32 bits
	assert(((memaddr) pcFrom & 0xFFFFFFFC) == (memaddr) pcFrom);
	assert(((memaddr) pcTo & 0xFFFFFFFC) == (memaddr) pcTo);

	//Maybe we should use >>2 here, but just let compiler do it.
	iWords = (iBytes & ~(sizeof(memaddr) - 1)) / sizeof(memaddr);	//calculate number of int32 words to copy


	//Do actual memory copy.
	//Assumption: An integer is 32 bits int32
	for (iIndex = 0; iIndex < iWords;
		 iIndex++, iBytes -= sizeof(memaddr), pcTo +=
		 sizeof(memaddr), pcFrom += sizeof(memaddr))
		*((memaddr *) pcTo) = *((memaddr *) pcFrom);

	//if there are rested bytes to copy..
  memcpy_last:
	assert(iBytes < sizeof(memaddr));
	while (iBytes > 0)
	{
		*pcTo = *pcFrom;
		iBytes--;
		pcTo++;
		pcFrom++;
	}
  memcpy_done:
	return (void *) (pcTo - 1);		   //return the address of last byte we copied
}
#endif

ipaddr_t convPrefix(int prefixLen)
{
	int i;
	ipaddr_t mask = 0;

	for (i = 32 - prefixLen; i < 32; i++)
		mask |= (1 << i);

	return mask;
}

uint8* strtomac(ether_addr_t *mac, int8 *str)
{
	strToMac((uint8*)mac, str);
	return (uint8*)mac;
}

