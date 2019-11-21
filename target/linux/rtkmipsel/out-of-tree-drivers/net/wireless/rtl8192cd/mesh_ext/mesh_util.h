
#ifndef _MESH_UTIL_H_
#define _MESH_UTIL_H_

#include <linux/list.h>
#include "./mesh.h"
#ifdef CONFIG_RTL8192CD
#include "../8192cd_util.h"
#else
#include "../rtl8190/8190n_util.h"
#endif

#define is_mesh_6addr_format_without_qos(pframe)	((*((unsigned char*)(pframe)+30) & 0x01))	///< AE field(mesh header) bit0 = 1
#define isMeshPoint(p)		(p&&(p->mesh_neighbor_TBL.State==MP_SUPERORDINATE_LINK_UP||p->mesh_neighbor_TBL.State==MP_SUBORDINATE_LINK_UP))

// Separate 3 define, Because decision MeshPoint/STA condition may different or NEW device (isXXX) in the future.
#define isPossibleNeighbor(p)	(MP_UNUSED != p->mesh_neighbor_TBL.State)
#define isSTA(p)  			(MP_UNUSED == p->mesh_neighbor_TBL.State)
#define isSTA2(p)  			(MP_UNUSED == p.mesh_neighbor_TBL.State)


#define SMP_LOCK_MESH_PATH(__x__)
#define SMP_UNLOCK_MESH_PATH(__x__)
#define SMP_LOCK_MESH_QUEUE(__x__)
#define SMP_UNLOCK_MESH_QUEUE(__x__)
#define SMP_LOCK_MESH_PROXY(__x__)
#define SMP_UNLOCK_MESH_PROXY(__x__)
#define SMP_LOCK_MESH_PROXYUPDATE(__x__)
#define SMP_UNLOCK_MESH_PROXYUPDATE(__x__)
#define SMP_LOCK_MESH_PREQ(__x__)
#define SMP_UNLOCK_MESH_PREQ(__x__)
#define SMP_LOCK_MESH_ACL(__x__)
#define SMP_UNLOCK_MESH_ACL(__x__)
#define SMP_LOCK_MESH_MP_HDR(__X__)
#define SMP_UNLOCK_MESH_MP_HDR(__X__)




/*
 *	@brief	MESH  PeerLink_CAP number routine
 */
#define MESH_PEER_LINK_CAP_NUM(priv)	(priv->mesh_PeerCAP_cap)

#ifdef PU_STANDARD
typedef struct {
	UINT8 flag;
	UINT8 PUseq;
	UINT8 proxyaddr[MACADDRLEN];
	UINT16 addrNum;
	struct list_head addrs;
} ProxyUpdate;
#endif

#endif	// _MESH_UTIL_H_
