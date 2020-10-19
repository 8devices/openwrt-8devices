/*
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
 */

#include <linux/kernel.h>
#include <linux/netfilter.h>
#include <linux/etherdevice.h>

#include "sp_mapdb.h"
#include "sp_types.h"

/* Spinlock for SMP updating the rule table. */
DEFINE_SPINLOCK(sp_mapdb_lock);

/* SP Rule manager */
static struct sp_mapdb_rule_manager rule_manager;

/* Rule update callback */
static __rcu sp_mapdb_rule_update_callback_t sp_callback;

/* TSL protection of single writer rule update. */
static unsigned long single_writer = 0;

/*
 * sp_mapdb_rules_init()
 * 	Initializes prec_map and ruleid_hashmap.
 */
static inline void sp_mapdb_rules_init(void)
{
	int i;

	spin_lock(&sp_mapdb_lock);
	for (i = 0; i < SP_MAPDB_RULE_MAX_PRECEDENCENUM; i++) {
		INIT_LIST_HEAD(&rule_manager.prec_map[i].rule_list);
	}
	spin_unlock(&sp_mapdb_lock);

	hash_init(rule_manager.rule_id_hashmap);
	rule_manager.rule_count = 0;

	DEBUG_TRACE("%px: Finish Initializing SP ruledb\n", &rule_manager);
}

/*
 * sp_mapdb_notify()
 * 	Notifies the registered module about the rule table changes.
 */
static void sp_mapdb_notify(uint8_t add_remove_modify, uint8_t new_prec, uint8_t old_prec, bool field_update, struct sp_rule *newrule)
{
	sp_mapdb_rule_update_callback_t cb;

	rcu_read_lock();
	cb = rcu_dereference(sp_callback);
	if (cb) {
		cb(add_remove_modify, new_prec, old_prec, field_update, newrule);
	}
	rcu_read_unlock();
}

/*
 * sp_rule_destroy_rcu()
 * 	Destroys a rule node.
 */
static void sp_rule_destroy_rcu(struct rcu_head *head)
{
	struct sp_mapdb_rule_node *node_p =
		container_of(head, struct sp_mapdb_rule_node, rcu);

	DEBUG_INFO("%px: Removed rule id = %d\n", head, node_p->rule.id);
	kfree(node_p);
}

/*
 * sp_mapdb_search_hashentry()
 * 	Find the hashentry that stores the rule_node by the ruleid.
 */
static struct sp_mapdb_rule_id_hashentry *sp_mapdb_search_hashentry(uint32_t ruleid)
{
	struct sp_mapdb_rule_id_hashentry *hashentry_iter;

	hash_for_each_possible(rule_manager.rule_id_hashmap, hashentry_iter, hlist, ruleid) {
		if (hashentry_iter->rule_node->rule.id == ruleid) {
			return hashentry_iter;
		}
	}

	return NULL;
}

/*
 * sp_mapdb_rule_add()
 * 	Adds (or modifies) the SP rule in the rule table.
 *
 * It will also updating the add_remove_modify and old_prec and field_update argument.
 * old_prec : the precedence of the previous rule.
 * field_update : if fields other than precedence is different from the previous rule.
 */
static sp_mapdb_update_result_t sp_mapdb_rule_add(struct sp_rule *newrule)
{
	uint8_t newrule_precedence = newrule->rule_precedence;
	uint8_t old_prec;
	bool field_update;
	struct sp_mapdb_rule_node *cur_rule_node = NULL;
	struct sp_mapdb_rule_id_hashentry *cur_hashentry = NULL;
	struct sp_mapdb_rule_node *new_rule_node;
	struct sp_mapdb_rule_id_hashentry *new_hashentry;

	DEBUG_INFO("%px: Try adding rule id = %d\n", newrule, newrule->id);

	if (rule_manager.rule_count == SP_MAPDB_RULE_MAX) {
		DEBUG_WARN("%px:Ruletable is full. Error adding rule %d\n", newrule, newrule->id);
		return SP_MAPDB_UPDATE_RESULT_ERR_TBLFULL;
	}

	if (newrule->inner.rule_output >= SP_MAPDB_NO_MATCH) {
		DEBUG_WARN("%px:Invalid rule output value %d (valid range:0-9)\n", newrule, newrule->inner.rule_output);
		return SP_MAPDB_UPDATE_RESULT_ERR_INVALIDENTRY;
	}

	new_rule_node = (struct sp_mapdb_rule_node *)kzalloc(sizeof(struct sp_mapdb_rule_node), GFP_KERNEL);
	if (!new_rule_node) {
		DEBUG_ERROR("%px:Error, allocate rule node failed.\n", newrule);
		return SP_MAPDB_UPDATE_RESULT_ERR_ALLOCNODE;
	}

	memcpy(&new_rule_node->rule, newrule, sizeof(struct sp_rule));

	spin_lock(&sp_mapdb_lock);
	cur_hashentry = sp_mapdb_search_hashentry(newrule->id);
	if (!cur_hashentry) {
		spin_unlock(&sp_mapdb_lock);
		new_hashentry = (struct sp_mapdb_rule_id_hashentry *)kzalloc(sizeof(struct sp_mapdb_rule_id_hashentry), GFP_KERNEL);
		if (!new_hashentry) {
			DEBUG_ERROR("%px:Error, allocate hashentry failed.\n", newrule);
			kfree(new_rule_node);
			return SP_MAPDB_UPDATE_RESULT_ERR_ALLOCHASH;
		}

		/*
		 * Inserting new rule node and hash entry in prec_map
		 * and hashmap respectively.
		 */
		spin_lock(&sp_mapdb_lock);
		new_hashentry->rule_node = new_rule_node;
		list_add_rcu(&new_rule_node->rule_list, &rule_manager.prec_map[newrule_precedence].rule_list);
		hash_add(rule_manager.rule_id_hashmap, &new_hashentry->hlist,newrule->id);
		rule_manager.rule_count++;
		spin_unlock(&sp_mapdb_lock);

		DEBUG_INFO("%px:Success rule id=%d\n", newrule, newrule->id);

		/*
		 * Since this is inserting a new rule, the old precendence
		 * and field update do not possess any meaning.
		 */
		sp_mapdb_notify(SP_MAPDB_ADD_RULE, newrule_precedence, 0, false, newrule);

		return SP_MAPDB_UPDATE_RESULT_SUCCESS_ADD;
	}

	cur_rule_node = cur_hashentry->rule_node;

	if (cur_rule_node->rule.rule_precedence == newrule_precedence) {
		list_replace_rcu(&cur_rule_node->rule_list, &new_rule_node->rule_list);
		cur_hashentry->rule_node = new_rule_node;
		spin_unlock(&sp_mapdb_lock);

		DEBUG_INFO("%px:overwrite rule id =%d success.\n", newrule, newrule->id);

		/*
		 * If precedence doesn't change then it has to be some fields modified.
		 */
		sp_mapdb_notify(SP_MAPDB_MODIFY_RULE, newrule_precedence, newrule_precedence, true, newrule);

		call_rcu(&cur_rule_node->rcu, sp_rule_destroy_rcu);

		return SP_MAPDB_UPDATE_RESULT_SUCCESS_MODIFY;
	}

	list_del_rcu(&cur_rule_node->rule_list);
	list_add_rcu(&new_rule_node->rule_list, &rule_manager.prec_map[newrule_precedence].rule_list);
	cur_hashentry->rule_node = new_rule_node;
	spin_unlock(&sp_mapdb_lock);

	/*
	 * Fields other than rule_precedence can still be updated along with rule_precedence.
	 */
	old_prec = cur_rule_node->rule.rule_precedence;
	field_update = memcmp(&cur_rule_node->rule.inner, &newrule->inner, sizeof(struct sp_rule_inner)) ? true : false;
	DEBUG_INFO("%px:Success rule id=%d\n", newrule, newrule->id);
	sp_mapdb_notify(SP_MAPDB_MODIFY_RULE, newrule_precedence, old_prec, field_update, newrule);
	call_rcu(&cur_rule_node->rcu, sp_rule_destroy_rcu);

	return SP_MAPDB_UPDATE_RESULT_SUCCESS_MODIFY;
}

/*
 * sp_mapdb_rule_delete()
 * 	Deletes a rule from the rule table by the rule id.
 *
 * The memory for the rule node will also be deleted as hash entry will also be freed.
 */
static sp_mapdb_update_result_t sp_mapdb_rule_delete(uint32_t ruleid)
{
	struct sp_mapdb_rule_node *tobedeleted;
	struct sp_mapdb_rule_id_hashentry *cur_hashentry = NULL;

	spin_lock(&sp_mapdb_lock);
	if (rule_manager.rule_count == 0) {
		spin_unlock(&sp_mapdb_lock);
		DEBUG_WARN("rule table is empty\n");
		return SP_MAPDB_UPDATE_RESULT_ERR_TBLEMPTY;
	}

	cur_hashentry = sp_mapdb_search_hashentry(ruleid);
	if (!cur_hashentry) {
		spin_unlock(&sp_mapdb_lock);
		DEBUG_WARN("there is no such rule as ruleID = %d\n", ruleid);
		return SP_MAPDB_UPDATE_RESULT_ERR_RULENOEXIST;
	}

	tobedeleted = cur_hashentry->rule_node;
	list_del_rcu(&tobedeleted->rule_list);
	hash_del(&cur_hashentry->hlist);
	kfree(cur_hashentry);
	rule_manager.rule_count--;
	spin_unlock(&sp_mapdb_lock);

	DEBUG_INFO("Successful deletion\n");

	/*
	 * There is no point on having old_prec
	 * and field_update in remove rules case.
	 */
	sp_mapdb_notify(SP_MAPDB_REMOVE_RULE, tobedeleted->rule.rule_precedence, 0, false, &tobedeleted->rule);
	call_rcu(&tobedeleted->rcu, sp_rule_destroy_rcu);

	return SP_MAPDB_UPDATE_RESULT_SUCCESS_DELETE;
}

/*
 * sp_mapdb_rule_match()
 * 	Performs rule match on received skb.
 *
 * It is called per packet basis and fields are checked and compared with the SP rule (rule).
 */
static bool sp_mapdb_rule_match(struct sk_buff *skb, struct sp_rule *rule)
{
	struct ethhdr *eth_header;
	bool compare_result, sense;
	uint8_t flags = rule->inner.flags;

	if (flags & SP_RULE_FLAG_MATCH_ALWAYS_TRUE) {
		DEBUG_INFO("Basic match case.\n");
		return true;
	}

	eth_header = eth_hdr(skb);

	if (flags & SP_RULE_FLAG_MATCH_UP) {
		DEBUG_INFO("Matching UP..\n");
		DEBUG_INFO("skb->up = %d , rule->up = %d\n", skb->priority, rule->inner.user_priority);

		compare_result = (skb->priority == rule->inner.user_priority) ? true:false;
		sense = !!(flags & SP_RULE_FLAG_MATCH_UP_SENSE);
		if (!(compare_result ^ sense)) {
			DEBUG_WARN("Match UP failed\n");
			return false;
		}
	}

	if (flags & SP_RULE_FLAG_MATCH_SOURCE_MAC) {
		DEBUG_INFO("Matching SRC..\n");
		DEBUG_INFO("skb src = %pM\n", eth_header->h_source);
		DEBUG_INFO("rule src = %pM\n", rule->inner.sa);

		compare_result = ether_addr_equal(eth_header->h_source, rule->inner.sa);
		sense = !!(flags & SP_RULE_FLAG_MATCH_SOURCE_MAC_SENSE);
		if (!(compare_result ^ sense)) {
			DEBUG_WARN("SRC match failed!\n");
			return false;
		}
	}
	if (flags & SP_RULE_FLAG_MATCH_DST_MAC) {
		DEBUG_INFO("Matching DST..\n");
		DEBUG_INFO("skb dst = %pM\n", eth_header->h_dest);
		DEBUG_INFO("rule dst = %pM\n", rule->inner.da);

		compare_result = ether_addr_equal(eth_header->h_dest, rule->inner.da);
		sense = !!(flags & SP_RULE_FLAG_MATCH_DST_MAC_SENSE);
		if (!(compare_result ^ sense)) {
			DEBUG_WARN("DST match failed!\n");
			return false;
		}
	}

	return true;
}

/*
 * sp_mapdb_ruletable_search()
 * 	Performs rules match for a skb over an entire rule table.
 *
 * According to the specification, the rules will be enumerated in a precedence descending order.
 * Once there is a match, the enumeration stops.
 * In details, it enumerates from prec_map[0xFE] to prec_map[0] for each packet, and exits loop if there is a
 * rule match. The output value defined in a matched rule
 * will be used to determine which fields(UP,DSCP) will be used for
 * PCP value.
 */
static uint8_t sp_mapdb_ruletable_search(struct sk_buff *skb)
{
	uint8_t output = SP_MAPDB_NO_MATCH;
	struct sp_mapdb_rule_node *curnode;
	int i, protocol;

	/*
	 * The iteration loop goes backward because
	 * rules should be matched in the precedence
	 * descending order.
	 */
	for (i = SP_MAPDB_RULE_MAX_PRECEDENCENUM - 1; i >= 0; i--) {
		list_for_each_entry_rcu(curnode, &(rule_manager.prec_map[i].rule_list), rule_list) {
			DEBUG_INFO("Matching with rid = %d\n", curnode->rule.id);
			if (sp_mapdb_rule_match(skb, &curnode->rule)) {
				output = curnode->rule.inner.rule_output;
				goto set_output;
			}
		}
	}

set_output:
	switch (output) {
	case SP_MAPDB_USE_UP:
		output = skb->priority;
		break;

	case SP_MAPDB_USE_DSCP:

		/*
		 * >> 2 first(dscp field) and then >>3 (DSCP->PCP mapping)
		 */
		protocol = ntohs(skb->protocol);

		switch (protocol) {
		case ETH_P_IP:
			output = (ipv4_get_dsfield(ip_hdr(skb))) >> 5;
			break;

		case ETH_P_IPV6:
			output = (ipv6_get_dsfield(ipv6_hdr(skb))) >> 5;
			break;

		default:

			/*
			 * non-IP protocol does not have dscp field, apply DEFAULT_PCP.
			 */
			output = SP_MAPDB_RULE_DEFAULT_PCP;
			break;
		}
		break;

	case SP_MAPDB_NO_MATCH:
		output = SP_MAPDB_RULE_DEFAULT_PCP;
		break;

	default:
		break;
	}

	return output;
}

/*
 * sp_mapdb_ruletable_flush()
 * 	Clear the rule table and frees the memory allocated for the rules.
 *
 * It will enumerate all the precedence in the prec_map,
 * and start from the head node in each of the precedence in the prec_map,
 * and free all the rule nodes
 * as well as the associated hashentry, with
 * these precedence.
 */
void sp_mapdb_ruletable_flush(void)
{
	int i;

	struct sp_mapdb_rule_node *node_list, *node, *tmp;
	struct sp_mapdb_rule_id_hashentry *hashentry_iter;
	struct hlist_node *hlist_tmp;
	int hash_bkt;

	spin_lock(&sp_mapdb_lock);
	for (i = 0; i < SP_MAPDB_RULE_MAX_PRECEDENCENUM; i++) {
		node_list = &rule_manager.prec_map[i];
		/*
		 * tmp as a temporary pointer to store the address of next node.
		 * This is required because we are using list_for_each_entry_safe,
		 * which allows in-loop deletion of the node.
		 *
		 * kfree is sufficient since free_list is called in sp_mapdb_ruletable_flush,
		 * which happens before sp_hook_init. So no datapath is reading at this point.
		 */
		list_for_each_entry_safe(node, tmp, &node_list->rule_list, rule_list) {
			kfree(node);
		}
	}

	/* Free hash list. */
	hash_for_each_safe(rule_manager.rule_id_hashmap, hash_bkt, hlist_tmp, hashentry_iter, hlist) {
		hash_del(&hashentry_iter->hlist);
		kfree(hashentry_iter);
	}
	rule_manager.rule_count = 0;
	spin_unlock(&sp_mapdb_lock);
}

/*
 * sp_mapdb_rule_update()
 * 	Perfoms rule update.
 *
 * It will first check the add/remove filter bit of
 * the newrule and pass it to sp_mapdb_rule_add and sp_mapdb_rule_delete acccordingly.
 * sp_mapdb_rule_update will also collect add_remove_modify based on the updated result, as:
 * add = 0, remove = 1, modify = 2
 * and field_update (meaning whether the field(other than precence)
 * is modified), these are useful in perform precise matching in ECM.
 */
sp_mapdb_update_result_t sp_mapdb_rule_update(struct sp_rule *newrule)
{
	sp_mapdb_update_result_t error_code = 0;

	if (!newrule) {
		return SP_MAPDB_UPDATE_RESULT_ERR_NEWRULE_NULLPTR;
	}

	if (test_and_set_bit_lock(0, &single_writer)) {
		DEBUG_ERROR("%px: single writer allowed", newrule);
		return SP_MAPDB_UPDATE_RESULT_ERR_SINGLE_WRITER;
	}

	switch (newrule->cmd) {
	case SP_MAPDB_ADD_REMOVE_FILTER_DELETE:
		error_code = sp_mapdb_rule_delete(newrule->id);
		break;

	case SP_MAPDB_ADD_REMOVE_FILTER_ADD:
		error_code = sp_mapdb_rule_add(newrule);
		break;

	default:
		DEBUG_ERROR("%px: Error, unknown Add/Remove filter bit\n", newrule);
		error_code = SP_MAPDB_UPDATE_RESULT_ERR_UNKNOWNBIT;
		break;
	}

	clear_bit_unlock(0, &single_writer);

	return error_code;
}
EXPORT_SYMBOL(sp_mapdb_rule_update);

/*
 * sp_mapdb_rule_update_register_notify()
 * 	Notification registration function.
 *
 * The callback function is to get notification upon rule update.
 * sp_mapdb_rule_update_callback_t - callback function pointer.
 * See definition in header file about its argument's meaning.
 */
int sp_mapdb_rule_update_register_notify(sp_mapdb_rule_update_callback_t cb)
{
	if (rcu_access_pointer(sp_callback)) {
		DEBUG_WARN("Fail to register callback(cb_block busy)\n");

		return -1;
	}

	rcu_assign_pointer(sp_callback, cb);
	synchronize_rcu();
	DEBUG_INFO("Callback successfully registered.");

	return 0;
}
EXPORT_SYMBOL(sp_mapdb_rule_update_register_notify);

/*
 * sp_mapdb_rule_update_unregister_notify()
 * 	Notification unregistration function.
 */
void sp_mapdb_rule_update_unregister_notify(void)
{
	rcu_assign_pointer(sp_callback, NULL);

	synchronize_rcu();

	DEBUG_INFO("Successfully invoked unregister callback.");
}
EXPORT_SYMBOL(sp_mapdb_rule_update_unregister_notify);

/*
 * sp_mapdb_ruletable_print()
 *	Print the rule table.
 */
void sp_mapdb_ruletable_print(void)
{
	int i;
	struct sp_mapdb_rule_node *curnode = NULL;

	rcu_read_lock();
	printk("\n====Rule table start====\nTotal rule count = %d\n", rule_manager.rule_count);
	for (i = SP_MAPDB_RULE_MAX_PRECEDENCENUM - 1; i >= 0; i--) {
		if (!list_empty(&(rule_manager.prec_map[i].rule_list))) {
			printk("\nPrecedence=%d:\n", i);
		}

		list_for_each_entry_rcu(curnode, &(rule_manager.prec_map[i].rule_list), rule_list) {
			printk("[id=%d, precedence=%d, output=%d] v\n", curnode->rule.id, curnode->rule.rule_precedence, curnode->rule.inner.rule_output);
		}
	}
	rcu_read_unlock();
	printk("====Rule table ends====\n");
}

/*
 * sp_mapdb_apply()
 * 	Assign the desired PCP value into skb->priority.
 */
void sp_mapdb_apply(struct sk_buff *skb)
{
	rcu_read_lock();
	skb->priority = sp_mapdb_ruletable_search(skb);
	rcu_read_unlock();
}

/*
 * sp_mapdb_init()
 * 	Initialize ruledb.
 */
void sp_mapdb_init(void)
{
	sp_mapdb_rules_init();
}

/*
 * sp_mapdb_fini()
 * 	This is the function called when SPM is unloaded.
 */
void sp_mapdb_fini(void)
{
	sp_mapdb_ruletable_flush();
}
