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

#define _8192CD_11K_NEIGHBOR_C_

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

#ifdef CONFIG_IEEE80211V
#include "../8192cd_11v.h"
#endif

#ifdef DOT11K
#define DEFAULT_BSSINFO_VALUE    0x000E
const static unsigned char wildcard_bssid[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

unsigned char * construct_neighbor_report_ie(unsigned char *pbuf, unsigned int *frlen,
        struct dot11k_neighbor_report * report)
{
    report->bssinfo.value = cpu_to_le32(report->bssinfo.value);
    pbuf = set_ie(pbuf, _NEIGHBOR_REPORT_IE_, sizeof(struct dot11k_neighbor_report), (unsigned char*)report, frlen);
    report->bssinfo.value = le32_to_cpu(report->bssinfo.value);    
    return pbuf;
}


static int issue_neighbor_report_response(struct rtl8192cd_priv *priv, struct stat_info *pstat,
        unsigned char dialog_token, unsigned char *ssid, unsigned char ssid_len)
{
    unsigned char   *pbuf;
    unsigned int frlen;
    int i;
    int neighbor_size;
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
        goto issue_neighbor_report_fail;

    txinsn.phdr = get_wlanhdr_from_poll(priv);
    if (txinsn.phdr == NULL)
        goto issue_neighbor_report_fail;

    memset((void *)(txinsn.phdr), 0, sizeof(struct wlan_hdr));

    pbuf[0] = _RADIO_MEASUREMENT_CATEGORY_ID_;
    pbuf[1] = _NEIGHBOR_REPORT_RESPONSE_ACTION_ID_;
    pbuf[2] = dialog_token;
    frlen = 3;
    pbuf += frlen;

    neighbor_size = sizeof(struct dot11k_neighbor_report);
    for(i = 0; i < MAX_NEIGHBOR_REPORT; i++) {
        if((priv->rm_neighbor_bitmask[i>>3] & (1<<(i&7))) == 0)
            continue;
        if(frlen + neighbor_size > MAX_REPORT_FRAME_SIZE)
            break;

        if(ssid) {
            if(strlen(priv->rm_neighbor_info[i].ssid)) {                
                if(ssid_len != strlen(priv->rm_neighbor_info[i].ssid) ||
                    memcmp((unsigned char*)priv->rm_neighbor_info[i].ssid, ssid, ssid_len))
                    continue;
            }
            else {
                if(ssid_len != SSID_LEN || memcmp(ssid, SSID, SSID_LEN)) {
                    continue;
                }
            }
        }
        pbuf = construct_neighbor_report_ie(pbuf, &frlen, &priv->rm_neighbor_report[i]);
    }

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
        goto issue_neighbor_report_fail;
    else if (ret==1)
        return 0;
    else
#endif
    if ((rtl8192cd_firetx(priv, &txinsn)) == SUCCESS)
        return 0;

issue_neighbor_report_fail:

    if (txinsn.phdr)
        release_wlanhdr_to_poll(priv, txinsn.phdr);
    if (txinsn.pframe)
        release_mgtbuf_to_poll(priv, txinsn.pframe);
    return -1;

}

void OnNeighborReportRequest(struct rtl8192cd_priv *priv, struct stat_info *pstat, unsigned char *pframe, int frame_len)
{
    int len;
    unsigned char * p;
        
    if((OPMODE & WIFI_AP_STATE) == 0)
        return;

	// checking SSID
	p = get_ie(pframe + 3, _SSID_IE_, &len, frame_len-3);
	if (p == NULL) {
        issue_neighbor_report_response(priv, pstat, pframe[2], SSID, SSID_LEN);
    }
    else if((len == 0) ||		// NULL AP case 2
		(*(p+2) == '\0'))	// NULL AP case 3 (like 8181/8186)
	{
        issue_neighbor_report_response(priv, pstat, pframe[2], NULL, 0);
    }
    else {
        issue_neighbor_report_response(priv, pstat, pframe[2], p+2, len);
    }
    return;

}


int rm_construct_neighbor_report(struct rtl8192cd_priv *priv, struct rx_frinfo *pfrinfo, unsigned char channel)
{
    int i, len, empty_slot;
    unsigned char *pframe, *p, *bssid;
    unsigned int oldest_aging;
    int oldest_idx;
    unsigned char * ssid_ptr;
    int ssid_len;
    unsigned char cap_info;
    unsigned char mdid_cap = 0;
    unsigned short sta_count = 0;
    unsigned char channel_utilization = 0;

    pframe = get_pframe(pfrinfo);
    bssid = GetAddr3Ptr(pframe);

    /* checking SSID */
    p = get_ie(pframe + WLAN_HDR_A3_LEN + _BEACON_IE_OFFSET_, _SSID_IE_, &len,
               pfrinfo->pktlen - WLAN_HDR_A3_LEN - _BEACON_IE_OFFSET_);
    if ((p == NULL) ||		// NULL AP case 1
         (len == 0) ||		// NULL AP case 2
         (*(p+2) == '\0'))	// NULL AP case 3 (like 8181/8186)
    {
        return 0;
    }

    if (SSID_LEN != len || memcmp(SSID, p+2, len))
        return 0;
    
    ssid_ptr = p+2;
    ssid_len = len;


    /*search for neighbor report*/
    for(i = 0, empty_slot = -1; i < MAX_NEIGHBOR_REPORT; i++)
    {
        if((priv->rm_neighbor_bitmask[i>>3] & (1<<(i&7))) == 0)
        {
            if(empty_slot == -1)
                empty_slot = i;
        }
        else if(0 == memcmp(bssid, priv->rm_neighbor_report[i].bssid, MACADDRLEN))
        {
            break;
        }
    }
    if(i == MAX_NEIGHBOR_REPORT && empty_slot != -1)   /*not found, and has empty slot*/
    {
        i = empty_slot;        
        memcpy(priv->rm_neighbor_report[i].bssid, bssid, MACADDRLEN);
    }
    
    if(i == MAX_NEIGHBOR_REPORT)  /*not found and no empty slot, find the oldest neighbor and replace it*/
    {
        for(i = 0; i < MAX_NEIGHBOR_REPORT; i++)
        {
            if(priv->rm_neighbor_bitmask[i>>3] & (1<<(i&7)))
            {
                if(oldest_aging < priv->rm_neighbor_info[i].aging) {
                    oldest_aging = priv->rm_neighbor_info[i].aging;
                    oldest_idx = i;
                }
            }
        }        
        i = oldest_idx;        
        memcpy(priv->rm_neighbor_report[i].bssid, bssid, MACADDRLEN);
    }

    priv->rm_neighbor_report[i].bssinfo.value = DEFAULT_BSSINFO_VALUE;
    priv->rm_neighbor_report[i].channel = channel;
    priv->rm_neighbor_report[i].op_class = rm_get_op_class(priv, channel);
    priv->rm_neighbor_report[i].phytype = 0;

    /* parsing fixed-length field*/
    /*capability byte 1*/
    cap_info = *(pframe + WLAN_HDR_A3_LEN + 11);
    if(cap_info & 0x01) { /*Specture mgmt*/
        priv->rm_neighbor_report[i].bssinfo.field.cap_spectrum = 1;
    }
    if(cap_info & 0x02) { /*Qos*/
        priv->rm_neighbor_report[i].bssinfo.field.cap_qos = 1;
    }
    if(cap_info & 0x08) { /*APSD*/
        priv->rm_neighbor_report[i].bssinfo.field.cap_apsd = 1;
    }
    if(cap_info & 0x10) { /*Radio Measurement*/
        priv->rm_neighbor_report[i].bssinfo.field.cap_rm = 1;
    }            
    if(cap_info & 0x40) { /*Delayed Block Ack*/
        priv->rm_neighbor_report[i].bssinfo.field.cap_delay_ba = 1;
    }  
    if(cap_info & 0x80) { /*Immediate  Block Ack*/
        priv->rm_neighbor_report[i].bssinfo.field.cap_im_ba = 1;
    } 

    /* checking MDID IE */
    if(FT_ENABLE) {
        p = get_ie(pframe + WLAN_HDR_A3_LEN + _BEACON_IE_OFFSET_, _MOBILITY_DOMAIN_IE_, &len,
                pfrinfo->pktlen - WLAN_HDR_A3_LEN - _BEACON_IE_OFFSET_);
        if (p)
        {
            if (priv->pmib->dot11FTEntry.dot11FTOverDSEnabled)
                mdid_cap |= BIT(0);
            if (priv->pmib->dot11FTEntry.dot11FTResourceRequestSupported)
                mdid_cap |= BIT(1);
       
            if(memcmp(p+2, MDID, 2) == 0 && mdid_cap == p[4])
                priv->rm_neighbor_report[i].bssinfo.field.mde = 1;
        }
    }
    
    /* checking HT_CAP IE */
    p = get_ie(pframe + WLAN_HDR_A3_LEN + _BEACON_IE_OFFSET_, _HT_CAP_, &len,
         pfrinfo->pktlen - WLAN_HDR_A3_LEN - _BEACON_IE_OFFSET_);
    if (p)
    {
        if (priv->ht_cap_len == len) {
            if(memcmp(p+2, (unsigned char *)&priv->ht_cap_buf, priv->ht_cap_len) == 0) /*get HT CAP ie, check the content is same as our AP*/
                priv->rm_neighbor_report[i].bssinfo.field.high_tp = 1;
        }   
    }

    /* checking BSS Load IE */
    p = get_ie(pframe + WLAN_HDR_A3_LEN + _BEACON_IE_OFFSET_, _BSS_LOAD_IE_, &len,
         pfrinfo->pktlen - WLAN_HDR_A3_LEN - _BEACON_IE_OFFSET_);
    if (p)
    {
        sta_count = le16_to_cpu(*(unsigned short *)(p+2));
        channel_utilization = p+4; 
    }

    memcpy(priv->rm_neighbor_info[i].ssid, ssid_ptr, ssid_len);
    priv->rm_neighbor_info[i].ssid[ssid_len] = 0;
    priv->rm_neighbor_info[i].aging = 0;
    priv->rm_neighbor_info[i].bssload.sta_count = sta_count;
    priv->rm_neighbor_info[i].bssload.channel_utilization = channel_utilization;
    priv->rm_neighbor_bitmask[i>>3] |= (1<<(i&7));        

#ifdef CONFIG_IEEE80211V
    if(WNM_ENABLE) {
   		priv->rm_neighbor_report[i].subelemnt.subelement_id = _WNM_BSS_TRANS_CANDIDATE_PREFRENCE_;
   		priv->rm_neighbor_report[i].subelemnt.len = 1;
   		priv->rm_neighbor_report[i].subelemnt.preference = getPreferredVal(priv, channel_utilization, pfrinfo->rssi<<1, 0);
    }
#endif		

}

#ifdef CLIENT_MODE
void OnNeighborReportResponse(struct rtl8192cd_priv *priv, struct stat_info *pstat,
                              unsigned char *pframe, int frame_len)
{
    int len;
    unsigned char element_id;
    unsigned char element_len;
    struct dot11k_neighbor_report *report;

    if(priv->rm.neighbor_dialog_token != pframe[2] || priv->rm.neighbor_result != MEASUREMENT_PROCESSING)
    {
        return;
    }


    priv->rm.neighbor_dialog_token = 0;

    len = 3;
    while(len + 5 <= frame_len)
    {
   
        element_id = pframe[len];
        element_len = pframe[len + 1];
        /*parsing every radio measurment report element*/
        if(element_id == _NEIGHBOR_REPORT_IE_)
        {
            report = &priv->rm.neighbor_report[priv->rm.neighbor_report_num];
            memcpy(report->bssid, pframe+len+2, MACADDRLEN);
            report->bssinfo.value = le32_to_cpu(*(unsigned int *)&pframe[len + 8]);
            report->op_class = pframe[len+12];
            report->channel = pframe[len+13];
            report->phytype = pframe[len+14];

            priv->rm.neighbor_report_num++;
            if(priv->rm.neighbor_report_num >= MAX_NEIGHBOR_REPORT)
                break;
        }
        len += 2 + element_len;
    }

    priv->rm.neighbor_result = MEASUREMENT_SUCCEED;
}



static int issue_neighbor_report_request(struct rtl8192cd_priv *priv, struct stat_info *pstat, char* ssid)
{
    unsigned char   *pbuf;
    unsigned int frlen;

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
        goto issue_beacon_request_fail;

    txinsn.phdr = get_wlanhdr_from_poll(priv);
    if (txinsn.phdr == NULL)
        goto issue_beacon_request_fail;

    memset((void *)(txinsn.phdr), 0, sizeof(struct wlan_hdr));

    pbuf[0] = _RADIO_MEASUREMENT_CATEGORY_ID_;
    pbuf[1] = _NEIGHBOR_REPORT_REQEST_ACTION_ID_;

    if (!(++pstat->dialog_token))	// dialog token set to a non-zero value
        pstat->dialog_token++;

    priv->rm.neighbor_dialog_token = pstat->dialog_token;
    pbuf[2] = pstat->dialog_token;
    frlen = 3;
    if(ssid)
    {
        if ((strlen(ssid) == 3) &&
                ((ssid[0] == 'A') || (ssid[0] == 'a')) &&
                ((ssid[1] == 'N') || (ssid[1] == 'n')) &&
                ((ssid[2] == 'Y') || (ssid[2] == 'y')))
        {
            pbuf = set_ie(pbuf+frlen, _SSID_IE_, 0, NULL, &frlen);
        }
        else
        {
            pbuf = set_ie(pbuf+frlen, _SSID_IE_, strlen(ssid), ssid, &frlen);
        }
    }

    txinsn.fr_len = frlen;

    SetFrameSubType((txinsn.phdr), WIFI_WMM_ACTION);
#ifdef CONFIG_IEEE80211W
	if (txinsn.isPMF)
		*(unsigned char*)(txinsn.phdr+1) |= BIT(6); // enable privacy 
#endif
    memcpy((void *)GetAddr1Ptr((txinsn.phdr)), pstat->hwaddr, MACADDRLEN);
    memcpy((void *)GetAddr2Ptr((txinsn.phdr)), GET_MY_HWADDR, MACADDRLEN);
    memcpy((void *)GetAddr3Ptr((txinsn.phdr)), BSSID, MACADDRLEN);

    if ((rtl8192cd_firetx(priv, &txinsn)) == SUCCESS)
        return 0;

issue_beacon_request_fail:

    if (txinsn.phdr)
        release_wlanhdr_to_poll(priv, txinsn.phdr);
    if (txinsn.pframe)
        release_mgtbuf_to_poll(priv, txinsn.pframe);
    return -1;

}

int rm_neighbor_request(struct rtl8192cd_priv *priv, char *ssid)
{
    int ret = -1;
    struct stat_info *pstat;

    if ((OPMODE & (WIFI_STATION_STATE | WIFI_ASOC_STATE)) != (WIFI_STATION_STATE | WIFI_ASOC_STATE))
        goto neighbor_req_fail;

    if(priv->pmib->dot11StationConfigEntry.dot11RadioMeasurementActivated &&
            priv->pmib->dot11StationConfigEntry.dot11RMNeighborReportActivated)
    {
        pstat = get_stainfo(priv, BSSID);
        if(pstat)
        {
            ret = issue_neighbor_report_request(priv, pstat, ssid);
            if(ret == 0)   /*issue beacon measurement request succeed*/
            {
                priv->rm.neighbor_report_num = 0;
                priv->rm.neighbor_result = MEASUREMENT_PROCESSING;
            }
        }

    }

neighbor_req_fail:
    return ret;

}

int rm_get_neighbor_report(struct rtl8192cd_priv *priv, unsigned char* result_buf)
{
    int len = -1;

    if ((OPMODE & (WIFI_STATION_STATE | WIFI_ASOC_STATE)) != (WIFI_STATION_STATE | WIFI_ASOC_STATE))
        goto neighbor_rep_fail;

    if(priv->pmib->dot11StationConfigEntry.dot11RadioMeasurementActivated &&
            priv->pmib->dot11StationConfigEntry.dot11RMNeighborReportActivated)
    {

        *result_buf = priv->rm.neighbor_result;
        len = 1;

        if(priv->rm.neighbor_result == MEASUREMENT_SUCCEED)
        {
            *(result_buf + len) = priv->rm.neighbor_report_num;
            len++;
            memcpy(result_buf + len, priv->rm.neighbor_report, priv->rm.neighbor_report_num * sizeof(struct dot11k_neighbor_report));
            len += priv->rm.neighbor_report_num * sizeof(struct dot11k_neighbor_report);
        }

        if(priv->rm.neighbor_result == MEASUREMENT_SUCCEED)
        {
            priv->rm.neighbor_result = MEASUREMENT_UNKNOWN;
        }
    }

neighbor_rep_fail:
    return len;

}

#endif

#ifdef CONFIG_RTL_PROC_NEW
int rtl8192cd_proc_neighbor_read(struct seq_file *s, void *data)
#else
int rtl8192cd_proc_neighbor_read(char *buf, char **start, off_t offset,
        int length, int *eof, void *data)
#endif
{
    struct net_device *dev = PROC_GET_DEV();
    struct rtl8192cd_priv *priv = GET_DEV_PRIV(dev);
    int pos = 0;
    int i,j;

    if((OPMODE & WIFI_AP_STATE) == 0)
    {
        panic_printk("\nwarning: invalid command!\n");
        return pos;
    }
    
    PRINT_ONE(" -- NEIGHBOR REPORT info -- ", "%s", 1);
    j = 1;
    for (i = 0 ; i < MAX_NEIGHBOR_REPORT; i++)
    {
        if((priv->rm_neighbor_bitmask[i>>3] & (1<<(i&7))) == 0)
            continue;

        PRINT_ONE(j, "  [%d]", 0);
        PRINT_ARRAY_ARG("BSSID: ", priv->rm_neighbor_report[i].bssid, "%02x", MACADDRLEN);
        PRINT_ONE("    Bss Info:", "%s", 0);
        PRINT_ONE(priv->rm_neighbor_report[i].bssinfo.value, "0x%04X", 1);
        PRINT_ONE("    Operating Class:", "%s", 0);
        PRINT_ONE(priv->rm_neighbor_report[i].op_class, "%d", 1);
        PRINT_ONE("    Channel:", "%s", 0);
        PRINT_ONE(priv->rm_neighbor_report[i].channel, "%d", 1); 
        PRINT_ONE("    Phy Type:", "%s", 0);
        PRINT_ONE(priv->rm_neighbor_report[i].phytype, "%d", 1);
#ifdef CONFIG_IEEE80211V
	 if(WNM_ENABLE) {
		PRINT_ONE("    Preference:", "%s", 0);
	 	PRINT_ONE(priv->rm_neighbor_report[i].subelemnt.preference, "%d", 1);	
	 }
#endif							
        PRINT_ONE("    SSID:", "%s", 0);
        PRINT_ONE(priv->rm_neighbor_info[i].ssid, "%s", 1);
        PRINT_ONE("    Aging:", "%s", 0);
        PRINT_ONE(priv->rm_neighbor_info[i].aging, "%ld", 1);
        PRINT_ONE("    Sta Count:", "%s", 0);
        PRINT_ONE(priv->rm_neighbor_info[i].bssload.sta_count, "%d", 1);
        PRINT_ONE("    Channel Utilization:", "%s", 0);
        PRINT_ONE(priv->rm_neighbor_info[i].bssload.channel_utilization, "%d", 1);        
        j++;
    }

    return pos;


}


#define NEIGHBOR_REPORT_PROC_LEN 100

#ifdef __ECOS
int rtl8192cd_proc_neighbor_write(char *tmp, void *data)
#else
int rtl8192cd_proc_neighbor_write(struct file *file, const char *buffer,
        unsigned long count, void *data)
#endif
{

#ifdef __ECOS
    return 0;
#else
    struct net_device *dev = (struct net_device *)data;
    struct rtl8192cd_priv *priv = GET_DEV_PRIV(dev);
    unsigned char error_code = 0;
    char * tokptr;
    int command = 0;
    int empty_slot;
    int i;
    char tmp[NEIGHBOR_REPORT_PROC_LEN];
    char *tmpptr;
    unsigned int oldest_aging;
    int oldest_idx;
    struct dot11k_neighbor_report report;
    char *ssid_ptr = NULL;
    unsigned short sta_count = 0;
    unsigned char channel_utilization = 0;

    if((OPMODE & WIFI_AP_STATE) == 0)
    {
        error_code = 1;
        goto end;
    }
    if (count < 2 || count >= NEIGHBOR_REPORT_PROC_LEN)
        return -EFAULT;

    if (buffer == NULL || copy_from_user(tmp, buffer, count))
        return -EFAULT;

    tmp[count] = 0;
    tmpptr = tmp;
    tmpptr = strsep((char **)&tmpptr, "\n");
    tokptr = strsep((char **)&tmpptr, " ");
    if(!memcmp(tokptr, "add", 3))
        command = 1;
    else if (!memcmp(tokptr, "delall", 6)) 
        command = 3;
    else if(!memcmp(tokptr, "del", 3))
        command = 2;

    if(command)
    {        
        if(command == 1 || command == 2) {
            tokptr = strsep((char **)&tmpptr," ");
            if(tokptr)
                get_array_val(report.bssid, tokptr, 12);
            else {
                error_code = 1;
                goto end;
            }
        }
        
        if(command == 1)   /*add*/
        {
            tokptr = strsep((char **)&tmpptr," ");
            if(tokptr)
            {
            	if ((!memcmp(tokptr, "0x", 2)) || (!memcmp(tokptr, "0X", 2)))            	           	
                    report.bssinfo.value = _atoi(tokptr+2, 16);            	
            	else            	
                    report.bssinfo.value = _atoi(tokptr, 10);            	         

            }
            else {
                error_code = 1;
                goto end;
            }

            tokptr = strsep((char **)&tmpptr," ");
            if(tokptr)
            {
                report.op_class = _atoi(tokptr, 10);

            }
            else {
                error_code = 1;
                goto end;
            }

            tokptr = strsep((char **)&tmpptr," ");
            if(tokptr)
            {
                report.channel = _atoi(tokptr, 10);

            }
            else {
                error_code = 1;
                goto end;
            }

            tokptr = strsep((char **)&tmpptr," ");
            if(tokptr)
            {
                report.phytype= _atoi(tokptr, 10);
            }
            else {
                error_code = 1;
                goto end;
            }


            while((tokptr = strsep((char **)&tmpptr," ")) != NULL) {
                if(0 == strcmp(tokptr, "ssid")) {
                    if(tmpptr[0] == '"') {
                        tmpptr++;
                        tokptr = strsep((char **)&tmpptr,"\"");
                        strsep((char **)&tmpptr," ");
                    }
                    else {
                        tokptr = strsep((char **)&tmpptr," ");
                    }
                    if(tokptr == NULL) {
                        error_code = 1;
                        goto end;
                    }
                ssid_ptr = tokptr;

                }
                else if(0 == strcmp(tokptr, "bssload")) {
                    tokptr = strsep((char **)&tmpptr," ");
                    if(tokptr == NULL) {
                        error_code = 1;
                        goto end;
                    }
                    sta_count = _atoi(tokptr, 10);
                    tokptr = strsep((char **)&tmpptr," ");
                    if(tokptr == NULL){
                        sta_count = channel_utilization = 0;
                        error_code = 1;
                        goto end;
                    }                            
                    channel_utilization= _atoi(tokptr, 10);
            }
            else {
                    error_code = 1;
                    goto end;
                }
                    
            }            
      
            for(i = 0, empty_slot = -1; i < MAX_NEIGHBOR_REPORT; i++)
            {
                if((priv->rm_neighbor_bitmask[i>>3] & (1<<(i&7))) == 0)
                {
                    if(empty_slot == -1)
                        empty_slot = i;
                }
                else if(0 == memcmp(report.bssid, priv->rm_neighbor_report[i].bssid, MACADDRLEN))
                {
                    break;
                }
            }
            if(i == MAX_NEIGHBOR_REPORT && empty_slot != -1)   /*not found, and has empty slot*/
            {
                i = empty_slot;
            }

            if(i == MAX_NEIGHBOR_REPORT)  /*not found and no empty slot, find the oldest neighbor and replace it*/
            {
                for(i = 0; i < MAX_NEIGHBOR_REPORT; i++)
                {
                    if(priv->rm_neighbor_bitmask[i>>3] & (1<<(i&7)))
                    {
                        if(oldest_aging < priv->rm_neighbor_info[i].aging) {
                            oldest_aging = priv->rm_neighbor_info[i].aging;
                            oldest_idx = i;
                        }
                    }
                }
                i = oldest_idx;
            
            }
            memcpy(&priv->rm_neighbor_report[i], &report, sizeof(struct dot11k_neighbor_report));
#ifdef CONFIG_IEEE80211V
	   	if(WNM_ENABLE) {
	   		priv->rm_neighbor_report[i].subelemnt.subelement_id = _WNM_BSS_TRANS_CANDIDATE_PREFRENCE_;
	   		priv->rm_neighbor_report[i].subelemnt.len = 1;
	   		priv->rm_neighbor_report[i].subelemnt.preference = getPreferredVal(priv, channel_utilization, 0, 0);
   		}
#endif		
            if(ssid_ptr)
               strcpy(priv->rm_neighbor_info[i].ssid, ssid_ptr);
            else
               priv->rm_neighbor_info[i].ssid[0] = 0;
            priv->rm_neighbor_info[i].aging = 0;
            priv->rm_neighbor_info[i].bssload.sta_count = sta_count;
            priv->rm_neighbor_info[i].bssload.channel_utilization = channel_utilization;            
            priv->rm_neighbor_bitmask[i>>3] |= (1<<(i&7));            
        }
        else if(command == 2)  /*delete*/
        {
            for (i = 0 ; i < MAX_NEIGHBOR_REPORT; i++)
            {
                if((priv->rm_neighbor_bitmask[i>>3] & (1<<(i&7))) == 0)
                    continue;
            
                if(0 == memcmp(report.bssid, priv->rm_neighbor_report[i].bssid, MACADDRLEN)) {
                    priv->rm_neighbor_bitmask[i>>3] &= ~(1<<(i&7));
                    break;
                }
            }
        }
        else if(command == 3)   /*delete all*/
        {
            memset(priv->rm_neighbor_bitmask, 0x00, sizeof(priv->rm_neighbor_bitmask));
        }
    }
    else
    {
        error_code = 1;
        goto end;
    }

end:
    if(error_code == 1)
        panic_printk("\nwarning: invalid command!\n");
    else if(error_code == 2)
        panic_printk("\nwarning: neighbor report table full!\n");
    return count;
#endif
}
#endif
