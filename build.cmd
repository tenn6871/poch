@echo off
setlocal
if "%MODTOPIA_INCLUDE%"=="" set "MODTOPIA_INCLUDE=%~dp0sdk\include"
if "%MODTOPIA_INCLUDE%"=="" (
    echo Set MODTOPIA_INCLUDE to the Modtopia SDK include directory.
    exit /b 1
)
if not exist build\bin\windows-x64 mkdir build\bin\windows-x64
cl /nologo /LD /O2 /std:c11 /utf-8 /W4 /WX /I "%MODTOPIA_INCLUDE%" /Fe:build\bin\windows-x64\mod.dll mod.c /link /NOLOGO
if errorlevel 1 exit /b 1
python tools\pack.py --platform windows-x64
