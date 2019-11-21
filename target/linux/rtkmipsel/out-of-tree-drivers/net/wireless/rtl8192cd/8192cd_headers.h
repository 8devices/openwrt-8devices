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

#include <linux/wireless.h>


#include "./8192cd_tx.h"





#include "./WlanHAL/RTL88XX/RTL8197F/RTL8197FE/Hal8197FEDef.h"

#include "./WlanHAL/RTL88XX/RTL8822B/RTL8822BE/Hal8822BEDef.h"

#if (BEAMFORMING_SUPPORT == 1)
#include "Beamforming.h"
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

#define SPRINTT(buf, fmt, args...) panic_printk(fmt, ##args)

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
EXTERN void APReqTXRptHandler(struct rtl8192cd_priv *priv,unsigned char* C2HContent);
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
__MIPS16
__IRAM_IN_865X
struct stat_info *get_stainfo (struct rtl8192cd_priv *priv, unsigned char *hwaddr);
#ifdef HW_FILL_MACID
EXTERN
__MIPS16
__IRAM_IN_865X
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
__MIPS16
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
int UseSwCrypto(struct rtl8192cd_priv *priv, struct stat_info *pstat, int isMulticast
#ifdef CONFIG_IEEE80211W
		, int isPMF
#endif
	);


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
#ifdef WIFI_WPAS
EXTERN int event_indicate_wpas(struct rtl8192cd_priv *priv, unsigned char *mac, int event, unsigned char *extra);
#endif
#ifdef USE_WEP_DEFAULT_KEY
EXTERN void init_DefaultKey_Enc(struct rtl8192cd_priv *priv, unsigned char *key, int algorithm);
#endif
EXTERN int changePreamble(struct rtl8192cd_priv *priv, int preamble);
EXTERN int HideAP(struct rtl8192cd_priv *priv);
EXTERN int DehideAP(struct rtl8192cd_priv *priv);

EXTERN void disable_vxd_ap(struct rtl8192cd_priv *priv);
EXTERN void enable_vxd_ap(struct rtl8192cd_priv *priv);
EXTERN int under_apmode_repeater(struct rtl8192cd_priv *priv);
EXTERN int get_ss_level(struct rtl8192cd_priv *priv);
EXTERN int send_arp_response(struct rtl8192cd_priv *priv, unsigned int *dip, unsigned int *sip, unsigned char *dmac, unsigned char *smac);
EXTERN void snoop_STA_IP(struct sk_buff *pskb, struct rtl8192cd_priv *priv);
EXTERN int takeOverHidden(unsigned char active, struct rtl8192cd_priv *priv);
EXTERN int isHiddenAP(unsigned char *pframe, struct rx_frinfo *pfrinfo, struct stat_info *pstat, struct rtl8192cd_priv *priv);
EXTERN unsigned int isDHCPpkt(struct sk_buff *pskb);
EXTERN void syncMulticastCipher(struct rtl8192cd_priv *priv, struct bss_desc *bss_target);
#ifdef GBWC
EXTERN void rtl8192cd_GBWC_timer(unsigned long task_priv);
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
EXTERN void add_ps_timer(unsigned long task_priv);

EXTERN void add_update_ps(struct rtl8192cd_priv *priv, struct stat_info *pstat);
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

EXTERN void rtk_queue_init(struct ring_que *que);
EXTERN void free_rtk_queue(struct rtl8192cd_priv *priv, struct ring_que *skb_que);



EXTERN unsigned int orForce20_Switch20Map(struct rtl8192cd_priv * priv);
EXTERN unsigned int orSTABitMap(STA_BITMAP *map);
EXTERN unsigned char getSTABitMap(STA_BITMAP *map, int bitIdx);
EXTERN void setSTABitMap(STA_BITMAP *map, int bitIdx);
EXTERN void clearSTABitMap(STA_BITMAP *map, int bitIdx);


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



EXTERN unsigned char oui_rfc1042[];
EXTERN unsigned char oui_8021h[];
EXTERN unsigned char oui_cisco[];
EXTERN unsigned char SNAP_ETH_TYPE_IPX[];
EXTERN unsigned char SNAP_ETH_TYPE_APPLETALK_AARP[];
EXTERN unsigned char SNAP_ETH_TYPE_APPLETALK_DDP[];
EXTERN unsigned char SNAP_HDR_APPLETALK_DDP[];



EXTERN int rtl_string_to_hex(char *string, unsigned char *key, int len);


#if defined(TXREPORT)
EXTERN void requestTxReport(struct rtl8192cd_priv *priv);
EXTERN void C2H_isr(struct rtl8192cd_priv *priv);
#endif

EXTERN struct stat_info* findNextSTA(struct rtl8192cd_priv *priv, int *idx);


#ifdef TLN_STATS
EXTERN void stats_conn_rson_counts(struct rtl8192cd_priv * priv, unsigned int reason);
EXTERN void stats_conn_status_counts(struct rtl8192cd_priv * priv, unsigned int status);
#endif


#ifdef RF_MIMO_SWITCH
EXTERN void RF_MIMO_check_timer(unsigned long task_priv);
EXTERN int assign_MIMO_TR_Mode(struct rtl8192cd_priv *priv, unsigned char *data);
#endif

EXTERN void update_RAMask_to_FW(struct rtl8192cd_priv *priv, int legacy);

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


#if 0	//defined(HS2_SUPPORT) || defined(DOT11K) || defined(CH_LOAD_CAL) || defined(RTK_SMART_ROAMING)
EXTERN void rtl8192cd_cu_cntdwn_timer(unsigned long task_priv);
EXTERN void rtl8192cd_cu_start(struct rtl8192cd_priv *priv);
EXTERN void rtl8192cd_cu_stop(struct rtl8192cd_priv *priv);
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
__MIPS16 __IRAM_IN_865X
void assign_wlanseq(struct rtl8192cd_hw *phw, unsigned char *pframe, struct stat_info *pstat, struct wifi_mib *pmib
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
void rtl8192cd_tx_dsr(unsigned long task_priv);

EXTERN int rtl8192cd_firetx(struct rtl8192cd_priv *priv, struct tx_insn *txcfg);
EXTERN int __rtl8192cd_firetx(struct rtl8192cd_priv *priv, struct tx_insn* txcfg);
EXTERN void rtl8192cd_signin_txdesc(struct rtl8192cd_priv *priv, struct tx_insn* txcfg);

#ifdef TX_SHORTCUT
//EXTERN void signin_txdesc_shortcut(struct rtl8190_priv *priv, struct tx_insn *txcfg);
EXTERN
__MIPS16
__IRAM_IN_865X
void rtl88XX_signin_txdesc_shortcut(struct rtl8192cd_priv *priv, struct tx_insn *txcfg, int idx);

#ifdef HW_DETEC_POWER_STATE
EXTERN void detect_hw_pwr_state(struct rtl8192cd_priv *priv, unsigned char macIDGroup);
EXTERN __IRAM_IN_865X void pwr_state_enhaced(struct rtl8192cd_priv *priv, unsigned char macID, unsigned char PwrBit);
#endif
EXTERN
 __MIPS16
__IRAM_IN_865X
void rtl8192cd_signin_txdesc_shortcut(struct rtl8192cd_priv *priv, struct tx_insn *txcfg, int idx);
#endif

#ifdef CONFIG_8814_AP_MAC_VERI //eric-8822
void issue_Test_NullData(struct rtl8192cd_priv *priv,unsigned char macID,unsigned char type,unsigned char IstxToAP);
void RX_MAC_Verify_8814(struct rtl8192cd_priv *priv,unsigned char * pframe,struct rx_frinfo *pfrinfo);
#endif

EXTERN int SetupOneCmdPacket(struct rtl8192cd_priv *priv, unsigned char *dat_content,
				unsigned short txLength, unsigned char LastPkt);
EXTERN void amsdu_timeout(struct rtl8192cd_priv *priv, unsigned int current_time);

EXTERN int rtl8192cd_SetupOneCmdPacket(struct rtl8192cd_priv *priv, unsigned char *dat_content, unsigned short txLength, unsigned char LastPkt);

EXTERN
__MIPS16
__IRAM_IN_865X
int __rtl8192cd_start_xmit_out(struct sk_buff *skb, struct stat_info *pstat);

#if defined(DRVMAC_LB) && defined(WIFI_WMM)
EXTERN void SendLbQosData(struct rtl8192cd_priv *priv);
EXTERN void SendLbQosNullData(struct rtl8192cd_priv *priv);
#endif

EXTERN void wep_fill_iv(struct rtl8192cd_priv *priv, unsigned char *pwlhdr, unsigned int hdrlen, unsigned long keyid);
EXTERN void tkip_fill_encheader(struct rtl8192cd_priv *priv, unsigned char *pwlhdr, unsigned int hdrlen, unsigned long keyid_out);
EXTERN
__MIPS16
#ifndef WIFI_MIN_IMEM_USAGE
__IRAM_IN_865X
#endif
void aes_fill_encheader(struct rtl8192cd_priv *priv, unsigned char *pwlhdr, unsigned int hdrlen, unsigned long keyid);


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
EXTERN void mgt_handler(struct rtl8192cd_priv *priv, struct rx_frinfo *pfrinfo);
EXTERN void pwr_state(struct rtl8192cd_priv *priv, struct rx_frinfo *pfrinfo);
EXTERN void check_PS_set_HIQLMT(struct rtl8192cd_priv *priv);
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
EXTERN void rtl8192cd_dfs_cntdwn_timer(unsigned long task_priv);
EXTERN void issue_PsPoll(struct rtl8192cd_priv *priv);
#endif
void issue_NullData(struct rtl8192cd_priv *priv, unsigned char *da);

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



EXTERN void default_WMM_para(struct rtl8192cd_priv *priv);
EXTERN unsigned char *search_wsc_tag(unsigned char *data, unsigned short id, int len, int *out_len);


#if (defined( UNIVERSAL_REPEATER) || defined(MBSSID)) && defined(CLIENT_MODE)
//EXTERN void set_vxd_rescan(struct rtl8192cd_priv *priv);
EXTERN void set_vxd_rescan(struct rtl8192cd_priv *priv,int rescantype);
EXTERN void  switch_profile(struct rtl8192cd_priv *priv, int idx);
EXTERN void  sync_channel_2ndch_bw(struct rtl8192cd_priv *priv);
void start_repeater_ss(struct rtl8192cd_priv *priv);

EXTERN void construct_ibss_beacon(struct rtl8192cd_priv *priv);
void issue_beacon_ibss_vxd(unsigned long task_priv);
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
EXTERN int under_apmode_repeater(struct rtl8192cd_priv *priv);
EXTERN int should_defer_ss(struct rtl8192cd_priv *priv);

EXTERN void DFS_SwitchChannel(struct rtl8192cd_priv *priv);
#undef EXTERN




/*-----------------------------------------------------------------------------
								8192cd_dfs_det.c
------------------------------------------------------------------------------*/
#ifndef _8192CD_DFS_DET_C_
#define EXTERN	extern
#else
#define EXTERN
#endif

EXTERN void rtl8192cd_dfs_det_chk(struct rtl8192cd_priv *priv);
EXTERN void rtl8192cd_dfs_dynamic_setting(struct rtl8192cd_priv *priv);
EXTERN void rtl8192cd_radar_type_differentiation(struct rtl8192cd_priv *priv);

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
__MIPS16
__IRAM_IN_865X
int validate_mpdu(struct rtl8192cd_priv *priv, struct rx_frinfo *pfrinfo);

EXTERN
__IRAM_IN_865X
void rtl88XX_rx_isr(struct rtl8192cd_priv *priv);

EXTERN void rtl8192cd_rx_tkl_isr(unsigned long task_priv);

EXTERN
__IRAM_IN_865X
void rtl8192cd_rx_isr(struct rtl8192cd_priv *priv);
EXTERN __IRAM_IN_865X void rtl8192cd_rx_dsr(unsigned long task_priv);
EXTERN
__IRAM_IN_865X
void rtl_netif_rx(struct rtl8192cd_priv *priv, struct sk_buff *pskb, struct stat_info *pstat);
//EXTERN void rtl8190_rxcmd_isr(struct rtl8190_priv *priv);
EXTERN
#if   !defined(WIFI_MIN_IMEM_USAGE)
__IRAM_IN_865X
#endif
void reorder_ctrl_timeout(unsigned long task_priv);
EXTERN
#ifndef WIFI_MIN_IMEM_USAGE
__IRAM_IN_865X
#endif
void reorder_ctrl_timeout_cli(unsigned long task_priv);

EXTERN void rtl8192cd_rx_mgntframe(struct rtl8192cd_priv*, struct list_head *,struct rx_frinfo*);

EXTERN
__MIPS16
#ifndef WIFI_MIN_IMEM_USAGE
__IRAM_IN_865X
#endif
void reorder_ctrl_consumeQ(struct rtl8192cd_priv *priv, struct stat_info *pstat, unsigned char tid, int seg);

EXTERN
__MIPS16
__IRAM_IN_865X
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
EXTERN BOOLEAN compareAvailableTXBD(struct rtl8192cd_priv * priv, unsigned int num, unsigned int qNum, int compareFlag);
EXTERN void check_chipID_MIMO(struct rtl8192cd_priv *priv);
EXTERN void TxACurrentCalibration(struct rtl8192cd_priv *priv);
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

//yllin
#ifdef GPIO_WAKEPIN
EXTERN int request_irq_for_wakeup_pin_V2(struct net_device *dev);
#endif

EXTERN void set_slot_time(struct rtl8192cd_priv *priv, int use_short);
EXTERN void SetTxPowerLevel(struct rtl8192cd_priv *priv, unsigned char channel);
EXTERN void SwChnl(struct rtl8192cd_priv *priv, unsigned char channel, int offset);
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
EXTERN int rtl8192cd_init_hw_PCI(struct rtl8192cd_priv *priv);
EXTERN int rtl8192cd_stop_hw(struct rtl8192cd_priv *priv);
EXTERN int check_MAC_IO_Enable(struct rtl8192cd_priv *priv);
EXTERN int PHY_ConfigBBWithParaFile(struct rtl8192cd_priv *priv, int reg_file);

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







EXTERN void PHY_IQCalibrate(struct rtl8192cd_priv *priv);

#ifdef TX_EARLY_MODE
EXTERN void enable_em(struct rtl8192cd_priv *priv);
EXTERN void disable_em(struct rtl8192cd_priv *priv);
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

EXTERN void start_bbp_ch_load(struct rtl8192cd_priv *priv, unsigned int units);
EXTERN unsigned int read_bbp_ch_load(struct rtl8192cd_priv *priv);
EXTERN void channle_loading_measurement(struct rtl8192cd_priv *priv);


#undef EXTERN







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

#define ioctl_copy_from_user(to,from,n)	_ioctl_copy_from_user(priv,to,from,n,&flags)
#define ioctl_copy_to_user(to,from,n)		_ioctl_copy_to_user(priv,to,from,n,&flags)

unsigned long _ioctl_copy_from_user (struct rtl8192cd_priv *priv, void * to, const void * from, unsigned long n, unsigned long *flags);
unsigned long _ioctl_copy_to_user (struct rtl8192cd_priv *priv, void * to, const void * from, unsigned long n, unsigned long *flags);


#ifdef INCLUDE_WPS
EXTERN	int set_mib(struct rtl8192cd_priv *priv, unsigned char *data);
EXTERN 	int get_mib(struct rtl8192cd_priv *priv, unsigned char *data);
#endif


#ifdef CONFIG_RTL_COMAPI_WLTOOLS
EXTERN int set_mib(struct rtl8192cd_priv *priv, unsigned char *data);
EXTERN int rtl8192cd_ss_req(struct rtl8192cd_priv *priv, unsigned char *data, int len);
#endif
EXTERN void delay_us(unsigned int t);
EXTERN void delay_ms(unsigned int t);

EXTERN int check_bss_encrypt(struct rtl8192cd_priv *priv);

EXTERN void rtl8192cd_init_one_cfg80211(struct rtknl *rtk);

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
EXTERN int hmac_sha256(const unsigned char *key, size_t key_len, const unsigned char *data,
		 size_t data_len, unsigned char *mac);
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
EXTERN const unsigned char* MCS_DATA_RATEStr[2][2][24];
#endif

#ifdef RTK_AC_SUPPORT
EXTERN int query_vht_rate(struct stat_info *pstat);
#if (MU_BEAMFORMING_SUPPORT == 1)
EXTERN int query_mu_vht_rate(struct stat_info *pstat);
#endif
#endif

#undef EXTERN



/*-----------------------------------------------------------------------------
								8192cd_br_ext.c
------------------------------------------------------------------------------*/
#ifndef _8192CD_BR_EXT_C_
#define EXTERN  extern
#else
#define EXTERN
#endif

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
EXTERN BOOLEAN Wlan_HAL_Link(struct rtl8192cd_priv *priv);

EXTERN int get_bonding_type_8881A(void);

EXTERN void rtl8192cd_set_mbssid(struct rtl8192cd_priv *priv, unsigned char *macAddr, unsigned char index);
EXTERN void rtl8192cd_clear_mbssid(struct rtl8192cd_priv *priv, unsigned char index);

#ifdef TPT_THREAD
EXTERN int kTPT_thread(void *p);
EXTERN int kTPT_task_init(struct rtl8192cd_priv *priv);
EXTERN void kTPT_task_stop(struct rtl8192cd_priv *priv);
#endif


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
#ifdef CLIENT_MODE
EXTERN void ClientSendEAPOL(struct rtl8192cd_priv *priv, struct stat_info *pstat, int resend);
#ifdef SUPPORT_CLIENT_MIXED_SECURITY
EXTERN void choose_cipher(struct rtl8192cd_priv *priv, struct bss_desc *bss_target);
#endif
#endif //CLIENT_MODE

EXTERN void UKRekeyTimeout(struct rtl8192cd_priv *priv, struct stat_info *pstat);



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


EXTERN void mp_reset_stats(struct rtl8192cd_priv * priv);
EXTERN int mp_get_txpwr(struct rtl8192cd_priv *priv, unsigned char *data);

#ifdef POWER_TRIM
EXTERN void do_kfree(struct rtl8192cd_priv *priv, unsigned char *data);
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
EXTERN struct _device_info_ wlan_device[];
#undef EXTERN

#ifdef WIFI_WPAS

#define EXTERN extern
EXTERN int rtl_net80211_setoptie(struct net_device *dev, struct iw_request_info *info, union iwreq_data *wrqu, char *extra);
EXTERN int rtl_wpas_custom(struct net_device *dev, struct iw_request_info *info, union iwreq_data *wrqu, char *extra);

#undef EXTERN

#endif







/*-----------------------------------------------------------------------------
								8192cd_sta_control.c
------------------------------------------------------------------------------*/
#ifndef _8192CD_STA_CONTROL_C_
#define EXTERN  extern
#else
#define EXTERN
#endif

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
                    ,int vap_idx
);
EXTERN struct stat_info * a4_rx_check_reuse_ap(struct rtl8192cd_priv *priv, struct stat_info *pstat, struct rx_frinfo *pfrinfo);
EXTERN unsigned char a4_rx_check_reuse(struct rtl8192cd_priv *priv, struct rx_frinfo *pfrinfo, unsigned char * myhwaddr);

EXTERN int a4_dump_sta_info(struct seq_file *s, void *data);
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

EXTERN void RRSR_power_control_11n(struct rtl8192cd_priv *priv, int lower);



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
// Tx Power
#ifdef HIGH_POWER_EXT_PA
EXTERN void tx_power_control(struct rtl8192cd_priv *priv);
#endif
EXTERN void tx_power_tracking(struct rtl8192cd_priv *priv);



// EDCA
EXTERN void EdcaParaInit(struct rtl8192cd_priv *priv);
EXTERN unsigned char *Get_Adaptivity_Version(void);

#if !defined(USE_OUT_SRC) || defined(_OUTSRC_COEXIST)
EXTERN void init_EDCA_para(struct rtl8192cd_priv *priv, int mode);
EXTERN void choose_IOT_main_sta(struct rtl8192cd_priv *priv, struct stat_info *pstat);
EXTERN void rxBB_dm(struct rtl8192cd_priv *priv);
EXTERN void IOT_engine(struct rtl8192cd_priv *priv);
#endif



#ifdef WIFI_WMM
EXTERN void IOT_EDCA_switch(struct rtl8192cd_priv *priv, int mode, char enable);
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

// Leaving STA check
#if defined(TXREPORT)
EXTERN void DetectSTAExistance(struct rtl8192cd_priv *priv, struct tx_rpt *report, struct stat_info *pstat );
EXTERN void DetectSTAExistance88XX(struct rtl8192cd_priv *priv, struct tx_rpt *report, struct stat_info *pstat );

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



EXTERN void CCK_CCA_dynamic_enhance(struct rtl8192cd_priv *priv, unsigned char rssi_strength);

#undef EXTERN




// P2P_SUPPORT
/*-----------------------------------------------------------------------------
								8192cd_p2p.c
------------------------------------------------------------------------------*/






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




/*-----------------------------------------------------------------------------
								ODM
------------------------------------------------------------------------------*/

#include "phydm/phydm_precomp.h"



#define EXTERN  extern




EXTERN void SwitchChannel(struct rtl8192cd_priv *priv);


#undef EXTERN

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



/*-----------------------------------------------------------------------------
								8192cd_config.c
------------------------------------------------------------------------------*/


/*-----------------------------------------------------------------------------
								inline function
------------------------------------------------------------------------------*/


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

