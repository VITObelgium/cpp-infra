function checkresult {
    "$@"
    local status=$?
    if [ $status -ne 0 ]; then
        echo "error with $@ status=$status" >&2
        exit $status
    fi
    return $status
}

export PATH="/tools/toolchains/x86_64-multilib-linux-musl/bin/:$PATH"

generator="Ninja"
# fall back to make if ninja is not installed
command -v ninja >/dev/null 2>&1 || { generator="Unix Makefiles"; }

config=""
toolchain=""
build_ui="OFF"

echo -n "Select configuration: [1:Debug 2:Release 3:Release with debug info]: "
read yno
case $yno in
    [1] ) config="Debug";;
    [2] ) config="Release";;
    [3] ) config="RelWithDebInfo";;
    * ) echo "Invalid selection" exit;;
esac

builddir="build/opaq_`echo "${config}" | tr '[:upper:]' '[:lower:]'`"
mkdir -p ${builddir}
cd ${builddir}
PWD=`pwd`

echo -n "Select toolchain to use: [1:Default 2:Musl (static linking) 3:Mingw 4: Mingw linux]: "
read yno
case $yno in
    [1] ) toolchain="";;
    [2] ) toolchain="${PWD}/../../deps/musl-linux.make";;
    [3] ) generator="MSYS Makefiles";;
    [4] ) toolchain="${PWD}/../../deps/mingw-linux.make";;
    * ) echo "Invalid selection" exit;;
esac

echo "Building configuration ${config} in ${builddir} toolchain (${toolchain})"

checkresult cmake ../.. -G "${generator}" -DCMAKE_PREFIX_PATH=${PWD}/../local -DCMAKE_TOOLCHAIN_FILE=${toolchain} -DCMAKE_BUILD_TYPE=${config} -DBUILD_UI=${build_ui} -DSTATIC_PLUGINS=ON
checkresult cmake --build .