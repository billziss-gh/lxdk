/**
 * @file CustomActions.cpp
 *
 * @copyright 2018-2019 Bill Zissimopoulos
 */
/*
 * This file is part of LxDK.
 *
 * You can redistribute it and/or modify it under the terms of the GNU
 * Lesser General Public License version 3 as published by the Free
 * Software Foundation.
 */

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <msiquery.h>
#include <wcautil.h>
#include <strutil.h>

#define ATOM_REBOOT                     L"LxDK.Reboot.{3D70E557-A771-4E42-B1D2-2852AC517AD7}"

UINT __stdcall ExecuteCommand(MSIHANDLE MsiHandle)
{
#if 0
    WCHAR MessageBuf[64];
    wsprintfW(MessageBuf, L"PID=%ld", GetCurrentProcessId());
    MessageBoxW(0, MessageBuf, L"" __FUNCTION__ " Break", MB_OK);
#endif

    HRESULT hr = S_OK;
    UINT err = ERROR_SUCCESS;
    PWSTR CommandLine = 0;
    STARTUPINFOW StartupInfo;
    PROCESS_INFORMATION ProcessInfo;
    DWORD ExitCode = ERROR_SUCCESS;
    DWORD RebootRequired;

    hr = WcaInitialize(MsiHandle, __FUNCTION__);
    ExitOnFailure(hr, "Failed to initialize");

    hr = WcaGetProperty(L"CustomActionData", &CommandLine);
    ExitOnFailure(hr, "Failed to get CommandLine");

    WcaLog(LOGMSG_STANDARD, "Initialized: \"%S\"", CommandLine);

    memset(&StartupInfo, 0, sizeof StartupInfo);
    StartupInfo.cb = sizeof StartupInfo;
    StartupInfo.dwFlags = STARTF_USESHOWWINDOW;
    StartupInfo.wShowWindow = SW_HIDE;

    if (!CreateProcessW(0, CommandLine, 0, 0, FALSE, 0, 0, 0, &StartupInfo, &ProcessInfo))
        ExitWithLastError(hr, "Failed to CreateProcessW");

    WaitForSingleObject(ProcessInfo.hProcess, INFINITE);
    GetExitCodeProcess(ProcessInfo.hProcess, &ExitCode);

    CloseHandle(ProcessInfo.hThread);
    CloseHandle(ProcessInfo.hProcess);

    RebootRequired = ERROR_SUCCESS_REBOOT_REQUIRED == ExitCode;
    if (RebootRequired)
    {
        if (0 == GlobalFindAtomW(ATOM_REBOOT))
            GlobalAddAtomW(ATOM_REBOOT);
        ExitCode = ERROR_SUCCESS;
    }

LExit:
    ReleaseStr(CommandLine);

    err = SUCCEEDED(hr) && ERROR_SUCCESS == ExitCode ? ERROR_SUCCESS : ERROR_INSTALL_FAILURE;
    return WcaFinalize(err);
}

UINT __stdcall CheckReboot(MSIHANDLE MsiHandle)
{
#if 0
    WCHAR MessageBuf[64];
    wsprintfW(MessageBuf, L"PID=%ld", GetCurrentProcessId());
    MessageBoxW(0, MessageBuf, L"" __FUNCTION__ " Break", MB_OK);
#endif

    HRESULT hr = S_OK;
    UINT err = ERROR_SUCCESS;
    DWORD RebootRequired = FALSE;

    hr = WcaInitialize(MsiHandle, __FUNCTION__);
    ExitOnFailure(hr, "Failed to initialize");

    WcaLog(LOGMSG_STANDARD, "Initialized");

    RebootRequired = 0 != GlobalFindAtomW(ATOM_REBOOT);
    GlobalDeleteAtom(GlobalFindAtomW(ATOM_REBOOT));

    WcaSetIntProperty(L"" __FUNCTION__, RebootRequired);

LExit:
    err = SUCCEEDED(hr) ? ERROR_SUCCESS : ERROR_INSTALL_FAILURE;
    return WcaFinalize(err);
}

extern "C"
BOOL WINAPI DllMain(HINSTANCE Instance, DWORD Reason, PVOID Reserved)
{
    switch(Reason)
    {
    case DLL_PROCESS_ATTACH:
        WcaGlobalInitialize(Instance);
        break;
    case DLL_PROCESS_DETACH:
        WcaGlobalFinalize();
        break;
    }

    return TRUE;
}
