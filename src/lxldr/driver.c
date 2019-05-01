/**
 * @file lxldr/driver.c
 *
 * @copyright 2019 Bill Zissimopoulos
 */

#include <ntifs.h>
#include <lxdk/lxdk.h>

NTSTATUS DriverEntry(
    PDRIVER_OBJECT DriverObject, PUNICODE_STRING RegistryPath)
{
    UNREFERENCED_PARAMETER(RegistryPath);

    return LxInitialize(DriverObject, 0);
}
