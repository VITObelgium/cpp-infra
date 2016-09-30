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
# overwrite invalid cmake files to avoid configuration errors
if [ "${config}" = "Debug" ]; then
    cp ./local/share/cmake/hdf5-targets-debug.cmake ./local/share/cmake/hdf5-targets-release.cmake
else
    cp ./local/share/cmake/hdf5-targets-release.cmake ./local/share/cmake/hdf5-targets-debug.cmake
fi

