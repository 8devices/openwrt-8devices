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
* Purpose :  ASIC-level driver implementation for QoS.
*
*  Feature :  This file consists of following modules:
*             1) 
*
*/

#include <rtk_types.h>
#include <asicdrv/rtl8316d_types.h>
#include <asicdrv/rtl8316d_asicDrv.h>
#include <rtl8316d_asicdrv_qos.h>
#include <rtl8316d_reg_struct.h>
#include <rtl8316d_debug.h>

static rtk_qos_queue_weights_t weight[RTL8316D_MAX_PHY_PORT];
static uint8 qos_weight_first = FALSE;

rtk_api_ret_t rtk_qos_queueNum_set(rtk_port_t port, rtk_queue_num_t queue_num)
{
    port = 0;       // kill compiler warning
    queue_num = 0;  // kill compiler warning 
    
    return RT_ERR_OK;
}

#if 0
rtk_api_ret_t rtk_qos_queueNum_get(rtk_port_t port, rtk_queue_num_t *pQueue_num)
{
    port = 0;    // kill compiler warning               
    *pQueue_num = MAX_LWS_QUEUE_NUM;
        
    return RT_ERR_OK;
}
#endif

rtk_api_ret_t rtk_qos_schedulingQueue_set(rtk_port_t port,rtk_qos_queue_weights_t *pQweights)
{
    uint8 i, j;

    if (qos_weight_first == FALSE)
    {
        for (i = 0; i < RTL8316D_MAX_PHY_PORT; i++)
        {
            for (j = 0; j < RTK_MAX_NUM_OF_QUEUE; j++)  
                weight[i].weights[j] = 0;
        }
         
        qos_weight_first = TRUE;
    }
    
    memcpy(&weight[port], pQweights, sizeof(rtk_qos_queue_weights_t));    

    return RT_ERR_OK;
}
    
rtk_api_ret_t rtk_qos_schedulingQueue_get(rtk_port_t port, rtk_qos_queue_weights_t *pQweights)
{
    
    uint8 i, j;

    if (qos_weight_first == FALSE)
    {
        for (i = 0; i < RTL8316D_MAX_PHY_PORT; i++)
        {
            for (j = 0; j < RTK_MAX_NUM_OF_QUEUE; j++)  
                weight[i].weights[j] = 0;
        }
         
        qos_weight_first = TRUE;
    }

    memcpy(pQweights, &weight[port], sizeof(rtk_qos_queue_weights_t));    

    return RT_ERR_OK;
}


/* Function Name:
 *      rtk_rate_shareMeter_set
 * Description:
 *      Set meter configuration
 * Input:
 *      index  - Shared meter index
 *      rate - Rate of share meter
 *      ifg_include     - Rate's calculation including IFG, ENABLE:include DISABLE:exclude
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK - Success
 *      FAILED  - Failure
 *      RT_ERR_FILTER_METER_ID - Invalid meter
 *      RT_ERR_RATE - Invalid rate
 *      RT_ERR_INPUT - Invalid input parameters.
 * Note:      
 *        The API can set shared meter rate and ifg include for each meter. 
 *        The rate unit is 1 kbps and the range is from 8k to 1048568k.
 *        The granularity of rate is 8 kbps. The ifg_include parameter is used 
 *        for rate calculation with/without inter-frame-gap and preamble . 
 *      - LIMIT_LEARN_CNT_ACTION_DROP
 *      - LIMIT_LEARN_CNT_ACTION_FORWARD
 *      - LIMIT_LEARN_CNT_ACTION_TO_CPU
 */  
rtk_api_ret_t rtk_rate_shareMeter_set(rtk_meter_id_t index, rtk_rate_t rate, rtk_enable_t ifg_include)
{
//    rtk_api_ret_t retVal;
#if 0    
    if (index>=RTL8370_METERNO)
        return RT_ERR_FILTER_METER_ID;

    if(rate>RTL8370_QOS_RATE_INPUT_MAX || rate<RTL8370_QOS_RATE_INPUT_MIN)
        return RT_ERR_RATE ;

    if(ifg_include>=RTK_ENABLE_END)
        return RT_ERR_ENABLE;    
        
    if((retVal=rtl8370_setAsicShareMeter(index,rate>>3,ifg_include))!=RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
#endif
    return RT_ERR_FAILED;
}


/* Function Name:
 *      rtk_rate_shareMeter_get
 * Description:
 *      Get meter configuration
 * Input:
 *      index  - Shared meter index
 *      pRate - Pointer of rate of share meter.
 *      pIfg_include     - Rate's calculation including IFG, ENABLE:include DISABLE:exclude
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK - Success
 *      FAILED  - Failure
 *      RT_ERR_FILTER_METER_ID - Invalid meter
 *      RT_ERR_RATE - Invalid rate
 * Note:      
 *      The API can get shared meter rate and ifg include for each meter. 
 *      The rate unit is 1 kbps and the granularity of rate is 8 kbps.
 *      The ifg_include parameter is used for rate calculation with/without inter-frame-gap and preamble 
 */ 
rtk_api_ret_t rtk_rate_shareMeter_get(rtk_meter_id_t index, uint32 *pRate ,rtk_enable_t *pIfg_include)
{
#if 0
    rtk_api_ret_t retVal;
    uint32 regData;
    
    if (index>=RTL8370_METERNO)
        return RT_ERR_FILTER_METER_ID;

    retVal=rtl8370_getAsicShareMeter(index, &regData, (uint32*)pIfg_include);
    
    if(retVal!=RT_ERR_OK)
        return retVal; 

    *pRate = regData<<3;
        
    return RT_ERR_OK;
#endif
        return RT_ERR_FAILED;
}

/* Function Name:
 *      rtk_rate_igrBandwidthCtrlRate_set
 * Description:
 *      Set port ingress bandwidth control
 * Input:
 *      port  - Port id.
 *      rate - Rate of share meter
 *      ifg_include     - Rate's calculation including IFG, ENABLE:include DISABLE:exclude
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK - Success.
 *      RT_ERR_SMI - SMI access error.
 *      RT_ERR_PORT_ID - Invalid port number.
 *      RT_ERR_ENABLE - Invalid IFG parameter.
 *      RT_ERR_INBW_RATE - Invalid ingress rate parameter.
 *      RT_ERR_FAILED - Failure. 
 * Note:      
 *   The rate unit is 16 kbps and the range is from 16K to 1G. The granularity of rate is 16 kbps. 
 *   The ifg_include parameter is used for rate calculation with/without inter-frame-gap and preamble. 
 */  
rtk_api_ret_t rtk_rate_igrBandwidthCtrlRate_set( rtk_port_t port, rtk_rate_t rate,  rtk_enable_t ifg_include)
{
    rtk_api_ret_t retVal;

    if(port >= RTL8316D_MAX_PORT)
        return RT_ERR_PORT_ID;    
    
    if(rate > RTL8316D_QOS_RATE_INPUT_MAX || rate < RTL8316D_QOS_RATE_INPUT_MIN)
        return RT_ERR_INBW_RATE ;

    if(ifg_include >= RTK_ENABLE_END)
        return RT_ERR_ENABLE;

    if((retVal = rtl8316d_setAsicPortIngressBandwidth(port, rate>>4, (uint32)ifg_include)) != RT_ERR_OK)
        return retVal;            

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_rate_igrBandwidthCtrlRate_get
 * Description:
 *      Get port ingress bandwidth control
 * Input:
 *      port  - Port id.
 *      pRate - Rate of share meter
 *      pIfg_include     - Rate's calculation including IFG, ENABLE:include DISABLE:exclude
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK | Success.
 *      RT_ERR_PORT_ID | Invalid port number.
 *      RT_ERR_INPUT | Invalid input parameters.
 *      RT_ERR_FAILED | Failure. 
 * Note:      
 *    The rate unit is 16 kbps and the range is from 16k to 1G. The granularity of rate is 16 kbps. 
 *      The ifg_include parameter is used for rate calculation with/without inter-frame-gap and preamble. 
 */
rtk_api_ret_t rtk_rate_igrBandwidthCtrlRate_get(rtk_port_t port, rtk_rate_t *pRate, rtk_enable_t *pIfg_include)
{
    rtk_api_ret_t retVal;
    uint32 regData;
    uint32 ifgInclude;

    if(port >= RTL8316D_MAX_PORT)
        return RT_ERR_PORT_ID;    

    if((retVal = rtl8316d_getAsicPortIngressBandwidth(port, &regData, &ifgInclude)) != RT_ERR_OK)
        return retVal;            

    if(NULL != pRate)
        *pRate = regData << 4;

    *pIfg_include = ifgInclude;

    return RT_ERR_OK;
}


/* Function Name:
 *      rtk_rate_egrBandwidthCtrlRate_set
 * Description:
 *      Set port egress bandwidth control
 * Input:
 *      port  - Port id
 *      rate - Rate of share meter
 *      ifg_include     - Rate's calculation including IFG, ENABLE:include DISABLE:exclude
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK | Success.
 *      RT_ERR_PORT_ID | Invalid port number.
 *      RT_ERR_ENABLE | Invalid IFG parameter.
 *      RT_ERR_INPUT | Invalid input parameters.
 *      RT_ERR_QOS_EBW_RATE | Invalid egress bandwidth/rate
 *      RT_ERR_FAILED | Failure.  
 * Note:
 *   The rate unit is 16 kbps and the range is from 16k to 1G. The granularity of rate is 16 kbps. 
 *   The ifg_include parameter is used for rate calculation with/without inter-frame-gap and preamble. 
 */
rtk_api_ret_t rtk_rate_egrBandwidthCtrlRate_set(rtk_port_t port, rtk_rate_t rate,  rtk_enable_t ifg_include)
{
    rtk_api_ret_t retVal;

    if(port >= RTL8316D_MAX_PORT)
        return RT_ERR_PORT_ID;    

    if(rate > RTL8316D_QOS_RATE_INPUT_MAX || rate < RTL8316D_QOS_RATE_INPUT_MIN)
        return RT_ERR_INBW_RATE ;

    if(ifg_include >= RTK_ENABLE_END)
        return RT_ERR_ENABLE;

    if((retVal = rtl8316d_setAsicPortEgressBandwidth(port, rate>>4, ifg_include)) != RT_ERR_OK)
        return retVal;            

    return RT_ERR_OK;
}


/* Function Name:
 *      rtk_rate_egrBandwidthCtrlRate_get
 * Description:
 *      Get port egress bandwidth control
 * Input:
 *      port  - Port id.
 *      pRate - Rate of share meter
 *      pIfg_include     - Rate's calculation including IFG, ENABLE:include DISABLE:exclude
 * Output:
 *      None
 * Return:
 *     RT_ERR_OK - Success.
 *     RT_ERR_PORT_ID - Invalid port number.
 *     RT_ERR_INPUT - Invalid input parameters.
 *     RT_ERR_FAILED - Failure. 
 * Note:
 *    The rate unit is 16 kbps and the range is from 16k to 1G. The granularity of rate is 16 kbps. 
 *    The ifg_include parameter is used for rate calculation with/without inter-frame-gap and preamble. 
 */
rtk_api_ret_t rtk_rate_egrBandwidthCtrlRate_get(rtk_port_t port, rtk_rate_t *pRate, rtk_enable_t *pIfg_include)
{
    rtk_api_ret_t retVal;
    uint32 regData;
    uint32 ifgInclude;

    if(port >= RTL8316D_MAX_PORT)
        return RT_ERR_PORT_ID;    

    if((retVal = rtl8316d_getAsicPortEgressBandwidth(port, &regData, &ifgInclude)) != RT_ERR_OK)
        return retVal;            

    if(NULL != pRate)
        *pRate = regData << 4;

    *pIfg_include = ifgInclude;
    
    return RT_ERR_OK;
}

rtk_api_ret_t rtk_rate_igrBandwidthEnable_set(uint32 port,uint32 enable)
{
    return rtl8316d_setAsicPortIngressBandwidthEnable(port, enable);
}

rtk_api_ret_t rtk_rate_igrBandwidthEnable_get(uint32 port,uint32 *enable)
{
    return rtl8316d_getAsicPortIngressBandwidthEnable(port, enable);
}

rtk_api_ret_t rtk_rate_egrBandwidthEnable_set(uint32 port,uint32 enable)
{
    return rtl8316d_setAsicPortEgressBandwidthEnable(port, enable);
}

rtk_api_ret_t rtk_rate_egrBandwidthEnable_get(uint32 port,uint32 *enable)
{
    return rtl8316d_getAsicPortEgressBandwidthEnable(port, enable);
}


/* Function Name:
 *      rtk_rate_stormControlRate_set
 * Description:
 *      Set per-port packet storm filter control rate.
 * Input:
 *      port  - Port id.
 *      storm_type - Storm filter control type.
 *      rate     - Rate of storm filter control.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK | Success.
 *      RT_ERR_SMI | SMI access error. 
 *      RT_ERR_PORT_ID | Invalid port number.
 *      RT_ERR_SFC_UNKNOWN_GROUP | unknown storm filter group.
 *      RT_ERR_ENABLE | Invalid IFG parameter.
 *      RT_ERR_INPUT | Invalid input parameters.
 *      RT_ERR_FAILED | Failure. 
 * Note:
 *      The storm filter control type can be:
 *          -STORM_GROUP_UNKNOWN_UNICAST 
 *          -STORM_GROUP_UNKNOWN_MULTICAST 
 *          -STORM_GROUP_MULTICAST
 *          -STORM_GROUP_BROADCAST
 *       The rate unit is 1 kbps and the range is from 1 to 2^20. The granularity of rate is 1 kbps. 
 *       The ifg_include parameter, a global configuration in RTL8316D, is used for rate calculation with/without inter-frame-gap and preamble.
 *       For RTL8316D, unknown unicast\unknown multicast\multicast and broadcast share unique bandwitdth each port.
 */
rtk_api_ret_t rtk_rate_stormControlRate_set(rtk_port_t port, rtk_rate_storm_group_t storm_type, rtk_rate_t rate, rtk_enable_t ifg_include)
{
    rtk_api_ret_t retVal;
//    uint32 enable;

    if(port > RTK_MAX_NUM_OF_PORT)
        return RT_ERR_PORT_ID;
    
    if(storm_type >= STORM_GROUP_END)
        return RT_ERR_SFC_UNKNOWN_GROUP;
    
    if(rate>RTL8316D_STORM_RATE_INPUT_MAX || rate < RTL8316D_STORM_RATE_INPUT_MIN)
        return RT_ERR_RATE ;

    if(ifg_include >= RTK_ENABLE_END)
        return RT_ERR_ENABLE;

    if ((retVal = reg_field_write(RTL8316D_UNIT, TRAFFSTORM_SUPPRESSION_GLOBAL1,SCMETER_INC_IFG, ifg_include))!=SUCCESS)
    {
        return retVal;
    }

    switch (storm_type)
    {
        case STORM_GROUP_UNKNOWN_UNICAST:
            retVal = reg_field_write(RTL8316D_UNIT, port+PORT0_TRAFFIC_STORM_CONTROL0, UNUCINCL, 1);
            break;
        case STORM_GROUP_UNKNOWN_MULTICAST:
            retVal = reg_field_write(RTL8316D_UNIT, port+PORT0_TRAFFIC_STORM_CONTROL0, UNMCINCL, 1);
            break;
        case STORM_GROUP_MULTICAST:
            retVal = reg_field_write(RTL8316D_UNIT, port+PORT0_TRAFFIC_STORM_CONTROL0, MCINCL, 1);
            break;
        case STORM_GROUP_BROADCAST:
            retVal = reg_field_write(RTL8316D_UNIT, port+PORT0_TRAFFIC_STORM_CONTROL0, BCINCL, 1);
            break;
        default:
            break;            
    }

    if (retVal != SUCCESS)
    {
        return retVal;
    }

    if ((retVal = reg_field_write(RTL8316D_UNIT, port+PORT0_TRAFFIC_STORM_CONTROL1, RATE, rate))!=SUCCESS)
    {
        return retVal;
    }
    
    return RT_ERR_OK;
}


/* Function Name:
 *      rtk_rate_stormControlRate_get
 * Description:
 *      get per-port packet storm filter control rate.
 * Input:
 *      port  - Port id.
 *      storm_type - Storm filter control type.
 *      pRate     - Rate of storm filter control.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK - Success.
 *      RT_ERR_SMI - SMI access error. 
 *      RT_ERR_PORT_ID - Invalid port number.
 *      RT_ERR_SFC_UNKNOWN_GROUP - unknown storm filter group.
 *      RT_ERR_INPUT - Invalid input parameters.
 *      RT_ERR_FAILED - Failure. 
 * Note:
 *      The storm filter control type can be:
 *          -STORM_GROUP_UNKNOWN_UNICAST 
 *          -STORM_GROUP_UNKNOWN_MULTICAST
 *          -STORM_GROUP_MULTICAST
 *          -STORM_GROUP_BROADCAST
 */
rtk_api_ret_t rtk_rate_stormControlRate_get(rtk_port_t port, rtk_rate_storm_group_t storm_type, rtk_rate_t *pRate, rtk_enable_t *pIfg_include)
{
    rtk_api_ret_t retVal;
    uint32 enable;
//    uint32 index;
//    uint32 regData;

    if(port > RTK_MAX_NUM_OF_PORT)
        return RT_ERR_PORT_ID;
    
    if(storm_type >= STORM_GROUP_END)
        return RT_ERR_SFC_UNKNOWN_GROUP;

    if (pRate == NULL)
    {
        return RT_ERR_INPUT;
    }

    if (pIfg_include == NULL)
    {
        return RT_ERR_INPUT;
    }

    if ((retVal = reg_field_read(RTL8316D_UNIT, TRAFFSTORM_SUPPRESSION_GLOBAL1, SCMETER_INC_IFG, pIfg_include)) != SUCCESS)
    {
        return retVal;
    }
    
    switch (storm_type)
    {
        case STORM_GROUP_UNKNOWN_UNICAST:
            if ((retVal = reg_field_read(RTL8316D_UNIT, port+PORT0_TRAFFIC_STORM_CONTROL0, UNUCINCL, &enable)) != SUCCESS )
            {
                return retVal;
            }
            if(enable)
            {
                retVal = reg_field_read(RTL8316D_UNIT, port+PORT0_TRAFFIC_STORM_CONTROL1, RATE, pRate);
            } else {
                *pRate = 0x3FFFFFFF;
            }
            break;
        case STORM_GROUP_UNKNOWN_MULTICAST:
            if ((retVal = reg_field_read(RTL8316D_UNIT, port+PORT0_TRAFFIC_STORM_CONTROL0, UNMCINCL, &enable)) != SUCCESS )
            {
                return retVal;
            }
            if(enable)
            {
                retVal = reg_field_read(RTL8316D_UNIT, port+PORT0_TRAFFIC_STORM_CONTROL1, RATE, pRate);
            } else {
                *pRate = 0x3FFFFFFF;
            }
            break;
        case STORM_GROUP_MULTICAST:
            if ((retVal = reg_field_read(RTL8316D_UNIT, port+PORT0_TRAFFIC_STORM_CONTROL0, MCINCL, &enable)) != SUCCESS )
            {
                return retVal;
            }
            if(enable)
            {
                retVal = reg_field_read(RTL8316D_UNIT, port+PORT0_TRAFFIC_STORM_CONTROL1, RATE, pRate);
            } else {
                *pRate = 0x3FFFFFFF;
            }
            break;
        case STORM_GROUP_BROADCAST:
            if ((retVal = reg_field_read(RTL8316D_UNIT, port+PORT0_TRAFFIC_STORM_CONTROL0, BCINCL, &enable)) != SUCCESS )
            {
                return retVal;
            }
            if(enable)
            {
                retVal = reg_field_read(RTL8316D_UNIT, port+PORT0_TRAFFIC_STORM_CONTROL1, RATE, pRate);
            } else {
                *pRate = 0x3FFFFFFF;
            }
            break;
            
        default:
            break;    
    }

    if (retVal != SUCCESS)
    {
        return retVal;
    }

    return RT_ERR_OK;
}


rtk_api_ret_t rtk_storm_controlRate_get(rtk_port_t port, rtk_rate_storm_group_t storm_type, rtk_rate_t *pRate, rtk_enable_t *pIfg_include, rtk_mode_t mode)
{
    rtk_api_ret_t ret;
    rtl8316d_storm_param_t info;

    if ((ret = rtl8316d_storm_get(port, &info)) != RT_ERR_OK)
        return ret;

    if (info.mode == 0)
    {
        return RT_ERR_FAILED;
    }

    *pRate = info.rate;
    switch (storm_type)
    {
    case STORM_GROUP_UNKNOWN_UNICAST:
        if (info.unknown_uni_inc == STORM_EXCLUDE)
            *pRate = STRM_CTRL_OFF;

        //printf("unknown_uni_inc: %ld\n", info.unknown_uni_inc);
        break;
    case STORM_GROUP_UNKNOWN_MULTICAST:
        if (info.unknown_mul_inc == STORM_EXCLUDE)
            *pRate = STRM_CTRL_OFF;

        //printf("unknown_mul_inc: %ld\n", info.unknown_mul_inc);
        break;
    case STORM_GROUP_MULTICAST:
        if (info.known_multi_inc == STORM_EXCLUDE)
            *pRate = STRM_CTRL_OFF;

        //printf("known_multi_inc: %ld\n", info.known_multi_inc);
        break;
    case STORM_GROUP_BROADCAST: 
        if (info.broad_inc == STORM_EXCLUDE)
            *pRate = STRM_CTRL_OFF;

        //printf("broad_inc: %ld\n", info.broad_inc);
        break;
    default:
        rtlglue_printf("Unknown Type!\n");
        return RT_ERR_FAILED;
    }  
    
    *pIfg_include = 0; // kill compiler warning
    mode = 0;   // kill compiler warning
    
    return RT_ERR_OK;
}

rtk_api_ret_t rtk_storm_controlRate_set(rtk_port_t port, rtk_rate_storm_group_t storm_type, rtk_rate_t rate, rtk_enable_t ifg_include, rtk_mode_t mode)
{
    rtk_api_ret_t ret;
    rtl8316d_storm_param_t info;

    if ((ret = rtl8316d_storm_get(port, &info)) != RT_ERR_OK)
        return ret;

    //printf("Strom Set: port: %ld, type: %ld Rate:%lx\n", (uint32)port, (uint32)storm_type, (uint32)rate);
    if (info.mode == 0)
    {
        rtlglue_printf("Invalid mode, please switch to kbps rather pps unit\n");
        return RT_ERR_FAILED;
    }

    switch (storm_type)
    {
    case STORM_GROUP_UNKNOWN_UNICAST:
        if (rate == STRM_CTRL_OFF)
            info.unknown_uni_inc = STORM_EXCLUDE;
        else
        {
            info.unknown_uni_inc = STORM_INCLUDE;
            info.rate = rate; 
        }

        break;
    case STORM_GROUP_UNKNOWN_MULTICAST:
        if (rate == STRM_CTRL_OFF)
            info.unknown_mul_inc = STORM_EXCLUDE;
        else
        {
            info.unknown_mul_inc = STORM_INCLUDE;
            info.rate = rate; 
        }

        break;
    case STORM_GROUP_MULTICAST:
        if (rate == STRM_CTRL_OFF)
            info.known_multi_inc = STORM_EXCLUDE;
        else
        {
            info.known_multi_inc = STORM_INCLUDE;
            info.rate = rate; 
        }
        
        break;
    case STORM_GROUP_BROADCAST: 
        if (rate == STRM_CTRL_OFF)
            info.broad_inc = STORM_EXCLUDE;
        else
        {
            info.broad_inc = STORM_INCLUDE;
            info.rate = rate; 
        }

        break;
    default:
        rtlglue_printf("Unknown Type!\n");
        return RT_ERR_FAILED;
    }  

    if ((ret = rtl8316d_storm_set(port, &info)) != RT_ERR_OK)
        return ret;
    
    ifg_include = 0; // kill compiler warning
    mode = 0;   // kill compiler warning

    return RT_ERR_OK;
}

rtk_api_ret_t rtl8316d_storm_get(uint32 port, rtl8316d_storm_param_t * storminfo)
{
    uint32 reg_val;
    uint32 retVal;

    if (port >= RTL8316D_MAX_PORT)
        return RT_ERR_PORT_ID;

    if((retVal=reg_field_read(RTL8316D_UNIT, PORT0_TRAFFIC_STORM_CONTROL0 + port, UNIT, &reg_val))!=RT_ERR_OK)
        return retVal;

    storminfo->mode = reg_val;

    if((retVal=reg_field_read(RTL8316D_UNIT, PORT0_TRAFFIC_STORM_CONTROL0 + port, BCINCL, &reg_val))!=RT_ERR_OK)
        return retVal;

    storminfo->broad_inc = reg_val;

    if((retVal=reg_field_read(RTL8316D_UNIT, PORT0_TRAFFIC_STORM_CONTROL0 + port, UNUCINCL, &reg_val))!=RT_ERR_OK)
        return retVal;

    storminfo->unknown_uni_inc = reg_val;

    if((retVal=reg_field_read(RTL8316D_UNIT, PORT0_TRAFFIC_STORM_CONTROL0 + port, MCINCL, &reg_val))!=RT_ERR_OK)
        return retVal;

    storminfo->known_multi_inc = reg_val;

    if((retVal=reg_field_read(RTL8316D_UNIT, PORT0_TRAFFIC_STORM_CONTROL0 + port, UNMCINCL, &reg_val))!=RT_ERR_OK)
        return retVal;

    storminfo->unknown_mul_inc = reg_val;

    if((retVal=reg_field_read(RTL8316D_UNIT, PORT0_TRAFFIC_STORM_CONTROL1 + port, RATE, &reg_val))!=RT_ERR_OK)
        return retVal;

    storminfo->rate = reg_val; 

    return RT_ERR_OK;
}


rtk_api_ret_t rtl8316d_storm_set(uint32 port, rtl8316d_storm_param_t * storminfo)
{
    uint32 reg_val;
    uint32 retVal;

    if (port >= RTL8316D_MAX_PORT)
        return RT_ERR_PORT_ID;

    reg_val = storminfo->mode;
    if((retVal=reg_field_write(RTL8316D_UNIT, PORT0_TRAFFIC_STORM_CONTROL0 + port, UNIT, reg_val))!=RT_ERR_OK)
        return retVal;

    reg_val = storminfo->broad_inc;
    if((retVal=reg_field_write(RTL8316D_UNIT, PORT0_TRAFFIC_STORM_CONTROL0 + port, BCINCL, reg_val))!=RT_ERR_OK)
        return retVal;

    reg_val = storminfo->unknown_uni_inc;
    if((retVal=reg_field_write(RTL8316D_UNIT, PORT0_TRAFFIC_STORM_CONTROL0 + port, UNUCINCL, reg_val))!=RT_ERR_OK)
        return retVal;

    reg_val = storminfo->known_multi_inc;
    if((retVal=reg_field_write(RTL8316D_UNIT, PORT0_TRAFFIC_STORM_CONTROL0 + port, MCINCL, reg_val))!=RT_ERR_OK)
        return retVal;

    reg_val = storminfo->unknown_mul_inc;
    if((retVal=reg_field_write(RTL8316D_UNIT, PORT0_TRAFFIC_STORM_CONTROL0 + port, UNMCINCL, reg_val))!=RT_ERR_OK)
        return retVal;

    reg_val = storminfo->rate;
    if((retVal=reg_field_write(RTL8316D_UNIT, PORT0_TRAFFIC_STORM_CONTROL1 + port, RATE, reg_val))!=RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}


/* Function Name:
 *      rtk_qos_priSel_set
 * Description:
 *      Configure the priority order among different priority mechanism.
 * Input:
 *      pPriDec  - Priority assign for port, dscp, 802.1q, cvlan, svlan priority decision 
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK - Success.
 *      RT_ERR_SMI - SMI access error.
 *      RT_ERR_INPUT - Invalid input parameters.
 *      RT_ERR_QOS_SEL_PRI_SOURCE - Invalid priority decision source parameter.
 *      RT_ERR_FAILED - Failure.
 * Note:
 *      ASIC will follow user priority setting of mechanisms to select mapped queue priority for receiving frame. 
 *      If two priority mechanisms are the same, the ASIC will chose the highest priority from mechanisms to 
 *      assign queue priority to receiving frame.  
 *      The priority sources are:
 *          -PRIDEC_PORT
 *          -PRIDEC_1Q
 *          -PRIDEC_DSCP
 *          -PRIDEC_CVLAN
 *          -PRIDEC_SVLAN
 *    RTL8316D has two groups of priority selection.
 */
rtk_api_ret_t rtk_qos_priSel_set(rtk_priority_select_t *pPriDec)
{ 
    rtk_api_ret_t retVal;
    
    if(pPriDec->port_pri >= 8 || pPriDec->dot1q_pri >= 8 || pPriDec->dscp_pri >= 8 ||
       pPriDec->cvlan_pri >= 8 || pPriDec->svlan_pri >= 8)
        return RT_ERR_QOS_SEL_PRI_SOURCE;

    if (pPriDec->group >= 2)
        return RT_ERR_QOS_SEL_PRI_GROUP;

    if ((retVal = reg_field_write(RTL8316D_UNIT, PRIORITY_ARBITRATION_CONTROL0+pPriDec->group, OTAG_PRIO, pPriDec->svlan_pri)) != SUCCESS)
        return retVal;
    if ((retVal = reg_field_write(RTL8316D_UNIT, PRIORITY_ARBITRATION_CONTROL0+pPriDec->group, ITAG_PRIO, pPriDec->cvlan_pri)) != SUCCESS)
        return retVal;
    if ((retVal = reg_field_write(RTL8316D_UNIT, PRIORITY_ARBITRATION_CONTROL0+pPriDec->group, DSCP_PRIO, pPriDec->dscp_pri)) != SUCCESS)
        return retVal;
    if ((retVal = reg_field_write(RTL8316D_UNIT, PRIORITY_ARBITRATION_CONTROL0+pPriDec->group, DOT1QBP_PRIO, pPriDec->dot1q_pri)) != SUCCESS)
        return retVal;
    if ((retVal = reg_field_write(RTL8316D_UNIT, PRIORITY_ARBITRATION_CONTROL0+pPriDec->group, PBP_PRIO, pPriDec->port_pri)) != SUCCESS)
        return retVal;    

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_qos_priSel_get
 * Description:
 *      Get the priority order configuration among different priority mechanism.
 * Input:
 *      None
 * Output:
 *      pPriDec  - Priority assign for port, dscp, 802.1p, cvlan, svlan, acl based priority decision .
 * Return:
 *      RT_ERR_OK | Success.
 *      RT_ERR_SMI | SMI access error.
 *      RT_ERR_INPUT | Invalid input parameters.
 *      RT_ERR_FAILED | Failure. 
 * Note:
 *      ASIC will follow user priority setting of mechanisms to select mapped queue priority for receiving frame. 
 *      If two priority mechanisms are the same, the ASIC will chose the highest priority from mechanisms to 
 *      assign queue priority to receiving frame. 
 *      The priority sources are:
 *      -PRIDEC_PORT,
 *      -PRIDEC_DSCP,
 *      -PRIDEC_1Q,
 *      -PRIDEC_CVLAN,
 *      -PRIDEC_SVLAN,
 *    RTL8316D has two groups of priority selection.
 */
rtk_api_ret_t rtk_qos_priSel_get(rtk_priority_select_t *pPriDec)
{
    rtk_api_ret_t retVal;

    if (pPriDec == NULL)
    {
        return RT_ERR_NULL_POINTER;
    }

    if (pPriDec->group >= 2)
        return RT_ERR_QOS_SEL_PRI_GROUP;

    if ((retVal = reg_field_read(RTL8316D_UNIT, PRIORITY_ARBITRATION_CONTROL0+pPriDec->group, OTAG_PRIO, &pPriDec->svlan_pri)) != SUCCESS)
        return retVal;
    if ((retVal = reg_field_read(RTL8316D_UNIT, PRIORITY_ARBITRATION_CONTROL0+pPriDec->group, ITAG_PRIO, &pPriDec->cvlan_pri)) != SUCCESS)
        return retVal;
    if ((retVal = reg_field_read(RTL8316D_UNIT, PRIORITY_ARBITRATION_CONTROL0+pPriDec->group, DSCP_PRIO, &pPriDec->dscp_pri)) != SUCCESS)
        return retVal;
    if ((retVal = reg_field_read(RTL8316D_UNIT, PRIORITY_ARBITRATION_CONTROL0+pPriDec->group, DOT1QBP_PRIO, &pPriDec->dot1q_pri)) != SUCCESS)
        return retVal;
    if ((retVal = reg_field_read(RTL8316D_UNIT, PRIORITY_ARBITRATION_CONTROL0+pPriDec->group, PBP_PRIO, &pPriDec->port_pri)) != SUCCESS)
        return retVal;
    
    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_qos_portPriSel_set
 * Description:
 *      Select priority selection group.
 * Input:
 *      port - port ID
 *      group - group ID
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK - Success.
 *      RT_ERR_SMI - SMI access error.
 *      RT_ERR_INPUT - Invalid input parameters.
 *      RT_ERR_QOS_SEL_PRI_SOURCE - Invalid priority decision source parameter.
 *      RT_ERR_FAILED - Failure. 
 * Note:
 *      RTL8316D has two groups of priority selection, and each port could select one group for priority arbitration.
 */
rtk_api_ret_t rtk_qos_portPriSel_set(rtk_port_t port, uint8 group)
{ 
    rtk_api_ret_t retVal;

    if(port > RTL8316D_MAX_PORT)
        return RT_ERR_PORT_ID; 

    if (group >= 2)
    {
        return RT_ERR_QOS_SEL_PRI_GROUP;
    }

    retVal = reg_field_write(RTL8316D_UNIT, PORT0_PRIORITY_CONTROL+port, PRIWGTTBLIDX, group);

    return retVal;
}


/* Function Name:
 *      rtk_qos_portPriSel_get
 * Description:
 *      Get priority selection group.
 * Input:
 *      port - port ID
 * Output:
 *      pGroup - group IDNone
 * Return:
 *      RT_ERR_OK - Success.
 *      RT_ERR_SMI - SMI access error.
 *      RT_ERR_INPUT - Invalid input parameters.
 *      RT_ERR_QOS_SEL_PRI_SOURCE - Invalid priority decision source parameter.
 *      RT_ERR_FAILED - Failure. 
 * Note:
 *      RTL8316D has two groups of priority selection, and each port could select one group for priority arbitration.
 */
rtk_api_ret_t rtk_qos_portPriSel_get(rtk_port_t port, uint8* pGroup)
{ 
    rtk_api_ret_t retVal;
    uint32 regVal;

    if(port > RTL8316D_MAX_PORT)
        return RT_ERR_PORT_ID; 

    if (pGroup == NULL)
    {
        return RT_ERR_NULL_POINTER;
    }

    retVal = reg_field_read(RTL8316D_UNIT, PORT0_PRIORITY_CONTROL+port, PRIWGTTBLIDX, &regVal);
    *pGroup = regVal;

    return retVal;
}

/* Function Name:
 *      rtk_qos_portCvlanPriRemapSel_set
 * Description:
 *      Select CVLAN priority remapping group.
 * Input:
 *      port - port ID
 *      group - group ID
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK - Success.
 *      RT_ERR_SMI - SMI access error.
 *      RT_ERR_INPUT - Invalid input parameters.
 *      RT_ERR_QOS_SEL_PRI_SOURCE - Invalid priority decision source parameter.
 *      RT_ERR_FAILED - Failure. 
 * Note:
 *      RTL8316D has two groups of CVLAN priority remapping, and each port could select one group for priority remapping.
 */
rtk_api_ret_t rtk_qos_portCvlanPriRemapSel_set(rtk_port_t port, uint8 group)
{ 
    rtk_api_ret_t retVal;

    if(port > RTL8316D_MAX_PORT)
        return RT_ERR_PORT_ID; 

    if (group >= 2)
    {
        return RT_ERR_QOS_SEL_PRI_GROUP;
    }

    retVal = reg_field_write(RTL8316D_UNIT, PORT0_PRIORITY_CONTROL+port, IPRIDPTBLIDX, group);

    return retVal;
}


/* Function Name:
 *      rtk_qos_portCvlanPriRemapSel_get
 * Description:
 *      get CVLAN priority remapping group
 * Input:
 *      port - port ID
 * Output:
 *      pGroup - group ID
 * Return:
 *      RT_ERR_OK - Success.
 *      RT_ERR_SMI - SMI access error.
 *      RT_ERR_INPUT - Invalid input parameters.
 *      RT_ERR_QOS_SEL_PRI_SOURCE - Invalid priority decision source parameter.
 *      RT_ERR_FAILED - Failure. 
 * Note:
 *      RTL8316D has two groups of CVLAN priority remapping, and each port could select one group for priority remapping.
 */
rtk_api_ret_t rtk_qos_portCvlanPriRemapSel_get(rtk_port_t port, uint8* pGroup)
{ 
    rtk_api_ret_t retVal;
    uint32 regVal;

    if(port > RTL8316D_MAX_PORT)
        return RT_ERR_PORT_ID; 

    if (pGroup == NULL)
    {
        return RT_ERR_NULL_POINTER;
    }

    retVal = reg_field_read(RTL8316D_UNIT, PORT0_PRIORITY_CONTROL+port, IPRIDPTBLIDX, &regVal);
    *pGroup = regVal;

    return retVal;
}


/* Function Name:
 *      rtk_qos_portSvlanPriRemapSel_set
 * Description:
 *      get CVLAN priority remapping group
 * Input:
 *      port - port ID
 *      group - group ID
 * Output:
 *      None 
 * Return:
 *      RT_ERR_OK - Success.
 *      RT_ERR_SMI - SMI access error.
 *      RT_ERR_INPUT - Invalid input parameters.
 *      RT_ERR_QOS_SEL_PRI_SOURCE - Invalid priority decision source parameter.
 *      RT_ERR_FAILED - Failure. 
 * Note:
 *      RTL8316D has two groups of SVLAN priority remapping, and each port could select one group for priority remapping.
 */
rtk_api_ret_t rtk_qos_portSvlanPriRemapSel_set(rtk_port_t port, uint8 group)
{ 
    rtk_api_ret_t retVal;

    if(port > RTL8316D_MAX_PORT)
        return RT_ERR_PORT_ID; 

    if (group >= 2)
    {
        return RT_ERR_QOS_SEL_PRI_GROUP;
    }

    retVal = reg_field_write(RTL8316D_UNIT, PORT0_PRIORITY_CONTROL+port, OPRIDPTBLIDX, group);

    return retVal;
}

/* Function Name:
 *      rtk_qos_portSvlanPriRemapSel_get
 * Description:
 *      get SVLAN priority remapping group
 * Input:
 *      port - port ID 
 * Output:
 *      pGroup - group ID
 * Return:
 *      RT_ERR_OK - Success.
 *      RT_ERR_SMI - SMI access error.
 *      RT_ERR_INPUT - Invalid input parameters.
 *      RT_ERR_QOS_SEL_PRI_SOURCE - Invalid priority decision source parameter.
 *      RT_ERR_FAILED - Failure. 
 * Note:
 *      RTL8316D has two groups of SVLAN priority remapping, and each port could select one group for SVLAN priority remapping.
 */
rtk_api_ret_t rtk_qos_portSvlanPriRemapSel_get(rtk_port_t port, uint8* pGroup)
{ 
    rtk_api_ret_t retVal;
    uint32 regVal;

    if(port > RTL8316D_MAX_PORT)
        return RT_ERR_PORT_ID; 

    if (pGroup == NULL)
    {
        return RT_ERR_NULL_POINTER;
    }

    retVal = reg_field_read(RTL8316D_UNIT, PORT0_PRIORITY_CONTROL+port, OPRIDPTBLIDX, &regVal);
    *pGroup = regVal;

    return retVal;
}

/* Function Name:
 *      rtk_qos_portDscpPriRemapSel_set
 * Description:
 *      get SVLAN priority remapping group
 * Input:
 *      port - port ID 
 *      group - group ID
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK - Success.
 *      RT_ERR_SMI - SMI access error.
 *      RT_ERR_INPUT - Invalid input parameters.
 *      RT_ERR_QOS_SEL_PRI_SOURCE - Invalid priority decision source parameter.
 *      RT_ERR_FAILED - Failure. 
 * Note:
 *      RTL8316D has two groups of DSCP priority remapping, and each port could select one group for priority remapping.
 */
rtk_api_ret_t rtk_qos_portDscpPriRemapSel_set(rtk_port_t port, uint8 group)
{ 
    rtk_api_ret_t retVal;

    if(port > RTL8316D_MAX_PORT)
        return RT_ERR_PORT_ID; 

    if (group >= 2)
    {
        return RT_ERR_QOS_SEL_PRI_GROUP;
    }

    retVal = reg_field_write(RTL8316D_UNIT, PORT0_PRIORITY_CONTROL+port, DSCPPRIDPTBLIDX, group);

    return retVal;
}

/* Function Name:
 *      rtk_qos_portDscpPriRemapSel_get
 * Description:
 *      get DSCP priority remapping group
 * Input:
 *      port - port ID 
 * Output:
 *       pGroup - group ID    
 * Return:
 *      RT_ERR_OK - Success.
 *      RT_ERR_SMI - SMI access error.
 *      RT_ERR_INPUT - Invalid input parameters.
 *      RT_ERR_QOS_SEL_PRI_SOURCE - Invalid priority decision source parameter.
 *      RT_ERR_FAILED - Failure. 
 * Note:
 *      RTL8316D has two groups of DSCP priority remapping, and each port could select one group for DSCP priority remapping.
 */
rtk_api_ret_t rtk_qos_portDscpPriRemapSel_get(rtk_port_t port, uint8* pGroup)
{ 
    rtk_api_ret_t retVal;
    uint32 regVal;

    if(port > RTL8316D_MAX_PORT)
        return RT_ERR_PORT_ID; 

    if (pGroup == NULL)
    {
        return RT_ERR_NULL_POINTER;
    }

    retVal = reg_field_read(RTL8316D_UNIT, PORT0_PRIORITY_CONTROL+port, DSCPPRIDPTBLIDX, &regVal);
    *pGroup = regVal;

    return retVal;
}


/* Function Name:
 *      rtk_qos_portPri_set
 * Description:
 *      Configure priority usage to each port.
 * Input:
 *      port - port ID 
 *      int_pri - internal priority value. 
 * Output:
 *       None    
 * Return:
 *      RT_ERR_OK - Success.
 *      RT_ERR_SMI - SMI access error.
 *      RT_ERR_INPUT - Invalid input parameters.
 *      RT_ERR_QOS_SEL_PRI_SOURCE - Invalid priority decision source parameter.
 *      RT_ERR_FAILED - Failure. 
 * Note:
 *      The API can set priority of port assignments for queue usage and packet scheduling.
 */
rtk_api_ret_t rtk_qos_portPri_set(rtk_port_t port, rtk_pri_t int_pri)
{
    rtk_api_ret_t retVal;

    if(port > RTL8316D_MAX_PORT)
        return RT_ERR_PORT_ID; 

    if (int_pri > 0x7 )
        return RT_ERR_QOS_INT_PRIORITY; 

    if((retVal= reg_field_write(RTL8316D_UNIT, PORT0_PRIORITY_CONTROL+port, PORTPRIORITY, int_pri))!=RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}



/* Function Name:
 *      rtk_qos_portPri_get
 * Description:
 *      Get priority usage to each port.
 * Input:
 *      port - port ID 
 * Output:
 *      pInt_pri - internal priority value.     
 * Return:
 *      RT_ERR_OK - Success.
 *      RT_ERR_SMI - SMI access error.
 *      RT_ERR_INPUT - Invalid input parameters.
 *      RT_ERR_QOS_SEL_PRI_SOURCE - Invalid priority decision source parameter.
 *      RT_ERR_FAILED - Failure. 
 * Note:
 *      The API can set priority of port assignments for queue usage and packet scheduling.
 */
rtk_api_ret_t rtk_qos_portPri_get(rtk_port_t port, rtk_pri_t *pInt_pri)
{
    rtk_api_ret_t retVal;
    
    if(port > RTL8316D_MAX_PORT)
        return RT_ERR_PORT_ID; 

    if (pInt_pri == NULL)
        return RT_ERR_NULL_POINTER;
    
    if((retVal= reg_field_read(RTL8316D_UNIT, PORT0_PRIORITY_CONTROL+port, PORTPRIORITY, pInt_pri))!=RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}



/* Function Name:
 *      rtk_qos_cvlanPriRemap_set
 * Description:
 *      CVLAN priority remapping configuration
 * Input:
 *      int_pri - internal priority value.
 *      dot1p_pri - 802.1p priority value.
 *      group    - group ID
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK - Success.
 *      RT_ERR_SMI - SMI access error.
 *      RT_ERR_INPUT - Invalid input parameters.
 *      RT_ERR_QOS_SEL_PRI_SOURCE - Invalid priority decision source parameter.
 *      RT_ERR_FAILED - Failure. 
 * Note:
 *      Priority of 802.1Q assignment for internal asic priority, and it is used for queue usage and packet scheduling.
 */
rtk_api_ret_t rtk_qos_cvlanPriRemap_set(uint8 group, rtk_pri_t int_pri, rtk_pri_t dot1p_pri)
{ 
    rtk_api_ret_t retVal;
    uint32 regVal;    

    if(group > 1)
        return RT_ERR_INPUT; 

    if (int_pri>7 || dot1p_pri>7)
        return  RT_ERR_QOS_INT_PRIORITY;

    retVal = reg_read(RTL8316D_UNIT, INNER_TAG_PRIORITY_MAP_CONTROL0+group, &regVal);
    if (retVal != SUCCESS)
        return retVal;
    
    regVal &= ~(0x7 << (dot1p_pri*3));
    regVal |= int_pri << (dot1p_pri*3);
    retVal = reg_write(RTL8316D_UNIT, INNER_TAG_PRIORITY_MAP_CONTROL0+group, regVal);
    
    return retVal;
}


/* Function Name:
 *      rtk_qos_cvlanPriRemap_get
 * Description:
 *      get CVLAN priority remapping configuration
 * Input:
 *      dot1p_pri - 802.1p priority value.
 *      group    - group ID
 * Output:
 *      PInt_pri -  internal priority value.
 * Return:
 *      RT_ERR_OK - Success.
 *      RT_ERR_SMI - SMI access error.
 *      RT_ERR_INPUT - Invalid input parameters.
 *      RT_ERR_QOS_SEL_PRI_SOURCE - Invalid priority decision source parameter.
 *      RT_ERR_FAILED - Failure. 
 * Note:
 *      Priority of 802.1Q assignment for internal asic priority, and it is used for queue usage and packet scheduling.
 */
rtk_api_ret_t rtk_qos_cvlanPriRemap_get(uint8 group, rtk_pri_t* PInt_pri, rtk_pri_t dot1p_pri)
{ 
    rtk_api_ret_t retVal;
    uint32 regVal;    

    if(group > 1)
        return RT_ERR_INPUT; 

    if (dot1p_pri>7)
        return  RT_ERR_QOS_INT_PRIORITY;
    if (PInt_pri == NULL)
        return RT_ERR_NULL_POINTER;

    retVal = reg_read(RTL8316D_UNIT, INNER_TAG_PRIORITY_MAP_CONTROL0+group, &regVal);
    *PInt_pri = (regVal >> (3*dot1p_pri)) & 0x7;
    
    return retVal;
}

/* Function Name:
 *      rtk_qos_svlanPriRemap_set
 * Description:
 *      get CVLAN priority remapping configuration
 * Input:
 *      dot1p_pri - 802.1p priority value.
 *      int_pri    -internal priority value.
 *      dei    - DEI value.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK - Success.
 *      RT_ERR_SMI - SMI access error.
 *      RT_ERR_INPUT - Invalid input parameters.
 *      RT_ERR_QOS_SEL_PRI_SOURCE - Invalid priority decision source parameter.
 *      RT_ERR_FAILED - Failure. 
 * Note:
 *      Priority of 802.1Q assignment for internal asic priority, and it is used for queue usage and packet scheduling. 
 */
rtk_api_ret_t rtk_qos_svlanPriRemap_set(uint8 group, rtk_pri_t int_pri, rtk_pri_t dot1p_pri, rtk_dei_t dei)
{ 
    rtk_api_ret_t retVal;
    uint32 regVal;    

    if(group > 1)
        return RT_ERR_INPUT; 

    if (int_pri>7 || dot1p_pri>7)
        return  RT_ERR_QOS_INT_PRIORITY;

    retVal = reg_read(RTL8316D_UNIT, OUTER_TAG_PRIORITY_MAP_CONTROL0+group+dei, &regVal);
    if (retVal != SUCCESS)
        return retVal;
    
    regVal &= ~(0x7 << (dot1p_pri*3));
    regVal |= int_pri << (dot1p_pri*3);
    retVal = reg_write(RTL8316D_UNIT, OUTER_TAG_PRIORITY_MAP_CONTROL0+group+dei, regVal);
    
    return retVal;
}


/* Function Name:
 *      rtk_qos_svlanPriRemap_get
 * Description:
 *      get SVLAN priority remapping configuration
 * Input:
 *      dot1p_pri - 802.1p priority value.
 *      dei    - DEI value.
 *      group    -group ID
 * Output:
 *      PInt_pri -  internal priority value.
 * Return:
 *      RT_ERR_OK - Success.
 *      RT_ERR_SMI - SMI access error.
 *      RT_ERR_INPUT - Invalid input parameters.
 *      RT_ERR_QOS_SEL_PRI_SOURCE - Invalid priority decision source parameter.
 *      RT_ERR_FAILED - Failure. 
 * Note:
 *     Priority of 802.1Q assignment for internal asic priority, and it is used for queue usage and packet scheduling.
 */
rtk_api_ret_t rtk_qos_svlanPriRemap_get(uint8 group, rtk_pri_t dot1p_pri, rtk_dei_t dei, rtk_pri_t* PInt_pri)
{ 
    rtk_api_ret_t retVal;
    uint32 regVal;    

    if(group > 1)
        return RT_ERR_INPUT; 

    if (dot1p_pri>7)
        return  RT_ERR_QOS_INT_PRIORITY;
    if (PInt_pri == NULL)
        return RT_ERR_NULL_POINTER;

    retVal = reg_read(RTL8316D_UNIT, OUTER_TAG_PRIORITY_MAP_CONTROL0+group+dei, &regVal);
    *PInt_pri = (regVal >> (3*dot1p_pri)) & 0x7;
    
    return retVal;
}


/* Function Name:
 *      rtk_qos_dscpPriRemap_set
 * Description:
 *      Map dscp value to internal priority.
 * Input:
 *      group - remapping group, RTL8316D has two remapping group
 *      dscp - Dscp value of receiving frame
 *      int_pri    -  internal priority value . 
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK - Success.
 *      RT_ERR_SMI - SMI access error.
 *      RT_ERR_INPUT - Invalid input parameters.
 *      RT_ERR_QOS_INT_PRIORITY - Invalid priority.
 *      RT_ERR_FAILED - Failure. 
 * Note:
 *      The Differentiated Service Code Point is a selector for router's per-hop behaviors. As a selector, there is no implication that a numerically 
 *      greater DSCP implies a better network service. As can be seen, the DSCP totally overlaps the old precedence field of TOS. So if values of 
 *      DSCP are carefully chosen then backward compatibility can be achieved.    
 */
rtk_api_ret_t rtk_qos_dscpPriRemap_set(uint8 group, rtk_pri_t int_pri, rtk_dscp_t dscp)
{ 
    rtk_api_ret_t retVal;
    uint32 regVal; 
    rtk_reg_list_t regAddr;

    if(group > 1)
        return RT_ERR_INPUT; 

    if (int_pri>7)
        return  RT_ERR_QOS_INT_PRIORITY;
    if (dscp>63)
        return RT_ERR_QOS_DSCP_VALUE;

    regAddr = DSCP_PRIORITY_MAP_CONTROL0 + dscp/8;
    
    retVal = reg_read(RTL8316D_UNIT, DSCP_PRIORITY_MAP_CONTROL0 + dscp/8 + group*8, &regVal);
    if (retVal != SUCCESS)
        return retVal;
    
    regVal &= ~(0xF << (dscp%8*4));
    regVal |= int_pri << (dscp%8*4);
    retVal = reg_write(RTL8316D_UNIT, DSCP_PRIORITY_MAP_CONTROL0 + dscp/8 + group*8, regVal);
    
    return retVal;
}


/* Function Name:
 *      rtk_qos_dscpPriRemap_get
 * Description:
 *      Get dscp value to internal priority.
 * Input:
 *      group - remapping group, RTL8316D has two remapping group
 *      dscp - Dscp value of receiving frame
 * Output:
 *      pInt_pri - internal priority value.
 * Return:
 *      RT_ERR_OK - Success.
 *      RT_ERR_SMI - SMI access error.
 *      RT_ERR_INPUT - Invalid input parameters.
 *      RT_ERR_QOS_INT_PRIORITY - Invalid priority.
 *      RT_ERR_FAILED - Failure. 
 * Note:
 *      The Differentiated Service Code Point is a selector for router's per-hop behaviors. As a selector, there is no implication that a numerically 
 *      greater DSCP implies a better network service. As can be seen, the DSCP totally overlaps the old precedence field of TOS. So if values of 
 *      DSCP are carefully chosen then backward compatibility can be achieved.    
 */
rtk_api_ret_t rtk_qos_dscpPriRemap_get(uint8 group, rtk_pri_t* pInt_pri, rtk_dscp_t dscp)
{ 
    rtk_api_ret_t retVal;
    uint32 regVal; 
    rtk_reg_list_t regAddr;

    if(group > 1)
        return RT_ERR_INPUT; 

    if (pInt_pri == NULL)
        return  RT_ERR_NULL_POINTER;

    if (dscp>63)
        return RT_ERR_QOS_DSCP_VALUE;

    regAddr = DSCP_PRIORITY_MAP_CONTROL0 + dscp/8;
    
    retVal = reg_read(RTL8316D_UNIT, DSCP_PRIORITY_MAP_CONTROL0 + dscp/8 + group*8, &regVal);
    *pInt_pri = (regVal >> (dscp%8*4)) & 0x7;
    
    return retVal;
}



/* Function Name:
 *      rtk_qos_portSvlanPriCopy_set
 * Description:
 *      SVLAN tag priority could be copied from priority field of SVLAN tag or CVLAN tag.
 * Input:
 *      port - port ID
 *      source - 0: do not copy, 1: copy from priority field of CVLAN tag, 2: copy from priority field of SVLAN tag
 * Output:
 *     None
 * Return:
 *      RT_ERR_OK - Success.
 *      RT_ERR_SMI - SMI access error.
 *      RT_ERR_INPUT - Invalid input parameters.
 *      RT_ERR_QOS_SEL_PRI_SOURCE -  Invalid priority decision source parameter.
 *      RT_ERR_FAILED - Failure. 
 * Note:
 *      The value of priority in SVLAN tag could be copied from priority field of SVLAN tag or CVLAN tag on a per-rx-port basis.
 */
rtk_api_ret_t rtk_qos_portSvlanPriCopy_set(rtk_port_t port, uint8 source)
{ 
    rtk_api_ret_t retVal;

    if(port > RTL8316D_MAX_PORT)
        return RT_ERR_PORT_ID; 

    if (source >= 3)
    {
        return RT_ERR_INPUT;
    }

    retVal = reg_field_write(RTL8316D_UNIT, PORT0_RX_PRIORITY_COPY_CONTROL+port, SELOPRI, source);

    return retVal;
}


/* Function Name:
 *      rtk_qos_portSvlanPriCopy_get
 * Description:
 *      SVLAN tag priority could be copied from priority field of SVLAN tag or CVLAN tag.
 * Input:
 *      port - port ID
 * Output:
 *     pSource - 0: do not copy, 1: copy from priority field of CVLAN tag, 2: copy from priority field of SVLAN tag
 * Return:
 *      RT_ERR_OK - Success.
 *      RT_ERR_SMI - SMI access error.
 *      RT_ERR_INPUT - Invalid input parameters.
 *      RT_ERR_QOS_SEL_PRI_SOURCE -  Invalid priority decision source parameter.
 *      RT_ERR_FAILED - Failure. 
 * Note:
 *       The value of priority in SVLAN tag could be copied from priority field of SVLAN tag or CVLAN tag on a per-rx-port basis.
 */
rtk_api_ret_t rtk_qos_portSvlanPriCopy_get(rtk_port_t port, uint8* pSource)
{ 
    rtk_api_ret_t retVal;
    uint32 regVal;

    if(port > RTL8316D_MAX_PORT)
        return RT_ERR_PORT_ID; 

    if (pSource == NULL)
    {
        return RT_ERR_NULL_POINTER;
    }

    retVal = reg_field_read(RTL8316D_UNIT, PORT0_RX_PRIORITY_COPY_CONTROL+port, SELOPRI, &regVal);
    *pSource = regVal;

    return retVal;
}

/* Function Name:
 *      rtk_qos_portCvlanPriCopy_set
 * Description:
 *      CVLAN tag priority could be copied from priority field of SVLAN tag or CVLAN tag.
 * Input:
 *      port - port ID
 *      source - 0: do not copy, 1: copy from priority field of CVLAN tag, 2: copy from priority field of SVLAN tag
 * Output:
 *     None
 * Return:
 *      RT_ERR_OK - Success.
 *      RT_ERR_SMI - SMI access error.
 *      RT_ERR_INPUT - Invalid input parameters.
 *      RT_ERR_QOS_SEL_PRI_SOURCE -  Invalid priority decision source parameter.
 *      RT_ERR_FAILED - Failure. 
 * Note:
 *       The value of priority in CVLAN tag could be copied from priority field of SVLAN tag or CVLAN tag on a per-rx-port basis.
 */
rtk_api_ret_t rtk_qos_portCvlanPriCopy_set(rtk_port_t port, uint8 source)
{ 
    rtk_api_ret_t retVal;

    if(port > RTL8316D_MAX_PORT)
        return RT_ERR_PORT_ID; 

    if (source >= 3)
    {
        return RT_ERR_INPUT;
    }

    retVal = reg_field_write(RTL8316D_UNIT, PORT0_RX_PRIORITY_COPY_CONTROL+port, SELIPRI, source);

    return retVal;
}

/* Function Name:
 *      rtk_qos_portCvlanPriCopy_get
 * Description:
 *      CVLAN tag priority could be copied from priority field of SVLAN tag or CVLAN tag.
 * Input:
 *      port - port ID
 * Output:
 *     pSource - 0: do not copy, 1: copy from priority field of CVLAN tag, 2: copy from priority field of SVLAN tag
 * Return:
 *      RT_ERR_OK - Success.
 *      RT_ERR_SMI - SMI access error.
 *      RT_ERR_INPUT - Invalid input parameters.
 *      RT_ERR_QOS_SEL_PRI_SOURCE -  Invalid priority decision source parameter.
 *      RT_ERR_FAILED - Failure. 
 * Note:
 *       The value of priority in CVLAN tag could be copied from priority field of SVLAN tag or CVLAN tag on a per-rx-port basis.
 */
rtk_api_ret_t rtk_qos_portCvlanPriCopy_get(rtk_port_t port, uint8* pSource)
{ 
    rtk_api_ret_t retVal;
    uint32 regVal;

    if(port > RTL8316D_MAX_PORT)
        return RT_ERR_PORT_ID; 

    if (pSource == NULL)
    {
        return RT_ERR_NULL_POINTER;
    }

    retVal = reg_field_read(RTL8316D_UNIT, PORT0_RX_PRIORITY_COPY_CONTROL+port, SELIPRI, &regVal);
    *pSource = regVal;

    return retVal;
}

/* Function Name:
 *      rtk_qos_dscpRemarkEnable_set
 * Description:
 *      Set DSCP remarking ability.
 * Input:
 *      port - port ID
 *      enable - status of dscp remark.
 * Output:
 *     None
 * Return:
 *      RT_ERR_OK - Success.
 *      RT_ERR_SMI - SMI access error.
 *      RT_ERR_INPUT - Invalid input parameters.
 *      RT_ERR_PORT_ID -  Invalid port number.
 *      RT_ERR_FAILED - Failure. 
 *      RT_ERR_ENABLE - Invalid enable parameter.
 * Note:
 *      The API can enable or disable DSCP remarking ability.
 *      The status of DSCP remark:
 *          DISABLED
 *          ENABLED
 */
rtk_api_ret_t rtk_qos_dscpRemarkEnable_set(rtk_port_t port, rtk_enable_t enable)
{
    rtk_api_ret_t retVal;
    
    if(port > RTL8316D_MAX_PORT)
        return RT_ERR_PORT_ID;

    if(enable >= RTK_ENABLE_END)
        return RT_ERR_INPUT;

    if((retVal= reg_field_write(RTL8316D_UNIT, PORT0_PRIORITY_REMARKING_CONTROL+port, DSCPRMK, enable))!=RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_qos_dscpRemarkEnable_get
 * Description:
 *      Get DSCP remarking ability.
 * Input:
 *      port - port ID
 * Output:
 *     pEnable - status of dscp remark.
 * Return:
 *      RT_ERR_OK - Success.
 *      RT_ERR_SMI - SMI access error.
 *      RT_ERR_INPUT - Invalid input parameters.
 *      RT_ERR_PORT_ID -  Invalid port number.
 *      RT_ERR_FAILED - Failure. 
 *      RT_ERR_ENABLE - Invalid enable parameter.
 * Note:
 *      The API can get DSCP remarking ability.
 *      The status of DSCP remark:
 *          DISABLED
 *          ENABLED
 */
rtk_api_ret_t rtk_qos_dscpRemarkEnable_get(rtk_port_t port, rtk_enable_t *pEnable)
{
    rtk_api_ret_t retVal;

    if(port > RTL8316D_MAX_PORT)
        return RT_ERR_PORT_ID;

    if (pEnable == NULL)
        return RT_ERR_NULL_POINTER;

    if((retVal = reg_field_read(RTL8316D_UNIT, PORT0_PRIORITY_REMARKING_CONTROL+port, DSCPRMK, pEnable))!=RT_ERR_OK)
        return retVal;
    
    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_qos_svlanRemarkEnable_set
 * Description:
 *      Set SVLAN priority remarking ability.
 * Input:
 *      port - port ID
 *      enable
 * Output:
 *     None
 * Return:
 *      RT_ERR_OK - Success.
 *      RT_ERR_SMI - SMI access error.
 *      RT_ERR_INPUT - Invalid input parameters.
 *      RT_ERR_PORT_ID -  Invalid port number.
 *      RT_ERR_FAILED - Failure. 
 *      RT_ERR_ENABLE - Invalid enable parameter.
 * Note:
 *      The API can enable or disable SVLAN priority remarking ability.
 *      The status of SVLAN priority remarking:
 *          DISABLED
 *          ENABLED
 */
rtk_api_ret_t rtk_qos_svlanRemarkEnable_set(rtk_port_t port, rtk_enable_t enable)
{
    rtk_api_ret_t retVal;
    
    /*for whole system function, the port value should be 0xFF*/
    if(port > RTL8316D_MAX_PORT)
        return RT_ERR_PORT_ID;

    if(enable >= RTK_ENABLE_END)
        return RT_ERR_INPUT;

    if((retVal= reg_field_write(RTL8316D_UNIT, PORT0_PRIORITY_REMARKING_CONTROL+port, OPRIRMK, enable))!=RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_qos_svlanRemarkEnable_get
 * Description:
 *      Get SVLAN priority remarking ability.
 * Input:
 *      port - port ID
 * Output:
 *     pEnable - 
 * Return:
 *      RT_ERR_OK - Success.
 *      RT_ERR_SMI - SMI access error.
 *      RT_ERR_INPUT - Invalid input parameters.
 *      RT_ERR_PORT_ID -  Invalid port number.
 *      RT_ERR_FAILED - Failure. 
 *      RT_ERR_ENABLE - Invalid enable parameter.
 * Note:
 *      The API can enable or disable SVLAN priority remarking ability.
 *      The status of SVLAN priority remarking:
 *          DISABLED
 *          ENABLED
 */
rtk_api_ret_t rtk_qos_svlanRemarkEnable_get(rtk_port_t port, rtk_enable_t *pEnable)
{
    rtk_api_ret_t retVal;

    if(port > RTL8316D_MAX_PORT)
        return RT_ERR_PORT_ID;

    if (pEnable == NULL)
        return RT_ERR_NULL_POINTER;

    if((retVal = reg_field_read(RTL8316D_UNIT, PORT0_PRIORITY_REMARKING_CONTROL+port, OPRIRMK, pEnable))!=RT_ERR_OK)
        return retVal;
    
    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_qos_cvlanRemarkEnable_set
 * Description:
 *      Set CVLAN priority remarking ability.
 * Input:
 *      port - port ID
 *      enable
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK - Success.
 *      RT_ERR_SMI - SMI access error.
 *      RT_ERR_INPUT - Invalid input parameters.
 *      RT_ERR_PORT_ID -  Invalid port number.
 *      RT_ERR_FAILED - Failure. 
 *      RT_ERR_ENABLE - Invalid enable parameter.
 * Note:
 *      The API can enable or disable CVLAN priority remarking ability.
 *       status: 
 *             -DISABLED
 *             -ENABLED
 */
rtk_api_ret_t rtk_qos_cvlanRemarkEnable_set(rtk_port_t port, rtk_enable_t enable)
{
    rtk_api_ret_t retVal;
    
    /*for whole system function, the port value should be 0xFF*/
    if(port > RTL8316D_MAX_PORT)
        return RT_ERR_PORT_ID;

    if(enable >= RTK_ENABLE_END)
        return RT_ERR_INPUT;

    if((retVal= reg_field_write(RTL8316D_UNIT, PORT0_PRIORITY_REMARKING_CONTROL+port, IPRIRMK, enable))!=RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_qos_cvlanRemarkEnable_get
 * Description:
 *      Get CVLAN priority remarking ability.
 * Input:
 *      port - port ID
 * Output:
 *     pEnable
 * Return:
 *      RT_ERR_OK - Success.
 *      RT_ERR_SMI - SMI access error.
 *      RT_ERR_INPUT - Invalid input parameters.
 *      RT_ERR_PORT_ID -  Invalid port number.
 *      RT_ERR_FAILED - Failure. 
 *      RT_ERR_ENABLE - Invalid enable parameter.
 * Note:
 *      The API can CVLAN priority remarking ability.
 *       status: 
 *             -DISABLED
 *             -ENABLED
 */
rtk_api_ret_t rtk_qos_cvlanRemarkEnable_get(rtk_port_t port, rtk_enable_t *pEnable)
{
    rtk_api_ret_t retVal;

    if(port > RTL8316D_MAX_PORT)
        return RT_ERR_PORT_ID;

    if (pEnable == NULL)
        return RT_ERR_NULL_POINTER;

    if((retVal = reg_field_read(RTL8316D_UNIT, PORT0_PRIORITY_REMARKING_CONTROL+port, IPRIRMK, pEnable))!=RT_ERR_OK)
        return retVal;
    
    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_qos_policerPreIfgCnt_set
 * Description:
 *      Enable/Disable policing include packets' premble and IFG
 * Input:
 *      enable - ENABLED or DISABLED
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK  - 
 *      RT_ERR_FAILED   - 
 *      RT_ERR_ENABLE - enable should be ENABLED or DISABLED.
 * Note:
 *      The API configures policing to include packets' premble and IFG or not.
 *       (1). ENABLED means policing will include packets' premble and IFG.
 *       (2). DISABLE means policing will not include packets' premble and IFG.
 */
rtk_api_ret_t rtk_qos_policerPreIfgCnt_set(rtk_enable_t enable)
{
    rtk_api_ret_t retVal;

    if(enable > RTK_ENABLE_END)
        return RT_ERR_ENABLE;

    retVal = reg_field_write(RTL8316D_UNIT, POLICER_TABLE_CONTROL, INCLPREIFG, enable);
    
    return retVal;
}

/* Function Name:
 *      rtk_qos_policerPreIfgCnt_get
 * Description:
 *      Retrieve whether policing will include packets' premble and IFG or not.
 * Input:
 *      None
 * Output:
 *      pEnable - ENABLED or DISABLED
 * Return:
 *      RT_ERR_OK  - 
 *      RT_ERR_FAILED   - 
 *      RT_ERR_NULL_POINTER - NULL pointer input.
 * Note:
 *      The API retrieve whether policing include packets' premble and IFG or not.
 *       (1). ENABLED means policing will include packets' premble and IFG.
 *       (2). DISABLE means policing will not include packets' premble and IFG.
 */
rtk_api_ret_t rtk_qos_policerPreIfgCnt_get(rtk_enable_t* pEnable)
{
    rtk_api_ret_t retVal;

    if(pEnable == NULL)
        return RT_ERR_NULL_POINTER;

    retVal = reg_field_read(RTL8316D_UNIT, POLICER_TABLE_CONTROL, INCLPREIFG, pEnable);
    
    return retVal;
}



/* Function Name:
 *      rtk_qos_dscpRemark_set
 * Description:
 *      Set DSCP remarking parameter.
 * Input:
 *      group - RTL8316D has two group of mapping table.
 *      int_pri - Internal priority value.
 *      dscp - DSCP value.
 * Output:
 *     None
 * Return:
 *      RT_ERR_OK - Success.
 *      RT_ERR_SMI - SMI access error.
 *      RT_ERR_QOS_INT_PRIORITY - Invalid priority.
 *      RT_ERR_QOS_DSCP_VALUE - Invalid DSCP value. 
 *      RT_ERR_FAILED - Failure
 * Note:
 *      The API can set DSCP value and mapping priority.
 */
rtk_api_ret_t rtk_qos_dscpRemark_set(uint8 group, rtk_pri_t int_pri, rtk_dscp_t dscp)
{
    rtk_api_ret_t retVal;
    uint32 regVal;

    if (group > 1)
        return RT_ERR_INPUT;
    
    if (int_pri > 7 )
        return RT_ERR_QOS_INT_PRIORITY; 

    if (dscp > 63)
        return RT_ERR_QOS_DSCP_VALUE;     

    if((retVal= reg_read(RTL8316D_UNIT, DSCP_REMARKING_TABLE_CONTROL0 + group*2 + int_pri/5, &regVal))!=RT_ERR_OK)
        return retVal;

    regVal &= ~(0x3F<<(int_pri%5)*6);
    regVal |= dscp << (int_pri%5)*6;
    retVal= reg_write(RTL8316D_UNIT, DSCP_REMARKING_TABLE_CONTROL0 + group*2 + int_pri/5, regVal);
   
    return retVal;
}

/* Function Name:
 *      rtk_qos_dscpRemark_get
 * Description:
 *      Get DSCP remarking parameter.
 * Input:
 *      group - RTL8316D has two group of mapping table.
 *      int_pri - Internal priority value.
 * Output:
 *     pDscp - DSCP value.
 * Return:
 *      RT_ERR_OK - Success.
 *      RT_ERR_SMI - SMI access error.
 *      RT_ERR_QOS_INT_PRIORITY - Invalid priority.
 *      RT_ERR_FAILED - Failure
 * Note:
 *      The API can get DSCP parameters. It would return DSCP value for mapping priority.
 */
rtk_api_ret_t rtk_qos_dscpRemark_get(uint8 group, rtk_pri_t int_pri, rtk_dscp_t *pDscp)
{
    rtk_api_ret_t retVal;
    uint32 regVal;

    if (group > 1)
        return RT_ERR_INPUT;
    
    if (int_pri > 7 )
        return RT_ERR_QOS_INT_PRIORITY; 

    if (pDscp == NULL)
        return RT_ERR_NULL_POINTER;     

    if((retVal= reg_read(RTL8316D_UNIT, DSCP_REMARKING_TABLE_CONTROL0 + group*2 + int_pri/5, &regVal))!=RT_ERR_OK)
        return retVal;

    *pDscp = (regVal >> (int_pri%5)*6) & 0x3F;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_qos_portDscpRemarkSel_set
 * Description:
 *       Select DSCP remarking group.
 * Input:
 *      group -  the group selected
 *      port - the physical port where packet is to be transmitted
 * Output:
 *     None
 * Return:
 *      RT_ERR_OK - Success.
 *      RT_ERR_PORT_ID - SMI access error.
 *      RT_ERR_NULL_POINTER - 
 *      RT_ERR_FAILED - Failure
 * Note:
 *      RTL8316D has two groups of DSCP remarking table, and each port could select one group for DSCP remarking.
 */
rtk_api_ret_t rtk_qos_portDscpRemarkSel_set(rtk_port_t port, uint8 group)
{ 
    rtk_api_ret_t retVal;

    if(port > RTL8316D_MAX_PORT)
        return RT_ERR_PORT_ID; 

    if (group >= 2)
    {
        return RT_ERR_QOS_SEL_PRI_GROUP;
    }

    retVal = reg_field_write(RTL8316D_UNIT, PORT0_PRIORITY_REMARKING_CONTROL+port, DSCPRMKTBLIDX, group);

    return retVal;
}

/* Function Name:
 *      rtk_qos_portDscpRemarkSel_get
 * Description:
 *       Get DSCP remarking group.
 * Input:
 *      port - the physical port where packet is to be transmitted
 * Output:
 *     pGroup - group returned
 * Return:
 *      RT_ERR_OK - Success.
 *      RT_ERR_PORT_ID - SMI access error.
 *      RT_ERR_NULL_POINTER - 
 *      RT_ERR_FAILED - Failure
 * Note:
 *      RTL8316D has two groups of DSCP remarking table, and each port could select one group for DSCP remarking.
 */
rtk_api_ret_t rtk_qos_portDscpRemarkSel_get(rtk_port_t port, uint8* pGroup)
{ 
    rtk_api_ret_t retVal;
    uint32 regVal;

    if(port > RTL8316D_MAX_PORT)
        return RT_ERR_PORT_ID; 

    if (pGroup == NULL)
    {
        return RT_ERR_NULL_POINTER;
    }

    retVal = reg_field_read(RTL8316D_UNIT, PORT0_PRIORITY_CONTROL+port, DSCPRMKTBLIDX, &regVal);
    *pGroup = regVal;

    return retVal;
}

/* Function Name:
 *      rtk_qos_portCvlanRemarkSel_set
 * Description:
 *      Select CVLAN priority remarking group.
 * Input:
 *      port  - the physical port where packet is to be transmitted
 *      group - the group select
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK - Success.
 *      RT_ERR_FAILED   - 
 *      RT_ERR_PORT_ID    
 *      RT_ERR_NULL_POINTER
 *      RT_ERR_INPUT - group should be 0 or 1
 * Note:
 *      (1)  RTL8316D has two groups of CVLAN priority remarking table, and each port could select one group for CVLAN priority remarking.
 */
rtk_api_ret_t rtk_qos_portCvlanRemarkSel_set(rtk_port_t port, uint8 group)
{ 
    rtk_api_ret_t retVal;

    if(port > RTL8316D_MAX_PORT)
        return RT_ERR_PORT_ID; 

    if (group >= 2)
    {
        return RT_ERR_INPUT;
    }

    retVal = reg_field_write(RTL8316D_UNIT, PORT0_PRIORITY_REMARKING_CONTROL+port, IPRIRMKTBLIDX, group);

    return retVal;
}

/* Function Name:
 *      rtk_qos_portCvlanRemarkSel_get
 * Description:
 *      Get CVLAN priority remarking group
 * Input:
 *      port  - the physical port where packet is to be transmitted
 * Output:
 *      pGroup - group returned
 * Return:
 *      RT_ERR_OK - Success.
 *      RT_ERR_FAILED   - 
 *      RT_ERR_PORT_ID   - 
 *      RT_ERR_NULL_POINTER
 * Note:
 *      (1)  RTL8316D has two groups of CVLAN priority remarking table, and each port could select one group for CVLAN priority remarking.
 */
rtk_api_ret_t rtk_qos_portCvlanRemarkSel_get(rtk_port_t port, uint8* pGroup)
{ 
    rtk_api_ret_t retVal;
    uint32 regVal;

    if(port > RTL8316D_MAX_PORT)
        return RT_ERR_PORT_ID; 

    if (pGroup == NULL)
    {
        return RT_ERR_NULL_POINTER;
    }

    retVal = reg_field_read(RTL8316D_UNIT, PORT0_PRIORITY_CONTROL+port, IPRIRMKTBLIDX, &regVal);
    *pGroup = regVal;

    return retVal;
}

/* Function Name:
 *      rtk_qos_portSvlanRemarkSel_set
 * Description:
 *      Select SVLAN priority remarking group.
 * Input:
 *      port  - the physical port where packet is to be transmitted
 *      group - the group select
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK - Success.
 *      RT_ERR_FAILED   - 
 *      RT_ERR_PORT_ID   - 
 *      RT_ERR_INPUT - group should be 0 or 1
 *      RT_ERR_NULL_POINTER
 * Note:
 *      (1)  RTL8316D has two groups of SVLAN priority remarking table, and each port could select one group for SVLAN priority remarking.
 */
rtk_api_ret_t rtk_qos_portSvlanRemarkSel_set(rtk_port_t port, uint8 group)
{ 
    rtk_api_ret_t retVal;

    if(port > RTL8316D_MAX_PORT)
        return RT_ERR_PORT_ID; 

    if (group >= 2)
    {
        return RT_ERR_INPUT;
    }

    retVal = reg_field_write(RTL8316D_UNIT, PORT0_PRIORITY_REMARKING_CONTROL+port, OPRIRMKTBLIDX, group);

    return retVal;
}

/* Function Name:
 *      rtk_qos_portSvlanRemarkSel_get
 * Description:
 *      Get SVLAN priority remarking group
 * Input:
 *      port  - the physical port where packet is to be transmitted
 * Output:
 *      pGroup - group returned
 * Return:
 *      RT_ERR_OK - Success.
 *      RT_ERR_FAILED   - 
 *      RT_ERR_PORT_ID   - 
 *      RT_ERR_NULL_POINTER
 * Note:
 *      (1)  RTL8316D has two groups of SVLAN priority remarking table, and each port could select one group for SVLAN priority remarking.
 */
rtk_api_ret_t rtk_qos_portSvlanRemarkSel_get(rtk_port_t port, uint8* pGroup)
{ 
    rtk_api_ret_t retVal;
    uint32 regVal;

    if(port > RTL8316D_MAX_PORT)
        return RT_ERR_PORT_ID; 

    if (pGroup == NULL)
    {
        return RT_ERR_NULL_POINTER;
    }

    retVal = reg_field_read(RTL8316D_UNIT, PORT0_PRIORITY_CONTROL+port, OPRIRMKTBLIDX, &regVal);
    *pGroup = regVal;

    return retVal;
}

/* Function Name:
 *      rtk_qos_cvlanPriRemark_set
 * Description:
 *      The API can set 802.1p parameters source priority and new priority
 * Input:
 *      group - RTL8316D has two groups of remarking configuration
 *      int_pri  - Internal priority value.
 *      dot1p_pri  - 802.1p priority value.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK - Success.
 *      RT_ERR_FAILED   - 
 *      RT_ERR_INPUT - group should be 0 or 1
 *      RT_ERR_QOS_INT_PRIORITY - error int_pri value
 *      RT_ERR_VLAN_PRIORITY - error dot1p_pri value
 * Note:
 *      (1) The API can set 802.1p parameters source priority and new priority.
 *      (2) RTL8316D has two groups of CVLAN priority(802.1p) remarking configuration
 */
rtk_api_ret_t rtk_qos_cvlanPriRemark_set(uint8 group, rtk_pri_t int_pri, rtk_pri_t dot1p_pri)
{
    rtk_api_ret_t retVal;
    uint32 regVal;

    if (group >= 2)
    {
        return RT_ERR_INPUT;
    }

    if (int_pri > 7 )
        return RT_ERR_QOS_INT_PRIORITY; 

    if (dot1p_pri > 7)
        return RT_ERR_VLAN_PRIORITY; 

    retVal = reg_read(RTL8316D_UNIT, INNER_TAG_PRIORITY_REMARKING_TABLE_CONTROL0+group, &regVal);
    if (retVal != SUCCESS)
        return retVal;

    regVal &= ~(0x7 << (int_pri*3));
    regVal |= dot1p_pri << (int_pri*3);
    retVal = reg_write(RTL8316D_UNIT, INNER_TAG_PRIORITY_REMARKING_TABLE_CONTROL0+group, regVal);

    return retVal;
}


/* Function Name:
 *      rtk_qos_cvlanPriRemark_get
 * Description:
 *      Get 802.1p remarking parameter. It would return new priority of ingress priority. 
 * Input:
 *      group - RTL8316D has two groups of remarking configuration
 *      int_pri  - Internal priority value.
 *      
 * Output:
 *      pDot1p_pri    - 802.1p priority value returned.
 * Return:
 *      RT_ERR_OK - Success.
 *      RT_ERR_FAILED   - 
 *      RT_ERR_INPUT - group should be 0 or 1
 *      RT_ERR_QOS_INT_PRIORITY - error int_pri value
 *      RT_ERR_NULL_POINTER - pDot1p_pri should not be NULL
 * Note:
 *      (1) The API can get 802.1p remarking priority by internal priority.
 *      (2) RTL8316D has two groups of CVLAN priority(802.1p) remarking configuration
 */
rtk_api_ret_t rtk_qos_cvlanPriRemark_get(uint8 group, rtk_pri_t int_pri, rtk_pri_t *pDot1p_pri)
{
    rtk_api_ret_t retVal;
    uint32 regVal;

    if (group >= 2)
    {
        return RT_ERR_INPUT;
    }

    if (int_pri > 7 )
        return RT_ERR_QOS_INT_PRIORITY; 

    if (pDot1p_pri == NULL)
        return RT_ERR_NULL_POINTER; 

    retVal = reg_read(RTL8316D_UNIT, INNER_TAG_PRIORITY_REMARKING_TABLE_CONTROL0+group, &regVal);
    *pDot1p_pri = (regVal >> (int_pri*3)) & 0x7;

    return retVal;
}

/* Function Name:
 *      rtk_qos_svlanPriRemark_set
 * Description:
 *      The API can set SVLAN remarking priority
 * Input:
 *      group - RTL8316D has two groups of SVLAN priority remarking configuration
 *      int_pri  - Internal priority value.
 *      rmk_pri  - SVLAN remarking priority value.
 *      rmk_dei - SVLAN remarking DEI value
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK - Success.
 *      RT_ERR_FAILED   - 
 *      RT_ERR_INPUT - group should be 0 or 1
 *      RT_ERR_QOS_INT_PRIORITY - error int_pri value
 *      RT_ERR_VLAN_PRIORITY - error dot1p_pri value
 *      RT_ERR_QOS_DEI - error DEI value
 * Note:
 *      (1) The API can set SVLAN remakring priority and DEI.
 *      (2) RTL8316D has two groups of SVLAN priority remarking configuration
 */
rtk_api_ret_t rtk_qos_svlanPriRemark_set(uint8 group, rtk_pri_t int_pri, rtk_pri_t rmk_pri, rtk_dei_t rmk_dei)
{
    rtk_api_ret_t retVal;
    uint32 regVal;

    if (group >= 2)
    {
        return RT_ERR_INPUT;
    }

    if (int_pri > 7 )
        return RT_ERR_QOS_INT_PRIORITY; 

    if (rmk_pri > 7)
        return RT_ERR_VLAN_PRIORITY; 
    
    if (rmk_dei > 1)
        return RT_ERR_QOS_DEI;

    retVal = reg_read(RTL8316D_UNIT, OUTER_TAG_PRIORITY_AND_DEI_REMARKING_TABLE_CONTROL0+group, &regVal);
    if (retVal != SUCCESS)
        return retVal;

    regVal &= ~(0xF << (int_pri*4));
    regVal |= rmk_pri << (int_pri*4);
    regVal |= rmk_dei << ((int_pri*4)+3);
    
    retVal = reg_write(RTL8316D_UNIT, OUTER_TAG_PRIORITY_AND_DEI_REMARKING_TABLE_CONTROL0+group, regVal);

    return retVal;
}


/* Function Name:
 *      rtk_qos_svlanPriRemark_get
 * Description:
 *      Get SVLAN remarking parameter. It would return new priority of ingress priority. 
 * Input:
 *      group - RTL8316D has two groups of SVLAN remarking configuration
 *      int_pri  - Internal priority value.
 * Output:
 *      pRmk_pri    - SVLAN remarking priority value returned.
 *      pDei            - DEI value returned
 * Return:
 *      RT_ERR_OK - Success.
 *      RT_ERR_FAILED   - 
 *      RT_ERR_INPUT - group should be 0 or 1
 *      RT_ERR_QOS_INT_PRIORITY - error int_pri value
 *      RT_ERR_NULL_POINTER - pointers input should not be NULL
 * Note:
 *      (1) The API can get SVLAN remarking priority and DEI by internal priority.
 *      (2) RTL8316D has two groups of SVLAN priority remarking configuration
 */
rtk_api_ret_t rtk_qos_svlanPriRemark_get(uint8 group, rtk_pri_t int_pri, rtk_pri_t *pRmk_pri, rtk_dei_t* pDei)
{
    rtk_api_ret_t retVal;
    uint32 regVal;

    if (group >= 2)
    {
        return RT_ERR_INPUT;
    }

    if (int_pri > 7 )
        return RT_ERR_QOS_INT_PRIORITY; 

    if (pRmk_pri == NULL)
        return RT_ERR_NULL_POINTER; 

    if (pDei == NULL)
        return RT_ERR_NULL_POINTER; 

    retVal = reg_read(RTL8316D_UNIT, OUTER_TAG_PRIORITY_AND_DEI_REMARKING_TABLE_CONTROL0+group, &regVal);
    *pRmk_pri = (regVal >> (int_pri*4)) & 0x7;
    *pDei = (regVal >> (int_pri*4+3)) & 0x1;

    return retVal;
}


/* Function Name:
 *      rtk_qos_portPriMap_set
 * Description:
 *      Set SVLAN remarking parameter. 
 * Input:
 *      port - port ID
 *      pPri2qid  - Priority mapping to queue ID.
 * Output:
 *      None
 * Return:
 *      SUCCESS  -
 *      FAILED   -
 *      RT_ERR_PORT_ID - error port ID
 *      RT_ERR_QUEUE_ID - error queue ID
 *      RT_ERR_NULL_POINTER - pointers input should not be NULL
 * Note:
 *      (1) RTL8316D could configure priority to queue mapping on a per-port basis.
 *      (2) Every port has 4 queues at most.
*/
rtk_api_ret_t rtk_qos_portPriMap_set(rtk_port_t port, rtk_qos_pri2queue_t *pPri2qid)
{    
    rtk_api_ret_t retVal;
    uint32 regVal;
    uint8 pri;
    
    if(port > RTL8316D_MAX_PORT)
        return RT_ERR_PORT_ID; 

    if (pPri2qid == NULL)
        return RT_ERR_NULL_POINTER;
    
    regVal = 0;
    for (pri=0; pri < RTK_MAX_NUM_OF_PRIORITY; pri++)
    {
        if (pPri2qid->pri2queue[pri] >= RTL8316D_QIDMAX) 
            return RT_ERR_QUEUE_ID;

        regVal |=  pPri2qid->pri2queue[pri] << pri*2;
    }

    retVal = reg_write(RTL8316D_UNIT, PORT0_PRIORITY_MAP_TO_QUEUENUMBER_CONTROL+port, regVal);
    return retVal;
}

 /* Function Name:
 *      rtk_qos_portPriMap_get
 * Description:
 *      Get SVLAN remarking parameter. It would return new priority of ingress priority. 
 * Input:
 *      port - port ID
 * Output:
 *      pPri2qid - Priority mapping to queue ID.
 * Return:
 *      SUCCESS  -
 *      FAILED   -
 *      RT_ERR_PORT_ID - error port ID
 *      RT_ERR_NULL_POINTER - pointers input should not be NULL
 * Note:
 *      (1) RTL8316D could configure priority to queue mapping on a per-port basis.
 *      (2) Every port has 4 queues at most.
*/
rtk_api_ret_t rtk_qos_portPriMap_get(rtk_port_t port, rtk_qos_pri2queue_t *pPri2qid)
{
    rtk_api_ret_t retVal;
    uint32 regVal;
    uint8 pri;
    
    if(port > RTL8316D_MAX_PORT)
        return RT_ERR_PORT_ID; 

    if (pPri2qid == NULL)
        return RT_ERR_NULL_POINTER;

    retVal = reg_read(RTL8316D_UNIT, PORT0_PRIORITY_MAP_TO_QUEUENUMBER_CONTROL+port, &regVal);
    
    for (pri=0; pri < RTK_MAX_NUM_OF_PRIORITY; pri++)
    {
        pPri2qid->pri2queue[pri] = (regVal >> (pri*2)) & 0x3;        
    }
    
    return retVal;
}

