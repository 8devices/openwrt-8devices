#include "8192cd.h"
#include "8192cd_cfg.h"
#include "8192cd_util.h"
#include "8192cd_headers.h"
#include "Beamforming.h"
#include "8812_reg.h"
#include "8812_vht_gen.h"



#if (BEAMFORMING_SUPPORT == 1)
VOID
Beamforming_SetBeamFormInit(
	struct rtl8192cd_priv *priv
		)
{
#ifdef CONFIG_WLAN_HAL_8822BE
		if(GET_CHIP_VER(priv) == VERSION_8822B)
			SetBeamformInit8822B(priv);
#endif
}


VOID
Beamforming_SetBeamFormLeave(
	struct rtl8192cd_priv *priv,
	u1Byte				Idx
	)
{

#ifdef CONFIG_RTL_8812_SUPPORT
		if(GET_CHIP_VER(priv) == VERSION_8812E)
			SetBeamformLeave8812(priv, Idx);	
#endif

#ifdef CONFIG_WLAN_HAL_8192EE
		if(GET_CHIP_VER(priv) == VERSION_8192E)
			SetBeamformLeave92E(priv,Idx);
#endif

#ifdef CONFIG_WLAN_HAL_8814AE
		if(GET_CHIP_VER(priv) == VERSION_8814A)
			SetBeamformLeave8814A(priv,Idx);
#endif

#ifdef CONFIG_WLAN_HAL_8881A
		if(GET_CHIP_VER(priv) == VERSION_8881A)
			SetBeamformLeave8881A(priv,Idx);

#endif

#ifdef CONFIG_WLAN_HAL_8197F
		if(GET_CHIP_VER(priv) == VERSION_8197F)
			SetBeamformLeave8197F(priv,Idx);
#endif

#ifdef CONFIG_WLAN_HAL_8822BE
		if(GET_CHIP_VER(priv) == VERSION_8822B)
			SetBeamformLeave8822B(priv,Idx);
#endif

}

VOID
Beamforming_SetBeamFormStatus(
	struct rtl8192cd_priv *priv,
	u1Byte				Idx
	)
{

#ifdef CONFIG_WLAN_HAL_8192EE
		if(GET_CHIP_VER(priv)== VERSION_8192E)
			SetBeamformStatus92E(priv, Idx);		
#endif
#ifdef CONFIG_RTL_8812_SUPPORT
		if(GET_CHIP_VER(priv)== VERSION_8812E)
			SetBeamformStatus8812(priv, Idx);
#endif

#ifdef CONFIG_WLAN_HAL_8814AE
		if(GET_CHIP_VER(priv) == VERSION_8814A)
			SetBeamformStatus8814A(priv,Idx);
#endif

#ifdef CONFIG_WLAN_HAL_8197F
		if(GET_CHIP_VER(priv) == VERSION_8197F)
			SetBeamformStatus8197F(priv,Idx);
#endif

#ifdef CONFIG_WLAN_HAL_8822BE
		if(GET_CHIP_VER(priv) == VERSION_8822B)
			SetBeamformStatus8822B(priv,Idx);
#endif

}

VOID
Beamforming_SetBeamFormEnter(
	struct rtl8192cd_priv *priv,
	u1Byte				Idx
	)
{
	ODM_RT_TRACE(ODMPTR, PHYDM_COMP_TXBF, ODM_DBG_LOUD, ("[%s] Enter\n", __FUNCTION__)); 
#ifdef CONFIG_WLAN_HAL_8192EE
		if (GET_CHIP_VER(priv)== VERSION_8192E)	
			SetBeamformEnter92E(priv, Idx);
#endif
#ifdef CONFIG_RTL_8812_SUPPORT
		if (GET_CHIP_VER(priv)== VERSION_8812E)	
			SetBeamformEnter8812(priv, Idx);
#endif
#ifdef CONFIG_WLAN_HAL_8814AE
		if(GET_CHIP_VER(priv) == VERSION_8814A)
			SetBeamformEnter8814A(priv,Idx);
#endif

#ifdef CONFIG_WLAN_HAL_8881A
		if(GET_CHIP_VER(priv)== VERSION_8881A)
			SetBeamformEnter8881A(priv, Idx);
#endif

#ifdef CONFIG_WLAN_HAL_8197F
		if(GET_CHIP_VER(priv) == VERSION_8197F)
			SetBeamformEnter8197F(priv,Idx);
#endif

#ifdef CONFIG_WLAN_HAL_8822BE
		if(GET_CHIP_VER(priv) == VERSION_8822B)
			SetBeamformEnter8822B(priv,Idx);
#endif
}

VOID
Beamforming_CSIRate(
	struct rtl8192cd_priv *priv
)
{
#ifdef CONFIG_WLAN_HAL_8822BE
		if(GET_CHIP_VER(priv) == VERSION_8822B) {
			Beamforming_CSIRate_8822B(priv);
		}		
#endif

}


VOID
Beamforming_NDPARate(
	struct rtl8192cd_priv *priv,
	BOOLEAN		Mode,
	u1Byte		BW,
	u1Byte		Rate
	)
{
#ifdef CONFIG_WLAN_HAL_8192EE
	if(GET_CHIP_VER(priv)== VERSION_8192E){
		Beamforming_NDPARate_92E(priv, Mode, BW, Rate);
	}		
#endif
#ifdef CONFIG_RTL_8812_SUPPORT
	if(GET_CHIP_VER(priv)== VERSION_8812E) {
		Beamforming_NDPARate_8812(priv, Mode, BW, Rate);  //
	}
#endif
#ifdef CONFIG_WLAN_HAL_8814AE
	if(GET_CHIP_VER(priv)== VERSION_8814A){
		Beamforming_NDPARate_8814A(priv, Mode, BW, Rate);
	}		
#endif

#ifdef CONFIG_WLAN_HAL_8197F
	if(GET_CHIP_VER(priv)== VERSION_8197F){
		Beamforming_NDPARate_8197F(priv, Mode, BW, Rate);
	}		
#endif

#ifdef CONFIG_WLAN_HAL_8822BE
	if(GET_CHIP_VER(priv) == VERSION_8822B) {
		Beamforming_NDPARate_8822B(priv, 0, 0, Rate);
	}		
#endif

}

VOID
Beamforming_SetHWTimer(
	struct rtl8192cd_priv *priv,
	u2Byte	t
	)
{
#ifdef CONFIG_WLAN_HAL_8192EE
		if(GET_CHIP_VER(priv)== VERSION_8192E)
		{			
			HW_VAR_HW_REG_TIMER_STOP_92E(priv);
			HW_VAR_HW_REG_TIMER_INIT_92E(priv, t);
			HW_VAR_HW_REG_TIMER_START_92E(priv);
		}
#endif
#ifdef CONFIG_RTL_8812_SUPPORT
		if(GET_CHIP_VER(priv)== VERSION_8812E)
		{
			HW_VAR_HW_REG_TIMER_STOP_8812(priv);
			HW_VAR_HW_REG_TIMER_INIT_8812(priv, t);
			HW_VAR_HW_REG_TIMER_START_8812(priv);
		}
#endif
#ifdef CONFIG_WLAN_HAL_8814AE
		if(GET_CHIP_VER(priv)== VERSION_8814A)
		{			
			HW_VAR_HW_REG_TIMER_STOP_8814A(priv);
			HW_VAR_HW_REG_TIMER_INIT_8814A(priv, t);
			HW_VAR_HW_REG_TIMER_START_8814A(priv);
		}
#endif
}

VOID
Beamforming_StopHWTimer(
	struct rtl8192cd_priv *priv
	)
{
#ifdef CONFIG_WLAN_HAL_8192EE
		if(GET_CHIP_VER(priv)== VERSION_8192E)
		{			
			HW_VAR_HW_REG_TIMER_STOP_92E(priv);
		}
#endif
#ifdef CONFIG_RTL_8812_SUPPORT
		if(GET_CHIP_VER(priv)== VERSION_8812E)
		{
			HW_VAR_HW_REG_TIMER_STOP_8812(priv);
		}
#endif
#ifdef CONFIG_WLAN_HAL_8814AE
		if(GET_CHIP_VER(priv)== VERSION_8814A)
		{			
			HW_VAR_HW_REG_TIMER_STOP_8814A(priv);
		}
#endif
}

u1Byte
Beamforming_GetTxSSNum(
	struct rtl8192cd_priv *priv
	)
{
	u1Byte			Ntx = 0;

	if(get_rf_mimo_mode(priv) == MIMO_4T4R)
		Ntx = 3;
	else if(get_rf_mimo_mode(priv) == MIMO_3T4R)
		Ntx = 2;
	else if(get_rf_mimo_mode(priv) == MIMO_3T3R)
		Ntx = 2;
	else if(get_rf_mimo_mode(priv) == MIMO_2T4R)
		Ntx = 1;
	else if(get_rf_mimo_mode(priv) == MIMO_2T3R)
		Ntx = 1;
	else if(get_rf_mimo_mode(priv) == MIMO_2T2R)
		Ntx = 1;
	else
		Ntx = 1;

	return Ntx;

}

u1Byte
Beamforming_GetHTNDPTxRate(
	struct rtl8192cd_priv *priv,
	u1Byte	CompSteeringNumofBFer
)
{
	u1Byte Nr_index = 0;
	u1Byte NDPTxRate;

#ifdef CONFIG_WLAN_HAL_8814AE
	if(GET_CHIP_VER(priv)== VERSION_8814A)
		Nr_index = TxBF_Nr(halTxbf8814A_GetNtx(priv), CompSteeringNumofBFer);		/*find Nr*/
	else
#endif
		Nr_index = TxBF_Nr(1, CompSteeringNumofBFer);		/*find Nr*/
	switch(Nr_index)
	{
		case 1:
		NDPTxRate = _MCS8_RATE_;
		break;

		case 2:
		NDPTxRate = _MCS16_RATE_;
		break;

		case 3:
		NDPTxRate = _MCS24_RATE_;
		break;
			
		default:
		NDPTxRate = _MCS8_RATE_;
		break;
	
	}

return NDPTxRate;

}

u1Byte
Beamforming_GetVHTNDPTxRate(
	struct rtl8192cd_priv *priv,
	u1Byte	CompSteeringNumofBFer
)
{
	u1Byte Nr_index = 0;
	u1Byte NDPTxRate;

#ifdef CONFIG_WLAN_HAL_8814AE
	if(GET_CHIP_VER(priv)== VERSION_8814A)
		Nr_index = TxBF_Nr(halTxbf8814A_GetNtx(priv), CompSteeringNumofBFer);		/*find Nr*/
	else
#endif
		Nr_index = TxBF_Nr(1, CompSteeringNumofBFer);		/*find Nr*/
	
	switch(Nr_index)
	{
		case 1:
		NDPTxRate = _NSS2_MCS0_RATE_;
		break;

		case 2:
		NDPTxRate = _NSS3_MCS0_RATE_;
		break;

		case 3:
		NDPTxRate = _NSS4_MCS0_RATE_;
		break;
			
		default:
		NDPTxRate = _NSS2_MCS0_RATE_;
		break;
	
	}

return NDPTxRate;

}


VOID
PacketAppendData(
	IN	POCTET_STRING	packet,
	IN	OCTET_STRING	data
	)
{
	pu1Byte buf = packet->Octet + packet->Length;
	memcpy( buf, data.Octet, data.Length);
	packet->Length = packet->Length + data.Length;
}



BEAMFORMING_CAP
Beamforming_GetEntryBeamCapByMacId(
	struct rtl8192cd_priv *priv,
	IN	u1Byte		MacId
	)
{
	u1Byte	i = 0;
	PRT_BEAMFORMING_INFO	pBeamformingInfo = &(priv->pshare->BeamformingInfo);
	BEAMFORMING_CAP			BeamformEntryCap = BEAMFORMING_CAP_NONE;
	
	for(i = 0; i < BEAMFORMEE_ENTRY_NUM; i++)
	{
		if(pBeamformingInfo->BeamformeeEntry[i].bUsed &&
			(MacId == pBeamformingInfo->BeamformeeEntry[i].MacId))
		{
			BeamformEntryCap =  pBeamformingInfo->BeamformeeEntry[i].BeamformEntryCap;
			i = BEAMFORMEE_ENTRY_NUM;
		}
	}

	return BeamformEntryCap;
}


PRT_BEAMFORMING_ENTRY
Beamforming_GetBFeeEntryByAddr(
	struct rtl8192cd_priv *priv,
	IN	pu1Byte		RA,
	OUT	pu1Byte		Idx
	)
{
	u1Byte	i = 0;
	PRT_BEAMFORMING_INFO pBeamformingInfo = &(priv->pshare->BeamformingInfo);
	
	for(i = 0; i < BEAMFORMEE_ENTRY_NUM; i++)
	{
		if((pBeamformingInfo->BeamformeeEntry[i].bUsed) && 
			((isEqualMACAddr(RA, pBeamformingInfo->BeamformeeEntry[i].MacAddr))))
		{
			*Idx = i;
			return &(pBeamformingInfo->BeamformeeEntry[i]);
		}
	}
	return NULL;
}

PRT_BEAMFORMER_ENTRY
Beamforming_GetBFerEntryByAddr(
	struct rtl8192cd_priv *priv,
	IN	pu1Byte		RA,
	OUT	pu1Byte		Idx
	)
{
	u1Byte	i = 0;
	PRT_BEAMFORMING_INFO pBeamformingInfo = &(priv->pshare->BeamformingInfo);
	
	for(i = 0; i < BEAMFORMER_ENTRY_NUM; i++)
	{
		if((pBeamformingInfo->BeamformerEntry[i].bUsed) && 
			((isEqualMACAddr(RA, pBeamformingInfo->BeamformerEntry[i].MacAddr))))
		{
			*Idx = i;
			return &(pBeamformingInfo->BeamformerEntry[i]);
		}
	}
	return NULL;
}


PRT_BEAMFORMING_ENTRY
Beamforming_GetEntryByMacId(
	struct rtl8192cd_priv *priv,
	u1Byte		MacId,
	pu1Byte		Idx
	)
{
	u1Byte	i = 0;
	PRT_BEAMFORMING_INFO pBeamformingInfo = &(priv->pshare->BeamformingInfo);
	
	for(i = 0; i < BEAMFORMEE_ENTRY_NUM; i++)
	{
		if(pBeamformingInfo->BeamformeeEntry[i].bUsed && 
			(MacId == pBeamformingInfo->BeamformeeEntry[i].MacId))
		{
			*Idx = i;
			return &(pBeamformingInfo->BeamformeeEntry[i]);
		}
	}

	return NULL;

}

PRT_BEAMFORMING_ENTRY
Beamforming_GetFreeBFeeEntry(
	struct rtl8192cd_priv *priv,
    OUT	pu1Byte		Idx,
    pu1Byte             RA
	)
{
	u1Byte	i = 0;
	PRT_BEAMFORMING_INFO pBeamformingInfo = &(priv->pshare->BeamformingInfo);
	
	ODM_RT_TRACE(ODMPTR, PHYDM_COMP_TXBF, ODM_DBG_LOUD, ("[Beamforming]%s Start!\n", __FUNCTION__));

	ODM_RT_TRACE(ODMPTR, PHYDM_COMP_TXBF, ODM_DBG_LOUD, ("[Beamforming]%s, RA= 0x %x%x%x%x%x%x\n",
		__FUNCTION__,
		RA[0],RA[1],RA[2],RA[3],RA[4],RA[5]));

	ODM_RT_TRACE(ODMPTR, PHYDM_COMP_TXBF, ODM_DBG_LOUD, ("[Beamforming]%s, DelBFeeEntry_Idx0= 0x %x%x%x%x%x%x\n",
	__FUNCTION__,
    pBeamformingInfo->DelEntryListByMACAddr.BFeeEntry_Idx0[0],
    pBeamformingInfo->DelEntryListByMACAddr.BFeeEntry_Idx0[1],
    pBeamformingInfo->DelEntryListByMACAddr.BFeeEntry_Idx0[2],
    pBeamformingInfo->DelEntryListByMACAddr.BFeeEntry_Idx0[3],
    pBeamformingInfo->DelEntryListByMACAddr.BFeeEntry_Idx0[4],
    pBeamformingInfo->DelEntryListByMACAddr.BFeeEntry_Idx0[5]));

    if( (isEqualMACAddr(RA, pBeamformingInfo->DelEntryListByMACAddr.BFeeEntry_Idx0)) ){
		ODM_RT_TRACE(ODMPTR, PHYDM_COMP_TXBF, ODM_DBG_LOUD, ("[Beamforming]@%s, (memcmp(RA, pBeamformingInfo->DelEntryListByMACAddr.BFeeEntry_Idx0, MACADDRLEN)) == 1, return NULL!!\n", __FUNCTION__));
        return NULL;
    }
    else if((isEqualMACAddr(RA, pBeamformingInfo->DelEntryListByMACAddr.BFeeEntry_Idx1))){
		ODM_RT_TRACE(ODMPTR, PHYDM_COMP_TXBF, ODM_DBG_LOUD, ("[Beamforming]@%s, (memcmp(RA, pBeamformingInfo->DelEntryListByMACAddr.BFeeEntry_Idx1, MACADDRLEN)) == 1, return NULL!!\n", __FUNCTION__));
        return NULL;
    }

	for(i = 0; i < BEAMFORMEE_ENTRY_NUM; i++)
	{
		//DbgPrint("Check Point 1: BeamformeeEntry[%d].bUsed = %d\n", i, pBeamformingInfo->BeamformeeEntry[i].bUsed);
		if(pBeamformingInfo->BeamformeeEntry[i].bUsed == FALSE)
		{
			*Idx = i;
			return &(pBeamformingInfo->BeamformeeEntry[i]);
		}	
	}
	return NULL;
}

PRT_BEAMFORMER_ENTRY
Beamforming_GetFreeBFerEntry(
	struct rtl8192cd_priv *priv,
	OUT	pu1Byte		Idx,
	pu1Byte             RA
	)
{
	u1Byte	i = 0;
	PRT_BEAMFORMING_INFO pBeamformingInfo = &(priv->pshare->BeamformingInfo);
	
	ODM_RT_TRACE(ODMPTR, PHYDM_COMP_TXBF, ODM_DBG_LOUD, ("%s Start!\n", __FUNCTION__));
    
    if((isEqualMACAddr(RA, pBeamformingInfo->DelEntryListByMACAddr.BFerEntry_Idx0))){
		ODM_RT_TRACE(ODMPTR, PHYDM_COMP_TXBF, ODM_DBG_LOUD, ("%s, (memcmp(RA, DelEntryListByMACAddr.BFerEntry_Idx0), return NULL!!\n", __FUNCTION__));
        return NULL;
    }
    else if((isEqualMACAddr(RA, pBeamformingInfo->DelEntryListByMACAddr.BFerEntry_Idx1))){
		ODM_RT_TRACE(ODMPTR, PHYDM_COMP_TXBF, ODM_DBG_LOUD, ("%s, (memcmp(RA, DelEntryListByMACAddr.BFerEntry_Idx1), return NULL!!\n", __FUNCTION__));
        return NULL;
    }
    
	for(i = 0; i < BEAMFORMER_ENTRY_NUM; i++)
	{
		if(pBeamformingInfo->BeamformerEntry[i].bUsed == FALSE)
		{
			*Idx = i;
			return &(pBeamformingInfo->BeamformerEntry[i]);
		}	
	}
	return NULL;
}

VOID
Beamforming_GidPAid(
	struct rtl8192cd_priv *priv,
	struct stat_info	*pstat)
{
//eric-mu ??
#if 1
	if (GET_CHIP_VER(priv) == VERSION_8822B) { //MU-MIMO: to be done
		PRT_BEAMFORMING_INFO pBeamformingInfo = &(priv->pshare->BeamformingInfo);
		PRT_BEAMFORMING_ENTRY	pEntry;
		u1Byte idx;
		
		pstat->g_id = 63;
		for (idx = 0; idx < BEAMFORMEE_ENTRY_NUM; idx++) {
			pEntry = &(pBeamformingInfo->BeamformeeEntry[idx]);
			
			if (pstat->aid == pEntry->AID) {
				pstat->p_aid = pEntry->P_AID;
				break;
			}
		}
	}else
#endif
	if (OPMODE & WIFI_AP_STATE)
	{
		u2Byte	AID = (u2Byte) ((pstat->aid) & 0x1ff); 		//AID[0:8]
		u2Byte	bssid = 0;	

		pstat->g_id = 63;
		
		bssid = ((BSSID[5] & 0xf0) >> 4) ^ (BSSID[5] & 0xf);	// BSSID[44:47] xor BSSID[40:43]
		pstat->p_aid = (AID + bssid * 32) & 0x1ff;		// (dec(A) + dec(B)*32) mod 512		

	}
	else if (OPMODE & WIFI_ADHOC_STATE)
	{
		pstat->p_aid = REMAP_AID(pstat);
		pstat->g_id = 63;

	}
	else if (OPMODE & WIFI_STATION_STATE)
	{
		pstat->g_id = 0;		
		pstat->p_aid = ((int)(pstat->hwaddr[5])<<1) | (pstat->hwaddr[4]>>7);
	}	
}


PRT_BEAMFORMING_ENTRY
Beamforming_AddBFeeEntry(
	struct rtl8192cd_priv *priv,
	struct stat_info	*pSTA,
		pu1Byte				RA,
		u2Byte				AID,
		u2Byte				MacID,
		u1Byte				BW,
		BEAMFORMING_CAP		BeamformCap,
		pu1Byte				Idx,
		u2Byte				CompSteeringNumofBFer
	)
{
	PRT_BEAMFORMING_ENTRY	pEntry;
	pEntry = Beamforming_GetFreeBFeeEntry(priv, Idx,RA);
	PRT_BEAMFORMING_INFO 		pBeamInfo = &(priv->pshare->BeamformingInfo);
	u4Byte i;

#if defined(BEAMFORMING_AUTO) && defined(TXPWR_LMT)
	if(priv->pshare->rf_ft_var.txbf_pwrlmt == TXBF_TXPWRLMT_AUTO) {
		if((BeamformCap & BEAMFORMER_CAP_VHT_SU)) {
			if((Beamforming_GetVHTNDPTxRate(priv, CompSteeringNumofBFer) == _NSS2_MCS0_RATE_) && !priv->pshare->txbferVHT2TX) {
				return NULL;
			}
			#ifdef CONFIG_WLAN_HAL_8814AE  
			if((Beamforming_GetVHTNDPTxRate(priv, CompSteeringNumofBFer) == _NSS3_MCS0_RATE_) && !priv->pshare->txbferVHT3TX) {
				return NULL;
			}
			#endif
		} else if((BeamformCap & BEAMFORMER_CAP_HT_EXPLICIT)) {
			if((Beamforming_GetHTNDPTxRate(priv, CompSteeringNumofBFer) == _MCS8_RATE_) && !priv->pshare->txbferHT2TX) {
				return NULL;
			}
			#ifdef CONFIG_WLAN_HAL_8814AE  
			if((Beamforming_GetHTNDPTxRate(priv, CompSteeringNumofBFer) == _MCS16_RATE_) && !priv->pshare->txbferHT3TX) {
				return NULL;
			}
			#endif
		}
	}
#endif		

    // Check if this MAC address is in DelEntryList
    
	ODM_RT_TRACE(ODMPTR, PHYDM_COMP_TXBF, ODM_DBG_LOUD, ("[%s]Get BFee Idx = %d\n", __FUNCTION__, *Idx));

	if(pEntry != NULL)
	{	

		if(GET_CHIP_VER(priv) == VERSION_8822B){
#if (MU_BEAMFORMING_SUPPORT == 1)
			pBeamInfo->beamformee_mu_cnt = 0;
#endif
			pBeamInfo->beamformee_su_cnt = 0;

			for(i = 0; i < BEAMFORMEE_ENTRY_NUM; i++)
			{
				if(pBeamInfo->BeamformeeEntry[i].bUsed){
#if (MU_BEAMFORMING_SUPPORT == 1)					
					if(pBeamInfo->BeamformeeEntry[i].BeamformEntryCap & BEAMFORM_CAP_VHT_MU_BFER)
						pBeamInfo->beamformee_mu_cnt ++ ;
					else
#endif						
						pBeamInfo->beamformee_su_cnt ++ ;
				}
			}
#if (MU_BEAMFORMING_SUPPORT == 1)
			if (BeamformCap & BEAMFORM_CAP_VHT_MU_BFER){
				if(pBeamInfo->beamformee_mu_cnt >= MAX_NUM_BEAMFORMEE_MU)
					return NULL;
			}
			else 
#endif				
			{
#if (MU_BEAMFORMING_SUPPORT == 1)			
				if((BeamformCap & BEAMFORM_CAP_VHT_MU_BFER) && pBeamInfo->beamformee_mu_cnt >= MAX_NUM_BEAMFORMEE_MU)
					return NULL;
#endif				
				if((BeamformCap & BEAMFORMER_CAP_VHT_SU) && pBeamInfo->beamformee_su_cnt >= MAX_NUM_BEAMFORMEE_SU)
					return NULL;
					
			}
		}

		pEntry->bUsed = TRUE;
		pEntry->AID = AID;
		pEntry->MacId = MacID;
		pEntry->BW = BW;
		pEntry->Sounding_En = TRUE;

		if (OPMODE & WIFI_AP_STATE)
		{
			u2Byte bssid = ((GET_MY_HWADDR[5]>> 4) & 0x0f ) ^ 
							(GET_MY_HWADDR[5] & 0xf);				// BSSID[44:47] xor BSSID[40:43]
			pEntry->P_AID = (AID + (bssid <<5)) & 0x1ff;				// (dec(A) + dec(B)*32) mod 512	

		}
		else if (OPMODE & WIFI_ADHOC_STATE)
		{
			pEntry->P_AID = 0;
	
		}
		else if (OPMODE & WIFI_STATION_STATE) {
			pEntry->P_AID =  RA[5];						// BSSID[39:47]
			pEntry->P_AID = (pEntry->P_AID << 1) | (RA[4] >> 7 );
		}
			
		memcpy(pEntry->MacAddr, RA, MACADDRLEN);
		pEntry->bTxBF = FALSE;
		pEntry->bSound = FALSE;
#if (MU_BEAMFORMING_SUPPORT == 1)		
		pEntry->SoundPeriod = priv->pshare->rf_ft_var.mutime;
#endif
		
		pEntry->BeamformEntryCap = BeamformCap;	
		pEntry->BeamformEntryState = BEAMFORMING_ENTRY_STATE_UNINITIALIZE;
		pEntry->LogStatusFailCnt = 0;

		pEntry->CompSteeringNumofBFer = CompSteeringNumofBFer;
		pEntry->pSTA = pSTA;
		pEntry->priv = priv;
		pEntry->pSTA->inTXBFEntry = 1;

		beamform_UpdateMinSoundingPeriod(priv, pEntry->SoundPeriod, FALSE);
#if 1
		if(GET_CHIP_VER(priv) == VERSION_8822B) {

			pEntry->HwState = BEAMFORM_ENTRY_HW_STATE_ADD_INIT;

#if (MU_BEAMFORMING_SUPPORT == 1)
			if (pEntry->BeamformEntryCap & BEAMFORM_CAP_VHT_MU_BFER)
			{			
				pBeamInfo->beamformee_mu_cnt ++ ;
				pBeamInfo->FirstMUBFeeIndex = beamform_GetFirstMUBFeeEntryIdx(priv);					
			}
			else 
#endif			
			{
				pBeamInfo->beamformee_su_cnt ++;
			}

		}
#endif

#ifdef MBSSID
		if(GET_CHIP_VER(priv) == VERSION_8812E)
			if (GET_MIB(GET_ROOT(priv))->miscEntry.vap_enable)
				rtl8192cd_set_mbssid(priv, RA, *Idx);
#endif
		return pEntry;
	}
	else
		return NULL;
}

PRT_BEAMFORMER_ENTRY
Beamforming_AddBFerEntry(
		struct rtl8192cd_priv *priv,
		pu1Byte				RA,
		u2Byte				AID,
		BEAMFORMING_CAP	BeamformCap,
		pu1Byte				Idx,
		u2Byte				NumofSoundingDim
	)
{
	PRT_BEAMFORMER_ENTRY	pEntry;
	PRT_BEAMFORMING_INFO 		pBeamInfo = &(priv->pshare->BeamformingInfo);
	u4Byte i;
    
	ODM_RT_TRACE(ODMPTR, PHYDM_COMP_TXBF, ODM_DBG_LOUD, ("[Beamforming]%s Start!\n", __FUNCTION__));

    pEntry = Beamforming_GetFreeBFerEntry(priv, Idx, RA);

	if(pEntry != NULL)
	{
		pEntry->bUsed = TRUE;			

		// AID -> P_AID
		if (OPMODE & WIFI_AP_STATE)
		{
                u2Byte bssid = ((GET_MY_HWADDR[5]>> 4) & 0x0f ) ^ (GET_MY_HWADDR[5] & 0xf);     // BSSID[44:47] xor BSSID[40:43]
			pEntry->P_AID = (AID + (bssid <<5)) & 0x1ff;				// (dec(A) + dec(B)*32) mod 512	
		}
		else if (OPMODE & WIFI_ADHOC_STATE)
			pEntry->P_AID = 0;
		else if (OPMODE & WIFI_STATION_STATE) {
			pEntry->P_AID =  RA[5];						// BSSID[39:47]
			pEntry->P_AID = (pEntry->P_AID << 1) | (RA[4] >> 7 );
		}

		memcpy(pEntry->MacAddr, RA, MACADDRLEN);
		pEntry->BeamformEntryCap = BeamformCap;	
		pEntry->NumofSoundingDim = NumofSoundingDim;
		
        pEntry->ClockResetTimes = 0;
        pEntry->NDPAPreLogSeq = 0;
        pEntry->NDPALogSeq = 0;
        pEntry->NDPALogRetryCnt = 0;
        pEntry->NDPALogSuccess = 0;


		if(GET_CHIP_VER(priv) == VERSION_8822B){
			
			ODM_RT_TRACE(ODMPTR, PHYDM_COMP_TXBF, ODM_DBG_LOUD, ("\n\n %s !!pEntry->BeamformEntryCap = 0x%x pBeamInfo->BeamformCap = 0x%x \n\n", __FUNCTION__, pEntry->BeamformEntryCap, pBeamInfo->BeamformCap));
			
			#if (MU_BEAMFORMING_SUPPORT == 1)
			if ((pBeamInfo->BeamformCap & BEAMFORM_CAP_VHT_MU_BFEE) &&
				(pEntry->BeamformEntryCap & BEAMFORM_CAP_VHT_MU_BFEE)) {
				pBeamInfo->beamformer_mu_cnt += 1;
				pEntry->AID = AID;
				ODM_RT_TRACE(ODMPTR, PHYDM_COMP_TXBF, ODM_DBG_LOUD, ("Add BFer entry beamformer_mu_cnt=%d\n", pBeamInfo->beamformer_mu_cnt));
			} 
			else 
			#endif						
			if ((pBeamInfo->BeamformCap & (BEAMFORMEE_CAP_VHT_SU|BEAMFORMEE_CAP_HT_EXPLICIT)) &&
				(pEntry->BeamformEntryCap & (BEAMFORMEE_CAP_VHT_SU|BEAMFORMEE_CAP_HT_EXPLICIT))) {
				pBeamInfo->beamformer_su_cnt += 1;

				// Record HW idx info
				for (i = 0; i < MAX_NUM_BEAMFORMER_SU; i++) {
					if ((pBeamInfo->beamformer_su_reg_maping & BIT(i)) == 0) {
						pBeamInfo->beamformer_su_reg_maping |= BIT(i);
						pEntry->su_reg_index = i;
						break;
					}
				}
				ODM_RT_TRACE(ODMPTR, PHYDM_COMP_TXBF, ODM_DBG_LOUD, ("Add BFer entry beamformer_su_reg_maping=%#X, su_reg_index=%d\n", pBeamInfo->beamformer_su_reg_maping, pEntry->su_reg_index));
			}
		}
			ODM_RT_TRACE(ODMPTR, PHYDM_COMP_TXBF, ODM_DBG_LOUD, ("%s, BFer Entry= %d\n", __FUNCTION__, *Idx));

		return pEntry;
	}
	else
		return NULL;
}


BOOLEAN
Beamforming_RemoveEntry(
	struct rtl8192cd_priv *priv,
	IN	pu1Byte		RA,
	OUT	pu1Byte		Idx
	)
{
	PRT_BEAMFORMER_ENTRY	pBFerEntry = Beamforming_GetBFerEntryByAddr(priv, RA, Idx);
    PRT_BEAMFORMING_ENTRY   pBFeeEntry = Beamforming_GetBFeeEntryByAddr(priv, RA, Idx);
    PRT_BEAMFORMING_INFO    pBeamInfo = &(priv->pshare->BeamformingInfo);

    // CurDelBFerBFeeEntrySel had been decided by the function who calls Beamforming_DeInitEntry
    DEL_ENTRY_TYPE_SEL  CurDelBFerBFeeEntrySel=pBeamInfo->CurDelBFerBFeeEntrySel;
    
	BOOLEAN ret = FALSE, bHwStateAddInit = FALSE;

    if (pBFerEntry != NULL && ((CurDelBFerBFeeEntrySel == BFerEntry)||(CurDelBFerBFeeEntrySel == BFerBFeeEntry)) )
    {
        pBFerEntry->bUsed = FALSE;
        pBFerEntry->BeamformEntryCap = BEAMFORMING_CAP_NONE;

        if(CurDelBFerBFeeEntrySel == BFerEntry)
        {
            if(*Idx==0)
		        memcpy(pBeamInfo->DelEntryListByMACAddr.BFerEntry_Idx0, RA, 6);                
            else
		        memcpy(pBeamInfo->DelEntryListByMACAddr.BFerEntry_Idx1, RA, 6);
		}
	
		if(GET_CHIP_VER(priv) == VERSION_8822B){		
			bHwStateAddInit = (pBFerEntry->HwState == BEAMFORM_ENTRY_HW_STATE_ADD_INIT)? TRUE:FALSE;
			pBFerEntry->HwState = BEAMFORM_ENTRY_HW_STATE_DELETE_INIT;
		}
		ret = TRUE;
	}

    if (pBFeeEntry != NULL && ((CurDelBFerBFeeEntrySel == BFeeEntry)||(CurDelBFerBFeeEntrySel == BFerBFeeEntry))) 
    {
        pBFeeEntry->bUsed = FALSE;
		pBFeeEntry->pSTA->inTXBFEntry = 0;
        pBFeeEntry->BeamformEntryCap = BEAMFORMING_CAP_NONE;
		
        if(CurDelBFerBFeeEntrySel == BFeeEntry)
        {
            if(*Idx==0)
		        memcpy(&pBeamInfo->DelEntryListByMACAddr.BFeeEntry_Idx0[0], RA, 6);            
            else
		        memcpy(&pBeamInfo->DelEntryListByMACAddr.BFeeEntry_Idx1[0], RA, 6);
		}

		if(GET_CHIP_VER(priv) == VERSION_8822B){
#if (MU_BEAMFORMING_SUPPORT == 1)	
			pBFeeEntry->gid_mgnt_retryCnt = 0;
#endif		
			bHwStateAddInit = (pBFeeEntry->HwState == BEAMFORM_ENTRY_HW_STATE_ADD_INIT)? TRUE:FALSE;
			pBFeeEntry->HwState = BEAMFORM_ENTRY_HW_STATE_DELETE_INIT;
			beamform_UpdateMinSoundingPeriod(priv, 0, TRUE);
		}
      
        ret = TRUE;    
    }
    
	return ret;
}

BOOLEAN
Beamforming_InitEntry(
	struct rtl8192cd_priv	*priv,
	struct stat_info		*pSTA,
	pu1Byte				BFerBFeeIdx	
	)
{

	PRT_BEAMFORMING_ENTRY	pBeamformEntry = NULL;
	PRT_BEAMFORMER_ENTRY	pBeamformerEntry = NULL;
	pu1Byte					RA; 
	u2Byte					AID, MacID;
	u1Byte					WirelessMode;
	u1Byte					BW = HT_CHANNEL_WIDTH_20;
	BEAMFORMING_CAP		BeamformCap = BEAMFORMING_CAP_NONE;	
	u1Byte					BFerIdx = 0xF, BFeeIdx = 0xF;
	u2Byte					CompSteeringNumofBFer = 0, NumofSoundingDim = 0;

	//DbgPrint("%s => txbf = %d, txbfer = %d, txbfee = %d\n", __FUNCTION__, priv->pmib->dot11RFEntry.txbf, priv->pmib->dot11RFEntry.txbfer, priv->pmib->dot11RFEntry.txbfee);
	

	// The current setting does not support Beaforming
	if (priv->pmib->dot11RFEntry.txbf == 0)
		return FALSE;


	// IBSS, AP mode
	if (pSTA != NULL) {
		 if(OPMODE & WIFI_AP_STATE)
			AID = pSTA->aid;
		 else
		 	AID = pSTA->assocID;
		 
		RA  = pSTA->hwaddr;
		MacID = pSTA->aid;

		WirelessMode = pSTA->WirelessMode;
		BW = pSTA->tx_bw;
	} else	// Client mode
		return FALSE;

	if ( WirelessMode < WIRELESS_MODE_N_24G)
		return FALSE;
	else 
	{

// BIT 4 implies capable of sending NDPA (BFER),
// BIT 3 implies capable of receiving NDPA (BFEE),
		if (WirelessMode == WIRELESS_MODE_N_24G || WirelessMode == WIRELESS_MODE_N_5G)
		{
			if(pSTA->ht_cap_len && (cpu_to_le32(pSTA->ht_cap_buf.txbf_cap) & 0x8)&& (priv->pmib->dot11RFEntry.txbfer == 1))	//bfer
			{
				BeamformCap |=BEAMFORMER_CAP_HT_EXPLICIT;
				CompSteeringNumofBFer = (u1Byte)((cpu_to_le32(pSTA->ht_cap_buf.txbf_cap) & (BIT(23)|BIT(24)))>>23);
			}
			if (pSTA->ht_cap_len && (cpu_to_le32(pSTA->ht_cap_buf.txbf_cap) & 0x10)&& (priv->pmib->dot11RFEntry.txbfee == 1))	//bfee
			{
				BeamformCap |=BEAMFORMEE_CAP_HT_EXPLICIT;
				NumofSoundingDim = (u1Byte)((cpu_to_le32(pSTA->ht_cap_buf.txbf_cap) & (BIT(27)|BIT(28)))>>27);
			}
		}
#ifdef RTK_AC_SUPPORT			
		if(WirelessMode == WIRELESS_MODE_AC_5G || WirelessMode == WIRELESS_MODE_AC_24G)
		{
			if(pSTA->vht_cap_len && (cpu_to_le32(pSTA->vht_cap_buf.vht_cap_info) & BIT(SU_BFEE_S)) && (priv->pmib->dot11RFEntry.txbfer == 1))  // AC還沒依據bfer or bfee擋
			{
			BeamformCap |=BEAMFORMER_CAP_VHT_SU;
			CompSteeringNumofBFer = (u1Byte)((cpu_to_le32(pSTA->vht_cap_buf.vht_cap_info) & (BIT(MAX_ANT_SUPP_S)|BIT(MAX_ANT_SUPP_S+1)|BIT(MAX_ANT_SUPP_E)))>>MAX_ANT_SUPP_S);
			}
			if(pSTA->vht_cap_len && (cpu_to_le32(pSTA->vht_cap_buf.vht_cap_info) & BIT(SU_BFER_S)) && (priv->pmib->dot11RFEntry.txbfee == 1))
			{
			BeamformCap |=BEAMFORMEE_CAP_VHT_SU;
			NumofSoundingDim = (u1Byte)((cpu_to_le32(pSTA->vht_cap_buf.vht_cap_info) & (BIT(SOUNDING_DIMENSIONS_S)|BIT(SOUNDING_DIMENSIONS_S+1)|BIT(SOUNDING_DIMENSIONS_E)))>>SOUNDING_DIMENSIONS_S);
			}
			#if (MU_BEAMFORMING_SUPPORT == 1)
			if((GET_CHIP_VER(priv) == VERSION_8822B) && priv->pmib->dot11RFEntry.txbf_mu){
				if(pSTA->vht_cap_len && (cpu_to_le32(pSTA->vht_cap_buf.vht_cap_info) & BIT(MU_BFEE_S)) && (priv->pmib->dot11RFEntry.txbfer == 1) && get_sta_vht_mimo_mode(pSTA) == MIMO_1T1R)  { // AC還沒依據bfer or bfee擋
					BeamformCap |=BEAMFORM_CAP_VHT_MU_BFER;
					CompSteeringNumofBFer = (u1Byte)((cpu_to_le32(pSTA->vht_cap_buf.vht_cap_info) & (BIT(MAX_ANT_SUPP_S)|BIT(MAX_ANT_SUPP_S+1)|BIT(MAX_ANT_SUPP_E)))>>MAX_ANT_SUPP_S);
				}
				
				if(pSTA->vht_cap_len && (cpu_to_le32(pSTA->vht_cap_buf.vht_cap_info) & BIT(MU_BFER_S)) && (priv->pmib->dot11RFEntry.txbfee == 1)) {
					BeamformCap |=BEAMFORM_CAP_VHT_MU_BFEE;
					NumofSoundingDim = (u1Byte)((cpu_to_le32(pSTA->vht_cap_buf.vht_cap_info) & (BIT(SOUNDING_DIMENSIONS_S)|BIT(SOUNDING_DIMENSIONS_S+1)|BIT(SOUNDING_DIMENSIONS_E)))>>SOUNDING_DIMENSIONS_S);
				}
			}
			#endif
		}
#endif		
	}


	ODM_RT_TRACE(ODMPTR, PHYDM_COMP_TXBF, ODM_DBG_LOUD, ("%s, CompSteeringNumofBFer = %d, NumofSoundingDim = %d\n", __FUNCTION__, CompSteeringNumofBFer, NumofSoundingDim));
// bfme
	if((BeamformCap & BEAMFORMEE_CAP_HT_EXPLICIT) || (BeamformCap & BEAMFORMEE_CAP_VHT_SU))
	{
		pBeamformerEntry = Beamforming_GetBFerEntryByAddr(priv, RA, &BFerIdx);

		if(pBeamformerEntry == NULL)
		{
			pBeamformerEntry = Beamforming_AddBFerEntry(priv, RA, AID, BeamformCap, &BFerIdx, NumofSoundingDim);

			if(pBeamformerEntry == NULL) {
				ODM_RT_TRACE(ODMPTR, PHYDM_COMP_TXBF, ODM_DBG_LOUD, ("%s, Not enough BFer entry!\n", __FUNCTION__));
#if defined(UNIVERSAL_REPEATER)
				if(IS_VXD_INTERFACE(priv)) {
					unsigned int reuse_idx = priv->dev->name[4]-'0';

					if(reuse_idx < BEAMFORMEE_ENTRY_NUM) {
						if(!Beamforming_DeInitEntry(priv,priv->pshare->BeamformingInfo.BeamformerEntry[reuse_idx].MacAddr)) {
							printk("[%s]Force remove BeamformerEntry[%d] failed\n",priv->dev->name,reuse_idx);
						} else {
							Beamforming_Notify(priv);
							pBeamformerEntry = Beamforming_AddBFerEntry(priv, RA, AID, BeamformCap, &BFerIdx, NumofSoundingDim);
							DEBUG_INFO("[%s]Reuse BeamformerEntry[%d]\n",priv->dev->name,BFerIdx);
							if(!pBeamformerEntry)
								printk("[%s]Reuse BeamformerEntry[%d] failed\n",priv->dev->name,BFerIdx);
						}
					} else {
						printk("Invalid index(%d) for reuse BeamformerEntry, beamformer is inactive!\n",reuse_idx);
					}
				}
#endif
			}
#if defined(UNIVERSAL_REPEATER)
			else {
				if(IS_VXD_INTERFACE(priv))
					DEBUG_INFO("[%s]Occupy BeamformerEntry[%d]\n",priv->dev->name,BFerIdx);
			}
#endif
		}
	}

//bfer
	if((BeamformCap & BEAMFORMER_CAP_HT_EXPLICIT) || (BeamformCap & BEAMFORMER_CAP_VHT_SU) && (pSTA->IOTPeer != HT_IOT_PEER_INTEL))
	{
		pBeamformEntry = Beamforming_GetBFeeEntryByAddr(priv, RA, &BFeeIdx);
		
		if(pBeamformEntry == NULL)
		{
			pBeamformEntry = Beamforming_AddBFeeEntry(priv, pSTA, RA, AID, MacID, BW, BeamformCap, &BFeeIdx, CompSteeringNumofBFer);

			if(pBeamformEntry == NULL)
			{
#if defined(UNIVERSAL_REPEATER)
				if(IS_VXD_INTERFACE(priv)) {
					unsigned int reuse_idx = priv->dev->name[4]-'0';

					if(reuse_idx < BEAMFORMEE_ENTRY_NUM) {
						if(!Beamforming_DeInitEntry(priv,priv->pshare->BeamformingInfo.BeamformeeEntry[reuse_idx].MacAddr)) {
							printk("[%s]Force remove BeamformeeEntry[%d] failed\n",priv->dev->name,reuse_idx);
							return FALSE;
						} else {
							Beamforming_Notify(priv);
							pBeamformEntry = Beamforming_AddBFeeEntry(priv, pSTA, RA, AID, MacID, BW, BeamformCap, &BFeeIdx, CompSteeringNumofBFer);
							DEBUG_INFO("[%s]Reuse BeamformeeEntry[%d]\n",priv->dev->name,BFeeIdx);
							if(pBeamformEntry)
								pBeamformEntry->BeamformEntryState = BEAMFORMING_ENTRY_STATE_INITIALIZEING;
							else {
								printk("[%s]Reuse BeamformeeEntry[%d] failed\n",priv->dev->name,BFeeIdx);
								return FALSE;
							}
						}
					} else {
						printk("Invalid index(%d) for reuse BeamformerEntry, beamformee is inactive!\n",reuse_idx);
						return FALSE;
					}
				} else
#endif
				return FALSE;
			}
			else
				pBeamformEntry->BeamformEntryState = BEAMFORMING_ENTRY_STATE_INITIALIZEING;
		}	
		else
		{
#if defined(UNIVERSAL_REPEATER)
			if(IS_VXD_INTERFACE(priv))
				DEBUG_INFO("[%s]Occupy BeamformEntry[%d]\n",priv->dev->name,BFerIdx);
#endif
			// Entry has been created. If entry is initialing or progressing then errors occur.
			if(	pBeamformEntry->BeamformEntryState != BEAMFORMING_ENTRY_STATE_INITIALIZED && 
				pBeamformEntry->BeamformEntryState != BEAMFORMING_ENTRY_STATE_PROGRESSED)
			{
				ODM_RT_TRACE(ODMPTR, PHYDM_COMP_TXBF, ODM_DBG_LOUD, ("%s, Error State of Beamforming\n", __FUNCTION__));
				return FALSE;
			}	
			else
				pBeamformEntry->BeamformEntryState = BEAMFORMING_ENTRY_STATE_INITIALIZEING;
		}

		pBeamformEntry->BeamformEntryState = BEAMFORMING_ENTRY_STATE_INITIALIZED;
		Beamforming_AutoTest(priv, BFeeIdx, pBeamformEntry);
	}

	*BFerBFeeIdx = (BFerIdx<<4) | BFeeIdx;
	ODM_RT_TRACE(ODMPTR, PHYDM_COMP_TXBF, ODM_DBG_LOUD, ("%s,  BFerIdx=0x%x, BFeeIdx=0x%x, BFerBFeeIdx=0x%x \n", __FUNCTION__, BFerIdx, BFeeIdx, *BFerBFeeIdx));

	return TRUE;
}

BOOLEAN
Beamforming_DeInitEntry(
	struct rtl8192cd_priv *priv,
	pu1Byte			RA
	)
{
	u1Byte					Idx = 0;

	ODM_RT_TRACE(ODMPTR, PHYDM_COMP_TXBF, ODM_DBG_LOUD, ("[Beamforming]%s Start!!!\n", __FUNCTION__));
	
	if(Beamforming_RemoveEntry(priv, RA, &Idx) == TRUE)
	{
		Beamforming_SetBeamFormLeave(priv, Idx);

#ifdef CONFIG_RTL_8812_SUPPORT				// 8812 only??
#ifdef MBSSID
		if (GET_MIB(GET_ROOT(priv))->miscEntry.vap_enable)
			rtl8192cd_clear_mbssid(priv, Idx);
#endif
#endif
		
		return TRUE;
	}
	else
	{
		// For AP debug, because when STA disconnect AP, release_stainfo will be triggered many times
		return FALSE;
	}

}

VOID
BeamformingReset(
	struct rtl8192cd_priv *priv
	)
{
	u1Byte		Idx = 0;
	PRT_BEAMFORMING_INFO pBeamformingInfo = &(priv->pshare->BeamformingInfo);

	for(Idx = 0; Idx < BEAMFORMEE_ENTRY_NUM; Idx++)
	{
		if(pBeamformingInfo->BeamformeeEntry[Idx].bUsed == TRUE)
		{
			ODM_RT_TRACE(ODMPTR, PHYDM_COMP_TXBF, ODM_DBG_LOUD, ("%s, Reset entry idx=%d\n", __FUNCTION__, Idx));
			
			pBeamformingInfo->BeamformeeEntry[Idx].bUsed = FALSE;
			pBeamformingInfo->BeamformeeEntry[Idx].BeamformEntryCap = BEAMFORMING_CAP_NONE;
			pBeamformingInfo->BeamformeeEntry[Idx].bBeamformingInProgress = FALSE;

			Beamforming_SetBeamFormLeave(priv, Idx);		
		}
	}

	for(Idx = 0; Idx < BEAMFORMER_ENTRY_NUM; Idx++)
	{
		pBeamformingInfo->BeamformerEntry[Idx].bUsed = FALSE;
	}

}



#define FillOctetString(_os,_octet,_len)		\
	(_os).Octet=(pu1Byte)(_octet);			\
	(_os).Length=(_len);

VOID
ConstructHTNDPAPacket(
	struct rtl8192cd_priv *priv,
	pu1Byte				RA,
	pu1Byte				Buffer,
	pu4Byte				pLength,
	u1Byte			 	BW
	)
{
	u2Byte					Duration= 0;
	OCTET_STRING			pNDPAFrame, ActionContent;
	u1Byte					ActionHdr[4] = {ACT_CAT_VENDOR, 0x00, 0xe0, 0x4c};
	int aSifsTime = ((priv->pmib->dot11BssType.net_work_type & WIRELESS_11N) && (priv->pshare->ht_sta_num)) ? 0x10 : 10;


	SET_80211_HDR_FRAME_CONTROL(Buffer,0);
	SET_80211_HDR_ORDER(Buffer, 1);
	SET_80211_HDR_TYPE_AND_SUBTYPE(Buffer,Type_Action_No_Ack);

	memcpy((void *)GetAddr1Ptr(Buffer), RA, MACADDRLEN);
	memcpy((void *)GetAddr2Ptr(Buffer), GET_MY_HWADDR, MACADDRLEN);
	memcpy((void *)GetAddr3Ptr(Buffer), BSSID, MACADDRLEN);

	Duration = 2*aSifsTime + 40;
	
	if(BW== HT_CHANNEL_WIDTH_20_40)
		Duration+= 87;
	else	
		Duration+= 180;

	SET_80211_HDR_DURATION(Buffer, Duration);

	//HT control field
	SET_HT_CTRL_CSI_STEERING(Buffer+sMacHdrLng, 3);
	SET_HT_CTRL_NDP_ANNOUNCEMENT(Buffer+sMacHdrLng, 1);
	
	FillOctetString(pNDPAFrame, Buffer, sMacHdrLng+sHTCLng);

	FillOctetString(ActionContent, ActionHdr, 4);
	PacketAppendData(&pNDPAFrame, ActionContent);	

	*pLength = 32;
}


BOOLEAN
SendHTNDPAPacket(
	struct rtl8192cd_priv *priv,
		pu1Byte				RA,
		u1Byte 				BW,
		u1Byte		NDPTxRate
	)
{
	BOOLEAN					ret = TRUE;
	unsigned char *pbuf 		= get_wlanllchdr_from_poll(priv);
	u4Byte PacketLength;
	DECLARE_TXINSN(txinsn);	

	if(pbuf) 
	{
		memset(pbuf, 0, sizeof(struct wlan_hdr));
		ConstructHTNDPAPacket(
				priv, 
				RA,
				pbuf,
				&PacketLength,
				BW
				);
		
		txinsn.q_num = MGNT_QUEUE;	
		txinsn.fr_type = _PRE_ALLOCLLCHDR_;				

		txinsn.phdr = pbuf;
		txinsn.hdr_len = PacketLength;
		txinsn.fr_len = 0;
		txinsn.tx_rate = NDPTxRate; //_MCS8_RATE_;, According to Nr
		txinsn.fixed_rate = 1;	
		txinsn.ndpa = 1;

		if (rtl8192cd_wlantx(priv, &txinsn) == CONGESTED) {		
			netif_stop_queue(priv->dev);		
			priv->ext_stats.tx_drops++; 	
//			panic_printk("TX DROP: Congested!\n");
			if (txinsn.phdr)
				release_wlanllchdr_to_poll(priv, txinsn.phdr); 			
			if (txinsn.pframe)
				release_mgtbuf_to_poll(priv, txinsn.pframe);			
			return 0;	
		}
	}
	else
		ret = FALSE;

	return ret;
}




VOID
ConstructVHTNDPAPacket(
	struct rtl8192cd_priv *priv,
	pu1Byte			RA,
	u2Byte			AID,
	pu1Byte			Buffer,
	pu4Byte			pLength,
	u1Byte 			BW
	)
{
	u2Byte					Duration= 0;
	u1Byte					Sequence = 0;
	pu1Byte					pNDPAFrame = Buffer;
	u2Byte					tmp16;
	
	RT_NDPA_STA_INFO		STAInfo;
	int aSifsTime = ((priv->pmib->dot11BssType.net_work_type & WIRELESS_11N) && (priv->pshare->ht_sta_num)) ? 0x10 : 10;

	// Frame control.
	SET_80211_HDR_FRAME_CONTROL(pNDPAFrame, 0);
	SET_80211_HDR_TYPE_AND_SUBTYPE(pNDPAFrame, Type_NDPA);

	memcpy((void *)GetAddr1Ptr(pNDPAFrame), RA, MACADDRLEN);
	memcpy((void *)GetAddr2Ptr(pNDPAFrame), GET_MY_HWADDR, MACADDRLEN);

	Duration = 2*aSifsTime + 44;
	
	if(BW == HT_CHANNEL_WIDTH_80)
		Duration += 40;
	else if(BW == HT_CHANNEL_WIDTH_20_40)
		Duration+= 87;
	else	
		Duration+= 180;

	SetDuration(pNDPAFrame, Duration);
	Sequence = GET_HW(priv)->sounding_seq<<2;
	GET_HW(priv)->sounding_seq =  (GET_HW(priv)->sounding_seq+1) & 0xfff;
	 
	memcpy(pNDPAFrame+16, &Sequence,1);

	if (OPMODE & WIFI_ADHOC_STATE)
		AID = 0;

	STAInfo.AID = AID;

	STAInfo.FeedbackType = 0;
	STAInfo.NcIndex = 0;
	
	memcpy(&tmp16, (pu1Byte)&STAInfo, 2);
	tmp16 = cpu_to_le16(tmp16);

	memcpy(pNDPAFrame+17, &tmp16, 2);

	*pLength = 19;
}


BOOLEAN
SendVHTNDPAPacket(
	struct rtl8192cd_priv *priv,
	IN	pu1Byte			RA,
	IN	u2Byte			AID,
	u1Byte 				BW,
	u1Byte		NDPTxRate
	)
{
	BOOLEAN					ret = TRUE;
	u4Byte 					PacketLength;
	unsigned char *pbuf 	= get_wlanllchdr_from_poll(priv);
	DECLARE_TXINSN(txinsn);	

	if(pbuf)
	{
		memset(pbuf, 0, sizeof(struct wlan_hdr));

		ConstructVHTNDPAPacket	(
			priv, 
			RA,
			AID,
			pbuf,
			&PacketLength,
			BW
			);

		txinsn.q_num = MANAGE_QUE_NUM;
		txinsn.fr_type = _PRE_ALLOCLLCHDR_;		
		txinsn.phdr = pbuf;
		txinsn.hdr_len = PacketLength;
		txinsn.fr_len = 0;
		txinsn.fixed_rate = 1;	
		txinsn.tx_rate = NDPTxRate;	// According to Nr	
		txinsn.ndpa = 1;

		if (rtl8192cd_wlantx(priv, &txinsn) == CONGESTED) {		
			netif_stop_queue(priv->dev);		
			priv->ext_stats.tx_drops++; 	
//			panic_printk("TX DROP: Congested!\n");
			if (txinsn.phdr)
				release_wlanllchdr_to_poll(priv, txinsn.phdr); 			
			if (txinsn.pframe)
				release_mgtbuf_to_poll(priv, txinsn.pframe);
			return 0;	
		}
	}
	else
		ret = FALSE;

	return ret;
}



u1Byte
beamforming_SoundingIdx(
	PRT_BEAMFORMING_INFO 			pBeamInfo
	)
{
	u1Byte							Idx = 0;
	PRT_BEAMFORMING_PERIOD_INFO	pBeamPeriodInfo = &(pBeamInfo->BeamformingPeriodInfo);

	if(	pBeamPeriodInfo->Mode == SOUNDING_SW_VHT_TIMER ||pBeamPeriodInfo->Mode == SOUNDING_SW_HT_TIMER ||
		pBeamPeriodInfo->Mode == SOUNDING_HW_VHT_TIMER ||pBeamPeriodInfo->Mode == SOUNDING_HW_HT_TIMER)
		Idx = pBeamPeriodInfo->Idx;
	else
		Idx = 0;

	return Idx;
}

BEAMFORMING_NOTIFY_STATE
beamfomring_bSounding_8822B(
	struct rtl8192cd_priv	*priv,
	PRT_BEAMFORMING_INFO 	pBeamInfo,
	pu1Byte					Idx
	)
{
	BEAMFORMING_NOTIFY_STATE		bSounding = BEAMFORMING_NOTIFY_NONE;
	//RT_BEAMFORMING_ENTRY			Entry = pBeamInfo->BeamformeeEntry[*Idx];
	RT_BEAMFORMING_PERIOD_INFO		BeamPeriodInfo = pBeamInfo->BeamformingPeriodInfo;

	if(BeamPeriodInfo.Mode == SOUNDING_STOP_All_TIMER)
		bSounding = BEAMFORMING_NOTIFY_RESET;
//	else if(BeamPeriodInfo.Mode == SOUNDING_STOP_OID_TIMER && Entry.bTxBF == FALSE)
//		bSounding = BEAMFORMING_NOTIFY_RESET;
	else
	{
		u1Byte i;

		for(i=0;i<BEAMFORMEE_ENTRY_NUM;i++)
		{
			//panic_printk("[David]@%s: BFee Entry %d bUsed=%d, bSound=%d \n", __FUNCTION__, i, pBeamInfo->BeamformeeEntry[i].bUsed, pBeamInfo->BeamformeeEntry[i].bSound);
#if (MU_BEAMFORMING_SUPPORT == 1)			
			ODM_RT_TRACE(ODMPTR, PHYDM_COMP_TXBF, ODM_DBG_LOUD, ("%s: BFee Entry %d bUsed=%d, bSound=%d, is_mu_sta=%d \n", __FUNCTION__, i, pBeamInfo->BeamformeeEntry[i].bUsed, pBeamInfo->BeamformeeEntry[i].bSound, pBeamInfo->BeamformeeEntry[i].is_mu_sta));
#endif
			if(pBeamInfo->BeamformeeEntry[i].bUsed && (!pBeamInfo->BeamformeeEntry[i].bSound))
			{
				*Idx = i;
#if (MU_BEAMFORMING_SUPPORT == 1)
				if (pBeamInfo->BeamformeeEntry[i].is_mu_sta)
					bSounding = BEAMFORMEE_NOTIFY_ADD_MU;
				else
#endif					
					bSounding = BEAMFORMEE_NOTIFY_ADD_SU;

			}

			if((!pBeamInfo->BeamformeeEntry[i].bUsed) && pBeamInfo->BeamformeeEntry[i].bSound)
			{
				*Idx = i;
#if (MU_BEAMFORMING_SUPPORT == 1)
				if (pBeamInfo->BeamformeeEntry[i].is_mu_sta)
					bSounding = BEAMFORMEE_NOTIFY_DELETE_MU;
				else
#endif					
					bSounding = BEAMFORMEE_NOTIFY_DELETE_SU;

			}
		}
	}

	return bSounding;
}


BEAMFORMING_NOTIFY_STATE
beamfomring_bSounding(
	struct rtl8192cd_priv	*priv,
	PRT_BEAMFORMING_INFO 	pBeamInfo,
	pu1Byte					Idx
	)
{
	BEAMFORMING_NOTIFY_STATE		bSounding = BEAMFORMING_NOTIFY_NONE;
	//RT_BEAMFORMING_ENTRY			Entry = pBeamInfo->BeamformeeEntry[*Idx];
	RT_BEAMFORMING_PERIOD_INFO		BeamPeriodInfo = pBeamInfo->BeamformingPeriodInfo;

	if(BeamPeriodInfo.Mode == SOUNDING_STOP_All_TIMER)
		bSounding = BEAMFORMING_NOTIFY_RESET;
//	else if(BeamPeriodInfo.Mode == SOUNDING_STOP_OID_TIMER && Entry.bTxBF == FALSE)
//		bSounding = BEAMFORMING_NOTIFY_RESET;
	else
	{
		u1Byte i;

		for(i=0;i<BEAMFORMEE_ENTRY_NUM;i++)
		{
			//panic_printk("[David]@%s: BFee Entry %d bUsed=%d, bSound=%d \n", __FUNCTION__, i, pBeamInfo->BeamformeeEntry[i].bUsed, pBeamInfo->BeamformeeEntry[i].bSound);
			if(pBeamInfo->BeamformeeEntry[i].bUsed && (!pBeamInfo->BeamformeeEntry[i].bSound))
			{
				*Idx = i;
				bSounding = BEAMFORMING_NOTIFY_ADD;
			}

			if((!pBeamInfo->BeamformeeEntry[i].bUsed) && pBeamInfo->BeamformeeEntry[i].bSound)
			{
				*Idx = i;
				bSounding = BEAMFORMING_NOTIFY_DELETE;
			}
		}
	}

	return bSounding;
}


SOUNDING_MODE
beamforming_SoundingMode(
	PRT_BEAMFORMING_INFO 	pBeamInfo,
	u1Byte					Idx
	)
{
	RT_BEAMFORMING_PERIOD_INFO		BeamPeriodInfo = pBeamInfo->BeamformingPeriodInfo;	
	SOUNDING_MODE					Mode = BeamPeriodInfo.Mode;
	RT_BEAMFORMING_ENTRY			Entry = pBeamInfo->BeamformeeEntry[Idx];

	if(	BeamPeriodInfo.Mode == SOUNDING_SW_VHT_TIMER || BeamPeriodInfo.Mode == SOUNDING_SW_HT_TIMER ||
		BeamPeriodInfo.Mode == SOUNDING_HW_VHT_TIMER || BeamPeriodInfo.Mode == SOUNDING_HW_HT_TIMER )
		Mode = BeamPeriodInfo.Mode;
	else	if(Entry.BeamformEntryCap & BEAMFORMER_CAP_VHT_SU)
		Mode = SOUNDING_AUTO_VHT_TIMER;
	else	if(Entry.BeamformEntryCap & BEAMFORMER_CAP_HT_EXPLICIT)
		Mode = SOUNDING_AUTO_HT_TIMER;

	return Mode;
}


u2Byte
beamforming_SoundingTime(
	PRT_BEAMFORMING_INFO 	pBeamInfo,
	SOUNDING_MODE			Mode
	)
{
	u2Byte							SoundingTime = 0xffff;
	RT_BEAMFORMING_PERIOD_INFO		BeamPeriodInfo = pBeamInfo->BeamformingPeriodInfo;	
	
	if(BeamPeriodInfo.BeamPeriod <= 0)
		BeamPeriodInfo.BeamPeriod = 20;
	
	if(Mode == SOUNDING_HW_HT_TIMER || Mode == SOUNDING_HW_VHT_TIMER)
		SoundingTime = BeamPeriodInfo.BeamPeriod * 32;
	else	if(Mode == SOUNDING_SW_HT_TIMER || Mode == SOUNDING_SW_VHT_TIMER)
		SoundingTime = BeamPeriodInfo.BeamPeriod ;
	else
		SoundingTime = 20*32;

	return SoundingTime;
}


u1Byte
beamforming_SoundingBW(
	PRT_BEAMFORMING_INFO 	pBeamInfo,
	SOUNDING_MODE			Mode,
	u1Byte					Idx
	)
{
	u1Byte							SoundingBW = HT_CHANNEL_WIDTH_20;
	RT_BEAMFORMING_ENTRY			Entry = pBeamInfo->BeamformeeEntry[Idx];
	RT_BEAMFORMING_PERIOD_INFO		BeamPeriodInfo = pBeamInfo->BeamformingPeriodInfo;

	if(Mode == SOUNDING_HW_HT_TIMER || Mode == SOUNDING_HW_VHT_TIMER)
		SoundingBW = BeamPeriodInfo.BW;
	else	if(Mode == SOUNDING_SW_HT_TIMER || Mode == SOUNDING_SW_VHT_TIMER)
		SoundingBW = BeamPeriodInfo.BW;
	else 
		SoundingBW = Entry.BW;

	return SoundingBW;
}


VOID
beamforming_StartPeriod(
	struct rtl8192cd_priv *priv,
	u1Byte				Idx
	)
{

	PRT_BEAMFORMING_INFO 		pBeamInfo = &(priv->pshare->BeamformingInfo);
	PRT_BEAMFORMING_TIMER_INFO	pBeamTimerInfo = &(pBeamInfo->BeamformingTimerInfo[Idx]);	

	ODM_RT_TRACE(ODMPTR, PHYDM_COMP_TXBF, ODM_DBG_LOUD, ("%s: Mode=%d, BeamPeriod=%d\n", __func__, pBeamTimerInfo->Mode, pBeamTimerInfo->BeamPeriod));
//	pBeamTimerInfo->Idx = Idx;
	pBeamTimerInfo->Mode = beamforming_SoundingMode(pBeamInfo, Idx);
	pBeamTimerInfo->BW = beamforming_SoundingBW(pBeamInfo, pBeamTimerInfo->Mode, Idx);
	pBeamTimerInfo->BeamPeriod = beamforming_SoundingTime(pBeamInfo, pBeamTimerInfo->Mode);

	if(pBeamTimerInfo->Mode == SOUNDING_SW_VHT_TIMER || pBeamTimerInfo->Mode == SOUNDING_SW_HT_TIMER) 
	{
		ODM_SetTimer(ODMPTR, &pBeamInfo->BeamformingTimer, pBeamTimerInfo->BeamPeriod);
	} 
	else
	{
		Beamforming_SetHWTimer(priv, pBeamTimerInfo->BeamPeriod);
	}	

//	panic_printk ("%s Idx %d Mode %d BW %d Period %d\n", __FUNCTION__, 
//			Idx, pBeamTimerInfo->Mode, pBeamTimerInfo->BW, pBeamTimerInfo->BeamPeriod);
}


VOID
beamforming_EndPeriod_SW(
		struct rtl8192cd_priv *priv,
		u1Byte		Idx
	)
{
	PRT_BEAMFORMING_INFO 		pBeamInfo = &(priv->pshare->BeamformingInfo);
	PRT_BEAMFORMING_TIMER_INFO	pBeamTimerInfo = &(pBeamInfo->BeamformingTimerInfo[Idx]);

	if(pBeamTimerInfo->Mode == SOUNDING_SW_VHT_TIMER || pBeamTimerInfo->Mode == SOUNDING_SW_HT_TIMER) 
	{
		
		ODM_CancelTimer(ODMPTR, &pBeamInfo->BeamformingTimer);

		if(GET_CHIP_VER(priv) == VERSION_8822B){
			ODM_CancelTimer(ODMPTR, &pBeamInfo->BFSoundingTimeoutTimer);
			pBeamInfo->SoundingInfoV2.State = SOUNDING_STATE_NONE;
		}
	}
	else
	{
		Beamforming_StopHWTimer(priv);
	}

}

VOID
beamforming_EndPeriod_FW(
		struct rtl8192cd_priv 	*priv,
		u1Byte				Idx
	)
{
	return;
}

VOID
beamforming_ClearEntry_SW(
		struct rtl8192cd_priv 	*priv,
		BOOLEAN				IsDelete,
		u1Byte				DeleteIdx
	)
{
	u1Byte						Idx = 0;
	PRT_BEAMFORMING_ENTRY		pBeamformEntry;
	PRT_BEAMFORMING_INFO 		pBeamInfo = &(priv->pshare->BeamformingInfo);

	if(IsDelete)
	{
		if(DeleteIdx<BEAMFORMEE_ENTRY_NUM)
		{
			pBeamformEntry = pBeamInfo->BeamformeeEntry + DeleteIdx;

			if(!((!pBeamformEntry->bUsed) && pBeamformEntry->bSound))
			{
				ODM_RT_TRACE(ODMPTR, PHYDM_COMP_TXBF, ODM_DBG_LOUD, ("%s, SW DeleteIdx is wrong!\n", __FUNCTION__)); 
				return;
			}
		}

		if(pBeamformEntry->BeamformEntryState == BEAMFORMING_ENTRY_STATE_PROGRESSING)
		{
			pBeamformEntry->bBeamformingInProgress = FALSE;
			pBeamformEntry->BeamformEntryState = BEAMFORMING_ENTRY_STATE_UNINITIALIZE;
		}
		else if(pBeamformEntry->BeamformEntryState == BEAMFORMING_ENTRY_STATE_PROGRESSED)
		{
			pBeamformEntry->BeamformEntryState  = BEAMFORMING_ENTRY_STATE_UNINITIALIZE;
			Beamforming_SetBeamFormStatus(priv, DeleteIdx);
		}	
		pBeamformEntry->bSound=FALSE;
		
	}
	else
	{
		for(Idx = 0; Idx < BEAMFORMEE_ENTRY_NUM; Idx++)
		{
			pBeamformEntry = pBeamInfo->BeamformeeEntry+Idx;

			if(pBeamformEntry->bSound)
			{
				ODM_RT_TRACE(ODMPTR, PHYDM_COMP_TXBF, ODM_DBG_LOUD, ("%s, SW Reset entry %d\n", __FUNCTION__, Idx)); 

				/*	
				*	If End procedure is 
				*	1. Between (Send NDPA, C2H packet return), reset state to initialized.
				*	After C2H packet return , status bit will be set to zero. 
				*r
				*	2. After C2H packet, then reset state to initialized and clear status bit.
				*/ 
				if(pBeamformEntry->BeamformEntryState == BEAMFORMING_ENTRY_STATE_PROGRESSING)
					{
					Beamforming_End(priv, 0);
					}	
				else if(pBeamformEntry->BeamformEntryState == BEAMFORMING_ENTRY_STATE_PROGRESSED)
				{
					pBeamformEntry->BeamformEntryState  = BEAMFORMING_ENTRY_STATE_INITIALIZED;
					Beamforming_SetBeamFormStatus(priv, Idx);
				}	
				
				pBeamformEntry->bSound=FALSE;
			}
		}			
	}

}

VOID
beamforming_ClearEntry_FW(
		struct rtl8192cd_priv 	*priv,
		BOOLEAN				IsDelete,
		u1Byte				DeleteIdx
	)
{
	return;
}

struct rtl8192cd_priv* 
getBeamEntryDev(struct rtl8192cd_priv *priv, PRT_BEAMFORMING_ENTRY pEntry)
{
	struct stat_info *pstat;
	struct rtl8192cd_priv *vxd_priv;	
	int j;

	pstat = get_stainfo(priv, pEntry->MacAddr);
	if(pstat)
		return priv;
	
#ifdef MBSSID
	  if ((OPMODE & WIFI_AP_STATE) && priv->pmib->miscEntry.vap_enable) 
	  {
		for (j=0; j<RTL8192CD_NUM_VWLAN; j++) 
		{
			if ((priv->pvap_priv[j]->assoc_num > 0) && IS_DRV_OPEN(priv->pvap_priv[j]))
			{
				pstat = get_stainfo(priv->pvap_priv[j], pEntry->MacAddr);
				if(pstat)
					return priv->pvap_priv[j];

			}
		}
	}
#endif			
#ifdef UNIVERSAL_REPEATER
	vxd_priv = GET_VXD_PRIV(priv);
	priv = vxd_priv;
	if((OPMODE & WIFI_STATION_STATE) && (vxd_priv->assoc_num > 0) && IS_DRV_OPEN(vxd_priv)	) 
	{ 	
		pstat = get_stainfo(vxd_priv, pEntry->MacAddr);
		if(pstat)
			return vxd_priv;
	}
#endif
	return NULL;

}

BOOLEAN
beamforming_by_rate_souding_decision(
	struct rtl8192cd_priv *priv,
	PRT_BEAMFORMING_ENTRY	pEntry
	)
{
	BOOLEAN	is_sounding = TRUE;
	u1Byte	ntx_num = 0;
	u1Byte	lower_rate_upperbound = 0;
	
	ntx_num = Beamforming_GetTxSSNum(priv);

	lower_rate_upperbound = pEntry->Sounding_En ? _MCS1_RATE_ : _MCS2_RATE_;
	if (priv->pshare->rf_ft_var.Nsnding &&
		(not_snding_rate_interval(pEntry, HT_RATE_ID + ntx_num*8 + 7, HT_RATE_ID + ntx_num*8) ||
		not_snding_rate_interval(pEntry, lower_rate_upperbound, _MCS0_RATE_)))	{
		is_sounding = FALSE;
	}
#ifdef RTK_AC_SUPPORT
	lower_rate_upperbound = pEntry->Sounding_En ? _NSS1_MCS1_RATE_ : _NSS1_MCS2_RATE_;
	if (priv->pshare->rf_ft_var.Nsnding &&
		(not_snding_rate_interval(pEntry, VHT_RATE_ID + ntx_num*10 + 9, VHT_RATE_ID + ntx_num*10) ||
		not_snding_rate_interval(pEntry, lower_rate_upperbound, _NSS1_MCS0_RATE_))) {
		is_sounding = FALSE;
	}
#endif

	if (is_sounding)
		pEntry->Sounding_En = TRUE;
	else {
		ODM_RT_TRACE(ODMPTR, PHYDM_COMP_TXBF, ODM_DBG_LOUD, ("%s, Not sounding, pEntry->Sounding_En = %d\n", __FUNCTION__, pEntry->Sounding_En));
		pEntry->Sounding_En = FALSE;
	}

	return is_sounding;

}


BOOLEAN
BeamformingStart_V2(
	struct rtl8192cd_priv *priv,
	u1Byte			Idx,
	u1Byte			Mode, 
	u1Byte			BW
	)
{
	pu1Byte					RA = NULL;
	BOOLEAN					ret = TRUE;
	PRT_BEAMFORMING_INFO 	pBeamformingInfo = &(priv->pshare->BeamformingInfo);
	PRT_BEAMFORMING_ENTRY	pEntry = &(pBeamformingInfo->BeamformeeEntry[Idx]);
	u1Byte					NDPTxRate;

	priv = getBeamEntryDev(priv, pEntry);
	if (!priv)
		return FALSE;
 
	if (pEntry->bUsed == FALSE) {
		pEntry->bBeamformingInProgress = FALSE;
		ODM_RT_TRACE(ODMPTR, PHYDM_COMP_TXBF, ODM_DBG_LOUD, ("%s, pEntry->bUsed = FALSE\n", __FUNCTION__));
		return FALSE;
	} else {
		pEntry->bBeamformingInProgress = TRUE;	
		pEntry->BeamformEntryState = BEAMFORMING_ENTRY_STATE_PROGRESSING; 
		pEntry->bSound = TRUE;

		RA = pEntry->MacAddr;

		{
			int i;
			struct stat_info *pstat = get_stainfo(GET_ROOT(priv), RA);	
#ifdef UNIVERSAL_REPEATER
			if ((pstat == NULL) && IS_DRV_OPEN(GET_VXD_PRIV(GET_ROOT(priv)))) {
				pstat = get_stainfo(GET_VXD_PRIV(priv), RA);
			}
#endif
#ifdef MBSSID
			if (pstat == NULL)	{
				if (GET_ROOT(priv)->pmib->miscEntry.vap_enable) {
					for (i=0; i<RTL8192CD_NUM_VWLAN; i++) {
						if (IS_DRV_OPEN(GET_ROOT(priv)->pvap_priv[i]))
							pstat = get_stainfo(GET_ROOT(priv)->pvap_priv[i], RA);
					}
				}
			}
#endif
			if (pstat && (pstat->state & WIFI_SLEEP_STATE)) {
				ODM_RT_TRACE(ODMPTR, PHYDM_COMP_TXBF, ODM_DBG_LOUD, ("%s, SLEEP mode, no NDPA\n", __FUNCTION__));
				return FALSE;
			}
		}

	}

	pEntry->BW = BW;
	pBeamformingInfo->BeamformeeCurIdx = Idx;
	
	Beamforming_SetBeamFormStatus(priv, (pBeamformingInfo->BeamformeeCurIdx));
	Beamforming_NDPARate(priv, Mode, BW, 0);	// soundingpreiod only for debug, use 0 for all case

//  debug
	if (!priv->pshare->rf_ft_var.soundingEnable) {
		pEntry->Sounding_En = 0;
		ODM_RT_TRACE(ODMPTR, PHYDM_COMP_TXBF, ODM_DBG_LOUD, ("%s, %d\n", __FUNCTION__, __LINE__));
		return FALSE;
	}
#ifdef CONFIG_WLAN_HAL_8192EE
	if ((OPMODE & WIFI_AP_STATE) && (priv->pshare->soundingLock)) {
		pEntry->Sounding_En = 0;
		ODM_RT_TRACE(ODMPTR, PHYDM_COMP_TXBF, ODM_DBG_LOUD, ("%s, %d\n", __FUNCTION__, __LINE__));
		return FALSE;
	}
#endif
//	
	if (Mode == SOUNDING_SW_HT_TIMER || Mode == SOUNDING_HW_HT_TIMER || Mode == SOUNDING_AUTO_HT_TIMER) {
		NDPTxRate = Beamforming_GetHTNDPTxRate(priv, pEntry->CompSteeringNumofBFer);
		if (beamforming_by_rate_souding_decision(priv, pEntry)) {
			ret = SendHTNDPAPacket(priv,RA, BW, NDPTxRate);
			if (ret)
				pEntry->Sounding_En = 1;
			else
				pEntry->Sounding_En = 0;
		}

		if(pEntry->Sounding_En == 0)
			Beamforming_End(priv,0);
		
		ODM_RT_TRACE(ODMPTR, PHYDM_COMP_TXBF, ODM_DBG_LOUD, ("%s, HT NDP Rate = %d\n", __FUNCTION__, NDPTxRate)); 
	} else {
		NDPTxRate = Beamforming_GetVHTNDPTxRate(priv, pEntry->CompSteeringNumofBFer);
		
		if (beamforming_by_rate_souding_decision(priv, pEntry)) {
			ret = SendVHTNDPAPacket(priv,RA, pEntry->AID, BW, NDPTxRate);
			if (ret)
				pEntry->Sounding_En = 1;
			else
				pEntry->Sounding_En = 0;
		}

		if(pEntry->Sounding_En == 0)
			Beamforming_End(priv,0);
		ODM_RT_TRACE(ODMPTR, PHYDM_COMP_TXBF, ODM_DBG_LOUD, ("%s, VHT NDP Rate = %d\n", __FUNCTION__, NDPTxRate));  
	}

	pEntry->bBeamformingInProgress = FALSE;

	if(ret == FALSE) {
		ODM_RT_TRACE(ODMPTR, PHYDM_COMP_TXBF, ODM_DBG_LOUD, ("%s, failure sending NDPA for addr =\n", __FUNCTION__));  
		pEntry->bBeamformingInProgress = TRUE;
		return FALSE;
	}

	return TRUE;
}

VOID
Beamforming_Notify_8822B(
	struct rtl8192cd_priv *priv
	)
{

		u1Byte						Idx=BEAMFORMEE_ENTRY_NUM;
		BEAMFORMING_NOTIFY_STATE	bSounding = BEAMFORMING_NOTIFY_NONE;
		PRT_BEAMFORMING_INFO		pBeamInfo = &(priv->pshare->BeamformingInfo);
		//PRT_SOUNDING_INFO 		pSoundInfo = &(pBeamInfo->SoundingInfo);
		PRT_BEAMFORMING_TIMER_INFO	pBeamTimerInfo = NULL;	
	
		ODM_RT_TRACE(ODMPTR, PHYDM_COMP_TXBF, ODM_DBG_LOUD, ("%s Start!\n", __func__));
	
#if 1 //eric-mu
		Idx = pBeamInfo->BeamformeeCurIdx;
		bSounding = beamfomring_bSounding_8822B(priv, pBeamInfo, &Idx);
#endif
	
		if(Idx<BEAMFORMEE_ENTRY_NUM)
			pBeamTimerInfo = &(pBeamInfo->BeamformingTimerInfo[Idx]);	
		else if(bSounding == BEAMFORMING_NOTIFY_RESET)
			pBeamTimerInfo = &(pBeamInfo->BeamformingTimerInfo[0]); 
		else
			panic_printk("[%s][%d]\n", __FUNCTION__, __LINE__); //eric-mu
	
#if 1 //eric-mu
		if(!pBeamTimerInfo){
			panic_printk("ERROR [%s][%d] pBeamTimerInfo = NULL !!\n", __FUNCTION__, __LINE__); //eric-mu
			panic_printk("Idx=%d BEAMFORMEE_ENTRY_NUM=%d bSounding = %d\n", Idx, BEAMFORMEE_ENTRY_NUM, bSounding); //eric-mu
			return;
		}
#endif
	
		ODM_RT_TRACE(ODMPTR, PHYDM_COMP_TXBF, ODM_DBG_LOUD, ("%s, Before notify, bSounding=%d, Idx=%d\n", __func__, bSounding, Idx));
#if (MU_BEAMFORMING_SUPPORT == 1)
		ODM_RT_TRACE(ODMPTR, PHYDM_COMP_TXBF, ODM_DBG_LOUD, ("%s: beamformee_su_cnt = %d, beamformee_mu_cnt=%d\n", __func__, pBeamInfo->beamformee_su_cnt, pBeamInfo->beamformee_mu_cnt));
#endif
		

	
		switch (bSounding) {
		case BEAMFORMEE_NOTIFY_ADD_SU:
		case BEAMFORMEE_NOTIFY_ADD_MU:
			ODM_RT_TRACE(ODMPTR, PHYDM_COMP_TXBF, ODM_DBG_LOUD, ("%s: BEAMFORMEE_NOTIFY_ADD_SU\n", __func__));
			beamforming_StartPeriod(priv, Idx);
		break;
	
		case BEAMFORMEE_NOTIFY_DELETE_SU:
		case BEAMFORMEE_NOTIFY_DELETE_MU:
			ODM_RT_TRACE(ODMPTR, PHYDM_COMP_TXBF, ODM_DBG_LOUD, ("%s: BEAMFORMEE_NOTIFY_DELETE_SU\n", __func__));
			if (pBeamTimerInfo->Mode == SOUNDING_FW_HT_TIMER || pBeamTimerInfo->Mode == SOUNDING_FW_VHT_TIMER) {
				beamforming_ClearEntry_FW(priv, TRUE, Idx);
#if (MU_BEAMFORMING_SUPPORT == 1)					
				if (pBeamInfo->beamformee_su_cnt + pBeamInfo->beamformee_mu_cnt == 0)  /* For 2->1 entry, we should not cancel SW timer */
#else
				if (pBeamInfo->beamformee_su_cnt == 0)  /* For 2->1 entry, we should not cancel SW timer */
#endif
				{
					beamforming_EndPeriod_FW(priv, Idx);
					ODM_RT_TRACE(ODMPTR, PHYDM_COMP_TXBF, ODM_DBG_LOUD, ("%s: No BFee left\n", __func__));
				}
			} else {
				beamforming_ClearEntry_SW(priv, TRUE, Idx);
#if (MU_BEAMFORMING_SUPPORT == 1)
				if (pBeamInfo->beamformee_su_cnt + pBeamInfo->beamformee_mu_cnt == 0)
#else
				if (pBeamInfo->beamformee_su_cnt == 0)
#endif				
				{
					beamforming_EndPeriod_SW(priv, Idx);
					ODM_RT_TRACE(ODMPTR, PHYDM_COMP_TXBF, ODM_DBG_LOUD, ("%s: No BFee left\n", __func__));
				}
			}
		break;
		case BEAMFORMING_NOTIFY_RESET:
			if (pBeamTimerInfo->Mode == SOUNDING_FW_HT_TIMER || pBeamTimerInfo->Mode == SOUNDING_FW_VHT_TIMER) {	
				beamforming_ClearEntry_FW(priv, FALSE, Idx);
				beamforming_EndPeriod_FW(priv, Idx);
			} else {
				beamforming_ClearEntry_SW(priv, FALSE, Idx);
				beamforming_EndPeriod_SW(priv, Idx);
			}
	
		break;
	
		default:
		break;
		}
		
}


VOID
Beamforming_Notify(
	struct rtl8192cd_priv *priv
	)
{

	u1Byte						Idx=BEAMFORMEE_ENTRY_NUM;
	BEAMFORMING_NOTIFY_STATE	bSounding = BEAMFORMING_NOTIFY_NONE;
	PRT_BEAMFORMING_INFO 		pBeamInfo = &(priv->pshare->BeamformingInfo);
	PRT_BEAMFORMING_TIMER_INFO	pBeamTimerInfo = NULL;	

	if (!(priv->drv_state & DRV_STATE_OPEN))
		return;

	if(GET_CHIP_VER(priv) == VERSION_8822B){
		Beamforming_Notify_8822B(priv);
		return;
	}
		
	bSounding = beamfomring_bSounding(priv, pBeamInfo, &Idx);

	if(Idx<BEAMFORMEE_ENTRY_NUM)
		pBeamTimerInfo = &(pBeamInfo->BeamformingTimerInfo[Idx]);	
	else if(bSounding == BEAMFORMING_NOTIFY_RESET)
		pBeamTimerInfo = &(pBeamInfo->BeamformingTimerInfo[0]);	
	else {
		//panic_printk("error !! [%s][%d] Idx=%d, bSounding=%d\n", __FUNCTION__, __LINE__, Idx, bSounding); //eric-mu
		return;
	}
		
	if(pBeamInfo->BeamformState == BEAMFORMING_STATE_END)
	{
		if(bSounding==BEAMFORMING_NOTIFY_ADD)
		{			
			beamforming_StartPeriod(priv, Idx);
			pBeamInfo->BeamformState = BEAMFORMING_STATE_START_1BFee;
		}
	}
	else if(pBeamInfo->BeamformState == BEAMFORMING_STATE_START_1BFee)
	{
		if(bSounding==BEAMFORMING_NOTIFY_ADD)
		{
			beamforming_StartPeriod(priv, Idx);
			pBeamInfo->BeamformState = BEAMFORMING_STATE_START_2BFee;
		}
		else if(bSounding == BEAMFORMING_NOTIFY_DELETE)
		{
			if(pBeamTimerInfo->Mode == SOUNDING_FW_HT_TIMER || pBeamTimerInfo->Mode == SOUNDING_FW_VHT_TIMER)
			{
				beamforming_EndPeriod_FW(priv, Idx);
				beamforming_ClearEntry_FW(priv, TRUE, Idx);
			}
			else
			{
				beamforming_EndPeriod_SW(priv, Idx);
				beamforming_ClearEntry_SW(priv, TRUE, Idx);
			}
			
			pBeamInfo->BeamformState = BEAMFORMING_STATE_END;
		}
		else if(bSounding == BEAMFORMING_NOTIFY_RESET)
		{
			if(pBeamTimerInfo->Mode == SOUNDING_FW_HT_TIMER || pBeamTimerInfo->Mode == SOUNDING_FW_VHT_TIMER)
			{
				beamforming_EndPeriod_FW(priv, Idx);
				beamforming_ClearEntry_FW(priv, FALSE, Idx);
			}
			else
			{
				beamforming_EndPeriod_SW(priv, Idx);
				beamforming_ClearEntry_SW(priv, FALSE, Idx);
			}
			
			pBeamInfo->BeamformState = BEAMFORMING_STATE_END;
		}
	}
	else if(pBeamInfo->BeamformState == BEAMFORMING_STATE_START_2BFee)
	{
		if(bSounding == BEAMFORMING_NOTIFY_ADD)
		{
			ODM_RT_TRACE(ODMPTR, PHYDM_COMP_TXBF, ODM_DBG_LOUD, ("%s, should be block\n", __FUNCTION__));  

		}
		else if(bSounding == BEAMFORMING_NOTIFY_DELETE)
		{
			if(pBeamTimerInfo->Mode == SOUNDING_FW_HT_TIMER || pBeamTimerInfo->Mode == SOUNDING_FW_VHT_TIMER)
			{
				beamforming_EndPeriod_FW(priv, Idx);
				beamforming_ClearEntry_FW(priv, TRUE, Idx);
			}
			else
			{
				// For 2->1 entry, we should not cancel SW timer
				beamforming_ClearEntry_SW(priv, TRUE, Idx);
			}
		
			pBeamInfo->BeamformState = BEAMFORMING_STATE_START_1BFee;
		}
		else if(bSounding == BEAMFORMING_NOTIFY_RESET)
		{
			if(pBeamTimerInfo->Mode == SOUNDING_FW_HT_TIMER || pBeamTimerInfo->Mode == SOUNDING_FW_VHT_TIMER)
			{
				beamforming_EndPeriod_FW(priv, Idx);
				beamforming_ClearEntry_FW(priv, FALSE, Idx);
			}
			else
			{
				beamforming_EndPeriod_SW(priv, Idx);
				beamforming_ClearEntry_SW(priv, FALSE, Idx);
			}
			
			pBeamInfo->BeamformState = BEAMFORMING_STATE_END;
		}
	}

}


VOID
Beamforming_AutoTest(
	struct rtl8192cd_priv *priv,
	u1Byte					Idx, 
	PRT_BEAMFORMING_ENTRY	pBeamformEntry
	)
{
	SOUNDING_MODE					Mode;

	BEAMFORMING_CAP				BeamformCap = pBeamformEntry->BeamformEntryCap;
	PRT_BEAMFORMING_INFO 		pBeamInfo = &(priv->pshare->BeamformingInfo);	
	PRT_BEAMFORMING_PERIOD_INFO pBeamPeriodInfo = &(pBeamInfo->BeamformingPeriodInfo);

	if(BeamformCap & BEAMFORMER_CAP_VHT_SU)
		Mode = SOUNDING_SW_VHT_TIMER;
	else if(BeamformCap & BEAMFORMER_CAP_HT_EXPLICIT)
	{
		Mode = SOUNDING_SW_HT_TIMER;		// use sw timer for all IC
	
/*#ifdef CONFIG_WLAN_HAL_8192EE
		if(GET_CHIP_VER(priv)== VERSION_8192E)
			Mode = SOUNDING_SW_HT_TIMER;
#endif
#ifdef CONFIG_RTL_8812_SUPPORT
		if(GET_CHIP_VER(priv)== VERSION_8812E)
			Mode = SOUNDING_HW_HT_TIMER;
#endif
#ifdef CONFIG_WLAN_HAL_8814AE
		if(GET_CHIP_VER(priv)== VERSION_8814A)
			Mode = SOUNDING_HW_HT_TIMER;
#endif*/
	}
	else 
		return;

	pBeamPeriodInfo->Idx = Idx;
	pBeamPeriodInfo->Mode = Mode;
	pBeamPeriodInfo->BW = pBeamformEntry->BW;

	pBeamPeriodInfo->BeamPeriod = priv->pshare->rf_ft_var.soundingPeriod;


}


VOID
Beamforming_End(
	struct rtl8192cd_priv *priv,
	BOOLEAN			Status	
	)
{

	PRT_BEAMFORMING_INFO pBeamformingInfo = &(priv->pshare->BeamformingInfo);
    PRT_BEAMFORMING_ENTRY	pBFeeEntry;

	if(pBeamformingInfo->BeamformeeCurIdx >= BEAMFORMEE_ENTRY_NUM)
		return;
		
	pBFeeEntry = &(pBeamformingInfo->BeamformeeEntry[pBeamformingInfo->BeamformeeCurIdx]);	

	ODM_RT_TRACE(ODMPTR, PHYDM_COMP_TXBF, ODM_DBG_LOUD, ("[%s] Status = %d, BeamformEntryState=%d, pBFeeEntry->bUsed=%d, pBFeeEntry->Sounding_En = %d\n",
	 __FUNCTION__, Status,
	pBFeeEntry->BeamformEntryState,
	pBFeeEntry->bUsed,
	pBFeeEntry->Sounding_En
	));

	if((pBFeeEntry->BeamformEntryState != BEAMFORMING_ENTRY_STATE_PROGRESSING)||(pBFeeEntry->bUsed==0)|| (pBFeeEntry->Sounding_En==0))
		return;

	//Because in this case 8814A STOP sounding @BeamformingStart_V2 , so NOT apply V-matrix here.
	if(((pBFeeEntry->pSTA) && (pBFeeEntry->pSTA->current_tx_rate >=_NSS3_MCS7_RATE_) && (pBFeeEntry->pSTA->current_tx_rate <=_NSS3_MCS9_RATE_)) &&
		priv->pshare->rf_ft_var.Nsnding)
	{
		ODM_RT_TRACE(ODMPTR, PHYDM_COMP_TXBF, ODM_DBG_LOUD, ("%s, VHT3SS 7,8,9, do not apply V matrix.\n", __FUNCTION__));
		pBFeeEntry->BeamformEntryState = BEAMFORMING_ENTRY_STATE_INITIALIZED;
		Beamforming_SetBeamFormStatus(priv, (pBeamformingInfo->BeamformeeCurIdx));
	}
	else if (Status == 1)        // Receive CSI successful
	{
		pBFeeEntry->LogStatusFailCnt = 0;
		pBFeeEntry->BeamformEntryState = BEAMFORMING_ENTRY_STATE_PROGRESSED;
		Beamforming_SetBeamFormStatus(priv, (pBeamformingInfo->BeamformeeCurIdx));
		ODM_RT_TRACE(ODMPTR, PHYDM_COMP_TXBF, ODM_DBG_LOUD, ("[%s], Sounding Success\n", __FUNCTION__));
	}
	else        // Receive CSI failure
	{
		pBFeeEntry->BeamformEntryState = BEAMFORMING_ENTRY_STATE_INITIALIZED;
		pBFeeEntry->LogStatusFailCnt++;
		Beamforming_SetBeamFormStatus(priv, (pBeamformingInfo->BeamformeeCurIdx));
		ODM_RT_TRACE(ODMPTR, PHYDM_COMP_TXBF, ODM_DBG_LOUD, ("[%s], Sounding Fail\n", __FUNCTION__));
	}

	pBFeeEntry->bBeamformingInProgress = FALSE;
	ODM_RT_TRACE(ODMPTR, PHYDM_COMP_TXBF, ODM_DBG_LOUD, ("%s, pEntry->LogStatusFailCnt : %d\n", __FUNCTION__, pBFeeEntry->LogStatusFailCnt));  

	//Receive CSI failure
	if (pBFeeEntry->LogStatusFailCnt > 50) {
		ODM_RT_TRACE(ODMPTR, PHYDM_COMP_TXBF, ODM_DBG_LOUD, ("[Beamforming]@%s, LogStatusFailCnt > 50\n", __FUNCTION__));
        	if ((GET_CHIP_VER(priv)== VERSION_8814A) || (GET_CHIP_VER(priv)== VERSION_8197F))
				ODM_RT_TRACE(ODMPTR, PHYDM_COMP_TXBF, ODM_DBG_LOUD, ("%s, 97F and 8814A don't remove entry\n", __FUNCTION__));
		else {
			pBeamformingInfo->CurDelBFerBFeeEntrySel = BFeeEntry;
			if(Beamforming_DeInitEntry(priv, pBFeeEntry->MacAddr))
				Beamforming_Notify(priv);
		}
	}
}	

int shortenSoundingPeriod(struct rtl8192cd_priv *priv)
{
	PRT_BEAMFORMING_INFO 		pBeamformingInfo = &(priv->pshare->BeamformingInfo);
	PRT_BEAMFORMING_ENTRY		pEntry = &(pBeamformingInfo->BeamformeeEntry[0]);
	struct stat_info 			*pstat;
	struct rtl8192cd_priv 		*vxd_priv;	
	u4Byte idx, j, ret=0;
	pBeamformingInfo->BeamformingPeriodState = 0;     //

	for(idx=0 ; idx<BEAMFORMEE_ENTRY_NUM; idx++)
	{
		pEntry = &(pBeamformingInfo->BeamformeeEntry[idx]);
		if( pEntry->bUsed)  
		{
			pstat = get_stainfo(priv, pEntry->MacAddr);
			if(pstat) 
			{
				if(pstat->tx_avarage > 625000)//5Mbps
				{
					++ret;
					if(idx == 0)                                                                        //
						pBeamformingInfo->BeamformingPeriodState+=1;        // entry 0 only = 1
					else                                                                                  // entry 1 only = 2
						pBeamformingInfo->BeamformingPeriodState+=2;        // entry 0 and 1 = 3
				}
			}

#ifdef MBSSID
		  if ((OPMODE & WIFI_AP_STATE) && priv->pmib->miscEntry.vap_enable)
		  {
			for (j=0; j<RTL8192CD_NUM_VWLAN; j++)
			{
				if ((priv->pvap_priv[j]->assoc_num > 0) && IS_DRV_OPEN(priv->pvap_priv[j]))
				{
					pstat = get_stainfo(priv->pvap_priv[j], pEntry->MacAddr);
					if(pstat)
					{
							if(pstat->tx_avarage > 625000)  // 5Mbps
						{
							++ret;
							if(idx == 0)                                                                        //
								pBeamformingInfo->BeamformingPeriodState|=1;        // entry 0 only = 1
							else                                                                                  // entry 1 only = 2
								pBeamformingInfo->BeamformingPeriodState|=2;        // entry 0 and 1 = 3
						}
					}
				}
			}
		}
#endif		
#ifdef UNIVERSAL_REPEATER
			vxd_priv = GET_VXD_PRIV(priv);
			if((OPMODE_VXD & WIFI_STATION_STATE) && (vxd_priv->assoc_num > 0) && IS_DRV_OPEN(vxd_priv) 	)
			{		
				pstat = get_stainfo(vxd_priv, pEntry->MacAddr);
				if(pstat)
				{
					if(pstat->tx_avarage >  625000)  // 5Mbps
					{
						++ret;
						if(idx == 0)                                                                        //
							pBeamformingInfo->BeamformingPeriodState|=1;        // entry 0 only = 1
						else                                                                                  // entry 1 only = 2
							pBeamformingInfo->BeamformingPeriodState|=2;        // entry 0 and 1 = 3
					}
				}
			}
#endif
			
		}
	}
	//panic_printk("BeamformPeriodState = %d\n", pBeamformingInfo->BeamformingPeriodState);
	return ret;
}

u1Byte
getBFeeStaNum(
	struct rtl8192cd_priv *priv
	)
{
	PRT_BEAMFORMING_INFO 		pBeamformingInfo = &(priv->pshare->BeamformingInfo);
	int idx;
	u1Byte BFee_STA_Num = 0;
	for(idx=0 ; idx<BEAMFORMEE_ENTRY_NUM; idx++)
	{
		if(pBeamformingInfo->BeamformeeEntry[idx].bUsed)
			BFee_STA_Num++;
	}
	return BFee_STA_Num;	
}


VOID
Beamforming_TimerCallback(
	struct rtl8192cd_priv *priv
	)
{
	BOOLEAN						ret = FALSE;
	PRT_BEAMFORMING_INFO 		pBeamformingInfo = &(priv->pshare->BeamformingInfo);
	u1Byte						idx = pBeamformingInfo->BeamformeeCurIdx;		
	PRT_BEAMFORMING_ENTRY		pEntry = &(pBeamformingInfo->BeamformeeEntry[idx]);
	PRT_BEAMFORMING_TIMER_INFO	pBeamformingTimerInfo = &(pBeamformingInfo->BeamformingTimerInfo[idx]);	
	u1Byte						BFee_STA_Num = 0;

	BFee_STA_Num = getBFeeStaNum(priv);
	
	if (!(priv->drv_state & DRV_STATE_OPEN))
		return;

	if (BFee_STA_Num == 2) {
		if(pBeamformingInfo->BeamformingPeriodState == 0 || pBeamformingInfo->BeamformingPeriodState == 3) {
			if(pBeamformingInfo->BeamformeeEntry[idx^1].bUsed)
				idx ^=1;
		} else if(pBeamformingInfo->BeamformingPeriodState == 2)
			idx = 1;
		else
			idx = 0;
	} else {  
		if(pBeamformingInfo->BeamformeeEntry[0].bUsed)
			idx = 0;
		else if(pBeamformingInfo->BeamformeeEntry[1].bUsed)
			idx = 1;
	}

	pBeamformingInfo->BeamformeeCurIdx = idx;

	pEntry = &(pBeamformingInfo->BeamformeeEntry[idx]);
	pBeamformingTimerInfo = &(pBeamformingInfo->BeamformingTimerInfo[idx]);
		
	if (pEntry->bBeamformingInProgress)
		Beamforming_End(priv, 0);
	if (pEntry->bUsed) 
		ret = BeamformingStart_V2(priv, idx, pBeamformingTimerInfo->Mode, pEntry->BW);

	if (ret == FALSE)
		ODM_RT_TRACE(ODMPTR, PHYDM_COMP_TXBF, ODM_DBG_LOUD, ("%s, BeamformingStart_V2 fail\n", __FUNCTION__));

	if (pBeamformingInfo->BeamformeeEntry[0].bUsed || pBeamformingInfo->BeamformeeEntry[1].bUsed) {		
		if (pBeamformingInfo->BeamformState >= BEAMFORMING_STATE_START_1BFee) {
			if (pBeamformingTimerInfo->Mode == SOUNDING_SW_VHT_TIMER || pBeamformingTimerInfo->Mode == SOUNDING_SW_HT_TIMER) {
				if (shortenSoundingPeriod(priv)) {
					if (pBeamformingInfo->BeamformingPeriodState == 1 || pBeamformingInfo->BeamformingPeriodState == 2)
						ODM_SetTimer(ODMPTR, &pBeamformingInfo->BeamformingTimer, pBeamformingTimerInfo->BeamPeriod/100);
					else   // pBeamformingInfo->BeamformingPeriodState == 3
						ODM_SetTimer(ODMPTR, &pBeamformingInfo->BeamformingTimer, pBeamformingTimerInfo->BeamPeriod/200);
				}
				else
					ODM_SetTimer(ODMPTR, &pBeamformingInfo->BeamformingTimer, pBeamformingTimerInfo->BeamPeriod);
			} else {	//for HW timer, not used now
				int BeamPeriod = priv->pshare->rf_ft_var.soundingPeriod;

				if (pBeamformingTimerInfo->Mode == SOUNDING_HW_VHT_TIMER || pBeamformingTimerInfo->Mode == SOUNDING_HW_HT_TIMER)
					BeamPeriod *=32;	//HW timer, clock = 32K

				if (shortenSoundingPeriod(priv)) {
					if(pBeamformingInfo->BeamformingPeriodState == 1 || pBeamformingInfo->BeamformingPeriodState == 2) //only one entry is in used
						BeamPeriod /= 100;
				else //two entries are in used
						BeamPeriod /= 200;
				}

				if (pBeamformingTimerInfo->BeamPeriod != BeamPeriod)
					pBeamformingTimerInfo->BeamPeriod = BeamPeriod;
				Beamforming_SetHWTimer(priv, pBeamformingTimerInfo->BeamPeriod);
			}
		}
	}

}

VOID Beamforming_SWTimerCallback(unsigned long task_priv)
{
	struct rtl8192cd_priv *priv = (struct rtl8192cd_priv *)task_priv;
	Beamforming_TimerCallback(priv);
}

VOID
beamform_InitSoundingVars(
	struct rtl8192cd_priv *priv
)
{
	PRT_BEAMFORMING_INFO 		pBeamInfo = &(priv->pshare->BeamformingInfo);
	PRT_SOUNDING_INFOV2			pSoundingInfo = &(pBeamInfo->SoundingInfoV2);
	
	pSoundingInfo->State = SOUNDING_STATE_NONE;
	pSoundingInfo->SUBFeeCurIdx = 0xFF;
#if (MU_BEAMFORMING_SUPPORT == 1)	
	pSoundingInfo->CandidateMUBFeeCnt = 0;
#endif
	pSoundingInfo->MinSoundingPeriod = 0;
	pSoundingInfo->SoundRemainCntPerPeriod = 0;	// Get from sounding list. Ex: SU STA1, SU STA2, MU STA(1~n) => the value will be 2+1=3.
	pSoundingInfo->SUSoundNumPerPeriod = 0;
#if (MU_BEAMFORMING_SUPPORT == 1)	
	pSoundingInfo->MUSoundNumPerPeriod = 0;
#endif
}

BOOLEAN
beamforming_rf_iqgen_setting (
	struct rtl8192cd_priv *priv
)
{
	BOOLEAN is_iqgen_setting_ok = FALSE;
	
#ifdef CONFIG_WLAN_HAL_8814AE
		if(GET_CHIP_VER(priv) == VERSION_8814A)
			is_iqgen_setting_ok = beamforming_setiqgen_8814a(priv);
#endif

	return is_iqgen_setting_ok;

}


VOID
Beamforming_Init(
	struct rtl8192cd_priv *priv
	)
{
	PRT_BEAMFORMING_INFO pBeamInfo = &(priv->pshare->BeamformingInfo);
	PRT_BEAMFORMING_PERIOD_INFO	pBeamPeriodInfo = &(pBeamInfo->BeamformingPeriodInfo);
	BOOLEAN is_iqgen_setting_ok = FALSE;

	Beamforming_SetBeamFormInit(priv);

	pBeamInfo->BeamformingPeriodState = 0;
	pBeamPeriodInfo->Mode = SOUNDING_STOP_OID_TIMER;

	ODM_RT_TRACE(ODMPTR, PHYDM_COMP_TXBF, ODM_DBG_LOUD, ("%s, Init Timer\n", __FUNCTION__));

	if(GET_CHIP_VER(priv)== VERSION_8822B) {

		if(priv->pmib->dot11RFEntry.txbfer)
		{
			pBeamInfo->BeamformCap |= (BEAMFORMER_CAP_HT_EXPLICIT);

			if(priv->pmib->dot11BssType.net_work_type & WIRELESS_11AC) {
				pBeamInfo->BeamformCap |= BEAMFORMER_CAP_VHT_SU;
				
				#if (MU_BEAMFORMING_SUPPORT == 1)		
				if (OPMODE & WIFI_AP_STATE)
				pBeamInfo->BeamformCap |= BEAMFORM_CAP_VHT_MU_BFER;
				#endif		
			}


		}
		if(priv->pmib->dot11RFEntry.txbfee)
		{
			pBeamInfo->BeamformCap |= (BEAMFORMEE_CAP_HT_EXPLICIT);

			if(priv->pmib->dot11BssType.net_work_type & WIRELESS_11AC)
				pBeamInfo->BeamformCap |= (BEAMFORMEE_CAP_VHT_SU
				#if (MU_BEAMFORMING_SUPPORT == 1)		
				| BEAMFORM_CAP_VHT_MU_BFEE
				#endif		
				);
		}


		init_timer(&pBeamInfo->BeamformingTimer);
		pBeamInfo->BeamformingTimer.function = beamform_SoundingTimerCallback;
		pBeamInfo->BeamformingTimer.data = (unsigned long)priv;
	
		init_timer(&pBeamInfo->BFSoundingTimeoutTimer);
		pBeamInfo->BFSoundingTimeoutTimer.function = beamform_SoundingTimeout;
		pBeamInfo->BFSoundingTimeoutTimer.data = (unsigned long)priv;
		beamform_InitSoundingVars(priv);
	}
	else
	{	
		init_timer(&pBeamInfo->BeamformingTimer);
		pBeamInfo->BeamformingTimer.function = Beamforming_SWTimerCallback;
		pBeamInfo->BeamformingTimer.data = (unsigned long)priv;
	}

	is_iqgen_setting_ok = beamforming_rf_iqgen_setting(priv);
	ODM_RT_TRACE(ODMPTR, PHYDM_COMP_TXBF, ODM_DBG_LOUD, ("%s, is_iqgen_setting_ok = %d\n", __FUNCTION__, is_iqgen_setting_ok));
	
}


VOID
Beamforming_Release(
	struct rtl8192cd_priv *priv
	)
{
	PRT_BEAMFORMING_INFO pBeamformingInfo = &(priv->pshare->BeamformingInfo);
	BeamformingReset(priv);	
	ODM_CancelTimer(ODMPTR, &pBeamformingInfo->BeamformingTimer);
	if(GET_CHIP_VER(priv)== VERSION_8822B)
		ODM_CancelTimer(ODMPTR, &pBeamformingInfo->BFSoundingTimeoutTimer);

	ODM_RT_TRACE(ODMPTR, PHYDM_COMP_TXBF, ODM_DBG_LOUD, ("%s, Release Timer\n", __FUNCTION__));  
}		



VOID
Beamforming_Enter(
	struct rtl8192cd_priv *priv,
	struct stat_info	*pstat
)
{
	u1Byte	BFerBFeeIdx = 0xff;
	PRT_BEAMFORMING_INFO 	pBeamInfo = &(priv->pshare->BeamformingInfo);

	if(priv->pmib->dot11RFEntry.txbf == 0) //eric-mu
		return;

	if(Beamforming_InitEntry(priv, pstat, &BFerBFeeIdx))
	{
		Beamforming_SetBeamFormEnter(priv, BFerBFeeIdx);

		if(GET_CHIP_VER(priv)== VERSION_8822B) {
			ODM_RT_TRACE(ODMPTR, PHYDM_COMP_TXBF, ODM_DBG_LOUD, ("%s, BeamformCap = 0x%x State = 0x%x \n", __FUNCTION__, pBeamInfo->BeamformCap, pBeamInfo->SoundingInfoV2.State));  
			//if(pBeamInfo->BeamformCap & (BEAMFORMER_CAP_HT_EXPLICIT|BEAMFORMER_CAP_VHT_SU|BEAMFORM_CAP_VHT_MU_BFER))
			{
				if(pBeamInfo->SoundingInfoV2.State == SOUNDING_STATE_NONE)
				{
					pBeamInfo->SoundingInfoV2.State = SOUNDING_STATE_INIT;
					ODM_RT_TRACE(ODMPTR, PHYDM_COMP_TXBF, ODM_DBG_LOUD, ("%s, Set BeamformingTimer = 2000\n", __FUNCTION__)); 
					ODM_SetTimer(ODMPTR, &pBeamInfo->BeamformingTimer, 2000); // Start sounding after 2 sec.
				}
			}
		}
	}	
}


VOID
Beamforming_Leave(
	struct rtl8192cd_priv *priv,
	pu1Byte			RA
	)
{
	u1Byte		Idx = 0;
	PRT_BEAMFORMING_INFO 	pBeamformingInfo = &(priv->pshare->BeamformingInfo);
	Beamforming_GetBFeeEntryByAddr(priv, RA, &Idx);

	if(RA == NULL)
	{
		BeamformingReset(priv);
		ODM_RT_TRACE(ODMPTR, PHYDM_COMP_TXBF, ODM_DBG_LOUD, ("%s, Reset entry\n", __FUNCTION__));  
	}
	else
	{
        pBeamformingInfo->CurDelBFerBFeeEntrySel = BFerBFeeEntry;
		Beamforming_DeInitEntry(priv, RA);
		ODM_RT_TRACE(ODMPTR, PHYDM_COMP_TXBF, ODM_DBG_LOUD, ("%s, DeInit entry %d\n", __FUNCTION__, Idx));  
	}

	Beamforming_Notify(priv);

}


VOID
Beamforming_SetTxBFen(
	struct rtl8192cd_priv *priv,
	u1Byte			MacId,
	BOOLEAN			bTxBF
	)
{
	u1Byte					Idx = 0;
//	PRT_BEAMFORMING_INFO 	pBeamformingInfo = &(priv->pshare->BeamformingInfo);
	PRT_BEAMFORMING_ENTRY	pEntry;

	pEntry = Beamforming_GetEntryByMacId(priv, MacId, &Idx);

	if(pEntry == NULL)
		return;
	else
		pEntry->bTxBF = bTxBF;

	Beamforming_Notify(priv);
}

BEAMFORMING_CAP
Beamforming_GetBeamCap(
	struct rtl8192cd_priv *priv,
	IN PRT_BEAMFORMING_INFO 	pBeamInfo
	)
{
	u1Byte					i;
	BOOLEAN 				bSelfBeamformer = FALSE;
	BOOLEAN 				bSelfBeamformee = FALSE;
	RT_BEAMFORMING_ENTRY	BeamformeeEntry;
	RT_BEAMFORMER_ENTRY	BeamformerEntry;
	BEAMFORMING_CAP 		BeamformCap = BEAMFORMING_CAP_NONE;

	/*
	for(i = 0; i < BEAMFORMEE_ENTRY_NUM; i++)
	{
		BeamformEntry = pBeamInfo->BeamformeeEntry[i];

		if(BeamformEntry.bUsed)
		{
			if( (BeamformEntry.BeamformEntryCap & BEAMFORMEE_CAP_VHT_SU) ||
				(BeamformEntry.BeamformEntryCap & BEAMFORMEE_CAP_HT_EXPLICIT))
				bSelfBeamformee = TRUE;
			if( (BeamformEntry.BeamformEntryCap & BEAMFORMER_CAP_VHT_SU) ||
				(BeamformEntry.BeamformEntryCap & BEAMFORMER_CAP_HT_EXPLICIT))
				bSelfBeamformer = TRUE;
		}

		if(bSelfBeamformer && bSelfBeamformee)
			i = BEAMFORMEE_ENTRY_NUM;
	}
	*/

	for(i = 0; i < BEAMFORMEE_ENTRY_NUM; i++)
	{
		BeamformeeEntry = pBeamInfo->BeamformeeEntry[i];

		if(BeamformeeEntry.bUsed)
		{
			bSelfBeamformer = TRUE;
			//panic_printk("[Beamform]%s, BFee entry %d bUsed=TRUE\n", __FUNCTION__, i);  
		}
	}

	for(i = 0; i < BEAMFORMER_ENTRY_NUM; i++)
	{
		BeamformerEntry = pBeamInfo->BeamformerEntry[i];

		if(BeamformerEntry.bUsed)
		{
			bSelfBeamformee = TRUE;
			//panic_printk"[Beamform]%s, BFer entry %d bUsed=TRUE\n", __FUNCTION__, i);  
		}
	}

	if(bSelfBeamformer)
		BeamformCap |= BEAMFORMER_CAP;
	if(bSelfBeamformee)
		BeamformCap |= BEAMFORMEE_CAP;

	return BeamformCap;
}

VOID
Beamforming_GetNDPAFrame(
	struct rtl8192cd_priv *priv,
	pu1Byte 					pNDPAFrame
	)
{
	pu1Byte						TA ;
	u1Byte						Idx, Sequence;
	PRT_BEAMFORMER_ENTRY		pBeamformerEntry = NULL;
    PRT_BEAMFORMING_INFO    pBeamformingInfo = &(priv->pshare->BeamformingInfo);

	if (GET_CHIP_VER(priv) != VERSION_8812E)
		return;
	if(IsCtrlNDPA(pNDPAFrame) == FALSE)
		return;

	TA = GetAddr2Ptr(pNDPAFrame);
	// Remove signaling TA. 
	TA[0] = TA[0] & 0xFE; 

        pBeamformerEntry = Beamforming_GetBFerEntryByAddr(priv, TA, &Idx);

        if(pBeamformerEntry == NULL)
		return;
        else if(!(pBeamformerEntry->BeamformEntryCap & BEAMFORMEE_CAP_VHT_SU))
		return;
        
        // NDPALogSuccess: As long as 8812A receive NDPA and feedback CSI succeed once, clock reset is NO LONGER needed !2015-04-10, Jeffery
        // ClockResetTimes: While BFer entry always doesn't receive our CSI, clock will reset again and again.So ClockResetTimes is limited to 5 times.2015-04-13, Jeffery
        else if( pBeamformerEntry->NDPALogSuccess==1 )
        {
			ODM_RT_TRACE(ODMPTR, PHYDM_COMP_TXBF, ODM_DBG_LOUD, ("%s ,ClockResetTimes=%d, NDPALogSuccess=%d, clock reset is no longer needed!!\n",
			__FUNCTION__, 
			pBeamformerEntry->ClockResetTimes,
			pBeamformerEntry->NDPALogSuccess));
            return;
        }
        else if( pBeamformerEntry->ClockResetTimes==3 )
        {
			 ODM_RT_TRACE(ODMPTR, PHYDM_COMP_TXBF, ODM_DBG_LOUD, ("%s ,ClockResetTimes=%d, DeInit BFer Entry !!\n",
						__FUNCTION__,
						pBeamformerEntry->ClockResetTimes));

            pBeamformingInfo->CurDelBFerBFeeEntrySel = BFerEntry ;
            Beamforming_DeInitEntry(priv, pBeamformerEntry->MacAddr);
			return;
        }

		Sequence = (pNDPAFrame[16]) >> 2;

        ODM_RT_TRACE(ODMPTR, PHYDM_COMP_TXBF, ODM_DBG_LOUD, ("%s Start, Sequence=%d, NDPALogSeq=%d, NDPAPreLogSeq=%d, NDPALogRetryCnt=%d, ClockResetTimes=%d, NDPALogSuccess=%d\n",
					__FUNCTION__,
					Sequence,
					pBeamformerEntry->NDPALogSeq,
					pBeamformerEntry->NDPAPreLogSeq,
					pBeamformerEntry->NDPALogRetryCnt,
					pBeamformerEntry->ClockResetTimes,
					pBeamformerEntry->NDPALogSuccess));

        if ((pBeamformerEntry->NDPALogSeq != 0) && (pBeamformerEntry->NDPAPreLogSeq != 0))
	{
            //2 Success Condition
            if( (pBeamformerEntry->NDPALogSeq!=Sequence)&&(pBeamformerEntry->NDPALogSeq!=pBeamformerEntry->NDPAPreLogSeq) )
	{
                /* break option for clcok reset, 2015-03-30, Jeffery */
                pBeamformerEntry->NDPALogRetryCnt = 0;

                /*As long as 8812A receive NDPA and feedback CSI succeed once, clock reset is no longer needed.*/
                /*That is, NDPALogSuccess is NOT needed to be reset to zero, 2015-04-13, Jeffery*/
                pBeamformerEntry->NDPALogSuccess=1;
	}

            //2 Fail Condition
	else
	{
                
                if (pBeamformerEntry->NDPALogRetryCnt == 5)
                {
                
                    pBeamformerEntry->ClockResetTimes++;
                    pBeamformerEntry->NDPALogRetryCnt = 0;

                    watchdog_kick();

                    ODM_RT_TRACE(ODMPTR, PHYDM_COMP_TXBF, ODM_DBG_LOUD, ("%s ,Clock Reset!!! ClockResetTimes=%d\n",
	                    __FUNCTION__, 
	                    pBeamformerEntry->ClockResetTimes));  
#ifdef CONFIG_RTL_8812_SUPPORT	
					_Beamforming_CLK(priv);
#endif		
				}
                else
                    pBeamformerEntry->NDPALogRetryCnt++;
            }
        }

        pBeamformerEntry->NDPAPreLogSeq = pBeamformerEntry->NDPALogSeq;
        pBeamformerEntry->NDPALogSeq = Sequence;

}


/*********************************************************/
/*                                          MU-MIMO                                                     */
/*********************************************************/
#if (MU_BEAMFORMING_SUPPORT == 1)
void input_value_16(pu2Byte p, unsigned char start, unsigned char end, u2Byte value)
{
	u2Byte bit_mask = 0;

	if(value > 0) //only none-zero value needs to be assigned 
	{
		if(start == end) //1-bit value
		{
			*p |= BIT(start);
		}
		else
		{
			unsigned char x = 0;
				
			for(x = 0; x<=(end-start); x ++)
				bit_mask |= BIT(x);

			*p |= ((value&bit_mask) << start);	
		}
	}

}

VOID
ConstructVHTNDPAPacket_MU(
	struct rtl8192cd_priv *priv,
	pu1Byte			RA,
	u2Byte			AID,
	pu1Byte			Buffer,
	pu4Byte			pLength,
	u1Byte 			BW
	)
{
	u2Byte					Duration= 0;
	u1Byte					Sequence = 0;
	pu1Byte					pNDPAFrame = Buffer;
	u2Byte					tmp16 = 0;
	char					idx =0, cnt ;
	
	
	RT_NDPA_STA_INFO		STAInfo;
	int aSifsTime = ((priv->pmib->dot11BssType.net_work_type & WIRELESS_11N) && (priv->pshare->ht_sta_num)) ? 0x10 : 10;

	PRT_BEAMFORMING_INFO		pBeamInfo = &(priv->pshare->BeamformingInfo);
	PRT_BEAMFORMING_ENTRY	pEntry;
	struct stat_info *pstat;

	// Frame control.
	SET_80211_HDR_FRAME_CONTROL(pNDPAFrame, 0);
	SET_80211_HDR_TYPE_AND_SUBTYPE(pNDPAFrame, Type_NDPA);

	memcpy((void *)GetAddr1Ptr(pNDPAFrame), RA, MACADDRLEN);
	memcpy((void *)GetAddr2Ptr(pNDPAFrame), GET_MY_HWADDR, MACADDRLEN);

	Duration = 2*aSifsTime + 44;
	
	if(BW == HT_CHANNEL_WIDTH_80)
		Duration += 40;
	else if(BW == HT_CHANNEL_WIDTH_20_40)
		Duration+= 87;
	else	
		Duration+= 180;

	SetDuration(pNDPAFrame, Duration);
	Sequence = GET_HW(priv)->sounding_seq<<2;
	GET_HW(priv)->sounding_seq =  (GET_HW(priv)->sounding_seq+1) & 0xfff;
	 
	memcpy(pNDPAFrame+16, &Sequence,1);

	//eric-8822 ?? there will be multiple STA infos ??

	if (OPMODE & WIFI_ADHOC_STATE)
		AID = 0;


	*pLength = 17;


	/* Construct STA info. for multiple STAs */
#ifdef CONFIG_VERIWAVE_MU_CHECK		
	cnt = 0;
	for (idx = 0; idx < BEAMFORMEE_ENTRY_NUM; idx++) {		
		pEntry = &(pBeamInfo->BeamformeeEntry[idx]);
		if (pEntry->bUsed && pEntry->is_mu_sta && pEntry->pSTA && pEntry->pSTA->isSendNDPA)
			cnt++;
	}
#endif
	for (idx = 0; idx < BEAMFORMEE_ENTRY_NUM; idx++) {		

		pEntry = &(pBeamInfo->BeamformeeEntry[idx]);

		if (pEntry->bUsed && pEntry->is_mu_sta && pEntry->pSTA && pEntry->pSTA->isSendNDPA) {
			//panic_printk("[%d] is_mu_sta=%d AID=%d P_AID=0x%x\n", idx, pEntry->is_mu_sta, pEntry->AID, pEntry->P_AID);
#ifdef CONFIG_VERIWAVE_MU_CHECK					
			if(cnt >=3 && pEntry->pSTA->isVeriwaveInValidSTA)
				continue;
#endif			
			STAInfo.AID = pEntry->AID;
			STAInfo.FeedbackType = 1; /* 1'b1: MU */
			STAInfo.NcIndex = 0;
			tmp16 = 0;
			memset(&tmp16, 0, sizeof(tmp16));
			
			input_value_16(&tmp16, 0, 11, pEntry->AID);
			input_value_16(&tmp16, 12, 12, 1);
			input_value_16(&tmp16, 13, 15, 0);

			tmp16 = cpu_to_le16(tmp16);
			memcpy(pNDPAFrame+(*pLength), &tmp16, 2);

			*pLength += 2;
		}
	}
	
}


BOOLEAN
SendVHTNDPAPacket_MU(
	struct rtl8192cd_priv *priv,
	IN	pu1Byte			RA,
	IN	u2Byte			AID,
	u1Byte 				BW,
	u1Byte		NDPTxRate,
	u1Byte		isUni
	)
{
	BOOLEAN					ret = TRUE;
	u4Byte 					PacketLength;
	unsigned char *pbuf 	= get_wlanllchdr_from_poll(priv);
	DECLARE_TXINSN(txinsn);	

	if(pbuf)
	{
		memset(pbuf, 0, sizeof(struct wlan_hdr));

		ConstructVHTNDPAPacket_MU	(
			priv, 
			RA,
			AID,
			pbuf,
			&PacketLength,
			BW
			);

#if 1 //eric-mu
		if(priv->pshare->rf_ft_var.sndlen != 0xff ){
			printk(" !!! Force PacketLength = %d \n", priv->pshare->rf_ft_var.sndlen);
			PacketLength = priv->pshare->rf_ft_var.sndlen;

		}
#endif


		txinsn.q_num = MANAGE_QUE_NUM;
		txinsn.fr_type = _PRE_ALLOCLLCHDR_;		
		txinsn.phdr = pbuf;
		txinsn.hdr_len = PacketLength;
		txinsn.fr_len = 0;
		txinsn.fixed_rate = 1;	
		txinsn.tx_rate = NDPTxRate;	// According to Nr
		txinsn.ndpa = 1; //broadcast NDPA

#if (MU_BEAMFORMING_SUPPORT == 1)
		if(!isUni)
			txinsn.SND_pkt_sel = 1; //broadcast NDPA
		else
#endif			
			txinsn.SND_pkt_sel = 0; //unicast NDPA

#if 1 //eric-mu
		if(priv->pshare->rf_ft_var.snd != 0xff ){
			printk(" !!! Force SND_pkt_sel = 0x%x \n", priv->pshare->rf_ft_var.snd);
			txinsn.SND_pkt_sel = priv->pshare->rf_ft_var.snd;

		}
#endif

		if (rtl8192cd_wlantx(priv, &txinsn) == CONGESTED) {		
			netif_stop_queue(priv->dev);		
			priv->ext_stats.tx_drops++; 	
			panic_printk("[%s]TX DROP: Congested!\n", __FUNCTION__);
			if (txinsn.phdr)
				release_wlanhdr_to_poll(priv, txinsn.phdr); 			
			if (txinsn.pframe)
				release_mgtbuf_to_poll(priv, txinsn.pframe);
			return 0;	
		}
	}
	else
		ret = FALSE;

	return ret;
}


VOID
ConstructReportPollPacket(
	struct rtl8192cd_priv *priv,
	pu1Byte			RA,
	pu1Byte			Buffer,
	pu4Byte			pLength,
	u1Byte 			BW
	)
{
	u2Byte					Duration= 0;
	u1Byte					FSR_Bitmap = 0xff;
	pu1Byte					pNDPAFrame = Buffer;
	
	int aSifsTime = ((priv->pmib->dot11BssType.net_work_type & WIRELESS_11N) && (priv->pshare->ht_sta_num)) ? 0x10 : 10;

	// Frame control.
	SET_80211_HDR_FRAME_CONTROL(pNDPAFrame, 0);
	SET_80211_HDR_TYPE_AND_SUBTYPE(pNDPAFrame, Type_Beamforming_Report_Poll);

	memcpy((void *)GetAddr1Ptr(pNDPAFrame), RA, MACADDRLEN);
	memcpy((void *)GetAddr2Ptr(pNDPAFrame), GET_MY_HWADDR, MACADDRLEN);

	Duration = 2*aSifsTime + 44;
	
	if(BW == HT_CHANNEL_WIDTH_80)
		Duration += 40;
	else if(BW == HT_CHANNEL_WIDTH_20_40)
		Duration+= 87;
	else	
		Duration+= 180;

	SetDuration(pNDPAFrame, Duration);

	memcpy(pNDPAFrame+16, &FSR_Bitmap, 1);

	*pLength = 17;
}


BOOLEAN
SendReportPollPacket(
	struct rtl8192cd_priv *priv,
	IN	pu1Byte			RA,
	u1Byte 				BW,
	u1Byte				TxRate,
	BOOLEAN				bFinalPoll
	)
{
	BOOLEAN					ret = TRUE;
	u4Byte 					PacketLength = 0;
	unsigned char *pbuf 	= get_wlanllchdr_from_poll(priv);
	DECLARE_TXINSN(txinsn);	

	if(pbuf)
	{
		memset(pbuf, 0, sizeof(struct wlan_hdr));

		ConstructReportPollPacket	(
			priv, 
			RA,
			pbuf,
			&PacketLength,
			BW
			);

		txinsn.q_num = MANAGE_QUE_NUM;
		txinsn.fr_type = _PRE_ALLOCLLCHDR_;		
		txinsn.phdr = pbuf;
		txinsn.hdr_len = PacketLength;
		txinsn.fr_len = 0;
		txinsn.fixed_rate = 1;	
		txinsn.tx_rate = TxRate;	// According to Nr	
		txinsn.ndpa = 1;
		if (bFinalPoll)
			txinsn.SND_pkt_sel = 3;
		else
			txinsn.SND_pkt_sel = 2;

#if 0 //eric-mu
		txinsn.ndpa = 0;
		txinsn.SND_pkt_sel = 0;
#endif

		if (rtl8192cd_wlantx(priv, &txinsn) == CONGESTED) {		
			netif_stop_queue(priv->dev);		
			priv->ext_stats.tx_drops++; 	
			panic_printk("[%s]TX DROP: Congested!\n", __FUNCTION__);
			if (txinsn.phdr)
				release_wlanhdr_to_poll(priv, txinsn.phdr); 			
			if (txinsn.pframe)
				release_mgtbuf_to_poll(priv, txinsn.pframe);
			return 0;	
		}
	}
	else
		ret = FALSE;

	return ret;
}

#if 1 //eric-8822
void input_value_64(unsigned long long *p, unsigned char start, unsigned char end, unsigned int value)
{
	unsigned long long bit_mask = 0;

	if(value > 0) //only none-zero value needs to be assigned 
	{
		if(start == end) //1-bit value
		{
			*p |= BIT(start);
		}
		else
		{
			unsigned char x = 0;
				
			for(x = 0; x<=(end-start); x ++)
				bit_mask |= BIT(x);

			*p |= ((value&bit_mask) << start);	
		}
	}

}
#endif

void issue_action_GROUP_ID(struct rtl8192cd_priv *priv, unsigned char idx)
{
	unsigned char	*pbuf;
	PRT_BEAMFORMING_INFO	pBeamInfo = &(priv->pshare->BeamformingInfo);
	struct stat_info *pstat;

	if (idx < BEAMFORMEE_ENTRY_NUM)
		pstat = pBeamInfo->BeamformeeEntry[idx].pSTA;
	else {
		panic_printk("%s(%d) Error value of idx=%d\n", __FUNCTION__, __LINE__, idx);
		return;
	}

	if(!pstat) {
		panic_printk("%s(%d) pstat is NULL\n", __FUNCTION__, __LINE__);
		return;
	}
	
	DECLARE_TXINSN(txinsn);

	txinsn.q_num = MANAGE_QUE_NUM;
	txinsn.fr_type = _PRE_ALLOCMEM_;
#ifdef P2P_SUPPORT	/*cfg p2p cfg p2p*/
	if(rtk_p2p_is_enabled(priv)){
      	txinsn.tx_rate = _6M_RATE_;
	}else
#endif    
    	txinsn.tx_rate = find_rate(priv, NULL, 0, 1);
#ifndef TX_LOWESTRATE
	txinsn.lowest_tx_rate = txinsn.tx_rate;
#endif	
	txinsn.fixed_rate = 1;

	txinsn.is_GID = 1;

	pbuf = txinsn.pframe = get_mgtbuf_from_poll(priv);
	if (pbuf == NULL)
		goto issue_opm_notification_fail;

	txinsn.phdr = get_wlanhdr_from_poll(priv);
	if (txinsn.phdr == NULL)
		goto issue_opm_notification_fail;

	memset((void *)(txinsn.phdr), 0, sizeof(struct wlan_hdr));

	pbuf[0] = _VHT_ACTION_CATEGORY_ID_;
	pbuf[1] = _VHT_ACTION_GROUPID_ID_;

#if defined(RTK_AC_SUPPORT) 
	if((priv->pshare->rf_ft_var.gidforce != 0xff) && (priv->pshare->rf_ft_var.gidforce <= 63)){
		unsigned long long tmp64 = 0;
		unsigned char gid = priv->pshare->rf_ft_var.gidforce;
		input_value_64(&tmp64, gid, gid, 1);
		tmp64 = cpu_to_le64(tmp64);
		memcpy((pbuf+2), &tmp64, 8);
	}
	else
#endif
	{
		memcpy((pbuf+2), &pBeamInfo->BeamformeeEntry[idx].gid_valid, 8);
	}

	memcpy((pbuf+2+8), &pBeamInfo->BeamformeeEntry[idx].user_position, 16);

	
	txinsn.fr_len = _GROUPID_Frame_Length_;

	SetFrameSubType((txinsn.phdr), WIFI_WMM_ACTION);


	memcpy((void *)GetAddr1Ptr((txinsn.phdr)), pstat->hwaddr, MACADDRLEN);	
	memcpy((void *)GetAddr2Ptr((txinsn.phdr)), GET_MY_HWADDR, MACADDRLEN);
	memcpy((void *)GetAddr3Ptr((txinsn.phdr)), BSSID, MACADDRLEN);


	if ((rtl8192cd_firetx(priv, &txinsn)) == SUCCESS) {
		return;
	}

issue_opm_notification_fail:

	if (txinsn.phdr)
		release_wlanhdr_to_poll(priv, txinsn.phdr);
	if (txinsn.pframe)
		release_mgtbuf_to_poll(priv, txinsn.pframe);
	return;
}

/*******************************************/
/*New SU/MU Sounding Procesure                                 */
/*******************************************/

u1Byte
beamform_GetFirstMUBFeeEntryIdx(
	struct rtl8192cd_priv *priv
	)
{
	PRT_BEAMFORMING_INFO 		pBeamInfo = &(priv->pshare->BeamformingInfo);
	u1Byte					idx = 0xFF;
	BOOLEAN					bFound = FALSE;

	for (idx = 0; idx < BEAMFORMEE_ENTRY_NUM; idx++) {
		if (pBeamInfo->BeamformeeEntry[idx].bUsed && 
			(pBeamInfo->BeamformeeEntry[idx].BeamformEntryCap & BEAMFORM_CAP_VHT_MU_BFEE)) {			
			//RT_TRACE(COMP_BF, DBG_LOUD, ("[%s] idx=%d!\n", __func__, idx));
			bFound = TRUE;
			break;
		}	
	}

	if (!bFound)
		idx = 0xFF;

	return idx;
}

VOID
beamform_checkMUCondition (
	struct rtl8192cd_priv *priv,
	struct stat_info	*pstat

)
{
	if(!pstat)
		return;
	
	if(!pstat->isSendNDPA && (pstat->tx_avarage >= priv->pshare->rf_ft_var.mutp_th_up * 125000)) {
		pstat->isSendNDPA = 1;
	} else if(pstat->isSendNDPA && ((pstat->tx_avarage < priv->pshare->rf_ft_var.mutp_th_lower * 125000))) {
		pstat->isSendNDPA = 0;
	}

	if(!pstat->isRssiApplyMU && pstat->rssi >= priv->pshare->rf_ft_var.murssi_th_up && (pstat->tx_avarage >= priv->pshare->rf_ft_var.mutp_th_up * 125000)) {
		pstat->isRssiApplyMU = 1;
	} else if(pstat->isRssiApplyMU && (pstat->rssi < priv->pshare->rf_ft_var.murssi_th_lower || (pstat->tx_avarage < priv->pshare->rf_ft_var.mutp_th_lower * 125000))) {
		pstat->isRssiApplyMU = 0;
	}
}
#endif

VOID
beamform_UpdateMinSoundingPeriod(
	struct rtl8192cd_priv *priv,
	IN u2Byte			CurBFeePeriod,
	IN BOOLEAN		bBFeeLeave
	)
{
	PRT_BEAMFORMING_INFO 		pBeamInfo = &(priv->pshare->BeamformingInfo);
	u1Byte		i = 0;
	u2Byte		tempMinVal = 0xFFFF;			

	ODM_RT_TRACE(ODMPTR, PHYDM_COMP_TXBF, ODM_DBG_LOUD, ("%s() ==> bBFeeLeave(%d)\n", __func__, bBFeeLeave));

	if(bBFeeLeave)
	{
		// When a BFee left, we need to find the latest min sounding period from the remaining BFees.
		for (i= 0; i < BEAMFORMEE_ENTRY_NUM; i++)
		{
			if (pBeamInfo->BeamformeeEntry[i].bUsed)
			{			
				ODM_RT_TRACE(ODMPTR, PHYDM_COMP_TXBF, ODM_DBG_LOUD, ("Idx(%d) SoundPeriod=%d!\n", i, pBeamInfo->BeamformeeEntry[i].SoundPeriod));
				if(pBeamInfo->BeamformeeEntry[i].SoundPeriod < tempMinVal)
					tempMinVal = pBeamInfo->BeamformeeEntry[i].SoundPeriod;
			}	
		}

		pBeamInfo->SoundingInfoV2.MinSoundingPeriod = (tempMinVal == 0xFFFF)? 0 : tempMinVal;
	}
	else
	{
		if(pBeamInfo->SoundingInfoV2.MinSoundingPeriod == 0)
		{
			// Use current BFee's sounding period as the minimun one.
			pBeamInfo->SoundingInfoV2.MinSoundingPeriod = CurBFeePeriod;	
		}
		else
		{
			// Update min sounding period
			if(CurBFeePeriod < pBeamInfo->SoundingInfoV2.MinSoundingPeriod)
				pBeamInfo->SoundingInfoV2.MinSoundingPeriod = CurBFeePeriod;
		}
	}
}

#if (MU_BEAMFORMING_SUPPORT == 1) 
u1Byte
Beamforming_GetReportPollTxRate(
	struct rtl8192cd_priv *priv
)
{
	if(priv->pshare->rssi_min > 30) // link RSSI > 30%
		return _NSS1_MCS3_RATE_;
	else
		return _NSS1_MCS0_RATE_;

}

VOID
beamform_MUMatch(struct rtl8192cd_priv *priv, PRT_BEAMFORMING_ENTRY	pEntry, u1Byte *pairResult)
{
	struct stat_info	*pstat = pEntry->pSTA;
	if(pstat->muPartner) {
				
				unsigned char e1e2_to_gid[MAX_NUM_BEAMFORMEE_MU][MAX_NUM_BEAMFORMEE_MU]={{2,2,4,6,8,10},
											{1,1,12,14,16,18},
											{3,11,20,20,22,24},
											{5,13,19,19,26,28},
											{7,15,21,25,25,30},
											{9,17,23,27,29,29}};
				u1Byte groupIndex;
				unsigned int dqnum = priv->pshare->rf_ft_var.dqnum;
	
				pstat->muFlagForAMSDU = 1;
				pstat->muPartner->muFlagForAMSDU = 1;
	
				groupIndex = e1e2_to_gid[pEntry->mu_reg_index][pairResult[pEntry->mu_reg_index]] - 1;		
				pstat->mu_rate = ODMPTR->DM_RA_Table.mu1_rate[groupIndex];
				pstat->mu_tx_rate = query_mu_vht_rate(pstat);
				groupIndex = e1e2_to_gid[pairResult[pEntry->mu_reg_index]][pEntry->mu_reg_index] - 1;
				pstat->muPartner->mu_rate = ODMPTR->DM_RA_Table.mu1_rate[groupIndex];
				pstat->muPartner->mu_tx_rate = query_mu_vht_rate(pstat->muPartner);
				
				if(pstat->mu_tx_rate > 0 && pstat->muPartner->mu_tx_rate > 0) {
					pstat->mu_deq_num = 2* dqnum * (pstat->mu_tx_rate) / (pstat->mu_tx_rate + pstat->muPartner->mu_tx_rate);
					pstat->muPartner->mu_deq_num = 2* dqnum - pstat->mu_deq_num;
				} else {
					pstat->mu_deq_num = dqnum;
					pstat->muPartner->mu_deq_num = dqnum;
				}
				
			}

}

VOID
beamform_MUGrouping(
	struct rtl8192cd_priv *priv,
	u2Byte bitmap,
	u1Byte *pairResult
)
{
	u1Byte					candidateIdx = -1;
	PRT_BEAMFORMING_ENTRY	CandidateEntry[MAX_NUM_BEAMFORMEE_MU], tmpEntry;
	u1Byte					numCandidate;
	PRT_BEAMFORMING_ENTRY	pEntry = NULL;
	PRT_BEAMFORMING_INFO	pBeamInfo = &(priv->pshare->BeamformingInfo);
	struct stat_info	*pstat;
	u1Byte	i, j;
	u1Byte  isCandidate;
	u4Byte bitmap_mask[6]={0x1f,0x1e1,0xe22,0x3244,0x5488,0x6910};

	for(i=0; i < MAX_NUM_BEAMFORMEE_MU; i++)
		pairResult[i] = 0;
	
	numCandidate = 0;
	for(i = 0; i < BEAMFORMEE_ENTRY_NUM; i++) {
		pEntry = &(pBeamInfo->BeamformeeEntry[i]);
		if( pEntry->bUsed && (pEntry->is_mu_sta == TXBF_TYPE_MU) && pEntry->mu_reg_index < MAX_NUM_BEAMFORMEE_MU) {
			//pstat = get_stainfo(priv, pBeamInfo->BeamformeeEntry[i].MacAddr);
			pstat = pEntry->pSTA;
			if(pstat) {
				pstat->muPartner = NULL;
				if(pstat->isSendNDPA && pstat->isRssiApplyMU)
					CandidateEntry[numCandidate++] = pEntry;
			}
		}
	}

	if(numCandidate > MAX_NUM_BEAMFORMEE_MU) {
		panic_printk("%s(%d) Error !! numCandidate > MAX_BEAMFORMEE_MU\n", __FUNCTION__, __LINE__);
		return;
	}

	if(numCandidate >=4
#ifdef CONFIG_VERIWAVE_MU_CHECK
		&& !priv->pshare->rf_ft_var.isExistVeriwaveSTA
#endif
		) { // sort by rssi
		for(i = 0; i< numCandidate - 1; i++) {
			for(j = i; j < numCandidate; j++) {
				if(CandidateEntry[i]->pSTA->rssi < CandidateEntry[j]->pSTA->rssi) {
					tmpEntry = CandidateEntry[i];
					CandidateEntry[i] = CandidateEntry[j];
					CandidateEntry[j] = tmpEntry;
				}
			}
		}
	}

	{
		unsigned int numCase, numPair;
		int bestCaseIdx;
		unsigned int numMatch, maxMatch=0;
		unsigned int idx1,idx2;
		unsigned char match6[15][3][2] = {
			{{0,1},{2,3},{4,5}}, {{0,1},{2,4},{3,5}},{{0,1},{2,5},{3,4}},
			{{0,2},{1,3},{4,5}}, {{0,2},{1,4},{3,5}}, {{0,2},{1,5},{3,4}},
			{{0,3},{1,2},{4,5}}, {{0,3},{1,4},{2,5}}, {{0,3},{1,5},{2,4}}, 
			{{0,4},{1,2},{3,5}}, {{0,4},{1,3},{2,5}}, {{0,4},{1,5},{2,3}}, 
			{{0,5},{1,2},{3,4}}, {{0,5},{1,3},{2,4}}, {{0,5},{1,4},{2,3}} 
			};
		unsigned char match5[15][2][2] = {
			{{0,1},{2,3}}, {{0,1},{2,4}}, {{0,1},{3,4}},
			{{0,2},{1,3}}, {{0,2},{1,4}}, {{0,2},{3,4}},
			{{0,3},{1,2}}, {{0,3},{1,4}}, {{0,3},{2,4}},
			{{0,4},{1,2}}, {{0,4},{1,3}}, {{0,4},{2,3}},
			{{1,2},{3,4}}, {{1,3},{2,4}}, {{1,4},{2,3}}
		};
		unsigned char match4[3][2][2] = {
			{{0,1},{2,3}},{{0,2},{1,3}},{{0,3},{1,2}}
		};
		unsigned char match3[3][1][2] = {
			{{0,1}},{{0,2}},{{1,2}}
		};
		unsigned char match2[1][1][2] = {
			{{0,1}}
		};
				
		if(numCandidate >=2) {
			maxMatch = 0;
			numMatch = 0;
			bestCaseIdx = -1;
			if(numCandidate == 6) 
				numPair = 3;
			else if(numCandidate == 4 || numCandidate == 5)
				numPair = 2;
			else
				numPair = 1;

			if(numCandidate == 6 || numCandidate == 5) 
				numCase = 15;
			else if(numCandidate == 4 || numCandidate == 3) 
				numCase = 3;
			else
				numCase = 1;

			for(i=0;i<numCase;i++) {
				numMatch = 0;
				for(j=0;j<numPair;j++) {
					idx1 = (numCandidate == 2)? match2[i][j][0]:(numCandidate == 3)? match3[i][j][0]:(numCandidate == 4)? match4[i][j][0]:(numCandidate == 5)?match5[i][j][0]:match6[i][j][0];
					idx2 = (numCandidate == 2)? match2[i][j][1]:(numCandidate == 3)? match3[i][j][1]:(numCandidate == 4)? match4[i][j][1]:(numCandidate == 5)?match5[i][j][1]:match6[i][j][1];
					pEntry = CandidateEntry[idx1];
					pstat = pEntry->pSTA;
					isCandidate = (pEntry->mu_reg_index != CandidateEntry[idx2]->mu_reg_index) && 
								  (pEntry->priv == CandidateEntry[idx2]->priv) &&
								  (bitmap & bitmap_mask[pEntry->mu_reg_index] & bitmap_mask[CandidateEntry[idx2]->mu_reg_index]);					
					if(isCandidate) numMatch++;
				}
				if(numMatch > maxMatch) {
					bestCaseIdx = i;
					maxMatch = numMatch;
					if(maxMatch == numPair)
						break;
				}
			}
			if(bestCaseIdx >= 0) {
				for(j=0;j<numPair;j++) {
					idx1 = (numCandidate == 2)? match2[bestCaseIdx][j][0]:(numCandidate == 3)? match3[bestCaseIdx][j][0]:(numCandidate == 4)? match4[bestCaseIdx][j][0]:(numCandidate == 5)?match5[bestCaseIdx][j][0]:match6[bestCaseIdx][j][0];
					idx2 = (numCandidate == 2)? match2[bestCaseIdx][j][1]:(numCandidate == 3)? match3[bestCaseIdx][j][1]:(numCandidate == 4)? match4[bestCaseIdx][j][1]:(numCandidate == 5)?match5[bestCaseIdx][j][1]:match6[bestCaseIdx][j][1];
					pEntry = CandidateEntry[idx1];
					pstat = pEntry->pSTA;
					if(pEntry == NULL || CandidateEntry[idx2] == NULL) {
						GDEBUG("numCandidate = %d, idx1=%d, idx2=%d\n", numCandidate, idx1, idx2);
					}
					isCandidate = (pEntry->mu_reg_index != CandidateEntry[idx2]->mu_reg_index) && 
								  (pEntry->priv == CandidateEntry[idx2]->priv) &&
								  (bitmap & bitmap_mask[pEntry->mu_reg_index] & bitmap_mask[CandidateEntry[idx2]->mu_reg_index]);
					if(isCandidate) {				
						pstat->muPartner = CandidateEntry[idx2]->pSTA;				
						pstat->muPartner->muPartner = pstat;
						pairResult[pEntry->mu_reg_index] = CandidateEntry[idx2]->mu_reg_index;
						pairResult[CandidateEntry[idx2]->mu_reg_index] = pEntry->mu_reg_index;
						beamform_MUMatch(priv,pEntry, pairResult);
					}
				}
			}
		}		
	}
} 
#endif
RT_STATUS
beamform_GetSoundingList(
	struct rtl8192cd_priv *priv
)
{
	PRT_BEAMFORMING_INFO 		pBeamInfo = &(priv->pshare->BeamformingInfo);
	u1Byte						i, MUIdx = 0, SUIdx = 0;
	RT_STATUS					rtStatus = RT_STATUS_SUCCESS;

	//
	// Add MU BFee list first because MU priority is higher than SU
	//
	for(i=0; i<BEAMFORMEE_ENTRY_NUM; i++)
	{
		if(pBeamInfo->BeamformeeEntry[i].bUsed)
		{
			if(pBeamInfo->BeamformeeEntry[i].HwState != BEAMFORM_ENTRY_HW_STATE_ADDED)
			{		
				ODM_RT_TRACE(ODMPTR, PHYDM_COMP_TXBF, ODM_DBG_LOUD, ("[%s]: Invalid BFee idx(%d) Hw state=%d\n", __func__, i, pBeamInfo->BeamformeeEntry[i].HwState));
				return RT_STATUS_FAILURE;
			}
			else
			{
				// Decrease BFee's SoundCnt per period.
				if(pBeamInfo->BeamformeeEntry[i].SoundCnt > 0)
					pBeamInfo->BeamformeeEntry[i].SoundCnt--;
			}

#if 1 //eric-mu
{
			PRT_SOUNDING_INFOV2			pSoundingInfo = &(pBeamInfo->SoundingInfoV2);
			
			if(pSoundingInfo->State == SOUNDING_STATE_INIT) 
				pBeamInfo->BeamformeeEntry[i].SoundCnt = 0;
}
#endif
		
			//
			// <tynli_Note> If the STA supports MU BFee capability then we add it to MUSoundingList directly
			//	because we can only sound one STA by unicast NDPA with MU cap enabled to get correct channel info.
			//	Suggested by BB team Luke Lee. 2015.11.25.
			//
			// MU BFee
			//ODM_RT_TRACE(ODMPTR, PHYDM_COMP_TXBF, ODM_DBG_LOUD, ("[%s]: idx = %d, BeamformEntryCap = 0x%x, SoundCnt = %d\n", __func__, i, pBeamInfo->BeamformeeEntry[i].BeamformEntryCap, pBeamInfo->BeamformeeEntry[i].SoundCnt));
#if (MU_BEAMFORMING_SUPPORT == 1)			
			if(pBeamInfo->BeamformeeEntry[i].BeamformEntryCap & (BEAMFORM_CAP_VHT_MU_BFER)) //eric-mu
			{
				if(MUIdx < MAX_NUM_BEAMFORMEE_MU)
				{
					if(pBeamInfo->BeamformeeEntry[i].SoundCnt == 0)
					{
						pBeamInfo->BeamformeeEntry[i].bCandidateSoundingPeer = TRUE;
						//panic_printk("eric-mu [%s][%d] %d %d\n", __FUNCTION__, __LINE__, pBeamInfo->BeamformeeEntry[i].SoundPeriod, pBeamInfo->SoundingInfoV2.MinSoundingPeriod);
						pBeamInfo->BeamformeeEntry[i].SoundCnt = GetInitSoundCnt(pBeamInfo->BeamformeeEntry[i].SoundPeriod, pBeamInfo->SoundingInfoV2.MinSoundingPeriod);
						pBeamInfo->SoundingInfoV2.MUSoundingList[MUIdx] = i;
						MUIdx++;
					}
				}
			}
			else
#endif				
			{
				// <1> Support SU BFee Cap
				if(pBeamInfo->BeamformeeEntry[i].BeamformEntryCap & (BEAMFORMER_CAP_VHT_SU|BEAMFORMER_CAP_HT_EXPLICIT)) //eric-mu
				{
					if(SUIdx < MAX_NUM_BEAMFORMEE_SU)
					{
						// <2> If the remain count is 0, then it can be sounded at this time.
						if(pBeamInfo->BeamformeeEntry[i].SoundCnt == 0)
						{
							pBeamInfo->BeamformeeEntry[i].bCandidateSoundingPeer = TRUE;							
							pBeamInfo->BeamformeeEntry[i].SoundCnt = GetInitSoundCnt(pBeamInfo->BeamformeeEntry[i].SoundPeriod, pBeamInfo->SoundingInfoV2.MinSoundingPeriod);
							pBeamInfo->SoundingInfoV2.SUSoundingList[SUIdx] = i;
							SUIdx++;
						}
					}
				}
			}
		}
	}

#if (MU_BEAMFORMING_SUPPORT == 1)
	pBeamInfo->SoundingInfoV2.CandidateMUBFeeCnt = MUIdx;
#endif


	if(SUIdx + MUIdx == 0)
		rtStatus = RT_STATUS_FAILURE;

	//ODM_RT_TRACE(ODMPTR, PHYDM_COMP_TXBF, ODM_DBG_LOUD, ("[%s]: There are %d SU and %d MU BFees in this sounding period\n", __func__, SUIdx, MUIdx));
	//panic_printk("eric-mu [%s] %d SU and %d MU BFees status = %d \n", 
		//__FUNCTION__, SUIdx, MUIdx, rtStatus);

	return rtStatus;
}

u1Byte
beamform_GetSUSoundingIdx(
	struct rtl8192cd_priv *priv
)
{
	PRT_BEAMFORMING_INFO 		pBeamInfo = &(priv->pshare->BeamformingInfo);
	PRT_SOUNDING_INFOV2			pSoundingInfo = &(pBeamInfo->SoundingInfoV2);
	u1Byte						i, idx = BEAMFORMEE_ENTRY_NUM;

	//
	// Get non-sound SU BFee index
	//
	for(idx=0; idx<BEAMFORMEE_ENTRY_NUM; idx++)
	{
		if(pBeamInfo->BeamformeeEntry[idx].bUsed && (!pBeamInfo->BeamformeeEntry[idx].bSound) &&
			pBeamInfo->BeamformeeEntry[idx].bCandidateSoundingPeer)
		{ //eric-txbf ??
			if((pBeamInfo->BeamformeeEntry[idx].BeamformEntryCap & (BEAMFORMEE_CAP_VHT_SU|BEAMFORMEE_CAP_HT_EXPLICIT|BEAMFORMER_CAP_VHT_SU))
#if (MU_BEAMFORMING_SUPPORT == 1)				
				&& !(pBeamInfo->BeamformeeEntry[idx].BeamformEntryCap & (BEAMFORM_CAP_VHT_MU_BFER))
#endif				
			)
			{
				pBeamInfo->BeamformeeEntry[idx].bSound = TRUE;
				break;
			}
		}
	}
	return idx;

}

BOOLEAN
beamform_IsLastSoundingPeer(
	struct rtl8192cd_priv *priv
)
{
	PRT_BEAMFORMING_INFO 		pBeamInfo = &(priv->pshare->BeamformingInfo);
	u1Byte						idx;
	BOOLEAN						bLastSoundPeer = TRUE;

	for(idx=0; idx<BEAMFORMEE_ENTRY_NUM; idx++)
	{
		if(pBeamInfo->BeamformeeEntry[idx].bUsed && (!pBeamInfo->BeamformeeEntry[idx].bSound) &&
			pBeamInfo->BeamformeeEntry[idx].bCandidateSoundingPeer)
		{
			bLastSoundPeer = FALSE;
			break;
		}
	}

	//ODM_RT_TRACE(ODMPTR, PHYDM_COMP_TXBF, ODM_DBG_LOUD, ("[%s]: %d\n", __func__, bLastSoundPeer));

	return bLastSoundPeer;
}

VOID
beamform_ResetSoundingVars(
	struct rtl8192cd_priv *priv
)
{
	PRT_BEAMFORMING_INFO 		pBeamInfo = &(priv->pshare->BeamformingInfo);
	PRT_SOUNDING_INFOV2			pSoundingInfo = &(pBeamInfo->SoundingInfoV2);
	u1Byte						idx;
	
	pSoundingInfo->SUSoundNumPerPeriod = 0;
#if (MU_BEAMFORMING_SUPPORT == 1)	
	pSoundingInfo->MUSoundNumPerPeriod = 0;
#endif

	// Clear bSound flag for the new period.
	for(idx=0; idx<BEAMFORMEE_ENTRY_NUM; idx++)
	{
		if(pBeamInfo->BeamformeeEntry[idx].bUsed && pBeamInfo->BeamformeeEntry[idx].bSound)
		{
			pBeamInfo->BeamformeeEntry[idx].bSound = FALSE;
			pBeamInfo->BeamformeeEntry[idx].bCandidateSoundingPeer = FALSE;
		}
	}
}

VOID
beamform_DecreaseBFeeSoundCnt(
	struct rtl8192cd_priv *priv
)
{
	PRT_BEAMFORMING_INFO 		pBeamInfo = &(priv->pshare->BeamformingInfo);
	u1Byte						idx;
	
	// Clear bSound flag for the new period.
	for(idx=0; idx<BEAMFORMEE_ENTRY_NUM; idx++)
	{
		if(pBeamInfo->BeamformeeEntry[idx].bUsed && (!pBeamInfo->BeamformeeEntry[idx].bSound))
		{
			if(pBeamInfo->BeamformeeEntry[idx].SoundCnt > 0)
				pBeamInfo->BeamformeeEntry[idx].SoundCnt--;
		}
	}
}

VOID 
beamform_SoundingTimerCallback(
	struct rtl8192cd_priv *priv
    )
{
	struct rtl8192cd_priv *priv_this;
	PRT_BEAMFORMING_INFO 		pBeamInfo = &(priv->pshare->BeamformingInfo);
	PRT_SOUNDING_INFOV2			pSoundingInfo = &(pBeamInfo->SoundingInfoV2);
	BOOLEAN					ret = TRUE;
	PRT_BEAMFORMING_ENTRY		pEntry = NULL;
#if (MU_BEAMFORMING_SUPPORT == 1) 	
	PRT_BEAMFORMING_ENTRY		pEntry_poll[TOTAL_BEAMFORMEE_ENTRY_NUM];
	u1Byte						cnt, cnt_valid;
	struct stat_info *pstat;
#endif	
	u1Byte			SUSoundingIdx = BEAMFORMEE_ENTRY_NUM;
	u4Byte			TimeoutPeriod = 0;
	BOOLEAN			bSetTimeoutTimer = FALSE;
	RT_STATUS					rtStatus = RT_STATUS_SUCCESS;
	static u2Byte		WaitCnt = 0;
	u1Byte					NDPTxRate, ReportPollRate;
	
	
	ODM_RT_TRACE(ODMPTR, PHYDM_COMP_TXBF, ODM_DBG_LOUD, ("[%s] ===> pSoundingInfo->State = %d\n", __func__, pSoundingInfo->State));	
	
	if(pSoundingInfo->State != SOUNDING_STATE_INIT && pSoundingInfo->State != SOUNDING_STATE_SU_SOUNDDOWN
		&& pSoundingInfo->State != SOUNDING_STATE_MU_SOUNDDOWN && pSoundingInfo->State != SOUNDING_STATE_SOUNDING_TIMEOUT)
	{
		ODM_RT_TRACE(ODMPTR, PHYDM_COMP_TXBF, ODM_DBG_LOUD, ("[%s]: Invalid State(%d) and return!\n", __func__, pSoundingInfo->State));
	}

	do
	{
		if(pSoundingInfo->State == SOUNDING_STATE_INIT)
		{
			// Init Var
			beamform_ResetSoundingVars(priv);
			
			Beamforming_NDPARate(priv, 0, 0, 0);	// soundingpreiod only for debug, use 0 for all case
			
			// Get the sounding list of this sounding period
			rtStatus = beamform_GetSoundingList(priv);
			if(rtStatus == RT_STATUS_FAILURE)
			{
				WaitCnt=0;
				pSoundingInfo->State = SOUNDING_STATE_NONE;
				ODM_RT_TRACE(ODMPTR, PHYDM_COMP_TXBF, ODM_DBG_LOUD, ("[%s]: No BFees found. Set to SOUNDING_STATE_NONE \n", __func__));
				break;
			}
			else if(rtStatus == RT_STATUS_FAILURE)
			{			
				// Set the next sounding timer
				if(WaitCnt < 500)
				{
					WaitCnt++;
					//PlatformSetTimer(Adapter, &(pBeamInfo->BFSoundingTimer), 2); // 2ms
					ODM_SetTimer(ODMPTR, &pBeamInfo->BeamformingTimer, 2); 
				} 
				else
				{
					WaitCnt=0;
					pSoundingInfo->State = SOUNDING_STATE_NONE;
					ODM_RT_TRACE(ODMPTR, PHYDM_COMP_TXBF, ODM_DBG_LOUD, ("[%s]: Wait changing Hw state timeout!!. Set to SOUNDING_STATE_NONE \n", __func__));
				}
				break;
			}
			else if(rtStatus == RT_STATUS_SUCCESS)
			{
				WaitCnt=0;
				// Decrease All BFees' SoundCnt per period.
				//beamform_DecreaseBFeeSoundCnt(Adapter);
			}
			else
			{
				WaitCnt=0;
				ODM_RT_TRACE(ODMPTR, PHYDM_COMP_TXBF, ODM_DBG_LOUD, ("[%s]: Unkown state! \n", __func__));
				break;
			}
		}
		
		SUSoundingIdx = beamform_GetSUSoundingIdx(priv);
		
		if((SUSoundingIdx < BEAMFORMEE_ENTRY_NUM))
		{
			pEntry = &(pBeamInfo->BeamformeeEntry[SUSoundingIdx]);
#if (MU_BEAMFORMING_SUPPORT == 1)		
			if(!pEntry->is_mu_sta)
#endif				
			{ // SU Case
				pSoundingInfo->SUBFeeCurIdx = SUSoundingIdx;
				// Set to sounding start state
				pSoundingInfo->State = SOUNDING_STATE_SU_START;
				//ODM_RT_TRACE(ODMPTR, PHYDM_COMP_TXBF, ODM_DBG_LOUD, ("[%s]: Set to SOUNDING_STATE_SU_START \n", __func__));			

				// Reset sounding timeout flag for the new sounding.
				pEntry->bSoundingTimeout = FALSE;				
				
				// Start SU sounding
				if (pEntry->pSTA && !(pEntry->pSTA->state & WIFI_SLEEP_STATE)) {			
					if(pEntry->BeamformEntryCap & BEAMFORMER_CAP_HT_EXPLICIT) { //eric-txbf
						if (beamforming_by_rate_souding_decision(priv, pEntry)) {
							NDPTxRate = Beamforming_GetHTNDPTxRate(priv, pEntry->CompSteeringNumofBFer);
							ret = SendHTNDPAPacket(priv, pEntry->MacAddr, pEntry->BW, NDPTxRate);
							if (ret)
								pEntry->Sounding_En = 1;
							else
								pEntry->Sounding_En = 0;
						} 
						
						if(pEntry->Sounding_En == 0)
							Beamform_SoundingDown(priv,0);

					} else if(pEntry->BeamformEntryCap & BEAMFORMER_CAP_VHT_SU) { //eric-txbf
						if (beamforming_by_rate_souding_decision(priv, pEntry)) {
							NDPTxRate = Beamforming_GetVHTNDPTxRate(priv, pEntry->CompSteeringNumofBFer);
							ret = SendVHTNDPAPacket(priv,pEntry->MacAddr, pEntry->AID, pEntry->BW, NDPTxRate);
							if (ret)
								pEntry->Sounding_En = 1;
							else
								pEntry->Sounding_En = 0;
						} 

						if(pEntry->Sounding_En == 0)
							Beamform_SoundingDown(priv,0);
					}
				}	
				pSoundingInfo->SUSoundNumPerPeriod++;
				// Count the next period
				TimeoutPeriod = SU_SOUNDING_TIMEOUT;
				bSetTimeoutTimer = TRUE;
			}			
		}
#if (MU_BEAMFORMING_SUPPORT == 1)		
		else
		{
			// If there is no SU BFee then find MU BFee and perform MU sounding.
			//
			// <tynli_note> Need to check the MU starting condition. 2015.12.15.
			//
			if(pSoundingInfo->CandidateMUBFeeCnt > 0)
			{				
				pSoundingInfo->State = SOUNDING_STATE_MU_START;
				ODM_RT_TRACE(ODMPTR, PHYDM_COMP_TXBF, ODM_DBG_LOUD, ("[%s]: Set to SOUNDING_STATE_MU_START \n", __func__));
	
				// Update MU BFee info
				u1Byte	i;
				u1Byte				idx;
				
				if(pSoundingInfo->CandidateMUBFeeCnt >= 1)
				{ /* More than 1 MU STA*/											
					unsigned long x;									

					SAVE_INT_AND_CLI(x);

					RTL_W8(0x719, 0x82);
					
					cnt = 0;
					priv_this = NULL;
					
					for(idx = 0; idx < BEAMFORMEE_ENTRY_NUM; idx++)
					{		
						pEntry = &(pBeamInfo->BeamformeeEntry[idx]);
						
						if(pEntry->bUsed && pEntry->pSTA)
							beamform_checkMUCondition(priv,pEntry->pSTA);
						else
							continue;
						
						if( (pEntry->is_mu_sta) && 							
							(pEntry->bSound == FALSE)) 
						{
							if((pEntry->pSTA->isSendNDPA)) {
								if(priv_this == NULL)
									priv_this = pEntry->priv;								

								if(priv_this == pEntry->priv) {
									pEntry->bSound = TRUE;
									pEntry_poll[cnt] = pEntry;
									cnt++;
								}
							} else {
								pEntry->bSound = TRUE;
							}
							
						}
					}

#ifdef CONFIG_VERIWAVE_MU_CHECK
					if(cnt >=3) {
						cnt_valid = 0;
						for(idx = 0; idx < cnt; idx++) {
							if(!pEntry_poll[idx]->pSTA->isVeriwaveInValidSTA) {
								pEntry_poll[cnt_valid] = pEntry_poll[idx];
								cnt_valid++;
							}
						}
						cnt = cnt_valid;
					}
#endif
					ODM_RT_TRACE(ODMPTR, PHYDM_COMP_TXBF, ODM_DBG_LOUD, ("[%s]: cnt=%d \n", __func__, cnt));
					if((cnt == 1) && (AC_SIGMA_MODE == AC_SIGMA_NONE)) { //start mu sounding when 2 sta connected  for 11ac logo r2
						pEntry = pEntry_poll[0];

						NDPTxRate = Beamforming_GetVHTNDPTxRate(priv, pEntry->CompSteeringNumofBFer);
							
						SendVHTNDPAPacket_MU(pEntry->priv, pEntry->MacAddr, pEntry->AID, pEntry->BW, NDPTxRate, 1);	
					} else if (cnt > 1) {
						ReportPollRate = Beamforming_GetReportPollTxRate(priv);

						for(idx = 0; idx < cnt; idx++) {
							pEntry = pEntry_poll[idx];
								
							if(idx == 0)
								SendVHTNDPAPacket_MU(pEntry->priv, pEntry->MacAddr, pEntry->AID, pEntry->BW, _NSS2_MCS0_RATE_, 0);
							else if(idx == cnt-1)
								SendReportPollPacket(pEntry->priv, pEntry->MacAddr, pEntry->BW, ReportPollRate,TRUE);	
							else
								SendReportPollPacket(pEntry->priv, pEntry->MacAddr, pEntry->BW, ReportPollRate,FALSE);									
						}
							
					} else if (cnt == 0) {						
						Beamforming_SetBeamFormStatus(priv, 0);			
#ifdef CONFIG_VERIWAVE_MU_CHECK							
						pSoundingInfo->MinSoundingPeriod = priv->pshare->rf_ft_var.mutime;						
						for(idx = 0; idx < BEAMFORMEE_ENTRY_NUM; idx++)
						{		
							pEntry = &(pBeamInfo->BeamformeeEntry[idx]);
							if(pEntry->pSTA)
							pEntry->pSTA->isVeriwaveInValidSTA = 0;
						}
#endif						
					}
					RESTORE_INT(x);
					
				
				}
				pSoundingInfo->MUSoundNumPerPeriod++;					
				TimeoutPeriod = MU_SOUNDING_TIMEOUT;
				bSetTimeoutTimer = TRUE;
			}			
		}
#endif //#if (MU_BEAMFORMING_SUPPORT == 1)		
		

		// Set the next timer
		if(bSetTimeoutTimer)
		{						
			ODM_SetTimer(ODMPTR, &pBeamInfo->BFSoundingTimeoutTimer, TimeoutPeriod); //eric-mu
			break;
			//ODM_RT_TRACE(ODMPTR, PHYDM_COMP_TXBF, ODM_DBG_LOUD, ("%s(): Set Timeout Timer %d\n", __func__, TimeoutPeriod));
			
		}
	} while(FALSE);

	//ODM_RT_TRACE(ODMPTR, PHYDM_COMP_TXBF, ODM_DBG_LOUD, ("%s(): <===\n", __func__));

}

VOID 
beamform_SoundingTimeout(
    struct rtl8192cd_priv *priv
    )
{
	PRT_BEAMFORMING_INFO 		pBeamInfo = &(priv->pshare->BeamformingInfo);
	PRT_SOUNDING_INFOV2			pSoundingInfo = &(pBeamInfo->SoundingInfoV2);
	PRT_BEAMFORMING_ENTRY	pBFeeEntry;
	u4Byte			NextSoundPeriod = 0;
	u1Byte			DelayTime;
	//ODM_RT_TRACE(ODMPTR, PHYDM_COMP_TXBF, ODM_DBG_LOUD, ("%s(): ==>\n", __func__));

	if(pSoundingInfo->State == SOUNDING_STATE_SU_START)
	{
		if(pSoundingInfo->SUBFeeCurIdx < BEAMFORMEE_ENTRY_NUM)
			pBFeeEntry = &(pBeamInfo->BeamformeeEntry[pSoundingInfo->SUBFeeCurIdx]);
		else {
			ODM_RT_TRACE(ODMPTR, PHYDM_COMP_TXBF, ODM_DBG_LOUD, ("[%s]: Error Value of SUBFeeCurIdx = %d!\n", __func__, pSoundingInfo->SUBFeeCurIdx));
			return;
		}
			
		
		if(!(pBFeeEntry->BeamformEntryCap & BEAMFORM_CAP_VHT_MU_BFEE) && 
			(pBFeeEntry->BeamformEntryCap & (BEAMFORMEE_CAP_VHT_SU|BEAMFORMEE_CAP_HT_EXPLICIT)))
		{	// SU BFee
			pBFeeEntry->bSoundingTimeout = TRUE;
			ODM_RT_TRACE(ODMPTR, PHYDM_COMP_TXBF, ODM_DBG_LOUD, ("[%s]: The BFee entry[%d] is set to bSoundingTimeout!\n", __func__, pSoundingInfo->SUBFeeCurIdx));
			
			pSoundingInfo->State = SOUNDING_STATE_SOUNDING_TIMEOUT;
			ODM_RT_TRACE(ODMPTR, PHYDM_COMP_TXBF, ODM_DBG_LOUD, ("[%s]: Set to SU SOUNDING_STATE_SOUNDING_TIMEOUT \n", __func__)); 	
		}
	}
	else if(pSoundingInfo->State == SOUNDING_STATE_MU_START)
	{
		pSoundingInfo->State = SOUNDING_STATE_SOUNDING_TIMEOUT;
		ODM_RT_TRACE(ODMPTR, PHYDM_COMP_TXBF, ODM_DBG_LOUD, ("[%s]: Set to MU SOUNDING_STATE_SOUNDING_TIMEOUT \n", __func__));		
	}

	if(!beamform_IsLastSoundingPeer(priv))
	{				
		ODM_RT_TRACE(ODMPTR, PHYDM_COMP_TXBF, ODM_DBG_LOUD, ("[%s]: Set next sounding start time to %d ms \n", __func__, NextSoundPeriod));
		beamform_SoundingTimerCallback(priv);
	}
	else {
		if(pBeamInfo->beamformee_su_cnt > 0 
#if (MU_BEAMFORMING_SUPPORT == 1)			
			|| pBeamInfo->beamformee_mu_cnt > 0
#endif			
		)
		{			
			DelayTime = SU_SOUNDING_TIMEOUT*pSoundingInfo->SUSoundNumPerPeriod;
			
#if (MU_BEAMFORMING_SUPPORT == 1)				
			DelayTime += MU_SOUNDING_TIMEOUT*pSoundingInfo->MUSoundNumPerPeriod;
#endif		
			
			NextSoundPeriod = (pSoundingInfo->MinSoundingPeriod > DelayTime)?(pSoundingInfo->MinSoundingPeriod - DelayTime):0 ;
		
			pSoundingInfo->State = SOUNDING_STATE_INIT;

			ODM_RT_TRACE(ODMPTR, PHYDM_COMP_TXBF, ODM_DBG_LOUD, ("[%s]: Set to SOUNDING_STATE_INIT \n", __func__));
			ODM_RT_TRACE(ODMPTR, PHYDM_COMP_TXBF, ODM_DBG_LOUD, ("[%s]: Set next sounding start time to %d ms \n", __func__, NextSoundPeriod));		
			
			if(NextSoundPeriod)
				ODM_SetTimer(ODMPTR, &pBeamInfo->BeamformingTimer, NextSoundPeriod);
			else
				beamform_SoundingTimerCallback(priv);
		}
		else
		{
			pSoundingInfo->State = SOUNDING_STATE_NONE;
			ODM_RT_TRACE(ODMPTR, PHYDM_COMP_TXBF, ODM_DBG_LOUD, ("[%s]: Set to SOUNDING_STATE_NONE \n", __func__));
		}
	}
}

VOID
Beamform_SoundingDown(
	struct rtl8192cd_priv *priv,
	IN BOOLEAN		Status	
	)
{	

	PRT_BEAMFORMING_INFO 		pBeamInfo = &(priv->pshare->BeamformingInfo);
	PRT_SOUNDING_INFOV2		pSoundingInfo = &(pBeamInfo->SoundingInfoV2);
	PRT_BEAMFORMING_ENTRY		pEntry = NULL;

	ODM_RT_TRACE(ODMPTR, PHYDM_COMP_TXBF, ODM_DBG_TRACE, ("%s(): sound down, State=%x, Status=%d\n", __func__, pSoundingInfo->State, Status));

#if (MU_BEAMFORMING_SUPPORT == 1)
	if (pSoundingInfo->State == SOUNDING_STATE_MU_START)
	{		
		//ODM_RT_TRACE(ODMPTR, PHYDM_COMP_TXBF, ODM_DBG_TRACE, ("%s: MU sounding done\n", __func__));
		//pBeamInfo->is_mu_sounding_in_progress = FALSE;
		pSoundingInfo->State = SOUNDING_STATE_MU_SOUNDDOWN;
		//ODM_RT_TRACE(ODMPTR, PHYDM_COMP_TXBF, ODM_DBG_TRACE, ("[%s]: Set to SOUNDING_STATE_MU_SOUNDDOWN \n", __func__));
		pBeamInfo->SetHalSoundownOnDemandCnt++;
		//Beamform_SetHwConfig(pAdapter, BEAMFORM_SET_HW_TYPE_SOUND_DOWN);	
		Beamforming_SetBeamFormStatus(priv, 0);
		//HalComTxbf_Set(pDM_Odm, TXBF_SET_SOUNDING_STATUS, (pu1Byte)&(pBeamInfo->BeamformeeCurIdx));
	} 
	else 
#endif		
	if(pSoundingInfo->State == SOUNDING_STATE_SU_START)
	{
		ODM_RT_TRACE(ODMPTR, PHYDM_COMP_TXBF, ODM_DBG_TRACE, ("%s(): SU entry[%d] sound down\n", __func__, pSoundingInfo->SUBFeeCurIdx));

		if(pSoundingInfo->SUBFeeCurIdx < BEAMFORMEE_ENTRY_NUM)
			pEntry = &(pBeamInfo->BeamformeeEntry[pSoundingInfo->SUBFeeCurIdx]);
		else {
			pSoundingInfo->State = SOUNDING_STATE_SU_SOUNDDOWN;
			ODM_RT_TRACE(ODMPTR, PHYDM_COMP_TXBF, ODM_DBG_LOUD, ("[%s]: Error Value of SUBFeeCurIdx = %d!\n", __func__, pSoundingInfo->SUBFeeCurIdx));
			return;
		}
		
		pSoundingInfo->State = SOUNDING_STATE_SU_SOUNDDOWN;

		if (!pEntry->bUsed) {
			ODM_RT_TRACE(ODMPTR, PHYDM_COMP_TXBF, ODM_DBG_TRACE, ("[%s] WARNING! The entry[%d] bUsed=FALSE\n", __func__, pSoundingInfo->SUBFeeCurIdx));
			return;
		}
			
		//
		// <tynli_note> pEntry->bSoundingTimeout this flag still cannot avoid old sound down event happens in the new sounding period. 2015.12.10
		//
		if (pEntry->bSoundingTimeout) {
			ODM_RT_TRACE(ODMPTR, PHYDM_COMP_TXBF, ODM_DBG_TRACE, ("[%s] WARNING! The entry[%d] is bSoundingTimeout!\n", __func__, pSoundingInfo->SUBFeeCurIdx));
			pEntry->bSoundingTimeout = FALSE;
			return;
		}


		if (Status == 1)	// success
		{
			pEntry->LogStatusFailCnt = 0;
			pEntry->BeamformEntryState = BEAMFORMING_ENTRY_STATE_PROGRESSED; //eric-txbf
			//HalComTxbf_Set(pDM_Odm, TXBF_SET_SOUNDING_STATUS, (pu1Byte)&(pBeamInfo->BeamformeeCurIdx));
			pEntry->bDeleteSounding = FALSE;
			pBeamInfo->SetHalSoundownOnDemandCnt++;
			//Beamform_SetHwConfig(pAdapter, BEAMFORM_SET_HW_TYPE_SOUND_DOWN); 
			Beamforming_SetBeamFormStatus(priv, pSoundingInfo->SUBFeeCurIdx);
		}
		else
		{
			pEntry->LogStatusFailCnt++;
			pEntry->BeamformEntryState = BEAMFORMING_ENTRY_STATE_INITIALIZED;
			Beamforming_SetBeamFormStatus(priv, pSoundingInfo->SUBFeeCurIdx);
			//HalComTxbf_Set(pDM_Odm, TXBF_SET_TX_PATH_RESET, (pu1Byte)&(pBeamInfo->BeamformeeCurIdx));
			ODM_RT_TRACE(ODMPTR, PHYDM_COMP_TXBF, ODM_DBG_TRACE, ("[%s] LogStatusFailCnt %d\n", __func__, pEntry->LogStatusFailCnt));
		}

		//pEntry->bBeamformingInProgress = FALSE;		
		ODM_RT_TRACE(ODMPTR, PHYDM_COMP_TXBF, ODM_DBG_TRACE, ("[%s]: Set to SOUNDING_STATE_SU_SOUNDDOWN \n", __func__));
	}
}	 

#endif

