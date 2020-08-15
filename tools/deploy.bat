@echo off

setlocal
setlocal EnableDelayedExpansion

set Config=Debug
set Suffix=x64
set Deploy=C:\Deploy\lxdk
set Target=Win10DBG
set Chkpnt=wsl1
if not X%1==X set Target=%1
if not X%2==X set Chkpnt=%2

(
    echo sc create lxldr type=kernel binPath=%%~dp0lxldr.sys
    echo sc create lxtstdrv type=kernel binPath=%%~dp0lxtstdrv.sys
    echo reg add HKLM\Software\LxDK\Services\lxtstdrv /f
    echo sc start lxldr
) > %~dp0..\build\VStudio\build\%Config%\deploy-setup.bat

set Files=
for %%f in (
    %~dp0..\build\VStudio\build\%Config%\
        lxldr.sys
        lxtstdrv.sys
        deploy-setup.bat
    ) do (
    set File=%%~f
    if [!File:~-1!] == [\] (
        set Dir=!File!
    ) else (
        if not [!Files!] == [] set Files=!Files!,
        set Files=!Files!'!Dir!!File!'
    )
)

powershell -NoProfile -ExecutionPolicy Bypass -Command "& '%~dp0deploy.ps1' -Name '%Target%' -CheckpointName '%Chkpnt%' -Files !Files! -Destination '%Deploy%'"
