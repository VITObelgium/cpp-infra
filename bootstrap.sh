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

mkdir -p build/deps
cd build/deps

PWD=`pwd`

config=""
toolchain=""
generator="Unix Makefiles"
build_ui="OFF"

echo -n "Select configuration: [1:Debug 2:Release]: "
read yno
case $yno in
    [1] ) config="Debug";;
    [2] ) config="Release";;
    * ) echo "Invalid selection" exit;;
esac

echo -n "Select toolchain to use: [1:Default 2:Musl (static linking) 3:Mingw 4:Mingw linux]: "
read yno
case $yno in
    [1] ) toolchain="${PWD}/../../deps/toolchain-cluster.cmake";;
    [2] ) toolchain="${PWD}/../../deps/toolchain-musl.cmake";;
    [3] ) toolchain="${PWD}/../../deps/toolchain-native.cmake" build_ui="ON";;
    [4] ) toolchain="${PWD}/../../deps/toolchain-mingw-cross.cmake";;
    * ) echo "Invalid selection" exit;;
esac

checkresult cmake -G "${generator}" ../../deps -DCMAKE_INSTALL_PREFIX=${PWD}/../local -DCMAKE_TOOLCHAIN_FILE=${toolchain} -DCMAKE_PREFIX_PATH=${PWD}/../local -DCMAKE_BUILD_TYPE=${config} -DBUILD_UI=${build_ui}
checkresult cmake --build .

cd ..
# overwrite invalid cmake files to avoid configuration errors
if [ "${config}" = "Debug" ]; then
    cp ./local/share/cmake/hdf5-targets-debug.cmake ./local/share/cmake/hdf5-targets-release.cmake
else
    cp ./local/share/cmake/hdf5-targets-release.cmake ./local/share/cmake/hdf5-targets-debug.cmake
fi

