/*
 *	 Headler file defines some data structure and macro of bridge extention
 *
 *	 $Id: 8192cd_br_ext.h,v 1.1.4.1 2010/07/28 13:15:27 davidhsu Exp $
 *
 *  Copyright (c) 2009 Realtek Semiconductor Corp.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 */

#ifndef _8192CD_BR_EXT_H_
#define _8192CD_BR_EXT_H_

#define NAT25_HASH_BITS		4
#define NAT25_HASH_SIZE		(1 << NAT25_HASH_BITS)
#define NAT25_AGEING_TIME	300

#ifdef CL_IPV6_PASS
#define MAX_NETWORK_ADDR_LEN	17
#else
#define MAX_NETWORK_ADDR_LEN	11
#endif

struct nat25_network_db_entry
{
	struct nat25_network_db_entry	*next_hash;
	struct nat25_network_db_entry	**pprev_hash;
	atomic_t						use_count;
	unsigned char					macAddr[6];
	unsigned long					ageing_timer;
#ifdef __ECOS
	unsigned	int					used;
#endif
	unsigned char    				networkAddr[MAX_NETWORK_ADDR_LEN];
};

enum NAT25_METHOD {
	NAT25_MIN,
	NAT25_CHECK,
	NAT25_INSERT,
	NAT25_LOOKUP,
	NAT25_PARSE,
	NAT25_MAX
};

#endif // _8192CD_BR_EXT_H_

