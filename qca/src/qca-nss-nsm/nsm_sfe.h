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

#ifndef __NSM_SFE_H
#define __NSM_SFE_H

#include <linux/ktime.h>
#include <linux/if_ether.h>
#include <sfe_api.h>

struct nsm_sfe_stats {
	uint64_t packets;
	uint64_t bytes;
	ktime_t time;
};

struct nsm_sfe_stats *nsm_sfe_get_stats(uint8_t sid);
int nsm_sfe_get_throughput(uint8_t sid, uint64_t *packet_rate, uint64_t *byte_rate);

#endif
