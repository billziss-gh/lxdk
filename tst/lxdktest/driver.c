/**
 * @file lxdktest/driver.c
 *
 * @copyright 2019 Bill Zissimopoulos
 */

#include <ntifs.h>
#include <lxdk/lxdk.h>

#pragma warning(disable:4100)           /* unreferenced formal parameter */

static INT CreateInitialNamespace(
    PLX_INSTANCE Instance)
{
    return 0;
}

NTSTATUS DriverEntry(
    PDRIVER_OBJECT DriverObject,
    PUNICODE_STRING RegistryPath)
{
    return LxldrRegisterService(DriverObject, TRUE, CreateInitialNamespace);
}
