/***************************************************************************
 * Linux PPP over L2TP (PPPoL2TP) Socket Implementation (RFC 2661)
 *
 * This file supplies definitions required by the PPP over L2TP driver
 * (l2tp_ppp.c).  All version information wrt this file is located in l2tp_ppp.c
 *
 * License:
 *		This program is free software; you can redistribute it and/or
 *		modify it under the terms of the GNU General Public License
 *		as published by the Free Software Foundation; either version
 *		2 of the License, or (at your option) any later version.
 *
 */
#ifndef __LINUX_IF_PPPOL2TP_H
#define __LINUX_IF_PPPOL2TP_H

#include <linux/in.h>
#include <linux/in6.h>
#include <uapi/linux/if_pppol2tp.h>

/*
 * Holds L2TP channel info
 */
struct  pppol2tp_common_addr {
	int tunnel_version;				/* v2 or v3 */
	__u32 local_tunnel_id, remote_tunnel_id;	/* tunnel id */
	__u32 local_session_id, remote_session_id;	/* session id */
	struct sockaddr_in local_addr, remote_addr; /* ip address and port */
};

/*
 * L2TP channel operations
 */
struct pppol2tp_channel_ops {
	struct ppp_channel_ops ops; /* ppp channel ops */
};

/*
 * exported function which calls pppol2tp channel's get addressing
 * function
 */
extern int pppol2tp_channel_addressing_get(struct ppp_channel *,
					   struct pppol2tp_common_addr *);
#endif
