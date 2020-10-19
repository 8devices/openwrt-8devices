/*
 * (C) Copyright 2000-2006
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

/*
 * CPU specific code for the MPC825x / MPC826x / MPC827x / MPC828x
 *
 * written or collected and sometimes rewritten by
 * Magnus Damm <damm@bitsmart.com>
 *
 * modified by
 * Wolfgang Denk <wd@denx.de>
 *
 * modified for 8260 by
 * Murray Jensen <Murray.Jensen@cmst.csiro.au>
 *
 * added 8260 masks by
 * Marius Groeger <mag@sysgo.de>
 *
 * added HiP7 (824x/827x/8280) processors support by
 * Yuli Barcohen <yuli@arabellasw.com>
 */

#include <common.h>
#include <watchdog.h>
#include <command.h>
#include <mpc8260.h>
#include <asm/processor.h>
#include <asm/cpm_8260.h>

DECLARE_GLOBAL_DATA_PTR;

int checkcpu (void)
{
	volatile immap_t *immap = (immap_t *) CFG_IMMR;
	ulong clock = gd->cpu_clk;
	uint pvr = get_pvr ();
	uint immr, rev, m, k;
	char buf[32];

	puts ("CPU:   ");

	switch (pvr) {
	case PVR_8260:
	case PVR_8260_HIP3:
		k = 3;
		break;
	case PVR_8260_HIP4:
		k = 4;
		break;
	case PVR_8260_HIP7R1:
	case PVR_8260_HIP7RA:
	case PVR_8260_HIP7:
		k = 7;
		break;
	default:
		return -1;	/* whoops! not an MPC8260 */
	}
	rev = pvr & 0xff;

	immr = immap->im_memctl.memc_immr;
	if ((immr & IMMR_ISB_MSK) != CFG_IMMR)
		return -1;	/* whoops! someone moved the IMMR */

	printf (CPU_ID_STR " (HiP%d Rev %02x, Mask ", k, rev);

	/*
	 * the bottom 16 bits of the immr are the Part Number and Mask Number
	 * (4-34); the 16 bits at PROFF_REVNUM (0x8af0) in dual port ram is the
	 * RISC Microcode Revision Number (13-10).
	 * For the 8260, Motorola doesn't include the Microcode Revision
	 * in the mask.
	 */
	m = immr & (IMMR_PARTNUM_MSK | IMMR_MASKNUM_MSK);
	k = *((ushort *) & immap->im_dprambase[PROFF_REVNUM]);

	switch (m) {
	case 0x0000:
		puts ("0.2 2J24M");
		break;
	case 0x0010:
		puts ("A.0 K22A");
		break;
	case 0x0011:
		puts ("A.1 1K22A-XC");
		break;
	case 0x0001:
		puts ("B.1 1K23A");
		break;
	case 0x0021:
		puts ("B.2 2K23A-XC");
		break;
	case 0x0023:
		puts ("B.3 3K23A");
		break;
	case 0x0024:
		puts ("C.2 6K23A");
		break;
	case 0x0060:
		puts ("A.0(A) 2K25A");
		break;
	case 0x0062:
		puts ("B.1 4K25A");
		break;
	case 0x0064:
		puts ("C.0 5K25A");
		break;
	case 0x0A00:
		puts ("0.0 0K49M");
		break;
	case 0x0A01:
		puts ("0.1 1K49M");
		break;
	case 0x0A10:
		puts ("1.0 1K49M");
		break;
	case 0x0C00:
		puts ("0.0 0K50M");
		break;
	case 0x0C10:
		puts ("1.0 1K50M");
		break;
	case 0x0D00:
		puts ("0.0 0K50M");
		break;
	case 0x0D10:
		puts ("1.0 1K50M");
		break;
	default:
		printf ("unknown [immr=0x%04x,k=0x%04x]", m, k);
		break;
	}

	printf (") at %s MHz\n", strmhz (buf, clock));

	return 0;
}

/* ------------------------------------------------------------------------- */
/* configures a UPM by writing into the UPM RAM array			     */
/* uses bank 11 and a dummy physical address (=BRx_BA_MSK)		     */
/* NOTE: the physical address chosen must not overlap into any other area    */
/* mapped by the memory controller because bank 11 has the lowest priority   */

void upmconfig (uint upm, uint * table, uint size)
{
	volatile immap_t *immap = (immap_t *) CFG_IMMR;
	volatile memctl8260_t *memctl = &immap->im_memctl;
	volatile uchar *dummy = (uchar *) BRx_BA_MSK;	/* set all BA bits */
	uint i;

	/* first set up bank 11 to reference the correct UPM at a dummy address */

	memctl->memc_or11 = ORxU_AM_MSK;	/* set all AM bits */

	switch (upm) {

	case UPMA:
		memctl->memc_br11 =
			((uint)dummy & BRx_BA_MSK) | BRx_PS_32 | BRx_MS_UPMA |
			BRx_V;
		memctl->memc_mamr = MxMR_OP_WARR;
		break;

	case UPMB:
		memctl->memc_br11 =
			((uint)dummy & BRx_BA_MSK) | BRx_PS_32 | BRx_MS_UPMB |
			BRx_V;
		memctl->memc_mbmr = MxMR_OP_WARR;
		break;

	case UPMC:
		memctl->memc_br11 =
			((uint)dummy & BRx_BA_MSK) | BRx_PS_32 | BRx_MS_UPMC |
			BRx_V;
		memctl->memc_mcmr = MxMR_OP_WARR;
		break;

	default:
		panic ("upmconfig passed invalid UPM number (%u)\n", upm);
		break;

	}

	/*
	 * at this point, the dummy address is set up to access the selected UPM,
	 * the MAD pointer is zero, and the MxMR OP is set for writing to RAM
	 *
	 * now we simply load the mdr with each word and poke the dummy address.
	 * the MAD is incremented on each access.
	 */

	for (i = 0; i < size; i++) {
		memctl->memc_mdr = table[i];
		*dummy = 0;
	}

	/* now kill bank 11 */
	memctl->memc_br11 = 0;
}

/* ------------------------------------------------------------------------- */

#if !defined(CONFIG_HAVE_OWN_RESET)
int
do_reset (cmd_tbl_t * cmdtp, int flag, int argc, char *argv[])
{
	ulong msr, addr;

	volatile immap_t *immap = (immap_t *) CFG_IMMR;

	immap->im_clkrst.car_rmr = RMR_CSRE;	/* Checkstop Reset enable */

	/* Interrupts and MMU off */
	__asm__ __volatile__ ("mfmsr    %0":"=r" (msr):);

	msr &= ~(MSR_ME | MSR_EE | MSR_IR | MSR_DR);
	__asm__ __volatile__ ("mtmsr    %0"::"r" (msr));

	/*
	 * Trying to execute the next instruction at a non-existing address
	 * should cause a machine check, resulting in reset
	 */
#ifdef CFG_RESET_ADDRESS
	addr = CFG_RESET_ADDRESS;
#else
	/*
	 * note: when CFG_MONITOR_BASE points to a RAM address, CFG_MONITOR_BASE
	 * - sizeof (ulong) is usually a valid address. Better pick an address
	 * known to be invalid on your system and assign it to CFG_RESET_ADDRESS.
	 */
	addr = CFG_MONITOR_BASE - sizeof (ulong);
#endif
	((void (*)(void)) addr) ();
	return 1;

}
#endif	/* CONFIG_HAVE_OWN_RESET */

/* ------------------------------------------------------------------------- */

/*
 * Get timebase clock frequency (like cpu_clk in Hz)
 *
 */
unsigned long get_tbclk (void)
{
	ulong tbclk;

	tbclk = (gd->bus_clk + 3L) / 4L;

	return (tbclk);
}

/* ------------------------------------------------------------------------- */

#if defined(CONFIG_WATCHDOG)
void watchdog_reset (void)
{
	int re_enable = disable_interrupts ();

	reset_8260_watchdog ((immap_t *) CFG_IMMR);
	if (re_enable)
		enable_interrupts ();
}
#endif /* CONFIG_WATCHDOG */

/* ------------------------------------------------------------------------- */
