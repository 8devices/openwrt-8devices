#include <linux/kernel.h>
#include <linux/device.h>
#include <linux/switch.h>
#include "rtl819x_switch.h"

struct rtl819x_smi {
	struct switch_dev	sw_dev;

	unsigned int		cpu_port;
	unsigned int		num_ports;
	unsigned int		num_vlan_mc;
	unsigned int		num_mib_counters;
	struct rtl819x_mib_counter *mib_counters;

	//struct rtl8366_smi_ops	*ops;
	int			vlan_enabled;
	int			vlan4k_enabled;
	int			learn_enabled;
	char			buf[4096];
};

static struct rtl819x_smi rtl819x_smi_driver;

//############## call driver layer


//extern from rtl_nic.c
extern void rtk_reset_rtl819x_setting(void) ;
extern void rtk_set_vlan_enable(int enable);
//extern unsigned int rtk_get_port_status(int port , char *buf, int buf_len);
unsigned int rtk_get_port_status(int port , struct switch_port_link *link);
extern void rtk_reset_mibCounter(void);
extern char *rtk_get_mibCounter_name(int mib_idx);
extern unsigned  int rtk_get_mibCounter_val(int port,int mib_idx);
extern int rtk_get_vlan_entry(u32 vid,struct rtl819x_vlan_4k *vlan4k);
extern int rtk_set_port_pvid(int port, unsigned int pvid);
extern int rtk_set_hw_vlan( unsigned int  vid, unsigned int  member, unsigned int  untag, unsigned int  fid);
extern  int rtk_get_port_pvid( int port, int *val);
extern int rtk_get_mibCounter_num(void);


static void rtk_reset_layer2_setting(void)
{
	rtk_reset_rtl819x_setting();
} 
	
//###############
static inline struct rtl819x_smi *sw_to_rtl819x_smi(struct switch_dev *sw)
{
	return container_of(sw, struct rtl819x_smi, sw_dev);
}


static int rtl819x_sw_get_learning_enable(struct switch_dev *dev,
					   const struct switch_attr *attr,
					   struct switch_val *val)
{
	struct rtl819x_smi *smi = sw_to_rtl819x_smi(dev);	

	//printk(" Not support swconfig get_learning_enable yet! ----- \n");	

	val->value.i = smi->learn_enabled;

	return 0;
}


static int rtl819x_sw_set_learning_enable(struct switch_dev *dev,
					   const struct switch_attr *attr,
					   struct switch_val *val)
{
	printk(" Not support swconfig rtl819x_sw_set_learning_enable yet! ----- \n");	
#if 0
	struct rtl8366_smi *smi = sw_to_rtl8366_smi(dev);
	u32 portmask = 0;
	int err = 0;

	if (!val->value.i)
		portmask = RTL8366S_PORT_ALL;

	/* set learning for all ports */
	REG_WR(smi, RTL8366S_SSCR0, portmask);

	/* set auto ageing for all ports */
	REG_WR(smi, RTL8366S_SSCR1, portmask);
#endif
	return 0;
}

int rtl819x_sw_set_vlan_enable(struct switch_dev *dev,
			       const struct switch_attr *attr,
			       struct switch_val *val)
{
	struct rtl819x_smi *smi = sw_to_rtl819x_smi(dev);

	//printk("rtl819x_sw_set_vlan_enable ----- \n");
	
	smi->vlan_enabled = val->value.i;
	smi->vlan4k_enabled = val->value.i;	

	rtk_set_vlan_enable(smi->vlan_enabled);
	
	return 0;
}

int rtl819x_sw_get_vlan_enable(struct switch_dev *dev,
			       const struct switch_attr *attr,
			       struct switch_val *val)
{
	struct rtl819x_smi *smi = sw_to_rtl819x_smi(dev);

	//printk("rtl819x_sw_get_vlan_enable ----- \n");
	if (attr->ofs > 2)
		return -EINVAL;

	if (attr->ofs == 1)
		val->value.i = smi->vlan_enabled;
	else
		val->value.i = smi->vlan4k_enabled;

	return 0;
}


static struct switch_attr rtl819x_globals[] = {
	{
		.type = SWITCH_TYPE_INT,
		.name = "enable_learning",
		.description = "Enable learning, enable aging",
		.set = rtl819x_sw_set_learning_enable,
		.get = rtl819x_sw_get_learning_enable,
		.max = 1,
	}, {
		.type = SWITCH_TYPE_INT,
		.name = "enable_vlan",
		.description = "Enable VLAN mode",
		.set = rtl819x_sw_set_vlan_enable,
		.get = rtl819x_sw_get_vlan_enable,
		.max = 1,
		.ofs = 1
	}, {
		.type = SWITCH_TYPE_INT,
		.name = "enable_vlan4k",
		.description = "Enable VLAN 4K mode",
		.set = rtl819x_sw_set_vlan_enable,
		.get = rtl819x_sw_get_vlan_enable,
		.max = 1,
		.ofs = 2
	}, 
#if 0
	{
		.type = SWITCH_TYPE_NOVAL,
		.name = "reset_mibs",
		.description = "Reset all MIB counters",
		.set = rtl8366s_sw_reset_mibs,
	}, {
		.type = SWITCH_TYPE_INT,
		.name = "blinkrate",
		.description = "Get/Set LED blinking rate (0 = 43ms, 1 = 84ms,"
		" 2 = 120ms, 3 = 170ms, 4 = 340ms, 5 = 670ms)",
		.set = rtl8366s_sw_set_blinkrate,
		.get = rtl8366s_sw_get_blinkrate,
		.max = 5
	},
#endif	
};
#if 0 //mark_bb
static int rtl819x_sw_get_port_link(struct switch_dev *dev,
				     const struct switch_attr *attr,
				     struct switch_val *val)
{	
	struct rtl819x_smi *smi = sw_to_rtl819x_smi(dev);	
	int len = 0 ;
	int port=0;	
	
	if (val->port_vlan >= RTL819X_NUM_PORTS)
		return -EINVAL;
	port= val->port_vlan;

	memset(smi->buf, '\0', sizeof(smi->buf));

	len = rtk_get_port_status(port,smi->buf,sizeof(smi->buf));	
	
	val->value.s = smi->buf;
	val->len = len;	
	
	return 0;
}
#endif
static int rtl819x_sw_get_port_link(struct switch_dev *dev,
                                     int port,
                                     struct switch_port_link *link)
{
	//struct rtl819x_smi *smi = sw_to_rtl819x_smi(dev);	

	if (port >= RTL819X_NUM_PORTS)
		return -EINVAL;

	return rtk_get_port_status(port,link);
}
static int rtl819x_sw_reset_port_mibs(struct switch_dev *dev,
				       const struct switch_attr *attr,
				       struct switch_val *val)
{
	//printk("rtl819x_sw_reset_port_mibs ----- \n");
	rtk_reset_mibCounter();
	return 0;
}

int rtl819x_sw_get_port_mib(struct switch_dev *dev,
			    const struct switch_attr *attr,
			    struct switch_val *val)
{
	struct rtl819x_smi *smi = sw_to_rtl819x_smi(dev);
	int i, len = 0;
	//unsigned long long counter = 0;
	char *buf = smi->buf;

	//printk("rtl819x_sw_get_port_mib ----- \n");
	
	if (val->port_vlan >= smi->num_ports)
		return -EINVAL;

	len += snprintf(buf + len, sizeof(smi->buf) - len,
			"Port %d MIB counters\n",
			val->port_vlan);

	for (i = 0; i < smi->num_mib_counters; ++i) {
		len += snprintf(buf + len, sizeof(smi->buf) - len,
				"%-36s: ",(char *)rtk_get_mibCounter_name(i));

			len += snprintf(buf + len, sizeof(smi->buf) - len,
					"%d\n", (unsigned int)rtk_get_mibCounter_val(val->port_vlan,i));
	}

	val->value.s = buf;
	val->len = len;

	return 0;
}

static struct switch_attr rtl819x_port[] = {
#if 0 //mark_bb , move to main ops in latest swconfig ops	
	{
		.type = SWITCH_TYPE_STRING,
		.name = "link",
		.description = "Get port link information",
		.max = 1,
		.set = NULL,
		.get = rtl819x_sw_get_port_link,
	}, 
#endif	
	{
		.type = SWITCH_TYPE_NOVAL,
		.name = "reset_mib",
		.description = "Reset single port MIB counters",
		.set = rtl819x_sw_reset_port_mibs,
	}, {
		.type = SWITCH_TYPE_STRING,
		.name = "mib",
		.description = "Get MIB counters for port",
		.max = 33,
		.set = NULL,
		.get = rtl819x_sw_get_port_mib,
	},
#if 0
	{
		.type = SWITCH_TYPE_INT,
		.name = "led",
		.description = "Get/Set port group (0 - 3) led mode (0 - 15)",
		.max = 15,
		.set = rtl8366s_sw_set_port_led,
		.get = rtl8366s_sw_get_port_led,
	},
#endif	
};

static int rtl819x_get_vlan_4k(u32 vid,struct rtl819x_vlan_4k *vlan4k)
{
   int ret=0;
   
   memset(vlan4k, '\0', sizeof(struct rtl819x_vlan_4k));

   if (vid >= RTL819X_NUM_VIDS || vid == 0)
		return -EINVAL;

   ret = rtk_get_vlan_entry(vid,vlan4k);  
	
   return ret;

}

int rtl819x_sw_get_vlan_info(struct switch_dev *dev,
			     const struct switch_attr *attr,
			     struct switch_val *val)
{

	int i;
	u32 len = 0;
	struct rtl819x_vlan_4k vlan4k;
	struct rtl819x_smi *smi = sw_to_rtl819x_smi(dev);
	char *buf = smi->buf;
	int err;

	//if (!smi->ops->is_vlan_valid(smi, val->port_vlan))
		//return -EINVAL;

	memset(buf, '\0', sizeof(smi->buf));

	err=  rtl819x_get_vlan_4k(val->port_vlan, &vlan4k);
	
	if (err)
		return err;

	len += snprintf(buf + len, sizeof(smi->buf) - len,
			"VLAN %d: Ports: '", vlan4k.vid);

	for (i = 0; i < smi->num_ports; i++) {
		if (!(vlan4k.member & (1 << i)))
			continue;

		len += snprintf(buf + len, sizeof(smi->buf) - len, "%d%s", i,
				(vlan4k.untag & (1 << i)) ? "" : "t");
	}

	len += snprintf(buf + len, sizeof(smi->buf) - len,
			"', members=%04x, untag=%04x, fid=%u",
			vlan4k.member, vlan4k.untag, vlan4k.fid);

	val->value.s = buf;
	val->len = len;
	
	return 0;
}


int rtl819x_sw_get_vlan_fid(struct switch_dev *dev,
			    const struct switch_attr *attr,
			    struct switch_val *val)
{
	
	struct rtl819x_vlan_4k vlan4k;
	//struct rtl819x_smi *smi = sw_to_rtl819x_smi(dev);
	int err;

	//if (!smi->ops->is_vlan_valid(smi, val->port_vlan))
		//return -EINVAL;

	err = rtl819x_get_vlan_4k(val->port_vlan, &vlan4k);

	if (err)
		return err;

	val->value.i = vlan4k.fid;

	return 0;
}


int rtl819x_sw_set_vlan_fid(struct switch_dev *dev,
			    const struct switch_attr *attr,
			    struct switch_val *val)
{
	//printk("rtl819x_sw_set_vlan_fid ----- \n");
	return 0;
}

int rtl819x_sw_get_vlan_ports(struct switch_dev *dev, struct switch_val *val)
{

	struct rtl819x_smi *smi = sw_to_rtl819x_smi(dev);
	struct switch_port *port;
	struct rtl819x_vlan_4k vlan4k;
	int i;
	//printk("rtl819x_sw_get_vlan_ports ----- \n");
	
	//if (!smi->ops->is_vlan_valid(smi, val->port_vlan))
	//	return -EINVAL;

	rtl819x_get_vlan_4k(val->port_vlan, &vlan4k);	

	port = &val->value.ports[0];
	val->len = 0;
	for (i = 0; i < smi->num_ports; i++) {
		if (!(vlan4k.member & BIT(i)))
			continue;

		port->id = i;
		port->flags = (vlan4k.untag & BIT(i)) ?
					0 : BIT(SWITCH_PORT_FLAG_TAGGED);
		val->len++;
		port++;
	}

	return 0;
}

int rtl819x_sw_set_vlan_ports(struct switch_dev *dev, struct switch_val *val)
{
	//struct rtl819x_smi *smi = sw_to_rtl819x_smi(dev);
	struct switch_port *port;
	u32 member = 0;
	u32 untag = 0;
	int err;
	int i;

	//printk("rtl819x_sw_set_vlan_ports -----vid=%d \n",val->port_vlan );
	port = &val->value.ports[0];

       for (i = 0; i < val->len; i++, port++) {
                member |= BIT(port->id);

                if (!(port->flags & BIT(SWITCH_PORT_FLAG_TAGGED)))
                {
                        untag |= BIT(port->id);
			//only change port's pvid if it is untag port in this vlan
                        err=rtk_set_port_pvid( port->id, val->port_vlan);
                }

        }

	return rtk_set_hw_vlan(val->port_vlan, member, untag, 0);

}


int rtl819x_sw_get_port_pvid(struct switch_dev *dev, int port, int *val)
{	
	
	rtk_get_port_pvid(port, val);
	//printk("rtl819x_sw_get_port_pvid port%d=%d ----- \n",port,*val);
	
	return 0;
}

int rtl819x_sw_set_port_pvid(struct switch_dev *dev, int port, int val)
{
	int err;
	//printk("rtl819x_sw_set_port_pvid ----- \n");
	err = rtk_set_port_pvid( port, val);
	
 	return err;
}

static int rtl819x_sw_reset_switch(struct switch_dev *dev)
{
	//printk("rtl819x_sw_reset_switch ----- \n");
	rtk_reset_layer2_setting();
      return 0;
}

static struct switch_attr rtl819x_vlan[] = {
	{
		.type = SWITCH_TYPE_STRING,
		.name = "info",
		.description = "Get vlan information",
		.max = 1,
		.set = NULL,
		.get = rtl819x_sw_get_vlan_info,
	}, {
		.type = SWITCH_TYPE_INT,
		.name = "fid",
		.description = "Get/Set vlan FID",
		.max = RTL819X_FIDMAX,
		.set = rtl819x_sw_set_vlan_fid,
		.get = rtl819x_sw_get_vlan_fid,
	},
};

static const struct switch_dev_ops rtl819x_swconfig_ops = {
	.attr_global = {
		.attr = rtl819x_globals,
		.n_attr = ARRAY_SIZE(rtl819x_globals),
	},
	.attr_port = {
		.attr = rtl819x_port,
		.n_attr = ARRAY_SIZE(rtl819x_port),
	},
	.attr_vlan = {
		.attr = rtl819x_vlan,
		.n_attr = ARRAY_SIZE(rtl819x_vlan),
	},

	.get_vlan_ports = rtl819x_sw_get_vlan_ports,
	.set_vlan_ports = rtl819x_sw_set_vlan_ports,
	.get_port_pvid = rtl819x_sw_get_port_pvid,
	.set_port_pvid = rtl819x_sw_set_port_pvid,
	.reset_switch = rtl819x_sw_reset_switch,
	.get_port_link =  rtl819x_sw_get_port_link, //mark_bb
};

static int rtl819x_smi_init(struct rtl819x_smi *rtl819x_smi_driver_p)
{
	memset((char *)rtl819x_smi_driver_p ,0,sizeof(struct rtl819x_smi));

	rtl819x_smi_driver_p->cpu_port = RTL819X_PORT_NUM_CPU;
	rtl819x_smi_driver_p->num_ports = RTL819X_NUM_PORTS;
	//rtl819x_smi_driver_p->num_vlan_mc = RTL8366S_NUM_VLANS;
	//rtl819x_smi_driver_p->mib_counters = rtl819x_mib_counters;
	rtl819x_smi_driver_p->num_mib_counters = rtk_get_mibCounter_num();
	rtl819x_smi_driver_p->learn_enabled= 1; //mark_bb , default say yes 
	return 0;
}

int rtl819x_swconfig_init(struct net_device *netdev)
{
	struct switch_dev *dev ;
	int err;

	rtl819x_smi_init(&rtl819x_smi_driver);
	dev = &rtl819x_smi_driver.sw_dev;

	dev->name = "rtl819x";
	dev->cpu_port = RTL819X_PORT_NUM_CPU;
	dev->ports = RTL819X_NUM_PORTS;
	dev->vlans = RTL819X_NUM_VIDS;
	dev->ops = &rtl819x_swconfig_ops;
	//dev->alias = dev_name(smi->parent);

	//err = register_switch(dev, NULL);
	err = register_switch(dev, netdev);
	if (err)
		printk("switch device registration failed\n");

	return err;
}
