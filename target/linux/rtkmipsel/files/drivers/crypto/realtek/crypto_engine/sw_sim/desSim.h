/*
* --------------------------------------------------------------------
* Copyright c                  Realtek Semiconductor Corporation, 2003
* All rights reserved.
*
* Program : Simulate DES behavior and providing final DES driver interface
* Abstract :
* $Id: desSim.h,v 1.1 2005-11-01 03:22:38 yjlou Exp $
* $Log: not supported by cvs2svn $
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
* Revision 1.5  2003/09/25 10:01:45  jzchen
* Add batch crypto simulator
*
* Revision 1.4  2003/09/23 02:23:32  jzchen
* Add generic API
*
* Revision 1.3  2003/08/28 02:52:19  jzchen
* Change prototype
*
* Revision 1.2  2003/08/27 02:08:55  jzchen
* Add embed IV type CBC mode API
*
* Revision 1.1  2003/08/22 11:51:46  jzchen
* Driver api simulation, include DES/3DES CBC/ECB and pass testing code
*
*
* --------------------------------------------------------------------
*/
#ifndef DESSIM_H
#define DESSIM_H

int32 desSim_ecb_encrypt(int8 *input, int8 * output, uint32 len, int8 * ks, int8 encrypt);
int32 desSim_ede_ecb_encrypt(int8 *input, int8 * output, uint32 len, int8 * key, int8 encrypt);
int32 desSim_cbc_encrypt(int8 *input, int8 * output, uint32 len, int8 * key, int8 * iv, int8 encrypt);
int32 desSim_ede_cbc_encrypt(int8 *input, int8 * output, uint32 len, int8 * key, int8 * iv, int8 encrypt);
int32 desSim_cbc_encryptEmbIV(int8 *input, int8 * output, uint32 len, int8 * key, int8 encrypt) ;
int32 desSim_ede_cbc_encryptEmbIV(int8 *input, int8 * output, uint32 len, int8 * key, int8 encrypt);

#define SWDECRYPT_CBC_DES		0x00
#define SWDECRYPT_CBC_3DES	0x01
#define SWDECRYPT_ECB_DES		0x02
#define SWDECRYPT_ECB_3DES	0x03
#define SWENCRYPT_CBC_DES		0x04
#define SWENCRYPT_CBC_3DES	0x05
#define SWENCRYPT_ECB_DES		0x06
#define SWENCRYPT_ECB_3DES	0x07
//data, key and iv does not have 4-byte alignment limitatiuon
int32 desSim_des(uint32 mode, int8 *input, int8 *output, uint32 len, int8 *key, int8 *iv );
int32 desSim_desBatch(uint32 mode, int8 ** input, int8 **output, uint32 *len, int8 **key, int8 **iv, uint32 dataNum);

#endif
