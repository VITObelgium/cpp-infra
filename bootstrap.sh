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

checkresult cmake ../../deps -DCMAKE_PREFIX_PATH=${PWD}/local -DCMAKE_BUILD_TYPE=Release
checkresult cmake --build . --config Release
