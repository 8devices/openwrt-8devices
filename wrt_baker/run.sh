#!/bin/bash
#
export LC_ALL=C

BASEURL=https://github.com/8devices
REPO=openwrt-8devices
REPO_URL="$BASEURL/$REPO"
RUNDSH=run_dock.sh
BRANCH_AR7="openwrt-19.07-ar71xx-4.14"
BRANCH_IPQ="openwrt-19.07-ipq40xx-4.14"
BR=""
REPODIR=""

cat run.tpl >$RUNDSH

if [ -d "$REPO" ]; then
    cd $REPO
    BR=$(git branch | awk '{print $2}')
    cd ..
fi

TARGET=$(whiptail --title "Choose target for building" --radiolist \
"What is the target?" 15 60 3 \
"ar71xx-generic" "Carambola2 Centipede Lima" ON \
"ar71xx-nand"  "Rambutan" OFF \
"ipq40xx" "Jalapeno Habanero-DVK" OFF 3>&1 1>&2 2>&3)

exitstatus=$?

if [ $exitstatus = 0 ]; then
    case "$TARGET" in 
	ar71xx-generic)
	    if [ "$BR" != "$BRANCH_AR7" ]; then
		REPODIR=$REPO-ar71xx
		echo "  git clone $REPO_URL -b $BRANCH_AR7 $REPODIR; \\" >>$RUNDSH
	    fi
#
# Choose `lima'. In fact it's equal to carambola2 | centipede | lima
	    BOARD="lima"
	    ;;
	ar71xx-nand)
	    if [ "$BR" != "$BRANCH_AR7" ]; then
		REPODIR=$REPO-ar71xx
		echo "  git clone $REPO_URL -b $BRANCH_AR7 $REPODIR; \\" >>$RUNDSH
	    fi
	    BOARD="rambutan"
	    ;;
	ipq40xx)
	    if [ "$BR" != "$BRANCH_IPQ" ]; then
		REPODIR=$REPO-ipq40xx
		echo "  git clone $REPO_URL -b $BRANCH_IPQ $REPODIR; \\" >>$RUNDSH
	    fi
#
# Empty BOARD string equals to jalapeno | habanero
	    BOARD=""
	    ;;
    esac
else
    clear
    exit
fi

echo "  cd $REPODIR; \\" >>$RUNDSH

CONFIG=$(whiptail --title "Choose config for building" --radiolist \
"What is the config?" 15 60 2 \
"minimal" "Minimal" ON \
"full"  "Full" OFF 3>&1 1>&2 2>&3)

exitstatus=$?

if [ $exitstatus = 0 ]; then
    case "$TARGET" in 
	ar71xx-generic|ar71xx-nand)
	    if [ "$CONFIG" = "minimal" ]; then
		CONFIG=""
	    fi
    	    echo "chmod 755 build.sh; ./build.sh $BOARD $CONFIG" >b.sh
	    ;;
	ipq40xx)
	    echo "chmod 755 build.sh; ./build.sh $BOARD 8dev_ipq40xx_$CONFIG" >b.sh
	    ;;
    esac

    chmod 755 b.sh
    echo "mv ../b.sh .;  exec ./b.sh'" >>$RUNDSH
    chmod 755 $RUNDSH
    clear
    exec ./$RUNDSH
else
    rm -f $RUNDSH >/dev/null 2>&1
    clear
    exit
fi
