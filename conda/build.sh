#!/bin/bash

set -e

mkdir ${SRC_DIR}/build
mkdir ${SRC_DIR}/thirdparty
mkdir -p ${SRC_DIR}/thirdparty/local/share/cmake

cd ${SRC_DIR}/thirdparty

git clone --branch 5.2.1 --depth 1 https://github.com/fmtlib/fmt.git
git clone --branch v1.2.1 --depth 1 https://github.com/gabime/spdlog.git
git clone --branch v2.0.0 --depth 1 https://github.com/Microsoft/GSL.git
git clone --branch v2.4.1 --depth 1 https://github.com/HowardHinnant/date

cd ${SRC_DIR}/thirdparty/fmt
cmake -G Ninja -DCMAKE_BUILD_TYPE=Release -DFMT_TEST=OFF -DFMT_DOC=OFF -DCMAKE_INSTALL_PREFIX=../local
cmake --build . --target install -j 4

cd ${SRC_DIR}/thirdparty/spdlog
cmake -G Ninja -DCMAKE_BUILD_TYPE=Release -DSPDLOG_BUILD_BENCH=OFF -DSPDLOG_BUILD_EXAMPLES=OFF -DSPDLOG_BUILD_TESTING=OFF -DCMAKE_INSTALL_PREFIX=../local
cmake --build . --target install -j 4

cd ${SRC_DIR}/thirdparty/GSL
cmake -G Ninja -DCMAKE_BUILD_TYPE=Release -DGSL_TEST=OFF -DCMAKE_INSTALL_PREFIX=../local
cmake --build . --target install -j 4

cd ${SRC_DIR}/thirdparty/date
cmake -G Ninja -DCMAKE_BUILD_TYPE=Release -DUSE_SYSTEM_TZ_DB=ON -DENABLE_DATE_TESTING=OFF -DCMAKE_INSTALL_PREFIX=../local
cmake --build . --target install -j 4

cd ${SRC_DIR}/build

cmake \
    -G "${CMAKE_GENERATOR}" \
    -DPACKAGE_VERSION_COMMITHASH=${GIT_FULL_HASH} \
    -DCMAKE_INSTALL_PREFIX="${PREFIX}" \
    -DCMAKE_PREFIX_PATH="${SRC_DIR}/thirdparty/local;${PREFIX}" \
    -DCMAKE_MODULE_PATH="${RECIPE_DIR}/cmake" \
    -DGDX_DISABLE_OPENMP=OFF \
    -DGDX_ENABLE_TOOLS=OFF \
    -DGDX_ENABLE_TESTS=OFF \
    -DGDX_ENABLE_TEST_UTILS=OFF \
    -DGDX_PYTHON_BINDINGS=ON \
    -DINFRA_ENABLE_TESTS=OFF \
    -DCMAKE_BUILD_TYPE=Release \
    -DGDX_INSTALL_DEVELOPMENT_FILES=OFF \
    ${SRC_DIR}

cmake --build . --target install -j 4
