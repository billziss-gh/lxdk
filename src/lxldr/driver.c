/**
 * @file lxldr/driver.c
 *
 * @copyright 2019-2020 Bill Zissimopoulos
 */
/*
 * This file is part of LxDK.
 *
 * You can redistribute it and/or modify it under the terms of the GNU
 * Lesser General Public License version 3 as published by the Free
 * Software Foundation.
 */

#include <lxldr/driver.h>

typedef struct
{
    PLX_SUBSYSTEM_CREATE_INITIAL_NAMESPACE CreateInitialNamespace;
    LIST_ENTRY ListEntry;
} SERVICE_INIT_ENTRY;

static EX_PUSH_LOCK ServiceListLock;
static LIST_ENTRY ServiceList;

NTSTATUS NTAPI LxldrRegisterService(
    PDRIVER_OBJECT DriverObject,
    BOOLEAN Register,
    PLX_SUBSYSTEM_CREATE_INITIAL_NAMESPACE CreateInitialNamespace)
{
    if (Register)
    {
        SERVICE_INIT_ENTRY *Init = ExAllocatePoolWithTag(NonPagedPool, sizeof *Init, 'LXLD');
        if (0 == Init)
            return STATUS_INSUFFICIENT_RESOURCES;

        RtlZeroMemory(Init, sizeof *Init);
        Init->CreateInitialNamespace = CreateInitialNamespace;

        KeEnterCriticalRegion();
        ExAcquirePushLockExclusive(&ServiceListLock);
        InsertTailList(&ServiceList, &Init->ListEntry);
        ExReleasePushLockExclusive(&ServiceListLock);
        KeLeaveCriticalRegion();

        return STATUS_SUCCESS;
    }
    else
    {
        SERVICE_INIT_ENTRY *Init = 0;

        KeEnterCriticalRegion();
        ExAcquirePushLockExclusive(&ServiceListLock);
        for (PLIST_ENTRY ListEntry = ServiceList.Flink;
            &ServiceList != ListEntry;
            ListEntry = ListEntry->Flink)
        {
            SERVICE_INIT_ENTRY *P = CONTAINING_RECORD(ListEntry, SERVICE_INIT_ENTRY, ListEntry);

            if (CreateInitialNamespace == P->CreateInitialNamespace)
            {
                Init = P;
                RemoveEntryList(ListEntry);
                break;
            }
        }
        ExReleasePushLockExclusive(&ServiceListLock);
        KeLeaveCriticalRegion();

        if (0 != Init)
            ExFreePoolWithTag(Init, 'LXLD');

        return STATUS_SUCCESS;
    }
}

static NTSTATUS LoadDriver(
    PUNICODE_STRING Name)
{
    WCHAR PathBuf[64 + 256];
    UNICODE_STRING Path;
    NTSTATUS Status;

    RtlInitEmptyUnicodeString(&Path, PathBuf, sizeof PathBuf);
    Status = RtlAppendUnicodeToString(&Path,
        L"\\Registry\\Machine\\System\\CurrentControlSet\\Services\\");
    Status = NT_SUCCESS(Status) ? RtlAppendUnicodeStringToString(&Path, Name) : Status;
    if (!NT_SUCCESS(Status))
    {
        Status = STATUS_INVALID_PARAMETER;
        goto exit;
    }

    Status = ZwLoadDriver(&Path);
    if (!NT_SUCCESS(Status) && STATUS_IMAGE_ALREADY_LOADED != Status)
        goto exit;

    Status = STATUS_SUCCESS;

exit:
    return Status;
}

static NTSTATUS LoadService(
    HANDLE Root,
    PUNICODE_STRING Name,
    PVOID Context)
{
    UNICODE_STRING RegistryValueName;
    union
    {
        KEY_VALUE_PARTIAL_INFORMATION V;
        UINT8 B[FIELD_OFFSET(KEY_VALUE_PARTIAL_INFORMATION, Data) + 256];
    } RegistryValue;
    ULONG RegistryValueLength;
    UNICODE_STRING DependName;
    NTSTATUS Status;

    RtlInitUnicodeString(&RegistryValueName, L"Depends");
    RegistryValueLength = sizeof RegistryValue - 2 * sizeof(WCHAR);
        /* leave space for MULTI_SZ term-0's that will be added below (see RtlZeroMemory) */
    Status = RegistryGetValue(Root, Name, &RegistryValueName, &RegistryValue.V, &RegistryValueLength);
    if (NT_SUCCESS(Status))
    {
        if (REG_SZ == RegistryValue.V.Type || REG_MULTI_SZ == RegistryValue.V.Type)
        {
            ASSERT(sizeof RegistryValue > RegistryValueLength);
            RtlZeroMemory(RegistryValue.B + RegistryValueLength,
                sizeof RegistryValue - RegistryValueLength);
                /* ensure that MULTI_SZ term-0's always exist */
            for (PWSTR P = (PWSTR)RegistryValue.V.Data; L'\0' != *P; P += wcslen(P) + 1)
            {
                RtlInitUnicodeString(&DependName, P);
                Status = LoadDriver(&DependName);
                if (!NT_SUCCESS(Status))
                {
                    LOG(": \"%wZ\": while loading dependency \"%wZ\": error = %lx",
                        Name, DependName, Status);
                    goto exit;
                }
            }
        }
    }

    Status = LoadDriver(Name);
    if (!NT_SUCCESS(Status))
    {
        LOG(": \"%wZ\": while loading: error = %lx", Name, Status);
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
    UNICODE_STRING RegistryPath;
    INT Error;
    NTSTATUS Status;

    /* load services */
    RtlInitUnicodeString(&RegistryPath, L"" LXDK_REGPATH_SERVICES);
    Status = RegistryEnumerateKeys(0, &RegistryPath, LoadService, 0);
    if (!NT_SUCCESS(Status))
        /* ignore */;

    /* initialize services for this LX instance */
    KeEnterCriticalRegion();
    ExAcquirePushLockExclusive(&ServiceListLock);
    for (PLIST_ENTRY ListEntry = ServiceList.Flink;
        &ServiceList != ListEntry;
        ListEntry = ListEntry->Flink)
    {
        SERVICE_INIT_ENTRY *P = CONTAINING_RECORD(ListEntry, SERVICE_INIT_ENTRY, ListEntry);

        Error = P->CreateInitialNamespace(Instance);
        if (0 > Error)
            /* ignore */;
    }
    ExReleasePushLockExclusive(&ServiceListLock);
    KeLeaveCriticalRegion();

    return 0;
}

NTSTATUS DriverEntry(
    PDRIVER_OBJECT DriverObject,
    PUNICODE_STRING RegistryPath)
{
#if DBG
    if (!KD_DEBUGGER_NOT_PRESENT)
        DbgBreakPoint();
#endif

    static LX_SUBSYSTEM Subsystem = { CreateInitialNamespace };
    NTSTATUS Status;

    ExInitializePushLock(&ServiceListLock);
    InitializeListHead(&ServiceList);

    Status = LxInitialize(DriverObject, &Subsystem);
    if (STATUS_TOO_LATE == Status)
        Status = STATUS_SUCCESS;

    return Status;
}
