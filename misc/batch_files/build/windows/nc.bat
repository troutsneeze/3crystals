@echo off
pushd .
setlocal

set TARGET="x"
set CFG_FLAGS=
set STEAMWORKS_FLAGS=
set DEMO_FLAGS=

:beginloop
if "%1"=="debug" goto debug
if "%1"=="release" goto endloop rem not supported in this script
if "%1"=="demo" goto demo
if "%1"=="steam" goto steam
if "%1"=="t" goto tgui6_flag
if "%1"=="s" goto shim4_flag
if "%1"=="g" goto crystals_flag
if "%1"=="d" goto data_flag
goto doneloop
:debug
set CFG_FLAGS="-DDEBUG=on"
goto endloop
:steam
set STEAMWORKS_FLAGS="-DSTEAMWORKS=on"
goto endloop
:demo
set DEMO_FLAGS="-DDEMO=on"
goto endloop
:tgui6_flag
set TARGET="t"
goto endloop
:shim4_flag
set TARGET="s"
goto endloop
:crystals_flag
set TARGET="g"
goto endloop
:data_flag
set TARGET="d"
goto endloop
:endloop
shift
goto beginloop
:doneloop

if %TARGET%=="t" goto tgui6
if %TARGET%=="s" goto shim4
if %TARGET%=="g" goto crystals
if %TARGET%=="d" goto data

echo Invalid target: %TARGET%
goto done

:tgui6
del c:\users\trent\code\t\tgui6.dll
cd c:\users\trent\code\tgui6
rmdir /s /q build
mkdir build
cd build
c:\users\trent\code\3crystals\misc\batch_files\cmake\windows\tgui6.bat %CFG_FLAGS% %STEAMWORKS_FLAGS%
goto done
:shim4
del c:\users\trent\code\t\shim4.dll
cd c:\users\trent\code\shim4
rmdir /s /q build
mkdir build
cd build
c:\users\trent\code\3crystals\misc\batch_files\cmake\windows\shim4.bat %CFG_FLAGS% %STEAMWORKS_FLAGS%
goto done
:crystals
del "c:\users\trent\code\t\3crystals.exe"
cd c:\users\trent\code\3crystals
rmdir /s /q build
mkdir build
cd build
c:\users\trent\code\3crystals\misc\batch_files\cmake\windows\3crystals.bat %CFG_FLAGS% %STEAMWORKS_FLAGS% %DEMO_FLAGS%
goto done
:data
rmdir /s /q c:\users\trent\code\t\data
del c:\users\trent\code\t\data.cpa
cd c:\users\trent\code\t
rmdir /s /q data
goto done
:done
endlocal
popd
