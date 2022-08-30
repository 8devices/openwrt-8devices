/*
 * Copyright (c) 2021, The Linux Foundation. All rights reserved.

 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.

 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */
#include <ieee80211_cfg80211.h>
#include <dp_mon.h>
#include <dp_mon_ol.h>

void *extract_command(struct ieee80211com *ic, struct wireless_dev *wdev,
		      int *cmd_type);
int cfg80211_reply_command(struct wiphy *wiphy, int length,
			   void *data, u_int32_t flag);

#if defined(WLAN_RX_PKT_CAPTURE_ENH) || defined(WLAN_TX_PKT_CAPTURE_ENH)
/*
 * wlan_cfg80211_set_peer_pkt_capture_params:
 * Enable/disable enhanced Rx or Tx pkt capture mode for a given peer
 */
int wlan_cfg80211_set_peer_pkt_capture_params(struct wiphy *wiphy,
					      struct wireless_dev *wdev,
					      struct wlan_cfg8011_genric_params *params)
{
	struct cfg80211_context *cfg_ctx = NULL;
	struct ieee80211com *ic = NULL;
	struct net_device *dev = NULL;
	struct ol_ath_softc_net80211 *scn = NULL;
	struct ieee80211_pkt_capture_enh peer_info;
	int return_val = -EOPNOTSUPP;
	void *cmd;
	int cmd_type;

	cfg_ctx = (struct cfg80211_context *)wiphy_priv(wiphy);

	if (!cfg_ctx) {
		dp_mon_err("Invalid context");
		return -EINVAL;
	}

	ic = cfg_ctx->ic;
	if (!ic) {
		dp_mon_err("Invalid interface");
		return -EINVAL;
	}

	cmd = extract_command(ic, wdev, &cmd_type);
	if (!cmd_type) {
		dp_mon_err("Command on invalid interface");
		return -EINVAL;
	}

	if (!params->data || (!params->data_len)) {
		dp_mon_err("%s: Invalid data length %d ", __func__,
			params->data_len);
		return -EINVAL;
	}

	if (params->data_len != QDF_MAC_ADDR_SIZE) {
		dp_mon_err("Invalid MAC address!");
		return -EINVAL;
	}

	qdf_mem_copy(peer_info.peer_mac, params->data, QDF_MAC_ADDR_SIZE);
	peer_info.rx_pkt_cap_enable = !!params->value;
	peer_info.tx_pkt_cap_enable = params->length;

	dp_mon_debug("Peer Pkt Capture: rx_enable = %u, "
		  "tx_enable = %u, mac = %02x:%02x:%02x:%02x:%02x:%02x\n",
		  peer_info.rx_pkt_cap_enable, peer_info.tx_pkt_cap_enable,
		  peer_info.peer_mac[0], peer_info.peer_mac[1],
		  peer_info.peer_mac[2], peer_info.peer_mac[3],
		  peer_info.peer_mac[4], peer_info.peer_mac[5]);

	if (ic->ic_wdev.netdev == wdev->netdev) {
		dev = wdev->netdev;
		scn =  ath_netdev_priv(dev);

		if (ic->ic_cfg80211_radio_handler.ic_set_peer_pkt_capture_params)
			return_val =
				ic->ic_cfg80211_radio_handler.ic_set_peer_pkt_capture_params((void *)scn,
											     &peer_info);
	} else {
			return_val = -EOPNOTSUPP;
	}
	return return_val;
}
#else
int wlan_cfg80211_set_peer_pkt_capture_params(struct wiphy *wiphy,
					      struct wireless_dev *wdev,
					      struct wlan_cfg8011_genric_params *params)
{
	return -EINVAL;
}
#endif /* WLAN_RX_PKT_CAPTURE_ENH || WLAN_TX_PKT_CAPTURE_ENH */

#ifdef QCA_UNDECODED_METADATA_SUPPORT
/* Allowed max phyrx error code are from 0 to 63.
 * Each bit in ic_undecoded_metadata_phyrx_error_mask
 * corresponding to specific error code.
 */
#define PHYRX_ERROR_MASK_CONTINUE 0x3FFFFF
int
wlan_cfg80211_set_phyrx_error_mask(struct wiphy *wiphy,
				   struct wireless_dev *wdev,
				   struct wlan_cfg8011_genric_params *params)
{
	struct cfg80211_context *cfg_ctx = NULL;
	struct ieee80211com *ic = NULL;
	struct net_device *dev = NULL;
	struct ol_ath_softc_net80211 *scn = NULL;
	int ret = -EOPNOTSUPP;
	void *cmd;
	int cmd_type;
	uint32_t mask = 0, mask_cont = 0;
	uint32_t *data = (uint32_t *)params->data;

	if (!data) {
		dp_mon_err("Invalid Arguments");
		return -EINVAL;
	}

	cfg_ctx = (struct cfg80211_context *)wiphy_priv(wiphy);
	if (!cfg_ctx) {
		dp_mon_err("Invalid context");
		return -EINVAL;
	}

	ic = cfg_ctx->ic;
	if (!ic) {
		dp_mon_err("Invalid interface");
		return -EINVAL;
	}

	cmd = extract_command(ic, wdev, &cmd_type);
	if (!cmd_type) {
		dp_mon_err("Command on invalid interface");
		return -EINVAL;
	}

	mask = (uint32_t)params->value;
	mask_cont = data[0];

	if (ic->ic_wdev.netdev == wdev->netdev) {
		dev = wdev->netdev;
		scn =  ath_netdev_priv(dev);

		ret = ol_ath_set_phyrx_error_mask((void *)scn, mask,
						  mask_cont);
		if (!ret) {
			ic->ic_phyrx_error_mask = mask;
			ic->ic_phyrx_error_mask_cont = mask_cont;
		} else {
			dp_mon_err("Failed to config error mask");
		}
	} else {
		ret = -EOPNOTSUPP;
	}

	dp_mon_info("Configured mask(31-0): 0x%x mask(53-32): 0x%x\n",
		    ic->ic_phyrx_error_mask, ic->ic_phyrx_error_mask_cont);

	return ret;
}

int
wlan_cfg80211_get_phyrx_error_mask(struct wiphy *wiphy,
				   struct wireless_dev *wdev,
				   struct wlan_cfg8011_genric_params *params)
{
#define PHYRX_ERROR_MASK_STRING 46
#define PHYRX_ERROR_MASK_ARGS 2
	struct cfg80211_context *cfg_ctx = NULL;
	struct ieee80211com *ic = NULL;
	struct net_device *dev = NULL;
	struct ol_ath_softc_net80211 *scn = NULL;
	int cmd_type, ret = 0;
	void *cmd;
	int val[PHYRX_ERROR_MASK_ARGS] = {0};
	char string[PHYRX_ERROR_MASK_STRING];
	uint32_t mask = 0, mask_cont = 0;

	cfg_ctx = (struct cfg80211_context *)wiphy_priv(wiphy);
	ic = cfg_ctx->ic;

	cmd = extract_command(ic, wdev, &cmd_type);

	if (!cmd_type) {
		qdf_err(" Command on Invalid interface");
		return -EINVAL;
	}

	if (ic->ic_wdev.netdev == wdev->netdev) {
		dev = wdev->netdev;
		scn =  ath_netdev_priv(dev);

		ret = ol_ath_get_phyrx_error_mask((void *)scn, &mask,
				&mask_cont);
		if (ret)
			dp_mon_err("Failed to get error mask");
	} else {
		ret = -EOPNOTSUPP;
	}

	val[0] = mask;
	val[1] = mask_cont;

	snprintf(string, PHYRX_ERROR_MASK_STRING,
		 "mask(31-0):0x%08X mask(53-32):0x%08X", val[0], val[1]);

	cfg80211_reply_command(wiphy, sizeof(string), string, 0);

	return ret;
}
#else
int
wlan_cfg80211_set_phyrx_error_mask(struct wiphy *wiphy,
				   struct wireless_dev *wdev,
				   struct wlan_cfg8011_genric_params *params)
{
	return -EINVAL;
}

int
wlan_cfg80211_get_phyrx_error_mask(struct wiphy *wiphy,
				   struct wireless_dev *wdev,
				   struct wlan_cfg8011_genric_params *params)
{
	return -EINVAL;
}
#endif /* QCA_UNDECODED_METADATA_SUPPORT */
