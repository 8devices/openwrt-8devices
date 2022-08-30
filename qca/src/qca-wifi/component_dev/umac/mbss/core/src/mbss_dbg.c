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
 * DOC: mbss_dbg.c
 * This file provides API implementation
 * for adding debug support for the MBSS framework
 */

#if defined WLAN_MBSS_DEBUG
#include "mbss_dbg.h"
#include "mbss.h"
#include "wlan_if_mgr_api.h"

#define MAX_DBG_TXT_SIZE 32

#define MBSS_DBG_LINE	"================================" \
			"================================" \
			"================================" \

void mbss_debug_add_entry(struct wlan_objmgr_vdev *vdev,
			  enum mbss_dbg_event_type ev_type,
			  struct if_mgr_event_data *if_ev_data,
			  enum wlan_vdev_state vdev_state)
{
	struct mbss_pdev *mbss_ctx;
	struct mbss_dbg_data *mbss_dbg;
	struct mbss_dbg_entry *entry;
	struct wlan_mbss_ev_data *ev_data = NULL;
	enum wlan_if_mgr_evt event = WLAN_IF_MGR_EV_MAX;;

	if (ev_type == MBSS_EV_IF_MGR && if_ev_data) {
		ev_data = (struct wlan_mbss_ev_data *)(if_ev_data->data);
		if (!ev_data) {
			mbss_err("Invalid Input");
			return;
		}
		event = ev_data->if_mgr_event;
	}

	mbss_ctx = mbss_get_pdev_ctx(wlan_vdev_get_pdev(vdev));
	if (!mbss_ctx) {
		mbss_err("MBSS context is NULL");
		return;
	}

	mbss_dbg = &mbss_ctx->mbss_dbg;
	mbss_dbg->index %= MBSS_DEBUG_ENTRIES;

	entry = &mbss_dbg->entry[mbss_dbg->index];
	entry->vdev_id = wlan_vdev_get_id(vdev);
	entry->valid = true;

	switch (ev_type) {
	case MBSS_EV_IF_MGR:
		entry->event.if_mgr_ev = event;
		break;
	case MBSS_EV_VDEV_SM:
		entry->event.vdev_state = event;
		break;
	default:
		return;
	}

	mbss_dbg->index++;
}

uint8_t mbss_get_vdev_print_offset(struct mbss_dbg_format *dbg_format,
				   uint8_t vdev_id)
{
	uint8_t offset = 0, idx;

	for (idx = 0; idx < WLAN_UMAC_PDEV_MAX_VDEVS; idx++) {
		if (dbg_format->vdev_offset[idx].vdev_id == vdev_id) {
			offset = dbg_format->vdev_offset[idx].print_offset;
			break;
		}
	}

	return offset;
}

void mbss_debug_print_bitmap(struct wlan_objmgr_pdev *pdev)
{
	struct mbss_pdev *mbss_ctx;
	mbss_bitmap_type *mbss_vdev_bmaps;
	uint32_t mbss_all_bmap_size;

	mbss_ctx = mbss_get_pdev_ctx(pdev);
	if (!mbss_ctx) {
		mbss_err("MBSS context is NULL");
		return;
	}

	mbss_vdev_bmaps = (mbss_bitmap_type *)&mbss_ctx->vdev_bitmaps;
	mbss_all_bmap_size = sizeof(mbss_ctx->vdev_bitmaps);

	mbss_hex_dump_debug(mbss_vdev_bmaps, mbss_all_bmap_size);
}

void mbss_debug_print_history(struct wlan_objmgr_pdev *pdev)
{
	struct mbss_pdev *mbss_ctx;
	struct mbss_dbg_data *mbss_dbg;
	uint32_t idx, data_idx, i, len;
	struct mbss_dbg_entry *entry;
	struct mbss_dbg_format dbg_format;
	uint8_t offset = 0, num_vdev = 0;
	char *buf, *buf_head;
	const char *ev_str;

	mbss_ctx = mbss_get_pdev_ctx(pdev);
	if (!mbss_ctx) {
		mbss_err("MBSS context is NULL");
		return;
	}
	mbss_dbg = &mbss_ctx->mbss_dbg;
	qdf_mem_zero(&dbg_format, sizeof(struct mbss_dbg_format));

	/*
	 * Find the number of vdevs that has entry and
	 * also update a bitmap with their vdev ids.
	 */
	for (idx = 0; idx < MBSS_DEBUG_ENTRIES; idx++) {
		data_idx = (mbss_dbg->index + idx) % MBSS_DEBUG_ENTRIES;
		entry = &mbss_dbg->entry[data_idx];
		if (entry->valid) {
			qdf_set_bit(entry->vdev_id,
				    dbg_format.debug_entry_bitmap);
		}
	}

	/*
	 * Update the print offset for vdevs
	 */
	for (idx = 0; idx < MBSS_BITMAP_SIZE &&
	     offset < WLAN_UMAC_PDEV_MAX_VDEVS; idx++) {
		if (qdf_test_bit(idx, dbg_format.debug_entry_bitmap)) {
			dbg_format.vdev_offset[offset].vdev_id = idx;
			dbg_format.vdev_offset[offset].print_offset = offset;
			offset++;
			num_vdev++;
		}
	}

	if (!num_vdev)
		goto exit;

	buf = qdf_mem_malloc(MAX_DBG_TXT_SIZE * WLAN_UMAC_PDEV_MAX_VDEVS);
	if (!buf) {
		mbss_nofl_err("Allocation failed [PSOC: %d |PDEV: %d]",
			      wlan_psoc_get_id(wlan_pdev_get_psoc(pdev)),
			      wlan_objmgr_pdev_get_pdev_id(pdev));
		goto exit;
	}
	buf_head = buf;

	mbss_nofl_debug(MBSS_DBG_LINE);

	mbss_nofl_debug("MBSS Framework History [PSOC: %d |PDEV: %d]",
			wlan_psoc_get_id(wlan_pdev_get_psoc(pdev)),
			wlan_objmgr_pdev_get_pdev_id(pdev));

	mbss_nofl_debug(MBSS_DBG_LINE);

	/* Print the debug header with VDEV id*/
	for (idx = 0; idx < MBSS_BITMAP_SIZE; idx++) {
		if (qdf_test_bit(idx, dbg_format.debug_entry_bitmap)) {
			len = qdf_scnprintf(
				buf, 32,
				"|VDEV:%2d%23s", idx, "");
			buf = buf + len;
		}
	}

	mbss_nofl_debug("%s", buf_head);
	mbss_nofl_debug(MBSS_DBG_LINE);

	buf = buf_head;
	qdf_mem_zero(buf, MAX_DBG_TXT_SIZE * WLAN_UMAC_PDEV_MAX_VDEVS);

	for (idx = 0; idx < MBSS_DEBUG_ENTRIES; idx++) {
		data_idx = (mbss_dbg->index + idx) % MBSS_DEBUG_ENTRIES;
		entry = &mbss_dbg->entry[data_idx];
		if (entry->valid) {
			ev_str = if_mgr_get_event_str(entry->event.if_mgr_ev);
			if (!ev_str)
				continue;
			offset = mbss_get_vdev_print_offset(
					&dbg_format,
					entry->vdev_id);
			for (i = 0; i < num_vdev; i++) {
				if (i == offset) {
					len = qdf_scnprintf(
						buf, 32, "|%30s", ev_str);
					buf = buf + len;
					continue;
				}
				len = qdf_scnprintf(buf, 32, "|%30s", "");
				buf = buf + len;
			}
			mbss_nofl_debug(buf_head);
		}
		buf = buf_head;
		qdf_mem_zero(buf, MAX_DBG_TXT_SIZE * WLAN_UMAC_PDEV_MAX_VDEVS);
	}

	mbss_nofl_debug(MBSS_DBG_LINE);
	qdf_mem_free(buf);
exit:
	return;
}
#endif
