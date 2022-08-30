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
 * DOC: mbss_events.c
 * This file implements APIs to handle events
 * from the interface manager.
 */

#include "mbss.h"
#include "mbss_sta.h"
#include "mbss_ap.h"
#include "wlan_if_mgr_ap.h"
#include "mbss_scan.h"
#include "mbss_events.h"
#include "wlan_if_mgr_ap.h"

QDF_STATUS mbss_if_mgr_send_event(struct wlan_objmgr_vdev *vdev,
				  void *data)
{
	QDF_STATUS status;
	enum wlan_if_mgr_evt event;
	struct if_mgr_event_data ev_data;

	event = ((struct wlan_mbss_ev_data *)data)->if_mgr_event;
	ev_data.data = data;

	status = if_mgr_deliver_event(vdev, event, &ev_data);
	return status;
}

QDF_STATUS
if_mgr_ap_start_bss(struct wlan_objmgr_vdev *vdev,
		    struct if_mgr_event_data *event_data)
{
	return mbss_ap_start(vdev, event_data);
}

QDF_STATUS
if_mgr_ap_start_bss_complete(struct wlan_objmgr_vdev *vdev,
			     struct if_mgr_event_data *event_data)
{
	return mbss_ap_start_complete(vdev, event_data);
}

QDF_STATUS
if_mgr_ap_stop_bss(struct wlan_objmgr_vdev *vdev,
		   struct if_mgr_event_data *event_data)
{
	return mbss_ap_stop(vdev, event_data);
}

QDF_STATUS
if_mgr_ap_stop_bss_complete(struct wlan_objmgr_vdev *vdev,
			    struct if_mgr_event_data *event_data)
{
	return mbss_ap_stop_complete(vdev, event_data);
}

QDF_STATUS if_mgr_ap_start_acs(struct wlan_objmgr_vdev *vdev,
			       struct if_mgr_event_data *event_data)
{
	return  mbss_ap_start_acs(vdev, event_data);
}

QDF_STATUS if_mgr_ap_stop_acs(struct wlan_objmgr_vdev *vdev,
			      struct if_mgr_event_data *event_data)
{
	return mbss_ap_stop_acs(vdev, event_data);
}

QDF_STATUS if_mgr_ap_done_acs(struct wlan_objmgr_vdev *vdev,
			      struct if_mgr_event_data *event_data)
{
	return mbss_ap_done_acs(vdev, event_data);
}

QDF_STATUS if_mgr_ap_cancel_acs(struct wlan_objmgr_vdev *vdev,
				struct if_mgr_event_data *event_data)
{
	return mbss_ap_cancel_acs(vdev, event_data);
}

QDF_STATUS if_mgr_ap_start_ht40(struct wlan_objmgr_vdev *vdev,
				struct if_mgr_event_data *event_data)
{
	return mbss_ap_start_ht40(vdev, event_data);
}

QDF_STATUS if_mgr_ap_stop_ht40(struct wlan_objmgr_vdev *vdev,
			       struct if_mgr_event_data *event_data)
{
	return mbss_ap_stop_ht40(vdev, event_data);
}

QDF_STATUS if_mgr_ap_done_ht40(struct wlan_objmgr_vdev *vdev,
			       struct if_mgr_event_data *event_data)
{
	return mbss_ap_done_ht40(vdev, event_data);
}

QDF_STATUS if_mgr_ap_cancel_ht40(struct wlan_objmgr_vdev *vdev,
				 struct if_mgr_event_data *event_data)
{
	return mbss_ap_cancel_ht40(vdev, event_data);
}

QDF_STATUS if_mgr_connect_start(struct wlan_objmgr_vdev *vdev,
				struct if_mgr_event_data *event_data)
{
	struct if_mgr_event_data ev_data;
	struct wlan_mbss_ev_data mbss_ev_data;

	mbss_ev_data.if_mgr_event = WLAN_IF_MGR_EV_CONNECT_START;
	ev_data.data = &mbss_ev_data;

	return mbss_connect_start(vdev, &ev_data);
}

QDF_STATUS if_mgr_connect_complete(struct wlan_objmgr_vdev *vdev,
				   struct if_mgr_event_data *event_data)
{
	struct if_mgr_event_data ev_data;
	struct wlan_mbss_ev_data mbss_ev_data;

	mbss_ev_data.if_mgr_event = WLAN_IF_MGR_EV_CONNECT_COMPLETE;
	ev_data.data = &mbss_ev_data;
	return mbss_connect_complete(vdev, &ev_data);
}

QDF_STATUS if_mgr_disconnect_start(struct wlan_objmgr_vdev *vdev,
				   struct if_mgr_event_data *event_data)
{
	struct if_mgr_event_data ev_data;
	struct wlan_mbss_ev_data mbss_ev_data;

	mbss_ev_data.if_mgr_event = WLAN_IF_MGR_EV_DISCONNECT_START;
	ev_data.data = &mbss_ev_data;
	return mbss_disconnect_start(vdev, &ev_data);
}

QDF_STATUS if_mgr_disconnect_complete(struct wlan_objmgr_vdev *vdev,
				      struct if_mgr_event_data *event_data)
{
	struct if_mgr_event_data ev_data;
	struct wlan_mbss_ev_data mbss_ev_data;

	mbss_ev_data.if_mgr_event = WLAN_IF_MGR_EV_DISCONNECT_COMPLETE;
	ev_data.data = &mbss_ev_data;
	return mbss_disconnect_complete(vdev, &ev_data);
}

QDF_STATUS if_mgr_validate_candidate(struct wlan_objmgr_vdev *vdev,
				     struct if_mgr_event_data *event_data)
{
	return QDF_STATUS_SUCCESS;
}

