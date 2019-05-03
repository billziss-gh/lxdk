/**
 * @file lxdktest/driver.c
 *
 * @copyright 2019 Bill Zissimopoulos
 */

#include <ntifs.h>
#include <lxdk/lxdk.h>

#pragma warning(disable:4100)           /* unreferenced formal parameter */

#define LOG(Format, ...)                DbgPrint("%s" Format "\n", __FUNCTION__, __VA_ARGS__)

static INT DeviceOpen(
    PLX_CALL_CONTEXT CallContext,
    PLX_DEVICE Device,
    ULONG OpenFlags,
    PLX_FILE *PFile)
{
    LOG(": Device=%p, OpenFlags=%lx", Device, OpenFlags);

    return 0;
}

static INT DeviceUninitialize(
    PLX_DEVICE Device)
{
    LOG(": Device=%p", Device);

    return 0;
}

static INT FileDelete(
    PLX_CALL_CONTEXT CallContext,
    PLX_FILE File)
{
    LOG(": File=%p", File);

    return 0;
}

static INT FileFlush(
    PLX_CALL_CONTEXT CallContext,
    PLX_FILE File)
{
    LOG(": File=%p", File);

    return 0;
}

static INT FileIoctl(
    PLX_CALL_CONTEXT CallContext,
    PLX_FILE File,
    ULONG Code,
    PVOID Buffer)
{
    LOG(": File=%p, Code=%lx", File, Code);

    return 0;
}

static INT FileRead(
    PLX_CALL_CONTEXT CallContext,
    PLX_FILE File,
    PVOID Buffer,
    ULONG Length,
    PLARGE_INTEGER ByteOffset,
    ULONG Flags,
    PUINT32 PBytesTransferred)
{
    LOG(": File=%p, Length=%lx, ByteOffset=%lx:%lx",
        File, Length, ByteOffset->HighPart, ByteOffset->LowPart);

    return 0;
}

static INT FileReadVector(
    PLX_CALL_CONTEXT CallContext,
    PLX_FILE File,
    PLX_IOVECTOR IoVector,
    PLARGE_INTEGER ByteOffset,
    ULONG Flags,
    PUINT32 PBytesTransferred)
{
    LOG(": File=%p, IoVector->Count=%lx, ByteOffset=%lx:%lx",
        File, IoVector->Count, ByteOffset->HighPart, ByteOffset->LowPart);

    return 0;
}

static INT FileRelease(
    PLX_CALL_CONTEXT CallContext,
    PLX_FILE File)
{
    LOG(": File=%p", File);

    return 0;
}

static INT FileWrite(
    PLX_CALL_CONTEXT CallContext,
    PLX_FILE File,
    PVOID Buffer,
    ULONG Length,
    PLARGE_INTEGER ByteOffset,
    ULONG Flags,
    PUINT32 PBytesTransferred)
{
    LOG(": File=%p, Length=%lx, ByteOffset=%lx:%lx",
        File, Length, ByteOffset->HighPart, ByteOffset->LowPart);

    return 0;
}

static INT FileWriteVector(
    PLX_CALL_CONTEXT CallContext,
    PLX_FILE File,
    PLX_IOVECTOR IoVector,
    PLARGE_INTEGER ByteOffset,
    ULONG Flags,
    PUINT32 PBytesTransferred)
{
    LOG(": File=%p, IoVector->Count=%lx, ByteOffset=%lx:%lx",
        File, IoVector->Count, ByteOffset->HighPart, ByteOffset->LowPart);

    return 0;
}

static INT CreateInitialNamespace(
    PLX_INSTANCE Instance)
{
    static LX_DEVICE_CALLBACKS DeviceCallbacks =
    {
        .Open = DeviceOpen,
        .Uninitialize = DeviceUninitialize,
    };
    static LX_FILE_CALLBACKS FileCallbacks =
    {
        .Delete = FileDelete,
        .Flush = FileFlush,
        .Ioctl = FileIoctl,
        .Read = FileRead,
        .ReadVector = FileReadVector,
        .Release = FileRelease,
        .Write = FileWrite,
        .WriteVector = FileWriteVector,
    };
    PLX_DEVICE Device = 0;
    LX_VFS_STARTUP_ENTRY Entry;
    INT Error;

    Device = VfsDeviceMinorAllocate(&DeviceCallbacks, sizeof(LX_DEVICE));
    if (0 == Device)
    {
        Error = -ENOMEM;
        goto exit;
    }

    RtlZeroMemory(&Entry, sizeof Entry);
    Entry.Kind = VfsStartupEntryNode;
    RtlInitUnicodeString(&Entry.Path, L"/dev/lxdktest");
    Entry.Node.Mode = 020666;
    Entry.Node.DeviceMajor = 10;
    Entry.Node.DeviceMinor = 0xFAB;

    LxpDevMiscRegister(Instance, Device, Entry.Node.DeviceMinor);

    Error = VfsInitializeStartupEntries(Instance, &Entry, 1);
    if (0 > Error)
        goto exit;

    Error = 0;

exit:
    if (0 != Device)
        VfsDeviceMinorDereference(Device);

    return Error;
}

NTSTATUS DriverEntry(
    PDRIVER_OBJECT DriverObject,
    PUNICODE_STRING RegistryPath)
{
    if (!KD_DEBUGGER_NOT_PRESENT)
        DbgBreakPoint();

    return LxldrRegisterService(DriverObject, TRUE, CreateInitialNamespace);
}
