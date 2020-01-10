@echo off

setlocal
setlocal EnableDelayedExpansion

set RegKey=HKLM\Software\LxDK\Services

if not X%1==X-u (
    if not X%1==X set lxname=%1
    if not X%2==X set lxpath="%~f2"

    if X!lxpath!==X goto usage
    if not exist !lxpath! goto notfound

    sc create !lxname! type=kernel binPath=!lxpath!
    if errorlevel 1 goto fail
    reg add !RegKey!\!lxname! /f
) else (
    if not X%2==X set lxname=%2

    if X!lxname!==X goto usage

    reg delete !RegKey!\!lxname! /f
    sc delete !lxname!
    if errorlevel 1 goto fail
)

exit /b 0

:fail
exit /b 1

:notfound
echo path !lxpath! not found >&2
exit /b 2

:usage
echo usage: lxreg NAME PATH >&2
echo usage: lxreg -u NAME >&2
exit /b 2
