/**
 * @file lxdktest/driver.c
 *
 * @copyright 2019 Bill Zissimopoulos
 */

#include <ntifs.h>
#include <lxdk/lxdk.h>

#pragma warning(disable:4100)           /* unreferenced formal parameter */

#define LOG(Format, ...)                DbgPrint("%s" Format "\n", __FUNCTION__, __VA_ARGS__)

static INT FileDelete(
    PLX_CALL_CONTEXT CallContext,
    PLX_FILE File)
{
    INT Error;

    Error = 0;

    LOG("(File=%p) = %d", File, Error);
    return Error;
}

static INT FileFlush(
    PLX_CALL_CONTEXT CallContext,
    PLX_FILE File)
{
    INT Error;

    Error = 0;

    LOG("(File=%p) = %d", File, Error);
    return Error;
}

static INT FileIoctl(
    PLX_CALL_CONTEXT CallContext,
    PLX_FILE File,
    ULONG Code,
    PVOID Buffer)
{
    INT Error;

    Error = 0;

    LOG("(File=%p, Code=%lx) = %d", File, Code, Error);
    return Error;
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
    INT Error;

    *PBytesTransferred = 0;
    Error = 0;

    LOG("(File=%p, Length=%lx, ByteOffset=%lx:%lx) = %d",
        File, Length,
        ByteOffset ? ByteOffset->HighPart : -1,
        ByteOffset ? ByteOffset->LowPart : -1,
        Error);
    return Error;
}

static INT FileReadVector(
    PLX_CALL_CONTEXT CallContext,
    PLX_FILE File,
    PLX_IOVECTOR IoVector,
    PLARGE_INTEGER ByteOffset,
    ULONG Flags,
    PUINT32 PBytesTransferred)
{
    INT Error;

    *PBytesTransferred = 0;
    Error = 0;

    LOG("(File=%p, IoVector->Count=%lx, ByteOffset=%lx:%lx) = %d",
        File, IoVector->Count,
        ByteOffset ? ByteOffset->HighPart : -1,
        ByteOffset ? ByteOffset->LowPart : -1,
        Error);
    return Error;
}

static INT FileRelease(
    PLX_CALL_CONTEXT CallContext,
    PLX_FILE File)
{
    INT Error;

    Error = 0;

    LOG("(File=%p) = %d", File, Error);
    return Error;
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
    INT Error;

    *PBytesTransferred = 0;
    Error = 0;

    LOG("(File=%p, Length=%lx, ByteOffset=%lx:%lx) = %d",
        File, Length,
        ByteOffset ? ByteOffset->HighPart : -1,
        ByteOffset ? ByteOffset->LowPart : -1,
        Error);
    return Error;
}

static INT FileWriteVector(
    PLX_CALL_CONTEXT CallContext,
    PLX_FILE File,
    PLX_IOVECTOR IoVector,
    PLARGE_INTEGER ByteOffset,
    ULONG Flags,
    PUINT32 PBytesTransferred)
{
    INT Error;

    *PBytesTransferred = 0;
    Error = 0;

    LOG("(File=%p, IoVector->Count=%lx, ByteOffset=%lx:%lx) = %d",
        File, IoVector->Count,
        ByteOffset ? ByteOffset->HighPart : -1,
        ByteOffset ? ByteOffset->LowPart : -1,
        Error);
    return Error;
}

static INT DeviceOpen(
    PLX_CALL_CONTEXT CallContext,
    PLX_DEVICE Device,
    ULONG OpenFlags,
    PLX_FILE *PFile)
{
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
    PLX_FILE File;
    INT Error;

    *PFile = 0;

    File = VfsFileAllocate(0, &FileCallbacks);
    if (0 == File)
    {
        Error = -ENOMEM;
        goto exit;
    }

    *PFile = File;
    Error = 0;

exit:
    LOG("(Device=%p, OpenFlags=%lx) = %d", Device, OpenFlags, Error);
    return Error;
}

static INT DeviceDelete(
    PLX_DEVICE Device)
{
    INT Error;

    Error = 0;

    LOG("(Device=%p) = %d", Device, Error);
    return Error;
}

static INT CreateInitialNamespace(
    PLX_INSTANCE Instance)
{
    static LX_DEVICE_CALLBACKS DeviceCallbacks =
    {
        .Open = DeviceOpen,
        .Delete = DeviceDelete,
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
    //RtlInitUnicodeString(&Entry.Path, L"/dev/lxdktest");
        /*
         * There appears to be a problem with creating entries in /dev.
         * These entries appear on the Windows backed rootfs, but they
         * do not appear in the Linux /dev tmpfs. This may be because
         * the /dev tmpfs appears to be mounted well after VfsInitialize
         * (by init).
         */
    RtlInitUnicodeString(&Entry.Path, L"/lxdktest");
    Entry.Node.Mode = 020666;
    Entry.Node.DeviceMajor = 10;
    Entry.Node.DeviceMinor = 0x5BABE;

    LxpDevMiscRegister(Instance, Device, Entry.Node.DeviceMinor);

    Error = VfsInitializeStartupEntries(Instance, &Entry, 1);
    if (0 > Error && -EEXIST != Error)
        goto exit;

    Error = 0;

exit:
    if (0 != Device)
        VfsDeviceMinorDereference(Device);

    LOG("(Instance=%p) = %d", Instance, Error);
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
