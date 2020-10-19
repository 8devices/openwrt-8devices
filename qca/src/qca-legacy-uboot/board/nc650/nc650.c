/*
 * (C) Copyright 2001
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

#include <common.h>
#include <config.h>
#include <mpc8xx.h>

/*
 *  Memory Controller Using
 *
 *  CS0 - Flash memory            (0x40000000)
 *  CS3 - SDRAM                   (0x00000000}
 */

/* ------------------------------------------------------------------------- */

#define _not_used_	0xffffffff

const uint sdram_table[] = {
	/* single read. (offset 0 in upm RAM) */
	0x1f07fc04, 0xeeaefc04, 0x11adfc04, 0xefbbbc00,
	0x1ff77c47,

	/* MRS initialization (offset 5) */

	0x1ff77c34, 0xefeabc34, 0x1fb57c35,

	/* burst read. (offset 8 in upm RAM) */
	0x1f07fc04, 0xeeaefc04, 0x10adfc04, 0xf0affc00,
	0xf0affc00, 0xf1affc00, 0xefbbbc00, 0x1ff77c47,
	_not_used_, _not_used_, _not_used_, _not_used_,
	_not_used_, _not_used_, _not_used_, _not_used_,

	/* single write. (offset 18 in upm RAM) */
	0x1f27fc04, 0xeeaebc00, 0x01b93c04, 0x1ff77c47,
	_not_used_, _not_used_, _not_used_, _not_used_,

	/* burst write. (offset 20 in upm RAM) */
	0x1f07fc04, 0xeeaebc00, 0x10ad7c00, 0xf0affc00,
	0xf0affc00, 0xe1bbbc04, 0x1ff77c47, _not_used_,
	_not_used_, _not_used_, _not_used_, _not_used_,
	_not_used_, _not_used_, _not_used_, _not_used_,

	/* refresh. (offset 30 in upm RAM) */
	0x1ff5fc84, 0xfffffc04, 0xfffffc04, 0xfffffc04,
	0xfffffc84, 0xfffffc07, _not_used_, _not_used_,
	_not_used_, _not_used_, _not_used_, _not_used_,

	/* exception. (offset 3c in upm RAM) */
	0x7ffffc07, _not_used_, _not_used_, _not_used_
};

const uint nand_flash_table[] = {
	/* single read. (offset 0 in upm RAM) */
	0x0ff3fc04, 0x0ff3fc04, 0x0ff3fc04, 0x0ffffc04,
	0xfffffc00, 0xfffffc05, 0xfffffc05, 0xfffffc05,

	/* burst read. (offset 8 in upm RAM) */
	0xffffcc05, 0xffffcc05, 0xffffcc05, 0xffffcc05,
	0xffffcc05, 0xffffcc05, 0xffffcc05, 0xffffcc05,
	0xffffcc05, 0xffffcc05, 0xffffcc05, 0xffffcc05,
	0xffffcc05, 0xffffcc05, 0xffffcc05, 0xffffcc05,

	/* single write. (offset 18 in upm RAM) */
	0x00fffc04, 0x00fffc04, 0x00fffc04, 0x0ffffc04,
	0x0ffffc84, 0x0ffffc84, 0xfffffc00, 0xfffffc05,

	/* burst write. (offset 20 in upm RAM) */
	0xffffcc05, 0xffffcc05, 0xffffcc05, 0xffffcc05,
	0xffffcc05, 0xffffcc05, 0xffffcc05, 0xffffcc05,
	0xffffcc05, 0xffffcc05, 0xffffcc05, 0xffffcc05,
	0xffffcc05, 0xffffcc05, 0xffffcc05, 0xffffcc05,

	/* refresh. (offset 30 in upm RAM) */
	0xffffcc05, 0xffffcc05, 0xffffcc05, 0xffffcc05,
	0xffffcc05, 0xffffcc05, 0xffffcc05, 0xffffcc05,
	0xffffcc05, 0xffffcc05, 0xffffcc05, 0xffffcc05,

	/* exception. (offset 3c in upm RAM) */
	0xffffcc05, 0xffffcc05, 0xffffcc05, 0xffffcc05
};

/* ------------------------------------------------------------------------- */

/*
 * Check Board Identity:
 */

int checkboard (void)
{
	puts ("Board: NC650\n");
	return 0;
}

/* ------------------------------------------------------------------------- */

static long int dram_size (long int, long int *, long int);

/* ------------------------------------------------------------------------- */

long int initdram (int board_type)
{
	volatile immap_t *immap = (immap_t *) CFG_IMMR;
	volatile memctl8xx_t *memctl = &immap->im_memctl;
	long int size8, size9;
	long int size_b0 = 0;
	unsigned long reg;

	upmconfig (UPMA, (uint *) sdram_table,
			   sizeof (sdram_table) / sizeof (uint));

	/*
	 * Preliminary prescaler for refresh (depends on number of
	 * banks): This value is selected for four cycles every 62.4 us
	 * with two SDRAM banks or four cycles every 31.2 us with one
	 * bank. It will be adjusted after memory sizing.
	 */
	memctl->memc_mptpr = CFG_MPTPR_2BK_8K;

	memctl->memc_mar = 0x00000088;

	/*
	 * Map controller bank 1 to the SDRAM bank at
	 * preliminary address - these have to be modified after the
	 * SDRAM size has been determined.
	 */
	memctl->memc_or3 = CFG_OR3_PRELIM;
	memctl->memc_br3 = CFG_BR3_PRELIM;

	memctl->memc_mamr = CFG_MAMR_8COL & (~(MAMR_PTAE));	/* no refresh yet */

	udelay (200);

	/* perform SDRAM initializsation sequence */

	memctl->memc_mcr = 0x80006105;	/* SDRAM bank 0 */
	udelay (200);
	memctl->memc_mcr = 0x80006230;	/* SDRAM bank 0 - execute twice */
	udelay (200);

	memctl->memc_mamr |= MAMR_PTAE;	/* enable refresh */

	udelay (1000);

	/*
	 * Check Bank 0 Memory Size for re-configuration
	 *
	 * try 8 column mode
	 */
	size8 = dram_size (CFG_MAMR_8COL, (ulong *) SDRAM_BASE3_PRELIM,
					   SDRAM_MAX_SIZE);

	udelay (1000);

	/*
	 * try 9 column mode
	 */
	size9 = dram_size (CFG_MAMR_9COL, (ulong *) SDRAM_BASE3_PRELIM,
					  SDRAM_MAX_SIZE);

	udelay (1000);

	if (size8 < size9) {
		size_b0 = size9;
	} else {
		size_b0 = size8;
		memctl->memc_mamr = CFG_MAMR_8COL;
		udelay (500);
	}

	/*
	 * Adjust refresh rate depending on SDRAM type, both banks.
	 * For types > 128 MBit leave it at the current (fast) rate
	 */
	if ((size_b0 < 0x02000000)) {
		/* reduce to 15.6 us (62.4 us / quad) */
		memctl->memc_mptpr = CFG_MPTPR_2BK_4K;
		udelay (1000);
	}

	/*
	 * Final mapping
	 */

	memctl->memc_or3 = ((-size_b0) & 0xFFFF0000) | CFG_OR_TIMING_SDRAM;
	memctl->memc_br3 = (CFG_SDRAM_BASE & BR_BA_MSK) | BR_MS_UPMA | BR_V;

	/* adjust refresh rate depending on SDRAM type, one bank */
	reg = memctl->memc_mptpr;
	reg >>= 1;					/* reduce to CFG_MPTPR_1BK_8K / _4K */
	memctl->memc_mptpr = reg;

	udelay (10000);

	/* Configure UPMB for NAND flash access */
	upmconfig (UPMB, (uint *) nand_flash_table,
			   sizeof (nand_flash_table) / sizeof (uint));

	memctl->memc_mbmr = CFG_MBMR_NAND;

	return (size_b0);
}

/* ------------------------------------------------------------------------- */

/*
 * Check memory range for valid RAM. A simple memory test determines
 * the actually available RAM size between addresses `base' and
 * `base + maxsize'. Some (not all) hardware errors are detected:
 * - short between address lines
 * - short between data lines
 */

static long int dram_size (long int mamr_value, long int *base, long int maxsize)
{
	volatile immap_t *immap = (immap_t *) CFG_IMMR;
	volatile memctl8xx_t *memctl = &immap->im_memctl;

	memctl->memc_mamr = mamr_value;

	return (get_ram_size(base, maxsize));
}

#if (CONFIG_COMMANDS & CFG_CMD_NAND)
void nand_init(void)
{
	extern unsigned long nand_probe(unsigned long physadr);

	unsigned long totlen = nand_probe(CFG_NAND_BASE);

	printf ("%4lu MB\n", totlen >> 20);
}
#endif
