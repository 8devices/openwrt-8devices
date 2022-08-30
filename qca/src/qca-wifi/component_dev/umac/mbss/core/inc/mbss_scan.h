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

/* DOC: mbss_scan.h
 * This file provides prototypes of the functions
 * to trigger scan request and cancel scan request
 */

#ifndef _MBSS_SCAN_H_
#define _MBSS_SCAN_H_

#include "mbss.h"

/* mbss_start_acs- Handle AP start ACS event
 * @vdev : vdev object
 * @event_data: event data
 *
 * Return: QDF_STATUS
 */
QDF_STATUS mbss_ap_start_acs(struct wlan_objmgr_vdev *vdev,
			     struct if_mgr_event_data *event_data);

/* mbss_ap_stop_acs - Handle AP stop ACS event
 * @vdev : vdev object
 * @event_data: event data
 *
 * Return: QDF_STATUS
 */
QDF_STATUS mbss_ap_stop_acs(struct wlan_objmgr_vdev *vdev,
			    struct if_mgr_event_data *event_data);

/* mbss_ap_done_acs - Handle AP ACS done event
 * @vdev : vdev object
 * @event_data: event data
 *
 * Return: QDF_STATUS
 */
QDF_STATUS mbss_ap_done_acs(struct wlan_objmgr_vdev *vdev,
			    struct if_mgr_event_data *event_data);

/* mbss_ap_cancel_acs - Handle AP ACS cancel event
 * @vdev : vdev object
 * @event_data: event data
 *
 * Return: QDF_STATUS
 */
QDF_STATUS mbss_ap_cancel_acs(struct wlan_objmgr_vdev *vdev,
			      struct if_mgr_event_data *event_data);

/* mbss_ap_start_ht40 - Handle start HT40 event
 * @vdev : vdev object
 * @event_data: event data
 *
 * Return: QDF_STATUS
 */
QDF_STATUS mbss_ap_start_ht40(struct wlan_objmgr_vdev *vdev,
			      struct if_mgr_event_data *event_data);

/* mbss_ap_stop_ht40- Handle AP stop HT40 event
 * @vdev : vdev object
 * @event_data: event data
 *
 * Return: QDF_STATUS
 */
QDF_STATUS mbss_ap_stop_ht40(struct wlan_objmgr_vdev *vdev,
			     struct if_mgr_event_data *event_data);

/* mbss_ap_done_ht40- Handle AP HT40 done event
 * @vdev : vdev object
 * @event_data: event data
 *
 * Return: QDF_STATUS
 */
QDF_STATUS mbss_ap_done_ht40(struct wlan_objmgr_vdev *vdev,
			     struct if_mgr_event_data *event_data);

/* mbss_ap_cancel_ht40- Handle AP HT40 cancel event
 * @vdev : vdev object
 * @event_data: event data
 *
 * Return: QDF_STATUS
 */
QDF_STATUS mbss_ap_cancel_ht40(struct wlan_objmgr_vdev *vdev,
			       struct if_mgr_event_data *event_data);

/* mbss_ap_start_acs_ht40 - Handle AP start ACS/HT40 event
 * @vdev : vdev object
 * @event_data: event data
 * @event: if mgr event
 *
 * Return: QDF_STATUS
 */
QDF_STATUS mbss_ap_start_acs_ht40(struct wlan_objmgr_vdev *vdev,
				  struct if_mgr_event_data *event_data,
				  enum wlan_if_mgr_evt event);

/* mbss_ap_stop_acs_ht40- Handle AP stop ACS/HT40 event
 * @vdev : vdev object
 * @event_data: event data
 * @event: if mgr event
 *
 * Return: QDF_STATUS
 */
QDF_STATUS mbss_ap_stop_acs_ht40(struct wlan_objmgr_vdev *vdev,
				 struct if_mgr_event_data *event_data,
				 enum wlan_if_mgr_evt event);

/* mbss_ap_done_acs_ht40- Handle AP ACS/HT40 done event
 * @vdev : vdev object
 * @event_data: event data
 * @event: if mgr event
 *
 * Return: QDF_STATUS
 */
QDF_STATUS mbss_ap_done_acs_ht40(struct wlan_objmgr_vdev *vdev,
				 struct if_mgr_event_data *event_data,
				 enum wlan_if_mgr_evt event);

/* mbss_ap_cancel_acs_ht40- Handle AP ACS/HT40 cancel event
 * @vdev : vdev object
 * @event_data: event data
 * @event: if mgr event
 *
 * Return: QDF_STATUS
 */
QDF_STATUS mbss_ap_cancel_acs_ht40(struct wlan_objmgr_vdev *vdev,
				   struct if_mgr_event_data *event_data,
				   enum wlan_if_mgr_evt event);

#endif
