@echo off

setlocal

set CONFIG=Debug
set TARGET_MACHINE=WIN10DBG
if not X%1==X set TARGET_MACHINE=%1
set TARGET_ACCOUNT=\Users\%USERNAME%\Downloads\lxdk\
set TARGET=\\%TARGET_MACHINE%%TARGET_ACCOUNT%

cd %~dp0..
mkdir %TARGET% 2>nul
for %%f in (lxldr.sys lxtstdrv.sys) do (
	copy build\VStudio\build\%CONFIG%\%%f %TARGET% >nul
)
echo sc create lxldr type=kernel binPath=%%~dp0lxldr.sys         >%TARGET%kminst.bat
echo sc create lxtstdrv type=kernel binPath=%%~dp0lxtstdrv.sys  >>%TARGET%kminst.bat
echo reg add HKLM\Software\LxDK\Services\lxtstdrv /f            >>%TARGET%kminst.bat
