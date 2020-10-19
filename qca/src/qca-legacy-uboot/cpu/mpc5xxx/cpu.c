/*
 * (C) Copyright 2000-2003
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
 * CPU specific code for the MPC5xxx CPUs
 */

#include <common.h>
#include <watchdog.h>
#include <command.h>
#include <mpc5xxx.h>
#include <asm/processor.h>

DECLARE_GLOBAL_DATA_PTR;

int checkcpu (void)
{
	ulong clock = gd->cpu_clk;
	char buf[32];
#ifndef CONFIG_MGT5100
	uint svr, pvr;
#endif

	puts ("CPU:   ");

#ifdef CONFIG_MGT5100
	puts   (CPU_ID_STR);
	printf (" (JTAG ID %08lx)", *(vu_long *)MPC5XXX_CDM_JTAGID);
#else
	svr = get_svr();
	pvr = get_pvr();
	switch (SVR_VER (svr)) {
	case SVR_MPC5200:
		printf ("MPC5200");
		break;
	default:
		printf ("MPC52??  (SVR %08x)", svr);
		break;
	}

	printf (" v%d.%d, Core v%d.%d", SVR_MJREV (svr), SVR_MNREV (svr), 
		PVR_MAJ(pvr), PVR_MIN(pvr));
#endif
	printf (" at %s MHz\n", strmhz (buf, clock));
	return 0;
}

/* ------------------------------------------------------------------------- */

int
do_reset (cmd_tbl_t * cmdtp, int flag, int argc, char *argv[])
{
	ulong msr;
	/* Interrupts and MMU off */
	__asm__ __volatile__ ("mfmsr    %0":"=r" (msr):);

	msr &= ~(MSR_ME | MSR_EE | MSR_IR | MSR_DR);
	__asm__ __volatile__ ("mtmsr    %0"::"r" (msr));

	/* Charge the watchdog timer */
	*(vu_long *)(MPC5XXX_GPT0_COUNTER) = 0x0001000f;
	*(vu_long *)(MPC5XXX_GPT0_ENABLE) = 0x9004; /* wden|ce|timer_ms */
	while(1);

	return 1;

}

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
