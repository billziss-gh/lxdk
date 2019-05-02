/**
 * @file lxldr/regutil.c
 *
 * @copyright 2019 Bill Zissimopoulos
 */

#include <lxldr/driver.h>

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
        KEY_NAME_INFORMATION V;
        UINT8 B[256];
    } NameInfo;
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
        Status = ZwEnumerateKey(Handle, I, KeyNameInformation, &NameInfo, Length, &Length);
        if (!NT_SUCCESS(Status))
        {
            if (STATUS_NO_MORE_ENTRIES == Status)
                break;
            else if (STATUS_BUFFER_OVERFLOW == Status)
                continue;
            goto exit;
        }

        Name.Length = Name.MaximumLength = (USHORT)NameInfo.V.NameLength;
        Name.Buffer = NameInfo.V.Name;
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

NTSTATUS RegistryEnumerateKeyValues(
    HANDLE Root,
    PUNICODE_STRING Path,
    NTSTATUS (*Func)(
        HANDLE Root,
        PUNICODE_STRING Name,
        ULONG Type,
        PVOID Buffer,
        ULONG Length,
        PVOID Context),
    PVOID Context)
{
    OBJECT_ATTRIBUTES ObjectAttributes;
    HANDLE Handle = 0;
    union
    {
        KEY_VALUE_FULL_INFORMATION V;
        UINT8 B[1024];
    } ValueInfo;
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
        Status = ZwEnumerateValueKey(Handle, I, KeyValueFullInformation, &ValueInfo, Length, &Length);
        if (!NT_SUCCESS(Status))
        {
            if (STATUS_NO_MORE_ENTRIES == Status)
                break;
            else if (STATUS_BUFFER_OVERFLOW == Status)
                continue;
            goto exit;
        }

        Name.Length = Name.MaximumLength = (USHORT)ValueInfo.V.NameLength;
        Name.Buffer = ValueInfo.V.Name;
        Status = Func(Handle, &Name, ValueInfo.V.Type, (PUINT8)&ValueInfo, ValueInfo.V.DataLength,
            Context);
        if (!NT_SUCCESS(Status))
            goto exit;
    }

    Status = STATUS_SUCCESS;

exit:
    if (0 != Handle)
        ZwClose(Handle);

    return Status;
}
