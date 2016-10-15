REM MAke sure we are using the 64bit compilers
CALL "C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\vcvarsall.bat" amd64

mkdir build
mkdir build\deps
mkdir build\deps_d
cd build\deps_d
cmake ..\..\deps -G "NMake Makefiles" -DCMAKE_INSTALL_PREFIX=%cd%\..\local -DCMAKE_PREFIX_PATH=%cd%\..\local -DCMAKE_BUILD_TYPE=Debug -DBUILD_UI=ON
cmake --build .
cd ..\deps
cmake ..\..\deps -G "NMake Makefiles" -DCMAKE_INSTALL_PREFIX=%cd%\..\local -DCMAKE_PREFIX_PATH=%cd%\..\local -DCMAKE_BUILD_TYPE=Release -DBUILD_UI=ON
cmake --build .
cd ..\..\
pause