# WRT Baker

## Build WRT Baker docker image
Simple way to build:

    ./build.sh

Or manualy:

    docker build -t wrt_baker:latest ./

if want to have same uid/gid (same as in build.sh)

    docker build --build-arg UID=$(id -u) --build-arg GID=$(id -g) -t wrt_baker:latest ./

## Compile code example
For compiling go to source dir and user usual commands

    cd $SRC
    docker run -h wrt_builder -v $PWD:/home/baker/${PWD##*/} --rm -it wrt_baker:latest /bin/bash -c "cd /home/baker/${PWD##*/}; exec make -j4"

## Interactive mode:
For running in interactive mode

    docker run -h wrt_builder -v $PWD:/home/baker/${PWD##*/} --rm -it wrt_baker:latest

## Running
For running you can source this script:

    wb () {
        PARAMS=""
        [[ -n $1 ]] && PARAMS="-c \"$@\""
        eval "docker run -h wrt_builder -v $PWD:/home/baker/${PWD##*/} --rm -it wrt_baker:latest /bin/bash -c 'cd /home/baker/${PWD##*/}; exec /bin/bash $PARAMS'"
    }


And then just:

    wb 'make -j4'

NOTE. everything in comma's will run inside docker eg:

    ➜  github git:(openwrt-18.06-rtkmipsel-3.18) ✗ wb 'whoami && whoami' && whoami
    baker
    baker
    rytis
