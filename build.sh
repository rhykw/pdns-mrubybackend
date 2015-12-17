#!/bin/sh

set -e

. ./pdns_version

PREFIX=$HOME/opt/pdns

if [ "$PDNS_CONFIG_OPT_ENV" != "" ]; then
    PDNS_CONFIG_OPT=$PDNS_CONFIG_OPT_ENV
else
    PDNS_CONFIG_OPT='--prefix='$PREFIX
fi

if [ ! -d "./mruby/src" ]; then
    echo "mruby Downloading ..."
    git submodule init
    git submodule update
    echo "mruby Downloading ... Done"
fi
cd mruby
if [ -d "./build" ]; then
    echo "mruby Cleaning ..."
    ./minirake clean
    echo "mruby Cleaning ... Done"
fi
cd ..

if [ $PDNS_SRC_ENV ]; then
    PDNS_SRC=$PDNS_SRC_ENV
else
    echo "powedns Downloading ..."
    if [ -d "./build" ]; then
        echo "build directory was found"
    else
        mkdir build
    fi
    cd build
    if [ ! -e ${PDNS_SRC_VER} ]; then
        wget https://downloads.powerdns.com/releases/${PDNS_SRC_VER}.tar.bz2
        echo "powerdns Downloading ... Done"
        tar --bzip2 -xf ${PDNS_SRC_VER}.tar.bz2
    fi
    cd ..

fi

echo "mruby building ..."
cp build_config.rb mruby
(cd mruby && ./minirake)
echo "mruby building ... Done"

echo "pdns configure ..."
( cd build/${PDNS_SRC_VER} && ./configure ${PDNS_CONFIG_OPT} )
echo "pdns configure ... Done"

echo "pdns building ... "
( cd build/${PDNS_SRC_VER} && make && make install )
echo "pdns building ... Done"

echo "mrubybackend building ..."
( cd src && ./minibuild.sh )
cp src/libmrubybackend.so $PREFIX/lib/pdns/
echo "mrubybackend building ... Done"


echo "build.sh ... successful"

