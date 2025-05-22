@echo off

REM Print the first argument of the script

FOR /F "tokens=* USEBACKQ" %%F IN (`"C:\Program Files (x86)\Microsoft Visual Studio\Installer\vswhere.exe" -latest -property InstallationPath`) DO (
SET vspath=%%F
)
set VSCMD_START_DIR=%1
CALL "%vspath%\VC\Auxiliary\Build\vcvarsall.bat" x64
just build_dist %2