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
* $Revision:  $
* $Date: $
*
* Purpose : ASIC-level driver implementation for RealTeK Proprietary Protocol.
*
*  Feature :  This file consists of following modules:
*             1) 
*
*/

#include <rtk_types.h>
#include <rtk_api.h>
#include <rtl8316d_asicdrv_rtkpp.h>
#include <asicdrv/rtl8316d_types.h> /*SDK*/
#include <rtl8316d_reg_struct.h>



/* Function Name:
 *      rtk_rldp_loopPortmask_get
 * Description:
 *      Get looped port mask
 * Input:
 *      None
 * Output:
 *      loopedPortmask  - Shared meter index
 * Return:
 *      RT_ERR_OK - Success.
 *      RT_ERR_FAILED  - Failure.
 *      RT_ERR_FILTER_METER_ID - Invalid meter
 *      RT_ERR_RATE - Invalid rate
 *      RT_ERR_INPUT - Invalid input parameters.
 * Note:
 *      The API can get looped port mask.
 */
rtk_api_ret_t rtk_rldp_loopPortmask_get(rtk_portmask_t* loopedPortmask)
{
    rtk_api_ret_t retVal;
    
    if (loopedPortmask == NULL)
    {
        return RT_ERR_NULL_POINTER;
    }

   retVal = reg_field_read(RTL8316D_UNIT, RLDP_LOOP_PORT_MASK_CONTROL, RLDP_LOOP_PMSK, loopedPortmask->bits);

    return retVal;
}


/* Function Name:
 *      rtk_rldp_loopedPortPair_get
 * Description:
 *      Get looped port pair
 * Input:
 *      port - port ID
 * Output:
 *      portPair  - port ID that is looped with the given port
 * Return:
 *      RT_ERR_OK - Success.
 *      RT_ERR_FAILED  - Failure.
 *      RT_ERR_NULL_POINTER - Invalid pointer input.
 *      RT_ERR_PORT_ID - Invalid port ID input
 *      RT_ERR_INPUT - Invalid input parameters.
 * Note:
 *      The API can get looped port pair.  
 */
rtk_api_ret_t rtk_rldp_loopedPortPair_get(rtk_port_t port, rtk_port_t *portPair)
{
     rtk_api_ret_t retVal;
     
     if (portPair == NULL)
     {
         return RT_ERR_NULL_POINTER;
     }

     if(port >= RTL8316D_MAX_PORT)
         return RT_ERR_PORT_ID;
    
     retVal = reg_field_read(RTL8316D_UNIT, PORT0_LOOP_PORT+port, LOOPED_PORT, portPair);
    
     return retVal;
}

/* Function Name:
 *      rtk_rldp_rxDisPortMsk_get
 * Description:
 *      Get Rx disabled port mask
 * Input:
 *      None
 * Output:
 *      portMask  - 
 * Return:
 *      RT_ERR_OK - Success.
 *      RT_ERR_FAILED  - Failure.
 *      RT_ERR_NULL_POINTER - Invalid pointer input.
 * Note:
 *      The API can retrieve rx disabled port mask.  
 */
rtk_api_ret_t rtk_rldp_rxDisPortMsk_get(rtk_portmask_t* portMask)
{
     rtk_api_ret_t retVal;
     
     if (portMask == NULL)
     {
         return RT_ERR_NULL_POINTER;
     }
   
     retVal = reg_read(RTL8316D_UNIT, RX_DISABLE_GLOBAL_CONTROL, portMask->bits);
    
     return retVal;
}


/* Function Name:
 *      rtk_rldp_rxDisPortMsk_set
 * Description:
 *      Set Rx disabled port mask
 * Input:
 *      pPortMask - 
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK - Success.
 *      RT_ERR_FAILED  - Failure.
 *      RT_ERR_NULL_POINTER - Invalid pointer input.
 *      RT_ERR_INPUT - Invalid port mask.
 * Note:
 *      The API can set tx disabled port mask.  
 */
rtk_api_ret_t rtk_rldp_rxDisPortMsk_set(rtk_portmask_t* pPortMask)
{
     rtk_api_ret_t retVal;
     uint32 regVal;
     
     if (pPortMask == NULL)
     {
         return RT_ERR_NULL_POINTER;
     }

     if (pPortMask->bits[0] >= (1 << RTL8316D_MAX_PORT))
    {
        return RT_ERR_INPUT;
    }

     retVal = reg_read(RTL8316D_UNIT, RX_DISABLE_GLOBAL_CONTROL, &regVal);
     if (retVal != SUCCESS)
     {
        return retVal;
     }

     regVal &= ~pPortMask->bits[0];
     regVal |= pPortMask->bits[0];
    
     retVal = reg_write(RTL8316D_UNIT, RX_DISABLE_GLOBAL_CONTROL, regVal);
    
     return retVal;
}

/* Function Name:
 *      rtk_rldp_txDisPortMsk_get
 * Description:
 *      Get Rx disabled port mask
 * Input:
 *      None
 * Output:
 *      pPortMask - 
 * Return:
 *      RT_ERR_OK - Success.
 *      RT_ERR_FAILED  - Failure.
 *      RT_ERR_NULL_POINTER - Invalid pointer input.
 * Note:
 *      The API can retrieve tx disabled port mask.  
 */
rtk_api_ret_t rtk_rldp_txDisPortMsk_get(rtk_portmask_t* pPortMask)
{
     rtk_api_ret_t retVal;
     
     if (pPortMask == NULL)
     {
         return RT_ERR_NULL_POINTER;
     }
   
     retVal = reg_read(RTL8316D_UNIT, TX_DISABLE_GLOBAL_CONTROL, pPortMask->bits);
    
     return retVal;
}


/* Function Name:
 *      rtk_rldp_txDisPortMsk_set
 * Description:
 *      Set Tx disabled port mask
 * Input:
 *      pPortMask - 
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK - Success.
 *      RT_ERR_FAILED  - Failure.
 *      RT_ERR_NULL_POINTER - Invalid pointer input.
 * Note:
 *      The API can configure tx disabled port mask. 
 */
rtk_api_ret_t rtk_rldp_txDisPortMsk_set(rtk_portmask_t* pPortMask)
{
     rtk_api_ret_t retVal;
     uint32 regVal;
     
     if (pPortMask == NULL)
     {
         return RT_ERR_NULL_POINTER;
     }

     retVal = reg_read(RTL8316D_UNIT, TX_DISABLE_GLOBAL_CONTROL, &regVal);
     if (retVal != SUCCESS)
     {
        return retVal;
     }

     regVal &= ~pPortMask->bits[0];
     regVal |= pPortMask->bits[0];
    
     retVal = reg_write(RTL8316D_UNIT, RX_DISABLE_GLOBAL_CONTROL, regVal);
    
     return retVal;
}

/* Function Name:
 *      rtk_rldp_cpuTrigger
 * Description:
 *      Trigger all RLDP enabled port to enter the checking state
 * Input:
 *      pPortMask - 
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK - Success.
 * Note:
 *      The API can trigger all RLDP enabled port to enter the checking state.
 */
rtk_api_ret_t rtk_rldp_cpuTrigger(void)
{

    reg_field_write(RTL8316D_UNIT, RLDP_GLOBAL_CONTROL, RLDP_CPUTRIG, 1);

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_rldp_linkupTrig_set
 * Description:
 *      Enable/disable RLDP when port links up
 * Input:
 *      pPortMask - 
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK - Success.
 *      RT_ERR_FAILED  - Failure.
 *      RT_ERR_ENABLE - value of enable should be either DISABLED or ENABLED.
 * Note:
 *      The API can enable/disable RLDP when port links up.
 */
rtk_api_ret_t rtk_rldp_linkupTrig_set(rtk_enable_t enable)
{
    rtk_api_ret_t retVal;

    if (enable >= RTK_ENABLE_END)
    {
        return RT_ERR_ENABLE;
    }

    retVal = reg_field_write(RTL8316D_UNIT, RLDP_GLOBAL_CONTROL, RLDP_AUTOTRIGGER, enable);

    return retVal;
}

/* Function Name:
 *      rtk_rldp_linkupTrig_get
 * Description:
 *     Get RLDP operation mode
 * Input:
 *      None
 * Output:
 *      pEnable - 
 * Return:
 *      RT_ERR_OK - Success.
 *      RT_ERR_FAILED  - Failure.
 *      RT_ERR_NULL_POINTER - NULL pointer input.
 * Note:
 *      The API can get RLDP operation mode
 */
rtk_api_ret_t rtk_rldp_linkupTrig_get(rtk_enable_t* pEnable)
{
    rtk_api_ret_t retVal;

    if (pEnable == NULL)
    {
        return RT_ERR_NULL_POINTER;
    }

    retVal = reg_field_read(RTL8316D_UNIT, RLDP_GLOBAL_CONTROL, RLDP_AUTOTRIGGER, pEnable);

    return retVal;
}

/* Function Name:
 *      rtk_rldp_vlanLeaky_set
 * Description:
 *      Enable/disable RLDP VLAN leaky
 * Input:
 *      enable - ENABLED/DISABLED
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK - Success.
 *      RT_ERR_FAILED  - Failure.
 *      RT_ERR_ENABLE - value of enable should be either DISABLED or ENABLED.
 * Note:
 *      The API can enable/disable RLDP VLAN leaky
 */
rtk_api_ret_t rtk_rldp_vlanLeaky_set(rtk_enable_t enable)
{
    rtk_api_ret_t retVal;

    if (enable >= RTK_ENABLE_END)
    {
        return RT_ERR_ENABLE;
    }

    retVal = reg_field_write(RTL8316D_UNIT, RLDP_GLOBAL_CONTROL, RLDP_VLEAKY, enable);

    return retVal;
}

/* Function Name:
 *      rtk_rldp_vlanLeaky_get
 * Description:
 *      Get RLDP VLAN leaky mode
 * Input:
 *      None
 * Output:
 *      pEnable - ENABLED/DISABLED
 * Return:
 *      RT_ERR_OK - Success.
 *      RT_ERR_FAILED  - Failure.
 *      RT_ERR_NULL_POINTER - NULL pointer input.
 * Note:
 *     The API can get RLDP VLAN leaky mode
 */
rtk_api_ret_t rtk_rldp_vlanLeaky_get(rtk_enable_t* pEnable)
{
    rtk_api_ret_t retVal;

    if (pEnable == NULL)
    {
        return RT_ERR_NULL_POINTER;
    }

    retVal = reg_field_read(RTL8316D_UNIT, RLDP_GLOBAL_CONTROL, RLDP_VLEAKY, pEnable);

    return retVal;
}

/* Function Name:
 *      rtk_rldp_autoBlock_set
 * Description:
 *      Enable/disable RLDP auto blocking when loop occurs
 * Input:
 *      enable - ENABLED/DISABLED
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK - Success.
 *      RT_ERR_FAILED  - Failure.
 *      RT_ERR_ENABLE - value of enable should be either DISABLED or ENABLED.
 * Note:
 *     The API can enable/disable RLDP auto blocking when loop occurs.
 */
rtk_api_ret_t rtk_rldp_autoBlock_set(rtk_enable_t enable)
{
    rtk_api_ret_t retVal;

    if (enable >= RTK_ENABLE_END)
    {
        return RT_ERR_ENABLE;
    }

    retVal = reg_field_write(RTL8316D_UNIT, RLDP_GLOBAL_CONTROL, RLDP_AUTOBLK, enable);

    return retVal;
}


/* Function Name:
 *      rtk_rldp_autoBlock_get
 * Description:
 *      Get RLDP auto block configuration
 * Input:
 *      None
 * Output:
 *      pEnable - ENABLED/DISABLED
 * Return:
 *      RT_ERR_OK - Success.
 *      RT_ERR_FAILED  - Failure.
 *      RT_ERR_NULL_POINTER - NULL pointer input.
 * Note:
 *     The API can get RLDP auto block configuration
 */
rtk_api_ret_t rtk_rldp_autoBlock_get(rtk_enable_t* pEnable)
{
    rtk_api_ret_t retVal;

    if (pEnable == NULL)
    {
        return RT_ERR_NULL_POINTER;
    }

    retVal = reg_field_read(RTL8316D_UNIT, RLDP_GLOBAL_CONTROL, RLDP_AUTOBLK, pEnable);

    return retVal;
}



/* Function Name:
 *      rtk_rldp_trap_set
 * Description:
 *      Configure to trap RLDP packets to CPU or not
 * Input:
 *      enable - ENABLED/DISABLED
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK - Success.
 *      RT_ERR_FAILED  - Failure.
 *      RT_ERR_ENABLE - value of enable should be either DISABLED or ENABLED.
 * Note:
 *     The API can configure the status of whether RLDP packets would be trapped or not.
 */
rtk_api_ret_t rtk_rldp_trap_set(rtk_enable_t enable)
{
    rtk_api_ret_t retVal;

    if (enable >= RTK_ENABLE_END)
    {
        return RT_ERR_ENABLE;
    }

    retVal = reg_field_write(RTL8316D_UNIT, RLDP_GLOBAL_CONTROL, RLDP_8051_ENABLE, enable);

    return retVal;
}

/* Function Name:
 *      rtk_rldp_trap_get
 * Description:
 *      Get RLDP trapped status
 * Input:
 *      None
 * Output:
 *      pEnable - ENABLED/DISABLED
 * Return:
 *      RT_ERR_OK - Success.
 *      RT_ERR_FAILED  - Failure.
 *      RT_ERR_NULL_POINTER - NULL pointer input.
 * Note:
 *     The API can retrieve the status of whether RLDP packets would be trapped or not.
 */
rtk_api_ret_t rtk_rldp_trap_get(rtk_enable_t* pEnable)
{
    rtk_api_ret_t retVal;

    if (pEnable == NULL)
    {
        return RT_ERR_NULL_POINTER;
    }

    retVal = reg_field_read(RTL8316D_UNIT, RLDP_GLOBAL_CONTROL, RLDP_8051_ENABLE, pEnable);

    return retVal;
}

/* Function Name:
 *      rtk_rldp_chkRetryCnt_set
 * Description:
 *      Configure RLDP retry count in checking state
 * Input:
 *      nr_retry - retry count
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK - Success.
 *      RT_ERR_FAILED  - Failure.
 * Note:
 *     The API can configure RLDP retry count in checking state.
 */
rtk_api_ret_t rtk_rldp_chkRetryCnt_set(uint8 nr_retry)
{
    rtk_api_ret_t retVal;

    retVal = reg_field_write(RTL8316D_UNIT, RLDP_CHECK_STATE_RETRY_CONTROL, RETRY_COUNT, nr_retry);
    return retVal;
}

/* Function Name:
 *      rtk_rldp_chkRetryCnt_get
 * Description:
 *      Retrieve RLDP retry count in checking state
 * Input:
 *      None
 * Output:
 *      pNr_retry - retry count
 * Return:
 *      RT_ERR_OK - Success.
 *      RT_ERR_FAILED  - Failure.
 *      RT_ERR_NULL_POINTER - NULL pointer input.
 * Note:
 *     The API can retrieve RLDP retry count in checking state.
 */
rtk_api_ret_t rtk_rldp_chkRetryCnt_get(uint8* pNr_retry)
{
    rtk_api_ret_t retVal;
    uint32 regVal;
    
    if (pNr_retry == NULL)
        return RT_ERR_NULL_POINTER;
    
    retVal = reg_field_read(RTL8316D_UNIT, RLDP_CHECK_STATE_RETRY_CONTROL, RETRY_COUNT, &regVal);

    *pNr_retry = regVal & 0xff;
    
    return retVal;
}

/* Function Name:
 *      rtk_rldp_chkRetryInterval_set
 * Description:
 *      Configure RLDP retry interval in checking state
 * Input:
 *      None
 * Output:
 *      pNr_retry - retry count
 * Return:
 *      RT_ERR_OK - Success.
 *      RT_ERR_FAILED  - Failure.
 * Note:
 *     The API can configure RLDP retry interval in checking state.
 */
rtk_api_ret_t rtk_rldp_chkRetryInterval_set(uint16 interval)
{
    rtk_api_ret_t retVal;

    retVal = reg_field_write(RTL8316D_UNIT, RLDP_CHECK_STATE_RETRY_CONTROL, RETRY_PERIOD, interval);
    return retVal;
}

/* Function Name:
 *      rtk_rldp_chkRetryInterval_get
 * Description:
 *      Retrieve RLDP retry interval in checking state
 * Input:
 *      None
 * Output:
 *      pNr_retry - retry count
 * Return:
 *      RT_ERR_OK - Success.
 *      RT_ERR_FAILED  - Failure.
 *      RT_ERR_NULL_POINTER - NULL pointer input.
 * Note:
 *     The API can retrieve RLDP retry interval in checking state.
 */
rtk_api_ret_t rtk_rldp_chkRetryInterval_get(uint16* pInterval)
{
    rtk_api_ret_t retVal;
    uint32 regVal;
    
    if (pInterval == NULL)
        return RT_ERR_NULL_POINTER;
    
    retVal = reg_field_read(RTL8316D_UNIT, RLDP_CHECK_STATE_RETRY_CONTROL, RETRY_PERIOD, &regVal);

    *pInterval = regVal & 0xff;
    
    return retVal;
}


/* Function Name:
 *      rtk_rldp_loopRetryCnt_set
 * Description:
 *      Configure RLDP retry count in loop state
 * Input:
 *      nr_retry - retry count
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK - Success.
 *      RT_ERR_FAILED  - Failure.
 * Note:
 *     The API can configure RLDP retry count in loop state.
 */
rtk_api_ret_t rtk_rldp_loopRetryCnt_set(uint8 nr_retry)
{
    rtk_api_ret_t retVal;

    retVal = reg_field_write(RTL8316D_UNIT, RLDP_LOOP_STATE_RETRY_CONTROL, RETRY_COUNT, nr_retry);
    return retVal;
}

/* Function Name:
 *      rtk_rldp_loopRetryCnt_get
 * Description:
 *      Retrieve RLDP retry count in loop state
 * Input:
 *      None
 * Output:
 *      pNr_retry - retry count
 * Return:
 *      RT_ERR_OK - Success.
 *      RT_ERR_FAILED  - Failure.
 *      RT_ERR_NULL_POINTER - NULL pointer input.
 * Note:
 *     The API can retrieve RLDP retry count in loop state.
 */
rtk_api_ret_t rtk_rldp_loopRetryCnt_get(uint8* pNr_retry)
{
    rtk_api_ret_t retVal;
    uint32 regVal;
    
    if (pNr_retry == NULL)
        return RT_ERR_NULL_POINTER;
    
    retVal = reg_field_read(RTL8316D_UNIT, RLDP_LOOP_STATE_RETRY_CONTROL, RETRY_COUNT, &regVal);

    *pNr_retry = regVal & 0xff;
    
    return retVal;
}

/* Function Name:
 *      rtk_rldp_loopRetryInterval_set
 * Description:
 *      Configure RLDP retry interval in loop state
 * Input:
 *      interval - retry interval
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK - Success.
 *      RT_ERR_FAILED  - Failure.
 * Note:
 *     The API can configure RLDP retry interval in loop state.
 */
rtk_api_ret_t rtk_rldp_loopRetryInterval_set(uint16 interval)
{
    rtk_api_ret_t retVal;

    retVal = reg_field_write(RTL8316D_UNIT, RLDP_LOOP_STATE_RETRY_CONTROL, RETRY_PERIOD, interval);
    return retVal;
}

/* Function Name:
 *      rtk_rldp_loopRetryInterval_get
 * Description:
 *      Retrieve RLDP retry interval in loop state
 * Input:
 *      None
 * Output:
 *      pInterval - retry interval
 * Return:
 *      RT_ERR_OK - Success.
 *      RT_ERR_FAILED  - Failure.
 *      RT_ERR_NULL_POINTER - NULL pointer input.
 * Note:
 *     The API can retrieve RLDP retry interval in loop state.
 */
rtk_api_ret_t rtk_rldp_loopRetryInterval_get(uint16* pInterval)
{
    rtk_api_ret_t retVal;
    uint32 regVal;
    
    if (pInterval == NULL)
        return RT_ERR_NULL_POINTER;
    
    retVal = reg_field_read(RTL8316D_UNIT, RLDP_LOOP_STATE_RETRY_CONTROL, RETRY_PERIOD, &regVal);

    *pInterval = regVal & 0xff;
    
    return retVal;
}


/* Function Name:
 *      rtk_rldp_seed_set
 * Description:
 *      Configure seed for random number
 * Input:
 *      pSeed
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK - Success.
 *      RT_ERR_FAILED  - Failure.
 *      RT_ERR_NULL_POINTER - NULL pointer input.
 * Note:
 *     The API can configure seed to generate random number.
 */
rtk_api_ret_t rtk_rldp_seed_set(rtk_rtkpp_seed_t* pSeed)
{
    rtk_api_ret_t retVal;

    if (pSeed == NULL)
    {
        return RT_ERR_NULL_POINTER;
    }

    retVal = reg_field_write(RTL8316D_UNIT, RTKPP_RANDOM_NUMBER_CONTROL1, RTKPP_SEED15_0, pSeed->seed[0] | pSeed->seed[1] << 16);
    if (retVal != SUCCESS)
        return retVal;
    
    retVal = reg_field_write(RTL8316D_UNIT, RTKPP_RANDOM_NUMBER_CONTROL2, RTKPP_SEED47_16, 
                pSeed->seed[2] | pSeed->seed[3] << 8 | pSeed->seed[4] << 16 | pSeed->seed[5] << 24);

    return retVal;
}

/* Function Name:
 *      rtk_rldp_seed_get
 * Description:
 *      Get seed for random number
 * Input:
 *      None
 * Output:
 *      pSeed
 * Return:
 *      RT_ERR_OK - Success.
 *      RT_ERR_FAILED  - Failure.
 *      RT_ERR_NULL_POINTER - NULL pointer input.
 * Note:
 *     The API can get seed for generating random number.
 */
rtk_api_ret_t rtk_rldp_seed_get(rtk_rtkpp_seed_t* pSeed)
{
    rtk_api_ret_t retVal;
    uint32 regVal;

    if (pSeed == NULL)
    {
        return RT_ERR_NULL_POINTER;
    }

    retVal = reg_field_read(RTL8316D_UNIT, RTKPP_RANDOM_NUMBER_CONTROL1, RTKPP_SEED15_0, &regVal);
    if (retVal != SUCCESS)
        return retVal;
    pSeed->seed[0] = regVal & 0xFF;
    pSeed->seed[1] = (regVal >> 8) & 0xFF;
    
    retVal = reg_field_read(RTL8316D_UNIT, RTKPP_RANDOM_NUMBER_CONTROL2, RTKPP_SEED47_16, &regVal);
    pSeed->seed[2] = regVal & 0xFF;
    pSeed->seed[3] = (regVal >> 8) & 0xFF;
    pSeed->seed[4] = (regVal >> 16) & 0xFF;
    pSeed->seed[5] = (regVal >> 24) & 0xFF;

    return retVal;
}

/* Function Name:
 *      rtk_rldp_randNum_get
 * Description:
 *      Get random number
 * Input:
 *      None
 * Output:
 *      pSeed
 * Return:
 *      RT_ERR_OK - Success.
 *      RT_ERR_FAILED  - Failure.
 *      RT_ERR_NULL_POINTER - NULL pointer input.
 * Note:
 *     The API can get random number.
 */
rtk_api_ret_t rtk_rldp_randNum_get(rtk_rtkpp_randomNum_t* pRandNum)
{
    rtk_api_ret_t retVal;
    uint32 regVal;

    if (pRandNum == NULL)
    {
        return RT_ERR_NULL_POINTER;
    }

    retVal = reg_field_read(RTL8316D_UNIT, RTKPP_RANDOM_NUMBER_CONTROL0, RTKPP_RANDOM_NUMBER31_0, &regVal);
    if (retVal != SUCCESS)
        return retVal;
    pRandNum->number[0] = regVal & 0xFF;
    pRandNum->number[1] = (regVal >> 8) & 0xFF;
    pRandNum->number[2] = (regVal >> 16) & 0xFF;
    pRandNum->number[3] = (regVal >> 24) & 0xFF;
    
    retVal = reg_field_read(RTL8316D_UNIT, RTKPP_RANDOM_NUMBER_CONTROL1, RTKPP_RANDOM_NUMBER47_32, &regVal);
    pRandNum->number[4] = regVal & 0xFF;
    pRandNum->number[5] = (regVal >> 8) & 0xFF;

    return retVal;
}


/* Function Name:
 *      rtk_udld_enable_set
 * Description:
 *      set UDLD enable port mask
 * Input:
 *      loopedPortmask - UDLD enabled port mask
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK - Success.
 *      RT_ERR_FAILED  - Failure.
 *      RT_ERR_NULL_POINTER - NULL pointer input.
 * Note:
 *     The API can set UDLD enabled port mask  
 */
rtk_api_ret_t rtk_udld_enable_set(rtk_portmask_t* loopedPortmask)
{
     rtk_api_ret_t retVal;
     uint32 regVal;
     
     if (loopedPortmask == NULL)
     {
         return RT_ERR_NULL_POINTER;
     }
    
     retVal = reg_field_read(RTL8316D_UNIT, UDLD_ENABLED_PORT_MASK_CONTROL, ENABLE_PMSK, &regVal);
     if (retVal != SUCCESS)
     {
        return retVal;
     }

     retVal |= loopedPortmask->bits[0];
     retVal = reg_field_write(RTL8316D_UNIT, UDLD_ENABLED_PORT_MASK_CONTROL, ENABLE_PMSK, regVal);
    
     return retVal;
}

/* Function Name:
 *      rtk_udld_enable_get
 * Description:
 *      Get UDLD enable port mask
 * Input:
 *      None
 * Output:
 *      loopedPortmask - UDLD enabled port mask
 * Return:
 *      RT_ERR_OK - Success.
 *      RT_ERR_FAILED  - Failure.
 *      RT_ERR_NULL_POINTER - NULL pointer input.
 * Note:
 *     The API can Get UDLD enabled port mask  
 */
rtk_api_ret_t rtk_udld_enable_get(rtk_portmask_t* loopedPortmask)
{
     rtk_api_ret_t retVal;
     uint32 regVal;
     
     if (loopedPortmask == NULL)
     {
         return RT_ERR_NULL_POINTER;
     }
    
     retVal = reg_field_read(RTL8316D_UNIT, UDLD_ENABLED_PORT_MASK_CONTROL, ENABLE_PMSK, &regVal);
     if (retVal != SUCCESS)
     {
        return retVal;
     }

    loopedPortmask->bits[0] = regVal;

     return retVal;
}

/* Function Name:
 *      rtk_udld_mode_set
 * Description:
 *      Set UDLD operation mode
 * Input:
 *      mode - operation mode
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK - Success.
 *      RT_ERR_FAILED  - Failure.
 * Note:
 *     The API can configure UDLD operation mode
 */
rtk_api_ret_t rtk_udld_mode_set(rtk_rtkpp_udldMode_t mode)
{
    rtk_api_ret_t retVal;

    if (mode >= UDLDMODE_END)
    {
        return RT_ERR_FAILED;
    }

    retVal = reg_field_write(RTL8316D_UNIT, UDLD_GLOBAL_CONTROL, UDLD_HEARTBEAT, mode);

    return retVal;
}

/* Function Name:
 *      rtk_udld_mode_get
 * Description:
 *      Get UDLD operation mode
 * Input:
 *      None
 * Output:
 *      pMode - operation mode
 * Return:
 *      RT_ERR_OK - Success.
 *      RT_ERR_FAILED  - Failure.
 *      RT_ERR_NULL_POINTER - NULL pointer input.
 * Note:
 *     The API can retrieve UDLD operation mode
 */
rtk_api_ret_t rtk_udld_mode_get(rtk_rtkpp_udldMode_t* pMode)
{
    rtk_api_ret_t retVal;

    if (pMode == NULL)
    {
        return RT_ERR_NULL_POINTER;
    }

    retVal = reg_field_read(RTL8316D_UNIT, UDLD_GLOBAL_CONTROL, UDLD_HEARTBEAT, pMode);

    return retVal;
}

/* Function Name:
 *      rtk_udld_cpuTrigger
 * Description:
 *      Trigger all UDLD enabled port to enter the checking state
 * Input:
 *      None
 * Output:
 *      pMode - operation mode
 * Return:
 *      RT_ERR_OK - Success.
 * Note:
 *     The API can trigger all UDLD enabled port to enter the checking state
 */
rtk_api_ret_t rtk_udld_cpuTrigger(void)
{

    reg_field_write(RTL8316D_UNIT, UDLD_GLOBAL_CONTROL, UDLD_CPUTRIG, 1);

    return RT_ERR_OK;
}


/* Function Name:
 *      rtk_udld_linkupTrig_set
 * Description:
 *      Enable/disable UDLD when port links up
 * Input:
 *      enable - ENABLED/DISABLED
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK - Success.
 *      RT_ERR_FAILED  - Failure.
 *      RT_ERR_ENABLE - value of enable should be ENABLED or DISABLED
 * Note:
 *     The API can enable/disable UDLD when port links up
 */
rtk_api_ret_t rtk_udld_linkupTrig_set(rtk_enable_t enable)
{
    rtk_api_ret_t retVal;

    if (enable >= RTK_ENABLE_END)
    {
        return RT_ERR_ENABLE;
    }

    retVal = reg_field_write(RTL8316D_UNIT, UDLD_GLOBAL_CONTROL, UDLD_AUTOTRIG, enable);

    return retVal;
}

/* Function Name:
 *      rtk_udld_linkupTrig_get
 * Description:
 *      Get UDLD operation mode
 * Input:
 *      None
 * Output:
 *      None
 *      pEnable - ENABLED/DISABLED
 * Return:
 *      RT_ERR_OK - Success.
 *      RT_ERR_FAILED  - Failure.
 *      RT_ERR_NULL_POINTER - NULL pointer input.
 * Note:
 *     The API get configure UDLD operation mode
 */
rtk_api_ret_t rtk_udld_linkupTrig_get(rtk_enable_t* pEnable)
{
    rtk_api_ret_t retVal;

    if (pEnable == NULL)
    {
        return RT_ERR_NULL_POINTER;
    }

    retVal = reg_field_read(RTL8316D_UNIT, UDLD_GLOBAL_CONTROL, UDLD_AUTOTRIG, pEnable);

    return retVal;
}

/* Function Name:
 *      rtk_udld_trap_set
 * Description:
 *      Configure to trap UDLD packets to CPU or not
 * Input:
 *      enable - ENABLED/DISABLED
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK - Success.
 *      RT_ERR_FAILED  - Failure.
 *      RT_ERR_ENABLE - value of enable should be either ENABLED or DISABLED.
 * Note:
 *     The API can configure the status of whether UDLD packets would be trapped or not.
 */
rtk_api_ret_t rtk_udld_trap_set(rtk_enable_t enable)
{
    rtk_api_ret_t retVal;

    if (enable >= RTK_ENABLE_END)
    {
        return RT_ERR_ENABLE;
    }

    retVal = reg_field_write(RTL8316D_UNIT, UDLD_GLOBAL_CONTROL, UDLD_8051_ENABLE, enable);

    return retVal;
}

/* Function Name:
 *      rtk_udld_trap_get
 * Description:
 *      Get UDLD trapped status
 * Input:
 *      None
 * Output:
 *      pEnable - ENABLED/DISABLED
 * Return:
 *      RT_ERR_OK - Success.
 *      RT_ERR_FAILED  - Failure.
 *      RT_ERR_NULL_POINTER - NULL pointer input.
 * Note:
 *     The API can retrieve the status of whether UDLD packets would be trapped or not.
 */
rtk_api_ret_t rtk_udld_trap_get(rtk_enable_t* pEnable)
{
    rtk_api_ret_t retVal;

    if (pEnable == NULL)
    {
        return RT_ERR_NULL_POINTER;
    }

    retVal = reg_field_read(RTL8316D_UNIT, UDLD_GLOBAL_CONTROL, UDLD_8051_ENABLE, pEnable);

    return retVal;
}

/* Function Name:
 *      rtk_udld_portmask_get
 * Description:
 *      Get UDL port mask
 * Input:
 *      None
 * Output:
 *      pPortmask - port mask returned
 * Return:
 *      RT_ERR_OK - Success.
 *      RT_ERR_FAILED  - Failure.
 *      RT_ERR_NULL_POINTER - NULL pointer input.
 * Note:
 *     The API can get UDL port mask
 */
rtk_api_ret_t rtk_udld_portmask_get(rtk_portmask_t* pPortmask)
{
    rtk_api_ret_t retVal;
    
    if (pPortmask == NULL)
    {
        return RT_ERR_NULL_POINTER;
    }

   retVal = reg_field_read(RTL8316D_UNIT, UDL_PORT_MASK_CONTROL, UDL_PMSK, pPortmask->bits);

    return retVal;
}

/* Function Name:
 *      rtk_rrcp_enable_set
 * Description:
 *      Enable RRCP or not
 * Input:
 *      enable - ENABLED/DISABLED
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK - Success.
 *      RT_ERR_FAILED  - Failure.
 *      RT_ERR_ENABLE - value of enable should be either ENABLED or DISABLED.
 * Note:
 *     The API can get UDL port mask
 */
rtk_api_ret_t rtk_rrcp_enable_set(rtk_enable_t enable)
{
    rtk_api_ret_t retVal;

    if (enable >= RTK_ENABLE_END)
    {
        return RT_ERR_ENABLE;
    }

    retVal = reg_field_write(RTL8316D_UNIT, SWITCH_CHIP_ID_CONTROL, RRCPENABLE, enable);
    
    return retVal;
}

/* Function Name:
 *      rtk_rrcp_enable_get
 * Description:
 *      Retrieve RRCP enable or not
 * Input:
 *      None
 * Output:
 *      pEnable - ENABLED/DISABLED
 * Return:
 *      RT_ERR_OK - Success.
 *      RT_ERR_FAILED  - Failure.
 *      RT_ERR_NULL_POINTER - NULL pointer input.
 * Note:
 *     The API can retrieve RRCP enabling state.
 */
rtk_api_ret_t rtk_rrcp_enable_get(rtk_enable_t* pEnable)
{
    rtk_api_ret_t retVal;

    if (pEnable == NULL)
    {
        return RT_ERR_NULL_POINTER;
    }

    retVal = reg_field_read(RTL8316D_UNIT, SWITCH_CHIP_ID_CONTROL, RRCPENABLE, pEnable);

    return retVal;
}


/* Function Name:
 *      rtk_rrcp_v1Crc_set
 * Description:
 *      Enable/disable RRCPv1 CRC check
 * Input:
 *      enable - ENABLED/DISABLED
 * Output:
 *      None 
 * Return:
 *      RT_ERR_OK - Success.
 *      RT_ERR_FAILED  - Failure.
 *      RT_ERR_ENABLE - value of enable should be either ENABLED or DISABLED.
 * Note:
 *     The API can enable/disable RRCPv1 CRC check function.
 */
rtk_api_ret_t rtk_rrcp_v1Crc_set(rtk_enable_t enable)
{
    rtk_api_ret_t retVal;

    if (enable >= RTK_ENABLE_END)
    {
        return RT_ERR_ENABLE;
    }

    retVal = reg_field_write(RTL8316D_UNIT, RRCP_GLOBAL_CONTROL, V1SETCRCEN, enable);
    if (retVal != SUCCESS)
        return retVal;
    
    retVal = reg_field_write(RTL8316D_UNIT, RRCP_GLOBAL_CONTROL, V1GETCRCEN, enable);
    
    return retVal;
}

/* Function Name:
 *      rtk_rrcp_v1Crc_get
 * Description:
 *      Retrieve RRCPv1 CRC check state
 * Input:
 *      None
 * Output:
 *      pEnable - ENABLED/DISABLED
 * Return:
 *      RT_ERR_OK - Success.
 *      RT_ERR_FAILED  - Failure.
 *      RT_ERR_NULL_POINTER - NULL pointer input.
 * Note:
 *     The API can retrieve RRCPv1 CRC check state.
 */
rtk_api_ret_t rtk_rrcp_v1Crc_get(rtk_enable_t* pEnable)
{
    rtk_api_ret_t retVal;

    if (pEnable == NULL)
    {
        return RT_ERR_NULL_POINTER;
    }

    retVal = reg_field_read(RTL8316D_UNIT, RRCP_GLOBAL_CONTROL, V1SETCRCEN, pEnable);

    return retVal;
}

/* Function Name:
 *      rtk_rrcp_getPktCopytoCpu_set
 * Description:
 *      Enable/disable RRCP get packet copy to CPU
 * Input:
 *      enable - ENABLED/DISABLED
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK - Success.
 *      RT_ERR_FAILED  - Failure.
 *      RT_ERR_ENABLE - value of enable should be either ENABLED or DISABLED.
 * Note:
 *     The API can enable/disable RRCP get packet copy to CPU.
 */
rtk_api_ret_t rtk_rrcp_getPktCopytoCpu_set(rtk_enable_t enable)
{
    rtk_api_ret_t retVal;

    if (enable >= RTK_ENABLE_END)
    {
        return RT_ERR_ENABLE;
    }

    retVal = reg_field_write(RTL8316D_UNIT, RRCP_GLOBAL_CONTROL, RRCPGETTRAP, enable);
    
    return retVal;
}

/* Function Name:
 *      rtk_rrcp_getPktCopytoCpu_get
 * Description:
 *      Retrieve wheterh RRCP get packet copy to CPU or not
 * Input:
 *      None
 * Output:
 *      pEnable - ENABLED/DISABLED
 * Return:
 *      RT_ERR_OK - Success.
 *      RT_ERR_FAILED  - Failure.
 *      RT_ERR_NULL_POINTER - NULL pointer input.
 * Note:
 *     The API can retrieve wheterh RRCP get packet copy to CPU or not.
 */
rtk_api_ret_t rtk_rrcp_getPktCopytoCpu_get(rtk_enable_t* pEnable)
{
    rtk_api_ret_t retVal;

    if (pEnable == NULL)
    {
        return RT_ERR_NULL_POINTER;
    }

    retVal = reg_field_read(RTL8316D_UNIT, RRCP_GLOBAL_CONTROL, RRCPGETTRAP, pEnable);

    return retVal;
}

/* Function Name:
 *      rtk_rrcp_setPktCopytoCpu_set
 * Description:
 *      Enable/disable RRCP set packet copy to CPU
 * Input:
 *      enable - ENABLED/DISABLED
 * Output:
 *      None 
 * Return:
 *      RT_ERR_OK - Success.
 *      RT_ERR_FAILED  - Failure.
 *      RT_ERR_ENABLE - value of enable should be either ENABLED or DISABLED.
 * Note:
 *     The API can retrieve wheterh RRCP set packet copy to CPU or not.
 */
rtk_api_ret_t rtk_rrcp_setPktCopytoCpu_set(rtk_enable_t enable)
{
    rtk_api_ret_t retVal;

    if (enable >= RTK_ENABLE_END)
    {
        return RT_ERR_ENABLE;
    }

    retVal = reg_field_write(RTL8316D_UNIT, RRCP_GLOBAL_CONTROL, RRCPSETTRAP, enable);
    
    return retVal;
}

/* Function Name:
 *      rtk_rrcp_setPktCopytoCpu_get
 * Description:
 *      Retrieve wheterh RRCP set packet copy to CPU or not
 * Input:
 *      None 
 * Output:
 *      pEnable - ENABLED/DISABLED
 * Return:
 *      RT_ERR_OK - Success.
 *      RT_ERR_FAILED  - Failure.
 *      RT_ERR_NULL_POINTER - NULL pointer input.
 * Note:
 *     The API can retrieve wheterh RRCP set packet copy to CPU or not.
 */
rtk_api_ret_t rtk_rrcp_setPktCopytoCpu_get(rtk_enable_t* pEnable)
{
    rtk_api_ret_t retVal;

    if (pEnable == NULL)
    {
        return RT_ERR_NULL_POINTER;
    }

    retVal = reg_field_read(RTL8316D_UNIT, RRCP_GLOBAL_CONTROL, RRCPSETTRAP, pEnable);

    return retVal;
}

/* Function Name:
 *      rtk_rrcp_vlanLeaky_set
 * Description:
 *      Enable/disable RRCP VLAN leaky
 * Input:
 *      enable - ENABLED/DISABLED
 * Output:
 *      None 
 * Return:
 *      RT_ERR_OK - Success.
 *      RT_ERR_FAILED  - Failure.
 *      RT_ERR_ENABLE - value of enable should be either ENABLED or DISABLED.
 * Note:
 *     The API can enable/disable RRCP VLAN leaky
 */
rtk_api_ret_t rtk_rrcp_vlanLeaky_set(rtk_enable_t enable)
{
    rtk_api_ret_t retVal;

    if (enable >= RTK_ENABLE_END)
    {
        return RT_ERR_ENABLE;
    }

    retVal = reg_field_write(RTL8316D_UNIT, RRCP_GLOBAL_CONTROL, RRCPVLEAKY, enable);

    return retVal;
}

/* Function Name:
 *      rtk_rrcp_vlanLeaky_get
 * Description:
 *      Get RRCP VLAN leaky mode
 * Input:
 *      None 
 * Output:
 *      pEnable - ENABLED/DISABLED
 * Return:
 *      RT_ERR_OK - Success.
 *      RT_ERR_FAILED  - Failure.
 *      RT_ERR_NULL_POINTER - NULL pointer input.
 * Note:
 *     The API can configure RRCP VLAN leaky mode
 */
rtk_api_ret_t rtk_rrcp_vlanLeaky_get(rtk_enable_t* pEnable)
{
    rtk_api_ret_t retVal;

    if (pEnable == NULL)
    {
        return RT_ERR_NULL_POINTER;
    }

    retVal = reg_field_read(RTL8316D_UNIT, RRCP_GLOBAL_CONTROL, RRCPVLEAKY, pEnable);

    return retVal;
}

/* Function Name:
 *      rtk_rrcp_pbVlan_set
 * Description:
 *      Enable/disable RRCP port-based VLAN
 * Input:
 *      enable - ENABLED/DISABLED
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK - Success.
 *      RT_ERR_FAILED  - Failure.
 *      RT_ERR_ENABLE - value of enable should be either ENABLED or DISABLED.
 * Note:
 *     The API can enable/disable RRCP port-based VLAN.
 */
rtk_api_ret_t rtk_rrcp_pbVlan_set(rtk_enable_t enable)
{
    rtk_api_ret_t retVal;

    if (enable >= RTK_ENABLE_END)
    {
        return RT_ERR_ENABLE;
    }

    retVal = reg_field_write(RTL8316D_UNIT, RRCP_GLOBAL_CONTROL, RRCPPBVLAN, enable);

    return retVal;
}

/* Function Name:
 *      rtk_rrcp_pbVlan_get
 * Description:
 *      Get RRCP port-based VLAN
 * Input:
 *      None
 * Output:
 *      pEnable - ENABLED/DISABLED
 * Return:
 *      RT_ERR_OK - Success.
 *      RT_ERR_FAILED  - Failure.
 *      RT_ERR_ENABLE - value of enable should be either ENABLED or DISABLED.
 * Note:
 *     The API can configure RRCP port-based VLAN.
 */
rtk_api_ret_t rtk_rrcp_pbVlan_get(rtk_enable_t* pEnable)
{
    rtk_api_ret_t retVal;

    if (pEnable == NULL)
    {
        return RT_ERR_NULL_POINTER;
    }

    retVal = reg_field_read(RTL8316D_UNIT, RRCP_GLOBAL_CONTROL, RRCPPBVLAN, pEnable);

    return retVal;
}

/* Function Name:
 *      rtk_rrcp_privateKey_set
 * Description:
 *      Configure RRCP private key
 * Input:
 *      key
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK - Success.
 *      RT_ERR_FAILED  - Failure.
 * Note:
 *     The API can configure RRCP private key.
 */
rtk_api_ret_t rtk_rrcp_privateKey_set(uint32 key)
{
    rtk_api_ret_t retVal;

    retVal = reg_field_write(RTL8316D_UNIT, RRCP_PRIVATE_KEY_CONTROL, RRCPPRIVKEY, key);

    return retVal;
}

/* Function Name:
 *      rtk_rrcp_privateKey_get
 * Description:
 *      Retrieve RRCP private key
 * Input:
 *      None
 * Output:
 *      pKey 
 * Return:
 *      RT_ERR_OK - Success.
 *      RT_ERR_FAILED  - Failure.
 *      RT_ERR_NULL_POINTER - NULL pointer input.
 * Note:
 *     The API can retrieve RRCP private key.
 */
rtk_api_ret_t rtk_rrcp_privateKey_get(uint32* pKey)
{
    rtk_api_ret_t retVal;

    if (pKey == NULL)
    {
        return RT_ERR_NULL_POINTER;
    }

    retVal = reg_field_read(RTL8316D_UNIT, RRCP_PRIVATE_KEY_CONTROL, RRCPPRIVKEY, pKey);

    return retVal;
}

/* Function Name:
 *      rtk_rrcp_authKey_set
 * Description:
 *      Configure RRCP authentication key
 * Input:
 *      key
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK - Success.
 *      RT_ERR_FAILED  - Failure.
 * Note:
 *     The API can configure RRCP authentication key.
 */
rtk_api_ret_t rtk_rrcp_authKey_set(uint16 key)
{
    rtk_api_ret_t retVal;

    retVal = reg_field_write(RTL8316D_UNIT, RRCP_AUTHENTICATION_KEY_CONTROL, RRCPAUTHKEY, key);

    return retVal;
}

/* Function Name:
 *      rtk_rrcp_authKey_get
 * Description:
 *      Retrieve RRCP authentication key
 * Input:
 *      None
 * Output:
 *      pKey
 * Return:
 *      RT_ERR_OK - Success.
 *      RT_ERR_FAILED  - Failure.
 *      RT_ERR_NULL_POINTER - NULL pointer input.
 * Note:
 *     The API can retrieve RRCP authentication key.
 */
rtk_api_ret_t rtk_rrcp_authKey_get(uint16* pKey)
{
    rtk_api_ret_t retVal;
    uint32 regVal;

    if (pKey == NULL)
    {
        return RT_ERR_NULL_POINTER;
    }

    retVal = reg_field_read(RTL8316D_UNIT, RRCP_AUTHENTICATION_KEY_CONTROL, RRCPAUTHKEY, &regVal);
    *pKey = regVal & 0xFFFF;
    return retVal;
}

/* Function Name:
 *      rtk_rrcp_customerCode_set
 * Description:
 *      Configure RRCP customer code
 * Input:
 *      code - 
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK - Success.
 *      RT_ERR_FAILED  - Failure.
 * Note:
 *     The API can configure RRCP customer code.
 */
rtk_api_ret_t rtk_rrcp_customerCode_set(uint32 code)
{
    rtk_api_ret_t retVal;

    retVal = reg_field_write(RTL8316D_UNIT, RRCP_CUSTOMER_CODE_CONTROL, CUSTOMER_CODE, code);

    return retVal;
}

/* Function Name:
 *      rtk_rrcp_customerCode_get
 * Description:
 *      Retrieve RRCP customer code
 * Input:
 *      None
 * Output:
 *      pCode
 * Return:
 *      RT_ERR_OK - Success.
 *      RT_ERR_FAILED  - Failure.
 *      RT_ERR_NULL_POINTER - NULL pointer input.
 * Note:
 *     The API can retrieve RRCP customer code.
 */
rtk_api_ret_t rtk_rrcp_customerCode_get(uint32* pCode)
{
    rtk_api_ret_t retVal;

    if (pCode == NULL)
    {
        return RT_ERR_NULL_POINTER;
    }

    retVal = reg_field_read(RTL8316D_UNIT, RRCP_CUSTOMER_CODE_CONTROL, CUSTOMER_CODE, pCode);

    return retVal;
}

/* Function Name:
 *      rtk_rrcp_defaultFid_set
 * Description:
 *      Configure RRCP default FID
 * Input:
 *      fid - the default FID
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK - Success.
 *      RT_ERR_FAILED  - Failure.
 *      RT_ERR_INPUT - error FID value range
 * Note:
 *     The API can configure RRCP default FID.
 */
rtk_api_ret_t rtk_rrcp_defaultFid_set(uint32 fid)
{
    rtk_api_ret_t retVal;

    if (fid >= (1 << 12))
    {
        return RT_ERR_INPUT;
    }
    
    retVal = reg_field_write(RTL8316D_UNIT, RRCP_DEFAULT_FID_CONTROL, RRCPDFID, fid);

    return retVal;
}

/* Function Name:
 *      rtk_rrcp_defaultFid_get
 * Description:
 *      Retrieve RRCP default FID
 * Input:
 *      None
 * Output:
 *      pFid - the default FID returned
 * Return:
 *      RT_ERR_OK - Success.
 *      RT_ERR_FAILED  - Failure.
 *      RT_ERR_NULL_POINTER - NULL pointer input.
 * Note:
 *     The API can retrieve RRCP default FID.
 */
rtk_api_ret_t rtk_rrcp_defaultFid_get(uint32* pFid)
{
    rtk_api_ret_t retVal;

    if (pFid == NULL)
    {
        return RT_ERR_NULL_POINTER;
    }

    retVal = reg_field_read(RTL8316D_UNIT, RRCP_DEFAULT_FID_CONTROL, RRCPDFID, pFid);

    return retVal;
}

/* Function Name:
 *      rtk_rrcp_adminPort_set
 * Description:
 *      Configure RRCP administrable port
 * Input:
 *      port - port ID
 *      enable - ENABLED/DISABLED
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK - Success.
 *      RT_ERR_FAILED  - Failure.
 *      RT_ERR_PORT_ID - error port ID input.
 *      RT_ERR_ENABLE - value of enable should be either ENABLED or DISABLED.
 * Note:
 *     The API can configure RRCP administrable port.
 */
rtk_api_ret_t rtk_rrcp_adminPort_set(rtk_port_t port, rtk_enable_t enable)
{
    rtk_api_ret_t retVal;
    uint32 regVal;

    if(port >= RTL8316D_MAX_PORT)
        return RT_ERR_PORT_ID;    

    if (enable >= RTK_ENABLE_END)
    {
        return RT_ERR_ENABLE;
    }

    retVal = reg_read(RTL8316D_UNIT, RRCP_ADMINISTRABLE_CONTROL, &regVal);
    if (retVal != SUCCESS)
        return retVal;

    if (enable == ENABLE)
    {
        regVal |= 1 << port;
    } else {
        regVal &= ~(1 << port);
    }
    retVal = reg_write(RTL8316D_UNIT, RRCP_ADMINISTRABLE_CONTROL, regVal);

    return retVal;
}

/* Function Name:
 *      rtk_rrcp_adminPort_get
 * Description:
 *      Retrieve RRCP administrable port state
 * Input:
 *      port - port ID
 * Output:
 *      pEnable - ENABLED/DISABLED
 * Return:
 *      RT_ERR_OK - Success.
 *      RT_ERR_FAILED  - Failure.
 *      RT_ERR_PORT_ID - error port ID input.
 *      RT_ERR_NULL_POINTER - NULL pointer input.
 * Note:
 *     The API can retrieve RRCP administrable port state.
 */
rtk_api_ret_t rtk_rrcp_adminPort_get(rtk_port_t port, rtk_enable_t* pEnable)
{
    rtk_api_ret_t retVal;
    uint32 regVal;

    if(port >= RTL8316D_MAX_PORT)
        return RT_ERR_PORT_ID;    

    if (pEnable == NULL)
    {
        return RT_ERR_NULL_POINTER;
    }

    retVal = reg_read(RTL8316D_UNIT, RRCP_ADMINISTRABLE_CONTROL, &regVal);
    if (retVal != SUCCESS)
        return retVal;

    *pEnable = (regVal >> port) & 0x1;
    
    return retVal;
}

/* Function Name:
 *      rtk_rrcp_authPort_set
 * Description:
 *      Configure RRCP authentication port
 * Input:
 *      port - port ID
 *      enable - ENABLED/DISABLED
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK - Success.
 *      RT_ERR_FAILED  - Failure.
 *      RT_ERR_PORT_ID - error port ID input.
 *      RT_ERR_ENABLE - value of enable should be either ENABLED or DISABLED.
 * Note:
 *     The API can configure RRCP authentication port.
 */
rtk_api_ret_t rtk_rrcp_authPort_set(rtk_port_t port, rtk_enable_t enable)
{
    rtk_api_ret_t retVal;
    uint32 regVal;

    if(port >= RTL8316D_MAX_PORT)
        return RT_ERR_PORT_ID;    

    if (enable >= RTK_ENABLE_END)
    {
        return RT_ERR_ENABLE;
    }

    retVal = reg_read(RTL8316D_UNIT, RRCP_AUTHENTICAED_CONTROL, &regVal);
    if (retVal != SUCCESS)
        return retVal;

    if (enable == ENABLE)
    {
        regVal |= 1 << port;
    } else {
        regVal &= ~(1 << port);
    }
    retVal = reg_write(RTL8316D_UNIT, RRCP_ADMINISTRABLE_CONTROL, regVal);

    return retVal;
}

/* Function Name:
 *      rtk_rrcp_authPort_get
 * Description:
 *      Retrieve RRCP authentication port state
 * Input:
 *      port - port ID
 * Output:
 *      pEnable - ENABLED/DISABLED
 * Return:
 *      RT_ERR_OK - Success.
 *      RT_ERR_FAILED  - Failure.
 *      RT_ERR_PORT_ID - error port ID input.
 *      RT_ERR_NULL_POINTER - NULL pointer input.
 * Note:
 *     The API can retrieve RRCP authentication port state.
 */
rtk_api_ret_t rtk_rrcp_authPort_get(rtk_port_t port, rtk_enable_t* pEnable)
{
    rtk_api_ret_t retVal;
    uint32 regVal;

    if(port >= RTL8316D_MAX_PORT)
        return RT_ERR_PORT_ID;    

    if (pEnable == NULL)
    {
        return RT_ERR_NULL_POINTER;
    }

    retVal = reg_read(RTL8316D_UNIT, RRCP_ADMINISTRABLE_CONTROL, &regVal);
    if (retVal != SUCCESS)
        return retVal;

    *pEnable = (regVal >> port) & 0x1;
    
    return retVal;
}

/* Function Name:
 *      rtk_rrcp_chipId_set
 * Description:
 *      Configure switch CHIP ID
 * Input:
 *      chipId - chip ID
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK - Success.
 *      RT_ERR_FAILED  - Failure.
 * Note:
 *     The API can configure switch CHIP ID.
 */
rtk_api_ret_t rtk_rrcp_chipId_set(uint16 chipId)
{
    rtk_api_ret_t retVal;

    retVal = reg_field_write(RTL8316D_UNIT, SWITCH_CHIP_ID_CONTROL, CHIPID, chipId);

    return retVal;
}

/* Function Name:
 *      rtk_rrcp_chipId_get
 * Description:
 *      Retrieve switch CHIP ID
 * Input:
 *      None
 * Output:
 *      pChipId - chip ID
 * Return:
 *      RT_ERR_OK - Success.
 *      RT_ERR_FAILED  - Failure.
 *      RT_ERR_NULL_POINTER - NULL pointer input.
 * Note:
 *     The API can retrieve switch CHIP ID.
 */
rtk_api_ret_t rtk_rrcp_chipId_get(uint16* pChipId)
{
    rtk_api_ret_t retVal;
    uint32 regVal;

    if (pChipId == NULL)
    {
        return RT_ERR_NULL_POINTER;
    }

    retVal = reg_field_read(RTL8316D_UNIT, RRCP_CUSTOMER_CODE_CONTROL, CUSTOMER_CODE, &regVal);
    *pChipId = regVal & 0xFFFF;

    return retVal;
}


/* Function Name:
 *      rtk_rrcp_venderId_set
 * Description:
 *      Configure switch vender ID
 * Input:
 *      venderId - vender ID
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK - Success.
 *      RT_ERR_FAILED  - Failure.
 * Note:
 *     The API can configure vender CHIP ID.
 */
rtk_api_ret_t rtk_rrcp_venderId_set(uint32 venderId)
{
    rtk_api_ret_t retVal;

    retVal = reg_field_write(RTL8316D_UNIT, SWITCH_VENDER_ID_CONTROL, VENDERID, venderId);

    return retVal;
}

/* Function Name:
 *      rtk_rrcp_venderId_get
 * Description:
 *      Retrieve switch vender ID
 * Input:
 *      None
 * Output:
 *      pVenderId - vender ID returned
 * Return:
 *      RT_ERR_OK - Success.
 *      RT_ERR_FAILED  - Failure.
 *      RT_ERR_NULL_POINTER - NULL pointer input.
 * Note:
 *     The API can retrieve switch vender ID.
 */
rtk_api_ret_t rtk_rrcp_venderId_get(uint32* pVenderId)
{
    rtk_api_ret_t retVal;

    if (pVenderId == NULL)
    {
        return RT_ERR_NULL_POINTER;
    }

    retVal = reg_field_read(RTL8316D_UNIT, SWITCH_VENDER_ID_CONTROL, VENDERID, pVenderId);

    return retVal;
}


/* Function Name:
 *      rtk_rrcp_switchMac_set
 * Description:
 *      Configure switch own MAC
 * Input:
 *      pSwitchOwnMac - siwtch own MAC
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK - Success.
 *      RT_ERR_FAILED  - Failure.
 *      RT_ERR_NULL_POINTER - NULL pointer input.
 * Note:
 *     The API can configure switch own MAC.
 */
rtk_api_ret_t rtk_rrcp_switchMac_set(rtk_mac_t* pSwitchOwnMac)
{
    rtk_api_ret_t retVal;
    uint32 regVal;
    
    if (pSwitchOwnMac == NULL)
    {
        return RT_ERR_NULL_POINTER;
    }

    regVal = pSwitchOwnMac->octet[2];
    regVal |= pSwitchOwnMac->octet[3] << 8;
    regVal |= pSwitchOwnMac->octet[4] << 16;
    regVal |= pSwitchOwnMac->octet[5] << 24;
    
    retVal = reg_write(RTL8316D_UNIT, SWITCH_OWN_MAC_CONTROL0, regVal);
    if (retVal != SUCCESS)
        return retVal;

    regVal = pSwitchOwnMac->octet[0];
    regVal |= pSwitchOwnMac->octet[1] << 8;
    retVal = reg_field_write(RTL8316D_UNIT, SWITCH_OWN_MAC_CONTROL1, SWMAC15_0, regVal);
    return retVal;
}


/* Function Name:
 *      rtk_rrcp_switchMac_get
 * Description:
 *      Get switch own MAC
 * Input:
 *      None
 * Output:
 *      pSwitchOwnMac - siwtch own MAC
 * Return:
 *      RT_ERR_OK - Success.
 *      RT_ERR_FAILED  - Failure.
 *      RT_ERR_NULL_POINTER - NULL pointer input.
 * Note:
 *     The API can get switch own MAC.
 */
rtk_api_ret_t rtk_rrcp_switchMac_get(rtk_mac_t* pSwitchOwnMac)
{
    rtk_api_ret_t retVal;
    uint32 regVal;
    
    if (pSwitchOwnMac == NULL)
    {
        return RT_ERR_NULL_POINTER;
    }

    retVal =reg_read(RTL8316D_UNIT, SWITCH_OWN_MAC_CONTROL0, &regVal);
    if (retVal != SUCCESS)
        return retVal;
    
    pSwitchOwnMac->octet[2] = regVal & 0xFF;
    pSwitchOwnMac->octet[3] = (regVal >> 8) & 0xFF;
    pSwitchOwnMac->octet[4] = (regVal >> 16) & 0xFF;
    pSwitchOwnMac->octet[5] = (regVal >> 24) & 0xFF;
    
    retVal = reg_field_read(RTL8316D_UNIT, SWITCH_OWN_MAC_CONTROL1, SWMAC15_0, &regVal);

    pSwitchOwnMac->octet[0] = regVal & 0xFF;
    pSwitchOwnMac->octet[1] = (regVal>>8) & 0xFF;

    return retVal;
}

/* Function Name:
 *      rtk_rrcp_switchMacEn_set
 * Description:
 *      Enable/disable switch own MAC
 * Input:
 *      enable
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK - Success.
 *      RT_ERR_FAILED  - Failure.
 *      RT_ERR_ENABLE - value of enable should be either ENABLED or DISABLED.
 * Note:
 *     The API can enable/disable switch own MAC.
 */
rtk_api_ret_t rtk_rrcp_switchMacEn_set(rtk_enable_t enable)
{
    rtk_api_ret_t retVal;
    
    if (enable >= RTK_ENABLE_END)
    {
        return RT_ERR_ENABLE;
    }

    retVal = reg_field_write(RTL8316D_UNIT, SWITCH_OWN_MAC_CONTROL1, SWMACVALID, enable);

    return retVal;
}

/* Function Name:
 *      rtk_rrcp_switchMacEn_get
 * Description:
 *      Retrieve whether switch own MAC valid or not
 * Input:
 *      None
 * Output:
 *      pEnable - 
 * Return:
 *      RT_ERR_OK - Success.
 *      RT_ERR_FAILED  - Failure.
 *      RT_ERR_NULL_POINTER - NULL pointer input.
 * Note:
 *     The API can retrieve whether switch own MAC valid or not.
 */
rtk_api_ret_t rtk_rrcp_switchMacEn_get(rtk_enable_t* pEnable)
{
    rtk_api_ret_t retVal;
    
    if (pEnable == NULL)
    {
        return RT_ERR_NULL_POINTER;
    }

    retVal = reg_field_read(RTL8316D_UNIT, SWITCH_OWN_MAC_CONTROL1, SWMACVALID, pEnable);

    return retVal;
}


/* Function Name:
 *      rtk_rrcp_switchOwnDmacAct_set
 * Description:
 *      Switch own DMAC action
 * Input:
 *      action - 0:drop, 1:trap, 2 forward
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK - Success.
 *      RT_ERR_FAILED  - Failure.
 *      RT_ERR_INPUT - error action type.
 * Note:
 *     The API configures action on packets whose DMACs are identical to switch own MAC.
 */
rtk_api_ret_t rtk_rrcp_switchOwnDmacAct_set(uint8 action)
{
    rtk_api_ret_t retVal;
    
    if (action >= 3)
    {
        return RT_ERR_INPUT;
    }

    retVal = reg_field_write(RTL8316D_UNIT, SWITCH_OWN_MAC_CONTROL1, DSTSWMACCHK, action);

    return retVal;
}

/* Function Name:
 *      rtk_rrcp_switchOwnDmacAct_get
 * Description:
 *      get switch own DMAC action
 * Input:
 *      None
 * Output:
 *      pAction - 0:drop, 1:trap, 2 forward
 * Return:
 *      RT_ERR_OK - Success.
 *      RT_ERR_FAILED  - Failure.
 *      RT_ERR_NULL_POINTER - NULL pointer input.
 * Note:
 *     The API retrieves action on packets whose DMACs are identical to switch own MAC.
 */
rtk_api_ret_t rtk_rrcp_switchOwnDmacAct_get(uint8* pAction)
{
    rtk_api_ret_t retVal;
    uint32 action;
    
    if (pAction == NULL)
    {
        return RT_ERR_NULL_POINTER;
    }

    retVal = reg_field_read(RTL8316D_UNIT, SWITCH_OWN_MAC_CONTROL1, DSTSWMACCHK, &action);
    *pAction = action & 0xFF;
    
    return retVal;
}

/* Function Name:
 *      rtk_rrcp_hiRetryCnt_set
 * Description:
 *      Configure RRCP hello indication retry count
 * Input:
 *      nr_retry - retry number, should smaller than 0x8
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK - Success.
 *      RT_ERR_FAILED  - Failure.
 *      RT_ERR_INPUT - retry count too large.
 * Note:
 *     The API can configure RRCP hello indication retry count.
 */
rtk_api_ret_t rtk_rrcp_hiRetryCnt_set(uint8 nr_retry)
{
    rtk_api_ret_t retVal;

    if (nr_retry >= 8)
    {
        return RT_ERR_INPUT;
    }

    retVal = reg_field_write(RTL8316D_UNIT, RRCP_HELLO_INDICATION_CONTROL, HICOUNT, nr_retry);
    return retVal;
}

/* Function Name:
 *      rtk_rrcp_hiRetryCnt_get
 * Description:
 *      Retrieve RRCP hello indication retry count
 * Input:
 *      None
 * Output:
 *      pNr_retry - retry number
 * Return:
 *      RT_ERR_OK - Success.
 *      RT_ERR_FAILED  - Failure.
 *      RT_ERR_NULL_POINTER - NULL pointer input.
 * Note:
 *     The API can retrieve RRCP hello indication retry count.
 */
rtk_api_ret_t rtk_rrcp_hiRetryCnt_get(uint8* pNr_retry)
{
    rtk_api_ret_t retVal;
    uint32 regVal;
    
    if (pNr_retry == NULL)
        return RT_ERR_NULL_POINTER;
    
    retVal = reg_field_read(RTL8316D_UNIT, RRCP_HELLO_INDICATION_CONTROL, HICOUNT, &regVal);

    *pNr_retry = regVal & 0x7;
    
    return retVal;
}

/* Function Name:
 *      rtk_rrcp_hiRetryInterval_set
 * Description:
 *      Configure RRCP hello indication retry interval
 * Input:
 *      interval - retry interval, 10-bit in length
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK - Success.
 *      RT_ERR_FAILED  - Failure.
 *      RT_ERR_INPUT - interval value is too large.
 * Note:
 *     The API can configure RRCP hello indication retry interval.
 */
rtk_api_ret_t rtk_rrcp_hiRetryInterval_set(uint16 interval)
{
    rtk_api_ret_t retVal;

    if (interval > 0x3FF)
    {
        return RT_ERR_INPUT;
    }

    retVal = reg_field_write(RTL8316D_UNIT, RRCP_HELLO_INDICATION_CONTROL, HIPERIOD, interval);
    return retVal;
}

/* Function Name:
 *      rtk_rrcp_hiRetryInterval_get
 * Description:
 *      Retrieve RRCP hello indication retry interval
 * Input:
 *      None
 * Output:
 *      pInterval - retry interval
 * Return:
 *      RT_ERR_OK - Success.
 *      RT_ERR_FAILED  - Failure.
 *      RT_ERR_NULL_POINTER - NULL pointer input.
 * Note:
 *     The API can retrieve RRCP hello indication retry interval.
 */
rtk_api_ret_t rtk_rrcp_hiRetryInterval_get(uint16* pInterval)
{
    rtk_api_ret_t retVal;
    uint32 regVal;
    
    if (pInterval == NULL)
        return RT_ERR_NULL_POINTER;
    
    retVal = reg_field_read(RTL8316D_UNIT, RRCP_HELLO_INDICATION_CONTROL, HIPERIOD, &regVal);

    *pInterval = regVal & 0x3ff;
    
    return retVal;
}



