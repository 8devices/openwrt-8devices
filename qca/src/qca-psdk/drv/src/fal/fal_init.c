/*
 * Copyright (c) 2012, 2016-2018, The Linux Foundation. All rights reserved.
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


/**
 * @defgroup fal_init FAL_INIT
 * @{
 */
#include "sw.h"
#include "hsl.h"
#include "hsl_dev.h"
#include "hsl_api.h"
/**
 * @brief Init fal layer.
 * @details Comments:
 *   This operation will init fal layer and hsl layer
 * @param[in] dev_id device id
 * @param[in] cfg configuration for initialization
 * @return SW_OK or error code
 */
sw_error_t
fal_init(a_uint32_t dev_id, ssdk_init_cfg *cfg)
{
    sw_error_t rv;
    HSL_DEV_ID_CHECK(dev_id);

    rv = hsl_api_init(dev_id);
    SW_RTN_ON_ERROR(rv);

    rv = hsl_dev_init(dev_id, cfg);
    SW_RTN_ON_ERROR(rv);

    return rv;
}
sw_error_t
fal_cleanup(void)
{
    sw_error_t rv;

    rv = hsl_dev_cleanup();
    SW_RTN_ON_ERROR(rv);
    return SW_OK;
}
