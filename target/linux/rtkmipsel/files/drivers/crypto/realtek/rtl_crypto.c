/*
 * -------------------------
 * Realtek Crypto Engine
 * -------------------------
 *
 * Realtek Crypto Engine Core Features:
 * 	- Authentication Only (md5/sha1/hmac-md5/hmac-sha1)
 * 	- Encryption/Decryption Only (des/3des/aes with ecb/cbc/ctr modes)
 *	- Authentication then Encryption/Decryption
 *	- Encryption/Decryption then Authentication
 *
 * TODO:
 * 	- Non-Blocking mode
 *
 * Realtek Crypto Engine API in Linux
 * 	- hash: md5/sha1
 * 	- blkcipher: des/des3_ede/aes
 * 	- blkcipher operation modes: ecb/cbc/ctr
 *
 * TODO:
 * 	- hmac
 * 	- aead transforms
 * 	- standard crypto api register procedure
 */

#include <net/rtl/rtl_types.h>
#include <net/rtl/rtl_glue.h>
//#include <asm/rtl865x/rtl865xc_asicregs.h>
#include "rtl_ipsec.h"
#include "../../net/rtl819x/AsicDriver/rtl865xc_asicregs.h"
#if defined(CRYPTO_USE_SCHEDULE)
#include <linux/completion.h>
#include <linux/kthread.h>
#endif

#define RTL_CRYPTO_VERSION "v0.1"

#if defined(CRYPTO_USE_SCHEDULE)
extern int thread_check_hw_crypto_done(void);
extern atomic_t hw_crypto_working;
extern struct completion hw_crypto_working_completion;
extern struct completion hw_crypto_done_completion;
static void *p_crypto_task = NULL;
#endif
#if 0 //def CONFIG_RTK_OPENVPN_HW_CRYPTO
extern int rtl_fast_openvpn_init(void);
#endif
static int __init rtl_crypto_init(void)
{
	printk("Realtek Crypto Engine %s\n", RTL_CRYPTO_VERSION);

#if defined(CONFIG_SMP)
	extern void init_ipsec_lock(void);
	init_ipsec_lock();
#endif

#if defined(CRYPTO_USE_SCHEDULE)
	if(!p_crypto_task) {/*only create once*/
		init_completion(&hw_crypto_working_completion);
		init_completion(&hw_crypto_done_completion);
		atomic_set(&hw_crypto_working,0);
		p_crypto_task = (void*)kthread_run(thread_check_hw_crypto_done,NULL,"thread_check_hw_crypto_done");
		if(!p_crypto_task)
		{
		    printk("[%s:] %d, Create HW Crypto Check Task Failed\n",__func__,__LINE__);
			return -1;
		}
	}
#endif
	rtl_ipsecEngine_init(DEFAULT_DESC_NUM, 2);

	rtl_ipsecSetOption(RTL_IPSOPT_SAWB, 0);
	#ifdef CONFIG_RTK_OPENVPN_HW_CRYPTO
	//rtl_fast_openvpn_init();
	//printk("\n%s:%d call rtl_ipsecSetOption()!!!\n",__FUNCTION__,__LINE__);
	rtl_ipsecSetOption(RTL_IPSOPT_SAWB, 1);
	#endif

	#if defined(CONFIG_CRYPTO_DEV_REALTEK_TEST)
	extern int32 init_proc_debug(void);
	init_proc_debug();
	#endif

#ifdef DBG_HW_REINIT
	rtl_ipsec_info();
#endif
	return 0;
}

subsys_initcall(rtl_crypto_init);
