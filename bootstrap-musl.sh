#!/bin/bash

function checkresult {
    "$@"
    local status=$?
    if [ $status -ne 0 ]; then
        echo "error with $@ status=$status" >&2
        exit $status
    fi
    return $status
}

config=""

echo -n "Select configuration: [1:Debug 2:Release]: "
read yno
case $yno in
    [1] ) config="Debug";;
    [2] ) config="Release";;
    * ) echo "Invalid selection" exit;;
esac

mkdir -p build/deps_musl
cd build/deps_musl

PWD=`pwd`
checkresult cmake ../../deps -DCMAKE_INSTALL_PREFIX=${PWD}/../local_musl -DCMAKE_PREFIX_PATH=${PWD}/../local_musl -DCMAKE_TOOLCHAIN_FILE=${PWD}/../../deps/toolchain-musl.make -DCMAKE_BUILD_TYPE=${config}
checkresult cmake --build .

