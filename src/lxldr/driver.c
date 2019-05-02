/**
 * @file lxldr/driver.c
 *
 * @copyright 2019 Bill Zissimopoulos
 */

#include <lxldr/driver.h>

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

static void skipws(const wchar_t *p, const wchar_t **endp)
{
    for (; ' ' == *p; p++)
        ;

    if (0 != endp)
        *endp = (wchar_t *)p;
}

typedef struct
{
    PLX_INSTANCE Instance;
} SERVICE_LOADER_CONTEXT;

static NTSTATUS LoadService(
    HANDLE Root,
    PUNICODE_STRING Name,
    PVOID Context)
{
    NTSTATUS Status;

    Status = ZwLoadDriver(Name);
    if (!NT_SUCCESS(Status) && STATUS_IMAGE_ALREADY_LOADED != Status)
    {
        LOG(": \"%wZ\": error: ZwLoadDriver = %lx", Name, Status);

        goto exit;
    }

    Status = STATUS_SUCCESS;

exit:
    /*
     * A driver that is misconfigured should not prevent other drivers from loading.
     * Therefore always return SUCCESS.
     */
    return STATUS_SUCCESS;
}

static NTSTATUS AddServiceVfsStartupEntry(
    HANDLE Root,
    PUNICODE_STRING Name,
    ULONG Type,
    PVOID Buffer,
    ULONG Length,
    PVOID Context0)
{
    SERVICE_LOADER_CONTEXT *Context = Context0;
    PWSTR P = Buffer;
    LX_VFS_STARTUP_ENTRY Entry;
    INT Error;
    NTSTATUS Status;

    if (REG_SZ != Type || sizeof(WCHAR) > Length || L'\0' != P[Length / sizeof(WCHAR) - 1])
    {
        LOG(": \"%wZ\": error: invalid vfs entry", Name);

        Status = STATUS_INVALID_PARAMETER;
        goto exit;
    }

    RtlZeroMemory(&Entry, 0);
    Entry.Kind = VfsStartEntryNode;
    Entry.Path = *Name;
    Entry.Node.DeviceMajor = wcstoint(P, &P, 0); skipws(P, &P);
    Entry.Node.DeviceMinor = wcstoint(P, &P, 0); skipws(P, &P);
    Entry.Node.Mode = wcstoint(P, &P, 0); skipws(P, &P);
    Entry.Node.Uid = wcstoint(P, &P, 0); skipws(P, &P);
    Entry.Node.Gid = wcstoint(P, &P, 0); skipws(P, &P);
    Error = VfsInitializeStartupEntries(Context->Instance, &Entry, 1);
    if (0 > Error)
    {
        LOG(": \"%wZ\": error: VfsInitializeStartupEntries = %d", Name, Error);

        Status = STATUS_INVALID_PARAMETER;
        goto exit;
    }

    Status = STATUS_SUCCESS;

exit:
    return Status;
}

static NTSTATUS AddServiceVfsStartupEntries(
    HANDLE Root,
    PUNICODE_STRING Name,
    PVOID Context)
{
    WCHAR PathBuf[256 + sizeof L"Devices"];
    UNICODE_STRING Path;
    NTSTATUS Status;

    RtlInitEmptyUnicodeString(&Path, PathBuf, sizeof PathBuf);
    Status = RtlAppendUnicodeStringToString(&Path, Name);
    Status = NT_SUCCESS(Status) ? RtlAppendUnicodeToString(&Path, L"\\Devices") : Status;
    if (!NT_SUCCESS(Status))
    {
        LOG(": \"%wZ\": error: name too long", Name);

        Status = STATUS_INVALID_PARAMETER;
        goto exit;
    }

    Status = RegistryEnumerateKeyValues(Root, &Path, AddServiceVfsStartupEntry, Context);
    if (!NT_SUCCESS(Status) && STATUS_OBJECT_NAME_NOT_FOUND != Status)
    {
        LOG(": \"%wZ\": error: RegistryEnumerateKeyValues = %lx", Name, Status);

        goto exit;
    }

    Status = STATUS_SUCCESS;

exit:
    /*
     * A driver that is misconfigured should not prevent other drivers from loading.
     * Therefore always return SUCCESS.
     */
    return STATUS_SUCCESS;
}

static INT CreateInitialNamespace(
    PLX_INSTANCE Instance)
{
    SERVICE_LOADER_CONTEXT Context;
    UNICODE_STRING RegistryPath;
    NTSTATUS Status;

    RtlZeroMemory(&Context, sizeof Context);
    Context.Instance = Instance;

    RtlInitUnicodeString(&RegistryPath, L"" LXDK_REGPATH_SERVICES);
    Status = RegistryEnumerateKeys(0, &RegistryPath, LoadService, &Context);
    if (!NT_SUCCESS(Status))
        /* ignore */;

    RtlInitUnicodeString(&RegistryPath, L"" LXDK_REGPATH_SERVICES);
    Status = RegistryEnumerateKeys(0, &RegistryPath, AddServiceVfsStartupEntries, &Context);
    if (!NT_SUCCESS(Status))
        /* ignore */;

    return 0;
}

__declspec(dllexport) NTSTATUS NTAPI RegisterService(
    PDRIVER_OBJECT DriverObject,
    BOOLEAN Register,
    NTSTATUS (*Func)(
        PLX_INSTANCE Instance))
{
    PUNICODE_STRING DriverName = &DriverObject->DriverName;

    if (Register)
    {
        (VOID)DriverName;
        (VOID)Func;
    }
    else
    {
        (VOID)DriverName;
        (VOID)Func;
    }

    return STATUS_NOT_IMPLEMENTED;
}

NTSTATUS DriverEntry(
    PDRIVER_OBJECT DriverObject,
    PUNICODE_STRING RegistryPath)
{
    static LX_SUBSYSTEM Subsystem =
    {
        CreateInitialNamespace,
    };

    return LxInitialize(DriverObject, &Subsystem);
}
