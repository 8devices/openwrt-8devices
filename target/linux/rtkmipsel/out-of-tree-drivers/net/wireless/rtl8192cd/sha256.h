/*
 * SHA256 hash implementation and interface functions
 * Copyright (c) 2003-2006, Jouni Malinen <j@w1.fi>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * Alternatively, this software may be distributed under the terms of BSD
 * license.
 *
 * See README and COPYING for more details.
 */

#ifndef SHA256_H
#define SHA256_H

#define SHA256_MAC_LEN 32

//typedef unsigned long u64;
//typedef unsigned int u32;
//typedef unsigned short u16;
//typedef unsigned char u8;

/**
 * sha256_vector - SHA256 hash for data vector
 * @num_elem: Number of elements in the data vector
 * @addr: Pointers to the data areas
 * @len: Lengths of the data blocks
 * @mac: Buffer for the hash
 * Returns: 0 on success, -1 on failure
 */
 
#define WPA_GET_BE16(a) ((u2Byte) (((a)[0] << 8) | (a)[1]))
#define WPA_PUT_BE16(a, val)			\
	do {					\
		(a)[0] = ((u2Byte) (val)) >> 8;	\
		(a)[1] = ((u2Byte) (val)) & 0xff;	\
	} while (0)

#define WPA_GET_LE16(a) ((u2Byte) (((a)[1] << 8) | (a)[0]))
#define WPA_PUT_LE16(a, val)			\
	do {					\
		(a)[1] = ((u2Byte) (val)) >> 8;	\
		(a)[0] = ((u2Byte) (val)) & 0xff;	\
	} while (0)

#define WPA_GET_BE24(a) ((((u32) (a)[0]) << 16) | (((u4Byte) (a)[1]) << 8) | \
			 ((u4Byte) (a)[2]))
#define WPA_PUT_BE24(a, val)					\
	do {							\
		(a)[0] = (u1Byte) ((((u4Byte) (val)) >> 16) & 0xff);	\
		(a)[1] = (u1Byte) ((((u4Byte) (val)) >> 8) & 0xff);	\
		(a)[2] = (u1Byte) (((u4Byte) (val)) & 0xff);		\
	} while (0)

#define WPA_GET_BE32(a) ((((u4Byte) (a)[0]) << 24) | (((u4Byte) (a)[1]) << 16) | \
			 (((u4Byte) (a)[2]) << 8) | ((u4Byte) (a)[3]))
#define WPA_PUT_BE32(a, val)					\
	do {							\
		(a)[0] = (u1Byte) ((((u4Byte) (val)) >> 24) & 0xff);	\
		(a)[1] = (u1Byte) ((((u4Byte) (val)) >> 16) & 0xff);	\
		(a)[2] = (u1Byte) ((((u4Byte) (val)) >> 8) & 0xff);	\
		(a)[3] = (u1Byte) (((u4Byte) (val)) & 0xff);		\
	} while (0)

#define WPA_GET_LE32(a) ((((u4Byte) (a)[3]) << 24) | (((u4Byte) (a)[2]) << 16) | \
			 (((u4Byte) (a)[1]) << 8) | ((u4Byte) (a)[0]))
#define WPA_PUT_LE32(a, val)					\
	do {							\
		(a)[3] = (u1Byte) ((((u4Byte) (val)) >> 24) & 0xff);	\
		(a)[2] = (u1Byte) ((((u4Byte) (val)) >> 16) & 0xff);	\
		(a)[1] = (u1Byte) ((((u4Byte) (val)) >> 8) & 0xff);	\
		(a)[0] = (u1Byte) (((u4Byte) (val)) & 0xff);		\
	} while (0)

#define WPA_GET_BE64(a) ((((u8Byte) (a)[0]) << 56) | (((u8Byte) (a)[1]) << 48) | \
			 (((u8Byte) (a)[2]) << 40) | (((u8Byte) (a)[3]) << 32) | \
			 (((u8Byte) (a)[4]) << 24) | (((u8Byte) (a)[5]) << 16) | \
			 (((u8Byte) (a)[6]) << 8) | ((u8Byte) (a)[7]))
#define WPA_PUT_BE64(a, val)				\
	do {						\
		(a)[0] = (u1Byte) (((u8Byte) (val)) >> 56);	\
		(a)[1] = (u1Byte) (((u8Byte) (val)) >> 48);	\
		(a)[2] = (u1Byte) (((u8Byte) (val)) >> 40);	\
		(a)[3] = (u1Byte) (((u8Byte) (val)) >> 32);	\
		(a)[4] = (u1Byte) (((u8Byte) (val)) >> 24);	\
		(a)[5] = (u1Byte) (((u8Byte) (val)) >> 16);	\
		(a)[6] = (u1Byte) (((u8Byte) (val)) >> 8);	\
		(a)[7] = (u1Byte) (((u8Byte) (val)) & 0xff);	\
	} while (0)

#define WPA_GET_LE64(a) ((((u8Byte) (a)[7]) << 56) | (((u8Byte) (a)[6]) << 48) | \
			 (((u8Byte) (a)[5]) << 40) | (((u8Byte) (a)[4]) << 32) | \
			 (((u8Byte) (a)[3]) << 24) | (((u8Byte) (a)[2]) << 16) | \
			 (((u8Byte) (a)[1]) << 8) | ((u8Byte) (a)[0]))


int sha256_vector(size_t num_elem, const unsigned char *addr[], const size_t *len,
			 unsigned char *mac);



int hmac_sha256_vector(const unsigned char *key, size_t key_len, size_t num_elem,
		      const unsigned char *addr[], const size_t *len, unsigned char *mac);
#ifndef __ECOS
int hmac_sha256(const unsigned char *key, size_t key_len, const unsigned char *data,
		 size_t data_len, unsigned char *mac);
#endif
void sha256_prf(const unsigned char *key, size_t key_len, const char *label,
	      const unsigned char *data, size_t data_len, unsigned char *buf, size_t buf_len);

#endif /* SHA256_H */
