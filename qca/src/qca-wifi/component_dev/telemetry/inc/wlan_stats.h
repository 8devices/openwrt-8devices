/*
 * Copyright (c) 2021 The Linux Foundation. All rights reserved.
 * Copyright (c) 2021-2022 Qualcomm Innovation Center, Inc. All rights reserved.
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

#ifndef _WLAN_STATS_H_
#define _WLAN_STATS_H_

#include <wlan_stats_define.h>

/**
 * Length of SoC interface name passed to user space as soc<psoc_id>
 * including null caracter.
 **/
#define SOC_IF_LEN 5

/**
 * Length of Radio interface name passed to user space as wifi<pdev_id>
 * including null caracter.
 **/
#define RADIO_IF_LEN 6

/**
 * Deriving feature indexes corresponding to feature attributes defined in
 * qca_wlan_vendor_attr_feat dynamically.
 * Feature attribute values starts from 1. So, Deduction of 1 is required to
 * start the index from 0.
 **/
#define DEF_INX(_x) \
	INX_FEAT_##_x = (QCA_WLAN_VENDOR_ATTR_FEAT_##_x - 1)
/**
 * QCA_WLAN_VENDOR_ATTR_RECURSIVE attribute is the last attribute
 * in qca_wlan_vendor_attr_feat. So the Maximum index should hold
 * the count of feature index.
 */
#define DEF_INX_MAX() \
	INX_FEAT_MAX = (QCA_WLAN_VENDOR_ATTR_RECURSIVE - 1)
/**
 * This is to get qca_wlan_vendor_attr_feat attributes from feat_index_e.
 * So, the addition of 1 is required to get corresponding attribute.
 */
#define GET_ATTR(_x) ((_x) + 1)

/**
 * enum stats_feat_index_e: Defines stats feature indexes
 * This will auto generate each index value corresponding to that feature
 * attribute defined in qca_wlan_vendor_attr_feat.
 */
enum stats_feat_index_e {
	DEF_INX(ME),
	DEF_INX(RX),
	DEF_INX(TX),
	DEF_INX(AST),
	DEF_INX(CFR),
	DEF_INX(FWD),
	DEF_INX(HTT),
	DEF_INX(RAW),
	DEF_INX(TSO),
	DEF_INX(TWT),
	DEF_INX(WDI),
	DEF_INX(WMI),
	DEF_INX(IGMP),
	DEF_INX(LINK),
	DEF_INX(MESH),
	DEF_INX(RATE),
	DEF_INX(NAWDS),
	DEF_INX(DELAY),
	DEF_INX(JITTER),
	DEF_INX(TXCAP),
	DEF_INX(MONITOR),
	DEF_INX(SAWFDELAY),
	DEF_INX(SAWFTX),
	DEF_INX_MAX(),
};

/**
 * struct stats_config: Structure to hold user configurations
 * @wiphy:  Pointer to wiphy structure which came as part of User request
 * @feat:  Feat flag set to dedicated bit of this field
 * @lvl:  Requested level of Stats (i.e. Basic, Advance or Debug)
 * @obj:  Requested stats for object (i.e. AP, Radio, Vap or STA)
 * @type:  Requested stats category
 * @recursive:  Flag for Recursiveness of request
 * @serviceid: service id for checking the level of sawf stats
 */
struct stats_config {
	struct wiphy           *wiphy;
	u_int64_t              feat;
	enum stats_level_e     lvl;
	enum stats_object_e    obj;
	enum stats_type_e      type;
	bool                   recursive;
	u_int8_t               serviceid;
};

/**
 * struct multi_reply_ctx: Structure to manage multi reply message
 * @pending: Flag to detect pending data from previous reply
 * @start_inx: Index from which the stats will be processed
 **/
struct multi_reply_ctx {
	bool pending;
	uint8_t start_inx;
};

/**
 * struct unified_stats: Structure to carry all feature specific stats in driver
 *                       level for stats response setup
 * All features are void pointers and its corresponding sizes.
 * This can hold Basic or Advance or Debug structures independently.
 */
struct unified_stats {
	void *feat[INX_FEAT_MAX];
	u_int32_t size[INX_FEAT_MAX];
};

/**
 * struct iterator_ctx: Structure is used internaly for iteration over all
 *                      peer/vdev to aggregate the stats
 * @pvt: Void pointer to carry stats config
 * @stats: Pointer to unified stats
 */
struct iterator_ctx {
	void *pvt;
	struct unified_stats *stats;
};

/**
 * wlan_stats_get_peer_stats(): Function to get peer specific stats
 * @psoc:  Pointer to Psoc object
 * @peer:  Pointer to Peer object
 * @cfg:  Pointer to stats config came as part of user request
 * @stats:  Pointer to unified stats object
 *
 * Return: QDF_STATUS_SUCCESS for success and Error code for failure
 */
QDF_STATUS wlan_stats_get_peer_stats(struct wlan_objmgr_psoc *psoc,
				     struct wlan_objmgr_peer *peer,
				     struct stats_config *cfg,
				     struct unified_stats *stats);

/**
 * wlan_stats_get_vdev_stats(): Function to get vdev specific stats
 * @psoc:  Pointer to Psoc object
 * @vdev:  Pointer to Vdev object
 * @cfg:  Pointer to stats config came as part of user request
 * @stats:  Pointer to unified stats object
 *
 * Return: QDF_STATUS_SUCCESS for success and Error code for failure
 */
QDF_STATUS wlan_stats_get_vdev_stats(struct wlan_objmgr_psoc *psoc,
				     struct wlan_objmgr_vdev *vdev,
				     struct stats_config *cfg,
				     struct unified_stats *stats);

/**
 * wlan_stats_get_pdev_stats(): Function to get pdev specific stats
 * @psoc:  Pointer to Psoc object
 * @pdev:  Pointer to Pdev object
 * @cfg:  Pointer to stats config came as part of user request
 * @stats:  Pointer to unified stats object
 *
 * Return: QDF_STATUS_SUCCESS for success and Error code for failure
 */
QDF_STATUS wlan_stats_get_pdev_stats(struct wlan_objmgr_psoc *psoc,
				     struct wlan_objmgr_pdev *pdev,
				     struct stats_config *cfg,
				     struct unified_stats *stats);

/**
 * wlan_stats_get_psoc_stats(): Function to get psoc specific stats
 * @psoc:  Pointer to Psoc object
 * @cfg:  Pointer to stats config came as part of user request
 * @stats:  Pointer to unified stats object
 *
 * Return: QDF_STATUS_SUCCESS for success and Error code for failure
 */
QDF_STATUS wlan_stats_get_psoc_stats(struct wlan_objmgr_psoc *psoc,
				     struct stats_config *cfg,
				     struct unified_stats *stats);

/**
 * wlan_stats_is_recursive_valid(): Function to check recursiveness
 * @cfg:  Pointer to stats config came as part of user request
 * @obj:  The object for which recursiveness is being checked
 *
 * So, this function will check if requested feature is valid for
 * underneath objects.
 *
 * Return: True if Recursive is possible or false if not
 */
bool wlan_stats_is_recursive_valid(struct stats_config *cfg,
				   enum stats_object_e obj);

/**
 * wlan_stats_free_unified_stats(): Function to free all feature holder pointers
 * @stats:  Pointer to unified stats object
 *
 * Return: None
 */
void wlan_stats_free_unified_stats(struct unified_stats *stats);

#endif /* _WLAN_STATS_H_ */
