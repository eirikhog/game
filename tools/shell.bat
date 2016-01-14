@echo off

echo Setting up environment...

call "%programfiles(x86)%\Microsoft Visual Studio 14.0\VC\vcvarsall.bat"
set PATH=%~dp0;%PATH%

echo Done!

