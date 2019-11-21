/*
 *  Header file defines some common inline funtions
 *
 *  $Id: 8192cd_util.h,v 1.10.2.4 2010/11/09 09:10:03 button Exp $
 *
 *  Copyright (c) 2009 Realtek Semiconductor Corp.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 */

#ifndef _8192CD_UTIL_H_
#define _8192CD_UTIL_H_

#include <linux/module.h>
#include <linux/pci.h>
#include <linux/spinlock.h>
#include <linux/circ_buf.h>
#include <asm/io.h>

#include "./8192cd_cfg.h"
#include "./8192cd.h"
#include "./wifi.h"
#include "./8192cd_hw.h"

//#include <bsp/bspchip.h>




#define CTRL_LEN_CHECK(__x__,__y__) \
	do { \
		if((__x__ < __y__) || (__y__ < 0)) { \
			panic_printk("!!! error [%s][%d] len=%d \n",__FUNCTION__, __LINE__, __y__); \
		} \
	} while(0)

#define SAVE_INT_AND_CLI(__x__)		spin_lock_irqsave(&priv->pshare->lock, (__x__))
#define RESTORE_INT(__x__)			spin_unlock_irqrestore(&priv->pshare->lock, (__x__))
#define SMP_LOCK(__x__)
#define SMP_UNLOCK(__x__)
#define SMP_LOCK_XMIT(__x__)
#define SMP_UNLOCK_XMIT(__x__)
#define SMP_LOCK_ACL(__x__)
#define SMP_UNLOCK_ACL(__x__)
#define SMP_LOCK_HASH_LIST(__x__)
#define SMP_UNLOCK_HASH_LIST(__x__)
#define SMP_LOCK_ASOC_LIST(__x__)
#define SMP_UNLOCK_ASOC_LIST(__x__)
#define SMP_LOCK_SLEEP_LIST(__x__)
#define SMP_UNLOCK_SLEEP_LIST(__x__)
#define SMP_LOCK_AUTH_LIST(__x__)
#define SMP_UNLOCK_AUTH_LIST(__x__)
#define SMP_LOCK_WAKEUP_LIST(__x__)
#define SMP_UNLOCK_WAKEUP_LIST(__x__)
#define SMP_LOCK_STACONTROL_LIST(__X__)
#define SMP_UNLOCK_STACONTROL_LIST(__X__)
#define SMP_LOCK_SKB(__x__)
#define SMP_UNLOCK_SKB(__x__)
#define SMP_LOCK_BUF(__x__)
#define SMP_UNLOCK_BUF(__x__)
#define SMP_LOCK_RECV(__x__)
#define SMP_UNLOCK_RECV(__x__)
#define SMP_LOCK_RX_DATA(__x__)
#define SMP_UNLOCK_RX_DATA(__x__)
#define SMP_LOCK_RX_MGT(__x__)
#define SMP_UNLOCK_RX_MGT(__x__)
#define SMP_LOCK_RX_CTRL(__x__)
#define SMP_UNLOCK_RX_CTRL(__x__)
#define SMP_LOCK_REORDER_CTRL(__x__)
#define SMP_UNLOCK_REORDER_CTRL(__x__)
#define DEFRAG_LOCK(__x__)			spin_lock_irqsave(&priv->defrag_lock, (__x__))
#define DEFRAG_UNLOCK(__x__)		spin_unlock_irqrestore(&priv->defrag_lock, (__x__))
#define SMP_LOCK_PSK_RESEND(__x__)
#define SMP_UNLOCK_PSK_RESEND(__x__)
#define SMP_LOCK_PSK_GKREKEY(__x__)
#define SMP_UNLOCK_PSK_GKREKEY(__x__)
#define SMP_LOCK_ASSERT()


#define REMAP_AID(p)   p->remapped_aid


/*NOTE if 1.5 seconds should be RTL_SECONDS_TO_JIFFIES(15)/10
  *RTL_MILISECONDS_TO_JIFFIES shoud consider the HZ value
  *for example HZ=100, x should large than 10
  */
#define RTL_SECONDS_TO_JIFFIES(x) ((x)*HZ)
#define RTL_MILISECONDS_TO_JIFFIES(x) (((x)*HZ-1)/1000+1)
#define RTL_10MILISECONDS_TO_JIFFIES(x) (((x)*HZ)/100)
#define RTL_JIFFIES_TO_MICROSECOND ((1000*1000)/HZ)
#define RTL_JIFFIES_TO_MILISECONDS(x) (((x)*1000)/HZ)
#define RTL_JIFFIES_TO_SECOND(x)  ((x)/HZ)

#define RTL_MICROSECONDS_TO_GTIMERCOUNTER(x) ((x)*100/3125 + 1)
#define CHIP_VER_92X_SERIES(priv)		( (priv->pshare->version_id&0xf) < 3)

#define GET_CHIP_VER(priv)		((priv->pshare->version_id&VERSION_MASK))

#define BIT_RTL_ID                            	BIT(23)
#define BIT_SHIFT_CHIP_VER                    	12
#define BIT_MASK_CHIP_VER                     	0xf
#define GET_BIT_CHIP_VER(x)  (((x)>>BIT_SHIFT_CHIP_VER) & BIT_MASK_CHIP_VER)

#define IS_TEST_CHIP_8814(priv)     (RTL_R32(0x0F0) & BIT_RTL_ID)
#define GET_CHIP_VER_8814(priv)     (GET_BIT_CHIP_VER(RTL_R32(0x0F0)))
#define GET_CHIP_VER_8822(priv)     (GET_BIT_CHIP_VER(RTL_R32(0x0F0)))


//#if defined(CONFIG_RTL_92C_SUPPORT) || defined(SUPPORT_RTL8188E_TC)
#define IS_TEST_CHIP(priv)		((priv->pshare->version_id&0x100))
//#endif

#define IS_OUTSRC_CHIP(priv)	(priv->pshare->use_outsrc)

#define IS_HAL_CHIP(priv)	(priv->pshare->use_hal)
#define IS_MACHAL_CHIP(priv)	(priv->pshare->use_macHalAPI)
#define GET_MACHAL_API(priv)	(priv->pHalmac_api)
#define USE_NHM_ACS2(priv) ((GET_CHIP_VER(priv) == VERSION_8192E) ||(GET_CHIP_VER(priv) == VERSION_8812E)||(GET_CHIP_VER(priv) == VERSION_8197F)||(GET_CHIP_VER(priv) == VERSION_8822B))
#define IS_TDMA_DIG_CHIP(priv)		((GET_CHIP_VER(priv) == VERSION_8197F) || ((GET_CHIP_VER(priv) == VERSION_8814A) && priv->pmib->dot11RFEntry.phyBandSelect == PHY_BAND_2G))


//#ifdef CONFIG_RTL_92C_SUPPORT /*tonyWifi 20150211 comment*/
#define IS_88RE(priv)			((priv->pshare->version_id&0x200))
//#endif
#define IS_UMC_A_CUT(priv)		((priv->pshare->version_id&0x4f0)==0x400)
#define IS_UMC_B_CUT(priv)		((priv->pshare->version_id&0x4f0)==0x410)
//#ifdef CONFIG_RTL_8812_SUPPORT
#define IS_B_CUT_8812(priv)	((GET_CHIP_VER(priv) == VERSION_8812E) && ((priv->pshare->version_id&0xf0)==0))
#define IS_C_CUT_8812(priv)	((GET_CHIP_VER(priv) == VERSION_8812E) && ((priv->pshare->version_id&0xf0)==0x10))
//#endif

#define IS_A_CUT_8881A(priv)	((GET_CHIP_VER(priv) == VERSION_8881A) && ((priv->pshare->version_id&0xf0)==0))
#define IS_C_CUT_8192E(priv)	((GET_CHIP_VER(priv) == VERSION_8192E) && ((priv->pshare->version_id&0xf0)>>4 == 0x2))
#define IS_D_CUT_8192E(priv)	((GET_CHIP_VER(priv) == VERSION_8192E) && ((priv->pshare->version_id&0xf0)>>4 == 0x3))

/*check addr1, addr2 is 2 byte alignment first,
    to prevent 97D and older CUP which do not support unalignment access have kernel unaligned access core dump*/
#if 0
#define isEqualMACAddr(addr1,addr2) ((((unsigned long)(addr1)&0x01) == 0 && ((unsigned long)(addr2)&0x01) == 0)? \
                                      ((*(unsigned short*)(addr1) == *(unsigned short*)(addr2)) && (*(unsigned short*)(addr1+2) == *(unsigned short*)(addr2+2)) && (*(unsigned short*)(addr1+4) == *(unsigned short*)(addr2+4))) : \
                                       !memcmp(addr1, addr2, MACADDRLEN)  \
                                     )
#else
#define isEqualMACAddr(addr1,addr2) ((((unsigned long)(addr1)|(unsigned long)(addr2))&0x01) ? \
									((*(unsigned char*)(addr1) ^ (*(unsigned char*)(addr2))) | (*(unsigned char*)(addr1+1) ^ (*(unsigned char*)(addr2+1))) | (*(unsigned char*)(addr1+2) ^ (*(unsigned char*)(addr2+2)))|\
									(*(unsigned char*)(addr1+3) ^ (*(unsigned char*)(addr2+3))) | (*(unsigned char*)(addr1+4) ^ (*(unsigned char*)(addr2+4))) | (*(unsigned char*)(addr1+5) ^ (*(unsigned char*)(addr2+5))))==0 :\
									((*(unsigned short*)(addr1) ^ (*(unsigned short*)(addr2))) | (*(unsigned short*)(addr1+2) ^ (*(unsigned short*)(addr2+2))) | (*(unsigned short*)(addr1+4) ^ (*(unsigned short*)(addr2+4))))==0  \
                                     )

static inline int rtk_memcmp(unsigned char *addr1, unsigned char *addr2, unsigned int len)
{
	unsigned int k, result=0;
	if(((unsigned long)(addr1)|(unsigned long)(addr2)| len) &1) {
		for(k = 0; k<len; k++)
			result |= (addr1[k]^addr2[k]);
		return result;
	} else {
		unsigned short *s1 = (unsigned short *)addr1;
		unsigned short *s2 = (unsigned short *)addr2;
		for(k = 0; k<(len/2); k++, s1++, s2++)
			result |= ((*s1) ^(*s2));
		return result;
	}
}

#if 1
#define copyMACAddr(addr1,addr2)  {\
									if(((unsigned long)(addr1)|(unsigned long)(addr2))&0x01) { \
									(*(unsigned char*)(addr1) = (*(unsigned char*)(addr2))) ; (*(unsigned char*)(addr1+1) = (*(unsigned short*)(addr2+1))) ; (*(unsigned char*)(addr1+2) = (*(unsigned char*)(addr2+2)));\
									(*(unsigned char*)(addr1+3) = (*(unsigned char*)(addr2+3))) ; (*(unsigned char*)(addr1+4) = (*(unsigned short*)(addr2+4))) ; (*(unsigned char*)(addr1+5) = (*(unsigned char*)(addr2+5))); }else {\
									(*(unsigned short*)(addr1) = (*(unsigned short*)(addr2))) ; (*(unsigned short*)(addr1+2) = (*(unsigned short*)(addr2+2))) ; (*(unsigned short*)(addr1+4) = (*(unsigned short*)(addr2+4))); } \
									};
#else
#define copyMACAddr(addr1,addr2)	memcpy(addr1, addr2, MACADDRLEN);
#endif
#endif

#define isEqualIPAddr(addr1,addr2) ((((unsigned long)(addr1)&0x01) == 0 && ((unsigned long)(addr2)&0x01) == 0)? \
                                     ((*(unsigned short*)(addr1) == *(unsigned short*)(addr2)) && (*(unsigned short*)(addr1+2) == *(unsigned short*)(addr2+2))) : \
                                     !memcmp(addr1, addr2, 4)\
                                   )

#define RTL_SET_MASK(reg,mask,val,shift) (((reg)&(~(mask)))|((val)<<(shift)))


#ifdef USE_IO_OPS

#define get_desc(val)           (val)
#define set_desc(val)           (val)

#define RTL_R8(reg)             inb(((unsigned long)priv->pshare->ioaddr) + (reg))
#define RTL_R16(reg)            inw(((unsigned long)priv->pshare->ioaddr) + (reg))
#define RTL_R32(reg)            ((unsigned long)inl(((unsigned long)priv->pshare->ioaddr) + (reg)))
#define RTL_W8(reg, val8)       outb((val8), ((unsigned long)priv->pshare->ioaddr) + (reg))
#define RTL_W16(reg, val16)     outw((val16), ((unsigned long)priv->pshare->ioaddr) + (reg))
#define RTL_W32(reg, val32)     outl((val32), ((unsigned long)priv->pshare->ioaddr) + (reg))
#define RTL_W8_F                RTL_W8
#define RTL_W16_F               RTL_W16
#define RTL_W32_F               RTL_W32
#undef readb
#undef readw
#undef readl
#undef writeb
#undef writew
#undef writel
#define readb(addr)             inb((unsigned long)(addr))
#define readw(addr)             inw((unsigned long)(addr))
#define readl(addr)             inl((unsigned long)(addr))
#define writeb(val,addr)        outb((val), (unsigned long)(addr))
#define writew(val,addr)        outw((val), (unsigned long)(addr))
#define writel(val,addr)        outl((val), (unsigned long)(addr))

#else // !USE_IO_OPS

#define PAGE_NUM 15

	#define IO_TYPE_CAST	(unsigned char *)

#ifndef REG32
    #define REG32(reg)      (*(volatile unsigned int *)(reg))
#endif


static __inline__ unsigned char RTL_R8_F(struct rtl8192cd_priv *priv, unsigned int reg)
{
	unsigned long ioaddr = priv->pshare->ioaddr;
	unsigned char val8 = 0;

	if (GET_CHIP_VER(priv) == VERSION_8197F) {
	  if(!(REG32(0xB8000064)&BIT0)){
	  	panic_printk("Should not access WiFi register since 0xB8000064[0]=0\n");
		return;
	  }
	}

#ifdef IO_MAPPING
	unsigned char page = ((unsigned char)(reg >> 8)) & PAGE_NUM;
	if (priv->pshare->io_mapping && page)
	{
		unsigned long x;
		SAVE_INT_AND_CLI(x);

		writeb(readb(IO_TYPE_CAST(ioaddr + _PSR_)) | page, IO_TYPE_CAST(ioaddr + _PSR_));
		val8 = readb(IO_TYPE_CAST(ioaddr + (reg & 0x000000ff)));
		writeb(readb(IO_TYPE_CAST(ioaddr + _PSR_)) & (~PAGE_NUM), IO_TYPE_CAST(ioaddr + _PSR_));

		RESTORE_INT(x);
	}
	else
#endif
	{
		val8 = readb(IO_TYPE_CAST(ioaddr + reg));
	}

	return val8;
}

static __inline__ unsigned short RTL_R16_F(struct rtl8192cd_priv *priv, unsigned int reg)
{
	unsigned long ioaddr = priv->pshare->ioaddr;
	unsigned short val16 = 0;

	if (GET_CHIP_VER(priv) == VERSION_8197F) {
	  if(!(REG32(0xB8000064)&BIT0)){
		panic_printk("Should not access WiFi register since 0xB8000064[0]=0\n");
		return;
	  }
	}


	if (reg & 0x00000001)
		panic_printk("Unaligned read to reg 0x%08x!\n", reg);

#ifdef IO_MAPPING
	unsigned char page = ((unsigned char)(reg >> 8)) & PAGE_NUM;
	if (priv->pshare->io_mapping && page)
	{
		unsigned long x;
		SAVE_INT_AND_CLI(x);

		writeb(readb(IO_TYPE_CAST(ioaddr + _PSR_)) | page, IO_TYPE_CAST(ioaddr + _PSR_));
		val16 = readw(IO_TYPE_CAST(ioaddr + (reg & 0x000000ff)));
		writeb(readb(IO_TYPE_CAST(ioaddr + _PSR_)) & (~PAGE_NUM), IO_TYPE_CAST(ioaddr + _PSR_));

		RESTORE_INT(x);
	}
	else
#endif
	{
		val16 = readw(IO_TYPE_CAST(ioaddr + reg));
	}

#ifdef CHECK_SWAP
	if (priv->pshare->type & ACCESS_SWAP_IO)
		val16 = le16_to_cpu(val16);
#endif

	return val16;
}

static __inline__ unsigned int RTL_R32_F(struct rtl8192cd_priv *priv, unsigned int reg)
{
	unsigned long ioaddr = priv->pshare->ioaddr;
	unsigned int val32 = 0;

	if (GET_CHIP_VER(priv) == VERSION_8197F) {
	  if(!(REG32(0xB8000064)&BIT0)){
		panic_printk("Should not access WiFi register since 0xB8000064[0]=0\n");
		return;
	  }
	}

	if (reg & 0x00000003)
		panic_printk("Unaligned read to reg 0x%08x!\n", reg);

#ifdef IO_MAPPING
	unsigned char page = ((unsigned char)(reg >> 8)) & PAGE_NUM;
	if (priv->pshare->io_mapping && page)
	{
		unsigned long x;
		SAVE_INT_AND_CLI(x);

		writeb(readb(IO_TYPE_CAST(ioaddr + _PSR_)) | page, IO_TYPE_CAST(ioaddr + _PSR_));
		val32 = readl(IO_TYPE_CAST(ioaddr + (reg & 0x000000ff)));
		writeb(readb(IO_TYPE_CAST(ioaddr + _PSR_)) & (~PAGE_NUM), IO_TYPE_CAST(ioaddr + _PSR_));

		RESTORE_INT(x);
	}
	else
#endif
	{
		val32 = readl(IO_TYPE_CAST(ioaddr + reg));
	}

#ifdef CHECK_SWAP
	if (priv->pshare->type & ACCESS_SWAP_IO)
		val32 = le32_to_cpu(val32);
#endif

	return val32;
}

static __inline__ void RTL_W8_F(struct rtl8192cd_priv *priv, unsigned int reg, unsigned char val8)
{
	unsigned long ioaddr = priv->pshare->ioaddr;

	if (GET_CHIP_VER(priv) == VERSION_8197F) {
	  if(!(REG32(0xB8000064)&BIT0)){
		panic_printk("Should not access WiFi register since 0xB8000064[0]=0\n");
		return;
	  }
	}

#ifdef IO_MAPPING
	unsigned char page = ((unsigned char)(reg >> 8)) & PAGE_NUM;
	if (priv->pshare->io_mapping && page)
	{
		unsigned long x;
		SAVE_INT_AND_CLI(x);

		writeb(readb(IO_TYPE_CAST(ioaddr + _PSR_)) | page, IO_TYPE_CAST(ioaddr + _PSR_));
		writeb(val8, IO_TYPE_CAST(ioaddr + (reg & 0x000000ff)));
		writeb(readb(IO_TYPE_CAST(ioaddr + _PSR_)) & (~PAGE_NUM), IO_TYPE_CAST(ioaddr + _PSR_));

		RESTORE_INT(x);
	}
	else
#endif
	{
		writeb(val8, IO_TYPE_CAST(ioaddr + reg));
	}
}

static __inline__ void RTL_W16_F(struct rtl8192cd_priv *priv, unsigned int reg, unsigned short val16)
{
	unsigned long ioaddr = priv->pshare->ioaddr;
	unsigned short val16_n = val16;
#ifdef IO_MAPPING
	unsigned char page;
#endif

	if (GET_CHIP_VER(priv) == VERSION_8197F) {
	  if(!(REG32(0xB8000064)&BIT0)){
		panic_printk("Should not access WiFi register since 0xB8000064[0]=0\n");
		return;
	  }
	}

	if (reg & 0x00000001)
		panic_printk("Unaligned write to reg 0x%08x!, val16=0x%08x!\n", reg, val16);

#ifdef CHECK_SWAP
	if (priv->pshare->type & ACCESS_SWAP_IO)
		val16_n = cpu_to_le16(val16);
#endif

#ifdef IO_MAPPING
	page = ((unsigned char)(reg >> 8)) & PAGE_NUM;
	if (priv->pshare->io_mapping && page)
	{
		unsigned long x;
		SAVE_INT_AND_CLI(x);

		writeb(readb(IO_TYPE_CAST(ioaddr + _PSR_)) | page, IO_TYPE_CAST(ioaddr + _PSR_));
		writew(val16_n, IO_TYPE_CAST(ioaddr + (reg & 0x000000ff)));
		writeb(readb(IO_TYPE_CAST(ioaddr + _PSR_)) & (~PAGE_NUM), IO_TYPE_CAST(ioaddr + _PSR_));

		RESTORE_INT(x);
	}
	else
#endif
	{
		writew(val16_n, IO_TYPE_CAST(ioaddr + reg));
	}
}

static __inline__ void RTL_W32_F(struct rtl8192cd_priv *priv, unsigned int reg, unsigned int val32)
{
	unsigned long ioaddr = priv->pshare->ioaddr;
	unsigned int val32_n = val32;
#ifdef IO_MAPPING
	unsigned char page;
#endif

	if (GET_CHIP_VER(priv) == VERSION_8197F) {
	  if(!(REG32(0xB8000064)&BIT0)){
		panic_printk("Should not access WiFi register since 0xB8000064[0]=0\n");
		return;
	  }
	}

	if (reg & 0x00000003)
		panic_printk("Unaligned write to reg 0x%08x!, val32=0x%08x!\n", reg, val32);

#ifdef CHECK_SWAP
	if (priv->pshare->type & ACCESS_SWAP_IO)
		val32_n = cpu_to_le32(val32);
#endif

#ifdef IO_MAPPING
	page = ((unsigned char)(reg >> 8)) & PAGE_NUM;
	if (priv->pshare->io_mapping && page)
	{
		unsigned long x;
		SAVE_INT_AND_CLI(x);

		writeb(readb(IO_TYPE_CAST(ioaddr + _PSR_)) | page, IO_TYPE_CAST(ioaddr + _PSR_));
		writel(val32_n, IO_TYPE_CAST(ioaddr + (reg & 0x000000ff)));
		writeb(readb(IO_TYPE_CAST(ioaddr + _PSR_)) & (~PAGE_NUM), IO_TYPE_CAST(ioaddr + _PSR_));

		RESTORE_INT(x);
	}
	else
#endif
	{
		writel(val32_n, IO_TYPE_CAST(ioaddr + reg));
	}
}

#if   defined(CONFIG_32K)

#define RTL_R8(reg)		\
	(priv->offload_32k_flag ==1 ? 0 :(RTL_R8_F(priv, reg)) )

#define RTL_R16(reg)	\
	(priv->offload_32k_flag ==1 ? 0 : (RTL_R16_F(priv, reg)))

#define RTL_R32(reg)	\
	(priv->offload_32k_flag ==1 ? 0 : (RTL_R32_F(priv, reg)))

#define RTL_W8(reg, val8)	\
	do { \
	if( priv->offload_32k_flag ==1) \
		{  	/*printk("Error!!! w8:%x,%x\n", reg, val8);*/} \
	else \
		RTL_W8_F(priv, reg, val8); \
	} while (0)

#define RTL_W16(reg, val16)	\
	do { \
	if( priv->offload_32k_flag ==1) \
		{/*printk("Err!!! w16:%x,%x\n", reg, val16);*/} \
	else \
		RTL_W16_F(priv, reg, val16); \
	} while (0)

#define RTL_W32(reg, val32)	\
	do { \
	if( priv->offload_32k_flag ==1) \
		{/*printk("Err!!! w32:%x,%x\n", reg, (unsigned int)val32); */} \
	else \
		RTL_W32_F(priv, reg, val32) ; \
	} while (0)


#else

#define RTL_R8(reg)		\
	(RTL_R8_F(priv, reg))

#define RTL_R16(reg)	\
	(RTL_R16_F(priv, reg))

#define RTL_R32(reg)	\
	(RTL_R32_F(priv, reg))

#define RTL_W8(reg, val8)	\
	do { \
		RTL_W8_F(priv, reg, val8); \
	} while (0)

#define RTL_W16(reg, val16)	\
	do { \
		RTL_W16_F(priv, reg, val16); \
	} while (0)

#define RTL_W32(reg, val32)	\
	do { \
		RTL_W32_F(priv, reg, val32) ; \
	} while (0)

#endif


#ifdef CHECK_SWAP
#define get_desc(val)	((priv->pshare->type & ACCESS_SWAP_MEM) ? le32_to_cpu(val) : val)
#define set_desc(val)	((priv->pshare->type & ACCESS_SWAP_MEM) ? cpu_to_le32(val) : val)
#else
#define get_desc(val)	(val)
#define set_desc(val)	(val)
#endif

#endif // USE_IO_OPS



#define get_tofr_ds(pframe)	((GetToDs(pframe) << 1) | GetFrDs(pframe))

#define is_qos_data(pframe)	((GetFrameSubType(pframe) & (WIFI_DATA_TYPE | BIT(7))) == (WIFI_DATA_TYPE | BIT(7)))

#define UINT32_DIFF(a, b)		((a >= b)? (a - b):(0xffffffff - b + a + 1))

static __inline__ struct list_head *dequeue_frame(struct rtl8192cd_priv *priv, struct list_head *head)
{
	unsigned long flags=0;
	struct list_head *pnext;

	SAVE_INT_AND_CLI(flags);
	if (list_empty(head)) {
		RESTORE_INT(flags);
		return (void *)NULL;
	}

	pnext = head->next;
	list_del_init(pnext);

	RESTORE_INT(flags);

	return pnext;
}

static __inline__ int wifi_mac_hash(unsigned char *mac)
{
	unsigned long x;

	x = mac[0];
	x = (x << 2) ^ mac[1];
	x = (x << 2) ^ mac[2];
	x = (x << 2) ^ mac[3];
	x = (x << 2) ^ mac[4];
	x = (x << 2) ^ mac[5];

	x ^= x >> 8;

	return x & (NUM_STAT - 1);
}

#define get_pfrinfo(pskb)		((struct rx_frinfo *)((unsigned long)(pskb->data) - sizeof(struct rx_frinfo)))

#define get_pskb(pfrinfo)		(pfrinfo->pskb)

#define get_pframe(pfrinfo)		((unsigned char *)((unsigned long)(pfrinfo->pskb->data)))


#define rtl_atomic_inc(ptr_atomic_t)	atomic_inc(ptr_atomic_t)
#define rtl_atomic_dec(ptr_atomic_t)	atomic_dec(ptr_atomic_t)
#define rtl_atomic_read(ptr_atomic_t)	atomic_read(ptr_atomic_t)
#define rtl_atomic_set(ptr_atomic_t, i)	atomic_set(ptr_atomic_t,i)

enum _skb_flag_ {
	_SKB_TX_ = 1,
	_SKB_RX_ = 2,
	_SKB_RX_IRQ_ = 4,
	_SKB_TX_IRQ_ = 8
};

// Allocate net device socket buffer
extern __MIPS16 __IRAM_IN_865X struct sk_buff *alloc_skb_from_queue(struct rtl8192cd_priv *priv);
static __inline__ struct sk_buff *rtl_dev_alloc_skb(struct rtl8192cd_priv *priv,
				unsigned int length, int flag, int could_alloc_from_kerenl)
{
	struct sk_buff *skb = NULL;

	skb = alloc_skb_from_queue(priv);

	if (skb == NULL && could_alloc_from_kerenl)
		skb = dev_alloc_skb(length);

#ifdef ENABLE_RTL_SKB_STATS
	if (NULL != skb) {
		if (flag & (_SKB_TX_ | _SKB_TX_IRQ_))
			rtl_atomic_inc(&priv->rtl_tx_skb_cnt);
		else
			rtl_atomic_inc(&priv->rtl_rx_skb_cnt);
	}
#endif

	return skb;
}

// Free net device socket buffer
static __inline__ void rtl_kfree_skb(struct rtl8192cd_priv *priv, struct sk_buff *skb, int flag)
{
#ifdef ENABLE_RTL_SKB_STATS
	if (flag & (_SKB_TX_ | _SKB_TX_IRQ_))
		rtl_atomic_dec(&priv->rtl_tx_skb_cnt);
	else
		rtl_atomic_dec(&priv->rtl_rx_skb_cnt);
#endif

	dev_kfree_skb_any(skb);
}

static __inline__ int is_CCK_rate(unsigned char rate)
{
	if ((rate == 2) || (rate == 4) || (rate == 11) || (rate == 22))
		return TRUE;
	else
		return FALSE;
}

static __inline__ int is_OFDM_rate(unsigned char rate)
{
	if ((rate == _6M_RATE_) || (rate == _9M_RATE_) || (rate == _12M_RATE_) || (rate == _18M_RATE_) || (rate == _24M_RATE_) || (rate == _36M_RATE_) || (rate == _48M_RATE_) || (rate == _54M_RATE_))
		return TRUE;
	else
		return FALSE;
}

static __inline__ int is_MCS_rate(unsigned char rate)
{
	if (rate >= HT_RATE_ID)
		return TRUE;
	else
		return FALSE;
}

static __inline__ int is_HT_rate(unsigned char rate)
{
	if (rate >= HT_RATE_ID && rate < VHT_RATE_ID)
		return TRUE;
	else
		return FALSE;
}

static __inline__ int is_2T_rate(unsigned char rate)
{
#ifdef RTK_AC_SUPPORT
	if ((rate >= _NSS2_MCS0_RATE_) && (rate <= _NSS2_MCS9_RATE_))
		return TRUE;
	else
#endif
		return ((rate >= _MCS8_RATE_) && (rate <= _MCS16_RATE_)) ? TRUE : FALSE;
}


static __inline__ int is_3T_rate(unsigned char rate)
{
#ifdef RTK_AC_SUPPORT
	if ((rate >= _NSS3_MCS0_RATE_) && (rate <= _NSS3_MCS9_RATE_))
		return TRUE;
	else
#endif
		return ((rate >= _MCS16_RATE_) && (rate <= _MCS23_RATE_)) ? TRUE : FALSE;
}

static __inline__ int is_4T_rate(unsigned char rate)
{
#ifdef RTK_AC_SUPPORT
	if ((rate >= _NSS4_MCS0_RATE_) && (rate <= _NSS4_MCS9_RATE_))
		return TRUE;
	else
#endif
		return ((rate >= _MCS24_RATE_) && (rate <= _MCS31_RATE_)) ? TRUE : FALSE;
}

static __inline__ int is_auto_rate(struct rtl8192cd_priv *priv, struct stat_info *pstat)
{
			return (priv->pmib->dot11StationConfigEntry.autoRate);
}

static __inline__ int is_fixedMCSTxRate(struct rtl8192cd_priv *priv, struct stat_info *pstat)
{
	return (priv->pmib->dot11StationConfigEntry.fixedTxRate & 0xffff000);
}


#ifdef  RTK_AC_SUPPORT
static __inline__ int is_VHT_rate(unsigned char rate)
{
	if (rate >= VHT_RATE_ID)
		return TRUE;
	else
		return FALSE;
}

static __inline__ int is_fixedVHTTxRate(struct rtl8192cd_priv *priv, struct stat_info *pstat)
{
	return ((priv->pmib->dot11StationConfigEntry.fixedTxRate & BIT(31)) ? 1 : 0);
}
#endif

static __inline__ int is_MCS_1SS_rate(unsigned char rate)
{
	if ((rate >= _MCS0_RATE_) && (rate <= _MCS7_RATE_))
		return TRUE;
	else
		return FALSE;
}

static __inline__ int is_MCS_2SS_rate(unsigned char rate)
{
	if ((rate >= _MCS8_RATE_) && (rate <= _MCS15_RATE_))
		return TRUE;
	else
		return FALSE;
}

static __inline__ int is_MCS_3SS_rate(unsigned char rate)
{
	if ((rate >= _MCS16_RATE_) && (rate <= _MCS23_RATE_))
		return TRUE;
	else
		return FALSE;
}

static __inline__ int is_MCS_4SS_rate(unsigned char rate)
{
	if ((rate >= _MCS24_RATE_) && (rate <= _MCS31_RATE_))
		return TRUE;
	else
		return FALSE;
}

static __inline__ int is_1SS_rate(unsigned char rate)
{
#ifdef RTK_AC_SUPPORT
	if ((rate >= _NSS1_MCS0_RATE_) && (rate <= _NSS1_MCS9_RATE_))
		return TRUE;
	else
#endif
		return (((rate >= _MCS0_RATE_) && (rate <= _MCS7_RATE_)) || rate <= _54M_RATE_) ? TRUE : FALSE;
}

static __inline__ int is_2SS_rate(unsigned char rate)
{
#ifdef RTK_AC_SUPPORT
	if ((rate >= _NSS2_MCS0_RATE_) && (rate <= _NSS2_MCS9_RATE_))
		return TRUE;
	else
#endif
		return ((rate >= _MCS8_RATE_) && (rate <= _MCS15_RATE_)) ? TRUE : FALSE;
}

static __inline__ int is_3SS_rate(unsigned char rate)
{
#ifdef RTK_AC_SUPPORT
	if ((rate >= _NSS3_MCS0_RATE_) && (rate <= _NSS3_MCS9_RATE_))
		return TRUE;
	else
#endif
		return ((rate >= _MCS16_RATE_) && (rate <= _MCS23_RATE_)) ? TRUE : FALSE;
}

static __inline__ int is_4SS_rate(unsigned char rate)
{
#ifdef RTK_AC_SUPPORT
	if ((rate >= _NSS4_MCS0_RATE_) && (rate <= _NSS4_MCS9_RATE_))
		return TRUE;
	else
#endif
		return ((rate >= _MCS23_RATE_) && (rate <= _MCS31_RATE_)) ? TRUE : FALSE;
}


static __inline__ void rtl_cache_sync_wback(struct rtl8192cd_priv *priv, unsigned long start,
				unsigned int size, int direction)
{
		if (0 == size) return;	// if the size of cache sync is equal to zero, don't do sync action
		start = CPHYSADDR(start)+CONFIG_LUNA_SLAVE_PHYMEM_OFFSET;//CPHYSADDR is virt_to_bus

		if (direction == PCI_DMA_FROMDEVICE)
		    _dma_cache_inv((unsigned long)bus_to_virt(start-CONFIG_LUNA_SLAVE_PHYMEM_OFFSET), size);
		else
    		_dma_cache_wback_inv((unsigned long)bus_to_virt(start-CONFIG_LUNA_SLAVE_PHYMEM_OFFSET), size);
}

static __inline__ unsigned long get_physical_addr(struct rtl8192cd_priv *priv, void *ptr,
				unsigned int size, int direction)
{
		return (virt_to_bus(ptr)+CONFIG_LUNA_SLAVE_PHYMEM_OFFSET);
}



static __inline__ int can_enable_rx_ldpc(struct rtl8192cd_priv *priv)
{
	if(priv->pmib->dot11nConfigEntry.dot11nLDPC == 3) //force enable rx ldpc
		return 1;

	if(GET_CHIP_VER(priv) == VERSION_8197F)
		return 0;
	else if(GET_CHIP_VER(priv) >= VERSION_8814A)
		return 1;
	else
#ifdef RTK_AC_SUPPORT //for 11ac logo
	if(AC_SIGMA_MODE != AC_SIGMA_NONE)
		return 1;
	else
#endif
		return 0;
}


/*
 *  find a token in a string. If succes, return pointer of token next. If fail, return null
 */
static __inline__ char *get_value_by_token(char *data, char *token)
{
		int idx=0, src_len=strlen(data), token_len=strlen(token);

		while (src_len >= token_len) {
			if (!memcmp(&data[idx], token, token_len))
				return (&data[idx+token_len]);
			src_len--;
			idx++;
		}
		return NULL;
}
static __inline__ int get_rf_NTx(unsigned char mimo_mode)
{
	u1Byte			Ntx = 0;

	if(mimo_mode == MIMO_4T4R)
		Ntx = 4;
	else if(mimo_mode== MIMO_3T3R)
		Ntx = 3;
	else if(mimo_mode == MIMO_2T4R)
		Ntx = 2;
	else if(mimo_mode == MIMO_2T2R)
		Ntx = 2;
	else
		Ntx = 1;

	return Ntx;

}

#ifdef RTK_AC_SUPPORT
static __inline__ int get_sta_vht_mimo_mode(struct stat_info *pstat) {

	u1Byte		sta_mimo_mode = -1;


	if(pstat->vht_cap_len) {
		if(((le32_to_cpu(pstat->vht_cap_buf.vht_support_mcs[0])>>8)&3)==3) // no support RX 5ss
			sta_mimo_mode = MIMO_4T4R;
		if(((le32_to_cpu(pstat->vht_cap_buf.vht_support_mcs[0])>>6)&3)==3) // no support RX 4ss
			sta_mimo_mode = MIMO_3T3R;
		if(((le32_to_cpu(pstat->vht_cap_buf.vht_support_mcs[0])>>4)&3)==3) // no support RX 3ss
			sta_mimo_mode = MIMO_2T2R;
		if(((le32_to_cpu(pstat->vht_cap_buf.vht_support_mcs[0])>>2)&3)==3) // no support RX 2ss
			sta_mimo_mode = MIMO_1T1R;
	}
	return sta_mimo_mode;
}
#endif

static __inline__ int get_rf_mimo_mode(struct rtl8192cd_priv *priv)
{

	//if(GET_CHIP_VER(priv) == VERSION_8822B)
	//	return MIMO_2T2R;

	if ((priv->pshare->phw->MIMO_TR_hw_support == MIMO_1T1R) ||
		(priv->pmib->dot11RFEntry.MIMO_TR_mode == MIMO_1T1R))
		return MIMO_1T1R;
	else if (priv->pshare->phw->MIMO_TR_hw_support == MIMO_2T2R ||
		priv->pmib->dot11RFEntry.MIMO_TR_mode == MIMO_2T2R ||
		priv->pmib->dot11RFEntry.MIMO_TR_mode == MIMO_2T4R)
		return MIMO_2T2R;
	else if (priv->pshare->phw->MIMO_TR_hw_support == MIMO_3T3R ||
		priv->pmib->dot11RFEntry.MIMO_TR_mode == MIMO_3T3R)
		return MIMO_3T3R;
	else if (priv->pshare->phw->MIMO_TR_hw_support == MIMO_4T4R)
		return MIMO_4T4R;

	return MIMO_2T2R; //2ss as default

}

static __inline__ unsigned int get_supported_mcs(struct rtl8192cd_priv *priv)
{
	if (get_rf_mimo_mode(priv) == MIMO_1T1R)
		return (priv->pmib->dot11nConfigEntry.dot11nSupportedMCS & 0x00ff);
	else if(get_rf_mimo_mode(priv) == MIMO_2T2R)
		return (priv->pmib->dot11nConfigEntry.dot11nSupportedMCS & 0xffff);
	else if(get_rf_mimo_mode(priv) == MIMO_3T3R)
		return (priv->pmib->dot11nConfigEntry.dot11nSupportedMCS & 0xffffff);

	return (priv->pmib->dot11nConfigEntry.dot11nSupportedMCS & 0xffff); //2ss as default
}

static __inline__ void tx_sum_up(struct rtl8192cd_priv *priv, struct stat_info *pstat, struct tx_insn* txcfg)
{
	unsigned int pktlen = txcfg->fr_len+txcfg->hdr_len+txcfg->iv+txcfg->llc+txcfg->mic+txcfg->icv;

	if (priv) {
#if defined(CONFIG_RTL8672) || defined(CONFIG_WLAN_STATS_EXTENTION)
		extern int IS_BCAST2(unsigned char *da);
		extern int IS_MCAST(unsigned char *da);
		extern unsigned char *get_da(unsigned char *pframe);
		unsigned char *da;
#ifdef SUPPORT_TX_AMSDU
		if ((txcfg->aggre_en == FG_AGGRE_MSDU_MIDDLE) || (txcfg->aggre_en == FG_AGGRE_MSDU_LAST))
			da = NULL;
		else
#endif
			da = get_da(txcfg->phdr);
		if (da != NULL)
		{
			if (IS_BCAST2(da))
				priv->ext_stats.tx_bcast_pkts_cnt++;
			else if (IS_MCAST(da))
				priv->ext_stats.tx_mcast_pkts_cnt++;
			else
				priv->ext_stats.tx_ucast_pkts_cnt++;
		}
#endif

#ifdef TRX_DATA_LOG
		if (txcfg->fr_type == _SKB_FRAME_TYPE_)
			priv->ext_stats.tx_data_packets++;
#endif
		priv->ext_stats.tx_byte_cnt += pktlen;

		// bcm old 11n chipset iot debug, and TXOP enlarge
		priv->pshare->current_tx_bytes += pktlen;

		if (pstat)
			priv->pshare->NumTxBytesUnicast += pktlen;
	}

	if (pstat) {

#if defined(TXREPORT)
        if(pstat->sta_in_firmware != 1)
#endif //TXREPORT
		{
#ifdef DONT_COUNT_PROBE_PACKET
			if (pstat->tx_probe_rsp_pkts) {
				// Don't increase pstat->tx_pkts for probe response
				pstat->tx_probe_rsp_pkts--;
			} else
#endif
			pstat->tx_pkts++;
		}
		pstat->tx_bytes += pktlen;
		pstat->tx_byte_cnt += pktlen;
	}
}

/*
 * rx_sum_up might called after the skb was freed, don't use get_pframe to retrieve any information in skb.
 */
static __inline__ void rx_sum_up(struct rtl8192cd_priv *priv, struct stat_info *pstat, struct rx_frinfo *pfrinfo)
{
	if (priv) {
#ifdef RX_CRC_EXPTIMER
        priv->ext_stats.rx_packets_exptimer++;
		priv->ext_stats.rx_packets_by_rate[pfrinfo->rx_rate]++;
#endif

#ifdef TRX_DATA_LOG
		if (pfrinfo->bdata && !priv->pmib->miscEntry.func_off)
			priv->ext_stats.rx_data_packets++;
#endif

		if (pfrinfo->retry)
			priv->ext_stats.rx_retrys++;
#if defined(CONFIG_RTL8672) || defined(CONFIG_WLAN_STATS_EXTENTION)
		if (pfrinfo->bcast)
			priv->ext_stats.rx_bcast_pkts_cnt++;
		else if (pfrinfo->mcast)
			priv->ext_stats.rx_mcast_pkts_cnt++;
		else
			priv->ext_stats.rx_ucast_pkts_cnt++;
#endif
		priv->ext_stats.rx_byte_cnt += pfrinfo->pktlen;

		// bcm old 11n chipset iot debug
		priv->pshare->current_rx_bytes += pfrinfo->pktlen;

#if (CONFIG_ADAPTIVE_SOML)
		if (IS_OUTSRC_CHIP(priv)) {
		PDM_ODM_T				p_dm_odm = &(priv->pshare->_dmODM);
		struct _ADAPTIVE_SOML_	*p_dm_soml_table = &(p_dm_odm->dm_soml_table);

			if(p_dm_odm->SupportAbility & ODM_BB_ADAPTIVE_SOML) {
		if (pfrinfo->rx_rate >= HT_RATE_ID && (pfrinfo->rx_rate <= (_MCS31_RATE_)))
			p_dm_soml_table->num_ht_bytes[pfrinfo->rx_rate-HT_RATE_ID] += pfrinfo->pktlen;
#ifdef RTK_AC_SUPPORT
		if (pfrinfo->rx_rate >= VHT_RATE_ID && (pfrinfo->rx_rate <= (_NSS4_MCS9_RATE_)))
			p_dm_soml_table->num_vht_bytes[pfrinfo->rx_rate-VHT_RATE_ID] += pfrinfo->pktlen;
#endif
			}
		}
#endif

	}

	if (pstat) {
		pstat->rx_pkts++;
		pstat->rx_bytes += pfrinfo->pktlen;
		pstat->rx_byte_cnt += pfrinfo->pktlen;
	}
}

static __inline__ void netif_tx_sum_up(struct rtl8192cd_priv *priv, int pktlen)
{
	struct net_device_stats *pnet_stats;
	pnet_stats = &(priv->net_stats);
	pnet_stats->tx_packets++;
	pnet_stats->tx_bytes += pktlen;
}

static __inline__ void netif_rx_sum_up(struct rtl8192cd_priv *priv, int pktlen)
{
	struct net_device_stats *pnet_stats;
	pnet_stats = &(priv->net_stats);
	pnet_stats->rx_packets++;
	pnet_stats->rx_bytes += pktlen;
}


static __inline__ unsigned char get_cck_swing_idx(unsigned int bandwidth, unsigned char ofdm_swing_idx)
{
	unsigned char cck_swing_idx;

	if (bandwidth == HT_CHANNEL_WIDTH_20) {
		if (ofdm_swing_idx >= TxPwrTrk_CCK_SwingTbl_Len)
			cck_swing_idx = TxPwrTrk_CCK_SwingTbl_Len - 1;
		else
			cck_swing_idx = ofdm_swing_idx;
	}
	else {	// 40M bw
		if (ofdm_swing_idx < 12)
			cck_swing_idx = 0;
		else if (ofdm_swing_idx > (TxPwrTrk_CCK_SwingTbl_Len - 1 + 12))
			cck_swing_idx = TxPwrTrk_CCK_SwingTbl_Len - 1;
		else
			cck_swing_idx = ofdm_swing_idx - 12;
	}

	return cck_swing_idx;
}

//Apollo slave cpu do not use watch dog

#if (defined(__ECOS)&&defined(CONFIG_RTL_819X)) || defined(CONFIG_RTL865X_WTDOG) || defined(CONFIG_RTL_WTDOG)

#ifndef BSP_WDTCNR
	#define BSP_WDTCNR 0xB800311C
#endif

#define _WDTCNR_			BSP_WDTCNR

#define _WDTKICK_			(1 << 23)
#define _WDTSTOP_			(0xA5f00000)

static __inline__ void watchdog_stop(struct rtl8192cd_priv *priv)
{
	*((volatile unsigned long *)_WDTCNR_) |= _WDTKICK_;
	priv->pshare->wtval = *((volatile unsigned long *)_WDTCNR_);
	*((volatile unsigned long *)_WDTCNR_) = _WDTSTOP_;
}

static __inline__ void watchdog_resume(struct rtl8192cd_priv *priv)
{
	*((volatile unsigned long *)_WDTCNR_) = priv->pshare->wtval;
	*((volatile unsigned long *)_WDTCNR_) |= _WDTKICK_;
}


static __inline__ void watchdog_kick(void)
{
	*((volatile unsigned long *)_WDTCNR_) |= _WDTKICK_;
}

#else // no watchdog support

static __inline__ void watchdog_stop(struct rtl8192cd_priv *priv)
{}

static __inline__ void watchdog_resume(struct rtl8192cd_priv *priv)
{}

static __inline__ void watchdog_kick(void)
{}

#endif

#ifndef _WDTCNR_
	#define	_WDTCNR_ 0xB800311C
#endif

static __inline__ void watchdog_reboot(void)
{
		local_irq_disable();
	*((volatile unsigned long *)_WDTCNR_) = 0;
	while (1);
}


#define CIRC_CNT_RTK(head,tail,size)	((head>=tail)?(head-tail):(size-tail+head))
#define CIRC_SPACE_RTK(head,tail,size)	CIRC_CNT_RTK((tail),((head)+1),(size))

#ifdef CONFIG_USE_VMALLOC
#define rtw_vmalloc(sz)			_rtw_vmalloc((sz))
#define rtw_zvmalloc(sz)			_rtw_zvmalloc((sz))
#define rtw_vmalloc_flag(sz,gfp)	_rtw_vmalloc((sz))
#define rtw_vmfree(pbuf, sz)		_rtw_vmfree((pbuf), (sz))
#else // !CONFIG_USE_VMALLOC
#define rtw_vmalloc(sz)			_rtw_malloc((sz))
#define rtw_vmalloc_flag(sz,gfp)	_rtw_malloc_flag((sz), (gfp))
#define rtw_zvmalloc(sz)			_rtw_zmalloc((sz))
#define rtw_vmfree(pbuf, sz)		_rtw_mfree((pbuf), (sz))
#endif // CONFIG_USE_VMALLOC
#define rtw_malloc(sz)			_rtw_malloc((sz))
#define rtw_zmalloc(sz)			_rtw_zmalloc((sz))
#define rtw_mfree(pbuf, sz)		_rtw_mfree((pbuf), (sz))

u8* _rtw_vmalloc(u32 sz);
u8* _rtw_zvmalloc(u32 sz);
void _rtw_vmfree(const void *pbuf, u32 sz);
u8* _rtw_zmalloc(u32 sz);
u8* _rtw_malloc_flag(u32 sz, gfp_t gfp);
u8* _rtw_malloc(u32 sz);
void _rtw_mfree(const void *pbuf, u32 sz);

void* rtw_malloc2d(int h, int w, int size);
void rtw_mfree2d(void *pbuf, int h, int w, int size);

#if defined(USE_PID_NOTIFY) && defined(LINUX_2_6_27_)
extern struct pid *_wlanapp_pid;
extern struct pid *_wlanwapi_pid;
#endif


#ifdef USE_TXQUEUE
int init_txq_pool(struct list_head *head, unsigned char **ppool);
void free_txq_pool(struct list_head *head, unsigned char *ppool);
void append_skb_to_txq_head(struct txq_list_head *head, struct rtl8192cd_priv *priv, struct sk_buff *skb, struct net_device *dev, struct list_head *pool);
void append_skb_to_txq_tail(struct txq_list_head *head, struct rtl8192cd_priv *priv, struct sk_buff *skb, struct net_device *dev, struct list_head *pool);
void remove_skb_from_txq(struct txq_list_head *head, struct sk_buff **pskb, struct net_device **pdev, struct list_head *pool);
#endif

static inline long timeval_to_us(const struct timeval *tv)
{
	return (tv->tv_sec*1000000L)+ tv->tv_usec;
}

// Get the N-bytes aligment offset from the current length
#define N_BYTE_ALIGMENT(__Value, __Aligment) ((__Aligment == 1) ? (__Value) : (((__Value + __Aligment - 1) / __Aligment) * __Aligment))



#if defined(RTK_ATM) && !defined(HS2_SUPPORT)
void staip_snooping_byarp(struct sk_buff *pskb, struct stat_info *pstat);
#endif
#ifdef HS2_SUPPORT
unsigned int getDSCP2UP(struct rtl8192cd_priv *priv, unsigned char DSCP); //after pf4
void setQoSMapConf(struct rtl8192cd_priv *priv);//after pf4
void staip_snooping_byarp(struct sk_buff *pskb, struct stat_info *pstat);
void staip_snooping_bydhcp(struct sk_buff *pskb, struct rtl8192cd_priv *priv);
void stav6ip_snooping_bynadvert(struct sk_buff *pskb, struct stat_info *pstat);
void stav6ip_snooping_bynsolic(struct rtl8192cd_priv *priv, struct sk_buff *pskb, struct stat_info *pstat);// modify after pf4
int proxy_arp_handle(struct rtl8192cd_priv *priv, struct sk_buff *skb);
int proxy_icmpv6_ndisc(struct rtl8192cd_priv *priv, struct sk_buff *skb);
void calcu_sta_v6ip(struct stat_info *pstat);
int check_nei_advt(struct rtl8192cd_priv *priv, struct sk_buff *skb);
#endif
#ifdef SUPPORT_MONITOR
void start_monitor_mode(struct rtl8192cd_priv * priv);
void stop_monitor_mode(struct rtl8192cd_priv * priv);
#endif

__MIPS16
__IRAM_IN_865X int __rtl8192cd_start_xmit(struct sk_buff *skb, struct net_device *dev, int tx_flag);


static inline int hash_list_add(struct rtl8192cd_priv *priv, struct stat_info *pstat)
{
	unsigned int index;
	struct list_head *plist;
	int ret = 0;

	index = wifi_mac_hash(pstat->hwaddr);
	plist = priv->stat_hash;
	plist += index;

	if (list_empty(&pstat->hash_list)) {
		list_add_tail(&pstat->hash_list, plist);
		ret = 1;
	}

	return ret;
}

static inline int hash_list_del(struct rtl8192cd_priv *priv, struct stat_info *pstat)
{
	int ret = 0;

	if (!list_empty(&pstat->hash_list)) {
		list_del_init(&pstat->hash_list);
		ret = 1;
	}

	return ret;
}
static inline struct list_head* asoc_list_get_next(struct rtl8192cd_priv *priv, struct list_head *plist)
{
	return plist->next;
}

static inline void asoc_list_unref(struct rtl8192cd_priv *priv, struct stat_info *pstat)
{

}

static inline int asoc_list_del(struct rtl8192cd_priv *priv, struct stat_info *pstat)
{
	int ret = 0;

	SMP_LOCK_ASOC_LIST(flags);

	if (!list_empty(&pstat->asoc_list)) {
		list_del_init(&pstat->asoc_list);
		ret = 1;
	}

	SMP_UNLOCK_ASOC_LIST(flags);

	return ret;
}

static inline int asoc_list_add(struct rtl8192cd_priv *priv, struct stat_info *pstat)
{
	int ret = 0;

	SMP_LOCK_ASOC_LIST(flags);

	if (list_empty(&pstat->asoc_list)) {
		list_add_tail(&pstat->asoc_list, &priv->asoc_list);
		ret = 1;
	}

	SMP_UNLOCK_ASOC_LIST(flags);

	return ret;
}

static inline int auth_list_del(struct rtl8192cd_priv *priv, struct stat_info *pstat)
{
		int ret = 0;


		if (!list_empty(&pstat->auth_list)) {
			list_del_init(&pstat->auth_list);
			ret = 1;
		}

		return ret;
}

static inline int auth_list_add(struct rtl8192cd_priv *priv, struct stat_info *pstat)
{
		int ret = 0;

		if (list_empty(&pstat->auth_list)) {
			list_add_tail(&pstat->auth_list, &priv->auth_list);
			ret = 1;
		}


		return ret;

}

static inline int sleep_list_del(struct rtl8192cd_priv *priv, struct stat_info *pstat)
{
		int ret = 0;

		SMP_LOCK_SLEEP_LIST(flags);

		if (!list_empty(&pstat->sleep_list)) {
			list_del_init(&pstat->sleep_list);
			ret = 1;
		}

		SMP_UNLOCK_SLEEP_LIST(flags);

		return ret;

}

static inline int sleep_list_add(struct rtl8192cd_priv *priv, struct stat_info *pstat)
{
		int ret = 0;

		SMP_LOCK_SLEEP_LIST(flags);

		if (list_empty(&pstat->sleep_list)) {
			list_add_tail(&pstat->sleep_list, &priv->sleep_list);
			ret = 1;
		}

		SMP_UNLOCK_SLEEP_LIST(flags);

		return ret;

}

static inline int wakeup_list_del(struct rtl8192cd_priv *priv, struct stat_info *pstat)
{
		int ret = 0;


		if (!list_empty(&pstat->wakeup_list)) {
			list_del_init(&pstat->wakeup_list);
			ret = 1;
		}


		return ret;

}

static inline int wakeup_list_add(struct rtl8192cd_priv *priv, struct stat_info *pstat)
{
		int ret = 0;


		if (list_empty(&pstat->wakeup_list)) {
			list_add_tail(&pstat->wakeup_list, &priv->wakeup_list);
			ret = 1;
		}


		return ret;

}




extern struct stat_info *get_stainfo_hash(struct rtl8192cd_priv *priv, unsigned char *hwaddr);

static inline struct stat_info *get_stainfo(struct rtl8192cd_priv *priv, unsigned char *hwaddr)
{
	struct stat_info	*pstat;

	if(hwaddr == NULL)
		return (struct stat_info *)NULL;

	//if (!memcmp(hwaddr, priv->stainfo_cache.hwaddr, MACADDRLEN) &&  priv->stainfo_cache.pstat)
	pstat = priv->pstat_cache;

	{
	    if(pstat && isEqualMACAddr(hwaddr, pstat->hwaddr))
			return pstat;
	}

	return get_stainfo_hash(priv, hwaddr);
}


#if !defined(CONFIG_WLAN_STATS_EXTENTION)
static inline int IS_BSSID(struct rtl8192cd_priv *priv, unsigned char *da)
{
	unsigned char *bssid;
	bssid = priv->pmib->dot11StationConfigEntry.dot11Bssid;

	//if (!memcmp(da, bssid, 6))
	if (isEqualMACAddr(da, bssid))
		return TRUE;
	else
		return FALSE;
}

static inline int IS_MCAST(unsigned char *da)
{
	if ((*da) & 0x01)
		return TRUE;
	else
		return FALSE;
}

static inline int IS_BCAST2(unsigned char *da)
{
     if ((*da) == 0xff)
         return TRUE;
     else
         return FALSE;
}
#endif

#define SET_PSEUDO_RANDOM_NUMBER(target)	{ \
	get_random_bytes(&(target), sizeof(target)); \
	target += (GET_MY_HWADDR[4] + GET_MY_HWADDR[5] + jiffies - priv->net_stats.rx_bytes \
	+ priv->net_stats.tx_bytes + priv->net_stats.rx_errors - priv->ext_stats.beacon_ok); \
}


#define PRINT_ONE(val, format, line_end) { 		\
	seq_printf(s, format, val); \
	if (line_end) \
		seq_printf(s, "\n");		\
}

#define PRINT_ARRAY(val, format, len, line_end) { 	\
	int index;					\
	for (index=0; index<len; index++)		\
		seq_printf(s, format, val[index]); \
	if (line_end)					\
		seq_printf(s, "\n");		\
							\
}

#define PRINT_SINGL_ARG(name, para, format) { \
	PRINT_ONE(name, "%s", 0); \
	PRINT_ONE(para, format, 1); \
}

#define PRINT_ARRAY_ARG(name, para, format, len) { \
	PRINT_ONE(name, "%s", 0); \
	PRINT_ARRAY(para, format, len, 1); \
}

#if (NUM_STAT >64)
#define PRINT_BITMAP_ARG(name, para) { \
    PRINT_ONE(name, "%s: ", 0); \
    PRINT_ONE(para._staMap_, "0x%08x", 1); \
    PRINT_ONE(name, "%s ext 1: ", 0); \
    PRINT_ONE(para._staMap_ext_1, "0x%08x", 1); \
    PRINT_ONE(name, "%s ext 2: ", 0); \
    PRINT_ONE(para._staMap_ext_2, "0x%08x", 1); \
    PRINT_ONE(name, "%s ext 3: ", 0); \
    PRINT_ONE(para._staMap_ext_3, "0x%08x", 1); \
}

#elif (NUM_STAT >32)
#define PRINT_BITMAP_ARG(name, para) { \
    PRINT_ONE(name, "%s: ", 0); \
    PRINT_ONE(para._staMap_, "0x%08x", 1); \
    PRINT_ONE(name, "%s ext 1: ", 0); \
    PRINT_ONE(para._staMap_ext_1, "0x%08x", 1); \
}
#else
#define PRINT_BITMAP_ARG(name, para) { \
    PRINT_ONE(name, "%s: ", 0); \
    PRINT_ONE(para._staMap_, "0x%08x", 1); \
}
#endif

#define CHECK_LEN do {} while(0)
#define CHECK_LEN_E do {} while(0)


#endif // _8192CD_UTIL_H_

