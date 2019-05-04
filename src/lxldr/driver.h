/**
 * @file lxldr/driver.h
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

#define LXDK_LDRSYM(RetType, Name, ...) __declspec(dllexport) RetType NTAPI Name ( __VA_ARGS__ );
#include <ntifs.h>
#include <lxdk/lxdk.h>

#pragma warning(disable:4100)           /* unreferenced formal parameter */

#define LOG(Format, ...)                DbgPrint("%s" Format "\n", __FUNCTION__, __VA_ARGS__)

#define LXDK_REGPATH                    "\\Registry\\Machine\\Software\\Lxdk"
#define LXDK_REGPATH_SERVICES           LXDK_REGPATH "\\Services"

NTSTATUS RegistryEnumerateKeys(
    HANDLE Root,
    PUNICODE_STRING Path,
    NTSTATUS (*Func)(
        HANDLE Root,
        PUNICODE_STRING Name,
        PVOID Context),
    PVOID Context);
