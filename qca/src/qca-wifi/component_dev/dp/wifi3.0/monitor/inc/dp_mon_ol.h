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

#ifndef _DP_MON_OL_H_
#define _DP_MON_OL_H_

#if defined(WLAN_TX_PKT_CAPTURE_ENH) || defined(WLAN_RX_PKT_CAPTURE_ENH)
int ol_ath_ucfg_set_peer_pkt_capture(void *vscn,
				     struct ieee80211_pkt_capture_enh *peer_info);
#endif /* WLAN_TX_PKT_CAPTURE_ENH || WLAN_RX_PKT_CAPTURE_ENH */
#ifdef QCA_UNDECODED_METADATA_SUPPORT
int ol_ath_set_phyrx_error_mask(void *vscn, uint32_t mask, uint32_t mask_cont);
int ol_ath_get_phyrx_error_mask(void *vscn, uint32_t *mask,
				uint32_t *mask_cont);
#endif

#if QCN_IE
int ol_ath_set_bpr_wifi3(struct ol_ath_softc_net80211 *scn, int val);
#endif

struct mon_ops {
	int (*mon_ath_set_bpr_wifi3)(struct ol_ath_softc_net80211 *scn,
				     int val);
	int (*mon_set_lite_mode)(struct ol_ath_softc_net80211 *scn,
				 uint8_t pdev_id,
				 ol_txrx_soc_handle soc_txrx_handle,
				 int val);
	int (*mon_cfg80211_set_peer_pkt_capture_params)(struct wiphy *wiphy,
							struct wireless_dev *wdev,
							struct wlan_cfg8011_genric_params *params);
	int (*mon_set_tx_sniffer_mode)(struct ol_ath_softc_net80211 *scn,
				       uint8_t pdev_id,
				       ol_txrx_soc_handle soc_txrx_handle);
#ifdef QCA_UNDECODED_METADATA_SUPPORT
	int (*mon_set_undeocded_metadata_capture)
		(struct ol_ath_softc_net80211 *scn, uint8_t pdev_id,
		 ol_txrx_soc_handle soc_txrx_handle, int val);
	int (*mon_cfg80211_set_phyrx_error_mask)(struct wiphy *wiphy,
						 struct wireless_dev *wdev,
						 struct wlan_cfg8011_genric_params *params);
	int (*mon_cfg80211_get_phyrx_error_mask)(struct wiphy *wiphy,
						 struct wireless_dev *wdev,
						 struct wlan_cfg8011_genric_params *params);
#endif
};

#ifdef QCA_UNDECODED_METADATA_SUPPORT
static inline int
monitor_ol_ath_set_undeocded_metadata_capture(struct ol_ath_softc_net80211 *scn,
					      uint8_t pdev_id,
					      ol_txrx_soc_handle soc_txrx_handle,
					      int val)
{
	ol_ath_soc_softc_t *soc;

	if (!scn || !scn->soc)
		return -EOPNOTSUPP;

	soc = scn->soc;
	if (!soc->soc_mon_ops ||
	    !soc->soc_mon_ops->mon_set_undeocded_metadata_capture)
		return -EOPNOTSUPP;

	return soc->soc_mon_ops->mon_set_undeocded_metadata_capture(scn,
			pdev_id, soc_txrx_handle, val);
}

static inline int
monitor_cfg80211_set_phyrx_error_mask(struct wiphy *wiphy,
				      struct wireless_dev *wdev,
				      struct wlan_cfg8011_genric_params *params)
{
	struct ol_ath_softc_net80211 *scn =  ath_netdev_priv(wdev->netdev);
	ol_ath_soc_softc_t *soc;

	if (!scn || !scn->soc)
		return -EOPNOTSUPP;

	soc = scn->soc;
	if (!soc->soc_mon_ops ||
	    !soc->soc_mon_ops->mon_cfg80211_set_phyrx_error_mask)
		return -EOPNOTSUPP;

	return soc->soc_mon_ops->mon_cfg80211_set_phyrx_error_mask(wiphy,
								   wdev,
								   params);
}

static inline int
monitor_cfg80211_get_phyrx_error_mask(struct wiphy *wiphy,
				      struct wireless_dev *wdev,
				      struct wlan_cfg8011_genric_params *params)
{
	struct ol_ath_softc_net80211 *scn =  ath_netdev_priv(wdev->netdev);
	ol_ath_soc_softc_t *soc;

	if (!scn || !scn->soc)
		return -EOPNOTSUPP;

	soc = scn->soc;
	if (!soc->soc_mon_ops ||
	    !soc->soc_mon_ops->mon_cfg80211_get_phyrx_error_mask)
		return -EOPNOTSUPP;

	return soc->soc_mon_ops->mon_cfg80211_get_phyrx_error_mask(wiphy,
								   wdev,
								   params);
}
#endif

static inline
int monitor_ol_ath_set_tx_sniffer_mode(struct ol_ath_softc_net80211 *scn,
				       uint8_t pdev_id,
				       ol_txrx_soc_handle soc_txrx_handle)
{
	ol_ath_soc_softc_t *soc;

	if (!scn || !scn->soc)
		return -EOPNOTSUPP;

	soc = scn->soc;
	if (!soc->soc_mon_ops ||
	    !soc->soc_mon_ops->mon_cfg80211_set_peer_pkt_capture_params)
		return -EOPNOTSUPP;

	return soc->soc_mon_ops->mon_set_tx_sniffer_mode(scn,
							 pdev_id,
							 soc_txrx_handle);
}

static inline int
monitor_cfg80211_set_peer_pkt_capture_params(struct wiphy *wiphy,
					     struct wireless_dev *wdev,
					     struct wlan_cfg8011_genric_params *params)
{
	struct ol_ath_softc_net80211 *scn =  ath_netdev_priv(wdev->netdev);
	ol_ath_soc_softc_t *soc;

	if (!scn || !scn->soc)
		return -EOPNOTSUPP;

	soc = scn->soc;

	if (!soc->soc_mon_ops ||
	    !soc->soc_mon_ops->mon_cfg80211_set_peer_pkt_capture_params)
		return -EOPNOTSUPP;

	return soc->soc_mon_ops->mon_cfg80211_set_peer_pkt_capture_params(wiphy,
									  wdev,
									  params);
}

static inline
int monitor_ol_ath_set_lite_mode(struct ol_ath_softc_net80211 *scn,
				 uint8_t pdev_id,
				 ol_txrx_soc_handle soc_txrx_handle,
				 int val)
{
	ol_ath_soc_softc_t *soc;

	if (!scn || !scn->soc)
		return -EOPNOTSUPP;

	soc = scn->soc;
	if (!soc->soc_mon_ops ||
	    !soc->soc_mon_ops->mon_set_lite_mode)
		return -EOPNOTSUPP;

	return soc->soc_mon_ops->mon_set_lite_mode(scn, pdev_id,
						   soc_txrx_handle, val);
}

static inline void
monitor_ol_ath_set_bpr_wifi3(struct ol_ath_softc_net80211 *scn,
			     int val)
{
	ol_ath_soc_softc_t *soc;

	if (!scn || !scn->soc)
		return;

	soc = scn->soc;
	if (!soc->soc_mon_ops ||
	    !soc->soc_mon_ops->mon_ath_set_bpr_wifi3)
		return;

	soc->soc_mon_ops->mon_ath_set_bpr_wifi3(scn, val);
}

#endif /* _DP_MON_OL_H_ */
