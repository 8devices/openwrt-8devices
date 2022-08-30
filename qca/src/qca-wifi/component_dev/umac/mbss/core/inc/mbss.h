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

/**
 * DOC: mbss.h
 * This file provides prototypes of the functions
 * needed by the core MBSS framework.
 */

#ifndef _MBSS_H_
#define _MBSS_H_

#include <wlan_cmn.h>
#include <qdf_lock.h>
#include <qdf_util.h>
#include <qdf_status.h>
#include <qdf_types.h>
#include "wlan_if_mgr_api.h"
#include "wlan_if_mgr_ap.h"
#include "wlan_if_mgr_sta.h"
#include <wlan_objmgr_pdev_obj.h>
#include <wlan_objmgr_vdev_obj.h>
#include "wlan_if_mgr_public_struct.h"
#include "wlan_mbss.h"
#include "mbss_dbg.h"

#define mbss_alert(params...) \
	QDF_TRACE_FATAL(QDF_MODULE_ID_MBSS, params)
#define mbss_err(params...) \
	QDF_TRACE_ERROR(QDF_MODULE_ID_MBSS, params)
#define mbss_warn(params...) \
	QDF_TRACE_WARN(QDF_MODULE_ID_MBSS, params)
#define mbss_info(params...) \
	QDF_TRACE_INFO(QDF_MODULE_ID_MBSS, params)
#define mbss_debug(params...) \
	QDF_TRACE_DEBUG(QDF_MODULE_ID_MBSS, params)

/* Rate Limited Logs */
#define mbss_alert_rl(params...) \
	QDF_TRACE_FATAL_RL(QDF_MODULE_ID_MBSS, params)
#define mbss_err_rl(params...) \
	QDF_TRACE_ERROR_RL(QDF_MODULE_ID_MBSS, params)
#define mbss_warn_rl(params...) \
	QDF_TRACE_WARN_RL(QDF_MODULE_ID_MBSS, params)
#define mbss_info_rl(params...) \
	QDF_TRACE_INFO_RL(QDF_MODULE_ID_MBSS, params)
#define mbss_debug_rl(params...) \
	QDF_TRACE_DEBUG_RL(QDF_MODULE_ID_MBSS, params)

#define mbss_nofl_alert(params...) \
	QDF_TRACE_FATAL_NO_FL(QDF_MODULE_ID_MBSS, params)
#define mbss_nofl_err(params...) \
	QDF_TRACE_ERROR_NO_FL(QDF_MODULE_ID_MBSS, params)
#define mbss_nofl_warn(params...) \
	QDF_TRACE_WARN_NO_FL(QDF_MODULE_ID_MBSS, params)
#define mbss_nofl_info(params...) \
	QDF_TRACE_INFO_NO_FL(QDF_MODULE_ID_MBSS, params)
#define mbss_nofl_debug(params...) \
	QDF_TRACE_DEBUG_NO_FL(QDF_MODULE_ID_MBSS, params)

#define mbss_hex_dump(level, data, buf_len) \
		qdf_trace_hex_dump(QDF_MODULE_ID_MBSS, level, data, buf_len)

#define mbss_hex_dump_debug(data, buf_len) \
		mbss_hex_dump(QDF_TRACE_LEVEL_DEBUG, data, buf_len)

#define MBSS_BITMAP_SIZE WLAN_UMAC_PSOC_MAX_VDEVS
#define mbss_bitmap_type unsigned long

#define mbss_bitmap_offset(bitmap) \
	qdf_offsetof(struct mbss_vdev_bitmaps, bitmap)

#define mbss_vdev_bitmap(variable) qdf_bitmap(variable, MBSS_BITMAP_SIZE)

#define mbss_set_vdev_bitmap(vdev, bitmap) \
	mbss_set_clear_bitmap( \
		vdev, \
		qdf_offsetof(struct mbss_vdev_bitmaps, bitmap), true)

#define mbss_clear_vdev_bitmap(vdev, bitmap) \
	mbss_set_clear_bitmap( \
		vdev, \
		qdf_offsetof(struct mbss_vdev_bitmaps, bitmap), false)

#define mbss_lock(mbss_ctx) qdf_spin_lock_bh(&mbss_ctx->mbss_lock)
#define mbss_unlock(mbss_ctx) qdf_spin_unlock_bh(&mbss_ctx->mbss_lock)

#define mbss_set_vdev_bit(vdev_id, bitmap) qdf_set_bit(vdev_id, bitmap)
#define mbss_clear_vdev_bit(vdev_id, bitmap) qdf_clear_bit(vdev_id, bitmap)
#define mbss_check_vdev_bit(vdev_id, bitmap) qdf_test_bit(vdev_id, bitmap)

#define mbss_bitmap_empty(bitmap) qdf_bitmap_empty(bitmap, MBSS_BITMAP_SIZE)
#define mbss_bitmaps_empty(bitmap, size) qdf_bitmap_empty(bitmap, size)

#define mbss_ffb_set(bitmap) qdf_find_first_bit(bitmap, MBSS_BITMAP_SIZE)

#define mbss_get_pdev_ctx(pdev) \
	wlan_objmgr_pdev_get_comp_private_obj(pdev, WLAN_UMAC_COMP_MBSS)

#if defined WLAN_MBSS_DEBUG
#define mbss_get_dbg_ctx(pdev) \
	mbss_get_pdev_ctx(pdev)->mbss_dbg
#endif

/**
 * struct mbss_vdev_bitmaps- MBSS pdev bitmaps
 * @start_vdevs: Bitmap for vdevs starting
 * @stop_vdevs: Bitmap for vdevs stopping
 * @connecting_vdevs: Bitmap for vdevs connecting
 * @disconnecting_vdevs: Bitmap for vdevs disconnecting
 */
struct mbss_vdev_bitmaps {
	mbss_vdev_bitmap(start_vdevs);
	mbss_vdev_bitmap(stop_vdevs);
	mbss_vdev_bitmap(connecting_vdevs);
	mbss_vdev_bitmap(disconnecting_vdevs);
};

/**
 * struct mbss_acs_data - MBSS ACS data
 * @vdevs_waiting_acs: Bitmap for vdevs waiting for ACS
 * @acs_cb: callback for notifying ACS completion
 */
struct mbss_acs_data {
	mbss_vdev_bitmap(vdevs_waiting_acs);
	if_mgr_ev_cb_t acs_cb;
};

/**
 * struct mbss_acs_ctx - MBSS ACS context
 * @data: ACS data
 * @acs_vdev_id: ACS vdev id
 * @acs_arg: argument for staring ACS
 */
struct mbss_acs_ctx {
	struct mbss_acs_data data[MBSS_ACS_SRC_MAX];
	uint8_t acs_vdev_id;
	void *acs_arg;
};

/**
 * struct mbss_ht40_data - MBSS HT40 data
 * @vdevs_waiting_ht40: Bitmap for vdevs waiting for HT40
 * @ht40_cb: callback for notifying HT40 completion
 */
struct mbss_ht40_data {
	mbss_vdev_bitmap(vdevs_waiting_ht40);
	if_mgr_ev_cb_t ht40_cb;
};

/**
 * struct mbss_ht40_ctx - MBSS HT40 context
 * @data: HT40 data
 * @ht40_vdev_id: HT40 vdev id
 * @acs_arg: argument for staring HT40
 */
struct mbss_ht40_ctx {
	struct mbss_ht40_data data[MBSS_HT40_SRC_MAX];
	uint8_t ht40_vdev_id;
	void *ht40_arg;
};

/**
 * struct mbss_pdev - MBSS pdev context
 * @mbss_lock: lock the protect the MBSS context
 * @vdev_bitmaps: MBSS bitmaps
 * @mbss_acs: MBSS ACS context
 * @mbss_ht40: MBSS HT40 context
 * @num_ap_vdev: number of AP vdevs
 * @num_sta_vdev: number of STA vdevs
 * @num_monitor_vdev: number of monitor vdevs
 * @num_vdev: number of total vdevs
 * @acs_in_progress: indicates ACS in progress
 * @ht40_scan_in_progress: indicated HT40 scan in progres
 * @start_vdevs_in_progress: indicates all vdevs start in progress
 * @stop_vdevs_in_progress: indicates all vdevs stop in progress
 * @start_ap_vdevs_in_progress: inidcates AP vdevs start in progress
 * @stop_ap_vdevs_in_progress: inidcates AP vdevs stop in progress
 * @start_sta_vdevs_in_progress: indicates STA vdevs start in progress
 * @stop_sta_vdevs_in_progress: inidcates STA vdevs down in progress
 * @mbss_dbg: MBSS debug data
 */
struct mbss_pdev {
	qdf_spinlock_t mbss_lock;

	struct mbss_vdev_bitmaps vdev_bitmaps;
	struct mbss_acs_ctx mbss_acs;
	struct mbss_ht40_ctx mbss_ht40;

	uint8_t num_ap_vdev;
	uint8_t num_sta_vdev;
	uint8_t num_monitor_vdev;
	uint8_t num_vdev;

	uint32_t acs_in_progress:1;
	uint32_t ht40_in_progress:1;

	uint32_t start_vdevs_in_progress:1;
	uint32_t stop_vdevs_in_progress:1;

	uint32_t start_ap_vdevs_in_progress:1;
	uint32_t stop_ap_vdevs_in_progress:1;

	uint32_t start_sta_vdevs_in_progress:1;
	uint32_t stop_sta_vdevs_in_progress:1;
#if defined WLAN_MBSS_DEBUG
	struct mbss_dbg_data mbss_dbg;
#endif
};

/* wlan_mbss_pdev_create_handler() - MBSS pdev create handler
 * @pdev: pdev object
 * @arg_list: Argument list
 *
 * Allocate memory for mbss pdev context
 * Initialize all the list with max size
 *
 * Return: QDF_STATUS
 */
QDF_STATUS mbss_pdev_create_handler(struct wlan_objmgr_pdev *pdev,
				    void *arg_list);

/* wlan_mbss_pdev_destroy_handler()- MBSS pdev destroy handler
 * @pdev: pdev object
 * @arg_list: Argument list
 *
 * Ensure all the bitmaps in the MBSS pdev context is cleared
 * Free the memory allocated for the MBSS context
 *
 * Return: QDF_STATUS
 */
QDF_STATUS mbss_pdev_destroy_handler(struct wlan_objmgr_pdev *pdev,
				     void *arg_list);

/* wlan_mbss_vdev_create_handler() - MBSS vdev create handler
 * @vdev: vdev object
 * @arg_list: Argument list
 *
 * Allocate memory for MBSS vdev context
 *
 * Return: QDF_STATUS
 */
QDF_STATUS mbss_vdev_create_handler(struct wlan_objmgr_vdev *vdev,
				    void *arg_list);

/* wlan_mbss_vdev_destroy_handler() - MBSS vdev destroy handler
 * @vdev: vdev object
 * @arg_list: Argument list
 *
 * Ensure vdev is not part of any other mbss pdev context bitmaps.
 *
 * Return: QDF_STATUS
 */
QDF_STATUS mbss_vdev_destroy_handler(struct wlan_objmgr_vdev *vdev,
				     void *arg_list);

/* mbss_check_pdev_all_bitmap() - checks all MBSS pdev bitmaps
 * @pdev: pdev object
 *
 * Return: QDF_STATUS
 */
QDF_STATUS mbss_check_pdev_all_bitmap(struct mbss_pdev *mbss_pdev_ctx);

/* mbss_check_vdev_all_bitmap() - check if vdev is set in MBSS pdev bitmaps
 * @pdev: pdev object
 * @vdev_id: vdev id
 *
 * Return: QDF_STATUS
 */
QDF_STATUS mbss_check_vdev_all_bitmap(struct mbss_pdev *mbss_pdev_ctx,
				      uint8_t vdev_id);

/* mbss_set_clear_bitmap() - MBSS set or clear bit in vdev bitmap
 * @vdev: vdev object
 * @offset: ofset of vdev bitmap
 * @set: flag to indicate set or clear bit
 *
 * Return: QDF_STATUS
 */
QDF_STATUS mbss_set_clear_bitmap(struct wlan_objmgr_vdev *vdev,
				 uint32_t offset, bool set);

/* mbss_get_ctx() - Get MBSS pdev context from VDEV
 * @vdev: vdev object
 *
 * Return: mbss pdev context
 */
struct mbss_pdev *mbss_get_ctx(struct wlan_objmgr_vdev *vdev);

#endif /* _MBSS_H_ */
