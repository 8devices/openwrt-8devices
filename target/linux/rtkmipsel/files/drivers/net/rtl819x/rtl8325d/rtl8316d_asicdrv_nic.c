/*
* Copyright (C) 2009 Realtek Semiconductor Corp.
* All Rights Reserved.
*
* This program is the proprietary software of Realtek Semiconductor
* Corporation and/or its licensors, and only be used, duplicated,
* modified or distributed under the authorized license from Realtek.
*
* ANY USE OF THE SOFTWARE OTEHR THAN AS AUTHORIZED UNDER
* THIS LICENSE OR COPYRIGHT LAW IS PROHIBITED.
*
* $Revision: 8334 $
* $Date: 2010-02-09 13:22:42 +0800 (Tue, 09 Feb 2010) $
*
* Purpose : ASIC-level driver implementation for NIC.
*
*  Feature :  This file consists of following modules:
*             1)
*
*/

#include <rtk_types.h>
#include <rtl8316d_asicdrv_nic.h>
#include <asicdrv/rtl8316d_types.h>

/* This static variable is used to record the received packet sequence number.
 * It can be changed only in nic_init() and rxPkt() */
uint8 xdata rxSeq;

/* This static variable is used to record the transmitted packet sequence number.
 * It can be changed only in nic_init() and txPkt() */
uint8 xdata txSeq;

/* This static variable is used to record the information about
 * received packets. These information will be used when txPkt() is called */
rx_pktInfo_t xdata rxPktInfo;

/*
This API is only for memory controller and NIC register
*/
uint32 getReg32(uint16 address)
{
    return (XBYTE[address] | (XBYTE[address+1] << 8) | ((uint32)XBYTE[address+2] << 16)| ((uint32)XBYTE[address+3] << 24));
}

/*
This API is only for memory controller and NIC register
*/
void setReg32(uint16 address, uint32 value)
{
    XBYTE[address] = (uint8)value;
    XBYTE[address+1] = (uint8)(value>>8);
    XBYTE[address+2] = (uint8)(value>>16);
    XBYTE[address+3] = (uint8)(value>>24);
}
/*
This API is only for memory controller and NIC register
*/
uint16 getReg16(uint16 address)
{
    return (XBYTE[address] | (XBYTE[address+1] << 8));
}

/*
This API is only for memory controller and NIC register
*/
void setReg16(uint16 address, uint16 value)
{
    XBYTE[address] = (uint8)value;
    XBYTE[address+1] = (uint8)(value>>8);
}

/*
This API is only for memory controller and NIC register
*/
uint8 getReg8(uint16 address)
{
    return XBYTE[address];
}

/*
This API is only for memory controller and NIC register
*/
void setReg8(uint16 address, uint8 value)
{
    XBYTE[address] = value;
}


/* Function Name:
 *      rtl8316d_getSWReg
 * Description:
 *      Get switch register
 * Input:
 *      address   -  register address
 * Output:
 *      none
 * Return:
 *      the register value
 */
uint32 rtl8316d_getSWReg(uint32 address)
{
    setReg32(CPU_SRAAR_ADDR, address);
    setReg8(CPU_SRACR_ADDR, CPU_SRACR_TRIGGER_MASK| CPU_SRACR_TYPE_READ);
    while(1)
    {
        if ( (getReg8(CPU_SRACR_ADDR) & CPU_SRACR_TRIGGER_MASK) == 0)
            break;
    }
    return getReg32(CPU_SRADR_ADDR);
}

/* Function Name:
 *      rtl8316d_setSWReg
 * Description:
 *      Set switch register
 * Input:
 *      address   -  register address
 *      value      -  value to be set
 * Output:
 *      none
 * Return:
 *
 */
void rtl8316d_setSWReg(uint32 address, uint32 value)
{
    setReg32(CPU_SRAAR_ADDR, address);
    setReg32(CPU_SRADR_ADDR, value);
    setReg8(CPU_SRACR_ADDR, CPU_SRACR_TRIGGER_MASK| CPU_SRACR_TYPE_WRITE);
    while(1)
    {
        if ( (getReg8(CPU_SRACR_ADDR) & CPU_SRACR_TRIGGER_MASK) == 0)
            break;
    }
}

/* Function Name:
 *      rtl8316d_getSWRegBit
 * Description:
 *      Get switch register bit value
 * Input:
 *      address   -  register address
 *      pos        -   bit position
 * Output:
 *      none
 * Return:
 *      the bit value
 */
uint8 rtl8316d_getSWRegBit(uint32 address, uint8 pos)
{
    return (rtl8316d_getSWReg(address)>>pos)&0x1;
}

/* Function Name:
 *      rtl8316d_setSWRegBit
 * Description:
 *      Get switch register bit value
 * Input:
 *      address   -  register address
 *      pos        -   bit position
 *      value      -   value to be set
 * Output:
 *      none
 * Return:
 *      the bit value
 */
void rtl8316d_setSWRegBit(uint32 address, uint8 pos, uint8 val)
{
    uint32 value;
    value = rtl8316d_getSWReg(address);
    if (val)
    {
        value |= (0x1UL << pos);
    }else
    {
        value &= ~(0x1UL<<pos);
    }
    rtl8316d_setSWReg(address, value);
}

