#!/bin/bash

help(){
    echo "Usage: $0 [board-name]"
    echo "valid board names: carambola2 centipede lima rambutan"
    exit
}

config=
feeds=

if [[ $# -lt 1 ]]
then
    config=config_minimal
else
    case $1 in
        rambutan)
            config=config_rambutan_minimal
            ;;
        carambola2|centipede|lima)
            config=config_minimal
            if [ -n "$KEFIR" ]; then
                config=config_minimal.kefir
                feeds=feeds.conf.kefir
            fi
            ;;
        help|--help|-h|*)
            help
            ;;
    esac
fi

[ -n "$feeds" ] && cp $feeds feeds.conf

echo "Building image using config: '$config'"

./scripts/feeds update -a
./scripts/feeds install -a
cp .config .config_save`date +%F_%T.%N`
cp $config .config
make defconfig
make -j4 IGNORE_ERRORS=m
