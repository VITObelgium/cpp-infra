SET script_dir=%~dp0
SET build_dir=%source_dir%/build
SET thirdparty_dir=%source_dir%/thirdparty
SET thirdparty_install_dir=%thirdparty_dir%/local

mkdir %SRC_DIR%\build
mkdir %SRC_DIR%\thirdparty

cd %SRC_DIR%\thirdparty

git clone --branch 5.2.1 --depth 1 https://github.com/fmtlib/fmt.git
git clone --branch v1.2.1 --depth 1 https://github.com/gabime/spdlog.git
git clone --branch v2.0.0 --depth 1 https://github.com/Microsoft/GSL.git
git clone --branch v2.4.1 --depth 1 https://github.com/HowardHinnant/date
git clone --branch 3.3.7 --depth 1 https://github.com/eigenteam/eigen-git-mirror.git eigen

cd %SRC_DIR%/thirdparty/fmt
cmake -G Ninja -DCMAKE_BUILD_TYPE=Release -DFMT_TEST=OFF -DFMT_DOC=OFF -DCMAKE_INSTALL_PREFIX=../local
cmake --build . --target install

cd %SRC_DIR%/thirdparty/spdlog
cmake -G Ninja -DCMAKE_BUILD_TYPE=Release -DSPDLOG_BUILD_BENCH=OFF -DSPDLOG_BUILD_EXAMPLES=OFF -DSPDLOG_BUILD_TESTING=OFF -DCMAKE_INSTALL_PREFIX=../local
cmake --build . --target install

cd %SRC_DIR%/thirdparty/GSL
cmake -G Ninja -DCMAKE_BUILD_TYPE=Release -DGSL_TEST=OFF -DCMAKE_INSTALL_PREFIX=../local
cmake --build . --target install

cd %SRC_DIR%/thirdparty/date
cmake -G Ninja -DCMAKE_BUILD_TYPE=Release -DUSE_SYSTEM_TZ_DB=ON -DENABLE_DATE_TESTING=OFF -DCMAKE_INSTALL_PREFIX=../local
cmake --build . --target install

mkdir %SRC_DIR%\thirdparty\eigen\build
cd %SRC_DIR%\thirdparty\eigen\build
cmake -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=../local ..
cmake --build . --target install

cd %SRC_DIR%/build

cmake ^
    -G Ninja ^
    -DPACKAGE_VERSION_COMMITHASH=%GIT_FULL_HASH% ^
    -DCMAKE_INSTALL_PREFIX:PATH="%PREFIX%" ^
    -DCMAKE_PREFIX_PATH:PATH="%PREFIX%" ^
    -DCMAKE_FIND_ROOT_PATH:PATH="%SRC_DIR%\thirdparty\local" ^
    -DCMAKE_MODULE_PATH:PATH="%RECIPE_DIR%\cmake" ^
    -DGdal_DATA_PATH:PATH="%PREFIX%\Library\share\gdal" ^
    -Dfmt_DIR:PATH="%SRC_DIR%\thirdparty\local" ^
    -DGDX_DISABLE_OPENMP=OFF ^
    -DGDX_ENABLE_TOOLS=OFF ^
    -DGDX_ENABLE_TESTS=OFF ^
    -DGDX_ENABLE_TEST_UTILS=OFF ^
    -DGDX_PYTHON_BINDINGS=ON ^
    -DINFRA_ENABLE_TESTS=OFF ^
    -DCMAKE_BUILD_TYPE=Release ^
    -DGDX_INSTALL_DEVELOPMENT_FILES=OFF ^
    ..

cmake --build . --target install
