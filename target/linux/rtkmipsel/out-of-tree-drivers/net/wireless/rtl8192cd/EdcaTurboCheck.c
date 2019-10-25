#include "./8192cd_cfg.h"
#include "./8192cd.h"
#include "./8192cd_util.h"
#include "./8192cd_headers.h"

void EdcaParaInit(
	struct rtl8192cd_priv *priv
	)
{
	int   mode=priv->pmib->dot11BssType.net_work_type;
	
	static unsigned int slot_time, sifs_time;
	struct ParaRecord EDCA[4];

	 memset(EDCA, 0, 4*sizeof(struct ParaRecord));

	sifs_time = 10;
	slot_time = 20;

	if (mode & (WIRELESS_11N))
		sifs_time = 16;

	if (mode & (WIRELESS_11N| WIRELESS_11G|WIRELESS_11A))
		slot_time = 9;


#ifdef RTK_AC_SUPPORT //for 11ac logo,  edit aifs time for cca test cases
	if(AC_SIGMA_MODE != AC_SIGMA_NONE)
		sifs_time = 10;	
#endif


#if(defined(RTL_MANUAL_EDCA))
	 if( priv->pmib->dot11QosEntry.ManualEDCA ) {
		 if( OPMODE & WIFI_AP_STATE )
			 memcpy(EDCA, priv->pmib->dot11QosEntry.AP_manualEDCA, 4*sizeof(struct ParaRecord));
		 else
			 memcpy(EDCA, priv->pmib->dot11QosEntry.STA_manualEDCA, 4*sizeof(struct ParaRecord));

		#ifdef WIFI_WMM
		if (QOS_ENABLE)
			RTL_W32(0x504, (EDCA[VI].TXOPlimit<< 16) | (EDCA[VI].ECWmax<< 12) | (EDCA[VI].ECWmin<< 8) | (sifs_time + EDCA[VI].AIFSN* slot_time));
		else
		#endif
			RTL_W32(0x504, (EDCA[BE].TXOPlimit<< 16) | (EDCA[BE].ECWmax<< 12) | (EDCA[BE].ECWmin<< 8) | (sifs_time + EDCA[VI].AIFSN* slot_time));

	}else
	#endif //RTL_MANUAL_EDCA
	{

		 if(OPMODE & WIFI_AP_STATE)
		 {
		 	memcpy(EDCA, rtl_ap_EDCA, 2*sizeof(struct ParaRecord));

			if(mode & (WIRELESS_11A|WIRELESS_11G|WIRELESS_11N))
				memcpy(&EDCA[VI], &rtl_ap_EDCA[VI_AG], 2*sizeof(struct ParaRecord));
			else
				memcpy(&EDCA[VI], &rtl_ap_EDCA[VI], 2*sizeof(struct ParaRecord));
		 }
		 else
		 {
		 	memcpy(EDCA, rtl_sta_EDCA, 2*sizeof(struct ParaRecord));

			if(mode & (WIRELESS_11A|WIRELESS_11G|WIRELESS_11N))
				memcpy(&EDCA[VI], &rtl_sta_EDCA[VI_AG], 2*sizeof(struct ParaRecord));
			else
				memcpy(&EDCA[VI], &rtl_sta_EDCA[VI], 2*sizeof(struct ParaRecord));
		 }
		 
	#ifdef WIFI_WMM
		if (QOS_ENABLE)
			RTL_W32(0x504, (EDCA[VI].TXOPlimit<< 16) | (EDCA[VI].ECWmax<< 12) | (EDCA[VI].ECWmin<< 8) | (sifs_time + EDCA[VI].AIFSN* slot_time));
		else
	#endif

	RTL_W32(0x504,  (EDCA[BK].ECWmax<< 12) | (EDCA[BK].ECWmin<< 8) | (sifs_time + EDCA[VI].AIFSN* slot_time));

			
	}

#if defined(WMM_DSCP_C42)
	if(GET_CHIP_VER(priv) == VERSION_8192E) {
		RTL_W32(0x500, (EDCA[VO].TXOPlimit<< 16) | (EDCA[VO].ECWmax<< 12) | (EDCA[VO].ECWmin<< 8) | (sifs_time + EDCA[VO].AIFSN* slot_time));
		RTL_W32(0x508,	(EDCA[BE].TXOPlimit<< 16) | (EDCA[BE].ECWmax<< 12) | (EDCA[BE].ECWmin<< 8) | (sifs_time + EDCA[BE].AIFSN* slot_time));
		RTL_W32(0x50C, (0x1e<< 16) | (EDCA[BK].ECWmax<< 12) | (EDCA[BK].ECWmin<< 8) | (sifs_time + EDCA[BE].AIFSN* slot_time));
	} else if(GET_CHIP_VER(priv) == VERSION_8814A) {
		RTL_W32(0x500, (EDCA[VO].TXOPlimit<< 16) | (EDCA[VO].ECWmax<< 12) | (EDCA[VO].ECWmin<< 8) | 0x11);
		RTL_W32(0x508,	(EDCA[BE].TXOPlimit<< 16) | (EDCA[BE].ECWmax<< 12) | (EDCA[BE].ECWmin<< 8) | (sifs_time + EDCA[BE].AIFSN* slot_time));
		RTL_W32(0x50C, (0x1e<< 16) | (EDCA[BK].ECWmax<< 12) | (EDCA[BK].ECWmin<< 8) | (sifs_time + EDCA[BK].AIFSN* slot_time));
	} else
#endif
	{
		RTL_W32(0x500, (EDCA[VO].TXOPlimit<< 16) | (EDCA[VO].ECWmax<< 12) | (EDCA[VO].ECWmin<< 8) | (sifs_time + EDCA[VO].AIFSN* slot_time));
		RTL_W32(0x508,  (EDCA[BE].TXOPlimit<< 16) | (EDCA[BE].ECWmax<< 12) | (EDCA[BE].ECWmin<< 8) | (sifs_time + EDCA[BE].AIFSN* slot_time));
		RTL_W32(0x50C, (EDCA[BK].TXOPlimit<< 16) | (EDCA[BK].ECWmax<< 12) | (EDCA[BK].ECWmin<< 8) | (sifs_time + EDCA[BK].AIFSN* slot_time));
	}

#if defined(RTK_AC_SUPPORT) && defined(RTL_MANUAL_EDCA) //for 11ac logo,  make BK worse to seperate with BE.
#ifdef CONFIG_WLAN_HAL_8814AE
	if(GET_CHIP_VER(priv) != VERSION_8814A)
#endif
	if((AC_SIGMA_MODE != AC_SIGMA_NONE) && (priv->pmib->dot11QosEntry.ManualEDCA))
	{
		RTL_W32(0x50C, (EDCA[BK].TXOPlimit<< 16) | (EDCA[BK].ECWmax<< 12) | (EDCA[BK].ECWmin<< 8) | 0xa4 );
	}
#endif

//	ODM_Write1Byte(pDM_Odm,ACMHWCTRL, 0x00);

	priv->pshare->iot_mode_enable = 0;

	if (priv->pshare->rf_ft_var.wifi_beq_iot)
		priv->pshare->iot_mode_VI_exist = 0;
	
	#ifdef WMM_VIBE_PRI
	priv->pshare->iot_mode_BE_exist = 0;
	#endif

#ifdef WMM_BEBK_PRI
	priv->pshare->iot_mode_BK_exist = 0;
#endif
	
	#ifdef LOW_TP_TXOP
	priv->pshare->BE_cwmax_enhance = 0;
	#endif


	priv->pshare->iot_mode_VO_exist = 0;
}

BOOLEAN
ChooseIotMainSTA(
	struct rtl8192cd_priv *priv,
	IN	PSTA_INFO_T		pstat
	)
{
	BOOLEAN		bhighTP_found_pstat=FALSE;
	
	if ((GET_ROOT(priv)->up_time % 2) == 0) {
		unsigned int tx_2s_avg = 0;
		unsigned int rx_2s_avg = 0;
		int i=0, aggReady=0;
		unsigned long total_sum = (priv->pshare->current_tx_bytes+priv->pshare->current_rx_bytes);
		int assoc_num = GET_ROOT(priv)->assoc_num;
#ifdef MBSSID
		if (GET_ROOT(priv)->pmib->miscEntry.vap_enable){
			for (i=0; i<RTL8192CD_NUM_VWLAN; ++i)
				assoc_num += GET_ROOT(priv)->pvap_priv[i]-> assoc_num;
		}
#endif	
#ifdef UNIVERSAL_REPEATER
		if (IS_DRV_OPEN(GET_VXD_PRIV(GET_ROOT(priv))))
			assoc_num += GET_VXD_PRIV(GET_ROOT(priv))-> assoc_num;
#endif
#ifdef WDS
		 if(GET_ROOT(priv)->pmib->dot11WdsInfo.wdsEnabled)
			assoc_num ++;
#endif


		pstat->current_tx_bytes += pstat->tx_byte_cnt;
		pstat->current_rx_bytes += pstat->rx_byte_cnt;

		if (total_sum != 0) {
			if (total_sum <= 1000000) {
			tx_2s_avg = (unsigned int)((pstat->current_tx_bytes*100) / total_sum);
			rx_2s_avg = (unsigned int)((pstat->current_rx_bytes*100) / total_sum);
			} else {
				tx_2s_avg = (unsigned int)(pstat->current_tx_bytes / (total_sum / 100));
				rx_2s_avg = (unsigned int)(pstat->current_rx_bytes / (total_sum / 100));
			}

		}

		for(i=0; i<8; i++)
			aggReady += (pstat->ADDBA_ready[i]);

		if ((pstat->ht_cap_len && (
#ifdef	SUPPORT_TX_AMSDU			
			AMSDU_ENABLE || 
#endif			
			aggReady)) || (pstat->IOTPeer==HT_IOT_PEER_INTEL))
		{
			if ((assoc_num==1) || (tx_2s_avg + rx_2s_avg >= 25)) {
				priv->pshare->highTP_found_pstat = pstat;
#if defined(UNIVERSAL_REPEATER)				
				if(under_apmode_repeater(priv)) {
					if(IS_ROOT_INTERFACE(priv)) {
						priv->pshare->highTP_found_root_pstat = pstat;
					}
				}
#endif				
			}
			
		#ifdef CLIENT_MODE
			if (OPMODE & WIFI_STATION_STATE) {
				if ((tx_2s_avg + rx_2s_avg) >= 20) {
					priv->pshare->highTP_found_pstat = pstat;
					#if defined(UNIVERSAL_REPEATER)				
					if(under_apmode_repeater(priv)) {
						if(IS_VXD_INTERFACE(priv)) {
							priv->pshare->highTP_found_vxd_pstat = pstat;
						}
					}
					#endif
				}
		}
		#endif				
	}

	} 
	else {
		pstat->current_tx_bytes = pstat->tx_byte_cnt;
		pstat->current_rx_bytes = pstat->rx_byte_cnt;
	}

	return bhighTP_found_pstat;
}


#ifdef WIFI_WMM
VOID
IotEdcaSwitch(
	struct rtl8192cd_priv *priv,
	IN	unsigned char		enable
	)
{
	int   mode=priv->pmib->dot11BssType.net_work_type;
	unsigned int slot_time = 20, sifs_time = 10, aifs_time = 41, BE_TXOP = 47, VI_TXOP = 94;
	unsigned int vi_cw_max = 4, vi_cw_min = 3, vi_aifs;
#if defined(CONFIG_USB_HCI) || defined(CONFIG_SDIO_HCI)
	u32 be_edca, vi_edca;
#endif

	if (!(!priv->pmib->dot11OperationEntry.wifi_specific ||
		((OPMODE & WIFI_AP_STATE) && (priv->pmib->dot11OperationEntry.wifi_specific))
	#ifdef CLIENT_MODE
		|| ((OPMODE & WIFI_STATION_STATE) && (priv->pmib->dot11OperationEntry.wifi_specific))
	#endif
		))
		return;


#ifdef RTK_AC_SUPPORT //for 11ac logo, do not dynamic switch edca 
	if(AC_SIGMA_MODE != AC_SIGMA_NONE)
		return;
#endif

	if ((mode & (ODM_WM_N24G|ODM_WM_N5G)) && (priv->pshare->ht_sta_num
	#ifdef WDS
		|| ((OPMODE & WIFI_AP_STATE) && priv->pmib->dot11WdsInfo.wdsEnabled && priv->pmib->dot11WdsInfo.wdsNum)
	#endif
		))
		sifs_time = 16;

	if (mode & (WIRELESS_11N|WIRELESS_11G|WIRELESS_11A)) {
		slot_time = 9;
	} 
	else
	{
		BE_TXOP = 94;
		VI_TXOP = 188;
	}
	if(GET_CHIP_VER(priv) == VERSION_8814A)
		BE_TXOP = 0x1e;
#if 0
	if (orSTABitMap(&priv->pshare->mimo_ps_dynamic_sta)) 
		BE_TXOP = 0;	
#endif
#if defined(CONFIG_USB_HCI) || defined(CONFIG_SDIO_HCI)
	be_edca = -1;
	vi_edca = -1;
#endif


	if ((OPMODE & WIFI_AP_STATE) && priv->pmib->dot11OperationEntry.wifi_specific) {
#if 0
		if (priv->pshare->iot_mode_VO_exist
			&& !(priv->pshare->iot_mode_BE_exist || priv->pshare->iot_mode_BK_exist)) {
	#ifdef WMM_VIBE_PRI
			if (priv->pshare->iot_mode_BE_exist) 
			{
				vi_cw_max = 5;
				vi_cw_min = 3;
				vi_aifs = (sifs_time + ((OPMODE & WIFI_AP_STATE)?1:2) * slot_time);
			}
			else 
	#endif
			{
			vi_cw_max = 6;
			vi_cw_min = 4;
			vi_aifs = 0x2b;
			}
		} else
#endif
		{
			vi_aifs = (sifs_time + ((OPMODE & WIFI_AP_STATE)?1:2) * slot_time);
		}

		if(GET_CHIP_VER(priv) == VERSION_8814A) {
				if(priv->pshare->iot_mode_VO_exist && priv->pshare->iot_mode_VI_exist && priv->pshare->iot_mode_BK_exist) {
					RTL_W8(0x502, 0);
					RTL_W8(0x506, 0);
					RTL_W8(0x50e, 0);
				} else {
					RTL_W8(0x502, rtl_ap_EDCA[VO_AG].TXOPlimit);
					RTL_W8(0x506, rtl_ap_EDCA[VI_AG].TXOPlimit);
					RTL_W8(0x50e, BE_TXOP);			
				}
		} else {
			{
#if defined(CONFIG_USB_HCI) || defined(CONFIG_SDIO_HCI)
		vi_edca = ((VI_TXOP*(1-priv->pshare->iot_mode_VO_exist)) << 16)
			| (vi_cw_max << 12) | (vi_cw_min << 8) | vi_aifs;
#else
		if(priv->pshare->iot_mode_VO_exist /*&& priv->pshare->iot_mode_VI_exist && priv->pshare->iot_mode_BE_exist*/) {
			RTL_W32(0x504, (0x16 << 16)
				| (vi_cw_max << 12) | (vi_cw_min << 8) | 0x19);
			RTL_W8(0x500, 0x13);
		}else{
			RTL_W32(0x504, ((VI_TXOP*(1-priv->pshare->iot_mode_VO_exist)) << 16)
				| (vi_cw_max << 12) | (vi_cw_min << 8) | vi_aifs);
		}
#endif
			}
		}
	#ifdef WMM_BEBK_PRI
	#ifdef CONFIG_RTL_88E_SUPPORT
		if ((GET_CHIP_VER(priv) == VERSION_8188E) && priv->pshare->iot_mode_BK_exist) {
#if defined(CONFIG_USB_HCI) || defined(CONFIG_SDIO_HCI)
			be_edca = (10 << 12) | (6 << 8) | 0x4f;
#else
			RTL_W32(0x50C, (10 << 12) | (6 << 8) | 0x4f);
#endif
		}
	#endif		
	#endif
#if defined(CONFIG_WLAN_HAL_8881A) 
		if (GET_CHIP_VER(priv) == VERSION_8881A) 
			RTL_W32(0x50C,  0xa64f);
#endif		
	}





 	if (priv->pshare->rf_ft_var.wifi_beq_iot && priv->pshare->iot_mode_VI_exist) {
#if defined(CONFIG_RTL_88E_SUPPORT) || defined(CONFIG_RTL_8812_SUPPORT)
		if (GET_CHIP_VER(priv) == VERSION_8188E || GET_CHIP_VER(priv) == VERSION_8812E) {
#if defined(CONFIG_USB_HCI) || defined(CONFIG_SDIO_HCI)
			be_edca = (10 << 12) | (6 << 8) | 0x4f;
#else
		  	RTL_W32(0x508, (10 << 12) | (6 << 8) | 0x4f);
#endif
		}
		else
#endif
		{
#if defined(CONFIG_USB_HCI) || defined(CONFIG_SDIO_HCI)
			be_edca = (10 << 12) | (4 << 8) | 0x4f;
#else
			RTL_W32(0x508, (10 << 12) | (4 << 8) | 0x4f);
#endif
		}
	} else if(!enable)
	{
#if defined(CONFIG_USB_HCI) || defined(CONFIG_SDIO_HCI)
		be_edca = (((OPMODE & WIFI_AP_STATE)?6:10) << 12) | (4 << 8)
			| (sifs_time + 3 * slot_time);
#else
#if defined(CONFIG_WLAN_HAL_8814AE) && defined(WMM_DSCP_C42)						
		if(GET_CHIP_VER(priv) == VERSION_8814A) {
			RTL_W32(0x508, (BE_TXOP << 16) | (((OPMODE & WIFI_AP_STATE)?6:10) << 12) | (4 << 8)
				| (sifs_time + 3 * slot_time));
		}
		else
#endif
		
#if 0//defined(CONFIG_WLAN_HAL_8197F) || defined(CONFIG_WLAN_HAL_8822BE) || defined(CONFIG_WLAN_HAL_8881A) //wishchen
/*
		if(GET_CHIP_VER(priv) == VERSION_8197F && priv->pshare->txop_enlarge == 2 && priv->pshare->iot_mode_BK_exist){
				RTL_W32(0x508, ((BE_TXOP*2) << 16) | (((OPMODE & WIFI_AP_STATE)?6:10) << 12) | (4 << 8)
					| (sifs_time + 3 * slot_time));
		}else
*/
	if((GET_CHIP_VER(priv) == VERSION_8197F || GET_CHIP_VER(priv) == VERSION_8822B || GET_CHIP_VER(priv) == VERSION_8881A) && priv->pshare->iot_mode_BK_exist){
			RTL_W32(0x508, ((BE_TXOP*2) << 16) | (((OPMODE & WIFI_AP_STATE)?6:10) << 12) | (4 << 8)
				| (sifs_time + 3 * slot_time));
			//panic_printk("IotEdcaSwitch: BK\n");
	}else
#endif
	{
		if(priv->pshare->iot_mode_VO_exist || priv->pshare->iot_mode_VI_exist)
			RTL_W32(0x508, (((OPMODE & WIFI_AP_STATE)?6:10) << 12) | (4 << 8) | (sifs_time + 3 * slot_time));
        else
            RTL_W32(0x508, ((BE_TXOP*2) << 16) | (((OPMODE & WIFI_AP_STATE)?6:10) << 12) | (4 << 8) | (sifs_time + 3 * slot_time));
	}
#endif
#ifdef CONFIG_PCI_HCI
//		ODM_Write2Byte(pDM_Odm, RD_CTRL, ODM_Read2Byte(pDM_Odm, RD_CTRL) | (DIS_TXOP_CFE));
#endif
	}
	else
	{
		int txop;
		unsigned int cw_max;
#ifdef LOW_TP_TXOP
		unsigned int txop_close;
#endif
		
	#if(defined LOW_TP_TXOP)
			cw_max = ((priv->pshare->BE_cwmax_enhance) ? 10 : 6);
			txop_close = ((priv->pshare->rf_ft_var.low_tp_txop && priv->pshare->rf_ft_var.low_tp_txop_close) ? 1 : 0);

			if(priv->pshare->txop_enlarge == 0xe)   //if intel case
				txop = (txop_close ? 0 : (BE_TXOP*2));
			else                                                        //if other case
				txop = (txop_close ? 0: (BE_TXOP*priv->pshare->txop_enlarge));
	#else
			cw_max=6;
			if(priv->pshare->txop_enlarge==0xe)
				txop = 0x138;
			else if(priv->pshare->txop_enlarge==0xd)
				txop=BE_TXOP*2;
			else
				txop=BE_TXOP*priv->pshare->txop_enlarge;

	#endif
                           
		if (priv->pshare->ht_sta_num
	#ifdef WDS
			|| ((OPMODE & WIFI_AP_STATE) && (mode & WIRELESS_11N) &&
			priv->pmib->dot11WdsInfo.wdsEnabled && priv->pmib->dot11WdsInfo.wdsNum)
	#endif
			) 
			{

			if (priv->pshare->txop_enlarge == 0xe) {
				// is intel client, use a different edca value
				//ODM_Write4Byte(pDM_Odm, ODM_EDCA_BE_PARAM, (txop<< 16) | (cw_max<< 12) | (4 << 8) | 0x1f);
				if (get_rf_mimo_mode(priv)==MIMO_1T1R) {
#if defined(CONFIG_USB_HCI) || defined(CONFIG_SDIO_HCI)
					be_edca = (txop << 16) | (5 << 12) | (3 << 8) | 0x1f;
#else
					RTL_W32(0x508, (txop << 16) | (5 << 12) | (3 << 8) | 0x1f);
#endif
				}
				else {
#if defined(CONFIG_USB_HCI) || defined(CONFIG_SDIO_HCI)
					be_edca = (txop << 16) | (8 << 12) | (5 << 8) | 0x1f;
#else
#if defined(CONFIG_WLAN_HAL_8814AE) || defined(CONFIG_WLAN_HAL_8822BE) || defined(CONFIG_RTL_8197F)	
					if(GET_CHIP_VER(priv) == VERSION_8814A || GET_CHIP_VER(priv) == VERSION_8822B || GET_CHIP_VER(priv) == VERSION_8197F)
					{
						if (priv->pshare->th_mode == 2) 
							RTL_W32(0x508, (0x0 << 16) | (8 << 12) | (5 << 8) | 0x1f);
						else
							RTL_W32(0x508, (txop << 16) | (8 << 12) | (5 << 8) | 0x1f);

					}
					else
#endif
						RTL_W32(0x508, (txop << 16) | (8 << 12) | (5 << 8) | 0x1f);
#endif
				}
				
#ifdef CONFIG_PCI_HCI
//				ODM_Write2Byte(pDM_Odm, RD_CTRL, ODM_Read2Byte(pDM_Odm, RD_CTRL) & ~(DIS_TXOP_CFE));
#endif
				priv->pshare->txop_enlarge = 2;
			} 

	#ifndef LOW_TP_TXOP
			 else if (priv->pshare->txop_enlarge == 0xd) {
				// is intel ralink, use a different edca value
#if defined(CONFIG_USB_HCI) || defined(CONFIG_SDIO_HCI)
				be_edca = (txop << 16) | (6 << 12) | (5 << 8) | 0x2b;
#else
				RTL_W32(0x508, (txop << 16) | (6 << 12) | (5 << 8) | 0x2b);
#endif
				priv->pshare->txop_enlarge = 2;
			} 
	#endif

			else 
			{
				PSTA_INFO_T 	pstat = priv->pshare->highTP_found_pstat;
#ifdef MCR_WIRELESS_EXTEND
#ifdef CONFIG_WLAN_HAL_8192EE
				if(GET_CHIP_VER(priv) == VERSION_8192E) {
					// TXOP normal
					RTL_W8(0x455,0x3f);
					RTL_W8(0x5BE,0x04);
					if (pstat && (pstat->IOTPeer == HT_IOT_PEER_REALTEK_8812)) {
						if (priv->pshare->th_mode == 2) {
							RTL_W32(0x508, (0xa << 12) | (4 << 8) | 0x19);
							//GDEBUG(" 0x508=0x%x\n", RTL_R32(0x508));
						} else {
							RTL_W32(0x508, (0x5e << 16) | (6 << 12) | (4 << 8) | 0x19);
							//GDEBUG(" 0x508=0x%x\n", RTL_R32(0x508));
						}
					} else {
						if (priv->pshare->th_mode == 2) 
							RTL_W32(0x508, (6 << 12) | (4 << 8) | 0x2b);
						else
							RTL_W32(0x508, (0x5e << 16) | (6 << 12) | (4 << 8) | 0x2b);
					}					
				} else
#endif
#if defined(CONFIG_WLAN_HAL_8814AE) || defined(CONFIG_WLAN_HAL_8822BE)				
				if(GET_CHIP_VER(priv) == VERSION_8814A || GET_CHIP_VER(priv) == VERSION_8822B) {
					if (pstat) {
						if((pstat->IOTPeer == HT_IOT_PEER_REALTEK || pstat->IOTPeer == HT_IOT_PEER_RTK_APCLIENT || pstat->IOTPeer == HT_IOT_PEER_REALTEK_8812
								#if (MU_BEAMFORMING_SUPPORT == 1)
								|| (pstat->IOTPeer == HT_IOT_PEER_REALTEK && is_support_bf(pstat, TXBF_TYPE_MU))
								#endif
								)){
							if(priv->pshare->th_mode == 1) {
								if(priv->pmib->dot11RFEntry.phyBandSelect ==  PHY_BAND_2G)
									txop = 0x138; // 10ms
								else
									txop = 0x5e;
								cw_max = 6;
								
								#if (MU_BEAMFORMING_SUPPORT == 1)
								if(priv->pmib->dot11RFEntry.txbf_mu && is_support_bf(pstat, TXBF_TYPE_MU)) {
									txop = 0x138; // 10ms
									aifs_time = 0x74;
								} else
								#endif								
								{
									aifs_time = 0x19;
								}
							} else if (priv->pshare->th_mode == 2) {
								RTL_W32(0x508, (6 << 12) | (5 << 8) | 0x19);
								// TXOP normal
								RTL_W8(0x455,0x3f);
								//RTL_W8(0x5BE,0x04);
							} else {
								// TXOP Turbo
								RTL_W8(0x455,0x7f);
								//RTL_W8(0x5BE,0x08);
								RTL_W32(0x508, (0x138 << 16) | (6 << 12) | (4 << 8) | 0x19);
							}
						}
						else if (pstat->IOTPeer == HT_IOT_PEER_BROADCOM) {
							if (priv->pshare->th_mode == 2) {
								RTL_W32(0x508, (8 << 12) | (5 << 8) | 0x1f);
								// TXOP normal
								RTL_W8(0x455,0x3f);
								//RTL_W8(0x5BE,0x04);
							} else {
								RTL_W32(0x508, (0x138 << 16) | (6 << 12) | (4 << 8) | 0x19);
								// TXOP Turbo
								RTL_W8(0x455,0x7f);
								//RTL_W8(0x5BE,0x08);
							}
						}
						else {
#if (MU_BEAMFORMING_SUPPORT == 1)
							if(pstat && is_support_bf(pstat, TXBF_TYPE_MU) && pstat->muPartner) {
								txop = 0x138;
								cw_max = 6;
								aifs_time = 0x74;
							} else
#endif
							if (priv->pshare->th_mode == 2) {
								RTL_W32(0x508, (6 << 12) | (4 << 8) | 0x2b);
								// TXOP normal
								RTL_W8(0x455,0x3f);
								//RTL_W8(0x5BE,0x04);
							} else {
								if (pstat->vht_cap_len) {
									RTL_W32(0x508, (0x138 << 16) | (6 << 12) | (4 << 8) | 0x2b);
									// TXOP Turbo
									RTL_W8(0x455,0x7f);
									//RTL_W8(0x5BE,0x08);
								} else {
									RTL_W32(0x508, (0x5e << 16) | (6 << 12) | (4 << 8) | 0x2b);
									// TXOP normal
									RTL_W8(0x455,0x3f);
									//RTL_W8(0x5BE,0x04);
								}
							}
						}
						RTL_W32(0x508, (txop << 16) |
	                            (cw_max << 12) | (4 << 8) | (aifs_time));
					}
				} else
#endif
#else
#if defined(CONFIG_WLAN_HAL_8814AE) || defined(CONFIG_WLAN_HAL_8822BE) || defined(CONFIG_RTL_8197F)				
				//if (get_rf_mimo_mode(priv)==MIMO_3T3R) 
				if(GET_CHIP_VER(priv) == VERSION_8814A || GET_CHIP_VER(priv) == VERSION_8822B || GET_CHIP_VER(priv) == VERSION_8197F)
				{
#if defined(CONFIG_USB_HCI) || defined(CONFIG_SDIO_HCI)
                                        be_edca = (txop << 16) | (cw_max << 12) | (4 << 8) | (sifs_time + 3 * slot_time);
#else					
					{
						PSTA_INFO_T 	pstat = priv->pshare->highTP_found_pstat;
						if ( pstat && (pstat->IOTPeer == HT_IOT_PEER_BROADCOM)) {					
							if(priv->pshare->th_mode == 1) {
								//GDEBUG("TXOP turbo for BroadComd\n");	
								if(priv->pmib->dot11RFEntry.phyBandSelect ==  PHY_BAND_2G){
									if(pstat->StaRxBuf <= 8)
										txop = 0x0; //turn off txop
									else	
										txop = 0x138; // 10ms
								}else
									txop = 0x138; // 10ms

								cw_max = 6;
								aifs_time = 0x1f;
							} else if(priv->pshare->th_mode == 2) {
								//GDEBUG("TXOP disable for BroadComd\n"); 
								txop = 0x0;
								cw_max = 8;
								if ((pstat->StaRxBuf <= 8)&&(priv->pmib->dot11RFEntry.phyBandSelect ==  PHY_BAND_2G))
									aifs_time = 0x2b;
								else
									aifs_time = 0x1f;
							} else {						
								//GDEBUG("TXOP ELSE for BroadComd\n"); 
								txop = 0x138; // 10ms
								cw_max = 6;
								aifs_time = 0x19;
							}
						} else if( pstat && (pstat->IOTPeer == HT_IOT_PEER_REALTEK || pstat->IOTPeer == HT_IOT_PEER_RTK_APCLIENT || pstat->IOTPeer == HT_IOT_PEER_REALTEK_8812
								#if (MU_BEAMFORMING_SUPPORT == 1)
								|| (pstat->IOTPeer == HT_IOT_PEER_REALTEK && is_support_bf(pstat, TXBF_TYPE_MU))
								#endif
								)){
							if(priv->pshare->th_mode == 1) {
								//GDEBUG("TXOP Turbo for Realtek\n");
								
									txop = 0x138; // 10ms
								cw_max = 6;
								
								#if (MU_BEAMFORMING_SUPPORT == 1)
								if(priv->pmib->dot11RFEntry.txbf_mu && is_support_bf(pstat, TXBF_TYPE_MU)) {
									txop = 0x138; // 10ms
									aifs_time = 0x74;
								} else
								#endif								
								{
									aifs_time = 0x19;
								}
							} else if(priv->pshare->th_mode == 2) {
								//GDEBUG("TXOP Disable for Realtek\n");
								txop = 0x0;
								cw_max = 6;
								aifs_time = 0x19;
							} else {						
								txop = 0x5e;
								cw_max = 6;
								aifs_time = 0x19;
							}
						}else{			
							#if (MU_BEAMFORMING_SUPPORT == 1)
							if(pstat && is_support_bf(pstat, TXBF_TYPE_MU) && pstat->muPartner) {
								txop = 0x138;
								cw_max = 6;
								aifs_time = 0x74;
							} else
							#endif
							{
								if(priv->pshare->th_mode == 2)
									txop = 0x0;
								else
								txop = 0x138;
								
								cw_max = 6;
								aifs_time = 0x2b;
							}
						}
						RTL_W32(0x508, (txop << 16) |
	                            (cw_max << 12) | (4 << 8) | (aifs_time));
					}									
#endif
                                }
                                else
#endif
#endif
				if (get_rf_mimo_mode(priv)==MIMO_2T2R) {
#if defined(CONFIG_USB_HCI) || defined(CONFIG_SDIO_HCI)
					be_edca = (txop << 16) | (cw_max << 12) | (4 << 8) | (sifs_time + 3 * slot_time);
#else
					RTL_W32(0x508, (txop << 16) |
						(cw_max << 12) | (4 << 8) | (sifs_time + 3 * slot_time));
#endif
				}
				else
				#if(DM_ODM_SUPPORT_TYPE==ODM_AP)&&(defined LOW_TP_TXOP)
				{
#if defined(CONFIG_USB_HCI) || defined(CONFIG_SDIO_HCI)
					be_edca = (txop << 16) |
						(((priv->pshare->BE_cwmax_enhance) ? 10 : 5) << 12) | (3 << 8) | (sifs_time + 2 * slot_time);
#else
					RTL_W32(0x508, (txop << 16) |
						(((priv->pshare->BE_cwmax_enhance) ? 10 : 5) << 12) | (3 << 8) | (sifs_time + 2 * slot_time));
#endif
				}
				#else
				{
					PSTA_INFO_T		pstat = priv->pshare->highTP_found_pstat;
					if ((GET_CHIP_VER(priv)==VERSION_8881A) && pstat && (pstat->IOTPeer == HT_IOT_PEER_HTC))
					RTL_W32(0x508, 0x642b);
					else {
#if defined(CONFIG_USB_HCI) || defined(CONFIG_SDIO_HCI)
					be_edca = (txop << 16) | (5 << 12) | (3 << 8) | (sifs_time + 2 * slot_time);
#else
					RTL_W32(0x508, (txop << 16) |
						(5 << 12) | (3 << 8) | (sifs_time + 2 * slot_time));
#endif
					}
				}
				#endif
			}
		}
              else 
              {
#if(defined LOW_TP_TXOP)
#if defined(CONFIG_USB_HCI) || defined(CONFIG_SDIO_HCI)
			 be_edca = (BE_TXOP << 16) | (cw_max << 12) | (4 << 8) | (sifs_time + 3 * slot_time);
#else
			 RTL_W32(0x508, (BE_TXOP << 16) | (cw_max << 12) | (4 << 8) | (sifs_time + 3 * slot_time));
#endif
#endif
              }

	}
	
#if defined(CONFIG_USB_HCI) || defined(CONFIG_SDIO_HCI)
	notify_IOT_EDCA_switch(priv, be_edca, vi_edca);
#endif
}
#endif

VOID 
IotEngine(
	struct rtl8192cd_priv *priv
	)
{
	PSTA_INFO_T pstat = NULL;
	u4Byte i;
#if defined(WMM_DSCP_C42) && defined(TX_SHORTCUT)
	 u2Byte do_tx_slowpath=0;
#endif	
#ifdef WIFI_WMM
	unsigned int switch_turbo = 0, avg_tp;
#endif	
	int assoc_num = GET_ROOT(priv)->assoc_num;
#ifdef MBSSID
	if (GET_ROOT(priv)->pmib->miscEntry.vap_enable){
		for (i=0; i<RTL8192CD_NUM_VWLAN; ++i) {
			if (IS_DRV_OPEN(GET_ROOT(priv)->pvap_priv[i]))
				assoc_num += GET_ROOT(priv)->pvap_priv[i]->assoc_num;
		}
	}
#endif	
#ifdef UNIVERSAL_REPEATER
	if (IS_DRV_OPEN(GET_VXD_PRIV(GET_ROOT(priv))))
		assoc_num += GET_VXD_PRIV(GET_ROOT(priv))-> assoc_num;
#endif
#ifdef WDS
	if(GET_ROOT(priv)->pmib->dot11WdsInfo.wdsEnabled)
		assoc_num ++;
#endif
////////////////////////////////////////////////////////
//  if EDCA Turbo function is not supported or Manual EDCA Setting
//  then return
////////////////////////////////////////////////////////

	
#if(defined(RTL_MANUAL_EDCA) && defined(WIFI_WMM))
	if(priv->pmib->dot11QosEntry.ManualEDCA){
		return ;
	}
#endif 

	pstat=priv->pshare->highTP_found_pstat;
	if(pstat) {
		if((pstat->tx_avarage + pstat->rx_avarage) < (3<<17))	// 3M bps
			pstat = NULL;
	}
	if((GET_ROOT(priv)->up_time % 2))
		return;
	
#ifdef WIFI_WMM
	if (QOS_ENABLE) {
		if (!priv->pmib->dot11OperationEntry.wifi_specific 
			||((OPMODE & WIFI_AP_STATE) && (priv->pmib->dot11OperationEntry.wifi_specific))
			) {
			if (priv->pshare->iot_mode_enable &&
				((priv->pshare->phw->VO_pkt_count > 50) ||
				 (priv->pshare->phw->VI_pkt_count > 50) ||
				 (priv->pshare->phw->BK_pkt_count > 50))) {
				priv->pshare->iot_mode_enable = 0;
				switch_turbo++;
#ifdef CONFIG_WLAN_HAL_8881A
				if (GET_CHIP_VER(priv) == VERSION_8881A) {
					RTL_W32(0x460, 0x03086666);
				}
#endif //CONFIG_WLAN_HAL_8881A
			} else if ((!priv->pshare->iot_mode_enable) &&
				((priv->pshare->phw->VO_pkt_count < 50) &&
				 (priv->pshare->phw->VI_pkt_count < 50) &&
				 (priv->pshare->phw->BK_pkt_count < 50))) {
				priv->pshare->iot_mode_enable++;
				switch_turbo++;
//#ifdef CONFIG_WLAN_HAL_8881A
#if 0
				if (GET_CHIP_VER(priv) == VERSION_8881A) {
					if (get_bonding_type_8881A()==BOND_8881AB) {
						RTL_W32(0x460, 0x03086666);
					}
					else {
						RTL_W32(0x460, 0x0320ffff);
					}
				}
#endif //CONFIG_WLAN_HAL_8881A
			}
		}

		if (priv->pshare->iot_mode_enable && orSTABitMap(&priv->pshare->mimo_ps_dynamic_sta)) {
		   switch_turbo++;
		}

		if ((OPMODE & WIFI_AP_STATE) && priv->pmib->dot11OperationEntry.wifi_specific)
		{
			if (!priv->pshare->iot_mode_VO_exist && (priv->pshare->phw->VO_pkt_count > 50)) {
				priv->pshare->iot_mode_VO_exist++;
				switch_turbo++;
#if defined(WMM_DSCP_C42) && defined(TX_SHORTCUT)
				do_tx_slowpath = 1;
#endif				
			} else if (priv->pshare->iot_mode_VO_exist && (priv->pshare->phw->VO_pkt_count < 50)) {
				priv->pshare->iot_mode_VO_exist = 0;
				switch_turbo++;
#if defined(WMM_DSCP_C42) && defined(TX_SHORTCUT)
				do_tx_slowpath = 1;
#endif				
			}
#if defined(WMM_DSCP_C42) && defined(TX_SHORTCUT)
			if ((IS_HAL_CHIP(priv)) && do_tx_slowpath &&!priv->pmib->dot11OperationEntry.disable_txsc ) {
				int j=0;
				struct stat_info *psta=NULL;						
				psta = findNextSTA(priv, &j);
				while(psta) {				
					memset(psta->tx_sc_ent, 0, sizeof(psta->tx_sc_ent));
					psta = findNextSTA(priv, &j);
				};	
			}				
#endif			
#if(defined WMM_VIBE_PRI)
			if (priv->pshare->iot_mode_VO_exist) {
				//printk("[%s %d] BE_pkt_count=%d\n", __FUNCTION__, __LINE__, priv->pshare->phw->BE_pkt_count);
				if (!priv->pshare->iot_mode_BE_exist && (priv->pshare->phw->BE_pkt_count > 250)) {
					priv->pshare->iot_mode_BE_exist++;
					switch_turbo++;
				} else if (priv->pshare->iot_mode_BE_exist && (priv->pshare->phw->BE_pkt_count < 250)) {
					priv->pshare->iot_mode_BE_exist = 0;
					switch_turbo++;
				}
			}
#endif

#if 1//(defined WMM_BEBK_PRI)
			if (priv->pshare->phw->BE_pkt_count) {
				//printk("[%s %d] BK_pkt_count=%d\n", __FUNCTION__, __LINE__, priv->pshare->phw->BK_pkt_count);
				if (!priv->pshare->iot_mode_BK_exist && (priv->pshare->phw->BK_pkt_count > 250)) {
					priv->pshare->iot_mode_BK_exist++;
					switch_turbo++;
				} else if (priv->pshare->iot_mode_BK_exist && (priv->pshare->phw->BK_pkt_count < 250)) {
					priv->pshare->iot_mode_BK_exist = 0;
					switch_turbo++;
				}
			}
#endif


			if (priv->pshare->rf_ft_var.wifi_beq_iot) 
			{
				if (!priv->pshare->iot_mode_VI_exist && (priv->pshare->phw->VI_rx_pkt_count > 50)) {
					priv->pshare->iot_mode_VI_exist++;
					switch_turbo++;
				} else if (priv->pshare->iot_mode_VI_exist && (priv->pshare->phw->VI_rx_pkt_count < 50)) {
					priv->pshare->iot_mode_VI_exist = 0;
					switch_turbo++;
				}
			} else {
				if (!priv->pshare->iot_mode_VI_exist && (priv->pshare->phw->VI_pkt_count > 50)) {
					priv->pshare->iot_mode_VI_exist++;
					switch_turbo++;
				} else if (priv->pshare->iot_mode_VI_exist && (priv->pshare->phw->VI_pkt_count < 50)) {
					priv->pshare->iot_mode_VI_exist = 0;
					switch_turbo++;
				}
			}


		}
		if (!pstat || ((priv->pmib->dot11RFEntry.phyBandSelect & PHY_BAND_2G) && pstat->rssi < priv->pshare->rf_ft_var.txop_enlarge_lower)) {
		   if (priv->pshare->txop_enlarge) {
			   priv->pshare->txop_enlarge = 0;
			   if (priv->pshare->iot_mode_enable)
					switch_turbo++;
				}
         	}

#if(defined(CLIENT_MODE) )
        if ((OPMODE & WIFI_STATION_STATE) && (priv->pmib->dot11OperationEntry.wifi_specific))
        {
            if (priv->pshare->iot_mode_enable &&
                (((priv->pshare->phw->VO_pkt_count > 50) ||
                 (priv->pshare->phw->VI_pkt_count > 50) ||
                 (priv->pshare->phw->BK_pkt_count > 50)) ||
                 (pstat && (!pstat->ADDBA_ready[0]) & (!pstat->ADDBA_ready[3]))))
            {
                priv->pshare->iot_mode_enable = 0;
                switch_turbo++;
            }
            else if ((!priv->pshare->iot_mode_enable) &&
                (((priv->pshare->phw->VO_pkt_count < 50) &&
                 (priv->pshare->phw->VI_pkt_count < 50) &&
                 (priv->pshare->phw->BK_pkt_count < 50)) &&
                 (pstat && (pstat->ADDBA_ready[0] | pstat->ADDBA_ready[3]))))
            {
                priv->pshare->iot_mode_enable++;
                switch_turbo++;
            }
        }
#endif

		priv->pshare->phw->VO_pkt_count = 0;
		priv->pshare->phw->VI_pkt_count = 0;
		priv->pshare->phw->BK_pkt_count = 0;

		priv->pshare->phw->VI_droppkt_count = 0;
		priv->pshare->phw->BE_droppkt_count = 0;	
		priv->pshare->phw->VO_droppkt_count = 0;
		priv->pshare->phw->BK_droppkt_count = 0;		

	#if 1//(defined WMM_VIBE_PRI)
		priv->pshare->phw->BE_pkt_count = 0;
	#endif
		
		if (priv->pshare->rf_ft_var.wifi_beq_iot)
			priv->pshare->phw->VI_rx_pkt_count = 0;

	}
#endif

	if ((priv->up_time % 2) == 0) {
		/*
		 * decide EDCA content for different chip vendor
		 */
#ifdef WIFI_WMM
		if (QOS_ENABLE && (!priv->pmib->dot11OperationEntry.wifi_specific || 
			((OPMODE & WIFI_AP_STATE) && (priv->pmib->dot11OperationEntry.wifi_specific == 2))
		#ifdef CLIENT_MODE
            || ((OPMODE & WIFI_STATION_STATE) && (priv->pmib->dot11OperationEntry.wifi_specific == 2))
		#endif
		))
	
		{

			if (pstat && pstat->rssi >= priv->pshare->rf_ft_var.txop_enlarge_upper) {
#ifdef LOW_TP_TXOP
				if (pstat->IOTPeer==HT_IOT_PEER_INTEL)
				{
					if (priv->pshare->txop_enlarge != 0xe)
					{
						priv->pshare->txop_enlarge = 0xe;

						if (priv->pshare->iot_mode_enable)
							switch_turbo++;
					}
				} 
				else if (pstat->IOTPeer==HT_IOT_PEER_RALINK)
				{
					priv->pshare->txop_enlarge = 0xd;		
					if (priv->pshare->iot_mode_enable)
						switch_turbo++;
				}
				else if (pstat->IOTPeer==HT_IOT_PEER_HTC)
				{
					priv->pshare->txop_enlarge = 0;		
					if (priv->pshare->iot_mode_enable)
						switch_turbo++;
				}
				else if (priv->pshare->txop_enlarge != 2) 
				{
					priv->pshare->txop_enlarge = 2;
					if (priv->pshare->iot_mode_enable)
						switch_turbo++;
				}
#else
				if (pstat->IOTPeer==HT_IOT_PEER_INTEL) {
					if (priv->pshare->txop_enlarge != 0xe)	{
						priv->pshare->txop_enlarge = 0xe;
						if (priv->pshare->iot_mode_enable)
							switch_turbo++;
					}
				} else if (priv->pshare->txop_enlarge != 2)
				{
					if (pstat->IOTPeer==HT_IOT_PEER_INTEL)
						priv->pshare->txop_enlarge = 0xe;
					else if (pstat->IOTPeer==HT_IOT_PEER_RALINK)
						priv->pshare->txop_enlarge = 0xd;
					else if (pstat->IOTPeer==HT_IOT_PEER_HTC)
						priv->pshare->txop_enlarge = 0;		
					else
						priv->pshare->txop_enlarge = 2;

					if (priv->pshare->iot_mode_enable)
						switch_turbo++;
				}
#endif
			}
			else if ((!pstat || ((priv->pmib->dot11RFEntry.phyBandSelect & PHY_BAND_2G) && pstat->rssi < priv->pshare->rf_ft_var.txop_enlarge_lower)))
			{
#ifdef SW_TX_QUEUE
			 	if(priv->pshare->swq_en == 0)	{
					if (priv->pshare->txop_enlarge) {
						priv->pshare->txop_enlarge = 0;
						if (priv->pshare->iot_mode_enable)
							switch_turbo++;
					}
				} else
#endif		
				{	
					if(priv->pshare->iot_mode_enable)  {
						priv->pshare->txop_enlarge=2;
						switch_turbo++;
					}
				}
			}

#if(defined LOW_TP_TXOP)
			// for Intel IOT, need to enlarge CW MAX from 6 to 10
			if (pstat && pstat->IOTPeer==HT_IOT_PEER_INTEL && (((pstat->tx_avarage+pstat->rx_avarage)>>10) < 
					priv->pshare->rf_ft_var.cwmax_enhance_thd)) 
			{
				if (!priv->pshare->BE_cwmax_enhance && priv->pshare->iot_mode_enable)
				{
					priv->pshare->BE_cwmax_enhance = 1;
					switch_turbo++;
				}
			} else {
				if (priv->pshare->BE_cwmax_enhance) {
					priv->pshare->BE_cwmax_enhance = 0;
					switch_turbo++;
				}
			}
#endif
		}
#endif
		priv->pshare->current_tx_bytes = 0;
		priv->pshare->current_rx_bytes = 0;
	}else {
		if ((GET_CHIP_VER(priv) == VERSION_8881A) || (GET_CHIP_VER(priv) == VERSION_8188E)){
			unsigned int uldl_tp = (priv->pshare->current_tx_bytes+priv->pshare->current_rx_bytes)>>17;
			if((uldl_tp > 40 || assoc_num>10) && (priv->pshare->agg_to!= 1)) {
				RTL_W8(0x462, 0x08);
				priv->pshare->agg_to = 1;
			} else if((uldl_tp < 35) && (priv->pshare->agg_to !=0)) {
				RTL_W8(0x462, 0x02);
				priv->pshare->agg_to = 0;
			} 
		}
	}	
	
#if(defined SW_TX_QUEUE)
	if(AMPDU_ENABLE) {
#ifdef TX_EARLY_MODE
		if (GET_TX_EARLY_MODE) {
			if (!GET_EM_SWQ_ENABLE &&
				((assoc_num > 1) ||
				(pstat && pstat->IOTPeer != HT_IOT_PEER_UNKNOWN))) {
				if ((priv->pshare->em_tx_byte_cnt >> 17) > EM_TP_UP_BOUND) 
					priv->pshare->reach_tx_limit_cnt++;				
				else					
					priv->pshare->reach_tx_limit_cnt = 0;	

				if (priv->pshare->txop_enlarge && priv->pshare->reach_tx_limit_cnt /*>= WAIT_TP_TIME*/) {
					GET_EM_SWQ_ENABLE = 1;			
					priv->pshare->reach_tx_limit_cnt = 0;

					if (pstat->IOTPeer == HT_IOT_PEER_INTEL)
						MAX_EM_QUE_NUM = 12;
					else if (pstat->IOTPeer == HT_IOT_PEER_RALINK)
						MAX_EM_QUE_NUM = 10;
					
					enable_em(priv);			
				}
			}
			else if (GET_EM_SWQ_ENABLE) {
				if ((priv->pshare->em_tx_byte_cnt >> 17) < EM_TP_LOW_BOUND)
					priv->pshare->reach_tx_limit_cnt++;				
				else					
					priv->pshare->reach_tx_limit_cnt = 0;	

				if (!priv->pshare->txop_enlarge || priv->pshare->reach_tx_limit_cnt >= WAIT_TP_TIME) {
					GET_EM_SWQ_ENABLE = 0;
					priv->pshare->reach_tx_limit_cnt = 0;
					disable_em(priv);
				}
			}
		}
#endif

        if((priv->ext_stats.tx_avarage>>17)>TP_HIGH_WATER_MARK) {
            if ((priv->pshare->txop_decision == 0)){
                switch_turbo++;

                if (pstat) {
                    if (pstat->IOTPeer==HT_IOT_PEER_INTEL)
                        priv->pshare->txop_enlarge = 0xe;
                    else if (pstat->IOTPeer==HT_IOT_PEER_RALINK)
                        priv->pshare->txop_enlarge = 0xd;
                    else if (pstat->IOTPeer==HT_IOT_PEER_HTC)
                        priv->pshare->txop_enlarge = 0;		
                    else
                        priv->pshare->txop_enlarge = 2;				

                } else if (priv->pshare->txop_enlarge == 0) {
                    priv->pshare->txop_enlarge = 2;
                }
                priv->pshare->txop_decision = 1;
            }
        }
        else{
            if (priv->pshare->swq_txmac_chg >= priv->pshare->rf_ft_var.swq_en_highthd){
                if ((priv->pshare->txop_decision == 0)){
                    switch_turbo++;
                    if (priv->pshare->txop_enlarge == 0) {
                        if( assoc_num < 10)
                            priv->pshare->txop_enlarge = 2;
                        else
                            priv->pshare->txop_enlarge = 1;
                    }
                    priv->pshare->txop_decision = 1;
                }
                else
                {
                    if ((switch_turbo > 0) && (priv->pshare->txop_enlarge == 0) && (priv->pshare->iot_mode_enable != 0))
                    {
                        if( assoc_num < 10)
                            priv->pshare->txop_enlarge = 2;
                        else
                            priv->pshare->txop_enlarge = 1;

                        switch_turbo--;
                    }
                }
            }
            else if(priv->pshare->swq_txmac_chg <= priv->pshare->rf_ft_var.swq_dis_lowthd){
                priv->pshare->txop_decision = 0;
            }
            else if ((priv->pshare->txop_decision == 1) && (switch_turbo > 0) && (priv->pshare->txop_enlarge == 0) && (priv->pshare->iot_mode_enable != 0)){
                if( assoc_num < 10)
                    priv->pshare->txop_enlarge = 2;
                else
                    priv->pshare->txop_enlarge = 1;
                switch_turbo--;
            }
        } 			

        if(priv->pshare->rf_ft_var.swq_enable == 0) {
            priv->pshare->swq_en = 0;
        }
        else {
            if(priv->pshare->swq_en == 0) {
                if(priv->pshare->swq_use_hw_timer) { /* new SWQ*/
                    priv->pshare->swq_en = 1;
                }
                else { /* Old SWQ, check TP and tx mac change*/
                    if((priv->ext_stats.tx_avarage>>17) > TP_HIGH_WATER_MARK ||
                        priv->pshare->swq_txmac_chg >= priv->pshare->rf_ft_var.swq_en_highthd) {
                        priv->pshare->swq_en = 1;
                    }      

                    #if (DM_ODM_SUPPORT_TYPE==ODM_AP) && defined(TX_EARLY_MODE)		
                    if (GET_EM_SWQ_ENABLE == 1)
                        priv->pshare->swq_en = 1;
                    #endif                       
                }
                                            
                if(priv->pshare->swq_en == 1) {
                    priv->pshare->swqen_keeptime = priv->up_time;
                    init_STA_SWQAggNum(priv);
                }
            }
            else {
                if(priv->pshare->swq_use_hw_timer == 0) {   /*Old SWQ, check TP and tx mac change*/  
                    if((priv->ext_stats.tx_avarage>>17) <= TP_HIGH_WATER_MARK && 
                        priv->pshare->swq_txmac_chg <= priv->pshare->rf_ft_var.swq_dis_lowthd
                        #if (DM_ODM_SUPPORT_TYPE==ODM_AP) && defined(TX_EARLY_MODE)     
                        && (GET_EM_SWQ_ENABLE == 0)
                        #endif 
                    ){
                        priv->pshare->swq_en = 0;
                    }
                }
                
            }

        }
    
    }
    else {
        priv->pshare->swq_en = 0;
    }
#if defined(MCR_WIRELESS_EXTEND) && !defined(WMM_DSCP_C42)
		if (GET_CHIP_VER(priv) == VERSION_8814A) {
			if (assoc_num > 9) {
				priv->pshare->swq_en = 1;
				priv->pshare->swqen_keeptime = priv->up_time;		
				extern void init_STA_SWQAggNum(struct rtl8192cd_priv *priv);
				init_STA_SWQAggNum(priv);	
			} else {
				priv->pshare->swq_en = 0;
				priv->pshare->swqen_keeptime = 0;
			}
		} 
#endif
		if(priv->pshare->rf_ft_var.swq_enable == 0) {
			priv->pshare->swq_en = 0;
			priv->pshare->swqen_keeptime = 0;
		} else if (priv->pshare->rf_ft_var.swq_enable == 2) {
			priv->pshare->swq_en = 1;
			priv->pshare->swqen_keeptime = priv->up_time;		
			extern void init_STA_SWQAggNum(struct rtl8192cd_priv *priv);
			init_STA_SWQAggNum(priv);	
		}	
#endif

#ifdef WIFI_WMM
#ifdef LOW_TP_TXOP
	if ((!priv->pmib->dot11OperationEntry.wifi_specific || (priv->pmib->dot11OperationEntry.wifi_specific == 2))
		&& QOS_ENABLE) {
		if (switch_turbo || priv->pshare->rf_ft_var.low_tp_txop) {
			unsigned int thd_tp;
			unsigned char under_thd;
			unsigned int curr_tp;

			if (priv->pmib->dot11BssType.net_work_type & (WIRELESS_11N| WIRELESS_11G))
			{
				// Determine the upper bound throughput threshold.
				if (priv->pmib->dot11BssType.net_work_type & WIRELESS_11N) {
					if (priv->assoc_num && priv->assoc_num != priv->pshare->ht_sta_num)
						thd_tp = priv->pshare->rf_ft_var.low_tp_txop_thd_g;
					else
						thd_tp = priv->pshare->rf_ft_var.low_tp_txop_thd_n;
				}
				else
					thd_tp = priv->pshare->rf_ft_var.low_tp_txop_thd_g;

				// Determine to close txop.
#if defined(UNIVERSAL_REPEATER) || defined(MBSSID)
				if(IS_STA_VALID(pstat)) 
				{	
					struct rtl8192cd_priv *tmppriv;
					struct aid_obj *aidarray;	
					aidarray = container_of(pstat, struct aid_obj, station);
					tmppriv = aidarray->priv;

					curr_tp = (unsigned int)(tmppriv->ext_stats.tx_avarage>>17) + (unsigned int)(tmppriv->ext_stats.rx_avarage>>17);
				} 
				else 
#endif
				curr_tp = (unsigned int)(priv->ext_stats.tx_avarage>>17) + (unsigned int)(priv->ext_stats.rx_avarage>>17);
				if (curr_tp <= thd_tp && curr_tp >= priv->pshare->rf_ft_var.low_tp_txop_thd_low)
					under_thd = 1;
				else
					under_thd = 0;
			}
			else
			{
				under_thd = 0;
			}

			if (switch_turbo) 
			{
				priv->pshare->rf_ft_var.low_tp_txop_close = under_thd;
				priv->pshare->rf_ft_var.low_tp_txop_count = 0;
			}
			else if (priv->pshare->iot_mode_enable && (priv->pshare->rf_ft_var.low_tp_txop_close != under_thd)) {
				priv->pshare->rf_ft_var.low_tp_txop_count++;
				if (priv->pshare->rf_ft_var.low_tp_txop_close) {
					priv->pshare->rf_ft_var.low_tp_txop_count = priv->pshare->rf_ft_var.low_tp_txop_delay;
				}
				if (priv->pshare->rf_ft_var.low_tp_txop_count ==priv->pshare->rf_ft_var.low_tp_txop_delay) 

				{					
					priv->pshare->rf_ft_var.low_tp_txop_count = 0;
					priv->pshare->rf_ft_var.low_tp_txop_close = under_thd;
					switch_turbo++;
				}
			} 
			else 
			{
				priv->pshare->rf_ft_var.low_tp_txop_count = 0;
			}
		}
	}
#endif		

#if defined(WMM_DSCP_C42) && defined(CONFIG_WLAN_HAL_8814AE)
	if (switch_turbo && (GET_CHIP_VER(priv) == VERSION_8814A) ) {
		if (!priv->pshare->iot_mode_enable && !priv->pshare->aggrmax_change) {
				RTL_W16(0x4ca, 0x2c2c);
			priv->pshare->aggrmax_change = 1;
		}
		else if (priv->pshare->iot_mode_enable && priv->pshare->aggrmax_change) {
			RTL_W16(0x4ca, priv->pshare->aggrmax_bak);
			priv->pshare->aggrmax_change = 0;
		}
	} 
#endif
#ifdef TX_EARLY_MODE
		unsigned int em_tp = ((priv->ext_stats.tx_avarage>>17) + (priv->ext_stats.rx_avarage>>17));
		if (em_tp > 80)
			RTL_W32(0x508, (0x5e << 16) | (4 << 12) | (3 << 8) | 0x19);
		else //if (em_tp < 75)
			RTL_W32(0x508, (0x5e << 16) | (6 << 12) | (5 << 8) | 0x2b);
#endif

#if defined(UNIVERSAL_REPEATER)
	if(under_apmode_repeater(priv)) {
		if(priv->pshare->highTP_found_root_pstat && priv->pshare->highTP_found_vxd_pstat) {
			if(priv->pshare->highTP_found_vxd_pstat->tx_avarage > priv->pshare->highTP_found_root_pstat->tx_avarage) {
				priv->pshare->highTP_found_pstat = priv->pshare->highTP_found_vxd_pstat;
			}
			else {
				priv->pshare->highTP_found_pstat = priv->pshare->highTP_found_root_pstat;
			}
		}
	}
#endif	

#if defined(CONFIG_WLAN_HAL_8814AE) || defined(CONFIG_WLAN_HAL_8822BE)
	if(GET_CHIP_VER(priv) == VERSION_8814A || GET_CHIP_VER(priv) == VERSION_8822B) {

		pstat = priv->pshare->highTP_found_pstat;
		if(pstat) {
			if((pstat->tx_avarage>>17) > 15 && (priv->pshare->th_mode!=1)) {				
				priv->pshare->th_mode = 1; // TX mode
				switch_turbo++;
			} else if((pstat->tx_avarage>>17) < 10 && (priv->pshare->th_mode!=2)) {				
				priv->pshare->th_mode = 2; // RX mode
				switch_turbo++;
			}

#if 0//(MU_BEAMFORMING_SUPPORT == 1)
			if(GET_CHIP_VER(priv) == VERSION_8822B) {
				if(priv->pshare->txop_turbo!=1) {					
					if(is_support_bf(pstat, TXBF_TYPE_MU) && pstat->muPartner) {
						priv->pshare->txop_turbo = 1;
						priv->pshare->rf_ft_var.reg_bcn_hold_backup = RTL_R32(0x540);
						RTL_W32(0x540, priv->pshare->rf_ft_var.reg_bcn_hold_backup & ~(0x000fff00) | (0x40 << 8));
						switch_turbo++;
					}
				} else if(priv->pshare->txop_turbo!=0) { //txop normal
					if(!(is_support_bf(pstat, TXBF_TYPE_MU) && pstat->muPartner)) {
						priv->pshare->txop_turbo = 0;
						if(priv->pshare->rf_ft_var.reg_bcn_hold_backup !=0)
						RTL_W32(0x540, priv->pshare->rf_ft_var.reg_bcn_hold_backup);
						switch_turbo++;
					}
				}
			}
			else 
#endif			
			{
				if(priv->pmib->dot11RFEntry.phyBandSelect ==  PHY_BAND_2G) {
					if(priv->pshare->txop_turbo!=1) { // txop turbo
	                    if((priv->pshare->CurrentChannelBW == 2 &&(pstat->tx_avarage>>17) > 500) ||
	                        (priv->pshare->CurrentChannelBW == 1 &&(pstat->tx_avarage>>17) > 200) ||
	                        (priv->pshare->CurrentChannelBW == 0)) {
	                        // TXOP Turbo
							RTL_W8(0x455,0x7f);
							//RTL_W8(0x5BE,0x08);
							priv->pshare->txop_turbo = 1;
						}				
					} else if(priv->pshare->txop_turbo!=0) { //txop normal
						if((priv->pshare->CurrentChannelBW == 2 &&(pstat->tx_avarage>>17) < 400) ||
						   (priv->pshare->CurrentChannelBW == 1 &&(pstat->tx_avarage>>17) < 150)) {
							RTL_W8(0x455,0x3f);
							//RTL_W8(0x5BE,0x04);
							priv->pshare->txop_turbo = 0;
						}				
					}
				}
			}

#if 0
			if(pstat->IOTPeer == HT_IOT_PEER_BROADCOM) {
				if((pstat->rx_avarage>>17) < 400 && priv->pshare->amsdu_mode !=0) {
					RTL_W32(0x144c,0x08060806);
					priv->pshare->amsdu_mode = 0;
				} else if((pstat->rx_avarage>>17) > 450 && priv->pshare->amsdu_mode !=1) { 
					RTL_W32(0x144c,0xffffffff);
					priv->pshare->amsdu_mode = 1;
				}
			} else { // highTP stat change
				if(priv->pshare->amsdu_mode == 1) {
					RTL_W32(0x144c,0x08060806);
					priv->pshare->amsdu_mode = 0;
				}
			}
#endif			
		}
	}
#endif	
#if defined(MBSSID)
//	if (priv->pmib->miscEntry.vap_enable && assoc_num>10 && switch_turbo) {
//		RTL_W16(RD_CTRL, RTL_R16(RD_CTRL) | (DIS_TXOP_CFE));
//	}
#endif
	if (switch_turbo) {
		IotEdcaSwitch( priv, priv->pshare->iot_mode_enable );
		if (priv->pshare->highTP_found_pstat && IS_HAL_CHIP(priv))
			GET_HAL_INTERFACE(priv)->UpdateHalRAMaskHandler(priv, priv->pshare->highTP_found_pstat, 3);		
	}
#endif
}


