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
 * $Revision: 8134 $
 * $Date: 2010-01-12 14:32:39 +0800 (Tue, 12 Jan 2010) $
 *
 * Purpose :This header defines RTK8316D/24D/25D's API. 
 *
 * Feature : This file includs following modules: 
 *           (1). QoS
 *           (2). Storm filtering
 *           (3). VLAN
 *           (4). PHY configuration
 *           (5). Spanning tree protocol
 *           (6). Address table lookup
 *           (7). IEEE802.1X
 *           (8). Link aggregation
 *           (9). Mirror
 *           (10). MIB
 *           (11). Interrupt
 *           (12). LED
 *           (13). ACL
 *           (14). RLDP
 *           (15). UDLD
 *           (16). RRCP
 *
 */



#ifndef __RTK_API_EXT_H__
#define __RTK_API_EXT_H__

#include <rtk_types.h>
#include <rtk_api.h>

extern void _rtk_powOf2(uint32 val, uint32 *ret);
extern uint32 _rtk_logOf2(uint32 val);
extern void _rtk_copy(uint8 *dest, const uint8 *src, uint32 n);
/*Misc*/
extern void rtk_switch_init(void);
extern int32 rtk_switch_maxPktLen_set(rtk_switch_maxPktLen_t len); 
extern int32 rtk_switch_maxPktLen_get(rtk_switch_maxPktLen_t *pLen);

/* Rate */

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
extern int32 rtk_rate_shareMeter_set(rtk_meter_id_t index, rtk_rate_t rate, rtk_enable_t ifg_include);

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
extern int32 rtk_rate_shareMeter_get(rtk_meter_id_t index, rtk_rate_t *pRate ,rtk_enable_t *pIfg_include);

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
extern int32 rtk_rate_igrBandwidthCtrlRate_set( rtk_port_t port, rtk_rate_t rate,  rtk_enable_t ifg_include);


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
extern int32 rtk_rate_igrBandwidthCtrlRate_get(rtk_port_t port, rtk_rate_t *pRate, rtk_enable_t *pIfg_include);


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
extern int32 rtk_rate_egrBandwidthCtrlRate_set(rtk_port_t port, rtk_rate_t rate,  rtk_enable_t ifg_includ);

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
extern int32 rtk_rate_egrBandwidthCtrlRate_get(rtk_port_t port, rtk_rate_t *pRate, rtk_enable_t *pIfg_include);

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
 *          (1) STORM_GROUP_UNKNOWN_UNICAST 
 *          (2) STORM_GROUP_UNKNOWN_MULTICAST 
 *          (3) STORM_GROUP_MULTICAST
 *          (4) STORM_GROUP_BROADCAST
 *       The rate unit is 1 kbps and the range is from 1 to 2^20. The granularity of rate is 1 kbps. 
 *       The ifg_include parameter, a global configuration in RTL8316D, is used for rate calculation with/without inter-frame-gap and preamble.
 *       For RTL8316D, unknown unicast\unknown multicast\multicast and broadcast share unique bandwitdth each port.
 */
extern int32 rtk_rate_stormControlRate_set(rtk_port_t port, rtk_rate_storm_group_t storm_type, rtk_rate_t rate, rtk_enable_t ifg_include);

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
extern int32 rtk_rate_stormControlRate_get(rtk_port_t port, rtk_rate_storm_group_t storm_type, rtk_rate_t *pRate, rtk_enable_t *pIfg_include);

/* QoS */
extern int32 rtk_qos_init(rtk_queue_num_t queueNum);

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
extern int32 rtk_qos_priSel_set(rtk_priority_select_t *pPriDec);

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
extern int32 rtk_qos_priSel_get(rtk_priority_select_t *pPriDec);


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
extern int32 rtk_qos_portPriSel_set(rtk_port_t port, uint8 group);

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
extern int32 rtk_qos_portPriSel_get(rtk_port_t port, uint8* pGroup);


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
extern int32 rtk_qos_portCvlanPriRemapSel_set(rtk_port_t port, uint8 group);


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
extern int32 rtk_qos_portCvlanPriRemapSel_get(rtk_port_t port, uint8* pGroup);

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
extern int32 rtk_qos_cvlanPriRemap_set(uint8 group, rtk_pri_t int_pri, rtk_pri_t dot1p_pri);

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
extern int32 rtk_qos_cvlanPriRemap_get(uint8 group, rtk_pri_t* PInt_pri, rtk_pri_t dot1p_pri);


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
extern int32 rtk_qos_svlanPriRemap_get(uint8 group, rtk_pri_t* PInt_pri, rtk_pri_t dot1p_pri, rtk_dei_t dei);

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
extern int32 rtk_qos_svlanPriRemap_set(uint8 group, rtk_pri_t int_pri, rtk_pri_t dot1p_pri, rtk_dei_t dei);


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
extern int32 rtk_qos_portSvlanPriRemapSel_get(rtk_port_t port, uint8* pGroup);

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
extern int32 rtk_qos_portSvlanPriRemapSel_set(rtk_port_t port, uint8 group);

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
extern int32 rtk_qos_dscpPriRemap_set(uint8 group, rtk_pri_t int_pri, rtk_dscp_t dscp);

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
extern int32 rtk_qos_dscpPriRemap_get(uint8 group, rtk_pri_t* pInt_pri, rtk_dscp_t dscp);

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
extern int32 rtk_qos_portDscpPriRemapSel_set(rtk_port_t port, uint8 group);

/* Function Name:
 *      rtk_qos_portDscpPriRemapSel_get
 * Description:
 *      get DSCP priority remapping group
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
 *      RTL8316D has two groups of DSCP priority remapping, and each port could select one group for DSCP priority remapping.
 */
extern int32 rtk_qos_portDscpPriRemapSel_get(rtk_port_t port, uint8* pGroup);

/* Function Name:
 *      rtk_qos_portSvlanPriCopy_set
 * Description:
 *      SVLAN tag priority could be copied from priority field of SVLAN tag or CVLAN tag.
 * Input:
 *      port - port ID
 *      source - 0: do not copy, 1: copy from priority field of CVLAN tag, 2: copy from priority field of SVLAN tag
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK - Success.
 *      RT_ERR_SMI - SMI access error.
 *      RT_ERR_INPUT - Invalid input parameters.
 *      RT_ERR_QOS_SEL_PRI_SOURCE -  Invalid priority decision source parameter.
 *      RT_ERR_FAILED - Failure. 
 * Note:
 *      The value of priority in SVLAN tag could be copied from priority field of SVLAN tag or CVLAN tag on a per-rx-port basis.
 */
extern int32 rtk_qos_portSvlanPriCopy_set(rtk_port_t port, uint8 source);

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
extern int32 rtk_qos_portSvlanPriCopy_get(rtk_port_t port, uint8* pSource);

/* Function Name:
 *      rtk_qos_portCvlanPriCopy_set
 * Description:
 *      CVLAN tag priority could be copied from priority field of SVLAN tag or CVLAN tag.
 * Input:
 *      port - port ID
 *      source - 0: do not copy, 1: copy from priority field of CVLAN tag, 2: copy from priority field of SVLAN tag
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK - Success.
 *      RT_ERR_SMI - SMI access error.
 *      RT_ERR_INPUT - Invalid input parameters.
 *      RT_ERR_QOS_SEL_PRI_SOURCE -  Invalid priority decision source parameter.
 *      RT_ERR_FAILED - Failure. 
 * Note:
 *       The value of priority in CVLAN tag could be copied from priority field of SVLAN tag or CVLAN tag on a per-rx-port basis.
 */
extern int32 rtk_qos_portCvlanPriCopy_set(rtk_port_t port, uint8 source);

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
extern int32 rtk_qos_portCvlanPriCopy_get(rtk_port_t port, uint8* pSource);


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
extern int32 rtk_qos_portPri_set(rtk_port_t port, rtk_pri_t int_pri);


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
extern int32 rtk_qos_portPri_get(rtk_port_t port, rtk_pri_t *pInt_pri) ;

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
extern int32 rtk_qos_portPriMap_get(rtk_port_t port, rtk_qos_pri2queue_t *pPri2qid);

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
extern int32 rtk_qos_portPriMap_set(rtk_port_t port, rtk_qos_pri2queue_t *pPri2qid);

extern int32 rtk_qos_schedulingQueue_set(rtk_port_t port, rtk_qos_queue_weights_t *pQweights);

extern int32 rtk_qos_schedulingQueue_get(rtk_port_t port, rtk_qos_queue_weights_t *pQweights);

extern int32 rtk_qos_schedulingAlgorithm_get(rtk_port_t port, rtk_qos_scheduling_type_t *pScheduling_type);

extern int32 rtk_qos_schedulingAlgorithm_set(rtk_port_t port, rtk_qos_scheduling_type_t scheduling_type);

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
extern int32 rtk_qos_svlanRemarkEnable_set(rtk_port_t port, rtk_enable_t enable);

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
extern int32 rtk_qos_svlanRemarkEnable_get(rtk_port_t port, rtk_enable_t *pEnable);

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
extern int32 rtk_qos_cvlanRemarkEnable_get(rtk_port_t port, rtk_enable_t *pEnable);

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
extern int32 rtk_qos_cvlanRemarkEnable_set(rtk_port_t port, rtk_enable_t enable);

/* Function Name:
 *      rtk_qos_portDscpRemarkSel_get
 * Description:
 *      Get DSCP remarking group
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
 *      (1)  RTL8316D has two groups of DSCP remarking table, and each port could select one group for DSCP remarking.
 */
extern int32 rtk_qos_portDscpRemarkSel_get(rtk_port_t port, uint8* pGroup);


/* Function Name:
 *      rtk_qos_portDscpRemarkSel_set
 * Description:
 *      Select DSCP remarking group.
 * Input:
 *      port  - the physical port where packet is to be transmitted
 *      group - the group select
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK - Success.
 *      RT_ERR_FAILED   - 
 *      RT_ERR_PORT_ID   - 
 *      RT_ERR_NULL_POINTER
 * Note:
 *      (1)  RTL8316D has two groups of DSCP remarking table, and each port could select one group for DSCP remarking.
 */
extern int32 rtk_qos_portDscpRemarkSel_set(rtk_port_t port, uint8 group);


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
 *      FAILED   - 
 *      RT_ERR_PORT_ID   - 
 *      RT_ERR_NULL_POINTER
 * Note:
 *      (1)  RTL8316D has two groups of CVLAN priority remarking table, and each port could select one group for CVLAN priority remarking.
 */
extern int32 rtk_qos_portCvlanRemarkSel_set(rtk_port_t port, uint8 group);


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
extern int32 rtk_qos_portCvlanRemarkSel_get(rtk_port_t port, uint8* pGroup);

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
 *      FAILED   - 
 *      RT_ERR_PORT_ID   - 
 *      RT_ERR_NULL_POINTER
 * Note:
 *      (1)  RTL8316D has two groups of SVLAN priority remarking table, and each port could select one group for SVLAN priority remarking.
 */
extern int32 rtk_qos_portSvlanRemarkSel_set(rtk_port_t port, uint8 group);


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
extern int32 rtk_qos_portSvlanRemarkSel_get(rtk_port_t port, uint8* pGroup);

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
 *      FAILED   - 
 *      RT_ERR_INPUT - group should be 0 or 1
 *      RT_ERR_QOS_INT_PRIORITY - error int_pri value
 *      RT_ERR_VLAN_PRIORITY - error dot1p_pri value
 * Note:
 *      (1) The API can set 802.1p parameters source priority and new priority.
 *      (2) RTL8316D has two groups of CVLAN priority(802.1p) remarking configuration
 */
extern int32 rtk_qos_cvlanPriRemark_set(uint8 group, rtk_pri_t int_pri, rtk_pri_t dot1p_pri);


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
extern int32 rtk_qos_cvlanPriRemark_get(uint8 group, rtk_pri_t int_pri, rtk_pri_t *pDot1p_pri);

/* Function Name:
  *     rtk_qos_dscpRemarkEnable_get
  * Description:
  *     Get DSCP remarking ability.
  * Input:
  *     port - Port id
  * Output:
  *     pEnable - status of 802.1p remark
  * Return:
  *     RT_ERR_OK
  *     RT_ERR_FAILED
  *     RT_ERR_PORT_ID - invalid port id
  *     RT_ERR_NULL_POINTER - input parameter may be null pointer
  * Note:
  *    (1) The API can get DSCP remarking ability.
  *          The status of DSCP remark:
  *             - DISABLED
  *             - ENABLED
  *    (2) Please check the return value for error handling
*/
extern int32 rtk_qos_dscpRemarkEnable_get(rtk_port_t port, rtk_enable_t *pEnable);

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
extern int32 rtk_qos_dscpRemarkEnable_set(rtk_port_t port, rtk_enable_t enable);

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
 *      FAILED   - 
 *      RT_ERR_INPUT - group should be 0 or 1
 *      RT_ERR_QOS_INT_PRIORITY - error int_pri value
 *      RT_ERR_NULL_POINTER - pointers input should not be NULL
 * Note:
 *      (1) The API can get SVLAN remarking priority and DEI by internal priority.
 *      (2) RTL8316D has two groups of SVLAN priority remarking configuration
 */
extern int32 rtk_qos_svlanPriRemark_get(uint8 group, rtk_pri_t int_pri, rtk_pri_t *pRmk_pri, rtk_dei_t* pDei);

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
extern int32 rtk_qos_svlanPriRemark_set(uint8 group, rtk_pri_t int_pri, rtk_pri_t rmk_pri, rtk_dei_t rmk_dei);

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
extern int32 rtk_qos_dscpRemark_set(uint8 group, rtk_pri_t int_pri, rtk_dscp_t dscp);

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
extern int32 rtk_qos_dscpRemark_get(uint8 group, rtk_pri_t int_pri, rtk_dscp_t *pDscp);

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
extern int32 rtk_qos_policerPreIfgCnt_set(rtk_enable_t enable);

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
extern int32 rtk_qos_policerPreIfgCnt_get(rtk_enable_t* pEnable);


/* Trap & Reserved Multicast Address (More Action like leaky, bypass storm not define) */

/* Function Name:
 *      rtk_trap_unknownUnicastPktAction_set
 * Description:
 *      Set unknown unicast packet action configuration.
 * Input:
 *      type - Unknown unicast packet type.
 *      ucast_action - Unknown unicast action. 
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK - 
 *      RT_ERR_FAILED  - 
 *      RT_ERR_NULL_POINTER - Invalid mode pointer.
 *      RT_ERR_NOT_ALLOWED - Invalid action.
 * Note:
 *    This API can set unknown unicast packet action configuration.
 *   The unknown unicast packet type is as following:
 *        -UCAST_UNKNOWNDA
 *   The unknown unicast action is as following:
 *        -UCAST_ACTION_FORWARD 
 *        -UCAST_ACTION_DROP
 *        -UCAST_ACTION_TRAP2CPU
 *        -UCAST_ACTION_FLOOD
 *    UCAST_ACTION_FORWARD means flood within VLAN, while UCAST_ACTION_FLOOD would flood to all ports.
 */
extern int32 rtk_trap_unknownUnicastPktAction_set(rtk_trap_ucast_type_t type, rtk_trap_ucast_action_t ucast_action); //for unknown & unmatch


/* Function Name:
 *      rtk_trap_unknownUnicastPktAction_get
 * Description:
 *      Get unknown unicast packet action configuration.
 * Input:
 *      type - Unknown unicast packet type.
 * Output:
 *      pUcast_action - unknown unicast action. 
 * Return:
 *      RT_ERR_OK - 
 *      RT_ERR_FAILED  - 
 *      RT_ERR_INPUT - Invalid input parameters.
 * Note:
 *    This API can get unknown unicast packet action configuration.
 *   The unknown unicast packet type is as following:
 *        -UCAST_UNKNOWNDA
 *   The unknown unicast action is as following:
 *        -UCAST_ACTION_FORWARD 
 *        -UCAST_ACTION_DROP
 *        -UCAST_ACTION_TRAP2CPU
 *        -UCAST_ACTION_FLOOD
 *    UCAST_ACTION_FORWARD means flood within VLAN, while UCAST_ACTION_FLOOD would flood to all ports.
 */
extern int32 rtk_trap_unknownUnicastPktAction_get(rtk_trap_ucast_type_t type, rtk_trap_ucast_action_t *pUcast_action); //for unknown & unmatch

/* Function Name:
 *      rtk_trap_unknownMcastPktAction_set
 * Description:
 *      Set behavior of unknown multicast
 * Input:
 *      port - Port id.
 *      type - unknown multicast packet type.
 *      mcast_action - unknown multicast action. 
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK - 
 *      RT_ERR_FAILED  - 
 *      RT_ERR_INPUT - Invalid input parameters.
 *      RT_ERR_PORT_ID - Invalid port number.
 *      RT_ERR_NOT_ALLOWED - Invalid operation.
 * Note:
 *      When receives an unknown multicast packet, switch may trap, drop or flood this packet
 *      The unknown multicast packet type is as following:
 *          -MCAST_L2
 *          -MCAST_IPV4
 *          -MCAST_IPV6
 *          -MCAST_L3
 *      The unknown multicast action is as following:
 *          -MCAST_ACTION_DROP 
 *          -MCAST_ACTION_TRAP2CPU    
 *          -MCAST_ACTION_FORWARD
 *          -MCAST_ACTION_FLOOD
 *       MCAST_ACTION_FORWARD means flood in VLAN, while MCAST_ACTION_FLOOD means flood to all ports. 
 */
extern int32 rtk_trap_unknownMcastPktAction_set(rtk_port_t port, rtk_mcast_type_t type, rtk_trap_mcast_action_t mcast_action);

/* Function Name:
 *      rtk_trap_unknownMcastPktAction_get
 * Description:
 *      Get behavior of unknown multicast
 * Input:
 *      port - Port id.
 *      type - unknown multicast packet type.
 * Output:
 *      pMcast_action - unknown multicast action. 
 * Return:
 *      RT_ERR_OK - 
 *      RT_ERR_FAILED  - 
 *      RT_ERR_INPUT - Invalid input type.
 *      RT_ERR_PORT_ID - Invalid port number.
 *      RT_ERR_NULL_POINTER - Null pointer input.
 * Note:
 *      When receives an unknown multicast packet, switch may trap, drop or flood this packet
 *      The unknown multicast packet type is as following:
 *          -MCAST_L2
 *          -MCAST_IPV4
 *          -MCAST_IPV6
 *          -MCAST_L3
 *      The unknown multicast action is as following:
 *          -MCAST_ACTION_DROP 
 *          -MCAST_ACTION_TRAP2CPU    
 *          -MCAST_ACTION_FORWARD
 *          -MCAST_ACTION_FLOOD
 *       MCAST_ACTION_FORWARD means flood in VLAN, while MCAST_ACTION_FLOOD means flood to all ports. 
 */
extern int32 rtk_trap_unknownMcastPktAction_get(rtk_port_t port, rtk_mcast_type_t type, rtk_trap_mcast_action_t *pMcast_action);

/* Function Name:
 *      rtk_trap_igmpCtrlPktAction_set
 * Description:
 *      Set IGMP/MLD trap function
 * Input:
 *      type -  IGMP/MLD packet type.
 *      igmp_action - IGMP/MLD action. 
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK - 
 *      RT_ERR_FAILED  - 
 *      RT_ERR_INPUT - Invalid input type.
 *      RT_ERR_NOT_ALLOWED - Invalid action
 * Note:
 *      This API can set both IPv4 IGMP/IPv6 MLD trapping function.
 *      IGMP and MLD can be set seperately.
 *      The IGMP/MLD packet type is as following:
 *           -IGMP_IPV4
 *           -IGMP_MLD
 *      The IGMP/MLD action is as following:
 *           -IGMP_ACTION_FORWARD
 *           -IGMP_ACTION_TRAP2CPU
 *           -IGMP_ACTION_FORWARD_COPY2_CPU
 *           -IGMP_ACTION_DROP
 */
extern int32 rtk_trap_igmpCtrlPktAction_set(rtk_igmp_type_t type, rtk_trap_igmp_action_t igmp_action);

/* Function Name:
 *      rtk_trap_igmpCtrlPktAction_get
 * Description:
 *      Get IGMP/MLD trap function
 * Input:
 *      type -  IGMP/MLD packet type.
 * Output:
 *      pIgmp_action - IGMP/MLD action. 
 * Return:
 *      RT_ERR_OK - 
 *      RT_ERR_FAILED  - 
 *      RT_ERR_INPUT - Invalid input type.
 * Note:
 *      This API can get both IPv4 IGMP/IPv6 MLD trapping function.
 *      The IGMP/MLD packet type is as following:
 *           -IGMP_IPV4
 *           -IGMP_MLD
 *      The IGMP/MLD action is as following:
 *           -IGMP_ACTION_FORWARD
 *           -IGMP_ACTION_TRAP2CPU
 *           -IGMP_ACTION_FORWARD_COPY2_CPU
 *           -IGMP_ACTION_DROP
 */
extern int32 rtk_trap_igmpCtrlPktAction_get(rtk_igmp_type_t type, rtk_trap_igmp_action_t *pIgmp_action);

/* Function Name:
 *      rtk_trap_rmaAction_set
 * Description:
 *      Set reserved multicast address frame action.
 * Input:
 *      pRma_frame - Reserved multicast address.
 *      rma_action - RMA action.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK - Success.
 *      RT_ERR_FAILED   - 
 *      RT_ERR_INPUT - Invalid input parameters.
 *      RT_ERR_RMA_ADDR - Invalid RMA address. 
 *      RT_ERR_RMA_ACTION - Invalid RMA action.
 * Note:
 *      The RMA action is as following:
 *          RMA_ACTION_FORWARD
 *          RMA_ACTION_DROP
 *          RMA_ACTION_TRAP2CPU
 *          RMA_ACTION_FORWARD_COPY2_CPU
 */
extern int32 rtk_trap_rmaAction_set(rtk_mac_t *pRma_frame, rtk_trap_rma_action_t rma_action);

/* Function Name:
 *      rtk_trap_rmaAction_get
 * Description:
 *      Retrieve RMA frame action.
 * Input:
 *      pRma_frame - Reserved multicast address.
 * Output:
 *      pRma_action - RMA action.
 * Return:
 *      RT_ERR_OK - Success.
 *      RT_ERR_FAILED   - 
 *      RT_ERR_INPUT - Invalid input parameters.
 *      RT_ERR_RMA_ADDR - Invalid RMA address. 
 *      RT_ERR_NULL_POINTER - Invalid RMA address. 
 *      RT_ERR_RMA_ACTION - Invalid RMA action.
 * Note:
 *      The RMA action is as following:
 *          -RMA_ACTION_FORWARD
 *          -RMA_ACTION_DROP
 *          -RMA_ACTION_TRAP2CPU
 *          -RMA_ACTION_FORWARD_COPY2_CPU
 */
extern int32 rtk_trap_rmaAction_get(rtk_mac_t *pRma_frame, rtk_trap_rma_action_t *pRma_action);


/* Function Name:
 *      rtk_trap_reasonTrapToCPUPriority_get
 * Description:
 *       Get priority assigned to packets when trapped to CPU
 * Input:
 *      type -  trap reason.
 * Output:
 *      pPriority - priority retrived. 
 * Return:
 *      RT_ERR_OK - 
 *      RT_ERR_FAILED  - 
 *      RT_ERR_INPUT - Invalid input type.
 *      RT_ERR_NULL_POINTER - Null pointer input
 * Note:
 *      There are a couple of reasons that packets would be trapped to CPU. This API could be used to
 *       retrive the priority assigned to packets trapped to CPU for specific reason.
 */
extern int32 rtk_trap_reasonTrapToCPUPriority_get(rtk_trap_reason_type_t type, rtk_pri_t *pPriority); 

/* Function Name:
 *      rtk_trap_reasonTrapToCPUPriority_set
 * Description:
 *       Set priority assigned to packets when trapped to CPU
 * Input:
 *      type -  trap reason.
 *      priority - priority assigned . 
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK - 
 *      RT_ERR_FAILED  - 
 *      RT_ERR_INPUT - Invalid input priority or type.
 * Note:
 *      There are a couple of reasons that packets would be trapped to CPU. This API could be used to
 *       set the priority assigned to packets trapped to CPU for specific reason.
 */
extern int32 rtk_trap_reasonTrapToCPUPriority_set(rtk_trap_reason_type_t type, rtk_pri_t priority);   


/* Function Name:
 *      rtk_trap_rmaToCPUPriority_get
 * Description:
 *      Get priority assigned to packets when trapped to CPU.
 * Input:
 *      pRma_frame - Reserved multicast address.
 * Output:
 *      pPriority - priority assigned when packets with specific RMA trapped to CPU. 
 * Return:
 *      RT_ERR_OK - Success.
 *      RT_ERR_FAILED   - 
 *      RT_ERR_RMA_ADDR - Invalid RMA address. 
 *      RT_ERR_NULL_POINTER - Invalid RMA address. 
 * Note:
 *      Packets with DMAC which is RMA could be assigned priority when trapped to CPU. This API could be used to
 *     get the priority for each RMA classes.
 */
extern int32 rtk_trap_rmaToCPUPriority_get(rtk_mac_t *pRma_frame, rtk_pri_t *pPriority); 

/* Function Name:
 *      rtk_trap_rmaToCPUPriority_set
 * Description:
 *      Set priority assigned to packets when trapped to CPU.
 * Input:
 *      pRma_frame - Reserved multicast address.
 *      priority - priority assigned when packets with specific RMA trapped to CPU. 
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK - Success.
 *      RT_ERR_FAILED   - 
 *      RT_ERR_RMA_ADDR - Invalid RMA address. 
 *      RT_ERR_NULL_POINTER - Invalid RMA address. 
 * Note:
 *      Packets with DMAC which is RMA could be assigned priority when trapped to CPU. This API could be used to
 *     get the priority for each RMA classes.
 */
extern int32 rtk_trap_rmaToCPUPriority_set(rtk_mac_t *pRma_frame, rtk_pri_t priority);   

/* Leaky */

/* Function Name:
 *      rtk_leaky_vlan_set
 * Description:
 *      Set VLAN leaky
 * Input:
 *      type             - Packet type for VLAN leaky
 *      enable          - Leaky status
 * Output:
 *      none
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_SMI  
 *      RT_ERR_INPUT
 *      RT_ERR_ENABLE 
 * Note:
 *    This API can set VLAN leaky.
 *    The leaky frame types are as following:
 *    LEAKY_STATIC_LUT.
 *    LEAKY_RRCP,
 *    LEAKY_RLDP.
 */
extern int32 rtk_leaky_vlan_set(rtk_leaky_type_t type, rtk_enable_t enable);


/* Function Name:
 *      rtk_leaky_vlan_get
 * Description:
 *      Get VLAN leaky setting
 * Input:
 *      type                - Packet type for VLAN leaky
 * Output:
 *      rtk_enable_t     - Leaky status
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_SMI  
 *      RT_ERR_INPUT
 *      RT_ERR_NULL_POINTER 
 * Note:
 *    This API can get VLAN leaky setting
 *    The leaky frame types are as following:
 *    LEAKY_STATIC_LUT.
 *    LEAKY_RRCP,
 *    LEAKY_RLDP.
 */
extern int32 rtk_leaky_vlan_get(rtk_leaky_type_t type, rtk_enable_t *pEnable);

/* Port and PHY setting */

/* Function Name:
 *      rtk_port_phyAutoNegoAbility_set
 * Description:
 *      Set ethernet PHY auto-negotiation desired ability.
 * Input:
 *      port  - the physical port where packet is to be transmitted
 * Output:
 *      pAbility - Ability structure
 * Return:
 *      RT_ERR_OK - Success.
 *      RT_ERR_FAILED   - 
 *      RT_ERR_PORT_ID - Invalid port number.   
 *      RT_ERR_NULL_POINTER
 *      RT_ERR_INPUT - group should be 0 or 1
 *      RT_ERR_BUSYWAIT_TIMEOUT - PHY access busy
 *      RT_ERR_PHY_REG_ID - Invalid PHY address
 *      RT_ERR_BUSYWAIT_TIMEOUT - PHY access busy
 * Note:
 *      If Full_1000 bit is set to 1, the AutoNegotiation will be automatic set to 1. While both AutoNegotiation and Full_1000 are set to 0, the PHY speed and duplex selection will
 *      be set as following 100F > 100H > 10F > 10H priority sequence.
 */
extern int32 rtk_port_phyAutoNegoAbility_set(rtk_port_t port, rtk_port_phy_ability_t *pAbility);

/* Function Name:
 *      rtk_port_phyAutoNegoAbility_get
 * Description:
 *      Get ethernet PHY auto-negotiation desired ability.
 * Input:
 *      port  - the physical port where packet is to be transmitted
 * Output:
 *      pAbility - Ability structure
 * Return:
 *      RT_ERR_OK - Success.
 *      RT_ERR_FAILED   - 
 *      RT_ERR_PORT_ID - Invalid port number.   
 *      RT_ERR_NULL_POINTER
 *      RT_ERR_INPUT - group should be 0 or 1
 *      RT_ERR_BUSYWAIT_TIMEOUT - PHY access busy
 *      RT_ERR_PHY_REG_ID - Invalid PHY address
 *      RT_ERR_BUSYWAIT_TIMEOUT - PHY access busy
 * Note:
 *      Get the capablity of specified PHY.
 */
extern int32 rtk_port_phyAutoNegoAbility_get(rtk_port_t port, rtk_port_phy_ability_t *pAbility); 

/* CVLAN */

/* Function Name:
 *      rtk_vlan_init
 * Description:
 *      Initialize VLAN
 * Input:
 *      void
 * Output:
 *      void
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_SMI 
 * Note:
 *     VLAN is disabled by default. User has to call this API to enable VLAN before
 *     using it. And It will set a default VLAN(vid 1) including all ports and set 
 *     all ports PVID to the default VLAN.
 */
extern int32 rtk_vlan_init(void);


/* Function Name:
 *      rtk_vlan_set
 * Description:
 *      Set a VLAN entry
 * Input:
 *      vid          - VLAN ID to configure
 *      mbrmsk    - VLAN member set portmask
 *      untagmsk  - VLAN untag set portmask
 *      fid           -  filtering database id
 * Output:
 *      none
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_SMI  
 *      RT_ERR_VLAN_VID
 *      RT_ERR_VLAN_PORT_MBR_EXIST
 *      RT_ERR_L2_FID
 *      RT_ERR_TBL_FULL
 * Note:
 *     There are 4K VLAN entry supported. User could configure the member set and untag set
 *     for specified vid through this API. The portmask's bit N means port N.
 *     For example, mbrmask 23=0x17=010111 means port 0,1,2,4 in the member set.
 *     FID is for SVL/IVL usage, and the range is 0~4095.
 *     Because SVLAN and CVLAN share the same vlan table, so SVLAN also use this API to set vlan information.
 */
extern int32 rtk_vlan_set(rtk_vlan_t vid, rtk_portmask_t mbrmsk, rtk_portmask_t untagmsk, rtk_fid_t fid);

/* Function Name:
 *      rtk_vlan_get
 * Description:
 *      Get a VLAN entry
 * Input:
 *      vid             - VLAN ID to configure
 * Output:
 *      pMbrmsk     - VLAN member set portmask
 *      pUntagmsk  - VLAN untag set portmask
 *      pFid           -  filtering database id
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_SMI  
 *      RT_ERR_VLAN_VID
 *      RT_ERR_NULL_POINTER 
 *      RT_ERR_VLAN_ENTRY_NOT_FOUND
 * Note:
 *     There are 4K VLAN entry supported. User could configure the member set and untag set
 *     for specified vid through this API. The portmask's bit N means port N.
 *     For example, mbrmask 23=0x17=010111 means port 0,1,2,4 in the member set.
 *     FID is for SVL/IVL usage, and the range is 0~4095.
 *     Because SVLAN and CVLAN share the same vlan table, so SVLAN also use this API to set vlan information.
 */
extern int32 rtk_vlan_get(rtk_vlan_t vid, rtk_portmask_t *pMbrmsk, rtk_portmask_t *pUntagmsk, rtk_fid_t *pFid);

/* Function Name:
 *      rtk_vlan_destroy
 * Description:
 *      remove a VLAN entry
 * Input:
 *      vid             - VLAN ID to configure
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_SMI  
 *      RT_ERR_VLAN_VID
 *      RT_ERR_NULL_POINTER 
 *      RT_ERR_VLAN_ENTRY_NOT_FOUND
 * Note:
 *     There are 4K VLAN entry supported. User could configure the member set and untag set
 *     for specified vid through this API. The portmask's bit N means port N.
 *     For example, mbrmask 23=0x17=010111 means port 0,1,2,4 in the member set.
 *     FID is for SVL/IVL usage, and the range is 0~4095.
 */
extern int32 rtk_vlan_destroy(rtk_vlan_t vid);

/* Function Name:
 *      rtk_vlan_portPvid_set
 * Description:
 *      Set port to specified VLAN ID(PVID)
 * Input:
 *      port             - Port id
 *      pvid             - Specified VLAN ID
 *      priority         - 802.1p priority for the PVID
 * Output:
 *      none
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_SMI  
 *      RT_ERR_VLAN_VID
 *      RT_ERR_VLAN_PRIORITY 
 *      RT_ERR_VLAN_ENTRY_NOT_FOUND
 * Note:
 *    The API is used for Port-based VLAN. The untagged frame received from the
 *    port will be classified to the specified VLAN and assigned to the specified priority.
 */
extern int32 rtk_vlan_portPvid_set(rtk_port_t port, rtk_vlan_t pvid, rtk_pri_t priority);

/* Function Name:
 *      rtk_vlan_portPvid_get
 * Description:
 *      Get VLAN ID(PVID) on specified port
 * Input:
 *      port             - Port id
 *      pPvid            - Specified VLAN ID
 *      pPriority        - 802.1p priority for the PVID
 * Output:
 *      none
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_SMI  
 *      RT_ERR_PORT_ID
 *      RT_ERR_NULL_POINTER 
 * Note:
 *    The API is used for Port-based VLAN. The untagged frame received from the
 *    port will be classified to the specified VLAN and assigned to the specified priority.
 */
extern int32 rtk_vlan_portPvid_get(rtk_port_t port, rtk_vlan_t *pPvid, rtk_pri_t *pPriority);

/*
@func rtk_api_ret_t | rtk_vlan_portIgrFilterEnable_set | Set VLAN ingress for each port.
@parm rtk_port_t | port | Port id.
@parm rtk_enable_t | igr_filter | VLAN ingress function enable status.
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_INPUT | Invalid input parameters.
@rvalue RT_ERR_PORT_ID | Invalid port number.
@rvalue RT_ERR_ENABLE | Invalid enable input.
@rvalue RT_ERR_FAILED | Failure. 
@comm
    The status of vlan ingress filter is as following:
    DISABLED
    ENABLED
    While VLAN function is enabled, ASIC will decide VLAN ID for each received frame and get belonged member
    ports from VLAN table. If received port is not belonged to VLAN member ports, ASIC will drop received frame if VLAN ingress function is enabled.
    
*/
extern int32 rtk_vlan_portIgrFilterEnable_set(rtk_port_t port, rtk_enable_t igr_filter);

/*
@func rtk_api_ret_t | rtk_vlan_portIgrFilterEnable_get | Get VLAN Ingress Filter
@parm rtk_port_t | port | Port id.
@parm rtk_enable_t* | pIgr_filter | VLAN ingress function enable status.
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error.
@rvalue RT_ERR_INPUT | Invalid input parameter.
@rvalue RT_ERR_PORT_ID | Invalid port number.
@rvalue RT_ERR_PORT_ID | Invalid port number.
@comm
    The API can Get the VLAN ingress filter status.
    The status of vlan ingress filter is as following:
    DISABLED
    ENABLED   
*/
extern int32 rtk_vlan_portIgrFilterEnable_get(rtk_port_t port, rtk_enable_t *pIgr_filter);

/* Function Name:
 *      rtk_vlan_portAcceptFrameType_set
 * Description:
 *      Set VLAN support frame type
 * Input:
 *      port                                 - Port id
 *      accept_frame_type             - accept frame type
 * Output:
 *      none
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_SMI  
 *      RT_ERR_PORT_ID
 * Note:
 *    The API is used for checking 802.1Q tagged frames.
 *    The accept frame type as following:
 *    ACCEPT_FRAME_TYPE_ALL
 *    ACCEPT_FRAME_TYPE_TAG_ONLY
 *    ACCEPT_FRAME_TYPE_UNTAG_ONLY
 */
extern int32 rtk_vlan_portAcceptFrameType_set(rtk_port_t port, rtk_vlan_acceptFrameType_t accept_frame_type);

/* Function Name:
 *      rtk_vlan_portAcceptFrameType_get
 * Description:
 *      Get VLAN support frame type
 * Input:
 *      port                                 - Port id
 *      accept_frame_type             - accept frame type
 * Output:
 *      none
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_SMI  
 *      RT_ERR_PORT_ID
 * Note:
 *    The API is used for checking 802.1Q tagged frames.
 *    The accept frame type as following:
 *    ACCEPT_FRAME_TYPE_ALL
 *    ACCEPT_FRAME_TYPE_TAG_ONLY
 *    ACCEPT_FRAME_TYPE_UNTAG_ONLY
 */
extern int32 rtk_vlan_portAcceptFrameType_get(rtk_port_t port, rtk_vlan_acceptFrameType_t *pAccept_frame_type);

/* Function Name:
 *      rtk_vlan_tagMode_set
 * Description:
 *      Set VLAN egress tag mode
 * Input:
 *      port                - Port id
 *      tag_mode        - Egress tag mode
 * Output:
 *      none
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_SMI  
 *      RT_ERR_INPUT
 * Note:
 *    The API is used for setting port vlan egress tag mode
 *    The tag mode  as following:
 *    VLAN_TAG_MODE_ORIGINAL
 *    VLAN_TAG_MODE_KEEP_FORMAT
 *    VLAN_TAG_MODE_REAL_KEEP_FORMAT
 */
extern int32 rtk_vlan_tagMode_set(rtk_port_t port, rtk_vlan_tagMode_t tag_mode);

/* Function Name:
 *      rtk_vlan_tagMode_get
 * Description:
 *      Get VLAN egress tag mode
 * Input:
 *      port                - Port id
 *      pTag_mode      - Egress tag mode
 * Output:
 *      none
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_SMI  
 *      RT_ERR_INPUT
 * Note:
 *    The API is used for setting port vlan egress tag mode
 *    The tag mode  as following:
 *    VLAN_TAG_MODE_ORIGINAL
 *    VLAN_TAG_MODE_KEEP_FORMAT
 *    VLAN_TAG_MODE_REAL_KEEP_FORMAT
 */
extern int32 rtk_vlan_tagMode_get(rtk_port_t port, rtk_vlan_tagMode_t *pTag_mode);

/* Function Name:
 *      rtk_vlan_stg_get
 * Description:
 *      Set spanning tree group instance of the vlan to the specified device
 * Input:
 *      vid                -  specified VLAN ID
 * Output:
 *      pStg             -   spanning tree group instance
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_SMI
 *      RT_ERR_NULL_POINTER
 *      RT_ERR_VLAN_ENTRY_NOT_FOUND
 * Note:
 *     The API can set spanning tree group instance of the vlan to the specified device.
 *     The spanning tree group number is from 0 to 31.  
 */
extern int32 rtk_vlan_stg_get(rtk_vlan_t vid, rtk_stg_t *pStg);

/* Function Name:
 *      rtk_vlan_stg_set
 * Description:
 *      Set spanning tree group instance of the vlan to the specified device
 * Input:
 *      vid                -  specified VLAN ID
 *      stg                -  spanning tree group instance
 * Output:
 *      none
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_INPUT
 *      RT_ERR_SMI
 *      RT_ERR_VLAN_ENTRY_NOT_FOUND
 * Note:
 *     The API can set spanning tree group instance of the vlan to the specified device.
 *     The spanning tree group number is from 0 to 31.  
 */
extern int32 rtk_vlan_stg_set(rtk_vlan_t vid, rtk_stg_t stg);

/* Function Name:
 *      rtk_leaky_vlan_set
 * Description:
 *      Set VLAN leaky
 * Input:
 *      type             - Packet type for VLAN leaky
 *      enable          - Leaky status
 * Output:
 *      none
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_SMI  
 *      RT_ERR_INPUT
 *      RT_ERR_ENABLE 
 * Note:
 *    This API can set VLAN leaky.
 *    The leaky frame types are as following:
 *    LEAKY_STATIC_LUT.
 *    LEAKY_RRCP,
 *    LEAKY_RLDP.
 */
extern int32 rtk_leaky_vlan_set(rtk_leaky_type_t type, rtk_enable_t enable);

/* Function Name:
 *      rtk_leaky_vlan_get
 * Description:
 *      Get VLAN leaky setting
 * Input:
 *      type                - Packet type for VLAN leaky
 * Output:
 *      rtk_enable_t     - Leaky status
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_SMI  
 *      RT_ERR_INPUT
 *      RT_ERR_NULL_POINTER 
 * Note:
 *    This API can get VLAN leaky setting
 *    The leaky frame types are as following:
 *    LEAKY_STATIC_LUT.
 *    LEAKY_RRCP,
 *    LEAKY_RLDP.
 */
extern int32 rtk_leaky_vlan_get(rtk_leaky_type_t type, rtk_enable_t *pEnable);

/*Spanning Tree*/

/* Function Name:
 *      rtk_stp_init
 * Description:
 *      Initialize stp module of the specified device.
 * Input:
 *      None
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK - Success.
 *      RT_ERR_FAILED   - 
 * Note:
 *      Initialize stp module before calling any vlan APIs
 */
extern int32 rtk_stp_init(void);

/* Function Name:
 *      rtk_stp_mstpState_set
 * Description:
 *      Configure spanning tree state per each port.
 * Input:
 *      port - port ID
 *      msti - Multiple spanning tree instance.
 *      stp_state - Spanning tree state for msti
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK - Success.
 *      RT_ERR_FAILED   - 
 *      RT_ERR_PORT_ID - Invalid port number.
 *      RT_ERR_MSTI -Invalid msti parameter
 *      RT_ERR_MSTP_STATE - Invalid STP state.
 * Note:
 *    System supports per-port multiple spanning tree state for each msti. 
 *    There are four states supported by ASIC.
 *      -STP_STATE_DISABLED
 *      -STP_STATE_BLOCKING
 *      -STP_STATE_LEARNING
 *      -STP_STATE_FORWARDING
 */
extern int32 rtk_stp_mstpState_set(rtk_stp_msti_id_t msti, rtk_port_t port, rtk_stp_state_t stp_state);

/* Function Name:
 *      rtk_stp_mstpState_get
 * Description:
 *      Configure spanning tree state per each port.
 * Input:
 *      port - port ID
 *      msti - Multiple spanning tree instance.
 * Output:
 *      pStp_state - Spanning tree state for msti
 * Return:
 *      RT_ERR_OK - Success.
 *      RT_ERR_FAILED   - 
 *      RT_ERR_PORT_ID - Invalid port number.
 *      RT_ERR_MSTI -Invalid msti parameter
 * Note:
 *    System supports per-port multiple spanning tree state for each msti. 
 *    There are four states supported by ASIC.
 *      -STP_STATE_DISABLED
 *      -STP_STATE_BLOCKING
 *      -STP_STATE_LEARNING
 *      -STP_STATE_FORWARDING
 */
extern int32 rtk_stp_mstpState_get(rtk_stp_msti_id_t msti, rtk_port_t port, rtk_stp_state_t *pStp_state);

/* LUT */

/* Function Name:
 *      rtk_l2_init
 * Description:
 *      Initialize l2 module of the specified device.
 * Input:
 *      None
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 * Note:
 *      Should initialize l2 module before calling any l2 APIs.
 */  
extern int32 rtk_l2_init(void);

/* Function Name:
 *      rtk_l2_addr_add
 * Description:
 *      Set LUT unicast entry.
 * Input:
 *      pMac  - 6 bytes unicast(I/G bit is 0) mac address to be written into LUT.
 *      port - Port id.
 *      fid     - filtering database for the input LUT entry.
 *      is_static  - static or dynamic for the input LUT entry.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK - 
 *      RT_ERR_MAC - Invalid MAC address.
 *      RT_ERR_INPUT - Invalid input parameters.
 *      RT_ERR_PORT_ID - Invalid port number.
 *      RT_ERR_L2_FID - Invalid FID .
 *      RT_ERR_L2_INDEXTBL_FULL - hashed index is full of entries.
 *      RT_ERR_FAILED - 
 * Note:
 *      If the unicast mac address already existed in LUT, it will udpate the status of the entry. 
 *      Otherwise, it will find an empty or asic auto learned entry to write. If all the entries 
 *      with the same hash value can't be replaced, ASIC will return a RT_ERR_L2_INDEXTBL_FULL error.
 */
extern int32 rtk_l2_addr_add(rtk_mac_t *pMac, rtk_l2_ucastAddr_t *pL2_data);

/* Function Name:
 *      rtk_l2_addr_get
 * Description:
 *      Get LUT unicast entry.
 * Input:
 *      pMac  - 6 bytes unicast(I/G bit is 0) mac address to get.
 *      fid - filtering database.
 * Output:
 *      pL2_data     -entry returned
 * Return:
 *      RT_ERR_OK - 
 *      RT_ERR_MAC - Invalid MAC address.
 *      RT_ERR_INPUT - Invalid input parameters.
 *      RT_ERR_L2_FID - Invalid FID .
 *      RT_ERR_L2_ENTRY_NOTFOUND -No such LUT entry.
 *      RT_ERR_FAILED - 
 * Note:
 *      If the unicast mac address existed in LUT, it will return the port and fid where
 *      the mac is learned. Otherwise, it will return a RT_ERR_L2_ENTRY_NOTFOUND error.
 */
extern int32 rtk_l2_addr_get(rtk_mac_t *pMac, rtk_fid_t fid, rtk_l2_ucastAddr_t *pL2_data);

/* Function Name:
 *      rtk_l2_addr_del
 * Description:
 *      Delete LUT unicast entry.
 * Input:
 *      mac  - 6 bytes unicast mac address to be deleted.
 *      fid - filtering database for the mac address to be deleted.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK - 
 *      RT_ERR_MAC - Invalid MAC address.
 *      RT_ERR_INPUT - Invalid input parameters.
 *      RT_ERR_L2_FID - Invalid FID .
 *      RT_ERR_L2_ENTRY_NOTFOUND -No such LUT entry.
 *      RT_ERR_FAILED - 
 * Note:
 *      If the mac has existed in the LUT, it will be deleted.
 *      Otherwise, it will return RT_ERR_L2_ENTRY_NOTFOUND.
 */
extern int32 rtk_l2_addr_del(rtk_mac_t *pMac, rtk_fid_t fid); 

/* Function Name:
 *      rtk_l2_mcastAddr_add
 * Description:
 *      Add LUT multicast entry.
 * Input:
 *      pMac  - 6 bytes multicast(I/G bit isn't 0) mac address to be written into LUT.
 *      portmask - Port mask to be forwarded to.
 *      fid     - filtering database for the input LUT entry.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK - 
 *      RT_ERR_FAILED  - 
 *      RT_ERR_MAC - Invalid MAC address.
 *      RT_ERR_INPUT - Invalid input parameters.
 *      RT_ERR_L2_FID - Invalid FID .
 *      RT_ERR_L2_INDEXTBL_FULL - hashed index is full of entries.
 *      RT_ERR_PORT_MASK - Invalid portmask.
 * Note:
 *      If the multicast mac address already existed in the LUT, it will udpate the
 *      port mask of the entry. Otherwise, it will find an empty or asic auto learned
 *      entry to write. If all the entries with the same hash value can't be replaced, 
 *      ASIC will return a RT_ERR_L2_INDEXTBL_FULL error.
 */
extern int32 rtk_l2_mcastAddr_add(rtk_mac_t *pMac, rtk_fid_t fid, rtk_portmask_t portmask);

/* Function Name:
 *      rtk_l2_mcastAddr_get
 * Description:
 *      Get LUT multicast entry.
 * Input:
 *      pMac  - 6 bytes multicast(I/G bit is 1) mac address to get.
 *      pPort - pointer to returned port number.
 *      fid     - filtering database for the input LUT entry.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK - 
 *      RT_ERR_FAILED  - 
 *      RT_ERR_MAC - Invalid MAC address.
 *      RT_ERR_INPUT - Invalid input parameters.
 *      RT_ERR_L2_ENTRY_NOTFOUND - No such LUT entry.
 * Note:
 *      If the unicast mac address existed in the LUT, it will return the port mask. Otherwise, it will return a RT_ERR_L2_ENTRY_NOTFOUND error.
 */
extern int32 rtk_l2_mcastAddr_get(rtk_mac_t *pMac, rtk_fid_t fid, rtk_portmask_t *pPortmask);

/* Function Name:
 *      rtk_l2_mcastAddr_del
 * Description:
 *      Delete LUT unicast entry.
 * Input:
 *      mac  -  6 bytes multicast mac address to be deleted.
 *      fid     - filtering database for the mac address to be deleted.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK - 
 *      RT_ERR_FAILED  - 
 *      RT_ERR_MAC - Invalid MAC address.
 *      RT_ERR_INPUT - Invalid input parameters.
 *      RT_ERR_L2_ENTRY_NOTFOUND - No such LUT entry.
 * Note:
 *      If the mac has existed in the LUT, it will be deleted.
 *      Otherwise, it will return RT_ERR_L2_ENTRY_NOTFOUND.
 */
extern int32 rtk_l2_mcastAddr_del(rtk_mac_t *pMac, rtk_fid_t fid);    

/* Function Name:
 *      rtk_l2_ipMcastAddr_add
 * Description:
 *      Set Lut IP multicast lookup function.
 * Input:
 *      dip  -  Destination IP Address. 
 *      sip     -  Source IP Address.
 *      portmask     - Destination port mask.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK - 
 *      RT_ERR_FAILED  - 
 *      RT_ERR_PORT_MASK - Invalid portmask.
 *      RT_ERR_INPUT - Invalid input parameters.
 *      RT_ERR_L2_INDEXTBL_FULL - hashed index is full of entries.
 *      RT_ERR_L2_FWDTBL_FULL - forwarding table is full.
 * Note:
 *      System supports L2 entry with IP multicast DIP/SIP to forward IP multicasting frame as user 
 *      desired. If this function is enabled, then system will be looked up L2 IP multicast entry to 
 *      forward IP multicast frame directly without flooding.
 */
extern int32 rtk_l2_ipMcastAddr_add(ipaddr_t sip, ipaddr_t dip, rtk_portmask_t portmask);

/* Function Name:
 *      rtk_l2_ipMcastAddr_get
 * Description:
 *      Get Lut IP multicast lookup function setting.
 * Input:
 *      dip  -  Destination IP Address. 
 *      sip     -  Source IP Address.
 * Output:
 *      pPortmask     - Destination port mask.
 * Return:
 *      RT_ERR_OK - 
 *      RT_ERR_FAILED  - 
 *      RT_ERR_PORT_MASK - Invalid portmask.
 *      RT_ERR_INPUT - Invalid input parameters.
 *      RT_ERR_L2_INDEXTBL_FULL - hashed index is full of entries.
 * Note:
 *     The API can get Lut table of IP multicast entry.
 */
extern int32 rtk_l2_ipMcastAddr_get(ipaddr_t sip, ipaddr_t dip, rtk_portmask_t *pPortmask);

/* Function Name:
 *      rtk_l2_ipMcastAddr_del
 * Description:
 *      Delete a L2 ip multicast address entry from the specified device.
 * Input:
 *      dip  -  Destination IP Address. 
 *      sip     -  Source IP Address.
 * Output:
 *      pPortmask     - Destination port mask.
 * Return:
 *      RT_ERR_OK - 
 *      RT_ERR_FAILED  - 
 *      RT_ERR_INPUT - Invalid input parameters.
 *      RT_ERR_L2_ENTRY_NOTFOUND - No such LUT entry.
 * Note:
 *      The API can delete a L2 ip multicast address entry from the specified device.
 */   
extern int32 rtk_l2_ipMcastAddr_del(ipaddr_t sip, ipaddr_t dip);

/* Function Name:
 *      rtk_l2_flushType_set
 * Description:
 *      Flush L2 mac address by type in the specified device.
 * Input:
 *      type  -  flush type
 *      vid     -  VLAN id
 *      key     -  key for flushed item
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK - 
 *      RT_ERR_FAILED  - 
 *      RT_ERR_INPUT - Invalid input parameters.
 *      RT_ERR_PORT_ID - Invalid port number.
 *      RT_ERR_PORT_MASK - Invalid portmask.
 *      RT_ERR_VLAN_VID - Invalid VID parameter.
 * Note:
 *      This function trigger flushing of per-port L2 learning.
 *      When flushing operaton completes, the corresponding bit will be clear.
 *      The flush type as following:
 *      FLUSH_TYPE_BY_PORT        (physical port)
 *      FLUSH_TYPE_BY_FID
 *      FLUSH_TYPE_BY_MAC
 *      As for RTL8316D, flush types could be combined by '|' operator
 */ 
extern int32 rtk_l2_flushType_set(rtk_l2_flushType_t type, rtk_vlan_t vid, uint32 portOrTid);


/* Function Name:
 *      rtk_l2_flushLinkDownPortAddrEnable_set
 * Description:
 *      Set HW flush linkdown port mac configuration of the specified device.
 * Input:
 *      port  -  Port id.
 *      enable     -  link down flush status
 *      key     -  key for flushed item
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK - 
 *      RT_ERR_FAILED  - 
 *      RT_ERR_INPUT - Invalid input parameters.
 *      RT_ERR_PORT_ID - Invalid port number.
 * Note:
 *      The status of flush linkdown port address is as following:
 *           -DISABLED
 *           -ENABLED
 */  
extern int32 rtk_l2_flushLinkDownPortAddrEnable_set(rtk_port_t port, rtk_enable_t enable);

/* Function Name:
 *      rtk_l2_flushLinkDownPortAddrEnable_get
 * Description:
 *      Get HW flush linkdown port mac configuration of the specified device.
 * Input:
 *      port  -  Port id.
 *      enable     -  link down flush status
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK - 
 *      RT_ERR_FAILED  - 
 *      RT_ERR_INPUT - Invalid input parameters.
 *      RT_ERR_PORT_ID - Invalid port number.
 * Note:
 *      The status of flush linkdown port address is as following:
 *           -DISABLED
 *           -ENABLED
 */
extern int32 rtk_l2_flushLinkDownPortAddrEnable_get(rtk_port_t port, rtk_enable_t *pEnable);

/* Function Name:
 *      rtk_l2_agingEnable_set
 * Description:
 *      Set L2 LUT aging status per port setting.
 * Input:
 *      port  -  Port id.
 *      enable     -  Aging status
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK - 
 *      RT_ERR_FAILED  - 
 *      RT_ERR_INPUT - Invalid input parameters.
 *      RT_ERR_PORT_ID - Invalid port number.
 * Note:
 *      This API can be used to set L2 LUT aging status on port basis. 
 */ 
extern int32 rtk_l2_agingEnable_set(rtk_port_t port, rtk_enable_t enable);

/* Function Name:
 *      rtk_l2_agingEnable_get
 * Description:
 *      Get L2 LUT aging status per port setting.
 * Input:
 *      port  -  Port id.
 * Output:
 *      pEnable     -  Aging status
 * Return:
 *      RT_ERR_OK - 
 *      RT_ERR_FAILED  - 
 *      RT_ERR_INPUT - Invalid input parameters.
 *      RT_ERR_PORT_ID - Invalid port number.
 * Note:
 *      This API can be used to get L2 LUT aging function on port basis.  
 */
extern int32 rtk_l2_agingEnable_get(rtk_port_t port, rtk_enable_t *pEnable);

/* Function Name:
 *      rtk_l2_limitLearningCnt_set
 * Description:
 *       Set per-Port auto learning limit number
 * Input:
 *      port  -  Port id.
 *      mac_cnt  - Auto learning entries limit number
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK - 
 *      RT_ERR_FAILED  - 
 *      RT_ERR_LIMITED_L2ENTRY_NUM - Invalid auto learning limit number
 *      RT_ERR_PORT_ID - Invalid port number.
 * Note:
 *      The API can set per-port ASIC auto learning limit number from 0(disable learning) 
 *      to 0x1FFFF.
 */
extern int32 rtk_l2_limitLearningCnt_set(rtk_port_t port, rtk_mac_cnt_t mac_cnt);

/* Function Name:
 *      rtk_l2_limitLearningCnt_get
 * Description:
 *       Get per-Port auto learning limit number
 * Input:
 *      port  -  Port id.
 * Output:
 *      pMac_cnt     -  Auto learning entries limit number
 * Return:
 *      RT_ERR_OK - 
 *      RT_ERR_FAILED  - 
 *      RT_ERR_LIMITED_L2ENTRY_NUM - Invalid auto learning limit number
 *      RT_ERR_PORT_ID - Invalid port number.
 * Note:
 *     The API can get per-port ASIC auto learning limit number.
 */
extern int32 rtk_l2_limitLearningCnt_get(rtk_port_t port, rtk_mac_cnt_t *pMac_cnt);

/* Function Name:
 *      rtk_l2_limitLearningCntAction_set
 * Description:
 *      Configure auto learn over limit number action.
 * Input:
 *      port  -  Port id.
 *      action  -  Auto learning entries limit action
 * Output:
 *     None
 * Return:
 *      RT_ERR_OK - 
 *      RT_ERR_FAILED  - 
 *      RT_ERR_NOT_ALLOWED - Invalid learn over action
 *      RT_ERR_PORT_ID - Invalid port number.
 * Note:
 *      The API can set SA unknown packet action while auto learn limit number is over 
 *      The action symbol as following:
 *          -LIMIT_LEARN_CNT_ACTION_DROP,
 *          -LIMIT_LEARN_CNT_ACTION_FWD,
 *          -LIMIT_LEARN_CNT_ACTION_TO_CPU,
 */
extern int32 rtk_l2_limitLearningCntAction_set(rtk_port_t port, rtk_l2_limitLearnCntAction_t action);

/* Function Name:
 *      rtk_l2_limitLearningCntAction_get
 * Description:
 *      Configure auto learn over limit number action.
 * Input:
 *      port  -  Port id.
 * Output:
 *      pAction  -  Learn over action
 * Return:
 *      RT_ERR_OK - 
 *      RT_ERR_FAILED  - 
 *      RT_ERR_PORT_ID - Invalid port number.
 * Note:
 *      The API can get SA unknown packet action while auto learn limit number is over 
 *      The action symbol as following:
 *          -LIMIT_LEARN_CNT_ACTION_DROP,
 *          -LIMIT_LEARN_CNT_ACTION_FWD,
 *          -LIMIT_LEARN_CNT_ACTION_TO_CPU,
 */
extern int32 rtk_l2_limitLearningCntAction_get(rtk_port_t port, rtk_l2_limitLearnCntAction_t *pAction);

/* Function Name:
 *      rtk_l2_learningCnt_get
 * Description:
 *      Get per-Port auto learning number
 * Input:
 *      port  -  Port id.
 * Output:
 *      pMac_cnt  -  ASIC auto learning entries number
 * Return:
 *      RT_ERR_OK - 
 *      RT_ERR_FAILED  - 
 *      RT_ERR_PORT_ID - Invalid port number.
 * Note:
 *      The API can get per-port ASIC auto learning number
 */
extern int32 rtk_l2_learningCnt_get(rtk_port_t port, rtk_mac_cnt_t *pMac_cnt);

/* Function Name:
 *      rtk_l2_floodPortMask_set
 * Description:
 *      Set flooding portmask
 * Input:
 *      type  -  flooding type.
 *      flood_portmask  -  flooding porkmask
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK - 
 *      RT_ERR_FAILED  - 
 *      RT_ERR_PORT_MASK - Invalid portmask.
 * Note:
 *      This API set the flooding mask.
 *      The flooding type is as following:
 *           -FLOOD_UNKNOWNDA
 *           -FLOOD_UNKNOWNMC
 *           -FLOOD_BC
 */
extern int32 rtk_l2_floodPortMask_set(rtk_l2_flood_type_t floood_type, rtk_portmask_t flood_portmask);

/* Function Name:
 *      rtk_l2_floodPortMask_get
 * Description:
 *      Get flooding portmask
 * Input:
 *      type  -  flooding type.
 * Output:
 *      pFlood_portmask  -  flooding porkmask
 * Return:
 *      RT_ERR_OK - 
 *      RT_ERR_FAILED  - 
 *      RT_ERR_INPUT - Invalid input parameters.
 * Note:
 *      This API get the flooding mask.
 *      The flooding type is as following:
 *           -FLOOD_UNKNOWNDA
 *           -FLOOD_UNKNOWNMC
 *           -FLOOD_BC
 */
extern int32 rtk_l2_floodPortMask_get(rtk_l2_flood_type_t floood_type, rtk_portmask_t *pFlood_portmask);

/* Function Name:
 *      rtk_l2_localPktPermit_set
 * Description:
 *      Set permittion of frames if source port and destination port are the same.
 * Input:
 *      port  -  Port id.
 *      permit  -  permittion status
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK - 
 *      RT_ERR_FAILED  - 
 *      RT_ERR_PORT_ID - Invalid port number.
 *      RT_ERR_INPUT - Invalid input parameters.
 *      RT_ERR_ENABLE - Invalid permit value.
 * Note:
 *      This API is setted to permit frame if its source port is equal to destination port. 
 */
extern int32 rtk_l2_localPktPermit_set(rtk_port_t port, rtk_enable_t permit);

/* Function Name:
 *      rtk_l2_localPktPermit_get
 * Description:
 *      Get permittion of frames if source port and destination port are the same.
 * Input:
 *      port  -  Port id.
 * Output:
 *      pPermit  -  permittion status
 * Return:
 *      RT_ERR_OK - 
 *      RT_ERR_FAILED  - 
 *      RT_ERR_PORT_ID - Invalid port number.
 *      RT_ERR_INPUT - Invalid input parameters.
 *      RT_ERR_ENABLE - Invalid permit value.
 * Note:
 *      This API is to get permittion status for frames if its source port is equal to destination port.
 */
extern int32 rtk_l2_localPktPermit_get(rtk_port_t port, rtk_enable_t *pPermit);

/* Function Name:
 *      rtk_l2_aging_set
 * Description:
 *      Set LUT agging out speed
 * Input:
 *      aging_time  -  Agging out time.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK - 
 *      RT_ERR_FAILED  - 
 *      RT_ERR_OUT_OF_RANGE - input out of range.
 *      RT_ERR_ENABLE - Invalid permit value.
 * Note:
 *      The API can set LUT agging out period for each entry and the range is from 14s to 800s.
 */
extern int32 rtk_l2_aging_set(rtk_l2_age_time_t aging_time); 

/* Function Name:
 *      rtk_l2_aging_get
 * Description:
 *      Get LUT agging out time
 * Input:
 *      None
 * Output:
 *      pAging_time  -  Agging out time.
 * Return:
 *      RT_ERR_OK - 
 *      RT_ERR_FAILED  - 
 *      RT_ERR_ENABLE - Invalid permit value.
 * Note:
 *      The API can get LUT agging out period for each entry. 
 */
extern int32 rtk_l2_aging_get(rtk_l2_age_time_t *pAging_time);

/* Function Name:
 *      rtk_l2_hashMethod_set
 * Description:
 *      Select LUT hash algorithm.
 * Input:
 *      mode - hash algorithm, HASH_OPT0 or HASH_OPT1 .
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK - 
 *      RT_ERR_FAILED  - 
 *      RT_ERR_INPUT - Invalid mode.
 * Note:
 *      
 */
extern int32 rtk_l2_hashMethod_set(rtk_hash_method_t mode);

/* Function Name:
 *      rtk_l2_hashMethod_get
 * Description:
 *      Get LUT hash algorithm.
 * Input:
 *      None
 * Output:
 *      mode - hash algorithm, HASH_OPT0 or HASH_OPT1 .
 * Return:
 *      RT_ERR_OK - 
 *      RT_ERR_FAILED  - 
 *      RT_ERR_NULL_POINTER - Invalid mode pointer.
 * Note:
 *      
 */
extern int32 rtk_l2_hashMethod_get(rtk_hash_method_t *pMode);

/* Function Name:
 *      rtk_l2_ipMcastAddrLookup_set
 * Description:
 *      Configure the method of lookuping address table for IP multicast packets.
 * Input:
 *      type - method type.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK - 
 *      RT_ERR_FAILED  - 
 *      RT_ERR_NULL_POINTER - Invalid mode pointer.
 *      RT_ERR_INPUT - Invalid type.
 * Note:
 *      As for IP multicast packets, they could use MAC or SIP+DIP to lookup address table for forwarding.
 */
extern int32 rtk_l2_ipMcastAddrLookup_set(rtk_l2_lookup_type_t type);


/* Function Name:
 *      rtk_l2_ipMcastAddrLookup_get
 * Description:
 *      Get the method of lookuping address table for IP multicast packets.
 * Input:
 *      None
 * Output:
 *      type - method type.
 * Return:
 *      RT_ERR_OK - 
 *      RT_ERR_FAILED  - 
 *      RT_ERR_NULL_POINTER - Invalid mode pointer.
 * Note:
 *      As for IP multicast packets, they could use MAC or SIP+DIP to lookup address table for forwarding.
 */
extern int32 rtk_l2_ipMcastAddrLookup_get(rtk_l2_lookup_type_t *pType);

/* SVLAN */
/* Function Name:
 *      rtk_svlan_init
 * Description:
 *      Initialize SVLAN Configuration
 * Input:
 *      none
 * Output:
 *      none
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_SMI
 *      RT_ERR_INPUT
 * Note:
 *    Ether type of S-tag in 802.1ad is 0x88a8 and there are existed ether type 0x9100 and 0x9200 for Q-in-Q SLAN design. 
 *    User can set mathced ether type as service provider supported protocol. After call this API, 
 *    all ports are set as CVLAN port. you can use rtk_svlan_servicePort_add to add SVLAN port. 
 *    
 */
extern rtk_api_ret_t rtk_svlan_init(void);

/* Function Name:
 *      rtk_svlan_servicePort_add
 * Description:
 *      Enable one service port in the specified device
 * Input:
 *      port     -  Port id
 * Output:
 *      none
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_SMI
 * Note:
 *    This API is setting which port is connected to provider switch. All frames receiving from this port
 *    will recognize Service Vlan Tag.
 *    
 */
extern rtk_api_ret_t rtk_svlan_servicePort_add(rtk_port_t port);

/* Function Name:
 *      rtk_svlan_servicePort_del
 * Description:
 *      Disable one service port in the specified device
 * Input:
 *      none
 * Output:
 *      pSvlan_portmask  - svlan ports mask
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_SMI
 * Note:
 *      Ether type of S-tag in 802.1ad is 0x88a8 and there are existed ether type
 *      0x9100 and 0x9200 for Q-in-Q SLAN design. User can set mathced ether type as service 
 *      provider supported protocol. 
 */
extern rtk_api_ret_t rtk_svlan_servicePort_del(rtk_port_t port);

/* Function Name:
 *      rtk_svlan_servicePort_get
 * Description:
 *      Disable one service port in the specified device
 * Input:
 *      none
 * Output:
 *      pSvlan_portmask  - svlan ports mask
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 * Note:
 *      Ether type of S-tag in 802.1ad is 0x88a8 and there are existed ether type 0x9100 
 *      and 0x9200 for Q-in-Q SLAN design. User can set mathced ether type as service 
 *      provider supported protocol. 
 */
extern rtk_api_ret_t rtk_svlan_servicePort_get(rtk_portmask_t *pSvlan_portmask);

/* Function Name:
 *      rtk_svlan_tpidEntry_set
 * Description:
 *      Configure accepted S-VLAN ether type. The default ether type of S-VLAN is 0x88a8
 * Input:
 *      svlan_tag_id  - Ether type of S-tag frame parsing in uplink ports
 * Output:
 *      none
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_SMI
 * Note:
 *      Ether type of S-tag in 802.1ad is 0x88a8 and there are existed ether type 0x9100 
 *      and 0x9200 for Q-in-Q SLAN design. User can set mathced ether type as service 
 *      provider supported protocol. 
 */
extern rtk_api_ret_t rtk_svlan_tpidEntry_set(rtk_svlan_tpid_t svlan_tag_id);

/* Function Name:
 *      rtk_svlan_tpidEntry_get
 * Description:
 *      Get accepted S-VLAN ether type. The default ether type of S-VLAN is 0x88a8
 * Input:
 *      pSvlan_tag_id       - Ether type of S-tag frame parsing in uplink ports
 * Output:
 *      none
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 * Note:
 *      Ether type of S-tag in 802.1ad is 0x88a8 and there are existed ether type 0x9100 
 *      and 0x9200 for Q-in-Q SLAN design. User can set mathced ether type as service 
 *      provider supported protocol. 
 */
extern rtk_api_ret_t rtk_svlan_tpidEntry_get(rtk_svlan_tpid_t *pSvlan_tag_id);

/* Function Name:
 *      rtk_svlan_tpidEntry_set
 * Description:
 *      Configure accepted S-VLAN ether type. The default ether type of S-VLAN is 0x88a8
 * Input:
 *      svlan_tag_id  - Ether type of S-tag frame parsing in uplink ports
 * Output:
 *      none
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_SMI
 * Note:
 *      Ether type of S-tag in 802.1ad is 0x88a8 and there are existed ether type 0x9100 
 *      and 0x9200 for Q-in-Q SLAN design. User can set mathced ether type as service 
 *      provider supported protocol. 
 */
extern rtk_api_ret_t rtk_svlan_portPvid_set(rtk_port_t port, rtk_vlan_t pvid, rtk_pri_t priority, rtk_dei_t dei);

/* Function Name:
 *      rtk_svlan_tpidEntry_get
 * Description:
 *      Get accepted S-VLAN ether type. The default ether type of S-VLAN is 0x88a8
 * Input:
 *      pSvlan_tag_id       - Ether type of S-tag frame parsing in uplink ports
 * Output:
 *      none
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 * Note:
 *      Ether type of S-tag in 802.1ad is 0x88a8 and there are existed ether type 0x9100 
 *      and 0x9200 for Q-in-Q SLAN design. User can set mathced ether type as service 
 *      provider supported protocol. 
 */
extern rtk_api_ret_t rtk_svlan_portPvid_get(rtk_port_t port, rtk_vlan_t *pPvid, rtk_pri_t* pPriority, rtk_dei_t *pDei);

/* Function Name:
 *      rtk_svlan_portPvid_set
 * Description:
 *      Set port to specified VLAN ID(PVID) for Service Provider Port
 * Input:
 *      port             - Port id
 *      pvid             - Specified Service VLAN ID
 *      priority         - 802.1p priority for the PVID
 *      dei               - Service VLAN tag DEI bit
 * Output:
 *      none
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_VLAN_VID
 *      RT_ERR_SMI 
 *      RT_ERR_VLAN_PRIORITY 
 *      RT_ERR_VLAN_ENTRY_NOT_FOUND
 * Note:
 *    The API is used for Port-based VLAN. The untagged frame received from the
 *    port will be classified to the specified VLAN and assigned to the specified priority.
 */
extern rtk_api_ret_t rtk_svlan_portAcceptFrameType_set(rtk_port_t port, rtk_vlan_acceptFrameType_t accept_frame_type);

/* Function Name:
 *      rtk_svlan_portPvid_get
 * Description:
 *      Get Service VLAN ID(PVID) on specified port
 * Input:
 *      port             - Port id
 *      pPvid            - Specified VLAN ID
 *      pPriority        - 802.1p priority for the PVID
 *      pDei             - DEI bit
 * Output:
 *      none
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_SMI
 *      RT_ERR_PORT_ID
 *      RT_ERR_NULL_POINTER 
 * Note:
 *    The API is used for Port-based VLAN. The untagged frame received from the
 *    port will be classified to the specified VLAN and assigned to the specified priority.
 */
extern rtk_api_ret_t rtk_svlan_portAcceptFrameType_get(rtk_port_t port, rtk_vlan_acceptFrameType_t *pAccept_frame_type);

/* Function Name:
 *      rtk_vlan_stagMode_set
 * Description:
 *      Set SVLAN egress tag mode
 * Input:
 *      port                - Port id
 *      tag_mode        - Egress tag mode
 * Output:
 *      none
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_SMI 
 *      RT_ERR_INPUT
 * Note:
 *    The API is used for setting port vlan egress tag mode
 *    The tag mode  as following:
 *    VLAN_TAG_MODE_ORIGINAL
 *    VLAN_TAG_MODE_KEEP_FORMAT
 *    VLAN_TAG_MODE_REAL_KEEP_FORMAT
 */
extern rtk_api_ret_t rtk_svlan_tagMode_set(rtk_port_t port, rtk_vlan_tagMode_t tag_mode);

/* Function Name:
 *      rtk_svlan_tagMode_get
 * Description:
 *      Get SVLAN egress tag mode
 * Input:
 *      port                - Port id
 *      pTag_mode      - Egress tag mode
 * Output:
 *      none
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_SMI 
 *      RT_ERR_INPUT
 * Note:
 *    The API is used for setting port vlan egress tag mode
 *    The tag mode  as following:
 *    VLAN_TAG_MODE_ORIGINAL
 *    VLAN_TAG_MODE_KEEP_FORMAT
 *    VLAN_TAG_MODE_REAL_KEEP_FORMAT
 */
extern rtk_api_ret_t rtk_svlan_tagMode_get(rtk_port_t port, rtk_vlan_tagMode_t *pTag_mode);

/* CPU Port */
extern int32 rtk_cpu_enable_set(rtk_enable_t enable);
extern int32 rtk_cpu_enable_get(rtk_enable_t *pEnable);
extern int32 rtk_cpu_tagPort_set(rtk_port_t port, rtk_cpu_insert_t mode);
extern int32 rtk_cpu_tagPort_get(rtk_port_t *pPort, rtk_cpu_insert_t *pMode);
extern int32 rtk_cpu_tagPosition_set(rtk_cpu_position_t position);
extern int32 rtk_cpu_tagPosition_get(rtk_cpu_position_t *pPosition);

/* 802.1X */
/* Function Name:
 *      rtk_dot1x_tagunauthPacketOper_set
 * Description:
 *      Set 802.1x unauth action configuration 
 * Input:
 *      port                 - Port id
 *      unauth_action   - 802.1X unauth action
 * Output:
 *      none
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_SMI
 *      RT_ERR_PORT_ID
 *      RT_ERR_DOT1X_PROC
 * Note:
 *      This API can set 802.1x unauth action configuration 
 *      The unauth action is as following:
 *      DOT1X_ACTION_DROP
 *      DOT1X_ACTION_TRAP2CPU
 *      DOT1X_ACTION_GUESTVLAN 
 */
extern rtk_api_ret_t rtk_dot1x_unauthPacketOper_set(rtk_port_t port, rtk_dot1x_unauth_action_t unauth_action);

/* Function Name:
 *      rtk_dot1x_unauthPacketOper_get
 * Description:
 *      get 802.1x unauth action configuration 
 * Input:
 *      port                  - Port id
 * Output:
 *      pUnauth_action   - 802.1X unauth action
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_SMI
 *      RT_ERR_PORT_ID
 *      RT_ERR_NULL_POINTER
 * Note:
 *      This API can set 802.1x unauth action configuration 
 *      The unauth action is as following:
 *      DOT1X_ACTION_DROP
 *      DOT1X_ACTION_TRAP2CPU
 *      DOT1X_ACTION_GUESTVLAN 
 */
extern rtk_api_ret_t rtk_dot1x_unauthPacketOper_get(rtk_port_t port, rtk_dot1x_unauth_action_t *pUnauth_action);

/* Function Name:
 *      rtk_dot1x_eapolFrame2CpuEnable_set
 * Description:
 *      Set 802.1x EAPOL packet trap to CPU configuration 
 * Input:
 *      enable  - the status of 802.1x EAPOL packet
 * Output:
 *      none
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_SMI
 *      RT_ERR_ENABLE
 * Note:
 *   To support 802.1x authentication functionality, EAPOL frame (DMAC = 0x01-80-C2-00-00-03) has to
 *    be trapped to CPU.The status of EAPOL frame trap to CPU is as following:
 *    DISABLED
 *    ENABLED
 */
extern rtk_api_ret_t rtk_dot1x_eapolFrame2CpuEnable_set(rtk_enable_t enable);

/* Function Name:
 *      rtk_dot1x_eapolFrame2CpuEnable_get
 * Description:
 *      Get 802.1x EAPOL packet trap to CPU configuration
 * Input:
 *      none
 * Output:
 *      pEnable  - the status of 802.1x EAPOL packet 
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_SMI
 *      RT_ERR_NULL_POINTER
 * Note:
 *   To support 802.1x authentication functionality, EAPOL frame (DMAC = 0x01-80-C2-00-00-03) has to
 *    be trapped to CPU.The status of EAPOL frame trap to CPU is as following:
 *    DISABLED
 *    ENABLED
 */
extern rtk_api_ret_t rtk_dot1x_eapolFrame2CpuEnable_get(rtk_enable_t *pEnable);

/* Function Name:
 *      rtk_dot1x_portBasedEnable_set
 * Description:
 *      Set 802.1x port-based enable configuration
 * Input:
 *      port      - Port id
 *      enable   - The status of 802.1x port
 * Output:
 *      none
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_SMI
 *      RT_ERR_PORT_ID
 *      RT_ERR_ENABLE
 *      RT_ERR_DOT1X_PORTBASEDPNEN
 * Note:
 *      The API can update the port-based port enable register content. If a port is 802.1x 
 *       port based network access control "enabled", it should be authenticated so packets 
 *       from that port won't be dropped or trapped to CPU. The status of 802.1x port-based 
 *       network access control is as following:
 *       DISABLED
 *       ENABLED
 */
extern rtk_api_ret_t rtk_dot1x_portBasedEnable_set(rtk_port_t port, rtk_enable_t enable);

/* Function Name:
 *      rtk_dot1x_portBasedEnable_get
 * Description:
 *      Get 802.1x port-based enable configuration
 * Input:
 *      port        - Port id
 * Output:
 *      pEnable   - The status of 802.1x port
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_SMI
 *      RT_ERR_PORT_ID
 *      RT_ERR_ENABLE
 * Note:
 *     The API can get the 802.1x port-based port status.
 */
extern rtk_api_ret_t rtk_dot1x_portBasedEnable_get(rtk_port_t port, rtk_enable_t *pEnable);

/* Function Name:
 *      rtk_dot1x_portBasedAuthStatus_set
 * Description:
 *      Set 802.1x port-based auth. port configuration
 * Input:
 *      port          - Port id
 *      port_auth  - The status of 802.1x port
 * Output:
 *      none
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_SMI
 *      RT_ERR_PORT_ID
 *      RT_ERR_DOT1X_PORTBASEDAUTH
 * Note:
 *     The authenticated status of 802.1x port-based network access control is as following:
 *     UNAUTH
 *     AUTH
 */
extern rtk_api_ret_t rtk_dot1x_portBasedAuthStatus_set(rtk_port_t port, rtk_dot1x_auth_status_t port_auth);


/* Function Name:
 *      rtk_dot1x_portBasedAuthStatus_get
 * Description:
 *      get 802.1x port-based auth. port configuration
 * Input:
 *      port           - Port id
 * Output:
 *      pPort_auth  - The status of 802.1x port
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_SMI
 *      RT_ERR_INPUT
 *      RT_ERR_PORT_ID
 * Note:
 *      The API can get 802.1x port-based port auth.information.
 */
extern rtk_api_ret_t rtk_dot1x_portBasedAuthStatus_get(rtk_port_t port, rtk_dot1x_auth_status_t *pPort_auth);

/* Function Name:
 *      rtk_dot1x_portBasedDirection_set
 * Description:
 *      Set 802.1x port-based operational direction configuration
 * Input:
 *      port                 - Port id
 *      port_direction    - Operation direction
 * Output:
 *      none
 * Return:
 *      RT_ERR_OK                      
 *      RT_ERR_FAILED
 *      RT_ERR_SMI
 *      RT_ERR_INPUT
 *      RT_ERR_PORT_ID
 *      RT_ERR_DOT1X_PORTBASEDOPDIR
 * Note:
 *      The operate controlled direction of 802.1x port-based network access control is as following:
 *      BOTH
 *      IN
 */
extern rtk_api_ret_t rtk_dot1x_portBasedDirection_set(rtk_port_t port, rtk_dot1x_direction_t port_direction);

/* Function Name:
 *      rtk_dot1x_portBasedDirection_get
 * Description:
 *      get 802.1X port-based operational direction configuration
 * Input:
 *      port                   - Port id
 * Output:
 *      pPort_direction    - Operation direction
 * Return:
 *      RT_ERR_OK                      
 *      RT_ERR_FAILED
 *      RT_ERR_SMI
 *      RT_ERR_INPUT
 *      RT_ERR_PORT_ID
 * Note:
 *      The API can get 802.1x port-based operational direction information.
 */
extern rtk_api_ret_t rtk_dot1x_portBasedDirection_get(rtk_port_t port, rtk_dot1x_direction_t *pPort_direction);


/* Function Name:
 *      rtk_dot1x_macBasedEnable_set
 * Description:
 *      Set 802.1x mac-based port enable configuration
 * Input:
 *      port                   - Port id
 *      enable                - the status of 802.1x port 
 * Output:
 *      pPort_direction    - Operation direction
 * Return:
 *      RT_ERR_OK                      
 *      RT_ERR_FAILED
 *      RT_ERR_SMI
 *      RT_ERR_ENABLE
 *      RT_ERR_PORT_ID
 *      RT_ERR_DOT1X_MACBASEDPNEN
 * Note:
 *     If a port is 802.1x MAC based network access control "enabled", the incoming packets should 
 *     be authenticated so packets from that port won't be dropped or trapped to CPU.
 *     The status of 802.1x MAC-based network access control is as following:
 *     DISABLED
 *     ENABLED
 */
extern rtk_api_ret_t rtk_dot1x_macBasedEnable_set(rtk_port_t port, rtk_enable_t enable);

/* Function Name:
 *      rtk_dot1x_macBasedEnable_get
 * Description:
 *      get 802.1x mac-based port enable configuration
 * Input:
 *      port                   - Port id
 * Output:
 *      pEnable              - the status of 802.1x port 
 * Return:
 *      RT_ERR_OK                      
 *      RT_ERR_FAILED
 *      RT_ERR_SMI
 *      RT_ERR_ENABLE
 *      RT_ERR_PORT_ID
 *      RT_ERR_INPUT
 * Note:
 *   If a port is 802.1x MAC based network access control "enabled", the incoming packets should 
 *   be authenticated so packets from that port wont be dropped or trapped to CPU.
 *   The status of 802.1x MAC-based network access control is as following:
 *   DISABLED
 *   ENABLED
 */
extern rtk_api_ret_t rtk_dot1x_macBasedEnable_get(rtk_port_t port, rtk_enable_t *pEnable);

/* Function Name:
 *      rtk_dot1x_macBasedAuthMac_add
 * Description:
 *      Add an authenticated MAC to ASIC
 * Input:
 *      port                   - Port id
 *      pAuth_mac          - The authenticated MAC
 *      fid                      -  filtering database id  
 * Output:
 *      none
 * Return:
 *      RT_ERR_OK                      
 *      RT_ERR_FAILED
 *      RT_ERR_SMI
 *      RT_ERR_MAC
 *      RT_ERR_INPUT
 *      RT_ERR_PORT_ID
 * Note:
 *     The API can add a 802.1x authenticated MAC address to port. If the MAC does not exist in LUT, 
 *     user can't add this MAC to auth status.
 */
extern rtk_api_ret_t rtk_dot1x_macBasedAuthMac_add(rtk_port_t port, rtk_mac_t *pAuth_mac, rtk_fid_t fid);

/* Function Name:
 *      rtk_dot1x_macBasedAuthMac_del
 * Description:
 *      Delete an authenticated MAC to ASIC
 * Input:
 *      port                 -  Port id
 *      fid                   -  filtering database id  
 * Output:
 *      pAuth_mac        - The authenticated MAC
 * Return:
 *      RT_ERR_OK                      
 *      RT_ERR_FAILED
 *      RT_ERR_SMI
 *      RT_ERR_MAC
 *      RT_ERR_INPUT
 *      RT_ERR_PORT_ID
 * Note:
 *     The API can delete a 802.1x authenticated MAC address to port. It only change the auth status of
 *     the MAC and won't delete it from LUT.
 */
extern rtk_api_ret_t rtk_dot1x_macBasedAuthMac_del(rtk_port_t port, rtk_mac_t *pAuth_mac, rtk_fid_t fid);

/* Function Name:
 *      rtk_dot1x_macBasedDirection_set
 * Description:
 *      Set 802.1x mac-based operational direction configuration
 * Input:
 *      mac_direction    -  Operation direction
 * Output:
 * Return:
 *      RT_ERR_OK                      
 *      RT_ERR_FAILED
 *      RT_ERR_SMI
 *      RT_ERR_INPUT
 *      RT_ERR_PORT_ID
 *      RT_ERR_DOT1X_MACBASEDOPDIR
 * Note:
 *     The operate controlled direction of 802.1x mac-based network access control is as following:
 *     BOTH
 *     IN
 */
extern rtk_api_ret_t rtk_dot1x_macBasedDirection_set(rtk_dot1x_direction_t mac_direction);

/* Function Name:
 *      rtk_dot1x_macBasedDirection_get
 * Description:
 *      get 802.1x mac-based operational direction configuration
 * Input:
 *      none  
 * Output:
 *      pMac_direction - Operation direction
 * Return:
 *      RT_ERR_OK                      
 *      RT_ERR_FAILED
 *      RT_ERR_SMI
 *      RT_ERR_INPUT
 * Note:
 *      The API can get 802.1x mac-based operational direction information.
 */
extern rtk_api_ret_t rtk_dot1x_macBasedDirection_get(rtk_dot1x_direction_t *pMac_direction);

/* Function Name:
 *      rtk_dot1x_guestVlan_set
 * Description:
 *      Set 802.1x guest VLAN configuration
 * Input:
 *      vid  - guest VLAN ID
 * Output:
 *      none
 * Return:
 *      RT_ERR_OK                      
 *      RT_ERR_FAILED
 *      RT_ERR_SMI
 * Note:
 *      The API can set 802.1x guest VLAN ID.
 */
extern rtk_api_ret_t rtk_dot1x_guestVlan_set(rtk_vlan_t vid);

/* Function Name:
 *      rtk_dot1x_guestVlan_get
 * Description:
 *      Get 802.1x guest VLAN configuration
 * Input:
 *      none  
 * Output:
 *      pVid - guest VLAN ID
 * Return:
 *      RT_ERR_OK                      
 *      RT_ERR_FAILED
 *      RT_ERR_SMI
 *      RT_ERR_NULL_POINTER
 * Note:
 *      The API can get 802.1x guest VLAN ID.
 */
extern rtk_api_ret_t rtk_dot1x_guestVlan_get(rtk_vlan_t *pVid);

/* Function Name:
 *      rtk_dot1x_guestVlan2Auth_set
 * Description:
 *      Set 802.1x guest VLAN to auth host configuration
 * Input:
 *      enable  - The status of guest VLAN to auth host 
 * Output:
 *      none
 * Return:
 *      RT_ERR_OK                      
 *      RT_ERR_FAILED
 *      RT_ERR_SMI
 *      RT_ERR_NULL_POINTER
 * Note:
 *      The operational direction of 802.1x guest VLAN to auth host control is as following:
 *      DISABLED
 *      ENABLED
 */
extern rtk_api_ret_t rtk_dot1x_guestVlan2Auth_set(rtk_enable_t enable);

/* Function Name:
 *      rtk_dot1x_guestVlan2Auth_get
 * Description:
 *      Get 802.1x guest VLAN to auth host configuration
 * Input:
 *      none
 * Output:
 *      pEnable  - The status of guest VLAN to auth host 
 * Return:
 *      RT_ERR_OK                      
 *      RT_ERR_FAILED
 *      RT_ERR_SMI
 *      RT_ERR_NULL_POINTER
 * Note:
 *      The operational direction of 802.1x guest VLAN to auth host control is as following:
 *      DISABLED
 *      ENABLED
 */
extern rtk_api_ret_t rtk_dot1x_guestVlan2Auth_get(rtk_enable_t *pEnable);

/* Port Trunk */

/* Function Name:
 *      rtk_trunk_port_get
 * Description:
 *      Get trunking group available port mask
 * Input:
 *      trk_gid - trunk group id
 * Output:
 *      pTrunk_member_portmask  - Logic trunking member port mask
 * Return:
 *      RT_ERR_OK                      
 *      RT_ERR_FAILED
 *      RT_ERR_SMI
 *      RT_ERR_NULL_POINTER - NULL pointer input
 *      RT_ERR_LA_TRUNK_ID - error trunk ID
 * Note:
 *      RTL8316D has four groups of trunk.
 */
extern int32 rtk_trunk_port_get(rtk_trunk_group_t trk_gid, rtk_portmask_t *pTrunk_member_portmask);


/* Function Name:
 *      rtk_trunk_port_set
 * Description:
 *      Set trunking group available port mask
 * Input:
 *      trk_gid - trunk group id
 *      pTrunk_member_portmask  - Logic trunking member port mask
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK                      
 *      RT_ERR_FAILED
 *      RT_ERR_SMI
 *      RT_ERR_LA_TRUNK_ID - error trunk ID
 *      RT_ERR_PORT_MASK - error port mask
 *      RT_ERR_LA_PORTNUM_NORMAL - every port could support no more than 4 ports.
 * Note:
 *      The API can set 4 port trunking group enabled port mask. Each port trunking group has max 4 ports.
 *      If enabled port mask has less than 2 ports available setting, then this trunking group function is disabled.    
 */
extern int32 rtk_trunk_port_set(rtk_trunk_group_t trk_gid, rtk_portmask_t trunk_member_portmask); 

/* Function Name:
 *      rtk_trunk_qeueuEmptyStatus_get
 * Description:
 *      get current output queue if empty status
 * Input:
 *     None
 * Output:
 *      pPortmask  - queue empty port mask, 1 for empty and 0 for not empty
 * Return:
 *      RT_ERR_OK                      
 *      RT_ERR_FAILED
 *      RT_ERR_SMI
 *      RT_ERR_NULL_POINTER - NULL pointer input
 * Note:
 *      The API can get port mask whose queues are empty.
 */
 extern int32 rtk_trunk_qeueuEmptyStatus_get(rtk_portmask_t *pPortmask);

/*Port Mirror */

/* Function Name:
 *      rtk_mirror_portBased_set
 * Description:
 *      Set port mirror function
 * Input:
 *      pMirrored_rx_portmask   - Rx mirror port mask
 *      pMirrored_tx_portmask   - Tx mirror port mask
 * Output:
 *      none
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_SMI
 *      RT_ERR_PORT_ID
 *      RT_ERR_PORT_MASK
 * Note:
 *      The API is to set mirror function of  mirror port and Rx/Tx mirror ports
 *    
 */
extern rtk_api_ret_t rtk_mirror_portBased_set(rtk_port_t mirroring_port, rtk_portmask_t *pMirrored_rx_portmask, rtk_portmask_t *pMirrored_tx_portmask);


/* Function Name:
 *      rtk_mirror_portBased_get
 * Description:
 *      Get port mirror function
 * Input:
 *      none
 * Output:
 *      pMirrored_rx_portmask   - Rx mirror port mask
 *      pMirrored_tx_portmask   - Tx mirror port mask
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_SMI 
 *      RT_ERR_NULL_POINTER
 * Note:
 *      The API is to get mirror function of  mirror port and Rx/Tx mirror ports
 *    
 */
extern rtk_api_ret_t rtk_mirror_portBased_get(rtk_port_t* pMirroring_port, rtk_portmask_t *pMirrored_rx_portmask, rtk_portmask_t *pMirrored_tx_portmask);


/* Function Name:
 *      rtk_mirror_portIso_set
 * Description:
 *      Set mirror port isolation
 * Input:
 *      enable   - Mirror isolation status
 * Output:
 *      none
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_SMI
 *      RT_ERR_ENABLE
 * Note:
 *    The API is to set mirror isolation function that prevent normal forwarding packets to miror port.
 *    
 */
extern rtk_api_ret_t rtk_mirror_portIso_set(rtk_enable_t enable);

/* Function Name:
 *      rtk_mirror_portIso_get
 * Description:
 *      Get mirror port isolation
 * Input:
 *      none
 * Output:
 *      pEnable   - Mirror isolation status
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_SMI
 *      RT_ERR_ENABLE
 * Note:
 *    The API is to get mirror isolation status.
 *    
 */
extern rtk_api_ret_t rtk_mirror_portIso_get(rtk_enable_t *pEnable);

/* MIB */

/* Function Name:
 *      rtk_stat_global_reset
 * Description:
 *     Reset global MIB counter.
 * Input:
 *      None
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK  - 
 *      RT_ERR_FAILED   - 
 *      RT_ERR_INPUT - Invalid input parameter.
 * Note:
 *      Reset MIB counter of ports. API will use global reset while port mask is all-ports.
 */
extern int32 rtk_stat_global_reset(void);

/* Function Name:
 *      rtk_stat_port_reset
 * Description:
 *     Reset per port MIB counter by port.
 * Input:
 *      port -  port id.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK  - 
 *      RT_ERR_FAILED   - 
 *      RT_ERR_INPUT - Invalid input parameter.
 * Note:
 *     Reset MIB counter of ports. API will use global reset while port mask is all-ports.
 */
extern int32 rtk_stat_port_reset(rtk_port_t port);

/* Function Name:
 *      rtk_stat_global_get
 * Description:
 *      get global MIB counter.
 * Input:
 *      cntr_idx -  counter index
 * Output:
 *      pCntr -  counter returned
 * Return:
 *      RT_ERR_OK  - 
 *      RT_ERR_FAILED   - 
 *      RT_ERR_STAT_INVALID_GLOBAL_CNTR - Invalid input parameter.
 * Note:
 *     Get global MIB counter.
 */
extern int32 rtk_stat_global_get(rtk_stat_global_type_t cntr_idx, rtk_stat_counter_t *pCntr);

/* Function Name:
 *      rtk_stat_port_get
 * Description:
 *      Get port MIB counter.
 * Input:
 *      port -  port id.
 *      cntr_idx -  counter index.
 * Output:
 *      pCntr -  counter returned.
 * Return:
 *      RT_ERR_OK  - 
 *      RT_ERR_FAILED   - 
 *      RT_ERR_INPUT - Invalid input parameter.
 * Note:
 *     Get port MIB counter.
 */
extern int32 rtk_stat_port_get(rtk_port_t port, rtk_stat_port_type_t cntr_idx, rtk_stat_counter_t *pCntr);

/* Interrupt */
    
/* Function Name:
 *      rtk_int_globalEn_set
 * Description:
 *      Enable/disable the global interrupt mask.
 * Input:
 *      enable -  Interrupt mask, should be ENABLED or DISABLED.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK  - 
 *      RT_ERR_FAILED   - 
 *      RT_ERR_ENABLE - Invalid enable input.
 * Note:
 *      The API can enable/disable the global interrupt mask. Any interrupt could be signled to 
 *      CPU/external interrupt output pin only after enabling this global interrupt mask.  
 */
extern int32 rtk_int_globalEn_set(rtk_enable_t enable);
        
/* Function Name:
 *      rtk_int_globalEn_get
 * Description:
 *      Get the global interrupt mask .
 * Input:
 *      None
 * Output:
 *      pEnable -  Interrupt mask, should be ENABLED or DISABLED.
 * Return:
 *      RT_ERR_OK  - 
 *      RT_ERR_FAILED   - 
 *      RT_ERR_NULL_POINTER - NULL pointer input.
 * Note:
 *      The API can retrieve whether the global interrupt mask is enabled or not. Any interrupt 
 *      could be signled to CPU/external interrupt output pin only after enabling this global interrupt mask.  
 */
extern int32 rtk_int_globalEn_get(rtk_enable_t enable);

/* Function Name:
 *      rtk_int_modMask_set
 * Description:
 *      Set modules' interrupt mask configuration.
 * Input:
 *      type - Interrupt type.
 *      enable - Interrupt mask.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK  - 
 *      RT_ERR_FAILED   - 
 *      RT_ERR_INPUT - Error interrupt type.
 *      RT_ERR_ENABLE - enable should ENABLED or DISABLED.
 * Note:
 *      The API can set modules' global interrupt mask configuration.
 *      The modules' interrupt mask is shown in the following:
 *      -INT_GLOBAL_ACLHIT0
 *      -INT_GLOBAL_ACLHIT1
 *      -INT_GLOBAL_ACLHIT2
 *      -INT_GLOBAL_ACLHIT3
 *      -INT_GLOBAL_SAMOV
 *      -INT_GLOBAL_MIBOVERFLOW 
 *      -INT_GLOBAL_UDLD
 *      -INT_GLOBAL_LINKCHANGE
 *      -INT_GLOBAL_PKTDROP
 *      -INT_GLOBAL_SALRNLIMIT
 *      -INT_GLOBAL_LOOPDETECT
 *      -INT_GLOBAL_SERDES
 *      -INT_GLOBAL_DYINGGSP
 *      -INT_GLOBAL_LOOPRESOLVED
 */
extern int32 rtk_int_modMask_set(rtk_int_type_t type, rtk_enable_t enable);

/* Function Name:
 *      rtk_int_modMask_get
 * Description:
 *      Get modules' interrupt mask configuration.
 * Input:
 *      type - Interrupt type.
 * Output:
 *      pEnable - Interrupt mask value.
 * Return:
 *      RT_ERR_OK  - 
 *      RT_ERR_FAILED   - 
 *      RT_ERR_INPUT - Error interrupt type.
 *      RT_ERR_NULL_POINTER - NULL pointer input.
 * Note:
 *      The API can get modules' global interrupt mask configuration.
 *      The modules' interrupt mask is shown in the following:
 *      -INT_GLOBAL_ACLHIT0
 *      -INT_GLOBAL_ACLHIT1
 *      -INT_GLOBAL_ACLHIT2
 *      -INT_GLOBAL_ACLHIT3
 *      -INT_GLOBAL_SAMOV
 *      -INT_GLOBAL_MIBOVERFLOW 
 *      -INT_GLOBAL_UDLD
 *      -INT_GLOBAL_LINKCHANGE
 *      -INT_GLOBAL_PKTDROP
 *      -INT_GLOBAL_SALRNLIMIT
 *      -INT_GLOBAL_LOOPDETECT
 *      -INT_GLOBAL_SERDES
 *      -INT_GLOBAL_DYINGGSP
 *      -INT_GLOBAL_LOOPRESOLVED
 */
extern int32 rtk_int_modMask_get(rtk_int_type_t type, rtk_enable_t enable);

/* Function Name:
 *      rtk_int_modStatus_get
 * Description:
 *      Get modules' interrupt pending flags.
 * Input:
 *      None
 * Output:
 *      pStatusMask - Interrupt pending flag mask.
 * Return:
 *      RT_ERR_OK  - 
 *      RT_ERR_FAILED   - 
 *      RT_ERR_INPUT - Error interrupt type.
 *      RT_ERR_NULL_POINTER - NULL pointer input.
 * Note:
 *      The API can get modules' global interrupt pending flags.
 *      The bit mapping of modules' pending flags is shown in the following:
 *      bit0: INT_GLOBAL_RESERVED0 = 0,
 *      bit1: INT_GLOBAL_SERDES,
 *      bit2: INT_GLOBAL_LOOPDETECT,
 *      bit3: INT_GLOBAL_SALRNLIMIT,
 *      bit4: INT_GLOBAL_PKTDROP,
 *      bit5: INT_GLOBAL_LINKCHANGE,
 *      bit6: INT_GLOBAL_RESERVED6 = 6,
 *      bit7: INT_GLOBAL_RESERVED7 = 7,
 *      bit8: INT_GLOBAL_UDLD,
 *      bit9: INT_GLOBAL_MIBOVERFLOW,
 *      bit10:INT_GLOBAL_SAMOV,
 *      bit11:INT_GLOBAL_ACLHIT0,
 *      bit12:INT_GLOBAL_ACLHIT1,
 *      bit13:INT_GLOBAL_ACLHIT2,
 *      bit14:INT_GLOBAL_ACLHIT3,
 *      bit15:INT_GLOBAL_DYINGGSP,
 *      bit16:INT_GLOBAL_LOOPRESOLVED,    
 *      These flags reflect the corresponding modules' interrupt status, and will not be cleared unless the
 *      particular modules' (which have interrupt signaled) interrupt status relevent APIs are called.
 */
extern int32 rtk_int_modStatus_get(rtk_int_status_t* pStatusMask);

/* Function Name:
 *      rtk_int_linkChangeMsk_set
 * Description:
 *      Set ports' link status change interrupt masks.
 * Input:
 *      pPortMask - Port mask.
 *      enable - 
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK  - 
 *      RT_ERR_FAILED   - 
 *      RT_ERR_ENABLE -  Invalid enable input, shuold be ENABLED or DISABLED.
 *      RT_ERR_NULL_POINTER - NULL pointer input.
 * Note:
 *      The API can enable/disable ports to signal an interrupt when link status change. It will only affect
 *      those ports that are specificed in pPortMask.
 */
extern int32 rtk_int_linkChangeMsk_set(rtk_portmask_t* pPortMask, rtk_enable_t enable);

/* Function Name:
 *      rtk_int_linkChangeMsk_get
 * Description:
 *      Get port's link status change  interrupt mask.
 * Input:
 *      port - Port ID.
 * Output:
 *      pEnable  
 * Return:
 *      RT_ERR_OK  - 
 *      RT_ERR_FAILED   - 
 *      RT_ERR_PORT_ID -  Invalid port ID.
 *      RT_ERR_NULL_POINTER - NULL pointer input.
 * Note:
 *      The API can get port's link status change interrupt mask.
 */
extern int32 rtk_int_linkChangeMsk_get(rtk_port_t port, rtk_enable_t* pEnable);

/* Function Name:
 *      rtk_int_linkChangeStatus_get
 * Description:
 *      Get port's link status change interrupt pending flags.
 * Input:
 *      None
 * Output:
 *      pStatusMask -   Interrupt pending flags returned.
 * Return:
 *      RT_ERR_OK  - 
 *      RT_ERR_FAILED   - 
 *      RT_ERR_NULL_POINTER - NULL pointer input.
 * Note:
 *      The API can get port's link status change interrupt pending flags. Pending flags will be cleared after
  *     being retrieved.
 */
extern int32 rtk_int_linkChangeStatus_get(rtk_int_status_t* pStatusMask);

/* Function Name:
 *      rtk_int_smacLimitMsk_set
 * Description:
 *      Set ports' source MAC limit interrupt masks.
 * Input:
 *      pPortMask - Port mask.
 *      enable - ENABLED or DISABLED
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK  - 
 *      RT_ERR_FAILED   - 
 *      RT_ERR_ENABLE -  enable should be ENABLED or DISABLED.
 *      RT_ERR_PORT_ID -  Invalid port ID.
 *      RT_ERR_NULL_POINTER - NULL pointer input.
 * Note:
 *      The API can enable/disable ports to signal an interrupt when the number of source MAC auto-learned
 *      reaches the threshold allowed. It will only affect those ports that are specificed in pPortMask.
 */
extern int32 rtk_int_smacLimitMsk_set(rtk_portmask_t* pPortMask, rtk_enable_t enable);

/* Function Name:
 *      rtk_int_smacLimitMsk_get
 * Description:
 *      Get port's source MAC limit interrupt mask.
 * Input:
 *      port - Port ID.
 * Output:
 *      pEnable - ENABLED or DISABLED
 * Return:
 *      RT_ERR_OK  - 
 *      RT_ERR_FAILED   - 
 *      RT_ERR_PORT_ID -  Invalid port ID.
 *      RT_ERR_NULL_POINTER - NULL pointer input.
 * Note:
 *      The API can get port's interrupt mask of source MAC auto-learned limitation .
 */
extern int32 rtk_int_smacLimitMsk_get(rtk_port_t port, rtk_enable_t* pEnable);

/* Function Name:
 *      rtk_int_smacLimitStatus_get
 * Description:
 *      Get port's source MAC limit interrupt pending flags.
 * Input:
 *      None
 * Output:
 *      pStatusMask - Interrupt pending flags returned.
 * Return:
 *      RT_ERR_OK  - 
 *      RT_ERR_FAILED   - 
 *      RT_ERR_NULL_POINTER - NULL pointer input.
 * Note:
 *      The API can get ports' interrupt pending flags of source MAC auto-learned limitation .The interrupt 
 *      pending flags will be cleared after retrieved.
 */
extern int32 rtk_int_smacLimitStatus_get(rtk_int_status_t* pStatusMask);

/* Function Name:
 *      rtk_int_saMovStatus_get
 * Description:
 *      Get Source MAC moving interrupt pending flags.
 * Input:
 *      None
 * Output:
 *      pStatusMask - Interrupt pending flags returned.
 * Return:
 *      RT_ERR_OK  - 
 *      RT_ERR_FAILED   - 
 *      RT_ERR_NULL_POINTER - NULL pointer input.
 * Note:
 *      The API can get source MAC moving interrupt pending flags. Pending flags will be cleared after
 *      being retrieved.
 */
extern int32 rtk_int_saMovStatus_get(rtk_int_status_t* pStatusMask);

/* Function Name:
 *      rtk_int_loopDetecStatus_get
 * Description:
 *      Get loop detected interrupt pending flags.
 * Input:
 *      None
 * Output:
 *      pStatusMask - Interrupt pending flags returned.
 * Return:
 *      RT_ERR_OK  - 
 *      RT_ERR_FAILED   - 
 *      RT_ERR_NULL_POINTER - NULL pointer input.
 * Note:
 *      The API can get loop detected interrupt pending flags. Pending flags will be cleared after
 *      being retrieved.
 */
extern int32 rtk_int_loopDetecStatus_get(rtk_int_status_t* pStatusMask);

/* Function Name:
 *      rtk_int_udldStatus_get
 * Description:
 *      Get UniDirection Link Detect interrupt pending flags.
 * Input:
 *      None
 * Output:
 *      pStatusMask - Interrupt pending flags returned.
 * Return:
 *      RT_ERR_OK  - 
 *      RT_ERR_FAILED   - 
 *      RT_ERR_NULL_POINTER - NULL pointer input.
 * Note:
 *      The API can get UDLD interrupt pending flags. Pending flags will be cleared after
 *       being retrieved.
 */
extern int32 rtk_int_udldStatus_get(rtk_int_status_t* pStatusMask);

/* Function Name:
 *      rtk_int_aclHitMsk_set
 * Description:
 *      Set ACL rule hit interrupt masks.
 * Input:
 *      blockId - PIE module ID.
 *      ruleId - ACL rule ID.
 *      enable - Interrupt mask.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK  - 
 *      RT_ERR_FAILED   - 
 *      RT_ERR_INPUT - Error block ID or rule ID.
 *      RT_ERR_ENABLE - enable should be ENABLED or DISABLED.
 * Note:
 *      The API can enable/disable specified ACL rule to signal an interrupt when the incoming packet hit the rule.
 */
extern int32 rtk_int_aclHitMsk_set(rtk_aclBlock_id_t blockId, rtk_aclRule_id_t ruleId, rtk_enable_t enable);

/* Function Name:
 *      rtk_int_aclHitMsk_get
 * Description:
 *      Get ACL rule hit interrupt masks.
 * Input:
 *      blockId - PIE module ID.
 *      ruleId - ACL rule ID.
 * Output:
 *      pEnable - Interrupt mask returned.
 * Return:
 *      RT_ERR_OK  - 
 *      RT_ERR_FAILED   - 
 *      RT_ERR_INPUT - Error block ID or rule ID.
 *      RT_ERR_NULL_POINTER - NULL pointer returned.
 * Note:
 *      The API can get ACL rule hit interrupt mask.
 */
extern int32 rtk_int_aclHitMsk_get(rtk_aclBlock_id_t blockId, rtk_aclRule_id_t ruleId, rtk_enable_t* pEnable);

/* Function Name:
 *      rtk_int_aclHitStatus_get
 * Description:
 *      Get ACL rule hit interrupt pending flags.
 * Input:
 *      blockId - PIE module ID.
 * Output:
 *      pStatusMask - Interrupt pending flags returned.
 * Return:
 *      RT_ERR_OK  - 
 *      RT_ERR_FAILED   - 
 *      RT_ERR_INPUT - Error block ID.
 *      RT_ERR_NULL_POINTER - NULL pointer returned.
 * Note:
 *      The API can get ACL rule hit interrupt pending flags. The interrupt pending flags will be cleared
 *      after retrieved.
 */
extern int32 rtk_int_aclHitStatus_get(rtk_aclBlock_id_t blockId, rtk_int_status_t* pStatusMask);



/*LED*/
extern int32 rtk_led_mode_set(rtk_led_mode_t mode);
extern int32 rtk_led_mode_get(rtk_led_mode_t *pMode);
extern int32 rtk_led_blinkRate_set(rtk_led_blink_rate_t blinkRate);
extern int32 rtk_led_blinkRate_get(rtk_led_blink_rate_t *pBlinkRate);

/*Green ethernet*/
extern int32 rtk_green_feature_set(uint32 greenFeature, uint32 powerSaving);
extern int32 rtk_green_feature_get(uint32 greenFeature, uint32 powerSaving);

/*ACL APIs*/

/* Function Name:
 *      rtk_filter_igrAcl_init
 * Description:
 *      ACL module initialization.
 * Input:
 *     None
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK - Success.
 *      RT_ERR_FAILED   - 
 *      RT_ERR_INPUT - Invalid input parameters.
 * Note:
 *      The API initialized ACL modules. 
 */
extern int32 rtk_filter_igrAcl_init(void);


/* Function Name:
 *      rtk_filter_igrAcl_reInit
 * Description:
 *      ACL module re-initialization
 * Input:
 *      port  - the physical port where packet is to be transmitted
 * Output:
 *      pAbility - Ability structure
 * Return:
 *      RT_ERR_OK - Success.
 *      RT_ERR_FAILED   - 
 *      RT_ERR_INPUT - Invalid input parameters.
 * Note:
 *      The API re-initialized ACL modules. 
 */
extern int32 rtk_filter_igrAcl_reInit(void);

/* Function Name:
 *      rtk_filter_igrAcl_apply
 * Description:
 *      write ACL rules into ASIC
 * Input:
  *      blockId  - ACL block ID
  * Output:
 *      None
 * Return:
 *      RT_ERR_OK - Success.
 *      RT_ERR_FAILED   - 
 *      RT_ERR_INPUT - Invalid input parameters.
 * Note:
 *      The API write ACL rules into ASIC. 
 */
extern int32 rtk_filter_igrAcl_apply(rtk_aclBlock_id_t blockId);

/* Function Name:
 *      rtk_filter_unmatchAction_set
 * Description:
 *      Set action to packets when no ACL rule match
 * Input:
 *      port - port ID
 *      action 
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK  - 
 *      RT_ERR_FAILED   - 
 *      RT_ERR_PORT_ID - error input port ID.
 *      RT_ERR_INPUT - Invalid input action.
 *      RT_ERR_SMI - SMI access error.
 * Note:
 *      This function sets action of packets when no ACL configruation matches.
 *      Valid action including:
 *          (1). FILTER_UNMATCH_PERMIT
 *          (2). FILTER_UNMATCH_DROP
 */
extern int32 rtk_filter_unmatchAction_set(rtk_filter_port_t port, rtk_filter_unmatch_action_t action);

/* Function Name:
 *      rtk_filter_unmatchAction_get
 * Description:
 *      Get action to packets when no ACL rule match
 * Input:
 *      port - port ID
 * Output:
 *      pAction - action returned 
 * Return:
 *      RT_ERR_OK  - 
 *      RT_ERR_FAILED   - 
 *      RT_ERR_NULL_POINTER - NULL pointer input.
 *      RT_ERR_PORT_ID - error input port ID.
 *      RT_ERR_SMI - SMI access error.
 * Note:
 *      This function gets action of packets when no ACL rule matches.
 *      Valid action including:
 *          (1). FILTER_UNMATCH_PERMIT
 *          (2). FILTER_UNMATCH_DROP
 */
extern int32 rtk_filter_unmatchAction_get(rtk_filter_port_t port, rtk_filter_unmatch_action_t* pAction);

/* Function Name:
 *      rtk_filter_templ_set
 * Description:
 *      Set template field type
 * Input:
 *      blockId  - ACL block ID
 *      fieldId  - template field index
 *      fieldType  - template type
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK - Success.
 *      RT_ERR_FAILED   - 
 *      RT_ERR_INPUT - Invalid input parameters.
 * Note:
 *      The API can set template field type  
 */
extern int32 rtk_filter_templ_set(rtk_aclBlock_id_t blockId, rtk_filter_templField_id_t fieldId, rtk_filter_field_type_t fieldType);

/* Function Name:
 *      rtk_filter_templ_get
 * Description:
 *      Get template field type
 * Input:
 *      blockId  - ACL block ID
 *      fieldId  - template field index
 * Output:
 *      pFieldType - template type
 * Return:
 *      RT_ERR_OK - Success.
 *      RT_ERR_FAILED   - 
 *      RT_ERR_INPUT - Invalid input parameters.
 * Note:
 *      The API can get template field type  
 */
extern int32 rtk_filter_templ_get(rtk_aclBlock_id_t blockId, rtk_filter_templField_id_t fieldId, rtk_filter_field_type_t* pFieldType);

/* Function Name:
 *      rtk_filter_reverseHit_set
 * Description:
 *      Reverse the ACL rule hit result
 * Input:
 *      blockId  - ACL block ID
 *      ruleId  - rule specified
 *      reverse  - 1: reverse, 0: don't reverse
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK - Success.
 *      RT_ERR_FAILED   - 
 *      RT_ERR_INPUT - Invalid input parameters.
 *      RT_ERR_ENABLE - Invalid input parameters.
 * Note:
 *      This API can reverse the hit result of specified ACL rule.
 */
extern int32 rtk_filter_reverseHit_set(rtk_aclBlock_id_t blockId, rtk_aclRule_id_t ruleId, rtk_enable_t reverse);

/* Function Name:
 *      rtk_filter_reverseHit_get
 * Description:
 *      Retrieve  reverse status of ACL rule hit result
 * Input:
 *      blockId  - ACL block ID
 *      ruleId  - rule specified
 * Output:
 *      pReverse  - 1: reverse, 0: don't reverse
 * Return:
 *      RT_ERR_OK - Success.
 *      RT_ERR_FAILED   - 
 *      RT_ERR_INPUT - Invalid input parameters.
 *      RT_ERR_NULL_POINTER - 
 * Note:
 *      The API can retrieve the reverse status of the specified ACL rule
 */
extern int32 rtk_filter_reverseHit_get(rtk_aclBlock_id_t blockId, rtk_aclRule_id_t ruleId, rtk_enable_t* pReverse);

/* Function Name:
 *      rtk_filter_portEnable_set
 * Description:
 *      Enable/Disable ACL lookup on a per-port basis.
 * Input:
 *      port - port Id
 *      enable - ENABLED or DISABLED
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK - Success.
 *      RT_ERR_FAILED   - 
 *      RT_ERR_PORT_ID - Invalid port ID.
 *      RT_ERR_ENABLE - enable should be ENABLED or DISABLED.
 * Note:
 *      The API enable/disable ACL lookup on a per-port basis.
 */
extern int32 rtk_filter_portEnable_set(rtk_filter_port_t port, rtk_enable_t enable);

/* Function Name:
 *      rtk_filter_portEnable_get
 * Description:
 *      Retrieve ACL lookup ability on a per-port basis.
 * Input:
 *      port - port Id
 * Output:
 *      pEnable - ENABLED or DISABLED
 * Return:
 *      RT_ERR_OK - Success.
 *      RT_ERR_FAILED   - 
 *      RT_ERR_PORT_ID - Invalid port ID.
 *      RT_ERR_NULL_POINTER - NULL pointer input.
 * Note:
 *      The API retrieve ACL lookup ability on a per-port basis..
 */
extern int32 rtk_filter_portEnable_get(rtk_filter_port_t port, rtk_enable_t* pEnable);

/* Function Name:
 *      rtk_filter_l2CrcErrProc_set
 * Description:
 *      Enable/Disable ACL lookup on packets which have Layer2 CRC error.
 * Input:
 *      enable - ENABLED or DISABLED
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK - Success.
 *      RT_ERR_FAILED   - 
 *      RT_ERR_ENABLE - enable should be ENABLED or DISABLED.
 * Note:
 *      The API enable/disable ACL lookup on packets which have Layer2 CRC error.
 *       (1). ENABLED means ACL will process packets with CRC error.
 *       (2). DISABLE means ACL will skip packets which have CRC error.
 */
extern int32 rtk_filter_l2CrcErrProc_set(rtk_enable_t enable);

/* Function Name:
 *      rtk_filter_l2CrcErrProc_set
 * Description:
 *      Enable/Disable ACL lookup on packets which have Layer2 CRC error.
 * Input:
 *      None
 * Output:
 *      pEnable - ENABLED or DISABLED
 * Return:
 *      RT_ERR_OK - Success.
 *      RT_ERR_FAILED   - 
 *      RT_ERR_NULL_POINTER - NULL pointer input.
 * Note:
 *      The API retrieve whether ACL lookup applies on packets which have Layer2 CRC error or not.
 *       (1). ENABLED means ACL will process packets with CRC error.
 *       (2). DISABLE means ACL will skip packets which have CRC error.
 */
extern int32 rtk_filter_l2CrcErrProc_get(rtk_enable_t* pEnable);

/* Function Name:
 *      rtk_filter_cfiSetProc_set
 * Description:
 *      Enable/Disable ACL lookup on packets which are tagged and have CFI bit set.
 * Input:
 *      enable - ENABLED or DISABLED
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK - Success.
 *      RT_ERR_FAILED   - 
 *      RT_ERR_ENABLE - enable should be ENABLED or DISABLED.
 * Note:
 *      The API enable/disable ACL lookup on packets which are tagged and have CFI bit set.
 *       (1). ENABLED means ACL will process packets with CFI set.
 *       (2). DISABLE means ACL will skip packets which have CFI set.
 */
extern int32 rtk_filter_cfiSetProc_set(rtk_enable_t enable);

/* Function Name:
 *      rtk_filter_cfiSetProc_get
 * Description:
 *      Retrieve whether ACL lookup applies on packets which are tagged and have CFI set.
 * Input:
 *      None
 * Output:
 *      pEnable - ENABLED or DISABLED
 * Return:
 *      RT_ERR_OK - Success.
 *      RT_ERR_FAILED   - 
 *      RT_ERR_NULL_POINTER - NULL pointer input.
 * Note:
 *      The API retrieve whether ACL processes packets which are tagged and have CFI bit set or not.
 *       (1). ENABLED means ACL will process packets tagged with CFI set.
 *       (2). DISABLE means ACL will skip packets which are tagged and have CFI set.
 */
extern int32 rtk_filter_cfiSetProc_get(rtk_enable_t* pEnable);


/* Function Name:
 *      rtk_filter_aclEntry_alloc
 * Description:
 *      alloc an ACL entry
 * Input:
 *      None
 * Output:
 *      blockId  - pointer to ACL entry address returned
 * Return:
 *      RT_ERR_OK - Success.
 *      RT_ERR_FAILED   - 
 *      RT_ERR_INPUT - Invalid input parameters.
 *      RT_ERR_NULL_POINTER - 
 * Note:
 *      The API can alloc an ACL entry. 
 */
extern int32 rtk_filter_aclEntry_alloc(rtk_filter_aclEntry_t** ppAclEntry);

/* Function Name:
 *      rtk_filter_aclEntry_init
 * Description:
 *      Initialize ACL entry
 * Input:
 *      pAclEntry - ACL entry specified
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK - Success.
 *      RT_ERR_FAILED   - 
 *      RT_ERR_INPUT - Invalid input parameters.
 *      RT_ERR_NULL_POINTER - 
 * Note:
 *      The API can initialize an ACL entry  
 */
extern int32 rtk_filter_aclEntry_init(rtk_filter_aclEntry_t* pAclEntry);

/* Function Name:
 *      rtk_filter_aclEntry_free
 * Description:
 *      Release resource occupied by ACL entry
 * Input:
 *      pAclEntry - ACL entry specified
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK - Success.
 *      RT_ERR_FAILED   - 
 *      RT_ERR_INPUT - Invalid input parameters.
 *      RT_ERR_NULL_POINTER - 
 * Note:
 *      The API can destroy an ACL entry, and release all resource it occupies.  
 */
extern int32 rtk_filter_aclEntry_free(rtk_filter_aclEntry_t* pAclEntry);


/* Function Name:
 *      rtk_filter_aclField_add
 * Description:
 *       Add field to ACL rule
 * Input:
 *      pAclEntry - ACL rule
 *      type -  field type
 *      fieldData - field data
 *      fieldMask - field Mask
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK - Success.
 *      RT_ERR_FAILED   - 
 *      RT_ERR_INPUT - Invalid input parameters.
 *      RT_ERR_NULL_POINTER -NULL pointer input
 * Note:
 *      The API can add particular field to specified ACL rule.  
 */
extern int32 rtk_filter_aclField_add(rtk_filter_aclEntry_t* pAclEntry, rtk_filter_field_type_t type, rtk_filter_pieTempl_field_t fieldData, rtk_filter_pieTempl_field_t fieldMask);

/* Function Name:
 *      rtk_filter_aclField_del
 * Description:
 *       Delete field from ACL rule
 * Input:
 *      pAclEntry - ACL rule
 *      type -  field type
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK - Success.
 *      RT_ERR_FAILED   - 
 *      RT_ERR_INPUT - Invalid input parameters.
 *      RT_ERR_NULL_POINTER -
 * Note:
 *      The API can remove particular field from specified ACL rule. 
 */
extern int32 rtk_filter_aclField_del(rtk_filter_aclEntry_t* pAclEntry, rtk_filter_field_type_t type);

/* Function Name:
 *      rtk_filter_aclField_getByType
 * Description:
 *       Get specified type field of ACL rule
 * Input:
 *      pAclEntry - ACL rule
 *      type -  field type
 *      pField -  field returned
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK - Success.
 *      RT_ERR_FAILED   - 
 *      RT_ERR_INPUT - Invalid input parameters.
 *      RT_ERR_NULL_POINTER - NULL pointer input
 * Note:
 *      The API can retrieve particular type field from ACL rule.  
 */
extern int32 rtk_filter_aclField_getByType(rtk_filter_aclEntry_t* pAclEntry, rtk_filter_field_type_t type, rtk_filter_aclField_t* pField);

/* Function Name:
 *      rtk_filter_aclField_getFirst
 * Description:
 *      Get first field of ACL rule
 * Input:
 *      pAclEntry - ACL rule
 *      pField -  field returned
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK - Success.
 *      RT_ERR_FAILED   - 
 *      RT_ERR_INPUT - Invalid input parameters.
 *      RT_ERR_NULL_POINTER - 
 * Note:
 *      The API can retrieve first field from ACL rule.    
 */
extern int32 rtk_filter_aclField_getFirst(rtk_filter_aclEntry_t* pAclEntry, rtk_filter_aclField_t* pField);

/* Function Name:
 *      rtk_filter_aclField_getNext
 * Description:
 *      Get next field of ACL rule
 * Input:
 *      pAclEntry - ACL rule
 *      type -  field type
 * Output:
 *      pField - field information returned
 * Return:
 *      RT_ERR_OK - Success.
 *      RT_ERR_FAILED   - 
 *      RT_ERR_INPUT - Invalid input parameters.
 *      RT_ERR_NULL_POINTER - 
 * Note:
 *      The API can retrieve next field which type is larger than that specified in argument type from ACL rule. 
 */
extern int32 rtk_filter_aclField_getNext(rtk_filter_aclEntry_t* pAclEntry, rtk_filter_field_type_t type, rtk_filter_aclField_t* pField);

/* Function Name:
 *      rtk_filter_aclField_replace
 * Description:
 *      Replace specified type field of ACL rule
 * Input:
 *      pAclEntry - ACL rule
 *      pField - new field specified
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK - Success.
 *      RT_ERR_FAILED   - 
 *      RT_ERR_INPUT - Invalid input parameters.
 *      RT_ERR_NULL_POINTER - 
 * Note:
 *      The API can replace particular type field of ACL rule.   
 */
extern int32 rtk_filter_aclField_replace(rtk_filter_aclEntry_t* pAclEntry, rtk_filter_aclField_t* pField);

/* Function Name:
 *      rtk_filter_aclRule_append
 * Description:
 *      Append ACL
 * Input:
 *      pAclEntry - ACL rule
 *      blockId - ACL block ID
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK - Success.
 *      RT_ERR_FAILED   - 
 *      RT_ERR_INPUT - Invalid input parameters.
 *      RT_ERR_NULL_POINTER - 
 *      RT_ERR_FILTER_BLOCKNUM - Invalid block ID
 *      RT_ERR_FILTER_INACL_RULENUM - Insufficient space.
 * Note:
 *      The API can replace particular type field of ACL rule.   
 */
extern int32 rtk_filter_aclRule_append(rtk_filter_aclEntry_t* pAclEntry, rtk_aclBlock_id_t blockId);

/* Function Name:
 *      rtk_filter_aclRule_insert
 * Description:
 *      Insert ACL
 * Input:
 *      pAclEntry - ACL rule
 *      blockId - ACL block ID
 *      ruleId - ACL rule ID
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK - Success.
 *      RT_ERR_FAILED   - 
 *      RT_ERR_INPUT - Invalid input parameters.
 *      RT_ERR_NULL_POINTER - 
 *      RT_ERR_FILTER_BLOCKNUM - Invalid block ID
 *      RT_ERR_FILTER_INACL_RULENUM - No room available for more ACL rules.
 * Note:
 *      The API can insert ACL rule to the position specified in pAclEntry->ruleId.
 */
extern int32 rtk_filter_aclRule_insert(rtk_filter_aclEntry_t* pAclEntry, rtk_aclBlock_id_t blockId, rtk_aclRule_id_t ruleId);

/* Function Name:
 *      rtk_filter_aclRule_insert
 * Description:
 *      Insert ACL
 * Input:
 *      pAclEntry - ACL rule
 *      blockId - ACL block ID
 *      ruleId - ACL rule ID
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK - Success.
 *      RT_ERR_FAILED   - 
 *      RT_ERR_INPUT - Invalid input parameters.
 *      RT_ERR_NULL_POINTER - 
 *      RT_ERR_FILTER_BLOCKNUM - Invalid block ID
 *      RT_ERR_FILTER_INACL_RULENUM - No room available for more ACL rules.
 * Note:
 *      The API can remove ACL rule from ACL list, the rule removed should be released
 *      by calling rtk_filter_aclEntry_free( ) .   
 */
extern int32 rtk_filter_aclRule_remove(rtk_aclBlock_id_t blockId, rtk_aclRule_id_t ruleId, rtk_filter_aclEntry_t** ppAclEntry);


/* Function Name:
 *      rtk_filter_aclRule_get
 * Description:
 *      Get ACL rule
 * Input:
 *      blockId - ACL block ID
 *      ruleId - ACL rule ID
 * Output:
 *      ppAclEntry - Pointer to pointer which refers to the address of ACL rule retrieved.
 * Return:
 *      RT_ERR_OK - Success.
 *      RT_ERR_FAILED   - 
 *      RT_ERR_INPUT - Invalid input parameters.
 *      RT_ERR_NULL_POINTER - 
 *      RT_ERR_FILTER_BLOCKNUM - Invalid block ID
 *      RT_ERR_FILTER_INACL_RULENUM - No room available for more ACL rules.
 * Note:
 *      The API can get ACL rule from ACL list.     
 */
extern int32 rtk_filter_aclRule_get(rtk_aclBlock_id_t blockId, rtk_aclRule_id_t ruleId, rtk_filter_aclEntry_t** ppAclEntry);


/* Function Name:
 *      rtk_filter_aclRule_getFirst
 * Description:
 *      Get ACL first rule
 * Input:
 *      blockId - ACL block ID
 * Output:
 *      ppAclEntry - Pointer to pointer which refers to the address of ACL rule retrieved.
 * Return:
 *      RT_ERR_OK - Success.
 *      RT_ERR_FAILED   - 
 *      RT_ERR_INPUT - Invalid input parameters.
 *      RT_ERR_NULL_POINTER - 
 *      RT_ERR_FILTER_BLOCKNUM - Invalid block ID
 * Note:
 *      The API can get ACL rule from ACL list.     
 */
extern int32 rtk_filter_aclRule_getFirst(rtk_aclBlock_id_t blockId, rtk_filter_aclEntry_t** ppAclEntry);

/* Function Name:
 *      rtk_filter_aclRule_write
 * Description:
 *      Write ACL rule into ASIC
 * Input:
 *      pAclEntry - ACL rule to be written
 *      blockId - ACL block ID
 *      ruleId - ACL rule ID
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK - Success.
 *      RT_ERR_FAILED   - 
 *      RT_ERR_INPUT - Invalid input parameters.
 *      RT_ERR_NULL_POINTER - 
 *      RT_ERR_FILTER_BLOCKNUM - Invalid block ID
 *      RT_ERR_FILTER_ENTRYIDX - Invalid ACL rule index.
 * Note:
 *       The API can write ACL rule into ASIC.       
 */
extern int32 rtk_filter_aclRule_write(rtk_filter_aclEntry_t* pAclEntry, rtk_aclBlock_id_t blockId, rtk_aclRule_id_t ruleId);

/* Function Name:
 *      rtk_filter_aclAction_set
 * Description:
 *      Add specified type of ACL action
 * Input:
 *      pAclEntry - ACL rule to be written
 *      actType - action type
 *      pAction - ACL action
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK - Success.
 *      RT_ERR_FAILED   - 
 *      RT_ERR_INPUT - Invalid input parameters.
 *      RT_ERR_NULL_POINTER - 
 *      RT_ERR_FILTER_ACTION - Invalid action.
 * Note:
 *       The API can add ACL action.      
 */
extern int32 rtk_filter_aclAction_set(rtk_filter_aclEntry_t* pAclEntry, rtk_filter_actionType_t actType, rtk_filter_aclAction_t* pAction);

/* Function Name:
 *      rtk_filter_aclAction_get
 * Description:
 *      Get specified type of ACL action
 * Input:
 *      pAclEntry - ACL rule to be written
 *      actType - action type
 * Output:
 *      pAction - ACL action returned
 * Return:
 *      RT_ERR_OK - Success.
 *      RT_ERR_FAILED   - 
 *      RT_ERR_INPUT - Invalid input parameters.
 *      RT_ERR_NULL_POINTER - 
 *      RT_ERR_FILTER_ACTION - Invalid action.
 * Note:
 *       The API can get ACL action.      
 */
extern int32 rtk_filter_aclAction_get(rtk_filter_aclEntry_t* pAclEntry, rtk_filter_actionType_t actType, rtk_filter_aclAction_t* pAction);

/* Function Name:
 *      rtk_filter_aclAction_del
 * Description:
 *      Delete specified type of ACL action
 * Input:
 *      pAclEntry - ACL rule to be written
 *      actType - action type specified
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK - Success.
 *      RT_ERR_FAILED   - 
 *      RT_ERR_INPUT - Invalid input parameters.
 *      RT_ERR_NULL_POINTER - 
 * Note:
 *       The API can delete ACL action.      
 */
extern int32 rtk_filter_aclAction_del(rtk_filter_aclEntry_t* pAclEntry, rtk_filter_actionType_t actType);

/*RLDP APIs*/

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
extern int32 rtk_rldp_loopPortmask_get(rtk_portmask_t* portMask);

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
extern int32 rtk_rldp_loopedPortPair_get(uint32 port, uint32 *portPair);

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
extern int32 rtk_rldp_txDisPortMsk_set(rtk_portmask_t* portMask);

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
extern int32 rtk_rldp_txDisPortMsk_get(rtk_portmask_t* portMask);

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
extern int32 rtk_rldp_rxDisPortMsk_set(rtk_portmask_t* portMask);

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
extern int32 rtk_rldp_rxDisPortMsk_get(rtk_portmask_t* portMask);

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
extern int32 rtk_rldp_trap_set(rtk_enable_t enable);

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
extern int32 rtk_rldp_trap_get(rtk_enable_t* pEnable);

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
extern int32 rtk_rldp_seed_set(rtk_rtkpp_seed_t* pSeed);

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
extern int32 rtk_rldp_seed_get(rtk_rtkpp_seed_t* pSeed);

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
extern int32 rtk_rldp_randNum_get(rtk_rtkpp_randomNum_t* pRandNum);

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
extern int32 rtk_rldp_chkRetryCnt_set(uint8 nr_retry);

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
extern int32 rtk_rldp_chkRetryCnt_get(uint8* pNr_retry);

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
extern int32 rtk_rldp_chkRetryInterval_set(uint8 interval);

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
extern int32 rtk_rldp_chkRetryInterval_get(uint8* pInterval);

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
extern int32 rtk_rldp_loopRetryCnt_set(uint8 nr_retry);

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
extern int32 rtk_rldp_loopRetryCnt_get(uint8* pNr_retry);

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
extern int32 rtk_rldp_loopRetryInterval_set(uint8 interval);

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
extern int32 rtk_rldp_loopRetryInterval_get(uint8* pInterval);

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
extern int32 rtk_rldp_cpuTrigger(void);

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
extern int32 rtk_rldp_linkupTrig_get(rtk_enable_t* pEnable);

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
extern int32 rtk_rldp_linkupTrig_set(rtk_enable_t enable);

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
extern int32 rtk_rldp_vlanLeaky_get(rtk_enable_t* pEnable);

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
extern int32 rtk_rldp_vlanLeaky_set(rtk_enable_t enable);

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
extern int32 rtk_rldp_autoBlock_get(rtk_enable_t* pEnable);

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
extern int32 rtk_rldp_autoBlock_set(rtk_enable_t enable);


/*UDLD APIs*/

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
extern int32 rtk_udld_enable_set(rtk_portmask_t* loopedPortmask);

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
extern int32 rtk_udld_enable_get(rtk_portmask_t* loopedPortmask);

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
extern int32 rtk_udld_mode_Set(rtk_rtkpp_udldMode_t mode);

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
extern int32 rtk_udld_mode_get(rtk_rtkpp_udldMode_t* pMode);

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
extern int32 rtk_udld_cpuTrigger(void);

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
extern int32 rtk_udld_linkupTrig_set(rtk_enable_t enable);

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
extern int32 rtk_udld_linkupTrig_get(rtk_enable_t* pEnable);

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
extern int32 rtk_udld_trap_set(rtk_enable_t enable);

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
extern int32 rtk_udld_trap_get(rtk_enable_t* pEnable);

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
extern int32 rtk_udld_portmask_get(rtk_portmask_t* pPortmask);

/*RRCP*/

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
extern int32 rtk_rrcp_enable_set(rtk_enable_t enable);

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
extern int32 rtk_rrcp_enable_get(rtk_enable_t* pEnable);

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
extern int32 rtk_rrcp_v1Crc_set(rtk_enable_t enable);

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
extern int32 rtk_rrcp_v1Crc_get(rtk_enable_t* pEnable);

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
extern int32 rtk_rrcp_getPktCopytoCpu_set(rtk_enable_t enable);

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
extern int32 rtk_rrcp_getPktCopytoCpu_get(rtk_enable_t* pEnable);

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
extern int32 rtk_rrcp_setPktCopytoCpu_set(rtk_enable_t enable);

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
extern int32 rtk_rrcp_setPktCopytoCpu_get(rtk_enable_t* pEnable);

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
extern int32 rtk_rrcp_vlanLeaky_set(rtk_enable_t enable);

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
extern int32 rtk_rrcp_vlanLeaky_get(rtk_enable_t* pEnable);

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
extern int32 rtk_rrcp_pbVlan_set(rtk_enable_t enable);

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
extern int32 rtk_rrcp_pbVlan_get(rtk_enable_t* pEnable);

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
extern int32 rtk_rrcp_privateKey_set(uint32 key);

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
extern int32 rtk_rrcp_privateKey_get(uint32* pKey);

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
extern int32 rtk_rrcp_authKey_set(uint16 key);

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
extern int32 rtk_rrcp_authKey_get(uint16* pKey);

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
extern int32 rtk_rrcp_defaultFid_set(uint32 fid);

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
extern int32 rtk_rrcp_defaultFid_get(uint32* pFid);

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
extern int32 rtk_rrcp_customerCode_get(uint32* pCode);

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
extern int32 rtk_rrcp_customerCode_set(uint32 code);

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
extern int32 rtk_rrcp_adminPort_set(rtk_port_t port, rtk_enable_t enable);

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
extern int32 rtk_rrcp_adminPort_get(rtk_port_t port, rtk_enable_t* pEnable);


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
extern int32 rtk_rrcp_authPort_set(rtk_port_t port, rtk_enable_t enable);

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
extern int32 rtk_rrcp_authPort_get(rtk_port_t port, rtk_enable_t* pEnable);

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
extern int32 rtk_rrcp_chipId_set(uint16 chipId);

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
extern int32 rtk_rrcp_chipId_get(uint16* pChipId);

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
extern int32 rtk_rrcp_venderId_set(uint32 venderId);

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
extern int32 rtk_rrcp_venderId_get(uint32* pVenderId);

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
extern int32 rtk_rrcp_switchMac_set(rtk_mac_t* pSwitchOwnMac);

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
extern int32 rtk_rrcp_switchMac_get(rtk_mac_t* pSwitchOwnMac);

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
extern int32 rtk_rrcp_switchMacEn_set(rtk_enable_t enable);

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
extern int32 rtk_rrcp_switchMacEn_get(rtk_enable_t* pEnable);

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
extern int32 rtk_rrcp_switchOwnDmacAct_set(uint8 action);

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
extern int32 rtk_rrcp_switchOwnDmacAct_get(uint8* pAction);

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
extern int32 rtk_rrcp_hiRetryCnt_set(uint8 nr_retry);

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
extern int32 rtk_rrcp_hiRetryCnt_get(uint8* pNr_retry);

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
extern int32 rtk_rrcp_hiRetryInterval_set(uint16 interval);

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
extern int32 rtk_rrcp_hiRetryInterval_get(uint16* pInterval);



#endif /* __RTK_API_EXT_H__ */

