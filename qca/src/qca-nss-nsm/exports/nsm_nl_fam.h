/*
 **************************************************************************
 * Copyright (c) 2022, Qualcomm Innovation Cetner, Inc. All rights reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for
 * any purpose with or without fee is hereby granted, provided that the
 * above copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT
 * OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 **************************************************************************
 */

#ifndef __NSM_NL_FAM_H
#define __NSM_NL_FAM_H

#define NSM_NL_NAME "NSM_NL_FAM"

/*
 * nsm_nl_attr
 *	Attributes for the nsm_nl generic netlink family.
 */
enum nsm_nl_attr {
	NSM_NL_ATTR_UNSPEC,
	NSM_NL_ATTR_RX_PACKETS,
	NSM_NL_ATTR_RX_BYTES,
	NSM_NL_ATTR_SERVICE_ID,
	NSM_NL_ATTR_PAD,
	NSM_NL_ATTR_MAX
};

/*
 * nsm_nl_cmd
 *	Commands for the nsm_nl generic netlink family.
 */
enum nsm_nl_cmd {
	NSM_NL_CMD_UNUSED,
	NSM_NL_CMD_GET_STATS,
	NSM_NL_CMD_GET_THROUGHPUT,
	NSM_NL_CMD_MAX
};

#endif
