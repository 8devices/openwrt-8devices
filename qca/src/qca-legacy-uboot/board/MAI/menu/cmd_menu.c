#include <common.h>
#include <command.h>

int do_menu( cmd_tbl_t *cmdtp, /*bd_t *bd,*/ int flag, int argc, char *argv[] )
{
/*	printf("<NOT YET IMPLEMENTED>\n"); */
	return 0;
}

#if defined(CONFIG_AMIGAONEG3SE) && (CONFIG_COMMANDS & CFG_CMD_BSP)
U_BOOT_CMD(
	menu,   1,      1,      do_menu,
	"menu    - display BIOS setup menu\n",
	""
);
#endif
