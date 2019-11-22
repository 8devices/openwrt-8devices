/*
 *      Handling routines for Mesh in 802.11 TX
 *
 *      PS: All extern function in ../8190n_headers.h
 */
#define _MESH_TX_C_

#ifdef CONFIG_RTL8192CD
#include "../8192cd_cfg.h"
#include "../8192cd.h"
#include "../8192cd_headers.h"
#else
#include "../rtl8190/8190n_cfg.h"
#include "../rtl8190/8190n.h"
#include "../rtl8190/8190n_headers.h"
#endif
#include "./mesh.h"
#include "./mesh_route.h"

