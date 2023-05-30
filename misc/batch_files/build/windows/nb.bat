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
if "%1"=="d" goto endloop rem not supported by this script
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

if %TARGET%=="t" goto tgui6_type
if %TARGET%=="s" goto shim4_type
if %TARGET%=="g" goto crystals_type

:tgui6_type
if %CFG%=="release" goto tgui6_release
goto tgui6

:shim4_type
if %CFG%=="release" goto shim4_release
goto shim4

:crystals_type
if %CFG%=="release" goto crystals_release
goto crystals

echo Invalid target: %TARGET%
goto done

:tgui6
if "%2"=="r" goto tgui6_release
cd c:\users\trent\code\tgui6\build
msbuild /p:configuration=relwithdebinfo tgui6.sln
goto done
:tgui6_release
cd c:\users\trent\code\tgui6\build
msbuild /p:configuration=release tgui6.sln
goto done
:shim4
if "%2"=="r" goto shim4_release
cd c:\users\trent\code\shim4\build
msbuild /p:configuration=relwithdebinfo shim4.sln
goto done
:shim4_release
cd c:\users\trent\code\shim4\build
msbuild /p:configuration=release shim4.sln
goto done
:crystals
if "%2"=="r" goto crystals_release
cd c:\users\trent\code\3crystals\build
msbuild /p:configuration=relwithdebinfo "3crystals.sln"
goto done
:crystals_release
cd c:\users\trent\code\3crystals\build
msbuild /p:configuration=release "3crystals.sln"
goto done
:done
endlocal
popd
