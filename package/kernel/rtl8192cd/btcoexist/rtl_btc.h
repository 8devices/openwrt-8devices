/******************************************************************************
 *
 * Copyright(c) 2009-2010  Realtek Corporation.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110, USA
 *
 * The full GNU General Public License is included in this distribution in the
 * file called LICENSE.
 *
 * Contact Information:
 * wlanfae <wlanfae@realtek.com>
 * Realtek Corporation, No. 2, Innovation Road II, Hsinchu Science Park,
 * Hsinchu 300, Taiwan.
 * Larry Finger <Larry.Finger@lwfinger.net>
 *
 *****************************************************************************/

#ifndef __RTL_BTC_H__
#define __RTL_BTC_H__

//#include "halbt_precomp.h"
//#include "../phydm/phydm_precomp.h"


#if 0
void rtl_btc_init_variables(struct rtl8192cd_priv *priv);
void rtl_btc_init_hal_vars(struct rtl8192cd_priv *priv);
void rtl_btc_init_hw_config(struct rtl8192cd_priv *priv);
void rtl_btc_ips_notify(struct rtl8192cd_priv *priv, u8 type);
void rtl_btc_lps_notify(struct rtl8192cd_priv *priv, u8 type);
void rtl_btc_scan_notify(struct rtl8192cd_priv *priv, u8 scantype);
void rtl_btc_connect_notify(struct rtl8192cd_priv *priv, u8 action);
void rtl_btc_mediastatus_notify(struct rtl8192cd_priv *priv,
	enum rt_media_status mstatus);
void rtl_btc_periodical(struct rtl8192cd_priv *priv);
void rtl_btc_halt_notify(void);
void rtl_btc_btinfo_notify(struct rtl8192cd_priv *priv, u8 *tmpBuf, u8 length);
bool rtl_btc_is_limited_dig(struct rtl8192cd_priv *priv);
bool rtl_btc_is_disable_edca_turbo(struct rtl8192cd_priv *priv);
bool rtl_btc_is_bt_disabled(struct rtl8192cd_priv *priv);
void rtl_btc_special_packet_notify(struct rtl8192cd_priv *priv, u8 pkt_type);
void rtl_btc_set_hci_version(u16 hci_version);
void rtl_btc_set_bt_patch_version(u16 bt_hci_version, u16 bt_patch_version);
void rtl_btc_stack_update_profile_info(void);
#endif
void rtl_btc_init_socket(struct rtl8192cd_priv *priv);
void rtl_btc_close_socket(struct rtl8192cd_priv *priv);

/*extern struct rtl_btc_ops rtl_btc_operation;*/
extern struct rtl_btc_ops *rtl_btc_get_ops_pointer(void);
#if 0

u8 rtl_get_hwpg_ant_num(struct rtl8192cd_priv *priv);
u8 rtl_get_hwpg_bt_exist(struct rtl8192cd_priv *priv);
u8 rtl_get_hwpg_bt_type(struct rtl8192cd_priv *priv);
enum rt_media_status mgnt_link_status_query(struct ieee80211_hw *hw);
#endif







#endif
