function checkresult {
    "$@"
    local status=$?
    if [ $status -ne 0 ]; then
        echo "error with $@ status=$status" >&2
        exit $status
    fi
    return $status
}

generator="Ninja"
# fall back to make if ninja is not installed
command -v ninja >/dev/null 2>&1 || { generator="Unix Makefiles"; }

config="Debug"

echo -n "Select configuration: [1:Debug 2:Release 3:Release with debug info]: "
read yno
case $yno in
    [1] ) config="Debug";;
    [2] ) config="Release";;
    [3] ) config="RelWithDebInfo";;
    * ) echo "Invalid selection" exit;;
esac

builddir="build/opaq_musl"

mkdir -p ${builddir}
cd ${builddir}

echo "Building configuration ${config} in ${builddir}"

PWD=`pwd`
checkresult cmake ../.. -G ${generator} -DCMAKE_PREFIX_PATH=${PWD}/../local_musl -DSTATIC_BUILD=ON -DCMAKE_BUILD_TYPE=${config} -DCMAKE_TOOLCHAIN_FILE=${PWD}/../../deps/toolchain-musl.make -DBUILD_UI=OFF
checkresult cmake --build .
