@echo off
pushd .
setlocal

set CFG="relwithdebinfo"
set TARGET="x"

:beginloop
if "%1"=="debug" goto debug
if "%1"=="release" goto release
if "%1"=="demo" goto endloop rem not supported in this script
if "%1"=="steam" goto endloop rem not supported in this script
if "%1"=="t" goto tgui6_flag
if "%1"=="s" goto shim4_flag
if "%1"=="g" goto crystals_flag
if "%1"=="d" goto data_flag
goto doneloop
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
:release
set CFG="release"
goto endloop
:debug
set CFG="relwithdebinfo"
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
cd c:\users\trent\code\tgui6\build
if %CFG%=="release" goto tgui6_release
rem copy relwithdebinfo\tgui6.dll ..\..\3
goto done
:tgui6_release
rem copy release\tgui6.dll ..\..\3
goto done
:shim4
cd c:\users\trent\code\shim4\build
if %CFG%=="release" goto shim4_release
rem copy relwithdebinfo\shim4.dll ..\..\3
goto done
:shim4_release
rem copy release\shim4.dll ..\..\3
goto done
:crystals
cd c:\users\trent\code\3crystals\build
if %CFG%=="release" goto crystals_release
copy "relwithdebinfo\3 Crystals.exe" ..\..\3
goto done
:crystals_release
copy "release\3 Crystals.exe" ..\..\3
goto done
:data
if %CFG%=="release" goto data_release
cd c:\users\trent\code\3
xcopy /q /e /y ..\3crystals\data data\
copy ..\3crystals\docs\3rd_party.html .
goto done
:data_release
cd c:\users\trent\code\3crystals\data
c:\users\trent\code\compress_dir\compress_dir.exe > nul
move ..\data.cpa c:\users\trent\code\3
copy ..\docs\3rd_party.html c:\users\trent\code\3
goto done
:done
endlocal
popd
