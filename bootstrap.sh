#!/usr/bin/env bash
set -e

pwd=$(pwd)
platform=unknown
unamestr=$(uname)


if [[ "$unamestr" == 'Linux' ]]; then
   platform='linux'
elif [[ "$unamestr" == 'darwin' ]]; then
   platform='apple'
fi

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

cd ./deps/vcpkg && ./vcpkg-bootstrap.py --triplet=$triplet --ports-file=$pwd/deps/ports.txt
cd $pwd
