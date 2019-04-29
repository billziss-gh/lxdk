/**
 * @file lxdk/lxdk.c
 *
 * @copyright 2019 Bill Zissimopoulos
 */

#include <ntifs.h>
#include <lxdk/lxdk.h>

NTSTATUS DriverEntry(
    PDRIVER_OBJECT DriverObject, PUNICODE_STRING RegistryPath)
{
    UNREFERENCED_PARAMETER(DriverObject);
    UNREFERENCED_PARAMETER(RegistryPath);

    return STATUS_SUCCESS;
}
