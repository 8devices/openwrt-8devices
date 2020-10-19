/*
 ***************************************************************************
 * Copyright (c) 2020, The Linux Foundation. All rights reserved.
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
 ***************************************************************************
 */

#include <linux/version.h>
#include <linux/types.h>
#include <linux/ip.h>
#include <linux/module.h>
#include <linux/skbuff.h>
#include <linux/debugfs.h>
#include <linux/string.h>
#include <linux/netfilter_bridge.h>
#include <net/ip.h>
#include <linux/inet.h>

#include <sp_api.h>

/*
 * Debug output levels
 * 0 = OFF
 * 1 = ASSERTS / ERRORS
 * 2 = 1 + WARN
 * 3 = 2 + INFO
 * 4 = 3 + TRACE
 */
#define DEBUG_LEVEL ECM_CLASSIFIER_EMESH_DEBUG_LEVEL

#include "ecm_types.h"
#include "ecm_db_types.h"
#include "ecm_state.h"
#include "ecm_tracker.h"
#include "ecm_front_end_types.h"
#include "ecm_classifier.h"
#include "ecm_db.h"

/*
 * Magic numbers
 */
#define ECM_CLASSIFIER_EMESH_INSTANCE_MAGIC 0xFECA

/*
 * struct ecm_classifier_emesh_instance
 * 	State to allow tracking of dynamic qos for a connection
 */
struct ecm_classifier_emesh_instance {
	struct ecm_classifier_instance base;			/* Base type */

	struct ecm_classifier_emesh_instance *next;		/* Next classifier state instance (for accouting and reporting purposes) */
	struct ecm_classifier_emesh_instance *prev;		/* Next classifier state instance (for accouting and reporting purposes) */

	uint32_t ci_serial;					/* RO: Serial of the connection */
	uint32_t pcp[ECM_CONN_DIR_MAX];				/* PCP values for the connections */
	struct ecm_classifier_process_response process_response;/* Last process response computed */

	int refs;						/* Integer to trap we never go negative */
	uint8_t packet_seen[ECM_CONN_DIR_MAX];				/* Per direction packet seen flag */

#if (DEBUG_LEVEL > 0)
	uint16_t magic;
#endif
};

/*
 * Operational control
 */
static uint32_t ecm_classifier_emesh_enabled = 0;			/* Operational behaviour */

/*
 * Management thread control
 */
static bool ecm_classifier_emesh_terminate_pending = false;	/* True when the user wants us to terminate */

/*
 * Debugfs dentry object.
 */
static struct dentry *ecm_classifier_emesh_dentry;

/*
 * Locking of the classifier structures
 */
static DEFINE_SPINLOCK(ecm_classifier_emesh_lock);			/* Protect SMP access. */

/*
 * List of our classifier instances
 */
static struct ecm_classifier_emesh_instance *ecm_classifier_emesh_instances = NULL;
								/* list of all active instances */
static int ecm_classifier_emesh_count = 0;			/* Tracks number of instances allocated */

/*
 * ecm_classifier_emesh_ref()
 *	Ref
 */
static void ecm_classifier_emesh_ref(struct ecm_classifier_instance *ci)
{
	struct ecm_classifier_emesh_instance *cemi;
	cemi = (struct ecm_classifier_emesh_instance *)ci;

	DEBUG_CHECK_MAGIC(cemi, ECM_CLASSIFIER_EMESH_INSTANCE_MAGIC, "%px: magic failed\n", cemi);
	spin_lock_bh(&ecm_classifier_emesh_lock);
	cemi->refs++;
	DEBUG_TRACE("%px: cemi ref %d\n", cemi, cemi->refs);
	DEBUG_ASSERT(cemi->refs > 0, "%px: ref wrap\n", cemi);
	spin_unlock_bh(&ecm_classifier_emesh_lock);
}

/*
 * ecm_classifier_emesh_deref()
 *	Deref
 */
static int ecm_classifier_emesh_deref(struct ecm_classifier_instance *ci)
{
	struct ecm_classifier_emesh_instance *cemi;
	cemi = (struct ecm_classifier_emesh_instance *)ci;

	DEBUG_CHECK_MAGIC(cemi, ECM_CLASSIFIER_EMESH_INSTANCE_MAGIC, "%px: magic failed\n", cemi);
	spin_lock_bh(&ecm_classifier_emesh_lock);
	cemi->refs--;
	DEBUG_ASSERT(cemi->refs >= 0, "%px: refs wrapped\n", cemi);
	DEBUG_TRACE("%px: EMESH classifier deref %d\n", cemi, cemi->refs);
	if (cemi->refs) {
		int refs = cemi->refs;
		spin_unlock_bh(&ecm_classifier_emesh_lock);
		return refs;
	}

	/*
	 * Object to be destroyed
	 */
	ecm_classifier_emesh_count--;
	DEBUG_ASSERT(ecm_classifier_emesh_count >= 0, "%px: ecm_classifier_emesh_count wrap\n", cemi);

	/*
	 * UnLink the instance from our list
	 */
	if (cemi->next) {
		cemi->next->prev = cemi->prev;
	}

	if (cemi->prev) {
		cemi->prev->next = cemi->next;
	} else {
		DEBUG_ASSERT(ecm_classifier_emesh_instances == cemi, "%px: list bad %px\n", cemi, ecm_classifier_emesh_instances);
		ecm_classifier_emesh_instances = cemi->next;
	}
	spin_unlock_bh(&ecm_classifier_emesh_lock);

	/*
	 * Final
	 */
	DEBUG_INFO("%px: Final EMESH classifier instance\n", cemi);
	kfree(cemi);

	return 0;
}

/*
 * ecm_classifier_emesh_is_bidi_packet_seen()
 *	Return true if both direction packets are seen.
 */
static inline bool ecm_classifier_emesh_is_bidi_packet_seen(struct ecm_classifier_emesh_instance *cemi)
{
	return ((cemi->packet_seen[ECM_CONN_DIR_FLOW] == true) && (cemi->packet_seen[ECM_CONN_DIR_RETURN] == true));
}

/*
 * ecm_classifier_emesh_fill_pcp()
 *	Save the PCP value in the classifier instance.
 */
static void ecm_classifier_emesh_fill_pcp(struct ecm_classifier_emesh_instance *cemi,
		 ecm_tracker_sender_type_t sender, enum ip_conntrack_info ctinfo,
		 struct sk_buff *skb)
{
	if (sender == ECM_TRACKER_SENDER_TYPE_SRC) {
		cemi->pcp[ECM_CONN_DIR_FLOW] = skb->priority;
		cemi->packet_seen[ECM_CONN_DIR_FLOW] = true;
	} else {
		cemi->pcp[ECM_CONN_DIR_RETURN] = skb->priority;
		cemi->packet_seen[ECM_CONN_DIR_RETURN] = true;
	}
}

/*
 * ecm_classifier_emesh_process()
 *	Process new data for connection
 */
static void ecm_classifier_emesh_process(struct ecm_classifier_instance *aci, ecm_tracker_sender_type_t sender,
						struct ecm_tracker_ip_header *ip_hdr, struct sk_buff *skb,
						struct ecm_classifier_process_response *process_response)
{
	struct ecm_classifier_emesh_instance *cemi;
	ecm_classifier_relevence_t relevance;
	struct ecm_db_connection_instance *ci = NULL;
	struct ecm_front_end_connection_instance *feci;
	ecm_front_end_acceleration_mode_t accel_mode;
	uint32_t became_relevant = 0;
	struct nf_conn *ct;
	enum ip_conntrack_info ctinfo;
	int protocol, slow_pkts;

	cemi = (struct ecm_classifier_emesh_instance *)aci;
	DEBUG_CHECK_MAGIC(cemi, ECM_CLASSIFIER_EMESH_INSTANCE_MAGIC, "%px: magic failed\n", cemi);

	spin_lock_bh(&ecm_classifier_emesh_lock);
	relevance = cemi->process_response.relevance;

	/*
	 * Are we relevant?
	 * If the classifier is set as ir-relevant to the connection,
	 * the process response of the classifier instance was set from
	 * the earlier packets.
	 */
	if (relevance == ECM_CLASSIFIER_RELEVANCE_NO) {
		/*
		 * Lock still held
		 */
		goto emesh_classifier_out;
	}

	/*
	 * Yes or maybe relevant.
	 *
	 * Need to decide our relevance to this connection.
	 * We are only relevent to a connection iff:
	 * 1. We are enabled.
	 * 2. Connection can be accelerated.
	 * Any other condition and we are not and will stop analysing this connection.
	 */
	if (!ecm_classifier_emesh_enabled) {
		/*
		 * Lock still held
		 */
		cemi->process_response.relevance = ECM_CLASSIFIER_RELEVANCE_NO;
		goto emesh_classifier_out;
	}
	spin_unlock_bh(&ecm_classifier_emesh_lock);

	/*
	 * Can we accelerate?
	 */
	ci = ecm_db_connection_serial_find_and_ref(cemi->ci_serial);
	if (!ci) {
		DEBUG_TRACE("%px: No ci found for %u\n", cemi, cemi->ci_serial);
		spin_lock_bh(&ecm_classifier_emesh_lock);
		cemi->process_response.relevance = ECM_CLASSIFIER_RELEVANCE_NO;
		goto emesh_classifier_out;
	}

	feci = ecm_db_connection_front_end_get_and_ref(ci);
	accel_mode = feci->accel_state_get(feci);
	spin_lock_bh(&feci->lock);
	slow_pkts = feci->stats.slow_path_packets;
	spin_unlock_bh(&feci->lock);
	feci->deref(feci);
	protocol = ecm_db_connection_protocol_get(ci);
	ecm_db_connection_deref(ci);

	if (ECM_FRONT_END_ACCELERATION_NOT_POSSIBLE(accel_mode)) {
		spin_lock_bh(&ecm_classifier_emesh_lock);
		cemi->process_response.relevance = ECM_CLASSIFIER_RELEVANCE_NO;
		goto emesh_classifier_out;
	}

	/*
	 * Is there a valid conntrack?
	 */
	ct = nf_ct_get(skb, &ctinfo);
	if (!ct) {
		spin_lock_bh(&ecm_classifier_emesh_lock);
		cemi->process_response.relevance = ECM_CLASSIFIER_RELEVANCE_NO;
		goto emesh_classifier_out;
	}

	/*
	 * We are relevant to the connection.
	 * Set the process response to its default value, that is, to
	 * allow the acceleration.
	 */
	became_relevant = ecm_db_time_get();

	spin_lock_bh(&ecm_classifier_emesh_lock);
	cemi->process_response.relevance = ECM_CLASSIFIER_RELEVANCE_YES;
	cemi->process_response.became_relevant = became_relevant;

	cemi->process_response.process_actions |= ECM_CLASSIFIER_PROCESS_ACTION_ACCEL_MODE;
	cemi->process_response.accel_mode = ECM_CLASSIFIER_ACCELERATION_MODE_ACCEL;
	spin_unlock_bh(&ecm_classifier_emesh_lock);

	if (protocol == IPPROTO_TCP) {
		/*
		 * Stop the processing if both side packets are already seen.
		 * Above the process response is already set to allow the acceleration.
		 */
		if (ecm_classifier_emesh_is_bidi_packet_seen(cemi)) {
			spin_lock_bh(&ecm_classifier_emesh_lock);
			goto emesh_classifier_out;
		}

		/*
		 * Store the PCP value in the classifier instance and deny the
		 * acceleration if both side PCP value is not yet available.
		 */
		ecm_classifier_emesh_fill_pcp(cemi, sender, ctinfo, skb);
		if (!ecm_classifier_emesh_is_bidi_packet_seen(cemi)) {
			DEBUG_TRACE("%px: Both side PCP value is not yet picked\n", cemi);
			spin_lock_bh(&ecm_classifier_emesh_lock);
			cemi->process_response.accel_mode = ECM_CLASSIFIER_ACCELERATION_MODE_NO;
			goto emesh_classifier_out;
		}
	} else {
		/*
		 * If the acceleration delay option is enabled, we will wait
		 * until seeing both side traffic.
		 *
		 * There are 2 options:
		 * Option 1: Wait forever until to see the reply direction traffic
		 * Option 2: Wait for seeing N number of packets. If we still don't see reply,
		 * set the uni-directional values.
		 */
		if (ecm_classifier_accel_delay_pkts) {
			/*
			 * Stop the processing if both side packets are already seen.
			 * Above the process response is already set to allow the
			 * acceleration.
			 */
			if (ecm_classifier_emesh_is_bidi_packet_seen(cemi)) {
				spin_lock_bh(&ecm_classifier_emesh_lock);
				goto emesh_classifier_out;
			}

			/*
			 * Store the PCP value in the classifier instance and allow the
			 * acceleration if both side PCP value is not yet available.
			 */
			ecm_classifier_emesh_fill_pcp(cemi, sender, ctinfo, skb);
			if (ecm_classifier_emesh_is_bidi_packet_seen(cemi)) {
				DEBUG_TRACE("%px: Both side PCP value is picked\n", cemi);
				goto done;
			}

			/*
			 * Deny the acceleration if any of the below options holds true.
			 * For option 1, we wait forever
			 * For option 2, we wait until seeing ecm_classifier_accel_delay_pkts.
			 */
			if ((ecm_classifier_accel_delay_pkts == 1) || (slow_pkts < ecm_classifier_accel_delay_pkts)) {
				DEBUG_TRACE("%px: accel_delay_pkts: %d slow_pkts: %d accel is not allowed yet\n",
						cemi, ecm_classifier_accel_delay_pkts, slow_pkts);
				spin_lock_bh(&ecm_classifier_emesh_lock);
				cemi->process_response.accel_mode = ECM_CLASSIFIER_ACCELERATION_MODE_NO;
				goto emesh_classifier_out;
			}
		}

		/*
		 * If we didn't see both direction traffic during the acceleration
		 * delay time, we can allow the acceleration by setting the uni-directional
		 * values to both flow and return PCP.
		 */
		cemi->pcp[ECM_CONN_DIR_FLOW] = skb->priority;
		cemi->pcp[ECM_CONN_DIR_RETURN] = skb->priority;
	}

done:
	DEBUG_TRACE("Protocol: %d, Flow Priority: %d, Return priority: %d, sender: %d\n",
			protocol, cemi->pcp[ECM_CONN_DIR_FLOW],
			cemi->pcp[ECM_CONN_DIR_RETURN], sender);

	spin_lock_bh(&ecm_classifier_emesh_lock);

	cemi->process_response.process_actions |= ECM_CLASSIFIER_PROCESS_ACTION_QOS_TAG;
	cemi->process_response.process_actions |= ECM_CLASSIFIER_PROCESS_ACTION_EMESH_SP_FLOW;

	if (((sender == ECM_TRACKER_SENDER_TYPE_SRC) && (IP_CT_DIR_ORIGINAL == CTINFO2DIR(ctinfo))) ||
			((sender == ECM_TRACKER_SENDER_TYPE_DEST) && (IP_CT_DIR_REPLY == CTINFO2DIR(ctinfo)))) {
		cemi->process_response.flow_qos_tag = cemi->pcp[ECM_CONN_DIR_FLOW];
		cemi->process_response.return_qos_tag = cemi->pcp[ECM_CONN_DIR_RETURN];
	} else {
		cemi->process_response.flow_qos_tag = cemi->pcp[ECM_CONN_DIR_RETURN];
		cemi->process_response.return_qos_tag = cemi->pcp[ECM_CONN_DIR_FLOW];
	}

emesh_classifier_out:

	/*
	 * Return our process response
	 */
	*process_response = cemi->process_response;
	spin_unlock_bh(&ecm_classifier_emesh_lock);
}

/*
 * ecm_classifier_emesh_sync_to_v4()
 *	Front end is pushing accel engine state to us
 */
static void ecm_classifier_emesh_sync_to_v4(struct ecm_classifier_instance *aci, struct ecm_classifier_rule_sync *sync)
{
	struct ecm_classifier_emesh_instance *cemi __attribute__((unused));

	cemi = (struct ecm_classifier_emesh_instance *)aci;
	DEBUG_CHECK_MAGIC(cemi, ECM_CLASSIFIER_EMESH_INSTANCE_MAGIC, "%px: magic failed", cemi);
}

/*
 * ecm_classifier_emesh_sync_from_v4()
 *	Front end is retrieving accel engine state from us
 */
static void ecm_classifier_emesh_sync_from_v4(struct ecm_classifier_instance *aci, struct ecm_classifier_rule_create *ecrc)
{
	struct ecm_classifier_emesh_instance *cemi __attribute__((unused));

	cemi = (struct ecm_classifier_emesh_instance *)aci;
	DEBUG_CHECK_MAGIC(cemi, ECM_CLASSIFIER_EMESH_INSTANCE_MAGIC, "%px: magic failed", cemi);
}

/*
 * ecm_classifier_emesh_sync_to_v6()
 *	Front end is pushing accel engine state to us
 */
static void ecm_classifier_emesh_sync_to_v6(struct ecm_classifier_instance *aci, struct ecm_classifier_rule_sync *sync)
{
	struct ecm_classifier_emesh_instance *cemi __attribute__((unused));

	cemi = (struct ecm_classifier_emesh_instance *)aci;
	DEBUG_CHECK_MAGIC(cemi, ECM_CLASSIFIER_EMESH_INSTANCE_MAGIC, "%px: magic failed", cemi);
}

/*
 * ecm_classifier_emesh_sync_from_v6()
 *	Front end is retrieving accel engine state from us
 */
static void ecm_classifier_emesh_sync_from_v6(struct ecm_classifier_instance *aci, struct ecm_classifier_rule_create *ecrc)
{
	struct ecm_classifier_emesh_instance *cemi __attribute__((unused));

	cemi = (struct ecm_classifier_emesh_instance *)aci;
	DEBUG_CHECK_MAGIC(cemi, ECM_CLASSIFIER_EMESH_INSTANCE_MAGIC, "%px: magic failed", cemi);
}

/*
 * ecm_classifier_emesh_type_get()
 *	Get type of classifier this is
 */
static ecm_classifier_type_t ecm_classifier_emesh_type_get(struct ecm_classifier_instance *ci)
{
	struct ecm_classifier_emesh_instance *cemi;
	cemi = (struct ecm_classifier_emesh_instance *)ci;

	DEBUG_CHECK_MAGIC(cemi, ECM_CLASSIFIER_EMESH_INSTANCE_MAGIC, "%px: magic failed\n", cemi);
	return ECM_CLASSIFIER_TYPE_EMESH;
}

/*
 * ecm_classifier_emesh_last_process_response_get()
 *	Get result code returned by the last process call
 */
static void ecm_classifier_emesh_last_process_response_get(struct ecm_classifier_instance *ci,
							struct ecm_classifier_process_response *process_response)
{
	struct ecm_classifier_emesh_instance *cemi;

	cemi = (struct ecm_classifier_emesh_instance *)ci;
	DEBUG_CHECK_MAGIC(cemi, ECM_CLASSIFIER_EMESH_INSTANCE_MAGIC, "%px: magic failed\n", cemi);

	spin_lock_bh(&ecm_classifier_emesh_lock);
	*process_response = cemi->process_response;
	spin_unlock_bh(&ecm_classifier_emesh_lock);
}

/*
 * ecm_classifier_emesh_reclassify_allowed()
 *	Indicate if reclassify is allowed
 */
static bool ecm_classifier_emesh_reclassify_allowed(struct ecm_classifier_instance *ci)
{
	struct ecm_classifier_emesh_instance *cemi;
	cemi = (struct ecm_classifier_emesh_instance *)ci;
	DEBUG_CHECK_MAGIC(cemi, ECM_CLASSIFIER_EMESH_INSTANCE_MAGIC, "%px: magic failed\n", cemi);

	return true;
}

/*
 * ecm_classifier_emesh_reclassify()
 *	Reclassify
 */
static void ecm_classifier_emesh_reclassify(struct ecm_classifier_instance *ci)
{
	struct ecm_classifier_emesh_instance *cemi;
	cemi = (struct ecm_classifier_emesh_instance *)ci;
	DEBUG_CHECK_MAGIC(cemi, ECM_CLASSIFIER_EMESH_INSTANCE_MAGIC, "%px: magic failed\n", cemi);

	/*
	 * Revert back to MAYBE relevant - we will evaluate when we get the next process() call.
	 */
	spin_lock_bh(&ecm_classifier_emesh_lock);
	cemi->process_response.relevance = ECM_CLASSIFIER_RELEVANCE_MAYBE;
	spin_unlock_bh(&ecm_classifier_emesh_lock);
}

#ifdef ECM_STATE_OUTPUT_ENABLE
/*
 * ecm_classifier_emesh_state_get()
 *	Return state
 */
static int ecm_classifier_emesh_state_get(struct ecm_classifier_instance *ci, struct ecm_state_file_instance *sfi)
{
	int result;
	struct ecm_classifier_emesh_instance *cemi;
	struct ecm_classifier_process_response process_response;

	cemi = (struct ecm_classifier_emesh_instance *)ci;
	DEBUG_CHECK_MAGIC(cemi, ECM_CLASSIFIER_EMESH_INSTANCE_MAGIC, "%px: magic failed", cemi);

	if ((result = ecm_state_prefix_add(sfi, "emesh"))) {
		return result;
	}

	spin_lock_bh(&ecm_classifier_emesh_lock);
	process_response = cemi->process_response;
	spin_unlock_bh(&ecm_classifier_emesh_lock);

	/*
	 * Output our last process response
	 */
	if ((result = ecm_classifier_process_response_state_get(sfi, &process_response))) {
		return result;
	}

	return ecm_state_prefix_remove(sfi);
}
#endif

/*
 * ecm_classifier_emesh_instance_alloc()
 *	Allocate an instance of the EMESH classifier
 */
struct ecm_classifier_emesh_instance *ecm_classifier_emesh_instance_alloc(struct ecm_db_connection_instance *ci)
{
	struct ecm_classifier_emesh_instance *cemi;

	/*
	 * Allocate the instance
	 */
	cemi = (struct ecm_classifier_emesh_instance *)kzalloc(sizeof(struct ecm_classifier_emesh_instance), GFP_ATOMIC | __GFP_NOWARN);
	if (!cemi) {
		DEBUG_WARN("Failed to allocate EMESH instance\n");
		return NULL;
	}

	DEBUG_SET_MAGIC(cemi, ECM_CLASSIFIER_EMESH_INSTANCE_MAGIC);
	cemi->refs = 1;
	cemi->base.process = ecm_classifier_emesh_process;
	cemi->base.sync_from_v4 = ecm_classifier_emesh_sync_from_v4;
	cemi->base.sync_to_v4 = ecm_classifier_emesh_sync_to_v4;
	cemi->base.sync_from_v6 = ecm_classifier_emesh_sync_from_v6;
	cemi->base.sync_to_v6 = ecm_classifier_emesh_sync_to_v6;
	cemi->base.type_get = ecm_classifier_emesh_type_get;
	cemi->base.last_process_response_get = ecm_classifier_emesh_last_process_response_get;
	cemi->base.reclassify_allowed = ecm_classifier_emesh_reclassify_allowed;
	cemi->base.reclassify = ecm_classifier_emesh_reclassify;
#ifdef ECM_STATE_OUTPUT_ENABLE
	cemi->base.state_get = ecm_classifier_emesh_state_get;
#endif
	cemi->base.ref = ecm_classifier_emesh_ref;
	cemi->base.deref = ecm_classifier_emesh_deref;
	cemi->ci_serial = ecm_db_connection_serial_get(ci);
	cemi->process_response.process_actions = 0;
	cemi->process_response.relevance = ECM_CLASSIFIER_RELEVANCE_MAYBE;

	spin_lock_bh(&ecm_classifier_emesh_lock);

	/*
	 * Final check if we are pending termination
	 */
	if (ecm_classifier_emesh_terminate_pending) {
		spin_unlock_bh(&ecm_classifier_emesh_lock);
		DEBUG_INFO("%px: Terminating\n", ci);
		kfree(cemi);
		return NULL;
	}

	/*
	 * Link the new instance into our list at the head
	 */
	cemi->next = ecm_classifier_emesh_instances;
	if (ecm_classifier_emesh_instances) {
		ecm_classifier_emesh_instances->prev = cemi;
	}
	ecm_classifier_emesh_instances = cemi;

	/*
	 * Increment stats
	 */
	ecm_classifier_emesh_count++;
	DEBUG_ASSERT(ecm_classifier_emesh_count > 0, "%px: ecm_classifier_emesh_count wrap\n", cemi);
	spin_unlock_bh(&ecm_classifier_emesh_lock);

	DEBUG_INFO("EMESH instance alloc: %px\n", cemi);
	return cemi;
}
EXPORT_SYMBOL(ecm_classifier_emesh_instance_alloc);

/*
 * ecm_classifier_emesh_rule_update_cb()
 *	Callback for service prioritization notification update.
 */
static void ecm_classifier_emesh_rule_update_cb(uint8_t add_rm_md, uint8_t newprec,
		 uint8_t oldprec, bool field_update, struct sp_rule *r)
{
	/*
	 * Return if E-Mesh functionality is not enabled.
	 */
	if (!ecm_classifier_emesh_enabled) {
		return;
	}

	DEBUG_TRACE("SP update notification received\n");

	/*
	 * Destroy all the connections.
	 * The usage of the incoming parameters in this service prioritization
	 * callback will be done in future to perform more refined flush of
	 * connections.
	 */
	ecm_db_connection_defunct_all();
}

/*
 * ecm_classifier_emesh_get_enabled()
 */
static int ecm_classifier_emesh_get_enabled(void *data, u64 *val)
{
	*val = ecm_classifier_emesh_enabled;

	return 0;
}

/*
 * ecm_classifier_emesh_set_enabled()
 */
static int ecm_classifier_emesh_set_enabled(void *data, u64 val)
{
	DEBUG_TRACE("ecm_classifier_emesh_enabled = %u\n", (uint32_t)val);

	if ((val != 0) && (val != 1)) {
		DEBUG_WARN("Invalid value: %u. Valid values are 0 and 1.\n", (uint32_t)val);
		return -EINVAL;
	}

	ecm_classifier_emesh_enabled = (uint32_t)val;

	return 0;
}

/*
 * Debugfs attribute for Emesh Enabled parameter.
 */
DEFINE_SIMPLE_ATTRIBUTE(ecm_classifier_emesh_enabled_fops, ecm_classifier_emesh_get_enabled, ecm_classifier_emesh_set_enabled, "%llu\n");

/*
 * ecm_classifier_emesh_init()
 */
int ecm_classifier_emesh_init(struct dentry *dentry)
{
	int ret;

	DEBUG_INFO("EMESH classifier Module init\n");

	ecm_classifier_emesh_dentry = debugfs_create_dir("ecm_classifier_emesh", dentry);
	if (!ecm_classifier_emesh_dentry) {
		DEBUG_ERROR("Failed to create ecm emesh directory in debugfs\n");
		return -1;
	}

	if (!debugfs_create_file("enabled", S_IRUGO | S_IWUSR,
				 ecm_classifier_emesh_dentry, NULL,
				 &ecm_classifier_emesh_enabled_fops)) {
		DEBUG_ERROR("Failed to create emesh enabled file in debugfs\n");
		debugfs_remove_recursive(ecm_classifier_emesh_dentry);
		return -1;
	}

	/*
	 * Register for service prioritization notification update.
	 */
	ret = sp_mapdb_rule_update_register_notify(ecm_classifier_emesh_rule_update_cb);
	if (ret) {
		DEBUG_ERROR("SP update registration failed: %d\n", ret);
		debugfs_remove_recursive(ecm_classifier_emesh_dentry);
		return -1;
	}

	return 0;
}
EXPORT_SYMBOL(ecm_classifier_emesh_init);

/*
 * ecm_classifier_emesh_exit()
 */
void ecm_classifier_emesh_exit(void)
{
	DEBUG_INFO("Emesh classifier Module exit\n");

	spin_lock_bh(&ecm_classifier_emesh_lock);
	ecm_classifier_emesh_terminate_pending = true;
	spin_unlock_bh(&ecm_classifier_emesh_lock);

	/*
	 * Remove the debugfs files recursively.
	 */
	if (ecm_classifier_emesh_dentry) {
		debugfs_remove_recursive(ecm_classifier_emesh_dentry);
	}

	/*
	 * De-register service prioritization notification update.
	 */
	sp_mapdb_rule_update_unregister_notify();
}
EXPORT_SYMBOL(ecm_classifier_emesh_exit);
