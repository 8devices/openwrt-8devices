/*
 * Copyright (c) 2022 Qualcomm Innovation Center, Inc. All rights reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for
 * any purpose with or without fee is hereby granted, provided that the
 * above copyright notice and this permission notice appear in all
 * copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL
 * WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE
 * AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL
 * DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR
 * PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */

/**
 * DOC: wlan_twt_ext_defs.h
 *
 * This file provide definition for structure/enums/defines related to
 * twt component
 */

#ifndef __WLAN_TWT_EXT_DEFS_H__
#define __WLAN_TWT_EXT_DEFS_H__

/*
 * struct twt_ic_cfg_params - TWT related cfg items
 * @twt_enable: global twt configuration
 * @sta_cong_timer_ms: STA TWT congestion timer TO value in terms of ms
 * @mbss_support: Flag indicating if AP TWT feature supported in
 *                MBSS mode or not.
 * @default_slot_size: This is the default value for the TWT slot setup
 *                by AP (units = microseconds)
 * @congestion_thresh_setup: Minimum congestion required to start setting
 *                up TWT sessions
 * @congestion_thresh_teardown: Minimum congestion below which TWT will be
 *                torn down (in percent of occupied airtime)
 * @congestion_thresh_critical: Threshold above which TWT will not be active
 *                (in percent of occupied airtime)
 * @interference_thresh_teardown: Minimum interference above that TWT
 *                 will not be active. The interference parameters use an
 *                 abstract method of evaluating interference.
 *                 The parameters are in percent, ranging from 0 for no
 *                 interference, to 100 for interference extreme enough
 *                 to completely block the signal of interest.
 * @interference_thresh_setup: Minimum interference below that TWT session
 *                 can be setup. The interference parameters use an
 *                 abstract method of evaluating interference.
 *                 The parameters are in percent, ranging from 0 for no
 *                 interference, to 100 for interference extreme enough
 *                 to completely block the signal of interest.
 * @min_no_sta_setup: Minimum no of STA required to start TWT setup
 * @min_no_sta_teardown: Minimum no of STA below which TWT will be torn down
 * @no_of_bcast_mcast_slots: Number of default slot sizes reserved for
 *                 BCAST/MCAST delivery
 * @min_no_twt_slots: Minimum no of available slots for TWT to be operational
 * @max_no_sta_twt: Max no of STA with which TWT is possible
 *                 (must be <= the wmi_resource_config's twt_ap_sta_count value)
 *      * The below interval parameters have units of milliseconds.
 * @mode_check_interval: Interval between two successive check to decide the
 *                 mode of TWT. (units = milliseconds)
 * @add_sta_slot_interval: Interval between decisions making to create
 *                 TWT slots for STAs. (units = milliseconds)
 * @remove_sta_slot_interval: Inrerval between decisions making to remove TWT
 *                 slot of STAs. (units = milliseconds)
 * @b_twt_enable: Enable or disable broadcast TWT.
 */
struct twt_ic_cfg_params {
	bool twt_enable;
	uint32_t sta_cong_timer_ms;
	uint32_t mbss_support;
	uint32_t default_slot_size;
	uint32_t congestion_thresh_setup;
	uint32_t congestion_thresh_teardown;
	uint32_t congestion_thresh_critical;
	uint32_t interference_thresh_teardown;
	uint32_t interference_thresh_setup;
	uint32_t min_no_sta_setup;
	uint32_t min_no_sta_teardown;
	uint32_t no_of_bcast_mcast_slots;
	uint32_t min_no_twt_slots;
	uint32_t max_no_sta_twt;
	uint32_t mode_check_interval;
	uint32_t add_sta_slot_interval;
	uint32_t remove_sta_slot_interval;
	bool b_twt_enable:1;
};

#endif /* __WLAN_TWT_EXT_DEFS_H__ */

