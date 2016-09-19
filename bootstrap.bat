mkdir build
mkdir build\deps
cd build\deps
cmake ..\..\deps -DCMAKE_PREFIX_PATH=%cd%\local -DCMAKE_BUILD_TYPE=Release
cmake --build . --config Release
pause