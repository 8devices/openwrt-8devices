//============================================================
//
// File Name: HalDMOutSrc_AP.c 
//
// Description:
//
// This file is for common outsource dynamic mechanism for partner.
//
//
//============================================================

#ifndef _HALDM_COMMON_C_
#define _HALDM_COMMON_C_

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <asm/uaccess.h>
#include <linux/fcntl.h>
#include <linux/fs.h>
#include <linux/file.h>

#include "8192cd_cfg.h"
#include "8192cd.h"
#include "8192cd_hw.h"
#include "8192cd_headers.h"
#include "8192cd_debug.h"
#include "./WlanHAL/HalHeader/HalComPhyReg.h"
#include "./phydm/phydm_precomp.h"
#if !defined(USE_OUT_SRC) || defined(_OUTSRC_COEXIST)
#define TX_POWER_NEAR_FIELD_THRESH_AP	HP_LOWER
#endif

#if defined(CONFIG_RTL_819X) && defined(USE_RLX_BSP)
#include <bspchip.h>
#endif

#ifdef HW_ANT_SWITCH
#define RXDVY_A_EN		((HW_DIV_ENABLE && !priv->pshare->rf_ft_var.antSw_select) ? 0x80 : 0)
#define RXDVY_B_EN		((HW_DIV_ENABLE &&  priv->pshare->rf_ft_var.antSw_select) ? 0x80 : 0)
#endif


//3 ============================================================
//3 DIG related functions
//3 ============================================================

int getIGIFor1RCCA(int value_IGI)
{
	#define ONERCCA_LOW_TH		0x30
	#define ONERCCA_LOW_DIFF	8

	if (value_IGI < ONERCCA_LOW_TH) {
		if ((ONERCCA_LOW_TH - value_IGI) < ONERCCA_LOW_DIFF)
			return ONERCCA_LOW_TH;
		else
			return value_IGI + ONERCCA_LOW_DIFF;
	} else {
		return value_IGI;
	}
}

#if 1

#if defined(DETECT_STA_EXISTANCE)  && defined(CONFIG_WLAN_HAL)
// Check for STA existance. If STA disappears, disconnect it. Added by Annie, 2010-08-10.
void DetectSTAExistance88XX(struct rtl8192cd_priv *priv, struct tx_rpt *report, struct stat_info *pstat)
{
	//unsigned char tmpbuf[16];

	// Parameters
	const unsigned int		maxTxFailCnt = 300;		// MAX Tx fail packet count
	const unsigned int		minTxFailCnt = 30;		// MIN Tx fail packet count; this value should be less than maxTxFailCnt.
	const unsigned int		txFailSecThr=  3;			// threshold of Tx Fail Time (in second)
	//const unsigned int		NoTXOKSecThr=  5;			// threshold of NO Tx ok Time (in second)
	//const unsigned int		NoRXOKSecThr=  5;			// threshold of NO Tx ok Time (in second)	

	// Temporarily change Retry Limit when TxFail. (tfrl: TxFailRetryLimit)
	//const unsigned char	TFRL = 7;				// New Retry Limit value
	//const unsigned char	TFRL_FailCnt = 2;		// Tx Fail Count threshold to set Retry Limit
	//const unsigned char	TFRL_SetTime = 2;		// Time to set Retry Limit (in second)
	//const unsigned char	TFRL_RcvTime = 3;		// Time to recover Retry Limit (in second)

    //const unsigned short Back_retty_value = 0x10;
    
#ifdef MCR_WIRELESS_EXTEND
	if (pstat->IOTPeer==HT_IOT_PEER_CMW)
		return;
#endif
    if( report->txok != 0 )
    { // Reset Counter
            pstat->tx_conti_fail_cnt = 0;
            pstat->tx_last_good_time = priv->up_time;
		if (pstat->leave) {
			GET_HAL_INTERFACE(priv)->UpdateHalMSRRPTHandler(priv, pstat, INCREASE);
			pstat->leave = 0;
		}
    }
    else if( report->txfail != 0 )
    {
		if (pstat->leave)
			return;
	
        pstat->tx_conti_fail_cnt += report->txfail;
        DEBUG_WARN( "detect: txfail=%d, tx_conti_fail_cnt=%d\n", report->txfail, pstat->tx_conti_fail_cnt );

        //RTL_W16(RL, (TFRL&SRL_Mask)<<SRL_SHIFT|(TFRL&LRL_Mask)<<LRL_SHIFT);

        #if 0 // Not to modify retry limit
        if( priv->up_time >= (pstat->tx_last_good_time+TFRL_SetTime) &&
            pstat->tx_conti_fail_cnt >= TFRL_FailCnt && 
            !priv->pshare->bRLShortened )
        { // Shorten retry limit, because AP spending too much time to send out g mode STA pending packets in HW queue.
            
            priv->pshare->bRLShortened = TRUE;
            printk( "== Shorten RetryLimit to 0x%04X ==\n", RTL_R16(RL) );
        }
        #endif 

        if(     (pstat->tx_conti_fail_cnt >= maxTxFailCnt) ||
            (pstat->tx_conti_fail_cnt >= minTxFailCnt && priv->up_time >= (pstat->tx_last_good_time+txFailSecThr) )
            )
        { 
            // This STA is considered as disappeared, so delete it.
            DEBUG_WARN( "** tx_conti_fail_cnt=%d (min=%d,max=%d)\n", pstat->tx_conti_fail_cnt, minTxFailCnt, maxTxFailCnt);
            DEBUG_WARN( "** tx_last_good_time=%d, up_time=%d (Thr:%d)\n", (int)pstat->tx_last_good_time, (int)priv->up_time, txFailSecThr );
            DEBUG_WARN( "AP is going to del_sta %02X:%02X:%02X:%02X:%02X:%02X\n", pstat->hwaddr[0],pstat->hwaddr[1],pstat->hwaddr[2],pstat->hwaddr[3],pstat->hwaddr[4],pstat->hwaddr[5] );

            // del_sta(priv, tmpbuf);
            ++(pstat->leave);
            
            GET_HAL_INTERFACE(priv)->UpdateHalMSRRPTHandler(priv, pstat, DECREASE);
            pstat->bDrop = 1;                               
            //printk("Drop client packet, AID = %x TxFailCnt= %x \n",pstat->aid,pstat->tx_conti_fail_cnt);
            //RTL_W16(RL, (Back_retty_value&SRL_Mask)<<SRL_SHIFT|(Back_retty_value&LRL_Mask)<<LRL_SHIFT);
            // send H2C drop packet

            // Reset Counter
            pstat->tx_conti_fail_cnt = 0;
            pstat->tx_last_good_time = priv->up_time;
        }
    }
    else
    {
#if 0        
        //printk("AID[%x](pstat->leave)=%x priv->up_time=%x (pstat->tx_last_good_time+NoTXOKSecThr)=%x \n",pstat->aid,(pstat->leave),priv->up_time,(pstat->tx_last_good_time+NoTXOKSecThr));
        if (!(pstat->leave) && (priv->up_time  > (pstat->tx_last_good_time+NoTXOKSecThr)) 
                && (priv->up_time  > (pstat->rx_last_good_time+NoRXOKSecThr)) ) 
        {
            DEBUG_WARN("%s %d expire timeout, set MACID 0 AID = %x \n",__FUNCTION__,__LINE__,REMAP_AID(pstat));
            GET_HAL_INTERFACE(priv)->UpdateHalMSRRPTHandler(priv, pstat, DECREASE);
            pstat->bDrop = 1;                               
            printk("Client no TRX over 10 Secs , Drop client packet, AID = %x \n",pstat->aid);
            ++(pstat->leave);
	    }           
#endif
    }
}
#endif

#endif 



void set_DIG_state(struct rtl8192cd_priv *priv, int state)
{
	int value_IGI;

	if (state) {
		priv->pshare->DIG_on = 1;
		if(IS_OUTSRC_CHIP(priv) && priv->pshare->restore) {
			ODM_Write_DIG(ODMPTR, priv->pshare->restore);
//			panic_printk("%s, 1-> %x, %x\n", __FUNCTION__, RTL_R8(0xc50), priv->pshare->restore);
		}
		priv->pshare->restore = 0;
	} else {
		priv->pshare->DIG_on = 0;
		if (priv->pshare->restore == 0) {
				value_IGI = 0x20;

			priv->pshare->restore = PHY_QueryBBReg(priv, 0xc50, 0x7f);
			if(IS_OUTSRC_CHIP(priv)	) {
				ODM_Write_DIG(ODMPTR, value_IGI);
				//panic_printk("%s, 0-> %x, %x\n", __FUNCTION__, RTL_R8(0xc50), priv->pshare->restore);
			}
		}
#ifdef INTERFERENCE_CONTROL
		priv->pshare->phw->signal_strength = 0;
#endif
	}
}

#if !defined(USE_OUT_SRC) || defined(_OUTSRC_COEXIST)
void check_DIG_by_rssi(struct rtl8192cd_priv *priv, unsigned char rssi_strength)
{
	unsigned int dig_on = 0;

	if (OPMODE & WIFI_SITE_MONITOR)
		return;

	if((GET_CHIP_VER(priv)== VERSION_8812E)||(GET_CHIP_VER(priv)== VERSION_8881A)||(GET_CHIP_VER(priv)== VERSION_8814A)||(GET_CHIP_VER(priv)== VERSION_8822B))
		return;

	if ((rssi_strength > priv->pshare->rf_ft_var.digGoUpperLevel)
		&& (rssi_strength < TX_POWER_NEAR_FIELD_THRESH_AP+1) && (priv->pshare->phw->signal_strength != 2)) {
		if (priv->pshare->is_40m_bw)
			// RTL_W8(0xc87, (RTL_R8(0xc87) & 0xf) | 0x30); 92D
			RTL_W8(0xc87, 0x30);
		else
			RTL_W8(0xc30, 0x44);

		if (priv->pshare->phw->signal_strength != 3)
			dig_on++;

		priv->pshare->phw->signal_strength = 2;
	}
	else if ((rssi_strength > TX_POWER_NEAR_FIELD_THRESH_AP+5) && (priv->pshare->phw->signal_strength != 3)) {
		if (priv->pshare->is_40m_bw)
			// RTL_W8(0xc87, (RTL_R8(0xc87) & 0xf) | 0x30); 92D
			RTL_W8(0xc87, 0x30);
		else
			RTL_W8(0xc30, 0x44);

		if (priv->pshare->phw->signal_strength != 2)
			dig_on++;

		priv->pshare->phw->signal_strength = 3;
	}
	else if (((rssi_strength < priv->pshare->rf_ft_var.digGoLowerLevel)
		&& (priv->pshare->phw->signal_strength != 1)) || !priv->pshare->phw->signal_strength) {
		// DIG off
#if 0
		set_DIG_state(priv, 0);
#endif

		if (priv->pshare->is_40m_bw)
			//RTL_W8(0xc87, (RTL_R8(0xc87) & 0xf) | 0x30); 92D
			RTL_W8(0xc87, 0x30);
		else
			RTL_W8(0xc30, 0x44);

		priv->pshare->phw->signal_strength = 1;
	}

	if (dig_on) {
		// DIG on
		set_DIG_state(priv, 1);
	}

	//check_DC_TH_by_rssi(priv, rssi_strength);
}
#endif

void DIG_for_site_survey(struct rtl8192cd_priv *priv, int do_ss)
{
	if (do_ss) {
		// DIG off
		set_DIG_state(priv, 0);
	}
	else {
		// DIG on
#ifndef INTERFERENCE_CONTROL
//		if (priv->pshare->phw->signal_strength > 1) 
#endif
		{
			set_DIG_state(priv, 1);
		}
	}
}

 extern unsigned char CCKSwingTable_Ch14 [][8];
 extern unsigned char CCKSwingTable_Ch1_Ch13[][8];
#ifndef CCK_TABLE_SIZE
#define CCK_TABLE_SIZE	33
#endif

 int get_CCK_swing_index(struct rtl8192cd_priv *priv)
 {
	 int TempCCk, index=12, i;
	 short channel;
#ifdef MP_TEST
	 if ((OPMODE & WIFI_MP_STATE) || priv->pshare->rf_ft_var.mp_specific)
		 channel=priv->pshare->working_channel;
	 else
#endif
		 channel = (priv->pmib->dot11RFEntry.dot11channel);
 
	 //Query CCK default setting From 0xa24
	 TempCCk = PHY_QueryBBReg(priv, rCCK0_TxFilter2, bMaskDWord)&bMaskCCK;
	 TempCCk = cpu_to_le32(TempCCk);
	 for(i=0 ; i<CCK_TABLE_SIZE ; i++)		 {
		 if(channel==14) {
			 if(memcmp((void*)&TempCCk, (void*)&CCKSwingTable_Ch14[i][2], 4)==0) {
				 index = i;
				 break;
			 }
		 } else {
			 if(memcmp((void*)&TempCCk, (void*)&CCKSwingTable_Ch1_Ch13[i][2], 4)==0) {
				 index = i;
				 break;
			 }
		 }
	 }
	 DEBUG_INFO("Initial reg0x%x = 0x%x, CCK_index=0x%x, ch %d\n",
							 rCCK0_TxFilter2, TempCCk, index, channel);
	 return index;
 }

 void set_CCK_swing_index(struct rtl8192cd_priv *priv, short CCK_index)
{
	short channel;
#ifdef MP_TEST
	if ((OPMODE & WIFI_MP_STATE) || priv->pshare->rf_ft_var.mp_specific)
		channel=priv->pshare->working_channel;
	else
#endif
		channel = (priv->pmib->dot11RFEntry.dot11channel);

		if (GET_CHIP_VER(priv) == VERSION_8197F) {
			if (channel !=14) {
				RTL_W8( 0xa22, 0x2C);
				RTL_W8( 0xa23, 0xE8);
				RTL_W8( 0xa24, 0x1C);
				RTL_W8( 0xa25, 0x0C);
				RTL_W8( 0xa26, 0xB8);
				RTL_W8( 0xa27, 0x64);
				RTL_W8( 0xa28, 0x10);
				RTL_W8( 0xa29, 0x88);
				RTL_W8( 0xa9a, 0x00);
				RTL_W8( 0xa9b, 0x00);
				RTL_W8( 0xa9c, 0x00);
				RTL_W8( 0xa9d, 0x00);
				RTL_W8( 0xaa0, 0x00);
				RTL_W8( 0xaa1, 0x00);
				RTL_W8( 0xaa2, 0x00);
				RTL_W8( 0xaa3, 0x00);
				RTL_W8( 0xaac, 0x67);
				RTL_W8( 0xaad, 0x56);
				RTL_W8( 0xaae, 0x23);
				RTL_W8( 0xaaf, 0x01);
				RTL_W8( 0xab0, 0x00);
				RTL_W8( 0xab1, 0x00);
				RTL_W8( 0xab2, 0x00);
				RTL_W8( 0xab3, 0x00);
			}
			else{
				RTL_W8( 0xa22, 0x2C);
				RTL_W8( 0xa23, 0xE8);
				RTL_W8( 0xa24, 0x1C);
				RTL_W8( 0xa25, 0xB8);
				RTL_W8( 0xa26, 0x00);
				RTL_W8( 0xa27, 0x00);
				RTL_W8( 0xa28, 0x00);
				RTL_W8( 0xa29, 0x00);
				RTL_W8( 0xa9a, 0x00);
				RTL_W8( 0xa9b, 0x00);
				RTL_W8( 0xa9c, 0x00);
				RTL_W8( 0xa9d, 0x00);
				RTL_W8( 0xaa0, 0x00);
				RTL_W8( 0xaa1, 0x00);
				RTL_W8( 0xaa2, 0x00);
				RTL_W8( 0xaa3, 0x00);
				RTL_W8( 0xaac, 0x67);
				RTL_W8( 0xaad, 0x36);
				RTL_W8( 0xaae, 0x00);
				RTL_W8( 0xaaf, 0x00);
				RTL_W8( 0xab0, 0x00);
				RTL_W8( 0xab1, 0x00);
				RTL_W8( 0xab2, 0x00);
				RTL_W8( 0xab3, 0x00);
			}		
		}	 else

		{
			if(channel !=14) {
				RTL_W8( 0xa22, CCKSwingTable_Ch1_Ch13[CCK_index][0]);
				RTL_W8( 0xa23, CCKSwingTable_Ch1_Ch13[CCK_index][1]);
				RTL_W8( 0xa24, CCKSwingTable_Ch1_Ch13[CCK_index][2]);
				RTL_W8( 0xa25, CCKSwingTable_Ch1_Ch13[CCK_index][3]);
				RTL_W8( 0xa26, CCKSwingTable_Ch1_Ch13[CCK_index][4]);
				RTL_W8( 0xa27, CCKSwingTable_Ch1_Ch13[CCK_index][5]);
				RTL_W8( 0xa28, CCKSwingTable_Ch1_Ch13[CCK_index][6]);
				RTL_W8( 0xa29, CCKSwingTable_Ch1_Ch13[CCK_index][7]);
			}
			else{
				RTL_W8( 0xa22, CCKSwingTable_Ch14[CCK_index][0]);
				RTL_W8( 0xa23, CCKSwingTable_Ch14[CCK_index][1]);
				RTL_W8( 0xa24, CCKSwingTable_Ch14[CCK_index][2]);
				RTL_W8( 0xa25, CCKSwingTable_Ch14[CCK_index][3]);
				RTL_W8( 0xa26, CCKSwingTable_Ch14[CCK_index][4]);
				RTL_W8( 0xa27, CCKSwingTable_Ch14[CCK_index][5]);
				RTL_W8( 0xa28, CCKSwingTable_Ch14[CCK_index][6]);
				RTL_W8( 0xa29, CCKSwingTable_Ch14[CCK_index][7]);
			}				
		}
}
 
#ifdef WIFI_WMM
void check_NAV_prot_len(struct rtl8192cd_priv *priv, struct stat_info *pstat, unsigned int disassoc)
{
/*
#if defined(CONFIG_RTL_8812_SUPPORT)||defined(CONFIG_WLAN_HAL_8881A) ||defined(CONFIG_WLAN_HAL_8814AE) || defined(CONFIG_WLAN_HAL_8822BE)
	if((GET_CHIP_VER(priv)== VERSION_8812E)||(GET_CHIP_VER(priv)== VERSION_8881A)||(GET_CHIP_VER(priv)== VERSION_8814A)||(GET_CHIP_VER(priv)== VERSION_8822B))
		return;
#endif
*/

	if ((priv->pmib->dot11BssType.net_work_type & WIRELESS_11N) && pstat

		&& pstat->ht_cap_len /*&& (pstat->IOTPeer==HT_IOT_PEER_INTEL)*/) 

	{
		if (!disassoc && (pstat->MIMO_ps & _HT_MIMO_PS_DYNAMIC_)) {
            setSTABitMap(&priv->pshare->mimo_ps_dynamic_sta, pstat->aid);
		} else {
            clearSTABitMap(&priv->pshare->mimo_ps_dynamic_sta, pstat->aid);
		}
/*
#ifdef CONFIG_RTL_88E_SUPPORT
		if (GET_CHIP_VER(priv) != VERSION_8188E)
#endif
*/
		if(CHIP_VER_92X_SERIES(priv))
		{
            if (orSTABitMap(&priv->pshare->mimo_ps_dynamic_sta)) {
				RTL_W8(NAV_PROT_LEN, 0x40);
			} else {
				RTL_W8(NAV_PROT_LEN, 0x20);
			}
		}
	}
}
#endif


//3 ============================================================
//3 FA statistic functions
//3 ============================================================

#if !defined(CONFIG_RTL_NEW_AUTOCH)
static
#endif
void reset_FA_reg(struct rtl8192cd_priv *priv)
{
	if((GET_CHIP_VER(priv)== VERSION_8812E)||(GET_CHIP_VER(priv)== VERSION_8881A)||(GET_CHIP_VER(priv)== VERSION_8814A)||(GET_CHIP_VER(priv)== VERSION_8822B)) {

		// reset OFDM FA coutner
		PHY_SetBBReg(priv, 0x9a4, BIT(17), 1);
		PHY_SetBBReg(priv, 0x9a4, BIT(17), 0);

		// reset CCK FA counter
		PHY_SetBBReg(priv, 0xa2c, BIT(17), 0);
		PHY_SetBBReg(priv, 0xa2c, BIT(17), 1);

		// reset CCA counter
		PHY_SetBBReg(priv, 0xb58, BIT(17), 1);
		PHY_SetBBReg(priv, 0xb58, BIT(17), 0);

		return;
	}

#if !defined(CONFIG_RTL_NEW_AUTOCH)
	unsigned char value8;

	value8 = RTL_R8(0xd03);
	RTL_W8(0xd03, value8 | 0x08);	// regD00[27]=1 to reset these OFDM FA counters
	value8 = RTL_R8(0xd03);
	RTL_W8(0xd03, value8 & 0xF7);	// regD00[27]=0 to start counting
	value8 = RTL_R8(0xa2d);
	RTL_W8(0xa2d, value8 & 0x3F);	// regA2D[7:6]=00 to disable counting
	value8 = RTL_R8(0xa2d);
	RTL_W8(0xa2d, value8 | 0x80);	// regA2D[7:6]=10 to enable counting
#else
	/* cck CCA */
	PHY_SetBBReg(priv, 0xa2c, BIT(13) | BIT(12), 0);
	PHY_SetBBReg(priv, 0xa2c, BIT(13) | BIT(12), 2);
	/* cck FA*/
	PHY_SetBBReg(priv, 0xa2c, BIT(15) | BIT(14), 0);
	PHY_SetBBReg(priv, 0xa2c, BIT(15) | BIT(14), 2);
	/* ofdm */
	PHY_SetBBReg(priv, 0xd00, BIT(27), 1);
	PHY_SetBBReg(priv, 0xd00, BIT(27), 0);
#endif


	if (GET_CHIP_VER(priv)==VERSION_8188E || GET_CHIP_VER(priv)==VERSION_8192E || GET_CHIP_VER(priv)==VERSION_8197F) {
		PHY_SetBBReg(priv, 0xc0c, BIT(31), 1);
		PHY_SetBBReg(priv, 0xc0c, BIT(31), 0);
	}
}

#if defined(CONFIG_RTL_NEW_AUTOCH)
void hold_CCA_FA_counter(struct rtl8192cd_priv *priv)
{

	/* hold cck CCA & FA counter */
	PHY_SetBBReg(priv, 0xa2c, BIT(12), 1);
	PHY_SetBBReg(priv, 0xa2c, BIT(14), 1);

	/* hold ofdm CCA & FA counter */
	PHY_SetBBReg(priv, 0xc00, BIT(31), 1);
	PHY_SetBBReg(priv, 0xd00, BIT(31), 1);
}

void release_CCA_FA_counter(struct rtl8192cd_priv *priv)
{
	if((GET_CHIP_VER(priv)== VERSION_8812E)||(GET_CHIP_VER(priv)== VERSION_8881A) ||(GET_CHIP_VER(priv)== VERSION_8814A) ||(GET_CHIP_VER(priv)== VERSION_8822B))
		return;

	/* release cck CCA & FA counter */
	PHY_SetBBReg(priv, 0xa2c, BIT(12), 0);
	PHY_SetBBReg(priv, 0xa2c, BIT(14), 0);

	/* release ofdm CCA & FA counter */
	PHY_SetBBReg(priv, 0xc00, BIT(31), 0);
	PHY_SetBBReg(priv, 0xd00, BIT(31), 0);

}


void _FA_statistic(struct rtl8192cd_priv *priv)
{
	if((GET_CHIP_VER(priv)== VERSION_8812E)||(GET_CHIP_VER(priv)== VERSION_8881A)||(GET_CHIP_VER(priv)== VERSION_8814A)||(GET_CHIP_VER(priv)== VERSION_8822B))
		return;

	// read OFDM FA counters
	priv->pshare->ofdm_FA_cnt1 = RTL_R16(0xda2);
	priv->pshare->ofdm_FA_cnt2 = RTL_R16(0xda4);
	priv->pshare->ofdm_FA_cnt3 = RTL_R16(0xda6);
	priv->pshare->ofdm_FA_cnt4 = RTL_R16(0xda8);

	priv->pshare->cck_FA_cnt = (RTL_R8(0xa5b) << 8) + RTL_R8(0xa5c);

#ifdef INTERFERENCE_CONTROL
	priv->pshare->ofdm_FA_total_cnt = (unsigned int) priv->pshare->ofdm_FA_cnt1 +
			priv->pshare->ofdm_FA_cnt2 + priv->pshare->ofdm_FA_cnt3 +
			priv->pshare->ofdm_FA_cnt4 + RTL_R16(0xcf0) + RTL_R16(0xcf2);
	
	priv->pshare->FA_total_cnt = priv->pshare->ofdm_FA_total_cnt + priv->pshare->cck_FA_cnt;
#else
	priv->pshare->FA_total_cnt = priv->pshare->ofdm_FA_cnt1 + priv->pshare->ofdm_FA_cnt2 +
	                             priv->pshare->ofdm_FA_cnt3 + priv->pshare->ofdm_FA_cnt4 +
	                             priv->pshare->cck_FA_cnt + RTL_R16(0xcf0) + RTL_R16(0xcf2);
#endif
}
#endif

void FA_statistic(struct rtl8192cd_priv *priv)
{



#if !defined(CONFIG_RTL_NEW_AUTOCH)
	signed char value8;

	// read OFDM FA counters
	priv->pshare->ofdm_FA_cnt1 = RTL_R16(0xda2);
	priv->pshare->ofdm_FA_cnt2 = RTL_R16(0xda4);
	priv->pshare->ofdm_FA_cnt3 = RTL_R16(0xda6);
	priv->pshare->ofdm_FA_cnt4 = RTL_R16(0xda8);

	// read the CCK FA counters
	value8 = RTL_R8(0xa2d);
	RTL_W8(0xa2d, value8 | 0x40);	// regA2D[6]=1 to hold and read the CCK FA counters
	priv->pshare->cck_FA_cnt = RTL_R8(0xa5b);
	priv->pshare->cck_FA_cnt = priv->pshare->cck_FA_cnt << 8;
	priv->pshare->cck_FA_cnt += RTL_R8(0xa5c);

	priv->pshare->FA_total_cnt = priv->pshare->ofdm_FA_cnt1 + priv->pshare->ofdm_FA_cnt2 +
	                             priv->pshare->ofdm_FA_cnt3 + priv->pshare->ofdm_FA_cnt4 +
	                             priv->pshare->cck_FA_cnt + RTL_R16(0xcf0) + RTL_R16(0xcf2);

	if (priv->pshare->rf_ft_var.rssi_dump)
		priv->pshare->CCA_total_cnt = ((RTL_R8(0xa60)<<8)|RTL_R8(0xa61)) + RTL_R16(0xda0);
#else
	hold_CCA_FA_counter(priv);
	_FA_statistic(priv);

	if (priv->pshare->rf_ft_var.rssi_dump)
		priv->pshare->CCA_total_cnt = ((RTL_R8(0xa60)<<8)|RTL_R8(0xa61)) + RTL_R16(0xda0);

	release_CCA_FA_counter(priv);
#endif

	reset_FA_reg(priv);

}


//3 ============================================================
//3 Rate Adaptive
//3 ============================================================

void check_RA_by_rssi(struct rtl8192cd_priv *priv, struct stat_info *pstat)
{
	int level = 0;

	switch (pstat->rssi_level) {
		case 1:
			if (pstat->rssi >= priv->pshare->rf_ft_var.raGoDownUpper)
				level = 1;
			else if ((pstat->rssi >= priv->pshare->rf_ft_var.raGoDown20MLower) ||
				((priv->pshare->is_40m_bw) && (pstat->ht_cap_len) &&
				(pstat->rssi >= priv->pshare->rf_ft_var.raGoDown40MLower) &&
				(pstat->ht_cap_buf.ht_cap_info & cpu_to_le16(_HTCAP_SUPPORT_CH_WDTH_))))
				level = 2;
			else
				level = 3;
			break;
		case 2:
			if (pstat->rssi > priv->pshare->rf_ft_var.raGoUpUpper)
				level = 1;
			else if ((pstat->rssi < priv->pshare->rf_ft_var.raGoDown40MLower) ||
				((!pstat->ht_cap_len || !priv->pshare->is_40m_bw ||
				!(pstat->ht_cap_buf.ht_cap_info & cpu_to_le16(_HTCAP_SUPPORT_CH_WDTH_))) &&
				(pstat->rssi < priv->pshare->rf_ft_var.raGoDown20MLower)))
				level = 3;
			else
				level = 2;
			break;
		case 3:
			if (pstat->rssi > priv->pshare->rf_ft_var.raGoUpUpper)
				level = 1;
			else if ((pstat->rssi > priv->pshare->rf_ft_var.raGoUp20MLower) ||
				((priv->pshare->is_40m_bw) && (pstat->ht_cap_len) &&
				(pstat->rssi > priv->pshare->rf_ft_var.raGoUp40MLower) &&
				(pstat->ht_cap_buf.ht_cap_info & cpu_to_le16(_HTCAP_SUPPORT_CH_WDTH_))))
				level = 2;
			else
				level = 3;
			break;
		default:
			if (isErpSta(pstat))
				DEBUG_ERR("wrong rssi level setting\n");
			break;
	}

	if (level != pstat->rssi_level && CHIP_VER_92X_SERIES(priv)) {
		pstat->rssi_level = level;
		{
		}
	}
}



void add_RATid(struct rtl8192cd_priv *priv, struct stat_info *pstat)
{
	unsigned char limit=16;
	int i;
	unsigned long flags;

	SAVE_INT_AND_CLI(flags);

	pstat->tx_ra_bitmap = 0;

	for (i=0; i<32; i++) {
		if (pstat->bssrateset[i])
			pstat->tx_ra_bitmap |= get_bit_value_from_ieee_value(pstat->bssrateset[i]&0x7f);
	}

	if (pstat->ht_cap_len) {
		if ((pstat->MIMO_ps & _HT_MIMO_PS_STATIC_) ||
			(get_rf_mimo_mode(priv)== MIMO_1T2R) ||
			(get_rf_mimo_mode(priv)== MIMO_1T1R))
			limit=8;

		for (i=0; i<limit; i++) {
			if (pstat->ht_cap_buf.support_mcs[i/8] & BIT(i%8))
				pstat->tx_ra_bitmap |= BIT(i+12);
		}
	}

	if (pstat->ht_cap_len) {
		unsigned int set_sgi = 0;
		if (priv->pshare->is_40m_bw && (pstat->tx_bw == HT_CHANNEL_WIDTH_20_40)
#ifdef WIFI_11N_2040_COEXIST
			&& !((OPMODE & WIFI_AP_STATE) && COEXIST_ENABLE &&
			(priv->bg_ap_timeout || orForce20_Switch20Map(priv)
			))
#endif
			) {
			if (pstat->ht_cap_buf.ht_cap_info & cpu_to_le16(_HTCAP_SHORTGI_40M_)
				&& priv->pmib->dot11nConfigEntry.dot11nShortGIfor40M)
				set_sgi++;
		} else if (pstat->ht_cap_buf.ht_cap_info & cpu_to_le16(_HTCAP_SHORTGI_20M_) &&
			priv->pmib->dot11nConfigEntry.dot11nShortGIfor20M) {
			set_sgi++;
		}

		if (set_sgi) {
				pstat->tx_ra_bitmap |= BIT(28);
		}
	}

	if(IS_OUTSRC_CHIP(priv)) {
		//if ((pstat->rssi_level < 1) || (pstat->rssi_level > 4)) 
			//ODM_RAStateCheck(ODMPTR, (s4Byte)pstat->rssi, FALSE, &pstat->rssi_level) ;
	} else 
	{
		if ((pstat->rssi_level < 1) || (pstat->rssi_level > 3)) {
			if (pstat->rssi >= priv->pshare->rf_ft_var.raGoDownUpper)
				pstat->rssi_level = 1;
			else if ((pstat->rssi >= priv->pshare->rf_ft_var.raGoDown20MLower) ||
				((priv->pshare->is_40m_bw) && (pstat->ht_cap_len) &&
				(pstat->rssi >= priv->pshare->rf_ft_var.raGoDown40MLower) &&
				(pstat->ht_cap_buf.ht_cap_info & cpu_to_le16(_HTCAP_SUPPORT_CH_WDTH_))))
				pstat->rssi_level = 2;
			else
				pstat->rssi_level = 3;
		}
	}

	if ((priv->pmib->dot11BssType.net_work_type & WIRELESS_11A) &&
		((OPMODE & WIFI_AP_STATE) || (priv->pmib->dot11RFEntry.phyBandSelect & PHY_BAND_5G)))
		pstat->tx_ra_bitmap &= 0xfffffff0; //disable cck rate


	// rate adaptive by rssi
	if ((priv->pmib->dot11BssType.net_work_type & WIRELESS_11N) && pstat->ht_cap_len && (!should_restrict_Nrate(priv, pstat))) {
		if ((get_rf_mimo_mode(priv) == MIMO_1T2R) || (get_rf_mimo_mode(priv) == MIMO_1T1R)) {
			switch (pstat->rssi_level) {
				case 1:
					pstat->tx_ra_bitmap &= 0x100f0000;
					break;
				case 2:
					pstat->tx_ra_bitmap &= 0x100ff000;
					break;
				case 3:
					if (priv->pshare->is_40m_bw)
						pstat->tx_ra_bitmap &= 0x100fe00f;
					else
						pstat->tx_ra_bitmap &= 0x100fe00f;
					break;
			}
		} else {
			switch (pstat->rssi_level) {
				case 1:
					pstat->tx_ra_bitmap &= 0x1f8f0000;
					break;
				case 2:
					pstat->tx_ra_bitmap &= 0x1f8ff00D;
					break;
				case 3:
					if (priv->pshare->is_40m_bw)
						pstat->tx_ra_bitmap &= 0x010fe00f;
					else
						pstat->tx_ra_bitmap &= 0x010ff00f;
					break;
			}

			// Don't need to mask high rates due to new rate adaptive parameters
			//if (pstat->is_broadcom_sta)		// use MCS12 as the highest rate vs. Broadcom sta
			//	pstat->tx_ra_bitmap &= 0x81ffffff;

			// NIC driver will report not supporting MCS15 and MCS14 in asoc req
			//if (pstat->is_rtl8190_sta && !pstat->is_2t_mimo_sta)
			//	pstat->tx_ra_bitmap &= 0x83ffffff;		// if Realtek 1x2 sta, don't use MCS15 and MCS14
		}
	}
	else if (((priv->pmib->dot11BssType.net_work_type & WIRELESS_11G) && isErpSta(pstat)) ||
			((priv->pmib->dot11BssType.net_work_type & WIRELESS_11A) &&
			((OPMODE & WIFI_AP_STATE) || (priv->pmib->dot11RFEntry.phyBandSelect & PHY_BAND_5G))))
	{
		switch (pstat->rssi_level) {
			case 1:
				pstat->tx_ra_bitmap &= 0x00000f00;
				break;
			case 2:
				pstat->tx_ra_bitmap &= 0x00000ff0;
				break;
			case 3:
				pstat->tx_ra_bitmap &= 0x00000fef;
				break;
		}
	} else {
		pstat->tx_ra_bitmap &= 0x0000000d;
	}

// Client mode IOT issue, Button 2009.07.17
#ifdef CLIENT_MODE
	if(OPMODE & WIFI_STATION_STATE) {

		if((pstat->IOTPeer!=HT_IOT_PEER_REALTEK_92SE) && pstat->is_realtek_sta && pstat->is_legacy_encrpt)

			pstat->tx_ra_bitmap &= 0x0001ffff;					// up to MCS4
	}
#endif
    update_remapAid(priv,pstat);
	RESTORE_INT(flags);
}

void rtl8192cd_NHMBBInit(struct rtl8192cd_priv *priv)
{
	RTL_W16(0x894+2, 0xc350);	//0x894[31:16]=0xffff	Time duration for NHM unit: 4us, 0x2710=40ms
	RTL_W16(0x890+2, 0xffff);	//0x890[31:16]=0xffff	th_9, th_10
	RTL_W32(0x898, 0xffffff50);	//0x898=0xffffff50 		th_3, th_2, th_1, th_0
	RTL_W32(0x89c, 0xffffffff);	//0x89c=0xffffffff		th_7, th_6, th_5, th_4
	PHY_SetBBReg(priv, 0xe28, bMaskByte0, 0xff);		//0xe28[7:0]=0xff		th_8
	PHY_SetBBReg(priv, 0x890, BIT10|BIT9|BIT8, 0x1);	//0x890[9:8]=3			enable CCX
	PHY_SetBBReg(priv, 0xc0c, BIT7, 0x1);		//0xc0c[7]=1		
}

void rtl8192cd_GetNHMCounterStatistics(struct rtl8192cd_priv *priv)
{
	u4Byte		value32 = 0;

	value32 = PHY_QueryBBReg(priv, 0x8d8, bMaskDWord);

	priv->pshare->rf_ft_var.NHM_cnt_0 = (u1Byte)(value32 & bMaskByte0);
	priv->pshare->rf_ft_var.NHM_cnt_1 = (u1Byte)((value32 & bMaskByte1)>>8);	

}

void rtl8192cd_NHMCounterStatisticsReset(struct rtl8192cd_priv *priv)
{
	PHY_SetBBReg(priv, 0x890, BIT1, 0);
	PHY_SetBBReg(priv, 0x890, BIT1, 1);
}

void rtl8192cd_NHMCounterStatistics(struct rtl8192cd_priv *priv)
{
	// Get NHM report
	rtl8192cd_GetNHMCounterStatistics(priv);
	// Reset NHM counter
	rtl8192cd_NHMCounterStatisticsReset(priv);
}

typedef enum {
	disable_LNA,
	enable_LNA,
} set_LNA;

void rtl8192cd_SetLNA(struct rtl8192cd_priv *priv, int type)
{
	unsigned int rf_path, max_rf_path = 1;

	if ( get_rf_mimo_mode(priv) == MIMO_2T2R )
		max_rf_path = 2;

	if ( GET_CHIP_VER(priv) == VERSION_8188C || GET_CHIP_VER(priv) == VERSION_8192C ) 
	{		
		for ( rf_path = 0 ; rf_path != max_rf_path ; rf_path++ )
		{
			PHY_SetRFReg(priv, rf_path, 0x10, 0xf0000, 0x3);				/*select Rx mode*/
			PHY_SetRFReg(priv, rf_path, 0x10, 0x0000f, 0xf);

			if ( type == disable_LNA )
				PHY_SetRFReg(priv, rf_path, 0x11, 0xfffff, 0xef500);		/*disable LNA*/
			else if ( type == enable_LNA )
				PHY_SetRFReg(priv, rf_path, 0x11, 0xfffff, 0xff500);		/*back to normal*/

			// verify only -----------------------------------------------------
			//unsigned char verify_0x05;
			//PHY_SetRFReg(priv, rf_path, 0x00, 0xf0000, 0x3);
			//verify_0x05 = PHY_QueryRFReg(priv, rf_path, 0x05, 0x40000, 1);
			//printk("[%s:%d] %s LNA\n", __func__, __LINE__, (verify_0x05?"Enable":"Disable"));
		}
	} 
	else if ( GET_CHIP_VER(priv) == VERSION_8188E )							/* for osk platform*/
	{
		for ( rf_path = 0 ; rf_path != max_rf_path ; rf_path++ )
		{
			PHY_SetRFReg(priv, rf_path, 0xef, 0x80000, 0x1);
			PHY_SetRFReg(priv, rf_path, 0x30, 0xfffff, 0x18000);			/*select Rx mode*/
			PHY_SetRFReg(priv, rf_path, 0x31, 0xfffff, 0x0000f);

			if ( type == disable_LNA )
				PHY_SetRFReg(priv, rf_path, 0x32, 0xfffff, 0x37f82);		/*disable LNA*/
			else if ( type == enable_LNA )
				PHY_SetRFReg(priv, rf_path, 0x32, 0xfffff, 0x77f82);		/*back to normal*/
			
			PHY_SetRFReg(priv, rf_path, 0xef, 0x80000, 0x0);
		}
	}
}

void rtl8192cd_SetTRxMux(struct rtl8192cd_priv *priv, u1Byte txMode, u1Byte rxMode)
{
	PHY_SetBBReg(priv, 0x824, BIT3|BIT2|BIT1, txMode);		// set TXmode to standby mode to remove outside noise affect
	PHY_SetBBReg(priv, 0x824, BIT22|BIT21|BIT20, rxMode);	// set RXmode to standby mode to remove outside noise affect

	if(get_rf_mimo_mode(priv) != MIMO_1T1R)
	{
		PHY_SetBBReg(priv, 0x82c, BIT3|BIT2|BIT1, txMode);		// set TXmode to standby mode to remove outside noise affect
		PHY_SetBBReg(priv, 0x82c, BIT22|BIT21|BIT20, rxMode);	// set RXmode to standby mode to remove outside noise affect
	}

}


void rtl8192cd_SetEDCCAThreshold(struct rtl8192cd_priv *priv, s1Byte H2L, s1Byte L2H)
{
	PHY_SetBBReg(priv,rOFDM0_ECCAThreshold, bMaskByte0, (u1Byte)L2H);
	PHY_SetBBReg(priv,rOFDM0_ECCAThreshold, bMaskByte2, (u1Byte)H2L);
}


void rtl8192cd_SearchPwdBLowerBound(struct rtl8192cd_priv *priv)
{
	u4Byte			value32;
	u1Byte			cnt, IGI_Pause = 0x7f, IGI_Resume = 0x20, IGI = 0x50;	//IGI = 0x50 for cal EDCCA lower bound
	BOOLEAN			bAdjust = TRUE, bAdjust2 = TRUE;
	s1Byte 			TH_L2H_dmc, TH_H2L_dmc, Diff, IGI_target = 0x32;
	u1Byte			txEdcca1 = 0, txEdcca0 = 0;

	if ( GET_CHIP_VER(priv) == VERSION_8188C || GET_CHIP_VER(priv) == VERSION_8192C || GET_CHIP_VER(priv) == VERSION_8188E )
		rtl8192cd_SetLNA(priv, disable_LNA);
	else
	{
		rtl8192cd_SetTRxMux(priv, 1, 1); // Set both Tx mode and Rx mode to standby mode
		RTL_W8(0xc50,IGI_Pause);
		RTL_W8(0xc58,IGI_Pause);
	}
	
	Diff = IGI_target -(s1Byte)IGI;
	TH_L2H_dmc = priv->pshare->rf_ft_var.TH_L2H_ini + Diff;
	
	if(TH_L2H_dmc > 10) 	
		TH_L2H_dmc = 10;
	
	TH_H2L_dmc = TH_L2H_dmc - priv->pshare->rf_ft_var.TH_EDCCA_HL_diff;
	rtl8192cd_SetEDCCAThreshold(priv,TH_H2L_dmc,TH_L2H_dmc);

	delay_ms(5);
	while(bAdjust)
	{
		for(cnt=0; cnt<20; cnt ++)
		{
			value32 = PHY_QueryBBReg(priv,0xDF4, bMaskDWord);
			
			if ( (GET_CHIP_VER(priv) == VERSION_8188E) && (value32 & BIT30) )
				txEdcca1 = txEdcca1 + 1;
			else if(value32 & BIT29)
				txEdcca1 = txEdcca1 + 1;
			else
				txEdcca0 = txEdcca0 + 1;
		}
	
		if(txEdcca1 > 1)
		{
			IGI = IGI -1;
			TH_L2H_dmc = TH_L2H_dmc + 1;
			if(TH_L2H_dmc > 10)
				TH_L2H_dmc = 10;
			TH_H2L_dmc = TH_L2H_dmc - priv->pshare->rf_ft_var.TH_EDCCA_HL_diff;
			
			rtl8192cd_SetEDCCAThreshold(priv,TH_H2L_dmc,TH_L2H_dmc);
		
			if(TH_L2H_dmc == 10)
			{
				bAdjust = FALSE;
				priv->pshare->rf_ft_var.H2L_lb = TH_H2L_dmc;
				priv->pshare->rf_ft_var.L2H_lb = TH_L2H_dmc;
				priv->pshare->rf_ft_var.Adaptivity_IGI_upper = IGI;
			}
		}
		else
		{
			bAdjust = FALSE;
			priv->pshare->rf_ft_var.H2L_lb = TH_H2L_dmc;
			priv->pshare->rf_ft_var.L2H_lb = TH_L2H_dmc;	
			priv->pshare->rf_ft_var.Adaptivity_IGI_upper = IGI;
		}

		txEdcca1 = 0;
		txEdcca0 = 0;
	}	
	
	priv->pshare->rf_ft_var.Adaptivity_IGI_upper -= priv->pshare->rf_ft_var.dcbackoff;
	priv->pshare->rf_ft_var.H2L_lb += priv->pshare->rf_ft_var.dcbackoff;
	priv->pshare->rf_ft_var.L2H_lb += priv->pshare->rf_ft_var.dcbackoff;

	if ( GET_CHIP_VER(priv) == VERSION_8188C || GET_CHIP_VER(priv) == VERSION_8192C || GET_CHIP_VER(priv) == VERSION_8188E )
		rtl8192cd_SetLNA(priv, enable_LNA);
	else
	{
		rtl8192cd_SetTRxMux(priv, 2, 3); // Set  2:Tx mode and 3:Rx mode 
		RTL_W8(0xc50,IGI_Resume);
		RTL_W8(0xc58,IGI_Resume);	
	}

	rtl8192cd_SetEDCCAThreshold(priv,0x7f,0x7f);
}

BOOLEAN rtl8192cd_reSearchCondition(struct rtl8192cd_priv *priv)
{
	unsigned char Adaptivity_IGI_upper;

	Adaptivity_IGI_upper = priv->pshare->rf_ft_var.Adaptivity_IGI_upper;

	if (Adaptivity_IGI_upper <= 0x26)
		return TRUE;
	else
		return FALSE;	
}

void rtl8192cd_MACEDCCAState(struct rtl8192cd_priv *priv, u1Byte State)
{
	if(State == 0)//ignore edcca
	{
		PHY_SetBBReg(priv, REG_TX_PTCL_CTRL, BIT15, 1);	//ignore EDCCA	reg520[15]=1
		PHY_SetBBReg(priv, REG_RD_CTRL, BIT11, 0);		//reg524[11]=0
	}
	else		// don't set MAC ignore EDCCA signal
	{
		PHY_SetBBReg(priv, REG_TX_PTCL_CTRL, BIT15, 0);	//don't ignore EDCCA	 reg520[15]=0
		PHY_SetBBReg(priv, REG_RD_CTRL, BIT11, 1);	//reg524[11]=1	
	}
}

void rtl8192cd_AdaptivityInit(struct rtl8192cd_priv *priv)
{
	rtl8192cd_NHMBBInit(priv);

	priv->pshare->rf_ft_var.TH_L2H_ini = priv->pshare->rf_ft_var.TH_L2H_ini_backup; 
	priv->pshare->rf_ft_var.TH_EDCCA_HL_diff = 7;
	priv->pshare->rf_ft_var.TH_L2H_ini_mode2 = 20;
	priv->pshare->rf_ft_var.TH_EDCCA_HL_diff_mode2 = 8;
	priv->pshare->rf_ft_var.TH_EDCCA_HL_diff_backup = priv->pshare->rf_ft_var.TH_EDCCA_HL_diff ;
	priv->pshare->rf_ft_var.IGI_Base = 0x32;	
	priv->pshare->rf_ft_var.IGI_target = 0x1c;
	priv->pshare->rf_ft_var.H2L_lb = 0;
	priv->pshare->rf_ft_var.L2H_lb = 0;
	priv->pshare->rf_ft_var.Adaptivity_IGI_upper = 0;	
	priv->pshare->rf_ft_var.NHMWait = 0;
	
	priv->pshare->rf_ft_var.bCheck = FALSE;
	priv->pshare->rf_ft_var.bFirstLink = TRUE;
	priv->pshare->rf_ft_var.Adaptivity_enable = FALSE;	// use this flag to judge enable or disable	
	priv->pshare->rf_ft_var.isCleanEnvironment = FALSE;

	if (priv->pshare->rf_ft_var.adaptivity_enable == 3)
		priv->pshare->rf_ft_var.AcsForAdaptivity = TRUE;
	else
		priv->pshare->rf_ft_var.AcsForAdaptivity = FALSE;
	
	if (priv->pshare->rf_ft_var.adaptivity_enable == 2)
		priv->pshare->rf_ft_var.DynamicLinkAdaptivity = TRUE;
	else
		priv->pshare->rf_ft_var.DynamicLinkAdaptivity = FALSE;

	rtl8192cd_MACEDCCAState(priv, 1);
	PHY_SetBBReg(priv,0x908, bMaskDWord, 0x208);
	rtl8192cd_SearchPwdBLowerBound(priv);
	if ( rtl8192cd_reSearchCondition(priv) )
		rtl8192cd_SearchPwdBLowerBound(priv);
}


BOOLEAN	rtl8192cd_CalNHMcnt(struct rtl8192cd_priv *priv)
{
	u2Byte			Base = 0;

	Base = priv->pshare->rf_ft_var.NHM_cnt_0 + priv->pshare->rf_ft_var.NHM_cnt_1;

	if(Base != 0)
	{
		priv->pshare->rf_ft_var.NHM_cnt_0 = ((priv->pshare->rf_ft_var.NHM_cnt_0) << 8) / Base;
		priv->pshare->rf_ft_var.NHM_cnt_1 = ((priv->pshare->rf_ft_var.NHM_cnt_1) << 8) / Base;
	}
	
	if((priv->pshare->rf_ft_var.NHM_cnt_0 - priv->pshare->rf_ft_var.NHM_cnt_1) >= 100)
		return TRUE;			// clean environment
	else
		return FALSE;		//noisy environment

}


void rtl8192cd_CheckEnvironment(struct rtl8192cd_priv *priv)
{
	BOOLEAN 	isCleanEnvironment = FALSE;

	if(priv->pshare->rf_ft_var.bFirstLink == TRUE)
	{
		priv->pshare->rf_ft_var.adaptivity_flag = TRUE;
		priv->pshare->rf_ft_var.bFirstLink = FALSE;
		return;
	}
	else
	{
		if(priv->pshare->rf_ft_var.NHMWait < 3)			// Start enter NHM after 4 NHMWait
		{
			priv->pshare->rf_ft_var.NHMWait ++;
			rtl8192cd_NHMCounterStatistics(priv);
			return;
		}
		else
		{
			rtl8192cd_NHMCounterStatistics(priv);
			isCleanEnvironment = rtl8192cd_CalNHMcnt(priv);
			if(isCleanEnvironment == TRUE)
			{
				priv->pshare->rf_ft_var.TH_L2H_ini = priv->pshare->rf_ft_var.TH_L2H_ini_backup;			//mode 1
				priv->pshare->rf_ft_var.TH_EDCCA_HL_diff= priv->pshare->rf_ft_var.TH_EDCCA_HL_diff_backup;
				priv->pshare->rf_ft_var.Adaptivity_enable = TRUE;
				priv->pshare->rf_ft_var.adaptivity_flag = TRUE;
				priv->pshare->rf_ft_var.isCleanEnvironment = TRUE;
			}
			else
			{
				if ( !priv->pshare->rf_ft_var.AcsForAdaptivity ) {
					priv->pshare->rf_ft_var.TH_L2H_ini = priv->pshare->rf_ft_var.TH_L2H_ini_mode2;			// for AP mode 2
					priv->pshare->rf_ft_var.TH_EDCCA_HL_diff= priv->pshare->rf_ft_var.TH_EDCCA_HL_diff_mode2;
					priv->pshare->rf_ft_var.Adaptivity_enable = FALSE;
					priv->pshare->rf_ft_var.adaptivity_flag = FALSE;
				}
				priv->pshare->rf_ft_var.isCleanEnvironment = FALSE;
			}

			priv->pshare->rf_ft_var.NHMWait = 0;
			priv->pshare->rf_ft_var.bFirstLink = TRUE;
			priv->pshare->rf_ft_var.bCheck = TRUE;
		}
		
	}

}

void rtl8192cd_CheckAdaptivity(struct rtl8192cd_priv *priv)
{
	if(priv->pshare->rf_ft_var.adaptivity_enable)
	{
		if(priv->pshare->rf_ft_var.DynamicLinkAdaptivity || priv->pshare->rf_ft_var.AcsForAdaptivity)
		{
			if( priv->pshare->rf_ft_var.bLinked && !priv->pshare->rf_ft_var.bCheck )
			{
				rtl8192cd_NHMCounterStatistics(priv);
				rtl8192cd_CheckEnvironment(priv);
			}
			else if( !priv->pshare->rf_ft_var.bLinked )
			{
				priv->pshare->rf_ft_var.bCheck = FALSE;
			} 	
		}
		else
		{
			priv->pshare->rf_ft_var.Adaptivity_enable = TRUE;
			priv->pshare->rf_ft_var.adaptivity_flag = TRUE;
		}
	}
	else
	{
		priv->pshare->rf_ft_var.Adaptivity_enable = FALSE;
			priv->pshare->rf_ft_var.adaptivity_flag = FALSE;				
	}	 
}

void rtl8192cd_Adaptivity(struct rtl8192cd_priv *priv, u1Byte IGI)
{
	s1Byte Diff, IGI_target, TH_L2H_dmc, TH_H2L_dmc;
	
	if(priv->pshare->CurrentChannelBW == HT_CHANNEL_WIDTH_20) 
		IGI_target = priv->pshare->rf_ft_var.IGI_Base;
	else if(priv->pshare->CurrentChannelBW == HT_CHANNEL_WIDTH_20_40)
		IGI_target = priv->pshare->rf_ft_var.IGI_Base + 2;
	else
		IGI_target = priv->pshare->rf_ft_var.IGI_Base;
	priv->pshare->rf_ft_var.IGI_target = IGI_target;

	
	DEBUG_INFO("adaptivity_flag=%d, Adaptivity_enable=%d, bLinked=%d\n", 
		priv->pshare->rf_ft_var.adaptivity_flag, priv->pshare->rf_ft_var.Adaptivity_enable, priv->pshare->rf_ft_var.bLinked);
	DEBUG_INFO("DynamicLinkAdaptivity=%d, AcsForAdaptivity=%d, isCleanEnvironment=%d\n", 
		priv->pshare->rf_ft_var.DynamicLinkAdaptivity, priv->pshare->rf_ft_var.AcsForAdaptivity, priv->pshare->rf_ft_var.isCleanEnvironment);
	

	if ( priv->pshare->rf_ft_var.DynamicLinkAdaptivity
		&& !priv->pshare->rf_ft_var.bLinked
		&& !priv->pshare->rf_ft_var.Adaptivity_enable )
	{
		rtl8192cd_SetEDCCAThreshold(priv, 0x7f, 0x7f);
		return;
	}

	Diff = IGI_target -IGI;
	TH_L2H_dmc = priv->pshare->rf_ft_var.TH_L2H_ini + Diff;
	TH_H2L_dmc = TH_L2H_dmc - priv->pshare->rf_ft_var.TH_EDCCA_HL_diff;
		
	if(TH_L2H_dmc >10)
		TH_L2H_dmc = 10;
		
	TH_H2L_dmc = TH_L2H_dmc - priv->pshare->rf_ft_var.TH_EDCCA_HL_diff;
		
	//replace lower bound to prevent EDCCA always equal 1
	if(TH_H2L_dmc < priv->pshare->rf_ft_var.H2L_lb)				
		TH_H2L_dmc = priv->pshare->rf_ft_var.H2L_lb;
	if(TH_L2H_dmc < priv->pshare->rf_ft_var.L2H_lb)
		TH_L2H_dmc = priv->pshare->rf_ft_var.L2H_lb;

	DEBUG_INFO("TH_L2H_ini=%d, TH_EDCCA_HL_diff=%d\n", priv->pshare->rf_ft_var.TH_L2H_ini, priv->pshare->rf_ft_var.TH_EDCCA_HL_diff);
	DEBUG_INFO("IGI=0x%02x, TH_H2L_dmc=%d, TH_L2H_dmc=%d\n", IGI, TH_H2L_dmc, TH_L2H_dmc);
	DEBUG_INFO("Adaptivity_IGI_upper=0x%02x, H2L_lb=%d, L2H_lb=%d\n", priv->pshare->rf_ft_var.Adaptivity_IGI_upper, priv->pshare->rf_ft_var.H2L_lb, priv->pshare->rf_ft_var.L2H_lb);	

	rtl8192cd_SetEDCCAThreshold(priv,TH_H2L_dmc,TH_L2H_dmc);
}

void check_NBI_by_rssi(struct rtl8192cd_priv *priv, unsigned char rssi_strength)
{
	if (OPMODE & WIFI_SITE_MONITOR)
		return;

	if (priv->pmib->dot11RFEntry.phyBandSelect == PHY_BAND_5G)
		return;

	if (priv->pshare->phw->nbi_filter_on) {
		if (rssi_strength < 20) 
			NBI_filter_off(priv);
		
	} else {	// NBI OFF previous
		if (rssi_strength > 25) 
			NBI_filter_on(priv);		
	}
}


void NBI_filter_on(struct rtl8192cd_priv *priv)
{
	priv->pshare->phw->nbi_filter_on = 1;

#ifdef MP_TEST
	if ((OPMODE & WIFI_MP_STATE) || priv->pshare->rf_ft_var.mp_specific)
		return;
#endif

#ifdef RTK_AC_SUPPORT	
	if((GET_CHIP_VER(priv) == VERSION_8812E) || (GET_CHIP_VER(priv) == VERSION_8881A))
		RTL_W16(rFPGA0_XCD_RFParam, RTL_R16(rFPGA0_XCD_RFParam) | BIT(13));		// NBI on
	else
#endif		
		RTL_W16(rOFDM0_RxDSP, RTL_R16(rOFDM0_RxDSP) | BIT(9));		// NBI on
}

void NBI_filter_off(struct rtl8192cd_priv *priv)
{
	priv->pshare->phw->nbi_filter_on = 0;

#ifdef MP_TEST
	if ((OPMODE & WIFI_MP_STATE) || priv->pshare->rf_ft_var.mp_specific)
		return;
#endif

#ifdef RTK_AC_SUPPORT	
	if((GET_CHIP_VER(priv) == VERSION_8812E) || (GET_CHIP_VER(priv) == VERSION_8881A))
		RTL_W16(rFPGA0_XCD_RFParam, RTL_R16(rFPGA0_XCD_RFParam) & ~ BIT(13));		// NBI off
	else
#endif		
		RTL_W16(rOFDM0_RxDSP, RTL_R16(rOFDM0_RxDSP) & ~ BIT(9));	// NBI off
}

void RRSR_power_control_11n(struct rtl8192cd_priv *priv, int lower)
{
	unsigned long x;
	unsigned long val32;

	if ( lower && priv->pshare->phw->lower_tx_power== 0 )
	{
	  SAVE_INT_AND_CLI(x);
	  priv->pshare->phw->lower_tx_power = 1;

	  val32 = RTL_R32(0x6d8);
#ifdef HIGH_POWER_EXT_PA
	  if (priv->pshare->rf_ft_var.use_ext_pa)
		val32 |= 0xc0000;
	  else
#endif
		val32 |= 0x80000;	 
	  RTL_W32(0x6d8, val32);

	  RESTORE_INT(x);
	} 
	else if ( !lower && priv->pshare->phw->lower_tx_power )
	{	
	  SAVE_INT_AND_CLI(x);
	  priv->pshare->phw->lower_tx_power = 0;
	  
	  val32 = RTL_R32(0x6d8);
#ifdef HIGH_POWER_EXT_PA
	  if (priv->pshare->rf_ft_var.use_ext_pa)
		val32 &= ~0xc0000;
	  else
#endif
		val32 &= ~0x80000;
	  RTL_W32(0x6d8, val32);
			  
	  RESTORE_INT(x);	
	}		

}


//3 ============================================================
//3 PHY calibration
//3 ============================================================
#ifndef CALIBRATE_BY_ODM


void _PHY_SaveADDARegisters(struct rtl8192cd_priv *priv, unsigned int* ADDAReg,	unsigned int* ADDABackup, unsigned int RegisterNum)
{
	unsigned int	i;
	for( i = 0 ; i < RegisterNum ; i++){
		ADDABackup[i] = PHY_QueryBBReg(priv, ADDAReg[i], bMaskDWord);
	}
}

void _PHY_SetADDARegisters(struct rtl8192cd_priv *priv, unsigned int* ADDAReg,	unsigned int* ADDASettings, unsigned int RegisterNum)
{
	unsigned int	i;

	for( i = 0 ; i < RegisterNum ; i++){
		PHY_SetBBReg(priv, ADDAReg[i], bMaskDWord, ADDASettings[i]);
	}
}

void _PHY_SaveMACRegisters(struct rtl8192cd_priv *priv, unsigned int* MACReg, unsigned int* MACBackup)
{
	unsigned int	i;
	for( i = 0 ; i < (IQK_MAC_REG_NUM - 1); i++){
		MACBackup[i] = RTL_R8(MACReg[i]);
	}
	MACBackup[i] = RTL_R32( MACReg[i]);
}

void _PHY_ReloadADDARegisters(struct rtl8192cd_priv *priv, unsigned int* ADDAReg, unsigned int*	ADDABackup, unsigned int RegiesterNum)
{
	unsigned int	i;
	for(i = 0 ; i < RegiesterNum; i++){
		PHY_SetBBReg(priv, ADDAReg[i], bMaskDWord, ADDABackup[i]);
	}
}

void _PHY_ReloadMACRegisters(struct rtl8192cd_priv *priv,unsigned int* MACReg, unsigned int*	 MACBackup)
{
	unsigned int	i;
	for(i = 0 ; i < (IQK_MAC_REG_NUM - 1); i++){
		RTL_W8( MACReg[i], (unsigned char)MACBackup[i]);
	}
	RTL_W32( MACReg[i], MACBackup[i]);
}

void _PHY_MACSettingCalibration(struct rtl8192cd_priv *priv, unsigned int* MACReg, unsigned int* MACBackup)
{
	unsigned int	i = 0;
	RTL_W8(MACReg[i], 0x3F);
	for(i = 1 ; i < (IQK_MAC_REG_NUM - 1); i++){
		RTL_W8( MACReg[i], (unsigned char)(MACBackup[i]&(~ BIT(3))));
	}
	RTL_W8( MACReg[i], (unsigned char)(MACBackup[i]&(~ BIT(5))));
}

void _PHY_PathADDAOn(struct rtl8192cd_priv *priv, unsigned int* ADDAReg, char isPathAOn, char is2T)
{
	unsigned int	pathOn;
	unsigned int	i;

	pathOn = isPathAOn ? 0x04db25a4 : 0x0b1b25a4;
	if(FALSE == is2T){
		pathOn = 0x0bdb25a0;
		PHY_SetBBReg(priv, ADDAReg[0], bMaskDWord, 0x0b1b25a0);
	}
	else{
		PHY_SetBBReg(priv, ADDAReg[0], bMaskDWord, pathOn);
	}

	for( i = 1 ; i < IQK_ADDA_REG_NUM ; i++){
		PHY_SetBBReg(priv, ADDAReg[i], bMaskDWord, pathOn);
	}
}
#endif


void PHY_LCCalibrate(struct rtl8192cd_priv *priv)
{
	unsigned char tmpReg, value_IGI;
	unsigned int LC_Cal;
	int isNormal;

	isNormal = 1;

	/* Check continuous TX and Packet TX */
	tmpReg = RTL_R8(0xd03);

	if ((tmpReg & 0x70) != 0)			/* Deal with contisuous TX case */
		RTL_W8(0xd03, tmpReg&0x8F);	/* disable all continuous TX */
	else								/* Deal with Packet TX case */
		RTL_W8(0x522, 0xFF);			/* block all queues */

	/* 2. Set RF mode = standby mode */
	if ((tmpReg & 0x70) != 0) {
		/* Path-A */
		PHY_SetRFReg(priv, RF92CD_PATH_A, 0x00, bMask20Bits, 0x10000);

		/* Path-B */
		if (get_rf_mimo_mode(priv) != MIMO_1T1R)
			PHY_SetRFReg(priv, RF92CD_PATH_B, 0x00, bMask20Bits, 0x10000);
	}

	/* 3. Read RF reg18 */
	LC_Cal = PHY_QueryRFReg(priv, RF92CD_PATH_A, 0x18, bMask12Bits, 1);

	/* 4. Set LC calibration begin */
	PHY_SetRFReg(priv, RF92CD_PATH_A, 0x18, bMask12Bits, LC_Cal|0x08000);

	if (GET_CHIP_VER(priv)==VERSION_8192D)
		watchdog_kick();

	if (isNormal)
		delay_ms(100);
	else
		delay_ms(3);

	/* Restore original situation */
	if ((tmpReg & 0x70) != 0) {
		/* Deal with contisuous TX case */

		/* Path-A */
		RTL_W8(0xd03, tmpReg);

		/* Restore RF mdoe & RF gain by change IGI to trigger HW tristate */
		value_IGI = (RTL_R8(0xc50) & 0x7F);
		RTL_W8(0xc50, ((value_IGI!=0x30)?0x30:0x31));
		RTL_W8(0xc50, value_IGI);

		/* Path-B */
		if (get_rf_mimo_mode(priv) != MIMO_1T1R) {
			/* Restore RF mdoe & RF gain by change IGI to trigger HW tristate */
			value_IGI = (RTL_R8(0xc58) & 0x7F);
			RTL_W8(0xc58, ((value_IGI!=0x30)?0x30:0x31));
			RTL_W8(0xc58, value_IGI);
		}
	} else {
		/* Deal with Packet TX case */

		RTL_W8(0x522, 0x00);
	}
}

#ifdef RTL_MANUAL_EDCA
void dynamic_EDCA_para(struct rtl8192cd_priv *priv, int mode)
{
	static unsigned int slot_time, VO_TXOP, VI_TXOP, sifs_time;
    struct ParaRecord EDCA[4];
	slot_time = 20;
	sifs_time = 10;

	if (mode & WIRELESS_11N)
		sifs_time = 16;

	 if( priv->pmib->dot11QosEntry.ManualEDCA ) {
		 memset(EDCA, 0, 4*sizeof(struct ParaRecord));
		 if( OPMODE & WIFI_AP_STATE )
			 memcpy(EDCA, priv->pmib->dot11QosEntry.AP_manualEDCA, 4*sizeof(struct ParaRecord));
		 else
			 memcpy(EDCA, priv->pmib->dot11QosEntry.STA_manualEDCA, 4*sizeof(struct ParaRecord));

		if ((mode & WIRELESS_11N) ||
			(mode & WIRELESS_11G)) {
			slot_time = 9;
		}

		RTL_W32(EDCA_VO_PARA, (EDCA[VO].TXOPlimit << 16) | (EDCA[VO].ECWmax << 12) | (EDCA[VO].ECWmin << 8) | (sifs_time + EDCA[VO].AIFSN * slot_time));
#ifdef WIFI_WMM
		if (QOS_ENABLE)
			RTL_W32(EDCA_VI_PARA, (EDCA[VI].TXOPlimit << 16) | (EDCA[VI].ECWmax << 12) | (EDCA[VI].ECWmin << 8) | (sifs_time + EDCA[VI].AIFSN * slot_time));
		else
#endif
			RTL_W32(EDCA_VI_PARA, (EDCA[BE].TXOPlimit << 16) | (EDCA[BE].ECWmax << 12) | (EDCA[BE].ECWmin << 8) | (sifs_time + EDCA[VI].AIFSN * slot_time));

		RTL_W32(EDCA_BE_PARA, (EDCA[BE].TXOPlimit << 16) | (EDCA[BE].ECWmax << 12) | (EDCA[BE].ECWmin << 8) | (sifs_time + EDCA[BE].AIFSN * slot_time));

		RTL_W32(EDCA_BK_PARA, (EDCA[BK].TXOPlimit << 16) | (EDCA[BK].ECWmax << 12) | (EDCA[BK].ECWmin << 8) | (sifs_time + EDCA[BK].AIFSN * slot_time));
	}

}
#endif //RTL_MANUAL_EDCA

#endif

