
#define _8812_HW_C_

#include "8192cd.h"
#include "8192cd_cfg.h"
#include "8192cd_util.h"

#include "8192c_reg.h"
#include "8812_reg.h"
#include "8812_vht_gen.h"

#ifdef __KERNEL__
#include <linux/kernel.h>
#endif
//eric_8812 #include "8192cd_debug.h"
#include "8192cd_headers.h"



#if defined(CONFIG_RTL_8812_SUPPORT) || defined(CONFIG_RTL_8723B_SUPPORT)
typedef struct _C2H_EVT_HDR{
	u1Byte	CmdID: 4;
	u1Byte	CmdLen: 4; 
	u1Byte	CmdSeq;
}C2H_EVT_HDR, *PC2H_EVT_HDR;

typedef enum _RTL8812_C2H_EVT
{
	C2H_8812_DBG = 0,
	C2H_8812_LB = 1,
	C2H_8812_TXBF = 2,
	CCXRPT = 3,
	C2H_8812_TXPERORT = 4,
	C2H_8812_RA_RPT = 12,
#ifdef TXRETRY_CNT
	C2H_8812_TXRETRY = 13, //0x0D
#endif
	C2H_8812_RA_PARA_RPT=14,
	C2H_8812_EXTEND_IND = 0xFF,
	MAX_8812_C2HEVENT
}RTL8812_C2H_EVT;

typedef enum _RTL8812_EXTEND_C2H_EVT
{
	EXTEND_C2H_8812_DBG_PRINT = 0

}RTL8812_EXTEND_C2H_EVT;

void UpdateBBRFVal8812(struct rtl8192cd_priv *priv, unsigned char channel)
{
	unsigned char current_is_5g = 0, switch_bw = 0;
	unsigned char bTmp = 0;
	unsigned int dwTmp = 0;
	unsigned int tmp_cb0=0, tmp_eb0 = 0; //for_8812_mp_chip
	unsigned int eRFPath, curMaxRFPath;

	curMaxRFPath = RF92CD_PATH_MAX;

	//check CCK_CHECK_en BIT7
	if ( RTL_R8(0x454) & BIT(7) )
		current_is_5g = 1;
	else
		current_is_5g = 0;

	if (((channel > 14) && (current_is_5g == 0)) || ((channel <= 14) && (current_is_5g == 1)))
		switch_bw = 1;

	//printk("\n\n eric-current_is_5g = %d switch_bw = %d pre= %d\n\n", current_is_5g, switch_bw, priv->pshare->pre_channel);

	if (channel > 14 && (switch_bw || (priv->pshare->pre_channel == 0))) //2.4G to 5G
		RTL_W8(0x454, (RTL_R8(0x454) | BIT(7)));

	//Set fc_area 0x860
	dwTmp = RTL_R32(0x860);
	dwTmp &= ~(BIT(17) | BIT(18) | BIT(19) | BIT(20) | BIT(21) | BIT(22) | BIT(23) | BIT(24) | BIT(25) | BIT(26) | BIT(27) | BIT(28));

	if (channel <= 14)
		dwTmp |= (0x96a << 17);
	else if (channel <= 48)
		dwTmp |= (0x494 << 17);
	else if (channel <= 64)
		dwTmp |= (0x453 << 17);
	else if (channel <= 116)
		dwTmp |= (0x452 << 17);
	else if (channel <= 177)
		dwTmp |= (0x412 << 17);

	RTL_W32(0x860, dwTmp);

	//Set RF MOD AG rf_0x18
	for (eRFPath = RF92CD_PATH_A; eRFPath < curMaxRFPath; eRFPath++) {
		dwTmp = PHY_QueryRFReg(priv, eRFPath, rRfChannel, bMask20Bits, 1);
		dwTmp &= ~(BIT(8) | BIT(9) | BIT(16) | BIT(17) | BIT(18));

		if (channel <= 14)
			dwTmp |= 0x0;
		else if (channel <= 64)
			dwTmp |= (BIT(8) | BIT(16));
		else if (channel <= 140)
			dwTmp |= (BIT(8) | BIT(16) | BIT(17));
		else
			dwTmp |= (BIT(8) | BIT(16) | BIT(18));

		PHY_SetRFReg(priv, eRFPath, rRfChannel, bMask20Bits, dwTmp);
	}

	//Set BB registers  //for_8812_mp_chip
	if (switch_bw) {
		if (!IS_TEST_CHIP(priv)) {
			tmp_cb0 = RTL_R32(0xcb0);
			tmp_cb0 &= ~(BIT(16) | BIT(17) | BIT(18) | BIT(19) | BIT(20) | BIT(21) | BIT(22) | BIT(23));
			tmp_eb0 = RTL_R32(0xeb0);
			tmp_eb0 &= ~(BIT(16) | BIT(17) | BIT(18) | BIT(19) | BIT(20) | BIT(21) | BIT(22) | BIT(23));
		}

		if (!current_is_5g) {
			bTmp = RTL_R8(0x82c);
			bTmp &= ~(BIT(0) | BIT(1));
			bTmp |= BIT(0);
			RTL_W8(0x82c, bTmp);
			if (GET_CHIP_VER(priv) == VERSION_8881A) {
				PHY_SetBBReg(priv, 0xcb0, 0xff, 0x55);
				PHY_SetBBReg(priv, 0xcb0, 0xf000, 0x1);
			} else {
				if (IS_TEST_CHIP(priv)) {
					RTL_W32(0xcb8, RTL_R32(0xcb8) | BIT(12));
					RTL_W32(0xeb8, RTL_R32(0xcb8) | BIT(12));
					RTL_W32(0xcb8, RTL_R32(0xcb8) & (~BIT(15)));
					RTL_W32(0xeb8, RTL_R32(0xeb8) & (~BIT(15)));
				} else {
#ifdef HIGH_POWER_EXT_PA
					if (priv->pshare->rf_ft_var.use_ext_pa) 
					{
	#if 0
						if (priv->pmib->dot11RFEntry.pa_type == PA_RFMD_4501) {											
							RTL_W32(0xcb0, tmp_cb0 | (0x53 << 16));
							RTL_W32(0xeb0, tmp_eb0 | (0x53 << 16));
						} else {
							RTL_W32(0xcb0, tmp_cb0 | (0x54 << 16));
							RTL_W32(0xeb0, tmp_eb0 | (0x54 << 16));
						}
	#else					
						//For 8812 PHY_PARAMETER after V40.
						RTL_W32(0xcb0, 0x77337717);
						RTL_W32(0xeb0, 0x77337717);

						if (priv->pmib->dot11RFEntry.pa_type == PA_RFMD_4501 || priv->pmib->dot11RFEntry.pa_type == PA_SKYWORKS_85712_HP) {
							PHY_SetBBReg(priv, 0xcb4, 0x3ff00000, 0x00);	
							PHY_SetBBReg(priv, 0xeb4, 0x3ff00000, 0x00);										
						} else {
							PHY_SetBBReg(priv, 0xcb4, 0x3ff00000, 0x10);	
							PHY_SetBBReg(priv, 0xeb4, 0x3ff00000, 0x10);					
						}					
	#endif		
					} else
#endif				
					{
#ifdef CONFIG_PA_RTC5634
					// internal PA+PAPE
						if (priv->pmib->dot11RFEntry.pa_type == PA_RTC5634) {
							//panic_printk("Set PAPE on\n");
							RTL_W32(0xcb0, 0x77537717);
							RTL_W32(0xeb0, 0x77537717);
						}					
#endif
					}
				}
			}
//			RTL_W32(0x808, RTL_R32(0x808) & (~BIT(28)));
			RTL_W32(0x808, RTL_R32(0x808) | BIT(28));
			RTL_W8(0xa07, RTL_R8(0xa07) | 0xf);
			RTL_W32(0x8c0, RTL_R32(0x8c0) & (~BIT(17)));
			PHY_SetBBReg(priv, 0x830, BIT(17)|BIT(16)|BIT(15)|BIT(14)|BIT(13), 0x15);
		} else {
			bTmp = RTL_R8(0x82c);
			bTmp &= ~(BIT(0) | BIT(1));
			RTL_W8(0x82c, bTmp);

			if (IS_TEST_CHIP(priv)) {
				RTL_W32(0xcb8, RTL_R32(0xcb8) & (~BIT(12)));
				RTL_W32(0xeb8, RTL_R32(0xcb8) & (~BIT(12)));
				RTL_W32(0xcb8, RTL_R32(0xcb8) | BIT(15));
				RTL_W32(0xeb8, RTL_R32(0xeb8) | BIT(15));
			} else {
				//RTL_W32(0xcb0, tmp_cb0 | (0x77 << 16));
				//RTL_W32(0xeb0, tmp_eb0 | (0x77 << 16));
				RTL_W32(0xcb0, 0x77777777);
				RTL_W32(0xeb0, 0x77777777);
			}

			RTL_W32(0x808, RTL_R32(0x808) | BIT(28));
			RTL_W8(0xa07, (RTL_R8(0xa07) & 0xf0)|0x01);
			RTL_W32(0x8c0, RTL_R32(0x8c0) | BIT(17));
			PHY_SetBBReg(priv, 0x830, BIT(17)|BIT(16)|BIT(15)|BIT(14)|BIT(13), 0x17);
		}
	} else if (priv->pshare->pre_channel == 0) { //first launch
		if (channel > 14) {
			bTmp = RTL_R8(0x82c);
			bTmp &= ~(BIT(0) | BIT(1));
			bTmp |= BIT(0);
			RTL_W8(0x82c, bTmp);

			if (IS_TEST_CHIP(priv)) {
				RTL_W32(0xcb8, RTL_R32(0xcb8) | BIT(12));
				RTL_W32(0xeb8, RTL_R32(0xcb8) | BIT(12));
				RTL_W32(0xcb8, RTL_R32(0xcb8) & (~BIT(15)));
				RTL_W32(0xeb8, RTL_R32(0xeb8) & (~BIT(15)));
			} else {
#ifdef HIGH_POWER_EXT_PA
				if (priv->pshare->rf_ft_var.use_ext_pa) 
				{
	#if 0
					if (priv->pmib->dot11RFEntry.pa_type == PA_RFMD_4501) { 										
						RTL_W32(0xcb0, tmp_cb0 | (0x53 << 16));
						RTL_W32(0xeb0, tmp_eb0 | (0x53 << 16));
					} else {
						RTL_W32(0xcb0, tmp_cb0 | (0x54 << 16));
						RTL_W32(0xeb0, tmp_eb0 | (0x54 << 16));
					}
	#else					
					//For 8812 PHY_PARAMETER after V40.
					RTL_W32(0xcb0, 0x77337717);
					RTL_W32(0xeb0, 0x77337717);
				
					if (priv->pmib->dot11RFEntry.pa_type == PA_RFMD_4501 || priv->pmib->dot11RFEntry.pa_type == PA_SKYWORKS_85712_HP) {
						PHY_SetBBReg(priv, 0xcb4, 0x3ff00000, 0x00);	
						PHY_SetBBReg(priv, 0xeb4, 0x3ff00000, 0x00);										
					} else {
						PHY_SetBBReg(priv, 0xcb4, 0x3ff00000, 0x10);	
						PHY_SetBBReg(priv, 0xeb4, 0x3ff00000, 0x10);					
					}					
	#endif		
				} else
#endif
				{
#ifdef CONFIG_PA_RTC5634
					// internal PA+PAPE
					if (priv->pmib->dot11RFEntry.pa_type == PA_RTC5634) {
						//panic_printk("Set PAPE on\n");
						RTL_W32(0xcb0, 0x77537717);
						RTL_W32(0xeb0, 0x77537717);
					}					
#endif
				}
			}
//			RTL_W32(0x808, RTL_R32(0x808) & (~BIT(28)));
			RTL_W32(0x808, RTL_R32(0x808) | BIT(28));
            RTL_W8(0xa07, RTL_R8(0xa07) | 0xf);
			RTL_W32(0x8c0, RTL_R32(0x8c0) & (~BIT(17)));
			PHY_SetBBReg(priv, 0x830, BIT(17)|BIT(16)|BIT(15)|BIT(14)|BIT(13), 0x15);
		} else {
			bTmp = RTL_R8(0x82c);
			bTmp &= ~(BIT(0) | BIT(1));
			RTL_W8(0x82c, bTmp);
			if (GET_CHIP_VER(priv) == VERSION_8881A) {
				PHY_SetBBReg(priv, 0xcb0, 0xff, 0x00);
				PHY_SetBBReg(priv, 0xcb0, 0xf000, 0x0);
			} else {

				if (IS_TEST_CHIP(priv)) {
					RTL_W32(0xcb8, RTL_R32(0xcb8) & (~BIT(12)));
					RTL_W32(0xeb8, RTL_R32(0xcb8) & (~BIT(12)));
					RTL_W32(0xcb8, RTL_R32(0xcb8) | BIT(15));
					RTL_W32(0xeb8, RTL_R32(0xeb8) | BIT(15));
				} else {
					//RTL_W32(0xcb0, tmp_cb0 | (0x77 << 16));
					//RTL_W32(0xeb0, tmp_eb0 | (0x77 << 16));
					RTL_W32(0xcb0, 0x77777777);
					RTL_W32(0xeb0, 0x77777777);
				}
			}
			RTL_W32(0x808, RTL_R32(0x808) | BIT(28));
			RTL_W32(0x8c0, RTL_R32(0x8c0) | BIT(17));
			PHY_SetBBReg(priv, 0x830, BIT(17)|BIT(16)|BIT(15)|BIT(14)|BIT(13), 0x17);
		}
	} 

	if (channel <= 14 && (switch_bw || (priv->pshare->pre_channel == 0)))
		RTL_W8(0x454, (RTL_R8(0x454) & ~(BIT(7))));

#ifdef AC2G_256QAM
	if(is_ac2g(priv))
	{
		RTL_W32(0x8c0, RTL_R32(0x8c0) & (~BIT(17)));	//enable tx vht rates
	}
#endif
	SpurCheck8812(priv, 1, channel, 0);

	priv->pshare->pre_channel = channel;

}


/* 8812 2.4G spur at 2480MHz */
void SpurCheck8812(struct rtl8192cd_priv *priv, unsigned char ch_bw, unsigned char channel, unsigned int bandwidth)
{
	unsigned int dwTmp = 0;
	unsigned char primary_channel = 0;	
#ifdef RTK_AC_SUPPORT
	if(ch_bw == 1 && (GET_CHIP_VER(priv) != VERSION_8723B)){ /* if switch channel */
		if((priv->pshare->CurrentChannelBW == HT_CHANNEL_WIDTH_AC_40 && (channel == 11 || channel == 12)) ||
		   (priv->pshare->CurrentChannelBW == HT_CHANNEL_WIDTH_AC_20 && (channel == 13 || channel == 14))){
				RTL_W32(0x8ac,0x0ff0ff04);
				RTL_W32(0X8c4,0x40000000);
				if(priv->pshare->CurrentChannelBW == HT_CHANNEL_WIDTH_AC_40){
					dwTmp = RTL_R32(0x8ac);
					dwTmp |= BIT(0);
					RTL_W32(0x8ac, dwTmp);
				}
			}else if((priv->pshare->CurrentChannelBW == HT_CHANNEL_WIDTH_AC_40 && (priv->pshare->pre_channel == 11 || priv->pshare->pre_channel == 12)) ||
				   (priv->pshare->CurrentChannelBW == HT_CHANNEL_WIDTH_AC_20 && (priv->pshare->pre_channel == 13 || priv->pshare->pre_channel == 14))){

				/* Restore 0x8ac, 0x8c4*/
				PHY_SetBBReg(priv, 0x8ac, BIT(10), 0);	
				RTL_W32(0X8c4,0);
				primary_channel = priv->pshare->txsc_20;
				dwTmp = RTL_R32(0x8ac);			
				dwTmp &= ~(BIT(0) | BIT(1) | BIT(6) | BIT(7) | BIT(8) | BIT(9) | BIT(20) | BIT(21));
			
				switch (priv->pshare->CurrentChannelBW) {
				case HT_CHANNEL_WIDTH_AC_5:
					dwTmp |= (BIT(6) | BIT(20));
					RTL_W32(0x8ac, dwTmp);
					break;
				case HT_CHANNEL_WIDTH_AC_10:
					dwTmp |= (BIT(7) | BIT(8) | BIT(21));
					RTL_W32(0x8ac, dwTmp);
					break;
				case HT_CHANNEL_WIDTH_AC_20:
					dwTmp |= (BIT(9) | BIT(20) | BIT(21));
					RTL_W32(0x8ac, dwTmp);
					break;
				case HT_CHANNEL_WIDTH_AC_40:
					dwTmp |= (BIT(0) | BIT(9) | BIT(20) | BIT(21));
					RTL_W32(0x8ac, dwTmp);
					break;
				}
				if (IS_C_CUT_8812(priv)) {
					switch (priv->pshare->CurrentChannelBW)
					{
					case HT_CHANNEL_WIDTH_AC_5:
						PHY_SetBBReg(priv, 0x8c4, BIT(30), 0);	
						break;
					case HT_CHANNEL_WIDTH_AC_10:
						PHY_SetBBReg(priv, 0x8c4, BIT(30), 0);				
						break;
					case HT_CHANNEL_WIDTH_AC_20:		
						PHY_SetBBReg(priv, 0x8c4, BIT(30), 0);				
						break;
					case HT_CHANNEL_WIDTH_AC_40:
						PHY_SetBBReg(priv, 0x8c4, BIT(30), 0);				
						break;
					}
				}
			
				dwTmp = RTL_R32(0x8ac);
				dwTmp &= ~(BIT(2)|BIT(3)|BIT(4)|BIT(5));
			
				switch (bandwidth) {
				case HT_CHANNEL_WIDTH_AC_5:
				case HT_CHANNEL_WIDTH_AC_10:
				case HT_CHANNEL_WIDTH_AC_20:
					break;
				case HT_CHANNEL_WIDTH_AC_40:
					dwTmp |= (primary_channel << 2);
					RTL_W32(0x8ac, dwTmp);
					break;
				}
			}
	}else if(ch_bw == 0){ /* if switch bandwidth */
		if((bandwidth == HT_CHANNEL_WIDTH_AC_20 && (priv->pmib->dot11RFEntry.dot11channel == 13 || priv->pmib->dot11RFEntry.dot11channel == 14)) || 
			(bandwidth == HT_CHANNEL_WIDTH_AC_40 && (priv->pmib->dot11RFEntry.dot11channel == 11 || priv->pmib->dot11RFEntry.dot11channel == 12))
			){
			RTL_W32(0x8ac,0x0ff0ff04);
			RTL_W32(0X8c4,0x40000000);
			if(priv->pshare->CurrentChannelBW == HT_CHANNEL_WIDTH_AC_40){
				dwTmp = RTL_R32(0x8ac);
				dwTmp |= BIT(0);
				RTL_W32(0x8ac, dwTmp);
			}
		}
	}
#endif
}


signed char convert_diff(signed char value)
{
	// range from -8 ~ 7
	if (value <= 7)
		return value;
	else
		return (value - 16);
}

void Write_OFDM_A(struct rtl8192cd_priv *priv, unsigned int writeVal)
{
	RTL_W32(rTxAGC_A_Ofdm18_Ofdm6_JAguar, writeVal);
	RTL_W32(rTxAGC_A_Ofdm54_Ofdm24_JAguar, writeVal);
}

void Write_1S_A(struct rtl8192cd_priv *priv, unsigned int writeVal)
{
	RTL_W32(rTxAGC_A_MCS3_MCS0_JAguar, writeVal);
	RTL_W32(rTxAGC_A_MCS7_MCS4_JAguar, writeVal);
	RTL_W32(rTxAGC_A_Nss1Index3_Nss1Index0_JAguar, writeVal);
	RTL_W32(rTxAGC_A_Nss1Index7_Nss1Index4_JAguar, writeVal);
	RTL_W32(rTxAGC_A_Nss2Index1_Nss1Index8_JAguar, writeVal);
}

void Write_2S_A(struct rtl8192cd_priv *priv, unsigned int writeVal)
{
	RTL_W32(rTxAGC_A_MCS11_MCS8_JAguar, writeVal);
	RTL_W32(rTxAGC_A_MCS15_MCS12_JAguar, writeVal);
	RTL_W32(rTxAGC_A_Nss2Index5_Nss2Index2_JAguar, writeVal);
	RTL_W32(rTxAGC_A_Nss2Index9_Nss2Index6_JAguar, writeVal);
	writeVal = (writeVal & 0xffff0000) | (RTL_R32(rTxAGC_A_Nss2Index1_Nss1Index8_JAguar) & 0xffff);
	RTL_W32(rTxAGC_A_Nss2Index1_Nss1Index8_JAguar, writeVal);
}

void Write_HT1S_A(struct rtl8192cd_priv *priv, unsigned int writeVal)
{
	RTL_W32(rTxAGC_A_MCS3_MCS0_JAguar, writeVal);
	RTL_W32(rTxAGC_A_MCS7_MCS4_JAguar, writeVal);
}

void Write_HT2S_A(struct rtl8192cd_priv *priv, unsigned int writeVal)
{
	RTL_W32(rTxAGC_A_MCS11_MCS8_JAguar, writeVal);
	RTL_W32(rTxAGC_A_MCS15_MCS12_JAguar, writeVal);
}

void Write_VHT1S_A(struct rtl8192cd_priv *priv, unsigned int writeVal)
{
	RTL_W32(rTxAGC_A_Nss1Index3_Nss1Index0_JAguar, writeVal);
	RTL_W32(rTxAGC_A_Nss1Index7_Nss1Index4_JAguar, writeVal);
	RTL_W32(rTxAGC_A_Nss2Index1_Nss1Index8_JAguar, writeVal);
}

void Write_VHT2S_A(struct rtl8192cd_priv *priv, unsigned int writeVal)
{
	RTL_W32(rTxAGC_A_Nss2Index5_Nss2Index2_JAguar, writeVal);
	RTL_W32(rTxAGC_A_Nss2Index9_Nss2Index6_JAguar, writeVal);
	writeVal = (writeVal & 0xffff0000) | (RTL_R32(rTxAGC_A_Nss2Index1_Nss1Index8_JAguar) & 0xffff);
	RTL_W32(rTxAGC_A_Nss2Index1_Nss1Index8_JAguar, writeVal);
}
void Write_OFDM_B(struct rtl8192cd_priv *priv, unsigned int writeVal)
{
	RTL_W32(rTxAGC_B_Ofdm18_Ofdm6_JAguar, writeVal);
	RTL_W32(rTxAGC_B_Ofdm54_Ofdm24_JAguar, writeVal);
}


void Write_1S_B(struct rtl8192cd_priv *priv, unsigned int writeVal)
{
	RTL_W32(rTxAGC_B_MCS3_MCS0_JAguar, writeVal);
	RTL_W32(rTxAGC_B_MCS7_MCS4_JAguar, writeVal);
	RTL_W32(rTxAGC_B_Nss1Index3_Nss1Index0_JAguar, writeVal);
	RTL_W32(rTxAGC_B_Nss1Index7_Nss1Index4_JAguar, writeVal);
	RTL_W32(rTxAGC_B_Nss2Index1_Nss1Index8_JAguar, writeVal);
}

void Write_2S_B(struct rtl8192cd_priv *priv, unsigned int writeVal)
{
	RTL_W32(rTxAGC_B_MCS11_MCS8_JAguar, writeVal);
	RTL_W32(rTxAGC_B_MCS15_MCS12_JAguar, writeVal);
	RTL_W32(rTxAGC_B_Nss2Index5_Nss2Index2_JAguar, writeVal);
	RTL_W32(rTxAGC_B_Nss2Index9_Nss2Index6_JAguar, writeVal);
	writeVal = (writeVal & 0xffff0000) | (RTL_R32(rTxAGC_A_Nss2Index1_Nss1Index8_JAguar) & 0xffff);
	RTL_W32(rTxAGC_B_Nss2Index1_Nss1Index8_JAguar, writeVal);
}

void Write_HT1S_B(struct rtl8192cd_priv *priv, unsigned int writeVal)
{
	RTL_W32(rTxAGC_B_MCS3_MCS0_JAguar, writeVal);
	RTL_W32(rTxAGC_B_MCS7_MCS4_JAguar, writeVal);
}

void Write_HT2S_B(struct rtl8192cd_priv *priv, unsigned int writeVal)
{
	RTL_W32(rTxAGC_B_MCS11_MCS8_JAguar, writeVal);
	RTL_W32(rTxAGC_B_MCS15_MCS12_JAguar, writeVal);
}

void Write_VHT1S_B(struct rtl8192cd_priv *priv, unsigned int writeVal)
{
	RTL_W32(rTxAGC_B_Nss1Index3_Nss1Index0_JAguar, writeVal);
	RTL_W32(rTxAGC_B_Nss1Index7_Nss1Index4_JAguar, writeVal);
	RTL_W32(rTxAGC_B_Nss2Index1_Nss1Index8_JAguar, writeVal);
}

void Write_VHT2S_B(struct rtl8192cd_priv *priv, unsigned int writeVal)
{
	RTL_W32(rTxAGC_B_Nss2Index5_Nss2Index2_JAguar, writeVal);
	RTL_W32(rTxAGC_B_Nss2Index9_Nss2Index6_JAguar, writeVal);
	writeVal = (writeVal & 0xffff0000) | (RTL_R32(rTxAGC_A_Nss2Index1_Nss1Index8_JAguar) & 0xffff);
	RTL_W32(rTxAGC_B_Nss2Index1_Nss1Index8_JAguar, writeVal);
}

void use_DefaultOFDMTxPower_8812(struct rtl8192cd_priv *priv)
{

	unsigned int def_power = 0x20202020;//0x12121212;
	//printk("NO Calibration data, use default OFDM power = 0x%x\n", def_power);
	priv->pshare->rf_ft_var.bcn_pwr_idex= def_power & 0x3f;

	RTL_W32(rTxAGC_A_Ofdm18_Ofdm6_JAguar, def_power);
	RTL_W32(rTxAGC_A_Ofdm54_Ofdm24_JAguar, def_power);
	RTL_W32(rTxAGC_A_MCS3_MCS0_JAguar, def_power);
	RTL_W32(rTxAGC_A_MCS7_MCS4_JAguar, def_power);
	RTL_W32(rTxAGC_A_MCS11_MCS8_JAguar, def_power);
	RTL_W32(rTxAGC_A_MCS15_MCS12_JAguar, def_power);
	RTL_W32(rTxAGC_A_Nss1Index3_Nss1Index0_JAguar, def_power);
	RTL_W32(rTxAGC_A_Nss1Index7_Nss1Index4_JAguar, def_power);
	RTL_W32(rTxAGC_A_Nss2Index1_Nss1Index8_JAguar, def_power);
	RTL_W32(rTxAGC_A_Nss2Index5_Nss2Index2_JAguar, def_power);
	RTL_W32(rTxAGC_A_Nss2Index9_Nss2Index6_JAguar, def_power);

	RTL_W32(rTxAGC_B_Ofdm18_Ofdm6_JAguar, def_power);
	RTL_W32(rTxAGC_B_Ofdm54_Ofdm24_JAguar, def_power);
	RTL_W32(rTxAGC_B_MCS3_MCS0_JAguar, def_power);
	RTL_W32(rTxAGC_B_MCS7_MCS4_JAguar, def_power);
	RTL_W32(rTxAGC_B_MCS11_MCS8_JAguar, def_power);
	RTL_W32(rTxAGC_B_MCS15_MCS12_JAguar, def_power);
	RTL_W32(rTxAGC_B_Nss1Index3_Nss1Index0_JAguar, def_power);
	RTL_W32(rTxAGC_B_Nss1Index7_Nss1Index4_JAguar, def_power);
	RTL_W32(rTxAGC_B_Nss2Index1_Nss1Index8_JAguar, def_power);
	RTL_W32(rTxAGC_B_Nss2Index5_Nss2Index2_JAguar, def_power);
	RTL_W32(rTxAGC_B_Nss2Index9_Nss2Index6_JAguar, def_power);

}


void Cal_OFDMTxPower_5G(struct rtl8192cd_priv *priv, unsigned char ch_idx)
{
	unsigned char tmp_TPI = 0;
	unsigned char pwr_40_1s = 0, pwr_80_1s = 0;
	signed char diff_ofdm_1t = 0;
	signed char diff_bw40_2s = 0;
	signed char diff_bw20_1s = 0;
	signed char diff_bw20_2s = 0;
	signed char diff_bw80_1s = 0;
	signed char diff_bw80_2s = 0;

	unsigned int  writeVal = 0;
	unsigned char ch_idx_vht80 = get_center_channel(priv, priv->pmib->dot11RFEntry.dot11channel, priv->pmib->dot11nConfigEntry.dot11n2ndChOffset, 1) -1;
	unsigned char ch_idx_ht40 = ((priv->pmib->dot11nConfigEntry.dot11n2ndChOffset==HT_2NDCH_OFFSET_BELOW) ? (priv->pmib->dot11RFEntry.dot11channel-2) : (priv->pmib->dot11RFEntry.dot11channel+2)) - 1; 	
#ifdef POWER_PERCENT_ADJUSTMENT
	signed char pwrdiff_percent = PwrPercent2PwrLevel(priv->pmib->dot11RFEntry.power_percent);
#endif

	// PATH A, OFDM
	pwr_40_1s = priv->pmib->dot11RFEntry.pwrlevel5GHT40_1S_A[ch_idx];
#ifdef POWER_PERCENT_ADJUSTMENT
	pwr_40_1s = POWER_RANGE_CHECK(pwr_40_1s + pwrdiff_percent);
#endif
	diff_ofdm_1t = (priv->pmib->dot11RFEntry.pwrdiff_5G_20BW1S_OFDM1T_A[ch_idx] & 0x0f);
	diff_ofdm_1t = convert_diff(diff_ofdm_1t);
	tmp_TPI = POWER_RANGE_CHECK(pwr_40_1s + diff_ofdm_1t);
	writeVal = (tmp_TPI << 24) | (tmp_TPI << 16) | (tmp_TPI << 8) | tmp_TPI;
	Write_OFDM_A(priv, writeVal);

	//printk("Write_OFDM_A %d = %d + %d \n", tmp_TPI, pwr_40_1s , diff_ofdm_1t);


	// PATH B, OFDM
	pwr_40_1s = priv->pmib->dot11RFEntry.pwrlevel5GHT40_1S_B[ch_idx];
#ifdef POWER_PERCENT_ADJUSTMENT
	pwr_40_1s = POWER_RANGE_CHECK(pwr_40_1s + pwrdiff_percent);
#endif
	diff_ofdm_1t = (priv->pmib->dot11RFEntry.pwrdiff_5G_20BW1S_OFDM1T_B[ch_idx] & 0x0f);
	diff_ofdm_1t = convert_diff(diff_ofdm_1t);
	tmp_TPI = POWER_RANGE_CHECK(pwr_40_1s + diff_ofdm_1t);
	writeVal = (tmp_TPI << 24) | (tmp_TPI << 16) | (tmp_TPI << 8) | tmp_TPI;
	Write_OFDM_B(priv, writeVal);

	//printk("Write_OFDM_A %d = %d + %d \n", tmp_TPI, pwr_40_1s , diff_ofdm_1t);


	if (priv->pshare->CurrentChannelBW == 0) {
		//PATH A, BW20-1S
		pwr_40_1s = priv->pmib->dot11RFEntry.pwrlevel5GHT40_1S_A[ch_idx];
#ifdef POWER_PERCENT_ADJUSTMENT
		pwr_40_1s = POWER_RANGE_CHECK(pwr_40_1s + pwrdiff_percent);
#endif
		diff_bw20_1s = ((priv->pmib->dot11RFEntry.pwrdiff_5G_20BW1S_OFDM1T_A[ch_idx] & 0xf0) >> 4);
		diff_bw20_1s = convert_diff(diff_bw20_1s);
		tmp_TPI = POWER_RANGE_CHECK(pwr_40_1s + diff_bw20_1s);
		writeVal = (tmp_TPI << 24) | (tmp_TPI << 16) | (tmp_TPI << 8) | tmp_TPI;
		Write_1S_A(priv, writeVal);

		//printk("Write_1S_A %d = %d + %d \n", tmp_TPI, pwr_40_1s , diff_bw20_1s);


		//PATH A, BW20-2S
		diff_bw20_2s = (priv->pmib->dot11RFEntry.pwrdiff_5G_40BW2S_20BW2S_A[ch_idx] & 0x0f);
		diff_bw20_2s = convert_diff(diff_bw20_2s);
		tmp_TPI = POWER_RANGE_CHECK(pwr_40_1s + diff_bw20_1s + diff_bw20_2s);
		writeVal = (tmp_TPI << 24) | (tmp_TPI << 16) | (tmp_TPI << 8) | tmp_TPI;
		Write_2S_A(priv, writeVal);

		//printk("Write_2S_A %d = %d + %d + %d\n", tmp_TPI, pwr_40_1s , diff_bw20_1s, diff_bw20_2s);

		// ==== //

		//PATH B, BW20-1S
		pwr_40_1s = priv->pmib->dot11RFEntry.pwrlevel5GHT40_1S_B[ch_idx];
#ifdef POWER_PERCENT_ADJUSTMENT
		pwr_40_1s = POWER_RANGE_CHECK(pwr_40_1s + pwrdiff_percent);
#endif
		diff_bw20_1s = ((priv->pmib->dot11RFEntry.pwrdiff_5G_20BW1S_OFDM1T_B[ch_idx] & 0xf0) >> 4);
		diff_bw20_1s = convert_diff(diff_bw20_1s);
		tmp_TPI = POWER_RANGE_CHECK(pwr_40_1s + diff_bw20_1s);
		writeVal = (tmp_TPI << 24) | (tmp_TPI << 16) | (tmp_TPI << 8) | tmp_TPI;
		Write_1S_B(priv, writeVal);

		//printk("Write_1S_B %d = %d + %d \n", tmp_TPI, pwr_40_1s , diff_bw20_1s);

		//PATH B, BW20-2S
		diff_bw20_2s = (priv->pmib->dot11RFEntry.pwrdiff_5G_40BW2S_20BW2S_B[ch_idx] & 0x0f);
		diff_bw20_2s = convert_diff(diff_bw20_2s);
		tmp_TPI = POWER_RANGE_CHECK(pwr_40_1s + diff_bw20_1s + diff_bw20_2s);
		writeVal = (tmp_TPI << 24) | (tmp_TPI << 16) | (tmp_TPI << 8) | tmp_TPI;
		Write_2S_B(priv, writeVal);

		//printk("Write_2S_B %d = %d + %d + %d\n", tmp_TPI, pwr_40_1s , diff_bw20_1s, diff_bw20_2s);

	} else if (priv->pshare->CurrentChannelBW == 1) {
		//PATH A, BW40-1S
		pwr_40_1s = priv->pmib->dot11RFEntry.pwrlevel5GHT40_1S_A[ch_idx_ht40];
#ifdef POWER_PERCENT_ADJUSTMENT
		pwr_40_1s = POWER_RANGE_CHECK(pwr_40_1s + pwrdiff_percent);
#endif
		tmp_TPI = pwr_40_1s ;
		writeVal = (tmp_TPI << 24) | (tmp_TPI << 16) | (tmp_TPI << 8) | tmp_TPI;
		Write_1S_A(priv, writeVal);

		//printk("Write_1S_A %d = %d \n", tmp_TPI, pwr_40_1s);


		//PATH A, BW40-2S
		diff_bw40_2s = ((priv->pmib->dot11RFEntry.pwrdiff_5G_40BW2S_20BW2S_A[ch_idx_ht40] & 0xf0) >> 4);
		diff_bw40_2s = convert_diff(diff_bw40_2s);
		tmp_TPI = POWER_RANGE_CHECK(pwr_40_1s + diff_bw40_2s);
		writeVal = (tmp_TPI << 24) | (tmp_TPI << 16) | (tmp_TPI << 8) | tmp_TPI;
		Write_2S_A(priv, writeVal);

		//printk("Write_2S_A %d = %d + %d \n", tmp_TPI, pwr_40_1s , diff_bw40_2s);

		// ==== //

		//PATH B, BW40-1S
		pwr_40_1s = priv->pmib->dot11RFEntry.pwrlevel5GHT40_1S_B[ch_idx_ht40];
#ifdef POWER_PERCENT_ADJUSTMENT
		pwr_40_1s = POWER_RANGE_CHECK(pwr_40_1s + pwrdiff_percent);
#endif
		tmp_TPI = pwr_40_1s ;
		writeVal = (tmp_TPI << 24) | (tmp_TPI << 16) | (tmp_TPI << 8) | tmp_TPI;
		Write_1S_B(priv, writeVal);

		//printk("Write_1S_B %d = %d \n", tmp_TPI, pwr_40_1s);


		//PATH A, BW40-2S
		diff_bw40_2s = ((priv->pmib->dot11RFEntry.pwrdiff_5G_40BW2S_20BW2S_B[ch_idx_ht40] & 0xf0) >> 4);
		diff_bw40_2s = convert_diff(diff_bw40_2s);
		tmp_TPI = POWER_RANGE_CHECK(pwr_40_1s + diff_bw40_2s);
		writeVal = (tmp_TPI << 24) | (tmp_TPI << 16) | (tmp_TPI << 8) | tmp_TPI;
		Write_2S_B(priv, writeVal);

		//printk("Write_2S_B %d = %d + %d \n", tmp_TPI, pwr_40_1s , diff_bw40_2s);

	} else if (priv->pshare->CurrentChannelBW == 2) {

		//PATH A, BW40-1S
		//printk("ch_idx_ht40 %d >>> [0x%x  0x%x]\n", ch_idx_ht40, priv->pmib->dot11RFEntry.pwrlevel5GHT40_1S_A[ch_idx_ht40], priv->pmib->dot11RFEntry.pwrlevel5GHT40_1S_B[ch_idx_ht40]);
		pwr_40_1s = priv->pmib->dot11RFEntry.pwrlevel5GHT40_1S_A[ch_idx_ht40];
#ifdef POWER_PERCENT_ADJUSTMENT
		pwr_40_1s = POWER_RANGE_CHECK(pwr_40_1s + pwrdiff_percent);
#endif
		tmp_TPI = pwr_40_1s ;
		writeVal = (tmp_TPI << 24) | (tmp_TPI << 16) | (tmp_TPI << 8) | tmp_TPI;
		Write_HT1S_A(priv, writeVal);

		//printk("Write_HT1S_A 0x%x = 0x%x\n", tmp_TPI, pwr_40_1s);


		//PATH A, BW40-2S
		diff_bw40_2s = ((priv->pmib->dot11RFEntry.pwrdiff_5G_40BW2S_20BW2S_A[ch_idx_ht40] & 0xf0) >> 4);
		diff_bw40_2s = convert_diff(diff_bw40_2s);
		tmp_TPI = POWER_RANGE_CHECK(pwr_40_1s + diff_bw40_2s);
		writeVal = (tmp_TPI << 24) | (tmp_TPI << 16) | (tmp_TPI << 8) | tmp_TPI;
		Write_HT2S_A(priv, writeVal);

		//printk("Write_HT2S_A 0x%x = 0x%x + 0x%x \n", tmp_TPI, pwr_40_1s , diff_bw40_2s);

		// ==== //

		//PATH B, BW40-1S
		pwr_40_1s = priv->pmib->dot11RFEntry.pwrlevel5GHT40_1S_B[ch_idx_ht40];
#ifdef POWER_PERCENT_ADJUSTMENT
		pwr_40_1s = POWER_RANGE_CHECK(pwr_40_1s + pwrdiff_percent);
#endif
		tmp_TPI = pwr_40_1s ;
		writeVal = (tmp_TPI << 24) | (tmp_TPI << 16) | (tmp_TPI << 8) | tmp_TPI;
		Write_HT1S_B(priv, writeVal);

		//printk("Write_1S_B %d = %d \n", tmp_TPI, pwr_40_1s);


		//PATH A, BW40-2S
		diff_bw40_2s = ((priv->pmib->dot11RFEntry.pwrdiff_5G_40BW2S_20BW2S_B[ch_idx_ht40] & 0xf0) >> 4);
		diff_bw40_2s = convert_diff(diff_bw40_2s);
		tmp_TPI = POWER_RANGE_CHECK(pwr_40_1s + diff_bw40_2s);
		writeVal = (tmp_TPI << 24) | (tmp_TPI << 16) | (tmp_TPI << 8) | tmp_TPI;
		Write_HT2S_B(priv, writeVal);

	
		//PATH A, BW80-1S
		pwr_80_1s = (priv->pmib->dot11RFEntry.pwrlevel5GHT40_1S_A[ch_idx_vht80 - 4] +
					 priv->pmib->dot11RFEntry.pwrlevel5GHT40_1S_A[ch_idx_vht80 + 4]) / 2  ;
#ifdef POWER_PERCENT_ADJUSTMENT
		pwr_80_1s = POWER_RANGE_CHECK(pwr_80_1s + pwrdiff_percent);
#endif
		diff_bw80_1s = ((priv->pmib->dot11RFEntry.pwrdiff_5G_80BW1S_160BW1S_A[ch_idx_vht80] & 0xf0) >> 4);
		diff_bw80_1s = convert_diff(diff_bw80_1s);
		tmp_TPI = POWER_RANGE_CHECK(pwr_80_1s + diff_bw80_1s);
		writeVal = (tmp_TPI << 24) | (tmp_TPI << 16) | (tmp_TPI << 8) | tmp_TPI;
		Write_VHT1S_A(priv, writeVal);

		//printk("Write_1S_A %d = %d + %d \n", tmp_TPI, pwr_40_1s , diff_bw80_1s);


		//PATH A, BW80-2S
		diff_bw80_2s = ((priv->pmib->dot11RFEntry.pwrdiff_5G_80BW2S_160BW2S_A[ch_idx_vht80] & 0xf0) >> 4);
		diff_bw80_2s = convert_diff(diff_bw80_2s);
		tmp_TPI = POWER_RANGE_CHECK(pwr_80_1s + diff_bw80_1s + diff_bw80_2s);
		writeVal = (tmp_TPI << 24) | (tmp_TPI << 16) | (tmp_TPI << 8) | tmp_TPI;
		Write_VHT2S_A(priv, writeVal);

		//printk("Write_2S_A %d = %d + %d + %d \n", tmp_TPI, pwr_40_1s , diff_bw80_1s, diff_bw80_1s);


		//PATH B, BW80-1S
		pwr_40_1s = (priv->pmib->dot11RFEntry.pwrlevel5GHT40_1S_B[ch_idx_vht80 - 4] +
					 priv->pmib->dot11RFEntry.pwrlevel5GHT40_1S_B[ch_idx_vht80 + 4]) / 2  ;
#ifdef POWER_PERCENT_ADJUSTMENT
		pwr_40_1s = POWER_RANGE_CHECK(pwr_40_1s + pwrdiff_percent);
#endif
		diff_bw80_1s = ((priv->pmib->dot11RFEntry.pwrdiff_5G_80BW1S_160BW1S_B[ch_idx_vht80] & 0xf0) >> 4);
		diff_bw80_1s = convert_diff(diff_bw80_1s);
		tmp_TPI = POWER_RANGE_CHECK(pwr_40_1s + diff_bw80_1s);
		writeVal = (tmp_TPI << 24) | (tmp_TPI << 16) | (tmp_TPI << 8) | tmp_TPI;
		Write_VHT1S_B(priv, writeVal);

		//printk("Write_1S_B %d = %d + %d \n", tmp_TPI, pwr_40_1s , diff_bw80_1s);


		//PATH B, BW80-2S
		diff_bw80_2s = ((priv->pmib->dot11RFEntry.pwrdiff_5G_80BW2S_160BW2S_B[ch_idx_vht80] & 0xf0) >> 4);
		diff_bw80_2s = convert_diff(diff_bw80_2s);
		tmp_TPI = POWER_RANGE_CHECK(pwr_40_1s + diff_bw80_1s + diff_bw80_2s);
		writeVal = (tmp_TPI << 24) | (tmp_TPI << 16) | (tmp_TPI << 8) | tmp_TPI;
		Write_VHT2S_B(priv, writeVal);

		//printk("Write_2S_B %d = %d + %d + %d \n", tmp_TPI, pwr_40_1s , diff_bw80_1s, diff_bw80_1s);

	}
}

void Cal_OFDMTxPower_2G(struct rtl8192cd_priv *priv, unsigned char ch_idx)
{
	unsigned char tmp_TPI = 0;
	unsigned char pwr_40_1s = 0;
	signed char diff_ofdm_1t = 0;
	signed char diff_bw40_2s = 0;
	signed char diff_bw20_1s = 0;
	signed char diff_bw20_2s = 0;

	unsigned int  writeVal = 0;
	//unsigned char ch_idx_ht20 = priv->pmib->dot11RFEntry.dot11channel -1;
	
#ifdef POWER_PERCENT_ADJUSTMENT
	signed char pwrdiff_percent = PwrPercent2PwrLevel(priv->pmib->dot11RFEntry.power_percent);
#endif

	// PATH A, OFDM
	pwr_40_1s = priv->pmib->dot11RFEntry.pwrlevelHT40_1S_A[ch_idx];
#ifdef POWER_PERCENT_ADJUSTMENT
	pwr_40_1s = POWER_RANGE_CHECK(pwr_40_1s + pwrdiff_percent);
#endif
	diff_ofdm_1t = (priv->pmib->dot11RFEntry.pwrdiff_20BW1S_OFDM1T_A[ch_idx] & 0x0f);
	diff_ofdm_1t = convert_diff(diff_ofdm_1t);
	tmp_TPI = POWER_RANGE_CHECK(pwr_40_1s + diff_ofdm_1t);
	writeVal = (tmp_TPI << 24) | (tmp_TPI << 16) | (tmp_TPI << 8) | tmp_TPI;
	Write_OFDM_A(priv, writeVal);


	// PATH B, OFDM
	pwr_40_1s = priv->pmib->dot11RFEntry.pwrlevelHT40_1S_B[ch_idx];
#ifdef POWER_PERCENT_ADJUSTMENT
	pwr_40_1s = POWER_RANGE_CHECK(pwr_40_1s + pwrdiff_percent);
#endif
	diff_ofdm_1t = (priv->pmib->dot11RFEntry.pwrdiff_20BW1S_OFDM1T_B[ch_idx] & 0x0f);
	diff_ofdm_1t = convert_diff(diff_ofdm_1t);
	tmp_TPI = POWER_RANGE_CHECK(pwr_40_1s + diff_ofdm_1t);
	writeVal = (tmp_TPI << 24) | (tmp_TPI << 16) | (tmp_TPI << 8) | tmp_TPI;
	Write_OFDM_B(priv, writeVal);

	if (priv->pshare->CurrentChannelBW == 0) {
		//PATH A, BW20-1S
		pwr_40_1s = priv->pmib->dot11RFEntry.pwrlevelHT40_1S_A[ch_idx];
#ifdef POWER_PERCENT_ADJUSTMENT
		pwr_40_1s = POWER_RANGE_CHECK(pwr_40_1s + pwrdiff_percent);
#endif
		diff_bw20_1s = ((priv->pmib->dot11RFEntry.pwrdiff_20BW1S_OFDM1T_A[ch_idx] & 0xf0) >> 4);
		diff_bw20_1s = convert_diff(diff_bw20_1s);
		tmp_TPI = POWER_RANGE_CHECK(pwr_40_1s + diff_bw20_1s);
		writeVal = (tmp_TPI << 24) | (tmp_TPI << 16) | (tmp_TPI << 8) | tmp_TPI;
		Write_HT1S_A(priv, writeVal);


		//PATH A, BW20-2S
		diff_bw20_2s = (priv->pmib->dot11RFEntry.pwrdiff_40BW2S_20BW2S_A[ch_idx] & 0x0f);
		diff_bw20_2s = convert_diff(diff_bw20_2s);
		tmp_TPI = POWER_RANGE_CHECK(pwr_40_1s + diff_bw20_1s + diff_bw20_2s);
		writeVal = (tmp_TPI << 24) | (tmp_TPI << 16) | (tmp_TPI << 8) | tmp_TPI;
		Write_HT2S_A(priv, writeVal);

		// ==== //

		//PATH B, BW20-1S
		pwr_40_1s = priv->pmib->dot11RFEntry.pwrlevelHT40_1S_B[ch_idx];
#ifdef POWER_PERCENT_ADJUSTMENT
		pwr_40_1s = POWER_RANGE_CHECK(pwr_40_1s + pwrdiff_percent);
#endif
		diff_bw20_1s = ((priv->pmib->dot11RFEntry.pwrdiff_20BW1S_OFDM1T_B[ch_idx] & 0xf0) >> 4);
		diff_bw20_1s = convert_diff(diff_bw20_1s);
		tmp_TPI = POWER_RANGE_CHECK(pwr_40_1s + diff_bw20_1s);
		writeVal = (tmp_TPI << 24) | (tmp_TPI << 16) | (tmp_TPI << 8) | tmp_TPI;
		Write_HT1S_B(priv, writeVal);

		//PATH B, BW20-2S
		diff_bw20_2s = (priv->pmib->dot11RFEntry.pwrdiff_40BW2S_20BW2S_B[ch_idx] & 0x0f);
		diff_bw20_2s = convert_diff(diff_bw20_2s);
		tmp_TPI = POWER_RANGE_CHECK(pwr_40_1s + diff_bw20_1s + diff_bw20_2s);
		writeVal = (tmp_TPI << 24) | (tmp_TPI << 16) | (tmp_TPI << 8) | tmp_TPI;
		Write_HT2S_B(priv, writeVal);

	} else if (priv->pshare->CurrentChannelBW == 1) {
		//PATH A, BW40-1S
		pwr_40_1s = priv->pmib->dot11RFEntry.pwrlevelHT40_1S_A[ch_idx];
#ifdef POWER_PERCENT_ADJUSTMENT
		pwr_40_1s = POWER_RANGE_CHECK(pwr_40_1s + pwrdiff_percent);
#endif
		tmp_TPI = pwr_40_1s ;
		writeVal = (tmp_TPI << 24) | (tmp_TPI << 16) | (tmp_TPI << 8) | tmp_TPI;
		Write_HT1S_A(priv, writeVal);


		//PATH A, BW40-2S
		diff_bw40_2s = ((priv->pmib->dot11RFEntry.pwrdiff_40BW2S_20BW2S_A[ch_idx] & 0xf0) >> 4);
		diff_bw40_2s = convert_diff(diff_bw40_2s);
		tmp_TPI = POWER_RANGE_CHECK(pwr_40_1s + diff_bw40_2s);
		writeVal = (tmp_TPI << 24) | (tmp_TPI << 16) | (tmp_TPI << 8) | tmp_TPI;
		Write_HT2S_A(priv, writeVal);

		// ==== //

		//PATH B, BW40-1S
		pwr_40_1s = priv->pmib->dot11RFEntry.pwrlevelHT40_1S_B[ch_idx];
#ifdef POWER_PERCENT_ADJUSTMENT
		pwr_40_1s = POWER_RANGE_CHECK(pwr_40_1s + pwrdiff_percent);
#endif
		tmp_TPI = pwr_40_1s ;
		writeVal = (tmp_TPI << 24) | (tmp_TPI << 16) | (tmp_TPI << 8) | tmp_TPI;
		Write_HT1S_B(priv, writeVal);


		//PATH A, BW40-2S
		diff_bw40_2s = ((priv->pmib->dot11RFEntry.pwrdiff_40BW2S_20BW2S_B[ch_idx] & 0xf0) >> 4);
		diff_bw40_2s = convert_diff(diff_bw40_2s);
		tmp_TPI = POWER_RANGE_CHECK(pwr_40_1s + diff_bw40_2s);
		writeVal = (tmp_TPI << 24) | (tmp_TPI << 16) | (tmp_TPI << 8) | tmp_TPI;
		Write_HT2S_B(priv, writeVal);

	}
}
#ifdef TX_PG_8812
void get_tx_pwr(unsigned int tmp_dw, unsigned char *tmp_b1, unsigned char *tmp_b2,
				unsigned char *tmp_b3, unsigned char *tmp_b4)
{
	*tmp_b1 = (tmp_dw & (0xff));
	*tmp_b2 = ((tmp_dw & (0xff00)) >> 8);
	*tmp_b3 = ((tmp_dw & (0xff0000)) >> 16);
	*tmp_b4 = ((tmp_dw & (0xff000000)) >> 24);
}

void TxPG_CCK_8812(struct rtl8192cd_priv *priv)
{
	unsigned char b1, b2, b3, b4;
	unsigned char* tmp_b1 = &b1;
	unsigned char* tmp_b2 = &b2;
	unsigned char* tmp_b3 = &b3;
	unsigned char* tmp_b4 = &b4;
	unsigned int  tmp_dw = 0;
	unsigned int  writeVal = 0;

	tmp_dw = RTL_R32(rTxAGC_A_CCK11_CCK1_JAguar);
	get_tx_pwr(tmp_dw, tmp_b1, tmp_b2, tmp_b3, tmp_b4);
	*tmp_b1 = (unsigned char)(POWER_RANGE_CHECK(*tmp_b1 + priv->pshare->phw->CCKTxAgc_A[3]));
	*tmp_b2 = (unsigned char)(POWER_RANGE_CHECK(*tmp_b2 + priv->pshare->phw->CCKTxAgc_A[2]));
	*tmp_b3 = (unsigned char)(POWER_RANGE_CHECK(*tmp_b3 + priv->pshare->phw->CCKTxAgc_A[1]));
	*tmp_b4 = (unsigned char)(POWER_RANGE_CHECK(*tmp_b4 + priv->pshare->phw->CCKTxAgc_A[0]));
	writeVal = (*tmp_b4 << 24) | (*tmp_b3 << 16) | (*tmp_b2 << 8) | *tmp_b1;
	RTL_W32(rTxAGC_A_CCK11_CCK1_JAguar, writeVal);

	tmp_dw = RTL_R32(rTxAGC_B_CCK11_CCK1_JAguar);
	get_tx_pwr(tmp_dw, tmp_b1, tmp_b2, tmp_b3, tmp_b4);
	*tmp_b1 = (unsigned char)(POWER_RANGE_CHECK(*tmp_b1 + priv->pshare->phw->CCKTxAgc_B[3]));
	*tmp_b2 = (unsigned char)(POWER_RANGE_CHECK(*tmp_b2 + priv->pshare->phw->CCKTxAgc_B[2]));
	*tmp_b3 = (unsigned char)(POWER_RANGE_CHECK(*tmp_b3 + priv->pshare->phw->CCKTxAgc_B[1]));
	*tmp_b4 = (unsigned char)(POWER_RANGE_CHECK(*tmp_b4 + priv->pshare->phw->CCKTxAgc_B[0]));
	writeVal = (*tmp_b4 << 24) | (*tmp_b3 << 16) | (*tmp_b2 << 8) | *tmp_b1;
	RTL_W32(rTxAGC_B_CCK11_CCK1_JAguar, writeVal);
}


void TxPG_OFDM_8812_A(struct rtl8192cd_priv *priv)
{
	unsigned char b1, b2, b3, b4, b5;
	unsigned char* tmp_b1 = &b1;
	unsigned char* tmp_b2 = &b2;
	unsigned char* tmp_b3 = &b3;
	unsigned char* tmp_b4 = &b4;
	unsigned char* tmp_b5 = &b5;
	unsigned int  tmp_dw = 0;
	unsigned int  tmp_dw1 =0;
	unsigned int  writeVal = 0;

	//printk("TxPG_OFDM_8812_A!!\n");
	tmp_dw = RTL_R32(rTxAGC_A_Ofdm18_Ofdm6_JAguar);
	get_tx_pwr(tmp_dw, tmp_b1, tmp_b2, tmp_b3, tmp_b4);
	*tmp_b1 = POWER_RANGE_CHECK(*tmp_b1 + priv->pshare->phw->OFDMTxAgcOffset_A[3]);
	*tmp_b2 = POWER_RANGE_CHECK(*tmp_b2 + priv->pshare->phw->OFDMTxAgcOffset_A[2]);
	*tmp_b3 = POWER_RANGE_CHECK(*tmp_b3 + priv->pshare->phw->OFDMTxAgcOffset_A[1]);
	*tmp_b4 = POWER_RANGE_CHECK(*tmp_b4 + priv->pshare->phw->OFDMTxAgcOffset_A[0]);
	if(*tmp_b1 > priv->pshare->rf_ft_var.bcn_pwr_max)
		*tmp_b1 = priv->pshare->rf_ft_var.bcn_pwr_max;
	priv->pshare->rf_ft_var.bcn_pwr_idex= *tmp_b1;	
	writeVal = (*tmp_b4 << 24) | (*tmp_b3 << 16) | (*tmp_b2 << 8) | *tmp_b1;
	RTL_W32(rTxAGC_A_Ofdm18_Ofdm6_JAguar, writeVal);
	tmp_dw = RTL_R32(rTxAGC_A_Ofdm54_Ofdm24_JAguar);
	get_tx_pwr(tmp_dw, tmp_b1, tmp_b2, tmp_b3, tmp_b4);
	*tmp_b1 = POWER_RANGE_CHECK(*tmp_b1 + priv->pshare->phw->OFDMTxAgcOffset_A[7]);
	*tmp_b2 = POWER_RANGE_CHECK(*tmp_b2 + priv->pshare->phw->OFDMTxAgcOffset_A[6]);
	*tmp_b3 = POWER_RANGE_CHECK(*tmp_b3 + priv->pshare->phw->OFDMTxAgcOffset_A[5]);
	*tmp_b4 = POWER_RANGE_CHECK(*tmp_b4 + priv->pshare->phw->OFDMTxAgcOffset_A[4]);
	writeVal = (*tmp_b4 << 24) | (*tmp_b3 << 16) | (*tmp_b2 << 8) | *tmp_b1;
	RTL_W32(rTxAGC_A_Ofdm54_Ofdm24_JAguar, writeVal);
	tmp_dw = RTL_R32(rTxAGC_A_MCS3_MCS0_JAguar);
	get_tx_pwr(tmp_dw, tmp_b1, tmp_b2, tmp_b3, tmp_b4);
	*tmp_b1 = POWER_RANGE_CHECK(*tmp_b1 + priv->pshare->phw->MCSTxAgcOffset_A[3]);
	*tmp_b2 = POWER_RANGE_CHECK(*tmp_b2 + priv->pshare->phw->MCSTxAgcOffset_A[2]);
	*tmp_b3 = POWER_RANGE_CHECK(*tmp_b3 + priv->pshare->phw->MCSTxAgcOffset_A[1]);
	*tmp_b4 = POWER_RANGE_CHECK(*tmp_b4 + priv->pshare->phw->MCSTxAgcOffset_A[0]);
	writeVal = (*tmp_b4 << 24) | (*tmp_b3 << 16) | (*tmp_b2 << 8) | *tmp_b1;
	RTL_W32(rTxAGC_A_MCS3_MCS0_JAguar, writeVal);
	tmp_dw = RTL_R32(rTxAGC_A_MCS7_MCS4_JAguar);
	get_tx_pwr(tmp_dw, tmp_b1, tmp_b2, tmp_b3, tmp_b4);
	*tmp_b1 = POWER_RANGE_CHECK(*tmp_b1 + priv->pshare->phw->MCSTxAgcOffset_A[7]);
	*tmp_b2 = POWER_RANGE_CHECK(*tmp_b2 + priv->pshare->phw->MCSTxAgcOffset_A[6]);
	*tmp_b3 = POWER_RANGE_CHECK(*tmp_b3 + priv->pshare->phw->MCSTxAgcOffset_A[5]);
	*tmp_b4 = POWER_RANGE_CHECK(*tmp_b4 + priv->pshare->phw->MCSTxAgcOffset_A[4]);
	writeVal = (*tmp_b4 << 24) | (*tmp_b3 << 16) | (*tmp_b2 << 8) | *tmp_b1;
	RTL_W32(rTxAGC_A_MCS7_MCS4_JAguar, writeVal);
	tmp_dw = RTL_R32(rTxAGC_A_MCS11_MCS8_JAguar);
	get_tx_pwr(tmp_dw, tmp_b1, tmp_b2, tmp_b3, tmp_b4);
	*tmp_b1 = POWER_RANGE_CHECK(*tmp_b1 + priv->pshare->phw->MCSTxAgcOffset_A[11]);
	*tmp_b2 = POWER_RANGE_CHECK(*tmp_b2 + priv->pshare->phw->MCSTxAgcOffset_A[10]);
	*tmp_b3 = POWER_RANGE_CHECK(*tmp_b3 + priv->pshare->phw->MCSTxAgcOffset_A[9]);
	*tmp_b4 = POWER_RANGE_CHECK(*tmp_b4 + priv->pshare->phw->MCSTxAgcOffset_A[8]);
	writeVal = (*tmp_b4 << 24) | (*tmp_b3 << 16) | (*tmp_b2 << 8) | *tmp_b1;
	RTL_W32(rTxAGC_A_MCS11_MCS8_JAguar, writeVal);
	tmp_dw = RTL_R32(rTxAGC_A_MCS15_MCS12_JAguar);
	get_tx_pwr(tmp_dw, tmp_b1, tmp_b2, tmp_b3, tmp_b4);
	*tmp_b1 = POWER_RANGE_CHECK(*tmp_b1 + priv->pshare->phw->MCSTxAgcOffset_A[15]);
	*tmp_b2 = POWER_RANGE_CHECK(*tmp_b2 + priv->pshare->phw->MCSTxAgcOffset_A[14]);
	*tmp_b3 = POWER_RANGE_CHECK(*tmp_b3 + priv->pshare->phw->MCSTxAgcOffset_A[13]);
	*tmp_b4 = POWER_RANGE_CHECK(*tmp_b4 + priv->pshare->phw->MCSTxAgcOffset_A[12]);
	writeVal = (*tmp_b4 << 24) | (*tmp_b3 << 16) | (*tmp_b2 << 8) | *tmp_b1;
	RTL_W32(rTxAGC_A_MCS15_MCS12_JAguar, writeVal);
	tmp_dw = RTL_R32(rTxAGC_A_Nss1Index3_Nss1Index0_JAguar);
	get_tx_pwr(tmp_dw, tmp_b1, tmp_b2, tmp_b3, tmp_b4);
	*tmp_b1 = POWER_RANGE_CHECK(*tmp_b1 + priv->pshare->phw->VHTTxAgcOffset_A[3]);
	*tmp_b2 = POWER_RANGE_CHECK(*tmp_b2 + priv->pshare->phw->VHTTxAgcOffset_A[2]);
	*tmp_b3 = POWER_RANGE_CHECK(*tmp_b3 + priv->pshare->phw->VHTTxAgcOffset_A[1]);
	*tmp_b4 = POWER_RANGE_CHECK(*tmp_b4 + priv->pshare->phw->VHTTxAgcOffset_A[0]);
	writeVal = (*tmp_b4 << 24) | (*tmp_b3 << 16) | (*tmp_b2 << 8) | *tmp_b1;
	RTL_W32(rTxAGC_A_Nss1Index3_Nss1Index0_JAguar, writeVal);
	tmp_dw = RTL_R32(rTxAGC_A_Nss1Index7_Nss1Index4_JAguar);
	get_tx_pwr(tmp_dw, tmp_b1, tmp_b2, tmp_b3, tmp_b4);
	*tmp_b1 = POWER_RANGE_CHECK(*tmp_b1 + priv->pshare->phw->VHTTxAgcOffset_A[7]);
	*tmp_b2 = POWER_RANGE_CHECK(*tmp_b2 + priv->pshare->phw->VHTTxAgcOffset_A[6]);
	*tmp_b3 = POWER_RANGE_CHECK(*tmp_b3 + priv->pshare->phw->VHTTxAgcOffset_A[5]);
	*tmp_b4 = POWER_RANGE_CHECK(*tmp_b4 + priv->pshare->phw->VHTTxAgcOffset_A[4]);
	writeVal = (*tmp_b4 << 24) | (*tmp_b3 << 16) | (*tmp_b2 << 8) | *tmp_b1;
	RTL_W32(rTxAGC_A_Nss1Index7_Nss1Index4_JAguar, writeVal);

	tmp_dw = RTL_R32(rTxAGC_A_Nss2Index1_Nss1Index8_JAguar);
	tmp_dw1= RTL_R32(rTxAGC_A_Nss1Index7_Nss1Index4_JAguar);
	get_tx_pwr(tmp_dw1, tmp_b1, tmp_b2, tmp_b3, tmp_b5);
	get_tx_pwr(tmp_dw, tmp_b1, tmp_b2, tmp_b3, tmp_b4);
	*tmp_b1 = POWER_RANGE_CHECK(*tmp_b5 - priv->pshare->phw->VHTTxAgcOffset_A[11]);
	*tmp_b2 = POWER_RANGE_CHECK(*tmp_b5 - priv->pshare->phw->VHTTxAgcOffset_A[10]);
	*tmp_b3 = POWER_RANGE_CHECK(*tmp_b3 + priv->pshare->phw->VHTTxAgcOffset_A[9]);
	*tmp_b4 = POWER_RANGE_CHECK(*tmp_b4 + priv->pshare->phw->VHTTxAgcOffset_A[8]);
	writeVal = (*tmp_b4 << 24) | (*tmp_b3 << 16) | (*tmp_b2 << 8) | *tmp_b1;
	RTL_W32(rTxAGC_A_Nss2Index1_Nss1Index8_JAguar, writeVal);

	tmp_dw = RTL_R32(rTxAGC_A_Nss2Index5_Nss2Index2_JAguar);
	get_tx_pwr(tmp_dw, tmp_b1, tmp_b2, tmp_b3, tmp_b4);
	*tmp_b1 = POWER_RANGE_CHECK(*tmp_b1 + priv->pshare->phw->VHTTxAgcOffset_A[15]);
	*tmp_b2 = POWER_RANGE_CHECK(*tmp_b2 + priv->pshare->phw->VHTTxAgcOffset_A[14]);
	*tmp_b3 = POWER_RANGE_CHECK(*tmp_b3 + priv->pshare->phw->VHTTxAgcOffset_A[13]);
	*tmp_b4 = POWER_RANGE_CHECK(*tmp_b4 + priv->pshare->phw->VHTTxAgcOffset_A[12]);
	writeVal = (*tmp_b4 << 24) | (*tmp_b3 << 16) | (*tmp_b2 << 8) | *tmp_b1;
	RTL_W32(rTxAGC_A_Nss2Index5_Nss2Index2_JAguar, writeVal);

	tmp_dw = RTL_R32(rTxAGC_A_Nss2Index9_Nss2Index6_JAguar);
	get_tx_pwr(tmp_dw, tmp_b1, tmp_b2, tmp_b3, tmp_b4);
	*tmp_b1 = POWER_RANGE_CHECK(*tmp_b1 + priv->pshare->phw->VHTTxAgcOffset_A[19]);
	*tmp_b2 = POWER_RANGE_CHECK(*tmp_b2 + priv->pshare->phw->VHTTxAgcOffset_A[18]);
	*tmp_b3 = POWER_RANGE_CHECK(*tmp_b3 - priv->pshare->phw->VHTTxAgcOffset_A[17]);
	*tmp_b4 = POWER_RANGE_CHECK(*tmp_b4 - priv->pshare->phw->VHTTxAgcOffset_A[16]);
	writeVal = (*tmp_b4 << 24) | (*tmp_b3 << 16) | (*tmp_b2 << 8) | *tmp_b1;
	RTL_W32(rTxAGC_A_Nss2Index9_Nss2Index6_JAguar, writeVal);
}


void TxPG_OFDM_8812_B(struct rtl8192cd_priv *priv)
{
	unsigned char b1, b2, b3, b4, b5;
	unsigned char* tmp_b1 = &b1;
	unsigned char* tmp_b2 = &b2;
	unsigned char* tmp_b3 = &b3;
	unsigned char* tmp_b4 = &b4;
	unsigned char* tmp_b5 = &b5;
	unsigned int  tmp_dw = 0;
	unsigned int  tmp_dw1 =0;
	unsigned int  writeVal = 0;

	//printk("TxPG_OFDM_8812_B!!\n");
	tmp_dw = RTL_R32(rTxAGC_B_Ofdm18_Ofdm6_JAguar);
	get_tx_pwr(tmp_dw, tmp_b1, tmp_b2, tmp_b3, tmp_b4);
	*tmp_b1 = POWER_RANGE_CHECK(*tmp_b1 + priv->pshare->phw->OFDMTxAgcOffset_B[3]);
	*tmp_b2 = POWER_RANGE_CHECK(*tmp_b2 + priv->pshare->phw->OFDMTxAgcOffset_B[2]);
	*tmp_b3 = POWER_RANGE_CHECK(*tmp_b3 + priv->pshare->phw->OFDMTxAgcOffset_B[1]);
	*tmp_b4 = POWER_RANGE_CHECK(*tmp_b4 + priv->pshare->phw->OFDMTxAgcOffset_B[0]);
	if(*tmp_b1 > priv->pshare->rf_ft_var.bcn_pwr_max)
		*tmp_b1 = priv->pshare->rf_ft_var.bcn_pwr_max;
	if(*tmp_b1 > priv->pshare->rf_ft_var.bcn_pwr_idex)
		priv->pshare->rf_ft_var.bcn_pwr_idex= *tmp_b1;
	writeVal = (*tmp_b4 << 24) | (*tmp_b3 << 16) | (*tmp_b2 << 8) | *tmp_b1;
	RTL_W32(rTxAGC_B_Ofdm18_Ofdm6_JAguar, writeVal);
	tmp_dw = RTL_R32(rTxAGC_B_Ofdm54_Ofdm24_JAguar);
	get_tx_pwr(tmp_dw, tmp_b1, tmp_b2, tmp_b3, tmp_b4);
	*tmp_b1 = POWER_RANGE_CHECK(*tmp_b1 + priv->pshare->phw->OFDMTxAgcOffset_B[7]);
	*tmp_b2 = POWER_RANGE_CHECK(*tmp_b2 + priv->pshare->phw->OFDMTxAgcOffset_B[6]);
	*tmp_b3 = POWER_RANGE_CHECK(*tmp_b3 + priv->pshare->phw->OFDMTxAgcOffset_B[5]);
	*tmp_b4 = POWER_RANGE_CHECK(*tmp_b4 + priv->pshare->phw->OFDMTxAgcOffset_B[4]);
	writeVal = (*tmp_b4 << 24) | (*tmp_b3 << 16) | (*tmp_b2 << 8) | *tmp_b1;
	RTL_W32(rTxAGC_B_Ofdm54_Ofdm24_JAguar, writeVal);
	tmp_dw = RTL_R32(rTxAGC_B_MCS3_MCS0_JAguar);
	get_tx_pwr(tmp_dw, tmp_b1, tmp_b2, tmp_b3, tmp_b4);
	*tmp_b1 = POWER_RANGE_CHECK(*tmp_b1 + priv->pshare->phw->MCSTxAgcOffset_B[3]);
	*tmp_b2 = POWER_RANGE_CHECK(*tmp_b2 + priv->pshare->phw->MCSTxAgcOffset_B[2]);
	*tmp_b3 = POWER_RANGE_CHECK(*tmp_b3 + priv->pshare->phw->MCSTxAgcOffset_B[1]);
	*tmp_b4 = POWER_RANGE_CHECK(*tmp_b4 + priv->pshare->phw->MCSTxAgcOffset_B[0]);
	writeVal = (*tmp_b4 << 24) | (*tmp_b3 << 16) | (*tmp_b2 << 8) | *tmp_b1;
	RTL_W32(rTxAGC_B_MCS3_MCS0_JAguar, writeVal);
	tmp_dw = RTL_R32(rTxAGC_B_MCS7_MCS4_JAguar);
	get_tx_pwr(tmp_dw, tmp_b1, tmp_b2, tmp_b3, tmp_b4);
	*tmp_b1 = POWER_RANGE_CHECK(*tmp_b1 + priv->pshare->phw->MCSTxAgcOffset_B[7]);
	*tmp_b2 = POWER_RANGE_CHECK(*tmp_b2 + priv->pshare->phw->MCSTxAgcOffset_B[6]);
	*tmp_b3 = POWER_RANGE_CHECK(*tmp_b3 + priv->pshare->phw->MCSTxAgcOffset_B[5]);
	*tmp_b4 = POWER_RANGE_CHECK(*tmp_b4 + priv->pshare->phw->MCSTxAgcOffset_B[4]);
	writeVal = (*tmp_b4 << 24) | (*tmp_b3 << 16) | (*tmp_b2 << 8) | *tmp_b1;
	RTL_W32(rTxAGC_B_MCS7_MCS4_JAguar, writeVal);
	tmp_dw = RTL_R32(rTxAGC_B_MCS11_MCS8_JAguar);
	get_tx_pwr(tmp_dw, tmp_b1, tmp_b2, tmp_b3, tmp_b4);
	*tmp_b1 = POWER_RANGE_CHECK(*tmp_b1 + priv->pshare->phw->MCSTxAgcOffset_B[11]);
	*tmp_b2 = POWER_RANGE_CHECK(*tmp_b2 + priv->pshare->phw->MCSTxAgcOffset_B[10]);
	*tmp_b3 = POWER_RANGE_CHECK(*tmp_b3 + priv->pshare->phw->MCSTxAgcOffset_B[9]);
	*tmp_b4 = POWER_RANGE_CHECK(*tmp_b4 + priv->pshare->phw->MCSTxAgcOffset_B[8]);
	writeVal = (*tmp_b4 << 24) | (*tmp_b3 << 16) | (*tmp_b2 << 8) | *tmp_b1;
	RTL_W32(rTxAGC_B_MCS11_MCS8_JAguar, writeVal);
	tmp_dw = RTL_R32(rTxAGC_B_MCS15_MCS12_JAguar);
	get_tx_pwr(tmp_dw, tmp_b1, tmp_b2, tmp_b3, tmp_b4);
	*tmp_b1 = POWER_RANGE_CHECK(*tmp_b1 + priv->pshare->phw->MCSTxAgcOffset_B[15]);
	*tmp_b2 = POWER_RANGE_CHECK(*tmp_b2 + priv->pshare->phw->MCSTxAgcOffset_B[14]);
	*tmp_b3 = POWER_RANGE_CHECK(*tmp_b3 + priv->pshare->phw->MCSTxAgcOffset_B[13]);
	*tmp_b4 = POWER_RANGE_CHECK(*tmp_b4 + priv->pshare->phw->MCSTxAgcOffset_B[12]);
	writeVal = (*tmp_b4 << 24) | (*tmp_b3 << 16) | (*tmp_b2 << 8) | *tmp_b1;
	RTL_W32(rTxAGC_B_MCS15_MCS12_JAguar, writeVal);
	tmp_dw = RTL_R32(rTxAGC_B_Nss1Index3_Nss1Index0_JAguar);
	get_tx_pwr(tmp_dw, tmp_b1, tmp_b2, tmp_b3, tmp_b4);
	*tmp_b1 = POWER_RANGE_CHECK(*tmp_b1 + priv->pshare->phw->VHTTxAgcOffset_B[3]);
	*tmp_b2 = POWER_RANGE_CHECK(*tmp_b2 + priv->pshare->phw->VHTTxAgcOffset_B[2]);
	*tmp_b3 = POWER_RANGE_CHECK(*tmp_b3 + priv->pshare->phw->VHTTxAgcOffset_B[1]);
	*tmp_b4 = POWER_RANGE_CHECK(*tmp_b4 + priv->pshare->phw->VHTTxAgcOffset_B[0]);
	writeVal = (*tmp_b4 << 24) | (*tmp_b3 << 16) | (*tmp_b2 << 8) | *tmp_b1;
	RTL_W32(rTxAGC_B_Nss1Index3_Nss1Index0_JAguar, writeVal);
	tmp_dw = RTL_R32(rTxAGC_B_Nss1Index7_Nss1Index4_JAguar);
	get_tx_pwr(tmp_dw, tmp_b1, tmp_b2, tmp_b3, tmp_b4);
	*tmp_b1 = POWER_RANGE_CHECK(*tmp_b1 + priv->pshare->phw->VHTTxAgcOffset_B[7]);
	*tmp_b2 = POWER_RANGE_CHECK(*tmp_b2 + priv->pshare->phw->VHTTxAgcOffset_B[6]);
	*tmp_b3 = POWER_RANGE_CHECK(*tmp_b3 + priv->pshare->phw->VHTTxAgcOffset_B[5]);
	*tmp_b4 = POWER_RANGE_CHECK(*tmp_b4 + priv->pshare->phw->VHTTxAgcOffset_B[4]);
	writeVal = (*tmp_b4 << 24) | (*tmp_b3 << 16) | (*tmp_b2 << 8) | *tmp_b1;
	RTL_W32(rTxAGC_B_Nss1Index7_Nss1Index4_JAguar, writeVal);
	tmp_dw = RTL_R32(rTxAGC_B_Nss2Index1_Nss1Index8_JAguar);
	tmp_dw1= RTL_R32(rTxAGC_B_Nss1Index7_Nss1Index4_JAguar);
	get_tx_pwr(tmp_dw1, tmp_b1, tmp_b2, tmp_b3, tmp_b5);
	get_tx_pwr(tmp_dw, tmp_b1, tmp_b2, tmp_b3, tmp_b4);
	*tmp_b1 = POWER_RANGE_CHECK(*tmp_b5 - priv->pshare->phw->VHTTxAgcOffset_B[11]);
	*tmp_b2 = POWER_RANGE_CHECK(*tmp_b5 - priv->pshare->phw->VHTTxAgcOffset_B[10]);
	*tmp_b3 = POWER_RANGE_CHECK(*tmp_b3 + priv->pshare->phw->VHTTxAgcOffset_B[9]);
	*tmp_b4 = POWER_RANGE_CHECK(*tmp_b4 + priv->pshare->phw->VHTTxAgcOffset_B[8]);
	writeVal = (*tmp_b4 << 24) | (*tmp_b3 << 16) | (*tmp_b2 << 8) | *tmp_b1;
	RTL_W32(rTxAGC_B_Nss2Index1_Nss1Index8_JAguar, writeVal);
	tmp_dw = RTL_R32(rTxAGC_B_Nss2Index5_Nss2Index2_JAguar);
	get_tx_pwr(tmp_dw, tmp_b1, tmp_b2, tmp_b3, tmp_b4);
	*tmp_b1 = POWER_RANGE_CHECK(*tmp_b1 + priv->pshare->phw->VHTTxAgcOffset_B[15]);
	*tmp_b2 = POWER_RANGE_CHECK(*tmp_b2 + priv->pshare->phw->VHTTxAgcOffset_B[14]);
	*tmp_b3 = POWER_RANGE_CHECK(*tmp_b3 + priv->pshare->phw->VHTTxAgcOffset_B[13]);
	*tmp_b4 = POWER_RANGE_CHECK(*tmp_b4 + priv->pshare->phw->VHTTxAgcOffset_B[12]);
	writeVal = (*tmp_b4 << 24) | (*tmp_b3 << 16) | (*tmp_b2 << 8) | *tmp_b1;
	RTL_W32(rTxAGC_B_Nss2Index5_Nss2Index2_JAguar, writeVal);
	tmp_dw = RTL_R32(rTxAGC_B_Nss2Index9_Nss2Index6_JAguar);
	get_tx_pwr(tmp_dw, tmp_b1, tmp_b2, tmp_b3, tmp_b4);
	*tmp_b1 = POWER_RANGE_CHECK(*tmp_b1 + priv->pshare->phw->VHTTxAgcOffset_B[19]);
	*tmp_b2 = POWER_RANGE_CHECK(*tmp_b2 + priv->pshare->phw->VHTTxAgcOffset_B[18]);
	*tmp_b3 = POWER_RANGE_CHECK(*tmp_b3 - priv->pshare->phw->VHTTxAgcOffset_B[17]);
	*tmp_b4 = POWER_RANGE_CHECK(*tmp_b4 - priv->pshare->phw->VHTTxAgcOffset_B[16]);
	writeVal = (*tmp_b4 << 24) | (*tmp_b3 << 16) | (*tmp_b2 << 8) | *tmp_b1;
	RTL_W32(rTxAGC_B_Nss2Index9_Nss2Index6_JAguar, writeVal);
}
#endif


#ifdef TXPWR_LMT_8812

unsigned char get_byte_from_dw(unsigned int tmp_dw, unsigned char byte_num)
{

	if (byte_num == 0)
		return (tmp_dw & (0xff));
	else if (byte_num == 1)
		return ((tmp_dw & (0xff00)) >> 8);
	else if (byte_num == 2)
		return ((tmp_dw & (0xff0000)) >> 16);
	else if (byte_num == 3)
		return ((tmp_dw & (0xff000000)) >> 24);
	else
		return 0;
}

unsigned int assign_lmt_reg_value(struct rtl8192cd_priv *priv, unsigned int reg_offset, unsigned char max_idx, unsigned char max_idx2)
{

	unsigned int tmp_dw; 
	unsigned char reg_val[4];
	unsigned char i=0;

	tmp_dw = RTL_R32(reg_offset);

	//printk("[0x%03x] 0x%08x >> ", reg_offset, tmp_dw);
	
	get_tx_pwr(tmp_dw, (unsigned char *)&reg_val[0], (unsigned char *)&reg_val[1],
		(unsigned char *)&reg_val[2], (unsigned char *)&reg_val[3]);

	if((reg_offset == rTxAGC_A_Nss2Index1_Nss1Index8_JAguar) ||
		(reg_offset == rTxAGC_B_Nss2Index1_Nss1Index8_JAguar))
	{
			reg_val[0] = POWER_MIN_CHECK(reg_val[0], max_idx);
			reg_val[1] = POWER_MIN_CHECK(reg_val[1], max_idx);
			reg_val[2] = POWER_MIN_CHECK(reg_val[2], max_idx2);
			reg_val[3] = POWER_MIN_CHECK(reg_val[3], max_idx2);
	}
	else
	{
		for(i=0; i<4; i++)
		{
			reg_val[i] = POWER_MIN_CHECK(reg_val[i], max_idx);
		}
	}

	for(i=0; i<4; i++)
	{
		reg_val[i] = POWER_RANGE_CHECK(reg_val[i]);
	}

	tmp_dw = (reg_val[3] << 24) | (reg_val[2] << 16) | (reg_val[1] << 8) | reg_val[0];

	//printk("0x%08x\n", tmp_dw);

	RTL_W32(reg_offset, tmp_dw);

}

unsigned char check_lmt_valid(struct rtl8192cd_priv *priv, unsigned char phy_band)
{
	unsigned char lmt_valid = 1;
	
	if(phy_band == PHY_BAND_2G)
		if(priv->pshare->txpwr_lmt_CCK == 0)
			lmt_valid = 0;

	if(priv->pshare->txpwr_lmt_OFDM == 0)
		lmt_valid = 0;

	if(priv->pshare->txpwr_lmt_HT1S == 0)
		lmt_valid = 0;
	
	if(priv->pshare->txpwr_lmt_HT2S == 0)
		lmt_valid = 0;
	
	if(phy_band == PHY_BAND_5G)
	{
		if(priv->pshare->txpwr_lmt_VHT1S == 0)
			lmt_valid = 0;

		if(priv->pshare->txpwr_lmt_VHT2S == 0)
			lmt_valid = 0;
	}

	if(lmt_valid == 0)
	DEBUG_WARN("NO Limit Value, NO limit TX POWER !!!\n limit for cck=%d, ofdm=%d, ht1s=%d, ht2s=%d, vht1s=%d, vht2s=%d\n",
		priv->pshare->txpwr_lmt_CCK, 
		priv->pshare->txpwr_lmt_OFDM, 
		priv->pshare->txpwr_lmt_HT1S,
		priv->pshare->txpwr_lmt_HT2S, 
		priv->pshare->txpwr_lmt_VHT1S,
		priv->pshare->txpwr_lmt_VHT2S
	);

	return lmt_valid;

}


void TxLMT_CCK_8812_A(struct rtl8192cd_priv *priv)
{

	unsigned int  tmp_dw = 0;
	unsigned char	idx_cck_11m = 0;
	char	lmt_pg_idx_cck = 0;
	char	max_lmt_idx_cck = 0;

	lmt_pg_idx_cck = priv->pshare->txpwr_lmt_CCK - priv->pshare->tgpwr_CCK_new[0];

	//Cal Max tx pwr idx for CCK
	tmp_dw = RTL_R32(rTxAGC_A_CCK11_CCK1_JAguar);
	idx_cck_11m = get_byte_from_dw(tmp_dw, 3);
	max_lmt_idx_cck = idx_cck_11m + lmt_pg_idx_cck;
	max_lmt_idx_cck = POWER_MIN_INDEX(max_lmt_idx_cck);

	assign_lmt_reg_value(priv, rTxAGC_A_CCK11_CCK1_JAguar, max_lmt_idx_cck, 0);
}

void TxLMT_CCK_8812_B(struct rtl8192cd_priv *priv)
{

	unsigned int  tmp_dw = 0;
	unsigned char	idx_cck_11m = 0;
	char	lmt_pg_idx_cck = 0;
	char	max_lmt_idx_cck = 0;

	lmt_pg_idx_cck = priv->pshare->txpwr_lmt_CCK - priv->pshare->tgpwr_CCK_new[0];

	//Cal Max tx pwr idx for CCK
	tmp_dw = RTL_R32(rTxAGC_B_CCK11_CCK1_JAguar);
	idx_cck_11m = get_byte_from_dw(tmp_dw, 3);
	max_lmt_idx_cck = idx_cck_11m + lmt_pg_idx_cck;
	max_lmt_idx_cck = POWER_MIN_INDEX(max_lmt_idx_cck);

	assign_lmt_reg_value(priv, rTxAGC_B_CCK11_CCK1_JAguar, max_lmt_idx_cck, 0);
}


void TxLMT_OFDM_8812_A(struct rtl8192cd_priv *priv)
{

	unsigned int  tmp_dw = 0;

	unsigned char	idx_cck_11m = 0;
	unsigned char	idx_odfm_54m = 0;
	unsigned char	idx_ht_mcs7 = 0;
	unsigned char	idx_ht_mcs15 = 0;
	unsigned char	idx_vht_1ss_mcs7 =0;
	unsigned char	idx_vht_2ss_mcs7 =0;

	char	lmt_pg_idx_cck, lmt_pg_idx_ofdm = 0;
	char	lmt_pg_idx_ht1s, lmt_pg_idx_ht2s = 0;
	char	lmt_pg_idx_vht1s, lmt_pg_idx_vht2s = 0;

	char	max_lmt_idx_cck, max_lmt_idx_ofdm = 0;
	char	max_lmt_idx_ht1s, max_lmt_idx_ht2s = 0;
	char	max_lmt_idx_vht1s, max_lmt_idx_vht2s = 0;

	lmt_pg_idx_cck = priv->pshare->txpwr_lmt_CCK - priv->pshare->tgpwr_CCK_new[0];
	lmt_pg_idx_ofdm = priv->pshare->txpwr_lmt_OFDM - priv->pshare->tgpwr_OFDM_new[0];
	lmt_pg_idx_ht1s = priv->pshare->txpwr_lmt_HT1S - priv->pshare->tgpwr_HT1S_new[0];
	lmt_pg_idx_ht2s = priv->pshare->txpwr_lmt_HT2S - priv->pshare->tgpwr_HT2S_new[0];
	lmt_pg_idx_vht1s = priv->pshare->txpwr_lmt_VHT1S - priv->pshare->tgpwr_VHT1S_new[0];
	lmt_pg_idx_vht2s = priv->pshare->txpwr_lmt_VHT2S - priv->pshare->tgpwr_VHT2S_new[0];

	//printk("%d %d %d %d %d %d\n", lmt_pg_idx_cck, lmt_pg_idx_ofdm,
		//lmt_pg_idx_ht1s, lmt_pg_idx_ht2s, lmt_pg_idx_vht1s, lmt_pg_idx_vht2s);


	//Cal Max tx pwr idx for CCK
	tmp_dw = RTL_R32(rTxAGC_A_CCK11_CCK1_JAguar);
	idx_cck_11m = get_byte_from_dw(tmp_dw, 3);
	max_lmt_idx_cck = idx_cck_11m + lmt_pg_idx_cck;
	max_lmt_idx_cck = POWER_MIN_INDEX(max_lmt_idx_cck);
	//Cal Max tx pwr idx for OFDM
	tmp_dw = RTL_R32(rTxAGC_A_Ofdm54_Ofdm24_JAguar);
	idx_odfm_54m = get_byte_from_dw(tmp_dw, 3);
	max_lmt_idx_ofdm = idx_odfm_54m + lmt_pg_idx_ofdm;
	max_lmt_idx_ofdm = POWER_MIN_INDEX(max_lmt_idx_ofdm);

	//Cal Max tx pwr idx for HT1S
	tmp_dw = RTL_R32(rTxAGC_A_MCS7_MCS4_JAguar);
	idx_ht_mcs7 = get_byte_from_dw(tmp_dw, 3);
	max_lmt_idx_ht1s = idx_ht_mcs7 + lmt_pg_idx_ht1s;
	max_lmt_idx_ht1s = POWER_MIN_INDEX(max_lmt_idx_ht1s);

	//Cal Max tx pwr idx for HT2S
	tmp_dw = RTL_R32(rTxAGC_A_MCS15_MCS12_JAguar);
	idx_ht_mcs15 = get_byte_from_dw(tmp_dw, 3);
	max_lmt_idx_ht2s = idx_ht_mcs15 + lmt_pg_idx_ht2s;
	max_lmt_idx_ht2s = POWER_MIN_INDEX(max_lmt_idx_ht2s);

	//Cal Max tx pwr idx for VHT1S
	tmp_dw = RTL_R32(rTxAGC_A_Nss1Index7_Nss1Index4_JAguar);
	idx_vht_1ss_mcs7 = get_byte_from_dw(tmp_dw, 3);
	max_lmt_idx_vht1s = idx_vht_1ss_mcs7 + lmt_pg_idx_vht1s;
	max_lmt_idx_vht1s = POWER_MIN_INDEX(max_lmt_idx_vht1s);

	//Cal Max tx pwr idx for VHT2S
	tmp_dw = RTL_R32(rTxAGC_A_Nss2Index9_Nss2Index6_JAguar);
	idx_vht_2ss_mcs7 = get_byte_from_dw(tmp_dw, 1);
	max_lmt_idx_vht2s = idx_vht_2ss_mcs7 + lmt_pg_idx_vht2s;
	max_lmt_idx_vht2s = POWER_MIN_INDEX(max_lmt_idx_vht2s);
	
	assign_lmt_reg_value(priv, rTxAGC_A_CCK11_CCK1_JAguar, max_lmt_idx_cck, 0);
	
	assign_lmt_reg_value(priv, rTxAGC_A_Ofdm18_Ofdm6_JAguar, max_lmt_idx_ofdm, 0);
	assign_lmt_reg_value(priv, rTxAGC_A_Ofdm54_Ofdm24_JAguar, max_lmt_idx_ofdm, 0);
	
	assign_lmt_reg_value(priv, rTxAGC_A_MCS3_MCS0_JAguar, max_lmt_idx_ht1s, 0);
	assign_lmt_reg_value(priv, rTxAGC_A_MCS7_MCS4_JAguar, max_lmt_idx_ht1s, 0);
	assign_lmt_reg_value(priv, rTxAGC_A_MCS11_MCS8_JAguar, max_lmt_idx_ht2s, 0);
	assign_lmt_reg_value(priv, rTxAGC_A_MCS15_MCS12_JAguar, max_lmt_idx_ht2s, 0);

	assign_lmt_reg_value(priv, rTxAGC_A_Nss1Index3_Nss1Index0_JAguar, max_lmt_idx_vht1s, 0);
	assign_lmt_reg_value(priv, rTxAGC_A_Nss1Index7_Nss1Index4_JAguar, max_lmt_idx_vht1s, 0);
	assign_lmt_reg_value(priv, rTxAGC_A_Nss2Index1_Nss1Index8_JAguar, max_lmt_idx_vht1s, max_lmt_idx_vht2s);
	assign_lmt_reg_value(priv, rTxAGC_A_Nss2Index5_Nss2Index2_JAguar, max_lmt_idx_vht2s, 0);
	assign_lmt_reg_value(priv, rTxAGC_A_Nss2Index9_Nss2Index6_JAguar, max_lmt_idx_vht2s, 0);

}


void TxLMT_OFDM_8812_B(struct rtl8192cd_priv *priv)
{

	unsigned int  tmp_dw = 0;

	unsigned char	idx_cck_11m = 0;
	unsigned char	idx_odfm_54m = 0;
	unsigned char	idx_ht_mcs7 = 0;
	unsigned char	idx_ht_mcs15 = 0;
	unsigned char	idx_vht_1ss_mcs7 =0;
	unsigned char	idx_vht_2ss_mcs7 =0;

	char	lmt_pg_idx_cck, lmt_pg_idx_ofdm = 0;
	char	lmt_pg_idx_ht1s, lmt_pg_idx_ht2s = 0;
	char	lmt_pg_idx_vht1s, lmt_pg_idx_vht2s = 0;

	char	max_lmt_idx_cck, max_lmt_idx_ofdm = 0;
	char	max_lmt_idx_ht1s, max_lmt_idx_ht2s = 0;
	char	max_lmt_idx_vht1s, max_lmt_idx_vht2s = 0;

	lmt_pg_idx_cck = priv->pshare->txpwr_lmt_CCK - priv->pshare->tgpwr_CCK_new[1];
	lmt_pg_idx_ofdm = priv->pshare->txpwr_lmt_OFDM - priv->pshare->tgpwr_OFDM_new[1];
	lmt_pg_idx_ht1s = priv->pshare->txpwr_lmt_HT1S - priv->pshare->tgpwr_HT1S_new[1];
	lmt_pg_idx_ht2s = priv->pshare->txpwr_lmt_HT2S - priv->pshare->tgpwr_HT2S_new[1];
	lmt_pg_idx_vht1s = priv->pshare->txpwr_lmt_VHT1S - priv->pshare->tgpwr_VHT1S_new[1];
	lmt_pg_idx_vht2s = priv->pshare->txpwr_lmt_VHT2S - priv->pshare->tgpwr_VHT2S_new[1];

	//printk("%d %d %d %d %d %d\n", lmt_pg_idx_cck, lmt_pg_idx_ofdm,
		//lmt_pg_idx_ht1s, lmt_pg_idx_ht2s, lmt_pg_idx_vht1s, lmt_pg_idx_vht2s);

	//Cal Max tx pwr idx for CCK
	tmp_dw = RTL_R32(rTxAGC_B_CCK11_CCK1_JAguar);
	idx_cck_11m = get_byte_from_dw(tmp_dw, 3);
	max_lmt_idx_cck = idx_cck_11m + lmt_pg_idx_cck;
	max_lmt_idx_cck = POWER_MIN_INDEX(max_lmt_idx_cck);

	//Cal Max tx pwr idx for OFDM
	tmp_dw = RTL_R32(rTxAGC_B_Ofdm54_Ofdm24_JAguar);
	idx_odfm_54m = get_byte_from_dw(tmp_dw, 3);
	max_lmt_idx_ofdm = idx_odfm_54m + lmt_pg_idx_ofdm;
	max_lmt_idx_ofdm = POWER_MIN_INDEX(max_lmt_idx_ofdm);

	//Cal Max tx pwr idx for HT1S
	tmp_dw = RTL_R32(rTxAGC_B_MCS7_MCS4_JAguar);
	idx_ht_mcs7 = get_byte_from_dw(tmp_dw, 3);
	max_lmt_idx_ht1s = idx_ht_mcs7 + lmt_pg_idx_ht1s;
	max_lmt_idx_ht1s = POWER_MIN_INDEX(max_lmt_idx_ht1s);

	//Cal Max tx pwr idx for HT2S
	tmp_dw = RTL_R32(rTxAGC_B_MCS15_MCS12_JAguar);
	idx_ht_mcs15 = get_byte_from_dw(tmp_dw, 3);
	max_lmt_idx_ht2s = idx_ht_mcs15 + lmt_pg_idx_ht2s;
	max_lmt_idx_ht2s = POWER_MIN_INDEX(max_lmt_idx_ht2s);

	//Cal Max tx pwr idx for VHT1S
	tmp_dw = RTL_R32(rTxAGC_B_Nss1Index7_Nss1Index4_JAguar);
	idx_vht_1ss_mcs7 = get_byte_from_dw(tmp_dw, 3);
	max_lmt_idx_vht1s = idx_vht_1ss_mcs7 + lmt_pg_idx_vht1s;
	max_lmt_idx_vht1s = POWER_MIN_INDEX(max_lmt_idx_vht1s);

	//Cal Max tx pwr idx for VHT2S
	tmp_dw = RTL_R32(rTxAGC_B_Nss2Index9_Nss2Index6_JAguar);
	idx_vht_2ss_mcs7 = get_byte_from_dw(tmp_dw, 1);
	max_lmt_idx_vht2s = idx_vht_2ss_mcs7 + lmt_pg_idx_vht2s;
	max_lmt_idx_vht2s = POWER_MIN_INDEX(max_lmt_idx_vht2s);

	assign_lmt_reg_value(priv, rTxAGC_B_CCK11_CCK1_JAguar, max_lmt_idx_cck, 0);
	
	assign_lmt_reg_value(priv, rTxAGC_B_Ofdm18_Ofdm6_JAguar, max_lmt_idx_ofdm, 0);
	assign_lmt_reg_value(priv, rTxAGC_B_Ofdm54_Ofdm24_JAguar, max_lmt_idx_ofdm, 0);
	
	assign_lmt_reg_value(priv, rTxAGC_B_MCS3_MCS0_JAguar, max_lmt_idx_ht1s, 0);
	assign_lmt_reg_value(priv, rTxAGC_B_MCS7_MCS4_JAguar, max_lmt_idx_ht1s, 0);
	assign_lmt_reg_value(priv, rTxAGC_B_MCS11_MCS8_JAguar, max_lmt_idx_ht2s, 0);
	assign_lmt_reg_value(priv, rTxAGC_B_MCS15_MCS12_JAguar, max_lmt_idx_ht2s, 0);

	assign_lmt_reg_value(priv, rTxAGC_B_Nss1Index3_Nss1Index0_JAguar, max_lmt_idx_vht1s, 0);
	assign_lmt_reg_value(priv, rTxAGC_B_Nss1Index7_Nss1Index4_JAguar, max_lmt_idx_vht1s, 0);
	assign_lmt_reg_value(priv, rTxAGC_B_Nss2Index1_Nss1Index8_JAguar, max_lmt_idx_vht1s, max_lmt_idx_vht2s);
	assign_lmt_reg_value(priv, rTxAGC_B_Nss2Index5_Nss2Index2_JAguar, max_lmt_idx_vht2s, 0);
	assign_lmt_reg_value(priv, rTxAGC_B_Nss2Index9_Nss2Index6_JAguar, max_lmt_idx_vht2s, 0);

}


#endif

void PHY_SetOFDMTxPower_8812(struct rtl8192cd_priv *priv, unsigned char channel)
{
	unsigned char ch_idx = 0;
	//	unsigned char tmp_TPI = 0;
	unsigned char phy_band = 0;
	u4Byte pwrdiff = 0x06060606;

	if (channel > 0)
		ch_idx = (channel - 1);
	else {
		printk("Error Channel !!\n");
		return;
	}

	if (channel > 14)
		phy_band = PHY_BAND_5G;
	else
		phy_band = PHY_BAND_2G;

#if 0
	printk("pwrlevel5GHT40_1S_A[%d]= %d \n", channel, priv->pmib->dot11RFEntry.pwrlevel5GHT40_1S_A[ch_idx]);
	printk("pwrdiff_5G_20BW1S_OFDM1T_A[%d]= 0x%x \n", channel, priv->pmib->dot11RFEntry.pwrdiff_5G_20BW1S_OFDM1T_A[ch_idx]);
	printk("pwrdiff_40BW2S_20BW2S_A[%d]= 0x%x \n", channel, priv->pmib->dot11RFEntry.pwrdiff_40BW2S_20BW2S_A[ch_idx]);
	printk("pwrdiff_5G_20BW1S_OFDM1T_A[%d]= 0x%x \n", channel, priv->pmib->dot11RFEntry.pwrdiff_5G_20BW1S_OFDM1T_A[ch_idx]);
	printk("pwrdiff_5G_40BW2S_20BW2S_A[%d]= 0x%x \n", channel, priv->pmib->dot11RFEntry.pwrdiff_5G_40BW2S_20BW2S_A[ch_idx]);
	printk("pwrdiff_5G_80BW1S_160BW1S_A[%d]= 0x%x \n", channel, priv->pmib->dot11RFEntry.pwrdiff_5G_80BW1S_160BW1S_A[ch_idx]);
	printk("pwrdiff_5G_80BW2S_160BW2S_A[%d]= 0x%x \n", channel, priv->pmib->dot11RFEntry.pwrdiff_5G_80BW2S_160BW2S_A[ch_idx]);

	printk("pwrlevel5GHT40_1S_B[%d]= %d \n", channel, priv->pmib->dot11RFEntry.pwrlevel5GHT40_1S_B[ch_idx]);
	printk("pwrdiff_5G_20BW1S_OFDM1T_B[%d]= 0x%x \n", channel, priv->pmib->dot11RFEntry.pwrdiff_5G_20BW1S_OFDM1T_B[ch_idx]);
	printk("pwrdiff_40BW2S_20BW2S_B[%d]= 0x%x \n", channel, priv->pmib->dot11RFEntry.pwrdiff_40BW2S_20BW2S_B[ch_idx]);
	printk("pwrdiff_5G_20BW1S_OFDM1T_B[%d]= 0x%x \n", channel, priv->pmib->dot11RFEntry.pwrdiff_5G_20BW1S_OFDM1T_B[ch_idx]);
	printk("pwrdiff_5G_40BW2S_20BW2S_B[%d]= 0x%x \n", channel, priv->pmib->dot11RFEntry.pwrdiff_5G_40BW2S_20BW2S_B[ch_idx]);
	printk("pwrdiff_5G_80BW1S_160BW1S_B[%d]= 0x%x \n", channel, priv->pmib->dot11RFEntry.pwrdiff_5G_80BW1S_160BW1S_B[ch_idx]);
	printk("pwrdiff_5G_80BW2S_160BW2S_B[%d]= 0x%x \n", channel, priv->pmib->dot11RFEntry.pwrdiff_5G_80BW2S_160BW2S_B[ch_idx]);
#endif

	if (phy_band == PHY_BAND_5G) {
		if ((priv->pmib->dot11RFEntry.pwrlevel5GHT40_1S_A[ch_idx] == 0)
			|| (priv->pmib->dot11RFEntry.pwrlevel5GHT40_1S_B[ch_idx] == 0)) {
			use_DefaultOFDMTxPower_8812(priv);
			return;
		}		
		Cal_OFDMTxPower_5G(priv, ch_idx);
	} else if (phy_band == PHY_BAND_2G) {
		if ((priv->pmib->dot11RFEntry.pwrlevelHT40_1S_A[ch_idx] == 0)
			|| (priv->pmib->dot11RFEntry.pwrlevelHT40_1S_B[ch_idx] == 0)) {
			use_DefaultOFDMTxPower_8812(priv);
			return;
		}	
		Cal_OFDMTxPower_2G(priv, ch_idx);
	}

#ifdef TX_PG_8812
	TxPG_OFDM_8812_A(priv);
	TxPG_OFDM_8812_B(priv);
#endif

#ifdef TXPWR_LMT_8812
	if (!priv->pshare->rf_ft_var.disable_txpwrlmt)
	{
		if (check_lmt_valid(priv, phy_band))
		{
			TxLMT_OFDM_8812_A(priv);
			TxLMT_OFDM_8812_B(priv);
		}		
	}		
#endif
	{
		int ofdm6a = RTL_R32(rTxAGC_A_Ofdm18_Ofdm6_JAguar) & 0x3f;
		int ofdm6b = RTL_R32(rTxAGC_B_Ofdm18_Ofdm6_JAguar) & 0x3f;
		if(ofdm6a>priv->pshare->rf_ft_var.bcn_pwr_max) {
			ofdm6a = priv->pshare->rf_ft_var.bcn_pwr_max;
			RTL_W8(rTxAGC_A_Ofdm18_Ofdm6_JAguar, ofdm6a);
		}
		if(ofdm6b>priv->pshare->rf_ft_var.bcn_pwr_max) {
			ofdm6b = priv->pshare->rf_ft_var.bcn_pwr_max;
			RTL_W8(rTxAGC_B_Ofdm18_Ofdm6_JAguar, ofdm6b);
		}				
		priv->pshare->rf_ft_var.bcn_pwr_idex= max(ofdm6a,ofdm6b);
	}	

}

void PHY_SetCCKTxPower_8812(struct rtl8192cd_priv *priv, unsigned char channel)
{
	unsigned int def_power = 0x20202020;//0x12121212;
	unsigned char ch_idx = 0;
	unsigned char tmp_TPI = 0;
	unsigned char phy_band = 0;
	unsigned int  writeVal = 0;
	u4Byte pwrdiff = 0x06060606;
#ifdef POWER_PERCENT_ADJUSTMENT
	signed char pwrdiff_percent = PwrPercent2PwrLevel(priv->pmib->dot11RFEntry.power_percent);
#endif

	if (channel > 0)
		ch_idx = (channel - 1);
	else {
		printk("Error Channel !!\n");
		return;
	}

	if (channel > 14)
		phy_band = PHY_BAND_5G;
	else
		phy_band = PHY_BAND_2G;

	if ((priv->pmib->dot11RFEntry.pwrlevelCCK_A[ch_idx] == 0)
			&& (priv->pmib->dot11RFEntry.pwrlevelCCK_B[ch_idx] == 0)) {
		//printk("NO Calibration data, use default CCK power = 0x%x\n", def_power);
		RTL_W32(rTxAGC_A_CCK11_CCK1_JAguar, def_power);
		RTL_W32(rTxAGC_B_CCK11_CCK1_JAguar, def_power);
		return;
	}

	if (phy_band == PHY_BAND_2G) {
		//printk("pwrlevelCCK_A[%d]= %d \n", ch_idx, priv->pmib->dot11RFEntry.pwrlevelCCK_A[ch_idx]);
		//printk("pwrlevelCCK_B[%d]= %d \n", ch_idx, priv->pmib->dot11RFEntry.pwrlevelCCK_B[ch_idx]);

		//PATH A
		tmp_TPI = priv->pmib->dot11RFEntry.pwrlevelCCK_A[ch_idx];
#ifdef POWER_PERCENT_ADJUSTMENT
		tmp_TPI = POWER_RANGE_CHECK(tmp_TPI + pwrdiff_percent);
#endif
		writeVal = (tmp_TPI << 24) | (tmp_TPI << 16) | (tmp_TPI << 8) | tmp_TPI;
		RTL_W32(rTxAGC_A_CCK11_CCK1_JAguar, writeVal);

		//PATH B
		tmp_TPI = priv->pmib->dot11RFEntry.pwrlevelCCK_B[ch_idx];
#ifdef POWER_PERCENT_ADJUSTMENT
		tmp_TPI = POWER_RANGE_CHECK(tmp_TPI + pwrdiff_percent);
#endif
		writeVal = (tmp_TPI << 24) | (tmp_TPI << 16) | (tmp_TPI << 8) | tmp_TPI;
		RTL_W32(rTxAGC_B_CCK11_CCK1_JAguar, writeVal);
	}
#ifdef TX_PG_8812
	TxPG_CCK_8812(priv);
#endif

#ifdef TXPWR_LMT_8812	
	if (!priv->pshare->rf_ft_var.disable_txpwrlmt)
	{			
		if (check_lmt_valid(priv, phy_band))
		{
			TxLMT_CCK_8812_A(priv);
			TxLMT_CCK_8812_B(priv);
		}
	}
#endif	

}








// Firmware

#define MAX_PAGE_SIZE				4096	// @ page : 4k bytes
#define FW_START_ADDRESS			0x1000
#define FWDL_ChkSum_rpt				BIT(2)

extern unsigned char *data_rtl8812fw_start, *data_rtl8812fw_end;
extern unsigned char *data_rtl8812fw_n_start, *data_rtl8812fw_n_end; //for_8812_mp_chip
#ifdef AC2G_256QAM
extern unsigned char *data_rtl8812fw_n_2g_start, *data_rtl8812fw_n_2g_end;
#endif


VOID
_8051Reset8812(
	struct rtl8192cd_priv *priv
)
{
	u1Byte	u1bTmp;
	u1bTmp = RTL_R8(REG_SYS_FUNC_EN_8812 + 1);
	RTL_W8(REG_SYS_FUNC_EN_8812 + 1, u1bTmp & (~BIT2));
	RTL_W8( REG_SYS_FUNC_EN_8812 + 1, u1bTmp | (BIT2));
//	RT_TRACE(COMP_INIT, DBG_LOUD, ("=====> _8051Reset8812(): 8051 reset success .\n"));
}


VOID
_FWDownloadEnable_8812(
	struct rtl8192cd_priv *priv,
	BOOLEAN			enable
)
{
	u1Byte	tmp;

	if (enable) {
		// MCU firmware download enable.
		RTL_W8( REG_MCUFWDL_8812, 0x05);

		// Clear Rom DL enable
		tmp = RTL_R8( REG_MCUFWDL_8812 + 2);
		RTL_W8( REG_MCUFWDL_8812 + 2, tmp & 0xf7);
	} else {
		// MCU firmware download enable.
		tmp = RTL_R8( REG_MCUFWDL_8812);
		RTL_W8( REG_MCUFWDL_8812, tmp & 0xfe);
	}
}

VOID
_FillDummy_8812(
	pu1Byte		pFwBuf,
	pu4Byte		pFwLen
)
{
	u4Byte	FwLen = *pFwLen;
	u1Byte	remain = (u1Byte)(FwLen % 4);
	remain = (remain == 0) ? 0 : (4 - remain);

	while (remain > 0) {
		pFwBuf[FwLen] = 0;
		FwLen++;
		remain--;
	}

	*pFwLen = FwLen;
}



// BlockWrite:
// 92DU----------use 64-Byte/8-Byte/1-Byte (PlatformIOWriteNByte)
// 92CU/8723U----use 4-Byte/1-Byte  (PlatformIOWriteNByte)
// PCI/SDIO------use 4-Byte/1-Byte  (PlatformEFIOWrite4Byte)
// 92CU [MacOS]-- use 196-Byte/8-Byte/1-Byte  (PlatformIOWriteNByte)

VOID
_BlockWrite_8812(
	struct rtl8192cd_priv 	*priv,
	IN		PVOID			buffer,
	IN		u4Byte			buffSize
)
{
	u4Byte			blockSize_p1 = 4;	// (Default) Phase #1 : PCI muse use 4-byte write to download FW
	u4Byte			blockSize_p2 = 8;	// Phase #2 : Use 8-byte, if Phase#1 use big size to write FW.
	u4Byte			blockSize_p3 = 1;	// Phase #3 : Use 1-byte, the remnant of FW image.
	u4Byte			blockCount_p1 = 0, blockCount_p2 = 0, blockCount_p3 = 0;
	u4Byte			remainSize_p1 = 0, remainSize_p2 = 0;
	pu1Byte			bufferPtr	= (pu1Byte)buffer;
	u4Byte			i = 0, offset = 0;


	//3 Phase #1
	blockCount_p1 = buffSize / blockSize_p1;
	remainSize_p1 = buffSize % blockSize_p1;


	for (i = 0 ; i < blockCount_p1 ; i++) {
		RTL_W32( (FW_START_ADDRESS + i * blockSize_p1), cpu_to_le32(*((pu4Byte)(bufferPtr + i * blockSize_p1))));
	}

	//3 Phase #2
	if (remainSize_p1) {
		offset = blockCount_p1 * blockSize_p1;

		blockCount_p2 = remainSize_p1 / blockSize_p2;
		remainSize_p2 = remainSize_p1 % blockSize_p2;

	}

	//3 Phase #3
	if (remainSize_p2) {
		offset = (blockCount_p1 * blockSize_p1) + (blockCount_p2 * blockSize_p2);
		blockCount_p3 = remainSize_p2 / blockSize_p3;

		//		RT_TRACE(COMP_INIT,DBG_LOUD,("_BlockWrite_8812[P3]  ::buffSize_p3( %d) blockSize_p3( %d) blockCount_p3( %d) \n",(buffSize-offset),blockSize_p3, blockCount_p3));

		for (i = 0 ; i < blockCount_p3 ; i++) {
			RTL_W8( (FW_START_ADDRESS + offset + i), *(bufferPtr + offset + i));
		}
	}
}


VOID
_PageWrite_8812(
	struct rtl8192cd_priv 	*priv,
	IN		u4Byte			page,
	IN		PVOID			buffer,
	IN		u4Byte			size
)
{
	u1Byte value8;
	u1Byte u8Page = (u1Byte) (page & 0x07) ;

	value8 = (RTL_R8(REG_MCUFWDL_8812 + 2) & 0xF8 ) | u8Page ;
	RTL_W8(REG_MCUFWDL_8812 + 2, value8);

	_BlockWrite_8812(priv, buffer, size);
}


VOID
_WriteFW_8812(
	struct rtl8192cd_priv *priv,
	IN		PVOID			buffer,
	IN		u4Byte			size
)
{
	// Since we need dynamic decide method of dwonload fw, so we call this function to get chip version.
	// We can remove _ReadChipVersion from ReadAdapterInfo8192C later.
	u4Byte 			pageNums, remainSize ;
	u4Byte 			page, offset;
	pu1Byte			bufferPtr = (pu1Byte)buffer;

#if 1//DEV_BUS_TYPE==RT_PCI_INTERFACE
	// 20100120 Joseph: Add for 88CE normal chip.
	// Fill in zero to make firmware image to dword alignment.
	//	_FillDummy_8812(bufferPtr, &size);
#endif

	pageNums = size / MAX_PAGE_SIZE ;

	//	RT_ASSERT((pageNums <= 8), ("Page numbers should not greater then 8 \n"));

	remainSize = size % MAX_PAGE_SIZE;

	for (page = 0; page < pageNums;  page++) {
		offset = page * MAX_PAGE_SIZE;
		_PageWrite_8812(priv, page, (bufferPtr + offset), MAX_PAGE_SIZE);
	}
	if (remainSize) {
		offset = pageNums * MAX_PAGE_SIZE;
		page = pageNums;
		_PageWrite_8812(priv, page, (bufferPtr + offset), remainSize);
	}
	//	RT_TRACE(COMP_INIT, DBG_LOUD, ("_WriteFW_8812 Done- for Normal chip.\n"));
}


RT_STATUS
_FWFreeToGo8812(
	struct rtl8192cd_priv *priv

)
{
	u4Byte	counter = 0;
	u4Byte	value32;

	// polling CheckSum report
	do {
		value32 = RTL_R32( REG_MCUFWDL_8812);
	} while ((counter ++ < 6000) && (!(value32 & FWDL_ChkSum_rpt  )));

	if (counter >= 24000) {
		//		RT_TRACE(COMP_INIT, DBG_SERIOUS, ("_FWFreeToGo8812:: chksum report faill ! REG_MCUFWDL:0x%08x .\n",value32));
		return RT_STATUS_FAILURE;
	}
	//	RT_TRACE(COMP_INIT, DBG_LOUD, ("_FWFreeToGo8812:: Checksum report OK ! REG_MCUFWDL:0x%08x .\n",value32));

	value32 = RTL_R32( REG_MCUFWDL_8812);
	value32 |= MCUFWDL_RDY;
	value32 &= ~WINTINI_RDY;
	RTL_W32( REG_MCUFWDL_8812, value32);

	_8051Reset8812(priv);

	// polling for FW ready
	counter = 0;
	do {
		if (RTL_R32( REG_MCUFWDL_8812) & WINTINI_RDY) {
			//			RT_TRACE(COMP_INIT, DBG_SERIOUS, ("Polling FW ready success!! REG_MCUFWDL:0x%08x in %d times.\n",PlatformEFIORead4Byte(Adapter, REG_MCUFWDL_8812),counter));
			return RT_STATUS_SUCCESS;
		}
		//		PlatformStallExecution(5);
		delay_us(5);
	} while (counter++ < 24000);

	panic_printk("Polling FW ready fail!! REG_MCUFWDL:0x%08x .\n", RTL_R32( REG_MCUFWDL_8812) );
	return RT_STATUS_FAILURE;
}


RT_STATUS
FirmwareDownload8812(
	struct rtl8192cd_priv *priv
)
{
	RT_STATUS		rtStatus = RT_STATUS_SUCCESS;

	//u4Byte			FwImageLen = 0;
	u1Byte			*pFirmwareBuf;
	u4Byte			FirmwareLen;

	RTL_W8( 0xf0, (RTL_R8(0xf0)& ~BIT(7)));
#ifdef CONFIG_RTL_8812_SUPPORT
	if (IS_TEST_CHIP(priv)) { //for_8812_mp_chip
		pFirmwareBuf = data_rtl8812fw_start + 32;
		FirmwareLen = data_rtl8812fw_end - data_rtl8812fw_start - 32;
#ifdef AC2G_256QAM
	} else if (is_ac2g(priv)) {
		pFirmwareBuf = data_rtl8812fw_n_2g_start + 32;
		FirmwareLen = data_rtl8812fw_n_2g_end - data_rtl8812fw_n_2g_start - 32;
#endif
	} else {
		pFirmwareBuf = data_rtl8812fw_n_start + 32;
		FirmwareLen = data_rtl8812fw_n_end - data_rtl8812fw_n_start - 32;
	}
#endif
#ifdef CONFIG_RTL_8723B_SUPPORT
		if (GET_CHIP_VER(priv) == VERSION_8723B) { 
			pFirmwareBuf = data_rtl8723bfw_start + 32;
			FirmwareLen = data_rtl8723bfw_end - data_rtl8723bfw_start - 32;
		}
#endif


	// panic_printk("%s, %d%x, %x\n", __FUNCTION__, __LINE__, pFirmwareBuf, FirmwareLen);


	{
		if (RTL_R8( REG_MCUFWDL_8812)&BIT7) { //8051 RAM code
			RTL_W8(REG_MCUFWDL_8812, 0x00);
			_8051Reset8812(priv);
		}
	}



	_FWDownloadEnable_8812(priv, TRUE);

	_WriteFW_8812(priv, pFirmwareBuf, FirmwareLen);

	_FWDownloadEnable_8812(priv, FALSE);


	rtStatus = _FWFreeToGo8812(priv);

	if (RT_STATUS_SUCCESS != rtStatus) {
		panic_printk("Firmware is not ready to run!\n") ;
		goto Exit;
	}


Exit:

	DEBUG_INFO(" <=== FirmwareDownload8812()\n");
	return rtStatus;

}


// 8812 H2C

BOOLEAN
CheckFwReadLastH2C_8812(
	struct rtl8192cd_priv *priv,
	IN	u1Byte			BoxNum
)
{
	u1Byte	valHMETFR;
	BOOLEAN	Result = FALSE;

	valHMETFR = RTL_R8( REG_HMETFR_8812);

	// Do not seperate to 91C and 88C, we use the same setting. Suggested by SD4 Filen. 2009.12.03.
	if (((valHMETFR >> BoxNum)&BIT0) == 0)
		Result = TRUE;

	return Result;
}


u1Byte
FillH2CCmd8812(
	struct rtl8192cd_priv *priv,
	IN	u1Byte 		ElementID,
	IN	u4Byte 		CmdLen,
	IN	pu1Byte		pCmdBuffer
)
{

	u1Byte 	ioStatus = 0;
	u1Byte	BoxNum;
	u2Byte	BOXReg = 0, BOXExtReg = 0;
	//	u1Byte	U1btmp; //Read 0x1bf
	BOOLEAN bFwReadClear = FALSE;
	u1Byte	BufIndex = 0;
	u1Byte	WaitH2cLimmit = 0;
	u1Byte	BoxContent[4], BoxExtContent[4];
	u1Byte	idx = 0;

	if (!GET_ROOT(priv)->bFWReady)
		return 1;

	// 1. Find the last BOX number which has been writen.
	BoxNum = priv->pshare->fw_q_fifo_count;	//pHalData->LastHMEBoxNum;
	switch (BoxNum) {
		case 0:
			BOXReg = REG_HMEBOX_0_8812;
			BOXExtReg = REG_HMEBOX_EXT0_8812;
			break;
		case 1:
			BOXReg = REG_HMEBOX_1_8812;
			BOXExtReg = REG_HMEBOX_EXT1_8812;
			break;
		case 2:
			BOXReg = REG_HMEBOX_2_8812;
			BOXExtReg = REG_HMEBOX_EXT2_8812;
			break;
		case 3:
			BOXReg = REG_HMEBOX_3_8812;
			BOXExtReg = REG_HMEBOX_EXT3_8812;
			break;
		default:
			break;
	}

	// 2. Check if the box content is empty.
	while (!bFwReadClear) {
		bFwReadClear = CheckFwReadLastH2C_8812(priv, BoxNum);
		if (WaitH2cLimmit++ >= 100) {
			ioStatus = 1;
			return ioStatus;
		} else if (!bFwReadClear) {
			delay_us(10); //us
		}
	}

	// 4. Fill the H2C cmd into box
	memset(BoxContent, 0, sizeof(BoxContent));
	memset(BoxExtContent, 0, sizeof(BoxExtContent));

	BoxContent[0] = ElementID; // Fill element ID
	//	RTPRINT(FFW, FW_MSG_H2C_CONTENT, ("[FW], Write ElementID BOXReg(%4x) = %2x \n", BOXReg, ElementID));

	switch (CmdLen) {
		case 1:
		case 2:
		case 3: {
			//BoxContent[0] &= ~(BIT7);
			memcpy((pu1Byte)(BoxContent) + 1, pCmdBuffer + BufIndex, CmdLen);
			//For Endian Free.
			for (idx = 0; idx < 4; idx++) {
				RTL_W8(BOXReg + idx, BoxContent[idx]);
			}
			break;
		}
		case 4:
		case 5:
		case 6:
		case 7: {
			//BoxContent[0] |= (BIT7);
			memcpy((pu1Byte)(BoxExtContent), pCmdBuffer + BufIndex + 3, (CmdLen - 3));
			memcpy((pu1Byte)(BoxContent) + 1, pCmdBuffer + BufIndex, 3);
			//For Endian Free.
			for (idx = 0 ; idx < 4 ; idx ++) {
				RTL_W8( BOXExtReg + idx, BoxExtContent[idx]);
			}
			for (idx = 0 ; idx < 4 ; idx ++) {
				RTL_W8( BOXReg + idx, BoxContent[idx]);
			}
			break;
		}

		default:
			//		RTPRINT(FFW, FW_MSG_H2C_STATE, ("[FW], Invalid command len=%d!!!\n", CmdLen));
			ioStatus = 2;
			return ioStatus;
			break;
	}

	if (++priv->pshare->fw_q_fifo_count > 3)
		priv->pshare->fw_q_fifo_count = 0;

	//	RTPRINT(FFW, FW_MSG_H2C_CONTENT, ("[FW], pHalData->LastHMEBoxNum = %d\n", pHalData->LastHMEBoxNum));
	return ioStatus;
}







VOID
SetBcnCtrlReg_8812(
	struct rtl8192cd_priv *priv,
	IN	u1Byte		SetBits,
	IN	u1Byte		ClearBits
)
{
	u1Byte tmp = RTL_R8(REG_BCN_CTRL_8812);

	tmp |=  SetBits;
	tmp &= ~ClearBits;

	RTL_W8(REG_BCN_CTRL_8812, tmp);
}


u1Byte
MRateIdxToARFRId8812(
	struct rtl8192cd_priv *priv,
	u1Byte			RateIdx,
	u1Byte			RfType
)
{
	u1Byte Ret = 0;

	switch (RateIdx) {

		case RATR_INX_WIRELESS_NGB:
			if (RfType == MIMO_1T1R)
				Ret = 1;
			else
				Ret = 0;
			break;

		case RATR_INX_WIRELESS_N:
		case RATR_INX_WIRELESS_NG:
			if (RfType == MIMO_1T1R)
				Ret = 5;
			else
				Ret = 4;
			break;

		case RATR_INX_WIRELESS_NB:
			if (RfType == MIMO_1T1R)
				Ret = 3;
			else
				Ret = 2;
			break;

		case RATR_INX_WIRELESS_GB:
			Ret = 6;
			break;

		case RATR_INX_WIRELESS_G:
			Ret = 7;
			break;

		case RATR_INX_WIRELESS_B:
			Ret = 8;
			break;

		case RATR_INX_WIRELESS_MC:
			if (!(priv->pmib->dot11BssType.net_work_type & WIRELESS_11A))
				Ret = 6;
			else
				Ret = 7;
			break;
		case RATR_INX_WIRELESS_AC_N:
			if (RfType == MIMO_1T1R)
				Ret = 10;
			else
				Ret = 9;
			break;

		default:
			Ret = 0;
			break;
	}

	return Ret;
}

u1Byte
Get_RA_BW(
	BOOLEAN 	bCurTxBW80MHz,
	BOOLEAN		bCurTxBW40MHz
)
{
	u1Byte	BW = 0;
	if (bCurTxBW80MHz)
		BW = 2;
	else if (bCurTxBW40MHz)
		BW = 1;
	else
		BW = 0;

	return BW;
}



#if 0
typedef enum _WIRELESS_MODE {
	WIRELESS_MODE_UNKNOWN = 0x00,
	WIRELESS_MODE_A = 0x01,
	WIRELESS_MODE_B = 0x02,
	WIRELESS_MODE_G = 0x04,
	WIRELESS_MODE_AUTO = 0x08,
	WIRELESS_MODE_N_24G = 0x10,
	WIRELESS_MODE_N_5G = 0x20,
	WIRELESS_MODE_AC_5G = 0x40
} WIRELESS_MODE;
#endif

#define MIMO_PS_STATIC				0
#define MIMO_PS_DYNAMIC			1
#define MIMO_PS_NOLIMIT			3

#define FillOctetString(_os,_octet,_len)		\
	(_os).Octet=(pu1Byte)(_octet);			\
	(_os).Length=(_len);


u1Byte
Get_VHT_ENI(
	u4Byte			IOTAction,
	u1Byte			WirelessMode,
	u4Byte			ratr_bitmap
)
{
	u1Byte Ret = 0;
	if (WirelessMode < WIRELESS_MODE_N_24G)
		Ret =  0;
	else if (WirelessMode == WIRELESS_MODE_N_24G || WirelessMode == WIRELESS_MODE_N_5G) {
#if 0
		//if(IOTAction == HT_IOT_VHT_HT_MIX_MODE)
		{
			if (ratr_bitmap & BIT20)	// Mix , 2SS
				Ret = 3;
			else 					// Mix, 1SS
				Ret = 2;
		}
#else
                     Ret =  0;
#endif

	} else if (WirelessMode == WIRELESS_MODE_AC_5G)
		Ret = 1;						// VHT

	return (Ret << 4);
}



BOOLEAN
Get_RA_ShortGI(
	struct rtl8192cd_priv *priv,
	struct stat_info *		pEntry,
	IN	WIRELESS_MODE		WirelessMode,
	IN	u1Byte				ChnlBW
)
{
	BOOLEAN						bShortGI;

	BOOLEAN	bShortGI20MHz = FALSE, bShortGI40MHz = FALSE, bShortGI80MHz = FALSE;

	if (	WirelessMode == WIRELESS_MODE_N_24G ||
			WirelessMode == WIRELESS_MODE_N_5G ||
			WirelessMode == WIRELESS_MODE_AC_5G  ) {
		if (pEntry->ht_cap_buf.ht_cap_info & cpu_to_le16(_HTCAP_SHORTGI_20M_) &&
				   priv->pmib->dot11nConfigEntry.dot11nShortGIfor20M) {
			bShortGI20MHz = TRUE;
		}
		if (pEntry->ht_cap_buf.ht_cap_info & cpu_to_le16(_HTCAP_SHORTGI_40M_)
				&& priv->pmib->dot11nConfigEntry.dot11nShortGIfor40M) {
			bShortGI40MHz = TRUE;
		}
     #ifdef RTK_AC_SUPPORT
		if (WirelessMode == WIRELESS_MODE_AC_5G && (GET_CHIP_VER(priv) != VERSION_8723B)) {
			if ( (cpu_to_le32(pEntry->vht_cap_buf.vht_cap_info) & BIT(SHORT_GI80M_E))
				&& priv->pmib->dot11nConfigEntry.dot11nShortGIfor80M)
			bShortGI80MHz = TRUE;
#if 0
// disable SGI when LDPC is enabled in AC mode
			if((pEntry->is_realtek_sta) && (priv->pmib->dot11nConfigEntry.dot11nLDPC == 1) && 
				((pEntry->ht_cap_len && cpu_to_le16(pEntry->ht_cap_buf.ht_cap_info) & _HTCAP_SUPPORT_RX_LDPC_) ||
					(pEntry->vht_cap_len && (cpu_to_le32(pEntry->vht_cap_buf.vht_cap_info) & BIT(RX_LDPC_E))))	) {
					bShortGI80MHz = bShortGI40MHz = bShortGI20MHz = 0;
				}
#endif			
		}
	#endif
	}

	switch (ChnlBW) {
		case HT_CHANNEL_WIDTH_20_40:
			bShortGI = bShortGI40MHz;
			break;
		case HT_CHANNEL_WIDTH_80:
			bShortGI = bShortGI80MHz;
			break;
		default:
		case HT_CHANNEL_WIDTH_20:
			bShortGI = bShortGI20MHz;
			break;
	}
	return bShortGI;
}


u4Byte
RateToBitmap_2SSVHT(
	pu1Byte			pVHTRate
)
{

	u1Byte	i, j , tmpRate;
	u4Byte	RateBitmap = 0;

	for (i = j = 0; i < 4; i += 2, j += 10) {
		tmpRate = (pVHTRate[0] >> i) & 3;

		switch (tmpRate) {
			case 2:
				RateBitmap = RateBitmap | (0x03ff << j);
				break;
			case 1:
				RateBitmap = RateBitmap | (0x01ff << j);
				break;

			case 0:
				RateBitmap = RateBitmap | (0x00ff << j);
				break;

			default:
				break;
		}
	}

	return RateBitmap;
}


u4Byte
Get_VHT_HT_Mix_Ratrbitmap(
	u4Byte					IOTAction,
	WIRELESS_MODE			WirelessMode,
	u4Byte					HT_ratr_bitmap,
	u4Byte					VHT_ratr_bitmap
)
{
	u4Byte	ratr_bitmap = 0;
	if (WirelessMode == WIRELESS_MODE_N_24G || WirelessMode == WIRELESS_MODE_N_5G) {
		/*
				if(IOTAction == HT_IOT_VHT_HT_MIX_MODE)
					ratr_bitmap = HT_ratr_bitmap | BIT28 | BIT29;
				else
					ratr_bitmap =  HT_ratr_bitmap;
		*/
	} else
		ratr_bitmap =  VHT_ratr_bitmap;

	return ratr_bitmap;
}


#ifdef AC2G_256QAM
char is_ac2g(struct rtl8192cd_priv * priv)
{

	unsigned char ac2g = 0;

	if((GET_CHIP_VER(priv)== VERSION_8812E) || (GET_CHIP_VER(priv)== VERSION_8881A))
	if(priv->pmib->dot11BssType.net_work_type & WIRELESS_11AC 
		&& ((priv->pmib->dot11BssType.net_work_type & WIRELESS_11A) == 0)
		&& (priv->pshare->rf_ft_var.ac2g_enable)
		)
		ac2g = 1;

	return ac2g; 

}
#endif

VOID
UpdateHalRAMask8812(
	struct rtl8192cd_priv *priv,
	struct stat_info 		*pEntry,
	u1Byte				rssi_level
)
{

	u1Byte						WirelessMode = WIRELESS_MODE_A;
	u1Byte						BW = HT_CHANNEL_WIDTH_20;
	//	u2Byte		RateSet=0, i;
	u1Byte		MimoPs = MIMO_PS_NOLIMIT, MimoPs_enable = FALSE, ratr_index = 8, H2CCommand[7] = { 0};
	u4Byte		ratr_bitmap = 0, IOTAction = 0;
	u1Byte		disable_cck_rate = FALSE;
	u4Byte		ratr_bitmap_msb = 0;
	BOOLEAN		bShortGI = FALSE, bCurTxBW80MHz = FALSE, bCurTxBW40MHz = FALSE;
	struct 		stat_info *pstat = pEntry;
	u1Byte 		rf_mimo_mode = get_rf_mimo_mode(priv);
	#if CFG_HAL_RTK_AC_SUPPORT
	unsigned int VHT_TxMap = priv->pmib->dot11acConfigEntry.dot11VHT_TxMap;
	#endif
	
	if (pEntry == NULL) {
		return;
	}
	{
		if (pEntry->MIMO_ps & _HT_MIMO_PS_STATIC_)
			MimoPs = MIMO_PS_STATIC;
		else if (pEntry->MIMO_ps & _HT_MIMO_PS_DYNAMIC_)
			MimoPs = MIMO_PS_DYNAMIC;

#if	1
		BW = pstat->tx_bw;
		if( BW > priv->pshare->CurrentChannelBW)
			BW = priv->pshare->CurrentChannelBW;
#endif
		add_RATid(priv, pEntry);
		rssi_level = pstat->rssi_level;
		ratr_bitmap =  0xfffffff;
		//
		//		if(pstat->vht_cap_len && ( priv->pshare->rf_ft_var.support_11ac)) {
	#ifdef RTK_AC_SUPPORT	
		if (pstat->vht_cap_len && (priv->pmib->dot11BssType.net_work_type & WIRELESS_11AC) && (!should_restrict_Nrate(priv, pstat)) && (GET_CHIP_VER(priv) != VERSION_8723B)) { //for 11ac logo 

			if( (IS_B_CUT_8812(priv)) 
				&& ((priv->pmib->dot11nConfigEntry.dot11nUse40M==0)||(BW == 0)) )
			{
				//printk("\n  !!! B CUT + AC STA + 20M, NO AC RATES !!!\n");
				//printk("AP=%dM, STA=%dM\n", (0x1<<(priv->pmib->dot11nConfigEntry.dot11nUse40M))*20 , (0x1<<(pstat->tx_bw))*20 );
				WirelessMode = WIRELESS_MODE_N_5G;
			}
			else{
				WirelessMode = WIRELESS_MODE_AC_5G;
			}
			
			if (((le32_to_cpu(pstat->vht_cap_buf.vht_support_mcs[0]) >> 2) & 3) == 3)
				rf_mimo_mode = MIMO_1T1R;
			if (pstat->nss == 1)
				rf_mimo_mode = MIMO_1T1R;
		} else
	#endif
		if ((priv->pmib->dot11BssType.net_work_type & WIRELESS_11N) && pstat->ht_cap_len && (!should_restrict_Nrate(priv, pstat))) {
			if ((priv->pmib->dot11BssType.net_work_type & WIRELESS_11A) && (priv->pmib->dot11RFEntry.dot11channel > 14))
				WirelessMode = WIRELESS_MODE_N_5G;
			else
				WirelessMode = WIRELESS_MODE_N_24G;
			if ((pstat->tx_ra_bitmap & 0xff00000) == 0)
				rf_mimo_mode = MIMO_1T1R;
		} else if (((priv->pmib->dot11BssType.net_work_type & WIRELESS_11G) && isErpSta(pstat)) && 
					(priv->pmib->dot11RFEntry.dot11channel <= 14)) {
			WirelessMode = WIRELESS_MODE_G;
		} else if ((priv->pmib->dot11BssType.net_work_type & WIRELESS_11A) &&
					(priv->pmib->dot11RFEntry.dot11channel > 14) &&
				   ((OPMODE & WIFI_AP_STATE) || (priv->pmib->dot11RFEntry.phyBandSelect & PHY_BAND_5G))) {
			WirelessMode = WIRELESS_MODE_A;
		} else if (priv->pmib->dot11BssType.net_work_type & WIRELESS_11B) {
			WirelessMode = WIRELESS_MODE_B;
		}

		pstat->WirelessMode = WirelessMode;

		if (WirelessMode == WIRELESS_MODE_AC_5G) {
			ratr_bitmap &= 0xfff;
		#ifdef RTK_AC_SUPPORT	
			if(GET_CHIP_VER(priv) != VERSION_8723B) {
				int tmp32 = (pstat->vht_cap_buf.vht_support_mcs[0]);
				// special 1SS & 2SS
				if (pstat->nss == 1)
					tmp32 |=   cpu_to_le32(0xfffc);
				else if ((pstat->nss == 2) && (le32_to_cpu(tmp32) & 0x0c) == 0x0c)
					tmp32 &= cpu_to_le32(0xfffffff3);
				//
				ratr_bitmap |= RateToBitmap_2SSVHT((pu1Byte)&tmp32) << 12;
			}
		#endif
			if (IS_TEST_CHIP(priv)) {					// Test Chip...	2SS MCS7
				if (rf_mimo_mode == MIMO_1T1R)
					ratr_bitmap &= 0x000fffff;
				else
					ratr_bitmap &= 0x3FCFFFFF;
			} else {									// MP Chip...	MCS0~9
				if (rf_mimo_mode == MIMO_1T1R)
					ratr_bitmap &= 0x003fffff;
			}

			if (BW == HT_CHANNEL_WIDTH_80)
				bCurTxBW80MHz = TRUE;
		}
//		if (priv->pshare->is_40m_bw && (pstat->tx_bw == HT_CHANNEL_WIDTH_20_40))
//			bCurTxBW40MHz = TRUE;

		if (priv->pshare->is_40m_bw && (BW == HT_CHANNEL_WIDTH_20_40)
#ifdef WIFI_11N_2040_COEXIST
				&& !((((GET_MIB(priv))->dot11OperationEntry.opmode) & WIFI_AP_STATE) 
				&& COEXIST_ENABLE
				&& (priv->bg_ap_timeout || orForce20_Switch20Map(priv)

				))
#endif
		)
		
		bCurTxBW40MHz = TRUE;
		
	}

	if(((GET_MIB(priv))->dot11OperationEntry.opmode) & WIFI_STATION_STATE) {
		if(((GET_MIB(priv))->dot11Bss.t_stamp[1] & 0x6) == 0) {
			bCurTxBW40MHz = bCurTxBW80MHz = FALSE;
		}
	}
	BW = Get_RA_BW(bCurTxBW80MHz, bCurTxBW40MHz);
	#if CFG_HAL_RTK_AC_SUPPORT
	if(BW == 0)
	{
		//remove MCS9 for BW=20m
		if (rf_mimo_mode == MIMO_1T1R)
			VHT_TxMap &= ~(BIT(9));
		else if (rf_mimo_mode == MIMO_2T2R)
			VHT_TxMap &= ~(BIT(9)|BIT(19));
	}
	#endif

#if 1

	bShortGI = Get_RA_ShortGI(priv, pEntry, WirelessMode, BW);
	pstat->tx_bw_fw = BW;

	if (MimoPs <= MIMO_PS_DYNAMIC)
		MimoPs_enable = TRUE;
		
	phydm_UpdateHalRAMask(ODMPTR, WirelessMode, rf_mimo_mode, BW, MimoPs_enable, disable_cck_rate, &ratr_bitmap_msb, &ratr_bitmap, rssi_level);
	pstat->ratr_idx = phydm_rate_id_mapping(ODMPTR, WirelessMode, rf_mimo_mode, BW);

#else
	// assign band mask and rate bitmap
	switch (WirelessMode) {
		case WIRELESS_MODE_B: {
			ratr_index = RATR_INX_WIRELESS_B;
			if (ratr_bitmap & 0x0000000c)		//11M or 5.5M enable
				ratr_bitmap &= 0x0000000d;
			else
				ratr_bitmap &= 0x0000000f;
		}
		break;

		case WIRELESS_MODE_G: {
			ratr_index = RATR_INX_WIRELESS_GB;

			if (rssi_level == 1)
				ratr_bitmap &= 0x00000f00;
			else if (rssi_level == 2)
				ratr_bitmap &= 0x00000ff0;
			else
				ratr_bitmap &= 0x00000ff5;
		}
		break;

		case WIRELESS_MODE_A: {
			ratr_index = RATR_INX_WIRELESS_G;
			ratr_bitmap &= 0x00000ff0;
		}
		break;

		case WIRELESS_MODE_N_24G:
		case WIRELESS_MODE_N_5G: {
			if (WirelessMode == WIRELESS_MODE_N_24G)
				ratr_index = RATR_INX_WIRELESS_NGB;
			else
				ratr_index = RATR_INX_WIRELESS_NG;

			//			if(MimoPs <= MIMO_PS_DYNAMIC)
			if (MimoPs < MIMO_PS_DYNAMIC) {
				if (rssi_level == 1)
					ratr_bitmap &= 0x000f0000;
				else if (rssi_level == 2)
					ratr_bitmap &= 0x000ff000;
				else
					ratr_bitmap &= 0x000ff005;
			} else {
				if (rf_mimo_mode == MIMO_1T1R) {
					if (bCurTxBW40MHz) {
						if (rssi_level == 1)
							ratr_bitmap &= 0x000f0000;
						else if (rssi_level == 2)
							ratr_bitmap &= 0x000ff000;
						else
							ratr_bitmap &= 0x000ff015;
					} else {
						if (rssi_level == 1)
							ratr_bitmap &= 0x000f0000;
						else if (rssi_level == 2)
							ratr_bitmap &= 0x000ff000;
						else
							ratr_bitmap &= 0x000ff005;
					}
				} else {
					if (bCurTxBW40MHz) {
						if (rssi_level == 1)
							ratr_bitmap &= 0x0fff0000;
						else if (rssi_level == 2)
							ratr_bitmap &= 0x0ffff000;
						else
							ratr_bitmap &= 0x0ffff015;
					} else {
						if (rssi_level == 1)
							ratr_bitmap &= 0x0fff0000;
						else if (rssi_level == 2)
							ratr_bitmap &= 0x0ffff000;
						else
							ratr_bitmap &= 0x0ffff005;
					}
				}
			}
		}
		break;
#ifdef RTK_AC_SUPPORT
		case WIRELESS_MODE_AC_5G: {
			ratr_index = RATR_INX_WIRELESS_AC_N;

			if(pstat->rssi && priv->pshare->rf_ft_var.rssi_ra)
			{
				if (pstat->rssi >= priv->pshare->rf_ft_var.rssi_thd1)
					rssi_level = 1;
				else if (pstat->rssi >= priv->pshare->rf_ft_var.rssi_thd2)
					rssi_level = 2;
				else
					rssi_level = 3;

				if(pstat->rssi_level_ac && (pstat->rssi_level_ac==rssi_level))
					return;

				if (rf_mimo_mode == MIMO_1T1R)
				{
					if(rssi_level == 1) 			// add by Gary for ac-series
						ratr_bitmap &= 0x003f8000;
					else if (rssi_level == 2)
						ratr_bitmap &= 0x003ff000;
					else
						ratr_bitmap &= 0x003ff010;
				}	
				else
				{
					if(rssi_level == 1) 			// add by Gary for ac-series
						ratr_bitmap &= 0xfe3f8000;		 // VHT 2SS MCS3~9
					else if (rssi_level == 2)
						ratr_bitmap &= 0xfffff000;		 // VHT 2SS MCS0~9
					else
						ratr_bitmap &= 0xfffff010;		 // All
				}	

				//panic_printk("[%s][%d] rssi_level=[%d->%d] ratr_bitmap=0x%x\n", __FUNCTION__, __LINE__, 
					//pstat->rssi_level_ac, rssi_level, ratr_bitmap);

				pstat->rssi_level_ac = rssi_level;

			}
			else
			{
				if (rf_mimo_mode == MIMO_1T1R)
					ratr_bitmap &= 0x003ff010;
				else
					ratr_bitmap &= 0xfffff010;
			}

			ratr_bitmap &= (VHT_TxMap << 12)|0xff0;

		}
		break;
#endif
		default:
			ratr_index = RATR_INX_WIRELESS_NGB;

			if (rf_mimo_mode == MIMO_1T1R)
				ratr_bitmap &= 0x000ff0ff;
			else
				ratr_bitmap &= 0x0ffff0ff;
			break;
	}

	bShortGI = Get_RA_ShortGI(priv, pEntry, WirelessMode, BW);



	pstat->ratr_idx = MRateIdxToARFRId8812(priv, ratr_index, rf_mimo_mode) ;
	pstat->tx_bw_fw = BW;

#endif


#ifdef AC2G_256QAM
	 
	 if(is_ac2g(priv) && pstat->vht_cap_len )
	 {
		 printk("AC2G STA Associated !!\n");
		 if (rf_mimo_mode == MIMO_1T1R)
		 {
			 //bShortGI = 1;
			 ratr_bitmap = 0x003ff015;

			 if(BW == 2)
			 	pstat->ratr_idx = 10;
			 else
			 	pstat->ratr_idx = 11;
 
			 if(BW == 0)
			 	VHT_TxMap = 0x1ff;
			 else
			 	VHT_TxMap = 0x3ff;
 
		 }
		 else if (rf_mimo_mode == MIMO_2T2R)
		 {
			// bShortGI = 0;
			 ratr_bitmap = 0xffcff015;

			 if(BW == 2)
			 	pstat->ratr_idx = 9;
			 else
			 	pstat->ratr_idx = 12;

			 if(BW == 0)
			 	VHT_TxMap = 0x7fdff;
			 else
			 	VHT_TxMap = 0xfffff;
		 }

		 ratr_bitmap &= ((VHT_TxMap << 12)|0xfff);
		 
		 pstat->WirelessMode = WIRELESS_MODE_AC_24G;
	 }
	 
#endif

	// for debug, set vht_txmap for RAMask
	#if CFG_HAL_RTK_AC_SUPPORT
	if (WirelessMode == WIRELESS_MODE_AC_5G)
		ratr_bitmap &= (VHT_TxMap << 12)|0xff0;
	#endif

	H2CCommand[0] = REMAP_AID(pstat);
	H2CCommand[1] =  (pstat->ratr_idx) | (bShortGI ? 0x80 : 0x00) ;
	H2CCommand[2] = BW | Get_VHT_ENI(IOTAction, WirelessMode, ratr_bitmap);


	H2CCommand[2] |= BIT6;			// DisableTXPowerTraining

	H2CCommand[3] = (u1Byte)(ratr_bitmap & 0x000000ff);
	H2CCommand[4] = (u1Byte)((ratr_bitmap & 0x0000ff00) >> 8);
	H2CCommand[5] = (u1Byte)((ratr_bitmap & 0x00ff0000) >> 16);
	H2CCommand[6] = (u1Byte)((ratr_bitmap & 0xff000000) >> 24);

	FillH2CCmd8812(priv, H2C_8812_RA_MASK, 7, H2CCommand);

	SetBcnCtrlReg_8812(priv, BIT3, 0);
	/*
		panic_printk("UpdateHalRAMask8812E(): bitmap = %x ratr_index = %1x, MacID:%x, ShortGI:%x, MimoPs=%d\n",
			ratr_bitmap, pstat->ratr_idx,  (pstat->aid), bShortGI, MimoPs);

		panic_printk("Cmd: %02x, %02x, %02x, %02x, %02x, %02x, %02x  \n",
			H2CCommand[0] ,H2CCommand[1], H2CCommand[2],
			H2CCommand[3] ,H2CCommand[4], H2CCommand[5], H2CCommand[6]		);
	*/
}
void
UpdateHalMSRRPT8812(
	struct rtl8192cd_priv *priv,
	struct stat_info 		*pstat,
	unsigned char		opmode
)
{
	u1Byte		H2CCommand[3] = { 0};

    update_remapAid(priv,pstat);
	H2CCommand[0] = opmode & 0x01;
	H2CCommand[1] = REMAP_AID(pstat) & 0xff;
	H2CCommand[2] = 0;
	FillH2CCmd8812(priv, H2C_8812_MSRRPT, 3, H2CCommand);

	//	panic_printk("UpdateHalMSRRPT8812 Cmd: %02x, %02x, %02x  \n",
	//		H2CCommand[0] ,H2CCommand[1], H2CCommand[2]);
}
#if 0
void check_txrate_by_reg_8812(struct rtl8192cd_priv *priv, struct stat_info *pstat)
{
	unsigned char initial_rate = 0x7f;
	unsigned char legacyRA = 0 ;
	unsigned int autoRate1 = 0;

//	if (!priv->pshare->rf_ft_var.update_rainfo)
//		return;

	RTL_W8(0x8d, 0x01);
	RTL_W8(0x8f, 0x40);

	if ( should_restrict_Nrate(priv, pstat) && is_fixedMCSTxRate(priv))
		legacyRA = 1;

	if (pstat->sta_in_firmware == 1) {


#ifdef WDS
		if (pstat->state & WIFI_WDS) {
			autoRate1 =	(priv->pmib->dot11WdsInfo.entry[pstat->wds_idx].txRate == 0) ? 1 : 0;
		} else
#endif
		{
			autoRate1 = priv->pmib->dot11StationConfigEntry.autoRate;
		}

		if (autoRate1 || legacyRA) {
			RTL_W8(0x8c, REMAP_AID(pstat) & 0x1f);
			initial_rate = RTL_R8(0x2f0);
			if ((initial_rate & 0x7f) == 0x7f)
				return;


			if ((initial_rate & 0x3f) < 12) {
				pstat->current_tx_rate = dot11_rate_table[initial_rate & 0x3f];
				pstat->ht_current_tx_info &= ~TX_USE_SHORT_GI;
			} else {
				if ((initial_rate & 0x3f) >= 44)
					pstat->current_tx_rate = VHT_RATE_ID + ((initial_rate & 0x3f) - 44);
				else
					pstat->current_tx_rate = HT_RATE_ID + ((initial_rate & 0x3f) - 12);

				if (initial_rate & BIT(7))
					pstat->ht_current_tx_info |= TX_USE_SHORT_GI;
				else
					pstat->ht_current_tx_info &= ~TX_USE_SHORT_GI;
			}

			priv->pshare->current_tx_rate    = pstat->current_tx_rate;
			priv->pshare->ht_current_tx_info = pstat->ht_current_tx_info;
		}
	}
}
#endif


void odm_TXPowerTrackingCallback_ThermalMeter_8812E(struct rtl8192cd_priv * priv)
{
	unsigned char			ThermalValue = 0, delta, channel, is_decrease, rf_mimo_mode;
	unsigned char			ThermalValue_AVG_count = 0;
	unsigned int			ThermalValue_AVG = 0;
	int 					ele_D;
	char					OFDM_index[2];
	unsigned int			i = 0, rf = 2;

	unsigned char			OFDM_min_index = 7; //OFDM BB Swing should be less than +2.5dB, which is required by Arthur

	rf_mimo_mode = get_rf_mimo_mode(priv);

#ifdef MP_TEST
	if ((OPMODE & WIFI_MP_STATE) || priv->pshare->rf_ft_var.mp_specific) {
		channel = priv->pshare->working_channel;
		if (priv->pshare->mp_txpwr_tracking == FALSE)
			return;
	} else
#endif
	{
		channel = (priv->pmib->dot11RFEntry.dot11channel);
	}

	if (priv->pshare->Power_tracking_on_8812 == 0) {
		priv->pshare->Power_tracking_on_8812 = 1;
		PHY_SetRFReg(priv, RF92CD_PATH_A, 0x42, (BIT(17) | BIT(16)), 0x03);
		return;
	} else {
		priv->pshare->Power_tracking_on_8812 = 0;
		ThermalValue = (unsigned char)PHY_QueryRFReg(priv, RF_PATH_A, 0x42, 0xfc00, 1);	//0x42: RF Reg[15:10] 88E
		//printk("\nReadback Thermal Meter = 0x%x pre thermal meter 0x%x EEPROMthermalmeter 0x%x\n", ThermalValue, priv->pshare->ThermalValue, priv->pmib->dot11RFEntry.ther);
	}

	switch (rf_mimo_mode) {
		case MIMO_1T1R:
			rf = 1;
			break;
		case MIMO_2T2R:
			rf = 2;
			break;
		default:
			panic_printk("%s:%d get_rf_mimo_mode error!\n", __FUNCTION__, __LINE__);
			break;
	}
	#ifdef CONFIG_RTL_8812_SUPPORT
	if(GET_CHIP_VER(priv) != VERSION_8723B) {
		//Query OFDM path A default setting 	Bit[31:21]
		ele_D = PHY_QueryBBReg(priv, 0xc1c, 0xffe00000);
		for (i = 0; i < OFDM_TABLE_SIZE_8812; i++) {
			if (ele_D == OFDMSwingTable_8812[i]) {
				OFDM_index[0] = (unsigned char)i;
				//printk("PathA 0xc1c[32:21] = 0x%x, OFDM_index=%d\n", ele_D, OFDM_index[0]);
				break;
			}
		}

		//Query OFDM path B default setting
		if (rf_mimo_mode == MIMO_2T2R) {
			ele_D = PHY_QueryBBReg(priv, 0xe1c, 0xffe00000);
			for (i = 0; i < OFDM_TABLE_SIZE_8812; i++) {
				if (ele_D == OFDMSwingTable_8812[i]) {
					OFDM_index[1] = (unsigned char)i;
					//printk("PathB 0xe1c[31:21] = 0x%x, OFDM_index=%d\n", ele_D, OFDM_index[1]);
					break;
				}
			}
		}
	}
	#endif
	/* Initialize */
	if (!priv->pshare->ThermalValue) {
		priv->pshare->ThermalValue = priv->pmib->dot11RFEntry.ther;

	}

	/* calculate average thermal meter */
	{
		priv->pshare->ThermalValue_AVG_8812[priv->pshare->ThermalValue_AVG_index_8812] = ThermalValue;
		priv->pshare->ThermalValue_AVG_index_8812++;
		if (priv->pshare->ThermalValue_AVG_index_8812 == AVG_THERMAL_NUM_8812)
			priv->pshare->ThermalValue_AVG_index_8812 = 0;

		for (i = 0; i < AVG_THERMAL_NUM_8812; i++) {
			if (priv->pshare->ThermalValue_AVG_8812[i]) {
				ThermalValue_AVG += priv->pshare->ThermalValue_AVG_8812[i];
				ThermalValue_AVG_count++;
			}
		}

		if (ThermalValue_AVG_count) {
			ThermalValue = (unsigned char)(ThermalValue_AVG / ThermalValue_AVG_count);
			//printk("AVG Thermal Meter = 0x%x \n", ThermalValue);
		}
	}

	if (ThermalValue != priv->pshare->ThermalValue) {
		//printk("\n******** START:%s() ********\n", __FUNCTION__);
		//printk("\nReadback Thermal Meter = 0x%x pre thermal meter 0x%x EEPROMthermalmeter 0x%x\n", ThermalValue, priv->pshare->ThermalValue, priv->pmib->dot11RFEntry.ther);

		delta = RTL_ABS(ThermalValue, priv->pmib->dot11RFEntry.ther);
		is_decrease = ((ThermalValue < priv->pmib->dot11RFEntry.ther) ? 1 : 0);
		if (1) { //AC2G_256QAM (priv->pmib->dot11RFEntry.phyBandSelect == PHY_BAND_5G) {
#ifdef _TRACKING_TABLE_FILE
			if (priv->pshare->rf_ft_var.pwr_track_file) {
				if (is_decrease) {
					for (i = 0; i < rf; i++) {
						OFDM_index[i] = priv->pshare->OFDM_index0[i] + get_tx_tracking_index(priv, channel, i, delta, is_decrease, 0);
						OFDM_index[i] = ((OFDM_index[i] > (OFDM_TABLE_SIZE_8812 - 1)) ? (OFDM_TABLE_SIZE_8812 - 1) : OFDM_index[i]);
						//printk(">>> decrese power ---> new OFDM_INDEX:%d (%d + %d)\n", OFDM_index[i], priv->pshare->OFDM_index0[i], get_tx_tracking_index(priv, channel, i, delta, is_decrease, 0));
					}
				} else {
					for (i = 0; i < rf; i++) {
						OFDM_index[i] = priv->pshare->OFDM_index0[i] - get_tx_tracking_index(priv, channel, i, delta, is_decrease, 0);
						OFDM_index[i] = ((OFDM_index[i] < OFDM_min_index) ?  OFDM_min_index : OFDM_index[i]);
						//printk(">>> increse power ---> new OFDM_INDEX:%d (%d - %d)\n", OFDM_index[i], priv->pshare->OFDM_index0[i], get_tx_tracking_index(priv, channel, i, delta, is_decrease, 0));
					}
				}
			}
#endif
		#ifdef CONFIG_RTL_8812_SUPPORT
			if(GET_CHIP_VER(priv) != VERSION_8723B) {
				PHY_SetBBReg(priv, 0xc1c, 0xffe00000, OFDMSwingTable_8812[(unsigned int)OFDM_index[0]]);
				if (rf_mimo_mode == MIMO_2T2R)
					PHY_SetBBReg(priv, 0xe1c, 0xffe00000, OFDMSwingTable_8812[(unsigned int)OFDM_index[1]]);
			}
		#endif

		}
/*
		printk("PathA >>>>> 0xc1c[31:21] = 0x%x, OFDM_index:%d\n", PHY_QueryBBReg(priv, 0xc1c, 0xffe00000), OFDM_index[0]);
		if (rf_mimo_mode == MIMO_2T2R)
			printk("PathB >>>>> 0xe1c[31:21] = 0x%x, OFDM_index:%d\n", PHY_QueryBBReg(priv, 0xe1c, 0xffe00000), OFDM_index[1]);
		printk("\n******** END:%s() ********\n", __FUNCTION__);
*/
		//update thermal meter value
		priv->pshare->ThermalValue = ThermalValue;
		for (i = 0 ; i < rf ; i++)
			priv->pshare->OFDM_index[i] = OFDM_index[i];
	}
}


void requestTxReport_8812(struct rtl8192cd_priv *priv)
{
	unsigned char h2cresult, counter = 20;
	struct stat_info *sta;
	unsigned char H2CCommand[2] = {0xff, 0xff};

	if ( priv->pshare->sta_query_idx == -1)
		return;

#ifdef TXRETRY_CNT
	priv->pshare->sta_query_retry_idx = priv->pshare->sta_query_idx;
#endif

	while (is_h2c_buf_occupy(priv)) {
		delay_ms(2);
		if (--counter == 0)
			break;
	}

	if (!counter)
		return;

	sta = findNextSTA(priv, &priv->pshare->sta_query_idx);
	if (sta)
		H2CCommand[0] = REMAP_AID(sta);
	else {
		priv->pshare->sta_query_idx = -1;
		return;
	}

	sta = findNextSTA(priv, &priv->pshare->sta_query_idx);
	if (sta)	{
		H2CCommand[1] = REMAP_AID(sta);
	} else {
		priv->pshare->sta_query_idx = -1;
	}

	//WDEBUG("\n");
	h2cresult = FillH2CCmd8812(priv, H2C_8812_TX_REPORT, 2 , H2CCommand);
	//WDEBUG("h2cresult=%d\n",h2cresult);


}

#ifdef TXRETRY_CNT
void requestTxRetry_8812(struct rtl8192cd_priv *priv)
{
	unsigned char h2cresult, counter = 20;
	struct stat_info *sta;
	unsigned char H2CCommand[3] = {0xff, 0xff, 0x02};

	if ( priv->pshare->sta_query_retry_idx == -1)
		return;

	while (is_h2c_buf_occupy(priv)) {
		delay_ms(2);
		if (--counter == 0)
			break;
	}

	if (!counter)
		return;

	sta = findNextSTA(priv, &priv->pshare->sta_query_retry_idx);
	if (sta)
		H2CCommand[0] = REMAP_AID(sta);
	else {
		priv->pshare->sta_query_retry_idx = -1;
		return;
	}

	sta = findNextSTA(priv, &priv->pshare->sta_query_retry_idx);
	if (sta)	{
		H2CCommand[1] = REMAP_AID(sta);
	} else {
		priv->pshare->sta_query_retry_idx = -1;
	}

	//WDEBUG("\n");
	priv->pshare->sta_query_retry_macid[0] = H2CCommand[0];
	priv->pshare->sta_query_retry_macid[1] = H2CCommand[1];
	h2cresult = FillH2CCmd8812(priv, H2C_8812_TX_REPORT, 3 , H2CCommand);
	//WDEBUG("h2cresult=%d\n",h2cresult);

}
#endif

/*C2H_isr_8812()
AP Req Txrpt
"Start Address:USB:0xFD20PCIe/USB: 0x01A0"
|ID|SEQ 	|CONTENT		|LEN|TRIGGER|
-----------------------------------------
|0 | 1 		|2~13			|14 |15		|
-----------------------------------------

|ID=0x04|SEQ|STA1 MACID(1B)|Tx_ok1(2B)|Tx_fail1(2B)|initial rate1(1B)|STA2 MACID(1B)|Tx_ok2(2B)|Tx_fail2(2B)|Initial rate2(1B)|Len=12(1B)|FF(1B)|
*/
#if 0
void C2H_isr_8812(struct rtl8192cd_priv *priv)
{
	struct tx_rpt rpt1;
	int j, C2H_ID;
	unsigned char MacID = 0xff;
	int idx = 0x1a2;
#ifndef SMP_SYNC
	unsigned long flags=0;
#endif

	//WDEBUG("\n");

	SAVE_INT_AND_CLI(flags);
	C2H_ID = RTL_R8(0x1a0);

	if ( (C2H_ID) == C2H_8812_TX_REPORT ) {

		for (j = 0; j < 2; j++) {

			MacID = RTL_R8(idx);
			if (MacID == 0xff)
				continue;

			rpt1.macid =  MacID & TXdesc_92E_MacIdMask;

			if (rpt1.macid) {

				rpt1.txok = (RTL_R8(idx + 2)<<8) | RTL_R8(idx + 1);
				rpt1.txfail = (RTL_R8(idx + 4)<<8) | RTL_R8(idx + 3);
/*
#ifdef _BIG_ENDIAN_
				rpt1.txok = le16_to_cpu(RTL_R16(idx + 1));
				rpt1.txfail = le16_to_cpu(RTL_R16(idx + 3));
#else
				rpt1.txok = be16_to_cpu(RTL_R16(idx + 1));
				rpt1.txfail = be16_to_cpu(RTL_R16(idx + 3));
#endif
*/
				rpt1.initil_tx_rate = RTL_R8(idx + 5);
				txrpt_handler(priv, &rpt1);	// add inital tx rate handle for 8812E
			}
			idx += 6;
		}
	}

// check sounding BW also...



	RTL_W8( 0x1af, 0);
	if ( (C2H_ID) == C2H_8812_TX_REPORT )
	requestTxReport_8812(priv);
	RESTORE_INT(flags);
}
#endif

VOID
C2HRaReportHandler_8812(
	struct rtl8192cd_priv *priv,
	pu1Byte			CmdBuf,
	u1Byte			CmdLen
)
{
	u1Byte 	Rate = CmdBuf[0] & 0x3F;
	u1Byte	MacId = CmdBuf[1];
	BOOLEAN	bLDPC = CmdBuf[2] & BIT0;
	BOOLEAN	bTxBF = (CmdBuf[2] & BIT1) >> 1;
#if (BEAMFORMING_SUPPORT == 1)		
	Beamforming_SetTxBFen(priv, MacId, bTxBF);
#endif	
}

VOID
C2HTxTxReportHandler_8812(
	struct rtl8192cd_priv *priv,
		pu1Byte			CmdBuf,
		u1Byte			CmdLen
)
{
	struct tx_rpt rpt1;
	int k=0, j=0;
	for(j=0; j<2; j++) {
		rpt1.macid= CmdBuf[k];
		rpt1.txok = CmdBuf[k+1] | ((short)CmdBuf[k+2]<<8);
		rpt1.txfail = CmdBuf[k+3] | ((short)CmdBuf[k+4]<<8);
		rpt1.initil_tx_rate = CmdBuf[k+5];
		if(rpt1.macid != 0xff)
			txrpt_handler(priv, &rpt1);
		k+=6;
	}
}

#if (BEAMFORMING_SUPPORT == 1) 
VOID C2HTxBeamformingHandler_8812(	struct rtl8192cd_priv *priv, pu1Byte CmdBuf, u1Byte	CmdLen);
#endif

VOID
_C2HContentParsing8812(
	struct rtl8192cd_priv *priv,
		u1Byte			c2hCmdId, 
		u1Byte			c2hCmdLen,
		pu1Byte 			tmpBuf
)
{

	switch(c2hCmdId)
	{
		case C2H_8812_TXBF:
#if (BEAMFORMING_SUPPORT == 1) 			
			C2HTxBeamformingHandler_8812(priv, tmpBuf, c2hCmdLen);
#endif
			break;
		case C2H_8812_TXPERORT:
#ifdef TXREPORT		
			C2HTxTxReportHandler_8812(priv, tmpBuf, c2hCmdLen);

	#ifdef TXRETRY_CNT
			requestTxRetry_8812(priv);	
	#else
			requestTxReport_8812(priv);			
	#endif			

#endif
			break;

#ifdef TXRETRY_CNT
		case C2H_8812_TXRETRY:
			C2HTxTxRetryHandler(priv, tmpBuf);
			requestTxReport_8812(priv);
			break;  
#endif

		default:
			if(!(phydm_c2H_content_parsing(ODMPTR, c2hCmdId, c2hCmdLen, tmpBuf))) {
				printk("[C2H], Unkown packet!! CmdId(%#X)!\n", c2hCmdId);
			}
			break;
	}
}

VOID
C2HPacketHandler_8812(
	struct rtl8192cd_priv *priv,
		pu1Byte			Buffer,
		u1Byte			Length
	)
{
	u1Byte	c2hCmdId=0, c2hCmdSeq=0, c2hCmdLen=0;
	pu1Byte tmpBuf=NULL;
	c2hCmdId = Buffer[0];
	c2hCmdSeq = Buffer[1];
/*
#ifdef CONFIG_WLAN_HAL
	if(c2hCmdId==C2H_88XX_EXTEND_IND)
	{
		c2hCmdLen = Length;
		tmpBuf = Buffer;
		C2HExtEventHandler88XX(NULL, c2hCmdId, c2hCmdLen, tmpBuf);
	}
	else
#endif
*/
	{
		c2hCmdLen = Length -2;
		tmpBuf = Buffer+2;
		
		_C2HContentParsing8812(priv, c2hCmdId, c2hCmdLen, tmpBuf);		
	}
}

#if (BEAMFORMING_SUPPORT == 1)
VOID
SetBeamformRfMode8812(
	struct rtl8192cd_priv *priv,
	PRT_BEAMFORMING_INFO 	pBeamformingInfo
	)
{
	u1Byte					i;
	u4Byte					bSelfBeamformer = FALSE;
	u4Byte					bSelfBeamformee = FALSE;
	RT_BEAMFORMING_ENTRY	BeamformEntry;
	BEAMFORMING_CAP		BeamformCap = BEAMFORMING_CAP_NONE;
	
	BeamformCap = Beamforming_GetBeamCap(priv, pBeamformingInfo);
	
	if(BeamformCap == pBeamformingInfo->BeamformCap)
		return;
	else 
		pBeamformingInfo->BeamformCap = BeamformCap;
	if(get_rf_mimo_mode(priv) == MIMO_1T1R)
		return;
	PHY_SetRFReg(priv, ODM_RF_PATH_A, RF_WeLut_Jaguar, 0x80000,0x1); // RF Mode table write enable
	PHY_SetRFReg(priv, ODM_RF_PATH_B, RF_WeLut_Jaguar, 0x80000,0x1); // RF Mode table write enable

	bSelfBeamformer = BeamformCap & BEAMFORMER_CAP;
	bSelfBeamformee = BeamformCap & BEAMFORMEE_CAP;
	
	if(bSelfBeamformer)
	{	
		PHY_SetRFReg(priv, ODM_RF_PATH_A, RF_ModeTableAddr, 0x78000,0x3); // Select RX mode
		PHY_SetRFReg(priv, ODM_RF_PATH_A, RF_ModeTableData0, 0xfffff,0x3F7FF); // Set Table data
		PHY_SetRFReg(priv, ODM_RF_PATH_A, RF_ModeTableData1, 0xfffff,0xE26BF); // Enable TXIQGEN in RX mode
		PHY_SetRFReg(priv, ODM_RF_PATH_B, RF_ModeTableAddr, 0x78000, 0x3); // Select RX mode
		PHY_SetRFReg(priv, ODM_RF_PATH_B, RF_ModeTableData0, 0xfffff,0x3F7FF); // Set Table data
		PHY_SetRFReg(priv, ODM_RF_PATH_B, RF_ModeTableData1, 0xfffff,0xE26BF); // Enable TXIQGEN in RX mode
	}
	else if (!priv->pmib->dot11RFEntry.tx2path)
	{
		PHY_SetRFReg(priv, ODM_RF_PATH_A, RF_ModeTableAddr, 0x78000, 0x3); // Select RX mode
		PHY_SetRFReg(priv, ODM_RF_PATH_A, RF_ModeTableData0, 0xfffff,0x3F7FF); // Set Table data
		PHY_SetRFReg(priv, ODM_RF_PATH_A, RF_ModeTableData1, 0xfffff,0xC26BF); // Disable TXIQGEN in RX mode
		PHY_SetRFReg(priv, ODM_RF_PATH_B, RF_ModeTableAddr, 0x78000, 0x3); // Select RX mode
		PHY_SetRFReg(priv, ODM_RF_PATH_B, RF_ModeTableData0, 0xfffff,0x3F7FF); // Set Table data
		PHY_SetRFReg(priv, ODM_RF_PATH_B, RF_ModeTableData1, 0xfffff,0xC26BF); // Disable TXIQGEN in RX mode
	}
	PHY_SetRFReg(priv, ODM_RF_PATH_A, RF_WeLut_Jaguar, 0x80000,0x0); // RF Mode table write disable
	PHY_SetRFReg(priv, ODM_RF_PATH_B, RF_WeLut_Jaguar, 0x80000,0x0); // RF Mode table write disable
	
	if(bSelfBeamformer) {
		PHY_SetBBReg(priv, rTxPath_Jaguar, bMaskByte1, 0x33);
#ifdef RF_MIMO_SWITCH		
		priv->pshare->rf_phy_bb_backup[1] &= ~ bMaskByte1;
		priv->pshare->rf_phy_bb_backup[1] |= 0x3300;
#endif		

	} else {
		PHY_SetBBReg(priv, rTxPath_Jaguar, bMaskByte1, 0x11);
#ifdef RF_MIMO_SWITCH		
		priv->pshare->rf_phy_bb_backup[1] &= ~ bMaskByte1;
		priv->pshare->rf_phy_bb_backup[1] |= 0x1100;		
#endif		
	}
}
VOID
SetBeamformEnter8812(
	struct rtl8192cd_priv *priv,
	u1Byte				BFerBFeeIdx
	)
{
	u1Byte					i = 0;
	u1Byte					BFerIdx = (BFerBFeeIdx & 0xF0)>>4;
	u1Byte					BFeeIdx = (BFerBFeeIdx & 0xF);
	u4Byte					CSI_Param;	
	PRT_BEAMFORMING_INFO 	pBeamformingInfo = &(priv->pshare->BeamformingInfo);
	RT_BEAMFORMING_ENTRY	BeamformeeEntry;
	RT_BEAMFORMER_ENTRY	BeamformerEntry;
	u2Byte					STAid = 0;

	SetBeamformRfMode8812(priv, pBeamformingInfo);

	if(get_rf_mimo_mode(priv) == MIMO_2T2R)
		RTL_W32(0x9B4, 0x01081008);

	if((pBeamformingInfo->BeamformCap & BEAMFORMEE_CAP) && (BFerIdx < BEAMFORMER_ENTRY_NUM))
	{
		BeamformerEntry = pBeamformingInfo->BeamformerEntry[BFerIdx];
		
		if (!IS_TEST_CHIP(priv))		{
			if (IS_C_CUT_8812(priv))
				RTL_W16( REG_SND_PTCL_CTRL_8812, 0x2CB); 		// Disable SIG-B CRC8 check
			else
				RTL_W16( REG_SND_PTCL_CTRL_8812, 0x0B); 
		}
		else
			RTL_W16( REG_SND_PTCL_CTRL_8812, 0x1B);	

		// MAC addresss/Partial AID of Beamformer
		if(BFerIdx == 0)
		{
			for(i = 0; i < 6 ; i++)
				RTL_W8( (REG_BFMER0_INFO_8812+i), BeamformerEntry.MacAddr[i]);
			
			//RTL_W16( REG_BFMER0_INFO_8812+6, BeamformerEntry.P_AID);
		}
		else
		{
			for(i = 0; i < 6 ; i++)
				RTL_W8( (REG_BFMER1_INFO_8812+i), BeamformerEntry.MacAddr[i]);

			//RTL_W16( REG_BFMER1_INFO_8812+6, BeamformerEntry.P_AID);
		}

		// CSI report parameters of Beamformer
		if((BeamformerEntry.BeamformEntryCap & BEAMFORMEE_CAP_VHT_SU) ||(BeamformerEntry.BeamformEntryCap & BEAMFORMER_CAP_VHT_SU) )
			CSI_Param = 0x01090109;
		else 
			CSI_Param = 0x03090309;

		RTL_W32( REG_CSI_RPT_PARAM_BW20_8812, CSI_Param);
		RTL_W32( REG_CSI_RPT_PARAM_BW40_8812, CSI_Param);
		RTL_W32( REG_CSI_RPT_PARAM_BW80_8812, CSI_Param);

		// Timeout value for MAC to leave NDP_RX_standby_state 60 us
		//	RTL_W8( REG_SND_PTCL_CTRL_8812+3, 0x3C);
		RTL_W8( REG_SND_PTCL_CTRL_8812+3, 0x50);				// // ndp_rx_standby_timer
	}

	if((pBeamformingInfo->BeamformCap & BEAMFORMER_CAP) && (BFeeIdx < BEAMFORMEE_ENTRY_NUM))
	{
		BeamformeeEntry = pBeamformingInfo->BeamformeeEntry[BFeeIdx];

		if(OPMODE & WIFI_ADHOC_STATE)
			STAid = BeamformeeEntry.AID;
		else 
			STAid = BeamformeeEntry.P_AID;

		// P_AID of Beamformee & enable NDPA transmission
		if(BFeeIdx == 0)
		{	
			RTL_W16( REG_TXBF_CTRL_8812, STAid);	
			RTL_W8( REG_TXBF_CTRL_8812+3, RTL_R8( REG_TXBF_CTRL_8812+3)|BIT6|BIT7|BIT4);
		}	
		else
		{
			RTL_W16( REG_TXBF_CTRL_8812+2, STAid |BIT14| BIT15|BIT12);
		}	

		// CSI report parameters of Beamformee
		if(BFeeIdx == 0)	
		{
			// Get BIT24 & BIT25
			u1Byte	tmp = RTL_R8( REG_BFMEE_SEL_8812+3) & 0x3;	
			RTL_W8( REG_BFMEE_SEL_8812+3, tmp | 0x60);
			RTL_W16( REG_BFMEE_SEL_8812, STAid | BIT9);
		}	
		else
		{
			// Set BIT25
			RTL_W16( REG_BFMEE_SEL_8812+2, STAid | 0xE200);
		}

	//	if(pHalData->bIsMPChip == FALSE) 
		if (IS_TEST_CHIP(priv))		
		{
			// VHT category value 
			RTL_W8( REG_SND_PTCL_CTRL_8812+1, ACT_CAT_VHT);
			// NDPA subtype
			RTL_W8( REG_SND_PTCL_CTRL_8812+2, Type_NDPA >> 4);
		}	

		Beamforming_Notify(priv);
	}
}


VOID
SetBeamformLeave8812(
	struct rtl8192cd_priv *priv,
	u1Byte				Idx
	)
{
    PRT_BEAMFORMING_INFO    pBeamformingInfo = &(priv->pshare->BeamformingInfo);
    RT_BEAMFORMING_ENTRY    BeamformeeEntry = pBeamformingInfo->BeamformeeEntry[Idx];
    RT_BEAMFORMER_ENTRY	    BeamformerEntry = pBeamformingInfo->BeamformerEntry[Idx];

	/*	Clear P_AID of Beamformee
	* 	Clear MAC addresss of Beamformer
	*	Clear Associated Bfmee Sel
	*/
    if (BeamformeeEntry.BeamformEntryCap == BEAMFORMING_CAP_NONE)
    {
	if(Idx == 0)
	{	
		ODM_RT_TRACE(ODMPTR, PHYDM_COMP_TXBF, ODM_DBG_LOUD, ("[Beamforming]@%s, BeamformeeEntryCap == NONE, Idx=%d\n", __FUNCTION__, Idx));
		RTL_W16( REG_TXBF_CTRL_8812, 0);	
            RTL_W16( REG_BFMEE_SEL_8812, 0);        
        }
        else
        {
			ODM_RT_TRACE(ODMPTR, PHYDM_COMP_TXBF, ODM_DBG_LOUD, ("[Beamforming]@%s, BeamformeeEntryCap == NONE, Idx=%d\n", __FUNCTION__, Idx));
            RTL_R16( (REG_TXBF_CTRL_8812+2) & 0xF000);
            RTL_W16( REG_BFMEE_SEL_8812+2, RTL_R16( REG_BFMEE_SEL_8812+2) & 0x60);        
        }
    }

    if (BeamformerEntry.BeamformEntryCap == BEAMFORMING_CAP_NONE) 
    {
        if(Idx == 0)
        {
			ODM_RT_TRACE(ODMPTR, PHYDM_COMP_TXBF, ODM_DBG_LOUD, ("[Beamforming]@%s, BeamformerEntryCap == NONE, Idx=%d\n", __FUNCTION__, Idx));
			RTL_W32( REG_BFMER0_INFO_8812, 0);
			RTL_W16( REG_BFMER0_INFO_8812+4, 0);
		}	
		else
		{
			ODM_RT_TRACE(ODMPTR, PHYDM_COMP_TXBF, ODM_DBG_LOUD, ("[Beamforming]@%s, BeamformerEntryCap == NONE, Idx=%d\n", __FUNCTION__, Idx));
			RTL_W32( REG_BFMER1_INFO_8812, 0);
			RTL_W16( REG_BFMER1_INFO_8812+4, 0);
		}
    }

    if(((pBeamformingInfo->BeamformerEntry[0]).BeamformEntryCap == BEAMFORMING_CAP_NONE)
        && ((pBeamformingInfo->BeamformerEntry[1]).BeamformEntryCap == BEAMFORMING_CAP_NONE)){
		ODM_RT_TRACE(ODMPTR, PHYDM_COMP_TXBF, ODM_DBG_LOUD, ("[Beamforming]@%s, All BeamformerEntryCap == NONE, STOP feedback CSI\n", __FUNCTION__, Idx));
        RTL_W8( REG_SND_PTCL_CTRL_8812, 0xC8);
	}	
    
}
VOID
SetBeamformStatus8812(
	struct rtl8192cd_priv *priv,
	 u1Byte				Idx
	)
{
	u2Byte					BeamCtrlVal;
	u4Byte					BeamCtrlReg;
	PRT_BEAMFORMING_INFO pBeamformingInfo = &(priv->pshare->BeamformingInfo);
	RT_BEAMFORMING_ENTRY	BeamformEntry = pBeamformingInfo->BeamformeeEntry[Idx];
	if(OPMODE & WIFI_ADHOC_STATE)
		BeamCtrlVal = BeamformEntry.MacId;
	else 
		BeamCtrlVal = BeamformEntry.P_AID;
	if(Idx == 0)
		BeamCtrlReg = REG_TXBF_CTRL_8812;
	else
	{
		BeamCtrlReg = REG_TXBF_CTRL_8812+2;
		BeamCtrlVal |= BIT12 | BIT14|BIT15;
	}
	if((BeamformEntry.BeamformEntryState == BEAMFORMING_ENTRY_STATE_PROGRESSED)&& (priv->pshare->rf_ft_var.applyVmatrix))
	{
		if(BeamformEntry.BW == HT_CHANNEL_WIDTH_20)
			BeamCtrlVal |= BIT9;
		else if(BeamformEntry.BW == HT_CHANNEL_WIDTH_20_40)
			BeamCtrlVal |= (BIT9 | BIT10);
		else if(BeamformEntry.BW == HT_CHANNEL_WIDTH_80)
			BeamCtrlVal |= (BIT9 | BIT10 | BIT11);
	} else	{
		BeamCtrlVal &= ~(BIT9|BIT10|BIT11);
	}
	RTL_W16(BeamCtrlReg, BeamCtrlVal);
}
//2REG_C2HEVT_CLEAR
#define		C2H_EVT_HOST_CLOSE			0x00	// Set by driver and notify FW that the driver has read the C2H command message
#define		C2H_EVT_FW_CLOSE			0xFF	// Set by FW indicating that FW had set the C2H command message and it's not yet read by driver.



VOID Beamforming_NDPARate_8812(
	struct rtl8192cd_priv *priv,
	BOOLEAN		Mode,
	u1Byte		BW,
	u1Byte		Rate)
{

	u2Byte	NDPARate = Rate;

	if(NDPARate == 0)
	{
// ???	
#if 0
		if(Mode == 1 || Mode == 4)
			NDPARate = 0x0c;		//MGN_MCS0;
		else
			NDPARate = 0x2c;		//MGN_VHT1SS_MCS0;
#else
		if(priv->pshare->rssi_min > 30) // link RSSI > 30%
			NDPARate = 8;				//MGN_24M
		else
			NDPARate = 4;				//MGN_6M

#endif
	}

	if(NDPARate < 0x0c)
		BW = HT_CHANNEL_WIDTH_20;	

	RTL_W8(REG_NDPA_OPT_CTRL_8812, (NDPARate<<2) |  (BW & 0x03));


}

VOID
C2HTxBeamformingHandler_8812(
	struct rtl8192cd_priv *priv,
		pu1Byte			CmdBuf,
		u1Byte			CmdLen
)
{
	u1Byte 	status = CmdBuf[0] & BIT0;
	Beamforming_End(priv, status);
}

VOID HW_VAR_HW_REG_TIMER_START_8812(struct rtl8192cd_priv *priv)
{
	RTL_W8(0x15F, 0x0);
	RTL_W8(0x15F, 0x5);

}

VOID HW_VAR_HW_REG_TIMER_INIT_8812(struct rtl8192cd_priv *priv, int t)
{
	RTL_W8(0x164, 1);
	RTL_W16(0x15C, t);

}

VOID HW_VAR_HW_REG_TIMER_STOP_8812(struct rtl8192cd_priv *priv)
{
	RTL_W8(0x15F, 0);
}


VOID _Beamforming_CLK(struct rtl8192cd_priv *priv)
{
	u2Byte	u2btmp;
	u1Byte	QueueID, Count = 0, u1btmp;
#ifndef SMP_SYNC
	unsigned long flags;
#endif
	unsigned int loop = 5;

	if(priv->pshare->bScanInProcess==TRUE)
		return;

	SAVE_INT_AND_CLI(flags);
	
	// Stop Usb TxDMA
	RTL_W16(PCIE_CTRL_REG, 0xff00 );

	// Wait TXFF empty
	for(Count = 0; Count < loop; Count++) {
		u2btmp = RTL_R16(REG_TXPKT_EMPTY_8812);
		u2btmp = u2btmp & 0x7ff;
		if(u2btmp != 0x7ff) {
			delay_ms(10);
			continue;
		} else {
			break;
		}
	}

	// TX pause
	RTL_W8(REG_TXPAUSE_8812, 0xFF);

	// Wait TX State Machine OK
	for(Count = 0; Count < 100; Count++) {
		if(RTL_R32(REG_SCH_TXCMD_8812) != 0)
			continue;
		else 
			break;
	}
	
	// Stop RX DMA path
	u1btmp = RTL_R8(REG_RXDMA_CONTROL_8812);
	RTL_W8(REG_RXDMA_CONTROL_8812, u1btmp| BIT2);

	for(Count = 0; Count < loop; Count++) {
		u1btmp = RTL_R8( REG_RXDMA_CONTROL_8812);
		if(u1btmp & BIT1)
			break;
		else
			delay_ms(10);
	}

	// Disable clock
	RTL_W8(REG_SYS_CLKR_8812+1, 0xf0);
	// Disable 320M
	RTL_W8(REG_AFE_PLL_CTRL_8812+3, 0x8);
	// Enable 320M
	RTL_W8(REG_AFE_PLL_CTRL_8812+3, 0xa);
	// Enable clock
	RTL_W8( REG_SYS_CLKR_8812+1, 0xfc);

	// Release Tx pause
	RTL_W8(REG_TXPAUSE_8812, 0);

	// Enable RX DMA path
	u1btmp = RTL_R8(REG_RXDMA_CONTROL_8812);
	RTL_W8(REG_RXDMA_CONTROL_8812, u1btmp & (~ BIT2));

	// Start Usb TxDMA
	RTL_W16(PCIE_CTRL_REG, 0x00 );
	RESTORE_INT(flags);
//	panic_printk("FBEAM_ERROR <==%s\n", __FUNCTION__);
}


#endif 


VOID RTL8812_MACID_PAUSE(
	struct rtl8192cd_priv *priv,
    BOOLEAN      bSleep,   
    u4Byte       aid
)
{
	if (priv->pshare->rf_ft_var.enable_macid_sleep) {
		if (bSleep) {
			if (aid > MACID_REGION3_LIMIT)
				RTL_W32(REG_MACID_PKT_SLEEP_3, RTL_R32(REG_MACID_PKT_SLEEP_3) | BIT(aid-MACID_REGION3_LIMIT-1));                
            else if(aid > MACID_REGION2_LIMIT)                
				RTL_W32(REG_MACID_PKT_SLEEP_2, RTL_R32(REG_MACID_PKT_SLEEP_2) | BIT(aid-MACID_REGION2_LIMIT-1));
			else if(aid > MACID_REGION1_LIMIT)
                RTL_W32(REG_MACID_PKT_SLEEP_1, RTL_R32(REG_MACID_PKT_SLEEP_1) | BIT(aid-MACID_REGION1_LIMIT-1));
            else                
				RTL_W32(REG_MACID_PKT_SLEEP_0, RTL_R32(REG_MACID_PKT_SLEEP_0) | BIT(aid));
		} else {
		    if (aid > MACID_REGION3_LIMIT)
				RTL_W32(REG_MACID_PKT_SLEEP_3, RTL_R32(REG_MACID_PKT_SLEEP_3) & ~BIT(aid-MACID_REGION3_LIMIT-1));                
            else if(aid > MACID_REGION2_LIMIT)                
				RTL_W32(REG_MACID_PKT_SLEEP_2, RTL_R32(REG_MACID_PKT_SLEEP_2) & ~BIT(aid-MACID_REGION2_LIMIT-1));
			else if(aid > MACID_REGION1_LIMIT)
                RTL_W32(REG_MACID_PKT_SLEEP_1, RTL_R32(REG_MACID_PKT_SLEEP_1) & ~BIT(aid-MACID_REGION1_LIMIT-1));
            else                
				RTL_W32(REG_MACID_PKT_SLEEP_0, RTL_R32(REG_MACID_PKT_SLEEP_0) & ~BIT(aid));	
		}
	}
}


#endif //CONFIG_RTL_8812_SUPPORT


#if defined(DFS) || defined(RTK_AC_SUPPORT) //for 11ac logo
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
#ifdef DFS
 	if (!priv->pmib->dot11DFSEntry.disable_DFS &&
		(OPMODE & WIFI_AP_STATE)) {
		if (timer_pending(&priv->DFS_timer))
			del_timer_sync(&priv->DFS_timer);

		if (timer_pending(&priv->ch_avail_chk_timer))
			del_timer_sync(&priv->ch_avail_chk_timer);

		if (timer_pending(&priv->dfs_det_chk_timer))
			del_timer_sync(&priv->dfs_det_chk_timer);
 	}
#endif

	priv->pshare->CurrentChannelBW = priv->pshare->is_40m_bw = priv->pmib->dot11nConfigEntry.dot11nUse40M;

	if (priv->pmib->dot11nConfigEntry.dot11nUse40M == HT_CHANNEL_WIDTH_80 && 
		priv->pmib->dot11RFEntry.band5GSelected == PHY_BAND_5G_3) 
	{
		int channel = priv->pmib->dot11RFEntry.dot11channel;

		if (!is80MChannel(priv->available_chnl, priv->available_chnl_num, channel)) {
			priv->pshare->CurrentChannelBW = priv->pshare->is_40m_bw = HT_CHANNEL_WIDTH_AC_20;
		}
	}
#ifdef DFS
	if(priv->pshare->rf_ft_var.dfs_next_ch != 0){
		priv->pmib->dot11RFEntry.dot11channel = priv->pshare->rf_ft_var.dfs_next_ch;
	}
#endif
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

#ifdef MBSSID
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
#endif

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

#if defined(CONFIG_RTL_8812_SUPPORT) ||defined(CONFIG_RTL_8723B_SUPPORT) 
		if (GET_CHIP_VER(priv) == VERSION_8812E || GET_CHIP_VER(priv) == VERSION_8723B) {
			UpdateHalRAMask8812(priv, pstat, 3);
		}
#endif
#ifdef  CONFIG_WLAN_HAL
		if (IS_HAL_CHIP(priv)) {
			GET_HAL_INTERFACE(priv)->UpdateHalRAMaskHandler(priv, pstat, 3);
		}
#endif

		pstat = findNextSTA(priv, &staidx);

	}	

#ifdef DFS
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
#endif
}
#endif

