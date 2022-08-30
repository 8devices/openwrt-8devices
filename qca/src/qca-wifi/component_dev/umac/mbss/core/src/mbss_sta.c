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

/*
 * DOC: mbss_sta.c
 * This file implements APIs to handle STA vdev events
 * and take actions for inter vdev co-ordination
 */

#include "mbss_sta.h"

QDF_STATUS
mbss_connect_start(struct wlan_objmgr_vdev *vdev,
		   struct if_mgr_event_data *ev_data)
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

	mbss_lock(mbss_ctx);

	mbss_debug_add_if_event_entry(vdev, ev_data);
	bitmap_ptr = mbss_ctx->vdev_bitmaps.connecting_vdevs;
	mbss_set_vdev_bit(wlan_vdev_get_id(vdev), bitmap_ptr);

	mbss_unlock(mbss_ctx);
exit:
	return status;
}

QDF_STATUS
mbss_connect_complete(struct wlan_objmgr_vdev *vdev,
		      struct if_mgr_event_data *ev_data)
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

	mbss_lock(mbss_ctx);

	mbss_debug_add_if_event_entry(vdev, ev_data);
	bitmap_ptr = mbss_ctx->vdev_bitmaps.connecting_vdevs;
	mbss_clear_vdev_bit(wlan_vdev_get_id(vdev), bitmap_ptr);

	mbss_unlock(mbss_ctx);
exit:
	return status;
}

QDF_STATUS
mbss_disconnect_start(struct wlan_objmgr_vdev *vdev,
		      struct if_mgr_event_data *ev_data)
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

	mbss_lock(mbss_ctx);

	mbss_debug_add_if_event_entry(vdev, ev_data);
	bitmap_ptr = mbss_ctx->vdev_bitmaps.disconnecting_vdevs;
	mbss_set_vdev_bit(wlan_vdev_get_id(vdev), bitmap_ptr);

	mbss_unlock(mbss_ctx);
exit:
	return status;
}

QDF_STATUS
mbss_disconnect_complete(struct wlan_objmgr_vdev *vdev,
			 struct if_mgr_event_data *ev_data)
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

	mbss_lock(mbss_ctx);

	mbss_debug_add_if_event_entry(vdev, ev_data);
	bitmap_ptr = mbss_ctx->vdev_bitmaps.disconnecting_vdevs;
	mbss_clear_vdev_bit(wlan_vdev_get_id(vdev), bitmap_ptr);

	mbss_unlock(mbss_ctx);
exit:
	return status;
}

