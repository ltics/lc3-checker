#!/bin/bash
set +e
set -x

rm -rf build
mkdir build && cd build
cmake ..
make
make test
if [ ! -f "main" ]
then
    echo "File main does not exist"
    exit 1
fi
