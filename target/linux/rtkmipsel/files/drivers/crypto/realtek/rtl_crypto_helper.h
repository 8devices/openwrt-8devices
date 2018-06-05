#ifndef __RTL_CRYPTO_HELPER_H
#define __RTL_CRYPTO_HELPER_H

struct rtl_hash_ctx {
	s32 mode;
	u8 *data;
	u32 length;
	#if defined(CRYPTOTEST_USE_UNCACHED_MALLOC)
	u8 *digest;
	#endif
};

struct rtl_cipher_ctx {
	s32 mode;
	u8 *key;
	u32 key_length;
	u8 __aes_dekey[32 + 32 + 32]; // +32 for cache align
	u8 *aes_dekey;
};

void rtl_crypto_hexdump(unsigned char *buf, unsigned int len);

int rtl_hash_init_ctx(struct crypto_tfm *tfm, struct rtl_hash_ctx *ctx);

int rtl_hash_update(struct rtl_hash_ctx *ctx, u8 *data, u32 length);

int rtl_hash_final(struct rtl_hash_ctx *ctx, u8 *out);

int rtl_hash_digest(struct rtl_hash_ctx *ctx, u8 *data, u32 length,
	u8 *out);

int rtl_cipher_init_ctx(struct crypto_tfm *tfm,
	struct rtl_cipher_ctx *ctx);

int	rtl_cipher_setkey(struct crypto_cipher *cipher,
	struct rtl_cipher_ctx *ctx, const u8 *key, unsigned int keylen);

int rtl_cipher_crypt(struct crypto_cipher *cipher, u8 bEncrypt,
	struct rtl_cipher_ctx *ctx, u8 *src, unsigned int nbytes, u8 *iv, u8 *dst);

#endif
