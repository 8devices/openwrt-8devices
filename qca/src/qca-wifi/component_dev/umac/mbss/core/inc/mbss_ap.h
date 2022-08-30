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

/* DOC: mbss_events.h
 * This file provides prototypes of the functions
 * to handle AP vdev events and take actions for
 * inter vdev co-ordination.
 */

#ifndef _MBSS_AP_H_
#define _MBSS_AP_H_

#include "mbss.h"

/* mbss_ap_start - Handle AP start event
 * @vdev : vdev object
 * @event_data: event data
 *
 * Return: QDF_STATUS
 */
QDF_STATUS mbss_ap_start(struct wlan_objmgr_vdev *vdev,
			 struct if_mgr_event_data *event_data);

/* mbss_start_complete - Handle AP start complete event
 * @vdev : vdev object
 * @event_data: event data
 *
 * Return: QDF_STATUS
 */
QDF_STATUS mbss_ap_start_complete(struct wlan_objmgr_vdev *vdev,
				  struct if_mgr_event_data *event_data);

/* mbss_ap_stop - Handle AP stop event
 * @vdev : vdev object
 * @event_data: event data
 *
 * Return: QDF_STATUS
 */
QDF_STATUS mbss_ap_stop(struct wlan_objmgr_vdev *vdev,
			struct if_mgr_event_data *event_data);

/* mbss_stop_complete - Handle AP stop complete event
 * @vdev : vdev object
 * @event_data: event data
 *
 * Return: QDF_STATUS
 */
QDF_STATUS mbss_ap_stop_complete(struct wlan_objmgr_vdev *vdev,
				 struct if_mgr_event_data *event_data);
#endif
