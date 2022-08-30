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
 * DOC: mbss_scan.c
 * This file implements APIs to trigger scan request
 * and cancel scan request
 */

#include "mbss_scan.h"

QDF_STATUS mbss_ap_start_acs_ht40(struct wlan_objmgr_vdev *vdev,
				  struct if_mgr_event_data *ev_data,
				   enum wlan_if_mgr_evt event)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;

	struct mbss_pdev *mbss_pdev;
	struct wlan_mbss_ops *mbss_ops;
	struct wlan_mbss_ev_data *mbss_ev;

	struct mbss_acs_ctx *mbss_acs;
	struct mbss_acs_data *acs_data;
	struct mbss_ht40_ctx *mbss_ht40;
	struct mbss_ht40_data *ht40_data;

	bool start_acs = false;
	bool start_ht40 = false;

	mbss_pdev = mbss_get_ctx(vdev);
	if (!mbss_pdev) {
		mbss_err("MBSS ctx is null");
		status = QDF_STATUS_E_FAILURE;
		goto err;
	}

	mbss_ops = wlan_mbss_get_ops();

	mbss_lock(mbss_pdev);
	mbss_ev = (struct wlan_mbss_ev_data *)ev_data->data;
	mbss_debug_add_if_event_entry(vdev, ev_data);

	if (event == WLAN_IF_MGR_EV_AP_START_ACS) {
		if (mbss_ev->source.acs_source >= MBSS_ACS_SRC_MAX) {
			mbss_debug("Invalid Index");
			goto exit;
		}
		mbss_debug("Received ACS start req for vdev: %d",
			   wlan_vdev_get_id(vdev));
		mbss_acs = &mbss_pdev->mbss_acs;
		acs_data = &mbss_acs->data[mbss_ev->source.acs_source];
	} else {
		if (mbss_ev->source.ht40_source >= MBSS_HT40_SRC_MAX) {
			mbss_debug("Invalid Index");
			goto exit;
		}
		mbss_debug("Received HT40 start req for vdev: %d",
			   wlan_vdev_get_id(vdev));
		mbss_ht40 = &mbss_pdev->mbss_ht40;
		ht40_data = &mbss_ht40->data[mbss_ev->source.ht40_source];
	}

	if (!mbss_pdev->acs_in_progress && !mbss_pdev->ht40_in_progress) {
		if (event == WLAN_IF_MGR_EV_AP_START_ACS) {
			start_acs = true;
			qdf_mem_zero(mbss_acs, sizeof(struct mbss_acs_ctx));
		} else if (event == WLAN_IF_MGR_EV_AP_START_HT40) {
			start_ht40 = true;
			qdf_mem_zero(mbss_ht40, sizeof(struct mbss_ht40_ctx));
		}
	}

	if (event == WLAN_IF_MGR_EV_AP_START_ACS) {
		mbss_set_vdev_bit(wlan_vdev_get_id(vdev),
				  acs_data->vdevs_waiting_acs);
		acs_data->acs_cb = mbss_ev->ext_cb;
		mbss_acs->acs_arg = mbss_ev->arg;
		mbss_pdev->acs_in_progress = true;
	} else {
		mbss_set_vdev_bit(wlan_vdev_get_id(vdev),
				  ht40_data->vdevs_waiting_ht40);
		ht40_data->ht40_cb = mbss_ev->ext_cb;
		mbss_ht40->ht40_arg = mbss_ev->arg;
		mbss_pdev->ht40_in_progress = true;
	}

	if (start_acs) {
		mbss_debug("Staring ACS with the ACS module");
		mbss_acs->acs_vdev_id = wlan_vdev_get_id(vdev);
		mbss_unlock(mbss_pdev);
		if (mbss_ops->ext_ops.mbss_start_acs)
			mbss_ops->ext_ops.mbss_start_acs(vdev, mbss_ev->arg);
		mbss_lock(mbss_pdev);
		goto exit;
	}

	if (start_ht40) {
		mbss_debug("Staring HT40 with the ACS module");
		mbss_ht40->ht40_vdev_id = wlan_vdev_get_id(vdev);
		mbss_unlock(mbss_pdev);
		if (mbss_ops->ext_ops.mbss_start_ht40)
			mbss_ops->ext_ops.mbss_start_ht40(vdev, mbss_ev->arg);
		mbss_lock(mbss_pdev);
		goto exit;
	}
exit:
	mbss_unlock(mbss_pdev);
err:
	return status;
}

QDF_STATUS mbss_ap_stop_acs_ht40(struct wlan_objmgr_vdev *vdev,
				 struct if_mgr_event_data *ev_data,
				 enum wlan_if_mgr_evt event)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	uint8_t index, vdev_id, src_vdev_id;
	struct mbss_pdev *mbss_pdev;
	struct wlan_mbss_ops *mbss_ops;

	struct mbss_acs_ctx *mbss_acs;
	struct mbss_acs_data *acs_data;
	mbss_bitmap_type *acs_bitmap;
	bool acs_running = false;

	struct mbss_ht40_ctx *mbss_ht40;
	struct mbss_ht40_data *ht40_data;
	mbss_bitmap_type *ht40_bitmap;
	bool ht40_running = false;

	vdev_id = wlan_vdev_get_id(vdev);

	mbss_pdev = mbss_get_ctx(vdev);
	if (!mbss_pdev) {
		mbss_err("MBSS ctx is null");
		status = QDF_STATUS_E_FAILURE;
		goto exit;
	}

	mbss_ops = wlan_mbss_get_ops();

	mbss_lock(mbss_pdev);

	if (event == WLAN_IF_MGR_EV_AP_STOP_ACS) {
		mbss_acs = &mbss_pdev->mbss_acs;

		/* Clear the bit of the vdev requesting ACS stop */
		for (index = 0; index < MBSS_ACS_SRC_MAX; index++) {
			acs_data = &mbss_acs->data[index];
			acs_bitmap = acs_data->vdevs_waiting_acs;
			if (mbss_check_vdev_bit(vdev_id, acs_bitmap)) {
				acs_running = true;
				mbss_clear_vdev_bit(vdev_id, acs_bitmap);
			}
		}
	} else {
		mbss_ht40 = &mbss_pdev->mbss_ht40;

		/* Clear the bit of the vdev requesting ACS stop */
		for (index = 0; index < MBSS_HT40_SRC_MAX; index++) {
			ht40_data = &mbss_ht40->data[index];
			ht40_bitmap = ht40_data->vdevs_waiting_ht40;
			if (mbss_check_vdev_bit(vdev_id, ht40_bitmap)) {
				ht40_running = true;
				mbss_clear_vdev_bit(vdev_id, ht40_bitmap);
			}
		}
	}

	if (!acs_running && !ht40_running)
		goto done;

	/* If the VDEV requesting for STOP ACS/HT40 is also the source
	 * vdev for ACS/HT40, send ACS/HT40 cancel and on receiving
	 * ACS/HT40 cancellation completion, trigger ACS on other VDEV if any
	 */

	mbss_debug_add_if_event_entry(vdev, ev_data);

	if (acs_running) {
		mbss_debug("ACS Stop received for vdev: %d", vdev_id);
		mbss_debug("ACS vdev-id:%d", mbss_acs->acs_vdev_id);

		src_vdev_id = mbss_acs->acs_vdev_id;
		if (src_vdev_id == vdev_id && mbss_pdev->acs_in_progress) {
			mbss_debug("ACS stop for ACS src vdev:%d", vdev_id);
			mbss_debug("ACS cancel for src vdev:%d", vdev_id);

			mbss_unlock(mbss_pdev);
			if (mbss_ops->ext_ops.mbss_cancel_acs)
				mbss_ops->ext_ops.mbss_cancel_acs(vdev, NULL);
			mbss_lock(mbss_pdev);

			goto done;
		}
	}
	if (ht40_running) {
		mbss_debug("HT40 Stop received for vdev: %d", vdev_id);
		mbss_debug("HT40 vdev-id:%d", mbss_ht40->ht40_vdev_id);

		src_vdev_id = mbss_ht40->ht40_vdev_id;
		if (src_vdev_id == vdev_id && mbss_pdev->ht40_in_progress) {
			mbss_debug("HT40 stop for ACS src vdev:%d", vdev_id);
			mbss_debug("HT40 cancel for src vdev:%d", vdev_id);

			mbss_unlock(mbss_pdev);
			if (mbss_ops->ext_ops.mbss_cancel_ht40)
				mbss_ops->ext_ops.mbss_cancel_ht40(vdev, NULL);
			mbss_lock(mbss_pdev);

			goto done;
		}
	}

done:
	mbss_unlock(mbss_pdev);
exit:
	return status;
}

QDF_STATUS mbss_ap_done_acs_ht40(struct wlan_objmgr_vdev *vdev,
				 struct if_mgr_event_data *ev_data,
				 enum wlan_if_mgr_evt event)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	struct mbss_pdev *mbss_pdev;
	struct wlan_objmgr_pdev *pdev;
	struct wlan_objmgr_vdev *iter_vdev;
	uint8_t index, vdev_id = 0;
	struct wlan_mbss_ev_data *mbss_ev;

	struct mbss_acs_ctx *mbss_acs;
	struct mbss_acs_data *acs_data;
	mbss_bitmap_type *acs_bitmap;

	struct mbss_ht40_ctx *mbss_ht40;
	struct mbss_ht40_data *ht40_data;
	mbss_bitmap_type *ht40_bitmap;

	pdev = wlan_vdev_get_pdev(vdev);

	mbss_pdev = mbss_get_ctx(vdev);
	if (!mbss_pdev) {
		mbss_err("MBSS ctx is null");
		status = QDF_STATUS_E_FAILURE;
		goto exit;
	}

	mbss_lock(mbss_pdev);

	mbss_debug_add_if_event_entry(vdev, ev_data);
	mbss_ev = ev_data->data;

	/*Call ACS handler for each VDEV*/
	for (index = 0; index < MBSS_ACS_SRC_MAX; index++) {
		mbss_acs = &mbss_pdev->mbss_acs;
		acs_data = &mbss_acs->data[index];

		acs_bitmap = acs_data->vdevs_waiting_acs;
		while (!mbss_bitmap_empty(acs_bitmap)) {
			vdev_id = mbss_ffb_set(acs_bitmap);
			mbss_debug("ACS Completed for vdev: %d", vdev_id);
			iter_vdev = wlan_objmgr_get_vdev_by_id_from_pdev(
					pdev, vdev_id, WLAN_MBSS_ID);
			mbss_unlock(mbss_pdev);
			if (acs_data->acs_cb)
				acs_data->acs_cb(iter_vdev, mbss_ev->arg);
			mbss_lock(mbss_pdev);
			mbss_clear_vdev_bit(vdev_id, acs_bitmap);
			wlan_objmgr_vdev_release_ref(iter_vdev, WLAN_MBSS_ID);
		}
	}

	/*Call HT40 handler for each VDEV*/
	for (index = 0; index < MBSS_HT40_SRC_MAX; index++) {
		mbss_ht40 = &mbss_pdev->mbss_ht40;
		ht40_data = &mbss_ht40->data[index];

		ht40_bitmap = ht40_data->vdevs_waiting_ht40;
		while (!mbss_bitmap_empty(ht40_bitmap)) {
			vdev_id = mbss_ffb_set(ht40_bitmap);
			mbss_debug("HT40 Completed for vdev: %d", vdev_id);
			iter_vdev = wlan_objmgr_get_vdev_by_id_from_pdev(
					pdev, vdev_id, WLAN_MBSS_ID);
			mbss_unlock(mbss_pdev);
			if (ht40_data->ht40_cb)
				ht40_data->ht40_cb(iter_vdev, mbss_ev->arg);
			mbss_lock(mbss_pdev);
			mbss_clear_vdev_bit(vdev_id, ht40_bitmap);
			wlan_objmgr_vdev_release_ref(iter_vdev, WLAN_MBSS_ID);
		}
	}

	qdf_mem_zero(mbss_acs, sizeof(struct mbss_acs_ctx));
	mbss_pdev->acs_in_progress = false;

	qdf_mem_zero(mbss_ht40, sizeof(struct mbss_ht40_ctx));
	mbss_pdev->ht40_in_progress = false;

	mbss_unlock(mbss_pdev);
exit:
	return status;
}

QDF_STATUS mbss_ap_cancel_acs_ht40(struct wlan_objmgr_vdev *vdev,
				   struct if_mgr_event_data *ev_data,
				   enum wlan_if_mgr_evt event)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	struct mbss_pdev *mbss_pdev;
	struct wlan_objmgr_pdev *pdev;
	struct wlan_mbss_ops *mbss_ops;
	struct wlan_objmgr_vdev *new_src_vdev;
	uint8_t index, src_vdev_id;

	struct mbss_acs_ctx *mbss_acs;
	struct mbss_acs_data *acs_data;
	uint8_t acs_vdev_id;
	mbss_bitmap_type *acs_bitmap;
	bool acs_vdev_identified = false;

	struct mbss_ht40_ctx *mbss_ht40;
	struct mbss_ht40_data *ht40_data;
	uint8_t ht40_vdev_id = WLAN_UMAC_PDEV_MAX_VDEVS;
	mbss_bitmap_type *ht40_bitmap;
	bool ht40_vdev_identified = false;

	pdev = wlan_vdev_get_pdev(vdev);
	mbss_pdev = mbss_get_ctx(vdev);

	if (!mbss_pdev) {
		mbss_err("MBSS ctx is null");
		status = QDF_STATUS_E_FAILURE;
		goto exit;
	}

	mbss_ops = wlan_mbss_get_ops();

	mbss_lock(mbss_pdev);

	mbss_debug_add_if_event_entry(vdev, ev_data);
	mbss_acs = &mbss_pdev->mbss_acs;

	mbss_debug("ACS Cancel Completed for vdev: %d", wlan_vdev_get_id(vdev));

	/* Check if there is any other vdev waiting for ACS */
	for (index = 0; index < MBSS_ACS_SRC_MAX; index++) {
		mbss_acs = &mbss_pdev->mbss_acs;
		acs_data = &mbss_acs->data[index];
		acs_bitmap = acs_data->vdevs_waiting_acs;
		while (!mbss_bitmap_empty(acs_bitmap)) {
			acs_vdev_id = mbss_ffb_set(acs_bitmap);
			acs_vdev_identified = true;
			mbss_debug("Found vdev:%d waiting for ACS",
				   acs_vdev_id);
			break;
		}
		if (acs_vdev_identified)
			break;
	}

	/*So the cancel was for the last vdev waiting ACS, reset ACS in progress flag*/
	if (!acs_vdev_identified)
		mbss_pdev->acs_in_progress = false;

	for (index = 0; index < MBSS_HT40_SRC_MAX; index++) {
		mbss_ht40 = &mbss_pdev->mbss_ht40;
		ht40_data = &mbss_ht40->data[index];
		ht40_bitmap = ht40_data->vdevs_waiting_ht40;
		while (!mbss_bitmap_empty(ht40_bitmap)) {
			ht40_vdev_id = mbss_ffb_set(ht40_bitmap);
			ht40_vdev_identified = true;
			mbss_debug("Found vdev:%d waiting for HT40",
				   ht40_vdev_id);
			break;
		}
		if (ht40_vdev_identified)
			break;
	}

	/*So the cancel was for the last vdev waiting HT40, reset HT40 in progress flag*/
	if (!ht40_vdev_identified)
		mbss_pdev->ht40_in_progress = false;

	/* There is no vdev waiting for either ACS or HT40 return,
	 * but if there's a vdev waiting for either ACS/HT40 completion,
	 * trigger the same again for the waiting vdevs
	 */
	if (!acs_vdev_identified && !ht40_vdev_identified)
		goto done;

	/* Found vdev waiting for ACS/HT40,
	 * triggering ACS/HT40 on this new vdev, provided
	 * there was a vdev waiting for the ACS or HT40
	 */
	if (event == WLAN_IF_MGR_EV_AP_CANCEL_ACS) {
		/*
		 * Before staring ACS, make sure there is a VDEV waiting
		 * for ACS , else start on the HT40 waiting vdevs
		 */
		if (acs_vdev_identified)
			src_vdev_id = acs_vdev_id;
		else
			src_vdev_id = ht40_vdev_id;

		new_src_vdev = wlan_objmgr_get_vdev_by_id_from_pdev(
				pdev, src_vdev_id, WLAN_MBSS_ID);
		mbss_debug("Trigger ACS handoff for %d", src_vdev_id);
		if (mbss_ops->ext_ops.mbss_start_acs) {
			mbss_unlock(mbss_pdev);
			mbss_ops->ext_ops.mbss_start_acs(new_src_vdev,
							 NULL);
			mbss_lock(mbss_pdev);
		}
		wlan_objmgr_vdev_release_ref(new_src_vdev, WLAN_MBSS_ID);
		goto done;
	}
	if (event == WLAN_IF_MGR_EV_AP_CANCEL_HT40) {
		/*
		 * Before staring HT40, make sure there is a VDEV waiting
		 * for Ht40, else start on the ACS waiting vdevs
		 */
		if (ht40_vdev_identified)
			src_vdev_id = ht40_vdev_id;
		else
			src_vdev_id = acs_vdev_id;
		new_src_vdev = wlan_objmgr_get_vdev_by_id_from_pdev(
				pdev, ht40_vdev_id, WLAN_MBSS_ID);
		if (!new_src_vdev)
			goto done;
		mbss_debug("Trigger HT40 handoff for %d", src_vdev_id);
		if (mbss_ops->ext_ops.mbss_start_ht40) {
			mbss_unlock(mbss_pdev);
			mbss_ops->ext_ops.mbss_start_ht40(new_src_vdev, NULL);
			mbss_lock(mbss_pdev);
		}
		wlan_objmgr_vdev_release_ref(new_src_vdev, WLAN_MBSS_ID);
		goto done;
	}

done:
	mbss_unlock(mbss_pdev);
exit:
	return status;
}

QDF_STATUS
mbss_ap_start_acs(struct wlan_objmgr_vdev *vdev,
		  struct if_mgr_event_data *ev_data)
{
	return mbss_ap_start_acs_ht40(vdev, ev_data,
				      WLAN_IF_MGR_EV_AP_START_ACS);
}

QDF_STATUS
mbss_ap_stop_acs(struct wlan_objmgr_vdev *vdev,
		 struct if_mgr_event_data *ev_data)
{
	return mbss_ap_stop_acs_ht40(vdev, ev_data,
				     WLAN_IF_MGR_EV_AP_STOP_ACS);
}

QDF_STATUS
mbss_ap_done_acs(struct wlan_objmgr_vdev *vdev,
		 struct if_mgr_event_data *ev_data)
{
	return mbss_ap_done_acs_ht40(vdev, ev_data,
				     WLAN_IF_MGR_EV_AP_DONE_ACS);
}

QDF_STATUS
mbss_ap_cancel_acs(struct wlan_objmgr_vdev *vdev,
		   struct if_mgr_event_data *ev_data)
{
	return mbss_ap_cancel_acs_ht40(vdev, ev_data,
				       WLAN_IF_MGR_EV_AP_CANCEL_ACS);
}

QDF_STATUS
mbss_ap_start_ht40(struct wlan_objmgr_vdev *vdev,
		   struct if_mgr_event_data *ev_data)
{
	return mbss_ap_start_acs_ht40(vdev, ev_data,
				      WLAN_IF_MGR_EV_AP_START_HT40);
}

QDF_STATUS
mbss_ap_stop_ht40(struct wlan_objmgr_vdev *vdev,
		  struct if_mgr_event_data *ev_data)
{
	return mbss_ap_stop_acs_ht40(vdev, ev_data,
				     WLAN_IF_MGR_EV_AP_STOP_HT40);
}

QDF_STATUS
mbss_ap_done_ht40(struct wlan_objmgr_vdev *vdev,
		  struct if_mgr_event_data *ev_data)
{
	return mbss_ap_done_acs_ht40(vdev, ev_data,
				     WLAN_IF_MGR_EV_AP_DONE_HT40);
}

QDF_STATUS
mbss_ap_cancel_ht40(struct wlan_objmgr_vdev *vdev,
		    struct if_mgr_event_data *ev_data)
{
	return mbss_ap_cancel_acs_ht40(vdev, ev_data,
				       WLAN_IF_MGR_EV_AP_CANCEL_HT40);
}

