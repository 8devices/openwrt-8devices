/*
* --------------------------------------------------------------------
* Copyright c                  Realtek Semiconductor Corporation, 2003
* All rights reserved.
*
* Program : Authentication simulator code
* Abstract :
* $Id: authSim.c,v 1.1 2005-11-01 03:22:38 yjlou Exp $
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
* Revision 1.5  2003/09/29 09:05:36  jzchen
* Add authentication simulator batch api
*
* Revision 1.4  2003/09/23 02:22:30  jzchen
* Provide generic API
*
* Revision 1.3  2003/09/09 02:52:32  jzchen
* Reduce the memory operaiton cycles
*
* Revision 1.2  2003/09/08 09:22:05  jzchen
* Fix length calculation error
*
* Revision 1.1  2003/09/08 04:40:01  jzchen
* Add HMAC authentication simulator
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
//#include "rtl_glue.h"
#include <net/rtl/rtl_glue.h>
#include "md5.h"
#include "sha1.h"
#include "hmac.h"
#include "authSim.h"

static uint8 authSimInputPad[HMAC_MAX_MD_CBLOCK], authSimOutputPad[HMAC_MAX_MD_CBLOCK];
static uint8 authSimMd5TempKey[MD5_DIGEST_LENGTH], authSimSha1TempKey[SHA_DIGEST_LENGTH];

static const uint8 authSim_md5Paddat[MD5_BUFLEN] = {
	0x80,	0,	0,	0,	0,	0,	0,	0,
	0,		0,	0,	0,	0,	0,	0,	0,
	0,		0,	0,	0,	0,	0,	0,	0,
	0,		0,	0,	0,	0,	0,	0,	0,
	0,		0,	0,	0,	0,	0,	0,	0,
	0,		0,	0,	0,	0,	0,	0,	0,
	0,		0,	0,	0,	0,	0,	0,	0,
	0,		0,	0,	0,	0,	0,	0,	0,
};

static uint8 authSim_tempMd5Digest[MD5_BUFLEN] = {
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x80, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

static uint8 authSim_tempSha1Digest[MD5_BUFLEN] = {
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0xA0,
};

//Data length must be long enough to fill padding ((64+8)-byte at most)
int32 authSim_md5(uint8 * data, uint32 dataLen, uint8 * digest) {
	MD5_CTX context;
	uint32 i, dmaLen;
	uint64 len64;
	uint8 *uint8Ptr = (uint8 *)&len64;

	i = MD5_BUFLEN - (dataLen&0x3F);
	if(i>8) {
		memcpy((void *)(data + dataLen), (void *)authSim_md5Paddat, i-8);
		dmaLen = (dataLen+64)&0xFFFFFFC0;
	}
	else {
		memcpy((void *)(data + dataLen), (void *)authSim_md5Paddat, i+56);
		dmaLen = (dataLen+72)&0xFFFFFFC0;
	}
	len64 = dataLen<<3;//First padding length is
	for(i=0; i<8; i++)
		data[dmaLen-i-1] = uint8Ptr[i];

	md5_init(&context);
	md5_loop(&context, data, dmaLen);
	md5_result(digest, &context);

	return SUCCESS;
}

//Data length must be long enough to fill padding ((64+8)-byte at most)
int32 authSim_sha1(uint8 * data, uint32 dataLen, uint8 * digest) {
	SHA1_CTX context;
	uint32 i, dmaLen;
	uint64 len64;
	uint8 *uint8Ptr = (uint8 *)&len64;

	i = MD5_BUFLEN - (dataLen&0x3F);
	if(i>8) {
		memcpy((void *)(data + dataLen), (void *)authSim_md5Paddat, i-8);
		dmaLen = (dataLen+64)&0xFFFFFFC0;
	}
	else {
		memcpy((void *)(data + dataLen), (void *)authSim_md5Paddat, i+56);
		dmaLen = (dataLen+72)&0xFFFFFFC0;
	}
	len64 = dataLen<<3;//First padding length is
	for(i=0; i<8; i++)
		data[dmaLen-8+i] = uint8Ptr[i];

	sha1_init(&context);
	sha1_loop(&context, data, dmaLen);
	memcpy(digest, &context.h.b8[0], 20);

	return SUCCESS;
}


//Data length must be long enough to fill padding ((64+8)-byte at most)
int32 authSim_hmacMd5(uint8 * data, uint32 dataLen, uint8 * key, uint32 keyLen, uint8 * digest) {
	MD5_CTX context;
	uint32 i, dmaLen;
	uint64 len64;
	uint8 *uint8Ptr = (uint8 *)&len64;

	if(keyLen > HMAC_MAX_MD_CBLOCK) {
		MD5Init(&context);
		MD5Update(&context, key, keyLen);
		MD5Final(authSimMd5TempKey, &context);
		key = authSimMd5TempKey;
		keyLen = MD5_DIGEST_LENGTH;
	}

	memset(authSimInputPad, 0x36, HMAC_MAX_MD_CBLOCK);
	memset(authSimOutputPad, 0x5c, HMAC_MAX_MD_CBLOCK);
	for(i=0; i<keyLen; i++) {
		authSimInputPad[i] ^= key[i];
		authSimOutputPad[i] ^= key[i];
	}

	i = MD5_BUFLEN - (dataLen&0x3F);
	if(i>8) {
		memcpy((void *)(data + dataLen), (void *)authSim_md5Paddat, i-8);
		dmaLen = (dataLen+64)&0xFFFFFFC0;
	}
	else {
		memcpy((void *)(data + dataLen), (void *)authSim_md5Paddat, i+56);
		dmaLen = (dataLen+72)&0xFFFFFFC0;
	}
	len64 = (dataLen+64)<<3;//First padding length is
	for(i=0; i<8; i++)
		data[dmaLen-i-1] = uint8Ptr[i];

	md5_init(&context);
	md5_loop(&context, authSimInputPad, 64);
	md5_loop(&context, data, dmaLen);
	md5_result(authSim_tempMd5Digest, &context);
	md5_init(&context);
	md5_loop(&context, authSimOutputPad, 64);
	md5_loop(&context, authSim_tempMd5Digest, 64);
	md5_result(digest, &context);

	return SUCCESS;
}

int32 authSim_hmacSha1(uint8 * data, uint32 dataLen, uint8 * key, uint32 keyLen, uint8 * digest){
	SHA1_CTX context;
	uint32 i, dmaLen;
	uint64 len64;
	uint8 *uint8Ptr = (uint8 *)&len64;

	if(keyLen > HMAC_MAX_MD_CBLOCK) {
		SHA1Init(&context);
		SHA1Update(&context, key, keyLen);
		SHA1Final(authSimSha1TempKey, &context);
		key = authSimSha1TempKey;
		keyLen = SHA_DIGEST_LENGTH;
	}

	memset(authSimInputPad, 0x36, HMAC_MAX_MD_CBLOCK);
	memset(authSimOutputPad, 0x5c, HMAC_MAX_MD_CBLOCK);
	for(i=0; i<keyLen; i++) {
		authSimInputPad[i] ^= key[i];
		authSimOutputPad[i] ^= key[i];
	}

	i = MD5_BUFLEN - (dataLen&0x3F);
	if(i>8) {
		memcpy((void *)(data + dataLen), (void *)authSim_md5Paddat, i-8);
		dmaLen = (dataLen+64)&0xFFFFFFC0;
	}
	else {
		memcpy((void *)(data + dataLen), (void *)authSim_md5Paddat, i+56);
		dmaLen = (dataLen+72)&0xFFFFFFC0;
	}
	len64 = (dataLen+64)<<3;//First padding length is
	for(i=0; i<8; i++)
		data[dmaLen-8+i] = uint8Ptr[i];

	sha1_init(&context);
	sha1_loop(&context, authSimInputPad, 64);
	sha1_loop(&context, data, dmaLen);
	memcpy(authSim_tempSha1Digest, &context.h.b8[0], 20);
	sha1_init(&context);
	sha1_loop(&context, authSimOutputPad, 64);
	sha1_loop(&context, authSim_tempSha1Digest, 64);
	memcpy(digest, &context.h.b8[0], 20);

	return SUCCESS;
}

int32 authSim(uint32 mode, uint8 * data, uint32 dataLen, uint8 * key, uint32 keyLen, uint8 * digest) {
	MD5_CTX md5Context;
	SHA1_CTX sha1Context;
	uint32 i, dmaLen;
	uint64 len64;
	uint8 *uint8Ptr = (uint8 *)&len64;
	uint8 tempDigest[SHA_DIGEST_LENGTH];

	assert(data && digest);//Either Hashing or key must exist
	//Calculating ipad and opad
	if(mode&0x2) {//HMAC
		assert(key);
		if(keyLen > HMAC_MAX_MD_CBLOCK) {
			if(mode&0x1) {
				SHA1Init(&sha1Context);
				SHA1Update(&sha1Context, key, keyLen);
				SHA1Final(tempDigest, &sha1Context);
				key = tempDigest;
				keyLen = SHA_DIGEST_LENGTH;
			}
			else {
				MD5Init(&md5Context);
				MD5Update(&md5Context, key, keyLen);
				MD5Final(tempDigest, &md5Context);
				key = tempDigest;
				keyLen = MD5_DIGEST_LENGTH;
			}
		}
		memset(authSimInputPad, 0x36, HMAC_MAX_MD_CBLOCK);
		memset(authSimOutputPad, 0x5c, HMAC_MAX_MD_CBLOCK);
		for(i=0; i<keyLen; i++) {
			authSimInputPad[i] ^= key[i];
			authSimOutputPad[i] ^= key[i];
		}
	}
	//Do padding for MD5 and SHA-1
	i = HMAC_MAX_MD_CBLOCK - (dataLen&0x3F);
	if(i>8) {
		memcpy((void *)(data + dataLen), (void *)authSim_md5Paddat, i-8);
		dmaLen = (dataLen+64)&0xFFFFFFC0;
	}
	else {
		memcpy((void *)(data + dataLen), (void *)authSim_md5Paddat, i+56);
		dmaLen = (dataLen+72)&0xFFFFFFC0;
	}
	if(mode&0x2) //HMAC
		len64 = (dataLen+64)<<3;//First padding length is
	else
		len64 = dataLen<<3;//First padding length is
	for(i=0; i<8; i++) {
		if(mode&0x1) //SHA-1
			data[dmaLen-8+i] = uint8Ptr[i];
		else
			data[dmaLen-i-1] = uint8Ptr[i];
	}
	switch(mode) {
		case SWHASH_MD5:
			md5_init(&md5Context);
			md5_loop(&md5Context, data, dmaLen);
			md5_result(tempDigest, &md5Context);
		break;
		case SWHASH_SHA1:
			sha1_init(&sha1Context);
			sha1_loop(&sha1Context, data, dmaLen);
			memcpy(tempDigest, &sha1Context.h.b8[0], 20);
		break;
		case SWHMAC_MD5:
			md5_init(&md5Context);
			md5_loop(&md5Context, authSimInputPad, 64);
			md5_loop(&md5Context, data, dmaLen);
			md5_result(authSim_tempMd5Digest, &md5Context);
			md5_init(&md5Context);
			md5_loop(&md5Context, authSimOutputPad, 64);
			md5_loop(&md5Context, authSim_tempMd5Digest, 64);
			md5_result(tempDigest, &md5Context);
		break;
		case SWHMAC_SHA1:
			sha1_init(&sha1Context);
			sha1_loop(&sha1Context, authSimInputPad, 64);
			sha1_loop(&sha1Context, data, dmaLen);
			memcpy(authSim_tempSha1Digest, &sha1Context.h.b8[0], 20);
			sha1_init(&sha1Context);
			sha1_loop(&sha1Context, authSimOutputPad, 64);
			sha1_loop(&sha1Context, authSim_tempSha1Digest, 64);
			memcpy(tempDigest, &sha1Context.h.b8[0], 20);
		break;
	}

	if(mode&0x1)
	{
		// strange, using memcpy() here will cause the tailing 4-bytes lost!
		//memcpy(digest, tempDigest, SHA_DIGEST_LENGTH);//Avoid 4-byte alignment limitation
		for(i=0;i<SHA_DIGEST_LENGTH;i++) digest[i]=tempDigest[i];
	}
	else
	{
		// strange, using memcpy() here will cause the tailing 4-bytes lost!
		//memcpy(digest, tempDigest, MD5_DIGEST_LENGTH);//Avoid 4-byte alignment limitation
		for(i=0;i<MD5_DIGEST_LENGTH;i++) digest[i]=tempDigest[i];
	}

	return SUCCESS;
}

int32 authSimBatch(uint32 mode, uint8 ** data, uint32 * dataLen, uint8 ** key, uint32 * keyLen, uint8 ** digest, uint32 dataNum) {
	uint32 i;

	for(i=0; i<dataNum; i++) {
		if(authSim(mode, data[i], dataLen[i], key[i], keyLen[i], digest[i]) == FAILED)
			return FAILED;
	}
	return SUCCESS;
}
