#!/bin/bash

set -ex

LIBMRUBY=../mruby/build/host/lib/libmruby.a
LIBS="-lmysqlclient -lGeoIP"
MRUBY_INCLUDE_DIR=../mruby/include/

PDNS_SRC_DIR=../build/pdns-src
PDNS_BUILD_DIR=${PDNS_SRC_DIR}
PDNS_INSTALL_DIR=/usr/local/pdns

( rm *.so *.o || true )

gcc -g -fPIC -I. -I${MRUBY_INCLUDE_DIR} -c pdns_mruby_core.cc
g++ -g -fPIC -I. -I${MRUBY_INCLUDE_DIR} -I${PDNS_BUILD_DIR} -I${PDNS_SRC_DIR} -c mrubybackend.cc

g++ -g -shared -fPIC -o libmrubybackend.so pdns_mruby_core.o mrubybackend.o ${LIBMRUBY} $LIBS

# sudo cp libmrubybackend.so ${PDNS_INSTALL_DIR}/lib/pdns/
# cp ../pdns-mruby.conf.sample ${PDNS_INSTALL_DIR}/etc/pdns.conf
# sudo ${PDNS_INSTALL_DIR}/sbin/pdns_server

