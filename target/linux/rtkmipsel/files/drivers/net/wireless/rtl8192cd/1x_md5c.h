/*
 * 8021x_md5c.h - header file for MD5C.C
 */

#ifndef P8021X_MD5C_H
#define P8021X_MD5C_H

/* Copyright (C) 1991-2, RSA Data Security, Inc. Created 1991. All
rights reserved.

License to copy and use this software is granted provided that it
is identified as the "RSA Data Security, Inc. MD5 Message-Digest
Algorithm" in all material mentioning or referencing this software
or this function.

License is also granted to make and use derivative works provided
that such works are identified as "derived from the RSA Data
Security, Inc. MD5 Message-Digest Algorithm" in all material
mentioning or referencing the derived work.

RSA Data Security, Inc. makes no representations concerning either
the merchantability of this software or the suitability of this

software for any particular purpose. It is provided "as is"
without express or implied warranty of any kind.

These notices must be retained in any copies of any part of this
documentation and/or software.
 */


/* MD5_TYPE.H - RSAREF types and constants */

/* POINTER defines a generic pointer type */
typedef unsigned char *POINTER;

/* UINT2 defines a two byte word */
typedef unsigned short int UINT2;

/* UINT4 defines a four byte word */
typedef unsigned int UINT4;

#if 0
typedef unsigned char UINT8;
typedef unsigned long UINT32;
typedef int INT32;
#endif


/* MD5 context. */
typedef struct {
  UINT4 state[4];                                   /* state (ABCD) */
  UINT4 count[2];        /* number of bits, modulo 2^64 (lsb first) */
  unsigned char buffer[64];                         /* input buffer */
} MD5_CTX;

void wlan_MD5_Init(MD5_CTX *);
void wlan_MD5_Update(MD5_CTX *, UINT8 *, UINT32);
void wlan_MD5_Final(unsigned char [16], MD5_CTX *);
//void hmac_md5(UINT8 *, INT32, UINT8 *  , INT32, UINT8 *  );


#endif
