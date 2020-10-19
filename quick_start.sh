#!/bin/bash


TTY=$(tty)

CLEAN="no"
COMPILE="no"
CONFIG="undefined"
DRYRUN="no"
INTERACTIVE="yes"
UPDATE_FEEDS="yes"

CONFIG_DIR="configs"
CONFIGS=$(ls $CONFIG_DIR)

select_config(){
	i=1
	echo Please select config: > $TTY

	for config in $CONFIGS; do
		echo $i: ${config#config_} > $TTY
		i=$(($i + 1))
	done

	read NR
	CONFIG=$(echo "$CONFIGS" |sed "${NR}q;d")
}

update_feeds(){
	[[ -n "$FEEDS" ]] &&\
		$RUN "cp $FEEDS feeds.conf"

	$RUN "rm tmp -rf"
	$RUN "./scripts/feeds update -a"
	$RUN "./scripts/feeds install -a -f"
}

update_config(){
	[ -f ".config" ] &&\
		$RUN "cp .config .config_backup"

	$RUN "cp $CONFIG_DIR/$CONFIG .config"
}

compile(){
	$RUN "make defconfig"
	$RUN "make -j4"
}

clean(){
		$RUN "make clean"
}

para_parsing(){
	[[ -z "$1" ]] && return 0
	case $1 in
		-C|--clean)
			CLEAN="yes"
			;;
		-c|--compile)
			COMPILE="yes"
			;;
		-d|--dry-run)
			DRYRUN="yes"
			;;
		-i|--interactive)
			INTERACTIVE="yes"
			;;
		-h|--help)
			help
			exit 0
			;;
		-n|--no-feeds-update)
			UPDATE_FEEDS="no"
			;;
		-l|--list-configs)
			echo "${CONFIGS}"
			exit 0
			;;
		--)
			;;
		*)
			CONFIG="$(echo "$CONFIGS" | grep "$1" | head)" ||\
				(echo "Config \"$1\" not found" && exit 1)
			;;
	esac
	shift 1
	para_parsing $@
}

help(){
	echo "Usage: $0 <config> [-Ccdhin] [--OPTION1 [--OPTION2...]]"
	echo "	Options:"
	echo "	-C|--clean"
	echo "	-c|--compile"
	echo "	-d|--dry-run"
	echo "	-h|--help"
	echo "	-i|--interactive"
	echo "	-n|--no-feeds-update"
	echo "	-l|--list-configs"
}

set -e
set -o pipefail

if [[ -n "$1" ]] ; then
	INTERACTIVE="no"
	PARAMS=$(getopt -u -l "clean,compile,dryrun,help,interactive,no-feeds-update,list-configs" -o "Ccdhinl" -- "$@") || exit 1
	para_parsing $PARAMS
fi

[[ "$DRYRUN" == "yes" ]] && RUN="echo" || RUN="eval"
[[ "$INTERACTIVE" == "yes" ]] && select_config
[[ "$CLEAN" == "yes" ]] && clean
[[ "$UPDATE_FEEDS" == "yes" ]] && update_feeds
[[ "$CONFIG" != "undefined" ]] && update_config
[[ "$COMPILE" == "yes" ]] && compile
exit 0
