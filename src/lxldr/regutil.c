/**
 * @file lxldr/regutil.c
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

NTSTATUS RegistryGetValue(
    HANDLE Root,
    PUNICODE_STRING Path,
    PUNICODE_STRING ValueName,
    PKEY_VALUE_PARTIAL_INFORMATION ValueInformation,
    PULONG PValueInformationLength)
{
    OBJECT_ATTRIBUTES ObjectAttributes;
    HANDLE Handle = 0;
    NTSTATUS Status;

    InitializeObjectAttributes(&ObjectAttributes,
        Path, OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE, Root, 0);

    Status = ZwOpenKey(&Handle, KEY_QUERY_VALUE, &ObjectAttributes);
    if (!NT_SUCCESS(Status))
        goto exit;

    Status = ZwQueryValueKey(Handle, ValueName,
        KeyValuePartialInformation, ValueInformation,
        *PValueInformationLength, PValueInformationLength);
    if (!NT_SUCCESS(Status))
        goto exit;

    Status = STATUS_SUCCESS;

exit:
    if (0 != Handle)
        ZwClose(Handle);

    return Status;
}

NTSTATUS RegistryEnumerateKeys(
    HANDLE Root,
    PUNICODE_STRING Path,
    NTSTATUS (*Func)(
        HANDLE Root,
        PUNICODE_STRING Name,
        PVOID Context),
    PVOID Context)
{
    OBJECT_ATTRIBUTES ObjectAttributes;
    HANDLE Handle = 0;
    union
    {
        KEY_BASIC_INFORMATION V;
        UINT8 B[256];
    } BasicInfo;
    ULONG Length;
    UNICODE_STRING Name;
    NTSTATUS Status;

    InitializeObjectAttributes(&ObjectAttributes,
        Path, OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE, Root, 0);
    Status = ZwOpenKey(&Handle, KEY_READ, &ObjectAttributes);
    if (!NT_SUCCESS(Status))
        goto exit;

    for (ULONG I = 0;; I++)
    {
        Length = sizeof BasicInfo;
        Status = ZwEnumerateKey(Handle, I, KeyBasicInformation, &BasicInfo, Length, &Length);
        if (!NT_SUCCESS(Status))
        {
            if (STATUS_NO_MORE_ENTRIES == Status)
                break;
            else if (STATUS_BUFFER_OVERFLOW == Status)
                continue;
            goto exit;
        }

        Name.Length = Name.MaximumLength = (USHORT)BasicInfo.V.NameLength;
        Name.Buffer = BasicInfo.V.Name;
        Status = Func(Handle, &Name, Context);
        if (!NT_SUCCESS(Status))
            goto exit;
    }

    Status = STATUS_SUCCESS;

exit:
    if (0 != Handle)
        ZwClose(Handle);

    return Status;
}
