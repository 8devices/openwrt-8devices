/*
 * Copyright (c) 2014, 2016-2018, The Linux Foundation. All rights reserved.
 * Permission to use, copy, modify, and/or distribute this software for
 * any purpose with or without fee is hereby granted, provided that the
 * above copyright notice and this permission notice appear in all copies.
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT
 * OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */


#include <stdio.h>
#include "shell.h"
#include "fal.h"

static int sw_devid = 0;

sw_error_t
cmd_set_devid(a_ulong_t *arg_val)
{
    sw_error_t rtn;
    ssdk_cfg_t ssdk_cfg_new;

    if (arg_val[1] >= SW_MAX_NR_DEV)
    {
        dprintf("dev_id should be less than <%d>\n", SW_MAX_NR_DEV);
        return SW_FAIL;
    }
    sw_devid = arg_val[1];

    rtn = fal_ssdk_cfg(sw_devid, &ssdk_cfg_new);
    if (rtn == SW_OK)
    {
	    ssdk_cfg = ssdk_cfg_new;
    }

    return rtn;
}

int
get_devid(void)
{
    return sw_devid;
}

int
set_devid(int dev_id)
{
	sw_devid = dev_id;
	return SW_OK;
}
