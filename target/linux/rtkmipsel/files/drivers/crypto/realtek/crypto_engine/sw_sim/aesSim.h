/*
* --------------------------------------------------------------------
* Copyright c                  Realtek Semiconductor Corporation, 2003
* All rights reserved.
*
* Program : Simulate DES behavior and providing final DES driver interface
* Abstract :
* $Id: aesSim.h,v 1.2 2006-07-13 16:03:06 chenyl Exp $
*/

#ifndef AESSIM_H
#define AESSIM_H

#define SWDECRYPT_CBC_AES 0x20
#define SWDECRYPT_ECB_AES 0x22
#define SWDECRYPT_CTR_AES 0x23
#define SWENCRYPT_CBC_AES 0x24
#define SWENCRYPT_ECB_AES 0x26
#define SWENCRYPT_CTR_AES 0x27

int32 aesSim_aes(uint32 mode, int8 *input, int8 *output, uint32 len, uint32 lenKey, int8 *key, int8 *iv );

#endif
