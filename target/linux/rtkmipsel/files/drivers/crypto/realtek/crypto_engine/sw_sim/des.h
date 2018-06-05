/*
* --------------------------------------------------------------------
* Copyright c                  Realtek Semiconductor Corporation, 2002
* All rights reserved.
*
* Program :
* Abstract :
* $Id: des.h,v 1.1 2005-11-01 03:22:38 yjlou Exp $
* $Log: not supported by cvs2svn $
* Revision 1.3  2005/09/08 14:07:00  yjlou
* *: fixed the porting bugS of software DES in re865x/crypto: We always use Linux kernel's DES library.
*
* Revision 1.2  2004/06/23 10:15:45  yjlou
* *: convert DOS format to UNIX format
*
* Revision 1.1  2004/06/23 09:18:57  yjlou
* +: support 865xB CRYPTO Engine
*   +: CONFIG_RTL865XB_EXP_CRYPTOENGINE
*   +: basic encry/decry functions (DES/3DES/SHA1/MAC)
*   +: old-fashion API (should be removed in next version)
*   +: batch functions (should be removed in next version)
*
* Revision 1.3  2003/09/01 03:47:40  jzchen
* Change des_cblock to int8
*
* Revision 1.2  2003/08/21 07:15:50  jzchen
* Add ecb 2 mode definition, other proto-type will cause warning so...
*
* Revision 1.1  2002/11/11 11:12:48  hiwu
* initial version
*
* --------------------------------------------------------------------
*/
/*	$FreeBSD: src/sys/crypto/des/des.h,v 1.1.2.2 2001/07/03 11:01:31 ume Exp $	*/
/*	$KAME: des.h,v 1.7 2000/09/18 20:59:21 itojun Exp $	*/

/* lib/des/des.h */
/* Copyright (C) 1995-1996 Eric Young (eay@mincom.oz.au)
 * All rights reserved.
 *
 * This file is part of an SSL implementation written
 * by Eric Young (eay@mincom.oz.au).
 * The implementation was written so as to conform with Netscapes SSL
 * specification.  This library and applications are
 * FREE FOR COMMERCIAL AND NON-COMMERCIAL USE
 * as long as the following conditions are aheared to.
 *
 * Copyright remains Eric Young's, and as such any Copyright notices in
 * the code are not to be removed.  If this code is used in a product,
 * Eric Young should be given attribution as the author of the parts used.
 * This can be in the form of a textual message at program startup or
 * in documentation (online or textual) provided with the package.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *    This product includes software developed by Eric Young (eay@mincom.oz.au)
 *
 * THIS SOFTWARE IS PROVIDED BY ERIC YOUNG ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * The licence and distribution terms for any publically available version or
 * derivative of this code cannot be changed.  i.e. this code cannot simply be
 * copied and put under another distribution licence
 * [including the GNU Public Licence.]
 */

#ifndef HEADER_DES_H
#define HEADER_DES_H

#ifdef  __cplusplus
extern "C" {
#endif

/* must be 32bit quantity */
#define DES_LONG uint32

typedef uint8 des_cblock[8];
typedef struct des_ks_struct
	{
	union	{
		des_cblock _;
		/* make sure things are correct size on machines with
		 * 8 byte longs */
		DES_LONG pad[2];
		} ks;
#undef _
#define _	ks._
	} des_key_schedule[16];

#define DES_KEY_SZ 	(sizeof(des_cblock))
#define DES_SCHEDULE_SZ (sizeof(des_key_schedule))

#define DES_ENCRYPT	1
#define DES_DECRYPT	0

#define DES_CBC_MODE	0
#define DES_PCBC_MODE	1

extern int32 des_check_key;	/* defaults to false */

char *des_options (void);
void des_ecb_encrypt (des_cblock *, des_cblock *,
	des_key_schedule, int32);
void des_encrypt (DES_LONG *, des_key_schedule, int32);
void des_encrypt2 (DES_LONG *, des_key_schedule, int32);

void des_set_odd_parity (uint8 *);
int32 des_is_weak_key (int8 *);
int32 des_set_key (int8 *, des_key_schedule);
int32 des_key_sched (int8 *, des_key_schedule);
void des_ecb3_encrypt( des_cblock *input, des_cblock *output,
		      des_key_schedule *ks1, des_key_schedule *ks2,
		      des_key_schedule *ks3,
	     int enc);
void des_ncbc_encrypt(const unsigned char *in, unsigned char *out, long length,
		     des_key_schedule *_schedule, des_cblock *ivec, int enc);
void des_ede3_cbc_encrypt(const unsigned char *input, unsigned char *output,
			  long length, des_key_schedule *ks1,
			  des_key_schedule *ks2, des_key_schedule *ks3,
			  des_cblock *ivec, int enc);
void des_ede3_cbcm_encrypt(const unsigned char *in, unsigned char *out,
             long length, des_key_schedule ks1, des_key_schedule ks2,
             des_key_schedule ks3, des_cblock *ivec1, des_cblock *ivec2,
             int enc);
void des_xcbc_encrypt( des_cblock *_in,des_cblock *_out,
                      long length, des_key_schedule schedule,
                      des_cblock *ivec, des_cblock *inw,
                      des_cblock *outw, int enc);

#define des_ecb2_encrypt(i,o,k1,k2,e) \
	des_ecb3_encrypt((i),(o),(k1),(k2),(k1),(e))

#ifdef  __cplusplus
}
#endif

#endif
