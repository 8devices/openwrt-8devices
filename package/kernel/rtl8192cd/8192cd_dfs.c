/*
 *   Handling routines for DFS (Dynamic Frequency Selection) functions
 *
 *  $Id: 8192cd_dfs.c,v 1.1 2012/05/04 12:49:07 jimmylin Exp $
 *
 *  Copyright (c) 2012 Realtek Semiconductor Corp.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 */

#define _8192CD_DFS_C_

#include "./8192cd_cfg.h"
#include "./8192cd.h"
#include "./8192cd_headers.h"
#include "./8192cd_debug.h"
#ifdef RTK_NL80211
#include "8192cd_cfg80211.h" 
#endif

#ifdef DFS


#define DFS_VERSION		"2.0.14"


void rtl8192cd_dfs_chk_timer(unsigned long task_priv)
{
	struct rtl8192cd_priv *priv = (struct rtl8192cd_priv *)task_priv;

	if (!(priv->drv_state & DRV_STATE_OPEN))
		return;
	
	if (timer_pending(&priv->dfs_chk_timer)){
		del_timer_sync(&priv->dfs_chk_timer);
	}
	
	if (GET_CHIP_VER(priv) == VERSION_8192D)
		PHY_SetBBReg(priv, 0xcdc, BIT(8)|BIT(9), 1);
	PRINT_INFO("DFS CP END.\n");
}

void set_CHXX_timer(struct rtl8192cd_priv *priv, unsigned int channel)
{
	switch(channel) {
		case 52:
			mod_timer(&priv->ch52_timer, jiffies + NONE_OCCUPANCY_PERIOD);
			break;
		case 56:
			mod_timer(&priv->ch56_timer, jiffies + NONE_OCCUPANCY_PERIOD);
			break;
		case 60:
			mod_timer(&priv->ch60_timer, jiffies + NONE_OCCUPANCY_PERIOD);
			break;
		case 64:
			mod_timer(&priv->ch64_timer, jiffies + NONE_OCCUPANCY_PERIOD);
			break;
		case 100:
			mod_timer(&priv->ch100_timer, jiffies + NONE_OCCUPANCY_PERIOD);
			break;
		case 104:
			mod_timer(&priv->ch104_timer, jiffies + NONE_OCCUPANCY_PERIOD);
			break;
		case 108:
			mod_timer(&priv->ch108_timer, jiffies + NONE_OCCUPANCY_PERIOD);
			break;
		case 112:
			mod_timer(&priv->ch112_timer, jiffies + NONE_OCCUPANCY_PERIOD);
			break;
		case 116:
			mod_timer(&priv->ch116_timer, jiffies + NONE_OCCUPANCY_PERIOD);
			break;
		case 120:
			mod_timer(&priv->ch120_timer, jiffies + NONE_OCCUPANCY_PERIOD);
			break;
		case 124:
			mod_timer(&priv->ch124_timer, jiffies + NONE_OCCUPANCY_PERIOD);
			break;
		case 128:
			mod_timer(&priv->ch128_timer, jiffies + NONE_OCCUPANCY_PERIOD);
			break;
		case 132:
			mod_timer(&priv->ch132_timer, jiffies + NONE_OCCUPANCY_PERIOD);
			break;
		case 136:
			mod_timer(&priv->ch136_timer, jiffies + NONE_OCCUPANCY_PERIOD);
			break;
		case 140:
			mod_timer(&priv->ch140_timer, jiffies + NONE_OCCUPANCY_PERIOD);
			break;
		case 144:
			mod_timer(&priv->ch144_timer, jiffies + NONE_OCCUPANCY_PERIOD);
			break;
		default:
			DEBUG_ERR("DFS_timer: Channel match none!\n");
			break;
	}
}

void rtl8192cd_DFS_TXPAUSE_timer(unsigned long task_priv)
{
	struct rtl8192cd_priv *priv = (struct rtl8192cd_priv *)task_priv;
	unsigned int which_channel;

	printk("rtl8192cd_DFS_TXPAUSE_timer\n");

	if (!(priv->drv_state & DRV_STATE_OPEN))
		return;
	
	if (priv->available_chnl_num != 0) {
		if (timer_pending(&priv->DFS_TXPAUSE_timer)) 
			del_timer_sync(&priv->DFS_TXPAUSE_timer);
		
		printk("rtl8192cd_DFS_TXPAUSE_timer PATH2\n");
#if defined(UNIVERSAL_REPEATER)
		if (!under_apmode_repeater(priv))
#endif
		{

		/* select a channel */
		which_channel = DFS_SelectChannel(priv);

		priv->pshare->dfsSwitchChannel = which_channel;

		if (priv->pshare->dfsSwitchChannel == 0) {
			panic_printk("It should not run to here\n");
			return;
		}
		else
			priv->pmib->dot11DFSEntry.DFS_detected = 1;
		panic_printk("rtl8192cd_DFS_TXPAUSE_timer,dfsSwitchChannel=%d\n",priv->pshare->dfsSwitchChannel);
#ifdef MBSSID
		if (priv->pmib->miscEntry.vap_enable)
			priv->pshare->dfsSwitchChCountDown = 6;
		else
#endif
			priv->pshare->dfsSwitchChCountDown = 5;
		}

		if (priv->pmib->dot11StationConfigEntry.dot11DTIMPeriod >= priv->pshare->dfsSwitchChCountDown)
			priv->pshare->dfsSwitchChCountDown = priv->pmib->dot11StationConfigEntry.dot11DTIMPeriod+1;

	}
	else {
		printk("rtl8192cd_DFS_TXPAUSE_timer PATH3\n");
		mod_timer(&priv->DFS_TXPAUSE_timer, jiffies + DFS_TXPAUSE_TO);
	}

		
}

void dfs_histogram_radar_distinguish(struct rtl8192cd_priv *priv)
{
	unsigned char dfs_plusewidth_thd1=0, dfs_plusewidth_thd2=0, dfs_plusewidth_thd3=0, dfs_plusewidth_thd4=0, dfs_plusewidth_thd5=0;
	unsigned int dfs_hist1_peak_index=0, dfs_hist2_peak_index=0, dfs_hist1_pulse_width=0, dfs_hist2_pulse_width=0;
	unsigned int i=0,j=0,g_plusewidth[6]={0},g_peakindex[16]={0},g_mask_32=0;
	unsigned int false_peak_hist1=0, false_peak_hist2_above10=0, false_peak_hist2_above0=0;
	unsigned int dfs_hist1_pulse_interval=0, dfs_hist2_pulse_interval=0, g_pluseinterval[6]={0}, pulsewidth_sum_g0g5=0, pulsewidth_sum_g1g2g3g4=0, pulseinterval_sum_g0g5=0, pulseinterval_sum_g1g2g3g4=0, pulsewidth_sum_ss_g1g2g3g4=0, pulseinterval_sum_ss_g1g2g3g4=0, max_pri_cnt=0, max_pw_cnt=0;
	unsigned char dfs_pluseinterval_thd1=0, dfs_pluseinterval_thd2=0, dfs_pluseinterval_thd3=0, dfs_pluseinterval_thd4=0, dfs_pluseinterval_thd5=0, pri_th=0, max_pri_idx=0, max_pw_idx=0, max_pri_cnt_th=0, max_pri_cnt_fcc_g1_th=0, max_pri_cnt_fcc_g3_th =0, safe_pri_width_diff_th=0, safe_pri_width_diff_fcc_th=0, safe_pri_width_diff_w53_th=0, safe_pri_width_diff_fcc_idle_th=0;
		
		/*read peak index hist report*/
		PHY_SetBBReg(priv, 0x19e4, BIT(22)|BIT(23), 0x0);
		dfs_hist1_peak_index = PHY_QueryBBReg(priv, 0xf5c, 0xffffffff);
		dfs_hist2_peak_index = PHY_QueryBBReg(priv, 0xf74, 0xffffffff);
				
		g_peakindex[15] = ((dfs_hist1_peak_index & 0x0000000f)>>0);
		g_peakindex[14] = ((dfs_hist1_peak_index & 0x000000f0)>>4);
		g_peakindex[13] = ((dfs_hist1_peak_index & 0x00000f00)>>8);
		g_peakindex[12] = ((dfs_hist1_peak_index & 0x0000f000)>>12);
		g_peakindex[11] = ((dfs_hist1_peak_index & 0x000f0000)>>16);
		g_peakindex[10] = ((dfs_hist1_peak_index & 0x00f00000)>>20);
		g_peakindex[9] = ((dfs_hist1_peak_index & 0x0f000000)>>24);
		g_peakindex[8] = ((dfs_hist1_peak_index & 0xf0000000)>>28);			
		g_peakindex[7] = ((dfs_hist2_peak_index & 0x0000000f)>>0);
		g_peakindex[6] = ((dfs_hist2_peak_index & 0x000000f0)>>4);
		g_peakindex[5] = ((dfs_hist2_peak_index & 0x00000f00)>>8);
		g_peakindex[4] = ((dfs_hist2_peak_index & 0x0000f000)>>12);
		g_peakindex[3] = ((dfs_hist2_peak_index & 0x000f0000)>>16);
		g_peakindex[2] = ((dfs_hist2_peak_index & 0x00f00000)>>20);
		g_peakindex[1] = ((dfs_hist2_peak_index & 0x0f000000)>>24);
		g_peakindex[0] = ((dfs_hist2_peak_index & 0xf0000000)>>28);

		/*read pulse width hist report*/
		PHY_SetBBReg(priv, 0x19e4, BIT(22)|BIT(23), 0x1);
		dfs_hist1_pulse_width = PHY_QueryBBReg(priv, 0xf5c, 0xffffffff);
		dfs_hist2_pulse_width = PHY_QueryBBReg(priv, 0xf74, 0xffffffff);
		
		g_plusewidth[0] = (unsigned int)((dfs_hist2_pulse_width & 0xff000000)>>24);
		g_plusewidth[1] = (unsigned int)((dfs_hist2_pulse_width & 0x00ff0000)>>16);
		g_plusewidth[2] = (unsigned int)((dfs_hist2_pulse_width & 0x0000ff00)>>8);
		g_plusewidth[3] = (unsigned int)dfs_hist2_pulse_width & 0x000000ff;
		g_plusewidth[4] = (unsigned int)((dfs_hist1_pulse_width & 0xff000000)>>24);
		g_plusewidth[5] = (unsigned int)((dfs_hist1_pulse_width & 0x00ff0000)>>16);
		
		/*read pulse repetition interval hist report*/
		PHY_SetBBReg(priv, 0x19e4, BIT(22)|BIT(23), 0x3);
		dfs_hist1_pulse_interval = PHY_QueryBBReg(priv, 0xf5c, 0xffffffff);
		dfs_hist2_pulse_interval = PHY_QueryBBReg(priv, 0xf74, 0xffffffff);
		PHY_SetBBReg(priv, 0x19b4, 0x10000000, 1);  // reset histogram report
		PHY_SetBBReg(priv, 0x19b4, 0x10000000, 0);  // continue histogram report
				
		g_pluseinterval[0] = (unsigned int)((dfs_hist2_pulse_interval & 0xff000000)>>24);
		g_pluseinterval[1] = (unsigned int)((dfs_hist2_pulse_interval & 0x00ff0000)>>16);
		g_pluseinterval[2] = (unsigned int)((dfs_hist2_pulse_interval & 0x0000ff00)>>8);
		g_pluseinterval[3] = (unsigned int)dfs_hist2_pulse_interval & 0x000000ff;
		g_pluseinterval[4] = (unsigned int)((dfs_hist1_pulse_interval & 0xff000000)>>24);
		g_pluseinterval[5] = (unsigned int)((dfs_hist1_pulse_interval & 0x00ff0000)>>16);
			
		priv->pri_cond1 = 0;
		priv->pri_cond2 = 0;
		priv->pri_cond3 = 0;
		priv->pri_cond4 = 0;
		priv->pri_cond5 = 0;
		priv->pw_cond1 = 0;
		priv->pw_cond2 = 0;
		priv->pw_cond3 = 0;
		priv->pri_type3_4_cond1 = 0;	//for ETSI
		priv->pri_type3_4_cond2 = 0;	//for ETSI
		priv->pw_long_cond1 = 0;	//for long radar
		priv->pw_long_cond2 = 0;	//for long radar
		priv->pri_long_cond1 = 0;	//for long radar
		priv->pulsewidth_flag = 0;
		priv->pulseinterval_flag = 0;
		priv->pri_type3_4_flag = 0;	//for ETSI
		priv->long_radar_flag = 0;
		priv->pw_std = 0;	//The std(variance) of reasonable num of pw group
		priv->pri_std = 0;	//The std(variance) of reasonable num of pri group

		for (i=0; i<6; i++){
			priv->plusewidth_hold_sum[i] = 0;
			priv->pluseinterval_hold_sum[i] = 0;
			priv->plusewidth_long_hold_sum[i] = 0;
			priv->pluseinterval_long_hold_sum[i] = 0;
		}

		if (priv->idle_flag == 1){
			pri_th = priv->pshare->rf_ft_var.interval_hist_th;			
		}
		else{
			pri_th = priv->pshare->rf_ft_var.interval_hist_th -1;
		}
		
		for (i=0; i<6; i++){
			priv->plusewidth_hold[priv->hist_idx][i] = g_plusewidth[i];
			priv->pluseinterval_hold[priv->hist_idx][i] = g_pluseinterval[i];
			/*collect whole histogram report may take some time, so we add the counter of 2 time slots in FCC and ETSI*/
			if (priv->pshare->rf_ft_var.manual_dfs_regdomain == 1 || priv->pshare->rf_ft_var.manual_dfs_regdomain == 3){
				priv->plusewidth_hold_sum[i] = priv->plusewidth_hold_sum[i] + priv->plusewidth_hold[(priv->hist_idx+1)%3][i] + priv->plusewidth_hold[(priv->hist_idx+2)%3][i];					
				priv->pluseinterval_hold_sum[i] = priv->pluseinterval_hold_sum[i] + priv->pluseinterval_hold[(priv->hist_idx+1)%3][i] + priv->pluseinterval_hold[(priv->hist_idx+2)%3][i];
			}	
			/*collect whole histogram report may take some time, so we add the counter of 3 time slots in MKK or else*/
			else{
				priv->plusewidth_hold_sum[i] = priv->plusewidth_hold_sum[i] + priv->plusewidth_hold[(priv->hist_idx+1)%4][i] + priv->plusewidth_hold[(priv->hist_idx+2)%4][i] + priv->plusewidth_hold[(priv->hist_idx+3)%4][i];					
				priv->pluseinterval_hold_sum[i] = priv->pluseinterval_hold_sum[i] + priv->pluseinterval_hold[(priv->hist_idx+1)%4][i] + priv->pluseinterval_hold[(priv->hist_idx+2)%4][i] + priv->pluseinterval_hold[(priv->hist_idx+3)%4][i];
			}
			
		}
		/*For long radar type*/
		for (i=0; i<6; i++){

			priv->plusewidth_long_hold[priv->hist_long_idx][i] = g_plusewidth[i];
			priv->pluseinterval_long_hold[priv->hist_long_idx][i] = g_pluseinterval[i];
			/*collect whole histogram report may take some time, so we add the counter of 299 time slots for long radar*/
			for (j=1; j<300; j++){
			priv->plusewidth_long_hold_sum[i] = priv->plusewidth_long_hold_sum[i] + priv->plusewidth_long_hold[(priv->hist_long_idx+j)%300][i];					
			priv->pluseinterval_long_hold_sum[i] = priv->pluseinterval_long_hold_sum[i] + priv->pluseinterval_long_hold[(priv->hist_long_idx+j)%300][i];

			}
		}
		priv->hist_idx++;
		priv->hist_long_idx++;
		if (priv->hist_long_idx == 300)
			priv->hist_long_idx = 0;
		if (priv->pshare->rf_ft_var.manual_dfs_regdomain == 1 || priv->pshare->rf_ft_var.manual_dfs_regdomain == 3){
			if (priv->hist_idx == 3)
				priv->hist_idx = 0;
		}
		else if (priv->hist_idx == 4){
			priv->hist_idx = 0;
		}
		
		max_pri_cnt = 0;
		max_pri_idx = 0;
		max_pw_cnt = 0;
		max_pw_idx = 0;
		max_pri_cnt_th = priv->pshare->rf_ft_var.pri_sum_g1_th;
		max_pri_cnt_fcc_g1_th = priv->pshare->rf_ft_var.pri_sum_g1_fcc_th;
		max_pri_cnt_fcc_g3_th = priv->pshare->rf_ft_var.pri_sum_g3_fcc_th;
		safe_pri_width_diff_th = priv->pshare->rf_ft_var.pri_width_diff_th;
		safe_pri_width_diff_fcc_th = priv->pshare->rf_ft_var.pri_width_diff_fcc_th;
		safe_pri_width_diff_fcc_idle_th = priv->pshare->rf_ft_var.pri_width_diff_fcc_idle_th;
		safe_pri_width_diff_w53_th = priv->pshare->rf_ft_var.pri_width_diff_w53_th;

		/*g1 to g4 is the reseasonable range of pri and pw*/
		for (i=1; i<=4; i++){
			if (priv->pluseinterval_hold_sum[i]>max_pri_cnt){
				max_pri_cnt = priv->pluseinterval_hold_sum[i];
				max_pri_idx = i;
			}
			if (priv->plusewidth_hold_sum[i]>max_pw_cnt){
				max_pw_cnt = priv->plusewidth_hold_sum[i];
				max_pw_idx = i;
			}			
			if (priv->pluseinterval_hold_sum[i]>=pri_th)
				priv->pri_cond1 = 1;
		}		

		pulseinterval_sum_g0g5 = priv->pluseinterval_hold_sum[0];
		if (pulseinterval_sum_g0g5 == 0)
			pulseinterval_sum_g0g5 = 1;
		pulseinterval_sum_g1g2g3g4 = priv->pluseinterval_hold_sum[1] + priv->pluseinterval_hold_sum[2] + priv->pluseinterval_hold_sum[3] + priv->pluseinterval_hold_sum[4];
		
		/*pw will reduce because of dc, so we do not treat g0 as illegal group*/
		pulsewidth_sum_g0g5 = priv->plusewidth_hold_sum[5];				
		if (pulsewidth_sum_g0g5 == 0)
			pulsewidth_sum_g0g5 = 1;
		pulsewidth_sum_g1g2g3g4 = priv->plusewidth_hold_sum[1] + priv->plusewidth_hold_sum[2] + priv->plusewidth_hold_sum[3] + priv->plusewidth_hold_sum[4];

		/*Calculate the variation from g1 to g4*/
		for(i=1; i<5; i++){	
			/*Sum of square*/
			pulsewidth_sum_ss_g1g2g3g4 = pulsewidth_sum_ss_g1g2g3g4 + (priv->plusewidth_hold_sum[i] - (pulsewidth_sum_g1g2g3g4/4))*(priv->plusewidth_hold_sum[i] - (pulsewidth_sum_g1g2g3g4/4));
			pulseinterval_sum_ss_g1g2g3g4 = pulseinterval_sum_ss_g1g2g3g4 + (priv->pluseinterval_hold_sum[i] - (pulseinterval_sum_g1g2g3g4/4))*(priv->pluseinterval_hold_sum[i] - (pulseinterval_sum_g1g2g3g4/4));
		}
			priv->pw_std = (pulsewidth_sum_ss_g1g2g3g4/4);		//The value may less than the normal variance, since the variable type is int (not float)
			priv->pri_std= (pulseinterval_sum_ss_g1g2g3g4/4);	//The value may less than the normal variance, since the variable type is int (not float)
			
		if (priv->pshare->rf_ft_var.manual_dfs_regdomain == 1) {
			priv->pri_type3_4_flag = 1;	// ETSI flag
			
			/*PRI judgment conditions for short radar type*/
			if ((pulseinterval_sum_g0g5 + pulseinterval_sum_g1g2g3g4) / pulseinterval_sum_g0g5 > 2 && priv->pri_std >= priv->pshare->rf_ft_var.pri_std_th)
				priv->pri_cond2 = 1;	// ratio of reasonable group and illegal group && pri variation of short radar should be large (=6)

			if (max_pw_idx == 1 && max_pri_idx == 3){
				if (priv->pluseinterval_hold_sum[3] >= priv->pshare->rf_ft_var.pri_type1_low_fcc_th && priv->pluseinterval_hold_sum[3] <= priv->pshare->rf_ft_var.pri_type1_upp_fcc_th){
					priv->pri_cond3 = 1;	// To distinguish between type 1 radar and other radars
				}
			}

			if (max_pw_idx == 2 && max_pri_idx == 3){
				if (priv->pluseinterval_hold_sum[3] <= priv->pshare->rf_ft_var.pri_type1_cen_fcc_th){
					priv->pri_cond3 = 1;	// To distinguish between type 1 radar and other radars in center freq
				}
			}
			
			else if (pulseinterval_sum_g1g2g3g4 <= priv->pshare->rf_ft_var.pri_sum_safe_fcc_th){
				priv->pri_cond3 = 1;	//  reasonable group shouldn't too large
			}
			
			if (priv->idle_flag == 1){	// the difference between pri and pw for idle mode (thr=2)
				if (abs(pulsewidth_sum_g1g2g3g4 - pulseinterval_sum_g1g2g3g4) <= safe_pri_width_diff_fcc_idle_th)
					priv->pri_cond4 = 1;			
			}
			else{	// the difference between pri and pw for throughput mode (thr=8)
				if (abs(pulsewidth_sum_g1g2g3g4 - pulseinterval_sum_g1g2g3g4) <= safe_pri_width_diff_fcc_th)
					priv->pri_cond4 = 1;
			}

			if (max_pri_idx == 1){
				if (max_pri_cnt >= max_pri_cnt_fcc_g1_th){
					priv->pri_cond5 = 1;	//In FCC, we set threshold = 7 (>pri_th : which is 4) for distinguishing type 2 (g1) 
				}
			}
			else if (max_pri_idx == 3){
				if (max_pri_cnt >= max_pri_cnt_fcc_g3_th){
					priv->pri_cond5 = 1;	//In FCC, we set threshold = 7 (>pri_th : which is 4) for distinguishing type 3,4 (g3) 
				}
			}
			else{
				priv->pri_cond5 = 1;
			}
			
			if(priv->pri_cond1 && priv->pri_cond2 && priv->pri_cond3 && priv->pri_cond4 && priv->pri_cond5)
				priv->pulseinterval_flag = 1;

			/* PW judgment conditions for short radar type */
			if (((pulsewidth_sum_g0g5 + pulsewidth_sum_g1g2g3g4) / pulsewidth_sum_g0g5 > 2) && (priv->plusewidth_hold_sum[5] <= 1))
				priv->pw_cond1 = 1;	// ratio of reasonable group and illegal group && g5 should be zero
			if (priv->plusewidth_hold_sum[0] <= priv->pshare->rf_ft_var.pw_g0_th)
				priv->pw_cond2 = 1;	// unreasonable group shouldn't too large
			if (priv->pw_std >= priv->pshare->rf_ft_var.pw_std_th)
				priv->pw_cond3 = 1;	// pw's std (short radar) should be large (=9)
			if(priv->pw_cond1 && priv->pw_cond2 && priv->pw_cond3)
				priv->pulsewidth_flag = 1;
			
			/* Judgment conditions of long radar type */
			if (priv->pmib->dot11nConfigEntry.dot11nUse40M == HT_CHANNEL_WIDTH_20){
				if (priv->plusewidth_long_hold_sum[4] >= priv->pshare->rf_ft_var.pulsewidth_long_lower_20m_th)
					priv->pw_long_cond1 = 1;	
			}
			else{
				if (priv->plusewidth_long_hold_sum[4] >= priv->pshare->rf_ft_var.pulsewidth_long_lower_th)
					priv->pw_long_cond1 = 1;
			}
			if (priv->plusewidth_long_hold_sum[1] + priv->plusewidth_long_hold_sum[2] + priv->plusewidth_long_hold_sum[3] + priv->plusewidth_long_hold_sum[4] <= priv->pshare->rf_ft_var.pulsewidth_long_sum_upper_th)
				priv->pw_long_cond2 = 1;
			if (priv->pluseinterval_long_hold_sum[4] <= priv->pshare->rf_ft_var.pri_long_upper_th)	//g4 should be large for long radar
				priv->pri_long_cond1 = 1;
			if(priv->pw_long_cond1 && priv->pw_long_cond2 && priv->pri_long_cond1)
				priv->long_radar_flag = 1;
		}
		else if (priv->pshare->rf_ft_var.manual_dfs_regdomain == 2) {
			priv->pri_type3_4_flag = 1;	// ETSI flag
			
			/*PRI judgment conditions for short radar type*/
			if ((pulseinterval_sum_g0g5 + pulseinterval_sum_g1g2g3g4) / pulseinterval_sum_g0g5 > 2)
				priv->pri_cond2 = 1;
			
			if (pulseinterval_sum_g1g2g3g4 <= priv->pshare->rf_ft_var.pri_sum_safe_fcc_th)
				priv->pri_cond3 = 1;	//  reasonable group shouldn't too large
			
			if (priv->idle_flag == 1){	// the difference between pri and pw for idle mode (thr=2)
				if (abs(pulsewidth_sum_g1g2g3g4 - pulseinterval_sum_g1g2g3g4) <= safe_pri_width_diff_fcc_idle_th)
					priv->pri_cond4 = 1;			
			}
			else{	
				if((priv->pmib->dot11RFEntry.dot11channel >= 52) && (priv->pmib->dot11RFEntry.dot11channel <= 64)){
						if (abs(pulsewidth_sum_g1g2g3g4 - pulseinterval_sum_g1g2g3g4) <= safe_pri_width_diff_w53_th)
							priv->pri_cond4 = 1;	// the difference between pri and pw for w53 throughput mode (thr=15)
				}
				else{
					if (abs(pulsewidth_sum_g1g2g3g4 - pulseinterval_sum_g1g2g3g4) <= safe_pri_width_diff_fcc_th)
						priv->pri_cond4 = 1;	// the difference between pri and pw for throughput mode (thr=8)
				}
			}
			
			if (priv->idle_flag == 1){
				if (priv->pri_std >= priv->pshare->rf_ft_var.pri_std_idle_th){
					if (max_pw_idx == 3 && pulseinterval_sum_g1g2g3g4 <= priv->pshare->rf_ft_var.pri_sum_type4_th){
						priv->pri_cond5 = 1;	// To distinguish between type 6 radar and false detection
					}
					else if (max_pw_idx == 1 && pulseinterval_sum_g1g2g3g4 >= priv->pshare->rf_ft_var.pri_sum_type6_th){
						priv->pri_cond5 = 1;	// To distinguish between type 4 radar and false detection
					}
					else{
						priv->pri_cond5 = 1;	// pri variation of short radar should be large (idle mode)
					}
				}
			}
			else{
				if (priv->pri_std >= priv->pshare->rf_ft_var.pri_std_th)
					priv->pri_cond5 = 1;	// pri variation of short radar should be large (TP mode)
			}
			
			if (priv->pri_cond1 && priv->pri_cond2 && priv->pri_cond3 && priv->pri_cond4 && priv->pri_cond5)
				priv->pulseinterval_flag = 1;
			
			/* PW judgment conditions for short radar type */
			if (((pulsewidth_sum_g0g5 + pulsewidth_sum_g1g2g3g4) / pulsewidth_sum_g0g5 > 2) && (priv->plusewidth_hold_sum[5] <= 1))
				priv->pw_cond1 = 1;	// ratio of reasonable group and illegal group && g5 should be zero

			if ((priv->pmib->dot11RFEntry.dot11channel >= 52) && (priv->pmib->dot11RFEntry.dot11channel <= 64))
				priv->pw_cond2 = 1;
			else if (priv->plusewidth_hold_sum[0] <= priv->pshare->rf_ft_var.pw_g0_th)
				priv->pw_cond2 = 1;	// unreasonable group shouldn't too large

			if (priv->idle_flag == 1){
				if (priv->pw_std >= priv->pshare->rf_ft_var.pw_std_idle_th)
					priv->pw_cond3 = 1;	// pw variation of short radar should be large (idle mode)
			}
			else{
				if (priv->pw_std >= priv->pshare->rf_ft_var.pw_std_th)
					priv->pw_cond3 = 1;	// pw variation of short radar should be large (TP mode)
			}
			if(priv->pw_cond1 && priv->pw_cond2 && priv->pw_cond3)
				priv->pulsewidth_flag = 1;
			
			/* Judgment conditions of long radar type */
			if (priv->pmib->dot11nConfigEntry.dot11nUse40M == HT_CHANNEL_WIDTH_20){
				if (priv->plusewidth_long_hold_sum[4] >= priv->pshare->rf_ft_var.pulsewidth_long_lower_20m_th)
					priv->pw_long_cond1 = 1;	
			}
			else{
				if (priv->plusewidth_long_hold_sum[4] >= priv->pshare->rf_ft_var.pulsewidth_long_lower_th)
					priv->pw_long_cond1 = 1;
			}
			if (priv->plusewidth_long_hold_sum[1] + priv->plusewidth_long_hold_sum[2] + priv->plusewidth_long_hold_sum[3] + priv->plusewidth_long_hold_sum[4] <= priv->pshare->rf_ft_var.pulsewidth_long_sum_upper_th)
				priv->pw_long_cond2 = 1;
			if (priv->pluseinterval_long_hold_sum[4] <= priv->pshare->rf_ft_var.pri_long_upper_th)	//g4 should be large for long radar
				priv->pri_long_cond1 = 1;
			if(priv->pw_long_cond1 && priv->pw_long_cond2 && priv->pri_long_cond1)
				priv->long_radar_flag = 1;
		}
		else if (priv->pshare->rf_ft_var.manual_dfs_regdomain == 3) {			
			
			/*ratio of reasonable group and illegal group */
			if ((pulseinterval_sum_g0g5 + pulseinterval_sum_g1g2g3g4) / pulseinterval_sum_g0g5 > 2 )
				priv->pri_cond2=1;		
			
			if (pulseinterval_sum_g1g2g3g4 <= priv->pshare->rf_ft_var.pri_sum_safe_th)
				priv->pri_cond3=1;
			
			if (abs(pulsewidth_sum_g1g2g3g4 - pulseinterval_sum_g1g2g3g4) <= safe_pri_width_diff_th)
				priv->pri_cond4=1;
			
			if (priv->pluseinterval_hold_sum[5] <= priv->pshare->rf_ft_var.pri_sum_g5_th)
				priv->pri_cond5=1;
			
			if (priv->pmib->dot11nConfigEntry.dot11nUse40M == HT_CHANNEL_WIDTH_20_40){			
				if(max_pw_idx == 4){				
					if(max_pw_cnt >= priv->pshare->rf_ft_var.type4_pw_max_cnt && pulseinterval_sum_g1g2g3g4 >=priv->pshare->rf_ft_var.type4_safe_pri_sum_th){
						priv->pri_cond1=1;					
						priv->pri_cond4=1;
						priv->pri_type3_4_cond1=1;
					}
				}
			}
			
			if(priv->pri_cond1 && priv->pri_cond2 && priv->pri_cond3 && priv->pri_cond4 && priv->pri_cond5)
				priv->pulseinterval_flag = 1;		
					
			if (((pulsewidth_sum_g0g5 + pulsewidth_sum_g1g2g3g4) / pulsewidth_sum_g0g5 > 2) && (priv->plusewidth_hold_sum[5] == 0))
				priv->pulsewidth_flag = 1;	

			/*max num pri group is g1 means radar type3 or type4*/
			if (max_pri_idx == 1){			
				if (max_pri_cnt >= max_pri_cnt_th){
					priv->pri_type3_4_cond1=1;
				}			
				if (priv->pluseinterval_hold_sum[4] <= priv->pshare->rf_ft_var.pri_sum_g5_under_g1_th && priv->pluseinterval_hold_sum[5] <= priv->pshare->rf_ft_var.pri_sum_g5_under_g1_th){
					priv->pri_type3_4_cond2=1;
				}
			}
			else{			
				priv->pri_type3_4_cond1=1;
				priv->pri_type3_4_cond2=1;
			}
			if(priv->pri_type3_4_cond1 && priv->pri_type3_4_cond2)
				priv->pri_type3_4_flag = 1;

			/* False Detect */
			priv->pshare->rf_ft_var.dfs_false_reason_code =0;
			false_peak_hist1=0;
			false_peak_hist2_above0=0;
			false_peak_hist2_above10=0;		
			for (i=0;i<16;i++) {
				if (g_peakindex[i]>=0xf)
					false_peak_hist1++;
				if (g_peakindex[i]>0)
					false_peak_hist2_above0++;
				if (g_peakindex[i]>=0xb)
					false_peak_hist2_above10++;
			}
					
			if ((false_peak_hist1>=priv->pshare->rf_ft_var.false_peak_hist1_th_max)){
				if (priv->idle_flag == 1){
					priv->pshare->rf_ft_var.dfs_false_reason_code =1;
					if (priv->pshare->rf_ft_var.dfs_det_print4){
						panic_printk("false_peak_hist1=%d, false_peak_hist1_th_max=%d\n", false_peak_hist1, priv->pshare->rf_ft_var.false_peak_hist1_th_max);
						panic_printk("criterion 1 satisfied!\n");
					}
				}
			}		
			else if ((false_peak_hist1>=priv->pshare->rf_ft_var.false_peak_hist1_th) && (g_plusewidth[4] + g_plusewidth[5] < priv->pshare->rf_ft_var.false_width_hist_th_g4g5)){
				if (priv->idle_flag == 1){
					priv->pshare->rf_ft_var.dfs_false_reason_code =2;
					if (priv->pshare->rf_ft_var.dfs_det_print4){
						panic_printk("false_peak_hist1=%d, false_peak_hist1_th=%d, g_plusewidth[4]=%d, g_plusewidth[5]=%d, false_width_hist_th_g4g5=%d\n, ", false_peak_hist1, priv->pshare->rf_ft_var.false_peak_hist1_th,g_plusewidth[4],g_plusewidth[5],priv->pshare->rf_ft_var.false_width_hist_th_g4g5);
						panic_printk("criterion 2 satisfied!\n");
					}
				}
			}		
			else if ((false_peak_hist2_above0>=priv->pshare->rf_ft_var.false_peak_hist2_th_above0) &&
				(false_peak_hist2_above10>=priv->pshare->rf_ft_var.false_peak_hist2_th_above10) && (g_plusewidth[4] + g_plusewidth[5] < priv->pshare->rf_ft_var.false_width_hist_th_g4g5)){
				if (priv->idle_flag == 1){
					priv->pshare->rf_ft_var.dfs_false_reason_code =3;
					if (priv->pshare->rf_ft_var.dfs_det_print4){
						panic_printk("false_peak_hist2_above0=%d, false_peak_hist2_above10=%d, false_peak_hist2_th_above0=%d, false_peak_hist2_th_above10=%d, g_plusewidth[4]=%d, g_plusewidth[5]=%d, false_width_hist_th_g4g5=%d\n", false_peak_hist2_above0, false_peak_hist2_above10, priv->pshare->rf_ft_var.false_peak_hist2_th_above0, priv->pshare->rf_ft_var.false_peak_hist2_th_above10,g_plusewidth[4],g_plusewidth[5],priv->pshare->rf_ft_var.false_width_hist_th_g4g5);
						panic_printk("criterion 3 satisfied!\n");
					}
				}
			}		
			else if (g_plusewidth[0]>priv->pshare->rf_ft_var.false_width_hist_th_g0){
				if (priv->idle_flag == 1){
					priv->pshare->rf_ft_var.dfs_false_reason_code =4;
					if (priv->pshare->rf_ft_var.dfs_det_print4){
						panic_printk("g_plusewidth[0]=%d, false_width_hist_th_g0=%d\n", g_plusewidth[0], priv->pshare->rf_ft_var.false_width_hist_th_g0);
						panic_printk("criterion 4 satisfied!\n");
					}
				}
			}		
			else if (g_pluseinterval[5]>=priv->pshare->rf_ft_var.false_interval_hist_th_g5){
				priv->pshare->rf_ft_var.dfs_false_reason_code =5;
				if (priv->pshare->rf_ft_var.dfs_det_print4){
					panic_printk("g_pluseinterval[5]=%d, false_interval_hist_th_g5=%d\n", g_plusewidth[0], priv->pshare->rf_ft_var.false_interval_hist_th_g5);
					panic_printk("criterion 5 satisfied!\n");
				}
			}		
			if (priv->pshare->rf_ft_var.dfs_false_reason_code!=0) {
				priv->pshare->rf_ft_var.dfs_false_mask = priv->pshare->rf_ft_var.dfs_false_mask_th;
				priv->pshare->rf_ft_var.dfs_false_trigger=1;
			}
			else
				priv->pshare->rf_ft_var.dfs_false_trigger=0;

			if (priv->pshare->rf_ft_var.dfs_false_mask>0) {
				priv->pshare->rf_ft_var.dfs_false_mask --;
			}
		}else{}
#if defined(CONFIG_WLAN_HAL_8814AE) || defined(CONFIG_WLAN_HAL_8822BE)
		if(priv->pshare->rf_ft_var.dfs_print_hist_report){			
			dfs_plusewidth_thd1 = PHY_QueryBBReg(priv, 0x19e4, 0xff000000);
			dfs_plusewidth_thd2 = PHY_QueryBBReg(priv, 0x19e8, 0x000000ff);
			dfs_plusewidth_thd3 = PHY_QueryBBReg(priv, 0x19e8, 0x0000ff00);
			dfs_plusewidth_thd4 = PHY_QueryBBReg(priv, 0x19e8, 0x00ff0000);
			dfs_plusewidth_thd5 = PHY_QueryBBReg(priv, 0x19e8, 0xff000000);
						
			dfs_pluseinterval_thd1 = PHY_QueryBBReg(priv, 0x19b8, 0x7F80);
			dfs_pluseinterval_thd2 = PHY_QueryBBReg(priv, 0x19ec, 0x000000ff);
			dfs_pluseinterval_thd3 = PHY_QueryBBReg(priv, 0x19ec, 0x0000ff00);
			dfs_pluseinterval_thd4 = PHY_QueryBBReg(priv, 0x19ec, 0x00ff0000);
			dfs_pluseinterval_thd5 = PHY_QueryBBReg(priv, 0x19ec, 0xff000000);
		
			panic_printk("peak index hist\n");
			panic_printk("dfs_hist_peak_index=%x %x\n",
				dfs_hist1_peak_index, dfs_hist2_peak_index);
			panic_printk("g_peak_index_hist = ");
			for (i=0 ; i<16 ; i++)
				panic_printk(" %x",g_peakindex[i]);			
			panic_printk("\ndfs_plusewidth_thd=%d %d %d %d %d\n",
				dfs_plusewidth_thd1, dfs_plusewidth_thd2, dfs_plusewidth_thd3, dfs_plusewidth_thd4, dfs_plusewidth_thd5);
			panic_printk("-----pulse width hist-----\n");
			panic_printk("dfs_hist_pulse_width=%x %x\n",
				dfs_hist1_pulse_width, dfs_hist2_pulse_width);
			panic_printk("g_pulse_width_hist = %x %x %x %x %x %x\n",
				g_plusewidth[0], g_plusewidth[1],g_plusewidth[2],g_plusewidth[3],g_plusewidth[4],g_plusewidth[5]);
			panic_printk("dfs_pluseinterval_thd=%d %d %d %d %d\n",
			dfs_pluseinterval_thd1, dfs_pluseinterval_thd2, dfs_pluseinterval_thd3, dfs_pluseinterval_thd4, dfs_pluseinterval_thd5);
			panic_printk("-----pulse interval hist-----\n");
			panic_printk("dfs_hist_pulse_interval=%x %x\n",	dfs_hist1_pulse_interval, dfs_hist2_pulse_interval);
			panic_printk("g_pulse_interval_hist = %x %x %x %x %x %x, pulsewidth_flag = %d, pulseinterval_flag = %d\n",
			g_pluseinterval[0], g_pluseinterval[1],g_pluseinterval[2],g_pluseinterval[3],g_pluseinterval[4],g_pluseinterval[5],priv->pulsewidth_flag,priv->pulseinterval_flag);		
			panic_printk("FalseDetectReason: %d\n",priv->pshare->rf_ft_var.dfs_false_reason_code);
			panic_printk("FD>>peak_index1= %d, TH = %d\n", false_peak_hist1, priv->pshare->rf_ft_var.false_peak_hist1_th);
			panic_printk("FD>>peak_index2=>over10 = %d, TH10 = %d, over_0 = %d, TH0 = %d\n",
				false_peak_hist2_above10, priv->pshare->rf_ft_var.false_peak_hist2_th_above0, false_peak_hist2_above0, priv->pshare->rf_ft_var.false_peak_hist2_th_above10);
			panic_printk("FD>>pulse_width : g0 = %d, false_width_hist_th_g0= %d, false_width_hist_th_gtotal= %d\n",
				g_plusewidth[0], priv->pshare->rf_ft_var.false_width_hist_th_g0, priv->pshare->rf_ft_var.false_width_hist_th_gtotal);
			panic_printk("FD mask = %d, TH = %d\n",priv->pshare->rf_ft_var.dfs_false_mask, priv->pshare->rf_ft_var.dfs_false_mask_th);
			if (priv->pshare->rf_ft_var.manual_dfs_regdomain == 1 || priv->pshare->rf_ft_var.manual_dfs_regdomain == 3)
				panic_printk("hist_idx= %d\n", (priv->hist_idx+2)%3);
			else
				panic_printk("hist_idx= %d\n", (priv->hist_idx+3)%4);
			panic_printk("hist_long_idx= %d\n", (priv->hist_long_idx+299)%300);
			panic_printk("pulsewidth_sum_g0g5 = %d, pulsewidth_sum_g1g2g3g4 = %d\n",
			pulsewidth_sum_g0g5, pulsewidth_sum_g1g2g3g4);
			panic_printk("pulseinterval_sum_g0g5 = %d, pulseinterval_sum_g1g2g3g4 = %d\n",
			pulseinterval_sum_g0g5, pulseinterval_sum_g1g2g3g4);
			panic_printk("plusewidth_hold_sum = %d %d %d %d %d %d\n",
			priv->plusewidth_hold_sum[0], priv->plusewidth_hold_sum[1],priv->plusewidth_hold_sum[2],priv->plusewidth_hold_sum[3],priv->plusewidth_hold_sum[4],priv->plusewidth_hold_sum[5]);
			panic_printk("pluseinterval_hold_sum = %d %d %d %d %d %d\n",
			priv->pluseinterval_hold_sum[0], priv->pluseinterval_hold_sum[1],priv->pluseinterval_hold_sum[2],priv->pluseinterval_hold_sum[3],priv->pluseinterval_hold_sum[4],priv->pluseinterval_hold_sum[5]);
			panic_printk("plusewidth_long_hold_sum = %d %d %d %d %d %d\n",
			priv->plusewidth_long_hold_sum[0], priv->plusewidth_long_hold_sum[1],priv->plusewidth_long_hold_sum[2],priv->plusewidth_long_hold_sum[3],priv->plusewidth_long_hold_sum[4],priv->plusewidth_long_hold_sum[5]);
			panic_printk("pluseinterval_long_hold_sum = %d %d %d %d %d %d\n",
			priv->pluseinterval_long_hold_sum[0], priv->pluseinterval_long_hold_sum[1],priv->pluseinterval_long_hold_sum[2],priv->pluseinterval_long_hold_sum[3],priv->pluseinterval_long_hold_sum[4],priv->pluseinterval_long_hold_sum[5]);
			panic_printk("idle_flag = %d\n",priv->idle_flag);
			panic_printk("pulsewidth_standard = %d\n",priv->pw_std);
			panic_printk("pulseinterval_standard = %d\n",priv->pri_std);
			for (j=0;j<4;j++){
				for (i=0;i<6;i++){
					panic_printk("pluseinterval_hold = %d ", priv->pluseinterval_hold[j][i]);
				}
				panic_printk("\n");
			}			
			panic_printk("\n");
			panic_printk("pri_cond1 = %d, pri_cond2 = %d, pri_cond3 = %d, pri_cond4 = %d, pri_cond5 = %d\n",
			priv->pri_cond1, priv->pri_cond2, priv->pri_cond3, priv->pri_cond4, priv->pri_cond5);
			panic_printk("priv->pmib->dot11nConfigEntry.dot11nUse40M = %d, pri_th = %d, max_pri_cnt_th = %d, safe_pri_width_diff_th = %d\n",priv->pmib->dot11nConfigEntry.dot11nUse40M, pri_th, max_pri_cnt_th, safe_pri_width_diff_th);
		}
#endif
}

void rtl8192cd_DFS_timer(unsigned long task_priv)
{
	struct rtl8192cd_priv *priv = (struct rtl8192cd_priv *)task_priv;
	unsigned int dfs_chk = 0;
	unsigned long throughput = 0;
	int j;
	int tp_th = ((priv->pshare->is_40m_bw)?45:20);
#ifndef SMP_SYNC
	unsigned long flags;
#endif

	SAVE_INT_AND_CLI(flags);
	SMP_LOCK(flags);
	if (!(priv->drv_state & DRV_STATE_OPEN)) {
		SMP_UNLOCK(flags);
		RESTORE_INT(flags);
		return;
	}

#ifdef PCIE_POWER_SAVING
	if ((priv->pwr_state == L2) || (priv->pwr_state == L1))
		goto exit_timer;
#endif
#ifdef PCIE_POWER_SAVING_TEST //yllin
    if((priv->pwr_state >= L2) || (priv->pwr_state == L1)) {
        goto exit_timer;
    }
#endif

#ifdef CONFIG_32K 
            if(priv->offload_32k_flag==1) {
                //printk("32K DFS_timer return/n");
                goto exit_timer;
            }
#endif


	throughput = priv->ext_stats.tx_avarage+priv->ext_stats.rx_avarage;
#ifdef UNIVERSAL_REPEATER
	if (GET_VXD_PRIV(priv) && (GET_VXD_PRIV(priv)->drv_state & DRV_STATE_OPEN))
		throughput += (GET_VXD_PRIV(priv)->ext_stats.tx_avarage + GET_VXD_PRIV(priv)->ext_stats.rx_avarage);
#endif

#ifdef MBSSID
	if (priv->pmib->miscEntry.vap_enable) {
		for (j=0; j<RTL8192CD_NUM_VWLAN; j++) {
			if (IS_DRV_OPEN(priv->pvap_priv[j])) {
				throughput += priv->pvap_priv[j]->ext_stats.tx_avarage+priv->pvap_priv[j]->ext_stats.rx_avarage;
			}
		}
	}
#endif

	if (throughput>>17>tp_th) {
		if (GET_CHIP_VER(priv) == VERSION_8192D){
			dfs_chk = 1;
			PHY_SetBBReg(priv, 0xcdc, BIT(8)|BIT(9), 0);
		}
	} else {
		dfs_chk = 0;
		if (GET_CHIP_VER(priv) == VERSION_8192D)
			PHY_SetBBReg(priv, 0xcdc, BIT(8)|BIT(9), 1);
	}
	// debug print 
	/*#if defined(CONFIG_WLAN_HAL_8814AE) || defined(CONFIG_WLAN_HAL_8822BE)
			if((GET_CHIP_VER(priv) == VERSION_8814A) || (GET_CHIP_VER(priv) == VERSION_8822B)){
				if(priv->pshare->rf_ft_var.dfs_radar_diff_on){
					dfs_dbg_report(priv);
				}
			}            
	#endif*/
	// ETSI
	if (!(priv->pshare->rf_ft_var.manual_dfs_regdomain == 3))
	{
		if (GET_CHIP_VER(priv) == VERSION_8192D) {
			if (PHY_QueryBBReg(priv, 0xcf8, BIT(31))) {
				priv->radar_type++;
				priv->pmib->dot11DFSEntry.DFS_detected = 1;
			}
		}
		else if ((GET_CHIP_VER(priv) == VERSION_8812E) || (GET_CHIP_VER(priv) == VERSION_8881A) || (GET_CHIP_VER(priv) == VERSION_8814A) || (GET_CHIP_VER(priv) == VERSION_8822B)) {
			if (priv->pshare->rf_ft_var.dfs_det_off == 1) {
				if (PHY_QueryBBReg(priv, 0xf98, BIT(19))) {
					priv->radar_type++;
					priv->pmib->dot11DFSEntry.DFS_detected = 1;

				}
			}
		}
	}

	if (GET_CHIP_VER(priv) == VERSION_8192D) {
		if (PHY_QueryBBReg(priv, 0xcf8, BIT(23)))
			priv->pmib->dot11DFSEntry.DFS_detected = 1;
	}
	else if ((GET_CHIP_VER(priv) == VERSION_8812E) || (GET_CHIP_VER(priv) == VERSION_8881A) || (GET_CHIP_VER(priv) == VERSION_8814A) || (GET_CHIP_VER(priv) == VERSION_8822B)) {
		if (priv->pshare->rf_ft_var.dfs_det_off == 1) {
			if (PHY_QueryBBReg(priv, 0xf98, BIT(17)))
				priv->pmib->dot11DFSEntry.DFS_detected = 1;
		}
	}
	
	/*
	 *	DFS debug mode for logo test
	 */
	if (priv->pmib->dot11DFSEntry.DFS_detected && (priv->pshare->rf_ft_var.dfs_false_mask!=0)) {
		priv->pmib->dot11DFSEntry.DFS_detected=0;
		priv->pshare->rf_ft_var.dfs_false_mask ++;
		if (priv->pshare->rf_ft_var.dfs_det_print4)
			panic_printk("False Detect in Mask = %d", priv->pshare->rf_ft_var.dfs_false_mask);
		
	}
	if (!priv->pmib->dot11DFSEntry.disable_DFS && priv->pshare->rf_ft_var.dfsdbgmode 
		&& priv->pmib->dot11DFSEntry.DFS_detected) {
		if ((jiffies - priv->pshare->rf_ft_var.dfsrctime)>RTL_SECONDS_TO_JIFFIES(10))
			priv->pshare->rf_ft_var.dfsdbgcnt = 1;
		else
			priv->pshare->rf_ft_var.dfsdbgcnt++;
		if (GET_CHIP_VER(priv) == VERSION_8192D) {
			if (priv->pshare->rf_ft_var.dfs_det_print1)
				panic_printk("[%d] DFS dbg mode, Radar is detected as %x %08x (%d)!\n", priv->pshare->rf_ft_var.dfsdbgcnt,
					priv->radar_type, PHY_QueryBBReg(priv, 0xcf4, bMaskDWord), (unsigned int)RTL_JIFFIES_TO_MILISECONDS(jiffies));
		}
		else if ((GET_CHIP_VER(priv) == VERSION_8812E) || (GET_CHIP_VER(priv) == VERSION_8881A) || (GET_CHIP_VER(priv) == VERSION_8814A) || (GET_CHIP_VER(priv) == VERSION_8822B)) {
			if (priv->pshare->rf_ft_var.dfs_det_print1)
				panic_printk("[%d] DFS dbg mode, Radar is detected as %x %08x (%d)!\n", priv->pshare->rf_ft_var.dfsdbgcnt,
					priv->radar_type, PHY_QueryBBReg(priv, 0xf98, 0xffffffff), (unsigned int)RTL_JIFFIES_TO_MILISECONDS(jiffies));
			RTL_W32(0x920, RTL_R32(0x920) | (BIT(24) | BIT(25) | BIT(28)));
			RTL_W32(0x920, RTL_R32(0x920) & ~(BIT(24) | BIT(25) | BIT(28)));
		}
		priv->pshare->rf_ft_var.dfsrctime = jiffies;
		priv->pmib->dot11DFSEntry.DFS_detected = 0;
		if (GET_CHIP_VER(priv) == VERSION_8192D) {
			PHY_SetBBReg(priv, 0xc84, BIT(25), 0);
			PHY_SetBBReg(priv, 0xc84, BIT(25), 1);
		}
		goto exit_timer;
	}


	if (!priv->pmib->dot11DFSEntry.disable_DFS && priv->pmib->dot11DFSEntry.DFS_detected) {
		if (GET_CHIP_VER(priv) == VERSION_8192D) {
			PRINT_INFO("Radar is detected as %x %08x (%d)!\n",
				priv->radar_type, PHY_QueryBBReg(priv, 0xcf4, bMaskDWord), (unsigned int)RTL_JIFFIES_TO_MILISECONDS(jiffies));
		}
		else if ((GET_CHIP_VER(priv) == VERSION_8812E) || (GET_CHIP_VER(priv) == VERSION_8881A) || (GET_CHIP_VER(priv) == VERSION_8814A) || (GET_CHIP_VER(priv) == VERSION_8822B)) {
			PRINT_INFO("Radar is detected as %x %08x (%d)!\n",
				priv->radar_type, PHY_QueryBBReg(priv, 0xf98, 0xffffffff), (unsigned int)RTL_JIFFIES_TO_MILISECONDS(jiffies));
		}
		
		if (timer_pending(&priv->dfs_chk_timer)) {
			del_timer(&priv->dfs_chk_timer);
			if (GET_CHIP_VER(priv) == VERSION_8192D)
				PHY_SetBBReg(priv, 0xcdc, BIT(8)|BIT(9), 1);
			PRINT_INFO("DFS CP2. Switch channel!\n");
		} else {
			if (dfs_chk){
				// reset dfs flag and counter
				priv->pmib->dot11DFSEntry.DFS_detected = 0;
				if (GET_CHIP_VER(priv) == VERSION_8192D) {
					PHY_SetBBReg(priv, 0xc84, BIT(25), 0);
					PHY_SetBBReg(priv, 0xc84, BIT(25), 1);
				}
				
				PRINT_INFO("DFS CP1.\n");

				init_timer(&priv->dfs_chk_timer);
				priv->dfs_chk_timer.data = (unsigned long) priv;
				priv->dfs_chk_timer.function = rtl8192cd_dfs_chk_timer;

				mod_timer(&priv->dfs_chk_timer, jiffies + RTL_SECONDS_TO_JIFFIES(300));

				goto exit_timer;
			}
		}
		
		priv->pmib->dot11DFSEntry.disable_tx = 1;

		if (timer_pending(&priv->ch_avail_chk_timer)) {
			del_timer(&priv->ch_avail_chk_timer);
			RTL_W8(TXPAUSE, 0xff);
		}
		else
			RTL_W8(TXPAUSE, 0xf);	/* disable transmitter */
#if !defined(RTK_NL80211)
		//For OpwenWRT SDK, do not consider block channel list
		if(priv->pshare->CurrentChannelBW == HT_CHANNEL_WIDTH_80 && 
			priv->pmib->dot11RFEntry.band5GSelected == PHY_BAND_5G_3) {
			int i, channel;
			channel = priv->pmib->dot11RFEntry.dot11channel;

			if ((channel >= 104) && (channel <= 112))
				channel = 100;
			else if ((channel >= 120) && (channel <= 128))
				channel = 116;
			else if ((channel >= 136) && (channel <= 144))
				channel = 132;

			for(i=0;i<4;i++) {
				set_CHXX_timer(priv, channel+i*4);
			}
		}
		else {
			if(priv->pshare->CurrentChannelBW == HT_CHANNEL_WIDTH_80){
				int i, channel;
				channel = priv->pmib->dot11RFEntry.dot11channel;

				if ((channel >= 56) && (channel <= 64))
						channel = 52;
				if ((channel >= 104) && (channel <= 112))
						channel = 100;
				else if ((channel >= 120) && (channel <= 128))
						channel = 116;
				else if ((channel >= 136) && (channel <= 144))
						channel = 132;

				for(i=0;i<4;i++) {
					set_CHXX_timer(priv, channel+i*4);
				}

			}else	if(priv->pshare->CurrentChannelBW == HT_CHANNEL_WIDTH_20_40){

				int i, channel;
				int CH_BW40m[8]={56,64,104,112,120,128,136,144};	

				channel = priv->pmib->dot11RFEntry.dot11channel;	

				for(i=0;i<8;i++){

					if(channel<=CH_BW40m[i]){
						set_CHXX_timer(priv, CH_BW40m[i]);
						set_CHXX_timer(priv, CH_BW40m[i]-4);
						break;
					}
				}

			}else{
				set_CHXX_timer(priv,priv->pmib->dot11RFEntry.dot11channel);
			}
		}

		/* add the channel in the blocked-channel list */
		if(priv->pshare->CurrentChannelBW == HT_CHANNEL_WIDTH_80 && 
			priv->pmib->dot11RFEntry.band5GSelected == PHY_BAND_5G_3) {
			int i, channel;
			channel = priv->pmib->dot11RFEntry.dot11channel;

			if ((channel >= 104) && (channel <= 112))
				channel = 100;
			else if ((channel >= 120) && (channel <= 128))
				channel = 116;
			else if ((channel >= 136) && (channel <= 144))
				channel = 132;

			for (i=0;i<4;i++) {
				if (RemoveChannel(priv, priv->available_chnl, &priv->available_chnl_num, channel+i*4))
					InsertChannel(priv->NOP_chnl, &priv->NOP_chnl_num, channel+i*4);									
#ifdef UNIVERSAL_REPEATER
				if (IS_DRV_OPEN(GET_VXD_PRIV(priv)) && (RemoveChannel(priv->pvxd_priv, priv->pvxd_priv->available_chnl, &priv->pvxd_priv->available_chnl_num, channel+i*4)) )
					InsertChannel(priv->pvxd_priv->NOP_chnl, &priv->pvxd_priv->NOP_chnl_num, channel+i*4);
#endif
			}			
		}
		else {
			if(priv->pshare->CurrentChannelBW == HT_CHANNEL_WIDTH_80){
				int i, channel;
				channel = priv->pmib->dot11RFEntry.dot11channel;

				if ((channel >= 56) && (channel <= 64))
						channel = 52;
				if ((channel >= 104) && (channel <= 112))
						channel = 100;
				else if ((channel >= 120) && (channel <= 128))
						channel = 116;
				else if ((channel >= 136) && (channel <= 144))
						channel = 132;

				for (i=0;i<4;i++) {
				if (RemoveChannel(priv, priv->available_chnl, &priv->available_chnl_num, channel+i*4))
					InsertChannel(priv->NOP_chnl, &priv->NOP_chnl_num, channel+i*4);									
#ifdef UNIVERSAL_REPEATER
				if (IS_DRV_OPEN(GET_VXD_PRIV(priv)) && (RemoveChannel(priv->pvxd_priv, priv->pvxd_priv->available_chnl, &priv->pvxd_priv->available_chnl_num, channel+i*4)) )
					InsertChannel(priv->pvxd_priv->NOP_chnl, &priv->pvxd_priv->NOP_chnl_num, channel+i*4);
#endif
				}			

			}else	if(priv->pshare->CurrentChannelBW == HT_CHANNEL_WIDTH_20_40){

				int i, channel;
				int CH_BW40m[8]={56,64,104,112,120,128,136,144};	

				channel = priv->pmib->dot11RFEntry.dot11channel;	

				for(i=0;i<8;i++){

					if(channel<=CH_BW40m[i]){
						if (RemoveChannel(priv, priv->available_chnl, &priv->available_chnl_num, CH_BW40m[i]))
							InsertChannel(priv->NOP_chnl, &priv->NOP_chnl_num, CH_BW40m[i]);
						if (RemoveChannel(priv, priv->available_chnl, &priv->available_chnl_num, CH_BW40m[i]-4))
							InsertChannel(priv->NOP_chnl, &priv->NOP_chnl_num, CH_BW40m[i]-4);			
#ifdef UNIVERSAL_REPEATER
						if (IS_DRV_OPEN(GET_VXD_PRIV(priv)) && (RemoveChannel(priv->pvxd_priv, priv->pvxd_priv->available_chnl, &priv->pvxd_priv->available_chnl_num, CH_BW40m[i])) )
							InsertChannel(priv->pvxd_priv->NOP_chnl, &priv->pvxd_priv->NOP_chnl_num, CH_BW40m[i]);
						if (IS_DRV_OPEN(GET_VXD_PRIV(priv)) && (RemoveChannel(priv->pvxd_priv, priv->pvxd_priv->available_chnl, &priv->pvxd_priv->available_chnl_num, CH_BW40m[i]-4)) )
							InsertChannel(priv->pvxd_priv->NOP_chnl, &priv->pvxd_priv->NOP_chnl_num, CH_BW40m[i]-4);
#endif
						break;
					}
				}			

			}else	{

			int j;
			InsertChannel(priv->NOP_chnl, &priv->NOP_chnl_num, priv->pmib->dot11RFEntry.dot11channel);			
			RemoveChannel(priv, priv->available_chnl, &priv->available_chnl_num, priv->pmib->dot11RFEntry.dot11channel);
#ifdef UNIVERSAL_REPEATER
				if (IS_DRV_OPEN(GET_VXD_PRIV(priv))){
					RemoveChannel(priv->pvxd_priv, priv->pvxd_priv->available_chnl, &priv->pvxd_priv->available_chnl_num, priv->pmib->dot11RFEntry.dot11channel);
					InsertChannel(priv->pvxd_priv->NOP_chnl, &priv->pvxd_priv->NOP_chnl_num, priv->pmib->dot11RFEntry.dot11channel);
				}
#endif
			}

		}
#endif
        if (timer_pending(&priv->DFS_timer))
            del_timer(&priv->DFS_timer);
        if (timer_pending(&priv->dfs_det_chk_timer))
            del_timer(&priv->dfs_det_chk_timer);


#if !defined(RTK_NL80211)
		//For OpenWRT, do not select another channel automatically
		/* select a channel */
		priv->pshare->dfsSwitchChannel = DFS_SelectChannel(priv);
		if(priv->pshare->dfsSwitchChannel == 0) {
			priv->pmib->dot11DFSEntry.DFS_detected = 0;		

			if (priv->pmib->dot11RFEntry.band5GSelected == PHY_BAND_5G_3) {
				// when band 2 is selected, AP does not come back to band2 
				// even when NOP (NONE_OCCUPANCY_PERIOD) timer is expired.	
				RTL_W8(TXPAUSE, 0xff);// after cac, still has to pause if channel is run out
				mod_timer(&priv->DFS_TXPAUSE_timer, jiffies + DFS_TXPAUSE_TO);
			}
		}
#else
	//Clear disable_tx here becoz turnkey do it at DFS_SelectChannel()
	priv->pmib->dot11DFSEntry.disable_tx = 0;
#endif


        
#ifdef CONFIG_RTK_MESH
        mesh_DFS_switch_channel(priv);
        if(priv->pmib->dot1180211sInfo.mesh_enable)
            priv->pshare->dfsSwitchChCountDown = MESH_DFS_SWITCH_COUNTDOWN;
        else
#endif
#ifdef MBSSID
		if (priv->pmib->miscEntry.vap_enable)
			priv->pshare->dfsSwitchChCountDown = 6;
		else
#endif
			priv->pshare->dfsSwitchChCountDown = 5;


		if (priv->pmib->dot11StationConfigEntry.dot11DTIMPeriod >= priv->pshare->dfsSwitchChCountDown)
			priv->pshare->dfsSwitchChCountDown = priv->pmib->dot11StationConfigEntry.dot11DTIMPeriod+1;						

#if defined(RTK_NL80211)
		event_indicate_cfg80211(priv, NULL, CFG80211_RADAR_DETECTED, NULL);
#endif

		SMP_UNLOCK(flags);
		RESTORE_INT(flags);
		return;
	}

exit_timer:
	mod_timer(&priv->DFS_timer, jiffies + DFS_TO);
	SMP_UNLOCK(flags);
	RESTORE_INT(flags);
}


#ifdef CLIENT_MODE
void rtl8192cd_dfs_cntdwn_timer(unsigned long task_priv)
{
	struct rtl8192cd_priv *priv = (struct rtl8192cd_priv *)task_priv;
#ifdef SMP_SYNC
	unsigned long flags;
#endif

	SMP_LOCK(flags);
	if (!(priv->drv_state & DRV_STATE_OPEN)) {
		SMP_UNLOCK(flags);
		return;
	}
	
	DEBUG_INFO("rtl8192cd_dfs_cntdwn_timer timeout!\n");

	priv->pshare->dfsSwCh_ongoing = 0;	
	SMP_UNLOCK(flags);
}
#endif

void rtl8192cd_ch_avail_chk_timer(unsigned long task_priv)
{
	struct rtl8192cd_priv *priv = (struct rtl8192cd_priv *)task_priv;
#ifdef SMP_SYNC
	unsigned long flags;
#endif

	SMP_LOCK(flags);
	if (!(priv->drv_state & DRV_STATE_OPEN)) {
		SMP_UNLOCK(flags);
		return;
	}

	priv->pmib->dot11DFSEntry.disable_tx = 0;

if (GET_CHIP_VER(priv) == VERSION_8192D || GET_CHIP_VER(priv) == VERSION_8881A || GET_CHIP_VER(priv) == VERSION_8812E || (GET_CHIP_VER(priv) == VERSION_8814A) || (GET_CHIP_VER(priv) == VERSION_8822B)) {
		if (GET_CHIP_VER(priv) == VERSION_8881A){
			PHY_SetBBReg(priv, 0xcb0, 0x000000f0, 4);
		}
		if (priv->pshare->rf_ft_var.dfsdelayiqk)
			PHY_IQCalibrate(priv);
		if (GET_CHIP_VER(priv) == VERSION_8881A){		
			PHY_SetBBReg(priv, 0xcb0, 0x000000f0, 5);
		}
	}

#ifdef DPK_92D
	if (GET_CHIP_VER(priv) == VERSION_8192D){
		if (priv->pshare->rf_ft_var.dfsdelayiqk && priv->pshare->rf_ft_var.dpk_on)
			PHY_DPCalibrate(priv);
	}
#endif

	if (GET_CHIP_VER(priv) == VERSION_8192D)
		RTL_W16(PCIE_CTRL_REG, RTL_R16(PCIE_CTRL_REG) & (~BCNQSTOP));
	else
		RTL_W8(TXPAUSE, 0);

	SMP_UNLOCK(flags);
	// ETSI
	if(priv->pshare->rf_ft_var.manual_dfs_regdomain == 3){	
		if((IS_METEOROLOGY_CHANNEL(priv->pmib->dot11RFEntry.dot11channel))){
			if ((GET_CHIP_VER(priv) == VERSION_8814A) || (GET_CHIP_VER(priv) == VERSION_8822B)){
				PHY_SetBBReg(priv, 0x918, bMaskDWord, 0x1c17acdf);
				PHY_SetBBReg(priv, 0x924, 0xfffffff, 0x095a8400);
			}
			else{
				PHY_SetBBReg(priv, 0x918, bMaskDWord, 0x1c17ecdf);
				PHY_SetBBReg(priv, 0x924, bMaskDWord, 0x01528400);
			}
			PHY_SetBBReg(priv, 0x91c, bMaskDWord, 0x0fc21a20);			
			PHY_SetBBReg(priv, 0x920, bMaskDWord, 0xe0f70604);
			priv->ch_120_132_CAC_end = 1;
		}
	}
	if(GET_ROOT(priv)->pmib->dot11DFSEntry.CAC_enable)
		priv->pmib->dot11DFSEntry.CAC_ss_counter = 3;
	panic_printk("Transmitter is enabled!\n");
#if defined(RTK_NL80211)
	event_indicate_cfg80211(priv, NULL, CFG80211_RADAR_CAC_FINISHED, NULL);
#endif
}


void rtl8192cd_ch52_timer(unsigned long task_priv)
{
	struct rtl8192cd_priv *priv = (struct rtl8192cd_priv *)task_priv;

	if (!(priv->drv_state & DRV_STATE_OPEN))
		return;

	//still block channel 52 if in adhoc mode in Japan
	if (((priv->pmib->dot11StationConfigEntry.dot11RegDomain == DOMAIN_MKK) ||
		 (priv->pmib->dot11StationConfigEntry.dot11RegDomain == DOMAIN_MKK3)) &&
		(OPMODE & WIFI_ADHOC_STATE))
		return;

	//remove the channel from NOP_chnl[4] and place it in available_chnl[32]
	if (RemoveChannel(priv,priv->NOP_chnl, &priv->NOP_chnl_num, 52)) {
		if (priv->pmib->dot11BssType.net_work_type & WIRELESS_11A)
			InsertChannel(priv->available_chnl, &priv->available_chnl_num, 52);
		DEBUG_INFO("Channel 52 is released!\n");
	}
#ifdef UNIVERSAL_REPEATER
	if (IS_DRV_OPEN(GET_VXD_PRIV(priv)) && RemoveChannel(priv->pvxd_priv, priv->pvxd_priv->NOP_chnl, &priv->pvxd_priv->NOP_chnl_num, 52)) {
		if (priv->pvxd_priv->pmib->dot11BssType.net_work_type & WIRELESS_11A)
			InsertChannel(priv->pvxd_priv->available_chnl, &priv->pvxd_priv->available_chnl_num, 52);
	}
#endif
}

void rtl8192cd_ch56_timer(unsigned long task_priv)
{
	struct rtl8192cd_priv *priv = (struct rtl8192cd_priv *)task_priv;

	if (!(priv->drv_state & DRV_STATE_OPEN))
		return;

	if (((priv->pmib->dot11StationConfigEntry.dot11RegDomain == DOMAIN_MKK) ||
		 (priv->pmib->dot11StationConfigEntry.dot11RegDomain == DOMAIN_MKK3)) &&
		(OPMODE & WIFI_ADHOC_STATE))
		return;
	if (RemoveChannel(priv, priv->NOP_chnl, &priv->NOP_chnl_num, 56)) {
		if (priv->pmib->dot11BssType.net_work_type & WIRELESS_11A)
			InsertChannel(priv->available_chnl, &priv->available_chnl_num, 56);
		DEBUG_INFO("Channel 56 is released!\n");
#ifdef UNIVERSAL_REPEATER
	if (IS_DRV_OPEN(GET_VXD_PRIV(priv)) && RemoveChannel(priv->pvxd_priv, priv->pvxd_priv->NOP_chnl, &priv->pvxd_priv->NOP_chnl_num, 56)) {
		if (priv->pvxd_priv->pmib->dot11BssType.net_work_type & WIRELESS_11A)
			InsertChannel(priv->pvxd_priv->available_chnl, &priv->pvxd_priv->available_chnl_num, 56);
	}
#endif
	}
}

void rtl8192cd_ch60_timer(unsigned long task_priv)
{
	struct rtl8192cd_priv *priv = (struct rtl8192cd_priv *)task_priv;

	if (!(priv->drv_state & DRV_STATE_OPEN))
		return;

	if (((priv->pmib->dot11StationConfigEntry.dot11RegDomain == DOMAIN_MKK) ||
		 (priv->pmib->dot11StationConfigEntry.dot11RegDomain == DOMAIN_MKK3)) &&
		(OPMODE & WIFI_ADHOC_STATE))
		return;
	if (RemoveChannel(priv, priv->NOP_chnl, &priv->NOP_chnl_num, 60)) {
		if (priv->pmib->dot11BssType.net_work_type & WIRELESS_11A)
			InsertChannel(priv->available_chnl, &priv->available_chnl_num, 60);
		DEBUG_INFO("Channel 60 is released!\n");
	}
#ifdef UNIVERSAL_REPEATER
	if (IS_DRV_OPEN(GET_VXD_PRIV(priv)) && RemoveChannel(priv->pvxd_priv, priv->pvxd_priv->NOP_chnl, &priv->pvxd_priv->NOP_chnl_num, 60)) {
		if (priv->pvxd_priv->pmib->dot11BssType.net_work_type & WIRELESS_11A)
			InsertChannel(priv->pvxd_priv->available_chnl, &priv->pvxd_priv->available_chnl_num, 60);
	}
#endif
}

void rtl8192cd_ch64_timer(unsigned long task_priv)
{
	struct rtl8192cd_priv *priv = (struct rtl8192cd_priv *)task_priv;

	if (!(priv->drv_state & DRV_STATE_OPEN))
		return;

	if (((priv->pmib->dot11StationConfigEntry.dot11RegDomain == DOMAIN_MKK) ||
		 (priv->pmib->dot11StationConfigEntry.dot11RegDomain == DOMAIN_MKK3)) &&
		(OPMODE & WIFI_ADHOC_STATE))
		return;
	if (RemoveChannel(priv, priv->NOP_chnl, &priv->NOP_chnl_num, 64)) {
		if (priv->pmib->dot11BssType.net_work_type & WIRELESS_11A)
			InsertChannel(priv->available_chnl, &priv->available_chnl_num, 64);
		DEBUG_INFO("Channel 64 is released!\n");
	}
#ifdef UNIVERSAL_REPEATER
	if (IS_DRV_OPEN(GET_VXD_PRIV(priv)) && RemoveChannel(priv->pvxd_priv, priv->pvxd_priv->NOP_chnl, &priv->pvxd_priv->NOP_chnl_num, 64)) {
		if (priv->pvxd_priv->pmib->dot11BssType.net_work_type & WIRELESS_11A)
			InsertChannel(priv->pvxd_priv->available_chnl, &priv->pvxd_priv->available_chnl_num, 64);
	}
#endif
}

void rtl8192cd_ch100_timer(unsigned long task_priv)
{
	struct rtl8192cd_priv *priv = (struct rtl8192cd_priv *)task_priv;

	if (!(priv->drv_state & DRV_STATE_OPEN))
		return;

	if (RemoveChannel(priv, priv->NOP_chnl, &priv->NOP_chnl_num, 100)) {
		if (priv->pmib->dot11BssType.net_work_type & WIRELESS_11A)
			InsertChannel(priv->available_chnl, &priv->available_chnl_num, 100);
		DEBUG_INFO("Channel 100 is released!\n");
#ifdef UNIVERSAL_REPEATER
	if (IS_DRV_OPEN(GET_VXD_PRIV(priv)) && RemoveChannel(priv->pvxd_priv, priv->pvxd_priv->NOP_chnl, &priv->pvxd_priv->NOP_chnl_num, 100)) {
		if (priv->pvxd_priv->pmib->dot11BssType.net_work_type & WIRELESS_11A)
			InsertChannel(priv->pvxd_priv->available_chnl, &priv->pvxd_priv->available_chnl_num, 100);
	}
#endif
	}
}

void rtl8192cd_ch104_timer(unsigned long task_priv)
{
	struct rtl8192cd_priv *priv = (struct rtl8192cd_priv *)task_priv;

	if (!(priv->drv_state & DRV_STATE_OPEN))
		return;

	if (RemoveChannel(priv, priv->NOP_chnl, &priv->NOP_chnl_num, 104)) {
		if (priv->pmib->dot11BssType.net_work_type & WIRELESS_11A)
			InsertChannel(priv->available_chnl, &priv->available_chnl_num, 104);
		DEBUG_INFO("Channel 104 is released!\n");
	}
#ifdef UNIVERSAL_REPEATER
	if (IS_DRV_OPEN(GET_VXD_PRIV(priv)) && RemoveChannel(priv->pvxd_priv, priv->pvxd_priv->NOP_chnl, &priv->pvxd_priv->NOP_chnl_num, 104)) {
		if (priv->pvxd_priv->pmib->dot11BssType.net_work_type & WIRELESS_11A)
			InsertChannel(priv->pvxd_priv->available_chnl, &priv->pvxd_priv->available_chnl_num, 104);
	}
#endif
}

void rtl8192cd_ch108_timer(unsigned long task_priv)
{
	struct rtl8192cd_priv *priv = (struct rtl8192cd_priv *)task_priv;

	if (!(priv->drv_state & DRV_STATE_OPEN))
		return;

	if (RemoveChannel(priv, priv->NOP_chnl, &priv->NOP_chnl_num, 108)) {
		if (priv->pmib->dot11BssType.net_work_type & WIRELESS_11A)
			InsertChannel(priv->available_chnl, &priv->available_chnl_num, 108);
		DEBUG_INFO("Channel 108 is released!\n");
	}
#ifdef UNIVERSAL_REPEATER
	if (IS_DRV_OPEN(GET_VXD_PRIV(priv)) && RemoveChannel(priv->pvxd_priv, priv->pvxd_priv->NOP_chnl, &priv->pvxd_priv->NOP_chnl_num, 108)) {
		if (priv->pvxd_priv->pmib->dot11BssType.net_work_type & WIRELESS_11A)
			InsertChannel(priv->pvxd_priv->available_chnl, &priv->pvxd_priv->available_chnl_num, 108);
	}
#endif
}

void rtl8192cd_ch112_timer(unsigned long task_priv)
{
	struct rtl8192cd_priv *priv = (struct rtl8192cd_priv *)task_priv;

	if (!(priv->drv_state & DRV_STATE_OPEN))
		return;

	if (RemoveChannel(priv, priv->NOP_chnl, &priv->NOP_chnl_num, 112)) {
		if (priv->pmib->dot11BssType.net_work_type & WIRELESS_11A)
			InsertChannel(priv->available_chnl, &priv->available_chnl_num, 112);
		DEBUG_INFO("Channel 112 is released!\n");
	}
#ifdef UNIVERSAL_REPEATER
	if (IS_DRV_OPEN(GET_VXD_PRIV(priv)) && RemoveChannel(priv->pvxd_priv, priv->pvxd_priv->NOP_chnl, &priv->pvxd_priv->NOP_chnl_num, 112)) {
		if (priv->pvxd_priv->pmib->dot11BssType.net_work_type & WIRELESS_11A)
			InsertChannel(priv->pvxd_priv->available_chnl, &priv->pvxd_priv->available_chnl_num, 112);
	}
#endif
}

void rtl8192cd_ch116_timer(unsigned long task_priv)
{
	struct rtl8192cd_priv *priv = (struct rtl8192cd_priv *)task_priv;

	if (!(priv->drv_state & DRV_STATE_OPEN))
		return;

	if (RemoveChannel(priv, priv->NOP_chnl, &priv->NOP_chnl_num, 116)) {
		if (priv->pmib->dot11BssType.net_work_type & WIRELESS_11A)
			InsertChannel(priv->available_chnl, &priv->available_chnl_num, 116);
		DEBUG_INFO("Channel 116 is released!\n");
	}
#ifdef UNIVERSAL_REPEATER
	if (IS_DRV_OPEN(GET_VXD_PRIV(priv)) && RemoveChannel(priv->pvxd_priv, priv->pvxd_priv->NOP_chnl, &priv->pvxd_priv->NOP_chnl_num, 116)) {
		if (priv->pvxd_priv->pmib->dot11BssType.net_work_type & WIRELESS_11A)
			InsertChannel(priv->pvxd_priv->available_chnl, &priv->pvxd_priv->available_chnl_num, 116);
	}
#endif
}

void rtl8192cd_ch120_timer(unsigned long task_priv)
{
	struct rtl8192cd_priv *priv = (struct rtl8192cd_priv *)task_priv;

	if (!(priv->drv_state & DRV_STATE_OPEN))
		return;

	if (RemoveChannel(priv, priv->NOP_chnl, &priv->NOP_chnl_num, 120)) {
		if (priv->pmib->dot11BssType.net_work_type & WIRELESS_11A)
			InsertChannel(priv->available_chnl, &priv->available_chnl_num, 120);
		DEBUG_INFO("Channel 120 is released!\n");
	}
#ifdef UNIVERSAL_REPEATER
	if (IS_DRV_OPEN(GET_VXD_PRIV(priv)) && RemoveChannel(priv->pvxd_priv, priv->pvxd_priv->NOP_chnl, &priv->pvxd_priv->NOP_chnl_num, 120)) {
		if (priv->pvxd_priv->pmib->dot11BssType.net_work_type & WIRELESS_11A)
			InsertChannel(priv->pvxd_priv->available_chnl, &priv->pvxd_priv->available_chnl_num, 120);
	}
#endif
}

void rtl8192cd_ch124_timer(unsigned long task_priv)
{
	struct rtl8192cd_priv *priv = (struct rtl8192cd_priv *)task_priv;

	if (!(priv->drv_state & DRV_STATE_OPEN))
		return;

	if (RemoveChannel(priv, priv->NOP_chnl, &priv->NOP_chnl_num, 124)) {
		if (priv->pmib->dot11BssType.net_work_type & WIRELESS_11A)
			InsertChannel(priv->available_chnl, &priv->available_chnl_num, 124);
		DEBUG_INFO("Channel 124 is released!\n");
	}
#ifdef UNIVERSAL_REPEATER
	if (IS_DRV_OPEN(GET_VXD_PRIV(priv)) && RemoveChannel(priv->pvxd_priv, priv->pvxd_priv->NOP_chnl, &priv->pvxd_priv->NOP_chnl_num, 124)) {
		if (priv->pvxd_priv->pmib->dot11BssType.net_work_type & WIRELESS_11A)
			InsertChannel(priv->pvxd_priv->available_chnl, &priv->pvxd_priv->available_chnl_num, 124);
	}
#endif
}

void rtl8192cd_ch128_timer(unsigned long task_priv)
{
	struct rtl8192cd_priv *priv = (struct rtl8192cd_priv *)task_priv;

	if (!(priv->drv_state & DRV_STATE_OPEN))
		return;

	if (RemoveChannel(priv, priv->NOP_chnl, &priv->NOP_chnl_num, 128)) {
		if (priv->pmib->dot11BssType.net_work_type & WIRELESS_11A)
			InsertChannel(priv->available_chnl, &priv->available_chnl_num, 128);
		DEBUG_INFO("Channel 128 is released!\n");
	}
#ifdef UNIVERSAL_REPEATER
	if (IS_DRV_OPEN(GET_VXD_PRIV(priv)) && RemoveChannel(priv->pvxd_priv, priv->pvxd_priv->NOP_chnl, &priv->pvxd_priv->NOP_chnl_num, 128)) {
		if (priv->pvxd_priv->pmib->dot11BssType.net_work_type & WIRELESS_11A)
			InsertChannel(priv->pvxd_priv->available_chnl, &priv->pvxd_priv->available_chnl_num, 128);
	}
#endif
}

void rtl8192cd_ch132_timer(unsigned long task_priv)
{
	struct rtl8192cd_priv *priv = (struct rtl8192cd_priv *)task_priv;

	if (!(priv->drv_state & DRV_STATE_OPEN))
		return;

	if (RemoveChannel(priv, priv->NOP_chnl, &priv->NOP_chnl_num, 132)) {
		if (priv->pmib->dot11BssType.net_work_type & WIRELESS_11A)
			InsertChannel(priv->available_chnl, &priv->available_chnl_num, 132);
		DEBUG_INFO("Channel 132 is released!\n");
	}
#ifdef UNIVERSAL_REPEATER
	if (IS_DRV_OPEN(GET_VXD_PRIV(priv)) && RemoveChannel(priv->pvxd_priv, priv->pvxd_priv->NOP_chnl, &priv->pvxd_priv->NOP_chnl_num, 132)) {
		if (priv->pvxd_priv->pmib->dot11BssType.net_work_type & WIRELESS_11A)
			InsertChannel(priv->pvxd_priv->available_chnl, &priv->pvxd_priv->available_chnl_num, 132);
	}
#endif
}

void rtl8192cd_ch136_timer(unsigned long task_priv)
{
	struct rtl8192cd_priv *priv = (struct rtl8192cd_priv *)task_priv;

	if (!(priv->drv_state & DRV_STATE_OPEN))
		return;

	if (RemoveChannel(priv, priv->NOP_chnl, &priv->NOP_chnl_num, 136)) {
		if (priv->pmib->dot11BssType.net_work_type & WIRELESS_11A)
			InsertChannel(priv->available_chnl, &priv->available_chnl_num, 136);
		DEBUG_INFO("Channel 136 is released!\n");
	}
#ifdef UNIVERSAL_REPEATER
	if (IS_DRV_OPEN(GET_VXD_PRIV(priv)) && RemoveChannel(priv->pvxd_priv, priv->pvxd_priv->NOP_chnl, &priv->pvxd_priv->NOP_chnl_num, 136)) {
		if (priv->pvxd_priv->pmib->dot11BssType.net_work_type & WIRELESS_11A)
			InsertChannel(priv->pvxd_priv->available_chnl, &priv->pvxd_priv->available_chnl_num, 136);
	}
#endif
}

void rtl8192cd_ch140_timer(unsigned long task_priv)
{
	struct rtl8192cd_priv *priv = (struct rtl8192cd_priv *)task_priv;

	if (!(priv->drv_state & DRV_STATE_OPEN))
		return;

	if (RemoveChannel(priv, priv->NOP_chnl, &priv->NOP_chnl_num, 140)) {
		if (priv->pmib->dot11BssType.net_work_type & WIRELESS_11A)
			InsertChannel(priv->available_chnl, &priv->available_chnl_num, 140);
		DEBUG_INFO("Channel 140 is released!\n");
	}
#ifdef UNIVERSAL_REPEATER
	if (IS_DRV_OPEN(GET_VXD_PRIV(priv)) && RemoveChannel(priv->pvxd_priv, priv->pvxd_priv->NOP_chnl, &priv->pvxd_priv->NOP_chnl_num, 140)) {
		if (priv->pvxd_priv->pmib->dot11BssType.net_work_type & WIRELESS_11A)
			InsertChannel(priv->pvxd_priv->available_chnl, &priv->pvxd_priv->available_chnl_num, 140);
	}
#endif
}

void rtl8192cd_ch144_timer(unsigned long task_priv)
{
	struct rtl8192cd_priv *priv = (struct rtl8192cd_priv *)task_priv;

	if (!(priv->drv_state & DRV_STATE_OPEN))
		return;

	if (RemoveChannel(priv, priv->NOP_chnl, &priv->NOP_chnl_num, 144)) {
		if (priv->pmib->dot11BssType.net_work_type & WIRELESS_11A)
			InsertChannel(priv->available_chnl, &priv->available_chnl_num, 144);
		DEBUG_INFO("Channel 144 is released!\n");
	}
#ifdef UNIVERSAL_REPEATER
	if (IS_DRV_OPEN(GET_VXD_PRIV(priv)) && RemoveChannel(priv->pvxd_priv, priv->pvxd_priv->NOP_chnl, &priv->pvxd_priv->NOP_chnl_num, 144)) {
		if (priv->pvxd_priv->pmib->dot11BssType.net_work_type & WIRELESS_11A)
			InsertChannel(priv->pvxd_priv->available_chnl, &priv->pvxd_priv->available_chnl_num, 144);
}
#endif
}

unsigned int DFS_SelectChannel(struct rtl8192cd_priv *priv)
{
    unsigned int random;
    unsigned int num, random_base, which_channel = -1;
    int reg = priv->pmib->dot11StationConfigEntry.dot11RegDomain;	

    if(priv->pmib->dot11nConfigEntry.dot11nUse40M == HT_CHANNEL_WIDTH_80){ 
        // When user select band 3 with 80M channel bandwidth 
        which_channel = find80MChannel(priv->available_chnl,priv->available_chnl_num);

        if(which_channel == -1) // select non-DFS band 80M (ch 36 or 149) or down to 40/20M
        {
#ifdef __ECOS
            // generate random number
            {
            unsigned int random_buf[4];
            get_random_bytes(random_buf, 4);
            random = random_buf[3];
            }
#else
            get_random_bytes(&random, 4);
#endif
            if(random % 2 == 0){ // try ch155 if not available choose ch36
                if(is80MChannel(priv->available_chnl, priv->available_chnl_num, 149)){
                    which_channel = 149;
                }
                else{
                    if(is80MChannel(priv->available_chnl, priv->available_chnl_num, 36)){
                        which_channel = 36;
                    }
                }
            }
            else{
                if(is80MChannel(priv->available_chnl, priv->available_chnl_num, 36)){
                    which_channel = 36;
                }
                else{
                    if(is80MChannel(priv->available_chnl, priv->available_chnl_num, 149)){
                        which_channel = 149;
                    }
                }
            }
        }
    }


    if(priv->pmib->dot11nConfigEntry.dot11nUse40M == HT_CHANNEL_WIDTH_80 || 
        priv->pmib->dot11nConfigEntry.dot11nUse40M == HT_CHANNEL_WIDTH_20_40) {
        if(which_channel == -1){ // down to 40M
            priv->pmib->dot11nConfigEntry.dot11nUse40M = HT_CHANNEL_WIDTH_20_40;    
            which_channel = find40MChannel(priv->available_chnl,priv->available_chnl_num);
        }
    }


    if(which_channel == -1){ // down to 20M
        priv->pmib->dot11nConfigEntry.dot11nUse40M = HT_CHANNEL_WIDTH_20;        
#ifdef __ECOS
        unsigned int random_buf[4];
        get_random_bytes(random_buf, 4);
        random = random_buf[3];
#else
        get_random_bytes(&random, 4);
#endif
        if(priv->available_chnl_num){
            num = random % priv->available_chnl_num;
            which_channel = priv->available_chnl[num];
        }else{
            which_channel = 0;
        }
    }

    return which_channel;
}


//insert the channel into the channel list
//if successful, return 1, else return 0
int InsertChannel(unsigned int chnl_list[], unsigned int *chnl_num, unsigned int channel)
{
	unsigned int i, j;

	if (*chnl_num==0) {
		chnl_list[0] = channel;
		(*chnl_num)++;
		return SUCCESS;
	}

	for (i=0; i < *chnl_num; i++) {
		if (chnl_list[i] == channel) {
			_DEBUG_INFO("Inserting channel failed: channel %d already exists!\n", channel);
			return FAIL;
		} else if (chnl_list[i] > channel) {
			break;
		}
	}

	if (i == *chnl_num) {
		chnl_list[(*chnl_num)++] = channel;
	} else {
		for (j=*chnl_num; j > i; j--)
			chnl_list[j] = chnl_list[j-1];
		chnl_list[j] = channel;
		(*chnl_num)++;
	}

	return SUCCESS;
}


/*
 *	remove the channel from the channel list
 *	if successful, return 1, else return 0
 */
int RemoveChannel(struct rtl8192cd_priv *priv, unsigned int chnl_list[], unsigned int *chnl_num, unsigned int channel)
{
	unsigned int i, j;

	if (*chnl_num) {
		for (i=0; i < *chnl_num; i++)
			if (channel == chnl_list[i])
				break;
		if (i == *chnl_num)  {
			_DEBUG_INFO("Can not remove channel %d!\n", channel);
			return FAIL;
		} else {
			for (j=i; j < (*chnl_num-1); j++)
				chnl_list[j] = chnl_list[j+1];
			(*chnl_num)--;
			return SUCCESS;
		}
	} else {
		_DEBUG_INFO("Can not remove channel %d!\n", channel);
		return FAIL;
	}
}

void DFS_SwChnl_clnt(struct rtl8192cd_priv *priv)
{
	/* signin non-DFS channel */
	priv->pmib->dot11RFEntry.dot11channel = priv->pshare->dfsSwitchChannel;
	priv->pshare->dfsSwitchChannel = 0;
	RTL_W8(TXPAUSE, 0xff);
	panic_printk("1. Swiching channel to %d!\n", priv->pmib->dot11RFEntry.dot11channel);
	reload_txpwr_pg(priv);
	SwChnl(priv, priv->pmib->dot11RFEntry.dot11channel, priv->pshare->offset_2nd_chan);
	RTL_W8(TXPAUSE, 0x00);

	if (is_DFS_channel(priv->pmib->dot11RFEntry.dot11channel)) {
			panic_printk("Switched to DFS band (ch %d) again!!\n", priv->pmib->dot11RFEntry.dot11channel);
	 }

#ifdef CONFIG_RTL_92D_SUPPORT
	if ((GET_CHIP_VER(priv) == VERSION_8192D) && (priv->pmib->dot11Bss.channel > 14)) {
		priv->pshare->iqk_5g_done = 0;
		PHY_IQCalibrate(priv);
	}
#endif
}


void DFS_SwitchChannel(struct rtl8192cd_priv *priv)
{
	int ch = priv->pshare->dfsSwitchChannel;		
	
	if (!(priv->drv_state & DRV_STATE_OPEN)) 
		return;
	priv->pmib->dot11RFEntry.dot11channel = ch;
	priv->pshare->dfsSwitchChannel = 0;
	RTL_W8(TXPAUSE, 0xff);

	DEBUG_INFO("2. Swiching channel to %d!\n", priv->pmib->dot11RFEntry.dot11channel);
	priv->pshare->CurrentChannelBW = priv->pshare->is_40m_bw = priv->pmib->dot11nConfigEntry.dot11nUse40M;
	
	if( (ch>144) ? ((ch-1)%8) : (ch%8)) {
		GET_MIB(priv)->dot11nConfigEntry.dot11n2ndChOffset = HT_2NDCH_OFFSET_ABOVE;
		priv->pshare->offset_2nd_chan	= HT_2NDCH_OFFSET_ABOVE;
	} else {
		GET_MIB(priv)->dot11nConfigEntry.dot11n2ndChOffset = HT_2NDCH_OFFSET_BELOW;
		priv->pshare->offset_2nd_chan	= HT_2NDCH_OFFSET_BELOW;
	}

	//priv->pshare->No_RF_Write = 0;
	SwBWMode(priv, priv->pshare->CurrentChannelBW, priv->pshare->offset_2nd_chan);
	SwChnl(priv, priv->pmib->dot11RFEntry.dot11channel, priv->pshare->offset_2nd_chan);

	//priv->pshare->No_RF_Write = 1;
	GET_ROOT(priv)->pmib->dot11DFSEntry.DFS_detected = priv->pshare->dfsSwitchChannel = 0;
	GET_ROOT(priv)->pmib->dot11DFSEntry.disable_tx = priv->pmib->dot11DFSEntry.disable_tx = 0;
	priv->ht_cap_len = 0;
	update_beacon(priv);
	RTL_W8(TXPAUSE, 0x00);
}


void rtl8192cd_dfs_det_chk_timer(unsigned long task_priv)
{
	struct rtl8192cd_priv *priv = (struct rtl8192cd_priv *)task_priv;
	unsigned long throughput = 0;
	int j;

	if (!(priv->drv_state & DRV_STATE_OPEN)) 
		return;
	priv->ini_gain_cur = RTL_R8(0xc50);

	priv->st_L2H_cur = PHY_QueryBBReg(priv, 0x91c, 0x000000ff);

	throughput = priv->ext_stats.tx_avarage+priv->ext_stats.rx_avarage;
#ifdef UNIVERSAL_REPEATER
	if (GET_VXD_PRIV(priv) && (GET_VXD_PRIV(priv)->drv_state & DRV_STATE_OPEN))
		throughput += (GET_VXD_PRIV(priv)->ext_stats.tx_avarage + GET_VXD_PRIV(priv)->ext_stats.rx_avarage);
#endif

#ifdef MBSSID
	if (priv->pmib->miscEntry.vap_enable) {
		for (j=0; j<RTL8192CD_NUM_VWLAN; j++) {
			if (IS_DRV_OPEN(priv->pvap_priv[j])) {
				throughput += priv->pvap_priv[j]->ext_stats.tx_avarage+priv->pvap_priv[j]->ext_stats.rx_avarage;
			}
		}
	}
#endif

	if ((throughput >> 17) <= priv->pshare->rf_ft_var.dfs_psd_tp_th) {
		priv->idle_flag = 1;
		if(priv->pshare->rf_ft_var.dfs_force_TP_mode)
			priv->idle_flag = 0;
	}
	else{
		priv->idle_flag = 0;
	}

#if !defined(CONFIG_RTL_92D_SUPPORT)
	// dfs_det.c
	if((GET_CHIP_VER(priv) == VERSION_8812E) || (GET_CHIP_VER(priv) == VERSION_8881A) || (GET_CHIP_VER(priv) == VERSION_8814A) || (GET_CHIP_VER(priv) == VERSION_8822B))
	{
		if (priv->pshare->rf_ft_var.dfs_det_off == 0) {
					// debug print 
#if defined(CONFIG_WLAN_HAL_8814AE) || defined(CONFIG_WLAN_HAL_8822BE)
			if((GET_CHIP_VER(priv) == VERSION_8814A) || (GET_CHIP_VER(priv) == VERSION_8822B)){
				if(priv->pshare->rf_ft_var.dfs_radar_diff_on){
					dfs_histogram_radar_distinguish(priv);
				}
			}
#endif
			rtl8192cd_dfs_det_chk(priv);
			rtl8192cd_dfs_dynamic_setting(priv);
		}
	}
#endif

	// dynamic pwdb calibration
	if (priv->ini_gain_pre != priv->ini_gain_cur) {
		priv->pwdb_th = ((int)priv->st_L2H_cur - (int)priv->ini_gain_cur)/2 + priv->pshare->rf_ft_var.dfs_pwdb_scalar_factor;
		priv->pwdb_th = MAX_NUM(priv->pwdb_th, (int)priv->pshare->rf_ft_var.dfs_pwdb_th); // limit the pwdb value to absoulte lower bound 0xa
		priv->pwdb_th = MIN_NUM(priv->pwdb_th, 0x1f);    // limit the pwdb value to absoulte upper bound 0x1f
		PHY_SetBBReg(priv, 0x918, 0x00001f00, priv->pwdb_th);
	}

	priv->ini_gain_pre = priv->ini_gain_cur;

	if(((priv->pmib->dot11RFEntry.dot11channel >= 52) &&
					(priv->pmib->dot11RFEntry.dot11channel <= 64)) && (priv->pshare->rf_ft_var.manual_dfs_regdomain == 2)){
		// MKK w53
		mod_timer(&priv->dfs_det_chk_timer, jiffies + RTL_MILISECONDS_TO_JIFFIES(priv->pshare->rf_ft_var.dfs_det_period_jp_w53*10));
	}
	else{
		mod_timer(&priv->dfs_det_chk_timer, jiffies + RTL_MILISECONDS_TO_JIFFIES(priv->pshare->rf_ft_var.dfs_det_period*10));
	}
}


void DFS_SetReg(struct rtl8192cd_priv *priv)
{
	int short_pulse_width_upperbound;
	int reg91c_value;
	// DFS region domain setting
	// by regdomain
	if (priv->pshare->rf_ft_var.manual_dfs_regdomain == 0){
		if (priv->pmib->dot11StationConfigEntry.dot11RegDomain == DOMAIN_FCC){
			// FCC
			priv->pshare->rf_ft_var.manual_dfs_regdomain = 1;
		}else if(priv->pmib->dot11StationConfigEntry.dot11RegDomain == DOMAIN_MKK || priv->pmib->dot11StationConfigEntry.dot11RegDomain == DOMAIN_MKK1 || priv->pmib->dot11StationConfigEntry.dot11RegDomain == DOMAIN_MKK2 || priv->pmib->dot11StationConfigEntry.dot11RegDomain == DOMAIN_MKK3){
			// MKK
			priv->pshare->rf_ft_var.manual_dfs_regdomain = 2;
		}else{
			// ETSI
			priv->pshare->rf_ft_var.manual_dfs_regdomain = 3;
		}		
	}
	// by manual_dfs_regdomain mib value
	else{		
	}
	//if (GET_CHIP_VER(priv) == VERSION_8822B)
		//priv->pshare->rf_ft_var.dfs_dpt_st_l2h_min = 0x26;
	
	if (!(priv->drv_state & DRV_STATE_OPEN)) 
		return;
	if (GET_CHIP_VER(priv) == VERSION_8192D) {
		PHY_SetBBReg(priv, 0xc38, BIT(23) | BIT(22), 2);
		PHY_SetBBReg(priv, 0x814, bMaskDWord, 0x04cc4d10);
	}
	else if ((GET_CHIP_VER(priv) == VERSION_8812E) || (GET_CHIP_VER(priv) == VERSION_8881A) || (GET_CHIP_VER(priv) == VERSION_8814A) || (GET_CHIP_VER(priv) == VERSION_8822B)) {
		PHY_SetBBReg(priv, 0x814, 0x3fffffff, 0x04cc4d10);
		PHY_SetBBReg(priv, 0x834, bMaskByte0, 0x06);
		
		// 8822B only, when BW = 20M, DFIR ouput is 40Mhz, but DFS input is 80MMHz, so it need to upgrade to 80MHz
		if(GET_CHIP_VER(priv) == VERSION_8822B){
			if(priv->pmib->dot11nConfigEntry.dot11nUse40M == HT_CHANNEL_WIDTH_20)
				PHY_SetBBReg(priv, 0x1984, BIT(26), 1);
			else
				PHY_SetBBReg(priv, 0x1984, BIT(26), 0);
		}
		
		// FCC
		if (priv->pshare->rf_ft_var.manual_dfs_regdomain == 1) {
			if(priv->pmib->dot11nConfigEntry.dot11nUse40M == HT_CHANNEL_WIDTH_20){
				PHY_SetBBReg(priv, 0x91c, bMaskDWord, 0x64741a20);
			}
			else{
				PHY_SetBBReg(priv, 0x91c, bMaskDWord, 0x62741a20);
			}
			if(priv->pshare->rf_ft_var.dfs_det_off == 1){
				if (GET_CHIP_VER(priv) == VERSION_8814A || (GET_CHIP_VER(priv) == VERSION_8822B)){
					PHY_SetBBReg(priv, 0x918, bMaskDWord, 0x1c166cdf);
					PHY_SetBBReg(priv, 0x924, 0xfffffff, 0x095a8400);
				}
				else{
					PHY_SetBBReg(priv, 0x918, bMaskDWord, 0x1c16acdf);
					PHY_SetBBReg(priv, 0x924, bMaskDWord, 0x01528400);
				}				
				PHY_SetBBReg(priv, 0x920, bMaskDWord, 0xe0967231);
			}
			else{
				if (GET_CHIP_VER(priv) == VERSION_8814A || (GET_CHIP_VER(priv) == VERSION_8822B)){
					PHY_SetBBReg(priv, 0x918, bMaskDWord, 0x1c176cdf);
					PHY_SetBBReg(priv, 0x924, 0xfffffff, 0x095a8400);
				}
				else{
					PHY_SetBBReg(priv, 0x918, bMaskDWord, 0x1c17acdf);
					PHY_SetBBReg(priv, 0x924, bMaskDWord, 0x01528400);
				}				
				PHY_SetBBReg(priv, 0x920, bMaskDWord, 0xe096d231);
			}
		}
		// ETSI
		else if (priv->pshare->rf_ft_var.manual_dfs_regdomain == 3) {
			if(priv->pshare->rf_ft_var.dfs_det_off == 1){
				if ((GET_CHIP_VER(priv) == VERSION_8814A) || (GET_CHIP_VER(priv) == VERSION_8822B)){
					PHY_SetBBReg(priv, 0x918, bMaskDWord, 0x1c16acdf);
					PHY_SetBBReg(priv, 0x924, 0xfffffff, 0x095a8400);
				}
				else{
					PHY_SetBBReg(priv, 0x918, bMaskDWord, 0x1c16ecdf);
					PHY_SetBBReg(priv, 0x924, bMaskDWord, 0x01528400);
				}
				PHY_SetBBReg(priv, 0x91c, bMaskDWord, 0x0fa21a20);				
				PHY_SetBBReg(priv, 0x920, bMaskDWord, 0xe0f57204);
			}
			else{
				if(IS_METEOROLOGY_CHANNEL(priv->pmib->dot11RFEntry.dot11channel)){
					if(priv->pmib->dot11nConfigEntry.dot11nUse40M == HT_CHANNEL_WIDTH_80 || priv->pmib->dot11nConfigEntry.dot11nUse40M == HT_CHANNEL_WIDTH_20){
						if ((GET_CHIP_VER(priv) == VERSION_8814A) || (GET_CHIP_VER(priv) == VERSION_8822B)){
							PHY_SetBBReg(priv, 0x918, bMaskDWord, 0x1c16acdf);
							PHY_SetBBReg(priv, 0x924, 0xfffffff, 0x095a8400);
						}
						else{
							PHY_SetBBReg(priv, 0x918, bMaskDWord, 0x1c16ecdf);
							PHY_SetBBReg(priv, 0x924, bMaskDWord, 0x01528400);
						}
						PHY_SetBBReg(priv, 0x91c, bMaskDWord, 0x0fa21a20);						
						PHY_SetBBReg(priv, 0x920, bMaskDWord, 0xe0f70604);
					}
					else{
						if ((GET_CHIP_VER(priv) == VERSION_8814A) || (GET_CHIP_VER(priv) == VERSION_8822B)){
							PHY_SetBBReg(priv, 0x918, bMaskDWord, 0x1c16acdf);
							PHY_SetBBReg(priv, 0x924, 0xfffffff, 0x095a8400);
						}
						else{
							PHY_SetBBReg(priv, 0x918, bMaskDWord, 0x1c16ecdf);
							PHY_SetBBReg(priv, 0x924, bMaskDWord, 0x01528400);
						}
						PHY_SetBBReg(priv, 0x91c, bMaskDWord, 0x0fa21a20);						
						PHY_SetBBReg(priv, 0x920, bMaskDWord, 0xe0f70604);
					}
					priv->ch_120_132_CAC_end = 0;
				}
				else{
					if ((GET_CHIP_VER(priv) == VERSION_8814A) || (GET_CHIP_VER(priv) == VERSION_8822B)){
						PHY_SetBBReg(priv, 0x918, bMaskDWord, 0x1c17acdf);
						PHY_SetBBReg(priv, 0x924, 0xfffffff, 0x095a8400);						
					}
					else{
						PHY_SetBBReg(priv, 0x918, bMaskDWord, 0x1c17ecdf);
						PHY_SetBBReg(priv, 0x924, bMaskDWord, 0x01528400);						
					}
					PHY_SetBBReg(priv, 0x920, bMaskDWord, 0xe0f70604);
					PHY_SetBBReg(priv, 0x91c, bMaskDWord, 0x0fc21a20);					
				}
			}
		}
                // MKK
		else if(priv->pshare->rf_ft_var.manual_dfs_regdomain == 2){		
			if(priv->pshare->rf_ft_var.dfs_det_off == 1){
				if ((GET_CHIP_VER(priv) == VERSION_8814A) || (GET_CHIP_VER(priv) == VERSION_8822B)){
					PHY_SetBBReg(priv, 0x924, 0xfffffff, 0x095aa400);
				}
				else{
					PHY_SetBBReg(priv, 0x924, bMaskDWord, 0x0152a400);
				}				
				PHY_SetBBReg(priv, 0x920, bMaskDWord, 0xe0d67234);
				if((priv->pmib->dot11RFEntry.dot11channel >= 52) &&
					(priv->pmib->dot11RFEntry.dot11channel <= 64)){
						PHY_SetBBReg(priv, 0x918, bMaskDWord, 0x1c16ecdf);
						PHY_SetBBReg(priv, 0x91c, bMaskDWord, 0x0f141a20);
				}
				else{
					if(priv->pmib->dot11nConfigEntry.dot11nUse40M == HT_CHANNEL_WIDTH_20){
						PHY_SetBBReg(priv, 0x91c, bMaskDWord, 0x64721a20);
					}
					else{
						PHY_SetBBReg(priv, 0x91c, bMaskDWord, 0x68721a20);
					}
					if ((GET_CHIP_VER(priv) == VERSION_8814A) || (GET_CHIP_VER(priv) == VERSION_8822B)){
						PHY_SetBBReg(priv, 0x918, bMaskDWord, 0x1c166cdf);
					}
					else{
						PHY_SetBBReg(priv, 0x918, bMaskDWord, 0x1c16acdf);					
					}
				}
			}
			else{
				if((priv->pmib->dot11RFEntry.dot11channel >= 52) &&
					(priv->pmib->dot11RFEntry.dot11channel <= 64)){
					PHY_SetBBReg(priv, 0x920, bMaskDWord, 0xe0fe6a34);
					PHY_SetBBReg(priv, 0x91c, bMaskDWord, 0x0f141a20);					
					PHY_SetBBReg(priv, 0x918, bMaskDWord, 0x1c17ecdf);
					if ((GET_CHIP_VER(priv) == VERSION_8814A) || (GET_CHIP_VER(priv) == VERSION_8822B)){
						PHY_SetBBReg(priv, 0x924, 0xfffffff, 0x095a8400);
					}
					else{
						PHY_SetBBReg(priv, 0x924, bMaskDWord, 0x01528400);
					}
				}
				else{
					PHY_SetBBReg(priv, 0x920, bMaskDWord, 0xe0a67234);					
					if ((GET_CHIP_VER(priv) == VERSION_8814A) || (GET_CHIP_VER(priv) == VERSION_8822B)){						
						PHY_SetBBReg(priv, 0x918, bMaskDWord, 0x1c176cdf);
						PHY_SetBBReg(priv, 0x924, 0xfffffff, 0x095a8400);
					}
					else{						
						PHY_SetBBReg(priv, 0x918, bMaskDWord, 0x1c17acdf);
						PHY_SetBBReg(priv, 0x924, bMaskDWord, 0x01528400);
					}
					if(priv->pmib->dot11nConfigEntry.dot11nUse40M == HT_CHANNEL_WIDTH_20){
						PHY_SetBBReg(priv, 0x91c, bMaskDWord, 0x60721a20);
					}
					else{
						PHY_SetBBReg(priv, 0x91c, bMaskDWord, 0x62721a20);
					}							
				}
			}
		}
		else{   // default: FCC
			if(priv->pmib->dot11nConfigEntry.dot11nUse40M == HT_CHANNEL_WIDTH_20){
				PHY_SetBBReg(priv, 0x91c, bMaskDWord, 0x64741a20);
			}
			else{
				PHY_SetBBReg(priv, 0x91c, bMaskDWord, 0x62741a20);
			}
			if(priv->pshare->rf_ft_var.dfs_det_off == 1){
				if ((GET_CHIP_VER(priv) == VERSION_8814A) || (GET_CHIP_VER(priv) == VERSION_8822B)){
					PHY_SetBBReg(priv, 0x918, bMaskDWord, 0x1c166cdf);
					PHY_SetBBReg(priv, 0x924, 0xfffffff, 0x095a8400);
				}
				else{
					PHY_SetBBReg(priv, 0x918, bMaskDWord, 0x1c16acdf);
					PHY_SetBBReg(priv, 0x924, bMaskDWord, 0x01528400);
				}				
				PHY_SetBBReg(priv, 0x920, bMaskDWord, 0xe0967231);
			}
			else{
				if ((GET_CHIP_VER(priv) == VERSION_8814A) || (GET_CHIP_VER(priv) == VERSION_8822B)){
					PHY_SetBBReg(priv, 0x918, bMaskDWord, 0x1c176cdf);
					PHY_SetBBReg(priv, 0x924, 0xfffffff, 0x095a8400);
				}
				else{
					PHY_SetBBReg(priv, 0x918, bMaskDWord, 0x1c17acdf);
					PHY_SetBBReg(priv, 0x924, bMaskDWord, 0x01528400);
				}				
				PHY_SetBBReg(priv, 0x920, bMaskDWord, 0xe096d231);
			}
		}

		priv->st_L2H_cur = PHY_QueryBBReg(priv, 0x91c, 0x000000ff);
		priv->pwdb_th = (int)PHY_QueryBBReg(priv, 0x918, 0x00001f00);
		priv->peak_th = PHY_QueryBBReg(priv, 0x918, 0x00030000);
		priv->short_pulse_cnt_th = PHY_QueryBBReg(priv, 0x920, 0x000f0000);
		priv->long_pulse_cnt_th = PHY_QueryBBReg(priv, 0x920, 0x00f00000);
		priv->peak_window = PHY_QueryBBReg(priv, 0x920, 0x00000300);
		priv->nb2wb_th = PHY_QueryBBReg(priv, 0x920, 0x0000e000);
		priv->three_peak_opt = PHY_QueryBBReg(priv, 0x924, 0x00000180);
		priv->three_peak_th2 = PHY_QueryBBReg(priv, 0x924, 0x00007000);
		
		// RXHP low corner will extend the pulse width, so we need to increase the uppper bound
		if(GET_CHIP_VER(priv) == VERSION_8822B){
			if (PHY_QueryBBReg(priv, 0x8d8, BIT28|BIT27|BIT26) == 0){
				short_pulse_width_upperbound = PHY_QueryBBReg(priv, 0x91c, BIT23|BIT22|BIT21|BIT20);
				if((short_pulse_width_upperbound+4) >15)
					PHY_SetBBReg(priv, 0x91c, BIT23|BIT22|BIT21|BIT20, 15);
				else
					PHY_SetBBReg(priv, 0x91c, BIT23|BIT22|BIT21|BIT20, short_pulse_width_upperbound+4);
			}
			PHY_SetBBReg(priv, 0x19e4, 0x003C0000, 13);      // if peak index -1~+1, use original NB method
			PHY_SetBBReg(priv, 0x924, 0x70000, 0);
		}
		
		if (GET_CHIP_VER(priv) == VERSION_8881A)
			PHY_SetBBReg(priv, 0xb00, 0xc0000000, 3);

		if ((GET_CHIP_VER(priv) == VERSION_8814A) || (GET_CHIP_VER(priv) == VERSION_8822B)){        // for 8814 new dfs mechanism setting
			PHY_SetBBReg(priv, 0x19e4, 0x1fff, 0x0c00);       // turn off dfs  scaling factor
			PHY_SetBBReg(priv, 0x19e4, 0x30000, 1);     //NonDC peak_th = 2times DC peak_th
			PHY_SetBBReg(priv, 0x9f8, 0xc0000000, 3);  // power for debug and auto test flow latch after ST 
			// low pulse width radar pattern will cause wrong drop
			PHY_SetBBReg(priv, 0x9f4, 0x80000000, 0);  // disable peak index should the same during the same short pulse (new mechanism)
			PHY_SetBBReg(priv, 0x924, 0x20000000, 0);  // disable peak index should the same during the same short pulse (old mechanism)
			PHY_SetBBReg(priv, 0x19e4, 0xe000, 2);      // if peak index diff >=2, then drop the result
			// MKK
			if (priv->pshare->rf_ft_var.manual_dfs_regdomain == 2) {
				if((priv->pmib->dot11RFEntry.dot11channel >= 52) && (priv->pmib->dot11RFEntry.dot11channel <= 64)){
					// pulse width hist th setting
					PHY_SetBBReg(priv, 0x19e4, 0xff000000, 2);  // th1=2*04us
					PHY_SetBBReg(priv, 0x19e8, bMaskDWord, 0x22070403); // set th2 = 3*0.4us, th3 = 4*0.4us, th4 = 7*0.4, th5 = 34*0.4		
					// pulse repetition interval hist th setting
					PHY_SetBBReg(priv, 0x19b8, 0x00007f80, 42);  // th1=42*32us
					PHY_SetBBReg(priv, 0x19ec, bMaskDWord, 0x827b732f); // set th2 = 47*32us, th3 = 115*32us, th4 = 123*32us, th5 = 130*32us		
				}
				else{
					// pulse width hist th setting
					PHY_SetBBReg(priv, 0x19e4, 0xff000000, 1);  // th1=1*04us
					PHY_SetBBReg(priv, 0x19e8, bMaskDWord, 0xff4b1a0d); // set th2 = 4*0.4us, th3 = 6*0.4us, th4 = 14*0.4us, th5 = 26*0.4us					
					// pulse repetition interval hist th setting
					PHY_SetBBReg(priv, 0x19b8, 0x00007f80, 4);  // th1=4*32us
					PHY_SetBBReg(priv, 0x19ec, bMaskDWord, 0x80201008); // set th2 = 8*32us, th3 = 16*32us, th4 = 32*32us, th5 = 128*32 = 4096us
				}
			}
			// ETSI
			else if(priv->pshare->rf_ft_var.manual_dfs_regdomain == 3){
				// pulse width hist th setting
				PHY_SetBBReg(priv, 0x19e4, 0xff000000, 1);  // th1=1*04us
				PHY_SetBBReg(priv, 0x19e8, bMaskDWord, 0x68260d06); // set th2 = 25*0.4us, th3 = 44*0.4us, th4, th5 = max
				// pulse repetition interval hist th setting
				PHY_SetBBReg(priv, 0x19b8, 0x00007f80, 7);  // th1=7*32us
				PHY_SetBBReg(priv, 0x19ec, bMaskDWord, 0x9d6e2010); // set th2=40*32us, th3=80*32us, th4=110*32us, th5=157*32=5024	
			}
			// FCC
			else if(priv->pshare->rf_ft_var.manual_dfs_regdomain == 1){
				// pulse width hist th setting
				PHY_SetBBReg(priv, 0x19e4, 0xff000000, 2);  // th1=2*04us
				PHY_SetBBReg(priv, 0x19e8, bMaskDWord, 0xff4b1a0d); // set th2 = 13*0.4us, th3 = 26*0.4us, th4 = 75*0.4us, th5 = 255*0.4us
				// pulse repetition interval hist th setting
				PHY_SetBBReg(priv, 0x19b8, 0x00007f80, 4);  // th1=4*32us
				PHY_SetBBReg(priv, 0x19ec, bMaskDWord, 0x60200b08); // set th2 = 8*32us, th3 = 11*32us, th4 = 32*32us, th5 = 96*32 = 3072	
			}
			else{
			}			
		}

		RTL_W8(TXPAUSE, 0xff);
	}

	// ETSI
	if (priv->pshare->rf_ft_var.manual_dfs_regdomain == 3) {
		if (GET_CHIP_VER(priv) == VERSION_8192D) {
			PHY_SetBBReg(priv, 0xc8c, BIT(23) | BIT(22), 3);
			PHY_SetBBReg(priv, 0xc30, 0xf, 0xa);
			PHY_SetBBReg(priv, 0xcdc, 0xf0000, 4);
		}
	} else {
		if (GET_CHIP_VER(priv) == VERSION_8192D) {
			PHY_SetBBReg(priv, 0xc8c, BIT(23) | BIT(22), 0);
			PHY_SetBBReg(priv, 0xcd8, 0xffff, 0x1a1f);
		}
	}

	/*
	 *	Enable h/w DFS detect
	 */
	if (GET_CHIP_VER(priv) == VERSION_8192D) {
		PHY_SetBBReg(priv, 0xc84, BIT(25), 1);

		if (!priv->pshare->rf_ft_var.dfsdbgmode){
			PHY_SetBBReg(priv, 0xc7c, BIT(28), 1); // ynlin dbg
		}
	}
	
	// DFS reset after initialization
	RTL_W32(0x924, RTL_R32(0x924) & ~BIT(15));
	RTL_W32(0x924, RTL_R32(0x924) | BIT(15));
}

unsigned char *get_DFS_version(void)
{
	return DFS_VERSION;
}
#endif
 
