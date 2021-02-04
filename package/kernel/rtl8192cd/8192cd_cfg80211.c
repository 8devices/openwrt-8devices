/*
 * Copyright (c) 2004-2011 Atheros Communications Inc.
 * Copyright (c) 2011-2012 Qualcomm Atheros, Inc.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#ifdef __KERNEL__
#include <linux/module.h>
#include <linux/init.h>
#include <asm/uaccess.h>
#include <linux/unistd.h>
#include <linux/file.h>
#include <linux/fs.h>
#include <linux/delay.h>
#endif

#include "./8192cd_cfg.h"

#ifdef RTK_NL80211

#include <linux/kernel.h>
#include <linux/if_arp.h>
#include <linux/netdevice.h>
#include <linux/rtnetlink.h>
#include <net/cfg80211.h>
#include <net/mac80211.h>
#include <net/ieee80211_radiotap.h>
#include <linux/wireless.h>
#include <linux/device.h>
#include <linux/if_ether.h>
#include <linux/nl80211.h>
#include <linux/ieee80211.h>



//#include "./nl80211_copy.h"

#ifdef __LINUX_2_6__
#include <linux/initrd.h>
#include <linux/syscalls.h>
#endif

#include "./8192cd.h"
#include "./8192cd_debug.h"
#include "./8192cd_cfg80211.h"
#include "./8192cd_headers.h"
#include "./8192cd_p2p.h"

#include <net80211/ieee80211.h>
#include <net80211/ieee80211_crypto.h>
#include <net80211/ieee80211_ioctl.h>
#include "./8192cd_net80211.h"  

//#define DEBUG_NL80211
#ifdef DEBUG_NL80211
//#define NLENTER	{}
//#define NLEXIT {}
#define NLENTER	printk("[RTKNL][%s][%s] +++ \n", priv->dev->name, (char *)__FUNCTION__)
#define NLEXIT  printk("[RTKNL][%s][%s] --- \n\n", priv->dev->name, (char *)__FUNCTION__)
#define NLINFO(fmt, args...) printk("[RTKNL][%s %d]"fmt, __FUNCTION__,__LINE__, ## args)
#define NLNOT   printk("[RTKNL][%s %d] !!! NOT implement YET !!!\n", (char *)__FUNCTION__,__LINE__)
#else
#define NLENTER
#define NLEXIT
#define NLINFO
#define NLNOT
#endif
#if  !defined(CONFIG_OPENWRT_SDK)
struct cfg80211_mgmt_tx_params {
    struct ieee80211_channel *chan;
    bool offchan;
    unsigned int wait;
    const u8 *buf;
    size_t len;
    bool no_cck;
    bool dont_wait_for_ack;
 };
#endif
void void_printk_nl80211(const char *fmt, ...)
{
	;
}


#ifdef DEBUG_NL80211
#define NLMSG	panic_printk
#else
#define NLMSG	void_printk_nl80211
#endif

//#define	SIGNAL_TYPE_UNSPEC

extern unsigned char WFA_OUI_PLUS_TYPE[];

#define RTK_MAX_WIFI_PHY 2
static int rtk_phy_idx=0;
struct rtknl *rtk_phy[RTK_MAX_WIFI_PHY]; 
static dev_t rtk_wifi_dev[RTK_MAX_WIFI_PHY]; 
static char *rtk_dev_name[RTK_MAX_WIFI_PHY]={"RTKWiFi0","RTKWiFi1"}; 

char rtk_fake_addr[6]={0x00,0xe0,0x4c,0xcc,0xdd,0x01}; //mark_dual , FIXME if wlan_mac readable

#define MAX_5G_DIFF_NUM		14
#define PIN_LEN					8
#define SIGNATURE_LEN			4
#if 0
#ifdef CONFIG_RTL_HW_SETTING_OFFSET
#define HW_SETTING_OFFSET		CONFIG_RTL_HW_SETTING_OFFSET
#else
#define HW_SETTING_OFFSET		0x6000
#endif
#endif
extern unsigned int HW_SETTING_OFFSET; //mark_hw,from rtl819x_flash.c
#define HW_WLAN_SETTING_OFFSET	13

__PACK struct hw_wlan_setting {
	unsigned char macAddr[6] ;
	unsigned char macAddr1[6] ;
	unsigned char macAddr2[6] ;
	unsigned char macAddr3[6] ;
	unsigned char macAddr4[6] ;
	unsigned char macAddr5[6] ; 
	unsigned char macAddr6[6] ; 
	unsigned char macAddr7[6] ; 
	unsigned char pwrlevelCCK_A[MAX_2G_CHANNEL_NUM] ; 	
	unsigned char pwrlevelCCK_B[MAX_2G_CHANNEL_NUM] ; 
	unsigned char pwrlevelHT40_1S_A[MAX_2G_CHANNEL_NUM] ;	
	unsigned char pwrlevelHT40_1S_B[MAX_2G_CHANNEL_NUM] ;	
	unsigned char pwrdiffHT40_2S[MAX_2G_CHANNEL_NUM] ;	
	unsigned char pwrdiffHT20[MAX_2G_CHANNEL_NUM] ; 
	unsigned char pwrdiffOFDM[MAX_2G_CHANNEL_NUM] ; 
	unsigned char regDomain ; 	
	unsigned char rfType ; 
	unsigned char ledType ; // LED type, see LED_TYPE_T for definition	
	unsigned char xCap ;	
	unsigned char TSSI1 ;	
	unsigned char TSSI2 ;	
	unsigned char Ther ;	
	unsigned char Reserved1 ;
	unsigned char Reserved2 ;
	unsigned char Reserved3 ;
	unsigned char Reserved4 ;
	unsigned char Reserved5 ;	
	unsigned char Reserved6 ;
	unsigned char Reserved7 ;	
	unsigned char Reserved8 ;
	unsigned char Reserved9 ;
	unsigned char Reserved10 ;
	unsigned char pwrlevel5GHT40_1S_A[MAX_5G_CHANNEL_NUM] ;	
	unsigned char pwrlevel5GHT40_1S_B[MAX_5G_CHANNEL_NUM] ;	
	unsigned char pwrdiff5GHT40_2S[MAX_5G_CHANNEL_NUM] ; 
	unsigned char pwrdiff5GHT20[MAX_5G_CHANNEL_NUM] ;	
	unsigned char pwrdiff5GOFDM[MAX_5G_CHANNEL_NUM] ;

	
	unsigned char wscPin[PIN_LEN+1] ;	

#ifdef RTK_AC_SUPPORT
	unsigned char pwrdiff_20BW1S_OFDM1T_A[MAX_2G_CHANNEL_NUM] ;
	unsigned char pwrdiff_40BW2S_20BW2S_A[MAX_2G_CHANNEL_NUM] ;
	unsigned char pwrdiff_OFDM2T_CCK2T_A[MAX_2G_CHANNEL_NUM] ;
	unsigned char pwrdiff_40BW3S_20BW3S_A[MAX_2G_CHANNEL_NUM] ;
	unsigned char pwrdiff_4OFDM3T_CCK3T_A[MAX_2G_CHANNEL_NUM] ;
	unsigned char pwrdiff_40BW4S_20BW4S_A[MAX_2G_CHANNEL_NUM] ;
	unsigned char pwrdiff_OFDM4T_CCK4T_A[MAX_2G_CHANNEL_NUM] ;

	unsigned char pwrdiff_5G_20BW1S_OFDM1T_A[MAX_5G_DIFF_NUM] ;
	unsigned char pwrdiff_5G_40BW2S_20BW2S_A[MAX_5G_DIFF_NUM] ;
	unsigned char pwrdiff_5G_40BW3S_20BW3S_A[MAX_5G_DIFF_NUM] ;
	unsigned char pwrdiff_5G_40BW4S_20BW4S_A[MAX_5G_DIFF_NUM] ;
	unsigned char pwrdiff_5G_RSVD_OFDM4T_A[MAX_5G_DIFF_NUM] ;
	unsigned char pwrdiff_5G_80BW1S_160BW1S_A[MAX_5G_DIFF_NUM] ;
	unsigned char pwrdiff_5G_80BW2S_160BW2S_A[MAX_5G_DIFF_NUM] ;
	unsigned char pwrdiff_5G_80BW3S_160BW3S_A[MAX_5G_DIFF_NUM] ;
	unsigned char pwrdiff_5G_80BW4S_160BW4S_A[MAX_5G_DIFF_NUM] ;


	unsigned char pwrdiff_20BW1S_OFDM1T_B[MAX_2G_CHANNEL_NUM] ;
	unsigned char pwrdiff_40BW2S_20BW2S_B[MAX_2G_CHANNEL_NUM] ;
	unsigned char pwrdiff_OFDM2T_CCK2T_B[MAX_2G_CHANNEL_NUM] ;
	unsigned char pwrdiff_40BW3S_20BW3S_B[MAX_2G_CHANNEL_NUM] ;
	unsigned char pwrdiff_OFDM3T_CCK3T_B[MAX_2G_CHANNEL_NUM] ;
	unsigned char pwrdiff_40BW4S_20BW4S_B[MAX_2G_CHANNEL_NUM] ;
	unsigned char pwrdiff_OFDM4T_CCK4T_B[MAX_2G_CHANNEL_NUM] ;

	unsigned char pwrdiff_5G_20BW1S_OFDM1T_B[MAX_5G_DIFF_NUM] ;
	unsigned char pwrdiff_5G_40BW2S_20BW2S_B[MAX_5G_DIFF_NUM] ;
	unsigned char pwrdiff_5G_40BW3S_20BW3S_B[MAX_5G_DIFF_NUM] ;
	unsigned char pwrdiff_5G_40BW4S_20BW4S_B[MAX_5G_DIFF_NUM] ;
	unsigned char pwrdiff_5G_RSVD_OFDM4T_B[MAX_5G_DIFF_NUM] ;
	unsigned char pwrdiff_5G_80BW1S_160BW1S_B[MAX_5G_DIFF_NUM] ;
	unsigned char pwrdiff_5G_80BW2S_160BW2S_B[MAX_5G_DIFF_NUM] ;
	unsigned char pwrdiff_5G_80BW3S_160BW3S_B[MAX_5G_DIFF_NUM] ;
	unsigned char pwrdiff_5G_80BW4S_160BW4S_B[MAX_5G_DIFF_NUM] ;

    unsigned char pwrdiff_20BW1S_OFDM1T_C[MAX_2G_CHANNEL_NUM];  
    unsigned char pwrdiff_40BW2S_20BW2S_C[MAX_2G_CHANNEL_NUM];  
    unsigned char pwrdiff_OFDM2T_CCK2T_C[MAX_2G_CHANNEL_NUM];
    unsigned char pwrdiff_40BW3S_20BW3S_C[MAX_2G_CHANNEL_NUM];  
    unsigned char pwrdiff_4OFDM3T_CCK3T_C[MAX_2G_CHANNEL_NUM];  
    unsigned char pwrdiff_40BW4S_20BW4S_C[MAX_2G_CHANNEL_NUM];  
    unsigned char pwrdiff_OFDM4T_CCK4T_C[MAX_2G_CHANNEL_NUM];

    unsigned char pwrdiff_5G_20BW1S_OFDM1T_C[MAX_5G_CHANNEL_NUM];   
    unsigned char pwrdiff_5G_40BW2S_20BW2S_C[MAX_5G_CHANNEL_NUM];   
    unsigned char pwrdiff_5G_40BW3S_20BW3S_C[MAX_5G_CHANNEL_NUM];   
    unsigned char pwrdiff_5G_40BW4S_20BW4S_C[MAX_5G_CHANNEL_NUM];   
    unsigned char pwrdiff_5G_RSVD_OFDM4T_C[MAX_5G_CHANNEL_NUM]; 
    unsigned char pwrdiff_5G_80BW1S_160BW1S_C[MAX_5G_CHANNEL_NUM];  
    unsigned char pwrdiff_5G_80BW2S_160BW2S_C[MAX_5G_CHANNEL_NUM];  
    unsigned char pwrdiff_5G_80BW3S_160BW3S_C[MAX_5G_CHANNEL_NUM];  
    unsigned char pwrdiff_5G_80BW4S_160BW4S_C[MAX_5G_CHANNEL_NUM];

    unsigned char pwrdiff_20BW1S_OFDM1T_D[MAX_2G_CHANNEL_NUM];  
    unsigned char pwrdiff_40BW2S_20BW2S_D[MAX_2G_CHANNEL_NUM];  
    unsigned char pwrdiff_OFDM2T_CCK2T_D[MAX_2G_CHANNEL_NUM];
    unsigned char pwrdiff_40BW3S_20BW3S_D[MAX_2G_CHANNEL_NUM];  
    unsigned char pwrdiff_4OFDM3T_CCK3T_D[MAX_2G_CHANNEL_NUM];  
    unsigned char pwrdiff_40BW4S_20BW4S_D[MAX_2G_CHANNEL_NUM];  
    unsigned char pwrdiff_OFDM4T_CCK4T_D[MAX_2G_CHANNEL_NUM];

    unsigned char pwrdiff_5G_20BW1S_OFDM1T_D[MAX_5G_CHANNEL_NUM];   
    unsigned char pwrdiff_5G_40BW2S_20BW2S_D[MAX_5G_CHANNEL_NUM];   
    unsigned char pwrdiff_5G_40BW3S_20BW3S_D[MAX_5G_CHANNEL_NUM];   
    unsigned char pwrdiff_5G_40BW4S_20BW4S_D[MAX_5G_CHANNEL_NUM];   
    unsigned char pwrdiff_5G_RSVD_OFDM4T_D[MAX_5G_CHANNEL_NUM]; 
    unsigned char pwrdiff_5G_80BW1S_160BW1S_D[MAX_5G_CHANNEL_NUM];  
    unsigned char pwrdiff_5G_80BW2S_160BW2S_D[MAX_5G_CHANNEL_NUM];  
    unsigned char pwrdiff_5G_80BW3S_160BW3S_D[MAX_5G_CHANNEL_NUM];  
    unsigned char pwrdiff_5G_80BW4S_160BW4S_D[MAX_5G_CHANNEL_NUM];

    unsigned char pwrlevelCCK_C[MAX_2G_CHANNEL_NUM];
	unsigned char pwrlevelCCK_D[MAX_2G_CHANNEL_NUM];
	unsigned char pwrlevelHT40_1S_C[MAX_2G_CHANNEL_NUM];
	unsigned char pwrlevelHT40_1S_D[MAX_2G_CHANNEL_NUM];
	unsigned char pwrlevel5GHT40_1S_C[MAX_5G_CHANNEL_NUM];
	unsigned char pwrlevel5GHT40_1S_D[MAX_5G_CHANNEL_NUM];
#endif
}__WLAN_ATTRIB_PACK__;

__PACK struct param_header {
	unsigned char signature[SIGNATURE_LEN];  // Tag + version
	unsigned short len ;
} __WLAN_ATTRIB_PACK__;

unsigned char is_WRT_scan_iface(unsigned char* if_name)
{
	if((strcmp(if_name, "tmp.wlan0")==0) || (strcmp(if_name, "tmp.wlan1")==0))
		return 1;
	else
		return 0;
}

int is_zero_mac(const unsigned char *mac)
{
	return !(mac[0] | mac[1] | mac[2] | mac[3] | mac[4] | mac[5]| mac[6]| mac[7]);
}

void dump_mac(struct rtl8192cd_priv *priv, unsigned char *mac)
{
	if(mac && !is_zero_mac(mac)){
		NDEBUG(" %02x:%02x:%02x:%02x:%02x:%02x\n", mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);
    }
}

#if defined(P2P_SUPPORT)
// function declaration 
int realtek_remain_on_channel(struct wiphy *wiphy,
    struct wireless_dev *wdev,
	struct ieee80211_channel *channel,
	unsigned int duration,
	u64 *cookie);

static int realtek_cancel_remain_on_channel(struct wiphy *wiphy,
			struct wireless_dev *wdev,	u64 cookie);

#endif //CONFIG_P2P

#if defined(VAP_MAC_DRV_READ_FLASH)
int read_flash_hw_mac_vap(unsigned char *mac, int vap_idx)
{
	unsigned int offset;

	//NLENTER;
	
	if(!mac)
		return -1;

	vap_idx +=1; 
	
	if(vap_idx > 7)
		return -1;
	
	offset = HW_SETTING_OFFSET+ sizeof(struct param_header)+ HW_WLAN_SETTING_OFFSET + sizeof(struct hw_wlan_setting) * (rtk_phy_idx-1);
	offset += (vap_idx*ETH_ALEN);
	offset |= 0xbd000000;
	memcpy(mac,(unsigned char *)offset,ETH_ALEN);

	if(is_zero_mac(mac))
		return -1;

	DEBUG_INFO("VAP[%d][%d]=%02x:%02x:%02x:%02x:%02x:%02x\n", (rtk_phy_idx-1), vap_idx, mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);

	return 0;
}
#endif

//brian, get MAC address from utility, rtk_tx_calr
#if 0
void read_flash_hw_mac( unsigned char *mac,int idx)
{
	unsigned int offset;
	if(!mac)
		return;
	offset = HW_SETTING_OFFSET+ sizeof(struct param_header)+ HW_WLAN_SETTING_OFFSET + sizeof(struct hw_wlan_setting) * idx;
	offset |= 0xbd000000;
	memcpy(mac,(unsigned char *)offset,ETH_ALEN);
}
#endif
//#define CPTCFG_CFG80211_MODULE 1 // mark_com

#if 1 //_eric_nl event 

struct rtl8192cd_priv* realtek_get_priv(struct wiphy *wiphy, struct net_device *dev)
{
	struct rtknl *rtk = wiphy_priv(wiphy);
	struct rtl8192cd_priv *priv = NULL;

	if(dev)
	{
#ifdef NETDEV_NO_PRIV
		priv = ((struct rtl8192cd_priv *)netdev_priv(dev))->wlan_priv;
#else
		priv = (struct rtl8192cd_priv *)dev->priv;
#endif
	}
	else
		priv = rtk->priv;

	return priv;
}

struct ieee80211_channel *rtk_get_iee80211_channel(struct wiphy *wiphy, unsigned int channel)
{
	unsigned int  freq = 0;

	if(channel >= 34)
		freq = ieee80211_channel_to_frequency(channel, IEEE80211_BAND_5GHZ);
	else
		freq = ieee80211_channel_to_frequency(channel, IEEE80211_BAND_2GHZ);

	return ieee80211_get_channel(wiphy, freq);

}


void realtek_cfg80211_inform_bss(struct rtl8192cd_priv *priv)
{
	struct wiphy *wiphy = priv->rtk->wiphy;
	struct ieee80211_channel *channel = NULL;
	struct ieee80211_bss *bss = NULL;
	char tmpbuf[33];
	UINT8 *mac = NULL;
	unsigned long timestamp = 0;
	unsigned char ie[MAX_IE_LEN];
	unsigned char ie_len = 0;
	unsigned char wpa_ie_len = 0;
	unsigned char rsn_ie_len = 0;
	unsigned int  freq = 0;

	mac = priv->pmib->dot11Bss.bssid;
	wpa_ie_len = priv->rtk->clnt_info.wpa_ie.wpa_ie_len;
	rsn_ie_len = priv->rtk->clnt_info.rsn_ie.rsn_ie_len;
	
	channel = rtk_get_iee80211_channel(wiphy, priv->pmib->dot11Bss.channel);
		
	if(channel == NULL)
	{
		printk("Null channel!!\n");
		return;
	}
		
	timestamp = priv->pmib->dot11Bss.t_stamp[0] + (priv->pmib->dot11Bss.t_stamp[0]<<32);

	ie[0]= _SSID_IE_;
	ie[1]= priv->pmib->dot11Bss.ssidlen;
	memcpy(ie+2, priv->pmib->dot11Bss.ssid, priv->pmib->dot11Bss.ssidlen);
	ie_len += (priv->pmib->dot11Bss.ssidlen + 2);
			
	if((ie_len + wpa_ie_len + rsn_ie_len) < MAX_IE_LEN)
	{
		if(wpa_ie_len)
		{
			memcpy(ie+ie_len, priv->rtk->clnt_info.wpa_ie.data, wpa_ie_len);
			ie_len += wpa_ie_len;
		}

		if(rsn_ie_len)
		{
				memcpy(ie+ie_len, priv->rtk->clnt_info.rsn_ie.data, rsn_ie_len);
				ie_len += rsn_ie_len;
		}
	}
	else
		printk("ie_len too long !!!\n");
#if 1
	NDEBUG2("bss[%s],hw_value[%03d],timestamp[0x%08x]",
        priv->pmib->dot11Bss.ssid, 
        channel->hw_value, 
        timestamp);

	NDEBUG2("capa[0x%02x],beacon_prd[%d],ie_len[%d],rssi[%d]\n", 
        priv->pmib->dot11Bss.capability,
		priv->pmib->dot11Bss.beacon_prd, 
		ie_len, 
		priv->pmib->dot11Bss.rssi);
#endif

	bss = cfg80211_inform_bss(wiphy, channel, 
#ifdef OPENWRT_CC
			CFG80211_BSS_FTYPE_UNKNOWN,
#endif
			mac, timestamp, 
			priv->pmib->dot11Bss.capability, 
			priv->pmib->dot11Bss.beacon_prd, 
			ie, ie_len, priv->pmib->dot11Bss.rssi, GFP_ATOMIC);

	if(bss)
		cfg80211_put_bss(wiphy, bss);
	else {
		//printk("%s bss = null\n",__func__);
	}
}

void realtek_cfg80211_inform_bss_ies(unsigned char *ie_buf, unsigned int *total_ie_len,
    						unsigned char ie_id, unsigned char *ie, unsigned char ie_len)
{
	ie_buf[*total_ie_len] = ie_id;
	ie_buf[*total_ie_len+1] = ie_len;
	*total_ie_len += 2;
	memcpy(ie_buf+*total_ie_len,ie,ie_len);
	*total_ie_len += ie_len;
}

#if !defined(SIGNAL_TYPE_UNSPEC)
int translate_ss_result_rssi(int rssi)
{
	//reference libiwinfo/iwinfo_nl80211.c - scan_cb
	return ((rssi-100+0x100)*100);
}
#endif

void realtek_cfg80211_inform_ss_result(struct rtl8192cd_priv *priv)
{
	int i;
	struct wiphy *wiphy = priv->rtk->wiphy;
	struct ieee80211_channel *channel = NULL;
	struct ieee80211_bss *bss = NULL;

	NLMSG("SiteSurvey Count=%d\n", priv->site_survey->count);
	//printk("SSID                 BSSID        ch  prd cap  bsc  oper ss sq bd 40m\n");
	
	if (!priv->scan_req)
	{
		NLMSG("[%s][%s] No scan_req, No report to NL80211!!\n", priv->dev->name, __FUNCTION__);
		return;
	}
	
	for(i=0; i<priv->site_survey->count; i++)
	{
		char tmpbuf[33];
		UINT8 *mac = priv->site_survey->bss[i].bssid;
		unsigned long timestamp = 0;
		unsigned char report_ie[MAX_IE_LEN];
		unsigned int report_ie_len = 0;
		unsigned char wpa_ie_len = priv->site_survey->bss[i].wpa_ie_len;
		unsigned char rsn_ie_len = priv->site_survey->bss[i].rsn_ie_len;
		unsigned char wps_ie_len = priv->site_survey->wscie[i].wps_ie_len; //wrt-wps-clnt        
		unsigned char rtk_p2p_ie_len = priv->site_survey->rtk_p2p_ie[i].p2p_ie_len; //wrt-wps-clnt       
		unsigned int  freq = 0;

		channel = rtk_get_iee80211_channel(wiphy, priv->site_survey->bss[i].channel);
		
		if(channel == NULL)
		{
			NDEBUG("Null channel!!\n");
			continue;
		}
		
		timestamp = priv->site_survey->bss[i].t_stamp[0] + (priv->site_survey->bss[i].t_stamp[0]<<32);

		report_ie[0]= _SSID_IE_;
		report_ie[1]= priv->site_survey->bss[i].ssidlen;
		memcpy(report_ie+2, priv->site_survey->bss[i].ssid, priv->site_survey->bss[i].ssidlen);
		report_ie_len += (priv->site_survey->bss[i].ssidlen + 2);
			
		if((report_ie_len + wpa_ie_len + rsn_ie_len + wps_ie_len + rtk_p2p_ie_len) < MAX_IE_LEN)
		{
            //NDEBUG("\n");                		
			if(wpa_ie_len)
			{

                memcpy(report_ie+report_ie_len, priv->site_survey->bss[i].wpa_ie, wpa_ie_len);
				report_ie_len += wpa_ie_len;
                //NDEBUG("\n");                
			}

			if(rsn_ie_len)
			{

				memcpy(report_ie+report_ie_len, priv->site_survey->bss[i].rsn_ie, rsn_ie_len);
				report_ie_len += rsn_ie_len;
                //NDEBUG("\n");                
			}

            if(wps_ie_len)
            {
                memcpy(report_ie+report_ie_len, priv->site_survey->wscie[i].data, wps_ie_len);
                report_ie_len += wps_ie_len;
                //NDEBUG("bss include wps ie[%d]\n",wps_ie_len);
            }

			if(rtk_p2p_ie_len)
			{
				memcpy(report_ie+report_ie_len, priv->site_survey->rtk_p2p_ie[i].data, rtk_p2p_ie_len);
				report_ie_len += rtk_p2p_ie_len;
                //NDEBUG("scan include p2p ie[%d]\n",rtk_p2p_ie_len);
			}
			//for DSSET IE
			realtek_cfg80211_inform_bss_ies(report_ie,&report_ie_len,_DSSET_IE_,&priv->site_survey->bss[i].channel,DSSET_IE_LEN);
			//for ht cap
			realtek_cfg80211_inform_bss_ies(report_ie,&report_ie_len,_HT_CAP_,&priv->site_survey->bss[i].ht_cap,HTCAP_IE_LEN);
			//for ht oper
			realtek_cfg80211_inform_bss_ies(report_ie,&report_ie_len,_HT_IE_,&priv->site_survey->bss[i].ht_info,HTINFO_IE_LEN);
		}
		else{
			printk("report_ie_len too long !!!\n");
        }

		#if 0
		printk("[%d=%s] %03d 0x%08x 0x%02x %d ie_len=%d rssi=%d\n", i, priv->site_survey->bss[i].ssid, 
			channel->hw_value, timestamp, 
			priv->site_survey->bss[i].capability, 
			priv->site_survey->bss[i].beacon_prd, report_ie_len, 
			priv->site_survey->bss[i].rssi);
		#endif

		bss = cfg80211_inform_bss(wiphy, channel, 
#ifdef OPENWRT_CC
				CFG80211_BSS_FTYPE_UNKNOWN,
#endif
				mac, timestamp, 
				priv->site_survey->bss[i].capability, 
				priv->site_survey->bss[i].beacon_prd,
#if defined(SIGNAL_TYPE_UNSPEC)
				report_ie, report_ie_len, priv->site_survey->bss[i].rssi, GFP_ATOMIC);
#else
				report_ie, report_ie_len, translate_ss_result_rssi(priv->site_survey->bss[i].rssi), GFP_ATOMIC);
#endif

		if(bss)
			cfg80211_put_bss(wiphy, bss);
		else {
			//printk("%s bss = null\n",__func__);
		}
	}
}

// static void realtek_cfg80211_sscan_disable(struct rtl8192cd_priv *priv) rename to easy understand name rtk_abort_scan
static void rtk_abort_scan(struct rtl8192cd_priv *priv, enum scan_abort_case abort_case) 
{    
	struct rtl8192cd_priv *priv_root = GET_ROOT(priv);
	struct cfg80211_scan_request *scan_req = priv_root->scan_req;
	
	NLMSG("[%s] rtk_abort_scan [0x%x]+++ \n", priv->dev->name, GET_ROOT(priv)->scan_req);
	
	if(priv_root->scan_req)
	{	
		priv_root->ss_req_ongoing = 0;
		priv_root->scan_req = NULL;
	}
	
	if(priv->scan_req) //VXD can also do scan
	{	
		priv->ss_req_ongoing = 0;
		priv->scan_req = NULL;
	}

	//event_indicate_cfg80211(priv, NULL, CFG80211_SCAN_ABORDED, NULL);
	//cfg80211_sched_scan_stopped(wiphy);
}



#define HAPD_READY_RX_EVENT	5

void event_to_name(int event, char *event_name)
{

	switch (event) {
	case CFG80211_CONNECT_RESULT:
		strcpy(event_name, "CFG80211_CONNECT_RESULT");
		break;
	case CFG80211_ROAMED:
		strcpy(event_name, "CFG80211_ROAMED");
		break;
	case CFG80211_DISCONNECTED:
		strcpy(event_name, "CFG80211_DISCONNECTED");
		break;
	case CFG80211_IBSS_JOINED:
		strcpy(event_name, "CFG80211_IBSS_JOINED");
		break;
    case CFG80211_NEW_STA:
        strcpy(event_name, "CFG80211_NEW_STA");
        break;
	case CFG80211_SCAN_DONE:
		strcpy(event_name, "CFG80211_SCAN_DONE");
		break;
	case CFG80211_SCAN_ABORDED:
		strcpy(event_name, "CFG80211_SCAN_ABORDED");
		break;
	case CFG80211_DEL_STA:
		strcpy(event_name, "CFG80211_DEL_STA");
		break;
	case CFG80211_RADAR_CAC_FINISHED:
		strcpy(event_name, "CFG80211_RADAR_CAC_FINISHED");
		break;
	case CFG80211_RADAR_DETECTED:
		strcpy(event_name, "CFG80211_RADAR_DETECTED");
		break;
	default:
		strcpy(event_name, "UNKNOWN EVENT");
		break;
	}
	
}

int event_indicate_cfg80211(struct rtl8192cd_priv *priv, unsigned char *mac, int event, unsigned char *extra)
{
	struct net_device	*dev = (struct net_device *)priv->dev;
	struct stat_info	*pstat = NULL;
	struct station_info sinfo;
	struct wiphy *wiphy = priv->rtk->wiphy;

    u8 assoc_req_ies_buf[256*3];        
    u8* ie_pos=assoc_req_ies_buf;                   

	int flags;
	
	//NLENTER;
	{
		char event_name[32];
		event_to_name(event, event_name);
		NLMSG("EVENT [%s][%s=%d]\n", priv->dev->name, event_name, event);
	}

    /*cfg p2p 2014-0330 , report CFG80211_NEW_STA , ASAP*/ 
	if( (event != CFG80211_SCAN_DONE)  && (event != CFG80211_NEW_STA) && (event != CFG80211_DEL_STA) && (event != CFG80211_RADAR_CAC_FINISHED) ){ //eric-bb
    	if( (OPMODE & WIFI_AP_STATE) && (priv->up_time <= HAPD_READY_RX_EVENT) ) 
    	{
    		NLMSG("ignore cfg event,up_time[%d],event[%d]\n", priv->up_time,event);
    		return; 
    	}
    }

	if(mac)
		pstat = get_stainfo(priv, mac);

#ifdef SMP_SYNC
	SMP_LOCK_CFG80211(flags); 
#endif

	switch(event) {
		case CFG80211_CONNECT_RESULT:
			{
				NDEBUG3("cfg80211_event [CFG80211_CONNECT_RESULT][%d]\n", event);
				struct cfg80211_bss *bss = NULL;

				if(priv->receive_connect_cmd == 0)
				{
					NDEBUG3("Not received connect cmd yet !! No report CFG80211_CONNECT_RESULT\n");
					break;
				}

#if 0
				bss = cfg80211_get_bss(wiphy, 
						priv->pmib->dot11Bss.channel, priv->pmib->dot11Bss.bssid,
						priv->pmib->dot11Bss.ssid, priv->pmib->dot11Bss.ssidlen, 
						WLAN_CAPABILITY_ESS, WLAN_CAPABILITY_ESS);

				if(bss==NULL)
				{
					NDEBUG2("report this bss\n");
					realtek_cfg80211_inform_bss(priv);
				}
#endif

				cfg80211_connect_result(priv->dev, BSSID,
						priv->rtk->clnt_info.assoc_req, priv->rtk->clnt_info.assoc_req_len,
						priv->rtk->clnt_info.assoc_rsp, priv->rtk->clnt_info.assoc_rsp_len,
						WLAN_STATUS_SUCCESS, GFP_KERNEL);
			}
			break;
		case CFG80211_ROAMED:
			{
				NDEBUG3("cfg80211_event [CFG80211_ROAMED][%d]\n", event);						
				break;
			}
			break;
		case CFG80211_DISCONNECTED:
			{
				//_eric_nl ?? disconnect event no mac, for station mode only ??
				NDEBUG3("cfg80211_event [CFG80211_DISCONNECTED][%d]\n", event);
//#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 1, 0)) && defined(OPENWRT_CC)
#if  defined(OPENWRT_CC) //mark_cc
				cfg80211_disconnected(priv->dev, 0, NULL, 0, 1, GFP_KERNEL);
#else
				cfg80211_disconnected(priv->dev, 0, NULL, 0, GFP_KERNEL);
#endif
				break;
			}
			break;
		case CFG80211_IBSS_JOINED:
			{
				struct cfg80211_bss *bss = NULL;
				struct ieee80211_channel *channel = NULL;

				bss = cfg80211_get_bss(wiphy, 
						priv->pmib->dot11Bss.channel, priv->pmib->dot11Bss.bssid,
						priv->pmib->dot11Bss.ssid, priv->pmib->dot11Bss.ssidlen, 
						WLAN_CAPABILITY_ESS, WLAN_CAPABILITY_ESS);

				if(bss==NULL)
				{
					printk("report this bss\n");
					realtek_cfg80211_inform_bss(priv);
				}

				channel = rtk_get_iee80211_channel(wiphy, priv->pmib->dot11Bss.channel);
#ifdef CONFIG_OPENWRT_SDK			
				cfg80211_ibss_joined(priv->dev, BSSID, channel, GFP_KERNEL);				
#else
				cfg80211_ibss_joined(priv->dev, BSSID, GFP_KERNEL);
#endif
			}
			break;
		case CFG80211_NEW_STA:
			{	
				NDEBUG3("cfg80211_event [CFG80211_NEW_STA][%d]\n", event);				
				/* send event to application */
				memset(&sinfo, 0, sizeof(struct station_info));
				memset(assoc_req_ies_buf, 0, sizeof(256*3));
		        sinfo.assoc_req_ies = assoc_req_ies_buf;            

				if(pstat == NULL)
				{
					NDEBUG3("!!PSTA = NULL, MAC = %02x:%02x:%02x:%02x:%02x:%02x\n",mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

					if(extra == NULL){
						NDEBUG3("NO PSTA for CFG80211_NEW_STA\n");
						break;
					} else 
						pstat = extra;
				}
				
				/* TODO: sinfo.generation ???*/		        
				if(pstat->wpa_ie[1] > 0){
					sinfo.assoc_req_ies_len += pstat->wpa_ie[1]+2;
		            memcpy(ie_pos,pstat->wpa_ie, pstat->wpa_ie[1]+2);
		            ie_pos+=pstat->wpa_ie[1]+2;
		        }
				

				if(pstat->wps_ie[1] > 0)    // for wrt-wps
				{				
					sinfo.assoc_req_ies_len += pstat->wps_ie[1]+2;
		            memcpy(ie_pos,pstat->wps_ie, pstat->wps_ie[1]+2);
		            ie_pos+=pstat->wps_ie[1]+2;                
				}

#if defined(P2P_SUPPORT)
		        /*p2p support , cfg p2p , 2014 0330 , report p2p_ie included in assoc_req*/
				if(pstat->p2p_ie[1] > 0)
				{
					sinfo.assoc_req_ies_len += pstat->p2p_ie[1]+2;
		            memcpy(ie_pos,pstat->p2p_ie, pstat->p2p_ie[1]+2);
		            ie_pos += (pstat->p2p_ie[1]+2); 
				}
		        /*p2p support , cfg p2p , 2014 0330 , report p2p_ie included in assoc_req*/
#endif

#ifndef OPENWRT_CC
				if(sinfo.assoc_req_ies_len)
					sinfo.filled |= STATION_INFO_ASSOC_REQ_IES; 
#endif
				
				NDEBUG2("cfg80211_new_sta assoc req,[idx=%d] Rx assoc_req_ies_len = %d\n", priv->dev->ifindex, sinfo.assoc_req_ies_len);			
				cfg80211_new_sta(priv->dev, mac, &sinfo, GFP_KERNEL);
				NDEBUG3("cfg80211_new_sta ,STA[%02x%02x%02x:%02x%02x%02x]\n",mac[0], mac[1], mac[2], mac[3], mac[4], mac[5] );

				
				netif_wake_queue(priv->dev); //wrt-vap
			}
			break;
		case CFG80211_SCAN_ABORDED:
			{
				//NDEBUG2("cfg80211_event [CFG80211_SCAN_DONE][%d]\n", event);		
				priv->ss_req_ongoing = 0;
		        
				if (priv->scan_req) 
				{
					struct cfg80211_scan_request *scan_req = priv->scan_req;
					
					priv->scan_req = NULL;
					
					cfg80211_scan_done(scan_req, true);		
				}
			}
			break;
		case CFG80211_SCAN_DONE:
			{
				//NDEBUG2("cfg80211_event [CFG80211_SCAN_DONE][%d]\n", event);		
				priv->ss_req_ongoing = 0;
				priv->site_survey->count_backup = priv->site_survey->count;
				memcpy(priv->site_survey->bss_backup, priv->site_survey->bss, sizeof(struct bss_desc)*priv->site_survey->count);
#if defined(P2P_SUPPORT)
		        if(rtk_p2p_is_enabled(priv)==CFG80211_P2P){
		            rtk_p2p_set_role(priv,priv->p2pPtr->pre_p2p_role);            
		            rtk_p2p_set_state(priv,priv->p2pPtr->pre_p2p_state);

		            NDEBUG2("role[%d]\n",rtk_p2p_get_role(priv));
		            NDEBUG2("state[%d]\n",rtk_p2p_get_state(priv));                
		        }
#endif  
		        
				if (priv->scan_req)
				{
					struct cfg80211_scan_request *scan_req = priv->scan_req;
					priv->scan_req = NULL;				
					cfg80211_scan_done(scan_req, false);
				}
			}
			break;
		case CFG80211_DEL_STA:
			NDEBUG("cfg80211_event [CFG80211_DEL_STA][%d]\n", event);
			cfg80211_del_sta(priv->dev, mac, GFP_KERNEL);
			break;
		case CFG80211_RADAR_CAC_FINISHED:
			NDEBUG("cfg80211_event [CFG80211_RADAR_CAC_FINISHED][%d]\n", event);
#ifdef CONFIG_OPENWRT_SDK		
			cfg80211_cac_event(priv->dev, priv->pshare->dfs_chan_def, NL80211_RADAR_CAC_FINISHED, GFP_KERNEL);
#else			
			cfg80211_cac_event(priv->dev, NL80211_RADAR_CAC_FINISHED, GFP_KERNEL);					
#endif			
			break;
		case CFG80211_RADAR_DETECTED:
			NDEBUG("cfg80211_event [CFG80211_RADAR_DETECTED][%d]\n", event);
			cfg80211_radar_event(wiphy, priv->pshare->dfs_chan_def, GFP_KERNEL);
			break;
		default:
			NDEBUG("cfg80211_event [Unknown Event !!][%d]\n", event);            
    }

#ifdef SMP_SYNC
	SMP_UNLOCK_CFG80211(flags);
#endif

	return -1;
}


#endif

#if 0
void realtek_ap_calibration(struct rtl8192cd_priv	*priv)
{
	NLENTER;
	
#if 0
	unsigned char CCK_A[3] = {0x2a,0x2a,0x28};
	unsigned char CCK_B[3] = {0x2a,0x2a,0x28};
	unsigned char HT40_A[3] = {0x2b,0x2b,0x29};
	unsigned char HT40_B[3] = {0x2b,0x2b,0x29};
	unsigned char DIFF_HT40_2S[3] = {0x0,0x0,0x0};
	unsigned char DIFF_20[3] = {0x02,0x02,0x02};
	unsigned char DIFF_OFDM[3] = {0x04,0x04,0x04};
	unsigned int thermal = 0x19;
	unsigned int crystal = 32;
#else
	unsigned char CCK_A[3] = {0x2b,0x2a,0x29};
	unsigned char CCK_B[3] = {0x2b,0x2a,0x29};
	unsigned char HT40_A[3] = {0x2c,0x2b,0x2a};
	unsigned char HT40_B[3] = {0x2c,0x2b,0x2a};
	unsigned char DIFF_HT40_2S[3] = {0x0,0x0,0x0};
	unsigned char DIFF_20[3] = {0x02,0x02,0x02};
	unsigned char DIFF_OFDM[3] = {0x04,0x04,0x04};
	unsigned int thermal = 0x16;
	unsigned int crystal = 32;
#endif

	int tmp = 0;
	int tmp2 = 0;

	for(tmp = 0; tmp <=13; tmp ++)
	{
		if(tmp < 3)
			tmp2 = 0;
		else if(tmp < 9)
			tmp2 = 1;
		else
			tmp2 = 2;
	
		priv->pmib->dot11RFEntry.pwrlevelCCK_A[tmp] = CCK_A[tmp2];
		priv->pmib->dot11RFEntry.pwrlevelCCK_B[tmp] = CCK_B[tmp2];
		priv->pmib->dot11RFEntry.pwrlevelHT40_1S_A[tmp] = HT40_A[tmp2];
		priv->pmib->dot11RFEntry.pwrlevelHT40_1S_B[tmp] = HT40_B[tmp2];
		priv->pmib->dot11RFEntry.pwrdiffHT40_2S[tmp] = DIFF_HT40_2S[tmp2];
		priv->pmib->dot11RFEntry.pwrdiffHT20[tmp] = DIFF_20[tmp2];
		priv->pmib->dot11RFEntry.pwrdiffOFDM[tmp] = DIFF_OFDM[tmp2];
	}

	priv->pmib->dot11RFEntry.ther = thermal;
	priv->pmib->dot11RFEntry.xcap = crystal;

	NLEXIT;
}
#endif


//mark_swc	
static void rtk_set_phy_channel(struct rtl8192cd_priv *priv,unsigned int channel,unsigned int bandwidth,unsigned int chan_offset)
{
    NDEBUG3("ch[%d]bw[%d]offset[%d]\n",channel,bandwidth,chan_offset);
	//priv , share  part
	priv->pshare->CurrentChannelBW = bandwidth;
	priv->pshare->offset_2nd_chan =chan_offset ;

	// wifi chanel  hw settting  API
	SwBWMode(priv, priv->pshare->CurrentChannelBW, priv->pshare->offset_2nd_chan);
	SwChnl(priv, channel, priv->pshare->offset_2nd_chan);
	//printk("rtk_set_phy_channel end !!!\n  chan=%d \n",channel );

}

static void rtk_get_band_capa(struct rtl8192cd_priv *priv,BOOLEAN *band_2gig ,BOOLEAN *band_5gig)
{
	//default register as 2.4GHz
	*band_2gig = true;				
	*band_5gig = false;
	
	if ((GET_CHIP_VER(priv) == VERSION_8812E) || (GET_CHIP_VER(priv) == VERSION_8814A) || (GET_CHIP_VER(priv) == VERSION_8822B))
	{
#if defined(CONFIG_WLAN_HAL_8814AE)
		if(priv->pshare->is5g)
#endif
		{
			*band_2gig = false;
			*band_5gig = true;
		}
	}	
	else if (GET_CHIP_VER(priv) == VERSION_8192D) 
	{
		*band_2gig = false;	
		*band_5gig = true;
	}
	else if (GET_CHIP_VER(priv) == VERSION_8881A)
	{
#if defined(CONFIG_RTL_8881A_SELECTIVE)
		//8881A selective mode
		*band_2gig = true;
		*band_5gig = true;
#else
		//use pcie slot 0 for 2.4G 88E/92E, 8881A is 5G now
		*band_2gig = false;
		*band_5gig = true;
#endif
	}
	//mark_sel
	//if 881a , then it is possible to  *band_2gig = true ,*band_5gig = true in selective mode(FLAG?)
	//FIXME 
}

void realtek_ap_default_config(struct rtl8192cd_priv *priv) 
{	
	//short GI default
	priv->pmib->dot11nConfigEntry.dot11nShortGIfor20M = 1;
	priv->pmib->dot11nConfigEntry.dot11nShortGIfor40M = 1;
    priv->pmib->dot11nConfigEntry.dot11nShortGIfor80M = 1;
	//APMDU
	priv->pmib->dot11nConfigEntry.dot11nAMPDU = 1;	

#ifdef MBSSID
	if(IS_ROOT_INTERFACE(priv))
	{
		priv->pmib->miscEntry.vap_enable = 1; //eric-vap //eric-brsc
	}
	else
	{	
#if 0
		if(IS_VAP_INTERFACE(priv))
		{
			struct rtl8192cd_priv *priv_root = GET_ROOT(priv);
			struct rtl8192cd_priv *priv_vxd = GET_VXD_PRIV(priv_root);
			unsigned char is_vxd_running = 0; 

			if(priv_vxd)
				is_vxd_running = netif_running(priv_vxd->dev);
			
			if(priv_root->pmib->miscEntry.vap_enable == 0)
			{
				priv_root->pmib->miscEntry.vap_enable = 1;

				if(is_vxd_running)
				rtl8192cd_close(priv_vxd->dev);
				
				rtl8192cd_close(priv_root->dev);
				rtl8192cd_open(priv_root->dev);	

				if(is_vxd_running)
				rtl8192cd_open(priv_vxd->dev);	

			}
		}
#endif	
		//vif copy settings from root
		priv->pmib->dot11BssType.net_work_type = GET_ROOT(priv)->pmib->dot11BssType.net_work_type;
		priv->pmib->dot11RFEntry.phyBandSelect = GET_ROOT(priv)->pmib->dot11RFEntry.phyBandSelect;
		priv->pmib->dot11RFEntry.dot11channel = GET_ROOT(priv)->pmib->dot11RFEntry.dot11channel; 
		priv->pmib->dot11nConfigEntry.dot11nUse40M = GET_ROOT(priv)->pmib->dot11nConfigEntry.dot11nUse40M;
		priv->pmib->dot11nConfigEntry.dot11n2ndChOffset = GET_ROOT(priv)->pmib->dot11nConfigEntry.dot11n2ndChOffset;

		priv->pmib->dot11OperationEntry.dot11FragmentationThreshold = GET_ROOT(priv)->pmib->dot11OperationEntry.dot11FragmentationThreshold;
		priv->pmib->dot11OperationEntry.dot11RTSThreshold = GET_ROOT(priv)->pmib->dot11OperationEntry.dot11RTSThreshold;
		priv->pmib->dot11OperationEntry.dot11ShortRetryLimit = GET_ROOT(priv)->pmib->dot11OperationEntry.dot11ShortRetryLimit;
		priv->pmib->dot11OperationEntry.dot11LongRetryLimit = GET_ROOT(priv)->pmib->dot11OperationEntry.dot11LongRetryLimit;
	}
#endif

}

//mark_priv
#define RTK_PRIV_BW_5M 1 
#define RTK_PRIV_BW_10M 2
#define RTK_PRIV_BW_80M_MINUS 3
#define RTK_PRIV_BW_80M_PLUS 4

static inline int is_hw_vht_support(struct rtl8192cd_priv *priv)
{	
	int support=0;
	
	if (GET_CHIP_VER(priv) == VERSION_8812E) 
		support=1;
	else if (GET_CHIP_VER(priv) == VERSION_8881A) 
		support=1;

	return support;
}
//priv low bandwidth
static inline int is_hw_lbw_support(struct rtl8192cd_priv *priv)
{	
	int support=0;
	
	if (GET_CHIP_VER(priv) == VERSION_8812E) 
		support=1;
#if defined(CONFIG_WLAN_HAL_8192EE)
	if ((GET_CHIP_VER(priv) == VERSION_8192E) && (_GET_HAL_DATA(priv)->cutVersion == ODM_CUT_C))
		support=1;
#endif
	if(!support)
		printk("This IC NOT support 5M10M !! \n");

	return support;
}

static inline int convert_privBW(char *str_bw) //mark_priv
{
	int priv_bw=0;

    if(!strcmp(str_bw,"5M"))
		priv_bw = RTK_PRIV_BW_5M;
    else if(!strcmp(str_bw,"10M"))	
		priv_bw = RTK_PRIV_BW_10M;
    //future 160M 
   	
    return priv_bw;
}

int check_5M10M_config(struct rtl8192cd_priv *priv)
{

	int priv_bw=0;	
	int ret = 0;

	priv_bw = convert_privBW(priv->pshare->rf_ft_var.rtk_uci_PrivBandwidth);
	//printk("rtk_set_channel_mode , priv_band= %s , val=%d \n", priv->pshare->rf_ft_var.rtk_uci_PrivBandwidth, priv_bw);

	//first check if priv_band is set			  
	if(priv_bw)
	{
		//check 5/10M	
		if( (priv_bw == RTK_PRIV_BW_10M) && is_hw_lbw_support(priv))
		{
			priv->pmib->dot11nConfigEntry.dot11nUse40M = HT_CHANNEL_WIDTH_10;
			priv->pmib->dot11nConfigEntry.dot11n2ndChOffset = HT_2NDCH_OFFSET_DONTCARE;
			ret = 1;
			NDEBUG("Force config bandwidth=10M\n");
		}
		else if( (priv_bw == RTK_PRIV_BW_5M) && is_hw_lbw_support(priv))			
		{
			priv->pmib->dot11nConfigEntry.dot11nUse40M = HT_CHANNEL_WIDTH_5;
			priv->pmib->dot11nConfigEntry.dot11n2ndChOffset = HT_2NDCH_OFFSET_DONTCARE;
			ret = 1;
			NDEBUG("Force config bandwidth=5M\n");
		}
		else			
			NDEBUG("No such priv channel type !!!\n");
		
	}

	return ret;

}

int is_support_ac(struct rtl8192cd_priv *priv)
{
	int ret=0;

	switch(GET_CHIP_VER(priv)) {
		case VERSION_8812E:
		case VERSION_8881A:
			ret=1;
			break;
#if defined(CONFIG_WLAN_HAL_8814AE)
		case VERSION_8814A:
			if(priv->pshare->is5g)
				ret=1;
#endif
	}

	return ret;
}

static void rtk_set_band_mode(struct rtl8192cd_priv *priv,enum ieee80211_band band ,enum nl80211_chan_width channel_width) 
{
    NDEBUG2("\n");

	if(band == IEEE80211_BAND_2GHZ)
	{
		priv->pmib->dot11BssType.net_work_type = WIRELESS_11B|WIRELESS_11G;
		priv->pmib->dot11RFEntry.phyBandSelect = PHY_BAND_2G;
	}
	else if(band == IEEE80211_BAND_5GHZ)
	{
		priv->pmib->dot11BssType.net_work_type = WIRELESS_11A;
		priv->pmib->dot11RFEntry.phyBandSelect = PHY_BAND_5G;
	}
	if(channel_width != NL80211_CHAN_WIDTH_20_NOHT) {
		priv->pmib->dot11BssType.net_work_type |= WIRELESS_11N;
		//configure AMPDU for client mode
		priv->pmib->dot11nConfigEntry.dot11nAMPDU = 1;
		//configure shortGI for client mode
		priv->pmib->dot11nConfigEntry.dot11nShortGIfor20M = priv->pmib->dot11nConfigEntry.dot11nShortGIfor40M = 1;
	} else {
		if((OPMODE & WIFI_AP_STATE) && !under_apmode_repeater(priv))
			priv->rtk->keep_legacy = 1;
	}

	if(channel_width == NL80211_CHAN_WIDTH_80 || is_support_ac(priv)) {
		priv->pmib->dot11BssType.net_work_type |= WIRELESS_11AC;
#ifdef CONFIG_WLAN_HAL_8814AE
		if(GET_CHIP_VER(priv) == VERSION_8814A)
			priv->pmib->dot11nConfigEntry.dot11nAMSDU = 2;
#endif
		//configure shortGI for client mode
		priv->pmib->dot11nConfigEntry.dot11nShortGIfor80M = 1;
	}

#ifdef UNIVERSAL_REPEATER
	if(IS_ROOT_INTERFACE(priv) && priv->pvxd_priv)
	{
		priv->pvxd_priv->pmib->dot11BssType.net_work_type = priv->pmib->dot11BssType.net_work_type;
		priv->pvxd_priv->pmib->dot11RFEntry.phyBandSelect = priv->pmib->dot11RFEntry.phyBandSelect;
		priv->pvxd_priv->pmib->dot11nConfigEntry.dot11nAMPDU = priv->pmib->dot11nConfigEntry.dot11nAMPDU;
		priv->pvxd_priv->pmib->dot11nConfigEntry.dot11nSTBC = priv->pmib->dot11nConfigEntry.dot11nSTBC;
		priv->pvxd_priv->pmib->dot11nConfigEntry.dot11nShortGIfor20M = priv->pmib->dot11nConfigEntry.dot11nShortGIfor20M;
		priv->pvxd_priv->pmib->dot11nConfigEntry.dot11nShortGIfor40M = priv->pmib->dot11nConfigEntry.dot11nShortGIfor40M;
		priv->pvxd_priv->pmib->dot11nConfigEntry.dot11nShortGIfor80M = priv->pmib->dot11nConfigEntry.dot11nShortGIfor80M;
#ifdef CONFIG_WLAN_HAL_8814AE
		if(GET_CHIP_VER(priv) == VERSION_8814A)
			priv->pvxd_priv->pmib->dot11nConfigEntry.dot11nAMSDU = priv->pmib->dot11nConfigEntry.dot11nAMSDU;
#endif
	}
#endif
}

static void rtk_set_channel_mode(struct rtl8192cd_priv *priv, struct cfg80211_chan_def *chandef)
{
	int config_BW5m10m=0;

	config_BW5m10m = check_5M10M_config(priv);

	//printk("[%s]rtk_set_channel_mode , priv_band= %s , val=%d \n", priv->dev->name, priv->pshare->rf_ft_var.rtk_uci_PrivBandwidth);

	//first check if priv_band is set  			  
	if(!config_BW5m10m)
	{
		//normal channel setup path from cfg80211
		if(chandef->width == NL80211_CHAN_WIDTH_40) {
			//printk("NL80211_CHAN_WIDTH_40\n");
			priv->pshare->is_40m_bw = HT_CHANNEL_WIDTH_20_40;
			priv->pmib->dot11nConfigEntry.dot11nUse40M = HT_CHANNEL_WIDTH_20_40;
			if (chandef->center_freq1 > chandef->chan->center_freq) {
				//printk("NL80211_CHAN_WIDTH_40-PLUS\n");
				priv->pmib->dot11nConfigEntry.dot11n2ndChOffset = HT_2NDCH_OFFSET_ABOVE; //above
			} else {
				//printk("NL80211_CHAN_WIDTH_40-MINUS\n");
				priv->pmib->dot11nConfigEntry.dot11n2ndChOffset = HT_2NDCH_OFFSET_BELOW; //below
			}
		} else if(chandef->width == NL80211_CHAN_WIDTH_80) {
			//printk("NL80211_CHAN_WIDTH_80\n");
			priv->pshare->is_40m_bw = HT_CHANNEL_WIDTH_80;
			priv->pmib->dot11nConfigEntry.dot11nUse40M = HT_CHANNEL_WIDTH_80;

			if (chandef->center_freq1 > chandef->chan->center_freq) {
				priv->pmib->dot11nConfigEntry.dot11n2ndChOffset = HT_2NDCH_OFFSET_ABOVE; //dontcare
			} else {
				priv->pmib->dot11nConfigEntry.dot11n2ndChOffset = HT_2NDCH_OFFSET_BELOW; //dontcare
			}
		} else {
#if 0
			if(chandef->width == NL80211_CHAN_WIDTH_20 || chandef->width == NL80211_CHAN_WIDTH_20_NOHT)
				printk("NL80211_CHAN_WIDTH_20\/NL80211_CHAN_WIDTH_20_NOHT\n");
            else
                printk("Unknown bandwidth: %d, use 20Mhz be default\n", chandef->width);
#endif
			priv->pshare->is_40m_bw = HT_CHANNEL_WIDTH_20;
			priv->pmib->dot11nConfigEntry.dot11nUse40M = HT_CHANNEL_WIDTH_20;
			priv->pmib->dot11nConfigEntry.dot11n2ndChOffset = HT_2NDCH_OFFSET_DONTCARE;
		}

#ifdef UNIVERSAL_REPEATER
		if(IS_ROOT_INTERFACE(priv) && priv->pvxd_priv)
		{
			priv->pvxd_priv->pmib->dot11nConfigEntry.dot11nUse40M = priv->pmib->dot11nConfigEntry.dot11nUse40M;
			priv->pvxd_priv->pmib->dot11nConfigEntry.dot11n2ndChOffset = priv->pmib->dot11nConfigEntry.dot11n2ndChOffset;
		}
#endif

	}
}

void realtek_ap_config_apply(struct rtl8192cd_priv	*priv)
{
    #if	0	//def P2P_SUPPORT
    int keep_go_state=0;

    if(priv->pmib->p2p_mib.p2p_enabled==CFG80211_P2P){
        keep_go_state=1;
    }
    #endif

	NLENTER;

	if(under_apmode_repeater(priv) && (GET_VXD_PRIV(priv)->pmib->dot11OperationEntry.opmode & WIFI_ASOC_STATE)) {
		NLINFO("Repeater! STA is alive, skip down-up\n");
	} else {
		NLINFO("[%s][down up]\n",priv->dev->name);

		rtl8192cd_close(priv->dev);
		priv->dev->flags &= ~IFF_UP;
		#if 0 //def P2P_SUPPORT    
		if(keep_go_state){
			NDEBUG3("[P2P GO mode]\n");        
			priv->pmib->p2p_mib.p2p_enabled=CFG80211_P2P;  
		}
		#endif
		rtl8192cd_open(priv->dev);
	}

}

int realtek_cfg80211_ready(struct rtl8192cd_priv	*priv)
{

	if (netif_running(priv->dev))
		return 1;
	else
		return 0;
}


void realtek_reset_security(struct rtl8192cd_priv *priv)
{
	NLENTER; 
	priv->pmib->dot1180211AuthEntry.dot11AuthAlgrthm = 0;
	priv->pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm = 0;
	priv->pmib->dot1180211AuthEntry.dot11EnablePSK = 0;
	priv->pmib->dot118021xAuthEntry.dot118021xAlgrthm = 0;
	priv->pmib->dot1180211AuthEntry.dot11WPACipher = 0;
	priv->pmib->dot1180211AuthEntry.dot11WPA2Cipher = 0;
}

void realtek_auth_wep(struct rtl8192cd_priv *priv, int cipher)
{
	//_eric_nl ?? wep auto/shared/open ??
	NLENTER;
	priv->pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm = cipher;
	priv->pmib->dot1180211AuthEntry.dot11EnablePSK = 0;
	priv->pmib->dot118021xAuthEntry.dot118021xAlgrthm = 0;
	priv->pmib->dot1180211AuthEntry.dot11WPACipher = 0;
	priv->pmib->dot1180211AuthEntry.dot11WPA2Cipher = 0;
	NLEXIT;
}

void realtek_auth_wpa(struct rtl8192cd_priv *priv, int wpa, int psk, int cipher)
{
	int wpa_cipher;

	// bit0-wep64, bit1-tkip, bit2-wrap,bit3-ccmp, bit4-wep128
	if(cipher & _TKIP_PRIVACY_)
		wpa_cipher |= BIT(1);
	if(cipher & _CCMP_PRIVACY_)
		wpa_cipher |= BIT(3);
	
	NLENTER;
	NDEBUG3("%s wpa[%d] psk[%d] cipher[0x%x] wpa_cipher[0x%x]\n",priv->dev->name ,wpa ,psk ,cipher ,wpa_cipher);
	priv->pmib->dot1180211AuthEntry.dot11AuthAlgrthm = 2;
	priv->pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm = 2;

	if(psk)
    	priv->pmib->dot1180211AuthEntry.dot11EnablePSK = wpa;
	
	priv->pmib->dot118021xAuthEntry.dot118021xAlgrthm = 1;

	if(wpa& BIT(0))
    	priv->pmib->dot1180211AuthEntry.dot11WPACipher = wpa_cipher;
	if(wpa& BIT(1))
    	priv->pmib->dot1180211AuthEntry.dot11WPA2Cipher = wpa_cipher;
	NLEXIT;
}


void realtek_set_security(struct rtl8192cd_priv *priv, struct rtknl *rtk, struct cfg80211_crypto_settings crypto)
{
	int wpa = 0;
	int psk = 0;
	int cipher = 0;
	int i = 0;

	NDEBUG2("n_akm_suites=[%d], n_ciphers_pairwise=[%d]\n", crypto.n_akm_suites, crypto.n_ciphers_pairwise);
	for (i = 0; i < crypto.n_akm_suites; i++) {
		switch (crypto.akm_suites[i]) {
		case WLAN_AKM_SUITE_8021X:
			psk = 0;
			if (crypto.wpa_versions & NL80211_WPA_VERSION_1)
				wpa |= BIT(0);
			if (crypto.wpa_versions & NL80211_WPA_VERSION_2)
				wpa |= BIT(1);
			break;
		case WLAN_AKM_SUITE_PSK:
			psk = 1;
			if (crypto.wpa_versions & NL80211_WPA_VERSION_1)
				wpa |= BIT(0);
			if (crypto.wpa_versions & NL80211_WPA_VERSION_2)
				wpa |= BIT(1);
			break;
		}
	}
	
//_eric_nl ?? multiple ciphers ??
	for (i = 0; i < crypto.n_ciphers_pairwise; i++) {
		switch (crypto.ciphers_pairwise[i]) {
		case WLAN_CIPHER_SUITE_WEP40:
			rtk->cipher = WLAN_CIPHER_SUITE_WEP40;
			realtek_auth_wep(priv, _WEP_40_PRIVACY_);
			break;
		case WLAN_CIPHER_SUITE_WEP104:
			rtk->cipher = WLAN_CIPHER_SUITE_WEP104;
			realtek_auth_wep(priv, _WEP_104_PRIVACY_);
			break;
		case WLAN_CIPHER_SUITE_TKIP:
			rtk->cipher |= WLAN_CIPHER_SUITE_TKIP;
			cipher |= _TKIP_PRIVACY_;
			NDEBUG3("TKIP[%d]\n", i);
			break;
		case WLAN_CIPHER_SUITE_CCMP:
			rtk->cipher |= WLAN_CIPHER_SUITE_CCMP;
			cipher |= _CCMP_PRIVACY_;
			NDEBUG3("CCMP[%d]\n", i);
			break;
		}
	}

	if(wpa)
		realtek_auth_wpa(priv, wpa, psk, cipher);

#if 1
	switch (crypto.cipher_group) {
		case WLAN_CIPHER_SUITE_WEP40:
		case WLAN_CIPHER_SUITE_WEP104:
			NDEBUG3("WEP GROUP\n");
			break;
		case WLAN_CIPHER_SUITE_TKIP:
			NDEBUG3("TKIP GROUP\n");
			break;
		case WLAN_CIPHER_SUITE_CCMP:
			NDEBUG3("CCMP GROUP\n");
			break;
		case WLAN_CIPHER_SUITE_SMS4:
			NDEBUG3("WAPI GROUP\n");
			break;
		default:
			NDEBUG3("NONE GROUP\n");
			break;
	}
#endif
}

unsigned int realtek_get_key_from_sta(struct rtl8192cd_priv *priv, struct stat_info *pstat, struct key_params *params)
{
	unsigned int cipher = 0; 
	struct Dot11EncryptKey *pEncryptKey;

	//_eric_cfg ?? key len data seq for get_key ??
	if(pstat == NULL)
	{
		cipher = priv->pmib->dot11GroupKeysTable.dot11Privacy;
		pEncryptKey = &priv->pmib->dot11GroupKeysTable.dot11EncryptKey;
	}
	else
	{
		cipher = pstat->dot11KeyMapping.dot11Privacy;
		pEncryptKey = &pstat->dot11KeyMapping.dot11EncryptKey;
	}

	switch (cipher) {
	case _WEP_40_PRIVACY_:
		params->cipher = WLAN_CIPHER_SUITE_WEP40;
		params->key_len = 5;
		memcpy(params->key, pEncryptKey->dot11TTKey.skey, pEncryptKey->dot11TTKeyLen);
	case _WEP_104_PRIVACY_:
		params->cipher = WLAN_CIPHER_SUITE_WEP104;
		params->key_len = 10;
		memcpy(params->key, pEncryptKey->dot11TTKey.skey, pEncryptKey->dot11TTKeyLen);
	case _CCMP_PRIVACY_:
		params->cipher = WLAN_CIPHER_SUITE_CCMP;/*eric refine*/
		params->key_len = 32;
		memcpy(params->key, pEncryptKey->dot11TTKey.skey, pEncryptKey->dot11TTKeyLen);
		memcpy(params->key+16, pEncryptKey->dot11TMicKey1.skey, pEncryptKey->dot11TMicKeyLen);
	case _TKIP_PRIVACY_:
		params->cipher = WLAN_CIPHER_SUITE_TKIP;/*eric refine*/
		params->key_len = 32;
		memcpy(params->key, pEncryptKey->dot11TTKey.skey, pEncryptKey->dot11TTKeyLen);
		memcpy(params->key+16, pEncryptKey->dot11TMicKey1.skey, pEncryptKey->dot11TMicKeyLen);
		memcpy(params->key+24, pEncryptKey->dot11TMicKey2.skey, pEncryptKey->dot11TMicKeyLen);
	default:
		return -ENOTSUPP;
	}
}



void clear_wps_ies(struct rtl8192cd_priv *priv)
{
	priv->pmib->wscEntry.wsc_enable = 0;
	
	priv->pmib->wscEntry.beacon_ielen = 0;
	priv->pmib->wscEntry.probe_rsp_ielen = 0;
	priv->pmib->wscEntry.probe_req_ielen = 0;
	priv->pmib->wscEntry.assoc_ielen = 0;
}


/*void copy_wps_ie(struct rtl8192cd_priv *priv, unsigned char *wps_ie, unsigned char mgmt_type) move to 8192cd_util.c*/
/*the function can be replaced by rtk_cfg80211_set_wps_p2p_ie*/
//void rtk_set_ie(struct rtl8192cd_priv *priv, unsigned char *pies, unsigned int ies_len, unsigned char mgmt_type)


//static int rtw_cfg80211_set_probe_req_wpsp2pie(struct rtl8192cd_priv *priv, char *buf, int len)
int rtk_cfg80211_set_wps_p2p_ie(struct rtl8192cd_priv *priv, char *buf, int len, int mgmt_type)    
{
	int ret = 0;
	int wps_ielen = 0;
	u8 *wps_ie;
	u32	p2p_ielen = 0;	
	u8 *p2p_ie;
	//u32	wfd_ielen = 0;	
	//u8 *wfd_ie;
    u8* p2p_ie_listen_tag_ptr=NULL;	
	int p2p_ie_listen_tag;
    
    if(len<=0)
        return;
    
    NDEBUG2("mgmt_type=[%d]\n",mgmt_type);	    

    /*set WPS IE*/
	if((wps_ie = rtk_get_wps_ie(buf, len, NULL, &wps_ielen)))
	{
        copy_wps_ie(priv,wps_ie,mgmt_type);                                    
	}
    
#if defined(P2P_SUPPORT)
    /*set P2P IE*/
	if((p2p_ie = rtk_get_p2p_ie(buf, len, NULL, &p2p_ielen)))
	{
        copy_p2p_ie(priv,p2p_ie,mgmt_type);            

        if(mgmt_type == MGMT_PROBEREQ){
            /*check if  listen channel from cfg80211 equl my keep*/                
            p2p_ie_listen_tag_ptr = p2p_search_tag(p2p_ie, p2p_ielen ,TAG_LISTEN_CHANNEL, &p2p_ie_listen_tag);
            if(p2p_ie_listen_tag_ptr && p2p_ie_listen_tag==5){
                if(  priv->pmib->p2p_mib.p2p_listen_channel !=  p2p_ie_listen_tag_ptr[4]){
                    NDEBUG("listen ch no equl\n");
                    priv->pmib->p2p_mib.p2p_listen_channel =  p2p_ie_listen_tag_ptr[4];
                }
            }
        }
        
	}
#endif
    /*set WFD IE to do */	
}

void dump_ies(struct rtl8192cd_priv *priv, 
					unsigned char *pies, unsigned int ies_len, unsigned char mgmt_type)
{
	unsigned char *pie = pies;
	unsigned int len, total_len = 0;
	int i = 0;

	while(1)
	{
		len = pie[1];

		total_len += (len+2);	
		if(total_len > ies_len)
		{
			printk("Exceed !!\n");
			break;
		}

		if(pie[0] == _WPS_IE_)
			copy_wps_ie(priv, pie, mgmt_type);
		
		//printk("[Tag=0x%02x Len=%d(0x%x)]\n", pie[0], len, len);
		pie+=2; 
		
#if 0		
		for(i=0; i<len; i++)
		{
			if((i%10) == 9)
				printk("\n");
			
			printk("%02x ", pie[i]);
		}

		printk("\n");
#endif

		pie+=len;
		
		if(total_len == ies_len)
		{
			//printk("Done \n");
			break;
		}
		
	}

}

void realtek_set_ies_apmode(struct rtl8192cd_priv *priv, struct cfg80211_beacon_data *info)
{

	clear_wps_ies(priv);

	if(info->beacon_ies)
	{
		NDEBUG2("beacon_ies_len[%d]\n", info->beacon_ies_len);
		rtk_cfg80211_set_wps_p2p_ie(priv, info->beacon_ies, info->beacon_ies_len, MGMT_BEACON);
	}
	if(info->proberesp_ies)
	{
		NDEBUG2("proberesp_ies_len[%d]\n", info->proberesp_ies_len);
		rtk_cfg80211_set_wps_p2p_ie(priv, info->proberesp_ies, info->proberesp_ies_len, MGMT_PROBERSP);
	}
	if(info->assocresp_ies)
	{
		NDEBUG2("assocresp_ies_len[%d]\n", info->assocresp_ies_len);
		rtk_cfg80211_set_wps_p2p_ie(priv, info->assocresp_ies, info->assocresp_ies_len, MGMT_ASSOCRSP);
	}
}

static int realtek_set_bss(struct rtl8192cd_priv *priv, struct cfg80211_ap_settings *info)
{
	NDEBUG3("SSID[%s]\n", info->ssid);
	memcpy(SSID, info->ssid, info->ssid_len);	
	SSID_LEN = info->ssid_len;
	
	switch(info->hidden_ssid)
	{	
		case NL80211_HIDDEN_SSID_NOT_IN_USE:
			HIDDEN_AP=0;
			break;
		case NL80211_HIDDEN_SSID_ZERO_CONTENTS:
			HIDDEN_AP=1;
			break;
		case NL80211_HIDDEN_SSID_ZERO_LEN:
			HIDDEN_AP=2;
            break;
		default:
            NDEBUG("fail, unknown hidden SSID option[%s]\n", info->hidden_ssid);
			return -EOPNOTSUPP;  
	}

	priv->pmib->dot11StationConfigEntry.dot11BeaconPeriod = info->beacon_interval;

	return 0;
}

static int realtek_set_auth_type(struct rtl8192cd_priv *priv, enum nl80211_auth_type auth_type)
{
	//NDEBUG3("auth_type[0x%02X]\n", auth_type);

	switch (auth_type) {
	case NL80211_AUTHTYPE_OPEN_SYSTEM:
    	NDEBUG3("NL80211_AUTHTYPE_OPEN_SYSTEM\n");
		break;
	case NL80211_AUTHTYPE_SHARED_KEY:
    	NDEBUG3("NL80211_AUTHTYPE_SHARED_KEY\n");
		priv->pmib->dot1180211AuthEntry.dot11AuthAlgrthm = 1;
		break;
	case NL80211_AUTHTYPE_NETWORK_EAP:
    	NDEBUG3("NL80211_AUTHTYPE_NETWORK_EAP\n");
		break;
	case NL80211_AUTHTYPE_AUTOMATIC:
    	NDEBUG3("NL80211_AUTHTYPE_AUTOMATIC\n");
		priv->pmib->dot1180211AuthEntry.dot11AuthAlgrthm = 2;
		break;

	default:
    	NDEBUG("no support auth type[02%02x]\n",auth_type);
		return -ENOTSUPP;
		}

	return 0;
}



static int realtek_change_beacon(struct wiphy *wiphy, struct net_device *dev,
				struct cfg80211_beacon_data *beacon)
{
	struct rtknl *rtk = wiphy_priv(wiphy);
	struct rtl8192cd_priv *priv = realtek_get_priv(wiphy, dev);

	NLENTER;
	
	if (!realtek_cfg80211_ready(priv))
		return -EIO;

	if ((OPMODE & WIFI_AP_STATE) == 0)
		return -EOPNOTSUPP;

	realtek_set_ies_apmode(priv, beacon);/*cfg p2p*/

	NLEXIT;

	return 0; 

	}

static int realtek_cfg80211_del_beacon(struct wiphy *wiphy, struct net_device *dev)
{
	struct rtknl *rtk = wiphy_priv(wiphy);
	struct rtl8192cd_priv *priv = realtek_get_priv(wiphy, dev);

	NLENTER;

	if(priv->pmib->p2p_mib.p2p_enabled){
		OPMODE = WIFI_STATION_STATE;
	    priv->pmib->p2p_mib.p2p_enabled=0;        
	}

	if (OPMODE & WIFI_AP_STATE == 0)
		return -EOPNOTSUPP;

	rtl8192cd_close(priv->dev);
	priv->dev->flags &= ~IFF_UP;

	NLEXIT;
	return 0;
}


static int realtek_stop_ap(struct wiphy *wiphy, struct net_device *dev)
{
	struct rtknl *rtk = wiphy_priv(wiphy);
	struct rtl8192cd_priv *priv = realtek_get_priv(wiphy, dev);
	int ret = 0;

	NLENTER;

	if(under_apmode_repeater(priv)) {
		NLINFO("Repeater! Do nothing\n");
	} else {
		ret = realtek_cfg80211_del_beacon(wiphy, dev);
	}

	NLEXIT;
	return ret;
}

#if 0
static int realtek_cfg80211_add_beacon(struct wiphy *wiphy, struct net_device *dev,
				struct beacon_parameters *info)
{
	struct rtknl *rtk = wiphy_priv(wiphy);
	struct rtl8192cd_priv *priv = realtek_get_priv(wiphy, dev);

	NLENTER;
	
	realtek_ap_beacon(wiphy, dev, info, true);
	realtek_ap_config_apply(priv);

	NLEXIT;
	return 0;
}

//_eric_nl ?? what's the diff between st & add beacon??
static int realtek_cfg80211_set_beacon(struct wiphy *wiphy, struct net_device *dev,
				struct beacon_parameters *info)
{
	struct rtknl *rtk = wiphy_priv(wiphy);
	struct rtl8192cd_priv *priv = realtek_get_priv(wiphy, dev);

	NLENTER;

	realtek_ap_beacon(wiphy, dev, info, false);
	
	NLEXIT;
	return 0;

}

//_eric_nl ?? what's the purpose of del_beacon ??
static int realtek_cfg80211_del_beacon(struct wiphy *wiphy, struct net_device *dev)
{
	struct rtknl *rtk = wiphy_priv(wiphy);
	struct rtl8192cd_priv *priv = realtek_get_priv(wiphy, dev);

	NLENTER;

	if (OPMODE & WIFI_AP_STATE == 0)
		return -EOPNOTSUPP;
	if (priv->assoc_num == 0)
		return -ENOTCONN;

	rtl8192cd_close(priv->dev);

	NLEXIT;
	return 0;
}
#endif


static int realtek_cfg80211_set_channel(struct wiphy *wiphy, struct net_device *dev,
			      struct cfg80211_chan_def *chandef)
{
	struct rtknl *rtk = wiphy_priv(wiphy);
	struct rtl8192cd_priv *priv = realtek_get_priv(wiphy, NULL);
	int channel = 0;

	NLENTER;

	channel = ieee80211_frequency_to_channel(chandef->chan->center_freq);
	
	NDEBUG3("[%s]center_freq=[%u] channel=[%d] hw_value=[%u] bandwidth=[%d]\n", priv->dev->name,
		chandef->chan->center_freq, channel, chandef->chan->hw_value, chandef->width);

	priv->pmib->dot11RFEntry.dot11channel = channel;

	rtk_set_band_mode(priv,chandef->chan->band , chandef->width);
	rtk_set_channel_mode(priv,chandef);

//	realtek_ap_default_config(priv);
//	realtek_ap_config_apply(priv);

	NLEXIT;
	return 0;
}


//Not in ath6k
static int realtek_cfg80211_change_bss(struct wiphy *wiphy,
				struct net_device *dev,
				struct bss_parameters *params)
{
	struct rtknl *rtk = wiphy_priv(wiphy);
	struct rtl8192cd_priv *priv = realtek_get_priv(wiphy, dev);
	
	unsigned char dot11_rate_table[]={2,4,11,22,12,18,24,36,48,72,96,108,0};

	NLENTER;

#if 0
if (params->use_cts_prot >= 0) {
	sdata->vif.bss_conf.use_cts_prot = params->use_cts_prot;
	changed |= BSS_CHANGED_ERP_CTS_PROT;
}
#endif

	priv->pmib->dot11RFEntry.shortpreamble = params->use_short_preamble;
	changePreamble(priv, params->use_short_preamble);
#if 0
if (params->use_short_slot_time >= 0) {
	sdata->vif.bss_conf.use_short_slot =
		params->use_short_slot_time;
	changed |= BSS_CHANGED_ERP_SLOT;
}
#endif

	if (params->basic_rates) {
		int i, j;
		u32 rates = 0;

		//printk("rate = ");
		for (i = 0; i < params->basic_rates_len; i++) {
			int rate = params->basic_rates[i];
			//printk("%d ", rate);

			for (j = 0; j < 13; j++) {
				if ((dot11_rate_table[j]) == rate)
				{
					//printk("BIT(%d) ", j);
					rates |= BIT(j);
				}

			}
		}
		//printk("\n");
		priv->pmib->dot11StationConfigEntry.dot11BasicRates = rates;
	}

	NLEXIT;
	return 0;
}



#if 0

static int realtek_cfg80211_add_key(struct wiphy *wiphy, struct net_device *dev,
			     u8 key_idx, const u8 *mac_addr,
			     struct key_params *params)
{
	struct rtknl *rtk = wiphy_priv(wiphy);
	struct rtl8192cd_priv	*priv = rtk->priv;

	NLENTER;
	return 0;

}


static int realtek_cfg80211_del_key(struct wiphy *wiphy, struct net_device *dev,
			     u8 key_idx, const u8 *mac_addr)
{
	struct rtknl *rtk = wiphy_priv(wiphy);
	struct rtl8192cd_priv	*priv = rtk->priv;

	NLENTER;
	return 0;
}

static int realtek_cfg80211_get_key(struct wiphy *wiphy, struct net_device *dev,
			     u8 key_idx, const u8 *mac_addr, void *cookie,
			     void (*callback)(void *cookie,
					      struct key_params *params))
{
	struct rtknl *rtk = wiphy_priv(wiphy);
	struct rtl8192cd_priv	*priv = rtk->priv;

	NLENTER;
	return 0;
}

#else


void set_pairwise_key_for_ibss(struct rtl8192cd_priv *priv, union iwreq_data *wrqu)
{
	int i = 0;
	struct stat_info *pstat = NULL;
	struct ieee80211req_key *wk = (struct ieee80211req_key *)wrqu->data.pointer;

	printk("set_pairwise_key_for_ibss +++ \n");
	
	for(i=0; i<NUM_STAT; i++)
	{
		if(priv->pshare->aidarray[i] && (priv->pshare->aidarray[i]->used == TRUE)) 
		{
			pstat = get_stainfo(priv, priv->pshare->aidarray[i]->station.hwaddr);

			if(pstat)
			{
				memcpy(wk->ik_macaddr, priv->pshare->aidarray[i]->station.hwaddr, ETH_ALEN);
				rtl_net80211_setkey(priv->dev, NULL, wrqu, NULL);
			}
		}
	}
}

//#define TOTAL_CAM_ENTRY (priv->pshare->total_cam_entry)

#ifdef CPTCFG_CFG80211_MODULE
static int realtek_cfg80211_add_key(struct wiphy *wiphy, struct net_device *dev,
				   u8 key_index, BOOLEAN pairwise,
				   const u8 *mac_addr,
				   struct key_params *params)			   
{
	struct rtknl *rtk = wiphy_priv(wiphy);
	struct rtl8192cd_priv *priv = realtek_get_priv(wiphy, dev);
	union iwreq_data wrqu;
	struct ieee80211req_key wk;

	NLENTER;

	if (!realtek_cfg80211_ready(priv))
		return -EIO;

#if 0
	if (key_index > TOTAL_CAM_ENTRY) {
		NDEBUG("key index [%d] out of bounds\n", key_index);
		return -ENOENT;
	}

	if(mac_addr == NULL) {
		printk("NO MAC Address !!\n");
		return -ENOENT;;
	}
#endif

	memset(&wk, 0, sizeof(struct ieee80211req_key));

	wk.ik_keyix = key_index;

	if(mac_addr != NULL)
		memcpy(wk.ik_macaddr, mac_addr, ETH_ALEN);
	else
		memset(wk.ik_macaddr, 0, ETH_ALEN);

#if 1
	if (!pairwise) //in rtl_net80211_setkey(), group identification is by mac address
	{
		unsigned char	MULTICAST_ADD[6]={0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};

		memcpy(wk.ik_macaddr, MULTICAST_ADD, ETH_ALEN);
	}
#endif

	switch (params->cipher) {
	case WLAN_CIPHER_SUITE_WEP40:
	case WLAN_CIPHER_SUITE_WEP104:
		wk.ik_type = IEEE80211_CIPHER_WEP;
		//printk("WEP !!\n");
		break;
	case WLAN_CIPHER_SUITE_TKIP:
		wk.ik_type = IEEE80211_CIPHER_TKIP;
		//printk("TKIP !!\n");
		break;
	case WLAN_CIPHER_SUITE_CCMP:
		wk.ik_type = IEEE80211_CIPHER_AES_CCM;
		//printk("AES !!\n");
		break;
	default:
		return -EINVAL;
	}

#if 0
	switch (rtk->cipher) { //_eric_cfg ?? mixed mode ?? 
	case WLAN_CIPHER_SUITE_WEP40:
	case WLAN_CIPHER_SUITE_WEP104:
		wk.ik_type = IEEE80211_CIPHER_WEP;
		break;
	case WLAN_CIPHER_SUITE_TKIP:
		wk.ik_type = IEEE80211_CIPHER_TKIP;
		break;
	case WLAN_CIPHER_SUITE_CCMP:
		wk.ik_type = IEEE80211_CIPHER_AES_CCM;
		break;
	default:
		return -ENOTSUPP;
	}
#endif
	wk.ik_keylen = params->key_len;
	memcpy(wk.ik_keydata, params->key, params->key_len);

#if 0
{
	int tmp = 0;
	printk("keylen = %d: ", wk.ik_keylen);
	for(tmp = 0; tmp < wk.ik_keylen; tmp ++)		
		printk("%02x ", wk.ik_keydata[tmp]);
	printk("\n");
}

	//_eric_cfg ?? key seq is not used ??
	
	printk("[%s] add keyid = %d, mac = 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x\n",
			priv->dev->name , wk.ik_keyix, wk.ik_macaddr[0], wk.ik_macaddr[1], wk.ik_macaddr[2], 
				wk.ik_macaddr[3], wk.ik_macaddr[4], wk.ik_macaddr[5]);
	printk("type = 0x%x, flags = 0x%x, keylen = 0x%x \n"
			, wk.ik_type, wk.ik_flags, wk.ik_keylen);
#endif


	wrqu.data.pointer = &wk;
	
	rtl_net80211_setkey(priv->dev, NULL, &wrqu, NULL);

#if 1 //wrt-adhoc
	if(OPMODE & WIFI_ADHOC_STATE)
	{
		if(!pairwise)
			set_pairwise_key_for_ibss(priv, &wrqu);	//or need to apply set_default_key
	}
#endif

	NLEXIT;
	return 0;

}

static int realtek_cfg80211_del_key(struct wiphy *wiphy, struct net_device *dev,
				   u8 key_index, BOOLEAN pairwise,
				   const u8 *mac_addr)			   
{
	struct rtknl *rtk = wiphy_priv(wiphy);
	struct rtl8192cd_priv *priv = realtek_get_priv(wiphy, dev);
	union iwreq_data wrqu;
	struct ieee80211req_del_key wk;
	int ret=0;

	NLENTER;

	if(!pairwise)
	{
		NDEBUG2("No need to delete Groupe key !!\n");
		goto realtek_cfg80211_del_key_end;
	}


	if (!realtek_cfg80211_ready(priv)){
		NDEBUG("No realtek_cfg80211_ready !!\n");        
		ret = -EIO;
		goto realtek_cfg80211_del_key_end;
    }

#if 0
	if (key_index > TOTAL_CAM_ENTRY) {
		NDEBUG("key index %d out of bounds\n" ,key_index);
		return -ENOENT;
	}
#endif
	
 	memset(&wk, 0, sizeof(struct ieee80211req_del_key));

	wk.idk_keyix = key_index;

	if(mac_addr != NULL)
		memcpy(wk.idk_macaddr, mac_addr, ETH_ALEN);
	else
		memset(wk.idk_macaddr, 0, ETH_ALEN);

#if 0
	if (!pairwise) //in rtl_net80211_delkey(), group identification is by mac address
	{
		unsigned char	MULTICAST_ADD[6]={0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};
		unsigned char	GROUP_ADD[6]={0x0,0x0,0x0,0x0,0x0,0x0};

		if(OPMODE & WIFI_AP_STATE)
			memcpy(wk->idk_macaddr, GROUP_ADD, ETH_ALEN);
	}

	printk("keyid = %d, mac = 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x\n"
			, wk.idk_keyix, wk.idk_macaddr[0], wk.idk_macaddr[1], wk.idk_macaddr[2], 
				wk.idk_macaddr[3], wk.idk_macaddr[4], wk.idk_macaddr[5]);
#endif


	wrqu.data.pointer = &wk;
	
	rtl_net80211_delkey(priv->dev, NULL, &wrqu, NULL);

realtek_cfg80211_del_key_end:
	NLEXIT;
	return ret;

}



static int realtek_cfg80211_get_key(struct wiphy *wiphy, struct net_device *dev,
				   u8 key_index, BOOLEAN pairwise,
				   const u8 *mac_addr, void *cookie,
				   void (*callback) (void *cookie,
						     struct key_params *))						     
{
	struct rtknl *rtk = wiphy_priv(wiphy);
	struct rtl8192cd_priv *priv = realtek_get_priv(wiphy, dev);
	struct key_params params;
	struct stat_info	*pstat = NULL;
	unsigned int cipher = 0;
	u8 key[64] = {0};

	NLENTER;

	if(mac_addr)
		pstat = get_stainfo(priv, mac_addr);

	NDEBUG2("key_index [%d]\n",  key_index);


	if (!realtek_cfg80211_ready(priv))
		return -EIO;

#if 0
	if (key_index > TOTAL_CAM_ENTRY) {
		NDEBUG("key index [%d] out of bounds\n" ,  key_index);
		return -ENOENT;
	}
#endif

    #if 0
	if(pairwise)
	{
		pstat = get_stainfo(priv, mac_addr);
		if (pstat == NULL)
			return -ENOENT;
	}
    #endif

	memset(&params, 0, sizeof(params));
	params.key = key;
	realtek_get_key_from_sta(priv, pstat, &params);

	//_eric_cfg ?? key seq is not used ??
    #if 0
	params.seq_len = key->seq_len;
	params.seq = key->seq;
    #endif

	callback(cookie, &params);

	NLEXIT;

	return 0;
}

#endif

static int realtek_cfg80211_set_default_key(struct wiphy *wiphy,
					   struct net_device *dev,
					   u8 key_index, BOOLEAN unicast,
					   BOOLEAN multicast)						
{
	struct rtknl *rtk = wiphy_priv(wiphy);
	struct rtl8192cd_priv *priv = realtek_get_priv(wiphy, dev);

	NLENTER;
	NDEBUG2("defaukt key_index[%d] unicast[%d] multicast[%d] \n", key_index, unicast, multicast);
	NLEXIT;
	return 0;
}

static int realtek_cfg80211_set_default_mgmt_key(struct wiphy *wiphy,
					     struct net_device *dev,
					     u8 key_idx)
{
	struct rtknl *rtk = wiphy_priv(wiphy);
	struct rtl8192cd_priv *priv = realtek_get_priv(wiphy, dev);

	NLENTER;
	return 0;
}

//not in ath6k
static int realtek_cfg80211_auth(struct wiphy *wiphy, struct net_device *dev,
			  struct cfg80211_auth_request *req)
{
	struct rtknl *rtk = wiphy_priv(wiphy);
	struct rtl8192cd_priv *priv = realtek_get_priv(wiphy, dev);

	NLENTER;
	return 0;
}

static int realtek_cfg80211_assoc(struct wiphy *wiphy, struct net_device *dev,
			   struct cfg80211_assoc_request *req)
{
	struct rtknl *rtk = wiphy_priv(wiphy);
	struct rtl8192cd_priv *priv = realtek_get_priv(wiphy, dev);

	NLENTER;
	return 0;
}

static int realtek_cfg80211_deauth(struct wiphy *wiphy, struct net_device *dev,
			    struct cfg80211_deauth_request *req,
			    void *cookie)
{
	struct rtknl *rtk = wiphy_priv(wiphy);
	struct rtl8192cd_priv *priv = realtek_get_priv(wiphy, dev);

	NLENTER;
	return 0;
}

static int realtek_cfg80211_disassoc(struct wiphy *wiphy, struct net_device *dev,
			      struct cfg80211_disassoc_request *req,
			      void *cookie)
{
	struct rtknl *rtk = wiphy_priv(wiphy);
	struct rtl8192cd_priv *priv = realtek_get_priv(wiphy, dev);

	NLENTER;		
	return 0;
}

//Not in ath6k
static int realtek_cfg80211_add_station(struct wiphy *wiphy, struct net_device *dev,
				 u8 *mac, struct station_parameters *params)
{
	struct rtknl *rtk = wiphy_priv(wiphy);
	struct rtl8192cd_priv *priv = realtek_get_priv(wiphy, dev);

	NLENTER;

	NLEXIT;
	return 0;
}


void realtek_del_station(struct rtl8192cd_priv *priv, struct stat_info *pstat)
{
	unsigned long	flags;
    NDEBUG3("\n");
    if(pstat->state & WIFI_ASOC_STATE){
        // if we even received dis_assoc from this STA don't send dis_assoc to it again
        NDEBUG3("\n");        
    	issue_disassoc(priv, pstat->hwaddr, _RSON_AUTH_NO_LONGER_VALID_);
    }

	SAVE_INT_AND_CLI(flags);

	if (!SWCRYPTO && pstat->dot11KeyMapping.keyInCam) {
		if (CamDeleteOneEntry(priv, pstat->hwaddr, 0, 0)) {
			pstat->dot11KeyMapping.keyInCam = FALSE;
			pstat->tmp_rmv_key = TRUE;
			priv->pshare->CamEntryOccupied--;
		}
	}
	
	if (!list_empty(&pstat->asoc_list))
	{
		list_del_init(&pstat->asoc_list);
		if (pstat->expire_to > 0)
		{
			cnt_assoc_num(priv, pstat, DECREASE, (char *)__FUNCTION__);
			check_sta_characteristic(priv, pstat, DECREASE);
			LOG_MSG("A STA is rejected by nl80211 - %02X:%02X:%02X:%02X:%02X:%02X\n",
					pstat->hwaddr[0],pstat->hwaddr[1],pstat->hwaddr[2],pstat->hwaddr[3],pstat->hwaddr[4],pstat->hwaddr[5]);
		}
	}
	free_stainfo(priv, pstat);
	RESTORE_INT(flags);

}

//eric ?? can apply to disconnect ??
#ifdef OPENWRT_CC
static int realtek_cfg80211_del_station(struct wiphy *wiphy, struct net_device *dev, struct station_del_parameters *params)
#else
static int realtek_cfg80211_del_station(struct wiphy *wiphy, struct net_device *dev, u8 *mac)
#endif
{
	struct rtknl *rtk = wiphy_priv(wiphy);
	struct rtl8192cd_priv *priv = realtek_get_priv(wiphy, dev);
	struct stat_info	*pstat;
	int ret=0;
#ifdef OPENWRT_CC
	const u8 *mac = params->mac;
#endif

	NLENTER;
	
	pstat = get_stainfo(priv, mac);

	if (pstat == NULL){
		goto realtek_cfg80211_del_station_end;
    }
		
	NDEBUG("try disassoc sta[%02X%02X%02X%02X%02X%02X]\n",
		mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);

	realtek_del_station(priv, pstat);

realtek_cfg80211_del_station_end:
	NLEXIT;
	return ret;
}

static int realtek_cfg80211_change_station(struct wiphy *wiphy,
				    struct net_device *dev,
				    u8 *mac,
				    struct station_parameters *params)
{
	struct rtknl *rtk = wiphy_priv(wiphy);
	struct rtl8192cd_priv *priv = realtek_get_priv(wiphy, dev);
	struct stat_info *pstat = NULL;
	union iwreq_data wrqu;
	struct ieee80211req_mlme mlme;
	int err = 0;

	NLENTER;

	if(mac)
	{
		//dump_mac(priv, mac);
		pstat = get_stainfo(priv, mac);
	}

	if(pstat == NULL)
		goto realtek_cfg80211_change_station_end;
		
#if 0
	if ((OPMODE & WIFI_AP_STATE) == 0)
	{
		return -EOPNOTSUPP;
	}
#endif

	memcpy(mlme.im_macaddr, mac, ETH_ALEN);

#if 1
	err = cfg80211_check_station_change(wiphy, params,
						CFG80211_STA_AP_MLME_CLIENT);

	if (err)
	{
		NDEBUG("cfg80211_check_station_change error !! \n");
		goto realtek_cfg80211_change_station_end;
	}
#else
	/* Use this only for authorizing/unauthorizing a station */
	if (!(params->sta_flags_mask & BIT(NL80211_STA_FLAG_AUTHORIZED)))
		return -EOPNOTSUPP;
#endif

	if (params->sta_flags_set & BIT(NL80211_STA_FLAG_AUTHORIZED))
		mlme.im_op = IEEE80211_MLME_AUTHORIZE;
	else
		mlme.im_op = IEEE80211_MLME_UNAUTHORIZE;

	wrqu.data.pointer = &mlme;

#if 0
	printk("NO SET PORT !!\n");
#else

	if(mlme.im_op == IEEE80211_MLME_AUTHORIZE){
		NDEBUG3("IEEE80211_MLME_AUTHORIZE(4-way success!)\n");
	}else{
		NDEBUG3("IEEE80211_MLME_UNAUTHORIZE(clean port)\n");
	}
	
	if(priv->pmib->dot1180211AuthEntry.dot11EnablePSK || priv->pmib->dot118021xAuthEntry.dot118021xAlgrthm)//OPENWRT_RADIUS 
		rtl_net80211_setmlme(priv->dev, NULL, &wrqu, NULL);
#endif

realtek_cfg80211_change_station_end:
	NLEXIT;
	return err;
}

static void realtek_cfg80211_set_rate_info(struct rate_info *r_info, unsigned int rate, unsigned char bw, unsigned char isSgi)
{
	r_info->flags = 0;

#ifdef RTK_AC_SUPPORT
	if (is_VHT_rate(rate)) 
	{
		r_info->flags |= RATE_INFO_FLAGS_VHT_MCS;
		r_info->mcs = (rate - VHT_RATE_ID) % 10;
		r_info->nss = (rate - VHT_RATE_ID) / 10 + 1;		
	}
	else
#endif
	if (is_MCS_rate(rate)) 
	{		
		r_info->flags |= RATE_INFO_FLAGS_MCS;
		r_info->mcs = (rate - HT_RATE_ID);
	}
	else 
		r_info->legacy = (rate&0x7f) / 2;

	if(isSgi)
		r_info->flags |= RATE_INFO_FLAGS_SHORT_GI;

	switch(bw) {
#ifdef OPENWRT_CC
		case HT_CHANNEL_WIDTH_160:
			r_info->bw = RATE_INFO_BW_160;
			break;
		case HT_CHANNEL_WIDTH_80:
			r_info->bw = RATE_INFO_BW_80;
			break;
		case HT_CHANNEL_WIDTH_20_40:
			r_info->bw = RATE_INFO_BW_40;
			break;
		case HT_CHANNEL_WIDTH_20:
			r_info->bw = RATE_INFO_BW_20;
			break;
		case HT_CHANNEL_WIDTH_10:
			r_info->bw = RATE_INFO_BW_10;
			break;
		case HT_CHANNEL_WIDTH_5:
			r_info->bw = RATE_INFO_BW_5;
			break;
		default:
			NDEBUG2("Unknown bw(=%d)\n", bw);
			r_info->bw = RATE_INFO_BW_20;
			break;
#else
		case HT_CHANNEL_WIDTH_160:
			r_info->flags |= RATE_INFO_FLAGS_160_MHZ_WIDTH;
			break;
		case HT_CHANNEL_WIDTH_80:
			r_info->flags |= RATE_INFO_FLAGS_80_MHZ_WIDTH;
			break;
		case HT_CHANNEL_WIDTH_20_40:
			r_info->flags |= RATE_INFO_FLAGS_40_MHZ_WIDTH;
			break;
#endif
	}

}

static int realtek_cfg80211_get_station(struct wiphy *wiphy, struct net_device *dev,
				 u8 *mac, struct station_info *sinfo)
{
	struct rtknl *rtk = wiphy_priv(wiphy);
	struct rtl8192cd_priv *priv = realtek_get_priv(wiphy, dev);
	struct stat_info *pstat = NULL;

	unsigned int tx_rate, rx_rate;
	unsigned char tx_bw, rx_bw, tx_sgi, rx_sgi;

	if(mac)
		pstat = get_stainfo(priv, mac);

	if(pstat==NULL)
		return -ENOENT;
	
	tx_rate = pstat->current_tx_rate;
	rx_rate = pstat->rx_rate;
	tx_bw = pstat->tx_bw;
	rx_bw = pstat->rx_bw;
	tx_sgi = (pstat->ht_current_tx_info&BIT(1))?TRUE:FALSE;
	rx_sgi = (pstat->rx_splcp)?TRUE:FALSE;


	sinfo->filled = 0;

#ifdef OPENWRT_CC
	sinfo->filled = BIT(NL80211_STA_INFO_INACTIVE_TIME) |
					BIT(NL80211_STA_INFO_CONNECTED_TIME)|
					BIT(NL80211_STA_INFO_RX_BYTES64)	|
					BIT(NL80211_STA_INFO_RX_PACKETS)	|
					BIT(NL80211_STA_INFO_TX_BYTES64)	|
					BIT(NL80211_STA_INFO_TX_PACKETS)	|
					BIT(NL80211_STA_INFO_SIGNAL)		|
					BIT(NL80211_STA_INFO_TX_BITRATE)	|
					BIT(NL80211_STA_INFO_RX_BITRATE)	|
					0;
#else
	sinfo->filled = STATION_INFO_INACTIVE_TIME	|
					STATION_INFO_CONNECTED_TIME	|
					STATION_INFO_RX_BYTES64		|
					STATION_INFO_RX_PACKETS		|
					STATION_INFO_TX_BYTES64		|
					STATION_INFO_TX_PACKETS		|
					STATION_INFO_SIGNAL			|
					STATION_INFO_TX_BITRATE		|
					STATION_INFO_RX_BITRATE		|
					0;
#endif

	sinfo->inactive_time = (priv->expire_to - pstat->expire_to)*1000;
	sinfo->connected_time = pstat->link_time;
	sinfo->rx_bytes = pstat->rx_bytes;
	sinfo->rx_packets = pstat->rx_pkts;
	sinfo->tx_bytes = pstat->tx_bytes;
	sinfo->tx_packets = pstat->tx_pkts;
#if defined(SIGNAL_TYPE_UNSPEC)
	sinfo->signal = pstat->rssi;
#else
	if(pstat->rssi > 100)
		sinfo->signal = -20;
	else
		sinfo->signal = pstat->rssi-100;
#endif

	realtek_cfg80211_set_rate_info(&sinfo->txrate, tx_rate, tx_bw, tx_sgi);
	realtek_cfg80211_set_rate_info(&sinfo->rxrate, rx_rate, rx_bw, rx_sgi);

#if 0 //_eric_nl ?? sinfo->bss_param ??
	if(OPMODE & WIFI_STATION_STATE)
	{
		sinfo->filled |= STATION_INFO_BSS_PARAM;
		sinfo->bss_param.flags = 0;
		sinfo->bss_param.dtim_period = priv->pmib->dot11Bss.dtim_prd;
		sinfo->bss_param.beacon_interval = priv->pmib->dot11StationConfigEntry.dot11BeaconPeriod;
	}
#endif 

	//NLEXIT;
	return 0;
}


static int realtek_cfg80211_dump_station(struct wiphy *wiphy, struct net_device *dev,
				 int idx, u8 *mac, struct station_info *sinfo)
{
	struct rtknl *rtk = wiphy_priv(wiphy);
	struct rtl8192cd_priv *priv = realtek_get_priv(wiphy, dev);
	int num = 0;
	struct list_head *phead, *plist;
	struct stat_info *pstat;
	int ret = -ENOENT;

	//NDEBUG2("\n");
	//printk("try dump sta[%d]\n", idx);

	if(idx >= priv->assoc_num)
		return -ENOENT;
	
	phead = &priv->asoc_list;
	if (!netif_running(priv->dev) || list_empty(phead)) {
		return -ENOENT;
	}

	plist = phead->next;
	
	while (plist != phead) {
		pstat = list_entry(plist, struct stat_info, asoc_list);

		if(num == idx){
			if(mac)
				memcpy(mac, pstat->hwaddr, ETH_ALEN);
			else
				mac = pstat->hwaddr;
			
			ret = realtek_cfg80211_get_station(wiphy, dev, pstat->hwaddr, sinfo);
			break;
		}
		num++;
		plist = plist->next;
	}

	//NLEXIT; 
	return ret;
}

#if 0
//not in ath6k
static int realtek_cfg80211_set_txq_params(struct wiphy *wiphy,
				    struct ieee80211_txq_params *params)
{
	struct rtknl *rtk = wiphy_priv(wiphy);
	struct rtl8192cd_priv *priv = realtek_get_priv(wiphy, NULL);

	NLENTER;
	NLNOT; 
	
	printk("queue = %d\n", params->queue);

	return 0;

}
#endif

static int realtek_cfg80211_set_wiphy_params(struct wiphy *wiphy, u32 changed)
{
	struct rtknl *rtk = wiphy_priv(wiphy);
	struct rtl8192cd_priv *priv = realtek_get_priv(wiphy, NULL);

	NLENTER;

	if (changed & WIPHY_PARAM_FRAG_THRESHOLD)
		priv->pmib->dot11OperationEntry.dot11FragmentationThreshold = wiphy->frag_threshold;
	if (changed & WIPHY_PARAM_RTS_THRESHOLD)
		priv->pmib->dot11OperationEntry.dot11RTSThreshold = wiphy->rts_threshold;
	if (changed & WIPHY_PARAM_RETRY_SHORT)
		priv->pmib->dot11OperationEntry.dot11ShortRetryLimit = wiphy->retry_short;
	if (changed & WIPHY_PARAM_RETRY_LONG)
		priv->pmib->dot11OperationEntry.dot11LongRetryLimit = wiphy->retry_long;

    if(under_apmode_repeater) {
        priv = GET_VXD_PRIV(priv);

        if (changed & WIPHY_PARAM_FRAG_THRESHOLD)
            priv->pmib->dot11OperationEntry.dot11FragmentationThreshold = wiphy->frag_threshold;
        if (changed & WIPHY_PARAM_RTS_THRESHOLD)
            priv->pmib->dot11OperationEntry.dot11RTSThreshold = wiphy->rts_threshold;
        if (changed & WIPHY_PARAM_RETRY_SHORT)
            priv->pmib->dot11OperationEntry.dot11ShortRetryLimit = wiphy->retry_short;
        if (changed & WIPHY_PARAM_RETRY_LONG)
            priv->pmib->dot11OperationEntry.dot11LongRetryLimit = wiphy->retry_long;

        NLMSG("Apply advanced settings to VXD(%s)\n",priv->dev->name);
    }
    
	NLEXIT;
	return 0;
}

static int realtek_cfg80211_set_ap_chanwidth(struct wiphy *wiphy,
				      struct net_device *dev,
				      struct cfg80211_chan_def *chandef)
{
	struct rtknl *rtk = wiphy_priv(wiphy);
	struct rtl8192cd_priv *priv = realtek_get_priv(wiphy, NULL);

	NLENTER;
	return realtek_cfg80211_set_channel(wiphy, dev, chandef);
}

static int realtek_cfg80211_set_monitor_channel(struct wiphy *wiphy,
					 struct cfg80211_chan_def *chandef)
{
	struct rtknl *rtk = wiphy_priv(wiphy);
	struct rtl8192cd_priv *priv = realtek_get_priv(wiphy, NULL);
	
	NLENTER;
	return realtek_cfg80211_set_channel(wiphy, priv->dev, chandef);
}

#define MAX_2G_CHANNEL_NUM_MIB		14
#define MAX_5G_CHANNEL_NUM_MIB		196

#define MAX_2G_POWER_dBm 20 //defined by OpenWrt Webpage
#define MAX_5G_POWER_dBm 20 //defined by OpenWrt Webpage


unsigned int get_max_power(struct rtl8192cd_priv *priv)
{
	int max_power = 0;

#ifdef TXPWR_LMT
	if(!priv->pshare->rf_ft_var.disable_txpwrlmt) {
		if((priv->pshare->txpwr_lmt_HT1S) 
			&& (priv->pshare->txpwr_lmt_HT1S <= priv->pshare->tgpwr_HT1S_new[RF_PATH_A]))
			max_power = priv->pshare->txpwr_lmt_HT1S;
		else
			max_power = priv->pshare->tgpwr_HT1S_new[RF_PATH_A];
	}
	else
		max_power = priv->pshare->tgpwr_HT1S_new[RF_PATH_A]; 
#else
	max_power = priv->pshare->tgpwr_HT1S_new[RF_PATH_A]; 
#endif

	max_power = (max_power/2);

	//panic_printk("[%s][%s][%d] max_power=%d dBm \n", priv->dev->name, __FUNCTION__, __LINE__, max_power);

	return max_power;
}

static int realtek_cfg80211_set_tx_power(struct wiphy *wiphy,
				  struct wireless_dev *wdev,
				  enum nl80211_tx_power_setting type, int mbm) 			  
{
	struct rtknl *rtk = wiphy_priv(wiphy);
	struct rtl8192cd_priv *priv = realtek_get_priv(wiphy, NULL);
	int dbm = MBM_TO_DBM(mbm);
	int max_pwr=0, rfuoput=0, new_rfuoput=0, i;

	NLENTER;
	
	max_pwr = get_max_power(priv);

	if(max_pwr == 0) {
		if(priv->pmib->dot11RFEntry.phyBandSelect == PHY_BAND_5G)
			max_pwr=MAX_5G_POWER_dBm;
		else
			max_pwr=MAX_2G_POWER_dBm;
	}

	//panic_printk("### max_pwr=%d dbm=%d(mbm=%d) \n", max_pwr, dbm, mbm);

	rtk->pwr_set_dbm = dbm;

	if(dbm >= max_pwr)
	{
		rfuoput = 0;
		rtk->pwr_rate = 100;
	}
#if defined(DEC_PWR_BY_PERCENTAGE)
	else if(dbm >= ((max_pwr*70)/100))	
	{
		rfuoput = 1;
		rtk->pwr_rate = 70;
	}
	else if(dbm >= ((max_pwr*50)/100))	
	{
		rfuoput = 2;
		rtk->pwr_rate = 50;
	}
	else if(dbm >= ((max_pwr*35)/100))	
	{
		rfuoput = 3;
		rtk->pwr_rate = 35;
	}
	else
	{
		rfuoput = 4;
		rtk->pwr_rate = 15;
	}
	//panic_printk("### rfuoput idx=%d rtk->pwr_rate=%d(percent) \n", rfuoput, rtk->pwr_rate);

	if(rfuoput == 1)
		rfuoput = -3;
	else if(rfuoput == 2)
		rfuoput = -6;
	else if(rfuoput == 3)
		rfuoput = -9;
	else if(rfuoput == 4)
		rfuoput = -17;
#else
	else{
		rfuoput = (dbm-max_pwr)*2;
	}
#endif
	new_rfuoput = rfuoput;

	//panic_printk("### from cur_pwr=%d to rfuoput=%d \n",rtk->pwr_cur, rfuoput);
	
	rfuoput = rfuoput - rtk->pwr_cur;	
	rtk->pwr_cur = new_rfuoput;

	//panic_printk("### adjust power=%d\n", rfuoput);
	
	if(priv->pmib->dot11RFEntry.phyBandSelect & PHY_BAND_2G) {
		for (i=0; i<MAX_2G_CHANNEL_NUM_MIB; i++) {
			if(priv->pmib->dot11RFEntry.pwrlevelCCK_A[i] != 0){ 
				if ((priv->pmib->dot11RFEntry.pwrlevelCCK_A[i] + rfuoput) >= 1)
					priv->pmib->dot11RFEntry.pwrlevelCCK_A[i] += rfuoput;
				else
					priv->pmib->dot11RFEntry.pwrlevelCCK_A[i] = 1;
			}
			if(priv->pmib->dot11RFEntry.pwrlevelCCK_B[i] != 0){ 
				if ((priv->pmib->dot11RFEntry.pwrlevelCCK_B[i] + rfuoput) >= 1)
					priv->pmib->dot11RFEntry.pwrlevelCCK_B[i] += rfuoput;
				else
					priv->pmib->dot11RFEntry.pwrlevelCCK_B[i] = 1;
			}
			if(priv->pmib->dot11RFEntry.pwrlevelCCK_C[i] != 0){ 
				if ((priv->pmib->dot11RFEntry.pwrlevelCCK_C[i] + rfuoput) >= 1)
					priv->pmib->dot11RFEntry.pwrlevelCCK_C[i] += rfuoput;
				else
					priv->pmib->dot11RFEntry.pwrlevelCCK_C[i] = 1;
			}
			if(priv->pmib->dot11RFEntry.pwrlevelCCK_D[i] != 0){ 
				if ((priv->pmib->dot11RFEntry.pwrlevelCCK_D[i] + rfuoput) >= 1)
					priv->pmib->dot11RFEntry.pwrlevelCCK_D[i] += rfuoput;
				else
					priv->pmib->dot11RFEntry.pwrlevelCCK_D[i] = 1;
			}

			if(priv->pmib->dot11RFEntry.pwrlevelHT40_1S_A[i] != 0){ 
				if ((priv->pmib->dot11RFEntry.pwrlevelHT40_1S_A[i] + rfuoput) >= 1)
					priv->pmib->dot11RFEntry.pwrlevelHT40_1S_A[i] += rfuoput;
				else
					priv->pmib->dot11RFEntry.pwrlevelHT40_1S_A[i] = 1;
			}
			if(priv->pmib->dot11RFEntry.pwrlevelHT40_1S_B[i] != 0){ 
				if ((priv->pmib->dot11RFEntry.pwrlevelHT40_1S_B[i] + rfuoput) >= 1)
					priv->pmib->dot11RFEntry.pwrlevelHT40_1S_B[i] += rfuoput;
				else
					priv->pmib->dot11RFEntry.pwrlevelHT40_1S_B[i] = 1;			
			}
			if(priv->pmib->dot11RFEntry.pwrlevelHT40_1S_C[i] != 0){ 
				if ((priv->pmib->dot11RFEntry.pwrlevelHT40_1S_C[i] + rfuoput) >= 1)
					priv->pmib->dot11RFEntry.pwrlevelHT40_1S_C[i] += rfuoput;
				else
					priv->pmib->dot11RFEntry.pwrlevelHT40_1S_C[i] = 1;			
			}
			if(priv->pmib->dot11RFEntry.pwrlevelHT40_1S_D[i] != 0){ 
				if ((priv->pmib->dot11RFEntry.pwrlevelHT40_1S_D[i] + rfuoput) >= 1)
					priv->pmib->dot11RFEntry.pwrlevelHT40_1S_D[i] += rfuoput;
				else
					priv->pmib->dot11RFEntry.pwrlevelHT40_1S_D[i] = 1;			
			}
		}
	} else {
		for (i=0; i<MAX_5G_CHANNEL_NUM_MIB; i++) {
			if(priv->pmib->dot11RFEntry.pwrlevel5GHT40_1S_A[i] != 0){ 
				if ((priv->pmib->dot11RFEntry.pwrlevel5GHT40_1S_A[i] + rfuoput) >= 1)
					priv->pmib->dot11RFEntry.pwrlevel5GHT40_1S_A[i] += rfuoput;
				else
					priv->pmib->dot11RFEntry.pwrlevel5GHT40_1S_A[i] = 1;					
			}
			if(priv->pmib->dot11RFEntry.pwrlevel5GHT40_1S_B[i] != 0){ 
				if ((priv->pmib->dot11RFEntry.pwrlevel5GHT40_1S_B[i] + rfuoput) >= 1)
					priv->pmib->dot11RFEntry.pwrlevel5GHT40_1S_B[i] += rfuoput;
				else
					priv->pmib->dot11RFEntry.pwrlevel5GHT40_1S_B[i] = 1;
			}
			if(priv->pmib->dot11RFEntry.pwrlevel5GHT40_1S_C[i] != 0){ 
				if ((priv->pmib->dot11RFEntry.pwrlevel5GHT40_1S_C[i] + rfuoput) >= 1)
					priv->pmib->dot11RFEntry.pwrlevel5GHT40_1S_C[i] += rfuoput;
				else
					priv->pmib->dot11RFEntry.pwrlevel5GHT40_1S_C[i] = 1;
			}
			if(priv->pmib->dot11RFEntry.pwrlevel5GHT40_1S_D[i] != 0){ 
				if ((priv->pmib->dot11RFEntry.pwrlevel5GHT40_1S_D[i] + rfuoput) >= 1)
					priv->pmib->dot11RFEntry.pwrlevel5GHT40_1S_D[i] += rfuoput;
				else
					priv->pmib->dot11RFEntry.pwrlevel5GHT40_1S_D[i] = 1;
			}
		}
	}

	//Apply config immediately for AP mode
	if(OPMODE & WIFI_AP_STATE)
	{
		if(priv->pmib->dot11RFEntry.dot11channel)
			SwChnl(priv, priv->pmib->dot11RFEntry.dot11channel, priv->pmib->dot11nConfigEntry.dot11n2ndChOffset);
	}

	return 0;
}

struct rtl8192cd_priv* get_priv_from_wdev(struct rtknl *rtk, struct wireless_dev *wdev)
{
	struct rtl8192cd_priv *priv = NULL;
	int tmp = 0;

	for(tmp = 0; tmp<(IF_NUM); tmp++)
	{
		if(rtk->rtk_iface[tmp].priv)
		if(wdev == &(rtk->rtk_iface[tmp].priv->wdev))
		{
			priv = rtk->rtk_iface[tmp].priv;
			break;
		}
	}

	//printk("wdev = 0x%x priv = 0x%x \n", wdev, priv);

	return priv;
}


static int realtek_cfg80211_get_tx_power(struct wiphy *wiphy, 
				  struct wireless_dev *wdev, int *dbm)
{
	struct rtknl *rtk = wiphy_priv(wiphy);
	struct rtl8192cd_priv *priv = get_priv_from_wdev(rtk, wdev);

	//NLENTER;

	if(rtk->pwr_set_dbm)
		*dbm = rtk->pwr_set_dbm;
	else
	*dbm = 13;

	//NLEXIT;
	return 0;

}


#endif


#if 1

//_eric_nl ?? suspend/resume use open/close ??
static int realtek_cfg80211_suspend(struct wiphy *wiphy, struct cfg80211_wowlan *wow)
{
	struct rtknl *rtk = wiphy_priv(wiphy);
	struct rtl8192cd_priv *priv = realtek_get_priv(wiphy, NULL);

	NLENTER;
	NLNOT;
	
	return 0;
}

static int realtek_cfg80211_resume(struct wiphy *wiphy)
{
	struct rtknl *rtk = wiphy_priv(wiphy);
	struct rtl8192cd_priv *priv = realtek_get_priv(wiphy, NULL);

	NLENTER;
	NLNOT;

	return 0;
}


/*cfg p2p*/
int realtek_cfg80211_fill_available_channel(struct rtl8192cd_priv *priv,
			  struct cfg80211_scan_request *request)
{
	int idx=0;
	unsigned char n_channels = 0;    
    //NDEBUG("\n");    
	if (request->n_channels > 0) {
		//request should scan only one band, apply band here
		rtk_set_band_mode(priv, request->channels[0]->band, request->scan_width);

		if(IS_HAL_CHIP(priv) && request->channels[0]->band == IEEE80211_BAND_5GHZ && !(RTL_R8(0x454) & BIT(7))) {
			//To prevent treated as 2.4GHz at CheckBand88XX_AC(), Hal88XXPhyCfg.c
			//0x454 is configured at rtl8192cd_init_hw_PCI(), rtl8192cd_hw.c
			RTL_W8(0x454, RTL_R8(0x454) | BIT(7));
		}

		if(request->n_channels == 3 &&
			request->channels[0]->hw_value == 1 &&
			request->channels[1]->hw_value == 6 &&
			request->channels[2]->hw_value == 11
		){
			NDEBUG2("social_channel from cfg80211\n");
		}        

		n_channels = request->n_channels;
		priv->available_chnl_num = n_channels;
		//NDEBUG2("n_channels[%d]\n",n_channels);
		if(n_channels==1){
			NDEBUG2("n_channels[%d],ch[%d]\n",n_channels ,ieee80211_frequency_to_channel(request->channels[0]->center_freq));            
		}        
		for (idx = 0; idx < n_channels; idx++){
			priv->available_chnl[idx] = ieee80211_frequency_to_channel(request->channels[idx]->center_freq);            
		}
	}
}
/*cfg p2p*/
static int realtek_cfg80211_scan(struct wiphy *wiphy,
			  struct cfg80211_scan_request *request)
{
	struct rtknl *rtk = wiphy_priv(wiphy);
	struct rtl8192cd_priv *priv = realtek_get_priv(wiphy, NULL);/*eric refine 23390*/ 
	struct cfg80211_ssid *ssids = request->ssids;

	NLENTER;
	NLMSG("request->flags = 0x%x \n", request->flags);

	priv = get_priv_from_wdev(rtk, request->wdev);

    if(is_WRT_scan_iface(priv->dev->name))
		priv = GET_ROOT(priv);

	if (!netif_running(priv->dev)){
		NLMSG("scan disable (!netif_running %s)\n", priv->dev->name);
		return -1;
	}
	
	if (priv->ss_req_ongoing ){
        NLMSG("scan disable (ss_req_ongoing %s)\n", priv->dev->name);
        return -1;
    }

	if (priv->scan_req){
        NLMSG("scan disable (scan_req %s)\n", priv->dev->name);
        return -1;
    }

#if defined(DFS)
	if (timer_pending(&priv->ch_avail_chk_timer)){
		NLMSG("%s ch_avail_chk_timer pending\n", priv->dev->name);
		return -1;
	}
#endif
    #if 0
    {
      	unsigned char idx;
		if(ssids->ssid != NULL ){
            NDEBUG("p2pssid=[%s]\n",ssids->ssid);
        }    

    	if (request->ie) {
    		printk("request->ie = ");
    		for(idx=0; idx<request->ie_len; idx++)
    		{
    			printk(" %02x", request->ie);
    		}
    		printk("\n");
    	}

        printk("\n");

    	if (request->n_channels > 0) {
    		unsigned char n_channels = 0;
    		n_channels = request->n_channels;
    		for (idx = 0; idx < n_channels; idx++){
    			NDEBUG("channel[%d]=%d\n", idx,	ieee80211_frequency_to_channel(request->channels[idx]->center_freq));
            }
    	}
    }
    #endif

    //rtk_abort_scan(priv);   

	priv->ss_ssidlen = 0;

#if defined(P2P_SUPPORT)
    if(ssids->ssid != NULL
        && !memcmp(ssids->ssid, "DIRECT-", 7)
        && rtk_get_p2p_ie((u8 *)request->ie, request->ie_len, NULL, NULL))
    {
        NDEBUG("Ssid=[%s],len[%d]...\n",ssids->ssid,ssids->ssid_len);

        priv->ss_ssidlen = ssids->ssid_len;
        memcpy(priv->ss_ssid,ssids->ssid,ssids->ssid_len);

		if(!rtk_p2p_is_enabled(priv)) {
            NDEBUG3("==>rtk_p2p_enable(CFG80211_P2P)\n");
            rtk_p2p_enable(priv , P2P_DEVICE , CFG80211_P2P);            
        }

        priv->p2pPtr->pre_p2p_role=rtk_p2p_get_role(priv);
        priv->p2pPtr->pre_p2p_state=rtk_p2p_get_state(priv);

        rtk_p2p_set_role(priv,P2P_DEVICE);
        rtk_p2p_set_state(priv, P2P_S_SEARCH);        
        //GET_ROOT(priv)->site_survey_times = SS_COUNT-2;   // pre-channel just scan twice
    } else
#endif
	if(ssids->ssid != NULL) {
        NDEBUG3("Ssid=[%s],len[%d]...\n",ssids->ssid,ssids->ssid_len);
        priv->ss_ssidlen = ssids->ssid_len;
        memcpy(priv->ss_ssid,ssids->ssid,ssids->ssid_len);
    }
#if defined(P2P_SUPPORT)
    /*set WPS P2P IE to probe_req*/
	if(request->ie && request->ie_len>0)
	{
		rtk_cfg80211_set_wps_p2p_ie(priv, (u8 *)request->ie, request->ie_len ,MGMT_PROBEREQ );
	}
#endif
#ifdef CUSTOMIZE_SCAN_HIDDEN_AP
	//scan for HiddenAP
	if(request->n_ssids && request->ssids[0].ssid_len)
	{
		priv->ss_ssidlen = request->ssids[0].ssid_len;
		memcpy(priv->ss_ssid, request->ssids[0].ssid, request->ssids[0].ssid_len);
	}
#endif

    #if 0//def WIFI_SIMPLE_CONFIG
	if (len == 2)
		priv->ss_req_ongoing = 2;	// WiFi-Simple-Config scan-req
	else
    #endif

	if(IS_VXD_INTERFACE(priv))
		priv->ss_req_ongoing = SSFROM_REPEATER_VXD;
	else
		priv->ss_req_ongoing = SSFROM_WEB;

	priv->scan_req = request;
	if(request->n_channels > 0){
		/*use channels from cfg80211 parameter*/
		realtek_cfg80211_fill_available_channel(priv,request);
	}else{
		/*use rtk default available channels*/
		get_available_channel(priv);
	}

	start_clnt_ss(priv);

	NLEXIT;
	return 0;
}


static int realtek_start_ap(struct wiphy *wiphy, struct net_device *dev,
			   struct cfg80211_ap_settings *info)
{
	struct rtknl *rtk = wiphy_priv(wiphy);
	struct rtl8192cd_priv *priv = realtek_get_priv(wiphy, dev);
	struct cfg80211_scan_request *scan_req_pending = NULL;
	int ret = 0;

	NLENTER;

	if (!realtek_cfg80211_ready(priv))
		return -EIO;

	if ((OPMODE & (WIFI_AP_STATE | WIFI_ADHOC_STATE)) == 0) //wrt-adhoc
		return -EOPNOTSUPP;

	if (info->ssid == NULL)
		return -EINVAL;

	scan_req_pending = priv->scan_req;

	if(IS_ROOT_INTERFACE(priv))
		rtk_abort_scan(priv, SCAN_ABORT_START_AP);

	/*fixme, should not enable carrier here.
	 Under mac80211 architecture will be invoked by compatible-wireless */
	netif_carrier_on(priv->dev);

	realtek_cfg80211_set_channel(wiphy, dev, &info->chandef);

	realtek_set_ies_apmode(priv, &info->beacon);

	ret = realtek_set_bss(priv, info);

	realtek_reset_security(priv);

	ret = realtek_set_auth_type(priv, info->auth_type);
	
	realtek_set_security(priv, rtk, info->crypto);

	realtek_ap_default_config(priv);

	realtek_ap_config_apply(priv);

	priv->pmib->miscEntry.func_off = 0;

#if defined(DFS)
	if((OPMODE&WIFI_AP_STATE) && info->chandef.chan->dfs_state == NL80211_DFS_AVAILABLE) {
		printk("*** [%s]Under DFS channel, radar detection is active ***\n",priv->dev->name);
		/* DFS activated after 1 sec; prevent switching channel due to DFS false alarm */
		if (timer_pending(&priv->DFS_timer))
			del_timer(&priv->DFS_timer);
		else {
			init_timer(&priv->DFS_timer);
			priv->DFS_timer.data = (unsigned long) priv;
			priv->DFS_timer.function = rtl8192cd_DFS_timer;
		}
		mod_timer(&priv->DFS_timer, jiffies + RTL_SECONDS_TO_JIFFIES(1));

        if (timer_pending(&priv->dfs_det_chk_timer))
			del_timer(&priv->dfs_det_chk_timer);
		else {
			init_timer(&priv->dfs_det_chk_timer);
			priv->dfs_det_chk_timer.data = (unsigned long) priv;
			priv->dfs_det_chk_timer.function = rtl8192cd_dfs_det_chk_timer;
		}
		mod_timer(&priv->dfs_det_chk_timer, jiffies + RTL_MILISECONDS_TO_JIFFIES(priv->pshare->rf_ft_var.dfs_det_period*10));

		DFS_SetReg(priv);
		RTL_W8(TXPAUSE, 0x00);
	}
#endif
	if(scan_req_pending)
		realtek_cfg80211_scan(wiphy, scan_req_pending);

    if(ret){
        NDEBUG("fail[%d]\n",ret);
	}
    
	NLEXIT;    
	
	return ret;

}


static int realtek_cfg80211_join_ibss(struct wiphy *wiphy, struct net_device *dev,
			       struct cfg80211_ibss_params *ibss_param)
{
	struct rtknl *rtk = wiphy_priv(wiphy);
	struct rtl8192cd_priv *priv = realtek_get_priv(wiphy, dev);

	NLENTER;

	if (!realtek_cfg80211_ready(priv))
		return -EIO;

	if (OPMODE & WIFI_ADHOC_STATE == 0)
		return -EOPNOTSUPP;

	//printk("Ad-Hoc join [%s] \n", ibss_param->ssid);
		
	memcpy(SSID, ibss_param->ssid, ibss_param->ssid_len);	
	SSID_LEN = ibss_param->ssid_len;

	realtek_reset_security(priv);

	if (ibss_param->privacy) 
	{
		realtek_auth_wep(priv, _WEP_40_PRIVACY_);
	}

#if 0
	if (ibss_param->chandef.chan)
	{
		realtek_cfg80211_set_channel(wiphy, dev, ibss_param->chandef.chan, ibss_param->channel_type);
	}
#endif

#if 0	

	if (ibss_param->channel_fixed) {
		/*
		 * TODO: channel_fixed: The channel should be fixed, do not
		 * search for IBSSs to join on other channels. Target
		 * firmware does not support this feature, needs to be
		 * updated.
		 */
		return -EOPNOTSUPP;
	}

	memset(vif->req_bssid, 0, sizeof(vif->req_bssid));
	if (ibss_param->bssid && !is_broadcast_ether_addr(ibss_param->bssid))
		memcpy(vif->req_bssid, ibss_param->bssid,
			sizeof(vif->req_bssid));
#endif

	if(IS_VXD_INTERFACE(priv))
	{
		//printk("launch vxd_ibss_beacon timer !!\n");
		construct_ibss_beacon(priv);
		issue_beacon_ibss_vxd(priv);
	}

	priv->join_res = STATE_Sta_No_Bss;
	start_clnt_lookup(priv, 1);

	NLEXIT;

	return 0;
}

static int realtek_cfg80211_leave_ibss(struct wiphy *wiphy, struct net_device *dev)
{
	struct rtknl *rtk = wiphy_priv(wiphy);
	struct rtl8192cd_priv *priv = realtek_get_priv(wiphy, dev);

	NLENTER;
	return 0;
}


static int realtek_cfg80211_set_wds_peer(struct wiphy *wiphy, struct net_device *dev,
				  u8 *addr)
{
	struct rtknl *rtk = wiphy_priv(wiphy);
	struct rtl8192cd_priv *priv = realtek_get_priv(wiphy, dev);

	NLENTER;
	return 0;
}

static void realtek_cfg80211_rfkill_poll(struct wiphy *wiphy)
{
	struct rtknl *rtk = wiphy_priv(wiphy);
	struct rtl8192cd_priv *priv = realtek_get_priv(wiphy, NULL);

	NLENTER;
	return 0;
}


static int realtek_cfg80211_set_power_mgmt(struct wiphy *wiphy, struct net_device *dev,
				    BOOLEAN enabled, int timeout)
{
	struct rtknl *rtk = wiphy_priv(wiphy);
	struct rtl8192cd_priv *priv = realtek_get_priv(wiphy, dev);

	NLENTER;
	return 0;
}

//not in ath6k
static int realtek_cfg80211_set_bitrate_mask(struct wiphy *wiphy,
				      struct net_device *dev,
				      const u8 *addr,
				      const struct cfg80211_bitrate_mask *mask)
{
	struct rtknl *rtk = wiphy_priv(wiphy);
	struct rtl8192cd_priv *priv = realtek_get_priv(wiphy, dev);

	NLENTER;
	NLNOT;

	//printk("fixed=%d, maxrate=%d\n", mask->fixed, mask->maxrate);  //mark_com

	return 0;
}
#endif

static int apply_acl_rules(struct rtl8192cd_priv *priv)
{
	unsigned int i=0;

	for (i=0; i<priv->pmib->dot11StationConfigEntry.dot11AclNum; i++)
	{
		struct list_head *pnewlist;
		struct wlan_acl_node *paclnode;

		pnewlist = priv->wlan_aclpolllist.next;
		list_del_init(pnewlist);

		paclnode = list_entry(pnewlist,	struct wlan_acl_node, list);
		memcpy((void *)paclnode->addr, priv->pmib->dot11StationConfigEntry.dot11AclAddr[i], MACADDRLEN);
		paclnode->mode = (unsigned char)priv->pmib->dot11StationConfigEntry.dot11AclMode;
		NDEBUG("[Drv]Sync MAC ACL entry[%d]: %02x:%02x:%02x:%02x:%02x:%02x,%s from MIB\n",i,
			paclnode->addr[0],paclnode->addr[1],paclnode->addr[2],
			paclnode->addr[3],paclnode->addr[4],paclnode->addr[5],
			(paclnode->mode&1U)? "Allowed":"Denied");
		list_add_tail(pnewlist, &priv->wlan_acl_list);
	}

	return 0;
}

static int realtek_reset_mac_acl(struct rtl8192cd_priv *priv)
{
	int i=0;

	for(i;i<priv->pmib->dot11StationConfigEntry.dot11AclNum;i++) {
		NDEBUG("Reset MAC ACL entry[%d]: %02x:%02x:%02x:%02x:%02x:%02x\n",i,
			priv->pmib->dot11StationConfigEntry.dot11AclAddr[i][0],priv->pmib->dot11StationConfigEntry.dot11AclAddr[i][1],
			priv->pmib->dot11StationConfigEntry.dot11AclAddr[i][2],priv->pmib->dot11StationConfigEntry.dot11AclAddr[i][3],
			priv->pmib->dot11StationConfigEntry.dot11AclAddr[i][4],priv->pmib->dot11StationConfigEntry.dot11AclAddr[i][5]);
		memset(priv->pmib->dot11StationConfigEntry.dot11AclAddr[i],0,MACADDRLEN);
	}
	priv->pmib->dot11StationConfigEntry.dot11AclNum = 0;
	priv->pmib->dot11StationConfigEntry.dot11AclMode = 0;

	return 0;
}

static int realtek_set_mac_acl(struct wiphy *wiphy, struct net_device *dev,
			       const struct cfg80211_acl_data *params)
{
	struct rtl8192cd_priv *priv = realtek_get_priv(wiphy, dev);
	int i=0;

	priv->pmib->dot11StationConfigEntry.dot11AclNum = 0;
    /*
		params->acl_policy:
		enum nl80211_acl_policy {
			NL80211_ACL_POLICY_ACCEPT_UNLESS_LISTED,
			NL80211_ACL_POLICY_DENY_UNLESS_LISTED,
		};		
     */
	NDEBUG("MAC ACL mode:%s configured\n",(params->acl_policy&NL80211_ACL_POLICY_DENY_UNLESS_LISTED)? "Allow":"Deny");
	priv->pmib->dot11StationConfigEntry.dot11AclMode = (params->acl_policy&NL80211_ACL_POLICY_DENY_UNLESS_LISTED)? 1:2;

	for(i;i<params->n_acl_entries;i++) {
		priv->pmib->dot11StationConfigEntry.dot11AclNum++;
		memcpy(priv->pmib->dot11StationConfigEntry.dot11AclAddr[i],params->mac_addrs[i].addr,MACADDRLEN);
		NDEBUG("Append MAC ACL entry[%d]: %02x:%02x:%02x:%02x:%02x:%02x\n",i,
			priv->pmib->dot11StationConfigEntry.dot11AclAddr[i][0],priv->pmib->dot11StationConfigEntry.dot11AclAddr[i][1],
			priv->pmib->dot11StationConfigEntry.dot11AclAddr[i][2],priv->pmib->dot11StationConfigEntry.dot11AclAddr[i][3],
			priv->pmib->dot11StationConfigEntry.dot11AclAddr[i][4],priv->pmib->dot11StationConfigEntry.dot11AclAddr[i][5]);
	}
	NDEBUG("MAC ACL total entry number:%d\n",priv->pmib->dot11StationConfigEntry.dot11AclNum);

	apply_acl_rules(priv);
	return 0;
}

#if defined(DFS)
static int realtek_start_radar_detection (struct wiphy *wiphy,
					 struct net_device *dev,
					 struct cfg80211_chan_def *chandef,
					 u32 cac_time_ms)
{
	int ret=0;
	struct rtknl *rtk = wiphy_priv(wiphy);
	struct rtl8192cd_priv *priv = realtek_get_priv(wiphy, NULL);
	int channel = 0;
	enum nl80211_dfs_regions dfs_region;

	NLENTER;

	/*fixme, should not enable carrier here.
	 Under mac80211 architecture will be invoked by compatible-wireless */
	netif_carrier_on(priv->dev);

	if(priv->pshare->dfs_chan_def) {
		if(priv->pshare->dfs_chan_def->chan)
			kfree(priv->pshare->dfs_chan_def->chan);

		kfree(priv->pshare->dfs_chan_def);
	}

	//backup chandef for DFS report
	priv->pshare->dfs_chan_def = (struct cfg80211_chan_def *)kmalloc(sizeof(*chandef), GFP_KERNEL);
	if(priv->pshare->dfs_chan_def)
		memset(priv->pshare->dfs_chan_def, 0, sizeof(*chandef));
	else
		return -1;
	memcpy(priv->pshare->dfs_chan_def,chandef,sizeof(*chandef));

	priv->pshare->dfs_chan_def->chan = (struct ieee80211_channel *)kmalloc(sizeof(struct ieee80211_channel), GFP_KERNEL);
	if(priv->pshare->dfs_chan_def->chan)
		memset(priv->pshare->dfs_chan_def->chan, 0, sizeof(struct ieee80211_channel));
	else
		return -1;
	memcpy(priv->pshare->dfs_chan_def->chan,chandef->chan,sizeof(struct ieee80211_channel));

	channel = ieee80211_frequency_to_channel(chandef->chan->center_freq);

	NDEBUG3("center_freq=[%u] channel=[%d] hw_value=[%u] bandwidth=[%d]\n",
		chandef->chan->center_freq, channel, chandef->chan->hw_value, chandef->width);

	EXTERN enum nl80211_dfs_regions reg_get_dfs_region(struct wiphy *wiphy);
	dfs_region = reg_get_dfs_region(wiphy);
	switch(dfs_region) {
		case NL80211_DFS_ETSI:
			priv->pmib->dot11StationConfigEntry.dot11RegDomain = DOMAIN_ETSI;
			break;
		case NL80211_DFS_FCC:
			priv->pmib->dot11StationConfigEntry.dot11RegDomain = DOMAIN_FCC;
			break;
		case NL80211_DFS_JP:
			priv->pmib->dot11StationConfigEntry.dot11RegDomain = DOMAIN_MKK;
			break;
		default:
			printk("%s %d Unset region, keep \"World Wide\"\n",__func__,__LINE__);
			break;
	}

	priv->pmib->dot11RFEntry.dot11channel = channel;

	rtk_set_band_mode(priv,chandef->chan->band , chandef->width);
	rtk_set_channel_mode(priv,chandef);
	SwBWMode(priv, priv->pmib->dot11nConfigEntry.dot11nUse40M, priv->pmib->dot11nConfigEntry.dot11n2ndChOffset);
	SwChnl(priv, channel, priv->pmib->dot11nConfigEntry.dot11n2ndChOffset);

	if (timer_pending(&priv->ch_avail_chk_timer))
		del_timer(&priv->ch_avail_chk_timer);
	else {
		init_timer(&priv->ch_avail_chk_timer);
		priv->ch_avail_chk_timer.data = (unsigned long) priv;
		priv->ch_avail_chk_timer.function = rtl8192cd_ch_avail_chk_timer;
	}
	mod_timer(&priv->ch_avail_chk_timer, jiffies + RTL_MILISECONDS_TO_JIFFIES(cac_time_ms));
	printk("*** [%s]Activate DFS-CAC for %d miliseconds ***\n",priv->dev->name,cac_time_ms);

	if(!timer_pending(&priv->DFS_TXPAUSE_timer)) {
		init_timer(&priv->DFS_TXPAUSE_timer);
		priv->DFS_TXPAUSE_timer.data = (unsigned long) priv;
		priv->DFS_TXPAUSE_timer.function = rtl8192cd_DFS_TXPAUSE_timer;
	}

	/* DFS activated after 5 sec; prevent switching channel due to DFS false alarm */
	if (timer_pending(&priv->DFS_timer))
		del_timer(&priv->DFS_timer);
	else {
		init_timer(&priv->DFS_timer);
		priv->DFS_timer.data = (unsigned long) priv;
		priv->DFS_timer.function = rtl8192cd_DFS_timer;
	}
	mod_timer(&priv->DFS_timer, jiffies + RTL_SECONDS_TO_JIFFIES(5));

	if (timer_pending(&priv->dfs_det_chk_timer))
		del_timer(&priv->dfs_det_chk_timer);
	else {
		init_timer(&priv->dfs_det_chk_timer);
		priv->dfs_det_chk_timer.data = (unsigned long) priv;
		priv->dfs_det_chk_timer.function = rtl8192cd_dfs_det_chk_timer;
	}
	mod_timer(&priv->dfs_det_chk_timer, jiffies + RTL_MILISECONDS_TO_JIFFIES(priv->pshare->rf_ft_var.dfs_det_period*10));

	DFS_SetReg(priv);

	if (!priv->pmib->dot11DFSEntry.CAC_enable) {
		del_timer_sync(&priv->ch_avail_chk_timer);
		mod_timer(&priv->ch_avail_chk_timer, jiffies + RTL_MILISECONDS_TO_JIFFIES(200));
	}

	priv->pmib->dot11DFSEntry.disable_tx = 1;

	return ret;
}
#endif

void copy_bss_ie(struct rtl8192cd_priv *priv, int ix)
{
	int wpa_ie_len = priv->site_survey->bss[ix].wpa_ie_len;
	int rsn_ie_len = priv->site_survey->bss[ix].rsn_ie_len;
	
	priv->rtk->clnt_info.wpa_ie.wpa_ie_len = wpa_ie_len;
	memcpy(priv->rtk->clnt_info.wpa_ie.data, priv->site_survey->bss[ix].wpa_ie, wpa_ie_len);

	priv->rtk->clnt_info.rsn_ie.rsn_ie_len = rsn_ie_len;
	memcpy(priv->rtk->clnt_info.rsn_ie.data, priv->site_survey->bss[ix].rsn_ie, rsn_ie_len);
}

int get_bss_by_bssid(struct rtl8192cd_priv *priv, unsigned char* bssid, unsigned int bssdb_count, struct bss_desc *bssdb)
{
	int ix = 0, found = 0;

	STADEBUG("count = %d %02x:%02x:%02x:%02x:%02x:%02x\n", bssdb_count, bssid[0],bssid[1],bssid[2],bssid[3],bssid[4],bssid[5]);
	//dump_mac(priv, bssid);

	for(ix = 0 ; ix < bssdb_count ; ix++) //_Eric ?? will bss_backup be cleaned?? -> Not found in  codes
	{
		STADEBUG("[%d]Match %02x:%02x:%02x:%02x:%02x:%02x with %02x:%02x:%02x:%02x:%02x:%02x\n",
			ix,bssdb[ix].bssid[0],bssdb[ix].bssid[1],
			bssdb[ix].bssid[2],bssdb[ix].bssid[3],
			bssdb[ix].bssid[4],bssdb[ix].bssid[5],
			bssid[0],bssid[1],bssid[2],bssid[3],bssid[4],bssid[5]);
		if(!memcmp(bssdb[ix].bssid , bssid, 6))
		{
			found = 1;
			copy_bss_ie(priv, ix);
			break;
		}
	}

	if(found == 0)
	{	
		STADEBUG("%s BSSID NOT Found !!\n",__func__);
		return -EINVAL;
	}
	else
		return ix;
	
}


int get_bss_by_ssid(struct rtl8192cd_priv *priv, unsigned char* ssid, int ssid_len, unsigned int bssdb_count, struct bss_desc *bssdb)
{
	int ix = 0, found = 0;

	STADEBUG("count[%d] ssid[%s]\n", bssdb_count, ssid);

	for(ix = 0 ; ix < bssdb_count ; ix++) //_Eric ?? will bss_backup be cleaned?? -> Not found in  codes
	{
		STADEBUG("[%d]Match %s to %s\n",ix,bssdb[ix].ssid, ssid);
		if(!memcmp(bssdb[ix].ssid , ssid, ssid_len))
		{
			found = 1;
			copy_bss_ie(priv, ix);
			break;
		}
	}

	if(found == 0)
	{	
		STADEBUG("SSID NOT Found !!\n");
		return -EINVAL;
	}
	else
		return ix;
	
}

static int realtek_cfg80211_connect(struct wiphy *wiphy, struct net_device *dev,
					  struct cfg80211_connect_params *sme)
{

	struct rtknl *rtk = wiphy_priv(wiphy);
	struct rtl8192cd_priv *priv = realtek_get_priv(wiphy, dev);
	unsigned int bssdb_count=0;
	struct bss_desc *bssdb=NULL;
	int status = 0;
	int bss_num = -1;
	int ret = 0;

	NLENTER;

	if(dev){
    	//dump_mac(priv, GET_MY_HWADDR);
    }
#ifdef  UNIVERSAL_REPEATER //wrt-vxd
	if((!IS_ROOT_INTERFACE(priv)) && (!IS_VXD_INTERFACE(priv)))
#else
	if(!IS_ROOT_INTERFACE(priv))
#endif
	{
		NDEBUG("vap can not connect, switch to root\n");
		priv = GET_ROOT(priv);
	}
	
	if (!realtek_cfg80211_ready(priv)){
        NDEBUG3("return!\n");
		return -EIO;
    }
	
#if 1 //wrt_clnt
	if((OPMODE & WIFI_STATION_STATE) == 0)
	{
		printk("NOT in Client Mode, can NOT Associate !!!\n");
		return -1;
	}
#endif

    //rtk_abort_scan(priv);
	priv->receive_connect_cmd = 1;

#if 1 //wrt-wps-clnt
	priv->pmib->wscEntry.assoc_ielen = 0;
	priv->pmib->wscEntry.wsc_enable = 0;
	
	if (sme->ie && (sme->ie_len > 0)) {
		NDEBUG("ie from cfg,len=[%d]\n", sme->ie_len);
        /*set WPS P2P IE to Assoc_Req*/
		rtk_cfg80211_set_wps_p2p_ie(priv, sme->ie, sme->ie_len, MGMT_ASSOCREQ);
	}

	if(priv->pmib->wscEntry.wsc_enable)
		priv->wps_issue_join_req = 1;
#endif	

//=== check parameters
	if((sme->bssid == NULL) && (sme->ssid == NULL))
	{
		NDEBUG("No bssid&ssid from request !!!\n");
		return -1;
	}

	if(OPMODE & WIFI_STATION_STATE) {
		bssdb_count = priv->site_survey->count_target;
		bssdb = priv->site_survey->bss_target;
	} else {
		bssdb_count = priv->site_survey->count_backup;
		bssdb = priv->site_survey->bss_backup;
	}

	if(sme->bssid){
		bss_num = get_bss_by_bssid(priv, sme->bssid, bssdb_count, bssdb);
	}else if(sme->ssid){ //?? channel parameter check ??
		bss_num = get_bss_by_ssid(priv, sme->ssid, sme->ssid_len, bssdb_count, bssdb);
    }else{
		NDEBUG("Unknown rule to search BSS!!\n");
		return -1;
	}

	if(bss_num < 0)
	{
		NDEBUG("Can not found this bss from SiteSurvey result!!\n");
		return -1;
	}

	priv->ss_req_ongoing = 0; //found bss, no need to scan ...

//=== set security 
	realtek_reset_security(priv); 

	realtek_set_security(priv, rtk, sme->crypto);

	if(priv->pmib->dot1180211AuthEntry.dot11EnablePSK)
		psk_init(priv);
	
//=== set key (for wep only)
	if((sme->key_len) && 
		((rtk->cipher == WLAN_CIPHER_SUITE_WEP40)||(rtk->cipher == WLAN_CIPHER_SUITE_WEP104)))
	{
		printk("Set wep key to connect ! \n");
		
		if(rtk->cipher == WLAN_CIPHER_SUITE_WEP40)
		{
			priv->pmib->dot11GroupKeysTable.dot11Privacy = DOT11_ENC_WEP40;

		}
		else if(rtk->cipher == WLAN_CIPHER_SUITE_WEP104)
		{
			priv->pmib->dot11GroupKeysTable.dot11Privacy = DOT11_ENC_WEP104;

		}
		
		memcpy(&priv->pmib->dot11DefaultKeysTable.keytype[sme->key_idx].skey[0], sme->key, sme->key_len);

		priv->pmib->dot11GroupKeysTable.dot11EncryptKey.dot11TTKeyLen = sme->key_len;
		memcpy(&priv->pmib->dot11GroupKeysTable.dot11EncryptKey.dot11TTKey.skey[0], sme->key, sme->key_len);
		
		if(sme->auth_type==NL80211_AUTHTYPE_SHARED_KEY)
			priv->pmib->dot1180211AuthEntry.dot11AuthAlgrthm = 1;		
	}

	syncMulticastCipher(priv, &bssdb[bss_num]); /*eric refine 23277*/

    if(OPMODE&(WIFI_AUTH_SUCCESS|WIFI_ASOC_STATE)==(WIFI_AUTH_SUCCESS|WIFI_ASOC_STATE)){
        NDEBUG3("try issue deauth to...\n");
        if(memcmp(priv->pmib->dot11StationConfigEntry.dot11Bssid , bssdb[bss_num].bssid , 6)==0){
            NDEBUG3("issue deauth to...\n");            
            //dump_mac(priv,priv->site_survey->bss_target[bss_num].bssid);            
            issue_deauth(priv,priv->site_survey->bss_target[bss_num].bssid,_RSON_DEAUTH_STA_LEAVING_);
            OPMODE &= (~(WIFI_AUTH_SUCCESS|WIFI_ASOC_STATE))    ;
        }
    }

    //=== connect
	ret = rtl_wpas_join(priv, bss_num);

	NLEXIT;
	return ret;
}


static int realtek_cfg80211_disconnect(struct wiphy *wiphy,
						  struct net_device *dev, u16 reason_code)
{
	struct rtknl *rtk = wiphy_priv(wiphy);
	struct rtl8192cd_priv *priv = realtek_get_priv(wiphy, dev);

	NLENTER;
	//rtk_abort_scan(priv);

	return 0;

}

static int realtek_cfg80211_channel_switch(struct wiphy *wiphy, 
			struct net_device *dev, struct cfg80211_csa_settings *params)

{
	struct rtknl *rtk = wiphy_priv(wiphy);
	struct rtl8192cd_priv *priv = realtek_get_priv(wiphy, dev);

	NLENTER;
	return 0;

}

static void realtek_mgmt_frame_register(struct wiphy *wiphy,
				       struct  wireless_dev *wdev,
				       u16 frame_type, BOOLEAN reg)
{
	struct rtknl *rtk = wiphy_priv(wiphy);
	struct rtl8192cd_priv *priv = get_priv_from_wdev(rtk, wdev);
	NLENTER;
	if (frame_type != (IEEE80211_FTYPE_MGMT | IEEE80211_STYPE_PROBE_REQ)){
        NDEBUG3("frame_type=[%02X]\n",frame_type);        
		goto realtek_mgmt_frame_register_end;
    }

realtek_mgmt_frame_register_end:
	NLEXIT;
}




static struct device_type wiphy_type = {
	.name	= "wlan",
};


int register_netdevice_name_rtk(struct net_device *dev)
{
	int err;

	if (strchr(dev->name, '%')) {
		err = dev_alloc_name(dev, dev->name);
		if (err < 0)
			return err;
	}
	
	return register_netdevice(dev);
}

#if 1 //wrt-vap

static int realtek_nliftype_to_drv_iftype(enum nl80211_iftype type, u8 *nw_type)
{
	switch (type) {
	case NL80211_IFTYPE_STATION:
	//case NL80211_IFTYPE_P2P_CLIENT:
		*nw_type = INFRA_NETWORK;
		break;
	case NL80211_IFTYPE_ADHOC:
		*nw_type = ADHOC_NETWORK;
		break;
	case NL80211_IFTYPE_AP:
	//case NL80211_IFTYPE_P2P_GO:
		*nw_type = AP_NETWORK;
		break;
	default:
		printk("invalid interface type %u\n", type);
		return -ENOTSUPP;
	}

	return 0;
}

static BOOLEAN realtek_is_valid_iftype(struct rtknl *rtk, enum nl80211_iftype type,
				   u8 *if_idx, u8 *nw_type)
{
	int i;

	if (realtek_nliftype_to_drv_iftype(type, nw_type))
		return false;

	if (  type == NL80211_IFTYPE_AP 
        || type == NL80211_IFTYPE_STATION 
        || type == NL80211_IFTYPE_ADHOC
#if defined(P2P_SUPPORT)
        || type == 	NL80211_IFTYPE_P2P_CLIENT
        || type == 	NL80211_IFTYPE_P2P_GO
        || type == 	NL80211_IFTYPE_P2P_DEVICE
#endif
        ) //wrt-adhoc
		return true;

	return false;
}

char check_vif_existed(struct rtl8192cd_priv *priv, struct rtknl *rtk, unsigned char *name)
{
	char tmp = 0;
	
	for(tmp =0; tmp<= VIF_NUM; tmp++)
	{
		if(!strcmp(name, rtk->ndev_name[tmp]))
		{
			printk("%s = %s, existed in vif[%d]\n", name, rtk->ndev_name[tmp]);
			return 1;
		}
	}

	return 0;
}

unsigned char check_vif_type_match(struct rtl8192cd_priv *priv, unsigned char is_vxd)
{
	unsigned char ret = 0;

	NDEBUG3("priv[0x%x],(root=%d vxd=%d vap=%d)\n",	priv, IS_ROOT_INTERFACE(priv), IS_VXD_INTERFACE(priv), IS_VAP_INTERFACE(priv));
	NDEBUG3("proot_priv[0x%x],vap_id[%d]\n", priv->proot_priv, priv->vap_id);
	
	if(is_vxd && IS_VXD_INTERFACE(priv))
		ret = 1;
	
	if((!is_vxd) && IS_VAP_INTERFACE(priv))
		ret = 1;

	if(ret){
		NDEBUG2("is_vxd[%d],type OK \n", is_vxd);
	}else{
		NDEBUG2("is_vxd[%d],type NOT match \n", is_vxd);
	}

	return ret;
}

void rtk_change_netdev_name(struct rtl8192cd_priv *priv, unsigned char *name)
{
#if 0
	printk("rtk_change_netdev_name for priv = 0x%x (root=%d vxd=%d vap=%d) +++ \n", 
		priv, IS_ROOT_INTERFACE(priv), IS_VXD_INTERFACE(priv), IS_VAP_INTERFACE(priv));
	printk("from %s to %s \n", priv->dev->name, name);
#endif
#if defined(_INCLUDE_PROC_FS_) && defined(__KERNEL__)
	rtl8192cd_proc_remove(priv->dev);
#endif
	dev_change_name(priv->dev, name); //Need to modify kernel code to export this API
#if defined(_INCLUDE_PROC_FS_) && defined(__KERNEL__)
	rtl8192cd_proc_init(priv->dev);
#endif

}

struct rtl8192cd_priv* get_priv_vxd_from_rtk(struct rtknl *rtk)
{
	struct rtl8192cd_priv *priv = NULL;
	int tmp = 0;

	for(tmp = 0; tmp<(IF_NUM); tmp++)
	{
		if(rtk->rtk_iface[tmp].priv)
		if(IS_VXD_INTERFACE(rtk->rtk_iface[tmp].priv))
		{
			priv = rtk->rtk_iface[tmp].priv;
			break;
		}
	}

	//printk("name = %s priv_vxd = 0x%x \n", priv->dev->name, priv);

	return priv;
}

struct rtl8192cd_priv* get_priv_from_rtk(struct rtknl *rtk, unsigned char *name)
{
	struct rtl8192cd_priv *priv = NULL;
	int tmp = 0;

	for(tmp = 0; tmp<(IF_NUM); tmp++)
	{
		if(rtk->rtk_iface[tmp].priv)
		if(!strcmp(rtk->rtk_iface[tmp].priv->dev->name, name))
		{
			priv = rtk->rtk_iface[tmp].priv;
			break;
		}
	}

#if 0
	if(priv) //rtk_vap
	printk("get_priv_from_rtk name = %s priv = 0x%x %s\n", name, priv, priv->dev->name);
	else
		printk("get_priv_from_rtk = NULL !!\n");
#endif

	return priv;
}


struct rtl8192cd_priv* get_priv_from_ndev(struct rtknl *rtk, struct net_device *ndev)
{
	struct rtl8192cd_priv *priv = NULL;
	int tmp = 0;

	for(tmp = 0; tmp<(IF_NUM); tmp++)
	{
		if(rtk->rtk_iface[tmp].priv)
		if(ndev == rtk->rtk_iface[tmp].priv->dev)
		{
			priv = rtk->rtk_iface[tmp].priv;
			break;
		}
	}

	//printk("ndev = 0x%x priv = 0x%x \n", ndev, priv);

	return priv;
}

void rtk_add_priv(struct rtl8192cd_priv *priv_add, struct rtknl *rtk)
{
	
	int tmp = 0;

	for(tmp = 0; tmp<(IF_NUM); tmp++)
	{
		if(rtk->rtk_iface[tmp].priv == NULL)
		{
			rtk->rtk_iface[tmp].priv = priv_add;
			strcpy(rtk->rtk_iface[tmp].ndev_name, priv_add->dev->name); /*eric refine 23390*/ 
			break;
		}
	}

}

void rtk_del_priv(struct rtl8192cd_priv *priv_del, struct rtknl *rtk)
{

	int tmp = 0;

	for(tmp = 0; tmp<(IF_NUM); tmp++)
	{
		if(rtk->rtk_iface[tmp].priv == priv_del)
		{
			rtk->rtk_iface[tmp].priv = NULL;
			memset(rtk->rtk_iface[tmp].ndev_name, 0, 32);/*eric refine 23390*/ 
			break;
		}
	}

}

unsigned char find_ava_vif_idx(struct rtknl *rtk)
{
	unsigned char idx = 0;

	for(idx = 0; idx < VIF_NUM; idx ++)
	{
		if(rtk->ndev_name[idx][0] == 0)
			return idx;
	}

	return -1;
}

unsigned char get_vif_idx(struct rtknl *rtk, unsigned char *name)
{
	unsigned char idx = 0;

	for(idx = 0; idx < VIF_NUM; idx ++)
	{
		if(rtk->ndev_name[idx][0] != 0)
		if(strcmp(name, rtk->ndev_name[idx])==0)
			return idx;
	}

	return -1;
}


void realtek_create_vap_iface(struct rtknl *rtk, unsigned char *name)
{
	struct rtl8192cd_priv *priv = rtk->priv;

	if(check_vif_existed(priv, rtk, name))
	{
		printk("vif interface already existed !! \n");
		return 0;
	}

	if (rtk->num_vif == VIF_NUM) 
	{
		printk("Reached maximum number of supported vif\n");
		return -1;
	}

	rtk->idx_vif = find_ava_vif_idx(rtk);

	printk("rtk->idx_vif = %d\n", rtk->idx_vif);

	if(rtk->idx_vif < 0)
	{
		printk("rtk->idx_vif < 0 \n");
		return;
	}

	if(name){
		if(dev_valid_name(name))
			strcpy(rtk->ndev_name[rtk->idx_vif], name);
	}
	else
	{
		printk("No interface name !!\n");
		return -1;
	}

	rtl8192cd_init_one_cfg80211(rtk);
	rtk->num_vif++;

}

#endif

int realtek_interface_add(struct rtl8192cd_priv *priv, 
					  struct rtknl *rtk, const char *name,
					  enum nl80211_iftype type,
					  u8 fw_vif_idx, u8 nw_type)
{

 	struct net_device *ndev;
	struct ath6kl_vif *vif;

	NLENTER;

	NDEBUG("type[%d]\n", type);

	ndev = priv->dev;

	//dump_mac(priv, ndev->dev_addr);
	
	if (!ndev)
	{
		NDEBUG("ndev = NULL !!\n");
		free_netdev(ndev);
		return -1;
	}

	strcpy(ndev->name, name);
	realtek_change_iftype(priv, type);

	dev_net_set(ndev, wiphy_net(rtk->wiphy));

	priv->wdev.wiphy = rtk->wiphy;	

	ndev->ieee80211_ptr = &priv->wdev;

	SET_NETDEV_DEV(ndev, wiphy_dev(rtk->wiphy));
	
	priv->wdev.netdev = ndev;	
	priv->wdev.iftype = type;
	
	SET_NETDEV_DEVTYPE(ndev, &wiphy_type);

	priv->cfg80211_interface_add = FALSE;

	register_netdev(ndev);

#if 0
	if(IS_ROOT_INTERFACE(priv))
		register_netdev(ndev);
#ifdef UNIVERSAL_REPEATER //wrt-vxd
	else if(IS_VXD_INTERFACE(priv))
		register_netdev(ndev);
#endif
	else
		register_netdevice_name_rtk(ndev);
#endif

	rtk->ndev_add = ndev;

	NDEBUG2("add priv=[%p] wdev=[0x%x] ndev=[0x%x]\n", priv, &priv->wdev, ndev);
	rtk_add_priv(priv, rtk);

	NLEXIT;

	return 0;

}

#ifdef SUPPORT_MONITOR
void rtk_enable_monitor_mode(struct rtl8192cd_priv *priv)
{
	//if(priv->pmib->miscEntry.scan_enable)
		priv->chan_num = 0;

	priv->is_monitor_mode = TRUE;	
	RTL_W32(RCR, RCR_APP_FCS | RCR_APP_MIC | RCR_APP_ICV | RCR_APP_PHYSTS | RCR_HTC_LOC_CTRL
		  | RCR_AMF | RCR_ADF | RCR_AICV | RCR_ACRC32 | RCR_CBSSID_ADHOC | RCR_AB | RCR_AM | RCR_APM | RCR_AAP);

	init_timer(&priv->chan_switch_timer);
	priv->chan_switch_timer.data = (unsigned long) priv;
	priv->chan_switch_timer.function = rtl8192cd_chan_switch_timer;
	mod_timer(&priv->chan_switch_timer, jiffies + RTL_MILISECONDS_TO_JIFFIES(priv->pmib->miscEntry.chan_switch_time));
}

void rtk_disable_monitor_mode(struct rtl8192cd_priv *priv)
{		
	priv->is_monitor_mode = FALSE;
	if (timer_pending(&priv->chan_switch_timer))
		del_timer(&priv->chan_switch_timer);
}
#endif

void realtek_change_iftype(struct rtl8192cd_priv *priv ,enum nl80211_iftype type)
{
	//OPMODE &= ~(WIFI_STATION_STATE|WIFI_ADHOC_STATE|WIFI_AP_STATE);
#if defined(P2P_SUPPORT)
	if(IS_ROOT_INTERFACE(priv) && IS_DRV_OPEN(priv)){
		if(type != NL80211_IFTYPE_P2P_CLIENT) 
		rtl8192cd_close(priv->dev);
	}
#endif
	switch (type) {
	case NL80211_IFTYPE_STATION:
		OPMODE = WIFI_STATION_STATE;
		priv->pmib->p2p_mib.p2p_enabled=0;        
		priv->wdev.iftype = type;
		_NDEBUG("switch to [NL80211_IFTYPE_STATION]\n");                
		break;
	case NL80211_IFTYPE_ADHOC:
		OPMODE = WIFI_ADHOC_STATE;
		priv->wdev.iftype = type;
        priv->pmib->p2p_mib.p2p_enabled=0;                
		_NDEBUG("switch to [NL80211_IFTYPE_ADHOC]\n");                
		break;
	case NL80211_IFTYPE_AP:
		OPMODE = WIFI_AP_STATE;
		priv->wdev.beacon_interval = 0; 
		priv->pmib->miscEntry.func_off = 1;
		priv->pmib->p2p_mib.p2p_enabled=0;
		priv->wdev.iftype = type;
#ifdef SUPPORT_MONITOR
		rtk_disable_monitor_mode(priv);
#endif
#if defined(DFS)
		/*fixme, should not disable carrier here.
		  Under mac80211 architecture will be invoked by compatible-wireless */
		netif_carrier_off(priv->dev);
#endif
		_NDEBUG("switch to [NL80211_IFTYPE_AP]\n");
		break;
#if defined(P2P_SUPPORT)
	case NL80211_IFTYPE_P2P_CLIENT:
		OPMODE = (WIFI_STATION_STATE );      
        rtk_p2p_set_role(priv,P2P_DEVICE);        
        priv->pmib->p2p_mib.p2p_enabled=CFG80211_P2P;        
		priv->wdev.iftype = type;        
		_NDEBUG("switch to [NL80211_IFTYPE_P2P_CLIENT]\n");        
		break;
	case NL80211_IFTYPE_P2P_GO:
		OPMODE = (WIFI_AP_STATE );        
        rtk_p2p_set_role(priv,P2P_TMP_GO);        
        priv->pmib->p2p_mib.p2p_enabled=CFG80211_P2P;                
		priv->wdev.iftype = type;        
		_NDEBUG("switch to [NL80211_IFTYPE_P2P_GO]\n");        
		break;

    case NL80211_IFTYPE_P2P_DEVICE:
        OPMODE = (WIFI_STATION_STATE);
        rtk_p2p_set_role(priv,P2P_DEVICE);        
        priv->pmib->p2p_mib.p2p_enabled=CFG80211_P2P;         
        priv->wdev.iftype = type;        
		_NDEBUG("switch to [NL80211_IFTYPE_P2P_DEVICE]\n");
        break;
#endif
#ifdef SUPPORT_MONITOR
    case NL80211_IFTYPE_MONITOR:		
		OPMODE = (WIFI_SITE_MONITOR);
		priv->wdev.iftype = type;  
		priv->pmib->p2p_mib.p2p_enabled=0;
		rtk_enable_monitor_mode(priv);
		_NDEBUG("switch to [NL80211_IFTYPE_MONITOR]\n");
        break;
#endif
	default:
		NDEBUG("invalid interface type [%d]\n", type);
		OPMODE = WIFI_AP_STATE;
		return -EOPNOTSUPP;
	}
}

#ifdef OPENWRT_CC
void type_to_name(enum nl80211_iftype type, unsigned char* type_name)
#else
void type_to_name(type, type_name)
#endif
{

	switch (type) {
	case NL80211_IFTYPE_STATION:
		strcpy(type_name, "NL80211_IFTYPE_STATION");
		break;
	case NL80211_IFTYPE_ADHOC:
		strcpy(type_name, "NL80211_IFTYPE_ADHOC");
		break;
	case NL80211_IFTYPE_AP:
		strcpy(type_name, "NL80211_IFTYPE_AP");
		break;
    case NL80211_IFTYPE_P2P_CLIENT:
        strcpy(type_name, "NL80211_IFTYPE_P2P_CLIENT");
        break;
	case NL80211_IFTYPE_P2P_GO:
		strcpy(type_name, "NL80211_IFTYPE_P2P_GO");
		break;
	case NL80211_IFTYPE_MONITOR:	
		strcpy(type_name, "NL80211_IFTYPE_MONITOR");
		break;
	default:
		strcpy(type_name, "NOT SUPPORT TYPE");
		return -EOPNOTSUPP;
	}

}

static struct wireless_dev *realtek_cfg80211_add_iface(struct wiphy *wiphy,
						      const char *name,
//#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 1, 0))
#ifdef OPENWRT_CC
						      char name_assign_type,
#endif
						      enum nl80211_iftype type,
						      u32 *flags,
						      struct vif_params *params)
{
	struct rtknl *rtk = wiphy_priv(wiphy); //return &wiphy->priv;
	struct rtl8192cd_priv	*priv = rtk->priv;
	struct rtl8192cd_priv *priv_add = NULL;
	u8 if_idx, nw_type;
	unsigned char type_name[32];

	NLENTER;
	
	type_to_name(type, type_name);
	//NLMSG("ADD [%s][%s=%d]\n", name, type_name, type);

#if 0//def WDS
	if(params)
	{
		printk("use_4addr = %d \n", params->use_4addr);
	}
#endif

	if((strcmp(name, "wlan0")==0) || (strcmp(name, "wlan1")==0))
	{
		//NLMSG("Root interface, just change type\n");
		realtek_change_iftype(priv, type);
		return &rtk->priv->wdev;
	}	

	priv_add = get_priv_from_rtk(rtk, name);

	if(priv_add)
	{
		unsigned char type_match = 0; 
		unsigned char is_vxd = 0;

		if(is_WRT_scan_iface(name))
		{
			printk("Add Scan interface, do nothing\n");
			return &priv_add->wdev;
		}	

		if(type == NL80211_IFTYPE_AP)
		{
			is_vxd = 0;
			rtk->num_vap ++ ;
		}
		else
		{
			is_vxd = 1;
			rtk->num_vxd = 1;
		}
	
		type_match = check_vif_type_match(priv_add, is_vxd);

		if(!type_match)
		{
			unsigned char name_vxd[32];
			unsigned char name_vap[32];
			unsigned char name_tmp[32];
			struct rtl8192cd_priv *priv_vxd = NULL;
			struct rtl8192cd_priv *priv_vap = NULL;
			struct rtl8192cd_priv *priv_tmp = NULL;
			
			printk("Type NOT Match !!! need to change name\n");

			if(is_vxd)
			{
				priv_vap = priv_add;
				priv_vxd = get_priv_vxd_from_rtk(rtk);
			}
			else
			{
				sprintf(name_vap, "%s-%d", rtk->priv->dev->name, (RTL8192CD_NUM_VWLAN));
				priv_vap = get_priv_from_rtk(rtk, name_vap);
				priv_vxd = priv_add;
			}

			sprintf(name_tmp, "%s-%d", rtk->priv->dev->name, (RTL8192CD_NUM_VWLAN+10));
			
			strcpy(name_vap, priv_vap->dev->name);
			strcpy(name_vxd, priv_vxd->dev->name);
			
#if 0
			printk(" [BEFORE] +++ \n");
			printk("VAP = 0x%x(0x%x) name=%s \n", priv_vap, priv_vap->dev, priv_vap->dev->name);
			printk("VXD = 0x%x(0x%x) name=%s \n", priv_vxd, priv_vxd->dev, priv_vxd->dev->name);
#endif
			
			rtk_change_netdev_name(priv_vap, name_tmp);
			rtk_change_netdev_name(priv_vxd, name_vap);
			rtk_change_netdev_name(priv_vap, name_vxd);
	
#if 0
			printk(" [AFTER] --- \n");
			printk("VAP = 0x%x(0x%x) name=%s \n", priv_vap, priv_vap->dev, priv_vap->dev->name);
			printk("VXD = 0x%x(0x%x) name=%s \n", priv_vxd, priv_vxd->dev, priv_vxd->dev->name);
#endif


			if(is_vxd)
			{
#if 1 //wrt-adhoc
				{
					NDEBUG("\n\nVXD change type to %d \n\n", type);
					realtek_change_iftype(priv_vxd, type);
				}
#endif
				priv_vxd->cfg80211_interface_add = TRUE;
				return &priv_vxd->wdev;
			}
			else {
				priv_vap->cfg80211_interface_add = TRUE;
				return &priv_vap->wdev;
			}

		}
		else
		{
			printk("Type OK, do nothing\n");

#if 1 //wrt-adhoc
			if(is_vxd)
			{
    			NDEBUG("\n\nVXD change type to %d \n\n", type);                
				realtek_change_iftype(priv_add, type);
			}
#endif
			priv_add->cfg80211_interface_add = TRUE;
			return &priv_add->wdev;
		}

	}
	else 
	{
		printk("Can not find correspinding priv for %s !!\n", name);
		return -1;
	}

	NLEXIT;
	
	return &rtk->priv->wdev;

}


void close_vxd_vap(struct rtl8192cd_priv *priv_root)
{
	int i = 0;

//#ifdef UNIVERSAL_REPEATER
#if 0	//prevent drop vxd connection
	if(IS_DRV_OPEN(priv_root->pvxd_priv))
		rtl8192cd_close(priv_root->pvxd_priv->dev);
#endif
	
#ifdef MBSSID
	for (i=0; i<RTL8192CD_NUM_VWLAN; i++) { 
		if(IS_DRV_OPEN(priv_root->pvap_priv[i]))
			rtl8192cd_close(priv_root->pvap_priv[i]->dev);
	}
#endif

}

static int realtek_cfg80211_del_iface(struct wiphy *wiphy,
				     struct wireless_dev *wdev)
{
	struct rtknl *rtk = wiphy_priv(wiphy);
	struct rtl8192cd_priv	*priv = get_priv_from_wdev(rtk, wdev);
	unsigned char *name = NULL;

	NLENTER;

	if(priv) 
	{
		name = priv->dev->name;
		NDEBUG3("del_iface:name[%s] priv[0x%x] wdev[0x%x]\n", name, priv, wdev);
		memset(&wdev->preset_chandef,0,sizeof(wdev->preset_chandef));
	}
	else
	{
		NDEBUG3("Can NOT find priv from wdev[0x%x]", wdev);
		return -1;
	}	

	netif_stop_queue(priv->dev);

	//printk("Just close this interface\n");
	if(priv->cfg80211_interface_add == FALSE)
		return 0;

#ifdef MBSSID
	if(IS_ROOT_INTERFACE(priv))
	{
		close_vxd_vap(priv);
	}

#ifdef UNIVERSAL_REPEATER
	if(IS_VXD_INTERFACE(priv))
		rtk->num_vxd = 0;
#endif

	if(IS_VAP_INTERFACE(priv))
		rtk->num_vap --;
#endif
	priv->cfg80211_interface_add = FALSE;
	
	priv->receive_connect_cmd = 0;

	rtk_abort_scan(priv, SCAN_ABORT_DEL_IFACE);

    if(priv->pmib->dot11StationConfigEntry.dot11AclMode)
		realtek_reset_mac_acl(priv);

	rtl8192cd_close(priv->dev);

	NLEXIT;
	
	return 0;
}

//survey_dump
static int realtek_dump_survey(struct wiphy *wiphy, 
								struct net_device *dev,	
								int idx, 
								struct survey_info *survey)
{
	struct rtknl *rtk = wiphy_priv(wiphy);
	struct rtl8192cd_priv *priv = rtk->priv;
	struct ieee80211_supported_band *sband=NULL;
	int freq, band=0;

	//NLENTER;
	for (band = 0; band < IEEE80211_NUM_BANDS; band++) {
		sband = wiphy->bands[band];

		if (sband)
			break;;
	}

	if(!sband) {		
		NDEBUG("%s under unknown band!!\n",dev->name);
        return -1;
	} else {
		//this ops will be invoked several times, until statistic of all channels reported
		if(idx > sband->n_channels-1)
		{
			NDEBUG2("Exceed maximum:%d, statistic of all channels were reported \n", sband->n_channels-1);
			return -1;
		}
	}

	if(sband->band == NL80211_BAND_2GHZ)
		freq = ieee80211_channel_to_frequency(priv->rtk->survey_info[idx].channel, IEEE80211_BAND_2GHZ);
	else
		freq = ieee80211_channel_to_frequency(priv->rtk->survey_info[idx].channel, IEEE80211_BAND_5GHZ);

	survey->channel = ieee80211_get_channel(wiphy, freq);

	survey->noise = priv->rtk->survey_info[idx].noise;

#ifdef OPENWRT_CC
	survey->time = 1000;
	survey->time_busy = priv->rtk->survey_info[idx].chbusytime;
	survey->time_rx = priv->rtk->survey_info[idx].rx_time;
	survey->time_tx = priv->rtk->survey_info[idx].tx_time;
	survey->filled = SURVEY_INFO_NOISE_DBM|SURVEY_INFO_TIME|SURVEY_INFO_TIME_BUSY|SURVEY_INFO_TIME_RX|SURVEY_INFO_TIME_TX;
#else
	survey->channel_time = 1000;
	survey->channel_time_busy = priv->rtk->survey_info[idx].chbusytime;
	survey->channel_time_rx = priv->rtk->survey_info[idx].rx_time;
	survey->channel_time_tx = priv->rtk->survey_info[idx].tx_time;
	survey->filled = SURVEY_INFO_NOISE_DBM|SURVEY_INFO_CHANNEL_TIME|SURVEY_INFO_CHANNEL_TIME_BUSY|SURVEY_INFO_CHANNEL_TIME_RX|SURVEY_INFO_CHANNEL_TIME_TX;
#endif

#if 0
	if(priv->rtk->survey_info[idx].chbusytime > priv->rtk->survey_info[idx].rx_time)
	{
		survey->channel_time_busy = priv->rtk->survey_info[idx].chbusytime + priv->rtk->survey_info[idx].tx_time;
		survey->channel_time_rx = priv->rtk->survey_info[idx].rx_time;
	}
	else
	{
		survey->channel_time_busy = priv->rtk->survey_info[idx].rx_time + priv->rtk->survey_info[idx].tx_time;
		survey->channel_time_rx = priv->rtk->survey_info[idx].rx_time - priv->rtk->survey_info[idx].chbusytime;
	}
	survey->channel_time_tx = priv->rtk->tx_time;
#endif

	//NLEXIT;
	
	return 0;
}


static int realtek_cfg80211_change_iface(struct wiphy *wiphy,
					struct net_device *ndev,
					enum nl80211_iftype type, u32 *flags,
					struct vif_params *params)
{
	struct rtknl *rtk = wiphy_priv(wiphy);
	struct rtl8192cd_priv	*priv = get_priv_from_ndev(rtk, ndev); //rtk->priv;
	int i;
	unsigned char type_name[32];

	NLENTER;

	type_to_name(type, type_name);
	NLMSG("CHANGE [%s][%s=%d]\n", priv->dev->name, type_name, type);
	realtek_change_iftype(priv, type);

	NLEXIT;
	return 0;
}




#if 0
static int realtek_cfg80211_sscan_start(struct wiphy *wiphy,
			struct net_device *dev,
			struct cfg80211_sched_scan_request *request)
{

	return 0;
}

static int realtek_cfg80211_sscan_stop(struct wiphy *wiphy,
				      struct net_device *dev)
{

	return 0;
}

static int realtek_cfg80211_set_bitrate(struct wiphy *wiphy,
				       struct net_device *dev,
				       const u8 *addr,
				       const struct cfg80211_bitrate_mask *mask)
{

	return 0;
}

static int realtek_cfg80211_set_txe_config(struct wiphy *wiphy,
					  struct net_device *dev,
					  u32 rate, u32 pkts, u32 intvl)
{

	return 0;
}
#endif

#if defined(P2P_SUPPORT)
int realtek_remain_on_channel(struct wiphy *wiphy,
    struct wireless_dev *wdev,
	struct ieee80211_channel *channel,
	unsigned int duration,
	u64 *cookie)

{
	struct rtknl *rtk = wiphy_priv(wiphy);
	struct rtl8192cd_priv *priv = get_priv_from_wdev(rtk, wdev);
    //    struct wireless_dev *wdev = (&priv->wdev);   
	u8 remain_ch = (u8) ieee80211_frequency_to_channel(channel->center_freq);
    
	NLENTER;

    if (timer_pending(&priv->p2pPtr->remain_on_ch_timer))
        del_timer(&priv->p2pPtr->remain_on_ch_timer);

    //rtk_abort_scan(priv);  // abort on going scan
	memcpy(&priv->p2pPtr->remain_on_ch_channel, channel, sizeof(struct ieee80211_channel));
	priv->p2pPtr->remain_on_ch_cookie= *cookie;
    priv->p2pPtr->restore_channel=GET_ROOT(priv)->pmib->dot11RFEntry.dot11channel;  /*restore orignal channel*/ 

    priv->pmib->p2p_mib.p2p_listen_channel=remain_ch;   /*set listen channel to remain channel */ 

    priv->p2pPtr->pre_p2p_role=rtk_p2p_get_role(priv);
    priv->p2pPtr->pre_p2p_state=rtk_p2p_get_state(priv);
    
    //priv->p2pPtr->pre_opmode = OPMODE;
    #if 0 //before 0326 1450
    if(rtk_p2p_is_enabled(priv)==CFG80211_P2P){                // rdy under p2p mode 
        if(!rtk_p2p_chk_role(priv,P2P_DEVICE)){
            NDEBUG("change mode to p2p-device\n");
            rtk_p2p_set_role(priv,P2P_DEVICE);
        }
    }else{
        NDEBUG("rtk_p2p_enable\n");                
        rtk_p2p_enable(priv,P2P_DEVICE, CFG80211_P2P);
    }
    #else
    if(!rtk_p2p_is_enabled(priv)){
        NDEBUG3("==>rtk_p2p_enable(CFG80211_P2P)\n");                
        rtk_p2p_enable(priv,P2P_DEVICE, CFG80211_P2P);
    }
    #endif

    rtk_p2p_set_role(priv,P2P_DEVICE);    // role to device
    P2P_listen(priv,NULL);                //state to listen  

    
    NDEBUG3("role[%d]\n",rtk_p2p_get_role(priv));
    NDEBUG3("state[%d]\n",rtk_p2p_get_state(priv));    
    priv->pshare->rtk_remain_on_channel=1; // for lock channel switch    and  indicate now under remain channel mode

    cfg80211_ready_on_channel(wdev, *cookie, channel, duration, GFP_KERNEL);    

	if(duration < 400)
		duration = duration*3;//extend from exper. unit ms

	mod_timer(&priv->p2pPtr->remain_on_ch_timer,jiffies + RTL_MILISECONDS_TO_JIFFIES(duration)); 
    //NDEBUG("ch[%d],for[%d]ms\n",remain_ch,duration);
	return 0;

}
static int realtek_cancel_remain_on_channel(struct wiphy *wiphy,
			struct wireless_dev *wdev,	u64 cookie)
{
	struct rtknl *rtk = wiphy_priv(wiphy);
	struct rtl8192cd_priv *priv = get_priv_from_wdev(rtk, wdev);
    NLENTER;

    if(timer_pending(&priv->p2pPtr->remain_on_ch_timer)){
        del_timer(&priv->p2pPtr->remain_on_ch_timer);
    }
    NDEBUG("\n");

    priv->pshare->rtk_remain_on_channel=0; // for unlock channel switch    
    rtk_p2p_set_role(priv,priv->p2pPtr->pre_p2p_role);            
    rtk_p2p_set_state(priv,priv->p2pPtr->pre_p2p_state);
	return 0;
}

void realtek_cfg80211_RemainOnChExpire(unsigned long task_priv)
{

	struct rtl8192cd_priv *priv = (struct rtl8192cd_priv *)task_priv;	
    struct wireless_dev *wdev = &priv->wdev;    

    priv->pshare->rtk_remain_on_channel=0; // for unlock channel switch    
    rtk_p2p_set_role(priv,priv->p2pPtr->pre_p2p_role);        
    rtk_p2p_set_state(priv,priv->p2pPtr->pre_p2p_state);    
    //NDEBUG2("\n");
    cfg80211_remain_on_channel_expired(wdev,priv->p2pPtr->remain_on_ch_cookie,&priv->p2pPtr->remain_on_ch_channel,GFP_KERNEL);
    return;		  					
}

int rtk_cfg80211_mgmt_tx(struct rtl8192cd_priv *priv,int tx_ch,const u8 *mgmt_buf_from_cfg, int mgmt_buf_from_cfg_len)
{
	unsigned char	*pbuf;
	unsigned char	*frame_after_wlan_hrd=NULL;	
  	int	frame_after_wlan_hrd_len = 0;	
  	int	sizeof_mgmt_wlan_hrd = 0;	
    struct ieee80211_mgmt *mgmt;    
    int idx=0;
    u8 category,action;
    u8 OUI_Subtype;
    u8 dialogToken;    
    unsigned char	P2P_OUI[] = {0x50,0x6F,0x9A,0x09};    
    u8* p2p_ie_start;    
    int  p2p_ie_len;
    int  my_p2p_role;
    unsigned char* p2p_capa;                          
    //struct ieee80211_mgmt_hrd mgmt_hrd;    
    
	DECLARE_TXINSN(txinsn);

    if(priv->pshare->working_channel2 != tx_ch){
        NDEBUG("    chk , our working ch != assigned by cfg\n\n");
        priv->pshare->CurrentChannelBW = HT_CHANNEL_WIDTH_20;
        SwBWMode(priv, priv->pshare->CurrentChannelBW, priv->pshare->offset_2nd_chan);  
        SwChnl(priv, tx_ch, priv->pshare->offset_2nd_chan);        
    }
    //rtk_set_scan_deny(priv,300);   // deny channel switch for 300 ms  
	pbuf = txinsn.pframe = get_mgtbuf_from_poll(priv);
	if (pbuf == NULL)
		goto fail;

	txinsn.phdr = get_wlanhdr_from_poll(priv);

	if (txinsn.phdr == NULL)
		goto fail;
    

	txinsn.q_num = MANAGE_QUE_NUM;
	txinsn.fr_type = _PRE_ALLOCMEM_;
	txinsn.tx_rate = _6M_RATE_;
#ifndef TX_LOWESTRATE
	txinsn.lowest_tx_rate = txinsn.tx_rate;
#endif
	txinsn.fixed_rate = true;
    txinsn.need_ack = true;
    txinsn.retry = true;



    sizeof_mgmt_wlan_hrd = sizeof(struct ieee80211_mgmt_hrd); 
	frame_after_wlan_hrd = mgmt_buf_from_cfg + sizeof_mgmt_wlan_hrd;
  	frame_after_wlan_hrd_len = mgmt_buf_from_cfg_len-sizeof_mgmt_wlan_hrd;
    

    /*-------------parse p2p_ie and do some handling----------------*/    
    category = frame_after_wlan_hrd[0];
    if(category == WLAN_CATEGORY_PUBLIC)  // 0x4 p2p public action frame
    {
        action = frame_after_wlan_hrd[1];
        if (action == _P2P_PUBLIC_ACTION_FIELD_ && !memcmp(frame_after_wlan_hrd+2, WFA_OUI_PLUS_TYPE, 4))
        {
            OUI_Subtype = frame_after_wlan_hrd[6];
            dialogToken = frame_after_wlan_hrd[7];    
            switch(OUI_Subtype){
                case P2P_GO_NEG_REQ:
                    NDEBUG("P2P_GO_NEG_REQ ,dialog[%d]\n",dialogToken);
                    break;                                  
                case P2P_GO_NEG_RESP:
                    NDEBUG("P2P_GO_NEG_RESP \n");                    
                    
                    break;                        
                case P2P_GO_NEG_CONF:
                    NDEBUG("P2P_GO_NEG_CONF\n");
                                    
                    break;                        
                case P2P_INVITATION_REQ:
                    NDEBUG("P2P_INVITATION_REQ,dialog[%d]\n",dialogToken);
                    break;                        
                case P2P_INVITATION_RESP:
                    NDEBUG("P2P_INVITATION_RESP,dialog[%d]\n",dialogToken);
                    break;                        
                case P2P_DEV_DISC_REQ:
                    NDEBUG("P2P_INVITATION_RESP,dialog[%d]\n",dialogToken);
                    rtk_p2p_set_state(priv,P2P_S_IDLE);
                    break;                        
                case P2P_DEV_DISC_RESP:
                    NDEBUG("P2P_DEV_DISC_RESP,dialog[%d]\n",dialogToken);
                    rtk_p2p_set_state(priv,P2P_S_IDLE);                    
                    break;                        
                case P2P_PROV_DISC_REQ:
                    NDEBUG("P2P_PROV_DISC_REQ,dialog[%d]\n",dialogToken);
                    break;                        
                case P2P_PROV_DISC_RSP:
                    NDEBUG("P2P_PROV_DISC_RSP,dialog[%d]\n",dialogToken);
                    break;  
                 default:
                    NDEBUG("unknown [%d]\n",dialogToken);                    
                    break;
            }    
        }
        
    }   
    else if(category == WLAN_CATEGORY_VENDOR_SPECIFIC)  // 0x7F action frame
    {
        OUI_Subtype = frame_after_wlan_hrd[5];
        dialogToken = frame_after_wlan_hrd[6];          
        switch(OUI_Subtype)
        {
            case P2P_NOA:
                NDEBUG("P2P_NOA,dialog[%d]\n",dialogToken);
                break;          
            case P2P_PRESENCE_REQ:
                NDEBUG("P2P_PRESENCE_REQ,dialog[%d]\n",dialogToken);
                break;          
            case P2P_PRESENCE_RSP:
                NDEBUG("P2P_PRESENCE_RSP,dialog[%d]\n",dialogToken);
                break;          
            case P2P_GO_DISCOVERY:
                NDEBUG("P2P_GO_DISCOVERY,dialog[%d]\n",dialogToken);
                break;          
            default:
                NDEBUG("unknown,dialog[%d]\n",dialogToken);
                break;          
        }   

    }   
    else
    {
        NDEBUG("unknown action frame\n");
    }
    /*-------------parse p2p_ie and do some handling----------------*/
    

    /*fill wlan head*/
	memset((void *)(txinsn.phdr), 0, sizeof(struct wlan_hdr));
    memcpy((void *)(txinsn.phdr) , mgmt_buf_from_cfg , sizeof(struct ieee80211_mgmt_hrd));

    /*fill frame content after wlan head*/
    memcpy((void *)pbuf , frame_after_wlan_hrd , frame_after_wlan_hrd_len);

    
	txinsn.fr_len += frame_after_wlan_hrd_len;

	if ((rtl8192cd_firetx(priv, &txinsn)) == SUCCESS){
		return SUCCESS;
	}else{
		P2P_DEBUG("TX action fail\n");
    }

fail:
    NDEBUG("fail !!!\n");

	if (txinsn.phdr)
		release_wlanhdr_to_poll(priv, txinsn.phdr);
	if (txinsn.pframe)
		release_mgtbuf_to_poll(priv, txinsn.pframe);
	return FAIL;

	
}
#endif
static int realtek_mgmt_tx(struct wiphy *wiphy, struct wireless_dev *wdev,
           struct cfg80211_mgmt_tx_params *params,
           u64 *cookie)
{
	struct rtknl *rtk = wiphy_priv(wiphy);
	struct rtl8192cd_priv *priv = get_priv_from_wdev(rtk, wdev);
	int tx_ch = ieee80211_frequency_to_channel(params->chan->center_freq);

	int ret = 0;
    u32 cookie_id;
    const struct ieee80211_mgmt *mgmt;    
	NLENTER;    
   
	*cookie = 0;    
#ifdef P2P_SUPPORT
    priv->p2pPtr->send_action_id++;    
    if(priv->p2pPtr->send_action_id==0)
        priv->p2pPtr->send_action_id++;            

    
    *cookie = priv->p2pPtr->send_action_id;
#else
	priv->mgmt_action_id++;
	if(!priv->mgmt_action_id)
		priv->mgmt_action_id++;
	*cookie = priv->mgmt_action_id;
#endif

	mgmt = (const struct ieee80211_mgmt *)params->buf;    
    
	if (ieee80211_is_mgmt(mgmt->frame_control)) {
		if (ieee80211_is_probe_resp(mgmt->frame_control)) {
			s32 ie_offset =  WLAN_HDR_A3_LEN + _PROBERSP_IE_OFFSET_;
			s32 ie_len = params->len - ie_offset;
            NDEBUG("!!!set probe rsp from cfg80211\n");
            rtk_cfg80211_set_wps_p2p_ie(priv,params->buf+ie_offset,ie_len,MGMT_PROBERSP);
            cfg80211_mgmt_tx_status(wdev, *cookie, params->buf, params->len, TRUE, GFP_KERNEL);//GFP_ATOMIC            
            return 0;            			
		} else if (ieee80211_is_disassoc(mgmt->frame_control) ||
			ieee80211_is_deauth(mgmt->frame_control)) {
			NDEBUG("Disconnect STA[%02X%02X%02X:%02X%02X%02X] reason[%d]\n",
                mgmt->da[0],mgmt->da[1],mgmt->da[2],
                mgmt->da[3],mgmt->da[4],mgmt->da[5],mgmt->u.disassoc.reason_code);

            issue_deauth(priv,mgmt->da,mgmt->u.disassoc.reason_code);
            cfg80211_mgmt_tx_status(wdev, *cookie, params->buf, params->len, TRUE, GFP_KERNEL);//GFP_ATOMIC            
            return 0;

		} else if (ieee80211_is_action(mgmt->frame_control)) {

                /* indicate ack before issue frame to avoid racing with rsp frame */
                    
                //rtk_abort_scan(priv);  // abort on going scan
#ifdef P2P_SUPPORT
                rtk_cfg80211_mgmt_tx(priv, tx_ch, params->buf, params->len);
#endif
                cfg80211_mgmt_tx_status(wdev, *cookie, params->buf, params->len, TRUE, GFP_KERNEL);//GFP_ATOMIC                            
                //NDEBUG("cookie_id[%02X]\n",priv->p2pPtr->send_action_id);    

		}

	}    
	return 0;

}


struct cfg80211_ops realtek_cfg80211_ops = {
	.add_virtual_intf = realtek_cfg80211_add_iface,
	.del_virtual_intf = realtek_cfg80211_del_iface,
	.change_virtual_intf = realtek_cfg80211_change_iface,
	.add_key = realtek_cfg80211_add_key,
	.del_key = realtek_cfg80211_del_key,
	.get_key = realtek_cfg80211_get_key,
	.set_default_key = realtek_cfg80211_set_default_key,
	.set_default_mgmt_key = realtek_cfg80211_set_default_mgmt_key,
	//.add_beacon = realtek_cfg80211_add_beacon,
	//.set_beacon = realtek_cfg80211_set_beacon,
	//.del_beacon = realtek_cfg80211_del_beacon,
	.add_station = realtek_cfg80211_add_station,
	.del_station = realtek_cfg80211_del_station,
	.change_station = realtek_cfg80211_change_station,
	.get_station = realtek_cfg80211_get_station,
	.dump_station = realtek_cfg80211_dump_station,
#if 0//def CONFIG_MAC80211_MESH
		.add_mpath = realtek_cfg80211_add_mpath,
		.del_mpath = realtek_cfg80211_del_mpath,
		.change_mpath = realtek_cfg80211_change_mpath,
		.get_mpath = realtek_cfg80211_get_mpath,
		.dump_mpath = realtek_cfg80211_dump_mpath,
		.set_mesh_params = realtek_cfg80211_set_mesh_params,
		.get_mesh_params = realtek_cfg80211_get_mesh_params,
#endif
	.change_bss = realtek_cfg80211_change_bss,
	//.set_txq_params = realtek_cfg80211_set_txq_params,
	//.set_channel = realtek_cfg80211_set_channel,
	.suspend = realtek_cfg80211_suspend,
	.resume = realtek_cfg80211_resume,
	.scan = realtek_cfg80211_scan,
#if 0
	.auth = realtek_cfg80211_auth,
	.assoc = realtek_cfg80211_assoc,
	.deauth = realtek_cfg80211_deauth,
	.disassoc = realtek_cfg80211_disassoc,
#endif
	.join_ibss = realtek_cfg80211_join_ibss,
	.leave_ibss = realtek_cfg80211_leave_ibss,
	.set_wiphy_params = realtek_cfg80211_set_wiphy_params,
	.set_ap_chanwidth = realtek_cfg80211_set_ap_chanwidth,
	.set_monitor_channel = realtek_cfg80211_set_monitor_channel,
	.set_tx_power = realtek_cfg80211_set_tx_power,
	.get_tx_power = realtek_cfg80211_get_tx_power,
	.set_power_mgmt = realtek_cfg80211_set_power_mgmt,
	.set_wds_peer = realtek_cfg80211_set_wds_peer,
	.rfkill_poll = realtek_cfg80211_rfkill_poll,
	//CFG80211_TESTMODE_CMD(ieee80211_testmode_cmd)
	.set_bitrate_mask = realtek_cfg80211_set_bitrate_mask,
	.connect = realtek_cfg80211_connect,
	.disconnect = realtek_cfg80211_disconnect,
	#if defined(P2P_SUPPORT)
	.remain_on_channel = realtek_remain_on_channel,
	.cancel_remain_on_channel = realtek_cancel_remain_on_channel,
	#endif
	.mgmt_tx = realtek_mgmt_tx,
    .mgmt_frame_register = realtek_mgmt_frame_register,
	.channel_switch = realtek_cfg80211_channel_switch,
	.dump_survey = realtek_dump_survey,//survey_dump
	.start_ap = realtek_start_ap,
	.change_beacon = realtek_change_beacon,
	.stop_ap = realtek_stop_ap,
#if 0
	.sched_scan_start = realtek_cfg80211_sscan_start,
	.sched_scan_stop = realtek_cfg80211_sscan_stop,
	.set_bitrate_mask = realtek_cfg80211_set_bitrate,
	.set_cqm_txe_config = realtek_cfg80211_set_txe_config,
#endif
	.set_mac_acl = realtek_set_mac_acl,
#if defined(DFS)
	.start_radar_detection = realtek_start_radar_detection
#endif
};

static void  rtk_create_dev(struct rtknl *rtk,int idx)
{
	/* define class here */
	unsigned char zero[] = {0, 0, 0, 0, 0, 0};
    rtk->cl = class_create(THIS_MODULE, rtk_dev_name[idx]); 
 
    /* create first device */
    rtk->dev = device_create(rtk->cl, NULL, rtk_wifi_dev[idx], NULL, rtk_dev_name[idx]);

 	dev_set_name(rtk->dev, rtk_dev_name[idx]);
  	printk("Device Name = %s \n", dev_name(rtk->dev));

	printk("VIF_NUM=%d\n", VIF_NUM);
	memset(rtk->ndev_name, 0, VIF_NUM*VIF_NAME_SIZE);

	//init rtk phy root name
	sprintf(rtk->root_ifname, "wlan%d", idx);

	//mark_dual ,init with fake mac for diff phy
	rtk_fake_addr[3] += ((unsigned char)idx) ;
	memcpy(rtk->root_mac, rtk_fake_addr, ETH_ALEN);
}

void rtk_remove_dev(struct rtknl *rtk,int idx)
{
	printk("remove cfg80211 device[%d]\n", idx);
	//remove device
	if(rtk->dev)
	{
		device_destroy(rtk->cl, rtk_wifi_dev[idx]);
		rtk->dev = NULL;
	}

	//remove class
	if(rtk->cl)
	{
		class_destroy(rtk->cl);
		rtk->cl = NULL;
	}
}


//struct rtknl *realtek_cfg80211_create(struct rtl8192cd_priv *priv)
struct rtknl *realtek_cfg80211_create(void) 
{
	struct wiphy *wiphy;
	struct rtknl *rtk;

	//NLENTER;

	/* create a new wiphy for use with cfg80211 */
	wiphy = wiphy_new(&realtek_cfg80211_ops, sizeof(struct rtknl));

	if (!wiphy) {
		printk("couldn't allocate wiphy device\n"); 
		return NULL;
	}

	rtk = wiphy_priv(wiphy);
	rtk->wiphy = wiphy;
	//rtk->priv = priv;	 //mark_dual2
	
	//sync to global rtk_phy
	if(rtk_phy_idx > RTK_MAX_WIFI_PHY)
	{		
		printk("ERROR!! rtk_phy_idx >  RTK_MAX_WIFI_PHY\n");
		wiphy_free(wiphy);
		return NULL;
	}
	rtk_create_dev(rtk,rtk_phy_idx);	
	rtk_phy[rtk_phy_idx] = rtk;
	rtk_phy_idx++;

	//priv->rtk = rtk ; //mark_dual2

	//NLEXIT;
	return rtk;
}

/*cfg p2p*/
#if 1
static const struct ieee80211_txrx_stypes rtw_cfg80211_default_mgmt_stypes[NUM_NL80211_IFTYPES] = {
	[NL80211_IFTYPE_ADHOC] = {
		.tx = 0xffff,
		.rx = BIT(IEEE80211_STYPE_ACTION >> 4)
	},
	[NL80211_IFTYPE_STATION] = {
		.tx = 0xffff,
		.rx = BIT(IEEE80211_STYPE_ACTION >> 4) |
		BIT(IEEE80211_STYPE_PROBE_REQ >> 4)
	},
	[NL80211_IFTYPE_AP] = {
		.tx = 0xffff,
		.rx = BIT(IEEE80211_STYPE_ASSOC_REQ >> 4) |
		BIT(IEEE80211_STYPE_REASSOC_REQ >> 4) |
		BIT(IEEE80211_STYPE_PROBE_REQ >> 4) |
		BIT(IEEE80211_STYPE_DISASSOC >> 4) |
		BIT(IEEE80211_STYPE_AUTH >> 4) |
		BIT(IEEE80211_STYPE_DEAUTH >> 4) |
		BIT(IEEE80211_STYPE_ACTION >> 4)
	},
	[NL80211_IFTYPE_AP_VLAN] = {
		/* copy AP */
		.tx = 0xffff,
		.rx = BIT(IEEE80211_STYPE_ASSOC_REQ >> 4) |
		BIT(IEEE80211_STYPE_REASSOC_REQ >> 4) |
		BIT(IEEE80211_STYPE_PROBE_REQ >> 4) |
		BIT(IEEE80211_STYPE_DISASSOC >> 4) |
		BIT(IEEE80211_STYPE_AUTH >> 4) |
		BIT(IEEE80211_STYPE_DEAUTH >> 4) |
		BIT(IEEE80211_STYPE_ACTION >> 4)
	},
#if defined(P2P_SUPPORT)
	[NL80211_IFTYPE_P2P_CLIENT] = {
		.tx = 0xffff,
		.rx = BIT(IEEE80211_STYPE_ACTION >> 4) |
		BIT(IEEE80211_STYPE_PROBE_REQ >> 4)
	},
	[NL80211_IFTYPE_P2P_GO] = {
		.tx = 0xffff,
		.rx = BIT(IEEE80211_STYPE_ASSOC_REQ >> 4) |
		BIT(IEEE80211_STYPE_REASSOC_REQ >> 4) |
		BIT(IEEE80211_STYPE_PROBE_REQ >> 4) |
		BIT(IEEE80211_STYPE_DISASSOC >> 4) |
		BIT(IEEE80211_STYPE_AUTH >> 4) |
		BIT(IEEE80211_STYPE_DEAUTH >> 4) |
		BIT(IEEE80211_STYPE_ACTION >> 4)
	},
#endif
};
#endif

#define MAX_REMAIN_ON_CHANNEL_DURATION 5000 //ms
#define SCAN_IE_LEN_MAX                2304	
#define SSID_SCAN_AMOUNT               1 // for WEXT_CSCAN_AMOUNT 9
#define MAX_NUM_PMKIDS                 32 
#define	NL_MAX_INTF						9

static const struct ieee80211_iface_limit rtk_if_limits[] = {
	{ .max = 8,	.types = BIT(NL80211_IFTYPE_AP) },
	{ .max = 1,	.types = BIT(NL80211_IFTYPE_STATION) },
};

static const struct ieee80211_iface_combination rtk_2g_comb = {
	.limits = rtk_if_limits,
	.n_limits = ARRAY_SIZE(rtk_if_limits),
	.max_interfaces = NL_MAX_INTF,
	.num_different_channels = 1,
};

static const struct ieee80211_iface_combination rtk_5g_comb = {
	.limits = rtk_if_limits,
	.n_limits = ARRAY_SIZE(rtk_if_limits),
	.max_interfaces = NL_MAX_INTF,
	.num_different_channels = 1,
	.radar_detect_widths =	BIT(NL80211_CHAN_WIDTH_20_NOHT) |
					BIT(NL80211_CHAN_WIDTH_20) |
					BIT(NL80211_CHAN_WIDTH_40) |
					BIT(NL80211_CHAN_WIDTH_80),
};

/**
 * enum wiphy_flags - wiphy capability flags
 *
 * @WIPHY_FLAG_CUSTOM_REGULATORY:  tells us the driver for this device
 * 	has its own custom regulatory domain and cannot identify the
 * 	ISO / IEC 3166 alpha2 it belongs to. When this is enabled
 * 	we will disregard the first regulatory hint (when the
 * 	initiator is %REGDOM_SET_BY_CORE).

 * @WIPHY_FLAG_STRICT_REGULATORY: tells us the driver for this device will
 *	ignore regulatory domain settings until it gets its own regulatory
 *	domain via its regulatory_hint() unless the regulatory hint is
 *	from a country IE. After its gets its own regulatory domain it will
 *	only allow further regulatory domain settings to further enhance
 *	compliance. For example if channel 13 and 14 are disabled by this
 *	regulatory domain no user regulatory domain can enable these channels
 *	at a later time. This can be used for devices which do not have
 *	calibration information guaranteed for frequencies or settings
 *	outside of its regulatory domain. If used in combination with
 *	WIPHY_FLAG_CUSTOM_REGULATORY the inspected country IE power settings
 *	will be followed.

 * @WIPHY_FLAG_DISABLE_BEACON_HINTS: enable this if your driver needs to ensure
 *	that passive scan flags and beaconing flags may not be lifted by
 *	cfg80211 due to regulatory beacon hints. For more information on beacon
 *	hints read the documenation for regulatory_hint_found_beacon()

 * @WIPHY_FLAG_NETNS_OK: if not set, do not allow changing the netns of this
 *	wiphy at all

 * @WIPHY_FLAG_PS_ON_BY_DEFAULT: if set to true, powersave will be enabled
 *	by default -- this flag will be set depending on the kernel's default
 *	on wiphy_new(), but can be changed by the driver if it has a good
 *	reason to override the default

 * @WIPHY_FLAG_4ADDR_AP: supports 4addr mode even on AP (with a single station
 *	on a VLAN interface)

 * @WIPHY_FLAG_4ADDR_STATION: supports 4addr mode even as a station

 * @WIPHY_FLAG_CONTROL_PORT_PROTOCOL: This device supports setting the
 *	control port protocol ethertype. The device also honours the
 *	control_port_no_encrypt flag.

 * @WIPHY_FLAG_IBSS_RSN: The device supports IBSS RSN.

 * @WIPHY_FLAG_MESH_AUTH: The device supports mesh authentication by routing
 *	auth frames to userspace. See @NL80211_MESH_SETUP_USERSPACE_AUTH.

 * @WIPHY_FLAG_SUPPORTS_SCHED_SCAN: The device supports scheduled scans.

 * @WIPHY_FLAG_SUPPORTS_FW_ROAM: The device supports roaming feature in the 	firmware.
 * @WIPHY_FLAG_AP_UAPSD: The device supports uapsd on AP.
 * @WIPHY_FLAG_SUPPORTS_TDLS: The device supports TDLS (802.11z) operation.
 * @WIPHY_FLAG_TDLS_EXTERNAL_SETUP: The device does not handle TDLS (802.11z)
 *	link setup/discovery operations internally. Setup, discovery and
 *	teardown packets should be sent through the @NL80211_CMD_TDLS_MGMT
 *	command. When this flag is not set, @NL80211_CMD_TDLS_OPER should be
 *	used for asking the driver/firmware to perform a TDLS operation.
 * @WIPHY_FLAG_HAVE_AP_SME: device integrates AP SME
 * @WIPHY_FLAG_REPORTS_OBSS: the device will report beacons from other BSSes
 *	when there are virtual interfaces in AP mode by calling
 *	cfg80211_report_obss_beacon().
 * @WIPHY_FLAG_AP_PROBE_RESP_OFFLOAD: When operating as an AP, the device
 *	responds to probe-requests in hardware.
 * @WIPHY_FLAG_OFFCHAN_TX: Device supports direct off-channel TX.
 * @WIPHY_FLAG_HAS_REMAIN_ON_CHANNEL: Device supports remain-on-channel call.
 */
int realtek_cfg80211_init(struct rtknl *rtk,struct rtl8192cd_priv *priv)  
{
	struct wiphy *wiphy = rtk->wiphy;
	BOOLEAN band_2gig = false, band_5gig = false;
	int ret;
#ifdef EN_EFUSE
	char efusemac[ETH_ALEN];
#endif
	char zero[ETH_ALEN] = {0,0,0,0,0,0};
	unsigned char txbf_max_ant, txbf_sounding_dim;
	NLENTER;
	rtk->priv = priv;  //mark_dual	

	rtk_get_band_capa(priv,&band_2gig,&band_5gig);

	//wiphy->mgmt_stypes = realtek_mgmt_stypes; //_eric_cfg ??
	wiphy->mgmt_stypes = rtw_cfg80211_default_mgmt_stypes; /*cfg p2p*/

#if defined(SIGNAL_TYPE_UNSPEC)
	wiphy->signal_type=CFG80211_SIGNAL_TYPE_UNSPEC;
#else
	wiphy->signal_type = CFG80211_SIGNAL_TYPE_MBM; //mark_priv
#endif
	/* max num of ssids that can be probed during scanning */
	//wiphy->max_scan_ssids = MAX_PROBED_SSIDS;
    wiphy->max_scan_ssids = SSID_SCAN_AMOUNT;	

	/* max num of ssids that can be matched after scan */
	//wiphy->max_match_sets = MAX_PROBED_SSIDS;
    
	//wiphy->max_scan_ie_len = 1000; /* FIX: what is correct limit? */    
    wiphy->max_scan_ie_len = SCAN_IE_LEN_MAX;   
    wiphy->max_num_pmkids = MAX_NUM_PMKIDS;  
	wiphy->max_remain_on_channel_duration = MAX_REMAIN_ON_CHANNEL_DURATION; /*cfg p2p p2p related*/ 

	switch(GET_CHIP_VER(priv)) {
		case VERSION_8188C:
		case VERSION_8188E:
		case VERSION_8881A:
			wiphy->available_antennas_tx = 0x1;
			wiphy->available_antennas_rx = 0x1;
            break;
		case VERSION_8814A:
			wiphy->available_antennas_tx = 0x7;
			wiphy->available_antennas_rx = 0xf;
			break;
		default:
			wiphy->available_antennas_tx = 0x3;
			wiphy->available_antennas_rx = 0x3;
	}

	 /*The device supports roaming feature in the 	firmware*/    	//_eric_cfg ?? support these features ??
	wiphy->flags |= WIPHY_FLAG_SUPPORTS_FW_ROAM ;    
     /*device integrates AP SME*/
	wiphy->flags |= WIPHY_FLAG_HAVE_AP_SME ;        
	wiphy->flags |= WIPHY_FLAG_HAS_REMAIN_ON_CHANNEL ;/*cfg p2p ; p2p must use it*/	
    if(band_2gig){
		wiphy->interface_modes = BIT(NL80211_IFTYPE_AP)|
							BIT(NL80211_IFTYPE_STATION) | //_eric_cfg station mandatory ??
							BIT(NL80211_IFTYPE_ADHOC) |   //wrt-adhoc
#if defined(P2P_SUPPORT)
							BIT(NL80211_IFTYPE_P2P_CLIENT)|
							BIT(NL80211_IFTYPE_P2P_GO)|
							BIT(NL80211_IFTYPE_P2P_DEVICE)|
#endif
							BIT(NL80211_IFTYPE_MONITOR); 	
    }else{
        wiphy->interface_modes = BIT(NL80211_IFTYPE_AP)|
                                BIT(NL80211_IFTYPE_STATION) | //_eric_cfg station mandatory ??
                                BIT(NL80211_IFTYPE_ADHOC)|
								BIT(NL80211_IFTYPE_MONITOR);
    }                           

	wiphy->max_acl_mac_addrs = NUM_STAT;
	//printk("set_wiphy_dev +++ \n");
	set_wiphy_dev(wiphy, rtk->dev); //return wiphy->dev.parent;
	//printk("set_wiphy_dev --- \n");

#if defined(EN_EFUSE) && !defined(CUSTOMIZE_FLASH_EFUSE)
#ifdef CONFIG_WLAN_HAL_8881A
	if (GET_CHIP_VER(priv) != VERSION_8881A)
#endif		
	{
		memset(efusemac,0,ETH_ALEN);
		extern void read_efuse_mac_address(struct rtl8192cd_priv * priv,char * efusemac);
		read_efuse_mac_address(priv,efusemac);
		if( memcmp(efusemac,zero,ETH_ALEN) && !IS_MCAST(efusemac))
			memcpy(rtk->root_mac,efusemac,ETH_ALEN);
	}
#endif
	memcpy(wiphy->perm_addr, rtk->root_mac, ETH_ALEN); 
	memcpy(priv->pmib->dot11Bss.bssid, wiphy->perm_addr, ETH_ALEN);

	/*
	 * Even if the fw has HT support, advertise HT cap only when
	 * the firmware has support to override RSN capability, otherwise
	 * 4-way handshake would fail.
	 */
	if(band_2gig)
	{
		wiphy->bands[IEEE80211_BAND_2GHZ] = &realtek_band_2ghz;

		switch(GET_CHIP_VER(priv)){
			case VERSION_8814A:
				wiphy->bands[IEEE80211_BAND_2GHZ]->ht_cap.mcs.rx_highest = cpu_to_le16(450);
				wiphy->bands[IEEE80211_BAND_2GHZ]->ht_cap.mcs.rx_mask[0] = 0xff;
				wiphy->bands[IEEE80211_BAND_2GHZ]->ht_cap.mcs.rx_mask[1] = 0xff;
				wiphy->bands[IEEE80211_BAND_2GHZ]->ht_cap.mcs.rx_mask[2] = 0xff;
				break;
			case VERSION_8188E:
			case VERSION_8188C:
				wiphy->bands[IEEE80211_BAND_2GHZ]->ht_cap.mcs.rx_highest = cpu_to_le16(150);
				wiphy->bands[IEEE80211_BAND_2GHZ]->ht_cap.mcs.rx_mask[0] = 0xff;
				wiphy->bands[IEEE80211_BAND_2GHZ]->ht_cap.mcs.rx_mask[1] = 0;
				wiphy->bands[IEEE80211_BAND_2GHZ]->ht_cap.mcs.rx_mask[2] = 0;
				break;
		}
		wiphy->iface_combinations = &rtk_2g_comb;
		wiphy->n_iface_combinations = 1;
	}	

	if(band_5gig)
	{
		wiphy->bands[IEEE80211_BAND_5GHZ] = &realtek_band_5ghz;
		switch(GET_CHIP_VER(priv)){
			case VERSION_8814A:
				wiphy->bands[IEEE80211_BAND_5GHZ]->ht_cap.mcs.rx_highest = cpu_to_le16(450);
				wiphy->bands[IEEE80211_BAND_5GHZ]->ht_cap.mcs.rx_mask[0] = 0xff;
				wiphy->bands[IEEE80211_BAND_5GHZ]->ht_cap.mcs.rx_mask[1] = 0xff;
				wiphy->bands[IEEE80211_BAND_5GHZ]->ht_cap.mcs.rx_mask[2] = 0xff;
				break;
			case VERSION_8881A:
				wiphy->bands[IEEE80211_BAND_5GHZ]->ht_cap.mcs.rx_highest = cpu_to_le16(150);
				wiphy->bands[IEEE80211_BAND_5GHZ]->ht_cap.mcs.rx_mask[0] = 0xff;
				wiphy->bands[IEEE80211_BAND_5GHZ]->ht_cap.mcs.rx_mask[1] = 0;
				wiphy->bands[IEEE80211_BAND_5GHZ]->ht_cap.mcs.rx_mask[2] = 0;
				break;
		}
		wiphy->iface_combinations = &rtk_5g_comb;
		wiphy->n_iface_combinations = 1;
#ifdef RTK_AC_SUPPORT
		{
			extern void input_value_32(unsigned long *p, unsigned char start, unsigned char end, unsigned int value);
			unsigned int value=0;

			wiphy->bands[IEEE80211_BAND_5GHZ]->vht_cap.vht_supported = true;
	        input_value_32(&wiphy->bands[IEEE80211_BAND_5GHZ]->vht_cap.cap, MAX_MPDU_LENGTH_S, MAX_MPDU_LENGTH_E, 0x1);
			//Support 80MHz bandwidth only
			//0 - not support 160/80+80; 1 - support 160; 2 - support 80+80 
			if(priv->pshare->CurrentChannelBW == HT_CHANNEL_WIDTH_AC_160)
				value = 1;
			else
				value = 0;
			input_value_32(&wiphy->bands[IEEE80211_BAND_5GHZ]->vht_cap.cap, CHL_WIDTH_S, CHL_WIDTH_E, value);
			
			if((GET_CHIP_VER(priv) == VERSION_8814A) && (priv->pshare->is_40m_bw == HT_CHANNEL_WIDTH_AC_80))
				input_value_32(&wiphy->bands[IEEE80211_BAND_5GHZ]->vht_cap.cap, SHORT_GI80M_S, SHORT_GI80M_E, (priv->pmib->dot11nConfigEntry.dot11nShortGIfor40M ? 1 : 0));

			input_value_32(&wiphy->bands[IEEE80211_BAND_5GHZ]->vht_cap.cap, SHORT_GI160M_S, SHORT_GI160M_E, 0);
			input_value_32(&wiphy->bands[IEEE80211_BAND_5GHZ]->vht_cap.cap, RX_STBC_S, RX_STBC_E, 1);
			if ((get_rf_mimo_mode(priv) == MIMO_2T2R) || (get_rf_mimo_mode(priv) == MIMO_3T3R)) //eric_8814 
				input_value_32(&wiphy->bands[IEEE80211_BAND_5GHZ]->vht_cap.cap, TX_STBC_S, TX_STBC_E, 1);
            else
                input_value_32(&wiphy->bands[IEEE80211_BAND_5GHZ]->vht_cap.cap, TX_STBC_S, TX_STBC_E, 0);

#if defined(CONFIG_WLAN_HAL_8881A)
			if(GET_CHIP_VER(priv) == VERSION_8881A)
				input_value_32(&wiphy->bands[IEEE80211_BAND_5GHZ]->vht_cap.cap, RX_LDPC_S, RX_LDPC_E, 0);
			else
#endif
				input_value_32(&wiphy->bands[IEEE80211_BAND_5GHZ]->vht_cap.cap, RX_LDPC_S, RX_LDPC_E, 1);

#if (BEAMFORMING_SUPPORT == 1)
			input_value_32(&wiphy->bands[IEEE80211_BAND_5GHZ]->vht_cap.cap, SU_BFER_S, SU_BFER_E, 1);
			input_value_32(&wiphy->bands[IEEE80211_BAND_5GHZ]->vht_cap.cap, SU_BFEE_S, SU_BFEE_E, 1);
#else
			input_value_32(&wiphy->bands[IEEE80211_BAND_5GHZ]->vht_cap.cap, SU_BFER_S, SU_BFER_E, 0);
			input_value_32(&wiphy->bands[IEEE80211_BAND_5GHZ]->vht_cap.cap, SU_BFEE_S, SU_BFEE_E, 0);
#endif
#ifdef CONFIG_WLAN_HAL_8814AE
			if(priv->pshare->rf_ft_var.bf_sup_val != 0){        
				input_value_32(&wiphy->bands[IEEE80211_BAND_5GHZ]->vht_cap.cap, MAX_ANT_SUPP_S, MAX_ANT_SUPP_E, priv->pshare->rf_ft_var.bf_sup_val);
				input_value_32(&wiphy->bands[IEEE80211_BAND_5GHZ]->vht_cap.cap, SOUNDING_DIMENSIONS_S, SOUNDING_DIMENSIONS_E, priv->pshare->rf_ft_var.bf_sup_val);
			}else
#endif	
			{
#ifdef CONFIG_WLAN_HAL_8814AE
				if(GET_CHIP_VER(priv)==VERSION_8814A) {
					if(get_rf_mimo_mode(priv) == MIMO_4T4R) {       
						txbf_max_ant = 3;
					    txbf_sounding_dim = 3;
					} else if(get_rf_mimo_mode(priv) == MIMO_3T3R) {
						txbf_max_ant = 2;
					    txbf_sounding_dim = 3;
					} else if(get_rf_mimo_mode(priv) == MIMO_2T4R) {
						txbf_max_ant = 2;
					    txbf_sounding_dim = 1;
					} else  if(get_rf_mimo_mode(priv) == MIMO_2T2R) {
						txbf_max_ant = 2;
					    txbf_sounding_dim = 1;
					} else {
						txbf_max_ant = 1;
					    txbf_sounding_dim = 1;
					}
					input_value_32(&wiphy->bands[IEEE80211_BAND_5GHZ]->vht_cap.cap, MAX_ANT_SUPP_S, MAX_ANT_SUPP_E, txbf_max_ant);
					input_value_32(&wiphy->bands[IEEE80211_BAND_5GHZ]->vht_cap.cap, SOUNDING_DIMENSIONS_S, SOUNDING_DIMENSIONS_E, txbf_sounding_dim);
				} else
#endif
				{
					input_value_32(&wiphy->bands[IEEE80211_BAND_5GHZ]->vht_cap.cap, MAX_ANT_SUPP_S, MAX_ANT_SUPP_E, BEAMFORM_MAX_ANT_SUPP);
					input_value_32(&wiphy->bands[IEEE80211_BAND_5GHZ]->vht_cap.cap, SOUNDING_DIMENSIONS_S, SOUNDING_DIMENSIONS_E, BEAMFORM_SOUNDING_DIMENSIONS);
				}
			}

            input_value_32(&wiphy->bands[IEEE80211_BAND_5GHZ]->vht_cap.cap, HTC_VHT_S, HTC_VHT_E, 1);
            input_value_32(&wiphy->bands[IEEE80211_BAND_5GHZ]->vht_cap.cap, MAX_RXAMPDU_FACTOR_S, MAX_RXAMPDU_FACTOR_E, 7);
			if (get_rf_mimo_mode(priv) == MIMO_2T2R) {
				wiphy->bands[IEEE80211_BAND_5GHZ]->vht_cap.vht_mcs.rx_mcs_map = cpu_to_le16(0xfffa);
				wiphy->bands[IEEE80211_BAND_5GHZ]->vht_cap.vht_mcs.tx_mcs_map = cpu_to_le16(0xfffa);
				wiphy->bands[IEEE80211_BAND_5GHZ]->vht_cap.vht_mcs.rx_highest = cpu_to_le16(780);
				wiphy->bands[IEEE80211_BAND_5GHZ]->vht_cap.vht_mcs.tx_highest = cpu_to_le16(780);
			} else if(get_rf_mimo_mode(priv) == MIMO_3T3R || get_rf_mimo_mode(priv) == MIMO_4T4R) {
				wiphy->bands[IEEE80211_BAND_5GHZ]->vht_cap.vht_mcs.rx_mcs_map = cpu_to_le16(0xffea);
				wiphy->bands[IEEE80211_BAND_5GHZ]->vht_cap.vht_mcs.tx_mcs_map = cpu_to_le16(0xffea);
				wiphy->bands[IEEE80211_BAND_5GHZ]->vht_cap.vht_mcs.rx_highest = cpu_to_le16(1170);
				wiphy->bands[IEEE80211_BAND_5GHZ]->vht_cap.vht_mcs.tx_highest = cpu_to_le16(1170);
            } else {
				wiphy->bands[IEEE80211_BAND_5GHZ]->vht_cap.vht_mcs.rx_mcs_map = cpu_to_le16(0xfffe);
				wiphy->bands[IEEE80211_BAND_5GHZ]->vht_cap.vht_mcs.tx_mcs_map = cpu_to_le16(0xfffe);
				wiphy->bands[IEEE80211_BAND_5GHZ]->vht_cap.vht_mcs.rx_highest = cpu_to_le16(390);
				wiphy->bands[IEEE80211_BAND_5GHZ]->vht_cap.vht_mcs.tx_highest = cpu_to_le16(390);
			}
		}
#endif
	}	

	wiphy->cipher_suites = cipher_suites;
	wiphy->n_cipher_suites = ARRAY_SIZE(cipher_suites);
#if 0//def CONFIG_PM
	wiphy->wowlan.flags = WIPHY_WOWLAN_MAGIC_PKT |
			      WIPHY_WOWLAN_DISCONNECT |
			      WIPHY_WOWLAN_GTK_REKEY_FAILURE  |
			      WIPHY_WOWLAN_SUPPORTS_GTK_REKEY |
			      WIPHY_WOWLAN_EAP_IDENTITY_REQ   |
			      WIPHY_WOWLAN_4WAY_HANDSHAKE;
	wiphy->wowlan.n_patterns = WOW_MAX_FILTERS_PER_LIST;
	wiphy->wowlan.pattern_min_len = 1;
	wiphy->wowlan.pattern_max_len = WOW_PATTERN_SIZE;

	wiphy->max_sched_scan_ssids = MAX_PROBED_SSIDS;

	
	wiphy->features |= NL80211_FEATURE_INACTIVITY_TIMER;
	wiphy->probe_resp_offload =
		NL80211_PROBE_RESP_OFFLOAD_SUPPORT_WPS |
		NL80211_PROBE_RESP_OFFLOAD_SUPPORT_WPS2 |
		NL80211_PROBE_RESP_OFFLOAD_SUPPORT_P2P;
#endif
#ifdef CONFIG_OPENWRT_SDK
	wiphy->features |= NL80211_FEATURE_AP_MODE_CHAN_WIDTH_CHANGE;
#endif
	//printk("wiphy_register +++ \n");
	ret = wiphy_register(wiphy);
	//printk("wiphy_register --- \n");
	
	if (ret < 0) {
		printk("couldn't register wiphy device\n");
		return ret;
	}

	rtk->wiphy_registered = true;

	NLEXIT;
	return 0;
}

#endif //RTK_NL80211

