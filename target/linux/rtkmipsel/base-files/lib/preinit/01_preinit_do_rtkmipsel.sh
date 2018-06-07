#!/bin/sh

do_rtkmipsel_board_detection() {
	. /lib/rtkmipsel.sh

	rtkmipsel_board_detect
}

boot_hook_add preinit_main do_rtkmipsel_board_detection
