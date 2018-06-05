#ifndef RTL_IPSEC_H
#define RTL_IPSEC_H

//#define CRYPTOTEST_DEBUG

#define CRYPTOTEST_USE_UNCACHED_MALLOC
#define CRYPTO_USE_SCHEDULE

// MAX_AUTH_KEY is SW limited
#define MAX_AUTH_KEY 1024

////////////////////////////////////////////////////
#define IPS_SWDMAW      (1<<23)
#define IPS_SWCDES      (1<<22)
#define IPS_SWHASH      (1<<21)
#define IPS_SWDMAR      (1<<20)
#define IPS_SWFDES      (1<<19)
#define IPS_SWRXFF      (1<<18)
#define IPS_SWTXFF      (1<<17)
#define IPS_LXLITTLE    (1<<10)     /* Lx bus data endian*/


#define IPS_DDLEN_20    (3<<24)                         /* Destination Descriptor Length : Length is 20*DW : for SHA-1/MD5 */
#define IPS_DDLEN_16    (2<<24)                         /* Destination Descriptor Length : Length is 16*DW : for SHA-1/MD5 */
#define IPS_DDLEN_12    (1<<24)                         /* Destination Descriptor Length : Length is 12*DW : for SHA-1/MD5 */
#define IPS_DDLEN_8      (0<<24)                         /* Destination Descriptor Length : Length is 8*DW : for SHA-1/MD5 */
/////////////////////////////////////////////////////
#if defined(CRYPTOTEST_USE_UNCACHED_MALLOC)
#define CRYPTOTEST_KEY_LEN	(32 + 32 + 32)    // 32 for AES-256
#define CRYPTOTEST_IV_LEN	(32 + 16 + 32)    // 16 for AES
#define CRYPTOTEST_AUTH_KEY_LEN	(32 + MAX_AUTH_KEY + 32)  // 20 for SHA1
#define CRYPTOTEST_PAD_LEN	(32 + 128 + 32)//128 for ipad and opad
#define CRYPTOTEST_ASICORG_LEN	(32 + MAX_PKTLEN + 32)
#define CRYPTOTEST_ASICENC_LEN	(32 + MAX_PKTLEN + 32)
#define CRYPTOTEST_ASICDIGEST_LEN	(32 + SHA_DIGEST_LENGTH + 32)// 20 for SHA1
#endif
// modeCrypto
#define _MD_NOCRYPTO 			((uint32)-1)
#define _MD_CBC					(0)
#define _MD_ECB					(1<<1)
#define _MD_CTR					((1<<0)|(1<<1))
#define _MASK_CRYPTOTHENAUTH	(1<<2)
#define _MASK_CRYPTOAES			(1<<5)
#define _MASK_CRYPTODECRYPTO	(1<<2)
#define _MASK_CRYPTO3DESDES		(1<<0)
#define _MASK_CBCECBCTR			((1<<0)|(1<<1))
#define _MASK_ECBCBC			(1<<1)

// modeAuth
#define _MD_NOAUTH				((uint32)-1)
#define _MASK_AUTHSHA1MD5		(1<<0)
#define _MASK_AUTHHMAC			(1<<1)
#define _MASK_AUTH_IOPAD_READY	(1<<2)

#define MAX_PKTLEN (1<<14)

//Bit 0: 0:DES 1:3DES
//Bit 1: 0:CBC 1:ECB
//Bit 2: 0:Decrypt 1:Encrypt
#define DECRYPT_CBC_DES					0x00
#define DECRYPT_CBC_3DES				0x01
#define DECRYPT_ECB_DES					0x02
#define DECRYPT_ECB_3DES				0x03
#define ENCRYPT_CBC_DES					0x04
#define ENCRYPT_CBC_3DES				0x05
#define ENCRYPT_ECB_DES					0x06
#define ENCRYPT_ECB_3DES				0x07
#define RTL8651_CRYPTO_NON_BLOCKING		0x08
#define RTL8651_CRYPTO_GENERIC_DMA		0x10
#define DECRYPT_CBC_AES					0x20
#define DECRYPT_ECB_AES					0x22
#define DECRYPT_CTR_AES					0x23
#define ENCRYPT_CBC_AES					0x24
#define ENCRYPT_ECB_AES					0x26
#define ENCRYPT_CTR_AES					0x27

//Bit 0: 0:MD5 1:SHA1
//Bit 1: 0:Hash 1:HMAC
#define HASH_MD5		0x00
#define HASH_SHA1		0x01
#define HMAC_MD5		0x02
#define HMAC_SHA1		0x03

#define IPSEC_SYSTEM_BASE		0xB8000000
#define CLK_MANAGE		(IPSEC_SYSTEM_BASE+0x0010)	/* 0xB8000010 */
#define DLL_REG			(IPSEC_SYSTEM_BASE+0x0038)    /* 0xB8000038 */
#define OTG_CONTROL		(IPSEC_SYSTEM_BASE+0x0098)    /* 0xB8000098 */

/* System clock/reset manage */
#define _ACTIVE_1X1		(1<<12)
#define _ACTIVE_1X1_ARB (1<<13)
#define _ACTIVE_IPSEC   (1<<17)
#define _ACTIVE_1X2		(1<<19)
#define _ACTIVE_1X2_ARB (1<<20)

/* DLL and delay line control */
#define _EN_IPSEC (1<<23)


/* otg control */
#define _ACTIVE_OTGCTRL      (1<<0)
#define _OTGCTRL_STRAT       (1<<1)
#define _OTGCTRL_MUX_SEL     (1<<2)
#define _OTGCTRL_FORCE_DEV   (1<<3)
#define _PJ_RESET_ENABLE     (1<<4)

#if 1 //defined(CONFIG_SMP)
#include <linux/spinlock.h>
extern int lock_ipsec_owner;
extern spinlock_t lock_ipsec_engine;
#if 1
/* ============ release version ============ */
#define SMP_LOCK_IPSEC			spin_lock(&lock_ipsec_engine)
#define SMP_UNLOCK_IPSEC 		spin_unlock(&lock_ipsec_engine)
#else
/* ============ debug version ============ */
#define SMP_LOCK_IPSEC	\
	do { \
		if(lock_ipsec_owner!=smp_processor_id()) \
			spin_lock(&lock_ipsec_engine); \
		else \
			printk("[%s %s %d] recursion detection in ipsec engine\n",__FILE__, __FUNCTION__,__LINE__); \
		lock_ipsec_owner=smp_processor_id();\
	} while(0)

#define SMP_UNLOCK_IPSEC \
	do { \
		lock_ipsec_owner=-1; \
		spin_unlock(&lock_ipsec_engine); \
	} while(0)

#endif

#if 0
/* ============ release version ============ */
#define SMP_LOCK_BH_IPSEC			spin_lock_bh(&lock_ipsec_engine)
#define SMP_UNLOCK_BH_IPSEC 		spin_unlock_bh(&lock_ipsec_engine)
#else
/* ============ debug version ============ */
#define SMP_LOCK_BH_IPSEC	\
	do { \
		if(lock_ipsec_owner!=smp_processor_id()) \
			spin_lock_bh(&lock_ipsec_engine); \
		else \
			printk("[%s %s %d] recursion detection in ipsec engine\n",__FILE__, __FUNCTION__,__LINE__); \
		lock_ipsec_owner=smp_processor_id();\
	} while(0)

#define SMP_UNLOCK_BH_IPSEC \
	do { \
		lock_ipsec_owner=-1; \
		spin_unlock_bh(&lock_ipsec_engine); \
	} while(0)

#endif

#else
//undefine CONFIG_SMP
#define SMP_LOCK_IPSEC \
do { \
} while(0)

#define SMP_UNLOCK_IPSEC \
do { \
} while(0)
#endif

#if 0
#ifdef RTL_IPSEC_DEBUG
#define assert(expr) \
	do { \
		if(!(expr)) { \
			printk( "%s:%d: assert(%s)\n", \
			__FILE__,__LINE__,#expr); \
	        } \
	} while (0)
#else
#define assert(expr) do {} while (0)
#endif
#endif

/*
 *  ipsec engine supports scatter list: your data can be stored in several segments those are not continuous.
 *  Each scatter points to one segment of data.
 */
typedef struct rtl_ipsecScatter_s {
	int32 len;
	void* ptr;
} rtl_ipsecScatter_t;

enum RTL_IPSEC_OPTION
{
	RTL_IPSOPT_LBKM, /* loopback mode */
	RTL_IPSOPT_SAWB, /* Source Address Write Back */
	RTL_IPSOPT_DMBS, /* Dest Max Burst Size */
	RTL_IPSOPT_SMBS, /* Source Max Burst Size */
};

/**************************************************************************
 * Data Structure for Descriptor
 **************************************************************************/
typedef struct rtl_ipsec_source_s
{
#ifdef CONFIG_CPU_LITTLE_ENDIAN
	uint32 sbl:14;
	uint32 resv2:2;
	uint32 md5:1;
	uint32 hmac:1;
	uint32 ctr:1;
	uint32 cbc:1;
	uint32 trides:1;
	uint32 aeskl:2;
	uint32 kam:3;
	uint32 ms:2;
	uint32 resv1:1;
	uint32 fs:1;
	uint32 eor:1;
	uint32 own:1;

	uint32 enl:14;
	uint32 resv4:2;
	uint32 a2eo:8;
	uint32 resv3:8;

	uint32 resv6:16;
	uint32 apl:8;
	uint32 resv5:8;

#else
	uint32 own:1;
	uint32 eor:1;
	uint32 fs:1;
	uint32 resv1:1;
	uint32 ms:2;
	uint32 kam:3;
	uint32 aeskl:2;
	uint32 trides:1;
	uint32 cbc:1;
	uint32 ctr:1;
	uint32 hmac:1;
	uint32 md5:1;
	uint32 resv2:2;
	uint32 sbl:14;
	uint32 resv3:8;
	uint32 a2eo:8;
	uint32 resv4:2;
	uint32 enl:14;
	uint32 resv5:8;
	uint32 apl:8;
	uint32 resv6:16;
#endif
	uint32 sdbp;
} rtl_ipsec_source_t;

typedef struct rtl_ipsec_dest_s
{
#ifdef CONFIG_CPU_LITTLE_ENDIAN
	uint32 dbl:14;
	uint32 resv1:16;
	uint32 eor:1;
	uint32 own:1;
#else
	uint32 own:1;
	uint32 eor:1;
	uint32 resv1:16;
	uint32 dbl:14;
#endif
	uint32 ddbp;
	uint32 resv2;
	uint32 icv[5];
} rtl_ipsec_dest_t;

int32 rtl_ipsecEngine_init(uint32 descNum, int8 mode32Bytes);
int32 rtl_ipsecEngine_exit(void);
int32 rtl_ipsecEngine_dma_mode(void);

int32 rtl_ipsecEngine(uint32 modeCrypto, uint32 modeAuth,
	uint32 cntScatter, rtl_ipsecScatter_t *scatter, void *pCryptResult,
	uint32 lenCryptoKey, void* pCryptoKey,
	uint32 lenAuthKey, void* pAuthKey,
	void* pIv, void* pPad, void* pDigest,
	uint32 a2eo, uint32 enl);

int32 rtl_ipsecGetOption(enum RTL_IPSEC_OPTION option, uint32* value);
int32 rtl_ipsecSetOption(enum RTL_IPSEC_OPTION option, uint32 value);

rtl_ipsec_source_t *get_rtl_ipsec_ipssdar(void);
rtl_ipsec_dest_t *get_rtl_ipsec_ipsddar(void);

void rtl_ipsec_info(void);

extern int g_rtl_ipsec_dbg;
#if 1//defined(CRYPTOTEST_USE_UNCACHED_MALLOC)
static inline void *UNCACHED_MALLOC(int size)
{
	return ((void *)(((uint32)kmalloc(size, GFP_ATOMIC)) | UNCACHE_MASK));
}
#endif
#endif


/*-----------------*/
#define HW_REINIT

#ifdef HW_REINIT
	#define SRC_DESC_NUM	(5)
	#define DEFAULT_DESC_NUM (1*SRC_DESC_NUM)
	//#define DBG_HW_REINIT
#else
	#define DEFAULT_DESC_NUM (10)
#endif
/*-----------------*/
