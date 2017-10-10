#!/bin/bash
set +e
set -x

rm -rf build
mkdir build && cd build
cmake ..
make
make test
if [ $? != 0 ] || [ ! -f "main" ] ; then
    exit 1
fi
