/**
 * @file lxldr/driver.h
 *
 * @copyright 2019 Bill Zissimopoulos
 */

#define LXDK_LXLDR_INTERNAL
#include <ntifs.h>
#include <lxdk/lxdk.h>

#pragma warning(disable:4100)           /* unreferenced formal parameter */

#define LXDK_REGPATH                    "\\Registry\\Machine\\Software\\Lxdk"
#define LXDK_REGPATH_SERVICES           LXDK_REGPATH "\\Services"

#define LOG(Format, ...)                DbgPrint("%s" Format "\n", __FUNCTION__, __VA_ARGS__)

NTSTATUS RegistryEnumerateKeys(
    HANDLE Root,
    PUNICODE_STRING Path,
    NTSTATUS (*Func)(
        HANDLE Root,
        PUNICODE_STRING Name,
        PVOID Context),
    PVOID Context);
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
    PVOID Context);
