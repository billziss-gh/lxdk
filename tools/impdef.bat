@echo off

setlocal
setlocal EnableDelayedExpansion

if X%1==X goto usage
if X%2==X goto usage
set infile=%1
set infile=%infile:/=\%
set outfile=%2
set outfile=%outfile:/=\%
set workdir=!infile!.work
set workbase=!workdir!\%~n1

call "%VS140COMNTOOLS%\..\..\VC\vcvarsall.bat" x64
set INCLUDE=%~dp0..\inc;!WindowsSdkDir!Include\!WindowsSDKVersion!km\crt;!WindowsSdkDir!Include\!WindowsSDKVersion!km;!WindowsSdkDir!Include\!WindowsSDKVersion!km\shared;!INCLUDE!

if exist !workdir! rmdir /s/q !workdir!
mkdir !workdir!

echo #define LXDK_IMPORT(RetType, Name, ...) __declspec(dllexport) RetType NTAPI Name ( __VA_ARGS__ ) {} >!workbase!.c
type !infile! >>!workbase!.c
cl /LD /Fe!workbase!.dll /Fo!workbase!.obj /D_AMD64_ /wd4716 !workbase!.c
if errorlevel 1 goto fail

copy !workbase!.lib !outfile!
if errorlevel 1 goto fail

rmdir /s/q !workdir!

exit /b 0

:fail
exit /b 1

:usage
echo usage: impdef.bat infile.impdef outfile.lib 1>&2
exit /b 2
