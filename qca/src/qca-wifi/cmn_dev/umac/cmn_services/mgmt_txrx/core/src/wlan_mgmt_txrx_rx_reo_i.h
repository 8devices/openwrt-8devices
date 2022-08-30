/*
 * Copyright (c) 2021, The Linux Foundation. All rights reserved.
 * Copyright (c) 2021-2022 Qualcomm Innovation Center, Inc. All rights reserved.
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
 *  DOC: wlan_mgmt_txrx_rx_reo_i.h
 *  This file contains mgmt rx re-ordering related APIs
 */

#ifndef _WLAN_MGMT_TXRX_RX_REO_I_H
#define _WLAN_MGMT_TXRX_RX_REO_I_H

#ifdef WLAN_MGMT_RX_REO_SUPPORT
#include <qdf_list.h>
#include <qdf_timer.h>
#include <qdf_lock.h>
#include <qdf_nbuf.h>
#include <qdf_threads.h>
#include <qdf_defer.h>
#include <wlan_mgmt_txrx_rx_reo_utils_api.h>
#include <wlan_mgmt_txrx_rx_reo_public_structs.h>
#include <wlan_objmgr_pdev_obj.h>
#include <wlan_objmgr_psoc_obj.h>
#include <wlan_mlo_mgr_public_structs.h>

#define MGMT_RX_REO_LIST_MAX_SIZE             (100)
#define MGMT_RX_REO_LIST_TIMEOUT_US           (500 * USEC_PER_MSEC)
#define MGMT_RX_REO_AGEOUT_TIMER_PERIOD_MS    (250)
#define MGMT_RX_REO_STATUS_WAIT_FOR_FRAME_ON_OTHER_LINKS         (BIT(0))
#define MGMT_RX_REO_STATUS_AGED_OUT                              (BIT(1))
#define MGMT_RX_REO_STATUS_OLDER_THAN_LATEST_AGED_OUT_FRAME      (BIT(2))
#define MGMT_RX_REO_STATUS_LIST_MAX_SIZE_EXCEEDED                (BIT(3))

#define MGMT_RX_REO_INVALID_LINK_ID   (-1)

/* Reason to release an entry from the reorder list */
#define MGMT_RX_REO_LIST_ENTRY_RELEASE_REASON_ZERO_WAIT_COUNT           (BIT(0))
#define MGMT_RX_REO_LIST_ENTRY_RELEASE_REASON_AGED_OUT                  (BIT(1))
#define MGMT_RX_REO_LIST_ENTRY_RELEASE_REASON_OLDER_THAN_AGED_OUT_FRAME (BIT(2))
#define MGMT_RX_REO_LIST_ENTRY_RELEASE_REASON_LIST_MAX_SIZE_EXCEEDED    (BIT(3))
#define MGMT_RX_REO_RELEASE_REASON_MAX      \
	(MGMT_RX_REO_LIST_ENTRY_RELEASE_REASON_LIST_MAX_SIZE_EXCEEDED << 1)

#define MGMT_RX_REO_LIST_ENTRY_IS_WAITING_FOR_FRAME_ON_OTHER_LINK(entry)   \
	((entry)->status & MGMT_RX_REO_STATUS_WAIT_FOR_FRAME_ON_OTHER_LINKS)
#define MGMT_RX_REO_LIST_ENTRY_IS_AGED_OUT(entry)   \
	((entry)->status & MGMT_RX_REO_STATUS_AGED_OUT)
#define MGMT_RX_REO_LIST_ENTRY_IS_OLDER_THAN_LATEST_AGED_OUT_FRAME(entry)  \
	((entry)->status & MGMT_RX_REO_STATUS_OLDER_THAN_LATEST_AGED_OUT_FRAME)
#define MGMT_RX_REO_LIST_ENTRY_IS_MAX_SIZE_EXCEEDED(entry)  \
	((entry)->status & MGMT_RX_REO_STATUS_LIST_MAX_SIZE_EXCEEDED)

#ifdef WLAN_MGMT_RX_REO_DEBUG_SUPPORT
#define MGMT_RX_REO_INGRESS_FRAME_DEBUG_ENTRIES_MAX             (1000)
#define MGMT_RX_REO_EGRESS_FRAME_DEBUG_ENTRIES_MAX              (1000)

#define MGMT_RX_REO_EGRESS_FRAME_DEBUG_INFO_FLAG_MAX_SIZE   (3)
#define MGMT_RX_REO_EGRESS_FRAME_DEBUG_INFO_WAIT_COUNT_MAX_SIZE   (49)

#define MGMT_RX_REO_INGRESS_FRAME_DEBUG_INFO_FLAG_MAX_SIZE   (9)
#define MGMT_RX_REO_INGRESS_FRAME_DEBUG_INFO_WAIT_COUNT_MAX_SIZE   (49)
#endif /* WLAN_MGMT_RX_REO_DEBUG_SUPPORT*/

/*
 * struct mgmt_rx_reo_pdev_info - Pdev information required by the Management
 * Rx REO module
 * @host_snapshot: Latest snapshot seen at the Host.
 * It considers both MGMT Rx and MGMT FW consumed.
 * @last_valid_shared_snapshot: Array of last valid snapshots(for snapshots
 * shared between host and target)
 * @host_target_shared_snapshot: Array of snapshot addresses(for snapshots
 * shared between host and target)
 * @filter: MGMT Rx REO filter
 */
struct mgmt_rx_reo_pdev_info {
	struct mgmt_rx_reo_snapshot_params host_snapshot;
	struct mgmt_rx_reo_snapshot_params last_valid_shared_snapshot
				[MGMT_RX_REO_SHARED_SNAPSHOT_MAX];
	struct mgmt_rx_reo_snapshot *host_target_shared_snapshot
				[MGMT_RX_REO_SHARED_SNAPSHOT_MAX];
	struct mgmt_rx_reo_filter filter;
};

/**
 * mgmt_rx_reo_pdev_obj_open_notification() - pdev open handler for
 * management rx-reorder module
 * @pdev: pointer to pdev object
 * @mgmt_txrx_pdev_ctx: pdev private object of mgmt txrx module
 *
 * This function gets called from object manager when pdev is being opened and
 * creates management rx-reorder pdev context
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
mgmt_rx_reo_pdev_obj_open_notification
		(struct wlan_objmgr_pdev *pdev,
		 struct mgmt_txrx_priv_pdev_context *mgmt_txrx_pdev_ctx);

/**
 * mgmt_rx_reo_pdev_obj_create_notification() - pdev create handler for
 * management rx-reorder module
 * @pdev: pointer to pdev object
 * @mgmt_txrx_pdev_ctx: pdev private object of mgmt txrx module
 *
 * This function gets called from object manager when pdev is being created and
 * creates management rx-reorder pdev context
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
mgmt_rx_reo_pdev_obj_create_notification(
	struct wlan_objmgr_pdev *pdev,
	struct mgmt_txrx_priv_pdev_context *mgmt_txrx_pdev_ctx);

/**
 * mgmt_rx_reo_pdev_obj_destroy_notification() - pdev destroy handler for
 * management rx-reorder feature
 * @pdev: pointer to pdev object
 * @mgmt_txrx_pdev_ctx: pdev private object of mgmt txrx module
 *
 * This function gets called from object manager when pdev is being destroyed
 * and destroys management rx-reorder pdev context
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
mgmt_rx_reo_pdev_obj_destroy_notification(
	struct wlan_objmgr_pdev *pdev,
	struct mgmt_txrx_priv_pdev_context *mgmt_txrx_pdev_ctx);

/**
 * enum mgmt_rx_reo_frame_descriptor_type - Enumeration for management frame
 * descriptor type.
 * @MGMT_RX_REO_FRAME_DESC_HOST_CONSUMED_FRAME: Management frame to be consumed
 * by host.
 * @MGMT_RX_REO_FRAME_DESC_FW_CONSUMED_FRAME: Management frame consumed by FW
 * @MGMT_RX_REO_FRAME_DESC_ERROR_FRAME: Management frame which got dropped
 * at host due to any error
 * @MGMT_RX_REO_FRAME_DESC_TYPE_MAX: Maximum number of frame types
 */
enum mgmt_rx_reo_frame_descriptor_type {
	MGMT_RX_REO_FRAME_DESC_HOST_CONSUMED_FRAME = 0,
	MGMT_RX_REO_FRAME_DESC_FW_CONSUMED_FRAME,
	MGMT_RX_REO_FRAME_DESC_ERROR_FRAME,
	MGMT_RX_REO_FRAME_DESC_TYPE_MAX,
};

/**
 * struct mgmt_rx_reo_global_ts_info - This structure holds the global time
 * stamp information of a frame.
 * @global_ts: Global time stamp value
 * @valid: Indicates whether global time stamp is valid
 */
struct mgmt_rx_reo_global_ts_info {
	bool valid;
	uint32_t global_ts;
};

/**
 * struct mgmt_rx_reo_list – Linked list used to reorder the management frames
 * received. Each list entry would correspond to a management frame. List
 * entries would be sorted in the same order in which they are received by MAC
 * HW.
 * @list: List used for reordering
 * @list_lock: Lock to protect the list
 * @max_list_size: Maximum size of the reorder list
 * @list_entry_timeout_us: Time out value(microsecond) for the reorder list
 * entries
 * @ageout_timer: Periodic timer to age-out the list entries
 * @ts_last_released_frame: Stores the global time stamp for the last frame
 * removed from the reorder list
 */
struct mgmt_rx_reo_list {
	qdf_list_t list;
	qdf_spinlock_t list_lock;
	uint32_t max_list_size;
	uint32_t list_entry_timeout_us;
	qdf_timer_t ageout_timer;
	struct mgmt_rx_reo_global_ts_info ts_last_released_frame;
};

/*
 * struct mgmt_rx_reo_wait_count - Wait count for a mgmt frame
 * @per_link_count: Array of wait counts for all MLO links. Each array entry
 * holds the number of frames this mgmt frame should wait for on that
 * particular link.
 * @total_count: Sum of entries in @per_link_count
 */
struct mgmt_rx_reo_wait_count {
	unsigned int per_link_count[MAX_MLO_LINKS];
	unsigned long long int total_count;
};

/**
 * struct mgmt_rx_reo_list_entry - Entry in the Management reorder list
 * @node: List node
 * @nbuf: nbuf corresponding to this frame
 * @rx_params: Management rx event parameters
 * @wait_count: Wait counts for the frame
 * @insertion_ts: Host time stamp when this entry is inserted to the list.
 * @removal_ts: Host time stamp when this entry is removed from the list
 * @ingress_timestamp: Host time stamp when this frame has arrived reorder
 * module
 * @egress_timestamp: Host time stamp when this frame has exited reorder
 * module
 * @status: Status for this entry
 * @pdev: Pointer to pdev object corresponding to this frame
 * @release_reason: Release reason
 * @is_delivered: Indicates whether the frame is delivered successfully
 * @is_premature_delivery: Indicates whether the frame is delivered
 * prematurely
 */
struct mgmt_rx_reo_list_entry {
	qdf_list_node_t node;
	qdf_nbuf_t nbuf;
	struct mgmt_rx_event_params *rx_params;
	struct mgmt_rx_reo_wait_count wait_count;
	uint64_t insertion_ts;
	uint64_t removal_ts;
	uint64_t ingress_timestamp;
	uint64_t egress_timestamp;
	uint32_t status;
	struct wlan_objmgr_pdev *pdev;
	uint8_t release_reason;
	bool is_delivered;
	bool is_premature_delivery;
};

#ifdef WLAN_MGMT_RX_REO_SIM_SUPPORT

#define MGMT_RX_REO_SIM_INTER_FRAME_DELAY_MIN             (300 * USEC_PER_MSEC)
#define MGMT_RX_REO_SIM_INTER_FRAME_DELAY_MIN_MAX_DELTA   (200 * USEC_PER_MSEC)

#define MGMT_RX_REO_SIM_DELAY_MAC_HW_TO_FW_MIN            (1000 * USEC_PER_MSEC)
#define MGMT_RX_REO_SIM_DELAY_MAC_HW_TO_FW_MIN_MAX_DELTA  (500 * USEC_PER_MSEC)

#define MGMT_RX_REO_SIM_DELAY_FW_TO_HOST_MIN              (1000 * USEC_PER_MSEC)
#define MGMT_RX_REO_SIM_DELAY_FW_TO_HOST_MIN_MAX_DELTA    (500 * USEC_PER_MSEC)

#define MGMT_RX_REO_SIM_PERCENTAGE_FW_CONSUMED_FRAMES  (10)
#define MGMT_RX_REO_SIM_PERCENTAGE_ERROR_FRAMES        (10)

#define MGMT_RX_REO_SIM_PENDING_FRAME_LIST_MAX_SIZE    (1000)
#define MGMT_RX_REO_SIM_STALE_FRAME_LIST_MAX_SIZE      \
				(MGMT_RX_REO_SIM_PENDING_FRAME_LIST_MAX_SIZE)
#define MGMT_RX_REO_SIM_STALE_FRAME_TEMP_LIST_MAX_SIZE (100)

/**
 * struct mgmt_rx_frame_params - Parameters associated with a management frame.
 * This structure is used by the simulation framework.
 * @link_id: MLO HW link id
 * @mgmt_pkt_ctr: Management packet counter
 * @global_timestamp: Global time stamp in micro seconds
 */
struct mgmt_rx_frame_params {
	uint8_t link_id;
	uint16_t mgmt_pkt_ctr;
	uint32_t global_timestamp;
};

/**
 * struct mgmt_rx_reo_master_frame_list - List which contains all the
 * management frames received and not yet consumed by FW/Host. Order of frames
 * in the list is same as the order in which they are received in the air.
 * This is used by the simulation framework to confirm that the outcome of
 * reordering is correct.
 * @pending_list: List which contains all the frames received after the
 * last frame delivered to upper layer. These frames will eventually reach host.
 * @stale_list: List which contains all the stale management frames which
 * are not yet consumed by FW/Host. Stale management frames are the frames which
 * are older than last delivered frame to upper layer.
 * @lock: Spin lock to protect pending frame list and stale frame list.
 */
struct mgmt_rx_reo_master_frame_list {
	qdf_list_t pending_list;
	qdf_list_t stale_list;
	qdf_spinlock_t lock;
};

/**
 * struct mgmt_rx_reo_pending_frame_list_entry - Structure used to represent an
 * entry in the pending frame list.
 * @params: parameters related to the management frame
 * @node: linked list node
 */
struct mgmt_rx_reo_pending_frame_list_entry {
	struct mgmt_rx_frame_params params;
	qdf_list_node_t node;
};

/**
 * struct mgmt_rx_reo_stale_frame_list_entry - Structure used to represent an
 * entry in the stale frame list.
 * @params: parameters related to the management frame
 * @node: linked list node
 */
struct mgmt_rx_reo_stale_frame_list_entry {
	struct mgmt_rx_frame_params params;
	qdf_list_node_t node;
};

/**
 * struct mgmt_rx_frame_mac_hw - Structure used to represent the management
 * frame at MAC HW level
 * @params: parameters related to the management frame
 * @frame_handler_fw: Work structure to queue the frame to the FW
 * @sim_context: pointer management rx-reorder simulation context
 */
struct mgmt_rx_frame_mac_hw {
	struct mgmt_rx_frame_params params;
	qdf_work_t frame_handler_fw;
	struct mgmt_rx_reo_sim_context *sim_context;
};

/**
 * struct mgmt_rx_frame_fw - Structure used to represent the management
 * frame at FW level
 * @params: parameters related to the management frame
 * @is_consumed_by_fw: indicates whether the frame is consumed by FW
 * @frame_handler_host: Work structure to queue the frame to the host
 * @sim_context: pointer management rx-reorder simulation context
 */
struct mgmt_rx_frame_fw {
	struct mgmt_rx_frame_params params;
	bool is_consumed_by_fw;
	qdf_work_t frame_handler_host;
	struct mgmt_rx_reo_sim_context *sim_context;
};

/**
 * struct mgmt_rx_reo_sim_mac_hw - Structure used to represent the MAC HW
 * @mgmt_pkt_ctr: Stores the last management packet counter for all the links
 */
struct mgmt_rx_reo_sim_mac_hw {
	uint16_t mgmt_pkt_ctr[MAX_MLO_LINKS];
};

/**
 * struct mgmt_rx_reo_sim_link_id_to_pdev_map - Map from link id to pdev
 * object. This is used for simulation purpose only.
 * @map: link id to pdev map. Link id is the array index.
 * @lock: lock used to protect this structure
 * @num_mlo_links: Total number of MLO HW links. In case of simulation all the
 * pdevs are assumed to have MLO capability and number of MLO links is same as
 * the number of pdevs in the system.
 * @valid_link_list: List of valid link id values
 */
struct mgmt_rx_reo_sim_link_id_to_pdev_map {
	struct wlan_objmgr_pdev *map[MAX_MLO_LINKS];
	qdf_spinlock_t lock;
	uint8_t num_mlo_links;
	int8_t valid_link_list[MAX_MLO_LINKS];
};

/**
 * struct mgmt_rx_reo_mac_hw_simulator - Structure which stores the members
 * required for the MAC HW simulation
 * @mac_hw_info: MAC HW info
 * @mac_hw_thread: kthread which simulates MAC HW
 */
struct mgmt_rx_reo_mac_hw_simulator {
	struct mgmt_rx_reo_sim_mac_hw mac_hw_info;
	qdf_thread_t *mac_hw_thread;
};

/**
 * struct mgmt_rx_reo_sim_context - Management rx-reorder simulation context
 * @host_mgmt_frame_handler: Per link work queue to simulate the host layer
 * @fw_mgmt_frame_handler: Per link work queue to simulate the FW layer
 * @master_frame_list: List used to store information about all the management
 * frames
 * @mac_hw_sim:  MAC HW simulation object
 * @snapshot: snapshots required for reo algorithm
 * @link_id_to_pdev_map: link_id to pdev object map
 */
struct mgmt_rx_reo_sim_context {
	struct workqueue_struct *host_mgmt_frame_handler[MAX_MLO_LINKS];
	struct workqueue_struct *fw_mgmt_frame_handler[MAX_MLO_LINKS];
	struct mgmt_rx_reo_master_frame_list master_frame_list;
	struct mgmt_rx_reo_mac_hw_simulator mac_hw_sim;
	struct mgmt_rx_reo_snapshot snapshot[MAX_MLO_LINKS]
					    [MGMT_RX_REO_SHARED_SNAPSHOT_MAX];
	struct mgmt_rx_reo_sim_link_id_to_pdev_map link_id_to_pdev_map;
};
#endif /* WLAN_MGMT_RX_REO_SIM_SUPPORT */

#ifdef WLAN_MGMT_RX_REO_DEBUG_SUPPORT
/**
 * struct reo_ingress_debug_frame_info - Debug information about a frame
 * entering reorder algorithm
 * @link_id: link id
 * @mgmt_pkt_ctr: management packet counter
 * @global_timestamp: MLO global time stamp
 * @type: Type of the frame descriptor
 * @ingress_timestamp: Host time stamp when the frames enters the reorder
 * algorithm
 * @ingress_duration: Duration in us for processing the incoming frame.
 * ingress_duration = Time stamp at which reorder list update is done -
 * Time stamp at which frame has entered the reorder module
 * @wait_count: Wait count calculated for the current frame
 * @is_queued: Indicates whether this frame is queued to reorder list
 * @is_stale: Indicates whether this frame is stale.
 * @zero_wait_count_rx: Indicates whether this frame's wait count was
 * zero when received by host
 * @immediate_delivery: Indicates whether this frame can be delivered
 * immediately to the upper layers
 * @is_error: Indicates whether any error occurred during processing this frame
 * @ts_last_released_frame: Stores the global time stamp for the last frame
 * removed from the reorder list
 * @list_size_rx: Size of the reorder list when this frame is received (before
 * updating the list based on this frame).
 * @list_insertion_pos: Position in the reorder list where this frame is going
 * to get inserted (Applicable for only host consumed frames)
 */
struct reo_ingress_debug_frame_info {
	uint8_t link_id;
	uint16_t mgmt_pkt_ctr;
	uint32_t global_timestamp;
	enum mgmt_rx_reo_frame_descriptor_type type;
	uint64_t ingress_timestamp;
	uint64_t ingress_duration;
	struct mgmt_rx_reo_wait_count wait_count;
	bool is_queued;
	bool is_stale;
	bool zero_wait_count_rx;
	bool immediate_delivery;
	bool is_error;
	struct mgmt_rx_reo_global_ts_info ts_last_released_frame;
	int16_t list_size_rx;
	int16_t list_insertion_pos;
};

/**
 * struct reo_egress_debug_frame_info - Debug information about a frame
 * leaving the reorder module
 * @is_delivered: Indicates whether the frame is delivered to upper layers
 * @is_premature_delivery: Indicates whether the frame is delivered
 * prematurely
 * @link_id: link id
 * @mgmt_pkt_ctr: management packet counter
 * @global_timestamp: MLO global time stamp
 * @ingress_timestamp: Host time stamp when the frame enters the reorder module
 * @insertion_ts: Host time stamp when the frame is inserted into the reorder
 * list
 * @egress_timestamp: Host time stamp just before delivery of the frame to upper
 * layer
 * @egress_duration: Duration in us taken by the upper layer to process
 * the frame.
 * @removal_ts: Host time stamp when this entry is removed from the list
 * @wait_count: Wait count calculated for the current frame
 * @release_reason: Reason for delivering the frame to upper layers
 */
struct reo_egress_debug_frame_info {
	bool is_delivered;
	bool is_premature_delivery;
	uint8_t link_id;
	uint16_t mgmt_pkt_ctr;
	uint32_t global_timestamp;
	uint64_t ingress_timestamp;
	uint64_t insertion_ts;
	uint64_t egress_timestamp;
	uint64_t egress_duration;
	uint64_t removal_ts;
	struct mgmt_rx_reo_wait_count wait_count;
	uint8_t release_reason;
};

/**
 * struct reo_ingress_frame_stats - Structure to store statistics related to
 * incoming frames
 * @ingress_count: Number of frames entering reo module
 * @queued_count: Number of frames queued to reorder list
 * @zero_wait_count_rx_count: Number of frames for which wait count is
 * zero when received at host
 * @immediate_delivery_count: Number of frames which can be delivered
 * immediately to the upper layers without reordering. A frame can be
 * immediately delivered if it has wait count of zero on reception at host
 * and the global time stamp is less than or equal to the global time
 * stamp of all the frames in the reorder list. Such frames would get
 * inserted to the head of the reorder list and gets delivered immediately
 * to the upper layers.
 * @stale_count: Number of stale frames. Any frame older than the
 * last frame delivered to upper layer is a stale frame.
 * @error_count: Number of frames dropped due to error occurred
 * within the reorder module
 */
struct reo_ingress_frame_stats {
	uint64_t ingress_count
		[MAX_MLO_LINKS][MGMT_RX_REO_FRAME_DESC_TYPE_MAX];
	uint64_t queued_count[MAX_MLO_LINKS];
	uint64_t zero_wait_count_rx_count[MAX_MLO_LINKS];
	uint64_t immediate_delivery_count[MAX_MLO_LINKS];
	uint64_t stale_count[MAX_MLO_LINKS]
			    [MGMT_RX_REO_FRAME_DESC_TYPE_MAX];
	uint64_t error_count[MAX_MLO_LINKS]
			    [MGMT_RX_REO_FRAME_DESC_TYPE_MAX];
};

/**
 * struct reo_egress_frame_stats - Structure to store statistics related to
 * outgoing frames
 * @delivery_attempts_count: Number of attempts to deliver management
 * frames to upper layers
 * @delivery_success_count: Number of successful management frame
 * deliveries to upper layer
 * @premature_delivery_count:  Number of frames delivered
 * prematurely. Premature delivery is the delivery of a management frame
 * to the upper layers even before its wait count is reaching zero.
 * @delivery_count: Number frames delivered successfully for
 * each link and release  reason.
 */
struct reo_egress_frame_stats {
	uint64_t delivery_attempts_count[MAX_MLO_LINKS];
	uint64_t delivery_success_count[MAX_MLO_LINKS];
	uint64_t premature_delivery_count[MAX_MLO_LINKS];
	uint64_t delivery_count[MAX_MLO_LINKS]
			       [MGMT_RX_REO_RELEASE_REASON_MAX];
};

/**
 * struct reo_ingress_debug_info - Circular array to store the
 * debug information about the frames entering the reorder algorithm.
 * @frame_list: Circular array to store the debug info about frames
 * @next_index: The index at which information about next frame will be logged
 * @wrap_aroud: Flag to indicate whether wrap around occurred when logging
 * debug information to @frame_list
 * @stats: Stats related to incoming frames
 */
struct reo_ingress_debug_info {
	struct reo_ingress_debug_frame_info
			frame_list[MGMT_RX_REO_INGRESS_FRAME_DEBUG_ENTRIES_MAX];
	uint32_t next_index;
	bool wrap_aroud;
	struct reo_ingress_frame_stats stats;
};

/**
 * struct reo_egress_debug_info - Circular array to store the
 * debug information about the frames leaving the reorder module.
 * @debug_info: Circular array to store the debug info
 * @next_index: The index at which information about next frame will be logged
 * @wrap_aroud: Flag to indicate whether wrap around occurred when logging
 * debug information to @frame_list
 * @stats: Stats related to outgoing frames
 */
struct reo_egress_debug_info {
	struct reo_egress_debug_frame_info
			frame_list[MGMT_RX_REO_EGRESS_FRAME_DEBUG_ENTRIES_MAX];
	uint32_t next_index;
	bool wrap_aroud;
	struct reo_egress_frame_stats stats;
};
#endif /* WLAN_MGMT_RX_REO_DEBUG_SUPPORT */

/**
 * struct mgmt_rx_reo_context - This structure holds the info required for
 * management rx-reordering. Reordering is done across all the psocs.
 * So there should be only one instance of this structure defined.
 * @reo_list: Linked list used for reordering
 * @reo_algo_entry_lock: Spin lock to protect reo algorithm entry critical
 * section execution
 * @frame_release_lock: Spin lock to serialize the frame delivery to the
 * upper layers. This could prevent race conditions like the one given in
 * the following example.
 * Lets take an example of 2 links (Link A & B) and each has received
 * a management frame A1(deauth) and B1(auth) such that MLO global time
 * stamp of A1 < MLO global time stamp of B1. Host is concurrently
 * executing "mgmt_rx_reo_list_release_entries" for A1 and B1 in
 * 2 different CPUs. It is possible that frame B1 gets processed by
 * upper layers before frame A1 and this could result in unwanted
 * disconnection. Hence it is required to serialize the delivery
 * of management frames to upper layers in the strict order of MLO
 * global time stamp.
 * @sim_context: Management rx-reorder simulation context
 * @ingress_frame_debug_info: Debug object to log incoming frames
 * @egress_frame_debug_info: Debug object to log outgoing frames
 * @simulation_in_progress: Flag to indicate whether simulation is
 * in progress
 */
struct mgmt_rx_reo_context {
	struct mgmt_rx_reo_list reo_list;
	qdf_spinlock_t reo_algo_entry_lock;
	qdf_spinlock_t frame_release_lock;
#ifdef WLAN_MGMT_RX_REO_SIM_SUPPORT
	struct mgmt_rx_reo_sim_context sim_context;
#endif /* WLAN_MGMT_RX_REO_SIM_SUPPORT */
#ifdef WLAN_MGMT_RX_REO_DEBUG_SUPPORT
	struct  reo_ingress_debug_info ingress_frame_debug_info;
	struct  reo_egress_debug_info egress_frame_debug_info;
#endif /* WLAN_MGMT_RX_REO_DEBUG_SUPPORT */
	bool simulation_in_progress;
};

/**
 * struct mgmt_rx_reo_frame_descriptor - Frame Descriptor used to describe
 * a management frame in mgmt rx reo module.
 * @type: Frame descriptor type
 * @nbuf: nbuf corresponding to this frame
 * @rx_params: Management rx event parameters
 * @wait_count: Wait counts for the frame
 * @ingress_timestamp: Host time stamp when the frames enters the reorder
 * algorithm
 * @is_stale: Indicates whether this frame is stale. Any frame older than the
 * last frame delivered to upper layer is a stale frame. Stale frames should not
 * be delivered to the upper layers. These frames can be discarded after
 * updating the host snapshot and wait counts of entries currently residing in
 * the reorder list.
 * @zero_wait_count_rx: Indicates whether this frame's wait count was
 * zero when received by host
 * @immediate_delivery: Indicates whether this frame can be delivered
 * immediately to the upper layers
 * @list_size_rx: Size of the reorder list when this frame is received (before
 * updating the list based on this frame).
 * @list_insertion_pos: Position in the reorder list where this frame is going
 * to get inserted (Applicable for only host consumed frames)
 */
struct mgmt_rx_reo_frame_descriptor {
	enum mgmt_rx_reo_frame_descriptor_type type;
	qdf_nbuf_t nbuf;
	struct mgmt_rx_event_params *rx_params;
	struct mgmt_rx_reo_wait_count wait_count;
	uint64_t ingress_timestamp;
	bool is_stale;
	bool zero_wait_count_rx;
	bool immediate_delivery;
	int16_t list_size_rx;
	int16_t list_insertion_pos;
};

/**
 * mgmt_rx_reo_get_context_from_reo_list() - Helper API to get pointer to
 * management rx reorder context from pointer to management reorder list
 * @reo_list: Pointer to management rx reorder list
 *
 * Return: Pointer to management rx reorder context
 */
static inline struct mgmt_rx_reo_context *
mgmt_rx_reo_get_context_from_reo_list(const struct mgmt_rx_reo_list *reo_list) {
	qdf_assert_always(reo_list);

	return qdf_container_of(reo_list, struct mgmt_rx_reo_context,
				reo_list);
}

/**
 * mgmt_rx_reo_get_global_ts() - Helper API to get global time stamp
 * corresponding to the mgmt rx event
 * @rx_params: Management rx event params
 *
 * Return: global time stamp corresponding to the mgmt rx event
 */
static inline uint32_t
mgmt_rx_reo_get_global_ts(struct mgmt_rx_event_params *rx_params)
{
	qdf_assert_always(rx_params);
	qdf_assert_always(rx_params->reo_params);

	return rx_params->reo_params->global_timestamp;
}

/**
 * mgmt_rx_reo_get_pkt_counter() - Helper API to get packet counter
 * corresponding to the mgmt rx event
 * @rx_params: Management rx event params
 *
 * Return: Management packet counter corresponding to the mgmt rx event
 */
static inline uint16_t
mgmt_rx_reo_get_pkt_counter(struct mgmt_rx_event_params *rx_params)
{
	qdf_assert_always(rx_params);
	qdf_assert_always(rx_params->reo_params);

	return rx_params->reo_params->mgmt_pkt_ctr;
}

/**
 * mgmt_rx_reo_get_link_id() - Helper API to get link id corresponding to the
 * mgmt rx event
 * @rx_params: Management rx event params
 *
 * Return: link id corresponding to the mgmt rx event
 */
static inline uint8_t
mgmt_rx_reo_get_link_id(struct mgmt_rx_event_params *rx_params)
{
	qdf_assert_always(rx_params);
	qdf_assert_always(rx_params->reo_params);

	return rx_params->reo_params->link_id;
}

/**
 * mgmt_rx_reo_get_pdev_id() - Helper API to get pdev id corresponding to the
 * mgmt rx event
 * @rx_params: Management rx event params
 *
 * Return: pdev id corresponding to the mgmt rx event
 */
static inline uint8_t
mgmt_rx_reo_get_pdev_id(struct mgmt_rx_event_params *rx_params)
{
	qdf_assert_always(rx_params);
	qdf_assert_always(rx_params->reo_params);

	return rx_params->reo_params->pdev_id;
}

/**
 * mgmt_rx_reo_init_context() - Initialize the management rx-reorder context
 *
 * API to initialize the global management rx-reorder context object.
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
mgmt_rx_reo_init_context(void);

/**
 * mgmt_rx_reo_deinit_context() - De initialize the management rx-reorder
 * context
 *
 * API to de initialize the global management rx-reorder context object.
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
mgmt_rx_reo_deinit_context(void);

/**
 * mgmt_rx_reo_is_simulation_in_progress() - API to check whether
 * simulation is in progress
 *
 * Return: true if simulation is in progress, else false
 */
bool
mgmt_rx_reo_is_simulation_in_progress(void);

/**
 * mgmt_rx_reo_print_ingress_frame_debug_info() - Helper API to print
 * stats related to incoming management frames
 *
 * This API prints stats related to management frames entering management
 * Rx reorder module.
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
mgmt_rx_reo_print_ingress_frame_debug_info(void);

/**
 * mgmt_rx_reo_print_egress_frame_debug_info() - Helper API to print
 * stats related to outgoing management frames
 *
 * This API prints stats related to management frames exiting management
 * Rx reorder module.
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
mgmt_rx_reo_print_egress_frame_debug_info(void);

#ifdef WLAN_MGMT_RX_REO_SIM_SUPPORT
/**
 * mgmt_rx_reo_sim_start() - Helper API to start management Rx reorder
 * simulation
 *
 * This API starts the simulation framework which mimics the management frame
 * generation by target. MAC HW is modelled as a kthread. FW and host layers
 * are modelled as an ordered work queues.
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
mgmt_rx_reo_sim_start(void);

/**
 * mgmt_rx_reo_sim_stop() - Helper API to stop management Rx reorder
 * simulation
 *
 * This API stops the simulation framework which mimics the management frame
 * generation by target. MAC HW is modelled as a kthread. FW and host layers
 * are modelled as an ordered work queues.
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
mgmt_rx_reo_sim_stop(void);

/**
 * mgmt_rx_reo_sim_process_rx_frame() - API to process the management frame
 * in case of simulation
 * @pdev: pointer to pdev object
 * @buf: pointer to management frame buffer
 * @mgmt_rx_params: pointer to management frame parameters
 *
 * This API validates whether the reo algorithm has reordered frames correctly.
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
mgmt_rx_reo_sim_process_rx_frame(struct wlan_objmgr_pdev *pdev,
				 qdf_nbuf_t buf,
				 struct mgmt_rx_event_params *mgmt_rx_params);

/**
 * mgmt_rx_reo_sim_get_snapshot_address() - Get snapshot address
 * @pdev: pointer to pdev
 * @id: snapshot identifier
 * @address: pointer to snapshot address
 *
 * Helper API to get address of snapshot @id for pdev @pdev. For simulation
 * purpose snapshots are allocated in the simulation context object.
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
mgmt_rx_reo_sim_get_snapshot_address(
			struct wlan_objmgr_pdev *pdev,
			enum mgmt_rx_reo_shared_snapshot_id id,
			struct mgmt_rx_reo_snapshot **address);

/**
 * mgmt_rx_reo_sim_pdev_object_create_notification() - pdev create handler for
 * management rx-reorder simulation framework
 * @pdev: pointer to pdev object
 *
 * This function gets called from object manager when pdev is being created and
 * builds the link id to pdev map in simulation context object.
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
mgmt_rx_reo_sim_pdev_object_create_notification(struct wlan_objmgr_pdev *pdev);

/**
 * mgmt_rx_reo_sim_pdev_object_destroy_notification() - pdev destroy handler for
 * management rx-reorder simulation framework
 * @pdev: pointer to pdev object
 *
 * This function gets called from object manager when pdev is being destroyed
 * and destroys the link id to pdev map in simulation context.
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
mgmt_rx_reo_sim_pdev_object_destroy_notification(struct wlan_objmgr_pdev *pdev);

/**
 * mgmt_rx_reo_sim_get_mlo_link_id_from_pdev() - Helper API to get the MLO HW
 * link id from the pdev object.
 * @pdev: Pointer to pdev object
 *
 * This API is applicable for simulation only. A static map from MLO HW link id
 * to the pdev object is created at the init time. This API uses the map to
 * find the MLO HW link id for a given pdev.
 *
 * Return: On success returns the MLO HW link id corresponding to the pdev
 * object. On failure returns -1.
 */
int8_t
mgmt_rx_reo_sim_get_mlo_link_id_from_pdev(struct wlan_objmgr_pdev *pdev);

/**
 * mgmt_rx_reo_sim_get_pdev_from_mlo_link_id() - Helper API to get the pdev
 * object from the MLO HW link id.
 * @mlo_link_id: MLO HW link id
 * @refdbgid: Reference debug id
 *
 * This API is applicable for simulation only. A static map from MLO HW link id
 * to the pdev object is created at the init time. This API uses the map to
 * find the pdev object from the MLO HW link id.
 *
 * Return: On success returns the pdev object corresponding to the MLO HW
 * link id. On failure returns NULL.
 */
struct wlan_objmgr_pdev *
mgmt_rx_reo_sim_get_pdev_from_mlo_link_id(uint8_t mlo_link_id,
					  wlan_objmgr_ref_dbgid refdbgid);
#endif /* WLAN_MGMT_RX_REO_SIM_SUPPORT */

/**
 * is_mgmt_rx_reo_required() - Whether MGMT REO required for this frame/event
 * @pdev: pdev for which this frame/event is intended
 * @desc: Descriptor corresponding to this frame/event
 *
 * Return: true if REO is required; else false
 */
static inline bool is_mgmt_rx_reo_required(
			struct wlan_objmgr_pdev *pdev,
			struct mgmt_rx_reo_frame_descriptor *desc)
{
	/**
	 * NOTE: Implementing a simple policy based on INI and WMI serive bit
	 * for now. Finer policies like checking whether this pdev has
	 * any MLO VAPs or checking the frame type can be implemented later.
	 */
	return wlan_mgmt_rx_reo_is_feature_enabled_at_pdev(pdev);
}

/**
 * wlan_mgmt_rx_reo_algo_entry() - Entry point to the MGMT Rx REO algorithm for
 * a given MGMT frame/event.
 * @pdev: pdev for which this frame/event is intended
 * @desc: Descriptor corresponding to this frame/event
 * @is_queued: Whether this frame/event is queued in the REO list
 *
 * Return: QDF_STATUS of operation
 */
QDF_STATUS
wlan_mgmt_rx_reo_algo_entry(struct wlan_objmgr_pdev *pdev,
			    struct mgmt_rx_reo_frame_descriptor *desc,
			    bool *is_queued);

/**
 * mgmt_rx_reo_list_max_size_exceeded() - Helper API to check whether
 * list has exceeded the maximum configured size
 * @reo_list: Pointer to reorder list
 *
 * This API expects the caller to acquire the spin lock protecting the reorder
 * list.
 *
 * Return: true if reorder list has exceeded the max size
 */
static inline bool
mgmt_rx_reo_list_max_size_exceeded(struct mgmt_rx_reo_list *reo_list)
{
	return (qdf_list_size(&reo_list->list) > reo_list->max_list_size);
}

/**
 * mgmt_rx_reo_validate_mlo_hw_link_info() - Validate the MLO HW link info
 * obtained from the global shared memory arena
 * @psoc: Pointer to psoc object
 *
 * Validate the following MLO HW link related information extracted from
 * management Rx reorder related TLVs in global shared memory arena.
 *         1. Number of active MLO HW links
 *         2. Valid MLO HW link bitmap
 *
 * Return: QDF_STATUS of operation
 */
QDF_STATUS
mgmt_rx_reo_validate_mlo_hw_link_info(struct wlan_objmgr_psoc *psoc);
#endif /* WLAN_MGMT_RX_REO_SUPPORT */
#endif /* _WLAN_MGMT_TXRX_RX_REO_I_H */
