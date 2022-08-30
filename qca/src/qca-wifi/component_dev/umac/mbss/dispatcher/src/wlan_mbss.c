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
 * DOC: wlan_mbss.c
 * This file implements APIs exposed by the MBSS frmaework
 * to be called from the other components
 */

#include <wlan_mbss.h>
#include <mbss.h>
#include <mbss_events.h>
#include <mbss_utils.h>
#include <wlan_objmgr_global_obj.h>

struct wlan_mbss_ops *g_wlan_mbss_ops;

QDF_STATUS wlan_mbss_alloc_ops(void)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;

	g_wlan_mbss_ops = qdf_mem_malloc(sizeof(struct wlan_mbss_ops));
	if (!g_wlan_mbss_ops)
		status = QDF_STATUS_E_FAILURE;

	return status;
}

QDF_STATUS wlan_mbss_free_ops(void)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;

	qdf_mem_free(g_wlan_mbss_ops);

	return status;
}

struct wlan_mbss_ops *wlan_mbss_get_ops(void)
{
	return g_wlan_mbss_ops;
}

QDF_STATUS wlan_mbss_set_ext_ops(struct wlan_mbss_ext_cb *ext_ops)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	struct wlan_mbss_ops *mbss_ops;

	if (!ext_ops) {
		status = QDF_STATUS_E_FAILURE;
		goto exit;
	}

	mbss_ops = wlan_mbss_get_ops();
	qdf_mem_copy(&mbss_ops->ext_ops, ext_ops,
		     sizeof(struct wlan_mbss_ext_cb));
exit:
	return status;
}

struct wlan_mbss_ext_cb *wlan_mbss_get_ext_ops(void)
{
	struct wlan_mbss_ops *mbss_ops;

	mbss_ops = wlan_mbss_get_ops();
	return &mbss_ops->ext_ops;
}

QDF_STATUS wlan_mbss_init(void)
{
	QDF_STATUS status;

	status  = wlan_mbss_alloc_ops();
	if (QDF_IS_STATUS_ERROR(status)) {
		mbss_err("Failed to alloc mbss ext ops");
		goto exit;
	}

	status = wlan_objmgr_register_pdev_create_handler(
			WLAN_UMAC_COMP_MBSS,
			mbss_pdev_create_handler,
			NULL);

	if (QDF_IS_STATUS_ERROR(status)) {
		mbss_err("Failed to register pdev create handler");
		goto pdev_create_err;
	}

	status = wlan_objmgr_register_pdev_destroy_handler(
			WLAN_UMAC_COMP_MBSS,
			mbss_pdev_destroy_handler,
			NULL);

	if (QDF_IS_STATUS_ERROR(status)) {
		mbss_err("Failed to register pdev delete handler");
		goto pdev_destroy_err;
	}

	status = wlan_objmgr_register_vdev_create_handler(
			WLAN_UMAC_COMP_MBSS,
			mbss_vdev_create_handler,
			NULL);

	if (QDF_IS_STATUS_ERROR(status)) {
		mbss_err("Failed to register vdev create handler");
		goto vdev_create_err;
	}

	status = wlan_objmgr_register_vdev_destroy_handler(
			WLAN_UMAC_COMP_MBSS,
			mbss_vdev_destroy_handler,
			NULL);

	if (QDF_IS_STATUS_ERROR(status)) {
		mbss_err("Failed to register vdev delete handler");
		goto vdev_destroy_err;
	}

	mbss_debug("Registered callback with obj mgr successfully");

	goto exit;

vdev_destroy_err:
	wlan_objmgr_unregister_vdev_create_handler(
		WLAN_UMAC_COMP_MBSS,
		mbss_vdev_create_handler,
		NULL);
vdev_create_err:
	wlan_objmgr_unregister_pdev_destroy_handler(
		WLAN_UMAC_COMP_MBSS,
		mbss_pdev_destroy_handler,
		NULL);
pdev_destroy_err:
	wlan_objmgr_unregister_pdev_create_handler(
		WLAN_UMAC_COMP_MBSS,
		mbss_pdev_create_handler,
		NULL);
pdev_create_err:
	wlan_mbss_free_ops();

exit:
	return status;
}

QDF_STATUS wlan_mbss_deinit(void)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;

	status = wlan_objmgr_unregister_pdev_create_handler(
			WLAN_UMAC_COMP_MBSS,
			mbss_pdev_create_handler,
			NULL);

	if (QDF_IS_STATUS_ERROR(status))
		mbss_err("pdev create handler unregistration failed");

	status = wlan_objmgr_unregister_pdev_destroy_handler(
			WLAN_UMAC_COMP_MBSS,
			mbss_pdev_destroy_handler,
			NULL);

	if (QDF_IS_STATUS_ERROR(status))
		mbss_err("pdev destroy handler unregistration failed");

	status = wlan_objmgr_unregister_vdev_create_handler(
			WLAN_UMAC_COMP_MBSS,
			mbss_vdev_create_handler,
			NULL);

	if (QDF_IS_STATUS_ERROR(status))
		mbss_err("vdev create handler unregistration failed");

	status = wlan_objmgr_unregister_vdev_destroy_handler(
			WLAN_UMAC_COMP_MBSS,
			mbss_vdev_destroy_handler,
			NULL);

	if (QDF_IS_STATUS_ERROR(status))
		mbss_err("vdev destroy handler unregistration failed");

	status = wlan_mbss_free_ops();

	if (QDF_IS_STATUS_ERROR(status))
		mbss_err("Failed to free mbss ext ops");

	return status;
}

QDF_STATUS
wlan_mbss_if_mgr_send_event(struct wlan_objmgr_vdev *vdev,
			    void *data)
{
	return mbss_if_mgr_send_event(vdev, data);
}

qdf_export_symbol(wlan_mbss_if_mgr_send_event);

bool wlan_mbss_acs_in_progress(struct wlan_objmgr_vdev *vdev)
{
	return mbss_acs_in_progress(vdev);
}

qdf_export_symbol(wlan_mbss_acs_in_progress);

bool wlan_mbss_ht40_in_progress(struct wlan_objmgr_vdev *vdev)
{
	return mbss_ht40_in_progress(vdev);
}

qdf_export_symbol(wlan_mbss_ht40_in_progress);

bool wlan_mbss_vdev_acs_in_progress(struct wlan_objmgr_vdev *vdev,
				    enum wlan_mbss_acs_source acs_src)
{
	return mbss_vdev_acs_in_progress(vdev, acs_src);
}

qdf_export_symbol(wlan_mbss_vdev_acs_in_progress);

bool wlan_mbss_vdev_ht40_in_progress(struct wlan_objmgr_vdev *vdev,
				     enum wlan_mbss_ht40_source ht40_src)
{
	return mbss_vdev_ht40_in_progress(vdev, ht40_src);
}

qdf_export_symbol(wlan_mbss_vdev_ht40_in_progress);

bool wlan_mbss_sta_connecting(struct wlan_objmgr_vdev *vdev)
{
	return mbss_sta_connecting(vdev);
}

qdf_export_symbol(wlan_mbss_sta_connecting);

uint8_t wlan_mbss_num_sta_up(struct wlan_objmgr_pdev *pdev)
{
	return mbss_num_sta_up(pdev);
}

qdf_export_symbol(wlan_mbss_num_sta_up);

uint8_t wlan_mbss_num_ap_up(struct wlan_objmgr_pdev *pdev)
{
	return mbss_num_ap_up(pdev);
}

qdf_export_symbol(wlan_mbss_num_ap_up);

uint8_t wlan_mbss_num_sta_connecting(struct wlan_objmgr_pdev *pdev)
{
	return mbss_num_sta_connecting(pdev);
}

qdf_export_symbol(wlan_mbss_num_sta_connecting);

uint8_t wlan_mbss_num_sta(struct wlan_objmgr_pdev *pdev)
{
	return mbss_num_sta(pdev);
}

qdf_export_symbol(wlan_mbss_num_sta);

uint8_t wlan_mbss_num_ap(struct wlan_objmgr_pdev *pdev)
{
	return mbss_num_ap(pdev);
}

qdf_export_symbol(wlan_mbss_num_ap);

uint8_t wlan_mbss_num_monitor(struct wlan_objmgr_pdev *pdev)
{
	return mbss_num_monitor(pdev);
}

qdf_export_symbol(wlan_mbss_num_monitor);

uint8_t wlan_mbss_num_vdev(struct wlan_objmgr_pdev *pdev)
{
	return mbss_num_vdev(pdev);
}

qdf_export_symbol(wlan_mbss_num_vdev);

QDF_STATUS
wlan_mbss_start_vdevs(struct wlan_objmgr_pdev *pdev, void *arg)
{
	return mbss_start_vdevs(pdev, arg);
}

qdf_export_symbol(wlan_mbss_start_vdevs);

QDF_STATUS
wlan_mbss_start_ap_vdevs(struct wlan_objmgr_pdev *pdev, void *arg)
{
	return mbss_start_ap_vdevs(pdev, arg);
}

qdf_export_symbol(wlan_mbss_start_ap_vdevs);

QDF_STATUS
wlan_mbss_start_sta_vdevs(struct wlan_objmgr_pdev *pdev, void *arg)
{
	return mbss_start_sta_vdevs(pdev, arg);
}

qdf_export_symbol(wlan_mbss_start_sta_vdevs);

QDF_STATUS
wlan_mbss_stop_vdevs(struct wlan_objmgr_pdev *pdev, void *arg)
{
	return mbss_stop_vdevs(pdev, arg);
}

qdf_export_symbol(wlan_mbss_stop_vdevs);

QDF_STATUS
wlan_mbss_stop_ap_vdevs(struct wlan_objmgr_pdev *pdev, void *arg)
{
	return mbss_stop_ap_vdevs(pdev, arg);
}

qdf_export_symbol(wlan_mbss_stop_ap_vdevs);

QDF_STATUS
wlan_mbss_stop_sta_vdevs(struct wlan_objmgr_pdev *pdev, void *arg)
{
	return mbss_stop_sta_vdevs(pdev, arg);
}

qdf_export_symbol(wlan_mbss_stop_sta_vdevs);

QDF_STATUS
wlan_mbss_stop_ap_monitor_vdevs(struct wlan_objmgr_pdev *pdev, void *arg)
{
	return mbss_stop_ap_monitor_vdevs(pdev, arg);
}

qdf_export_symbol(wlan_mbss_stop_ap_monitor_vdevs);

QDF_STATUS
wlan_mbss_start_ap_monitor_vdevs(struct wlan_objmgr_pdev *pdev, void *arg)
{
	return mbss_start_ap_monitor_vdevs(pdev, arg);
}

qdf_export_symbol(wlan_mbss_start_ap_monitor_vdevs);

QDF_STATUS
wlan_mbss_stop_start_ap_vdevs(struct wlan_objmgr_pdev *pdev, void *arg)
{
	return mbss_stop_start_ap_vdevs(pdev, arg);
}

qdf_export_symbol(wlan_mbss_stop_start_ap_vdevs);

QDF_STATUS
wlan_mbss_stop_start_ap_monitor_vdevs(struct wlan_objmgr_pdev *pdev,
				      void *arg)
{
	return mbss_stop_start_ap_monitor_vdevs(pdev, arg);
}

qdf_export_symbol(wlan_mbss_stop_start_ap_monitor_vdevs);

QDF_STATUS
wlan_mbss_start_restart_ap_monitor_vdevs(struct wlan_objmgr_pdev *pdev,
					 void *arg)
{
	return mbss_start_restart_ap_monitor_vdevs(pdev, arg);
}

qdf_export_symbol(wlan_mbss_start_restart_ap_monitor_vdevs);

#ifdef WLAN_MBSS_DEBUG
void wlan_mbss_debug_print_history(struct wlan_objmgr_pdev *pdev)
{
	mbss_debug_print_history(pdev);
}

void wlan_mbss_debug_print_bitmap(struct wlan_objmgr_pdev *pdev)
{
	mbss_debug_print_bitmap(pdev);
}
#endif
