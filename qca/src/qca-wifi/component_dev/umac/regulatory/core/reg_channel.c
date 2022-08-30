/*
 * Copyright (c) 2020, The Linux Foundation. All rights reserved.
 * Copyright (c) 2022 Qualcomm Innovation Center, Inc. All rights reserved.
 *
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
 * DOC: reg_channel.c
 * This file defines the API to access/update/modify regulatory current channel
 * list by WIN host umac components.
 */

#include <wlan_cmn.h>
#include <reg_services_public_struct.h>
#include <reg_build_chan_list.h>
#include <wlan_objmgr_psoc_obj.h>
#include <wlan_objmgr_pdev_obj.h>
#include <reg_priv_objs.h>
#include <reg_services_common.h>
#include "reg_channel.h"
#include <wlan_reg_channel_api.h>
#include <wlan_reg_services_api.h>

#ifdef CONFIG_HOST_FIND_CHAN

#ifdef WLAN_FEATURE_11BE
static inline int is_11be_supported(uint64_t wireless_modes, uint32_t phybitmap)
{
	return (WIRELESS_11BE_MODES & wireless_modes) &&
		!(phybitmap & REGULATORY_PHYMODE_NO11BE);
}
#else
static inline int is_11be_supported(uint64_t wireless_modes, uint32_t phybitmap)
{
	return false;
}
#endif

static inline int is_11ax_supported(uint64_t wireless_modes, uint32_t phybitmap)
{
	return (WIRELESS_11AX_MODES & wireless_modes) &&
		!(phybitmap & REGULATORY_PHYMODE_NO11AX);
}

static inline int is_11ac_supported(uint64_t wireless_modes, uint32_t phybitmap)
{
	return (WIRELESS_11AC_MODES & wireless_modes) &&
		!(phybitmap & REGULATORY_PHYMODE_NO11AC);
}

static inline int is_11n_supported(uint64_t wireless_modes, uint32_t phybitmap)
{
	return (WIRELESS_11N_MODES & wireless_modes) &&
		!(phybitmap & REGULATORY_CHAN_NO11N);
}

static inline int is_11g_supported(uint64_t wireless_modes, uint32_t phybitmap)
{
	return (WIRELESS_11G_MODES & wireless_modes) &&
		!(phybitmap & REGULATORY_PHYMODE_NO11G);
}

static inline int is_11b_supported(uint64_t wireless_modes, uint32_t phybitmap)
{
	return (WIRELESS_11B_MODES & wireless_modes) &&
		!(phybitmap & REGULATORY_PHYMODE_NO11B);
}

static inline int is_11a_supported(uint64_t wireless_modes, uint32_t phybitmap)
{
	return (WIRELESS_11A_MODES & wireless_modes) &&
		!(phybitmap & REGULATORY_PHYMODE_NO11A);
}

#ifdef WLAN_FEATURE_11BE
static void fill_11be_max_phymode_chwidth(uint64_t wireless_modes,
					  uint32_t phybitmap,
					  enum phy_ch_width *max_chwidth,
					  enum reg_phymode *max_phymode)
{
	*max_phymode = REG_PHYMODE_11BE;
	if (wireless_modes & WIRELESS_320_MODES)
		*max_chwidth = CH_WIDTH_320MHZ;
	else if (wireless_modes & WIRELESS_160_MODES)
		*max_chwidth = CH_WIDTH_160MHZ;
	else if (wireless_modes & WIRELESS_80_MODES)
		*max_chwidth = CH_WIDTH_80MHZ;
	else if (wireless_modes & WIRELESS_40_MODES)
		*max_chwidth = CH_WIDTH_40MHZ;
}
#else
static inline void
fill_11be_max_phymode_chwidth(uint64_t wireless_modes,
			      uint32_t phybitmap,
			      enum phy_ch_width *max_chwidth,
			      enum reg_phymode *max_phymode)
{
}
#endif

void reg_update_max_phymode_chwidth_for_pdev(struct wlan_objmgr_pdev *pdev)
{
	uint64_t wireless_modes;
	uint32_t phybitmap;
	enum phy_ch_width max_chwidth = CH_WIDTH_20MHZ;
	enum reg_phymode max_phymode = REG_PHYMODE_MAX;
	struct wlan_regulatory_pdev_priv_obj *pdev_priv_obj;

	pdev_priv_obj = reg_get_pdev_obj(pdev);

	if (!IS_VALID_PDEV_REG_OBJ(pdev_priv_obj)) {
		reg_err("reg pdev priv obj is NULL");
		return;
	}

	wireless_modes = pdev_priv_obj->wireless_modes;
	phybitmap = pdev_priv_obj->phybitmap;

	if (is_11be_supported(wireless_modes, phybitmap)) {
		fill_11be_max_phymode_chwidth(wireless_modes, phybitmap,
					      &max_chwidth, &max_phymode);
	} else if (is_11ax_supported(wireless_modes, phybitmap)) {
		max_phymode = REG_PHYMODE_11AX;
		if (wireless_modes & WIRELESS_160_MODES)
			max_chwidth = CH_WIDTH_160MHZ;
		else if (wireless_modes & WIRELESS_80_MODES)
			max_chwidth = CH_WIDTH_80MHZ;
		else if (wireless_modes & WIRELESS_40_MODES)
			max_chwidth = CH_WIDTH_40MHZ;
	} else if (is_11ac_supported(wireless_modes, phybitmap)) {
		max_phymode = REG_PHYMODE_11AC;
		if (wireless_modes & WIRELESS_160_MODES)
			max_chwidth = CH_WIDTH_160MHZ;
		else if (wireless_modes & WIRELESS_80_MODES)
			max_chwidth = CH_WIDTH_80MHZ;
		else if (wireless_modes & WIRELESS_40_MODES)
			max_chwidth = CH_WIDTH_40MHZ;
	} else if (is_11n_supported(wireless_modes, phybitmap)) {
		max_phymode = REG_PHYMODE_11N;
		if (wireless_modes & WIRELESS_40_MODES)
			max_chwidth = CH_WIDTH_40MHZ;
	} else if (is_11g_supported(wireless_modes, phybitmap)) {
		max_phymode = REG_PHYMODE_11G;
	} else if (is_11b_supported(wireless_modes, phybitmap)) {
		max_phymode = REG_PHYMODE_11B;
	} else if (is_11a_supported(wireless_modes, phybitmap)) {
		max_phymode = REG_PHYMODE_11A;
	} else {
		reg_err("Device does not support any wireless_mode! %0llx",
			wireless_modes);
	}

	pdev_priv_obj->max_phymode = max_phymode;
	pdev_priv_obj->max_chwidth = max_chwidth;
}

/**
 * chwd_2_contbw_lst - Conversion array from channel width enum to value.
 * Array index of type phy_ch_width, return of type uint16_t.
 */
static uint16_t chwd_2_contbw_lst[CH_WIDTH_MAX + 1] = {
	BW_20_MHZ,   /* CH_WIDTH_20MHZ */
	BW_40_MHZ,   /* CH_WIDTH_40MHZ */
	BW_80_MHZ,   /* CH_WIDTH_80MHZ */
	BW_160_MHZ,  /* CH_WIDTH_160MHZ */
	BW_80_MHZ,   /* CH_WIDTH_80P80MHZ */
	BW_5_MHZ,    /* CH_WIDTH_5MHZ */
	BW_10_MHZ,   /* CH_WIDTH_10MHZ */
#ifdef WLAN_FEATURE_11BE
	BW_320_MHZ,  /* CH_WIDTH_320MHZ */
#endif
	0,           /* CH_WIDTH_INVALID */
#ifdef WLAN_FEATURE_11BE
	BW_320_MHZ,  /* CH_WIDTH_MAX */
#else
	BW_160_MHZ,  /* CH_WIDTH_MAX */
#endif

};

/**
 * reg_get_max_channel_width() - Get the maximum channel width supported
 * given a frequency and a global maximum channel width.
 * @pdev: Pointer to PDEV object.
 * @freq: Input frequency.
 * @g_max_width: Global maximum channel width.
 *
 * Return: Maximum channel width of type phy_ch_width.
 */
#ifdef WLAN_FEATURE_11BE
static enum phy_ch_width
reg_get_max_channel_width(struct wlan_objmgr_pdev *pdev,
			  qdf_freq_t freq,
			  enum phy_ch_width g_max_width,
			  enum supported_6g_pwr_types in_6g_pwr_mode)
{
	struct reg_channel_list chan_list = {0};
	uint16_t i, max_bw = 0;
	enum phy_ch_width output_width = CH_WIDTH_INVALID;

	wlan_reg_fill_channel_list_for_pwrmode(pdev, freq, 0,
					       g_max_width, 0,
					       &chan_list,
					       in_6g_pwr_mode, false);

	for (i = 0; i < chan_list.num_ch_params; i++) {
		struct ch_params *ch_param = &chan_list.chan_param[i];
		uint16_t cont_bw = chwd_2_contbw_lst[ch_param->ch_width];

		if (max_bw < cont_bw) {
			output_width = ch_param->ch_width;
			max_bw = cont_bw;
		}
	}
	return output_width;
}
#else
static enum phy_ch_width
reg_get_max_channel_width(struct wlan_objmgr_pdev *pdev,
			  qdf_freq_t freq,
			  enum phy_ch_width g_max_width,
			  enum supported_6g_pwr_types in_6g_pwr_mode)
{
	struct ch_params chan_params;

	chan_params.ch_width = g_max_width;
	reg_get_channel_params(pdev, freq, 0, &chan_params, in_6g_pwr_mode);
	return chan_params.ch_width;
}
#endif

void reg_modify_chan_list_for_max_chwidth_for_pwrmode(
		struct wlan_objmgr_pdev *pdev,
		struct regulatory_channel *cur_chan_list,
		enum supported_6g_pwr_types in_6g_pwr_mode)
{
	int i;
	struct wlan_regulatory_pdev_priv_obj *pdev_priv_obj;

	pdev_priv_obj = reg_get_pdev_obj(pdev);

	if (!IS_VALID_PDEV_REG_OBJ(pdev_priv_obj)) {
		reg_err("reg pdev priv obj is NULL");
		return;
	}

	for (i = 0; i < NUM_CHANNELS; i++) {
		enum phy_ch_width g_max_width = pdev_priv_obj->max_chwidth;
		enum phy_ch_width output_width = CH_WIDTH_INVALID;
		qdf_freq_t freq = cur_chan_list[i].center_freq;

		if (cur_chan_list[i].chan_flags & REGULATORY_CHAN_DISABLED)
			continue;

		/*
		 * Correct the max bandwidths if they were not taken care of
		 * while parsing the reg rules.
		 */
		output_width = reg_get_max_channel_width(pdev, freq,
							 g_max_width,
							 in_6g_pwr_mode);

		if (output_width != CH_WIDTH_INVALID)
			cur_chan_list[i].max_bw =
				qdf_min(cur_chan_list[i].max_bw,
					chwd_2_contbw_lst[output_width]);
	}
}

static uint64_t convregphymode2wirelessmodes[REG_PHYMODE_MAX] = {
	0xFFFFFFFF,                  /* REG_PHYMODE_INVALID */
	WIRELESS_11B_MODES,          /* REG_PHYMODE_11B     */
	WIRELESS_11G_MODES,          /* REG_PHYMODE_11G     */
	WIRELESS_11A_MODES,          /* REG_PHYMODE_11A     */
	WIRELESS_11N_MODES,          /* REG_PHYMODE_11N     */
	WIRELESS_11AC_MODES,         /* REG_PHYMODE_11AC    */
	WIRELESS_11AX_MODES,         /* REG_PHYMODE_11AX    */
#ifdef WLAN_FEATURE_11BE
	WIRELESS_11BE_MODES,         /* REG_PHYMODE_11BE    */
#endif
};

static uint64_t reg_is_phymode_in_wireless_modes(enum reg_phymode phy_in,
						 uint64_t wireless_modes)
{
	uint64_t sup_wireless_modes = convregphymode2wirelessmodes[phy_in];

	return sup_wireless_modes & wireless_modes;
}

bool reg_is_phymode_chwidth_allowed(
		struct wlan_regulatory_pdev_priv_obj *pdev_priv_obj,
		enum reg_phymode phy_in,
		enum phy_ch_width ch_width,
		qdf_freq_t freq,
		enum supported_6g_pwr_types in_6g_pwr_mode)
{
	uint32_t phymode_bitmap;
	uint64_t wireless_modes;
	enum phy_ch_width output_width = CH_WIDTH_INVALID;

	if (ch_width == CH_WIDTH_INVALID)
		return false;

	phymode_bitmap = pdev_priv_obj->phybitmap;
	wireless_modes = pdev_priv_obj->wireless_modes;

	if (reg_is_phymode_unallowed(phy_in, phymode_bitmap) ||
	    !reg_is_phymode_in_wireless_modes(phy_in, wireless_modes))
		return false;

	output_width = reg_get_max_channel_width(pdev_priv_obj->pdev_ptr,
						 freq,
						 ch_width,
						 in_6g_pwr_mode);

	if (output_width != ch_width)
		return false;

	return true;
}

void reg_set_chan_blocked(struct wlan_objmgr_pdev *pdev, qdf_freq_t freq)
{
	struct wlan_regulatory_pdev_priv_obj *pdev_priv_obj;
	struct regulatory_channel *cur_chan_list;
	int i;

	pdev_priv_obj = reg_get_pdev_obj(pdev);

	if (!IS_VALID_PDEV_REG_OBJ(pdev_priv_obj)) {
		reg_err("reg pdev priv obj is NULL");
		return;
	}

	cur_chan_list = pdev_priv_obj->cur_chan_list;

	for (i = 0; i < NUM_CHANNELS; i++) {
		if (cur_chan_list[i].center_freq == freq) {
			cur_chan_list[i].is_chan_hop_blocked = true;
			break;
		}
	}
}

bool reg_is_chan_blocked(struct wlan_objmgr_pdev *pdev, qdf_freq_t freq)
{
	struct wlan_regulatory_pdev_priv_obj *pdev_priv_obj;
	struct regulatory_channel *cur_chan_list;
	int i;

	pdev_priv_obj = reg_get_pdev_obj(pdev);

	if (!IS_VALID_PDEV_REG_OBJ(pdev_priv_obj)) {
		reg_err("reg pdev priv obj is NULL");
		return false;
	}

	cur_chan_list = pdev_priv_obj->cur_chan_list;

	for (i = 0; i < NUM_CHANNELS; i++)
		if (cur_chan_list[i].center_freq == freq)
			return cur_chan_list[i].is_chan_hop_blocked;

	return false;
}

void reg_clear_allchan_blocked(struct wlan_objmgr_pdev *pdev)
{
	struct wlan_regulatory_pdev_priv_obj *pdev_priv_obj;
	struct regulatory_channel *cur_chan_list;
	int i;

	pdev_priv_obj = reg_get_pdev_obj(pdev);

	if (!IS_VALID_PDEV_REG_OBJ(pdev_priv_obj)) {
		reg_err("reg pdev priv obj is NULL");
		return;
	}

	cur_chan_list = pdev_priv_obj->cur_chan_list;

	for (i = 0; i < NUM_CHANNELS; i++)
		cur_chan_list[i].is_chan_hop_blocked = false;
}

void reg_set_chan_ht40intol(struct wlan_objmgr_pdev *pdev, qdf_freq_t freq,
			    enum ht40_intol ht40intol_flags)
{
	struct wlan_regulatory_pdev_priv_obj *pdev_priv_obj;
	struct regulatory_channel *cur_chan_list;
	int i;

	pdev_priv_obj = reg_get_pdev_obj(pdev);

	if (!IS_VALID_PDEV_REG_OBJ(pdev_priv_obj)) {
		reg_err("reg pdev priv obj is NULL");
		return;
	}

	cur_chan_list = pdev_priv_obj->cur_chan_list;

	for (i = 0; i < NUM_CHANNELS; i++) {
		if (cur_chan_list[i].center_freq == freq)
			cur_chan_list[i].ht40intol_flags |=
					BIT(ht40intol_flags);
	}
}

void reg_clear_chan_ht40intol(struct wlan_objmgr_pdev *pdev, qdf_freq_t freq,
			      enum ht40_intol ht40intol_flags)
{
	struct wlan_regulatory_pdev_priv_obj *pdev_priv_obj;
	struct regulatory_channel *cur_chan_list;
	int i;

	pdev_priv_obj = reg_get_pdev_obj(pdev);

	if (!IS_VALID_PDEV_REG_OBJ(pdev_priv_obj)) {
		reg_err("reg pdev priv obj is NULL");
		return;
	}

	cur_chan_list = pdev_priv_obj->cur_chan_list;

	for (i = 0; i < NUM_CHANNELS; i++) {
		if (cur_chan_list[i].center_freq == freq)
			cur_chan_list[i].ht40intol_flags &=
				~(BIT(ht40intol_flags));
	}
}

bool reg_is_chan_ht40intol(struct wlan_objmgr_pdev *pdev, qdf_freq_t freq,
			   enum ht40_intol ht40intol_flags)
{
	struct wlan_regulatory_pdev_priv_obj *pdev_priv_obj;
	struct regulatory_channel *cur_chan_list;
	int i;

	pdev_priv_obj = reg_get_pdev_obj(pdev);

	if (!IS_VALID_PDEV_REG_OBJ(pdev_priv_obj)) {
		reg_err("reg pdev priv obj is NULL");
		return false;
	}

	cur_chan_list = pdev_priv_obj->cur_chan_list;

	for (i = 0; i < NUM_CHANNELS; i++)
		if (cur_chan_list[i].center_freq == freq)
			return (cur_chan_list[i].ht40intol_flags &
				BIT(ht40intol_flags));

	return false;
}

void reg_clear_allchan_ht40intol(struct wlan_objmgr_pdev *pdev)
{
	struct wlan_regulatory_pdev_priv_obj *pdev_priv_obj;
	struct regulatory_channel *cur_chan_list;
	int i;

	pdev_priv_obj = reg_get_pdev_obj(pdev);

	if (!IS_VALID_PDEV_REG_OBJ(pdev_priv_obj)) {
		reg_err("reg pdev priv obj is NULL");
		return;
	}

	cur_chan_list = pdev_priv_obj->cur_chan_list;

	for (i = 0; i < NUM_CHANNELS; i++)
		cur_chan_list[i].ht40intol_flags = 0;
}

/*
 * reg_is_band_found_internal - Check if a band channel is found in the
 * current channel list.
 *
 * @start_idx - Start index.
 * @end_idx - End index.
 * @cur_chan_list - Pointer to cur_chan_list.
 */
static bool reg_is_band_found_internal(enum channel_enum start_idx,
				       enum channel_enum end_idx,
				       struct regulatory_channel *cur_chan_list)
{
	uint8_t i;

	for (i = start_idx; i <= end_idx; i++)
		if (!(reg_is_chan_disabled_and_not_nol(&cur_chan_list[i])))
			return true;

	return false;
}

bool reg_is_band_present(struct wlan_objmgr_pdev *pdev,
			 enum reg_wifi_band reg_band)
{
	struct wlan_regulatory_pdev_priv_obj *pdev_priv_obj;
	struct regulatory_channel *cur_chan_list;
	enum channel_enum min_chan_idx, max_chan_idx;

	switch (reg_band) {
	case REG_BAND_2G:
		min_chan_idx = MIN_24GHZ_CHANNEL;
		max_chan_idx = MAX_24GHZ_CHANNEL;
		break;
	case REG_BAND_5G:
		min_chan_idx = MIN_49GHZ_CHANNEL;
		max_chan_idx = MAX_5GHZ_CHANNEL;
		break;
	case REG_BAND_6G:
		min_chan_idx = MIN_6GHZ_CHANNEL;
		max_chan_idx = MAX_6GHZ_CHANNEL;
		break;
	default:
		return false;
	}

	pdev_priv_obj = reg_get_pdev_obj(pdev);

	if (!IS_VALID_PDEV_REG_OBJ(pdev_priv_obj)) {
		reg_err("reg pdev priv obj is NULL");
		return false;
	}

	cur_chan_list = pdev_priv_obj->cur_chan_list;

	return reg_is_band_found_internal(min_chan_idx, max_chan_idx,
					  cur_chan_list);
}

#endif /* CONFIG_HOST_FIND_CHAN */

bool reg_is_nol_for_freq(struct wlan_objmgr_pdev *pdev, qdf_freq_t freq)
{
	enum channel_enum chan_enum;
	struct wlan_regulatory_pdev_priv_obj *pdev_priv_obj;

	chan_enum = reg_get_chan_enum_for_freq(freq);
	if (chan_enum == INVALID_CHANNEL) {
		reg_err("chan freq is not valid");
		return false;
	}

	pdev_priv_obj = reg_get_pdev_obj(pdev);

	if (!IS_VALID_PDEV_REG_OBJ(pdev_priv_obj)) {
		reg_err("pdev reg obj is NULL");
		return false;
	}

	return pdev_priv_obj->cur_chan_list[chan_enum].nol_chan;
}

bool reg_is_nol_hist_for_freq(struct wlan_objmgr_pdev *pdev, qdf_freq_t freq)
{
	enum channel_enum chan_enum;
	struct wlan_regulatory_pdev_priv_obj *pdev_priv_obj;

	chan_enum = reg_get_chan_enum_for_freq(freq);
	if (chan_enum == INVALID_CHANNEL) {
		reg_err("chan freq is not valid");
		return false;
	}

	pdev_priv_obj = reg_get_pdev_obj(pdev);

	if (!IS_VALID_PDEV_REG_OBJ(pdev_priv_obj)) {
		reg_err("pdev reg obj is NULL");
		return false;
	}

	return pdev_priv_obj->cur_chan_list[chan_enum].nol_history;
}

/**
 * reg_is_freq_band_dfs() - Find the bonded pair for the given frequency
 * and check if any of the sub frequencies in the bonded pair is DFS.
 * @pdev: Pointer to the pdev object.
 * @freq: Input frequency.
 * @bonded_chan_ptr: Frequency range of the given channel and width.
 *
 * Return: True if any of the channels in the bonded_chan_ar that contains
 * the input frequency is dfs, else false.
 */
static bool
reg_is_freq_band_dfs(struct wlan_objmgr_pdev *pdev,
		     qdf_freq_t freq,
		     const struct bonded_channel_freq *bonded_chan_ptr)
{
	qdf_freq_t chan_cfreq;
	bool is_dfs = false;

	chan_cfreq =  bonded_chan_ptr->start_freq;
	while (chan_cfreq <= bonded_chan_ptr->end_freq) {
		/* If any of the channel is disabled by regulatory, return. */
		if (reg_is_disable_for_pwrmode(pdev, chan_cfreq,
					       REG_CURRENT_PWR_MODE) &&
		    !reg_is_nol_for_freq(pdev, chan_cfreq))
			return false;
		if (reg_is_dfs_for_freq(pdev, chan_cfreq))
			is_dfs = true;
		chan_cfreq = chan_cfreq + NEXT_20_CH_OFFSET;
	}

	return is_dfs;
}

static
void reg_intersect_chan_list_power(struct wlan_objmgr_pdev *pdev,
				   struct regulatory_channel *pri_chan_list,
				   struct regulatory_channel *sec_chan_list,
				   uint32_t chan_list_size)
{
	bool chan_found_in_sec_list;
	uint32_t i, j;

	if (!pdev) {
		reg_err_rl("invalid pdev");
		return;
	}

	if (!pri_chan_list) {
		reg_err_rl("invalid pri_chan_list");
		return;
	}

	if (!sec_chan_list) {
		reg_err_rl("invalid sec_chan_list");
		return;
	}

	for (i = 0; i < chan_list_size; i++) {
		if ((pri_chan_list[i].state == CHANNEL_STATE_DISABLE) ||
		    (pri_chan_list[i].chan_flags & REGULATORY_CHAN_DISABLED)) {
			continue;
		}

		chan_found_in_sec_list = false;
		for (j = 0; j < chan_list_size; j++) {
			if ((sec_chan_list[j].state ==
						CHANNEL_STATE_DISABLE) ||
			    (sec_chan_list[j].chan_flags &
						REGULATORY_CHAN_DISABLED)) {
				continue;
			}

			if (pri_chan_list[i].center_freq ==
				sec_chan_list[j].center_freq) {
				chan_found_in_sec_list = true;
				break;
			}
		}

		if (!chan_found_in_sec_list) {
			pri_chan_list[i].state = CHANNEL_STATE_DISABLE;
			continue;
		}

		pri_chan_list[i].psd_flag = pri_chan_list[i].psd_flag &
						sec_chan_list[j].psd_flag;
		pri_chan_list[i].tx_power = QDF_MIN(
						pri_chan_list[i].tx_power,
						sec_chan_list[j].tx_power);
		pri_chan_list[i].psd_eirp = QDF_MIN(
					(int16_t)pri_chan_list[i].psd_eirp,
					(int16_t)sec_chan_list[j].psd_eirp);
	}
}

QDF_STATUS reg_get_ap_chan_list(struct wlan_objmgr_pdev *pdev,
				struct regulatory_channel *chan_list,
				bool get_cur_chan_list,
				enum reg_6g_ap_type ap_pwr_type)
{
	struct wlan_regulatory_pdev_priv_obj *pdev_priv_obj;
	struct wlan_objmgr_psoc *psoc;
	uint8_t i;
	const uint8_t reg_ap_pwr_type_2_supp_pwr_type[] = {
		[REG_INDOOR_AP] = REG_AP_LPI,
		[REG_STANDARD_POWER_AP] = REG_AP_SP,
		[REG_VERY_LOW_POWER_AP] = REG_AP_VLP,
	};


	if (!pdev) {
		reg_err_rl("invalid pdev");
		return QDF_STATUS_E_INVAL;
	}

	if (!chan_list) {
		reg_err_rl("invalid chanlist");
		return QDF_STATUS_E_INVAL;
	}

	pdev_priv_obj = reg_get_pdev_obj(pdev);
	psoc = wlan_pdev_get_psoc(pdev);

	if (get_cur_chan_list) {
		qdf_mem_copy(chan_list, pdev_priv_obj->cur_chan_list,
			NUM_CHANNELS * sizeof(struct regulatory_channel));
	} else {
		/* Get the current channel list for 2.4GHz and 5GHz */
		qdf_mem_copy(chan_list, pdev_priv_obj->cur_chan_list,
			NUM_CHANNELS * sizeof(struct regulatory_channel));

		/*
		 * If 6GHz channel list is present, populate it with desired
		 * power type
		 */
		if (pdev_priv_obj->is_6g_channel_list_populated) {
			if (ap_pwr_type >= REG_CURRENT_MAX_AP_TYPE) {
				reg_debug("invalid 6G AP power type");
				return QDF_STATUS_E_INVAL;
			}

			qdf_mem_copy(&chan_list[MIN_6GHZ_CHANNEL],
				pdev_priv_obj->mas_chan_list_6g_ap[ap_pwr_type],
				NUM_6GHZ_CHANNELS *
					sizeof(struct regulatory_channel));

#ifdef CONFIG_AFC_SUPPORT
			if (ap_pwr_type == REG_STANDARD_POWER_AP) {
				/*
				 * If the AP type is standard power, intersect
				 * the SP channel list with the AFC master
				 * channel list
				 */
				reg_intersect_chan_list_power(
					pdev,
					&chan_list[MIN_6GHZ_CHANNEL],
					pdev_priv_obj->mas_chan_list_6g_afc,
					NUM_6GHZ_CHANNELS);
			}
#endif

			/*
			 * Intersect the hardware frequency range with the
			 * 6GHz channels.
			 */
			for (i = 0; i < NUM_6GHZ_CHANNELS; i++) {
				if ((chan_list[MIN_6GHZ_CHANNEL+i].center_freq <
					pdev_priv_obj->range_5g_low) ||
				    (chan_list[MIN_6GHZ_CHANNEL+i].center_freq >
					pdev_priv_obj->range_5g_high)) {
					chan_list[MIN_6GHZ_CHANNEL+i].chan_flags
						|= REGULATORY_CHAN_DISABLED;
					chan_list[MIN_6GHZ_CHANNEL+i].state =
						CHANNEL_STATE_DISABLE;
				}
			}

			/*
			 * Check for edge channels
			 */
			if (!reg_is_lower_6g_edge_ch_supp(psoc)) {
				chan_list[CHAN_ENUM_5935].state =
						CHANNEL_STATE_DISABLE;
				chan_list[CHAN_ENUM_5935].chan_flags |=
						REGULATORY_CHAN_DISABLED;
			}

			if (reg_is_upper_6g_edge_ch_disabled(psoc)) {
				chan_list[CHAN_ENUM_7115].state =
						CHANNEL_STATE_DISABLE;
				chan_list[CHAN_ENUM_7115].chan_flags |=
						REGULATORY_CHAN_DISABLED;
			}
			reg_modify_chan_list_for_max_chwidth_for_pwrmode(pdev,
					chan_list,
					reg_ap_pwr_type_2_supp_pwr_type
					[ap_pwr_type]);
		}
	}

	return QDF_STATUS_SUCCESS;
}

bool reg_is_freq_width_dfs(struct wlan_objmgr_pdev *pdev,
			   qdf_freq_t freq,
			   enum phy_ch_width ch_width)
{
	const struct bonded_channel_freq *bonded_chan_ptr;

	if (ch_width == CH_WIDTH_20MHZ)
		return reg_is_dfs_for_freq(pdev, freq);

	bonded_chan_ptr = reg_get_bonded_chan_entry(freq, ch_width);

	if (!bonded_chan_ptr)
		return false;

	return reg_is_freq_band_dfs(pdev, freq, bonded_chan_ptr);
}

/**
 * reg_get_5g_channel_params ()- Set channel parameters like center
 * frequency for a bonded channel state. Also return the maximum bandwidth
 * supported by the channel.
 * @pdev: Pointer to pdev.
 * @freq: Channel center frequency.
 * @ch_params: Pointer to ch_params.
 * @in_6g_pwr_mode: Input power mode which decides the 6G channel list to be
 *
 * Return: void
 */
static void
reg_get_5g_channel_params(struct wlan_objmgr_pdev *pdev,
			  uint16_t freq,
			  struct ch_params *ch_params,
			  enum supported_6g_pwr_types in_6g_pwr_mode)
{
	/*
	 * Set channel parameters like center frequency for a bonded channel
	 * state. Also return the maximum bandwidth supported by the channel.
	*/

	enum channel_state chan_state = CHANNEL_STATE_ENABLE;
	enum channel_state chan_state2 = CHANNEL_STATE_ENABLE;
	const struct bonded_channel_freq *bonded_chan_ptr = NULL;
	const struct bonded_channel_freq *bonded_chan_ptr2 = NULL;
	struct wlan_regulatory_pdev_priv_obj *pdev_priv_obj;
	enum channel_enum chan_enum, sec_5g_chan_enum;
	uint16_t bw_80, sec_5g_freq_max_bw = 0;
	uint16_t max_bw;

	if (!ch_params) {
		reg_err("ch_params is NULL");
		return;
	}

	chan_enum = reg_get_chan_enum_for_freq(freq);
	if (chan_enum == INVALID_CHANNEL) {
		reg_err("chan freq is not valid");
		return;
	}

	pdev_priv_obj = reg_get_pdev_obj(pdev);
	if (!IS_VALID_PDEV_REG_OBJ(pdev_priv_obj)) {
		reg_err("reg pdev priv obj is NULL");
		return;
	}

	if (ch_params->ch_width >= CH_WIDTH_MAX) {
		if (ch_params->mhz_freq_seg1 != 0)
			ch_params->ch_width = CH_WIDTH_80P80MHZ;
		else
			ch_params->ch_width = CH_WIDTH_160MHZ;
	}

	if (reg_get_min_max_bw_cur_chan_list(pdev, chan_enum, in_6g_pwr_mode,
					     NULL, &max_bw)) {
		ch_params->ch_width = CH_WIDTH_INVALID;
		return;
	}

	bw_80 = reg_get_bw_value(CH_WIDTH_80MHZ);

	if (ch_params->ch_width == CH_WIDTH_80P80MHZ) {
		sec_5g_chan_enum =
			reg_get_chan_enum_for_freq(
				ch_params->mhz_freq_seg1 -
				NEAREST_20MHZ_CHAN_FREQ_OFFSET);
		if (sec_5g_chan_enum == INVALID_CHANNEL) {
			reg_err("secondary channel freq is not valid");
			return;
		}

		if (reg_get_min_max_bw_cur_chan_list(pdev, chan_enum,
						     in_6g_pwr_mode, NULL,
						     &sec_5g_freq_max_bw)) {
			ch_params->ch_width = CH_WIDTH_INVALID;
			return;
		}
	}

	while (ch_params->ch_width != CH_WIDTH_INVALID) {
		if (ch_params->ch_width == CH_WIDTH_80P80MHZ) {
			if ((max_bw < bw_80) || (sec_5g_freq_max_bw < bw_80))
				goto update_bw;
		} else if (max_bw < reg_get_bw_value(ch_params->ch_width)) {
			goto update_bw;
		}

		bonded_chan_ptr = NULL;
		bonded_chan_ptr2 = NULL;
		bonded_chan_ptr =
		    reg_get_bonded_chan_entry(freq, ch_params->ch_width);

		chan_state =
		    reg_get_5g_chan_state(pdev, freq, ch_params->ch_width,
					  in_6g_pwr_mode);

		if (ch_params->ch_width == CH_WIDTH_80P80MHZ) {
			chan_state2 = reg_get_5g_chan_state(
					pdev, ch_params->mhz_freq_seg1 -
					NEAREST_20MHZ_CHAN_FREQ_OFFSET,
					CH_WIDTH_80MHZ,
					in_6g_pwr_mode);

			chan_state = reg_combine_channel_states(
					chan_state, chan_state2);
		}

		if ((chan_state != CHANNEL_STATE_ENABLE) &&
		(chan_state != CHANNEL_STATE_DFS))
			goto update_bw;
		if (ch_params->ch_width <= CH_WIDTH_20MHZ) {
			ch_params->sec_ch_offset = NO_SEC_CH;
			ch_params->mhz_freq_seg0 = freq;
			ch_params->center_freq_seg0 =
				reg_freq_to_chan(pdev,
						 ch_params->mhz_freq_seg0);
			break;
		} else if (ch_params->ch_width >= CH_WIDTH_40MHZ) {
			bonded_chan_ptr2 =
				reg_get_bonded_chan_entry(freq, CH_WIDTH_40MHZ);

			if (!bonded_chan_ptr || !bonded_chan_ptr2)
				goto update_bw;
			if (freq == bonded_chan_ptr2->start_freq)
				ch_params->sec_ch_offset = LOW_PRIMARY_CH;
			else
				ch_params->sec_ch_offset = HIGH_PRIMARY_CH;

			ch_params->mhz_freq_seg0 =
				(bonded_chan_ptr->start_freq +
				 bonded_chan_ptr->end_freq) / 2;
			ch_params->center_freq_seg0 =
				reg_freq_to_chan(pdev,
						 ch_params->mhz_freq_seg0);
			break;
		}
update_bw:
		ch_params->ch_width =
			get_next_lower_bandwidth(ch_params->ch_width);
	}

	if (ch_params->ch_width == CH_WIDTH_160MHZ) {
		ch_params->mhz_freq_seg1 = ch_params->mhz_freq_seg0;
		ch_params->center_freq_seg1 =
			reg_freq_to_chan(pdev,
					   ch_params->mhz_freq_seg1);

		bonded_chan_ptr =
			reg_get_bonded_chan_entry(freq, CH_WIDTH_80MHZ);
		if (bonded_chan_ptr) {
			ch_params->mhz_freq_seg0 =
				(bonded_chan_ptr->start_freq +
		 bonded_chan_ptr->end_freq) / 2;
			ch_params->center_freq_seg0 =
				reg_freq_to_chan(pdev,
						 ch_params->mhz_freq_seg0);
		}
	}

	/* Overwrite mhz_freq_seg1 to 0 for non 160 and 80+80 width */
	if (!(ch_params->ch_width == CH_WIDTH_160MHZ ||
		ch_params->ch_width == CH_WIDTH_80P80MHZ)) {
		ch_params->mhz_freq_seg1 = 0;
		ch_params->center_freq_seg1 = 0;
	}
}

void reg_get_channel_params(struct wlan_objmgr_pdev *pdev,
			    qdf_freq_t freq,
			    qdf_freq_t sec_ch_2g_freq,
			    struct ch_params *ch_params,
			    enum supported_6g_pwr_types in_6g_pwr_mode)
{
    if (reg_is_5ghz_ch_freq(freq) || reg_is_6ghz_chan_freq(freq))
	reg_get_5g_channel_params(pdev, freq, ch_params, in_6g_pwr_mode);
    else if  (reg_is_24ghz_ch_freq(freq))
	reg_set_2g_channel_params_for_freq(pdev, freq, ch_params,
					   sec_ch_2g_freq);
}

/**
 * reg_get_max_channel_width_without_radar() - Get the maximum channel width
 * supported given a frequency and a global maximum channel width.
 * The radar infected subchannel are not part of the max bandwidth.
 * @pdev: Pointer to PDEV object.
 * @freq: Input frequency.
 * @g_max_width: Global maximum channel width.
 *
 * Return: Maximum channel width of type phy_ch_width.
 */
#ifdef WLAN_FEATURE_11BE
static enum phy_ch_width
reg_get_max_channel_width_without_radar(struct wlan_objmgr_pdev *pdev,
					qdf_freq_t freq,
					enum phy_ch_width g_max_width)
{
	struct reg_channel_list chan_list = {0};
	uint16_t i, max_bw = 0;
	enum phy_ch_width output_width = CH_WIDTH_INVALID;

	wlan_reg_fill_channel_list_for_pwrmode(pdev, freq, 0,
					       g_max_width, 0,
					       &chan_list,
					       REG_CURRENT_PWR_MODE, true);

	for (i = 0; i < chan_list.num_ch_params; i++) {
		struct ch_params *ch_param = &chan_list.chan_param[i];
		uint16_t cont_bw = chwd_2_contbw_lst[ch_param->ch_width];

		if (max_bw < cont_bw) {
			output_width = ch_param->ch_width;
			max_bw = cont_bw;
		}
	}
	return output_width;
}
#else
static enum phy_ch_width
reg_get_max_channel_width_without_radar(struct wlan_objmgr_pdev *pdev,
					qdf_freq_t freq,
					enum phy_ch_width g_max_width)
{
	struct ch_params chan_params;
	enum reg_6g_ap_type in_6g_pwr_mode;

	reg_get_cur_6g_ap_pwr_type(pdev, &in_6g_pwr_mode);
	chan_params.ch_width = g_max_width;
	reg_set_channel_params_for_pwrmode(pdev, freq, 0, &chan_params,
					   in_6g_pwr_mode, true);
	return chan_params.ch_width;
}
#endif

#ifdef WLAN_FEATURE_11BE
static void
reg_remove_320mhz_modes(int max_bw, uint64_t *wireless_modes)
{
	/**
	 * Check for max_bw greater than 160 to include both 240 and
	 * 320MHz support as 320 modes.
	 */
	if (max_bw <= BW_160_MHZ)
		*wireless_modes &= (~WIRELESS_320_MODES);
}
#else
static inline void
reg_remove_320mhz_modes(int max_bw, uint64_t *wireless_modes)
{
}
#endif

uint16_t reg_get_wmodes_and_max_chwidth(struct wlan_objmgr_pdev *pdev,
					uint64_t *mode_select,
					bool include_nol_chan)
{
	struct wlan_regulatory_pdev_priv_obj *pdev_priv_obj;
	uint64_t in_wireless_modes = *mode_select;
	struct regulatory_channel *chan_list;
	enum supported_6g_pwr_types pwr_mode;
	int i, max_bw = BW_20_MHZ;
	uint64_t band_modes = 0;

	pdev_priv_obj = reg_get_pdev_obj(pdev);

	if (!IS_VALID_PDEV_REG_OBJ(pdev_priv_obj)) {
		reg_err("pdev reg obj is NULL");
		return 0;
	}

	chan_list = qdf_mem_malloc(NUM_CHANNELS * sizeof(*chan_list));
	if (!chan_list)
		return 0;

	for (pwr_mode = REG_AP_LPI; pwr_mode <= REG_CLI_SUB_VLP; pwr_mode++) {

		qdf_mem_zero(chan_list, NUM_CHANNELS * sizeof(*chan_list));
		if (reg_get_pwrmode_chan_list(pdev, chan_list, pwr_mode)) {
			qdf_mem_free(chan_list);
			return 0;
		}

		for (i = 0; i < NUM_CHANNELS; i++) {
			qdf_freq_t freq = chan_list[i].center_freq;
			uint16_t cur_bw = chan_list[i].max_bw;

			if (reg_is_chan_disabled_and_not_nol(&chan_list[i]))
				continue;

			if (WLAN_REG_IS_24GHZ_CH_FREQ(freq))
				band_modes |= WIRELESS_2G_MODES;

			if (WLAN_REG_IS_49GHZ_FREQ(freq))
				band_modes |= WIRELESS_49G_MODES;

			if (WLAN_REG_IS_5GHZ_CH_FREQ(freq))
				band_modes |= WIRELESS_5G_MODES;

			if (WLAN_REG_IS_6GHZ_CHAN_FREQ(freq))
				band_modes |= WIRELESS_6G_MODES;

			if (!include_nol_chan &&
			    WLAN_REG_IS_5GHZ_CH_FREQ(freq)) {
				enum phy_ch_width in_chwidth, out_chwidth;

				in_chwidth = reg_find_chwidth_from_bw(cur_bw);
				out_chwidth =
				    reg_get_max_channel_width_without_radar(
								pdev,
								freq,
								in_chwidth);
				cur_bw = chwd_2_contbw_lst[out_chwidth];
			}

			if (max_bw < cur_bw)
				max_bw = cur_bw;
		}
	}
	qdf_mem_free(chan_list);

	in_wireless_modes &= band_modes;

	if (max_bw < BW_40_MHZ)
		in_wireless_modes &= (~WIRELESS_40_MODES);

	if (max_bw < BW_80_MHZ)
		in_wireless_modes &= (~WIRELESS_80_MODES);

	if (max_bw < BW_160_MHZ) {
		in_wireless_modes &= (~WIRELESS_160_MODES);
		if (include_nol_chan)
			in_wireless_modes &= (~WIRELESS_80P80_MODES);
	}

	reg_remove_320mhz_modes(max_bw, &in_wireless_modes);
	*mode_select = in_wireless_modes;

	return max_bw;
}

QDF_STATUS
reg_get_client_power_for_rep_ap(struct wlan_objmgr_pdev *pdev,
				enum reg_6g_ap_type ap_pwr_type,
				enum reg_6g_client_type client_type,
				qdf_freq_t chan_freq,
				bool *is_psd, uint16_t *reg_eirp,
				uint16_t *reg_psd)
{
	struct wlan_regulatory_pdev_priv_obj *pdev_priv_obj;
	struct regulatory_channel *master_chan_list;
	QDF_STATUS status = QDF_STATUS_SUCCESS;

	pdev_priv_obj = reg_get_pdev_obj(pdev);

	if (!IS_VALID_PDEV_REG_OBJ(pdev_priv_obj)) {
		reg_err("pdev reg obj is NULL");
		return QDF_STATUS_E_FAILURE;
	}

	master_chan_list = pdev_priv_obj->
			mas_chan_list_6g_client[ap_pwr_type][client_type];

	reg_find_txpower_from_6g_list(chan_freq, master_chan_list,
				      reg_eirp);

	*is_psd = reg_is_6g_psd_power(pdev);
	if (*is_psd)
		status = reg_get_6g_chan_psd_eirp_power(chan_freq,
							master_chan_list,
							reg_psd);

	return status;
}

/**
 * reg_is_freq_present_in_6g_reg_chan_list() - Check the input frequency
 * for 6Ghz channel
 * @pdev: Pointer to pdev
 * @freq: Channel center frequency in MHz
 * @in_6g_pwr_mode: Input 6G power type which will determine the 6G channel
 *
 * Check if the input channel center frequency is present in the super
 * channel list
 *
 * Return: Return true if channel center frequency is present in the super
 * channel list, else return false.
 */
static bool
reg_is_freq_present_in_6g_reg_chan_list(struct wlan_objmgr_pdev *pdev,
					qdf_freq_t freq,
					enum supported_6g_pwr_types in_6g_pwr_mode)
{
	enum channel_enum chan_enum;
	enum channel_state pm_state_arr;
	uint32_t pm_chan_flag;
	struct regulatory_channel *cur_chan_list;
	struct super_chan_info *p_sup_chan;
	struct super_chan_info *sup_chan_entry;
	struct wlan_regulatory_pdev_priv_obj *pdev_priv_obj;

	pdev_priv_obj = reg_get_pdev_obj(pdev);

	if (!IS_VALID_PDEV_REG_OBJ(pdev_priv_obj)) {
		reg_err_rl("pdev reg obj is null");
		return false;
	}
	p_sup_chan = &pdev_priv_obj->super_chan_list[0];
	cur_chan_list = pdev_priv_obj->cur_chan_list;
	for (chan_enum = 0; chan_enum < NUM_6GHZ_CHANNELS; chan_enum++) {
		if (in_6g_pwr_mode == REG_BEST_PWR_MODE)
			in_6g_pwr_mode = p_sup_chan[chan_enum].best_power_mode;

		if (in_6g_pwr_mode < REG_AP_LPI || 
			in_6g_pwr_mode > REG_CLI_SUB_VLP)
			return false;
		sup_chan_entry = &p_sup_chan[chan_enum];
		pm_state_arr = sup_chan_entry->state_arr[in_6g_pwr_mode];
		pm_chan_flag = sup_chan_entry->chan_flags_arr[in_6g_pwr_mode];
		if (cur_chan_list[chan_enum  + MIN_6GHZ_CHANNEL].center_freq == freq)
			if ((pm_state_arr != CHANNEL_STATE_DISABLE) &&
			    !(pm_chan_flag & REGULATORY_CHAN_DISABLED))
				return true;
	}
	reg_debug_rl("Channel center frequency %u not found", freq);

	return false;
}

bool
reg_is_freq_present_in_reg_chan_list(struct wlan_objmgr_pdev *pdev,
				     qdf_freq_t freq,
				     enum supported_6g_pwr_types in_6g_pwr_mode)
{
	if (!WLAN_REG_IS_6GHZ_CHAN_FREQ(freq))
		return reg_is_freq_present_in_cur_chan_list(pdev, freq);
	else
		return reg_is_freq_present_in_6g_reg_chan_list(pdev, freq,
				in_6g_pwr_mode);
}
