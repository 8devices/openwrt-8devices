/*
 *      Handling routines for Mesh in 802.11 Proc
 *
 *      PS: All extern function in ../8190n_headers.h
 */
#define _MESH_PROC_C_

#ifdef CONFIG_RTL8192CD
#include "../8192cd.h"
#include "../8192cd_headers.h"
#else
#include "../8190n.h"
#include "../8190n_headers.h"
#endif
#include "./mesh.h"
#include "./mesh_route.h"
#ifdef MESH_USE_METRICOP
#include "mesh_11kv.h"
#include <linux/init.h>
#include <asm/uaccess.h>
#endif

#if defined(_MESH_DEBUG_)
#include <linux/init.h>
#include <asm/uaccess.h>
#include <linux/module.h>
#endif

