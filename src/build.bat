@echo off

set SCRIPTDIR=%~dp0
set BUILDDIR=%SCRIPTDIR%../build
set CFLAGS=/Od /MTd /nologo /Oi /Gm- /GR /Zo /EHa /FC /Z7 /W4 /wd4100 /wd4189 /wd4244 /wd4456 /wd4701 /wd4505 /wd4703 /DGLEW_STATIC
set LFLAGS=/incremental:no /opt:ref
set INCLUDES=-I../dep/glew-1.13.0/include

set WIN32SRC=../src/win32/win32.cpp
set GAMESRC=../src/Game.cpp
set ASSETSRC=../src/Assets/Builder.cpp
set TESTSRC=../src/Test.cpp

:SETUP
if not exist "%BUILDDIR%" mkdir "%BUILDDIR%"

pushd .
cd %BUILDDIR%

if not "%1" == "test" goto CLEAN
echo Building test...
cl %CFLAGS% %TESTSRC% %INCLUDES% /link %LFLAGS% user32.lib gdi32.lib winmm.lib
goto END

:CLEAN
if not "%1" == "clean" goto ASSETS
del *.pdb *.dll *.exe *.obj *.exp *.lib > NUL 2> NUL
goto END


:ASSETS
if not "%1" == "assets" if not "%1" == "all" goto WIN32
cl %CFLAGS% %ASSETSRC% %INCLUDES% /link %LFLAGS% user32.lib gdi32.lib winmm.lib
Builder.exe
if not "%1" == "all" goto END

:WIN32
if not "%1" == "win32" if not "%1" == "all" goto GAME
cl %CFLAGS% %WIN32SRC% %INCLUDES% /link /PDB:win32.pdb %LFLAGS% opengl32.lib user32.lib gdi32.lib winmm.lib xinput.lib
if not "%1" == "all" goto END

:GAME
del game_*.pdb > NUL 2> NUL
cl %CFLAGS% %GAMESRC% %INCLUDES% /LD /link /PDB:game_%random%.pdb %LFLAGS% opengl32.lib -EXPORT:UpdateGame
if not "%1" == "all" goto END

:END
popd

