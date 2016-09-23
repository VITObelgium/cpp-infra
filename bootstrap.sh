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

mkdir -p build/deps
cd build/deps

PWD=`pwd`
checkresult cmake ../../deps -DCMAKE_INSTALL_PREFIX=${PWD}/../local -DCMAKE_PREFIX_PATH=${PWD}/../local -DCMAKE_BUILD_TYPE=${config}
checkresult cmake --build .

cd ..
if [ "${config}" = "Debug" ]; then
    # rename hdf lib suffixes so debug libraries have the same name as release libraries
    mv ./local/lib/libhdf5_cpp_debug.a ./local/lib/libhdf5_cpp.a
    mv ./local/lib/libhdf5_debug.a ./local/lib/libhdf5.a
fi