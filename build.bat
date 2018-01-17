SET projectdir=%cd%
CALL "C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\VC\Auxiliary\Build\vcvars64.bat"
cd %projectdir%

SET prefix_dir=%projectdir%\build\local
IF EXIST %projectdir%\..\deps\msvc-dynamic-x64\local (
    SET prefix_dir=%projectdir%\..\deps\msvc-dynamic-x64\local
)

mkdir build
mkdir build\opaq
cd build\opaq

cmake  ^
    -G "Visual Studio 15 2017" ^
    -DCMAKE_GENERATOR_PLATFORM=x64 ^
    -DCMAKE_TOOLCHAIN_FILE="%projectdir%\deps\cmake-scripts\toolchain-msvc-dynamic-runtime.cmake" ^
    -DCMAKE_INSTALL_PREFIX="%prefix_dir%" ^
    -DCMAKE_PREFIX_PATH="%prefix_dir%;C:\Qt\5.10.0\msvc2017_64" ^
    -DBUILD_UI=ON ^
    -DSTATIC_QT=OFF ^
    -DSTATIC_PLUGINS=OFF ^
    ..\..
cmake --build . --config Debug
pause