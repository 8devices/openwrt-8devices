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

#include "nsm_sfe.h"

/*
 * Precision of throughput calculation. Throughputs are
 * reported in terms of packets per NSM_SFE_PREC seconds.
 * A value of 100 means throughput is packets per 100 seconds.
 */
#define NSM_SFE_PREC 100
#define NSM_US_PER_SEC 1000000

static struct nsm_sfe_stats nsm_sfe_prev_stats[SFE_MAX_SERVICE_CLASS_ID];

/*
 * nsm_sfe_rate_calc()
 *	Calculate the rate based on the change in value divided by the change in time.
 */
static int64_t nsm_sfe_rate_calc(uint64_t dval, int64_t dtime)
{
	uint64_t scaled_dval = dval * NSM_SFE_PREC * NSM_US_PER_SEC;

	if (dtime == 0) {
		return -1;
	}

	/*
	 * The best way to calculate the rate accurately is to scale
	 * the numerator up, but if this results in an overflow, we
	 * can instead scale the denominator down.
	 */
	if (scaled_dval < dval) {
		uint64_t ret;
		uint64_t scaled_dtime = dtime;
		do_div(scaled_dtime, (NSM_SFE_PREC * NSM_US_PER_SEC));
		if (scaled_dtime == 0) {
			return -1;
		}
		ret = dval;
		do_div(ret, scaled_dtime);
		return ret;
	}

	do_div(scaled_dval, dtime);
	return scaled_dval;
}

/*
 * nsm_sfe_get_stats()
 *	Updates SFE stats for given service class ID.
 */
struct nsm_sfe_stats *nsm_sfe_get_stats(uint8_t sid)
{
	struct nsm_sfe_stats *stats = &nsm_sfe_prev_stats[sid];

	if (!sfe_service_class_stats_get(sid, &stats->bytes, &stats->packets)) {
		return NULL;
	}

	/*
	 * We get the time after collecting statistics since a seqlock can take many attempts.
	 */
	stats->time = ktime_get();

	return stats;
}

/*
 * nsm_sfe_get_throughput()
 *	Calculate throughput for the given service class ID.
 */
int nsm_sfe_get_throughput(uint8_t sid, uint64_t *packet_rate, uint64_t *byte_rate)
{
	struct nsm_sfe_stats prev_stats;
	struct nsm_sfe_stats *stats = &(nsm_sfe_prev_stats[sid]);
	uint64_t dbytes, dpackets;
	int64_t dtime, rate;

	prev_stats.bytes = stats->bytes;
	prev_stats.packets = stats->packets;
	prev_stats.time = stats->time;

	stats = nsm_sfe_get_stats(sid);

	if (!stats) {
		return -1;
	}

	dbytes = stats->bytes - prev_stats.bytes;
	dpackets = stats->packets - prev_stats.packets;
	dtime = ktime_us_delta(stats->time, prev_stats.time);

	rate = nsm_sfe_rate_calc(dbytes, dtime);
	if (rate < 0) {
		return -1;
	}

	*byte_rate = rate;

	rate = nsm_sfe_rate_calc(dpackets, dtime);
	if (rate < 0) {
		return -1;
	}

	*packet_rate = rate;

	return 0;
}
