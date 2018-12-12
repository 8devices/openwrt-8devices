#!/bin/sh
#
# Copyright (C) 2009-2018 OpenWrt.org
#

RTKMIPSEL_BOARD_NAME=
RTKMIPSEL_MODEL=

rtkmipsel_board_detect() {
	local machine
	local name

	machine=$(awk 'BEGIN{FS="[ \t]+:[ \t]"} /machine/ {print $2}' /proc/cpuinfo)

	case "$machine" in
	*"Kinkan"*)
		name="kinkan"
		;;
	*"Komikan"*)
		name="komikan"
		;;
	esac

	[ -z "$name" ] && name="unknown"

	[ -z "$RTKMIPSEL_BOARD_NAME" ] && RTKMIPSEL_BOARD_NAME="$name"
	[ -z "$RTKMIPSEL_MODEL" ] && RTKMIPSEL_MODEL="$machine"

	[ -e "/tmp/sysinfo/" ] || mkdir -p "/tmp/sysinfo/"

	echo "$RTKMIPSEL_BOARD_NAME" > /tmp/sysinfo/board_name
	echo "$RTKMIPSEL_MODEL" > /tmp/sysinfo/model
}
