/**
 * @file lxdktest/driver.c
 *
 * @copyright 2019 Bill Zissimopoulos
 */
/*
 * This file is part of Lxdk.
 *
 * You can redistribute it and/or modify it under the terms of the GNU
 * Lesser General Public License version 3 as published by the Free
 * Software Foundation.
 */

#include <ntifs.h>
#include <lxdk/lxdk.h>

#pragma warning(disable:4100)           /* unreferenced formal parameter */

#define LOG(Format, ...)                DbgPrint("%s" Format "\n", __FUNCTION__, __VA_ARGS__)
#define POOLTAG                         'LXDK'
#define BUFSIZE                         1024

typedef struct
{
    PVOID Buffer;
} FILE;

static INT FileDelete(
    PLX_CALL_CONTEXT CallContext,
    PLX_FILE File0)
{
    FILE *File = (FILE *)File0;
    INT Error;

    if (0 != File->Buffer)
        ExFreePoolWithTag(File->Buffer, POOLTAG);
    File->Buffer = 0;

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
    PLX_FILE File0,
    ULONG Code,
    PVOID Buffer)
{
    FILE *File = (FILE *)File0;
    INT Error;

    switch (Code)
    {
    case 0x8ead:
        try
        {
            ProbeForWrite(Buffer, sizeof(ULONG), 1);
            RtlCopyMemory(Buffer, File->Buffer, sizeof(ULONG));
        }
        except (EXCEPTION_EXECUTE_HANDLER)
        {
            Error = -EFAULT;
            goto exit;
        }
        break;

    case 0x817e:
        try
        {
            ProbeForRead(Buffer, sizeof(ULONG), 1);
            RtlCopyMemory(File->Buffer, Buffer, sizeof(ULONG));
        }
        except (EXCEPTION_EXECUTE_HANDLER)
        {
            Error = -EFAULT;
            goto exit;
        }

    default:
        Error = -EINVAL;
        goto exit;
    }

    Error = 0;

exit:
    LOG("(File=%p, Code=%lx) = %d", File, Code, Error);
    return Error;
}

static INT FileRead(
    PLX_CALL_CONTEXT CallContext,
    PLX_FILE File0,
    PVOID Buffer,
    ULONG Length,
    PLARGE_INTEGER ByteOffset,
    ULONG Flags,
    PUINT32 PBytesTransferred)
{
    FILE *File = (FILE *)File0;
    ULONG Offset, EndOffset;
    INT Error;

    *PBytesTransferred = 0;

    Offset = 0;
    if (0 != ByteOffset)
        Offset = ByteOffset->LowPart;
    EndOffset = Offset + Length;

    if (Offset > BUFSIZE)
        Offset = BUFSIZE;
    if (EndOffset > BUFSIZE)
        EndOffset = BUFSIZE;

    try
    {
        ProbeForWrite(Buffer, EndOffset - Offset, 1);
        RtlCopyMemory(Buffer, (PUINT8)File->Buffer + Offset, EndOffset - Offset);
    }
    except (EXCEPTION_EXECUTE_HANDLER)
    {
        Error = -EFAULT;
        goto exit;
    }

    *PBytesTransferred = EndOffset - Offset;
    Error = 0;

exit:
    LOG("(File=%p, Length=%lx, ByteOffset=%lx:%lx) = %d",
        File, Length,
        ByteOffset ? ByteOffset->HighPart : -1,
        ByteOffset ? ByteOffset->LowPart : -1,
        Error);
    return Error;
}

#if 0
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
#endif

static INT FileWrite(
    PLX_CALL_CONTEXT CallContext,
    PLX_FILE File0,
    PVOID Buffer,
    ULONG Length,
    PLARGE_INTEGER ByteOffset,
    ULONG Flags,
    PUINT32 PBytesTransferred)
{
    FILE *File = (FILE *)File0;
    ULONG Offset, EndOffset;
    INT Error;

    *PBytesTransferred = 0;

    Offset = 0;
    if (0 != ByteOffset)
        Offset = ByteOffset->LowPart;
    EndOffset = Offset + Length;

    if (Offset > BUFSIZE)
        Offset = BUFSIZE;
    if (EndOffset > BUFSIZE)
        EndOffset = BUFSIZE;

    try
    {
        ProbeForRead(Buffer, EndOffset - Offset, 1);
        RtlCopyMemory((PUINT8)File->Buffer + Offset, Buffer, EndOffset - Offset);
    }
    except (EXCEPTION_EXECUTE_HANDLER)
    {
        Error = -EFAULT;
        goto exit;
    }

    *PBytesTransferred = Length;
    Error = 0;

exit:
    LOG("(File=%p, Length=%lx, ByteOffset=%lx:%lx) = %d",
        File, Length,
        ByteOffset ? ByteOffset->HighPart : -1,
        ByteOffset ? ByteOffset->LowPart : -1,
        Error);
    return Error;
}

#if 0
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
#endif

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
#if 0
        .ReadVector = FileReadVector,
#endif
        .Write = FileWrite,
#if 0
        .WriteVector = FileWriteVector,
#endif
    };
    PVOID Buffer = 0;
    FILE *File;
    INT Error;

    *PFile = 0;

    Buffer = ExAllocatePoolWithTag(NonPagedPool, BUFSIZE, POOLTAG);
    if (0 == Buffer)
    {
        Error = -ENOMEM;
        goto exit;
    }

    File = (FILE *)VfsFileAllocate(sizeof *File, &FileCallbacks);
    if (0 == File)
    {
        Error = -ENOMEM;
        goto exit;
    }

    RtlZeroMemory(Buffer, BUFSIZE);
    RtlZeroMemory(File, sizeof *File);
    File->Buffer = Buffer;
    Buffer = 0;

    *PFile = (PLX_FILE)File;
    Error = 0;

exit:
    if (0 != Buffer)
        ExFreePoolWithTag(Buffer, POOLTAG);

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
    INT Error;

    Device = VfsDeviceMinorAllocate(&DeviceCallbacks, sizeof(LX_DEVICE));
    if (0 == Device)
    {
        Error = -ENOMEM;
        goto exit;
    }

    LxpDevMiscRegister(Instance, Device, 0x5BABE);

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
