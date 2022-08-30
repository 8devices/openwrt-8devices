/*
 * Copyright (c) 2017-2020 The Linux Foundation. All rights reserved.
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
 * @file cdp_txrx_mon_struct.h
 * @brief Define the monitor mode API structure
 * shared by data path and the OS interface module
 */

#ifndef _CDP_TXRX_MON_STRUCT_H_
#define _CDP_TXRX_MON_STRUCT_H_
/* XXX not really a mode; there are really multiple PHY's */
enum cdp_mon_phymode {
	/* autoselect */
	CDP_IEEE80211_MODE_AUTO	= 0,
	/* 5GHz, OFDM */
	CDP_IEEE80211_MODE_11A	= 1,
	/* 2GHz, CCK */
	CDP_IEEE80211_MODE_11B	= 2,
	/* 2GHz, OFDM */
	CDP_IEEE80211_MODE_11G	= 3,
	/* 2GHz, GFSK */
	CDP_IEEE80211_MODE_FH	= 4,
	/* 5GHz, OFDM, 2x clock dynamic turbo */
	CDP_IEEE80211_MODE_TURBO_A	= 5,
	   /* 2GHz, OFDM, 2x clock dynamic turbo */
	CDP_IEEE80211_MODE_TURBO_G	= 6,
	/* 5Ghz, HT20 */
	CDP_IEEE80211_MODE_11NA_HT20	= 7,
	/* 2Ghz, HT20 */
	CDP_IEEE80211_MODE_11NG_HT20	= 8,
	/* 5Ghz, HT40 (ext ch +1) */
	CDP_IEEE80211_MODE_11NA_HT40PLUS	= 9,
	/* 5Ghz, HT40 (ext ch -1) */
	CDP_IEEE80211_MODE_11NA_HT40MINUS = 10,
	  /* 2Ghz, HT40 (ext ch +1) */
	CDP_IEEE80211_MODE_11NG_HT40PLUS = 11,
	/* 2Ghz, HT40 (ext ch -1) */
	CDP_IEEE80211_MODE_11NG_HT40MINUS = 12,
	/* 2Ghz, Auto HT40 */
	CDP_IEEE80211_MODE_11NG_HT40	= 13,
	/* 5Ghz, Auto HT40 */
	CDP_IEEE80211_MODE_11NA_HT40	= 14,
	/* 5Ghz, VHT20 */
	CDP_IEEE80211_MODE_11AC_VHT20	= 15,
	/* 5Ghz, VHT40 (Ext ch +1) */
	CDP_IEEE80211_MODE_11AC_VHT40PLUS   = 16,
	/* 5Ghz  VHT40 (Ext ch -1) */
	CDP_IEEE80211_MODE_11AC_VHT40MINUS  = 17,
	/* 5Ghz, VHT40 */
	CDP_IEEE80211_MODE_11AC_VHT40	= 18,
	/* 5Ghz, VHT80 */
	CDP_IEEE80211_MODE_11AC_VHT80	= 19,
	/* 5Ghz, VHT160 */
	CDP_IEEE80211_MODE_11AC_VHT160	= 20,
	/* 5Ghz, VHT80_80 */
	CDP_IEEE80211_MODE_11AC_VHT80_80	= 21,
};

enum {
	CDP_PKT_TYPE_OFDM = 0,
	CDP_PKT_TYPE_CCK,
	CDP_PKT_TYPE_HT,
	CDP_PKT_TYPE_VHT,
	CDP_PKT_TYPE_HE,
};

enum {
	CDP_SGI_0_8_US = 0,
	CDP_SGI_0_4_US,
	CDP_SGI_1_6_US,
	CDP_SGI_3_2_US,
};

enum {
	CDP_RX_TYPE_SU = 0,
	CDP_RX_TYPE_MU_MIMO,
	CDP_RX_TYPE_MU_OFDMA,
	CDP_RX_TYPE_MU_OFDMA_MIMO,
};

/*
 *Band Width Types
 */
enum CMN_BW_TYPES {
	CMN_BW_20MHZ,
	CMN_BW_40MHZ,
	CMN_BW_80MHZ,
	CMN_BW_160MHZ,
	CMN_BW_80_80MHZ,
#ifdef WLAN_FEATURE_11BE
	CMN_BW_320MHZ,
#endif
	CMN_BW_CNT,
	CMN_BW_IDLE = 0xFF, /*default BW state */
};

struct cdp_mon_status {
	/* bss color value 1-63 used for update on ppdu_desc bsscolor */
	uint8_t bsscolor;
	int rs_numchains;
	int rs_flags;
#define IEEE80211_RX_FCS_ERROR      0x01
#define IEEE80211_RX_MIC_ERROR      0x02
#define IEEE80211_RX_DECRYPT_ERROR  0x04
/* holes in flags here between, ATH_RX_XXXX to IEEE80211_RX_XXX */
#define IEEE80211_RX_KEYMISS        0x200
#define IEEE80211_RX_PN_ERROR       0x400
	int rs_rssi;       /* RSSI (noise floor ajusted) */
	int rs_abs_rssi;   /* absolute RSSI */
	int rs_datarate;   /* data rate received */
	int rs_rateieee;
	int rs_ratephy1;
	int rs_ratephy2;
	int rs_ratephy3;

/* Keep the same as ATH_MAX_ANTENNA */
#define IEEE80211_MAX_ANTENNA       3
	/* RSSI (noise floor ajusted) */
	u_int8_t    rs_rssictl[IEEE80211_MAX_ANTENNA];
	/* RSSI (noise floor ajusted) */
	u_int8_t    rs_rssiextn[IEEE80211_MAX_ANTENNA];
	/* rs_rssi is valid or not */
	u_int8_t    rs_isvalidrssi;

	enum cdp_mon_phymode rs_phymode;
	int         rs_freq;

	union {
		u_int8_t            data[8];
		u_int64_t           tsf;
	} rs_tstamp;

	/*
	 * Detail channel structure of recv frame.
	 * It could be NULL if not available
	 */


#ifdef ATH_SUPPORT_AOW
	u_int16_t   rs_rxseq;      /* WLAN Sequence number */
#endif
#ifdef ATH_VOW_EXT_STATS
	/* Lower 16 bits holds the udp checksum offset in the data pkt */
	u_int32_t vow_extstats_offset;
	/* Higher 16 bits contains offset in the data pkt at which vow
	 * ext stats are embedded
	 */
#endif
	u_int8_t rs_isaggr;
	u_int8_t rs_isapsd;
	int16_t rs_noisefloor;
	u_int16_t  rs_channel;
#ifdef ATH_SUPPORT_TxBF
	u_int32_t   rs_rpttstamp;   /* txbf report time stamp*/
#endif

	/* The following counts are meant to assist in stats calculation.
	 * These variables are incremented only in specific situations, and
	 * should not be relied upon for any purpose other than the original
	 * stats related purpose they have been introduced for.
	*/

	u_int16_t   rs_cryptodecapcount; /* Crypto bytes decapped/demic'ed. */
	u_int8_t    rs_padspace;         /* No. of padding bytes present after
					  header in wbuf. */
	u_int8_t    rs_qosdecapcount;    /* QoS/HTC bytes decapped. */

	/* End of stats calculation related counts. */

	/*
	 * uint8_t     rs_lsig[IEEE80211_LSIG_LEN];
	 * uint8_t     rs_htsig[IEEE80211_HTSIG_LEN];
	 * uint8_t     rs_servicebytes[IEEE80211_SB_LEN];
	 * uint8_t     rs_fcs_error;
	*/

	/* cdp convergence monitor mode status */
	union {
		u_int8_t            cdp_data[8];
		u_int64_t           cdp_tsf;
	} cdp_rs_tstamp;

	uint8_t  cdp_rs_pream_type;
	uint32_t cdp_rs_user_rssi;
	uint8_t  cdp_rs_stbc;
	uint8_t  cdp_rs_sgi;
	uint32_t cdf_rs_rate_mcs;
	uint32_t cdp_rs_reception_type;
	uint32_t cdp_rs_bw;
	uint32_t cdp_rs_nss;
	uint8_t  cdp_rs_fcs_err;
	bool     cdp_rs_rxdma_err;
};

enum {
	CDP_MON_PPDU_START = 0,
	CDP_MON_PPDU_END,
};

#ifdef QCA_UNDECODED_METADATA_SUPPORT
/**
 * enum cdp_mon_phyrx_abort_reason_code: Phy err code to store the reason
 * why PHY generated an abort request.
 */
enum cdp_mon_phyrx_abort_reason_code {
	CDP_PHYRX_ERR_PHY_OFF = 0,
	CDP_PHYRX_ERR_SYNTH_OFF,
	CDP_PHYRX_ERR_OFDMA_TIMING,
	CDP_PHYRX_ERR_OFDMA_SIGNAL_PARITY,
	CDP_PHYRX_ERR_OFDMA_RATE_ILLEGAL,
	CDP_PHYRX_ERR_OFDMA_LENGTH_ILLEGAL,
	CDP_PHYRX_ERR_OFDMA_RESTART,
	CDP_PHYRX_ERR_OFDMA_SERVICE,
	CDP_PHYRX_ERR_PPDU_OFDMA_POWER_DROP,
	CDP_PHYRX_ERR_CCK_BLOKKER,
	CDP_PHYRX_ERR_CCK_TIMING = 10,
	CDP_PHYRX_ERR_CCK_HEADER_CRC,
	CDP_PHYRX_ERR_CCK_RATE_ILLEGAL,
	CDP_PHYRX_ERR_CCK_LENGTH_ILLEGAL,
	CDP_PHYRX_ERR_CCK_RESTART,
	CDP_PHYRX_ERR_CCK_SERVICE,
	CDP_PHYRX_ERR_CCK_POWER_DROP,
	CDP_PHYRX_ERR_HT_CRC_ERR,
	CDP_PHYRX_ERR_HT_LENGTH_ILLEGAL,
	CDP_PHYRX_ERR_HT_RATE_ILLEGAL,
	CDP_PHYRX_ERR_HT_ZLF = 20,
	CDP_PHYRX_ERR_FALSE_RADAR_EXT,
	CDP_PHYRX_ERR_GREEN_FIELD,
	CDP_PHYRX_ERR_BW_GT_DYN_BW,
	CDP_PHYRX_ERR_HT_LSIG_RATE_MISMATCH,
	CDP_PHYRX_ERR_VHT_CRC_ERROR,
	CDP_PHYRX_ERR_VHT_SIGA_UNSUPPORTED,
	CDP_PHYRX_ERR_VHT_LSIG_LEN_INVALID,
	CDP_PHYRX_ERR_VHT_NDP_OR_ZLF,
	CDP_PHYRX_ERR_VHT_NSYM_LT_ZERO,
	CDP_PHYRX_ERR_VHT_RX_EXTRA_SYMBOL_MISMATCH = 30,
	CDP_PHYRX_ERR_VHT_RX_SKIP_GROUP_ID0,
	CDP_PHYRX_ERR_VHT_RX_SKIP_GROUP_ID1TO62,
	CDP_PHYRX_ERR_VHT_RX_SKIP_GROUP_ID63,
	CDP_PHYRX_ERR_OFDM_LDPC_DECODER_DISABLED,
	CDP_PHYRX_ERR_DEFER_NAP,
	CDP_PHYRX_ERR_FDOMAIN_TIMEOUT,
	CDP_PHYRX_ERR_LSIG_REL_CHECK,
	CDP_PHYRX_ERR_BT_COLLISION,
	CDP_PHYRX_ERR_UNSUPPORTED_MU_FEEDBACK,
	CDP_PHYRX_ERR_PPDU_TX_INTERRUPT_RX = 40,
	CDP_PHYRX_ERR_UNSUPPORTED_CBF,
	CDP_PHYRX_ERR_OTHER,
	CDP_PHYRX_ERR_HE_SIGA_UNSUPPORTED,
	CDP_PHYRX_ERR_HE_SIGA_CRC_ERROR,
	CDP_PHYRX_ERR_HE_SIGB_UNSUPPORTED,
	CDP_PHYRX_ERR_HE_SIGB_CRC_ERROR,
	CDP_PHYRX_ERR_HE_MU_MODE_UNSUPPORTED,
	CDP_PHYRX_ERR_HE_NDP_OR_ZLF,
	CDP_PHYRX_ERR_HE_NSYM_LT_ZERO,
	CDP_PHYRX_ERR_HE_RU_PARAMS_UNSUPPORTED = 50,
	CDP_PHYRX_ERR_HE_NUM_USERS_UNSUPPORTED,
	CDP_PHYRX_ERR_HE_SOUNDING_PARAMS_UNSUPPORTED,
	CDP_PHYRX_ERR_HE_EXT_SU_UNSUPPORTED,
	CDP_PHYRX_ERR_HE_TRIG_UNSUPPORTED,
	CDP_PHYRX_ERR_HE_LSIG_LEN_INVALID = 55,
	CDP_PHYRX_ERR_HE_LSIG_RATE_MISMATCH,
	CDP_PHYRX_ERR_OFDMA_SIGNAL_RELIABILITY,
	CDP_PHYRX_ERR_HT_NSYM_LT_ZERO,
	CDP_PHYRX_ERR_VHT_LSIG_RATE_MISMATCH,
	CDP_PHYRX_ERR_VHT_PAID_GID_MISMATCH = 60,
	CDP_PHYRX_ERR_VHT_UNSUPPORTED_BW,
	CDP_PHYRX_ERR_VHT_GI_DISAM_MISMATCH,
	CDP_PHYRX_ERR_RX_WDG_TIMEOUT = 63,
	CDP_PHYRX_ERR_MAX
};
#endif

#define MAX_PPDU_ID_HIST 128

/**
 * struct cdp_pdev_mon_stats
 * @status_ppdu_state: state on PPDU start and end
 * @status_ppdu_start: status ring PPDU start TLV count
 * @status_ppdu_end: status ring PPDU end TLV count
 * @status_ppdu_compl: status ring matching start and end count on PPDU
 * @status_ppdu_start_mis: status ring missing start TLV count on PPDU
 * @status_ppdu_end_mis: status ring missing end TLV count on PPDU
 * @status_ppdu_done: status ring PPDU done TLV count
 * @dest_ppdu_done: destination ring PPDU count
 * @dest_mpdu_done: destination ring MPDU count
 * @dup_mon_linkdesc_cnt: duplicate link descriptor indications from HW
 * @dup_mon_buf_cnt: duplicate buffer indications from HW
 * @tlv_tag_status_err: status not correct in the tlv tag
 * @status_buf_done_war: Number of status ring buffers for which DMA not done
 *  WAR is applied.
 * @mon_rx_bufs_replenished_dest: Rx buffers replenish count
 * @mon_rx_bufs_reaped_dest: Rx buffer reap count
 * @ppdu_id_mismatch: counter to track ppdu id mismatch in
 *  mointor status and monitor destination ring
 * @ppdu_id_match: counter to track ppdu id match in
 *  mointor status and monitor destination ring
 * @status_ppdu_drop: Number of ppdu dropped from monitor status ring
 * @dest_ppdu_drop: Number of ppdu dropped from monitor destination ring
 * @mon_link_desc_invalid: msdu link desc invalid count
 * @mon_rx_desc_invalid: rx_desc invalid count
 * @rx_undecoded_count: Received undecoded frame count
 * @rx_undecoded_error: Rx undecoded errors
 */
struct cdp_pdev_mon_stats {
#ifndef REMOVE_MON_DBG_STATS
	uint32_t status_ppdu_state;
	uint32_t status_ppdu_start;
	uint32_t status_ppdu_end;
	uint32_t status_ppdu_compl;
	uint32_t status_ppdu_start_mis;
	uint32_t status_ppdu_end_mis;
#endif
	uint32_t status_ppdu_done;
	uint32_t dest_ppdu_done;
	uint32_t dest_mpdu_done;
	uint32_t dest_mpdu_drop;
	uint32_t dup_mon_linkdesc_cnt;
	uint32_t dup_mon_buf_cnt;
	uint32_t stat_ring_ppdu_id_hist[MAX_PPDU_ID_HIST];
	uint32_t dest_ring_ppdu_id_hist[MAX_PPDU_ID_HIST];
	uint32_t ppdu_id_hist_idx;
	uint32_t mon_rx_dest_stuck;
	uint32_t tlv_tag_status_err;
	uint32_t status_buf_done_war;
	uint32_t mon_rx_bufs_replenished_dest;
	uint32_t mon_rx_bufs_reaped_dest;
	uint32_t ppdu_id_mismatch;
	uint32_t ppdu_id_match;
	uint32_t status_ppdu_drop;
	uint32_t dest_ppdu_drop;
	uint32_t mon_link_desc_invalid;
	uint32_t mon_rx_desc_invalid;
	uint32_t mon_nbuf_sanity_err;
#ifdef QCA_UNDECODED_METADATA_SUPPORT
	uint32_t rx_undecoded_count;
	uint32_t rx_undecoded_error[CDP_PHYRX_ERR_MAX];
#endif
};
#endif
