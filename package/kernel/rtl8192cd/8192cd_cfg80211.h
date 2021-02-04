/*
 * Copyright (c) 2011 Atheros Communications Inc.
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

#ifndef RTK_NL80211
#define RTK_NL80211
#endif

#ifdef RTK_NL80211
#include "./wifi.h"/*cfg p2p cfg p2p*/

#define DSSET_IE_LEN	1
#define HTCAP_IE_LEN	26
#define HTINFO_IE_LEN	22
#define MAX_PROBED_SSIDS 32

#define VIF_NUM				RTL8192CD_NUM_VWLAN //eric-vap
#define IF_NUM				(VIF_NUM+2)  //#vap + root + vxd
#define VIF_NAME_SIZE		10
/*cfg p2p cfg p2p*/
#define MAX_IE_LEN 			768

#define MAX_ASSOC_REQ_LEN 	512
#define MAX_ASSOC_RSP_LEN 	512


#define RATETAB_ENT(_rate, _rateid, _flags) {   \
	.bitrate    = (_rate),                  \
	.flags      = (_flags),                 \
	.hw_value   = (_rateid),                \
}


static struct ieee80211_rate realtek_rates[] = {
	RATETAB_ENT(10, 0x1, 0),
	RATETAB_ENT(20, 0x2, 0),
	RATETAB_ENT(55, 0x4, 0),
	RATETAB_ENT(110, 0x8, 0),
	RATETAB_ENT(60, 0x10, 0),
	RATETAB_ENT(90, 0x20, 0),
	RATETAB_ENT(120, 0x40, 0),
	RATETAB_ENT(180, 0x80, 0),
	RATETAB_ENT(240, 0x100, 0),
	RATETAB_ENT(360, 0x200, 0),
	RATETAB_ENT(480, 0x400, 0),
	RATETAB_ENT(540, 0x800, 0),
};


#define realtek_g_rates     (realtek_rates + 0)
#define realtek_g_rates_size    12

#define realtek_a_rates     (realtek_rates + 4)
#define realtek_a_rates_size    8

#define realtek_g_htcap (IEEE80211_HT_CAP_SUP_WIDTH_20_40 | \
			IEEE80211_HT_CAP_SGI_20		 | \
			IEEE80211_HT_CAP_SGI_40)

#define realtek_a_htcap (IEEE80211_HT_CAP_SUP_WIDTH_20_40 | \
			IEEE80211_HT_CAP_SGI_20		 | \
			IEEE80211_HT_CAP_SGI_40)

#define realtek_a_vhtcap (IEEE80211_VHT_CAP_SUPP_CHAN_WIDTH_80MHZ)


/* WMI_CONNECT_CMDID  */
enum network_type {
	INFRA_NETWORK = 0x01,
	ADHOC_NETWORK = 0x02,
	ADHOC_CREATOR = 0x04,
	AP_NETWORK = 0x10,
};

enum scan_abort_case {
	SCAN_ABORT_DEL_IFACE = 0, 
	SCAN_ABORT_START_AP,
};

#if 0/*cfg p2p cfg p2p*/
enum mgmt_type {
	MGMT_BEACON = 0,
	MGMT_PROBERSP = 1,
	MGMT_ASSOCRSP = 2,
	MGMT_ASSOCREQ = 3,
	MGMT_PROBEREQ = 4,	
};
#endif

static const u32 cipher_suites[] = {
	WLAN_CIPHER_SUITE_WEP40,
	WLAN_CIPHER_SUITE_WEP104,
	WLAN_CIPHER_SUITE_TKIP,
	WLAN_CIPHER_SUITE_CCMP,
	//CCKM_KRK_CIPHER_SUITE,
	//WLAN_CIPHER_SUITE_SMS4,
};

#define CHAN2G(_channel, _freq, _flags) {   \
	.band           = IEEE80211_BAND_2GHZ,  \
	.hw_value       = (_channel),           \
	.center_freq    = (_freq),              \
	.flags          = (_flags),             \
	.max_antenna_gain   = 0,                \
	.max_power      = 30,                   \
}

#define CHAN5G(_channel, _flags) {                  \
        .band           = IEEE80211_BAND_5GHZ,      \
        .hw_value       = (_channel),               \
        .center_freq    = 5000 + (5 * (_channel)),  \
        .flags          = (_flags),                 \
        .max_antenna_gain   = 0,                    \
        .max_power      = 30,                       \
} 
   

static struct ieee80211_channel realtek_2ghz_channels[] = {
	CHAN2G(1, 2412, IEEE80211_CHAN_NO_HT40MINUS),
	CHAN2G(2, 2417, IEEE80211_CHAN_NO_HT40MINUS),
	CHAN2G(3, 2422, IEEE80211_CHAN_NO_HT40MINUS),
	CHAN2G(4, 2427, IEEE80211_CHAN_NO_HT40MINUS),
	CHAN2G(5, 2432, 0),
	CHAN2G(6, 2437, 0),
	CHAN2G(7, 2442, 0),
	CHAN2G(8, 2447, IEEE80211_CHAN_NO_HT40PLUS),
	CHAN2G(9, 2452, IEEE80211_CHAN_NO_HT40PLUS),
	CHAN2G(10, 2457, IEEE80211_CHAN_NO_HT40PLUS),
	CHAN2G(11, 2462, IEEE80211_CHAN_NO_HT40PLUS),
	CHAN2G(12, 2467, IEEE80211_CHAN_NO_HT40PLUS),
	CHAN2G(13, 2472, IEEE80211_CHAN_NO_HT40PLUS),
	CHAN2G(14, 2484, IEEE80211_CHAN_NO_HT40PLUS | IEEE80211_CHAN_NO_HT40MINUS | IEEE80211_CHAN_NO_OFDM)
};

static struct ieee80211_channel realtek_5ghz_a_channels[] = {
	/* UNII-1 */
	CHAN5G(36, IEEE80211_CHAN_NO_HT40MINUS),
	CHAN5G(40, IEEE80211_CHAN_NO_HT40PLUS),
	CHAN5G(44, IEEE80211_CHAN_NO_HT40MINUS),
	CHAN5G(48, IEEE80211_CHAN_NO_HT40PLUS),
	/* UNII-2 */
	CHAN5G(52,
		 IEEE80211_CHAN_RADAR | IEEE80211_CHAN_NO_HT40MINUS),
	CHAN5G(56,
		 IEEE80211_CHAN_RADAR | IEEE80211_CHAN_NO_HT40PLUS),
	CHAN5G(60,
		 IEEE80211_CHAN_RADAR | IEEE80211_CHAN_NO_HT40MINUS),
	CHAN5G(64,
		 IEEE80211_CHAN_RADAR | IEEE80211_CHAN_NO_HT40PLUS),
	/* MID */
	CHAN5G(100,
		 IEEE80211_CHAN_RADAR | IEEE80211_CHAN_NO_HT40MINUS),
	CHAN5G(104,
		 IEEE80211_CHAN_RADAR | IEEE80211_CHAN_NO_HT40PLUS),
	CHAN5G(108,
		 IEEE80211_CHAN_RADAR | IEEE80211_CHAN_NO_HT40MINUS),
	CHAN5G(112,
		 IEEE80211_CHAN_RADAR | IEEE80211_CHAN_NO_HT40PLUS),
	CHAN5G(116,
		 IEEE80211_CHAN_RADAR | IEEE80211_CHAN_NO_HT40MINUS),
	CHAN5G(120,
		 IEEE80211_CHAN_RADAR | IEEE80211_CHAN_NO_HT40PLUS),
	CHAN5G(124,
		 IEEE80211_CHAN_RADAR | IEEE80211_CHAN_NO_HT40MINUS),
	CHAN5G(128,
		 IEEE80211_CHAN_RADAR | IEEE80211_CHAN_NO_HT40PLUS),
	CHAN5G(132,
		 IEEE80211_CHAN_RADAR | IEEE80211_CHAN_NO_HT40MINUS),
	CHAN5G(136,
		 IEEE80211_CHAN_RADAR | IEEE80211_CHAN_NO_HT40PLUS),
	CHAN5G(140,
		 IEEE80211_CHAN_RADAR | IEEE80211_CHAN_NO_HT40PLUS |
		 IEEE80211_CHAN_NO_HT40MINUS),
	/* UNII-3 */
	CHAN5G(149, IEEE80211_CHAN_NO_HT40MINUS),
	CHAN5G(153, IEEE80211_CHAN_NO_HT40PLUS),
	CHAN5G(157, IEEE80211_CHAN_NO_HT40MINUS),
	CHAN5G(161, IEEE80211_CHAN_NO_HT40PLUS),
	CHAN5G(165, IEEE80211_CHAN_NO_HT40PLUS | IEEE80211_CHAN_NO_HT40MINUS)
};

static struct ieee80211_supported_band realtek_band_2ghz = {
	.band = NL80211_BAND_2GHZ,
	.n_channels = ARRAY_SIZE(realtek_2ghz_channels),
	.channels = realtek_2ghz_channels,
	.n_bitrates = realtek_g_rates_size,
	.bitrates = realtek_g_rates,
	.ht_cap = {
				.cap = IEEE80211_HT_CAP_SUP_WIDTH_20_40 | IEEE80211_HT_CAP_SGI_20 | IEEE80211_HT_CAP_SGI_40,
				.ht_supported = true,
				.ampdu_factor = IEEE80211_HT_MAX_AMPDU_64K,
				.ampdu_density = 7,
				.mcs = {
						.rx_mask = {0xff, 0xff, 0, 0, 0, 0, 0, 0, 0, 0},
						.rx_highest = cpu_to_le16(300),
						.tx_params = IEEE80211_HT_MCS_TX_DEFINED
						}
    			}
};

static struct ieee80211_supported_band realtek_band_5ghz = {
	.band = NL80211_BAND_5GHZ,
	.n_channels = ARRAY_SIZE(realtek_5ghz_a_channels),
	.channels = realtek_5ghz_a_channels,
	.n_bitrates = realtek_a_rates_size,
	.bitrates = realtek_a_rates,
	.ht_cap = {
				.cap = IEEE80211_HT_CAP_SUP_WIDTH_20_40 | IEEE80211_HT_CAP_SGI_20 | IEEE80211_HT_CAP_SGI_40,
				.ht_supported = true,
				.ampdu_factor = IEEE80211_HT_MAX_AMPDU_64K,
				.ampdu_density = 7,
				.mcs = {
						.rx_mask = {0xff, 0xff, 0, 0, 0, 0, 0, 0, 0, 0},
						.rx_highest = cpu_to_le16(300),
						.tx_params = IEEE80211_HT_MCS_TX_DEFINED
						}
				}
};

struct rtk_clnt_info {
	struct wpa_ie_info	wpa_ie;
	struct rsn_ie_info	rsn_ie;
	unsigned char assoc_req[MAX_ASSOC_REQ_LEN];
	unsigned char assoc_req_len;
	unsigned char assoc_rsp[MAX_ASSOC_RSP_LEN];
	unsigned char assoc_rsp_len;
};

struct rtk_iface_info {
	unsigned char used;
	unsigned char ndev_name[32];
	struct rtl8192cd_priv *priv;
};

struct survey_info_t {
	unsigned int channel;
	unsigned int chbusytime;
	unsigned int rx_time;
	unsigned int tx_time;
	s8 noise;
};

struct rtknl {
	struct class *cl;
	struct device *dev;
	struct wiphy *wiphy;
	struct rtl8192cd_priv *priv;
	struct net_device *ndev_add;
	struct rtk_clnt_info clnt_info;
	unsigned char num_vif;
	int 		idx_vif;
	unsigned char num_vap;
	unsigned char num_vxd;
	unsigned int  vif_flag;
	unsigned char wiphy_registered;
	unsigned int  cipher;
	unsigned char ndev_name[VIF_NUM][VIF_NAME_SIZE];
	unsigned char ndev_name_vxd[VIF_NAME_SIZE];
	unsigned char root_ifname[VIF_NAME_SIZE];
	unsigned char root_mac[ETH_ALEN];
	unsigned char vap_mac[VIF_NUM][ETH_ALEN];
	struct rtl8192cd_priv *priv_root;
	struct rtl8192cd_priv *priv_vxd;
	struct rtk_iface_info rtk_iface[VIF_NUM+2];
	//for survey_dump
	struct survey_info_t survey_info[ARRAY_SIZE(realtek_5ghz_a_channels)];
	/* By brian, to support per channel statistic
	unsigned int chbusytime;
	unsigned int rx_time;
	unsigned int tx_time;
	*/
	//openwrt_psd
	unsigned int psd_chnl;
	unsigned int psd_bw;
	unsigned int psd_pts;
	unsigned int psd_fft_info[1040];
	//openwrt_tx_power_use
	unsigned int pwr_rate;
	int 		 pwr_cur;
	int			 pwr_set_dbm;
	unsigned char	keep_legacy;
};

#if defined(VAP_MAC_DRV_READ_FLASH)
int read_flash_hw_mac_vap( unsigned char *mac, int vap_idx); 
#endif
unsigned char is_WRT_scan_iface(unsigned char* if_name); //eric-vap
void realtek_cfg80211_inform_ss_result(struct rtl8192cd_priv *priv);
struct rtknl *realtek_cfg80211_create(void); 
int realtek_rtknl_init(struct rtknl *rtk);
int realtek_cfg80211_init(struct rtknl *rtk,struct rtl8192cd_priv *priv); 
int realtek_interface_add(struct rtl8192cd_priv *priv, struct rtknl *rtk, const char *name, 
								enum nl80211_iftype type, u8 fw_vif_idx, u8 nw_type);
int event_indicate_cfg80211(struct rtl8192cd_priv *priv, unsigned char *mac, int event, unsigned char *extra);
void close_vxd_vap(struct rtl8192cd_priv *priv_root);
int check_5M10M_config(struct rtl8192cd_priv *priv);
void rtk_remove_dev(struct rtknl *rtk,int idx);
void realtek_change_iftype(struct rtl8192cd_priv *priv ,enum nl80211_iftype type);


#endif /* RTK_NL80211 */
