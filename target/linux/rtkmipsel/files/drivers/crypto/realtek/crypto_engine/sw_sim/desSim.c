/*
* --------------------------------------------------------------------
* Copyright c                  Realtek Semiconductor Corporation, 2003
* All rights reserved.
*
* Program : Simulate DES behavior and providing final DES driver interface
* Abstract :
* $Id: desSim.c,v 1.1 2005/11/01 03:22:38 yjlou Exp $
* $Log: desSim.c,v $
* Revision 1.1  2005/11/01 03:22:38  yjlou
* *** empty log message ***
*
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
* Revision 1.5  2003/09/25 10:00:13  jzchen
* Batch simulator
*
* Revision 1.4  2003/09/23 02:21:21  jzchen
* Provide generic API simulator
*
* Revision 1.3  2003/08/28 14:30:25  jzchen
* Change embedded IV function definition to meet ASIC behavior
*
* Revision 1.2  2003/08/28 02:51:11  jzchen
* Change DES_ENCRYPT and DES_DECRYPT define to TRUE and FALSE
*
* Revision 1.1  2003/08/22 11:52:03  jzchen
* Driver api simulation, include DES/3DES CBC/ECB and pass testing code
*
*
* --------------------------------------------------------------------
*/

//#include "rtl_types.h"
#ifdef CONFIG_RTL_ICTEST
#include <rtl_types.h>
#else
#include <net/rtl/rtl_types.h>
#endif
#include "desSim.h"
#include "des.h"

int32 desSim_ecb_encrypt(int8 *input, int8 * output, uint32 len, int8 * key, int8 encrypt) {
	uint32 idx;
	des_cblock in,out;
	des_key_schedule ks;

	if(encrypt != TRUE && encrypt != FALSE) //Only support either encrypt or decrypt operation
		return FAILED;

	for(idx = 0; idx<len; idx+=8) {
		des_set_key(key, ks);
		memcpy(in, &input[idx], 8);
		memset(out, 0,8);
		des_ecb_encrypt(&in,&out,ks,encrypt==TRUE? DES_ENCRYPT : DES_DECRYPT);
		memcpy(&output[idx], out, 8);
	}
	return SUCCESS;
}

int32 desSim_ede_ecb_encrypt(int8 *input, int8 * output, uint32 len, int8 * key, int8 encrypt) {
	uint32 idx;
	des_cblock in,out;
	des_key_schedule ks, ks2, ks3;

	if(encrypt != TRUE && encrypt != FALSE) //Only support either encrypt or decrypt operation
		return FAILED;

	for(idx = 0; idx<len; idx+=8) {
		if (des_set_key(&key[0], ks) != 0 || des_set_key(&key[8], ks2) != 0 || des_set_key(&key[16], ks3) != 0)
			return FAILED;

		memcpy(in, &input[idx], 8);
		memset(out, 0, 8);
		des_ecb3_encrypt((des_cblock *)&in,&out,&ks,&ks2, &ks3, encrypt==TRUE? DES_ENCRYPT : DES_DECRYPT);
		memcpy(&output[idx], out, 8);
	}
	return SUCCESS;
}

int32 desSim_cbc_encrypt(int8 *input, int8 * output, uint32 len, int8 * key, int8 * iv, int8 encrypt) {
	des_cblock iv3;
	des_key_schedule ks;

	if(encrypt != TRUE && encrypt != FALSE) //Only support either encrypt or decrypt operation
		return FAILED;

	if (des_set_key(&key[0], ks) != 0)
		return FAILED;

	memcpy(iv3, iv, 8);
	des_ncbc_encrypt(input, output, len, &ks, &iv3, encrypt==TRUE? DES_ENCRYPT : DES_DECRYPT);

	return SUCCESS;
}

int32 desSim_ede_cbc_encrypt(int8 *input, int8 * output, uint32 len, int8 * key, int8 * iv, int8 encrypt) {
	des_cblock iv3;
	des_key_schedule ks, ks2, ks3;

	if(encrypt != TRUE && encrypt != FALSE) //Only support either encrypt or decrypt operation
		return FAILED;

	if (des_set_key(&key[0], ks) != 0 || des_set_key(&key[8], ks2) != 0 || des_set_key(&key[16], ks3) != 0)
		return FAILED;

	memcpy(iv3, iv, 8);
	des_ede3_cbc_encrypt(input, output, len, &ks, &ks2, &ks3, &iv3, encrypt==TRUE? DES_ENCRYPT : DES_DECRYPT);

	return SUCCESS;
}

//IV is the first 8-byte of input and the real data length is len-8
int32 desSim_cbc_encryptEmbIV(int8 *input, int8 * output, uint32 len, int8 * key, int8 encrypt) {
	des_cblock iv3;
	des_key_schedule ks;

	if(encrypt != TRUE && encrypt != FALSE) //Only support either encrypt or decrypt operation
		return FAILED;

	if (des_set_key(&key[0], ks) != 0)
		return FAILED;

	memcpy(iv3, &input[0], 8);
	memcpy(&output[0], &input[0], 8);
	des_ncbc_encrypt(input+8, output+8, len-8, &ks, &iv3, encrypt==TRUE? DES_ENCRYPT : DES_DECRYPT);

	return SUCCESS;
}

//IV is the first 8-byte of input and the real data length is len-8
int32 desSim_ede_cbc_encryptEmbIV(int8 *input, int8 * output, uint32 len, int8 * key, int8 encrypt) {
	des_cblock iv3;
	des_key_schedule ks, ks2, ks3;

	if(encrypt != TRUE && encrypt != FALSE) //Only support either encrypt or decrypt operation
		return FAILED;

	if (des_set_key(&key[0], ks) != 0 || des_set_key(&key[8], ks2) != 0 || des_set_key(&key[16], ks3) != 0)
		return FAILED;

	memcpy(iv3, &input[0], 8);
	memcpy(&output[0], &input[0], 8);
	des_ede3_cbc_encrypt(input+8, output+8, len-8, &ks, &ks2, &ks3, &iv3, encrypt==TRUE? DES_ENCRYPT : DES_DECRYPT);

	return SUCCESS;
}


//data, key and iv does not have 4-byte alignment limitatiuon
int32 desSim_des(uint32 mode, int8 *input, int8 *output, uint32 len, int8 *key, int8 *iv ) {
	switch(mode) {
		case  SWDECRYPT_CBC_DES:
			return desSim_cbc_encrypt(input, output, len, key, iv, FALSE);
		case  SWDECRYPT_CBC_3DES:
			return desSim_ede_cbc_encrypt(input, output, len, key, iv, FALSE);
		case  SWDECRYPT_ECB_DES:
			return desSim_ecb_encrypt(input, output, len, key, FALSE);
		case  SWDECRYPT_ECB_3DES:
			return desSim_ede_ecb_encrypt(input, output, len, key, FALSE);
		case  SWENCRYPT_CBC_DES:
			return desSim_cbc_encrypt(input, output, len, key, iv, TRUE);
		case  SWENCRYPT_CBC_3DES:
			return desSim_ede_cbc_encrypt(input, output, len, key, iv, TRUE);
		case  SWENCRYPT_ECB_DES:
			return desSim_ecb_encrypt(input, output, len, key, TRUE);
		case  SWENCRYPT_ECB_3DES:
			return desSim_ede_ecb_encrypt(input, output, len, key, TRUE);
	}
	return FAILED;
}

int32 desSim_desBatch(uint32 mode, int8 ** input, int8 **output, uint32 *len, int8 **key, int8 **iv, uint32 dataNum){
	uint32 i;
	for(i=0; i<dataNum; i++) {
		if(desSim_des(mode, input[i], output[i], len[i], key[i], iv[i]) == FAILED)
			return FAILED;
	}
	return SUCCESS;
}
