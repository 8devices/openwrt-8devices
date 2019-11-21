/*
 *      Handling routines for Mesh in 802.11 SME (Station Management Entity)
 *
 *      PS: All extern function in ../8190n_headers.h
 */
#define _MESH_SME_C_

#ifdef CONFIG_RTL8192CD
#include "../8192cd.h"
#include "../8192cd_headers.h"
#include "../8192cd_util.h"
#include "../ieee802_mib.h"
#else
#include "../rtl8190/8190n.h"
#include "../rtl8190/8190n_headers.h"
#include "../rtl8190/8190n_util.h"
#include "../rtl8190/ieee802_mib.h"
#endif
#include "./mesh.h"
#include "./mesh_route.h"
#ifdef MESH_USE_METRICOP
#include "mesh_11kv.h"
#endif

