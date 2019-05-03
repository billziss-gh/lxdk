/**
 * @file lxdktest/driver.c
 *
 * @copyright 2019 Bill Zissimopoulos
 */

#include <ntifs.h>
#include <lxdk/lxdk.h>

#pragma warning(disable:4100)           /* unreferenced formal parameter */

static INT DeviceOpen(
    PLX_CALL_CONTEXT CallContext,
    PLX_DEVICE Device,
    ULONG OpenFlags,
    PLX_FILE* PFile)
{
    return 0;
}

static INT DeviceUninitialize(
    PLX_DEVICE Device)
{
    return 0;
}

static INT FileFlush(
    PLX_CALL_CONTEXT CallContext,
    PLX_FILE File)
{
    return 0;
}

static INT FileIoctl(
    PLX_CALL_CONTEXT CallContext,
    PLX_FILE File,
    ULONG Code,
    PVOID Buffer)
{
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
    return 0;
}

static INT FileRelease(
    PLX_CALL_CONTEXT CallContext,
    PLX_FILE File)
{
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

    Device = VfsDeviceMinorAllocate(&DeviceCallbacks, 0);
    if (0 == Device)
    {
        Error = -ENOMEM;
        goto exit;
    }

    RtlZeroMemory(&Entry, sizeof Entry);
    Entry.Kind = VfsStartupEntryNode;
    RtlInitUnicodeString(&Entry.Path, L"/dev/lxdktest");
    Entry.Node.Mode = 0666;
    Entry.Node.DeviceMajor = 10;
    Entry.Node.DeviceMinor = 0xFAB;

    LxpDevMiscRegister(Instance, Device, Entry.Node.DeviceMinor);
    Device = 0;

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
    return LxldrRegisterService(DriverObject, TRUE, CreateInitialNamespace);
}
