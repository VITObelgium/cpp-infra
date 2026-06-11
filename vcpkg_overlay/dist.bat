@echo off

FOR /F "tokens=* USEBACKQ" %%F IN (`"C:\Program Files (x86)\Microsoft Visual Studio\Installer\vswhere.exe" -latest -property InstallationPath`) DO (
SET vspath=%%F
)
set VSCMD_START_DIR=%1
echo "InstallationPath: %vspath%"
CALL "%vspath%\VC\Auxiliary\Build\vcvarsall.bat" x64

REM Run the command passed as remaining arguments
set "cmd="
:buildcmd
shift
if "%~1"=="" goto runcmd
if defined cmd (set "cmd=%cmd% %1") else (set "cmd=%1")
goto buildcmd

:runcmd
%cmd%
