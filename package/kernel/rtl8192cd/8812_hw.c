
#define _8812_HW_C_

#include "8192cd.h"
#include "8192cd_cfg.h"
#include "8192cd_util.h"

#include "8192c_reg.h"
#include "8812_reg.h"
#include "8812_vht_gen.h"

#include <linux/kernel.h>
//eric_8812 #include "8192cd_debug.h"
#include "8192cd_headers.h"





void SwitchChannel(struct rtl8192cd_priv *priv)
{

	int ch = priv->pshare->dfsSwitchChannel;
	int staidx = 0;
//#ifndef SMP_SYNC
	unsigned long flags=0;
//#endif
	struct stat_info	*pstat = findNextSTA(priv, &staidx);

	priv->pmib->dot11RFEntry.dot11channel = ch;
	priv->pshare->dfsSwitchChannel = 0;
	RTL_W8(TXPAUSE, 0xff);

	PHY_SetBBReg(priv, 0x924, BIT(15), 0); // disable HW DFS report
 	if (!priv->pmib->dot11DFSEntry.disable_DFS &&
		(OPMODE & WIFI_AP_STATE)) {
		if (timer_pending(&priv->DFS_timer))
			del_timer_sync(&priv->DFS_timer);

		if (timer_pending(&priv->ch_avail_chk_timer))
			del_timer_sync(&priv->ch_avail_chk_timer);

		if (timer_pending(&priv->dfs_det_chk_timer))
			del_timer_sync(&priv->dfs_det_chk_timer);
 	}

	priv->pshare->CurrentChannelBW = priv->pshare->is_40m_bw = priv->pmib->dot11nConfigEntry.dot11nUse40M;

	if (priv->pmib->dot11nConfigEntry.dot11nUse40M == HT_CHANNEL_WIDTH_80 &&
		priv->pmib->dot11RFEntry.band5GSelected == PHY_BAND_5G_3)
	{
		int channel = priv->pmib->dot11RFEntry.dot11channel;

		if (!is80MChannel(priv->available_chnl, priv->available_chnl_num, channel)) {
			priv->pshare->CurrentChannelBW = priv->pshare->is_40m_bw = HT_CHANNEL_WIDTH_AC_20;
		}
	}
	if(priv->pshare->rf_ft_var.dfs_next_ch != 0){
		priv->pmib->dot11RFEntry.dot11channel = priv->pshare->rf_ft_var.dfs_next_ch;
	}
	panic_printk("4. Swiching channel to %d! BW %dM mode\n", priv->pmib->dot11RFEntry.dot11channel,
		(priv->pshare->CurrentChannelBW == 2)? 80 : ((priv->pshare->CurrentChannelBW == 1)? 40 : 20));

	if ((ch>144) ? ((ch-1)%8) : (ch%8)) {
		GET_MIB(priv)->dot11nConfigEntry.dot11n2ndChOffset = HT_2NDCH_OFFSET_ABOVE;
		priv->pshare->offset_2nd_chan	= HT_2NDCH_OFFSET_ABOVE;
	} else {
		GET_MIB(priv)->dot11nConfigEntry.dot11n2ndChOffset = HT_2NDCH_OFFSET_BELOW;
		priv->pshare->offset_2nd_chan	= HT_2NDCH_OFFSET_BELOW;
	}

	SAVE_INT_AND_CLI(flags);
	SMP_LOCK(flags);
	SwBWMode(priv, priv->pshare->CurrentChannelBW, priv->pshare->offset_2nd_chan);
	SwChnl(priv, priv->pmib->dot11RFEntry.dot11channel, priv->pshare->offset_2nd_chan);
	PHY_IQCalibrate(priv); //FOR_8812_IQK
	SMP_UNLOCK(flags);
	RESTORE_INT(flags);


	priv->pmib->dot11DFSEntry.DFS_detected = 0;
	priv->ht_cap_len = 0;
	update_beacon(priv);

	if (priv->pmib->miscEntry.vap_enable) {
		int i;
		for (i=0; i<RTL8192CD_NUM_VWLAN; i++) {
			if (IS_DRV_OPEN(priv->pvap_priv[i])) {
				priv->pvap_priv[i]->pmib->dot11RFEntry.dot11channel = ch;
				priv->pvap_priv[i]->ht_cap_len = 0;
				update_beacon(priv->pvap_priv[i]);
			}
		}
	}

	RTL_W8(TXPAUSE, 0x00);


	while(pstat)
	{
#ifdef TX_SHORTCUT
		if (!priv->pmib->dot11OperationEntry.disable_txsc) {
			int i;
			for (i=0; i<TX_SC_ENTRY_NUM; i++)
				pstat->tx_sc_ent[i].hwdesc1.Dword7 &= set_desc(~TX_TxBufSizeMask);
		}
#endif

		if (IS_HAL_CHIP(priv)) {
			GET_HAL_INTERFACE(priv)->UpdateHalRAMaskHandler(priv, pstat, 3);
		}

		pstat = findNextSTA(priv, &staidx);

	}

	if (!priv->pmib->dot11DFSEntry.disable_DFS &&
			(OPMODE & WIFI_AP_STATE) &&
		(((priv->pmib->dot11RFEntry.dot11channel >= 52) &&
		(priv->pmib->dot11RFEntry.dot11channel <= 64)) ||
		((priv->pmib->dot11RFEntry.dot11channel >= 100) &&
		(priv->pmib->dot11RFEntry.dot11channel <= 140)))) {

		init_timer(&priv->ch_avail_chk_timer);
		priv->ch_avail_chk_timer.data = (unsigned long) priv;
		priv->ch_avail_chk_timer.function = rtl8192cd_ch_avail_chk_timer;

		if ((priv->pmib->dot11StationConfigEntry.dot11RegDomain == DOMAIN_ETSI) &&
			(IS_METEOROLOGY_CHANNEL(priv->pmib->dot11RFEntry.dot11channel)))
			mod_timer(&priv->ch_avail_chk_timer, jiffies + CH_AVAIL_CHK_TO_CE);
		else
			mod_timer(&priv->ch_avail_chk_timer, jiffies + CH_AVAIL_CHK_TO);

		init_timer(&priv->DFS_timer);
		priv->DFS_timer.data = (unsigned long) priv;
		priv->DFS_timer.function = rtl8192cd_DFS_timer;

		init_timer(&priv->DFS_TXPAUSE_timer);
		priv->DFS_TXPAUSE_timer.data = (unsigned long) priv;
		priv->DFS_TXPAUSE_timer.function = rtl8192cd_DFS_TXPAUSE_timer;

		/* DFS activated after 5 sec; prevent switching channel due to DFS false alarm */
		mod_timer(&priv->DFS_timer, jiffies + RTL_SECONDS_TO_JIFFIES(5));

		init_timer(&priv->dfs_det_chk_timer);
		priv->dfs_det_chk_timer.data = (unsigned long) priv;
		priv->dfs_det_chk_timer.function = rtl8192cd_dfs_det_chk_timer;

		mod_timer(&priv->dfs_det_chk_timer, jiffies + RTL_MILISECONDS_TO_JIFFIES(priv->pshare->rf_ft_var.dfs_det_period*10));

		DFS_SetReg(priv);

		priv->pmib->dot11DFSEntry.disable_tx = 1;
	}
	else
		priv->pmib->dot11DFSEntry.disable_tx = 0;
}

