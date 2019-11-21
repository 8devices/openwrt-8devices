/*
 *  Routines to access hardware
 *
 *  $Id: 8192cd_hw.c,v 1.107.2.43 2011/01/17 13:21:01 victoryman Exp $
 *
 *  Copyright (c) 2009 Realtek Semiconductor Corp.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 */

#define _8192CD_HW_C_
#define _printk printk

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <asm/uaccess.h>
#include <linux/fcntl.h>
#include <linux/fs.h>
#include <linux/file.h>
#include <asm/unistd.h>
#include <linux/gpio.h>

#include "./8192cd_cfg.h"
#include "./8192cd.h"
#include "./8192cd_hw.h"
#include "./8192cd_headers.h"
#include "./8192cd_debug.h"



#include "./efuse.h"

#include <linux/syscalls.h>


#include <bspchip.h>


#define MAX_CONFIG_FILE_SIZE (20*1024)

int rtl8192cd_fileopen(const char *filename, int flags, int mode);
void selectMinPowerIdex(struct rtl8192cd_priv *priv);
void PHY_RF6052SetOFDMTxPower(struct rtl8192cd_priv *priv, unsigned int channel);
void PHY_RF6052SetCCKTxPower(struct rtl8192cd_priv *priv, unsigned int channel);



#define VAR_MAPPING(dst,src) \
unsigned char *data_##dst##_start = &data_##src[0]; \
unsigned char *data_##dst##_end   = &data_##src[sizeof(data_##src)]; \






/*-----------------------------------------------------------------------------
 * Function:	PHYCheckIsLegalRfPath8192cPci()
 *
 * Overview:	Check different RF type to execute legal judgement. If RF Path is illegal
 *			We will return false.
 *
 * Input:		NONE
 *
 * Output:		NONE
 *
 * Return:		NONE
 *
 * Revised History:
 *	When		Who		Remark
 *	11/15/2007	MHC		Create Version 0.
 *
 *---------------------------------------------------------------------------*/
int PHYCheckIsLegalRfPath8192cPci(struct rtl8192cd_priv *priv, unsigned int eRFPath)
{
	unsigned int rtValue = TRUE;

	if (get_rf_mimo_mode(priv) == MIMO_2T2R) {
		if ((eRFPath == RF92CD_PATH_A) || (eRFPath == RF92CD_PATH_B))
			rtValue = TRUE;
		else
			rtValue = FALSE;
	} else if (get_rf_mimo_mode(priv) == MIMO_1T1R) {
		if (eRFPath == RF92CD_PATH_A)
			rtValue = TRUE;
		else
			rtValue = FALSE;
	} else {
		rtValue = FALSE;
	}

	return rtValue;
}
#if defined(CONFIG_RTL_8196CS)
void setBaseAddressRegister(void)
{
	int tmp32 = 0, status;
	while (++tmp32 < 100) {
		REG32(0xb8b00004) = 0x00100007;
		REG32(0xb8b10004) = 0x00100007;
		REG32(0xb8b10010) = 0x18c00001;
		REG32(0xb8b10018) = 0x19000004;
		status = (REG32(0xb8b10010) ^ 0x18c00001) | ( REG32(0xb8b10018) ^ 0x19000004);
		if (!status)
			break;
		else {
			printk("set BAR fail,%x\n", status);
			printk("%x %x %x %x \n",
				   REG32(0xb8b00004) , REG32(0xb8b10004) , REG32(0xb8b10010),  REG32(0xb8b10018) );
		}
	} ;
}
#endif
/**
* Function:	phy_CalculateBitShift
*
* OverView:	Get shifted position of the BitMask
*
* Input:
*			u4Byte		BitMask,
*
* Output:	none
* Return:		u4Byte		Return the shift bit bit position of the mask
*/
unsigned int phy_CalculateBitShift(unsigned int BitMask)
{
	unsigned int i;

	for (i = 0; i <= 31; i++) {
		if (((BitMask >> i) & 0x1) == 1)
			break;
	}

	return (i);
}

/**
* Function:	PHY_QueryBBReg
*
* OverView:	Read "sepcific bits" from BB register
*
* Input:
*			PADAPTER		Adapter,
*			u4Byte			RegAddr,		//The target address to be readback
*			u4Byte			BitMask		//The target bit position in the target address
*										//to be readback
* Output:	None
* Return:		u4Byte			Data			//The readback register value
* Note:		This function is equal to "GetRegSetting" in PHY programming guide
*/
unsigned int PHY_QueryBBReg(struct rtl8192cd_priv *priv, unsigned int RegAddr, unsigned int BitMask)
{
	unsigned int ReturnValue = 0, OriginalValue, BitShift;

#ifdef DISABLE_BB_RF
	return 0;
#endif //DISABLE_BB_RF


	OriginalValue = RTL_R32(RegAddr);
	BitShift = phy_CalculateBitShift(BitMask);
	ReturnValue = (OriginalValue & BitMask) >> BitShift;

	return (ReturnValue);
}


/**
* Function:	PHY_SetBBReg
*
* OverView:	Write "Specific bits" to BB register (page 8~)
*
* Input:
*			PADAPTER		Adapter,
*			u4Byte			RegAddr,		//The target address to be modified
*			u4Byte			BitMask		//The target bit position in the target address
*										//to be modified
*			u4Byte			Data			//The new register value in the target bit position
*										//of the target address
*
* Output:	None
* Return:		None
* Note:		This function is equal to "PutRegSetting" in PHY programming guide
*/
void PHY_SetBBReg(struct rtl8192cd_priv *priv, unsigned int RegAddr, unsigned int BitMask, unsigned int Data)
{
	unsigned int OriginalValue, BitShift, NewValue;

#ifdef DISABLE_BB_RF
	return;
#endif //DISABLE_BB_RF


	if (BitMask != bMaskDWord) {
		//if not "double word" write
		//_TXPWR_REDEFINE ?? if have original value, how to count tx power index from PG file ??
		OriginalValue = RTL_R32(RegAddr);
		BitShift = phy_CalculateBitShift(BitMask);
		NewValue = ((OriginalValue & (~BitMask)) | ((Data << BitShift) & BitMask));
		RTL_W32(RegAddr, NewValue);
	} else
		RTL_W32(RegAddr, Data);

	return;
}



/**
* Function:	phy_RFSerialWrite
*
* OverView:	Write data to RF register (page 8~)
*
* Input:
*			PADAPTER		Adapter,
*			RF92CD_RADIO_PATH_E	eRFPath,	//Radio path of A/B/C/D
*			u4Byte			Offset,		//The target address to be read
*			u4Byte			Data			//The new register Data in the target bit position
*										//of the target to be read
*
* Output:	None
* Return:		None
* Note:		Threre are three types of serial operations: (1) Software serial write
*			(2) Hardware LSSI-Low Speed Serial Interface (3) Hardware HSSI-High speed
*			serial write. Driver need to implement (1) and (2).
*			This function is equal to the combination of RF_ReadReg() and  RFLSSIRead()
 *
 * Note: 		  For RF8256 only
 *			 The total count of RTL8256(Zebra4) register is around 36 bit it only employs
 *			 4-bit RF address. RTL8256 uses "register mode control bit" (Reg00[12], Reg00[10])
 *			 to access register address bigger than 0xf. See "Appendix-4 in PHY Configuration
 *			 programming guide" for more details.
 *			 Thus, we define a sub-finction for RTL8526 register address conversion
 *		       ===========================================================
 *			 Register Mode		RegCTL[1]		RegCTL[0]		Note
 *								(Reg00[12])		(Reg00[10])
 *		       ===========================================================
 *			 Reg_Mode0				0				x			Reg 0 ~15(0x0 ~ 0xf)
 *		       ------------------------------------------------------------------
 *			 Reg_Mode1				1				0			Reg 16 ~30(0x1 ~ 0xf)
 *		       ------------------------------------------------------------------
 *			 Reg_Mode2				1				1			Reg 31 ~ 45(0x1 ~ 0xf)
 *		       ------------------------------------------------------------------
*/
void phy_RFSerialWrite(struct rtl8192cd_priv *priv, RF92CD_RADIO_PATH_E eRFPath, unsigned int Offset, unsigned int Data)
{
	struct rtl8192cd_hw			*phw = GET_HW(priv);
	unsigned int				DataAndAddr = 0;
	BB_REGISTER_DEFINITION_T	*pPhyReg = &phw->PHYRegDef[eRFPath];
	unsigned int				NewOffset;

		Offset &= 0x7f;
	//
	// Switch page for 8256 RF IC
	//
	NewOffset = Offset;

	//
	// Put write addr in [5:0]  and write data in [31:16]
	//
	//DataAndAddr = (Data<<16) | (NewOffset&0x3f);
	DataAndAddr = ((NewOffset << 20) | (Data & 0x000fffff)) & 0x0fffffff;	// T65 RF

	//
	// Write Operation
	//
	PHY_SetBBReg(priv, pPhyReg->rf3wireOffset, bMaskDWord, DataAndAddr);
}


/**
* Function:	phy_RFSerialRead
*
* OverView:	Read regster from RF chips
*
* Input:
*			PADAPTER		Adapter,
*			RF92CD_RADIO_PATH_E	eRFPath,	//Radio path of A/B/C/D
*			u4Byte			Offset,		//The target address to be read
*			u4Byte			dbg_avoid,	//set bitmask in reg 0 to prevent RF switchs to debug mode
*
* Output:	None
* Return:		u4Byte			reback value
* Note:		Threre are three types of serial operations: (1) Software serial write
*			(2) Hardware LSSI-Low Speed Serial Interface (3) Hardware HSSI-High speed
*			serial write. Driver need to implement (1) and (2).
*			This function is equal to the combination of RF_ReadReg() and  RFLSSIRead()
*/
unsigned int phy_RFSerialRead(struct rtl8192cd_priv *priv, RF92CD_RADIO_PATH_E eRFPath, unsigned int Offset, unsigned int dbg_avoid)
{
	struct rtl8192cd_hw			*phw = GET_HW(priv);
	unsigned int 				tmplong, tmplong2;
	unsigned int				retValue = 0;
	BB_REGISTER_DEFINITION_T	*pPhyReg = &phw->PHYRegDef[eRFPath];
	unsigned int				NewOffset;

	//
	// Make sure RF register offset is correct
	//
		Offset &= 0x7f;

	//
	// Switch page for 8256 RF IC
	//
	NewOffset = Offset;

#if 0 // 92E use PHY_QueryRFReg & PHY_WriteRFReg callback function
//#ifdef CONFIG_WLAN_HAL_8192EE
	if (GET_CHIP_VER(priv) == VERSION_8192E) {
		int RfPiEnable = 0;
		if (eRFPath == RF_PATH_A) {
			tmplong2 = PHY_QueryBBReg(priv, rFPGA0_XA_HSSIParameter2, bMaskDWord);;
			tmplong2 = (tmplong2 & (~bLSSIReadAddress)) | (NewOffset << 23) | bLSSIReadEdge;	//T65 RF
			PHY_SetBBReg(priv, rFPGA0_XA_HSSIParameter2, bMaskDWord, tmplong2 & (~bLSSIReadEdge));
		} else {
			tmplong2 = PHY_QueryBBReg(priv, rFPGA0_XB_HSSIParameter2, bMaskDWord);
			tmplong2 = (tmplong2 & (~bLSSIReadAddress)) | (NewOffset << 23) | bLSSIReadEdge;	//T65 RF
			PHY_SetBBReg(priv, rFPGA0_XB_HSSIParameter2, bMaskDWord, tmplong2 & (~bLSSIReadEdge));
		}

		tmplong2 = PHY_QueryBBReg(priv, rFPGA0_XA_HSSIParameter2, bMaskDWord);
		PHY_SetBBReg(priv, rFPGA0_XA_HSSIParameter2, bMaskDWord, tmplong2 & (~bLSSIReadEdge));
		PHY_SetBBReg(priv, rFPGA0_XA_HSSIParameter2, bMaskDWord, tmplong2 | bLSSIReadEdge);

		delay_us(20);
	} else
#endif
	{

		// For 92S LSSI Read RFLSSIRead
		// For RF A/B write 0x824/82c(does not work in the future)
		// We must use 0x824 for RF A and B to execute read trigger
		if( pPhyReg->rfHSSIPara2==0 )
		{
			printk(KERN_ERR"Warnning in %s %d : rfHSSIPara2 don't be initialized yet\n",__FUNCTION__,__LINE__);
			return 0;
		}
		tmplong = RTL_R32(rFPGA0_XA_HSSIParameter2);
		tmplong2 = RTL_R32(pPhyReg->rfHSSIPara2);
		tmplong2 = (tmplong2 & (~bLSSIReadAddress)) | ((NewOffset << 23) | bLSSIReadEdge);	//T65 RF

		RTL_W32(rFPGA0_XA_HSSIParameter2, tmplong & (~bLSSIReadEdge));
		delay_us(20);
		RTL_W32(pPhyReg->rfHSSIPara2, tmplong2);
		delay_us(20);

	}
	//Read from BBreg8a0, 12 bits for 8190, 20 bits for T65 RF
	if (((eRFPath == RF92CD_PATH_A) && (RTL_R32(0x820)&BIT(8)))
			|| ((eRFPath == RF92CD_PATH_B) && (RTL_R32(0x828)&BIT(8))))
		retValue = PHY_QueryBBReg(priv, pPhyReg->rfLSSIReadBackPi, bLSSIReadBackData);
	else
		retValue = PHY_QueryBBReg(priv, pPhyReg->rfLSSIReadBack, bLSSIReadBackData);

	return retValue;
}

static void set_rf_path_num(struct rtl8192cd_priv *priv){
	if(GET_CHIP_VER(priv) == VERSION_8192C || GET_CHIP_VER(priv) == VERSION_8192D ||
		GET_CHIP_VER(priv) == VERSION_8192E || GET_CHIP_VER(priv) == VERSION_8197F ||
		GET_CHIP_VER(priv) == VERSION_8812E|| GET_CHIP_VER(priv) == VERSION_8822B){
		priv->pshare->phw->tx_path_start = RF92CD_PATH_A;
		priv->pshare->phw->tx_path_end = RF92CD_PATH_B;
		priv->pshare->phw->rx_path_start = RF92CD_PATH_A;
		priv->pshare->phw->rx_path_end = RF92CD_PATH_B;
	} else
	{
		priv->pshare->phw->tx_path_start = RF92CD_PATH_A;
		priv->pshare->phw->tx_path_end = RF92CD_PATH_A;
		priv->pshare->phw->rx_path_start = RF92CD_PATH_A;
		priv->pshare->phw->rx_path_end = RF92CD_PATH_A;
	}
}


#if defined(AC2G_256QAM) || defined(WLAN_HAL_8814AE)
char is_ac2g(struct rtl8192cd_priv * priv)
{

	unsigned char ac2g = 0;

	if((GET_CHIP_VER(priv)== VERSION_8812E) || (GET_CHIP_VER(priv)== VERSION_8881A))
	{
	if(priv->pmib->dot11BssType.net_work_type & WIRELESS_11AC
		&& ((priv->pmib->dot11BssType.net_work_type & WIRELESS_11A) == 0)
		&& (priv->pshare->rf_ft_var.ac2g_enable)
		)
		ac2g = 1;

		if((OPMODE&WIFI_STATION_STATE)
			&& (priv->pmib->dot11BssType.net_work_type == (WIRELESS_11A|WIRELESS_11B|WIRELESS_11G|WIRELESS_11N|WIRELESS_11AC))
			&& ((priv->pmib->dot11RFEntry.dot11channel != 0) && (priv->pmib->dot11RFEntry.dot11channel<36)))
			ac2g = 1;
	}

	if(GET_CHIP_VER(priv)==VERSION_8814A) {
		if(priv->pmib->dot11RFEntry.phyBandSelect ==  PHY_BAND_2G)
			ac2g = 1;
	}

	return ac2g;

}
#endif



/**
* Function:	PHY_QueryRFReg
*
* OverView:	Query "Specific bits" to RF register (page 8~)
*
* Input:
*			PADAPTER		Adapter,
*			RF92CD_RADIO_PATH_E	eRFPath,	//Radio path of A/B/C/D
*			u4Byte			RegAddr,		//The target address to be read
*			u4Byte			BitMask		//The target bit position in the target address
*										//to be read
*			u4Byte			dbg_avoid	//set bitmask in reg 0 to prevent RF switchs to debug mode
*
* Output:	None
* Return:		u4Byte			Readback value
* Note:		This function is equal to "GetRFRegSetting" in PHY programming guide
*/
unsigned int PHY_QueryRFReg(struct rtl8192cd_priv *priv, RF92CD_RADIO_PATH_E eRFPath,
							unsigned int RegAddr, unsigned int BitMask, unsigned int dbg_avoid)
{
#ifdef DISABLE_BB_RF
	return 0;
#endif //DISABLE_BB_RF


	if(IS_HAL_CHIP(priv)) {
		return GET_HAL_INTERFACE(priv)->PHYQueryRFRegHandler(priv, eRFPath, RegAddr, BitMask);
	} else if (CONFIG_WLAN_NOT_HAL_EXIST)
	{
		unsigned int	Original_Value, Readback_Value, BitShift;


		Original_Value = phy_RFSerialRead(priv, eRFPath, RegAddr, dbg_avoid);
		BitShift =  phy_CalculateBitShift(BitMask);
		Readback_Value = (Original_Value & BitMask) >> BitShift;
		return (Readback_Value);
	}
}


/**
* Function:	PHY_SetRFReg
*
* OverView:	Write "Specific bits" to RF register (page 8~)
*
* Input:
*			PADAPTER		Adapter,
*			RF92CD_RADIO_PATH_E	eRFPath,	//Radio path of A/B/C/D
*			u4Byte			RegAddr,		//The target address to be modified
*			u4Byte			BitMask		//The target bit position in the target address
*										//to be modified
*			u4Byte			Data			//The new register Data in the target bit position
*										//of the target address
*
* Output:	None
* Return:		None
* Note:		This function is equal to "PutRFRegSetting" in PHY programming guide
*/
#ifdef CONFIG_RTL_KERNEL_MIPS16_WLAN
__NOMIPS16
#endif
void PHY_SetRFReg(struct rtl8192cd_priv *priv, RF92CD_RADIO_PATH_E eRFPath, unsigned int RegAddr,
				  unsigned int BitMask, unsigned int Data)
{
#ifdef DISABLE_BB_RF
	return;
#endif //DISABLE_BB_RF


	if(IS_HAL_CHIP(priv)) {
		return GET_HAL_INTERFACE(priv)->PHYSSetRFRegHandler(priv, eRFPath, RegAddr, BitMask, Data);
	} else if (CONFIG_WLAN_NOT_HAL_EXIST)
	{
		unsigned int Original_Value, BitShift, New_Value;
		unsigned long flags;


		SAVE_INT_AND_CLI(flags);

		if (BitMask != bMask20Bits) {
			Original_Value = phy_RFSerialRead(priv, eRFPath, RegAddr, 1);
			BitShift = phy_CalculateBitShift(BitMask);
			New_Value = ((Original_Value & (~BitMask)) | (Data << BitShift));

			phy_RFSerialWrite(priv, eRFPath, RegAddr, New_Value);
		} else {
			phy_RFSerialWrite(priv, eRFPath, RegAddr, Data);
		}

		RESTORE_INT(flags);
		delay_us(4);    // At least 500ns delay to avoid RF write fail.
	}
}


#ifdef TXPWR_LMT_NEWFILE
static int is_NA(char* s)
{
	if (( s[0] == 'N' || s[0] == 'n') && ( s[1] == 'A' || s[1] == 'a'))
		return 1;
	else
		return 0;
}
#endif

static int is_hex(char s)
{
	if (( s >= '0' && s <= '9') || ( s >= 'a' && s <= 'f') || (s >= 'A' && s <= 'F') || (s == 'x' || s == 'X'))
		return 1;
	else
		return 0;
}


static int is_item(char s)
{
	if (s == 't' || s == 'a' || s == 'b' || s == 'l' || s == 'e'  || s == ':')
		return 1;
	else
		return 0;
}

static unsigned char *get_digit(unsigned char **data)
{
	unsigned char *buf = *data;
	int i = 0;

	while (buf[i] && ((buf[i] == ' ') || (buf[i] == '\t')))
		i++;
	*data = &buf[i];

	while (buf[i]) {
		if ((buf[i] == ' ') || (buf[i] == '\t')) {
			buf[i] = '\0';
			break;
		}
		if (buf[i] >= 'A' && buf[i] <= 'Z')
			buf[i] += 32;

		if (!is_hex(buf[i]) && !is_item(buf[i]))
			return NULL;
		i++;
	}
	if (i == 0)
		return NULL;
	else
		return &buf[i + 1];
}

#ifdef TXPWR_LMT_NEWFILE

static unsigned char *get_digit_dot_8812(unsigned char **data)
{
	unsigned char *buf=*data;
	int i=0;

	while(buf[i] && ((buf[i] == ' ') || (buf[i] == '\t')))
		i++;

	*data = &buf[i];

	while(buf[i]) {
		if(buf[i] == '.'){
//			while((buf[i]==' ') ||(buf[i]=='\t') || (buf[i]=='\0') || (buf[i]=='/'))
//				i++;

			i++;
		}

		if ((buf[i] == ' ') || (buf[i] == '\t')) {
			buf[i] = '\0';
			break;
		}

		if (buf[i]>='A' && buf[i]<='Z')
			buf[i] += 32;

		if (!is_hex(buf[i])&&!is_item(buf[i])
			&& !is_NA((char *)&buf[i]))
			return NULL;

		i++;
	}
	if (i == 0)
		return NULL;
	else
		{
			return &buf[i+1];
		}
}


#define LMT_NUM 256
static int get_chnl_lmt_dot_new(struct rtl8192cd_priv *priv, unsigned char *line_head, unsigned int *channel, unsigned int *limit, unsigned int *table_idx)
{
	unsigned char *next, *next2, *next3;
	int base, idx;
	int num=0;
	int num_index;
	int val;
	int i; // for loop
	unsigned char *ch;
	unsigned char lmt[LMT_NUM];
	unsigned char *twpwr_lmt_index_array;
	unsigned char lmtIndex;
	extern int _atoi(char *s, int base);

	*channel = '\0';
	base = 10;

	//find limit table index first
	if (!memcmp(line_head+2,"Table",5)) {
		*channel = 0;
		//printk("found idx\n");
		goto find_idx;
	}
	else if (!memcmp(line_head,"CH0",3)) {
		//printk("found channel\n");
		ch = line_head+3;
	}
	else if (!memcmp(line_head,"CH",2)) {
		//printk("found channel\n");
		ch = line_head+2;
	}
	else
		return 0;

	*channel = _atoi(ch, base);


	num = 0;
	next = get_digit_dot_8812(&ch);

	while(next != NULL)
	{
		num++;
		if(is_NA(next))
			lmt[num] = 0; //No limit
		else {
			val = _convert_2_pwr_dot(next, base);
			if(val == -1)
				num --;
			else
				lmt[num] = val;
		}
		next = get_digit_dot_8812(&next);

	}
	if(num == 0)
		panic_printk("TX Power Porfile format Error\n");

	//printk("channel = %d, lmt = %d %d %d\n", *channel, lmt[1], lmt[2], lmt[3]);


	if(priv->pmib->dot11StationConfigEntry.txpwr_lmt_index <= 0) {
		if(priv->pmib->dot11StationConfigEntry.dot11RegDomain == DOMAIN_MKK)
			lmtIndex = 3;
		else if(priv->pmib->dot11StationConfigEntry.dot11RegDomain == DOMAIN_ETSI)
			lmtIndex = 2;
		else
			lmtIndex = 1;
	} else {
		lmtIndex = priv->pmib->dot11StationConfigEntry.txpwr_lmt_index;
	}

	*limit = lmt[lmtIndex];

	return 1;

find_idx:

	*channel = 0;
	*limit = 0;
	*table_idx = 0;

	line_head = line_head + 8;

	*table_idx = _atoi(line_head, base); //In this condition, 'limit' represents "# of table"
	//printk("\n[table_idx = %d]\n", *table_idx);

	return 1;

}
#endif


static unsigned char *get_digit_dot(unsigned char **data)
{
	unsigned char *buf = *data;
	int i = 0;

	while (buf[i] && ((buf[i] == ' ') || (buf[i] == '\t')))
		i++;

	*data = &buf[i];

	while (buf[i]) {
		if (buf[i] == '.') {
//			while ((buf[i] == ' ') || (buf[i] == '\t') || (buf[i] == '\0') || (buf[i] == '/'))
//				i++;

			i++;
		}

		if ((buf[i] == ' ') || (buf[i] == '\t')) {
			buf[i] = '\0';
			break;
		}
		if (buf[i] >= 'A' && buf[i] <= 'Z')
			buf[i] += 32;

		if (!is_hex(buf[i]) && !is_item(buf[i]))
			return NULL;
		i++;
	}
	if (i == 0)
		return NULL;
	else
		{
		return &buf[i + 1];
	}
}


#ifdef TXPWR_LMT
static int get_chnl_lmt_dot(unsigned char *line_head, unsigned int *ch_start, unsigned int *ch_end, unsigned int *limit, unsigned int *target)
{
	unsigned char *next, *next2;
	int base, idx;
	int num = 0, round = 0;
	unsigned char *ch;
	extern int _atoi(char * s, int base);

	*ch_start = *ch_start = '\0';

	// remove comments
	ch = line_head;
	while (1) {
		if ((*ch == '\0') || (*ch == '\n') || (*ch == '\r'))
			break;
		else if (*ch == '/') {
			*ch = '\0';
			break;
		} else {
			ch++;
		}

		if (++round > 10000) {
			panic_printk("%s[%d] while (1) goes too many\n", __FUNCTION__, __LINE__);
			break;
		}
	}

	next = get_digit_dot(&line_head);
	if (next == NULL)
		return num;
	num++;

	if (!memcmp(line_head, "table", 5)) {
		*ch_start = 0;
		*ch_end = 0;
	} else {
//		char *s;
		int format = 0;
		int idx2;

		if ((!memcmp(line_head, "0x", 2)) || (!memcmp(line_head, "0X", 2))) {
			base = 16;
			idx = 2;
		} else {
			base = 10;
			idx = 0;
		}
		idx2 = idx;
		while (line_head[idx2] != '\0') {
			//printk("(%c)",line_head[idx2]);
			if (line_head[idx2] == ':') {
				line_head[idx2] = '\0';
				format = 1; // format - start:end
				break;
			}
			idx2++;
		}
		*ch_start = _atoi((char *)&line_head[idx], base);
		if (format == 0) {
			*ch_end = *ch_start;
		} else {
			*ch_end = _atoi((char *)&line_head[idx2 + 1], base);
		}
	}


	*limit = 0;
	if (next) {
		if (!(next2 = get_digit_dot(&next)))
			return num;
		num++;

		base = 10;
		idx = 0;

		if ( (*ch_start == 0) && (*ch_end == 0) )
			*limit = _atoi((char *)&next[idx], base); //In this condition, 'limit' represents "# of table"
		else
			*limit = _convert_2_pwr_dot((char *)&next[idx], base);
	}


	*target = 0;
	if (next2) {
		if (!get_digit_dot(&next2))
			return num;
		num++;

		base = 10;
		idx = 0;

		*target = _convert_2_pwr_dot((char *)&next2[idx], base);
	}


	return num;
}
#endif

//static
int get_offset_val(unsigned char *line_head, unsigned int *u4bRegOffset, unsigned int *u4bRegValue)
{
	unsigned char *next;
	int base, idx;
	int num = 0, round = 0;
	unsigned char *ch;
	extern int _atoi(char * s, int base);

	*u4bRegOffset = *u4bRegValue = '\0';

	ch = line_head;
	while (1) {
		if ((*ch == '\0') || (*ch == '\n') || (*ch == '\r'))
			break;
		else if (*ch == '/') {
			*ch = '\0';
			break;
		} else {
			ch++;
		}

		if (++round > 10000) {
			panic_printk("%s[%d] while (1) goes too many\n", __FUNCTION__, __LINE__);
			break;
		}
	}

	next = get_digit(&line_head);
	if (next == NULL)
		return num;
	num++;
	if ((!memcmp(line_head, "0x", 2)) || (!memcmp(line_head, "0X", 2))) {
		base = 16;
		idx = 2;
	} else {
		base = 10;
		idx = 0;
	}
	*u4bRegOffset = _atoi((char *)&line_head[idx], base);

	if (next) {
		if (!get_digit(&next))
			return num;
		num++;
		if ((!memcmp(next, "0x", 2)) || (!memcmp(next, "0X", 2))) {
			base = 16;
			idx = 2;
		} else {
			base = 10;
			idx = 0;
		}
		*u4bRegValue = _atoi((char *)&next[idx], base);
	} else
		*u4bRegValue = 0;

	return num;
}


#if 0//defined(CONFIG_RTL_88E_SUPPORT) || defined(CONFIG_WLAN_HAL_8192EE)
void assign_target_value_88e_new(struct rtl8192cd_priv *priv, unsigned int *u4bRegOffset, unsigned char *tmp_byte)
{
	//CCK-11M
	if(*u4bRegOffset == rTxAGC_A_CCK11_2_B_CCK11)
	if(tmp_byte[3] != 0)
		priv->pshare->tgpwr_CCK_new[RF_PATH_A] = tmp_byte[3];

	if(*u4bRegOffset == rTxAGC_A_CCK11_2_B_CCK11)
	if(tmp_byte[0] != 0)
		priv->pshare->tgpwr_CCK_new[RF_PATH_B] = tmp_byte[0];

	//OFDM-54M
	if(*u4bRegOffset == rTxAGC_A_Rate54_24)
		priv->pshare->tgpwr_OFDM_new[RF_PATH_A] = tmp_byte[3];

	if(*u4bRegOffset == rTxAGC_B_Rate54_24)
		priv->pshare->tgpwr_OFDM_new[RF_PATH_B] = tmp_byte[3];

	//HT-MCS7
	if(*u4bRegOffset == rTxAGC_A_Mcs07_Mcs04)
		priv->pshare->tgpwr_HT1S_new[RF_PATH_A] = tmp_byte[3];

	if(*u4bRegOffset == rTxAGC_B_Mcs07_Mcs04)
		priv->pshare->tgpwr_HT1S_new[RF_PATH_B] = tmp_byte[3];

	//HT-MCS15
	if(*u4bRegOffset == rTxAGC_A_Mcs15_Mcs12)
		priv->pshare->tgpwr_HT2S_new[RF_PATH_A] = tmp_byte[3];

	if(*u4bRegOffset == rTxAGC_B_Mcs15_Mcs12)
		priv->pshare->tgpwr_HT2S_new[RF_PATH_B] = tmp_byte[3];


}
#endif
#if 0//defined( CONFIG_RTL_8812_SUPPORT) || defined(CONFIG_WLAN_HAL_8881A)
void assign_target_value_8812_new(struct rtl8192cd_priv *priv, unsigned int *u4bRegOffset, unsigned char *tmp_byte)
{
	//CCK-11M
	if(*u4bRegOffset == rTxAGC_A_CCK11_CCK1_JAguar)
		priv->pshare->tgpwr_CCK_new[RF_PATH_A] = tmp_byte[3];

	if(*u4bRegOffset == rTxAGC_B_CCK11_CCK1_JAguar)
		priv->pshare->tgpwr_CCK_new[RF_PATH_B] = tmp_byte[3];

	//OFDM-54M
	if(*u4bRegOffset == rTxAGC_A_Ofdm54_Ofdm24_JAguar)
		priv->pshare->tgpwr_OFDM_new[RF_PATH_A] = tmp_byte[3];

	if(*u4bRegOffset == rTxAGC_B_Ofdm54_Ofdm24_JAguar)
		priv->pshare->tgpwr_OFDM_new[RF_PATH_B] = tmp_byte[3];

	//HT-MCS7
	if(*u4bRegOffset == rTxAGC_A_MCS7_MCS4_JAguar)
		priv->pshare->tgpwr_HT1S_new[RF_PATH_A] = tmp_byte[3];

	if(*u4bRegOffset == rTxAGC_B_MCS7_MCS4_JAguar)
		priv->pshare->tgpwr_HT1S_new[RF_PATH_B] = tmp_byte[3];

	//HT-MCS15
	if(*u4bRegOffset == rTxAGC_A_MCS15_MCS12_JAguar)
		priv->pshare->tgpwr_HT2S_new[RF_PATH_A] = tmp_byte[3];

	if(*u4bRegOffset == rTxAGC_B_MCS15_MCS12_JAguar)
		priv->pshare->tgpwr_HT2S_new[RF_PATH_B] = tmp_byte[3];

	//VHT-NSS1-MCS7
	if(*u4bRegOffset == rTxAGC_A_Nss1Index7_Nss1Index4_JAguar)
		priv->pshare->tgpwr_VHT1S_new[RF_PATH_A] = tmp_byte[3];

	if(*u4bRegOffset == rTxAGC_B_Nss1Index7_Nss1Index4_JAguar)
		priv->pshare->tgpwr_VHT1S_new[RF_PATH_B] = tmp_byte[3];

	//VHT-NSS2-MCS7
	if(*u4bRegOffset == rTxAGC_A_Nss2Index9_Nss2Index6_JAguar)
		priv->pshare->tgpwr_VHT2S_new[RF_PATH_A] = tmp_byte[1];

	if(*u4bRegOffset == rTxAGC_B_Nss2Index9_Nss2Index6_JAguar)
		priv->pshare->tgpwr_VHT2S_new[RF_PATH_B] = tmp_byte[1];

}
#endif

unsigned int gen_reg_value(unsigned char* tmp_reg)
{
	return (tmp_reg[0] | (tmp_reg[1]<<8) | (tmp_reg[2]<<16) | (tmp_reg[3]<<24)) ;
}

unsigned int gen_reg_value_2(unsigned char target, unsigned char* tmp_byte, unsigned char* tmp_reg)
{
	unsigned char i =0;

	for(i=0; i<4; i++)
	{
		if(target < tmp_byte[i])
			tmp_reg[i] = tmp_byte[i] - target;
		else
			tmp_reg[i] = target - tmp_byte[i];
	}

	return (tmp_reg[0] | (tmp_reg[1]<<8) | (tmp_reg[2]<<16) | (tmp_reg[3]<<24)) ;
}


#if defined(RTK_AC_SUPPORT)
unsigned int generate_u4bRegValue_8812_new(struct rtl8192cd_priv *priv, unsigned int *u4bRegOffset, unsigned char *tmp_byte)
{
	unsigned char tmp_reg[4];
	unsigned int  reg_value = 0;
	unsigned char target = 0;
	int i;
	if(*u4bRegOffset == rTxAGC_A_Nss2Index1_Nss1Index8_JAguar)
	{
		tmp_reg[0]= tmp_byte[0]- priv->pshare->tgpwr_VHT1S_new[RF_PATH_A]; // tgpwr_VHT1S_new is ZERO?
		tmp_reg[1]= tmp_byte[1]- priv->pshare->tgpwr_VHT1S_new[RF_PATH_A];
		tmp_reg[2]= tmp_byte[2]- priv->pshare->tgpwr_VHT2S_new[RF_PATH_A];
		tmp_reg[3]= tmp_byte[3]- priv->pshare->tgpwr_VHT2S_new[RF_PATH_A];

		reg_value = gen_reg_value(tmp_reg);
		return reg_value;
	}
	else if(*u4bRegOffset == rTxAGC_B_Nss2Index1_Nss1Index8_JAguar)
	{
		tmp_reg[0]= tmp_byte[0]- priv->pshare->tgpwr_VHT1S_new[RF_PATH_B];
		tmp_reg[1]= tmp_byte[1]- priv->pshare->tgpwr_VHT1S_new[RF_PATH_B];
		tmp_reg[2]= tmp_byte[2]- priv->pshare->tgpwr_VHT2S_new[RF_PATH_B];
		tmp_reg[3]= tmp_byte[3]- priv->pshare->tgpwr_VHT2S_new[RF_PATH_B];

		reg_value = gen_reg_value(tmp_reg);
		return reg_value;
	}

	switch (*u4bRegOffset) {
		case rTxAGC_A_CCK11_CCK1_JAguar:
			target = priv->pshare->tgpwr_CCK_new[RF_PATH_A]; // tgpwr_CCK_new is ZERO?
			break;
		case rTxAGC_A_Ofdm18_Ofdm6_JAguar:
			target = priv->pshare->tgpwr_OFDM_new[RF_PATH_A];
			break;
		case rTxAGC_A_Ofdm54_Ofdm24_JAguar:
			target = priv->pshare->tgpwr_OFDM_new[RF_PATH_A];
			break;
		case rTxAGC_A_MCS3_MCS0_JAguar:
			target = priv->pshare->tgpwr_HT1S_new[RF_PATH_A];
			break;
		case rTxAGC_A_MCS7_MCS4_JAguar:
			target = priv->pshare->tgpwr_HT1S_new[RF_PATH_A];
			break;
		case rTxAGC_A_MCS11_MCS8_JAguar:
			target = priv->pshare->tgpwr_HT2S_new[RF_PATH_A];
			break;
		case rTxAGC_A_MCS15_MCS12_JAguar:
			target = priv->pshare->tgpwr_HT2S_new[RF_PATH_A];
			break;
		case rTxAGC_A_Nss1Index3_Nss1Index0_JAguar:
			target = priv->pshare->tgpwr_VHT1S_new[RF_PATH_A];
			break;
		case rTxAGC_A_Nss1Index7_Nss1Index4_JAguar:
			target = priv->pshare->tgpwr_VHT1S_new[RF_PATH_A];
			break;
		case rTxAGC_A_Nss2Index1_Nss1Index8_JAguar:
			break;
		case rTxAGC_A_Nss2Index5_Nss2Index2_JAguar:
			target = priv->pshare->tgpwr_VHT2S_new[RF_PATH_A];
			break;
		case rTxAGC_A_Nss2Index9_Nss2Index6_JAguar:
			target = priv->pshare->tgpwr_VHT2S_new[RF_PATH_A];
			break;
		case rTxAGC_B_CCK11_CCK1_JAguar:
			target = priv->pshare->tgpwr_CCK_new[RF_PATH_B];
			break;
		case rTxAGC_B_Ofdm18_Ofdm6_JAguar:
			target = priv->pshare->tgpwr_OFDM_new[RF_PATH_B];
			break;
		case rTxAGC_B_Ofdm54_Ofdm24_JAguar:
			target = priv->pshare->tgpwr_OFDM_new[RF_PATH_B];
			break;
		case rTxAGC_B_MCS3_MCS0_JAguar:
			target = priv->pshare->tgpwr_HT1S_new[RF_PATH_B];
			break;
		case rTxAGC_B_MCS7_MCS4_JAguar:
			target = priv->pshare->tgpwr_HT1S_new[RF_PATH_B];
			break;
		case rTxAGC_B_MCS11_MCS8_JAguar:
			target = priv->pshare->tgpwr_HT2S_new[RF_PATH_B];
			break;
		case rTxAGC_B_MCS15_MCS12_JAguar:
			target = priv->pshare->tgpwr_HT2S_new[RF_PATH_B];
			break;
		case rTxAGC_B_Nss1Index3_Nss1Index0_JAguar:
			target = priv->pshare->tgpwr_VHT1S_new[RF_PATH_B];
			break;
		case rTxAGC_B_Nss1Index7_Nss1Index4_JAguar:
			target = priv->pshare->tgpwr_VHT1S_new[RF_PATH_B];
			break;
		case rTxAGC_B_Nss2Index1_Nss1Index8_JAguar:
			break;
		case rTxAGC_B_Nss2Index5_Nss2Index2_JAguar:
			target = priv->pshare->tgpwr_VHT2S_new[RF_PATH_B];
			break;
		case rTxAGC_B_Nss2Index9_Nss2Index6_JAguar:
			target = priv->pshare->tgpwr_VHT2S_new[RF_PATH_B];
			break;
	}

	reg_value = gen_reg_value_2(target, tmp_byte, tmp_reg);
	return reg_value;

}
#endif

void set_power_by_rate(struct rtl8192cd_priv *priv, unsigned int *u4bRegOffset, unsigned char *tmp_byte)
{
	unsigned char tmp_reg[4];
	unsigned int  reg_value = 0;
	unsigned char target = 0;
	int i;

	//panic_printk("[%s] u4bRegOffset=%x, tmp_byte=%x\n",__FUNCTION__,*u4bRegOffset, *((unsigned int*)tmp_byte));
	switch (*u4bRegOffset) {
		case rTxAGC_A_CCK1_Mcs32:
				priv->pshare->phw->CCKTxAgc_A[3] = (char)tmp_byte[1];
				//printk("CCK_A[3] (1M)=%x\n",priv->pshare->phw->CCKTxAgc_A[3]);u
				break;
		case rTxAGC_A_CCK11_2_B_CCK11:
				priv->pshare->phw->CCKTxAgc_B[0] = (char)tmp_byte[0]?(char)tmp_byte[0]:priv->pshare->phw->CCKTxAgc_B[0];
				priv->pshare->phw->CCKTxAgc_A[2] = (char)tmp_byte[1]?(char)tmp_byte[1]:priv->pshare->phw->CCKTxAgc_A[2];
				priv->pshare->phw->CCKTxAgc_A[1] = (char)tmp_byte[2]?(char)tmp_byte[2]:priv->pshare->phw->CCKTxAgc_A[1] ;
				priv->pshare->phw->CCKTxAgc_A[0] = (char)tmp_byte[3]?(char)tmp_byte[3]:priv->pshare->phw->CCKTxAgc_A[0] ;
				//for(i=0;i<4;i++)
				//	printk("CCK_A[%d]=%x\n",i,priv->pshare->phw->CCKTxAgc_A[i]);
				//printk("CCK_B[0] (11M)=%x\n",priv->pshare->phw->CCKTxAgc_B[0]);
				break;
		case rTxAGC_A_Rate18_06:
				priv->pshare->phw->OFDMTxAgcOffset_A[3] = (char)tmp_byte[0];
				priv->pshare->phw->OFDMTxAgcOffset_A[2] = (char)tmp_byte[1];
				priv->pshare->phw->OFDMTxAgcOffset_A[1] = (char)tmp_byte[2];
				priv->pshare->phw->OFDMTxAgcOffset_A[0] = (char)tmp_byte[3];
				//for(i=0;i<4;i++)
				//	printk("OFDM_A[%d]=%x\n",i,priv->pshare->phw->OFDMTxAgcOffset_A[i]);
				break;
		case rTxAGC_A_Rate54_24:
				priv->pshare->phw->OFDMTxAgcOffset_A[7] = (char)tmp_byte[0];
				priv->pshare->phw->OFDMTxAgcOffset_A[6] = (char)tmp_byte[1];
				priv->pshare->phw->OFDMTxAgcOffset_A[5] = (char)tmp_byte[2];
				priv->pshare->phw->OFDMTxAgcOffset_A[4] = (char)tmp_byte[3];
				//for(i=4;i<8;i++)
				//	printk("OFDM_A[%d]=%x\n",i,priv->pshare->phw->OFDMTxAgcOffset_A[i]);
				break;
		case rTxAGC_A_Mcs03_Mcs00:
				priv->pshare->phw->MCSTxAgcOffset_A[3] = (char)tmp_byte[0];
				priv->pshare->phw->MCSTxAgcOffset_A[2] = (char)tmp_byte[1];
				priv->pshare->phw->MCSTxAgcOffset_A[1] = (char)tmp_byte[2];
				priv->pshare->phw->MCSTxAgcOffset_A[0] = (char)tmp_byte[3];
				//for(i=0;i<4;i++)
				//	printk("MCS_A[%d]=%x\n",i,priv->pshare->phw->MCSTxAgcOffset_A[i]);
				break;
		case rTxAGC_A_Mcs07_Mcs04:
				priv->pshare->phw->MCSTxAgcOffset_A[7] = (char)tmp_byte[0];
				priv->pshare->phw->MCSTxAgcOffset_A[6] = (char)tmp_byte[1];
				priv->pshare->phw->MCSTxAgcOffset_A[5] = (char)tmp_byte[2];
				priv->pshare->phw->MCSTxAgcOffset_A[4] = (char)tmp_byte[3];
				//for(i=4;i<8;i++)
				//	printk("MCS_A[%d]=%x\n",i,priv->pshare->phw->MCSTxAgcOffset_A[i]);
				break;
		case rTxAGC_A_Mcs11_Mcs08:
				priv->pshare->phw->MCSTxAgcOffset_A[11] = (char)tmp_byte[0];
				priv->pshare->phw->MCSTxAgcOffset_A[10] = (char)tmp_byte[1];
				priv->pshare->phw->MCSTxAgcOffset_A[9] = (char)tmp_byte[2];
				priv->pshare->phw->MCSTxAgcOffset_A[8] = (char)tmp_byte[3];
				//for(i=8;i<12;i++)
				//	printk("MCS_A[%d]=%x\n",i,priv->pshare->phw->MCSTxAgcOffset_A[i]);
				break;
		case rTxAGC_A_Mcs15_Mcs12:
				priv->pshare->phw->MCSTxAgcOffset_A[15] = (char)tmp_byte[0];
				priv->pshare->phw->MCSTxAgcOffset_A[14] = (char)tmp_byte[1];
				priv->pshare->phw->MCSTxAgcOffset_A[13] = (char)tmp_byte[2];
				priv->pshare->phw->MCSTxAgcOffset_A[12] = (char)tmp_byte[3];
				//for(i=12;i<16;i++)
				//	printk("MCS_A[%d]=%x\n",i,priv->pshare->phw->MCSTxAgcOffset_A[i]);
				break;
		case rTxAGC_B_CCK5_1_Mcs32:
				priv->pshare->phw->CCKTxAgc_B[3] = (char)tmp_byte[1];
				priv->pshare->phw->CCKTxAgc_B[2] = (char)tmp_byte[2];
				priv->pshare->phw->CCKTxAgc_B[1] = (char)tmp_byte[3];
				//for(i=0;i<4;i++)
				//	printk("CCK_B[%d]=%x\n",i,priv->pshare->phw->CCKTxAgc_B[i]);
				break;
		case rTxAGC_B_Rate18_06:
				priv->pshare->phw->OFDMTxAgcOffset_B[3] = (char)tmp_byte[0];
				priv->pshare->phw->OFDMTxAgcOffset_B[2] = (char)tmp_byte[1];
				priv->pshare->phw->OFDMTxAgcOffset_B[1] = (char)tmp_byte[2];
				priv->pshare->phw->OFDMTxAgcOffset_B[0] = (char)tmp_byte[3];
				//for(i=0;i<4;i++)
				//	printk("OFDM_B[%d]=%x\n",i,priv->pshare->phw->OFDMTxAgcOffset_B[i]);
				break;
		case rTxAGC_B_Rate54_24:
				priv->pshare->phw->OFDMTxAgcOffset_B[7] = (char)tmp_byte[0];
				priv->pshare->phw->OFDMTxAgcOffset_B[6] = (char)tmp_byte[1];
				priv->pshare->phw->OFDMTxAgcOffset_B[5] = (char)tmp_byte[2];
				priv->pshare->phw->OFDMTxAgcOffset_B[4] = (char)tmp_byte[3];
				//for(i=4;i<8;i++)
				//	printk("OFDM_B[%d]=%x\n",i,priv->pshare->phw->OFDMTxAgcOffset_B[i]);
				break;
		case rTxAGC_B_Mcs03_Mcs00:
				priv->pshare->phw->MCSTxAgcOffset_B[3] = (char)tmp_byte[0];
				priv->pshare->phw->MCSTxAgcOffset_B[2] = (char)tmp_byte[1];
				priv->pshare->phw->MCSTxAgcOffset_B[1] = (char)tmp_byte[2];
				priv->pshare->phw->MCSTxAgcOffset_B[0] = (char)tmp_byte[3];
				//for(i=0;i<4;i++)
				//	printk("MCS_B[%d]=%x\n",i,priv->pshare->phw->MCSTxAgcOffset_B[i]);
				break;
		case rTxAGC_B_Mcs07_Mcs04:
				priv->pshare->phw->MCSTxAgcOffset_B[7] = (char)tmp_byte[0];
				priv->pshare->phw->MCSTxAgcOffset_B[6] = (char)tmp_byte[1];
				priv->pshare->phw->MCSTxAgcOffset_B[5] = (char)tmp_byte[2];
				priv->pshare->phw->MCSTxAgcOffset_B[4] = (char)tmp_byte[3];
				//for(i=4;i<8;i++)
				//	printk("MCS_B[%d]=%x\n",i,priv->pshare->phw->MCSTxAgcOffset_B[i]);
				break;
		case rTxAGC_B_Mcs11_Mcs08:
				priv->pshare->phw->MCSTxAgcOffset_B[11] = (char)tmp_byte[0];
				priv->pshare->phw->MCSTxAgcOffset_B[10] = (char)tmp_byte[1];
				priv->pshare->phw->MCSTxAgcOffset_B[9] = (char)tmp_byte[2];
				priv->pshare->phw->MCSTxAgcOffset_B[8] = (char)tmp_byte[3];
				//for(i=8;i<12;i++)
				//	printk("MCS_B[%d]=%x\n",i,priv->pshare->phw->MCSTxAgcOffset_B[i]);
				break;
		case rTxAGC_B_Mcs15_Mcs12:
				priv->pshare->phw->MCSTxAgcOffset_B[15] = (char)tmp_byte[0];
				priv->pshare->phw->MCSTxAgcOffset_B[14] = (char)tmp_byte[1];
				priv->pshare->phw->MCSTxAgcOffset_B[13] = (char)tmp_byte[2];
				priv->pshare->phw->MCSTxAgcOffset_B[12] = (char)tmp_byte[3];
				//for(i=12;i<16;i++)
				//	printk("MCS_B[%d]=%x\n",i,priv->pshare->phw->MCSTxAgcOffset_B[i]);
				break;
	}
}
void set_power_by_rate_8814(struct rtl8192cd_priv *priv, unsigned int *u4bRegOffset, unsigned char *tmp_byte)
{
	unsigned char tmp_reg[4];
	unsigned int  reg_value = 0;
	unsigned char target = 0;
	int i;

	switch (*u4bRegOffset) {
		case rTxAGC_A_CCK11_CCK1_JAguar:
				priv->pshare->phw->CCKTxAgc_A[0] = (char)tmp_byte[0];
				priv->pshare->phw->CCKTxAgc_A[1] = (char)tmp_byte[1];
				priv->pshare->phw->CCKTxAgc_A[2] = (char)tmp_byte[2];
				priv->pshare->phw->CCKTxAgc_A[3] = (char)tmp_byte[3];
				//for(i=0;i<4;i++)
					//printk("priv->pshare->phw->CCKTxAgc_A[%d]=%x\n",i,priv->pshare->phw->CCKTxAgc_A[i]);
			break;
		case rTxAGC_A_Ofdm18_Ofdm6_JAguar:
				priv->pshare->phw->OFDMTxAgcOffset_A[0] = (char)tmp_byte[0];
				priv->pshare->phw->OFDMTxAgcOffset_A[1] = (char)tmp_byte[1];
				priv->pshare->phw->OFDMTxAgcOffset_A[2] = (char)tmp_byte[2];
				priv->pshare->phw->OFDMTxAgcOffset_A[3] = (char)tmp_byte[3];
				//for(i=0;i<4;i++)
					//printk("priv->pshare->phw->OFDMTxAgcOffset_A[%d]=%x\n",i,priv->pshare->phw->OFDMTxAgcOffset_A[i]);
			break;
		case rTxAGC_A_Ofdm54_Ofdm24_JAguar:
				priv->pshare->phw->OFDMTxAgcOffset_A[4] = (char)tmp_byte[0];
				priv->pshare->phw->OFDMTxAgcOffset_A[5] = (char)tmp_byte[1];
				priv->pshare->phw->OFDMTxAgcOffset_A[6] = (char)tmp_byte[2];
				priv->pshare->phw->OFDMTxAgcOffset_A[7] = (char)tmp_byte[3];
				//for(i=4;i<8;i++)
					//printk("priv->pshare->phw->OFDMTxAgcOffset_A[%d]=%x\n",i,priv->pshare->phw->OFDMTxAgcOffset_A[i]);
			break;
		case rTxAGC_A_MCS3_MCS0_JAguar:
				priv->pshare->phw->MCSTxAgcOffset_A[0] = (char)tmp_byte[0];
				priv->pshare->phw->MCSTxAgcOffset_A[1] = (char)tmp_byte[1];
				priv->pshare->phw->MCSTxAgcOffset_A[2] = (char)tmp_byte[2];
				priv->pshare->phw->MCSTxAgcOffset_A[3] = (char)tmp_byte[3];
				//for(i=0;i<4;i++)
					//printk("priv->pshare->phw->MCSTxAgcOffset_A[%d]=%x\n",i,priv->pshare->phw->MCSTxAgcOffset_A[i]);
			break;
		case rTxAGC_A_MCS7_MCS4_JAguar:
				priv->pshare->phw->MCSTxAgcOffset_A[4] = (char)tmp_byte[0];
				priv->pshare->phw->MCSTxAgcOffset_A[5] = (char)tmp_byte[1];
				priv->pshare->phw->MCSTxAgcOffset_A[6] = (char)tmp_byte[2];
				priv->pshare->phw->MCSTxAgcOffset_A[7] = (char)tmp_byte[3];
				//for(i=4;i<8;i++)
					//printk("priv->pshare->phw->MCSTxAgcOffset_A[%d]=%x\n",i,priv->pshare->phw->MCSTxAgcOffset_A[i]);
			break;
		case rTxAGC_A_MCS11_MCS8_JAguar:
				priv->pshare->phw->MCSTxAgcOffset_A[8] = (char)tmp_byte[0];
				priv->pshare->phw->MCSTxAgcOffset_A[9] = (char)tmp_byte[1];
				priv->pshare->phw->MCSTxAgcOffset_A[10] = (char)tmp_byte[2];
				priv->pshare->phw->MCSTxAgcOffset_A[11] = (char)tmp_byte[3];
				//for(i=8;i<12;i++)
				//	printk("priv->pshare->phw->MCSTxAgcOffset_A[%d]=%x\n",i,priv->pshare->phw->MCSTxAgcOffset_A[i]);
			break;
		case rTxAGC_A_MCS15_MCS12_JAguar:
				priv->pshare->phw->MCSTxAgcOffset_A[12] = (char)tmp_byte[0];
				priv->pshare->phw->MCSTxAgcOffset_A[13] = (char)tmp_byte[1];
				priv->pshare->phw->MCSTxAgcOffset_A[14] = (char)tmp_byte[2];
				priv->pshare->phw->MCSTxAgcOffset_A[15] = (char)tmp_byte[3];
				//for(i=12;i<16;i++)
				//	printk("priv->pshare->phw->MCSTxAgcOffset_A[%d]=%x\n",i,priv->pshare->phw->MCSTxAgcOffset_A[i]);
			break;
		case rTxAGC_A_Nss1Index3_Nss1Index0_JAguar:
				priv->pshare->phw->VHTTxAgcOffset_A[0] = (char)tmp_byte[0];
				priv->pshare->phw->VHTTxAgcOffset_A[1] = (char)tmp_byte[1];
				priv->pshare->phw->VHTTxAgcOffset_A[2] = (char)tmp_byte[2];
				priv->pshare->phw->VHTTxAgcOffset_A[3] = (char)tmp_byte[3];
				//for(i=0;i<4;i++)
				//	printk("priv->pshare->phw->VHTTxAgcOffset_A[%d]=%x\n",i,priv->pshare->phw->VHTTxAgcOffset_A[i]);
			break;
		case rTxAGC_A_Nss1Index7_Nss1Index4_JAguar:
				priv->pshare->phw->VHTTxAgcOffset_A[4] = (char)tmp_byte[0];
				priv->pshare->phw->VHTTxAgcOffset_A[5] = (char)tmp_byte[1];
				priv->pshare->phw->VHTTxAgcOffset_A[6] = (char)tmp_byte[2];
				priv->pshare->phw->VHTTxAgcOffset_A[7] = (char)tmp_byte[3];
				//for(i=4;i<8;i++)
				//	printk("priv->pshare->phw->VHTTxAgcOffset_A[%d]=%x\n",i,priv->pshare->phw->VHTTxAgcOffset_A[i]);
			break;
		case rTxAGC_A_Nss2Index1_Nss1Index8_JAguar:
				priv->pshare->phw->VHTTxAgcOffset_A[8] = (char)tmp_byte[0];
				priv->pshare->phw->VHTTxAgcOffset_A[9] = (char)tmp_byte[1];
				priv->pshare->phw->VHTTxAgcOffset_A[10] = (char)tmp_byte[2];
				priv->pshare->phw->VHTTxAgcOffset_A[11] = (char)tmp_byte[3];
				//for(i=8;i<12;i++)
				//	printk("priv->pshare->phw->VHTTxAgcOffset_A[%d]=%x\n",i,priv->pshare->phw->VHTTxAgcOffset_A[i]);
			break;
		case rTxAGC_A_Nss2Index5_Nss2Index2_JAguar:
				priv->pshare->phw->VHTTxAgcOffset_A[12] = (char)tmp_byte[0];
				priv->pshare->phw->VHTTxAgcOffset_A[13] = (char)tmp_byte[1];
				priv->pshare->phw->VHTTxAgcOffset_A[14] = (char)tmp_byte[2];
				priv->pshare->phw->VHTTxAgcOffset_A[15] = (char)tmp_byte[3];
				//for(i=12;i<16;i++)
				//	printk("priv->pshare->phw->VHTTxAgcOffset_A[%d]=%x\n",i,priv->pshare->phw->VHTTxAgcOffset_A[i]);
			break;
		case rTxAGC_A_Nss2Index9_Nss2Index6_JAguar:
				priv->pshare->phw->VHTTxAgcOffset_A[16] = (char)tmp_byte[0];
				priv->pshare->phw->VHTTxAgcOffset_A[17] = (char)tmp_byte[1];
				priv->pshare->phw->VHTTxAgcOffset_A[18] = (char)tmp_byte[2];
				priv->pshare->phw->VHTTxAgcOffset_A[19] = (char)tmp_byte[3];
				//for(i=16;i<20;i++)
				//	printk("priv->pshare->phw->VHTTxAgcOffset_A[%d]=%x\n",i,priv->pshare->phw->VHTTxAgcOffset_A[i]);
			break;
		case rTxAGC_B_CCK11_CCK1_JAguar:
				priv->pshare->phw->CCKTxAgc_B[0] = (char)tmp_byte[0];
				priv->pshare->phw->CCKTxAgc_B[1] = (char)tmp_byte[1];
				priv->pshare->phw->CCKTxAgc_B[2] = (char)tmp_byte[2];
				priv->pshare->phw->CCKTxAgc_B[3] = (char)tmp_byte[3];
				//for(i=0;i<4;i++)
				//	printk("priv->pshare->phw->CCKTxAgc_B[%d]=%x\n",i,priv->pshare->phw->CCKTxAgc_B[i]);
			break;
		case rTxAGC_B_Ofdm18_Ofdm6_JAguar:
				priv->pshare->phw->OFDMTxAgcOffset_B[0] = (char)tmp_byte[0];
				priv->pshare->phw->OFDMTxAgcOffset_B[1] = (char)tmp_byte[1];
				priv->pshare->phw->OFDMTxAgcOffset_B[2] = (char)tmp_byte[2];
				priv->pshare->phw->OFDMTxAgcOffset_B[3] = (char)tmp_byte[3];
				//for(i=0;i<4;i++)
				//	printk("priv->pshare->phw->OFDMTxAgcOffset_B[%d]=%x\n",i,priv->pshare->phw->OFDMTxAgcOffset_B[i]);
			break;
		case rTxAGC_B_Ofdm54_Ofdm24_JAguar:
				priv->pshare->phw->OFDMTxAgcOffset_B[4] = (char)tmp_byte[0];
				priv->pshare->phw->OFDMTxAgcOffset_B[5] = (char)tmp_byte[1];
				priv->pshare->phw->OFDMTxAgcOffset_B[6] = (char)tmp_byte[2];
				priv->pshare->phw->OFDMTxAgcOffset_B[7] = (char)tmp_byte[3];
				//for(i=4;i<8;i++)
				//	printk("priv->pshare->phw->OFDMTxAgcOffset_B[%d]=%x\n",i,priv->pshare->phw->OFDMTxAgcOffset_B[i]);
			break;
		case rTxAGC_B_MCS3_MCS0_JAguar:
				priv->pshare->phw->MCSTxAgcOffset_B[0] = (char)tmp_byte[0];
				priv->pshare->phw->MCSTxAgcOffset_B[1] = (char)tmp_byte[1];
				priv->pshare->phw->MCSTxAgcOffset_B[2] = (char)tmp_byte[2];
				priv->pshare->phw->MCSTxAgcOffset_B[3] = (char)tmp_byte[3];
				//for(i=0;i<4;i++)
				//	printk("priv->pshare->phw->MCSTxAgcOffset_B[%d]=%x\n",i,priv->pshare->phw->MCSTxAgcOffset_B[i]);
			break;
		case rTxAGC_B_MCS7_MCS4_JAguar:
			priv->pshare->phw->MCSTxAgcOffset_B[4] = (char)tmp_byte[0];
			priv->pshare->phw->MCSTxAgcOffset_B[5] = (char)tmp_byte[1];
			priv->pshare->phw->MCSTxAgcOffset_B[6] = (char)tmp_byte[2];
			priv->pshare->phw->MCSTxAgcOffset_B[7] = (char)tmp_byte[3];
			//for(i=4;i<8;i++)
			//	printk("priv->pshare->phw->MCSTxAgcOffset_B[%d]=%x\n",i,priv->pshare->phw->MCSTxAgcOffset_B[i]);
			break;
		case rTxAGC_B_MCS11_MCS8_JAguar:
			priv->pshare->phw->MCSTxAgcOffset_B[8] = (char)tmp_byte[0];
			priv->pshare->phw->MCSTxAgcOffset_B[9] = (char)tmp_byte[1];
			priv->pshare->phw->MCSTxAgcOffset_B[10] = (char)tmp_byte[2];
			priv->pshare->phw->MCSTxAgcOffset_B[11] = (char)tmp_byte[3];
			//for(i=8;i<12;i++)
			//	printk("priv->pshare->phw->MCSTxAgcOffset_B[%d]=%x\n",i,priv->pshare->phw->MCSTxAgcOffset_B[i]);
			break;
		case rTxAGC_B_MCS15_MCS12_JAguar:
			priv->pshare->phw->MCSTxAgcOffset_B[12] = (char)tmp_byte[0];
			priv->pshare->phw->MCSTxAgcOffset_B[13] = (char)tmp_byte[1];
			priv->pshare->phw->MCSTxAgcOffset_B[14] = (char)tmp_byte[2];
			priv->pshare->phw->MCSTxAgcOffset_B[15] = (char)tmp_byte[3];
			//for(i=12;i<16;i++)
			//	printk("priv->pshare->phw->MCSTxAgcOffset_B[%d]=%x\n",i,priv->pshare->phw->MCSTxAgcOffset_B[i]);
			break;
		case rTxAGC_B_Nss1Index3_Nss1Index0_JAguar:
			priv->pshare->phw->VHTTxAgcOffset_B[0] = (char)tmp_byte[0];
			priv->pshare->phw->VHTTxAgcOffset_B[1] = (char)tmp_byte[1];
			priv->pshare->phw->VHTTxAgcOffset_B[2] = (char)tmp_byte[2];
			priv->pshare->phw->VHTTxAgcOffset_B[3] = (char)tmp_byte[3];
			//for(i=0;i<4;i++)
			//	printk("priv->pshare->phw->VHTTxAgcOffset_B[%d]=%x\n",i,priv->pshare->phw->VHTTxAgcOffset_B[i]);
			break;
		case rTxAGC_B_Nss1Index7_Nss1Index4_JAguar:
			priv->pshare->phw->VHTTxAgcOffset_B[4] = (char)tmp_byte[0];
			priv->pshare->phw->VHTTxAgcOffset_B[5] = (char)tmp_byte[1];
			priv->pshare->phw->VHTTxAgcOffset_B[6] = (char)tmp_byte[2];
			priv->pshare->phw->VHTTxAgcOffset_B[7] = (char)tmp_byte[3];
			//for(i=4;i<8;i++)
			//	printk("priv->pshare->phw->VHTTxAgcOffset_B[%d]=%x\n",i,priv->pshare->phw->VHTTxAgcOffset_B[i]);

			break;
		case rTxAGC_B_Nss2Index1_Nss1Index8_JAguar:
			priv->pshare->phw->VHTTxAgcOffset_B[8] = (char)tmp_byte[0];
			priv->pshare->phw->VHTTxAgcOffset_B[9] = (char)tmp_byte[1];
			priv->pshare->phw->VHTTxAgcOffset_B[10] = (char)tmp_byte[2];
			priv->pshare->phw->VHTTxAgcOffset_B[11] = (char)tmp_byte[3];
			//for(i=8;i<12;i++)
			//	printk("priv->pshare->phw->VHTTxAgcOffset_B[%d]=%x\n",i,priv->pshare->phw->VHTTxAgcOffset_B[i]);
			break;
		case rTxAGC_B_Nss2Index5_Nss2Index2_JAguar:
			priv->pshare->phw->VHTTxAgcOffset_B[12] = (char)tmp_byte[0];
			priv->pshare->phw->VHTTxAgcOffset_B[13] = (char)tmp_byte[1];
			priv->pshare->phw->VHTTxAgcOffset_B[14] = (char)tmp_byte[2];
			priv->pshare->phw->VHTTxAgcOffset_B[15] = (char)tmp_byte[3];
			//for(i=12;i<16;i++)
			//	printk("priv->pshare->phw->VHTTxAgcOffset_B[%d]=%x\n",i,priv->pshare->phw->VHTTxAgcOffset_B[i]);
			break;
		case rTxAGC_B_Nss2Index9_Nss2Index6_JAguar:
			priv->pshare->phw->VHTTxAgcOffset_B[16] = (char)tmp_byte[0];
			priv->pshare->phw->VHTTxAgcOffset_B[17] = (char)tmp_byte[1];
			priv->pshare->phw->VHTTxAgcOffset_B[18] = (char)tmp_byte[2];
			priv->pshare->phw->VHTTxAgcOffset_B[19] = (char)tmp_byte[3];
			//for(i=16;i<20;i++)
			//	printk("priv->pshare->phw->VHTTxAgcOffset_B[%d]=%x\n",i,priv->pshare->phw->VHTTxAgcOffset_B[i]);
			break;
	}
}


static int find_str(char *line, char *str)
{
	int len=0, idx=0;
	char *ch = NULL;

	if (!line ||!str || (strlen(str)>strlen(line)))
		return 0;

	ch = line;
	while (1) {
		if (strlen(line) - strlen(str) < (idx+1))
			break;

		if (!memcmp(&(line[idx]), str, strlen(str))) {
			return idx;
			//break;
		}
		idx++;
	}

	return 0;
}

#if 0
static int get_target_val_new(struct rtl8192cd_priv *priv, unsigned char *line_head, unsigned int *u4bRegOffset, unsigned int *u4bMask, unsigned int *u4bRegValue)
{
	int base, idx;
	unsigned char *ch, *next, *pMask, *pValue;
	extern int _atoi(char *s, int base);

	unsigned char *b3, *b2, *b1, *b0;
	unsigned char tmp_byte[4];
	unsigned char lable = 0;		//lable = {0: none, 1: 1Tx, 2: 2Tx}


	*u4bRegOffset = *u4bRegValue = *u4bMask = '\0';

	ch = line_head;

	while (1) {
		if ((*ch == '\0') || (*ch == '\n') || (*ch == '\r'))
			break;
		else if (*ch == '/') {
			*ch = '\0';
			break;
		} else {
			ch++;
		}
	}

	if ((!memcmp(line_head, "0xff", 4)) || (!memcmp(line_head, "0XFF", 4)))
		return 0;

	if ((!memcmp(line_head, "0x", 2)) || (!memcmp(line_head, "0X", 2)))
	{
		base = 16;
		idx = 2;
	}
	else if (!memcmp(line_head, "[1Tx]", 5))
	{
		lable = 1;
		idx = find_str(line_head, "0x");
	}
	else if (!memcmp(line_head, "[2Tx]", 5))
	{
		lable = 2;
		idx = find_str(line_head, "0x");
	}
	else
	{
		return 0;
	}

	if (lable)
	{
		*u4bRegOffset = _atoi((char *)&line_head[idx+2], base);

		next = &(line_head[idx+2]);
		pMask = get_digit(&next);
		if (pMask) {
			//*u4bMask = _atoi((char *)&pMask[2], 16);

			if ((b3 = get_digit_dot(&pMask)) == NULL)
				return 0;
			//panic_printk("\nb3:%s\n", b3);
			if ((b2 = get_digit_dot(&b3)) == NULL)
				return 0;
			//panic_printk("b2:%s\n", b2);
			if ((b1 = get_digit_dot(&b2)) == NULL)
				return 0;
			//panic_printk("b1:%s\n", b1);

			if ((b0 = get_digit_dot(&b1)) == NULL)
				return 0;
			//panic_printk("b0:%s\n", b0);
		}
	}
	else
	{
		*u4bRegOffset = _atoi((char *)&line_head[idx], base);

		b3 = get_digit_dot(&line_head);
		if (b3 == NULL)
			return 0;
		//panic_printk("b3:%s\n", b3);
		b2 = get_digit_dot(&b3);
		if (b2 == NULL)
			return 0;
		//panic_printk("b2:%s\n", b2);
		b1 = get_digit_dot(&b2);
		if (b1 == NULL)
			return 0;
		//panic_printk("b1:%s\n", b1);
		b0 = get_digit_dot(&b1);
		if (b0 == NULL)
			return 0;
		//panic_printk("b0:%s\n", b0);
	}

	base = 10;
	idx = 0;

#if 1
	tmp_byte[3] = _convert_2_pwr_dot(b3, base);
	tmp_byte[2] = _convert_2_pwr_dot(b2, base);
	tmp_byte[1] = _convert_2_pwr_dot(b1, base);
	tmp_byte[0] = _convert_2_pwr_dot(b0, base);
#else
	tmp_byte[3] = _atoi(b3, base);
	tmp_byte[2] = _atoi(b2, base);
	tmp_byte[1] = _atoi(b1, base);
	tmp_byte[0] = _atoi(b0, base);
#endif

	//printk("[%s] 0x%x: %d %d %d %d\n", __FUNCTION__, *u4bRegOffset, tmp_byte[3], tmp_byte[2], tmp_byte[1], tmp_byte[0]);

#if 0
	panic_printk("[0x%x] tmp_byte = %02d-%02d-%02d-%02d\n", *u4bRegOffset,
		tmp_byte[3], tmp_byte[2], tmp_byte[1], tmp_byte[0]);
#endif

	return 1;
}
#endif

static int get_offset_mask_val_new(struct rtl8192cd_priv *priv, unsigned char *line_head, unsigned int *u4bRegOffset, unsigned int *u4bMask, unsigned int *u4bRegValue)
{
	int base, idx, round = 0;
	unsigned char *ch, *next, *pMask, *pValue;
	extern int _atoi(char *s, int base);

	unsigned char *b3, *b2, *b1, *b0;
	unsigned char tmp_byte[4];
	unsigned char lable = 0;		//lable = {0: none, 1: 1Tx, 2: 2Tx}

	*u4bRegOffset = *u4bRegValue = *u4bMask = '\0';

	ch = line_head;

	while (1) {
		if ((*ch == '\0') || (*ch == '\n') || (*ch == '\r'))
			break;
		else if (*ch == '/') {
			*ch = '\0';
			break;
		} else {
			ch++;
		}

		if (++round > 10000) {
			panic_printk("%s[%d] while (1) goes too many\n", __FUNCTION__, __LINE__);
			break;
		}
	}

	if ((!memcmp(line_head, "0xffff", 6)) || (!memcmp(line_head, "0XFFFF", 6))){
		*u4bRegOffset=0xffff;
		return 1;
	}
	if ((!memcmp(line_head, "0xff", 4)) || (!memcmp(line_head, "0XFF", 4)))
		return 0;

	if ((!memcmp(line_head, "0x", 2)) || (!memcmp(line_head, "0X", 2)))
	{
		idx = 2;
	}
	else if (!memcmp(line_head, "[1Tx]", 5))
	{
		lable = 1;
		idx = find_str(line_head, "0x");
	}
	else if (!memcmp(line_head, "[2Tx]", 5))
	{
		lable = 2;
		idx = find_str(line_head, "0x");
	}
	else if (!memcmp(line_head, "[3Tx]", 5))
	{
		lable = 3;
		idx = find_str(line_head, "0x");
	}
	else
	{
		return 0;
	}
	base = 16;

	if (lable) /* For 8814 */
	{
		next = &(line_head[idx+2]);
		pMask = get_digit(&next);
		if (NULL == pMask)
			return 0;
		*u4bRegOffset = _atoi((char *)next, base);

		next = &pMask[2];
		b3 = get_digit(&next);
		*u4bMask = _atoi((char *)next, 16);

		if (b3 == NULL)
			return 0;
		//panic_printk("b3:%s,", b3);

		b2 = get_digit_dot(&b3);
		if (b2 == NULL)
			return 0;
		//panic_printk("b2:%s,", b2);

		b1 = get_digit_dot(&b2);
		if (b1 == NULL)
			return 0;
		//panic_printk("b1:%s,", b1);

		b0 = get_digit_dot(&b1);
		if (b0 == NULL)
			return 0;
		//panic_printk("b0:%s\n", b0);
	}
	else
	{
		*u4bRegOffset = _atoi((char *)&line_head[idx], base);
		if ((GET_CHIP_VER(priv) == VERSION_8188E) || (GET_CHIP_VER(priv) == VERSION_8192E))
		*u4bMask = 0xffffffff;
		else if (GET_CHIP_VER(priv) == VERSION_8812E || GET_CHIP_VER(priv)==VERSION_8881A)
		*u4bMask = 0x7f7f7f7f;

		b3 = get_digit_dot(&line_head);

		if (b3 == NULL)
			return 0;

		b2 = get_digit_dot(&b3);

		if (b2 == NULL)
			return 0;

		b1 = get_digit_dot(&b2);

		if (b1 == NULL)
			return 0;

		b0 = get_digit_dot(&b1);

		if (b0 == NULL)
			return 0;
	}

	base = 10;
	idx = 0;

#if 1
	tmp_byte[3] = _convert_2_pwr_dot(b3, base);
	tmp_byte[2] = _convert_2_pwr_dot(b2, base);
	tmp_byte[1] = _convert_2_pwr_dot(b1, base);
	tmp_byte[0] = _convert_2_pwr_dot(b0, base);
#else
	tmp_byte[3] = _atoi(b3, base);
	tmp_byte[2] = _atoi(b2, base);
	tmp_byte[1] = _atoi(b1, base);
	tmp_byte[0] = _atoi(b0, base);
#endif

	if(GET_CHIP_VER(priv) == VERSION_8197F && !priv->pshare->rf_ft_var.disable_pwr_by_rate)
		set_power_by_rate(priv, u4bRegOffset, tmp_byte);
	if((GET_CHIP_VER(priv) == VERSION_8814A && !priv->pshare->rf_ft_var.disable_pwr_by_rate)||
             (GET_CHIP_VER(priv) == VERSION_8822B && !priv->pshare->rf_ft_var.disable_pwr_by_rate))
		set_power_by_rate_8814(priv, u4bRegOffset, tmp_byte);
#if 0
	panic_printk("[0x%x] tmp_byte = %02d-%02d-%02d-%02d ", *u4bRegOffset,
		tmp_byte[3], tmp_byte[2], tmp_byte[1], tmp_byte[0]);

	panic_printk(" [0x%08x]\n", *u4bRegValue);
#endif

	return 1;
}



static int get_offset_mask_val(unsigned char *line_head, unsigned int *u4bRegOffset, unsigned int *u4bMask, unsigned int *u4bRegValue)
{
	unsigned char *next, *n1;
	int base, idx;
	int num = 0, round = 0;
	unsigned char *ch;
	extern int _atoi(char * s, int base);

	*u4bRegOffset = *u4bRegValue = *u4bMask = '\0';

	ch = line_head;
	while (1) {
		if ((*ch == '\0') || (*ch == '\n') || (*ch == '\r'))
			break;
		else if (*ch == '/') {
			*ch = '\0';
			break;
		} else {
			ch++;
		}

		if (++round > 10000) {
			panic_printk("%s[%d] while (1) goes too many\n", __FUNCTION__, __LINE__);
			break;
		}
	}

	next = get_digit(&line_head);
	if (next == NULL)
		return num;
	num++;
	if ((!memcmp(line_head, "0x", 2)) || (!memcmp(line_head, "0X", 2))) {
		base = 16;
		idx = 2;
	} else {
		base = 10;
		idx = 0;
	}
	*u4bRegOffset = _atoi((char *)&line_head[idx], base);

	if (next) {
		n1 = get_digit(&next);
		if (n1 == NULL)
			return num;
		num++;
		if ((!memcmp(next, "0x", 2)) || (!memcmp(next, "0X", 2))) {
			base = 16;
			idx = 2;
		} else {
			base = 10;
			idx = 0;
		}
		*u4bMask = _atoi((char *)&next[idx], base);

		if (n1) {
			if (!get_digit(&n1))
				return num;
			num++;
			if ((!memcmp(n1, "0x", 2)) || (!memcmp(n1, "0X", 2))) {
				base = 16;
				idx = 2;
			} else {
				base = 10;
				idx = 0;
			}
			*u4bRegValue = _atoi((char *)&n1[idx], base);
		} else
			*u4bRegValue = 0;
	} else
		*u4bMask = 0;

	return num;
}


unsigned char *get_line(unsigned char **line)
{
	unsigned char *p = *line;

	while (*p && ((*p == '\n') || (*p == '\r')))
		p++;

	if (*p == '\0') {
		*line = NULL;
		return NULL;
	}
	*line = p;

	while (*p && (*p != '\n') && (*p != '\r'))
		p++;

	*p = '\0';
	return p + 1;
}

int ch2idx(int ch)
{
	int val = -1;
	// |1~14|36, 38, 40, ..., 64|100, 102, ..., 140|149, 151, ..., 165|
	if (ch <= 14)
		val = ch - 1;
	else if (ch <= 64)
		val = ((ch - 36) >> 1) + 14;
	else if (ch <= 140)
		val = ((ch - 100) >> 1) + 29;
	else if (ch <= 165)
		val = ((ch - 149) >> 1) + 50;
	else if (ch <= 177)
		val = ((ch - 169) >> 1) + 59;

	return val;
}

#ifdef TXPWR_LMT
#ifdef TXPWR_LMT_NEWFILE
void find_pwr_limit_new(struct rtl8192cd_priv *priv, int channel, int offset)
{
    int chidx_20 = 0,  chidx_40 = 0, chidx_80 = 0;
    int working_channel_40m = 0;
    int working_channel_80m = get_center_channel(priv, channel, offset,1);

    working_channel_40m = (priv->pshare->CurrentChannelBW ? ((offset==1) ? (channel-2) : (channel+2)) : channel);

    chidx_20 = ch2idx(channel);
    chidx_40 = ch2idx(working_channel_40m);
    chidx_80 = ch2idx(working_channel_80m);

#if 0
    panic_printk("[BW%dM]: central channel %d %d %d ==> index %d %d %d:\n",
        20*(1 << priv->pshare->CurrentChannelBW),
        channel, working_channel_40m, working_channel_80m,
        chidx_20, chidx_40, chidx_80);
#endif

	if ((chidx_20>=0) && (chidx_40>=0) && (chidx_80>=0)) {
			priv->pshare->txpwr_lmt_CCK = priv->pshare->ch_pwr_lmtCCK[chidx_20];
			priv->pshare->txpwr_lmt_OFDM = priv->pshare->ch_pwr_lmtOFDM[chidx_20];

		if (priv->pshare->CurrentChannelBW == 2){ //BW=80M
			priv->pshare->txpwr_lmt_HT1S = priv->pshare->ch_pwr_lmtHT40_1S[chidx_40];
			priv->pshare->txpwr_lmt_HT2S = priv->pshare->ch_pwr_lmtHT40_2S[chidx_40];
			priv->pshare->txpwr_lmt_VHT1S = priv->pshare->ch_pwr_lmtVHT80_1S[chidx_80];
			priv->pshare->txpwr_lmt_VHT2S = priv->pshare->ch_pwr_lmtVHT80_2S[chidx_80];
#ifdef BEAMFORMING_AUTO
			priv->pshare->txpwr_lmt_TXBF_HT1S = priv->pshare->ch_pwr_lmtHT40_TXBF_1S[chidx_40];
			priv->pshare->txpwr_lmt_TXBF_HT2S = priv->pshare->ch_pwr_lmtHT40_TXBF_2S[chidx_40];
			priv->pshare->txpwr_lmt_TXBF_VHT1S = priv->pshare->ch_pwr_lmtVHT80_TXBF_1S[chidx_80];
			priv->pshare->txpwr_lmt_TXBF_VHT2S = priv->pshare->ch_pwr_lmtVHT80_TXBF_2S[chidx_80];
#endif // BEAMFORMING_AUTO
		} else if (priv->pshare->CurrentChannelBW == 1){ //BW=40M
			priv->pshare->txpwr_lmt_HT1S = priv->pshare->ch_pwr_lmtHT40_1S[chidx_40];
			priv->pshare->txpwr_lmt_HT2S = priv->pshare->ch_pwr_lmtHT40_2S[chidx_40];
			priv->pshare->txpwr_lmt_VHT1S = priv->pshare->ch_pwr_lmtHT40_1S[chidx_40];
			priv->pshare->txpwr_lmt_VHT2S = priv->pshare->ch_pwr_lmtHT40_2S[chidx_40];
#ifdef BEAMFORMING_AUTO
			priv->pshare->txpwr_lmt_TXBF_HT1S = priv->pshare->ch_pwr_lmtHT40_TXBF_1S[chidx_40];
			priv->pshare->txpwr_lmt_TXBF_HT2S = priv->pshare->ch_pwr_lmtHT40_TXBF_2S[chidx_40];
			priv->pshare->txpwr_lmt_TXBF_VHT1S = priv->pshare->ch_pwr_lmtHT40_TXBF_1S[chidx_40];
			priv->pshare->txpwr_lmt_TXBF_VHT2S = priv->pshare->ch_pwr_lmtHT40_TXBF_2S[chidx_40];
#endif // BEAMFORMING_AUTO
		} else { //BW=20M
			priv->pshare->txpwr_lmt_HT1S = priv->pshare->ch_pwr_lmtHT20_1S[chidx_20];
			priv->pshare->txpwr_lmt_HT2S = priv->pshare->ch_pwr_lmtHT20_2S[chidx_20];
			priv->pshare->txpwr_lmt_VHT1S = priv->pshare->ch_pwr_lmtHT20_1S[chidx_20];
			priv->pshare->txpwr_lmt_VHT2S = priv->pshare->ch_pwr_lmtHT20_2S[chidx_20];
#ifdef BEAMFORMING_AUTO
			priv->pshare->txpwr_lmt_TXBF_HT1S = priv->pshare->ch_pwr_lmtHT20_TXBF_1S[chidx_20];
			priv->pshare->txpwr_lmt_TXBF_HT2S = priv->pshare->ch_pwr_lmtHT20_TXBF_2S[chidx_20];
			priv->pshare->txpwr_lmt_TXBF_VHT1S = priv->pshare->ch_pwr_lmtHT20_TXBF_1S[chidx_20];
			priv->pshare->txpwr_lmt_TXBF_VHT2S = priv->pshare->ch_pwr_lmtHT20_TXBF_2S[chidx_20];

#endif // BEAMFORMING_AUTO
		}

        if(GET_CHIP_VER(priv) != VERSION_8197F) /* 97F doesn't need to do this, already handled by power lmt table*/
        {
            if (priv->pmib->dot11RFEntry.tx2path && !priv->pshare->rf_ft_var.disable_txpwrlmt2path) {
                if (priv->pshare->txpwr_lmt_CCK)
                    priv->pshare->txpwr_lmt_CCK -= 6;
                if (priv->pshare->txpwr_lmt_OFDM)
                    priv->pshare->txpwr_lmt_OFDM -= 6;
                if (priv->pshare->txpwr_lmt_HT1S)
                    priv->pshare->txpwr_lmt_HT1S -= 6;
                if (priv->pshare->txpwr_lmt_VHT1S)
                    priv->pshare->txpwr_lmt_VHT1S -= 6;
            }
        }
    } else {
        priv->pshare->txpwr_lmt_CCK = 0;
        priv->pshare->txpwr_lmt_OFDM = 0;
        priv->pshare->txpwr_lmt_HT1S = 0;
        priv->pshare->txpwr_lmt_HT2S = 0;
        priv->pshare->txpwr_lmt_VHT1S = 0;
        priv->pshare->txpwr_lmt_VHT2S = 0;
	if(channel <= 165)
        printk("Cannot map current working channel (%d) to find power limit!\n", channel);
    }

}

#endif

void find_pwr_limit(struct rtl8192cd_priv *priv, int channel, int offset)
{
	int chidx_20 = 0,  chidx_40 = 0;
	int working_channel_40m = get_center_channel(priv, channel, offset,1);

	chidx_20 = ch2idx(channel);
	chidx_40 = ch2idx(working_channel_40m);

#if 0
	panic_printk("[BW%dM]: central channel[ %d %d ]==> index[ %d %d %d ]\n",
			20*(1 << priv->pshare->CurrentChannelBW),
			channel, working_channel_40m,
			chidx_20, chidx_40);
#endif

	if ((chidx_20 >= 0) && (chidx_40 >= 0)) {
		priv->pshare->txpwr_lmt_CCK = priv->pshare->ch_pwr_lmtCCK[chidx_20];
		priv->pshare->txpwr_lmt_OFDM = priv->pshare->ch_pwr_lmtOFDM[chidx_20];
		priv->pshare->tgpwr_CCK = priv->pshare->ch_tgpwr_CCK[chidx_20];
		priv->pshare->tgpwr_OFDM = priv->pshare->ch_tgpwr_OFDM[chidx_20];

		if (priv->pshare->CurrentChannelBW) {
			priv->pshare->txpwr_lmt_HT1S = priv->pshare->ch_pwr_lmtHT40_1S[chidx_40];
			priv->pshare->txpwr_lmt_HT2S = priv->pshare->ch_pwr_lmtHT40_2S[chidx_40];
			priv->pshare->tgpwr_HT1S = priv->pshare->ch_tgpwr_HT40_1S[chidx_40];
			priv->pshare->tgpwr_HT2S = priv->pshare->ch_tgpwr_HT40_2S[chidx_40];
		} else { //if 20M bw, tmp == tmp2 ??
			priv->pshare->txpwr_lmt_HT1S = priv->pshare->ch_pwr_lmtHT20_1S[chidx_20];
			priv->pshare->txpwr_lmt_HT2S = priv->pshare->ch_pwr_lmtHT20_2S[chidx_20];
			priv->pshare->tgpwr_HT1S = priv->pshare->ch_tgpwr_HT20_1S[chidx_20];
			priv->pshare->tgpwr_HT2S = priv->pshare->ch_tgpwr_HT20_2S[chidx_20];
		}
	} else {
		priv->pshare->txpwr_lmt_CCK = 0;
		priv->pshare->txpwr_lmt_OFDM = 0;
		priv->pshare->txpwr_lmt_HT1S = 0;
		priv->pshare->txpwr_lmt_HT2S = 0;

		if(channel <= 165)
		printk("Cannot map current working channel (%d) to find power limit!\n", channel);
	}

	//printk("txpwr_lmt_OFDM %d tgpwr_OFDM %d\n", priv->pshare->txpwr_lmt_OFDM, priv->pshare->tgpwr_OFDM);

}

#ifdef TXPWR_LMT_NEWFILE

#define	BAND2G_20M_1T_CCK 		1
#define	BAND2G_20M_1T_OFDM 		2
#define	BAND2G_20M_1T_HT 		3
#define	BAND2G_20M_2T_HT 		4
#define	BAND2G_20M_3T_HT 		5
#define	BAND2G_20M_4T_HT 		6
#define	BAND2G_40M_1T_HT		7
#define	BAND2G_40M_2T_HT		8
#define	BAND2G_40M_3T_HT		9
#define	BAND2G_40M_4T_HT		10

#define	BAND5G_20M_1T_OFDM 		11
#define	BAND5G_20M_1T_HT 		12
#define	BAND5G_20M_2T_HT 		13
#define	BAND5G_20M_3T_HT 		14
#define	BAND5G_20M_4T_HT 		15
#define	BAND5G_40M_1T_HT		16
#define	BAND5G_40M_2T_HT		17
#define	BAND5G_40M_3T_HT		18
#define	BAND5G_40M_4T_HT		19
#define	BAND5G_80M_1T_VHT		20
#define	BAND5G_80M_2T_VHT		21
#define	BAND5G_80M_3T_VHT		22
#define	BAND5G_80M_4T_VHT		23

int PHY_ConfigTXLmtWithParaFile_new(struct rtl8192cd_priv *priv)
{
    int read_bytes, found, num, len=0, round;
    unsigned int  channel, limit, table_idx = 0;
    unsigned char *mem_ptr, *line_head, *next_head;
    const unsigned char * table_index;
    const unsigned char table_index_old[24]={0,1,2,3,4,14,15,5,6,16,17,7,8,9,18,19,10,11,20,21,12,13,22,23};
    const unsigned char table_index_new[24]={0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23};
    struct TxPwrLmtTable_new *reg_table;

    unsigned int tmp =0;

    if(GET_CHIP_VER(priv) <= VERSION_8881A || GET_CHIP_VER(priv) == VERSION_8723B){/* for old ic format*/
        table_index = table_index_old;
    }
    else { /*new ic format*/
        table_index = table_index_new;
    }

    priv->pshare->txpwr_lmt_CCK = 0;
    priv->pshare->txpwr_lmt_OFDM = 0;
    priv->pshare->txpwr_lmt_HT1S = 0;
    priv->pshare->txpwr_lmt_HT2S = 0;
    priv->pshare->txpwr_lmt_VHT1S = 0;
    priv->pshare->txpwr_lmt_VHT2S = 0;

	reg_table = (struct TxPwrLmtTable *)priv->pshare->txpwr_lmt_buf;

	if((mem_ptr = (unsigned char *)kmalloc(MAX_CONFIG_FILE_SIZE, GFP_ATOMIC)) == NULL) {
		printk("PHY_ConfigMACWithParaFile(): not enough memory\n");
		return -1;
	}

	DEBUG_INFO("regdomain=%d\n",priv->pmib->dot11StationConfigEntry.dot11RegDomain);

	memset(mem_ptr, 0, MAX_CONFIG_FILE_SIZE); // clear memory

	if ( IS_HAL_CHIP(priv) ) {
#ifdef PWR_BY_RATE_92E_HP
#ifdef HIGH_POWER_EXT_PA
		if (priv->pshare->rf_ft_var.use_ext_pa && GET_CHIP_VER(priv) == VERSION_8192E) {
			panic_printk("[%s][TXPWR_LMT_92EE_hp]\n", __FUNCTION__);
			GET_HAL_INTERFACE(priv)->GetHwRegHandler(priv, HW_VAR_POWERLIMITFILE_HP_SIZE, (pu1Byte)&read_bytes);
			GET_HAL_INTERFACE(priv)->GetHwRegHandler(priv, HW_VAR_POWERLIMITFILE_HP_START, (pu1Byte)&next_head);
		} else
#endif
#endif
		{
			GET_HAL_INTERFACE(priv)->GetHwRegHandler(priv, HW_VAR_POWERLIMITFILE_SIZE, (pu1Byte)&read_bytes);
			GET_HAL_INTERFACE(priv)->GetHwRegHandler(priv, HW_VAR_POWERLIMITFILE_START, (pu1Byte)&next_head);
		}
	} else
	{
		printk("This WiFi IC not support Tx Power limit !!\n");
		kfree(mem_ptr);
		return -1;
	}

	memcpy(mem_ptr, next_head, read_bytes);

	next_head = mem_ptr;
	round = 0;
	while (1) {
		line_head = next_head;
		next_head = get_line(&line_head);

		if (line_head == NULL)
			break;

		if (((line_head[0] == '/') || (line_head[0] == '#')) && (line_head[2] != 'T')) /* get string "//Table 1:" */  //OSK: change "/" to "#"
			continue;

		found = get_chnl_lmt_dot_new(priv, line_head, &channel, &limit, &table_idx);

		if (found > 0) {
			reg_table[len].channel = channel;
			reg_table[len].limit = limit;
			reg_table[len].table_idx = table_idx;

			len++;
			if ((len * sizeof(struct TxPwrLmtTable_new)) > MAC_REG_SIZE)
				break;
		}

		if (++round > 10000) {
			panic_printk("%s[%d] while (1) goes too many\n", __FUNCTION__, __LINE__);
			break;
		}
	}

	kfree(mem_ptr);

	if ((len * sizeof(struct TxPwrLmtTable_new)) > MAC_REG_SIZE) {
		printk("TXPWR_LMT table buffer not large enough!\n");
		return -1;
	}

	num = 0;

	memset(priv->pshare->ch_pwr_lmtOFDM, 0, SUPPORT_CH_NUM);
	memset(priv->pshare->ch_pwr_lmtHT20_1S, 0, SUPPORT_CH_NUM);
	memset(priv->pshare->ch_pwr_lmtHT20_2S, 0, SUPPORT_CH_NUM);
	memset(priv->pshare->ch_pwr_lmtHT40_1S, 0, SUPPORT_CH_NUM);
	memset(priv->pshare->ch_pwr_lmtHT40_2S, 0, SUPPORT_CH_NUM);
	memset(priv->pshare->ch_pwr_lmtVHT80_1S, 0, SUPPORT_CH_NUM);
	memset(priv->pshare->ch_pwr_lmtVHT80_2S, 0, SUPPORT_CH_NUM);


	round = 0;
	while (1) {
		channel = reg_table[num].channel;
		limit = reg_table[num].limit;
		table_idx = reg_table[num].table_idx;

		if(channel > 0)
		{
			//printk(">> [%02d]-%03d-%02d\n", table_idx, channel , limit);


				int j = ch2idx(channel);

				if (table_idx == table_index[BAND2G_20M_1T_CCK]){
					priv->pshare->ch_pwr_lmtCCK[j] = limit;
				}else if (table_idx == table_index[BAND2G_20M_1T_OFDM]){
					priv->pshare->ch_pwr_lmtOFDM[j] = limit;
				}else if (table_idx == table_index[BAND2G_20M_1T_HT]){
					priv->pshare->ch_pwr_lmtHT20_1S[j] = limit;
				}else if (table_idx == table_index[BAND2G_20M_2T_HT]){
					priv->pshare->ch_pwr_lmtHT20_2S[j] = limit;
				}else if (table_idx == table_index[BAND2G_40M_1T_HT]){
					priv->pshare->ch_pwr_lmtHT40_1S[j] = limit;
				}else if (table_idx == table_index[BAND2G_40M_2T_HT]){
					priv->pshare->ch_pwr_lmtHT40_2S[j] = limit;
				}
				else if (table_idx == table_index[BAND5G_20M_1T_OFDM]){
					priv->pshare->ch_pwr_lmtOFDM[j] = limit;
				}else if (table_idx == table_index[BAND5G_20M_1T_HT]){
					priv->pshare->ch_pwr_lmtHT20_1S[j] = limit;
				}else if (table_idx == table_index[BAND5G_20M_2T_HT]){
					priv->pshare->ch_pwr_lmtHT20_2S[j] = limit;
				}else if (table_idx == table_index[BAND5G_40M_1T_HT]){
					priv->pshare->ch_pwr_lmtHT40_1S[j] = limit;
				}else if (table_idx == table_index[BAND5G_40M_2T_HT]){
					priv->pshare->ch_pwr_lmtHT40_2S[j] = limit;
				}else if (table_idx == table_index[BAND5G_80M_1T_VHT]){
					priv->pshare->ch_pwr_lmtVHT80_1S[j] = limit;
				}else if (table_idx == table_index[BAND5G_80M_2T_VHT]){
					priv->pshare->ch_pwr_lmtVHT80_2S[j] = limit;
				}


		}

		num++;

		if(num >= len)
			break;

		if (++round > 10000) {
			panic_printk("%s[%d] while (1) goes too many\n", __FUNCTION__, __LINE__);
			break;
		}
	}

	return 0;
}
#ifdef BEAMFORMING_AUTO
void BEAMFORMING_TXPWRLMT_Auto(struct rtl8192cd_priv *priv)
{
	unsigned int maxDelta;
	if(priv->pmib->dot11RFEntry.txbf == 1) {
		if (!priv->pshare->rf_ft_var.disable_txpwrlmt) {
			priv->pshare->txbf2TXbackoff = 0;
			#ifdef RTK_AC_SUPPORT //for 11ac logo
		    if (priv->pmib->dot11BssType.net_work_type & WIRELESS_11AC) {
				// For VHT STA
				maxDelta = (priv->pshare->txpwr_lmt_VHT2S - priv->pshare->txpwr_lmt_TXBF_VHT2S);
				maxDelta = max(maxDelta,(priv->pshare->txpwr_lmt_VHT1S - priv->pshare->txpwr_lmt_TXBF_VHT1S));
				if(maxDelta > 4)
				{
					priv->pshare->txbferVHT2TX = 0;
				} else
				{
					priv->pshare->txbferVHT2TX = 1;
					priv->pshare->txbf2TXbackoff = maxDelta;
				}
			}
			#endif
			// For HT STA

			maxDelta = (priv->pshare->txpwr_lmt_HT2S - priv->pshare->txpwr_lmt_TXBF_HT2S);
			maxDelta = max(maxDelta,(priv->pshare->txpwr_lmt_HT1S - priv->pshare->txpwr_lmt_TXBF_HT1S));
			if(maxDelta > 4)
			{
				priv->pshare->txbferHT2TX= 0;
			} else
			{
				priv->pshare->txbferHT2TX= 1;
				priv->pshare->txbf2TXbackoff = max(priv->pshare->txbf2TXbackoff,maxDelta);

			}
		}else {
		}
	}

}

int PHY_ConfigTXLmtWithParaFile_new_TXBF(struct rtl8192cd_priv *priv)
{
	int read_bytes, found, num, len=0, round;
	unsigned int  channel, limit, table_idx = 0;
	unsigned char *mem_ptr, *line_head, *next_head;
	int	tbl_idx[13], set_en=0, type=-1;
	struct TxPwrLmtTable_new *reg_table;

	unsigned int tmp =0;


	priv->pshare->txpwr_lmt_TXBF_HT1S = 0;
	priv->pshare->txpwr_lmt_TXBF_HT2S = 0;
	priv->pshare->txpwr_lmt_TXBF_VHT1S = 0;
	priv->pshare->txpwr_lmt_TXBF_VHT2S = 0;

	reg_table = (struct TxPwrLmtTable *)priv->pshare->txpwr_lmt_buf;

	if((mem_ptr = (unsigned char *)kmalloc(MAX_CONFIG_FILE_SIZE, GFP_ATOMIC)) == NULL) {
		printk("PHY_ConfigMACWithParaFile(): not enough memory\n");
		return -1;
	}

	DEBUG_INFO("regdomain=%d\n",priv->pmib->dot11StationConfigEntry.dot11RegDomain);

	memset(mem_ptr, 0, MAX_CONFIG_FILE_SIZE); // clear memory

	if ( IS_HAL_CHIP(priv) ) {
		GET_HAL_INTERFACE(priv)->GetHwRegHandler(priv, HW_VAR_POWERLIMITFILE_TXBF_SIZE, (pu1Byte)&read_bytes);
		GET_HAL_INTERFACE(priv)->GetHwRegHandler(priv, HW_VAR_POWERLIMITFILE_TXBF_START, (pu1Byte)&next_head);
	} else
	{
		printk("This WiFi IC not support Tx Power limit (TXBF)!!\n");
		kfree(mem_ptr);
		return -1;
	}

	memcpy(mem_ptr, next_head, read_bytes);

	next_head = mem_ptr;
	round = 0;
	while (1) {
		line_head = next_head;
		next_head = get_line(&line_head);

		if (line_head == NULL)
			break;

		if ((line_head[0] == '/') && (line_head[2] != 'T')) /* get string "//Table 1:" */
			continue;

		found = get_chnl_lmt_dot_new(priv, line_head, &channel, &limit, &table_idx);

		if (found > 0) {
			reg_table[len].channel = channel;
			reg_table[len].limit = limit;
			reg_table[len].table_idx = table_idx;

			len++;
			if ((len * sizeof(struct TxPwrLmtTable_new)) > MAC_REG_SIZE)
				break;
		}

		if (++round > 10000) {
			panic_printk("%s[%d] while (1) goes too many\n", __FUNCTION__, __LINE__);
			break;
		}
	}

	kfree(mem_ptr);

	if ((len * sizeof(struct TxPwrLmtTable_new)) > MAC_REG_SIZE) {
		printk("TXPWR_LMT (TXBF) table buffer not large enough!\n");
		return -1;
	}

	num = 0;

	memset(priv->pshare->ch_pwr_lmtHT20_TXBF_1S, 0, SUPPORT_CH_NUM);
	memset(priv->pshare->ch_pwr_lmtHT20_TXBF_2S, 0, SUPPORT_CH_NUM);
	memset(priv->pshare->ch_pwr_lmtHT40_TXBF_1S, 0, SUPPORT_CH_NUM);
	memset(priv->pshare->ch_pwr_lmtHT40_TXBF_2S, 0, SUPPORT_CH_NUM);
	memset(priv->pshare->ch_pwr_lmtVHT80_TXBF_1S, 0, SUPPORT_CH_NUM);
	memset(priv->pshare->ch_pwr_lmtVHT80_TXBF_2S, 0, SUPPORT_CH_NUM);


	round = 0;
	while (1) {
		channel = reg_table[num].channel;
		limit = reg_table[num].limit;
		table_idx = reg_table[num].table_idx;

		if(channel > 0)
		{
			//printk(">> [%02d]-%03d-%02d\n", table_idx, channel , limit);

			if(priv->pmib->dot11RFEntry.phyBandSelect == PHY_BAND_2G)
			if ((table_idx >= BAND2G_20M_1T_CCK) && (table_idx <= BAND2G_40M_4T_HT)){

				int j = ch2idx(channel);

				if (table_idx == BAND2G_20M_1T_HT){
					priv->pshare->ch_pwr_lmtHT20_TXBF_1S[j] = limit;
				}else if (table_idx == BAND2G_20M_2T_HT){
					priv->pshare->ch_pwr_lmtHT20_TXBF_2S[j] = limit;
				}else if (table_idx == BAND2G_40M_1T_HT){
					priv->pshare->ch_pwr_lmtHT40_TXBF_1S[j] = limit;
				}else if (table_idx == BAND2G_40M_2T_HT){
					priv->pshare->ch_pwr_lmtHT40_TXBF_2S[j] = limit;
				}

			}

			if(priv->pmib->dot11RFEntry.phyBandSelect == PHY_BAND_5G)
			if ( (table_idx >= BAND5G_20M_1T_OFDM) && (table_idx <= BAND5G_80M_4T_VHT)){

				int j = ch2idx(channel);

				if (table_idx == BAND5G_20M_1T_HT){
					priv->pshare->ch_pwr_lmtHT20_TXBF_1S[j] = limit;
				}else if (table_idx == BAND5G_20M_2T_HT){
					priv->pshare->ch_pwr_lmtHT20_TXBF_2S[j] = limit;
				}else if (table_idx == BAND5G_40M_1T_HT){
					priv->pshare->ch_pwr_lmtHT40_TXBF_1S[j] = limit;
				}else if (table_idx == BAND5G_40M_2T_HT){
					priv->pshare->ch_pwr_lmtHT40_TXBF_2S[j] = limit;
				}else if (table_idx == BAND5G_80M_1T_VHT){
					priv->pshare->ch_pwr_lmtVHT80_TXBF_1S[j] = limit;
				}else if (table_idx == BAND5G_80M_2T_VHT){
					priv->pshare->ch_pwr_lmtVHT80_TXBF_2S[j] = limit;
				}
			}

		}

		num++;

		if(num >= len)
			break;

		if (++round > 10000) {
			panic_printk("%s[%d] while (1) goes too many\n", __FUNCTION__, __LINE__);
			break;
		}
	}

	return 0;
}
#endif
#endif

int PHY_ConfigTXLmtWithParaFile(struct rtl8192cd_priv *priv)
{
	int read_bytes, num, len = 0, round;
	unsigned int  ch_start, ch_end, limit, target = 0;
	unsigned char *mem_ptr, *line_head, *next_head;
	int	tbl_idx[6], set_en = 0, type = -1;
	struct TxPwrLmtTable *reg_table;

	priv->pshare->txpwr_lmt_CCK = 0;
	priv->pshare->txpwr_lmt_OFDM = 0;
	priv->pshare->txpwr_lmt_HT1S = 0;
	priv->pshare->txpwr_lmt_HT2S = 0;

	reg_table = (struct TxPwrLmtTable *)priv->pshare->txpwr_lmt_buf;

	if ((GET_CHIP_VER(priv) != VERSION_8192D) && (GET_CHIP_VER(priv) != VERSION_8192C) && (GET_CHIP_VER(priv) != VERSION_8188C)) {
		printk("[%s]NOT support! TXPWR_LMT is for RTL8192D & 92C/88C ONLY!\n", __FUNCTION__);
		return -1;
	}

	if ((mem_ptr = (unsigned char *)kmalloc(MAX_CONFIG_FILE_SIZE, GFP_ATOMIC)) == NULL) {
		printk("PHY_ConfigTXLmtWithParaFile(): not enough memory\n");
		return -1;
	}


	tbl_idx[0] = 1;
	tbl_idx[1] = 2;
	tbl_idx[2] = 3;
	tbl_idx[3] = 4;
	tbl_idx[4] = 5;
	tbl_idx[5] = 6;


	DEBUG_INFO("regdomain=%d tbl_idx=%d,%d\n", priv->pmib->dot11StationConfigEntry.dot11RegDomain, tbl_idx[0], tbl_idx[1]);


	memset(mem_ptr, 0, MAX_CONFIG_FILE_SIZE); // clear memory

	DEBUG_INFO("[%s][TXPWR_LMT]\n", __FUNCTION__);




	memcpy(mem_ptr, next_head, read_bytes);

	next_head = mem_ptr;
	round = 0;
	while (1) {
		line_head = next_head;
		next_head = get_line(&line_head);

		if (line_head == NULL)
			break;

		if (line_head[0] == '/')
			continue;

		num = get_chnl_lmt_dot(line_head, &ch_start, &ch_end, &limit, &target);

		if (num > 0) {
			reg_table[len].start = ch_start;
			reg_table[len].end = ch_end;
			reg_table[len].limit = limit;
			reg_table[len].target = target;

			len++;
			if ((len * sizeof(struct TxPwrLmtTable)) > MAC_REG_SIZE)
				break;
		}

		if (++round > 10000) {
			panic_printk("%s[%d] while (1) goes too many\n", __FUNCTION__, __LINE__);
			break;
		}
	}

	reg_table[len].start = 0xff;

	kfree(mem_ptr);

	if ((len * sizeof(struct TxPwrLmtTable)) > MAC_REG_SIZE) {
		printk("TXPWR_LMT table buffer not large enough!\n");
		return -1;
	}

	num = 0;
	round = 0;
	while (1) {
		ch_start = reg_table[num].start;
		ch_end = reg_table[num].end;
		limit = reg_table[num].limit;
		target = reg_table[num].target;

		//printk(">> %d-%d-%d-%d\n",ch_start,ch_end,limit,target);
		if (ch_start == 0xff)
			break;

		if (ch_start == 0 && ch_end == 0) {
			if (limit == tbl_idx[0]) {
				set_en = 1;
				type = 0;
				memset(priv->pshare->ch_pwr_lmtCCK, 0, SUPPORT_CH_NUM);
				memset(priv->pshare->ch_tgpwr_CCK, 0, SUPPORT_CH_NUM);
			} else if (limit == tbl_idx[1]) {
				set_en = 1;
				type = 1;
				memset(priv->pshare->ch_pwr_lmtOFDM, 0, SUPPORT_CH_NUM);
				memset(priv->pshare->ch_tgpwr_OFDM, 0, SUPPORT_CH_NUM);
			} else if (limit == tbl_idx[2]) {
				set_en = 1;
				type = 2;
				memset(priv->pshare->ch_pwr_lmtHT20_1S, 0, SUPPORT_CH_NUM);
				memset(priv->pshare->ch_tgpwr_HT20_1S, 0, SUPPORT_CH_NUM);
			} else if (limit == tbl_idx[3]) {
				set_en = 1;
				type = 3;
				memset(priv->pshare->ch_pwr_lmtHT20_2S, 0, SUPPORT_CH_NUM);
				memset(priv->pshare->ch_tgpwr_HT20_2S, 0, SUPPORT_CH_NUM);
			} else if (limit == tbl_idx[4]) {
				set_en = 1;
				type = 4;
				memset(priv->pshare->ch_pwr_lmtHT40_1S, 0, SUPPORT_CH_NUM);
				memset(priv->pshare->ch_tgpwr_HT40_1S, 0, SUPPORT_CH_NUM);
			} else if (limit == tbl_idx[5]) {
				set_en = 1;
				type = 5;
				memset(priv->pshare->ch_pwr_lmtHT40_2S, 0, SUPPORT_CH_NUM);
				memset(priv->pshare->ch_tgpwr_HT40_2S, 0, SUPPORT_CH_NUM);
			} else {
				set_en = 0;
			}
		}

		if (set_en && ch_start) {
			int j;
			for (j = ch2idx(ch_start); j <= ch2idx(ch_end); j++) {
				if (j < 0 || j >= SUPPORT_CH_NUM) {
					panic_printk("channel out of bound!!\n");
					break;
				}

				if (type == 0) {
					priv->pshare->ch_pwr_lmtCCK[j] = limit;
					priv->pshare->ch_tgpwr_CCK[j] = target;
				} else if (type == 1) {
					priv->pshare->ch_pwr_lmtOFDM[j] = limit;
					priv->pshare->ch_tgpwr_OFDM[j] = target;
				} else if (type == 2) {
					priv->pshare->ch_pwr_lmtHT20_1S[j] = limit;
					priv->pshare->ch_tgpwr_HT20_1S[j] = target;
				} else if (type == 3) {
					priv->pshare->ch_pwr_lmtHT20_2S[j] = limit;
					priv->pshare->ch_tgpwr_HT20_2S[j] = target;
				} else if (type == 4) {
					priv->pshare->ch_pwr_lmtHT40_1S[j] = limit;
					priv->pshare->ch_tgpwr_HT40_1S[j] = target;
				} else if (type == 5) {
					priv->pshare->ch_pwr_lmtHT40_2S[j] = limit;
					priv->pshare->ch_tgpwr_HT40_2S[j] = target;
				}
			}
		}

		num++;

		if (++round > 10000) {
			panic_printk("%s[%d] while (1) goes too many\n", __FUNCTION__, __LINE__);
			break;
		}
	}

	return 0;
}

#endif


#ifdef _TRACKING_TABLE_FILE

static char TXPWR_TRACKING_NAME[][32] = {
	"2GCCKA_P",
	"2GCCKA_N",
	"2GCCKB_P",
	"2GCCKB_N",
	"2GA_P",
	"2GA_N",
	"2GB_P",
	"2GB_N",
	"5GLA_P",
	"5GLA_N",
	"5GLB_P",
	"5GLB_N",
	"5GMA_P",
	"5GMA_N",
	"5GMB_P",
	"5GMB_N",
	"5GHA_P",
	"5GHA_N",
	"5GHB_P",
	"5GHB_N",
};

static char TXPWR_TRACKING_NAME_NEW[][32] = {
	"[2G][A][+][CCK]",
	"[2G][A][-][CCK]",
	"[2G][B][+][CCK]",
	"[2G][B][-][CCK]",
	"[2G][A][+][ALL]",
	"[2G][A][-][ALL]",
	"[2G][B][+][ALL]",
	"[2G][B][-][ALL]",
	"[5G][A][+][ALL][0]",
	"[5G][A][-][ALL][0]",
	"[5G][B][+][ALL][0]",
	"[5G][B][-][ALL][0]",
	"[5G][A][+][ALL][1]",
	"[5G][A][-][ALL][1]",
	"[5G][B][+][ALL][1]",
	"[5G][B][-][ALL][1]",
	"[5G][A][+][ALL][2]",
	"[5G][A][-][ALL][2]",
	"[5G][B][+][ALL][2]",
	"[5G][B][-][ALL][2]",
};

//#ifndef TXPWR_LMT

/*
     Convert ascii value(*s) to integer
 */
int _convert_2_pwr_tracking(char *s, int base)
{
	int k = 0;

	k = 0;
	if (base == 10) {
		while (*s >= '0' && *s <= '9') {
			k = 10 * k + (*s - '0');
			s++;
		}
	} else
		return 0;

	return k;
}


static unsigned char *get_digit_tracking(unsigned char **data)
{
	unsigned char *buf = *data;
	int i = 0, round;
	BOOLEAN others=FALSE;

	*data = &buf[i];

	round = 0;
	while (buf[i]) {
		if (buf[i] == 0xff || (buf[i] == '\n') || (buf[i] == '\r')) { //4 Enter -> end of a line
			return NULL;
		}

		//if ((buf[i] == ' ') || (buf[i] == '\t'))   //4 Next is supposed to be a number so break...
		if(others && buf[i+1] >= '0' && buf[i+1] <= '9')  //4 get the next number by passing through "something other than numbers"
			break;

		i++;
		others=TRUE;

		if (++round > 10000) {
			panic_printk("%s[%d] while (1) goes too many\n", __FUNCTION__, __LINE__);
			break;
		}
	}

    /* buf[i+1] is a number */

	round = 0;
	while (buf[i]) {

		if ((buf[i] == '\n') || (buf[i] == '\r')) {
			return NULL;
		}

		if ((buf[i] >= '0') && (buf[i] <= '9')) {
			//printk("found buf[i] = %c \n", buf[i]);
			return &buf[i];
		}

		i++;

		if (++round > 10000) {
			panic_printk("%s[%d] while (1) goes too many\n", __FUNCTION__, __LINE__);
			break;
		}
	}

	return NULL;
}

//#endif


void input_tracking_value(struct rtl8192cd_priv *priv, int offset, int num, int value)
{

	//printk("[%d][%d]=%d\n",offset%8, num, value);
	switch (offset) {
		case CCKA_P:
		case CCKA_N:
		case CCKB_P:
		case CCKB_N:
			offset = (offset % 4);
			priv->pshare->txpwr_tracking_2G_CCK[offset][num] = value;
			//printk("txpwr_tracking_2G_CCK[%d][%d]=%d\n", offset, num,value);
			break;
		case A_P:
		case A_N:
		case B_P:
		case B_N:
			offset = (offset % 4);
			priv->pshare->txpwr_tracking_2G_OFDM[offset][num] = value;
			//printk("txpwr_tracking_2G_OFDM[%d][%d]=%d\n", offset, num,value);
			break;
		case LA_P:
		case LA_N:
		case LB_P:
		case LB_N:
			offset = (offset % 4);
			priv->pshare->txpwr_tracking_5GL[offset][num] = value;
			//printk("txpwr_tracking_5GL[%d][%d]=%d\n", offset, num,value);
			break;
		case MA_P:
		case MA_N:
		case MB_P:
		case MB_N:
			offset = (offset % 4);
			priv->pshare->txpwr_tracking_5GM[offset][num] = value;
			//printk("txpwr_tracking_5GM[%d][%d]=%d\n", offset, num,value);
			break;
		case HA_P:
		case HA_N:
		case HB_P:
		case HB_N:
			offset = (offset % 4);
			priv->pshare->txpwr_tracking_5GH[offset][num] = value;
			//printk("txpwr_tracking_5GH[%d][%d]=%d\n", offset, num,value);
			break;
		default:
			break;

		}


}

int get_tx_tracking_index(struct rtl8192cd_priv *priv, int channel, int rf_path, int delta, int is_decrease, int is_CCK)
{
	int index = 0;

	if (delta == 0)
		return 0;

	if (delta > index_mapping_NUM_MAX-1)
		delta = index_mapping_NUM_MAX-1;

	//printk("\n_eric_tracking +++ channel = %d, i = %d, delta = %d, is_decrease = %d, is_CCK = %d\n", channel, i, delta, is_decrease, is_CCK);

	if (!priv->pshare->tracking_table_new)
	delta = delta - 1;

	if (channel > 14) {
		if (channel <= 99) {
			index = priv->pshare->txpwr_tracking_5GL[(rf_path * 2) + is_decrease][delta];
		} else if (channel <= 140) {

			index = priv->pshare->txpwr_tracking_5GM[(rf_path * 2) + is_decrease][delta];
		} else {
			index = priv->pshare->txpwr_tracking_5GH[(rf_path * 2) + is_decrease][delta];
		}
	} else {
		if (is_CCK) {
			index = priv->pshare->txpwr_tracking_2G_CCK[(rf_path * 2) + is_decrease][delta];
		} else {
			index = priv->pshare->txpwr_tracking_2G_OFDM[(rf_path * 2) + is_decrease][delta];
		}
	}

	//printk("_eric_tracking +++ offset = %d\n", index);

	return index;

}

static int get_tracking_table(struct rtl8192cd_priv *priv, unsigned char *line_head)
{
	unsigned char *next;
	int base, idx;
	int num = 0;
	int offset = 0;
	unsigned char *swim;
	extern int _atoi(char * s, int base);

	swim = line_head + 1;
	priv->pshare->tracking_table_new = 0;

	for (offset = 0; offset < sizeof(TXPWR_TRACKING_NAME)/sizeof(TXPWR_TRACKING_NAME[0]); offset++) { //4 legacy format
		if (!memcmp(line_head, TXPWR_TRACKING_NAME[offset], strlen(TXPWR_TRACKING_NAME[offset]))){
			//printk("Legacy****************offset:%d   For %s\n",offset,TXPWR_TRACKING_NAME[offset]);
			break;
		}
	}

	if (offset >= sizeof(TXPWR_TRACKING_NAME)/sizeof(TXPWR_TRACKING_NAME[0])) { //4 new format
		for (offset = 0; offset < sizeof(TXPWR_TRACKING_NAME_NEW)/sizeof(TXPWR_TRACKING_NAME_NEW[0]); offset++) {
			if (!memcmp(line_head, TXPWR_TRACKING_NAME_NEW[offset], strlen(TXPWR_TRACKING_NAME_NEW[offset]))){
				//printk("****************   For %s\n",TXPWR_TRACKING_NAME_NEW[offset]);
				swim = line_head + strlen(TXPWR_TRACKING_NAME_NEW[offset]);
				break;
			}
		}

		if (offset >= sizeof(TXPWR_TRACKING_NAME_NEW)/sizeof(TXPWR_TRACKING_NAME_NEW[0])){
			//printk("offset >= TXPWR_TRACKING_NAME_NUM %d\n",sizeof(TXPWR_TRACKING_NAME)/sizeof(TXPWR_TRACKING_NAME[0]));
			return offset;
		}

		priv->pshare->tracking_table_new = 1;
	}

	if (offset >= sizeof(TXPWR_TRACKING_NAME_NEW)/sizeof(TXPWR_TRACKING_NAME_NEW[0])){
		//printk("ERROR !  offset >= TXPWR_TRACKING_NAME_NUM %d\n",sizeof(TXPWR_TRACKING_NAME)/sizeof(TXPWR_TRACKING_NAME[0]));
		return offset;
	}

	/*
		2GCCKA_P	0 1 1 2 2 2 2 3 3 3 4 4 5 5 6 6 6 7 7 7 8 8 8 9 9 9 9 12
		offset represent label "2GCCKA_P"
	*/

	//printk("_Eric offset = %d \n", offset);
	//printk("_Eric line_head = %s \n", line_head);


	next = get_digit_tracking(&swim); //4 Get the start address of next number
    /*
	     2GCCKA_P	0 1 1 2 2 2 2 3 3 3 4 4 5 5 6 6 6 7 7 7 8 8 8 9 9 9 9 12
	                        next
	*/
	//printk("*next = %c\n",*next);
	while (1) {

		if (next == NULL)
			break;

		if (next) {

			base = 10;
			idx = 0;

			//printk("[%d][%d] = %d \n",offset, num, _convert_2_pwr_tracking((char *)&next[idx], base)); //4 convert to 10-base for next number
            /* Fill in index vaule from table in power track data structure */
			input_tracking_value(priv, offset, num, _convert_2_pwr_tracking((char *)&next[idx], base));

			num++;
		} else
			break;

		if (num >= index_mapping_NUM_MAX){
			break;
		}
		next = get_digit_tracking(&next);
		/*
	 	     2GCCKA_P	0 1 1 2 2 2 2 3 3 3 4 4 5 5 6 6 6 7 7 7 8 8 8 9 9 9 9 12
	                                  next
         	*/
		//if(next != NULL)
		//	printk("   *next  =  %c\n",*next);

	}


	return offset;

}


void check_tracking_table(struct rtl8192cd_priv *priv)
{

	int tmp = 0;
	int tmp2 = 0;

	for (tmp = 0; tmp < 2*TXPWR_TRACKING_PATH_NUM; tmp++) {
		for (tmp2 = 1; tmp2 < index_mapping_NUM_MAX; tmp2 ++) {
			//printk("tracking_2G_CCK[%d][%d]=%d\n",tmp,tmp2-1,priv->pshare->txpwr_tracking_2G_CCK[tmp][tmp2-1]);
			if (priv->pshare->txpwr_tracking_2G_CCK[tmp][tmp2] < priv->pshare->txpwr_tracking_2G_CCK[tmp][tmp2 - 1])
				priv->pshare->txpwr_tracking_2G_CCK[tmp][tmp2] = priv->pshare->txpwr_tracking_2G_CCK[tmp][tmp2 - 1];
		}
	}

	for (tmp = 0; tmp < 2*TXPWR_TRACKING_PATH_NUM; tmp++) {
		for (tmp2 = 1; tmp2 < index_mapping_NUM_MAX; tmp2 ++) {
			//printk("txpwr_tracking_2G_OFDM[%d][%d]=%d\n",tmp,tmp2-1,priv->pshare->txpwr_tracking_2G_OFDM[tmp][tmp2-1]);
			if (priv->pshare->txpwr_tracking_2G_OFDM[tmp][tmp2] < priv->pshare->txpwr_tracking_2G_OFDM[tmp][tmp2 - 1])
				priv->pshare->txpwr_tracking_2G_OFDM[tmp][tmp2] = priv->pshare->txpwr_tracking_2G_OFDM[tmp][tmp2 - 1];
		}
	}

	for (tmp = 0; tmp < 2*TXPWR_TRACKING_PATH_NUM; tmp++) {
		for (tmp2 = 1; tmp2 < index_mapping_NUM_MAX; tmp2 ++) {
			//printk("txpwr_tracking_5GL[%d][%d]=%d\n",tmp,tmp2-1,priv->pshare->txpwr_tracking_5GL[tmp][tmp2-1]);
			if (priv->pshare->txpwr_tracking_5GL[tmp][tmp2] < priv->pshare->txpwr_tracking_5GL[tmp][tmp2 - 1])
				priv->pshare->txpwr_tracking_5GL[tmp][tmp2] = priv->pshare->txpwr_tracking_5GL[tmp][tmp2 - 1];
		}
	}

	for (tmp = 0; tmp < 2*TXPWR_TRACKING_PATH_NUM; tmp++) {
		for (tmp2 = 1; tmp2 < index_mapping_NUM_MAX; tmp2 ++) {
			//printk("txpwr_tracking_5GM[%d][%d]=%d\n",tmp,tmp2-1,priv->pshare->txpwr_tracking_5GM[tmp][tmp2-1]);
			if (priv->pshare->txpwr_tracking_5GM[tmp][tmp2] < priv->pshare->txpwr_tracking_5GM[tmp][tmp2 - 1])
				priv->pshare->txpwr_tracking_5GM[tmp][tmp2] = priv->pshare->txpwr_tracking_5GM[tmp][tmp2 - 1];
		}
	}

	for (tmp = 0; tmp < 2*TXPWR_TRACKING_PATH_NUM; tmp++) {
		for (tmp2 = 1; tmp2 < index_mapping_NUM_MAX; tmp2 ++) {
			//printk("txpwr_tracking_5GH[%d][%d]=%d\n",tmp,tmp2-1,priv->pshare->txpwr_tracking_5GH[tmp][tmp2-1]);
			if (priv->pshare->txpwr_tracking_5GH[tmp][tmp2] < priv->pshare->txpwr_tracking_5GH[tmp][tmp2 - 1])
				priv->pshare->txpwr_tracking_5GH[tmp][tmp2] = priv->pshare->txpwr_tracking_5GH[tmp][tmp2 - 1];
		}
	}

}

int PHY_ConfigTXPwrTrackingWithParaFile(struct rtl8192cd_priv *priv)
{
	int read_bytes, num, round = 0;
	unsigned char *mem_ptr, *line_head, *next_head;
	printk("[%s %d]\n",__FUNCTION__, __LINE__);

	memset(priv->pshare->txpwr_tracking_2G_CCK, 0, (2*TXPWR_TRACKING_PATH_NUM * index_mapping_NUM_MAX));
	memset(priv->pshare->txpwr_tracking_2G_OFDM, 0, (2*TXPWR_TRACKING_PATH_NUM * index_mapping_NUM_MAX));
	memset(priv->pshare->txpwr_tracking_5GL, 0, (2*TXPWR_TRACKING_PATH_NUM * index_mapping_NUM_MAX));
	memset(priv->pshare->txpwr_tracking_5GM, 0, (2*TXPWR_TRACKING_PATH_NUM * index_mapping_NUM_MAX));
	memset(priv->pshare->txpwr_tracking_5GH, 0, (2*TXPWR_TRACKING_PATH_NUM * index_mapping_NUM_MAX));

	if ((mem_ptr = (unsigned char *)kmalloc(MAX_CONFIG_FILE_SIZE, GFP_ATOMIC)) == NULL) {
		printk("PHY_ConfigTXPwrTrackingWithParaFile(): not enough memory\n");
		return -1;
	}

	memset(mem_ptr, 0, MAX_CONFIG_FILE_SIZE); // clear memory


	if ((GET_CHIP_VER(priv) == VERSION_8188C) || (GET_CHIP_VER(priv) == VERSION_8192C)) { //_Eric_?? any other IC types ??
	}


    else if (IS_HAL_CHIP(priv)){
        GET_HAL_INTERFACE(priv)->GetHwRegHandler(priv, HW_VAR_POWERTRACKINGFILE_SIZE, (pu1Byte)&read_bytes);
		printk("[%s %d] read_bytes=%d\n",__FUNCTION__, __LINE__, read_bytes);
        GET_HAL_INTERFACE(priv)->GetHwRegHandler(priv, HW_VAR_POWERTRACKINGFILE_START, (pu1Byte)&next_head);
    }

#if 0
#endif
	else {
		//printk("[%s][NOT SUPPORT]\n", __FUNCTION__);
		kfree(mem_ptr);
		return -1;
	}

	memcpy(mem_ptr, next_head, read_bytes);
	mem_ptr[read_bytes] = 0xff;  /* add 0xff for recognition for end of table*/
	next_head = mem_ptr;

	while (1) {
		line_head = next_head;
		next_head = get_line(&line_head); /* line_head: start of current line; next_head: start of next line*/

		if (line_head == NULL) /* end of table*/
			break;

		if (line_head[0] == '/'){
			if(find_str(line_head, "Parameter")){ /* Record PHY parameter version */
				char *ch = line_head+2;
				while (1) {
					if (*ch == '\n')
						break;
					else {
						printk("%c", *ch);
						ch++;
					}
				}
				printk("\n");
			}
			continue;
		}

		num = get_tracking_table(priv, line_head); //4 for each line

		if (++round > 10000) {
			panic_printk("%s[%d] while (1) goes too many\n", __FUNCTION__, __LINE__);
			break;
		}
	}

	check_tracking_table(priv);

#if 0

	{
		int tmp = 0;
		int tmp2 = 0;

		for (tmp = 0; tmp < 4; tmp++) {
			printk("txpwr_tracking_2G_CCK #%d = ", tmp);
			for (tmp2 = 0; tmp2 < index_mapping_NUM_MAX; tmp2 ++) {
				printk("%d ", priv->pshare->txpwr_tracking_2G_CCK[tmp][tmp2]);
			}
			printk("\n");
		}

		for (tmp = 0; tmp < 4; tmp++) {
			printk("txpwr_tracking_2G_OFDM #%d = ", tmp);
			for (tmp2 = 0; tmp2 < index_mapping_NUM_MAX; tmp2 ++) {
				printk("%d ", priv->pshare->txpwr_tracking_2G_OFDM[tmp][tmp2]);
			}
			printk("\n");
		}

		for (tmp = 0; tmp < 4; tmp++) {
			printk("txpwr_tracking_5GL #%d = ", tmp);
			for (tmp2 = 0; tmp2 < index_mapping_NUM_MAX; tmp2 ++) {
				printk("%d ", priv->pshare->txpwr_tracking_5GL[tmp][tmp2]);
			}
			printk("\n");
		}

		for (tmp = 0; tmp < 4; tmp++) {
			printk("txpwr_tracking_5GM #%d = ", tmp);
			for (tmp2 = 0; tmp2 < index_mapping_NUM_MAX; tmp2 ++) {
				printk("%d ", priv->pshare->txpwr_tracking_5GM[tmp][tmp2]);
			}
			printk("\n");
		}

		for (tmp = 0; tmp < 4; tmp++) {
			printk("txpwr_tracking_5GH #%d = ", tmp);
			for (tmp2 = 0; tmp2 < index_mapping_NUM_MAX; tmp2 ++) {
				printk("%d ", priv->pshare->txpwr_tracking_5GH[tmp][tmp2]);
			}
			printk("\n");
		}

	}

#endif

	kfree(mem_ptr);

	return 0;
}
#endif

#ifdef THERMAL_CONTROL

#define STATE_INIT 0
#define STATE_THERMAL_CONTROL 1
#define THER_DIFF 1    // the diff thermal idx. if current del_ther dosen't drop down by THER_DIFF, thermal control will go more aggressive
#define INITIAL_PA 80  // limit Tx throughput to origin's INITIAL_PA percent for the 1st-time of tx duty cycle mechanism
//#define SECONDARY_PA 92  // limit Tx to current TP's SECONDARY_PA after 1st limitation
#define SECONDARY_PA 94  // limit Tx to current TP's SECONDARY_PA after 1st limitation
#define LOW_TP_PA 96   //  limit Tx to current TP's SECONDARY_PA when TP under 10Mbps
#define LOWEST_LIMIT 500 // in Kbps, Tx duty cycle final lowest bound
//#define LOWER_BOUND_LIMIT 10*1024 // in Kbps, Tx duty first lower bound
#define LOWER_BOUND_LIMIT 25*1024 // in Kbps, Tx duty first lower bound ,  25Mbps approximately 18Mbps

#define thermal_printk(fmt, args...) \
	if (priv->pshare->rf_ft_var.debug) \
		panic_printk("[%s][%s] "fmt, priv->dev->name, __FUNCTION__, ## args);

void fix_path_92e(struct rtl8192cd_priv *priv, unsigned char path)
{
	if(path == 0){ /* fix path A Tx */
		PHY_SetBBReg(priv, 0x90c , bMaskDWord, 0x81121111);
		PHY_SetBBReg(priv, 0xa04, 0xf0000000, 0x8);
		priv->pshare->rf_ft_var.chosen_path = 1;
	}else if(path == 1){ /* fix path B Tx */
		PHY_SetBBReg(priv, 0x90c , bMaskDWord, 0x82221222);
		PHY_SetBBReg(priv, 0xa04, 0xf0000000, 0x8);
		priv->pshare->rf_ft_var.chosen_path = 2;
	}
}

void set_1t_92e(struct rtl8192cd_priv *priv)
{
	PHY_SetBBReg(priv, 0x90c , bMaskDWord, 0x81121111);
	PHY_SetBBReg(priv, 0xa04, 0xf0000000, 0x8);
	RTL_W8(0xA07, 0x81);
	RTL_W32(0xC8C, 0xa0e40000);

	priv->pshare->rf_ft_var.chosen_path = 1;
	thermal_printk("92E 1T\n");
	//3 To-Do: set default 0x6d8, 0x800, 0xa11, 0xc8c, .etc
}

void set_2t_92e(struct rtl8192cd_priv *priv)
{
	RTL_W8(0x6D8, RTL_R8(0x6D8) | 0x3F);
	RTL_W8(0x800, RTL_R8(0x800) & ~BIT(1));
	RTL_W32(0x80C, RTL_R32(0x80C) & ~BIT(31));
	RTL_W32(0x90C, 0x83321333);
	RTL_W8(0xA07, 0xC1);
	RTL_W8(0xA11, RTL_R8(0xA11) & ~BIT(5));
	RTL_W32(0xC8C, 0xa0240000);
	thermal_printk("92E 2T\n");
}

void fix_path_8814(struct rtl8192cd_priv *priv, unsigned char path)
{
	if(path == 1){ /* fix path B Tx */
		PHY_SetBBReg(priv,rTX_PATH_SEL_1, bMaskH12Bits,0x2);	// Tx path B for 1SS  //0x93c
		PHY_SetBBReg(priv,rTXPATH_AC, bMask4to7Bits,0x2);		// Tx path B for CCK //0x80c
		PHY_SetBBReg(priv,rCCK_RX_AC, bMaskH4Bits	,0x4);		// Tx path B for CCK //0xa04
		priv->pshare->rf_ft_var.chosen_path = 2;
		thermal_printk("ANTDIV fix path B\n");
	}else if(path == 2){ /* fix path C Tx */
		PHY_SetBBReg(priv,rTX_PATH_SEL_1, bMaskH12Bits,0x4);	// Tx path B for 1SS
		PHY_SetBBReg(priv,rTXPATH_AC, bMask4to7Bits,0x4);		// Tx path C for CCK
		PHY_SetBBReg(priv,rCCK_RX_AC, bMaskH4Bits	,0x2);		// Tx path C for CCK
		priv->pshare->rf_ft_var.chosen_path = 3;
		thermal_printk("ANTDIV fix path C\n");
	}
}

void set_1t_8814(struct rtl8192cd_priv *priv)
{
	fix_path_8814(priv,1);
	thermal_printk("8814 1T\n");
}

void set_2t_8814(struct rtl8192cd_priv *priv)
{
	PHY_SetBBReg(priv,rTX_PATH_SEL_1, bMaskH12Bits,0x106);	// Tx path B/C for 1SS
	PHY_SetBBReg(priv,rTXPATH_AC, bMask4to7Bits,0x6);		// Tx path B/C for CCK
	PHY_SetBBReg(priv,rCCK_RX_AC, bMaskH4Bits	,0x6);		// Tx path B/C for CCK
	thermal_printk("8814 2T\n");
}

//1 Not Sure Yet
void set_1t_97f(struct rtl8192cd_priv *priv)
{
	config_phydm_trx_mode_8197f(ODMPTR, (ODM_RF_A), (ODM_RF_A|ODM_RF_B), 0);
	priv->pshare->rf_ft_var.chosen_path = 1;
	thermal_printk("97f 1T\n");
}

void set_2t_97f(struct rtl8192cd_priv *priv)
{
	config_phydm_trx_mode_8197f(ODMPTR, (ODM_RF_A|ODM_RF_B), (ODM_RF_A|ODM_RF_B), 1);
	thermal_printk("97f 2T\n");
}

//1 Not Sure Yet
void fix_path_97f(struct rtl8192cd_priv *priv, unsigned char path)
{
	if(path == 0){ /* fix path A Tx */
		config_phydm_trx_mode_8197f(ODMPTR, ODM_RF_A, (ODM_RF_A|ODM_RF_B), 0);
		priv->pshare->rf_ft_var.chosen_path = 1;
	}else if(path == 1){ /* fix path B Tx */
		config_phydm_trx_mode_8197f(ODMPTR, ODM_RF_B, (ODM_RF_A|ODM_RF_B), 0);
		priv->pshare->rf_ft_var.chosen_path = 2;
	}
}

//1 Not Sure Yet
void set_1t_8822(struct rtl8192cd_priv *priv)
{
	config_phydm_trx_mode_8822b(ODMPTR, (ODM_RF_A), (ODM_RF_A|ODM_RF_B), 0);
	priv->pshare->rf_ft_var.chosen_path = 1;
	thermal_printk("8812b 1T\n");

}
void set_2t_8822(struct rtl8192cd_priv *priv)
{
	config_phydm_trx_mode_8822b(ODMPTR, (ODM_RF_A|ODM_RF_B), (ODM_RF_A|ODM_RF_B), 1);
	thermal_printk("8812b 2T\n");
}

//1 Not Sure Yet
void fix_path_8822(struct rtl8192cd_priv *priv, unsigned char path)
{
	if(path == 0){ /* fix path A Tx */
		config_phydm_trx_mode_8822b(ODMPTR, ODM_RF_A, (ODM_RF_A|ODM_RF_B), 0);
		priv->pshare->rf_ft_var.chosen_path = 1;
	}else if(path == 1){ /* fix path B Tx */
		config_phydm_trx_mode_8822b(ODMPTR, ODM_RF_B, (ODM_RF_A|ODM_RF_B), 0);
		priv->pshare->rf_ft_var.chosen_path = 2;
	}
}



void set_1t(struct rtl8192cd_priv *priv)
{
	priv->pshare->rf_ft_var.current_path = 1;
	if(GET_CHIP_VER(priv) == VERSION_8192E)
		set_1t_92e(priv);
	else if(GET_CHIP_VER(priv) == VERSION_8814A)
		set_1t_8814(priv);
	else if (GET_CHIP_VER(priv) == VERSION_8197F)
		;//set_1t_97f(priv);
	else if (GET_CHIP_VER(priv) == VERSION_8822B)
		set_1t_8822(priv);

}

void set_2t(struct rtl8192cd_priv *priv)
{

	priv->pshare->rf_ft_var.current_path = 2;
	if(GET_CHIP_VER(priv) == VERSION_8192E){

		set_2t_92e(priv);
	}else if(GET_CHIP_VER(priv) == VERSION_8814A)
		set_2t_8814(priv);
	else if (GET_CHIP_VER(priv) == VERSION_8197F)
		;//set_2t_97f(priv);
	else if (GET_CHIP_VER(priv) == VERSION_8822B)
		set_2t_8822(priv);
}


/* decides 2T to 1T for 1SS */
void set_path(struct rtl8192cd_priv *priv)
{
	if(priv->pshare->rf_ft_var.path == 0){ /* as default by original tx2path*/
		if(priv->pshare->rf_ft_var.current_path == 1 &&
			priv->pmib->dot11RFEntry.tx2path == 1){
			set_2t(priv);
		}else if(priv->pshare->rf_ft_var.current_path == 2 &&
			priv->pmib->dot11RFEntry.tx2path == 0){
			set_1t(priv);
		}

	}else if(priv->pshare->rf_ft_var.path == 1 &&
		priv->pshare->rf_ft_var.current_path == 2){ /* set to 1T */
		/* set to 1T*/
		set_1t(priv);
	}
	else if(priv->pshare->rf_ft_var.path == 2 &&
			priv->pshare->rf_ft_var.current_path == 1){ /* set to 2T */
		/* set to 2T*/
		set_2t(priv);
	}
}

void fix_path(struct rtl8192cd_priv *priv, unsigned char path)
{

	if(GET_CHIP_VER(priv) == VERSION_8192E){
		fix_path_92e(priv, path);
	}else if(GET_CHIP_VER(priv) == VERSION_8814A){
		fix_path_8814(priv, path);
	}
}

struct stat_info* get_sta(struct rtl8192cd_priv *priv)
{
	struct stat_info	*pstat, *pstat_max=NULL;
	struct list_head	*phead, *plist;
	unsigned int		 max_tp;
	phead = &priv->asoc_list;
	plist = phead;

	while ((plist = asoc_list_get_next(priv, plist)) != phead)
	{
		pstat = list_entry(plist, struct stat_info, asoc_list);
		if (pstat){
			if((pstat->current_tx_rate <= _MCS7_RATE_ || (pstat->current_tx_rate >= _NSS1_MCS0_RATE_ && pstat->current_tx_rate <= _NSS1_MCS9_RATE_))
				 &&	pstat->tx_avarage > max_tp) // find STA using 1SS and with highest Tx throughput
				pstat_max = pstat;
		}
	}
	return pstat_max;
}

void ant_div_2t(struct rtl8192cd_priv *priv)
{
	struct stat_info	*pstat;

	pstat = get_sta(priv);
	if (pstat) {
		if(pstat->rf_info.mimorssi[0] >= pstat->rf_info.mimorssi[1] && /* compare rssi */
			priv->pshare->rf_ft_var.chosen_path != 1){  /* currently not using path A*/
			fix_path(priv,0); /* use path A */
		}else if(pstat->rf_info.mimorssi[0] < pstat->rf_info.mimorssi[1] && /* compare rssi */
				priv->pshare->rf_ft_var.chosen_path != 2){	/* currently not using path B*/
			fix_path(priv,1); /* use path B */
		}
	}
}

void ant_div_8814(struct rtl8192cd_priv *priv)
{
	struct stat_info	*pstat;

	pstat = get_sta(priv);

	if(pstat != NULL){
		if(pstat->rf_info.mimorssi[1] > pstat->rf_info.mimorssi[2] && /* compare rssi */
			  priv->pshare->rf_ft_var.chosen_path != 2){  /* currently not using path B */
			fix_path(priv,1); /* use path B */
		}else if(pstat->rf_info.mimorssi[1] < pstat->rf_info.mimorssi[2] && /* compare rssi */
				priv->pshare->rf_ft_var.chosen_path != 3){	/* currently not using path C */
			fix_path(priv,2); /* use path C */
		}
	}
}


/*
*  decides path A or path B; B or C for 8814
*  path selection by higher rssi
*/
void ant_div(struct rtl8192cd_priv *priv)
{
	if(GET_CHIP_VER(priv) == VERSION_8192E
	|| GET_CHIP_VER(priv) == VERSION_8197F
	|| GET_CHIP_VER(priv) == VERSION_8822B
		){
		ant_div_2t(priv);
	}else if(GET_CHIP_VER(priv) == VERSION_8814A){
		ant_div_8814(priv);
	}
}

unsigned char degrade_power(struct rtl8192cd_priv *priv)
{

	struct stat_info	*pstat;
	struct list_head	*phead, *plist;
	unsigned char set_power=0;

	phead = &priv->asoc_list;
	plist = phead;

	/* degrad power per STA */
//	if((plist = asoc_list_get_next(priv, plist)) != phead)
	while ((plist = asoc_list_get_next(priv, plist)) != phead)
	{
		pstat = list_entry(plist, struct stat_info, asoc_list);

		if (pstat){
			if(pstat->power == 3){/* currently in -11dB*/
				if(pstat->rssi < RSSI2-TOLERANCE){
					pstat->power = 2; /* -7 dB*/
					set_power = 1;
				}
			}else if(pstat->power == 2){/* currently in -7dB*/
				if(pstat->rssi >= RSSI2+TOLERANCE){
					pstat->power = 3; /* -11 dB*/
					set_power = 1;
				}else if(pstat->rssi < RSSI1-TOLERANCE){
					pstat->power = 1; /* -3 dB*/
					set_power = 1;
				}
			}else if(pstat->power == 1){/* currently in -3dB*/
				if(pstat->rssi >= RSSI1+TOLERANCE){
					pstat->power = 2; /* -7 dB*/
					set_power = 1;
				}else if((pstat->current_tx_rate <= _MCS7_RATE_) ||
							(pstat->current_tx_rate >= _NSS1_MCS0_RATE_ && pstat->current_tx_rate <= _NSS1_MCS9_RATE_))
				{  /*when using 1SS rate, don't degrade power*/
					pstat->power = 0; /* 0 dB*/
					set_power = 1;
					thermal_printk("using 1SS Tx rate and cancel power degradation\n");
				}
			}else{ /* currently in 0dB*/
				if(pstat->rssi >= RSSI2){
					pstat->power = 3; /* -11 dB*/
					set_power = 1;
				}else if(pstat->rssi >= RSSI1){
					pstat->power = 2; /* -7 dB*/
					set_power = 1;
				}else if((pstat->current_tx_rate >= _NSS2_MCS0_RATE_) ||
							(pstat->current_tx_rate >= _MCS8_RATE_ && pstat->current_tx_rate <= _MCS15_RATE_))
				{
					pstat->power = 1; /* -3 dB*/
					set_power = 1;
				}
			}
		}
		if(set_power)
			thermal_printk(" set powr power_desc=%u\n", pstat->power);
	}

	return set_power;
}

unsigned char cancel_degrade_power(struct rtl8192cd_priv *priv)
{
	struct stat_info	*pstat;
	struct list_head	*phead, *plist;
	unsigned char set_power=0;

	phead = &priv->asoc_list;
	plist = phead;

	/* degrad power per STA */
	//if((plist = asoc_list_get_next(priv, plist)) != phead)
	while ((plist = asoc_list_get_next(priv, plist)) != phead)
	{
		pstat = list_entry(plist, struct stat_info, asoc_list);
		if (pstat){
			if(pstat->power != 0){
				pstat->power = 0; /* 0 dB*/
				set_power = 1;
			}
		}
	}
	if(set_power)
		thermal_printk("[sta%u] cancel power degradation\n", pstat->aid);
	return set_power;
}


void update_del_ther(struct rtl8192cd_priv *priv)
{
	int ther;

	// enable power and trigger
	if(priv->up_time % 2 == 1){
		if (GET_CHIP_VER(priv)==VERSION_8814A)
			PHY_SetRFReg(priv, RF92CD_PATH_A, 0x42, BIT(17), 0x1);
		else if (GET_CHIP_VER(priv)==VERSION_8192E)
			PHY_SetRFReg(priv, RF92CD_PATH_A, 0x42, (BIT(17) | BIT(16)), 0x03);
		else if (GET_CHIP_VER(priv) == VERSION_8197F)
			PHY_SetRFReg(priv, RF92CD_PATH_A, 0x42, (BIT(17) | BIT(16)), 0x03);
		else if (GET_CHIP_VER(priv) == VERSION_8822B)
			PHY_SetRFReg(priv, RF92CD_PATH_A, 0x42, BIT(17), 0x1);
	}else{
	// delay for 1 mili-second
	//delay_ms(1);

	// query rf reg 0x24[4:0], for thermal meter value
		if (GET_CHIP_VER(priv)==VERSION_8814A)
			ther = PHY_QueryRFReg(priv, RF92CD_PATH_A, 0x42, 0xfc00, 1);
		else if (GET_CHIP_VER(priv)==VERSION_8192E)
			ther = PHY_QueryRFReg(priv, RF92CD_PATH_A, 0x42, 0xfc00, 1);
		else if (GET_CHIP_VER(priv) == VERSION_8197F)
			ther = PHY_QueryRFReg(priv, RF92CD_PATH_A, 0x42, 0xfc00, 1);
		else if (GET_CHIP_VER(priv) == VERSION_8822B)
			ther = PHY_QueryRFReg(priv, RF92CD_PATH_A, 0x42, 0xfc00, 1);

		if(priv->pmib->dot11RFEntry.ther > 7)
			priv->pshare->rf_ft_var.del_ther = ther - (int)priv->pmib->dot11RFEntry.ther;
		else
			priv->pshare->rf_ft_var.del_ther = 0;
		thermal_printk("[%d]Delta Ther:%d, current ther: %d, cal ther: %u, hit[hi-%d low-%d]\n", priv->pshare->rf_ft_var.countdown,priv->pshare->rf_ft_var.del_ther, ther, priv->pmib->dot11RFEntry.ther, priv->pshare->rf_ft_var.hitcount_hi, priv->pshare->rf_ft_var.hitcount_low);
	}
}


void limit_tp(struct rtl8192cd_priv *priv)
{
	struct stat_info	*pstat;
	struct list_head	*phead, *plist;

	phead = &priv->asoc_list;
	plist = phead;

	/* calculating average Tx throughput per STA */
	while ((plist = asoc_list_get_next(priv, plist)) != phead)
	{
		pstat = list_entry(plist, struct stat_info, asoc_list);
		if (pstat){
			pstat->sta_bwcthrd_tx = priv->pshare->rf_ft_var.limit_tp; //tx_tp_base: kbps
			thermal_printk("[sta%u] limit tp %u Kbps, %u Mbps\n", pstat->aid, pstat->sta_bwcthrd_tx, pstat->sta_bwcthrd_tx/1024);
		}
	}
}


/* Calculating average Tx throughput for every station*/
void decide_limit_tp(struct rtl8192cd_priv *priv, unsigned char get_base)
{
	struct stat_info	*pstat;
	struct list_head	*phead, *plist;

	phead = &priv->asoc_list;
	plist = phead;

	/* calculating average Tx throughput per STA */
	while ((plist = asoc_list_get_next(priv, plist)) != phead)
	{
		pstat = list_entry(plist, struct stat_info, asoc_list);
		if (pstat){
			if(get_base == 1){
				pstat->tx_tp_base = pstat->tx_avarage * 8 / 1024; // tx_average: Bytes per second; tx_tp_base: kbps
				pstat->got_limit_tp = 1;
				thermal_printk("[sta%u] got limit tp base %u Kbps, %u Mbps\n", pstat->aid, pstat->tx_tp_base, pstat->tx_tp_base/1024);
			}
			if(pstat->got_limit_tp == 1){
				pstat->tx_tp_limit = pstat->tx_tp_base * priv->pshare->rf_ft_var.pa / 100;
				if(pstat->tx_tp_limit < LOWER_BOUND_LIMIT){
					pstat->tx_tp_limit = LOWER_BOUND_LIMIT; // limitation lower bound = 10Mbps. Unit for tx_tp_limit is Kbps
				}
				pstat->sta_bwcthrd_tx = pstat->tx_tp_limit;
				thermal_printk("[sta%u] limit tp %u Kbps, %u Mbps, (%u %%)\n", pstat->aid, pstat->tx_tp_limit, pstat->tx_tp_limit/1024, priv->pshare->rf_ft_var.pa);
			}
		}
	}
}

/* Calculating average Tx throughput for every station*/
void decide_limit_tp_new(struct rtl8192cd_priv *priv)
{
	struct stat_info	*pstat;
	struct list_head	*phead, *plist;

	phead = &priv->asoc_list;
	plist = phead;

	if(priv->pshare->rf_ft_var.limit_90pa == 1){ /* limit to 90%*/
		priv->pshare->rf_ft_var.pa = LOW_TP_PA;
		thermal_printk("Meet 90%% limit conditions\n");
		while ((plist = asoc_list_get_next(priv, plist)) != phead){
			pstat = list_entry(plist, struct stat_info, asoc_list);
			if (pstat){
				if(pstat->tx_avarage * 8 / 1024 < pstat->tx_tp_limit)  // update tx_tp_base only if current TP is lower than previous limit throughput
					pstat->tx_tp_base = pstat->tx_avarage * 8 / 1024; // tx_average: Bytes per second; tx_tp_base: kbps
				else{
					pstat->tx_tp_base = pstat->tx_tp_limit;
					thermal_printk("[sta%u] Current Throughput %u Kbps, %u Mbps higher than previous limit throughput %u Kbps, %u Mbps \n", pstat->aid, pstat->tx_avarage*8/1024, pstat->tx_avarage*8/1024/1024, pstat->tx_tp_limit, pstat->tx_tp_limit/1024);
				}
				pstat->got_limit_tp = 1;
				thermal_printk("[sta%u] got limit tp base %u Kbps, %u Mbps\n", pstat->aid, pstat->tx_tp_base, pstat->tx_tp_base/1024);

				pstat->tx_tp_limit = pstat->tx_tp_base * priv->pshare->rf_ft_var.pa / 100;
				if(pstat->tx_tp_limit <= LOWEST_LIMIT)
					pstat->tx_tp_limit = LOWEST_LIMIT;
				pstat->sta_bwcthrd_tx = pstat->tx_tp_limit;
				thermal_printk("[sta%u] limit tp %u Kbps, %u Mbps, (%u %%)\n", pstat->aid, pstat->tx_tp_limit, pstat->tx_tp_limit/1024, priv->pshare->rf_ft_var.pa);
			}
		}
	}else{
		if(priv->pshare->rf_ft_var.txduty_level == 1) /* limit to 55%*/
			priv->pshare->rf_ft_var.pa = INITIAL_PA;
		else
			priv->pshare->rf_ft_var.pa = SECONDARY_PA; /* limit to 80%*/

		//1 tmp off; only one stage
		//priv->pshare->rf_ft_var.limit_90pa = 1;

		while ((plist = asoc_list_get_next(priv, plist)) != phead){
			pstat = list_entry(plist, struct stat_info, asoc_list);
			if (pstat){
				if(pstat->current_tx_rate <= _54M_RATE_){
					thermal_printk("[sta%u] don't limit TP since using OFDM or CCK\n", pstat->aid); /*don't limit for CCK or OFDM*/
				}else{
					if(pstat->tx_avarage * 8 / 1024 < pstat->tx_tp_limit || pstat->tx_tp_limit == 0)  // update tx_tp_base only if current TP is lower than previous limit throughput
						pstat->tx_tp_base = pstat->tx_avarage * 8 / 1024; // tx_average: Bytes per second; tx_tp_base: kbps
					else{
						pstat->tx_tp_base = pstat->tx_tp_limit;
						thermal_printk("[sta%u] Current Throughput %u Kbps, %u Mbps higher than previous limit throughput %u Kbps, %u Mbps \n", pstat->aid, pstat->tx_avarage*8/1024, pstat->tx_avarage*8/1024/1024, pstat->tx_tp_limit, pstat->tx_tp_limit/1024);
					}
					pstat->got_limit_tp = 1;
					thermal_printk("[sta%u] Current Throughput %u Kbps, %u Mbps\n", pstat->aid, pstat->tx_tp_base, pstat->tx_tp_base/1024);

					pstat->tx_tp_limit = pstat->tx_tp_base * priv->pshare->rf_ft_var.pa / 100;
					if(pstat->tx_tp_limit < LOWER_BOUND_LIMIT){
						pstat->tx_tp_limit = LOWER_BOUND_LIMIT; // limitation lower bound = 10Mbps. Unit for tx_tp_limit is Kbps
						thermal_printk("[sta%u] Limit Throughput under 10Mbps\n", pstat->aid);
					}else{
						priv->pshare->rf_ft_var.limit_90pa = 0;
					}
					pstat->sta_bwcthrd_tx = pstat->tx_tp_limit;
					thermal_printk("[sta%u] Limit Throughput %u Kbps, %u Mbps, (%u %%)\n", pstat->aid, pstat->tx_tp_limit, pstat->tx_tp_limit/1024, priv->pshare->rf_ft_var.pa);
				}
			}
		}
	}
}


/* disable throughput limit */
void cancel_limit_tp(struct rtl8192cd_priv *priv)
{
	struct stat_info	*pstat;
	struct list_head	*phead, *plist;

	phead = &priv->asoc_list;
	plist = phead;

	/* calculating average Tx throughput per STA */
	//if((plist = asoc_list_get_next(priv, plist)) != phead)
	while ((plist = asoc_list_get_next(priv, plist)) != phead)
	{
		pstat = list_entry(plist, struct stat_info, asoc_list);
		if (pstat){
			if(pstat->got_limit_tp){
				pstat->tx_tp_base = 0;
				pstat->tx_tp_limit = 0;
				pstat->sta_bwcthrd_tx = 0;
				pstat->got_limit_tp = 0;
#ifdef RTK_STA_BWC
				pstat->sta_bwcdrop_cnt = 0;
#endif
				thermal_printk("[sta%u] Cancel Tx throughput limit\n", pstat->aid);
			}
		}
	}
	priv->pshare->rf_ft_var.limit_90pa = 0;
}


void enter_init_state(struct rtl8192cd_priv *priv)
{
	thermal_printk("Enter Initial State\n");
	priv->pshare->rf_ft_var.state = STATE_INIT;

	thermal_printk("Disable Tx power degradation. low_power = 0\n");
	priv->pshare->rf_ft_var.low_power = 0;

	thermal_printk("Disable 1T. path = 2, path_select = 0\n");
	priv->pshare->rf_ft_var.path = 2;
	priv->pshare->rf_ft_var.path_select = 0;

	thermal_printk("Disable Tx Duty Cycle\n");
	priv->pshare->rf_ft_var.txduty = 0;
	cancel_limit_tp(priv);
}

void enter_thermal_control_state(struct rtl8192cd_priv *priv)
{
	thermal_printk("Enter Thermal Control State\n");
	priv->pshare->rf_ft_var.state = STATE_THERMAL_CONTROL;

	priv->pshare->rf_ft_var.txduty_level = 1;
	priv->pshare->rf_ft_var.txduty = 1;
	decide_limit_tp_new(priv);
	thermal_printk("Enable Tx Duty Cycle with Level %d => limit to %d%%  \n", priv->pshare->rf_ft_var.txduty_level, priv->pshare->rf_ft_var.pa);

	thermal_printk("Enable Tx power degradation. low_power = 1\n");
	priv->pshare->rf_ft_var.low_power = 1;

	thermal_printk("Enable 1T. path = 1, path_select = 1\n");
	priv->pshare->rf_ft_var.path = 1;
	priv->pshare->rf_ft_var.path_select = 1;

	priv->pshare->rf_ft_var.countdown = priv->pshare->rf_ft_var.monitor_time; // wait monitor_tim then to check
	priv->pshare->rf_ft_var.ther_drop = priv->pshare->rf_ft_var.del_ther - THER_DIFF; //expected ther to drop to during monitor_tim
	thermal_printk("ther_drop = %u\n", priv->pshare->rf_ft_var.ther_drop);
}

void state_init_dm(struct rtl8192cd_priv *priv)
{
	//if(priv->pshare->rf_ft_var.del_ther >= priv->pshare->rf_ft_var.ther_hi)
	if (priv->pshare->rf_ft_var.hitcount_hi >= THER_HIT_COUNT)
	{
		priv->pshare->rf_ft_var.hitcount_hi = 0;
		enter_thermal_control_state(priv);
	}
}

void state_thermal_control_dm(struct rtl8192cd_priv *priv)
{
	if(priv->pshare->rf_ft_var.countdown)
	priv->pshare->rf_ft_var.countdown--;
	/* thermal reduced to expected ther_low, release thermal control */
	//if(priv->pshare->rf_ft_var.del_ther <= priv->pshare->rf_ft_var.ther_low)
	if (priv->pshare->rf_ft_var.hitcount_low >= THER_HIT_COUNT)
	{
		enter_init_state(priv);
		priv->pshare->rf_ft_var.hitcount_low = 0;
	}else if(priv->pshare->rf_ft_var.countdown == 0){/*already monitored for monitor_time */
		priv->pshare->rf_ft_var.countdown = priv->pshare->rf_ft_var.monitor_time;
		if(priv->pshare->rf_ft_var.del_ther > priv->pshare->rf_ft_var.ther_drop){// hasn't drop to expected ther yet during monitor_tim
			priv->pshare->rf_ft_var.txduty_level++;
			thermal_printk("Upgrade Limit Level %d\n", priv->pshare->rf_ft_var.txduty_level);
			decide_limit_tp_new(priv);
		}else{
			priv->pshare->rf_ft_var.ther_drop -= THER_DIFF;
		}
	}
}

void ther_dm_switch(struct rtl8192cd_priv *priv)
{

	if(priv->pshare->rf_ft_var.state == STATE_INIT){
		if (priv->pshare->rf_ft_var.del_ther >= priv->pshare->rf_ft_var.ther_hi)
			priv->pshare->rf_ft_var.hitcount_hi++;
		else if (priv->pshare->rf_ft_var.hitcount_hi)
			priv->pshare->rf_ft_var.hitcount_hi--;
		state_init_dm(priv);
	}else if(priv->pshare->rf_ft_var.state == STATE_THERMAL_CONTROL){
		if (priv->pshare->rf_ft_var.del_ther <= priv->pshare->rf_ft_var.ther_low)
			priv->pshare->rf_ft_var.hitcount_low++;
		else if (priv->pshare->rf_ft_var.hitcount_low)
			priv->pshare->rf_ft_var.hitcount_low--;
		state_thermal_control_dm(priv);
	}
}

void set_power_desc(struct rtl8192cd_priv *priv)
{
	struct stat_info	*pstat;
	struct list_head	*phead, *plist;

	phead = &priv->asoc_list;
	plist = phead;

	/* degrad power per STA */
	//if((plist = asoc_list_get_next(priv, plist)) != phead)
	while ((plist = asoc_list_get_next(priv, plist)) != phead)
	{
		pstat = list_entry(plist, struct stat_info, asoc_list);
		if (pstat){
			clear_short_cut_cache(priv, pstat); /*clear short cut cache so that new Tx power desc can take effect*/
			thermal_printk("[sta%u] clear short cut cache\n", pstat->aid);
		}
	}
}

void thermal_control_dm(struct rtl8192cd_priv *priv)
{

	/* get delta thermal  */
	if(!priv->pshare->rf_ft_var.man)
		update_del_ther(priv);

	/* decides mechanism on off*/
	if(priv->pshare->rf_ft_var.ther_dm == 1)
		ther_dm_switch(priv);

	/* 1. Tx Power*/
	if(priv->pshare->rf_ft_var.low_power == 1){
		if(degrade_power(priv)) /* degrade Tx power by descriptor per STA */
			set_power_desc(priv);
	}else if(priv->pshare->rf_ft_var.low_power == 2){ /*  set Tx power from MIB power_desc*/
		if(priv->pshare->rf_ft_var.current_power_desc != (priv->pshare->rf_ft_var.power_desc) ){
			priv->pshare->rf_ft_var.current_power_desc = priv->pshare->rf_ft_var.power_desc;
			set_power_desc(priv);
			thermal_printk("Use %ddB\n", priv->pshare->rf_ft_var.current_power_desc==3?-11:priv->pshare->rf_ft_var.current_power_desc==2?-7:priv->pshare->rf_ft_var.current_power_desc==1?-3:priv->pshare->rf_ft_var.current_power_desc==0?0:100);
		}
	}else if(priv->pshare->rf_ft_var.low_power == 0){
		if(cancel_degrade_power(priv)) /* cancel degrading Tx power by descriptor per STA */
			set_power_desc(priv);
	}

	/* 2. 2T -> 1S */
	set_path(priv); /* decides 2T to 1T for 1SS */

	/* 2.2 Antenna Diverisy */
#if 0
	if(priv->pshare->rf_ft_var.path == 1 &&  /* if uses 1T */
		priv->pshare->rf_ft_var.path_select == 1 ){ /* enable path selection */
		ant_div(priv);  /* decides path A or path B; B or C for 8814 */
	}
#endif

#if 1
	/* Manual Tx Duty Cycle*/
	if(priv->pshare->rf_ft_var.man == 1){
		if(priv->pshare->rf_ft_var.txduty == 1){
			limit_tp(priv);
		}
/*
		if(priv->pshare->rf_ft_var.pa_refresh == 1){
			decide_limit_tp(priv, 0);
			priv->pshare->rf_ft_var.pa_refresh = 0;
		}else if(priv->pshare->rf_ft_var.txduty == 1){
			decide_limit_tp(priv, 1);
		}else if(priv->pshare->rf_ft_var.txduty == 0){
			cancel_limit_tp(priv);
		}
*/
	}
#endif
}

#endif
#ifdef BT_COEXIST
int c2h_bt_cnt =0;
int bt_state;


void bt_coex_dm(struct rtl8192cd_priv *priv)
{
	int bt_disable=0;
	unsigned int reg770, reg774;
	reg770 = PHY_QueryBBReg(priv, 0x770, bMaskHWord);
	reg774 = PHY_QueryBBReg(priv, 0x774, bMaskHWord); /* BT TRx counter*/
	if(c2h_bt_cnt == 0 && reg770 == 0 && reg774 == 0){
		bt_disable = 1;
	} else{
		bt_disable = 0;
		c2h_bt_cnt = 0;
	}
	/*
	 *	BT enable --> BT disable, turn off TDMA
	*/
	if(bt_disable && bt_state){
		unsigned char H2CCommand[1]={0};
		H2CCommand[0] = 0x8;
		FillH2CCmd88XX(priv, H2C_88XX_BT_TDMA, 1, H2CCommand);
		if(priv->pshare->rf_ft_var.bt_dump)
			panic_printk("[%s] BT disable\n",__FUNCTION__);

		delay_ms(10);
		PHY_SetBBReg(priv, 0x40, bMaskDWord, 0x200);
		bt_state = 0;
	}
	/*
	 *	BT disable --> BT enable, turn on TDMA
	*/
	else if(!bt_disable && !bt_state){
		unsigned char H2CCommand[6]={0};
		unsigned int center_ch;
		if(priv->pshare->rf_ft_var.bt_dump)
			panic_printk("[%s] BT alive\n",__FUNCTION__);

		PHY_SetBBReg(priv, 0x40, bMaskDWord, 0x220);
		/*
		*	Parameter 1: e3,12,12,21,10,0
		*	Parameter 2: e3,12,03,31,10,0
		*/
		H2CCommand[0] = 0x61;
		H2CCommand[1] = 0x12;
		H2CCommand[2] = 0x03;
		H2CCommand[3] = 0x31;
		H2CCommand[4] = 0x10;
		H2CCommand[5] = 0x0;
		FillH2CCmd88XX(priv, H2C_88XX_BT_TDMA, 6, H2CCommand);

		center_ch = get_center_channel(priv, priv->pmib->dot11RFEntry.dot11channel, priv->pshare->offset_2nd_chan, 1);
		H2CCommand[0] = 0x01;
		H2CCommand[1] = center_ch;
		if(priv->pshare->CurrentChannelBW == HT_CHANNEL_WIDTH_20_40){
			H2CCommand[2] = 0x30;
		} else {
			H2CCommand[2] = 0x20;
		}
		FillH2CCmd88XX(priv, 0x66, 3, H2CCommand);
		bt_state = 1;
	}
}
#endif

#ifdef CONFIG_RF_DPK_SETTING_SUPPORT
void TX_DPK_Tracking(struct rtl8192cd_priv *priv)
{
	if (priv->up_time % priv->pshare->rf_ft_var.dpk_period == 0) {

	if (GET_CHIP_VER(priv) == VERSION_8197F) {
		phy_dpk_track_8197f(ODMPTR);
		}
	}
}
#endif


void TXPowerTracking(struct rtl8192cd_priv *priv)
{
	if (priv->up_time % priv->pshare->rf_ft_var.tpt_period == 0) {
#if !defined(USE_OUT_SRC) || defined(_OUTSRC_COEXIST)
#ifdef _OUTSRC_COEXIST
		if (!IS_OUTSRC_CHIP(priv))
#endif
		{
		}
#endif
	{
		if (IS_OUTSRC_CHIP(priv)) {
			#ifndef TPT_THREAD
			ODM_TXPowerTrackingCheck(ODMPTR);
			#else
			rtl_atomic_set(&priv->pshare->do_tpt, 1);
			#endif
		}
	}
	}
	else if ((priv->up_time % (priv->pshare->rf_ft_var.tpt_period)) == (priv->pshare->rf_ft_var.tpt_period -1))
	{
		// It will execute the TX power tracking at next Sec, so Trigger the Thermo Meter update first.
		// for 8192D, 8188E, 8192E, 8821 serious, the Thermo meter register address is 0x42
		PHY_SetRFReg(priv, RF92CD_PATH_A, RF_T_METER_92D, BIT(16)|BIT(17), 0x3);

		//panic_printk("%s:%d trigger thermalmeter!\n", __FUNCTION__, __LINE__);
		return;
	}
}

void SetTxPowerLevel(struct rtl8192cd_priv *priv, unsigned char channel)
{
	//unsigned int channel = priv->pmib->dot11RFEntry.dot11channel;

	if ((GET_CHIP_VER(priv) == VERSION_8881A) || (GET_CHIP_VER(priv) == VERSION_8814A) || (GET_CHIP_VER(priv) == VERSION_8822B)) {
		GET_HAL_INTERFACE(priv)->PHYSetOFDMTxPowerHandler(priv, channel);
		if (priv->pshare->curr_band == BAND_2G) {
			if (RT_STATUS_FAILURE == GET_HAL_INTERFACE(priv)->PHYSetCCKTxPowerHandler(priv, channel)) {
				DEBUG_WARN("PHYSetCCKTxPower Fail !\n");
			}
		}
		priv->pshare->No_RF_Write = 1;
	}

	if (GET_CHIP_VER(priv) == VERSION_8192E ||(GET_CHIP_VER(priv) == VERSION_8197F))  {
		GET_HAL_INTERFACE(priv)->PHYSetOFDMTxPowerHandler(priv, channel);
		if (priv->pshare->curr_band == BAND_2G) {
			GET_HAL_INTERFACE(priv)->PHYSetCCKTxPowerHandler(priv, channel);
		}
	}


	selectMinPowerIdex(priv);

	return;
}

#ifdef _DEBUG_RTL8192CD_

//_TXPWR_REDEFINE
int Read_PG_File(struct rtl8192cd_priv *priv, int reg_file, int table_number,
				  char *MCSTxAgcOffset_A, char *MCSTxAgcOffset_B, char *OFDMTxAgcOffset_A,
				  char *OFDMTxAgcOffset_B, char *CCKTxAgc_A, char *CCKTxAgc_B)
{
	int                read_bytes = 0, num, len = 0, round;
	unsigned int       u4bRegOffset, u4bRegValue, u4bRegMask;
	unsigned char      *mem_ptr, *line_head, *next_head = NULL;
	struct PhyRegTable *phyreg_table = NULL;
	struct MacRegTable *macreg_table = NULL;
	unsigned short     max_len = 0;
	int                file_format = TWO_COLUMN;

	//printk("PHYREG_PG = %d\n", PHYREG_PG);

	if (reg_file == PHYREG_PG) {
		//printk("[%s][PHY_REG_PG]\n",__FUNCTION__);


		macreg_table = (struct MacRegTable *)priv->pshare->phy_reg_pg_buf;
		max_len = PHY_REG_PG_SIZE;
		file_format = THREE_COLUMN;
	}


#ifdef MP_TEST
	else if (reg_file == PHYREG_MP) {
	}
#endif

	{
		if ((mem_ptr = (unsigned char *)kmalloc(MAX_CONFIG_FILE_SIZE, GFP_ATOMIC)) == NULL) {
			printk("PHY_ConfigBBWithParaFile(): not enough memory\n");
			return -1;
		}

		memset(mem_ptr, 0, MAX_CONFIG_FILE_SIZE); // clear memory
		memcpy(mem_ptr, next_head, read_bytes);

		next_head = mem_ptr;
		round = 0;
		while (1) {
			line_head = next_head;
			next_head = get_line(&line_head);
			if (line_head == NULL)
				break;

			if (line_head[0] == '/')
				continue;

			if (file_format == TWO_COLUMN) {
				num = get_offset_val(line_head, &u4bRegOffset, &u4bRegValue);
				if (num > 0) {
					phyreg_table[len].offset = u4bRegOffset;
					phyreg_table[len].value = u4bRegValue;
					len++;

					if (u4bRegOffset == 0xff)
						break;
					if ((len * sizeof(struct PhyRegTable)) > max_len)
						break;
				}
			} else {
				num = get_offset_mask_val(line_head, &u4bRegOffset, &u4bRegMask , &u4bRegValue);
				if (num > 0) {
					macreg_table[len].offset = u4bRegOffset;
					macreg_table[len].mask = u4bRegMask;
					macreg_table[len].value = u4bRegValue;
					len++;
					if (u4bRegOffset == 0xff)
						break;
					if ((len * sizeof(struct MacRegTable)) > max_len)
						break;
				}
			}

			if (++round > 10000) {
				panic_printk("%s[%d] while (1) goes too many\n", __FUNCTION__, __LINE__);
				break;
			}
		}

		kfree(mem_ptr);

		if ((len * sizeof(struct PhyRegTable)) > max_len) {
			printk("PHY REG PG table buffer not large enough!\n");
			printk("len=%d,sizeof(struct PhyRegTable)=%d,max_len=%u",len,sizeof(struct PhyRegTable),max_len);
			return -1;
		}
	}

	num = 0;
	round = 0;
	while (1) {
		if (file_format == THREE_COLUMN) {
			u4bRegOffset = macreg_table[num].offset;
			u4bRegValue = macreg_table[num].value;
			u4bRegMask = macreg_table[num].mask;
		} else {
			u4bRegOffset = phyreg_table[num].offset;
			u4bRegValue = phyreg_table[num].value;
		}

		if (u4bRegOffset == 0xff)
			break;
		else if (file_format == THREE_COLUMN) {

			{
				//PHY_SetBBReg(priv, u4bRegOffset, u4bRegMask, u4bRegValue);
				//printk("3C - 92C %x %x %x \n", u4bRegOffset, u4bRegMask, u4bRegValue);
			}
		} else {
			//printk("Not 3C - %x %x %x \n", u4bRegOffset, bMaskDWord, u4bRegValue);
			//PHY_SetBBReg(priv, u4bRegOffset, bMaskDWord, u4bRegValue);
		}
		num++;

		if (++round > 10000) {
			panic_printk("%s[%d] while (1) goes too many\n", __FUNCTION__, __LINE__);
			break;
		}
	}

	return 0;
}

#endif

/*-----------------------------------------------------------------------------
 * Function:    PHY_ConfigBBWithParaFile()
 *
 * Overview:    This function read BB parameters from general file format, and do register
 *			  Read/Write
 *
 * Input:      	PADAPTER		Adapter
 *			ps1Byte 			pFileName
 *
 * Output:      NONE
 *
 * Return:      RT_STATUS_SUCCESS: configuration file exist
 *
 *---------------------------------------------------------------------------*/
int PHY_ConfigBBWithParaFile(struct rtl8192cd_priv *priv, int reg_file)
{
	int                read_bytes = 0, num = 0, len = 0, round;
	unsigned int       u4bRegOffset, u4bRegValue, u4bRegMask = 0;
	int 			   file_format = TWO_COLUMN;
	unsigned char      *mem_ptr, *line_head, *next_head = NULL;
#if 1//defined(CONFIG_RTL_8812_SUPPORT) || defined(CONFIG_RTL_88E_SUPPORT) || defined(CONFIG_WLAN_HAL)//TXPWR_LMT_8812 TXPWR_LMT_88E
	unsigned char      *mem_ptr2, *next_head2=NULL;
#endif
	struct PhyRegTable *phyreg_table = NULL;
	struct MacRegTable *macreg_table = NULL;
	unsigned short     max_len = 0;
	unsigned int 		regstart, regend;
	int				idx = 0, pg_tbl_idx = BGN_2040_ALL, write_en = 0;

	if (reg_file == AGCTAB) {
		phyreg_table = (struct PhyRegTable *)priv->pshare->agc_tab_buf;





		if (IS_HAL_CHIP(priv)) {
#ifdef HIGH_POWER_EXT_PA
#ifdef HIGH_POWER_EXT_LNA
			if (priv->pshare->rf_ft_var.use_ext_pa && priv->pshare->rf_ft_var.use_ext_lna) {
				GET_HAL_INTERFACE(priv)->GetHwRegHandler(priv, HW_VAR_PHYREGFILE_AGC_HP_SIZE, (pu1Byte)&read_bytes);
				GET_HAL_INTERFACE(priv)->GetHwRegHandler(priv, HW_VAR_PHYREGFILE_AGC_HP_START, (pu1Byte)&next_head);
			} else if (priv->pshare->rf_ft_var.use_ext_pa && !priv->pshare->rf_ft_var.use_ext_lna) {
				GET_HAL_INTERFACE(priv)->GetHwRegHandler(priv, HW_VAR_PHYREGFILE_AGC_EXTPA_SIZE, (pu1Byte)&read_bytes);
				GET_HAL_INTERFACE(priv)->GetHwRegHandler(priv, HW_VAR_PHYREGFILE_AGC_EXTPA_START, (pu1Byte)&next_head);
			} else if (!priv->pshare->rf_ft_var.use_ext_pa && priv->pshare->rf_ft_var.use_ext_lna) {
				GET_HAL_INTERFACE(priv)->GetHwRegHandler(priv, HW_VAR_PHYREGFILE_AGC_EXTLNA_SIZE, (pu1Byte)&read_bytes);
				GET_HAL_INTERFACE(priv)->GetHwRegHandler(priv, HW_VAR_PHYREGFILE_AGC_EXTLNA_START, (pu1Byte)&next_head);
			} else
#else
			if (priv->pshare->rf_ft_var.use_ext_pa) {
				GET_HAL_INTERFACE(priv)->GetHwRegHandler(priv, HW_VAR_PHYREGFILE_AGC_EXTPA_SIZE, (pu1Byte)&read_bytes);
				GET_HAL_INTERFACE(priv)->GetHwRegHandler(priv, HW_VAR_PHYREGFILE_AGC_EXTPA_START, (pu1Byte)&next_head);
			} else
#endif
#else
#ifdef HIGH_POWER_EXT_LNA
			if (priv->pshare->rf_ft_var.use_ext_lna) {
				GET_HAL_INTERFACE(priv)->GetHwRegHandler(priv, HW_VAR_PHYREGFILE_AGC_EXTLNA_SIZE, (pu1Byte)&read_bytes);
				GET_HAL_INTERFACE(priv)->GetHwRegHandler(priv, HW_VAR_PHYREGFILE_AGC_EXTLNA_START, (pu1Byte)&next_head);
			} else
#endif
#endif
			{
				GET_HAL_INTERFACE(priv)->GetHwRegHandler(priv, HW_VAR_PHYREGFILE_AGC_SIZE, (pu1Byte)&read_bytes);
				GET_HAL_INTERFACE(priv)->GetHwRegHandler(priv, HW_VAR_PHYREGFILE_AGC_START, (pu1Byte)&next_head);
			}
		}

		max_len = AGC_TAB_SIZE;
	} else if (reg_file == PHYREG_PG) {
		//printk("[%s][PHY_REG_PG]\n",__FUNCTION__);





		if ( IS_HAL_CHIP(priv) ) {
#ifdef PWR_BY_RATE_92E_HP
#ifdef HIGH_POWER_EXT_PA
			if (priv->pshare->rf_ft_var.use_ext_pa) {
				GET_HAL_INTERFACE(priv)->GetHwRegHandler(priv, HW_VAR_PHYREGFILE_PG_HP_SIZE, (pu1Byte)&read_bytes);
				GET_HAL_INTERFACE(priv)->GetHwRegHandler(priv, HW_VAR_PHYREGFILE_PG_HP_START, (pu1Byte)&next_head);
			} else
#endif
#endif
			{
				GET_HAL_INTERFACE(priv)->GetHwRegHandler(priv, HW_VAR_PHYREGFILE_PG_SIZE, (pu1Byte)&read_bytes);
				GET_HAL_INTERFACE(priv)->GetHwRegHandler(priv, HW_VAR_PHYREGFILE_PG_START, (pu1Byte)&next_head);
			}
			if(IS_HAL_CHIP(priv)) {
				if (priv->pshare->curr_band == BAND_5G)
					pg_tbl_idx = 1;
				else
					pg_tbl_idx = 0;

			} else
			{
			/* In Noraml Driver mode, and if mib 'pwr_by_rate' = 0 >> Use default power by rate table  */
			if (
#ifdef MP_TEST
				priv->pshare->rf_ft_var.mp_specific ||
#endif
				priv->pshare->rf_ft_var.pwr_by_rate) {
				if (priv->pshare->is_40m_bw == 0) {
					if (priv->pmib->dot11RFEntry.dot11channel <= 3)
						pg_tbl_idx = BGN_20_CH1_3;
					else if (priv->pmib->dot11RFEntry.dot11channel <= 9)
						pg_tbl_idx = BGN_20_CH4_9;
					else
						pg_tbl_idx = BGN_20_CH10_14;
				} else {
					int val = priv->pmib->dot11RFEntry.dot11channel;

					if (priv->pshare->offset_2nd_chan == 1)
						val -= 2;
					else
						val += 2;

					if (val <= 3)
						pg_tbl_idx = BGN_40_CH1_3;
					else if (val <= 9)
						pg_tbl_idx = BGN_40_CH4_9;
					else
						pg_tbl_idx = BGN_40_CH10_14;
				}
			}

#ifdef MP_TEST
			if (priv->pshare->rf_ft_var.mp_specific)
				pg_tbl_idx = 0;
#endif
			}
			DEBUG_INFO("channel=%d pg_tbl_idx=%d\n", priv->pmib->dot11RFEntry.dot11channel, pg_tbl_idx);
		}

		macreg_table = (struct MacRegTable *)priv->pshare->phy_reg_pg_buf;
		max_len = PHY_REG_PG_SIZE;
		file_format = THREE_COLUMN;
		priv->pshare->txpwr_pg_format_abs = 0;

		unsigned char      *mem_ptr_tmp, *next_head_tmp=NULL;
		if ((mem_ptr_tmp= (unsigned char *)kmalloc(MAX_CONFIG_FILE_SIZE, GFP_ATOMIC)) == NULL) {
			printk("PHY_ConfigBBWithParaFile(): not enough memory\n");
			return -1;
		}

		memset(mem_ptr_tmp, 0, MAX_CONFIG_FILE_SIZE); // clear memory
		memcpy(mem_ptr_tmp, next_head, read_bytes);

		next_head_tmp= mem_ptr_tmp;
		round = 0;
		while(1) {
			line_head = next_head_tmp;
			next_head_tmp= get_line(&line_head);

			if (line_head == NULL)
				break;
			if (line_head[0] == '/')
				continue;

			if (line_head[0] == '#') {
				int line_idx = 1; //line_len = strlen(line_head);

				while (line_idx <= strlen(line_head))
				{
					if (!memcmp(&(line_head[line_idx]), "Exact", 5)) {
						//panic_printk("\n******** PG Exact format !!! ********\n");
						priv->pshare->txpwr_pg_format_abs = 1;
						file_format = FIVE_COLUMN;
						break;
					}
					line_idx++;
				}
			}

			if (!memcmp(line_head, "0x", 2) || !memcmp(line_head, "0X", 2))
				break;

			if (++round > 10000) {
				panic_printk("%s[%d] while (1) goes too many\n", __FUNCTION__, __LINE__);
				break;
			}
		}

		kfree(mem_ptr_tmp);

		//panic_printk("PG file_format : %s\n", ((file_format == THREE_COLUMN) ? "THREE_COLUMN" : "FIVE_COLUMN"));

#ifdef TXPWR_LMT_NEWFILE
		if ((file_format == THREE_COLUMN) && (GET_CHIP_VER(priv) >= VERSION_8188E)) {
			panic_printk("%s() fail !!!:  Wrong PHY_REG_PG format\n", __FUNCTION__);
			return -1;
		}
#endif
		memset(priv->pshare->tgpwr_CCK_new, 0, 2);
		memset(priv->pshare->tgpwr_OFDM_new, 0, 2);
		memset(priv->pshare->tgpwr_HT1S_new, 0, 2);
		memset(priv->pshare->tgpwr_HT2S_new, 0, 2);
		memset(priv->pshare->tgpwr_VHT1S_new, 0, 2);
		memset(priv->pshare->tgpwr_VHT2S_new, 0, 2);
	}
#if 0
	else if (reg_file == PHYREG_1T2R) {
		macreg_table = (struct MacRegTable *)priv->pshare->phy_reg_2to1;
		max_len = PHY_REG_1T2R;
		file_format = THREE_COLUMN;
		if (priv->pshare->rf_ft_var.pathB_1T == 0) { // PATH A
			next_head = __PHY_to1T2R_start;
			read_bytes = (int)(__PHY_to1T2R_end - __PHY_to1T2R_start);
		} else { // PATH B
			next_head = __PHY_to1T2R_b_start;
			read_bytes = (int)(__PHY_to1T2R_b_end - __PHY_to1T2R_b_start);
		}
	}
#endif
	else if (reg_file == PHYREG) {

		if ( IS_HAL_CHIP(priv) ) {
			phyreg_table = (struct PhyRegTable *)priv->pshare->phy_reg_buf;
#ifdef HIGH_POWER_EXT_PA
#ifdef HIGH_POWER_EXT_LNA
			if (priv->pshare->rf_ft_var.use_ext_pa && priv->pshare->rf_ft_var.use_ext_lna) {
				GET_HAL_INTERFACE(priv)->GetHwRegHandler(priv, HW_VAR_PHYREGFILE_HP_SIZE, (pu1Byte)&read_bytes);
				GET_HAL_INTERFACE(priv)->GetHwRegHandler(priv, HW_VAR_PHYREGFILE_HP_START, (pu1Byte)&next_head);
			} else if (priv->pshare->rf_ft_var.use_ext_pa && !priv->pshare->rf_ft_var.use_ext_lna) {
				GET_HAL_INTERFACE(priv)->GetHwRegHandler(priv, HW_VAR_PHYREGFILE_EXTPA_SIZE, (pu1Byte)&read_bytes);
				GET_HAL_INTERFACE(priv)->GetHwRegHandler(priv, HW_VAR_PHYREGFILE_EXTPA_START, (pu1Byte)&next_head);
			} else if (!priv->pshare->rf_ft_var.use_ext_pa && priv->pshare->rf_ft_var.use_ext_lna) {
				GET_HAL_INTERFACE(priv)->GetHwRegHandler(priv, HW_VAR_PHYREGFILE_EXTLNA_SIZE, (pu1Byte)&read_bytes);
				GET_HAL_INTERFACE(priv)->GetHwRegHandler(priv, HW_VAR_PHYREGFILE_EXTLNA_START, (pu1Byte)&next_head);
			} else
#else
			if (priv->pshare->rf_ft_var.use_ext_pa) {
				GET_HAL_INTERFACE(priv)->GetHwRegHandler(priv, HW_VAR_PHYREGFILE_EXTPA_SIZE, (pu1Byte)&read_bytes);
				GET_HAL_INTERFACE(priv)->GetHwRegHandler(priv, HW_VAR_PHYREGFILE_EXTPA_START, (pu1Byte)&next_head);
			} else
#endif
#else
#ifdef HIGH_POWER_EXT_LNA
			if ( priv->pshare->rf_ft_var.use_ext_lna) {
				GET_HAL_INTERFACE(priv)->GetHwRegHandler(priv, HW_VAR_PHYREGFILE_EXTLNA_SIZE, (pu1Byte)&read_bytes);
				GET_HAL_INTERFACE(priv)->GetHwRegHandler(priv, HW_VAR_PHYREGFILE_EXTLNA_START, (pu1Byte)&next_head);
			} else
#endif
#endif
			{
				GET_HAL_INTERFACE(priv)->GetHwRegHandler(priv, HW_VAR_PHYREGFILE_SIZE, (pu1Byte)&read_bytes);
				GET_HAL_INTERFACE(priv)->GetHwRegHandler(priv, HW_VAR_PHYREGFILE_START, (pu1Byte)&next_head);
			}

			max_len = PHY_REG_SIZE;
		}

	}
#ifdef MP_TEST
	else if (reg_file == PHYREG_MP) {



		if ( IS_HAL_CHIP(priv) ) {
			phyreg_table = (struct PhyRegTable *)priv->pshare->phy_reg_mp_buf;
			printk("[%s][PHY_REG_MP_HAL]\n", __FUNCTION__);
			GET_HAL_INTERFACE(priv)->GetHwRegHandler(priv, HW_VAR_PHYREGFILE_MP_SIZE, (pu1Byte)&read_bytes);
			GET_HAL_INTERFACE(priv)->GetHwRegHandler(priv, HW_VAR_PHYREGFILE_MP_START, (pu1Byte)&next_head);
			max_len = PHY_REG_SIZE;
		}

	}
#endif
	else if (reg_file == PHYREG_1T1R) { // PATH A
		phyreg_table = (struct PhyRegTable *)priv->pshare->phy_reg_buf;


		if ( IS_HAL_CHIP(priv) ) {
			printk("[%s][PHY_REG_1T_HAL]\n", __FUNCTION__);
			GET_HAL_INTERFACE(priv)->GetHwRegHandler(priv, HW_VAR_PHYREGFILE_1T_SIZE, (pu1Byte)&read_bytes);
			GET_HAL_INTERFACE(priv)->GetHwRegHandler(priv, HW_VAR_PHYREGFILE_1T_START, (pu1Byte)&next_head);
		}

		max_len = PHY_REG_SIZE;
#if 0
		if (priv->pshare->rf_ft_var.pathB_1T == 0) {
			next_head = __PHY_to1T1R_start;
			read_bytes = (int)(__PHY_to1T1R_end - __PHY_to1T1R_start);
		} else { // PATH B
			next_head = __PHY_to1T1R_b_start;
			read_bytes = (int)(__PHY_to1T1R_b_end - __PHY_to1T1R_b_start);
		}
#endif
	}

	{
		if (GET_CHIP_VER(priv) == VERSION_8812E || GET_CHIP_VER(priv)==VERSION_8881A  || GET_CHIP_VER(priv)==VERSION_8814A  || GET_CHIP_VER(priv)==VERSION_8822B) {
			if (pg_tbl_idx == 0) { //4 2G
				regstart = 0xc20;
				if(GET_CHIP_VER(priv) == VERSION_8814A || GET_CHIP_VER(priv)==VERSION_8822B)
					regend = 0x1ae8;
				else
					regend = 0xe38;
			} else {
				regstart = 0xc24;
				if(GET_CHIP_VER(priv) == VERSION_8814A || GET_CHIP_VER(priv)==VERSION_8822B)
					regend = 0x1ae8;
				else
					regend = 0xe4c;
			}
		} else {
			regstart = 0xe00;
			regend = 0x868;
		}
		//printk("regstart=%x\nregend=%x\n",regstart,regend);
#if 0
		if ((file_format == FIVE_COLUMN) || (file_format == SEVEN_COLUMN))
		{

			if ((mem_ptr2 = (unsigned char *)kmalloc(MAX_CONFIG_FILE_SIZE, GFP_ATOMIC)) == NULL) {
				printk("PHY_ConfigBBWithParaFile(): not enough memory\n");
				return -1;
			}

			memset(mem_ptr2, 0, MAX_CONFIG_FILE_SIZE); // clear memory
			memcpy(mem_ptr2, next_head, read_bytes);

			next_head2 = mem_ptr2;
			while(1) {
				line_head = next_head2;
				next_head2 = get_line(&line_head);

				if (line_head == NULL)
					break;
				if (line_head[0] == '/')
					continue;
				get_target_val_new(priv, line_head, &u4bRegOffset, &u4bRegMask ,&u4bRegValue);
			}

			kfree(mem_ptr2);
		}
#endif
		if((mem_ptr = (unsigned char *)kmalloc(MAX_CONFIG_FILE_SIZE, GFP_ATOMIC)) == NULL) {
			printk("PHY_ConfigBBWithParaFile(): not enough memory\n");
			return -1;
		}

		memset(mem_ptr, 0, MAX_CONFIG_FILE_SIZE); // clear memory
		memcpy(mem_ptr, next_head, read_bytes);

		next_head = mem_ptr;
		round = 0;
		while (1) {
			line_head = next_head;
			next_head = get_line(&line_head);
			if (line_head == NULL)
				break;
			if (line_head[0] == '/'){
				if(find_str(line_head, "Parameter")||find_str(line_head, "version")){ /* Record PHY parameter version */
					char *ch = line_head+2;
					while (1) {
						if (*ch == '\0')
							break;
						else {
							printk("%c", *ch);
							ch++;
						}
					}
					printk("\n");
				}
				continue;
			}
			if (line_head[0] == '#')
				continue;

			if (file_format == TWO_COLUMN) {
				num = get_offset_val(line_head, &u4bRegOffset, &u4bRegValue);
				if (num > 0) {
					phyreg_table[len].offset = u4bRegOffset;
					phyreg_table[len].value = u4bRegValue;
					len++;

					if ((len & 0x7ff) == 0)
						watchdog_kick();

					if ((GET_CHIP_VER(priv) == VERSION_8188E) || (GET_CHIP_VER(priv) == VERSION_8812E) || (IS_HAL_CHIP(priv))) {
						if (u4bRegOffset == 0xffff)
							break;
					} else if (CONFIG_WLAN_NOT_HAL_EXIST)
					{
						if (u4bRegOffset == 0xff)
							break;
					}
					if ((len * sizeof(struct PhyRegTable)) > max_len)
						break;
				}
			} else {
				if (reg_file == PHYREG_PG) {
					if (file_format == THREE_COLUMN){
						num = get_offset_mask_val(line_head, &u4bRegOffset, &u4bRegMask , &u4bRegValue);
					}else{
						num = get_offset_mask_val_new(priv, line_head, &u4bRegOffset, &u4bRegMask , &u4bRegValue);
						//printk("reg=%x, mask=%x, power=%x\n",u4bRegOffset,u4bRegMask,u4bRegValue);
						/* 2G parse first parts, 5G parse both parts but only use the last one*/
						if((GET_CHIP_VER(priv) == VERSION_8814A)||(GET_CHIP_VER(priv) == VERSION_8822B)){
							if (pg_tbl_idx == 0 && u4bRegOffset == regend) {
								macreg_table[len].offset = u4bRegOffset;
								macreg_table[len].mask = u4bRegMask;
								macreg_table[len].value = u4bRegValue;
								len++;
								macreg_table[len].offset = 0Xffff;

								break;
							}
						}

					}
				}

				if (num > 0) {
					macreg_table[len].offset = u4bRegOffset;
					macreg_table[len].mask = u4bRegMask;
					macreg_table[len].value = u4bRegValue;
					len++;
					if ((GET_CHIP_VER(priv) == VERSION_8188E) || (GET_CHIP_VER(priv) == VERSION_8812E) || (IS_HAL_CHIP(priv))) {
						if (u4bRegOffset == 0xffff)
							break;
					} else if (CONFIG_WLAN_NOT_HAL_EXIST)
					{
						if (u4bRegOffset == 0xff)
							break;
					}
					if ((len * sizeof(struct MacRegTable)) > max_len)
						break;

					if ((len & 0x7ff) == 0)
						watchdog_kick();
				}
			}

			if (++round > 10000) {
				panic_printk("%s[%d] while (1) goes too many\n", __FUNCTION__, __LINE__);
				break;
			}
		}

#if 0//defined(CONFIG_WLAN_HAL_8814AE)
	if(GET_CHIP_VER(priv) == VERSION_8814A){
		int i;
		for(i=0;i<4;i++)
			printk("priv->pshare->phw->CCKTxAgc_A[%d]=%x\n",i,priv->pshare->phw->CCKTxAgc_A[i]);
		for(i=0;i<8;i++)
			printk("priv->pshare->phw->OFDMTxAgcOffset_A[%d]=%x\n",i,priv->pshare->phw->OFDMTxAgcOffset_A[i]);
		for(i=0;i<24;i++)
			printk("priv->pshare->phw->MCSTxAgcOffset_A[%d]=%x\n",i,priv->pshare->phw->MCSTxAgcOffset_A[i]);
		for(i=0;i<30;i++)
			printk("priv->pshare->phw->VHTTxAgcOffset_A[%d]=%x\n",i,priv->pshare->phw->VHTTxAgcOffset_A[i]);
		for(i=0;i<4;i++)
			printk("priv->pshare->phw->CCKTxAgc_B[%d]=%x\n",i,priv->pshare->phw->CCKTxAgc_B[i]);
		for(i=0;i<8;i++)
			printk("priv->pshare->phw->OFDMTxAgcOffset_B[%d]=%x\n",i,priv->pshare->phw->OFDMTxAgcOffset_B[i]);
		for(i=0;i<24;i++)
			printk("priv->pshare->phw->MCSTxAgcOffset_B[%d]=%x\n",i,priv->pshare->phw->MCSTxAgcOffset_B[i]);
		for(i=0;i<30;i++)
			printk("priv->pshare->phw->VHTTxAgcOffset_B[%d]=%x\n",i,priv->pshare->phw->VHTTxAgcOffset_B[i]);
		for(i=0;i<4;i++)
			printk("priv->pshare->phw->CCKTxAgc_C[%d]=%x\n",i,priv->pshare->phw->CCKTxAgc_C[i]);
		for(i=0;i<8;i++)
			printk("priv->pshare->phw->OFDMTxAgcOffset_C[%d]=%x\n",i,priv->pshare->phw->OFDMTxAgcOffset_C[i]);
		for(i=0;i<24;i++)
			printk("priv->pshare->phw->MCSTxAgcOffset_C[%d]=%x\n",i,priv->pshare->phw->MCSTxAgcOffset_C[i]);
		for(i=0;i<30;i++)
			printk("priv->pshare->phw->VHTTxAgcOffset_C[%d]=%x\n",i,priv->pshare->phw->VHTTxAgcOffset_C[i]);
		for(i=0;i<4;i++)
			printk("priv->pshare->phw->CCKTxAgc_D[%d]=%x\n",i,priv->pshare->phw->CCKTxAgc_D[i]);
		for(i=0;i<8;i++)
			printk("priv->pshare->phw->OFDMTxAgcOffset_D[%d]=%x\n",i,priv->pshare->phw->OFDMTxAgcOffset_D[i]);
		for(i=0;i<24;i++)
			printk("priv->pshare->phw->MCSTxAgcOffset_D[%d]=%x\n",i,priv->pshare->phw->MCSTxAgcOffset_D[i]);
		for(i=0;i<30;i++)
			printk("priv->pshare->phw->VHTTxAgcOffset_D[%d]=%x\n",i,priv->pshare->phw->VHTTxAgcOffset_D[i]);
	}
#endif
		kfree(mem_ptr);

		if ((len * sizeof(struct PhyRegTable)) > max_len) {
			printk("PHY REG table buffer not large enough!\n");
			printk("len=%d,sizeof(struct PhyRegTable)=%d,max_len=%u",len,sizeof(struct PhyRegTable),max_len);
			return -1;
		}
	}

	num = 0;
	round = 0;
	while (1) {
		if (file_format == THREE_COLUMN || file_format == FIVE_COLUMN || file_format == SEVEN_COLUMN) {
			u4bRegOffset = macreg_table[num].offset;
			u4bRegValue = macreg_table[num].value;
			u4bRegMask = macreg_table[num].mask;
		} else {
			u4bRegOffset = phyreg_table[num].offset;
			u4bRegValue = phyreg_table[num].value;
		}

		if ((GET_CHIP_VER(priv) == VERSION_8188E) || (GET_CHIP_VER(priv) == VERSION_8812E) || (IS_HAL_CHIP(priv))) {
			if (u4bRegOffset == 0xffff)
				break;
		} else if (CONFIG_WLAN_NOT_HAL_EXIST)
		{
			if (u4bRegOffset == 0xff)
				break;
		}


        if (GET_CHIP_VER(priv) == VERSION_8197F) {
    		if (reg_file == AGCTAB)
    		{
    			odm_UpdateAgcBigJumpLmt_8197F(ODMPTR, u4bRegOffset, u4bRegValue);
    		}
        }



		if (file_format == THREE_COLUMN || file_format == FIVE_COLUMN) {
			if (reg_file == PHYREG_PG && (
				(GET_CHIP_VER(priv) == VERSION_8192D)||(GET_CHIP_VER(priv) == VERSION_8188E) || (GET_CHIP_VER(priv) == VERSION_8812E) || (IS_HAL_CHIP(priv)) || (GET_CHIP_VER(priv) == VERSION_8723B)
					)) {

					if (u4bRegOffset == regstart) {
						if (idx == pg_tbl_idx)
							write_en = 1;
						idx++;
					}
					if (write_en
						&& (GET_CHIP_VER(priv) != VERSION_8197F) /* 97F doesn't set power by rate to Tx AGC registers */
						&& (GET_CHIP_VER(priv) != VERSION_8822B) /* 8822 needs not to set Tx AGC registers */

						) { //3 5 column PG set regs
						//panic_printk("0x%05x	0x%08x\n", u4bRegOffset, u4bRegValue);
						PHY_SetBBReg(priv, u4bRegOffset, u4bRegMask, u4bRegValue);
						if (u4bRegOffset == regend) {
							write_en = 0;
							break;
						}
					}

			} else
			{
				PHY_SetBBReg(priv, u4bRegOffset, u4bRegMask, u4bRegValue);
				//panic_printk("0x%05x	0x%08x\n", u4bRegOffset, u4bRegValue);
			}
		} else
			PHY_SetBBReg(priv, u4bRegOffset, bMaskDWord, u4bRegValue);
		if (reg_file == AGCTAB &&  priv->pshare->_dmODM.print_agc)
		{
			panic_printk("Reg %x [31:0]  =  %x \n", u4bRegOffset, u4bRegValue);
		}
		num++;

		if (++round > 10000) {
			panic_printk("%s[%d] while (1) goes too many\n", __FUNCTION__, __LINE__);
			break;
		}
	}

	return 0;
}


/*-----------------------------------------------------------------------------
 * Function:    PHY_ConfigRFWithParaFile()
 *
 * Overview:    This function read RF parameters from general file format, and do RF 3-wire
 *
 * Input:      	PADAPTER			Adapter
 *			ps1Byte 				pFileName
 *			RF92CD_RADIO_PATH_E	eRFPath
 *
 * Output:      NONE
 *
 * Return:      RT_STATUS_SUCCESS: configuration file exist
 *
 * Note:		Delay may be required for RF configuration
 *---------------------------------------------------------------------------*/
int PHY_ConfigRFWithParaFile(struct rtl8192cd_priv *priv,
							 unsigned char *start, int read_bytes,
							 RF92CD_RADIO_PATH_E eRFPath)
{
	int           num, round = 0, round1 = 0;
	unsigned int  u4bRegOffset, u4bRegValue;
	unsigned char *mem_ptr, *line_head, *next_head;

	if ((mem_ptr = (unsigned char *)kmalloc(MAX_CONFIG_FILE_SIZE, GFP_ATOMIC)) == NULL) {
		printk("PHY_ConfigRFWithParaFile(): not enough memory\n");
		return -1;
	}

	memset(mem_ptr, 0, MAX_CONFIG_FILE_SIZE); // clear memory
	memcpy(mem_ptr, start, read_bytes);

	next_head = mem_ptr;
	while (1) {
		line_head = next_head;
		next_head = get_line(&line_head);
		if (line_head == NULL)
			break;

		if (line_head[0] == '/'){
			if(find_str(line_head, "Parameter") || find_str(line_head, "version")){ /* Record PHY parameter version */
				char *ch = line_head+2;
				while (1) {
					if (*ch == '\n')
						break;
					else {
						printk("%c", *ch);
						ch++;
					}
					if (++round1 > 10000) {
						panic_printk("%s[%d] while (1) goes too many\n", __FUNCTION__, __LINE__);
						break;
					}
				}
				printk("\n");
			}
			continue;
		}

		num = get_offset_val(line_head, &u4bRegOffset, &u4bRegValue);
		if (num > 0) {
			if ((GET_CHIP_VER(priv) == VERSION_8188E) || (GET_CHIP_VER(priv) == VERSION_8812E) || (IS_HAL_CHIP(priv))) {
				if (u4bRegOffset == 0xffff) {
					break;
				} else if ((u4bRegOffset == 0xfe) || (u4bRegOffset == 0xffe)) {
					delay_ms(50);	// Delay 50 ms. Only RF configuration require delay
				} else if (num == 2) {
					PHY_SetRFReg(priv, eRFPath, u4bRegOffset, bMask20Bits, u4bRegValue);
					delay_ms(1);
				}
			} else if (CONFIG_WLAN_NOT_HAL_EXIST)
			{
				if (u4bRegOffset == 0xff) {
					break;
				} else if (u4bRegOffset == 0xfe) {
					delay_ms(50);	// Delay 50 ms. Only RF configuration require delay
				} else if (num == 2) {
					PHY_SetRFReg(priv, eRFPath, u4bRegOffset, bMask20Bits, u4bRegValue);
					delay_ms(1);
				}
			}
		}

		watchdog_kick();

		if (++round > 10000) {
			panic_printk("%s[%d] while (1) goes too many\n", __FUNCTION__, __LINE__);
			break;
		}
	}

	kfree(mem_ptr);

	return 0;
}

#if 1
int PHY_ConfigMACWithParaFile(struct rtl8192cd_priv *priv)
{
	int read_bytes, num, len = 0, round = 0, round1 = 0;
	unsigned int  u4bRegOffset, u4bRegValue;
	unsigned char *mem_ptr, *line_head, *next_head;
	struct PhyRegTable *reg_table = (struct PhyRegTable *)priv->pshare->mac_reg_buf;

	{
		if ((mem_ptr = (unsigned char *)kmalloc(MAX_CONFIG_FILE_SIZE, GFP_ATOMIC)) == NULL) {
			printk("PHY_ConfigMACWithParaFile(): not enough memory\n");
			return -1;
		}

		memset(mem_ptr, 0, MAX_CONFIG_FILE_SIZE); // clear memory
		if (IS_HAL_CHIP(priv)) {
			u8 *pMACRegStart;
			GET_HAL_INTERFACE(priv)->GetHwRegHandler(priv, HW_VAR_MACREGFILE_SIZE, (pu1Byte)&read_bytes);
			GET_HAL_INTERFACE(priv)->GetHwRegHandler(priv, HW_VAR_MACREGFILE_START, (pu1Byte)&pMACRegStart);
			memcpy(mem_ptr, pMACRegStart, read_bytes);
		}


		next_head = mem_ptr;
		while (1) {
			line_head = next_head;
			next_head = get_line(&line_head);
			if (line_head == NULL)
				break;

			if (line_head[0] == '/'){
				if(find_str(line_head, "Parameter") || find_str(line_head, "version")){ /* Record PHY parameter version */
					char *ch = line_head+2;
					while (1) {
						if (*ch == '\n')
							break;
						else {
							printk("%c", *ch);
							ch++;
						}

						if (++round1 > 10000) {
							panic_printk("%s[%d] while (1) goes too many\n", __FUNCTION__, __LINE__);
							break;
						}
					}
					printk("\n");
				}
				continue;
			}

			num = get_offset_val(line_head, &u4bRegOffset, &u4bRegValue);
			if (num > 0) {
				reg_table[len].offset = u4bRegOffset;
				reg_table[len].value = u4bRegValue;
				len++;
				if ((GET_CHIP_VER(priv) == VERSION_8188E) || (GET_CHIP_VER(priv) == VERSION_8812E) || (IS_HAL_CHIP(priv))) {
					if (u4bRegOffset == 0xffff)
						break;
				} else if (CONFIG_WLAN_NOT_HAL_EXIST)
				{
					if (u4bRegOffset == 0xff)
						break;
				}
				if ((len * sizeof(struct MacRegTable)) > MAC_REG_SIZE)
					break;
			}

			if (++round > 10000) {
				panic_printk("%s[%d] while (1) goes too many\n", __FUNCTION__, __LINE__);
				break;
			}
		}
		if ((GET_CHIP_VER(priv) == VERSION_8188E) || (GET_CHIP_VER(priv) == VERSION_8812E) || (IS_HAL_CHIP(priv)))
			reg_table[len].offset = 0xffff;
		else if (CONFIG_WLAN_NOT_HAL_EXIST)
			reg_table[len].offset = 0xff;

		kfree(mem_ptr);

		if ((len * sizeof(struct MacRegTable)) > MAC_REG_SIZE) {
			printk("MAC REG table buffer not large enough!\n");
			return -1;
		}
	}

	num = 0;
	round = 0;
	while (1) {
		u4bRegOffset = reg_table[num].offset;
		u4bRegValue = reg_table[num].value;

		if ((GET_CHIP_VER(priv) == VERSION_8188E) || (GET_CHIP_VER(priv) == VERSION_8812E) || (IS_HAL_CHIP(priv))) {
			if (u4bRegOffset == 0xffff)
				break;
		} else if (CONFIG_WLAN_NOT_HAL_EXIST)
		{
			if (u4bRegOffset == 0xff)
				break;
		}

		RTL_W8(u4bRegOffset, u4bRegValue);
		num++;

		if (++round > 10000) {
			panic_printk("%s[%d] while (1) goes too many\n", __FUNCTION__, __LINE__);
			break;
		}
	}

#if 0 //defined(CONFIG_RTL_8812_SUPPORT)
	if (GET_CHIP_VER(priv) == VERSION_8812E || GET_CHIP_VER(priv) == VERSION_8723B) {
		if (priv->pmib->dot11nConfigEntry.dot11nSTBC && priv->pmib->dot11nConfigEntry.dot11nLDPC)
			RTL_W16(REG_RESP_SIFS_OFDM_8812, 0x0c0c);
	}
#endif

	return 0;
}
#endif

static struct rtl8192cd_priv *get_another_interface_priv(struct rtl8192cd_priv *priv)
{
	if (IS_DRV_OPEN(GET_VXD_PRIV(priv)))
		return GET_VXD_PRIV(priv);
	else if (IS_DRV_OPEN(GET_ROOT(priv)))
		return GET_ROOT(priv);
	else
		return NULL;
}


static int get_shortslot_for_another_interface(struct rtl8192cd_priv *priv)
{
	struct rtl8192cd_priv *p_priv;

	p_priv = get_another_interface_priv(priv);
	if (p_priv) {
		if (p_priv->pmib->dot11OperationEntry.opmode & WIFI_AP_STATE)
			return (p_priv->pmib->dot11ErpInfo.shortSlot);
		else {
			if (p_priv->pmib->dot11OperationEntry.opmode & WIFI_ASOC_STATE)
				return (p_priv->pmib->dot11ErpInfo.shortSlot);
		}
	}
	return -1;
}


void set_slot_time(struct rtl8192cd_priv *priv, int use_short)
{
	int is_short;
	is_short = get_shortslot_for_another_interface(priv);
	if (is_short != -1) { // not abtained
		use_short &= is_short;
	}

	if (use_short)
		RTL_W8(SLOT_TIME, 0x09);
	else
		RTL_W8(SLOT_TIME, 0x14);
}

void SwChnl(struct rtl8192cd_priv *priv, unsigned char channel, int offset)
{
	unsigned int val = channel, eRFPath, curMaxRFPath;
	unsigned int is_need_reload_txpwr=0;
	int org_val, tmp_val;

		if ((GET_CHIP_VER(priv) == VERSION_8812E)||(GET_CHIP_VER(priv) == VERSION_8822B)||(GET_CHIP_VER(priv) == VERSION_8881A) || (GET_CHIP_VER(priv) == VERSION_8814A)) {
			if ((channel < 36) && (priv->pmib->dot11RFEntry.phyBandSelect==PHY_BAND_5G)) {
				GDEBUG("[8822B] Error Channel = %d , Force to Ch149 !!\n", channel);
				priv->pmib->dot11RFEntry.dot11channel = val = channel = 149;
			}
		}


		if(priv->pshare->CurrentChannelBW != HT_CHANNEL_WIDTH_20)
	    if(((channel < 5) && (offset == HT_2NDCH_OFFSET_BELOW)) ||
	     ((channel > 9) && (channel <=13) && (offset == HT_2NDCH_OFFSET_ABOVE))){
		panic_printk("Error Channel = %d Offset = %d, Force to 20M BW \n", channel, offset);

		offset = HT_2NDCH_OFFSET_DONTCARE;
		priv->pshare->offset_2nd_chan =  HT_2NDCH_OFFSET_DONTCARE;

		SwBWMode(priv, HT_CHANNEL_WIDTH_20, HT_2NDCH_OFFSET_DONTCARE);
	}



// TODO: 8814AE BB/RF
// TODO: after all BB/RF init ready, change (8881A || 8814AE) to (WLAN_HAL && 11AC_SUPPORT) ??
	if ((GET_CHIP_VER(priv) == VERSION_8881A) || (GET_CHIP_VER(priv) == VERSION_8814A) || (GET_CHIP_VER(priv) == VERSION_8822B)) {
		priv->pshare->No_RF_Write = 0;
		GET_HAL_INTERFACE(priv)->PHYUpdateBBRFValHandler(priv, channel, offset);
	}

#if defined(AC2G_256QAM)
	if(is_ac2g(priv)) //for 8812 & 8881a & 8194
	{
		RTL_W32(0x8c0, RTL_R32(0x8c0) & (~BIT(17)));	//enable tx vht rates
	}
#endif



		curMaxRFPath = RF92CD_PATH_MAX;

	if (channel > 14)
		priv->pshare->curr_band = BAND_5G;
	else
		priv->pshare->curr_band = BAND_2G;


//_TXPWR_REDEFINE ?? Working channel also apply to 5G ?? what if channel = 165 + 2 or 36 -2 ??
#if 0
#ifdef RTK_AC_SUPPORT //todo, find working channel for 80M
	if (priv->pshare->CurrentChannelBW == HT_CHANNEL_WIDTH_80) {
#ifdef AC2G_256QAM
		if(is_ac2g(priv))
			val = 7;		//in 2g band, only channel 7 can expand to 80M bw
		else
#endif
		if (channel <= 48)
			val = 42;
		else if (channel <= 64)
			val = 58;
		else if (channel <= 112)
			val = 106;
		else if (channel <= 128)
			val = 122;
		else if (channel <= 144)
			val = 138;
		else if (channel <= 161)
			val = 155;
		else if (channel <= 177)
			val = 171;
	} else
#endif
		if (priv->pshare->CurrentChannelBW == HT_CHANNEL_WIDTH_20_40) {
			if (offset == 1)
				val -= 2;
			else
				val += 2;
		}
#else
		val = get_center_channel(priv, channel, offset, 1);
#endif
		if (priv->pshare->rf_ft_var.use_frq_2_3G)
			val += 14;

	for (eRFPath = RF92CD_PATH_A; eRFPath < curMaxRFPath; eRFPath++)	{

		{
			PHY_SetRFReg(priv, eRFPath, rRfChannel, 0xff, val);
		}

	}


		if (priv->pshare->rf_ft_var.use_frq_2_3G)
			val -= 14;

		/*cfg p2p cfg p2p rm
		priv->pshare->working_channel = val;
		*/

#ifdef TXPWR_LMT
	{
		if (!priv->pshare->rf_ft_var.disable_txpwrlmt) {

			if ((GET_CHIP_VER(priv) == VERSION_8192C) || (GET_CHIP_VER(priv) == VERSION_8188E)) //92c_pwrlmt
			{
				//printk("reload_txpwr_pg\n");
				reload_txpwr_pg(priv);
			}

#ifdef TXPWR_LMT_NEWFILE
			if ((GET_CHIP_VER(priv) == VERSION_8812E) || (GET_CHIP_VER(priv) == VERSION_8188E) || IS_HAL_CHIP(priv))
				find_pwr_limit_new(priv, channel, offset);
			else if (CONFIG_WLAN_NOT_HAL_EXIST)
#endif
				find_pwr_limit(priv, channel, offset);

#ifdef BEAMFORMING_AUTO
			if(priv->pshare->rf_ft_var.txbf_pwrlmt == TXBF_TXPWRLMT_AUTO)
				BEAMFORMING_TXPWRLMT_Auto(priv);
#endif //BEAMFORMING_AUTO
		}
	}
#endif


/*cfg p2p cfg p2p*/
	priv->pshare->working_channel = val;
	priv->pshare->working_channel2 = channel;
	SetTxPowerLevel(priv, channel);


    if(GET_CHIP_VER(priv) == VERSION_8197F && _GET_HAL_DATA(priv)->cutVersion == ODM_CUT_A) {
		set_lck_cv(priv, val);
    }

	if (GET_CHIP_VER(priv) == VERSION_8197F) {
		if (channel == 13) {
			PHY_SetBBReg(priv, 0xd18, BIT(27), 1);
			PHY_SetBBReg(priv, 0xd2C, BIT(28), 1);
			if (PHY_QueryRFReg(priv,0,0x33,bMask20Bits,1) == 0x6000d)
				PHY_SetBBReg(priv, 0xd40, BIT(26), 0);
			else
				PHY_SetBBReg(priv, 0xd40, BIT(26), 1);
		} else {
			PHY_SetBBReg(priv, 0xd18, BIT(27), 0);
			PHY_SetBBReg(priv, 0xd2C, BIT(28), 0);
			PHY_SetBBReg(priv, 0xd40, BIT(26), 0);
		}
	}

#ifdef BT_COEXIST
	if(GET_CHIP_VER(priv) == VERSION_8192E && priv->pshare->rf_ft_var.btc == 1){
		extern int bt_state;
		if(bt_state){ /* BT enable, set channel for BT*/
			unsigned char H2CCommand[3]={0};
			unsigned int center_ch;
			if(priv->pshare->rf_ft_var.bt_dump)
				panic_printk("[%s] BT alive\n",__FUNCTION__);

			H2CCommand[0] = 0x01;
			center_ch = get_center_channel(priv, priv->pmib->dot11RFEntry.dot11channel, priv->pshare->offset_2nd_chan, 1);
			H2CCommand[1] = center_ch;
			if(priv->pshare->CurrentChannelBW == HT_CHANNEL_WIDTH_20_40){
				H2CCommand[2] = 0x30;
			} else {
				H2CCommand[2] = 0x20;
			}
			FillH2CCmd88XX(priv, 0x66, 3, H2CCommand);
		}
	}
#endif

#if 0
		if(!(OPMODE & WIFI_SITE_MONITOR)&& !(priv->ss_req_ongoing)
			 && !(OPMODE_VXD & WIFI_SITE_MONITOR) && !(GET_VXD_PRIV(priv)->ss_req_ongoing)
		){
			/*
panic_printk("     ++++ from SwChnl's ");  */
			PHY_IQCalibrate(priv);
		}
#endif
		if(!(OPMODE & WIFI_SITE_MONITOR)
			 && !(OPMODE_VXD & WIFI_SITE_MONITOR)
		){
			PHY_IQCalibrate(priv);
		}

		if(GET_CHIP_VER(priv) == VERSION_8192E || GET_CHIP_VER(priv) == VERSION_8197F || GET_CHIP_VER(priv) == VERSION_8188E || GET_CHIP_VER(priv) == VERSION_8723B || (GET_CHIP_VER(priv) == VERSION_8814A && priv->pmib->dot11RFEntry.phyBandSelect == PHY_BAND_2G)){
			if (priv->pmib->dot11RFEntry.phyBandSelect == PHY_BAND_2G
#ifdef HIGH_POWER_EXT_LNA
			&& !(GET_CHIP_VER(priv) == VERSION_8188E && priv->pshare->rf_ft_var.use_ext_lna)
#endif
			){

					if (GET_CHIP_VER(priv) == VERSION_8192E || GET_CHIP_VER(priv) == VERSION_8197F || (GET_CHIP_VER(priv) == VERSION_8814A && priv->pmib->dot11RFEntry.phyBandSelect == PHY_BAND_2G)) {
#ifdef HIGH_POWER_EXT_PA
						if (priv->pshare->rf_ft_var.use_ext_pa)
							priv->pshare->mp_cck_swing_idx = 30;
						else
#endif
							priv->pshare->mp_cck_swing_idx = 20;
					} else
					{
						priv->pshare->mp_cck_swing_idx = 12;
					}

				//printk("==> mp_ofdm_swing_idx=%d\n", priv->pshare->mp_ofdm_swing_idx);
				//printk("==> mp_cck_swing_idx=%d\n", priv->pshare->mp_cck_swing_idx);
				set_CCK_swing_index(priv, priv->pshare->mp_cck_swing_idx);
			}
		}

		if((GET_CHIP_VER(priv) == VERSION_8197F) && (!(OPMODE & WIFI_SITE_MONITOR)))		{
			if(priv->pshare->CurrentChannelBW == HT_CHANNEL_WIDTH_20_40)				{
				if(offset == HT_2NDCH_OFFSET_ABOVE)
					phydm_api_primary_cca(ODMPTR, 1);
				else if(offset == HT_2NDCH_OFFSET_BELOW)
					phydm_api_primary_cca(ODMPTR, 2);
			}
			else
			{
					phydm_api_primary_cca(ODMPTR, 0);
			}
		}

	return;
}

#ifdef MCR_WIRELESS_EXTEND
#endif
#ifdef EXPERIMENTAL_WIRELESS_EXTEND
// switch 1 spatial stream path
//antPath: 01 for PathA,10 for PathB, 11for Path AB
void Switch_1SS_Antenna(struct rtl8192cd_priv *priv, unsigned int antPath )
{
	unsigned int dword = 0;
	if (get_rf_mimo_mode(priv) != MIMO_2T2R)
		return;

	if(GET_CHIP_VER(priv) == VERSION_8812E) {
		switch (antPath) {
		case 1:
			dword = RTL_R32(0x80C);
			if ((dword & 0xf000) == 0x1000)
				goto switch_1ss_end;
			dword &= 0xffff0fff;
			dword |= 0x1000; // Path A
			RTL_W32(0x80C, dword);
			break;
		case 2:
			dword = RTL_R32(0x80C);
			if ((dword & 0xf000) == 0x2000)
				goto switch_1ss_end;
			dword &= 0xffff0fff;
			dword |= 0x2000; // Path B
			RTL_W32(0x80C, dword);
			break;

		case 3:
			if (priv->pshare->rf_ft_var.ofdm_1ss_oneAnt == 1) // use one ANT for 1ss
				goto switch_1ss_end;// do nothing
			dword = RTL_R32(0x80C);
			if ((dword & 0xf000) == 0x3000)
				goto switch_1ss_end;
			dword &= 0xffff0fff;
			dword |= 0x3000; // Path A, B
			RTL_W32(0x80C, dword);
			break;

		default:// do nothing
			break;
		}

	} else {

		switch (antPath) {
		case 1:
			dword = RTL_R32(0x90C);
			if ((dword & 0x0ff00000) == 0x01100000)
				goto switch_1ss_end;
			dword &= 0xf00fffff;
			dword |= 0x01100000; // Path A
			RTL_W32(0x90C, dword);
			break;
		case 2:
			dword = RTL_R32(0x90C);
			if ((dword & 0x0ff00000) == 0x02200000)
				goto switch_1ss_end;
			dword &= 0xf00fffff;
			dword |= 0x02200000;	// Path B
			RTL_W32(0x90C, dword);
			break;

		case 3:
			if (priv->pshare->rf_ft_var.ofdm_1ss_oneAnt == 1) // use one ANT for 1ss
				goto switch_1ss_end;// do nothing
			dword = RTL_R32(0x90C);
			if ((dword & 0x0ff00000) == 0x03300000)
				goto switch_1ss_end;
			dword &= 0xf00fffff;
			dword |= 0x03300000; // Path A,B
			RTL_W32(0x90C, dword);
			break;

		default:// do nothing
			break;
		}
	}
switch_1ss_end:
	return;

}

// switch OFDM path
//antPath: 01 for PathA,10 for PathB, 11for Path AB
void Switch_OFDM_Antenna(struct rtl8192cd_priv *priv, unsigned int antPath )
{
	unsigned int dword = 0;
	if (get_rf_mimo_mode(priv) != MIMO_2T2R)
		return;

	if(GET_CHIP_VER(priv) == VERSION_8812E) {

		switch (antPath) {
			case 1:
				dword = RTL_R32(0x80C);
				if ((dword & 0xf00) == 0x100)
					goto switch_OFDM_end;
				dword &= 0xfffff0ff;
				dword |= 0x100; // Path A
				RTL_W32(0x80C, dword);
				break;
			case 2:
				dword = RTL_R32(0x80C);
				if ((dword & 0xf00) == 0x200)
					goto switch_OFDM_end;
				dword &= 0xfffff0ff;
				dword |= 0x200; // Path B
				RTL_W32(0x80C, dword);
				break;

			case 3:
				if (priv->pshare->rf_ft_var.ofdm_1ss_oneAnt == 1) // use one ANT for 1ss
					goto switch_OFDM_end;// do nothing
				dword = RTL_R32(0x80C);
				if ((dword & 0xf00) == 0x300)
					goto switch_OFDM_end;
				dword &= 0xfffff0ff;
				dword |= 0x300; // Path A, B
				RTL_W32(0x80C, dword);
				break;

			default:// do nothing
				break;
			}

	} else {

		switch (antPath) {
			case 1:
				dword = RTL_R32(0x90C);
				if ((dword & 0x000000f0) == 0x00000010)
					goto switch_OFDM_end;
				dword &= 0xffffff0f;
				dword |= 0x00000010; // Path A
				RTL_W32(0x90C, dword);
				break;
			case 2:
				dword = RTL_R32(0x90C);
				if ((dword & 0x000000f0) == 0x00000020)
					goto switch_OFDM_end;
				dword &= 0xffffff0f;
				dword |= 0x00000020;	// Path B
				RTL_W32(0x90C, dword);
				break;

			case 3:
				if (priv->pshare->rf_ft_var.ofdm_1ss_oneAnt == 1) // use one ANT for 1ss
					goto switch_OFDM_end;// do nothing
				dword = RTL_R32(0x90C);
				if ((dword & 0x000000f0) == 0x00000030)
					goto switch_OFDM_end;
				dword &= 0xffffff0f;
				dword |= 0x00000030; // Path A,B
				RTL_W32(0x90C, dword);
				break;

			default:// do nothing
				break;
		}
	}
switch_OFDM_end:
	return;

}



#endif

void enable_hw_LED(struct rtl8192cd_priv *priv, unsigned int led_type)
{


	// TODO: we should check register then set
	if (IS_HAL_CHIP(priv))
		return;
	else if(CONFIG_WLAN_NOT_HAL_EXIST)
	{//not HAL
	switch (led_type) {
	case LEDTYPE_HW_TX_RX:
		break;
	case LEDTYPE_HW_LINKACT_INFRA:
		RTL_W32(LEDCFG, LED_TX_RX_EVENT_ON << LED0CM_SHIFT);
		if ((OPMODE & WIFI_AP_STATE) || (OPMODE & WIFI_STATION_STATE))
			RTL_W32(LEDCFG, RTL_R32(LEDCFG) & 0x0ff);
		else
			RTL_W32(LEDCFG, (RTL_R32(LEDCFG) & 0xfffff0ff) | LED1SV);
		break;
	default:
		break;
	}

	}
}


/**
* Function:	phy_InitBBRFRegisterDefinition
*
* OverView:	Initialize Register definition offset for Radio Path A/B/C/D
*
* Input:
*			PADAPTER		Adapter,
*
* Output:	None
* Return:		None
* Note:		The initialization value is constant and it should never be changes
*/
void phy_InitBBRFRegisterDefinition(struct rtl8192cd_priv *priv)
{
	struct rtl8192cd_hw *phw = GET_HW(priv);

	// RF Interface Sowrtware Control
	phw->PHYRegDef[RF92CD_PATH_A].rfintfs = rFPGA0_XAB_RFInterfaceSW; // 16 LSBs if read 32-bit from 0x870
	phw->PHYRegDef[RF92CD_PATH_B].rfintfs = rFPGA0_XAB_RFInterfaceSW; // 16 MSBs if read 32-bit from 0x870 (16-bit for 0x872)

	// RF Interface Readback Value
	phw->PHYRegDef[RF92CD_PATH_A].rfintfi = rFPGA0_XAB_RFInterfaceRB; // 16 LSBs if read 32-bit from 0x8E0
	phw->PHYRegDef[RF92CD_PATH_B].rfintfi = rFPGA0_XAB_RFInterfaceRB;// 16 MSBs if read 32-bit from 0x8E0 (16-bit for 0x8E2)

	// RF Interface Output (and Enable)
	phw->PHYRegDef[RF92CD_PATH_A].rfintfo = rFPGA0_XA_RFInterfaceOE; // 16 LSBs if read 32-bit from 0x860
	phw->PHYRegDef[RF92CD_PATH_B].rfintfo = rFPGA0_XB_RFInterfaceOE; // 16 LSBs if read 32-bit from 0x864

	// RF Interface (Output and)  Enable
	phw->PHYRegDef[RF92CD_PATH_A].rfintfe = rFPGA0_XA_RFInterfaceOE; // 16 MSBs if read 32-bit from 0x860 (16-bit for 0x862)
	phw->PHYRegDef[RF92CD_PATH_B].rfintfe = rFPGA0_XB_RFInterfaceOE; // 16 MSBs if read 32-bit from 0x864 (16-bit for 0x866)

	//Addr of LSSI. Wirte RF register by driver
	phw->PHYRegDef[RF92CD_PATH_A].rf3wireOffset = rFPGA0_XA_LSSIParameter; //LSSI Parameter
	phw->PHYRegDef[RF92CD_PATH_B].rf3wireOffset = rFPGA0_XB_LSSIParameter;

	// RF parameter
	phw->PHYRegDef[RF92CD_PATH_A].rfLSSI_Select = rFPGA0_XAB_RFParameter;  //BB Band Select
	phw->PHYRegDef[RF92CD_PATH_B].rfLSSI_Select = rFPGA0_XAB_RFParameter;

	// Tx AGC Gain Stage (same for all path. Should we remove this?)
	phw->PHYRegDef[RF92CD_PATH_A].rfTxGainStage = rFPGA0_TxGainStage; //Tx gain stage
	phw->PHYRegDef[RF92CD_PATH_B].rfTxGainStage = rFPGA0_TxGainStage; //Tx gain stage

	// Tranceiver A~D HSSI Parameter-1
	phw->PHYRegDef[RF92CD_PATH_A].rfHSSIPara1 = rFPGA0_XA_HSSIParameter1;  //wire control parameter1
	phw->PHYRegDef[RF92CD_PATH_B].rfHSSIPara1 = rFPGA0_XB_HSSIParameter1;  //wire control parameter1

	// Tranceiver A~D HSSI Parameter-2
	phw->PHYRegDef[RF92CD_PATH_A].rfHSSIPara2 = rFPGA0_XA_HSSIParameter2;  //wire control parameter2
	phw->PHYRegDef[RF92CD_PATH_B].rfHSSIPara2 = rFPGA0_XB_HSSIParameter2;  //wire control parameter2

	// RF switch Control
	phw->PHYRegDef[RF92CD_PATH_A].rfSwitchControl = rFPGA0_XAB_SwitchControl; //TR/Ant switch control
	phw->PHYRegDef[RF92CD_PATH_B].rfSwitchControl = rFPGA0_XAB_SwitchControl;

	// AGC control 1
	phw->PHYRegDef[RF92CD_PATH_A].rfAGCControl1 = rOFDM0_XAAGCCore1;
	phw->PHYRegDef[RF92CD_PATH_B].rfAGCControl1 = rOFDM0_XBAGCCore1;

	// AGC control 2
	phw->PHYRegDef[RF92CD_PATH_A].rfAGCControl2 = rOFDM0_XAAGCCore2;
	phw->PHYRegDef[RF92CD_PATH_B].rfAGCControl2 = rOFDM0_XBAGCCore2;

	// RX AFE control 1
	phw->PHYRegDef[RF92CD_PATH_A].rfRxIQImbalance = rOFDM0_XARxIQImbalance;
	phw->PHYRegDef[RF92CD_PATH_B].rfRxIQImbalance = rOFDM0_XBRxIQImbalance;

	// RX AFE control 1
	phw->PHYRegDef[RF92CD_PATH_A].rfRxAFE = rOFDM0_XARxAFE;
	phw->PHYRegDef[RF92CD_PATH_B].rfRxAFE = rOFDM0_XBRxAFE;

	// Tx AFE control 1
	phw->PHYRegDef[RF92CD_PATH_A].rfTxIQImbalance = rOFDM0_XATxIQImbalance;
	phw->PHYRegDef[RF92CD_PATH_B].rfTxIQImbalance = rOFDM0_XBTxIQImbalance;

	// Tx AFE control 2
	phw->PHYRegDef[RF92CD_PATH_A].rfTxAFE = rOFDM0_XATxAFE;
	phw->PHYRegDef[RF92CD_PATH_B].rfTxAFE = rOFDM0_XBTxAFE;

	// Tranceiver LSSI Readback SI mode
	phw->PHYRegDef[RF92CD_PATH_A].rfLSSIReadBack = rFPGA0_XA_LSSIReadBack;
	phw->PHYRegDef[RF92CD_PATH_B].rfLSSIReadBack = rFPGA0_XB_LSSIReadBack;

	// Tranceiver LSSI Readback PI mode
	phw->PHYRegDef[RF92CD_PATH_A].rfLSSIReadBackPi = TransceiverA_HSPI_Readback;
	phw->PHYRegDef[RF92CD_PATH_B].rfLSSIReadBackPi = TransceiverB_HSPI_Readback;
}


void check_chipID_MIMO(struct rtl8192cd_priv *priv)
{
	if (GET_CHIP_VER(priv) == VERSION_8881A)	{
		priv->pshare->version_id |= (RTL_R16(0xf0)>>8) & 0x0f0;		// save b[15:12]
		goto exit_func;
	}else if (GET_CHIP_VER(priv) == VERSION_8192E)	{
		priv->pshare->version_id |= (RTL_R16(0xf0)>>8) & 0x0f0;		// 92E c-cut
		goto exit_func;
	}else if (GET_CHIP_VER(priv) == VERSION_8814A)	{
		u8 ChipCut      = _GET_HAL_DATA(priv)->cutVersion & 0xF;
        u8 bTestChip    = _GET_HAL_DATA(priv)->bTestChip;

        priv->pshare->version_id |= (ChipCut<<4);
        if (bTestChip) {
            priv->pshare->version_id |= 0x100;
        }

        goto exit_func;
	}




exit_func:
	return;
}

void SelectTXPowerOffset(struct rtl8192cd_priv *priv)
{

	unsigned char isMinus11db = 0;


#if 0//defined(CONFIG_WLAN_HAL_8814AE)
	if(GET_CHIP_VER(priv) == VERSION_8814A &&
		(priv->pmib->dot11RFEntry.rfe_type == 3 || priv->pmib->dot11RFEntry.rfe_type == 5 )) // hp board
		isMinus11db = 1;
#endif

	if (isMinus11db && priv->pshare->rf_ft_var.min_pwr_idex >= 22) {
		priv->pshare->phw->TXPowerOffset = 3; // -11 dB
	} else if(priv->pshare->rf_ft_var.min_pwr_idex >= 14){
		priv->pshare->phw->TXPowerOffset = 2; // -7 dB
	} else if(priv->pshare->rf_ft_var.min_pwr_idex >= 6){
		priv->pshare->phw->TXPowerOffset = 1; // -3 dB
	} else {
		priv->pshare->phw->TXPowerOffset = 0; // 0 dB
	}
}

void selectMinPowerIdex(struct rtl8192cd_priv *priv)
{
	int i = 0, idx, pwr_min = 0xff;
	unsigned int val32;
	unsigned char val;

	if (GET_CHIP_VER(priv) == VERSION_8188E || GET_CHIP_VER(priv) == VERSION_8192C || GET_CHIP_VER(priv) == VERSION_8192D || GET_CHIP_VER(priv) == VERSION_8192E || GET_CHIP_VER(priv) == VERSION_8197F) {
		unsigned int pwr_regA[] = {0xe00, 0xe04, 0xe08, 0x86c, 0xe10, 0xe14, 0xe18, 0xe1c};
		unsigned int pwr_regB[] = {0x830, 0x834, 0x838, 0x86c, 0x83c, 0x848, 0x84c, 0x868}; // For 92C,92D

		for (idx = 0 ; idx < 8 ; idx++) {
			val32 = RTL_R32(pwr_regA[idx]);
			switch (pwr_regA[idx]) {
			case 0xe08:
				pwr_min = POWER_MIN_CHECK(pwr_min, (val32 >> 8) & 0xff);
				break;

			case 0x86c:
				for (i = 8 ; i < 32 ; i += 8)
					pwr_min = POWER_MIN_CHECK(pwr_min, (val32 >> i) & 0xff);
				break;

			default:
				for (i = 0 ; i < 32 ; i += 8)
					pwr_min = POWER_MIN_CHECK(pwr_min, (val32 >> i) & 0xff);
				break;
			}
		}

		if(get_rf_mimo_mode(priv) == MIMO_2T2R)
		{
			for (idx = 0 ; idx < 8 ; idx++) {
				val32 = RTL_R32(pwr_regB[idx]);
				switch (pwr_regB[idx]) {
				case 0x86c:
					pwr_min = POWER_MIN_CHECK(pwr_min, val32 & 0xff);
					break;

				case 0x838:
					for (i = 8 ; i < 32 ; i += 8)
						pwr_min = POWER_MIN_CHECK(pwr_min, (val32 >> i) & 0xff);
					break;

				default:
					for (i = 0 ; i < 32 ; i += 8)
						pwr_min = POWER_MIN_CHECK(pwr_min, (val32 >> i) & 0xff);
					break;
				}
			}
		}
	//#endif
	}


	if (GET_CHIP_VER(priv) == VERSION_8822B) {
		unsigned int path,i;
		for(path=0;path<2;path++) {
			if(priv->pshare->phw->CurrentTxAgcCCK[path][3]!=0 &&
				priv->pshare->phw->CurrentTxAgcCCK[path][3] < pwr_min)
				pwr_min = priv->pshare->phw->CurrentTxAgcCCK[path][3];

			if(priv->pshare->phw->CurrentTxAgcOFDM[path][7]!=0 &&
				priv->pshare->phw->CurrentTxAgcOFDM[path][7] < pwr_min)
				pwr_min = priv->pshare->phw->CurrentTxAgcOFDM[path][7];

			if(priv->pshare->phw->CurrentTxAgcMCS[path][15]!=0 &&
				priv->pshare->phw->CurrentTxAgcMCS[path][15] < pwr_min)
				pwr_min = priv->pshare->phw->CurrentTxAgcMCS[path][15];

			if(priv->pshare->phw->CurrentTxAgcVHT[path][19]!=0 &&
				priv->pshare->phw->CurrentTxAgcVHT[path][19] < pwr_min)
				pwr_min = priv->pshare->phw->CurrentTxAgcVHT[path][19];
		}
	}



	priv->pshare->rf_ft_var.min_pwr_idex = pwr_min;
	SelectTXPowerOffset(priv);

}

#ifdef POWER_PERCENT_ADJUSTMENT
s1Byte PwrPercent2PwrLevel(int percentage)
{
#define ARRAYSIZE(x)	(sizeof(x)/sizeof((x)[0]))

	const int percent_threshold[] = {95, 85, 75, 67, 60, 54, 48, 43, 38, 34, 30, 27, 24, 22, 19, 17, 15, 14, 12, 11, 10};
	const s1Byte pwrlevel_diff[9] = { -40, -34, -30, -28, -26, -24, -23, -22, -21};	// for < 10% case
	int i;

	for (i = 0; i < ARRAYSIZE(percent_threshold); ++i) {
		if (percentage >= percent_threshold[i]) {
			return (s1Byte) - i;
		}
	}

	if (percentage < 1) percentage = 1;

	return pwrlevel_diff[percentage - 1];
}
#endif

void PHY_RF6052SetOFDMTxPower(struct rtl8192cd_priv *priv, unsigned int channel)
{
	unsigned int writeVal, defValue = 0x28 ;
	unsigned char  offset;
	unsigned char base, byte0, byte1, byte2, byte3;
	unsigned char pwrlevelHT40_1S_A = priv->pmib->dot11RFEntry.pwrlevelHT40_1S_A[channel - 1];
	unsigned char pwrlevelHT40_1S_B = priv->pmib->dot11RFEntry.pwrlevelHT40_1S_B[channel - 1];
	unsigned char pwrdiffHT40_2S = priv->pmib->dot11RFEntry.pwrdiffHT40_2S[channel - 1];
	unsigned char pwrdiffHT20 = priv->pmib->dot11RFEntry.pwrdiffHT20[channel - 1];
	unsigned char pwrdiffOFDM = priv->pmib->dot11RFEntry.pwrdiffOFDM[channel - 1];
#ifdef USB_POWER_SUPPORT
//_TXPWR_REDEFINE
	unsigned char pwrlevelHT40_6dBm_1S_A;
	unsigned char pwrlevelHT40_6dBm_1S_B;
	unsigned char pwrdiffHT40_6dBm_2S;
	unsigned char pwrdiffHT20_6dBm;
	unsigned char pwrdiffOFDM_6dBm;
	unsigned char offset_6dBm;
	s1Byte base_6dBm;
#endif
#ifdef POWER_PERCENT_ADJUSTMENT
	s1Byte pwrdiff_percent = PwrPercent2PwrLevel(priv->pmib->dot11RFEntry.power_percent);
#endif





#ifdef TXPWR_LMT
#ifdef TXPWR_LMT_88E
	if(GET_CHIP_VER(priv) == VERSION_8188E) {
		if(!priv->pshare->rf_ft_var.disable_txpwrlmt){
			int i;
			int max_idx_a, max_idx_b =0;

			if (!priv->pshare->txpwr_lmt_OFDM || !priv->pshare->tgpwr_OFDM_new[RF_PATH_A]) {
				//printk("No limit for OFDM TxPower\n");
				max_idx_a = 255;
				max_idx_b = 255;
			} else {
				// maximum additional power index
				max_idx_a = (priv->pshare->txpwr_lmt_OFDM - priv->pshare->tgpwr_OFDM_new[RF_PATH_A]);
				max_idx_b = (priv->pshare->txpwr_lmt_OFDM - priv->pshare->tgpwr_OFDM_new[RF_PATH_B]);
			}

			for (i = 0; i <= 7; i++) {
				//printk("priv->pshare->phw->OFDMTxAgcOffset_A[%d]=0x%x max=0x%x\n",i, priv->pshare->phw->OFDMTxAgcOffset_A[i], max_idx_a);
				priv->pshare->phw->OFDMTxAgcOffset_A[i] = POWER_MIN_CHECK(priv->pshare->phw->OFDMTxAgcOffset_A[i], max_idx_a);
				priv->pshare->phw->OFDMTxAgcOffset_B[i] = POWER_MIN_CHECK(priv->pshare->phw->OFDMTxAgcOffset_B[i], max_idx_b);
				//printk("priv->pshare->phw->OFDMTxAgcOffset_B[%d]=%x\n",i, priv->pshare->phw->OFDMTxAgcOffset_B[i]);
			}

			if (!priv->pshare->txpwr_lmt_HT1S || !priv->pshare->tgpwr_HT1S_new[RF_PATH_A]) {
				//printk("No limit for HT1S TxPower\n");
				max_idx_a = 255;
				max_idx_b = 255;
			} else {
				// maximum additional power index
				max_idx_a = (priv->pshare->txpwr_lmt_HT1S - priv->pshare->tgpwr_HT1S_new[RF_PATH_A]);
				max_idx_b = (priv->pshare->txpwr_lmt_HT1S - priv->pshare->tgpwr_HT1S_new[RF_PATH_B]);
			}

			for (i = 0; i <= 7; i++) {
				//printk("priv->pshare->phw->MCSTxAgcOffset_A[%d]=0x%x max=0x%x\n",i, priv->pshare->phw->MCSTxAgcOffset_A[i], max_idx_a);
				priv->pshare->phw->MCSTxAgcOffset_A[i] = POWER_MIN_CHECK(priv->pshare->phw->MCSTxAgcOffset_A[i], max_idx_a);
				priv->pshare->phw->MCSTxAgcOffset_B[i] = POWER_MIN_CHECK(priv->pshare->phw->MCSTxAgcOffset_B[i], max_idx_b);
				//printk("priv->pshare->phw->MCSTxAgcOffset_B[%d]=%x\n",i, priv->pshare->phw->MCSTxAgcOffset_B[i]);
			}

			if (!priv->pshare->txpwr_lmt_HT2S || !priv->pshare->tgpwr_HT2S_new[RF_PATH_A]) {
				//printk("No limit for HT2S TxPower\n");
				max_idx_a = 255;
				max_idx_b = 255;
			} else {
				// maximum additional power index
				max_idx_a = (priv->pshare->txpwr_lmt_HT2S - priv->pshare->tgpwr_HT2S_new[RF_PATH_A]);
				max_idx_b = (priv->pshare->txpwr_lmt_HT2S - priv->pshare->tgpwr_HT2S_new[RF_PATH_B]);
			}

			for (i = 8; i <= 15; i++) {
				//printk("priv->pshare->phw->MCSTxAgcOffset_A[%d]=%x max=0x%x\n",i, priv->pshare->phw->MCSTxAgcOffset_A[i], max_idx_a);
				priv->pshare->phw->MCSTxAgcOffset_A[i] = POWER_MIN_CHECK(priv->pshare->phw->MCSTxAgcOffset_A[i], max_idx_a);
				priv->pshare->phw->MCSTxAgcOffset_B[i] = POWER_MIN_CHECK(priv->pshare->phw->MCSTxAgcOffset_B[i], max_idx_b);
				//printk("priv->pshare->phw->MCSTxAgcOffset_B[%d]=%x\n",i, priv->pshare->phw->MCSTxAgcOffset_B[i]);
			}
		}
	}
	else
#endif
	if ((GET_CHIP_VER(priv) == VERSION_8192D) || (GET_CHIP_VER(priv) == VERSION_8192C) || (GET_CHIP_VER(priv) == VERSION_8188C))
		if (!priv->pshare->rf_ft_var.disable_txpwrlmt) {
			int i;
			int max_idx;

			if (!priv->pshare->txpwr_lmt_OFDM || !priv->pshare->tgpwr_OFDM) {
				//printk("No limit for OFDM TxPower\n");
				max_idx = 255;
			} else {
				// maximum additional power index
				max_idx = (priv->pshare->txpwr_lmt_OFDM - priv->pshare->tgpwr_OFDM);
			}

			for (i = 0; i <= 7; i++) {
				priv->pshare->phw->OFDMTxAgcOffset_A[i] = POWER_MIN_CHECK(priv->pshare->phw->OFDMTxAgcOffset_A[i], max_idx);
				priv->pshare->phw->OFDMTxAgcOffset_B[i] = POWER_MIN_CHECK(priv->pshare->phw->OFDMTxAgcOffset_B[i], max_idx);
				//printk("priv->pshare->phw->OFDMTxAgcOffset_A[%d]=%x\n",i, priv->pshare->phw->OFDMTxAgcOffset_A[i]);
				//printk("priv->pshare->phw->OFDMTxAgcOffset_B[%d]=%x\n",i, priv->pshare->phw->OFDMTxAgcOffset_B[i]);
			}

			if (!priv->pshare->txpwr_lmt_HT1S || !priv->pshare->tgpwr_HT1S) {
				//printk("No limit for HT1S TxPower\n");
				max_idx = 255;
			} else {
				// maximum additional power index
				max_idx = (priv->pshare->txpwr_lmt_HT1S - priv->pshare->tgpwr_HT1S);
			}

			for (i = 0; i <= 7; i++) {
				priv->pshare->phw->MCSTxAgcOffset_A[i] = POWER_MIN_CHECK(priv->pshare->phw->MCSTxAgcOffset_A[i], max_idx);
				priv->pshare->phw->MCSTxAgcOffset_B[i] = POWER_MIN_CHECK(priv->pshare->phw->MCSTxAgcOffset_B[i], max_idx);
				//printk("priv->pshare->phw->MCSTxAgcOffset_A[%d]=%x\n",i, priv->pshare->phw->MCSTxAgcOffset_A[i]);
				//printk("priv->pshare->phw->MCSTxAgcOffset_B[%d]=%x\n",i, priv->pshare->phw->MCSTxAgcOffset_B[i]);
			}

			if (!priv->pshare->txpwr_lmt_HT2S || !priv->pshare->tgpwr_HT2S) {
				//printk("No limit for HT2S TxPower\n");
				max_idx = 255;
			} else {
				// maximum additional power index
				max_idx = (priv->pshare->txpwr_lmt_HT2S - priv->pshare->tgpwr_HT2S);
			}

			for (i = 8; i <= 15; i++) {
				priv->pshare->phw->MCSTxAgcOffset_A[i] = POWER_MIN_CHECK(priv->pshare->phw->MCSTxAgcOffset_A[i], max_idx);
				priv->pshare->phw->MCSTxAgcOffset_B[i] = POWER_MIN_CHECK(priv->pshare->phw->MCSTxAgcOffset_B[i], max_idx);
				//printk("priv->pshare->phw->MCSTxAgcOffset_A[%d]=%x\n",i, priv->pshare->phw->MCSTxAgcOffset_A[i]);
				//printk("priv->pshare->phw->MCSTxAgcOffset_B[%d]=%x\n",i, priv->pshare->phw->MCSTxAgcOffset_B[i]);
			}
		}
#endif


	if ((pwrlevelHT40_1S_A == 0)
	   )

	{
		// use default value

#ifdef HIGH_POWER_EXT_PA
		if(!(priv->pshare->rf_ft_var.mp_specific) && !(OPMODE & WIFI_MP_STATE))
			if (priv->pshare->rf_ft_var.use_ext_pa)
				defValue = HP_OFDM_POWER_DEFAULT ;
#endif
#ifndef ADD_TX_POWER_BY_CMD
		writeVal = (defValue << 24) | (defValue << 16) | (defValue << 8) | (defValue);
		RTL_W32(rTxAGC_A_Rate18_06, writeVal);
		RTL_W32(rTxAGC_A_Rate54_24, writeVal);
		RTL_W32(rTxAGC_A_Mcs03_Mcs00, writeVal);
		RTL_W32(rTxAGC_A_Mcs07_Mcs04, writeVal);
		RTL_W32(rTxAGC_B_Rate18_06, writeVal);
		RTL_W32(rTxAGC_B_Rate54_24, writeVal);
		RTL_W32(rTxAGC_B_Mcs03_Mcs00, writeVal);
		RTL_W32(rTxAGC_B_Mcs07_Mcs04, writeVal);

#ifdef USB_POWER_SUPPORT
//_TXPWR_REDEFINE, pwrlevelHT40_1S_A == 0 >> No 6dBm Power >> default value >> so USB = def - 14
		writeVal = POWER_RANGE_CHECK(defValue - USB_HT_2S_DIFF);
		writeVal |= (writeVal << 24) | (writeVal << 16) | (writeVal << 8);
#endif
		RTL_W32(rTxAGC_A_Mcs11_Mcs08, writeVal);
		RTL_W32(rTxAGC_A_Mcs15_Mcs12, writeVal);
		RTL_W32(rTxAGC_B_Mcs11_Mcs08, writeVal);
		RTL_W32(rTxAGC_B_Mcs15_Mcs12, writeVal);
#else
		base = defValue;
		byte0 = byte1 = byte2 = byte3 = 0;
		ASSIGN_TX_POWER_OFFSET(byte0, priv->pshare->rf_ft_var.txPowerPlus_ofdm_18);
		ASSIGN_TX_POWER_OFFSET(byte1, priv->pshare->rf_ft_var.txPowerPlus_ofdm_12);
		ASSIGN_TX_POWER_OFFSET(byte2, priv->pshare->rf_ft_var.txPowerPlus_ofdm_9);
		ASSIGN_TX_POWER_OFFSET(byte3, priv->pshare->rf_ft_var.txPowerPlus_ofdm_6);

		byte0 = POWER_RANGE_CHECK(base + byte0);
		byte1 = POWER_RANGE_CHECK(base + byte1);
		byte2 = POWER_RANGE_CHECK(base + byte2);
		byte3 = POWER_RANGE_CHECK(base + byte3);
		writeVal = (byte0 << 24) | (byte1 << 16) | (byte2 << 8) | byte3;
		RTL_W32(rTxAGC_A_Rate18_06, writeVal);
		RTL_W32(rTxAGC_B_Rate18_06, writeVal);

		byte0 = byte1 = byte2 = byte3 = 0;
		ASSIGN_TX_POWER_OFFSET(byte0, priv->pshare->rf_ft_var.txPowerPlus_ofdm_54);
		ASSIGN_TX_POWER_OFFSET(byte1, priv->pshare->rf_ft_var.txPowerPlus_ofdm_48);
		ASSIGN_TX_POWER_OFFSET(byte2, priv->pshare->rf_ft_var.txPowerPlus_ofdm_36);
		ASSIGN_TX_POWER_OFFSET(byte3, priv->pshare->rf_ft_var.txPowerPlus_ofdm_24);
		byte0 = POWER_RANGE_CHECK(base + byte0);
		byte1 = POWER_RANGE_CHECK(base + byte1);
		byte2 = POWER_RANGE_CHECK(base + byte2);
		byte3 = POWER_RANGE_CHECK(base + byte3);
		writeVal = (byte0 << 24) | (byte1 << 16) | (byte2 << 8) | byte3;
		RTL_W32(rTxAGC_A_Rate54_24, writeVal);
		RTL_W32(rTxAGC_B_Rate54_24, writeVal);

		byte0 = byte1 = byte2 = byte3 = 0;
		ASSIGN_TX_POWER_OFFSET(byte0, priv->pshare->rf_ft_var.txPowerPlus_mcs_3);
		ASSIGN_TX_POWER_OFFSET(byte1, priv->pshare->rf_ft_var.txPowerPlus_mcs_2);
		ASSIGN_TX_POWER_OFFSET(byte2, priv->pshare->rf_ft_var.txPowerPlus_mcs_1);
		ASSIGN_TX_POWER_OFFSET(byte3, priv->pshare->rf_ft_var.txPowerPlus_mcs_0);
		byte0 = POWER_RANGE_CHECK(base + byte0);
		byte1 = POWER_RANGE_CHECK(base + byte1);
		byte2 = POWER_RANGE_CHECK(base + byte2);
		byte3 = POWER_RANGE_CHECK(base + byte3);
		writeVal = (byte0 << 24) | (byte1 << 16) | (byte2 << 8) | byte3;
		RTL_W32(rTxAGC_A_Mcs03_Mcs00, writeVal);
		RTL_W32(rTxAGC_B_Mcs03_Mcs00, writeVal);

		byte0 = byte1 = byte2 = byte3 = 0;
		ASSIGN_TX_POWER_OFFSET(byte0, priv->pshare->rf_ft_var.txPowerPlus_mcs_7);
		ASSIGN_TX_POWER_OFFSET(byte1, priv->pshare->rf_ft_var.txPowerPlus_mcs_6);
		ASSIGN_TX_POWER_OFFSET(byte2, priv->pshare->rf_ft_var.txPowerPlus_mcs_5);
		ASSIGN_TX_POWER_OFFSET(byte3, priv->pshare->rf_ft_var.txPowerPlus_mcs_4);
		byte0 = POWER_RANGE_CHECK(base + byte0);
		byte1 = POWER_RANGE_CHECK(base + byte1);
		byte2 = POWER_RANGE_CHECK(base + byte2);
		byte3 = POWER_RANGE_CHECK(base + byte3);
		writeVal = (byte0 << 24) | (byte1 << 16) | (byte2 << 8) | byte3;
		RTL_W32(rTxAGC_A_Mcs07_Mcs04, writeVal);
		RTL_W32(rTxAGC_B_Mcs07_Mcs04, writeVal);

//_TXPWR_REDEFINE
#ifdef USB_POWER_SUPPORT
		byte0 = byte1 = byte2 = byte3 = -USB_HT_2S_DIFF;
#else
		byte0 = byte1 = byte2 = byte3 = 0;
		ASSIGN_TX_POWER_OFFSET(byte0, priv->pshare->rf_ft_var.txPowerPlus_mcs_11);
		ASSIGN_TX_POWER_OFFSET(byte1, priv->pshare->rf_ft_var.txPowerPlus_mcs_10);
		ASSIGN_TX_POWER_OFFSET(byte2, priv->pshare->rf_ft_var.txPowerPlus_mcs_9);
		ASSIGN_TX_POWER_OFFSET(byte3, priv->pshare->rf_ft_var.txPowerPlus_mcs_8);
#endif

		byte0 = POWER_RANGE_CHECK(base + byte0);
		byte1 = POWER_RANGE_CHECK(base + byte1);
		byte2 = POWER_RANGE_CHECK(base + byte2);
		byte3 = POWER_RANGE_CHECK(base + byte3);
		writeVal = (byte0 << 24) | (byte1 << 16) | (byte2 << 8) | byte3;
		RTL_W32(rTxAGC_A_Mcs11_Mcs08, writeVal);
		RTL_W32(rTxAGC_B_Mcs11_Mcs08, writeVal);

//_TXPWR_REDEFINE
#ifdef USB_POWER_SUPPORT
		byte0 = byte1 = byte2 = byte3 = -USB_HT_2S_DIFF;
#else
		byte0 = byte1 = byte2 = byte3 = 0;
		ASSIGN_TX_POWER_OFFSET(byte0, priv->pshare->rf_ft_var.txPowerPlus_mcs_15);
		ASSIGN_TX_POWER_OFFSET(byte1, priv->pshare->rf_ft_var.txPowerPlus_mcs_14);
		ASSIGN_TX_POWER_OFFSET(byte2, priv->pshare->rf_ft_var.txPowerPlus_mcs_13);
		ASSIGN_TX_POWER_OFFSET(byte3, priv->pshare->rf_ft_var.txPowerPlus_mcs_12);
#endif

		byte0 = POWER_RANGE_CHECK(base + byte0);
		byte1 = POWER_RANGE_CHECK(base + byte1);
		byte2 = POWER_RANGE_CHECK(base + byte2);
		byte3 = POWER_RANGE_CHECK(base + byte3);
		writeVal = (byte0 << 24) | (byte1 << 16) | (byte2 << 8) | byte3;
		RTL_W32(rTxAGC_A_Mcs15_Mcs12, writeVal);
		RTL_W32(rTxAGC_B_Mcs15_Mcs12, writeVal);

#endif // ADD_TX_POWER_BY_CMD
		return; // use default
	}

	/******************************  PATH A  ******************************/
	base = pwrlevelHT40_1S_A;
	offset = (pwrdiffOFDM & 0x0f);
	base = COUNT_SIGN_OFFSET(base, offset);
#ifdef POWER_PERCENT_ADJUSTMENT
	base = POWER_RANGE_CHECK(base + pwrdiff_percent);
#endif


	byte0 = POWER_RANGE_CHECK(base + priv->pshare->phw->OFDMTxAgcOffset_A[0]);
	byte1 = POWER_RANGE_CHECK(base + priv->pshare->phw->OFDMTxAgcOffset_A[1]);
	byte2 = POWER_RANGE_CHECK(base + priv->pshare->phw->OFDMTxAgcOffset_A[2]);
	byte3 = POWER_RANGE_CHECK(base + priv->pshare->phw->OFDMTxAgcOffset_A[3]);
	writeVal = (byte0 << 24) | (byte1 << 16) | (byte2 << 8) | byte3;
	RTL_W32(rTxAGC_A_Rate18_06, writeVal);

	byte0 = POWER_RANGE_CHECK(base + priv->pshare->phw->OFDMTxAgcOffset_A[4]);
	byte1 = POWER_RANGE_CHECK(base + priv->pshare->phw->OFDMTxAgcOffset_A[5]);
	byte2 = POWER_RANGE_CHECK(base + priv->pshare->phw->OFDMTxAgcOffset_A[6]);
	byte3 = POWER_RANGE_CHECK(base + priv->pshare->phw->OFDMTxAgcOffset_A[7]);
	writeVal = (byte0 << 24) | (byte1 << 16) | (byte2 << 8) | byte3;
	RTL_W32(rTxAGC_A_Rate54_24, writeVal);

	base = pwrlevelHT40_1S_A;
	if (priv->pshare->CurrentChannelBW == HT_CHANNEL_WIDTH_20) {
		offset = (pwrdiffHT20 & 0x0f);
		base = COUNT_SIGN_OFFSET(base, offset);
	}
#ifdef POWER_PERCENT_ADJUSTMENT
	base = POWER_RANGE_CHECK(base + pwrdiff_percent);
#endif


	byte0 = POWER_RANGE_CHECK(base + priv->pshare->phw->MCSTxAgcOffset_A[0]);
	byte1 = POWER_RANGE_CHECK(base + priv->pshare->phw->MCSTxAgcOffset_A[1]);
	byte2 = POWER_RANGE_CHECK(base + priv->pshare->phw->MCSTxAgcOffset_A[2]);
	byte3 = POWER_RANGE_CHECK(base + priv->pshare->phw->MCSTxAgcOffset_A[3]);
	writeVal = (byte0 << 24) | (byte1 << 16) | (byte2 << 8) | byte3;
	RTL_W32(rTxAGC_A_Mcs03_Mcs00, writeVal);

	byte0 = POWER_RANGE_CHECK(base + priv->pshare->phw->MCSTxAgcOffset_A[4]);
	byte1 = POWER_RANGE_CHECK(base + priv->pshare->phw->MCSTxAgcOffset_A[5]);
	byte2 = POWER_RANGE_CHECK(base + priv->pshare->phw->MCSTxAgcOffset_A[6]);
	byte3 = POWER_RANGE_CHECK(base + priv->pshare->phw->MCSTxAgcOffset_A[7]);
	writeVal = (byte0 << 24) | (byte1 << 16) | (byte2 << 8) | byte3;
	RTL_W32(rTxAGC_A_Mcs07_Mcs04, writeVal);

	offset = (pwrdiffHT40_2S & 0x0f);
	base = COUNT_SIGN_OFFSET(base, offset);

//_TXPWR_REDEFINE
#ifdef USB_POWER_SUPPORT

	base_6dBm = pwrlevelHT40_6dBm_1S_A;

	if (priv->pshare->CurrentChannelBW == HT_CHANNEL_WIDTH_20) {
		offset_6dBm = (pwrdiffHT20_6dBm & 0x0f);


		base_6dBm = COUNT_SIGN_OFFSET(base_6dBm, offset_6dBm);
	}

	offset_6dBm = (pwrdiffHT40_6dBm_2S & 0x0f);


	base_6dBm = COUNT_SIGN_OFFSET(base_6dBm, offset_6dBm);

	if ((pwrlevelHT40_6dBm_1S_A != 0) && (pwrlevelHT40_6dBm_1S_A != pwrlevelHT40_1S_A))
		byte0 = byte1 = byte2 = byte3 =	base_6dBm;
	else if ((base - USB_HT_2S_DIFF) > 0)
		byte0 = byte1 = byte2 = byte3 =	POWER_RANGE_CHECK(base - USB_HT_2S_DIFF);
	else
		byte0 = byte1 = byte2 = byte3 =	POWER_RANGE_CHECK(defValue - USB_HT_2S_DIFF);

#else
//_TXPWR_REDEFINE ?? MCS 8 - 11, shall NOT add power by rate even NOT USB power ??
	byte0 = POWER_RANGE_CHECK(base + priv->pshare->phw->MCSTxAgcOffset_A[8]);
	byte1 = POWER_RANGE_CHECK(base + priv->pshare->phw->MCSTxAgcOffset_A[9]);
	byte2 = POWER_RANGE_CHECK(base + priv->pshare->phw->MCSTxAgcOffset_A[10]);
	byte3 = POWER_RANGE_CHECK(base + priv->pshare->phw->MCSTxAgcOffset_A[11]);
#endif

	writeVal = (byte0 << 24) | (byte1 << 16) | (byte2 << 8) | byte3;

	//DEBUG_INFO("debug e18:%x,%x,[%x,%x,%x,%x],%x\n", offset, base, byte0, byte1, byte2, byte3, writeVal);
	RTL_W32(rTxAGC_A_Mcs11_Mcs08, writeVal);

#ifdef USB_POWER_SUPPORT
//_TXPWR_REDEFINE
	if ((pwrlevelHT40_6dBm_1S_A != 0) && (pwrlevelHT40_6dBm_1S_A != pwrlevelHT40_1S_A))
		byte0 = byte1 = byte2 = byte3 =	base_6dBm;
	else if ((base - USB_HT_2S_DIFF) > 0)
		byte0 = byte1 = byte2 = byte3 =	POWER_RANGE_CHECK(base - USB_HT_2S_DIFF);
	else
		byte0 = byte1 = byte2 = byte3 =	POWER_RANGE_CHECK(defValue - USB_HT_2S_DIFF);

#else
	byte0 = POWER_RANGE_CHECK(base + priv->pshare->phw->MCSTxAgcOffset_A[12]);
	byte1 = POWER_RANGE_CHECK(base + priv->pshare->phw->MCSTxAgcOffset_A[13]);
	byte2 = POWER_RANGE_CHECK(base + priv->pshare->phw->MCSTxAgcOffset_A[14]);
	byte3 = POWER_RANGE_CHECK(base + priv->pshare->phw->MCSTxAgcOffset_A[15]);
#endif

	writeVal = (byte0 << 24) | (byte1 << 16) | (byte2 << 8) | byte3;
	RTL_W32(rTxAGC_A_Mcs15_Mcs12, writeVal);

	/******************************  PATH B  ******************************/
	base = pwrlevelHT40_1S_B;
	offset = ((pwrdiffOFDM & 0xf0) >> 4);
	base = COUNT_SIGN_OFFSET(base, offset);
#ifdef POWER_PERCENT_ADJUSTMENT
	base = POWER_RANGE_CHECK(base + pwrdiff_percent);
#endif


	byte0 = POWER_RANGE_CHECK(base + priv->pshare->phw->OFDMTxAgcOffset_B[0]);
	byte1 = POWER_RANGE_CHECK(base + priv->pshare->phw->OFDMTxAgcOffset_B[1]);
	byte2 = POWER_RANGE_CHECK(base + priv->pshare->phw->OFDMTxAgcOffset_B[2]);
	byte3 = POWER_RANGE_CHECK(base + priv->pshare->phw->OFDMTxAgcOffset_B[3]);
	writeVal = (byte0 << 24) | (byte1 << 16) | (byte2 << 8) | byte3;
	RTL_W32(rTxAGC_B_Rate18_06, writeVal);

	byte0 = POWER_RANGE_CHECK(base + priv->pshare->phw->OFDMTxAgcOffset_B[4]);
	byte1 = POWER_RANGE_CHECK(base + priv->pshare->phw->OFDMTxAgcOffset_B[5]);
	byte2 = POWER_RANGE_CHECK(base + priv->pshare->phw->OFDMTxAgcOffset_B[6]);
	byte3 = POWER_RANGE_CHECK(base + priv->pshare->phw->OFDMTxAgcOffset_B[7]);
	writeVal = (byte0 << 24) | (byte1 << 16) | (byte2 << 8) | byte3;
	RTL_W32(rTxAGC_B_Rate54_24, writeVal);

	base = pwrlevelHT40_1S_B;
	if (priv->pshare->CurrentChannelBW == HT_CHANNEL_WIDTH_20) {
		offset = ((pwrdiffHT20 & 0xf0) >> 4);
		base = COUNT_SIGN_OFFSET(base, offset);
	}
#ifdef POWER_PERCENT_ADJUSTMENT
	base = POWER_RANGE_CHECK(base + pwrdiff_percent);
#endif


	byte0 = POWER_RANGE_CHECK(base + priv->pshare->phw->MCSTxAgcOffset_B[0]);
	byte1 = POWER_RANGE_CHECK(base + priv->pshare->phw->MCSTxAgcOffset_B[1]);
	byte2 = POWER_RANGE_CHECK(base + priv->pshare->phw->MCSTxAgcOffset_B[2]);
	byte3 = POWER_RANGE_CHECK(base + priv->pshare->phw->MCSTxAgcOffset_B[3]);
	writeVal = (byte0 << 24) | (byte1 << 16) | (byte2 << 8) | byte3;
	RTL_W32(rTxAGC_B_Mcs03_Mcs00, writeVal);

	byte0 = POWER_RANGE_CHECK(base + priv->pshare->phw->MCSTxAgcOffset_B[4]);
	byte1 = POWER_RANGE_CHECK(base + priv->pshare->phw->MCSTxAgcOffset_B[5]);
	byte2 = POWER_RANGE_CHECK(base + priv->pshare->phw->MCSTxAgcOffset_B[6]);
	byte3 = POWER_RANGE_CHECK(base + priv->pshare->phw->MCSTxAgcOffset_B[7]);
	writeVal = (byte0 << 24) | (byte1 << 16) | (byte2 << 8) | byte3;
	RTL_W32(rTxAGC_B_Mcs07_Mcs04, writeVal);

	offset = ((pwrdiffHT40_2S & 0xf0) >> 4);
	base = COUNT_SIGN_OFFSET(base, offset);

#ifdef USB_POWER_SUPPORT
//_TXPWR_REDEFINE ?? 2.4G
	base_6dBm = pwrlevelHT40_6dBm_1S_B;
	if (priv->pshare->CurrentChannelBW == HT_CHANNEL_WIDTH_20) {
		offset_6dBm = ((pwrdiffHT20_6dBm & 0xf0) >> 4);
		base_6dBm = COUNT_SIGN_OFFSET(base_6dBm, offset_6dBm);
	}

	offset_6dBm = ((pwrdiffHT40_6dBm_2S & 0xf0) >> 4);
	base_6dBm = COUNT_SIGN_OFFSET(base_6dBm, offset_6dBm);

	if (( pwrlevelHT40_6dBm_1S_B != 0 ) && (pwrlevelHT40_6dBm_1S_B != pwrlevelHT40_1S_B))
		byte0 = byte1 = byte2 = byte3 = base_6dBm;
	else if ((base - USB_HT_2S_DIFF) > 0)
		byte0 = byte1 = byte2 = byte3 = POWER_RANGE_CHECK(base - USB_HT_2S_DIFF);
	else
		byte0 = byte1 = byte2 = byte3 =	POWER_RANGE_CHECK(defValue - USB_HT_2S_DIFF);

#else
	byte0 = POWER_RANGE_CHECK(base + priv->pshare->phw->MCSTxAgcOffset_B[8]);
	byte1 = POWER_RANGE_CHECK(base + priv->pshare->phw->MCSTxAgcOffset_B[9]);
	byte2 = POWER_RANGE_CHECK(base + priv->pshare->phw->MCSTxAgcOffset_B[10]);
	byte3 = POWER_RANGE_CHECK(base + priv->pshare->phw->MCSTxAgcOffset_B[11]);
#endif

	writeVal = (byte0 << 24) | (byte1 << 16) | (byte2 << 8) | byte3;
	RTL_W32(rTxAGC_B_Mcs11_Mcs08, writeVal);

#ifdef USB_POWER_SUPPORT
//_TXPWR_REDEFINE ?? 2.4G
	if (( pwrlevelHT40_6dBm_1S_B != 0 ) && (pwrlevelHT40_6dBm_1S_B != pwrlevelHT40_1S_B))
		byte0 = byte1 = byte2 = byte3 = base_6dBm;
	else if ((base - USB_HT_2S_DIFF) > 0)
		byte0 = byte1 = byte2 = byte3 = POWER_RANGE_CHECK(base - USB_HT_2S_DIFF);
	else
		byte0 = byte1 = byte2 = byte3 =	POWER_RANGE_CHECK(defValue - USB_HT_2S_DIFF);

#else
	byte0 = POWER_RANGE_CHECK(base + priv->pshare->phw->MCSTxAgcOffset_B[12]);
	byte1 = POWER_RANGE_CHECK(base + priv->pshare->phw->MCSTxAgcOffset_B[13]);
	byte2 = POWER_RANGE_CHECK(base + priv->pshare->phw->MCSTxAgcOffset_B[14]);
	byte3 = POWER_RANGE_CHECK(base + priv->pshare->phw->MCSTxAgcOffset_B[15]);
#endif

	writeVal = (byte0 << 24) | (byte1 << 16) | (byte2 << 8) | byte3;
	RTL_W32(rTxAGC_B_Mcs15_Mcs12, writeVal);
}	/* PHY_RF6052SetOFDMTxPower */


void PHY_RF6052SetCCKTxPower(struct rtl8192cd_priv *priv, unsigned int channel)
{
	unsigned int writeVal = 0;
	u1Byte byte, byte1, byte2;
	u1Byte pwrlevelCCK_A = priv->pmib->dot11RFEntry.pwrlevelCCK_A[channel - 1];
	u1Byte pwrlevelCCK_B = priv->pmib->dot11RFEntry.pwrlevelCCK_B[channel - 1];
#ifdef POWER_PERCENT_ADJUSTMENT
	s1Byte pwrdiff_percent = PwrPercent2PwrLevel(priv->pmib->dot11RFEntry.power_percent);
#endif

#ifdef TXPWR_LMT
	if (!priv->pshare->rf_ft_var.disable_txpwrlmt) {
#ifdef TXPWR_LMT_88E
		if(GET_CHIP_VER(priv) == VERSION_8188E) {
			int max_idx_a, max_idx_b, i;
			if (!priv->pshare->txpwr_lmt_CCK ||
				!priv->pshare->tgpwr_CCK_new[RF_PATH_A]) {
				DEBUG_INFO("No limit for CCK TxPower\n");
				max_idx_a = 255;
				max_idx_b = 255;
			} else {
				// maximum additional power index
				max_idx_a = (priv->pshare->txpwr_lmt_CCK - priv->pshare->tgpwr_CCK_new[RF_PATH_A]);
				max_idx_b = (priv->pshare->txpwr_lmt_CCK - priv->pshare->tgpwr_CCK_new[RF_PATH_B]);
			}
			for (i = 0; i <= 3; i++) {
				//printk("priv->pshare->phw->CCKTxAgc_A[%d]=0x%x max=0x%x\n",i, priv->pshare->phw->CCKTxAgc_A[i], max_idx_a);
				//printk("priv->pshare->phw->CCKTxAgc_B[%d]=%x\n",i, priv->pshare->phw->CCKTxAgc_B[i]);
				priv->pshare->phw->CCKTxAgc_A[i] = POWER_MIN_CHECK(priv->pshare->phw->CCKTxAgc_A[i], max_idx_a);
				priv->pshare->phw->CCKTxAgc_B[i] = POWER_MIN_CHECK(priv->pshare->phw->CCKTxAgc_B[i], max_idx_b);
			}
		}
		else
#endif
		{
		int max_idx, i;
		if (!priv->pshare->txpwr_lmt_CCK || !priv->pshare->tgpwr_CCK) {
			DEBUG_INFO("No limit for CCK TxPower\n");
			max_idx = 255;
		} else {
			// maximum additional power index
			max_idx = (priv->pshare->txpwr_lmt_CCK - priv->pshare->tgpwr_CCK);
		}

		for (i = 0; i <= 3; i++) {
			priv->pshare->phw->CCKTxAgc_A[i] = POWER_MIN_CHECK(priv->pshare->phw->CCKTxAgc_A[i], max_idx);
			priv->pshare->phw->CCKTxAgc_B[i] = POWER_MIN_CHECK(priv->pshare->phw->CCKTxAgc_B[i], max_idx);
			//printk("priv->pshare->phw->CCKTxAgc_A[%d]=%x\n",i, priv->pshare->phw->CCKTxAgc_A[i]);
			//printk("priv->pshare->phw->CCKTxAgc_B[%d]=%x\n",i, priv->pshare->phw->CCKTxAgc_B[i]);
		}
	}
	}
#endif

	if (priv->pshare->rf_ft_var.cck_pwr_max) {
		//byte = POWER_RANGE_CHECK(priv->pshare->rf_ft_var.cck_pwr_max);
		byte = (priv->pshare->rf_ft_var.cck_pwr_max > 0x3f) ? 0x3f : priv->pshare->rf_ft_var.cck_pwr_max;
		writeVal = byte;
		PHY_SetBBReg(priv, rTxAGC_A_CCK1_Mcs32, 0x0000ff00, writeVal);
		writeVal = (byte << 16) | (byte << 8) | byte;
		PHY_SetBBReg(priv, rTxAGC_B_CCK5_1_Mcs32, 0xffffff00, writeVal);
		writeVal = (byte << 24) | (byte << 16) | (byte << 8) | byte;
		PHY_SetBBReg(priv, rTxAGC_A_CCK11_2_B_CCK11, 0xffffffff, writeVal);
		return;
	}

	if ((pwrlevelCCK_A == 0 && pwrlevelCCK_B == 0)
	   ) {
		// use default value
#ifdef HIGH_POWER_EXT_PA
		if (priv->pshare->rf_ft_var.use_ext_pa)
			byte = HP_CCK_POWER_DEFAULT;
		else
#endif
			byte = 0x24;


#ifndef ADD_TX_POWER_BY_CMD
		writeVal = byte;
		PHY_SetBBReg(priv, rTxAGC_A_CCK1_Mcs32, 0x0000ff00, writeVal);
		writeVal = (byte << 16) | (byte << 8) | byte;
		PHY_SetBBReg(priv, rTxAGC_B_CCK5_1_Mcs32, 0xffffff00, writeVal);
		writeVal = (byte << 24) | (byte << 16) | (byte << 8) | byte;
		PHY_SetBBReg(priv, rTxAGC_A_CCK11_2_B_CCK11, 0xffffffff, writeVal);
#else
		pwrlevelCCK_A = pwrlevelCCK_B = byte;
		byte = 0;
		ASSIGN_TX_POWER_OFFSET(byte, priv->pshare->rf_ft_var.txPowerPlus_cck_1);
		writeVal = POWER_RANGE_CHECK(pwrlevelCCK_A + byte);
		PHY_SetBBReg(priv, rTxAGC_A_CCK1_Mcs32, 0x0000ff00, writeVal);

		byte = byte1 = byte2 = 0;
		ASSIGN_TX_POWER_OFFSET(byte, priv->pshare->rf_ft_var.txPowerPlus_cck_1);
		ASSIGN_TX_POWER_OFFSET(byte1, priv->pshare->rf_ft_var.txPowerPlus_cck_2);
		ASSIGN_TX_POWER_OFFSET(byte2, priv->pshare->rf_ft_var.txPowerPlus_cck_5);
		byte  = POWER_RANGE_CHECK(pwrlevelCCK_B + byte);
		byte1 = POWER_RANGE_CHECK(pwrlevelCCK_B + byte1);
		byte2 = POWER_RANGE_CHECK(pwrlevelCCK_B + byte2);
		writeVal = ((byte2 << 16) | (byte1 << 8) | byte);
		PHY_SetBBReg(priv, rTxAGC_B_CCK5_1_Mcs32, 0xffffff00, writeVal);

		byte = byte1 = byte2 = 0;
		ASSIGN_TX_POWER_OFFSET(byte, priv->pshare->rf_ft_var.txPowerPlus_cck_2);
		ASSIGN_TX_POWER_OFFSET(byte1, priv->pshare->rf_ft_var.txPowerPlus_cck_5);
		ASSIGN_TX_POWER_OFFSET(byte2, priv->pshare->rf_ft_var.txPowerPlus_cck_11);
		byte  = POWER_RANGE_CHECK(pwrlevelCCK_A + byte);
		byte1 = POWER_RANGE_CHECK(pwrlevelCCK_A + byte1);
		byte2 = POWER_RANGE_CHECK(pwrlevelCCK_A + byte2);
		writeVal = ((byte2 << 24) | (byte1 << 16) | (byte << 8) | byte2);
		PHY_SetBBReg(priv, rTxAGC_A_CCK11_2_B_CCK11, 0xffffffff, writeVal);
#endif
		return; // use default
	}

	if ((get_rf_mimo_mode(priv) == MIMO_2T2R) && (pwrlevelCCK_B == 0)) {
		pwrlevelCCK_B = pwrlevelCCK_A +
						priv->pmib->dot11RFEntry.pwrlevelHT40_1S_B[channel - 1] - priv->pmib->dot11RFEntry.pwrlevelHT40_1S_A[channel - 1];
	}

#ifdef POWER_PERCENT_ADJUSTMENT
	pwrlevelCCK_A = POWER_RANGE_CHECK(pwrlevelCCK_A + pwrdiff_percent);
	pwrlevelCCK_B = POWER_RANGE_CHECK(pwrlevelCCK_B + pwrdiff_percent);
#endif


	writeVal = POWER_RANGE_CHECK(pwrlevelCCK_A + priv->pshare->phw->CCKTxAgc_A[3] + priv->pmib->dot11RFEntry.add_cck1M_pwr);
	PHY_SetBBReg(priv, rTxAGC_A_CCK1_Mcs32, 0x0000ff00, writeVal);
	writeVal = (POWER_RANGE_CHECK(pwrlevelCCK_B + priv->pshare->phw->CCKTxAgc_B[1]) << 16) |
			   (POWER_RANGE_CHECK(pwrlevelCCK_B + priv->pshare->phw->CCKTxAgc_B[2]) << 8)  |
	            POWER_RANGE_CHECK(pwrlevelCCK_B + priv->pshare->phw->CCKTxAgc_B[3] + priv->pmib->dot11RFEntry.add_cck1M_pwr);
	PHY_SetBBReg(priv, rTxAGC_B_CCK5_1_Mcs32, 0xffffff00, writeVal);
	writeVal = (POWER_RANGE_CHECK(pwrlevelCCK_A + priv->pshare->phw->CCKTxAgc_A[0]) << 24) |
			   (POWER_RANGE_CHECK(pwrlevelCCK_A + priv->pshare->phw->CCKTxAgc_A[1]) << 16) |
			   (POWER_RANGE_CHECK(pwrlevelCCK_A + priv->pshare->phw->CCKTxAgc_A[2]) << 8)  |
			   POWER_RANGE_CHECK(pwrlevelCCK_B + priv->pshare->phw->CCKTxAgc_B[0]);
	PHY_SetBBReg(priv, rTxAGC_A_CCK11_2_B_CCK11, 0xffffffff, writeVal);
}



// TODO: Filen, comfirm register setting below
static int phy_BB88XX_Config_ParaFile(struct rtl8192cd_priv *priv)
{
	int rtStatus = 0;
	unsigned short val16;
	unsigned int val32;
	unsigned int i, j;

	phy_InitBBRFRegisterDefinition(priv);


	// Enable BB and RF
	val16 = RTL_R16(REG_SYS_FUNC_EN);
	RTL_W16(REG_SYS_FUNC_EN, val16 | BIT(13) | BIT(0) | BIT(1));


    /* temporary enable BIT_RF1_SDMRSTB/BIT_RF1_RSTB/BIT_RF1_EN of REG_AFE_CTRL4(0x78),
            need to be moved to MAC_REG.txt in the future (suggested by SD1 Eric.Liu)
            temporary enable BIT_POW_REGU_P0(2)/BIT_POW_REGU_P1(10) of REG_AFE_LDO_CTRL(0x20),
            need to be moved to MAC_REG.txt in the future (suggested by Yen-Cheng Hsu)
        */
	if (GET_CHIP_VER(priv) == VERSION_8197F)  {
				RTL_W8(REG_RF_CTRL, RTL_R8(REG_RF_CTRL) & ~(RF_EN | RF_RSTB | RF_SDMRSTB));
        RTL_W32(0x78, RTL_R32(0x78) & ~(BIT24 | BIT25 | BIT26));
        RTL_W32(0x78, RTL_R32(0x78) | BIT24 | BIT25 | BIT26);
        RTL_W32(0x20, RTL_R32(0x20) | BIT2 | BIT10);
    }


	RTL_W8(REG_RF_CTRL, RF_EN | RF_RSTB | RF_SDMRSTB);

	/*----Check chip ID and hw TR MIMO config----*/
//	check_chipID_MIMO(priv);

// 8814 merge issue, why eric mark this flag in branch 3.4
#if 0//def CONFIG_WLAN_HAL
	if ( IS_HAL_CHIP(priv) ) {


	}
#endif  //CONFIG_WLAN_HAL

	if(ODMPTR->ConfigBBRF)
		rtStatus = ODM_ConfigBBWithHeaderFile(ODMPTR, CONFIG_BB_PHY_REG);
	else
		rtStatus = PHY_ConfigBBWithParaFile(priv, PHYREG);

	if (GET_CHIP_VER(priv) == VERSION_8197F) {
		phy_path_a_dpk_init_8197f(ODMPTR);
		phy_path_b_dpk_init_8197f(ODMPTR);
	}

#ifdef CONFIG_RF_DPK_SETTING_SUPPORT
	ODMPTR->RFCalibrateInfo.bDPKEnable = 1;

	if (priv->pmib->dot11RFDPKEntry.bDPPathAOK == 1 && priv->pmib->dot11RFDPKEntry.bDPPathBOK == 1) {
		ODMPTR->RFCalibrateInfo.bDPPathAOK = priv->pmib->dot11RFDPKEntry.bDPPathAOK;
		ODMPTR->RFCalibrateInfo.bDPPathBOK = priv->pmib->dot11RFDPKEntry.bDPPathBOK;
		//printk("bDPPathAOK=%d   bDPPathBOK=%d\n", priv->pmib->dot11RFDPKEntry.bDPPathAOK, priv->pmib->dot11RFDPKEntry.bDPPathBOK);
		for (i = 0; i < 3; i++) 	{
			ODMPTR->RFCalibrateInfo.pwsf_2g_a[i] = priv->pmib->dot11RFDPKEntry.pwsf_2g_a[i];
			ODMPTR->RFCalibrateInfo.pwsf_2g_b[i] = priv->pmib->dot11RFDPKEntry.pwsf_2g_b[i];
			//printk("!!! pwsf_2g_a[%d]=0x%x   pwsf_2g_b[%d]=0x%x\n", i, priv->pmib->dot11RFDPKEntry.pwsf_2g_a[i], i, priv->pmib->dot11RFDPKEntry.pwsf_2g_b[i]);
			for (j = 0; j < 64; j++)
			{
				ODMPTR->RFCalibrateInfo.lut_2g_even_a[i][j] = priv->pmib->dot11RFDPKEntry.lut_2g_even_a[i][j];
				ODMPTR->RFCalibrateInfo.lut_2g_odd_a[i][j] = priv->pmib->dot11RFDPKEntry.lut_2g_odd_a[i][j];
				ODMPTR->RFCalibrateInfo.lut_2g_even_b[i][j] = priv->pmib->dot11RFDPKEntry.lut_2g_even_b[i][j];
				ODMPTR->RFCalibrateInfo.lut_2g_odd_b[i][j] = priv->pmib->dot11RFDPKEntry.lut_2g_odd_b[i][j];
				//printk("!!! lut_2g_even_a[%d][%d]=0x%x   lut_2g_odd_a[%d][%d]=0x%x\n", i, j, priv->pmib->dot11RFDPKEntry.lut_2g_even_a[i][j], i, j, priv->pmib->dot11RFDPKEntry.lut_2g_odd_a[i][j]);
				//printk("!!! lut_2g_even_b[%d][%d]=0x%x   lut_2g_odd_b[%d][%d]=0x%x\n", i, j, priv->pmib->dot11RFDPKEntry.lut_2g_even_b[i][j], i, j, priv->pmib->dot11RFDPKEntry.lut_2g_odd_b[i][j]);
			}
		}
	}
#endif
#if 0
	printk("bDPPathAOK=%d	bDPPathBOK=%d\n", priv->pmib->dot11RFDPKEntry.bDPPathAOK, priv->pmib->dot11RFDPKEntry.bDPPathBOK);
	for (i = 0; i < 3; i++)
	{
		printk("!!! pwsf_2g_a[%d]=0x%x	 pwsf_2g_b[%d]=0x%x\n", i, ODMPTR->RFCalibrateInfo.pwsf_2g_a[i], i, ODMPTR->RFCalibrateInfo.pwsf_2g_b[i]);
	}
	for (i = 0; i < 3; i++)
	{
		for (j = 0; j < 64; j++)
		printk("!!! lut_2g_even_a[%2d][%2d]=0x%x\n", i, j, priv->pmib->dot11RFDPKEntry.lut_2g_even_a[i][j]);
		for (j = 0; j < 64; j++)
		printk("!!!  lut_2g_odd_a[%2d][%2d]=0x%x\n", i, j, priv->pmib->dot11RFDPKEntry.lut_2g_odd_a[i][j]);
		for (j = 0; j < 64; j++)
		printk("!!! lut_2g_even_b[%2d][%2d]=0x%x\n", i, j, priv->pmib->dot11RFDPKEntry.lut_2g_even_b[i][j]);
		for (j = 0; j < 64; j++)
		printk("!!!  lut_2g_odd_b[%2d][%2d]=0x%x\n", i, j, priv->pmib->dot11RFDPKEntry.lut_2g_odd_b[i][j]);
	}
#endif

#if 1   //Filen, BB have no release these files
#ifdef MP_TEST
	if (priv->pshare->rf_ft_var.mp_specific) {
		delay_ms(10);
	if(ODMPTR->ConfigBBRF)
		rtStatus |= ODM_ConfigBBWithHeaderFile(ODMPTR, CONFIG_BB_PHY_REG_MP);
	else
		rtStatus |= PHY_ConfigBBWithParaFile(priv, PHYREG_MP);
	}
#endif

	if (rtStatus) {
		printk("phy_BB88XX_Config_ParaFile(): PHYREG_MP Reg Fail!!\n");
		return rtStatus;
	}

	/*----If EEPROM or EFUSE autoload OK, We must config by PHY_REG_PG.txt----*/
	if (0)//(GET_CHIP_VER(priv) == VERSION_8822B) //eric-8822 ?? PG use txt ??
		rtStatus = ODM_ConfigBBWithHeaderFile(ODMPTR, CONFIG_BB_PHY_REG_PG);
	else
		rtStatus = PHY_ConfigBBWithParaFile(priv, PHYREG_PG);

	if (rtStatus) {
		printk("phy_BB88XX_Config_ParaFile():BB_PG Reg Fail!!\n");
		return rtStatus;
	}
#endif
	/*----BB AGC table Initialization----*/
	if(ODMPTR->ConfigBBRF)
		rtStatus = ODM_ConfigBBWithHeaderFile(ODMPTR, CONFIG_BB_AGC_TAB);
	else
		rtStatus = PHY_ConfigBBWithParaFile(priv, AGCTAB);

	if (rtStatus) {
		printk("phy_BB8192CD_Config_ParaFile(): Write BB AGC Table Fail!!\n");
		return rtStatus;
	}


	if(ODMPTR->ConfigBBRF)
		priv->pshare->PhyVersion = ODM_GetHWImgVersion(ODMPTR);
	DEBUG_INFO("PHY-BB Initialization Success\n");

	return 0;
}

#if(CONFIG_WLAN_NOT_HAL_EXIST==1)
static int phy_BB8192CD_Config_ParaFile(struct rtl8192cd_priv *priv)
{
	int rtStatus = 0;
	unsigned short val16;
	unsigned int val32;

	phy_InitBBRFRegisterDefinition(priv);

	// Enable BB and RF
	val16 = RTL_R16(REG_SYS_FUNC_EN);
	RTL_W16(REG_SYS_FUNC_EN, val16 | BIT(13) | BIT(0) | BIT(1));

	// 20090923 Joseph: Advised by Steven and Jenyu. Power sequence before init RF.
	RTL_W8(REG_RF_CTRL, RF_EN | RF_RSTB | RF_SDMRSTB);

	//RTL_W8(REG_SYS_FUNC_EN, FEN_PPLL|FEN_PCIEA|FEN_DIO_PCIE|FEN_USBA|FEN_BB_GLB_RST|FEN_BBRSTB);
	//RTL_W8(REG_LDOHCI12_CTRL, 0x1f);

	/*----Check chip ID and hw TR MIMO config----*/
//	check_chipID_MIMO(priv);
	if(ODMPTR->ConfigBBRF)
		rtStatus = ODM_ConfigBBWithHeaderFile(ODMPTR, CONFIG_BB_PHY_REG);
	else
	{
		if ((GET_CHIP_VER(priv) == VERSION_8192C) || (GET_CHIP_VER(priv) == VERSION_8188C))
		{
			if (get_rf_mimo_mode(priv) == MIMO_2T2R)
				rtStatus = PHY_ConfigBBWithParaFile(priv, PHYREG_2T2R);
			else if (get_rf_mimo_mode(priv) == MIMO_1T1R)
				rtStatus = PHY_ConfigBBWithParaFile(priv, PHYREG_1T1R);
		}
		else
			rtStatus = PHY_ConfigBBWithParaFile(priv, PHYREG);
	}

#ifdef MP_TEST
	if ((priv->pshare->rf_ft_var.mp_specific)
	   ) {
		delay_ms(10);
	if(ODMPTR->ConfigBBRF)
		rtStatus |= ODM_ConfigBBWithHeaderFile(ODMPTR, CONFIG_BB_PHY_REG_MP);
	else
		rtStatus |= PHY_ConfigBBWithParaFile(priv, PHYREG_MP);
	}
#endif

	if (rtStatus) {
		printk("phy_BB8192CD_Config_ParaFile(): Write BB Reg Fail!!\n");
		return rtStatus;
	}

	/*----If EEPROM or EFUSE autoload OK, We must config by PHY_REG_PG.txt----*/
	if (0)
		rtStatus = ODM_ConfigBBWithHeaderFile(ODMPTR, CONFIG_BB_PHY_REG_PG);
	else
		rtStatus = PHY_ConfigBBWithParaFile(priv, PHYREG_PG);

	if (rtStatus) {
		printk("phy_BB8192CD_Config_ParaFile():BB_PG Reg Fail!!\n");
		return rtStatus;
	}

	/*----BB AGC table Initialization----*/
	if(ODMPTR->ConfigBBRF)
		rtStatus = ODM_ConfigBBWithHeaderFile(ODMPTR, CONFIG_BB_AGC_TAB);
	else
		rtStatus = PHY_ConfigBBWithParaFile(priv, AGCTAB);

	{
		PHY_SetBBReg(priv, rFPGA0_RFMOD, bOFDMEn, 1);
			PHY_SetBBReg(priv, rFPGA0_RFMOD, bCCKEn, 1);
	}

	if (rtStatus) {
		printk("phy_BB8192CD_Config_ParaFile(): Write BB AGC Table Fail!!\n");
		return rtStatus;
	}


#if 0
	/*----For 1T2R Config----*/
	if (get_rf_mimo_mode(priv) == MIMO_1T2R) {
		rtStatus = PHY_ConfigBBWithParaFile(priv, PHYREG_1T2R);
		if (rtStatus) {
			printk("phy_BB8192CD_Config_ParaFile(): Write BB Reg for 1T2R Fail!!\n");
			return rtStatus;
		}
	} else if (get_rf_mimo_mode(priv) == MIMO_1T1R) {
		delay_ms(100);
		rtStatus = PHY_ConfigBBWithParaFile(priv, PHYREG_1T1R);
		if (rtStatus) {
			printk("phy_BB8192CD_Config_ParaFile(): Write BB Reg for 1T1R Fail!!\n");
			return rtStatus;
		}
	}
#endif

	DEBUG_INFO("PHY-BB Initialization Success\n");
	return 0;
}
#else
static int phy_BB8192CD_Config_ParaFile(struct rtl8192cd_priv *priv)
{
	return 0;
}
#endif//CONFIG_WLAN_NOT_HAL_EXIST

#if 1
//#if !defined(CONFIG_MACBBRF_BY_ODM) || !defined(CONFIG_RTL_88E_SUPPORT)

int phy_RF6052_Config_ParaFile(struct rtl8192cd_priv *priv)
{
	int rtStatus = 0;
	RF92CD_RADIO_PATH_E eRFPath;
	BB_REGISTER_DEFINITION_T *pPhyReg;
	unsigned int  u4RegValue = 0, NumTotalRFPath;
	unsigned long x;

#if defined(HIGH_POWER_EXT_PA) && defined(HIGH_POWER_EXT_LNA)
	HW_VARIABLES RF_REG_FILE_RADIO_HP_SIZE[RF92CD_PATH_MAX]  = {
		HW_VAR_RFREGFILE_RADIO_A_HP_SIZE,  HW_VAR_RFREGFILE_RADIO_B_HP_SIZE,
	};
	HW_VARIABLES RF_REG_FILE_RADIO_HP_START[RF92CD_PATH_MAX] = {
		HW_VAR_RFREGFILE_RADIO_A_HP_START, HW_VAR_RFREGFILE_RADIO_B_HP_START,
	};
#endif // HIGH_POWER_EXT_PA && HIGH_POWER_EXT_LNA

	HW_VARIABLES RF_REG_FILE_RADIO_SIZE[RF92CD_PATH_MAX]     = {
		HW_VAR_RFREGFILE_RADIO_A_SIZE,     HW_VAR_RFREGFILE_RADIO_B_SIZE,
	};
	HW_VARIABLES RF_REG_FILE_RADIO_START[RF92CD_PATH_MAX]    = {
		HW_VAR_RFREGFILE_RADIO_A_START,    HW_VAR_RFREGFILE_RADIO_B_START,
	};

#ifdef HIGH_POWER_EXT_PA
	HW_VARIABLES RF_REG_FILE_RADIO_EXTPA_SIZE[RF92CD_PATH_MAX]	= {
		HW_VAR_RFREGFILE_RADIO_A_EXTPA_SIZE,  HW_VAR_RFREGFILE_RADIO_B_EXTPA_SIZE,
		};
	HW_VARIABLES RF_REG_FILE_RADIO_EXTPA_START[RF92CD_PATH_MAX] = {
		HW_VAR_RFREGFILE_RADIO_A_EXTPA_START, HW_VAR_RFREGFILE_RADIO_B_EXTPA_START,
		};
#endif // HIGH_POWER_EXT_PA

#ifdef HIGH_POWER_EXT_LNA
	HW_VARIABLES RF_REG_FILE_RADIO_EXTLNA_SIZE[RF92CD_PATH_MAX]  = {
		HW_VAR_RFREGFILE_RADIO_A_EXTLNA_SIZE,  HW_VAR_RFREGFILE_RADIO_B_EXTLNA_SIZE,
		};

	HW_VARIABLES RF_REG_FILE_RADIO_EXTLNA_START[RF92CD_PATH_MAX] = {
		HW_VAR_RFREGFILE_RADIO_A_EXTLNA_START, HW_VAR_RFREGFILE_RADIO_B_EXTLNA_START,
		};
#endif // HIGH_POWER_EXT_LNA

//	SAVE_INT_AND_CLI(x);

		if ( IS_HAL_CHIP(priv)) {
			GET_HAL_INTERFACE(priv)->GetHwRegHandler(priv, HW_VAR_NUM_TOTAL_RF_PATH, (pu1Byte)&priv->pshare->phw->NumTotalRFPath);
		} else if(CONFIG_WLAN_NOT_HAL_EXIST)
		priv->pshare->phw->NumTotalRFPath = 2;

		NumTotalRFPath = priv->pshare->phw->NumTotalRFPath;
	for (eRFPath = RF92CD_PATH_A; eRFPath < NumTotalRFPath; eRFPath++) {

		pPhyReg = &priv->pshare->phw->PHYRegDef[eRFPath];

		/*----Store original RFENV control type----*/
		switch (eRFPath) {
		case RF92CD_PATH_A:
			u4RegValue = PHY_QueryBBReg(priv, pPhyReg->rfintfs, bRFSI_RFENV);
			break;
		case RF92CD_PATH_B :
			u4RegValue = PHY_QueryBBReg(priv, pPhyReg->rfintfs, bRFSI_RFENV << 16);
			break;
		case RF92CD_PATH_MAX:
			break;
		}

		/*----Set RF_ENV enable----*/
		PHY_SetBBReg(priv, pPhyReg->rfintfe, bRFSI_RFENV << 16, 0x1);

		/*----Set RF_ENV output high----*/
		PHY_SetBBReg(priv, pPhyReg->rfintfo, bRFSI_RFENV, 0x1);

		/* Set bit number of Address and Data for RF register */
		PHY_SetBBReg(priv, pPhyReg->rfHSSIPara2, b3WireAddressLength, 0x0);
		PHY_SetBBReg(priv, pPhyReg->rfHSSIPara2, b3WireDataLength, 0x0);
		/*----Initialize RF fom connfiguration file----*/




		if (IS_HAL_CHIP(priv)) {
			pu4Byte FileStartPtr;
			u4Byte  FileLength;
#ifdef HIGH_POWER_EXT_PA
#ifdef HIGH_POWER_EXT_LNA
			if (priv->pshare->rf_ft_var.use_ext_pa && priv->pshare->rf_ft_var.use_ext_lna) {
				//printk("[%s][Radio_HAL_hp] path%d\n",__FUNCTION__,eRFPath+1);
    			GET_HAL_INTERFACE(priv)->GetHwRegHandler(priv, RF_REG_FILE_RADIO_HP_SIZE[eRFPath], (pu1Byte)&FileLength);
    			GET_HAL_INTERFACE(priv)->GetHwRegHandler(priv, RF_REG_FILE_RADIO_HP_START[eRFPath], (pu1Byte)&FileStartPtr);
    			rtStatus = PHY_ConfigRFWithParaFile(priv, (pu1Byte)FileStartPtr, (int)FileLength, eRFPath);
			} else if (priv->pshare->rf_ft_var.use_ext_pa && !priv->pshare->rf_ft_var.use_ext_lna) {
				//printk("[%s][Radio_HAL_extpa] path%d\n",__FUNCTION__,eRFPath+1);
    			GET_HAL_INTERFACE(priv)->GetHwRegHandler(priv, RF_REG_FILE_RADIO_EXTPA_SIZE[eRFPath], (pu1Byte)&FileLength);
    			GET_HAL_INTERFACE(priv)->GetHwRegHandler(priv, RF_REG_FILE_RADIO_EXTPA_START[eRFPath], (pu1Byte)&FileStartPtr);
    			rtStatus = PHY_ConfigRFWithParaFile(priv, (pu1Byte)FileStartPtr, (int)FileLength, eRFPath);
			} else if (!priv->pshare->rf_ft_var.use_ext_pa && priv->pshare->rf_ft_var.use_ext_lna) {
				//printk("[%s][Radio_HAL_extlna] path%d\n",__FUNCTION__,eRFPath+1);
    			GET_HAL_INTERFACE(priv)->GetHwRegHandler(priv, RF_REG_FILE_RADIO_EXTLNA_SIZE[eRFPath], (pu1Byte)&FileLength);
    			GET_HAL_INTERFACE(priv)->GetHwRegHandler(priv, RF_REG_FILE_RADIO_EXTLNA_START[eRFPath], (pu1Byte)&FileStartPtr);
    			rtStatus = PHY_ConfigRFWithParaFile(priv, (pu1Byte)FileStartPtr, (int)FileLength, eRFPath);
			} else
#else
			if (priv->pshare->rf_ft_var.use_ext_pa) {
				//printk("[%s][Radio_HAL_extpa] path%d\n",__FUNCTION__,eRFPath+1);
				GET_HAL_INTERFACE(priv)->GetHwRegHandler(priv, RF_REG_FILE_RADIO_EXTPA_SIZE[eRFPath], (pu1Byte)&FileLength);
				GET_HAL_INTERFACE(priv)->GetHwRegHandler(priv, RF_REG_FILE_RADIO_EXTPA_START[eRFPath], (pu1Byte)&FileStartPtr);
				rtStatus = PHY_ConfigRFWithParaFile(priv, (pu1Byte)FileStartPtr, (int)FileLength, eRFPath);
			}else
#endif  //HIGH_POWER_EXT_LNA
#else
#ifdef HIGH_POWER_EXT_LNA
			if (priv->pshare->rf_ft_var.use_ext_lna) {
				//printk("[%s][Radio_HAL_extlna] path%d\n",__FUNCTION__,eRFPath+1);
    			GET_HAL_INTERFACE(priv)->GetHwRegHandler(priv, RF_REG_FILE_RADIO_EXTLNA_SIZE[eRFPath], (pu1Byte)&FileLength);
    			GET_HAL_INTERFACE(priv)->GetHwRegHandler(priv, RF_REG_FILE_RADIO_EXTLNA_START[eRFPath], (pu1Byte)&FileStartPtr);
    			rtStatus = PHY_ConfigRFWithParaFile(priv, (pu1Byte)FileStartPtr, (int)FileLength, eRFPath);
			} else
#endif
#endif  //HIGH_POWER_EXT_PA
			{
				//printk("[%s][Radio_HAL] path%d\n",__FUNCTION__,eRFPath+1);
				GET_HAL_INTERFACE(priv)->GetHwRegHandler(priv, RF_REG_FILE_RADIO_SIZE[eRFPath], (pu1Byte)&FileLength);
				GET_HAL_INTERFACE(priv)->GetHwRegHandler(priv, RF_REG_FILE_RADIO_START[eRFPath], (pu1Byte)&FileStartPtr);
				rtStatus = PHY_ConfigRFWithParaFile(priv, (pu1Byte)FileStartPtr, (int)FileLength, eRFPath);
			}
#if 0 // TODO: can be removed after 8814A test ok
			switch (eRFPath) {
			case RF92CD_PATH_A:
#ifdef HIGH_POWER_EXT_PA
				if ( priv->pshare->rf_ft_var.use_ext_pa) {
					GET_HAL_INTERFACE(priv)->GetHwRegHandler(priv, HW_VAR_RFREGFILE_RADIO_A_HP_SIZE, (pu1Byte)&FileLength);
					GET_HAL_INTERFACE(priv)->GetHwRegHandler(priv, HW_VAR_RFREGFILE_RADIO_A_HP_START, (pu1Byte)&FileStartPtr);
					printk("[%s][%s][RadioA_HAL_hp]\n", __FUNCTION__, ((GET_CHIP_VER(priv) == VERSION_8881A) ? "RTL_8881A" : "RTL_8192E"));
					rtStatus = PHY_ConfigRFWithParaFile(priv, FileStartPtr,
														(int)FileLength, eRFPath);
				} else
#endif
#ifdef HIGH_POWER_EXT_LNA
					if ( priv->pshare->rf_ft_var.use_ext_lna) {
						GET_HAL_INTERFACE(priv)->GetHwRegHandler(priv, HW_VAR_RFREGFILE_RADIO_A_EXTLNA_SIZE, (pu1Byte)&FileLength);
						GET_HAL_INTERFACE(priv)->GetHwRegHandler(priv, HW_VAR_RFREGFILE_RADIO_A_EXTLNA_START, (pu1Byte)&FileStartPtr);
						printk("[%s][%s][RadioA_HAL_extlna]\n", __FUNCTION__, ((GET_CHIP_VER(priv) == VERSION_8881A) ? "RTL_8881A" : "RTL_8192E"));
						rtStatus = PHY_ConfigRFWithParaFile(priv, FileStartPtr,
															(int)FileLength, eRFPath);
					} else
#endif
				{
					GET_HAL_INTERFACE(priv)->GetHwRegHandler(priv, HW_VAR_RFREGFILE_RADIO_A_SIZE, (pu1Byte)&FileLength);
					GET_HAL_INTERFACE(priv)->GetHwRegHandler(priv, HW_VAR_RFREGFILE_RADIO_A_START, (pu1Byte)&FileStartPtr);
					printk("[%s][RadioA_HAL]\n", __FUNCTION__);
					rtStatus = PHY_ConfigRFWithParaFile(priv, (pu1Byte)FileStartPtr, (int)FileLength, eRFPath);
				}
				break;
			case RF92CD_PATH_B:
#ifdef HIGH_POWER_EXT_PA
				if ( priv->pshare->rf_ft_var.use_ext_pa) {
					GET_HAL_INTERFACE(priv)->GetHwRegHandler(priv, HW_VAR_RFREGFILE_RADIO_B_HP_SIZE, (pu1Byte)&FileLength);
					GET_HAL_INTERFACE(priv)->GetHwRegHandler(priv, HW_VAR_RFREGFILE_RADIO_B_HP_START, (pu1Byte)&FileStartPtr);
					printk("[%s][%s][RadioB_HAL_hp]\n", __FUNCTION__, ((GET_CHIP_VER(priv) == VERSION_8881A) ? "RTL_8881A" : "RTL_8192E"));
					rtStatus = PHY_ConfigRFWithParaFile(priv, FileStartPtr,
														(int)FileLength, eRFPath);
				} else
#endif
#ifdef HIGH_POWER_EXT_LNA
				if ( priv->pshare->rf_ft_var.use_ext_lna) {
					GET_HAL_INTERFACE(priv)->GetHwRegHandler(priv, HW_VAR_RFREGFILE_RADIO_B_EXTLNA_SIZE, (pu1Byte)&FileLength);
					GET_HAL_INTERFACE(priv)->GetHwRegHandler(priv, HW_VAR_RFREGFILE_RADIO_B_EXTLNA_START, (pu1Byte)&FileStartPtr);
					printk("[%s][%s][RadioB_HAL_extlna]\n", __FUNCTION__, ((GET_CHIP_VER(priv) == VERSION_8881A) ? "RTL_8881A" : "RTL_8192E"));
					rtStatus = PHY_ConfigRFWithParaFile(priv, FileStartPtr,
														(int)FileLength, eRFPath);
				} else
#endif
				{
					GET_HAL_INTERFACE(priv)->GetHwRegHandler(priv, HW_VAR_RFREGFILE_RADIO_B_SIZE, (pu1Byte)&FileLength);
					GET_HAL_INTERFACE(priv)->GetHwRegHandler(priv, HW_VAR_RFREGFILE_RADIO_B_START, (pu1Byte)&FileStartPtr);
					printk("[%s][RadioB_HAL]\n", __FUNCTION__);
					rtStatus = PHY_ConfigRFWithParaFile(priv, (pu1Byte)FileStartPtr,
														(int)FileLength, eRFPath);
				}
				break;

			default:
				break;
			}
#endif // end if 0

		}


		/*----Restore RFENV control type----*/;
		switch (eRFPath) {
		case RF92CD_PATH_A:
			PHY_SetBBReg(priv, pPhyReg->rfintfs, bRFSI_RFENV, u4RegValue);
			break;
		case RF92CD_PATH_B :
			PHY_SetBBReg(priv, pPhyReg->rfintfs, bRFSI_RFENV << 16, u4RegValue);
			break;
		case RF92CD_PATH_MAX:
			break;
		}

	}


	DEBUG_INFO("PHY-RF Initialization Success\n");

//	RESTORE_INT(x);
	return rtStatus;
}
#endif

//
// Description:
//	Set the MAC offset [0x09] and prevent all I/O for a while (about 20us~200us, suggested from SD4 Scott).
//	If the protection is not performed well or the value is not set complete, the next I/O will cause the system hang.
// Note:
//	This procudure is designed specifically for 8192S and references the platform based variables
//	which violates the stucture of multi-platform.
//	Thus, we shall not extend this procedure to common handler.
// By Bruce, 2009-01-08.
//
unsigned char
HalSetSysClk8192CD(	struct rtl8192cd_priv *priv,	unsigned char Data)
{
	RTL_W8((SYS_CLKR + 1), Data);
	delay_us(200);
	return TRUE;
}



static void LLT_table_init(struct rtl8192cd_priv *priv)
{
	unsigned txpktbufSz, bufBd;
	unsigned int i, count = 0;

#if 1
	{
		{
			txpktbufSz = 246; // 0xF6
			bufBd = 255;
		}
	}
#else
	unsigned txpktbufSz = 252; //174(0xAE) 120(0x78) 252(0xFC)
#endif


	for ( i = 0; i < txpktbufSz - 1; i++) {
		RTL_W32(LLT_INI, ((LLTE_RWM_WR & LLTE_RWM_Mask) << LLTE_RWM_SHIFT) | (i & LLTINI_ADDR_Mask) << LLTINI_ADDR_SHIFT
				| ((i + 1)&LLTINI_HDATA_Mask) << LLTINI_HDATA_SHIFT);

		count = 0;
		do {
			if (!(RTL_R32(LLT_INI) & ((LLTE_RWM_RD & LLTE_RWM_Mask) << LLTE_RWM_SHIFT)))
				break;
			if (++count >= 100) {
				printk("LLT_init, section 01, i=%d\n", i);
				printk("LLT Polling failed 01 !!!\n");
				return;
			}
		} while (count < 100);
	}

	RTL_W32(LLT_INI, ((LLTE_RWM_WR & LLTE_RWM_Mask) << LLTE_RWM_SHIFT)
			| ((txpktbufSz - 1)&LLTINI_ADDR_Mask) << LLTINI_ADDR_SHIFT | (255 & LLTINI_HDATA_Mask) << LLTINI_HDATA_SHIFT);

	count = 0;
	do {
		if (!(RTL_R32(LLT_INI) & ((LLTE_RWM_RD & LLTE_RWM_Mask) << LLTE_RWM_SHIFT)))
			break;
		if (++count >= 100) {
			printk("LLT Polling failed 02 !!!\n");
			return;
		}
	} while (count < 100);


	for (i = txpktbufSz; i < bufBd; i++) {
		RTL_W32(LLT_INI, ((LLTE_RWM_WR & LLTE_RWM_Mask) << LLTE_RWM_SHIFT) | (i & LLTINI_ADDR_Mask) << LLTINI_ADDR_SHIFT
				| ((i + 1)&LLTINI_HDATA_Mask) << LLTINI_HDATA_SHIFT);

		do {
			if (!(RTL_R32(LLT_INI) & ((LLTE_RWM_RD & LLTE_RWM_Mask) << LLTE_RWM_SHIFT)))
				break;
			if (++count >= 100) {
				printk("LLT Polling failed 03 !!!\n");
				return;
			}
		} while (count < 100);
	}

	RTL_W32(LLT_INI, ((LLTE_RWM_WR & LLTE_RWM_Mask) << LLTE_RWM_SHIFT) | (bufBd & LLTINI_ADDR_Mask) << LLTINI_ADDR_SHIFT
			| (txpktbufSz & LLTINI_HDATA_Mask) << LLTINI_HDATA_SHIFT);

	count = 0;
	do {
		if (!(RTL_R32(LLT_INI) & ((LLTE_RWM_RD & LLTE_RWM_Mask) << LLTE_RWM_SHIFT)))
			break;
		if (++count >= 100) {
			printk("LLT Polling failed 04 !!!\n");
			return;
		}
	} while (count < 100);

// Set reserved page for each queue

#if 1
	/* normal queue init MUST be previoius of RQPN enable */
	//RTL_W8(RQPN_NPQ, 4);		//RQPN_NPQ
	{
		{
			RTL_W8(RQPN_NPQ, 0x29);
			//RTL_W32(RQPN, 0x809f2929);
			//RTL_W32(RQPN, 0x80a82029);
#ifdef DRVMAC_LB
			RTL_W8(RQPN_NPQ + 2, 0x4);
			RTL_W32(RQPN, 0x80380404);
#else
			if (priv->pmib->dot11OperationEntry.wifi_specific == 0) {
				RTL_W32(RQPN, 0x80C50404);
			} else {
				RTL_W32(RQPN, 0x80a92004);
			}
#endif
		}
	}
#else
	if (txpktbufSz == 120)
		RTL_W32(RQPN, 0x80272828);
	else if (txpktbufSz == 252) {
		//RTL_W32(RQPN, 0x80c31c1c);

		// Joseph test
		//RTL_W32(RQPN, 0x80838484);
		RTL_W32(RQPN, 0x80bd1c1c);
	} else
		RTL_W32(RQPN, 0x80393a3a);
#endif


	//RTL_W32(TDECTRL, RTL_R32(TDECTRL)|(txpktbufSz&BCN_HEAD_Mask)<<BCN_HEAD_SHIFT);
	RTL_W8(TXPKTBUF_BCNQ_BDNY, txpktbufSz);
	RTL_W8(TXPKTBUF_MGQ_BDNY, txpktbufSz);
	RTL_W8(TRXFF_BNDY, txpktbufSz);
	RTL_W8(TDECTRL + 1, txpktbufSz);

	RTL_W8(0x45D, txpktbufSz);
}





#if(CONFIG_WLAN_NOT_HAL_EXIST==1)
static int MacInit(struct rtl8192cd_priv *priv)
{
	volatile unsigned char bytetmp;
	unsigned short retry;

	DEBUG_INFO("CP: MacInit===>>");


	RTL_W8(RSV_CTRL0, 0x00);




	// Power on when re-enter from IPS/Radio off/card disable
	{
		/* just don't change BIT(1),Crystal engine setting refine*/
		//RTL_W8(AFE_XTAL_CTRL, 0x0d);	// enable XTAL		// clk inverted
		RTL_W8(SPS0_CTRL, 0x2b);		// enable SPS into PWM
	}
	delay_ms(1);

#if 0
	// Enable AFE BANDGAP
	RTL_W8(AFE_MISC, RTL_R8(AFE_MISC) | AFE_BGEN);
	DEBUG_INFO("AFE_MISC = 0x%02x\n", RTL_R8(AFE_MISC));

	// Enable AFE MBIAS
	RTL_W8(AFE_MISC, RTL_R8(AFE_MISC) | AFE_MBEN);
	DEBUG_INFO("AFE_MISC = 0x%02x\n", RTL_R8(AFE_MISC));

	// Enable PLL Power (LDOA15V)

	// Enable VDDCORE (LDOD12V)
	RTL_W8(LDOV12D_CTRL, RTL_R8(LDOV12D_CTRL) | LDV12_EN);

	// Release XTAL Gated for AFE PLL
//	RTL_W32(AFE_XTAL_CTRL, RTL_R32(AFE_XTAL_CTRL)|XTAL_GATE_AFE);
	RTL_W32(AFE_XTAL_CTRL, RTL_R32(AFE_XTAL_CTRL) & ~XTAL_GATE_AFE);

	// Enable AFE PLL
	RTL_W32(AFE_PLL_CTRL, RTL_R32(AFE_PLL_CTRL) | APLL_EN);

	// Release Isolation AFE PLL & MD
	RTL_W16(SYS_ISO_CTRL, RTL_R16(SYS_ISO_CTRL) & ~ISO_MD2PP);

	// Enable WMAC Clock
	RTL_W16(SYS_CLKR, RTL_R16(SYS_CLKR) | MAC_CLK_EN | SEC_CLK_EN);

	// Release WMAC reset & register reset
	RTL_W16(SYS_FUNC_EN, RTL_R16(SYS_FUNC_EN) | FEN_MREGEN | FEN_DCORE);

	// Release IMEM Isolation
	RTL_W16(SYS_ISO_CTRL, RTL_R16(SYS_ISO_CTRL) & ~(BIT(10) | ISO_DIOR));	//	need to confirm

	/*	// need double setting???
		// Enable MAC IO registers
		RTL_W16(SYS_FUNC_EN, RTL_R16(SYS_FUNC_EN)|FEN_MREGEN);
	*/

	// Switch HWFW select
	RTL_W16(SYS_CLKR, (RTL_R16(SYS_CLKR) | CLKR_80M_SSC_DIS) & ~BIT(6));	//	need to confirm
#else
	// auto enable WLAN

	// Power On Reset for MAC Block
	bytetmp = RTL_R8(APS_FSMCO + 1) | BIT(0);
	delay_us(2);
	RTL_W8(APS_FSMCO + 1, bytetmp);
	delay_us(2);

	bytetmp = RTL_R8(APS_FSMCO + 1);
	delay_us(2);
	retry = 0;
	while ((bytetmp & BIT(0)) && retry < 1000) {
		retry++;
		delay_us(50);
		bytetmp = RTL_R8(APS_FSMCO + 1);
		delay_us(50);
	}

	if (bytetmp & BIT(0)) {
		DEBUG_ERR("%s ERROR: auto enable WLAN failed!!(0x%02X)\n", __FUNCTION__, bytetmp);
	}

		RTL_W16(APS_FSMCO, 0x1012);			// when enable HWPDN

	// release RF digital isolation

	delay_us(2);
#endif

	// Release MAC IO register reset
	RTL_W32(CR, RTL_R32(CR) | MACRXEN | MACTXEN | SCHEDULE_EN | PROTOCOL_EN
			| RXDMA_EN | TXDMA_EN | HCI_RXDMA_EN | HCI_TXDMA_EN);




	//System init
	LLT_table_init(priv);

	// Clear interrupt and enable interrupt
	{
		RTL_W32(HISR, 0xFFFFFFFF);
		RTL_W16(HISRE, 0xFFFF);
	}

		{
			// Set Rx FF0 boundary : 9K/10K
			RTL_W32(TRXFF_BNDY, (RTL_R32(TRXFF_BNDY) & 0x0000FFFF) | (0x27FF & RXFF0_BNDY_Mask) << RXFF0_BNDY_SHIFT);
		}

	{
#if 0//def CONFIG_RTL_88E_SUPPORT
		if (GET_CHIP_VER(priv) == VERSION_8188E)
			RTL_W16(TRXDMA_CTRL, (/*0xF5B1*/ 0xB5B1 | RXSHFT_EN | RXDMA_ARBBW_EN));
		else
#endif
			//RTL_W16(TRXDMA_CTRL, (0xB770 | RXSHFT_EN | RXDMA_ARBBW_EN));

#if defined(CONFIG_RTL_ULINKER_BRSC)
			RTL_W16(TRXDMA_CTRL, (0x5660 | RXDMA_ARBBW_EN)); //disable IP(layer3) auto aligne to 4bytes
#else
				RTL_W16(TRXDMA_CTRL, (0x5660 | RXSHFT_EN | RXDMA_ARBBW_EN));
#endif
	}



//	RTL_W8(TDECTRL, 0x11);	// need to confirm

	// Set Network type: ap mode
	RTL_W32(CR, RTL_R32(CR) | ((NETYPE_AP & NETYPE_Mask) << NETYPE_SHIFT));

	// Set SLOT time
	RTL_W8(SLOT_TIME, 0x09);

	// Set AMPDU min space
	RTL_W8(AMPDU_MIN_SPACE, 0);	//	need to confirm

	// Set Tx/Rx page size (Tx must be 128 Bytes, Rx can be 64,128,256,512,1024 bytes)
	RTL_W8(PBP, (PBP_128B & PSTX_Mask) << PSTX_SHIFT | (PBP_128B & PSRX_Mask) << PSRX_SHIFT);

	// Set RCR register
	RTL_W32(RCR, RCR_APP_FCS | RCR_APP_MIC | RCR_APP_ICV | RCR_APP_PHYSTS | RCR_HTC_LOC_CTRL
			| RCR_AMF | RCR_ADF | RCR_ACRC32 | RCR_AB | RCR_AM | RCR_APM | RCR_AAP);


	// Set Driver info size
	RTL_W8(RX_DRVINFO_SZ, 4);

	// This part is not in WMAC InitMAC()
	// Set SEC register
	RTL_W16(SECCFG, RTL_R16(SECCFG) & ~(RXUSEDK | TXUSEDK));

	// Set TCR register
//	RTL_W32(TCR, RTL_R32(TCR)|CRC|CFE_FORM);
	RTL_W32(TCR, RTL_R32(TCR) | CFE_FORM);

	// Set TCR to avoid deadlock
	RTL_W32(TCR, RTL_R32(TCR) | BIT(15) | BIT(14) | BIT(13) | BIT(12));

	// Set RRSR (response rate set reg)
	//SetResponseRate();
	// Set RRSR (response rate set reg)
	// Set RRSR to all legacy rate and HT rate
	// CCK rate is supported by default.
	// CCK rate will be filtered out only when associated AP does not support it.
	// Only enable ACK rate to OFDM 24M
	{
		RTL_W16(RRSR, 0xFFFF);
		RTL_W8(RRSR + 2, 0xFF);
	}

	// Set Spec SIFS (used in NAV)
	// Joseph test
	RTL_W16(SPEC_SIFS_A, (0x0A & SPEC_SIFS_OFDM_Mask) << SPEC_SIFS_OFDM_SHIFT
			| (0x0A & SPEC_SIFS_CCK_Mask) << SPEC_SIFS_CCK_SHIFT);

	// Set SIFS for CCK
	// Joseph test
	RTL_W16(SIFS_CCK, (0x0A & SIFS_TRX_Mask) << SIFS_TRX_SHIFT | (0x0A & SIFS_CTX_Mask) << SIFS_CTX_SHIFT);

	// Set SIFS for OFDM
	// Joseph test
	RTL_W16(SIFS_OFDM, (0x0A & SIFS_TRX_Mask) << SIFS_TRX_SHIFT | (0x0A & SIFS_CTX_Mask) << SIFS_CTX_SHIFT);

	// Set retry limit
	if (priv->pmib->dot11OperationEntry.dot11LongRetryLimit)
		priv->pshare->RL_setting = priv->pmib->dot11OperationEntry.dot11LongRetryLimit & 0xff;
	else {
#ifdef CLIENT_MODE
	    if (priv->pmib->dot11OperationEntry.opmode & WIFI_STATION_STATE)
			priv->pshare->RL_setting = 0x30;
		else
#endif
		{
			if (priv->pmib->dot11RFEntry.phyBandSelect == PHY_BAND_2G)
				priv->pshare->RL_setting = 0x10;
			else
				priv->pshare->RL_setting = 0x10;
		}
	}
	if (priv->pmib->dot11OperationEntry.dot11ShortRetryLimit)
		priv->pshare->RL_setting |= ((priv->pmib->dot11OperationEntry.dot11ShortRetryLimit & 0xff) << 8);
	else {
#ifdef CLIENT_MODE
	    if (priv->pmib->dot11OperationEntry.opmode & WIFI_STATION_STATE)
			priv->pshare->RL_setting |= (0x30 << 8);
		else
#endif
		{
			if (priv->pmib->dot11RFEntry.phyBandSelect == PHY_BAND_2G)
				priv->pshare->RL_setting |= (0x10 << 8);
			else
				priv->pshare->RL_setting |= (0x10 << 8);
		}
	}
	RTL_W16(RL, priv->pshare->RL_setting);

	//Set Desc Address
	RTL_W32(BCNQ_DESA, priv->pshare->phw->tx_ringB_addr);
	RTL_W32(MGQ_DESA, priv->pshare->phw->tx_ring0_addr);
	RTL_W32(VOQ_DESA, priv->pshare->phw->tx_ring4_addr);
	RTL_W32(VIQ_DESA, priv->pshare->phw->tx_ring3_addr);
	RTL_W32(BEQ_DESA, priv->pshare->phw->tx_ring2_addr);
	RTL_W32(BKQ_DESA, priv->pshare->phw->tx_ring1_addr);
	RTL_W32(HQ_DESA, priv->pshare->phw->tx_ring5_addr);
	RTL_W32(RX_DESA, priv->pshare->phw->ring_dma_addr);
//	RTL_W32(RCDA, priv->pshare->phw->rxcmd_ring_addr);
//	RTL_W32(TCDA, priv->pshare->phw->txcmd_ring_addr);
//	RTL_W32(TCDA, phw->tx_ring5_addr);
	// 2009/03/13 MH Prevent incorrect DMA write after accident reset !!!
//	RTL_W16(CMDR, 0x37FC);

		RTL_W32(PCIE_CTRL_REG, RTL_R32(PCIE_CTRL_REG) | (0x07 & MAX_RXDMA_Mask) << MAX_RXDMA_SHIFT
				| (0x07 & MAX_TXDMA_Mask) << MAX_TXDMA_SHIFT | BCNQSTOP);

	// 20090928 Joseph
	// Reconsider when to do this operation after asking HWSD.
	RTL_W8(APSD_CTRL, RTL_R8(APSD_CTRL) & ~ BIT(6));
	retry = 0;
	do {
		retry++;
		bytetmp = RTL_R8(APSD_CTRL);
	} while ((retry < 200) && (bytetmp & BIT(7))); //polling until BIT7 is 0. by tynli

	if (bytetmp & BIT(7)) {
		DEBUG_ERR("%s ERROR: APSD_CTRL=0x%02X\n", __FUNCTION__, bytetmp);
	}
	// disable BT_enable
	RTL_W8(GPIO_MUXCFG, 0);



#ifdef CONFIG_EXT_CLK_26M
	_InitClockTo26MHz(priv);
#endif


	DEBUG_INFO("DONE\n");

	return TRUE;
}	//	MacInit
#else
static int MacInit(struct rtl8192cd_priv *priv)
{
	return TRUE;
}
#endif//CONFIG_WLAN_NOT_HAL_EXIST

#if(CONFIG_WLAN_NOT_HAL_EXIST==1)
static void MacConfig(struct rtl8192cd_priv *priv)
{
	//RTL_W8(INIRTS_RATE_SEL, 0x8); // 24M
	priv->pshare->phw->RTSInitRate_Candidate = priv->pshare->phw->RTSInitRate = 0x8; // 24M
	RTL_W8(INIRTS_RATE_SEL, priv->pshare->phw->RTSInitRate);

	// 2007/02/07 Mark by Emily becasue we have not verify whether this register works
	//For 92C,which reg?
	//	RTL_W8(BWOPMODE, BW_20M);	//	set if work at 20m

	// Ack timeout.
	if ((priv->pmib->miscEntry.ack_timeout > 0) && (priv->pmib->miscEntry.ack_timeout < 0xff))
		RTL_W8(ACKTO, priv->pmib->miscEntry.ack_timeout);
	else
		RTL_W8(ACKTO, 0x40);

	// clear for mbid beacon tx
	RTL_W8(MULTI_BCNQ_EN, 0);
	RTL_W8(MULTI_BCNQ_OFFSET, 0);

	{
		// set user defining ARFR table for 11n 1T
		RTL_W32(ARFR0, 0xFF015);	// 40M mode
		RTL_W32(ARFR1, 0xFF005);	// 20M mode
	}
	/*
	 * Disable TXOP CFE
	 */
	RTL_W16(RD_CTRL, RTL_R16(RD_CTRL) | DIS_TXOP_CFE);


	/*
	 *	RA try rate aggr limit
	 */
	RTL_W8(RA_TRY_RATE_AGG_LMT, 0);

	/*
	 *	Max mpdu number per aggr
	 */
	RTL_W16(PROT_MODE_CTRL + 2, 0x0909);

	if(!CHIP_VER_92X_SERIES(priv))
		RTL_W8(RESP_SIFS_OFDM+1, 0x0a);		//R2T



}
#else
static void MacConfig(struct rtl8192cd_priv *priv)
{
	return;
}
#endif//CONFIG_WLAN_NOT_HAL_EXIST


unsigned int get_mean_of_2_close_value(unsigned int *val_array)
{
	unsigned int tmp1, tmp2;

	//printk("v1 %08x v2 %08x v3 %08x\n", val_array[0], val_array[1], val_array[2]);
	if (val_array[0] > val_array[1]) {
		tmp1 = val_array[1];
		val_array[1] = val_array[0];
		val_array[0] = tmp1;
	}
	if (val_array[1] > val_array[2]) {
		tmp1 = val_array[2];
		val_array[2] = val_array[1];
		val_array[1] = tmp1;
	}
	if (val_array[0] > val_array[1]) {
		tmp1 = val_array[1];
		val_array[1] = val_array[0];
		val_array[0] = tmp1;
	}
	//printk("v1 %08x v2 %08x v3 %08x\n", val_array[0], val_array[1], val_array[2]);

	tmp1 = val_array[1] - val_array[0];
	tmp2 = val_array[2] - val_array[1];
	if (tmp1 < tmp2)
		tmp1 = (val_array[0] + val_array[1]) / 2;
	else
		tmp1 = (val_array[1] + val_array[2]) / 2;

	//printk("final %08x\n", tmp1);
	return tmp1;
}


void PHY_IQCalibrate(struct rtl8192cd_priv *priv)
{
	unsigned long x;

#ifdef RF_MIMO_SWITCH
	if(priv->pshare->rf_status) {
		if (get_rf_mimo_mode(priv) == MIMO_2T2R)
			set_MIMO_Mode(priv, MIMO_2T2R);
		 else if(get_rf_mimo_mode(priv) == MIMO_3T3R)
		 	set_MIMO_Mode(priv, MIMO_3T3R);
	}
#endif




/*
*   Disable IQK for 8814 MP chip
*/

	if (GET_CHIP_VER(priv) == VERSION_8822B) {
		PHY_IQCalibrate_8822B(ODMPTR, FALSE);
	}


	if (GET_CHIP_VER(priv) == VERSION_8197F) {
		//PHY_IQCalibrate_8197F(ODMPTR, FALSE);
		PHY_IQCalibrate_8197F(ODMPTR, FALSE);

#ifdef CONFIG_RF_DPK_SETTING_SUPPORT
		//if ((ODMPTR->RFCalibrateInfo.bDPPathAOK == 0) && (ODMPTR->RFCalibrateInfo.bDPPathBOK == 0)) {
			//printk("call %s 97F DPK\n",__FUNCTION__);
			//u1Byte k;
			//for (k = 0; k < 3; k++) {
				//do_dpk_8197f(ODMPTR, TRUE, k);
				//}
			//}

		if ((ODMPTR->RFCalibrateInfo.bDPPathAOK == 1) && (ODMPTR->RFCalibrateInfo.bDPPathBOK == 1))
			phy_lut_sram_write_8197f(ODMPTR);
#endif
	}

}


#ifdef ADD_TX_POWER_BY_CMD
static void assign_txpwr_offset(struct rtl8192cd_priv *priv)
{
	ASSIGN_TX_POWER_OFFSET(priv->pshare->phw->CCKTxAgc_A[0], priv->pshare->rf_ft_var.txPowerPlus_cck_11);
	ASSIGN_TX_POWER_OFFSET(priv->pshare->phw->CCKTxAgc_A[1], priv->pshare->rf_ft_var.txPowerPlus_cck_5);
	ASSIGN_TX_POWER_OFFSET(priv->pshare->phw->CCKTxAgc_A[2], priv->pshare->rf_ft_var.txPowerPlus_cck_2);
	ASSIGN_TX_POWER_OFFSET(priv->pshare->phw->CCKTxAgc_A[3], priv->pshare->rf_ft_var.txPowerPlus_cck_1);
	ASSIGN_TX_POWER_OFFSET(priv->pshare->phw->CCKTxAgc_B[0], priv->pshare->rf_ft_var.txPowerPlus_cck_11);
	ASSIGN_TX_POWER_OFFSET(priv->pshare->phw->CCKTxAgc_B[1], priv->pshare->rf_ft_var.txPowerPlus_cck_5);
	ASSIGN_TX_POWER_OFFSET(priv->pshare->phw->CCKTxAgc_B[2], priv->pshare->rf_ft_var.txPowerPlus_cck_2);
	ASSIGN_TX_POWER_OFFSET(priv->pshare->phw->CCKTxAgc_B[3], priv->pshare->rf_ft_var.txPowerPlus_cck_1);

	ASSIGN_TX_POWER_OFFSET(priv->pshare->phw->OFDMTxAgcOffset_A[0], priv->pshare->rf_ft_var.txPowerPlus_ofdm_18);
	ASSIGN_TX_POWER_OFFSET(priv->pshare->phw->OFDMTxAgcOffset_A[1], priv->pshare->rf_ft_var.txPowerPlus_ofdm_12);
	ASSIGN_TX_POWER_OFFSET(priv->pshare->phw->OFDMTxAgcOffset_A[2], priv->pshare->rf_ft_var.txPowerPlus_ofdm_9);
	ASSIGN_TX_POWER_OFFSET(priv->pshare->phw->OFDMTxAgcOffset_A[3], priv->pshare->rf_ft_var.txPowerPlus_ofdm_6);
	ASSIGN_TX_POWER_OFFSET(priv->pshare->phw->OFDMTxAgcOffset_B[0], priv->pshare->rf_ft_var.txPowerPlus_ofdm_18);
	ASSIGN_TX_POWER_OFFSET(priv->pshare->phw->OFDMTxAgcOffset_B[1], priv->pshare->rf_ft_var.txPowerPlus_ofdm_12);
	ASSIGN_TX_POWER_OFFSET(priv->pshare->phw->OFDMTxAgcOffset_B[2], priv->pshare->rf_ft_var.txPowerPlus_ofdm_9);
	ASSIGN_TX_POWER_OFFSET(priv->pshare->phw->OFDMTxAgcOffset_B[3], priv->pshare->rf_ft_var.txPowerPlus_ofdm_6);

	ASSIGN_TX_POWER_OFFSET(priv->pshare->phw->OFDMTxAgcOffset_A[4], priv->pshare->rf_ft_var.txPowerPlus_ofdm_54);
	ASSIGN_TX_POWER_OFFSET(priv->pshare->phw->OFDMTxAgcOffset_A[5], priv->pshare->rf_ft_var.txPowerPlus_ofdm_48);
	ASSIGN_TX_POWER_OFFSET(priv->pshare->phw->OFDMTxAgcOffset_A[6], priv->pshare->rf_ft_var.txPowerPlus_ofdm_36);
	ASSIGN_TX_POWER_OFFSET(priv->pshare->phw->OFDMTxAgcOffset_A[7], priv->pshare->rf_ft_var.txPowerPlus_ofdm_24);
	ASSIGN_TX_POWER_OFFSET(priv->pshare->phw->OFDMTxAgcOffset_B[4], priv->pshare->rf_ft_var.txPowerPlus_ofdm_54);
	ASSIGN_TX_POWER_OFFSET(priv->pshare->phw->OFDMTxAgcOffset_B[5], priv->pshare->rf_ft_var.txPowerPlus_ofdm_48);
	ASSIGN_TX_POWER_OFFSET(priv->pshare->phw->OFDMTxAgcOffset_B[6], priv->pshare->rf_ft_var.txPowerPlus_ofdm_36);
	ASSIGN_TX_POWER_OFFSET(priv->pshare->phw->OFDMTxAgcOffset_B[7], priv->pshare->rf_ft_var.txPowerPlus_ofdm_24);

	ASSIGN_TX_POWER_OFFSET(priv->pshare->phw->MCSTxAgcOffset_A[0], priv->pshare->rf_ft_var.txPowerPlus_mcs_3);
	ASSIGN_TX_POWER_OFFSET(priv->pshare->phw->MCSTxAgcOffset_A[1], priv->pshare->rf_ft_var.txPowerPlus_mcs_2);
	ASSIGN_TX_POWER_OFFSET(priv->pshare->phw->MCSTxAgcOffset_A[2], priv->pshare->rf_ft_var.txPowerPlus_mcs_1);
	ASSIGN_TX_POWER_OFFSET(priv->pshare->phw->MCSTxAgcOffset_A[3], priv->pshare->rf_ft_var.txPowerPlus_mcs_0);
	ASSIGN_TX_POWER_OFFSET(priv->pshare->phw->MCSTxAgcOffset_B[0], priv->pshare->rf_ft_var.txPowerPlus_mcs_3);
	ASSIGN_TX_POWER_OFFSET(priv->pshare->phw->MCSTxAgcOffset_B[1], priv->pshare->rf_ft_var.txPowerPlus_mcs_2);
	ASSIGN_TX_POWER_OFFSET(priv->pshare->phw->MCSTxAgcOffset_B[2], priv->pshare->rf_ft_var.txPowerPlus_mcs_1);
	ASSIGN_TX_POWER_OFFSET(priv->pshare->phw->MCSTxAgcOffset_B[3], priv->pshare->rf_ft_var.txPowerPlus_mcs_0);

	ASSIGN_TX_POWER_OFFSET(priv->pshare->phw->MCSTxAgcOffset_A[4], priv->pshare->rf_ft_var.txPowerPlus_mcs_7);
	ASSIGN_TX_POWER_OFFSET(priv->pshare->phw->MCSTxAgcOffset_A[5], priv->pshare->rf_ft_var.txPowerPlus_mcs_6);
	ASSIGN_TX_POWER_OFFSET(priv->pshare->phw->MCSTxAgcOffset_A[6], priv->pshare->rf_ft_var.txPowerPlus_mcs_5);
	ASSIGN_TX_POWER_OFFSET(priv->pshare->phw->MCSTxAgcOffset_A[7], priv->pshare->rf_ft_var.txPowerPlus_mcs_4);
	ASSIGN_TX_POWER_OFFSET(priv->pshare->phw->MCSTxAgcOffset_B[4], priv->pshare->rf_ft_var.txPowerPlus_mcs_7);
	ASSIGN_TX_POWER_OFFSET(priv->pshare->phw->MCSTxAgcOffset_B[5], priv->pshare->rf_ft_var.txPowerPlus_mcs_6);
	ASSIGN_TX_POWER_OFFSET(priv->pshare->phw->MCSTxAgcOffset_B[6], priv->pshare->rf_ft_var.txPowerPlus_mcs_5);
	ASSIGN_TX_POWER_OFFSET(priv->pshare->phw->MCSTxAgcOffset_B[7], priv->pshare->rf_ft_var.txPowerPlus_mcs_4);

	ASSIGN_TX_POWER_OFFSET(priv->pshare->phw->MCSTxAgcOffset_A[8], priv->pshare->rf_ft_var.txPowerPlus_mcs_11);
	ASSIGN_TX_POWER_OFFSET(priv->pshare->phw->MCSTxAgcOffset_A[9], priv->pshare->rf_ft_var.txPowerPlus_mcs_10);
	ASSIGN_TX_POWER_OFFSET(priv->pshare->phw->MCSTxAgcOffset_A[10], priv->pshare->rf_ft_var.txPowerPlus_mcs_9);
	ASSIGN_TX_POWER_OFFSET(priv->pshare->phw->MCSTxAgcOffset_A[11], priv->pshare->rf_ft_var.txPowerPlus_mcs_8);
	ASSIGN_TX_POWER_OFFSET(priv->pshare->phw->MCSTxAgcOffset_B[8], priv->pshare->rf_ft_var.txPowerPlus_mcs_11);
	ASSIGN_TX_POWER_OFFSET(priv->pshare->phw->MCSTxAgcOffset_B[9], priv->pshare->rf_ft_var.txPowerPlus_mcs_10);
	ASSIGN_TX_POWER_OFFSET(priv->pshare->phw->MCSTxAgcOffset_B[10], priv->pshare->rf_ft_var.txPowerPlus_mcs_9);
	ASSIGN_TX_POWER_OFFSET(priv->pshare->phw->MCSTxAgcOffset_B[11], priv->pshare->rf_ft_var.txPowerPlus_mcs_8);

	ASSIGN_TX_POWER_OFFSET(priv->pshare->phw->MCSTxAgcOffset_A[12], priv->pshare->rf_ft_var.txPowerPlus_mcs_15);
	ASSIGN_TX_POWER_OFFSET(priv->pshare->phw->MCSTxAgcOffset_A[13], priv->pshare->rf_ft_var.txPowerPlus_mcs_14);
	ASSIGN_TX_POWER_OFFSET(priv->pshare->phw->MCSTxAgcOffset_A[14], priv->pshare->rf_ft_var.txPowerPlus_mcs_13);
	ASSIGN_TX_POWER_OFFSET(priv->pshare->phw->MCSTxAgcOffset_A[15], priv->pshare->rf_ft_var.txPowerPlus_mcs_12);
	ASSIGN_TX_POWER_OFFSET(priv->pshare->phw->MCSTxAgcOffset_B[12], priv->pshare->rf_ft_var.txPowerPlus_mcs_15);
	ASSIGN_TX_POWER_OFFSET(priv->pshare->phw->MCSTxAgcOffset_B[13], priv->pshare->rf_ft_var.txPowerPlus_mcs_14);
	ASSIGN_TX_POWER_OFFSET(priv->pshare->phw->MCSTxAgcOffset_B[14], priv->pshare->rf_ft_var.txPowerPlus_mcs_13);
	ASSIGN_TX_POWER_OFFSET(priv->pshare->phw->MCSTxAgcOffset_B[15], priv->pshare->rf_ft_var.txPowerPlus_mcs_12);
}
#endif


void reload_txpwr_pg(struct rtl8192cd_priv *priv)
{
	int i;

	PHY_ConfigBBWithParaFile(priv, PHYREG_PG);

#if 0 //def HIGH_POWER_EXT_PA
	if (!priv->pshare->rf_ft_var.use_ext_pa)
#endif

	{
		// get default Tx AGC offset
		//_TXPWR_REDEFINE ?? CCKTxAgc_A[1] [2] [3] ??
		*(unsigned int *)(&priv->pshare->phw->MCSTxAgcOffset_A[0])	= cpu_to_be32(RTL_R32(rTxAGC_A_Mcs03_Mcs00));
		*(unsigned int *)(&priv->pshare->phw->MCSTxAgcOffset_A[4])	= cpu_to_be32(RTL_R32(rTxAGC_A_Mcs07_Mcs04));
		*(unsigned int *)(&priv->pshare->phw->MCSTxAgcOffset_A[8])	= cpu_to_be32(RTL_R32(rTxAGC_A_Mcs11_Mcs08));
		*(unsigned int *)(&priv->pshare->phw->MCSTxAgcOffset_A[12]) = cpu_to_be32(RTL_R32(rTxAGC_A_Mcs15_Mcs12));
		*(unsigned int *)(&priv->pshare->phw->OFDMTxAgcOffset_A[0]) = cpu_to_be32(RTL_R32(rTxAGC_A_Rate18_06));
		*(unsigned int *)(&priv->pshare->phw->OFDMTxAgcOffset_A[4]) = cpu_to_be32(RTL_R32(rTxAGC_A_Rate54_24));
		*(unsigned int *)(&priv->pshare->phw->CCKTxAgc_A[0]) = cpu_to_be32((RTL_R32(rTxAGC_A_CCK11_2_B_CCK11) & 0xffffff00)
				| RTL_R8(rTxAGC_A_CCK1_Mcs32 + 1));


		*(unsigned int *)(&priv->pshare->phw->MCSTxAgcOffset_B[0])	= cpu_to_be32(RTL_R32(rTxAGC_B_Mcs03_Mcs00));
		*(unsigned int *)(&priv->pshare->phw->MCSTxAgcOffset_B[4])	= cpu_to_be32(RTL_R32(rTxAGC_B_Mcs07_Mcs04));
		*(unsigned int *)(&priv->pshare->phw->MCSTxAgcOffset_B[8])	= cpu_to_be32(RTL_R32(rTxAGC_B_Mcs11_Mcs08));
		*(unsigned int *)(&priv->pshare->phw->MCSTxAgcOffset_B[12]) = cpu_to_be32(RTL_R32(rTxAGC_B_Mcs15_Mcs12));
		*(unsigned int *)(&priv->pshare->phw->OFDMTxAgcOffset_B[0]) = cpu_to_be32(RTL_R32(rTxAGC_B_Rate18_06));
		*(unsigned int *)(&priv->pshare->phw->OFDMTxAgcOffset_B[4]) = cpu_to_be32(RTL_R32(rTxAGC_B_Rate54_24));
		*(unsigned int *)(&priv->pshare->phw->CCKTxAgc_B[0]) = cpu_to_be32((RTL_R8(rTxAGC_A_CCK11_2_B_CCK11) << 24)
				| (RTL_R32(rTxAGC_B_CCK5_1_Mcs32) >> 8));

	}

       if (priv->pshare->txpwr_pg_format_abs)
       {
               //CCK
               if (priv->pmib->dot11RFEntry.phyBandSelect == PHY_BAND_2G) {
                       priv->pshare->tgpwr_CCK_new[RF_PATH_A] = priv->pshare->phw->CCKTxAgc_A[0];
                       priv->pshare->tgpwr_CCK_new[RF_PATH_B] = priv->pshare->phw->CCKTxAgc_B[0];
                       for (i=0;i<4;i++) {
                               priv->pshare->phw->CCKTxAgc_A[i] -= priv->pshare->tgpwr_CCK_new[RF_PATH_A];
                               priv->pshare->phw->CCKTxAgc_B[i] -= priv->pshare->tgpwr_CCK_new[RF_PATH_B];
                       }
               }
               //OFDM
               priv->pshare->tgpwr_OFDM_new[RF_PATH_A] = priv->pshare->phw->OFDMTxAgcOffset_A[4];
               priv->pshare->tgpwr_OFDM_new[RF_PATH_B] = priv->pshare->phw->OFDMTxAgcOffset_B[4];
               for (i=0;i<8;i++) {
                       priv->pshare->phw->OFDMTxAgcOffset_A[i] -= priv->pshare->tgpwr_OFDM_new[RF_PATH_A];
                       priv->pshare->phw->OFDMTxAgcOffset_B[i] -= priv->pshare->tgpwr_OFDM_new[RF_PATH_B];
               }

               //HT-1S
               priv->pshare->tgpwr_HT1S_new[RF_PATH_A] = priv->pshare->phw->MCSTxAgcOffset_A[4];
               priv->pshare->tgpwr_HT1S_new[RF_PATH_B] = priv->pshare->phw->MCSTxAgcOffset_B[4];
               for (i=0;i<8;i++) {
                       priv->pshare->phw->MCSTxAgcOffset_A[i] -= priv->pshare->tgpwr_HT1S_new[RF_PATH_A];
                       priv->pshare->phw->MCSTxAgcOffset_B[i] -= priv->pshare->tgpwr_HT1S_new[RF_PATH_B];
               }

               //HT-2S
               priv->pshare->tgpwr_HT2S_new[RF_PATH_A] = priv->pshare->phw->MCSTxAgcOffset_A[12];
               priv->pshare->tgpwr_HT2S_new[RF_PATH_B] = priv->pshare->phw->MCSTxAgcOffset_B[12];
               for (i=8;i<16;i++) {
                       priv->pshare->phw->MCSTxAgcOffset_A[i] -= priv->pshare->tgpwr_HT2S_new[RF_PATH_A];
                       priv->pshare->phw->MCSTxAgcOffset_B[i] -= priv->pshare->tgpwr_HT2S_new[RF_PATH_B];
               }
#ifdef RTK_AC_SUPPORT
               if ((GET_CHIP_VER(priv)==VERSION_8812E) || (GET_CHIP_VER(priv)==VERSION_8881A)) {
                       //VHT-1S
                       priv->pshare->tgpwr_VHT1S_new[RF_PATH_A] = priv->pshare->phw->VHTTxAgcOffset_A[4];
                       priv->pshare->tgpwr_VHT1S_new[RF_PATH_B] = priv->pshare->phw->VHTTxAgcOffset_B[4];
                       //VHT-2S
                       priv->pshare->tgpwr_VHT2S_new[RF_PATH_A] = priv->pshare->phw->VHTTxAgcOffset_A[18];
                       priv->pshare->tgpwr_VHT2S_new[RF_PATH_B] = priv->pshare->phw->VHTTxAgcOffset_B[18];

                       for (i=0 ; i<20 ; i++) {
                               if (i<8) {
                                       priv->pshare->phw->VHTTxAgcOffset_A[i] -= priv->pshare->tgpwr_VHT1S_new[RF_PATH_A];
                                       priv->pshare->phw->VHTTxAgcOffset_B[i] -= priv->pshare->tgpwr_VHT1S_new[RF_PATH_B];
                               } else if (i==10 || i==11) {
                                       priv->pshare->phw->VHTTxAgcOffset_A[i] = priv->pshare->tgpwr_VHT1S_new[RF_PATH_A] - priv->pshare->phw->VHTTxAgcOffset_A[i];
                                       priv->pshare->phw->VHTTxAgcOffset_B[i] = priv->pshare->tgpwr_VHT1S_new[RF_PATH_B] - priv->pshare->phw->VHTTxAgcOffset_B[i];
                               } else if (i==16 || i==17) {
                                       priv->pshare->phw->VHTTxAgcOffset_A[i] = priv->pshare->tgpwr_VHT2S_new[RF_PATH_A] - priv->pshare->phw->VHTTxAgcOffset_A[i];
                                       priv->pshare->phw->VHTTxAgcOffset_B[i] = priv->pshare->tgpwr_VHT2S_new[RF_PATH_B] - priv->pshare->phw->VHTTxAgcOffset_B[i];
                               } else {
                                       priv->pshare->phw->VHTTxAgcOffset_A[i] -= priv->pshare->tgpwr_VHT2S_new[RF_PATH_A];
                                       priv->pshare->phw->VHTTxAgcOffset_B[i] -= priv->pshare->tgpwr_VHT2S_new[RF_PATH_B];
                               }
                       }
               }
#endif
       }

#ifdef TXPWR_LMT
	if (!priv->pshare->rf_ft_var.disable_txpwrlmt) {
		//4 Set Target Power from PG table values
		if ((GET_CHIP_VER(priv) == VERSION_8192C) || (GET_CHIP_VER(priv) == VERSION_8188E)) //92c_pwrlmt
		{
			if (priv->pmib->dot11RFEntry.phyBandSelect == PHY_BAND_2G) {
				//CCK
				priv->pshare->tgpwr_CCK_new[RF_PATH_A] = priv->pshare->phw->CCKTxAgc_A[0];
				priv->pshare->tgpwr_CCK_new[RF_PATH_B] = priv->pshare->phw->CCKTxAgc_B[0];
				//4 Set PG table values to difference
				for (i=0;i<4;i++) {
					priv->pshare->phw->CCKTxAgc_A[i] -= priv->pshare->tgpwr_CCK_new[RF_PATH_A];
					priv->pshare->phw->CCKTxAgc_B[i] -= priv->pshare->tgpwr_CCK_new[RF_PATH_B];
				}
			}
			//OFDM
			priv->pshare->tgpwr_OFDM_new[RF_PATH_A] = priv->pshare->phw->OFDMTxAgcOffset_A[4];
			priv->pshare->tgpwr_OFDM_new[RF_PATH_B] = priv->pshare->phw->OFDMTxAgcOffset_B[4];
			for (i=0;i<8;i++) {
				priv->pshare->phw->OFDMTxAgcOffset_A[i] -= priv->pshare->tgpwr_OFDM_new[RF_PATH_A];
				priv->pshare->phw->OFDMTxAgcOffset_B[i] -= priv->pshare->tgpwr_OFDM_new[RF_PATH_B];
			}

			//HT-1S
			priv->pshare->tgpwr_HT1S_new[RF_PATH_A] = priv->pshare->phw->MCSTxAgcOffset_A[4];
			priv->pshare->tgpwr_HT1S_new[RF_PATH_B] = priv->pshare->phw->MCSTxAgcOffset_B[4];
			for (i=0;i<8;i++) {
				priv->pshare->phw->MCSTxAgcOffset_A[i] -= priv->pshare->tgpwr_HT1S_new[RF_PATH_A];
				priv->pshare->phw->MCSTxAgcOffset_B[i] -= priv->pshare->tgpwr_HT1S_new[RF_PATH_B];
			}

			//HT-2S
			priv->pshare->tgpwr_HT2S_new[RF_PATH_A] = priv->pshare->phw->MCSTxAgcOffset_A[12];
			priv->pshare->tgpwr_HT2S_new[RF_PATH_B] = priv->pshare->phw->MCSTxAgcOffset_B[12];
			for (i=8;i<16;i++) {
				priv->pshare->phw->MCSTxAgcOffset_A[i] -= priv->pshare->tgpwr_HT2S_new[RF_PATH_A];
				priv->pshare->phw->MCSTxAgcOffset_B[i] -= priv->pshare->tgpwr_HT2S_new[RF_PATH_B];
			}
		}
	}
#endif

#ifdef ADD_TX_POWER_BY_CMD
	assign_txpwr_offset(priv);
#endif
}


#ifdef BT_COEXIST
void bt_coex_init(struct rtl8192cd_priv *priv)
{

	unsigned char H2CCommand[6]={0};
	PHY_SetBBReg(priv, 0x6c0, bMaskDWord, 0x5ddd5ddd);
	PHY_SetBBReg(priv, 0x6c4, bMaskDWord, 0x5fdb5fdb);
	PHY_SetBBReg(priv, 0x6c8, bMaskDWord, 0xffffff);
	PHY_SetBBReg(priv, 0x6cc, bMaskDWord, 0x0);
	PHY_SetBBReg(priv, 0x778, bMaskDWord, 0x1);
	PHY_SetBBReg(priv, 0x790, bMaskDWord, 0x5);
	PHY_SetBBReg(priv, 0x76c, bMaskDWord, 0xc0000);
	PHY_SetBBReg(priv, 0x40, bMaskDWord, 0x200); /* BT reopen issue*/
	PHY_SetBBReg(priv, 0x100, bMaskDWord, 0x314ff);
	PHY_SetBBReg(priv, 0x118, bMaskDWord, 0x107);

	H2CCommand[0] = 0x1;

	FillH2CCmd88XX(priv, 0x71, 6, H2CCommand);

	/* TDMA on
	H2CCommand[0] = 0xe3;
	H2CCommand[1] = 0x12;
	H2CCommand[2] = 0x12;
	H2CCommand[3] = 0x21;
	H2CCommand[4] = 0x10;
	H2CCommand[5] = 0x0;
	delay_ms(10);
	FillH2CCmd88XX(priv, H2C_88XX_BT_TDMA, 6, H2CCommand);
	*/
}
#endif



void ODM_software_init(struct rtl8192cd_priv *priv)
{
	unsigned long ability;
	unsigned int	BoardType = ODM_BOARD_DEFAULT;
	priv->pshare->_dmODM.priv = priv;

	//
	// Init Value
	//
	// 1. u1Byte SupportPlatform
	panic_printk("[ODM_software_init] \n");
	ODM_CmnInfoInit(ODMPTR, ODM_CMNINFO_PLATFORM, ODM_AP);
	PHYDM_InitDebugSetting(ODMPTR);

#if 0// defined(CONFIG_RTL_92E_SUPPORT) && defined(CONFIG_PHYDM_ANTENNA_DIVERSITY)
	ODM_CmnInfoInit(ODMPTR,ODM_CMNINFO_RF_ANTENNA_TYPE,CGCS_RX_HW_ANTDIV);
#endif
	// 2. u4Byte SupportAbility
	ability =	\
			ODM_BB_DIG				|
			ODM_BB_RA_MASK			|
			ODM_BB_FA_CNT			|
			ODM_BB_RATE_ADAPTIVE	|
			ODM_MAC_EDCA_TURBO	|
			ODM_RF_RX_GAIN_TRACK	|
			ODM_RF_CALIBRATION		|
			ODM_BB_DYNAMIC_TXPWR	|
			ODM_RF_TX_PWR_TRACK 	|
			ODM_BB_NHM_CNT		|
			ODM_BB_PRIMARY_CCA	|
			0;




    if (GET_CHIP_VER(priv) == VERSION_8197F) {
        ability =   \
			ODM_BB_DIG				|
			ODM_BB_RA_MASK			|
			ODM_BB_FA_CNT			|
			ODM_BB_RSSI_MONITOR		|
//			ODM_MAC_EDCA_TURBO	|
			ODM_BB_DYNAMIC_TXPWR	|
			ODM_RF_TX_PWR_TRACK 	|
			ODM_RF_CALIBRATION		|
			ODM_BB_CCK_PD			|
			ODM_BB_RATE_ADAPTIVE	|
			ODM_BB_LNA_SAT_CHK		|
			ODM_BB_ADAPTIVE_SOML	|
        0;
    }



//eric-8822 ?? odm ability

		if (GET_CHIP_VER(priv) == VERSION_8822B) {
			ability = \
				ODM_BB_DIG			|
				ODM_BB_RA_MASK			|
				ODM_BB_FA_CNT			|
				ODM_BB_RSSI_MONITOR |
	//			ODM_MAC_EDCA_TURBO	|
				ODM_BB_DYNAMIC_TXPWR	|
				ODM_RF_TX_PWR_TRACK 	|
	//			ODM_RF_CALIBRATION		|
	//			ODM_BB_CCK_PD			|
				ODM_BB_ADAPTIVE_SOML	|
				0;
		}


	if ((OPMODE & WIFI_MP_STATE) || priv->pshare->rf_ft_var.mp_specific) {
		ability = \
			ODM_RF_CALIBRATION 		|
			ODM_RF_TX_PWR_TRACK	|
			ODM_BB_FA_CNT			|
			0;

		if (GET_CHIP_VER(priv) == VERSION_8822B)
			ability |= ODM_BB_DYNAMIC_PSDTOOL;
	}

#if defined(SW_ANT_SWITCH)
	if (priv->pshare->rf_ft_var.antSw_enable)
		ability |= ODM_BB_ANT_DIV;
#endif
#ifdef TX_EARLY_MODE
	ability |= ODM_MAC_EARLY_MODE;
#endif

	ODM_CmnInfoInit(ODMPTR, ODM_CMNINFO_ABILITY, ability);
	ODM_CmnInfoUpdate(ODMPTR, ODM_CMNINFO_ABILITY, ability);

	// 3. u1Byte SupportInterface
	ODM_CmnInfoInit(ODMPTR, ODM_CMNINFO_INTERFACE, ODM_ITRF_PCIE);

	// 4. u4Byte SupportICType
	if (GET_CHIP_VER(priv) == VERSION_8188E)
		ODM_CmnInfoInit(ODMPTR, ODM_CMNINFO_IC_TYPE, ODM_RTL8188E);
	else if (GET_CHIP_VER(priv) == VERSION_8812E)
		ODM_CmnInfoInit(ODMPTR, ODM_CMNINFO_IC_TYPE, ODM_RTL8812);
	else if (GET_CHIP_VER(priv) == VERSION_8881A)
		ODM_CmnInfoInit(ODMPTR, ODM_CMNINFO_IC_TYPE, ODM_RTL8881A);
	else if (GET_CHIP_VER(priv) == VERSION_8192E)
		ODM_CmnInfoInit(ODMPTR, ODM_CMNINFO_IC_TYPE, ODM_RTL8192E);
	else if (GET_CHIP_VER(priv) == VERSION_8814A)
		ODM_CmnInfoInit(ODMPTR, ODM_CMNINFO_IC_TYPE, ODM_RTL8814A);
	else if (GET_CHIP_VER(priv) == VERSION_8723B)
		ODM_CmnInfoInit(ODMPTR, ODM_CMNINFO_IC_TYPE, ODM_RTL8723B);
	else if (GET_CHIP_VER(priv) == VERSION_8822B) //eric-8822
		ODM_CmnInfoInit(ODMPTR, ODM_CMNINFO_IC_TYPE, ODM_RTL8822B);
	else if (GET_CHIP_VER(priv) == VERSION_8197F)
		ODM_CmnInfoInit(ODMPTR, ODM_CMNINFO_IC_TYPE, ODM_RTL8197F);

	// 5. u1Byte CutVersion
	if (IS_TEST_CHIP(priv)) {
		ODM_CmnInfoInit(ODMPTR, ODM_CMNINFO_CUT_VER, ODM_CUT_TEST);
	} else {
		ODM_CmnInfoInit(ODMPTR, ODM_CMNINFO_CUT_VER, (RTL_R32(SYS_CFG) >> 12) & 0xF);

		ODM_CmnInfoInit(ODMPTR, ODM_CMNINFO_MP_TEST_CHIP, 1);
	}

	// 6. u1Byte FabVersion
	ODM_CmnInfoInit(ODMPTR, ODM_CMNINFO_FAB_VER, ODM_TSMC);

	// 7. u1Byte RFType
	if (get_rf_mimo_mode(priv) == MIMO_1T1R)
		ODM_CmnInfoInit(ODMPTR, ODM_CMNINFO_RF_TYPE, ODM_1T1R);
	else if (get_rf_mimo_mode(priv) == MIMO_1T2R)
		ODM_CmnInfoInit(ODMPTR, ODM_CMNINFO_RF_TYPE, ODM_1T2R);
	else if (get_rf_mimo_mode(priv) == MIMO_2T2R)
		ODM_CmnInfoInit(ODMPTR, ODM_CMNINFO_RF_TYPE, ODM_2T2R);
	//else if (get_rf_mimo_mode(priv) == MIMO_2T2R_GREEN)
		//ODM_CmnInfoInit(ODMPTR, ODM_CMNINFO_RF_TYPE, ODM_2T2R_GREEN);
	else if (get_rf_mimo_mode(priv) == MIMO_2T3R)
		ODM_CmnInfoInit(ODMPTR, ODM_CMNINFO_RF_TYPE, ODM_2T3R);
	else if (get_rf_mimo_mode(priv) == MIMO_2T4R)
		ODM_CmnInfoInit(ODMPTR, ODM_CMNINFO_RF_TYPE, ODM_2T4R);
	else if (get_rf_mimo_mode(priv) == MIMO_3T3R)
		ODM_CmnInfoInit(ODMPTR, ODM_CMNINFO_RF_TYPE, ODM_3T3R);
	else if (get_rf_mimo_mode(priv) == MIMO_3T4R)
		ODM_CmnInfoInit(ODMPTR, ODM_CMNINFO_RF_TYPE, ODM_3T4R);
	else if (get_rf_mimo_mode(priv) == MIMO_4T4R)
		ODM_CmnInfoInit(ODMPTR, ODM_CMNINFO_RF_TYPE, ODM_4T4R);
	else
		ODM_CmnInfoInit(ODMPTR, ODM_CMNINFO_RF_TYPE, ODM_XTXR);

	// 8. u1Byte BoardType
#if defined(HIGH_POWER_EXT_PA) && defined(HIGH_POWER_EXT_LNA)
	if(priv->pshare->rf_ft_var.use_ext_pa && priv->pshare->rf_ft_var.use_ext_lna) { //For 88C/92C only
		priv->pmib->dot11RFEntry.trswitch = 1;
	}
#endif


#ifdef HIGH_POWER_EXT_PA
	if(priv->pshare->rf_ft_var.use_ext_pa)
	{
		if(GET_CHIP_VER(priv) == VERSION_8814A)
			BoardType |= (ODM_BOARD_EXT_LNA|ODM_BOARD_EXT_LNA_5G|ODM_BOARD_EXT_PA|ODM_BOARD_EXT_PA_5G);
		else if((GET_CHIP_VER(priv) == VERSION_8812E)||(GET_CHIP_VER(priv) == VERSION_8881A))
			BoardType |= ODM_BOARD_EXT_PA_5G;
		else //2G Chip: 92C/88E/92E
			BoardType |= (ODM_BOARD_EXT_LNA|ODM_BOARD_EXT_PA);
	}
#endif
#ifdef HIGH_POWER_EXT_LNA
	if(priv->pshare->rf_ft_var.use_ext_lna){
		if((GET_CHIP_VER(priv) == VERSION_8812E)||(GET_CHIP_VER(priv) == VERSION_8881A))
			BoardType |= ODM_BOARD_EXT_LNA_5G;
		else if((GET_CHIP_VER(priv) == VERSION_8188E)||(GET_CHIP_VER(priv) == VERSION_8192E))
			BoardType |= ODM_BOARD_EXT_LNA;
	}
#endif
	if(priv->pmib->dot11RFEntry.trswitch)
		BoardType |= ODM_BOARD_EXT_TRSW;

	if((GET_CHIP_VER(priv) == VERSION_8814A)&&((priv->pmib->dot11RFEntry.rfe_type == 7)||(priv->pmib->dot11RFEntry.rfe_type == 9)))
		BoardType = (ODM_BOARD_EXT_LNA|ODM_BOARD_EXT_LNA_5G); // 8814 new internal

	ODM_CmnInfoInit(ODMPTR, ODM_CMNINFO_BOARD_TYPE, BoardType);

	extern unsigned int rtl819x_bond_option(void);
	if(GET_CHIP_VER(priv) == VERSION_8197F){
		if(rtl819x_bond_option() == BSP_BOND_97FN){
			panic_printk("[97F] Bonding Type 97FN, PKG2\n");
			ODM_CmnInfoInit(ODMPTR, ODM_CMNINFO_PACKAGE_TYPE, 2); /* 97FN */
		}else if(rtl819x_bond_option() == BSP_BOND_97FS){
			panic_printk("[97F] Bonding Type 97FS, PKG1\n");
			ODM_CmnInfoInit(ODMPTR, ODM_CMNINFO_PACKAGE_TYPE, 1); /* 97FS */
		}else if(rtl819x_bond_option() == BSP_BOND_97FB){
			panic_printk("[97F] Bonding Type 97FB, PKG0\n");
			ODM_CmnInfoInit(ODMPTR, ODM_CMNINFO_PACKAGE_TYPE, 0); /* 97FB */
		}else{
			panic_printk("[97F] Bonding Type Unknown, PKG1\n");
			ODM_CmnInfoInit(ODMPTR, ODM_CMNINFO_PACKAGE_TYPE, 1); /* Unknown */
		}
	}

	ODM_CmnInfoInit(ODMPTR, ODM_CMNINFO_RFE_TYPE, priv->pmib->dot11RFEntry.rfe_type);

	// ExtLNA & ExtPA Type
	if(GET_CHIP_VER(priv) == VERSION_8812E)
	{
		if(priv->pmib->dot11RFEntry.pa_type == PA_SKYWORKS_5023)
			ODM_CmnInfoInit(ODMPTR, ODM_CMNINFO_APA, 0x05);
		else
			ODM_CmnInfoInit(ODMPTR, ODM_CMNINFO_APA, 0x00);

		ODM_CmnInfoInit(ODMPTR, ODM_CMNINFO_GPA, 0x00);
		ODM_CmnInfoInit(ODMPTR, ODM_CMNINFO_GLNA, 0x00);
		ODM_CmnInfoInit(ODMPTR, ODM_CMNINFO_ALNA, 0x00);
	}
	else if(GET_CHIP_VER(priv) == VERSION_8814A)
	{
		if(priv->pmib->dot11RFEntry.rfe_type == 2) {
			panic_printk("PHY paratemters: RFE type 2 APA1+ALNA1+GPA1+GLNA1\n");
			ODM_CmnInfoInit(ODMPTR, ODM_CMNINFO_GPA, TYPE_GPA1);  // path (A,B,C,D) = (1,1,1,1)
			ODM_CmnInfoInit(ODMPTR, ODM_CMNINFO_APA, TYPE_APA1);
			ODM_CmnInfoInit(ODMPTR, ODM_CMNINFO_GLNA, TYPE_GLNA1);
			ODM_CmnInfoInit(ODMPTR, ODM_CMNINFO_ALNA, TYPE_ALNA1);
		} else if(priv->pmib->dot11RFEntry.rfe_type == 3) {
			panic_printk("PHY paratemters: RFE type 3 APA2+ALNA2+GPA2+GLNA2\n");
			ODM_CmnInfoInit(ODMPTR, ODM_CMNINFO_GPA, TYPE_GPA2);	// path (A,B,C,D) = (2,2,2,2)
			ODM_CmnInfoInit(ODMPTR, ODM_CMNINFO_APA, TYPE_APA2);
			ODM_CmnInfoInit(ODMPTR, ODM_CMNINFO_GLNA, TYPE_GLNA2);
			ODM_CmnInfoInit(ODMPTR, ODM_CMNINFO_ALNA, TYPE_ALNA2);
		} else if(priv->pmib->dot11RFEntry.rfe_type == 4) {
			panic_printk("PHY paratemters: RFE type 4 APA1+ALNA3+GPA1+GLNA1\n");
			ODM_CmnInfoInit(ODMPTR, ODM_CMNINFO_GPA, TYPE_GPA1); // path (A,B,C,D) = (2,2,2,2)
			ODM_CmnInfoInit(ODMPTR, ODM_CMNINFO_APA, TYPE_APA1);
			ODM_CmnInfoInit(ODMPTR, ODM_CMNINFO_GLNA, TYPE_GLNA1); // path (A,B,C,D) = (3,3,3,3)
			ODM_CmnInfoInit(ODMPTR, ODM_CMNINFO_ALNA, TYPE_ALNA3);
		} else if(priv->pmib->dot11RFEntry.rfe_type == 5) {
			panic_printk("PHY paratemters: RFE type 5 APA2+ALNA4+GPA2+GLNA2\n");
			ODM_CmnInfoInit(ODMPTR, ODM_CMNINFO_GPA, TYPE_GPA2);
			ODM_CmnInfoInit(ODMPTR, ODM_CMNINFO_APA, TYPE_APA2);
			ODM_CmnInfoInit(ODMPTR, ODM_CMNINFO_GLNA, TYPE_GLNA2);
			ODM_CmnInfoInit(ODMPTR, ODM_CMNINFO_ALNA, TYPE_ALNA4);
		} else if(priv->pmib->dot11RFEntry.rfe_type == 7) {
			panic_printk("PHY paratemters: RFE type 0 ALNA5+GLNA3\n");
			ODM_CmnInfoInit(ODMPTR, ODM_CMNINFO_GLNA, TYPE_GLNA3);
			ODM_CmnInfoInit(ODMPTR, ODM_CMNINFO_ALNA, TYPE_ALNA5);
		} else if(priv->pmib->dot11RFEntry.rfe_type == 0){
			panic_printk("PHY paratemters: RFE type 0\n");
			ODM_CmnInfoInit(ODMPTR, ODM_CMNINFO_GPA, TYPE_GPA0);
			ODM_CmnInfoInit(ODMPTR, ODM_CMNINFO_APA, TYPE_APA0);
			ODM_CmnInfoInit(ODMPTR, ODM_CMNINFO_GLNA, TYPE_GLNA0);
			ODM_CmnInfoInit(ODMPTR, ODM_CMNINFO_ALNA, TYPE_ALNA0);
		}else {
			panic_printk("PHY paratemters: RFE type %d!!\n",priv->pmib->dot11RFEntry.rfe_type);
			ODM_CmnInfoInit(ODMPTR, ODM_CMNINFO_GPA, TYPE_GPA0);
			ODM_CmnInfoInit(ODMPTR, ODM_CMNINFO_APA, TYPE_APA0);
			ODM_CmnInfoInit(ODMPTR, ODM_CMNINFO_GLNA, TYPE_GLNA0);
			ODM_CmnInfoInit(ODMPTR, ODM_CMNINFO_ALNA, TYPE_ALNA0);
		}
	}

#ifdef HIGH_POWER_EXT_PA
	//priv->pshare->rf_ft_var.use_ext_lna is useless
	if(priv->pshare->rf_ft_var.use_ext_pa)
	{
		// 9. u1Byte ExtLNA
		// 10. u1Byte ExtPA
		if ((GET_CHIP_VER(priv) == VERSION_8188E)||(GET_CHIP_VER(priv) == VERSION_8192E)){
			ODM_CmnInfoInit(ODMPTR, ODM_CMNINFO_EXT_PA, TRUE);
			ODM_CmnInfoInit(ODMPTR, ODM_CMNINFO_EXT_LNA, TRUE);
		} else {
			ODM_CmnInfoInit(ODMPTR, ODM_CMNINFO_EXT_PA, TRUE);
			ODM_CmnInfoInit(ODMPTR, ODM_CMNINFO_5G_EXT_PA, TRUE);
			ODM_CmnInfoInit(ODMPTR, ODM_CMNINFO_EXT_LNA, TRUE);
			ODM_CmnInfoInit(ODMPTR, ODM_CMNINFO_5G_EXT_LNA, TRUE);
		}
	}
#endif
#ifdef HIGH_POWER_EXT_LNA
		if(priv->pshare->rf_ft_var.use_ext_lna){
			// 9. u1Byte ExtLNA
			if ((GET_CHIP_VER(priv) == VERSION_8188E)||(GET_CHIP_VER(priv) == VERSION_8192E)){
				ODM_CmnInfoInit(ODMPTR, ODM_CMNINFO_EXT_LNA, TRUE);
				ODM_CmnInfoInit(ODMPTR, ODM_CMNINFO_EXT_LNA_GAIN, priv->pshare->rf_ft_var.ext_lna_gain);
			} else{
				ODM_CmnInfoInit(ODMPTR, ODM_CMNINFO_EXT_LNA, TRUE);
				ODM_CmnInfoInit(ODMPTR, ODM_CMNINFO_5G_EXT_LNA, TRUE);
			}
		}
#endif


	// 11. u1Byte ExtTRSW, ODM_CMNINFO_EXT_TRSW:
	// follows variable "trswitch" which is modified in rtl8192cd_init_hw_PCI().
	ODM_CmnInfoInit(ODMPTR, ODM_CMNINFO_EXT_TRSW, priv->pmib->dot11RFEntry.trswitch);

	// 12. u1Byte PatchID
	ODM_CmnInfoInit(ODMPTR, ODM_CMNINFO_PATCH_ID, 0);

	// 13. BOOLEAN bInHctTest
	ODM_CmnInfoInit(ODMPTR, ODM_CMNINFO_BINHCT_TEST, FALSE);

	// 14. BOOLEAN bWIFITest
	ODM_CmnInfoInit(ODMPTR, ODM_CMNINFO_BWIFI_TEST, (priv->pmib->dot11OperationEntry.wifi_specific > 0));

	// 15. BOOLEAN bDualMacSmartConcurrent
	ODM_CmnInfoInit(ODMPTR, ODM_CMNINFO_SMART_CONCURRENT, FALSE);

	// Config BB/RF by ODM
	if ((GET_CHIP_VER(priv) == VERSION_8188E) || (GET_CHIP_VER(priv) == VERSION_8814A) || (GET_CHIP_VER(priv) == VERSION_8822B) || (GET_CHIP_VER(priv) == VERSION_8197F) || (GET_CHIP_VER(priv) == VERSION_8723B)) //eric-8822 ??
		ODM_CmnInfoInit(ODMPTR, ODM_CMNINFO_CONFIG_BB_RF, TRUE);
	else
		ODM_CmnInfoInit(ODMPTR, ODM_CMNINFO_CONFIG_BB_RF, FALSE);

	//
	// Dynamic Value
	//

	// 1. u1Byte *pMacPhyMode
	ODM_CmnInfoHook(ODMPTR, ODM_CMNINFO_MAC_PHY_MODE, &priv->pmib->dot11RFEntry.macPhyMode);

	// 2. u8Byte *pNumTxBytesUnicast
	ODM_CmnInfoHook(ODMPTR, ODM_CMNINFO_TX_UNI, &priv->pshare->NumTxBytesUnicast);

	// 3. u8Byte *pNumRxBytesUnicast
	ODM_CmnInfoHook(ODMPTR, ODM_CMNINFO_RX_UNI, &priv->pshare->NumRxBytesUnicast);

	// 4. u1Byte *pWirelessMode
	ODM_CmnInfoHook(ODMPTR, ODM_CMNINFO_WM_MODE, &priv->pmib->dot11BssType.net_work_type);

	// 5. u1Byte *pBandType
	ODM_CmnInfoHook(ODMPTR, ODM_CMNINFO_BAND, &priv->pmib->dot11RFEntry.phyBandSelect);

	// 6. u1Byte *pSecChOffset
	ODM_CmnInfoHook(ODMPTR, ODM_CMNINFO_SEC_CHNL_OFFSET, &priv->pshare->offset_2nd_chan);

	// 7. u1Byte *pSecurity
	ODM_CmnInfoHook(ODMPTR, ODM_CMNINFO_SEC_MODE, &priv->pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm);

	// 8. u1Byte *pBandWidth
	ODM_CmnInfoHook(ODMPTR, ODM_CMNINFO_BW, &priv->pshare->CurrentChannelBW);

	// 9. u1Byte *pChannel
//	ODM_CmnInfoHook(ODMPTR, ODM_CMNINFO_CHNL, &priv->pshare->working_channel);
	ODM_CmnInfoHook(ODMPTR, ODM_CMNINFO_CHNL, &priv->pmib->dot11RFEntry.dot11channel);

	// 10. BOOLEAN *pbMasterOfDMSP
//	ODM_CmnInfoHook(ODMPTR, ODM_CMNINFO_DMSP_IS_MASTER, NULL);
	ODM_CmnInfoHook(ODMPTR, ODM_CMNINFO_DMSP_IS_MASTER, &priv->pshare->dummy);


	// 11. BOOLEAN *pbGetValueFromOtherMac
//	ODM_CmnInfoHook(ODMPTR, ODM_CMNINFO_DMSP_GET_VALUE, NULL);
	ODM_CmnInfoHook(ODMPTR, ODM_CMNINFO_DMSP_GET_VALUE, &priv->pshare->dummy);

	// 12. PADAPTER *pBuddyAdapter
	ODM_CmnInfoHook(ODMPTR, ODM_CMNINFO_BUDDY_ADAPTOR, NULL);

	// 13. BOOLEAN *pbBTOperation
//	ODM_CmnInfoHook(ODMPTR, ODM_CMNINFO_BT_OPERATION, NULL);
	ODM_CmnInfoHook(ODMPTR, ODM_CMNINFO_BT_OPERATION, &priv->pshare->dummy);

	// 14. BOOLEAN *pbBTDisableEDCATurbo
//	ODM_CmnInfoHook(pOdm, ODM_CMNINFO_BT_DISABLE_EDCA, NULL);
	ODM_CmnInfoHook(ODMPTR, ODM_CMNINFO_BT_DISABLE_EDCA, &priv->pshare->dummy);


	// 15. BOOLEAN *pbScanInProcess
	ODM_CmnInfoHook(ODMPTR, ODM_CMNINFO_SCAN, &priv->pshare->bScanInProcess);

	// 16. pU4byte force data rate add by YuChen
	ODM_CmnInfoHook(ODMPTR, ODM_CMNINFO_FORCED_RATE, &priv->pshare->current_tx_rate);

	ODM_CmnInfoHook(ODMPTR, ODM_CMNINFO_POWER_SAVING, &priv->pshare->dummy);


	ODM_CmnInfoHook(ODMPTR, ODM_CMNINFO_ONE_PATH_CCA, &priv->pshare->rf_ft_var.one_path_cca);


// dummy

	ODM_CmnInfoHook(ODMPTR, ODM_CMNINFO_DRV_STOP, &priv->pshare->dummy);
	ODM_CmnInfoHook(ODMPTR, ODM_CMNINFO_PNP_IN, &priv->pshare->dummy);
	ODM_CmnInfoHook(ODMPTR, ODM_CMNINFO_INIT_ON, &priv->pshare->dummy);
	ODM_CmnInfoHook(ODMPTR, ODM_CMNINFO_BT_BUSY, &priv->pshare->dummy);
//	ODM_CmnInfoHook(ODMPTR, ODM_CMNINFO_ANT_DIV, &priv->pshare->dummy);

	//For Phy para verison
	ODM_GetHWImgVersion(ODMPTR);


// DM parameters init
//	ODM_DMInit(ODMPTR);

	ODM_InitAllTimers(ODMPTR);

#ifdef TPT_THREAD
	if (IS_ROOT_INTERFACE(priv))
#ifdef CHECK_HANGUP
	if (!priv->reset_hangup)
#endif
	if (!priv->pshare->switch_chan_rp)
		ODM_InitAllThreads(ODMPTR);
#endif
}


// TODO : TEMP add by Eric , check 8197F need add ?


void set_target_power(struct rtl8192cd_priv *priv)
{
	int i;
	if (priv->pmib->dot11RFEntry.phyBandSelect == PHY_BAND_2G) {
		//CCK
		priv->pshare->tgpwr_CCK_new[RF_PATH_A] = priv->pshare->phw->CCKTxAgc_A[0];
		priv->pshare->tgpwr_CCK_new[RF_PATH_B] = priv->pshare->phw->CCKTxAgc_B[0];
		for (i=0;i<4;i++) {
			priv->pshare->phw->CCKTxAgc_A[i] -= priv->pshare->tgpwr_CCK_new[RF_PATH_A];
			priv->pshare->phw->CCKTxAgc_B[i] -= priv->pshare->tgpwr_CCK_new[RF_PATH_B];
		}
	}
	//OFDM
	priv->pshare->tgpwr_OFDM_new[RF_PATH_A] = priv->pshare->phw->OFDMTxAgcOffset_A[4];
	priv->pshare->tgpwr_OFDM_new[RF_PATH_B] = priv->pshare->phw->OFDMTxAgcOffset_B[4];
	for (i=0;i<8;i++) {
		priv->pshare->phw->OFDMTxAgcOffset_A[i] -= priv->pshare->tgpwr_OFDM_new[RF_PATH_A];
		priv->pshare->phw->OFDMTxAgcOffset_B[i] -= priv->pshare->tgpwr_OFDM_new[RF_PATH_B];
	}

	//HT-1S
	priv->pshare->tgpwr_HT1S_new[RF_PATH_A] = priv->pshare->phw->MCSTxAgcOffset_A[4];
	priv->pshare->tgpwr_HT1S_new[RF_PATH_B] = priv->pshare->phw->MCSTxAgcOffset_B[4];
	for (i=0;i<8;i++) {
		priv->pshare->phw->MCSTxAgcOffset_A[i] -= priv->pshare->tgpwr_HT1S_new[RF_PATH_A];
		priv->pshare->phw->MCSTxAgcOffset_B[i] -= priv->pshare->tgpwr_HT1S_new[RF_PATH_B];
	}

	//HT-2S
	priv->pshare->tgpwr_HT2S_new[RF_PATH_A] = priv->pshare->phw->MCSTxAgcOffset_A[12];
	priv->pshare->tgpwr_HT2S_new[RF_PATH_B] = priv->pshare->phw->MCSTxAgcOffset_B[12];
	for (i=8;i<16;i++) {
		priv->pshare->phw->MCSTxAgcOffset_A[i] -= priv->pshare->tgpwr_HT2S_new[RF_PATH_A];
		priv->pshare->phw->MCSTxAgcOffset_B[i] -= priv->pshare->tgpwr_HT2S_new[RF_PATH_B];
	}
#ifdef RTK_AC_SUPPORT
	if ((GET_CHIP_VER(priv)==VERSION_8812E) || (GET_CHIP_VER(priv)==VERSION_8881A) ||(GET_CHIP_VER(priv) == VERSION_8814A)) {
	//VHT-1S
	priv->pshare->tgpwr_VHT1S_new[RF_PATH_A] = priv->pshare->phw->VHTTxAgcOffset_A[4];
	priv->pshare->tgpwr_VHT1S_new[RF_PATH_B] = priv->pshare->phw->VHTTxAgcOffset_B[4];
	//VHT-2S
	priv->pshare->tgpwr_VHT2S_new[RF_PATH_A] = priv->pshare->phw->VHTTxAgcOffset_A[18];
	priv->pshare->tgpwr_VHT2S_new[RF_PATH_B] = priv->pshare->phw->VHTTxAgcOffset_B[18];

	for (i=0 ; i<20 ; i++) {
		if (i<8) {
			priv->pshare->phw->VHTTxAgcOffset_A[i] -= priv->pshare->tgpwr_VHT1S_new[RF_PATH_A];
			priv->pshare->phw->VHTTxAgcOffset_B[i] -= priv->pshare->tgpwr_VHT1S_new[RF_PATH_B];
		} else if (i==10 || i==11) {
			priv->pshare->phw->VHTTxAgcOffset_A[i] = priv->pshare->tgpwr_VHT1S_new[RF_PATH_A] - priv->pshare->phw->VHTTxAgcOffset_A[i];
			priv->pshare->phw->VHTTxAgcOffset_B[i] = priv->pshare->tgpwr_VHT1S_new[RF_PATH_B] - priv->pshare->phw->VHTTxAgcOffset_B[i];
		} else if (i==16 || i==17) {
			priv->pshare->phw->VHTTxAgcOffset_A[i] = priv->pshare->tgpwr_VHT2S_new[RF_PATH_A] - priv->pshare->phw->VHTTxAgcOffset_A[i];
			priv->pshare->phw->VHTTxAgcOffset_B[i] = priv->pshare->tgpwr_VHT2S_new[RF_PATH_B] - priv->pshare->phw->VHTTxAgcOffset_B[i];
		} else {
			priv->pshare->phw->VHTTxAgcOffset_A[i] -= priv->pshare->tgpwr_VHT2S_new[RF_PATH_A];
			priv->pshare->phw->VHTTxAgcOffset_B[i] -= priv->pshare->tgpwr_VHT2S_new[RF_PATH_B];
		}
	}
	}
#endif
#if 0
	panic_printk("[%s]\n",__FUNCTION__);
	for(i=0;i<4;i++)
		printk("priv->pshare->phw->CCKTxAgc_A[%d]=%x\n",i,priv->pshare->phw->CCKTxAgc_A[i]);
	for(i=0;i<8;i++)
		printk("priv->pshare->phw->OFDMTxAgcOffset_A[%d]=%x\n",i,priv->pshare->phw->OFDMTxAgcOffset_A[i]);
	for(i=0;i<16;i++)
		printk("priv->pshare->phw->MCSTxAgcOffset_A[%d]=%x\n",i,priv->pshare->phw->MCSTxAgcOffset_A[i]);
	for(i=0;i<4;i++)
		printk("priv->pshare->phw->CCKTxAgc_B[%d]=%x\n",i,priv->pshare->phw->CCKTxAgc_B[i]);
	for(i=0;i<8;i++)
		printk("priv->pshare->phw->OFDMTxAgcOffset_B[%d]=%x\n",i,priv->pshare->phw->OFDMTxAgcOffset_B[i]);
	for(i=0;i<16;i++)
		printk("priv->pshare->phw->MCSTxAgcOffset_B[%d]=%x\n",i,priv->pshare->phw->MCSTxAgcOffset_B[i]);
#endif

}


void set_target_power_8814(struct rtl8192cd_priv *priv)
{
	int i;
	if (priv->pmib->dot11RFEntry.phyBandSelect == PHY_BAND_2G) {
		priv->pshare->tgpwr_CCK_new[RF_PATH_A] = priv->pshare->phw->CCKTxAgc_A[3]; /* CCK 11M */
		priv->pshare->tgpwr_CCK_new[RF_PATH_B] = priv->pshare->phw->CCKTxAgc_B[3];
		for (i=0;i<4;i++) {
			priv->pshare->phw->CCKTxAgc_A[i] -= priv->pshare->tgpwr_CCK_new[RF_PATH_A];
			priv->pshare->phw->CCKTxAgc_B[i] -= priv->pshare->tgpwr_CCK_new[RF_PATH_B];
		}
	}
	priv->pshare->tgpwr_OFDM_new[RF_PATH_A] = priv->pshare->phw->OFDMTxAgcOffset_A[7]; /* OFDM 54M */
	priv->pshare->tgpwr_OFDM_new[RF_PATH_B] = priv->pshare->phw->OFDMTxAgcOffset_B[7];
	for (i=0;i<8;i++) {
		priv->pshare->phw->OFDMTxAgcOffset_A[i] -= priv->pshare->tgpwr_OFDM_new[RF_PATH_A];
		priv->pshare->phw->OFDMTxAgcOffset_B[i] -= priv->pshare->tgpwr_OFDM_new[RF_PATH_B];
	}

	priv->pshare->tgpwr_HT1S_new[RF_PATH_A] = priv->pshare->phw->MCSTxAgcOffset_A[7]; /* MCS 7 */
	priv->pshare->tgpwr_HT1S_new[RF_PATH_B] = priv->pshare->phw->MCSTxAgcOffset_B[7];
	for (i=0;i<8;i++) {
		priv->pshare->phw->MCSTxAgcOffset_A[i] -= priv->pshare->tgpwr_HT1S_new[RF_PATH_A];
		priv->pshare->phw->MCSTxAgcOffset_B[i] -= priv->pshare->tgpwr_HT1S_new[RF_PATH_B];
	}

	priv->pshare->tgpwr_HT2S_new[RF_PATH_A] = priv->pshare->phw->MCSTxAgcOffset_A[15]; /* MCS 15 */
	priv->pshare->tgpwr_HT2S_new[RF_PATH_B] = priv->pshare->phw->MCSTxAgcOffset_B[15];
	for (i=8;i<16;i++) {
		priv->pshare->phw->MCSTxAgcOffset_A[i] -= priv->pshare->tgpwr_HT2S_new[RF_PATH_A];
		priv->pshare->phw->MCSTxAgcOffset_B[i] -= priv->pshare->tgpwr_HT2S_new[RF_PATH_B];
	}
	priv->pshare->tgpwr_VHT1S_new[RF_PATH_A] = priv->pshare->phw->VHTTxAgcOffset_A[7]; /* VHT MCS 7 */
	priv->pshare->tgpwr_VHT1S_new[RF_PATH_B] = priv->pshare->phw->VHTTxAgcOffset_B[7];
	priv->pshare->tgpwr_VHT2S_new[RF_PATH_A] = priv->pshare->phw->VHTTxAgcOffset_A[17]; /* VHT NSS2 MCS7 */
	priv->pshare->tgpwr_VHT2S_new[RF_PATH_B] = priv->pshare->phw->VHTTxAgcOffset_B[17];
	for (i=0 ; i<30 ; i++) {
		if (i < 8) {
			priv->pshare->phw->VHTTxAgcOffset_A[i] -= priv->pshare->tgpwr_VHT1S_new[RF_PATH_A];
			priv->pshare->phw->VHTTxAgcOffset_B[i] -= priv->pshare->tgpwr_VHT1S_new[RF_PATH_B];
		}else if(i == 8 || i == 9) {	/* VHT NSS1 MCS8,9 */
			priv->pshare->phw->VHTTxAgcOffset_A[i] = priv->pshare->tgpwr_VHT1S_new[RF_PATH_A] - priv->pshare->phw->VHTTxAgcOffset_A[i];
			priv->pshare->phw->VHTTxAgcOffset_B[i] = priv->pshare->tgpwr_VHT1S_new[RF_PATH_B] - priv->pshare->phw->VHTTxAgcOffset_B[i];
		}else if(i < 18){
			priv->pshare->phw->VHTTxAgcOffset_A[i] -= priv->pshare->tgpwr_VHT2S_new[RF_PATH_A];
			priv->pshare->phw->VHTTxAgcOffset_B[i] -= priv->pshare->tgpwr_VHT2S_new[RF_PATH_B];
		}else if(i == 18 || i == 19) {	 /* VHT NSS2 MCS8,9 */
			priv->pshare->phw->VHTTxAgcOffset_A[i] = priv->pshare->tgpwr_VHT2S_new[RF_PATH_A] - priv->pshare->phw->VHTTxAgcOffset_A[i];
			priv->pshare->phw->VHTTxAgcOffset_B[i] = priv->pshare->tgpwr_VHT2S_new[RF_PATH_B] - priv->pshare->phw->VHTTxAgcOffset_B[i];
		}
	}
#if 0
	for(i=0;i<4;i++)
		printk("priv->pshare->phw->CCKTxAgc_A[%d]=%x\n",i,priv->pshare->phw->CCKTxAgc_A[i]);
	for(i=0;i<8;i++)
		printk("priv->pshare->phw->OFDMTxAgcOffset_A[%d]=%x\n",i,priv->pshare->phw->OFDMTxAgcOffset_A[i]);
	for(i=0;i<24;i++)
		printk("priv->pshare->phw->MCSTxAgcOffset_A[%d]=%x\n",i,priv->pshare->phw->MCSTxAgcOffset_A[i]);
	for(i=0;i<30;i++)
		printk("priv->pshare->phw->VHTTxAgcOffset_A[%d]=%x\n",i,priv->pshare->phw->VHTTxAgcOffset_A[i]);
	for(i=0;i<4;i++)
		printk("priv->pshare->phw->CCKTxAgc_B[%d]=%x\n",i,priv->pshare->phw->CCKTxAgc_B[i]);
	for(i=0;i<8;i++)
		printk("priv->pshare->phw->OFDMTxAgcOffset_B[%d]=%x\n",i,priv->pshare->phw->OFDMTxAgcOffset_B[i]);
	for(i=0;i<24;i++)
		printk("priv->pshare->phw->MCSTxAgcOffset_B[%d]=%x\n",i,priv->pshare->phw->MCSTxAgcOffset_B[i]);
	for(i=0;i<30;i++)
		printk("priv->pshare->phw->VHTTxAgcOffset_B[%d]=%x\n",i,priv->pshare->phw->VHTTxAgcOffset_B[i]);
	for(i=0;i<4;i++)
		printk("priv->pshare->phw->CCKTxAgc_C[%d]=%x\n",i,priv->pshare->phw->CCKTxAgc_C[i]);
	for(i=0;i<8;i++)
		printk("priv->pshare->phw->OFDMTxAgcOffset_C[%d]=%x\n",i,priv->pshare->phw->OFDMTxAgcOffset_C[i]);
	for(i=0;i<24;i++)
		printk("priv->pshare->phw->MCSTxAgcOffset_C[%d]=%x\n",i,priv->pshare->phw->MCSTxAgcOffset_C[i]);
	for(i=0;i<30;i++)
		printk("priv->pshare->phw->VHTTxAgcOffset_C[%d]=%x\n",i,priv->pshare->phw->VHTTxAgcOffset_C[i]);
	for(i=0;i<4;i++)
		printk("priv->pshare->phw->CCKTxAgc_D[%d]=%x\n",i,priv->pshare->phw->CCKTxAgc_D[i]);
	for(i=0;i<8;i++)
		printk("priv->pshare->phw->OFDMTxAgcOffset_D[%d]=%x\n",i,priv->pshare->phw->OFDMTxAgcOffset_D[i]);
	for(i=0;i<24;i++)
		printk("priv->pshare->phw->MCSTxAgcOffset_D[%d]=%x\n",i,priv->pshare->phw->MCSTxAgcOffset_D[i]);
	for(i=0;i<30;i++)
		printk("priv->pshare->phw->VHTTxAgcOffset_D[%d]=%x\n",i,priv->pshare->phw->VHTTxAgcOffset_D[i]);
#endif
}

void set_8822_trx_regs(struct rtl8192cd_priv *priv)
{
	unsigned tmp32 = 0;
	unsigned tmp8  = 0;

	panic_printk("[%s] +++ \n", __FUNCTION__);

//0x804[3] = 1
	tmp8 = RTL_R8(0x804);
	tmp8 |= BIT3;
	RTL_W8(0x804, tmp8);

//0x73=4
	RTL_W8(0x73, 4);

//0x1704 = 0000 0000
	RTL_W32(0x1704, 0);

//0x1700 = c00f 0038
	RTL_W32(0x1700, 0xc00f0038);

//0x1080[16] = 1
	tmp32 = RTL_R32(0x1080);
	tmp32 |= BIT16;
	RTL_W32(0x1080, tmp32);

//0x523[7] = 1
	tmp8 = RTL_R8(0x523);
	tmp8 |= BIT7;
	RTL_W8(0x523, tmp8);


// ====== special mod by eric-8822 ========

//0x8a4= 7f7f 7f7f (edcca)
	RTL_W32(0x8a4, 0x7f7f7f7f);

//0x524[10] = 1
	tmp32 = RTL_R32(0x524);
	tmp32 |= BIT10;
	RTL_W32(0x524, tmp32);

//0x55d = 0x10
	RTL_W8(0x55d, 0x10);

//0x608[28] = 1; Enable PHYST for rssi level
	tmp32 = RTL_R32(0x608);
	tmp32 |= BIT28;
	RTL_W32(0x608, tmp32);

	RTL_W8(0x60f, 4);

//0x29 = 0xa9 from RF-Brian refine TX EVM
	RTL_W8(0x29, 0xa9);

#if 0 //eric-8822 TX HANG
//0x26= 0xcf Set MAC clock from 40M to 80M
	RTL_W8(0x26, 0xcf);

// Stop MGMT Q
// 0x522 = 0x10
//	RTL_W8(0x522, 0x10);
// 0x300[13] = 1
//	tmp32 = RTL_R32(0x300);
//	tmp32 |= BIT13;
//	RTL_W32(0x300, tmp32);
#endif


#if 1 //eric-8822 tp
	RTL_W8(0x420, 0);
	RTL_W16(0x4ca, 0x3b3b);
	RTL_W8(0x455, 0x70);
	RTL_W32(0x4bc, 0);
	if (GET_ROOT(priv)->vap_count == 0)
		RTL_W8(0x5BE,0x08);
	//panic_printk("Disable SOML, Set 0x19a8 form 0x%x to 0x0 \n", RTL_R32(0x19a8));
	//RTL_W32(0x19a8, 0);

	RTL_W8(0x60c, (RTL_R8(0x60c)|0x3f));

	RTL_W8(0x7d4, 0x98);
#endif

#if 1 //eric-mu set REG
	RTL_W8(0x719, 0x82);

	if(priv->pshare->rf_ft_var.mu_retry){
		if((RTL_R8(0x42a) & 0x3f) <= priv->pshare->rf_ft_var.mu_retry)
			RTL_W8(0x42a, (priv->pshare->rf_ft_var.mu_retry+1));

		if((RTL_R8(0x42b) & 0x3f) <= priv->pshare->rf_ft_var.mu_retry)
			RTL_W8(0x42b, (priv->pshare->rf_ft_var.mu_retry+1));
	} else {
		RTL_W8(0x42a, 0xb);
		RTL_W8(0x42b, 0xb);
	}
#endif


	//for MU performance
	//0x528[17] = 1
	tmp32 = RTL_R32(0x528);
	tmp32 |= BIT17;
	RTL_W32(0x528, tmp32);

#ifdef DEBUG_8822TX
		RTL_W8(0x520, 0x7);
    //panic_printk("\n>>> 0x520=0x%x\n", RTL_R8(0x520));
    RTL_W8(0x515, 0x10);
    //panic_printk("\n>>> 0x515=0x%x\n", RTL_R8(0x515));
    RTL_W8(0x429, 0x20);
    //panic_printk("\n>>> 0x429=0x%x\n", RTL_R8(0x429));
    RTL_W8(NAV_PROT_LEN, 0x40);
    //panic_printk("\n>>> 0x546=0x%x\n\n", RTL_R8(NAV_PROT_LEN));
#endif

#ifdef MP_TEST
	if(priv->pshare->rf_ft_var.mp_specific)
	RTL_W32(0x19a8, 0x00);	// Disable SoftML, SoftML make Rx Sensitivity worse in some cases (from RF-Brian)
#endif

	if (PHY_QueryRFReg(priv, RF92CD_PATH_A, 0xC9, BIT7|BIT6|BIT5|BIT4|BIT3, 1) <= 3) {
		printk("%s:%d 0xC9[7:3]=%d\n", __FUNCTION__, __LINE__, PHY_QueryRFReg(priv, RF92CD_PATH_A, 0xC9, BIT7|BIT6|BIT5|BIT4|BIT3, 1));
		PHY_SetRFReg(priv, RF92CD_PATH_A, 0xCA, BIT19, 0) ;
		PHY_SetRFReg(priv, RF92CD_PATH_A, 0xB2, BIT18|BIT17|BIT16|BIT15|BIT14, 6) ;
	}


	// Mod for extFEM MAX input power
	 if((priv->pmib->dot11RFEntry.rfe_type == 1)
	  ||(priv->pmib->dot11RFEntry.rfe_type == 6)
	  ||(priv->pmib->dot11RFEntry.rfe_type == 7)
	  ||(priv->pmib->dot11RFEntry.rfe_type == 9)
	  ||(priv->pmib->dot11RFEntry.rfe_type == 11))
	 RTL_W8(0x847, 0x4d);

//==================================

}

void set_lck_cv(struct rtl8192cd_priv *priv, unsigned char channel)
{
	u1Byte eRFPath, cv=0, ch;
	u1Byte delta[14] = {0,4,4,4,4,4,4,4,4,4,4,4,4,12};
	cv = priv->pshare->rf_ft_var.cv_ch1;
	for(ch=2; ch <= channel; ch++)
		cv += delta[ch-1];
	PHY_SetRFReg(priv, RF92CD_PATH_A, 0xC9, BIT13, 1); /* enable manually setting cv */
	PHY_SetRFReg(priv, RF92CD_PATH_A, 0xB2, 0xff, cv);
	//panic_printk("[%s] center channel=%u,cv=%x\n",__FUNCTION__, channel, cv);
    return;
}


// Note: Not all FW version support H2C cmd for disable beacon when edcca on
// 92e -> v28+, 8812 -> v43+, 8881a -> v10+
void set_bcn_dont_ignore_edcca(struct rtl8192cd_priv *priv)
{
	BOOLEAN carrier_sense_enable = FALSE,
			bcn_dont_ignore_edcca = priv->pshare->rf_ft_var.bcn_dont_ignore_edcca,
			adaptivity_enable = priv->pshare->rf_ft_var.adaptivity_enable;

	if (IS_OUTSRC_CHIP(priv))
		carrier_sense_enable = ODMPTR->Carrier_Sense_enable;

	if ( carrier_sense_enable || (/*adaptivity_enable &&*/ bcn_dont_ignore_edcca) ) {

		DEBUG_INFO("set bcn care edcca\n");

		if ( (GET_CHIP_VER(priv) == VERSION_8881A) || (GET_CHIP_VER(priv)== VERSION_8192E) || (GET_CHIP_VER(priv)==VERSION_8814A) || (GET_CHIP_VER(priv)==VERSION_8197F) || (GET_CHIP_VER(priv)==VERSION_8822B)) {
			unsigned char H2CCommand[1]={0};
			GET_HAL_INTERFACE(priv)->FillH2CCmdHandler(priv, H2C_88XX_BCN_IGNORE_EDCCA, 1, H2CCommand);
		}


	}
}

int rtl8192cd_init_hw_PCI(struct rtl8192cd_priv *priv)
{
	struct wifi_mib *pmib;
	unsigned int opmode;
	unsigned long val32;
	unsigned short val16;
	int i;
	unsigned int errorFlag = 0;
	unsigned long x;

	SAVE_INT_AND_CLI(x);

	pmib = GET_MIB(priv);
	opmode = priv->pmib->dot11OperationEntry.opmode;

	DBFENTER;

//1 For Test, Firmware Downloading

//	MacConfigBeforeFwDownload(priv);

#if 0 	//	==========>> later
	// ===========================================================================================
	// Download Firmware
	// allocate memory for tx cmd packet
	if ((priv->pshare->txcmd_buf = (unsigned char *)kmalloc((LoadPktSize), GFP_ATOMIC)) == NULL) {
		printk("not enough memory for txcmd_buf\n");
		return -1;
	}

	priv->pshare->cmdbuf_phyaddr = get_physical_addr(priv, priv->pshare->txcmd_buf,
								   LoadPktSize, PCI_DMA_TODEVICE);

	if (LoadFirmware(priv) == FALSE) {
//		panic_printk("Load Firmware Fail!\n");
		panic_printk("Load Firmware check!\n");
		return -1;
	} else {
//		delay_ms(20);
		PRINT_INFO("Load firmware successful! \n");
	}
#endif
//	MacConfigAfterFwDownload(priv);

#if 0 // defined(CONFIG_RTL_92D_SUPPORT) && defined(MP_TEST)  // 92D MP DUAL-PHY SETTING
	if (priv->pshare->rf_ft_var.mp_specific && (GET_CHIP_VER(priv) == VERSION_8192D))
		priv->pmib->dot11RFEntry.macPhyMode = DUALMAC_DUALPHY;
	//priv->pmib->dot11RFEntry.macPhyMode = SINGLEMAC_SINGLEPHY;
#endif

#if 0 //def CONFIG_RTL_92D_SUPPORT
	if (check_MacPhyMode(priv) < 0) {
		printk("Check macPhyMode Fail!\n");
		return -1;
	}
#endif


	if (IS_OUTSRC_CHIP(priv))
		ODM_software_init(priv);
	if(GET_CHIP_VER(priv) == VERSION_8822B){
		if(GET_CHIP_VER_8822(priv) < PHYDM_SUPPORT_8822_CUT_VERSION){
			panic_printk("8822 CHIP VER = (%d %d) use TXT \n", GET_CHIP_VER_8822(priv), PHYDM_SUPPORT_8822_CUT_VERSION);
			ODMPTR->ConfigBBRF = 0;
		}
	}


	if (IS_HAL_CHIP(priv)) {

		unsigned int ClkSel = XTAL_CLK_SEL_40M;

#if defined(CONFIG_AUTO_PCIE_PHY_SCAN)
		// Get XTAL information from platform
        if(GET_CHIP_VER(priv) == VERSION_8881A || GET_CHIP_VER(priv) == VERSION_8197F) {
    		if ((REG32(0xb8000008) & 0x1000000) != 0x1000000)
    			 ClkSel = XTAL_CLK_SEL_25M;
    		else
    			 ClkSel = XTAL_CLK_SEL_40M;
        }
#else //CONFIG_AUTO_PCIE_PHY_SCAN

    	if(GET_CHIP_VER(priv) == VERSION_8197F) {
            #ifdef CONFIG_PHY_EAT_40MHZ
    		ClkSel = XTAL_CLK_SEL_40M;
            #else
    		ClkSel = XTAL_CLK_SEL_25M;
            #endif
        }
        else
           // independent with platform
        #ifdef CONFIG_PHY_WLAN_EAT_40MHZ
  		ClkSel = XTAL_CLK_SEL_40M;
        #else
    	ClkSel = XTAL_CLK_SEL_25M;
        #endif //CONFIG_PHY_WLAN_EAT_40MHZ
#endif //defined(CONFIG_AUTO_PCIE_PHY_SCAN)

		if(ClkSel == XTAL_CLK_SEL_25M)
			panic_printk("clock 25MHz\n");
		else if(ClkSel == XTAL_CLK_SEL_40M)
			panic_printk("clock 40MHz\n");
		else
			panic_printk("Unknown Clock Frequency\n");
		if ( RT_STATUS_SUCCESS != GET_HAL_INTERFACE(priv)->InitPONHandler(priv, ClkSel) ) {
			GET_HAL_INTERFACE(priv)->GetHwRegHandler(priv, HW_VAR_DRV_DBG, (pu1Byte)&errorFlag);
			errorFlag |= DRV_ER_INIT_PON;
			GET_HAL_INTERFACE(priv)->SetHwRegHandler(priv, HW_VAR_DRV_DBG, (pu1Byte)&errorFlag);
			panic_printk("InitPON Failed =>\n");
		} else {
				u1Byte	efuse0x3F0;
				EFUSE_DATA efuse_virtual_data;
				load_efuse_data_to_reg();
				if(load_efuse_data(&efuse_virtual_data, 0))
					panic_printk("eFuse read failed!\n");
				else {

					if(efuse_virtual_data.special) {
						#if 1
						//for RC check
						if( ( REG32(0xb8b00728) & 0x1f ) != 0x11) {
                                        			extern  int PCIE_reset_procedure_97F(unsigned int PCIeIdx, unsigned int mdioReset);
                                        			if ((PCIE_reset_procedure_97F(0,1))  != 1) {

									panic_printk("$$$$\r");
                                                			return -1;
                                        			}
								else {
									if( (REG32(0xb8b10000) >>  16 ) != 0xb822) {
										panic_printk("$$XX\r");
										return -1;
									}
								}
						}
						else {

							if( (REG32(0xb8b10000) >>  16 ) != 0xb822) {
								panic_printk("$$XX\r");
								return -1;
							}


						}
						#endif
					}
				}

			DEBUG_INFO("InitPON OK\n");
		}

		if ( RT_STATUS_SUCCESS != GET_HAL_INTERFACE(priv)->InitMACHandler(priv) ) {
			GET_HAL_INTERFACE(priv)->GetHwRegHandler(priv, HW_VAR_DRV_DBG, (pu1Byte)&errorFlag);
			errorFlag |= DRV_ER_INIT_MAC;
			GET_HAL_INTERFACE(priv)->SetHwRegHandler(priv, HW_VAR_DRV_DBG, (pu1Byte)&errorFlag);
			panic_printk("InitMAC Failed\n");
			RESTORE_INT(x);
			return -1;
		} else {
			DEBUG_INFO("InitMAC OK\n");
		}

		//TXPAUSE BCN for 8822B, avoid TX Spur after download FW
		if(priv->pshare->rf_ft_var.lock5d1
			&& (GET_CHIP_VER(priv) == VERSION_8822B)
			&& (OPMODE & WIFI_AP_STATE)){
			RTL_W8(0x5d1, RTL_R8(0x5d1) | STOP_BCN);
		}

		if ( RT_STATUS_SUCCESS != GET_HAL_INTERFACE(priv)->InitHCIDMARegHandler(priv) ) {
			GET_HAL_INTERFACE(priv)->GetHwRegHandler(priv, HW_VAR_DRV_DBG, (pu1Byte)&errorFlag);
			errorFlag |= DRV_ER_INIT_HCIDMA;
			GET_HAL_INTERFACE(priv)->SetHwRegHandler(priv, HW_VAR_DRV_DBG, (pu1Byte)&errorFlag);
			panic_printk("InitHCIDMAReg Failed\n");
		} else {
			DEBUG_INFO("InitHCIDMAReg OK\n");
		}

	} else if(CONFIG_WLAN_NOT_HAL_EXIST)
	{//not HAL
		if (MacInit(priv) == FALSE) {
		}
	}

//	RTL_W32(AGGLEN_LMT, RTL_R32(AGGLEN_LMT)|(0x0F&AGGLMT_MCS15S_Mask)<<AGGLMT_MCS15S_SHIFT
//		|(0x0F&AGGLMT_MCS15_Mask)<<AGGLMT_MCS15_SHIFT);

//	RTL_W8(AGGR_BK_TIME, 0x10);


	//
	// 2. Initialize MAC/PHY Config by MACPHY_reg.txt
	//
/*
#if defined(CONFIG_MACBBRF_BY_ODM) && defined(CONFIG_RTL_88E_SUPPORT)
	if (GET_CHIP_VER(priv) == VERSION_8188E)	{
		ODM_ConfigMACWithHeaderFile(ODMPTR);
	} else
#endif
*/
	set_rf_path_num(priv);
	if ( IS_HAL_CHIP(priv)) {
		GET_HAL_INTERFACE(priv)->GetHwRegHandler(priv, HW_VAR_NUM_TOTAL_RF_PATH, (pu1Byte)&priv->pshare->phw->NumTotalRFPath);
	}


	if(GET_CHIP_VER(priv) == VERSION_8822B) {
		//prepare to init 8822B PHY setting
		priv->pshare->phw->NumTotalRFPath = 2;
		config_phydm_parameter_init(ODMPTR, ODM_PRE_SETTING);

		//disable loop back mode
		if (!priv->pmib->miscEntry.drvmac_lb)
		RTL_W8(0x103, 0);
	}


	if(GET_CHIP_VER(priv) == VERSION_8197F) {
		config_phydm_parameter_8197f_init(ODMPTR, ODM_PRE_SETTING);
	}

	if (IS_OUTSRC_CHIP(priv) && ODMPTR->ConfigBBRF) //eric-?? will 8822 go here ??
	{
	   ODM_ConfigMACWithHeaderFile(ODMPTR);
	}
	else
	{
		panic_printk("\n\n************* Initialize MAC/PHY parameter *************\n");
		if (PHY_ConfigMACWithParaFile(priv) < 0) {
			if (IS_HAL_CHIP(priv)) {
				GET_HAL_INTERFACE(priv)->GetHwRegHandler(priv, HW_VAR_DRV_DBG, (pu1Byte)&errorFlag);
				errorFlag |= DRV_ER_INIT_MACPHYREGFILE;
				GET_HAL_INTERFACE(priv)->SetHwRegHandler(priv, HW_VAR_DRV_DBG, (pu1Byte)&errorFlag);
			}
			printk("Initialize MAC/PHY Config failure\n");
			RESTORE_INT(x);
			return -1;
		}

	}
	if(IS_C_CUT_8192E(priv))
	{
		if(priv->pmib->dot11nConfigEntry.dot11nUse40M == HT_CHANNEL_WIDTH_5
			|| priv->pmib->dot11nConfigEntry.dot11nUse40M == HT_CHANNEL_WIDTH_10)
					RTL_W8(0x303,0x80);
	}

	//
	// 3. Initialize BB After MAC Config PHY_reg.txt, AGC_Tab.txt
	//

#ifdef DRVMAC_LB
	if (!priv->pmib->miscEntry.drvmac_lb)
#endif
	{
		if (IS_HAL_CHIP(priv)) {
			RESTORE_INT(x);
			val32 = phy_BB88XX_Config_ParaFile(priv);
			SAVE_INT_AND_CLI(x);
			if (val32) {
				GET_HAL_INTERFACE(priv)->GetHwRegHandler(priv, HW_VAR_DRV_DBG, (pu1Byte)&errorFlag);
				errorFlag |= DRV_ER_INIT_BBEGFILE;
				GET_HAL_INTERFACE(priv)->SetHwRegHandler(priv, HW_VAR_DRV_DBG, (pu1Byte)&errorFlag);
				printk("Initialize phy_BB88XX_Config_ParaFile failure\n");
			}
		} else if(CONFIG_WLAN_NOT_HAL_EXIST)
		val32 = phy_BB8192CD_Config_ParaFile(priv);
		if (val32) {
			RESTORE_INT(x);
			return -1;
		}
	}

	if(GET_CHIP_VER(priv) == VERSION_8192E){//for 92e only
		if (priv->pmib->dot11nConfigEntry.dot11nLDPC &1)	// Rx LDPC
			RTL_W8(0x913, RTL_R8(0x913)|0x02);
		else
			RTL_W8(0x913, RTL_R8(0x913)&~0x02);
	}





	if ((GET_CHIP_VER(priv) == VERSION_8197F)||(GET_CHIP_VER(priv) == VERSION_8822B)) {
			int org_val, tmp_val;
			org_val = (RTL_R32(0x24) >> 25) & 0x3F;
		if (GET_CHIP_VER(priv) == VERSION_8197F && priv->pmib->dot11RFEntry.share_xcap){

				for (tmp_val=org_val; tmp_val<=0x3F; tmp_val++) {
					PHY_SetBBReg(priv, 0x24, BIT(30) | BIT(29) | BIT(28) | BIT(27) | BIT(26) | BIT(25), tmp_val);
					PHY_SetBBReg(priv, 0x28, BIT(6) | BIT(5) | BIT(4) | BIT(3) | BIT(2) | BIT(1), tmp_val);
				}

		}else{
			if (priv->pmib->dot11RFEntry.xcap > 0 && priv->pmib->dot11RFEntry.xcap < 0x3F) {
				if (GET_CHIP_VER(priv) == VERSION_8822B)
					PHY_SetBBReg(priv, 0x10, 0x00000040, 0x01); // control by WiFi

				if (org_val > priv->pmib->dot11RFEntry.xcap) {
					for (tmp_val=org_val; tmp_val>=priv->pmib->dot11RFEntry.xcap; tmp_val--) {
						PHY_SetBBReg(priv, 0x24, BIT(30) | BIT(29) | BIT(28) | BIT(27) | BIT(26) | BIT(25), tmp_val & 0x3F);
						PHY_SetBBReg(priv, 0x28, BIT(6) | BIT(5) | BIT(4) | BIT(3) | BIT(2) | BIT(1), tmp_val & 0x3F);
					}
				}else {
					for (tmp_val=org_val; tmp_val<=priv->pmib->dot11RFEntry.xcap; tmp_val++) {
						PHY_SetBBReg(priv, 0x24, BIT(30) | BIT(29) | BIT(28) | BIT(27) | BIT(26) | BIT(25), tmp_val & 0x3F);
						PHY_SetBBReg(priv, 0x28, BIT(6) | BIT(5) | BIT(4) | BIT(3) | BIT(2) | BIT(1), tmp_val & 0x3F);
					}
				}
			}
		}
	}



	// support up to MCS7 for 1T1R, modify rx capability here
	/*
	if (get_rf_mimo_mode(priv) == MIMO_1T1R)
		pmib->dot11nConfigEntry.dot11nSupportedMCS &= 0x00ff;

	*/
	/*
		// Set NAV protection length
		// CF-END Threshold
		if (priv->pmib->dot11OperationEntry.wifi_specific) {
			RTL_W16(NAV_PROT_LEN, 0x80);
	//		RTL_W8(CFEND_TH, 0x2);
		}
		else {
			RTL_W16(NAV_PROT_LEN, 0x01C0);
	//		RTL_W8(CFEND_TH, 0xFF);
		}
	*/
	//
	// 4. Initiailze RF RAIO_A.txt RF RAIO_B.txt
	//
	// 2007/11/02 MH Before initalizing RF. We can not use FW to do RF-R/W.
	// close loopback, normal mode

	// For RF test only from Scott's suggestion
//	RTL_W8(0x27, 0xDB);	//	==========>> ???
//	RTL_W8(0x1B, 0x07); // ACUT

	/*
		// set RCR: RX_SHIFT and disable ACF
	//	RTL_W8(0x48, 0x3e);
	//	RTL_W32(0x48, RTL_R32(0x48) & ~ RCR_ACF  & ~RCR_ACRC32);
		RTL_W16(RCR, RCR_AAP | RCR_APM | RCR_ACRC32);
		RTL_W32(RCR, RTL_R32(RCR) & ~ RCR_ACF  & ~RCR_ACRC32);
		// for debug by victoryman, 20081119
	//	RTL_W32(RCR, RTL_R32(RCR) | RCR_APP_PHYST_RXFF);
		RTL_W32(RCR, RTL_R32(RCR) | RCR_APP_PHYSTS);
	*/

	RESTORE_INT(x);

#ifdef DRVMAC_LB
	if (priv->pmib->miscEntry.drvmac_lb) {
		if (IS_HAL_CHIP(priv)) {
			RT_OP_MODE  OP_Mode = RT_OP_MODE_NO_LINK;
			GET_HAL_INTERFACE(priv)->SetHwRegHandler(priv, HW_VAR_MEDIA_STATUS, (pu1Byte)&OP_Mode);
		} else if(CONFIG_WLAN_NOT_HAL_EXIST)
		{//not HAL
			RTL_W32(CR, (RTL_R32(CR) & ~(NETYPE_Mask << NETYPE_SHIFT)) | ((NETYPE_NOLINK & NETYPE_Mask) << NETYPE_SHIFT));
		}
		drvmac_loopback(priv);
	} else
#endif
    {

        if (IS_HAL_CHIP(priv)) {
            if(GET_CHIP_VER(priv) == VERSION_8822B){
                //Power ON 8822 RF
                RTL_W8(0x1f, 0x0);//for B cut
                RTL_W8(0x1f, 0x7);
                RTL_W8(0xef, 0x0);//for B cut
                RTL_W8(0xef, 0x7);
            }

			if(GET_CHIP_VER(priv) == VERSION_8197F){
				RTL_W8(0x7B, 0x0);
				RTL_W8(0x7B, 0x7);
			}

			if(ODMPTR->ConfigBBRF)
            {

                val32 = ODM_ConfigRFWithHeaderFile(ODMPTR, CONFIG_RF_RADIO, ODM_RF_PATH_A);
                val32 |= ODM_ConfigRFWithHeaderFile(ODMPTR, CONFIG_RF_RADIO, ODM_RF_PATH_B);
                if(GET_CHIP_VER(priv) == VERSION_8814A){
                    val32 |= ODM_ConfigRFWithHeaderFile(ODMPTR, CONFIG_RF_RADIO, ODM_RF_PATH_C);
                    val32 |= ODM_ConfigRFWithHeaderFile(ODMPTR, CONFIG_RF_RADIO, ODM_RF_PATH_D);
                }

            }
            else
                val32 = phy_RF6052_Config_ParaFile(priv);

            if (val32) {
                GET_HAL_INTERFACE(priv)->GetHwRegHandler(priv, HW_VAR_DRV_DBG, (pu1Byte)&errorFlag);
                errorFlag |= DRV_ER_INIT_PHYRF;
                GET_HAL_INTERFACE(priv)->SetHwRegHandler(priv, HW_VAR_DRV_DBG, (pu1Byte)&errorFlag);
            }
            if (val32)
                return -1;

            priv->pshare->No_RF_Write = 1;

            //Finish 8822B PHY Setting
            if(GET_CHIP_VER(priv) == VERSION_8822B)
                config_phydm_parameter_init(ODMPTR, ODM_POST_SETTING);

            if(GET_CHIP_VER(priv) == VERSION_8197F) {
                config_phydm_parameter_8197f_init(ODMPTR, ODM_POST_SETTING);
				if (PHY_QueryRFReg(priv, RF92CD_PATH_A, 0xb4, bMask20Bits, 1)==0x140a0){
					delay_ms(1);
					RTL_W8(0x7A,0);
				}
				RTL_W8(0x7d4,0x90);
            }



        }
        else if(CONFIG_WLAN_NOT_HAL_EXIST)
        {//not HAL

            if(ODMPTR->ConfigBBRF)
            {
                val32 = ODM_ConfigRFWithHeaderFile(ODMPTR, CONFIG_RF_RADIO, ODM_RF_PATH_A);
                val32 |= ODM_ConfigRFWithHeaderFile(ODMPTR, CONFIG_RF_RADIO, ODM_RF_PATH_B);
                val32 |= ODM_ConfigRFWithHeaderFile(ODMPTR, CONFIG_RF_RADIO, ODM_RF_PATH_C);
                val32 |= ODM_ConfigRFWithHeaderFile(ODMPTR, CONFIG_RF_RADIO, ODM_RF_PATH_D);
            }
            else
                val32 = phy_RF6052_Config_ParaFile(priv);

            if (GET_CHIP_VER(priv) == VERSION_8188E)
                priv->pshare->phw->NumTotalRFPath = 1;
            else
                priv->pshare->phw->NumTotalRFPath = 2; //8812A


            if (val32)
                return -1;

            SAVE_INT_AND_CLI(x);

            RESTORE_INT(x);
        }
    }


	SAVE_INT_AND_CLI(x);


	if (IS_HAL_CHIP(priv)) {
		//Do nothing
		//MacConfig() is integrated in code below
		//Don't enable BB here
	} else if(CONFIG_WLAN_NOT_HAL_EXIST)
	{//not HAL
		{
			/*---- Set CCK and OFDM Block "ON"----*/
			PHY_SetBBReg(priv, rFPGA0_RFMOD, bOFDMEn, 1);
				PHY_SetBBReg(priv, rFPGA0_RFMOD, bCCKEn, 1);

			MacConfig(priv);
		}

		/*
		 *	Force CCK CCA for High Power products
		 */
#ifdef HIGH_POWER_EXT_LNA
		if (priv->pshare->rf_ft_var.use_ext_lna)
			RTL_W8(0xa0a, 0xcd);
#endif
	}

//	RTL_W8(BW_OPMODE, BIT(2)); // 40Mhz:0 20Mhz:1
//	RTL_W32(MACIDR,0x0);

	// under loopback mode
//	RTL_W32(MACIDR,0xffffffff);		//	need to confirm
	/*
	#ifdef CONFIG_NET_PCI
		if (IS_PCIBIOS_TYPE)
			pci_unmap_single(priv->pshare->pdev, priv->pshare->cmdbuf_phyaddr,
				(LoadPktSize), PCI_DMA_TODEVICE);
	#endif
	*/
#if	0
//	RTL_W32(0x230, 0x40000000);	//for test
////////////////////////////////////////////////////////////

	printk("init_hw: 1\n");

	RTL_W16(SIFS_OFDM, 0x1010);
	RTL_W8(SLOT_TIME, 0x09);

	RTL_W8(MSR, MSR_AP);

//	RTL_W8(MSR,MSR_INFRA);
	// for test, loopback
//	RTL_W8(MSR, MSR_NOLINK);
//	RTL_W8(LBKMD_SEL, BIT(0)| BIT(1) );
//	RTL_W16(LBDLY, 0xffff);

	//beacon related
	RTL_W16(BCN_INTERVAL, pmib->dot11StationConfigEntry.dot11BeaconPeriod);
	RTL_W16(ATIMWND, 2); //0
	RTL_W16(BCN_DRV_EARLY_INT, 10 << 4); // 2
	RTL_W16(BCN_DMATIME, 256); // 0xf
	RTL_W16(SIFS_OFDM, 0x0e0e);
	RTL_W8(SLOT_TIME, 0x10);

//	CamResetAllEntry(priv);
	RTL_W16(SECR, 0x0000);

// By H.Y. advice
//	RTL_W16(_BCNTCFG_, 0x060a);
//	if (OPMODE & WIFI_AP_STATE)
//		RTL_W16(BCNTCFG, 0x000a);
//	else
// for debug
//	RTL_W16(_BCNTCFG_, 0x060a);
//	RTL_W16(BCNTCFG, 0x0204);

	init_beacon(priv);

	priv->pshare->InterruptMask = (IMR_ROK | IMR_VODOK | IMR_VIDOK | IMR_BEDOK | IMR_BKDOK |		\
								   IMR_HCCADOK | IMR_MGNTDOK | IMR_COMDOK | IMR_HIGHDOK | 					\
								   IMR_BDOK | /*IMR_RXCMDOK | IMR_TIMEOUT0 |*/ IMR_RDU | IMR_RXFOVW	|			\
								   IMR_BcnInt/* | IMR_TXFOVW*/ /*| IMR_TBDOK | IMR_TBDER*/) ;// IMR_ROK | IMR_BcnInt | IMR_RDU | IMR_RXFOVW | IMR_RXCMDOK;

//	priv->pshare->InterruptMask = IMR_ROK| IMR_BDOK | IMR_BcnInt | IMR_MGNTDOK | IMR_TBDOK | IMR_RDU ;
//	priv->pshare->InterruptMask  = 0;
	priv->pshare->InterruptMaskExt = 0;
	RTL_W32(IMR, priv->pshare->InterruptMask);
	RTL_W32(IMR + 4, priv->pshare->InterruptMaskExt);

//////////////////////////////////////////////////////////////
	printk("end of init hw\n");

	return 0;

#endif
// clear TPPoll
//	RTL_W16(TPPoll, 0x0);
// Should 8192SE do this initialize? I don't know yet, 080812. Joshua
	// PJ 1-5-2007 Reset PHY parameter counters
//	RTL_W32(0xD00, RTL_R32(0xD00)|BIT(27));
//	RTL_W32(0xD00, RTL_R32(0xD00)&(~(BIT(27))));
	/*
		// configure timing parameter
		RTL_W8(ACK_TIMEOUT, 0x30);
		RTL_W8(PIFS_TIME,0x13);
	//	RTL_W16(LBDLY, 0x060F);
	//	RTL_W16(SIFS_OFDM, 0x0e0e);
	//	RTL_W8(SLOT_TIME, 0x10);
		if (priv->pmib->dot11BssType.net_work_type & WIRELESS_11N) {
			RTL_W16(SIFS_OFDM, 0x0a0a);
			RTL_W8(SLOT_TIME, 0x09);
		}
		else if (priv->pmib->dot11BssType.net_work_type & WIRELESS_11A) {
			RTL_W16(SIFS_OFDM, 0x0a0a);
			RTL_W8(SLOT_TIME, 0x09);
		}
		else if (priv->pmib->dot11BssType.net_work_type & WIRELESS_11G) {
			RTL_W16(SIFS_OFDM, 0x0a0a);
			RTL_W8(SLOT_TIME, 0x09);
		}
		else { // WIRELESS_11B
			RTL_W16(SIFS_OFDM, 0x0a0a);
			RTL_W8(SLOT_TIME, 0x14);
		}
	*/

	if (IS_HAL_CHIP(priv)) {
		RT_OP_MODE      OP_Mode;
		MACCONFIG_PARA  MacCfgPara;
		u2Byte          beaconPeriod;

		if (opmode & WIFI_AP_STATE) {
			DEBUG_INFO("AP-mode enabled...\n");
			if (priv->pmib->dot11WdsInfo.wdsPure)
			{
				OP_Mode = RT_OP_MODE_NO_LINK;
			} else {
				OP_Mode = RT_OP_MODE_AP;
			}
		} else if (opmode & WIFI_STATION_STATE) {
			DEBUG_INFO("Station-mode enabled...\n");
			OP_Mode = RT_OP_MODE_INFRASTRUCTURE;
		} else if (opmode & WIFI_ADHOC_STATE) {
			DEBUG_INFO("Adhoc-mode enabled...\n");
			OP_Mode = RT_OP_MODE_IBSS;
		} else {
			printk("Operation mode error!\n");
			RESTORE_INT(x);
			return 2;
		}

		GET_HAL_INTERFACE(priv)->SetHwRegHandler(priv, HW_VAR_MEDIA_STATUS, (pu1Byte)&OP_Mode);

		MacCfgPara.AckTO                = priv->pmib->miscEntry.ack_timeout;
		MacCfgPara.vap_enable           = GET_ROOT(priv)->pmib->miscEntry.vap_enable;
		MacCfgPara.OP_Mode              = OP_Mode;
		MacCfgPara.dot11DTIMPeriod      = priv->pmib->dot11StationConfigEntry.dot11DTIMPeriod;
		MacCfgPara.WdsPure      		= priv->pmib->dot11WdsInfo.wdsPure;
		beaconPeriod                    = pmib->dot11StationConfigEntry.dot11BeaconPeriod;

		GET_HAL_INTERFACE(priv)->SetHwRegHandler(priv, HW_VAR_BEACON_INTERVAL, (pu1Byte)&beaconPeriod);
		GET_HAL_INTERFACE(priv)->SetHwRegHandler(priv, HW_VAR_MAC_CONFIG, (pu1Byte)&MacCfgPara);
	}

#if !defined(USE_OUT_SRC) || defined(_OUTSRC_COEXIST)
#ifdef _OUTSRC_COEXIST
	if (!IS_OUTSRC_CHIP(priv))
#endif
		init_EDCA_para(priv, pmib->dot11BssType.net_work_type);
#endif

	// we don't have EEPROM yet, Mark this for FPGA Platform
//	RTL_W8(_9346CR_, CR9346_CFGRW);

//	92SE Windows driver does not set the PCIF as 0x77, seems HW bug?
	// Set Tx and Rx DMA burst size
//	RTL_W8(PCIF, 0x77);
	// Enable byte shift
//	RTL_W8(_PCIF_+2, 0x01);

	/*
		// Retry Limit
		if (priv->pmib->dot11OperationEntry.dot11LongRetryLimit)
			val16 = priv->pmib->dot11OperationEntry.dot11LongRetryLimit & 0xff;
		else {
			if (priv->pmib->dot11BssType.net_work_type & WIRELESS_11N)
				val16 = 0x30;
			else
				val16 = 0x06;
		}
		if (priv->pmib->dot11OperationEntry.dot11ShortRetryLimit)
			val16 |= ((priv->pmib->dot11OperationEntry.dot11ShortRetryLimit & 0xff) << 8);
		else {
			if (priv->pmib->dot11BssType.net_work_type & WIRELESS_11N)
				val16 |= (0x30 << 8);
			else
				val16 |= (0x06 << 8);
		}
		RTL_W16(RETRY_LIMIT, val16);

		This should be done later, but Windows Driver not done yet.
		// Response Rate Set
		// let ACK sent by highest of 24Mbps
		val32 = 0x1ff;
		if (pmib->dot11RFEntry.shortpreamble)
			val32 |= BIT(23);
		RTL_W32(_RRSR_, val32);
	*/




//	panic_printk("0x2c4 = bitmap = 0x%08x\n", (unsigned int)val32);
//	panic_printk("0x2c0 = cmd | macid | band = 0x%08x\n", 0xfd0000a2 | (1<<9 | (sta_band & 0xf))<<8);
//	panic_printk("Add id %d val %08x to ratr\n", 0, (unsigned int)val32);

	/*	for (i = 0; i < 8; i++)
			RTL_W32(ARFR0+i*4, val32 & 0x1f0ff0f0);
	*/

	//settting initial rate for control frame to 24M
//	RTL_W8(INIRTSMCS_SEL, 8);	//	==========>> later

	if (IS_HAL_CHIP(priv)) {
		u8  MulticastAddr[8] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
		GET_HAL_INTERFACE(priv)->SetHwRegHandler(priv, HW_VAR_MULTICAST_REG, (pu1Byte)MulticastAddr);

		GET_HAL_INTERFACE(priv)->SetHwRegHandler(priv, HW_VAR_BSSID, (pu1Byte)pmib->dot11OperationEntry.hwaddr);
	} else if(CONFIG_WLAN_NOT_HAL_EXIST)
	{//not HAL
		//setting MAR
		RTL_W32(MAR, 0xffffffff);
		RTL_W32((MAR + 4), 0xffffffff);

		//setting BSSID, not matter AH/AP/station
		memcpy((void *)&val32, (pmib->dot11OperationEntry.hwaddr), 4);
		memcpy((void *)&val16, (pmib->dot11OperationEntry.hwaddr + 4), 2);
		RTL_W32(BSSIDR, cpu_to_le32(val32));
		RTL_W16((BSSIDR + 4), cpu_to_le16(val16));
		//	RTL_W32(BSSIDR, 0x814ce000);
		//	RTL_W16((BSSIDR + 4), 0xee92);
	}

	//setting TCR
	//TCR, use default value

	//setting RCR // set in MacConfigAfterFwDownload
//	RTL_W32(_RCR_, _APWRMGT_ | _AMF_ | _ADF_ | _AICV_ | _ACRC32_ | _AB_ | _AM_ | _APM_);
//	if (pmib->dot11OperationEntry.crc_log)
//		RTL_W32(_RCR_, RTL_R32(_RCR_) | _ACRC32_);

	if (IS_HAL_CHIP(priv)) {
		//3 Integrated into SetHwRegHandler(priv, HW_VAR_MAC_CONFIG, &MacCfgPara);
	} else if(CONFIG_WLAN_NOT_HAL_EXIST)
	{//not HAL
		// setting network type
		if (opmode & WIFI_AP_STATE) {
			DEBUG_INFO("AP-mode enabled...\n");

			if (priv->pmib->dot11WdsInfo.wdsPure)
			{
				{
					RTL_W32(CR, (RTL_R32(CR) & ~(NETYPE_Mask << NETYPE_SHIFT)) | ((NETYPE_NOLINK & NETYPE_Mask) << NETYPE_SHIFT));
				}

			} else {
				RTL_W32(CR, (RTL_R32(CR) & ~(NETYPE_Mask << NETYPE_SHIFT)) | ((NETYPE_AP & NETYPE_Mask) << NETYPE_SHIFT));
			}
			// Move init beacon after f/w download
#if 0
			if (priv->auto_channel == 0) {
				DEBUG_INFO("going to init beacon\n");
				init_beacon(priv);
			}
#endif
		}
#ifdef CLIENT_MODE
		else if (opmode & WIFI_STATION_STATE) {
			DEBUG_INFO("Station-mode enabled...\n");
			RTL_W32(CR, (RTL_R32(CR) & ~(NETYPE_Mask << NETYPE_SHIFT)) | ((NETYPE_INFRA & NETYPE_Mask) << NETYPE_SHIFT));
		} else if (opmode & WIFI_ADHOC_STATE) {
			DEBUG_INFO("Adhoc-mode enabled...\n");
			RTL_W32(CR, (RTL_R32(CR) & ~(NETYPE_Mask << NETYPE_SHIFT)) | ((NETYPE_ADHOC & NETYPE_Mask) << NETYPE_SHIFT));
		}
#endif
		else {
			printk("Operation mode error!\n");
			RESTORE_INT(x);
			return 2;
		}
	}


	//3 Security Related

	CamResetAllEntry(priv);
	if (IS_HAL_CHIP(priv)) {
		SECURITY_CONFIG_OPERATION  SCO = 0;
		GET_HAL_INTERFACE(priv)->SetHwRegHandler(priv, HW_VAR_SECURITY_CONFIG, (pu1Byte)&SCO);
	} else if(CONFIG_WLAN_NOT_HAL_EXIST)
	{//not HAL
		RTL_W16(SECCFG, 0);
	}
	if ((OPMODE & (WIFI_AP_STATE | WIFI_STATION_STATE | WIFI_ADHOC_STATE)) &&
			!priv->pmib->dot118021xAuthEntry.dot118021xAlgrthm &&
			(pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm == _WEP_40_PRIVACY_ ||
			 pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm == _WEP_104_PRIVACY_)) {
		pmib->dot11GroupKeysTable.dot11Privacy = pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm;
		if (pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm == _WEP_40_PRIVACY_)
			i = 5;
		else
			i = 13;
#ifdef USE_WEP_DEFAULT_KEY
		memcpy(pmib->dot11GroupKeysTable.dot11EncryptKey.dot11TTKey.skey,
			   &priv->pmib->dot11DefaultKeysTable.keytype[pmib->dot1180211AuthEntry.dot11PrivacyKeyIndex].skey[0], i);
		pmib->dot11GroupKeysTable.dot11EncryptKey.dot11TTKeyLen = i;
		pmib->dot11GroupKeysTable.keyid = pmib->dot1180211AuthEntry.dot11PrivacyKeyIndex;
		pmib->dot11GroupKeysTable.keyInCam = 0;
		if (IS_HAL_CHIP(priv)) {
			SECURITY_CONFIG_OPERATION  SCO;
			GET_HAL_INTERFACE(priv)->GetHwRegHandler(priv, HW_VAR_SECURITY_CONFIG, (pu1Byte)&SCO);
			SCO |= SCO_NOSKMC;
			GET_HAL_INTERFACE(priv)->SetHwRegHandler(priv, HW_VAR_SECURITY_CONFIG, (pu1Byte)&SCO);
		} else if(CONFIG_WLAN_NOT_HAL_EXIST)
		{//not HAL
			RTL_W16(SECCFG, RTL_R16(SECCFG) | NOSKMC);	// no search multicast
		}
#else
#if defined(CONFIG_RTL8196B_KLD) || defined(CONFIG_RTL8196C_KLD)
		memcpy(pmib->dot11GroupKeysTable.dot11EncryptKey.dot11TTKey.skey,
			   &priv->pmib->dot11DefaultKeysTable.keytype[pmib->dot1180211AuthEntry.dot11PrivacyKeyIndex].skey[0], i);
#else
		memcpy(pmib->dot11GroupKeysTable.dot11EncryptKey.dot11TTKey.skey,
			   &priv->pmib->dot11DefaultKeysTable.keytype[0].skey[0], i);
#endif
		pmib->dot11GroupKeysTable.dot11EncryptKey.dot11TTKeyLen = i;
		pmib->dot11GroupKeysTable.keyid = pmib->dot1180211AuthEntry.dot11PrivacyKeyIndex;
		pmib->dot11GroupKeysTable.keyInCam = 0;
#endif
	}

// for debug
#if 0
// when hangup reset, re-init TKIP/AES key in ad-hoc mode
#ifdef CLIENT_MODE
	if ((OPMODE & WIFI_ADHOC_STATE) && pmib->dot11OperationEntry.keep_rsnie &&
			(pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm == _TKIP_PRIVACY_ ||
			 pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm == _CCMP_PRIVACY_)) {
		DOT11_SET_KEY Set_Key;
		Set_Key.KeyType = DOT11_KeyType_Group;
		Set_Key.EncType = pmib->dot11GroupKeysTable.dot11Privacy;
		DOT11_Process_Set_Key(priv->dev, NULL, &Set_Key, pmib->dot11GroupKeysTable.dot11EncryptKey.dot11TTKey.skey);
	} else
#endif
//-------------------------------------- david+2006-06-30
		// restore group key if it has been set before open, david
		if (pmib->dot11GroupKeysTable.keyInCam) {
			int retVal;
			retVal = CamAddOneEntry(priv, (unsigned char *)"\xff\xff\xff\xff\xff\xff",
									pmib->dot11GroupKeysTable.keyid,
									pmib->dot11GroupKeysTable.dot11Privacy << 2,
									0, pmib->dot11GroupKeysTable.dot11EncryptKey.dot11TTKey.skey);
			if (retVal)
				priv->pshare->CamEntryOccupied++;
			else {
				DEBUG_ERR("Add group key failed!\n");
			}
		}
#endif
	//here add if legacy WEP
	// if 1x is enabled, do not set default key, david
#ifdef USE_WEP_DEFAULT_KEY
	if (!(OPMODE & WIFI_AP_STATE) || !priv->pmib->miscEntry.vap_enable)
	{
		if (!SWCRYPTO && !IEEE8021X_FUN &&
				(pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm == _WEP_104_PRIVACY_ ||
				 pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm == _WEP_40_PRIVACY_))
			init_DefaultKey_Enc(priv, NULL, 0);
	}
#endif


	//3 MAC Beacon Tming Related

	if (IS_HAL_CHIP(priv)) {
		//Do Nothing
		// Integrated into code above
		//3      Integrated into SetHwRegHandler(priv, HW_VAR_MAC_CONFIG, &MacCfgPara);
	} else if(CONFIG_WLAN_NOT_HAL_EXIST)
	{//not HAL

		//Setup Beacon Interval/interrupt interval, ATIM-WIND ATIM-Interrupt
		RTL_W32(MBSSID_BCN_SPACE, pmib->dot11StationConfigEntry.dot11BeaconPeriod);
		//Setting BCNITV is done by firmware now
//	set_fw_reg(priv, (0xF1000000 | (pmib->dot11StationConfigEntry.dot11BeaconPeriod << 8)), 0, 0);
		// Set max AMPDU aggregation time
//	int max_aggre_time = 0xc0; // in 4us
//	set_fw_reg(priv, (0xFD0000B1|((max_aggre_time << 8) & 0xff00)), 0 ,0);

//	RTL_W32(0x2A4, 0x00006300);
//	RTL_W32(0x2A0, 0xb026007C);
//	delay_ms(1);
//	while(RTL_R32(0x2A0) != 0){};
		//RTL_W16(TBTT_PROHIBIT, 0xc804);

        if((OPMODE & WIFI_AP_STATE ))
        {   // for pure AP mode + repeater mode + APWDS + Mbssid
    		if ( (priv->pmib->miscEntry.vap_enable) || (priv->pmib->dot11StationConfigEntry.dot11BeaconPeriod <= 40)){
    			RTL_W32(TBTT_PROHIBIT, 0x1df04);
    		}else{
    			RTL_W32(TBTT_PROHIBIT, 0x40004);
            }
            // for pure WDS
            if((priv->pmib->dot11WdsInfo.wdsEnabled && priv->pmib->dot11WdsInfo.wdsPure)) // pure wds mode
            {
               RTL_W32(TBTT_PROHIBIT, 0x104);
            }
       }else{
            // for STA mode only
            RTL_W32(TBTT_PROHIBIT, 0x4004); //tbtt=2ms
       }


		RTL_W8(DRVERLYINT, 10);
		RTL_W8(BCNDMATIM, 1);
		/*
			if (priv->pshare->rf_ft_var.bcast_to_dzq)
				RTL_W16(ATIMWND, 0x0a);
			else
				RTL_W16(ATIMWND, 5);
		*/

		{
			RTL_W16(ATIMWND, 1);
		}

		/*
			if (!((OPMODE & WIFI_AP_STATE)
		#if defined(WDS) && defined(CONFIG_RTK_MESH)
				&& ((priv->pmib->dot11WdsInfo.wdsEnabled && priv->pmib->dot11WdsInfo.wdsPure) || priv->pmib->dot1180211sInfo.meshSilence))
		#elif defined(WDS)
				&& priv->pmib->dot11WdsInfo.wdsEnabled && priv->pmib->dot11WdsInfo.wdsPure )
		#elif defined(CONFIG_RTK_MESH)	//Mesh Mode but mesh not enable
				&& priv->pmib->dot1180211sInfo.meshSilence )
		#else
				)
		#endif
			)

				RTL_W16(BCN_DRV_EARLY_INT, RTL_R16(BCN_DRV_EARLY_INT)|BIT(15)); // sw beacon
		*/
		if (priv->pmib->miscEntry.vap_enable && RTL8192CD_NUM_VWLAN == 1 &&
				priv->pmib->dot11StationConfigEntry.dot11BeaconPeriod < 30)
			//RTL_W16(BCN_DRV_EARLY_INT, (RTL_R16(BCN_DRV_EARLY_INT)&0xf00f) | ((6<<4)&0xff0));
			RTL_W8(DRVERLYINT, 6);


		{
			RTL_W8(BCN_CTRL, DIS_TSF_UPDATE_N |  DIS_SUB_STATE_N  );
			RTL_W8(BCN_MAX_ERR, 0xff);
			RTL_W16(0x518, 0);
			RTL_W8(0x553, 3);
			if (OPMODE & WIFI_STATION_STATE)
				RTL_W8(0x422, RTL_R8(0x422) ^ BIT(6));

			if ((priv->pmib->dot11WdsInfo.wdsPure == 0)
#ifdef MP_TEST
					&& (!priv->pshare->rf_ft_var.mp_specific)
#endif
			   ) {
				RTL_W8(BCN_CTRL, RTL_R8(BCN_CTRL) | EN_BCN_FUNCTION | EN_TXBCN_RPT);
			} else {

			}
		}


//--------------

// By H.Y. advice
//	RTL_W16(_BCNTCFG_, 0x060a);
		if (OPMODE & WIFI_AP_STATE)
			RTL_W16(BCNTCFG, 0x320c);	//RTL_W16(BCNTCFG, 0x000a);
		else
// for debug
//	RTL_W16(_BCNTCFG_, 0x060a);
			RTL_W16(BCNTCFG, 0x0204);
	}

	//3 IMR Related
	//3 Download Firmware Related
	// Ack ISR, and then unmask IMR
	if (IS_HAL_CHIP(priv)) {
		if (opmode & WIFI_AP_STATE) {
			GET_HAL_INTERFACE(priv)->InitIMRHandler(priv, RT_OP_MODE_AP);
		} else if (opmode & WIFI_STATION_STATE) {
			GET_HAL_INTERFACE(priv)->InitIMRHandler(priv, RT_OP_MODE_INFRASTRUCTURE);
		} else if (opmode & WIFI_ADHOC_STATE) {
			GET_HAL_INTERFACE(priv)->InitIMRHandler(priv, RT_OP_MODE_IBSS);
		}

	GET_HAL_INTERFACE(priv)->AddInterruptMaskHandler(priv, HAL_INT_TYPE_PSTIMEOUT2);

        #if defined(SW_TX_QUEUE) || defined(RTK_ATM)
        GET_HAL_INTERFACE(priv)->AddInterruptMaskHandler(priv, HAL_INT_TYPE_GTIMER4);
        #endif
		// TODO: Filen, no need to sync !?
		priv->pshare->InterruptMask = _GET_HAL_DATA(priv)->IntMask[0];
		priv->pshare->InterruptMask = _GET_HAL_DATA(priv)->IntMask[1];

		// TODO: Filen, check download 8051 firmware
		//Download Firmware

	if(priv->pshare->rf_ft_var.load_fw){
		RESTORE_INT(x);
    if(!IS_HARDWARE_MACHAL_SUPPORT(priv))
    {
		if ( RT_STATUS_SUCCESS != GET_HAL_INTERFACE(priv)->InitFirmwareHandler(priv)) {
			GET_HAL_INTERFACE(priv)->GetHwRegHandler(priv, HW_VAR_DRV_DBG, (pu1Byte)&errorFlag);
			errorFlag |= DRV_ER_INIT_DLFW;
			GET_HAL_INTERFACE(priv)->SetHwRegHandler(priv, HW_VAR_DRV_DBG, (pu1Byte)&errorFlag);
			panic_printk("InitDownload FW Fail\n");
		} else {
			DEBUG_INFO("InitDownload FW OK \n");
#ifdef AP_PS_Offlaod
            priv->APPS_phase = 0; //0:phase1, 1:phase2
            GET_HAL_INTERFACE(priv)->SetAPPSOffloadHandler(priv,1, priv->APPS_phase, 4, 1);
#endif
		}
    }

		SAVE_INT_AND_CLI(x);
		}else
		panic_printk("\n[%s] Not To Use FW\n",__FUNCTION__);
	} else if(CONFIG_WLAN_NOT_HAL_EXIST)
	{//not HAL
#if 1
		// Ack ISR, and then unmask IMR
#if 0
		RTL_W32(ISR, RTL_R32(ISR));
		RTL_W32(ISR + 4, RTL_R16(ISR + 4));
		RTL_W32(IMR, 0x0);
		RTL_W32(IMR + 4, 0x0);
		priv->pshare->InterruptMask = _ROK_ | _BCNDMAINT_ | _RDU_ | _RXFOVW_ | _RXCMDOK_;
		priv->pshare->InterruptMask = (IMR_ROK | IMR_VODOK | IMR_VIDOK | IMR_BEDOK | IMR_BKDOK |		\
									   IMR_HCCADOK | IMR_MGNTDOK | IMR_COMDOK | IMR_HIGHDOK | 					\
									   IMR_BDOK | IMR_RXCMDOK | /*IMR_TIMEOUT0 |*/ IMR_RDU | IMR_RXFOVW	|			\
									   IMR_BcnInt/* | IMR_TXFOVW*/ /*| IMR_TBDOK | IMR_TBDER*/);// IMR_ROK | IMR_BcnInt | IMR_RDU | IMR_RXFOVW | IMR_RXCMDOK;
#endif
		//priv->pshare->InterruptMask = HIMR_ROK | HIMR_BCNDMA0 | HIMR_RDU | HIMR_RXFOVW;

        {
            priv->pshare->InterruptMask = HIMR_ROK | HIMR_BCNDMA0 | HIMR_RXFOVW;
            priv->pshare->InterruptMaskExt = 0;
        }

#ifdef MP_TEST
		if (priv->pshare->rf_ft_var.mp_specific) {
					//3 Integrated into HAL code
					priv->pshare->InterruptMask	|= HIMR_BEDOK;
		}
#endif

		if (opmode & WIFI_AP_STATE) {
#if defined(CONFIG_RTL_92E_SUPPORT) || defined(CONFIG_RTL_8812_SUPPORT) || defined(CONFIG_RTL_8723B_SUPPORT)
				if ((GET_CHIP_VER(priv) == VERSION_8192E) || (GET_CHIP_VER(priv) == VERSION_8812E) || (GET_CHIP_VER(priv) == VERSION_8723B)) {
					if (priv->pmib->dot11WdsInfo.wdsPure) {
						//WDEBUG("pure-WDS mode don't enable HIMR_92E_BcnInt | HIMR_92E_TBDOK | HIMR_92E_TBDER \n");
					} else {
						priv->pshare->InterruptMask |= HIMR_92E_BcnInt | HIMR_92E_TBDOK | HIMR_92E_TBDER;
					}
				} else
#endif
				{
					priv->pshare->InterruptMask |= HIMR_BCNDOK0 | HIMR_TXBCNERR;
				}
		}
#ifdef CLIENT_MODE
		else if (opmode & WIFI_ADHOC_STATE) {
					priv->pshare->InterruptMask |= (HIMR_TXBCNERR | HIMR_TXBCNOK);
		}
#endif

#endif


		// FGPA does not have eeprom now
//	RTL_W8(_9346CR_, 0);
		/*
			// ===========================================================================================
			// Download Firmware
			// allocate memory for tx cmd packet
			if((priv->pshare->txcmd_buf = (unsigned char *)kmalloc((LoadPktSize), GFP_ATOMIC)) == NULL) {
				printk("not enough memory for txcmd_buf\n");
				return -1;
			}

			priv->pshare->cmdbuf_phyaddr = get_physical_addr(priv, priv->pshare->txcmd_buf,
					LoadPktSize, PCI_DMA_TODEVICE);
		*/



	}

	if (errorFlag) {
		panic_printk("%s, %d, errorFlag:%08x \n", __FUNCTION__, __LINE__, errorFlag);
		RESTORE_INT(x);
		return -1;
	}
	/*
		MacConfigAfterFwDownload(priv);
	*/

	// Adaptive Rate Table for Basic Rate
	val32 = 0;
	for (i = 0; i < 32; i++) {
		if (AP_BSSRATE[i]) {
			if (AP_BSSRATE[i] & 0x80)
				val32 |= get_bit_value_from_ieee_value(AP_BSSRATE[i] & 0x7f);
		}
	}
	val32 |= (priv->pmib->dot11nConfigEntry.dot11nBasicMCS << 12);

	if (IS_HAL_CHIP(priv)) {
		// Do Nothing
		// Filen: it is not necessary to check !?
	} else if(CONFIG_WLAN_NOT_HAL_EXIST)
	{//not HAL
		unsigned int delay_count = 10;
		do {
			if (!is_h2c_buf_occupy(priv))
				break;
			delay_us(5);
			delay_count--;
		} while (delay_count);
	}


		{
		}

//	kfree(priv->pshare->txcmd_buf);

#ifdef MP_TEST
	if (!priv->pshare->rf_ft_var.mp_specific)
#endif
		if (opmode & WIFI_AP_STATE) {
			if (priv->auto_channel == 0) {
				DEBUG_INFO("going to init beacon\n");
				init_beacon(priv);
			}
		}

	//3 Enable IMR
	if (IS_HAL_CHIP(priv)) {
#if 0   //Filen: defer to open after drv_open
		GET_HAL_INTERFACE(priv)->EnableIMRHandler(priv);
#endif
	} else if(CONFIG_WLAN_NOT_HAL_EXIST)
	{//not HAL
		//enable interrupt

			{
				RTL_W32(HIMR, priv->pshare->InterruptMask);
			}
//	RTL_W32(IMR+4, priv->pshare->InterruptMaskExt);
//	RTL_W32(IMR, 0xffffffff);
//	RTL_W8(IMR+4, 0x3f);

	}

	// ===========================================================================================

#ifdef CHECK_HANGUP
	if (priv->reset_hangup)
		priv->pshare->CurrentChannelBW = priv->pshare->is_40m_bw;
	else
#endif
	{
		if (opmode & WIFI_AP_STATE)
			priv->pshare->CurrentChannelBW = priv->pshare->is_40m_bw;
		else
			priv->pshare->CurrentChannelBW = HT_CHANNEL_WIDTH_20;
	}

	if (!priv->pshare->rf_ft_var.disable_pwr_by_rate
#ifdef SUPPORT_RTL8188E_TC
		&& (!((GET_CHIP_VER(priv) == VERSION_8188E) && IS_TEST_CHIP(priv)))
#endif
	)
	{
		if(1
		    && (GET_CHIP_VER(priv) != VERSION_8822B)
			&& (GET_CHIP_VER(priv) != VERSION_8197F) /* 97F doesn't set power by rate to Tx AGC registers */
		)
		{
			// get default Tx AGC offset
			*(unsigned int *)(&priv->pshare->phw->MCSTxAgcOffset_A[0])  = cpu_to_be32(RTL_R32(rTxAGC_A_Mcs03_Mcs00));
			*(unsigned int *)(&priv->pshare->phw->MCSTxAgcOffset_A[4])  = cpu_to_be32(RTL_R32(rTxAGC_A_Mcs07_Mcs04));
			*(unsigned int *)(&priv->pshare->phw->MCSTxAgcOffset_A[8])  = cpu_to_be32(RTL_R32(rTxAGC_A_Mcs11_Mcs08));
			*(unsigned int *)(&priv->pshare->phw->MCSTxAgcOffset_A[12]) = cpu_to_be32(RTL_R32(rTxAGC_A_Mcs15_Mcs12));
			*(unsigned int *)(&priv->pshare->phw->OFDMTxAgcOffset_A[0]) = cpu_to_be32(RTL_R32(rTxAGC_A_Rate18_06));
			*(unsigned int *)(&priv->pshare->phw->OFDMTxAgcOffset_A[4]) = cpu_to_be32(RTL_R32(rTxAGC_A_Rate54_24));
			*(unsigned int *)(&priv->pshare->phw->CCKTxAgc_A[0]) = cpu_to_be32((RTL_R32(rTxAGC_A_CCK11_2_B_CCK11) & 0xffffff00)
					| RTL_R8(rTxAGC_A_CCK1_Mcs32 + 1));


			*(unsigned int *)(&priv->pshare->phw->MCSTxAgcOffset_B[0])  = cpu_to_be32(RTL_R32(rTxAGC_B_Mcs03_Mcs00));
			*(unsigned int *)(&priv->pshare->phw->MCSTxAgcOffset_B[4])  = cpu_to_be32(RTL_R32(rTxAGC_B_Mcs07_Mcs04));
			*(unsigned int *)(&priv->pshare->phw->MCSTxAgcOffset_B[8])  = cpu_to_be32(RTL_R32(rTxAGC_B_Mcs11_Mcs08));
			*(unsigned int *)(&priv->pshare->phw->MCSTxAgcOffset_B[12]) = cpu_to_be32(RTL_R32(rTxAGC_B_Mcs15_Mcs12));
			*(unsigned int *)(&priv->pshare->phw->OFDMTxAgcOffset_B[0]) = cpu_to_be32(RTL_R32(rTxAGC_B_Rate18_06));
			*(unsigned int *)(&priv->pshare->phw->OFDMTxAgcOffset_B[4]) = cpu_to_be32(RTL_R32(rTxAGC_B_Rate54_24));
			*(unsigned int *)(&priv->pshare->phw->CCKTxAgc_B[0]) = cpu_to_be32((RTL_R8(rTxAGC_A_CCK11_2_B_CCK11) << 24)
					| (RTL_R32(rTxAGC_B_CCK5_1_Mcs32) >> 8));
#if 0
			for(i=0;i<=12;i+=4){
				panic_printk("MCSTxAgcOffset_A[%d]=%x\n",i,*(unsigned int *)(&priv->pshare->phw->MCSTxAgcOffset_A[i]));
			}
			for(i=0;i<=12;i+=4){
				panic_printk("MCSTxAgcOffset_B[%d]=%x\n",i,*(unsigned int *)(&priv->pshare->phw->MCSTxAgcOffset_B[i]));
			}
			for(i=0;i<=4;i+=4){
				panic_printk("OFDMTxAgcOffset_A[%d]=%x\n",i,*(unsigned int *)(&priv->pshare->phw->OFDMTxAgcOffset_A[i]));
			}
			for(i=0;i<=4;i+=4){
				panic_printk("OFDMTxAgcOffset_B[%d]=%x\n",i,*(unsigned int *)(&priv->pshare->phw->OFDMTxAgcOffset_B[i]));
			}
			panic_printk("CCKTxAgc_A[0]=%x\n",*(unsigned int *)(&priv->pshare->phw->CCKTxAgc_A[0]));
			panic_printk("CCKTxAgc_B[0]=%x\n",*(unsigned int *)(&priv->pshare->phw->CCKTxAgc_B[0]));
#endif
		}

		if (priv->pshare->txpwr_pg_format_abs)
		{
			if(( GET_CHIP_VER(priv) == VERSION_8814A)||( GET_CHIP_VER(priv) == VERSION_8822B))
				set_target_power_8814(priv);
			else
			set_target_power(priv);
		}

	}

#ifdef ADD_TX_POWER_BY_CMD
	assign_txpwr_offset(priv);
#endif

#ifdef TXPWR_LMT
	if (!priv->pshare->rf_ft_var.disable_txpwrlmt)
	{
#ifdef TXPWR_LMT_NEWFILE
	if((GET_CHIP_VER(priv) == VERSION_8812E) || (GET_CHIP_VER(priv) == VERSION_8188E) || IS_HAL_CHIP(priv)) {
		PHY_ConfigTXLmtWithParaFile_new(priv);
#ifdef BEAMFORMING_AUTO
		if( GET_CHIP_VER(priv) == VERSION_8814A || GET_CHIP_VER(priv) == VERSION_8822B || GET_CHIP_VER(priv) == VERSION_8197F)
			PHY_ConfigTXLmtWithParaFile_new_TXBF(priv);
#endif
	} else if (CONFIG_WLAN_NOT_HAL_EXIST)
#endif
		PHY_ConfigTXLmtWithParaFile(priv);
	}
#endif
	RESTORE_INT(x);

#ifdef _TRACKING_TABLE_FILE
	if (priv->pshare->rf_ft_var.pwr_track_file) {
		if((ODMPTR->ConfigBBRF) && (GET_CHIP_VER(priv) != VERSION_8188E))
			ODM_ConfigRFWithTxPwrTrackHeaderFile(ODMPTR);
		else
			PHY_ConfigTXPwrTrackingWithParaFile(priv);
	}
#endif


	{
		if ((priv->pmib->dot11RFEntry.ther < 0x07) || (priv->pmib->dot11RFEntry.ther > 0x32)) {
			DEBUG_ERR("TPT: unreasonable target ther %d, disable tpt\n", priv->pmib->dot11RFEntry.ther);
			priv->pmib->dot11RFEntry.ther = 0;
		}
	}


	/*
		if (opmode & WIFI_AP_STATE)
		{
			if (priv->auto_channel == 0) {
				DEBUG_INFO("going to init beacon\n");
				init_beacon(priv);
			}
		}
	*/

#ifdef BT_COEXIST
	if(GET_CHIP_VER(priv) == VERSION_8192E && priv->pshare->rf_ft_var.btc == 1){
			bt_coex_init(priv);
	}
#endif

	if (IS_HAL_CHIP(priv)) {
		if ((GET_CHIP_VER(priv) == VERSION_8881A)||(GET_CHIP_VER(priv) == VERSION_8814A)||(GET_CHIP_VER(priv) == VERSION_8197F)){
			//Don't enable BB here
		}

	} else if(CONFIG_WLAN_NOT_HAL_EXIST)
	{//not HAL
		{
			/*---- Set CCK and OFDM Block "ON"----*/
			PHY_SetBBReg(priv, rFPGA0_RFMOD, bOFDMEn, 1);
				PHY_SetBBReg(priv, rFPGA0_RFMOD, bCCKEn, 1);
		}
	}

	delay_ms(2);

#ifdef MP_TEST
	if (priv->pshare->rf_ft_var.mp_specific) {
		if ( IS_HAL_CHIP(priv) ) {
			u4Byte  MACAddr = 0x87654321;
			GET_HAL_INTERFACE(priv)->SetHwRegHandler(priv, HW_VAR_ETHER_ADDR, (pu1Byte)&MACAddr);
		} else if(CONFIG_WLAN_NOT_HAL_EXIST)
			RTL_W32(MACID, 0x87654321);

		delay_ms(150);
	}
#endif
//	RESTORE_INT(x);


	/*
		if(priv->pshare->rf_ft_var.ofdm_1ss_oneAnt == 1){// use one PATH for ofdm and 1SS
			Switch_1SS_Antenna(priv, 2);
			Switch_OFDM_Antenna(priv, 2);
		}
	*/

	watchdog_kick();
	delay_ms(100);
	SAVE_INT_AND_CLI(x);

	//RTL_W32(0x100, RTL_R32(0x100) | BIT(14)); //for 8190 fw debug

	// init DIG variables
//	val32 = 0x40020064;	// 0x20010020
	val32 = 0x20010064;	// 0x20010020
//
	priv->pshare->threshold0 = (unsigned short)(val32 & 0x000000FF);
	priv->pshare->threshold1 = (unsigned short)((val32 & 0x000FFF00) >> 8);
	priv->pshare->threshold2 = (unsigned short)((val32 & 0xFFF00000) >> 20);
	priv->pshare->digDownCount = 0;
	priv->pshare->digDeadPoint = 0;
	priv->pshare->digDeadPointHitCount = 0;

	if (priv->pshare->rf_ft_var.nbi_filter_enable &&
		(priv->pmib->dot11RFEntry.phyBandSelect == PHY_BAND_2G)) {
		NBI_filter_on(priv);
	}

	set_DIG_state(priv, 1);		// DIG on


    if(GET_CHIP_VER(priv) == VERSION_8197F) {
        if (priv->pmib->dot11RFEntry.MIMO_TR_mode == MIMO_1T1R) {
            config_phydm_trx_mode_8197f(ODMPTR, ODM_RF_A, ODM_RF_A, 0);
        }
        else {
            config_phydm_trx_mode_8197f(ODMPTR, (ODM_RF_A|ODM_RF_B), (ODM_RF_A|ODM_RF_B), priv->pmib->dot11RFEntry.tx2path);
        }
    }

    if(GET_CHIP_VER(priv) == VERSION_8822B) {
        if (priv->pmib->dot11RFEntry.MIMO_TR_mode == MIMO_1T1R) {
            config_phydm_trx_mode_8822b(ODMPTR, ODM_RF_A, ODM_RF_A, 0);
        }
        else {
            config_phydm_trx_mode_8822b(ODMPTR, (ODM_RF_A|ODM_RF_B), (ODM_RF_A|ODM_RF_B), priv->pmib->dot11RFEntry.tx2path);
        }
    }

	if ((GET_CHIP_VER(priv) == VERSION_8812E) || IS_HAL_CHIP(priv)) {
		if (GET_CHIP_VER(priv) == VERSION_8192E) {
			if (priv->pmib->dot11RFEntry.tx2path)
			{
				RTL_W32(0x90C, 0xc3321333);
				RTL_W32(0x80C, RTL_R32(0x80C) & ~BIT(31));
//				RTL_W8(0x6D8, RTL_R8(0x6D8) | 0x3F);
				RTL_W8(0x6D8, RTL_R8(0x6D8) | 0xFF);

				RTL_W8(0xA07, 0xC1);
				RTL_W8(0xA11, RTL_R8(0xA11) & ~BIT(5));
				//RTL_W8(0xA20, (RTL_R8(0xA20) & ~BIT(5)) | BIT(4));
				//RTL_W8(0xA2E, RTL_R8(0xA2E) | BIT(3) | BIT(2));
				//RTL_W8(0xA2F, (RTL_R8(0xA2F) & ~BIT(5)) | BIT(4));
				//RTL_W8(0xA75, RTL_R8(0xA75) | BIT(0));
				RTL_W32(0xC8C, 0xa0240000);
				RTL_W8(0x800, RTL_R8(0x800) & ~BIT(1));
			} else {
				PHY_SetBBReg(priv, 0x90C, BIT(30), 0);
			}
			if (priv->pmib->dot11RFEntry.bcn2path){
				RTL_W32(0x80c,RTL_R32(0x80c)|BIT(31));
				RTL_W8(0x6D8, RTL_R8(0x6D8) | 0xCF);
			}
		}
	} else if (CONFIG_WLAN_NOT_HAL_EXIST)
    {
        if (priv->pshare->rf_ft_var.cck_tx_pathB) {
            RTL_W8(0xa07, 0x40);	// 0x80 -> 0x40    CCK path B Tx
            RTL_W8(0xa0b, 0x84);	// 0x88 -> 0x84    CCK path B Tx
        }

        // CCK path A Tx
        #ifdef CONFIG_POCKET_ROUTER_SUPPORT
        {
            RTL_W8(0xa07, (RTL_R8(0xa07) & 0xbf));
            RTL_W8(0xa0b, (RTL_R8(0xa0b) & 0xfb));
        }
        #endif



		if (IS_HAL_CHIP(priv)) {
		} else if(CONFIG_WLAN_NOT_HAL_EXIST)
		{//not HAL
			RTL_W16(EIFS, 0x0040);	// eifs = 40 us

			RTL_W32(0x350, RTL_R32(0x350) | BIT(26));	// tx status check
		}

#ifdef HIGH_POWER_EXT_PA
		if ((GET_CHIP_VER(priv) == VERSION_8192C) || (GET_CHIP_VER(priv) == VERSION_8188C)) {
			if (priv->pshare->rf_ft_var.use_ext_pa) {
				priv->pmib->dot11RFEntry.trswitch = 1;
				PHY_SetBBReg(priv, 0x870, BIT(10), 0);
				if (GET_CHIP_VER(priv) == VERSION_8192C)
					PHY_SetBBReg(priv, 0x870, BIT(26), 0);
			}
		}
#endif

#if defined(SW_ANT_SWITCH) || defined(HW_ANT_SWITCH)
//	priv->pmib->dot11RFEntry.trswitch = 1;
#endif
		if(GET_CHIP_VER(priv) != VERSION_8188E){
			if (priv->pmib->dot11RFEntry.trswitch)
				RTL_W8(GPIO_MUXCFG, RTL_R8(GPIO_MUXCFG) | TRSW0EN);
			else
				RTL_W8(GPIO_MUXCFG, RTL_R8(GPIO_MUXCFG) & ~TRSW0EN);
		}


		if (!priv->pmib->dot11DFSEntry.disable_DFS) {
			RTL_W8(0xc50, 0x24);
			delay_us(10);
			RTL_W8(0xc50, 0x20);
		}

		if (get_rf_mimo_mode(priv) == MIMO_2T2R) {
			RTL_W8(0xA20, (RTL_R8(0xA20) & ~BIT(5)) | BIT(4));
			RTL_W8(0xA2E, RTL_R8(0xA2E) | BIT(3) | BIT(2));
			RTL_W8(0x800, RTL_R8(0x800) & ~BIT(1));

			if (priv->pmib->dot11RFEntry.tx2path) {
				RTL_W32(0x90C, 0x83321333);
				RTL_W32(0x80C, RTL_R32(0x80C) & ~BIT(31));
				RTL_W8(0x6D8, RTL_R8(0x6D8) | 0x3F);
				RTL_W8(0xA07, 0xC1);
				RTL_W8(0xA11, RTL_R8(0xA11) & ~BIT(5));
				RTL_W8(0xA2F, (RTL_R8(0xA2F) & ~BIT(5)) | BIT(4));
				RTL_W8(0xA75, RTL_R8(0xA75) | BIT(0));
				RTL_W32(0xC8C, 0xa0240000);
			}
				PHY_SetBBReg(priv, 0x90C, BIT(30), 0);
		} else if (get_rf_mimo_mode(priv) == MIMO_1T1R) {
			if (priv->pmib->dot11RFEntry.tx2path) {
				DEBUG_INFO("Not 2T2R, disable tx2path\n");
				priv->pmib->dot11RFEntry.tx2path = 0;
			}
			if (priv->pmib->dot11RFEntry.txbf) {
				DEBUG_INFO("Not 2T2R, disable txbf\n");
				priv->pmib->dot11RFEntry.txbf = 0;
			}
		}

	}


{
	//Set 0x55d=0 except ADHOC Mode for all IC (BCN_MAX_ERR)
	#ifdef CLIENT_MODE
		if (OPMODE & WIFI_ADHOC_STATE)
			RTL_W8(0x55d, 0xff);
		else
	#endif
		if(OPMODE & WIFI_AP_STATE) {
			if((GET_CHIP_VER(priv) == VERSION_8192C) || (GET_CHIP_VER(priv)==VERSION_8188C))
				RTL_W8(0x55d, 0xff); //Set 92C to MAX value to avoid init fail for some un-healthy demo boards.
			else
				RTL_W8(0x55d, 0x1); //Allow 1 Beacon Error only //Always Tx Beacon within PIFS
		}

//	printk("0x55d = 0x%x\n", RTL_R8(0x55d));
}

#ifdef DRVMAC_LB
	if (!priv->pmib->miscEntry.drvmac_lb)
#endif
	{
		if (IS_OUTSRC_CHIP(priv)) {
			ODM_DMInit(ODMPTR);			// DM parameters init
			EdcaParaInit(priv);
#if (PHYDM_LA_MODE_SUPPORT == 1)
			ADCSmp_Init(ODMPTR);
#endif
        }
        else
        {
            rtl8192cd_AdaptivityInit(priv);
        }

// TODO: the following code need to be integrated into WlanHAL.... for 8814A...
		if ((GET_CHIP_VER(priv) == VERSION_8812E) || (GET_CHIP_VER(priv) == VERSION_8881A) || (GET_CHIP_VER(priv) == VERSION_8814A) || (GET_CHIP_VER(priv) == VERSION_8822B)) { //eric-8822
// ?	did twice ?
//		phy_BB8192CD_Check_ParaFile(priv);

			priv->pshare->No_RF_Write = 0;

			//Set some fine-tune TRX related Registers (Avoid PHY parameters error)
			if(GET_CHIP_VER(priv) == VERSION_8822B){

				//WorkAround Solution to Stop 8822 HW Tx (if fw downloaded)
				if(priv->pmib->dot11DFSEntry.disable_tx)
				RTL_W8(TXPAUSE, 0xff);


				set_8822_trx_regs(priv);

				//eric-8822 lck
				PHY_LCCalibrate_8822B(ODMPTR);
			}

			SwBWMode(priv, priv->pshare->CurrentChannelBW, priv->pshare->offset_2nd_chan);
			SwChnl(priv, priv->pmib->dot11RFEntry.dot11channel, priv->pshare->offset_2nd_chan);
			priv->pshare->No_RF_Write = 1;
			if (priv->pmib->dot11RFEntry.phyBandSelect & PHY_BAND_5G) {
				RTL_W32(0x808, RTL_R32(0x808) | BIT(29));
				RTL_W8(0x454, RTL_R8(0x454) | BIT(7));
			} else { //PHY_BAND_2G
				RTL_W32(0x808, RTL_R32(0x808) | BIT(29) | BIT(28));
				RTL_W8(0x454, RTL_R8(0x454) & ~ BIT(7));
			}
			if ((GET_CHIP_VER(priv) != VERSION_8814A) && (GET_CHIP_VER(priv) != VERSION_8822B))
			{
				RTL_W16(0x4ca, 0x1f1f);
				RTL_W8(REG_RA_TRY_RATE_AGG_LMT_8812, 0x0);				// try rate agg limit
			}



//		RTL_W32(0x2c, 0x28a3e200);


			// 8814 merge issue

#if 1 //for 8812 IOT issue with 11N NICs
			if (GET_CHIP_VER(priv) != VERSION_8822B)
			{
				DEBUG_INFO("0x838 B(1)= 0, 0x456 = 0x32 \n");
				RTL_W8(0x838, (RTL_R8(0x838)& ~ BIT(0))); //Disbale CCA
			}

#endif
		}
	}


	RESTORE_INT(x);

    if(GET_CHIP_VER(priv) == VERSION_8197F) {
        PHY_LCCalibrate_8197F(ODMPTR);
	unsigned int rf_path=0, count=0, no_iqk=0;
	for(rf_path=0; rf_path<2 ; rf_path++) {
		PHY_SetRFReg(priv, rf_path, RF_WE_LUT, 0x80000,0x1); // RF Mode table write enable
		PHY_SetRFReg(priv, rf_path, RF_WE_LUT, 0x80000,0x1); // RF Mode table write enable

		PHY_SetRFReg(priv, rf_path, RF_RCK_OS, 0xfffff,0x20000); // Select IQK Tx mode 0x30=0x18000
		count=0;
		while(PHY_QueryRFReg(priv, rf_path, RF_RCK_OS, 0xfffff, 1)!=0x20000) {
			delay_us(2);
			PHY_SetRFReg(priv, rf_path, RF_RCK_OS, 0xfffff,0x20000);
			if (++count > 100)
				break;
		}
		if (PHY_QueryRFReg(priv, rf_path, RF_RCK_OS, 0xfffff, 1)!=0x20000) {
			printk("%s:%d Set RF mode table to TX TQK mode fail!!!\n", __FUNCTION__, __LINE__);
			no_iqk = 1;
		} else {
			PHY_SetRFReg(priv, rf_path, RF_TXPA_G1, 0xfffff,0x0005f); // Set Table data
			PHY_SetRFReg(priv, rf_path, RF_TXPA_G2, 0xfffff,0x01ff7); // /*PA off, default: 0x1fff*/
		}

		PHY_SetRFReg(priv, rf_path, RF_RCK_OS, 0xfffff,0x30000); // Select IQK Rx mode 0x30=0x30000
		count=0;
		while(PHY_QueryRFReg(priv, rf_path, RF_RCK_OS, 0xfffff, 1)!=0x30000) {
			delay_us(2);
			PHY_SetRFReg(priv, rf_path, RF_RCK_OS, 0xfffff,0x30000);
			if (++count > 100)
				break;
		}
		if (PHY_QueryRFReg(priv, rf_path, RF_RCK_OS, 0xfffff, 1)!=0x30000) {
			printk("%s:%d Set RF mode table to RX TQK mode-1 fail!!!\n", __FUNCTION__, __LINE__);
			no_iqk = 1;
		} else {
			PHY_SetRFReg(priv, rf_path, RF_TXPA_G1, 0xfffff,0x0005f); // Set Table data
			PHY_SetRFReg(priv, rf_path, RF_TXPA_G2, 0xfffff,0xf1df3); ///*PA off, deafault:0xf1dfb*/
		}

		PHY_SetRFReg(priv, rf_path, RF_RCK_OS, 0xfffff,0x38000); // Select IQK Rx mode 0x30=0x38000
		count=0;
		while(PHY_QueryRFReg(priv, rf_path, RF_RCK_OS, 0xfffff, 1)!=0x38000) {
			delay_us(2);
			PHY_SetRFReg(priv, rf_path, RF_RCK_OS, 0xfffff,0x38000);
			if (++count > 100)
				break;
		}
		if (PHY_QueryRFReg(priv, rf_path, RF_RCK_OS, 0xfffff, 1)!=0x38000) {
			printk("%s:%d Set RF mode table to RX TQK mode-2 fail!!!\n", __FUNCTION__, __LINE__);
			no_iqk = 1;
		} else {
			PHY_SetRFReg(priv, rf_path, RF_TXPA_G1, 0xfffff,0x0005f); // Set Table data
			PHY_SetRFReg(priv, rf_path, RF_TXPA_G2, 0xfffff,0xf1ff2); // /*PA off : default:0xf1ffa*/
		}

		PHY_SetRFReg(priv, rf_path, RF_WE_LUT, 0x80000,0x0); // RF Mode table write disable
		PHY_SetRFReg(priv, rf_path, RF_WE_LUT, 0x80000,0x0); // RF Mode table write disable

		if (no_iqk) {
			printk("%s:%d Dont't do IQK!!!\n", __FUNCTION__, __LINE__);
			ODM_CmnInfoUpdate(ODMPTR, ODM_CMNINFO_ABILITY, ODMPTR->SupportAbility & (~ ODM_RF_CALIBRATION));
		}
	}

        if(_GET_HAL_DATA(priv)->cutVersion == ODM_CUT_A){
            /* Get channel 1 CV for LCK */
            PHY_SetRFReg(priv, RF92CD_PATH_A, rRfChannel, 0xff, 1);
            priv->pshare->rf_ft_var.cv_ch1 = PHY_QueryRFReg(priv, RF92CD_PATH_A, 0xB2, 0xff, 1);
            //panic_printk("cv_ch1=%x\n", priv->pshare->rf_ft_var.cv_ch1);
        }
        SwBWMode(priv, priv->pshare->CurrentChannelBW, priv->pshare->offset_2nd_chan);
        SwChnl(priv, priv->pmib->dot11RFEntry.dot11channel, priv->pshare->offset_2nd_chan);
        //PHY_QueryBBReg(priv, rOFDM0_XATxIQImbalance, bMaskOFDM_D);
        panic_printk("[97F] Default BB Swing=%u\n",ODMPTR->RFCalibrateInfo.DefaultOfdmIndex);
        /* 97F BB swing default values are set by PHY_REG, currently 0dB for intpa, -5dB for extpa*/
    }



#if 1 // TODO: Filen, tmp setting for tuning TP
#endif

// TODO : TEMP add by Eric , check 8197F need add ?
    if (get_rf_mimo_mode(priv) == MIMO_1T1R){
    }

#ifdef WMM_DSCP_C42
	priv->pshare->aggrmax_bak = RTL_R16(PROT_MODE_CTRL + 2);
#endif

    if (IS_OUTSRC_CHIP(priv)) {
        if((GET_CHIP_VER(priv) != VERSION_8197F) && (GET_CHIP_VER(priv) != VERSION_8822B)) /* 97F & 8822 need to disable CFE (to prevent tx hang)*/
        {
			if(priv->pmib->dot11BssType.net_work_type != WIRELESS_11B)
            	RTL_W16( RD_CTRL, RTL_R16( RD_CTRL) & ~(DIS_TXOP_CFE));
        }
    }

	//Tego added power on protection to avoid wlan hang after suddenly wdg reboot, button reviewed

#ifdef THERMAL_CONTROL
	priv->pshare->rf_ft_var.current_path = priv->pmib->dot11RFEntry.tx2path+1;
#endif



#ifdef CONFIG_1RCCA_RF_POWER_SAVING
#ifdef MP_TEST
	if (!priv->pshare->rf_ft_var.mp_specific)
#endif
	if (priv->pshare->rf_ft_var.one_path_cca_ps) {
		one_path_cca_power_save(priv, 1);
	}
#endif // CONFIG_1RCCA_RF_POWER_SAVING

	set_bcn_dont_ignore_edcca(priv);

#ifdef USB_INTERFERENCE_ISSUE
#endif // USB_INTERFERENCE_ISSUE




#ifdef THERMAL_CONTROL
	if(priv->pmib->dot11RFEntry.phyBandSelect & PHY_BAND_2G){
		priv->pshare->rf_ft_var.ther_hi = THER_HIGH_2G;
		priv->pshare->rf_ft_var.ther_low =	THER_LOW_2G;
	}else{
		priv->pshare->rf_ft_var.ther_hi = THER_HIGH_5G;
		priv->pshare->rf_ft_var.ther_low =	THER_LOW_5G;
	}
#endif

#ifdef BT_COEXIST
	if(GET_CHIP_VER(priv) == VERSION_8192E){
		extern int bt_state;
		bt_state = 0;
	}
#endif

#ifdef POWER_TRIM
	/* apply kfree if Flash KFREE_ENABLE set or in MP mode*/
	if(GET_CHIP_VER(priv) == VERSION_8197F){
		if (priv->pmib->dot11RFEntry.kfree_enable || priv->pshare->rf_ft_var.mp_specific){
			/* Load power trim (or k free) value from efuse and set RF gain offset */
			power_trim(priv,0);
			priv->pmib->dot11RFEntry.kfree_enable = 1;
		}else{
			/* restore original RF gain offset*/
			power_trim(priv,1);
		}
	}else{
		priv->pmib->dot11RFEntry.kfree_enable = 0;
	}
#endif

#ifdef THER_TRIM
	if(GET_CHIP_VER(priv) == VERSION_8197F){
		ther_trim_efuse(priv);
	}else{
		priv->pshare->rf_ft_var.ther_trim_enable = 0;
	}
#endif

	if((GET_CHIP_VER(priv) == VERSION_8822B) && (GET_CHIP_VER_8822(priv) >=3))
		TxACurrentCalibration(priv);

		if((GET_CHIP_VER(priv) == VERSION_8822B) || (GET_CHIP_VER(priv) == VERSION_8197F)){
			if(priv->pshare->rf_ft_var.force_bcn){
				PHY_SetBBReg(priv,0x528,BIT18,0);  //enable force beacon
				RTL_W8(0x58c,0x80);//4ms
				RTL_W8(0x55d,0);//force beacon every time
				PHY_SetBBReg(priv,0x574,BIT26,0);
				RTL_W8(0x58e,0x8);
				PHY_SetBBReg(priv,0x524,BIT12,0x1);
				PHY_SetBBReg(priv,0x524,BIT13,0x1);
			}else{
				PHY_SetBBReg(priv,TBTT_PROHIBIT,0x000FFF00,0x138); //10ms
			}
		}



	// for new IC, don't receive ICV error packet
	if (GET_CHIP_VER(priv) >= VERSION_8192E && GET_CHIP_VER(priv) != VERSION_8822B) {
		RTL_W32(RCR, RTL_R32(RCR) & ~RCR_AICV);
	}

	RESTORE_INT(x);
	DBFEXIT;

	return 0;

}






#define	SET_RTL8192CD_RF_HALT(priv)						\
{ 														\
	unsigned char u1bTmp;								\
														\
	do													\
	{													\
		u1bTmp = RTL_R8(LDOV12D_CTRL);					\
		u1bTmp |= BIT(0); 								\
		RTL_W8(LDOV12D_CTRL, u1bTmp);					\
		RTL_W8(SPS1_CTRL, 0x0);							\
		RTL_W8(TXPAUSE, 0xFF);							\
		RTL_W16(CMDR, 0x57FC);							\
		delay_us(100);									\
		RTL_W16(CMDR, 0x77FC);							\
		RTL_W8(PHY_CCA, 0x0);							\
		delay_us(10);									\
		RTL_W16(CMDR, 0x37FC);							\
		delay_us(10);									\
		RTL_W16(CMDR, 0x77FC);							\
		delay_us(10);									\
		RTL_W16(CMDR, 0x57FC);							\
		RTL_W16(CMDR, 0x0000);							\
		u1bTmp = RTL_R8((SYS_CLKR + 1));				\
		if (u1bTmp & BIT(7))							\
		{												\
			u1bTmp &= ~(BIT(6) | BIT(7));				\
			if (!HalSetSysClk8192CD(priv, u1bTmp))		\
			break;										\
		}												\
		RTL_W8(0x03, 0x71);								\
		RTL_W8(0x09, 0x70);								\
		RTL_W8(0x29, 0x68);								\
		RTL_W8(0x28, 0x00);								\
		RTL_W8(0x20, 0x50);								\
		RTL_W8(0x26, 0x0E);								\
	} while (FALSE);									\
}

void Unlock_MCU(struct rtl8192cd_priv *priv)
{
	// 1. To clear C2H
	RTL_W8(C2H_SYNC_BYTE, 0x0);
	// 2. Unlock Overall MCU while(1)
	RTL_W8(MCU_UNLOCK, 0xFF);
}

void FirmwareSelfReset(struct rtl8192cd_priv *priv)
{
	unsigned char u1bTmp;
	unsigned int  Delay = 1000;
	if (priv->pshare->fw_version > 0x21
	   )	{
		RTL_W32(FWIMR, 0x20);
		RTL_W8(REG_HMETFR + 3, 0x20);
		u1bTmp = RTL_R8( REG_SYS_FUNC_EN + 1);
		while (u1bTmp & BIT(2)) {
			Delay--;
			DEBUG_INFO("polling 0x03[2] Delay = %d \n", Delay);
			if (Delay == 0)
				break;
			delay_us(50);
			Unlock_MCU(priv);
			u1bTmp = RTL_R8( REG_SYS_FUNC_EN + 1);
		}
		// restore MCU internal while(1) loop
		RTL_W8(MCU_UNLOCK, 0);
		if (u1bTmp & BIT(2)) {
			DEBUG_ERR("FirmwareSelfReset fail: 0x03=%02x, 0x1EB=0x%02x\n", u1bTmp, RTL_R8(0x1EB));
		} else {
			DEBUG_INFO("FirmwareSelfReset success: 0x03 = %x\n", u1bTmp);
		}
	}
}

//Return Value:
//	1: MAC I/O Registers Enable
//	0: MAC I/O Registers Disable
int check_MAC_IO_Enable(struct rtl8192cd_priv *priv)
{
	//Check PON register to decide
	return ( (RTL_R16(SYS_FUNC_EN) & (FEN_MREGEN | FEN_DCORE)) == (FEN_MREGEN | FEN_DCORE) );
}

int rtl8192cd_stop_hw(struct rtl8192cd_priv *priv)
{
	RTL_W8(0x1c, RTL_R8(0x1c)& ~BIT(1));// unlock reg0x00~0x03 for 8812


		{
			RTL_W32(HIMR, 0);
			RTL_W16(HIMRE, 0);
			RTL_W16(HIMRE + 2, 0);
			RTL_W32(CR, (RTL_R32(CR) & ~(NETYPE_Mask << NETYPE_SHIFT)) | ((NETYPE_NOLINK & NETYPE_Mask) << NETYPE_SHIFT));
		}

	RTL_W8(RCR, 0);
	RTL_W8(TXPAUSE, 0xff);								// Pause MAC TX queue

	{

		// ==== Reset digital sequence ====

			RTL_W8(SYS_FUNC_EN + 1, 0x51);								// reset MCU, MAC register, DCORE
		RTL_W8(MCUFWDL, 0);											// reset MCU ready status



			{
				//3 4.) ==== Disable analog sequence ====
				RTL_W8(AFE_PLL_CTRL, 0x80);			// disable PLL

				{
						RTL_W8(SPS0_CTRL, 0x23);
				}
			}

		{
			RTL_W8(APS_FSMCO + 1, 0x10);
		}
		RTL_W8(RSV_CTRL0, 0x0e);				// lock ISO/CLK/Power control register

		//3 5.) ==== interface into suspend ====
//		RTL_W16(APS_FSMCO, (RTL_R16(APS_FSMCO) & 0x00ff) | (0x18 << 8));	// PCIe suspend mode

	}
	return SUCCESS;
}


#ifdef RTK_AC_SUPPORT  //#if defined(CONFIG_RTL_8812_SUPPORT) || defined(CONFIG_WLAN_HAL_8881A) || defined(CONFIG_WLAN_HAL_8814AE)

#ifdef AC2G_256QAM
#define MAX_NUM_80M 8

unsigned char available_channel_AC_80m[MAX_NUM_80M][4] = {
	{36, 40, 44, 48},
	{52, 56, 60, 64},
	{100, 104, 108, 112},
	{116, 120, 124, 128},
	{132, 136, 140, 144},
	{149, 153, 157, 161},
	{165, 169, 173, 177},
	{1, 5, 9, 13},
};
#else
#define MAX_NUM_80M 7

unsigned char available_channel_AC_80m[MAX_NUM_80M][4] = {
	{36, 40, 44, 48},
	{52, 56, 60, 64},
	{100, 104, 108, 112},
	{116, 120, 124, 128},
	{132, 136, 140, 144},
	{149, 153, 157, 161},
	{165, 169, 173, 177},
};
#endif


void get_txsc_AC(struct rtl8192cd_priv *priv, unsigned char channel)
{
	unsigned char tmp, i, found = 0;

	if (priv->pshare->CurrentChannelBW == HT_CHANNEL_WIDTH_80) {
		for (tmp = 0; tmp < MAX_NUM_80M; tmp ++) {
			for (i = 0; i < 4; i++) {
				if (channel == available_channel_AC_80m[tmp][i]) {
					found = 1;
					//printk("found channel[%d] at [%d][%d]\n", channel, tmp, i);
					break;
				}
			}

			if (found)
				break;
		}

		if (priv->pshare->rf_ft_var.mp_specific==1){
				if(GET_CHIP_VER(priv) == VERSION_8822B){
				i = 2;
				}
		}

		switch (i) {
		case 0:
			//printk("case 0 \n");
			priv->pshare->txsc_20 = _20_B_40_B;
			priv->pshare->txsc_40 = _40_B;
			priv->pshare->offset_2nd_chan = HT_2NDCH_OFFSET_ABOVE;
			break;
		case 1:
			priv->pshare->txsc_20 = _20_A_40_B;
			priv->pshare->txsc_40 = _40_B;
			priv->pshare->offset_2nd_chan = HT_2NDCH_OFFSET_BELOW;
			break;
		case 2:
			priv->pshare->txsc_20 = _20_B_40_A;
			priv->pshare->txsc_40 = _40_A;
			priv->pshare->offset_2nd_chan = HT_2NDCH_OFFSET_ABOVE;
			break;
		case 3:
			priv->pshare->txsc_20 = _20_A_40_A;
			priv->pshare->txsc_40 = _40_A;
			priv->pshare->offset_2nd_chan = HT_2NDCH_OFFSET_BELOW;
			break;
		default:
			break;
		}
	} else if (priv->pshare->CurrentChannelBW == HT_CHANNEL_WIDTH_20_40) {
		if (priv->pshare->offset_2nd_chan == HT_2NDCH_OFFSET_BELOW)
			priv->pshare->txsc_20 = 1;
		else
			priv->pshare->txsc_20 = 2;

		priv->pshare->txsc_40 = 0;
	} else if (priv->pshare->CurrentChannelBW == HT_CHANNEL_WIDTH_20) {
		priv->pshare->txsc_20 = 0;
		priv->pshare->txsc_40 = 0;
	}

	//printk("get_txsc_8812= %d %d \n", priv->pshare->txsc_20, priv->pshare->txsc_40);

}

#endif


void get_txsc_92e(struct rtl8192cd_priv *priv, unsigned char channel)
{
	if (priv->pshare->CurrentChannelBW == HT_CHANNEL_WIDTH_20_40) {
		if (priv->pshare->offset_2nd_chan == HT_2NDCH_OFFSET_BELOW)
			priv->pshare->txsc_20_92e = 1;
		else
			priv->pshare->txsc_20_92e = 2;
	} else if (priv->pshare->CurrentChannelBW == HT_CHANNEL_WIDTH_20) {
		priv->pshare->txsc_20_92e = 0;
	}
}

void SwBWMode(struct rtl8192cd_priv *priv, unsigned int bandwidth, int offset)
{
	unsigned char regBwOpMode, regRRSR_RSC, nCur40MhzPrimeSC;
	unsigned int eRFPath, curMaxRFPath, val;
	priv->pshare->CurrentChannelBW = bandwidth;


//eric-8822
	if ((GET_CHIP_VER(priv) == VERSION_8881A)
		|| (GET_CHIP_VER(priv) == VERSION_8814A)
		|| (GET_CHIP_VER(priv) == VERSION_8822B))
    {
		priv->pshare->No_RF_Write = 0;
		get_txsc_AC(priv, priv->pmib->dot11RFEntry.dot11channel);
		GET_HAL_INTERFACE(priv)->PHYSwBWModeHandler(priv, bandwidth, offset);
		priv->pshare->No_RF_Write = 1;
		return;
	}

	if (GET_CHIP_VER(priv) == VERSION_8192E || GET_CHIP_VER(priv) == VERSION_8197F) {
		get_txsc_92e(priv, priv->pmib->dot11RFEntry.dot11channel);
		GET_HAL_INTERFACE(priv)->PHYSwBWModeHandler(priv, bandwidth, offset);
		return;
	}

	DEBUG_INFO("SwBWMode(): Switch to %s bandwidth\n", bandwidth ? "40MHz" : "20MHz");

		curMaxRFPath = RF92CD_PATH_MAX;

	if (offset == 1)
		nCur40MhzPrimeSC = 2;
	else
		nCur40MhzPrimeSC = 1;

	//3 <1> Set MAC register
	regBwOpMode = RTL_R8(BWOPMODE);
	regRRSR_RSC = RTL_R8(RRSR + 2);

	switch (bandwidth) {
	case HT_CHANNEL_WIDTH_20:
		regBwOpMode |= BW_OPMODE_20MHZ;
		RTL_W8(BWOPMODE, regBwOpMode);
		break;
	case HT_CHANNEL_WIDTH_20_40:
		regBwOpMode &= ~BW_OPMODE_20MHZ;
		RTL_W8(BWOPMODE, regBwOpMode);
		regRRSR_RSC = (regRRSR_RSC & 0x90) | (nCur40MhzPrimeSC << 5);
		RTL_W8(RRSR + 2, regRRSR_RSC);

		// Let 812cd_rx, re-assign value
		if (priv->pshare->is_40m_bw) {
			priv->pshare->Reg_RRSR_2 = 0;
			priv->pshare->Reg_81b = 0;
		}
		break;
	default:
		DEBUG_ERR("SwBWMode(): bandwidth mode error!\n");
		return;
		break;
	}

	//3 <2> Set PHY related register
	switch (bandwidth) {
	case HT_CHANNEL_WIDTH_20:
		PHY_SetBBReg(priv, rFPGA0_RFMOD, bRFMOD, 0x0);
		PHY_SetBBReg(priv, rFPGA1_RFMOD, bRFMOD, 0x0);
		break;
	case HT_CHANNEL_WIDTH_20_40:
		PHY_SetBBReg(priv, rFPGA0_RFMOD, bRFMOD, 0x1);
		PHY_SetBBReg(priv, rFPGA1_RFMOD, bRFMOD, 0x1);
		// Set Control channel to upper or lower. These settings are required only for 40MHz
		PHY_SetBBReg(priv, rCCK0_System, bCCKSideBand, (nCur40MhzPrimeSC >> 1));
		PHY_SetBBReg(priv, rOFDM1_LSTF, 0xC00, nCur40MhzPrimeSC);
		PHY_SetBBReg(priv, 0x818, (BIT(26) | BIT(27)), (nCur40MhzPrimeSC == 2) ? 1 : 2);
		break;
	default:
		DEBUG_ERR("SwBWMode(): bandwidth mode error! %d\n", __LINE__);
		return;
		break;
	}

	//3<3> Set RF related register
	switch (bandwidth) {
	case HT_CHANNEL_WIDTH_20: {
			val = 1;
	}
	break;
	case HT_CHANNEL_WIDTH_20_40:
		val = 0;
		break;
	default:
		DEBUG_ERR("SwBWMode(): bandwidth mode error! %d\n", __LINE__);
		return;
		break;
	}

	for (eRFPath = RF92CD_PATH_A; eRFPath < curMaxRFPath; eRFPath++)	{
	}

#if 0
	if (priv->pshare->rf_ft_var.use_frq_2_3G)
		PHY_SetRFReg(priv, RF90_PATH_C, 0x2c, 0x60, 0);
#endif
#ifdef TX_EARLY_MODE
#endif
}


#if defined(SUPPORT_TX_AMSDU) || defined (P2P_SUPPORT)
void setup_timer2(struct rtl8192cd_priv *priv, unsigned int timeout)
{
	unsigned int current_value;

    if ( IS_HAL_CHIP(priv) ) {
        GET_HAL_INTERFACE(priv)->GetHwRegHandler(priv, HW_VAR_TSF_TIMER, (pu1Byte)&current_value);
    } else
    {
        current_value = RTL_R32(TSFTR);
    }

	if (TSF_LESS(timeout, current_value))
		timeout = current_value + 20;

    if ( IS_HAL_CHIP(priv) ) {
        GET_HAL_INTERFACE(priv)->SetHwRegHandler(priv, HW_VAR_PS_TIMER, (pu1Byte)&timeout);
    } else
    {
    	RTL_W32(TIMER1, timeout);
    }

	if ( IS_HAL_CHIP(priv) ) {
        GET_HAL_INTERFACE(priv)->AddInterruptMaskHandler(priv, HAL_INT_TYPE_FS_TIMEOUT0);
	} else if(CONFIG_WLAN_NOT_HAL_EXIST)
	{//not HAL
			RTL_W32(HIMR, RTL_R32(HIMR) | HIMR_TIMEOUT2);
	}
}


void cancel_timer2(struct rtl8192cd_priv *priv)
{

	if ( IS_HAL_CHIP(priv) ) {
        GET_HAL_INTERFACE(priv)->RemoveInterruptMaskHandler(priv, HAL_INT_TYPE_FS_TIMEOUT0);
	} else if(CONFIG_WLAN_NOT_HAL_EXIST)
	{//not HAL
			RTL_W32(HIMR, RTL_R32(HIMR) & ~HIMR_TIMEOUT2);
	}
}
#endif




#if 0
void tx_path_by_rssi(struct rtl8192cd_priv *priv, struct stat_info *pstat, unsigned char enable)
{

	if ((get_rf_mimo_mode(priv) != MIMO_2T2R))
		return; // 1T2R, 1T1R; do nothing

	if (pstat == NULL)
		return;

	if (pstat->sta_in_firmware != 1 || getSTABitMap(&priv->pshare->has_2r_sta, REMAP_AID(pstat))) // 2r STA
		return; // do nothing

	// for debug, by victoryman 20090623
	if (pstat->tx_ra_bitmap & 0xff00000) {
		// this should be a 2r station!!!
		return;
	}

	if (pstat->tx_ra_bitmap & 0xffff000) { // 11n 1R client
		if (enable) {
			if (pstat->rf_info.mimorssi[0] > pstat->rf_info.mimorssi[1])
				Switch_1SS_Antenna(priv, 1);
			else
				Switch_1SS_Antenna(priv, 2);
		} else
			Switch_1SS_Antenna(priv, 3);
	} else if (pstat->tx_ra_bitmap & 0xff0) { // 11bg client
		if (enable) {
			if (pstat->rf_info.mimorssi[0] > pstat->rf_info.mimorssi[1])
				Switch_OFDM_Antenna(priv, 1);
			else
				Switch_OFDM_Antenna(priv, 2);
		} else
			Switch_OFDM_Antenna(priv, 3);
	}

#if 0  // original  setup
	if (priv->pmib->dot11BssType.net_work_type & WIRELESS_11N) { // for 11n 1ss sta
		if (enable) {
			if (pstat->rf_info.mimorssi[0] > pstat->rf_info.mimorssi[1])
				Switch_1SS_Antenna(priv, 1);
			else
				Switch_1SS_Antenna(priv, 2);
		} else
			Switch_1SS_Antenna(priv, 3);
	} else if (priv->pmib->dot11BssType.net_work_type & WIRELESS_11G) { // for 11g
		if (enable) {
			if (pstat->rf_info.mimorssi[0] > pstat->rf_info.mimorssi[1])
				Switch_OFDM_Antenna(priv, 1);
			else
				Switch_OFDM_Antenna(priv, 2);
		} else
			Switch_OFDM_Antenna(priv, 3);
	}
#endif


}
//#endif


// dynamic Rx path selection by signal strength
void rx_path_by_rssi(struct rtl8192cd_priv *priv, struct stat_info *pstat, int enable)
{
	unsigned char highest_rssi = 0, higher_rssi = 0, under_ss_th_low = 0;
	RF92CD_RADIO_PATH_E eRFPath, eRFPath_highest = 0, eRFPath_higher = 0;
	int ant_on_processing = 0;
#ifdef _DEBUG_RTL8192CD_
	char path_name[] = {'A', 'B'};
#endif

	if (enable == FALSE) {
		if (priv->pshare->phw->ant_off_num) {
			priv->pshare->phw->ant_off_num = 0;
			priv->pshare->phw->ant_off_bitmap = 0;
			RTL_W8(rOFDM0_TRxPathEnable, 0x0f);
			RTL_W8(rOFDM1_TRxPathEnable, 0x0f);
			DEBUG_INFO("More than 1 sta, turn on all path\n");
		}
		return;
	}

	for (eRFPath = RF92CD_PATH_A; eRFPath < priv->pshare->phw->NumTotalRFPath; eRFPath++) {
		if (priv->pshare->phw->ant_off_bitmap & BIT(eRFPath))
			continue;

		if (pstat->rf_info.mimorssi[eRFPath] > highest_rssi) {
			higher_rssi = highest_rssi;
			eRFPath_higher = eRFPath_highest;
			highest_rssi = pstat->rf_info.mimorssi[eRFPath];
			eRFPath_highest = eRFPath;
		}

		else if (pstat->rf_info.mimorssi[eRFPath] > higher_rssi) {
			higher_rssi = pstat->rf_info.mimorssi[eRFPath];
			eRFPath_higher = eRFPath;
		}

		if (pstat->rf_info.mimorssi[eRFPath] < priv->pshare->rf_ft_var.ss_th_low)
			under_ss_th_low = 1;
	}

	// for OFDM
	if (priv->pshare->phw->ant_off_num > 0) {
		for (eRFPath = RF92CD_PATH_A; eRFPath < priv->pshare->phw->NumTotalRFPath; eRFPath++) {
			if (!(priv->pshare->phw->ant_off_bitmap & BIT(eRFPath)))
				continue;

			if (highest_rssi >= priv->pshare->phw->ant_on_criteria[eRFPath]) {
				priv->pshare->phw->ant_off_num--;
				priv->pshare->phw->ant_off_bitmap &= (~BIT(eRFPath));
				RTL_W8(rOFDM0_TRxPathEnable, ~(priv->pshare->phw->ant_off_bitmap) & 0x0f);
				RTL_W8(rOFDM1_TRxPathEnable, ~(priv->pshare->phw->ant_off_bitmap) & 0x0f);
				DEBUG_INFO("Path %c is on due to >= %d%%\n",
						   path_name[eRFPath], priv->pshare->phw->ant_on_criteria[eRFPath]);
				ant_on_processing = 1;
			}
		}
	}

	if (!ant_on_processing) {
		if (priv->pshare->phw->ant_off_num < 2) {
			for (eRFPath = RF92CD_PATH_A; eRFPath < priv->pshare->phw->NumTotalRFPath; eRFPath++) {
				if ((eRFPath == eRFPath_highest) || (priv->pshare->phw->ant_off_bitmap & BIT(eRFPath)))
					continue;

				if ((pstat->rf_info.mimorssi[eRFPath] < priv->pshare->rf_ft_var.ss_th_low) &&
						((highest_rssi - pstat->rf_info.mimorssi[eRFPath]) > priv->pshare->rf_ft_var.diff_th)) {
					priv->pshare->phw->ant_off_num++;
					priv->pshare->phw->ant_off_bitmap |= BIT(eRFPath);
					priv->pshare->phw->ant_on_criteria[eRFPath] = highest_rssi + 5;
					RTL_W8(rOFDM0_TRxPathEnable, ~(priv->pshare->phw->ant_off_bitmap) & 0x0f);
					RTL_W8(rOFDM1_TRxPathEnable, ~(priv->pshare->phw->ant_off_bitmap) & 0x0f);
					DEBUG_INFO("Path %c is off due to under th_low %d%% and diff %d%%, will be on at %d%%\n",
							   path_name[eRFPath], priv->pshare->rf_ft_var.ss_th_low,
							   (highest_rssi - pstat->rf_info.mimorssi[eRFPath]),
							   priv->pshare->phw->ant_on_criteria[eRFPath]);
					break;
				}
			}
		}
	}

	// For CCK
	if (priv->pshare->rf_ft_var.cck_sel_ver == 1) {
		if (under_ss_th_low && (pstat->rx_pkts > 20)) {
			if (priv->pshare->phw->ant_cck_sel != ((eRFPath_highest << 2) | eRFPath_higher)) {
				priv->pshare->phw->ant_cck_sel = ((eRFPath_highest << 2) | eRFPath_higher);
				RTL_W8(0xa07, (RTL_R8(0xa07) & 0xf0) | priv->pshare->phw->ant_cck_sel);
				DEBUG_INFO("CCK select default: path %c, optional: path %c\n",
						   path_name[eRFPath_highest], path_name[eRFPath_higher]);
			}
		}
	}
}


// dynamic Rx path selection by signal strength
void rx_path_by_rssi_cck_v2(struct rtl8192cd_priv *priv, struct stat_info *pstat)
{
	int highest_rssi = -1000, higher_rssi = -1000;
	RF92CD_RADIO_PATH_E eRFPath, eRFPath_highest = 0, eRFPath_higher = 0;
#ifdef _DEBUG_RTL8192CD_
	char path_name[] = {'A', 'B'};
#endif

	for (eRFPath = RF92CD_PATH_A; eRFPath < priv->pshare->phw->NumTotalRFPath; eRFPath++) {
		if (pstat->cck_mimorssi_total[eRFPath] > highest_rssi) {
			higher_rssi = highest_rssi;
			eRFPath_higher = eRFPath_highest;
			highest_rssi = pstat->cck_mimorssi_total[eRFPath];
			eRFPath_highest = eRFPath;
		}

		else if (pstat->cck_mimorssi_total[eRFPath] > higher_rssi) {
			higher_rssi = pstat->cck_mimorssi_total[eRFPath];
			eRFPath_higher = eRFPath;
		}
	}

	if (priv->pshare->phw->ant_cck_sel != ((eRFPath_highest << 2) | eRFPath_higher)) {
		priv->pshare->phw->ant_cck_sel = ((eRFPath_highest << 2) | eRFPath_higher);
		RTL_W8(0xa07, (RTL_R8(0xa07) & 0xf0) | priv->pshare->phw->ant_cck_sel);
		DEBUG_INFO("CCK rssi A:%d B:%d C:%d D:%d accu %d pkts\n", pstat->cck_mimorssi_total[0],
				   pstat->cck_mimorssi_total[1], pstat->cck_mimorssi_total[2], pstat->cck_mimorssi_total[3], pstat->cck_rssi_num);
		DEBUG_INFO("CCK select default: path %c, optional: path %c\n",
				   path_name[eRFPath_highest], path_name[eRFPath_higher]);
	}
}


// Tx power control
void tx_power_control(struct rtl8192cd_priv *priv, struct stat_info *pstat, int enable)
{
	if (enable) {
		if (!priv->pshare->phw->lower_tx_power) {
			// TX High power enable
//			set_fw_reg(priv, 0xfd000009, 0, 0);
			if (!priv->pshare->bcnTxAGC)
				RTL_W8(0x364, RTL_R8(0x364) | FW_REG364_HP);
			priv->pshare->phw->lower_tx_power++;

			if ((!priv->pshare->is_40m_bw || (pstat->tx_bw == HT_CHANNEL_WIDTH_20)) &&
					(!pstat->is_rtl8190_sta && !pstat->is_broadcom_sta && !pstat->is_marvell_sta && !pstat->is_intel_sta))
				set_fw_reg(priv, 0xfd004314, 0, 0);
			else
				set_fw_reg(priv, 0xfd000015, 0, 0);
		}
	} else {
		if (priv->pshare->phw->lower_tx_power) {
			//TX High power disable
//			set_fw_reg(priv, 0xfd000008, 0, 0);
			RTL_W8(0x364, RTL_R8(0x364) & ~FW_REG364_HP);
			priv->pshare->phw->lower_tx_power = 0;
		}
	}
}


void tx_power_tracking(struct rtl8192cd_priv *priv)
{
	if (priv->pmib->dot11RFEntry.ther) {
		DEBUG_INFO("TPT: triggered(every %d seconds)\n", priv->pshare->rf_ft_var.tpt_period);

		// enable rf reg 0x24 power and trigger, to get ther value in 1 second
		PHY_SetRFReg(priv, RF92CD_PATH_A, 0x24, bMask20Bits, 0x60);
		mod_timer(&priv->pshare->phw->tpt_timer, jiffies + RTL_MILISECONDS_TO_JIFFIES(1000)); // 1000ms
	}
}


void rtl8192cd_tpt_timer(unsigned long task_priv)
{
	struct rtl8192cd_priv *priv = (struct rtl8192cd_priv *)task_priv;
	unsigned int val32;

	if (!(priv->drv_state & DRV_STATE_OPEN))
		return;

	if (timer_pending(&priv->pshare->phw->tpt_timer))
		del_timer_sync(&priv->pshare->phw->tpt_timer);

	if (priv->pmib->dot11RFEntry.ther) {
		// query rf reg 0x24[4:0], for thermal meter value
		val32 = PHY_QueryRFReg(priv, RF92CD_PATH_A, 0x24, bMask20Bits, 1) & 0x01f;

		if (val32) {
			set_fw_reg(priv, 0xfd000019 | (priv->pmib->dot11RFEntry.ther & 0xff) << 8 | val32 << 16, 0, 0);
			DEBUG_INFO("TPT: finished once (ther: current=0x%02x, target=0x%02x)\n",
					   val32, priv->pmib->dot11RFEntry.ther);
		} else {
			DEBUG_WARN("TPT: cannot finish, since wrong current ther value report\n");
		}
	}
}
#endif


/*
 *
 * CAM related functions
 *
 */

/*******************************************************/
/*CAM related utility                                  */
/*CamAddOneEntry                                       */
/*CamDeleteOneEntry                                    */
/*CamResetAllEntry                                     */
/*******************************************************/
#define TOTAL_CAM_ENTRY (priv->pshare->total_cam_entry)

#define CAM_CONTENT_COUNT 8
#define CAM_CONTENT_USABLE_COUNT 6

#define CFG_VALID        BIT(15)
#define CFG_RPT_MD       BIT(8)


static UCHAR CAM_find_usable(struct rtl8192cd_priv *priv)
{
	unsigned long command = 0, content = 0;
	unsigned char index;
	int for_begin = 4;
	int round = 0;
	u4Byte retVal;


	if(GET_CHIP_VER(priv) >= VERSION_8812E) {
#if defined(MBSSID) && defined(HW_ENC_FOR_GROUP_CIPHER)
		for_begin = RTL8192CD_NUM_VWLAN + 1;
#else
		for_begin = 0;
#endif
	}

	if (IS_HAL_CHIP(priv)) {
#ifdef USE_WEP_4_KEYS
		for_begin = 0;
#endif
#if defined(MBSSID) && defined(HW_ENC_FOR_GROUP_CIPHER)
		for_begin = RTL8192CD_NUM_VWLAN + 1;
#endif
		retVal = GET_HAL_INTERFACE(priv)->CAMFindUsableHandler(priv, for_begin);
		return (unsigned char)retVal;
	} else if(CONFIG_WLAN_NOT_HAL_EXIST)
	{//not HAL
		for (index = for_begin; index < TOTAL_CAM_ENTRY; index++) {
			// polling bit, and No Write enable, and address
			command = CAM_CONTENT_COUNT * index;
			RTL_W32(CAMCMD, (SECCAM_POLL | command));

			// Check polling bit is clear
			while (1) {
				command = RTL_R32(CAMCMD);
				if (command & SECCAM_POLL)
					continue;
				else
					break;

				if (++round > 10000) {
					panic_printk("%s[%d] while (1) goes too many\n", __FUNCTION__, __LINE__);
					break;
				}
			}
			content = RTL_R32(CAMREAD);

			// check valid bit. if not valid,
			if ((content & CFG_VALID) == 0) {
				return index;
			}
		}
	}
	return TOTAL_CAM_ENTRY;
}


static void CAM_program_entry(struct rtl8192cd_priv *priv, unsigned char index, unsigned char* macad,
							  unsigned char* key128, unsigned short config)
{
	unsigned long target_command = 0, target_content = 0;
	signed char entry_i = 0;
	struct stat_info *pstat;

	if (IS_HAL_CHIP(priv)) {
		GET_HAL_INTERFACE(priv)->CAMProgramEntryHandler(
			priv,
			index,
			macad,
			key128,
			config
		);
	} else if(CONFIG_WLAN_NOT_HAL_EXIST)
	{//not HAL
		for (entry_i = (CAM_CONTENT_USABLE_COUNT-1); entry_i >= 0; entry_i--) {
			// polling bit, and write enable, and address
			target_command = entry_i + CAM_CONTENT_COUNT * index;
			target_command = target_command | SECCAM_POLL | SECCAM_WE;
			if (entry_i == 0) {
				//first 32-bit is MAC address and CFG field
				target_content = (ULONG)(*(macad + 0)) << 16
								 | (ULONG)(*(macad + 1)) << 24
								 | (ULONG)config;
				target_content = target_content | config;
			} else if (entry_i == 1) {
				//second 32-bit is MAC address
				target_content = (ULONG)(*(macad + 5)) << 24
								 | (ULONG)(*(macad + 4)) << 16
								 | (ULONG)(*(macad + 3)) << 8
								 | (ULONG)(*(macad + 2));
			} else {
				target_content = (ULONG)(*(key128 + (entry_i * 4 - 8) + 3)) << 24
								 | (ULONG)(*(key128 + (entry_i * 4 - 8) + 2)) << 16
								 | (ULONG)(*(key128 + (entry_i * 4 - 8) + 1)) << 8
								 | (ULONG)(*(key128 + (entry_i * 4 - 8) + 0));
			}

			RTL_W32(CAMWRITE, target_content);
			RTL_W32(CAMCMD, target_command);
		}

		target_content = RTL_R32(CR);
		if ((target_content & MAC_SEC_EN) == 0)
			RTL_W32(CR, (target_content | MAC_SEC_EN));
	}

	pstat = get_stainfo(priv, macad);
	if (pstat) {
		pstat->cam_id = index;
	}
// move above
#if 0
	target_content = RTL_R32(CR);
	if ((target_content & MAC_SEC_EN) == 0)
		RTL_W32(CR, (target_content | MAC_SEC_EN));
#endif
}


int CamAddOneEntry(struct rtl8192cd_priv *priv, unsigned char *pucMacAddr, unsigned long keyId,
				   unsigned long encAlg, unsigned long useDK, unsigned char *pKey)
{
	unsigned char retVal = 0, camIndex = 0, wpaContent = 0;
	unsigned short usConfig = 0;
	unsigned int set_dk_margin = 4;

	//use Hardware Polling to check the valid bit.
	//in reality it should be done by software link-list
	if ((!memcmp(pucMacAddr, "\xff\xff\xff\xff\xff\xff", 6)) || (useDK
#if defined(CONFIG_RTL_HW_WAPI_SUPPORT)
			&& ((encAlg >> 2) != DOT11_ENC_WAPI)
#endif
		)) {
#if defined(MBSSID) && defined(HW_ENC_FOR_GROUP_CIPHER)
		camIndex = priv->vap_init_seq;
#else
		camIndex = keyId;
#endif
	}
	else
		camIndex = CAM_find_usable(priv);

	if (camIndex == TOTAL_CAM_ENTRY)
		return retVal;

	usConfig = usConfig | CFG_VALID | ((USHORT)(encAlg)) | (UCHAR)keyId;

#if defined(CONFIG_RTL_HW_WAPI_SUPPORT)
	if ((encAlg >> 2) == DOT11_ENC_WAPI) {
		//ulUseDK is used to diff Parwise and Group
		//if sw crypto or enable vap, wapi driver does not place the multiple key to cam
		if((!SWCRYPTO) &&
		(!(GET_ROOT(priv)->pmib->miscEntry.vap_enable))  &&
		camIndex < 2
		)
		//if (camIndex < 2) //is group key
			usConfig |= BIT(6);

		if (useDK == 1) // ==0 sec key; == 1mic key
			usConfig |= BIT(5);

		useDK = 0;
	}
#endif

	CAM_program_entry(priv, camIndex, pucMacAddr, pKey, usConfig);

	if (priv->pshare->CamEntryOccupied == 0) {
		if (IS_HAL_CHIP(priv)) {
			SECURITY_CONFIG_OPERATION  SCO;

			GET_HAL_INTERFACE(priv)->GetHwRegHandler(priv, HW_VAR_SECURITY_CONFIG, (pu1Byte)&SCO);
			if (useDK == 1) {
				SCO |= SCO_RXUSEDK | SCO_TXUSEDK;
			}
			SCO |= SCO_RXDEC | SCO_TXENC | SCO_NOSKMC | SCO_CHK_KEYID;
			GET_HAL_INTERFACE(priv)->SetHwRegHandler(priv, HW_VAR_SECURITY_CONFIG, (pu1Byte)&SCO);
		} else if(CONFIG_WLAN_NOT_HAL_EXIST)
		{//not HAL
			if (useDK == 1)
				wpaContent = RXUSEDK | TXUSEDK;
			RTL_W16(SECCFG, RTL_R16(SECCFG) | RXDEC | TXENC | wpaContent | NOSKMC | CHK_KEYID);
		}
	}


	if(GET_CHIP_VER(priv) >= VERSION_8812E)
		set_dk_margin = 0;

#if defined(MBSSID) && defined(HW_ENC_FOR_GROUP_CIPHER)
	set_dk_margin = 0;
#endif

//for WAPI, camIndex 0 and 1 are used to store broadcast keys (crypto key and mic key)
#if defined(CONFIG_RTL_HW_WAPI_SUPPORT)
	if ((encAlg >> 2) == DOT11_ENC_WAPI)
	{
		if((!SWCRYPTO) &&
		(!(GET_ROOT(priv)->pmib->miscEntry.vap_enable))  &&
		GET_CHIP_VER(priv) >= VERSION_8812E
		)
			set_dk_margin = 2;
	}
#endif

	if (camIndex < set_dk_margin) {
		if (IS_HAL_CHIP(priv)) {
			SECURITY_CONFIG_OPERATION  SCO;
			GET_HAL_INTERFACE(priv)->GetHwRegHandler(priv, HW_VAR_SECURITY_CONFIG, (pu1Byte)&SCO);
			SCO = (SCO & ~SCO_NOSKMC) | (SCO_RXBCUSEDK | SCO_TXBCUSEDK);
			GET_HAL_INTERFACE(priv)->SetHwRegHandler(priv, HW_VAR_SECURITY_CONFIG, (pu1Byte)&SCO);
		} else if(CONFIG_WLAN_NOT_HAL_EXIST)
		{//not HAL
			RTL_W16(SECCFG, (RTL_R16(SECCFG) & ~NOSKMC) | (RXBCUSEDK | TXBCUSEDK));
		}
	}

	return 1;
}

#if(CONFIG_WLAN_NOT_HAL_EXIST==1)
void CAM_read_mac_config(struct rtl8192cd_priv *priv, unsigned char index, unsigned char* pMacad,
						 unsigned short* pTempConfig)
{
	unsigned long command = 0, content = 0;
	int round;

	// polling bit, and No Write enable, and address
	// cam address...
	// first 32-bit
	command = CAM_CONTENT_COUNT * index + 0;
	command = command | SECCAM_POLL;
	RTL_W32(CAMCMD, command);

	//Check polling bit is clear
	round = 0;
	while (1) {
		command = RTL_R32(CAMCMD);
		if (command & SECCAM_POLL)
			continue;
		else
			break;

		if (++round > 10000) {
			panic_printk("%s[%d] while (1) goes too many\n", __FUNCTION__, __LINE__);
			break;
		}
	}
	content = RTL_R32(CAMREAD);

	//first 32-bit is MAC address and CFG field
	*(pMacad + 0) = (UCHAR)((content >> 16) & 0x000000FF);
	*(pMacad + 1) = (UCHAR)((content >> 24) & 0x000000FF);
	*pTempConfig  = (USHORT)(content & 0x0000FFFF);

	command = CAM_CONTENT_COUNT * index + 1;
	command = command | SECCAM_POLL;
	RTL_W32(CAMCMD, command);

	//Check polling bit is clear
	round = 0;
	while (1) {
		command = RTL_R32(CAMCMD);
		if (command & SECCAM_POLL)
			continue;
		else
			break;

		if (++round > 10000) {
			panic_printk("%s[%d] while (1) goes too many\n", __FUNCTION__, __LINE__);
			break;
		}
	}
	content = RTL_R32(CAMREAD);

	*(pMacad + 5) = (UCHAR)((content >> 24) & 0x000000FF);
	*(pMacad + 4) = (UCHAR)((content >> 16) & 0x000000FF);
	*(pMacad + 3) = (UCHAR)((content >> 8) & 0x000000FF);
	*(pMacad + 2) = (UCHAR)((content) & 0x000000FF);
}
#else
void CAM_read_mac_config(struct rtl8192cd_priv *priv, unsigned char index, unsigned char* pMacad,
						 unsigned short* pTempConfig)
{
	return;
}
#endif//CONFIG_WLAN_NOT_HAL_EXIST


#if 0
void CAM_mark_invalid(struct rtl8192cd_priv *priv, UCHAR ucIndex)
{
	ULONG ulCommand = 0;
	ULONG ulContent = 0;

	// polling bit, and No Write enable, and address
	ulCommand = CAM_CONTENT_COUNT * ucIndex;
	ulCommand = ulCommand | _CAM_POLL_ | _CAM_WE_;
	// write content 0 is equall to mark invalid
	RTL_W32(_CAM_W_, ulContent);
	RTL_W32(_CAMCMD_, ulCommand);
}
#endif


static void CAM_empty_entry(struct rtl8192cd_priv *priv, unsigned char index)
{
	unsigned long command = 0, content = 0;
	unsigned int i;

	for (i = 0; i < CAM_CONTENT_COUNT; i++) {
		// polling bit, and No Write enable, and address
		command = CAM_CONTENT_COUNT * index + i;
		command = command | SECCAM_POLL | SECCAM_WE;
		// write content 0 is equal to mark invalid
		RTL_W32(CAMWRITE, content);
		RTL_W32(CAMCMD, command);
	}
}


int CamDeleteOneEntry(struct rtl8192cd_priv *priv, unsigned char *pMacAddr, unsigned long keyId, unsigned int useDK)
{
	unsigned char ucIndex;
	unsigned char ucTempMAC[6];
	unsigned short usTempConfig = 0;
	int for_begin = 4;

	CAM_ENTRY_CFG   CamEntryCfg;

	// group key processing
	if ((!memcmp(pMacAddr, "\xff\xff\xff\xff\xff\xff", 6)) || (useDK)) {
		if (IS_HAL_CHIP(priv)) {
#if defined(MBSSID) && defined(HW_ENC_FOR_GROUP_CIPHER)
			GET_HAL_INTERFACE(priv)->CAMReadMACConfigHandler(priv, priv->vap_init_seq, ucTempMAC, &CamEntryCfg);
#else
			GET_HAL_INTERFACE(priv)->CAMReadMACConfigHandler(priv, keyId, ucTempMAC, &CamEntryCfg);
#endif
			if ( _TRUE == CamEntryCfg.bValid ) {
#if defined(MBSSID) && defined(HW_ENC_FOR_GROUP_CIPHER)
				GET_HAL_INTERFACE(priv)->CAMEmptyEntryHandler(priv, priv->vap_init_seq);
#else
				GET_HAL_INTERFACE(priv)->CAMEmptyEntryHandler(priv, keyId);
#endif
				if (priv->pshare->CamEntryOccupied == 1) {
					SECURITY_CONFIG_OPERATION  SCO = 0;
					GET_HAL_INTERFACE(priv)->SetHwRegHandler(priv, HW_VAR_SECURITY_CONFIG, (pu1Byte)&SCO);
				}

				return 1;
			} else {
				return 0;
			}
		} else if(CONFIG_WLAN_NOT_HAL_EXIST)
		{//not HAL
#if defined(MBSSID) && defined(HW_ENC_FOR_GROUP_CIPHER)
			CAM_read_mac_config(priv, priv->vap_init_seq, ucTempMAC, &usTempConfig);
#else
			CAM_read_mac_config(priv, keyId, ucTempMAC, &usTempConfig);
#endif
			if (usTempConfig & CFG_VALID) {
#if defined(MBSSID) && defined(HW_ENC_FOR_GROUP_CIPHER)
				CAM_empty_entry(priv, priv->vap_init_seq);
#else
				CAM_empty_entry(priv, keyId);
#endif
				if (priv->pshare->CamEntryOccupied == 1)
					RTL_W16(SECCFG, 0);
				return 1;
			} else
				return 0;
		}
	}

	// TODO:    check 8881A desgin below
#ifdef USE_WEP_4_KEYS
	for_begin = 0;
#endif
#if defined(MBSSID) && defined(HW_ENC_FOR_GROUP_CIPHER)
	for_begin = RTL8192CD_NUM_VWLAN + 1;
#endif


	if(GET_CHIP_VER(priv) >= VERSION_8812E) {
#if defined(MBSSID) && defined(HW_ENC_FOR_GROUP_CIPHER)
		for_begin = RTL8192CD_NUM_VWLAN + 1;
#else
		for_begin = 0;
#endif
	}

	// unicast key processing
	// key processing for RTL818X(B) series
	for (ucIndex = for_begin; ucIndex < TOTAL_CAM_ENTRY; ucIndex++) {
		if (IS_HAL_CHIP(priv)) {
			GET_HAL_INTERFACE(priv)->CAMReadMACConfigHandler(priv, ucIndex, ucTempMAC, &CamEntryCfg);
			if (!memcmp(pMacAddr, ucTempMAC, 6)) {
#if defined(CONFIG_RTL_HW_WAPI_SUPPORT)
				if ( (CamEntryCfg.EncAlgo == DOT11_ENC_WAPI) && (CamEntryCfg.KeyID != keyId)) {
					continue;
				}
#endif
#ifdef USE_WEP_4_KEYS
				if (((CamEntryCfg.EncAlgo == DOT11_ENC_WEP40)||(CamEntryCfg.EncAlgo == DOT11_ENC_WEP104) )
					&& (CamEntryCfg.KeyID != keyId)) {
					continue;
				}
#endif

				GET_HAL_INTERFACE(priv)->CAMEmptyEntryHandler(priv, ucIndex);

				if (priv->pshare->CamEntryOccupied == 1) {
					SECURITY_CONFIG_OPERATION  SCO = 0;
					GET_HAL_INTERFACE(priv)->SetHwRegHandler(priv, HW_VAR_SECURITY_CONFIG, (pu1Byte)&SCO);
				}
				return 1;
			}
		} else if(CONFIG_WLAN_NOT_HAL_EXIST)
		{//not HAL
			CAM_read_mac_config(priv, ucIndex, ucTempMAC, &usTempConfig);
			if (!memcmp(pMacAddr, ucTempMAC, 6)) {

#if defined(CONFIG_RTL_HW_WAPI_SUPPORT)
				if ((((usTempConfig & 0x1c) >> 2) == DOT11_ENC_WAPI) && ((usTempConfig & 0x3) != keyId))
					continue;
#endif
				CAM_empty_entry(priv, ucIndex);	// reset MAC address, david+2007-1-15

				if (priv->pshare->CamEntryOccupied == 1)
					RTL_W16(SECCFG, 0);

				return 1;
			}
		}
	}
	return 0;
}


/*now use empty to fill in the first 4 entries*/
void CamResetAllEntry(struct rtl8192cd_priv *priv)
{
	if (IS_HAL_CHIP(priv)) {
		GET_HAL_INTERFACE(priv)->SetHwRegHandler(priv, HW_VAR_CAM_RESET_ALL_ENTRY, NULL);
	} else if (CONFIG_WLAN_NOT_HAL_EXIST)
	{
		unsigned char index;

		RTL_W32(CAMCMD, SECCAM_CLR);

		for (index = 0; index < TOTAL_CAM_ENTRY; index++)
			CAM_empty_entry(priv, index);

		RTL_W32(CR, RTL_R32(CR) & (~MAC_SEC_EN));
	}
	priv->pshare->CamEntryOccupied = 0;
	priv->pmib->dot11GroupKeysTable.keyInCam = 0;

//	RTL_W32(CR, RTL_R32(CR) & (~MAC_SEC_EN));
}


void CAM_read_entry(struct rtl8192cd_priv *priv, unsigned char index, unsigned char* macad,
					unsigned char* key128, unsigned short* config)
{
	unsigned long  target_command = 0, target_content = 0;
	unsigned char entry_i = 0;
	unsigned long status;
	int round = 0;

	for (entry_i = 0; entry_i < CAM_CONTENT_USABLE_COUNT; entry_i++) {
		// polling bit, and No Write enable, and address
		target_command = (unsigned long)(entry_i + CAM_CONTENT_COUNT * index);
		target_command = target_command | SECCAM_POLL;

		RTL_W32(CAMCMD, target_command);
		//Check polling bit is clear
		while (1) {
			status = RTL_R32(CAMCMD);
			if (status & SECCAM_POLL)
				continue;
			else
				break;

			if (++round > 10000) {
				panic_printk("%s[%d] while (1) goes too many\n", __FUNCTION__, __LINE__);
				break;
			}
		}
		target_content = RTL_R32(CAMREAD);

		if (entry_i == 0) {
			//first 32-bit is MAC address and CFG field
			*(config)  = (unsigned short)((target_content) & 0x0000FFFF);
			*(macad + 0) = (unsigned char)((target_content >> 16) & 0x000000FF);
			*(macad + 1) = (unsigned char)((target_content >> 24) & 0x000000FF);
		} else if (entry_i == 1) {
			*(macad + 5) = (unsigned char)((target_content >> 24) & 0x000000FF);
			*(macad + 4) = (unsigned char)((target_content >> 16) & 0x000000FF);
			*(macad + 3) = (unsigned char)((target_content >> 8) & 0x000000FF);
			*(macad + 2) = (unsigned char)((target_content) & 0x000000FF);
		} else {
			*(key128 + (entry_i * 4 - 8) + 3) = (unsigned char)((target_content >> 24) & 0x000000FF);
			*(key128 + (entry_i * 4 - 8) + 2) = (unsigned char)((target_content >> 16) & 0x000000FF);
			*(key128 + (entry_i * 4 - 8) + 1) = (unsigned char)((target_content >> 8) & 0x000000FF);
			*(key128 + (entry_i * 4 - 8) + 0) = (unsigned char)(target_content & 0x000000FF);
		}

		target_content = 0;
	}
}


#ifdef CAM_SWAP

void dump_mac3(struct rtl8192cd_priv *priv, unsigned char *mac)
{
	if(mac)
		printk(" %02x:%02x:%02x:%02x:%02x:%02x\n", mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);
}


int is_hw_encrypt(struct stat_info	*pstat)
{

	if(pstat->dot11KeyMapping.keyInCam)
		return 1;
	else
		return 0;
}

void swap_to_sw_encrypt(struct rtl8192cd_priv *priv, struct stat_info	 *pstat)
{
	int ret = 0;

	//dump_mac3(priv, pstat->hwaddr);

	ret = CamDeleteOneEntry(priv, pstat->hwaddr, pstat->dot11KeyMapping.keyid, 0);

	pstat->dot11KeyMapping.keyInCam = 0;
	priv->pshare->CamEntryOccupied--;
}

void swap_to_hw_encrypt(struct rtl8192cd_priv *priv, struct stat_info *pstat)
{
	int ret = 0;

	//dump_mac3(priv, pstat->hwaddr);

	ret = CamAddOneEntry(priv, pstat->hwaddr, pstat->dot11KeyMapping.keyid,
		pstat->dot11KeyMapping.dot11Privacy<<2, 0, GET_UNICAST_ENCRYP_KEY);

	pstat->dot11KeyMapping.keyInCam = 1;
	priv->pshare->CamEntryOccupied++;
}

struct sta_list{
	struct stat_info *pstat_array[NUM_STAT];
	unsigned char	num;
};

static struct sta_list sta_idle_hw_cam[NUM_STAT];
static struct sta_list sta_low_hw_cam[NUM_STAT];
static struct sta_list sta_mid_sw_cam[NUM_STAT];
static struct sta_list sta_high_sw_cam[NUM_STAT];

int get_sw_encrypt_sta_num(struct rtl8192cd_priv *priv)
{
	int i;
	struct stat_info *pstat = NULL;
	int num = 0;

	for(i=0; i<NUM_STAT; i++) {
		if (priv->pshare->aidarray[i] && (priv->pshare->aidarray[i]->used == TRUE)) {

			if (!((GET_MIB(priv->pshare->aidarray[i]->priv))->dot11OperationEntry.opmode & WIFI_AP_STATE))
				continue;

			pstat = &(priv->pshare->aidarray[i]->station);

			if((pstat->wpa_sta_info->RSNEnabled) && (pstat->dot11KeyMapping.keyInCam==0))
				num ++;
		}
	}

	return num;

}

void get_sta_traffic_level(struct rtl8192cd_priv *priv, struct stat_info *pstat)
{
	int total_cnt = pstat->traffic.delta_tx_bytes + pstat->traffic.delta_rx_bytes;

	//printk("%s +++ \n", __FUNCTION__);

	if(total_cnt <= TRAFFIC_THRESHOLD_LOW)
		pstat->traffic.level = STA_TRAFFIC_IDLE;
#if 0
	else if(total_cnt <= TRAFFIC_THRESHOLD_MID)
		pstat->traffic.level = STA_TRAFFIC_LOW;
	else if(total_cnt <= TRAFFIC_THRESHOLD_HIGH)
		pstat->traffic.level = STA_TRAFFIC_MID;
#endif
	else if(total_cnt > TRAFFIC_THRESHOLD_HIGH)
		pstat->traffic.level = STA_TRAFFIC_HIGH;
	else
		pstat->traffic.level = STA_TRAFFIC_DONT_CARE;

	//printk("total_cnt=%d level=%d\n", total_cnt, pstat->traffic.level);

}

void add_sta_traffic_list(struct stat_info *pstat)
{
	//printk("%s +++ \n", __FUNCTION__);
	//printk("pstat = 0x%x is_hw_encrypt=%d level=%d \n",
		//pstat, is_hw_encrypt(pstat), pstat->traffic.level);

	switch(pstat->traffic.level) {
		case STA_TRAFFIC_IDLE:
			if(is_hw_encrypt(pstat)){
				sta_idle_hw_cam->pstat_array[sta_idle_hw_cam->num] = pstat;
				sta_idle_hw_cam->num ++;
			}
			break;
#if 0
		case STA_TRAFFIC_LOW:
			if(is_hw_encrypt(pstat)){
				sta_low_hw_cam->pstat_array[sta_low_hw_cam->num] = pstat;
				sta_low_hw_cam->num ++;
			}
			break;

		case STA_TRAFFIC_MID:
			if(!is_hw_encrypt(pstat)){
				sta_mid_sw_cam->pstat_array[sta_mid_sw_cam->num] = pstat;
				sta_mid_sw_cam->num ++;
			}
			break;
#endif
		case STA_TRAFFIC_HIGH:
			if(!is_hw_encrypt(pstat)){
				sta_high_sw_cam->pstat_array[sta_high_sw_cam->num] = pstat;
				sta_high_sw_cam->num ++;
			}
			break;

	}

	//printk("cam num = %d %d %d %d \n",
		//sta_idle_hw_cam->num, sta_low_hw_cam->num,
		//sta_mid_sw_cam->num, sta_high_sw_cam->num);


}


void cal_sta_traffic(struct rtl8192cd_priv *priv, int sta_use_sw_encrypt)
{
	int i;
	struct stat_info *pstat = NULL;

	//printk("%s +++ \n", __FUNCTION__);

	sta_idle_hw_cam->num = 0;
	sta_low_hw_cam->num = 0;
	sta_mid_sw_cam->num = 0;
	sta_high_sw_cam->num = 0;

	for(i=0; i<NUM_STAT; i++) {
		if (priv->pshare->aidarray[i] && (priv->pshare->aidarray[i]->used == TRUE)) {

			if (!((GET_MIB(priv->pshare->aidarray[i]->priv))->dot11OperationEntry.opmode & WIFI_AP_STATE))
				continue;

			pstat = &(priv->pshare->aidarray[i]->station);

			if(pstat->wpa_sta_info->RSNEnabled){
				 pstat->traffic.delta_tx_bytes = pstat->tx_bytes - pstat->traffic.prev_tx_bytes;
				 pstat->traffic.delta_rx_bytes = pstat->rx_bytes - pstat->traffic.prev_rx_bytes;

				 pstat->traffic.prev_tx_bytes = pstat->tx_bytes;
				 pstat->traffic.prev_rx_bytes = pstat->rx_bytes;

				 get_sta_traffic_level(priv, pstat);

				 add_sta_traffic_list(pstat);
			}
		}
	}


}

void rotate_sta_cam(struct rtl8192cd_priv *priv, int sta_use_sw_encrypt)
{
	int i = 0;
	int num_remove_idle = 0;
	int num_remove_low = 0;

	struct stat_info *pstat_remove_hw = NULL;
	struct stat_info *pstat_move2_hw = NULL;

	//printk("%s +++ \n", __FUNCTION__);

	//printk("cam num = %d %d %d %d \n",
		//sta_idle_hw_cam->num, sta_low_hw_cam->num,
		//sta_mid_sw_cam->num, sta_high_sw_cam->num);


	for(i = 0; i<sta_high_sw_cam->num; i++){

		if(num_remove_idle < sta_idle_hw_cam->num){

			pstat_remove_hw = sta_idle_hw_cam->pstat_array[i];
			pstat_move2_hw = sta_high_sw_cam->pstat_array[i];

			swap_to_sw_encrypt(priv, pstat_remove_hw);
			swap_to_hw_encrypt(priv, pstat_move2_hw);

			num_remove_idle++;
			continue;
		}
#if 0
		if(num_remove_low < sta_low_hw_cam->num){

			pstat_remove_hw = sta_low_hw_cam->pstat_array[i];
			pstat_move2_hw = sta_high_sw_cam->pstat_array[i];

			swap_to_sw_encrypt(priv, pstat_remove_hw);
			swap_to_hw_encrypt(priv, pstat_move2_hw);

			num_remove_low++;
		}
#endif

	}

#if 0
	if(num_remove_idle < sta_idle_hw_cam->num)
	for(i = 0; i<sta_mid_sw_cam->num; i++){

 		if(num_remove_idle < sta_idle_hw_cam->num){

			pstat_remove_hw = sta_idle_hw_cam->pstat_array[i];
			pstat_move2_hw = sta_mid_sw_cam->pstat_array[i];

			swap_to_sw_encrypt(priv, pstat_remove_hw);
			swap_to_hw_encrypt(priv, pstat_move2_hw);

			num_remove_idle++;
			continue;
		}

	}
#endif

}

#endif

#if 0
void debug_cam(UCHAR*TempOutputMac, UCHAR*TempOutputKey, USHORT TempOutputCfg)
{
	printk("MAC Address\n");
	printk(" %X %X %X %X %X %X\n", *TempOutputMac
		   , *(TempOutputMac + 1)
		   , *(TempOutputMac + 2)
		   , *(TempOutputMac + 3)
		   , *(TempOutputMac + 4)
		   , *(TempOutputMac + 5));
	printk("Config:\n");
	printk(" %X\n", TempOutputCfg);

	printk("Key:\n");
	printk("%X %X %X %X,%X %X %X %X,\n%X %X %X %X,%X %X %X %X\n"
		   , *TempOutputKey, *(TempOutputKey + 1), *(TempOutputKey + 2)
		   , *(TempOutputKey + 3), *(TempOutputKey + 4), *(TempOutputKey + 5)
		   , *(TempOutputKey + 6), *(TempOutputKey + 7), *(TempOutputKey + 8)
		   , *(TempOutputKey + 9), *(TempOutputKey + 10), *(TempOutputKey + 11)
		   , *(TempOutputKey + 12), *(TempOutputKey + 13), *(TempOutputKey + 14)
		   , *(TempOutputKey + 15));
}


void CamDumpAll(struct rtl8192cd_priv *priv)
{
	UCHAR TempOutputMac[6];
	UCHAR TempOutputKey[16];
	USHORT TempOutputCfg = 0;
	unsigned long flags;
	int i;

	SAVE_INT_AND_CLI(flags);
	for (i = 0; i < TOTAL_CAM_ENTRY; i++) {
		printk("%X-", i);
		CAM_read_entry(priv, i, TempOutputMac, TempOutputKey, &TempOutputCfg);
		debug_cam(TempOutputMac, TempOutputKey, TempOutputCfg);
		printk("\n\n");
	}
	RESTORE_INT(flags);
}


void CamDump4(struct rtl8192cd_priv *priv)
{
	UCHAR TempOutputMac[6];
	UCHAR TempOutputKey[16];
	USHORT TempOutputCfg = 0;
	unsigned long flags;
	int i;

	SAVE_INT_AND_CLI(flags);
	for (i = 0; i < 4; i++) {
		printk("%X", i);
		CAM_read_entry(priv, i, TempOutputMac, TempOutputKey, &TempOutputCfg);
		debug_cam(TempOutputMac, TempOutputKey, TempOutputCfg);
		printk("\n\n");
	}
	RESTORE_INT(flags);
}
#endif

#ifdef SDIO_AP_OFFLOAD
int offloadTestFunction(struct rtl8192cd_priv *priv, unsigned char *data)
{
	int mode;
	mode = _atoi(data, 16);

	if (strlen(data) == 0) {
		printk("Usage:\n");
		printk("offload 6 - Leave offload\n");
		return 0;
	}

	if (mode == 0x6) {
		schedule_work(&GET_ROOT(priv)->ap_cmd_queue);
	}

	return 0;
}
#endif // SDIO_AP_OFFLOAD

#ifdef CONFIG_OFFLOAD_FUNCTION
int offloadTestFunction(struct rtl8192cd_priv *priv, unsigned char *data)
{
	int mode = 0, status;
    unsigned long flags;
    u32 tmp32, haddr, saddr, cnt, ctr;
    PHCI_RX_DMA_MANAGER_88XX        prx_dma;
    PHCI_RX_DMA_QUEUE_STRUCT_88XX   cur_q;
    PHCI_TX_DMA_MANAGER_88XX        ptx_dma;
    PHCI_TX_DMA_QUEUE_STRUCT_88XX   cur_q_tx;
    u4Byte RXBDReg;

	mode = _atoi(data, 16);
    haddr = 0xb8b00000; //pcie host addr
    saddr = 0xb8b10000; //pcie slave addr

	if (strlen(data) == 0) {
		printk("offloadTest 0x1 downlaod Rsvd page\n");
		printk("offloadTest 0x2: AP offload enable \n");

		return 0;
	}

	if (mode == 0x0) {
		u1Byte reg_val;
		 //clear the non-valid bit
		GET_HAL_INTERFACE(priv)->GetHwRegHandler(priv, HW_VAR_REG_CCK_CHECK, (pu1Byte)&reg_val);
		reg_val = reg_val & ~BIT(6);
		GET_HAL_INTERFACE(priv)->SetHwRegHandler(priv, HW_VAR_REG_CCK_CHECK, (pu1Byte)&reg_val);
	}

	if (mode == 0x1) {
		printk("epdn 6e: download probe rsp\n");
		priv->offload_function_ctrl = 1;
		//RTL_W16(0x100 , RTL_R16(0x100) | BIT(8));		// enable sw beacon
//		tasklet_hi_schedule(&priv->pshare->rx_tasklet);
		//RTL_W8(HWSEQ_CTRL,RTL_R8(HWSEQ_CTRL) | BIT(6)); // HW SEQ

		u4Byte u4_val;

		// enable sw beacon
		GET_HAL_INTERFACE(priv)->GetHwRegHandler(priv, HW_VAR_REG_CR, (pu4Byte)&u4_val);
		u4_val = u4_val | BIT(8);
		GET_HAL_INTERFACE(priv)->SetHwRegHandler(priv, HW_VAR_REG_CR, (pu4Byte)&u4_val);

		u1Byte u1_val;

#if (IS_RTL8192E_SERIES | IS_RTL8881A_SERIES)
		 // HW SEQ
		GET_HAL_INTERFACE(priv)->GetHwRegHandler(priv, HW_VAR_HWSEQ_CTRL, (pu1Byte)&u1_val);
		u1_val = u1_val | BIT(6);
		GET_HAL_INTERFACE(priv)->SetHwRegHandler(priv, HW_VAR_HWSEQ_CTRL, (pu1Byte)&u1_val);
#endif

	}


	if (mode == 0x2)	 {

		unsigned char      loc_bcn[1];
		unsigned char      loc_probe[1];
        unsigned short     BcnHead;
        u1Byte temp_120count=0;
        u1Byte H2CCommand[2]={0};
        u1Byte reg_val;
        u4Byte u4_val;

#ifdef GPIO_WAKEPIN
        if(IS_HARDWARE_TYPE_8822B(priv)) //for GPIO8 wakeup, test only
            RTL_W8(REG_LED_CFG+2,0x40);
#endif
        GET_HAL_INTERFACE(priv)->GetHwRegHandler(priv, HW_VAR_BCN_HEAD, (pu2Byte)&BcnHead);

		loc_bcn[0] = priv->offload_bcn_page;
		loc_probe[0] = priv->offload_proc_page;

		printk("loc_bcn[0]= %x \n", loc_bcn[0]);
		printk("loc_probe[0]= %x \n", loc_probe[0]);

#if (IS_RTL8197F_SERIES || IS_RTL8822B_SERIES)
        if ( IS_HARDWARE_TYPE_8197F(priv) || IS_HARDWARE_TYPE_8822B(priv)) { //3081

            loc_bcn[0] = (u1Byte) (priv->offload_bcn_page - BcnHead);
            loc_probe[0] = (u1Byte) (priv->offload_proc_page - BcnHead);

            printk("short loc_bcn[0]= %x \n", loc_bcn[0]);
            printk("short loc_probe[0]= %x \n", loc_probe[0]);
        }
#endif

#ifdef CONFIG_SAPPS

        H2CCommand[0] = BIT0|BIT1;
        H2CCommand[1] = 20;
        FillH2CCmd88XX(priv,0x26,2,H2CCommand);
        printk("enter SAPPS 0x1c4=0x%x\n",RTL_R8(0x1c4));
#endif
		GET_HAL_INTERFACE(priv)->SetAPOffloadHandler(priv, 1, 1, 0, 0, loc_bcn, loc_probe);

        temp_120count=0;
        while( !(RTL_R8(0x284+2)&BIT2)){//tingchu 20161006 only check 0x284 bit18
			if(temp_120count > 100){
				printk("enable apoffload error, 0x120=0x%x, 0x284=0x%x, 0xc0=0x%x\n",RTL_R32(0x120),RTL_R32(0x284),RTL_R32(0xc0));
                printk("enable apoffload error\n");

                return FAIL;
				//break;
			}
			printk("wait apoffload enable\n");
            printk("0x1a0=0x%x,0x1ac=0x%x\n",RTL_R32(0x1a0),RTL_R32(0x1ac));
            delay_us(100);
            temp_120count++;

        }

        printk("apoffload enable success 0x1a0=0x%x,0x1ac=0x%x\n",RTL_R32(0x1a0),RTL_R32(0x1ac));

#ifdef CONFIG_32K
        RTL_W8(0x3d9,(RTL_R8(0x3d9)|BIT0)^BIT7);
        printk("enable 32k, 3d9h=0x%x, 0x1ac=0x%x\n",RTL_R8(0x3d9),RTL_R8(0x1ac));
        priv->offload_32k_flag = 1;
#endif
		delay_ms(10);
	}

	if (mode == 0x3) {
		GET_HAL_INTERFACE(priv)->SetMACIDSleepHandler(priv, 1 , 1);
	}

	if (mode == 0x4) {
		GET_HAL_INTERFACE(priv)->SetMACIDSleepHandler(priv, 0 , 1);
	}

	if (mode == 0x05){
#ifdef PCIE_PME_WAKEUP_TEST//yllin
        //setting PCIE
        u32 PME_enable;
        PME_enable = REG32(0xb8b0008c) | 0x8;
        REG32(0xb8b0008c) = PME_enable;

        PME_enable = REG32(0xb8b10044) | 0x100;
        REG32(0xb8b10044) = PME_enable;
#endif
    }

    if (mode == 0x05){
#ifdef CONFIG_32K
        priv->offload_32k_flag = 0;
        RTL_W8(0x3d9,(RTL_R8(0x3d9)&~BIT0)^BIT7);
        printk("disable 32k\n");

#endif
    }



	return 0;
}
#endif //#ifdef CONFIG_OFFLOAD_FUNCTION

#ifdef GPIO_WAKEPIN
irqreturn_t gpio_wakeup_isr_V2(int irq, void *dev_instance);
int request_irq_for_wakeup_pin_V2(struct net_device *dev)
{
	struct rtl8192cd_priv *priv = GET_DEV_PRIV(dev);
	unsigned int saddr;
    //test, setting GPIO H1
    gpio_request(BSP_GPIO_PIN_H1, "WLAN Sleep INT");
    gpio_direction_input(BSP_GPIO_PIN_H1);

    int h1irq = gpio_to_irq(BSP_GPIO_PIN_H1);
    printk("GOIP H1 IRQ num=%d\n",h1irq);
    return request_irq(h1irq, gpio_wakeup_isr_V2, IRQF_SHARED |  IRQF_TRIGGER_FALLING, "rtk_gpio", dev);


}
#endif

#ifdef GPIO_WAKEPIN
irqreturn_t gpio_wakeup_isr_V2(int irq, void *dev_instance)
{
	struct net_device *dev = NULL;
	struct rtl8192cd_priv *priv = NULL;
    u1Byte reg_val;
    u4Byte u4_val;


    PHCI_RX_DMA_MANAGER_88XX        prx_dma;
    PHCI_RX_DMA_QUEUE_STRUCT_88XX   cur_q;
    PHCI_TX_DMA_MANAGER_88XX        ptx_dma;
    PHCI_TX_DMA_QUEUE_STRUCT_88XX   cur_q_tx;
    unsigned long flags;
    u4Byte                      RXBDReg;

    u32 tmp32, haddr, saddr, cnt, ctr;
    int status;
    haddr = 0xb8b00000;
    saddr = 0xb8b10000;


	dev = (struct net_device *)dev_instance;
	priv = GET_DEV_PRIV(dev);

	printk("%s, PEFGH_ISR=%x\t", dev->name, REG32(PEFGH_ISR));

//yllin test
    REG32(PEFGH_ISR) = BIT(25); // clear int status

    //stop apofflaod
    GET_HAL_INTERFACE(priv)->SetAPOffloadHandler(priv, 0, 1, 0, 0, NULL, NULL);
    //restore setting
    GET_HAL_INTERFACE(priv)->GetHwRegHandler(priv, HW_VAR_REG_CCK_CHECK, (pu1Byte)&reg_val);
	reg_val = reg_val | BIT(6);
	GET_HAL_INTERFACE(priv)->SetHwRegHandler(priv, HW_VAR_REG_CCK_CHECK, (pu1Byte)&reg_val);

    GET_HAL_INTERFACE(priv)->GetHwRegHandler(priv, HW_VAR_REG_CR, (pu4Byte)&u4_val);
	u4_val = u4_val & ~BIT(8);
	GET_HAL_INTERFACE(priv)->SetHwRegHandler(priv, HW_VAR_REG_CR, (pu4Byte)&u4_val);
    //printk("0x284=%x\n",RTL_R32(0x284));
    GET_HAL_INTERFACE(priv)->GetHwRegHandler(priv, HW_VAR_RXPKT_NUM, (pu4Byte)&u4_val); //release RXDMA
	u4_val = u4_val & ~BIT(18);
	GET_HAL_INTERFACE(priv)->SetHwRegHandler(priv, HW_VAR_RXPKT_NUM, (pu4Byte)&u4_val);

	return IRQ_HANDLED;
}
#define CLK_MANAGE	0xb8000010

void PCIE_reset_procedure3_V2(struct rtl8192cd_priv *priv)
{
	//PCIE Register
	unsigned int PCIE_PHY0_REG, PCIE_PHY0, linkstatus, haddr;
	int status = 0, counter1 = 0, counter2 = 0;

	haddr = 0xb8b00000;

	PCIE_PHY0_REG  = haddr + 0x1000;
	PCIE_PHY0 = haddr + 0x1008;
	linkstatus = haddr + 0x728;

	do {
		// PCIE PHY Reset
		REG32(PCIE_PHY0) = 0x1; //bit7 PHY reset=0   bit0 Enable LTSSM=1
		delay_ms(2);

		REG32(0xb8000050) &= ~(1 << 1);  //perst=0 off.
		delay_ms(10);

		REG32(PCIE_PHY0) = 0x81;   //bit7 PHY reset=1   bit0 Enable LTSSM=1
		delay_ms(5);


		//---------------------------------------
		// PCIE Device Reset

		delay_ms(50);
		REG32(0xb8000050) |=  (1 << 1); //PERST=1
		delay_ms(10);

		counter1 = 20;
		while(--counter1)
		{
			status = REG32(linkstatus) & 0x1f;
			if ( status == 0x11 )
			{
				break;
			}
			delay_ms(50);
		}

		if (counter1 == 0)
		{
			if ( ++counter2 > 10) {
				panic_printk("PCIe reset fail!!!!\n");
				break;
			}
		}else
		{
			break;
		}
	} while (1);

//	printk("PCIE_reset_procedure3\t devid=%x\n",REG32(0xb8b10000));

}
void setBaseAddressRegister_V2(void)
{
	int tmp32 = 0, status;
	while (++tmp32 < 100) {
		REG32(0xb8b00004) = 0x00100007;
		REG32(0xb8b10004) = 0x00100007;
		REG32(0xb8b10010) = 0x18c00001;
		REG32(0xb8b10018) = 0x19000004;
		status = (REG32(0xb8b10010) ^ 0x18c00001) | ( REG32(0xb8b10018) ^ 0x19000004);
		if (!status)
			break;
		else {
			printk("set BAR fail,%x\n", status);
			printk("%x %x %x %x \n",
				   REG32(0xb8b00004) , REG32(0xb8b10004) , REG32(0xb8b10010),  REG32(0xb8b10018) );
		}
	} ;
}

#endif


/*
 *
 * Power Saving related functions
 *
 */


#if defined(RF_MIMO_SWITCH) || defined(PCIE_POWER_SAVING)

int MIMO_Mode_1Plus1(struct rtl8192cd_priv *priv)
{
	//panic_printk("switch to 1x1\n");
	return 1;
}

int MIMO_Mode_2Plus2(struct rtl8192cd_priv *priv)
{
	//panic_printk("switch to 2x2\n");

	if(GET_CHIP_VER(priv) == VERSION_8814A) {
		priv->pshare->rf_phy_bb_backup[0] = RTL_R32(0x808);
		priv->pshare->rf_phy_bb_backup[1] = RTL_R8(0x93e);
		priv->pshare->rf_phy_bb_backup[2] = RTL_R8(0x93f);

		RTL_W8(0x808, 0x66);
		RTL_W8(0x93e, 0x2c);
		RTL_W8(0x93f, 0x00);
	}
	return 1;
}

int MIMO_Mode_3Plus3(struct rtl8192cd_priv *priv)
{
	//panic_printk("switch to 3x3\n");
	if(GET_CHIP_VER(priv) == VERSION_8814A) {
			PHY_SetBBReg(priv, 0x808, bMaskDWord, priv->pshare->rf_phy_bb_backup[0]);
			PHY_SetBBReg(priv, 0x93c, 0xff0000, priv->pshare->rf_phy_bb_backup[1]);
			PHY_SetBBReg(priv, 0x93c, bMaskByte3, priv->pshare->rf_phy_bb_backup[2]);
	}
	return 1;
}

int MIMO_Mode_Switch(struct rtl8192cd_priv *priv, int mode)
{
	unsigned int retval = 0;
#ifdef MP_TEST
	if (((OPMODE & WIFI_MP_STATE) || priv->pshare->rf_ft_var.mp_specific))
		return 0;
#endif

	switch(mode) {
		case MIMO_1T1R:
				retval = MIMO_Mode_1Plus1(priv);
				priv->pshare->rf_status= MIMO_1T1R;
				break;
		case MIMO_2T2R:
				retval = MIMO_Mode_2Plus2(priv);
				priv->pshare->rf_status= MIMO_2T2R;
				break;
		case MIMO_3T3R:
				retval = MIMO_Mode_3Plus3(priv);
				priv->pshare->rf_status= MIMO_3T3R;
				break;
		default:
			break;
	}
	return retval;
}

void set_MIMO_Mode(struct rtl8192cd_priv *priv, unsigned pref_mode)
{
	 if(MIMO_Mode_Switch(priv, pref_mode)) {
		if(pref_mode == MIMO_2T2R && get_rf_mimo_mode(priv) == MIMO_2T2R) {
		#if defined(WIFI_11N_2040_COEXIST_EXT)
			if((priv->pshare->CurrentChannelBW == HT_CHANNEL_WIDTH_20) && (priv->pmib->dot11nConfigEntry.dot11nUse40M != HT_CHANNEL_WIDTH_20)) {
				priv->pshare->CurrentChannelBW = priv->pmib->dot11nConfigEntry.dot11nUse40M;
				SwBWMode(priv, priv->pshare->CurrentChannelBW, priv->pshare->offset_2nd_chan);
				SwChnl(priv, priv->pmib->dot11RFEntry.dot11channel, priv->pshare->offset_2nd_chan);
				update_RAMask_to_FW(priv, 1);
				SetTxPowerLevel(priv, priv->pmib->dot11RFEntry.dot11channel);
			}
		#endif
		}
	 }else
	 	panic_printk("(%s)line=%d, No such Mode. \n", __FUNCTION__, __LINE__);

}

#endif


void ReadEFuseByte(struct rtl8192cd_priv *priv, unsigned short _offset, unsigned char *pbuf)
{
	unsigned int   	value32;
	unsigned char 	readbyte;
	unsigned short 	retry;


	//Write Address
	RTL_W8(EFUSE_CTRL + 1, (_offset & 0xff));
	readbyte = RTL_R8(EFUSE_CTRL + 2);
	RTL_W8(EFUSE_CTRL + 2, ((_offset >> 8) & 0x03) | (readbyte & 0xfc));

	//Write bit 32 0
	readbyte = RTL_R8(EFUSE_CTRL + 3);
	RTL_W8(EFUSE_CTRL + 3, 0x72); //read cmd
	//RTL_W8(EFUSE_CTRL+3, (readbyte & 0x7f));

	//Check bit 32 read-ready
	retry = 0;
	value32 = RTL_R32(EFUSE_CTRL);

	while (!(((value32 >> 24) & 0xff) & 0x80) && (retry < 10000)) {
		value32 = RTL_R32(EFUSE_CTRL);
		retry++;
	}

	// 20100205 Joseph: Add delay suggested by SD1 Victor.
	// This fix the problem that Efuse read error in high temperature condition.
	// Designer says that there shall be some delay after ready bit is set, or the
	// result will always stay on last data we read.
	delay_us(50);
	value32 = RTL_R32(EFUSE_CTRL);

	*pbuf = (unsigned char)(value32 & 0xff);
}	/* ReadEFuseByte */

#if defined(CONFIG_SDIO_HCI) || defined(POWER_TRIM)
#define RF_GAIN_OFFSET_ON       BIT4
#define REG_RF_BB_GAIN          0x55
#define RF_GAIN_OFFSET_MASK     0xfffff
#define EFUSE_GAIN_FLAG_OFFSET  0xC1

/*
  @action=
  	0: set RF gain offset from efuse
  	1: restore original RF gain offset from backup
*/
void power_trim(struct rtl8192cd_priv *priv, unsigned char action)
{
	unsigned short efuse_gain_offset;
	u8 val=0xff, valb;
	u8 patha_4bit, pathb_4bit;
	u32 res, resb;
	char tmp[2];
	int i;

#if 0
	// if we want to write a physical address,
	 efuse_PowerSwitch(priv, TRUE, TRUE);
	 WriteEFuseByte(priv, EFUSE_GAIN_OFFSET, 0x60);
	 efuse_PowerSwitch(priv, TRUE, FALSE);
#endif

	if(GET_CHIP_VER(priv) != VERSION_8197F){
		return;
	}


	if(GET_CHIP_VER(priv) == VERSION_8192E){
		efuse_gain_offset = 0x1F6;
	}
	else if(GET_CHIP_VER(priv) == VERSION_8197F){
		efuse_gain_offset = 0x51;
	}
	else{
		efuse_gain_offset = 0xF6;
	}

	/*set RF gain offset from efuse*/
	if(action == 0){
		if(GET_CHIP_VER(priv) == VERSION_8197F){
			load_efuse_data_to_reg();
			priv->pshare->kfree_value = val = read_efuse_byte(efuse_gain_offset); /*0x51 = 81*/
			panic_printk("efuse 0x51=%x\n", val);
		}else
		{
			ReadEFuseByte(priv, efuse_gain_offset, &val);
		}

		valb = ((val & 0xF0) >> 4);
		panic_printk("val & 0xF= %x\n",val & 0xF);

		/* path A is calibrated with K-free*/
		if ( (val & 0xF) == 0xF ) {
			panic_printk("path A no calibrated k-free value!\n");
		}else{
			res = PHY_QueryRFReg(priv, RF92CD_PATH_A, REG_RF_BB_GAIN, bMask20Bits, 1);
			priv->pshare->kfree_reg_backup_pathA = res;
			panic_printk("orig RF A 0x55 = 0x%x\n",res);
			if(GET_CHIP_VER(priv) == VERSION_8197F){
				priv->pshare->kfree_reg_0x65_pathA = PHY_QueryRFReg(priv, RF92CD_PATH_A, 0x65, BIT17, 1);
				PHY_SetRFReg(priv, RF92CD_PATH_A, 0x65, BIT17, 0);
				/* set RF 0x55[19, 16:14] <-- efuse 0x51[0, 3:1]
				     set RF 0x55[6:5] <--2b'11  */
				res &= 0xFFF63F9F;  /* backup 0x55[19, 16:14, 6:5]*/
				res |= ((val&0xF) & BIT0) << 19;
				res |= (((val&0xF) & (BIT3|BIT2|BIT1))>>1) << 14;
				res |= 3 << 5;
				panic_printk("res=%x\n",res);
			}else
			{
				/* set 0x55[18:15]*/
				res &= 0xfff87fff;
				res |= (val & 0x0f)<< 15;
			}
			PHY_SetRFReg(priv, RF92CD_PATH_A, REG_RF_BB_GAIN, bMask20Bits, res);
			val = PHY_QueryRFReg(priv, RF92CD_PATH_A, REG_RF_BB_GAIN, bMask20Bits, 1);
			panic_printk("write RF A offset 0x%02x val [0x%05x],  read back [0x%05x]\n",
				REG_RF_BB_GAIN, res&0xfffff, val&0xfffff);
		}
		/* path B is calibrated with K-free*/
		if ( valb == 0xF ) {
			panic_printk("path B no calibrated k-free value!\n");
		}else{
			res = PHY_QueryRFReg(priv, RF92CD_PATH_B, REG_RF_BB_GAIN, bMask20Bits, 1);
			priv->pshare->kfree_reg_backup_pathB = res;
			panic_printk("orig RF B 0x55 = 0x%x\n",res);
			if(GET_CHIP_VER(priv) == VERSION_8197F){
				priv->pshare->kfree_reg_0x65_pathB = PHY_QueryRFReg(priv, RF92CD_PATH_B, 0x65, BIT17, 1);
				PHY_SetRFReg(priv, RF92CD_PATH_B, 0x65, BIT17, 0);
				/* set RF 0x55[19, 16:14] <-- efuse 0x51[0, 3:1]
				     set RF 0x55[6:5] <--2b'11  */
				res &= 0xFFF63F9F;  /* backup 0x55[19, 16:14, 6:5]*/
				res |= ((val&0xF) & BIT0) << 19;
				res |= (((val&0xF) & (BIT3|BIT2|BIT1))>>1) << 14;
				res |= 3 << 5;
				panic_printk("res=%x\n",res);
			}else
			{
				/* set 0x55[18:15]*/
				res &= 0xfff87fff;
				res |= (val & 0x0f) << 15;
			}
			PHY_SetRFReg(priv, RF92CD_PATH_B, REG_RF_BB_GAIN, bMask20Bits, res);
			val = PHY_QueryRFReg(priv, RF92CD_PATH_B, REG_RF_BB_GAIN, bMask20Bits, 1);
			panic_printk("write RF B offset 0x%02x val [0x%05x],  read back [0x%05x]\n",
				REG_RF_BB_GAIN, res&0xfffff, val&0xfffff);
		}
	}/*restore original RF gain offset from backup*/
	else if(action == 1){
		/*Restore path A 0x55*/
		if(priv->pshare->kfree_reg_backup_pathA){
			panic_printk("orig RF A 0x55 = 0x%x\n", priv->pshare->kfree_reg_backup_pathA);
			res = PHY_QueryRFReg(priv, RF92CD_PATH_A, REG_RF_BB_GAIN, bMask20Bits, 1);
			if(GET_CHIP_VER(priv) == VERSION_8197F){
				PHY_SetRFReg(priv, RF92CD_PATH_A, 0x65, BIT17, priv->pshare->kfree_reg_0x65_pathA&BIT0);
				/* RF 0x55[19, 16:14]*/
				res &= 0xFFF63F9F;
				res |= (priv->pshare->kfree_reg_backup_pathA & (BIT19|BIT16|BIT15|BIT14|BIT6|BIT5));
			}else
			{
				/* RF 0x55[18:15]*/
				res &= 0xfff87fff;
				res |= (priv->pshare->kfree_reg_backup_pathA & (BIT18|BIT17|BIT16|BIT15));
			}
			PHY_SetRFReg(priv, RF92CD_PATH_A, 0x55, bMask20Bits, res);
		}else{
			panic_printk("path A didn't apply k-free\n");
		}

		/*Restore path B 0x55*/
		if(priv->pshare->kfree_reg_backup_pathB){
			panic_printk("orig RF B 0x55 = 0x%x\n", priv->pshare->kfree_reg_backup_pathB);
			res = PHY_QueryRFReg(priv, RF92CD_PATH_B, REG_RF_BB_GAIN, bMask20Bits, 1);
			if(GET_CHIP_VER(priv) == VERSION_8197F){
				PHY_SetRFReg(priv, RF92CD_PATH_B, 0x65, BIT17, priv->pshare->kfree_reg_0x65_pathB&BIT0);
				/* RF 0x55[19, 16:14]*/
				res &= 0xFFF63F9F;
				res |= (priv->pshare->kfree_reg_backup_pathB & (BIT19|BIT16|BIT15|BIT14|BIT6|BIT5));
			}else
			{
				/* RF 0x55[18:15]*/
				res &= 0xfff87fff;
				res |= (priv->pshare->kfree_reg_backup_pathB & (BIT18|BIT17|BIT16|BIT15));
			}
			PHY_SetRFReg(priv, RF92CD_PATH_B, 0x55, bMask20Bits, res);
		}else{
			panic_printk("path B didn't apply k-free\n");
		}
	}else{
		panic_printk("Error: Not support action %d for kfree()\n", action);
	}
}
#endif

#ifdef POWER_TRIM
void do_kfree(struct rtl8192cd_priv *priv, unsigned char *data)
{
	char *value;
	u8 val,valb;
	u32 res,resb;

	if (!netif_running(priv->dev)) {
		panic_printk("\nFail: interface not opened\n");
		return;
	}

	if (IS_D_CUT_8192E(priv) == FALSE && !GET_CHIP_VER(priv) == VERSION_8197F) {
		panic_printk("Fail: %s() only support 97F/ 92E D-cut !\n", __FUNCTION__);
		return;
	}

	value = get_value_by_token((char *)data, "on");
	if (value) {
		if (priv->pmib->dot11RFEntry.kfree_enable)
			return;

		power_trim(priv, 0);
		priv->pmib->dot11RFEntry.kfree_enable = 1;
		panic_printk("kfree on\n");
	}

	value = get_value_by_token((char *)data, "off");
	if (value) {
		if (priv->pmib->dot11RFEntry.kfree_enable == 0)
			return;
		power_trim(priv, 1);
		priv->pmib->dot11RFEntry.kfree_enable = 0;
		panic_printk("kfree off\n");
	}
}
#endif

#ifdef THER_TRIM
/*
*  load thermal trim value from efuse
*/
void ther_trim_efuse(struct rtl8192cd_priv *priv){
	unsigned short efuse_gain_offset;
	u8 val=0xff;

	if(GET_CHIP_VER(priv) != VERSION_8197F){
		return;
	}

	if(GET_CHIP_VER(priv) == VERSION_8197F){
		efuse_gain_offset = 0x50; /* efuse thermal trim offset*/
		load_efuse_data_to_reg();
		val = read_efuse_byte(efuse_gain_offset); /*0x51 = 81*/

	/* fake value */
//	val = 14;

		phy_printk("efuse 0x50=%x\n", val);
		if(val == 0xFF){
			phy_printk("efuse 0xEF=0xFF, not to apply thermal trim\n");
			priv->pshare->rf_ft_var.ther_trim_val = 0;
			return;
		}else{
			if(val & 0x1){ /* positive */
				priv->pshare->rf_ft_var.ther_trim_val = val>>1;
				phy_printk(" positive ther_trim_val=%d\n", priv->pshare->rf_ft_var.ther_trim_val);
			}else{ /* negative */
				priv->pshare->rf_ft_var.ther_trim_val = -(val>>1);
				phy_printk(" negative ther_trim_val=%d\n", priv->pshare->rf_ft_var.ther_trim_val);
			}
			if(priv->pshare->rf_ft_var.ther_trim_val < -12 || priv->pshare->rf_ft_var.ther_trim_val > 12){
				priv->pshare->rf_ft_var.ther_trim_val = 0;
				phy_printk(" unreasonable value, fineal ther_trim_val=%d\n", priv->pshare->rf_ft_var.ther_trim_val);
			}
		}
	}
}

/*
* apply or un-do thermal trim value to mib ther
*/
void ther_trim_act(struct rtl8192cd_priv *priv, unsigned char action){
	int val;

	if(GET_CHIP_VER(priv) != VERSION_8197F)
		return;

	if(priv->pshare->rf_ft_var.ther_trim_val < -12 || priv->pshare->rf_ft_var.ther_trim_val > 12)
		return;

	/*set apply thermal trim*/
	if(action == 0){
		val = priv->pmib->dot11RFEntry.ther + priv->pshare->rf_ft_var.ther_trim_val;
		GDEBUG("Apply thermal trim\n");
	}else{ /*undo thermal trim*/
		val = priv->pmib->dot11RFEntry.ther - priv->pshare->rf_ft_var.ther_trim_val;
		phy_printk("Undo thermal trim\n");
	}

	if(val < 0)
		priv->pmib->dot11RFEntry.ther = 0;
	else
		priv->pmib->dot11RFEntry.ther = val;
	phy_printk("ther_trim_val=%d, final ther=%u\n", priv->pshare->rf_ft_var.ther_trim_val, priv->pmib->dot11RFEntry.ther);
}
#endif

// for 8822B PCIE D-cut patch only
// Normal driver and MP driver need this patch
//
void _setTxACaliValue(
	struct rtl8192cd_priv *priv,
	IN	u1Byte	 eRFPath,
	IN	u1Byte	 offset,
	IN	u1Byte	 TxABiaOffset
	)
{
	u4Byte modiTxAValue = 0;
	u1Byte tmp1Byte = 0;
	BOOLEAN bMinus = FALSE;
	u1Byte compValue = 0;

	switch(offset)
	{
		case 0x0:
			PHY_SetRFReg(priv, eRFPath, 0x18, 0xFFFFF, 0X10124);
			break;
		case 0x1:
			PHY_SetRFReg(priv, eRFPath, 0x18, 0xFFFFF, 0X10524);
			break;
		case 0x2:
			PHY_SetRFReg(priv, eRFPath, 0x18, 0xFFFFF, 0X10924);
			break;
		case 0x3:
			PHY_SetRFReg(priv, eRFPath, 0x18, 0xFFFFF, 0X10D24);
			break;
		case 0x4:
			PHY_SetRFReg(priv, eRFPath, 0x18, 0xFFFFF, 0X30164);
			break;
		case 0x5:
			PHY_SetRFReg(priv, eRFPath, 0x18, 0xFFFFF, 0X30564);
			break;
		case 0x6:
			PHY_SetRFReg(priv, eRFPath, 0x18, 0xFFFFF, 0X30964);
			break;
		case 0x7:
			PHY_SetRFReg(priv, eRFPath, 0x18, 0xFFFFF, 0X30D64);
			break;
		case 0x8:
			PHY_SetRFReg(priv, eRFPath, 0x18, 0xFFFFF, 0X50195);
			break;
		case 0x9:
			PHY_SetRFReg(priv, eRFPath, 0x18, 0xFFFFF, 0X50595);
			break;
		case 0xa:
			PHY_SetRFReg(priv, eRFPath, 0x18, 0xFFFFF, 0X50995);
			break;
		case 0xb:
			PHY_SetRFReg(priv, eRFPath, 0x18, 0xFFFFF, 0X50D95);
			break;
		default:
			panic_printk("Invalid TxA band offset...\n");
			return;
			break;
	}

	// Get TxA value
	modiTxAValue = PHY_QueryRFReg(priv, eRFPath, 0x61, 0xFFFFF,1);
	tmp1Byte = (u1Byte)modiTxAValue&(BIT3|BIT2|BIT1|BIT0);

	// check how much need to calibration
	switch(TxABiaOffset)
	{
		case 0xF8:
			bMinus = TRUE;
			compValue = 4;
			break;

		case 0xF6:
			bMinus = TRUE;
			compValue = 3;
			break;

		case 0xF4:
			bMinus = TRUE;
			compValue = 2;
			break;

		case 0xF2:
			bMinus = TRUE;
			compValue = 1;
			break;

		case 0xF3:
			bMinus = FALSE;
			compValue = 1;
			break;

		case 0xF5:
			bMinus = FALSE;
			compValue = 2;
			break;

		case 0xF7:
			bMinus = FALSE;
			compValue = 3;
			break;

		case 0xF9:
			bMinus = FALSE;
			compValue = 4;
			break;

		// do nothing case
		case 0xF0:
		default:
			return;
			break;
	}

	// calc correct value to calibrate
	if(bMinus)
	{
		if(tmp1Byte >= compValue)
		{
			tmp1Byte -= compValue;
			//modiTxAValue += tmp1Byte;
		}
		else
		{
			tmp1Byte = 0;
		}
	}
	else
	{
		tmp1Byte += compValue;
		if(tmp1Byte >= 7)
		{
			tmp1Byte = 7;
		}
	}

	// Write back to RF reg
	PHY_SetRFReg(priv, eRFPath, 0x30, 0xFFFF, (offset<<12|(modiTxAValue&0xFF0)|tmp1Byte));
}

void _txaBiasCali4eachPath(
	struct rtl8192cd_priv *priv,
	IN	u1Byte	 eRFPath,
	IN	u1Byte	 efuseValue
	)
{
	// switch on set TxA bias
	PHY_SetRFReg(priv, eRFPath, 0xEF, 0xFFFFF, 0x200);

	// Set 12 sets of TxA value
	_setTxACaliValue(priv, eRFPath, 0x0, efuseValue);
	_setTxACaliValue(priv, eRFPath, 0x1, efuseValue);
	_setTxACaliValue(priv, eRFPath, 0x2, efuseValue);
	_setTxACaliValue(priv, eRFPath, 0x3, efuseValue);
	_setTxACaliValue(priv, eRFPath, 0x4, efuseValue);
	_setTxACaliValue(priv, eRFPath, 0x5, efuseValue);
	_setTxACaliValue(priv, eRFPath, 0x6, efuseValue);
	_setTxACaliValue(priv, eRFPath, 0x7, efuseValue);
	_setTxACaliValue(priv, eRFPath, 0x8, efuseValue);
	_setTxACaliValue(priv, eRFPath, 0x9, efuseValue);
	_setTxACaliValue(priv, eRFPath, 0xa, efuseValue);
	_setTxACaliValue(priv, eRFPath, 0xb, efuseValue);

	// switch off set TxA bias
	PHY_SetRFReg(priv, eRFPath, 0xEF, 0xFFFFF, 0x0);
}

void TxACurrentCalibration(struct rtl8192cd_priv *priv)
{
	//HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(priv);
	//PEFUSE_HAL		pEfuseHal = &(pHalData->EfuseHal);
	//u1Byte 			eFuseContent[DCMD_EFUSE_MAX_SECTION_NUM * EFUSE_MAX_WORD_UNIT * 2];
	u1Byte			efuse0x3D8, efuse0x3D7;
	u4Byte			origRF0x18PathA = 0, origRF0x18PathB = 0;
	u8 val=0xff;
	// save original 0x18 value
	origRF0x18PathA = PHY_QueryRFReg(priv, ODM_RF_PATH_A, 0x18, 0xFFFFF,1);
	origRF0x18PathB = PHY_QueryRFReg(priv, ODM_RF_PATH_B, 0x18, 0xFFFFF,1);


#if 0
	// if we want to write a physical address,
	 efuse_PowerSwitch(priv, TRUE, TRUE);
	 WriteEFuseByte(priv, 0x3D7, 0xF6);
	 WriteEFuseByte(priv, 0x3D8, 0xF9);
	 efuse_PowerSwitch(priv, TRUE, FALSE);
#endif
	// Step 1 : read efuse content
#if 0
	GET_MACHALAPI_INTERFACE(priv)->halmac_dump_efuse_map(priv->pHalmac_adapter,1);
	efuse0x3D7=*(priv->pHalmac_adapter->pHalEfuse_map+0x3D7);
	efuse0x3D8=*(priv->pHalmac_adapter->pHalEfuse_map+0x3D8);

	panic_printk("********************************\n");
	panic_printk("8822 efuse content 0x3D7 = 0x%x\n",efuse0x3D7);
	panic_printk("8822 efuse content 0x3D8 = 0x%x\n",efuse0x3D8);
	panic_printk("********************************\n");
#endif
#if 1
	ReadEFuseByte(priv, 0x3D7, &efuse0x3D7);
	panic_printk("efuse content 0x3D7 = 0x%x\n",efuse0x3D7);
	ReadEFuseByte(priv, 0x3D8, &efuse0x3D8);
	panic_printk("efuse content 0x3D8 = 0x%x\n",efuse0x3D8);
#endif
	// Step 2 : check efuse content to judge whether need to calibration or not
	if(efuse0x3D7 == 0xFF)
	{
		panic_printk("efuse content 0x3D7 == 0xFF, No need to do TxA cali\n");
		return;
	}

	// Step 3 : write RF register for calibration
	_txaBiasCali4eachPath(priv, ODM_RF_PATH_A, efuse0x3D7);
	_txaBiasCali4eachPath(priv, ODM_RF_PATH_B, efuse0x3D8);


	// restore original 0x18 value
	PHY_SetRFReg(priv, ODM_RF_PATH_A, 0x18, 0xFFFFF, origRF0x18PathA);
	PHY_SetRFReg(priv, ODM_RF_PATH_B, 0x18, 0xFFFFF, origRF0x18PathB);
}



#ifdef TX_EARLY_MODE
void enable_em(struct rtl8192cd_priv *priv)
{
	{
		RTL_W32(EARLY_MODE_CTRL, RTL_R32(EARLY_MODE_CTRL) | 0x8000000f); // enable signel AMPDU, early mode for vi/vo/be/bk queue
	}

	RTL_W16(TCR, RTL_R16(TCR) | WMAC_TCR_ERRSTEN2);
}

void disable_em(struct rtl8192cd_priv *priv)
{
	{
		RTL_W32(EARLY_MODE_CTRL, RTL_R32(EARLY_MODE_CTRL) & ~0x8000000f); // disable signel AMPDU, early mode for vi/vo/be/bk queue
	}

	RTL_W16(TCR, RTL_R16(TCR) & ~WMAC_TCR_ERRSTEN2 );
}
#endif



#ifdef RTLWIFINIC_GPIO_CONTROL


void reg_bit_set(struct rtl8192cd_priv *priv, unsigned int reg, unsigned int value)
{
	unsigned int tmp_value = 0;

	if(!(reg%4)){
		tmp_value = RTL_R32(reg);
		tmp_value |= value;
		RTL_W32(reg, tmp_value);
	}
	else if(!(reg%2)){
		tmp_value = RTL_R16(reg);
		tmp_value |= value;
		RTL_W16(reg, tmp_value);
	}
	else {
		tmp_value = RTL_R8(reg);
		tmp_value |= value;
		RTL_W8(reg, tmp_value);
	}
}

void reg_bit_clear(struct rtl8192cd_priv *priv, unsigned int reg, unsigned int value)
{
	unsigned int tmp_value = 0;

	if(!(reg%4)){
		tmp_value = RTL_R32(reg);
		tmp_value &= (~value);
		RTL_W32(reg, tmp_value);
	}
	else if(!(reg%2)){
		tmp_value = RTL_R16(reg);
		tmp_value &= (~value);
		RTL_W16(reg, tmp_value);
	}
	else {
		tmp_value = RTL_R8(reg);
		tmp_value &= (~value);
		RTL_W8(reg, tmp_value);
	}
}

void assign_pin_as_gpio(struct rtl8192cd_priv *priv, unsigned int num)
{
	//need to assign some specific values to assign pin fun as gpio (follow docu from MAC team)

	switch(num)
	{
		case 0:
				reg_bit_clear(priv, 0x40, (BIT(0)|BIT(1)|BIT(2)|BIT(10)));
				reg_bit_set(priv, 0x40, BIT(9));
				reg_bit_clear(priv, 0x66, (BIT(0)|BIT(1)|BIT(2)));

				return;
		case 1:
				reg_bit_clear(priv, 0x40, (BIT(0)|BIT(1)|BIT(2)|BIT(10)));
				reg_bit_set(priv, 0x40, BIT(9));
				reg_bit_clear(priv, 0x66, (BIT(0)|BIT(1)|BIT(2)));

				return;
		case 2:
				reg_bit_clear(priv, 0x40, (BIT(0)|BIT(1)|BIT(2)|BIT(10)));
				reg_bit_set(priv, 0x40, BIT(9));
				reg_bit_clear(priv, 0x66, (BIT(0)|BIT(1)|BIT(2)));
				reg_bit_clear(priv, 0x54, BIT(18));

				return;
		case 3:
				reg_bit_clear(priv, 0x40, (BIT(0)|BIT(1)|BIT(2)|BIT(10)));
				reg_bit_set(priv, 0x40, BIT(9));
				reg_bit_clear(priv, 0x66, (BIT(0)|BIT(1)|BIT(2)));
				reg_bit_clear(priv, 0x54, BIT(18));

				return;
		case 4:
				reg_bit_clear(priv, 0x40, (BIT(0)|BIT(1)|BIT(19)));
				reg_bit_clear(priv, 0x66, (BIT(4)|BIT(8)));
				reg_bit_clear(priv, 0x70, (BIT(18)|BIT(27)));

				return;

		case 5:
				reg_bit_clear(priv, 0x40, (BIT(0)|BIT(1)|BIT(19)));
				reg_bit_clear(priv, 0x66, (BIT(4)|BIT(8)));
				reg_bit_clear(priv, 0x70, BIT(27));

				return;

		case 6:
				reg_bit_clear(priv, 0x38, (BIT(3)|BIT(6)));
				reg_bit_clear(priv, 0x40, (BIT(0)|BIT(1)|BIT(4)|BIT(19)));
				reg_bit_clear(priv, 0x66, (BIT(4)|BIT(8)));
				reg_bit_clear(priv, 0x64, BIT(25));

				return;
		case 7:
				reg_bit_clear(priv, 0x38, (BIT(3)|BIT(6)));
				reg_bit_clear(priv, 0x40, (BIT(0)|BIT(1)|BIT(4)|BIT(8)|BIT(19)));
				reg_bit_clear(priv, 0x66, (BIT(4)|BIT(8)));
				reg_bit_clear(priv, 0x64, BIT(26));

				return;
		case 8:
				reg_bit_clear(priv, 0x48, BIT(16));
				reg_bit_clear(priv, 0x4c, BIT(21));

				return;

		case 9:
				reg_bit_clear(priv, 0x48, BIT(16));
				reg_bit_clear(priv, 0x66, BIT(6));
				reg_bit_clear(priv, 0x68, (BIT(0)|BIT(3)));

				return;

		case 10:

				return;

		case 11:

				reg_bit_clear(priv, 0x40, BIT(27));
				reg_bit_clear(priv, 0x66, BIT(6));
				reg_bit_clear(priv, 0x6a, (BIT(0)|BIT(3)));

				return;

		case 12:

				reg_bit_clear(priv, 0x38, BIT(6));
				reg_bit_clear(priv, 0x66, BIT(6));

				return;

		case 13:

				reg_bit_clear(priv, 0x4c, BIT(22));
				return;

		case 14:

				reg_bit_clear(priv, 0x4c, BIT(22));
				return;

		case 15:

				reg_bit_clear(priv, 0x66, BIT(7));
				return;

		default:

			return;

	}

}

void write_gpio_8822(struct rtl8192cd_priv *priv, unsigned int num, unsigned int value)
{
	unsigned int reg_gpio_ctrl = 0x0;
	unsigned int offset = 0;
	unsigned int tmp_value = 0x0;

	if(num < 8)
	{
		reg_gpio_ctrl = 0x44;
		offset = num;
	}
	else if(num < 16)
	{
		reg_gpio_ctrl = 0x60;
		offset = (num - 8);
	}
	else
		return;

	//panic_printk("[%s] num=%d value=%d \n", __FUNCTION__, num, value);

	tmp_value = RTL_R32(reg_gpio_ctrl);

	if(value)
		tmp_value |= (BIT(8+offset));
	else
		tmp_value &= (~(BIT(8+offset)));

	RTL_W32(reg_gpio_ctrl, tmp_value);

}


int read_gpio_8822(struct rtl8192cd_priv *priv, unsigned int num)
{
	unsigned int reg_gpio_ctrl = 0x0;
	unsigned int offset = 0;
	unsigned int tmp_value = 0x0;
	unsigned int ret = 0;

	if(num < 8)
	{
		reg_gpio_ctrl = 0x44;
		offset = num;
	}
	else if(num < 16)
	{
		reg_gpio_ctrl = 0x60;
		offset = (num - 8);
	}
	else
		return;

	tmp_value = RTL_R32(reg_gpio_ctrl);

	if(tmp_value & BIT(offset))
		ret = 1;
	else
		ret = 0;

	//panic_printk("[%s] num=%d ret=%d \n", __FUNCTION__, num, ret);

	return ret;

}

void config_gpio_8822(struct rtl8192cd_priv *priv, unsigned int num, unsigned int dir)
{
	unsigned int reg_gpio_ctrl = 0x0;
	unsigned int offset = 0;
	unsigned int tmp_value = 0x0;

	if(num < 8)
	{
		reg_gpio_ctrl = 0x44;
		offset = num;
	}
	else if(num < 16)
	{
		reg_gpio_ctrl = 0x60;
		offset = (num - 8);
	}
	else
		return;

	//panic_printk("[%s] num=%d dir=0x%x \n", __FUNCTION__, num, dir);

	priv->pshare->phw->GPIO_dir[num] = dir;

	assign_pin_as_gpio(priv, num);

	tmp_value = RTL_R32(reg_gpio_ctrl);
	tmp_value &= (~(BIT(24+offset))); //clear interrupt mode

	if(dir == 0x10)
		tmp_value |= (BIT(16+offset)); //gpio = output
	else if(dir == 0x1)
		tmp_value &= (~(BIT(16+offset))); //gpio = input

	RTL_W32(reg_gpio_ctrl, tmp_value);

}


struct rtl8192cd_priv *root_priv;

void RTLWIFINIC_GPIO_init_priv(struct rtl8192cd_priv *priv)
{
	root_priv = priv;
}

void RTLWIFINIC_GPIO_config(unsigned int gpio_num, unsigned int direction)
{
	struct rtl8192cd_priv *priv = root_priv;

	if (!root_priv)
		return;

	if (GET_CHIP_VER(priv) == VERSION_8822B){

		config_gpio_8822(priv, gpio_num, direction);

		return;
	}


	if ((gpio_num >= 0) && (gpio_num <= 7)) {
		priv->pshare->phw->GPIO_dir[gpio_num] = direction;

		if (direction == 0x01) {
			RTL_W32(GPIO_PIN_CTRL, RTL_R32(GPIO_PIN_CTRL) & ~(BIT(gpio_num + 24) | BIT(gpio_num + 16)));
			return;
		} else if (direction == 0x10) {
			RTL_W32(GPIO_PIN_CTRL, (RTL_R32(GPIO_PIN_CTRL) & ~BIT(gpio_num + 24)) | (BIT(gpio_num + 16) | BIT(gpio_num + 8)));
			return;
		}
	}

	if ((GET_CHIP_VER(priv) == VERSION_8192E) || (GET_CHIP_VER(priv) == VERSION_8881A) || (GET_CHIP_VER(priv) == VERSION_8814A) ||
		(GET_CHIP_VER(priv) == VERSION_8822B)) {
		if ((gpio_num >= 8) && (gpio_num <= 15)) {
			priv->pshare->phw->GPIO_dir[gpio_num] = direction;

			if (direction == 0x01) {
				RTL_W32(0x060, RTL_R32(0x060) & ~(BIT(gpio_num + 16) | BIT(gpio_num + 8)));
				return;
			} else if (direction == 0x10) {
				RTL_W32(0x060, (RTL_R32(0x060) & ~BIT(gpio_num + 16)) | (BIT(gpio_num + 8) | BIT(gpio_num)));
				return;
			}
		}
	}
	else {
		if ((gpio_num >= 8) && (gpio_num <= 11)) {
			priv->pshare->phw->GPIO_dir[gpio_num] = direction;

			if (direction == 0x01) {
				RTL_W32(GPIO_MUXCFG, RTL_R32(GPIO_MUXCFG) & ~(BIT(gpio_num + 20) | BIT(gpio_num + 16)));
				return;
			} else if (direction == 0x10) {
				RTL_W32(GPIO_MUXCFG, (RTL_R32(GPIO_MUXCFG) & ~BIT(gpio_num + 20)) | (BIT(gpio_num + 16) | BIT(gpio_num + 12)));
				return;
			}
		}
	}

	panic_printk("GPIO %d action %d not support!\n", gpio_num, direction);
	return;
}

void RTLWIFINIC_GPIO_write(unsigned int gpio_num, unsigned int value)
{

	struct rtl8192cd_priv *priv = root_priv;
	if (!root_priv)
		return;


	if (priv->pshare->phw->GPIO_dir[gpio_num] != 0x10)
		RTLWIFINIC_GPIO_config(gpio_num, 0x10);

	if (GET_CHIP_VER(priv) == VERSION_8822B){
		write_gpio_8822(priv, gpio_num, value);
		return;
	}

	if (((gpio_num >= 0) && (gpio_num <= 7)) && (priv->pshare->phw->GPIO_dir[gpio_num] == 0x10)) {
		if (value)
			RTL_W32(GPIO_PIN_CTRL, RTL_R32(GPIO_PIN_CTRL) & ~BIT(gpio_num + 8));
		else
			RTL_W32(GPIO_PIN_CTRL, RTL_R32(GPIO_PIN_CTRL) | BIT(gpio_num + 8));
		return;
	}

	if ((GET_CHIP_VER(priv) == VERSION_8192E) || (GET_CHIP_VER(priv) == VERSION_8881A) || (GET_CHIP_VER(priv) == VERSION_8814A) ||
		(GET_CHIP_VER(priv) == VERSION_8822B)) {
		if (((gpio_num >= 8) && (gpio_num <= 15)) && (priv->pshare->phw->GPIO_dir[gpio_num] == 0x10)) {
			if (value)
				RTL_W32(0x060, RTL_R32(0x060) & ~BIT(gpio_num));
			else
				RTL_W32(0x060, RTL_R32(0x060) | BIT(gpio_num));
			return;
		}
	}
	else {
		if (((gpio_num >= 8) && (gpio_num <= 11)) && (priv->pshare->phw->GPIO_dir[gpio_num] == 0x10)) {
			if (value)
				RTL_W32(GPIO_MUXCFG, RTL_R32(GPIO_MUXCFG) & ~BIT(gpio_num + 12));
			else
				RTL_W32(GPIO_MUXCFG, RTL_R32(GPIO_MUXCFG) | BIT(gpio_num + 12));
			return;
		}
	}

	panic_printk("GPIO %d set value %d not support!\n", gpio_num, value);
	return;
}


int RTLWIFINIC_GPIO_read(unsigned int gpio_num)
{
	struct rtl8192cd_priv *priv = root_priv;
	unsigned int val32;
	if (!root_priv)
		return -1;

	if (priv->pshare->phw->GPIO_dir[gpio_num] != 0x01)
		RTLWIFINIC_GPIO_config(gpio_num, 0x01);

	if (GET_CHIP_VER(priv) == VERSION_8822B){
		return read_gpio_8822(priv, gpio_num);
	}

	if (((gpio_num >= 0) && (gpio_num <= 7)) && (priv->pshare->phw->GPIO_dir[gpio_num] == 0x01)) {
			val32 = RTL_R32(GPIO_PIN_CTRL);
		if (val32 & BIT(gpio_num))
			return 0;
		else
			return 1;
	}

	if ((GET_CHIP_VER(priv) == VERSION_8192E) || (GET_CHIP_VER(priv) == VERSION_8881A) || (GET_CHIP_VER(priv) == VERSION_8814A) ||
		(GET_CHIP_VER(priv) == VERSION_8822B)) {
		if (((gpio_num >= 8) && (gpio_num <= 15)) && (priv->pshare->phw->GPIO_dir[gpio_num] == 0x01)) {
			val32 = RTL_R32(0x060);
			if (val32 & BIT(gpio_num - 8))
				return 0;
			else
				return 1;
		}
	}
	else {
		if (((gpio_num >= 8) && (gpio_num <= 11)) && (priv->pshare->phw->GPIO_dir[gpio_num] == 0x01)) {
				val32 = RTL_R32(GPIO_MUXCFG);
			if (val32 & BIT(gpio_num + 8))
				return 0;
			else
				return 1;
		}
	}

	panic_printk("GPIO %d get value not support!\n", gpio_num);
	return -1;
}

void RTLWIFINIC_GPIO_config_proc(struct rtl8192cd_priv *priv, unsigned int gpio_num, unsigned int direction)
{


	if ((gpio_num >= 0) && (gpio_num <= 7)) {
		priv->pshare->phw->GPIO_dir[gpio_num] = direction;

		if (direction == 0x01) {
			RTL_W32(GPIO_PIN_CTRL, RTL_R32(GPIO_PIN_CTRL) & ~(BIT(gpio_num + 24) | BIT(gpio_num + 16)));
			return;
		} else if (direction == 0x10) {
			RTL_W32(GPIO_PIN_CTRL, (RTL_R32(GPIO_PIN_CTRL) & ~BIT(gpio_num + 24)) | (BIT(gpio_num + 16) | BIT(gpio_num + 8)));
			return;
		}
	}

	if ((GET_CHIP_VER(priv) == VERSION_8192E) || (GET_CHIP_VER(priv) == VERSION_8881A) || (GET_CHIP_VER(priv) == VERSION_8814A) ||
		(GET_CHIP_VER(priv) == VERSION_8822B)) {
		if ((gpio_num >= 8) && (gpio_num <= 15)) {
			priv->pshare->phw->GPIO_dir[gpio_num] = direction;

			if (direction == 0x01) {
				RTL_W32(0x060, RTL_R32(0x060) & ~(BIT(gpio_num + 16) | BIT(gpio_num + 8)));
				return;
			} else if (direction == 0x10) {
				RTL_W32(0x060, (RTL_R32(0x060) & ~BIT(gpio_num + 16)) | (BIT(gpio_num + 8) | BIT(gpio_num)));
				return;
			}
		}
	}
	else {
		if ((gpio_num >= 8) && (gpio_num <= 11)) {
			priv->pshare->phw->GPIO_dir[gpio_num] = direction;

			if (direction == 0x01) {
				RTL_W32(GPIO_MUXCFG, RTL_R32(GPIO_MUXCFG) & ~(BIT(gpio_num + 20) | BIT(gpio_num + 16)));
				return;
			} else if (direction == 0x10) {
				RTL_W32(GPIO_MUXCFG, (RTL_R32(GPIO_MUXCFG) & ~BIT(gpio_num + 20)) | (BIT(gpio_num + 16) | BIT(gpio_num + 12)));
				return;
			}
		}
	}

	panic_printk("GPIO %d action %d not support!\n", gpio_num, direction);
	return;
}

void RTLWIFINIC_GPIO_write_proc(struct rtl8192cd_priv *priv, unsigned int gpio_num, unsigned int value)
{

	if (priv->pshare->phw->GPIO_dir[gpio_num] != 0x10)
		RTLWIFINIC_GPIO_config(gpio_num, 0x10);

	if (((gpio_num >= 0) && (gpio_num <= 7)) && (priv->pshare->phw->GPIO_dir[gpio_num] == 0x10)) {
		if (value)
			RTL_W32(GPIO_PIN_CTRL, RTL_R32(GPIO_PIN_CTRL) & ~BIT(gpio_num + 8));
		else
			RTL_W32(GPIO_PIN_CTRL, RTL_R32(GPIO_PIN_CTRL) | BIT(gpio_num + 8));
		return;
	}

	if ((GET_CHIP_VER(priv) == VERSION_8192E) || (GET_CHIP_VER(priv) == VERSION_8881A) || (GET_CHIP_VER(priv) == VERSION_8814A) ||
		(GET_CHIP_VER(priv) == VERSION_8822B)) {
		if (((gpio_num >= 8) && (gpio_num <= 15)) && (priv->pshare->phw->GPIO_dir[gpio_num] == 0x10)) {
			if (value)
				RTL_W32(0x060, RTL_R32(0x060) & ~BIT(gpio_num));
			else
				RTL_W32(0x060, RTL_R32(0x060) | BIT(gpio_num));
			return;
		}
	}
	else {
		if (((gpio_num >= 8) && (gpio_num <= 11)) && (priv->pshare->phw->GPIO_dir[gpio_num] == 0x10)) {
			if (value)
				RTL_W32(GPIO_MUXCFG, RTL_R32(GPIO_MUXCFG) & ~BIT(gpio_num + 12));
			else
				RTL_W32(GPIO_MUXCFG, RTL_R32(GPIO_MUXCFG) | BIT(gpio_num + 12));
			return;
		}
	}

	panic_printk("GPIO %d set value %d not support!\n", gpio_num, value);
	return;
}


int RTLWIFINIC_GPIO_read_proc(struct rtl8192cd_priv *priv, unsigned int gpio_num)
{
	unsigned int val32;

	if (priv->pshare->phw->GPIO_dir[gpio_num] != 0x01)
		RTLWIFINIC_GPIO_config(gpio_num, 0x01);

	if (((gpio_num >= 0) && (gpio_num <= 7)) && (priv->pshare->phw->GPIO_dir[gpio_num] == 0x01)) {
			val32 = RTL_R32(GPIO_PIN_CTRL);
		if (val32 & BIT(gpio_num))
			return 0;
		else
			return 1;
	}

	if ((GET_CHIP_VER(priv) == VERSION_8192E) || (GET_CHIP_VER(priv) == VERSION_8881A) || (GET_CHIP_VER(priv) == VERSION_8814A) ||
		(GET_CHIP_VER(priv) == VERSION_8822B)) {
		if (((gpio_num >= 8) && (gpio_num <= 15)) && (priv->pshare->phw->GPIO_dir[gpio_num] == 0x01)) {
			val32 = RTL_R32(0x060);
			if (val32 & BIT(gpio_num - 8))
				return 0;
			else
				return 1;
		}
	}
	else {
		if (((gpio_num >= 8) && (gpio_num <= 11)) && (priv->pshare->phw->GPIO_dir[gpio_num] == 0x01)) {
				val32 = RTL_R32(GPIO_MUXCFG);
			if (val32 & BIT(gpio_num + 8))
				return 0;
			else
				return 1;
		}
	}

	panic_printk("GPIO %d get value not support!\n", gpio_num);
	return -1;
}
#endif


// TODO: move into HAL
BOOLEAN
compareAvailableTXBD(
	struct rtl8192cd_priv   *priv,
	unsigned int            num,
	unsigned int            qNum,
	int                     compareFlag
)
{
	PHCI_TX_DMA_MANAGER_88XX    ptx_dma = (PHCI_TX_DMA_MANAGER_88XX)(_GET_HAL_DATA(priv)->PTxDMA88XX);
	unsigned int                halQnum = GET_HAL_INTERFACE(priv)->MappingTxQueueHandler(priv, qNum);
	unsigned long               avail_txbd_flag;

	SAVE_INT_AND_CLI(avail_txbd_flag);

	if (compareFlag == 1) {
		if (ptx_dma->tx_queue[halQnum].avail_txbd_num > num)  {
			RESTORE_INT(avail_txbd_flag);
			return TRUE;
		} else {
			RESTORE_INT(avail_txbd_flag);
			return FALSE;
		}
	} else if (compareFlag == 2) {
		if (ptx_dma->tx_queue[halQnum].avail_txbd_num < num)  {
			RESTORE_INT(avail_txbd_flag);
			return TRUE;
		} else {
			RESTORE_INT(avail_txbd_flag);
			return FALSE;
		}
	} else {
		printk("%s(%d): Error setting !!! \n", __FUNCTION__, __LINE__);
	}

	RESTORE_INT(avail_txbd_flag);

	return FALSE;
}


/* Hotsport 2.0 Release 1 */
void start_bbp_ch_load(struct rtl8192cd_priv *priv, unsigned int units)
{
    unsigned short chip_ver = GET_CHIP_VER(priv);
    if(chip_ver == VERSION_8188C || chip_ver == VERSION_8192C || chip_ver ==VERSION_8192D ||
        chip_ver == VERSION_8188E || chip_ver == VERSION_8192E || chip_ver == VERSION_8197F) /*all N-series ic*/
    {
        RTL_W16(0x894, units); //set ch load period time units*4 usec
        RTL_W32(0x890, 0x0);	//reset
        RTL_W32(0x890, 0x100);
        RTL_W32(0x890, 0x101);
    }
    else /* 8812, 8881A, 8814A, all AC-series ic*/
    {
        RTL_W16(0x990, units); //set ch load period time units*4 usec
        RTL_W32(0x994, 0x0);	//reset
        RTL_W32(0x994, 0x100);
        RTL_W32(0x994, 0x101);
    }
}


/*/return busy count ,its units = 4us*/
unsigned int read_bbp_ch_load(struct rtl8192cd_priv *priv)
{
    unsigned short chip_ver = GET_CHIP_VER(priv);
	unsigned char retry = 0;
    if(chip_ver == VERSION_8188C || chip_ver == VERSION_8192C || chip_ver ==VERSION_8192D ||
        chip_ver == VERSION_8188E || chip_ver == VERSION_8192E || chip_ver == VERSION_8197F) /*all N-series ic*/
    {
	if (RTL_R32(0x8b4) & BIT16)
	            	return (RTL_R32(0x8d0) & 0xffff) ;

    }else{ /* 8812, 8881A, 8813, all AC-series ic*/

	if (RTL_R32(0xfa4) & BIT16)
	            	return (RTL_R32(0xfa4) & 0xffff);
    }

	return 0;
}


/*CLM [channle loading measurement]*/
void channle_loading_measurement(struct rtl8192cd_priv *priv){

	unsigned int clm_count=0;

	if (phydm_checkCLMready(ODMPTR) ){ 		/* --Getting CLM info previous time--*/
		clm_count = phydm_getCLMresult(ODMPTR);
	}
	 if(clm_count >0 ){
		/*IEEE,Std 802.11-2012,page567,use 0~255 to representing 0~100%*/
		//priv->ext_stats.ch_utilization = (clm_count * 255) /CLM_SAMPLE_NUM2;
		priv->ext_stats.ch_utilization = (clm_count * 255) /CLM_SAMPLE_NUM2;
		//GDEBUG("busy time[%d]ms\n",RTK80211_SAMPLE_NUM_TO_TIME(clm_count));
	}
	phydm_CLMtrigger(ODMPTR);/* trigger next time CLM */

}


#ifdef CONFIG_1RCCA_RF_POWER_SAVING
void one_path_cca_power_save(struct rtl8192cd_priv *priv, int enable)
{
	if (enable && !priv->pshare->rf_ft_var.one_path_cca_ps_active) {
		priv->pshare->rf_ft_var.one_path_cca_ps_active = 1;
		RTL_W8(0x800, RTL_R8(0x800) | BIT1);
		RTL_W8(0xc04, 0x13);
	} else if (!enable && priv->pshare->rf_ft_var.one_path_cca_ps_active) {
		priv->pshare->rf_ft_var.one_path_cca_ps_active = 0;
		RTL_W8(0x800, RTL_R8(0x800) & ~BIT1);
		RTL_W8(0xc04, 0x33);
	}
}
#endif // CONFIG_1RCCA_RF_POWER_SAVING

