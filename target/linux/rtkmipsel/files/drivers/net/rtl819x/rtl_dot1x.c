/*
 * Copyright c                Realtek Semiconductor Corporation, 2013
 * All rights reserved.
 *
 *
 */

#include <linux/version.h>
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,4,0)
#include <linux/kconfig.h>
#else
#include <linux/config.h>
#include <linux/autoconf.h>
#endif
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/compiler.h>
#include <linux/netdevice.h>
#include <linux/inetdevice.h>
#include <linux/etherdevice.h>
#include <linux/init.h>
#include <linux/pci.h>
#include <linux/delay.h>
#include <linux/ethtool.h>
#include <linux/mii.h>
#include <linux/if_vlan.h>
#include <linux/crc32.h>
#include <asm/io.h>
#include <asm/uaccess.h>
#include <linux/slab.h>
#include <linux/signal.h>
#include <linux/proc_fs.h>
#include <linux/time.h>
#include <linux/rtc.h>
#include <bsp/bspchip.h>
#include <linux/timer.h>


#include "version.h"
#include <net/rtl/rtl_types.h>
#include <net/rtl/rtl_glue.h>

#include "AsicDriver/asicRegs.h"
#include "AsicDriver/rtl865x_asicCom.h"
#include "AsicDriver/rtl865x_asicL2.h"
#ifdef CONFIG_RTL_LAYERED_ASIC_DRIVER_L3
#include "AsicDriver/rtl865x_asicL3.h"
#endif

#include "common/mbuf.h"
#include <net/rtl/rtl_queue.h>
#include "common/rtl_errno.h"
#include "rtl865xc_swNic.h"

/*common*/
#include "common/rtl865x_vlan.h"
#include <net/rtl/rtl865x_netif.h>
#include "common/rtl865x_netif_local.h"

/*l2*/
#ifdef CONFIG_RTL_LAYERED_DRIVER_L2
#include "l2Driver/rtl865x_fdb.h"
#include <net/rtl/rtl865x_fdb_api.h>
#endif

/*l3*/
#ifdef CONFIG_RTL_LAYERED_DRIVER_L3
#include "l3Driver/rtl865x_ip.h"
#include "l3Driver/rtl865x_nexthop.h"
#include <net/rtl/rtl865x_ppp.h>
#include "l3Driver/rtl865x_ppp_local.h"
#include "l3Driver/rtl865x_route.h"
#include "l3Driver/rtl865x_arp.h"
#include <net/rtl/rtl865x_nat.h>
#endif

#include <net/rtl/rtl_dot1x.h>

static unsigned int old_link_port_mask=0;
static unsigned int new_link_port_mask=0;
rtl802Dot1xConfig dot1x_config;
struct pid *_authapp_pid;
pid_t       authapp_pid;
rtl802Dot1xQueue dot1x_queue;
CTAILQ_HEAD(dot1x_list_inuse_head, __rtl802Dot1xCacheEntry) dot1x_list_inuse;
CTAILQ_HEAD(dot1x_list_free_head, __rtl802Dot1xCacheEntry) dot1x_list_free;

struct __rtl802Dot1xCacheTable *table_dot1x;
static int dot1x_table_list_max;



extern int32 rtl865x_updateAuthState(int32 auth, const unsigned char *addr);
unsigned int rtl865x_getPhysicalPortLinkStatus(void);

#define rtl_82dot1xIsEmptyQueue(q) (q->item_num==0 ? 1:0)
#define rtl_82dot1xIsFullQueue(q) (q->item_num==q->max_item? 1:0)
#define rtl_82dot1xNumItemQueue(q) q->item_num

#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,4,0)
#define NETDRV_PRIV(X) netdev_priv(X)
#else
#define NETDRV_PRIV(X) ((X)->priv)
#endif

#if defined(CONFIG_RTL_PROC_NEW)
extern struct proc_dir_entry proc_root;

static int32 read_proc_8021x_type(struct seq_file *s, void *v);
static int32 read_proc_8021x_enable(struct seq_file *s, void *v);
static int32 read_proc_8021x_auth_port_mode(struct seq_file *s, void *v);
static int32 read_proc_8021x_entry(struct seq_file *s, void *v);
static int32 read_proc_8021x_auth_server_port(struct seq_file *s, void *v);

static int32 write_proc_8021x_enable( struct file *filp, const char *buff,unsigned long len, void *data );
static int32 write_proc_8021x_type( struct file *filp, const char *buff,unsigned long len, void *data );
static int32 write_proc_8021x_auth_server_port( struct file *filp, const char *buff,unsigned long len, void *data );
static int32 write_proc_8021x_auth_port_mode( struct file *filp, const char *buff,unsigned long len, void *data );


int dot1x_enable_single_open(struct inode *inode, struct file *file)
{
        return(single_open(file, read_proc_8021x_enable, PDE_DATA(inode)));
}
static ssize_t dot1x_enable_single_write(struct file * file, const char __user * userbuf,
		     size_t count, loff_t * off)
{
	    return write_proc_8021x_enable(file, userbuf,count, off);
}
struct file_operations dot1x_enable_proc_fops= {
        .open           = dot1x_enable_single_open,
        .write		    = dot1x_enable_single_write,
        .read           = seq_read,
        .llseek         = seq_lseek,
        .release        = single_release,
};

int dot1x_type_single_open(struct inode *inode, struct file *file)
{
        return(single_open(file, read_proc_8021x_type, PDE_DATA(inode)));
}
static ssize_t dot1x_type_single_write(struct file * file, const char __user * userbuf,
		     size_t count, loff_t * off)
{
	    return write_proc_8021x_type(file, userbuf,count, off);
}
struct file_operations dot1x_type_proc_fops= {
        .open           = dot1x_type_single_open,
        .write		    = dot1x_type_single_write,
        .read           = seq_read,
        .llseek         = seq_lseek,
        .release        = single_release,
};

int dot1x_mode_single_open(struct inode *inode, struct file *file)
{
        return(single_open(file, read_proc_8021x_auth_port_mode, PDE_DATA(inode)));
}
static ssize_t dot1x_mode_single_write(struct file * file, const char __user * userbuf,
		     size_t count, loff_t * off)
{
	    return write_proc_8021x_auth_port_mode(file, userbuf,count, off);
}
struct file_operations dot1x_mode_proc_fops= {
        .open           = dot1x_mode_single_open,
        .write		    = dot1x_mode_single_write,
        .read           = seq_read,
        .llseek         = seq_lseek,
        .release        = single_release,
};


int dot1x_server_port_single_open(struct inode *inode, struct file *file)
{
        return(single_open(file, read_proc_8021x_auth_server_port, PDE_DATA(inode)));
}
static ssize_t dot1x_server_port_single_write(struct file * file, const char __user * userbuf,
		     size_t count, loff_t * off)
{
	    return write_proc_8021x_auth_server_port(file, userbuf,count, off);
}
struct file_operations dot1x_server_port_proc_fops= {
        .open           = dot1x_server_port_single_open,
        .write		    = dot1x_server_port_single_write,
        .read           = seq_read,
        .llseek         = seq_lseek,
        .release        = single_release,
};

int dot1x_entry_single_open(struct inode *inode, struct file *file)
{
        return(single_open(file, read_proc_8021x_entry, PDE_DATA(inode)));
}
struct file_operations dot1x_entry_proc_fops= {
        .open           = dot1x_entry_single_open,
        .write		    = NULL,
        .read           = seq_read,
        .llseek         = seq_lseek,
        .release        = single_release,
};

#endif


__IRAM_GEN inline static uint32
rtl_82dot1xHashEntry(uint8 *mac_addr)
{
	register uint32 hash;
	
	if (mac_addr == NULL)
		return 0;
	
	hash = mac_addr[0] | (mac_addr[1]<<8);
	hash ^= (mac_addr[2] | (mac_addr[3]<<8));
	hash ^= (mac_addr[4] | (mac_addr[5]<<8));
	
	return (dot1x_table_list_max-1) & (hash ^ (hash >> 12));


}

int rtl_82dot1xAddEntry(uint8 *mac_addr, 
								uint32 id,
								uint8  rx_port_num,
							    uint8 auth_state)
{
	uint32 hash = 0;
	rtl802Dot1xCacheEntry *ep;
	
	if (mac_addr == NULL)
		return FAILED;
	
	hash = rtl_82dot1xHashEntry(mac_addr);

	printk("%s %d: mac=0x%x:%x:%x:%x:%x:%x id=%u portnum=%d state=%d\n", __FUNCTION__, __LINE__, mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5], id, rx_port_num, auth_state);
	/* Lookup */
	CTAILQ_FOREACH(ep, &table_dot1x->list[hash], cache_link) {
		if ((!memcmp(mac_addr, ep->mac_addr, ETH_ALEN))&&(ep->valid == DOT1X_TABLE_ENTRY_VALID)){
			ep->id = id;
			ep->rx_port_num = rx_port_num;
			ep->auth_state = auth_state;
			printk("already exist! \n");
			return SUCCESS;
		}
	}

	/* Create */
	if(!CTAILQ_EMPTY(&dot1x_list_free)) {
		rtl802Dot1xCacheEntry *entry;
		entry = CTAILQ_FIRST(&dot1x_list_free);
		entry->id = id;
		memcpy(entry->mac_addr, mac_addr, ETH_ALEN);
		entry->rx_port_num = rx_port_num;
		entry->auth_state = auth_state;
		entry->valid = DOT1X_TABLE_ENTRY_VALID;
		CTAILQ_REMOVE(&dot1x_list_free, entry, tqe_link);
		CTAILQ_INSERT_TAIL(&dot1x_list_inuse, entry, tqe_link);
		CTAILQ_INSERT_TAIL(&table_dot1x->list[hash], entry, cache_link);
	} else {
		printk("add: ERROR - dot1x_list_free is empty! \n");
		return FAILED;
	}
	
	return SUCCESS;
}

int rtl_82dot1xModifyEntryByMac(uint8 *mac_addr, 
											uint32 id,
											uint8  rx_port_num,
										    uint8 auth_state)
{
	uint32 hash = 0;
	rtl802Dot1xCacheEntry *ep;
	if (mac_addr == NULL)
		return FAILED;
	
	hash = rtl_82dot1xHashEntry(mac_addr);

	printk("%s %d: mac=0x%x:%x:%x:%x:%x:%x id=%u portnum=%d state=%d\n", __FUNCTION__, __LINE__, mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5], id, rx_port_num, auth_state);
	/* Lookup */
	CTAILQ_FOREACH(ep, &table_dot1x->list[hash], cache_link) {
		if ((!memcmp(mac_addr, ep->mac_addr, ETH_ALEN))&&(ep->valid == DOT1X_TABLE_ENTRY_VALID)){
			ep->id = id;
			ep->rx_port_num = rx_port_num;
			ep->auth_state = auth_state;
			return SUCCESS;
		}
	}
	
	return SUCCESS;
}

static inline rtl802Dot1xCacheEntry *rtl_82dot1xFindEntryByMac(uint8 *mac_addr)
{
	
	uint32 hash = 0;
	rtl802Dot1xCacheEntry *entry;
	if (mac_addr == NULL)
		return NULL;
	
	hash = rtl_82dot1xHashEntry(mac_addr);
	
	CTAILQ_FOREACH(entry, &table_dot1x->list[hash], cache_link)
	{
		if ((!memcmp(mac_addr, entry->mac_addr, ETH_ALEN))&&(entry->valid == DOT1X_TABLE_ENTRY_VALID))
		{
			return ((rtl802Dot1xCacheEntry *)entry);
		}
	}
	return NULL;
}

static inline rtl802Dot1xCacheEntry *rtl_82dot1xFindEntryById(uint32 id)
{	
	rtl802Dot1xCacheEntry *entry;

	printk("%s %d: id=%u \n", __FUNCTION__, __LINE__, id);
	CTAILQ_FOREACH(entry, &dot1x_list_inuse, cache_link)
	{
		if ((entry->id == id) && (entry->valid == DOT1X_TABLE_ENTRY_VALID))
		{
			return ((rtl802Dot1xCacheEntry *)entry);
		}
	}
	
	return NULL;
}

int rtl_82dot1xModifyEntryById(uint32 id,
									    uint8 auth_state,
									    uint8 *mac_addr,
									    uint8 *portnum)
{
	rtl802Dot1xCacheEntry *ep;

	printk("%s %d: id=%u state=%d\n", __FUNCTION__, __LINE__, id, auth_state);
	/* Lookup */
	CTAILQ_FOREACH(ep, &dot1x_list_inuse, cache_link) {
		if ((ep->id == id) && (ep->valid == DOT1X_TABLE_ENTRY_VALID)){
			if (mac_addr != NULL)
				memcpy(mac_addr, ep->mac_addr, ETH_ALEN);
			*portnum = ep->rx_port_num;
			ep->auth_state = auth_state;
			return SUCCESS;
		}
	}
	
	return FAILED;
}



int rtl_82dot1xdelEntryByMac(uint8 *mac_addr)
{
	uint32 hash = 0;
	rtl802Dot1xCacheEntry *ep;

	if (mac_addr == NULL)
		return FAILED;
	
	hash = rtl_82dot1xHashEntry(mac_addr);
	

	printk("%s %d: mac=0x%x:%x:%x:%x:%x:%x \n", __FUNCTION__, __LINE__, mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
	/* Lookup */
	CTAILQ_FOREACH(ep, &table_dot1x->list[hash], cache_link) {
		if (!memcmp(mac_addr, ep->mac_addr, ETH_ALEN)){
			ep->valid = DOT1X_TABLE_ENTRY_INVALID;
			CTAILQ_REMOVE(&table_dot1x->list[hash], ep, cache_link);
			CTAILQ_REMOVE(&dot1x_list_inuse, ep, tqe_link);
			CTAILQ_INSERT_TAIL(&dot1x_list_free, ep, tqe_link);
			return SUCCESS;
		}
	}
	
	return FAILED;
}

int rtl_82dot1xdelEntryByPid(uint32 pid)
{
	rtl802Dot1xCacheEntry *ep;	
	uint32 hash = 0;
	#if defined(DOT1X_DEBUG)
	panic_printk("%s %d: pid=%d \n", __FUNCTION__, __LINE__, pid);
	#endif
	//if ((dot1x_config.mode[pid].auth_mode == DOT1X_AUTH_MODE_SNOOPING)&&
		//(dot1x_config.mode[pid].port_enable))
	{
		/* Lookup snooping mode port down handle //other mode handle by auth daemon */
		CTAILQ_FOREACH(ep, &dot1x_list_inuse, cache_link) {
			if (ep->rx_port_num == pid) {
				ep->valid = DOT1X_TABLE_ENTRY_INVALID;
				hash = rtl_82dot1xHashEntry(ep->mac_addr);
				CTAILQ_REMOVE(&table_dot1x->list[hash], ep, cache_link);
				CTAILQ_REMOVE(&dot1x_list_inuse, ep, tqe_link);
				CTAILQ_INSERT_TAIL(&dot1x_list_free, ep, tqe_link);
				if (dot1x_config.type==DOT1X_AUTH_TYPE_MAC_BASED)
				{
					rtl865x_updateAuthState(0, ep->mac_addr);
				}
			}
		}
		
		if (dot1x_config.type==DOT1X_AUTH_TYPE_PORT_BASED)
		{
			rtl_802dot1xSetPortAuthState(pid, 0);
		}
	}
	
	return SUCCESS;
}



void rtl_82dot1xInitQueue(rtl802Dot1xQueue *q)
{
	if (q == NULL)
		return ;
	memset(q, 0x00, sizeof(rtl802Dot1xQueue));
	
	q->head = 0;
	q->tail = 0;
	q->item_num = 0;
	q->max_item = DOT1X_MAX_QUEUE_SIZE;

	return ;
}

int rtl_82dot1xEnQueue(unsigned long task_priv, rtl802Dot1xQueue *q, unsigned char *item, int item_size)
{
#if 0//def __KERNEL__
	struct dev_priv	*priv = (struct dev_priv	*)task_priv;
#endif
	unsigned long flags;

	if(rtl_82dot1xIsFullQueue(q))
		return DOT1X_ERROR_QFULL;
	if(item_size > DOT1X_MAX_DATA_LEN)
		return DOT1X_ERROR_2LARGE;

	//SAVE_INT_AND_CLI(flags);
	local_irq_save(flags);
	q->item_array[q->tail].item_size = item_size;
	memset(q->item_array[q->tail].item, 0, sizeof(q->item_array[q->tail].item));
	memcpy(q->item_array[q->tail].item, item, item_size);
	q->item_num++;
	
	if((q->tail+1) == DOT1X_MAX_QUEUE_SIZE)
		q->tail = 0;
	else
		q->tail++;

	//RESTORE_INT(flags);
	local_irq_restore(flags);
	
	return 0;
}

int rtl_82dot1xDeQueue(unsigned long task_priv, rtl802Dot1xQueue *q, unsigned char *item, int *itemsize)
{
#if 0//def __KERNEL__
	struct dev_priv	*priv = (struct dev_priv	*)task_priv;
#endif
	unsigned long flags;
	rtl802Dot1xEapPkt *pdata;


	if ((q == NULL) || (item == NULL))
		return DOT1X_ERROR_ERROR;
	
	if(rtl_82dot1xIsEmptyQueue(q))
		return DOT1X_ERROR_QEMPTY;
	
	memset(item, 0x00, DOT1X_MAX_DATA_LEN);
	//SAVE_INT_AND_CLI(flags);
	local_irq_save(flags);
	memcpy(item, &q->item_array[q->head].item, q->item_array[q->head].item_size);
	q->item_num--;
	pdata = (rtl802Dot1xEapPkt *)item;
	pdata->flag = q->item_num;//update remain pkts number
	*itemsize = q->item_array[q->head].item_size;
	
	if((q->head+1) == DOT1X_MAX_QUEUE_SIZE)
		q->head = 0;
	else
		q->head++;

	//RESTORE_INT(flags);
	local_irq_restore(flags);
	
	return 0;
}

int32  rtl_802dot1xSetPortAuthState(int port_num, int auth_state)
{
	if (port_num > RTL8651_PORT_NUMBER || port_num < 0)
		return FAILED;

	if (auth_state)
	{
		WRITE_MEM32(DOT1XPORTCR, (READ_MEM32(DOT1XPORTCR)|(0x1<<((port_num*3) + 1))));
	}
    else
	{
		WRITE_MEM32(DOT1XPORTCR, (READ_MEM32(DOT1XPORTCR)&(~(0x1<<((port_num*3) + 1)))));
	}
	
    return 0;
            
}

int rtl_802dot1xIoctl(struct net_device *dev, struct ifreq *rq, int cmd)
{
	struct dev_priv	*priv = NETDRV_PRIV(dev);
    //int32 rc = 0;
	rtl802Dot1xAuthResult auth_result;
	unsigned int *data;
	uint32 args[4];
	static uint8 queue_data[DOT1X_MAX_DATA_LEN+4];
	int		queue_data_len;
	uint32 *pdata;
	
	data = (unsigned int *)rq->ifr_data;
	if (copy_from_user(args, data, 4*sizeof(unsigned int)))
	{
		return -EFAULT;
	}
	
    switch (args[0])
    {
		case RTL8651_IOCTL_DOT1X_GET_INFO:
			pdata = (uint32 *)args[3];
			rtl_82dot1xDeQueue((unsigned long)priv, &dot1x_queue, queue_data, &queue_data_len);
			if (copy_to_user((void *)pdata, (void *)queue_data, queue_data_len)) {
				return -EFAULT;
			}
			break;
        case RTL8651_IOCTL_DOT1X_SETPID:
            authapp_pid = -1;
			pdata = (uint32 *)args[1];
			if (pdata != NULL)
				authapp_pid = *pdata;
            if (authapp_pid != -1)
            {
                rcu_read_lock();
                _authapp_pid = get_pid(find_vpid(authapp_pid));
                rcu_read_unlock();
            }
			#if defined(DOT1X_DEBUG)
			panic_printk("%s %d authapp_pid=%d\n", __FUNCTION__, __LINE__, authapp_pid);
			#endif
            break;
		case RTL8651_IOCTL_DOT1X_SET_AUTH_RESULT:
			pdata = (uint32 *)args[1];
			memset(&auth_result, 0x00, sizeof(rtl802Dot1xAuthResult));
			if (copy_from_user((void *)&auth_result, (void *)pdata, sizeof(rtl802Dot1xAuthResult))) {
				return -EFAULT;
			}
			//set to hw
			#if defined(DOT1X_DEBUG)
			panic_printk("%s %d args=0x%x portnum=%d state=%d type=%d mac:0x%x:%x:%x:%x:%x:%x\n", 
			__FUNCTION__, __LINE__, args[1], auth_result.port_num, auth_result.auth_state, auth_result.type,
			auth_result.mac_addr[0], auth_result.mac_addr[1], auth_result.mac_addr[2], auth_result.mac_addr[3]
			, auth_result.mac_addr[4], auth_result.mac_addr[5]);
			#endif
			if (auth_result.type == DOT1X_AUTH_TYPE_PORT_BASED)
				rtl_802dot1xSetPortAuthState(auth_result.port_num, auth_result.auth_state);
			else if (auth_result.type == DOT1X_AUTH_TYPE_MAC_BASED)
				rtl865x_updateAuthState(auth_result.auth_state, auth_result.mac_addr);
			break;
		#if 0
		case RTL8651_IOCTL_DOT1X_GET_PORT_STATE_INFO:
			pdata = (uint32 *)args[1];
			if (copy_to_user(pdata, (void *)&port_state, sizeof(rtl802Dot1xPortStateInfo))) {
				return -EFAULT;
			}
			break;
		#endif	
         default:
            return -EOPNOTSUPP;
            break;
    }
    return SUCCESS;

}

void rtl_82dot1xEventInform(int event)
{
    #ifdef __KERNEL__
	if (authapp_pid > 0)
        //#ifdef LINUX_2_6_27_
        #if LINUX_VERSION_CODE >= 0x02061B
	{
		kill_pid(_authapp_pid, SIGIO, event);
	}
        #else
		kill_proc(authapp_pid, SIGIO, event);
        #endif
    #endif

    return ;
}

void rtl_82dot1xCheckPortState(void)
{
	unsigned int port_num=0, ret = 0;
	rtl802Dot1xPortStateInfo port_down_state;
	rtl802Dot1xPortStateInfo port_up_state;
	new_link_port_mask = rtl865x_getPhysicalPortLinkStatus();
	//panic_printk("%s %d new_link_port_mask = 0x%x cur_link_port_mask=0x%x\n", __FUNCTION__, __LINE__, new_link_port_mask, old_link_port_mask);
	if (new_link_port_mask != old_link_port_mask)
	{
	
		memset(&port_down_state, 0x00, sizeof(rtl802Dot1xPortStateInfo));
		memset(&port_up_state, 0x00, sizeof(rtl802Dot1xPortStateInfo));
		for(port_num = 0; port_num <= RTL8651_PHY_NUMBER; port_num++)
		{
			
			if ((((1<<port_num) & new_link_port_mask)==0)&&
				(((1<<port_num) & old_link_port_mask)!=0))
			{
				//up->down , delete entry by pid
				
				if ((dot1x_config.mode[port_num].auth_mode == DOT1X_AUTH_MODE_SNOOPING)&&
					(dot1x_config.mode[port_num].port_enable))
				{
					rtl_82dot1xdelEntryByPid(port_num);
				}
				port_down_state.port_mask |= (1<<port_num);
			}
			else if((((1<<port_num) & new_link_port_mask)!= 0)&&
				(((1<<port_num) & old_link_port_mask)==0)){
				//down -->up
				port_up_state.port_mask |= (1<<port_num);
			}
		}
		if (port_down_state.port_mask != 0)
		{
			
			#if defined(DOT1X_DEBUG)
			panic_printk("%s %d port_state.port_mask = 0x%x \n", __FUNCTION__, __LINE__, port_down_state.port_mask);
			#endif
			port_down_state.event_id = DOT1X_EVENT_PORT_DOWN;
			ret = rtl_82dot1xEnQueue(0, &dot1x_queue,
				(unsigned char*)&port_down_state, sizeof(rtl802Dot1xPortStateInfo));
			
		}
		if (port_up_state.port_mask != 0)
		{
			
			#if defined(DOT1X_DEBUG)
			panic_printk("%s %d port_state.port_mask = 0x%x \n", __FUNCTION__, __LINE__, port_down_state.port_mask);
			#endif
			port_up_state.event_id = DOT1X_EVENT_PORT_UP;
			ret = rtl_82dot1xEnQueue(0, &dot1x_queue,
				(unsigned char*)&port_up_state, sizeof(rtl802Dot1xPortStateInfo));
		
		}
		if(port_up_state.port_mask != 0 || port_down_state.port_mask != 0){
			if (ret == 0)
			{
				//tell auth daemon some port down.....
				rtl_82dot1xEventInform(1);
			}
			else
				printk("%s %d enqueue error ret = %d\n", __FUNCTION__, __LINE__, ret);
		}
	}
	old_link_port_mask = new_link_port_mask;

	return;
}
#if 0
static int read_proc_8021x_l2_mac_entry(char *page, char **start, off_t off,
        int count, int *eof, void *data)
{
    return 0;
}
static int write_proc_8021x_l2_mac_entry(struct file *file, const char *buffer,
              unsigned long count, void *data)
{
    return 0;
}

static int read_proc_8021x_mac_based_ctrl(char *page, char **start, off_t off,
        int count, int *eof, void *data)
{
    return 0;
}
static int write_proc_8021x_mac_based_ctrl(struct file *file, const char *buffer,
              unsigned long count, void *data)
{
    return 0;
}

static int read_proc_8021x_port_based_ctrl(char *page, char **start, off_t off,
        int count, int *eof, void *data)
{
    return 0;
}
static int write_proc_8021x_port_based_ctrl(struct file *file, const char *buffer,
              unsigned long count, void *data)
{
    /* usage: echo "portnum enable/disable auth/unauth direction" > /proc/802dot1x/rtl_8021x_port_based_ctrl 
        *           eg:port0 enable auth BOTH direction
        *           echo "0 1 1 0" >/proc/802dot1x/rtl_8021x_port_based_ctrl
        */
    int portnum = 0;
    int enable = 0;
    int auth = 0;
    int direction = 0;
    #define	MAX_802DOT1X_INPUT_LEN	128
    char *tmp;

    tmp = kmalloc(MAX_802DOT1X_INPUT_LEN, GFP_KERNEL);
    if (count < 2 || tmp==NULL)
        goto out;

    if(rtl_802dot1x_enable_type != 1)
        goto out;

    if (buffer && !copy_from_user(tmp, buffer, MAX_802DOT1X_INPUT_LEN))
    {
            int num = sscanf(tmp, "%d %d %d %d",
            &portnum, &enable,
            &auth, &direction
            );

        if (num !=4)
        {
            printk("invalid 802dot1x portbased parameter!\n");
            goto out;
        }

        //WRITE_MEM32(DOT1XPORTCR, REG32(REG32)|0x00);

    }
    
out:
    if(tmp)
        kfree(tmp);

    return count;
}
#endif

#if defined(CONFIG_RTL_PROC_NEW)
static int32 read_proc_8021x_entry(struct seq_file *s, void *v)
{
	rtl802Dot1xCacheEntry *ep;

	/* Lookup */
	CTAILQ_FOREACH(ep, &dot1x_list_inuse, cache_link) {
		if ((ep->valid == DOT1X_TABLE_ENTRY_VALID)){
			seq_printf(s, "portnum:%d mac:0x%x:%x:%x:%x:%x:%x authstate:%d id:%d\n", ep->rx_port_num, ep->mac_addr[0],ep->mac_addr[1]
				, ep->mac_addr[2],ep->mac_addr[3], ep->mac_addr[4],ep->mac_addr[5], ep->auth_state, ep->id);
			}
		}
	
	return 0;
}
static int32 read_proc_8021x_auth_server_port(struct seq_file *s, void *v)
{
	seq_printf(s, "%s %d\n", "802dot1x server port:",dot1x_config.server_port);
	
	return 0;
}

#else
static int32 read_proc_8021x_entry( char *page, char **start, off_t off, int count, int *eof, void *data )
{
	int len = 0;

	rtl802Dot1xCacheEntry *ep;

	/* Lookup */
	CTAILQ_FOREACH(ep, &dot1x_list_inuse, cache_link) {
		if ((ep->valid == DOT1X_TABLE_ENTRY_VALID)){
			len += sprintf(page+len, "portnum:%d mac:0x%x:%x:%x:%x:%x:%x authstate:%d id:%d\n", ep->rx_port_num, ep->mac_addr[0],ep->mac_addr[1]
				, ep->mac_addr[2],ep->mac_addr[3], ep->mac_addr[4],ep->mac_addr[5], ep->auth_state, ep->id);
			}
		}
	if (len <= off+count) *eof = 1;
	*start = page + off;
	len -= off;
	if (len>count)
		len = count;
	if (len<0)
	  	len = 0;

	return len;
}

static int32 read_proc_8021x_auth_server_port( char *page, char **start, off_t off, int count, int *eof, void *data )
{
	int len;
	len = sprintf(page, "%s %d\n", "802dot1x server port:",dot1x_config.server_port);


	if (len <= off+count) *eof = 1;
	*start = page + off;
	len -= off;
	if (len>count)
		len = count;
	if (len<0)
	  	len = 0;

	return len;
}
#endif
static int32 write_proc_8021x_auth_server_port( struct file *filp, const char *buff,unsigned long len, void *data )
{
	char 		tmpbuf[32];
    int num;
    int32 port;

    
	if(!dot1x_config.enable)
	{
		printk("rtl ethernet 8021x support not enabled!!!\n");
		return len;
	}

	if (buff && !copy_from_user(tmpbuf, buff, len))
	{
	    
		num = sscanf(tmpbuf,"%d",&port);
        
        if((num != 1) ||(port > RTL8651_PORT_NUMBER || port < 0))
        {
            printk("rtl ethernet 8021x set server port parameter error!!\n");
            return len; 
        }
		
        dot1x_config.server_port = port;

	}
    
	return len;
}

#if defined(CONFIG_RTL_PROC_NEW)
static int32 read_proc_8021x_auth_port_mode(struct seq_file *s, void *v)
{
	int i = 0;
	
	seq_printf(s, "%s\n", "port config:");
	
	for (i = 0; i < RTL8651_PORT_NUMBER; i++)
	{
		seq_printf(s, "%s %d %s %d %s %d %s %d\n", 
			"mode:",dot1x_config.mode[i].auth_mode, "portnum:", i, "enable:", dot1x_config.mode[i].port_enable,
			"dir", dot1x_config.mode[i].auth_dir);
	}
	
	return 0;
}
#else
static int32 read_proc_8021x_auth_port_mode( char *page, char **start, off_t off, int count, int *eof, void *data )
{
	int len, i;
	
	len = sprintf(page, "%s\n", "port config:");
	for (i = 0; i < RTL8651_PORT_NUMBER; i++)
	{
		len += sprintf(page+len, "%s %d %s %d %s %d %s %d\n", 
			"mode:",dot1x_config.mode[i].auth_mode, "portnum:", i, "enable:", dot1x_config.mode[i].port_enable,
			"dir", dot1x_config.mode[i].auth_dir);
	}


	if (len <= off+count) *eof = 1;
	*start = page + off;
	len -= off;
	if (len>count)
		len = count;
	if (len<0)
	  	len = 0;

	return len;
}
#endif
static int32 write_proc_8021x_auth_port_mode( struct file *filp, const char *buff,unsigned long len, void *data )
{
	char 		tmpbuf[32];
	int mode, portnum, enable, num, dir;
    
	if(!dot1x_config.enable)
	{
		printk("rtl ethernet 8021x support not enabled!!!\n");
		return len;
	}

	if (buff && !copy_from_user(tmpbuf, buff, len))
	{
	
		num=sscanf(tmpbuf,"%d %d %d %d",&enable, &portnum, &mode, &dir);
		
		if(num!=4){
			printk("set 801dot1x mode parameter error !!!\n");
			return len;
		}
		if (portnum > RTL8651_PORT_NUMBER || portnum < 0)
		{
			printk("set 801dot1x mode portnum invalid!!!\n");
			return len;
		}
		if (mode > 3 || mode < 1)
		{
			printk("set 801dot1x mode mode invalid!!!\n");
			return len;
		}
		
		dot1x_config.mode[portnum].auth_mode = mode & 0xff;
		dot1x_config.mode[portnum].port_enable = enable & 0xff;
		dot1x_config.mode[portnum].auth_dir = dir & 0xff;
		
		//should set dot1xConfig.type first
		if ((mode == DOT1X_AUTH_MODE_SNOOPING) || (mode == DOT1X_AUTH_MODE_PROXY))
		{
			if (dot1x_config.type == DOT1X_AUTH_TYPE_PORT_BASED)
			{
				rtl_enablePortbase802dot1x(enable, portnum);
				rtl_setPortbase802dot1xDir(dir, portnum);
			}
			else if (dot1x_config.type == DOT1X_AUTH_TYPE_MAC_BASED)
			{
				rtl_enableMacbase802dot1x(enable, portnum);
				rtl_setMacbase802dot1xDir(dir, portnum);
			}
		}
		else if (mode == DOT1X_AUTH_MODE_CLIENT)
		{
			//donot need set register, clear....portbase/mac base
			if (dot1x_config.type == DOT1X_AUTH_TYPE_PORT_BASED)
			{
				rtl_enablePortbase802dot1x(0, portnum);
			}
			else if (dot1x_config.type == DOT1X_AUTH_TYPE_MAC_BASED)
			{
				rtl_enableMacbase802dot1x(0, portnum);
			}
		}
		else
		{
			printk("current support: 1/2/3\n");
			return len;
		}
    

	}
	return len;
}
#if defined(CONFIG_RTL_PROC_NEW)
static int32 read_proc_8021x_type(struct seq_file *s, void *v)
{
	seq_printf(s, "%s %d\n", "802dot1x auth type:",dot1x_config.type);

	return 0;
}

static int32 read_proc_8021x_enable(struct seq_file *s, void *v)
{
	seq_printf(s, "%s %d %s %d\n", "802dot1x enable:",dot1x_config.enable, "unicast response enable", dot1x_config.enable_unicastresp);
	
    return 0;
}

#else
static int32 read_proc_8021x_type( char *page, char **start, off_t off, int count, int *eof, void *data )
{
	int len;
	len = sprintf(page, "%s %d\n", "802dot1x auth type:",dot1x_config.type);


	if (len <= off+count) *eof = 1;
	*start = page + off;
	len -= off;
	if (len>count)
		len = count;
	if (len<0)
	  	len = 0;

	return len;
}

static int32 read_proc_8021x_enable( char *page, char **start, off_t off, int count, int *eof, void *data )
{
	int len;
	len = sprintf(page, "%s %d %s %d\n", "802dot1x enable:",dot1x_config.enable, "unicast response enable", dot1x_config.enable_unicastresp);


	if (len <= off+count) *eof = 1;
	*start = page + off;
	len -= off;
	if (len>count)
		len = count;
	if (len<0)
	  	len = 0;

	return len;
}
#endif
int32  rtl_disable802dot1x(void)
{
    /* clear hw register */
    WRITE_MEM32(DOT1XPORTCR, 0x00);
    WRITE_MEM32(DOT1XMACCR, 0x00);
    WRITE_MEM32(GVGCR, 0x00);
    WRITE_MEM32(RMACR, (READ_MEM32(RMACR)&(~MADDR03)));
    
    /* reinit l2 table?? */
#ifdef CONFIG_RTL_LAYERED_DRIVER_L2
	rtl865x_layer2_reinit();
#endif
    
    //memset(&dot1xPortBasedInfo, 0x00, RTL8651_PORT_NUMBER*sizeof(rtl802Dot1xPortBasedInfo));

#if 0//test8021x
#if 1//enable port based
    WRITE_MEM32(DOT1XPORTCR, 0x25b); //port0/port1 authorized,port2/port3 unauthorized
    WRITE_MEM32(GVGCR, 1<<12);//trap unauthenticated frame to CPU
#else//enable mac based
    //bind authenticated host (macaddr:0x00:21:70:9d:a9:2f) to a specified port3
    WRITE_MEM32(DOT1XMACCR, 0xf); //port0/port1/port2/port3 enable mac based 
    WRITE_MEM32(GVGCR, 1<<12);//trap unauthenticated frame to CPU
#endif
#endif

    return 0;
            
}

#if 0
static void rtl_802dot1x_portbase_timeout(unsigned long arg)
{
	rtl802Dot1xPortBasedInfo *entry = (rtl802Dot1xPortBasedInfo *)arg;
    /* if a client from the beginning (send out EAPOL-Start) to timeout still in state of DOT1X_AUTH_STATE_START
        * will regard as  un-authentication, clear related info wait for another EAPOL-Start
        */
    if (entry->authState == DOT1X_AUTH_STATE_START)
    {
        entry->authState = DOT1X_AUTH_STATE_INACTIVE;
        entry->portNum = -1;
        memset(entry->macaddr, 0x00, ETH_ALEN);
    }
    mod_timer(&entry->expire_timer, jiffies + DOT1X_AUTH_TIME_OUT);
    
    return;
}
#endif
int32  rtl_enablePortbase802dot1x(int32 enable, int32 portnum)
{

	if (portnum > RTL8651_PORT_NUMBER || portnum < 0)
	{
		printk("%s %d port number invalid\n", __FUNCTION__, __LINE__);
		return FAILED;
	}
	
    /* enable/disable  Port-based Access Control */
	if (enable)
	    WRITE_MEM32(DOT1XPORTCR, (READ_MEM32(DOT1XPORTCR)|(0x1<<(portnum*3))));
	else
	    WRITE_MEM32(DOT1XPORTCR, (READ_MEM32(DOT1XPORTCR)&(~(0x1<<(portnum*3)))));
    
    return 0;
            
}

int32  rtl_enableMacbase802dot1x(int32 enable, int32 portnum)
{
    
	if (portnum > RTL8651_PORT_NUMBER || portnum < 0)
	{
		printk("%s %d port number invalid\n", __FUNCTION__, __LINE__);
		return FAILED;
	}

    /* enable/disable MAC Address Based Control on each port  */
	if (enable)
	    WRITE_MEM32(DOT1XMACCR, (READ_MEM32(DOT1XMACCR)|(0x1<<portnum)));
	else
	    WRITE_MEM32(DOT1XMACCR, (READ_MEM32(DOT1XMACCR)&(~(0x1<<portnum))));

    return 0;
            
}

int32  rtl_setPortbase802dot1xDir(int32 value, int32 portnum)
{

	if (portnum > RTL8651_PORT_NUMBER || portnum < 0)
	{
		printk("%s %d port number invalid\n", __FUNCTION__, __LINE__);
		return FAILED;
	}
	
	if (dot1x_config.enable_unicastresp)
	{
		/* 
		  *If enable authenticator response EAP packet's destination MAC address as unicast, auth direction 's value must be 1(IN direction only) ,
		  *otherwise, cannot send unicast packets out in unauthorized port or MAC address.
		  */
		WRITE_MEM32(DOT1XPORTCR, (READ_MEM32(DOT1XPORTCR)|(0x1<<((portnum*3) +2))));
	}
	else
	{
		if (value)
			WRITE_MEM32(DOT1XPORTCR, (READ_MEM32(DOT1XPORTCR)|(0x1<<((portnum*3) +2))));
		else
			WRITE_MEM32(DOT1XPORTCR, (READ_MEM32(DOT1XPORTCR)&(~(0x1<<((portnum*3)+2)))));
	}
    
    return 0;
            
}

int32  rtl_setMacbase802dot1xDir(int32 value, int32 portnum)
{
    #if 0
	if (portnum > RTL8651_PORT_NUMBER || portnum < 0)
	{
		printk("%s %d port number invalid\n", __FUNCTION__, __LINE__);
		return FAILED;
	}
	#endif
	
	if (dot1x_config.enable_unicastresp)
	{
		/* if enable authenticater respones eap packets dmac as unicast, auth_dir's value must be 1(IN direction only) 
		  * otherwise cannot send unicast packets out in unauth port/mac
		  */
		WRITE_MEM32(DOT1XMACCR, (READ_MEM32(DOT1XMACCR)|(Dot1xMAC_OPDIR)));
	}
	else
	{
		if (value)
			WRITE_MEM32(DOT1XMACCR, (READ_MEM32(DOT1XMACCR)|(Dot1xMAC_OPDIR)));
		else
			WRITE_MEM32(DOT1XMACCR, (READ_MEM32(DOT1XMACCR)&~(Dot1xMAC_OPDIR)));
	}

    return 0;
            
}

void  rtl_trapDot1xFrameToCpu(int enabletrap)
{
       
    /* disable/enable trap 802dot1x frame to cpu */
	if (enabletrap)
    	WRITE_MEM32(RMACR, (READ_MEM32(RMACR)|MADDR03));
	else
    	WRITE_MEM32(RMACR, (READ_MEM32(RMACR)&(~MADDR03)));
    
    return;
            
}

#if 0
int32  enable_portbase_802dot1x(void)
{
    int i;
    
    /* clear related register */
    WRITE_MEM32(DOT1XPORTCR, 0x00);
    WRITE_MEM32(DOT1XMACCR, 0x00);
    WRITE_MEM32(GVGCR, 0x00);
    /* reinit l2 table?? */
#ifdef CONFIG_RTL_LAYERED_DRIVER_L2
    rtl865x_layer2_reinit();
#endif
    memset(&dot1xPortBasedInfo, 0x00, RTL8651_PORT_NUMBER*sizeof(rtl802Dot1xPortBasedInfo));

    /* enable each Port-based Access Control and in a state of  unauthorized */
    for(i = 0; i < RTL8651_PORT_NUMBER; i++)
    {
        WRITE_MEM32(DOT1XPORTCR, (READ_MEM32(DOT1XPORTCR)|(0x1<<(i*3))));
        dot1xPortBasedInfo[i].portAuthEnable = 1;
		init_timer(&dot1xPortBasedInfo[i].expire_timer);
		dot1xPortBasedInfo[i].expire_timer.data = (unsigned long)&dot1xPortBasedInfo[i];	
		dot1xPortBasedInfo[i].expire_timer.function = rtl_802dot1x_portbase_timeout;		/* expire function	*/
        dot1xPortBasedInfo[i].expire_timer.expires = jiffies + DOT1X_AUTH_TIME_OUT;
        add_timer(&dot1xPortBasedInfo[i].expire_timer);
        
    }
    /* only trap 802dot1x frame to cpu */
    WRITE_MEM32(RMACR, (READ_MEM32(RMACR)|MADDR03));
    
    return 0;
            
}

int32  enable_macbase_802dot1x(void)
{
    
    int i;

    /* clear hw register */
    WRITE_MEM32(DOT1XPORTCR, 0x00);
    WRITE_MEM32(DOT1XMACCR, 0x00);
    WRITE_MEM32(GVGCR, 0x00);
    /* reinit l2 table?? */
#ifdef CONFIG_RTL_LAYERED_DRIVER_L2
	rtl865x_layer2_reinit();
#endif

    /* enable MAC Address Based Control on each port and all mac addresses in a state of unauthorized */
    for(i = 0; i < 5; i++)
    {
        WRITE_MEM32(DOT1XMACCR, (READ_MEM32(DOT1XMACCR)|(0x1<<i)));
    }
    /* only trap 802dot1x frame to cpu */
    WRITE_MEM32(RMACR, (READ_MEM32(RMACR)|MADDR03));

    return 0;
            
}
#endif

static int32 write_proc_8021x_type( struct file *filp, const char *buff,unsigned long len, void *data )
{
	char 		tmpbuf[32];

	if (buff && !copy_from_user(tmpbuf, buff, len))
	{
		tmpbuf[len] = '\0';
		if(tmpbuf[0] == '1')
		{
		    /* enable port-based 802dot1x */
			dot1x_config.type = DOT1X_AUTH_TYPE_PORT_BASED;
            //enable_portbase_802dot1x();

		}
		else if(tmpbuf[0] == '2')
		{
            /* enable mac-based 802dot1x  */
			dot1x_config.type = DOT1X_AUTH_TYPE_MAC_BASED;
            //enable_macbase_802dot1x();
		}
		else
		{
			printk("current support: 1/2\n");
			return len;
		}
    

	}
	return len;
}

static int32 write_proc_8021x_enable( struct file *filp, const char *buff,unsigned long len, void *data )
{
	char 		tmpbuf[32];
	int num, enable_1x, enable_unicast;

	if (buff && !copy_from_user(tmpbuf, buff, len))
	{
	
		num=sscanf(tmpbuf,"%d %d",&enable_1x, &enable_unicast);
		
		if(num!=2){
			printk("set 801dot1x enable parameter error !!!\n");
			return len;
		}
		dot1x_config.enable_unicastresp = enable_unicast & 0xff;
		
		if (enable_1x == 1)
		{
			dot1x_config.enable = 1;
			rtl_trapDot1xFrameToCpu(1);
		}
		else if (!enable_1x)
		{
			dot1x_config.enable = 0;
            rtl_disable802dot1x();
			rtl_trapDot1xFrameToCpu(0);
		}
		else
		{
			printk("current support: 0/1\n");
			return len;
		}
		#if 0
		tmpbuf[len] = '\0';
		if(tmpbuf[0] == '0')
		{
			dot1x_config.enable = 0;
            rtl_disable802dot1x();
			rtl_trapDot1xFrameToCpu(0);
		}
		else if(tmpbuf[0] == '1')
		{
			dot1x_config.enable = 1;
			rtl_trapDot1xFrameToCpu(1);
		}
		else
		{
			printk("current support: 0/1\n");
			return len;
		}
		#endif
    

	}
	return len;
}

int rtl_initDot1xtable(int dot1x_tbl_list_max, int dot1x_tbl_entry_max)
{
	int i;

	table_dot1x = (rtl802Dot1xCacheTable *)kmalloc(sizeof(rtl802Dot1xCacheTable), GFP_ATOMIC);
	if (table_dot1x == NULL) {
		printk("MALLOC Failed! (dot1x Table) \n");
		return -1;
	}
	CTAILQ_INIT(&dot1x_list_inuse);
	CTAILQ_INIT(&dot1x_list_free);

	dot1x_table_list_max=dot1x_tbl_list_max;
	table_dot1x->list=(struct __rtl802Dot1xCacheListEntryHead *)kmalloc(dot1x_tbl_list_max*sizeof(struct __rtl802Dot1xCacheListEntryHead), GFP_ATOMIC);
	if (table_dot1x->list == NULL) {
		printk("MALLOC Failed! (dot1x Table list) \n");
		return -1;
	}
	for (i=0; i<dot1x_tbl_list_max; i++) {
		CTAILQ_INIT(&table_dot1x->list[i]);
	}

	/* entry -List Init */
	for (i=0; i<dot1x_tbl_entry_max; i++) {
		rtl802Dot1xCacheEntry *entry = (rtl802Dot1xCacheEntry *)kmalloc(sizeof(rtl802Dot1xCacheEntry), GFP_ATOMIC);
		if (entry == NULL) {
			printk("MALLOC Failed! (dot1x Table Entry) \n");
			return -2;
		}
		CTAILQ_INSERT_TAIL(&dot1x_list_free, entry, tqe_link);
	}

	return 0;
}

int rtl_init802dot1x(void)
{   
    struct proc_dir_entry *root_8021x;
	#ifndef CONFIG_RTL_PROC_NEW
    struct proc_dir_entry *res_stats;
	#endif
	int ret;

    memset(&dot1x_config, 0x00, sizeof(rtl802Dot1xConfig)); //clear sw
	/* init queue */
	rtl_82dot1xInitQueue(&dot1x_queue);	
    rtl_disable802dot1x();//clear hw

	
	/* dot1x Table Init */
	ret=rtl_initDot1xtable(DOT1X_TABLE_LIST_MAX,DOT1X_TABLE_ENTRY_MAX);
	if(ret!=0) {
		printk("rtl_initDot1xtable Failed!\n");
	}
	
#if defined(CONFIG_RTL_PROC_NEW)
	root_8021x = proc_mkdir("802dot1x", &proc_root);
#else
    root_8021x = proc_mkdir("802dot1x", NULL);
#endif
    if (root_8021x == NULL)
    {
        printk("proc_mkdir 8021x failed!\n");
    }
	
#if defined(CONFIG_RTL_PROC_NEW)
	proc_create_data("enable",0644,root_8021x,&dot1x_enable_proc_fops,NULL);

	proc_create_data("type",0644,root_8021x,&dot1x_type_proc_fops,NULL);
	
	proc_create_data("mode",0644,root_8021x,&dot1x_mode_proc_fops,NULL);

	proc_create_data("server_port",0644,root_8021x,&dot1x_server_port_proc_fops,NULL);

	proc_create_data("entry",0644,root_8021x,&dot1x_entry_proc_fops,NULL);

#else
    if ((res_stats = create_proc_read_entry("enable", 0644, root_8021x,
        read_proc_8021x_enable, NULL)) == NULL)
    {
        printk("create_proc_read_entry failed!\n");
    }
    res_stats->write_proc = write_proc_8021x_enable;

    if ((res_stats = create_proc_read_entry("type", 0644, root_8021x,
        read_proc_8021x_type, NULL)) == NULL)
    {
        printk("create_proc_read_entry failed!\n");
    }
    res_stats->write_proc = write_proc_8021x_type;

    if ((res_stats = create_proc_read_entry("mode", 0644, root_8021x,
        read_proc_8021x_auth_port_mode, NULL)) == NULL)
    {
        printk("create_proc_read_entry failed!\n");
    }
    res_stats->write_proc = write_proc_8021x_auth_port_mode;

    
    if ((res_stats = create_proc_read_entry("server_port", 0644, root_8021x,
        read_proc_8021x_auth_server_port, NULL)) == NULL)
    {
        printk("create_proc_read_entry failed!\n");
    }
    res_stats->write_proc = write_proc_8021x_auth_server_port;
	#if 1
    if ((res_stats = create_proc_read_entry("entry", 0644, root_8021x,
        read_proc_8021x_entry, NULL)) == NULL)
    {
        printk("create_proc_read_entry failed!\n");
    }
    #endif
#endif
    #if 0
    if ((res_stats = create_proc_read_entry("rtl_8021x_port_based_ctrl", 0644, root_8021x,
        read_proc_8021x_port_based_ctrl, NULL)) == NULL)
    {
        printk("create_proc_read_entry failed!\n");
    }
    res_stats->write_proc = write_proc_8021x_port_based_ctrl;


    if ((res_stats = create_proc_read_entry("rtl_8021x_mac_based_ctrl", 0644, root_8021x,
        read_proc_8021x_mac_based_ctrl, NULL)) == NULL)
    {
        printk("create_proc_read_entry failed!\n");
    }
    res_stats->write_proc = write_proc_8021x_mac_based_ctrl;

    
    if ((res_stats = create_proc_read_entry("rtl_8021x_l2_mac_entry", 0644, root_8021x,
        read_proc_8021x_l2_mac_entry, NULL)) == NULL)
    {
        printk("create_proc_read_entry failed!\n");
    }
    res_stats->write_proc = write_proc_8021x_l2_mac_entry;
    #endif
    
    return 0;    
}

int rtl_is802dot1xFrame(uint8 *macFrame)
{
    
	uint8 *ptr;

	ptr = macFrame + 12;
	if((*(int16 *)(ptr)) == (int16)htons(ETH_P_8021Q))
	{
	    /* skip vlan header */
		ptr=ptr+4;
	}

	/* is 802dot1x frame? */
	if((*(int16 *)(ptr)) == (int16)htons(ETH_P_PAE))
	{
		return 1;
	}
    else
    {
        return 0;
    }

}

inline int rtl_parse802dot1xFrame(uint8 *macFrame, uint32 *id)
{
    
	uint8 *ptr, *ptype, *pcode, *pid;

	ptr = macFrame + 12;
	if((*(int16 *)(ptr)) == (int16)htons(ETH_P_8021Q))
	{
	    /* skip vlan header */
		ptr=ptr+4;
	}

	/* is 802dot1x frame? */
	if((*(int16 *)(ptr)) == (int16)htons(ETH_P_PAE))
	{
	    ptype = ptr + DOT1X_EAPOL_PACKET_TYPE_OFFSET;//skip protocol version
	    if (*ptype == DOT1X_EAPOL_TYPE_START)
        {
            return DOT1X_EAPOL_TYPE_START;
        }
        else if (*ptype == DOT1X_EAPOL_TYPE_LOGOFF)
        {
            return DOT1X_EAPOL_TYPE_LOGOFF;
        }
        else if (*ptype == DOT1X_EAPOL_TYPE_EAP_PACKET)
        {
            pcode = ptr + DOT1X_EAPOL_PACKET_CODE_OFFSET;
			pid = ptr + DOT1X_EAPOL_PACKET_ID_OFFSET;
			*id = *pid;
            if (*pcode == DOT1X_EAPOL_PACKET_CODE_REQUEST)
            {
                return DOT1X_EAPOL_TYPE_EAP_REQUEST;
            }
            else if (*pcode == DOT1X_EAPOL_PACKET_CODE_RESPONSE)
            {
                return DOT1X_EAPOL_TYPE_EAP_RESPONSE;
            }
            else if (*pcode == DOT1X_EAPOL_PACKET_CODE_SUCCESS)
            {
                return DOT1X_EAPOL_TYPE_EAP_SUCCESS;
            }
            else if (*pcode == DOT1X_EAPOL_PACKET_CODE_FAILURE)
            {
                return DOT1X_EAPOL_TYPE_EAP_FAILURE;
            }
        }
	    
	}

    return FAILED;


}
#if 0
inline int rtl_82dot1xFindPortNumByMac(uint8 *mac)
{
    int i;

    if (mac == NULL)
        return FAILED;
    
    for (i = 0; i < RTL8651_PORT_NUMBER; i++)
    {
       if (!memcmp(dot1xPortBasedInfo[i].macaddr, mac, ETH_ALEN))
           return dot1xPortBasedInfo[i].portNum;
    }

    return FAILED;
    
}
#endif

static inline void rtl_802dot1xRelayTrapped(struct sk_buff *skb,rtl_nicRx_info *info, uint32 id)
{
    rtl_nicTx_info  nic_tx;
    int vid=info->vid;
    int pid=info->pid;
	uint32 forward_portmsk = (~(1<<pid)) & ((1<<RTL8651_MAC_NUMBER)-1) ;
	rtl802Dot1xCacheEntry *entry;

    //unicast relay
    if((skb->data[0] &0x01) ==0)
    {
        rtl_hwLookup_txInfo(&nic_tx);
    }
    else//multicast/broadcast relay
    {
        if (pid != dot1x_config.server_port)
        {
            //rx from client,tx to server port msk
			rtl_direct_txInfo((1<<dot1x_config.server_port), &nic_tx);
			#if defined(DOT1X_DEBUG)
			panic_printk("%s %d forward_portmsk=0x%x (1<<dot1x_config.server_port)=0x%x pid=%d id=%u\n", __FUNCTION__, __LINE__, forward_portmsk, (1<<dot1x_config.server_port),pid, id);
			#endif
        }
		else
		{
			#if 0
			//rx from server,flooding to client
			rtl_direct_txInfo(forward_portmsk, &nic_tx);
			panic_printk("%s %d forward_portmsk=0x%x (1<<dot1x_config.server_port)=0x%x pid=%d\n", __FUNCTION__, __LINE__, forward_portmsk, (1<<dot1x_config.server_port),pid);
			#else
			//rx from server,tx to related port
			entry = rtl_82dot1xFindEntryById(id-1);
			if (entry)
			{
				//normal case .....???????
				forward_portmsk = (1<<entry->rx_port_num);
				#if defined(DOT1X_DEBUG)
				panic_printk("%s %d entry->rx_port_num=%d pid=%d id=%u\n", __FUNCTION__, __LINE__, entry->rx_port_num,pid, id);
				#endif
			}
			rtl_direct_txInfo(forward_portmsk, &nic_tx);
			#if defined(DOT1X_DEBUG)
			panic_printk("%s %d forward_portmsk=0x%x (1<<dot1x_config.server_port)=0x%x pid=%d id=%u\n", __FUNCTION__, __LINE__, forward_portmsk, (1<<dot1x_config.server_port),pid, id);
			#endif
			#endif
		}
    }
        
    /////vlan process????            
   if(skb!=NULL)
   {
        nic_tx.txIdx=0;

       
        nic_tx.vid = vid;
        //nicTx.srcExtPort = 0;
        //nicTx.flags = (PKTHDR_USED|PKT_OUTGOING);


        _dma_cache_wback_inv((unsigned long)skb->data, skb->len);
		//panic_printk("%s %d vid=%d srcmac=0x%x:%x:%x:%x:%x:%x forward_portmsk=0x%x\n", __FUNCTION__, __LINE__,vid, 
			//skb->data[6], skb->data[7], skb->data[8],skb->data[9], skb->data[10], skb->data[11], forward_portmsk);
        if (RTL_swNic_send((void *)skb, skb->data, skb->len, &nic_tx) < 0)
        {
			printk("%s %d nic send error!!!!\n", __FUNCTION__, __LINE__);
            dev_kfree_skb_any(skb);
        }

    }
    return;
}


int  rtl_802dot1xSnoopingModeRxHandle(struct sk_buff *skb,rtl_nicRx_info *info)
{
	int pid=info->pid;
    int pkttype = -1;
	uint32 id = 0;
	uint8 supplicant_mac[ETH_ALEN] = {0};
	uint8 portnum = 0;

	#if 0
	if ((pid != dot1x_config.server_port) && (!dot1x_config.mode[pid].port_enable))
		return SUCCESS;
	#endif
	
    pkttype = rtl_parse802dot1xFrame(skb->data, &id);
	#if defined(DOT1X_DEBUG)
	panic_printk("%s %d pkttype=%d pid=%d id=%d srcmac=0x%x:%x:%x:%x:%x:%x\n", __FUNCTION__, __LINE__, pkttype, pid, id,
		skb->data[6], skb->data[7], skb->data[8],skb->data[9], skb->data[10], skb->data[11]);
	#endif
    if (pkttype == FAILED)
        return FAILED;
	
	if ((pkttype == DOT1X_EAPOL_TYPE_START) || (pkttype == DOT1X_EAPOL_TYPE_EAP_RESPONSE))
	{
		//add supplicant's mac to cache
		if (rtl_82dot1xAddEntry(&skb->data[ETH_ALEN], id, pid, DOT1X_AUTH_STATE_START)!=SUCCESS)
		{
			#if defined(DOT1X_DEBUG)
			panic_printk("%s %d rtl_82dot1xAddEntry error!!!\n", __FUNCTION__, __LINE__);
			#endif
		}
	}
    else if ((pkttype == DOT1X_EAPOL_TYPE_EAP_SUCCESS)&&
		    (dot1x_config.server_port == pid))
	{
		//rx from server port and authentication success
		if (rtl_82dot1xModifyEntryById(id, DOT1X_AUTH_STATE_SUCCESS,supplicant_mac,&portnum)==SUCCESS)
		{
			if (dot1x_config.type == DOT1X_AUTH_TYPE_PORT_BASED)
			{
				rtl_802dot1xSetPortAuthState(portnum, 1);
			}
			else if (dot1x_config.type == DOT1X_AUTH_TYPE_MAC_BASED)
			{
	            rtl865x_updateAuthState(1, supplicant_mac);
			}	
		}
		else
			printk("%s %d rtl_82dot1xModifyEntryById error!!!\n", __FUNCTION__, __LINE__);
	}
	else if (pkttype == DOT1X_EAPOL_TYPE_LOGOFF)
	{		
		if (rtl_82dot1xdelEntryByMac(&skb->data[ETH_ALEN]) == SUCCESS)
		{
			
			if (dot1x_config.type == DOT1X_AUTH_TYPE_PORT_BASED)
			{
				rtl_802dot1xSetPortAuthState(portnum, 0);
			}
			else if (dot1x_config.type == DOT1X_AUTH_TYPE_MAC_BASED)
			{
	            rtl865x_updateAuthState(0, supplicant_mac);
			}
		}
		else
			printk("%s %d rtl_82dot1xdelEntryByMac error!!!\n", __FUNCTION__, __LINE__);
	}
	#if 0
	else if ((dot1x_config.server_port != pid))
	{
		//update id and rx port number
		if (rtl_82dot1xModifyEntryByMac(&skb->data[ETH_ALEN], id, pid, DOT1X_AUTH_STATE_PROCESSING)!=SUCCESS)
		{
			panic_printk("%s %d rtl_82dot1xModifyEntryByMac error!!!\n", __FUNCTION__, __LINE__);
		}
	}
	#endif

    //tx process???
     rtl_802dot1xRelayTrapped(skb, info, id);
	return DOT1X_CONSUME_PKT;
    
    
}


int  rtl_802dot1xProxyModeRxHandle(struct sk_buff *skb,rtl_nicRx_info *info)
{
	int ret;
	static rtl802Dot1xEapPkt eap;
	if (skb == NULL || info == NULL)
		return FAILED;
	
	memset(&eap, 0x00, sizeof(rtl802Dot1xEapPkt));
	eap.event_id = DOT1X_EVENT_EAP_PACKET;
	eap.rx_port_num = info->pid;
	eap.item_size = ((skb->len > DOT1X_MAX_EAP_PACKET_LEN)? DOT1X_MAX_EAP_PACKET_LEN:skb->len);
	memcpy(&eap.item[0], skb->data, eap.item_size);
	ret = rtl_82dot1xEnQueue((unsigned long)info->priv, &dot1x_queue,
		(unsigned char*)&eap, sizeof(rtl802Dot1xEapPkt));
	if (ret == 0)
	{
		rtl_82dot1xEventInform(1);

		dev_kfree_skb_any(skb);
		return DOT1X_CONSUME_PKT;
	}
	else
	{
		printk("%s %d dot1x enqueue error!", __FUNCTION__, __LINE__);
		return ret;
	}
}

int  rtl_802dot1xClientModeRxHandle(struct sk_buff *skb,rtl_nicRx_info *info)
{
	int ret;
	static rtl802Dot1xEapPkt eap;
	
	if (skb == NULL || info == NULL)
		return FAILED;
	
	memset(&eap, 0x00, sizeof(rtl802Dot1xEapPkt));
	eap.event_id = DOT1X_EVENT_EAP_PACKET;
	eap.rx_port_num = info->pid;
	eap.item_size = ((skb->len > DOT1X_MAX_EAP_PACKET_LEN)? DOT1X_MAX_EAP_PACKET_LEN:skb->len);
	memcpy(&eap.item[0], skb->data, eap.item_size);
	ret = rtl_82dot1xEnQueue((unsigned long)info->priv, &dot1x_queue,
		(unsigned char*)&eap, sizeof(rtl802Dot1xEapPkt));
	if (ret == 0)
	{
		rtl_82dot1xEventInform(1);

		dev_kfree_skb_any(skb);
		return DOT1X_CONSUME_PKT;
	}
	else
	{
		printk("%s %d dot1x enqueue error!", __FUNCTION__, __LINE__);
		return ret;
	}
}


__MIPS16
int  rtl_802dot1xRxHandle(struct sk_buff *skb,rtl_nicRx_info *info)
{
    int ret = 0;

    /* dot1x function disable or not 1x frame just return */
	#if 0
    if ((!dot1x_config.enable)||(rtl_is802dot1xFrame(skb->data) == 0))
    {
        return SUCCESS;
    }
	#endif
    if ((dot1x_config.mode[info->pid].port_enable)&&
		(dot1x_config.mode[info->pid].auth_mode==DOT1X_AUTH_MODE_PROXY))
	{
		ret = rtl_802dot1xProxyModeRxHandle(skb,info);
	}
	else if ((dot1x_config.mode[info->pid].port_enable)&&
		(dot1x_config.mode[info->pid].auth_mode==DOT1X_AUTH_MODE_CLIENT))
	{
		ret = rtl_802dot1xClientModeRxHandle(skb,info);
	}
	else if (((dot1x_config.mode[info->pid].port_enable)&&
		(dot1x_config.mode[info->pid].auth_mode==DOT1X_AUTH_MODE_SNOOPING))||
		(info->pid == dot1x_config.server_port))
	{
		ret = rtl_802dot1xSnoopingModeRxHandle(skb,info);
	}
	#if 0
    switch (dot1x_config.mode[info->pid].auth_mode)
    {
        case DOT1X_AUTH_MODE_SNOOPING:
            ret = rtl_802dot1xSnoopingModeRxHandle(skb,info);
            break;
            
        case DOT1X_AUTH_MODE_PROXY:
            ret = rtl_802dot1xProxyModeRxHandle(skb,info);
            break;
		case DOT1X_AUTH_MODE_CLIENT:
            ret = rtl_802dot1xClientModeRxHandle(skb,info);
            break;

        default:
            printk("%s %d unknown dot1x auth mode!!", __FUNCTION__, __LINE__);
            return SUCCESS;
            
    }
	#endif
    
	return ret;    
}

void  rtl_802dot1xFilltxInfo(struct sk_buff *skb,rtl_nicTx_info *info)
{
	unsigned char *p;
	unsigned int pad = 0;
	int port_offset = 0;
	unsigned int portlist = 0, portnum = 0;
	
	if (!skb || !info)
		return;
	
	port_offset = skb->len - sizeof(unsigned int);
	p = skb->data + port_offset;
	if (!p)
		return;
	memcpy((void *)&pad, (void *)p, sizeof(unsigned int));
	
	if (!((DOT1X_SPECIAL_IDENTITY<<16)&pad))
		return;

	//check port mode
	portnum = (pad&0xffff);
	if (dot1x_config.mode[portnum].auth_mode != DOT1X_AUTH_MODE_PROXY)
		return;
	
	portlist = 1 << portnum;
	rtl_direct_txInfo(portlist, info);
	skb->len -= sizeof(unsigned int);
	#if defined(DOT1X_DEBUG)
	panic_printk("%s %d pad=0x%x portnum=%d portlist=0x%x skb->len=%d\n", __FUNCTION__, __LINE__, pad, portnum, info->portlist, skb->len);
	#endif
	
	return;
	
}

