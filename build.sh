function checkresult {
    "$@"
    local status=$?
    if [ $status -ne 0 ]; then
        echo "error with $@ status=$status" >&2
        exit $status
    fi
    return $status
}

platform='unknown'
unamestr=`uname`
if [[ "$unamestr" == 'Linux' ]]; then
   platform='linux'
elif [[ "$unamestr" == 'darwin' ]]; then
   platform='apple'
fi

emscripten=0
generator="Ninja"
# fall back to make if ninja is not installed
command -v ninja >/dev/null 2>&1 || { generator="Unix Makefiles"; }

printf "Select configuration: [1:Debug 2:Release]: "
read yno
case $yno in
    [1] ) config="Debug";;
    [2] ) config="Release";;
    * ) echo "Invalid selection" exit;;
esac

printf "Select toolchain to use:\n1:Default\n2:Musl (static linking)\n3:Mingw\n4:Gcc6\n5:Gcc7\n6:Gcc8\n7:Clang\n8:emscripten\n"
read yno
case $yno in
    [1] ) toolchain="toolchain-cluster";;
    [2] ) toolchain="toolchain-musl";;
    [3] ) toolchain="toolchain-mingw"
        if [[ "$platform" == "linux" ]]; then
            toolchain="toolchain-mingw-cross"
        else
            toolchain="toolchain-mingw"
        fi
        ;;
    [4] ) toolchain="toolchain-gcc6";;
    [5] ) toolchain="toolchain-gcc7";;
    [6] ) toolchain="toolchain-gcc8";;
    [7] )
        if [[ "$platform" == "linux" ]]; then
            toolchain="toolchain-cluster-clang"
        elif [[ $platform == MINGW64* ]]; then
            toolchain="toolchain-mingw-clang"
        else
            toolchain="toolchain-clang"
        fi
        ;;
    [8] ) toolchain="toolchain-wasm"; emscripten=1;;
    * ) echo "Invalid selection" exit;;
esac

pwd=$(pwd)
printf "Platform $platform\n"
printf "Platform Using toolchain: ${pwd}/deps/cmake-scripts/${toolchain}.cmake\n"

builddir="build"
projectdir="opaq-"$(echo ${toolchain} | cut -d'-' -f 2,3)
mkdir -p ${builddir}/${projectdir}
cd ${builddir}/${projectdir}

if [ ${emscripten} -eq 0 ];
then
checkresult cmake ../.. -G ${generator} \
    -DCMAKE_PREFIX_PATH=${pwd}/build/local-${toolchain}/ \
    -DCMAKE_TOOLCHAIN_FILE=${pwd}/deps/cmake-scripts/${toolchain}.cmake \
    -DCMAKE_BUILD_TYPE=${config}
else
checkresult emcmake cmake ../.. -G ${generator} \
    -DCMAKE_PREFIX_PATH=${pwd}/build/local-${toolchain}/ \
    -DCMAKE_FIND_ROOT_PATH=${pwd}/build/local-${toolchain} \
    -DCMAKE_BUILD_TYPE=${config}
fi


checkresult cmake --build .
