/*
 * Copyright (c) 2021 The Linux Foundation. All rights reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

/* DOC: mbss_utils.h
 * This file provides prototypes of the functions
 * to handle actions across multiple vdevs of the system
 */

#ifndef _MBSS_UTILS_H_
#define _MBSS_UTILS_H_

#include "mbss.h"

/*
 * mbss_acs_in_progress() - check if ACS in progress
 *
 * @vdev: vdev object
 * Return: return true or false
 */
bool mbss_acs_in_progress(struct wlan_objmgr_vdev *vdev);

/* mbss_ht40_in_progress() - check if HT40 in progress
 *
 * @vdev: vdev object
 * Return: return true or false
 */
bool mbss_ht40_in_progress(struct wlan_objmgr_vdev *vdev);

/*
 * mbss_vdev_acs_in_progress() - check if ACS in progress for vdev
 *
 * @vdev: vdev object
 * @acs_source: ACS source
 * Return: return true or false
 */
bool mbss_vdev_acs_in_progress(struct wlan_objmgr_vdev *vdev,
			       enum wlan_mbss_acs_source acs_src);

/*
 * mbss_vdev_ht40_in_progress() - check if ht40 in progress for vdev
 *
 * @vdev: vdev object
 * @ht40_source: ht40 source
 * Return: return true or false
 */
bool mbss_vdev_ht40_in_progress(struct wlan_objmgr_vdev *vdev,
				enum wlan_mbss_ht40_source ht40_src);

/* mbss_num_sta_up() - find number of STA vdevs UP
 *
 * @pdev: pdev object
 * Return: nubner of STA vdevs UP
 */
uint8_t mbss_num_sta_up(struct wlan_objmgr_pdev *pdev);

/* mbss_num_ap_up() - find number of AP vdevs UP
 *
 * @pdev: pdev object
 * Return: number of AP vdevs UP
 */
uint8_t mbss_num_ap_up(struct wlan_objmgr_pdev *pdev);

/* mbss_sta_connecting() - find if STA vdev connecting
 *
 * @vdev: vdev object
 * Return: QDF_STATUS
 */
bool mbss_sta_connecting(struct wlan_objmgr_vdev *vdev);

/* mbss_num_sta_connecting() - find number of STA vdevs connecting
 *
 * @pdev: pdev object
 * Return: number of STA vdevs connecting
 */
uint8_t mbss_num_sta_connecting(struct wlan_objmgr_pdev *pdev);

/* mbss_num_sta() - find number of STA vdevs
 *
 * @pdev: pdev object
 * Return: number of STA vdevs
 */
uint8_t mbss_num_sta(struct wlan_objmgr_pdev *pdev);

/* mbss_num_ap() - find number of AP vdev
 *
 * @pdev: pdev object
 * Return: number of AP vdevs
 */
uint8_t mbss_num_ap(struct wlan_objmgr_pdev *pdev);

/* mbss_num_monitor() - find number of monitor vdev
 *
 * @pdev: pdev object
 * Return: number of AP vdevs
 */
uint8_t mbss_num_monitor(struct wlan_objmgr_pdev *pdev);

/* mbss_num_ap() - find number of AP vdev
 *
 * @pdev: pdev object
 * Return: number of AP vdevs
 */
uint8_t mbss_num_vdev(struct wlan_objmgr_pdev *pdev);

/* mbss_start_vdevs() - start all the vdevs
 *
 * @pdev: pdev object
 * @arg: argument to vdev start/stop function
 * Return: QDF_STATUS
 */
QDF_STATUS
mbss_start_vdevs(struct wlan_objmgr_pdev *pdev, void *arg);

/* mbss_start_ap_vdevs() - start the AP vdevs
 *
 * @pdev: pdev object
 * @arg: argument to vdev start/stop function
 * Return: QDF_STATUS
 */
QDF_STATUS
mbss_start_ap_vdevs(struct wlan_objmgr_pdev *pdev, void *arg);

/* mbss_start_sta_vdevs() - start the STA vdevs
 *
 * @pdev: pdev object
 * @arg: argument to vdev start/stop function
 * Return: QDF_STATUS
 */
QDF_STATUS
mbss_start_sta_vdevs(struct wlan_objmgr_pdev *pdev, void *arg);

/* mbss_stop_vdevs() - stop all the vdevs
 *
 * @pdev: pdev object
 * @arg: argument to vdev start/stop function
 * Return: QDF_STATUS
 */
QDF_STATUS
mbss_stop_vdevs(struct wlan_objmgr_pdev *pdev, void *arg);

/* mbss_stop_ap_vdevs() - stop all the AP vdevs
 *
 * @pdev: pdev object
 * @arg: argument to vdev start/stop function
 * Return: QDF_STATUS
 */
QDF_STATUS
mbss_stop_ap_vdevs(struct wlan_objmgr_pdev *pdev, void *arg);

/* mbss_stop_sta_vdevs() - stop all the STA vdevs
 *
 * @pdev: pdev object
 * @arg: argument to vdev start/stop function
 * Return: QDF_STATUS
 */
QDF_STATUS
mbss_stop_sta_vdevs(struct wlan_objmgr_pdev *pdev, void *arg);

/* mbss_stop_ap_monitor_vdevs() - stop all the AP and monitor vdevs
 *
 * @pdev: pdev object
 * @arg: argument to vdev start/stop function
 * Return: QDF_STATUS
 */
QDF_STATUS
mbss_stop_ap_monitor_vdevs(struct wlan_objmgr_pdev *pdev, void *arg);

/* mbss_start_ap_monitor_vdevs() - start all the AP and monitor vdevs
 *
 * @pdev: pdev object
 * @arg: argument to vdev start/stop function
 * Return: QDF_STATUS
 */
QDF_STATUS
mbss_start_ap_monitor_vdevs(struct wlan_objmgr_pdev *pdev, void *arg);

/* mbss_stop_start_ap_vdevs() - stop and start all the AP vdevs
 *
 * @pdev: pdev object
 * @arg: argument to vdev start/stop function
 * Return: QDF_STATUS
 */
QDF_STATUS
mbss_stop_start_ap_vdevs(struct wlan_objmgr_pdev *pdev, void *arg);

/* mbss_stop_start_ap_monitor_vdevs() - stop and start AP/monitor vdevs
 *
 * @pdev: pdev object
 * @arg: argument to vdev start/stop function
 * Return: QDF_STATUS
 */
QDF_STATUS
mbss_stop_start_ap_monitor_vdevs(struct wlan_objmgr_pdev *pdev,
				 void *arg);

/* mbss_start_restart_ap_monitor_vdevs() - start/restart all the AP vdevs
 *
 * @pdev: pdev object
 * @arg: argument to vdev start/stop function
 * Return: QDF_STATUS
 */
QDF_STATUS
mbss_start_restart_ap_monitor_vdevs(struct wlan_objmgr_pdev *vdev,
				    void *arg);

#endif

