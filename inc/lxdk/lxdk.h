/**
 * @file lxdk/lxdk.h
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

#ifndef LXDK_LXDK_H_INCLUDED
#define LXDK_LXDK_H_INCLUDED

#pragma warning(push)
#pragma warning(disable:4200)           /* zero-sized array in struct/union */
#pragma warning(disable:4201)           /* nameless struct/union */

#include <lxdk/errno.h>

typedef struct _LX_SUBSYSTEM LX_SUBSYSTEM, *PLX_SUBSYSTEM;
typedef struct _LX_INSTANCE LX_INSTANCE, *PLX_INSTANCE;
typedef struct _LX_VFS_STARTUP_ENTRY LX_VFS_STARTUP_ENTRY, *PLX_VFS_STARTUP_ENTRY;
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

typedef INT LX_SUBSYSTEM_CREATE_INITIAL_NAMESPACE(
    PLX_INSTANCE Instance);

typedef LX_SUBSYSTEM_CREATE_INITIAL_NAMESPACE *PLX_SUBSYSTEM_CREATE_INITIAL_NAMESPACE;

typedef INT LX_DEVICE_OPEN(
    PLX_CALL_CONTEXT CallContext,
    PLX_DEVICE Device,
    ULONG Flags,
    PLX_FILE *PFile);
typedef INT LX_DEVICE_DELETE(
    PLX_DEVICE Device);

typedef LX_DEVICE_OPEN *PLX_DEVICE_OPEN;
typedef LX_DEVICE_DELETE *PLX_DEVICE_DELETE;

typedef INT LX_FILE_DELETE(
    PLX_CALL_CONTEXT CallContext,
    PLX_FILE File);
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
    SIZE_T Length,
    PLARGE_INTEGER ByteOffset,
    PSIZE_T PBytesTransferred);
typedef INT LX_FILE_READ_VECTOR(
    PLX_CALL_CONTEXT CallContext,
    PLX_FILE File,
    PLX_IOVECTOR IoVector,
    PLARGE_INTEGER ByteOffset,
    ULONG Flags,
    PSIZE_T PBytesTransferred);
typedef INT LX_FILE_RELEASE(
    PLX_CALL_CONTEXT CallContext,
    PLX_FILE File);
typedef INT LX_FILE_WRITE(
    PLX_CALL_CONTEXT CallContext,
    PLX_FILE File,
    PVOID Buffer,
    SIZE_T Length,
    PLARGE_INTEGER ByteOffset,
    PSIZE_T PBytesTransferred);
typedef INT LX_FILE_WRITE_VECTOR(
    PLX_CALL_CONTEXT CallContext,
    PLX_FILE File,
    PLX_IOVECTOR IoVector,
    PLARGE_INTEGER ByteOffset,
    ULONG Flags,
    PSIZE_T PBytesTransferred);

typedef LX_FILE_DELETE *PLX_FILE_DELETE;
typedef LX_FILE_FLUSH *PLX_FILE_FLUSH;
typedef LX_FILE_IOCTL *PLX_FILE_IOCTL;
typedef LX_FILE_READ *PLX_FILE_READ;
typedef LX_FILE_READ_VECTOR *PLX_FILE_READ_VECTOR;
typedef LX_FILE_RELEASE *PLX_FILE_RELEASE;
typedef LX_FILE_WRITE *PLX_FILE_WRITE;
typedef LX_FILE_WRITE_VECTOR *PLX_FILE_WRITE_VECTOR;

struct _LX_SUBSYSTEM
{
    PLX_SUBSYSTEM_CREATE_INITIAL_NAMESPACE CreateInitialNamespace;

    PVOID Reserved[7];
};

enum
{
    VfsStartupEntryDirectory,
    VfsStartupEntryMount,
    VfsStartupEntryNode,
    VfsStartupEntrySymlink,
    VfsStartupEntryFile,
};

struct _LX_VFS_STARTUP_ENTRY
{
    ULONG Kind;
    UNICODE_STRING Path;
    union
    {
        struct
        {
            ULONG Uid;
            ULONG Gid;
            ULONG Mode;
        } Directory;
        UINT8 Mount[72];
        struct
        {
            ULONG Uid;
            ULONG Gid;
            ULONG Mode;
            UINT32 DeviceMinor:20;
            UINT32 DeviceMajor:12;
        } Node;
        struct
        {
            UNICODE_STRING TargetPath;
        } Symlink;
        struct
        {
            ULONG Mode;
        } File;
    } DUMMYUNIONNAME;
};

struct _LX_DEVICE
{
    UINT8 Reserved[256];
};

struct _LX_DEVICE_CALLBACKS
{
    PLX_DEVICE_OPEN Open;
    PLX_DEVICE_DELETE Delete;

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
    struct LX_INODE_XATTR_CALLBACKS *ExtendedAttributeCallbacks;
    struct LX_INODE_XATTR_CALLBACKS *SystemExtendedAttributeCallbacks;

    PVOID Reserved[6];
};

struct _LX_FILE_CALLBACKS
{
    PLX_FILE_DELETE Delete;
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
    SIZE_T Length;
};

struct _LX_IOVECTOR
{
    INT Count;
    LX_IOVECTOR_BUFFER Vector[];
};

/*
 * lxcore exports
 */

#if !defined(LXDK_IMPORT)
#define LXDK_IMPORT(RetType, Name, ...) NTKERNELAPI RetType NTAPI Name ( __VA_ARGS__ );
#endif

LXDK_IMPORT(NTSTATUS, LxInitialize,
    PDRIVER_OBJECT DriverObject,
    PLX_SUBSYSTEM Subsystem)
LXDK_IMPORT(INT, VfsInitializeStartupEntries,
    PLX_INSTANCE Instance,
    PLX_VFS_STARTUP_ENTRY Entries,
    ULONG Count)
LXDK_IMPORT(PLX_DEVICE, VfsDeviceMinorAllocate,
    PLX_DEVICE_CALLBACKS Callbacks,
    SIZE_T Size)
LXDK_IMPORT(VOID, VfsDeviceMinorDereference,
    PLX_DEVICE Device)
LXDK_IMPORT(VOID, LxpDevMiscRegister,
    PLX_INSTANCE Instance,
    PLX_DEVICE Device,
    UINT32 DeviceMinor)
LXDK_IMPORT(PLX_INODE, VfsInodeAllocate,
    SIZE_T Size,
    PLX_INODE_CALLBACKS Callbacks)
LXDK_IMPORT(PLX_FILE, VfsFileAllocate,
    SIZE_T Size,
    PLX_FILE_CALLBACKS Callbacks)
LXDK_IMPORT(INT, LxpUtilTranslateStatus,
    NTSTATUS Status)

/*
 * lxldr exports
 */

#if !defined(LXDK_LDRSYM)
#define LXDK_LDRSYM(RetType, Name, ...) NTKERNELAPI RetType NTAPI Name ( __VA_ARGS__ );
#endif

LXDK_LDRSYM(NTSTATUS, LxldrRegisterService,
    PDRIVER_OBJECT DriverObject,
    BOOLEAN Register,
    PLX_SUBSYSTEM_CREATE_INITIAL_NAMESPACE CreateInitialNamespace)

#pragma warning(pop)

#endif
