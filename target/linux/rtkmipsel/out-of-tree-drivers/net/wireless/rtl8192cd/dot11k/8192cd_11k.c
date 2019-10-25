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

#define _8192CD_11K_C_

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
const unsigned char wfa_oui[] = {0x00, 0x50, 0xf2};

static unsigned char * construct_measure_report_ie(struct rtl8192cd_priv *priv, unsigned char	*pbuf, unsigned int *frlen, int index)
{
    unsigned char temp[3];
    temp[0] = priv->rm.measure_token[index];
    if(priv->rm.measure_result[index] == MEASUREMENT_REFUSED)
    {
        temp[1] = BIT2;
    }
    else if(priv->rm.measure_result[index] == MEASUREMENT_INCAPABLE)
    {
        temp[1] = BIT1;
    }
    else
    {
        temp[1] = 0;
    }
    temp[2] = priv->rm.measure_type[index];
    pbuf = set_ie(pbuf, _MEASUREMENT_REPORT_IE_, 3, temp, frlen);
    return pbuf;

}

unsigned char * construct_rm_enable_cap_ie(struct rtl8192cd_priv *priv, unsigned char	*pbuf, unsigned int *frlen)
{
    unsigned char temp[5];
    temp[0] = temp[1] = temp[2] = temp[3] = temp[4] = 0;
    if(priv->pmib->dot11StationConfigEntry.dot11RMLinkMeasurementActivated)
    {
        temp[0] |= BIT0;
    }

    if(priv->pmib->dot11StationConfigEntry.dot11RMNeighborReportActivated)
    {
        temp[0] |= BIT1;
    }

    if(priv->pmib->dot11StationConfigEntry.dot11RMBeaconPassiveMeasurementActivated)
    {
        temp[0] |= BIT4;
    }

    if(priv->pmib->dot11StationConfigEntry.dot11RMBeaconActiveMeasurementActivated)
    {
        temp[0] |= BIT5;
    }

    if(priv->pmib->dot11StationConfigEntry.dot11RMBeaconTableMeasurementActivated)
    {
        temp[0] |= BIT6;
    }

    if(priv->pmib->dot11StationConfigEntry.dot11RMAPChannelReportActivated)
    {
        temp[2] |= BIT0;
    }

    pbuf = set_ie(pbuf, _RM_ENABLE_CAP_IE_, 5, temp, frlen);
    return pbuf;
}

static int __issue_measurement_report(struct rtl8192cd_priv *priv, struct stat_info* pstat, int *measure_index)
{
    unsigned char   *pbuf;
    unsigned int frlen;
    unsigned char full = 0;
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
        goto issue_measure_report_fail;

    txinsn.phdr = get_wlanhdr_from_poll(priv);
    if (txinsn.phdr == NULL)
        goto issue_measure_report_fail;

    memset((void *)(txinsn.phdr), 0, sizeof(struct wlan_hdr));

    pbuf[0] = _RADIO_MEASUREMENT_CATEGORY_ID_;
    pbuf[1] = _RADIO_MEASUREMENT_REPORT_ACTION_ID_;
    pbuf[2] = priv->rm.dialog_token;

    frlen = 3;
    pbuf += frlen;
    while(*measure_index < priv->rm.measure_count)
    {
        if(priv->rm.measure_result[*measure_index] == MEASUREMENT_REFUSED||
                priv->rm.measure_result[*measure_index] == MEASUREMENT_INCAPABLE)
        {

            if(frlen + 5 > MAX_REPORT_FRAME_SIZE)
            {
                break;
            }
            pbuf = construct_measure_report_ie(priv, pbuf, &frlen, *measure_index);
        }
        else if(priv->rm.measure_result[*measure_index] == MEASUREMENT_SUCCEED)
        {
            switch(priv->rm.measure_type[*measure_index])
            {
                case MEASUREMENT_TYPE_BEACON:
                    pbuf = construct_beacon_report_ie(priv, pbuf, &frlen, priv->rm.measure_token[*measure_index], &full);
                    break;
                default:
                    break;
            }

            if(full)
            {
                break;
            }
        }

        (*measure_index)++;
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

#if defined(WIFI_WMM)
    ret = check_dz_mgmt(priv, pstat, &txinsn);
    if (ret < 0)
        goto issue_measure_report_fail;
    else if (ret==1)
        return 0;
    else
#endif
    if ((rtl8192cd_firetx(priv, &txinsn)) == SUCCESS)
        return 0;

issue_measure_report_fail:

    if (txinsn.phdr)
        release_wlanhdr_to_poll(priv, txinsn.phdr);
    if (txinsn.pframe)
        release_mgtbuf_to_poll(priv, txinsn.pframe);
    return -1;
}

static void issue_measurement_report(struct rtl8192cd_priv *priv, struct stat_info* pstat)
{
    int measure_index = 0;
    while(measure_index < priv->rm.measure_count)
    {
        __issue_measurement_report(priv, pstat, &measure_index);
    }

}

static void rm_check_result(struct rtl8192cd_priv *priv)
{
    unsigned int rand_time;
    unsigned char need_process = 0;
    int i;
    for(i = 0; i < priv->rm.measure_count; i++)
    {
        if(priv->rm.measure_result[i] == MEASUREMENT_PROCESSING)
        {
            need_process = 1;
            break;
        }
    }

    if(need_process)
    {
        if(priv->rm.measure_interval[i])   /*need random delay*/
        {
            SET_PSEUDO_RANDOM_NUMBER(rand_time);
            rand_time %= priv->rm.measure_interval[i];
            mod_timer(&priv->rm.delay_timer,jiffies + RTL_MILISECONDS_TO_JIFFIES(rand_time));
        }
        else
        {
            rm_do_next_measure((unsigned long)priv);
        }
    }
    else
    {
        /* all measurement is finished*/
        issue_measurement_report(priv, priv->rm.req_pstat);
        priv->rm.req_pstat = NULL;
    }

}



void rm_done(struct rtl8192cd_priv *priv, unsigned char measure_type, unsigned char check_result)
{
    int i;
    for(i = 0; i < priv->rm.measure_count; i++)
    {
        if(measure_type == priv->rm.measure_type[i])
        {
            priv->rm.measure_result[i] = MEASUREMENT_SUCCEED;
            break;
        }
    }

    if(check_result)
        rm_check_result(priv);
}
void rm_terminate(struct rtl8192cd_priv *priv)
{
    int i;
    if (timer_pending(&priv->rm.delay_timer))
        del_timer_sync(&priv->rm.delay_timer);

    for(i = 0; i < priv->rm.measure_count; i++)
    {
        if(priv->rm.measure_result[i] == MEASUREMENT_PROCESSING)
        {
            switch(priv->rm.measure_type[i])
            {
                case MEASUREMENT_TYPE_BEACON:
                    priv->rm.measure_result[i] = rm_terminate_beacon_measure(priv);
                    break;
                default:
                    priv->rm.measure_result[i] = MEASUREMENT_UNKNOWN;
                    break;
            }
        }
    }
}



void rm_do_next_measure(unsigned long task_priv)
{
#ifndef SMP_SYNC
    unsigned long flag;
#endif
    int i;
    struct rtl8192cd_priv *priv = (struct rtl8192cd_priv *)task_priv;

#ifdef SMP_SYNC
    SAVE_INT_AND_CLI(priv->pshare->irq_save);
    SMP_LOCK(priv->pshare->irq_save);
#else
    SAVE_INT_AND_CLI(flag);
#endif

    for(i = 0; i < priv->rm.measure_count; i++)
    {
        if(priv->rm.measure_result[i] == MEASUREMENT_PROCESSING)
        {
            switch(priv->rm.measure_type[i])
            {
                case MEASUREMENT_TYPE_BEACON:
                    priv->rm.measure_result[i] = rm_do_beacon_measure(priv);
                    break;
                default:
                    break;
            }

            if(priv->rm.measure_result[i] == MEASUREMENT_PROCESSING)
            {
                /*some measurement is processing*/
                break;
            }
        }
    }

    if(i >= priv->rm.measure_count)
    {
        /* all measurement is finished*/
        issue_measurement_report(priv, priv->rm.req_pstat);
        priv->rm.req_pstat = NULL;
    }

#ifdef SMP_SYNC
    RESTORE_INT(priv->pshare->irq_save);
    SMP_UNLOCK(priv->pshare->irq_save);
#else
    RESTORE_INT(flag);
#endif

}

void OnRadioMeasurementRequest(struct rtl8192cd_priv *priv, struct stat_info *pstat, unsigned char *pframe,
                               int frame_len)
{
    int len;
    unsigned char element_id;
    unsigned char element_len;
    unsigned char incapable_all = 0;
    enum MEASUREMENT_RESULT result = MEASUREMENT_PROCESSING;

    /*if previous measurement is on-going*/
    if(priv->rm.req_pstat)
    {
        if(priv->rm.req_pstat != pstat || priv->rm.dialog_token != pframe[2])
        {
            rm_terminate(priv);
            issue_measurement_report(priv, priv->rm.req_pstat);
            priv->rm.req_pstat = NULL;
        }
        else
            return;
    }


    /*parsing radio measurment request frame*/
    priv->rm.req_pstat = pstat;
    priv->rm.dialog_token = pframe[2]; /*dialog token*/
    if(pframe[3] != 0 || pframe[4] != 0)  /* repetition != 0 */
    {
        incapable_all = 1 ;
    }
    len = 5;

    priv->rm.measure_count = 0;
    while(len + 2 <= frame_len)
    {
        element_id = pframe[len];
        element_len = pframe[len + 1];

        /*parsing every radio measurment request element*/
        if(element_id == _MEASUREMENT_REQUEST_IE_)
        {
            result = MEASUREMENT_PROCESSING;
            priv->rm.measure_token[priv->rm.measure_count] = pframe[len + 2];
            priv->rm.measure_type[priv->rm.measure_count] = pframe[len + 4];

            if(pframe[len + 3] || incapable_all) /*check measurement mode*/
            {
                result = MEASUREMENT_INCAPABLE;
            }
            else
            {
                switch(priv->rm.measure_type[priv->rm.measure_count])
                {
                    case MEASUREMENT_TYPE_BEACON:
                        result = rm_parse_beacon_request(priv, pframe, len + 5, len + 2 + element_len, &priv->rm.measure_interval[priv->rm.measure_count]);
                        break;
                    default:
                        result = MEASUREMENT_INCAPABLE;
                        break;

                }
            }

            priv->rm.measure_result[priv->rm.measure_count] = result;
            priv->rm.measure_count++;
            if(priv->rm.measure_count >= MAX_MEASUREMENT_REQUEST)
                break;
        }
        len += 2 + element_len;
    }

    rm_check_result(priv);
    return;
}



void OnRadioMeasurementReport(struct rtl8192cd_priv *priv, struct stat_info *pstat,
                              unsigned char *pframe, int frame_len)
{
    int len;
    unsigned char element_id;
    unsigned char element_len;

    /*the report from iphone 6 always has dialog token 0, thus do not check dialog token when the value is 0*/
    if((pframe[2] != 0 && pstat->rm.dialog_token != pframe[2]) || pstat->rm.measure_result != MEASUREMENT_PROCESSING)
    {
        return;
    }

    len = 3;
    while(len + 5 <= frame_len)
    {
        element_id = pframe[len];
        element_len = pframe[len + 1];
        /*parsing every radio measurment report element*/
        if(element_id == _MEASUREMENT_REPORT_IE_)
        {
            if(pframe[len + 4] == MEASUREMENT_TYPE_BEACON)
            {
                len = rm_parse_beacon_report(pstat, pframe, len, frame_len);
                continue;
            }
        }
        len += 2 + element_len;
    }


}

#ifdef CLIENT_MODE
unsigned char * construct_WFA_TPC_report_ie(struct rtl8192cd_priv *priv, unsigned char	*pbuf, unsigned int *frlen)
{
    unsigned char temp[7];    
    memcpy(temp, wfa_oui, 3); /*oui*/    
    temp[3] = 0x08; /*oui type */    
    temp[4] = 0x00; /*oui subtype */    
    temp[5] = priv->pmib->dot11hTPCEntry.tpc_tx_power; /*transmit power*/    
    temp[6] = 0;    /*link margin, should be 0*/
    pbuf = set_ie(pbuf, _VENDOR_SPEC_IE_, 7, temp, frlen);
    return pbuf;
}
#endif

#endif
