#!/bin/sh
#
docker run -h wrt_builder -v $PWD:/home/baker/${PWD##*/} --rm -it wrt_baker:latest /bin/bash \
 -c 'cd /home/baker/wrt_baker; \
