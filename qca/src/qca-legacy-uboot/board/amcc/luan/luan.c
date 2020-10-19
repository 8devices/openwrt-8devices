/*
 * (C) Copyright 2005
 * John Otken, jotken@softadvances.com
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>
#include <command.h>
#include <ppc4xx.h>
#include <asm/processor.h>
#include <spd_sdram.h>
#include "epld.h"

DECLARE_GLOBAL_DATA_PTR;

extern flash_info_t flash_info[CFG_MAX_FLASH_BANKS]; /* info for FLASH chips */


/*************************************************************************
 *  int board_early_init_f()
 *
 ************************************************************************/
int board_early_init_f(void)
{
	volatile epld_t *x = (epld_t *) CFG_EPLD_BASE;

	mtebc( pb0ap,  0x03800000 );	/* set chip selects */
	mtebc( pb0cr,  0xffc58000 );	/* ebc0_b0cr, 4MB at 0xffc00000 CS0 */
	mtebc( pb1ap,  0x03800000 );
	mtebc( pb1cr,  0xff018000 );	/* ebc0_b1cr, 1MB at 0xff000000 CS1 */
	mtebc( pb2ap,  0x03800000 );
	mtebc( pb2cr,  0xff838000 );	/* ebc0_b2cr, 2MB at 0xff800000 CS2 */

	mtdcr( uic1sr, 0xffffffff );	/* Clear all interrupts */
	mtdcr( uic1er, 0x00000000 );	/* disable all interrupts */
	mtdcr( uic1cr, 0x00000000 );	/* Set Critical / Non Critical interrupts */
	mtdcr( uic1pr, 0x7fff83ff );	/* Set Interrupt Polarities */
	mtdcr( uic1tr, 0x001f8000 );	/* Set Interrupt Trigger Levels */
	mtdcr( uic1vr, 0x00000001 );	/* Set Vect base=0,INT31 Highest priority */
	mtdcr( uic1sr, 0x00000000 );	/* clear all interrupts */
	mtdcr( uic1sr, 0xffffffff );

	mtdcr( uic0sr, 0xffffffff );	/* Clear all interrupts */
	mtdcr( uic0er, 0x00000000 );	/* disable all interrupts excepted cascade */
	mtdcr( uic0cr, 0x00000001 );	/* Set Critical / Non Critical interrupts */
	mtdcr( uic0pr, 0xffffffff );	/* Set Interrupt Polarities */
	mtdcr( uic0tr, 0x01000004 );	/* Set Interrupt Trigger Levels */
	mtdcr( uic0vr, 0x00000001 );	/* Set Vect base=0,INT31 Highest priority */
	mtdcr( uic0sr, 0x00000000 );	/* clear all interrupts */
	mtdcr( uic0sr, 0xffffffff );

	x->ethuart &= ~EPLD2_RESET_ETH_N; /* put Ethernet+PHY in reset */

	return  0;
}


/*************************************************************************
 *  int misc_init_r()
 *
 ************************************************************************/
int misc_init_r(void)
{
	volatile epld_t *x = (epld_t *) CFG_EPLD_BASE;
	x->ethuart |= EPLD2_RESET_ETH_N; /* take Ethernet+PHY out of reset */

	return  0;
}


/*************************************************************************
 *  int checkboard()
 *
 ************************************************************************/
int checkboard(void)
{
	char *s = getenv("serial#");

	printf("Board: Luan - AMCC PPC440SP Evaluation Board");

	if (s != NULL) {
		puts(", serial# ");
		puts(s);
	}
	putc('\n');

	return  0;
}


/*************************************************************************
 *  long int fixed_sdram()
 *
 ************************************************************************/
static long int fixed_sdram(void)
{					/* DDR2 init from BDI2000 script */
	mtdcr( 0x10, 0x00000021 );	/* MCIF0_MCOPT2 - zero DCEN bit */
	mtdcr( 0x11, 0x84000000 );
	mtdcr( 0x10, 0x00000020 );	/* MCIF0_MCOPT1 - no ECC, 64 bits, 4 banks, DDR2 */
	mtdcr( 0x11, 0x2D122000 );
	mtdcr( 0x10, 0x00000026 );	/* MCIF0_CODT  - die termination on */
	mtdcr( 0x11, 0x00800026 );
	mtdcr( 0x10, 0x00000081 );	/* MCIF0_WRDTR - Write DQS Adv 90 + Fractional DQS Delay */
	mtdcr( 0x11, 0x82000800 );
	mtdcr( 0x10, 0x00000080 );	/* MCIF0_CLKTR - advance addr clock by 180 deg */
	mtdcr( 0x11, 0x80000000 );
	mtdcr( 0x10, 0x00000040 );	/* MCIF0_MB0CF - turn on CS0, N x 10 coll */
	mtdcr( 0x11, 0x00000201 );
	mtdcr( 0x10, 0x00000044 );	/* MCIF0_MB1CF - turn on CS0, N x 10 coll */
	mtdcr( 0x11, 0x00000201 );
	mtdcr( 0x10, 0x00000030 );	/* MCIF0_RTR   - refresh every 7.8125uS */
	mtdcr( 0x11, 0x08200000 );
	mtdcr( 0x10, 0x00000085 );	/* MCIF0_SDTR1 - timing register 1 */
	mtdcr( 0x11, 0x80201000 );
	mtdcr( 0x10, 0x00000086 );	/* MCIF0_SDTR2 - timing register 2 */
	mtdcr( 0x11, 0x42103242 );
	mtdcr( 0x10, 0x00000087 );	/* MCIF0_SDTR3 - timing register 3 */
	mtdcr( 0x11, 0x0C100D14 );
	mtdcr( 0x10, 0x00000088 );	/* MCIF0_MMODE - CAS is 4 cycles */
	mtdcr( 0x11, 0x00000642 );
	mtdcr( 0x10, 0x00000089 );	/* MCIF0_MEMODE - diff DQS disabled */
	mtdcr( 0x11, 0x00000400 );	/*		  ODT term disabled */

	mtdcr( 0x10, 0x00000050 );	/* MCIF0_INITPLR0 - NOP */
	mtdcr( 0x11, 0x81b80000 );
	mtdcr( 0x10, 0x00000051 );	/* MCIF0_INITPLR1 - PRE */
	mtdcr( 0x11, 0x82100400 );
	mtdcr( 0x10, 0x00000052 );	/* MCIF0_INITPLR2 - EMR2 */
	mtdcr( 0x11, 0x80820000 );
	mtdcr( 0x10, 0x00000053 );	/* MCIF0_INITPLR3 - EMR3 */
	mtdcr( 0x11, 0x80830000 );
	mtdcr( 0x10, 0x00000054 );	/* MCIF0_INITPLR4 - EMR DLL ENABLE */
	mtdcr( 0x11, 0x80810000 );
	mtdcr( 0x10, 0x00000055 );	/* MCIF0_INITPLR5 - MR DLL RESET */
	mtdcr( 0x11, 0x80800542 );
	mtdcr( 0x10, 0x00000056 );	/* MCIF0_INITPLR6 - PRE */
	mtdcr( 0x11, 0x82100400 );
	mtdcr( 0x10, 0x00000057 );	/* MCIF0_INITPLR7 - refresh */
	mtdcr( 0x11, 0x99080000 );
	mtdcr( 0x10, 0x00000058 );	/* MCIF0_INITPLR8 */
	mtdcr( 0x11, 0x99080000 );
	mtdcr( 0x10, 0x00000059 );	/* MCIF0_INITPLR9 */
	mtdcr( 0x11, 0x99080000 );
	mtdcr( 0x10, 0x0000005A );	/* MCIF0_INITPLR10 */
	mtdcr( 0x11, 0x99080000 );
	mtdcr( 0x10, 0x0000005B );	/* MCIF0_INITPLR11 - MR */
	mtdcr( 0x11, 0x80800442 );
	mtdcr( 0x10, 0x0000005C );	/* MCIF0_INITPLR12 - EMR OCD Default */
	mtdcr( 0x11, 0x80810380 );
	mtdcr( 0x10, 0x0000005D );	/* MCIF0_INITPLR13 - EMR OCD exit */
	mtdcr( 0x11, 0x80810000 );
	udelay( 10*1000 );

	mtdcr( 0x10, 0x00000021 );	/* MCIF0_MCOPT2 - execute preloaded init */
	mtdcr( 0x11, 0x28000000 );	/*		  set DC_EN */
	udelay( 100*1000 );

	mtdcr( 0x40, 0x0000F800 );	/* MQ0_B0BAS: base addr 00000000 / 256MB */
	mtdcr( 0x41, 0x1000F800 );	/* MQ0_B1BAS: base addr 10000000 / 256MB */

	mtdcr( 0x10, 0x00000078 );	/* MCIF0_RDCC - auto set read stage */
	mtdcr( 0x11, 0x00000000 );
	mtdcr( 0x10, 0x00000070 );	/* MCIF0_RQDC - read DQS delay control */
	mtdcr( 0x11, 0x8000003A );	/*		enabled, frac DQS delay */
	mtdcr( 0x10, 0x00000074 );	/* MCIF0_RFDC - two clock feedback delay */
	mtdcr( 0x11, 0x00000200 );

	return  512 << 20;
}


/*************************************************************************
 *  long int initdram
 *
 ************************************************************************/
long int initdram( int board_type )
{
	long dram_size = 0;

#if defined(CONFIG_SPD_EEPROM)
	dram_size = spd_sdram (0);
#else
	dram_size = fixed_sdram ();
#endif

	return  dram_size;
}


/*************************************************************************
 *  int testdram()
 *
 ************************************************************************/
#if defined(CFG_DRAM_TEST)
int testdram(void)
{
	unsigned long *mem = (unsigned long *) 0;
	const unsigned long kend = (1024 / sizeof(unsigned long));
	unsigned long k, n;

	mtmsr(0);

	for (k = 0; k < CFG_KBYTES_SDRAM;
	     ++k, mem += (1024 / sizeof(unsigned long))) {
		if ((k & 1023) == 0) {
			printf("%3d MB\r", k / 1024);
		}

		memset(mem, 0xaaaaaaaa, 1024);
		for (n = 0; n < kend; ++n) {
			if (mem[n] != 0xaaaaaaaa) {
				printf("SDRAM test fails at: %08x\n",
				       (uint) & mem[n]);
				return 1;
			}
		}

		memset(mem, 0x55555555, 1024);
		for (n = 0; n < kend; ++n) {
			if (mem[n] != 0x55555555) {
				printf("SDRAM test fails at: %08x\n",
				       (uint) & mem[n]);
				return 1;
			}
		}
	}
	printf("SDRAM test passes\n");

	return  0;
}
#endif


/*************************************************************************
 *  pci_pre_init
 *
 *  This routine is called just prior to registering the hose and gives
 *  the board the opportunity to check things. Returning a value of zero
 *  indicates that things are bad & PCI initialization should be aborted.
 *
 *	Different boards may wish to customize the pci controller structure
 *	(add regions, override default access routines, etc) or perform
 *	certain pre-initialization actions.
 *
 ************************************************************************/
#if defined(CONFIG_PCI) && defined(CFG_PCI_PRE_INIT)
int pci_pre_init( struct pci_controller *hose )
{
	unsigned long strap;

	/*--------------------------------------------------------------------------+
	 *	The luan board is always configured as the host & requires the
	 *	PCI arbiter to be enabled.
	 *--------------------------------------------------------------------------*/
	mfsdr(sdr_sdstp1, strap);
	if( (strap & SDR0_SDSTP1_PAE_MASK) == 0 ) {
		printf("PCI: SDR0_STRP1[%08lX] - PCI Arbiter disabled.\n",strap);

		return  0;
	}

	return  1;
}
#endif /* defined(CONFIG_PCI) && defined(CFG_PCI_PRE_INIT) */


/*************************************************************************
 *  pci_target_init
 *
 *	The bootstrap configuration provides default settings for the pci
 *	inbound map (PIM). But the bootstrap config choices are limited and
 *	may not be sufficient for a given board.
 *
 ************************************************************************/
#if defined(CONFIG_PCI) && defined(CFG_PCI_TARGET_INIT)
void pci_target_init(struct pci_controller *hose)
{
	/*--------------------------------------------------------------------------+
	 * Disable everything
	 *--------------------------------------------------------------------------*/
	out32r( PCIX0_PIM0SA, 0 ); /* disable */
	out32r( PCIX0_PIM1SA, 0 ); /* disable */
	out32r( PCIX0_PIM2SA, 0 ); /* disable */
	out32r( PCIX0_EROMBA, 0 ); /* disable expansion rom */

	/*--------------------------------------------------------------------------+
	 * Map all of SDRAM to PCI address 0x0000_0000. Note that the 440 strapping
	 * options to not support sizes such as 128/256 MB.
	 *--------------------------------------------------------------------------*/
	out32r( PCIX0_PIM0LAL, CFG_SDRAM_BASE );
	out32r( PCIX0_PIM0LAH, 0 );
	out32r( PCIX0_PIM0SA, ~(gd->ram_size - 1) | 1 );

	out32r( PCIX0_BAR0, 0 );

	/*--------------------------------------------------------------------------+
	 * Program the board's subsystem id/vendor id
	 *--------------------------------------------------------------------------*/
	out16r( PCIX0_SBSYSVID, CFG_PCI_SUBSYS_VENDORID );
	out16r( PCIX0_SBSYSID, CFG_PCI_SUBSYS_DEVICEID );

	out16r( PCIX0_CMD, in16r(PCIX0_CMD) | PCI_COMMAND_MEMORY );
}
#endif /* defined(CONFIG_PCI) && defined(CFG_PCI_TARGET_INIT) */


/*************************************************************************
 *  is_pci_host
 *
 *	This routine is called to determine if a pci scan should be
 *	performed. With various hardware environments (especially cPCI and
 *	PPMC) it's insufficient to depend on the state of the arbiter enable
 *	bit in the strap register, or generic host/adapter assumptions.
 *
 *	Rather than hard-code a bad assumption in the general 440 code, the
 *	440 pci code requires the board to decide at runtime.
 *
 *	Return 0 for adapter mode, non-zero for host (monarch) mode.
 *
 *
 ************************************************************************/
#if defined(CONFIG_PCI)
int is_pci_host(struct pci_controller *hose)
{
	return  1;
}
#endif				/* defined(CONFIG_PCI) */


/*************************************************************************
 *  hw_watchdog_reset
 *
 *	This routine is called to reset (keep alive) the watchdog timer
 *
 ************************************************************************/
#if defined(CONFIG_HW_WATCHDOG)
void hw_watchdog_reset(void)
{
}
#endif


/*************************************************************************
 *  int on_off()
 *
 ************************************************************************/
static int on_off( const char *s )
{
	if (strcmp(s, "on") == 0) {
		return  1;
	} else if (strcmp(s, "off") == 0) {
		return  0;
	}
	return  -1;
}


/*************************************************************************
 *  void l2cache_disable()
 *
 ************************************************************************/
static void l2cache_disable(void)
{
	mtdcr( l2_cache_cfg, 0 );
}


/*************************************************************************
 *  void l2cache_enable()
 *
 ************************************************************************/
static void l2cache_enable(void)	/* see p258 7.4.1 Enabling L2 Cache */
{
	mtdcr( l2_cache_cfg, 0x80000000 );	/* enable L2_MODE L2_CFG[L2M] */

	mtdcr( l2_cache_addr, 0 );		/* set L2_ADDR with all zeros */

	mtdcr( l2_cache_cmd, 0x80000000 );	/* issue HCLEAR command via L2_CMD */

	while (!(mfdcr( l2_cache_stat ) & 0x80000000 ))  ;; /* poll L2_SR for completion */

	mtdcr( l2_cache_cmd, 0x10000000 );	/* clear cache errors L2_CMD[CCP] */

	mtdcr( l2_cache_cmd, 0x08000000 );	/* clear tag errors L2_CMD[CTE] */

	mtdcr( l2_cache_snp0, 0 );		/* snoop registers */
	mtdcr( l2_cache_snp1, 0 );

	__asm__ volatile ("sync");		/* msync */

	mtdcr( l2_cache_cfg, 0xe0000000 );	/* inst and data use L2 */

	__asm__ volatile ("sync");
}


/*************************************************************************
 *  int l2cache_status()
 *
 ************************************************************************/
static int l2cache_status(void)
{
	return  (mfdcr( l2_cache_cfg ) & 0x60000000) != 0;
}


/*************************************************************************
 *  int do_l2cache()
 *
 ************************************************************************/
int do_l2cache( cmd_tbl_t *cmdtp, int flag, int argc, char *argv[] )
{
	switch (argc) {
	case 2:			/* on / off	*/
		switch (on_off(argv[1])) {
		case 0:	l2cache_disable();
			break;
		case 1:	l2cache_enable();
			break;
		}
		/* FALL TROUGH */
	case 1:			/* get status */
		printf ("L2 Cache is %s\n",
			l2cache_status() ? "ON" : "OFF");
		return 0;
	default:
		printf ("Usage:\n%s\n", cmdtp->usage);
		return 1;
	}

	return  0;
}


U_BOOT_CMD(
	l2cache,   2,   1,     do_l2cache,
	"l2cache  - enable or disable L2 cache\n",
	"[on, off]\n"
	"    - enable or disable L2 cache\n"
	);
