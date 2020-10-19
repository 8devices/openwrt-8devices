/*
 * Copyright (c) 2013-2019 The Linux Foundation. All rights reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for
 * any purpose with or without fee is hereby granted, provided that the
 * above copyright notice and this permission notice appear in all
 * copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL
 * WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE
 * AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL
 * DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR
 * PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */
/**
 * DOC: Implement API's specific to Smart Antenna component.
 */

#ifndef _WMI_UNIFIED_SMART_ANT_API_H_
#define _WMI_UNIFIED_SMART_ANT_API_H_

#include "wmi_unified_smart_ant_param.h"

/**
 *  wmi_unified_smart_ant_enable_cmd_send() - WMI smart ant enable function
 *  @wmi_handle: handle to WMI.
 *  @param: pointer to hold antenna param
 *
 *  @return QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_smart_ant_enable_cmd_send(
		wmi_unified_t wmi_handle,
		struct smart_ant_enable_params *param);

/**
 *  wmi_unified_smart_ant_set_rx_ant_cmd_send() - WMI set rx antenna function
 *  @wmi_handle: handle to WMI.
 *  @param: pointer to hold antenna param
 *
 *  Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_smart_ant_set_rx_ant_cmd_send(
		wmi_unified_t wmi_handle,
		struct smart_ant_rx_ant_params *param);

/**
 *  wmi_unified_smart_ant_set_tx_ant_cmd_send() - WMI set tx antenna function
 *  @wmi_handle: handle to WMI.
 *  @param: pointer to hold antenna param
 *
 *  Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_smart_ant_set_tx_ant_cmd_send(
		wmi_unified_t wmi_handle,
		uint8_t macaddr[QDF_MAC_ADDR_SIZE],
		struct smart_ant_tx_ant_params *param);

/**
 *  wmi_unified_smart_ant_set_training_info_cmd_send() - WMI set tx antenna function
 *  @wmi_handle: handle to WMI.
 *  @param: pointer to hold antenna param
 *
 *  Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_smart_ant_set_training_info_cmd_send(
		wmi_unified_t wmi_handle,
		uint8_t macaddr[QDF_MAC_ADDR_SIZE],
		struct smart_ant_training_info_params *param);

/**
 *  wmi_unified_smart_ant_node_config_cmd_send() - WMI set node config function
 *  @wmi_handle: handle to WMI.
 *  @macaddr: MAC address
 *  @param: pointer to hold node parameter
 *
 *  Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_smart_ant_node_config_cmd_send(
		wmi_unified_t wmi_handle,
		uint8_t macaddr[QDF_MAC_ADDR_SIZE],
		struct smart_ant_node_config_params *param);

/**
 *  wmi_unified_set_ant_switch_tbl_cmd_send() - WMI ant switch tbl cmd function
 *  @wmi_handle: handle to WMI.
 *  @param: pointer to hold ant switch tbl param
 *
 *  Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_set_ant_switch_tbl_cmd_send(
		wmi_unified_t wmi_handle,
		struct ant_switch_tbl_params *param);

#endif /* _WMI_UNIFIED_SMART_ANT_API_H_ */
