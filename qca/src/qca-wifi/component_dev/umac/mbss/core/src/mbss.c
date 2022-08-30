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
 * DOC: mbss.c
 * This file implements API used by the core MBSS frmaework
 */

#include <mbss_dbg.h>
#include <mbss.h>

QDF_STATUS mbss_check_pdev_all_bitmap(struct mbss_pdev *mbss_ctx)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	mbss_bitmap_type *vdev_bitmaps;

	vdev_bitmaps = (mbss_bitmap_type *)&mbss_ctx->vdev_bitmaps;

	if (!mbss_bitmaps_empty(vdev_bitmaps,
				sizeof(mbss_ctx->vdev_bitmaps))) {
		mbss_err("MBSS pdev has still bit(s) set");
		status = QDF_STATUS_E_FAILURE;
		mbss_hex_dump_debug(vdev_bitmaps,
				    sizeof(mbss_ctx->vdev_bitmaps));
	}
	return status;
}

QDF_STATUS mbss_check_vdev_all_bitmap(struct mbss_pdev *mbss_ctx,
				      uint8_t vdev_id)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	uint32_t idx, mbss_num_vdev_bmap;
	uint32_t mbss_all_bmap_size, mbss_bmap_size;
	mbss_bitmap_type *mbss_vdev_bmaps;
	mbss_bitmap_type *vdev_bmap_ptr;
	uint8_t *offset;

	mbss_vdev_bmaps = (mbss_bitmap_type *)&mbss_ctx->vdev_bitmaps;

	mbss_all_bmap_size = sizeof(mbss_ctx->vdev_bitmaps);
	mbss_bmap_size = sizeof(mbss_ctx->vdev_bitmaps.start_vdevs);
	mbss_num_vdev_bmap = mbss_all_bmap_size / mbss_bmap_size;

	for (idx = 0; idx < mbss_num_vdev_bmap; idx++) {
		offset = ((uint8_t *)mbss_vdev_bmaps + (mbss_bmap_size * idx));
		vdev_bmap_ptr = (mbss_bitmap_type *)(offset);
		if (mbss_check_vdev_bit(vdev_id, vdev_bmap_ptr)) {
			mbss_err("vdev bit %d set for index %d", vdev_id, idx);
			status = QDF_STATUS_E_FAILURE;
			mbss_hex_dump_debug(mbss_vdev_bmaps,
					    mbss_all_bmap_size);
		}
	}
	return status;
}

QDF_STATUS mbss_pdev_create_handler(struct wlan_objmgr_pdev *pdev,
				    void *arg_list)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	struct mbss_pdev *mbss_pdev_ctx;

	mbss_pdev_ctx = qdf_mem_malloc(sizeof(struct mbss_pdev));
	if (!mbss_pdev_ctx) {
		mbss_err("Failed to allocate MBSS context");
		status = QDF_STATUS_E_FAILURE;
		goto exit;
	}

	qdf_spinlock_create(&mbss_pdev_ctx->mbss_lock);

	status = wlan_objmgr_pdev_component_obj_attach(
			pdev, WLAN_UMAC_COMP_MBSS,
			mbss_pdev_ctx, QDF_STATUS_SUCCESS);

	if (QDF_IS_STATUS_ERROR(status)) {
		mbss_err("PDEV attach failed");
		goto error;
	}

	mbss_debug("Attached MBSS pdev-%d comp",
		   wlan_objmgr_pdev_get_pdev_id(pdev));
	goto exit;

error:
	qdf_spinlock_destroy(&mbss_pdev_ctx->mbss_lock);
	qdf_mem_free(mbss_pdev_ctx);
exit:
	return status;
}

QDF_STATUS mbss_pdev_destroy_handler(struct wlan_objmgr_pdev *pdev,
				     void *arg_list)
{
	QDF_STATUS status;
	struct mbss_pdev *mbss_pdev_ctx;

	mbss_pdev_ctx = mbss_get_pdev_ctx(pdev);
	if (!mbss_pdev_ctx) {
		mbss_err("Invalid ctx");
		status = QDF_STATUS_E_NULL_VALUE;
		goto error;
	}

	status = mbss_check_pdev_all_bitmap(mbss_pdev_ctx);
	if (QDF_IS_STATUS_ERROR(status)) {
		mbss_debug_print_history(pdev);
		QDF_ASSERT(0);
	}

	status = wlan_objmgr_pdev_component_obj_detach(
			pdev, WLAN_UMAC_COMP_MBSS, mbss_pdev_ctx);

	if (QDF_IS_STATUS_ERROR(status))
		mbss_err("PDEV detatch failed");

	qdf_spinlock_destroy(&mbss_pdev_ctx->mbss_lock);
	qdf_mem_free(mbss_pdev_ctx);

	mbss_debug("Detached MBSS pdev comp obj");

error:
	return status;
}

QDF_STATUS mbss_vdev_create_handler(struct wlan_objmgr_vdev *vdev,
				    void *arg_list)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	struct mbss_pdev *mbss_pdev_ctx;
	enum QDF_OPMODE opmode;

	mbss_pdev_ctx = mbss_get_ctx(vdev);
	if (!mbss_pdev_ctx) {
		mbss_err("MBSS ctx is null");
		status = QDF_STATUS_E_FAILURE;
		goto exit;
	}

	opmode = wlan_vdev_mlme_get_opmode(vdev);

	mbss_lock(mbss_pdev_ctx);

	mbss_pdev_ctx->num_vdev++;
	switch (opmode) {
	case QDF_SAP_MODE:
		mbss_pdev_ctx->num_ap_vdev++;
		break;
	case QDF_STA_MODE:
		mbss_pdev_ctx->num_sta_vdev++;
		break;
	case QDF_MONITOR_MODE:
		mbss_pdev_ctx->num_monitor_vdev++;
		break;
	default:
		mbss_err("Mode not accounted");
		break;
	}

	mbss_unlock(mbss_pdev_ctx);

	mbss_debug("Attached MBSS vdev:%d comp", wlan_vdev_get_id(vdev));
exit:
	return status;
}

QDF_STATUS mbss_vdev_destroy_handler(struct wlan_objmgr_vdev *vdev,
				     void *arg_list)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	struct mbss_pdev *mbss_pdev_ctx;
	enum QDF_OPMODE opmode;

	mbss_pdev_ctx = mbss_get_ctx(vdev);
	if (!mbss_pdev_ctx) {
		mbss_err("MBSS ctx is null");
		status = QDF_STATUS_E_FAILURE;
		goto exit;
	}

	opmode = wlan_vdev_mlme_get_opmode(vdev);

	mbss_lock(mbss_pdev_ctx);

	mbss_pdev_ctx->num_vdev--;
	switch (opmode) {
	case QDF_SAP_MODE:
		mbss_pdev_ctx->num_ap_vdev--;
		break;
	case QDF_STA_MODE:
		mbss_pdev_ctx->num_sta_vdev--;
		break;
	case QDF_MONITOR_MODE:
		mbss_pdev_ctx->num_monitor_vdev--;
		break;
	default:
		mbss_err("Mode not accounted");
		break;
	}

	status = mbss_check_vdev_all_bitmap(mbss_pdev_ctx,
					    wlan_vdev_get_id(vdev));
	if (QDF_IS_STATUS_ERROR(status)) {
		mbss_debug_print_history(wlan_vdev_get_pdev(vdev));
		QDF_ASSERT(0);
	}

	mbss_unlock(mbss_pdev_ctx);
exit:
	mbss_debug("Detached MBSS vdev:%d comp", wlan_vdev_get_id(vdev));
	return status;
}

QDF_STATUS mbss_set_clear_bitmap(struct wlan_objmgr_vdev *vdev,
				 uint32_t offset, bool set)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	struct mbss_pdev *mbss_ctx;
	mbss_bitmap_type *bitmap_ptr;

	mbss_ctx = mbss_get_ctx(vdev);
	if (!mbss_ctx) {
		mbss_err("MBSS_ctx is null");
		status =  QDF_STATUS_E_FAILURE;
		goto exit;
	}
	bitmap_ptr = ((void *)(&mbss_ctx->vdev_bitmaps) + offset);

	if (set)
		mbss_set_vdev_bit(wlan_vdev_get_id(vdev), bitmap_ptr);
	else
		mbss_clear_vdev_bit(wlan_vdev_get_id(vdev), bitmap_ptr);
exit:
	return status;
}

struct mbss_pdev *mbss_get_ctx(struct wlan_objmgr_vdev *vdev)
{
	struct mbss_pdev *mbss_ctx = NULL;
	struct wlan_objmgr_pdev *pdev;

	if (!vdev) {
		mbss_err("vdev is NULL");
		goto exit;
	}

	pdev = wlan_vdev_get_pdev(vdev);
	if (!pdev) {
		mbss_err("pdev is NULL");
		goto exit;
	}

	mbss_ctx = mbss_get_pdev_ctx(pdev);
exit:
	return mbss_ctx;
}
