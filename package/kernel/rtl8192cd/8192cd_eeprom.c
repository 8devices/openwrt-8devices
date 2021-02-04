/*
 *  Routines to read and write eeprom
 *
 *  $Id: 8192cd_eeprom.c,v 1.1 2009/11/06 12:26:48 victoryman Exp $
 *
 *  Copyright (c) 2009 Realtek Semiconductor Corp.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 */

#define _8192CD_EEPROM_C_
#if 0
#ifdef __KERNEL__
#include <linux/config.h>
#include <linux/module.h>
#endif

#include "./8192cd_cfg.h"
#include "./8192cd.h"
#include "./8192cd_hw.h"
#include "./8192cd_util.h"
#include "./8192cd_debug.h"



#define VOID void
#define EEPROM_MAX_SIZE        	256
#define	CSR_EEPROM_CONTROL_REG	_9346CR_
#define	CLOCK_RATE				50			//100us


static VOID ShiftOutBits(struct rtl8192cd_priv *priv, USHORT data, USHORT count);
static USHORT ShiftInBits(struct rtl8192cd_priv *priv);
static VOID RaiseClock(struct rtl8192cd_priv *priv, USHORT *x);
static VOID LowerClock(struct rtl8192cd_priv *priv, USHORT *x);
static VOID EEpromCleanup(struct rtl8192cd_priv *priv);
static USHORT WaitEEPROMCmdDone(struct rtl8192cd_priv *priv);
static VOID StandBy(struct rtl8192cd_priv *priv);


//*****************************************************************************
//
//            I/O based Read EEPROM Routines
//
//*****************************************************************************
//-----------------------------------------------------------------------------
// Procedure:   ReadEEprom
//
// Description: This routine serially reads one word out of the EEPROM.
//
// Arguments:
//      Reg - EEPROM word to read.
//
// Returns:
//      Contents of EEPROM word (Reg).
//-----------------------------------------------------------------------------
static USHORT
ReadEEprom(
	struct rtl8192cd_priv *priv,
	UCHAR	AddressSize,
    USHORT 	Reg)
{
    USHORT x;
    USHORT data;

    // select EEPROM, reset bits, set EECS
    x = RTL_R8(CSR_EEPROM_CONTROL_REG);

    x &= ~(EEDI | EEDO | EESK | CR9346_EEM0);
    x |= CR9346_EEM1 | EECS;
    RTL_W8(CSR_EEPROM_CONTROL_REG, (UCHAR)x);

    // write the read opcode and register number in that order
    // The opcode is 3bits in length, reg is 6 bits long
    ShiftOutBits(priv, EEPROM_READ_OPCODE, 3);
    ShiftOutBits(priv, Reg, AddressSize);

    // Now read the data (16 bits) in from the selected EEPROM word
    data = ShiftInBits(priv);

    EEpromCleanup(priv);
    return data;
}

//-----------------------------------------------------------------------------
// Procedure:   ShiftOutBits
//
// Description: This routine shifts data bits out to the EEPROM.
//
// Arguments:
//      data - data to send to the EEPROM.
//      count - number of data bits to shift out.
//
// Returns: (none)
//-----------------------------------------------------------------------------

static VOID
ShiftOutBits(
	struct rtl8192cd_priv *priv,
    USHORT data,
	USHORT count)
{
    USHORT x,mask;

    mask = 0x01 << (count - 1);
    x = RTL_R8(CSR_EEPROM_CONTROL_REG);

    x &= ~(EEDO | EEDI);

    do
    {
        x &= ~EEDI;
        if(data & mask)
            x |= EEDI;

        RTL_W8(CSR_EEPROM_CONTROL_REG, (UCHAR)x);
        delay_us(CLOCK_RATE);
        RaiseClock(priv, &x);
        LowerClock(priv, &x);
        mask = mask >> 1;
    } while(mask);

    x &= ~EEDI;
    RTL_W8(CSR_EEPROM_CONTROL_REG, (UCHAR)x);
}

//-----------------------------------------------------------------------------
// Procedure:   ShiftInBits
//
// Description: This routine shifts data bits in from the EEPROM.
//
// Arguments:
//
// Returns:
//      The contents of that particular EEPROM word
//-----------------------------------------------------------------------------

static USHORT
ShiftInBits(
	struct rtl8192cd_priv *priv)
{
    USHORT x,d,i;
    x = RTL_R8(CSR_EEPROM_CONTROL_REG);

    x &= ~( EEDO | EEDI);
    d = 0;

    for(i=0; i<16; i++)
    {
        d = d << 1;
        RaiseClock(priv, &x);

        x = RTL_R8(CSR_EEPROM_CONTROL_REG);

        x &= ~(EEDI);
        if(x & EEDO)
            d |= 1;

        LowerClock(priv, &x);
    }

    return d;
}

//-----------------------------------------------------------------------------
// Procedure:   RaiseClock
//
// Description: This routine raises the EEPOM's clock input (EESK)
//
// Arguments:
//      x - Ptr to the EEPROM control register's current value
//
// Returns: (none)
//-----------------------------------------------------------------------------

static VOID
RaiseClock(
	struct rtl8192cd_priv *priv,
    USHORT *x)
{
    *x = *x | EESK;
    RTL_W8(CSR_EEPROM_CONTROL_REG, (UCHAR)(*x));
    delay_us(CLOCK_RATE);
}


//-----------------------------------------------------------------------------
// Procedure:   LowerClock
//
// Description: This routine lower's the EEPOM's clock input (EESK)
//
// Arguments:
//      x - Ptr to the EEPROM control register's current value
//
// Returns: (none)
//-----------------------------------------------------------------------------

static VOID
LowerClock(
	struct rtl8192cd_priv *priv,
    USHORT *x)
{
    *x = *x & ~EESK;
    RTL_W8(CSR_EEPROM_CONTROL_REG, (UCHAR)(*x));
    delay_us(CLOCK_RATE);
}

//-----------------------------------------------------------------------------
// Procedure:   EEpromCleanup
//
// Description: This routine returns the EEPROM to an idle state
//
// Arguments:
//
// Returns: (none)
//-----------------------------------------------------------------------------

static VOID
EEpromCleanup(
	struct rtl8192cd_priv *priv)
{
    USHORT x;
    x = RTL_R8(CSR_EEPROM_CONTROL_REG);

    x &= ~(EECS | EEDI);
    RTL_W8(CSR_EEPROM_CONTROL_REG, (UCHAR)x);

    RaiseClock(priv, &x);
    LowerClock(priv, &x);
}

//*****************************************************************************
//
//            EEPROM Write Routines
//
//*****************************************************************************

//-----------------------------------------------------------------------------
// Procedure:   D100UpdateChecksum
//
// Description: Calculates the checksum and writes it to the EEProm.  This
//              routine assumes that the checksum word is the last word in
//              a 64 word EEPROM.  It calculates the checksum accroding to
//              the formula: Checksum = 0xBABA - (sum of first 63 words).
//
// Arguments:
//      Adapter - Ptr to this card's adapter data structure
//
// Returns: (none)
//-----------------------------------------------------------------------------

/*static VOID
UpdateChecksum(
	struct rtl8192cd_priv *priv)
{
    USHORT    Checksum=0;
//    USHORT    Iter;

//    for (Iter = 0; Iter < 0x3F; Iter++)
//        Checksum += ReadEEprom( CSRBaseIoAddress, Iter );

    Checksum = (USHORT)0xBABA - Checksum;
//    WriteEEprom( CSRBaseIoAddress, 0x3F, Checksum );
}*/

//-----------------------------------------------------------------------------
// Procedure:   WriteEEprom
//
// Description: This routine writes a word to a specific EEPROM location.
//
// Arguments:
//      Adapter - Ptr to this card's adapter data structure.
//      reg - The EEPROM word that we are going to write to.
//      data - The data (word) that we are going to write to the EEPROM.
//
// Returns: (none)
//-----------------------------------------------------------------------------

static VOID
WriteEEprom(
	struct rtl8192cd_priv *priv,
	UCHAR	AddressSize,
	USHORT reg,
    USHORT data)
{
    UCHAR x;

    // select EEPROM, mask off ASIC and reset bits, set EECS
    x = RTL_R8(CSR_EEPROM_CONTROL_REG);

    x &= ~(EEDI | EEDO | EESK | CR9346_EEM0);
    x |= CR9346_EEM1 | EECS;
    RTL_W8(CSR_EEPROM_CONTROL_REG, x);

    ShiftOutBits(priv, EEPROM_EWEN_OPCODE, 5);
    /////ShiftOutBits(CSRBaseIoAddress, reg, 4);
	ShiftOutBits(priv, 0, 6);

    StandBy(priv);

    // Erase this particular word.  Write the erase opcode and register
    // number in that order. The opcode is 3bits in length; reg is 6 bits long.
    ShiftOutBits(priv, EEPROM_ERASE_OPCODE, 3);
    ShiftOutBits(priv, reg, AddressSize);

    if (WaitEEPROMCmdDone(priv) == FALSE)
    {
        return;
    }

    StandBy(priv);

    // write the new word to the EEPROM

    // send the write opcode the EEPORM
    ShiftOutBits(priv, EEPROM_WRITE_OPCODE, 3);

    // select which word in the EEPROM that we are writing to.
    ShiftOutBits(priv, reg, AddressSize);

    // write the data to the selected EEPROM word.
    ShiftOutBits(priv, data, 16);

    if (WaitEEPROMCmdDone(priv) == FALSE)
    {
//        DbgPrint("D100: Failed EEPROM Write");
        return;
    }

    StandBy(priv);

    ShiftOutBits(priv, EEPROM_EWDS_OPCODE, 5);
    ShiftOutBits(priv, reg, 4);

    EEpromCleanup(priv);
    return;
}

//-----------------------------------------------------------------------------
// Procedure:   WaitEEPROMCmdDone
//
// Description: This routine waits for the the EEPROM to finish its command.
//              Specifically, it waits for EEDO (data out) to go high.
//
// Arguments:
//      Adapter - Ptr to this card's adapter data structure.
//
// Returns:
//      TRUE - If the command finished
//      FALSE - If the command never finished (EEDO stayed low)
//-----------------------------------------------------------------------------

static USHORT
WaitEEPROMCmdDone(
	struct rtl8192cd_priv *priv)
{
    UCHAR 	x;
    USHORT	i;

    StandBy(priv);
    for (i=0; i<200; i++)
    {
        x = RTL_R8(CSR_EEPROM_CONTROL_REG);
        if (x & EEDO)
            return (TRUE);
        delay_us(CLOCK_RATE);
    }
    return FALSE;
}


//-----------------------------------------------------------------------------
// Procedure:   StandBy
//
// Description: This routine lowers the EEPROM chip select (EECS) for a few
//              microseconds.
//
// Arguments:
//      Adapter - Ptr to this card's adapter data structure.
//
// Returns: (none)
//-----------------------------------------------------------------------------

static VOID
StandBy(
	struct rtl8192cd_priv *priv)
{
    UCHAR    x;

    x = RTL_R8(CSR_EEPROM_CONTROL_REG);

    x &= ~(EECS | EESK);
    RTL_W8(CSR_EEPROM_CONTROL_REG, x);

    delay_us(CLOCK_RATE);
    x |= EECS;
    RTL_W8(CSR_EEPROM_CONTROL_REG, x);
	delay_us(CLOCK_RATE);
}


//*****************************************************************************
//
//            Main routines to read and write EEPROM
//
//*****************************************************************************
#define NUM_11A_CHANNEL 46
const UCHAR	ChannelNumberListOf11a[] = {
	26, 28, 30, 32,
	34, 36, 38, 40, 42, 44, 46, 48, 50, 52, 54, 56, 58, 60, 62, 64,
	66, 68, 70, 72, 74, 76, 78, 80, 82, 84, 86,
	100, 104, 108, 112, 116, 120, 124, 128, 132, 136, 140,
	149, 153, 157, 161
};
#define	READ_EEPROM(addr)	ReadEEprom(priv, EepromAddressSize, addr)
#define	WRITE_EEPROM(a,d)	WriteEEprom(priv, EepromAddressSize, a, d)

int ReadAdapterInfo(struct rtl8192cd_priv *priv, int entry_id, void *data)
{
	USHORT			Index;
	USHORT          usValue;
	ULONG			curRCR;
	UCHAR			EepromAddressSize;
	UCHAR			TxPowerLevel[64];

	if (!priv->EE_Cached)
	{
		curRCR = RTL_R32(_RCR_);
		EepromAddressSize = (curRCR & _9356SEL_)? 8 : 6;

		// ID
		priv->EE_ID = (unsigned int)READ_EEPROM(EEPROM_ID);
		DEBUG_INFO("ID 0x%04X\n", (USHORT)priv->EE_ID);
		if (priv->EE_ID != RTL8180_EEPROM_ID) {
			DEBUG_INFO("ID is invalid\n");
			priv->EE_AutoloadFail = TRUE;
		}
		else
			priv->EE_AutoloadFail = FALSE;

		// Version
//		priv->EE_Version = (unsigned int)READ_EEPROM((USHORT)(EEPROM_VERSION >> 1));
		usValue = READ_EEPROM(0x7C >> 1);
		priv->EE_Version = ((usValue&0xff00)>>8);
		DEBUG_INFO("Version 0x%x\n", (USHORT)priv->EE_Version);

		// MAC address
		for (Index = 0; Index < 6; Index += 2) {
			usValue = READ_EEPROM((USHORT)((EEPROM_NODE_ADDRESS_BYTE_0 + Index)>>1));
			priv->EE_Mac[Index] = usValue & 0xff;
			priv->EE_Mac[Index+1] = ((usValue&0xff00) >> 8);
		}
		DEBUG_INFO("Mac %02X-%02X-%02X-%02X-%02X-%02X\n",
			priv->EE_Mac[0], priv->EE_Mac[1], priv->EE_Mac[2], priv->EE_Mac[3],
			priv->EE_Mac[4], priv->EE_Mac[5]);

		// for identifying empty EEPROM
		if (!priv->EE_AutoloadFail)
		{
			// Tx Power Level
			memset(priv->EE_TxPower_CCK, 0, sizeof(priv->EE_TxPower_CCK));
			for (Index = 0; Index < MAX_CCK_CHANNEL_NUM; Index += 2) {
				usValue = READ_EEPROM((USHORT)((EEPROM_TX_POWER_LEVEL_0 + Index) >> 1));
				*((USHORT *)(&priv->EE_TxPower_CCK[Index])) = usValue;
			}

			memset(priv->EE_TxPower_OFDM, 0, sizeof(priv->EE_TxPower_OFDM));
			for (Index = 0; Index < NUM_11A_CHANNEL; Index += 2) {
				usValue = READ_EEPROM((USHORT)((EEPROM_11A_CHANNEL_TX_POWER_LEVEL_OFFSET + Index) >> 1));
				*((USHORT *)(&TxPowerLevel[Index])) = usValue;
			}
			for (Index = 0; Index < NUM_11A_CHANNEL; Index++)
				priv->EE_TxPower_OFDM[ChannelNumberListOf11a[Index] - 1] = TxPowerLevel[Index];

			for (Index = 0; Index < MAX_CCK_CHANNEL_NUM; Index += 2) {
				usValue = READ_EEPROM((USHORT)((EEPROM_11G_CHANNEL_OFDM_TX_POWER_LEVEL_OFFSET + Index) >> 1));
				*((USHORT *)(&TxPowerLevel[Index])) = usValue;
			}
			for (Index = 0; Index < MAX_CCK_CHANNEL_NUM; Index++)
				priv->EE_TxPower_OFDM[Index] = TxPowerLevel[Index];

			#ifdef _DEBUG_RTL8192CD_
			if (rtl8192cd_debug_info & _MODULE_DEFINE) {
				extern void debug_out(char *label, unsigned char *data, int data_length);
				debug_out("EEProm CCK TxPower", priv->EE_TxPower_CCK, MAX_CCK_CHANNEL_NUM);
				debug_out("EEProm OFDM TxPower", priv->EE_TxPower_OFDM, MAX_OFDM_CHANNEL_NUM);
			}
			#endif

			// RF chip id
//			priv->EE_RFTypeID = (unsigned int)(READ_EEPROM((USHORT)(EEPROM_RF_CHIP_ID >> 1)) & 0x00f);
			priv->EE_RFTypeID = (unsigned int)(READ_EEPROM((USHORT)(0x28 >> 1)) & 0x80 ) >> 7;
			DEBUG_INFO("RF ID 0x%02X\n", (UCHAR)priv->EE_RFTypeID);

			// AnaParm
			usValue = READ_EEPROM((USHORT)((EEPROM_ANA_PARM + 2) >> 1));
			priv->EE_AnaParm = (unsigned int)(usValue << 16);
			usValue = READ_EEPROM((USHORT)(EEPROM_ANA_PARM >> 1));
			priv->EE_AnaParm |= usValue;
			DEBUG_INFO("AnaParm 0x%08X\n", priv->EE_AnaParm);
			usValue = READ_EEPROM((USHORT)((EEPROM_ANA_PARM2 + 2) >> 1));
			priv->EE_AnaParm2 = (unsigned int)(usValue << 16);
			usValue = READ_EEPROM((USHORT)(EEPROM_ANA_PARM2 >> 1));
			priv->EE_AnaParm2 |= usValue;
			DEBUG_INFO("AnaParm2 0x%08X\n", priv->EE_AnaParm2);
			//add CrystalCap, joshua 20080502
			priv->EE_CrystalCap = (((unsigned int) READ_EEPROM( 0x2A >> 1)) & 0xf000) >> 12;
			priv->pmib->dot11RFEntry.crystalCap = priv->EE_CrystalCap;
			DEBUG_INFO("CrystalCap 0x%08X\n", priv->EE_CrystalCap);
		}

		priv->EE_Cached = 1;
	}

	if ((data != NULL) && (!priv->EE_AutoloadFail))
	{
		switch(entry_id)
		{
		case EEPROM_RF_CHIP_ID:
			*((UCHAR *)data) = (UCHAR)priv->EE_RFTypeID;
			break;

		case EEPROM_NODE_ADDRESS_BYTE_0:
			memcpy(data, priv->EE_Mac, MACADDRLEN);
			break;

		case EEPROM_TX_POWER_LEVEL_0:
			memcpy(data, priv->EE_TxPower_CCK, MAX_CCK_CHANNEL_NUM);
			break;

		case EEPROM_11G_CHANNEL_OFDM_TX_POWER_LEVEL_OFFSET:
			memcpy(data, priv->EE_TxPower_OFDM, MAX_OFDM_CHANNEL_NUM);
			break;

		default:
			DEBUG_INFO("not support this id yet\n");
			return 0;
		}
		return 1;
	}
	else
		return 0;
}


int WriteAdapterInfo(struct rtl8192cd_priv *priv, int entry_id, void *data)
{
	USHORT			Index;
	USHORT          usValue;
	ULONG			curRCR;
	UCHAR			EepromAddressSize;

	priv->EE_Cached = 0;
	curRCR = RTL_R32(_RCR_);
	EepromAddressSize = (curRCR & _9356SEL_)? 8 : 6;

	switch(entry_id)
	{
	case EEPROM_TX_POWER_LEVEL_0:
		Index = ((USHORT)((int)data)) & 0xfffe;
		usValue = *((USHORT *)(&priv->EE_TxPower_CCK[Index]));
		WRITE_EEPROM((USHORT)((EEPROM_TX_POWER_LEVEL_0 + Index) >> 1), usValue);
		break;

	case EEPROM_11G_CHANNEL_OFDM_TX_POWER_LEVEL_OFFSET:
		Index = ((USHORT)((int)data)) & 0xfffe;
		usValue = *((USHORT *)(&priv->EE_TxPower_OFDM[Index]));
		WRITE_EEPROM((USHORT)((EEPROM_11G_CHANNEL_OFDM_TX_POWER_LEVEL_OFFSET + Index) >> 1), usValue);
		break;

	default:
		return 0;
	}
	return 1;
}
#endif

