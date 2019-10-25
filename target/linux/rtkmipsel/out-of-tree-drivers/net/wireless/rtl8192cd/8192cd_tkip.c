/*
 *  Software TKIP encryption/descryption routines
 *
 *  $Id: 8192cd_tkip.c,v 1.4.4.2 2010/09/30 05:27:28 button Exp $
 *
 *  Copyright (c) 2009 Realtek Semiconductor Corp.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 */

#define _8192CD_TKIP_C_

#ifdef __KERNEL__
#include <linux/module.h>
#include <asm/byteorder.h>
#elif defined(__ECOS)
#include <cyg/io/eth/rltk/819x/wrapper/sys_support.h>
#include <cyg/io/eth/rltk/819x/wrapper/skbuff.h>
#include <cyg/io/eth/rltk/819x/wrapper/timer.h>
#include <cyg/io/eth/rltk/819x/wrapper/wrapper.h>
#endif

#include "./8192cd_cfg.h"

#if !defined(__KERNEL__) && !defined(__ECOS)
#include "./sys-support.h"
#endif

#include "./8192cd.h"
#if defined(__KERNEL__) || defined(__OSK__)
#include "./ieee802_mib.h"
#elif defined(__ECOS)
#include <cyg/io/eth/rltk/819x/wlan/ieee802_mib.h>
#endif
#include "./8192cd_util.h"
#include "./8192cd_headers.h"
#include "./8192cd_debug.h"

#ifdef __LINUX_2_6__
/*avoid mutli defined*/
#ifdef swap
#undef swap
#endif
#endif

#define MAX_MESSAGE_LENGTH 2048

#define rotr(n,x)   (((x) >> ((int)(n))) | ((x) << (32 - (int)(n))))
#define rotl(n,x)   (((x) << ((int)(n))) | ((x) >> (32 - (int)(n))))

#define block_function(l,r) { \
		r = r ^ rotl(17,l); \
		l = (l + r); \
		r ^= ((l & 0xff00ff00) >> 8) | ((l & 0x00ff00ff) << 8); \
		l = (l + r); \
		r = r ^ rotl(3,l); \
		l = (l + r); \
		r = r ^ rotr(2,l); \
		l = (l + r); \
}

void debug_out(unsigned char *label, unsigned char *data, int data_length)
{
    int i,j;
    int num_blocks;
    int block_remainder;

    num_blocks = data_length >> 4;
    block_remainder = data_length & 15;

	if (label) {
	    printk("%s\n", label);
	}

	if (data==NULL || data_length==0)
		return;

    for (i=0; i<num_blocks; i++)
    {
        printk("\t");
        for (j=0; j<16; j++)
        {
            printk("%02x ", data[j + (i<<4)]);
        }
        printk("\n");
    }

    if (block_remainder > 0)
    {
        printk("\t");
        for (j=0; j<block_remainder; j++)
        {
            printk("%02x ", data[j+(num_blocks<<4)]);
        }
        printk("\n");
    }
}


#if 0
static __inline__ unsigned long int rotr(int bits, unsigned long int a)
{
	unsigned long int c,d,e,f,g;
    c = (0x0001 << bits)-1;
    d = ~c;

    e = (a & d) >> bits;
    f = (a & c) << (32 - bits);

    g = e | f;

    return (g & 0xffffffff );
}


static __inline__ unsigned long int rotl(int bits, unsigned long int a)
{
	unsigned long int c,d,e,f,g;
    c = (0x0001 << (32-bits))-1;
    d = ~c;

    e = (a & c) << bits;
    f = (a & d) >> (32 - bits);

    g = e | f;

    return (g & 0xffffffff );
}


static __inline__ unsigned long int xswap(unsigned long int in)
{
	unsigned long int a,b,c,d,out;

    a = in & 0xff;
    b = (in >> 8) & 0xff;
    c = (in >> 16) & 0xff;
    d = (in >> 24) & 0xff;

    out = 0;
    out |= c << 24;
    out |= d << 16;
    out |= a << 8;
    out |= b;

    return out;
}


static __inline__ void block_function(
                    unsigned long int l,
                    unsigned long int r,
                    unsigned long int *l_out,
                    unsigned long int *r_out)
{
    r = r ^ rotl(17,l);
    l = (l + r);
    r = r ^ xswap(l);
    l = (l + r);
    r = r ^ rotl(3,l);
    l = (l + r);
    r = r ^ rotr(2,l);
	l = (l + r);
    *l_out = l;
    *r_out = r;
}
#endif


void michael(
                struct rtl8192cd_priv *priv,
                unsigned char *key,
                unsigned char *hdr,
                unsigned char *llc,
                unsigned char *message,
                int message_length,
                unsigned char *mic, int tx)
{
//    unsigned long int l_out, r_out;
    register unsigned int l,r;
    register unsigned int k0,k1;
    register unsigned int m;
    register unsigned char a,b,c,d;

    int num_blocks;
    int block;

    num_blocks = message_length >> 2;

    k0 = key[0] + (key[1] << 8) + (key[2] << 16) + (key[3] << 24);
    k1 = key[4] + (key[5] << 8) + (key[6] << 16) + (key[7] << 24);

    l = k0;
    r = k1;

    //in the beginning, perform the first 16 bytes from hdr...
	if (hdr)
	{
		for (block = 0; block < 4; block++)
		{
			a = hdr[block<<2];
			b = hdr[1+(block<<2)];
			c = hdr[2+(block<<2)];
			d = hdr[3+(block<<2)];
			m = a + (b << 8) + (c << 16) + (d << 24);
			l = l ^ m;
#if 0
			block_function(l,r,&l_out,&r_out);
			l = l_out;
			r = r_out;
#endif
			block_function(l,r);
		}

		num_blocks -= 4;
	}

	if (llc)
	{
		for (block = 0; block < 2; block++)
		{
			a = llc[block<<2];
			b = llc[1+(block<<2)];
			c = llc[2+(block<<2)];
			d = llc[3+(block<<2)];
			m = a + (b << 8) + (c << 16) + (d << 24);
			l = l ^ m;
#if 0
			block_function(l,r,&l_out,&r_out);
			l = l_out;
			r = r_out;
#endif
			block_function(l,r);
		}

		num_blocks -= 2;
	}

#ifndef NOT_RTK_BSP
	if ((priv->pshare->have_hw_mic) &&
		!(priv->pmib->dot11StationConfigEntry.swTkipMic))
	{
		if (tx)
			rtl_cache_sync_wback(priv, (unsigned long)message, (num_blocks*4), PCI_DMA_TODEVICE);
		
		*(volatile unsigned int *)GDMACNR  = 0;
		*(volatile unsigned int *)GDMACNR  = GDMA_ENABLE;
		*(volatile unsigned int *)GDMAIMR  = 0;
		*(volatile unsigned int *)GDMAISR  = 0xffffffff;
		*(volatile unsigned int *)GDMAICVL = (l);
		*(volatile unsigned int *)GDMAICVR = (r);
		*(volatile unsigned int *)GDMASBP0 = virt_to_bus(message)+CONFIG_LUNA_SLAVE_PHYMEM_OFFSET;
		*(volatile unsigned int *)GDMASBL0 = GDMA_LDB|(num_blocks*4);
		*(volatile unsigned int *)GDMADBP0 = 0;
		*(volatile unsigned int *)GDMADBL0 = 0;
		*(volatile unsigned int *)GDMACNR  = GDMA_ENABLE|GDMA_POLL|GDMA_MIC|internalUsedGDMACNR;
	}
	else
#endif // NOT_RTK_BSP
	{
		for (block = 0; block < num_blocks; block++)
		{
			a = message[block<<2];
			b = message[1+(block<<2)];
			c = message[2+(block<<2)];
			d = message[3+(block<<2)];
			m = a + (b << 8) + (c << 16) + (d << 24);
			l = l ^ m;
#if 0
			block_function(l,r,&l_out,&r_out);
			l = l_out;
			r = r_out;
#endif
			block_function(l,r);
		}

		mic[0] = (unsigned char)(l & 0xff);
		mic[1] = (unsigned char)((l >> 8) & 0xff);
		mic[2] = (unsigned char)((l >> 16) & 0xff);
		mic[3] = (unsigned char)((l >> 24) & 0xff);
		mic[4] = (unsigned char)(r & 0xff);
		mic[5] = (unsigned char)((r >> 8) & 0xff);
		mic[6] = (unsigned char)((r >> 16) & 0xff);
		mic[7] = (unsigned char)((r >> 24) & 0xff);
	}
}


/*-----------------------------------------------------------------------------
					below is for key mixing function
------------------------------------------------------------------------------*/

/* The Sbox can be reduced to two 16 bit wide tables, each with 256 entries.*/
/* The second table is the same as the first but with the upper and lower   */
/* bytes swapped. To allow an endian tolerant implementation, the byte      */
/* halves have been expressed independently here.                           */
unsigned char Tkip_Sbox_Lower[256] =
{
    0xA5,0x84,0x99,0x8D,0x0D,0xBD,0xB1,0x54,
    0x50,0x03,0xA9,0x7D,0x19,0x62,0xE6,0x9A,
    0x45,0x9D,0x40,0x87,0x15,0xEB,0xC9,0x0B,
    0xEC,0x67,0xFD,0xEA,0xBF,0xF7,0x96,0x5B,
    0xC2,0x1C,0xAE,0x6A,0x5A,0x41,0x02,0x4F,
    0x5C,0xF4,0x34,0x08,0x93,0x73,0x53,0x3F,
    0x0C,0x52,0x65,0x5E,0x28,0xA1,0x0F,0xB5,
    0x09,0x36,0x9B,0x3D,0x26,0x69,0xCD,0x9F,
    0x1B,0x9E,0x74,0x2E,0x2D,0xB2,0xEE,0xFB,
    0xF6,0x4D,0x61,0xCE,0x7B,0x3E,0x71,0x97,
    0xF5,0x68,0x00,0x2C,0x60,0x1F,0xC8,0xED,
    0xBE,0x46,0xD9,0x4B,0xDE,0xD4,0xE8,0x4A,
    0x6B,0x2A,0xE5,0x16,0xC5,0xD7,0x55,0x94,
    0xCF,0x10,0x06,0x81,0xF0,0x44,0xBA,0xE3,
    0xF3,0xFE,0xC0,0x8A,0xAD,0xBC,0x48,0x04,
    0xDF,0xC1,0x75,0x63,0x30,0x1A,0x0E,0x6D,
    0x4C,0x14,0x35,0x2F,0xE1,0xA2,0xCC,0x39,
    0x57,0xF2,0x82,0x47,0xAC,0xE7,0x2B,0x95,
    0xA0,0x98,0xD1,0x7F,0x66,0x7E,0xAB,0x83,
    0xCA,0x29,0xD3,0x3C,0x79,0xE2,0x1D,0x76,
    0x3B,0x56,0x4E,0x1E,0xDB,0x0A,0x6C,0xE4,
    0x5D,0x6E,0xEF,0xA6,0xA8,0xA4,0x37,0x8B,
    0x32,0x43,0x59,0xB7,0x8C,0x64,0xD2,0xE0,
    0xB4,0xFA,0x07,0x25,0xAF,0x8E,0xE9,0x18,
    0xD5,0x88,0x6F,0x72,0x24,0xF1,0xC7,0x51,
    0x23,0x7C,0x9C,0x21,0xDD,0xDC,0x86,0x85,
    0x90,0x42,0xC4,0xAA,0xD8,0x05,0x01,0x12,
    0xA3,0x5F,0xF9,0xD0,0x91,0x58,0x27,0xB9,
    0x38,0x13,0xB3,0x33,0xBB,0x70,0x89,0xA7,
    0xB6,0x22,0x92,0x20,0x49,0xFF,0x78,0x7A,
    0x8F,0xF8,0x80,0x17,0xDA,0x31,0xC6,0xB8,
    0xC3,0xB0,0x77,0x11,0xCB,0xFC,0xD6,0x3A
};

unsigned char Tkip_Sbox_Upper[256] =
{
    0xC6,0xF8,0xEE,0xF6,0xFF,0xD6,0xDE,0x91,
    0x60,0x02,0xCE,0x56,0xE7,0xB5,0x4D,0xEC,
    0x8F,0x1F,0x89,0xFA,0xEF,0xB2,0x8E,0xFB,
    0x41,0xB3,0x5F,0x45,0x23,0x53,0xE4,0x9B,
    0x75,0xE1,0x3D,0x4C,0x6C,0x7E,0xF5,0x83,
    0x68,0x51,0xD1,0xF9,0xE2,0xAB,0x62,0x2A,
    0x08,0x95,0x46,0x9D,0x30,0x37,0x0A,0x2F,
    0x0E,0x24,0x1B,0xDF,0xCD,0x4E,0x7F,0xEA,
    0x12,0x1D,0x58,0x34,0x36,0xDC,0xB4,0x5B,
    0xA4,0x76,0xB7,0x7D,0x52,0xDD,0x5E,0x13,
    0xA6,0xB9,0x00,0xC1,0x40,0xE3,0x79,0xB6,
    0xD4,0x8D,0x67,0x72,0x94,0x98,0xB0,0x85,
    0xBB,0xC5,0x4F,0xED,0x86,0x9A,0x66,0x11,
    0x8A,0xE9,0x04,0xFE,0xA0,0x78,0x25,0x4B,
    0xA2,0x5D,0x80,0x05,0x3F,0x21,0x70,0xF1,
    0x63,0x77,0xAF,0x42,0x20,0xE5,0xFD,0xBF,
    0x81,0x18,0x26,0xC3,0xBE,0x35,0x88,0x2E,
    0x93,0x55,0xFC,0x7A,0xC8,0xBA,0x32,0xE6,
    0xC0,0x19,0x9E,0xA3,0x44,0x54,0x3B,0x0B,
    0x8C,0xC7,0x6B,0x28,0xA7,0xBC,0x16,0xAD,
    0xDB,0x64,0x74,0x14,0x92,0x0C,0x48,0xB8,
    0x9F,0xBD,0x43,0xC4,0x39,0x31,0xD3,0xF2,
    0xD5,0x8B,0x6E,0xDA,0x01,0xB1,0x9C,0x49,
    0xD8,0xAC,0xF3,0xCF,0xCA,0xF4,0x47,0x10,
    0x6F,0xF0,0x4A,0x5C,0x38,0x57,0x73,0x97,
    0xCB,0xA1,0xE8,0x3E,0x96,0x61,0x0D,0x0F,
    0xE0,0x7C,0x71,0xCC,0x90,0x06,0xF7,0x1C,
    0xC2,0x6A,0xAE,0x69,0x17,0x99,0x3A,0x27,
    0xD9,0xEB,0x2B,0x22,0xD2,0xA9,0x07,0x33,
    0x2D,0x3C,0x15,0xC9,0x87,0xAA,0x50,0xA5,
    0x03,0x59,0x09,0x1A,0x65,0xD7,0x84,0xD0,
    0x82,0x29,0x5A,0x1E,0x7B,0xA8,0x6D,0x2C
};

#ifdef _USE_DRAM_
unsigned char *pTkip_Sbox_Lower, *pTkip_Sbox_Upper;
#endif

/*****************************/
/**** Function Prototypes ****/
/*****************************/

unsigned int tkip_sbox(unsigned int index);
static __inline__ unsigned int rotr1(unsigned int a);

/* Mixes key from TA, TK and TSC */
// key: TTK (16 bytes), ta (transmitted address)
void mix_key(
                unsigned char    *key,
                unsigned char    *ta,
                unsigned int pnl,  /* Least significant 16 bits of PN */
                unsigned int pnh,  /* Most significant 32 bits of PN */
                unsigned char    *rc4key);


/************************************************************/
/* tkip_sbox()                                              */
/* Returns a 16 bit value from a 64K entry table. The Table */
/* is synthesized from two 256 entry byte wide tables.      */
/************************************************************/
unsigned int tkip_sbox(unsigned int index)
{
    unsigned int index_low;
    unsigned int index_high;
    unsigned int left, right;

    index_low = (index & 255);
    index_high = ((index >> 8) & 255);

#if defined(_USE_DRAM_)
    left = ((int)pTkip_Sbox_Lower[index_low]) + (((int)pTkip_Sbox_Upper[index_low]) << 8);
    right = ((int)pTkip_Sbox_Upper[index_high]) + (((int)pTkip_Sbox_Lower[index_high]) << 8);
#else
    left = Tkip_Sbox_Lower[index_low] + (Tkip_Sbox_Upper[index_low] << 8);
    right = Tkip_Sbox_Upper[index_high] + (Tkip_Sbox_Lower[index_high] << 8);
#endif

    return (left ^ right);
}


static __inline__ unsigned int rotr1(unsigned int a)
{
    unsigned int b;

    if ((a & 0x01) == 0x01)
    {
        b = (a >> 1) | 0x8000;
    }
    else
    {
        b = (a >> 1) & 0x7fff;
    }
    b = b & 0xffff;
    return b;
}


/****************************************************/
/* mix_key()                                        */
/* Takes a key, PN and TK. Calculates an RC4 key.   */
/****************************************************/
void mix_key(
                unsigned char    *key,
                unsigned char    *ta,
                unsigned int pnl,  /* Least significant 16 bits of PN */
                unsigned int pnh,  /* Most significant 32 bits of PN */
                unsigned char    *rc4key)
{
	unsigned int p1k[5];
    /* 16 bit numbers */
    unsigned int tsc0;
    unsigned int tsc1;
    unsigned int tsc2;

    unsigned int ppk0;
    unsigned int ppk1;
    unsigned int ppk2;
    unsigned int ppk3;
    unsigned int ppk4;
    unsigned int ppk5;

    int i;
    int j;

    tsc0 = (unsigned int)((pnh >> 16) & 0xffff);  /* tsc0 is most significant */
    tsc1 = (unsigned int)(pnh & 0xffff);
    tsc2 = (unsigned int)(pnl & 0xffff);          /* tsc2 is least significant */

    /* Phase 1, step 1 */
    p1k[0] = tsc1;
    p1k[1] = tsc0;
    p1k[2] = (unsigned int)(ta[0] + (ta[1]<< 8));
    p1k[3] = (unsigned int)(ta[2] + (ta[3]<< 8));
    p1k[4] = (unsigned int)(ta[4] + (ta[5]<< 8));

    /* Phase 1, step 2 */
    for (i=0; i<8; i++)
    {
        j = (i & 1) << 1;
        p1k[0] =  (p1k[0] + tkip_sbox( (p1k[4] ^ ((key[1+j] << 8) + key[j])) & 0xffff )) & 0xffff;
        p1k[1] =  (p1k[1] + tkip_sbox( (p1k[0] ^ ((key[5+j] << 8) + key[4+j])) & 0xffff )) & 0xffff;
        p1k[2] =  (p1k[2] + tkip_sbox( (p1k[1] ^ ((key[9+j] << 8) + key[8+j])) & 0xffff )) & 0xffff;
        p1k[3] =  (p1k[3] + tkip_sbox( (p1k[2] ^ ((key[13+j]<< 8) + key[12+j])) & 0xffff )) & 0xffff;
        p1k[4] =  (p1k[4] + tkip_sbox( (p1k[3] ^ (((key[1+j]<< 8) + key[j]))) & 0xffff )) & 0xffff;
        p1k[4] =  (p1k[4] + i) & 0xffff;
    }

    /* Phase 2, Step 1 */
    ppk0 = p1k[0];
    ppk1 = p1k[1];
    ppk2 = p1k[2];
    ppk3 = p1k[3];
    ppk4 = p1k[4];
    ppk5 = (p1k[4] + tsc2) & (65536-1);

    /* Phase2, Step 2 */
    ppk0 = ppk0 + tkip_sbox( (ppk5 ^ ((key[1]<<8) + key[0])) & 0xffff);
    ppk1 = ppk1 + tkip_sbox( (ppk0 ^ ((key[3]<<8) + key[2])) & 0xffff);
    ppk2 = ppk2 + tkip_sbox( (ppk1 ^ ((key[5]<<8) + key[4])) & 0xffff);
    ppk3 = ppk3 + tkip_sbox( (ppk2 ^ ((key[7]<<8) + key[6])) & 0xffff);
    ppk4 = ppk4 + tkip_sbox( (ppk3 ^ ((key[9]<<8) + key[8])) & 0xffff);
    ppk5 = ppk5 + tkip_sbox( (ppk4 ^ ((key[11]<<8) + key[10])) & 0xffff);

    ppk0 = ppk0 + rotr1(ppk5 ^ ((key[13]<<8) + key[12]));
    ppk1 = ppk1 + rotr1(ppk0 ^ ((key[15]<<8) + key[14]));
    ppk2 = ppk2 + rotr1(ppk1);
    ppk3 = ppk3 + rotr1(ppk2);
    ppk4 = ppk4 + rotr1(ppk3);
    ppk5 = ppk5 + rotr1(ppk4);

    /* Phase 2, Step 3 */
    rc4key[0] = (tsc2 >> 8) & 255;
    rc4key[1] = (((tsc2 >> 8) & 255) | 0x20) & 0x7f;
    rc4key[2] = tsc2 & 255;
    rc4key[3] = ((ppk5 ^ ((key[1]<<8) + key[0])) >> 1) & 255;

    rc4key[4] = ppk0 & 255;
    rc4key[5] = (ppk0 >> 8) & 255;

    rc4key[6] = ppk1 & 255;
    rc4key[7] = (ppk1 >> 8) & 255;

    rc4key[8] = ppk2 & 255;
    rc4key[9] = (ppk2 >> 8) & 255;

    rc4key[10] = ppk3 & 255;
    rc4key[11] = (ppk3 >> 8) & 255;

    rc4key[12] = ppk4 & 255;
    rc4key[13] = (ppk4 >> 8) & 255;

    rc4key[14] = ppk5 & 255;
    rc4key[15] = (ppk5 >> 8) & 255;
}


void rc4(
            unsigned char *key,
            int key_length,
            int cipherstream_length,
            unsigned char *cipherstream);

void rc4_encrypt(
            unsigned char *key,
            int key_length,
            unsigned char *data,
            int data_length,
            unsigned char *ciphertext);


void xor_block(int length, unsigned char *a, unsigned char *b, unsigned char *out)
{
    int i;
    for (i=0;i<length; i++)
    {
        out[i] = a[i] ^ b[i];
    }
}


static __inline__ void swap(unsigned char *a, unsigned char *b)
{
    unsigned char tmp;

    tmp = *a;
    *a = *b;
    *b = tmp;
}


#ifdef _USE_DRAM_
unsigned char *en_cipherstream;
unsigned char *rc4sbox, *rc4kbox;
#else
//__DRAM_IN_865X
unsigned char en_cipherstream[MAX_MESSAGE_LENGTH+1];
#endif


void rc4(
            unsigned char *key,
            int key_length,
            int cipherstream_length,
            unsigned char *cipherstream)
{
    int i, j, x;

#ifdef _USE_DRAM_
    unsigned char *s = rc4sbox;
    unsigned char *k = rc4kbox;
#else
    unsigned char s[256];
    unsigned char k[256];
#endif

    /* Create Key Stream */
    for (i=0; i<256; i++)
        k[i] = key[i % key_length];

    /* Initialize SBOX */
    for (j=0; j<256; j++)
        s[j] = j;

    /* Seed the SBOX */
    i = 0;
    for (j=0; j<256; j++)
    {
        i = (i + s[j] + k[j]) & 255;
        swap(&s[j], &s[i]);
    }

    /* Generate the cipherstream */
    j = 0;
    i = 0;

    for (x=0; x<cipherstream_length; x++)
    {
        j = (j + 1) & 255;
        i = (i + s[j]) & 255;
        swap(&s[j], &s[i]);
        cipherstream[x] = s[(s[j] + s[i]) & 255];
    };
}


/****************************************/
/* rc4_encrypt()                        */
/****************************************/
void rc4_encrypt(
            unsigned char *key,
            int key_length,
            unsigned char *data,
            int data_length,
            unsigned char *ciphertext)
{
    rc4(key, key_length, data_length, en_cipherstream);

    xor_block(data_length, en_cipherstream, data, ciphertext);
}


void rc4_frag_encrypt(
        	unsigned char *frag1, unsigned int	frag1_len,
        	unsigned char *frag2, unsigned int	frag2_len,
        	unsigned char *frag3, unsigned int	frag3_len,
        	unsigned char *cipher)
{
	unsigned int i;

	for(i=0; i<frag1_len; i++)
		frag1[i] = cipher[i] ^ frag1[i];
	for(i=0; i<frag2_len; i++)
		frag2[i] = cipher[i+frag1_len] ^ frag2[i];
	for(i=0; i<frag3_len; i++)
		frag3[i] = cipher[i+frag1_len+frag2_len] ^ frag3[i];
}


#define REVERSE_BIT(val8)\
   (unsigned char)( ((val8<<7)&0x80) | ((val8<<5)&0x40) | ((val8<<3)&0x20) | ((val8<<1)&0x10) | \
   ((val8>>1)&0x08) | ((val8>>3)&0x04) | ((val8>>5)&0x02) | ((val8>>7)&0x01) )
#define CRC32_POLY 0x04c11db7

static unsigned int crc32_table[256];

void init_crc32_table(void)
{
	int i, j;
	unsigned int c;
	unsigned char *p=(unsigned char *)&c, *p1;
	unsigned char k;

	c = 0x12340000;

	for (i = 0; i < 256; ++i)
	{
		k = REVERSE_BIT( (unsigned char)(i) );
		for (c = ((unsigned int)k) << 24, j = 8; j > 0; --j){
		   c = (c & 0x80000000) ? ((c << 1) ^ CRC32_POLY) : (c << 1);
		}
		p1 = (unsigned char *)&crc32_table[i];

		p1[0] = REVERSE_BIT(p[3]);
		p1[1] = REVERSE_BIT(p[2]);
		p1[2] = REVERSE_BIT(p[1]);
		p1[3] = REVERSE_BIT(p[0]);
	}
}


unsigned int crc32(unsigned char *buf, int len)
{
	unsigned char *p;
	unsigned int  crc;

	crc = 0xffffffff;       /* preload shift register, per CRC-32 spec */

	for (p = buf; len > 0; ++p, --len) {
		crc = crc32_table[(crc ^ *p) & 0xff] ^ (crc >> 8);
	}

	return ~crc;    /* transmit complement, per CRC-32 spec */
}


unsigned int crc32_frag(unsigned long crc, unsigned char *buf, int len)
{
	unsigned char *p;

	for (p = buf; len > 0; ++p, --len) {
		crc = crc32_table[(crc ^ *p) & 0xff] ^ (crc >> 8);
	}

	return ~crc;    /* transmit complement, per CRC-32 spec */
}

#if 0
void appendICV(unsigned char *src, unsigned int len, unsigned char *dest)
{
	unsigned char CRC[4];

	*((unsigned long *)CRC) = crc32(src,len);
	*(unsigned char *)dest=*((unsigned char *)CRC+3);
	*(unsigned char *)(dest+1)=*((unsigned char *)CRC+2);
	*(unsigned char *)(dest+2)=*((unsigned char *)CRC+1);
	*(unsigned char *)(dest+3)=*((unsigned char *)CRC);
}
#endif


void tkip_icv(unsigned char *picv, unsigned char *frag1, unsigned int frag1_len,
				unsigned char *frag2,unsigned int frag2_len)
{
	unsigned int *pcrc;
	unsigned int crc= 0xffffffff;

	pcrc = &crc;
	if (frag1)
		crc = crc32_frag(crc, frag1, frag1_len);
	else
		crc = ~crc;
	if (frag2)
		crc = crc32_frag(~crc, frag2, frag2_len);

#if defined(_LITTLE_ENDIAN_)
    *(unsigned char *)picv=*((unsigned char *)pcrc);
    *(unsigned char *)(picv+1)=*((unsigned char *)pcrc+1);
    *(unsigned char *)(picv+2)=*((unsigned char *)pcrc+2);
    *(unsigned char *)(picv+3)=*((unsigned char *)pcrc+3);
#else
	*(unsigned char *)picv=*((unsigned char *)pcrc+3);
    *(unsigned char *)(picv+1)=*((unsigned char *)pcrc+2);
    *(unsigned char *)(picv+2)=*((unsigned char *)pcrc+1);
    *(unsigned char *)(picv+3)=*((unsigned char *)pcrc);
#endif
}


#ifdef _USE_DRAM_
unsigned char *tx_cipherstream;
#else
__DRAM_IN_865X unsigned char tx_cipherstream[MAX_MESSAGE_LENGTH+1];
#endif

/*-----------------------------------------------------------------------------
								TKIP Flow
1.	Key Mixing (TTK key(128 bits), TA(48 bits), pnl(16 bits), pnh(32 bits)
	==>rc4 key
2.  Using rc4 key to generate Ciphertext/Cipherstream.
3.  Most of the time, we used frag1 to points to llc, frag2 to points to mpdu,
	and frag3 to points to icv
------------------------------------------------------------------------------*/
void tkip_encrypt(
				struct rtl8192cd_priv *priv, unsigned char *pwlhdr, unsigned int hdrlen,
				unsigned char *frag1, unsigned int frag1_len,
				unsigned char *frag2, unsigned int frag2_len,
				unsigned char *frag3, unsigned int frag3_len)
{
	unsigned char *ttkey = NULL;
	unsigned char rc4key[16];
	union PN48 	  *ptsc48 = NULL;
	unsigned short	pnl;	// 16 LSB
	unsigned int	pnh;	// 32 MSB
	unsigned int	keyid = 0;
	unsigned char	*iv = pwlhdr + hdrlen;
	struct stat_info	*pstat = NULL;
	unsigned char *ta = GET_MY_HWADDR;
	unsigned char *ra;
#ifdef WDS
	unsigned int to_fr_ds = (GetToDs(pwlhdr) << 1) | GetFrDs(pwlhdr);
#endif

	DEBUG_TRACE;

	ra = GetAddr1Ptr(pwlhdr);
	pstat = get_stainfo(priv, ra);

#ifdef WDS
	if ( (to_fr_ds == 3) && pstat && (pstat->state & WIFI_WDS)) {
		ttkey = GET_UNICAST_ENCRYP_KEY;
		ptsc48 = GET_UNICAST_ENCRYP_PN;
		keyid = pstat->keyid;
		goto do_tkip_encrypt;
	}
#endif

	if (OPMODE & WIFI_AP_STATE)
	{
		if (IS_MCAST(ra))
		{
			ttkey = GET_GROUP_ENCRYP_KEY;
			ptsc48 = GET_GROUP_ENCRYP_PN;
			//keyid = 1;
			keyid = priv->pmib->dot11GroupKeysTable.keyid;//use mib keyid
		}
		else
		{
			if (pstat == NULL) {
				DEBUG_ERR("tx tkip pstat == NULL\n");
				return;
			}
			ttkey = GET_UNICAST_ENCRYP_KEY;
			ptsc48 = GET_UNICAST_ENCRYP_PN;
			keyid = 0;
		}
	}
#ifdef CLIENT_MODE
	else if (OPMODE & WIFI_STATION_STATE)
	{
		if (IS_MCAST(ra))
		{
			ttkey = GET_GROUP_ENCRYP_KEY;
			ptsc48 = GET_GROUP_ENCRYP_PN;
			keyid = GET_ROOT(priv)->pmib->dot11GroupKeysTable.keyid;
		}
		else
		{
			if (pstat == NULL) {
				DEBUG_ERR("tx tkip pstat == NULL\n");
				return;
			}
			ttkey = GET_UNICAST_ENCRYP_KEY;
			ptsc48 = GET_UNICAST_ENCRYP_PN;
			keyid = 0;
		}
	}
	else if (OPMODE & WIFI_ADHOC_STATE)
	{
		ttkey = GET_GROUP_ENCRYP_KEY;
		ptsc48 = GET_GROUP_ENCRYP_PN;
		keyid = 0;
	}
#endif

	if ((ttkey == NULL) || (ptsc48 == NULL)) {
		DEBUG_ERR("no encrypt key for TKIP due to ttkey=NULL or ptsc48=NULL\n");
		return;
	}

#ifdef WDS
do_tkip_encrypt:
#endif
	pnl = get_pnl(ptsc48);
	pnh = get_pnh(ptsc48);

	// below is for key mixing!
	mix_key(ttkey, ta, pnl, pnh, rc4key);

	//okay, use rc4_encrypt go generate ciphertext.
	rc4(rc4key, 16, (frag1_len + frag2_len + frag3_len), tx_cipherstream);

	// now, using cipherstream to xor all the plaintext!
	rc4_frag_encrypt(frag1, frag1_len, frag2, frag2_len, frag3, frag3_len, tx_cipherstream);

	// Wa-oh...finally, let's fill in the IV field...
	iv[0] = ptsc48->_byte_.TSC1;
	iv[1] = (iv[0] | 0x20) & 0x7f;
	iv[2] = ptsc48->_byte_.TSC0;
	iv[3] = 0x20 | (keyid << 6);
	iv[4] = ptsc48->_byte_.TSC2;
	iv[5] = ptsc48->_byte_.TSC3;
	iv[6] = ptsc48->_byte_.TSC4;
	iv[7] = ptsc48->_byte_.TSC5;

	if (ptsc48->val48 == 0xffffffffffffULL)
		ptsc48->val48 = 0;
	else
		ptsc48->val48++;
}


unsigned int tkip_decrypt(struct rtl8192cd_priv *priv, struct rx_frinfo *pfrinfo, unsigned int fr_len)
{
	union TSC48		tsc48;
	unsigned char	*da;
	unsigned char	*ta;
	unsigned int	keylen=0, hdr_len, pnh, crc;
	unsigned short	pnl;
	unsigned char	*ttkey = NULL;
	unsigned char	rc4key[16];
	unsigned char		*pframe;
	struct stat_info	*pstat = NULL;

	pframe = get_pframe(pfrinfo);
	da = pfrinfo->da;
	hdr_len = pfrinfo->hdr_len;

	ta = GetAddr2Ptr(pframe);
	pstat = get_stainfo(priv, ta);

	if (pstat) {
#ifdef WDS
		if ((pfrinfo->to_fr_ds == 3) && (pstat->state & WIFI_WDS)) {
			keylen = GET_UNICAST_ENCRYP_KEYLEN;
			ttkey  = GET_UNICAST_ENCRYP_KEY;
			goto do_tkip_decrypt;
		}
#endif
	}

	if (OPMODE & WIFI_AP_STATE)
	{
		if (pstat == NULL) {
			DEBUG_WARN("rx tkip pstat == NULL\n");
			return FALSE;
		}
		keylen = GET_UNICAST_ENCRYP_KEYLEN;
		ttkey  = GET_UNICAST_ENCRYP_KEY;
	}
#ifdef CLIENT_MODE
	else if (OPMODE & WIFI_STATION_STATE)
	{
		if (IS_MCAST(da))
		{
			if((pframe[WLAN_HDR_A3_LEN + 3]&BIT(7))){
				keylen = GET_GROUP_IDX2_ENCRYP_KEYLEN;
				ttkey = GET_GROUP_ENCRYP2_KEY;
			}
			else{				
				keylen = GET_GROUP_ENCRYP_KEYLEN;
				ttkey = GET_GROUP_ENCRYP_KEY;
			}
		}
		else
		{
			if (pstat == NULL) {
				DEBUG_WARN("rx tkip pstat == NULL\n");
				return FALSE;
			}
			keylen = GET_UNICAST_ENCRYP_KEYLEN;
			ttkey  = GET_UNICAST_ENCRYP_KEY;
		}
	}
	else if (OPMODE & WIFI_ADHOC_STATE)
	{
		keylen = GET_GROUP_ENCRYP_KEYLEN;
		ttkey  = GET_GROUP_ENCRYP_KEY;
	}
#endif

	if (keylen == 0) {
		DEBUG_ERR("no descrypt key for TKIP due to keylen=0\n");
		return FALSE;
	}
#ifdef WDS
do_tkip_decrypt:
#endif
	tsc48._byte_.TSC1 = *(pframe + hdr_len + 0);
	tsc48._byte_.TSC0 = *(pframe + hdr_len + 2);
	tsc48._byte_.TSC2 = *(pframe + hdr_len + 4);
	tsc48._byte_.TSC3 = *(pframe + hdr_len + 5);
	tsc48._byte_.TSC4 = *(pframe + hdr_len + 6);
	tsc48._byte_.TSC5 = *(pframe + hdr_len + 7);
	pnl = get_pnl((union PN48 *)&tsc48);
	pnh = get_pnh((union PN48 *)&tsc48);

	mix_key(ttkey, ta, pnl, pnh, rc4key);

	rc4_encrypt(rc4key, 16, pframe + hdr_len + 8, fr_len - hdr_len - 8, pframe + hdr_len + 8);

	// now, let's check if icv is correct!
	crc = crc32(pframe + hdr_len + 8, fr_len - hdr_len - 8 - 4);

	crc = le32_to_cpu(crc);  //crc is big endian located in the payload
	if (memcmp((void *)&crc, (void *)(pframe + fr_len -4), 4))
	{
		DEBUG_ERR("crc error!crc=%08X, vs %02X%02X%02X%02X\n", crc,
									*(pframe + fr_len -4),
									*(pframe + fr_len -3),
									*(pframe + fr_len -2),
									*(pframe + fr_len -1));
		return FALSE;
	}
	else
		return TRUE;
}


void wep_encrypt(struct rtl8192cd_priv *priv, unsigned char *pwlhdr, unsigned int hdrlen,
				unsigned char *frag1, unsigned int frag1_len,
				unsigned char *frag2, unsigned int frag2_len,
				unsigned char *frag3, unsigned int frag3_len,
				int type)
{
	unsigned char *ra;
	unsigned char *ttkey = NULL;
	unsigned char rc4key[16];
	unsigned int keyid = 0;
	unsigned char *iv = pwlhdr + hdrlen;
	struct stat_info *pstat = NULL;
	unsigned int *piv;
	int keylen;
#ifdef WDS
	unsigned int to_fr_ds = (GetToDs(pwlhdr) << 1) | GetFrDs(pwlhdr);
#endif

	DEBUG_TRACE;

	ra = GetAddr1Ptr(pwlhdr);
	pstat = get_stainfo(priv, ra);

	if (pstat) {
#ifdef WDS
		if ((to_fr_ds == 3) && (pstat->state & WIFI_WDS)) {
			ttkey = GET_UNICAST_ENCRYP_KEY;
			keyid = pstat->keyid;
			goto do_encrypt;
		}
#endif
	}

	if (priv->pmib->dot118021xAuthEntry.dot118021xAlgrthm)	// 1x enabled, get key from mapping table
	{
		if (OPMODE & WIFI_AP_STATE)
		{
			if (IS_MCAST(ra))
			{
				ttkey = GET_GROUP_ENCRYP_KEY;
				keyid = 0;
			}
			else
			{
				if (pstat == NULL) {
					DEBUG_ERR("tx wep pstat == NULL\n");
					return;
				}
				ttkey = GET_UNICAST_ENCRYP_KEY;
				keyid = 3;
			}
		}
#ifdef CLIENT_MODE
		else if (OPMODE & WIFI_STATION_STATE)
		{
			if (pstat == NULL) {
				DEBUG_ERR("tx wep pstat == NULL\n");
				return;
			}
			ttkey = GET_UNICAST_ENCRYP_KEY;
			keyid = 3;
		}
		else if (OPMODE & WIFI_ADHOC_STATE)
		{
			ttkey = GET_GROUP_ENCRYP_KEY;
			keyid = 0;
		}
#endif
	}
	else
	{
		keyid = priv->pmib->dot1180211AuthEntry.dot11PrivacyKeyIndex;
		ttkey = priv->pmib->dot11DefaultKeysTable.keytype[keyid&3].skey;
	}

#ifdef WDS
do_encrypt:
#endif

	piv = (unsigned int *)GET_GROUP_ENCRYP_PN;
	if (type == _WEP_40_PRIVACY_)
		keylen = 8;
	else
		keylen = 16;

#ifdef __OSK__
	{
		/* 20100612 iv pointer is not always 4byte alignment, when qos enabled on station and ap, and mb
		ssid enabled, txcfg->hdrlen is 26 for QoS*/
       unsigned long temp;
       temp = cpu_to_le32(((*piv) & 0x00FFFFFF) |((keyid&0x03)<<30));
       *((unsigned char*)iv) = (temp>>24)&0xFF;
       *(((unsigned char*)iv)+1) = (temp>>16)&0xFF;
       *(((unsigned char*)iv)+2) = (temp>>8)&0xFF;
       *(((unsigned char*)iv)+3) = temp&0xFF;
	}
#else
	*((unsigned int *)iv) = cpu_to_le32(((*piv) & 0x00FFFFFF) |((keyid&0x03)<<30));
#endif
	*piv = *piv + 1;

	memcpy(rc4key, iv, 3);
	memcpy(&rc4key[3], ttkey, keylen-3);

#ifdef _DEBUG_RTL8192CD_
{
	char tmpbuf[400], tmp1[100];
	int i;
	
	sprintf(tmpbuf, "wep encrypt: iv=%d, keyid=%d, type=%s, key=",
		(le32_to_cpup((u32 *)iv) & 0xFFFFFF), keyid,
		(type==_WEP_40_PRIVACY_ ? "64b" : "128b"));

	for (i=0; i<keylen-3; i++) {
		sprintf(tmp1, "%02x", ttkey[i]);
		strcat(tmpbuf, tmp1);
		if (i+1 != keylen-3)
			strcat(tmpbuf, ":");
		else
			strcat(tmpbuf, "\n");
	}
	DEBUG_INFO("%s", tmpbuf);
}
#endif

	//okay, use rc4_encrypt go generate ciphertext.
	rc4(rc4key, keylen, (frag1_len + frag2_len + frag3_len), tx_cipherstream);

	// now, using cipherstream to xor all the plaintext!
	rc4_frag_encrypt(frag1, frag1_len, frag2, frag2_len, frag3, frag3_len, tx_cipherstream);
}


unsigned int wep_decrypt(struct rtl8192cd_priv *priv, struct rx_frinfo *pfrinfo,
				unsigned int fr_len, int type, int using_keymap)
{
	unsigned char	*ta;
	unsigned int	keylen, hdr_len, crc;
	unsigned char	*ttkey = NULL;
	unsigned char	iv[4];
	unsigned char	rc4key[16];
	unsigned char	*pframe;
	struct stat_info *pstat = NULL;

	DEBUG_TRACE;

	pframe = get_pframe(pfrinfo);
	ta = GetAddr2Ptr(pframe);

	hdr_len = pfrinfo->hdr_len;
	memcpy(iv, pframe+hdr_len, sizeof(iv));

	if (type == _WEP_40_PRIVACY_)
		keylen = 8;
	else
		keylen = 16;

	pstat = get_stainfo(priv, ta);

	if (pstat) {
#ifdef WDS
		if ((pfrinfo->to_fr_ds == 3) && (pstat->state & WIFI_WDS)) {
			ttkey = GET_UNICAST_ENCRYP_KEY;
			goto do_decrypt;
		}
#endif
	}

	if (priv->pmib->dot118021xAuthEntry.dot118021xAlgrthm)	// 1x enabled, get key from mapping table
	{
		if (OPMODE & WIFI_AP_STATE)
		{
			if (using_keymap) {
				if (pstat == NULL) {
					DEBUG_ERR("rx wep pstat == NULL\n");
					return FALSE;
				}
				ttkey = GET_UNICAST_ENCRYP_KEY;
			}
			else if ((iv[3] & 0xC0) == 0)	// key id=0, use group key
				ttkey = GET_GROUP_ENCRYP_KEY;
			else if ((iv[3] & 0xC0) == 0xC0)
			{
				if (pstat == NULL) {
					DEBUG_ERR("rx wep pstat == NULL\n");
					return FALSE;
				}
				ttkey = GET_UNICAST_ENCRYP_KEY;
			}
			else
			{
				DEBUG_ERR("rx wep keyid != 0 and 3 \n");
				return FALSE;
			}
		}
#ifdef CLIENT_MODE
		else if (OPMODE & WIFI_STATION_STATE) {
			if ((iv[3] & 0xC0) == 0)	// key id=0, use group key
				ttkey = GET_GROUP_ENCRYP_KEY;
			else if ((iv[3] & 0xC0) == 0xC0)
			{
				if (pstat == NULL) {
					DEBUG_ERR("rx wep pstat == NULL\n");
					return FALSE;
				}
				ttkey = GET_UNICAST_ENCRYP_KEY;
			}
			else
			{
				DEBUG_ERR("rx wep keyid != 0 and 3 \n");
				return FALSE;
			}
		}
		else if (OPMODE & WIFI_ADHOC_STATE)
			ttkey = GET_GROUP_ENCRYP_KEY;
#endif
	}
	else // default key
		ttkey = priv->pmib->dot11DefaultKeysTable.keytype[(iv[3]>>6) & 3].skey;

#ifdef WDS
do_decrypt:
#endif

	memcpy(rc4key, iv, 3);
	memcpy(&rc4key[3], ttkey, keylen-3);

#ifdef _DEBUG_RTL8192CD_
{
	char tmpbuf[400], tmp1[100];
	int i;
	
	sprintf(tmpbuf, "wep decript: iv=%d, keyid=%d, type=%s, key=",
		(le32_to_cpup((u32 *)iv) & 0xFFFFFF), (int)((iv[3]>>6)&3),
		(type==_WEP_40_PRIVACY_ ? "64b" : "128b"));

	for (i=0; i<keylen-3; i++) {
		sprintf(tmp1, "%02x", ttkey[i]);
		strcat(tmpbuf, tmp1);
		if (i+1 != keylen-3)
			strcat(tmpbuf, ":");
		else
			strcat(tmpbuf, "\n");
	}
	DEBUG_INFO("%s", tmpbuf);
}
#endif

	rc4_encrypt(rc4key, keylen, pframe+hdr_len+4, fr_len-hdr_len-4, pframe+hdr_len+4);

	// now, let's check if icv is correct!
	crc = crc32(pframe+hdr_len+4, fr_len-hdr_len-4-4);

	crc = le32_to_cpu(crc);  //crc is big endian located in the payload
	if (memcmp((void *)&crc, (void *)(pframe + fr_len -4), 4)) {
		DEBUG_ERR("crc error!crc=%08X, vs %02X%02X%02X%02X\n", crc,
									*(pframe + fr_len -4),
									*(pframe + fr_len -3),
									*(pframe + fr_len -2),
									*(pframe + fr_len -1));
		return FALSE;
	}
	else
		return TRUE;
}


int tkip_rx_mic(struct rtl8192cd_priv *priv, unsigned char *pframe, unsigned char *da,
				unsigned char *sa, unsigned char priority, unsigned char *pbuf, unsigned int len,
				unsigned char *tkipmic, int no_wait)
{
	// now check what's the mic key we should apply...

	unsigned char	*mickey = NULL;
	unsigned int	keylen = 0;
	struct stat_info	*pstat;
	unsigned char	hdr[16], *ta;
	unsigned int	num_blocks;

	ta = GetAddr2Ptr(pframe);
	pstat = get_stainfo(priv, ta);

	if (OPMODE & WIFI_AP_STATE)
	{
#ifdef WDS
		unsigned int to_fr_ds = (GetToDs(pframe) << 1) | GetFrDs(pframe);
#endif

		if (pstat == NULL) {
			DEBUG_ERR("rx mic pstat == NULL\n");
			return FALSE;
		}

		keylen = GET_UNICAST_MIC_KEYLEN;
#ifdef WDS
		if ( to_fr_ds==3 && (pstat->state & WIFI_WDS))
#ifdef __DRAYTEK_OS__
		mickey = GET_UNICAST_TKIP_MIC2_KEY;
#else
		mickey = GET_UNICAST_TKIP_MIC1_KEY;
#endif
		else
#endif
		mickey = GET_UNICAST_TKIP_MIC2_KEY;
	}
#ifdef CLIENT_MODE
	else if (OPMODE & WIFI_STATION_STATE)
	{

        #ifdef A4_STA
        if((GetToDs(pframe) << 1) | GetFrDs(pframe) == 3) {
            keylen = GET_UNICAST_MIC_KEYLEN;
            mickey = GET_UNICAST_TKIP_MIC1_KEY;
        }
        else
        #endif     
		if (IS_MCAST(da))
		{
	 		if((pframe[WLAN_HDR_A3_LEN + 3]&BIT(7))){
				keylen = GET_GROUP_IDX2_MIC_KEYLEN;
				mickey = GET_GROUP_TKIP_IDX2_MIC1_KEY;
			}
			else{
				keylen = GET_GROUP_MIC_KEYLEN;
				mickey = GET_GROUP_TKIP_MIC1_KEY;		
			}
		}
		else
		{
			if (pstat == NULL) {
				DEBUG_ERR("rx mic pstat == NULL\n");
				return FALSE;
			}

	 		keylen = GET_UNICAST_MIC_KEYLEN;
			mickey = GET_UNICAST_TKIP_MIC1_KEY;
		}
	}
	else if (OPMODE & WIFI_ADHOC_STATE)
	{
 		keylen = GET_GROUP_MIC_KEYLEN;
		mickey = GET_GROUP_TKIP_MIC1_KEY;
	}
#endif

	if (keylen == 0)
	{
		DEBUG_ERR("no mic padding for TKIP due to keylen=0\n");
		return FALSE;
	}
	memcpy((void *)hdr, (void *)da, WLAN_ADDR_LEN);
	memcpy((void *)(hdr + WLAN_ADDR_LEN), (void *)sa, WLAN_ADDR_LEN);
	hdr[12] = priority;
	hdr[13] = hdr[14] = hdr[15] = 0;

	pbuf[len] = 0x5a;   /* Insert padding */
	pbuf[len+1] = 0x00;
	pbuf[len+2] = 0x00;
	pbuf[len+3] = 0x00;
	pbuf[len+4] = 0x00;
	pbuf[len+5] = 0x00;
	pbuf[len+6] = 0x00;
	pbuf[len+7] = 0x00;

	num_blocks = (16 + len + 5)/4;  // 8 is for snap_llc length = 8
	if ((16 + len + 5) & (4-1))
		num_blocks++;

#ifndef NOT_RTK_BSP
	if ((priv->pshare->have_hw_mic) &&
		!(priv->pmib->dot11StationConfigEntry.swTkipMic))
	{
		register unsigned int l,r;

		michael(priv, mickey, hdr, pbuf, pbuf+8, (num_blocks << 2), tkipmic, 0);

		if (no_wait)
			return FALSE;

		do {
			delay_us(60);
		} while ((*(volatile unsigned int *)GDMAISR & GDMA_COMPIP) == 0);

		l = *(volatile unsigned int *)GDMAICVL;
		r = *(volatile unsigned int *)GDMAICVR;

		tkipmic[0] = (unsigned char)(l & 0xff);
		tkipmic[1] = (unsigned char)((l >> 8) & 0xff);
		tkipmic[2] = (unsigned char)((l >> 16) & 0xff);
		tkipmic[3] = (unsigned char)((l >> 24) & 0xff);
		tkipmic[4] = (unsigned char)(r & 0xff);
		tkipmic[5] = (unsigned char)((r >> 8) & 0xff);
		tkipmic[6] = (unsigned char)((r >> 16) & 0xff);
		tkipmic[7] = (unsigned char)((r >> 24) & 0xff);
	}
	else
#endif // NOT_RTK_BSP
		michael(priv, mickey, hdr, pbuf, pbuf+8, (num_blocks << 2), tkipmic, 0);

	return TRUE;
}

