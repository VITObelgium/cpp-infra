mkdir build
mkdir build\opaq
cd build\opaq

cmake  ^
    -G "Visual Studio 15 2017" ^
    -DCMAKE_GENERATOR_PLATFORM=x64 ^
    -DCMAKE_TOOLCHAIN_FILE="%cd%\..\..\deps\cmake-scripts\toolchain-msvc-dynamic-runtime.cmake" ^
    -DCMAKE_INSTALL_PREFIX="%cd%\..\local" ^
    -DCMAKE_PREFIX_PATH="%cd%\..\local" ^
    -DBUILD_UI=ON ^
    -DSTATIC_QT=ON ^
    -DSTATIC_PLUGINS=ON ^
    ..\..
cmake --build . --config Debug
pause