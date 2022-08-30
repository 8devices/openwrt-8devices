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
#include <qdf_util.h>
#include <wlan_osif_priv.h>
#include "ieee80211_radiotap.h"
#include <ieee80211_cfg80211.h>
#include <ieee80211_var.h>
#include <ieee80211_bsscolor.h>
#include <ol_if_athvar.h>
#include <dp_mon_ol.h>
#include <cdp_txrx_cmn.h>
#include <cdp_txrx_cmn_struct.h>
#include <wlan_objmgr_psoc_obj.h>
#include <qdf_nbuf.h>
#include <cfg_dp.h>
#include <init_deinit_lmac.h>
#include <cdp_txrx_ctrl.h>
#include <cfg_ucfg_api.h>
#include <target_type.h>
#include <target_if.h>
#ifdef QCA_NSS_WIFI_OFFLOAD_SUPPORT
#include <osif_nss_wifiol_if.h>
#endif
#include <dp_mon.h>

#define _HT_SGI_PRESENT 0x80

void monitor_osif_process_rx_mpdu(osif_dev *osifp, qdf_nbuf_t mpdu_ind);
void monitor_osif_deliver_tx_capture_data(osif_dev *osifp, struct sk_buff *skb);
int wlan_cfg80211_set_peer_pkt_capture_params(struct wiphy *wiphy,
					      struct wireless_dev *wdev,
					      struct wlan_cfg8011_genric_params *params);
#ifdef QCA_UNDECODED_METADATA_SUPPORT
int wlan_cfg80211_set_phyrx_error_mask(struct wiphy *wiphy,
				       struct wireless_dev *wdev,
				       struct wlan_cfg8011_genric_params *params);
int wlan_cfg80211_get_phyrx_error_mask(struct wiphy *wiphy,
				       struct wireless_dev *wdev,
				       struct wlan_cfg8011_genric_params *params);
void monitor_osif_deliver_rx_capture_undecoded_metadata(osif_dev *osifp,
							struct sk_buff *skb);
#define DEBUG_SNIFFER_TEST_RX_UNDEOCDED_FRAME_CAPTURE     "UNDECO"
#endif
#define DEBUG_SNIFFER_SIGNATURE_LEN 6
/*
 * expected values for filter (val) 0, 1, 2, 4, 8
 * these values could be for FP, MO, or both by using first 16 bits.
 * i.e. 0x10004 (FP only enable with filter 4 on)
 */
static int ol_ath_set_rx_monitor_filter(struct ieee80211com *ic)
{
	struct wlan_objmgr_psoc *psoc = wlan_pdev_get_psoc(ic->ic_pdev_obj);
	struct cdp_monitor_filter filter_val;
	ol_txrx_soc_handle soc_txrx_handle;
	uint64_t val = ic->ic_os_monrxfilter;
	uint32_t filter_mode;

	soc_txrx_handle = wlan_psoc_get_dp_handle(psoc);
	filter_val.mode = RX_MON_FILTER_PASS | RX_MON_FILTER_OTHER;

	filter_mode = FILTER_MODE(val);
	if (filter_mode == FILTER_PASS_ONLY)
		filter_val.mode = RX_MON_FILTER_PASS;
	else if (filter_mode == MONITOR_OTHER_ONLY)
		filter_val.mode = RX_MON_FILTER_OTHER;

	if (filter_val.mode & RX_MON_FILTER_PASS) {
		filter_val.fp_mgmt = SET_MON_FILTER_MGMT(val);
		filter_val.fp_ctrl = SET_MON_FILTER_CTRL(val);
		filter_val.fp_data = SET_MON_FILTER_DATA(val);
	}
	if (filter_val.mode & RX_MON_FILTER_OTHER) {
		filter_val.mo_mgmt = SET_MON_FILTER_MGMT(val);
		filter_val.mo_ctrl = SET_MON_FILTER_CTRL(val);
		filter_val.mo_data = SET_MON_FILTER_DATA(val);
	}

	return cdp_set_monitor_filter(soc_txrx_handle,
				      wlan_objmgr_pdev_get_pdev_id(ic->ic_pdev_obj),
				      &filter_val);
}

static inline
void set_rate_a(struct mon_rx_status *rx_status)
{
	switch (rx_status->mcs) {
	case CDP_LEGACY_MCS0:
		rx_status->rate = CDP_11A_RATE_0MCS;
		break;
	case CDP_LEGACY_MCS1:
		rx_status->rate = CDP_11A_RATE_1MCS;
		break;
	case CDP_LEGACY_MCS2:
		rx_status->rate = CDP_11A_RATE_2MCS;
		break;
	case CDP_LEGACY_MCS3:
		rx_status->rate = CDP_11A_RATE_3MCS;
		break;
	case CDP_LEGACY_MCS4:
		rx_status->rate = CDP_11A_RATE_4MCS;
		break;
	case CDP_LEGACY_MCS5:
		rx_status->rate = CDP_11A_RATE_5MCS;
		break;
	case CDP_LEGACY_MCS6:
		rx_status->rate = CDP_11A_RATE_6MCS;
		break;
	case CDP_LEGACY_MCS7:
		rx_status->rate = CDP_11A_RATE_7MCS;
		break;
	default:
		break;
	}
}

static inline
void set_rate_b(struct mon_rx_status *rx_status)
{
	switch (rx_status->mcs) {
	case CDP_LEGACY_MCS0:
		rx_status->rate = CDP_11B_RATE_0MCS;
		break;
	case CDP_LEGACY_MCS1:
		rx_status->rate = CDP_11B_RATE_1MCS;
		break;
	case CDP_LEGACY_MCS2:
		rx_status->rate = CDP_11B_RATE_2MCS;
		break;
	case CDP_LEGACY_MCS3:
		rx_status->rate = CDP_11B_RATE_3MCS;
		break;
	case CDP_LEGACY_MCS4:
		rx_status->rate = CDP_11B_RATE_4MCS;
		break;
	case CDP_LEGACY_MCS5:
		rx_status->rate = CDP_11B_RATE_5MCS;
		break;
	case CDP_LEGACY_MCS6:
		rx_status->rate = CDP_11B_RATE_6MCS;
		break;
	default:
		break;
	}
}

static inline
void radiotap_vht_setup(struct cdp_tx_indication_info *tx_info,
			struct mon_rx_status *rx_status)
{
	struct cdp_tx_completion_ppdu *ppdu_desc = tx_info->ppdu_desc;
	struct cdp_tx_completion_ppdu_user *user;
	uint8_t usr_idx;

	usr_idx = tx_info->mpdu_info.usr_idx;
	user = &ppdu_desc->user[usr_idx];

	rx_status->ldpc = user->ldpc;
	rx_status->vht_flag_values5 = user->mu_group_id;
	rx_status->is_stbc = user->stbc;
	rx_status->nss = user->nss + 1;

	rx_status->vht_flag_values3[0] = (((rx_status->mcs) << 4)
					  | rx_status->nss);
	rx_status->vht_flag_values2 = rx_status->bw;
	/* ldpc same as su_mu_conding */
	rx_status->vht_flag_values4 = rx_status->ldpc;
	rx_status->beamformed = user->txbf ? 1 : 0;
}

static inline
void radiotap_he_setup(struct cdp_tx_indication_info *tx_info,
		       struct mon_rx_status *rx_status)
{
	struct cdp_tx_completion_ppdu *ppdu_desc = tx_info->ppdu_desc;
	struct cdp_tx_completion_ppdu_user *user;
	uint32_t value;
	uint8_t usr_idx = 0;

	usr_idx = tx_info->mpdu_info.usr_idx;
	user = &ppdu_desc->user[usr_idx];

	if (user->ppdu_type == CDP_PPDU_STATS_PPDU_TYPE_SU) {
		rx_status->he_data1 = (user->he_re) ?
			QDF_MON_STATUS_HE_EXT_SU_FORMAT_TYPE :
			QDF_MON_STATUS_HE_SU_FORMAT_TYPE;
	} else if (user->ppdu_type == CDP_PPDU_STATS_PPDU_TYPE_MU_MIMO ||
		   user->ppdu_type == CDP_PPDU_STATS_PPDU_TYPE_MU_OFDMA ||
		   user->ppdu_type == CDP_PPDU_STATS_PPDU_TYPE_MU_MIMO_OFDMA) {
		rx_status->he_data1 = QDF_MON_STATUS_HE_MU_FORMAT_TYPE;
	} else if (user->ppdu_type == CDP_PPDU_STATS_PPDU_TYPE_UL_TRIG) {
		rx_status->he_data1 = QDF_MON_STATUS_HE_TRIG_FORMAT_TYPE;
	}

	if (rx_status->he_data1 != QDF_MON_STATUS_HE_MU_FORMAT_TYPE) {
		/* radiotap he_data 1 */
		rx_status->he_data1 |= QDF_MON_STATUS_HE_BSS_COLOR_KNOWN |
				QDF_MON_STATUS_HE_BEAM_CHANGE_KNOWN |
				QDF_MON_STATUS_HE_DL_UL_KNOWN |
				QDF_MON_STATUS_HE_MCS_KNOWN |
				QDF_MON_STATUS_HE_DCM_KNOWN |
				QDF_MON_STATUS_HE_CODING_KNOWN |
				QDF_MON_STATUS_HE_STBC_KNOWN |
				QDF_MON_STATUS_HE_DATA_BW_RU_KNOWN |
				QDF_MON_STATUS_HE_DOPPLER_KNOWN;

		/* radiotap he_data 2 */
		rx_status->he_data2 = QDF_MON_STATUS_HE_GI_KNOWN;
		rx_status->he_data2 |=
			QDF_MON_STATUS_TXBF_KNOWN |
			QDF_MON_STATUS_LTF_SYMBOLS_KNOWN;
	} else {
		/* radiotap he_data 1 */
		rx_status->he_data1 |=
			QDF_MON_STATUS_HE_BSS_COLOR_KNOWN |
			QDF_MON_STATUS_HE_DL_UL_KNOWN |
			QDF_MON_STATUS_HE_MCS_KNOWN |
			QDF_MON_STATUS_HE_CODING_KNOWN |
			QDF_MON_STATUS_HE_STBC_KNOWN |
			QDF_MON_STATUS_HE_DATA_BW_RU_KNOWN |
			QDF_MON_STATUS_HE_DOPPLER_KNOWN;
		/* radiotap he_data 2 */
		rx_status->he_data2 = QDF_MON_STATUS_HE_GI_KNOWN;
		rx_status->he_data2 |= QDF_MON_STATUS_LTF_SYMBOLS_KNOWN;
	}

	/* radiotap he_data 3 */
	value = ppdu_desc->bss_color; /* bss color host value */
	rx_status->he_data3 = value;
	value = ppdu_desc->beam_change; /* beam change fw value */
	rx_status->he_data3 |= value << QDF_MON_STATUS_BEAM_CHANGE_SHIFT;
	value = 0; /* UL/DL... DL value */
	rx_status->he_data3 |= value << QDF_MON_STATUS_DL_UL_SHIFT;
	value = user->mcs; /* mcs fw value */
	rx_status->he_data3 |= value  << QDF_MON_STATUS_TRANSMIT_MCS_SHIFT;
	value = user->dcm; /* dcm fw value */
	rx_status->he_data3 |= value  << QDF_MON_STATUS_DCM_SHIFT;
	value = user->ldpc; /* ldpc fw value */
	rx_status->he_data3 |= value << QDF_MON_STATUS_CODING_SHIFT;
	value = user->stbc; /* stbc fw value */
	rx_status->he_data3 |= value << QDF_MON_STATUS_STBC_SHIFT;

	/* radiotap he_data 4 */
	if (user->ppdu_type == CDP_PPDU_STATS_PPDU_TYPE_SU)
	    rx_status->he_data4 = ppdu_desc->spatial_reuse;

	/* radiotap he_data 5 */
	rx_status->he_data5 = user->bw;
	value = user->gi;
	rx_status->he_data5 |= value << QDF_MON_STATUS_GI_SHIFT;
	value = user->ltf_size;
	rx_status->he_data5 |= value << QDF_MON_STATUS_HE_LTF_SIZE_SHIFT;
	value = user->txbf;
	rx_status->he_data5 |= value << QDF_MON_STATUS_TXBF_SHIFT;

	/* radiotap he_data 6 */
	rx_status->he_data6 = user->nss + 1;
	value = ppdu_desc->doppler;
	rx_status->he_data6 |= value << QDF_MON_STATUS_DOPPLER_SHIFT;
}

/**
 * convert_tx_to_rx_stats(): Function to update mpdu info
 * from ppdu_desc
 * @ppdu_id: ppdu_id
 * @mpdu_info: cdp_tx_indication_mpdu_info
 * @user: cdp_tx_completion_ppdu_user
 *
 * return: void
 */
static inline
QDF_STATUS convert_tx_to_rx_stats(struct cdp_tx_indication_info *tx_info,
				  struct mon_rx_status *rx_status)
{
	struct cdp_tx_indication_mpdu_info *m_info = &tx_info->mpdu_info;
	uint8_t usr_idx = 0;

	usr_idx = m_info->usr_idx;
	rx_status->tsft = m_info->ppdu_start_timestamp;
	rx_status->chan_num = m_info->channel_num;
	rx_status->chan_freq = m_info->channel;
	rx_status->ppdu_id = m_info->ppdu_id;
	rx_status->rssi_comb = m_info->ack_rssi;
	rx_status->tid = m_info->tid;
	rx_status->frame_control_info_valid = m_info->frame_ctrl;
	rx_status->nss = m_info->nss;
	rx_status->sgi = m_info->gi;
	rx_status->mcs = m_info->mcs;
	rx_status->preamble_type = m_info->preamble;
	rx_status->bw = m_info->bw;
	rx_status->beamformed = m_info->txbf;
	rx_status->ldpc = m_info->ldpc;
	rx_status->ofdm_flag = 1;

	if ((rx_status->preamble_type != DOT11_A) &&
	    (rx_status->preamble_type != DOT11_B)) {
		if (tx_info->ppdu_desc->user[usr_idx].is_ampdu == 1)
			rx_status->rs_flags |= IEEE80211_AMPDU_FLAG;
	}

	switch (rx_status->preamble_type) {
	case DOT11_B:
		rx_status->ofdm_flag = 0;
		rx_status->cck_flag = 1;
		set_rate_b(rx_status);
		break;
	case DOT11_N:
		rx_status->ht_flags = 1;
		rx_status->rtap_flags |= _HT_SGI_PRESENT;
		break;
	case DOT11_AC:
		rx_status->vht_flags = 1;
		radiotap_vht_setup(tx_info, rx_status);
		break;
	case DOT11_AX:
		rx_status->he_flags = 1;
		radiotap_he_setup(tx_info, rx_status);
		break;
	default:
		set_rate_a(rx_status);
		break;
	};

	return QDF_STATUS_SUCCESS;
}

/* size of struct rx_pkt_tlv */
#define RX_PADDING_SIZE 384
/**
 * ol_ath_process_tx_frames() - Callback registered for WDI_EVENT_TX_DATA
 * @pdev_hdl: pdev pointer
 * @event: WDi event
 * @data: skb received
 * @peer_id: peer_id
 * @status: status from event
 *
 * The MSDU payload is appended with a tx capture identifier containing ppdu_id,
 * tx and rx address and is sent to this function.
 *
 * @Return: None
 */
static void ol_ath_process_tx_frames(void *pdev_hdl, enum WDI_EVENT event,
				     void *data, u_int16_t peer_id,
				     uint32_t status)
{
	qdf_nbuf_t skb = NULL;
	struct wlan_objmgr_pdev *pdev_obj =
		(struct wlan_objmgr_pdev *)pdev_hdl;
	struct ieee80211com *ic = wlan_pdev_get_mlme_ext_obj(pdev_obj);
	struct ieee80211vap *vap;
	osif_dev  *osifp;
	struct cdp_tx_indication_info *ptr_tx_info;
	struct mon_rx_status rx_status = {0};

	ptr_tx_info = (struct cdp_tx_indication_info *)data;

	if (!ptr_tx_info->mpdu_nbuf)
		return;

	if (!ic) {
		qdf_nbuf_free(ptr_tx_info->mpdu_nbuf);
		dp_mon_debug("ic is NULL");
		return;
	}

	vap = (struct ieee80211vap *)ic->ic_mon_vap;
	if (!vap) {
		qdf_nbuf_free(ptr_tx_info->mpdu_nbuf);
		ptr_tx_info->mpdu_nbuf = NULL;
		dp_mon_debug("No monitor vap created, to dump skb");
		return;
	}

	/**
	 * We are currently forwarding the buffer to user space or freeing the
	 * buffer based on the feature.
	 * Have callback here to extract the data if required.
	 */
	if (!vap->iv_lite_monitor && (ol_ath_is_mcopy_enabled(ic))) {
		qdf_nbuf_free(ptr_tx_info->mpdu_nbuf);
		ptr_tx_info->mpdu_nbuf = NULL;
		return;
	}

	osifp = (osif_dev *)vap->iv_ifp;

	dp_mon_info("ppdu_id[%d] frm_type[%d] [%p]sending to stack!!!!",
		  ptr_tx_info->mpdu_info.ppdu_id,
		  ptr_tx_info->mpdu_info.frame_type,
		  ptr_tx_info->mpdu_nbuf);

	skb = ptr_tx_info->mpdu_nbuf;

	if (!ptr_tx_info->mpdu_nbuf) {
		dp_mon_err("mpdu_nbuf is NULL!!!!");
		return;
	}

	ptr_tx_info->mpdu_nbuf = NULL;

	/* differentiate Lithium and Beryllium */
	if (!ptr_tx_info->radiotap_done) {
		/* update radiotap header */
		convert_tx_to_rx_stats(ptr_tx_info, &rx_status);
		qdf_nbuf_update_radiotap(&rx_status, skb, RX_PADDING_SIZE);
	}

	monitor_osif_deliver_tx_capture_data(osifp, skb);
}

static int ol_set_tx_sniffer_mode(struct ol_ath_softc_net80211 *scn,
				  uint8_t pdev_id,
				  ol_txrx_soc_handle soc_txrx_handle)
{
	struct ieee80211com *ic = &scn->sc_ic;

	if (ic->ic_tx_pkt_capture != TX_ENH_PKT_CAPTURE_DISABLE) {
		scn->stats_tx_data_subscriber.callback =
					ol_ath_process_tx_frames;
		scn->stats_tx_data_subscriber.context = scn->sc_pdev;
		if (cdp_wdi_event_sub(soc_txrx_handle,
				      pdev_id,
				      &scn->stats_tx_data_subscriber,
				      WDI_EVENT_TX_PKT_CAPTURE))
			return A_ERROR;
	} else {
		if (cdp_wdi_event_unsub(soc_txrx_handle,
					pdev_id,
					&scn->stats_tx_data_subscriber,
					WDI_EVENT_TX_PKT_CAPTURE))
			return A_ERROR;
	}

	return 0;
}

#if defined(WLAN_TX_PKT_CAPTURE_ENH) || defined(WLAN_RX_PKT_CAPTURE_ENH)
int
ol_ath_ucfg_set_peer_pkt_capture(void *vscn,
				 struct ieee80211_pkt_capture_enh *peer_info)
{
	struct ol_ath_softc_net80211 *scn =
			(struct ol_ath_softc_net80211 *)vscn;
	ol_txrx_soc_handle soc_handle;
	QDF_STATUS status;

#ifdef QCA_NSS_WIFI_OFFLOAD_SUPPORT
	uint32_t nss_soc_cfg = cfg_get(scn->soc->psoc_obj, CFG_NSS_WIFI_OL);

	if (nss_soc_cfg) {
		dp_mon_info("Rx/Tx Packet Capture not supported when NSS offload is enabled");
		return 0;
	}
#endif /* QCA_NSS_WIFI_OFFLOAD_SUPPORT */

	soc_handle =
		(ol_txrx_soc_handle)wlan_psoc_get_dp_handle(scn->soc->psoc_obj);
	if (!soc_handle) {
		dp_mon_err("psoc handle is NULL");
		return -EFAULT;
	}

	status =
		cdp_update_peer_pkt_capture_params(soc_handle,
						   wlan_objmgr_pdev_get_pdev_id(scn->sc_pdev),
						   peer_info->rx_pkt_cap_enable,
						   peer_info->tx_pkt_cap_enable,
						   peer_info->peer_mac);
	if (status != QDF_STATUS_SUCCESS)
		return -EINVAL;

	dp_mon_info("Set Rx & TX packet capture [%d, %d] for peer %02x:%02x:%02x:%02x:%02x:%02x",
		 peer_info->rx_pkt_cap_enable, peer_info->tx_pkt_cap_enable,
		 peer_info->peer_mac[0], peer_info->peer_mac[1],
		 peer_info->peer_mac[2], peer_info->peer_mac[3],
		 peer_info->peer_mac[4], peer_info->peer_mac[5]);

#ifdef QCA_NSS_WIFI_OFFLOAD_SUPPORT
	if (scn->sc_ic.nss_radio_ops) {
		if (peer_info->tx_pkt_cap_enable) {
			status = scn->sc_ic.nss_radio_ops->ic_nss_ol_peer_cfg_tx_capture(scn,
					peer_info->tx_pkt_cap_enable, peer_info->peer_mac);
		}
	}
	if (status != QDF_STATUS_SUCCESS) {
		qdf_err("Peer Tx capture enable for NSS offload failed for peer = " QDF_MAC_ADDR_FMT,
			QDF_MAC_ADDR_REF(peer_info->peer_mac));
		return -EINVAL;
	}
#endif  /* QCA_NSS_WIFI_OFFLOAD_SUPPORT */

	return 0;
}
#endif /* WLAN_TX_PKT_CAPTURE_ENH || WLAN_RX_PKT_CAPTURE_ENH */

#ifdef QCA_UNDECODED_METADATA_SUPPORT
int ol_ath_set_phyrx_error_mask(void *vscn, uint32_t mask, uint32_t mask_cont)
{
	struct ol_ath_softc_net80211 *scn =
			(struct ol_ath_softc_net80211 *)vscn;
	ol_txrx_soc_handle soc_txrx_handle;
	uint8_t pdev_id = wlan_objmgr_pdev_get_pdev_id(scn->sc_pdev);
	QDF_STATUS status;

	soc_txrx_handle =
		(ol_txrx_soc_handle)wlan_psoc_get_dp_handle(scn->soc->psoc_obj);
	if (!soc_txrx_handle) {
		dp_mon_err("psoc handle is NULL");
		return -EFAULT;
	}

	status =
		cdp_txrx_set_pdev_phyrx_error_mask(soc_txrx_handle, pdev_id,
						   mask, mask_cont);
	if (status != QDF_STATUS_SUCCESS) {
		dp_mon_err("Phyrx error mask configuration failed");
		return -EINVAL;
	}
	dp_mon_info("mask(0 to 31):0x%x mask(32 to 64):0x%x", mask, mask_cont);

	return 0;
}

int
ol_ath_get_phyrx_error_mask(void *vscn, uint32_t *mask, uint32_t *mask_cont)
{
	struct ol_ath_softc_net80211 *scn =
			(struct ol_ath_softc_net80211 *)vscn;
	ol_txrx_soc_handle soc_txrx_handle;
	uint8_t pdev_id = wlan_objmgr_pdev_get_pdev_id(scn->sc_pdev);
	QDF_STATUS status;

	soc_txrx_handle =
		(ol_txrx_soc_handle)wlan_psoc_get_dp_handle(scn->soc->psoc_obj);
	if (!soc_txrx_handle) {
		dp_mon_err("psoc handle is NULL");
		return -EFAULT;
	}

	status =
		cdp_txrx_get_pdev_phyrx_error_mask(soc_txrx_handle, pdev_id,
						   mask, mask_cont);
	if (status != QDF_STATUS_SUCCESS) {
		dp_mon_err("Get configured phyrx error mask failed");
		return -EINVAL;
	}

	return 0;
}
#endif /* QCA_UNDECODED_METADATA_SUPPORT */

#ifdef WLAN_RX_PKT_CAPTURE_ENH
QDF_STATUS
convert_mpdu_info_to_stats(struct cdp_rx_indication_mpdu_info *mpdu_info,
			   struct mon_rx_status *rx_status)
{
	A_MEMZERO(rx_status, sizeof(struct mon_rx_status));
	rx_status->tsft = mpdu_info->timestamp;
	rx_status->rs_fcs_err = mpdu_info->fcs_err;
	rx_status->chan_num = mpdu_info->channel;
	rx_status->chan_freq = mpdu_info->chan_freq;
	rx_status->ppdu_id = mpdu_info->ppdu_id;
	rx_status->rssi_comb = mpdu_info->rssi_comb;
	rx_status->chan_noise_floor = mpdu_info->nf;

	rx_status->rate = mpdu_info->rate;
	rx_status->nss = mpdu_info->nss;
	rx_status->sgi = mpdu_info->gi;
	rx_status->mcs = mpdu_info->mcs;
	rx_status->preamble_type = mpdu_info->preamble;
	rx_status->bw = mpdu_info->bw;
	rx_status->ldpc = mpdu_info->ldpc;

	/*testing pupose*/
	if (mpdu_info->preamble == DOT11_N ||
	    mpdu_info->preamble == DOT11_AC ||
	    mpdu_info->preamble == DOT11_AX) {
	    rx_status->ht_flags = 1;
	    rx_status->ht_mcs = rx_status->mcs;
	    rx_status->rtap_flags |= 0x80;
	}
	rx_status->l_sig_a_info = mpdu_info->preamble |
		rx_status->ppdu_id << 16;
	rx_status->l_sig_b_info = mpdu_info->ppdu_type;
	rx_status->device_id = rx_status->nss |
	mpdu_info->mu_ul_info_valid << 8 |
	mpdu_info->ofdma_ru_start_index << 16 |
	mpdu_info->ofdma_ru_width << 24;
	return QDF_STATUS_SUCCESS;
}

static inline void
ol_if_process_rx_mpdu(struct ol_ath_softc_net80211 *scn, enum WDI_EVENT event,
		      void *data, u_int16_t peer_id)
{
	struct ieee80211com *ic  = &scn->sc_ic;
	struct ieee80211vap *vap = NULL;
	osif_dev  *osifp = NULL;
	struct cdp_rx_indication_mpdu *rx_ind_mpdu =
			(struct cdp_rx_indication_mpdu *)data;
	qdf_nbuf_t mpdu_ind;
	struct cdp_rx_indication_mpdu_info *mpdu_info =
			&rx_ind_mpdu->mpdu_info;
	struct mon_rx_status rx_status;

	mpdu_ind = rx_ind_mpdu->nbuf;

	if (!mpdu_ind)
		return;

	if (!ic) {
		qdf_nbuf_free(mpdu_ind);
		qdf_info("ic is NULL");
		return;
	}

	vap = ic->ic_mon_vap;

	if (!vap) {
		qdf_nbuf_free(mpdu_ind);
		return;
	}

	convert_mpdu_info_to_stats(mpdu_info, &rx_status);
	qdf_nbuf_update_radiotap(&rx_status, mpdu_ind, RADIOTAP_HEADER_LEN);

	osifp = (osif_dev *)vap->iv_ifp;

	monitor_osif_process_rx_mpdu(osifp, mpdu_ind);
}

void process_rx_mpdu(void *pdev, enum WDI_EVENT event, void *data,
		     u_int16_t peer_id, enum htt_cmn_rx_status status)
{
	struct ol_ath_softc_net80211 *scn =
			(struct ol_ath_softc_net80211 *)pdev;

	ol_if_process_rx_mpdu(scn, event, data, peer_id);
}

static int ol_set_lite_mode(struct ol_ath_softc_net80211 *scn,
			    uint8_t pdev_id,
			    ol_txrx_soc_handle soc_txrx_handle,
			    int value)
{
	cdp_config_param_type val = {0};
	struct ieee80211com *ic = &scn->sc_ic;

	val.cdp_pdev_param_en_tx_cap = value;
	if (QDF_STATUS_SUCCESS !=
	    cdp_txrx_set_pdev_param(soc_txrx_handle, pdev_id,
				    CDP_CONFIG_ENH_RX_CAPTURE, val))
		return -EINVAL;
	if (ic->ic_rx_mon_lite == RX_ENH_CAPTURE_DISABLED &&
	    value != RX_ENH_CAPTURE_DISABLED) {
		scn->soc->scn_rx_lite_monitor_mpdu_subscriber.callback
				= process_rx_mpdu;
		scn->soc->scn_rx_lite_monitor_mpdu_subscriber.context  = scn;
		cdp_wdi_event_sub(soc_txrx_handle, pdev_id,
				  &scn->soc->scn_rx_lite_monitor_mpdu_subscriber,
				  WDI_EVENT_RX_MPDU);
	} else if (ic->ic_rx_mon_lite != RX_ENH_CAPTURE_DISABLED &&
		   value == RX_ENH_CAPTURE_DISABLED) {
		scn->soc->scn_rx_lite_monitor_mpdu_subscriber.context  = scn;
		cdp_wdi_event_unsub(soc_txrx_handle, pdev_id,
				    &scn->soc->scn_rx_lite_monitor_mpdu_subscriber,
				    WDI_EVENT_RX_MPDU);
	}
	ic->ic_rx_mon_lite = value;

	return EOK;
}
#else
static int ol_set_lite_mode(struct ol_ath_softc_net80211 *scn,
			    uint8_t pdev_id,
			    ol_txrx_soc_handle soc_txrx_handle,
			    int value)
{
	return EOK;
}
#endif

static inline QDF_STATUS
ol_ath_config_full_mon_support(struct ol_ath_soc_softc *soc, uint8_t val)
{
	int target_type = lmac_get_tgt_type(soc->psoc_obj);

	if (target_type != TARGET_TYPE_QCN9000) {
		dp_mon_err("Full mon mode is not supported for target_type: %d",
			target_type);
		return QDF_STATUS_SUCCESS;
	}

	if (!cfg_get(soc->psoc_obj, CFG_DP_FULL_MON_MODE)) {
		dp_mon_err("Full monitor is not supported");
		return QDF_STATUS_E_FAILURE;
	}

	if (val) {
		if (soc->full_mon_mode_support) {
			dp_mon_debug("Full monitor mode support is already enabled");
			return QDF_STATUS_E_FAILURE;
		}
	} else {
		if (!soc->full_mon_mode_support) {
			dp_mon_debug("Full monitor mode support is already disabled");
			return QDF_STATUS_E_FAILURE;
		}
	}
	soc->full_mon_mode_support = val;
	cdp_soc_config_full_mon_mode(wlan_psoc_get_dp_handle(soc->psoc_obj),
				     val);
	dp_mon_info("Full monitor mode configured for qcn9000 target_type: %d val: %d ",
		 target_type, val);

	return QDF_STATUS_SUCCESS;
}

#if QCN_IE
#define    IEEE80211_MACHDR_OFFSET	4
#endif
#if QCN_IE
/**
 * ol_ath_process_offload_beacon() - Callback registered for WDI_EVENT_TX_BEACON
 * @pdev_hdl: pdev pointer
 * @event:	 WDI event
 * @data:	 skb received
 * @peer_id: peer_id
 * @status:  status from event
 *
 * @Return: None
 */
static void ol_ath_process_offload_beacon(void *pdev_hdl, enum WDI_EVENT event,
					  void *data,
					  u_int16_t peer_id, uint32_t status)
{
	qdf_nbuf_t skb = (qdf_nbuf_t)data;
	struct wlan_objmgr_pdev *pdev_obj = (struct wlan_objmgr_pdev *)pdev_hdl;
	struct ieee80211com *ic = wlan_pdev_get_mlme_ext_obj(pdev_obj);
	struct ieee80211_node *ni;
	struct ieee80211vap *vap = NULL;
	qdf_hrtimer_data_t *bpr_hrtimer = NULL;
	struct ieee80211_frame *hdr;
	uint8_t *frm = NULL;

	if (!ic) {
		qdf_nbuf_free(skb);
		qdf_debug(" ic is NULL");
		return;
	}
	frm = (uint8_t *)qdf_nbuf_data(skb);
	/* Add IEEE80211_MACHDR_OFFSET to frm to point to ieee80211 beacon flags*/
	frm = frm + IEEE80211_MACHDR_OFFSET;
	hdr = (struct ieee80211_frame *)frm;
	if ((hdr->i_fc[0] & (IEEE80211_FC0_TYPE_MASK |
			     IEEE80211_FC0_SUBTYPE_MASK)) ==
	    (IEEE80211_FC0_TYPE_MGT | IEEE80211_FC0_SUBTYPE_BEACON)) {
		ni = ieee80211_find_node(ic, hdr->i_addr2, WLAN_MLME_SB_ID);
		if (ni) {
			vap = ni->ni_vap;
			if (!vap) {
				ieee80211_free_node(ni, WLAN_MLME_SB_ID);
				if ((ic->ic_debug_sniffer == SNIFFER_DISABLE) &&
				    (ic->ic_tx_pkt_capture ==
				     TX_ENH_PKT_CAPTURE_DISABLE)) {
					qdf_nbuf_free(skb);
				}
				return;
			}
			if (ni == vap->iv_bss) {
				bpr_hrtimer = &vap->bpr_timer;
				if (qdf_hrtimer_active(bpr_hrtimer)) {
					/* Cancel the timer as beacon is sent
					 *instead of a broadcast response
					 */
					qdf_hrtimer_cancel(bpr_hrtimer);
					vap->iv_bpr_timer_cancel_count++;

					IEEE80211_DPRINTF(vap,
							  IEEE80211_MSG_MLME,
							  "Cancel timer: %s| %d | Delay: %d | Current time: %lld | "
							  "Next beacon tstamp: %lld | beacon interval: %d ms | "
							  "Timer cb: %d | Enqueued: %d\n",
							  __func__, __LINE__, vap->iv_bpr_delay,
							  qdf_ktime_to_ns(qdf_ktime_get()),
							  qdf_ktime_to_ns(vap->iv_next_beacon_tstamp),
							  ic->ic_intval,
							  qdf_hrtimer_callback_running(bpr_hrtimer),
							  qdf_hrtimer_is_queued(bpr_hrtimer));
				}

				/* Calculate the next beacon timestamp */
				vap->iv_next_beacon_tstamp =
					qdf_ktime_add_ns(qdf_ktime_get(),
							 ic->ic_intval * QDF_NSEC_PER_MSEC);
			}
			/**
			 * We are currently freeing the buffer based on vap->iv_lite_monitor flag.
			 * In monitor mode, the buffer will be handeled by ol_ath_process_tx_nondata_frames.
			 */
			if (ic->ic_debug_sniffer == SNIFFER_DISABLE &&
			    ic->ic_tx_pkt_capture == TX_ENH_PKT_CAPTURE_DISABLE)
				qdf_nbuf_free(skb);

			ieee80211_free_node(ni, WLAN_MLME_SB_ID);
		} else {
			if (ic->ic_debug_sniffer == SNIFFER_DISABLE &&
			    ic->ic_tx_pkt_capture == TX_ENH_PKT_CAPTURE_DISABLE)
				qdf_nbuf_free(skb);
		}
	} else {
		if (ic->ic_debug_sniffer == SNIFFER_DISABLE &&
		    ic->ic_tx_pkt_capture == TX_ENH_PKT_CAPTURE_DISABLE)
			qdf_nbuf_free(skb);
	}
}
#endif

#if QCN_IE
int ol_ath_set_bpr_wifi3(struct ol_ath_softc_net80211 *scn, int val)
{
	struct ieee80211com *ic = &scn->sc_ic;
	ol_txrx_soc_handle soc_txrx_handle;
	uint8_t pdev_id = wlan_objmgr_pdev_get_pdev_id(scn->sc_pdev);
	cdp_config_param_type value = {0};

	if (ol_ath_is_beacon_offload_enabled(scn->soc)) {
		soc_txrx_handle = wlan_psoc_get_dp_handle(scn->soc->psoc_obj);
		if (val) {
			if (qdf_atomic_inc_return(&ic->ic_bpr_enable) == 1) {
				scn->stats_bpr_subscriber.callback =
						ol_ath_process_offload_beacon;
				scn->stats_bpr_subscriber.context =
						scn->sc_pdev;
				if (cdp_wdi_event_sub(soc_txrx_handle,
						      pdev_id,
						      &scn->stats_bpr_subscriber,
						      WDI_EVENT_TX_BEACON))
					return A_ERROR;
				value.cdp_pdev_param_bpr_enable = val;
				cdp_txrx_set_pdev_param(soc_txrx_handle,
							pdev_id,
							CDP_CONFIG_BPR_ENABLE,
							value);
			}
		} else {
			if (qdf_atomic_dec_return(&ic->ic_bpr_enable) == 0) {
				if (cdp_wdi_event_unsub(soc_txrx_handle,
							pdev_id,
							&scn->stats_bpr_subscriber,
							WDI_EVENT_TX_BEACON))
					return A_ERROR;
				value.cdp_pdev_param_bpr_enable = val;
				cdp_txrx_set_pdev_param(soc_txrx_handle,
							pdev_id,
							CDP_CONFIG_BPR_ENABLE,
							value);
			}
		}
	}
	return 0;
}
#else
int ol_ath_set_bpr_wifi3(struct ol_ath_softc_net80211 *scn, int val)
{
	return A_ERROR;
}
#endif

#if QCA_UNDECODED_METADATA_SUPPORT
struct ieee80211vap *
ol_ath_get_special_vap(struct ieee80211com *ic)
{
	struct ieee80211vap *tmp_vap = NULL;

	TAILQ_FOREACH(tmp_vap, &ic->ic_vaps, iv_next) {
		if (tmp_vap->iv_opmode == IEEE80211_M_HOSTAP &&
		    IEEE80211_IS_SPECIAL_VAP_ONLY(tmp_vap)) {
			return tmp_vap;
		}
	}
	return NULL;
}

static void
ol_if_process_rx_data_undecoded_frame(struct ieee80211com *ic, void *data)
{
	qdf_nbuf_t skb = (qdf_nbuf_t)data;
	struct ieee80211vap *vap = NULL;
	osif_dev  *osifp = NULL;
	char *buf = NULL;

	if (ic->ic_mon_vap)
		vap = ic->ic_mon_vap;
	else
		vap = ol_ath_get_special_vap(ic);

	if (!vap) {
		qdf_nbuf_free(skb);
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_DEBUG,
			  "No monitor or special vap created");
		return;
	}

	/**
	 * Forwarding the buffer to user space or freeing the
	 * buffer based on the feature flag if not enabled.
	 * Have callback here to extract the data if required.
	 */
	if (!ic->ic_undecoded_metadata_enable) {
		qdf_debug("undecoded_metadata not enabled drop the frame");
		qdf_nbuf_free(skb);
		return;
	}

	osifp = (osif_dev *)vap->iv_ifp;

	buf = (char *)qdf_nbuf_put_tail(skb, DEBUG_SNIFFER_SIGNATURE_LEN);
	if (buf) {
		qdf_mem_copy(buf,
			     DEBUG_SNIFFER_TEST_RX_UNDEOCDED_FRAME_CAPTURE,
			     DEBUG_SNIFFER_SIGNATURE_LEN);
	}

	/* Ensure to free the skb here if not being sent to upper layer */
	monitor_osif_deliver_rx_capture_undecoded_metadata(osifp, skb);
}

/**
 * ol_ath_process_rx_data_undecoded_frame() - Callback registered for
 *                          WDI_EVENT_RX_PPDU_DESC_UNDECODED_METADATA
 * @pdev_hdl: pdev pointer
 * @event:   WDi event
 * @data:    skb received
 * @peer_id: peer_id
 * @status:  status from event
 *
 * @Return: None
 */
void
ol_ath_process_ppdu_info_undecoded_frame(void *pdev_hdl, enum WDI_EVENT event,
					 void *data, uint16_t peer_id,
					 enum htt_cmn_rx_status status)
{
	struct wlan_objmgr_pdev *pdev_obj =
		(struct wlan_objmgr_pdev *)pdev_hdl;
	struct ieee80211com *ic;
	qdf_nbuf_t nbuf = (qdf_nbuf_t)data;

	if (qdf_unlikely((!pdev_obj || !data))) {
		qdf_err("Invalid pdev %p or data %p event %d",
			pdev_obj, data, event);
		qdf_nbuf_free(nbuf);
		return;
	}

	ic = wlan_pdev_get_mlme_ext_obj(pdev_obj);
	if (!ic) {
		qdf_err(" ic is NULL");
		qdf_nbuf_free(nbuf);
		return;
	}

	ol_if_process_rx_data_undecoded_frame(ic, data);
}

static int
ol_set_undeocded_metadata_capture(struct ol_ath_softc_net80211 *scn,
				  uint8_t pdev_id,
				  ol_txrx_soc_handle soc_txrx_handle, int val)
{
	struct ieee80211com *ic = &scn->sc_ic;
	cdp_config_param_type value = {0};

	if (val) {
		/* registere wdi event */
		scn->rx_undecoded_ppdu_info_subscriber.callback =
				ol_ath_process_ppdu_info_undecoded_frame;
		scn->rx_undecoded_ppdu_info_subscriber.context =
				scn->sc_pdev;

		if (cdp_wdi_event_sub(soc_txrx_handle, pdev_id,
				      &scn->rx_undecoded_ppdu_info_subscriber,
				      WDI_EVENT_RX_PPDU_DESC_UNDECODED_METADATA)) {
			qdf_err("wdi event subscription failed");
			return A_ERROR;
		}

		value.cdp_pdev_param_undecoded_metadata_enable = val;
		if (cdp_txrx_set_pdev_param(soc_txrx_handle, pdev_id,
					    CDP_CONFIG_UNDECODED_METADATA_CAPTURE_ENABLE,
					    value)) {
			qdf_err("Enable undecoded metadata capture failed.");

			/* unsubscribe registered wdi event */
			if (cdp_wdi_event_unsub(soc_txrx_handle, pdev_id,
						&scn->rx_undecoded_ppdu_info_subscriber,
						WDI_EVENT_RX_PPDU_DESC_UNDECODED_METADATA)) {
				qdf_err("wdi event unsubscription failed");
				return A_ERROR;
			}
			return -EINVAL;
		}
		ic->ic_undecoded_metadata_enable = val;

		if (cdp_txrx_set_pdev_phyrx_error_mask(soc_txrx_handle,
						       pdev_id,
						       ic->ic_phyrx_error_mask,
						       ic->ic_phyrx_error_mask_cont)) {
			dp_mon_err("Phyrx error mask configuration failed");
		}
		dp_mon_info("mask(0 to 31):0x%x mask(32 to 64):0x%x",
			    ic->ic_phyrx_error_mask,
			    ic->ic_phyrx_error_mask_cont);
	} else {
		if (!ic->ic_undecoded_metadata_enable) {
			qdf_info("Undecoded metadata capture already disabled");
			return 0;
		}
		ic->ic_undecoded_metadata_enable = val;

		value.cdp_pdev_param_undecoded_metadata_enable = val;
		cdp_txrx_set_pdev_param(soc_txrx_handle, pdev_id,
					CDP_CONFIG_UNDECODED_METADATA_CAPTURE_ENABLE,
					value);

		/* unsubscribe registered wdi event */
		if (cdp_wdi_event_unsub(soc_txrx_handle, pdev_id,
					&scn->rx_undecoded_ppdu_info_subscriber,
					WDI_EVENT_RX_PPDU_DESC_UNDECODED_METADATA)) {
			qdf_err("wdi event unsubscription failed");
			return A_ERROR;
		}
	}

	return 0;
}
#endif /* QCA_UNDECODED_METADATA_SUPPORT */

static struct mon_ops monitor_ops = {
	.mon_ath_set_bpr_wifi3 = ol_ath_set_bpr_wifi3,
	.mon_set_lite_mode = ol_set_lite_mode,
	.mon_cfg80211_set_peer_pkt_capture_params =
		wlan_cfg80211_set_peer_pkt_capture_params,
	.mon_set_tx_sniffer_mode = ol_set_tx_sniffer_mode,
#ifdef QCA_UNDECODED_METADATA_SUPPORT
	.mon_set_undeocded_metadata_capture = ol_set_undeocded_metadata_capture,
	.mon_cfg80211_set_phyrx_error_mask =
		wlan_cfg80211_set_phyrx_error_mask,
	.mon_cfg80211_get_phyrx_error_mask =
		wlan_cfg80211_get_phyrx_error_mask,
#endif
};

QDF_STATUS mon_soc_ol_attach(struct wlan_objmgr_psoc *psoc)
{
	struct target_psoc_info *tgt_hdl;
	ol_ath_soc_softc_t *soc;
	struct wlan_objmgr_pdev *pdev;
	struct ol_ath_softc_net80211 *scn;
	struct ieee80211com *ic;
	struct ieee80211_bsscolor_handle *bsscolor_hdl;
	int i;
	int status = QDF_STATUS_E_INVAL;
	cdp_config_param_type value = {0};
	ol_txrx_soc_handle soc_txrx_handle;

	tgt_hdl = wlan_psoc_get_tgt_if_handle(psoc);

	if (!tgt_hdl) {
		dp_mon_err("target_psoc_info is null");
		return status;
	}

	soc = (ol_ath_soc_softc_t *)target_psoc_get_feature_ptr(tgt_hdl);

	if (!soc) {
		dp_mon_err("feature ptr is null");
		return status;
	}
	soc->soc_mon_ops = &monitor_ops;

	for (i = 0; i < WMI_HOST_MAX_PDEV; i++) {
		pdev = wlan_objmgr_get_pdev_by_id(psoc, i, WLAN_MLME_NB_ID);
		if (!pdev) {
			dp_mon_err("pdev object (id: %d) is NULL", i);
			continue;
		}
		scn = (struct ol_ath_softc_net80211 *)
				lmac_get_pdev_feature_ptr(pdev);
		if (!scn) {
			dp_mon_err("scn is NULL");

			wlan_objmgr_pdev_release_ref(pdev, WLAN_MLME_NB_ID);
			continue;
		}
		ic = &scn->sc_ic;
		/* rx monitor filter */
		ic->ic_set_rx_monitor_filter = ol_ath_set_rx_monitor_filter;
#if defined(WLAN_TX_PKT_CAPTURE_ENH) || defined(WLAN_RX_PKT_CAPTURE_ENH)
		ic->ic_cfg80211_radio_handler.ic_set_peer_pkt_capture_params =
					ol_ath_ucfg_set_peer_pkt_capture;
#endif // WLAN_TX_PKT_CAPTURE_ENH || WLAN_RX_PKT_CAPTURE_ENH
		bsscolor_hdl = &ic->ic_bsscolor_hdl;
		soc_txrx_handle = wlan_psoc_get_dp_handle(scn->soc->psoc_obj);
		if (bsscolor_hdl->state ==
		    IEEE80211_BSS_COLOR_CTX_STATE_ACTIVE) {
			value.cdp_pdev_param_bss_color = ic->ic_he_bsscolor;
			cdp_txrx_set_pdev_param(soc_txrx_handle, i,
						CDP_CONFIG_BSS_COLOR, value);
		}
		wlan_objmgr_pdev_release_ref(pdev, WLAN_MLME_NB_ID);
	}

	return QDF_STATUS_SUCCESS;
}

void mon_soc_ol_detach(struct wlan_objmgr_psoc *psoc)
{
	struct target_psoc_info *tgt_hdl;
	ol_ath_soc_softc_t *soc;
	struct wlan_objmgr_pdev *pdev;
	struct ol_ath_softc_net80211 *scn;
	struct ieee80211com *ic;
	int i;

	tgt_hdl = wlan_psoc_get_tgt_if_handle(psoc);

	if (!tgt_hdl) {
		dp_mon_err("target_psoc_info is null");
		return;
	}

	soc = (ol_ath_soc_softc_t *)target_psoc_get_feature_ptr(tgt_hdl);

	if (!soc) {
		dp_mon_err("feature ptr is null");
		return;
	}

	for (i = 0; i < WMI_HOST_MAX_PDEV; i++) {
		pdev = wlan_objmgr_get_pdev_by_id(psoc, i, WLAN_MLME_NB_ID);
		if (!pdev) {
			dp_mon_err("pdev object (id: %d) is NULL", i);
			continue;
		}
		scn = (struct ol_ath_softc_net80211 *)
				lmac_get_pdev_feature_ptr(pdev);
		if (!scn) {
			dp_mon_err("scn is NULL");

			wlan_objmgr_pdev_release_ref(pdev, WLAN_MLME_NB_ID);
			continue;
		}
		ic = &scn->sc_ic;
		/* rx monitor filter */
		ic->ic_set_rx_monitor_filter = NULL;
#if defined(WLAN_TX_PKT_CAPTURE_ENH) || defined(WLAN_RX_PKT_CAPTURE_ENH)
		ic->ic_cfg80211_radio_handler.ic_set_peer_pkt_capture_params =
									NULL;
#endif // WLAN_TX_PKT_CAPTURE_ENH || WLAN_RX_PKT_CAPTURE_ENH
		wlan_objmgr_pdev_release_ref(pdev, WLAN_MLME_NB_ID);
	}

	soc->soc_mon_ops = NULL;
}
