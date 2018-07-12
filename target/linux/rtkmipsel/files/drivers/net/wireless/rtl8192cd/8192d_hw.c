
#define _8192D_HW_C_

#ifdef __KERNEL__
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <asm/uaccess.h>
#include <asm/unistd.h>
#endif

#include "./8192cd_cfg.h"
#include "./8192cd.h"
#include "./8192cd_hw.h"
#include "./8192cd_headers.h"
#include "./8192cd_debug.h"


#ifdef CONFIG_RTL_92D_SUPPORT

#if defined(CONFIG_RTL_819X) && defined(USE_RLX_BSP)
#if defined(CONFIG_OPENWRT_SDK) && !defined(CONFIG_ARCH_CPU_RLX)
#include <bspchip.h>
#else
#include <bsp/bspchip.h>
#endif //CONFIG_OPENWRT_SDK
#endif

#ifndef USE_OUT_SRC
#define IQK_ADDA_REG_NUM 16
#endif


#ifdef CONFIG_RTL_92D_DMDP

extern u32 if_priv[];

__inline__ unsigned char DMDP_RTL_R8(unsigned int phy, unsigned int reg)
{
	struct rtl8192cd_priv *priv;
	//printk("++++++++++++++++++++++++++%s(%x)++++++++++++++++++++++++++\n", __FUNCTION__, reg);
	if (phy >= NUM_WLAN_IFACE || phy < 0) {
		printk("%s: phy index[%d] out of bound !!\n", __FUNCTION__, phy);
		return -1;
	}
	priv = (struct rtl8192cd_priv *)if_priv[phy];
	return RTL_R8(reg);
}

__inline__ void DMDP_RTL_W8(unsigned int phy, unsigned int reg, unsigned char val8)
{
	struct rtl8192cd_priv *priv;
	//printk("++++++++++++++++++++++++++%s(%x,%x)++++++++++++++++++++++++++\n", __FUNCTION__, reg, val8);
	if (phy >= NUM_WLAN_IFACE || phy < 0) {
		printk("%s: phy index[%d] out of bound !!\n", __FUNCTION__, phy);
		return;
	}
	priv = (struct rtl8192cd_priv *)if_priv[phy];
	RTL_W8(reg, val8);
}

__inline__ unsigned short DMDP_RTL_R16(unsigned int phy, unsigned int reg)
{
	struct rtl8192cd_priv *priv;
	//printk("++++++++++++++++++++++++++%s++++++++++++++++++++++++++\n", __FUNCTION__);
	if (phy >= NUM_WLAN_IFACE || phy < 0) {
		printk("%s: phy index[%d] out of bound !!\n", __FUNCTION__, phy);
		return -1;
	}
	priv = (struct rtl8192cd_priv *)if_priv[phy];
	return RTL_R16(reg);
}

__inline__ void DMDP_RTL_W16(unsigned int phy, unsigned int reg, unsigned short val16)
{
	struct rtl8192cd_priv *priv;
	//printk("++++++++++++++++++++++++++%s++++++++++++++++++++++++++\n", __FUNCTION__);
	if (phy >= NUM_WLAN_IFACE || phy < 0) {
		printk("%s: phy index[%d] out of bound !!\n", __FUNCTION__, phy);
		return;
	}
	priv = (struct rtl8192cd_priv *)if_priv[phy];
	RTL_W16(reg, val16);
}

__inline__ unsigned int DMDP_RTL_R32(unsigned int phy, unsigned int reg)
{
	struct rtl8192cd_priv *priv;
	//printk("++++++++++++++++++++++++++%s(%x)++++++++++++++++++++++++++\n", __FUNCTION__, reg);
	if (phy >= NUM_WLAN_IFACE || phy < 0) {
		printk("%s: phy index[%d] out of bound !!\n", __FUNCTION__, phy);
		return -1;
	}
	priv = (struct rtl8192cd_priv *)if_priv[phy];
	return RTL_R32(reg);
}

__inline__ void DMDP_RTL_W32(unsigned int phy, unsigned int reg, unsigned int val32)
{
	struct rtl8192cd_priv *priv;
	//printk("++++++++++++++++++++++++++%s(%x, %x)++++++++++++++++++++++++++\n", __FUNCTION__, reg, val32);
	if (phy >= NUM_WLAN_IFACE || phy < 0) {
		printk("%s: phy index[%d] out of bound !!\n", __FUNCTION__, phy);
		return;
	}
	priv = (struct rtl8192cd_priv *)if_priv[phy];
	RTL_W32(reg, val32);
}

unsigned int DMDP_PHY_QueryBBReg(unsigned int phy, unsigned int RegAddr, unsigned int BitMask)
{
	//printk("++++++++++++++++++++++++++%s++++++++++++++++++++++++++\n", __FUNCTION__);
	if (phy >= NUM_WLAN_IFACE || phy < 0) {
		printk("%s: phy index[%d] out of bound !!\n", __FUNCTION__, phy);
		return -1;
	}
	return PHY_QueryBBReg((struct rtl8192cd_priv *)if_priv[phy], RegAddr, BitMask);
}

void DMDP_PHY_SetBBReg(unsigned int phy, unsigned int RegAddr, unsigned int BitMask, unsigned int Data)
{
	//printk("++++++++++++++++++++++++++%s++++++++++++++++++++++++++\n", __FUNCTION__);
	if (phy >= NUM_WLAN_IFACE || phy < 0) {
		printk("%s: phy index[%d] out of bound !!\n", __FUNCTION__, phy);
		return;
	}
	PHY_SetBBReg((struct rtl8192cd_priv *)if_priv[phy], RegAddr, BitMask, Data);
}

unsigned int DMDP_PHY_QueryRFReg(unsigned int phy, RF92CD_RADIO_PATH_E eRFPath,
				unsigned int RegAddr, unsigned int BitMask, unsigned int dbg_avoid)
{
	//printk("++++++++++++++++++++++++++%s++++++++++++++++++++++++++\n", __FUNCTION__);
	if (phy >= NUM_WLAN_IFACE || phy < 0) {
		printk("%s: phy index[%d] out of bound !!\n", __FUNCTION__, phy);
		return -1;
	}
	return PHY_QueryRFReg((struct rtl8192cd_priv *)if_priv[phy], eRFPath, RegAddr, BitMask, dbg_avoid);
}

void DMDP_PHY_SetRFReg(unsigned int phy, RF92CD_RADIO_PATH_E eRFPath, unsigned int RegAddr,
				unsigned int BitMask, unsigned int Data)
{
	//printk("++++++++++++++++++++++++++%s++++++++++++++++++++++++++\n", __FUNCTION__);
	if (phy >= NUM_WLAN_IFACE || phy < 0) {
		printk("%s: phy index[%d] out of bound !!\n", __FUNCTION__, phy);
		return;
	}
	PHY_SetRFReg((struct rtl8192cd_priv *)if_priv[phy], eRFPath, RegAddr, BitMask, Data);
}

#endif //CONFIG_RTL_92D_DMDP

void SetSYN_para(struct rtl8192cd_priv *priv, unsigned char channel)
{
	unsigned int eRFPath, tmp=0;
	unsigned int idx=-1, i;
	unsigned int SYN_PARA[8][8] = {
		{0xe43be, 0xfc638, 0x77c0a, 0xde471, 0xd7110, 0x8cb04, 0x00000, 0x00000},	// CH36-140 20MHz
		{0xe43be, 0xfc078, 0xf7c1a, 0xe0c71, 0xd7550, 0xacb04, 0x00000, 0x00000},	// CH36-140 40MHz
		{0xe43bf, 0xff038, 0xf7c0a, 0xde471, 0xe5550, 0xacb04, 0x00000, 0x00000},	// CH149, 155, 161
		{0xe43bf, 0xff079, 0xf7c1a, 0xde471, 0xe5550, 0xacb04, 0x00000, 0x00000},	// CH151, 153, 163, 165
		{0xe43bf, 0xff038, 0xf7c1a, 0xde471, 0xd7550, 0xacb04, 0x00000, 0x00000},	// CH157, 159
#ifdef SW_LCK_92D
		{0x643bc, 0xfc038, 0x77c1a, 0x00000, 0x00000, 0x00000, 0x61289, 0x01840},	// CH1,2,4,9,10,11,12
		{0x643bc, 0xfc038, 0x07c1a, 0x00000, 0x00000, 0x00000, 0x61289, 0x01840},	// CH3,13,14
		{0x243bc, 0xfc438, 0x07c1a, 0x00000, 0x00000, 0x00000, 0x6128b, 0x0fc41}	// CH5-8
#else
		{0x643bc, 0xfc038, 0x77c1a, 0x00000, 0x00000, 0x00000, 0x41289, 0x01840},	// CH1,2,4,9,10,11,12
		{0x643bc, 0xfc038, 0x07c1a, 0x00000, 0x00000, 0x00000, 0x41289, 0x01840},	// CH3,13,14
		{0x243bc, 0xfc438, 0x07c1a, 0x00000, 0x00000, 0x00000, 0x4128b, 0x0fc41}	// CH5-8
#endif
	};


	if (priv->pmib->dot11RFEntry.phyBandSelect==PHY_BAND_2G)
		eRFPath = RF92CD_PATH_B;
	else
		eRFPath = RF92CD_PATH_A;

	if (priv->pmib->dot11RFEntry.phyBandSelect==PHY_BAND_5G){
		if (channel >=36 && channel <=140){
			if (!priv->pshare->CurrentChannelBW)
				idx = 0;
			else
				idx = 1;
		} else if (channel == 149 || channel == 155 || channel == 161)
			idx = 2;
		else if (channel==151 || channel==153 || channel==163 || channel==165)
			idx = 3;
		else if (channel==157 || channel==159)
			idx = 4;
	} else {
		if (channel==1 || channel==2 || channel==4 || channel==9 || channel==10 || channel==11 || channel==12)
			idx = 5;
		else if (channel==3 || channel==13 || channel==14)
			idx = 6;
		else if (channel>=5 && channel<=8)
			idx = 7;
	}

	if (idx==-1){
		DEBUG_ERR("No suitable channel (%d) for setting synthersizer parameter!\n", channel);
		return;
	}

	for (i=0;i<8;i++){
#ifdef CONFIG_RTL_92D_DMDP
		if (i==0 && (idx>=0 && idx <=4) &&
			(priv->pmib->dot11RFEntry.macPhyMode == DUALMAC_DUALPHY))
			tmp = 0xe439d;
		else
#endif
			tmp = SYN_PARA[idx][i];

		if (tmp!=0) {

#ifdef CONFIG_RTL_92D_DMDP
			if (priv->pmib->dot11RFEntry.macPhyMode == DUALMAC_DUALPHY && eRFPath == RF92CD_PATH_B) {
				DMDP_PHY_SetRFReg(1, RF92CD_PATH_A, (0x25+i), bMask20Bits, tmp);
				//DEBUG_INFO("DMDP_PHY_SetRFReg(1, %d, 0x%x, bMask20Bits, 0x%x)\n", eRFPath, (0x25+i), tmp);
			} else
#endif
			{
				PHY_SetRFReg(priv, eRFPath, (0x25+i), bMask20Bits, tmp);
				//DEBUG_INFO("PHY_SetRFReg(priv, %d, 0x%x, bMask20Bits, 0x%x)\n", eRFPath, (0x25+i), tmp);
			}
			if (i==3)
				priv->pshare->RegRF28[eRFPath] = tmp;
		}
	}
}

unsigned int IMR_SET_N[3][11] = {
  {0x00ff0, 0x4400f, 0x00ff0, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x64888, 0xe266c, 0x00090}, //G-mode
  {0x22880, 0x4470f, 0x55880, 0x00070, 0x88000, 0x00000, 0x88080, 0x70000, 0x64a82, 0xe466c, 0x00090}, //36-64
  {0x44880, 0x4477f, 0x77880, 0x00070, 0x88000, 0x00000, 0x880b0, 0x00000, 0x64b82, 0xe466c, 0x00090}, // 100-165
};

void SetIMR_n(struct rtl8192cd_priv *priv, unsigned char channel)
{
	unsigned int eRFPath, curMaxRFPath;
	int imr_idx = -1;
	unsigned char temp_800;

#ifdef CONFIG_RTL_92D_DMDP
	if (priv->pmib->dot11RFEntry.macPhyMode == DUALMAC_DUALPHY)
		curMaxRFPath = RF92CD_PATH_B;
	else
#endif
		curMaxRFPath = RF92CD_PATH_MAX;

	if (priv->pmib->dot11RFEntry.phyBandSelect==PHY_BAND_2G)
		imr_idx = 0;
	else {
		if (channel>=36 && channel <=64)
			imr_idx = 1;
		else
			imr_idx = 2;
	}
	PHY_SetBBReg(priv, rFPGA0_AnalogParameter4, 0x00f00000, 0xf);
	temp_800 = PHY_QueryBBReg(priv, rFPGA0_RFMOD, 0x0f000000);
	PHY_SetBBReg(priv, rFPGA0_RFMOD, 0x0f000000, 0);

	for(eRFPath = RF92CD_PATH_A; eRFPath < curMaxRFPath; eRFPath++) {
		int i;

		PHY_SetRFReg(priv, eRFPath, 0x00, bMask20Bits, 0x70000);
		//DEBUG_INFO("IMR [0x00] %05x\n", PHY_QueryRFReg(priv, eRFPath, 0x00, bMask20Bits,1));
		//delay_us(5);

		for (i=0;i<11;i++) {
			PHY_SetRFReg(priv, eRFPath, (0x2f+i), bMask20Bits, IMR_SET_N[imr_idx][i]);
			//DEBUG_INFO("IMR [0x%x] %05x\n", (0x2f+i), PHY_QueryRFReg(priv, eRFPath, (0x2f+i), bMask20Bits,1));
			//delay_us(5);
		}

		if (priv->pmib->dot11RFEntry.phyBandSelect==PHY_BAND_2G)
			PHY_SetRFReg(priv, eRFPath, 0x00, bMask20Bits, 0x32fff);
		else
			PHY_SetRFReg(priv, eRFPath, 0x00, bMask20Bits, 0x32c9a);

		//DEBUG_INFO("IMR [0x00] %05x\n", PHY_QueryRFReg(priv, eRFPath, 0x00, bMask20Bits,1));
		//delay_us(5);
	}

	PHY_SetBBReg(priv, rFPGA0_RFMOD, 0x0f000000, temp_800);
	PHY_SetBBReg(priv, rFPGA0_AnalogParameter4, 0x00f00000, 0x0);
}


/*
 *  Follow WS-20101228-Willis-xxxx dynamic parameter-R00
 */
void Update92DRFbyChannel(struct rtl8192cd_priv *priv, unsigned char channel)
{
#ifdef RTL8192D_INT_PA
	u8	eRFPath = 0, curMaxRFPath;
	if (priv->pmib->dot11RFEntry.macPhyMode == DUALMAC_DUALPHY)
		curMaxRFPath = RF92CD_PATH_B;
	else
		curMaxRFPath = RF92CD_PATH_MAX;

	if (priv->pshare->rf_ft_var.use_intpa92d){
		for(eRFPath = RF92CD_PATH_A; eRFPath <curMaxRFPath; eRFPath++) {
			if (priv->pmib->dot11RFEntry.phyBandSelect==PHY_BAND_5G){
				if (channel>=36 && channel<=64){
					PHY_SetRFReg(priv, eRFPath, 0x0b, bMask20Bits, 0x01a00);
					PHY_SetRFReg(priv, eRFPath, 0x48, bMask20Bits, 0x40443);
					PHY_SetRFReg(priv, eRFPath, 0x49, bMask20Bits, 0x00eb5);
					//PHY_SetRFReg(priv, eRFPath, 0x4a, bMask20Bits, 0x50f0f);
					PHY_SetRFReg(priv, eRFPath, 0x4b, bMask20Bits, 0x89bec);
					//PHY_SetRFReg(priv, eRFPath, 0x4c, bMask20Bits, 0x0dded);
					PHY_SetRFReg(priv, eRFPath, 0x03, bMask20Bits, 0x94a12);
					delay_us(10);	
					PHY_SetRFReg(priv, eRFPath, 0x04, bMask20Bits, 0x94a12);
					PHY_SetRFReg(priv, eRFPath, 0x0e, bMask20Bits, 0x94a12);
				}else if (channel>=100 && channel<=140){
					PHY_SetRFReg(priv, eRFPath, 0x0b, bMask20Bits, 0x01800);
					PHY_SetRFReg(priv, eRFPath, 0x48, bMask20Bits, 0xc0443);
					PHY_SetRFReg(priv, eRFPath, 0x49, bMask20Bits, 0x00730);
					//PHY_SetRFReg(priv, eRFPath, 0x4a, bMask20Bits, 0x50f0f);
					PHY_SetRFReg(priv, eRFPath, 0x4b, bMask20Bits, 0x896ee);
					//PHY_SetRFReg(priv, eRFPath, 0x4c, bMask20Bits, 0x0dded);
					PHY_SetRFReg(priv, eRFPath, 0x03, bMask20Bits, 0x94a52);
					delay_us(10);	
					PHY_SetRFReg(priv, eRFPath, 0x04, bMask20Bits, 0x94a52);
					PHY_SetRFReg(priv, eRFPath, 0x0e, bMask20Bits, 0x94a52);
				}else if (channel>=149 && channel<=165){
					PHY_SetRFReg(priv, eRFPath, 0x0b, bMask20Bits, 0x01800);
					PHY_SetRFReg(priv, eRFPath, 0x48, bMask20Bits, 0xc0443);
					PHY_SetRFReg(priv, eRFPath, 0x49, bMask20Bits, 0x00730);
					//PHY_SetRFReg(priv, eRFPath, 0x4a, bMask20Bits, 0x50f0f);
					PHY_SetRFReg(priv, eRFPath, 0x4b, bMask20Bits, 0x896ee);
					//PHY_SetRFReg(priv, eRFPath, 0x4c, bMask20Bits, 0x0dded);
					PHY_SetRFReg(priv, eRFPath, 0x03, bMask20Bits, 0x94a12);
					delay_us(10);	
					PHY_SetRFReg(priv, eRFPath, 0x04, bMask20Bits, 0x94a12);
					PHY_SetRFReg(priv, eRFPath, 0x0e, bMask20Bits, 0x94a12);
				}
			}else{
					PHY_SetRFReg(priv, eRFPath, 0x0b, bMask20Bits, 0x1c000);
					PHY_SetRFReg(priv, eRFPath, 0x03, bMask20Bits, 0x18c63);
					delay_us(10);	
					PHY_SetRFReg(priv, eRFPath, 0x04, bMask20Bits, 0x18c63);
					PHY_SetRFReg(priv, eRFPath, 0x0e, bMask20Bits, 0x18c67);
			}
		}
	}
#endif

	if (priv->pmib->dot11RFEntry.phyBandSelect==PHY_BAND_5G){
		//update fc_area
		if (priv->pmib->dot11RFEntry.dot11channel<149)
			PHY_SetBBReg(priv, rOFDM1_CFOTracking, BIT(14) | BIT(13), 1);
		else
			PHY_SetBBReg(priv, rOFDM1_CFOTracking, BIT(14) | BIT(13), 2);
		// VCO_BF_LDO= 1.12V->1.27V for  40M spur issue
		PHY_SetRFReg(priv, RF92CD_PATH_A, 0x2A, BIT(13)|BIT(12), 2);
		// RX Ch36 40M spurs
		if (channel==36){
			priv->pshare->RegRF28[RF92CD_PATH_A] &= (~BIT(6));
			priv->pshare->RegRF28[RF92CD_PATH_A] |= BIT(5);
			PHY_SetRFReg(priv, RF92CD_PATH_A, 0x28, bMask20Bits, priv->pshare->RegRF28[RF92CD_PATH_A]);
			//PHY_SetRFReg(priv, RF92CD_PATH_A, 0x28, BIT(6)|BIT(5), 0);
		}
	} else {
		//update fc_area
		PHY_SetBBReg(priv, rOFDM1_CFOTracking, BIT(14) | BIT(13), 0);
	}

}

int Load_92D_Firmware(struct rtl8192cd_priv *priv)
{
	int fw_len, wait_cnt=0;
	unsigned int CurPtr=0;
	unsigned int WriteAddr;
	unsigned int Temp;
	unsigned char *ptmp;
#ifndef SMP_SYNC
	unsigned long flags = 0;
#endif

#ifdef CONFIG_RTL8672
	printk("val=%x\n",RTL_R8(0x80));
#endif

#ifdef MP_TEST
	if (priv->pshare->rf_ft_var.mp_specific)
		return TRUE;
#endif

	printk("===> %s\n", __FUNCTION__);

	SAVE_INT_AND_CLI(flags);

	printk("Firmware check %x(%x)\n", RTL_R32(MCUFWDL), (RTL_R8(MCUFWDL) & MCUFWDL_RDY));
	if (RTL_R8(MCUFWDL) & MCUFWDL_RDY){
		printk("<=== Firmware Downloaded\n");
		goto check_fwdl_rdy;
	}

	wait_cnt=0;
	while(RTL_R8(RF_CTRL) & FW_DL_INPROC){
		wait_cnt++;
		delay_ms(50);
	}

#ifdef CONFIG_RTL_92D_DMDP
	if (wait_cnt==0) {
		if (priv->pshare->wlandev_idx == 0)
			RTL_W8(RF_CTRL, RTL_R8(RF_CTRL)|FW_DL_INPROC);
		else {
			if (RTL_R8(RSV_MAC0_CTRL) & MAC0_EN)
				goto check_fwdl_rdy;
			else
				RTL_W8(RF_CTRL, RTL_R8(RF_CTRL)|FW_DL_INPROC);
		}
	} else {
		if (RTL_R8(MCUFWDL) & MCUFWDL_RDY){
			printk("<=== Firmware Downloaded\n");
			RESTORE_INT(flags);
			return TRUE;
		}else{
			RTL_W8(RF_CTRL, RTL_R8(RF_CTRL)|FW_DL_INPROC);
		}
	}
#else
	if (wait_cnt==0) {
		RTL_W8(RF_CTRL, RTL_R8(RF_CTRL)|FW_DL_INPROC);
	} else {
		if (RTL_R8(MCUFWDL) & MCUFWDL_RDY){
			printk("<=== Firmware Downloaded\n");
			RESTORE_INT(flags);
			return TRUE;
		}else{
			RTL_W8(RF_CTRL, RTL_R8(RF_CTRL)|FW_DL_INPROC);
		}
	}

#endif

	if ((priv->pshare->fw_signature & 0xfff0 ) == 0x92D0)
		ptmp = data_rtl8192dfw_n_start + RT_8192CD_FIRMWARE_HDR_SIZE;
	else
		ptmp = data_rtl8192dfw_n_start;

	fw_len = (int)(data_rtl8192dfw_n_end - ptmp);
	printk("[%s][rtl8192dfw_n]\n",__FUNCTION__);

	// Disable SIC
	RTL_W16(GPIO_MUXCFG, (RTL_R16(GPIO_MUXCFG) & 0xff) | HTP_EN);
	delay_ms(1);

	// Enable MCU
	RTL_W16(SYS_FUNC_EN, (RTL_R16(SYS_FUNC_EN) & 0x0ff) | FEN_MREGEN
		| FEN_HWPDN | FEN_DIO_RF | FEN_ELDR | FEN_DCORE |FEN_CPUEN | FEN_PCIED);
	delay_ms(1);

#ifdef CONFIG_RTL8672
	RTL_W8(APS_FSMCO, RTL_R8(APS_FSMCO) | PFM_ALDN);
	delay_ms(1);  //czyao
#endif

	// Load SRAM
	WriteAddr = 0x1000;
	RTL_W8(MCUFWDL, RTL_R8(MCUFWDL) | MCUFWDL_EN);
	delay_ms(1);

	RTL_W32(MCUFWDL, RTL_R32(MCUFWDL) & 0xfff0ffff);

	delay_ms(1);

	while (CurPtr < fw_len) {
		if ((CurPtr+4) > fw_len) {
			// Reach the end of file.
			while (CurPtr < fw_len) {
				Temp = *(ptmp + CurPtr);
				RTL_W8(WriteAddr, (unsigned char)Temp);
				WriteAddr++;
				CurPtr++;
			}
		} else {
			// Write FW content to memory.
			Temp = *((unsigned int *)(ptmp + CurPtr));
			Temp = cpu_to_le32(Temp);
			RTL_W32(WriteAddr, Temp);
			WriteAddr += 4;

			if(WriteAddr == 0x2000) {
				unsigned char tmp = RTL_R8(MCUFWDL+2);
				tmp += 1;
				WriteAddr = 0x1000;
				RTL_W8(MCUFWDL+2, tmp) ;
				delay_ms(10);
//				printk("\n[CurPtr=%x, 0x82=%x]\n", CurPtr, RTL_R8(0x82));
			}
			CurPtr += 4;
		}
	}

	watchdog_kick();

	RTL_W8(TCR+3, 0x7f);
	RTL_W8(MCUFWDL, (RTL_R8(MCUFWDL) & 0xfe) | MCUFWDL_RDY);
	delay_ms(1);
	//RTL_W8(RF_CTRL, RTL_R8(RF_CTRL) | BIT(6));
	RTL_W8(RF_CTRL, RTL_R8(RF_CTRL) & (~FW_DL_INPROC));
	delay_ms(1);

check_fwdl_rdy:

	printk("<=== %s\n", __FUNCTION__);
	// check if firmware is ready
	wait_cnt = 0;
#ifdef CONFIG_RTL_92D_DMDP
	if (priv->pshare->wlandev_idx == 0)
#endif
	{
		while (!(RTL_R8(RSV_MAC0_FWCTRL) & MAC0_WINTINI_RDY)) {
			if (++wait_cnt > 10) {
				RTL_W8(MCUFWDL, RTL_R8(MCUFWDL) & (~MCUFWDL_RDY));
				RESTORE_INT(flags);
				DEBUG_ERR("8192d mac0 firmware not ready\n");
				return FALSE;
			}
			delay_ms(2*wait_cnt);
		}
	}
#ifdef CONFIG_RTL_92D_DMDP
	else {
		while (!(RTL_R8(RSV_MAC1_FWCTRL) & MAC1_WINTINI_RDY)) {
			if (++wait_cnt > 10) {
				RTL_W8(MCUFWDL, RTL_R8(MCUFWDL) & (~MCUFWDL_RDY));
				RESTORE_INT(flags);
				DEBUG_ERR("8192d mac1 firmware not ready\n");
				return FALSE;
			}
			delay_ms(2*wait_cnt);

		}
	}
#endif
	RESTORE_INT(flags);
#ifdef CONFIG_RTL8672
	printk("val=%x\n",RTL_R8(MCUFWDL));
#endif
	return TRUE;
}


/*
 *	92DE Operation Mode
 */
void UpdateBBRFVal8192DE(struct rtl8192cd_priv *priv)
{
	u8	eRFPath = 0, curMaxRFPath;
	//u32	u4RegValue=0;

	//Update BB
	if (priv->pmib->dot11RFEntry.phyBandSelect & PHY_BAND_5G) {
		/*
		 *	5G
		 */
		//r_select_5G for path_A/B
		if (priv->pmib->dot11RFEntry.macPhyMode != DUALMAC_DUALPHY)
			PHY_SetBBReg(priv, rFPGA0_XAB_RFParameter, BIT(16), 0x1);
		PHY_SetBBReg(priv, rFPGA0_XAB_RFParameter, BIT(0), 0x1);
		//rssi_table_select:index 0 for 2.4G.1~3 for 5G
		PHY_SetBBReg(priv, rOFDM0_AGCRSSITable, BIT(7) | BIT(6), 0x01);
		//5G PA power on
		if (priv->pmib->dot11RFEntry.macPhyMode != DUALMAC_DUALPHY)
			PHY_SetBBReg(priv, rFPGA0_XAB_RFParameter, BIT(31), (priv->pmib->dot11RFEntry.trsw_pape_CC & BIT(6))>>6);
		PHY_SetBBReg(priv, rFPGA0_XAB_RFParameter, BIT(15), (priv->pmib->dot11RFEntry.trsw_pape_CC & BIT(4))>>4);

		//TRSW.TRSWB and PAPE2G mode table
		if (priv->pmib->dot11RFEntry.macPhyMode != DUALMAC_DUALPHY){
			// TRSW_2, TRSWB_2
			PHY_SetBBReg(priv, 0x870, BIT(22)|BIT(21), 0);
			// PAPE2G_2
			PHY_SetBBReg(priv, 0x870, BIT(26), (priv->pmib->dot11RFEntry.trsw_pape_CC & BIT(7))>>7);
			PHY_SetBBReg(priv, 0x864, BIT(10), (priv->pmib->dot11RFEntry.trsw_pape_CC & BIT(6))>>6);
		}
		// TRSW_1, TRSWB_1
		PHY_SetBBReg(priv, 0x870, BIT(6)|BIT(5), 0);
		// PAPE2G_1
		PHY_SetBBReg(priv, 0x870, BIT(10), (priv->pmib->dot11RFEntry.trsw_pape_CC & BIT(5))>>5);
		PHY_SetBBReg(priv, 0x860, BIT(10), (priv->pmib->dot11RFEntry.trsw_pape_CC & BIT(4))>>4);
#ifdef RTL8192D_INT_PA
		if (!priv->pshare->rf_ft_var.use_intpa92d)
#endif
		{
			//5G PA power on
			if (priv->pmib->dot11RFEntry.macPhyMode != DUALMAC_DUALPHY)
				PHY_SetBBReg(priv, rFPGA0_XAB_RFParameter, BIT(31), 0x1);
			PHY_SetBBReg(priv, rFPGA0_XAB_RFParameter, BIT(15), 0x1);
		}
		// 5G LNA on
		PHY_SetBBReg(priv, 0xb30, 0x00f00000, 0x0);
		//fc_area
		if (priv->pmib->dot11RFEntry.dot11channel<149)
			PHY_SetBBReg(priv, rOFDM1_CFOTracking, BIT(14) | BIT(13), 1);
		else
			PHY_SetBBReg(priv, rOFDM1_CFOTracking, BIT(14) | BIT(13), 2);
		//cck_disable
		PHY_SetBBReg(priv, rFPGA0_RFMOD, bCCKEn, 0x0);
		//TX BB gain shift
#ifdef RTL8192D_INT_PA
		if (priv->pshare->rf_ft_var.use_intpa92d){
			PHY_SetBBReg(priv, rOFDM0_XATxIQImbalance, bMaskDWord, 0x2d4000b5);
			if (priv->pmib->dot11RFEntry.macPhyMode != DUALMAC_DUALPHY)
				PHY_SetBBReg(priv, rOFDM0_XBTxIQImbalance, bMaskDWord, 0x2d4000b5);
		} else
#endif
		{
			PHY_SetBBReg(priv, rOFDM0_XATxIQImbalance, bMaskDWord, 0x20000080);
			if (priv->pmib->dot11RFEntry.macPhyMode != DUALMAC_DUALPHY)
				PHY_SetBBReg(priv, rOFDM0_XBTxIQImbalance, bMaskDWord, 0x20000080);
		}
		// Reset IQC
		PHY_SetBBReg(priv, 0xc94, 0xF0000000, 0);
		if (priv->pmib->dot11RFEntry.macPhyMode != DUALMAC_DUALPHY)
			PHY_SetBBReg(priv, 0xc9c, 0xF0000000, 0);
		//BB/DP IQC
		PHY_SetBBReg(priv, 0xb00, bMaskDWord, 0x010170b8);
		if (priv->pmib->dot11RFEntry.macPhyMode != DUALMAC_DUALPHY)
			PHY_SetBBReg(priv, 0xb70, bMaskDWord, 0x010170b8);

 	} else {
		/*
		 *	2.4G
		 */
		// r_select_5G for path_A/B
		if (priv->pmib->dot11RFEntry.macPhyMode != DUALMAC_DUALPHY)
			PHY_SetBBReg(priv, rFPGA0_XAB_RFParameter, BIT(16), 0x0);
		PHY_SetBBReg(priv, rFPGA0_XAB_RFParameter, BIT(0), 0);
		//rssi_table_select:index 0 for 2.4G.1~3 for 5G
		PHY_SetBBReg(priv, rOFDM0_AGCRSSITable, BIT(7) | BIT(6), 0x00);
		//5G PA power on
		if (priv->pmib->dot11RFEntry.macPhyMode != DUALMAC_DUALPHY)
			PHY_SetBBReg(priv, rFPGA0_XAB_RFParameter, BIT(31), 0x0);
		PHY_SetBBReg(priv, rFPGA0_XAB_RFParameter, BIT(15), 0x0);

		//TRSW.TRSWB and PAPE2G mode table
		if (priv->pmib->dot11RFEntry.macPhyMode != DUALMAC_DUALPHY){
			// TRSW_2
			PHY_SetBBReg(priv, 0x870, BIT(21), (priv->pmib->dot11RFEntry.trsw_pape_C9 & BIT(7))>>7);
			PHY_SetBBReg(priv, 0x864, BIT(5), (priv->pmib->dot11RFEntry.trsw_pape_C9 & BIT(6))>>6);
			// TRSWB_2
			PHY_SetBBReg(priv, 0x870, BIT(22), (priv->pmib->dot11RFEntry.trsw_pape_C9 & BIT(5))>>5);
			PHY_SetBBReg(priv, 0x864, BIT(6), (priv->pmib->dot11RFEntry.trsw_pape_C9 & BIT(4))>>4);
			// PAPE2G_2
			PHY_SetBBReg(priv, 0x870, BIT(26), (priv->pmib->dot11RFEntry.trsw_pape_CC & BIT(3))>>3);
			PHY_SetBBReg(priv, 0x864, BIT(10), (priv->pmib->dot11RFEntry.trsw_pape_CC & BIT(2))>>2);
		}
		// TRSW_1
		PHY_SetBBReg(priv, 0x870, BIT(5), (priv->pmib->dot11RFEntry.trsw_pape_C9 & BIT(3))>>3);
		PHY_SetBBReg(priv, 0x860, BIT(5), (priv->pmib->dot11RFEntry.trsw_pape_C9 & BIT(2))>>2);
		// TRSWB_1
		PHY_SetBBReg(priv, 0x870, BIT(6), (priv->pmib->dot11RFEntry.trsw_pape_C9 & BIT(1))>>1);
		PHY_SetBBReg(priv, 0x860, BIT(6), (priv->pmib->dot11RFEntry.trsw_pape_C9 & BIT(0))>>0);
		// PAPE2G_1
		PHY_SetBBReg(priv, 0x870, BIT(10), (priv->pmib->dot11RFEntry.trsw_pape_CC & BIT(1))>>1);
		PHY_SetBBReg(priv, 0x860, BIT(10), (priv->pmib->dot11RFEntry.trsw_pape_CC & BIT(0))>>0);

		// 5G LNA on
		PHY_SetBBReg(priv, 0xb30, 0x00f00000, 0xa);
		//fc_area
		PHY_SetBBReg(priv, rOFDM1_CFOTracking, BIT(14) | BIT(13), 0x00);
		//cck_enable
		PHY_SetBBReg(priv, rFPGA0_RFMOD, bCCKEn, 0x1);
		//TX BB gain shift
		PHY_SetBBReg(priv, rOFDM0_XATxIQImbalance, bMaskDWord, 0x40000100);
		if (priv->pmib->dot11RFEntry.macPhyMode != DUALMAC_DUALPHY)
			PHY_SetBBReg(priv, rOFDM0_XBTxIQImbalance, bMaskDWord, 0x40000100);
		// Reset IQC
		PHY_SetBBReg(priv, 0xc94, 0xF0000000, 0);
		if (priv->pmib->dot11RFEntry.macPhyMode != DUALMAC_DUALPHY)
			PHY_SetBBReg(priv, 0xc9c, 0xF0000000, 0);
		//BB/DP IQC
		PHY_SetBBReg(priv, 0xb00, bMaskDWord, 0x01017038);
		if (priv->pmib->dot11RFEntry.macPhyMode != DUALMAC_DUALPHY)
			PHY_SetBBReg(priv, 0xb70, bMaskDWord, 0x01017038);
	}

	//Update RF
	if (priv->pmib->dot11RFEntry.macPhyMode == DUALMAC_DUALPHY)
		curMaxRFPath = RF92CD_PATH_B;
	else
		curMaxRFPath = RF92CD_PATH_MAX;

	for(eRFPath = RF92CD_PATH_A; eRFPath <curMaxRFPath; eRFPath++) {
		if(priv->pmib->dot11RFEntry.phyBandSelect & PHY_BAND_5G) {
			/*
			 *	5G
			 */
			priv->pshare->RegRF18[eRFPath] &= 0xffffff00;
			priv->pshare->RegRF18[eRFPath] |= (BIT(16)|BIT(8)|0x24); //set channel 36
			PHY_SetRFReg(priv,eRFPath, rRfChannel,bMask20Bits, priv->pshare->RegRF18[eRFPath]);
			delay_ms(1);
			// LDO_DIV
			priv->pshare->RegRF28[eRFPath] = RTL_SET_MASK(priv->pshare->RegRF28[eRFPath],BIT(7)|BIT(6),1,6);
			//PHY_SetRFReg(priv,eRFPath, 0x28, BIT(7)|BIT(6), 0x01);
			PHY_SetRFReg(priv,eRFPath, 0x28, bMask20Bits, priv->pshare->RegRF28[eRFPath]);

			delay_ms(30);
		} else {
			/*
			 *	2.4G
			 */
			priv->pshare->RegRF18[eRFPath] &=  ~(BIT(16)|BIT(8)|0xFF);
			priv->pshare->RegRF18[eRFPath] |= 1; //set channel 1.
			PHY_SetRFReg(priv,eRFPath, rRfChannel,bMask20Bits, priv->pshare->RegRF18[eRFPath]);
			delay_ms(1);
			// LDO_DIV
			priv->pshare->RegRF28[eRFPath] &= (~(BIT(7)|BIT(6)));
			//PHY_SetRFReg(priv,eRFPath, 0x28, BIT(7)|BIT(6), 0x00);
			PHY_SetRFReg(priv,eRFPath, 0x28, bMask20Bits, priv->pshare->RegRF28[eRFPath]);

			delay_ms(30);
		}
	}

#ifdef CONFIG_RTL_92D_DMDP
	if (priv->pmib->dot11RFEntry.macPhyMode == DUALMAC_DUALPHY) {
		//Use antenna 0 & 1
		PHY_SetBBReg(priv, rOFDM0_TRxPathEnable, bMaskByte0, 0x11);
		PHY_SetBBReg(priv, rOFDM1_TRxPathEnable, bDWord, 0x1);

		//disable ad/da clock1
		if (!(DMDP_RTL_R8(0,SYS_FUNC_EN)&(FEN_BB_GLB_RST|FEN_BBRSTB))){
			DMDP_RTL_W8(0, SYS_FUNC_EN, (DMDP_RTL_R8(0,SYS_FUNC_EN)|FEN_BB_GLB_RST|FEN_BBRSTB));
		}
		DMDP_PHY_SetBBReg(0, rFPGA0_AdDaClockEn, BIT(13)|BIT(12), 3);
	} else
#endif
	{
		//Use antenna 0 & 1
		PHY_SetBBReg(priv, rOFDM0_TRxPathEnable, bMaskByte0, 0x33);
		PHY_SetBBReg(priv, rOFDM1_TRxPathEnable, bDWord, 0x3);

		//disable ad/da clock1
		PHY_SetBBReg(priv, rFPGA0_AdDaClockEn, BIT(13) | BIT(12), 0);
	}
}


#if 0 //def CLIENT_MODE
void clnt_save_IQK_res(struct rtl8192cd_priv *priv)
{
	priv->site_survey->bk_iqc[0] = PHY_QueryBBReg(priv,0xc80, bMaskDWord);
	priv->site_survey->bk_iqc[1] = PHY_QueryBBReg(priv,0xc94, bMaskByte3);
	priv->site_survey->bk_iqc[2] = PHY_QueryBBReg(priv,0xc4c, bMaskByte3);
	priv->site_survey->bk_iqc[3] = PHY_QueryBBReg(priv,0xc88, bMaskDWord);
	priv->site_survey->bk_iqc[4] = PHY_QueryBBReg(priv,0xc9c, bMaskByte3);
	priv->site_survey->bk_iqc[5] = PHY_QueryBBReg(priv,0xc14, bMaskDWord);
	priv->site_survey->bk_iqc[6] = PHY_QueryBBReg(priv,0xca0, bMaskByte3);
	priv->site_survey->bk_iqc[7] = PHY_QueryBBReg(priv,0xc1c, bMaskDWord);
	priv->site_survey->bk_iqc[8] = PHY_QueryBBReg(priv,0xc78, bMaskByte1);
	priv->site_survey->bk_iqc[9] = PHY_QueryRFReg(priv, RF92CD_PATH_A, 0x08, bMask20Bits, 1);
	priv->site_survey->bk_iqc[10] = PHY_QueryRFReg(priv, RF92CD_PATH_B, 0x08, bMask20Bits, 1);
}


void clnt_load_IQK_res(struct rtl8192cd_priv *priv)
{
	PHY_SetBBReg(priv,0xc80, bMaskDWord, priv->site_survey->bk_iqc[0]);
	PHY_SetBBReg(priv,0xc94, bMaskByte3, priv->site_survey->bk_iqc[1]);
	PHY_SetBBReg(priv,0xc4c, bMaskByte3, priv->site_survey->bk_iqc[2]);
	PHY_SetBBReg(priv,0xc88, bMaskDWord, priv->site_survey->bk_iqc[3]);
	PHY_SetBBReg(priv,0xc9c, bMaskByte3, priv->site_survey->bk_iqc[4]);
	PHY_SetBBReg(priv,0xc14, bMaskDWord, priv->site_survey->bk_iqc[5]);
	PHY_SetBBReg(priv,0xca0, bMaskByte3, priv->site_survey->bk_iqc[6]);
	PHY_SetBBReg(priv,0xc1c, bMaskDWord, priv->site_survey->bk_iqc[7]);
	PHY_SetBBReg(priv,0xc78, bMaskByte1, priv->site_survey->bk_iqc[8]);
	PHY_SetRFReg(priv,RF92CD_PATH_A, 0x08, bMask20Bits, priv->site_survey->bk_iqc[9]);
	PHY_SetRFReg(priv,RF92CD_PATH_B, 0x08, bMask20Bits, priv->site_survey->bk_iqc[10]);
}

#endif

#ifdef CONFIG_RTL_92D_DMDP
#if 0 //def CLIENT_MODE

void clnt_92D_2T_AGSwitch(struct rtl8192cd_priv * priv, int target)
{
	unsigned int flags, i;
	int rtStatus = 0;
	unsigned char temp_0522, temp_0550, temp_0551, temp_0800;
	unsigned char reg;

	SAVE_INT_AND_CLI(flags);

	/*
	 * Save MAC default value
	 */
	temp_0522 = RTL_R8(0x522);
	temp_0550 = RTL_R8(0x550);
	temp_0551 = RTL_R8(0x551);

	/*
	 *	MAC register setting
	 */
	RTL_W8(0x522, 0x3f);
	RTL_W8(0x550, temp_0550& (~BIT(3)));
	RTL_W8(0x551, temp_0551& (~BIT(3)));

	// stop BB
	PHY_SetBBReg(priv, rFPGA0_AnalogParameter4, 0x00f00000, 0xf);
	temp_0800 = PHY_QueryBBReg(priv, rFPGA0_RFMOD, 0x0f000000);
	PHY_SetBBReg(priv, rFPGA0_RFMOD, 0x0f000000, 0);

	// 5G_PAPE Select & external PA power on
	PHY_SetBBReg(priv, 0x878, BIT(0), 0);
	PHY_SetBBReg(priv, 0x878, BIT(16), 0);
	PHY_SetBBReg(priv, 0x878, BIT(15), 0);
	PHY_SetBBReg(priv, 0x878, BIT(31), 0);
	// RSSI Table Select
	PHY_SetBBReg(priv, 0xc78, BIT(7)|BIT(6), 0);
	// fc_area
	PHY_SetBBReg(priv, 0xd2c, BIT(14)|BIT(13), 0);
	// cck_enable
	PHY_SetBBReg(priv, rFPGA0_RFMOD, bCCKEn, 0x1);
	// LDO_DIV
	PHY_SetRFReg(priv, RF92CD_PATH_A, 0x28, BIT(7)|BIT(6), 0);
	PHY_SetRFReg(priv, RF92CD_PATH_B, 0x28, BIT(7)|BIT(6), 0);
	// MOD_AG // Set channel number
	PHY_SetRFReg(priv, RF92CD_PATH_A, 0x18, BIT(16), 0);
	PHY_SetRFReg(priv, RF92CD_PATH_A, 0x18, BIT(8), 0);
	PHY_SetRFReg(priv, RF92CD_PATH_B, 0x18, BIT(16), 0);
	PHY_SetRFReg(priv, RF92CD_PATH_B, 0x18, BIT(8), 0);
 	// CLOAD for path_A
	PHY_SetRFReg(priv, RF92CD_PATH_A, 0xB, BIT(16)|BIT(15)|BIT(14), 0x7);
	PHY_SetRFReg(priv, RF92CD_PATH_B, 0xB, BIT(16)|BIT(15)|BIT(14), 0x7);

	// IMR
	PHY_SetRFReg(priv, RF92CD_PATH_A, 0x00, bMask20Bits, 0x70000);
	for (i=0;i<11;i++) {
		PHY_SetRFReg(priv, RF92CD_PATH_A, (0x2f+i), bMask20Bits, IMR_SET_N[0][i]);
	}
	PHY_SetRFReg(priv, RF92CD_PATH_A, 0x00, bMask20Bits, 0x32fff);

	// Enable BB
	PHY_SetBBReg(priv, rFPGA0_RFMOD, 0x0f000000, temp_0800);
	// IQK
	PHY_SetBBReg(priv, 0xc80, bMaskDWord, 0x40000100);
	PHY_SetBBReg(priv, 0xc94, bMaskByte3, 0);
	PHY_SetBBReg(priv, 0xc4c, bMaskByte3, 0);
	PHY_SetBBReg(priv, 0xc88, bMaskDWord, 0x40000100);
	PHY_SetBBReg(priv, 0xc9c, bMaskByte3, 0);
	PHY_SetBBReg(priv, 0xc14, bMaskDWord, 0x40000100);
	PHY_SetBBReg(priv, 0xca0, bMaskByte3, 0);
	PHY_SetBBReg(priv, 0xc1c, bMaskDWord, 0x40000100);
	PHY_SetBBReg(priv, 0xc78, bMaskByte1, 0);
	PHY_SetRFReg(priv, RF92CD_PATH_A, 0x08, bMask20Bits, 0x84000);
	PHY_SetRFReg(priv, RF92CD_PATH_B, 0x08, bMask20Bits, 0x84000);

	//Set related registers for BW config

	PHY_SetBBReg(priv, rFPGA0_AnalogParameter4, 0x00f00000, 0x0);

	/*
	 *	Reload MAC default value
	 */
	RTL_W8(0x550, temp_0550);
	RTL_W8(0x551, temp_0551);
	RTL_W8(0x522, temp_0522);

	RESTORE_INT(flags);
}

#endif

#endif //CONFIG_RTL_92D_DMDP

#endif // CONFIG_RTL_92D_SUPPORT



