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
typedef struct _LX_INODE LX_INODE, *PLX_INODE;
typedef struct _LX_INODE_CALLBACKS LX_INODE_CALLBACKS, *PLX_INODE_CALLBACKS;
typedef struct _LX_INODE_XATTR_CALLBACKS LX_INODE_XATTR_CALLBACKS, *PLX_INODE_XATTR_CALLBACKS;
typedef struct _LX_FILE LX_FILE, *PLX_FILE;
typedef struct _LX_FILE_CALLBACKS LX_FILE_CALLBACKS, *PLX_FILE_CALLBACKS;
typedef struct _LX_CALL_CONTEXT LX_CALL_CONTEXT, *PLX_CALL_CONTEXT;
typedef struct _LX_IOVECTOR LX_IOVECTOR, *PLX_IOVECTOR;
typedef struct _LX_IOVECTOR_BUFFER LX_IOVECTOR_BUFFER, *PLX_IOVECTOR_BUFFER;

typedef INT LX_DEVICE_OPEN(
    PLX_CALL_CONTEXT CallContext,
    PLX_DEVICE Device,
    ULONG OpenFlags,
    PLX_FILE *PFile);
typedef INT LX_DEVICE_UNINITIALIZE(
    PLX_DEVICE Device);

typedef LX_DEVICE_OPEN *PLX_DEVICE_OPEN;
typedef LX_DEVICE_UNINITIALIZE *PLX_DEVICE_UNINITIALIZE;

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
    PLX_DEVICE_OPEN Open;
    PLX_DEVICE_UNINITIALIZE Uninitialize;

    PVOID Reserved[6];
};

struct _LX_INODE_XATTR_CALLBACKS
{
    PVOID GetExtendedAttribute;
    PVOID SetExtendedAttribute;
    PVOID RemoveExtendedAttribute;

    PVOID Reserved[5];
};

struct _LX_INODE_CALLBACKS
{
    PVOID Open;
    PVOID Delete;
    PVOID Lookup;
    PVOID Stat;
    PVOID Chown;
    PVOID CreateFile;
    PVOID Symlink;
    PVOID Link;
    PVOID CreateDirectory;
    PVOID Unlink;
    PVOID Rmdir;
    PVOID Rename;
    PVOID ReadLink;
    PVOID CreateNode;
    PVOID Chmod;
    PVOID PrePermissionsCheck;
    PVOID PostPermissionsCheck;
    PVOID SetTimes;
    PVOID ListExtendedAttributes;
    PVOID FollowLink;
    PVOID InotifyNtWatchDecrementWatchCount;
    PVOID InotifyNtWatchIncrementWatchCount;
    PVOID ReferenceNtFileObject;
    PVOID Pin;
    struct LX_INODE_XATTR_CALLBACKS * ExtendedAttributeCallbacks;
    struct LX_INODE_XATTR_CALLBACKS * SystemExtendedAttributeCallbacks;

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

struct _LX_IOVECTOR_BUFFER
{
    PVOID Buffer;
    ULONG Length;
};

struct _LX_IOVECTOR
{
    INT Count;
    LX_IOVECTOR_BUFFER Vector[];
};

NTKERNELAPI PLX_DEVICE NTAPI VfsDeviceMinorAllocate(
    PLX_DEVICE_CALLBACKS Callbacks,
    SIZE_T Size);
NTKERNELAPI VOID NTAPI LxpDevMiscRegister(
    PLX_INSTANCE Instance,
    PLX_DEVICE Device,
    UINT32 DeviceMinor);

NTKERNELAPI PLX_INODE NTAPI VfsInodeAllocate(
    SIZE_T Size,
    PLX_INODE_CALLBACKS Callbacks);

NTKERNELAPI PLX_FILE NTAPI VfsFileAllocate(
    SIZE_T Size,
    PLX_FILE_CALLBACKS Callbacks);

#pragma warning(pop)

#endif
