/*
* --------------------------------------------------------------------
* Copyright c                  Realtek Semiconductor Corporation, 2003
* All rights reserved.
*
* Program : Authentication simulator API definition
* Abstract :
* $Id: authSim.h,v 1.1 2005-11-01 03:22:38 yjlou Exp $
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
* Revision 1.4  2003/09/29 09:03:59  jzchen
* Add authentication simulator batch command
*
* Revision 1.3  2003/09/23 02:23:03  jzchen
* Add generic simulator API
*
* Revision 1.2  2003/09/08 09:20:19  jzchen
* Add MD5 and SHA1 hashing simulation function
*
* Revision 1.1  2003/09/08 04:40:22  jzchen
* Add HMAC authentication simulator
*
*
* --------------------------------------------------------------------
*/

#ifndef AUTHSIM_H
#define AUTHSIM_H

int32 authSim_md5(uint8 * data, uint32 dataLen, uint8 * digest);
int32 authSim_sha1(uint8 * data, uint32 dataLen, uint8 * digest);
int32 authSim_hmacMd5(uint8 * data, uint32 dataLen, uint8 * key, uint32 keyLen, uint8 * digest);
int32 authSim_hmacSha1(uint8 * data, uint32 dataLen, uint8 * key, uint32 keyLen, uint8 * digest);

#define SWHASH_MD5			0x00
#define SWHASH_SHA1		0x01
#define SWHMAC_MD5		0x02
#define SWHMAC_SHA1		0x03
int32 authSim(uint32 mode, uint8 * data, uint32 dataLen, uint8 * key, uint32 keyLen, uint8 * digest);
int32 authSimBatch(uint32 mode, uint8 ** data, uint32 * dataLen, uint8 ** key, uint32 * keyLen, uint8 ** digest, uint32 dataNum);

#endif
