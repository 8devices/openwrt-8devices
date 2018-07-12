/*
 *   API-compatible handling routines
 *
 *
 *
 *  Copyright (c) 2009 Realtek Semiconductor Corp.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
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

#if defined(WIFI_HAPD) || defined(RTK_NL80211)

#ifdef __LINUX_2_6__
#include <linux/initrd.h>
#include <linux/syscalls.h>
#endif

#include "./8192cd_debug.h"
#include "./8192cd_headers.h"

#include <linux/if_arp.h>
#include <linux/wireless.h>
#include <net/iw_handler.h>
#include <net80211/ieee80211.h>
#include <net80211/ieee80211_crypto.h>
#include <net80211/ieee80211_ioctl.h>

#include "./8192cd_net80211.h"


//#define HAPD_DEBUG

void void_printk(const char *fmt, ...)
{
	;
}

#ifdef HAPD_DEBUG
#define HAPD_MSG	printk
#else
#define HAPD_MSG	void_printk
#endif



const char* ether_sprintf(const u_int8_t *mac)
{
	static char etherbuf[18]; 	/* XXX */
	snprintf(etherbuf, sizeof(etherbuf), "%02x:%02x:%02x:%02x:%02x:%02x",
		mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
	return etherbuf;
}

static __inline__ void set_ttkeylen(struct Dot11EncryptKey *pEncryptKey, UINT8 len)
{
	pEncryptKey->dot11TTKeyLen = len;
}


static __inline__ void set_tmickeylen(struct Dot11EncryptKey *pEncryptKey, UINT8 len)
{
	pEncryptKey->dot11TMicKeyLen = len;
}

static __inline__ void set_tkip_key_for_wpas(struct Dot11EncryptKey *pEncryptKey, UINT8 *src)
{
	memcpy(pEncryptKey->dot11TTKey.skey, src, pEncryptKey->dot11TTKeyLen);

	memcpy(pEncryptKey->dot11TMicKey2.skey, src + 16, pEncryptKey->dot11TMicKeyLen);
	memcpy(pEncryptKey->dot11TMicKey1.skey, src + 24, pEncryptKey->dot11TMicKeyLen);

	memcpy(src + 16, pEncryptKey->dot11TMicKey2.skey, pEncryptKey->dot11TMicKeyLen);
	memcpy(src + 24, pEncryptKey->dot11TMicKey1.skey, pEncryptKey->dot11TMicKeyLen);

	pEncryptKey->dot11TXPN48.val48 = 0;
}

static __inline__ void set_tkip_key(struct Dot11EncryptKey *pEncryptKey, UINT8 *src)
{
	memcpy(pEncryptKey->dot11TTKey.skey, src, pEncryptKey->dot11TTKeyLen);

	memcpy(pEncryptKey->dot11TMicKey1.skey, src + 16, pEncryptKey->dot11TMicKeyLen);

	memcpy(pEncryptKey->dot11TMicKey2.skey, src + 24, pEncryptKey->dot11TMicKeyLen);

	pEncryptKey->dot11TXPN48.val48 = 0;
}


static __inline__ void set_aes_key(struct Dot11EncryptKey *pEncryptKey, UINT8 *src)
{
	memcpy(pEncryptKey->dot11TTKey.skey, src, pEncryptKey->dot11TTKeyLen);

	memcpy(pEncryptKey->dot11TMicKey1.skey, src, pEncryptKey->dot11TMicKeyLen);
}


static __inline__ void set_wep40_key(struct Dot11EncryptKey *pEncryptKey, UINT8 *src)
{
	memcpy(pEncryptKey->dot11TTKey.skey, src, pEncryptKey->dot11TTKeyLen);
}


static __inline__ void set_wep104_key(struct Dot11EncryptKey *pEncryptKey, UINT8 *src)
{
	memcpy(pEncryptKey->dot11TTKey.skey, src, pEncryptKey->dot11TTKeyLen);
}


static int HAPD_Process_Set_Port(struct net_device *dev, unsigned char *MACAddr, int PortStatus)
{
	struct stat_info	*pstat;
	struct rtl8192cd_priv *priv = GET_DEV_PRIV(dev);
	struct wifi_mib		*pmib = priv->pmib;

	HAPD_MSG("HAPD_Process_Set_Port +++ sta: %02X%02X%02X%02X%02X%02X Status %X\n",
		MACAddr[0],MACAddr[1],MACAddr[2],
		MACAddr[3],MACAddr[4],MACAddr[5],
		PortStatus);

	// if driver is not opened, return immediately, david
	if (!netif_running(priv->dev))
		return (-1);

	pstat = get_stainfo(priv, MACAddr);

	if ((pstat == NULL) || (!(pstat->state & WIFI_ASOC_STATE)))
		return (-1);

	if (PortStatus)
		pstat->ieee8021x_ctrlport = PortStatus;
	else
		pstat->ieee8021x_ctrlport = pmib->dot118021xAuthEntry.dot118021xDefaultPort;

	HAPD_MSG("HAPD_Process_Set_Port ---\n");

	return 0;
}

#if defined(WIFI_WPAS) || defined(RTK_NL80211)

int	rtl_wpas_join(struct rtl8192cd_priv *priv, int bss_num)
{
	char tmpbuf[33];

	if (!netif_running(priv->dev)) {
		printk("WiFi driver is NOT open!!\n");
		return -1;
	} else if (priv->ss_req_ongoing) {
		printk("Site Survey is not finished yet!!\n");
		return -1;
	}

#if defined(RTK_NL80211)
	memcpy((void *)&(priv->pmib->dot11Bss) ,
		(void *)&priv->site_survey->bss_target[bss_num] , sizeof(struct bss_desc));
#else
	memcpy((void *)&(priv->pmib->dot11Bss) ,
		(void *)&priv->site_survey->bss_backup[bss_num] , sizeof(struct bss_desc));
#endif

#ifdef WIFI_SIMPLE_CONFIG
	//_Eric if (priv->pmib->wscEntry.wsc_enable && (priv->pmib->dot11Bss.bsstype&WIFI_WPS)) 
	if (priv->pmib->wscEntry.wsc_enable)
	{
		//priv->pmib->dot11Bss.bsstype &= ~WIFI_WPS;
		priv->wps_issue_join_req = 1;
	}
	else
#endif
	{
		if (check_bss_encrypt(priv) == FAIL)
		{
			printk("Encryption mismatch!\n");
			return -1;
		}
	}

	if ((priv->pmib->dot11Bss.ssidlen == 0) || (priv->pmib->dot11Bss.ssid[0] == '\0')) 
	{
		printk("Error !! Join to a hidden AP!\n");
		return -1;
	}

#ifdef UNIVERSAL_REPEATER
	disable_vxd_ap(GET_VXD_PRIV(priv));
#endif

	memcpy(tmpbuf, priv->pmib->dot11Bss.ssid, priv->pmib->dot11Bss.ssidlen);
	tmpbuf[priv->pmib->dot11Bss.ssidlen] = '\0';

	NDEBUG3("[%s %d]going to join bss: [%s]\n",__FUNCTION__,__LINE__, tmpbuf);

	memcpy(SSID2SCAN, priv->pmib->dot11Bss.ssid, priv->pmib->dot11Bss.ssidlen);
	SSID2SCAN_LEN = priv->pmib->dot11Bss.ssidlen;

	SSID_LEN = SSID2SCAN_LEN;
	memcpy(SSID, SSID2SCAN, SSID_LEN);
	memset(BSSID, 0, MACADDRLEN);

#ifdef INCLUDE_WPA_PSK //_Eric ??
	//if (priv->pmib->dot1180211AuthEntry.dot11EnablePSK)
		//derivePSK(priv);
#endif
	priv->join_req_ongoing = 1;
	priv->authModeRetry = 0;

	//mod_timer(&priv->WPAS_timer, jiffies + 300);

#ifdef RTK_NL80211
	if(priv->pmib->dot11Bss.channel >=36)
	{
		if(GET_CHIP_VER(priv)==VERSION_8812E || GET_CHIP_VER(priv)==VERSION_8881A || GET_CHIP_VER(priv)==VERSION_8814A)
			priv->pmib->dot11BssType.net_work_type = WIRELESS_11A|WIRELESS_11N|WIRELESS_11AC;
		else
			priv->pmib->dot11BssType.net_work_type = WIRELESS_11A|WIRELESS_11N;
	}
	else
	{
	    #ifdef P2P_SUPPORT
        if(rtk_p2p_is_enabled(priv))
    		priv->pmib->dot11BssType.net_work_type = WIRELESS_11G|WIRELESS_11N;        // p2p mode no included 11B        
        else
        #endif
    		priv->pmib->dot11BssType.net_work_type = WIRELESS_11B|WIRELESS_11G|WIRELESS_11N;  

	}

	priv->pmib->dot11nConfigEntry.dot11nAMPDU = 1;
	priv->pmib->dot11nConfigEntry.dot11nShortGIfor20M = 1;
	priv->pmib->dot11nConfigEntry.dot11nShortGIfor40M = 1;

	extern int is_support_ac(struct rtl8192cd_priv *priv);
    
	if(is_support_ac(priv))
		priv->pmib->dot11nConfigEntry.dot11nShortGIfor80M = 1;

#ifdef CONFIG_WLAN_HAL_8814AE
	if(GET_CHIP_VER(priv) == VERSION_8814A)
		priv->pmib->dot11nConfigEntry.dot11nAMSDU=2;
#endif
#endif

	start_clnt_join(priv);

	return 0;
}

#endif


void rtl_net80311_authmode(struct net_device *dev, int authtype, int encmode, 
									int pskenable, int dot11802_1x)
{
	struct rtl8192cd_priv *priv = GET_DEV_PRIV(dev);

	HAPD_MSG("rtl_net80311_authmode +++\n");
	HAPD_MSG("authtype=%d, encmode=%d, pskenable=%d, dot11802_1x=%d \n", authtype, encmode, pskenable, dot11802_1x);

	priv->pmib->dot1180211AuthEntry.dot11AuthAlgrthm = authtype;
	priv->pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm = encmode;
	priv->pmib->dot1180211AuthEntry.dot11EnablePSK = pskenable;
	priv->pmib->dot118021xAuthEntry.dot118021xAlgrthm = dot11802_1x;	
}

int rtl_net80211_setparam(struct net_device *dev, struct iw_request_info *info, union iwreq_data *wrqu, char *extra)
{
	struct rtl8192cd_priv *priv = GET_DEV_PRIV(dev);

	int *i = (int *) extra;
	int param = wrqu->mode;		/* parameter id is 1st */
	int value = 0;		/* NB: most values are TYPE_INT */
	int ret = 0;

	int authtype = priv->pmib->dot1180211AuthEntry.dot11AuthAlgrthm; 
	int encmode = priv->pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm;
	int pskenable = priv->pmib->dot1180211AuthEntry.dot11EnablePSK;
	int dot11802_1x = priv->pmib->dot118021xAuthEntry.dot118021xAlgrthm;

	memcpy(&value, wrqu->name+sizeof(value), sizeof(value));

	HAPD_MSG("rtl_net80211_setparam +++\n");
	HAPD_MSG("rtl8192cd_net80211_ioctl, param = %d, value =%d\n", param, value);


	switch (param) {
		case IEEE80211_PARAM_TURBO:			/* turbo mode */
			break;
		case IEEE80211_PARAM_MODE:			/* phy mode (11a, 11b, etc.) */
			break;
		case IEEE80211_PARAM_AUTHMODE:		/* authentication mode */
			switch (value) {
			case IEEE80211_AUTH_NONE:
				{//Disable
					authtype = 0;
					encmode = 0;
					pskenable = 0;
					dot11802_1x = 0;
					rtl_net80311_authmode(dev, authtype, encmode, pskenable, dot11802_1x);
					break;
				}
			case IEEE80211_AUTH_OPEN:
				{//WEP Open
					authtype = 0;
					//WEP encmode depends on key length
					//encmode = 1;
					pskenable = 0;
					dot11802_1x = 0;
					rtl_net80311_authmode(dev, authtype, encmode, pskenable, dot11802_1x);
					break;
				}
			case IEEE80211_AUTH_SHARED:
				{//WEP Shared
					authtype = 1;
					//WEP encmode depends on key length
					//encmode = 1;
					pskenable = 0;
					dot11802_1x = 0;
					rtl_net80311_authmode(dev, authtype, encmode, pskenable, dot11802_1x);
					break;
				}
			case IEEE80211_AUTH_AUTO:
				{//WEP Auto
					authtype = 2;
					//WEP encmode depends on key length
					//encmode = 1;
					pskenable = 0;
					dot11802_1x = 0;
					rtl_net80311_authmode(dev, authtype, encmode, pskenable, dot11802_1x);
					break;
				}
			case IEEE80211_AUTH_WPA:
				{//WPA-PSK-AES
				 //_Eric ?? if there is no valid passphrase ??
				 	authtype = 2;
					encmode = 2;
					//_Eric pskmode (WPA 0,1,2) will be set by IEEE80211_PARAM_WPA
					//pskenable = 1;
					dot11802_1x = 1;
					rtl_net80311_authmode(dev, authtype, encmode, pskenable, dot11802_1x);
					break;
				}
			case IEEE80211_AUTH_8021X:
				{
					//8021x encmode depends on key length
					if((encmode == _WEP_40_PRIVACY_) || (encmode == _WEP_104_PRIVACY_))
						authtype = 2;
					else
						authtype = 0;

					pskenable = 0;
					dot11802_1x = 1;
					rtl_net80311_authmode(dev, authtype, encmode, pskenable, dot11802_1x);
					break;
				}
			default:
					return -EINVAL;
			}
		case IEEE80211_PARAM_PROTMODE:		/* 802.11g protection */
			break;
		case IEEE80211_PARAM_MCASTCIPHER:		/* multicast/default cipher */
			break;
		case IEEE80211_PARAM_MCASTKEYLEN:		/* multicast key length */
			break;
		case IEEE80211_PARAM_UCASTCIPHERS:		/* unicast cipher suites */
			/*	1. Only support AES & TKIP for WPA1&WPA2
			  	2. Set both wpa_cipher and wpa2_cipher, because IEEE80211_PARAM_WPA will clear unused cipher mode		*/
			HAPD_MSG("IEEE80211_PARAM_UCASTCIPHERS\n");

			if(value & (1<<IEEE80211_CIPHER_WEP))
				return -EINVAL;
			if(value & (1<<IEEE80211_CIPHER_AES_OCB))
				return -EINVAL;
			if(value & (1<<IEEE80211_CIPHER_CKIP))
				return -EINVAL;
			if(value & (1<<IEEE80211_CIPHER_NONE))
				return -EINVAL;

			if(value & (1<<IEEE80211_CIPHER_TKIP))
				{
					if(1)//((!(priv->pmib->dot11BssType.net_work_type & WIRELESS_11N)) || (OPMODE & WIFI_STATION_STATE))
						{
							priv->pmib->dot1180211AuthEntry.dot11WPACipher |= BIT(1);
							priv->pmib->dot1180211AuthEntry.dot11WPA2Cipher |= BIT(1);
						}
					else
						return -EINVAL;
				}
			else
				{
					priv->pmib->dot1180211AuthEntry.dot11WPACipher &= ~(BIT(1));
					priv->pmib->dot1180211AuthEntry.dot11WPA2Cipher &= ~(BIT(1));
				}
			

			if(value & (1<<IEEE80211_CIPHER_AES_CCM))
				{
					priv->pmib->dot1180211AuthEntry.dot11WPACipher |= BIT(3);
					priv->pmib->dot1180211AuthEntry.dot11WPA2Cipher |= BIT(3);			
				}
			else
				{
					priv->pmib->dot1180211AuthEntry.dot11WPACipher &= ~(BIT(3));
					priv->pmib->dot1180211AuthEntry.dot11WPA2Cipher &= ~(BIT(3));	
				}

			priv->pmib->dot118021xAuthEntry.dot118021xAlgrthm = 1;
			
			return 0; //_Eric if value > BIT(8) ?
				
		case IEEE80211_PARAM_UCASTCIPHER:		/* unicast cipher */
			break;
		case IEEE80211_PARAM_UCASTKEYLEN:		/* unicast key length */
			if(value == 5)
				priv->pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm = _WEP_40_PRIVACY_;
			else if(value == 13)
				priv->pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm = _WEP_104_PRIVACY_;
			else 
				priv->pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm = _NO_PRIVACY_;
			break;
		case IEEE80211_PARAM_WPA:			/* WPA mode (0,1,2) */
			HAPD_MSG("IEEE80211_PARAM_WPA\n");
			if((value & BIT(0)) && (value & BIT(1)))
				return 0;
			else if((value & BIT(0)))
				{//only WPA1, so clear mib of wpa2_cipher & wpa2_psk
					priv->pmib->dot1180211AuthEntry.dot11WPA2Cipher = 0;
					priv->pmib->dot1180211AuthEntry.dot11EnablePSK & ~(BIT(1));
					return 0;
				}
			else if((value & BIT(1)))
				{//only WPA2, so clear mib of wpa_cipher & wps2_psk
					priv->pmib->dot1180211AuthEntry.dot11WPACipher = 0;
					priv->pmib->dot1180211AuthEntry.dot11EnablePSK & ~(BIT(0));
					return 0;
				}
			
		case IEEE80211_PARAM_ROAMING:			/* roaming mode */
#ifdef WIFI_WPAS
			if(value == IEEE80211_ROAMING_DEVICE)
				priv->pmib->dot11StationConfigEntry.fastRoaming = 1;
			else if(value == IEEE80211_ROAMING_MANUAL)
				priv->pmib->dot11StationConfigEntry.fastRoaming = 0;
			else
				return -EINVAL;
#endif
			break;
		case IEEE80211_PARAM_PRIVACY:			/* privacy invoked */
			ret =0;	//It seems useless, just return status OK for hostapd.
			break; 
		case IEEE80211_PARAM_COUNTERMEASURES:		/* WPA/TKIP countermeasures */
			//_Eric ?? realtek do not have to support ??
			break;
		case IEEE80211_PARAM_DROPUNENCRYPTED:		/* discard unencrypted frames */
			break;
		case IEEE80211_PARAM_DRIVER_CAPS:		/* driver capabilities */
			break;
		case IEEE80211_PARAM_MACCMD:			/* MAC ACL operation */
			break;
		case IEEE80211_PARAM_WMM:			/* WMM mode (on, off) */
			break;
		case IEEE80211_PARAM_HIDESSID:		/* hide SSID mode (on, off) */
			break;
		case IEEE80211_PARAM_APBRIDGE:   	   	/* AP inter-sta bridging */
			break;
		case IEEE80211_PARAM_KEYMGTALGS:		/* key management algorithms */
			HAPD_MSG("IEEE80211_PARAM_KEYMGTALGS\n");
			if((0<=value) && (value<=3))
				{
					priv->pmib->dot1180211AuthEntry.dot11EnablePSK = value;
					break;
				}
			else
				return -EINVAL;
		case IEEE80211_PARAM_RSNCAPS:			/* RSN capabilities */
			break;
		case IEEE80211_PARAM_INACT:			/* station inactivity timeout */
			break;
		case IEEE80211_PARAM_INACT_AUTH:		/* station auth inact timeout */
			break;
		case IEEE80211_PARAM_INACT_INIT:		/* station init inact timeout */
			break;
		case IEEE80211_PARAM_ABOLT:			/* Atheros Adv. Capabilities */
			break;
		case IEEE80211_PARAM_DTIM_PERIOD:		/* DTIM period (beacons) */
			break;
		case IEEE80211_PARAM_BEACON_INTERVAL:		/* beacon interval (ms) */
			break;
		case IEEE80211_PARAM_DOTH:			/* 11.h is on/off */
			break;
		case IEEE80211_PARAM_PWRTARGET:		/* Current Channel Pwr Constraint */
			break;
		case IEEE80211_PARAM_GENREASSOC:		/* Generate a reassociation request */
			break;
		case IEEE80211_PARAM_COMPRESSION:		/* compression */
			break;
		case IEEE80211_PARAM_FF:			/* fast frames support  */
			break;
		case IEEE80211_PARAM_XR:			/* XR support */
			break;
		case IEEE80211_PARAM_BURST:			/* burst mode */
			break;
		case IEEE80211_PARAM_PUREG:			/* pure 11g (no 11b stations) */
			break;
		case IEEE80211_PARAM_AR:			/* AR support */
			break;
		case IEEE80211_PARAM_WDS:			/* Enable 4 address processing */
			break;
		case IEEE80211_PARAM_BGSCAN:			/* bg scanning (on, off) */
			break;
		case IEEE80211_PARAM_BGSCAN_IDLE:		/* bg scan idle threshold */
			break;
		case IEEE80211_PARAM_BGSCAN_INTERVAL:		/* bg scan interval */
			break;
		case IEEE80211_PARAM_MCAST_RATE:		/* Multicast Tx Rate */
			break;
		case IEEE80211_PARAM_COVERAGE_CLASS:		/* coverage class */
			break;
		case IEEE80211_PARAM_COUNTRY_IE:		/* enable country IE */
			break;
		case IEEE80211_PARAM_SCANVALID:		/* scan cache valid threshold */
			break;
		case IEEE80211_PARAM_ROAM_RSSI_11A:		/* rssi threshold in 11a */
			break;
		case IEEE80211_PARAM_ROAM_RSSI_11B:		/* rssi threshold in 11b */
			break;
		case IEEE80211_PARAM_ROAM_RSSI_11G:		/* rssi threshold in 11g */
			break;
		case IEEE80211_PARAM_ROAM_RATE_11A:		/* tx rate threshold in 11a */
			break;
		case IEEE80211_PARAM_ROAM_RATE_11B:		/* tx rate threshold in 11b */
			break;
		case IEEE80211_PARAM_ROAM_RATE_11G:		/* tx rate threshold in 11g */
			break;
		case IEEE80211_PARAM_UAPSDINFO:		/* value for qos info field */
			break;
		case IEEE80211_PARAM_SLEEP:			/* force sleep/wake */
			break;
		case IEEE80211_PARAM_QOSNULL:			/* force sleep/wake */
			break;
		case IEEE80211_PARAM_PSPOLL:			/* force ps-poll generation (sta only) */
			break;
		case IEEE80211_PARAM_EOSPDROP:		/* force uapsd EOSP drop (ap only) */
			break;
		case IEEE80211_PARAM_MARKDFS:			/* mark a dfs interference channel when found */
			break;
		case IEEE80211_PARAM_REGCLASS:		/* enable regclass ids in country IE */
			break;
		case IEEE80211_PARAM_DROPUNENC_EAPOL:		/* drop unencrypted eapol frames */
			break;
	 	case IEEE80211_PARAM_SHPREAMBLE:		/* Short Preamble */
			break;
			
	}

	HAPD_MSG("rtl_net80211_setparam ---\n");
	return ret;

}

/*
Management frame type to which application IE is added 
enum {
	IEEE80211_APPIE_FRAME_BEACON		= 0,
	IEEE80211_APPIE_FRAME_PROBE_REQ		= 1,
	IEEE80211_APPIE_FRAME_PROBE_RESP	= 2,
	IEEE80211_APPIE_FRAME_ASSOC_REQ		= 3,
	IEEE80211_APPIE_FRAME_ASSOC_RESP	= 4,
	IEEE80211_APPIE_NUM_OF_FRAME		= 5
};
*/

int rtl_net80211_setappiebuf(struct net_device *dev, struct iw_request_info *info, union iwreq_data *wrqu, char *extra)
{
	struct rtl8192cd_priv *priv = GET_DEV_PRIV(dev);
	struct ieee80211req_getset_appiebuf * ie = (struct ieee80211req_getset_appiebuf *)wrqu->data.pointer;
	int ret = 0;

	HAPD_MSG("rtl_net80211_setappiebuf +++\n");
	HAPD_MSG("frametype =%d, iebuflen=%d \n", ie->app_frmtype, ie->app_buflen);

	//_Eric ???? hostapd has no interface to set wsc_enable.

	if((ie->app_buflen == 0) && (ie->app_frmtype != IEEE80211_APPIE_FRAME_ASSOC_RESP))
		priv->pmib->wscEntry.wsc_enable = 0;
	else if(ie->app_buflen <= 256)
		priv->pmib->wscEntry.wsc_enable = 2; //Work as AP
	else 
		return -EINVAL;

	if (ie->app_frmtype == IEEE80211_APPIE_FRAME_BEACON) {
		HAPD_MSG("WSC: set beacon IE\n");
		priv->pmib->wscEntry.beacon_ielen = ie->app_buflen;
		memcpy((void *)priv->pmib->wscEntry.beacon_ie, ie->app_buf, ie->app_buflen);
	}
	else if (ie->app_frmtype == IEEE80211_APPIE_FRAME_PROBE_RESP) {
		HAPD_MSG("WSC: set probe response IE\n");
		priv->pmib->wscEntry.probe_rsp_ielen = ie->app_buflen;
		memcpy((void *)priv->pmib->wscEntry.probe_rsp_ie, ie->app_buf, ie->app_buflen);
	}
	else if (ie->app_frmtype == IEEE80211_APPIE_FRAME_ASSOC_RESP) {
		HAPD_MSG("WSC: set association response IE\n");
		priv->pmib->wscEntry.assoc_ielen = ie->app_buflen;
		memcpy((void *)priv->pmib->wscEntry.assoc_ie, ie->app_buf, ie->app_buflen);
	}
#ifdef WIFI_WPAS
	else if (ie->app_frmtype == IEEE80211_APPIE_FRAME_PROBE_REQ) {
		HAPD_MSG("WSC: set probe request IE\n");
		priv->pmib->wscEntry.probe_req_ielen = ie->app_buflen;
		memcpy((void *)priv->pmib->wscEntry.probe_req_ie, ie->app_buf, ie->app_buflen);
	}
#endif
	else
		return -EINVAL;


	HAPD_MSG("rtl_net80211_setappiebuf ---\n");
	return ret;

}


#ifdef WIFI_WPAS

int rtl_net80211_setoptie(struct net_device *dev, struct iw_request_info *info, union iwreq_data *wrqu, char *extra)
{
	struct rtl8192cd_priv *priv = GET_DEV_PRIV(dev);
	WPAS_ASSOCIATION_INFO Assoc_Info;

//Check if WPS IE
	UINT8 WSC_IE_OUI[4] = {0x00, 0x50, 0xf2, 0x04};
	int x;
	unsigned char *ie = (unsigned char *)wrqu->data.pointer;

	if( ie[0] == _WPS_IE_)
		if (!memcmp(ie+2, WSC_IE_OUI, 4)) 
			{
				priv->pmib->wscEntry.assoc_ielen = wrqu->data.length;
				memcpy((void *)priv->pmib->wscEntry.assoc_ie, wrqu->data.pointer, wrqu->data.length);
				priv->pmib->wscEntry.wsc_enable = 1;

				return 0;
			}

	priv->pmib->wscEntry.wsc_enable = 0;

	memset(priv->pmib->dot11RsnIE.rsnie, 0x0, 128);
	memcpy(priv->pmib->dot11RsnIE.rsnie, wrqu->data.pointer, wrqu->data.length);
	priv->pmib->dot11RsnIE.rsnielen = wrqu->data.length;
	
	memset((void *)&Assoc_Info, 0, sizeof(struct _WPAS_ASSOCIATION_INFO));
	Assoc_Info.ReqIELen = priv->pmib->dot11RsnIE.rsnie[1]+ 2;
	memcpy(Assoc_Info.ReqIE, priv->pmib->dot11RsnIE.rsnie, Assoc_Info.ReqIELen);
	//event_indicate_wpas(priv, NULL, WPAS_ASSOC_INFO, (UINT8 *)&Assoc_Info);

	return 0;
	
}

#endif

int rtl_net80211_setmlme(struct net_device *dev, struct iw_request_info *info, union iwreq_data *wrqu, char *extra)
{
	struct rtl8192cd_priv *priv = GET_DEV_PRIV(dev);
	struct ieee80211req_mlme *mlme = (struct ieee80211req_key *)wrqu->data.pointer;
	struct stat_info *pstat = get_stainfo(priv, mlme->im_macaddr);
			
	int ret = 0;

	HAPD_MSG("rtl_net80211_setmlme +++\n");
	HAPD_MSG("auth_state =%d, mac = 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x\n"
			, mlme->im_op, mlme->im_macaddr[0], mlme->im_macaddr[1], mlme->im_macaddr[2], 
				mlme->im_macaddr[3], mlme->im_macaddr[4], mlme->im_macaddr[5]);

	//_Eric ????

	if(mlme->im_op == IEEE80211_MLME_AUTHORIZE)
		{
				
			if (pstat == NULL){
				pstat = alloc_stainfo(priv, mlme->im_macaddr, -1);
				if (pstat == NULL) {
					printk("Exceed the upper limit of supported clients...\n");
					return -1;
				}
				init_stainfo(priv, pstat);
				pstat->state |= (WIFI_AUTH_SUCCESS | WIFI_ASOC_STATE);
				pstat->expire_to = priv->assoc_to;
				asoc_list_add(priv, pstat);
			}

			HAPD_Process_Set_Port(dev, mlme->im_macaddr ,DOT11_PortStatus_Authorized);

		}
	else if(mlme->im_op == IEEE80211_MLME_UNAUTHORIZE)
		{	
			if (pstat == NULL)
			return -EINVAL;

			//pstat->state |= WIFI_AUTH_NULL;

			HAPD_Process_Set_Port(dev, mlme->im_macaddr ,DOT11_PortStatus_Unauthorized);

		}
	else if(mlme->im_op == IEEE80211_MLME_DISASSOC)
		{	
			if (pstat == NULL)
			return -EINVAL;

#ifdef WIFI_WPAS
						
			if(OPMODE & WIFI_STATION_STATE)
			{
				HAPD_MSG("An AP is disconnected by WPAS \n");
				issue_disassoc(priv, mlme->im_macaddr, _RSON_UNSPECIFIED_);
			
				if (pstat->expire_to > 0)
				{
					cnt_assoc_num(priv, pstat, DECREASE, (char *)__FUNCTION__);
					check_sta_characteristic(priv, pstat, DECREASE);
				}
			
				free_stainfo(priv, pstat);
				memset(&priv->pmib->dot11Bss, 0, sizeof(struct bss_desc));
				memset(priv->pmib->dot11StationConfigEntry.dot11Bssid, 0, MACADDRLEN);
				memset(priv->pmib->dot11StationConfigEntry.dot11DesiredSSID, 0, sizeof(priv->pmib->dot11StationConfigEntry.dot11DesiredSSID));
				priv->pmib->dot11StationConfigEntry.dot11DesiredSSIDLen = 0;
				memset(priv->pmib->dot11StationConfigEntry.dot11DefaultSSID, 0, sizeof(priv->pmib->dot11StationConfigEntry.dot11DesiredSSID));
				priv->pmib->dot11StationConfigEntry.dot11DefaultSSIDLen = 0;
				memset(priv->pmib->dot11StationConfigEntry.dot11SSIDtoScan, 0, sizeof(priv->pmib->dot11StationConfigEntry.dot11SSIDtoScan));
				priv->pmib->dot11StationConfigEntry.dot11SSIDtoScanLen = 0;
				memset(priv->pmib->dot11StationConfigEntry.dot11DesiredBssid, 0, 6);

				priv->wpas_manual_assoc = 1; //_Eric ??  when to let driver auto-connect ??

				priv->join_res = STATE_Sta_No_Bss;
				//pstat->state &= (~WIFI_ASOC_STATE);
				return ret; 
			}
					
#endif

			if (asoc_list_del(priv, pstat))
			{
				if (pstat->expire_to > 0)
				{
					cnt_assoc_num(priv, pstat, DECREASE, (char *)__FUNCTION__);
					check_sta_characteristic(priv, pstat, DECREASE);
				}
			}

			// Need change state back to autehnticated
			release_stainfo(priv, pstat);
			init_stainfo(priv, pstat);
			pstat->state |= WIFI_AUTH_SUCCESS;
			pstat->expire_to = priv->assoc_to;
			auth_list_add(priv, pstat);
		}
	else if(mlme->im_op == IEEE80211_MLME_DEAUTH)
		{
			unsigned char	MULTICAST_ADD[6]={0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};

			if(!memcmp(mlme->im_macaddr, MULTICAST_ADD, 6)) //_Eric ??How to free all stainfo??
				return 0;
			
			if (pstat == NULL)
			return -EINVAL;
			
			if (asoc_list_del(priv, pstat))
			{
				if (pstat->expire_to > 0)
				{
					cnt_assoc_num(priv, pstat, DECREASE, (char *)__FUNCTION__);
					check_sta_characteristic(priv, pstat, DECREASE);
				}
			}
			
			//release_stainfo(priv, pstat);
			free_stainfo(priv, pstat);

		}
#ifdef WIFI_WPAS
	else if(mlme->im_op == IEEE80211_MLME_ASSOC)
		{
			int ix = 0, found = 0;

			if((OPMODE & WIFI_STATION_STATE) == 0)
				{
					HAPD_MSG("NOT in Client Mode, can NOT Associate !!!\n");
					return -1;
				}
			
			for(ix = 0 ; ix < priv->site_survey->count_backup ; ix++) //_Eric ?? will bss_backup be cleaned?? -> Not found in  codes
			{	
				if(!memcmp(priv->site_survey->bss_backup[ix].bssid , mlme->im_macaddr, 6))
				{
					found = 1;
					break;
				}
			}

			if(found == 0)
			{	
				printk("BSSID NOT Found !!\n");
				return -EINVAL;
			}
			else
				ret = rtl_wpas_join(priv, ix);

			if(ret != 0)
				printk("rtl_wpas_join Failed !!\n");
		
		}
#endif
	else
		{
			HAPD_MSG("unknown auth_state !!!\n");
			return -EINVAL;
		}
		
	HAPD_MSG("rtl_net80211_setmlme ---\n");
	return ret;


}


int rtl_net80211_setkey(struct net_device *dev, struct iw_request_info *info, union iwreq_data *wrqu, char *extra)
{
	struct rtl8192cd_priv *priv = GET_DEV_PRIV(dev);
	struct ieee80211req_key *wk = (struct ieee80211req_key *)wrqu->data.pointer;
	struct wifi_mib	*pmib = priv->pmib;
	struct Dot11EncryptKey	*pEncryptKey = NULL;
	struct stat_info	*pstat = NULL;
	unsigned char	MULTICAST_ADD[6]={0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};
#ifdef WIFI_WPAS
	unsigned char	GROUP_ADD[6]={0x0,0x0,0x0,0x0,0x0,0x0};
#endif
	int cipher =0;
	int retVal = 0;
	int group_key = 0;

	HAPD_MSG("rtl_net80211_setkey +++\n");


	HAPD_MSG("keyid = %d, mac = 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x\n"
			, wk->ik_keyix, wk->ik_macaddr[0], wk->ik_macaddr[1], wk->ik_macaddr[2], 
				wk->ik_macaddr[3], wk->ik_macaddr[4], wk->ik_macaddr[5]);
	HAPD_MSG("type = 0x%x, flags = 0x%x, keylen = 0x%x \n"
			, wk->ik_type, wk->ik_flags, wk->ik_keylen);


	//check if the interface is down
	if (!netif_running(priv->dev))
	{	
		if(wk->ik_type == IEEE80211_CIPHER_WEP)
		{
			HAPD_MSG("set WEP Key in driver DOWN\n");
			memcpy(&priv->pmib->dot11DefaultKeysTable.keytype[wk->ik_keyix].skey[0], wk->ik_keydata, wk->ik_keylen);
			return 0;
		}

		HAPD_MSG("\nFail: interface not opened\n");
		return 0;
	}

	if(!memcmp(wk->ik_macaddr, MULTICAST_ADD, 6))
		group_key = 1;

#ifdef WIFI_WPAS //_Eric ?? if oxff not group addr in wpas ??
	if(OPMODE & WIFI_STATION_STATE)
		if(!memcmp(wk->ik_macaddr, GROUP_ADD, 6))
			group_key = 1;
#endif

	if(wk->ik_type == IEEE80211_CIPHER_WEP)
		{

#ifdef WIFI_WPAS
		   if(OPMODE & WIFI_STATION_STATE)
		   	if(priv->pmib->dot118021xAuthEntry.dot118021xAlgrthm == 0)
		   	{
		   		memcpy(&priv->pmib->dot11DefaultKeysTable.keytype[wk->ik_keyix].skey[0], wk->ik_keydata, wk->ik_keylen);
				return 0;
		   	}
#endif
#ifdef RTK_NL80211
			HAPD_MSG("set WEP Key for NL80211\n");
			memcpy(&priv->pmib->dot11DefaultKeysTable.keytype[wk->ik_keyix].skey[0], wk->ik_keydata, wk->ik_keylen);
#endif
		   if(priv->pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm == _WEP_40_PRIVACY_)
		   	cipher = (DOT11_ENC_WEP40);
		   else if(priv->pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm == _WEP_104_PRIVACY_)
		   	cipher = (DOT11_ENC_WEP104);
		   else
		   	return -EINVAL;
		}
	else if(wk->ik_type == IEEE80211_CIPHER_TKIP)
		cipher = (DOT11_ENC_TKIP);
	else if(wk->ik_type == IEEE80211_CIPHER_AES_CCM)
		cipher = (DOT11_ENC_CCMP);
	else
		{
			HAPD_MSG("unknown encAlg !!!\n");
			return -EINVAL;
		}

	//_Eric ?? if call DOT11_Process_Set_Key
	//CamAddOneEntry(priv, wk->ik_macaddr, wk->ik_keyix, cipher, 0, wk->ik_keydata);

	if(group_key)
	{
		int set_gkey_to_cam = 0;
		HAPD_MSG("set group key !!\n");

#ifdef UNIVERSAL_REPEATER
		if (IS_VXD_INTERFACE(priv))
			set_gkey_to_cam = 0;
		else {
			if (IS_ROOT_INTERFACE(priv)) {
				if (IS_DRV_OPEN(GET_VXD_PRIV(priv)))
					set_gkey_to_cam = 0;
			}
		}
#endif

#ifdef MBSSID
		if (GET_ROOT(priv)->pmib->miscEntry.vap_enable)
		{
			// No matter root or vap, don't set key to cam if vap is enabled.
			set_gkey_to_cam = 0;
		}
#endif

#ifdef CONFIG_RTK_MESH
		//modify by Joule for SECURITY
		if (dev == priv->mesh_dev)
		{
			pmib->dot11sKeysTable.dot11Privacy = cipher;
			pEncryptKey = &pmib->dot11sKeysTable.dot11EncryptKey;
			pmib->dot11sKeysTable.keyid = (UINT)wk->ik_keyix;
		}
		else
#endif
		{
			pmib->dot11GroupKeysTable.dot11Privacy = cipher;
			pmib->dot11GroupKeysTable.keyid = (UINT)wk->ik_keyix;

			if (wk->ik_keyix == GKEY_ID_SECOND)
				pEncryptKey = &pmib->dot11GroupKeysTable.dot11EncryptKey2;				
			else
				pEncryptKey = &pmib->dot11GroupKeysTable.dot11EncryptKey;
		}

		switch(cipher)
		{
		case DOT11_ENC_TKIP:
			set_ttkeylen(pEncryptKey, 16);
			set_tmickeylen(pEncryptKey, 8);
#ifdef RTK_NL80211
			if(OPMODE & WIFI_STATION_STATE)
				set_tkip_key_for_wpas(pEncryptKey, wk->ik_keydata);
			else
#endif
			set_tkip_key(pEncryptKey, wk->ik_keydata);

			HAPD_MSG("going to set TKIP group key! id %X\n", (UINT)wk->ik_keyix);
			if (!SWCRYPTO) {
				if (set_gkey_to_cam)
				{
					retVal = CamDeleteOneEntry(priv, MULTICAST_ADD, wk->ik_keyix, 0);
					if (retVal) {
						priv->pshare->CamEntryOccupied--;
						pmib->dot11GroupKeysTable.keyInCam = FALSE;
					}
					retVal = CamAddOneEntry(priv, MULTICAST_ADD, wk->ik_keyix, DOT11_ENC_TKIP<<2, 0, wk->ik_keydata);
					if (retVal) {
						priv->pshare->CamEntryOccupied++;
						pmib->dot11GroupKeysTable.keyInCam = TRUE;
					}
				}
			}
			break;

		case DOT11_ENC_WEP40:
			set_ttkeylen(pEncryptKey, 5);
			set_tmickeylen(pEncryptKey, 0);
			set_wep40_key(pEncryptKey, wk->ik_keydata);

			HAPD_MSG("going to set WEP40 group key!\n");
			if (!SWCRYPTO) {
				if (set_gkey_to_cam)
				{
					retVal = CamDeleteOneEntry(priv, MULTICAST_ADD, wk->ik_keyix, 0);
					if (retVal) {
						priv->pshare->CamEntryOccupied--;
						pmib->dot11GroupKeysTable.keyInCam = FALSE;
					}
					retVal = CamAddOneEntry(priv, MULTICAST_ADD, wk->ik_keyix, DOT11_ENC_WEP40<<2, 0, wk->ik_keydata);
					if (retVal) {
						priv->pshare->CamEntryOccupied++;
						pmib->dot11GroupKeysTable.keyInCam = TRUE;
					}
				}
			}
			break;

		case DOT11_ENC_WEP104:
			set_ttkeylen(pEncryptKey, 13);
			set_tmickeylen(pEncryptKey, 0);
			set_wep104_key(pEncryptKey, wk->ik_keydata);

			HAPD_MSG("going to set WEP104 group key!\n");
			if (!SWCRYPTO) {
				if (set_gkey_to_cam)
				{
					retVal = CamDeleteOneEntry(priv, MULTICAST_ADD, wk->ik_keyix, 0);
					if (retVal) {
						priv->pshare->CamEntryOccupied--;
						pmib->dot11GroupKeysTable.keyInCam = FALSE;
					}
					retVal = CamAddOneEntry(priv, MULTICAST_ADD, wk->ik_keyix, DOT11_ENC_WEP104<<2, 0, wk->ik_keydata);
					if (retVal) {
						priv->pshare->CamEntryOccupied++;
						pmib->dot11GroupKeysTable.keyInCam = TRUE;
					}
				}
			}
			break;

		case DOT11_ENC_CCMP:
			set_ttkeylen(pEncryptKey, 16);
			set_tmickeylen(pEncryptKey, 16);
			set_aes_key(pEncryptKey, wk->ik_keydata);

			HAPD_MSG("going to set CCMP-AES group key!\n");
#ifdef CONFIG_RTK_MESH
			if (dev == priv->mesh_dev)
				pmib->dot11sKeysTable.keyInCam = TRUE;		// keyInCam means key in driver
			else
#endif
			if (!SWCRYPTO) {
				if (set_gkey_to_cam)
				{
					retVal = CamDeleteOneEntry(priv, MULTICAST_ADD, wk->ik_keyix, 0);
					if (retVal) {
						priv->pshare->CamEntryOccupied--;
						pmib->dot11GroupKeysTable.keyInCam = FALSE;
					}
					retVal = CamAddOneEntry(priv, MULTICAST_ADD, wk->ik_keyix, DOT11_ENC_CCMP<<2, 0, wk->ik_keydata);
					if (retVal) {
						priv->pshare->CamEntryOccupied++;
						pmib->dot11GroupKeysTable.keyInCam = TRUE;
					}
				}
			}
			break;

		case DOT11_ENC_NONE:
		default:
			HAPD_MSG("No group encryption key is set!\n");
			set_ttkeylen(pEncryptKey, 0);
			set_tmickeylen(pEncryptKey, 0);
			break;
		}
	}
	else
	{
		pstat = get_stainfo(priv, wk->ik_macaddr);
		if (pstat == NULL) {
			DEBUG_ERR("Set key failed, invalid mac address: %02x%02x%02x%02x%02x%02x\n",
				wk->ik_macaddr[0], wk->ik_macaddr[1], wk->ik_macaddr[2], wk->ik_macaddr[3],
				wk->ik_macaddr[4], wk->ik_macaddr[5]);
			return (-1);
		}

		pstat->dot11KeyMapping.dot11Privacy = cipher;
		pEncryptKey = &pstat->dot11KeyMapping.dot11EncryptKey;
		pstat->keyid = wk->ik_keyix;

#if defined(__DRAYTEK_OS__) && defined(WDS)
		if (pstat->state & WIFI_WDS)
			priv->pmib->dot11WdsInfo.wdsPrivacy = cipher;
#endif

#ifdef RTK_NL80211 //eric-ath
		//printk(" +++ pstat->state=0x%x wps_join=%d \n", pstat->state, (pstat->state & WIFI_WPS_JOIN));

		if(OPMODE & WIFI_STATION_STATE)
		if(pstat->state & WIFI_WPS_JOIN)
		{
			//printk("REMOVE WIFI_WPS_JOIN State !!\n");
			pstat->state &= (~(WIFI_WPS_JOIN));
		}
#endif

		switch(cipher)
		{
		case DOT11_ENC_TKIP:
			set_ttkeylen(pEncryptKey, 16);
			set_tmickeylen(pEncryptKey, 8);
			
#ifdef RTK_NL80211
			if(OPMODE & WIFI_STATION_STATE)
			set_tkip_key_for_wpas(pEncryptKey, wk->ik_keydata);
			else
#endif
			set_tkip_key(pEncryptKey, wk->ik_keydata);

			HAPD_MSG("going to set TKIP Unicast key for sta %02X%02X%02X%02X%02X%02X, id=%d\n",
				wk->ik_macaddr[0], wk->ik_macaddr[1], wk->ik_macaddr[2],
				wk->ik_macaddr[3], wk->ik_macaddr[4], wk->ik_macaddr[5], pstat->keyid);
			if (!SWCRYPTO) {
				retVal = CamDeleteOneEntry(priv, wk->ik_macaddr, wk->ik_keyix, 0);
				if (retVal) {
					priv->pshare->CamEntryOccupied--;
					if (pstat)	pstat->dot11KeyMapping.keyInCam = FALSE;
				}
				retVal = CamAddOneEntry(priv, wk->ik_macaddr, wk->ik_keyix, DOT11_ENC_TKIP<<2, 0, wk->ik_keydata);
				if (retVal) {
					priv->pshare->CamEntryOccupied++;
					if (pstat)	pstat->dot11KeyMapping.keyInCam = TRUE;
				}
				else {
					if (pstat->aggre_mthd != AGGRE_MTHD_NONE)
						pstat->aggre_mthd = AGGRE_MTHD_NONE;
				}
			}
			break;

		case DOT11_ENC_WEP40:
			set_ttkeylen(pEncryptKey, 5);
			set_tmickeylen(pEncryptKey, 0);
			set_wep40_key(pEncryptKey, wk->ik_keydata);

			HAPD_MSG("going to set WEP40 unicast key for sta %02X%02X%02X%02X%02X%02X, id=%d\n",
				wk->ik_macaddr[0], wk->ik_macaddr[1], wk->ik_macaddr[2],
				wk->ik_macaddr[3], wk->ik_macaddr[4], wk->ik_macaddr[5], pstat->keyid);
			if (!SWCRYPTO) {
				retVal = CamDeleteOneEntry(priv, wk->ik_macaddr, wk->ik_keyix, 0);
				if (retVal) {
					priv->pshare->CamEntryOccupied--;
					if (pstat)	pstat->dot11KeyMapping.keyInCam = FALSE;
				}
				retVal = CamAddOneEntry(priv, wk->ik_macaddr, wk->ik_keyix, DOT11_ENC_WEP40<<2, 0, wk->ik_keydata);
				if (retVal) {
					priv->pshare->CamEntryOccupied++;
					if (pstat)	pstat->dot11KeyMapping.keyInCam = TRUE;
				}
				else {
					if (pstat->aggre_mthd != AGGRE_MTHD_NONE)
						pstat->aggre_mthd = AGGRE_MTHD_NONE;
				}
			}
			break;

		case DOT11_ENC_WEP104:
			set_ttkeylen(pEncryptKey, 13);
			set_tmickeylen(pEncryptKey, 0);
			set_wep104_key(pEncryptKey, wk->ik_keydata);

			HAPD_MSG("going to set WEP104 unicast key for sta %02X%02X%02X%02X%02X%02X, id=%d\n",
				wk->ik_macaddr[0], wk->ik_macaddr[1], wk->ik_macaddr[2],
				wk->ik_macaddr[3], wk->ik_macaddr[4], wk->ik_macaddr[5], pstat->keyid);
			if (!SWCRYPTO) {
				retVal = CamDeleteOneEntry(priv, wk->ik_macaddr, wk->ik_keyix, 0);
				if (retVal) {
					priv->pshare->CamEntryOccupied--;
					if (pstat)	pstat->dot11KeyMapping.keyInCam = FALSE;
				}
				retVal = CamAddOneEntry(priv, wk->ik_macaddr, wk->ik_keyix, DOT11_ENC_WEP104<<2, 0, wk->ik_keydata);
				if (retVal) {
					priv->pshare->CamEntryOccupied++;
					if (pstat)	pstat->dot11KeyMapping.keyInCam = TRUE;
				}
				else {
					if (pstat->aggre_mthd != AGGRE_MTHD_NONE)
						pstat->aggre_mthd = AGGRE_MTHD_NONE;
				}
			}
			break;

		case DOT11_ENC_CCMP:
			set_ttkeylen(pEncryptKey, 16);
			set_tmickeylen(pEncryptKey, 16);
			set_aes_key(pEncryptKey, wk->ik_keydata);

			HAPD_MSG("going to set CCMP-AES unicast key for sta %02X%02X%02X%02X%02X%02X, id=%d\n",
				wk->ik_macaddr[0], wk->ik_macaddr[1], wk->ik_macaddr[2],
				wk->ik_macaddr[3], wk->ik_macaddr[4], wk->ik_macaddr[5], pstat->keyid);
			if (!SWCRYPTO) {
				retVal = CamDeleteOneEntry(priv, wk->ik_macaddr, wk->ik_keyix, 0);
				if (retVal) {
					priv->pshare->CamEntryOccupied--;
					if (pstat)	pstat->dot11KeyMapping.keyInCam = FALSE;
				}
				retVal = CamAddOneEntry(priv, wk->ik_macaddr, wk->ik_keyix, DOT11_ENC_CCMP<<2, 0, wk->ik_keydata);
				if (retVal) {
					HAPD_MSG("CamAddOneEntry of CCMP OK\n");
					priv->pshare->CamEntryOccupied++;
					if (pstat)	pstat->dot11KeyMapping.keyInCam = TRUE;
					assign_aggre_mthod(priv, pstat);
				}
				else {
					HAPD_MSG("CamAddOneEntry of CCMP FAIL\n");
					if (pstat->aggre_mthd != AGGRE_MTHD_NONE)
						pstat->aggre_mthd = AGGRE_MTHD_NONE;
				}
			}
			break;

		case DOT11_ENC_NONE:
		default:
			DEBUG_ERR("No pairewise encryption key is set!\n");
			set_ttkeylen(pEncryptKey, 0);
			set_tmickeylen(pEncryptKey, 0);
			break;
		}
	}


	HAPD_MSG("rtl_net80211_setkey ---\n");
	
	return 0;



}

int rtl_net80211_delkey(struct net_device *dev, struct iw_request_info *info, union iwreq_data *wrqu, char *extra)
{
	struct rtl8192cd_priv *priv = GET_DEV_PRIV(dev);

#ifdef RTK_NL80211
	struct ieee80211req_del_key *wk = (struct ieee80211req_del_key *)wrqu->data.pointer;
#else
	struct ieee80211req_del_key *wk = (struct ieee80211req_del_key *)wrqu->name;
#endif
	struct stat_info	*pstat = NULL;
	struct wifi_mib 	*pmib = priv->pmib;

	unsigned char MULTICAST_ADD[6]={0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};
	int ret = 0;

	//check if the interface is down
	if (!netif_running(priv->dev))
	{
		HAPD_MSG("\nFail: interface not opened\n");
		return 0;
	}

	HAPD_MSG("rtl_net80211_delkey +++ \n");
	HAPD_MSG("keyid = %d, mac = 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x\n"
			, wk->idk_keyix, wk->idk_macaddr[0], wk->idk_macaddr[1], wk->idk_macaddr[2], 
				wk->idk_macaddr[3], wk->idk_macaddr[4], wk->idk_macaddr[5]);

	if(!memcmp(wk->idk_macaddr, "\x00\x00\x00\x00\x00\x00", 6))
		{
			HAPD_MSG("reset ALL key !!!!!\n");
			CamResetAllEntry(priv);
		}

	if (!memcmp(wk->idk_macaddr, MULTICAST_ADD, 6))
	{
		pmib->dot11GroupKeysTable.dot11EncryptKey.dot11TTKeyLen = 0;
		pmib->dot11GroupKeysTable.dot11EncryptKey.dot11TMicKeyLen = 0;

		HAPD_MSG("Delete Group Key\n");
		if (CamDeleteOneEntry(priv, MULTICAST_ADD, 1, 0))
			priv->pshare->CamEntryOccupied--;
		#if defined(CONFIG_RTL_HW_WAPI_SUPPORT)
		if (CamDeleteOneEntry(priv, MULTICAST_ADD, 1, 0))
			priv->pshare->CamEntryOccupied--;
		#endif

	}
	else
	{
		pstat = get_stainfo(priv, (UINT8 *)wk->idk_macaddr);
		if (pstat == NULL)
			return (-1);

		pstat->dot11KeyMapping.dot11EncryptKey.dot11TTKeyLen = 0;
		pstat->dot11KeyMapping.dot11EncryptKey.dot11TMicKeyLen = 0;

		HAPD_MSG("Delete Unicast Key\n");
		if (pstat->dot11KeyMapping.keyInCam == TRUE) {
			if (CamDeleteOneEntry(priv, (unsigned char *)wk->idk_macaddr, 0, 0)) {
				priv->pshare->CamEntryOccupied--;
				if (pstat)	pstat->dot11KeyMapping.keyInCam = FALSE;
			}
			#if defined(CONFIG_RTL_HW_WAPI_SUPPORT)
			if (CamDeleteOneEntry(priv, (unsigned char *)wk->idk_macaddr, 0, 0)) {
				priv->pshare->CamEntryOccupied--;
				if (pstat)	pstat->dot11KeyMapping.keyInCam = FALSE;
			}
			#endif
		}
	}

	HAPD_MSG("rtl_net80211_delkey --- \n");

	return ret;


}

int rtl_net80211_getwpaie(struct net_device *dev, struct iw_request_info *info, union iwreq_data *wrqu, char *extra)
{
	struct rtl8192cd_priv *priv = GET_DEV_PRIV(dev);
	struct ieee80211req_wpaie *ie = (struct ieee80211req_wpaie *)wrqu->data.pointer;
	struct stat_info *pstat = get_stainfo(priv, ie->wpa_macaddr);
	int ret = 0;
	HAPD_MSG("rtl_net80211_getwpaie +++ \n");
	HAPD_MSG("mac =  0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x\n",
			ie->wpa_macaddr[0], ie->wpa_macaddr[1], ie->wpa_macaddr[2],
			ie->wpa_macaddr[3], ie->wpa_macaddr[4], ie->wpa_macaddr[5]);	

	if(pstat == NULL)
		return -EINVAL;
	
#ifndef HAPD_DRV_PSK_WPS
	HAPD_MSG("RSNEnabled = %d\n"
			"wpa_ie = 0x%02x 0x%02x 0x%02x \n"
			"wps_ie = 0x%02x 0x%02x 0x%02x \n" 
			,pstat->wpa_sta_info->RSNEnabled,
			pstat->wpa_ie[0], pstat->wpa_ie[1], pstat->wpa_ie[2], 
			pstat->wps_ie[0], pstat->wps_ie[1], pstat->wps_ie[2]
			);
#else
	HAPD_MSG("RSNEnabled = %d\n"
			"wpa_ie = 0x%02x 0x%02x 0x%02x \n"
			,pstat->wpa_sta_info->RSNEnabled,
			pstat->wpa_ie[0], pstat->wpa_ie[1], pstat->wpa_ie[2]
			);
#endif

	if(pstat->wpa_sta_info->RSNEnabled & BIT(0))
		memcpy(ie->wpa_ie, pstat->wpa_ie, pstat->wpa_ie[1]+2);
	else if(pstat->wpa_sta_info->RSNEnabled & BIT(1))
		memcpy(ie->rsn_ie, pstat->wpa_ie, pstat->wpa_ie[1]+2);

#ifndef HAPD_DRV_PSK_WPS
	if((priv->pmib->wscEntry.wsc_enable & 2) && (pstat->wps_ie[1]))
		memcpy(ie->wps_ie, pstat->wps_ie, pstat->wps_ie[1]+2);
#endif

	HAPD_MSG("rtl_net80211_getwpaie --- \n");
	return ret;
	
}


#if	((defined(WIFI_HAPD) || defined(RTK_NL80211)) && defined(WDS)) && !defined(HAPD_DRV_PSK_WPS)
int rtl_net80211_wdsaddmac(struct net_device *dev, struct iw_request_info *info, union iwreq_data *wrqu, char *extra)
{
	struct rtl8192cd_priv *priv = GET_DEV_PRIV(dev);
	struct rtk_wds_config *wds = (struct hapd_wds_info *)wrqu->data.pointer;
	int ret = 0;
	int num = 0;
	
	HAPD_MSG("rtl_net80211_wdsaddmac +++ \n");	

	if(wds->wdsEnabled == 0)
		goto bad;
	if(wds->wdsNum > NUM_WDS)
		goto bad;
	if( (wds->wdsPrivacy != _NO_PRIVACY_) && (wds->wdsPrivacy != _WEP_40_PRIVACY_) &&
		(wds->wdsPrivacy != _TKIP_PRIVACY_) && (wds->wdsPrivacy != _CCMP_PRIVACY_) &&
		(wds->wdsPrivacy != _WEP_104_PRIVACY_))
		goto bad;
	
	if((wds->wdsPrivacy == _WEP_40_PRIVACY_) && (wds->wdsWepKeyLen != 5))
		goto bad;
	if((wds->wdsPrivacy == _WEP_104_PRIVACY_) && (wds->wdsWepKeyLen != 13))
		goto bad;
	
	priv->pmib->dot11WdsInfo.wdsEnabled = 1;
	priv->pmib->dot11WdsInfo.wdsNum = wds->wdsNum;
		
	for(num=0 ; num < wds->wdsNum; num++)
		memcpy(priv->pmib->dot11WdsInfo.entry[num].macAddr, wds->macAddr[num], MACADDRLEN);

	priv->pmib->dot11WdsInfo.wdsPrivacy = wds->wdsPrivacy;

	if((wds->wdsPrivacy == _WEP_40_PRIVACY_)||(wds->wdsPrivacy == _WEP_104_PRIVACY_))
			memcpy(priv->pmib->dot11WdsInfo.wdsWepKey, wds->wdsWepKey, wds->wdsWepKeyLen);
	else if((wds->wdsPrivacy == _TKIP_PRIVACY_)||(wds->wdsPrivacy == _CCMP_PRIVACY_))
		hapd_set_wdskey(dev, wds->wdsPskPassPhrase, wds->ssid, wds->wdsNum);
	
	
	HAPD_MSG("rtl_net80211_wdsaddmac --- \n");
	return 0;

bad:
	priv->pmib->dot11WdsInfo.wdsEnabled = 0;
	return -EINVAL;
	

}

int rtl_net80211_wdsdelmac(struct net_device *dev, struct iw_request_info *info, union iwreq_data *wrqu, char *extra)
{
	struct rtl8192cd_priv *priv = GET_DEV_PRIV(dev);
	struct rtk_wds_config *wds = (struct rtk_wds_config *)wrqu->data.pointer;
	HAPD_MSG("rtl_net80211_wdsdelmac +++ \n");

	priv->pmib->dot11WdsInfo.wdsEnabled = 0;
	priv->pmib->dot11WdsInfo.wdsNum = 0;

	HAPD_MSG("rtl_net80211_wdsdelmac --- \n");
	return 0;
}
#endif


int rtl_hapd_config(struct net_device *dev, struct iw_request_info *info, union iwreq_data *wrqu, char *extra)
{
	struct rtl8192cd_priv *priv = GET_DEV_PRIV(dev);
	struct rtk_hapd_config *config = (struct rtk_hapd_config *)wrqu->data.pointer;
	int size = 0;

	HAPD_MSG("rtl_hapd_config +++\n");

	priv->pmib->dot11BssType.net_work_type    					= config->band;
	priv->pmib->dot11RFEntry.dot11channel	  					= config->channel;
	priv->pmib->dot11StationConfigEntry.dot11BeaconPeriod	  	= config->bcnint; 		  
	priv->pmib->dot11StationConfigEntry.dot11DTIMPeriod 	  	= config->dtimperiod; 

	if(config->stanum <= NUM_STAT)
	priv->pmib->dot11StationConfigEntry.supportedStaNum 	  	= config->stanum; 
	else
	{
		HAPD_MSG("Invalid Station Number!!!\n");
		return -1;
	}
	
	priv->pmib->dot11OperationEntry.dot11RTSThreshold		  	= config->rtsthres;		  
	priv->pmib->dot11OperationEntry.dot11FragmentationThreshold = config->fragthres;	
	priv->pmib->dot11StationConfigEntry.dot11SupportedRates   	= config->oprates;		  
	priv->pmib->dot11StationConfigEntry.dot11BasicRates 	  	= config->basicrates; 
	priv->pmib->dot11RFEntry.shortpreamble	  					= config->preamble;	
	priv->pmib->dot11StationConfigEntry.dot11AclMode	 		= config->aclmode;		  
	priv->pmib->dot11StationConfigEntry.dot11AclNum 	  		= config->aclnum;
	
	size = sizeof(priv->pmib->dot11StationConfigEntry.dot11AclAddr);
	memcpy(priv->pmib->dot11StationConfigEntry.dot11AclAddr, config->acladdr, size);
	
	priv->pmib->dot11OperationEntry.hiddenAP			  		= config->hiddenAP;	
#ifdef WIFI_WMM
	priv->pmib->dot11QosEntry.dot11QosEnable			  		= config->qos_enable;
#endif
	priv->pmib->dot11OperationEntry.expiretime					= config->expired_time * 100;	// 10ms unit vs 1s unit
	priv->pmib->dot11OperationEntry.block_relay					= config->block_relay;
	priv->pmib->dot11nConfigEntry.dot11nShortGIfor20M 			= config->shortGI20M; 		  
	priv->pmib->dot11nConfigEntry.dot11nShortGIfor40M 			= config->shortGI40M; 


//Above are for Hostapd owned configurations
//=====================================================
//Below are for RTK private configurations

	size = sizeof(priv->pmib->dot11RFEntry.pwrlevelCCK_A);
	memcpy(priv->pmib->dot11RFEntry.pwrlevelCCK_A, config->pwrlevelCCK_A, size);

	size = sizeof(priv->pmib->dot11RFEntry.pwrlevelCCK_B);
	memcpy(priv->pmib->dot11RFEntry.pwrlevelCCK_B, config->pwrlevelCCK_B, size);

	size = sizeof(priv->pmib->dot11RFEntry.pwrlevelHT40_1S_A);
	memcpy(priv->pmib->dot11RFEntry.pwrlevelHT40_1S_A, config->pwrlevelHT40_1S_A, size);

	size = sizeof(priv->pmib->dot11RFEntry.pwrlevelHT40_1S_B);
	memcpy(priv->pmib->dot11RFEntry.pwrlevelHT40_1S_B, config->pwrlevelHT40_1S_A, size);

	size = sizeof(priv->pmib->dot11RFEntry.pwrdiffHT40_2S);
	memcpy(priv->pmib->dot11RFEntry.pwrdiffHT40_2S, config->pwrdiffHT40_2S, size);

	size = sizeof(priv->pmib->dot11RFEntry.pwrdiffHT20);
	memcpy(priv->pmib->dot11RFEntry.pwrdiffHT20, config->pwrdiffHT20, size);

	size = sizeof(priv->pmib->dot11RFEntry.pwrdiffOFDM);
	memcpy(priv->pmib->dot11RFEntry.pwrdiffOFDM, config->pwrdiffOFDM, size);
		  
	priv->pmib->dot11RFEntry.ther		  		= config->ther;	
	
#ifdef CONFIG_RTL_92D_SUPPORT           
	priv->pmib->dot11RFEntry.phyBandSelect	  	= config->phyBandSelect;		  
#endif   

	priv->pmib->dot11StationConfigEntry.dot11swcrypto	  	= config->swcrypto;		      
	priv->pmib->dot11StationConfigEntry.dot11RegDomain		= config->regdomain;		  
	priv->pmib->dot11StationConfigEntry.autoRate		   	= config->autorate;	  
	priv->pmib->dot11StationConfigEntry.fixedTxRate			= config->fixrate;
	priv->pmib->dot11StationConfigEntry.protectionDisabled	= config->disable_protection; 	  
	priv->pmib->dot11StationConfigEntry.olbcDetectDisabled	= config->disable_olbc;		  
	priv->pmib->dot11StationConfigEntry.legacySTADeny	  	= config->deny_legacy;		  	     
	priv->pmib->dot11OperationEntry.opmode				  	= config->opmode; 		  	  																			  		 		  																																																		                  																			 
	priv->pmib->dot11nConfigEntry.dot11nUse40M		  		= config->use40M; 		  
	priv->pmib->dot11nConfigEntry.dot11n2ndChOffset   		= config->_2ndchoffset;		    
	priv->pmib->dot11nConfigEntry.dot11nAMPDU	  			= config->ampdu;			                             
	priv->pmib->dot11OperationEntry.guest_access			= config->guest_access;
																							
	priv->pmib->dot11RFEntry.macPhyMode 					= config->macPhyMode;
																							
#ifdef WIFI_11N_2040_COEXIST            
	priv->pmib->dot11nConfigEntry.dot11nCoexist = config->coexist;		  
#endif    
	 
#ifdef MBSSID                           
	priv->pmib->miscEntry.vap_enable 			= config->vap_enable;		  
#endif                            
																							
	priv->pshare->rf_ft_var.rssi_dump	  		= config->rssi_dump;			  
																										  
#ifdef MP_TEST                          
	priv->pshare->rf_ft_var.mp_specific  		= config->mp_specific;		  
#endif                                  
																							
#ifdef HIGH_POWER_EXT_PA                
	priv->pshare->rf_ft_var.use_ext_pa 			= config->use_ext_pa; 		  
#endif    

	HAPD_MSG("rtl_hapd_config ---\n");
	return 0;
}



#ifdef WIFI_WPAS

int rtl_wpas_config_2G(struct rtl8192cd_priv *priv)
{
	HAPD_MSG("wpas config wlan 2.4G\n");
	
	priv->pmib->dot11BssType.net_work_type					= 1 + 2 + 8;
	priv->pmib->dot11RFEntry.dot11channel					= 11;
	priv->pmib->dot11nConfigEntry.dot11nShortGIfor20M		= 1;		  
	priv->pmib->dot11nConfigEntry.dot11nShortGIfor40M		= 1;
	priv->pmib->dot11StationConfigEntry.autoRate			= 1;																																																																																																						 
	priv->pmib->dot11nConfigEntry.dot11nUse40M				= 1;				
	priv->pmib->dot11nConfigEntry.dot11nAMPDU				= 1;
	//priv->pmib->dot11RFEntry.macPhyMode 					= DUALMAC_DUALPHY;
	priv->pmib->dot11nConfigEntry.dot11n2ndChOffset 		= HT_2NDCH_OFFSET_BELOW;
	priv->pmib->dot11StationConfigEntry.dot11SupportedRates = 0xfff;		  
	priv->pmib->dot11StationConfigEntry.dot11BasicRates 	= 0xf; 
	priv->pmib->dot11OperationEntry.wifi_specific			= 2; //_Eric ??

	priv->pmib->dot11OperationEntry.dot11FragmentationThreshold = 2346;
	
#ifdef CONFIG_RTL_92D_SUPPORT           
	priv->pmib->dot11RFEntry.phyBandSelect					= PHY_BAND_2G;		  
#endif   
	
#ifdef WIFI_WMM
	priv->pmib->dot11QosEntry.dot11QosEnable				= 1;
#endif

}

int rtl_wpas_config_5G(struct rtl8192cd_priv *priv)
{
	printk("wpas config wlan 5G\n");
				
	priv->pmib->dot11BssType.net_work_type					= 4 + 8;
	priv->pmib->dot11RFEntry.dot11channel					= 44;
	priv->pmib->dot11nConfigEntry.dot11nShortGIfor20M		= 1;		  
	priv->pmib->dot11nConfigEntry.dot11nShortGIfor40M		= 1;
	priv->pmib->dot11StationConfigEntry.autoRate			= 1;																																																																																																						 
	priv->pmib->dot11nConfigEntry.dot11nUse40M				= 1;				
	priv->pmib->dot11nConfigEntry.dot11nAMPDU				= 1;
	//priv->pmib->dot11RFEntry.macPhyMode 					= DUALMAC_DUALPHY; //_Eric ?? How to judge ??
	priv->pmib->dot11nConfigEntry.dot11n2ndChOffset 		= HT_2NDCH_OFFSET_ABOVE;
	priv->pmib->dot11StationConfigEntry.dot11SupportedRates = 0xff0;		  
	priv->pmib->dot11StationConfigEntry.dot11BasicRates 	= 0xff0; 
	priv->pmib->dot11OperationEntry.wifi_specific			= 2;

	priv->pmib->dot11OperationEntry.dot11FragmentationThreshold = 2346;
	
#ifdef CONFIG_RTL_92D_SUPPORT           
	priv->pmib->dot11RFEntry.phyBandSelect					= PHY_BAND_5G;		  
#endif   
	
#ifdef WIFI_WMM
	priv->pmib->dot11QosEntry.dot11QosEnable				= 1;
#endif

}


int rtl_wpas_config(struct rtl8192cd_priv *priv, unsigned char bandmode, unsigned char phymode)
{
	HAPD_MSG("wpas config interface: %s\n", priv->dev->name);

	priv->pmib->dot11OperationEntry.opmode = WIFI_STATION_STATE;

	if(bandmode == SINGLEMAC_SINGLEPHY)
		{
			priv->pmib->dot11RFEntry.macPhyMode = SINGLEMAC_SINGLEPHY;

			if(phymode == PHY_BAND_5G)
				rtl_wpas_config_5G(priv);
			else if(phymode == PHY_BAND_2G)
				rtl_wpas_config_2G(priv);
			else
				return -1;
			
		}
#if defined(CUSTOMIZE_WLAN_IF_NAME)
	else if(!strcmp(priv->dev->name, ROOT_IFNAME_5G))
#else
	else if(!strcmp(priv->dev->name, "wlan0"))
#endif
		{
			priv->pmib->dot11RFEntry.macPhyMode = DUALMAC_DUALPHY;
			rtl_wpas_config_5G(priv);
		}
#if defined(CUSTOMIZE_WLAN_IF_NAME)
		else if(!strcmp(priv->dev->name, ROOT_IFNAME_2G))
#else
	else if(!strcmp(priv->dev->name, "wlan1"))
#endif
		{
			priv->pmib->dot11RFEntry.macPhyMode = DUALMAC_DUALPHY;
			rtl_wpas_config_2G(priv);
		}
	else
		return -1;

	return 0;


}


int rtl_wpas_custom(struct net_device *dev, struct iw_request_info *info, union iwreq_data *wrqu, char *extra)
{
	struct rtl8192cd_priv *priv = GET_DEV_PRIV(dev);
	struct rtk_wpas_config *config = (struct rtk_wpas_config *)wrqu->data.pointer;

	int ret = 0;

	switch (config->type) {
	case WPAS_CONFIG_MIB:
		HAPD_MSG("bandmode %d phymode %d \n", config->bandmode, config->phymode);
		if((config->bandmode == SINGLEMAC_SINGLEPHY) || (config->bandmode == DUALMAC_DUALPHY))
			ret = rtl_wpas_config(priv, config->bandmode, config->phymode);
		else
			return -EINVAL;
		break;
	case WPAS_CONFIG_WEPKEY: 
		HAPD_MSG("WPAS_CONFIG_WEPKEY !!! \n");
		CTRL_LEN_CHECK(16,config->wep_keylen); 
		memcpy(&priv->pmib->dot11DefaultKeysTable.keytype[config->wep_keyidx].skey[0], config->wep_key, config->wep_keylen);
		break;

	}

	return ret;

}
#endif

static void Construct_RSNIE(struct rtl8192cd_priv *priv, unsigned char *pucOut, int *usOutLen)
{
	DOT11_RSN_IE_HEADER dot11RSNIEHeader = { 0 };
	DOT11_RSN_IE_SUITE dot11RSNGroupSuite;
	DOT11_RSN_IE_COUNT_SUITE *pDot11RSNPairwiseSuite = NULL;
	DOT11_RSN_IE_COUNT_SUITE *pDot11RSNAuthSuite = NULL;
	unsigned short usSuitCount;
	unsigned long ulIELength = 0;
	unsigned long ulIndex = 0;
	unsigned long ulPairwiseLength = 0;
	unsigned long ulAuthLength = 0;
	unsigned char *pucBlob;
	DOT11_RSN_IE_COUNT_SUITE countSuite, authCountSuite;
#ifdef RTL_WPA2
	DOT11_RSN_CAPABILITY dot11RSNCapability = { 0 };
	unsigned long uCipherAlgo = 0;
	int bCipherAlgoEnabled = FALSE;
	unsigned long uAuthAlgo = 0;
	int bAuthAlgoEnabled = FALSE;
	unsigned long ulRSNCapabilityLength = 0;
#endif

	*usOutLen = 0;
	if ( priv->pmib->dot1180211AuthEntry.dot11WPACipher != 0 ) {
		//
		// Construct Information Header
		//
		dot11RSNIEHeader.ElementID = RSN_ELEMENT_ID;
		dot11RSNIEHeader.OUI[0] = 0x00;
		dot11RSNIEHeader.OUI[1] = 0x50;
		dot11RSNIEHeader.OUI[2] = 0xf2;
		dot11RSNIEHeader.OUI[3] = 0x01;
		dot11RSNIEHeader.Version = cpu_to_le16(RSN_VER1);
		ulIELength += sizeof(DOT11_RSN_IE_HEADER);

		// Construct Cipher Suite:
		// - Multicast Suite:
		memset(&dot11RSNGroupSuite, 0, sizeof dot11RSNGroupSuite);
		dot11RSNGroupSuite.OUI[0] = 0x00;
		dot11RSNGroupSuite.OUI[1] = 0x50;
		dot11RSNGroupSuite.OUI[2] = 0xF2;
		dot11RSNGroupSuite.Type = priv->wpa_global_info->MulticastCipher;
		ulIELength += sizeof(DOT11_RSN_IE_SUITE);

    	// - UnicastSuite
        pDot11RSNPairwiseSuite = &countSuite;
        memset(pDot11RSNPairwiseSuite, 0, sizeof(DOT11_RSN_IE_COUNT_SUITE));
		usSuitCount = 0;

#ifdef RTK_NL80211
		for (ulIndex=0; ulIndex<priv->wpa_global_info->NumOfUnicastCipher; ulIndex++)
		{
			int i = ulIndex<priv->wpa_global_info->NumOfUnicastCipher - ulIndex - 1;
			pDot11RSNPairwiseSuite->dot11RSNIESuite[usSuitCount].OUI[0] = 0x00;
			pDot11RSNPairwiseSuite->dot11RSNIESuite[usSuitCount].OUI[1] = 0x50;
			pDot11RSNPairwiseSuite->dot11RSNIESuite[usSuitCount].OUI[2] = 0xF2;
			pDot11RSNPairwiseSuite->dot11RSNIESuite[usSuitCount].Type = priv->wpa_global_info->UnicastCipher[i];
			usSuitCount++;
		}
#else
        for (ulIndex=0; ulIndex<priv->wpa_global_info->NumOfUnicastCipher; ulIndex++)
        {
			pDot11RSNPairwiseSuite->dot11RSNIESuite[usSuitCount].OUI[0] = 0x00;
			pDot11RSNPairwiseSuite->dot11RSNIESuite[usSuitCount].OUI[1] = 0x50;
			pDot11RSNPairwiseSuite->dot11RSNIESuite[usSuitCount].OUI[2] = 0xF2;
			pDot11RSNPairwiseSuite->dot11RSNIESuite[usSuitCount].Type = priv->wpa_global_info->UnicastCipher[ulIndex];
			usSuitCount++;
        }
#endif

		pDot11RSNPairwiseSuite->SuiteCount = cpu_to_le16(usSuitCount);
        ulPairwiseLength = sizeof(pDot11RSNPairwiseSuite->SuiteCount) + usSuitCount*sizeof(DOT11_RSN_IE_SUITE);
        ulIELength += ulPairwiseLength;

		//
		// Construction of Auth Algo List
		//
        pDot11RSNAuthSuite = &authCountSuite;
        memset(pDot11RSNAuthSuite, 0, sizeof(DOT11_RSN_IE_COUNT_SUITE));
		usSuitCount = 0;
		pDot11RSNAuthSuite->dot11RSNIESuite[usSuitCount].OUI[0] = 0x00;
		pDot11RSNAuthSuite->dot11RSNIESuite[usSuitCount].OUI[1] = 0x50;
		pDot11RSNAuthSuite->dot11RSNIESuite[usSuitCount].OUI[2] = 0xF2;
		pDot11RSNAuthSuite->dot11RSNIESuite[usSuitCount].Type = DOT11_AuthKeyType_RSN;
	    usSuitCount++;

		pDot11RSNAuthSuite->SuiteCount = cpu_to_le16(usSuitCount);
        ulAuthLength = sizeof(pDot11RSNAuthSuite->SuiteCount) + usSuitCount*sizeof(DOT11_RSN_IE_SUITE);
        ulIELength += ulAuthLength;

		pucBlob = pucOut;
		pucBlob += sizeof(DOT11_RSN_IE_HEADER);
		memcpy(pucBlob, &dot11RSNGroupSuite, sizeof(DOT11_RSN_IE_SUITE));
		pucBlob += sizeof(DOT11_RSN_IE_SUITE);
		memcpy(pucBlob, pDot11RSNPairwiseSuite, ulPairwiseLength);
		pucBlob += ulPairwiseLength;
		memcpy(pucBlob, pDot11RSNAuthSuite, ulAuthLength);
		pucBlob += ulAuthLength;

		*usOutLen = (int)ulIELength;
		pucBlob = pucOut;
		dot11RSNIEHeader.Length = (unsigned char)ulIELength - 2; //This -2 is to minus elementID and Length in OUI header
		memcpy(pucBlob, &dot11RSNIEHeader, sizeof(DOT11_RSN_IE_HEADER));
	}

#ifdef RTL_WPA2
	if ( priv->pmib->dot1180211AuthEntry.dot11WPA2Cipher != 0 ) {
       	DOT11_WPA2_IE_HEADER dot11WPA2IEHeader = { 0 };
		ulIELength = 0;
		ulIndex = 0;
		ulPairwiseLength = 0;
		uCipherAlgo = 0;
		bCipherAlgoEnabled = FALSE;
		ulAuthLength = 0;
		uAuthAlgo = 0;
		bAuthAlgoEnabled = FALSE;
		ulRSNCapabilityLength = 0;

		//
		// Construct Information Header
		//
		dot11WPA2IEHeader.ElementID = WPA2_ELEMENT_ID;
		dot11WPA2IEHeader.Version = cpu_to_le16(RSN_VER1);
		ulIELength += sizeof(DOT11_WPA2_IE_HEADER);

		// Construct Cipher Suite:
		//      - Multicast Suite:
		//
		memset(&dot11RSNGroupSuite, 0, sizeof(dot11RSNGroupSuite));
		dot11RSNGroupSuite.OUI[0] = 0x00;
		dot11RSNGroupSuite.OUI[1] = 0x0F;
		dot11RSNGroupSuite.OUI[2] = 0xAC;
		dot11RSNGroupSuite.Type = priv->wpa_global_info->MulticastCipher;;
		ulIELength += sizeof(DOT11_RSN_IE_SUITE);

		//      - UnicastSuite
        pDot11RSNPairwiseSuite = &countSuite;
        memset(pDot11RSNPairwiseSuite, 0, sizeof(DOT11_RSN_IE_COUNT_SUITE));
		usSuitCount = 0;

#ifdef RTK_NL80211
		for (ulIndex=0; ulIndex<priv->wpa_global_info->NumOfUnicastCipherWPA2; ulIndex++)
		{
			int i = priv->wpa_global_info->NumOfUnicastCipherWPA2 - ulIndex - 1; 
			pDot11RSNPairwiseSuite->dot11RSNIESuite[usSuitCount].OUI[0] = 0x00;
			pDot11RSNPairwiseSuite->dot11RSNIESuite[usSuitCount].OUI[1] = 0x0F;
			pDot11RSNPairwiseSuite->dot11RSNIESuite[usSuitCount].OUI[2] = 0xAC;
			pDot11RSNPairwiseSuite->dot11RSNIESuite[usSuitCount].Type = priv->wpa_global_info->UnicastCipherWPA2[i];
			usSuitCount++;
		}
#else
		for (ulIndex=0; ulIndex<priv->wpa_global_info->NumOfUnicastCipherWPA2; ulIndex++)
        {
			pDot11RSNPairwiseSuite->dot11RSNIESuite[usSuitCount].OUI[0] = 0x00;
			pDot11RSNPairwiseSuite->dot11RSNIESuite[usSuitCount].OUI[1] = 0x0F;
			pDot11RSNPairwiseSuite->dot11RSNIESuite[usSuitCount].OUI[2] = 0xAC;
			pDot11RSNPairwiseSuite->dot11RSNIESuite[usSuitCount].Type = priv->wpa_global_info->UnicastCipherWPA2[ulIndex];
			usSuitCount++;
        }
#endif

		pDot11RSNPairwiseSuite->SuiteCount = cpu_to_le16(usSuitCount);
        ulPairwiseLength = sizeof(pDot11RSNPairwiseSuite->SuiteCount) + usSuitCount*sizeof(DOT11_RSN_IE_SUITE);
        ulIELength += ulPairwiseLength;

		//
		// Construction of Auth Algo List
		//
        pDot11RSNAuthSuite = &authCountSuite;
        memset(pDot11RSNAuthSuite, 0, sizeof(DOT11_RSN_IE_COUNT_SUITE));
		usSuitCount = 0;
		pDot11RSNAuthSuite->dot11RSNIESuite[usSuitCount].OUI[0] = 0x00;
		pDot11RSNAuthSuite->dot11RSNIESuite[usSuitCount].OUI[1] = 0x0F;
		pDot11RSNAuthSuite->dot11RSNIESuite[usSuitCount].OUI[2] = 0xAC;
		pDot11RSNAuthSuite->dot11RSNIESuite[usSuitCount].Type = DOT11_AuthKeyType_RSN;
	    usSuitCount++;

		pDot11RSNAuthSuite->SuiteCount = cpu_to_le16(usSuitCount);
        ulAuthLength = sizeof(pDot11RSNAuthSuite->SuiteCount) + usSuitCount*sizeof(DOT11_RSN_IE_SUITE);
        ulIELength += ulAuthLength;

		//---------------------------------------------------------------------------------------------
		// Do not encapsulate capability field to solve TI WPA issue
		//---------------------------------------------------------------------------------------------

		dot11RSNCapability.field.PreAuthentication = 0;

		ulRSNCapabilityLength = sizeof(DOT11_RSN_CAPABILITY);
		ulIELength += ulRSNCapabilityLength;

#if (defined(WIFI_HAPD) & defined(WIFI_WMM))  || (defined(RTK_NL80211) & defined(WIFI_WMM)) //eric-eap
		if(QOS_ENABLE){
			/* 4 PTKSA replay counters when using WMM consistent with hostapd code*/
			dot11RSNCapability.field.PtksaReplayCounter = 3;
		}
#endif

		pucBlob = pucOut + *usOutLen;
		pucBlob += sizeof(DOT11_WPA2_IE_HEADER);
		memcpy(pucBlob, &dot11RSNGroupSuite, sizeof(DOT11_RSN_IE_SUITE));
		pucBlob += sizeof(DOT11_RSN_IE_SUITE);
		memcpy(pucBlob, pDot11RSNPairwiseSuite, ulPairwiseLength);
		pucBlob += ulPairwiseLength;
		memcpy(pucBlob, pDot11RSNAuthSuite, ulAuthLength);
		pucBlob += ulAuthLength;
		memcpy(pucBlob, &dot11RSNCapability, ulRSNCapabilityLength);

		pucBlob = pucOut + *usOutLen;
		dot11WPA2IEHeader.Length = (unsigned char)ulIELength - 2; //This -2 is to minus elementID and Length in OUI header
		memcpy(pucBlob, &dot11WPA2IEHeader, sizeof(DOT11_WPA2_IE_HEADER));
		*usOutLen = *usOutLen + (int)ulIELength;
   	}
#endif // RTL_WPA2

}


static void ToDrv_SetRSNIE(struct rtl8192cd_priv *priv)
{
	struct iw_point wrq;
	DOT11_SET_RSNIE Set_Rsnie;


	debug_out("RSN: Set RSNIE", priv->wpa_global_info->AuthInfoElement.Octet,
								priv->wpa_global_info->AuthInfoElement.Length);


	wrq.pointer = (caddr_t)&Set_Rsnie;
	wrq.length = sizeof(DOT11_SET_RSNIE);
	Set_Rsnie.EventId = DOT11_EVENT_SET_RSNIE;
	Set_Rsnie.IsMoreEvent = FALSE;
	Set_Rsnie.Flag = DOT11_Ioctl_Set;
	Set_Rsnie.RSNIELen = priv->wpa_global_info->AuthInfoElement.Length;
	memcpy(&Set_Rsnie.RSNIE,
			priv->wpa_global_info->AuthInfoElement.Octet,
			priv->wpa_global_info->AuthInfoElement.Length);

	rtl8192cd_ioctl_priv_daemonreq(priv->dev, &wrq);
}


void rsn_init(struct rtl8192cd_priv *priv)
{
	WPA_GLOBAL_INFO *pGblInfo=priv->wpa_global_info;
	int i, j, low_cipher=0;

	DEBUG_TRACE;

	HAPD_MSG("rsn_init\n");

	memset((char *)pGblInfo, '\0', sizeof(WPA_GLOBAL_INFO));

	if (priv->pmib->dot1180211AuthEntry.dot11WPACipher) {
		for (i=0, j=0; i<_WEP_104_PRIVACY_; i++) {
			if (priv->pmib->dot1180211AuthEntry.dot11WPACipher & (1<<i)) {
				pGblInfo->UnicastCipher[j] = i+1;
				if (low_cipher == 0)
					low_cipher = pGblInfo->UnicastCipher[j];
				else {
					if (low_cipher == _WEP_104_PRIVACY_ &&
							pGblInfo->UnicastCipher[j] == _WEP_40_PRIVACY_)
						low_cipher = pGblInfo->UnicastCipher[j];
					else if (low_cipher == _TKIP_PRIVACY_ &&
							(pGblInfo->UnicastCipher[j] == _WEP_40_PRIVACY_ ||
								pGblInfo->UnicastCipher[j] == _WEP_104_PRIVACY_))
							low_cipher = pGblInfo->UnicastCipher[j];
					else if (low_cipher == _CCMP_PRIVACY_)
							low_cipher = pGblInfo->UnicastCipher[j];
				}
				if (++j >= MAX_UNICAST_CIPHER)
					break;
			}
		}
		pGblInfo->NumOfUnicastCipher = j;
	}

#ifdef RTL_WPA2
	if (priv->pmib->dot1180211AuthEntry.dot11WPA2Cipher) {
		for (i=0, j=0; i<_WEP_104_PRIVACY_; i++) {
			if (priv->pmib->dot1180211AuthEntry.dot11WPA2Cipher & (1<<i)) {
				pGblInfo->UnicastCipherWPA2[j] = i+1;
				if (low_cipher == 0)
					low_cipher = pGblInfo->UnicastCipherWPA2[j];
				else {
					if (low_cipher == _WEP_104_PRIVACY_ &&
							pGblInfo->UnicastCipherWPA2[j] == _WEP_40_PRIVACY_)
						low_cipher = pGblInfo->UnicastCipherWPA2[j];
					else if (low_cipher == _TKIP_PRIVACY_ &&
							(pGblInfo->UnicastCipherWPA2[j] == _WEP_40_PRIVACY_ ||
								pGblInfo->UnicastCipherWPA2[j] == _WEP_104_PRIVACY_))
							low_cipher = pGblInfo->UnicastCipherWPA2[j];
					else if (low_cipher == _CCMP_PRIVACY_)
							low_cipher = pGblInfo->UnicastCipherWPA2[j];
				}
				if (++j >= MAX_UNICAST_CIPHER)
					break;
			}
		}
		pGblInfo->NumOfUnicastCipherWPA2= j;
	}
#endif

	pGblInfo->MulticastCipher = low_cipher;

	HAPD_MSG("RSN: WPA unicast cipher= ");
	for (i=0; i<pGblInfo->NumOfUnicastCipher; i++)
		HAPD_MSG("%x ", pGblInfo->UnicastCipher[i]);
	HAPD_MSG("\n");

#ifdef RTL_WPA2
	HAPD_MSG("RSN: WPA2 unicast cipher= ");
	for (i=0; i<pGblInfo->NumOfUnicastCipherWPA2; i++)
		HAPD_MSG("%x ", pGblInfo->UnicastCipherWPA2[i]);
	HAPD_MSG("\n");
#endif

	HAPD_MSG("RSN: multicast cipher= %x\n", pGblInfo->MulticastCipher);


	pGblInfo->AuthInfoElement.Octet = pGblInfo->AuthInfoBuf;

	Construct_RSNIE(priv, pGblInfo->AuthInfoElement.Octet,
					 &pGblInfo->AuthInfoElement.Length);

	ToDrv_SetRSNIE(priv);
}

#endif //WIFI_HAPD
