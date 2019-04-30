/**
 * @file lxdk/lxdk.h
 *
 * @copyright 2019 Bill Zissimopoulos
 */

#ifndef LXDK_LXDK_H_INCLUDED
#define LXDK_LXDK_H_INCLUDED

#pragma warning(push)
#pragma warning(disable:4200)           /* zero-sized array in struct/union */

typedef struct _LX_INSTANCE LX_INSTANCE, *PLX_INSTANCE;
typedef struct _LX_DEVICE LX_DEVICE, *PLX_DEVICE;
typedef struct _LX_DEVICE_CALLBACKS LX_DEVICE_CALLBACKS, *PLX_DEVICE_CALLBACKS;
typedef struct _LX_FILE LX_FILE, *PLX_FILE;
typedef struct _LX_FILE_CALLBACKS LX_FILE_CALLBACKS, *PLX_FILE_CALLBACKS;
typedef struct _LX_CALL_CONTEXT LX_CALL_CONTEXT, *PLX_CALL_CONTEXT;
typedef struct _LX_IOVECTOR LX_IOVECTOR, *PLX_IOVECTOR;

typedef INT LX_FILE_FLUSH(
    PLX_CALL_CONTEXT CallContext,
    PLX_FILE File);
typedef INT LX_FILE_IOCTL(
    PLX_CALL_CONTEXT CallContext,
    PLX_FILE File,
    ULONG Code,
    PVOID Buffer);
typedef INT LX_FILE_READ(
    PLX_CALL_CONTEXT CallContext,
    PLX_FILE File,
    PVOID Buffer,
    ULONG Length,
    PLARGE_INTEGER ByteOffset,
    ULONG Flags,
    PUINT32 PBytesTransferred);
typedef INT LX_FILE_READ_VECTOR(
    PLX_CALL_CONTEXT CallContext,
    PLX_FILE File,
    PLX_IOVECTOR IoVector,
    PLARGE_INTEGER ByteOffset,
    ULONG Flags,
    PUINT32 PBytesTransferred);
typedef INT LX_FILE_RELEASE(
    PLX_CALL_CONTEXT CallContext,
    PLX_FILE File);
typedef INT LX_FILE_WRITE(
    PLX_CALL_CONTEXT CallContext,
    PLX_FILE File,
    PVOID Buffer,
    ULONG Length,
    PLARGE_INTEGER ByteOffset,
    ULONG Flags,
    PUINT32 PBytesTransferred);
typedef INT LX_FILE_WRITE_VECTOR(
    PLX_CALL_CONTEXT CallContext,
    PLX_FILE File,
    PLX_IOVECTOR IoVector,
    PLARGE_INTEGER ByteOffset,
    ULONG Flags,
    PUINT32 PBytesTransferred);

typedef LX_FILE_FLUSH *PLX_FILE_FLUSH;
typedef LX_FILE_IOCTL *PLX_FILE_IOCTL;
typedef LX_FILE_READ *PLX_FILE_READ;
typedef LX_FILE_READ_VECTOR *PLX_FILE_READ_VECTOR;
typedef LX_FILE_RELEASE *PLX_FILE_RELEASE;
typedef LX_FILE_WRITE *PLX_FILE_WRITE;
typedef LX_FILE_WRITE_VECTOR *PLX_FILE_WRITE_VECTOR;

struct _LX_DEVICE_CALLBACKS
{
    PVOID Open;
    PVOID Cleanup;

    PVOID Reserved[6];
};

struct _LX_FILE_CALLBACKS
{
    PVOID Delete;
    PLX_FILE_READ Read;
    PVOID ReadDir;
    PLX_FILE_WRITE Write;
    PLX_FILE_WRITE_VECTOR WriteVector;
    PVOID Map;
    PVOID MapManual;
    PLX_FILE_IOCTL Ioctl;
    PLX_FILE_FLUSH Flush;
    PVOID Sync;
    PLX_FILE_RELEASE Release;
    PLX_FILE_READ_VECTOR ReadVector;
    PVOID Truncate;
    PVOID Seek;
    PVOID FilterPollRegistration;
    PVOID FAllocate;
    PVOID GetPathString;
    PVOID GetNtDeviceType;

    PVOID Reserved[14];
};

struct _LX_IOVECTOR
{
    INT Count;
    struct
    {
        PVOID Buffer;
        ULONG Length;
    } Vector[];
};

NTKERNELAPI PLX_DEVICE NTAPI VfsDeviceMinorAllocate(
    PLX_DEVICE_CALLBACKS Callbacks,
    SIZE_T Size);
NTKERNELAPI VOID NTAPI VfsDeviceMinorDereference(
    PLX_DEVICE Device);
NTKERNELAPI VOID NTAPI LxpDevMiscRegister(
    PLX_INSTANCE Instance,
    PLX_DEVICE Device,
    UINT32 DeviceMinor);

NTKERNELAPI PLX_FILE *VfsFileAllocate(
    SIZE_T Size,
    PLX_FILE_CALLBACKS Callbacks);

#pragma warning(pop)

#endif
