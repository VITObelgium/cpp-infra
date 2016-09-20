function checkresult {
    "$@"
    local status=$?
    if [ $status -ne 0 ]; then
        echo "error with $@ status=$status" >&2
        exit $status
    fi
    return $status
}

mkdir -p build/opaq
cd build/opaq

PWD=`pwd`
checkresult cmake ../.. -GNinja -DCMAKE_PREFIX_PATH=${PWD}/../deps/local
checkresult cmake --build . --config Release