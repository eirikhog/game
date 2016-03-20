@echo off

pushd .
cd %~dp0/../build
devenv win32.sln
popd
