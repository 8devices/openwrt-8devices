

#include "8192cd.h"
#include "8192cd_cfg.h"
#include "8192cd_util.h"
#include "8192cd_headers.h"

#include "8812_vht_gen.h"

#ifdef RTK_AC_SUPPORT

void input_value_32(unsigned long *p, unsigned char start, unsigned char end, unsigned int value)
{
	unsigned int bit_mask = 0;

	{
		if(start == end) //1-bit value
		{
			if(value)
			*p |= BIT(start);
			else
			*p &= (~(BIT(start)));
		}
		else
		{
			unsigned char x = 0;
				
			for(x = 0; x<=(end-start); x ++)
				bit_mask |= BIT(x);

			*p &= (~(bit_mask << start));	
			*p |= ((value&bit_mask) << start);	
		}
	}
}

// 				20/40/80,	ShortGI,	MCS Rate 
const u2Byte VHT_MCS_DATA_RATE[3][2][30] = 
	{	{	{13, 26, 39, 52, 78, 104, 117, 130, 156, 156,
			 26, 52, 78, 104, 156, 208, 234, 260, 312, 312, 
			 39, 78, 117, 156, 234, 312, 351, 390, 468, 520},					// Long GI, 20MHz
			 
			{14, 29, 43, 58, 87, 116, 130, 144, 173, 173,
			 29, 58, 87, 116, 173, 231, 260, 289, 347, 347,
			 43, 86, 130, 173, 260, 347, 390, 433, 520, 578}			},		// Short GI, 20MHz
			 
		{	{27, 54, 81, 108, 162, 216, 243, 270, 324, 360, 
			 54, 108, 162, 216, 324, 432, 486, 540, 648, 720, 
			 81, 162, 243, 342, 486, 648, 729, 810, 972, 1080}, 				// Long GI, 40MHz
			 
			{30, 60, 90, 120, 180, 240, 270, 300,360, 400, 
			 60, 120, 180, 240, 360, 480, 540, 600, 720, 800,
			 90, 180, 270, 360, 540, 720, 810, 900, 1080, 1200}			},		// Short GI, 40MHz
			 
		{	{59, 117,  176, 234, 351, 468, 527, 585, 702, 780,
			 117, 234, 351, 468, 702, 936, 1053, 1170, 1404, 1560, 
			 176, 351, 527, 702, 1053, 1408, 1408, 1745, 2106, 2340}, 			// Long GI, 80MHz
			 
			{65, 130, 195, 260, 390, 520, 585, 650, 780, 867, 
			 130, 260, 390, 520, 780, 1040, 1170, 1300, 1560, 1733, 
			 195, 390, 585, 780, 1170, 1560, 1560, 1950, 2340, 2600}	}		// Short GI, 80MHz
			 
	};

enum _VHT_SUPPORT_RATE_MAP_ {
	SUPPORT_MCS_0_7_RATES = 0,
	SUPPORT_MCS_0_8_RATES = 1,
	SUPPORT_MCS_0_9_RATES = 2,
	NOT_SUPPORT_VHT_RATES = 3
};

/*
*	Description:
*		This function will get the highest speed rate in input MCS set.
*
*	/param 	Adapter			Pionter to Adapter entity
*			pMCSRateSet		Pointer to MCS rate bitmap
*			pMCSFilter		Pointer to MCS rate filter
*	
*	/return	Highest MCS rate included in pMCSRateSet and filtered by pMCSFilter.
*
*/
u1Byte
VHTGetHighestMCSRate(
	struct rtl8192cd_priv *priv,
	IN	pu1Byte			pVHTMCSRateSet
	)
{
	u1Byte		i, j;
	u1Byte		bitMap;
	u1Byte		VHTMcsRate = 0;
	
	for(i = 0; i < 2; i++)
	{
		if(pVHTMCSRateSet[i] != 0xff)
		{
			for(j = 0; j < 8; j += 2)
			{
				bitMap = (pVHTMCSRateSet[i] >> j) & 3;
				
				if(bitMap != 3)
					VHTMcsRate = _NSS1_MCS7_RATE_ + 5*j + i*40 + bitMap;  //VHT rate indications begin from 0x90
			}
		}
	}
	
	return VHTMcsRate;
}

u2Byte
VHTMcsToDataRate(
	struct rtl8192cd_priv *priv
	)
{
	BOOLEAN						isShortGI = FALSE;
	u2Byte			VHTMcsRate;

#if 1

	if(priv->pshare->rf_ft_var.lgirate == 0)
		return 0;

	isShortGI = 0;
	if((priv->pmib->dot11acConfigEntry.dot11SupportedVHT & 0x30) != 0x30)
		VHTMcsRate = _NSS3_MCS7_RATE_ +((priv->pmib->dot11acConfigEntry.dot11SupportedVHT>>4) &3);
	else if((priv->pmib->dot11acConfigEntry.dot11SupportedVHT & 0x000c) != 0x0c)
		VHTMcsRate = _NSS2_MCS7_RATE_ +((priv->pmib->dot11acConfigEntry.dot11SupportedVHT>>2) &3);	
	else
		VHTMcsRate = _NSS1_MCS7_RATE_ +((priv->pmib->dot11acConfigEntry.dot11SupportedVHT) &3);	;	
#else


	switch(priv->pshare->CurrentChannelBW){
		case HT_CHANNEL_WIDTH_20:
			isShortGI = priv->pmib->dot11nConfigEntry.dot11nShortGIfor20M?1:0;
			break;
		case HT_CHANNEL_WIDTH_20_40:
			isShortGI = priv->pmib->dot11nConfigEntry.dot11nShortGIfor40M?1:0;
			break;
		case HT_CHANNEL_WIDTH_80:
			isShortGI = priv->pmib->dot11nConfigEntry.dot11nShortGIfor40M?1:0;		// ??
			break;
	}
#endif	
	VHTMcsRate -=_NSS1_MCS0_RATE_;

	if( ((VHTMcsRate>20)&&get_rf_mimo_mode(priv)==MIMO_2T2R)|| (priv->pshare->is_40m_bw > 2))
		return 600;
	else
		return VHT_MCS_DATA_RATE[priv->pshare->is_40m_bw][isShortGI][(VHTMcsRate&0x3f)];
}

unsigned int filter_mcs_9(unsigned int supported_vht, int num_ss)
{
	int tmp = 0;

	//panic_printk(" +++ supported_vht = 0x%x \n", supported_vht);
	
	for(tmp = 1; tmp <= num_ss; tmp++)
	{
		int bit_shift = ((tmp-1)*2);

		if(tmp > 8)
			break;
			
		if((tmp==3) || (tmp==6)) //3SS & 6SS support 20M + MCS9
			continue;

		if(( supported_vht & (3 << bit_shift)) == (SUPPORT_MCS_0_9_RATES << bit_shift))
		{
			supported_vht &= ~(3 << bit_shift);
			supported_vht |= (SUPPORT_MCS_0_8_RATES << bit_shift);
		}
	
	}

	//panic_printk(" --- supported_vht = 0x%x \n", supported_vht);

	return supported_vht;

}

#ifdef MCR_WIRELESS_EXTEND
void construct_vht_ie_mcr(struct rtl8192cd_priv *priv, unsigned char channel_center, struct stat_info *pstat)
{
	struct vht_cap_elmt	*vht_cap;
	struct vht_oper_elmt *vht_oper;
	unsigned int value; 

	unsigned char txbf_max_ant, txbf_sounding_dim;

	unsigned int supported_vht = priv->pmib->dot11acConfigEntry.dot11SupportedVHT;

	
//// ===== VHT CAPABILITIES ELEMENT ===== /////
//VHT CAPABILITIES INFO field

	priv->vht_cap_len = sizeof(struct vht_cap_elmt);
	vht_cap = &priv->vht_cap_buf; 
	memset(vht_cap, 0, sizeof(struct vht_cap_elmt));

	switch(get_rf_mimo_mode(priv)) {
		case MIMO_1T1R:
			supported_vht |= 0xfffe;
			break;
		case MIMO_3T3R:
		case MIMO_4T4R:
			supported_vht |= 0xffea;
			break;
		default:	//2T2R
			supported_vht |= 0xfffa;
			break;
	}	        

    // TODO: MAX_MPDU_LENGTH_E in 11AC
	if(priv->pmib->dot11nConfigEntry.dot11nAMSDURecvMax) 
		input_value_32(&vht_cap->vht_cap_info, MAX_MPDU_LENGTH_S, MAX_MPDU_LENGTH_E, (priv->pmib->dot11nConfigEntry.dot11nAMSDURecvMax & 0x3));
	else	
	input_value_32(&vht_cap->vht_cap_info, MAX_MPDU_LENGTH_S, MAX_MPDU_LENGTH_E, 0);

	//0 - not support 160/80+80; 1 - support 160; 2 - support 80+80 
	if(priv->pshare->CurrentChannelBW == HT_CHANNEL_WIDTH_AC_160)
		value = 1;
	else
		value = 0;
	input_value_32(&vht_cap->vht_cap_info, CHL_WIDTH_S, CHL_WIDTH_E, value);

	if((GET_CHIP_VER(priv) == VERSION_8814A) && (priv->pshare->is_40m_bw == HT_CHANNEL_WIDTH_AC_80))
		input_value_32(&vht_cap->vht_cap_info, SHORT_GI80M_S, SHORT_GI80M_E, (priv->pmib->dot11nConfigEntry.dot11nShortGIfor40M ? 1 : 0));

	input_value_32(&vht_cap->vht_cap_info, SHORT_GI160M_S, SHORT_GI160M_E, 0);


	if( (priv->pmib->dot11nConfigEntry.dot11nLDPC&1)
#if defined(CONFIG_WLAN_HAL_8881A)
	 && (GET_CHIP_VER(priv) != VERSION_8881A)
#endif
	)	
		input_value_32(&vht_cap->vht_cap_info, RX_LDPC_S, RX_LDPC_E, 1);
	else
		input_value_32(&vht_cap->vht_cap_info, RX_LDPC_S, RX_LDPC_E, 0);
	
#if 1
	if (priv->pmib->dot11nConfigEntry.dot11nSTBC)
	{
		if ((get_rf_mimo_mode(priv) == MIMO_2T2R) || (get_rf_mimo_mode(priv) == MIMO_3T3R)) //eric_8814 
			input_value_32(&vht_cap->vht_cap_info, TX_STBC_S, TX_STBC_E, 1);
		
		//8822 A/B-cut disable RX STBC		
		if(GET_CHIP_VER(priv) == VERSION_8822B && GET_CHIP_VER_8822(priv) <= 0x1)
			input_value_32(&vht_cap->vht_cap_info, RX_STBC_S, RX_STBC_E, 0);
		else
			input_value_32(&vht_cap->vht_cap_info, RX_STBC_S, RX_STBC_E, 1);
	}
	else
#endif
	{
		input_value_32(&vht_cap->vht_cap_info, TX_STBC_S, TX_STBC_E, 0);
		input_value_32(&vht_cap->vht_cap_info, RX_STBC_S, RX_STBC_E, 0);
	}

#if (BEAMFORMING_SUPPORT == 1)
	if (priv->pmib->dot11RFEntry.txbf == 1) {
		if(priv->pmib->dot11RFEntry.txbfer == 1)
			input_value_32(&vht_cap->vht_cap_info, SU_BFER_S, SU_BFER_E, 1);
		if(priv->pmib->dot11RFEntry.txbfee == 1)
			input_value_32(&vht_cap->vht_cap_info, SU_BFEE_S, SU_BFEE_E, 1);
	} else
#endif
	{
	input_value_32(&vht_cap->vht_cap_info, SU_BFER_S, SU_BFER_E, 0);
	input_value_32(&vht_cap->vht_cap_info, SU_BFEE_S, SU_BFEE_E, 0);
	}
#ifdef CONFIG_WLAN_HAL_8814AE
	if(priv->pshare->rf_ft_var.bf_sup_val != 0){		
		input_value_32(&vht_cap->vht_cap_info, MAX_ANT_SUPP_S, MAX_ANT_SUPP_E, priv->pshare->rf_ft_var.bf_sup_val);
		input_value_32(&vht_cap->vht_cap_info, SOUNDING_DIMENSIONS_S, SOUNDING_DIMENSIONS_E, priv->pshare->rf_ft_var.bf_sup_val);
	}else
#endif	
	{
#ifdef CONFIG_WLAN_HAL_8814AE
		if(GET_CHIP_VER(priv)==VERSION_8814A) {
			if(get_rf_mimo_mode(priv) == MIMO_4T4R) {       
				txbf_max_ant = 3;
			    txbf_sounding_dim = 3;
			} else if(get_rf_mimo_mode(priv) == MIMO_3T3R) {
				txbf_max_ant = 2;
			    txbf_sounding_dim = 3;
			} else if(get_rf_mimo_mode(priv) == MIMO_2T4R) {
				txbf_max_ant = 2;
			    txbf_sounding_dim = 1;
			} else  if(get_rf_mimo_mode(priv) == MIMO_2T2R) {
				txbf_max_ant = 2;
			    txbf_sounding_dim = 1;
			} else {
				txbf_max_ant = 1;
			    txbf_sounding_dim = 1;
			}
			input_value_32(&vht_cap->vht_cap_info, MAX_ANT_SUPP_S, MAX_ANT_SUPP_E, txbf_max_ant);
			input_value_32(&vht_cap->vht_cap_info, SOUNDING_DIMENSIONS_S, SOUNDING_DIMENSIONS_E, txbf_sounding_dim);	
		}
		else
#endif
		{
			input_value_32(&vht_cap->vht_cap_info, MAX_ANT_SUPP_S, MAX_ANT_SUPP_E, BEAMFORM_MAX_ANT_SUPP);
			input_value_32(&vht_cap->vht_cap_info, SOUNDING_DIMENSIONS_S, SOUNDING_DIMENSIONS_E, BEAMFORM_SOUNDING_DIMENSIONS);
		}
	}


	input_value_32(&vht_cap->vht_cap_info, MU_BFER_S, MU_BFER_E, 0);
	input_value_32(&vht_cap->vht_cap_info, MU_BFEE_S, MU_BFEE_E, 0);
	
	input_value_32(&vht_cap->vht_cap_info, TXOP_PS_S, TXOP_PS_E, 0);

	input_value_32(&vht_cap->vht_cap_info, HTC_VHT_S, HTC_VHT_E, 1);
#ifdef MCR_WIRELESS_EXTEND
	input_value_32(&vht_cap->vht_cap_info, MAX_RXAMPDU_FACTOR_S, MAX_RXAMPDU_FACTOR_E, priv->pshare->rf_ft_var.ampdu_den_vht);
#else
	input_value_32(&vht_cap->vht_cap_info, MAX_RXAMPDU_FACTOR_S, MAX_RXAMPDU_FACTOR_E, 7);
#endif	
	input_value_32(&vht_cap->vht_cap_info, LINK_ADAPTION_S, LINK_ADAPTION_E, 0);
	
	input_value_32(&vht_cap->vht_cap_info, RX_ANT_PC_S, RX_ANT_PC_E, 0);
	input_value_32(&vht_cap->vht_cap_info, TX_ANT_PC_S, TX_ANT_PC_E, 0);

	//printk("vht_cap->vht_cap_info 0x%08X ", vht_cap->vht_cap_info);
	vht_cap->vht_cap_info = cpu_to_le32(vht_cap->vht_cap_info);
	//printk("0x%08X\n", vht_cap->vht_cap_info);

#if defined(AC2G_256QAM) || defined(WLAN_HAL_8814AE)
	if(is_ac2g(priv) && ((priv->pshare->CurrentChannelBW == HT_CHANNEL_WIDTH_AC_20)||(GET_CHIP_VER(priv)==VERSION_8814A))) //if bw = 20M, not support MCS9
	{
		if(get_rf_mimo_mode(priv) == MIMO_1T1R) 
		{
			supported_vht = 0xfffd;
		}
		else if(get_rf_mimo_mode(priv) == MIMO_2T2R)
		{
			supported_vht = 0xfff5;
		}
		else if(get_rf_mimo_mode(priv) == MIMO_3T3R || get_rf_mimo_mode(priv) == MIMO_4T4R)
		{
			supported_vht = 0xffd5;
		}
		else
			supported_vht = 0xfff5; //2ss as default
	}
#endif

	if(priv->pshare->CurrentChannelBW == HT_CHANNEL_WIDTH_AC_20)
	{
		// 1SS & 2SS do NOT support MCS9 in BW 20m
		if(get_rf_mimo_mode(priv) == MIMO_1T1R)
			supported_vht = filter_mcs_9(supported_vht, 1);
		else if(get_rf_mimo_mode(priv) == MIMO_2T2R)
			supported_vht = filter_mcs_9(supported_vht, 2);
	}

	if (GET_CHIP_VER(priv)==VERSION_8814A && get_rf_mimo_mode(priv) == MIMO_2T2R) {		
		if (pstat->IOTPeer == HT_IOT_PEER_REALTEK_8812) {
			supported_vht = 0xfff5;
		}
	}

	{
		input_value_32(&vht_cap->vht_support_mcs[0], MCS_RX_MAP_S, MCS_RX_MAP_E, supported_vht);
		value = (VHTMcsToDataRate(priv)+1)>>1;
		input_value_32(&vht_cap->vht_support_mcs[0], MCS_RX_HIGHEST_RATE_S, MCS_RX_HIGHEST_RATE_E, value);
		vht_cap->vht_support_mcs[0] = cpu_to_le32(vht_cap->vht_support_mcs[0]);

		input_value_32(&vht_cap->vht_support_mcs[1], MCS_TX_MAP_S, MCS_TX_MAP_E, supported_vht);
		value = (VHTMcsToDataRate(priv)+1)>>1;
		input_value_32(&vht_cap->vht_support_mcs[1], MCS_TX_HIGHEST_RATE_S, MCS_TX_HIGHEST_RATE_E, value);
		vht_cap->vht_support_mcs[1] = cpu_to_le32(vht_cap->vht_support_mcs[1]);
	}

//// ===== VHT CAPABILITIES ELEMENT ===== /////
	priv->vht_oper_len = sizeof(struct vht_oper_elmt);
	vht_oper = &priv->vht_oper_buf; 
	memset(vht_oper, 0, sizeof(struct vht_oper_elmt));

	if((priv->pshare->is_40m_bw == HT_CHANNEL_WIDTH_AC_80) || (GET_CHIP_VER(priv)==VERSION_8814A))
	{
		vht_oper->vht_oper_info[0] = (priv->pshare->is_40m_bw ==2) ? 1 : 0;	

		if(OPMODE & (WIFI_STATION_STATE))
		vht_oper->vht_oper_info[0] = 1; //8812_client

		if(priv->pshare->is_40m_bw ==2) {
			int channel = priv->pmib->dot11RFEntry.dot11channel;
			if (channel <= 48)
				channel_center = 42;
			else if (channel <= 64)
				channel_center = 58;
			else if (channel <= 112)
				channel_center = 106;
			else if (channel <= 128)
				channel_center = 122;
			else if (channel <= 144)
				channel_center = 138;
			else if (channel <= 161)
				channel_center = 155;
			else if (channel <= 177)
				channel_center = 171;
		}
			
		vht_oper->vht_oper_info[1] = channel_center;
		vht_oper->vht_oper_info[2] = 0;
	}

	if(get_rf_mimo_mode(priv) == MIMO_1T1R) 
		value = 0xfffc; 
	else if(get_rf_mimo_mode(priv) == MIMO_2T2R)
		value = 0xfff0; 
	else if(get_rf_mimo_mode(priv) == MIMO_3T3R || get_rf_mimo_mode(priv) == MIMO_4T4R)
		value = 0xffea;
	else
		value = 0xfff0; //2ss as default
	
	vht_oper->vht_basic_msc = value; 
	vht_oper->vht_basic_msc = cpu_to_le16(vht_oper->vht_basic_msc);


}

#endif


void construct_vht_ie(struct rtl8192cd_priv *priv, unsigned char channel_center)
{
	struct vht_cap_elmt	*vht_cap;
	struct vht_oper_elmt *vht_oper;
	unsigned int value; 

	unsigned char txbf_max_ant, txbf_sounding_dim;

	unsigned int supported_vht = priv->pmib->dot11acConfigEntry.dot11SupportedVHT;

	
//// ===== VHT CAPABILITIES ELEMENT ===== /////
//VHT CAPABILITIES INFO field

	priv->vht_cap_len = sizeof(struct vht_cap_elmt);
	vht_cap = &priv->vht_cap_buf; 
	memset(vht_cap, 0, sizeof(struct vht_cap_elmt));

	switch(get_rf_mimo_mode(priv)) {
		case MIMO_1T1R:
			supported_vht |= 0xfffe;
			break;
		case MIMO_3T3R:
		case MIMO_4T4R:
			supported_vht |= 0xffea;
			break;
		default:	//2T2R
			supported_vht |= 0xfffa;
			break;
	}	        

    // TODO: MAX_MPDU_LENGTH_E in 11AC
	if(priv->pmib->dot11nConfigEntry.dot11nAMSDURecvMax) 
		input_value_32(&vht_cap->vht_cap_info, MAX_MPDU_LENGTH_S, MAX_MPDU_LENGTH_E, (priv->pmib->dot11nConfigEntry.dot11nAMSDURecvMax & 0x3));
	else	
	input_value_32(&vht_cap->vht_cap_info, MAX_MPDU_LENGTH_S, MAX_MPDU_LENGTH_E, 0);

	//0 - not support 160/80+80; 1 - support 160; 2 - support 80+80 
	if(priv->pshare->CurrentChannelBW == HT_CHANNEL_WIDTH_AC_160)
		value = 1;
	else
		value = 0;
	input_value_32(&vht_cap->vht_cap_info, CHL_WIDTH_S, CHL_WIDTH_E, value);

	if(priv->pshare->is_40m_bw == HT_CHANNEL_WIDTH_AC_80)
		input_value_32(&vht_cap->vht_cap_info, SHORT_GI80M_S, SHORT_GI80M_E, (priv->pmib->dot11nConfigEntry.dot11nShortGIfor80M ? 1 : 0));

	input_value_32(&vht_cap->vht_cap_info, SHORT_GI160M_S, SHORT_GI160M_E, 0);


	if( (priv->pmib->dot11nConfigEntry.dot11nLDPC&1) && (can_enable_rx_ldpc(priv)) )	
		input_value_32(&vht_cap->vht_cap_info, RX_LDPC_S, RX_LDPC_E, 1);
	else
		input_value_32(&vht_cap->vht_cap_info, RX_LDPC_S, RX_LDPC_E, 0);
	
#if 1
	if (priv->pmib->dot11nConfigEntry.dot11nSTBC)
	{
		if ((get_rf_mimo_mode(priv) == MIMO_2T2R) || (get_rf_mimo_mode(priv) == MIMO_3T3R)) //eric_8814 
			input_value_32(&vht_cap->vht_cap_info, TX_STBC_S, TX_STBC_E, 1);
		
		//8822 A/B-cut disable RX STBC
		if(GET_CHIP_VER(priv) == VERSION_8822B && GET_CHIP_VER_8822(priv) <= 0x1)
			input_value_32(&vht_cap->vht_cap_info, RX_STBC_S, RX_STBC_E, 0);
		else
			input_value_32(&vht_cap->vht_cap_info, RX_STBC_S, RX_STBC_E, 1);
	}
	else
#endif
	{
		input_value_32(&vht_cap->vht_cap_info, TX_STBC_S, TX_STBC_E, 0);
		input_value_32(&vht_cap->vht_cap_info, RX_STBC_S, RX_STBC_E, 0);
	}

#if (BEAMFORMING_SUPPORT == 1)
	if (priv->pmib->dot11RFEntry.txbf == 1) {
		if(priv->pmib->dot11RFEntry.txbfer == 1)
			input_value_32(&vht_cap->vht_cap_info, SU_BFER_S, SU_BFER_E, 1);
		if(priv->pmib->dot11RFEntry.txbfee == 1)
			input_value_32(&vht_cap->vht_cap_info, SU_BFEE_S, SU_BFEE_E, 1);
	} else
#endif
	{
			input_value_32(&vht_cap->vht_cap_info, SU_BFER_S, SU_BFER_E, 0);
			input_value_32(&vht_cap->vht_cap_info, SU_BFEE_S, SU_BFEE_E, 0);
	}
#ifdef CONFIG_WLAN_HAL_8814AE
	if(priv->pshare->rf_ft_var.bf_sup_val != 0){		
		input_value_32(&vht_cap->vht_cap_info, MAX_ANT_SUPP_S, MAX_ANT_SUPP_E, priv->pshare->rf_ft_var.bf_sup_val);
		input_value_32(&vht_cap->vht_cap_info, SOUNDING_DIMENSIONS_S, SOUNDING_DIMENSIONS_E, priv->pshare->rf_ft_var.bf_sup_val);
	}else
#endif	
	{
#ifdef CONFIG_WLAN_HAL_8814AE
		if(GET_CHIP_VER(priv)==VERSION_8814A) {
			if(get_rf_mimo_mode(priv) == MIMO_4T4R) {       
				txbf_max_ant = 3;
			    txbf_sounding_dim = 3;
			} else if(get_rf_mimo_mode(priv) == MIMO_3T3R) {
				txbf_max_ant = 2;
			    txbf_sounding_dim = 2;
			} else if(get_rf_mimo_mode(priv) == MIMO_2T4R) {
				txbf_max_ant = 2;
			    txbf_sounding_dim = 1;
			} else  if(get_rf_mimo_mode(priv) == MIMO_2T2R) {
				txbf_max_ant = 2;
			    txbf_sounding_dim = 1;
			} else {
				txbf_max_ant = 1;
			    txbf_sounding_dim = 1;
			}
			input_value_32(&vht_cap->vht_cap_info, MAX_ANT_SUPP_S, MAX_ANT_SUPP_E, txbf_max_ant);
			input_value_32(&vht_cap->vht_cap_info, SOUNDING_DIMENSIONS_S, SOUNDING_DIMENSIONS_E, txbf_sounding_dim);	
		}
		else
#endif
		{
			input_value_32(&vht_cap->vht_cap_info, MAX_ANT_SUPP_S, MAX_ANT_SUPP_E, BEAMFORM_MAX_ANT_SUPP);
			input_value_32(&vht_cap->vht_cap_info, SOUNDING_DIMENSIONS_S, SOUNDING_DIMENSIONS_E, BEAMFORM_SOUNDING_DIMENSIONS);
		}
	}


	input_value_32(&vht_cap->vht_cap_info, MU_BFER_S, MU_BFER_E, 0);
	input_value_32(&vht_cap->vht_cap_info, MU_BFEE_S, MU_BFEE_E, 0);

	if(GET_CHIP_VER(priv) == VERSION_8822B && priv->pmib->dot11RFEntry.txbf) { //eric-8822 add mu txbf cap
		input_value_32(&vht_cap->vht_cap_info, MAX_ANT_SUPP_S, MAX_ANT_SUPP_E, 3);
		input_value_32(&vht_cap->vht_cap_info, SOUNDING_DIMENSIONS_S, SOUNDING_DIMENSIONS_E, 1);
		if(priv->pmib->dot11RFEntry.txbf_mu) {

		if((priv->pmib->dot11RFEntry.txbfer == 1) && (OPMODE & WIFI_AP_STATE))
			input_value_32(&vht_cap->vht_cap_info, MU_BFER_S, MU_BFER_E, 1);
		
		if((priv->pmib->dot11RFEntry.txbfee == 1) && (OPMODE & WIFI_STATION_STATE))
			input_value_32(&vht_cap->vht_cap_info, MU_BFEE_S, MU_BFEE_E, 1);


		}
	
	}

	input_value_32(&vht_cap->vht_cap_info, TXOP_PS_S, TXOP_PS_E, 0);

	input_value_32(&vht_cap->vht_cap_info, HTC_VHT_S, HTC_VHT_E, 1);

		
#ifdef CONFIG_WLAN_HAL_8814AE
	if((GET_CHIP_VER(priv)==VERSION_8814A) && priv->pmib->dot11nConfigEntry.dot11nUse40M == HT_CHANNEL_WIDTH_20)
		input_value_32(&vht_cap->vht_cap_info, MAX_RXAMPDU_FACTOR_S, MAX_RXAMPDU_FACTOR_E, 2);
	else
#endif		
		input_value_32(&vht_cap->vht_cap_info, MAX_RXAMPDU_FACTOR_S, MAX_RXAMPDU_FACTOR_E, 7);
	
	input_value_32(&vht_cap->vht_cap_info, LINK_ADAPTION_S, LINK_ADAPTION_E, 0);
	
	input_value_32(&vht_cap->vht_cap_info, RX_ANT_PC_S, RX_ANT_PC_E, 0);
	input_value_32(&vht_cap->vht_cap_info, TX_ANT_PC_S, TX_ANT_PC_E, 0);

	//printk("vht_cap->vht_cap_info 0x%08X ", vht_cap->vht_cap_info);
	vht_cap->vht_cap_info = cpu_to_le32(vht_cap->vht_cap_info);
	//printk("0x%08X\n", vht_cap->vht_cap_info);

#if defined(AC2G_256QAM) || defined(WLAN_HAL_8814AE)
	if(is_ac2g(priv) && ((priv->pmib->dot11nConfigEntry.dot11nUse40M == HT_CHANNEL_WIDTH_AC_20)||(GET_CHIP_VER(priv)==VERSION_8814A))) //if bw = 20M, not support MCS9
	{
		if(get_rf_mimo_mode(priv) == MIMO_1T1R) 
		{
			supported_vht = 0xfffd;
		}
		else if(get_rf_mimo_mode(priv) == MIMO_2T2R)
		{
			supported_vht = 0xfff5;
		}
		else if(get_rf_mimo_mode(priv) == MIMO_3T3R || get_rf_mimo_mode(priv) == MIMO_4T4R)
		{
			supported_vht = 0xffd5;
		}
		else
			supported_vht = 0xfff5; //2ss as default
	}
#endif

	if(priv->pmib->dot11nConfigEntry.dot11nUse40M == HT_CHANNEL_WIDTH_AC_20)
	{
		// 1SS & 2SS do NOT support MCS9 in BW 20m
		if(get_rf_mimo_mode(priv) == MIMO_1T1R)
			supported_vht = filter_mcs_9(supported_vht, 1);
		else if(get_rf_mimo_mode(priv) == MIMO_2T2R)
			supported_vht = filter_mcs_9(supported_vht, 2);
		else if (get_rf_mimo_mode(priv) == MIMO_3T3R)
			supported_vht = filter_mcs_9(supported_vht, 3);			
		else if (get_rf_mimo_mode(priv) == MIMO_4T4R)
			supported_vht = filter_mcs_9(supported_vht, 4);			
	}

	{
		input_value_32(&vht_cap->vht_support_mcs[0], MCS_RX_MAP_S, MCS_RX_MAP_E, supported_vht);
		value = (VHTMcsToDataRate(priv)+1)>>1;
		input_value_32(&vht_cap->vht_support_mcs[0], MCS_RX_HIGHEST_RATE_S, MCS_RX_HIGHEST_RATE_E, value);
		vht_cap->vht_support_mcs[0] = cpu_to_le32(vht_cap->vht_support_mcs[0]);

		input_value_32(&vht_cap->vht_support_mcs[1], MCS_TX_MAP_S, MCS_TX_MAP_E, supported_vht);
		value = (VHTMcsToDataRate(priv)+1)>>1;
		input_value_32(&vht_cap->vht_support_mcs[1], MCS_TX_HIGHEST_RATE_S, MCS_TX_HIGHEST_RATE_E, value);
		vht_cap->vht_support_mcs[1] = cpu_to_le32(vht_cap->vht_support_mcs[1]);
	}

//// ===== VHT CAPABILITIES ELEMENT ===== /////
	priv->vht_oper_len = sizeof(struct vht_oper_elmt);
	vht_oper = &priv->vht_oper_buf; 
	memset(vht_oper, 0, sizeof(struct vht_oper_elmt));

	if((priv->pshare->is_40m_bw == HT_CHANNEL_WIDTH_AC_80) || (GET_CHIP_VER(priv)==VERSION_8814A))
	{
		vht_oper->vht_oper_info[0] = ((priv->pshare->is_40m_bw == HT_CHANNEL_WIDTH_AC_80)
#if 0//defined(dybw_beacon)
										&& (priv->pshare->CurrentChannelBW == HT_CHANNEL_WIDTH_AC_80)
#endif
									  ) ? 1 : 0;

		if(OPMODE & (WIFI_STATION_STATE))
			vht_oper->vht_oper_info[0] = 1; //8812_client

		if(priv->pshare->is_40m_bw ==HT_CHANNEL_WIDTH_AC_80) {
			int channel = priv->pmib->dot11RFEntry.dot11channel;
			if (channel <= 48)
				channel_center = 42;
			else if (channel <= 64)
				channel_center = 58;
			else if (channel <= 112)
				channel_center = 106;
			else if (channel <= 128)
				channel_center = 122;
			else if (channel <= 144)
				channel_center = 138;
			else if (channel <= 161)
				channel_center = 155;
			else if (channel <= 177)
				channel_center = 171;
		}
			
		vht_oper->vht_oper_info[1] = channel_center;
		vht_oper->vht_oper_info[2] = 0;
	}

	if(get_rf_mimo_mode(priv) == MIMO_1T1R) 
		value = 0xfffc; 
	else if(get_rf_mimo_mode(priv) == MIMO_2T2R)
		value = 0xfff0; 
	else if(get_rf_mimo_mode(priv) == MIMO_3T3R || get_rf_mimo_mode(priv) == MIMO_4T4R)
		value = 0xffea;
	else
		value = 0xfff0; //2ss as default

	if(priv->pmib->dot11nConfigEntry.dot11nUse40M == HT_CHANNEL_WIDTH_AC_20)
	{
		if(get_rf_mimo_mode(priv) == MIMO_1T1R)
			value = filter_mcs_9(value, 1);
		else if(get_rf_mimo_mode(priv) == MIMO_2T2R)
			value = filter_mcs_9(value, 2);
		else if (get_rf_mimo_mode(priv) == MIMO_3T3R)
			value = filter_mcs_9(value, 3);			
		else if (get_rf_mimo_mode(priv) == MIMO_4T4R)
			value = filter_mcs_9(value, 4);			
	}
	
	vht_oper->vht_basic_msc = value; 
	vht_oper->vht_basic_msc = cpu_to_le16(vht_oper->vht_basic_msc);
}

#endif //CONFIG_RTL_8812_SUPPORT

