@echo off

rem TODO: Avoid hardcoding paths..
rem For now, we assume a working drive w:

set BUILDDIR=w:/game/build
set CFLAGS=/Od /MTd /nologo /Oi /EHa /W4 /wd4100 /wd4189 /wd4244 /wd4456 /wd4701 /wd4505 /wd4703 /DGLEW_STATIC
set LFLAGS=
set INCLUDES=-I../dep/glew-1.13.0/include

set WIN32SRC=../src/win32/win32.cpp
set GAMESRC=../src/Game.cpp
set ASSETSRC=../src/Assets/Builder.cpp

:SETUP
if not exist "%BUILDDIR%" mkdir "%BUILDDIR%"

pushd .
cd %BUILDDIR%

rem Cleanup from previous build\
del *.pdb > NUL 2> NUL

:ASSETS
if not "%1" == "assets" if not "%1" == "all" goto WIN32
cl %CFLAGS% %ASSETSRC% %INCLUDES% /link %LFLAGS% user32.lib gdi32.lib winmm.lib
Builder.exe
if not "%1" == "all" goto END

:WIN32
if not "%1" == "win32" if not "%1" == "all" goto GAME
cl %CFLAGS% %WIN32SRC% %INCLUDES% /link %LFLAGS% opengl32.lib user32.lib gdi32.lib winmm.lib xinput.lib
if not "%1" == "all" goto END

:GAME
cl %CFLAGS% %GAMESRC% %INCLUDES% /LD /link %LFLAGS% opengl32.lib -EXPORT:UpdateGame
if not "%1" == "all" goto END

:END
popd

