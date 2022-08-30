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
 * DOC: reg_channel.h
 * This file provides prototypes of the channel list APIs in addition to
 * predefined macros.
 */

#ifndef __REG_CHANNEL_H_
#define __REG_CHANNEL_H_

#include <wlan_reg_channel_api.h>

#define NEXT_20_CH_OFFSET 20

#ifdef CONFIG_HOST_FIND_CHAN

/**
 * reg_is_phymode_chwidth_allowed() - Check if requested phymode is allowed
 * @pdev_priv_obj: Pointer to regulatory pdev private object.
 * @phy_in: phymode that the user requested.
 * @ch_width: Channel width that the user requested.
 * @primary_freq: Input primary frequency.
 * @in_6g_pwr_mode: Input 6g power mode based on which the 6g channel list
 * is determined.
 *
 * Return: true if phymode is allowed, else false.
 */
bool reg_is_phymode_chwidth_allowed(
		struct wlan_regulatory_pdev_priv_obj *pdev_priv_obj,
		enum reg_phymode phy_in,
		enum phy_ch_width ch_width,
		qdf_freq_t primary_freq,
		enum supported_6g_pwr_types in_6g_pwr_mode);

/**
 * reg_set_chan_blocked() - Set is_chan_hop_blocked to true for a frequency
 * in the current chan list.
 * @pdev: Pointer to pdev.
 * @freq: Channel frequency in MHz.
 *
 * Return: void.
 */
void reg_set_chan_blocked(struct wlan_objmgr_pdev *pdev, qdf_freq_t freq);

/**
 * wlan_reg_is_chan_blocked() - Check if is_chan_hop_blocked to true for a
 * frequency in the current chan list.
 * @pdev: Pointer to pdev.
 * @freq: Channel frequency in MHz.
 *
 * Return: true if is_chan_hop_blocked is true for the input frequency, else
 * false.
 */
bool reg_is_chan_blocked(struct wlan_objmgr_pdev *pdev, qdf_freq_t freq);

/**
 * reg_is_chan_blocked() - Clear is_chan_hop_blocked for channel in the
 * current chan list.
 * @pdev: Pointer to pdev.
 *
 * Return: void.
 */
void reg_clear_allchan_blocked(struct wlan_objmgr_pdev *pdev);

/**
 * reg_set_chan_ht40intol() - Set ht40intol_flags to the value for a
 * frequency in the current chan list.
 * @pdev: Pointer to pdev.
 * @freq: Channel frequency in MHz.
 * @ht40intol_flags: ht40intol_flags to be set.
 *
 * Return: void.
 */
void reg_set_chan_ht40intol(struct wlan_objmgr_pdev *pdev, qdf_freq_t freq,
			    enum ht40_intol ht40intol_flags);

/**
 * reg_clear_chan_ht40intol() - Clear the ht40intol_flags from the
 * regulatory channel corresponding to the frequency in the current chan list.
 * @pdev: Pointer to pdev.
 * @freq: Channel frequency in MHz.
 * @ht40intol_flags: ht40intol_flags to be cleared.
 *
 * Return: void.
 */
void reg_clear_chan_ht40intol(struct wlan_objmgr_pdev *pdev, qdf_freq_t freq,
			      enum ht40_intol ht40intol_flags);

/**
 * reg_is_chan_ht40intol() - Check if the ht40intol flag is set to the
 * given enum for a frequency in the current chan list.
 * @pdev: Pointer to pdev.
 * @freq: Channel frequency in MHz.
 * @ht40intol_flags: The ht40intol flag (plus/minus) to check.
 *
 * Return: true if is_chan_htintol is set to given value for the input
 * frequency, else false.
 */
bool reg_is_chan_ht40intol(struct wlan_objmgr_pdev *pdev, qdf_freq_t freq,
			   enum ht40_intol ht40intol_flags);

/**
 * wlan_reg_clear_allchan_ht40intol() - Clear ht40intol_flags for all channels
 * in the current chan list.
 * @pdev: Pointer to pdev.
 *
 * Return: void.
 */
void reg_clear_allchan_ht40intol(struct wlan_objmgr_pdev *pdev);

/*
 * reg_is_band_present() - Check if input band channels are present
 * in the regulatory current channel list.
 * @pdev: pdev pointer.
 * @reg_band: regulatory band.
 *
 */
bool reg_is_band_present(struct wlan_objmgr_pdev *pdev,
			 enum reg_wifi_band reg_band);
#else
static inline bool reg_is_phymode_chwidth_allowed(
		struct wlan_regulatory_pdev_priv_obj *pdev_priv_obj,
		enum reg_phymode phy_in,
		enum phy_ch_width ch_width,
		qdf_freq_t primary_freq)
{
	return false;
}

static inline
void reg_set_chan_blocked(struct wlan_objmgr_pdev *pdev, qdf_freq_t freq)
{
}

static inline
bool reg_is_chan_blocked(struct wlan_objmgr_pdev *pdev, qdf_freq_t freq)
{
	return false;
}

static inline void reg_clear_allchan_blocked(struct wlan_objmgr_pdev *pdev)
{
}

static inline void
reg_set_chan_ht40intol(struct wlan_objmgr_pdev *pdev, qdf_freq_t freq,
		       enum ht40_intol ht40intol_flags)
{
}

	static inline void
reg_clear_allchan_ht40intol(struct wlan_objmgr_pdev *pdev, qdf_freq_t freq,
			    enum ht40_intol ht40intol_flags)
{
}

static inline bool
reg_is_chan_ht40intol(struct wlan_objmgr_pdev *pdev, qdf_freq_t freq,
		      enum ht40_intol ht40intol_flags)
{
	return false;
}

static inline void
reg_clear_allchan_ht40intol(struct wlan_objmgr_pdev *pdev)
{
}

static inline bool
reg_is_band_present(struct wlan_objmgr_pdev *pdev, enum reg_wifi_band reg_band)
{
	return false;
}
#endif /* CONFIG_HOST_FIND_CHAN */

/**
 * reg_is_nol_for_freq () - Checks the channel is a nol channel or not
 * @pdev: pdev ptr
 * @freq: Channel center frequency
 *
 * Return: true if nol channel else false.
 */
bool reg_is_nol_for_freq(struct wlan_objmgr_pdev *pdev, qdf_freq_t freq);

/**
 * reg_is_nol_hist_for_freq () - Checks the channel is a nol hist channel or not
 * @pdev: pdev ptr
 * @freq: Channel center frequency
 *
 * Return: true if nol channel else false.
 */
bool reg_is_nol_hist_for_freq(struct wlan_objmgr_pdev *pdev, qdf_freq_t freq);

/**
 * reg_get_ap_chan_list() - Get the AP master channel list
 * @pdev     : Pointer to pdev
 * @chan_list: Pointer to the channel list
 * @get_cur_chan_list: Flag to pull the current channel list
 * @ap_pwr_type: Type of AP power for 6GHz
 *
 * NOTE: If get_cur_chan_list is true, then ap_pwr_type is ignored.
 *
 * Return:
 * QDF_STATUS_SUCCESS: Success
 * QDF_STATUS_E_INVAL: Failed to get channel list
 */
QDF_STATUS reg_get_ap_chan_list(struct wlan_objmgr_pdev *pdev,
				struct regulatory_channel *chan_list,
				bool get_cur_chan_list,
				enum reg_6g_ap_type ap_pwr_type);

/**
 * reg_is_freq_width_dfs()- Check if a channel is DFS, given the channel
 * frequency and width combination.
 * @pdev: Pointer to pdev.
 * @freq: Channel center frequency.
 * @ch_width: Channel Width.
 *
 * Return: True if frequency + width has DFS subchannels, else false.
 */
bool reg_is_freq_width_dfs(struct wlan_objmgr_pdev *pdev,
			   qdf_freq_t freq,
			   enum phy_ch_width ch_width);

/**
 * reg_get_channel_params () - Sets channel parameteres for given
 * bandwidth
 * @pdev: Pointer to pdev
 * @freq: Channel center frequency.
 * @sec_ch_2g_freq: Secondary 2G channel frequency
 * @ch_params: pointer to the channel parameters.
 * @in_6g_pwr_mode: Input 6g power mode based on which the 6g channel list
 * is determined.
 *
 * Return: None
 */
void reg_get_channel_params(struct wlan_objmgr_pdev *pdev,
			    qdf_freq_t freq,
			    qdf_freq_t sec_ch_2g_freq,
			    struct ch_params *ch_params,
			    enum supported_6g_pwr_types in_6g_pwr_mode);

/**
 * reg_get_wmodes_and_max_chwidth() - Filter out the wireless modes
 * that are not supported by the available regulatory channels. Also,
 * return the maxbw supported by regulatory.
 * @pdev: Pointer to pdev.
 * @mode_select: Wireless modes to be filtered.
 * @include_nol_chan: boolean to indicate whether NOL channels are to be
 * considered as available channels.
 *
 * Return: Max reg bw
 */
uint16_t reg_get_wmodes_and_max_chwidth(struct wlan_objmgr_pdev *pdev,
					uint64_t *mode_select,
					bool include_nol_chan);

/**
 * reg_get_client_power_for_rep_ap() - Get the client power for the repeater AP
 * @pdev: Pointer to pdev.
 * @ap_pwr_type: AP power type
 * @client_type: Client type
 * @chan_freq: Channel frequency
 * @is_psd: Pointer to is_psd
 * @reg_eirp: Pointer to EIRP power
 * @reg_psd: Pointer to PSD
 *
 * Return: QDF_STATUS.
 */
QDF_STATUS
reg_get_client_power_for_rep_ap(struct wlan_objmgr_pdev *pdev,
				enum reg_6g_ap_type ap_pwr_type,
				enum reg_6g_client_type client_type,
				qdf_freq_t chan_freq,
				bool *is_psd, uint16_t *reg_eirp,
				uint16_t *reg_psd);

/**
 * reg_is_freq_present_in_reg_chan_list() - Check the input frequency
 * @pdev: Pointer to pdev
 * @freq: Channel center frequency in MHz
 * @in_6g_pwr_mode: Input 6G power type which will determine the 6G channel
 *
 * Check if the input channel center frequency is present in the current/super
 * channel list
 *
 * Return: Return true if channel center frequency is present in the current/super
 * channel list, else return false.
 */
bool
reg_is_freq_present_in_reg_chan_list(struct wlan_objmgr_pdev *pdev,
				     qdf_freq_t freq,
				     enum supported_6g_pwr_types in_6g_pwr_mode);

#endif /* __REG_CHANNEL_H_ */
