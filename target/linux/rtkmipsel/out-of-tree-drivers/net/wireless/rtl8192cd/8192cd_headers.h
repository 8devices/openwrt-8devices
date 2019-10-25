/*
 *  Headler file defines global functions PROTO types
 *
 *  $Id: 8192cd_headers.h,v 1.32.2.22 2011/01/11 13:48:37 button Exp $
 *
 *  Copyright (c) 2009 Realtek Semiconductor Corp.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 */

/*-----------------------------------------------------------------------------
				This file define the header files
------------------------------------------------------------------------------*/

#ifndef _8192CD_HEADERS_H_
#define _8192CD_HEADERS_H_

#ifdef __KERNEL__
#include <linux/wireless.h>
#endif

#ifdef __ECOS
#include <pkgconf/system.h>
#include <pkgconf/devs_eth_rltk_819x_wrapper.h>
#include <sys/socket.h>
#include <cyg/io/eth/rltk/819x/wrapper/wireless.h>
#endif

#include "./8192cd_tx.h"

#if defined(CONFIG_RTL_88E_SUPPORT) || defined(CONFIG_RTL_8812_SUPPORT ) || defined(CONFIG_RTL_8723B_SUPPORT)
#include "HalPwrSeqCmd.h"
#endif

#ifdef CONFIG_WLAN_HAL_8192EE
#if defined(CONFIG_PCI_HCI)
#include "./WlanHAL/RTL88XX/RTL8192E/RTL8192EE/Hal8192EEDef.h"
#elif defined(CONFIG_SDIO_HCI)
#include "./WlanHAL/RTL88XX/RTL8192E/RTL8192ES/Hal8192ESDef.h"
#endif
#endif

#ifdef CONFIG_WLAN_HAL_8814AE
#include "./WlanHAL/RTL88XX/RTL8814A/RTL8814AE/Hal8814AEDef.h"
#endif

#ifdef CONFIG_WLAN_HAL_8881A
#include "./WlanHAL/RTL88XX/RTL8881A/RTL8881AE/Hal8881AEDef.h"
#endif

#ifdef CONFIG_WLAN_HAL_8197F
#include "./WlanHAL/RTL88XX/RTL8197F/RTL8197FE/Hal8197FEDef.h"
#endif

#ifdef CONFIG_WLAN_HAL_8822BE
#include "./WlanHAL/RTL88XX/RTL8822B/RTL8822BE/Hal8822BEDef.h"
#endif

#if (BEAMFORMING_SUPPORT == 1)
#include "Beamforming.h"
#endif

#ifdef __KERNEL__
#ifdef SMP_SYNC
#include <linux/interrupt.h>
#endif
#endif

#ifdef CONFIG_IEEE80211V
#include "./8192cd_11v.h"
#endif
//#include "./8192cd_p2p.h"


#ifdef P2P_DEBUGMSG
#define P2P_DEBUG(fmt, args...)     (panic_printk("[%s %d][%s]:"fmt,__FUNCTION__ , __LINE__ ,priv->dev->name, ## args))
#define P2P_DEBUG2(fmt, args...)	(panic_printk("[%s %d]:"fmt,__FUNCTION__ , __LINE__ , ## args))
#define P2P_TAG_PRINT(fmt, args...)     (panic_printk("Tag:"fmt, ## args))
#define P2P_PRINT(fmt, args...) ( panic_printk(" "fmt,## args))
#define MAC_PRINT(fmt) ( panic_printk("%02X%02X%02X:%02X%02X%02X\n",fmt[0],fmt[1],fmt[2],fmt[3],fmt[4],fmt[5]))
#define WSC_DEBUG(fmt, args...) (panic_printk("[%s %d]"fmt,__FUNCTION__,__LINE__,## args))
#define PDEBUG(fmt, args...)    (panic_printk(""fmt, ## args))
#else
#define P2P_TAG_PRINT(fmt, args...) {}
#define P2P_DEBUG(fmt, args...)  {}
#define P2P_DEBUG2(fmt, args...) {}
#define P2P_PRINT(fmt, args...) {}
#define MAC_PRINT(fmt)          {}
#ifdef PERIODIC_AUTO_CHANNEL_DEBUG
#define PDEBUG(fmt,args...)		panic_printk("%s[%s %d auto_channel]"fmt,priv->dev->name,__FUNCTION__,__LINE__, ## args)
#else
#define PDEBUG(fmt, args...)    {}
#endif
#endif


#ifdef ROMEPERF_SUPPORT
#include "../../rtl819x/m24kctrl.h"
#define STARTROMEPERF(priv, index) if(priv->pshare->rf_ft_var.perf_index==index) startCP3Ctrl(0)
#define STOPROMEPERF(priv, record, index) if(priv->pshare->rf_ft_var.perf_index==index) stopCP3Ctrl(record, index)
#else
#define STARTROMEPERF(priv, index)
#define STOPROMEPERF(priv, record, index)
#endif

#ifdef TXSC_CMPTXBD
#define COMPAREAVAILABLETXBD(ptx_dma,halQnum,needTxBD) (((ptx_dma->tx_queue[halQnum].avail_txbd_num < needTxBD)))
#else
#define COMPAREAVAILABLETXBD(ptx_dma,halQnum,needTxBD) (compareAvailableTXBD(priv, needTxBD, halQnum, 2))
#endif

#ifdef HS2_DEBUGMSG
extern unsigned int _HS2_debug_info;
extern unsigned int _HS2_debug_err;		/* err flag */
extern unsigned int _HS2_debug_trace;	/* trace flag */
extern unsigned int _HS2_debug_warn;	/* warn flag */

#define HS2_DEBUG_INFO(fmt, args...) if(_HS2_debug_info) panic_printk("[%s %d]"fmt,__FUNCTION__,__LINE__,## args)
#define HS2_DEBUG_ERR(fmt, args...) if(_HS2_debug_err) panic_printk("[%s %d]Err: "fmt,__FUNCTION__,__LINE__,## args)
#define HS2_DEBUG_TRACE(level, fmt, args...) if(_HS2_debug_trace >= level) panic_printk("[%s %d]"fmt,__FUNCTION__,__LINE__,## args)
#define HS2_DEBUG_WARN(fmt, args...) if(_HS2_debug_warn) panic_printk("[%s %d]"fmt,__FUNCTION__,__LINE__,## args)
#define HS2DEBUG(fmt, args...) panic_printk("[%s %d]"fmt,__FUNCTION__,__LINE__,## args)
#define _HS2DEBUG(fmt, args...) panic_printk(""fmt,## args)
#else
#define HS2_DEBUG_INFO(fmt, args...)
#define HS2_DEBUG_ERR(fmt, args...)
#define HS2_DEBUG_TRACE(level, fmt, args...)
#define HS2_DEBUG_INFO(fmt, args...)
#define HS2DEBUG(fmt, args...) {}
#define _HS2DEBUG(fmt, args...) {}

#endif


#ifdef PMF_DEBUGMSG
#define PMFDEBUG(fmt, args...) panic_printk("[%s %d]"fmt,__FUNCTION__,__LINE__,## args)
#else
#define PMFDEBUG(fmt, args...) {}
#endif
#define EDEBUG(fmt, args...) {}
//#define SDEBUG(fmt, args...) panic_printk("[%s %d]"fmt,__FUNCTION__,__LINE__,## args)
#define SDEBUG(fmt, args...) { }
//#define STADEBUG(fmt, args...) panic_printk("[%d %s ][%s]"fmt,__LINE__, __FUNCTION__,priv->dev->name,## args)
#define STADEBUG(fmt, args...) { }
#define PSKDEBUG(fmt, args...) { }
//#define PSKDEBUG(fmt, args...) panic_printk("[%d %s ][%s]"fmt,__LINE__, __FUNCTION__,priv->dev->name,## args)

#define NDEBUG(fmt, args...) { }
//#define NDEBUG(fmt, args...) printk("[%s %d][%s]"fmt,__FUNCTION__,__LINE__,priv->dev->name,## args)
#define _NDEBUG(fmt, args...) { }
//#define _NDEBUG(fmt, args...) printk("[%s][%s]"fmt,__FUNCTION__,priv->dev->name,## args)

#define GDEBUG(fmt, args...) panic_printk("[%s %d]"fmt,__FUNCTION__,__LINE__,## args)
#define HALDEBUG(fmt, args...) panic_printk("[%s %d][%s]"fmt,__FUNCTION__,__LINE__,Adapter->dev->name,## args)


//#define NDEBUG2(fmt, args...) printk("[%s %d]"fmt,__FUNCTION__,__LINE__,## args)
#define NDEBUG2(fmt, args...) {}
//#define NDEBUG3(fmt, args...) printk("[%s %d]"fmt,__FUNCTION__,__LINE__,## args)
#define NDEBUG3(fmt, args...) {}

//#define ACSDEBUG(fmt, args...) panic_printk("[%s:%d] "fmt,__FUNCTION__,__LINE__,## args)
#define ACSDEBUG(fmt, args...) {}

#if defined(__ECOS)
#define SPRINTT(buf, fmt, args...) diag_printf(fmt, ##args)
#if defined(_DEBUG_RTL8192CD_) || defined(DEBUG_PSK)
#define printk diag_printf
#endif
#else
#define SPRINTT(buf, fmt, args...) panic_printk(fmt, ##args)
#endif

#if defined(THER_TRIM) || defined(POWER_TRIM)
#define phy_printk(fmt, args...) \
	panic_printk("[%s][%s] "fmt, priv->dev->name, __FUNCTION__, ## args);
#endif

/*-----------------------------------------------------------------------------
								8192cd_util.c
------------------------------------------------------------------------------*/
#ifndef	_8192CD_UTILS_C_
#define EXTERN	extern
#else
#define EXTERN
#endif

EXTERN unsigned long rtw_get_current_time(void);
EXTERN unsigned long rtw_get_passing_time_ms(unsigned long start_time);
#ifdef SW_TX_QUEUE
EXTERN void turbo_swq_setting(struct rtl8192cd_priv *priv);
EXTERN void adjust_swq_setting(struct rtl8192cd_priv *priv, struct stat_info *pstat, int i, int mode);
EXTERN void init_STA_SWQAggNum(struct rtl8192cd_priv *priv);
#endif


/*cfg p2p cfg p2p*/
EXTERN int rtk_p2p_is_enabled(struct rtl8192cd_priv *priv);
EXTERN u8* rtk_get_ie_with_oui(u8 *in_ie, int in_len, u8 ie_to_search, u8* oui_input ,int oui_len, int *report_ielen);
EXTERN int rtk_get_wfd_ie(u8 *in_ie, int in_len, u8 *wfd_ie, int *wfd_ielen);
EXTERN u8* rtk_get_p2p_ie(u8 *in_ie, int in_len, u8 *p2p_ie, int *p2p_ielen);
EXTERN u8* rtk_get_wps_ie(u8 *in_ie, int in_len, u8 *wps_ie, int *wps_ielen);
EXTERN void copy_wps_ie(struct rtl8192cd_priv *priv, unsigned char *wps_ie, unsigned char mgmt_type);
EXTERN void copy_p2p_ie(struct rtl8192cd_priv *priv, unsigned char *p2p_ie_input, unsigned char mgmt_type);
EXTERN void convert_bin_to_str(unsigned char *bin, int len, char *out);
/*cfg p2p cfg p2p*/
#ifdef TXREPORT
#ifdef CONFIG_WLAN_HAL
EXTERN void APReqTXRptHandler(struct rtl8192cd_priv *priv,unsigned char* C2HContent);
#endif
EXTERN void txrpt_handler(struct rtl8192cd_priv *priv, struct tx_rpt *report);
#endif

#ifdef TXRETRY_CNT
EXTERN int is_support_TxRetryCnt(struct rtl8192cd_priv *priv);
EXTERN void txretry_handler(struct rtl8192cd_priv *priv, struct tx_retry *retry);
EXTERN void C2HTxTxRetryHandler(struct rtl8192cd_priv *priv, unsigned char *CmdBuf);
#endif

EXTERN int is_passive_channel(struct rtl8192cd_priv *priv , int domain, int chan);//20131218 add
EXTERN int is_DFS_channel(int channelVal);


EXTERN void mem_dump(unsigned char *ptitle, unsigned char *pbuf, int len);
EXTERN int enque(struct rtl8192cd_priv *priv, int *head, int *tail, unsigned long ffptr, int ffsize, void *elm);
EXTERN void* deque(struct rtl8192cd_priv *priv, int *head, int *tail, unsigned long ffptr, int ffsize);
EXTERN void initque(struct rtl8192cd_priv *priv, int *head, int *tail);
EXTERN int isFFempty(int head, int tail);
EXTERN unsigned int find_rate(struct rtl8192cd_priv *priv, struct stat_info *pstat, int mode, int isBasicRate);
EXTERN unsigned int find_rate_MP(struct rtl8192cd_priv *priv, struct stat_info *pstat, struct ht_cap_elmt * peer_ht_cap, int peer_ht_cap_len, char * peer_rate, int peer_rate_len, int mode, int isBasicRate);
EXTERN void init_stainfo(struct rtl8192cd_priv *priv, struct stat_info *pstat);
#ifdef THERMAL_CONTROL
#ifdef TX_SHORTCUT
EXTERN void clear_short_cut_cache(struct rtl8192cd_priv *priv, struct stat_info *pstat);
#endif
#endif
EXTERN void release_stainfo(struct rtl8192cd_priv *priv, struct stat_info *pstat);
EXTERN struct stat_info *alloc_stainfo(struct rtl8192cd_priv *priv, unsigned char *hwaddr, int id);
EXTERN void free_sta_tx_skb(struct rtl8192cd_priv *priv, struct stat_info *pstat);
EXTERN void free_sta_skb(struct rtl8192cd_priv *priv, struct stat_info *pstat);
EXTERN int del_station(struct rtl8192cd_priv *priv, struct stat_info *pstat, int send_disasoc);
EXTERN int free_stainfo(struct rtl8192cd_priv *priv, struct	stat_info *pstat);
EXTERN 
#ifdef __OSK__
__IRAM_WIFI_PRI1
#else
__MIPS16
__IRAM_IN_865X
#endif
struct stat_info *get_stainfo (struct rtl8192cd_priv *priv, unsigned char *hwaddr);
#ifdef HW_FILL_MACID
EXTERN 
#ifdef __OSK__
__IRAM_WIFI_PRI1
#else
__MIPS16
__IRAM_IN_865X
#endif
struct stat_info *get_stainfo_fast(struct rtl8192cd_priv *priv, unsigned char *hwaddr, unsigned char macID);
EXTERN struct stat_info *get_HW_mapping_sta(struct rtl8192cd_priv *priv, unsigned char macID);  
#endif
EXTERN struct stat_info *get_aidinfo (struct rtl8192cd_priv *priv, unsigned int aid);
EXTERN int IS_BSSID(struct rtl8192cd_priv *priv, unsigned char *da);
EXTERN int IS_BCAST2(unsigned char *da);
EXTERN int IS_MCAST(unsigned char *da);
EXTERN int p80211_stt_findproto(UINT16 proto);
EXTERN int skb_ether_to_p80211(struct sk_buff *skb, UINT32 ethconv);
EXTERN int skb_p80211_to_ether(struct net_device *dev, int wep_mode, struct rx_frinfo *pfrinfo);
EXTERN int strip_amsdu_llc(struct rtl8192cd_priv *priv, struct sk_buff *skb, struct stat_info *pstat);
EXTERN unsigned char *get_da(unsigned char *pframe);
EXTERN unsigned char *get_bssid_mp(unsigned char *pframe);
EXTERN 
#if (!defined(__OSK__)) || (defined(__OSK__) && !defined(CONFIG_RTL6028))
__MIPS16
#endif
__IRAM_IN_865X
unsigned char get_hdrlen(struct rtl8192cd_priv *priv, UINT8 *pframe);
EXTERN unsigned int get_mcast_privacy(struct rtl8192cd_priv *priv, unsigned int *iv, unsigned int *icv,
				unsigned int *mic);
EXTERN unsigned int	get_privacy(struct rtl8192cd_priv *priv, struct stat_info *pstat,
				unsigned int *iv, unsigned int *icv, unsigned int *mic);
EXTERN unsigned char *get_mgtbuf_from_poll(struct rtl8192cd_priv *priv);
EXTERN void release_mgtbuf_to_poll(struct rtl8192cd_priv *priv, unsigned char *pbuf);
EXTERN unsigned char *get_wlanhdr_from_poll(struct rtl8192cd_priv *priv);
EXTERN void release_wlanhdr_to_poll(struct rtl8192cd_priv *priv, unsigned char *pbuf);
EXTERN __IRAM_IN_865X unsigned char *get_wlanllchdr_from_poll(struct rtl8192cd_priv *priv);
EXTERN 
#ifdef __OSK__
__IRAM_WIFI_PRI5
#endif
void release_wlanllchdr_to_poll(struct rtl8192cd_priv *priv, unsigned char *pbuf);
EXTERN unsigned char *get_icv_from_poll(struct rtl8192cd_priv *priv);
EXTERN void release_icv_to_poll(struct rtl8192cd_priv *priv, unsigned char *pbuf);
EXTERN unsigned char *get_mic_from_poll(struct rtl8192cd_priv *priv);
EXTERN void release_mic_to_poll(struct rtl8192cd_priv *priv, unsigned char *pbuf);
EXTERN unsigned short get_pnl(union PN48 *ptsc);
EXTERN unsigned int get_pnh(union PN48 *ptsc);
EXTERN void eth_2_llc(struct wlan_ethhdr_t *pethhdr, struct llc_snap *pllc_snap);
EXTERN void eth2_2_wlanhdr(struct rtl8192cd_priv *priv, struct wlan_ethhdr_t *pethhdr, struct tx_insn *txcfg);
EXTERN unsigned char *get_sa(unsigned char *pframe);
EXTERN unsigned int get_mcast_encrypt_algthm(struct rtl8192cd_priv *priv);
EXTERN unsigned int get_sta_encrypt_algthm(struct rtl8192cd_priv *priv, struct stat_info *pstat);
EXTERN UINT8 get_rate_from_bit_value(int bit_val);
EXTERN int get_rate_index_from_ieee_value(UINT8 val);
EXTERN int get_bit_value_from_ieee_value(UINT8 val);
EXTERN
#ifdef __OSK__
__IRAM_WIFI_PRI5
#endif
int UseSwCrypto(struct rtl8192cd_priv *priv, struct stat_info *pstat, int isMulticast
#ifdef CONFIG_IEEE80211W
		, int isPMF
#endif
	);

#ifdef WDS
EXTERN int getWdsIdxByDev(struct rtl8192cd_priv *priv, struct net_device *dev);
EXTERN struct net_device *getWdsDevByAddr(struct rtl8192cd_priv *priv, unsigned char *addr);
#endif

EXTERN void check_protection_shortslot(struct rtl8192cd_priv *priv);
EXTERN void check_sta_characteristic(struct rtl8192cd_priv *priv, struct stat_info *pstat, int act);
EXTERN int should_forbid_Nmode(struct rtl8192cd_priv *priv);
EXTERN int should_restrict_Nrate(struct rtl8192cd_priv *priv, struct stat_info *pstat);
EXTERN void validate_oper_rate(struct rtl8192cd_priv *priv);
EXTERN void get_oper_rate(struct rtl8192cd_priv *priv);
EXTERN int get_bssrate_set(struct rtl8192cd_priv *priv, int bssrate_ie, unsigned char **pbssrate, int *bssrate_len);
EXTERN int get_available_channel(struct rtl8192cd_priv *priv);
EXTERN int is80MChannel(unsigned int chnl_list[], unsigned int chnl_num,unsigned int channel); 
EXTERN int is40MChannel(unsigned int chnl_list[], unsigned int chnl_num,unsigned int channel);
EXTERN int find80MChannel(unsigned int chnl_list[], unsigned int chnl_num);
EXTERN int find40MChannel(unsigned int chnl_list[], unsigned int chnl_num);
EXTERN void cnt_assoc_num(struct rtl8192cd_priv *priv, struct stat_info *pstat, int act, char *func);
#ifdef INDICATE_LINK_CHANGE
EXTERN void indicate_sta_link_change(struct rtl8192cd_priv *priv, struct stat_info *pstat, int act, char *func);
#endif
EXTERN int get_assoc_sta_num(struct rtl8192cd_priv *priv, int mode);
EXTERN void event_indicate(struct rtl8192cd_priv *priv, unsigned char *mac, int event);
#ifdef WIFI_HAPD
EXTERN int event_indicate_hapd(struct rtl8192cd_priv *priv, unsigned char *mac, int event, unsigned char *extra);
#endif
#ifdef WIFI_WPAS
EXTERN int event_indicate_wpas(struct rtl8192cd_priv *priv, unsigned char *mac, int event, unsigned char *extra);
#endif
#ifdef USE_WEP_DEFAULT_KEY
EXTERN void init_DefaultKey_Enc(struct rtl8192cd_priv *priv, unsigned char *key, int algorithm);
#endif
EXTERN int changePreamble(struct rtl8192cd_priv *priv, int preamble);
EXTERN int HideAP(struct rtl8192cd_priv *priv);
EXTERN int DehideAP(struct rtl8192cd_priv *priv);

#if defined( UNIVERSAL_REPEATER) || defined(MBSSID)
EXTERN void disable_vxd_ap(struct rtl8192cd_priv *priv);
EXTERN void enable_vxd_ap(struct rtl8192cd_priv *priv);
EXTERN int under_apmode_repeater(struct rtl8192cd_priv *priv);
EXTERN int get_ss_level(struct rtl8192cd_priv *priv);
EXTERN int send_arp_response(struct rtl8192cd_priv *priv, unsigned int *dip, unsigned int *sip, unsigned char *dmac, unsigned char *smac);
EXTERN void snoop_STA_IP(struct sk_buff *pskb, struct rtl8192cd_priv *priv);
EXTERN int takeOverHidden(unsigned char active, struct rtl8192cd_priv *priv);
EXTERN int isHiddenAP(unsigned char *pframe, struct rx_frinfo *pfrinfo, struct stat_info *pstat, struct rtl8192cd_priv *priv);
#endif
EXTERN unsigned int isDHCPpkt(struct sk_buff *pskb);
EXTERN void syncMulticastCipher(struct rtl8192cd_priv *priv, struct bss_desc *bss_target);
#ifdef GBWC
EXTERN void rtl8192cd_GBWC_timer(unsigned long task_priv);
#endif
#ifdef SBWC
EXTERN void rtl8192cd_SBWC_timer(unsigned long task_priv);
#endif
#ifdef RTK_STA_BWC
EXTERN void rtl8192cd_sta_bwc_timer(unsigned long task_priv);
#endif

EXTERN void update_remapAid(struct rtl8192cd_priv *priv, struct stat_info *pstat);
EXTERN void release_remapAid(struct rtl8192cd_priv *priv, struct stat_info *pstat);
EXTERN unsigned char fw_was_full(struct rtl8192cd_priv *priv);
EXTERN int realloc_RATid(struct rtl8192cd_priv * priv);

// to avoid add RAtid fail
EXTERN unsigned int is_h2c_buf_occupy(struct rtl8192cd_priv *priv);
EXTERN short signin_h2c_cmd(struct rtl8192cd_priv *priv, unsigned int content, unsigned short ext_content);
#ifdef CONFIG_PCI_HCI
EXTERN void add_ps_timer(unsigned long task_priv);

EXTERN void add_update_ps(struct rtl8192cd_priv *priv, struct stat_info *pstat);
#endif
EXTERN void send_h2c_cmd_detect_wps_gpio(struct rtl8192cd_priv *priv, unsigned int gpio_num, unsigned int enable, unsigned int high_active);

#ifdef SDIO_AP_OFFLOAD
EXTERN void set_ap_ps_mode(struct rtl8192cd_priv *priv, unsigned char *data);
EXTERN void ap_offload_deactivate(struct rtl8192cd_priv *priv, int reason);
EXTERN void ap_offload_activate(struct rtl8192cd_priv *priv, int reason);
#endif


EXTERN void refill_skb_queue(struct rtl8192cd_priv *priv);
EXTERN void free_skb_queue(struct rtl8192cd_priv *priv, struct sk_buff_head *skb_que);

#ifdef FAST_RECOVERY
EXTERN void *backup_sta(struct rtl8192cd_priv *priv);
EXTERN void restore_backup_sta(struct rtl8192cd_priv *priv, void *pInfo);
#endif

#ifdef RTK_QUE
EXTERN void rtk_queue_init(struct ring_que *que);
EXTERN void free_rtk_queue(struct rtl8192cd_priv *priv, struct ring_que *skb_que);
#endif


#ifdef CONFIG_RTL8190_PRIV_SKB
EXTERN void init_priv_skb_buf(struct rtl8192cd_priv *priv);
EXTERN int is_rtl8190_priv_buf(unsigned char *head);
EXTERN void free_rtl8190_priv_buf(unsigned char *head);
#endif

EXTERN unsigned int orForce20_Switch20Map(struct rtl8192cd_priv * priv);
EXTERN unsigned int orSTABitMap(STA_BITMAP *map);
EXTERN unsigned char getSTABitMap(STA_BITMAP *map, int bitIdx);
EXTERN void setSTABitMap(STA_BITMAP *map, int bitIdx);
EXTERN void clearSTABitMap(STA_BITMAP *map, int bitIdx);

#if defined(CONFIG_USB_HCI) || defined(CONFIG_SDIO_HCI)
EXTERN void update_intel_sta_bitmap(struct rtl8192cd_priv *priv, struct stat_info *pstat, int release);
#endif

#if defined(WIFI_11N_2040_COEXIST_EXT)
EXTERN void update_40m_staMap(struct rtl8192cd_priv *priv, struct stat_info *pstat, int release);
EXTERN void checkBandwidth(struct rtl8192cd_priv *priv);
#endif

// global variables
EXTERN UINT8 Realtek_OUI[];
EXTERN UINT8 dot11_rate_table[];

/*
EXTERN unsigned int set_fw_reg(struct rtl8192cd_priv *priv, unsigned int cmd, unsigned int val, unsigned int with_val);
EXTERN void set_fw_A2_entry(struct rtl8192cd_priv * priv, unsigned int cmd, unsigned char * addr);
*/


#ifdef CONFIG_RTK_MESH
EXTERN int expire_mesh_txcache(struct rtl8192cd_priv*, unsigned char *);
#endif

EXTERN unsigned char oui_rfc1042[];
EXTERN unsigned char oui_8021h[];
EXTERN unsigned char oui_cisco[];
EXTERN unsigned char SNAP_ETH_TYPE_IPX[];
EXTERN unsigned char SNAP_ETH_TYPE_APPLETALK_AARP[];
EXTERN unsigned char SNAP_ETH_TYPE_APPLETALK_DDP[];
EXTERN unsigned char SNAP_HDR_APPLETALK_DDP[];

#ifdef PRIV_STA_BUF
EXTERN void init_priv_sta_buf(struct rtl8192cd_priv *priv);
EXTERN struct aid_obj *alloc_sta_obj(struct rtl8192cd_priv*);
EXTERN void free_sta_obj(struct rtl8192cd_priv *priv, struct aid_obj *obj);
#if defined(INCLUDE_WPA_PSK) || defined(WIFI_HAPD) || defined(RTK_NL80211)
//EXTERN void free_wpa_buf(struct rtl8192cd_priv *priv, WPA_STA_INFO *buf);
EXTERN void free_wpa_buf(struct rtl8192cd_priv *priv, struct _wpa_sta_info *buf);
#endif
#if defined(WIFI_WMM) && defined(WMM_APSD)
EXTERN void free_sta_que(struct rtl8192cd_priv *priv, struct apsd_pkt_queue *que);
#endif
#if defined(WIFI_WMM)
EXTERN void free_sta_mgt_que(struct rtl8192cd_priv *priv, struct dz_mgmt_queue *que);
#endif
#endif


EXTERN int rtl_string_to_hex(char *string, unsigned char *key, int len);


#if defined(TXREPORT)
EXTERN void requestTxReport(struct rtl8192cd_priv *priv);
EXTERN void C2H_isr(struct rtl8192cd_priv *priv);
#endif

EXTERN struct stat_info* findNextSTA(struct rtl8192cd_priv *priv, int *idx);

#if defined(DUALBAND_ONLY) && defined(CONFIG_RTL8190_PRIV_SKB)
void merge_pool(struct rtl8192cd_priv *priv);
void split_pool(struct rtl8192cd_priv *priv);
#endif

#ifdef TLN_STATS
EXTERN void stats_conn_rson_counts(struct rtl8192cd_priv * priv, unsigned int reason);
EXTERN void stats_conn_status_counts(struct rtl8192cd_priv * priv, unsigned int status);
#endif

#if defined(CONFIG_USB_HCI) || defined(CONFIG_SDIO_HCI)
EXTERN void _init_txservq(struct tx_servq *ptxservq, int q_num);

EXTERN struct list_head* asoc_list_get_next(struct rtl8192cd_priv *priv, struct list_head *plist);
EXTERN void asoc_list_unref(struct rtl8192cd_priv *priv, struct stat_info *pstat);
EXTERN int asoc_list_del(struct rtl8192cd_priv *priv, struct stat_info *pstat);
EXTERN int asoc_list_add(struct rtl8192cd_priv *priv, struct stat_info *pstat);
#endif

#ifdef RF_MIMO_SWITCH
EXTERN void RF_MIMO_check_timer(unsigned long task_priv);
EXTERN int assign_MIMO_TR_Mode(struct rtl8192cd_priv *priv, unsigned char *data);
#endif

#if defined(CONFIG_RTL_8812_SUPPORT) || defined(CONFIG_WLAN_HAL) || defined(CONFIG_RTL_8723B_SUPPORT)
EXTERN void update_RAMask_to_FW(struct rtl8192cd_priv *priv, int legacy);
#endif

EXTERN BOOLEAN CheckCts2SelfEnable(UINT8 rtsTxRate);
EXTERN UINT8 find_rts_rate(struct rtl8192cd_priv *priv, UINT8 TxRate, BOOLEAN bErpProtect);

EXTERN void Assert_BB_Reset(struct rtl8192cd_priv *priv);
EXTERN void Release_BB_Reset(struct rtl8192cd_priv *priv);

#ifdef WIFI_SIMPLE_CONFIG
void wsc_disconn_list_add(struct rtl8192cd_priv *priv, unsigned char *hwaddr);
void wsc_disconn_list_expire(struct rtl8192cd_priv *priv);
void wsc_disconn_list_update(struct rtl8192cd_priv *priv, unsigned char *hwaddr);
int wsc_disconn_list_check(struct rtl8192cd_priv *priv, unsigned char *hwaddr);
void wsc_disconn_list_flush(struct rtl8192cd_priv *priv);
#endif

#if defined(RTK_WLAN_EVENT_INDICATE)
EXTERN int rtk_wlan_event_indicate(char* ifname, int event, unsigned char* user, char reason);
#endif
	
#ifdef CONFIG_IEEE80211R
EXTERN void set_r0key_expire_time(struct rtl8192cd_priv *priv, unsigned char *sta_addr, unsigned long timeout);
EXTERN void check_r0key_expire(struct rtl8192cd_priv *priv);
EXTERN int store_r0kh(struct rtl8192cd_priv *priv, unsigned char *sta_addr, unsigned char *pmk_r0, unsigned char *pmk_r0_name);
EXTERN struct r0_key_holder *search_r0kh(struct rtl8192cd_priv *priv, unsigned char *sta_addr);
EXTERN void remove_r0kh(struct rtl8192cd_priv *priv, unsigned char *sta_addr);
EXTERN void free_r0kh(struct rtl8192cd_priv *priv);
EXTERN int store_r1kh(struct rtl8192cd_priv *priv, unsigned char *sta_addr, unsigned char *r1kh_id,
	unsigned char *r0kh_id, unsigned int r0kh_id_len, unsigned char *pmk_r1, unsigned char *pmk_r1_name,
	unsigned char *pmk_r0_name, int pairwise);
EXTERN struct r1_key_holder *search_r1kh(struct rtl8192cd_priv *priv, unsigned char *sta_addr, unsigned char *r1kh_id);
EXTERN struct r1_key_holder *search_r1kh_by_pmkid(struct rtl8192cd_priv *priv, unsigned char *pmkid, unsigned int id);
EXTERN int derive_r1_key(struct rtl8192cd_priv *priv, unsigned char *sta_addr, unsigned char *r1kh_id, struct r1_key_holder *out);
EXTERN void remove_r1kh(struct rtl8192cd_priv *priv, unsigned char *sta_addr, unsigned char *r1kh_id);
EXTERN void free_r1kh(struct rtl8192cd_priv *priv);
EXTERN void ft_event_indicate(struct rtl8192cd_priv *priv);
EXTERN unsigned int mapPairwise(unsigned char enc);
#endif

#if 0	//defined(HS2_SUPPORT) || defined(DOT11K) || defined(CH_LOAD_CAL) || defined(RTK_SMART_ROAMING)
EXTERN void rtl8192cd_cu_cntdwn_timer(unsigned long task_priv);
EXTERN void rtl8192cd_cu_start(struct rtl8192cd_priv *priv);
EXTERN void rtl8192cd_cu_stop(struct rtl8192cd_priv *priv);
#endif
#ifdef RSSI_MONITOR_NCR
EXTERN void rssi_monitor(struct rtl8192cd_priv *priv, struct stat_info *pstat, unsigned char rssi);
EXTERN void rssi_event_indicate(struct rtl8192cd_priv *priv);
#endif
EXTERN struct stat_info *get_macIDinfo(struct rtl8192cd_priv *rpriv, unsigned int macid);
#if defined(MULTI_STA_REFINE)
EXTERN void TxPktBuf_AgingTimer(unsigned long task_priv);
#endif
#ifdef TX_SHORTCUT
EXTERN void clearTxShortCutBufSize(struct rtl8192cd_priv *priv, struct stat_info *pstat);
EXTERN void clearAllTxShortCut(struct rtl8192cd_priv *priv);
#endif

EXTERN void dynamic_AC_bandwidth(struct rtl8192cd_priv *priv, struct stat_info *pstat);
EXTERN void dynamic_N_bandwidth(struct rtl8192cd_priv *priv, struct stat_info *pstat);

#undef EXTERN


/*-----------------------------------------------------------------------------
								8192cd_tx.c
------------------------------------------------------------------------------*/
#ifndef _8192CD_TX_C_
#define EXTERN	extern
#else
#define EXTERN
#endif

EXTERN 
#ifdef __OSK__
__IRAM_WIFI_PRI5
#else
__MIPS16 __IRAM_IN_865X 
#endif
void assign_wlanseq(struct rtl8192cd_hw *phw, unsigned char *pframe, struct stat_info *pstat, struct wifi_mib *pmib
#ifdef CONFIG_RTK_MESH	// For message if broadcast data frame via mesh (ex:ARP requst)
	, unsigned char is_11s
#endif
	);

#ifdef SW_TX_QUEUE
EXTERN void rtl8192cd_swq_timeout(unsigned long task_priv);
EXTERN void rtl8192cd_swq_deltimer(struct rtl8192cd_priv *priv, struct stat_info* pstat, unsigned char qnum);
#endif

#ifdef RTK_ATM
EXTERN void rtl8192cd_atm_swq_timeout(unsigned long task_priv);
#endif

EXTERN __IRAM_IN_865X int rtl8192cd_start_xmit(struct sk_buff *skb, struct net_device *dev);
#ifdef SUPPORT_TX_MCAST2UNI
EXTERN __IRAM_IN_865X int rtl8192cd_start_xmit_noM2U(struct sk_buff *skb, struct net_device *dev);
#else
#define		rtl8192cd_start_xmit_noM2U		rtl8192cd_start_xmit
#endif
EXTERN int rtl8192cd_wlantx(struct rtl8192cd_priv *priv, struct tx_insn *txcfg);
EXTERN
#ifdef __OSK__
__IRAM_WIFI_PRI3
#endif
void rtl8192cd_tx_dsr(unsigned long task_priv);

EXTERN int rtl8192cd_firetx(struct rtl8192cd_priv *priv, struct tx_insn *txcfg);
EXTERN int __rtl8192cd_firetx(struct rtl8192cd_priv *priv, struct tx_insn* txcfg);
#ifdef CONFIG_PCI_HCI
EXTERN void rtl8192cd_signin_txdesc(struct rtl8192cd_priv *priv, struct tx_insn* txcfg);
#endif

#ifdef TX_SHORTCUT
//EXTERN void signin_txdesc_shortcut(struct rtl8190_priv *priv, struct tx_insn *txcfg);
#ifdef CONFIG_WLAN_HAL
EXTERN
#if (!defined(__OSK__)) || (defined(__OSK__) && !defined(CONFIG_RTL6028))
__MIPS16
#endif
__IRAM_IN_865X 
void rtl88XX_signin_txdesc_shortcut(struct rtl8192cd_priv *priv, struct tx_insn *txcfg, int idx);
#endif // CONFIG_WLAN_HAL

#ifdef HW_DETEC_POWER_STATE
EXTERN void detect_hw_pwr_state(struct rtl8192cd_priv *priv, unsigned char macIDGroup);
EXTERN __IRAM_IN_865X void pwr_state_enhaced(struct rtl8192cd_priv *priv, unsigned char macID, unsigned char PwrBit);
#endif
#ifdef CONFIG_PCI_HCI
EXTERN
#if (!defined(__OSK__)) || (defined(__OSK__) && !defined(CONFIG_RTL6028))
 __MIPS16
#endif
__IRAM_IN_865X 
void rtl8192cd_signin_txdesc_shortcut(struct rtl8192cd_priv *priv, struct tx_insn *txcfg, int idx);
#endif
#endif

#ifdef CONFIG_8814_AP_MAC_VERI //eric-8822
void issue_Test_NullData(struct rtl8192cd_priv *priv,unsigned char macID,unsigned char type,unsigned char IstxToAP);
void RX_MAC_Verify_8814(struct rtl8192cd_priv *priv,unsigned char * pframe,struct rx_frinfo *pfrinfo);
#endif 

EXTERN int SetupOneCmdPacket(struct rtl8192cd_priv *priv, unsigned char *dat_content,
				unsigned short txLength, unsigned char LastPkt);
EXTERN void amsdu_timeout(struct rtl8192cd_priv *priv, unsigned int current_time);

EXTERN int rtl8192cd_SetupOneCmdPacket(struct rtl8192cd_priv *priv, unsigned char *dat_content, unsigned short txLength, unsigned char LastPkt);

#ifdef CONFIG_RTK_MESH
EXTERN 
#if (!defined(__OSK__)) || (defined(__OSK__) && !defined(CONFIG_RTL6028))
__MIPS16
#endif
__IRAM_IN_865X
int __rtl8192cd_start_xmit_out(struct sk_buff *skb, struct stat_info *pstat, struct tx_insn *ptxcfg);
EXTERN unsigned int get_tx_rate(struct rtl8192cd_priv *priv, struct stat_info *pstat);
EXTERN unsigned int get_lowest_tx_rate(struct rtl8192cd_priv *priv, struct stat_info *pstat,	unsigned int tx_rate);
EXTERN int isSpecialFloodMac(struct rtl8192cd_priv *priv, struct sk_buff *skb);
EXTERN int isICMPv6Mng(struct sk_buff *skb);
EXTERN int isMDNS(unsigned char *data);
#else
EXTERN 
#if (!defined(__OSK__)) || (defined(__OSK__) && !defined(CONFIG_RTL6028))
__MIPS16
#endif
__IRAM_IN_865X
int __rtl8192cd_start_xmit_out(struct sk_buff *skb, struct stat_info *pstat);
#endif

#if defined(DRVMAC_LB) && defined(WIFI_WMM)
EXTERN void SendLbQosData(struct rtl8192cd_priv *priv);
EXTERN void SendLbQosNullData(struct rtl8192cd_priv *priv);
#endif

EXTERN void wep_fill_iv(struct rtl8192cd_priv *priv, unsigned char *pwlhdr, unsigned int hdrlen, unsigned long keyid);
EXTERN void tkip_fill_encheader(struct rtl8192cd_priv *priv, unsigned char *pwlhdr, unsigned int hdrlen, unsigned long keyid_out);
EXTERN 
#if (!defined(__OSK__)) || (defined(__OSK__) && !defined(CONFIG_RTL6028))
__MIPS16
#endif
#ifndef WIFI_MIN_IMEM_USAGE
__IRAM_IN_865X
#endif
void aes_fill_encheader(struct rtl8192cd_priv *priv, unsigned char *pwlhdr, unsigned int hdrlen, unsigned long keyid);

#if defined(CONFIG_USB_HCI) || defined(CONFIG_SDIO_HCI)
#ifdef RTL_MANUAL_EDCA
EXTERN unsigned int PRI_TO_QNUM(struct rtl8192cd_priv *priv, int priority);
#endif
EXTERN __MIPS16 __IRAM_IN_865X unsigned int get_skb_priority(struct rtl8192cd_priv *priv, struct sk_buff *skb, struct stat_info *pstat);
EXTERN unsigned int get_tx_rate(struct rtl8192cd_priv *priv, struct stat_info *pstat);
EXTERN unsigned int get_lowest_tx_rate(struct rtl8192cd_priv *priv, struct stat_info *pstat,
				unsigned int tx_rate);
EXTERN void RtsCheck(struct rtl8192cd_priv *priv, struct tx_insn* txcfg, u4Byte fr_len,
	BOOLEAN *bRtsEnable, BOOLEAN *bCts2SelfEnable, BOOLEAN *bHwRts, BOOLEAN *bErpProtect, BOOLEAN *bNProtect);
EXTERN void rtl8192cd_fill_fwinfo(struct rtl8192cd_priv *priv, struct tx_insn* txcfg, struct tx_desc  *pdesc, unsigned int frag_idx);
EXTERN int __rtl8192cd_usb_start_xmit(struct rtl8192cd_priv *priv, struct tx_insn* txcfg);
EXTERN int rtl8192cd_tx_slowPath(struct rtl8192cd_priv *priv, struct sk_buff *skb, struct stat_info *pstat,
				struct net_device *dev, struct net_device *wdsDev, struct tx_insn *txcfg);
#endif // CONFIG_USB_HCI || CONFIG_SDIO_HCI

#undef EXTERN




/*-----------------------------------------------------------------------------
								8192cd_sme.c
------------------------------------------------------------------------------*/
#ifndef _8192CD_SME_C_
#define EXTERN	extern
#else
#define EXTERN
#endif

#if (BEAMFORMING_SUPPORT == 1) && defined(CONFIG_WLAN_HAL_8822BE)
EXTERN unsigned char is_support_bf(struct stat_info* psta, unsigned char type);
#endif

EXTERN unsigned int get_ava_2ndchoff(struct rtl8192cd_priv *priv, unsigned int channel, unsigned int bandwidth);
EXTERN void unchainned_all_frag(struct rtl8192cd_priv *priv, struct list_head *phead);
EXTERN void rtl8192cd_expire_timer(unsigned long task_priv);
EXTERN void rtl8192cd_frag_timer(unsigned long task_priv);
EXTERN void rtl8192cd_1sec_timer(unsigned long task_priv);
#if defined(CONFIG_USB_HCI) || defined(CONFIG_SDIO_HCI)
EXTERN void pre_rtl8192cd_1sec_timer(unsigned long task_priv);
EXTERN void fill_bcn_desc(struct rtl8192cd_priv *priv, struct tx_desc *pdesc, void *dat_content, unsigned short txLength, char forceUpdate);
#endif
#if defined(CONFIG_VERIWAVE_CHECK)
EXTERN void rtl8192cd_check_veriwave_timer(unsigned long task_priv);
EXTERN void update_wifi_allitf_txrx_stats(struct rtl8192cd_priv *priv);
EXTERN unsigned long crc32(unsigned char *buf, int len);
#endif
EXTERN void mgt_handler(struct rtl8192cd_priv *priv, struct rx_frinfo *pfrinfo);
EXTERN void pwr_state(struct rtl8192cd_priv *priv, struct rx_frinfo *pfrinfo);
#ifdef CONFIG_WLAN_HAL
EXTERN void check_PS_set_HIQLMT(struct rtl8192cd_priv *priv);
#endif
EXTERN void update_beacon(struct rtl8192cd_priv *priv);
EXTERN void init_beacon(struct rtl8192cd_priv *priv);
EXTERN void signin_beacon_desc(struct rtl8192cd_priv * priv,unsigned int * beaconbuf,unsigned int frlen);
EXTERN void issue_deauth(struct rtl8192cd_priv *priv,	unsigned char *da, int reason);
EXTERN void start_clnt_ss(struct rtl8192cd_priv *priv);

EXTERN void rtl8192cd_ss_timer(unsigned long task_priv);

EXTERN void process_dzqueue(struct rtl8192cd_priv *priv);
EXTERN void issue_asocrsp(struct rtl8192cd_priv *priv,	unsigned short status, struct stat_info *pstat, int pkt_type);
EXTERN void issue_disassoc(struct rtl8192cd_priv *priv, unsigned char *da, int reason);
#if defined(WIFI_WMM)
EXTERN int check_dz_mgmt(struct rtl8192cd_priv *priv, struct stat_info *pstat, struct tx_insn* txcfg);
EXTERN int issue_ADDBArsp(struct rtl8192cd_priv *priv, unsigned char *da, unsigned char dialog_token,
				unsigned char TID, unsigned short status_code, unsigned short timeout);
#endif
EXTERN int fill_probe_rsp_content(struct rtl8192cd_priv*, UINT8*, UINT8*, UINT8*, int , int , UINT8, UINT8);
EXTERN void assign_tx_rate(struct rtl8192cd_priv *priv, struct stat_info *pstat, struct rx_frinfo *pfrinfo);
EXTERN void assign_aggre_mthod(struct rtl8192cd_priv *priv, struct stat_info *pstat);
EXTERN void assign_aggre_size(struct rtl8192cd_priv *priv, struct stat_info *pstat);
EXTERN int isErpSta(struct stat_info *pstat);
EXTERN int chklink_wkstaQ(struct rtl8192cd_priv *priv);

#ifdef CLIENT_MODE
EXTERN void start_clnt_join(struct rtl8192cd_priv *priv);
EXTERN void start_clnt_lookup(struct rtl8192cd_priv *priv, int rescan);

EXTERN void rtl8192cd_reauth_timer(unsigned long task_priv);
EXTERN void rtl8192cd_reassoc_timer(unsigned long task_priv);
EXTERN void rtl8192cd_idle_timer(unsigned long task_priv);
#ifdef DFS
EXTERN void rtl8192cd_dfs_cntdwn_timer(unsigned long task_priv);
#endif
EXTERN void issue_PsPoll(struct rtl8192cd_priv *priv);
#endif
#if defined(MULTI_MAC_CLONE) || defined(UNIVERSAL_REPEATER)
void issue_NullData(struct rtl8192cd_priv *priv, unsigned char *da);
#endif

#ifdef WIFI_WMM
EXTERN void init_WMM_Para_Element(struct rtl8192cd_priv *priv, unsigned char *temp);
EXTERN void issue_ADDBAreq(struct rtl8192cd_priv *priv, struct stat_info *pstat, unsigned char TID);
#endif

EXTERN void process_mcast_dzqueue(struct rtl8192cd_priv *priv);
EXTERN void construct_ht_ie(struct rtl8192cd_priv *priv, int use_40m, int offset);

EXTERN BOOLEAN check_adaptivity_test(struct rtl8192cd_priv *priv);
#ifdef CHECK_HANGUP
EXTERN int check_hangup(struct rtl8192cd_priv *priv);
#endif

#ifndef USE_WEP_DEFAULT_KEY
EXTERN void set_keymapping_wep(struct rtl8192cd_priv *priv, struct stat_info *pstat);
#endif

#ifdef USB_PKT_RATE_CTRL_SUPPORT
EXTERN void register_usb_pkt_cnt_f(void *usbPktFunc);
EXTERN void usbPkt_timer_handler(struct rtl8192cd_priv *priv);
#endif

#ifdef RTK_WOW
EXTERN void issue_rtk_wow(struct rtl8192cd_priv * priv, unsigned char * da);
#endif

#ifdef SUPPORT_MONITOR
EXTERN void rtl8192cd_chan_switch_timer(unsigned long task_priv);
EXTERN unsigned char *get_ie(unsigned char *pbuf, int index, int *len, int limit);
#endif
EXTERN unsigned char *set_ie(unsigned char *pbuf, int index, unsigned int len, unsigned char *source, unsigned int *frlen);
EXTERN unsigned char *get_ie(unsigned char *pbuf, int index, int *len, int limit);

#ifdef CONFIG_RTK_MESH
EXTERN unsigned char WMM_IE[];
EXTERN unsigned char WMM_PARA_IE[];
EXTERN int check_basic_rate(struct rtl8192cd_priv *priv, unsigned char *pRate, int pLen);
EXTERN int collect_bss_info(struct rtl8192cd_priv *priv, struct rx_frinfo *pfrinfo);
EXTERN unsigned char *construct_ht_ie_old_form(struct rtl8192cd_priv *priv, unsigned char *pbuf, unsigned int *frlen);
EXTERN void get_matched_rate(struct rtl8192cd_priv *priv, unsigned char *pRate, int *pLen, int which);
EXTERN void issue_deauth_MP(struct rtl8192cd_priv *priv,	unsigned char *da, int reason, UINT8 is_11s);
EXTERN void issue_probersp_MP(struct rtl8192cd_priv *priv, unsigned char *da, UINT8 *ssid, int ssid_len, int set_privacy, UINT8 is_11s, UINT8 legacy_b_sta);
EXTERN unsigned char *set_fixed_ie(unsigned char *pbuf, unsigned int len, unsigned char *source, unsigned int *frlen);
EXTERN void update_support_rate(struct	stat_info *pstat, unsigned char* buf, int len);
#endif

#if defined(CONFIG_RTK_MESH) || defined(CONFIG_IEEE80211R)
EXTERN void issue_auth(struct rtl8192cd_priv *priv, struct stat_info *pstat, unsigned short status);
#endif

EXTERN void default_WMM_para(struct rtl8192cd_priv *priv);
EXTERN unsigned char *search_wsc_tag(unsigned char *data, unsigned short id, int len, int *out_len);
#ifdef CONFIG_RTL_92D_DMDP
EXTERN void reset_dmdp_peer(struct rtl8192cd_priv *from);
#endif
#ifdef CONFIG_RTL_92D_SUPPORT
int clnt_ss_check_band(struct rtl8192cd_priv *priv, unsigned int channel);
#endif


#if (defined( UNIVERSAL_REPEATER) || defined(MBSSID)) && defined(CLIENT_MODE)
//EXTERN void set_vxd_rescan(struct rtl8192cd_priv *priv);
EXTERN void set_vxd_rescan(struct rtl8192cd_priv *priv,int rescantype);
EXTERN void  switch_profile(struct rtl8192cd_priv *priv, int idx);
EXTERN void  sync_channel_2ndch_bw(struct rtl8192cd_priv *priv);
void start_repeater_ss(struct rtl8192cd_priv *priv);

#ifdef RTK_NL80211 //wrt-adhoc
EXTERN void construct_ibss_beacon(struct rtl8192cd_priv *priv);
#ifdef __KERNEL__
void issue_beacon_ibss_vxd(unsigned long task_priv);
#elif defined(__ECOS)
EXTERN void issue_beacon_ibss_vxd(void *task_priv);
#endif
#endif 
#endif 

#if defined(TXREPORT) && defined(CONFIG_WLAN_HAL)
EXTERN void requestTxReport88XX(struct rtl8192cd_priv *priv);
#endif

#if (MU_BEAMFORMING_SUPPORT == 1)
EXTERN void requestTxReport88XX_MU(struct rtl8192cd_priv *priv);
#endif

#ifdef TXRETRY_CNT
EXTERN void requestTxRetry88XX(struct rtl8192cd_priv *priv);
#endif

#if defined(HS2_SUPPORT)
EXTERN int issue_BSS_TSM_req(struct rtl8192cd_priv *priv, DOT11_HS2_TSM_REQ *tsm_req);
#endif
EXTERN int get_center_channel(struct rtl8192cd_priv *priv, int channel, int offset, int cur); 

#if defined(MULTI_MAC_CLONE) || defined(CONFIG_RTK_MESH)
EXTERN void rtl8192cd_mclone_reauth_timer(unsigned long data);
EXTERN void rtl8192cd_mclone_reassoc_timer(unsigned long data);
EXTERN void start_clnt_auth(struct rtl8192cd_priv *priv);
extern void clear_Multi_Mac_Clone(struct rtl8192cd_priv *priv, int idx);
EXTERN int __del_mclone_addr(struct rtl8192cd_priv *priv, unsigned char *buf);
EXTERN void ap_sync_chan_to_bss(struct rtl8192cd_priv *priv, int bss_channel, int bss_bw, int bss_offset);
#endif


#ifdef CONFIG_IEEE80211R
EXTERN unsigned char *set_ft_rsnie_with_pmkid(struct rtl8192cd_priv *priv, unsigned char *pbuf, unsigned int *frlen, struct stat_info *pstat, unsigned int id);
EXTERN unsigned char *construct_mobility_domain_ie(struct rtl8192cd_priv *priv, unsigned char *pbuf, unsigned int *frlen);
EXTERN unsigned char *construct_fast_bss_transition_ie(struct rtl8192cd_priv *priv, unsigned char *pbuf, unsigned int *frlen, struct stat_info *pstat);
EXTERN unsigned char *construct_timeout_interval_ie(struct rtl8192cd_priv *priv, unsigned char *pbuf, unsigned int *frlen, int type, int value);
EXTERN void issue_ft_action(struct rtl8192cd_priv *priv, struct stat_info *pstat, unsigned char *data, unsigned int len);
#endif
#ifdef STA_ASSOC_STATISTIC
EXTERN void add_sta_assoc_status(struct rtl8192cd_priv *priv,unsigned char* addr, unsigned char rssi, unsigned char status);
#endif
#undef EXTERN




/*-----------------------------------------------------------------------------
								8192cd_led.c
------------------------------------------------------------------------------*/
#ifndef _8192CD_LED_C_
#define EXTERN	extern
#else
#define EXTERN
#endif

EXTERN void enable_sw_LED(struct rtl8192cd_priv *priv, int init);
EXTERN void disable_sw_LED(struct rtl8192cd_priv *priv);
EXTERN void calculate_sw_LED_interval(struct rtl8192cd_priv *priv);
EXTERN void control_wireless_led(struct rtl8192cd_priv *priv, int enable);
#if defined(SUPPORT_UCFGING_LED) 
EXTERN void StartCFGINGTimer(void);
extern unsigned int LED_Configuring;
#endif
#undef EXTERN




/*-----------------------------------------------------------------------------
								8192cd_dfs.c
------------------------------------------------------------------------------*/
#ifndef _8192CD_DFS_C_
#define EXTERN	extern
#else
#define EXTERN
#endif

#ifdef DFS
EXTERN void rtl8192cd_DFS_timer(unsigned long task_priv);
EXTERN void rtl8192cd_DFS_TXPAUSE_timer(unsigned long task_priv);
EXTERN void rtl8192cd_ch_avail_chk_timer(unsigned long task_priv);
EXTERN void rtl8192cd_dfs_det_chk_timer(unsigned long task_priv);
EXTERN void rtl8192cd_ch52_timer(unsigned long task_priv);
EXTERN void rtl8192cd_ch56_timer(unsigned long task_priv);
EXTERN void rtl8192cd_ch60_timer(unsigned long task_priv);
EXTERN void rtl8192cd_ch64_timer(unsigned long task_priv);
EXTERN void rtl8192cd_ch100_timer(unsigned long task_priv);
EXTERN void rtl8192cd_ch104_timer(unsigned long task_priv);
EXTERN void rtl8192cd_ch108_timer(unsigned long task_priv);
EXTERN void rtl8192cd_ch112_timer(unsigned long task_priv);
EXTERN void rtl8192cd_ch116_timer(unsigned long task_priv);
EXTERN void rtl8192cd_ch120_timer(unsigned long task_priv);
EXTERN void rtl8192cd_ch124_timer(unsigned long task_priv);
EXTERN void rtl8192cd_ch128_timer(unsigned long task_priv);
EXTERN void rtl8192cd_ch132_timer(unsigned long task_priv);
EXTERN void rtl8192cd_ch136_timer(unsigned long task_priv);
EXTERN void rtl8192cd_ch140_timer(unsigned long task_priv);
EXTERN void rtl8192cd_ch144_timer(unsigned long task_priv);

EXTERN unsigned int DFS_SelectChannel(struct rtl8192cd_priv *priv);
EXTERN int RemoveChannel(struct rtl8192cd_priv *priv, unsigned int chnl_list[], unsigned int *chnl_num, unsigned int channel);
EXTERN int InsertChannel(unsigned int chnl_list[], unsigned int *chnl_num, unsigned int channel);
EXTERN void DFS_SwChnl_clnt(struct rtl8192cd_priv *priv);
EXTERN void DFS_SwitchChannel(struct rtl8192cd_priv *priv);
EXTERN void DFS_SetReg(struct rtl8192cd_priv *priv);
EXTERN unsigned char *get_DFS_version(void);
#if defined(UNIVERSAL_REPEATER)
EXTERN int under_apmode_repeater(struct rtl8192cd_priv *priv);
#endif
EXTERN int should_defer_ss(struct rtl8192cd_priv *priv);
#endif

#if defined(DFS) || defined(RTK_AC_SUPPORT)
EXTERN void DFS_SwitchChannel(struct rtl8192cd_priv *priv);
#endif
#undef EXTERN




/*-----------------------------------------------------------------------------
								8192cd_dfs_det.c
------------------------------------------------------------------------------*/
#ifndef _8192CD_DFS_DET_C_
#define EXTERN	extern
#else
#define EXTERN
#endif

#ifdef DFS
EXTERN void rtl8192cd_dfs_det_chk(struct rtl8192cd_priv *priv);
EXTERN void rtl8192cd_dfs_dynamic_setting(struct rtl8192cd_priv *priv);
#if defined(CONFIG_WLAN_HAL_8814AE) || defined(CONFIG_WLAN_HAL_8822BE)
EXTERN void rtl8192cd_radar_type_differentiation(struct rtl8192cd_priv *priv);
#endif
#endif

#undef EXTERN




/*-----------------------------------------------------------------------------
								8192cd_rx.c
------------------------------------------------------------------------------*/
#ifndef _8192CD_RX_C_
#define EXTERN	extern
#else
#define EXTERN
#endif

EXTERN
#ifdef __OSK__
__IRAM_WIFI_PRI2
#else
__MIPS16
__IRAM_IN_865X
#endif
int validate_mpdu(struct rtl8192cd_priv *priv, struct rx_frinfo *pfrinfo);
#if defined(CONFIG_USB_HCI) || defined(CONFIG_SDIO_HCI)
#ifdef USE_OUT_SRC
EXTERN void translate_rssi_sq_outsrc(struct rtl8192cd_priv *priv, struct rx_frinfo *pfrinfo, char rate);
#endif
EXTERN void translate_CRC32_outsrc(struct rtl8192cd_priv *priv, struct rx_frinfo *pfrinfo, BOOLEAN CRC32,u2Byte PKT_LEN);
#if !defined(USE_OUT_SRC) || defined(_OUTSRC_COEXIST)
EXTERN void translate_rssi_sq(struct rtl8192cd_priv *priv, struct rx_frinfo *pfrinfo);
#endif
EXTERN void rx_pkt_exception(struct rtl8192cd_priv *priv, unsigned int cmd);
#if defined(WIFI_WMM) && defined(WMM_APSD)
EXTERN void SendQosNullData(struct rtl8192cd_priv *priv, unsigned char *da);
#endif
#endif // CONFIG_USB_HCI || CONFIG_SDIO_HCI

EXTERN 
#ifdef __OSK__
__IRAM_WIFI_PRI3
#else
#if !defined(__LINUX_2_6__) && !defined(__ECOS)
__MIPS16
#endif
__IRAM_IN_865X
#endif
void rtl88XX_rx_isr(struct rtl8192cd_priv *priv);

#ifdef CONFIG_PCI_HCI
#if defined(RX_TASKLET) || defined(__ECOS)
EXTERN void rtl8192cd_rx_tkl_isr(unsigned long task_priv);
#endif

EXTERN
#ifdef __OSK__
__IRAM_WIFI_PRI3
#else
#if !defined(__LINUX_2_6__) && !defined(__ECOS)
__MIPS16
#endif
__IRAM_IN_865X
#endif
void rtl8192cd_rx_isr(struct rtl8192cd_priv *priv);
EXTERN __IRAM_IN_865X void rtl8192cd_rx_dsr(unsigned long task_priv);
#endif
EXTERN
#if defined(CONFIG_RTL_8196E) && (!defined(__OSK__)) || (defined(__OSK__) && !defined(CONFIG_RTL6028))
__MIPS16
#endif
__IRAM_IN_865X
void rtl_netif_rx(struct rtl8192cd_priv *priv, struct sk_buff *pskb, struct stat_info *pstat);
//EXTERN void rtl8190_rxcmd_isr(struct rtl8190_priv *priv);
EXTERN 
#ifdef __OSK__
__IRAM_WIFI_PRI6
#elif !defined(WIFI_MIN_IMEM_USAGE)
__IRAM_IN_865X
#endif
void reorder_ctrl_timeout(unsigned long task_priv);
EXTERN 
#ifndef WIFI_MIN_IMEM_USAGE
__IRAM_IN_865X
#endif
void reorder_ctrl_timeout_cli(unsigned long task_priv);

EXTERN void rtl8192cd_rx_mgntframe(struct rtl8192cd_priv*, struct list_head *,struct rx_frinfo*);
#ifdef CONFIG_RTK_MESH
EXTERN
#ifdef __OSK__
__IRAM_WIFI_PRI4
#else
__MIPS16
#ifndef WIFI_MIN_IMEM_USAGE
__IRAM_IN_865X
#endif
#endif
int reorder_ctrl_check(struct rtl8192cd_priv *priv, struct stat_info *pstat, struct rx_frinfo *pfrinfo);
#endif

EXTERN
#if (!defined(__OSK__)) || (defined(__OSK__) && !defined(CONFIG_RTL6028))
__MIPS16
#endif
#ifndef WIFI_MIN_IMEM_USAGE
__IRAM_IN_865X
#endif
void reorder_ctrl_consumeQ(struct rtl8192cd_priv *priv, struct stat_info *pstat, unsigned char tid, int seg);

EXTERN
#ifdef __OSK__
__IRAM_WIFI_PRI3
#else
__MIPS16
__IRAM_IN_865X
#endif
void rtl8192cd_rx_dataframe(struct rtl8192cd_priv*, struct list_head *,struct rx_frinfo*);

#ifdef PREVENT_BROADCAST_STORM
EXTERN unsigned int get_free_memory(void);
#endif

#ifdef RX_BUFFER_GATHER
EXTERN void flush_rx_list(struct rtl8192cd_priv *priv);
#endif
#if !(defined(RTL8190_ISR_RX) && defined(RTL8190_DIRECT_RX))
EXTERN void flush_rx_queue(struct rtl8192cd_priv *priv);
#endif


#ifdef CONFIG_RTL_KERNEL_MIPS16_WLAN
__NOMIPS16
#endif
void process_amsdu(struct rtl8192cd_priv *priv, struct stat_info *pstat, struct rx_frinfo *pfrinfo);

#undef EXTERN




/*-----------------------------------------------------------------------------
								8192cd_hw.c
------------------------------------------------------------------------------*/
#ifndef _8192CD_HW_C_
#define EXTERN	extern
#else
#define EXTERN
#endif
#ifdef CONFIG_WLAN_HAL
EXTERN BOOLEAN compareAvailableTXBD(struct rtl8192cd_priv * priv, unsigned int num, unsigned int qNum, int compareFlag);
#endif
EXTERN void check_chipID_MIMO(struct rtl8192cd_priv *priv);
#ifdef EN_EFUSE
EXTERN int efuse_get(struct rtl8192cd_priv *priv, unsigned char *data);
EXTERN int efuse_set(struct rtl8192cd_priv *priv, unsigned char *data);
EXTERN int efuse_sync(struct rtl8192cd_priv *priv, unsigned char *data);
EXTERN void ReadMacAddressFromEfuse(struct rtl8192cd_priv *priv);
EXTERN void ReadTxPowerInfoFromHWPG(struct rtl8192cd_priv *priv);
EXTERN void ReadThermalMeterFromEfuse(struct rtl8192cd_priv *priv);
EXTERN void ReadLNATypeFromEfuse(struct rtl8192cd_priv *priv);
EXTERN void ReadCrystalCalibrationFromEfuse(struct rtl8192cd_priv *priv);
EXTERN void ReadTxBBSwingFromEfuse(struct rtl8192cd_priv *priv);
EXTERN void ReadDeltaValFromEfuse(struct rtl8192cd_priv * priv);
EXTERN void ReadTRSWPAPEFromEfuse(struct rtl8192cd_priv * priv);
EXTERN int ReadAdapterInfo8192CE(struct rtl8192cd_priv *priv);
EXTERN void ReadEFuseByte(struct rtl8192cd_priv *priv, unsigned short _offset, unsigned char *pbuf);
#endif
#if defined(CONFIG_WLAN_HAL_8822BE)
EXTERN void TxACurrentCalibration(struct rtl8192cd_priv *priv);
#endif
#ifdef POWER_TRIM
EXTERN void power_trim(struct rtl8192cd_priv *priv, unsigned char action);
#endif
#ifdef THER_TRIM
EXTERN void ther_trim_efuse(struct rtl8192cd_priv *priv);
EXTERN void ther_trim_act(struct rtl8192cd_priv *priv, unsigned char action);
#endif
#if defined(CONFIG_OFFLOAD_FUNCTION) || defined(SDIO_AP_OFFLOAD)
EXTERN int offloadTestFunction(struct rtl8192cd_priv *priv, unsigned char *data);
#endif

#if defined(RF_MIMO_SWITCH) || defined(PCIE_POWER_SAVING)
EXTERN void set_MIMO_Mode(struct rtl8192cd_priv *priv, unsigned pref_mode);
EXTERN int MIMO_Mode_Switch(struct rtl8192cd_priv *priv, int mode);
#endif

#ifdef PCIE_POWER_SAVING
EXTERN void PCIE_reset_procedure3(struct rtl8192cd_priv *priv);
EXTERN void PCIe_power_save_timer(unsigned long task_priv);
EXTERN void PCIe_power_save_tasklet(unsigned long task_priv);
EXTERN void init_pcie_power_saving(struct rtl8192cd_priv *priv);
EXTERN void switch_to_1x1(struct rtl8192cd_priv *priv, int mode) ;
EXTERN void PCIeWakeUp(struct rtl8192cd_priv *priv, unsigned int expTime);
EXTERN void radio_off(struct rtl8192cd_priv *priv);
#ifdef PCIE_POWER_SAVING_DEBUG
EXTERN int PCIE_PowerDown(struct rtl8192cd_priv *priv, unsigned char *data);
#endif
#ifdef GPIO_WAKEPIN
EXTERN int request_irq_for_wakeup_pin(struct net_device *dev);
#endif
#endif
//yllin
#ifdef GPIO_WAKEPIN
EXTERN int request_irq_for_wakeup_pin_V2(struct net_device *dev);
#endif
#ifdef PCIE_POWER_SAVING_TEST
EXTERN void PCIE_reset_procedure3_V2(struct rtl8192cd_priv *priv);
EXTERN void setBaseAddressRegister_V2(void);
#endif

EXTERN void set_slot_time(struct rtl8192cd_priv *priv, int use_short);
EXTERN void SetTxPowerLevel(struct rtl8192cd_priv *priv, unsigned char channel);
EXTERN void SwChnl(struct rtl8192cd_priv *priv, unsigned char channel, int offset);
#ifdef CONFIG_WLAN_HAL_8192EE
EXTERN void Check_92E_Spur_Valid(struct rtl8192cd_priv *priv, BOOLEAN long_delay);
#endif
EXTERN void enable_hw_LED(struct rtl8192cd_priv *priv, unsigned int led_type);

EXTERN unsigned int PHY_QueryRFReg(struct rtl8192cd_priv *priv, RF92CD_RADIO_PATH_E eRFPath,
				unsigned int RegAddr, unsigned int BitMask, unsigned int dbg_avoid);
EXTERN unsigned int PHY_QueryBBReg(struct rtl8192cd_priv *priv, unsigned int RegAddr, unsigned int BitMask);
EXTERN void PHY_SetBBReg(struct rtl8192cd_priv *priv, unsigned int RegAddr, unsigned int BitMask, unsigned int Data);
EXTERN void PHY_SetRFReg(struct rtl8192cd_priv *priv, RF92CD_RADIO_PATH_E eRFPath, unsigned int RegAddr,
				unsigned int BitMask, unsigned int Data);
EXTERN int phy_RF8256_Config_ParaFile(struct rtl8192cd_priv *priv);
//EXTERN int PHY_ConfigMACWithParaFile(struct rtl8192cd_priv *priv);
EXTERN void SwBWMode(struct rtl8192cd_priv *priv, unsigned int bandwidth, int offset);

EXTERN void setup_timer1(struct rtl8192cd_priv *priv, int timeout);
EXTERN void cancel_timer1(struct rtl8192cd_priv *priv);
EXTERN void setup_timer2(struct rtl8192cd_priv *priv, unsigned int timeout);
EXTERN void cancel_timer2(struct rtl8192cd_priv *priv);


//EXTERN void CCK_txpower_by_rssi(struct rtl8192cd_priv *priv, unsigned char rssi_strength);
EXTERN void reload_txpwr_pg(struct rtl8192cd_priv *priv);
EXTERN void set_lck_cv(struct rtl8192cd_priv *priv, unsigned char channel);
#ifdef CONFIG_WLAN_HAL_8814AE
EXTERN void spread_spectrum(struct rtl8192cd_priv *priv);
#endif
EXTERN int rtl8192cd_init_hw_PCI(struct rtl8192cd_priv *priv);
EXTERN int rtl8192cd_stop_hw(struct rtl8192cd_priv *priv);
EXTERN int check_MAC_IO_Enable(struct rtl8192cd_priv *priv);
#if defined(CONFIG_RTL_92D_SUPPORT) || defined(CONFIG_RTL_8812_SUPPORT) || defined(CONFIG_WLAN_HAL)
EXTERN int PHY_ConfigBBWithParaFile(struct rtl8192cd_priv *priv, int reg_file);
#endif

#ifdef CONFIG_RTL_92D_SUPPORT

EXTERN unsigned int get_mean_of_2_close_value(unsigned int *val_array);
#endif
#ifdef _TRACKING_TABLE_FILE
EXTERN int PHY_ConfigTXPwrTrackingWithParaFile(struct rtl8192cd_priv * priv);
#endif

#ifdef TXPWR_LMT
EXTERN int PHY_ConfigTXLmtWithParaFile(struct rtl8192cd_priv * priv);
#ifdef TXPWR_LMT_NEWFILE
EXTERN int PHY_ConfigTXLmtWithParaFile_new(struct rtl8192cd_priv * priv);
#endif
#ifdef BEAMFORMING_AUTO
EXTERN int PHY_ConfigTXLmtWithParaFile_new_TXBF(struct rtl8192cd_priv * priv);
EXTERN void BEAMFORMING_TXPWRLMT_Auto(struct rtl8192cd_priv *priv);
#endif
#endif

//EXTERN void tx_path_by_rssi(struct rtl8192cd_priv *priv, struct stat_info *pstat, unsigned char enable);
//EXTERN void rx_path_by_rssi(struct rtl8192cd_priv *priv, struct stat_info *pstat, int enable);
//EXTERN void rx_path_by_rssi_cck_v2(struct rtl8192cd_priv *priv, struct stat_info *pstat);
//EXTERN void rtl8192cd_tpt_timer(unsigned long task_priv);



//EXTERN void SwitchExtAnt(struct rtl8192cd_priv *priv, unsigned char EXT_ANT_PATH);
//EXTERN void rtl8192cd_ePhyInit(struct rtl8192cd_priv * priv);
//EXTERN void Switch_1SS_Antenna(struct rtl8192cd_priv *priv, unsigned int antPath );
//EXTERN void Switch_OFDM_Antenna(struct rtl8192cd_priv *priv, unsigned int antPath );
#ifdef MCR_WIRELESS_EXTEND
#ifdef CONFIG_WLAN_HAL_8814AE
EXTERN int Switch_Antenna_8814(struct rtl8192cd_priv *priv, unsigned char *data);
#endif
#endif
EXTERN void TXPowerTracking(struct rtl8192cd_priv *priv);
#ifdef CONFIG_RF_DPK_SETTING_SUPPORT
EXTERN void TX_DPK_Tracking(struct rtl8192cd_priv *priv);
#endif
#ifdef THERMAL_CONTROL
EXTERN void thermal_control_dm(struct rtl8192cd_priv * priv);
EXTERN void thermal_control_assc_dm(struct rtl8192cd_priv * priv);
#endif
#ifdef BT_COEXIST
EXTERN void bt_coex_dm(struct rtl8192cd_priv *priv);
#endif
EXTERN int get_tx_tracking_index(struct rtl8192cd_priv *priv, int channel, int i, int delta, int is_decrease, int is_CCK);

// CAM related functions
EXTERN int CamAddOneEntry(struct rtl8192cd_priv *priv, unsigned char *pucMacAddr,
				unsigned long ulKeyId, unsigned long ulEncAlg, unsigned long ulUseDK,
				unsigned char *pucKey);
EXTERN int CamDeleteOneEntry(struct rtl8192cd_priv *priv, unsigned char *pucMacAddr,
				unsigned long ulKeyId, unsigned int useDK);
EXTERN void CamResetAllEntry(struct rtl8192cd_priv *priv);
EXTERN void CamDumpAll(struct rtl8192cd_priv *priv);
EXTERN void CAM_read_entry(struct rtl8192cd_priv *priv, unsigned char index, unsigned char *macad,
				unsigned char *key128, unsigned short *config);
EXTERN int get_offset_val(unsigned char *line_head, unsigned int *u4bRegOffset, unsigned int *u4bRegValue);
EXTERN unsigned char *get_line(unsigned char **line);

#ifdef CAM_SWAP
EXTERN int get_sw_encrypt_sta_num(struct rtl8192cd_priv *priv);
EXTERN void cal_sta_traffic(struct rtl8192cd_priv *priv, int sta_use_sw_encrypt);
EXTERN void rotate_sta_cam(struct rtl8192cd_priv *priv, int sta_use_sw_encrypt);
#endif

#ifdef TXPWR_LMT
EXTERN int ch2idx(int ch);
EXTERN void find_pwr_limit(struct rtl8192cd_priv *priv, int channel, int offset);
#ifdef TXPWR_LMT_NEWFILE
EXTERN void find_pwr_limit_new(struct rtl8192cd_priv *priv, int channel, int offset);
#endif
#endif

#ifdef POWER_PERCENT_ADJUSTMENT
EXTERN s1Byte PwrPercent2PwrLevel(int percentage);
#endif
#ifdef _DEBUG_RTL8192CD_	
//_TXPWR_REDEFINE
EXTERN int Read_PG_File(struct rtl8192cd_priv *priv, int reg_file, int table_number, 
				char *MCSTxAgcOffset_A, char *MCSTxAgcOffset_B, char *OFDMTxAgcOffset_A,
				char *OFDMTxAgcOffset_B, char *CCKTxAgc_A, char *CCKTxAgc_B);
#endif


#ifdef CONFIG_RTL_92D_SUPPORT
	EXTERN unsigned char *data_MACPHY_REG_start, *data_MACPHY_REG_end;
  	EXTERN unsigned char *data_PHY_REG_n_start, *data_PHY_REG_n_end;
  	EXTERN unsigned char *data_PHY_REG_MP_n_start, *data_PHY_REG_MP_n_end;
	EXTERN unsigned char *data_PHY_REG_PG_start, *data_PHY_REG_PG_end;
	EXTERN unsigned char *data_PHY_REG_PG_FCC_start, *data_PHY_REG_PG_FCC_end;
	EXTERN unsigned char *data_PHY_REG_PG_CE_start, *data_PHY_REG_PG_CE_end;

#ifdef _TRACKING_TABLE_FILE
	EXTERN unsigned char *data_REG_TXPWR_TRK_n_92d_start, *data_REG_TXPWR_TRK_n_92d_end;
	EXTERN unsigned char *data_REG_TXPWR_TRK_n_92d_hp_start, *data_REG_TXPWR_TRK_n_92d_hp_end;
#endif
	EXTERN unsigned char *data_AGC_TAB_n_start, *data_AGC_TAB_n_end;
	EXTERN unsigned char *data_AGC_TAB_2G_n_start, *data_AGC_TAB_2G_n_end;
	EXTERN unsigned char *data_AGC_TAB_5G_n_start, *data_AGC_TAB_5G_n_end;
	EXTERN unsigned char *data_radio_a_n_start, *data_radio_a_n_end;
	EXTERN unsigned char *data_radio_b_n_start, *data_radio_b_n_end;
#ifdef RTL8192D_INT_PA
#ifdef USB_POWER_SUPPORT

#if defined (RTL8192D_INT_PA_GAIN_TABLE_NEW)
	EXTERN unsigned char *data_radio_a_intPA_GM_new_start, *data_radio_a_intPA_GM_new_end;
	EXTERN unsigned char *data_radio_b_intPA_GM_new_start, *data_radio_b_intPA_GM_new_end;
#elif defined (RTL8192D_INT_PA_GAIN_TABLE_NEW1)
	EXTERN unsigned char *data_radio_a_intPA_GM_new1_start, *data_radio_a_intPA_GM_new1_end;
	EXTERN unsigned char *data_radio_b_intPA_GM_new1_start, *data_radio_b_intPA_GM_new1_end;
#else
	EXTERN unsigned char *data_radio_a_intPA_GM_start, *data_radio_a_intPA_GM_end;
	EXTERN unsigned char *data_radio_b_intPA_GM_start, *data_radio_b_intPA_GM_end;
#endif

#else // USB_POWER_SUPPORT

#if defined (RTL8192D_INT_PA_GAIN_TABLE_NEW)
	EXTERN unsigned char *data_radio_a_intPA_new_start, *data_radio_a_intPA_new_end;
	EXTERN unsigned char *data_radio_b_intPA_new_start, *data_radio_b_intPA_new_end;
#else
	EXTERN unsigned char *data_radio_a_intPA_start, *data_radio_a_intPA_end;
	EXTERN unsigned char *data_radio_b_intPA_start, *data_radio_b_intPA_end;
#endif

#endif // USB_POWER_SUPPORT
#endif // RTL8192D_INT_PA

//_TXPWR_REDEFINE
#ifdef HIGH_POWER_EXT_PA
	EXTERN unsigned char *data_AGC_TAB_n_92d_hp_start, *data_AGC_TAB_n_92d_hp_end;
	EXTERN unsigned char *data_PHY_REG_PG_92d_hp_start, *data_PHY_REG_PG_92d_hp_end;
	EXTERN unsigned char *data_PHY_REG_n_92d_hp_start, *data_PHY_REG_n_92d_hp_end;
	EXTERN unsigned char *data_radio_a_n_92d_hp_start, *data_radio_a_n_92d_hp_end;
	EXTERN unsigned char *data_radio_b_n_92d_hp_start, *data_radio_b_n_92d_hp_end;
#endif

	EXTERN unsigned char *data_rtl8192dfw_n_start, *data_rtl8192dfw_n_end;
#endif // CONFIG_RTL_92D_SUPPORT

#ifdef CONFIG_RTL_92C_SUPPORT
#ifdef TESTCHIP_SUPPORT
	EXTERN unsigned char *data_AGC_TAB_start, *data_AGC_TAB_end;
	EXTERN unsigned char *data_PHY_REG_2T_start, *data_PHY_REG_2T_end;
	EXTERN unsigned char *data_PHY_REG_1T_start, *data_PHY_REG_1T_end;
	EXTERN unsigned char *data_radio_a_1T_start, *data_radio_a_1T_end;
	EXTERN unsigned char *data_radio_a_2T_start, *data_radio_a_2T_end;
	EXTERN unsigned char *data_radio_b_2T_start, *data_radio_b_2T_end;
	EXTERN unsigned char *data_rtl8192cfw_start, *data_rtl8192cfw_end;
#endif

EXTERN unsigned char *data_AGC_TAB_n_92C_start, *data_AGC_TAB_n_92C_end;
EXTERN unsigned char *data_PHY_REG_2T_n_start, *data_PHY_REG_2T_n_end;
EXTERN unsigned char *data_PHY_REG_1T_n_start, *data_PHY_REG_1T_n_end;
EXTERN unsigned char *data_radio_a_2T_n_start, *data_radio_a_2T_n_end;
EXTERN unsigned char *data_radio_b_2T_n_start, *data_radio_b_2T_n_end;
EXTERN unsigned char *data_radio_a_1T_n_start, *data_radio_a_1T_n_end;
EXTERN unsigned char *data_rtl8192cfw_n_start, *data_rtl8192cfw_n_end;
EXTERN unsigned char *data_rtl8192cfw_ua_start, *data_rtl8192cfw_ua_end;

EXTERN unsigned char *data_MACPHY_REG_92C_start, *data_MACPHY_REG_92C_end;
EXTERN unsigned char *data_PHY_REG_PG_92C_start, *data_PHY_REG_PG_92C_end;
EXTERN unsigned char *data_PHY_REG_MP_n_92C_start, *data_PHY_REG_MP_n_92C_end;

EXTERN unsigned char *data_AGC_TAB_n_hp_start, *data_AGC_TAB_n_hp_end;
EXTERN unsigned char *data_PHY_REG_2T_n_lna_start, *data_PHY_REG_2T_n_lna_end;
EXTERN unsigned char *data_PHY_REG_2T_n_hp_start, *data_PHY_REG_2T_n_hp_end;
EXTERN unsigned char *data_PHY_REG_1T_n_hp_start, *data_PHY_REG_1T_n_hp_end;
EXTERN unsigned char *data_radio_a_2T_n_lna_start, *data_radio_a_2T_n_lna_end;
EXTERN unsigned char *data_radio_b_2T_n_lna_start, *data_radio_b_2T_n_lna_end;

#ifdef HIGH_POWER_EXT_PA
	EXTERN unsigned char *data_radio_a_2T_n_hp_start, *data_radio_a_2T_n_hp_end;
	EXTERN unsigned char *data_radio_b_2T_n_hp_start, *data_radio_b_2T_n_hp_end;
	EXTERN unsigned char *data_PHY_REG_PG_hp_start, *data_PHY_REG_PG_hp_end;
#endif

#ifdef _TRACKING_TABLE_FILE
	EXTERN unsigned char *data_REG_TXPWR_TRK_start, *data_REG_TXPWR_TRK_end;
	EXTERN unsigned char *data_REG_TXPWR_TRK_hp_start, *data_REG_TXPWR_TRK_hp_end;
#endif

#endif // CONFIG_RTL_92C_SUPPORT

#ifdef CONFIG_RTL_88E_SUPPORT
#if !defined(USE_OUT_SRC) || defined(_OUTSRC_COEXIST)
//For 8188E IQK
EXTERN unsigned char *data_AGC_TAB_1T_88E_start, *data_AGC_TAB_1T_88E_end;
EXTERN unsigned char *data_MAC_REG_88E_start, *data_MAC_REG_88E_end;
EXTERN unsigned char *data_PHY_REG_1T_88E_start, *data_PHY_REG_1T_88E_end;
EXTERN unsigned char *data_PHY_REG_MP_88E_start, *data_PHY_REG_MP_88E_end;
EXTERN unsigned char *data_PHY_REG_PG_88E_start, *data_PHY_REG_PG_88E_end;
EXTERN unsigned char *data_radio_a_1T_88E_start, *data_radio_a_1T_88E_end;
#ifdef SUPPORT_RTL8188E_TC
EXTERN unsigned char *data_MAC_REG_88E_TC_start, *data_MAC_REG_88E_TC_end;
EXTERN unsigned char *data_PHY_REG_1T_88E_TC_start, *data_PHY_REG_1T_88E_TC_end;
EXTERN unsigned char *data_radio_a_1T_88E_TC_start, *data_radio_a_1T_88E_TC_end;
#endif
EXTERN unsigned char *data_PHY_REG_PG_88E_new_start, *data_PHY_REG_PG_88E_new_end;
#ifdef TXPWR_LMT_88E
EXTERN unsigned char *data_TXPWR_LMT_88E_new_start, *data_TXPWR_LMT_88E_new_end;
#endif
#endif
#endif // CONFIG_RTL_88E_SUPPORT

#ifdef CONFIG_RTL_8723B_SUPPORT
EXTERN unsigned char *data_TxPowerTrack_AP_8723_start,    *data_TxPowerTrack_AP_8723_end;
EXTERN unsigned char *data_TXPWR_LMT_8723_start, *data_TXPWR_LMT_8723_end;
EXTERN unsigned char *data_PHY_REG_PG_8723_start, *data_PHY_REG_PG_8723_end;
EXTERN unsigned char *data_rtl8723bfw_start, *data_rtl8723bfw_end;
#endif 

#ifdef CONFIG_RTL_8812_SUPPORT
EXTERN unsigned char *data_AGC_TAB_8812_start, *data_AGC_TAB_8812_end;
EXTERN unsigned char *data_MAC_REG_8812_start, *data_MAC_REG_8812_end;
EXTERN unsigned char *data_PHY_REG_8812_start, *data_PHY_REG_8812_end;
EXTERN unsigned char *data_PHY_REG_MP_8812_start, *data_PHY_REG_MP_8812_end;
EXTERN unsigned char *data_PHY_REG_PG_8812_start, *data_PHY_REG_PG_8812_end;
EXTERN unsigned char *data_RadioA_8812_start, *data_RadioA_8812_end;
EXTERN unsigned char *data_RadioB_8812_start, *data_RadioB_8812_end;
#ifdef _TRACKING_TABLE_FILE
EXTERN unsigned char *data_REG_TXPWR_TRK_8812_start, *data_REG_TXPWR_TRK_8812_end;
#ifdef HIGH_POWER_EXT_PA
EXTERN unsigned char *data_REG_TXPWR_TRK_8812_hp_start, *data_REG_TXPWR_TRK_8812_hp_end;
EXTERN unsigned char *data_PHY_REG_PG_8812_hp_start, *data_PHY_REG_PG_8812_hp_end;
#endif
#endif

//FOR_8812_MP_CHIP
EXTERN unsigned char *data_MAC_REG_8812_n_start, *data_MAC_REG_8812_n_end;
EXTERN unsigned char *data_AGC_TAB_8812_n_default_start, *data_AGC_TAB_8812_n_default_end;
EXTERN unsigned char *data_PHY_REG_8812_n_default_start, *data_PHY_REG_8812_n_default_end;
EXTERN unsigned char *data_RadioA_8812_n_default_start, *data_RadioA_8812_n_default_end;
EXTERN unsigned char *data_RadioB_8812_n_default_start, *data_RadioB_8812_n_default_end;

EXTERN unsigned char *data_AGC_TAB_8812_n_extlna_start, *data_AGC_TAB_8812_n_extlna_end;
EXTERN unsigned char *data_PHY_REG_8812_n_extlna_start, *data_PHY_REG_8812_n_extlna_end;
EXTERN unsigned char *data_RadioA_8812_n_extlna_start, *data_RadioA_8812_n_extlna_end;
EXTERN unsigned char *data_RadioB_8812_n_extlna_start, *data_RadioB_8812_n_extlna_end;

EXTERN unsigned char *data_AGC_TAB_8812_n_extpa_start, *data_AGC_TAB_8812_n_extpa_end;
EXTERN unsigned char *data_PHY_REG_8812_n_extpa_start, *data_PHY_REG_8812_n_extpa_end;
EXTERN unsigned char *data_RadioA_8812_n_extpa_start, *data_RadioA_8812_n_extpa_end;
EXTERN unsigned char *data_RadioB_8812_n_extpa_start, *data_RadioB_8812_n_extpa_end;

EXTERN unsigned char *data_AGC_TAB_8812_hp_start, *data_AGC_TAB_8812_hp_end;
EXTERN unsigned char *data_RadioA_8812_hp_start, *data_RadioA_8812_hp_end;
EXTERN unsigned char *data_RadioB_8812_hp_start, *data_RadioB_8812_hp_end;

//FOR_8812_MP_CHIP
EXTERN unsigned char *data_AGC_TAB_8812_n_hp_start, *data_AGC_TAB_8812_n_hp_end;
EXTERN unsigned char *data_PHY_REG_8812_n_hp_start, *data_PHY_REG_8812_n_hp_end;
EXTERN unsigned char *data_RadioA_8812_n_hp_start, *data_RadioA_8812_n_hp_end;
EXTERN unsigned char *data_RadioB_8812_n_hp_start, *data_RadioB_8812_n_hp_end;
EXTERN unsigned char *data_RadioA_8812_n_ultra_hp_start, *data_RadioA_8812_n_ultra_hp_end;
EXTERN unsigned char *data_RadioB_8812_n_ultra_hp_start, *data_RadioB_8812_n_ultra_hp_end;

EXTERN unsigned char *data_PHY_REG_PG_8812_new_start, *data_PHY_REG_PG_8812_new_end;
#ifdef TXPWR_LMT_8812
EXTERN unsigned char *data_TXPWR_LMT_8812_new_start, *data_TXPWR_LMT_8812_new_end;
#endif

#endif

EXTERN void PHY_IQCalibrate(struct rtl8192cd_priv *priv);

#ifdef TX_EARLY_MODE
EXTERN void enable_em(struct rtl8192cd_priv *priv);
EXTERN void disable_em(struct rtl8192cd_priv *priv);
#endif

#if defined(CONFIG_RTL_88E_SUPPORT) && defined(CALIBRATE_BY_ODM)
EXTERN void PHY_RF6052SetCCKTxPower(struct rtl8192cd_priv *priv, unsigned int channel);
EXTERN void PHY_RF6052SetOFDMTxPower(struct rtl8192cd_priv *priv, unsigned int channel);
#endif
#ifdef RTLWIFINIC_GPIO_CONTROL
EXTERN void RTLWIFINIC_GPIO_init_priv(struct rtl8192cd_priv *priv);
EXTERN void RTLWIFINIC_GPIO_config(unsigned int gpio_num, unsigned int direction);
EXTERN void RTLWIFINIC_GPIO_write(unsigned int gpio_num, unsigned int value);
EXTERN int RTLWIFINIC_GPIO_read(unsigned int gpio_num);
EXTERN void RTLWIFINIC_GPIO_config_proc(struct rtl8192cd_priv *priv, unsigned int gpio_num, unsigned int direction);
EXTERN void RTLWIFINIC_GPIO_write_proc(struct rtl8192cd_priv *priv, unsigned int gpio_num, unsigned int value);
EXTERN int RTLWIFINIC_GPIO_read_proc(struct rtl8192cd_priv *priv, unsigned int gpio_num);

#endif


#ifdef AC2G_256QAM
EXTERN char is_ac2g(struct rtl8192cd_priv* priv);
#endif

#ifdef CONFIG_1RCCA_RF_POWER_SAVING
void one_path_cca_power_save(struct rtl8192cd_priv *priv, int enable);
#endif

#if defined(HS2_SUPPORT) || defined(RTK_NL80211)/*survey_dump*/ || defined(DOT11K) || defined(CH_LOAD_CAL) || defined(RTK_SMART_ROAMING)
EXTERN void start_bbp_ch_load(struct rtl8192cd_priv *priv, unsigned int units);
EXTERN unsigned int read_bbp_ch_load(struct rtl8192cd_priv *priv);
EXTERN void channle_loading_measurement(struct rtl8192cd_priv *priv);
#endif


#undef EXTERN




#ifdef CONFIG_RTL_92D_SUPPORT
/*-----------------------------------------------------------------------------
								8192d_hw.c
------------------------------------------------------------------------------*/
#ifndef _8192D_HW_C_
#define EXTERN  extern
#else
#define EXTERN
#endif

#ifdef CONFIG_RTL_92D_DMDP
EXTERN unsigned int DMDP_RTL_R32(unsigned int phy,unsigned int reg);
EXTERN unsigned short DMDP_RTL_R16(unsigned int phy,unsigned int reg);
EXTERN unsigned char DMDP_RTL_R8(unsigned int phy,unsigned int reg);
EXTERN void DMDP_RTL_W32(unsigned int phy,unsigned int reg,unsigned int val32);
EXTERN void DMDP_RTL_W16(unsigned int phy,unsigned int reg,unsigned short val16);
EXTERN void DMDP_RTL_W8(unsigned int phy,unsigned int reg,unsigned char val8);
EXTERN unsigned int DMDP_PHY_QueryBBReg(unsigned int phy,unsigned int RegAddr,unsigned int BitMask);
EXTERN void DMDP_PHY_SetBBReg(unsigned int phy,unsigned int RegAddr,unsigned int BitMask,unsigned int Data);
EXTERN unsigned int DMDP_PHY_QueryRFReg(unsigned int phy,RF92CD_RADIO_PATH_E eRFPath,unsigned int RegAddr,unsigned int BitMask,unsigned int dbg_avoid);
EXTERN void DMDP_PHY_SetRFReg(unsigned int phy,RF92CD_RADIO_PATH_E eRFPath,unsigned int RegAddr,unsigned int BitMask,unsigned int Data);
EXTERN void clnt_load_IQK_res(struct rtl8192cd_priv * priv);
#endif

EXTERN int Load_92D_Firmware(struct rtl8192cd_priv *priv);
EXTERN void UpdateBBRFVal8192DE(struct rtl8192cd_priv *priv);
EXTERN void Update92DRFbyChannel(struct rtl8192cd_priv * priv,unsigned char channel);


EXTERN void SetSYN_para(struct rtl8192cd_priv *priv, unsigned char channel);
EXTERN void SetIMR_n(struct rtl8192cd_priv * priv, unsigned char channel);
EXTERN void IQK_92D_2G(struct rtl8192cd_priv * priv);
EXTERN void IQK_92D_5G_n(struct rtl8192cd_priv * priv);
#ifdef CONFIG_RTL_92D_DMDP
EXTERN void IQK_92D_5G_phy0_n(struct rtl8192cd_priv * priv);
#endif

#ifdef DPK_92D
EXTERN void rtl8192cd_DPK_timer(unsigned long task_priv);
EXTERN void PHY_DPCalibrate(struct rtl8192cd_priv *priv);
#endif

#undef EXTERN

#endif //CONFIG_RTL_92D_SUPPORT



/*-----------------------------------------------------------------------------
								8192cd_ioctl.c
------------------------------------------------------------------------------*/
#ifndef _8192CD_IOCTL_C_
#define EXTERN  extern
#else
#define EXTERN
#endif

#ifdef CONFIG_RTL_COMAPI_WLTOOLS

EXTERN const struct iw_handler_def rtl8192cd_iw_handler_def;

#endif

EXTERN int _convert_2_pwr_dot(char *s, int base);
EXTERN int _atoi(char *s, int base);
EXTERN void set_mib_default_tbl(struct rtl8192cd_priv *priv);
EXTERN int get_array_val (unsigned char *dst, char *src, int len);
EXTERN int del_sta(struct rtl8192cd_priv *priv, unsigned char *data);
EXTERN void drvmac_loopback(struct rtl8192cd_priv *priv);
EXTERN int rtl8192cd_ioctl(struct net_device *dev, struct ifreq *ifr, int cmd);
#ifdef	SUPPORT_TX_MCAST2UNI
EXTERN int ioctl_AddDelMCASTGroup2STA(struct net_device *dev, struct ifreq *ifr, int cmd);
#endif

#if defined(CONFIG_PCI_HCI)
#define ioctl_copy_from_user(to,from,n)	_ioctl_copy_from_user(priv,to,from,n,&flags)
#define ioctl_copy_to_user(to,from,n)		_ioctl_copy_to_user(priv,to,from,n,&flags)
#else
#define ioctl_copy_from_user(to,from,n)	copy_from_user(to,from,n)
#define ioctl_copy_to_user(to,from,n)		copy_to_user(to,from,n)
#endif

unsigned long _ioctl_copy_from_user (struct rtl8192cd_priv *priv, void * to, const void * from, unsigned long n, unsigned long *flags);
unsigned long _ioctl_copy_to_user (struct rtl8192cd_priv *priv, void * to, const void * from, unsigned long n, unsigned long *flags);


#ifdef INCLUDE_WPS
EXTERN	int set_mib(struct rtl8192cd_priv *priv, unsigned char *data);
EXTERN 	int get_mib(struct rtl8192cd_priv *priv, unsigned char *data);
#endif

#ifdef CONFIG_RTL8672
// MBSSID Port Mapping
struct port_map {
	struct net_device *dev_pointer;
	int dev_ifgrp_member;
};
#if !defined(__OSK__)
EXTERN void wlan_sta_state_notify(struct rtl8192cd_priv *priv, unsigned int join_res);
#endif
#endif

#ifdef CONFIG_RTL_COMAPI_WLTOOLS
EXTERN int set_mib(struct rtl8192cd_priv *priv, unsigned char *data);
EXTERN int rtl8192cd_ss_req(struct rtl8192cd_priv *priv, unsigned char *data, int len);
#endif
EXTERN void delay_us(unsigned int t);
EXTERN void delay_ms(unsigned int t);

#if defined(WIFI_WPAS) || defined(RTK_NL80211)
EXTERN int check_bss_encrypt(struct rtl8192cd_priv *priv);
#endif

#ifdef RTK_NL80211
EXTERN void rtl8192cd_init_one_cfg80211(struct rtknl *rtk);
#endif

EXTERN int rtl8192cd_autochannel_sel(struct rtl8192cd_priv *priv);

#undef EXTERN


int rtk_cfg80211_set_wps_p2p_ie(struct rtl8192cd_priv *priv, char *buf, int len, int mgmt_type)    ;
/*cfg p2p cfg p2p*/

#if 0	//move to hw.c
/*-----------------------------------------------------------------------------
								8190n_cam.c
------------------------------------------------------------------------------*/
#ifndef _8190N_CAM_C_
#define EXTERN  extern
#else
#define EXTERN
#endif

EXTERN int CamAddOneEntry(struct rtl8190_priv *priv, unsigned char *pucMacAddr,
				unsigned long ulKeyId, unsigned long ulEncAlg, unsigned long ulUseDK,
				unsigned char *pucKey);
EXTERN int CamDeleteOneEntry(struct rtl8190_priv *priv, unsigned char *pucMacAddr,
				unsigned long ulKeyId, unsigned int useDK);
EXTERN void CamResetAllEntry(struct rtl8190_priv *priv);
EXTERN void CamDumpAll(struct rtl8190_priv *priv);
EXTERN void CAM_read_entry(struct rtl8190_priv *priv, unsigned char index, unsigned char *macad,
				unsigned char *key128, unsigned short *config);

#undef EXTERN
#endif




/*-----------------------------------------------------------------------------
								8192cd_security.c
------------------------------------------------------------------------------*/
#ifndef _8192CD_SECURITY_C_
#define EXTERN  extern
#else
#define EXTERN
#endif

EXTERN int DOT11_Indicate_MIC_Failure(struct net_device *dev, struct stat_info *pstat);
EXTERN void DOT11_Indicate_MIC_Failure_Clnt(struct rtl8192cd_priv *priv, unsigned char *sa);

EXTERN void DOT11_Process_MIC_Timerup(unsigned long data);
EXTERN void DOT11_Process_Reject_Assoc_Timerup(unsigned long data);

EXTERN int rtl8192cd_ioctl_priv_daemonreq(struct net_device *dev, struct iw_point *data);
EXTERN int DOT11_Process_Delete_Key(struct net_device *dev, struct iw_point *data);
EXTERN int DOT11_Process_Set_Key(struct net_device *dev, struct iw_point *data,
				DOT11_SET_KEY *pSetKey, unsigned char *pKey);
#if defined(CONFIG_USB_HCI) || defined(CONFIG_SDIO_HCI)
EXTERN int __DOT11_Indicate_MIC_Failure(struct net_device *dev, struct stat_info *pstat);
EXTERN void __DOT11_Indicate_MIC_Failure_Clnt(struct rtl8192cd_priv *priv, unsigned char *sa);
EXTERN int __DOT11_Process_Disconnect_Req(struct net_device *dev, struct iw_point *data);
#endif // CONFIG_USB_HCI || CONFIG_SDIO_HCI


#undef EXTERN




/*-----------------------------------------------------------------------------
								8192cd_tkip.c
------------------------------------------------------------------------------*/
#ifndef _8192CD_TKIP_C_
#define EXTERN  extern
#else
#define EXTERN
#endif

EXTERN void tkip_icv(unsigned char *picv, unsigned char *frag1, unsigned int frag1_len,
				unsigned char *frag2,unsigned int frag2_len);
EXTERN void tkip_encrypt(struct rtl8192cd_priv *priv, unsigned char *pwlhdr,
				unsigned int hdrlen, unsigned char *frag1, unsigned int frag1_len,
				unsigned char *frag2, unsigned int frag2_len, unsigned char *frag3,
				unsigned int frag3_len);
EXTERN void michael(struct rtl8192cd_priv *priv, unsigned char *key, unsigned char *hdr,
				unsigned char *llc, unsigned char *message, int message_length, unsigned char *mic, int tx);
EXTERN unsigned int tkip_decrypt(struct rtl8192cd_priv *priv, struct rx_frinfo *pfrinfo,
				unsigned int fr_len);
EXTERN int tkip_rx_mic(struct rtl8192cd_priv *priv, unsigned char *pframe, unsigned char *da,
				unsigned char *sa, unsigned char priority, unsigned char *pbuf, unsigned int len,
				unsigned char *tkipmic, int no_wait);
EXTERN void debug_out(unsigned char *label, unsigned char *data, int data_length);
EXTERN unsigned int wep_decrypt(struct rtl8192cd_priv *priv, struct rx_frinfo *pfrinfo,
				unsigned int fr_len, int type, int keymap);
EXTERN void wep_encrypt(struct rtl8192cd_priv *priv, unsigned char *pwlhdr, unsigned int hdrlen,
				unsigned char *frag1, unsigned int frag1_len,
				unsigned char *frag2, unsigned int frag2_len,
				unsigned char *frag3, unsigned int frag3_len,
				int type);
EXTERN void init_crc32_table(void);

#undef EXTERN




/*-----------------------------------------------------------------------------
								8192cd_aes.c
------------------------------------------------------------------------------*/
#ifndef _8192CD_AES_C_
#define EXTERN  extern
#else
#define EXTERN
#endif

EXTERN void aesccmp_encrypt(struct rtl8192cd_priv *priv, unsigned char *pwlhdr,
				unsigned char *frag1,unsigned char *frag2, unsigned int frag2_len,
				unsigned char *frag3
#ifdef CONFIG_IEEE80211W				
				, unsigned char isMgmt
#endif
			);
EXTERN unsigned int aesccmp_decrypt(struct rtl8192cd_priv *priv, struct rx_frinfo *pfrinfo
#ifdef CONFIG_IEEE80211W				
				, unsigned char isMgmt
#endif	
			);

EXTERN int omac1_aes_128(const unsigned char *key, const unsigned char *data, 
				int data_len, unsigned char *mac);
EXTERN void sha256_prf(const unsigned char *key, size_t key_len, const char *label,
		const unsigned char *data, size_t data_len, unsigned char *buf, size_t buf_len);
EXTERN int hmac_sha256_vector(const unsigned char *key, size_t key_len, size_t num_elem,
			const unsigned char *addr[], const size_t *len, unsigned char *mac);
EXTERN int aesccmp_checkmic(struct rtl8192cd_priv *priv, struct rx_frinfo *pfrinfo, unsigned char *pmic);
#ifndef __ECOS
EXTERN int hmac_sha256(const unsigned char *key, size_t key_len, const unsigned char *data,
		 size_t data_len, unsigned char *mac);
#endif
#ifdef CONFIG_IEEE80211W
EXTERN void BIP_encrypt(struct rtl8192cd_priv *priv, unsigned char *pwlhdr,
				unsigned char *frag1,
				unsigned char *frag2, unsigned int frag2_len,
				unsigned char *frag3, 
				unsigned char isMgmt);
EXTERN int MMIE_check(struct rtl8192cd_priv *priv, struct rx_frinfo *pfrinfo);
EXTERN int issue_SA_Query_Rsp(struct net_device *dev, unsigned char *da, unsigned char *trans_id);
EXTERN int issue_SA_Query_Req(struct net_device *dev, unsigned char *da);
EXTERN void rtl8192cd_sa_query_timer(unsigned long task_priv);

#endif		

#undef EXTERN




/*-----------------------------------------------------------------------------
								8192cd_proc.c
------------------------------------------------------------------------------*/
#if !defined(__OSK__)
#ifndef _8192CD_PROC_C_
#define EXTERN  extern
#else
#define EXTERN
#endif

#ifdef _INCLUDE_PROC_FS_
EXTERN void rtl8192cd_proc_init (struct net_device *dev);
EXTERN void rtl8192cd_proc_remove (struct net_device *dev);
#endif

#if defined(_INCLUDE_PROC_FS_) || defined(__ECOS)
#ifdef __ECOS
EXTERN const char* MCS_DATA_RATEStr[2][2][24];
#else
EXTERN const unsigned char* MCS_DATA_RATEStr[2][2][24];
#endif
#endif

#ifdef RTK_AC_SUPPORT
EXTERN int query_vht_rate(struct stat_info *pstat);
#if (MU_BEAMFORMING_SUPPORT == 1)
EXTERN int query_mu_vht_rate(struct stat_info *pstat);
#endif
#endif

#undef EXTERN
#endif  //!defined(__OSK__)



/*-----------------------------------------------------------------------------
								8192cd_br_ext.c
------------------------------------------------------------------------------*/
#ifndef _8192CD_BR_EXT_C_
#define EXTERN  extern
#else
#define EXTERN
#endif

#ifdef RTK_BR_EXT
EXTERN void nat25_db_cleanup(struct rtl8192cd_priv *priv);
EXTERN void nat25_db_expire(struct rtl8192cd_priv *priv);
EXTERN int nat25_db_handle(struct rtl8192cd_priv *priv,	struct sk_buff *skb, int method);
EXTERN int nat25_handle_frame(struct rtl8192cd_priv *priv, struct sk_buff *skb);
EXTERN int mac_clone_handle_frame(struct rtl8192cd_priv *priv, struct sk_buff *skb);
EXTERN void dhcp_flag_bcast(struct rtl8192cd_priv *priv, struct sk_buff *skb);
EXTERN void dhcp_dst_bcast(struct rtl8192cd_priv * priv,struct sk_buff * skb);
EXTERN void *scdb_findEntry(struct rtl8192cd_priv *priv, unsigned char *macAddr, unsigned char *ipAddr);
EXTERN void nat25_filter_default(struct rtl8192cd_priv *priv);
EXTERN unsigned char nat25_filter(struct rtl8192cd_priv *priv, struct sk_buff *skb);
#ifdef __ECOS
EXTERN void nat25_db_init(void);
#endif
#ifdef MULTI_MAC_CLONE
EXTERN int mclone_find_address(struct rtl8192cd_priv *priv, unsigned char *addr, struct sk_buff *pskb, unsigned char direction);
EXTERN int mclone_dhcp_caddr(struct rtl8192cd_priv *priv, struct sk_buff *skb);
#endif
#endif //RTK_BR_EXT

#undef EXTERN




/*-----------------------------------------------------------------------------
								8192cd_eeprom.c
------------------------------------------------------------------------------*/
#ifndef _8192CD_EEPROM_C_
#define EXTERN  extern
#else
#define EXTERN
#endif
/*
EXTERN int ReadAdapterInfo(struct rtl8192cd_priv *priv, int entry_id, void *data);
EXTERN int WriteAdapterInfo(struct rtl8192cd_priv *priv, int entry_id, void *data);
*/
#undef EXTERN




/*-----------------------------------------------------------------------------
								8192cd_osdep.c
------------------------------------------------------------------------------*/
#ifndef _8192CD_OSDEP_C_
#define EXTERN  extern
#else
#define EXTERN
#endif

EXTERN int  rtl8192cd_set_hwaddr(struct net_device *dev, void *addr);
EXTERN int rtl8192cd_open (struct net_device *dev);
EXTERN int rtl8192cd_close(struct net_device *dev);
EXTERN void update_fwtbl_asoclst(struct rtl8192cd_priv *priv, struct stat_info *pstat);
EXTERN void clear_shortcut_cache(void);
#ifdef CONFIG_WLAN_HAL
EXTERN BOOLEAN Wlan_HAL_Link(struct rtl8192cd_priv *priv);
#endif
#ifdef WDS
#ifdef LAZY_WDS
EXTERN void delete_wds_entry(struct rtl8192cd_priv *priv, struct stat_info *pstat);
#endif
EXTERN struct stat_info *add_wds_entry(struct rtl8192cd_priv *priv, int idx, unsigned char *mac);
#endif

EXTERN int get_bonding_type_8881A(void);

EXTERN void rtl8192cd_set_mbssid(struct rtl8192cd_priv *priv, unsigned char *macAddr, unsigned char index);
EXTERN void rtl8192cd_clear_mbssid(struct rtl8192cd_priv *priv, unsigned char index);
#ifdef MULTI_MAC_CLONE
EXTERN void mclone_set_mbssid(struct rtl8192cd_priv *priv, unsigned char *macAddr);
EXTERN void mclone_stop_mbssid(struct rtl8192cd_priv *priv, int entIdx);
#endif

#ifdef TPT_THREAD
EXTERN int kTPT_thread(void *p);
EXTERN int kTPT_task_init(struct rtl8192cd_priv *priv);
EXTERN void kTPT_task_stop(struct rtl8192cd_priv *priv);
#endif

#ifdef BR_SHORTCUT
#ifdef RTL_CACHED_BR_STA
EXTERN void release_brsc_cache(unsigned char* da);
#endif
EXTERN __MIPS16 __IRAM_IN_865X struct net_device *get_shortcut_dev(unsigned char *da);
#endif // BR_SHORTCUT

#undef EXTERN




/*-----------------------------------------------------------------------------
								8192cd_host.c
------------------------------------------------------------------------------*/
#ifndef _8192CD_HOST_C_
#define EXTERN  extern
#else
#define EXTERN
#endif

EXTERN int PCIE_reset_procedure(int portnum, int ext_clk, int mdio_reset, unsigned long conf_addr);
EXTERN void HostPCIe_Close(void);

#undef EXTERN




/*-----------------------------------------------------------------------------
								8192cd_psk.c
------------------------------------------------------------------------------*/
#if defined(INCLUDE_WPA_PSK) || defined(WIFI_HAPD) || defined(RTK_NL80211)
#ifndef _8192CD_PSK_C_
#define EXTERN  extern
#else
#define EXTERN
#endif

EXTERN void ToDrv_SetIE(struct rtl8192cd_priv *priv); 
EXTERN void ConstructIE(struct rtl8192cd_priv *priv, unsigned char *pucOut, int *usOutLen);
EXTERN void derivePSK(struct rtl8192cd_priv *priv);
EXTERN void psk_init(struct rtl8192cd_priv *priv);
EXTERN int psk_indicate_evt(struct rtl8192cd_priv *priv, int id,
				unsigned char *mac, unsigned char *msg, int len);
EXTERN void ToDrv_SetGTK(struct rtl8192cd_priv *priv);
#ifdef WDS
EXTERN void wds_psk_init(struct rtl8192cd_priv *priv);
EXTERN void wds_psk_set(struct rtl8192cd_priv *priv, int idx, unsigned char *key);
#if defined(WIFI_HAPD) || defined(RTK_NL80211)
EXTERN void hapd_set_wdskey(struct net_device *dev, char *wdsPskPassPhrase, char *ssid, int wds_num);
#endif
#endif
#ifdef CLIENT_MODE
EXTERN void ClientSendEAPOL(struct rtl8192cd_priv *priv, struct stat_info *pstat, int resend);
#ifdef SUPPORT_CLIENT_MIXED_SECURITY
EXTERN void choose_cipher(struct rtl8192cd_priv *priv, struct bss_desc *bss_target);
#endif
#endif //CLIENT_MODE

EXTERN void UKRekeyTimeout(struct rtl8192cd_priv *priv, struct stat_info *pstat);
#endif // INCLUDE_WPA_PSK

#ifdef CONFIG_RTK_MESH
EXTERN void dot11s_mp_set_key(DRV_PRIV *priv, unsigned char *mac);
#endif

#ifdef CONFIG_IEEE80211R
EXTERN int derive_ft_pmk_r1(struct r0_key_holder *r0kh, unsigned char *s1kh_id, unsigned char *r1kh_id, unsigned char *pmk, unsigned char *pmkid);
EXTERN void derive_ft_pmk_r1_id(struct rtl8192cd_priv *priv, struct stat_info *pstat, unsigned char *pmk_r0_id, unsigned char *pmk_r1_id);
EXTERN int ft_check_imd_assoc(struct rtl8192cd_priv *priv, struct stat_info *pstat, unsigned char *pbuf, unsigned int limit, unsigned int *status);
EXTERN int ft_check_imd_4way(struct rtl8192cd_priv *priv, struct stat_info *pstat, unsigned char *pbuf, unsigned int limit, unsigned int *status);
EXTERN int ft_check_ft_auth(struct rtl8192cd_priv *priv, struct stat_info *pstat, unsigned char *pbuf, unsigned int limit, unsigned int *status);
EXTERN int ft_check_ft_auth_rrq(struct rtl8192cd_priv *priv, struct stat_info *pstat, unsigned char *pbuf, unsigned int limit, unsigned int *status);
EXTERN int ft_check_ft_assoc(struct rtl8192cd_priv *priv, struct stat_info *pstat, unsigned char *pbuf, unsigned int limit, unsigned int *status);
EXTERN void calc_ft_mic(struct rtl8192cd_priv *priv, struct stat_info *pstat, unsigned char *rsnie, unsigned int rsnie_len, 
	unsigned char *mdie, unsigned int mdie_len, unsigned char *ftie, unsigned int ftie_len, unsigned int seq);
EXTERN void derive_ft_keys(struct rtl8192cd_priv *priv, struct stat_info *pstat);
EXTERN void install_ft_keys(struct rtl8192cd_priv *priv, struct stat_info *pstat);
EXTERN void FT_IndicateEvent(struct rtl8192cd_priv *priv, struct stat_info *pstat, unsigned char event, void *data);
EXTERN void ft_enc_gtk(struct rtl8192cd_priv *priv, struct stat_info *pstat, unsigned char *out, unsigned short *outlen);
EXTERN void ft_init_1x(struct rtl8192cd_priv *priv, struct stat_info *pstat);
#endif

#undef EXTERN




/*-----------------------------------------------------------------------------
								8192cd_mp.c
------------------------------------------------------------------------------*/
#ifndef _8192CD_MP_C_
#define EXTERN  extern
#else
#define EXTERN
#endif

EXTERN char *get_value_by_token(char *data, char *token);
EXTERN void mp_start_test(struct rtl8192cd_priv *priv);
EXTERN void mp_stop_test(struct rtl8192cd_priv *priv);
EXTERN void mp_set_datarate(struct rtl8192cd_priv *priv, unsigned char *data);
EXTERN void mp_set_channel(struct rtl8192cd_priv *priv, unsigned char *data);
#ifdef CONFIG_RF_DPK_SETTING_SUPPORT
EXTERN int mp_set_dpk(struct rtl8192cd_priv *priv, unsigned char *data);
EXTERN void mp_tx_dpk_tracking(struct rtl8192cd_priv *priv, unsigned char *data);
#endif
EXTERN void mp_set_bandwidth(struct rtl8192cd_priv *priv, unsigned char *data);
EXTERN void mp_set_tx_power(struct rtl8192cd_priv *priv, unsigned char *data);
EXTERN void mp_ctx(struct rtl8192cd_priv *priv, unsigned char *data);
EXTERN int mp_query_stats(struct rtl8192cd_priv *priv, unsigned char *data);
#if (IC_LEVEL >= LEVEL_8814) || (IC_LEVEL == LEVEL_92E)
EXTERN void mp_set_tmac_tx(struct rtl8192cd_priv *priv);
EXTERN void mp_pmac_tx(struct rtl8192cd_priv *priv, unsigned char *data);
EXTERN void mp_reset_rx_mac(struct rtl8192cd_priv *priv);
EXTERN void mp_reset_rx_phy(struct rtl8192cd_priv *priv);
EXTERN void mp_reset_rx_macphy(struct rtl8192cd_priv *priv);
#endif
#if (IC_LEVEL >= LEVEL_8814)
EXTERN void mp_IQCalibrate(struct rtl8192cd_priv *priv);
EXTERN void mp_LCCalibrate(struct rtl8192cd_priv *priv);
EXTERN void mp_cal_rx_mac(struct rtl8192cd_priv *priv);
EXTERN void mp_cal_rx_phy(struct rtl8192cd_priv *priv);
EXTERN int  mp_query_rx_macphy(struct rtl8192cd_priv *priv, unsigned char *data);
EXTERN void mp_set_rx_gain(struct rtl8192cd_priv *priv, unsigned char *data);
#endif
EXTERN void mp_txpower_tracking(struct rtl8192cd_priv *priv, unsigned char *data);
EXTERN int mp_query_tssi(struct rtl8192cd_priv *priv, unsigned char *data);
EXTERN int mp_query_ther(struct rtl8192cd_priv *priv, unsigned char *data);
EXTERN int mp_query_rssi(struct rtl8192cd_priv *priv, unsigned char *data);
#ifdef MP_PSD_SUPPORT
EXTERN int mp_query_psd(struct rtl8192cd_priv *priv, unsigned char * data);
#endif
EXTERN int mp_tx(struct rtl8192cd_priv *priv, unsigned char *data);
EXTERN void mp_validate_rx_packet(struct rtl8192cd_priv *priv, unsigned char *data, int len);
EXTERN int mp_brx(struct rtl8192cd_priv *priv, unsigned char *data);
EXTERN int mp_arx(struct rtl8192cd_priv *priv, unsigned char *data);
EXTERN void mp_set_bssid(struct rtl8192cd_priv *priv, unsigned char *data);
EXTERN void mp_set_ant_tx(struct rtl8192cd_priv *priv, unsigned char *data);
EXTERN void mp_set_ant_rx(struct rtl8192cd_priv *priv, unsigned char *data);
EXTERN void mp_set_phypara(struct rtl8192cd_priv *priv, unsigned char *data);

#if defined(CONFIG_RTL_92D_SUPPORT) || defined(CONFIG_RTL_8812_SUPPORT) || defined(CONFIG_WLAN_HAL_8881A)
EXTERN void mp_set_phyBand(struct rtl8192cd_priv * priv, unsigned char * data);
#endif

EXTERN void mp_reset_stats(struct rtl8192cd_priv * priv);
EXTERN int mp_get_txpwr(struct rtl8192cd_priv *priv, unsigned char *data);
#if defined(CONFIG_RTL_8812_SUPPORT) || defined(CONFIG_WLAN_HAL_8814AE)
EXTERN void mp_dig(struct rtl8192cd_priv *priv, unsigned char *data);
#endif

#ifdef POWER_TRIM
EXTERN void do_kfree(struct rtl8192cd_priv *priv, unsigned char *data);
#endif


#if defined(CONFIG_WLAN_HAL_8814AE)
EXTERN int mp_version(struct rtl8192cd_priv *priv, unsigned char *data);
#endif
EXTERN void mp_help(struct rtl8192cd_priv *priv);

#undef EXTERN




/*-----------------------------------------------------------------------------
								8192cd_mib.c
------------------------------------------------------------------------------*/
#ifdef SUPPORT_SNMP_MIB

#ifndef _8192CD_MIB_C_
#define EXTERN  extern
#else
#define EXTERN
#endif

EXTERN int mib_get(struct rtl8192cd_priv *priv, char *oid, unsigned char *data, int *pLen);
EXTERN void mib_init(struct rtl8192cd_priv *priv);

#undef EXTERN

#endif // SUPPORT_SNMP_MIB




/*-----------------------------------------------------------------------------
								8192cd_comapi.c
------------------------------------------------------------------------------*/

#ifndef _8192CD_COMAPI_C_
#define EXTERN  extern
#else
#define EXTERN
#endif

#ifdef CONFIG_RTL_COMAPI_WLTOOLS
#include <net/iw_handler.h>
/*
EXTERN struct iw_statistics *rtl8192cd_get_wireless_stats(struct net_device *net_dev);
*/
#ifdef WIFI_WPAS_CLI
EXTERN int rtl_wx_dummy(struct net_device *dev, struct iw_request_info *info, union iwreq_data *wrqu, char *extra);
EXTERN int rtl_wx_get_name(struct net_device *dev, struct iw_request_info *info, union iwreq_data *wrqu, char *extra);
EXTERN int rtl_wx_set_mode(struct net_device *dev, struct iw_request_info *info, union iwreq_data *wrqu, char *extra);
EXTERN int rtl_wx_get_sens(struct net_device *dev, struct iw_request_info *info, union iwreq_data *wrqu, char *extra);
#if WIRELESS_EXT >= 18
EXTERN int rtl_wx_set_mlme(struct net_device *dev, struct iw_request_info *info, struct iw_point *erq, char *extra);
#endif
EXTERN int rtl_wx_get_freq(struct net_device *dev, struct iw_request_info *info, union iwreq_data *wrqu, char *extra);

EXTERN int rtl_gipriv(struct net_device *dev, struct iw_request_info *info, union iwreq_data *wrqu, char *extra);
EXTERN int rtl_sipriv(struct net_device *dev, struct iw_request_info *info, union iwreq_data *wrqu, char *extra);
#endif // WIFI_WPAS_CLI

EXTERN int rtl_siwfreq(struct net_device *dev, struct iw_request_info *info, union iwreq_data *wrqu, char *extra);
EXTERN int rtl_giwfreq(struct net_device *dev, struct iw_request_info *info, union iwreq_data *wrqu, char *extra);
EXTERN int rtl_siwmode(struct net_device *dev, struct iw_request_info *info, union iwreq_data *wrqu, char *extra);
EXTERN int rtl_giwmode(struct net_device *dev, struct iw_request_info *info, union iwreq_data *wrqu, char *extra);
EXTERN int rtl_giwrange(struct net_device *dev, struct iw_request_info *info, union iwreq_data *wrqu, char *extra);
EXTERN int rtl_siwap(struct net_device *dev, struct iw_request_info *info, union iwreq_data *wrqu, char *extra);
EXTERN int rtl_giwap(struct net_device *dev, struct iw_request_info *info, union iwreq_data *wrqu, char *extra);
EXTERN int rtl_iwaplist(struct net_device *dev, struct iw_request_info *info, struct iw_point *data, char *extra);
EXTERN int rtl_siwessid(struct net_device *dev, struct iw_request_info *info, union iwreq_data *wrqu, char *essid);
EXTERN int rtl_giwessid(struct net_device *dev, struct iw_request_info *info, union iwreq_data *wrqu, char *essid);
EXTERN int rtl_siwrate(struct net_device *dev, struct iw_request_info *info, union iwreq_data *wrqu, char *extra);
EXTERN int rtl_giwrate(struct net_device *dev, struct iw_request_info *info, union iwreq_data *wrqu, char *extra);
EXTERN int rtl_siwrts(struct net_device *dev, struct iw_request_info *info, union iwreq_data *wrqu, char *extra);
EXTERN int rtl_giwrts(struct net_device *dev, struct iw_request_info *info, union iwreq_data *wrqu, char *extra);
EXTERN int rtl_siwfrag(struct net_device *dev, struct iw_request_info *info, union iwreq_data *wrqu, char *extra);
EXTERN int rtl_giwfrag(struct net_device *dev, struct iw_request_info *info, union iwreq_data *wrqu, char *extra);
EXTERN int rtl_siwretry(struct net_device *dev, struct iw_request_info *info, union iwreq_data *wrqu, char *extra);
EXTERN int rtl_giwretry(struct net_device *dev, struct iw_request_info *info, union iwreq_data *wrqu, char *extra);
EXTERN int rtl_siwencode(struct net_device *dev, struct iw_request_info *info, union iwreq_data *wrqu, char *keybuf);
EXTERN int rtl_giwencode(struct net_device *dev, struct iw_request_info *info, union iwreq_data *wrqu, char *key);
EXTERN int rtl_giwpower(struct net_device *dev, struct iw_request_info *info, union iwreq_data *wrqu, char *extra);
EXTERN int rtl_siwscan(struct net_device *dev, struct iw_request_info *info, union iwreq_data *wrqu, char *extra);
EXTERN int rtl_giwscan(struct net_device *dev, struct iw_request_info *info, union iwreq_data *wrqu, char *extra);

#ifdef WIFI_WPAS_CLI
EXTERN int rtl_siwgenie(struct net_device *dev, struct iw_request_info *info, union iwreq_data *wrqu, char *extra);
EXTERN int rtl_siwauth(struct net_device *dev, struct iw_request_info *info, union iwreq_data *wrqu, char *extra);
EXTERN int rtl_giwauth(struct net_device *dev, struct iw_request_info *info, union iwreq_data *wrqu, char *extra);
EXTERN int rtl_siwencodeext(struct net_device *dev, struct iw_request_info *info, union iwreq_data *wrqu, char *extra);
EXTERN int rtl_giwencodeext(struct net_device *ndev, struct iw_request_info *info, union iwreq_data *wrqu, char *extra);
EXTERN int rtl_siwpmkid(struct net_device *dev,	struct iw_request_info *info, union iwreq_data *wrqu, char *extra);
#endif // WIFI_WPAS_CLI

#endif

#ifdef CONFIG_RTL_COMAPI_CFGFILE

EXTERN int CfgFileProc(struct net_device *dev);
EXTERN int CfgFileRead(struct net_device *dev, char *buf);

#endif // CONFIG_RTL_COMAPI_CFGFILE

#undef EXTERN


#define EXTERN extern
#if defined(CONFIG_RTL_CUSTOM_PASSTHRU)
/*-----------------------------------------------------------------------------
								passthrough/8192cd_osdep.c
------------------------------------------------------------------------------*/
EXTERN int passThruStatusWlan;
EXTERN int passThruWanIdx;
#endif
EXTERN struct _device_info_ wlan_device[];
#undef EXTERN

#ifdef WIFI_WPAS

#define EXTERN extern
EXTERN int rtl_net80211_setoptie(struct net_device *dev, struct iw_request_info *info, union iwreq_data *wrqu, char *extra);
EXTERN int rtl_wpas_custom(struct net_device *dev, struct iw_request_info *info, union iwreq_data *wrqu, char *extra);

#undef EXTERN

#endif




#ifdef CONFIG_RTK_MESH
/*-----------------------------------------------------------------------------
								mesh_ext/mesh_proc.c
------------------------------------------------------------------------------*/
#ifndef _MESH_PROC_C_
#define EXTERN  extern
#else
#define EXTERN
#endif

#ifdef CONFIG_RTL_PROC_NEW
EXTERN int dump_mesh_one_mpflow_sta(struct seq_file *s, struct stat_info *pstat);
EXTERN int mesh_proc_flow_stats_read(struct seq_file *s, void *data);
EXTERN int mesh_assoc_mpinfo(struct seq_file *s, void *data);
#ifdef MESH_BOOTSEQ_AUTH
EXTERN int mesh_auth_mpinfo(struct seq_file *s, void *data);
#endif
EXTERN int mesh_unEstablish_mpinfo(struct seq_file *s, void *data);
EXTERN int mesh_pathsel_routetable_info(struct seq_file *s, void *data);
EXTERN int mesh_portal_table_info(struct seq_file *s, void *data);
EXTERN int mesh_stats(struct seq_file *s, void *data);
EXTERN int mesh_proxy_table_info(struct seq_file *s, void *data);
EXTERN int mesh_root_info(struct seq_file *s, void *data);
#ifdef MESH_USE_METRICOP
EXTERN int mesh_metric_r(struct seq_file *s, void *data);
#endif

#else

EXTERN int dump_mesh_one_mpflow_sta(struct stat_info *pstat, char *buf, char **start, off_t offset, int length, int *eof, void *data);
EXTERN int mesh_proc_flow_stats_read(char *buf, char **start, off_t offset, int length, int *eof, void *data);
EXTERN int mesh_assoc_mpinfo(char *buf, char **start, off_t offset, int length, int *eof, void *data);
#ifdef MESH_BOOTSEQ_AUTH
EXTERN int mesh_auth_mpinfo(char *buf, char **start, off_t offset, int length, int *eof, void *data);
#endif
EXTERN int mesh_unEstablish_mpinfo(char *buf, char **start, off_t offset, int length, int *eof, void *data);
EXTERN int mesh_pathsel_routetable_info(char *buf, char **start, off_t offset, int length, int *eof, void *data);
EXTERN int mesh_portal_table_info(char *buf, char **start, off_t offset, int length, int *eof, void *data);
EXTERN int mesh_stats(char *buf, char **start, off_t offset, int length, int *eof, void *data);
EXTERN int mesh_proxy_table_info(char *buf, char **start, off_t offset, int length, int *eof, void *data);
EXTERN int mesh_root_info(char *buf, char **start, off_t offset, int length, int *eof, void *data);
#ifdef MESH_USE_METRICOP
EXTERN int mesh_metric_r(char *buf, char **start, off_t offset, int length, int *eof, void *data);
#endif
#endif //CONFIG_RTL_PROC_NEW

EXTERN int mesh_proc_flow_stats_write(struct file *file, const char *buffer, unsigned long count, void *data);
#ifdef MESH_USE_METRICOP
EXTERN int mesh_metric_w (struct file *file, const char *buffer, unsigned long count, void *data);
#endif

#undef EXTERN




/*-----------------------------------------------------------------------------
								mesh_ext/mesh_route.c
------------------------------------------------------------------------------*/
#ifndef _MESH_ROUTE_C_
#define EXTERN  extern
#else
#define EXTERN
#endif

EXTERN void aodv_expire(void *task_priv);
EXTERN unsigned short chkMeshSeq(struct path_sel_entry *pPathselEntry, unsigned short seq);
EXTERN void GEN_PREQ_PACKET(char *targetMac, struct rtl8192cd_priv *priv);
EXTERN unsigned short getMeshSeq(struct rtl8192cd_priv *priv);
EXTERN unsigned short getMeshMCastSeq(struct rtl8192cd_priv *priv);
EXTERN unsigned short getMeshMulticastSeq(DRV_PRIV *priv);
EXTERN void init_mpp_pool(struct mpp_tb* pTB);
EXTERN void notifyPathSelection(struct rtl8192cd_priv *priv);
EXTERN int pathsel_modify_table_entry(struct rtl8192cd_priv *priv, struct path_sel_entry *pEntry);
#if defined(CONFIG_RTL_MESH_CROSSBAND)
EXTERN int sync_proxy_info(struct rtl8192cd_priv *priv,unsigned char *sta, unsigned char action);
#endif
EXTERN int set_metric_manually(struct rtl8192cd_priv *,unsigned char *);

#ifdef MESH_ROUTE_MAINTENANCE
EXTERN void route_maintenance(struct rtl8192cd_priv *priv);
#endif

#ifdef PU_STANDARD
EXTERN UINT8 getPUSeq(struct rtl8192cd_priv *priv);
#endif

EXTERN unsigned char* getMeshHeader(struct rtl8192cd_priv *priv, int wep_mode, unsigned char* pframe);
EXTERN int insert_PREQ_entry(unsigned char *targetMac, DRV_PRIV *priv);
#undef EXTERN




/*-----------------------------------------------------------------------------
								mesh_ext/mesh_rx.c
------------------------------------------------------------------------------*/
#ifndef _MESH_RX_C_
#define EXTERN  extern
#else
#define EXTERN
#endif

EXTERN int process_11s_datafrme(struct rtl8192cd_priv *priv, struct rx_frinfo *pfrinfo, struct stat_info *pstat);
EXTERN int __process_11s_datafrme(DRV_PRIV *priv, struct rx_frinfo *pfrinfo, struct stat_info *pstat, struct MESH_HDR *meshHdrPtr);
EXTERN int rx_dispatch_mesh(struct rtl8192cd_priv *priv, struct rx_frinfo *pfrinfo);
EXTERN int mesh_shortcut_update(DRV_PRIV *priv, struct rx_frinfo *pfrinfo, struct rx_sc_entry* rxsc, DRV_PRIV **dest_priv, unsigned char* meshNextHop,  unsigned char *to_mesh, struct MESH_HDR **meshHdrPt);
#undef EXTERN




/*-----------------------------------------------------------------------------
								mesh_ext/mesh_tx.c
------------------------------------------------------------------------------*/
#ifndef _MESH_TX_C_
#define EXTERN  extern
#else
#define EXTERN
#endif

EXTERN void do_aodv_routing(struct rtl8192cd_priv *priv, struct sk_buff *skb, unsigned char *Mesh_dest);
#if defined(MESH_TX_SHORTCUT)
EXTERN int mesh_txsc_decision(struct tx_insn* cfgNew, struct tx_insn* cfgOld);
#endif

EXTERN int mesh_xmit_video_stream(DRV_PRIV *priv, struct tx_insn* ptxinsn, struct sk_buff *skb, struct rx_frinfo *pfrinfo); 
EXTERN int mesh_xmit_multicast_to_portal(DRV_PRIV *priv, struct tx_insn* ptxinsn, struct sk_buff *skb, struct rx_frinfo *pfrinfo);
EXTERN int mesh_xmit_multicast_to_neighbor(DRV_PRIV *priv, struct tx_insn* ptxinsn, struct sk_buff *skb, struct rx_frinfo *pfrinfo);
__IRAM_IN_865X
EXTERN int mesh_start_xmit(struct sk_buff *skb, struct net_device *dev);
EXTERN int notify_path_found(unsigned char *destaddr, struct rtl8192cd_priv *priv);
#undef EXTERN




/*-----------------------------------------------------------------------------
								mesh_ext/mesh_sme.c
------------------------------------------------------------------------------*/
#ifndef _MESH_SME_C_
#define EXTERN  extern
#else
#define EXTERN
#endif
EXTERN void mesh_standalone_timer_expire(struct rtl8192cd_priv*);
EXTERN int init_mesh(struct rtl8192cd_priv *priv);
EXTERN unsigned int acl_query(struct rtl8192cd_priv *priv, unsigned char *sa);
EXTERN int close_MeshPeerLink(struct rtl8192cd_priv *priv, UINT8 *da);
EXTERN int is_11s_mgt_frame(int num, struct rtl8192cd_priv *priv, struct rx_frinfo *pfrinfo);
EXTERN unsigned int issue_assocreq_MP(struct rtl8192cd_priv *priv,  struct stat_info *pstat);
EXTERN void issue_assocrsp_MP(struct rtl8192cd_priv *priv, unsigned short status, struct stat_info *pstat, int pkt_type);
EXTERN void issue_disassoc_MP(struct rtl8192cd_priv * priv, struct stat_info * pstat, int reason, UINT8 peerLinkReason);
EXTERN void issue_probereq_MP(struct rtl8192cd_priv *priv, unsigned char *ssid, int ssid_len, unsigned char *da, int is_11s);

#ifdef MESH_BOOTSEQ_AUTH
EXTERN void mesh_auth_timer(unsigned long pVal);
#endif

EXTERN void mesh_expire(struct rtl8192cd_priv* priv);
EXTERN unsigned int mesh_ie_MeshID(struct rtl8192cd_priv *priv, UINT8 meshiearray[], UINT8 isWildcard);
EXTERN unsigned int mesh_ie_WLANMeshCAP(struct rtl8192cd_priv *priv, UINT8 meshiearray[]);
EXTERN unsigned int mesh_ie_OFDM(struct rtl8192cd_priv *priv, UINT8 meshiearray[]);
EXTERN unsigned int mesh_ie_MeshChannelSwitch(DRV_PRIV *priv, UINT8 meshiearray[]);
EXTERN unsigned int mesh_ie_ChannelSwitchAnnoun(DRV_PRIV *priv, UINT8 meshiearray[]);
EXTERN unsigned int mesh_ie_SecondaryChannelOffset(DRV_PRIV *priv, UINT8 meshiearray[]);
EXTERN void mesh_peer_link_timer(unsigned long pVal);
EXTERN unsigned int OnAssocReq_MP(struct rtl8192cd_priv *priv, struct rx_frinfo *pfrinfo);
EXTERN unsigned int OnAssocRsp_MP(struct rtl8192cd_priv *priv, struct rx_frinfo *pfrinfo);
EXTERN unsigned int OnBeacon_MP(struct rtl8192cd_priv *priv, struct rx_frinfo *pfrinfo);
EXTERN unsigned int OnDisassoc_MP(struct rtl8192cd_priv *priv, struct rx_frinfo *pfrinfo);
EXTERN unsigned int OnProbeReq_MP(struct rtl8192cd_priv *priv, struct rx_frinfo *pfrinfo);
EXTERN unsigned int OnProbeRsp_MP(struct rtl8192cd_priv *priv, struct rx_frinfo *pfrinfo);
EXTERN unsigned int OnPathSelectionManagFrame(struct rtl8192cd_priv *priv, struct rx_frinfo *pfrinfo, int Is_6Addr);
EXTERN void OnLocalLinkStateANNOU_MP(struct rtl8192cd_priv *priv, struct rx_frinfo *pfrinfo);
EXTERN int start_MeshPeerLink(struct rtl8192cd_priv *priv, struct rx_frinfo *pfrinfo, struct stat_info *pstat, UINT16 cap);
EXTERN int mesh_channel_switch_initiate(struct rtl8192cd_priv *priv);
#if defined(DFS) || defined(RTK_AC_SUPPORT)
EXTERN int mesh_DFS_switch_channel(struct rtl8192cd_priv *priv);
#endif
#ifdef PU_STANDARD
EXTERN void issue_proxyupdate_MP(struct rtl8192cd_priv *priv, struct proxyupdate_table_entry *);
EXTERN void  issue_proxyupdate_ADD(struct rtl8192cd_priv *, unsigned char *);
EXTERN void OnProxyUpdate_MP(struct rtl8192cd_priv *priv, struct rx_frinfo *pfrinfo);
EXTERN void OnProxyUpdateConfirm_MP(struct rtl8192cd_priv *priv, struct rx_frinfo *pfrinfo);
#endif

EXTERN int mesh_close(DRV_PRIV *priv);
#undef EXTERN




/*-----------------------------------------------------------------------------
								mesh_ext/mesh_util.c
------------------------------------------------------------------------------*/
#ifndef _MESH_UTIL_C_
#define EXTERN  extern
#else
#define EXTERN
#endif

EXTERN void mesh_cnt_ASSOC_PeerLink_CAP(struct rtl8192cd_priv *priv, struct stat_info *pstat, int act);
EXTERN void mesh_set_PeerLink_CAP(struct  rtl8192cd_priv *priv, UINT16 meshCapSetValue);
EXTERN int remove_proxy_owner(struct  rtl8192cd_priv *priv, unsigned char *owner);
EXTERN int remove_proxy_entry(DRV_PRIV *priv, unsigned char *owner, unsigned char* sta);
EXTERN int mesh_proxy_update(DRV_PRIV *priv, unsigned char* owner, unsigned char* sta);
EXTERN void mesh_proxy_insert(struct rtl8192cd_priv *priv, unsigned char *sta);
EXTERN void mac12_to_6(unsigned char *in, unsigned char *out);
EXTERN int clear_route_info(struct  rtl8192cd_priv *priv, unsigned char *delMAC);
#undef EXTERN




/*-----------------------------------------------------------------------------
								mesh_ext/mesh_security.c
------------------------------------------------------------------------------*/
#ifndef _MESH_SECURITY_C_
#define EXTERN  extern
#else
#define EXTERN
#endif

EXTERN int DOT11_EnQueue2(unsigned long task_priv, DOT11_QUEUE2 *q, unsigned char *item, int itemsize);
EXTERN int DOT11_DeQueue2(unsigned long task_priv, DOT11_QUEUE2 *q, unsigned char *item, int *itemsize);
EXTERN void DOT11_InitQueue2(DOT11_QUEUE2 * q, int szMaxItem, int szMaxData);

#undef EXTERN

#endif // CONFIG_RTK_MESH



/*-----------------------------------------------------------------------------
								8192cd_sta_control.c
------------------------------------------------------------------------------*/
#ifndef _8192CD_STA_CONTROL_C_
#define EXTERN  extern
#else
#define EXTERN
#endif

#ifdef STA_CONTROL
EXTERN void stactrl_init(struct rtl8192cd_priv *priv);
EXTERN void stactrl_deinit(struct rtl8192cd_priv *priv);
#if STA_CONTROL_ALGO == STA_CONTROL_ALGO3
EXTERN void stactrl_preferband_sta_add(struct rtl8192cd_priv *priv, unsigned char *mac, unsigned char rssi, unsigned int is_local);//20170105
EXTERN void stactrl_preferband_sta_del(struct rtl8192cd_priv *priv, unsigned char *mac);//20170105
EXTERN unsigned char stactrl_OnAssocReq(struct rtl8192cd_priv *priv, unsigned char *mac, int frame_type, unsigned char rssi, unsigned char * candidate);
#else
EXTERN void stactrl_preferband_sta_add(struct rtl8192cd_priv *priv, unsigned char *mac, unsigned char rssi);
EXTERN void stactrl_non_prefer_expire(unsigned long task_priv);
#endif
EXTERN void stactrl_expire(struct rtl8192cd_priv *priv);
EXTERN unsigned char stactrl_check_request(struct rtl8192cd_priv *priv, unsigned char *mac, int frame_type, unsigned char rssi);
#ifdef CONFIG_RTL_PROC_NEW
EXTERN int stactrl_info_read(struct seq_file *s, void *data);
#else
EXTERN int stactrl_info_read(char *buf, char **start, off_t offset, int length, int *eof, void *data);
#endif
EXTERN int stactrl_info_write(struct file *file, const char *buffer, unsigned long count, void *data);
#endif //STA_CONTROL
#undef EXTERN


/*-----------------------------------------------------------------------------
								8192cd_a4_sta.c
------------------------------------------------------------------------------*/
#ifndef _8192CD_A4_STA_C_
#define EXTERN  extern
#else
#define EXTERN
#endif

#ifdef A4_STA
EXTERN void a4_sta_update(struct rtl8192cd_priv *root_priv, struct rtl8192cd_priv *priv,  unsigned char *mac);
EXTERN void a4_sta_cleanup_all(struct rtl8192cd_priv *priv);
EXTERN void a4_sta_cleanup(struct rtl8192cd_priv *priv, struct stat_info * pstat);
EXTERN void a4_sta_expire(struct rtl8192cd_priv *priv);
EXTERN void a4_sta_add(struct rtl8192cd_priv *priv, struct stat_info *pstat, unsigned char *mac);
EXTERN void a4_sta_del(struct rtl8192cd_priv *priv,  unsigned char *mac);
EXTERN struct stat_info *a4_sta_lookup(struct rtl8192cd_priv *priv, unsigned char *mac);
EXTERN void add_a4_client(struct rtl8192cd_priv *priv, struct stat_info *pstat);
EXTERN unsigned char parse_a4_ie(struct rtl8192cd_priv *priv, unsigned char* p, int limit);
EXTERN void a4_tx_unknown_unicast(struct rtl8192cd_priv *priv, struct sk_buff *skb);
EXTERN unsigned char a4_tx_mcast_to_unicast(struct rtl8192cd_priv *priv, struct sk_buff *skb);
EXTERN int a4_rx_dispatch(struct rtl8192cd_priv *priv, struct rx_frinfo *pfrinfo
#ifdef MBSSID
                    ,int vap_idx
#endif
);
EXTERN struct stat_info * a4_rx_check_reuse_ap(struct rtl8192cd_priv *priv, struct stat_info *pstat, struct rx_frinfo *pfrinfo);
EXTERN unsigned char a4_rx_check_reuse(struct rtl8192cd_priv *priv, struct rx_frinfo *pfrinfo, unsigned char * myhwaddr);

#ifdef CONFIG_RTL_PROC_NEW
EXTERN int a4_dump_sta_info(struct seq_file *s, void *data);
#else
EXTERN int a4_dump_sta_info(char *buf, char **start, off_t offset, int length, int *eof, void *data);
#endif	
#endif //A4_STA

#if defined(TV_MODE) || defined(A4_STA)
EXTERN unsigned char * construct_ecm_tvm_ie(struct rtl8192cd_priv *priv, unsigned char	*pbuf, unsigned int *frlen, unsigned char mode);
#endif

#ifdef TV_MODE
EXTERN unsigned char * construct_tv_mode_ie(struct rtl8192cd_priv *priv, unsigned char	*pbuf, unsigned int *frlen);
EXTERN unsigned char parse_tv_mode_ie(struct rtl8192cd_priv *priv, unsigned char* p, int limit);
EXTERN void tv_mode_auto_support_check(struct rtl8192cd_priv *priv);
#ifdef SUPPORT_TX_MCAST2UNI
EXTERN unsigned char tv_mode_igmp_group_check(struct rtl8192cd_priv *priv, struct sk_buff *skb);
#endif
#endif //TV_MODE
#undef EXTERN




/*-----------------------------------------------------------------------------
								HALDM_OUTSRC.c
------------------------------------------------------------------------------*/
#ifndef _HALDM_COMMON_C_
#define EXTERN  extern
#else
#define EXTERN
#endif

// Rate Adaptive
EXTERN void check_RA_by_rssi(struct rtl8192cd_priv *priv, struct stat_info *pstat);
EXTERN void add_RATid(struct rtl8192cd_priv *priv, struct stat_info *pstat);

EXTERN int getIGIFor1RCCA(int value_IGI);
EXTERN void PHY_LCCalibrate(struct rtl8192cd_priv *priv);
EXTERN int get_CCK_swing_index(struct rtl8192cd_priv *priv);
EXTERN void set_CCK_swing_index(struct rtl8192cd_priv * priv,short CCK_index);
EXTERN void set_DIG_state(struct rtl8192cd_priv *priv, int state);
EXTERN void check_DIG_by_rssi(struct rtl8192cd_priv *priv, unsigned char rssi_strength);
EXTERN void DIG_for_site_survey(struct rtl8192cd_priv *priv, int do_ss);
EXTERN void check_EDCCA(struct rtl8192cd_priv * priv, short rssi);
EXTERN void rtl8192cd_Adaptivity(struct rtl8192cd_priv *priv, unsigned char IGI);
EXTERN void rtl8192cd_NHMBBInit(struct rtl8192cd_priv *priv);
EXTERN void rtl8192cd_AdaptivityInit(struct rtl8192cd_priv *priv);
EXTERN void rtl8192cd_CheckAdaptivity(struct rtl8192cd_priv *priv);

#ifndef CALIBRATE_BY_ODM
EXTERN void _PHY_SaveADDARegisters(struct rtl8192cd_priv *priv, unsigned int *ADDAReg,	unsigned int *ADDABackup, unsigned int RegisterNum);
EXTERN void _PHY_SetADDARegisters(struct rtl8192cd_priv *priv, unsigned int* ADDAReg,	unsigned int* ADDASettings, unsigned int RegisterNum);
EXTERN void _PHY_SaveMACRegisters(struct rtl8192cd_priv *priv, unsigned int *MACReg, unsigned int *MACBackup);
EXTERN void _PHY_ReloadADDARegisters(struct rtl8192cd_priv *priv, unsigned int *ADDAReg, unsigned int *ADDABackup, unsigned int RegiesterNum);
EXTERN void _PHY_ReloadMACRegisters(struct rtl8192cd_priv *priv,unsigned int *MACReg, unsigned int *MACBackup);
EXTERN void _PHY_MACSettingCalibration(struct rtl8192cd_priv *priv, unsigned int* MACReg, unsigned int* MACBackup);
EXTERN void _PHY_PathADDAOn(struct rtl8192cd_priv *priv, unsigned int* ADDAReg, char isPathAOn, char is2T);
#endif

// FA
EXTERN void FA_statistic(struct rtl8192cd_priv * priv);
#ifdef CONFIG_RTL_NEW_AUTOCH
EXTERN void _FA_statistic(struct rtl8192cd_priv* priv);
EXTERN void reset_FA_reg(struct rtl8192cd_priv* priv);
EXTERN void hold_CCA_FA_counter(struct rtl8192cd_priv* priv);
EXTERN void release_CCA_FA_counter(struct rtl8192cd_priv* priv);
#endif

#ifdef WIFI_WMM
EXTERN void check_NAV_prot_len(struct rtl8192cd_priv * priv, struct stat_info * pstat, unsigned int disassoc);
#endif

#if defined(DETECT_STA_EXISTANCE)
EXTERN void DetectSTAExistance(struct rtl8192cd_priv *priv, struct tx_rpt *report, struct stat_info *pstat );
EXTERN void RetryLimitRecovery(unsigned long task_priv);
EXTERN void LeavingSTA_RLCheck(struct rtl8192cd_priv *priv);
#endif


EXTERN void check_NBI_by_rssi(struct rtl8192cd_priv *priv, unsigned char rssi_strength);
EXTERN void NBI_filter_on(struct rtl8192cd_priv *priv);
EXTERN void NBI_filter_off(struct rtl8192cd_priv *priv);

#if defined(CONFIG_WLAN_HAL_8192EE) || defined(CONFIG_WLAN_HAL_8822BE)
EXTERN void RRSR_power_control_11n(struct rtl8192cd_priv *priv, int lower);
#endif

#ifdef CONFIG_WLAN_HAL_8814AE
EXTERN void RRSR_power_control_14(struct rtl8192cd_priv *priv, int lower);
#endif


#undef EXTERN



/*-----------------------------------------------------------------------------
								HAL_8192CDMl.c
------------------------------------------------------------------------------*/
#ifndef _HAL8192CDM_C_
#define EXTERN  extern
#else
#define EXTERN
#endif

// DIG
EXTERN void DIG_process(struct rtl8192cd_priv *priv);
#ifdef CONFIG_RTL_92D_SUPPORT
EXTERN void MP_DIG_process(unsigned long task_priv);
#endif
// Tx Power
#ifdef HIGH_POWER_EXT_PA
EXTERN void tx_power_control(struct rtl8192cd_priv *priv);
#endif
EXTERN void tx_power_tracking(struct rtl8192cd_priv *priv);

#ifdef CONFIG_RTL_92D_SUPPORT
EXTERN void tx_power_tracking_92D(struct rtl8192cd_priv * priv);
#endif

#ifdef CONFIG_RTL_88E_SUPPORT //for 88e tx power tracking

#ifndef CALIBRATE_BY_ODM
EXTERN void odm_TXPowerTrackingCallback_ThermalMeter_8188E(struct rtl8192cd_priv * priv);
#endif
#endif

// EDCA
#ifdef USE_OUT_SRC
EXTERN void EdcaParaInit(struct rtl8192cd_priv *priv);
EXTERN unsigned char *Get_Adaptivity_Version(void);
#endif 

#if !defined(USE_OUT_SRC) || defined(_OUTSRC_COEXIST)
EXTERN void init_EDCA_para(struct rtl8192cd_priv *priv, int mode);
EXTERN void choose_IOT_main_sta(struct rtl8192cd_priv *priv, struct stat_info *pstat);
EXTERN void rxBB_dm(struct rtl8192cd_priv *priv);
EXTERN void IOT_engine(struct rtl8192cd_priv *priv);
#endif



#ifdef WIFI_WMM
EXTERN void IOT_EDCA_switch(struct rtl8192cd_priv *priv, int mode, char enable);
#endif

#if defined(CONFIG_RTL_92D_SUPPORT) || defined(CONFIG_RTL_92C_SUPPORT)
EXTERN void check_txrate_by_reg(struct rtl8192cd_priv *priv, struct stat_info *pstat);
EXTERN void set_RATid_cmd(struct rtl8192cd_priv * priv, unsigned int macid, unsigned int rateid, unsigned int ratemask);
EXTERN void add_update_RATid(struct rtl8192cd_priv *priv, struct stat_info *pstat);
EXTERN void add_update_rssi(struct rtl8192cd_priv * priv, struct stat_info * pstat);
#ifdef CONFIG_PCI_HCI
EXTERN void add_RATid_timer(unsigned long task_priv);
EXTERN void add_rssi_timer(unsigned long task_priv);
#endif
#endif


// Antenna diversity
#ifdef SW_ANT_SWITCH
EXTERN void dm_SW_AntennaSwitchCallback(unsigned long task_priv) ;
EXTERN void dm_SWAW_RSSI_Check(struct rtl8192cd_priv *priv, struct rx_frinfo *pfrinfo);
EXTERN void dm_SW_AntennaSwitchInit(struct rtl8192cd_priv *priv);
EXTERN void dm_SW_AntennaSwitch(struct rtl8192cd_priv *priv, char Step);
#endif
#if defined(SW_ANT_SWITCH) || defined(HW_ANT_SWITCH)
EXTERN int diversity_antenna_select(struct rtl8192cd_priv *priv, unsigned char *data);
#endif
#if defined(HW_ANT_SWITCH)
EXTERN void dm_HW_AntennaSwitchInit(struct rtl8192cd_priv *priv);
EXTERN void setRxIdleAnt(struct rtl8192cd_priv *priv, char nextAnt) ;
EXTERN void dm_STA_Ant_Select(struct rtl8192cd_priv *priv, struct stat_info *pstat);
EXTERN void dm_HW_IdleAntennaSelect(struct rtl8192cd_priv *priv);
#endif

// Dynamic Noise Control
#if defined(CONFIG_RTL_92D_SUPPORT) && defined(CONFIG_RTL_NOISE_CONTROL)
EXTERN void dnc_timer(unsigned long task_priv);
#endif

// Leaving STA check
#if defined(TXREPORT)
EXTERN void DetectSTAExistance(struct rtl8192cd_priv *priv, struct tx_rpt *report, struct stat_info *pstat );
#ifdef CONFIG_WLAN_HAL
EXTERN void DetectSTAExistance88XX(struct rtl8192cd_priv *priv, struct tx_rpt *report, struct stat_info *pstat );
#endif

//EXTERN void RetryLimitRecovery(unsigned long task_priv);
//EXTERN void LeavingSTA_RLCheck(struct rtl8192cd_priv *priv);
#endif

/* Hotspot 2.0 Release 1 */
#ifdef HS2_SUPPORT
EXTERN int issue_GASrsp(struct rtl8192cd_priv *priv, DOT11_HS2_GAS_RSP *gas_rsp);
EXTERN int issue_WNM_Notify(struct rtl8192cd_priv *priv);
EXTERN int issue_WNM_Deauth_Req(struct rtl8192cd_priv *priv, unsigned char *da, unsigned char reason, unsigned short ReAuthDelay, unsigned char *URL);
EXTERN int issue_QoS_MAP_Configure(struct rtl8192cd_priv *priv, unsigned char *da, unsigned char indexQoSMAP);
EXTERN void rtl8192cd_disassoc_timer(unsigned long task_priv);
EXTERN int issue_BSS_TSM_req(struct rtl8192cd_priv *priv, DOT11_HS2_TSM_REQ *tsm_req);
#ifdef HS2_CLIENT_TEST
EXTERN int issue_BSS_TSM_query(struct rtl8192cd_priv *priv, unsigned char *list, unsigned char list_len);
#endif
#endif

#ifdef CONFIG_RTL_92C_SUPPORT
EXTERN void APK_MAIN(struct rtl8192cd_priv *priv, unsigned int is2T);
EXTERN void PHY_IQCalibrate_92C(struct rtl8192cd_priv *priv);
#endif

#ifdef CONFIG_RTL_92D_SUPPORT
#ifdef SW_LCK_92D
EXTERN void PHY_LCCalibrate_92D(struct rtl8192cd_priv *priv);
EXTERN void phy_ReloadLCKSetting(struct rtl8192cd_priv *priv);
#endif
#ifdef CONFIG_RTL_92D_DMDP
EXTERN void IQK_92D_2G_phy1(struct rtl8192cd_priv * priv);
#endif
#endif

EXTERN void CCK_CCA_dynamic_enhance(struct rtl8192cd_priv *priv, unsigned char rssi_strength);

#undef EXTERN




// P2P_SUPPORT
/*-----------------------------------------------------------------------------
								8192cd_p2p.c
------------------------------------------------------------------------------*/
#ifdef P2P_SUPPORT
#undef EXTERN
#ifndef _8192CD_P2P_C_
#define EXTERN  extern
#else
#define EXTERN
#endif


EXTERN int rtk_parse_ssid_psk_from_wsc(struct rtl8192cd_priv *priv , unsigned char *data);
//EXTERN void p2p_as_GC(struct rtl8192cd_priv *priv , char* SSIDin, char* PSKin );
EXTERN void p2p_as_GC(struct rtl8192cd_priv *priv , unsigned char* data); // 20160328

//EXTERN int rtk_p2p_enable(struct rtl8192cd_priv *priv, int type);
EXTERN int rtk_p2p_enable(struct rtl8192cd_priv *priv, int p2p_role , int p2p_type);

EXTERN int P2P_listen(struct rtl8192cd_priv* priv,unsigned char* data);
EXTERN unsigned char*	p2p_search_tag2(unsigned char *ie_start,int ie_len,unsigned char tag, int *out_len);// search before hton

EXTERN unsigned char*	p2p_search_tag(unsigned char *data_be_search, 	int data_len,unsigned char tag, int *out_len); // search after ntoh
	


/*cfg p2p cfg p2p*/
#ifdef CONFIG_P2P_RTK_SUPPORT
//EXTERN int rtk_p2p_ioctl_cmd(struct net_device *dev, struct ifreq *ifr, int cmd);
EXTERN void rtk_p2p_store_state(struct rtl8192cd_priv *priv);
EXTERN void rtk_p2p_restore_state(struct rtl8192cd_priv *priv);
EXTERN void realtek_cfg80211_RemainOnChExpire(unsigned long task_priv);
EXTERN void rtk_p2p_scan_deny_expire(unsigned long task_priv);
EXTERN void rtk_set_scan_deny(struct rtl8192cd_priv *priv,int denyms);
#endif

EXTERN int rtk_p2p_chk_role(struct rtl8192cd_priv *priv, int role_chk);
EXTERN int rtk_p2p_chk_state(struct rtl8192cd_priv *priv, int state_chk);
EXTERN void rtk_p2p_set_role(struct rtl8192cd_priv *priv, int role_set);
EXTERN void rtk_p2p_set_state(struct rtl8192cd_priv *priv, int state_set);
EXTERN int rtk_p2p_get_role(struct rtl8192cd_priv *priv);    
EXTERN int rtk_p2p_get_state(struct rtl8192cd_priv *priv);
/*cfg p2p cfg p2p*/

EXTERN void issue_probersp(struct rtl8192cd_priv *priv, unsigned char *da,
				UINT8 *ssid, int ssid_len, int set_privacy, UINT8 is_11b_only);

EXTERN unsigned char *get_ie(unsigned char *pbuf, int index, int *len, int limit);
EXTERN int process_p2p_cmd(struct rtl8192cd_priv *priv, unsigned char *data);
EXTERN int	P2P_on_public_action(struct rtl8192cd_priv *priv,struct rx_frinfo *pfrinfo);

EXTERN void P2P_on_probe_req(
	struct rtl8192cd_priv *priv, struct rx_frinfo *pfrinfo, 
	unsigned char *IEaddr, unsigned int IElen );

/*cfg p2p cfg p2p*/
extern int rtw_android_priv_cmd2(struct net_device *dev, struct ifreq *ifr, int cmd);
extern int rtw_android_priv_cmd(struct net_device *net, struct ifreq *ifr, int cmd);
//extern int rtw_android_priv_cmd2(struct rtl8192cd_priv *priv, unsigned char *data);
EXTERN void rtk_p2p_init(struct rtl8192cd_priv *priv);
EXTERN void P2P_1sec_timer(struct rtl8192cd_priv *priv);
EXTERN void P2P_search_timer(unsigned long task_priv);	
EXTERN void p2p_start_timer(unsigned long task_priv);
EXTERN void p2p_find_timer(unsigned long task_priv);
EXTERN int p2pcmd_discovery(struct rtl8192cd_priv *priv, unsigned char *data);
/*cfg p2p cfg p2p*/

EXTERN int req_p2p_wsc_confirm(struct rtl8192cd_priv *priv, unsigned char *data);
EXTERN int req_p2p_provision_req(struct rtl8192cd_priv *priv,unsigned char *data);
EXTERN int p2p_get_role_from_ie(struct rtl8192cd_priv *priv,unsigned char *p2p_ie ,int p2pIElen );/*cfg p2p cfg p2p*/
EXTERN int p2p_get_device_info(struct rtl8192cd_priv *priv, 
	unsigned char *p2p_ie ,int p2pIElen ,struct device_info_s* devinfo_ptr);
EXTERN int  p2p_get_GO_p2p_info(struct rtl8192cd_priv *priv, 
	unsigned char *p2p_ie ,int p2pIElen ,struct device_info_s* devinfo_ptr);
EXTERN void  p2p_get_GO_wsc_info(struct rtl8192cd_priv *priv, 
	unsigned char *wsc_ie ,int wscIElen ,struct device_info_s *devinfo_ptr);
EXTERN int p2p_build_beacon_ie(struct rtl8192cd_priv *priv, unsigned char *data);
EXTERN void P2P_on_assoc_req(struct rtl8192cd_priv *priv,
	unsigned char *IEaddr, unsigned int IElen ,unsigned char *sa);
EXTERN int p2pcmd_apply(struct rtl8192cd_priv *priv, unsigned char *data);
EXTERN void P2P_client_on_beacon(struct rtl8192cd_priv *priv,
	unsigned char *IEaddr, unsigned int IElen, int seq);
EXTERN void p2p_noa_timer(struct rtl8192cd_priv *priv);
EXTERN int P2P_on_action(struct rtl8192cd_priv *priv, struct rx_frinfo *pfrinfo);
EXTERN void p2p_client_remove(struct rtl8192cd_priv *priv , struct stat_info *pstat );
EXTERN int p2p_build_assocRsp_ie(struct rtl8192cd_priv *priv, unsigned char *data
		,unsigned char status);
EXTERN void P2P_on_assoc_rsp(struct rtl8192cd_priv *priv,unsigned char *sa);
EXTERN int P2P_filter_manage_ap(struct rtl8192cd_priv *priv,
		unsigned char *IEaddr, unsigned int IElen );
EXTERN void p2p_debug_out(unsigned char *label, unsigned char *data, int data_length);

#undef EXTERN

#endif




#ifdef CONFIG_RTL_88E_SUPPORT
/*-----------------------------------------------------------------------------
								HalPwrSeqCmd.c
------------------------------------------------------------------------------*/
#define EXTERN  extern
EXTERN unsigned int HalPwrSeqCmdParsing(struct rtl8192cd_priv *priv, unsigned char CutVersion, 
				unsigned char FabVersion, unsigned char InterfaceType, WLAN_PWR_CFG PwrSeqCmd[ ]);
#undef EXTERN




/*-----------------------------------------------------------------------------
								8188e_hw.c
------------------------------------------------------------------------------*/
#ifndef _8188E_HW_C_
#define EXTERN  extern
#else
#define EXTERN
#endif

EXTERN void RTL8188E_MACID_NOLINK(struct rtl8192cd_priv * priv, unsigned int nolink, unsigned int aid);
EXTERN void RTL8188E_MACID_PAUSE(struct rtl8192cd_priv * priv, unsigned int pause, unsigned int aid);
#if defined(CONFIG_USB_HCI) || defined(CONFIG_SDIO_HCI)
EXTERN void __RTL8188E_MACID_NOLINK(struct rtl8192cd_priv * priv, unsigned int nolink, unsigned int aid);
EXTERN void __RTL8188E_MACID_PAUSE(struct rtl8192cd_priv * priv, unsigned int pause, unsigned int aid);
#endif

#ifdef SUPPORT_RTL8188E_TC
EXTERN void check_RTL8188E_testChip(struct rtl8192cd_priv * priv);
#endif

#ifdef TXREPORT
EXTERN void RTL8188E_EnableTxReport(struct rtl8192cd_priv *priv);
EXTERN void RTL8188E_DisableTxReport(struct rtl8192cd_priv *priv);
EXTERN void RTL8188E_ResumeTxReport(struct rtl8192cd_priv *priv);
EXTERN void RTL8188E_SuspendTxReport(struct rtl8192cd_priv *priv);
EXTERN void RTL8188E_AssignTxReportMacId(struct rtl8192cd_priv *priv);
EXTERN void RTL8188E_TxReportHandler(struct rtl8192cd_priv *priv, struct sk_buff *pskb, unsigned int bitmapLowByte,
	unsigned int bitmapHighByte, struct rx_desc *pdesc);
EXTERN void RTL8188E_SetTxReportTimeByRA(struct rtl8192cd_priv *priv, int extend);
#endif // TXREPORT

#ifndef CALIBRATE_BY_ODM
EXTERN void PHY_IQCalibrate_8188E(struct rtl8192cd_priv *priv, char bReCovery);
#endif



#undef EXTERN




/*-----------------------------------------------------------------------------
								RateAdaptive.c
------------------------------------------------------------------------------*/
#define EXTERN  extern

#ifdef TXREPORT
EXTERN int RateAdaptiveInfoInit(PSTATION_RA_INFO  pRaInfo);
EXTERN int ARFBRefresh(struct rtl8192cd_priv *priv, PSTATION_RA_INFO  pRaInfo);
EXTERN void RateDecision(struct rtl8192cd_priv *priv, PSTATION_RA_INFO  pRaInfo);
#endif

#undef EXTERN
#endif


/*-----------------------------------------------------------------------------
								8192cd_11h.c
------------------------------------------------------------------------------*/
#ifndef _8192CD_11H_C_
#define EXTERN  extern
#else
#define EXTERN
#endif
EXTERN void check_country_channel_table(struct rtl8192cd_priv *priv);
EXTERN unsigned char search_country_txpower(struct rtl8192cd_priv *priv, unsigned char channel);
EXTERN unsigned char * construct_country_ie(struct rtl8192cd_priv *priv, unsigned char	*pbuf, unsigned int *frlen);
EXTERN unsigned char * construct_power_capability_ie(struct rtl8192cd_priv *priv, unsigned char	*pbuf, unsigned int *frlen);
EXTERN unsigned char * construct_supported_channel_ie(struct rtl8192cd_priv *priv, unsigned char	*pbuf, unsigned int *frlen);
EXTERN unsigned char * construct_TPC_report_ie(struct rtl8192cd_priv *priv, unsigned char	*pbuf, unsigned int *frlen);
EXTERN void issue_TPC_report(struct rtl8192cd_priv *priv, unsigned char *da, unsigned char dialog_token);

#undef EXTERN

#ifdef RTK_SMART_ROAMING

/*-----------------------------------------------------------------------------
								8192cd_smart_roaming.c
------------------------------------------------------------------------------*/
#ifndef _8192CD_SMART_ROAMING_C_
#define EXTERN  extern
#else
#define EXTERN
#endif

EXTERN void send_roam_info(unsigned long task_priv);
EXTERN void add_neighbor_unicast_sta(struct rtl8192cd_priv *priv,unsigned char* addr, unsigned char rssi);
EXTERN void clear_send_info(struct rtl8192cd_priv *priv);
EXTERN void construct_netlink_send(struct rtl8192cd_priv *priv);
EXTERN void notify_new_sta(struct rtl8192cd_priv *priv, unsigned char *mac, int type, unsigned char rssi);
EXTERN void timer_ready(struct rtl8192cd_priv *priv);
EXTERN void timer_del(struct rtl8192cd_priv *priv);
EXTERN int rtl_netlink_init(void);
EXTERN void rtl_netlink_exit(void);

EXTERN void smart_roaming_block_init(struct rtl8192cd_priv *priv);
EXTERN void smart_roaming_block_deinit(struct rtl8192cd_priv *priv);
EXTERN void smart_roaming_block_expire(struct rtl8192cd_priv *priv, unsigned char *mac);
EXTERN unsigned char smart_roaming_block_check_request(struct rtl8192cd_priv *priv, unsigned char *mac);
EXTERN void smart_roaming_block_add(struct rtl8192cd_priv *priv, unsigned char *mac);


#undef EXTERN

#endif //8192cd_smart_roaming.c

#ifdef DOT11K

/*-----------------------------------------------------------------------------
								dot11k/8192cd_11k.c
------------------------------------------------------------------------------*/
#ifndef _8192CD_11K_C_
#define EXTERN  extern
#else
#define EXTERN
#endif
EXTERN unsigned char * construct_rm_enable_cap_ie(struct rtl8192cd_priv *priv, unsigned char	*pbuf, unsigned int *frlen);
EXTERN void rm_do_next_measure(unsigned long task_priv);
EXTERN void rm_terminate(struct rtl8192cd_priv *priv);
EXTERN void rm_done(struct rtl8192cd_priv *priv, unsigned char measure_type, unsigned char check_result);
EXTERN void OnRadioMeasurementRequest(struct rtl8192cd_priv *priv, struct stat_info *pstat, unsigned char *pframe, int frame_len);
EXTERN void OnRadioMeasurementReport(struct rtl8192cd_priv *priv, struct stat_info *pstat, unsigned char *pframe, int frame_len);
#ifdef CLIENT_MODE
EXTERN unsigned char * construct_WFA_TPC_report_ie(struct rtl8192cd_priv *priv, unsigned char	*pbuf, unsigned int *frlen);
#endif
#undef EXTERN


/*-----------------------------------------------------------------------------
								dot11k/8192cd_11k_beacon.c
------------------------------------------------------------------------------*/
#ifndef _8192CD_11K_BEACON_C_
#define EXTERN  extern
#else
#define EXTERN
#endif
EXTERN unsigned char rm_get_op_class(struct rtl8192cd_priv *priv, unsigned char channel);
EXTERN int rm_parse_beacon_report(struct stat_info *pstat, unsigned char *pframe, int len, int frame_len);
EXTERN enum MEASUREMENT_RESULT rm_parse_beacon_request(struct rtl8192cd_priv *priv, unsigned char *pframe, int offset, int frame_len, unsigned short * interval);
EXTERN int rm_beacon_measurement_request(struct rtl8192cd_priv *priv, unsigned char *macaddr, struct dot11k_beacon_measurement_req* beacon_req);
EXTERN int rm_get_beacon_report(struct rtl8192cd_priv *priv, unsigned char *macaddr, unsigned char** result_buf2);
EXTERN void rm_beacon_expire(unsigned long task_pstat);
EXTERN int rm_collect_bss_info(struct rtl8192cd_priv *priv, struct rx_frinfo *pfrinfo);
EXTERN void rm_beacon_done(struct rtl8192cd_priv *priv);
EXTERN enum MEASUREMENT_RESULT rm_terminate_beacon_measure(struct rtl8192cd_priv *priv);
EXTERN enum MEASUREMENT_RESULT  rm_do_beacon_measure(struct rtl8192cd_priv *priv);
EXTERN unsigned char * construct_beacon_report_ie(struct rtl8192cd_priv *priv, unsigned char *pbuf, unsigned int *frlen, unsigned char token, unsigned char *full);
EXTERN unsigned char *construct_ap_channel_rep_ie(struct rtl8192cd_priv *priv, unsigned char	*pbuf, unsigned int *frlen);
#ifdef CONFIG_RTL_PROC_NEW
EXTERN int rtl8192cd_proc_ap_channel_report_read(struct seq_file *s, void *data);
#else
EXTERN int rtl8192cd_proc_ap_channel_report_read(char *buf, char **start, off_t offset, int length, int *eof, void *data);
#endif
#ifdef __ECOS
EXTERN int rtl8192cd_proc_ap_channel_report_write(char *tmp, void *data);
#else
EXTERN int rtl8192cd_proc_ap_channel_report_write(struct file *file, const char *buffer, unsigned long count, void *data);
#endif
#ifdef CLIENT_MODE
EXTERN void rm_parse_ap_channel_report(struct rtl8192cd_priv *priv, unsigned char *pframe, int frame_len);
#endif
#undef EXTERN



/*-----------------------------------------------------------------------------
								dot11k/8192cd_11k_link.c
------------------------------------------------------------------------------*/
#ifndef _8192CD_11K_LINK_C_
#define EXTERN  extern
#else
#define EXTERN
#endif
EXTERN int rm_link_measurement_request(struct rtl8192cd_priv *priv, unsigned char *macaddr);
EXTERN int rm_get_link_report(struct rtl8192cd_priv *priv, unsigned char *macaddr, unsigned char* result_buf);
EXTERN int OnLinkMeasurementRequest(struct rtl8192cd_priv *priv, struct stat_info *pstat, struct rx_frinfo *pfrinfo);
EXTERN void OnLinkMeasurementReport(struct rtl8192cd_priv *priv, struct stat_info *pstat, unsigned char *pframe);
#undef EXTERN


/*-----------------------------------------------------------------------------
								dot11k/8192cd_11k_neighbor.c
------------------------------------------------------------------------------*/
#ifndef _8192CD_11K_NEIGHBOR_C_
#define EXTERN  extern
#else
#define EXTERN
#endif
EXTERN void OnNeighborReportRequest(struct rtl8192cd_priv *priv, struct stat_info *pstat, unsigned char *pframe, int frame_len);
EXTERN int rm_construct_neighbor_report(struct rtl8192cd_priv *priv, struct rx_frinfo *pfrinfo, unsigned char channel);
#ifdef CLIENT_MODE
EXTERN void OnNeighborReportResponse(struct rtl8192cd_priv *priv, struct stat_info *pstat, unsigned char *pframe, int frame_len);
EXTERN int rm_neighbor_request(struct rtl8192cd_priv *priv, char *ssid);
EXTERN int rm_get_neighbor_report(struct rtl8192cd_priv *priv, unsigned char* result_buf);
#endif
#ifdef CONFIG_RTL_PROC_NEW
EXTERN int rtl8192cd_proc_neighbor_read(struct seq_file *s, void *data);
#else
EXTERN int rtl8192cd_proc_neighbor_read(char *buf, char **start, off_t offset, int length, int *eof, void *data);
#endif

#ifdef __ECOS
EXTERN int rtl8192cd_proc_neighbor_write(char *tmp, void *data)
#else
EXTERN int rtl8192cd_proc_neighbor_write(struct file *file, const char *buffer, unsigned long count, void *data);
#endif

#undef EXTERN

#endif //DOT11K


/*-----------------------------------------------------------------------------
								ODM
------------------------------------------------------------------------------*/

#ifdef USE_OUT_SRC
#include "phydm/phydm_precomp.h"
#endif


#if defined(CONFIG_RTL_8812_SUPPORT) || defined(CONFIG_WLAN_HAL_8881A) || defined(CONFIG_WLAN_HAL_8814AE) || defined(CONFIG_RTL_8723B_SUPPORT) || defined(CONFIG_WLAN_HAL_8822BE)

#define EXTERN  extern



#if defined(CONFIG_RTL_8812_SUPPORT) || defined(CONFIG_WLAN_HAL_8881A) || defined(CONFIG_RTL_8723B_SUPPORT)

/*-----------------------------------------------------------------------------
								HalPwrSeqCmd.c
------------------------------------------------------------------------------*/
#ifndef CONFIG_WLAN_HAL_8881A
EXTERN unsigned int HalPwrSeqCmdParsing(struct rtl8192cd_priv *priv, unsigned char CutVersion, 
				unsigned char FabVersion, unsigned char InterfaceType, WLAN_PWR_CFG PwrSeqCmd[ ]);
#endif //CONFIG_WLAN_HAL_8881A



/*-----------------------------------------------------------------------------
								8812_hw.c
------------------------------------------------------------------------------*/

EXTERN void UpdateBBRFVal8812(struct rtl8192cd_priv *priv, unsigned char channel);
EXTERN void SpurCheck8812(struct rtl8192cd_priv *priv, unsigned char ch_bw, unsigned char channel, unsigned int bandwidth);
EXTERN void PHY_SetOFDMTxPower_8812(struct rtl8192cd_priv *priv, unsigned char channel);
EXTERN void PHY_SetCCKTxPower_8812(struct rtl8192cd_priv *priv, unsigned char channel);
//EXTERN void UpdateHalMSRRPT8812(struct rtl8192cd_priv *priv, unsigned short	aid, unsigned char opmode);
EXTERN void UpdateHalMSRRPT8812(struct rtl8192cd_priv *priv, struct stat_info *pstat, unsigned char opmode);

EXTERN VOID UpdateHalRAMask8812(struct rtl8192cd_priv*,	struct stat_info*, u1Byte);
//EXTERN void check_txrate_by_reg_8812(struct rtl8192cd_priv *priv, struct stat_info *pstat);
EXTERN u1Byte FillH2CCmd8812(struct rtl8192cd_priv *, u1Byte , u4Byte ,	pu1Byte	);

EXTERN void odm_TXPowerTrackingCallback_ThermalMeter_8812E(struct rtl8192cd_priv * priv);
EXTERN int get_tx_tracking_index(struct rtl8192cd_priv *priv, int channel, int i, int delta, int is_decrease, int is_CCK);
EXTERN void requestTxReport_8812(struct rtl8192cd_priv *priv);
//EXTERN void C2H_isr_8812(struct rtl8192cd_priv *priv);
EXTERN VOID C2HPacketHandler_8812(struct rtl8192cd_priv *priv, pu1Byte, u1Byte);

#if (BEAMFORMING_SUPPORT == 1)
EXTERN void TXBF_timer_callback(unsigned long task_priv);
EXTERN VOID SetBeamformStatus8812(struct rtl8192cd_priv *priv, u1Byte	Idx	);
EXTERN VOID SetBeamformEnter8812(struct rtl8192cd_priv *priv, u1Byte Idx);

EXTERN VOID SetBeamformLeave8812(struct rtl8192cd_priv *priv, u1Byte Idx);

//EXTERN void C2H_isr8812(struct rtl8192cd_priv *priv) ;
EXTERN VOID Beamforming_NDPARate_8812(struct rtl8192cd_priv *priv, BOOLEAN mode, u1Byte BW, u1Byte rate);

EXTERN VOID HW_VAR_HW_REG_TIMER_START_8812(struct rtl8192cd_priv *priv);
EXTERN VOID HW_VAR_HW_REG_TIMER_INIT_8812(struct rtl8192cd_priv *priv, int t);
EXTERN VOID HW_VAR_HW_REG_TIMER_STOP_8812(struct rtl8192cd_priv *priv);
#endif



EXTERN VOID RTL8812_MACID_PAUSE(struct rtl8192cd_priv *priv, BOOLEAN bSleep, u4Byte aid);
#if (BEAMFORMING_SUPPORT == 1)
EXTERN VOID SetBeamformStatus8812(struct rtl8192cd_priv *priv, u1Byte Idx);
EXTERN VOID _Beamforming_CLK(struct rtl8192cd_priv *priv);
#endif

#endif //#if defined(CONFIG_RTL_8812_SUPPORT) || defined(CONFIG_WLAN_HAL_8881A)

EXTERN void SwitchChannel(struct rtl8192cd_priv *priv);


#undef EXTERN
#endif

/*-----------------------------------------------------------------------------
								8812_vht_gen.c
------------------------------------------------------------------------------*/
#ifdef RTK_AC_SUPPORT
#define EXTERN  extern
EXTERN	void construct_vht_ie(struct rtl8192cd_priv *priv, unsigned char channel_center);
#ifdef MCR_WIRELESS_EXTEND
EXTERN	void construct_vht_ie_mcr(struct rtl8192cd_priv *priv, unsigned char channel_center, struct stat_info *pstat);
#endif
#endif


#ifdef __ECOS
/*-----------------------------------------------------------------------------
								8192cd_ecos.c
------------------------------------------------------------------------------*/
#ifndef _8192CD_ECOS_C_
#define EXTERN  extern
#else
#define EXTERN
#endif

EXTERN void interrupt_dsr(struct net_device *dev);
EXTERN int can_xmit(struct net_device *dev);

#undef EXTERN

#endif // __ECOS

/*-----------------------------------------------------------------------------
								8192cd_config.c
------------------------------------------------------------------------------*/
#if defined(CONFIG_RTL_SIMPLE_CONFIG)

#ifndef _8192CD_ECOS_C_
#define EXTERN  extern
#else
#define EXTERN
#endif
EXTERN int get_sc_sync_state(void);
EXTERN int clean_sc_syn(void);
EXTERN int get_sc_syn_stats(void);
EXTERN int get_sc_smac(struct rx_frinfo *pfrinfo);
EXTERN int rtk_sc_init(struct rtl8192cd_priv *priv);
EXTERN int rtk_sc_start_simple_config(struct rtl8192cd_priv *priv);
EXTERN int rtk_sc_start_parse_packet(struct rtl8192cd_priv *priv, struct rx_frinfo *pfrinfo);
EXTERN int rtk_sc_check_security(struct rtl8192cd_priv *priv, struct bss_desc *bss);
EXTERN int rtk_sc_set_passwd(struct rtl8192cd_priv * priv);
EXTERN int rtk_sc_stop_simple_config(struct rtl8192cd_priv *priv);
EXTERN int rtk_sc_start_connect_target(struct rtl8192cd_priv *priv);
EXTERN int rtk_sc_restart_simple_config(struct rtl8192cd_priv *priv);
EXTERN int rtk_sc_sync_vxd_to_root(struct rtl8192cd_priv * priv);
EXTERN int rtk_sc_clean_profile_value();
EXTERN int rtk_sc_set_value(unsigned int id, unsigned int mib_value);
EXTERN int rtk_sc_get_value(unsigned int id);
EXTERN int rtk_sc_set_string_value(unsigned int id, unsigned char *mib_value);
EXTERN int rtk_sc_get_string_value(unsigned int id, unsigned char *mib_value);
EXTERN int rtk_sc_get_security_type();
EXTERN int rtk_sc_get_magic(unsigned int *magic, unsigned int len);
EXTERN int rtk_sc_set_para(struct rtl8192cd_priv *priv);
EXTERN int rtk_sc_is_channel_fixed(struct rtl8192cd_priv * priv);
EXTERN int rtk_sc_get_scan_offset(struct rtl8192cd_priv * priv, int channel, int bw);
EXTERN int rtk_sc_during_simple_config_scan(struct rtl8192cd_priv * priv);
EXTERN int get_sc_smac(struct rx_frinfo *pfrinfo);

#undef EXTERN

#endif


/*-----------------------------------------------------------------------------
								inline function
------------------------------------------------------------------------------*/

#if defined(CONFIG_USB_HCI) || defined(CONFIG_SDIO_HCI)
static inline void update_STA_RATid(struct rtl8192cd_priv *priv, struct stat_info *pstat)
{
	notify_update_sta_RATid(priv, pstat);
}
#endif

#if defined(CONFIG_RTL_819X_ECOS)
static inline void list_splice_init(struct list_head *list, struct list_head *head)
{
	if (!list_empty(list)) {
		struct list_head *first = list->next;
		struct list_head *last = list->prev;
		struct list_head *at = head->next;
		first->prev = head;
		head->next = first;
		last->next = at;
		at->prev = last;
		INIT_LIST_HEAD(list);
	}
}
#endif
#endif // _8192CD_HEADERS_H_

