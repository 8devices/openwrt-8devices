#include <linux/kernel.h>
#include <linux/errno.h>
#include <crypto/aes.h>

#include <net/rtl/rtl_types.h>
#include <net/rtl/rtl_glue.h>
//#include <asm/rtl865x/rtl865xc_asicregs.h>
#include "rtl_ipsec.h"
#include "rtl_crypto_helper.h"
#include "../../net/rtl819x/AsicDriver/rtl865xc_asicregs.h"
#include "md5.h"
#include "sha1.h"

//#define CONFIG_RTK_VOIP_DBG
#define CACHED_ADDRESS(x) CKSEG0ADDR(x)
extern void (*_dma_cache_wback_inv)(unsigned long start, unsigned long size);
static uint8 *cryptoKey, *cryptoIv, *authKey, *pad, *aesDeKey;
static uint8 *asic_orig, *asic_enc, *asic_digest;

#if defined(CRYPTOTEST_USE_UNCACHED_MALLOC)
int32 rtl_crypto_helper_alloc(void)
{
	cryptoKey =(uint8 *) UNCACHED_MALLOC(CRYPTOTEST_KEY_LEN);
	aesDeKey = (uint8 *) UNCACHED_MALLOC(CRYPTOTEST_KEY_LEN);

	cryptoIv =(uint8 *) UNCACHED_MALLOC(CRYPTOTEST_IV_LEN);

	asic_orig = (uint8 *) UNCACHED_MALLOC(CRYPTOTEST_ASICORG_LEN);
	asic_enc = (uint8 *) UNCACHED_MALLOC(CRYPTOTEST_ASICENC_LEN);
	asic_digest = (uint8 *) UNCACHED_MALLOC(CRYPTOTEST_ASICDIGEST_LEN);

	if (!cryptoKey || !aesDeKey || !cryptoIv || !asic_orig || !asic_enc || !asic_digest)
	{
		printk("%s %d malloc memory failed \n", __FUNCTION__,__LINE__);
		return FAILED;
	}
	return SUCCESS;
}

int32 rtl_crypto_helper_free( void )
{
	if (cryptoKey)
	{
		kfree((void *) CACHED_ADDRESS(cryptoKey));
		cryptoKey = NULL;
	}

	if (aesDeKey)
	{
		kfree((void *) CACHED_ADDRESS(aesDeKey));
		aesDeKey = NULL;
	}

	if (cryptoIv)
	{
		kfree((void *) CACHED_ADDRESS(cryptoIv));
		cryptoIv = NULL;
	}

	if (asic_orig)
	{
		kfree((void *) CACHED_ADDRESS(asic_orig));
		asic_orig = NULL;
	}
	if (asic_enc)
	{

		kfree((void *) CACHED_ADDRESS(asic_enc));
		asic_enc = NULL;
	}
	if (asic_digest)
	{
		kfree((void *) CACHED_ADDRESS(asic_digest));
		asic_digest = NULL;
	}

	return SUCCESS;
}

int32 rtl_crypto_helper_Init( void )
{
	if (cryptoKey)
	{
		memset(cryptoKey, 0, CRYPTOTEST_KEY_LEN);
	}

	if (aesDeKey)
	{
		memset(aesDeKey, 0, CRYPTOTEST_KEY_LEN);
	}

	if (cryptoIv)
	{
		memset(cryptoIv, 0, CRYPTOTEST_IV_LEN);
	}
	if (asic_orig)
	{
		memset(asic_orig, 0, CRYPTOTEST_ASICORG_LEN);
	}
	if (asic_enc)
	{

		memset(asic_enc, 0, CRYPTOTEST_ASICENC_LEN);
	}
	if (asic_digest)
	{
		memset(asic_digest, 0, CRYPTOTEST_ASICDIGEST_LEN);
	}

	return SUCCESS;
}
#endif

void rtl_crypto_hexdump(unsigned char *buf, unsigned int len)
{
	print_hex_dump(KERN_CONT, "", DUMP_PREFIX_OFFSET,
			16, 1,
			buf, len, false);
}

int rtl_hash_init_ctx(struct crypto_tfm *tfm, struct rtl_hash_ctx *ctx)
{
	const char *algname = crypto_tfm_alg_name(tfm);

	if (strcmp(algname, "md5") == 0)
		ctx->mode = 0x00;
	else if (strcmp(algname, "sha1") == 0)
		ctx->mode = 0x01;
	else
		ctx->mode = -1;
	#if defined(CRYPTOTEST_USE_UNCACHED_MALLOC)
	ctx->data = (u8 *) asic_orig; ///????
	ctx->digest= (u8 *) asic_digest;
	#else
	ctx->data = (u8 *) kmalloc(MAX_PKTLEN, GFP_KERNEL);
	#endif
	ctx->length = 0;
	return 0;
}

int rtl_hash_update(struct rtl_hash_ctx *ctx, u8 *data, u32 length)
{
	memcpy(ctx->data + ctx->length, data, length);
	ctx->length += length;
	return 0;
}

int rtl_hash_final(struct rtl_hash_ctx *ctx, u8 *out)
{
	int ret;

	ret = rtl_hash_digest(ctx, ctx->data, ctx->length, out);
	#if defined(CRYPTOTEST_USE_UNCACHED_MALLOC)
	#else
	kfree(ctx->data);
	#endif
	return ret;
}

int rtl_hash_digest(struct rtl_hash_ctx *ctx, u8 *data, u32 length, u8 *out)
{
	rtl_ipsecScatter_t scatter[1];
	#if defined(CRYPTOTEST_USE_UNCACHED_MALLOC)
	int ret = 0;
	#endif
	scatter[0].len = length;
	#if defined(CRYPTOTEST_USE_UNCACHED_MALLOC)
	scatter[0].ptr = (void *)data;
	#else
	scatter[0].ptr = (void *) CKSEG1ADDR(data);
	#endif

	/*
		int32 rtl_ipsecEngine(uint32 modeCrypto, uint32 modeAuth,
			uint32 cntScatter, rtl_ipsecScatter_t *scatter, void *pCryptResult,
			uint32 lenCryptoKey, void* pCryptoKey,
			uint32 lenAuthKey, void* pAuthKey,
			void* pIv, void* pPad, void* pDigest,
			uint32 a2eo, uint32 enl)
	*/
	#if defined(CRYPTOTEST_USE_UNCACHED_MALLOC)
	ret = rtl_ipsecEngine(-1, ctx->mode, 1, scatter, NULL,
		0, NULL,
		0, NULL,
		//NULL, NULL, ctx->digest,
		NULL, NULL, out,
		0, length);

	if (unlikely(ret))
		printk("%s %d: rtl_ipsecEngine failed, ret=%d \n", __FUNCTION__, __LINE__, ret);

	if (ctx->mode == 0x00)
	{
		_dma_cache_wback_inv((u32) out, MD5_DIGEST_LENGTH);
	}
	else
	{
		_dma_cache_wback_inv((u32) out, SHA_DIGEST_LENGTH);
	}

	return ret;

	#else
	_dma_cache_wback_inv((u32) data, MAX_PKTLEN);
	ret = rtl_ipsecEngine(-1, ctx->mode, 1, scatter, NULL,
		0, NULL,
		0, NULL,
		NULL, NULL, out,
		0, length);
	if (ctx->mode == 0x00)
	{
		_dma_cache_wback_inv((u32) out, MD5_DIGEST_LENGTH);
	}
	else
	{
		_dma_cache_wback_inv((u32) out, SHA_DIGEST_LENGTH);
	}

	return ret;
	#endif
}

int rtl_cipher_init_ctx(struct crypto_tfm *tfm,
	struct rtl_cipher_ctx *ctx)
{
	const char *algname = crypto_tfm_alg_name(tfm);

	memset(ctx, 0, sizeof(*ctx));

	if (strcmp(algname, "cbc(des)") == 0)
		ctx->mode = 0x00;
	else if (strcmp(algname, "cbc(des3_ede)") == 0)
		ctx->mode = 0x01;
	else if (strcmp(algname, "ecb(des)") == 0)
		ctx->mode = 0x02;
	else if (strcmp(algname, "ecb(des3_ede)") == 0)
		ctx->mode = 0x03;
	else if (strcmp(algname, "cbc(aes)") == 0)
		ctx->mode = 0x20;
	else if (strcmp(algname, "ecb(aes)") == 0)
		ctx->mode = 0x22;
	else if (strcmp(algname, "ctr(aes)") == 0)
		ctx->mode = 0x23;
	else
		ctx->mode = -1;

#ifdef CONFIG_RTK_VOIP_DBG
	printk("%s: alg=%s, driver=%s, mode=%x\n", __FUNCTION__,
		crypto_tfm_alg_name(tfm),
		crypto_tfm_alg_driver_name(tfm),
		ctx->mode
	);
#endif
	#if defined(CRYPTOTEST_USE_UNCACHED_MALLOC)
	ctx->aes_dekey = aesDeKey; // cache align issue
	ctx->key = cryptoKey; // ???
	#else
	ctx->aes_dekey = &ctx->__aes_dekey[32]; // cache align issue
	#endif

	return 0;
}

int	rtl_cipher_setkey(struct crypto_cipher *cipher,
	struct rtl_cipher_ctx *ctx, const u8 *key, unsigned int keylen)
{
	#if defined(CRYPTOTEST_USE_UNCACHED_MALLOC)
	memcpy(ctx->key, key, keylen);//??uncached
	#else
	ctx->key = (u8 *) key;
	#endif
	ctx->key_length = keylen;

	// setup aes-dekey
	if ((ctx->mode >= 0) && (ctx->mode & 0x20))
	{
		struct crypto_aes_ctx *aes_ctx;
		u8 *aes_dekey = ctx->aes_dekey;
		u32 *round_key;

		aes_ctx = crypto_tfm_ctx(crypto_cipher_tfm(cipher));
		round_key = aes_ctx->key_enc;

		// IC accept the de-key in reverse order
		switch (aes_ctx->key_length)
		{
			case 128/8:
				((u32*) aes_dekey)[0] = cpu_to_le32(round_key[4 * 10 + 0]);
				((u32*) aes_dekey)[1] = cpu_to_le32(round_key[4 * 10 + 1]);
				((u32*) aes_dekey)[2] = cpu_to_le32(round_key[4 * 10 + 2]);
				((u32*) aes_dekey)[3] = cpu_to_le32(round_key[4 * 10 + 3]);
				break;
			case 192/8:
				((u32*) aes_dekey)[0] = cpu_to_le32(round_key[4 * 12 + 0]);
				((u32*) aes_dekey)[1] = cpu_to_le32(round_key[4 * 12 + 1]);
				((u32*) aes_dekey)[2] = cpu_to_le32(round_key[4 * 12 + 2]);
				((u32*) aes_dekey)[3] = cpu_to_le32(round_key[4 * 12 + 3]);
				((u32*) aes_dekey)[4] = cpu_to_le32(round_key[4 * 11 + 2]);
				((u32*) aes_dekey)[5] = cpu_to_le32(round_key[4 * 11 + 3]);
				break;
			case 256/8:
				((u32*) aes_dekey)[0] = cpu_to_le32(round_key[4 * 14 + 0]);
				((u32*) aes_dekey)[1] = cpu_to_le32(round_key[4 * 14 + 1]);
				((u32*) aes_dekey)[2] = cpu_to_le32(round_key[4 * 14 + 2]);
				((u32*) aes_dekey)[3] = cpu_to_le32(round_key[4 * 14 + 3]);
				((u32*) aes_dekey)[4] = cpu_to_le32(round_key[4 * 13 + 0]);
				((u32*) aes_dekey)[5] = cpu_to_le32(round_key[4 * 13 + 1]);
				((u32*) aes_dekey)[6] = cpu_to_le32(round_key[4 * 13 + 2]);
				((u32*) aes_dekey)[7] = cpu_to_le32(round_key[4 * 13 + 3]);
				break;
			default:
				printk("%s: unknown aes key length=%d\n",
					__FUNCTION__, aes_ctx->key_length);
				return -EINVAL;
		}
	}

	return 0;
}

int rtl_cipher_crypt(struct crypto_cipher *cipher, u8 bEncrypt,
	struct rtl_cipher_ctx *ctx, u8 *src, unsigned int nbytes, u8 *iv, u8 *dst)
{
	unsigned int bsize = crypto_cipher_blocksize(cipher);
	u8 *key = bEncrypt ? ctx->key : (ctx->mode & 0x20) ? ctx->aes_dekey : ctx->key;
	rtl_ipsecScatter_t scatter[1];
	u32 flag_encrypt = bEncrypt ? 4 : 0;
	int err;

#ifdef CONFIG_RTK_VOIP_DBG
	printk("%s: src=%p, len=%d, blk=%d, key=%p, iv=%p, dst=%p ctx->mode=0x%x bEncrypt=%d\n", __FUNCTION__,
		src, nbytes, bsize, key, iv, dst, ctx->mode, bEncrypt);
	rtl_crypto_hexdump((void *) src, nbytes);
	rtl_crypto_hexdump((void *) key, ctx->key_length);
	rtl_crypto_hexdump((void *) iv, bsize);
#endif

#if 1//!defined(CRYPTOTEST_USE_UNCACHED_MALLOC)
	dma_cache_wback((u32) src, nbytes);
	dma_cache_wback((u32) key, ctx->key_length);
	dma_cache_wback((u32) iv, bsize);

	scatter[0].len = (nbytes / bsize) * bsize;
	scatter[0].ptr = (void *) CKSEG1ADDR(src);

	/*
		int32 rtl_ipsecEngine(uint32 modeCrypto, uint32 modeAuth,
			uint32 cntScatter, rtl_ipsecScatter_t *scatter, void *pCryptResult,
			uint32 lenCryptoKey, void* pCryptoKey,
			uint32 lenAuthKey, void* pAuthKey,
			void* pIv, void* pPad, void* pDigest,
			uint32 a2eo, uint32 enl)
	*/
	err = rtl_ipsecEngine(ctx->mode | flag_encrypt,
		-1, 1, scatter,
		(void *) CKSEG1ADDR(dst),
		ctx->key_length, (void *) CKSEG1ADDR(key),
		0, NULL,
		(void *) CKSEG1ADDR(iv), NULL, NULL,
		0, scatter[0].len);
#else
	scatter[0].len = (nbytes / bsize) * bsize;
	scatter[0].ptr = (void *) asic_orig;
	memcpy(scatter[0].ptr, src, scatter[0].len);
	memcpy(cryptoIv, iv, bsize);

	err = rtl_ipsecEngine(ctx->mode | flag_encrypt,
		-1, 1, scatter,
		(void *) asic_enc,
		ctx->key_length, (void *) key,
		0, NULL,
		(void *) cryptoIv, NULL, NULL,
		0, scatter[0].len);
#endif
	if (unlikely(err))
		printk("%s: rtl_ipsecEngine failed\n", __FUNCTION__);

	/*Fix jwj, change from dma_cache_inv to dma_cache_wback_inv*/
	//dma_cache_inv((u32) dst, nbytes);
	#if 1//!defined(CRYPTOTEST_USE_UNCACHED_MALLOC)
	dma_cache_wback_inv((u32) dst, nbytes);
	#else
	memcpy(dst ,asic_enc, scatter[0].len);
	dma_cache_wback_inv((u32) dst, nbytes);
	#endif
#ifdef CONFIG_RTK_VOIP_DBG
	printk("result:\n");
	rtl_crypto_hexdump(dst, scatter[0].len);
#endif

	// return handled bytes, even err! (for blkcipher_walk)
	return nbytes - scatter[0].len;
}
