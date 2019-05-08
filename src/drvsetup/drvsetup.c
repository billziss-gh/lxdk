/**
 * @file drvsetup/drvsetup.c
 *
 * @copyright 2019 Bill Zissimopoulos
 */
/*
 * This file is part of LxDK.
 *
 * You can redistribute it and/or modify it under the terms of the GNU
 * Lesser General Public License version 3 as published by the Free
 * Software Foundation.
 */

#include <windows.h>
#include <shared/minimal.h>

#define PROGNAME                        L"drvsetup"

/*
 * Define MSITEST for MSI testing and no actual driver setup.
 */
//#define MSITEST

static int MessageBoxFormat(DWORD Type, PWSTR Caption, PWSTR Format, ...)
{
    WCHAR Buf[1024];
    va_list ap;

    va_start(ap, Format);
    wvsprintfW(Buf, Format, ap);
    va_end(ap);

    return MessageBoxW(0, Buf, Caption, Type);
}

static void usage(void)
{
    MessageBoxFormat(MB_ICONEXCLAMATION | MB_OK, PROGNAME, L""
        "usage: %s COMMAND ARGS\n"
        "\n"
        "commands:\n"
        "    add DriverName DriverPath.sys [StartType]\n"
        "    remove DriverName\n",
        PROGNAME);

    ExitProcess(ERROR_INVALID_PARAMETER);
}

#if !defined(MSITEST)

static DWORD WaitServiceStatus(SC_HANDLE SvcHandle, DWORD State, DWORD Timeout)
{
    SERVICE_STATUS SvcStatus;

    for (DWORD I = 0, N = (Timeout + 499) / 500; N > I; I++)
    {
        if (!QueryServiceStatus(SvcHandle, &SvcStatus))
            return GetLastError();

        if (State == SvcStatus.dwCurrentState)
            return ERROR_SUCCESS;

        Sleep(500);
    }

    return ERROR_TIMEOUT;
}

static DWORD add(PWSTR DriverName, PWSTR DriverPath, DWORD StartType)
{
    SC_HANDLE ScmHandle = 0;
    SC_HANDLE SvcHandle = 0;
    PVOID VersionInfo = 0;
    SERVICE_DESCRIPTIONW SvcDescription;
    DWORD Size;
    BOOL Created = FALSE;
    DWORD Result;

    ScmHandle = OpenSCManagerW(0, 0, SC_MANAGER_CREATE_SERVICE);
    if (0 == ScmHandle)
    {
        Result = GetLastError();
        goto exit;
    }

    SvcHandle = CreateServiceW(ScmHandle, DriverName, DriverName,
        SERVICE_CHANGE_CONFIG | SERVICE_START | SERVICE_QUERY_STATUS | DELETE,
        SERVICE_KERNEL_DRIVER, StartType, SERVICE_ERROR_NORMAL, DriverPath,
        0, 0, 0, 0, 0);
    if (0 == SvcHandle)
    {
        Result = GetLastError();
        if (ERROR_SERVICE_EXISTS == Result)
            Result = ERROR_SUCCESS_REBOOT_REQUIRED;
        goto exit;
    }
    Created = TRUE;

    Size = GetFileVersionInfoSizeW(DriverPath, &Size/*dummy*/);
    if (0 < Size)
    {
        VersionInfo = MemAlloc(Size);
        if (0 != VersionInfo &&
            GetFileVersionInfoW(DriverPath, 0, Size, VersionInfo) &&
            VerQueryValueW(VersionInfo, L"\\StringFileInfo\\040904b0\\FileDescription",
                &SvcDescription.lpDescription, &Size))
        {
            ChangeServiceConfig2W(SvcHandle, SERVICE_CONFIG_DESCRIPTION, &SvcDescription);
        }
    }

    if (SERVICE_BOOT_START == StartType ||
        SERVICE_SYSTEM_START == StartType ||
        SERVICE_AUTO_START == StartType)
    {
        if (!StartService(SvcHandle, 0, 0))
        {
            Result = GetLastError();
            if (ERROR_SERVICE_ALREADY_RUNNING == Result)
                Result = ERROR_SUCCESS;
            goto exit;
        }

        Result = WaitServiceStatus(SvcHandle, SERVICE_RUNNING, 5000);
        if (ERROR_SUCCESS != Result && ERROR_TIMEOUT != Result)
            goto exit;
    }

    Result = ERROR_SUCCESS;

exit:
    if (ERROR_SUCCESS != Result && Created)
        DeleteService(SvcHandle);
    MemFree(VersionInfo);
    if (0 != SvcHandle)
        CloseServiceHandle(SvcHandle);
    if (0 != ScmHandle)
        CloseServiceHandle(ScmHandle);

    return Result;
}

static DWORD remove(PWSTR DriverName)
{
    SC_HANDLE ScmHandle = 0;
    SC_HANDLE SvcHandle = 0;
    SERVICE_STATUS SvcStatus;
    BOOL RebootRequired = FALSE;
    DWORD Result;

    ScmHandle = OpenSCManagerW(0, 0, SC_MANAGER_CREATE_SERVICE);
        /*
         * The SC_MANAGER_CREATE_SERVICE access right is not strictly needed here,
         * but we use it to enforce admin rights.
         */
    if (0 == ScmHandle)
    {
        Result = GetLastError();
        goto exit;
    }

    SvcHandle = OpenServiceW(ScmHandle, DriverName,
        SERVICE_STOP | SERVICE_QUERY_STATUS | DELETE);
    if (0 == SvcHandle)
    {
        Result = GetLastError();
        if (ERROR_SERVICE_DOES_NOT_EXIST == Result)
            Result = ERROR_SUCCESS;
        goto exit;
    }

    ControlService(SvcHandle, SERVICE_CONTROL_STOP, &SvcStatus);
    Result = WaitServiceStatus(SvcHandle, SERVICE_STOPPED, 5000);
    if (ERROR_SUCCESS != Result && ERROR_TIMEOUT != Result)
        goto exit;
    RebootRequired = ERROR_TIMEOUT == Result;

    if (!DeleteService(SvcHandle))
    {
        Result = GetLastError();
        if (ERROR_SERVICE_MARKED_FOR_DELETE == Result)
            RebootRequired = TRUE;
        else
            goto exit;
    }

    Result = RebootRequired ? ERROR_SUCCESS_REBOOT_REQUIRED : ERROR_SUCCESS;

exit:
    if (0 != SvcHandle)
        CloseServiceHandle(SvcHandle);
    if (0 != ScmHandle)
        CloseServiceHandle(ScmHandle);

    return Result;
}

#else

static DWORD add(PWSTR DriverName, PWSTR DriverPath, DWORD StartType)
{
    MessageBoxFormat(MB_ICONINFORMATION | MB_OK, PROGNAME,
        L"add(DriverName=%s, DriverPath=%s, StartType=0x%lx)",
        DriverName, DriverPath, StartType);

    return ERROR_SUCCESS;
}

static DWORD remove(PWSTR DriverName)
{
    MessageBoxFormat(MB_ICONINFORMATION | MB_OK, PROGNAME,
        L"remove(DriverName=%s)", DriverName);

    return ERROR_SUCCESS;
}

#endif

static unsigned wcstoint(const wchar_t *p, const wchar_t **endp, int base)
{
    unsigned v;
    int maxdig, maxalp;

    if (0 == base)
    {
        if ('0' == *p)
        {
            p++;
            if ('x' == *p || 'X' == *p)
            {
                p++;
                base = 16;
            }
            else
                base = 8;
        }
        else
        {
            base = 10;
        }
    }

    maxdig = 10 < base ? '9' : (base - 1) + '0';
    maxalp = 10 < base ? (base - 1 - 10) + 'a' : 0;

    for (v = 0; *p; p++)
    {
        int c = *p;

        if ('0' <= c && c <= maxdig)
            v = base * v + (c - '0');
        else
        {
            c |= 0x20;
            if ('a' <= c && c <= maxalp)
                v = base * v + (c - 'a') + 10;
            else
                break;
        }
    }

    if (0 != endp)
        *endp = (wchar_t *)p;

    return v;
}

int wmain(int argc, wchar_t **argv)
{
    argc--;
    argv++;

    if (0 == argc)
        usage();

    if (0 == invariant_wcscmp(L"add", argv[0]))
    {
        wchar_t *endp;
        DWORD StartType = SERVICE_SYSTEM_START;
        if (4 == argc)
        {
            StartType = wcstoint(argv[3], &endp, 0);
            if (argv[3] == endp || L'\0' != *endp)
                usage();
        }
        else if (3 != argc)
            usage();

        return add(argv[1], argv[2], StartType);
    }
    else
    if (0 == invariant_wcscmp(L"remove", argv[0]))
    {
        if (2 != argc)
            usage();

        return remove(argv[1]);
    }
    else
        usage();

    return 0;
}

void WinMainCRTStartup(void)
{
    DWORD Argc;
    PWSTR *Argv;

    Argv = CommandLineToArgvW(GetCommandLineW(), &Argc);
    if (0 == Argv)
        ExitProcess(GetLastError());

    ExitProcess(wmain(Argc, Argv));
}
