/**
 * @file lxldr/driver.c
 *
 * @copyright 2019 Bill Zissimopoulos
 */

#include <ntifs.h>
#include <lxdk/lxdk.h>

static INT CreateInitialNamespace(
    PLX_INSTANCE Instance)
{
    UNREFERENCED_PARAMETER(Instance);

    return 0;
}

static LX_SUBSYSTEM Subsystem =
{
    CreateInitialNamespace,
};

NTSTATUS DriverEntry(
    PDRIVER_OBJECT DriverObject,
    PUNICODE_STRING RegistryPath)
{
    UNREFERENCED_PARAMETER(RegistryPath);

    return LxInitialize(DriverObject, &Subsystem);
}
