@echo off
pushd .
setlocal
if "%1"=="t" goto tgui6
if "%1"=="s" goto shim4
if "%1"=="g" goto crystals
:tgui6
cd c:\users\trent\code\tgui6
git pull --rebase
goto done
:shim4
cd c:\users\trent\code\shim4
git pull --rebase
goto done
:crystals
cd c:\users\trent\code\3crystals
git pull --rebase
goto done
:done
endlocal
popd
