/*
 *  Software AES encryption/descryption routines
 *
 *  $Id: 8192cd_aes.c,v 1.1.4.2 2010/11/29 12:23:52 button Exp $
 *
 *  Copyright (c) 2009 Realtek Semiconductor Corp.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 */

#define _8192CD_AES_C_

#ifdef __KERNEL__
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

#define SHOW_INIT_BLOCKS
#define SHOW_HEADER_FIELDS
#define SHOW_CTR_PRELOAD

#define MAX_MSG_SIZE	2048
/*****************************/
/******** SBOX Table *********/
/*****************************/

    unsigned char sbox_table[256] =
    {
        0x63, 0x7c, 0x77, 0x7b, 0xf2, 0x6b, 0x6f, 0xc5,
        0x30, 0x01, 0x67, 0x2b, 0xfe, 0xd7, 0xab, 0x76,
        0xca, 0x82, 0xc9, 0x7d, 0xfa, 0x59, 0x47, 0xf0,
        0xad, 0xd4, 0xa2, 0xaf, 0x9c, 0xa4, 0x72, 0xc0,
        0xb7, 0xfd, 0x93, 0x26, 0x36, 0x3f, 0xf7, 0xcc,
        0x34, 0xa5, 0xe5, 0xf1, 0x71, 0xd8, 0x31, 0x15,
        0x04, 0xc7, 0x23, 0xc3, 0x18, 0x96, 0x05, 0x9a,
        0x07, 0x12, 0x80, 0xe2, 0xeb, 0x27, 0xb2, 0x75,
        0x09, 0x83, 0x2c, 0x1a, 0x1b, 0x6e, 0x5a, 0xa0,
        0x52, 0x3b, 0xd6, 0xb3, 0x29, 0xe3, 0x2f, 0x84,
        0x53, 0xd1, 0x00, 0xed, 0x20, 0xfc, 0xb1, 0x5b,
        0x6a, 0xcb, 0xbe, 0x39, 0x4a, 0x4c, 0x58, 0xcf,
        0xd0, 0xef, 0xaa, 0xfb, 0x43, 0x4d, 0x33, 0x85,
        0x45, 0xf9, 0x02, 0x7f, 0x50, 0x3c, 0x9f, 0xa8,
        0x51, 0xa3, 0x40, 0x8f, 0x92, 0x9d, 0x38, 0xf5,
        0xbc, 0xb6, 0xda, 0x21, 0x10, 0xff, 0xf3, 0xd2,
        0xcd, 0x0c, 0x13, 0xec, 0x5f, 0x97, 0x44, 0x17,
        0xc4, 0xa7, 0x7e, 0x3d, 0x64, 0x5d, 0x19, 0x73,
        0x60, 0x81, 0x4f, 0xdc, 0x22, 0x2a, 0x90, 0x88,
        0x46, 0xee, 0xb8, 0x14, 0xde, 0x5e, 0x0b, 0xdb,
        0xe0, 0x32, 0x3a, 0x0a, 0x49, 0x06, 0x24, 0x5c,
        0xc2, 0xd3, 0xac, 0x62, 0x91, 0x95, 0xe4, 0x79,
        0xe7, 0xc8, 0x37, 0x6d, 0x8d, 0xd5, 0x4e, 0xa9,
        0x6c, 0x56, 0xf4, 0xea, 0x65, 0x7a, 0xae, 0x08,
        0xba, 0x78, 0x25, 0x2e, 0x1c, 0xa6, 0xb4, 0xc6,
        0xe8, 0xdd, 0x74, 0x1f, 0x4b, 0xbd, 0x8b, 0x8a,
        0x70, 0x3e, 0xb5, 0x66, 0x48, 0x03, 0xf6, 0x0e,
        0x61, 0x35, 0x57, 0xb9, 0x86, 0xc1, 0x1d, 0x9e,
        0xe1, 0xf8, 0x98, 0x11, 0x69, 0xd9, 0x8e, 0x94,
        0x9b, 0x1e, 0x87, 0xe9, 0xce, 0x55, 0x28, 0xdf,
        0x8c, 0xa1, 0x89, 0x0d, 0xbf, 0xe6, 0x42, 0x68,
        0x41, 0x99, 0x2d, 0x0f, 0xb0, 0x54, 0xbb, 0x16
    };

/*****************************/
/**** Function Prototypes ****/
/*****************************/

void bitwise_xor(unsigned char *ina, unsigned char *inb, unsigned char *out);
void construct_mic_iv(
                        unsigned char *mic_header1,
                        int qc_exists,
                        int a4_exists,
                        unsigned char *mpdu,
                        unsigned int payload_length,
                        unsigned char * pn_vector
#ifdef CONFIG_IEEE80211W
						,unsigned char isMgmt
#endif
						);
void construct_mic_header1(
                        unsigned char *mic_header1,
                        int header_length,
                        unsigned char *mpdu
#ifdef CONFIG_IEEE80211W
						,unsigned char isMgmt
#endif						
						);
void construct_mic_header2(
                    unsigned char *mic_header2,
                    unsigned char *mpdu,
                    int a4_exists,
                    int qc_exists);
void construct_ctr_preload(
                        unsigned char *ctr_preload,
                        int a4_exists,
                        int qc_exists,
                        unsigned char *mpdu,
                        unsigned char *pn_vector,
                        int c
#ifdef CONFIG_IEEE80211W
						,unsigned char isMgmt
#endif						
					);
void xor_128(unsigned char *a, unsigned char *b, unsigned char *out);
void xor_32(unsigned char *a, unsigned char *b, unsigned char *out);
unsigned char sbox(unsigned char a);
void next_key(unsigned char *key, int round);
void byte_sub(unsigned char *in, unsigned char *out);
void shift_row(unsigned char *in, unsigned char *out);
void mix_column(unsigned char *in, unsigned char *out);
void add_round_key( unsigned char *shiftrow_in,
                    unsigned char *mcol_in,
                    unsigned char *block_in,
                    int round,
                    unsigned char *out);
void aes128k128d(unsigned char *key, unsigned char *data, unsigned char *ciphertext);


/****************************************/
/* aes128k128d()                        */
/* Performs a 128 bit AES encrypt with  */
/* 128 bit data.                        */
/****************************************/
void xor_128(unsigned char *a, unsigned char *b, unsigned char *out)
{
    int i;
    for (i=0;i<16; i++)
    {
        out[i] = a[i] ^ b[i];
    }
}


void xor_32(unsigned char *a, unsigned char *b, unsigned char *out)
{
    int i;
    for (i=0;i<4; i++)
    {
        out[i] = a[i] ^ b[i];
    }
}


unsigned char sbox(unsigned char a)
{
    return sbox_table[(int)a];
}


void next_key(unsigned char *key, int round)
{
    unsigned char rcon;
    unsigned char sbox_key[4];
    unsigned char rcon_table[12] =
    {
        0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80,
        0x1b, 0x36, 0x36, 0x36
    };

    sbox_key[0] = sbox(key[13]);
    sbox_key[1] = sbox(key[14]);
    sbox_key[2] = sbox(key[15]);
    sbox_key[3] = sbox(key[12]);

    rcon = rcon_table[round];

    xor_32(&key[0], sbox_key, &key[0]);
    key[0] = key[0] ^ rcon;

    xor_32(&key[4], &key[0], &key[4]);
    xor_32(&key[8], &key[4], &key[8]);
    xor_32(&key[12], &key[8], &key[12]);
}


void byte_sub(unsigned char *in, unsigned char *out)
{
    int i;
    for (i=0; i< 16; i++)
    {
        out[i] = sbox(in[i]);
    }
}


void shift_row(unsigned char *in, unsigned char *out)
{
    out[0] =  in[0];
    out[1] =  in[5];
    out[2] =  in[10];
    out[3] =  in[15];
    out[4] =  in[4];
    out[5] =  in[9];
    out[6] =  in[14];
    out[7] =  in[3];
    out[8] =  in[8];
    out[9] =  in[13];
    out[10] = in[2];
    out[11] = in[7];
    out[12] = in[12];
    out[13] = in[1];
    out[14] = in[6];
    out[15] = in[11];
}


void mix_column(unsigned char *in, unsigned char *out)
{
    int i;
    unsigned char add1b[4];
    unsigned char add1bf7[4];
    unsigned char rotl[4];
    unsigned char swap_halfs[4];
    unsigned char andf7[4];
    unsigned char rotr[4];
    unsigned char temp[4];
    unsigned char tempb[4];

    for (i=0 ; i<4; i++)
    {
        if ((in[i] & 0x80)== 0x80)
            add1b[i] = 0x1b;
        else
            add1b[i] = 0x00;
    }

    swap_halfs[0] = in[2];    /* Swap halfs */
    swap_halfs[1] = in[3];
    swap_halfs[2] = in[0];
    swap_halfs[3] = in[1];

    rotl[0] = in[3];        /* Rotate left 8 bits */
    rotl[1] = in[0];
    rotl[2] = in[1];
    rotl[3] = in[2];

    andf7[0] = in[0] & 0x7f;
    andf7[1] = in[1] & 0x7f;
    andf7[2] = in[2] & 0x7f;
    andf7[3] = in[3] & 0x7f;

    for (i = 3; i>0; i--)    /* logical shift left 1 bit */
    {
        andf7[i] = andf7[i] << 1;
        if ((andf7[i-1] & 0x80) == 0x80)
        {
            andf7[i] = (andf7[i] | 0x01);
        }
    }
    andf7[0] = andf7[0] << 1;
    andf7[0] = andf7[0] & 0xfe;

    xor_32(add1b, andf7, add1bf7);

    xor_32(in, add1bf7, rotr);

    temp[0] = rotr[0];         /* Rotate right 8 bits */
    rotr[0] = rotr[1];
    rotr[1] = rotr[2];
    rotr[2] = rotr[3];
    rotr[3] = temp[0];

    xor_32(add1bf7, rotr, temp);
    xor_32(swap_halfs, rotl,tempb);
    xor_32(temp, tempb, out);
}


void aes128k128d(unsigned char *key, unsigned char *data, unsigned char *ciphertext)
{
    int round;
    int i;
    unsigned char intermediatea[16];
    unsigned char intermediateb[16];
    unsigned char round_key[16];

    for(i=0; i<16; i++) round_key[i] = key[i];

    for (round = 0; round < 11; round++)
    {
        if (round == 0)
        {
            xor_128(round_key, data, ciphertext);
            next_key(round_key, round);
        }
        else if (round == 10)
        {
            byte_sub(ciphertext, intermediatea);
            shift_row(intermediatea, intermediateb);
            xor_128(intermediateb, round_key, ciphertext);
        }
        else    /* 1 - 9 */
        {
            byte_sub(ciphertext, intermediatea);
            shift_row(intermediatea, intermediateb);
            mix_column(&intermediateb[0], &intermediatea[0]);
            mix_column(&intermediateb[4], &intermediatea[4]);
            mix_column(&intermediateb[8], &intermediatea[8]);
            mix_column(&intermediateb[12], &intermediatea[12]);
            xor_128(intermediatea, round_key, ciphertext);
            next_key(round_key, round);
        }
    }

}

#if defined(CONFIG_IEEE80211W) || defined(CONFIG_IEEE80211R)
extern void AES_Encrypt(const unsigned long *in_blk, unsigned long *out_blk);
extern unsigned long *AES_SetKey(const unsigned long *in_key, const unsigned long key_len);
#define AES_BLOCK_SIZE 16
static void gf_mulx(unsigned char *pad)
{
	int i, carry;

	carry = pad[0] & 0x80;
	for (i = 0; i < AES_BLOCK_SIZE - 1; i++)
		pad[i] = (pad[i] << 1) | (pad[i + 1] >> 7);
	pad[AES_BLOCK_SIZE - 1] <<= 1;
	if (carry)
		pad[AES_BLOCK_SIZE - 1] ^= 0x87;
}

/**
 * omac1_aes_128_vector - One-Key CBC MAC (OMAC1) hash with AES-128
 * @key: 128-bit key for the hash operation
 * @num_elem: Number of elements in the data vector
 * @addr: Pointers to the data areas
 * @len: Lengths of the data blocks
 * @mac: Buffer for MAC (128 bits, i.e., 16 bytes)
 * Returns: 0 on success, -1 on failure
 *
 * This is a mode for using block cipher (AES in this case) for authentication.
 * OMAC1 was standardized with the name CMAC by NIST in a Special Publication
 * (SP) 800-38B.
 */
int omac1_aes_128_vector(const unsigned char *key, int num_elem,
			 const unsigned char *addr[], const int *len, unsigned char *mac)
{
	void *ctx;
	unsigned char cbc[AES_BLOCK_SIZE], pad[AES_BLOCK_SIZE];
	const unsigned char *pos, *end;	
	int i, e, left, total_len;
	
	ctx = AES_SetKey(key, 128);
	if (ctx == NULL)
		return -1;
	memset(cbc, 0, AES_BLOCK_SIZE);

	total_len = 0;
	for (e = 0; e < num_elem; e++)
		total_len += len[e];
	left = total_len;
		

	e = 0;
	pos = addr[0];
	end = pos + len[0];

	while (left >= AES_BLOCK_SIZE) {
		for (i = 0; i < AES_BLOCK_SIZE; i++) {
			cbc[i] ^= *pos++;
			if (pos >= end) {
				e++;
				pos = addr[e];
				end = pos + len[e];
			}
		}
		if (left > AES_BLOCK_SIZE)
			AES_Encrypt(cbc, cbc);
		left -= AES_BLOCK_SIZE;
	}

	memset(pad, 0, AES_BLOCK_SIZE);
	
	AES_Encrypt(pad, pad);
	gf_mulx(pad);

	if (left || total_len == 0) {
		for (i = 0; i < left; i++) {
			cbc[i] ^= *pos++;
			if (pos >= end) {
				e++;
				pos = addr[e];
				end = pos + len[e];
			}
		}
		cbc[left] ^= 0x80;
		gf_mulx(pad);
	}

	for (i = 0; i < AES_BLOCK_SIZE; i++)
		pad[i] ^= cbc[i];
	AES_Encrypt(pad, mac);
	
	return 0;
}


/**
 * omac1_aes_128 - One-Key CBC MAC (OMAC1) hash with AES-128 (aka AES-CMAC)
 * @key: 128-bit key for the hash operation
 * @data: Data buffer for which a MAC is determined
 * @data_len: Length of data buffer in bytes
 * @mac: Buffer for MAC (128 bits, i.e., 16 bytes)
 * Returns: 0 on success, -1 on failure
 *
 * This is a mode for using block cipher (AES in this case) for authentication.
 * OMAC1 was standardized with the name CMAC by NIST in a Special Publication
 * (SP) 800-38B.
 */
int omac1_aes_128(const unsigned char *key, const unsigned char *data, int data_len, unsigned char *mac)
{
	return omac1_aes_128_vector(key, 1, &data, &data_len, mac);
}
#endif

/************************************************/
/* construct_mic_iv()                           */
/* Builds the MIC IV from header fields and PN  */
/************************************************/
void construct_mic_iv(
                        unsigned char *mic_iv,
                        int qc_exists,
                        int a4_exists,
                        unsigned char *mpdu,
                        unsigned int payload_length,
                        unsigned char *pn_vector
#ifdef CONFIG_IEEE80211W						
						,unsigned char isMgmt
#endif						
                        )
{
    int i;

    mic_iv[0] = 0x59;
    if (qc_exists && a4_exists) mic_iv[1] = mpdu[30] & 0x0f;    /* QoS_TC           */
    if (qc_exists && !a4_exists) mic_iv[1] = mpdu[24] & 0x0f;   /* mute bits 7-4    */
    if (!qc_exists) mic_iv[1] = 0x00;

#ifdef CONFIG_IEEE80211W
	if(isMgmt) mic_iv[1] |= BIT(4);  // Management fields of Nonce. Figure 11-20 in IEEE 802.11-2012
#endif
	
    for (i = 2; i < 8; i++)
        mic_iv[i] = mpdu[i + 8];                    /* mic_iv[2:7] = A2[0:5] = mpdu[10:15] */
#ifdef CONSISTENT_PN_ORDER
        for (i = 8; i < 14; i++)
            mic_iv[i] = pn_vector[i - 8];           /* mic_iv[8:13] = PN[0:5] */
#else
        for (i = 8; i < 14; i++)
            mic_iv[i] = pn_vector[13 - i];          /* mic_iv[8:13] = PN[5:0] */
#endif
    mic_iv[14] = (unsigned char) (payload_length / 256);
    mic_iv[15] = (unsigned char) (payload_length % 256);
}


/************************************************/
/* construct_mic_header1()                      */
/* Builds the first MIC header block from       */
/* header fields.                               */
/************************************************/
void construct_mic_header1(
                        unsigned char *mic_header1,
                        int header_length,
                        unsigned char *mpdu
#ifdef CONFIG_IEEE80211W
						, unsigned char isMgmt
#endif						
                        )
{
    mic_header1[0] = (unsigned char)((header_length - 2) / 256);
    mic_header1[1] = (unsigned char)((header_length - 2) % 256);
	
#ifdef CONFIG_IEEE80211W
	if(isMgmt)
		mic_header1[2] = mpdu[0];
	else
#endif		
    mic_header1[2] = mpdu[0] & 0xcf;    /* Mute CF poll & CF ack bits */
    mic_header1[3] = mpdu[1] & 0xc7;    /* Mute retry, more data and pwr mgt bits */
    mic_header1[4] = mpdu[4];       /* A1 */
    mic_header1[5] = mpdu[5];
    mic_header1[6] = mpdu[6];
    mic_header1[7] = mpdu[7];
    mic_header1[8] = mpdu[8];
    mic_header1[9] = mpdu[9];
    mic_header1[10] = mpdu[10];     /* A2 */
    mic_header1[11] = mpdu[11];
    mic_header1[12] = mpdu[12];
    mic_header1[13] = mpdu[13];
    mic_header1[14] = mpdu[14];
    mic_header1[15] = mpdu[15];
}


/************************************************/
/* construct_mic_header2()                      */
/* Builds the last MIC header block from        */
/* header fields.                               */
/************************************************/
void construct_mic_header2(
                unsigned char *mic_header2,
                unsigned char *mpdu,
                int a4_exists,
                int qc_exists
                )
{
    int i;

    for (i = 0; i<16; i++) mic_header2[i]=0x00;

    mic_header2[0] = mpdu[16];    /* A3 */
    mic_header2[1] = mpdu[17];
    mic_header2[2] = mpdu[18];
    mic_header2[3] = mpdu[19];
    mic_header2[4] = mpdu[20];
    mic_header2[5] = mpdu[21];

    //mic_header2[6] = mpdu[22] & 0xf0;   /* SC */
    mic_header2[6] = 0x00;
    mic_header2[7] = 0x00; /* mpdu[23]; */


    if (!qc_exists && a4_exists)
    {
        for (i=0;i<6;i++) mic_header2[8+i] = mpdu[24+i];   /* A4 */

    }

    if (qc_exists && !a4_exists)
    {
        mic_header2[8] = mpdu[24] & 0x0f; /* mute bits 15 - 4 */
        mic_header2[9] = mpdu[25] & 0x00;
    }

    if (qc_exists && a4_exists)
    {
        for (i=0;i<6;i++) mic_header2[8+i] = mpdu[24+i];   /* A4 */

        mic_header2[14] = mpdu[30] & 0x0f;
        mic_header2[15] = mpdu[31] & 0x00;
    }


}


/************************************************/
/* construct_mic_header2()                      */
/* Builds the last MIC header block from        */
/* header fields.                               */
/************************************************/
void construct_ctr_preload(
                        unsigned char *ctr_preload,
                        int a4_exists,
                        int qc_exists,
                        unsigned char *mpdu,
                        unsigned char *pn_vector,
                        int c
#ifdef CONFIG_IEEE80211W						
						,unsigned char isMgmt
#endif						
                        )
{
    int i = 0;
    for (i=0; i<16; i++) ctr_preload[i] = 0x00;
    i = 0;

    ctr_preload[0] = 0x01;                                  /* flag */
    if (qc_exists && a4_exists) ctr_preload[1] = mpdu[30] & 0x0f;   /* QoC_Control */
    if (qc_exists && !a4_exists) ctr_preload[1] = mpdu[24] & 0x0f;

#ifdef CONFIG_IEEE80211W
	if(isMgmt)	ctr_preload[1] |= BIT(4);
#endif

    for (i = 2; i < 8; i++)
        ctr_preload[i] = mpdu[i + 8];                       /* ctr_preload[2:7] = A2[0:5] = mpdu[10:15] */
#ifdef CONSISTENT_PN_ORDER
      for (i = 8; i < 14; i++)
            ctr_preload[i] =    pn_vector[i - 8];           /* ctr_preload[8:13] = PN[0:5] */
#else
      for (i = 8; i < 14; i++)
            ctr_preload[i] =    pn_vector[13 - i];          /* ctr_preload[8:13] = PN[5:0] */
#endif
    ctr_preload[14] =  (unsigned char) (c / 256); /* Ctr */
    ctr_preload[15] =  (unsigned char) (c % 256);
}


/************************************/
/* bitwise_xor()                    */
/* A 128 bit, bitwise exclusive or  */
/************************************/
void bitwise_xor(unsigned char *ina, unsigned char *inb, unsigned char *out)
{
    int i;
    for (i=0; i<16; i++)
    {
        out[i] = ina[i] ^ inb[i];
    }
}


/*-----------------------------------------------------------------------------
hdr: wlanhdr
llc: llc_snap
pframe: raw data payload
plen: length of raw data payload
mic: mic of AES
------------------------------------------------------------------------------*/
static void aes_tx(struct rtl8192cd_priv *priv, UINT8 *key, UINT8 keyid,
			union PN48 *pn48, UINT8 *hdr, UINT8 *llc,
			UINT8 *pframe, UINT32 plen, UINT8* txmic
#ifdef CONFIG_IEEE80211W			
			, unsigned char isMgmt
			, unsigned char checkmic
#endif			
			)
{
	static UINT8	message[MAX_MSG_SIZE];
	UINT32	qc_exists, a4_exists, i, j, payload_remainder,
			num_blocks,payload_length, payload_index;

	UINT8 pn_vector[6];
	UINT8 mic_iv[16];
    UINT8 mic_header1[16];
    UINT8 mic_header2[16];
    UINT8 ctr_preload[16];

    /* Intermediate Buffers */
    UINT8 chain_buffer[16];
    UINT8 aes_out[16];
    UINT8 padded_buffer[16];
    UINT8 mic[8];

	UINT32	offset = 0;
	UINT32	hdrlen  = get_hdrlen(priv, hdr);
#ifdef RTK_129X_PLATFORM
	dma_addr_t tmpaddr;
#endif

	memset((void *)mic_iv, 0, 16);
	memset((void *)mic_header1, 0, 16);
	memset((void *)mic_header2, 0, 16);
	memset((void *)ctr_preload, 0, 16);
	memset((void *)chain_buffer, 0, 16);
	memset((void *)aes_out, 0, 16);
	memset((void *)padded_buffer, 0, 16);

	if (get_tofr_ds(hdr) != 0x03)
		a4_exists = 0;
	else
		a4_exists = 1;

	if (is_qos_data(hdr)) {
		qc_exists = 1;
		//hdrlen += 2;	// these 2 bytes has already added
	}
	else
		qc_exists = 0;

	// below is to collecting each frag(hdr, llc, pay, and mic into single message buf)

	// extiv (8 bytes long) should have been appended
	pn_vector[0]  = hdr[hdrlen]   = pn48->_byte_.TSC0;
	pn_vector[1]  = hdr[hdrlen+1] = pn48->_byte_.TSC1;
	hdr[hdrlen+2] =  0x00;
	hdr[hdrlen+3] = (0x20 | (keyid << 6));
	pn_vector[2]  = hdr[hdrlen+4] = pn48->_byte_.TSC2;
	pn_vector[3]  = hdr[hdrlen+5] = pn48->_byte_.TSC3;
	pn_vector[4]  = hdr[hdrlen+6] = pn48->_byte_.TSC4;
	pn_vector[5]  = hdr[hdrlen+7] = pn48->_byte_.TSC5;

	memcpy((void *)message, hdr, (hdrlen + 8)); //8 is for ext iv len
	offset = (hdrlen + 8);
#ifdef CONFIG_IEEE80211W	
	if(isMgmt)
		llc = 0;
#endif	

	if (llc)
	{
		memcpy((void *)(message + offset), (void *)llc, 8);
		offset += 8;
	}
	memcpy((void *)(message + offset), (void *)pframe, plen);
	offset += plen;

	// now we have collecting all the bytes into single message buf

	payload_length = plen; // 8 is for llc

	if (llc)
		payload_length += 8;

	construct_mic_iv(
                        mic_iv,
                        qc_exists,
                        a4_exists,
                        message,
                        (payload_length),
                        pn_vector   
#ifdef CONFIG_IEEE80211W						
						,isMgmt
#endif						
                        );

    construct_mic_header1(
                            mic_header1,
                            hdrlen,
                            message
#ifdef CONFIG_IEEE80211W					
									,isMgmt
#endif							

                            );
    construct_mic_header2(
                            mic_header2,
                            message,
                            a4_exists,
                            qc_exists
                            );


	payload_remainder = (payload_length) % 16;
    num_blocks = (payload_length) / 16;

    /* Find start of payload */
    payload_index = (hdrlen + 8);

    /* Calculate MIC */
    aes128k128d(key, mic_iv, aes_out);
    bitwise_xor(aes_out, mic_header1, chain_buffer);
    aes128k128d(key, chain_buffer, aes_out);
    bitwise_xor(aes_out, mic_header2, chain_buffer);
    aes128k128d(key, chain_buffer, aes_out);

	for (i = 0; i < num_blocks; i++)
    {
        bitwise_xor(aes_out, &message[payload_index], chain_buffer);

        payload_index += 16;
        aes128k128d(key, chain_buffer, aes_out);
    }

    /* Add on the final payload block if it needs padding */
    if (payload_remainder > 0)
    {
        for (j = 0; j < 16; j++) padded_buffer[j] = 0x00;
        for (j = 0; j < payload_remainder; j++)
        {
            padded_buffer[j] = message[payload_index++];
        }
        bitwise_xor(aes_out, padded_buffer, chain_buffer);
        aes128k128d(key, chain_buffer, aes_out);

    }

    for (j = 0 ; j < 8; j++) mic[j] = aes_out[j];

    /* Insert MIC into payload */
    for (j = 0; j < 8; j++)
    	message[payload_index+j] = mic[j];

	payload_index = hdrlen + 8;
	for (i=0; i< num_blocks; i++)
    {
        construct_ctr_preload(
                                ctr_preload,
                                a4_exists,
                                qc_exists,
                                message,
                                pn_vector,
                                i+1
#ifdef CONFIG_IEEE80211W								
								,isMgmt
#endif								
							);
        aes128k128d(key, ctr_preload, aes_out);
        bitwise_xor(aes_out, &message[payload_index], chain_buffer);
        for (j=0; j<16;j++) message[payload_index++] = chain_buffer[j];
    }

    if (payload_remainder > 0)          /* If there is a short final block, then pad it,*/
    {                                   /* encrypt it and copy the unpadded part back   */
        construct_ctr_preload(
                                ctr_preload,
                                a4_exists,
                                qc_exists,
                                message,
                                pn_vector,
                                num_blocks+1
#ifdef CONFIG_IEEE80211W								
								,isMgmt
#endif						
		);

        for (j = 0; j < 16; j++) padded_buffer[j] = 0x00;
        for (j = 0; j < payload_remainder; j++)
        {
            padded_buffer[j] = message[payload_index+j];
        }
        aes128k128d(key, ctr_preload, aes_out);
        bitwise_xor(aes_out, padded_buffer, chain_buffer);
        for (j=0; j<payload_remainder;j++) message[payload_index++] = chain_buffer[j];
    }

    /* Encrypt the MIC */
    construct_ctr_preload(
                        ctr_preload,
                        a4_exists,
                        qc_exists,
                        message,
                        pn_vector,
                        0	
#ifdef CONFIG_IEEE80211W								
						,isMgmt
#endif						
	);

    for (j = 0; j < 16; j++) padded_buffer[j] = 0x00;
    for (j = 0; j < 8; j++)
    {
        padded_buffer[j] = message[j+hdrlen+8+payload_length];
    }

    aes128k128d(key, ctr_preload, aes_out);
    bitwise_xor(aes_out, padded_buffer, chain_buffer);
    for (j=0; j<8;j++) message[payload_index++] = chain_buffer[j];

	// now, going to copy the final result back to the input buf...
	offset =0;

	//if (llc)
	{
		memcpy((void *)hdr, (void *)(&message[offset]), (hdrlen + 8 )); //8 is for ext iv
    	offset += (hdrlen + 8);
    }

    if (llc)
    {
    	memcpy((void *)llc, (void *)(&message[offset]),  8 ); //8 is for llc
    	offset +=  (8);
    }

		// Don't copy data to pframe
#ifdef CONFIG_IEEE80211W	
	if(!checkmic)
#endif	
    	memcpy((void *)pframe, (void *)(&message[offset]), (plen)); //now is for plen
    offset += (plen);


    memcpy((void *)txmic, (void *)(&message[offset]), 8); //now is for mic
    offset += 8;

#ifdef RTK_129X_PLATFORM
	tmpaddr = get_physical_addr(priv, hdr, hdrlen + 8, PCI_DMA_TODEVICE);
	rtl_cache_sync_wback(priv, tmpaddr, hdrlen + 8, PCI_DMA_TODEVICE);
	pci_unmap_single(priv->pshare->pdev, tmpaddr, hdrlen + 8, PCI_DMA_TODEVICE);
	if (llc) {
		tmpaddr = get_physical_addr(priv, llc, 8, PCI_DMA_TODEVICE);
		rtl_cache_sync_wback(priv, tmpaddr, 8, PCI_DMA_TODEVICE);
		pci_unmap_single(priv->pshare->pdev, tmpaddr, 8, PCI_DMA_TODEVICE);
	}

	tmpaddr = get_physical_addr(priv, pframe, plen, PCI_DMA_TODEVICE);
	rtl_cache_sync_wback(priv, tmpaddr, plen, PCI_DMA_TODEVICE);
	pci_unmap_single(priv->pshare->pdev, tmpaddr, plen, PCI_DMA_TODEVICE);

	tmpaddr = get_physical_addr(priv, txmic, 8, PCI_DMA_TODEVICE);
	rtl_cache_sync_wback(priv, tmpaddr, 8, PCI_DMA_TODEVICE);
	pci_unmap_single(priv->pshare->pdev, tmpaddr, 8, PCI_DMA_TODEVICE);
#elif defined(CONFIG_PCI_HCI)
	rtl_cache_sync_wback(priv, (unsigned long)hdr, hdrlen + 8, PCI_DMA_TODEVICE);
	if (llc)
		rtl_cache_sync_wback(priv, (unsigned long)llc, 8, PCI_DMA_TODEVICE);
	rtl_cache_sync_wback(priv, (unsigned long)pframe, plen, PCI_DMA_TODEVICE);
	rtl_cache_sync_wback(priv, (unsigned long)txmic, 8, PCI_DMA_TODEVICE);
#endif

    _DEBUG_INFO("--txmic=%X %X %X %X %X %X %X %X\n",
    txmic[0], txmic[1], txmic[2], txmic[3],
    txmic[4], txmic[5], txmic[6], txmic[7]);

   	if (pn48->val48 == 0xffffffffffffULL)
		pn48->val48 = 0;
	else
		pn48->val48++;
}

#ifdef CONFIG_IEEE80211W
void BIP_encrypt(struct rtl8192cd_priv *priv, unsigned char *pwlhdr,
				unsigned char *frag1,
				unsigned char *frag2, unsigned int frag2_len,
				unsigned char *frag3, 
				unsigned char isMgmt)
{
#ifdef CONFIG_IEEE80211W_BIPTEST
	unsigned char BIP_data[40]={0xc0,0x00,0xff,0xff,0xff,0xff,0xff,0xff,
	                        0x02,0x00,0x00,0x00,0x00,0x00,0x02,0x00,
	                        0x00,0x00,0x00,0x00,0x02,0x00,0x4c,0x10,
	                        0x04,0x00,0x04,0x00,0x00,0x00,0x00,0x00,
	                        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};

	// Test Data

	unsigned char IGTK[16]={0x4e,0xa9,0x54,0x3e,0x09,0xcf,0x2b,0x1e,0xca,0x66,0xff,0xc5,0x8b,0xde,0xcb,0xcf};
	unsigned char Addr[6]={0x02,0x00,0x00,0x00,0x00,0x00};
#else
unsigned char *BIP_data;	
#endif		
	unsigned char IMIC[16];

	int i;
	union PN48 *pn48;
	union PN48 BIPPN;
	
	unsigned char *da;
	unsigned char *ttkey = NULL;
	unsigned int	keyid = 0;
	struct stat_info	*pstat = NULL;
	UINT32	hdrlen  = get_hdrlen(priv, pwlhdr);
	
	unsigned char *pn;
	unsigned int offset;
	unsigned char pn_vector[6];
	
	printk("frag2_len=%d\n",frag2_len);
	frag2_len -= 10; // payload length = frag2_len - 10, MMIE Length = 10
	ttkey = GET_IGROUP_ENCRYP_KEY;	
	PMFDEBUG("IGTK=");
	for(i=0;i<16;i++)
		PMFDEBUG("%x",priv->pmib->dot11IGTKTable.dot11EncryptKey.dot11TTKey.skey[i]);
	PMFDEBUG("\n");
	pn48 = GET_IGROUP_ENCRYP_PN;
#ifdef CONFIG_IEEE80211W_BIPTEST	
	// test data
	memcpy(pwlhdr+10,Addr,6);
	memcpy(pwlhdr+16,Addr,6);
	ttkey = IGTK;
	BIPPN.val48 = 4;
	pn48 = &BIPPN;
#endif	
	pn_vector[0] = pn48->_byte_.TSC0;
	pn_vector[1] = pn48->_byte_.TSC1;
	pn_vector[2] = pn48->_byte_.TSC2;
	pn_vector[3] = pn48->_byte_.TSC3;
	pn_vector[4] = pn48->_byte_.TSC4;
	pn_vector[5] = pn48->_byte_.TSC5;
	
	keyid = 4;
#ifndef CONFIG_IEEE80211W_BIPTEST
	BIP_data = (unsigned char *)kmalloc(20+ frag2_len + 18, GFP_ATOMIC);
#endif

	// Generate BIP AAD (FC | A1 | A2 | A3)
	BIP_data[0] = pwlhdr[0];
	pwlhdr[1] &= 0xBF;
	BIP_data[1] = pwlhdr[1];
	memcpy(BIP_data+2,pwlhdr+4,18); // A1 | A2 | A3 (18 bytes)
	// Generate Management Frame Body
	memcpy(BIP_data+20,frag2,frag2_len);

	// Generate MMIE	
	offset = 20 + frag2_len;
	BIP_data[offset++] = 0x4c; // element ID (Management MIC frame)
	BIP_data[offset++] = 0x10; // MMIC length
	BIP_data[offset++] = 0x04; // Key ID
	BIP_data[offset++] = 0x00; // Key ID
	memcpy(BIP_data+offset,pn_vector,6); // IPN
	offset += 6;
	memset(BIP_data+offset,0,8);
#ifdef CONFIG_IEEE80211W_BIPTEST	
	printk("BIP_data len=%d data=",frag2_len);
	for(i=0;i<20+ frag2_len + 18;i++)
		printk("%02x ",BIP_data[i]);
	printk("\n");
#endif	
	memcpy(frag2+frag2_len,BIP_data+20+frag2_len,10);
	// Use AES-128-CMAC to generate IMIC
#ifdef CONFIG_IEEE80211W
	omac1_aes_128(ttkey,BIP_data,20+ frag2_len + 18,IMIC);
#endif
#ifdef CONFIG_IEEE80211W_BIPTEST	
	printk("IMIC=");
	for(i=0;i<16;i++)
		printk("%02x",IMIC[i]);
	printk("\n");
#endif	
	memcpy(frag3,IMIC,8);
#ifdef CONFIG_PCI_HCI
	rtl_cache_sync_wback(priv, (unsigned long)pwlhdr, hdrlen, PCI_DMA_TODEVICE);
	rtl_cache_sync_wback(priv, (unsigned long)frag2, frag2_len+10, PCI_DMA_TODEVICE);
	rtl_cache_sync_wback(priv, (unsigned long)frag3, 8, PCI_DMA_TODEVICE);
#endif
	
	kfree(BIP_data);	
		
	if (pn48->val48 == 0xffffffffffffULL)
		pn48->val48 = 0;
	else
		pn48->val48++;

}

/*when STA mode rx broadcast deauth , check Management MIC Information Element (MMIE)
campare least 8 bytes(MIC)
ret:1 success
ret:0 fail
*/

int MMIE_check(struct rtl8192cd_priv *priv, struct rx_frinfo *pfrinfo)
{

#define FC_LEN 2
#define DUR_LEN 2
#define A1A2A3_LEN 18
#define SEQ_LEN 2
#define REASON_LEN 2
#define MMIE_LEN 10
#define MIC_LEN 8

    unsigned char *pframe = get_pframe(pfrinfo);
	unsigned char *ttkey = NULL;
    unsigned char BIP_data[40];
    unsigned char MIC_be_chk[16];    // 16 ? why not 8?
    #ifdef CONFIG_IEEE80211W_CLI_DEBUG 
    int i=0;
    #endif 

    memset(BIP_data,0,40);
    memset(MIC_be_chk,0,16);    
    
	ttkey = GET_IGROUP_ENCRYP_KEY;	 

    #ifdef CONFIG_IEEE80211W_CLI_DEBUG 
	PMFDEBUG("IGTK=");
	for(i=0;i<16;i++)
		PMFDEBUG("%x ",priv->pmib->dot11IGTKTable.dot11EncryptKey.dot11TTKey.skey[i]);
	PMFDEBUG("\n");
    #endif
    
	/*cp BIP AAD (FC | A1 | A2 | A3)*/ 
    memcpy(&BIP_data[0],pframe,FC_LEN);  // cp FC
    memcpy(&BIP_data[FC_LEN],pframe+FC_LEN+DUR_LEN ,A1A2A3_LEN );  // cp A1 , A2 ,A3

	/*cp Management Frame Body*/ 
    memcpy(&BIP_data[FC_LEN + A1A2A3_LEN],pframe+FC_LEN+DUR_LEN+A1A2A3_LEN+SEQ_LEN ,REASON_LEN );  

	/*cp MMIE	*/ 
    memcpy(&BIP_data[FC_LEN + A1A2A3_LEN + REASON_LEN],pframe+FC_LEN+DUR_LEN+A1A2A3_LEN+SEQ_LEN+REASON_LEN ,MMIE_LEN );   
   

	/*Use AES-128-CMAC to generate IMIC*/ 
	omac1_aes_128(ttkey,BIP_data,FC_LEN + A1A2A3_LEN + REASON_LEN + MMIE_LEN + MIC_LEN , MIC_be_chk );

    #ifdef CONFIG_IEEE80211W_CLI_DEBUG	
        PMFDEBUG("IMIC=");
        for(i=0;i<16;i++)
            PMFDEBUG("%02x",MIC_be_chk[i]);
        PMFDEBUG("\n");
    #endif	
    
    
    if(!memcmp(MIC_be_chk, pframe + BIP_HEADER_LEN + 10, MIC_LEN))
        return 1; //_SUCCESS ; //match!
    else    
        return 0; //_FAIL ;

}

int aesccmp_checkmic(struct rtl8192cd_priv *priv, struct rx_frinfo *pfrinfo, unsigned char *pmic)
{
	unsigned char *sa;
	unsigned char *ttkey = NULL;
	union PN48 	  pn48;
	unsigned int	keyid = 0;
	struct stat_info	*pstat = NULL;
	unsigned int i;
	unsigned char *pframe = get_pframe(pfrinfo);
	unsigned char calmic[8];
	unsigned char data[1460];
	sa = get_sa(pframe);
	
	if (OPMODE & WIFI_AP_STATE 
#ifdef CONFIG_IEEE80211W_CLI
	   || (OPMODE & WIFI_STATION_STATE)		
#endif 
	)
	{
		if (!IS_MCAST(sa))
		{
			pstat = get_stainfo(priv, sa);
			
			if (pstat == NULL) {
				DEBUG_ERR("tx aes pstat == NULL\n");
				return 0;
			}
			
			ttkey = GET_UNICAST_ENCRYP_KEY;

			pn48._byte_.TSC0 = pframe[pfrinfo->hdr_len];
			pn48._byte_.TSC1 = pframe[pfrinfo->hdr_len+1];
			pn48._byte_.TSC2 = pframe[pfrinfo->hdr_len+4];
			pn48._byte_.TSC3 = pframe[pfrinfo->hdr_len+5];
			pn48._byte_.TSC4 = pframe[pfrinfo->hdr_len+6];
			pn48._byte_.TSC5 = pframe[pfrinfo->hdr_len+7];
			keyid = 0;
			
		}
	}

	if ((ttkey == NULL)) {
		DEBUG_ERR("no encrypt key for AES due to ttkey=NULL\n");
		return 0;
	}

	memcpy(data, pframe + pfrinfo->hdr_len + 8, pfrinfo->pktlen - pfrinfo->hdr_len - 8 - 8);
	
	aes_tx(priv, ttkey, keyid, 
	       &pn48, pframe, 0, 
		   pframe + pfrinfo->hdr_len + 8, pfrinfo->pktlen - pfrinfo->hdr_len - 8 - 8,
		   calmic, 1, 1);

	if(!memcmp(pmic, calmic,8))
		return 1;
	else
		return 0;
}

#endif // CONFIG_IEEE80211W

void aesccmp_encrypt(struct rtl8192cd_priv *priv, unsigned char *pwlhdr,
				unsigned char *frag1,
				unsigned char *frag2, unsigned int frag2_len,
				unsigned char *frag3
#ifdef CONFIG_IEEE80211W				
				, unsigned char isMgmt
#endif				
			)
{
#ifdef CONFIG_IEEE80211W_TEST	
	unsigned char CCMPTK[16]={0x66,0xed,0x21,0x04,0x2f,0x9f,0x26,0xd7,0x11,0x57,0x06,0xe4,0x04,0x14,0xcf,0x2e}; // test for PMF
	unsigned char testDa[6]={0x02,0x00,0x00,0x00,0x01,0x00};// test for PMF (unicast Deauth frame)
	//unsigned char testDa[6]={0xff,0xff,0xff,0xff,0xff,0xff};// test for PMF (broadcast Deauth frame)
	unsigned char testSa[6]={0x02,0x00,0x00,0x00,0x00,0x00};// test for PMF
	union PN48 CCMPPN;// test for PMF
#endif

	unsigned char *da;
	unsigned char *ttkey = NULL;
	union PN48 	  *pn48 = NULL;
	unsigned int	keyid = 0;
	struct stat_info	*pstat = NULL;

#ifdef CONFIG_IEEE80211W_TEST		
	if(isMgmt) {
		printk("isMgmt\n");
		memcpy(pwlhdr+4,testDa,6);// test for PMF
		memcpy(pwlhdr+10,testSa,6);
		// test for PMF
		memcpy(pwlhdr+16,testSa,6);// test for PMF
		CCMPPN.val48 = 1;// test for PMF (unicast)
	}
#endif	

	da = get_da(pwlhdr);

	if (OPMODE & WIFI_AP_STATE)
	{
#if defined(WDS) || defined(CONFIG_RTK_MESH) || defined(A4_STA)
		unsigned int to_fr_ds = (GetToDs(pwlhdr) << 1) | GetFrDs(pwlhdr);
		if (to_fr_ds == 3)
			da = GetAddr1Ptr(pwlhdr);
#endif

		if (IS_MCAST(da))
		{
			if(priv->pmib->dot11GroupKeysTable.keyid == GKEY_ID_SECOND)
				ttkey = GET_GROUP_ENCRYP2_KEY;
			else
				ttkey = GET_GROUP_ENCRYP_KEY;
			pn48 = GET_GROUP_ENCRYP_PN;
			//keyid = 1;
			keyid = priv->pmib->dot11GroupKeysTable.keyid;//use mib keyid
		}
		else
		{
#ifdef CONFIG_IEEE80211W_TEST		
			// marked test for PMF
			if(isMgmt) { // when test and Mgmt frame
				ttkey = CCMPTK;
				pn48 = &CCMPPN; 			
			}
			else 
#endif			
			{
				pstat = get_stainfo(priv, da);
				if (pstat == NULL) {
					DEBUG_ERR("tx aes pstat == NULL\n");
					return;
				}
				ttkey = GET_UNICAST_ENCRYP_KEY;
				pn48 = GET_UNICAST_ENCRYP_PN;
			}
#ifdef CONFIG_IEEE80211W_TEST	
			if(isMgmt) {
				printk("isUnicast\n");
				printk("ttkey=");
				for(i=0;i<16;i++)
				printk("%02x",ttkey[i]);
				printk("\n");
				printk("pn48=%02x%02x%02x%02x%02x%02x\n",pn48->_byte_.TSC0,pn48->_byte_.TSC1,pn48->_byte_.TSC2,pn48->_byte_.TSC3,pn48->_byte_.TSC4,pn48->_byte_.TSC5);
			}
			ttkey = GET_UNICAST_ENCRYP_KEY;		
			pn48 = GET_UNICAST_ENCRYP_PN;
#endif
			keyid = 0;
		}
	}
#ifdef CLIENT_MODE
	else if (OPMODE & WIFI_STATION_STATE)
	{

		if (IS_MCAST(da)) //eric-ath
		{
			if(priv->pmib->dot11GroupKeysTable.keyid == GKEY_ID_SECOND)
				ttkey = GET_GROUP_ENCRYP2_KEY;
			else
				ttkey = GET_GROUP_ENCRYP_KEY;
			pn48 = GET_GROUP_ENCRYP_PN;
			keyid = priv->pmib->dot11GroupKeysTable.keyid;
		}
		else
		{
		pstat = get_stainfo(priv, BSSID);
		if (pstat == NULL) {
			DEBUG_ERR("tx aes pstat == NULL\n");
			return;
		}
		ttkey = GET_UNICAST_ENCRYP_KEY;
		pn48 = GET_UNICAST_ENCRYP_PN;
			
		keyid = 0;

		}
	}
	else if (OPMODE & WIFI_ADHOC_STATE)
	{
		ttkey = GET_GROUP_ENCRYP_KEY;
		pn48 = GET_GROUP_ENCRYP_PN;
		keyid = 0;
	}
#endif

	if ((ttkey == NULL) || (pn48 == NULL)) {
		DEBUG_ERR("no encrypt key for AES due to ttkey=NULL or pn48=NULL\n");
		return;
	}
#ifdef CONFIG_IEEE80211W
	if (isMgmt)
		frag1 = 0;
#endif		

	aes_tx(priv, ttkey, keyid, pn48, pwlhdr, frag1, frag2, frag2_len, frag3
#ifdef CONFIG_IEEE80211W	
		 	,isMgmt
			, 0
#endif		
		   );
}


static void aes_rx(UINT8 *ttkey, UINT8 qc_exists, UINT8 a4_exists,
				UINT8 *pframe, UINT32 hdrlen, UINT32 plen
#ifdef CONFIG_IEEE80211W				
				, unsigned char isMgmt
#endif
			)
{
	UINT32	i, j, payload_remainder,
			num_blocks, payload_index;

	UINT8 pn_vector[6];
	UINT8 mic_iv[16];
    UINT8 mic_header1[16];
    UINT8 mic_header2[16];
    UINT8 ctr_preload[16];

    UINT8 chain_buffer[16];
    UINT8 aes_out[16];
    UINT8 padded_buffer[16];
    UINT8 mic[8];

	memset((void *)mic_iv, 0, 16);
	memset((void *)mic_header1, 0, 16);
	memset((void *)mic_header2, 0, 16);
	memset((void *)ctr_preload, 0, 16);
	memset((void *)chain_buffer, 0, 16);
	memset((void *)aes_out, 0, 16);
	memset((void *)mic, 0, 8);

	num_blocks = plen / 16; //(plen including llc, payload_length and mic )

	payload_remainder = plen % 16;

	pn_vector[0]  = pframe[hdrlen];
	pn_vector[1]  = pframe[hdrlen+1];
	pn_vector[2]  = pframe[hdrlen+4];
	pn_vector[3]  = pframe[hdrlen+5];
	pn_vector[4]  = pframe[hdrlen+6];
	pn_vector[5]  = pframe[hdrlen+7];

	// now, decrypt pframe with hdrlen offset and plen long

	payload_index = hdrlen + 8; // 8 is for extiv
	for (i=0; i< num_blocks; i++)
    {
        construct_ctr_preload(
                                ctr_preload,
                                a4_exists,
                                qc_exists,
                                pframe,
                                pn_vector,
                                i+1       
#ifdef CONFIG_IEEE80211W								
								,isMgmt
#endif								
                            );

        aes128k128d(ttkey, ctr_preload, aes_out);
        bitwise_xor(aes_out, &pframe[payload_index], chain_buffer);

        for (j=0; j<16;j++) pframe[payload_index++] = chain_buffer[j];
    }

    if (payload_remainder > 0)          /* If there is a short final block, then pad it,*/
    {                                   /* encrypt it and copy the unpadded part back   */
        construct_ctr_preload(
                                ctr_preload,
                                a4_exists,
                                qc_exists,
                                pframe,
                                pn_vector,
                                num_blocks+1
#ifdef CONFIG_IEEE80211W								
								,isMgmt
#endif								
                            );

        for (j = 0; j < 16; j++) padded_buffer[j] = 0x00;
        for (j = 0; j < payload_remainder; j++)
        {
            padded_buffer[j] = pframe[payload_index+j];
        }
        aes128k128d(ttkey, ctr_preload, aes_out);
        bitwise_xor(aes_out, padded_buffer, chain_buffer);
        for (j=0; j<payload_remainder;j++) pframe[payload_index++] = chain_buffer[j];
    }
}


unsigned int aesccmp_decrypt(struct rtl8192cd_priv *priv, struct rx_frinfo *pfrinfo
#ifdef CONFIG_IEEE80211W				
								, unsigned char isMgmt
#endif
								)
{
	UINT8	*ttkey = NULL;
	UINT32	hdrlen, keylen = 0;
	UINT8	a4_exists;
	UINT8	qc_exists;
	UINT8	*sa	= pfrinfo->sa;
	UINT8	*pframe = get_pframe(pfrinfo);
	UINT8	to_fr_ds = pfrinfo->to_fr_ds;
	struct stat_info	*pstat = NULL;
#ifdef CONFIG_IEEE80211W_TEST	
		
		unsigned char CCMPTK[16]={0x66,0xed,0x21,0x04,0x2f,0x9f,0x26,0xd7,0x11,0x57,0x06,0xe4,0x04,0x14,0xcf,0x2e}; // test for PMF
		unsigned char testDa[6]={0x02,0x00,0x00,0x00,0x01,0x00};// test for PMF (unicast Deauth frame)
		//unsigned char testDa[6]={0xff,0xff,0xff,0xff,0xff,0xff};// test for PMF (broadcast Deauth frame)
		unsigned char testSa[6]={0x02,0x00,0x00,0x00,0x00,0x00};// test for PMF
		unsigned char testIV[8]={0x01,0x00,0x00,0x20,0x00,0x00,0x00,0x00};
		unsigned char testdata[10]={0x1d,0x07,0xca,0xfd,0x04,0x09,0xbb,0x8b,0xaf,0xef};
			
		if(isMgmt) {
			printk("isMgmt\n");
			pframe[0]=0xc0;
			pframe[1]=0x00;
			memcpy(pframe+4,testDa,6);// test for PMF
			memcpy(pframe+10,testSa,6);
			// test for PMF
			memcpy(pframe+16,testSa,6);// test for PMF
			memcpy(pframe+24,testIV,8);
			memcpy(pframe+32,testdata,10);
			pfrinfo->pktlen = 42;
		}
#endif

	if (to_fr_ds != 0x03) {
		hdrlen = WLAN_HDR_A3_LEN;
		a4_exists = 0;
	}
	else {
		hdrlen = WLAN_HDR_A4_LEN;
		a4_exists = 1;
	}

	if (is_qos_data(pframe)) {
		qc_exists = 1;
		hdrlen += 2;
	}
	else
		qc_exists = 0;

	if (OPMODE & WIFI_AP_STATE)
	{
#if defined(WDS) || defined(CONFIG_RTK_MESH) || defined(A4_STA)
		if (to_fr_ds == 3)
			pstat = get_stainfo (priv, GetAddr2Ptr(pframe));
		else			
#endif
		pstat = get_stainfo (priv, sa);

#ifdef CONFIG_IEEE80211W_TEST		
		// marked test for PMF
		if(isMgmt) { // when test and Mgmt frame
			ttkey = CCMPTK;
			keylen = 16;
		}
		else 
#endif	
		{
		if (pstat == NULL)
		{
			DEBUG_ERR("AES Rx fails! sa=%02X%02X%02X%02X%02X%02X\n",
				sa[0], sa[1], sa[2], sa[3], sa[4], sa[5]);
			return FALSE;
		}

		keylen = GET_UNICAST_ENCRYP_KEYLEN;
		ttkey  = GET_UNICAST_ENCRYP_KEY;
		}
	}
#ifdef CLIENT_MODE
	else if (OPMODE & WIFI_STATION_STATE)
	{
		if (IS_MCAST(pfrinfo->da))
		{
			if((pframe[WLAN_HDR_A3_LEN + 3]&BIT(7))){
				keylen = GET_GROUP_IDX2_ENCRYP_KEYLEN;
				ttkey  = GET_GROUP_ENCRYP2_KEY;
			}
			else{				
				keylen = GET_GROUP_ENCRYP_KEYLEN;
				ttkey  = GET_GROUP_ENCRYP_KEY;
			}
		}
		else
		{
			pstat = get_stainfo(priv, BSSID);
			if (pstat == NULL) {
				DEBUG_ERR("rx aes pstat == NULL\n");
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
		DEBUG_ERR("no descrypt key for AES due to keylen=0\n");
		return FALSE;
	}
#ifdef CONFIG_IEEE80211W_TEST	
	if(isMgmt) {
		int i;
		printk("pframe=");
		for(i=0;i<pfrinfo->pktlen;i++)
			printk("%02x ",pframe[i]);
		printk("\n");
		printk("ttkey=");
		for(i=0;i<keylen;i++)
			printk("%02x ",ttkey[i]);
		printk("\n");
		printk("hdrlen=%d\n",hdrlen);
	}
#endif		

	aes_rx(ttkey, qc_exists, a4_exists, pframe, hdrlen, pfrinfo->pktlen-hdrlen-8
#ifdef CONFIG_IEEE80211W				
			, isMgmt
#endif		
			);
	return TRUE;
}

