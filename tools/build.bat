@echo off

setlocal
setlocal EnableDelayedExpansion

set MsiName="LxDK - Development Kit for WSL"
set CrossCert="%~dp0DigiCert High Assurance EV Root CA.crt"
set Issuer="DigiCert"
set Subject="Navimatics LLC"

set Configuration=Release
set SignedPackage=

if not X%1==X set Configuration=%1
if not X%2==X set SignedPackage=%2

set vswhere="%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
for /f "usebackq tokens=*" %%i in (`%vswhere% -find VC\**\vcvarsall.bat`) do (
    call "%%i" x64
)

if not X%SignedPackage%==X (
    if not exist "%~dp0..\build\VStudio\build\%Configuration%\lxdk-*.msi" (echo previous build not found >&2 & exit /b 1)
    if not exist "%SignedPackage%" (echo signed package not found >&2 & exit /b 1)
    del "%~dp0..\build\VStudio\build\%Configuration%\lxdk-*.msi"
    if exist "%~dp0..\build\VStudio\build\%Configuration%\lxdk.*.nupkg" del "%~dp0..\build\VStudio\build\%Configuration%\lxdk.*.nupkg"
    for /R "%SignedPackage%" %%f in (*.sys) do (
        copy "%%f" "%~dp0..\build\VStudio\build\%Configuration%" >nul
    )
)

cd %~dp0..\build\VStudio
set signfail=0

if X%SignedPackage%==X (
    if exist build\ for /R build\ %%d in (%Configuration%) do (
        if exist "%%d" rmdir /s/q "%%d"
    )

    devenv lxdk.sln /build "%Configuration%|x64"
    if errorlevel 1 goto fail

    for %%f in (build\%Configuration%\lxldr.sys build\%Configuration%\lxtstdrv.sys) do (
        signtool sign /ac %CrossCert% /i %Issuer% /n %Subject% /fd sha1 /t http://timestamp.digicert.com %%f
        if errorlevel 1 set /a signfail=signfail+1
        signtool sign /as /ac %CrossCert% /i %Issuer% /n %Subject% /fd sha256 /tr http://timestamp.digicert.com /td sha256 %%f
        if errorlevel 1 set /a signfail=signfail+1
    )

    pushd build\%Configuration%
    echo .OPTION EXPLICIT >driver.ddf
    echo .Set CabinetFileCountThreshold=0 >>driver.ddf
    echo .Set FolderFileCountThreshold=0 >>driver.ddf
    echo .Set FolderSizeThreshold=0 >>driver.ddf
    echo .Set MaxCabinetSize=0 >>driver.ddf
    echo .Set MaxDiskFileCount=0 >>driver.ddf
    echo .Set MaxDiskSize=0 >>driver.ddf
    echo .Set CompressionType=MSZIP >>driver.ddf
    echo .Set Cabinet=on >>driver.ddf
    echo .Set Compress=on >>driver.ddf
    echo .Set CabinetNameTemplate=driver.cab >>driver.ddf
    echo .Set DiskDirectory1=. >>driver.ddf
    echo .Set DestinationDir=x64 >>driver.ddf
    echo .Set DestinationDir=lxldr >>driver.ddf
    echo lxldr.inf >>driver.ddf
    echo lxldr.sys >>driver.ddf
    echo .Set DestinationDir=lxtstdrv >>driver.ddf
    echo lxtstdrv.inf >>driver.ddf
    echo lxtstdrv.sys >>driver.ddf
    makecab /F driver.ddf
    signtool sign /ac %CrossCert% /i %Issuer% /n %Subject% /t http://timestamp.digicert.com driver.cab
    if errorlevel 1 set /a signfail=signfail+1
    popd
)

devenv lxdk.sln /build "Installer.%Configuration%|x64"
if errorlevel 1 goto fail

for %%f in (build\%Configuration%\lxdk-*.msi) do (
    signtool sign /ac %CrossCert% /i %Issuer% /n %Subject% /fd sha1 /t http://timestamp.digicert.com /d %MsiName% %%f
    if errorlevel 1 set /a signfail=signfail+1
    REM signtool sign /ac %CrossCert% /i %Issuer% /n %Subject% /fd sha256 /tr http://timestamp.digicert.com /td sha256 /d %MsiName% %%f
    REM if errorlevel 1 set /a signfail=signfail+1
)

if not %signfail%==0 echo SIGNING FAILED! The product has been successfully built, but not signed.

exit /b 0

:fail
exit /b 1
