/* crypto/aes/aes_cbc.c -*- mode:C; c-file-style: "eay" -*- */
/* ====================================================================
 * Copyright (c) 1998-2002 The OpenSSL Project.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *
 * 3. All advertising materials mentioning features or use of this
 *    software must display the following acknowledgment:
 *    "This product includes software developed by the OpenSSL Project
 *    for use in the OpenSSL Toolkit. (http://www.openssl.org/)"
 *
 * 4. The names "OpenSSL Toolkit" and "OpenSSL Project" must not be used to
 *    endorse or promote products derived from this software without
 *    prior written permission. For written permission, please contact
 *    openssl-core@openssl.org.
 *
 * 5. Products derived from this software may not be called "OpenSSL"
 *    nor may "OpenSSL" appear in their names without prior written
 *    permission of the OpenSSL Project.
 *
 * 6. Redistributions of any form whatsoever must retain the following
 *    acknowledgment:
 *    "This product includes software developed by the OpenSSL Project
 *    for use in the OpenSSL Toolkit (http://www.openssl.org/)"
 *
 * THIS SOFTWARE IS PROVIDED BY THE OpenSSL PROJECT ``AS IS'' AND ANY
 * EXPRESSED OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE OpenSSL PROJECT OR
 * ITS CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 * ====================================================================
 *
 */

#ifndef AES_DEBUG
# ifndef NDEBUG
#  define NDEBUG
# endif
#endif

//#include "rtl_types.h"
//#include "rtl_glue.h"
#ifdef CONFIG_RTL_ICTEST
#include "rtl_types.h"
#else
#include <net/rtl/rtl_types.h>
#endif
#include <net/rtl/rtl_glue.h>
#include "aes.h"
#include "aes_locl.h"

void soft_AES_cbc_encrypt(const uint8 *in, uint8 *out,
		     const uint32 length, const AES_KEY *key,
		     uint8 *ivec, const int32 enc)
{

	uint32 n;
	uint32 len = length;
	uint8 tmp[AES_BLOCK_SIZE];

	assert(in && out && key && ivec);
	assert((AES_ENCRYPT == enc)||(AES_DECRYPT == enc));

	if (AES_ENCRYPT == enc) {
		while (len >= AES_BLOCK_SIZE) {
			for(n=0; n < sizeof tmp; ++n)
				tmp[n] = in[n] ^ ivec[n];
			AES_encrypt(tmp, out, key);
			memcpy(ivec, out, AES_BLOCK_SIZE);
			len -= AES_BLOCK_SIZE;
			in += AES_BLOCK_SIZE;
			out += AES_BLOCK_SIZE;
		}
		if (len) {
			for(n=0; n < len; ++n)
				tmp[n] = in[n] ^ ivec[n];
			for(n=len; n < AES_BLOCK_SIZE; ++n)
				tmp[n] = ivec[n];
			AES_encrypt(tmp, tmp, key);
			memcpy(out, tmp, len);
			memcpy(ivec, tmp, sizeof tmp);
		}
	} else {
		while (len >= AES_BLOCK_SIZE) {
			memcpy(tmp, in, sizeof tmp);
			AES_decrypt(in, out, key);
			for(n=0; n < AES_BLOCK_SIZE; ++n)
				out[n] ^= ivec[n];
			memcpy(ivec, tmp, AES_BLOCK_SIZE);
			len -= AES_BLOCK_SIZE;
			in += AES_BLOCK_SIZE;
			out += AES_BLOCK_SIZE;
		}
		if (len) {
			memcpy(tmp, in, sizeof tmp);
			AES_decrypt(tmp, tmp, key);
			for(n=0; n < len; ++n)
				out[n] ^= ivec[n];
			memcpy(ivec, tmp, sizeof tmp);
		}
	}
}


void AES_cfb128_encrypt(const uint8 *in, uint8 *out,
	const uint32 length, const AES_KEY *key,
	uint8 *ivec, int32 *num, const int32 enc)
{

	uint32 n;
	uint32 l = length;
	uint8 c;

	assert(in && out && key && ivec && num);

	n = *num;

	if (enc) {
		while (l--) {
			if (n == 0) {
				AES_encrypt(ivec, ivec, key);
			}
			ivec[n] = *(out++) = *(in++) ^ ivec[n];
			n = (n+1) % AES_BLOCK_SIZE;
		}
	} else {
		while (l--) {
			if (n == 0) {
				AES_encrypt(ivec, ivec, key);
			}
			c = *(in);
			*(out++) = *(in++) ^ ivec[n];
			ivec[n] = c;
			n = (n+1) % AES_BLOCK_SIZE;
		}
	}

	*num=n;
}


/* increment counter (128-bit int) by 2^64 */
static void AES_ctr128_inc(uint8 *counter)
{
	uint32 c;

	/* Grab 3rd dword of counter and increment */
#ifdef L_ENDIAN
	c = GETU32(counter + 8);
	c++;
	PUTU32(counter + 8, c);
#else
	c = GETU32(counter + 4);
	c++;
	PUTU32(counter + 4, c);
#endif

	/* if no overflow, we're done */
	if (c)
		return;

	/* Grab top dword of counter and increment */
#ifdef L_ENDIAN
	c = GETU32(counter + 12);
	c++;
	PUTU32(counter + 12, c);
#else
	c = GETU32(counter +  0);
	c++;
	PUTU32(counter +  0, c);
#endif

}

/* The input encrypted as though 128bit counter mode is being
 * used.  The extra state information to record how much of the
 * 128bit block we have used is contained in *num, and the
 * encrypted counter is kept in ecount_buf.  Both *num and
 * ecount_buf must be initialised with zeros before the first
 * call to AES_ctr128_encrypt().
 */
void AES_ctr128_encrypt(const uint8 *in, uint8 *out,
	const uint32 length, const AES_KEY *key,
	uint8 counter[AES_BLOCK_SIZE],
	uint8 ecount_buf[AES_BLOCK_SIZE],
	uint32 *num)
{

	uint32 n;
	uint32 l=length;

	assert(in && out && key && counter && num);
	assert(*num < AES_BLOCK_SIZE);

	n = *num;

	while (l--) {
		if (n == 0) {
			AES_encrypt(counter, ecount_buf, key);
#if 1 /* Louis: According to RFC 3686, the word[3] is increased every block. */
			{
				uint32 c;
				c = GETU32(counter + 12);
				c++;
				PUTU32(counter + 12, c);
				while(0) AES_ctr128_inc(counter);
			}
#else
			AES_ctr128_inc(counter);
#endif
		}
		*(out++) = *(in++) ^ ecount_buf[n];
		n = (n+1) % AES_BLOCK_SIZE;
	}

	*num=n;
}


/* The input and output encrypted as though 128bit ofb mode is being
 * used.  The extra state information to record how much of the
 * 128bit block we have used is contained in *num;
 */
void AES_ofb128_encrypt(const uint8 *in, uint8 *out, const uint32 length, const AES_KEY *key,
	uint8 *ivec, int32 *num)
{

	unsigned int n;
	unsigned long l=length;

	assert(in && out && key && ivec && num);

	n = *num;

	while (l--) {
		if (n == 0) {
			AES_encrypt(ivec, ivec, key);
		}
		*(out++) = *(in++) ^ ivec[n];
		n = (n+1) % AES_BLOCK_SIZE;
	}

	*num=n;
}


void AES_ecb_encrypt(const uint8 *in, uint8 *out,
		     const AES_KEY *key, const int32 enc)
{

    assert(in && out && key);
	assert((AES_ENCRYPT == enc)||(AES_DECRYPT == enc));

	if (AES_ENCRYPT == enc)
		AES_encrypt(in, out, key);
	else
		AES_decrypt(in, out, key);
}
