#include <linux/ethtool.h>
#include <linux/netdevice.h>
#include <linux/switch.h>
#include <net/rtl/rtl_types.h>

#include "AsicDriver/rtl865x_asicCom.h"
#include "AsicDriver/rtl865x_asicL2.h"

#include "ethtool.h"

extern unsigned int rtk_get_port_status(int port, struct switch_port_link *link);

static struct rtl_switch_port_mapping rtl865x_port_mapping[] = {
	{"eth0", 0 },
	{"eth1", 4 },
};

static int get_port_by_name(char *name)
{
	int i;

	for(i = 0; i < ARRAY_SIZE(rtl865x_port_mapping); i++)
		if(!strncmp(rtl865x_port_mapping[i].ifname, name, strlen(name)))
			return rtl865x_port_mapping[i].port_num;

	return -1;
}

static int rtl865x_get_settings(struct net_device *netdev, struct ethtool_cmd *ecmd)
{
	struct switch_port_link spl;
	uint32_t statCtrlReg4;
	uint32_t statCtrlReg9;
	int port;
	int phyid;

	if((port = get_port_by_name(netdev->name)) < 0){
		printk("%s +%d\n: Failed to find %s intrerface switch port!!\n"
		      , __func__, __LINE__, netdev->name);

		return -1;
	}

	ecmd->supported = SUPPORTED_10baseT_Half |
		SUPPORTED_10baseT_Full |
		SUPPORTED_100baseT_Half |
		SUPPORTED_100baseT_Full;

	ecmd->supported |= (rtl8651AsicEthernetTable[port].isGPHY)? SUPPORTED_1000baseT_Full : 0;

	ecmd->supported |= SUPPORTED_Autoneg |
		SUPPORTED_TP |
		SUPPORTED_Pause |
		SUPPORTED_Asym_Pause;

	phyid = rtl8651AsicEthernetTable[port].phyId;
	if(rtl8651_getAsicEthernetPHYReg(phyid, 4, &statCtrlReg4))
		return -1;

	ecmd->advertising = 0;

	if ((statCtrlReg4 >> 8) & 1)
		ecmd->advertising |= ADVERTISED_100baseT_Full;
	if ((statCtrlReg4 >> 7) & 1)
		ecmd->advertising |= ADVERTISED_100baseT_Half;
	if ((statCtrlReg4 >> 6) & 1)
		ecmd->advertising |= ADVERTISED_10baseT_Full;
	if ((statCtrlReg4 >> 5) & 1)
		ecmd->advertising |= ADVERTISED_10baseT_Half;

	if (rtl8651AsicEthernetTable[port].isGPHY){
		if(rtl8651_getAsicEthernetPHYReg( phyid, 9, &statCtrlReg9))
			return -1;

		if ((statCtrlReg9 >> 9) & 1)
			ecmd->advertising |= ADVERTISED_1000baseT_Full;
	}

	ecmd->advertising |= ADVERTISED_TP;
	ecmd->advertising |= ADVERTISED_Pause;
	ecmd->advertising |= ADVERTISED_Asym_Pause;

	rtk_get_port_status(port, &spl);

	ecmd->autoneg = spl.aneg ? AUTONEG_ENABLE : AUTONEG_DISABLE;
	ecmd->advertising |= spl.aneg ? ADVERTISED_Autoneg : 0;

	ecmd->transceiver = XCVR_INTERNAL;
	ecmd->port = PORT_TP;
	ecmd->phy_address = phyid;

	if(spl.link){
		ethtool_cmd_speed_set(ecmd, spl.speed);
		ecmd->duplex = spl.duplex;
	}
	else{
		ethtool_cmd_speed_set(ecmd, SPEED_UNKNOWN);
		ecmd->duplex = DUPLEX_UNKNOWN;
	}

	return 0;
}

static int rtl865x_set_settings(struct net_device *netdev, struct ethtool_cmd *ecmd)
{
	uint32_t statCtrlReg4;
	uint32_t statCtrlReg9;
	int link_speed = 0;
	int port;
	int phyid;

	if((port = get_port_by_name(netdev->name)) < 0){
		printk("%s +%d\n: Failed to find %s intrerface switch port!!\n"
		      , __func__, __LINE__, netdev->name);

		return -1;
	}

	phyid = rtl8651AsicEthernetTable[port].phyId;

	if(rtl8651_getAsicEthernetPHYReg( phyid, 4, &statCtrlReg4))
		return -1;

	if(rtl8651_getAsicEthernetPHYReg( phyid, 9, &statCtrlReg9))
		return -1;

	statCtrlReg4 &= ~(0xF << 5);
	statCtrlReg9 &= ~(0x1 << 9);

	if (ecmd->advertising & ADVERTISED_100baseT_Full)
		statCtrlReg4 |=  1 << 8;
	if (ecmd->advertising & ADVERTISED_100baseT_Half)
		statCtrlReg4 |=  1 << 7;
	if (ecmd->advertising & ADVERTISED_10baseT_Full)
		statCtrlReg4 |=  1 << 6;
	if (ecmd->advertising & ADVERTISED_10baseT_Half)
		statCtrlReg4 |=  1 << 5;

	if (ecmd->advertising & ADVERTISED_1000baseT_Full)
		statCtrlReg9 |=  1 << 9;

	rtl8651_setAsicEthernetPHYReg(phyid, 4, statCtrlReg4);
	rtl8651_setAsicEthernetPHYReg(phyid, 9, statCtrlReg9);

	rtl8651_setAsicEthernetPHYAutoNeg(port, ecmd->autoneg);

	switch(ecmd->speed){
	case 10:
		link_speed=0;
		break;
	case 100:
		link_speed=1;
		break;
	case 1000:
		link_speed=2;
		break;
	}

	rtl865xC_setAsicEthernetForceModeRegs(port, !(ecmd->autoneg), 1, link_speed, ecmd->duplex);
	rtl8651_setAsicEthernetPHYSpeed(port, link_speed);
	rtl8651_setAsicEthernetPHYDuplex(port, ecmd->duplex);
	rtl8651_setAsicEthernetPHYAutoNeg(port, ecmd->autoneg);
	rtl8651_restartAsicEthernetPHYNway(port);

	return 0;
}

const struct ethtool_ops rtl865x_ethtool_ops = {
	.get_settings		= rtl865x_get_settings,
	.set_settings		= rtl865x_set_settings,
//	.get_pauseparam		= rtl865x_get_pauseparam,
//	.set_pauseparam		= rtl865x_set_pauseparam,
//	.get_msglevel		= rtl865x_get_msglevel,
//	.set_msglevel		= rtl865x_set_msglevel,
	.get_link		= ethtool_op_get_link,
//	.get_strings		= rtl865x_get_strings,
//	.get_sset_count		= rtl865x_get_sset_count,
//	.get_ethtool_stats	= rtl865x_get_ethtool_stats,
};
