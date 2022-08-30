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
 * DOC: mbss_dbg.h
 * This file provides API prototypes
 * for adding debug support for the MBSS framework
 */

#ifndef _MBSS_DBG_H_
#define _MBSS_DBG_H_

#if defined WLAN_MBSS_DEBUG
#include "wlan_if_mgr_public_struct.h"
#include "wlan_objmgr_vdev_obj.h"
#include "wlan_vdev_mlme_main.h"

#define MBSS_DEBUG_ENTRIES 100

/**
 * enum mbss_dbg_event_type: MBSS debug event type
 * @MBSS_EV_IF_MGR: MBSS Interface manager event type
 * @MBSS_EV_VDEV_SM: MBSS VDEV SM Event type
 * @MBSS_EV_MAX: MBSS Max Event type
 */
enum mbss_dbg_event_type {
	MBSS_EV_IF_MGR = 0,
	MBSS_EV_VDEV_SM = 1,
	MBSS_EV_MAX = 2,
};

/**
 * struct mbss_dbg_entry- MBSS debug entry
 * @vdev_id: vdev id
 * @valid: is entry updated
 * @event_type: event type
 * @event: event
*/
struct mbss_dbg_entry {
	uint8_t vdev_id;
	bool valid;
	enum mbss_dbg_event_type event_type;
	union event_t {
		enum wlan_if_mgr_evt if_mgr_ev;
		enum wlan_vdev_state vdev_state;
	} event;
};

/**
 * struct mbss_dbg_data- MBSS Debug data
 * @index: index
 * @entry: array of debug entries
*/
struct mbss_dbg_data {
	uint32_t index;
	struct mbss_dbg_entry entry[MBSS_DEBUG_ENTRIES];
};

/**
 * struct mbss_print_offset- store print offset based on vdev id
 * @vdev_id: vdev id
 * @print_offset: print offset
*/
struct mbss_print_offset {
	uint8_t vdev_id;
	uint8_t print_offset;
};

/**
 * struct - mbss_dbg_format
 * @debug_entry_bitmap: debug entry bitmap
 * @mbss_print_offset: print offset
*/
struct mbss_dbg_format {
	qdf_bitmap(debug_entry_bitmap, WLAN_UMAC_PSOC_MAX_VDEVS);
	struct mbss_print_offset vdev_offset[WLAN_UMAC_PDEV_MAX_VDEVS];
};

#define mbss_debug_add_if_event_entry(vdev, event) \
	mbss_debug_add_entry(vdev, MBSS_EV_IF_MGR, event, 0)

#define mbss_debug_add_vdev_sm_entry(vdev, state) \
	mbss_debug_add_entry(vdev, MBSS_EV_VDEV_SM, NULL, state)

/* mbss_debug_add_entry - Add MBSS debug entry
 * @vdev : vdev object
 * @ev_type: Event type
 * @data: Event Data
 * @vdev_state: VDEV state
 *
 * Return: void
 */
void mbss_debug_add_entry(struct wlan_objmgr_vdev *vdev,
			  enum mbss_dbg_event_type ev_type,
			  struct if_mgr_event_data *data,
			  enum wlan_vdev_state vdev_state);

/* mbss_debug_print_history -  Print MBSS debug history
 * @pdev : pdev object
 *
 * Return: void
 */
void mbss_debug_print_history(struct wlan_objmgr_pdev *pdev);

/* mbss_debug_print_bitmap- Print MBSS bitmaps
 * @pdev : pdev object
 *
 * Return: void
 */
void mbss_debug_print_bitmap(struct wlan_objmgr_pdev *pdev);
#else
#define mbss_debug_add_entry(vdev, ev_type, event)
#define mbss_debug_print_history(pdev)
#define mbss_debug_print_bitmap(pdev)
#define mbss_debug_add_if_event_entry(vdev, event)
#define mbss_debug_add_vdev_sm_entry(vdev, state)
#endif

#endif /* _MBSS_DBG_H_ */
