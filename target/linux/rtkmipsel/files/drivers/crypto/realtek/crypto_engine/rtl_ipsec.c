#include <linux/sched.h>
#include <asm/delay.h>

#ifdef CONFIG_RTL_ICTEST
#include "rtl_types.h"
#include "rtl_ipsec.h"
#include "md5.h"
#include "sha1.h"
#include "hmac.h"
#ifdef CONFIG_RTL865X_MODEL_KERNEL
#include "modelTrace.h"
#include "icExport.h"
#include "virtualMac.h"
#endif
#include <linux/autoconf.h>
#include <asicRegs.h>
#include <rtl_glue.h>
#include <rtl_utils.h>
#include <assert.h>
#else
#include <net/rtl/rtl_types.h>
#include <net/rtl/rtl_glue.h>
//#include <asm/rtl865x/rtl865xc_asicregs.h>
#include "../../../net/rtl819x/AsicDriver/rtl865xc_asicregs.h"
#include "md5.h"
#include "sha1.h"
#include "hmac.h"
#include "rtl_ipsec.h"
#endif
#if defined(CRYPTO_USE_SCHEDULE)
#include <linux/completion.h>
#include <linux/hardirq.h>
#include <linux/sched.h>
#include <asm/atomic.h>
#endif

// address macro
#define PHYSICAL_ADDRESS(x) CPHYSADDR(x)
#define UNCACHED_ADDRESS(x) CKSEG1ADDR(x)
#define CACHED_ADDRESS(x) CKSEG0ADDR(x)

/* Max Scatter List Number*/
#define MAX_SCATTER 8

enum _MODE_SELECT
{
	_MS_CRYPTO = 0,
	_MS_AUTH = 1,
	_MS_AUTH_THEN_DECRYPT = 2,
	_MS_ENCRYPT_THEN_AUTH = 3,
};

enum _AES_KEYlEN
{
	_AES_KEYLEN_NONE = 0,
	_AES_KEYLEN_128BIT = 1,
	_AES_KEYLEN_192BIT = 2,
	_AES_KEYLEN_256BIT = 3,
};

#if 1//defined(CONFIG_SMP)
int lock_ipsec_owner = -1;
spinlock_t lock_ipsec_engine;
EXPORT_SYMBOL(lock_ipsec_owner);
EXPORT_SYMBOL(lock_ipsec_engine);
#endif


int cond_resched_flag = 0;

// debug
int g_rtl_ipsec_dbg = 0;

// descriptors
uint8 g_numSrcDesc=0;
uint32 g_numSrc, g_numDst; /* the number of source & destination descriptor */
uint32 g_idxCpuSrc, g_idxAsicSrc;
uint32 g_idxCpuDst, g_idxAsicDst;
rtl_ipsec_source_t *g_ipssdar;
rtl_ipsec_dest_t *g_ipsddar;

static int8 g_mode32Bytes = 2; // DMA 64 bytes

/* buffer for g_rtl_authEngineInputPad and g_rtl_authEngineOutputPad */
static uint8 g_IOPAD[HMAC_MAX_MD_CBLOCK*2+32] __attribute__ ((aligned (32)));
static uint8 *g_rtl_authEngineInputPad, *g_rtl_authEngineOutputPad;

/* padding of packet tail */
static uint8 g_authPadding[HMAC_MAX_MD_CBLOCK*2+32] __attribute__ ((aligned (32)));
static uint8 *g_pAuthPadding;

#ifdef IPSEC_LOCK
spinlock_t lock_ipsec_engine2;
atomic_t called = ATOMIC_INIT(0);
atomic_t inited = ATOMIC_INIT(0);
#endif

#ifdef HW_REINIT
int is_sawb = -1;

rtl_ipsec_source_t g_src_desc[DEFAULT_DESC_NUM];
rtl_ipsec_dest_t   g_dst_desc[DEFAULT_DESC_NUM];

#define LX_CONTROL		0xb8000014
#define SYS_MSRR		0xb8001038

#define LOCK_LX1_BUS() \
do { \
	REG32(LX_CONTROL) |= (1<<3); /* request locking Lx1 bus */ \
	while( (REG32(LX_CONTROL)&(1<<13)) == 0 ) ; /* wait for Lx0 bus lock okay */ \
	while( (REG32(SYS_MSRR)&(1<<30)) == 0 ) ; /* wait for No on going DRAM command */ \
} while(0)

#define RELEASE_LX1_BUS() \
do { \
	REG32(LX_CONTROL) &= ~(1<<3); /* release Lx1 bus */ \
} while(0)
#endif /* HW_REINIT */

#ifdef DBG_HW_REINIT
static rtl_ipsec_source_t saved_prepSrc[4 + MAX_SCATTER];
static rtl_ipsec_dest_t saved_prepDst[1];

static unsigned long src_ptr = 0, pre_src_ptr = 0;
static unsigned long dst_ptr = 0, pre_dst_ptr = 0;
static unsigned long call_cnt = 0, fail_cnt = 0;
uint32 g_idxCpuSrc_pre = 0, g_idxCpuDst_pre = 0;

#define SAVE_DESC_ENTRY(DST, SRC, SIZE) \
do { \
	memset(DST, 0, SIZE); \
	memcpy(DST, SRC, SIZE); \
} while(0)

#endif /* DBG_HW_REINIT */


static int32 rtl_ipsecEngine_alloc(uint32 descNum)
{
	void *p;

	/* Allocate src descriptor ring */
	g_numSrc = descNum;

#ifdef HW_REINIT
	p = &g_src_desc[0];
#else /* !HW_REINIT */
	p = kmalloc(g_numSrc * sizeof(rtl_ipsec_source_t), GFP_ATOMIC);
	if (p == NULL)
	{
		g_numSrc = -1;
		printk("%s():%d memory allocate failed.\n", __FUNCTION__, __LINE__);
		return FAILED;
	}
#endif /* HW_REINIT */

	g_ipssdar = (rtl_ipsec_source_t *) UNCACHED_ADDRESS(p);
	memset(g_ipssdar, 0, g_numSrc * sizeof(rtl_ipsec_source_t));
	g_ipssdar[g_numSrc - 1].eor = 1;

	WRITE_MEM32(IPSSDAR, PHYSICAL_ADDRESS((uint32) g_ipssdar));

	if (g_rtl_ipsec_dbg)
	{
		printk("Set IPSSDAR=0x%08x, g_numSrc=%d\n",
			PHYSICAL_ADDRESS((uint32)g_ipssdar), g_numSrc);
	}

	/* Allocate dest descriptor ring */
#ifdef HW_REINIT
	//g_numDst = descNum;
	g_numDst = 1;//force one dst desc
	p = &g_dst_desc[0];
#else /* !HW_REINIT */
	g_numDst = descNum;
	p = kmalloc(g_numDst * sizeof(rtl_ipsec_dest_t), GFP_ATOMIC);
	if (p == NULL)
	{
		g_numDst = -1;
		printk("%s():%d memory allocate failed.\n", __FUNCTION__, __LINE__);
		return FAILED;
	}
#endif /* HW_REINIT */

	g_ipsddar = (rtl_ipsec_dest_t *) UNCACHED_ADDRESS(p);
	memset(g_ipsddar, 0, g_numDst * sizeof(rtl_ipsec_dest_t));
	g_ipsddar[g_numDst - 1].eor = 1;

	WRITE_MEM32(IPSDDAR, PHYSICAL_ADDRESS((uint32)g_ipsddar));

	if (g_rtl_ipsec_dbg)
	{
		printk("Set IPSDDAR=0x%08x, g_numDst=%d\n",
			PHYSICAL_ADDRESS((uint32)g_ipsddar), g_numDst);
	}

	return SUCCESS;
}

static int32 rtl_ipsecEngine_free( void )
{
	if (g_ipssdar)
	{
		WRITE_MEM32(IPSSDAR, 0);
	#ifndef HW_REINIT
		kfree((void *) CACHED_ADDRESS(g_ipssdar));
	#endif
		g_ipssdar = NULL;
	}

	if (g_ipsddar)
	{
		WRITE_MEM32(IPSDDAR, 0);
	#ifndef HW_REINIT
		kfree((void *) CACHED_ADDRESS(g_ipsddar));
	#endif
		g_ipsddar = NULL;
	}

	return SUCCESS;
}


/*
 *  descNum -- number of descriptor to be allocated.
 *  mode32Bytes -- 0: 16word
 *                 1: 32word
 *                 2: 64word
 */
int32 __rtl_ipsecEngine_init(uint32 descNum, int8 mode32Bytes)
{
	uint32 burstSize;

#ifdef DBG_HW_REINIT
	pre_src_ptr = pre_dst_ptr = src_ptr = dst_ptr = 0;
	g_idxCpuSrc_pre = g_idxCpuDst_pre = 0;
#endif

	/* key+iv+dataSegs */
	if (descNum < 3)
		return FAILED;

	/* mode32Bytes == 0 or 1 is backward-compatible */
	if (mode32Bytes == 0)
		burstSize = IPS_DMBS_16 | IPS_SMBS_16;
	else if (mode32Bytes == 1)
		burstSize = IPS_DMBS_32 | IPS_SMBS_32;
	else if (mode32Bytes == 2)
		burstSize = IPS_DMBS_64 | IPS_SMBS_64;
	else
		return FAILED;

	//WRITE_MEM32(0xb8000010, READ_MEM32(0xb8000010) | 0xFFFFFE00);
	//WRITE_MEM32(0xb8000010, READ_MEM32(0xb8000010) | (1<<17)|(1<<12)|(1<<13)|(1<<19)|(1<<20));
	#ifdef CONFIG_RTL_8197F
	(*(volatile unsigned int*) 0xb8000050) |= 4;
	(*(volatile unsigned int*) 0xb8000010) |= (0x1A3000);
	#else
	WRITE_MEM32(DLL_REG, READ_MEM32(DLL_REG) | _EN_IPSEC);
	WRITE_MEM32(CLK_MANAGE, READ_MEM32(CLK_MANAGE) | _ACTIVE_IPSEC | _ACTIVE_1X1 | _ACTIVE_1X1_ARB);
	#endif
	#if defined(CONFIG_RTL_8198C) || defined(CONFIG_RTL_8881A)
	/* make sure to turn off otg control when not use this function,  otherwise may casue the whole system unstable
	  * suggested by elvis_lin
	   */
	WRITE_MEM32(OTG_CONTROL, READ_MEM32(OTG_CONTROL) & (~(_ACTIVE_OTGCTRL|_OTGCTRL_STRAT|_PJ_RESET_ENABLE)));
	WRITE_MEM32(OTG_CONTROL, READ_MEM32(OTG_CONTROL) | _OTGCTRL_MUX_SEL);
	#endif

	/* Software Reset */
	WRITE_MEM32(IPSCSR, READ_MEM32(IPSCSR) | IPS_SRST);

	//SMP_LOCK_IPSEC;
	rtl_ipsecEngine_free();
	rtl_ipsecEngine_alloc(descNum);

	#if defined(CONFIG_CRYPTO_DEV_REALTEK_TEST) && defined(CRYPTOTEST_USE_UNCACHED_MALLOC)
	extern int32 rtl_ipsecTest_free( void );
	extern int32 rtl_ipsecTest_alloc(void);
	rtl_ipsecTest_free();
	rtl_ipsecTest_alloc();

	//for kernel testcase
	extern int32 rtl_crypto_helper_free( void );
	extern int32 rtl_crypto_helper_alloc(void);
	rtl_crypto_helper_free();
	rtl_crypto_helper_alloc();
	#endif
	//SMP_UNLOCK_IPSEC;

	g_idxCpuSrc = g_idxAsicSrc = 0;
	g_idxCpuDst = g_idxAsicDst = 0;

	/* We must delay a while after software reset. */
	#ifdef CONFIG_RTL_8197F
	WRITE_MEM32(IPSCTR, IPS_SWRXFF | IPS_SWHASH | IPS_SWTXFF | IPS_LXLITTLE | IPS_CKE | burstSize | IPS_DDLEN_8 );
	#else
	WRITE_MEM32(IPSCTR, 0/*IPS_LBKM*/ | IPS_SAWB | IPS_CKE | burstSize);
	#endif
	/* write 1 to clear */
	WRITE_MEM32(IPSCSR, READ_MEM32(IPSCSR) | IPS_SDUEIP | IPS_SDLEIP | IPS_DDUEIP |
		IPS_DDOKIP);

	// backup param
	g_mode32Bytes = mode32Bytes;

	return SUCCESS;
}

int32 rtl_ipsecEngine_init(uint32 descNum, int8 mode32Bytes)
{
	int32 ret;
	SMP_LOCK_IPSEC;
	ret = __rtl_ipsecEngine_init(descNum, mode32Bytes);
	SMP_UNLOCK_IPSEC;
	return ret;
}
EXPORT_SYMBOL(rtl_ipsecEngine_init);

#ifdef HW_REINIT
inline void show_ipsec_reg(const char *caller, int line)
{
	printk("==> %s:%d, IPSSDAR=0x%x, IPSDDAR=0x%x, "
		"IPSCSR=0x%x, IPSCTR=0x%x\n",
		caller, line,
		READ_MEM32(IPSSDAR), READ_MEM32(IPSDDAR),
		READ_MEM32(IPSCSR), READ_MEM32(IPSCTR));
}

void reinit_ipsecEngine_hw(const char *caller, int line)
{
	printk("==> %s:%d, reinit ipsec hw\n", caller, line);
	LOCK_LX1_BUS();
	WRITE_MEM32(CLK_MANAGE, READ_MEM32(CLK_MANAGE) & ~(_ACTIVE_IPSEC));
	WRITE_MEM32(DLL_REG, READ_MEM32(DLL_REG) & ~(_EN_IPSEC));
	show_ipsec_reg(__func__, __LINE__);

	__rtl_ipsecEngine_init(DEFAULT_DESC_NUM, g_mode32Bytes);

	if (is_sawb == 0 || is_sawb == 1)
		rtl_ipsecSetOption(RTL_IPSOPT_SAWB, is_sawb);

	show_ipsec_reg(__func__, __LINE__);
	RELEASE_LX1_BUS();
}
#endif /* HW_REINIT */


int32 rtl_ipsecEngine_exit(void)
{
	rtl_ipsecEngine_free();
#if defined(CONFIG_CRYPTO_DEV_REALTEK_TEST) && defined(CRYPTOTEST_USE_UNCACHED_MALLOC)
	extern int32 rtl_ipsecTest_free( void );
	rtl_ipsecTest_free();

	extern int32 rtl_crypto_helper_free( void );
	rtl_crypto_helper_free();
#endif
	return SUCCESS;
}

int32 rtl_ipsecEngine_dma_mode(void)
{
	return g_mode32Bytes;
}

#define MAX_SCATTER 8
 int32 g_prepSrc=0;;
int32 g_prepDst=0;
#if defined(CRYPTO_USE_SCHEDULE)
atomic_t hw_crypto_working;
struct completion hw_crypto_working_completion;
struct completion hw_crypto_done_completion;
int32 loopWait_thread;
int thread_check_hw_crypto_done(void)
{
	uint32 ips_mask;
	ips_mask = IPS_SDUEIP | IPS_SDLEIP | IPS_DDUEIP | IPS_DDOKIP |
		IPS_DABFIP;
	set_user_nice(current,19); /*kernel thread own SCHED_NORMAL,cfs defaut,Do not change sched policy*/
    while(1)
	{
		wait_for_completion(&hw_crypto_working_completion);
		/*
		if(atomic_read(&hw_crypto_working))
		{
		    loopWait_thread = 1000;  //only wait 1/1000 of ori most wait time
			while ((READ_MEM32(IPSCSR) & ips_mask) == 0)
			{
				loopWait_thread--;
				if (loopWait_thread == 0)
					break;
			}
		}
		*/
		/*do not wait, schedule cost time,let other task can be schedule*/
		complete(&hw_crypto_done_completion);
	}
}

#endif
/*************************************************************************
 *  Features of ipsec API:
 *    1. scatter list
 *    2. encryption, then hash
 *    3. hash, then decryption.
 *    4. variant key length for AES
 *
 *  scatter[n].ptr, pKey, pIv and pDigest does NOT have 4-byte alignment limitatiuon.
 *  The unit of lenKey is 'byte'.
 *  modeCrypto:
 *          0x00   0x20
 *          DES  / AES
 *   0x01:  3DES   none
 *   0x02:  ECB    ECB
 *   0x04:  Enc    Enc
 *   0x08:  nBlk   nBlk
 *   0x10:  DMA    DMA
 *
 *   CBC_AES:0x20
 *   ECB_AES:0x22
 *   CTR_AES:0x23
 *************************************************************************/
int32 rtl_ipsecEngine(uint32 modeCrypto, uint32 modeAuth,
	uint32 cntScatter, rtl_ipsecScatter_t *scatter, void *pCryptResult,
	uint32 lenCryptoKey, void* pCryptoKey,
	uint32 lenAuthKey, void* pAuthKey,
	void* pIv, void* pPad, void* pDigest,
	uint32 a2eo, uint32 enl)
{
	/* MS=10,11: KeyArray + IvArray + HMAC_PAD + DataSegs + AUTH_PAD */
	static rtl_ipsec_source_t prepSrc[4 + MAX_SCATTER];
	static rtl_ipsec_dest_t prepDst[1];
	static rtl_ipsec_source_t *srcDesc;
	enum _MODE_SELECT modeSelect;
	enum _AES_KEYlEN aesKeyLen;
	MD5_CTX md5Context;
	SHA1_CTX sha1Context;
	uint8 numSrcDesc;
	uint32 totalLen = 0;
	uint64 len64;
	uint8 *uint8Ptr = (uint8 *)&len64;
	uint8 *ipad, *opad;
	uint32 apl;
	uint32 hmac; /* for FS=1 crypto key array descriptor */
	uint32 md5; /* for FS=1 crypto key array descriptor */
	uint32 authPadSpace;
	int i;

#ifdef HW_REINIT
	unsigned long re_poll = 0;
	unsigned long src_offset;
	unsigned long dst_offset;

	if (is_sawb == -1) {
		is_sawb = (READ_MEM32(IPSCTR)&IPS_SAWB)?1:0;
	}
#endif /* HW_REINIT */

#ifdef DBG_HW_REINIT
	call_cnt++;
#endif

	if (g_rtl_ipsec_dbg)
	{
		printk("%s: modeCrypto=0x%x, modeAuth=0x%x, lenCryptoKey=%d, pCryptoKey=%p, "
			"lenAuthKey=%d, pAuthKey=%p, pIv=%p, a2eo=%d, enl=%d, cntScatter=%d\n",
			__FUNCTION__, modeCrypto, modeAuth, lenCryptoKey, pCryptoKey,
			lenAuthKey, pAuthKey, pIv, a2eo, enl, cntScatter);
	}

	/* check scatter list count */
	if (unlikely(cntScatter == 0 || cntScatter > MAX_SCATTER))
	{
		printk("%s():%d Invalid scatter count. (%d)\n", __FUNCTION__, __LINE__,
			cntScatter);
		return FAILED;
	}

	/* statistic total length in the scatter list */
	for (i = 0; i < cntScatter; i++)
	{
		totalLen += scatter[i].len;
		if (unlikely(scatter[i].ptr == NULL))
		{
			printk("%s():%d Invalid scatter pointer: %p\n", __FUNCTION__,
				__LINE__, scatter[i].ptr);
			return FAILED;
		}
	}

	if (unlikely(a2eo & 3))
	{
		printk("%s():%d A2EO(%u) should be 4-bytes alignment.\n",
			__FUNCTION__, __LINE__, a2eo);
		return FAILED;
	}

	if (unlikely(totalLen != a2eo + enl))
	{
		printk("%s():%d Parameters error: totalLen(%d) != a2eo+enl(%d).\n",
			__FUNCTION__, __LINE__, totalLen, a2eo + enl);
		return FAILED;
	}

	/***************************************
	prepare source data descriptor:

		 ----------
		|Key array   |
		 ----------
		|IV array     |
		 ----------
		|PAD array  |
		 ----------
		|Data 1       |
		|	.          |
		|	.          |
		|	.          |
		|Data N       |
		 ----------
		|HMAC APL  |
		 ----------

	*****************************************/

	/* set authenticate type flag */
	if (modeCrypto == _MD_NOCRYPTO)
	{
		modeSelect = _MS_AUTH; /* auth only */
		numSrcDesc = 2 + cntScatter; // HMAC_PAD + DATA + PAD
	}
	else
	{
		if (modeAuth == _MD_NOAUTH)
		{
			modeSelect = _MS_CRYPTO; /* Crypto only */
			numSrcDesc = 2 + cntScatter; // KEY + IV + DATA
		}
		else
		{
			if (modeCrypto & _MASK_CRYPTOTHENAUTH)
				/* encrypt, then auth */
				modeSelect = _MS_ENCRYPT_THEN_AUTH;
			else
				/* auth, then decrypt */
				modeSelect = _MS_AUTH_THEN_DECRYPT;

			numSrcDesc = 4 + cntScatter; // KEY + IV + HMAC_PAD + DATA + PAD
		}
	}

	if (unlikely(numSrcDesc > g_numSrc))
	{
		printk("%s():%d Src Descriptor is not enough. (%d > %d)\n",
			__FUNCTION__, __LINE__, numSrcDesc, g_numSrc);
		return FAILED;
	}
	 g_numSrcDesc=numSrcDesc;
	if (modeAuth == _MD_NOAUTH)
	{
		apl = 0;
		md5 = 0;
		hmac = 0;
	}
	else
	{
		md5 = modeAuth & _MASK_AUTHSHA1MD5 ? 0 : 1;
		hmac = modeAuth & _MASK_AUTHHMAC ? 1 : 0;

		/* calculate APL */
		authPadSpace = ((totalLen + HMAC_MAX_MD_CBLOCK -1) &
			(~(HMAC_MAX_MD_CBLOCK -1))) - totalLen;

		if (authPadSpace > 8)
			// Yes, we have enough space to store 0x80 and dmaLen
			apl = authPadSpace;
		else
			// We don't have enough space to store 0x80 and dmaLen. Allocate another 64 bytes.
			apl = authPadSpace + HMAC_MAX_MD_CBLOCK;

		if (hmac)
		{
			if (unlikely(totalLen + apl >= 0x3FC0)) /* 2**14, MAX_PKTLEN */
			{
				printk("%s():%d dmaLen=%d too long.\n", __FUNCTION__, __LINE__,
					totalLen + apl);
				return FAILED; /* DMA length larger than we can support */
			}
		}
		else
		{
			if (unlikely(totalLen + apl > 0x3FFF)) /* 2**14, MAX_PKTLEN */
			{
				printk("%s():%d dmaLen=%d too long.\n", __FUNCTION__, __LINE__,
					totalLen + apl);
				return FAILED; /* DMA length larger than we can support */
			}
		}
	}

	memset(prepSrc, 0, sizeof(prepSrc));
	memset(prepDst, 0, sizeof(prepDst));
	srcDesc = prepSrc;

	/********************************************
	 * step 1: prepare Key & IV array:
	 ********************************************/

	if (modeCrypto != _MD_NOCRYPTO)
	{
		if (modeCrypto & _MASK_CRYPTOAES) /* AES */
		{
			if (unlikely(totalLen == 0 || totalLen > 16368 || enl & 0xf))
			{
				printk("%s():%d Invalid input length (total=%d, enl=%d)\n",
					__FUNCTION__, __LINE__, totalLen, enl);
				return FAILED;
			}

			switch (lenCryptoKey)
			{
				case 128/8:
					aesKeyLen = _AES_KEYLEN_128BIT;
					break;
				case 192/8:
					aesKeyLen = _AES_KEYLEN_192BIT;
					break;
				case 256/8:
					aesKeyLen = _AES_KEYLEN_256BIT;
					break;
				default:
					printk("%s():%d Invalid AES key length: %d bytes.\n",
						__FUNCTION__, __LINE__, lenCryptoKey);
					return FAILED;
			}

			/* AES */
			srcDesc->own = 1; /* Own by crypto */
			srcDesc->fs = 1;
			srcDesc->ms = modeSelect;
			srcDesc->aeskl = aesKeyLen;

			if (modeCrypto & _MASK_CRYPTODECRYPTO)
				srcDesc->kam = 7; /* Key Application Mechanism: Encryption */
			else
				srcDesc->kam = 0; /* Key Application Mechanism: Decryption */

			switch (modeCrypto & _MASK_CBCECBCTR)
			{
				case _MD_CBC: /* CBC mode */
					srcDesc->cbc = 1;
					break;
				case _MD_ECB: /* ECB mode */
					srcDesc->cbc = 0;
					break;
				case _MD_CTR: /* Counter mode */
					srcDesc->ctr = 1;
					break;
				default:
					printk("%s():%d Unsupported crypto mode: %02x\n",
						__FUNCTION__, __LINE__, modeCrypto);
					return FAILED;
			}

			srcDesc->hmac = hmac;
			srcDesc->md5 = md5;

			/* KEY descriptor */
			srcDesc->sbl = lenCryptoKey; /* Key Array Length */
			srcDesc->a2eo = a2eo;
			srcDesc->enl = enl;
			srcDesc->apl = apl;
			srcDesc->sdbp = PHYSICAL_ADDRESS(pCryptoKey);

			if (unlikely(srcDesc->sdbp & 0x3))
			{
				printk("%s():%d Unable to process non-4-byte aligned KEY\n",
					__FUNCTION__, __LINE__);
				return FAILED;
			}

			srcDesc++;

			/* IV descriptor */
			srcDesc->own = 1; /* Own by crypto */
			srcDesc->fs = 0;
			srcDesc->sbl = 128/8; /* IV Array Length */
			srcDesc->sdbp = PHYSICAL_ADDRESS(pIv);

			if (unlikely(srcDesc->sdbp & 0x3))
			{
				printk("%s():%d Unable to process non-4-byte aligned IV\n",
					__FUNCTION__, __LINE__);
				return FAILED;
			}

			srcDesc++;
		}
		else /* DES/3DES */
		{
			if (unlikely(totalLen == 0 || totalLen > 16376 || enl & 0x7))
			{
				printk("%s():%d Invalid input length (total=%d, enl=%d)\n",
					__FUNCTION__, __LINE__, totalLen, enl);
				return FAILED;
			}

			/* DES/3DES */
			srcDesc->own = 1; /* Own by crypto */
			srcDesc->fs = 1;
			srcDesc->ms = modeSelect;
			srcDesc->aeskl = 0; /* DES/3DES */

			if (modeCrypto & 1)
			{
				/* 3DES */
				if (unlikely(lenCryptoKey != 64/8*3))
				{
					printk("%s():%d Invalid 3DES key length: %d bytes.\n",
						__FUNCTION__, __LINE__, lenCryptoKey);
					return FAILED;
				}

				srcDesc->trides = 1;
				srcDesc->sbl = 24; /* Key Array Length */

				if (modeCrypto & _MASK_CRYPTODECRYPTO)
				{
					srcDesc->kam = 5; /* Encryption */
				}
				else
				{
					static uint8 _swapped[32 + 24 + 32];
					uint8 *swapped;
					uint8* pk = pCryptoKey; /* for compiler */

					swapped = (uint8 *) UNCACHED_ADDRESS(&_swapped[32]);
					srcDesc->kam = 2; /* Decryption */
					/* for descryption, we need to swap K1 and K3 (K1,K2,K3)==>(K3,K2,K1). */
					memcpy(&swapped[16], &pk[0], 8);
					memcpy(&swapped[8], &pk[8], 8);
					memcpy(&swapped[0], &pk[16], 8);
					pCryptoKey = swapped; /* re-pointer to new key */
				}
			}
			else
			{
				/* DES */
				if (unlikely(lenCryptoKey != 64/8))
				{
					printk("%s():%d Invalid DES key length: %d bytes.\n",
						__FUNCTION__, __LINE__, lenCryptoKey);
					return FAILED;
				}

				srcDesc->trides = 0;
				srcDesc->sbl = 8; /* Key Array Length */
				if (modeCrypto & _MASK_CRYPTODECRYPTO)
					srcDesc->kam = 7; /* Encryption */
				else
					srcDesc->kam = 0; /* Decryption */
			}

			if (modeCrypto & _MASK_ECBCBC)
				srcDesc->cbc = 0; /* ECB */
			else
				srcDesc->cbc = 1; /* CBC */

			srcDesc->hmac = hmac;
			srcDesc->md5 = md5;

			/* KEY descriptor */
			srcDesc->a2eo = a2eo;
			srcDesc->enl = enl;
			srcDesc->apl = apl;
			srcDesc->sdbp = PHYSICAL_ADDRESS(pCryptoKey);

			if (unlikely(srcDesc->sdbp & 0x3))
			{
				printk("%s():%d Unable to process non-4-byte aligned KEY\n",
					__FUNCTION__, __LINE__);
				return FAILED;
			}

			srcDesc++;

			/* IV descriptor */
			srcDesc->own = 1;
			srcDesc->fs = 0;
			srcDesc->sbl = 8; /* IV Array Length */
			srcDesc->sdbp = PHYSICAL_ADDRESS(pIv);

			if (unlikely(srcDesc->sdbp & 0x3))
			{
				printk("%s():%d Unable to process non-4-byte aligned IV\n",
					__FUNCTION__, __LINE__);
				return FAILED;
			}

			srcDesc++;
		}
	}

	/********************************************
	 * step 2: prepare PAD array:
	 ********************************************/

	if (modeAuth != _MD_NOAUTH)
	{
		/* Calculating ipad and opad */
		if (hmac) /* HMAC */
		{
			if (modeAuth & _MASK_AUTH_IOPAD_READY)
			{
				/* ipad and opad is pre-computed. */
				ipad = pPad;
				opad = pPad + HMAC_MAX_MD_CBLOCK;
			}
			else
			{
				uint8 AuthTempKey[SHA_DIGEST_LENGTH];

				if (lenAuthKey > HMAC_MAX_MD_CBLOCK)
				{
					if (modeAuth & _MASK_AUTHSHA1MD5)
					{
						/* SHA1 */
						SHA1Init(&sha1Context);
						SHA1Update(&sha1Context, pAuthKey, lenAuthKey);
						SHA1Final(AuthTempKey, &sha1Context);
						pAuthKey = AuthTempKey;
						lenAuthKey = SHA_DIGEST_LENGTH;
					}
					else
					{
						/* MD5 */
						MD5Init(&md5Context);
						MD5Update(&md5Context, pAuthKey, lenAuthKey);
						MD5Final(AuthTempKey, &md5Context);
						pAuthKey = AuthTempKey;
						lenAuthKey = MD5_DIGEST_LENGTH;
					}
				}

				g_rtl_authEngineInputPad =
					(uint8 *) UNCACHED_ADDRESS(&g_IOPAD[0]);
				g_rtl_authEngineOutputPad =
					(uint8 *) UNCACHED_ADDRESS(&g_IOPAD[HMAC_MAX_MD_CBLOCK]);

				memset(g_rtl_authEngineInputPad, 0x36, HMAC_MAX_MD_CBLOCK);
				memset(g_rtl_authEngineOutputPad, 0x5c, HMAC_MAX_MD_CBLOCK);

				for (i=0; i<lenAuthKey; i++)
				{
					g_rtl_authEngineInputPad[i] ^= ((uint8*) pAuthKey)[i];
					g_rtl_authEngineOutputPad[i] ^= ((uint8*) pAuthKey)[i];
				}

				ipad = g_rtl_authEngineInputPad;
				opad = g_rtl_authEngineOutputPad;
			}
		}
		else
		{
			/* HashOnly */
			ipad = NULL;
			opad = NULL;
		}

		/* HMAC_PAD descriptor */
		srcDesc->own = 1; /* Own by crypto */
		srcDesc->fs = (modeSelect == _MS_AUTH); /* AuthOnly mode */

		if (srcDesc->fs)
		{
			srcDesc->ms = modeSelect;
			srcDesc->hmac = hmac;
			srcDesc->md5 = md5;
			srcDesc->a2eo = a2eo;
			srcDesc->enl = enl;
			srcDesc->apl = apl;
		}

		srcDesc->sbl = 128; /* PAD size */
		srcDesc->sdbp = PHYSICAL_ADDRESS(ipad);

		if (unlikely(srcDesc->sdbp & 0x3))
		{
			printk("%s():%d Unable to process non-4-byte aligned HMAC_PAD\n",
				__FUNCTION__, __LINE__);
			return FAILED;
		}

		srcDesc++;
	}

	/********************************************
	 * step 3: prepare Data1 ~ DataN
	 ********************************************/

	for (i = 0; i < cntScatter; i++)
	{
		/* DATA descriptor */
		srcDesc->own = 1;
		srcDesc->fs = 0;
		srcDesc->sbl = scatter[i].len; /* Data Length */
		srcDesc->sdbp = PHYSICAL_ADDRESS(scatter[i].ptr);
		srcDesc++;
	}

	/********************************************
	 * step 4: prepare padding for MD5 and SHA-1
	 ********************************************/

	if (modeAuth != _MD_NOAUTH)
	{
		/* build padding pattern */
		g_pAuthPadding = (uint8 *) UNCACHED_ADDRESS(g_authPadding);
		g_pAuthPadding[0] = 0x80; /* the first byte */
		for (i = 1; i < apl - 8; i++)
			g_pAuthPadding[i] = 0; /* zero bytes */

		/* final, length bytes */
		if (hmac)
			len64 = (totalLen + 64) << 3; /* First padding length is */
		else
			len64 = totalLen << 3; /* First padding length is */

		for (i=0; i<8; i++)
		{
		#ifdef CONFIG_CPU_LITTLE_ENDIAN
			if (md5)
				g_pAuthPadding[apl -8+i] = uint8Ptr[i];
			else
				g_pAuthPadding[apl - i - 1] = uint8Ptr[i];

		#else

			if (md5)
				g_pAuthPadding[apl - i - 1] = uint8Ptr[i];
			else
				g_pAuthPadding[apl - 8 + i] = uint8Ptr[i];

		#endif
		}

		/* AUTH_PAD descriptor */
		srcDesc->own = 1; /* Own by crypto */
		srcDesc->fs = 0;
		srcDesc->sbl = apl; /* PAD size */
		srcDesc->sdbp = PHYSICAL_ADDRESS(g_pAuthPadding);

		if (unlikely(srcDesc->sdbp & 0x3))
		{
			printk("%s():%d Unable to process non-4-byte aligned AUTH_PAD\n",
				__FUNCTION__, __LINE__);
			return FAILED;
		}
	}

	/********************************************
	 * step 5: write software parameters to ASIC
	 ********************************************/

	/* We assume the CPU and ASIC are pointed to the same descriptor.
	 * However, in async mode, this assumption is invalid.
	 */
	if (unlikely(((modeCrypto != (uint32) -1) && (modeCrypto&0x08)) ||
	    ((modeAuth != (uint32) -1) && (modeAuth&0x08 ))))
	{
		// TODO:
		/* non-blocking mode, we cannot expect where ASIC is pointing to. */
	}
	else
	{
		/* blocking mode */
		g_idxAsicSrc = ((rtl_ipsec_source_t*) UNCACHED_ADDRESS(READ_MEM32(IPSSDAR))) - g_ipssdar;
		assert(g_idxAsicSrc == g_idxCpuSrc);
		g_idxAsicDst = ((rtl_ipsec_dest_t*) UNCACHED_ADDRESS(READ_MEM32(IPSDDAR))) - g_ipsddar;
		assert(g_idxAsicDst == g_idxCpuDst);
	}

	/********************************************
	 * prepare destination descriptor.
	 ********************************************/

	prepDst[0].own = 1; /* set owned by ASIC */
	prepDst[0].eor = (g_idxCpuDst == (g_numDst - 1)); /* If final descriptor, set EOR bit */
	prepDst[0].dbl = totalLen + apl; /* destination data length */

	if (pCryptResult)
		prepDst[0].ddbp = PHYSICAL_ADDRESS(pCryptResult);
	else
		/* NOTE: the scatter must be continuous buffer if SAWB is disabled. */
		prepDst[0].ddbp = PHYSICAL_ADDRESS(scatter[0].ptr);

	/********************************************
	 * write prepared descriptors into ASIC
	 ********************************************/

#ifdef HW_REINIT
RE_POLL:

#ifdef DBG_HW_REINIT
	g_idxCpuSrc_pre = g_idxCpuSrc;
	g_idxCpuDst_pre = g_idxCpuDst;
#endif

	/* set g_idxCpuSrc/g_idxCpuDst by IPSSDAR/IPSDDAR */
	src_offset = (unsigned long)(UNCACHED_ADDRESS(READ_MEM32(IPSSDAR))) - (unsigned long)(&g_ipssdar[0]);
	g_idxCpuSrc = (src_offset==0?0:(src_offset/sizeof(rtl_ipsec_source_t)));

	dst_offset = (unsigned long)(UNCACHED_ADDRESS(READ_MEM32(IPSDDAR))) - (unsigned long)(&g_ipsddar[0]);
	g_idxCpuDst = (dst_offset==0?0:(dst_offset/sizeof(rtl_ipsec_dest_t)));

#ifdef DBG_HW_REINIT
	if (g_idxCpuSrc_pre != g_idxCpuSrc || g_idxCpuDst_pre != g_idxCpuDst) {
		printk("==> called[%lu]:fail[%lu]:re[%lu]\n",
			call_cnt, fail_cnt, re_poll);
		printk("\tsrc: pre[0x%x], last[0x%x], reg[0x%x]; idx[%d], re-cal[%d]\n",
			pre_src_ptr, src_ptr, (unsigned long)(UNCACHED_ADDRESS(READ_MEM32(IPSSDAR))),
			g_idxCpuSrc_pre, g_idxCpuSrc);
		printk("\tdst: pre[0x%x], last[0x%x], reg[0x%x]; idx[%d], re-cal[%d]\n\n",
			pre_dst_ptr, dst_ptr, (unsigned long)(UNCACHED_ADDRESS(READ_MEM32(IPSDDAR))),
			g_idxCpuDst_pre, g_idxCpuDst);
	}
#endif /* DBG_HW_REINIT */

#endif /* HW_REINIT */

	/* 1. destination first */
#if 0
	/* skip first word. */
	memcpy(((char*) &g_ipsddar[g_idxCpuDst]) + 4, ((char*) &prepDst[0]) + 4,
		sizeof(g_ipsddar[0]) - 4);
	/* copy the first word. */
	*(uint32*) &g_ipsddar[g_idxCpuDst] = *(uint32*) &prepDst[0];
#endif
	memcpy((char*) &g_ipsddar[g_idxCpuDst], (char*) &prepDst[0], sizeof(g_ipsddar[0]));
  #if 0 //move, set g_idxCpuDst later
	g_idxCpuDst = (g_idxCpuDst + 1) % g_numDst;
  #endif

	/* 2. then source */
	for (i = 0; i < numSrcDesc; i++)
	{
	#ifndef HW_REINIT
		if (likely(g_ipssdar[(g_idxCpuSrc + i) % g_numSrc].own == 0))
	#endif
		{
			/* Owned by CPU, overwrite it ! */
			/* If final descriptor, set EOR bit */
			prepSrc[i].eor = (((g_idxCpuSrc + i) % g_numSrc) == (g_numSrc - 1));
#if 0
			/* skip first word. */
			memcpy(((char*) &g_ipssdar[(g_idxCpuSrc + i) % g_numSrc]) + 4,
				((char*) &prepSrc[i]) + 4, sizeof(g_ipssdar[0]) -4 );
			/* copy the first word. */
			*(uint32*) &g_ipssdar[(g_idxCpuSrc + i) % g_numSrc] =
				*(uint32*) &prepSrc[i];
#endif
			memcpy((char*) &g_ipssdar[(g_idxCpuSrc + i) % g_numSrc],
				(char*) &prepSrc[i], sizeof(g_ipssdar[0]));
		}
	#ifndef HW_REINIT
		else
		{
			/* Owned by ASIC.
			 * Currently, we do not support this situation.
			 * This means one of following:
			 *  1. g_numSrc < prepNum
			 *  2. async operation is fired
			 */
			printk("%s():%d[%d] g_numSrc=%d, prepNum=%d\n",
				__FUNCTION__, __LINE__, (g_idxCpuSrc + i) % g_numSrc,
				g_numSrc, numSrcDesc);
			return FAILED;
		}
	#endif /* !HW_REINIT */
	}

#ifdef DBG_HW_REINIT
	/* check own bit */
	if (((rtl_ipsec_source_t*)(((unsigned long)(&g_ipssdar[g_idxCpuSrc % g_numSrc]))|0x20000000))->own == 0) {
		 printk("==> Src, Owned by CPU.\n");
	}
	if (((rtl_ipsec_dest_t*)(((unsigned long)(&g_ipsddar[g_idxCpuDst]))|0x20000000))->own == 0) {
		 printk("==> Dst, Owned by CPU.\n");
	}

	/* save previous desc addr */
	pre_src_ptr = src_ptr;
	pre_dst_ptr = dst_ptr;

	/* save desc addr */
	src_ptr = (unsigned long)(&g_ipssdar[g_idxCpuSrc]);
	dst_ptr = (unsigned long)(&g_ipsddar[g_idxCpuDst]);
#endif /* DBG_HW_REINIT */

	if (g_rtl_ipsec_dbg)
	{
		printk("g_numSrc=%d prepNum=%d g_idxCpuSrc=%d(%d) g_idxCpuDst=%d(%d)\n",
			g_numSrc, numSrcDesc, g_idxCpuSrc, g_idxAsicSrc, g_idxCpuDst,
			g_idxAsicDst);

		memDump((void*) IPSSDAR, 0x10, "Crypto Engine Registers");

		for (i = 0; i < numSrcDesc; i++)
			memDump(&g_ipssdar[(g_idxCpuSrc + i) % g_numSrc],
				sizeof(prepSrc[0]), "g_ipssdar");

		memDump(&g_ipsddar[(g_idxCpuDst + g_numDst - 1) % g_numDst],
			sizeof(prepDst[0]), "g_ipsddar");
		//memDump(pCryptoKey, lenCryptoKey, "key");
	}
	g_prepSrc= sizeof(prepSrc[0]);
	g_prepDst = sizeof(prepDst[0]);
	g_idxCpuSrc = (g_idxCpuSrc + numSrcDesc) % g_numSrc;
	g_idxCpuDst = (g_idxCpuDst + 1) % g_numDst;

#ifdef CONFIG_RTL865X_MODEL_KERNEL
	model_setTestTarget( IC_TYPE_REAL );
	modelExportSetIcType( IC_TYPE_REAL );
	modelExportSetOutputForm( EXPORT_ICEMON );
	modelExportCryptoRegisters();
#endif

	/********************************************
	 * kick off ipsec engine
	 ********************************************/

	/* Clear OK flag */
	WRITE_MEM32(IPSCSR, READ_MEM32(IPSCSR) | (IPS_SDUEIP | IPS_SDLEIP |
		IPS_DDUEIP | IPS_DDOKIP | IPS_DABFIP));

#ifdef CONFIG_RTL_8198
#ifdef CONFIG_RTL8198_REVISION_B
	if (REG32(BSP_REVR) < BSP_RTL8198_REVISION_B)
#endif
		asm volatile ("nop\n\t"); // need nop in 8198-RevA
#endif

	/* start! */
	WRITE_MEM32(IPSCSR, READ_MEM32(IPSCSR) | IPS_POLL);

	if (unlikely(((modeCrypto!=(uint32) -1) && (modeCrypto&0x08)) ||
	     ((modeAuth!=(uint32) -1) && (modeAuth&0x08))))
	{
		// TODO:
		/* non-blocking mode */
		return SUCCESS;
	}
	else
	{
		/* blocking mode */
		int32 loopWait;
		uint32 ips_mask;
		#if defined(CRYPTO_USE_SCHEDULE)
		atomic_t schedule_count;
		#endif

		ips_mask = IPS_SDUEIP | IPS_SDLEIP | IPS_DDUEIP | IPS_DDOKIP |
			IPS_DABFIP;

		#if defined(CRYPTO_USE_SCHEDULE)
				atomic_set(&schedule_count,0);
		atomic_set(&hw_crypto_working,1);
		loopWait = 1000000;  /*hope long enough,keep ori value*/
		#define MOST_SCHED_COUNT 10000
		while(((READ_MEM32(IPSCSR) & ips_mask) == 0)&&(atomic_read(&schedule_count) < MOST_SCHED_COUNT))/*schedule for long enough*/
		{
		    loopWait--;
		    if((!(in_interrupt())&&(!(preempt_count()&PREEMPT_MASK)))&&(need_resched()))
			{
				complete(&hw_crypto_working_completion);
				wait_for_completion(&hw_crypto_done_completion);
				atomic_add(1,&schedule_count);
			}

			if ((loopWait == 0)||((atomic_read(&schedule_count) >= MOST_SCHED_COUNT)&&((READ_MEM32(IPSCSR) & ips_mask) == 0)))
			{
				printk("%s():%d Wait Timeout. READ_MEM32(IPSCSR)=0x%08x.\n",
						__FUNCTION__, __LINE__, READ_MEM32(IPSCSR));

			#ifdef DBG_HW_REINIT
				fail_cnt++;
				SAVE_DESC_ENTRY(&saved_prepSrc, &prepSrc, sizeof(saved_prepSrc));
				SAVE_DESC_ENTRY(&saved_prepDst, &prepDst, sizeof(saved_prepDst));
			#endif

				rtl_ipsec_info();
				atomic_set(&hw_crypto_working,0);

			#ifdef HW_REINIT
				if (++re_poll < 5) {
					printk("==> %s:%d, re_poll[%d]\n", __FUNCTION__, __LINE__, re_poll);
					reinit_ipsecEngine_hw(__func__, __LINE__);
					goto RE_POLL;
				}
				else {
					printk("==> %s:%d, reach max re_poll, fail\n", __FUNCTION__, __LINE__);
					return FAILED; /*error occurs*/
				}
			#else /* !HW_REINIT */
				return FAILED; /*error occurs*/
			#endif /* HW_REINIT */
			}
		}
		atomic_set(&hw_crypto_working,0);
		#else
		/* wait until ipsec engine stop */
		loopWait = 10000; /* hope long enough */
		while ((READ_MEM32(IPSCSR) & ips_mask) == 0)
		{
			loopWait--;
			if (loopWait == 0)
			{
				printk("%s():%d Wait Timeout. READ_MEM32(IPSCSR)=0x%08x.\n",
					__FUNCTION__, __LINE__, READ_MEM32(IPSCSR));
				rtl_ipsec_info();
				return FAILED; /* error occurs */
			}
			udelay(50);
			if (cond_resched_flag)
			{
				cond_resched();
			}
		}
		#endif

		assert(g_ipsddar[(g_idxCpuDst + g_numDst - 1) % g_numDst].own == 0);

		ips_mask = IPS_SDUEIP | IPS_SDLEIP | IPS_DDUEIP | IPS_DABFIP;
		if (READ_MEM32(IPSCSR) & ips_mask)
		{
			printk("%s():%d IPS_SDLEIP or IPS_DABFI is ON. READ_MEM32(IPSCSR)=0x%08x.\n",
				__FUNCTION__, __LINE__, READ_MEM32(IPSCSR));

		#ifdef DBG_HW_REINIT
			fail_cnt++;
			SAVE_DESC_ENTRY(&saved_prepSrc, &prepSrc, sizeof(saved_prepSrc));
			SAVE_DESC_ENTRY(&saved_prepDst, &prepDst, sizeof(saved_prepDst));
		#endif

			rtl_ipsec_info();

		#ifdef HW_REINIT
			if (++re_poll < 5) {
				printk("==> %s:%d, re_poll[%d]\n", __FUNCTION__, __LINE__, re_poll);
				reinit_ipsecEngine_hw(__func__, __LINE__);
				goto RE_POLL;
			}
			else {
				printk("==> %s:%d, reach max re_poll, fail\n", __FUNCTION__, __LINE__);
				return FAILED; /* error occurs */
			}
		#else /* !HW_REINIT */
			return FAILED; /* error occurs */
		#endif /* HW_REINIT */
		}

	#ifdef HW_REINIT
		if (READ_MEM32(IPSCSR) & IPS_POLL) {
			printk("\n==> %s:%d, poll bit is set??\n",
				__func__, __LINE__);

			reinit_ipsecEngine_hw(__func__, __LINE__);

			if (READ_MEM32(IPSCSR) & IPS_POLL) {
				printk("==> %s:%d, afte hw reinit, poll bit is still set??\n",
					__func__, __LINE__);
			}
		}
	#endif /*  HW_REINIT */

	}

	if (modeAuth != _MD_NOAUTH)
	{
		if (g_rtl_ipsec_dbg)
		{
			if (md5)
				memDump(g_ipsddar[(g_idxCpuDst + g_numDst - 1) % g_numDst].icv, MD5_DIGEST_LENGTH,
					"MD5 Digest: g_ipsddar[(g_idxCpuDst+g_numDst-1) % g_numDst].icv");
			else
				memDump(g_ipsddar[(g_idxCpuDst + g_numDst - 1) % g_numDst].icv, SHA_DIGEST_LENGTH,
					"SHA Digest: g_ipsddar[(g_idxCpuDst+g_numDst-1) % g_numDst].icv");
		}

		if (md5)
			memcpy(pDigest, g_ipsddar[(g_idxCpuDst + g_numDst - 1) % g_numDst].icv,
				MD5_DIGEST_LENGTH); /* Avoid 4-byte alignment limitation */
		else
			memcpy(pDigest, g_ipsddar[(g_idxCpuDst + g_numDst - 1) % g_numDst].icv,
				SHA_DIGEST_LENGTH); /* Avoid 4-byte alignment limitation */
	}

	return SUCCESS;
}
EXPORT_SYMBOL(rtl_ipsecEngine);

int32 rtl_ipsecGetOption( enum RTL_IPSEC_OPTION option, uint32* value )
{
	switch ( option )
	{
		case RTL_IPSOPT_LBKM:
			if ( READ_MEM32(IPSCTR)&IPS_LBKM )
				*value = TRUE;
			else
				*value = FALSE;
			break;

		case RTL_IPSOPT_SAWB:
			if ( READ_MEM32(IPSCTR)&IPS_SAWB )
				*value = TRUE ;
			else
				*value = FALSE;
			break;

		case RTL_IPSOPT_DMBS:
			if ( (READ_MEM32(IPSCTR)&IPS_DMBS_MASK)==IPS_DMBS_16 )
				*value = 16;
			else if ( (READ_MEM32(IPSCTR)&IPS_DMBS_MASK)==IPS_DMBS_32 )
				*value = 32;
			else if ( (READ_MEM32(IPSCTR)&IPS_DMBS_MASK)==IPS_DMBS_64 )
				*value = 64;
			else
				return FAILED;
			break;

		case RTL_IPSOPT_SMBS:
			if ( (READ_MEM32(IPSCTR)&IPS_SMBS_MASK)==IPS_SMBS_16 )
				*value = 16;
			else if ( (READ_MEM32(IPSCTR)&IPS_SMBS_MASK)==IPS_SMBS_32 )
				*value = 32;
			else if ( (READ_MEM32(IPSCTR)&IPS_SMBS_MASK)==IPS_SMBS_64 )
				*value = 64;
			else
				return FAILED;
			break;

		default:
			return FAILED;
	}

	return SUCCESS;
}

int32 rtl_ipsecSetOption( enum RTL_IPSEC_OPTION option, uint32 value )
{
	switch ( option )
	{
		case RTL_IPSOPT_LBKM:
			if ( value == TRUE )
				WRITE_MEM32( IPSCTR, READ_MEM32(IPSCTR)|IPS_LBKM );
			else
				WRITE_MEM32( IPSCTR, READ_MEM32(IPSCTR)&~IPS_LBKM );
			break;

		case RTL_IPSOPT_SAWB:
			if ( value == TRUE )
				WRITE_MEM32( IPSCTR, READ_MEM32(IPSCTR)|IPS_SAWB);
			else
				WRITE_MEM32( IPSCTR, READ_MEM32(IPSCTR)&~IPS_SAWB );
			break;

		case RTL_IPSOPT_DMBS:
			if ( value == 16 )
				WRITE_MEM32( IPSCTR, (READ_MEM32(IPSCTR)&~IPS_DMBS_MASK)|IPS_DMBS_16 );
			else if ( value == 32 )
				WRITE_MEM32( IPSCTR, (READ_MEM32(IPSCTR)&~IPS_DMBS_MASK)|IPS_DMBS_32 );
			else if ( value == 64 )
				WRITE_MEM32( IPSCTR, (READ_MEM32(IPSCTR)&~IPS_DMBS_MASK)|IPS_DMBS_64 );
			else
				return FAILED;
			break;

		case RTL_IPSOPT_SMBS:
			if ( value == 16 )
				WRITE_MEM32( IPSCTR, (READ_MEM32(IPSCTR)&~IPS_SMBS_MASK)|IPS_SMBS_16 );
			else if ( value == 32 )
				WRITE_MEM32( IPSCTR, (READ_MEM32(IPSCTR)&~IPS_SMBS_MASK)|IPS_SMBS_32 );
			else if ( value == 64 )
				WRITE_MEM32( IPSCTR, (READ_MEM32(IPSCTR)&~IPS_SMBS_MASK)|IPS_SMBS_64 );
			else
				return FAILED;
			break;

		default:
			return FAILED;
	}

	return SUCCESS;
}
EXPORT_SYMBOL(rtl_ipsecSetOption);

rtl_ipsec_source_t *get_rtl_ipsec_ipssdar(void)
{
	return g_ipssdar;
}

rtl_ipsec_dest_t *get_rtl_ipsec_ipsddar(void)
{
	return g_ipsddar;
}

void rtl_ipsec_info(void)
{
	int i;
	char buffer[256];

	printk("=========================================\n");
	printk("Crypto Engine Registers\n");
	printk("=========================================\n");

	memDump((void*)IPSSDAR, 4, "IPSSDAR");
	memDump((void*)IPSDDAR, 4, "IPSDDAR");
	memDump((void*)IPSCSR, 4, "IPSCSR");
	memDump((void*)IPSCTR, 4, "IPSCTR");

	g_idxAsicSrc = ((rtl_ipsec_source_t*) UNCACHED_ADDRESS(READ_MEM32(IPSSDAR))) - g_ipssdar;
	for (i=0; i<g_numSrc; i++)
	{
		sprintf(buffer, "ipssdar[%d]:%s%s%s%s (%d+%d+%d,%d)", i,
			i == g_idxCpuSrc ? " [C]" : "",
			i == g_idxAsicSrc ? " [A]" : "",
			g_ipssdar[i].fs ? " [F]" : "",
			g_ipssdar[i].eor ? " [E]" : "",
			g_ipssdar[i].a2eo,
			g_ipssdar[i].enl,
			g_ipssdar[i].apl,
			g_ipssdar[i].sbl
		);
		memDump(&g_ipssdar[i], sizeof(g_ipssdar[0]), buffer);
	}

	g_idxAsicDst = ((rtl_ipsec_dest_t*) UNCACHED_ADDRESS(READ_MEM32(IPSDDAR))) - g_ipsddar;
	for (i=0; i<g_numDst; i++)
	{
		sprintf(buffer, "ipsddar[%d]:%s%s%s", i,
			i == g_idxCpuDst ? " [C]" : "",
			i == g_idxAsicDst ? " [A]" : "",
			g_ipsddar[i].eor ? " [E]" : ""
		);
		memDump(&g_ipsddar[i], sizeof(g_ipsddar[0]), buffer);
	}

#ifdef DBG_HW_REINIT
	sprintf(buffer, "\nSaved address:\n"
		"\tSRC: pre[0x%x], cur[0x%x]\n"
		"\tDST: pre[0x%x], cur[0x%x]\n"
		"Saved_Dst: ",
		pre_src_ptr, src_ptr, pre_dst_ptr, dst_ptr);

	memDump(&saved_prepDst[0], sizeof(saved_prepDst[0]), buffer);
	for (i=0; i<g_numSrcDesc; i++)
	{
		sprintf(buffer, "Saved_Src[%d]: ", i);
		memDump(&saved_prepSrc[i], sizeof(saved_prepSrc[0]), buffer);
	}
	printk("\n\n");
#endif /* DBG_HW_REINIT */

}

#if defined(CONFIG_SMP)
void init_ipsec_lock(void)
{
	lock_ipsec_owner = -1;
	spin_lock_init(&lock_ipsec_engine);

	return ;
}
#endif
