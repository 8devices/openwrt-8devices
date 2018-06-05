/*
 * Copyright (C) 2009 Realtek Semiconductor Corp. 
 * All Rights Reserved.
 *
 * This program is the proprietary software of Realtek Semiconductor
 * Corporation and/or its licensors, and only be used, duplicated, 
 * modified or distributed under the authorized license from Realtek. 
 *
 * ANY USE OF THE SOFTWARE OTHER THAN AS AUTHORIZED UNDER 
 * THIS LICENSE OR COPYRIGHT LAW IS PROHIBITED. 
 *
 * $Revision: 14202 $
 * $Date: 2010-11-16 15:13:00 +0800 (Tue 16 Nov 2010) $
 *
 * Purpose : RTL8367B switch high-level API for RTL8367B
 * Feature : Miscellaneous functions
 *
 */

#include <rtl8367b_asicdrv_misc.h>
#include <rtl8367b_asicdrv_phy.h>

/* Function Name:
 *      rtl8367b_setAsicMacAddress
 * Description:
 *      Set switch MAC address
 * Input:
 *      mac 	- switch mac
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 		- Success
 *      RT_ERR_SMI  	- SMI access error
 * Note:
 *      None
 */
ret_t rtl8367b_setAsicMacAddress(ether_addr_t mac)
{
	ret_t retVal;
	rtk_uint32 regData;
	rtk_uint8 *accessPtr;
	rtk_uint32 i;

	accessPtr =  (rtk_uint8*)&mac;

	regData = *accessPtr;
	accessPtr ++;
	regData = (regData << 8) | *accessPtr;
	accessPtr ++;
	for(i = 0; i <=2; i++)
	{
		retVal = rtl8367b_setAsicReg(RTL8367B_REG_SWITCH_MAC2 - i, regData);
		if(retVal != RT_ERR_OK)
			return retVal;

		regData = *accessPtr;
		accessPtr ++;
		regData = (regData << 8) | *accessPtr;
		accessPtr ++;
	}

	return retVal;
}
/* Function Name:
 *      rtl8367b_getAsicMacAddress
 * Description:
 *      Get switch MAC address
 * Input:
 *      pMac 	- switch mac
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 		- Success
 *      RT_ERR_SMI  	- SMI access error
 * Note:
 *      None
 */
ret_t rtl8367b_getAsicMacAddress(ether_addr_t *pMac)
{
	ret_t retVal;
	rtk_uint32 regData;
	rtk_uint8 *accessPtr;
	rtk_uint32 i;


	accessPtr = (rtk_uint8*)pMac;

	for(i = 0; i <= 2; i++)
	{
		retVal = rtl8367b_getAsicReg(RTL8367B_REG_SWITCH_MAC2 - i, &regData);
		if(retVal != RT_ERR_OK)
			return retVal;

		*accessPtr = (regData & 0xFF00) >> 8;
		accessPtr ++;
		*accessPtr = regData & 0xFF;
		accessPtr ++;
	}

	return retVal;
}
/* Function Name:
 *      rtl8367b_getAsicDebugInfo
 * Description:
 *      Get per-port packet forward debugging information
 * Input:
 *      port 		- Physical port number (0~7)
 *      pDebugifo 	- per-port packet trap/drop/forward reason
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 		- Success
 *      RT_ERR_SMI  	- SMI access error
 *      RT_ERR_PORT_ID  - Invalid port number
 * Note:
 *      None
 */
ret_t rtl8367b_getAsicDebugInfo(rtk_uint32 port, rtk_uint32 *pDebugifo)
{
    if(port > RTL8367B_PORTIDMAX)
        return RT_ERR_PORT_ID;

    return rtl8367b_getAsicRegBits(RTL8367B_DEBUG_INFO_REG(port), RTL8367B_DEBUG_INFO_MASK(port), pDebugifo);
}
/* Function Name:
 *      rtl8367b_setAsicPortJamMode
 * Description:
 *      Set half duplex flow control setting
 * Input:
 *      mode 	- 0: Back-Pressure 1: DEFER
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 		- Success
 *      RT_ERR_SMI  	- SMI access error
 * Note:
 *      None
 */
ret_t rtl8367b_setAsicPortJamMode(rtk_uint32 mode)
{
	return rtl8367b_setAsicRegBit(RTL8367B_REG_CFG_BACKPRESSURE, RTL8367B_LONGTXE_OFFSET,mode);
}
/* Function Name:
 *      rtl8367b_getAsicPortJamMode
 * Description:
 *      Get half duplex flow control setting
 * Input:
 *      pMode 	- 0: Back-Pressure 1: DEFER
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 		- Success
 *      RT_ERR_SMI  	- SMI access error
 * Note:
 *      None
 */
ret_t rtl8367b_getAsicPortJamMode(rtk_uint32* pMode)
{
	return rtl8367b_getAsicRegBit(RTL8367B_REG_CFG_BACKPRESSURE, RTL8367B_LONGTXE_OFFSET, pMode);
}
/* Function Name:
 *      rtl8367b_setAsicMaxLengthInRx
 * Description:
 *      Set Max receiving packet length
 * Input:
 *      maxLength 	- 0: 1522 bytes 1:1536 bytes 2:1552 bytes 3:16000bytes
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 		- Success
 *      RT_ERR_SMI  	- SMI access error
 * Note:
 *      None
 */
ret_t rtl8367b_setAsicMaxLengthInRx(rtk_uint32 maxLength)
{	
	return rtl8367b_setAsicRegBits(RTL8367B_REG_MAX_LENGTH_LIMINT_IPG, RTL8367B_MAX_LENTH_CTRL_MASK, maxLength);
}
/* Function Name:
 *      rtl8367b_getAsicMaxLengthInRx
 * Description:
 *      Get Max receiving packet length
 * Input:
 *      pMaxLength 	- 0: 1522 bytes 1:1536 bytes 2:1552 bytes 3:16000bytes
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 		- Success
 *      RT_ERR_SMI  	- SMI access error
 * Note:
 *      None
 */
ret_t rtl8367b_getAsicMaxLengthInRx(rtk_uint32* pMaxLength)
{
	return rtl8367b_getAsicRegBits(RTL8367B_REG_MAX_LENGTH_LIMINT_IPG, RTL8367B_MAX_LENTH_CTRL_MASK, pMaxLength);
}
/* Function Name:
 *      rtl8367b_setAsicMaxLengthAltTxRx
 * Description:
 *      Set per-port Max receiving/transmit packet length in different speed
 * Input:
 *      maxLength 	- 0: 1522 bytes 1:1536 bytes 2:1552 bytes 3:16000bytes
 *      pmskGiga 	- enable port mask in 100Mpbs
 *      pmask100M 	- enable port mask in Giga
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 		- Success
 *      RT_ERR_SMI  	- SMI access error
 * Note:
 *      None
 */
ret_t rtl8367b_setAsicMaxLengthAltTxRx(rtk_uint32 maxLength, rtk_uint32 pmskGiga, rtk_uint32 pmask100M)
{	
	ret_t retVal;

	retVal = rtl8367b_setAsicReg(RTL8367B_REG_MAX_LENGTH_CFG, ((pmskGiga << RTL8367B_MAX_LENGTH_GIGA_OFFSET) & RTL8367B_MAX_LENGTH_GIGA_MASK) | (pmask100M & RTL8367B_MAX_LENGTH_10_100M_MASK));
	if(retVal != RT_ERR_OK)
		return retVal;
    
	return rtl8367b_setAsicRegBits(RTL8367B_REG_MAX_LEN_RX_TX, RTL8367B_MAX_LEN_RX_TX_MASK, maxLength);
}

/* Function Name:
 *      rtl8367b_getAsicMaxLengthAltTxRx
 * Description:
 *      Get per-port Max receiving/transmit packet length in different speed
 * Input:
 *      pMaxLength 	- 0: 1522 bytes 1:1536 bytes 2:1552 bytes 3:16000bytes
 *      pPmskGiga 	- enable port mask in 100Mpbs
 *      pPmask100M 	- enable port mask in Giga
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 		- Success
 *      RT_ERR_SMI  	- SMI access error
 * Note:
 *      None
 */
ret_t rtl8367b_getAsicMaxLengthAltTxRx(rtk_uint32* pMaxLength, rtk_uint32* pPmskGiga, rtk_uint32* pPmask100M)
{	
	ret_t retVal;
	rtk_uint32 regData;

	retVal = rtl8367b_getAsicReg(RTL8367B_REG_MAX_LENGTH_CFG, &regData);
	if(retVal != RT_ERR_OK)
		return retVal;

    *pPmskGiga = (regData & RTL8367B_MAX_LENGTH_GIGA_MASK) >> RTL8367B_MAX_LENGTH_GIGA_OFFSET;
    *pPmask100M = regData & RTL8367B_MAX_LENGTH_10_100M_MASK;

    return rtl8367b_getAsicRegBits(RTL8367B_REG_MAX_LEN_RX_TX, RTL8367B_MAX_LEN_RX_TX_MASK, pMaxLength);
}

#include <rtk_api.h>

rtk_api_ret_t rtk_port_phyMdx_set(rtk_port_t port, rtk_port_phy_mdix_mode_t mode)
{
    rtk_uint32          data, index,  reg;
    rtk_api_ret_t       retVal;
    rtk_uint32 regData;
    CONST_T rtk_uint16 regData1[][2] = { {0x201F,0x0007}, {0x201E,0x002D}, {0x2018,0xF010}, {0x201F,0x0000},
                                         {0x2010,0x0166}};
    CONST_T rtk_uint16 regData2[][2] = { {0x201F,0x0007}, {0x201E,0x002D}, {0x2018,0xF020}, {0x201F,0x0000},
                                         {0x2010,0x0166}};
    CONST_T rtk_uint16 regData3[][2] = { {0x201F,0x0007}, {0x201E,0x002D}, {0x2018,0xF020}, {0x201F,0x0000},
                                         {0x2010,0x0146}};

   if (port > RTK_PHY_ID_MAX)
        return RT_ERR_PORT_ID;

   if (mode >= PHY_FORCE_MODE_END)
       return RT_ERR_FAILED;

   if((retVal = rtl8367b_setAsicReg(0x13C2, 0x0249)) != RT_ERR_OK)
        return retVal;

   if((retVal = rtl8367b_getAsicReg(0x1300, &data)) != RT_ERR_OK)
        return retVal;

   if( (data == 0x0276) || (data == 0x0597) || (data == 0x6367) )
   {
        switch (mode)
        {
            case PHY_AUTO_CROSSOVER_MODE:
                if ((retVal = rtl8367b_getAsicPHYReg(port, 24, &regData))!=RT_ERR_OK)
                    return retVal;

                regData &= ~(0x0001 << 9);

                if ((retVal = rtl8367b_setAsicPHYReg(port, 24, regData))!=RT_ERR_OK)
                    return retVal;
                break;
            case PHY_FORCE_MDI_MODE:
                if ((retVal = rtl8367b_getAsicPHYReg(port, 24, &regData))!=RT_ERR_OK)
                    return retVal;

                regData |= (0x0001 << 9);
                regData |= (0x0001 << 8);

                if ((retVal = rtl8367b_setAsicPHYReg(port, 24, regData))!=RT_ERR_OK)
                    return retVal;
                break;
            case PHY_FORCE_MDIX_MODE:
                if ((retVal = rtl8367b_getAsicPHYReg(port, 24, &regData))!=RT_ERR_OK)
                    return retVal;

                regData |= (0x0001 << 9);
                regData &= ~(0x0001 << 8);

                if ((retVal = rtl8367b_setAsicPHYReg(port, 24, regData))!=RT_ERR_OK)
                    return retVal;
                break;
            default:
                return RT_ERR_INPUT;
                break;
        }
   }
   else
   {
        switch (mode)
        {
            case PHY_AUTO_CROSSOVER_MODE:
                for (index = 0; index < (sizeof(regData1) / ((sizeof(rtk_uint16))*2)); index++)
                {
                    reg = (regData1[index][0] - 0x2000) % 0x0020;
                    if ((retVal = rtl8367b_setAsicPHYReg(port, reg, regData1[index][1])) != RT_ERR_OK)
                        return retVal;
                }
                break;
            case PHY_FORCE_MDI_MODE:
                for (index = 0; index < (sizeof(regData2) / ((sizeof(rtk_uint16))*2)); index++)
                {
                    reg = (regData2[index][0] - 0x2000) % 0x0020;
                    if ((retVal = rtl8367b_setAsicPHYReg(port, reg, regData2[index][1])) != RT_ERR_OK)
                        return retVal;
                }
                break;
            case PHY_FORCE_MDIX_MODE:
                for (index = 0; index < (sizeof(regData3) / ((sizeof(rtk_uint16))*2)); index++)
                {
                    reg = (regData3[index][0] - 0x2000) % 0x0020;
                    if ((retVal = rtl8367b_setAsicPHYReg(port, reg, regData3[index][1])) != RT_ERR_OK)
                        return retVal;
                }
                break;
            default:
                return RT_ERR_INPUT;
                break;
        }
   }

   /* Restart N-way */
    if ((retVal = rtl8367b_getAsicPHYReg(port, 0, &regData))!=RT_ERR_OK)
        return retVal;

    regData |= (0x0001 << 9);

    if ((retVal = rtl8367b_setAsicPHYReg(port, 0, regData))!=RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

rtk_api_ret_t rtk_port_phyMdx_get(rtk_port_t port, rtk_port_phy_mdix_mode_t *pMode)
{
    rtk_uint32          data;
    rtk_api_ret_t       retVal;
    rtk_uint32 regData;

    if (port > RTK_PHY_ID_MAX)
        return RT_ERR_PORT_ID;

    if ((retVal = rtl8367b_setAsicReg(0x13C2, 0x0249)) != RT_ERR_OK)
        return retVal;

    if ((retVal = rtl8367b_getAsicReg(0x1300, &data)) != RT_ERR_OK)
        return retVal;

    if ( (data == 0x0276) || (data == 0x0597) || (data == 0x6367) )
    {
        if ((retVal = rtl8367b_getAsicPHYReg(port, 24, &regData))!=RT_ERR_OK)
            return retVal;

        if (regData & (0x0001 << 9))
        {
            if (regData & (0x0001 << 8))
                *pMode = PHY_FORCE_MDI_MODE;
            else
                *pMode = PHY_FORCE_MDIX_MODE;
        }
        else
            *pMode = PHY_AUTO_CROSSOVER_MODE;
    }
    else
    {
        if ((retVal = rtl8367b_setAsicPHYReg(port, 0x1F, 0x7)) != RT_ERR_OK)
            return retVal;

        if ((retVal = rtl8367b_setAsicPHYReg(port, 0x1E, 0x2D)) != RT_ERR_OK)
            return retVal;

        if ((retVal = rtl8367b_getAsicPHYReg(port, 0x18, &data)) != RT_ERR_OK)
            return retVal;

        if ((retVal = rtl8367b_setAsicPHYReg(port, 0x1F, 0)) != RT_ERR_OK)
            return retVal;

        if (data&0x0020)
        {
            if ((retVal = rtl8367b_getAsicPHYReg(port, 0x10, &data)) != RT_ERR_OK)
                return retVal;

            if (data&0x0020)
                *pMode = PHY_FORCE_MDI_MODE;
            else
                *pMode = PHY_FORCE_MDIX_MODE;
        }
        else
            *pMode = PHY_AUTO_CROSSOVER_MODE;
    }

     return RT_ERR_OK;
}
