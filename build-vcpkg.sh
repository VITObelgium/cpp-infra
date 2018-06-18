#/usr/bin/env bash
set -e

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

pwd=$(pwd)

printf "Select triplet to use:\n1:default\n2:x64-cluster\n3:x64-musl\n4:x64-mingw\n"
read yno
case $yno in
    [1] )
        triplet="x64-osx"
        toolchain="${pwd}/deps/vcpkg/scripts/toolchains/osx.cmake"
        ;;
    [2] ) triplet="x64-cluster";;
    [3] ) triplet="x64-musl";;
    [4] ) triplet="x64-mingw";;
    * ) echo "Invalid selection" exit;;
esac

builddir="build"
projectdir="opaq-${triplet}"
mkdir -p ${builddir}/${projectdir}
cd ${builddir}/${projectdir}

cmake ../.. -G ${generator} \
    -DCMAKE_TOOLCHAIN_FILE=${pwd}/deps/vcpkg/scripts/buildsystems/vcpkg.cmake \
    -DVCPKG_TARGET_TRIPLET=${triplet} \
    -DVCPKG_LINKER_FLAGS="-stdlib=libc++" \
    -DVCPKG_CHAINLOAD_TOOLCHAIN_FILE=${toolchain} \
    -DCMAKE_BUILD_TYPE=${config}

cmake --build .
