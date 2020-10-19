/*-----------------------------------------------------------------------------+
 *
 *	 This source code has been made available to you by IBM on an AS-IS
 *	 basis.	 Anyone receiving this source is licensed under IBM
 *	 copyrights to use it in any way he or she deems fit, including
 *	 copying it, modifying it, compiling it, and redistributing it either
 *	 with or without modifications.	 No license under IBM patents or
 *	 patent applications is to be implied by the copyright license.
 *
 *	 Any user of this software should understand that IBM cannot provide
 *	 technical support for this software and will not be responsible for
 *	 any consequences resulting from the use of this software.
 *
 *	 Any person who transfers this source code or any derivative work
 *	 must include the IBM copyright notice, this paragraph, and the
 *	 preceding two paragraphs in the transferred software.
 *
 *	 COPYRIGHT   I B M   CORPORATION 1995
 *	 LICENSED MATERIAL  -  PROGRAM PROPERTY OF I B M
 *-----------------------------------------------------------------------------*/
/*-----------------------------------------------------------------------------+
 *
 *  File Name:	enetemac.c
 *
 *  Function:	Device driver for the ethernet EMAC3 macro on the 405GP.
 *
 *  Author:	Mark Wisner
 *
 *  Change Activity-
 *
 *  Date	Description of Change					    BY
 *  ---------	---------------------					    ---
 *  05-May-99	Created							    MKW
 *  27-Jun-99	Clean up						    JWB
 *  16-Jul-99	Added MAL error recovery and better IP packet handling	    MKW
 *  29-Jul-99	Added Full duplex support				    MKW
 *  06-Aug-99	Changed names for Mal CR reg				    MKW
 *  23-Aug-99	Turned off SYE when running at 10Mbs			    MKW
 *  24-Aug-99	Marked descriptor empty after call_xlc			    MKW
 *  07-Sep-99	Set MAL RX buffer size reg to ENET_MAX_MTU_ALIGNED / 16	    MCG
 *		to avoid chaining maximum sized packets. Push starting
 *		RX descriptor address up to the next cache line boundary.
 *  16-Jan-00	Added support for booting with IP of 0x0		    MKW
 *  15-Mar-00	Updated enetInit() to enable broadcast addresses in the
 *		EMAC_RXM register.					    JWB
 *  12-Mar-01	anne-sophie.harnois@nextream.fr
 *		 - Variables are compatible with those already defined in
 *		  include/net.h
 *		- Receive buffer descriptor ring is used to send buffers
 *		  to the user
 *		- Info print about send/received/handled packet number if
 *		  INFO_405_ENET is set
 *  17-Apr-01	stefan.roese@esd-electronics.com
 *		- MAL reset in "eth_halt" included
 *		- Enet speed and duplex output now in one line
 *  08-May-01	stefan.roese@esd-electronics.com
 *		- MAL error handling added (eth_init called again)
 *  13-Nov-01	stefan.roese@esd-electronics.com
 *		- Set IST bit in EMAC_M1 reg upon 100MBit or full duplex
 *  04-Jan-02	stefan.roese@esd-electronics.com
 *		- Wait for PHY auto negotiation to complete added
 *  06-Feb-02	stefan.roese@esd-electronics.com
 *		- Bug fixed in waiting for auto negotiation to complete
 *  26-Feb-02	stefan.roese@esd-electronics.com
 *		- rx and tx buffer descriptors now allocated (no fixed address
 *		  used anymore)
 *  17-Jun-02	stefan.roese@esd-electronics.com
 *		- MAL error debug printf 'M' removed (rx de interrupt may
 *		  occur upon many incoming packets with only 4 rx buffers).
 *-----------------------------------------------------------------------------*
 *  17-Nov-03	travis.sawyer@sandburst.com
 *		- ported from 405gp_enet.c to utilized upto 4 EMAC ports
 *		  in the 440GX.	 This port should work with the 440GP
 *		  (2 EMACs) also
 *  15-Aug-05	sr@denx.de
 *		- merged 405gp_enet.c and 440gx_enet.c to generic 4xx_enet.c
		  now handling all 4xx cpu's.
 *-----------------------------------------------------------------------------*/

#include <config.h>
#include <common.h>
#include <net.h>
#include <asm/processor.h>
#include <commproc.h>
#include <ppc4xx.h>
#include <ppc4xx_enet.h>
#include <405_mal.h>
#include <miiphy.h>
#include <malloc.h>
#include "vecnum.h"

/*
 * Only compile for platform with AMCC EMAC ethernet controller and
 * network support enabled.
 * Remark: CONFIG_405 describes Xilinx PPC405 FPGA without EMAC controller!
 */
#if (CONFIG_COMMANDS & CFG_CMD_NET) && !defined(CONFIG_405) && !defined(CONFIG_IOP480)

#if !(defined(CONFIG_MII) || (CONFIG_COMMANDS & CFG_CMD_MII))
#error "CONFIG_MII has to be defined!"
#endif

#if defined(CONFIG_NETCONSOLE) && !defined(CONFIG_NET_MULTI)
#error "CONFIG_NET_MULTI has to be defined for NetConsole"
#endif

#define EMAC_RESET_TIMEOUT 1000 /* 1000 ms reset timeout */
#define PHY_AUTONEGOTIATE_TIMEOUT 4000	/* 4000 ms autonegotiate timeout */

/* Ethernet Transmit and Receive Buffers */
/* AS.HARNOIS
 * In the same way ENET_MAX_MTU and ENET_MAX_MTU_ALIGNED are set from
 * PKTSIZE and PKTSIZE_ALIGN (include/net.h)
 */
#define ENET_MAX_MTU	       PKTSIZE
#define ENET_MAX_MTU_ALIGNED   PKTSIZE_ALIGN

/*-----------------------------------------------------------------------------+
 * Defines for MAL/EMAC interrupt conditions as reported in the UIC (Universal
 * Interrupt Controller).
 *-----------------------------------------------------------------------------*/
#define MAL_UIC_ERR ( UIC_MAL_SERR | UIC_MAL_TXDE  | UIC_MAL_RXDE)
#define MAL_UIC_DEF  (UIC_MAL_RXEOB | MAL_UIC_ERR)
#define EMAC_UIC_DEF UIC_ENET
#define EMAC_UIC_DEF1 UIC_ENET1
#define SEL_UIC_DEF(p) (p ? UIC_ENET1 : UIC_ENET )

#undef INFO_4XX_ENET

#define BI_PHYMODE_NONE	 0
#define BI_PHYMODE_ZMII	 1
#define BI_PHYMODE_RGMII 2


/*-----------------------------------------------------------------------------+
 * Global variables. TX and RX descriptors and buffers.
 *-----------------------------------------------------------------------------*/
/* IER globals */
static uint32_t mal_ier;

#if !defined(CONFIG_NET_MULTI)
struct eth_device *emac0_dev = NULL;
#endif

/*
 * Get count of EMAC devices (doesn't have to be the max. possible number
 * supported by the cpu)
 */
#if defined(CONFIG_HAS_ETH3)
#define LAST_EMAC_NUM	4
#elif defined(CONFIG_HAS_ETH2)
#define LAST_EMAC_NUM	3
#elif defined(CONFIG_HAS_ETH1)
#define LAST_EMAC_NUM	2
#else
#define LAST_EMAC_NUM	1
#endif

/*-----------------------------------------------------------------------------+
 * Prototypes and externals.
 *-----------------------------------------------------------------------------*/
static void enet_rcv (struct eth_device *dev, unsigned long malisr);

int enetInt (struct eth_device *dev);
static void mal_err (struct eth_device *dev, unsigned long isr,
		     unsigned long uic, unsigned long maldef,
		     unsigned long mal_errr);
static void emac_err (struct eth_device *dev, unsigned long isr);

extern int phy_setup_aneg (char *devname, unsigned char addr);
extern int emac4xx_miiphy_read (char *devname, unsigned char addr,
		unsigned char reg, unsigned short *value);
extern int emac4xx_miiphy_write (char *devname, unsigned char addr,
		unsigned char reg, unsigned short value);

/*-----------------------------------------------------------------------------+
| ppc_4xx_eth_halt
| Disable MAL channel, and EMACn
+-----------------------------------------------------------------------------*/
static void ppc_4xx_eth_halt (struct eth_device *dev)
{
	EMAC_4XX_HW_PST hw_p = dev->priv;
	uint32_t failsafe = 10000;

	out32 (EMAC_IER + hw_p->hw_addr, 0x00000000);	/* disable emac interrupts */

	/* 1st reset MAL channel */
	/* Note: writing a 0 to a channel has no effect */
#if defined(CONFIG_405EP) || defined(CONFIG_440EP) || defined(CONFIG_440GR)
	mtdcr (maltxcarr, (MAL_CR_MMSR >> (hw_p->devnum * 2)));
#else
	mtdcr (maltxcarr, (MAL_CR_MMSR >> hw_p->devnum));
#endif
	mtdcr (malrxcarr, (MAL_CR_MMSR >> hw_p->devnum));

	/* wait for reset */
	while (mfdcr (malrxcasr) & (MAL_CR_MMSR >> hw_p->devnum)) {
		udelay (1000);	/* Delay 1 MS so as not to hammer the register */
		failsafe--;
		if (failsafe == 0)
			break;
	}

	/* EMAC RESET */
	out32 (EMAC_M0 + hw_p->hw_addr, EMAC_M0_SRST);

#ifndef CONFIG_NETCONSOLE
	hw_p->print_speed = 1;	/* print speed message again next time */
#endif

	return;
}

#if defined (CONFIG_440GX)
int ppc_4xx_eth_setup_bridge(int devnum, bd_t * bis)
{
	unsigned long pfc1;
	unsigned long zmiifer;
	unsigned long rmiifer;

	mfsdr(sdr_pfc1, pfc1);
	pfc1 = SDR0_PFC1_EPS_DECODE(pfc1);

	zmiifer = 0;
	rmiifer = 0;

	switch (pfc1) {
	case 1:
		zmiifer |= ZMII_FER_RMII << ZMII_FER_V(0);
		zmiifer |= ZMII_FER_RMII << ZMII_FER_V(1);
		zmiifer |= ZMII_FER_RMII << ZMII_FER_V(2);
		zmiifer |= ZMII_FER_RMII << ZMII_FER_V(3);
		bis->bi_phymode[0] = BI_PHYMODE_ZMII;
		bis->bi_phymode[1] = BI_PHYMODE_ZMII;
		bis->bi_phymode[2] = BI_PHYMODE_ZMII;
		bis->bi_phymode[3] = BI_PHYMODE_ZMII;
		break;
	case 2:
		zmiifer = ZMII_FER_SMII << ZMII_FER_V(0);
		zmiifer = ZMII_FER_SMII << ZMII_FER_V(1);
		zmiifer = ZMII_FER_SMII << ZMII_FER_V(2);
		zmiifer = ZMII_FER_SMII << ZMII_FER_V(3);
		bis->bi_phymode[0] = BI_PHYMODE_ZMII;
		bis->bi_phymode[1] = BI_PHYMODE_ZMII;
		bis->bi_phymode[2] = BI_PHYMODE_ZMII;
		bis->bi_phymode[3] = BI_PHYMODE_ZMII;
		break;
	case 3:
		zmiifer |= ZMII_FER_RMII << ZMII_FER_V(0);
		rmiifer |= RGMII_FER_RGMII << RGMII_FER_V(2);
		bis->bi_phymode[0] = BI_PHYMODE_ZMII;
		bis->bi_phymode[1] = BI_PHYMODE_NONE;
		bis->bi_phymode[2] = BI_PHYMODE_RGMII;
		bis->bi_phymode[3] = BI_PHYMODE_NONE;
		break;
	case 4:
		zmiifer |= ZMII_FER_SMII << ZMII_FER_V(0);
		zmiifer |= ZMII_FER_SMII << ZMII_FER_V(1);
		rmiifer |= RGMII_FER_RGMII << RGMII_FER_V (2);
		rmiifer |= RGMII_FER_RGMII << RGMII_FER_V (3);
		bis->bi_phymode[0] = BI_PHYMODE_ZMII;
		bis->bi_phymode[1] = BI_PHYMODE_ZMII;
		bis->bi_phymode[2] = BI_PHYMODE_RGMII;
		bis->bi_phymode[3] = BI_PHYMODE_RGMII;
		break;
	case 5:
		zmiifer |= ZMII_FER_SMII << ZMII_FER_V (0);
		zmiifer |= ZMII_FER_SMII << ZMII_FER_V (1);
		zmiifer |= ZMII_FER_SMII << ZMII_FER_V (2);
		rmiifer |= RGMII_FER_RGMII << RGMII_FER_V(3);
		bis->bi_phymode[0] = BI_PHYMODE_ZMII;
		bis->bi_phymode[1] = BI_PHYMODE_ZMII;
		bis->bi_phymode[2] = BI_PHYMODE_ZMII;
		bis->bi_phymode[3] = BI_PHYMODE_RGMII;
		break;
	case 6:
		zmiifer |= ZMII_FER_SMII << ZMII_FER_V (0);
		zmiifer |= ZMII_FER_SMII << ZMII_FER_V (1);
		rmiifer |= RGMII_FER_RGMII << RGMII_FER_V(2);
		bis->bi_phymode[0] = BI_PHYMODE_ZMII;
		bis->bi_phymode[1] = BI_PHYMODE_ZMII;
		bis->bi_phymode[2] = BI_PHYMODE_RGMII;
		break;
	case 0:
	default:
		zmiifer = ZMII_FER_MII << ZMII_FER_V(devnum);
		rmiifer = 0x0;
		bis->bi_phymode[0] = BI_PHYMODE_ZMII;
		bis->bi_phymode[1] = BI_PHYMODE_ZMII;
		bis->bi_phymode[2] = BI_PHYMODE_ZMII;
		bis->bi_phymode[3] = BI_PHYMODE_ZMII;
		break;
	}

	/* Ensure we setup mdio for this devnum and ONLY this devnum */
	zmiifer |= (ZMII_FER_MDI) << ZMII_FER_V(devnum);

	out32 (ZMII_FER, zmiifer);
	out32 (RGMII_FER, rmiifer);

	return ((int)pfc1);

}
#endif

static int ppc_4xx_eth_init (struct eth_device *dev, bd_t * bis)
{
	int i, j;
	unsigned long reg = 0;
	unsigned long msr;
	unsigned long speed;
	unsigned long duplex;
	unsigned long failsafe;
	unsigned mode_reg;
	unsigned short devnum;
	unsigned short reg_short;
#if defined(CONFIG_440GX) || defined(CONFIG_440SP)
	sys_info_t sysinfo;
#if defined(CONFIG_440GX)
	int ethgroup = -1;
#endif
#endif

	EMAC_4XX_HW_PST hw_p = dev->priv;

	/* before doing anything, figure out if we have a MAC address */
	/* if not, bail */
	if (memcmp (dev->enetaddr, "\0\0\0\0\0\0", 6) == 0) {
		printf("ERROR: ethaddr not set!\n");
		return -1;
	}

#if defined(CONFIG_440GX) || defined(CONFIG_440SP)
	/* Need to get the OPB frequency so we can access the PHY */
	get_sys_info (&sysinfo);
#endif

	msr = mfmsr ();
	mtmsr (msr & ~(MSR_EE));	/* disable interrupts */

	devnum = hw_p->devnum;

#ifdef INFO_4XX_ENET
	/* AS.HARNOIS
	 * We should have :
	 * hw_p->stats.pkts_handled <=	hw_p->stats.pkts_rx <= hw_p->stats.pkts_handled+PKTBUFSRX
	 * In the most cases hw_p->stats.pkts_handled = hw_p->stats.pkts_rx, but it
	 * is possible that new packets (without relationship with
	 * current transfer) have got the time to arrived before
	 * netloop calls eth_halt
	 */
	printf ("About preceeding transfer (eth%d):\n"
		"- Sent packet number %d\n"
		"- Received packet number %d\n"
		"- Handled packet number %d\n",
		hw_p->devnum,
		hw_p->stats.pkts_tx,
		hw_p->stats.pkts_rx, hw_p->stats.pkts_handled);

	hw_p->stats.pkts_tx = 0;
	hw_p->stats.pkts_rx = 0;
	hw_p->stats.pkts_handled = 0;
#endif

	hw_p->tx_err_index = 0; /* Transmit Error Index for tx_err_log */
	hw_p->rx_err_index = 0; /* Receive Error Index for rx_err_log */

	hw_p->rx_slot = 0;	/* MAL Receive Slot */
	hw_p->rx_i_index = 0;	/* Receive Interrupt Queue Index */
	hw_p->rx_u_index = 0;	/* Receive User Queue Index */

	hw_p->tx_slot = 0;	/* MAL Transmit Slot */
	hw_p->tx_i_index = 0;	/* Transmit Interrupt Queue Index */
	hw_p->tx_u_index = 0;	/* Transmit User Queue Index */

#if defined(CONFIG_440) && !defined(CONFIG_440SP)
	/* set RMII mode */
	/* NOTE: 440GX spec states that mode is mutually exclusive */
	/* NOTE: Therefore, disable all other EMACS, since we handle */
	/* NOTE: only one emac at a time */
	reg = 0;
	out32 (ZMII_FER, 0);
	udelay (100);

#if defined(CONFIG_440EP) || defined(CONFIG_440GR)
	out32 (ZMII_FER, (ZMII_FER_RMII | ZMII_FER_MDI) << ZMII_FER_V (devnum));
#elif defined(CONFIG_440GX)
	ethgroup = ppc_4xx_eth_setup_bridge(devnum, bis);
#elif defined(CONFIG_440GP)
	/* set RMII mode */
	out32 (ZMII_FER, ZMII_RMII | ZMII_MDI0);
#else
	if ((devnum == 0) || (devnum == 1)) {
		out32 (ZMII_FER, (ZMII_FER_SMII | ZMII_FER_MDI) << ZMII_FER_V (devnum));
	}
	else { /* ((devnum == 2) || (devnum == 3)) */
		out32 (ZMII_FER, ZMII_FER_MDI << ZMII_FER_V (devnum));
		out32 (RGMII_FER, ((RGMII_FER_RGMII << RGMII_FER_V (2)) |
				   (RGMII_FER_RGMII << RGMII_FER_V (3))));
	}
#endif

	out32 (ZMII_SSR, ZMII_SSR_SP << ZMII_SSR_V(devnum));
#endif /* defined(CONFIG_440) && !defined(CONFIG_440SP) */

	__asm__ volatile ("eieio");

	/* reset emac so we have access to the phy */

	out32 (EMAC_M0 + hw_p->hw_addr, EMAC_M0_SRST);
	__asm__ volatile ("eieio");

	failsafe = 1000;
	while ((in32 (EMAC_M0 + hw_p->hw_addr) & (EMAC_M0_SRST)) && failsafe) {
		udelay (1000);
		failsafe--;
	}

#if defined(CONFIG_440GX) || defined(CONFIG_440SP)
	/* Whack the M1 register */
	mode_reg = 0x0;
	mode_reg &= ~0x00000038;
	if (sysinfo.freqOPB <= 50000000);
	else if (sysinfo.freqOPB <= 66666667)
		mode_reg |= EMAC_M1_OBCI_66;
	else if (sysinfo.freqOPB <= 83333333)
		mode_reg |= EMAC_M1_OBCI_83;
	else if (sysinfo.freqOPB <= 100000000)
		mode_reg |= EMAC_M1_OBCI_100;
	else
		mode_reg |= EMAC_M1_OBCI_GT100;

	out32 (EMAC_M1 + hw_p->hw_addr, mode_reg);
#endif /* defined(CONFIG_440GX) || defined(CONFIG_440SP) */

	/* wait for PHY to complete auto negotiation */
	reg_short = 0;
#ifndef CONFIG_CS8952_PHY
	switch (devnum) {
	case 0:
		reg = CONFIG_PHY_ADDR;
		break;
#if defined (CONFIG_PHY1_ADDR)
	case 1:
		reg = CONFIG_PHY1_ADDR;
		break;
#endif
#if defined (CONFIG_440GX)
	case 2:
		reg = CONFIG_PHY2_ADDR;
		break;
	case 3:
		reg = CONFIG_PHY3_ADDR;
		break;
#endif
	default:
		reg = CONFIG_PHY_ADDR;
		break;
	}

	bis->bi_phynum[devnum] = reg;

#if defined(CONFIG_PHY_RESET)
	/*
	 * Reset the phy, only if its the first time through
	 * otherwise, just check the speeds & feeds
	 */
	if (hw_p->first_init == 0) {
		miiphy_reset (dev->name, reg);

#if defined(CONFIG_440GX) || defined(CONFIG_440SP)
#if defined(CONFIG_CIS8201_PHY)
		/*
		 * Cicada 8201 PHY needs to have an extended register whacked
		 * for RGMII mode.
		 */
		if ( ((devnum == 2) || (devnum ==3)) && (4 == ethgroup) ) {
#if defined(CONFIG_CIS8201_SHORT_ETCH)
			miiphy_write (dev->name, reg, 23, 0x1300);
#else
			miiphy_write (dev->name, reg, 23, 0x1000);
#endif
			/*
			 * Vitesse VSC8201/Cicada CIS8201 errata:
			 * Interoperability problem with Intel 82547EI phys
			 * This work around (provided by Vitesse) changes
			 * the default timer convergence from 8ms to 12ms
			 */
			miiphy_write (dev->name, reg, 0x1f, 0x2a30);
			miiphy_write (dev->name, reg, 0x08, 0x0200);
			miiphy_write (dev->name, reg, 0x1f, 0x52b5);
			miiphy_write (dev->name, reg, 0x02, 0x0004);
			miiphy_write (dev->name, reg, 0x01, 0x0671);
			miiphy_write (dev->name, reg, 0x00, 0x8fae);
			miiphy_write (dev->name, reg, 0x1f, 0x2a30);
			miiphy_write (dev->name, reg, 0x08, 0x0000);
			miiphy_write (dev->name, reg, 0x1f, 0x0000);
			/* end Vitesse/Cicada errata */
		}
#endif
#endif
		/* Start/Restart autonegotiation */
		phy_setup_aneg (dev->name, reg);
		udelay (1000);
	}
#endif /* defined(CONFIG_PHY_RESET) */

	miiphy_read (dev->name, reg, PHY_BMSR, &reg_short);

	/*
	 * Wait if PHY is capable of autonegotiation and autonegotiation is not complete
	 */
	if ((reg_short & PHY_BMSR_AUTN_ABLE)
	    && !(reg_short & PHY_BMSR_AUTN_COMP)) {
		puts ("Waiting for PHY auto negotiation to complete");
		i = 0;
		while (!(reg_short & PHY_BMSR_AUTN_COMP)) {
			/*
			 * Timeout reached ?
			 */
			if (i > PHY_AUTONEGOTIATE_TIMEOUT) {
				puts (" TIMEOUT !\n");
				break;
			}

			if ((i++ % 1000) == 0) {
				putc ('.');
			}
			udelay (1000);	/* 1 ms */
			miiphy_read (dev->name, reg, PHY_BMSR, &reg_short);

		}
		puts (" done\n");
		udelay (500000);	/* another 500 ms (results in faster booting) */
	}
#endif /* #ifndef CONFIG_CS8952_PHY */

	speed = miiphy_speed (dev->name, reg);
	duplex = miiphy_duplex (dev->name, reg);

	if (hw_p->print_speed) {
		hw_p->print_speed = 0;
		printf ("ENET Speed is %d Mbps - %s duplex connection\n",
			(int) speed, (duplex == HALF) ? "HALF" : "FULL");
	}

#if defined(CONFIG_440) && !defined(CONFIG_440SP)
#if defined(CONFIG_440EP) || defined(CONFIG_440GR)
	mfsdr(sdr_mfr, reg);
	if (speed == 100) {
		reg = (reg & ~SDR0_MFR_ZMII_MODE_MASK) | SDR0_MFR_ZMII_MODE_RMII_100M;
	} else {
		reg = (reg & ~SDR0_MFR_ZMII_MODE_MASK) | SDR0_MFR_ZMII_MODE_RMII_10M;
	}
	mtsdr(sdr_mfr, reg);
#endif

	/* Set ZMII/RGMII speed according to the phy link speed */
	reg = in32 (ZMII_SSR);
	if ( (speed == 100) || (speed == 1000) )
		out32 (ZMII_SSR, reg | (ZMII_SSR_SP << ZMII_SSR_V (devnum)));
	else
		out32 (ZMII_SSR, reg & (~(ZMII_SSR_SP << ZMII_SSR_V (devnum))));

	if ((devnum == 2) || (devnum == 3)) {
		if (speed == 1000)
			reg = (RGMII_SSR_SP_1000MBPS << RGMII_SSR_V (devnum));
		else if (speed == 100)
			reg = (RGMII_SSR_SP_100MBPS << RGMII_SSR_V (devnum));
		else
			reg = (RGMII_SSR_SP_10MBPS << RGMII_SSR_V (devnum));

		out32 (RGMII_SSR, reg);
	}
#endif /* defined(CONFIG_440) && !defined(CONFIG_440SP) */

	/* set the Mal configuration reg */
#if defined(CONFIG_440GX) || defined(CONFIG_440SP)
	mtdcr (malmcr, MAL_CR_PLBB | MAL_CR_OPBBL | MAL_CR_LEA |
	       MAL_CR_PLBLT_DEFAULT | MAL_CR_EOPIE | 0x00330000);
#else
	mtdcr (malmcr, MAL_CR_PLBB | MAL_CR_OPBBL | MAL_CR_LEA | MAL_CR_PLBLT_DEFAULT);
	/* Errata 1.12: MAL_1 -- Disable MAL bursting */
	if (get_pvr() == PVR_440GP_RB) {
		mtdcr (malmcr, mfdcr(malmcr) & ~MAL_CR_PLBB);
	}
#endif

	/* Free "old" buffers */
	if (hw_p->alloc_tx_buf)
		free (hw_p->alloc_tx_buf);
	if (hw_p->alloc_rx_buf)
		free (hw_p->alloc_rx_buf);

	/*
	 * Malloc MAL buffer desciptors, make sure they are
	 * aligned on cache line boundary size
	 * (401/403/IOP480 = 16, 405 = 32)
	 * and doesn't cross cache block boundaries.
	 */
	hw_p->alloc_tx_buf =
		(mal_desc_t *) malloc ((sizeof (mal_desc_t) * NUM_TX_BUFF) +
				       ((2 * CFG_CACHELINE_SIZE) - 2));
	if (NULL == hw_p->alloc_tx_buf)
		return -1;
	if (((int) hw_p->alloc_tx_buf & CACHELINE_MASK) != 0) {
		hw_p->tx =
			(mal_desc_t *) ((int) hw_p->alloc_tx_buf +
					CFG_CACHELINE_SIZE -
					((int) hw_p->
					 alloc_tx_buf & CACHELINE_MASK));
	} else {
		hw_p->tx = hw_p->alloc_tx_buf;
	}

	hw_p->alloc_rx_buf =
		(mal_desc_t *) malloc ((sizeof (mal_desc_t) * NUM_RX_BUFF) +
				       ((2 * CFG_CACHELINE_SIZE) - 2));
	if (NULL == hw_p->alloc_rx_buf) {
		free(hw_p->alloc_tx_buf);
		hw_p->alloc_tx_buf = NULL;
		return -1;
	}

	if (((int) hw_p->alloc_rx_buf & CACHELINE_MASK) != 0) {
		hw_p->rx =
			(mal_desc_t *) ((int) hw_p->alloc_rx_buf +
					CFG_CACHELINE_SIZE -
					((int) hw_p->
					 alloc_rx_buf & CACHELINE_MASK));
	} else {
		hw_p->rx = hw_p->alloc_rx_buf;
	}

	for (i = 0; i < NUM_TX_BUFF; i++) {
		hw_p->tx[i].ctrl = 0;
		hw_p->tx[i].data_len = 0;
		if (hw_p->first_init == 0) {
			hw_p->txbuf_ptr =
				(char *) malloc (ENET_MAX_MTU_ALIGNED);
			if (NULL == hw_p->txbuf_ptr) {
				free(hw_p->alloc_rx_buf);
				free(hw_p->alloc_tx_buf);
				hw_p->alloc_rx_buf = NULL;
				hw_p->alloc_tx_buf = NULL;
				for(j = 0; j < i; j++) {
					free(hw_p->tx[i].data_ptr);
					hw_p->tx[i].data_ptr = NULL;
				}
			}
		}
		hw_p->tx[i].data_ptr = hw_p->txbuf_ptr;
		if ((NUM_TX_BUFF - 1) == i)
			hw_p->tx[i].ctrl |= MAL_TX_CTRL_WRAP;
		hw_p->tx_run[i] = -1;
#if 0
		printf ("TX_BUFF %d @ 0x%08lx\n", i,
			(ulong) hw_p->tx[i].data_ptr);
#endif
	}

	for (i = 0; i < NUM_RX_BUFF; i++) {
		hw_p->rx[i].ctrl = 0;
		hw_p->rx[i].data_len = 0;
		/*	 rx[i].data_ptr = (char *) &rx_buff[i]; */
		hw_p->rx[i].data_ptr = (char *) NetRxPackets[i];
		if ((NUM_RX_BUFF - 1) == i)
			hw_p->rx[i].ctrl |= MAL_RX_CTRL_WRAP;
		hw_p->rx[i].ctrl |= MAL_RX_CTRL_EMPTY | MAL_RX_CTRL_INTR;
		hw_p->rx_ready[i] = -1;
#if 0
		printf ("RX_BUFF %d @ 0x%08lx\n", i, (ulong) rx[i].data_ptr);
#endif
	}

	reg = 0x00000000;

	reg |= dev->enetaddr[0];	/* set high address */
	reg = reg << 8;
	reg |= dev->enetaddr[1];

	out32 (EMAC_IAH + hw_p->hw_addr, reg);

	reg = 0x00000000;
	reg |= dev->enetaddr[2];	/* set low address  */
	reg = reg << 8;
	reg |= dev->enetaddr[3];
	reg = reg << 8;
	reg |= dev->enetaddr[4];
	reg = reg << 8;
	reg |= dev->enetaddr[5];

	out32 (EMAC_IAL + hw_p->hw_addr, reg);

	switch (devnum) {
	case 1:
		/* setup MAL tx & rx channel pointers */
#if defined (CONFIG_405EP) || defined (CONFIG_440EP) || defined (CONFIG_440GR)
		mtdcr (maltxctp2r, hw_p->tx);
#else
		mtdcr (maltxctp1r, hw_p->tx);
#endif
#if defined(CONFIG_440)
		mtdcr (maltxbattr, 0x0);
		mtdcr (malrxbattr, 0x0);
#endif
		mtdcr (malrxctp1r, hw_p->rx);
		/* set RX buffer size */
		mtdcr (malrcbs1, ENET_MAX_MTU_ALIGNED / 16);
		break;
#if defined (CONFIG_440GX)
	case 2:
		/* setup MAL tx & rx channel pointers */
		mtdcr (maltxbattr, 0x0);
		mtdcr (malrxbattr, 0x0);
		mtdcr (maltxctp2r, hw_p->tx);
		mtdcr (malrxctp2r, hw_p->rx);
		/* set RX buffer size */
		mtdcr (malrcbs2, ENET_MAX_MTU_ALIGNED / 16);
		break;
	case 3:
		/* setup MAL tx & rx channel pointers */
		mtdcr (maltxbattr, 0x0);
		mtdcr (maltxctp3r, hw_p->tx);
		mtdcr (malrxbattr, 0x0);
		mtdcr (malrxctp3r, hw_p->rx);
		/* set RX buffer size */
		mtdcr (malrcbs3, ENET_MAX_MTU_ALIGNED / 16);
		break;
#endif /* CONFIG_440GX */
	case 0:
	default:
		/* setup MAL tx & rx channel pointers */
#if defined(CONFIG_440)
		mtdcr (maltxbattr, 0x0);
		mtdcr (malrxbattr, 0x0);
#endif
		mtdcr (maltxctp0r, hw_p->tx);
		mtdcr (malrxctp0r, hw_p->rx);
		/* set RX buffer size */
		mtdcr (malrcbs0, ENET_MAX_MTU_ALIGNED / 16);
		break;
	}

	/* Enable MAL transmit and receive channels */
#if defined(CONFIG_405EP) || defined(CONFIG_440EP) || defined(CONFIG_440GR)
	mtdcr (maltxcasr, (MAL_TXRX_CASR >> (hw_p->devnum*2)));
#else
	mtdcr (maltxcasr, (MAL_TXRX_CASR >> hw_p->devnum));
#endif
	mtdcr (malrxcasr, (MAL_TXRX_CASR >> hw_p->devnum));

	/* set transmit enable & receive enable */
	out32 (EMAC_M0 + hw_p->hw_addr, EMAC_M0_TXE | EMAC_M0_RXE);

	/* set receive fifo to 4k and tx fifo to 2k */
	mode_reg = in32 (EMAC_M1 + hw_p->hw_addr);
	mode_reg |= EMAC_M1_RFS_4K | EMAC_M1_TX_FIFO_2K;

	/* set speed */
	if (speed == _1000BASET) {
#if defined(CONFIG_440SP)
#define SDR0_PFC1_EM_1000	0x00200000
		unsigned long pfc1;
		mfsdr (sdr_pfc1, pfc1);
		pfc1 |= SDR0_PFC1_EM_1000;
		mtsdr (sdr_pfc1, pfc1);
#endif
		mode_reg = mode_reg | EMAC_M1_MF_1000MBPS | EMAC_M1_IST;
	} else if (speed == _100BASET)
		mode_reg = mode_reg | EMAC_M1_MF_100MBPS | EMAC_M1_IST;
	else
		mode_reg = mode_reg & ~0x00C00000;	/* 10 MBPS */
	if (duplex == FULL)
		mode_reg = mode_reg | 0x80000000 | EMAC_M1_IST;

	out32 (EMAC_M1 + hw_p->hw_addr, mode_reg);

	/* Enable broadcast and indvidual address */
	/* TBS: enabling runts as some misbehaved nics will send runts */
	out32 (EMAC_RXM + hw_p->hw_addr, EMAC_RMR_BAE | EMAC_RMR_IAE);

	/* we probably need to set the tx mode1 reg? maybe at tx time */

	/* set transmit request threshold register */
	out32 (EMAC_TRTR + hw_p->hw_addr, 0x18000000);	/* 256 byte threshold */

	/* set receive	low/high water mark register */
#if defined(CONFIG_440)
	/* 440GP has a 64 byte burst length */
	out32 (EMAC_RX_HI_LO_WMARK + hw_p->hw_addr, 0x80009000);
#else
	/* 405s have a 16 byte burst length */
	out32 (EMAC_RX_HI_LO_WMARK + hw_p->hw_addr, 0x0f002000);
#endif /* defined(CONFIG_440) */
	out32 (EMAC_TXM1 + hw_p->hw_addr, 0xf8640000);

	/* Set fifo limit entry in tx mode 0 */
	out32 (EMAC_TXM0 + hw_p->hw_addr, 0x00000003);
	/* Frame gap set */
	out32 (EMAC_I_FRAME_GAP_REG + hw_p->hw_addr, 0x00000008);

	/* Set EMAC IER */
	hw_p->emac_ier = EMAC_ISR_PTLE | EMAC_ISR_BFCS | EMAC_ISR_ORE | EMAC_ISR_IRE;
	if (speed == _100BASET)
		hw_p->emac_ier = hw_p->emac_ier | EMAC_ISR_SYE;

	out32 (EMAC_ISR + hw_p->hw_addr, 0xffffffff);	/* clear pending interrupts */
	out32 (EMAC_IER + hw_p->hw_addr, hw_p->emac_ier);

	if (hw_p->first_init == 0) {
		/*
		 * Connect interrupt service routines
		 */
		irq_install_handler (VECNUM_ETH0 + (hw_p->devnum * 2),
				     (interrupt_handler_t *) enetInt, dev);
	}

	mtmsr (msr);		/* enable interrupts again */

	hw_p->bis = bis;
	hw_p->first_init = 1;

	return (1);
}


static int ppc_4xx_eth_send (struct eth_device *dev, volatile void *ptr,
			      int len)
{
	struct enet_frame *ef_ptr;
	ulong time_start, time_now;
	unsigned long temp_txm0;
	EMAC_4XX_HW_PST hw_p = dev->priv;

	ef_ptr = (struct enet_frame *) ptr;

	/*-----------------------------------------------------------------------+
	 *  Copy in our address into the frame.
	 *-----------------------------------------------------------------------*/
	(void) memcpy (ef_ptr->source_addr, dev->enetaddr, ENET_ADDR_LENGTH);

	/*-----------------------------------------------------------------------+
	 * If frame is too long or too short, modify length.
	 *-----------------------------------------------------------------------*/
	/* TBS: where does the fragment go???? */
	if (len > ENET_MAX_MTU)
		len = ENET_MAX_MTU;

	/*   memcpy ((void *) &tx_buff[tx_slot], (const void *) ptr, len); */
	memcpy ((void *) hw_p->txbuf_ptr, (const void *) ptr, len);

	/*-----------------------------------------------------------------------+
	 * set TX Buffer busy, and send it
	 *-----------------------------------------------------------------------*/
	hw_p->tx[hw_p->tx_slot].ctrl = (MAL_TX_CTRL_LAST |
					EMAC_TX_CTRL_GFCS | EMAC_TX_CTRL_GP) &
		~(EMAC_TX_CTRL_ISA | EMAC_TX_CTRL_RSA);
	if ((NUM_TX_BUFF - 1) == hw_p->tx_slot)
		hw_p->tx[hw_p->tx_slot].ctrl |= MAL_TX_CTRL_WRAP;

	hw_p->tx[hw_p->tx_slot].data_len = (short) len;
	hw_p->tx[hw_p->tx_slot].ctrl |= MAL_TX_CTRL_READY;

	__asm__ volatile ("eieio");

	out32 (EMAC_TXM0 + hw_p->hw_addr,
	       in32 (EMAC_TXM0 + hw_p->hw_addr) | EMAC_TXM0_GNP0);
#ifdef INFO_4XX_ENET
	hw_p->stats.pkts_tx++;
#endif

	/*-----------------------------------------------------------------------+
	 * poll unitl the packet is sent and then make sure it is OK
	 *-----------------------------------------------------------------------*/
	time_start = get_timer (0);
	while (1) {
		temp_txm0 = in32 (EMAC_TXM0 + hw_p->hw_addr);
		/* loop until either TINT turns on or 3 seconds elapse */
		if ((temp_txm0 & EMAC_TXM0_GNP0) != 0) {
			/* transmit is done, so now check for errors
			 * If there is an error, an interrupt should
			 * happen when we return
			 */
			time_now = get_timer (0);
			if ((time_now - time_start) > 3000) {
				return (-1);
			}
		} else {
			return (len);
		}
	}
}


#if defined (CONFIG_440)

#if defined(CONFIG_440SP)
/*
 * Hack: On 440SP all enet irq sources are located on UIC1
 * Needs some cleanup. --sr
 */
#define UIC0MSR		uic1msr
#define UIC0SR		uic1sr
#else
#define UIC0MSR		uic0msr
#define UIC0SR		uic0sr
#endif

int enetInt (struct eth_device *dev)
{
	int serviced;
	int rc = -1;		/* default to not us */
	unsigned long mal_isr;
	unsigned long emac_isr = 0;
	unsigned long mal_rx_eob;
	unsigned long my_uic0msr, my_uic1msr;

#if defined(CONFIG_440GX)
	unsigned long my_uic2msr;
#endif
	EMAC_4XX_HW_PST hw_p;

	/*
	 * Because the mal is generic, we need to get the current
	 * eth device
	 */
#if defined(CONFIG_NET_MULTI)
	dev = eth_get_dev();
#else
	dev = emac0_dev;
#endif

	hw_p = dev->priv;

	/* enter loop that stays in interrupt code until nothing to service */
	do {
		serviced = 0;

		my_uic0msr = mfdcr (UIC0MSR);
		my_uic1msr = mfdcr (uic1msr);
#if defined(CONFIG_440GX)
		my_uic2msr = mfdcr (uic2msr);
#endif
		if (!(my_uic0msr & (UIC_MRE | UIC_MTE))
		    && !(my_uic1msr & (UIC_ETH0 | UIC_ETH1 | UIC_MS | UIC_MTDE | UIC_MRDE))) {
			/* not for us */
			return (rc);
		}
#if defined (CONFIG_440GX)
		if (!(my_uic0msr & (UIC_MRE | UIC_MTE))
		    && !(my_uic2msr & (UIC_ETH2 | UIC_ETH3))) {
			/* not for us */
			return (rc);
		}
#endif
		/* get and clear controller status interrupts */
		/* look at Mal and EMAC interrupts */
		if ((my_uic0msr & (UIC_MRE | UIC_MTE))
		    || (my_uic1msr & (UIC_MS | UIC_MTDE | UIC_MRDE))) {
			/* we have a MAL interrupt */
			mal_isr = mfdcr (malesr);
			/* look for mal error */
			if (my_uic1msr & (UIC_MS | UIC_MTDE | UIC_MRDE)) {
				mal_err (dev, mal_isr, my_uic0msr,
					 MAL_UIC_DEF, MAL_UIC_ERR);
				serviced = 1;
				rc = 0;
			}
		}

		/* port by port dispatch of emac interrupts */
		if (hw_p->devnum == 0) {
			if (UIC_ETH0 & my_uic1msr) {	/* look for EMAC errors */
				emac_isr = in32 (EMAC_ISR + hw_p->hw_addr);
				if ((hw_p->emac_ier & emac_isr) != 0) {
					emac_err (dev, emac_isr);
					serviced = 1;
					rc = 0;
				}
			}
			if ((hw_p->emac_ier & emac_isr)
			    || (my_uic1msr & (UIC_MS | UIC_MTDE | UIC_MRDE))) {
				mtdcr (UIC0SR, UIC_MRE | UIC_MTE);	/* Clear */
				mtdcr (uic1sr, UIC_ETH0 | UIC_MS | UIC_MTDE | UIC_MRDE);	/* Clear */
				return (rc);	/* we had errors so get out */
			}
		}

#if !defined(CONFIG_440SP)
		if (hw_p->devnum == 1) {
			if (UIC_ETH1 & my_uic1msr) {	/* look for EMAC errors */
				emac_isr = in32 (EMAC_ISR + hw_p->hw_addr);
				if ((hw_p->emac_ier & emac_isr) != 0) {
					emac_err (dev, emac_isr);
					serviced = 1;
					rc = 0;
				}
			}
			if ((hw_p->emac_ier & emac_isr)
			    || (my_uic1msr & (UIC_MS | UIC_MTDE | UIC_MRDE))) {
				mtdcr (UIC0SR, UIC_MRE | UIC_MTE);	/* Clear */
				mtdcr (uic1sr, UIC_ETH1 | UIC_MS | UIC_MTDE | UIC_MRDE);	/* Clear */
				return (rc);	/* we had errors so get out */
			}
		}
#if defined (CONFIG_440GX)
		if (hw_p->devnum == 2) {
			if (UIC_ETH2 & my_uic2msr) {	/* look for EMAC errors */
				emac_isr = in32 (EMAC_ISR + hw_p->hw_addr);
				if ((hw_p->emac_ier & emac_isr) != 0) {
					emac_err (dev, emac_isr);
					serviced = 1;
					rc = 0;
				}
			}
			if ((hw_p->emac_ier & emac_isr)
			    || (my_uic1msr & (UIC_MS | UIC_MTDE | UIC_MRDE))) {
				mtdcr (UIC0SR, UIC_MRE | UIC_MTE);	/* Clear */
				mtdcr (uic1sr, UIC_MS | UIC_MTDE | UIC_MRDE);	/* Clear */
				mtdcr (uic2sr, UIC_ETH2);
				return (rc);	/* we had errors so get out */
			}
		}

		if (hw_p->devnum == 3) {
			if (UIC_ETH3 & my_uic2msr) {	/* look for EMAC errors */
				emac_isr = in32 (EMAC_ISR + hw_p->hw_addr);
				if ((hw_p->emac_ier & emac_isr) != 0) {
					emac_err (dev, emac_isr);
					serviced = 1;
					rc = 0;
				}
			}
			if ((hw_p->emac_ier & emac_isr)
			    || (my_uic1msr & (UIC_MS | UIC_MTDE | UIC_MRDE))) {
				mtdcr (UIC0SR, UIC_MRE | UIC_MTE);	/* Clear */
				mtdcr (uic1sr, UIC_MS | UIC_MTDE | UIC_MRDE);	/* Clear */
				mtdcr (uic2sr, UIC_ETH3);
				return (rc);	/* we had errors so get out */
			}
		}
#endif /* CONFIG_440GX */
#endif /* !CONFIG_440SP */

		/* handle MAX TX EOB interrupt from a tx */
		if (my_uic0msr & UIC_MTE) {
			mal_rx_eob = mfdcr (maltxeobisr);
			mtdcr (maltxeobisr, mal_rx_eob);
			mtdcr (UIC0SR, UIC_MTE);
		}
		/* handle MAL RX EOB  interupt from a receive */
		/* check for EOB on valid channels	      */
		if (my_uic0msr & UIC_MRE) {
			mal_rx_eob = mfdcr (malrxeobisr);
			if ((mal_rx_eob & (0x80000000 >> hw_p->devnum)) != 0) { /* call emac routine for channel x */
				/* clear EOB
				   mtdcr(malrxeobisr, mal_rx_eob); */
				enet_rcv (dev, emac_isr);
				/* indicate that we serviced an interrupt */
				serviced = 1;
				rc = 0;
			}
		}

		mtdcr (UIC0SR, UIC_MRE);	/* Clear */
		mtdcr (uic1sr, UIC_MS | UIC_MTDE | UIC_MRDE);	/* Clear */
		switch (hw_p->devnum) {
		case 0:
			mtdcr (uic1sr, UIC_ETH0);
			break;
		case 1:
			mtdcr (uic1sr, UIC_ETH1);
			break;
#if defined (CONFIG_440GX)
		case 2:
			mtdcr (uic2sr, UIC_ETH2);
			break;
		case 3:
			mtdcr (uic2sr, UIC_ETH3);
			break;
#endif /* CONFIG_440GX */
		default:
			break;
		}
	} while (serviced);

	return (rc);
}

#else /* CONFIG_440 */

int enetInt (struct eth_device *dev)
{
	int serviced;
	int rc = -1;		/* default to not us */
	unsigned long mal_isr;
	unsigned long emac_isr = 0;
	unsigned long mal_rx_eob;
	unsigned long my_uicmsr;

	EMAC_4XX_HW_PST hw_p;

	/*
	 * Because the mal is generic, we need to get the current
	 * eth device
	 */
#if defined(CONFIG_NET_MULTI)
	dev = eth_get_dev();
#else
	dev = emac0_dev;
#endif

	hw_p = dev->priv;

	/* enter loop that stays in interrupt code until nothing to service */
	do {
		serviced = 0;

		my_uicmsr = mfdcr (uicmsr);

		if ((my_uicmsr & (MAL_UIC_DEF | EMAC_UIC_DEF)) == 0) {	/* not for us */
			return (rc);
		}
		/* get and clear controller status interrupts */
		/* look at Mal and EMAC interrupts */
		if ((MAL_UIC_DEF & my_uicmsr) != 0) {	/* we have a MAL interrupt */
			mal_isr = mfdcr (malesr);
			/* look for mal error */
			if ((my_uicmsr & MAL_UIC_ERR) != 0) {
				mal_err (dev, mal_isr, my_uicmsr, MAL_UIC_DEF, MAL_UIC_ERR);
				serviced = 1;
				rc = 0;
			}
		}

		/* port by port dispatch of emac interrupts */

		if ((SEL_UIC_DEF(hw_p->devnum) & my_uicmsr) != 0) {	/* look for EMAC errors */
			emac_isr = in32 (EMAC_ISR + hw_p->hw_addr);
			if ((hw_p->emac_ier & emac_isr) != 0) {
				emac_err (dev, emac_isr);
				serviced = 1;
				rc = 0;
			}
		}
		if (((hw_p->emac_ier & emac_isr) != 0) || ((MAL_UIC_ERR & my_uicmsr) != 0)) {
			mtdcr (uicsr, MAL_UIC_DEF | SEL_UIC_DEF(hw_p->devnum)); /* Clear */
			return (rc);		/* we had errors so get out */
		}

		/* handle MAX TX EOB interrupt from a tx */
		if (my_uicmsr & UIC_MAL_TXEOB) {
			mal_rx_eob = mfdcr (maltxeobisr);
			mtdcr (maltxeobisr, mal_rx_eob);
			mtdcr (uicsr, UIC_MAL_TXEOB);
		}
		/* handle MAL RX EOB  interupt from a receive */
		/* check for EOB on valid channels	      */
		if (my_uicmsr & UIC_MAL_RXEOB)
		{
			mal_rx_eob = mfdcr (malrxeobisr);
			if ((mal_rx_eob & (0x80000000 >> hw_p->devnum)) != 0) { /* call emac routine for channel x */
				/* clear EOB
				 mtdcr(malrxeobisr, mal_rx_eob); */
				enet_rcv (dev, emac_isr);
				/* indicate that we serviced an interrupt */
				serviced = 1;
				rc = 0;
			}
		}
		mtdcr (uicsr, MAL_UIC_DEF|EMAC_UIC_DEF|EMAC_UIC_DEF1);	/* Clear */
	}
	while (serviced);

	return (rc);
}

#endif /* CONFIG_440 */

/*-----------------------------------------------------------------------------+
 *  MAL Error Routine
 *-----------------------------------------------------------------------------*/
static void mal_err (struct eth_device *dev, unsigned long isr,
		     unsigned long uic, unsigned long maldef,
		     unsigned long mal_errr)
{
	EMAC_4XX_HW_PST hw_p = dev->priv;

	mtdcr (malesr, isr);	/* clear interrupt */

	/* clear DE interrupt */
	mtdcr (maltxdeir, 0xC0000000);
	mtdcr (malrxdeir, 0x80000000);

#ifdef INFO_4XX_ENET
	printf ("\nMAL error occured.... ISR = %lx UIC = = %lx	MAL_DEF = %lx  MAL_ERR= %lx \n", isr, uic, maldef, mal_errr);
#endif

	eth_init (hw_p->bis);	/* start again... */
}

/*-----------------------------------------------------------------------------+
 *  EMAC Error Routine
 *-----------------------------------------------------------------------------*/
static void emac_err (struct eth_device *dev, unsigned long isr)
{
	EMAC_4XX_HW_PST hw_p = dev->priv;

	printf ("EMAC%d error occured.... ISR = %lx\n", hw_p->devnum, isr);
	out32 (EMAC_ISR + hw_p->hw_addr, isr);
}

/*-----------------------------------------------------------------------------+
 *  enet_rcv() handles the ethernet receive data
 *-----------------------------------------------------------------------------*/
static void enet_rcv (struct eth_device *dev, unsigned long malisr)
{
	struct enet_frame *ef_ptr;
	unsigned long data_len;
	unsigned long rx_eob_isr;
	EMAC_4XX_HW_PST hw_p = dev->priv;

	int handled = 0;
	int i;
	int loop_count = 0;

	rx_eob_isr = mfdcr (malrxeobisr);
	if ((0x80000000 >> hw_p->devnum) & rx_eob_isr) {
		/* clear EOB */
		mtdcr (malrxeobisr, rx_eob_isr);

		/* EMAC RX done */
		while (1) {	/* do all */
			i = hw_p->rx_slot;

			if ((MAL_RX_CTRL_EMPTY & hw_p->rx[i].ctrl)
			    || (loop_count >= NUM_RX_BUFF))
				break;
			loop_count++;
			hw_p->rx_slot++;
			if (NUM_RX_BUFF == hw_p->rx_slot)
				hw_p->rx_slot = 0;
			handled++;
			data_len = (unsigned long) hw_p->rx[i].data_len;	/* Get len */
			if (data_len) {
				if (data_len > ENET_MAX_MTU)	/* Check len */
					data_len = 0;
				else {
					if (EMAC_RX_ERRORS & hw_p->rx[i].ctrl) {	/* Check Errors */
						data_len = 0;
						hw_p->stats.rx_err_log[hw_p->
								       rx_err_index]
							= hw_p->rx[i].ctrl;
						hw_p->rx_err_index++;
						if (hw_p->rx_err_index ==
						    MAX_ERR_LOG)
							hw_p->rx_err_index =
								0;
					}	/* emac_erros */
				}	/* data_len < max mtu */
			}	/* if data_len */
			if (!data_len) {	/* no data */
				hw_p->rx[i].ctrl |= MAL_RX_CTRL_EMPTY;	/* Free Recv Buffer */

				hw_p->stats.data_len_err++;	/* Error at Rx */
			}

			/* !data_len */
			/* AS.HARNOIS */
			/* Check if user has already eaten buffer */
			/* if not => ERROR */
			else if (hw_p->rx_ready[hw_p->rx_i_index] != -1) {
				if (hw_p->is_receiving)
					printf ("ERROR : Receive buffers are full!\n");
				break;
			} else {
				hw_p->stats.rx_frames++;
				hw_p->stats.rx += data_len;
				ef_ptr = (struct enet_frame *) hw_p->rx[i].
					data_ptr;
#ifdef INFO_4XX_ENET
				hw_p->stats.pkts_rx++;
#endif
				/* AS.HARNOIS
				 * use ring buffer
				 */
				hw_p->rx_ready[hw_p->rx_i_index] = i;
				hw_p->rx_i_index++;
				if (NUM_RX_BUFF == hw_p->rx_i_index)
					hw_p->rx_i_index = 0;

				/*  AS.HARNOIS
				 * free receive buffer only when
				 * buffer has been handled (eth_rx)
				 rx[i].ctrl |= MAL_RX_CTRL_EMPTY;
				 */
			}	/* if data_len */
		}		/* while */
	}			/* if EMACK_RXCHL */
}


static int ppc_4xx_eth_rx (struct eth_device *dev)
{
	int length;
	int user_index;
	unsigned long msr;
	EMAC_4XX_HW_PST hw_p = dev->priv;

	hw_p->is_receiving = 1; /* tell driver */

	for (;;) {
		/* AS.HARNOIS
		 * use ring buffer and
		 * get index from rx buffer desciptor queue
		 */
		user_index = hw_p->rx_ready[hw_p->rx_u_index];
		if (user_index == -1) {
			length = -1;
			break;	/* nothing received - leave for() loop */
		}

		msr = mfmsr ();
		mtmsr (msr & ~(MSR_EE));

		length = hw_p->rx[user_index].data_len;

		/* Pass the packet up to the protocol layers. */
		/*	 NetReceive(NetRxPackets[rxIdx], length - 4); */
		/*	 NetReceive(NetRxPackets[i], length); */
		NetReceive (NetRxPackets[user_index], length - 4);
		/* Free Recv Buffer */
		hw_p->rx[user_index].ctrl |= MAL_RX_CTRL_EMPTY;
		/* Free rx buffer descriptor queue */
		hw_p->rx_ready[hw_p->rx_u_index] = -1;
		hw_p->rx_u_index++;
		if (NUM_RX_BUFF == hw_p->rx_u_index)
			hw_p->rx_u_index = 0;

#ifdef INFO_4XX_ENET
		hw_p->stats.pkts_handled++;
#endif

		mtmsr (msr);	/* Enable IRQ's */
	}

	hw_p->is_receiving = 0; /* tell driver */

	return length;
}

int ppc_4xx_eth_initialize (bd_t * bis)
{
	static int virgin = 0;
	struct eth_device *dev;
	int eth_num = 0;
	EMAC_4XX_HW_PST hw = NULL;

#if defined(CONFIG_440GX)
	unsigned long pfc1;

	mfsdr (sdr_pfc1, pfc1);
	pfc1 &= ~(0x01e00000);
	pfc1 |= 0x01200000;
	mtsdr (sdr_pfc1, pfc1);
#endif
	/* set phy num and mode */
	bis->bi_phynum[0] = CONFIG_PHY_ADDR;
#if defined(CONFIG_PHY1_ADDR)
	bis->bi_phynum[1] = CONFIG_PHY1_ADDR;
#endif
#if defined(CONFIG_440GX)
	bis->bi_phynum[2] = CONFIG_PHY2_ADDR;
	bis->bi_phynum[3] = CONFIG_PHY3_ADDR;
	bis->bi_phymode[0] = 0;
	bis->bi_phymode[1] = 0;
	bis->bi_phymode[2] = 2;
	bis->bi_phymode[3] = 2;

#if defined (CONFIG_440GX)
	ppc_4xx_eth_setup_bridge(0, bis);
#endif
#endif

	for (eth_num = 0; eth_num < LAST_EMAC_NUM; eth_num++) {

		/* See if we can actually bring up the interface, otherwise, skip it */
		switch (eth_num) {
		default:		/* fall through */
		case 0:
			if (memcmp (bis->bi_enetaddr, "\0\0\0\0\0\0", 6) == 0) {
				bis->bi_phymode[eth_num] = BI_PHYMODE_NONE;
				continue;
			}
			break;
#ifdef CONFIG_HAS_ETH1
		case 1:
			if (memcmp (bis->bi_enet1addr, "\0\0\0\0\0\0", 6) == 0) {
				bis->bi_phymode[eth_num] = BI_PHYMODE_NONE;
				continue;
			}
			break;
#endif
#ifdef CONFIG_HAS_ETH2
		case 2:
			if (memcmp (bis->bi_enet2addr, "\0\0\0\0\0\0", 6) == 0) {
				bis->bi_phymode[eth_num] = BI_PHYMODE_NONE;
				continue;
			}
			break;
#endif
#ifdef CONFIG_HAS_ETH3
		case 3:
			if (memcmp (bis->bi_enet3addr, "\0\0\0\0\0\0", 6) == 0) {
				bis->bi_phymode[eth_num] = BI_PHYMODE_NONE;
				continue;
			}
			break;
#endif
		}

		/* Allocate device structure */
		dev = (struct eth_device *) malloc (sizeof (*dev));
		if (dev == NULL) {
			printf ("ppc_4xx_eth_initialize: "
				"Cannot allocate eth_device %d\n", eth_num);
			return (-1);
		}
		memset(dev, 0, sizeof(*dev));

		/* Allocate our private use data */
		hw = (EMAC_4XX_HW_PST) malloc (sizeof (*hw));
		if (hw == NULL) {
			printf ("ppc_4xx_eth_initialize: "
				"Cannot allocate private hw data for eth_device %d",
				eth_num);
			free (dev);
			return (-1);
		}
		memset(hw, 0, sizeof(*hw));

		switch (eth_num) {
		default:		/* fall through */
		case 0:
			hw->hw_addr = 0;
			memcpy (dev->enetaddr, bis->bi_enetaddr, 6);
			break;
#ifdef CONFIG_HAS_ETH1
		case 1:
			hw->hw_addr = 0x100;
			memcpy (dev->enetaddr, bis->bi_enet1addr, 6);
			break;
#endif
#ifdef CONFIG_HAS_ETH2
		case 2:
			hw->hw_addr = 0x400;
			memcpy (dev->enetaddr, bis->bi_enet2addr, 6);
			break;
#endif
#ifdef CONFIG_HAS_ETH3
		case 3:
			hw->hw_addr = 0x600;
			memcpy (dev->enetaddr, bis->bi_enet3addr, 6);
			break;
#endif
		}

		hw->devnum = eth_num;
		hw->print_speed = 1;

		sprintf (dev->name, "ppc_4xx_eth%d", eth_num);
		dev->priv = (void *) hw;
		dev->init = ppc_4xx_eth_init;
		dev->halt = ppc_4xx_eth_halt;
		dev->send = ppc_4xx_eth_send;
		dev->recv = ppc_4xx_eth_rx;

		if (0 == virgin) {
			/* set the MAL IER ??? names may change with new spec ??? */
			mal_ier =
				MAL_IER_DE | MAL_IER_NE | MAL_IER_TE |
				MAL_IER_OPBE | MAL_IER_PLBE;
			mtdcr (malesr, 0xffffffff);	/* clear pending interrupts */
			mtdcr (maltxdeir, 0xffffffff);	/* clear pending interrupts */
			mtdcr (malrxdeir, 0xffffffff);	/* clear pending interrupts */
			mtdcr (malier, mal_ier);

			/* install MAL interrupt handler */
			irq_install_handler (VECNUM_MS,
					     (interrupt_handler_t *) enetInt,
					     dev);
			irq_install_handler (VECNUM_MTE,
					     (interrupt_handler_t *) enetInt,
					     dev);
			irq_install_handler (VECNUM_MRE,
					     (interrupt_handler_t *) enetInt,
					     dev);
			irq_install_handler (VECNUM_TXDE,
					     (interrupt_handler_t *) enetInt,
					     dev);
			irq_install_handler (VECNUM_RXDE,
					     (interrupt_handler_t *) enetInt,
					     dev);
			virgin = 1;
		}

#if defined(CONFIG_NET_MULTI)
		eth_register (dev);
#else
		emac0_dev = dev;
#endif
#if defined(CONFIG_MII) || (CONFIG_COMMANDS & CFG_CMD_MII)
		miiphy_register (dev->name,
				 emac4xx_miiphy_read, emac4xx_miiphy_write);
#endif

	}			/* end for each supported device */
	return (1);
}


#if !defined(CONFIG_NET_MULTI)
void eth_halt (void) {
	if (emac0_dev) {
		ppc_4xx_eth_halt(emac0_dev);
		free(emac0_dev);
		emac0_dev = NULL;
	}
}

int eth_init (bd_t *bis)
{
	ppc_4xx_eth_initialize(bis);
	if (emac0_dev) {
		return ppc_4xx_eth_init(emac0_dev, bis);
	} else {
		printf("ERROR: ethaddr not set!\n");
		return -1;
	}
}

int eth_send(volatile void *packet, int length)
{
	return (ppc_4xx_eth_send(emac0_dev, packet, length));
}

int eth_rx(void)
{
	return (ppc_4xx_eth_rx(emac0_dev));
}

int emac4xx_miiphy_initialize (bd_t * bis)
{
#if defined(CONFIG_MII) || (CONFIG_COMMANDS & CFG_CMD_MII)
	miiphy_register ("ppc_4xx_eth0",
			 emac4xx_miiphy_read, emac4xx_miiphy_write);
#endif

	return 0;
}
#endif /* !defined(CONFIG_NET_MULTI) */

#endif /* #if (CONFIG_COMMANDS & CFG_CMD_NET) */
