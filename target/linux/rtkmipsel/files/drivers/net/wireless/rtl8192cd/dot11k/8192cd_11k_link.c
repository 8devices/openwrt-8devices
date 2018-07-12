/*
 *  Software TKIP encryption/descryption routines
 *
 *  $Id: 8192cd_tkip.c,v 1.4.4.2 2010/09/30 05:27:28 button Exp $
 *
 *  Copyright (c) 2009 Realtek Semiconductor Corp.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 */

#define _8192CD_11K_LINK_C_

#ifdef __KERNEL__
#include <linux/module.h>
#include <asm/byteorder.h>
#elif defined(__ECOS)
#include <cyg/io/eth/rltk/819x/wrapper/sys_support.h>
#include <cyg/io/eth/rltk/819x/wrapper/skbuff.h>
#include <cyg/io/eth/rltk/819x/wrapper/timer.h>
#include <cyg/io/eth/rltk/819x/wrapper/wrapper.h>
#endif

#include "../8192cd_cfg.h"

#if !defined(__KERNEL__) && !defined(__ECOS)
#include "../sys-support.h"
#endif

#include "../8192cd.h"
#ifdef __KERNEL__
#include "../ieee802_mib.h"
#elif defined(__ECOS)
#include <cyg/io/eth/rltk/819x/wlan/ieee802_mib.h>
#endif
#include "../8192cd_util.h"
#include "../8192cd_headers.h"
#include "../8192cd_debug.h"

#ifdef DOT11K
static int issue_link_measurement_request(struct rtl8192cd_priv *priv, struct stat_info *pstat)
{
    unsigned char   *pbuf;
    int ret;
    DECLARE_TXINSN(txinsn);

    txinsn.q_num = MANAGE_QUE_NUM;
    txinsn.fr_type = _PRE_ALLOCMEM_;
    txinsn.tx_rate = find_rate(priv, NULL, 0, 1);
    #ifndef TX_LOWESTRATE
    txinsn.lowest_tx_rate = txinsn.tx_rate;
    #endif
    txinsn.fixed_rate = 1;
#ifdef CONFIG_IEEE80211W		
	if(pstat)
		txinsn.isPMF = pstat->isPMF;
	else
		txinsn.isPMF = 0;	
#endif	
    pbuf = txinsn.pframe = get_mgtbuf_from_poll(priv);
    if (pbuf == NULL)
        goto issue_link_request_fail;

    txinsn.phdr = get_wlanhdr_from_poll(priv);
    if (txinsn.phdr == NULL)
        goto issue_link_request_fail;

    memset((void *)(txinsn.phdr), 0, sizeof(struct wlan_hdr));

    pbuf[0] = _RADIO_MEASUREMENT_CATEGORY_ID_;
    pbuf[1] = _LINK_MEASUREMENT_REQEST_ACTION_ID_;

    if (!(++pstat->dialog_token))	// dialog token set to a non-zero value
        pstat->dialog_token++;

    pstat->rm.link_dialog_token = pstat->dialog_token;
    pbuf[2] = pstat->dialog_token;
    pbuf[3] = priv->pmib->dot11hTPCEntry.tpc_tx_power; /*transmit power used*/

    /*max transmit power, looking from contry table*/
    if(priv->countryTableIdx)
    {
        pbuf[4] = search_country_txpower(priv, priv->pmib->dot11RFEntry.dot11channel);
    }

    if(pbuf[4] == 0)
        pbuf[4] = 20;

    txinsn.fr_len = 5;

    SetFrameSubType((txinsn.phdr), WIFI_WMM_ACTION);
#ifdef CONFIG_IEEE80211W
	if (txinsn.isPMF)
		*(unsigned char*)(txinsn.phdr+1) |= BIT(6); // enable privacy 
#endif
    memcpy((void *)GetAddr1Ptr((txinsn.phdr)), pstat->hwaddr, MACADDRLEN);
    memcpy((void *)GetAddr2Ptr((txinsn.phdr)), GET_MY_HWADDR, MACADDRLEN);
    memcpy((void *)GetAddr3Ptr((txinsn.phdr)), BSSID, MACADDRLEN);

#if defined(WIFI_WMM)
    ret = check_dz_mgmt(priv, pstat, &txinsn);
    if (ret < 0)
        goto issue_link_request_fail;
    else if (ret==1)
        return 0;
    else
#endif
    if ((rtl8192cd_firetx(priv, &txinsn)) == SUCCESS)
        return 0;

issue_link_request_fail:

    if (txinsn.phdr)
        release_wlanhdr_to_poll(priv, txinsn.phdr);
    if (txinsn.pframe)
        release_mgtbuf_to_poll(priv, txinsn.pframe);
    return -1;

}




static int issue_link_measurement_report(struct rtl8192cd_priv *priv, struct stat_info *pstat,
        unsigned char dialog_token, unsigned char rcpi, unsigned char rsni)
{
    unsigned char   *pbuf;
    unsigned int frlen;
    int ret;
    DECLARE_TXINSN(txinsn);
    txinsn.q_num = MANAGE_QUE_NUM;
    txinsn.fr_type = _PRE_ALLOCMEM_;
    txinsn.tx_rate = find_rate(priv, NULL, 0, 1);
    #ifndef TX_LOWESTRATE
    txinsn.lowest_tx_rate = txinsn.tx_rate;
    #endif
    txinsn.fixed_rate = 1;
#ifdef CONFIG_IEEE80211W		
	if(pstat)
		txinsn.isPMF = pstat->isPMF;
	else
		txinsn.isPMF = 0;	
#endif	
    pbuf = txinsn.pframe = get_mgtbuf_from_poll(priv);
    if (pbuf == NULL)
        goto issue_link_report_fail;

    txinsn.phdr = get_wlanhdr_from_poll(priv);
    if (txinsn.phdr == NULL)
        goto issue_link_report_fail;

    memset((void *)(txinsn.phdr), 0, sizeof(struct wlan_hdr));

    pbuf[0] = _RADIO_MEASUREMENT_CATEGORY_ID_;
    pbuf[1] = _LINK_MEASUREMENT_REPORT_ACTION_ID_;
    pbuf[2] = dialog_token;
    frlen = 3;

    construct_TPC_report_ie(priv, pbuf + frlen, &frlen);
    pbuf[frlen++] = 0; // Receive Antenna ID
    pbuf[frlen++] = 0; // Transmit Antenna ID
    pbuf[frlen++] = rcpi;
    pbuf[frlen++] = rsni;

    txinsn.fr_len += frlen;
    SetFrameSubType((txinsn.phdr), WIFI_WMM_ACTION);
#ifdef CONFIG_IEEE80211W
	if (txinsn.isPMF)
		*(unsigned char*)(txinsn.phdr+1) |= BIT(6); // enable privacy 
#endif
    memcpy((void *)GetAddr1Ptr((txinsn.phdr)), pstat->hwaddr, MACADDRLEN);
    memcpy((void *)GetAddr2Ptr((txinsn.phdr)), GET_MY_HWADDR, MACADDRLEN);
    memcpy((void *)GetAddr3Ptr((txinsn.phdr)), BSSID, MACADDRLEN);

#if defined(WIFI_WMM)
    ret = check_dz_mgmt(priv, pstat, &txinsn);
    if (ret < 0)
        goto issue_link_report_fail;
    else if (ret==1)
        return 0;
    else
#endif

    if ((rtl8192cd_firetx(priv, &txinsn)) == SUCCESS)
        return 0;

issue_link_report_fail:

    if (txinsn.phdr)
        release_wlanhdr_to_poll(priv, txinsn.phdr);
    if (txinsn.pframe)
        release_mgtbuf_to_poll(priv, txinsn.pframe);
    return -1;

}

int OnLinkMeasurementRequest(struct rtl8192cd_priv *priv, struct stat_info *pstat, struct rx_frinfo *pfrinfo)
{
    unsigned char * pframe = get_pframe(pfrinfo) + WLAN_HDR_A3_LEN;	//start of action frame content;
    return issue_link_measurement_report(priv, pstat, pframe[2], pfrinfo->rssi<<1, 0xFF);
}


void OnLinkMeasurementReport(struct rtl8192cd_priv *priv, struct stat_info *pstat, unsigned char *pframe)
{
    if(pstat->rm.link_dialog_token ==  pframe[2])
    {
        pstat->rm.link_dialog_token = 0;
        pstat->rm.link_measurement.tpc_tx_power = pframe[5];
        pstat->rm.link_measurement.tpc_link_margin = pframe[6];
        pstat->rm.link_measurement.recv_antenna_id = pframe[7];
        pstat->rm.link_measurement.xmit_antenna_id = pframe[8];
        pstat->rm.link_measurement.RCPI = pframe[9];
        pstat->rm.link_measurement.RSNI = pframe[10];

        pstat->rm.link_result = MEASUREMENT_SUCCEED;
    }
}


int rm_link_measurement_request(struct rtl8192cd_priv *priv, unsigned char *macaddr)
{
    int ret = -1;
    struct stat_info *pstat;
    if(priv->pmib->dot11StationConfigEntry.dot11RadioMeasurementActivated &&
            priv->pmib->dot11StationConfigEntry.dot11RMLinkMeasurementActivated)
    {
        pstat = get_stainfo(priv, macaddr);
        if(pstat)
        {
            ret = issue_link_measurement_request(priv, pstat);
            if(ret == 0)
            {
                pstat->rm.link_result = MEASUREMENT_PROCESSING;
            }
        }
    }
    return ret;
}

int rm_get_link_report(struct rtl8192cd_priv *priv, unsigned char *macaddr, unsigned char* result_buf)
{
    int len = -1;
    struct stat_info *pstat;

    if(priv->pmib->dot11StationConfigEntry.dot11RadioMeasurementActivated &&
            priv->pmib->dot11StationConfigEntry.dot11RMLinkMeasurementActivated)
    {

        pstat = get_stainfo(priv, macaddr);
        if(pstat)
        {
            *result_buf = pstat->rm.link_result;
            len = 1;

            if(pstat->rm.link_result == MEASUREMENT_SUCCEED)
            {
                memcpy(result_buf + 1, &pstat->rm.link_measurement, sizeof(struct dot11k_link_measurement));
                len += sizeof(struct dot11k_link_measurement);
                pstat->rm.link_result = MEASUREMENT_UNKNOWN;
            }

        }
    }

    return len;

}


#endif
