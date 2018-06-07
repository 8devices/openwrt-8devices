#
# Copyright (C) 2018 OpenWrt.org
#

. /lib/functions/system.sh
. /lib/rtkmipsel.sh

PART_NAME=firmware

get_magic_str() {
	(get_image "$@" | dd bs=4 count=1) 2>/dev/null
}

platform_check_image() {
	local signature

	[ "$#" -gt 1 ] && return 1

	signature=$(get_magic_str "$1")

	case "$signature" in
	cs6b|\
	cs6c|\
	csys|\
	cr6b|\
	cr6c|\
	csro)
		;;
	*)
		echo "Invalid image. Signature $signature not recognized."
		return 1;
		;;
	esac

	return 0;
}

platform_do_upgrade() {
	default_do_upgrade "$ARGV"
}
