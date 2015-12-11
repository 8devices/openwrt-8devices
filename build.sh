#!/bin/bash

./scripts/feeds update -a
./scripts/feeds install -a
cp .config .config_save`date +%F_%T.%N`
cp config_minimal .config
make defconfig
make -j4 IGNORE_ERRORS=m
