@echo off

echo Setting up environment...

set MAKEPATH=c:\MinGW\msys\1.0\bin

call "%programfiles(x86)%\Microsoft Visual Studio 14.0\VC\vcvarsall.bat"
set PATH=%~dp0;%MAKEPATH%;%PATH%

echo Done!

