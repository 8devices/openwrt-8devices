/*
 *  Routines for 802.1x hmac
 *
 *  $Id: 1x_kmsm_hmac.c,v 1.1.4.1 2010/10/25 05:34:47 eric Exp $
 *
 *  Copyright (c) 2009 Realtek Semiconductor Corp.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 */

#ifdef __KERNEL__
#include <linux/types.h>
#include <linux/string.h>
#include <linux/ctype.h>
#elif defined(__ECOS)
#include <cyg/hal/plf_intr.h>
#include <cyg/io/eth/rltk/819x/wrapper/sys_support.h>
#include <cyg/io/eth/rltk/819x/wrapper/skbuff.h>
#include <cyg/io/eth/rltk/819x/wrapper/timer.h>
#include <cyg/io/eth/rltk/819x/wrapper/wrapper.h>
#else
#include <string.h>
#endif

#include "./8192cd_cfg.h"

#if defined(INCLUDE_WPA_PSK) || defined(WIFI_HAPD) || defined(RTK_NL80211)

//#define MODULE_TEST

#include "1x_md5c.h"

//sc_yang
//typedef unsigned char *POINTER;
//typedef unsigned long int UINT4;
typedef unsigned char BYTE;

typedef struct
{
	UINT4 digest[ 5 ];            /* Message digest */
	UINT4 countLo, countHi;       /* 64-bit bit count */
	UINT4 data[ 16 ];             /* SHS data buffer */
	int Endianness;
} SHA_CTX;


void SHAInit(SHA_CTX *);
void SHAUpdate(SHA_CTX *, BYTE *buffer, int count);
void SHAFinal(BYTE *output, SHA_CTX *);
void endianTest(int *endianness);
static void SHAtoByte(BYTE *output, UINT4 *input, unsigned int len);

/* The SHS block size and message digest sizes, in bytes */

#define SHS_DATASIZE    64
#define SHS_DIGESTSIZE  20

#ifndef TRUE
  #define FALSE	0
  #define TRUE	( !FALSE )
#endif

/* The SHS f()-functions.  The f1 and f3 functions can be optimized to
   save one boolean operation each - thanks to Rich Schroeppel,
   rcs@cs.arizona.edu for discovering this */

/*#define f1(x,y,z) ( ( x & y ) | ( ~x & z ) )          // Rounds  0-19 */
#define f1(x,y,z)   ( z ^ ( x & ( y ^ z ) ) )           /* Rounds  0-19 */
#define f2(x,y,z)   ( x ^ y ^ z )                       /* Rounds 20-39 */
/*#define f3(x,y,z) ( ( x & y ) | ( x & z ) | ( y & z ) )   // Rounds 40-59 */
#define f3(x,y,z)   ( ( x & y ) | ( z & ( x | y ) ) )   /* Rounds 40-59 */
#define f4(x,y,z)   ( x ^ y ^ z )                       /* Rounds 60-79 */

/* The SHS Mysterious Constants */

#define K1  0x5A827999L                                 /* Rounds  0-19 */
#define K2  0x6ED9EBA1L                                 /* Rounds 20-39 */
#define K3  0x8F1BBCDCL                                 /* Rounds 40-59 */
#define K4  0xCA62C1D6L                                 /* Rounds 60-79 */

/* SHS initial values */

#define h0init  0x67452301L
#define h1init  0xEFCDAB89L
#define h2init  0x98BADCFEL
#define h3init  0x10325476L
#define h4init  0xC3D2E1F0L

/* Note that it may be necessary to add parentheses to these macros if they
   are to be called with expressions as arguments */
/* 32-bit rotate left - kludged with shifts */

#define ROTL(n,X)  ( ( ( X ) << n ) | ( ( X ) >> ( 32 - n ) ) )

/* The initial expanding function.  The hash function is defined over an
   80-UINT2 expanded input array W, where the first 16 are copies of the input
   data, and the remaining 64 are defined by

        W[ i ] = W[ i - 16 ] ^ W[ i - 14 ] ^ W[ i - 8 ] ^ W[ i - 3 ]

   This implementation generates these values on the fly in a circular
   buffer - thanks to Colin Plumb, colin@nyx10.cs.du.edu for this
   optimization.

   The updated SHS changes the expanding function by adding a rotate of 1
   bit.  Thanks to Jim Gillogly, jim@rand.org, and an anonymous contributor
   for this information */

#define expand(W,i) ( W[ i & 15 ] = ROTL( 1, ( W[ i & 15 ] ^ W[ (i - 14) & 15 ] ^ \
                                                 W[ (i - 8) & 15 ] ^ W[ (i - 3) & 15 ] ) ) )


/* The prototype SHS sub-round.  The fundamental sub-round is:

        a' = e + ROTL( 5, a ) + f( b, c, d ) + k + data;
        b' = a;
        c' = ROTL( 30, b );
        d' = c;
        e' = d;

   but this is implemented by unrolling the loop 5 times and renaming the
   variables ( e, a, b, c, d ) = ( a', b', c', d', e' ) each iteration.
   This code is then replicated 20 times for each of the 4 functions, using
   the next 20 values from the W[] array each time */

#define subRound(a, b, c, d, e, f, k, data) \
    ( e += ROTL( 5, a ) + f( b, c, d ) + k + data, b = ROTL( 30, b ) )

/* Initialize the SHS values */

void SHAInit(SHA_CTX *shsInfo)
{
    endianTest(&shsInfo->Endianness);
    /* Set the h-vars to their initial values */
    shsInfo->digest[ 0 ] = h0init;
    shsInfo->digest[ 1 ] = h1init;
    shsInfo->digest[ 2 ] = h2init;
    shsInfo->digest[ 3 ] = h3init;
    shsInfo->digest[ 4 ] = h4init;

    /* Initialise bit count */
    shsInfo->countLo = shsInfo->countHi = 0;
}


/* Perform the SHS transformation.  Note that this code, like MD5, seems to
   break some optimizing compilers due to the complexity of the expressions
   and the size of the basic block.  It may be necessary to split it into
   sections, e.g. based on the four subrounds

   Note that this corrupts the shsInfo->data area */

static void SHSTransform( UINT4 *digest,UINT4 *data )
    {
    UINT4 A, B, C, D, E;     /* Local vars */
    UINT4 eData[ 16 ];       /* Expanded data */

    /* Set up first buffer and local data buffer */
    A = digest[ 0 ];
    B = digest[ 1 ];
    C = digest[ 2 ];
    D = digest[ 3 ];
    E = digest[ 4 ];
    memcpy( (POINTER)eData, (POINTER)data, SHS_DATASIZE );

    /* Heavy mangling, in 4 sub-rounds of 20 interations each. */
    subRound( A, B, C, D, E, f1, K1, eData[  0 ] );
    subRound( E, A, B, C, D, f1, K1, eData[  1 ] );
    subRound( D, E, A, B, C, f1, K1, eData[  2 ] );
    subRound( C, D, E, A, B, f1, K1, eData[  3 ] );
    subRound( B, C, D, E, A, f1, K1, eData[  4 ] );
    subRound( A, B, C, D, E, f1, K1, eData[  5 ] );
    subRound( E, A, B, C, D, f1, K1, eData[  6 ] );
    subRound( D, E, A, B, C, f1, K1, eData[  7 ] );
    subRound( C, D, E, A, B, f1, K1, eData[  8 ] );
    subRound( B, C, D, E, A, f1, K1, eData[  9 ] );
    subRound( A, B, C, D, E, f1, K1, eData[ 10 ] );
    subRound( E, A, B, C, D, f1, K1, eData[ 11 ] );
    subRound( D, E, A, B, C, f1, K1, eData[ 12 ] );
    subRound( C, D, E, A, B, f1, K1, eData[ 13 ] );
    subRound( B, C, D, E, A, f1, K1, eData[ 14 ] );
    subRound( A, B, C, D, E, f1, K1, eData[ 15 ] );
    subRound( E, A, B, C, D, f1, K1, expand( eData, 16 ) );
    subRound( D, E, A, B, C, f1, K1, expand( eData, 17 ) );
    subRound( C, D, E, A, B, f1, K1, expand( eData, 18 ) );
    subRound( B, C, D, E, A, f1, K1, expand( eData, 19 ) );

    subRound( A, B, C, D, E, f2, K2, expand( eData, 20 ) );
    subRound( E, A, B, C, D, f2, K2, expand( eData, 21 ) );
    subRound( D, E, A, B, C, f2, K2, expand( eData, 22 ) );
    subRound( C, D, E, A, B, f2, K2, expand( eData, 23 ) );
    subRound( B, C, D, E, A, f2, K2, expand( eData, 24 ) );
    subRound( A, B, C, D, E, f2, K2, expand( eData, 25 ) );
    subRound( E, A, B, C, D, f2, K2, expand( eData, 26 ) );
    subRound( D, E, A, B, C, f2, K2, expand( eData, 27 ) );
    subRound( C, D, E, A, B, f2, K2, expand( eData, 28 ) );
    subRound( B, C, D, E, A, f2, K2, expand( eData, 29 ) );
    subRound( A, B, C, D, E, f2, K2, expand( eData, 30 ) );
    subRound( E, A, B, C, D, f2, K2, expand( eData, 31 ) );
    subRound( D, E, A, B, C, f2, K2, expand( eData, 32 ) );
    subRound( C, D, E, A, B, f2, K2, expand( eData, 33 ) );
    subRound( B, C, D, E, A, f2, K2, expand( eData, 34 ) );
    subRound( A, B, C, D, E, f2, K2, expand( eData, 35 ) );
    subRound( E, A, B, C, D, f2, K2, expand( eData, 36 ) );
    subRound( D, E, A, B, C, f2, K2, expand( eData, 37 ) );
    subRound( C, D, E, A, B, f2, K2, expand( eData, 38 ) );
    subRound( B, C, D, E, A, f2, K2, expand( eData, 39 ) );

    subRound( A, B, C, D, E, f3, K3, expand( eData, 40 ) );
    subRound( E, A, B, C, D, f3, K3, expand( eData, 41 ) );
    subRound( D, E, A, B, C, f3, K3, expand( eData, 42 ) );
    subRound( C, D, E, A, B, f3, K3, expand( eData, 43 ) );
    subRound( B, C, D, E, A, f3, K3, expand( eData, 44 ) );
    subRound( A, B, C, D, E, f3, K3, expand( eData, 45 ) );
    subRound( E, A, B, C, D, f3, K3, expand( eData, 46 ) );
    subRound( D, E, A, B, C, f3, K3, expand( eData, 47 ) );
    subRound( C, D, E, A, B, f3, K3, expand( eData, 48 ) );
    subRound( B, C, D, E, A, f3, K3, expand( eData, 49 ) );
    subRound( A, B, C, D, E, f3, K3, expand( eData, 50 ) );
    subRound( E, A, B, C, D, f3, K3, expand( eData, 51 ) );
    subRound( D, E, A, B, C, f3, K3, expand( eData, 52 ) );
    subRound( C, D, E, A, B, f3, K3, expand( eData, 53 ) );
    subRound( B, C, D, E, A, f3, K3, expand( eData, 54 ) );
    subRound( A, B, C, D, E, f3, K3, expand( eData, 55 ) );
    subRound( E, A, B, C, D, f3, K3, expand( eData, 56 ) );
    subRound( D, E, A, B, C, f3, K3, expand( eData, 57 ) );
    subRound( C, D, E, A, B, f3, K3, expand( eData, 58 ) );
    subRound( B, C, D, E, A, f3, K3, expand( eData, 59 ) );

    subRound( A, B, C, D, E, f4, K4, expand( eData, 60 ) );
    subRound( E, A, B, C, D, f4, K4, expand( eData, 61 ) );
    subRound( D, E, A, B, C, f4, K4, expand( eData, 62 ) );
    subRound( C, D, E, A, B, f4, K4, expand( eData, 63 ) );
    subRound( B, C, D, E, A, f4, K4, expand( eData, 64 ) );
    subRound( A, B, C, D, E, f4, K4, expand( eData, 65 ) );
    subRound( E, A, B, C, D, f4, K4, expand( eData, 66 ) );
    subRound( D, E, A, B, C, f4, K4, expand( eData, 67 ) );
    subRound( C, D, E, A, B, f4, K4, expand( eData, 68 ) );
    subRound( B, C, D, E, A, f4, K4, expand( eData, 69 ) );
    subRound( A, B, C, D, E, f4, K4, expand( eData, 70 ) );
    subRound( E, A, B, C, D, f4, K4, expand( eData, 71 ) );
    subRound( D, E, A, B, C, f4, K4, expand( eData, 72 ) );
    subRound( C, D, E, A, B, f4, K4, expand( eData, 73 ) );
    subRound( B, C, D, E, A, f4, K4, expand( eData, 74 ) );
    subRound( A, B, C, D, E, f4, K4, expand( eData, 75 ) );
    subRound( E, A, B, C, D, f4, K4, expand( eData, 76 ) );
    subRound( D, E, A, B, C, f4, K4, expand( eData, 77 ) );
    subRound( C, D, E, A, B, f4, K4, expand( eData, 78 ) );
    subRound( B, C, D, E, A, f4, K4, expand( eData, 79 ) );

    /* Build message digest */
    digest[ 0 ] += A;
    digest[ 1 ] += B;
    digest[ 2 ] += C;
    digest[ 3 ] += D;
    digest[ 4 ] += E;
    }

/* When run on a little-endian CPU we need to perform byte reversal on an
   array of long words. */

static void longReverse(UINT4 *buffer, int byteCount, int Endianness )
{
    UINT4 value;

    if (Endianness==(int)TRUE) return;
    byteCount /= sizeof( UINT4 );
    while( byteCount-- )
        {
        value = *buffer;
        value = ( ( value & 0xFF00FF00L ) >> 8  ) | \
                ( ( value & 0x00FF00FFL ) << 8 );
        *buffer++ = ( value << 16 ) | ( value >> 16 );
        }
}

/* Update SHS for a block of data */

void SHAUpdate(SHA_CTX *shsInfo, BYTE *buffer, int count)
{
    UINT4 tmp;
    int dataCount;

    /* Update bitcount */
    tmp = shsInfo->countLo;
    if ( ( shsInfo->countLo = tmp + ( ( UINT4 ) count << 3 ) ) < tmp )
        shsInfo->countHi++;             /* Carry from low to high */
    shsInfo->countHi += count >> 29;

    /* Get count of bytes already in data */
    dataCount = ( int ) ( tmp >> 3 ) & 0x3F;

    /* Handle any leading odd-sized chunks */
    if( dataCount )
        {
        BYTE *p = ( BYTE * ) shsInfo->data + dataCount;

        dataCount = SHS_DATASIZE - dataCount;
        if( count < dataCount )
            {
            memcpy( p, buffer, count );
            return;
            }
        memcpy( p, buffer, dataCount );
        longReverse( shsInfo->data, SHS_DATASIZE, shsInfo->Endianness);
        SHSTransform( shsInfo->digest, shsInfo->data );
        buffer += dataCount;
        count -= dataCount;
        }

    /* Process data in SHS_DATASIZE chunks */
    while( count >= SHS_DATASIZE )
        {
        memcpy( (POINTER)shsInfo->data, (POINTER)buffer, SHS_DATASIZE );
        longReverse( shsInfo->data, SHS_DATASIZE, shsInfo->Endianness );
        SHSTransform( shsInfo->digest, shsInfo->data );
        buffer += SHS_DATASIZE;
        count -= SHS_DATASIZE;
        }

    /* Handle any remaining bytes of data. */
    memcpy( (POINTER)shsInfo->data, (POINTER)buffer, count );
    }

/* Final wrapup - pad to SHS_DATASIZE-byte boundary with the bit pattern
   1 0* (64-bit count of bits processed, MSB-first) */

void SHAFinal(BYTE *output, SHA_CTX *shsInfo)
{
    int count;
    BYTE *dataPtr;

    /* Compute number of bytes mod 64 */
    count = ( int ) shsInfo->countLo;
    count = ( count >> 3 ) & 0x3F;

    /* Set the first char of padding to 0x80.  This is safe since there is
       always at least one byte free */
    dataPtr = ( BYTE * ) shsInfo->data + count;
    *dataPtr++ = 0x80;

    /* Bytes of padding needed to make 64 bytes */
    count = SHS_DATASIZE - 1 - count;

    /* Pad out to 56 mod 64 */
    if( count < 8 )
        {
        /* Two lots of padding:  Pad the first block to 64 bytes */
        memset( dataPtr, 0, count );
        longReverse( shsInfo->data, SHS_DATASIZE, shsInfo->Endianness );
        SHSTransform( shsInfo->digest, shsInfo->data );

        /* Now fill the next block with 56 bytes */
        memset( (POINTER)shsInfo->data, 0, SHS_DATASIZE - 8 );
        }
    else
        /* Pad block to 56 bytes */
        memset( dataPtr, 0, count - 8 );

    /* Append length in bits and transform */
    shsInfo->data[ 14 ] = shsInfo->countHi;
    shsInfo->data[ 15 ] = shsInfo->countLo;

    longReverse( shsInfo->data, SHS_DATASIZE - 8, shsInfo->Endianness );
    SHSTransform( shsInfo->digest, shsInfo->data );

	/* Output to an array of bytes */
	SHAtoByte(output, shsInfo->digest, SHS_DIGESTSIZE);

	/* Zeroise sensitive stuff */
	memset((POINTER)shsInfo, 0, sizeof(shsInfo));
}

static void SHAtoByte(BYTE *output, UINT4 *input, unsigned int len)
{	/* Output SHA digest in byte array */
	unsigned int i, j;

	for(i = 0, j = 0; j < len; i++, j += 4)
	{
        output[j+3] = (BYTE)( input[i]        & 0xff);
        output[j+2] = (BYTE)((input[i] >> 8 ) & 0xff);
        output[j+1] = (BYTE)((input[i] >> 16) & 0xff);
        output[j  ] = (BYTE)((input[i] >> 24) & 0xff);
	}
}


unsigned char digest[20];
unsigned char message[3] = {'a', 'b', 'c' };
unsigned char mess56[] =
	"abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq";

/* Correct solutions from FIPS PUB 180-1 */
char *dig1 = "A9993E36 4706816A BA3E2571 7850C26C 9CD0D89D";
char *dig2 = "84983E44 1C3BD26E BAAE4AA1 F95129E5 E54670F1";
char *dig3 = "34AA973C D4C4DAA4 F61EEB2B DBAD2731 6534016F";

/* Output should look like:-
 a9993e36 4706816a ba3e2571 7850c26c 9cd0d89d
 A9993E36 4706816A BA3E2571 7850C26C 9CD0D89D <= correct
 84983e44 1c3bd26e baae4aa1 f95129e5 e54670f1
 84983E44 1C3BD26E BAAE4AA1 F95129E5 E54670F1 <= correct
 34aa973c d4c4daa4 f61eeb2b dbad2731 6534016f
 34AA973C D4C4DAA4 F61EEB2B DBAD2731 6534016F <= correct
*/


/* endian.c */

void endianTest(int *endian_ness)
{
	if((*(unsigned short *) ("#S") >> 8) == '#')
	{
		/* printf("Big endian = no change\n"); */
		*endian_ness = !(0);
	}
	else
	{
		/* printf("Little endian = swap\n"); */
		*endian_ness = 0;
	}
}

/*main()
{
	SHA_CTX sha;
	int i;
	BYTE big[1000];

	SHAInit(&sha);
	SHAUpdate(&sha, message, 3);
	SHAFinal(digest, &sha);

	for (i = 0; i < 20; i++)
	{
		if ((i % 4) == 0) printf(" ");
		printf("%02x", digest[i]);
	}
	printf("\n");
	printf(" %s <= correct\n", dig1);

	SHAInit(&sha);
	SHAUpdate(&sha, mess56, 56);
	SHAFinal(digest, &sha);

	for (i = 0; i < 20; i++)
	{
		if ((i % 4) == 0) printf(" ");
		printf("%02x", digest[i]);
	}
	printf("\n");
	printf(" %s <= correct\n", dig2);

	// Fill up big array
	for (i = 0; i < 1000; i++)
		big[i] = 'a';

	SHAInit(&sha);
	// Digest 1 million x 'a'
	for (i = 0; i < 1000; i++)
		SHAUpdate(&sha, big, 1000);
	SHAFinal(digest, &sha);

	for (i = 0; i < 20; i++)
	{
		if ((i % 4) == 0) printf(" ");
		printf("%02x", digest[i]);
	}
	printf("\n");
	printf(" %s <= correct\n", dig3);

	return 0;
}*/


//== hmac_sha function ==============================
#ifndef SHA_DIGESTSIZE
#define SHA_DIGESTSIZE  20
#endif

#ifndef SHA_BLOCKSIZE
#define SHA_BLOCKSIZE   64
#endif

#ifndef MD5_DIGESTSIZE
#define MD5_DIGESTSIZE  16
#endif

#ifndef MD5_BLOCKSIZE
#define MD5_BLOCKSIZE   64
#endif



void truncate(
	unsigned char*   d1,   /* data to be truncated */
	unsigned char*   d2,   /* truncated data */
	int     len   /* length in bytes to keep */
	)
{
	int     i ;
	for (i = 0 ; i < len ; i++) d2[i] = d1[i];
}

void hmac_sha(
	unsigned char*    k,     /* secret key */
	int      lk,    /* length of the key in bytes */
	unsigned char*    d,     /* data */
	int      ld,    /* length of data in bytes */
	unsigned char*    out,   /* output buffer, at least "t" bytes */
	int      t
	)
{
	SHA_CTX ictx, octx ;
	unsigned char    isha[SHA_DIGESTSIZE], osha[SHA_DIGESTSIZE] ;
	unsigned char    key[SHA_DIGESTSIZE] ;
	unsigned char    buf[SHA_BLOCKSIZE] ;
	int     i ;

	if (lk > SHA_BLOCKSIZE)
	{
		SHA_CTX         tctx ;

		SHAInit(&tctx) ;
		SHAUpdate(&tctx, k, lk) ;
		SHAFinal(key, &tctx) ;

		k = key ;
		lk = SHA_DIGESTSIZE ;
	}

	/**** Inner Digest ****/

	SHAInit(&ictx) ;

	/* Pad the key for inner digest */
	for (i = 0 ; i < lk ; ++i) buf[i] = k[i] ^ 0x36 ;
	for (i = lk ; i < SHA_BLOCKSIZE ; ++i) buf[i] = 0x36 ;

	SHAUpdate(&ictx, buf, SHA_BLOCKSIZE) ;
	SHAUpdate(&ictx, d, ld) ;

	SHAFinal(isha, &ictx) ;

	/**** Outter Digest ****/

	SHAInit(&octx) ;

	for (i = 0 ; i < lk ; ++i) buf[i] = k[i] ^ 0x5C ;
	for (i = lk ; i < SHA_BLOCKSIZE ; ++i) buf[i] = 0x5C ;

	SHAUpdate(&octx, buf, SHA_BLOCKSIZE) ;
	SHAUpdate(&octx, isha, SHA_DIGESTSIZE) ;

	SHAFinal(osha, &octx) ;

	/* truncate and print the results */
	t = t > SHA_DIGESTSIZE ? SHA_DIGESTSIZE : t ;
	truncate(osha, out, t) ;
	//pr_sha(stdout, out, t) ;
}
//====================================================

/*void main()
{
	unsigned char secret[100]="Jefe";
	unsigned char data[100]="what do ya want for nothing?";
	unsigned char output[1000];
	hmac_sha(secret,4,data,28,output,20);
}*/

////////////////////////////////////////////////////////////////////
//Added by emily
/*
 *  sha1.c
 *
 *  Description:
 *      This file implements the Secure Hashing Algorithm 1 as
 *      defined in FIPS PUB 180-1 published April 17, 1995.
 *
 *      The SHA-1, produces a 160-bit message digest for a given
 *      data stream.  It should take about 2**n steps to find a
 *      message with the same digest as a given message and
 *      2**(n/2) to find any two messages with the same digest,
 *      when n is the digest size in bits.  Therefore, this
 *      algorithm can serve as a means of providing a
 *      "fingerprint" for a message.
 *
 *  Portability Issues:
 *      SHA-1 is defined in terms of 32-bit "words".  This code
 *      uses <stdint.h> (included via "sha1.h" to define 32 and 8
 *      bit unsigned integer types.  If your C compiler does not
 *      support 32 bit unsigned integers, this code is not
 *      appropriate.
 *
 *  Caveats:
 *      SHA-1 is designed to work with messages less than 2^64 bits
 *      long.  Although SHA-1 allows a message digest to be generated
 *      for messages of any number of bits less than 2^64, this
 *      implementation only works with messages with a length that is
 *      a multiple of the size of an 8-bit character.
 *
 */

/*
 *  sha1.h
 *
 *  Description:
 *      This is the header file for code which implements the Secure
 *      Hashing Algorithm 1 as defined in FIPS PUB 180-1 published
 *      April 17, 1995.
 *
 *      Many of the variable names in this code, especially the
 *      single character names, were used because those were the names
 *      used in the publication.
 *
 *      Please read the file sha1.c for more information.
 *
 */

#ifndef _SHA1_H_
#define _SHA1_H_

//#include <stdint.h>
/*
 * If you do not have the ISO standard stdint.h header file, then you
 * must typdef the following:
 *    name              meaning
 *  uint32_t         unsigned 32 bit integer
 *  uint8_t          unsigned 8 bit integer (i.e., unsigned char)
 *  int_least16_t    integer of >= 16 bits
 *
 */

#ifndef _SHA_enum_
#define _SHA_enum_
enum
{
    shaSuccess = 0,
    shaNull,            /* Null pointer parameter */
    shaInputTooLong,    /* input data too long */
    shaStateError       /* called Input after Result */
};
#endif
#define SHA1HashSize 20

#if !defined( __KERNEL__) && !defined(__ECOS) && !defined(__OSK__)
	typedef unsigned int uint32_t;
	typedef unsigned char uint8_t;
#endif
typedef unsigned short	int_least16_t;
/*
 *  This structure will hold context information for the SHA-1
 *  hashing operation
 */
typedef struct SHA1Context
{
    uint32_t Intermediate_Hash[SHA1HashSize/4]; /* Message Digest  */

    uint32_t Length_Low;            /* Message length in bits      */
    uint32_t Length_High;           /* Message length in bits      */

                               /* Index into message block array   */
    int_least16_t Message_Block_Index;
    uint8_t Message_Block[64];      /* 512-bit message blocks      */

    int Computed;               /* Is the digest computed?         */
    int Corrupted;             /* Is the message digest corrupted? */
} SHA1Context;

/*
 *  Function Prototypes
 */


int SHA1Reset(  SHA1Context *);
int SHA1Input(  SHA1Context *,
                const uint8_t *,
                unsigned int);
int SHA1Result( SHA1Context *,
                uint8_t Message_Digest[SHA1HashSize]);

#endif

/*
 *  Define the SHA1 circular left shift macro
 */
#define SHA1CircularShift(bits,word) \
                (((word) << (bits)) | ((word) >> (32-(bits))))

/* Local Function Prototyptes */
void SHA1PadMessage(SHA1Context *);
void SHA1ProcessMessageBlock(SHA1Context *);

/*
 *  SHA1Reset
 *
 *  Description:
 *      This function will initialize the SHA1Context in preparation
 *      for computing a new SHA1 message digest.
 *
 *  Parameters:
 *      context: [in/out]
 *          The context to reset.
 *
 *  Returns:
 *      sha Error Code.
 *
 */
int SHA1Reset(SHA1Context *context)
{
    if (!context)
    {
        return shaNull;
    }

    context->Length_Low             = 0;
    context->Length_High            = 0;
    context->Message_Block_Index    = 0;

    context->Intermediate_Hash[0]   = 0x67452301;
    context->Intermediate_Hash[1]   = 0xEFCDAB89;
    context->Intermediate_Hash[2]   = 0x98BADCFE;
    context->Intermediate_Hash[3]   = 0x10325476;
    context->Intermediate_Hash[4]   = 0xC3D2E1F0;

    context->Computed   = 0;
    context->Corrupted  = 0;

    return shaSuccess;
}

/*
 *  SHA1Result
 *
 *  Description:
 *      This function will return the 160-bit message digest into the
 *      Message_Digest array  provided by the caller.
 *      NOTE: The first octet of hash is stored in the 0th element,
 *            the last octet of hash in the 19th element.
 *
 *  Parameters:
 *      context: [in/out]
 *          The context to use to calculate the SHA-1 hash.
 *      Message_Digest: [out]
 *          Where the digest is returned.
 *
 *  Returns:
 *      sha Error Code.
 *
 */
int SHA1Result( SHA1Context *context,
                uint8_t Message_Digest[SHA1HashSize])
{
    int i;

    if (!context || !Message_Digest)
    {
        return shaNull;
    }

    if (context->Corrupted)
    {
        return context->Corrupted;
    }

    if (!context->Computed)
    {
        SHA1PadMessage(context);
        for(i=0; i<64; ++i)
        {
            /* message may be sensitive, clear it out */
            context->Message_Block[i] = 0;
        }
        context->Length_Low = 0;    /* and clear length */
        context->Length_High = 0;
        context->Computed = 1;

    }

    for(i = 0; i < SHA1HashSize; ++i)
    {
        Message_Digest[i] = context->Intermediate_Hash[i>>2]
                            >> 8 * ( 3 - ( i & 0x03 ) );
    }

    return shaSuccess;
}

/*
 *  SHA1Input
 *
 *  Description:
 *      This function accepts an array of octets as the next portion
 *      of the message.
 *
 *  Parameters:
 *      context: [in/out]
 *          The SHA context to update
 *      message_array: [in]
 *          An array of characters representing the next portion of
 *          the message.
 *      length: [in]
 *          The length of the message in message_array
 *
 *  Returns:
 *      sha Error Code.
 *
 */
int SHA1Input(    SHA1Context    *context,
                  const uint8_t  *message_array,
                  unsigned       length)
{
    if (!length)
    {
        return shaSuccess;
    }

    if (!context || !message_array)
    {
        return shaNull;
    }

    if (context->Computed)
    {
        context->Corrupted = shaStateError;

        return shaStateError;
    }

    if (context->Corrupted)
    {
         return context->Corrupted;
    }
    while(length-- && !context->Corrupted)
    {
    context->Message_Block[context->Message_Block_Index++] =
                    (*message_array & 0xFF);

    context->Length_Low += 8;
    if (context->Length_Low == 0)
    {
        context->Length_High++;
        if (context->Length_High == 0)
        {
            /* Message is too long */
            context->Corrupted = 1;
        }
    }

    if (context->Message_Block_Index == 64)
    {
        SHA1ProcessMessageBlock(context);
    }

    message_array++;
    }

    return shaSuccess;
}

/*
 *  SHA1ProcessMessageBlock
 *
 *  Description:
 *      This function will process the next 512 bits of the message
 *      stored in the Message_Block array.
 *
 *  Parameters:
 *      None.
 *
 *  Returns:
 *      Nothing.
 *
 *  Comments:

 *      Many of the variable names in this code, especially the
 *      single character names, were used because those were the
 *      names used in the publication.
 *
 *
 */
void SHA1ProcessMessageBlock(SHA1Context *context)
{
    const uint32_t K[] =    {       /* Constants defined in SHA-1   */
                            0x5A827999,
                            0x6ED9EBA1,
                            0x8F1BBCDC,
                            0xCA62C1D6
                            };
    int           t;                 /* Loop counter                */
    uint32_t      temp;              /* Temporary word value        */
    uint32_t      W[80];             /* Word sequence               */
    uint32_t      A, B, C, D, E;     /* Word buffers                */

    /*
     *  Initialize the first 16 words in the array W
     */
    for(t = 0; t < 16; t++)
    {
        W[t] = context->Message_Block[t * 4] << 24;
        W[t] |= context->Message_Block[t * 4 + 1] << 16;
        W[t] |= context->Message_Block[t * 4 + 2] << 8;
        W[t] |= context->Message_Block[t * 4 + 3];
    }

    for(t = 16; t < 80; t++)
    {
       W[t] = SHA1CircularShift(1,W[t-3] ^ W[t-8] ^ W[t-14] ^ W[t-16]);
    }

    A = context->Intermediate_Hash[0];
    B = context->Intermediate_Hash[1];
    C = context->Intermediate_Hash[2];
    D = context->Intermediate_Hash[3];
    E = context->Intermediate_Hash[4];

    for(t = 0; t < 20; t++)
    {
        temp =  SHA1CircularShift(5,A) +
                ((B & C) | ((~B) & D)) + E + W[t] + K[0];
        E = D;
        D = C;
        C = SHA1CircularShift(30,B);

        B = A;
        A = temp;
    }

    for(t = 20; t < 40; t++)
    {
        temp = SHA1CircularShift(5,A) + (B ^ C ^ D) + E + W[t] + K[1];
        E = D;
        D = C;
        C = SHA1CircularShift(30,B);
        B = A;
        A = temp;
    }

    for(t = 40; t < 60; t++)
    {
        temp = SHA1CircularShift(5,A) +
               ((B & C) | (B & D) | (C & D)) + E + W[t] + K[2];
        E = D;
        D = C;
        C = SHA1CircularShift(30,B);
        B = A;
        A = temp;
    }

    for(t = 60; t < 80; t++)
    {
        temp = SHA1CircularShift(5,A) + (B ^ C ^ D) + E + W[t] + K[3];
        E = D;
        D = C;
        C = SHA1CircularShift(30,B);
        B = A;
        A = temp;
    }

    context->Intermediate_Hash[0] += A;
    context->Intermediate_Hash[1] += B;
    context->Intermediate_Hash[2] += C;
    context->Intermediate_Hash[3] += D;
    context->Intermediate_Hash[4] += E;

    context->Message_Block_Index = 0;
}

/*
 *  SHA1PadMessage
 *

 *  Description:
 *      According to the standard, the message must be padded to an even
 *      512 bits.  The first padding bit must be a '1'.  The last 64
 *      bits represent the length of the original message.  All bits in
 *      between should be 0.  This function will pad the message
 *      according to those rules by filling the Message_Block array
 *      accordingly.  It will also call the ProcessMessageBlock function
 *      provided appropriately.  When it returns, it can be assumed that
 *      the message digest has been computed.
 *
 *  Parameters:
 *      context: [in/out]
 *          The context to pad
 *      ProcessMessageBlock: [in]
 *          The appropriate SHA*ProcessMessageBlock function
 *  Returns:
 *      Nothing.
 *
 */

void SHA1PadMessage(SHA1Context *context)
{
    /*
     *  Check to see if the current message block is too small to hold
     *  the initial padding bits and length.  If so, we will pad the
     *  block, process it, and then continue padding into a second
     *  block.
     */
    if (context->Message_Block_Index > 55)
    {
        context->Message_Block[context->Message_Block_Index++] = 0x80;
        while(context->Message_Block_Index < 64)
        {
            context->Message_Block[context->Message_Block_Index++] = 0;
        }

        SHA1ProcessMessageBlock(context);

        while(context->Message_Block_Index < 56)
        {
            context->Message_Block[context->Message_Block_Index++] = 0;
        }
    }
    else
    {
        context->Message_Block[context->Message_Block_Index++] = 0x80;
        while(context->Message_Block_Index < 56)
        {

            context->Message_Block[context->Message_Block_Index++] = 0;
        }
    }

    /*
     *  Store the message length as the last 8 octets
     */
    context->Message_Block[56] = context->Length_High >> 24;
    context->Message_Block[57] = context->Length_High >> 16;
    context->Message_Block[58] = context->Length_High >> 8;
    context->Message_Block[59] = (unsigned char)context->Length_High;
    context->Message_Block[60] = context->Length_Low >> 24;
    context->Message_Block[61] = context->Length_Low >> 16;
    context->Message_Block[62] = context->Length_Low >> 8;
    context->Message_Block[63] = (unsigned char)context->Length_Low;

    SHA1ProcessMessageBlock(context);
}

// using a sha1 library
//
void
hmac_sha1(unsigned char *text, int text_len, unsigned char *key,
		 int key_len, unsigned char *digest)
{

		SHA1Context	context;
		unsigned char k[20];
        unsigned char k_ipad[65];    /* inner padding -
                                      * key XORd with ipad
                                      */
        unsigned char k_opad[65];    /* outer padding -
                                      * key XORd with opad
                                      */
        int i;
        /* if key is longer than 64 bytes reset it to key=SHA1(key) */
        if (key_len > 64) {
                SHA1Context      tctx;

                SHA1Reset(&tctx);
                SHA1Input(&tctx, key, key_len);
                SHA1Result(&tctx, k);

				key = k;
                key_len = 20;
        }

        /*
         * the HMAC_SHA1 transform looks like:
         *
         * SHA1(K XOR opad, SHA1(K XOR ipad, text))
         *
         * where K is an n byte key
         * ipad is the byte 0x36 repeated 64 times
         * opad is the byte 0x5c repeated 64 times
         * and text is the data being protected
         */

        /* start out by storing key in pads */
        memset( k_ipad, 0, sizeof k_ipad);
        memset( k_opad, 0, sizeof k_opad);
        memcpy( k_ipad, key, key_len);
        memcpy( k_opad, key, key_len);

        /* XOR key with ipad and opad values */
        for (i=0; i<64; i++) {
                k_ipad[i] ^= 0x36;
                k_opad[i] ^= 0x5c;
        }
        /*
         * perform inner SHA1
         */
        SHA1Reset(&context);                   /* init context for 1st  * pass */
        SHA1Input(&context, k_ipad, 64);     /* start with inner pad  */
        SHA1Input(&context, text, text_len); /* then text of datagram */
        SHA1Result(&context, digest);		/* finish up 1st pass      */
	  /*
         * perform outer SHA1
         */
        SHA1Reset(&context);                   /* init context for 2nd pass */
        SHA1Input(&context, k_opad, 64);     /* start with outer pad   */
        SHA1Input(&context, digest, 20);     /* then results of 1st hash */
        SHA1Result(&context, digest);          /* finish up 2nd pass */


}
//////////////////////////////////////////////////////////////////////////////////////
/*
 *  sha1test.c
 *
 *  Description:
 *      This file will exercise the SHA-1 code performing the three
 *      tests documented in FIPS PUB 180-1 plus one which calls
 *      SHA1Input with an exact multiple of 512 bits, plus a few
 *      error test checks.
 *
 *  Portability Issues:
 *      None.
 *
 */

//#include <stdint.h>
#ifndef __KERNEL__
#ifndef __ECOS
#include <stdio.h>
#endif
#include <string.h>
#endif
//#include "sha1.h"

/*
 *  Define patterns for testing
 */
#define TEST1   "abc"
#define TEST2a  "abcdbcdecdefdefgefghfghighijhi"

#define TEST2b  "jkijkljklmklmnlmnomnopnopq"
#define TEST2   TEST2a TEST2b
#define TEST3   "a"
#define TEST4a  "01234567012345670123456701234567"
#define TEST4b  "01234567012345670123456701234567"
    /* an exact multiple of 512 bits */
#define TEST4   TEST4a TEST4b
char *testarray[4] =
{
    TEST1,
    TEST2,
    TEST3,
    TEST4
};
long int repeatcount[4] = { 1, 1, 1000000, 10 };
char *resultarray[4] =
{
    "A9 99 3E 36 47 06 81 6A BA 3E 25 71 78 50 C2 6C 9C D0 D8 9D",
    "84 98 3E 44 1C 3B D2 6E BA AE 4A A1 F9 51 29 E5 E5 46 70 F1",
    "34 AA 97 3C D4 C4 DA A4 F6 1E EB 2B DB AD 27 31 65 34 01 6F",
    "DE A3 56 A2 CD DD 90 C7 A7 EC ED C5 EB B5 63 93 4F 46 04 52"
};

#ifdef MODULE_TEST
int SHA1main()
{
    SHA1Context sha;
    int i, j, err;
    uint8_t Message_Digest[20];

    /*
     *  Perform SHA-1 tests
     */
    for(j = 0; j < 4; ++j)
    {
        /*
        printf( "\nTest %d: %d, '%s'\n",
                j+1,
                repeatcount[j],
                testarray[j]);
	*/
        err = SHA1Reset(&sha);
        if (err)
        {
            /*fprintf(stderr, "SHA1Reset Error %d.\n", err );*/
            break;    /* out of for j loop */
        }

        for(i = 0; i < repeatcount[j]; ++i)
        {

            err = SHA1Input(&sha,
                  (const unsigned char *) testarray[j],
                  strlen(testarray[j]));
            if (err)
            {
                /*fprintf(stderr, "SHA1Input Error %d.\n", err );*/
                break;    /* out of for i loop */
            }
        }

        err = SHA1Result(&sha, Message_Digest);
        if (err)
        {
            /*fprintf(stderr,
            "SHA1Result Error %d, could not compute message digest.\n",
            err );*/
        }
        else
        {
            /*
            for(i = 0; i < 20 ; ++i)
            {
                printf("%02X ", Message_Digest[i]);
            }
            printf("\n");
            */
        }
        /*
        printf("Should match:\n");
        printf("\t%s\n", resultarray[j]);
        */
    }

    /* Test some error returns */
    err = SHA1Input(&sha,(const unsigned char *) testarray[1], 1);
    /*printf ("\nError %d. Should be %d.\n", err, shaStateError );*/
    err = SHA1Reset(0);
    /*printf ("\nError %d. Should be %d.\n", err, shaNull );*/
    return 0;
}
#endif

////////////////////////////////////////////////
/// hmac_md5
////////////////////////////////////////////////
void
hmac_md5(unsigned char *text, int text_len, unsigned char *key,
		 int key_len, void * digest)
{
        MD5_CTX context;
        unsigned char k_ipad[65];    /* inner padding -
                                      * key XORd with ipad
                                      */
        unsigned char k_opad[65];    /* outer padding -
                                      * key XORd with opad
                                      */
        int i;
        /* if key is longer than 64 bytes reset it to key=MD5(key) */
        if (key_len > 64) {

                MD5_CTX      tctx;

                wlan_MD5_Init(&tctx);
                wlan_MD5_Update(&tctx, key, key_len);
                wlan_MD5_Final(key, &tctx);

                //key = tctx.digest;
                key_len = 16;
        }

        /*
         * the HMAC_MD5 transform looks like:
         *
         * MD5(K XOR opad, MD5(K XOR ipad, text))
         *
         * where K is an n byte key
         * ipad is the byte 0x36 repeated 64 times
         * opad is the byte 0x5c repeated 64 times
         * and text is the data being protected
         */

        /* start out by storing key in pads */
        memset( k_ipad, 0, sizeof k_ipad);
        memset( k_opad, 0, sizeof k_opad);
        memcpy( k_ipad, key, key_len);
        memcpy( k_opad, key, key_len);

        /* XOR key with ipad and opad values */
        for (i=0; i<64; i++) {
                k_ipad[i] ^= 0x36;
                k_opad[i] ^= 0x5c;
        }
        /*
         * perform inner MD5
         */
        wlan_MD5_Init(&context);                   /* init context for 1st * pass */
        wlan_MD5_Update(&context, k_ipad, 64);      /* start with inner pad   */
        wlan_MD5_Update(&context, text, text_len); /* then text of datagram  */
        wlan_MD5_Final((unsigned char*)digest, &context);	/* finish up 1st pass */
		//memcpy(digest, context.digest, 16);
        /*
         * perform outer MD5
         */
        wlan_MD5_Init(&context);                   /* init context for 2nd   * pass */
        wlan_MD5_Update(&context, k_opad, 64);     /* start with outer pad */
        wlan_MD5_Update(&context, digest, 16);     /* then results of 1st  * hash */
        wlan_MD5_Final((unsigned char*)digest, &context);          /* finish up 2nd pass */
		//memcpy(digest, context.digest, 16);
}

#ifdef MODULE_TEST
//Test hmac_md5
// test vectors from rfc2202
unsigned char key0[20] = {
0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b };
unsigned char digest0[16] = {
0x92, 0x94, 0x72, 0x7a, 0x36, 0x38, 0xbb, 0x1c, 0x13, 0xf4, 0x8e, 0xf8, 0x15, 0x8b, 0xfc, 0x9d };
unsigned char digest01[20] = {
0xb6, 0x17, 0x31, 0x86, 0x55, 0x05, 0x72, 0x64, 0xe2, 0x8b, 0xc0, 0xb6, 0xfb, 0x37, 0x8c, 0x8e, 0xf1, 0x46, 0xbe, 0x00 };
unsigned char prf01[] = {
	0xbc, 0xd4, 0xc6, 0x50, 0xb3, 0x0b, 0x96, 0x84, 0x95, 0x18, 0x29, 0xe0, 0xd7, 0x5f, 0x9d, 0x54, 0xb8, 0x62, 0x17, 0x5e, 0xd9, 0xf0, 0x06, 0x06, 0xe1, 0x7d,
	0x8d, 0xa3, 0x54, 0x02, 0xff, 0xee, 0x75, 0xdf, 0x78, 0xc3, 0xd3, 0x1e, 0x0f, 0x88, 0x9f, 0x01, 0x21, 0x20, 0xc0, 0x86, 0x2b, 0xeb, 0x67, 0x75, 0x3e, 0x74,
	0x39, 0xae, 0x24, 0x2e, 0xdb, 0x83, 0x73, 0x69, 0x83, 0x56, 0xcf, 0x5a
};
unsigned char key1[] =         "Jefe";
unsigned char data1[] =        "what do ya want for nothing?";
unsigned char digest1[] =      { 0x75, 0x0c, 0x78, 0x3e, 0x6a, 0xb0, 0xb5, 0x03, 0xea, 0xa8, 0x6e, 0x31, 0x0a, 0x5d, 0xb7, 0x38 };
unsigned char digest11[] = { 0xef, 0xfc, 0xdf, 0x6a, 0xe5, 0xeb, 0x2f, 0xa2, 0xd2, 0x74, 0x16, 0xd5, 0xf1, 0x84, 0xdf, 0x9c, 0x25, 0x9a, 0x7c, 0x79 };
unsigned char prf11[] = {
	0x51, 0xf4, 0xde, 0x5b, 0x33, 0xf2, 0x49, 0xad, 0xf8, 0x1a, 0xeb, 0x71, 0x3a, 0x3c, 0x20, 0xf4, 0xfe, 0x63, 0x14, 0x46, 0xfa, 0xbd, 0xfa, 0x58,
	0x24, 0x47, 0x59, 0xae, 0x58, 0xef, 0x90, 0x09, 0xa9, 0x9a, 0xbf, 0x4e, 0xac, 0x2c,
	0xa5, 0xfa, 0x87, 0xe6, 0x92, 0xc4, 0x40, 0xeb, 0x40, 0x02, 0x3e, 0x7b, 0xab, 0xb2, 0x06, 0xd6, 0x1d, 0xe7, 0xb9, 0x2f, 0x41, 0x52, 0x90, 0x92, 0xb8, 0xfc
};
unsigned char key2[] = { 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA };
unsigned char data2[] = {
	0xDD, 0xDD, 0xDD, 0xDD, 0xDD, 0xDD, 0xDD, 0xDD, 0xDD, 0xDD,
	0xDD, 0xDD, 0xDD, 0xDD, 0xDD, 0xDD, 0xDD, 0xDD, 0xDD, 0xDD,
	0xDD, 0xDD, 0xDD, 0xDD, 0xDD, 0xDD, 0xDD, 0xDD, 0xDD, 0xDD,
	0xDD, 0xDD, 0xDD, 0xDD, 0xDD, 0xDD, 0xDD, 0xDD, 0xDD, 0xDD,
	0xDD, 0xDD, 0xDD, 0xDD, 0xDD, 0xDD, 0xDD, 0xDD, 0xDD, 0xDD
};
unsigned char digest2[] = {
0x56, 0xbe, 0x34, 0x52, 0x1d, 0x14, 0x4c, 0x88, 0xdb, 0xb8, 0xc7, 0x33, 0xf0, 0xe8, 0xb3, 0xf6 };
unsigned char digest21[] = {
0x12, 0x5d, 0x73, 0x42, 0xb9, 0xac, 0x11, 0xcd, 0x91, 0xa3, 0x9a, 0xf4, 0x8a, 0xa1, 0x7b, 0x4f, 0x63, 0xf1, 0x75, 0xd3 };
unsigned char prf21[] = {
0xe1, 0xac, 0x54, 0x6e, 0xc4, 0xcb, 0x63, 0x6f, 0x99, 0x76, 0x48, 0x7b, 0xe5, 0xc8, 0x6b, 0xe1, 0x7a, 0x02, 0x52, 0xca, 0x5d, 0x8d, 0x8d, 0xf1, 0x2c, 0xfb,
0x04, 0x73, 0x52, 0x52, 0x49, 0xce, 0x9d, 0xd8, 0xd1, 0x77, 0xea, 0xd7, 0x10, 0xbc, 0x9b, 0x59, 0x05, 0x47, 0x23, 0x91, 0x07, 0xae, 0xf7, 0xb4, 0xab, 0xd4,
0x3d, 0x87, 0xf0, 0xa6, 0x8f, 0x1c, 0xbd, 0x9e, 0x2b, 0x6f, 0x76, 0x07
};

struct {
	unsigned char *key;
	int key_len;
	unsigned char *data;
	int data_len;
	unsigned char* digest;
	unsigned char* digest1;
	unsigned char* prf;
} tests[] = {
	{ key0,
	16,	// note for SHA1 this is 20 - code sets it to 20
	(unsigned char *)"Hi There",
	8,
	digest0,
	digest01,
	prf01
	},
	{ key1,
	4,
	data1,
	28,
	digest1,
	digest11,
	prf11
	},
	{ key2,
	16,		// sha1 20
	data2,
	50,
	digest2,
	digest21,
	prf21
	}
};

extern void i_PRF(
	unsigned char*	secret,
	int				secret_len,
	unsigned char*	prefix,
	int				prefix_len,
	unsigned char*	random,
	int				random_len,
	unsigned char*  digest,             // caller digest to be filled in
	int				digest_len			// in byte
	);
int _tmain_hmac()
{
	unsigned char digest[20];
	unsigned char output[64+20];
	int c;
	hmac_md5(tests[0].data, tests[0].data_len, tests[0].key, tests[0].key_len, digest);
	c = memcmp(digest, tests[0].digest, 16);
	printf("HMAC_MD5 %02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x %s\n",
		digest[0], digest[1], digest[2], digest[3],
		digest[4], digest[5], digest[6], digest[7],
		digest[8], digest[9], digest[10], digest[11],
		digest[12], digest[13], digest[14], digest[15], !c?"Pass":"Fail");
	hmac_sha1(tests[0].data, tests[0].data_len, tests[0].key, tests[0].key_len+4, digest);
	c = memcmp(digest, tests[0].digest1, 20);	printf("HMAC_SHA1 %02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x %s\n",
		digest[0], digest[1], digest[2], digest[3],
		digest[4], digest[5], digest[6], digest[7],
		digest[8], digest[9], digest[10], digest[11],
		digest[12], digest[13], digest[14], digest[15],
		digest[16], digest[17], digest[18], digest[19], !c?"Pass":"Fail");

	memset(output, 0, 64);
	i_PRF(tests[0].key, tests[0].key_len+4, (unsigned char *)"prefix", 6, tests[0].data, tests[0].data_len, output, 16);
	c = memcmp(output, tests[0].prf, 16);
	printf("PRF %02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x %s\n",
		output[0], output[1], output[2], output[3],
		output[4], output[5], output[6], output[7],
		output[8], output[9], output[10], output[11],
		output[12], output[13], output[14], output[15],
		!c?"Pass":"Fail");

	memset(output, 0, 64);
	i_PRF(tests[0].key, tests[0].key_len+4, (unsigned char *)"prefix", 6, tests[0].data, tests[0].data_len, output, 64);
	c = memcmp(output, tests[0].prf, 64);
	printf("PRF %02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x\
%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x\
%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x\
  %s\n",
		output[0], output[1], output[2], output[3],
		output[4], output[5], output[6], output[7],
		output[8], output[9], output[10], output[11],
		output[12], output[13], output[14], output[15],
		output[16], output[17], output[18], output[19],
		output[20], output[21], output[22], output[23],
		output[24], output[25], output[26], output[27],
		output[28], output[29], output[30], output[31],
		output[32], output[33], output[34], output[35],
		output[36], output[37], output[38], output[39],
		output[40], output[41], output[42], output[43],
		output[44], output[45], output[46], output[47],
		output[48], output[49], output[50], output[51],
		output[52], output[53], output[54], output[55],
		output[56], output[57], output[58], output[59],
		output[60], output[61], output[62], output[63],
		!c?"Pass":"Fail");



	hmac_md5(tests[1].data, tests[1].data_len, tests[1].key, tests[1].key_len, digest);
	c = memcmp(digest, tests[1].digest, 16);
	printf("HMAC_MD5 %02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x %s\n",
		digest[0], digest[1], digest[2], digest[3],
		digest[4], digest[5], digest[6], digest[7],
		digest[8], digest[9], digest[10], digest[11],
		digest[12], digest[13], digest[14], digest[15], !c?"Pass":"Fail");
	hmac_sha1(tests[1].data, tests[1].data_len, tests[1].key, tests[1].key_len, digest);
	c = memcmp(digest, tests[1].digest1, 20);
	printf("HMAC_SHA1 %02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x %s\n",
		digest[0], digest[1], digest[2], digest[3],
		digest[4], digest[5], digest[6], digest[7],
		digest[8], digest[9], digest[10], digest[11],
		digest[12], digest[13], digest[14], digest[15],
		digest[16], digest[17], digest[18], digest[19], !c?"Pass":"Fail");

	memset(output, 0, 64);
	i_PRF(tests[1].key, tests[1].key_len, (unsigned char *)"prefix", 6, tests[1].data, tests[1].data_len, output, 16);
	c = memcmp(output, tests[1].prf, 16);
	printf("PRF %02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x %s\n",
		output[0], output[1], output[2], output[3],
		output[4], output[5], output[6], output[7],
		output[8], output[9], output[10], output[11],
		output[12], output[13], output[14], output[15],
		!c?"Pass":"Fail");

	memset(output, 0, 64);
	i_PRF(tests[1].key, tests[1].key_len, (unsigned char *)"prefix", 6, tests[1].data, tests[1].data_len, output, 64);
	c = memcmp(output, tests[1].prf, 64);
	printf("PRF %02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x\
%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x\
%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x\
  %s\n",
		output[0], output[1], output[2], output[3],
		output[4], output[5], output[6], output[7],
		output[8], output[9], output[10], output[11],
		output[12], output[13], output[14], output[15],
		output[16], output[17], output[18], output[19],
		output[20], output[21], output[22], output[23],
		output[24], output[25], output[26], output[27],
		output[28], output[29], output[30], output[31],
		output[32], output[33], output[34], output[35],
		output[36], output[37], output[38], output[39],
		output[40], output[41], output[42], output[43],
		output[44], output[45], output[46], output[47],
		output[48], output[49], output[50], output[51],
		output[52], output[53], output[54], output[55],
		output[56], output[57], output[58], output[59],
		output[60], output[61], output[62], output[63],
		!c?"Pass":"Fail");


	hmac_md5(tests[2].data, tests[2].data_len, tests[2].key, tests[2].key_len, digest);
	c = memcmp(digest, tests[2].digest, 16);
	printf("HMAC_MD5 %02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x %s\n",
		digest[0], digest[1], digest[2], digest[3],
		digest[4], digest[5], digest[6], digest[7],
		digest[8], digest[9], digest[10], digest[11],
		digest[12], digest[13], digest[14], digest[15], !c?"Pass":"Fail");
	hmac_sha1(tests[2].data, tests[2].data_len, tests[2].key, tests[2].key_len+4, digest);
	c = memcmp(digest, tests[2].digest1, 20);
	printf("HMAC_SHA1 %02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x %s\n",
		digest[0], digest[1], digest[2], digest[3],
		digest[4], digest[5], digest[6], digest[7],
		digest[8], digest[9], digest[10], digest[11],
		digest[12], digest[13], digest[14], digest[15],
		digest[16], digest[17], digest[18], digest[19], !c?"Pass":"Fail");

	memset(output, 0, 64);
	i_PRF(tests[2].key, tests[2].key_len+4, (unsigned char *)"prefix", 6, tests[2].data, tests[2].data_len, output, 16);
	c = memcmp(output, tests[2].prf, 16);
	printf("PRF %02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x %s\n",
		output[0], output[1], output[2], output[3],
		output[4], output[5], output[6], output[7],
		output[8], output[9], output[10], output[11],
		output[12], output[13], output[14], output[15],
		!c?"Pass":"Fail");

	memset(output, 0, 64);
	i_PRF(tests[2].key, tests[2].key_len+4, (unsigned char *)"prefix", 6, tests[2].data, tests[2].data_len, output, 64);
	c = memcmp(output, tests[2].prf, 64);
	printf("PRF %02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x\
%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x\
%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x\
  %s\n",
		output[0], output[1], output[2], output[3],
		output[4], output[5], output[6], output[7],
		output[8], output[9], output[10], output[11],
		output[12], output[13], output[14], output[15],
		output[16], output[17], output[18], output[19],
		output[20], output[21], output[22], output[23],
		output[24], output[25], output[26], output[27],
		output[28], output[29], output[30], output[31],
		output[32], output[33], output[34], output[35],
		output[36], output[37], output[38], output[39],
		output[40], output[41], output[42], output[43],
		output[44], output[45], output[46], output[47],
		output[48], output[49], output[50], output[51],
		output[52], output[53], output[54], output[55],
		output[56], output[57], output[58], output[59],
		output[60], output[61], output[62], output[63],
		!c?"Pass":"Fail");

	/*
	int j;
	char key[80];
	for(j=0;j<80;j++)
		key[j] = 0xaa;
	//hmac_sha1((unsigned char*)"Test Using Larger Than Block-Size Key and Larger Than One Block-Size Data"
	//	, 73, (unsigned char*)key, 80, digest);

	hmac_sha(
	(unsigned char*)    key,
	80,
	(unsigned char*)"Test Using Larger Than Block-Size Key and Larger Than One Block-Size Data",
	73,
	digest,
	20
	);
	*/
	return 0;
}
#endif
#endif // INCLUDE_WPA_PSK
