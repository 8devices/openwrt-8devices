#!/bin/bash

help(){
    echo "Usage: $0 [board-name]"
    echo "valid board names: kinkan kinkan_full"
    exit
}

if [[ $# -lt 1 ]]
then
    config=config_kinkan_minimal
else
    case $1 in
        kinkan)
            config=config_kinkan_minimal
            ;;
        kinkan_otbr)
            config=config_kinkan_otbr
            ;;
        kinkan_full)
            config=config_kinkan_full
            ;;
        help|--help|-h|*)
            help
            ;;
    esac
fi

echo "Building image using config: '$config'"

./scripts/feeds update -a
./scripts/feeds install -a
cp .config .config_save`date +%F_%T.%N`
cp $config .config
make defconfig
make -j4 IGNORE_ERRORS='m n y'
