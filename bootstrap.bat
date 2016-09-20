mkdir build
mkdir build\deps
cd build\deps
cmake ..\..\deps -G "Visual Studio 14 2015 Win64" -DCMAKE_PREFIX_PATH=%cd%\local -DCMAKE_BUILD_TYPE=Release
cmake --build . --config Release
pause