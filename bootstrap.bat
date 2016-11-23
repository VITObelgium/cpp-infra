REM Make sure we are using the 64bit compilers
CALL "C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\vcvarsall.bat" amd64

SET GENERATOR="NMake Makefiles"

jom /VERSION
IF %errorlevel%==0 (
    SET GENERATOR="NMake Makefiles JOM"
)

mkdir build
mkdir build\deps
mkdir build\deps_d

cd build\deps
cmake ..\..\deps -G %GENERATOR% -DCMAKE_TOOLCHAIN_FILE=%cd%\..\..\deps\msvc.make -DCMAKE_INSTALL_PREFIX=%cd%\..\local -DCMAKE_PREFIX_PATH=%cd%\..\local -DCMAKE_BUILD_TYPE=Release -DBUILD_UI=ON
IF %errorlevel% NEQ 0 (
    ECHO Failed to perform Release configuration
    EXIT /b 1
)

cmake --build .
IF %errorlevel% NEQ 0 (
    ECHO Release build failed
    EXIT /b 1
)

cd ..\deps_d
cmake ..\..\deps -G %GENERATOR% -DCMAKE_TOOLCHAIN_FILE=%cd%\..\..\deps\msvc.make -DCMAKE_INSTALL_PREFIX=%cd%\..\local -DCMAKE_PREFIX_PATH=%cd%\..\local -DCMAKE_BUILD_TYPE=Debug -DBUILD_UI=ON
IF %errorlevel% NEQ 0 (
    ECHO Failed to perform Debug configuration
    EXIT /b 1
)

cmake --build .
IF %errorlevel% NEQ 0 (
    ECHO Debug build failed
    EXIT /b 1
)

cd ..\..\

REM Patch the windows integration plugin to avoid linker errors on release mode
copy /Y %cd%\deps\qt\Qt5Gui_QWindowsIntegrationPlugin.cmake %cd%\build\local\lib\cmake\Qt5Gui\

pause