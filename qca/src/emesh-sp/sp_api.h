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

#ifndef _SP_API_H
#define _SP_API_H

#include <linux/types.h>

/*
 * sp_rule_match_flag
 * 	Rule match mask
 */
#define	SP_RULE_FLAG_MATCH_ALWAYS_TRUE		0x01		/* Rule match always true. s(skip field matching) mask */
#define	SP_RULE_FLAG_MATCH_UP			0x02		/* Match up in 802.11 qos control mask */
#define	SP_RULE_FLAG_MATCH_UP_SENSE		0x04 		/* UP in 802.11 qos control match sense mask */
#define	SP_RULE_FLAG_MATCH_SOURCE_MAC		0x08		/* Match source mac address mask */
#define	SP_RULE_FLAG_MATCH_SOURCE_MAC_SENSE	0x10		/* Match source mac address mask */
#define	SP_RULE_FLAG_MATCH_DST_MAC		0x20		/* Match destination mac address mask */
#define	SP_RULE_FLAG_MATCH_DST_MAC_SENSE	0x40		/* Destination mac address match sense mask */

/*
 * sp_mapdb_update_results
 * 	Result values of rule update.
 */
enum sp_mapdb_update_results {
	SP_MAPDB_UPDATE_RESULT_ERR_TBLFULL, 		/* The rule table is full. */
	SP_MAPDB_UPDATE_RESULT_ERR_TBLEMPTY, 		/* The rule table is empty. */
	SP_MAPDB_UPDATE_RESULT_ERR_ALLOCNODE, 		/* Failed at allocating memory for a rule node. (struct sp_mapdb_rule_node) */
	SP_MAPDB_UPDATE_RESULT_ERR_ALLOCHASH, 		/* Failed at allocating memory for a hashentry. */
	SP_MAPDB_UPDATE_RESULT_ERR_RULENOEXIST, 	/* There is no such rule with the given rule id. */
	SP_MAPDB_UPDATE_RESULT_ERR_UNKNOWNBIT, 		/* Unknown add/remove filter bit. */
	SP_MAPDB_UPDATE_RESULT_ERR_SINGLE_WRITER,	/* Single writer protection violation. */
	SP_MAPDB_UPDATE_RESULT_ERR_INVALIDENTRY,	/* Invalid entry of rule field. */
	SP_MAPDB_UPDATE_RESULT_ERR_NEWRULE_NULLPTR,	/* New rule is a null pointer. */
	SP_MAPDB_UPDATE_RESULT_ERR, 			/* Delimiter */
	SP_MAPDB_UPDATE_RESULT_SUCCESS_ADD, 		/* Successful rule add */
	SP_MAPDB_UPDATE_RESULT_SUCCESS_DELETE, 		/* Successful rule deletion */
	SP_MAPDB_UPDATE_RESULT_SUCCESS_MODIFY, 		/* Successful rule modification */
	SP_MAPDB_UPDATE_RESULT_LAST,
};
typedef enum sp_mapdb_update_results sp_mapdb_update_result_t;

/*
 * sp_mapdb_notify_types
 * 	Types of notification.
 */
enum sp_mapdb_notify_types {
	SP_MAPDB_ADD_RULE,				/* Notify rule has been added. */
	SP_MAPDB_REMOVE_RULE,				/* Notify rule has been removed. */
	SP_MAPDB_MODIFY_RULE,				/* Notify rule has been modified. */
};
typedef enum sp_mapdb_notify_types sp_mapdb_notify_type_t;

/*
 * sp_mapdb_add_remove_filter_types
 * 	Possible value of Add-remove filter bit.
 */
enum sp_mapdb_add_remove_filter_types {
	SP_MAPDB_ADD_REMOVE_FILTER_DELETE, 		/* Delete a rule. */
	SP_MAPDB_ADD_REMOVE_FILTER_ADD, 		/* Add a rule. */
};
typedef enum sp_mapdb_add_remove_filter_types sp_mapdb_add_remove_filter_type_t;

struct sp_rule_inner {

	/*
	 * The value of rule_output determines how to set the pcp value
	 * to be marked in the matched packet.
	 */
	uint8_t rule_output;
	uint8_t flags;				/* Flag bits for rule match */
	uint8_t user_priority;			/* UP in 802.11 qos control */

	/*
	 * Source mac address
	 * If “match source mac address” flag bit is set,
	 * this field shall be included, otherwise this field shall be omitted.
	 */
	uint8_t sa[ETH_ALEN];

	/*
	 * Destination mac address
	 * If “match destination mac address” flag
	 * bit is set, this field shall be included,
	 * otherwise this field shall be omitted.
	*/
	uint8_t da[ETH_ALEN];
};

/*
 * sp_rule
 *	This struct stores Service Prioritization rule structure.
 */
struct sp_rule {
	u_int32_t id;						/* Service prioritization rule identifier */
	sp_mapdb_add_remove_filter_type_t cmd;			/* Command type. 1 means add 0 means delete. */
	struct sp_rule_inner inner;				/* Inner structure */
	uint8_t rule_precedence;				/* Rule precedence – higher number means higher priority. */
};

sp_mapdb_update_result_t sp_mapdb_rule_update(struct sp_rule*);

int sp_mapdb_rule_update_register_notify(void (*sp_mapdb_rule_update_callback)(uint8_t add_rm_md, uint8_t newprec, uint8_t oldprec, bool field_update, struct sp_rule *r));
void sp_mapdb_rule_update_unregister_notify(void);
#endif
