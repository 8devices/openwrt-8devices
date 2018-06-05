/**
 * Phases for control transfers.
 */
typedef enum dwc_otg_control_phase {
	DWC_OTG_CONTROL_SETUP,
	DWC_OTG_CONTROL_DATA,
	DWC_OTG_CONTROL_STATUS
} dwc_otg_control_phase_e;

/** Transaction types. */
typedef enum dwc_otg_transaction_type {
	DWC_OTG_TRANSACTION_NONE,
	DWC_OTG_TRANSACTION_PERIODIC,
	DWC_OTG_TRANSACTION_NON_PERIODIC,
	DWC_OTG_TRANSACTION_ALL
} dwc_otg_transaction_type_e;

/**
 * A Queue Transfer Descriptor (QTD) holds the state of a bulk, control,
 * interrupt, or isochronous transfer. A single QTD is created for each URB
 * (of one of these types) submitted to the HCD. The transfer associated with
 * a QTD may require one or multiple transactions.
 *
 * A QTD is linked to a Queue Head, which is entered in either the
 * non-periodic or periodic schedule for execution. When a QTD is chosen for
 * execution, some or all of its transactions may be executed. After
 * execution, the state of the QTD is updated. The QTD may be retired if all
 * its transactions are complete or if an error occurred. Otherwise, it
 * remains in the schedule so more transactions can be executed later.
 */
typedef struct dwc_otg_qtd {
	/**
	 * Determines the PID of the next data packet for the data phase of
	 * control transfers. Ignored for other transfer types.<br>
	 * One of the following values:
	 *	- DWC_OTG_HC_PID_DATA0
	 *	- DWC_OTG_HC_PID_DATA1
	 */
	uint8_t			data_toggle;

	/** Current phase for control transfers (Setup, Data, or Status). */
	dwc_otg_control_phase_e	control_phase;

	/** Keep track of the current split type
	 * for FS/LS endpoints on a HS Hub */
	uint8_t                 complete_split;

	/** How many bytes transferred during SSPLIT OUT */
	uint32_t                ssplit_out_xfer_count;

	/**
	 * Holds the number of bus errors that have occurred for a transaction
	 * within this transfer.
	 */
	uint8_t 		error_count;

	/**
	 * Index of the next frame descriptor for an isochronous transfer. A
	 * frame descriptor describes the buffer position and length of the
	 * data to be transferred in the next scheduled (micro)frame of an
	 * isochronous transfer. It also holds status for that transaction.
	 * The frame index starts at 0.
	 */
	int			isoc_frame_index;

	/** Position of the ISOC split on full/low speed */
	uint8_t                 isoc_split_pos;

	/** Position of the ISOC split in the buffer for the current frame */
	uint16_t                isoc_split_offset;

	/** URB for this transfer */
	struct urb 		*urb;

	/** This list of QTDs */
	struct list_head  	qtd_list_entry;

} dwc_otg_qtd_t;

/**
 * A Queue Head (QH) holds the static characteristics of an endpoint and
 * maintains a list of transfers (QTDs) for that endpoint. A QH structure may
 * be entered in either the non-periodic or periodic schedule.
 */
typedef struct dwc_otg_qh {
	/**
	 * Endpoint type.
	 * One of the following values:
	 * 	- USB_ENDPOINT_XFER_CONTROL
	 *	- USB_ENDPOINT_XFER_ISOC
	 *	- USB_ENDPOINT_XFER_BULK
	 *	- USB_ENDPOINT_XFER_INT
	 */
	uint8_t 		ep_type;
	uint8_t 		ep_is_in;

	/** wMaxPacketSize Field of Endpoint Descriptor. */
	uint16_t		maxp;

	/**
	 * Determines the PID of the next data packet for non-control
	 * transfers. Ignored for control transfers.<br>
	 * One of the following values:
	 *	- DWC_OTG_HC_PID_DATA0
	 * 	- DWC_OTG_HC_PID_DATA1
	 */
	uint8_t			data_toggle;

	/** Ping state if 1. */
	uint8_t 		ping_state;

	/**
	 * List of QTDs for this QH.
	 */
	struct list_head 	qtd_list;

	/** Host channel currently processing transfers for this QH. */
	dwc_hc_t		*channel;

	/** QTD currently assigned to a host channel for this QH. */
	dwc_otg_qtd_t		*qtd_in_process;

	/** Full/low speed endpoint on high-speed hub requires split. */
	uint8_t                 do_split;

	/** @name Periodic schedule information */
	/** @{ */

	/** Bandwidth in microseconds per (micro)frame. */
	uint8_t			usecs;

	/** Interval between transfers in (micro)frames. */
	uint16_t		interval;

	/**
	 * (micro)frame to initialize a periodic transfer. The transfer
	 * executes in the following (micro)frame.
	 */
	uint16_t		sched_frame;

	/** (micro)frame at which last start split was initialized. */
	uint16_t		start_split_frame;

	/** @} */

	/** Entry for QH in either the periodic or non-periodic schedule. */
	struct list_head        qh_list_entry;
} dwc_otg_qh_t;

/**
 * This structure holds the state of the HCD, including the non-periodic and
 * periodic schedules.
 */
typedef struct dwc_otg_hcd {

	/** DWC OTG Core Interface Layer */
	dwc_otg_core_if_t       *core_if;

	/** Internal DWC HCD Flags */
	volatile union dwc_otg_hcd_internal_flags {
		uint32_t d32;
		struct {
#if 1  //cathy have
			//eason
			unsigned reserved : 26;
			unsigned port_connect_status : 1;
			unsigned port_reset_change : 1;
			unsigned port_over_current_change : 1;
			unsigned port_suspend_change : 1;
			unsigned port_enable_change : 1;
			unsigned port_connect_status_change : 1;
#endif
		} b;
	} flags;

	/**
	 * Inactive items in the non-periodic schedule. This is a list of
	 * Queue Heads. Transfers associated with these Queue Heads are not
	 * currently assigned to a host channel.
	 */
	struct list_head 	non_periodic_sched_inactive;

	/**
	 * Active items in the non-periodic schedule. This is a list of
	 * Queue Heads. Transfers associated with these Queue Heads are
	 * currently assigned to a host channel.
	 */
	struct list_head 	non_periodic_sched_active;

	/**
	 * Pointer to the next Queue Head to process in the active
	 * non-periodic schedule.
	 */
	struct list_head 	*non_periodic_qh_ptr;

	/**
	 * Inactive items in the periodic schedule. This is a list of QHs for
	 * periodic transfers that are _not_ scheduled for the next frame.
	 * Each QH in the list has an interval counter that determines when it
	 * needs to be scheduled for execution. This scheduling mechanism
	 * allows only a simple calculation for periodic bandwidth used (i.e.
	 * must assume that all periodic transfers may need to execute in the
	 * same frame). However, it greatly simplifies scheduling and should
	 * be sufficient for the vast majority of OTG hosts, which need to
	 * connect to a small number of peripherals at one time.
	 *
	 * Items move from this list to periodic_sched_ready when the QH
	 * interval counter is 0 at SOF.
	 */
	struct list_head	periodic_sched_inactive;

	/**
	 * List of periodic QHs that are ready for execution in the next
	 * frame, but have not yet been assigned to host channels.
	 *
	 * Items move from this list to periodic_sched_assigned as host
	 * channels become available during the current frame.
	 */
	struct list_head	periodic_sched_ready;

	/**
	 * List of periodic QHs to be executed in the next frame that are
	 * assigned to host channels.
	 *
	 * Items move from this list to periodic_sched_queued as the
	 * transactions for the QH are queued to the DWC_otg controller.
	 */
	struct list_head	periodic_sched_assigned;

	/**
	 * List of periodic QHs that have been queued for execution.
	 *
	 * Items move from this list to either periodic_sched_inactive or
	 * periodic_sched_ready when the channel associated with the transfer
	 * is released. If the interval for the QH is 1, the item moves to
	 * periodic_sched_ready because it must be rescheduled for the next
	 * frame. Otherwise, the item moves to periodic_sched_inactive.
	 */
	struct list_head	periodic_sched_queued;

	/**
	 * Total bandwidth claimed so far for periodic transfers. This value
	 * is in microseconds per (micro)frame. The assumption is that all
	 * periodic transfers may occur in the same (micro)frame.
	 */
	uint16_t		periodic_usecs;

	/**
	 * Frame number read from the core at SOF. The value ranges from 0 to
	 * DWC_HFNUM_MAX_FRNUM.
	 */
	uint16_t		frame_number;

	/**
	 * Free host channels in the controller. This is a list of
	 * dwc_hc_t items.
	 */
	struct list_head 	free_hc_list;

	/**
	 * Number of host channels assigned to periodic transfers. Currently
	 * assuming that there is a dedicated host channel for each periodic
	 * transaction and at least one host channel available for
	 * non-periodic transactions.
	 */
	int			periodic_channels;

	/**
	 * Number of host channels assigned to non-periodic transfers.
	 */
	int			non_periodic_channels;

	/**
	 * Array of pointers to the host channel descriptors. Allows accessing
	 * a host channel descriptor given the host channel number. This is
	 * useful in interrupt handlers.
	 */
	dwc_hc_t		*hc_ptr_array[MAX_EPS_CHANNELS];

	/**
	 * Buffer to use for any data received during the status phase of a
	 * control transfer. Normally no data is transferred during the status
	 * phase. This buffer is used as a bit bucket.
	 */
	uint8_t			*status_buf;

	/**
	 * DMA address for status_buf.
	 */
	dma_addr_t		status_buf_dma;
#define DWC_OTG_HCD_STATUS_BUF_SIZE 64

	/**
	 * Structure to allow starting the HCD in a non-interrupt context
	 * during an OTG role change.
	 */
	struct work_struct	start_work;

	/**
	 * Connection timer. An OTG host must display a message if the device
	 * does not connect. Started when the VBus power is turned on via
	 * sysfs attribute "buspower".
	 */
        struct timer_list 	conn_timer;

	/* Tasket to do a reset */
	struct tasklet_struct   *reset_tasklet;
#if 1 //cathy have
//cathy
	atomic_t		out_channel_num;
	atomic_t		scanning;
	spinlock_t		lock;
	int				disconn_cnt;
	struct timer_list	disconn_cnt_timer;
#endif

#ifdef DEBUG
	uint32_t 		frrem_samples;
	uint64_t 		frrem_accum;

	uint32_t		hfnum_7_samples_a;
	uint64_t		hfnum_7_frrem_accum_a;
	uint32_t		hfnum_0_samples_a;
	uint64_t		hfnum_0_frrem_accum_a;
	uint32_t		hfnum_other_samples_a;
	uint64_t		hfnum_other_frrem_accum_a;

	uint32_t		hfnum_7_samples_b;
	uint64_t		hfnum_7_frrem_accum_b;
	uint32_t		hfnum_0_samples_b;
	uint64_t		hfnum_0_frrem_accum_b;
	uint32_t		hfnum_other_samples_b;
	uint64_t		hfnum_other_frrem_accum_b;
#endif

} dwc_otg_hcd_t;
