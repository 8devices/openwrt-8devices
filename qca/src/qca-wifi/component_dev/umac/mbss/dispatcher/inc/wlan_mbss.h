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

/* DOC: wlan_mbss.h
 * This file provides prototypes of the functions
 * exposed to be called from other components
 */

#ifndef _WLAN_MBSS_H_
#define _WLAN_MBSS_H_

#include "wlan_if_mgr_public_struct.h"
#include "qdf_status.h"
#include "wlan_objmgr_vdev_obj.h"
#include "scheduler_api.h"

typedef void (*if_mgr_ev_cb_t)(struct wlan_objmgr_vdev *vdev,
			       void *data);

/**
 * enum wlan_mbss_acs_source: MBSS ACS event sources
 * @MBSS_ACS_SRC_CFG: ACS event from cfg80211
 * @MBSS_ACS_SRC_MLME: ACS event from MLME
 * @MBSS_ACS_SRC_DCS: ACS event from DCS
 * @MBSS_ACS_SRC_SPECTRAL: ACS event from spectral
 * @MBSS_ACS_SRC_BG_SCAN: ACS event from background scan
 * @MBSS_ACS_SRC_MAX: ACS event max source
 */
enum wlan_mbss_acs_source {
	MBSS_ACS_SRC_CFG = 0,
	MBSS_ACS_SRC_MLME = 1,
	MBSS_ACS_SRC_DCS = 2,
	MBSS_ACS_SRC_SPECTRAL = 3,
	MBSS_ACS_SRC_BG_SCAN = 4,
	MBSS_ACS_SRC_MAX = 5,
};

/**
 * enum wlan_mbss_ht40_source: MBSS HT40 event sources
 * @MBSS_HT40_SRC_MLME: HT40 event from MLME
 * @MBSS_HT40_SRC_MAX: HT40 event max source
 */
enum wlan_mbss_ht40_source {
	MBSS_HT40_SRC_MLME = 0,
	MBSS_HT40_SRC_MAX = 1,
};

/**
 * struct wlan_mbss_ev_data: MBSS event data
 * @if_mgr_event: interface manager event
 * @ext_cb: cb for handling the event response
 * @source: source of the event
 * @arg: argument to be passed to the cb
 * @status: status for the event notification
 */
struct wlan_mbss_ev_data {
	enum wlan_if_mgr_evt if_mgr_event;
	if_mgr_ev_cb_t ext_cb;
	union {
		enum wlan_mbss_acs_source acs_source;
		enum wlan_mbss_ht40_source ht40_source;
	} source;
	void *arg;
	QDF_STATUS status;
};

/**
 * struct mbss_ext_cb - Legacy action callbacks
 * @mbss_start_acs: Trigger ext ACS scan
 * @mbss_cancel_acs: Trigger ext ACS cancel
 * @mbss_start_ht40: Notify HT40 intolerance scan
 * @mbss_cancel_ht40: Notify HT40 intolerance scan
 * @mbss_start_ap_vdevs: start AP vdevs
 * @mbss_stop_start_ap_vdevs: stop start AP vdevs
 * @mbss_stop_start_ap_monitor_vdevs: stop start AP and monitor vdevs
 * @mbss_stop_ap_monitor_vdevs: stop AP and monitor vdevs
 * @mbss_stop_sta_vdevs: stop STA vdevs
 * @mbss_start_vdevs: start all vdevs
 * @mbss_start_ap_monitor_vdevs: start AP and monitor vdevs
 * @mbss_stop_vdevs: stop all the vdevs
 * @mbss_start_sta_vdevs: start STA vdevs
 * @mbss_start_restart_ap_monitor_vdevs: restart AP monitor vdevs
 */
struct wlan_mbss_ext_cb {
	QDF_STATUS (*mbss_start_acs)(
		struct wlan_objmgr_vdev *vdev, void *event_data);
	QDF_STATUS (*mbss_cancel_acs)(
		struct wlan_objmgr_vdev *vdev, void *event_data);
	QDF_STATUS (*mbss_start_ht40)(
		struct wlan_objmgr_vdev *vdev, void *event_data);
	QDF_STATUS (*mbss_cancel_ht40)(
		struct wlan_objmgr_vdev *vdev, void *event_data);

	void (*mbss_start_ap_vdevs_cb)(
		struct wlan_objmgr_pdev *pdev, void *object, void *arg);
	void (*mbss_stop_ap_vdevs_cb)(
		struct wlan_objmgr_pdev *pdev, void *object, void *arg);
	void (*mbss_stop_start_ap_vdevs_cb)(
		struct wlan_objmgr_pdev *pdev, void *object, void *arg);
	void (*mbss_stop_start_ap_monitor_vdevs_cb)(
		struct wlan_objmgr_pdev *pdev, void *object, void *arg);
	void (*mbss_stop_ap_monitor_vdevs_cb)(
		struct wlan_objmgr_pdev *pdev, void *object, void *arg);
	void (*mbss_stop_sta_vdevs_cb)(
		struct wlan_objmgr_pdev *pdev, void *object, void *arg);
	void (*mbss_start_vdevs_cb)(
		struct wlan_objmgr_pdev *pdev, void *object, void *arg);
	void (*mbss_start_ap_monitor_vdevs_cb)(
		struct wlan_objmgr_pdev *pdev, void *object, void *arg);
	void (*mbss_stop_vdevs_cb)(
		struct wlan_objmgr_pdev *pdev, void *object, void *arg);
	void (*mbss_start_sta_vdevs_cb)(
		struct wlan_objmgr_pdev *pdev, void *object, void *arg);
	void (*mbss_start_restart_ap_monitor_vdevs_cb)(
		struct wlan_objmgr_pdev *pdev, void *object, void *arg);
};

/**
 * struct mbss_ops - MBSS callbacks
 * @ext_ops: MBSS ext ops
 */
struct wlan_mbss_ops {
	struct wlan_mbss_ext_cb ext_ops;
};

/**
 * enum wlan_mbss_acs_source: MBSS ACS event sources
 * @MBSS_SCHED_PDEV_STOP: schedule AP/monitor VDEVs stop
 * @MBSS_SCHED_PDEV_START: schedule AP/monitor VDEVs stop
 * @MBSS_SCHED_PDEV_STOP_START: schedule AP/monitor VDEVs stop start
 * @MBSS_SCHED_STA_VDEVS_STOP: schedule STA VDEVs stop
 * @MBSS_SCHED_STA_VDEVS_STOP_START: schedule STA VDEVs stop
 */
enum wlan_mbss_sched_actions {
	MBSS_SCHED_PDEV_STOP = 0,
	MBSS_SCHED_PDEV_START = 1,
	MBSS_SCHED_PDEV_STOP_START = 2,
	MBSS_SCHED_STA_VDEVS_STOP = 3,
	MBSS_SCHED_STA_VDEVS_START = 4,
	MBSS_SCHED_MAX = 5,
};

/**
 *  struct mbss_ops - MBSS callbacks
 *  @vdev: vdev
 *  @action: MBSS scheduler action
 */
struct mbss_sched_data {
	struct wlan_objmgr_vdev *vdev;
	enum wlan_mbss_sched_actions action;
};

/* wlan_mbss_alloc_ops() - alloc MBSS ops
 *
 * Allocate memory for MBSS ops
 *
 * Return: QDF_STATUS
 */

QDF_STATUS wlan_mbss_alloc_ops(void);

/* wlan_mbss_free_ops() - free MBSS ops
 *
 * Free memory for MSS ops
 *
 * Return: QDF_STATUS
 */
QDF_STATUS wlan_mbss_free_ops(void);

/* wlan_mbss_get_ops() - get MBSS ops
 *
 * Get the MBSS ops pointer
 *
 * Return: QDF_STATUS
 */
struct wlan_mbss_ops *wlan_mbss_get_ops(void);

/* wlan_mbss_set_ext_ops() - Set MBSS ext ops
 *
 * Set the MBSS ext ops pointer
 *
 * Return: QDF_STATUS
 */
QDF_STATUS wlan_mbss_set_ext_ops(struct wlan_mbss_ext_cb *ext_ops);

/* wlan_mbss_get_ext_ops() - get MBSS ext ops
 *
 * Get the MBSS ext ops pointer
 *
 * Return: MBSS ext ops
 */
struct wlan_mbss_ext_cb *wlan_mbss_get_ext_ops(void);

/* wlan_mbss_init(): Initialize the MBSS framework
 *
 * return: none
 */
QDF_STATUS wlan_mbss_init(void);

/* wlan_mbss_deinit() - Deinitialize the MBSS framework
 *
 * return: none
 */
QDF_STATUS wlan_mbss_deinit(void);

/* wlan_mbss_if_mgr_send_event() - Deliver if manager event to MBSS framework
 *
 * @vdev: vdev object
 * @data: event data
 *
 * return: QDF_STATUS
 */
QDF_STATUS wlan_mbss_if_mgr_send_event(struct wlan_objmgr_vdev *vdev,
				       void *data);

/*
 * wlan_mbss_acs_in_progress() - check if ACS in progress
 *
 * @vdev: vdev object
 * return: return true or false
 */
bool wlan_mbss_acs_in_progress(struct wlan_objmgr_vdev *vdev);

/* wlan_mbss_ht40_in_progress() - check if HT40 in progress
 *
 * @vdev: vdev object
 * return: return true or false
 */
bool wlan_mbss_ht40_in_progress(struct wlan_objmgr_vdev *vdev);

/*
 * wlan_mbss_vdev_acs_in_progress() - check if ACS in progress for a vdev
 *
 * @vdev: vdev object
 * @acs_source: ACS source
 * return: return true or false
 */
bool wlan_mbss_vdev_acs_in_progress(struct wlan_objmgr_vdev *vdev,
				    enum wlan_mbss_acs_source acs_src);

/*
 * wlan_mbss_vdev_ht40_in_progress() - check if ht40 in progress for a vdev
 *
 * @vdev: vdev object
 * @ht40_source: ht40 source
 * return: return true or false
 */
bool wlan_mbss_vdev_ht40_in_progress(struct wlan_objmgr_vdev *vdev,
				     enum wlan_mbss_ht40_source ht40_src);

/* wlan_mbss_num_sta_up() - find number of STA vdevs UP
 *
 * @pdev: pdev object
 * return: nubner of STA vdevs UP
 */
uint8_t wlan_mbss_num_sta_up(struct wlan_objmgr_pdev *pdev);

/* wlan_mbss_num_ap_up() - find number of AP vdevs UP
 *
 * @vdev: vdev object
 * return: number of AP vdevs UP
 */
uint8_t wlan_mbss_num_ap_up(struct wlan_objmgr_pdev *pdev);

/* wlan_mbss_sta_connecting() - find if STA vdev connecting
 *
 * @vdev: vdev object
 * return: QDF_STATUS
 */
bool wlan_mbss_sta_connecting(struct wlan_objmgr_vdev *vdev);

/* wlan_mbss_num_sta_connecting() - find number of STA vdevs connecting
 *
 * @vdev: vdev object
 * return: number of STA vdevs connecting
 */
uint8_t wlan_mbss_num_sta_connecting(struct wlan_objmgr_pdev *pdev);

/* wlan_mbss_num_sta() - find number of STA vdevs
 *
 * @pdev: pdev object
 * return: number of STA vdevs
 */
uint8_t wlan_mbss_num_sta(struct wlan_objmgr_pdev *pdev);

/* wlan_mbss_num_ap() - find number of AP vdev
 *
 * @pdev: pdev object
 * return: number of AP vdevs
 */
uint8_t wlan_mbss_num_ap(struct wlan_objmgr_pdev *pdev);

/* wlan_mbss_num_monitor() - find number of AP vdev
 *
 * @pdev: pdev object
 * return: number of AP vdevs
 */
uint8_t wlan_mbss_num_monitor(struct wlan_objmgr_pdev *pdev);

/* wlan_mbss_num_vdev() - find number of AP vdev
 *
 * @pdev: pdev object
 * return: number of AP vdevs
 */
uint8_t wlan_mbss_num_vdev(struct wlan_objmgr_pdev *pdev);

/* wlan_mbss_start_vdevs() - start all the vdevs
 *
 * @pdev: pdev object
 * @arg: argument to vdev start/stop function
 * return: QDF_STATUS
 */
QDF_STATUS
wlan_mbss_start_vdevs(struct wlan_objmgr_pdev *pdev, void *arg);

/* wlan_mbss_start_ap_vdevs() - start the AP vdevs
 *
 * @pdev: pdev object
 * @arg: argument to vdev start/stop function
 * return: QDF_STATUS
 */
QDF_STATUS
wlan_mbss_start_ap_vdevs(struct wlan_objmgr_pdev *pdev, void *arg);

/* wlan_mbss_start_sta_vdevs() - start the STA vdevs
 *
 * @pdev: pdev object
 * @arg: argument to vdev start/stop function
 * return: QDF_STATUS
 */
QDF_STATUS
wlan_mbss_start_sta_vdevs(struct wlan_objmgr_pdev *pdev, void *arg);

/* wlan_mbss_stop_vdevs() - stop all the vdevs
 *
 * @pdev: pdev object
 * @arg: argument to vdev start/stop function
 * return: QDF_STATUS
 */
QDF_STATUS
wlan_mbss_stop_vdevs(struct wlan_objmgr_pdev *pdev, void *arg);

/* wlan_mbss_stop_ap_vdevs() - stop all the AP vdevs
 *
 * @pdev: pdev object
 * @arg: argument to vdev start/stop function
 * return: QDF_STATUS
 */
QDF_STATUS
wlan_mbss_stop_ap_vdevs(struct wlan_objmgr_pdev *pdev, void *arg);

/* wlan_mbss_stop_sta_vdevs() - stop all the STA vdevs
 *
 * @pdev: pdev object
 * @arg: argument to vdev start/stop function
 * return: QDF_STATUS
 */
QDF_STATUS
wlan_mbss_stop_sta_vdevs(struct wlan_objmgr_pdev *pdev, void *arg);

/* wlan_mbss_stop_ap_monitor_vdevs() - stop all the AP and monitor vdevs
 *
 * @pdev: pdev object
 * @arg: argument to vdev start/stop function
 * return: QDF_STATUS
 */
QDF_STATUS
wlan_mbss_stop_ap_monitor_vdevs(struct wlan_objmgr_pdev *pdev, void *arg);

/* wlan_mbss_start_ap_monitor_vdevs() - start all the AP and monitor vdevs
 *
 * @pdev: pdev object
 * @arg: argument to vdev start/stop function
 * return: QDF_STATUS
 */
QDF_STATUS
wlan_mbss_start_ap_monitor_vdevs(struct wlan_objmgr_pdev *pdev, void *arg);

/* wlan_mbss_stop_start_ap_vdevs() - stop and start all the AP vdevs
 *
 * @pdev: pdev object
 * @arg: argument to vdev start/stop function
 * return: QDF_STATUS
 */
QDF_STATUS
wlan_mbss_stop_start_ap_vdevs(struct wlan_objmgr_pdev *pdev, void *arg);

/* wlan_mbss_stop_start_ap_monitor_vdevs() - stop and start AP/monitor vdevs
 *
 * @pdev: pdev object
 * @arg: argument to vdev start/stop function
 * return: QDF_STATUS
 */
QDF_STATUS
wlan_mbss_stop_start_ap_monitor_vdevs(struct wlan_objmgr_pdev *pdev,
				      void *arg);

/* wlan_mbss_start_restart_ap_monitor_vdevs() - start/restart all the AP vdevs
 *
 * @pdev: pdev object
 * @arg: argument to vdev start/stop function
 * return: QDF_STATUS
 */
QDF_STATUS
wlan_mbss_start_restart_ap_monitor_vdevs(struct wlan_objmgr_pdev *pdev,
					 void *arg);

/* wlan_mbss_sched_action_flush() - flush callback to for scheduler mbss msg
 *
 * @msg: scheduler msg
 * return: QDF_STATUS
 */
QDF_STATUS wlan_mbss_sched_action_flush(struct scheduler_msg *msg);

/* wlan_mbss_sched_action() - callback to handle scheduler mbss msg
 *
 * @msg: scheduler msg
 * return: QDF_STATUS
 */
QDF_STATUS
wlan_mbss_sched_action(struct scheduler_msg *msg);

/* wlan_mbss_sched_start_stop() - handle scheduler mbss start stop
 *
 * @vdev: vdev object
 * @action: mbss action to be scheduled
 * return: QDF_STATUS
 */
QDF_STATUS
wlan_mbss_sched_start_stop(struct wlan_objmgr_vdev *vdev,
			   enum wlan_mbss_sched_actions action);

#if defined WLAN_MBSS_DEBUG
/* wlan_mbss_debug_print_history() - Print MBSS framework history
 *
 * return: none
 */
void wlan_mbss_debug_print_history(struct wlan_objmgr_pdev *pdev);

/* wlan_mbss_debug_print_bitmap() - Print MBSS vdev bitmaps
 *
 * return: none
 */
void wlan_mbss_debug_print_bitmap(struct wlan_objmgr_pdev *pdev);
#else
#define wlan_mbss_debug_print_history
#define wlan_mbss_debug_print_bitmap

#endif
#endif /* _WLAN_MBSS_H_ */
