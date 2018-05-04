REM Make sure we are using the 32bit compilers
SET projectdir=%cd%
CALL "C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\VC\Auxiliary\Build\vcvars64.bat"
cd %projectdir%

SET GENERATOR="NMake Makefiles"

jom /VERSION
IF %errorlevel%==0 (
    SET GENERATOR="NMake Makefiles JOM"
)

mkdir build
mkdir build\deps
mkdir build\deps_d

cd build\deps
cmake ..\..\deps ^
    -G %GENERATOR% ^
    -DCMAKE_TOOLCHAIN_FILE="%projectdir%\deps\cmake-scripts\toolchain-msvc.cmake" ^
    -DCMAKE_INSTALL_PREFIX="%cd%\..\local" ^
    -DCMAKE_PREFIX_PATH="%cd%\..\local" ^
    -D32BIT=OFF ^
    -DVS2017=ON ^
    -DCMAKE_BUILD_TYPE=Release

IF %errorlevel% NEQ 0 (
    ECHO Failed to perform Release configuration
    cd ..\..
    EXIT /b 1
)

cmake --build .
IF %errorlevel% NEQ 0 (
    ECHO Release build failed
    cd ..\..
    EXIT /b 1
)

cd ..\deps_d
cmake ..\..\deps ^
    -G %GENERATOR% ^
    -DCMAKE_TOOLCHAIN_FILE="%projectdir%\deps\cmake-scripts\toolchain-msvc.cmake" ^
    -DCMAKE_INSTALL_PREFIX="%cd%\..\local" ^
    -DCMAKE_PREFIX_PATH="%cd%\..\local" ^
    -D32BIT=OFF ^
    -DVS2017=ON ^
    -DCMAKE_BUILD_TYPE=Debug

IF %errorlevel% NEQ 0 (
    ECHO Failed to perform Debug configuration
    cd ..\..
    EXIT /b 1
)

cmake --build .
IF %errorlevel% NEQ 0 (
    ECHO Debug build failed
    cd ..\..
    EXIT /b 1
)

cd ..\..\

pause