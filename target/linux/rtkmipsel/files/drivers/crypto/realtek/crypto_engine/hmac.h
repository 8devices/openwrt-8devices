/*
* --------------------------------------------------------------------
* Copyright c                  Realtek Semiconductor Corporation, 2002
* All rights reserved.
*
* Program : HMAC header files
* Abstract :
* $Id: hmac.h,v 1.1 2005-11-01 03:22:38 yjlou Exp $
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
* Revision 1.2  2003/09/02 14:58:16  jzchen
* HMAC emulation API
*
* --------------------------------------------------------------------
*/

#ifndef HEADER_HMAC_H
#define HEADER_HMAC_H

#define HMAC_MAX_MD_CBLOCK	64

int32 HMACMD5(uint8 * data, uint32 dataLen, uint8 * key, uint32 keyLen, uint8 * digest);
int32 HMACSHA1(uint8 * data, uint32 dataLen, uint8 * key, uint32 keyLen, uint8 * digest);

#endif
