mkdir build
mkdir build\opaq
cd build\opaq
cmake ..\.. -G "Visual Studio 14 2015 Win64" -DCMAKE_TOOLCHAIN_FILE=%cd%\..\..\deps\msvc.make -DCMAKE_PREFIX_PATH=%cd%\..\local -DBUILD_UI=ON -DSTATIC_PLUGINS=ON
cmake --build . --config Debug
pause