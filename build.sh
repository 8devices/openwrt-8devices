#!/bin/bash
#
# OpenWrt for 8devices boards build helper
#
help(){
    echo "Usage: $0 [board-name] <config_full>"
    echo "valid board names: carambola2 carambola3 centipede lima rambutan"
    exit
}

if [ $# -eq 0 ]; then
    help
fi

if [ $# -eq 1 ]; then
    case $1 in
        rambutan)
            config=config_rambutan_minimal
            ;;
        carambola2|carambola3|centipede|lima)
            config=config_minimal
            ;;
        help|--help|-h|*)
            help
            ;;
    esac
else
    case $1 in
        rambutan)
            config=config_rambutan_full
            ;;
        carambola2|carambola3|centipede|lima)
            config=config_full
            ;;
    esac
fi

echo "Building image using config: '$config'"

./scripts/feeds update -a
./scripts/feeds install -a

cp .config .config_save`date +%F_%T.%N`
cp $config .config
make defconfig
make
