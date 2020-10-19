/*
 * (C) Copyright 2003
 * Josef Baumgartner <josef.baumgartner@telex.de>
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
#include <watchdog.h>
#include <command.h>

#ifdef	CONFIG_M5272
#include <asm/immap_5272.h>
#include <asm/m5272.h>
#endif

#ifdef	CONFIG_M5282

#endif

#ifdef	CONFIG_M5249
#include <asm/m5249.h>
#endif


#ifdef	CONFIG_M5272
int do_reset (cmd_tbl_t *cmdtp, bd_t *bd, int flag, int argc, char *argv[]) {
	volatile wdog_t * wdp = (wdog_t *)(CFG_MBAR + MCFSIM_WRRR);

	wdp->wdog_wrrr = 0;
	udelay (1000);

	/* enable watchdog, set timeout to 0 and wait */
	wdp->wdog_wrrr = 1;
	while (1);

	/* we don't return! */
	return 0;
};

int checkcpu(void) {
	ulong *dirp = (ulong *)(CFG_MBAR + MCFSIM_DIR);
	uchar msk;
	char  *suf;

	puts ("CPU:   ");
	msk = (*dirp > 28) & 0xf;
	switch (msk) {
		case 0x2: suf = "1K75N"; break;
		case 0x4: suf = "3K75N"; break;
		default:
			suf = NULL;
			printf ("MOTOROLA MCF5272 (Mask:%01x)\n", msk);
			break;
		}

	if (suf)
		printf ("MOTOROLA MCF5272 %s\n", suf);
	return 0;
};


#if defined(CONFIG_WATCHDOG)
/* Called by macro WATCHDOG_RESET */
void watchdog_reset (void)
{
	volatile immap_t * regp = (volatile immap_t *)CFG_MBAR;
	regp->wdog_reg.wdog_wcr = 0;
}

int watchdog_disable (void)
{
	volatile immap_t *regp = (volatile immap_t *)CFG_MBAR;

	regp->wdog_reg.wdog_wcr = 0;	/* reset watchdog counter */
	regp->wdog_reg.wdog_wirr = 0;	/* disable watchdog interrupt */
	regp->wdog_reg.wdog_wrrr = 0;	/* disable watchdog timer */

	puts ("WATCHDOG:disabled\n");
	return (0);
}

int watchdog_init (void)
{
	volatile immap_t *regp = (volatile immap_t *)CFG_MBAR;

	regp->wdog_reg.wdog_wirr = 0;	/* disable watchdog interrupt */

	/* set timeout and enable watchdog */
	regp->wdog_reg.wdog_wrrr = ((CONFIG_WATCHDOG_TIMEOUT * CFG_HZ) / (32768 * 1000)) - 1;
	regp->wdog_reg.wdog_wcr = 0;	/* reset watchdog counter */

	puts ("WATCHDOG:enabled\n");
	return (0);
}
#endif /* #ifdef CONFIG_WATCHDOG */

#endif /* #ifdef CONFIG_M5272 */


#ifdef	CONFIG_M5282
int checkcpu (void)
{
	puts ("CPU:   MOTOROLA Coldfire MCF5282\n");
	return 0;
}

int do_reset (cmd_tbl_t *cmdtp, bd_t *bd, int flag, int argc, char *argv[]) {
	return 0;
};
#endif

#ifdef CONFIG_M5249 /* test-only: todo... */
int checkcpu (void)
{
	char buf[32];

	printf ("CPU:   MOTOROLA Coldfire MCF5249 at %s MHz\n", strmhz(buf, CFG_CLK));
	return 0;
}

int do_reset (cmd_tbl_t *cmdtp, bd_t *bd, int flag, int argc, char *argv[]) {
	/* enable watchdog, set timeout to 0 and wait */
	mbar_writeByte(MCFSIM_SYPCR, 0xc0);
	while (1);

	/* we don't return! */
	return 0;
};
#endif
