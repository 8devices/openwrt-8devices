/*
 *  Software TKIP encryption/descryption routines
 *
 *  $Id: 8192cd_tkip.c,v 1.4.4.2 2010/09/30 05:27:28 button Exp $
 *
 *  Copyright (c) 2009 Realtek Semiconductor Corp.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 */

#define _8192CD_11K_BEACON_C_

#include <linux/module.h>
#include <asm/byteorder.h>

#include "../8192cd_cfg.h"


#include "../8192cd.h"
#include "../ieee802_mib.h"
#include "../8192cd_util.h"
#include "../8192cd_headers.h"
#include "../8192cd_debug.h"

