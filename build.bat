mkdir build
mkdir build\opaq
cd build\opaq
cmake ..\.. -G "Visual Studio 14 2015 Win64" -DCMAKE_PREFIX_PATH=%cd%\..\local -DBUILD_UI=ON
cmake --build . --config Debug
pause