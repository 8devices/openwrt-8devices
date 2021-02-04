
/* This is an independent implementation of the encryption algorithm:   */
/*                                                                      */
/*         RIJNDAEL by Joan Daemen and Vincent Rijmen                   */
/*                                                                      */
/* which is a candidate algorithm in the Advanced Encryption Standard   */
/* programme of the US National Institute of Standards and Technology.  */
/*                                                                      */
/* Copyright in this implementation is held by Dr B R Gladman but I     */
/* hereby give permission for its free direct or derivative use subject */
/* to acknowledgment of its origin and compliance with any conditions   */
/* that the originators of the algorithm place on its exploitation.     */
/*                                                                      */
/* Dr Brian Gladman (gladman@seven77.demon.co.uk) 14th January 1999     */

#if 0
#include "stdafx.h"
#include <time.h>
#include <string.h>
#include <assert.h>
#endif

#ifdef __KERNEL__
#include <linux/time.h>
#include <linux/string.h>
#include <linux/slab.h>
#elif defined(__ECOS)
#include <cyg/hal/plf_intr.h>
#include <cyg/io/eth/rltk/819x/wrapper/sys_support.h>
#include <cyg/io/eth/rltk/819x/wrapper/skbuff.h>
#include <cyg/io/eth/rltk/819x/wrapper/timer.h>
#include <cyg/io/eth/rltk/819x/wrapper/wrapper.h>
#endif

#include "./8192cd_cfg.h"
#ifdef __OSK__
#include "./8192cd.h"
#endif

#ifdef INCLUDE_WPA_PSK
#ifdef RTL_WPA2
//#define MODULE_TEST

/*#include "aes_defs.h" */
#ifdef	_SHOW_
#define	_VERBOSE_	1
#elif !defined(_VERBOSE_)
#define	_VERBOSE_	0
#endif

// david, move define to Makefile
// kenny
//#define BIG_ENDIAN 1

#if   defined(__BORLANDC__)	/* show what compiler we used */
#define	COMPILER_ID "Borland"
//#define	LITTLE_ENDIAN	1
// modified by chilong
#define	AUTH_LITTLE_ENDIAN	1
#elif defined(_MSC_VER)
#define	COMPILER_ID "Microsoft"
//#define	LITTLE_ENDIAN	1
// modified by chilong
#define	AUTH_LITTLE_ENDIAN	1
#elif defined(__GNUC__)
#define	COMPILER_ID "GNU"
/* marked by chilong
#ifndef BIG_ENDIAN		// assume gcc = little-endian, unless told otherwise
#define	LITTLE_ENDIAN	1
#endif
*/
// modified by chilong
#ifndef _BIG_ENDIAN_		// assume gcc = little-endian, unless told otherwise
#define	AUTH_LITTLE_ENDIAN	1
#endif
// modified by chilong

#else				/* assume big endian, if compiler is unknown */
#define	COMPILER_ID "Unknown"
#endif

/* 1. Standard types for AES cryptography source code               */

typedef unsigned char   u08b; /* an 8 bit unsigned character type */
typedef unsigned short  u16b; /* a 16 bit unsigned integer type   */
typedef unsigned int	u32b; /* a 32 bit unsigned integer type   */

#ifndef _RTL_WPA_WINDOW
#if !defined(__LINUX_2_6__) && !defined(__ECOS) && !defined(__OSK__)
typedef int bool;
#endif
#endif

/* 2. Standard interface for AES cryptographic routines             */

/* These are all based on 32-bit unsigned values and will therefore */
/* require endian conversions for big-endian architectures          */

#ifdef  __cplusplus
    extern "C"
    {
#endif
    u32b *  AES_SetKey (const u32b in_key[ ], const u32b key_len);
    void    AES_Encrypt(const u32b in_blk[4], u32b out_blk[4]);
    void    AES_Decrypt(const u32b in_blk[4], u32b out_blk[4]);
#ifdef  __cplusplus
    };
#endif

/* 3. Basic macros for speeding up generic operations               */

/* Circular rotate of 32 bit values                                 */

#ifdef _MSC_VER

#include <stdlib.h>
#pragma intrinsic(_lrotr,_lrotl)
#define rotr(x,n) _lrotr(x,n)
#define rotl(x,n) _lrotl(x,n)

#else

#define rotr(x,n)   (((x) >> ((int)(n))) | ((x) << (32 - (int)(n))))
#define rotl(x,n)   (((x) << ((int)(n))) | ((x) >> (32 - (int)(n))))

#endif

/* Extract byte from a 32 bit quantity (little endian notation)     */

#define byte(x,n)   ((u08b)((x) >> (8 * n)))

/* For inverting byte order in input/output 32 bit words, if needed  */

// #ifdef  LITTLE_ENDIAN
// modified by chilong
#ifdef  AUTH_LITTLE_ENDIAN
#define bswap(x)    (x)
#else
#define bswap(x)    ((rotl((x), 8) & 0x00ff00ff) | (rotr((x), 8) & 0xff00ff00))
#endif


//end of aes_def.h

/*------------------ DLW debug code */
#if	_VERBOSE_
#include <stdio.h>
int		rNum;


void ShowBlk(const u32b *b,int final)
	{
	int  i,j;
	u32b x;
	u08b a;

	printf("%s %2d: ",(final) ? "Final" : "Round",rNum++);
	for (i=0;i<4;i++)
		{
		printf(" ");
		x = b[i];			/* always used internally as "little-endian" */
		for (j=0;j<4;j++)
			{
			a = byte(x,j);
			printf(" %02X",a);
			}
		}
	printf("\n");
	}

void ShowKeySched(const u32b *key,int cnt,const char *hdrMsg)
	{
	int  i,j;
	u32b x;
	u08b a;

	printf("%s\n",hdrMsg);
	for (i=0;i<4*cnt;i++)
		{
		x = key[i];		/* key always used as "little-endian" */
		printf(" ");
		for (j=0;j<4;j++)
			{
			a = byte(x,j);
			printf(" %02X",a);
			}
		if ((i%4) == 3) printf("\n");
		}
	}
#define	SetR(r)	{ rNum = r; }
#else
#define	ShowBlk(b,f)
#define	SetR(r)
#define	ShowKeySched(key,cnt,hdrMsg)
#endif
/*---------------- end of DLW debug */


#define LARGE_TABLES

u08b  pow_tab[256];
u08b  log_tab[256];
u08b  sbx_tab[256];
u08b  isb_tab[256];
u32b  rco_tab[ 10];
u32b  ft_tab[4][256];
u32b  it_tab[4][256];

#ifdef  LARGE_TABLES
  u32b  fl_tab[4][256];
  u32b  il_tab[4][256];
#endif

u32b  tab_gen = 0;

u32b  k_len;
u32b  e_key[60];
u32b  d_key[60];

#define ff_mult(a,b)    (a && b ? pow_tab[(log_tab[a] + log_tab[b]) % 255] : 0)

#define f_rn(bo, bi, n, k)                          \
    bo[n] =  ft_tab[0][byte(bi[n],0)] ^             \
             ft_tab[1][byte(bi[(n + 1) & 3],1)] ^   \
             ft_tab[2][byte(bi[(n + 2) & 3],2)] ^   \
             ft_tab[3][byte(bi[(n + 3) & 3],3)] ^ *(k + n)

#define i_rn(bo, bi, n, k)                          \
    bo[n] =  it_tab[0][byte(bi[n],0)] ^             \
             it_tab[1][byte(bi[(n + 3) & 3],1)] ^   \
             it_tab[2][byte(bi[(n + 2) & 3],2)] ^   \
             it_tab[3][byte(bi[(n + 1) & 3],3)] ^ *(k + n)

#ifdef LARGE_TABLES

#define ls_box(x)                \
    ( fl_tab[0][byte(x, 0)] ^    \
      fl_tab[1][byte(x, 1)] ^    \
      fl_tab[2][byte(x, 2)] ^    \
      fl_tab[3][byte(x, 3)] )

#define f_rl(bo, bi, n, k)                          \
    bo[n] =  fl_tab[0][byte(bi[n],0)] ^             \
             fl_tab[1][byte(bi[(n + 1) & 3],1)] ^   \
             fl_tab[2][byte(bi[(n + 2) & 3],2)] ^   \
             fl_tab[3][byte(bi[(n + 3) & 3],3)] ^ *(k + n)

#define i_rl(bo, bi, n, k)                          \
    bo[n] =  il_tab[0][byte(bi[n],0)] ^             \
             il_tab[1][byte(bi[(n + 3) & 3],1)] ^   \
             il_tab[2][byte(bi[(n + 2) & 3],2)] ^   \
             il_tab[3][byte(bi[(n + 1) & 3],3)] ^ *(k + n)

#else

#define ls_box(x)                            \
    ((u32b)sbx_tab[byte(x, 0)] <<  0) ^    \
    ((u32b)sbx_tab[byte(x, 1)] <<  8) ^    \
    ((u32b)sbx_tab[byte(x, 2)] << 16) ^    \
    ((u32b)sbx_tab[byte(x, 3)] << 24)

#define f_rl(bo, bi, n, k)                                      \
    bo[n] = (u32b)sbx_tab[byte(bi[n],0)] ^                    \
        rotl(((u32b)sbx_tab[byte(bi[(n + 1) & 3],1)]),  8) ^  \
        rotl(((u32b)sbx_tab[byte(bi[(n + 2) & 3],2)]), 16) ^  \
        rotl(((u32b)sbx_tab[byte(bi[(n + 3) & 3],3)]), 24) ^ *(k + n)

#define i_rl(bo, bi, n, k)                                      \
    bo[n] = (u32b)isb_tab[byte(bi[n],0)] ^                    \
        rotl(((u32b)isb_tab[byte(bi[(n + 3) & 3],1)]),  8) ^  \
        rotl(((u32b)isb_tab[byte(bi[(n + 2) & 3],2)]), 16) ^  \
        rotl(((u32b)isb_tab[byte(bi[(n + 1) & 3],3)]), 24) ^ *(k + n)

#endif

void gen_tabs(void)
{   u32b  i, t;
    u08b  p, q;

    /* log and power tables for GF(2**8) finite field with  */
    /* 0x11b as modular polynomial - the simplest prmitive  */
    /* root is 0x11, used here to generate the tables       */

    for(i = 0,p = 1; i < 256; ++i)
    {
        pow_tab[i] = (u08b)p; log_tab[p] = (u08b)i;

        p = p ^ (p << 1) ^ (p & 0x80 ? 0x01b : 0);
    }

    log_tab[1] = 0; p = 1;

    for(i = 0; i < 10; ++i)
    {
        rco_tab[i] = p;

        p = (p << 1) ^ (p & 0x80 ? 0x1b : 0);
    }

    /* note that the affine byte transformation matrix in   */
    /* rijndael specification is in big endian format with  */
    /* bit 0 as the most significant bit. In the remainder  */
    /* of the specification the bits are numbered from the  */
    /* least significant end of a byte.                     */

    for(i = 0; i < 256; ++i)
    {
        p = (i ? pow_tab[255 - log_tab[i]] : 0); q = p;
        q = (q >> 7) | (q << 1); p ^= q;
        q = (q >> 7) | (q << 1); p ^= q;
        q = (q >> 7) | (q << 1); p ^= q;
        q = (q >> 7) | (q << 1); p ^= q ^ 0x63;
        sbx_tab[i] = (u08b)p; isb_tab[p] = (u08b)i;
    }

    for(i = 0; i < 256; ++i)
    {
        p = sbx_tab[i];

#ifdef  LARGE_TABLES

        t = p; fl_tab[0][i] = t;
        fl_tab[1][i] = rotl(t,  8);
        fl_tab[2][i] = rotl(t, 16);
        fl_tab[3][i] = rotl(t, 24);
#endif
        t = ((u32b)ff_mult(2, p)) |
            ((u32b)p <<  8) |
            ((u32b)p << 16) |
            ((u32b)ff_mult(3, p) << 24);

        ft_tab[0][i] = t;
        ft_tab[1][i] = rotl(t,  8);
        ft_tab[2][i] = rotl(t, 16);
        ft_tab[3][i] = rotl(t, 24);

        p = isb_tab[i];

#ifdef  LARGE_TABLES

        t = p; il_tab[0][i] = t;
        il_tab[1][i] = rotl(t,  8);
        il_tab[2][i] = rotl(t, 16);
        il_tab[3][i] = rotl(t, 24);
#endif
        t = ((u32b)ff_mult(14, p)) |
            ((u32b)ff_mult( 9, p) <<  8) |
            ((u32b)ff_mult(13, p) << 16) |
            ((u32b)ff_mult(11, p) << 24);

        it_tab[0][i] = t;
        it_tab[1][i] = rotl(t,  8);
        it_tab[2][i] = rotl(t, 16);
        it_tab[3][i] = rotl(t, 24);

#if _VERBOSE_
		if (i<4)	/* helpful for debugging on new platform */
			{		/* (compare with results from known platform) */
			if (i==0)
				printf("%8s   : %08X %08X %08X %08X\n","rco_tab",
					   rco_tab[0],rco_tab[1],rco_tab[2],rco_tab[3]);
#define _ShowTab(tName)	printf("%8s[%d]: %08X %08X %08X %08X\n",#tName,i,	\
				   tName[0][i],tName[1][i],tName[2][i],tName[3][i]);
			_ShowTab(it_tab);
			_ShowTab(ft_tab);
#ifdef LARGE_TABLES
			_ShowTab(il_tab);
			_ShowTab(fl_tab);
#endif
			}
#endif
    }

    tab_gen = 1;
};

#define star_x(x) (((x) & 0x7f7f7f7f) << 1) ^ ((((x) & 0x80808080) >> 7) * 0x1b)

#define imix_col(y,x)       \
    u   = star_x(x);        \
    v   = star_x(u);        \
    w   = star_x(v);        \
    t   = w ^ (x);          \
   (y)  = u ^ v ^ w;        \
   (y) ^= rotr(u ^ t,  8) ^ \
          rotr(v ^ t, 16) ^ \
          rotr(t,24)

/* initialise the key schedule from the user supplied key   */

#define loop4(i)                                    \
{   t = ls_box(rotr(t,  8)) ^ rco_tab[i];           \
    t ^= e_key[4 * i];     e_key[4 * i + 4] = t;    \
    t ^= e_key[4 * i + 1]; e_key[4 * i + 5] = t;    \
    t ^= e_key[4 * i + 2]; e_key[4 * i + 6] = t;    \
    t ^= e_key[4 * i + 3]; e_key[4 * i + 7] = t;    \
}

#define loop6(i)                                    \
{   t = ls_box(rotr(t,  8)) ^ rco_tab[i];           \
    t ^= e_key[6 * i];     e_key[6 * i + 6] = t;    \
    t ^= e_key[6 * i + 1]; e_key[6 * i + 7] = t;    \
    t ^= e_key[6 * i + 2]; e_key[6 * i + 8] = t;    \
    t ^= e_key[6 * i + 3]; e_key[6 * i + 9] = t;    \
    t ^= e_key[6 * i + 4]; e_key[6 * i + 10] = t;   \
    t ^= e_key[6 * i + 5]; e_key[6 * i + 11] = t;   \
}

#define loop8(i)                                    \
{   t = ls_box(rotr(t,  8)) ^ rco_tab[i];           \
    t ^= e_key[8 * i];     e_key[8 * i + 8] = t;    \
    t ^= e_key[8 * i + 1]; e_key[8 * i + 9] = t;    \
    t ^= e_key[8 * i + 2]; e_key[8 * i + 10] = t;   \
    t ^= e_key[8 * i + 3]; e_key[8 * i + 11] = t;   \
    t  = e_key[8 * i + 4] ^ ls_box(t);              \
    e_key[8 * i + 12] = t;                          \
    t ^= e_key[8 * i + 5]; e_key[8 * i + 13] = t;   \
    t ^= e_key[8 * i + 6]; e_key[8 * i + 14] = t;   \
    t ^= e_key[8 * i + 7]; e_key[8 * i + 15] = t;   \
}

u32b *AES_SetKey(const u32b in_key[], const u32b key_len)
{   u32b  i, t, u, v, w;

    if(!tab_gen)
        gen_tabs();

    k_len = (key_len + 31) / 32;

    for (i=0;i<k_len;i++)
		e_key[i] = bswap(in_key[i]);
    t = e_key[k_len-1];

    switch(k_len)
    {
        case 4: for(i = 0; i < 10; ++i)
                    loop4(i);
                break;

        case 6: for(i = 0; i < 8; ++i)
                    loop6(i);
                break;

        case 8: for(i = 0; i < 7; ++i)
                    loop8(i);
                break;
    }

    d_key[0] = e_key[0]; d_key[1] = e_key[1];
    d_key[2] = e_key[2]; d_key[3] = e_key[3];

    for(i = 4; i < 4 * k_len + 24; ++i)
	{
		imix_col(d_key[i], e_key[i]);
    }
	ShowKeySched(e_key,4,"Encryption key schedule:");
	ShowKeySched(d_key,4,"Decryption key schedule:");

    return e_key;
};

/* encrypt a block of text  */
#define f_nround(bo, bi, k) \
    f_rn(bo, bi, 0, k);     \
    f_rn(bo, bi, 1, k);     \
    f_rn(bo, bi, 2, k);     \
    f_rn(bo, bi, 3, k);     \
	ShowBlk(bo,0);			\
    k += 4

#define f_lround(bo, bi, k) \
    f_rl(bo, bi, 0, k);     \
    f_rl(bo, bi, 1, k);     \
    f_rl(bo, bi, 2, k);     \
    f_rl(bo, bi, 3, k);		\
	ShowBlk(bo,1);

void AES_Encrypt(const u32b in_blk[4], u32b out_blk[4])
{   u32b  b0[4], b1[4], *kp;

    b0[0] = bswap(in_blk[0]) ^ e_key[0];
    b0[1] = bswap(in_blk[1]) ^ e_key[1];
    b0[2] = bswap(in_blk[2]) ^ e_key[2];
    b0[3] = bswap(in_blk[3]) ^ e_key[3];

	SetR(1); ShowBlk(b0,0);

    kp = e_key + 4;

    if(k_len > 6)
    {
        f_nround(b1, b0, kp); f_nround(b0, b1, kp);
    }

    if(k_len > 4)
    {
        f_nround(b1, b0, kp); f_nround(b0, b1, kp);
    }

    f_nround(b1, b0, kp); f_nround(b0, b1, kp);
    f_nround(b1, b0, kp); f_nround(b0, b1, kp);
    f_nround(b1, b0, kp); f_nround(b0, b1, kp);
    f_nround(b1, b0, kp); f_nround(b0, b1, kp);
    f_nround(b1, b0, kp); f_lround(b0, b1, kp);

    out_blk[0] = bswap(b0[0]);
    out_blk[1] = bswap(b0[1]);
    out_blk[2] = bswap(b0[2]);
    out_blk[3] = bswap(b0[3]);
};

/* decrypt a block of text  */

#define i_nround(bo, bi, k) \
    i_rn(bo, bi, 0, k);     \
    i_rn(bo, bi, 1, k);     \
    i_rn(bo, bi, 2, k);     \
    i_rn(bo, bi, 3, k);     \
    k -= 4

#define i_lround(bo, bi, k) \
    i_rl(bo, bi, 0, k);     \
    i_rl(bo, bi, 1, k);     \
    i_rl(bo, bi, 2, k);     \
    i_rl(bo, bi, 3, k)

void AES_Decrypt(const u32b in_blk[4], u32b out_blk[4])
{   u32b  b0[4], b1[4], *kp;

    b0[0] = bswap(in_blk[0]) ^ e_key[4 * k_len + 24];
    b0[1] = bswap(in_blk[1]) ^ e_key[4 * k_len + 25];
    b0[2] = bswap(in_blk[2]) ^ e_key[4 * k_len + 26];
    b0[3] = bswap(in_blk[3]) ^ e_key[4 * k_len + 27];

    kp = d_key + 4 * (k_len + 5);

    if(k_len > 6)
    {
        i_nround(b1, b0, kp); i_nround(b0, b1, kp);
    }

    if(k_len > 4)
    {
        i_nround(b1, b0, kp); i_nround(b0, b1, kp);
    }

    i_nround(b1, b0, kp); i_nround(b0, b1, kp);
    i_nround(b1, b0, kp); i_nround(b0, b1, kp);
    i_nround(b1, b0, kp); i_nround(b0, b1, kp);
    i_nround(b1, b0, kp); i_nround(b0, b1, kp);
    i_nround(b1, b0, kp); i_lround(b0, b1, kp);

    out_blk[0] = bswap(b0[0]);
    out_blk[1] = bswap(b0[1]);
    out_blk[2] = bswap(b0[2]);
    out_blk[3] = bswap(b0[3]);
};

enum
    {
    BLK_SIZE    =   16,     // # octets in an AES block
    MAX_PACKET  =   3*512,  // largest packet size
    N_RESERVED  =   0,      // reserved nonce octet value
    A_DATA      =   0x40,   // the Adata bit in the flags
    M_SHIFT     =   3,      // how much to shift the 3-bit M field
    L_SHIFT     =   0,      // how much to shift the 3-bit L field
    L_SIZE      =   2       // size of the l(m) length field (in octets)
    };

typedef union _block                 // AES cipher block
    {
    u32b  x[BLK_SIZE/4];    // access as 8-bit octets or 32-bit words
    u08b  b[BLK_SIZE];
    }block;

typedef struct _packet
    {
    BOOLEAN    encrypted;      // TRUE if encrypted
    u08b    TA[6];          // xmit address
    int     micLength;      // # octets of MIC appended to plaintext (M)
    int     clrCount;       // # cleartext octets covered by MIC
    u32b    pktNum[2];      // unique packet sequence number (like WEP IV)
    block   key;            // the encryption key (K)
    int     length;         // # octets in data[]
    u08b    data[MAX_PACKET+2*BLK_SIZE];    // packet contents
    }packet;


/*
	input : 256 bits, 32 bytes, 4 block for TKIP
			128 bits, 16 bytes, 2 block for CCMP, WRAP, and WEP
		up to 32 block for WPA2
	output: 8 bytes MIC | Wraped key data
*/
#define BLOCKSIZE8	8
void AES_WRAP(u08b * plain, int plain_len,
			  u08b * iv,	int iv_len,
			  u08b * kek,	int kek_len,
			  u08b *cipher, u16b *cipher_len)

{
	int		i, j, k, nblock = plain_len/BLOCKSIZE8;
#ifdef RTL_WPA2
	static	u08b	R[32][BLOCKSIZE8], A[BLOCKSIZE8], xor[BLOCKSIZE8];
#else
	u08b	R[4][BLOCKSIZE8], A[BLOCKSIZE8], xor[BLOCKSIZE8];
#endif

	static packet  p;
	static block   m,x;

	memcpy(&p.key.b , kek, kek_len);
	AES_SetKey(p.key.x, BLK_SIZE*8);     // run the key schedule

	//Initialize Variable
	memcpy(A, iv, BLOCKSIZE8);
	for(i = 0; i < nblock ; i++)
		memcpy(&R[i], plain + i*BLOCKSIZE8, BLOCKSIZE8);

	//Caalculate Intermediate Values

	for(j = 0 ; j < 6 ; j++ )
		for (i = 0 ; i < nblock ; i++)
		{
			memcpy(&m.b, A, BLOCKSIZE8);
			memcpy((&m.b[0]) + BLOCKSIZE8, &(R[i]), BLOCKSIZE8);
			// => B = AES(K, A|R[i])
			AES_Encrypt(m.x,x.x);
			// => A = MSB(64,B) ^t  where t = (n*j) + i
			memset(xor, 0, sizeof xor);
			xor[7] |= ((nblock * j) + i + 1);
			for(k = 0 ; k < 8 ; k++)
				A[k] = x.b[k] ^ xor[k];
			// => R[i] = LSB(64,B)
			for(k = 0 ; k < 8 ; k++)
				R[i][k] = x.b[k + BLOCKSIZE8];

		}

	//Output the result
	memcpy(cipher, A, BLOCKSIZE8);
	for(i = 0; i<nblock ; i++)
		memcpy(cipher + (i+1)*BLOCKSIZE8, &R[i], BLOCKSIZE8);
	*cipher_len = plain_len + BLOCKSIZE8;

}

void AES_UnWRAP(u08b * cipher, int cipher_len,
			    u08b * kek,	int kek_len,
			    u08b * plain)
{

	int		i, j, k, nblock = (cipher_len/BLOCKSIZE8) - 1;
#ifdef RTL_WPA2
	u08b	R[32][BLOCKSIZE8], A[BLOCKSIZE8], xor[BLOCKSIZE8];
#else
	u08b	R[4][BLOCKSIZE8], A[BLOCKSIZE8], xor[BLOCKSIZE8];
#endif
	packet  *p;
	block   m,x;

	p = (packet *)kmalloc(sizeof(packet), GFP_ATOMIC);
	if (p == NULL)
		return;

	memcpy(p->key.b , kek, kek_len);
	AES_SetKey(p->key.x, BLK_SIZE*8);     // run the key schedule

	//Initialize Variable
	memcpy(A, cipher, BLOCKSIZE8);
	for(i = 0; i < nblock ; i++)
		memcpy(&R[i], cipher + (i+1)*BLOCKSIZE8, BLOCKSIZE8);

	//Compute internediate Value
	for(j=5 ; j>=0 ; j--)
		for(i= nblock-1	; i>=0 ; i--)
		{
			// => B = AES-1((A^t) |R[i])

			memset(xor, 0, sizeof xor);
			xor[7] |= ((nblock * j) + i + 1);
			for(k = 0 ; k < 8 ; k++)
				x.b[k] = A[k] ^ xor[k];
			memcpy((&x.b[0]) + BLOCKSIZE8, &(R[i]), BLOCKSIZE8);

			AES_Decrypt(x.x,m.x);

			memcpy(A, &m.b[0], BLOCKSIZE8);
			//for(k=0 ; k<BLOCKSIZE8 ; k++)
			//	A[k] = m.b[k];
			for(k=0 ; k<BLOCKSIZE8 ; k++)
				R[i][k] = m.b[k + BLOCKSIZE8];

		}
	memcpy(plain, A, BLOCKSIZE8);
	for(i = 0; i < nblock ; i++)
		memcpy(plain + (i+1)*BLOCKSIZE8, &R[i],  BLOCKSIZE8);
	kfree(p);
}

#ifdef MODULE_TEST
void TestAESWRAP()
{

	unsigned char kek[] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F};
	unsigned char plaintext[] = {0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF,
								 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

	unsigned char cipher[16+ 8];

/*
	unsigned char iv[] = {0xA6, 0xA6, 0xA6, 0xA6, 0xA6, 0xA6, 0xA6, 0xA6};
	unsigned char plaintext1[] ={0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
						   0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F};
	AES_WRAP(plaintext, 16,
			 iv, 8,
			 kek, 16,
			 cipher);
*/


	AES_UnWRAP(cipher, 24,
			kek, 16,
			plaintext);
}
#endif
#endif // RTL_WPA2
#endif // INCLUDE_WPA_PSK

