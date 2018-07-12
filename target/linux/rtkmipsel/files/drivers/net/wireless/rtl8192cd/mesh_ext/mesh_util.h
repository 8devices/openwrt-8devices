
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


#ifdef GREEN_HILL
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

#elif defined(SMP_SYNC) //Add these spin locks to avoid deadlock under SMP platforms.

#if defined(CONFIG_USB_HCI) || defined(CONFIG_SDIO_HCI)
#define SMP_LOCK_MESH_PATH(__x__) do { } while (0)
#define SMP_UNLOCK_MESH_PATH(__x__) do { } while (0)
#define SMP_LOCK_MESH_QUEUE(__x__) do { } while (0)
#define SMP_UNLOCK_MESH_QUEUE(__x__) do { } while (0)
#define SMP_LOCK_MESH_PROXY(__x__)  do { } while (0)
#define SMP_UNLOCK_MESH_PROXY(__x__)  do { } while (0)
#define SMP_LOCK_MESH_PROXYUPDATE(__x__) do { } while (0)
#define SMP_UNLOCK_MESH_PROXYUPDATE(__x__) do { } while (0)
#define SMP_LOCK_MESH_PREQ(__x__)     do { } while (0)
#define SMP_UNLOCK_MESH_PREQ(__x__)   do { } while (0)
#define SMP_LOCK_MESH_MP_HDR(__X__)			do { spin_lock_bh(&priv->mesh_mp_hdr_lock); (void)(__x__); } while (0)
#define SMP_UNLOCK_MESH_MP_HDR(__X__)			do { spin_unlock_bh(&priv->mesh_mp_hdr_lock); (void)(__x__); } while (0)
#define SMP_LOCK_MESH_ACL(__x__)		do { spin_lock(&priv->mesh_acl_list_lock); (void)(__x__); } while (0)
#define SMP_UNLOCK_MESH_ACL(__x__)		do { spin_unlock(&priv->mesh_acl_list_lock); (void)(__x__); } while (0)

#elif defined(CONFIG_PCI_HCI)


#define SMP_LOCK_MESH_MP_HDR(__X__)
#define SMP_UNLOCK_MESH_MP_HDR(__X__)
#define SMP_LOCK_MESH_ACL(__x__)
#define SMP_UNLOCK_MESH_ACL(__x__)

#if 0

#define SMP_LOCK_MESH_PATH(__x__)      spin_lock_irqsave(&priv->mesh_path_lock, (__x__))
#define SMP_UNLOCK_MESH_PATH(__x__)    spin_unlock_irqrestore(&priv->mesh_path_lock, (__x__))
#define SMP_LOCK_MESH_QUEUE(__x__)     spin_lock_irqsave(&priv->mesh_queue_lock, (__x__))
#define SMP_UNLOCK_MESH_QUEUE(__x__)   spin_unlock_irqrestore(&priv->mesh_queue_lock, (__x__))
#define SMP_LOCK_MESH_PROXY(__x__)     spin_lock_irqsave(&priv->mesh_proxy_lock, (__x__))
#define SMP_UNLOCK_MESH_PROXY(__x__)   spin_unlock_irqrestore(&priv->mesh_proxy_lock, (__x__))
#define SMP_LOCK_MESH_PROXYUPDATE(__x__)     spin_lock_irqsave(&priv->mesh_proxyupdate_lock, (__x__))
#define SMP_UNLOCK_MESH_PROXYUPDATE(__x__)   spin_unlock_irqrestore(&priv->mesh_proxyupdate_lock, (__x__))
#define SMP_LOCK_MESH_PREQ(__x__)     spin_lock_irqsave(&priv->mesh_preq_lock, (__x__))
#define SMP_UNLOCK_MESH_PREQ(__x__)   spin_unlock_irqrestore(&priv->mesh_preq_lock, (__x__))

#else

#define SMP_LOCK_MESH_QUEUE(__x__)      do { \
                                            if(priv->mesh_queue_lock_owner!=smp_processor_id()) \
                                                spin_lock_irqsave(&priv->mesh_queue_lock, __x__); \
                                            else {\
                                                panic_printk("[%s %d] recursion detection, caller=%p\n",__FUNCTION__,__LINE__,__builtin_return_address(0)); \
                                                panic_printk("Previous Lock Function is %s\n",priv->mesh_queue_lock_func); \
                                                panic_printk("Priv is %s\n",priv->dev->name); \
                                            } \
                                            strcpy(priv->mesh_queue_lock_func, __FUNCTION__);\
                                            priv->mesh_queue_lock_owner=smp_processor_id();\
                                        }while(0)
#define SMP_UNLOCK_MESH_QUEUE(__x__)   do {priv->mesh_queue_lock_owner=-1;spin_unlock_irqrestore(&priv->mesh_queue_lock, __x__);}while(0)

#define SMP_LOCK_MESH_PATH(__x__)      do { \
                                            if(priv->mesh_path_lock_owner!=smp_processor_id()) \
                                                spin_lock_irqsave(&priv->mesh_path_lock, __x__); \
                                            else {\
                                                panic_printk("[%s %d] recursion detection, caller=%p\n", __FUNCTION__,__LINE__,__builtin_return_address(0)); \
                                                panic_printk("Previous Lock Function is %s\n",priv->mesh_path_lock_func); \
                                                panic_printk("Priv is %s\n",priv->dev->name); \
                                            } \
                                            strcpy(priv->mesh_path_lock_func, __FUNCTION__);\
                                            priv->mesh_path_lock_owner=smp_processor_id();\
                                        }while(0)
#define SMP_UNLOCK_MESH_PATH(__x__)   do {priv->mesh_path_lock_owner=-1;spin_unlock_irqrestore(&priv->mesh_path_lock, __x__);}while(0)



#define SMP_LOCK_MESH_PROXY(__x__)      do { \
                                            if(priv->mesh_proxy_lock_owner!=smp_processor_id()) \
                                                spin_lock_irqsave(&priv->mesh_proxy_lock, __x__); \
                                            else {\
                                                panic_printk("[%s %d] recursion detection, caller=%p\n", __FUNCTION__,__LINE__,__builtin_return_address(0)); \
                                                panic_printk("Previous Lock Function is %s\n",priv->mesh_proxy_lock_func); \
                                                panic_printk("Priv is %s\n",priv->dev->name); \
                                            } \
                                            strcpy(priv->mesh_proxy_lock_func, __FUNCTION__);\
                                            priv->mesh_proxy_lock_owner=smp_processor_id();\
                                        }while(0)
#define SMP_UNLOCK_MESH_PROXY(__x__)   do {priv->mesh_proxy_lock_owner=-1;spin_unlock_irqrestore(&priv->mesh_proxy_lock, __x__);}while(0)
#define SMP_LOCK_MESH_PROXYUPDATE(__x__) do { \
                                            if(priv->mesh_proxyupdate_lock_owner!=smp_processor_id()) \
                                                spin_lock_irqsave(&priv->mesh_proxyupdate_lock, __x__); \
                                            else {\
                                                panic_printk("[%s %d] recursion detection, caller=%p\n",__FUNCTION__,__LINE__,__builtin_return_address(0)); \
                                                panic_printk("Previous Lock Function is %s\n",priv->mesh_proxyupdate_lock_func); \
                                                panic_printk("Priv is %s\n",priv->dev->name); \
                                            } \
                                            strcpy(priv->mesh_proxyupdate_lock_func, __FUNCTION__);\
                                            priv->mesh_proxyupdate_lock_owner=smp_processor_id();\
                                        }while(0)
#define SMP_UNLOCK_MESH_PROXYUPDATE(__x__) do {priv->mesh_proxyupdate_lock_owner=-1;spin_unlock_irqrestore(&priv->mesh_proxyupdate_lock, __x__);}while(0)


#define SMP_LOCK_MESH_PREQ(__x__)      do { \
                                            if(priv->mesh_preq_lock_owner!=smp_processor_id()) \
                                                spin_lock_irqsave(&priv->mesh_preq_lock, __x__); \
                                            else {\
                                                panic_printk("[%s %d] recursion detection, caller=%p\n",__FUNCTION__,__LINE__,__builtin_return_address(0)); \
                                                panic_printk("Previous Lock Function is %s\n",priv->mesh_preq_lock_func); \
                                                panic_printk("Priv is %s\n",priv->dev->name); \
                                            } \
                                            strcpy(priv->mesh_preq_lock_func, __FUNCTION__);\
                                            priv->mesh_preq_lock_owner=smp_processor_id();\
                                        }while(0)
#define SMP_UNLOCK_MESH_PREQ(__x__)   do {priv->mesh_preq_lock_owner=-1;spin_unlock_irqrestore(&priv->mesh_preq_lock, __x__);}while(0)

#endif


#endif

#else
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


#endif // not GREEN_HILL


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
