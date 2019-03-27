#!/bin/bash

CONFIGS=$(ls | awk '/config_/{sub(/config_/,""); print}')

help(){
    echo "Usage: $0 [board-name]"
    echo "valid config names:"
    echo  "$CONFIGS"
    exit
}

if [[ $# -lt 1 ]]
then
    config=config_kinkan_minimal
else
    echo "$CONFIGS" | grep "^$1\$" > /dev/null && config=config_$1 ||\
    help
fi

echo "Building image using config: '$config'"

./scripts/feeds update -a
./scripts/feeds install -a
cp .config .config_save`date +%F_%T.%N`
cp $config .config
make defconfig
make -j4 IGNORE_ERRORS='m n y'
