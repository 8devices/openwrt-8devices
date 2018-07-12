/*
 *  Header file for Net80211-compatible handling routines
 *
 *
 *
 *  Copyright (c) 2010 Realtek Semiconductor Corp.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 */
 

#ifndef _8192CD_NET80211_H_
#define _8192CD_NET80211_H_
 

#define MAX_CONFIG_FILE_SIZE (20*1024) // for 8192, added to 20k
#define MAX_PARAM_BUF_SIZE (1024) // for 8192, added to 20k


 int rtl_net80211_setparam(struct net_device *dev, struct iw_request_info *info, union iwreq_data *wrqu, char *extra);
 int rtl_net80211_setappiebuf(struct net_device *dev, struct iw_request_info *info, union iwreq_data *wrqu, char *extra);
 int rtl_net80211_setmlme(struct net_device *dev, struct iw_request_info *info, union iwreq_data *wrqu, char *extra);
 int rtl_net80211_setkey(struct net_device *dev, struct iw_request_info *info, union iwreq_data *wrqu, char *extra);
 int rtl_net80211_delkey(struct net_device *dev, struct iw_request_info *info, union iwreq_data *wrqu, char *extra);
 int rtl_net80211_wdsaddmac(struct net_device *dev, struct iw_request_info *info, union iwreq_data *wrqu, char *extra);
 int rtl_net80211_wdsdelmac(struct net_device *dev, struct iw_request_info *info, union iwreq_data *wrqu, char *extra);
 int rtl_net80211_getwpaie(struct net_device *dev, struct iw_request_info *info, union iwreq_data *wrqu, char *extra);
 int rtl_hapd_config(struct net_device *dev, struct iw_request_info *info, union iwreq_data *wrqu, char *extra);
 void rsn_init(struct rtl8192cd_priv *priv);
#if defined(RTK_NL80211)
 int rtl_wpas_join(struct rtl8192cd_priv *priv, int bss_num);
#endif

#endif
