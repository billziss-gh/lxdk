/**
 * @file shared/minimal.h
 *
 * @copyright 2019 Bill Zissimopoulos
 */
/*
 * This file is part of LxDK.
 *
 * You can redistribute it and/or modify it under the terms of the GNU
 * Lesser General Public License version 3 as published by the Free
 * Software Foundation.
 */

#ifndef LXDK_SHARED_MINIMAL_H_INCLUDED
#define LXDK_SHARED_MINIMAL_H_INCLUDED

/*
 * Eliminate dependency on the MSVCRT libraries.
 *
 * For this to work the following project settings must be set:
 * - "C/C++ > General > SDL checks" must be empty (not "Yes" or "No").
 * - "C/C++ > Code Generation > Basic Runtime Checks" must be set to "Default"
 * - "C/C++ > Code Generation > Runtime Library" must be set to "Multi-threaded (/MT)".
 * - "C/C++ > Code Generation > Security Check" must be disabled (/GS-).
 * - "Linker > Input > Ignore All Default Libraries" must be "Yes".
 *
 *
 * Update 1:
 *
 * It is possible to have the "Linker > Input > Ignore All Default Libraries"
 * setting to "No" and still eliminate most dependencies on the MSVCRT libraries.
 * For example, the WinFsp DLL does this on 32-bit builds (only) to include the
 * __allmul symbol that is used when doing int64_t multiplications.
 *
 * The following project setting must be changed:
 * - "Linker > Input > Ignore All Default Libraries" must be "No".
 *
 * Extreme care must be taken to ensure that the linker does not pull in symbols
 * that are not required (or worse create a half-baked CRT). For example, the WinFsp
 * DLL ensures this by setting the "Linker > Input > Ignore All Default Libraries"
 * to "Yes" on 64-bit builds and "No" on 32-bit builds.
 *
 *
 * Update 2:
 *
 * Using the /Gs[size] compiler option with a large size is a very bad idea.
 * Turns out that the compiler uses the _chkstk call to ensure that enough
 * stack space has been committed even when a function accesses locations in
 * the stack below the guard page.
 *
 * The following links explain the problem very well:
 * - http://stackoverflow.com/questions/8400118/what-is-the-purpose-of-the-chkstk-function#8400171
 * - https://support.microsoft.com/en-us/kb/100775
 *
 * A library/program that does not wish to use the MSVCRT libraries (and hence
 * does not have _chkstk available) must take care to not use more than a page
 * (4096 bytes) of stack within a single function.
 */

#undef RtlFillMemory
#undef RtlMoveMemory
NTSYSAPI VOID NTAPI RtlFillMemory(VOID *Destination, DWORD Length, BYTE Fill);
NTSYSAPI VOID NTAPI RtlMoveMemory(VOID *Destination, CONST VOID *Source, DWORD Length);

#pragma function(memset)
#pragma function(memcpy)
static inline
void *memset(void *dst, int val, size_t siz)
{
    RtlFillMemory(dst, (DWORD)siz, val);
    return dst;
}
static inline
void *memcpy(void *dst, const void *src, size_t siz)
{
    RtlMoveMemory(dst, src, (DWORD)siz);
    return dst;
}
static inline
void *memmove(void *dst, const void *src, size_t siz)
{
    RtlMoveMemory(dst, src, (DWORD)siz);
    return dst;
}

#define LXDK_SHARED_MINIMAL_STRCMP(NAME, TYPE, CONV)\
    static inline\
    int NAME(const TYPE *s, const TYPE *t)\
    {\
        int v = 0;\
        while (0 == (v = CONV(*s) - CONV(*t)) && *t)\
            ++s, ++t;\
        return v;/*(0 < v) - (0 > v);*/\
    }
#define LXDK_SHARED_MINIMAL_STRNCMP(NAME, TYPE, CONV)\
    static inline\
    int NAME(const TYPE *s, const TYPE *t, size_t n)\
    {\
        int v = 0;\
        const void *e = t + n;\
        while (e > (const void *)t && 0 == (v = CONV(*s) - CONV(*t)) && *t)\
            ++s, ++t;\
        return v;/*(0 < v) - (0 > v);*/\
    }
static inline
unsigned invariant_toupper(unsigned c)
{
    return ('a' <= c && c <= 'z') ? c & ~0x20 : c;
}
LXDK_SHARED_MINIMAL_STRCMP(invariant_strcmp, char, (unsigned))
LXDK_SHARED_MINIMAL_STRCMP(invariant_stricmp, char, invariant_toupper)
LXDK_SHARED_MINIMAL_STRNCMP(invariant_strncmp, char, (unsigned))
LXDK_SHARED_MINIMAL_STRNCMP(invariant_strnicmp, char, invariant_toupper)
LXDK_SHARED_MINIMAL_STRCMP(invariant_wcscmp, wchar_t, (unsigned))
LXDK_SHARED_MINIMAL_STRCMP(invariant_wcsicmp, wchar_t, invariant_toupper)
LXDK_SHARED_MINIMAL_STRNCMP(invariant_wcsncmp, wchar_t, (unsigned))
LXDK_SHARED_MINIMAL_STRNCMP(invariant_wcsnicmp, wchar_t, invariant_toupper)
#undef LXDK_SHARED_MINIMAL_STRCMP
#undef LXDK_SHARED_MINIMAL_STRNCMP

static inline void *MemAlloc(size_t Size)
{
    return HeapAlloc(GetProcessHeap(), 0, Size);
}
static inline void *MemRealloc(void *Pointer, size_t Size)
{
    return HeapReAlloc(GetProcessHeap(), 0, Pointer, Size);
}
static inline void MemFree(void *Pointer)
{
    if (0 != Pointer)
        HeapFree(GetProcessHeap(), 0, Pointer);
}

static FORCEINLINE
VOID InsertTailList(PLIST_ENTRY ListHead, PLIST_ENTRY Entry)
{
    PLIST_ENTRY Blink;

    Blink = ListHead->Blink;
    Entry->Flink = ListHead;
    Entry->Blink = Blink;
    Blink->Flink = Entry;
    ListHead->Blink = Entry;
}
static FORCEINLINE
BOOLEAN RemoveEntryList(PLIST_ENTRY Entry)
{
    PLIST_ENTRY Blink;
    PLIST_ENTRY Flink;

    Flink = Entry->Flink;
    Blink = Entry->Blink;
    Blink->Flink = Flink;
    Flink->Blink = Blink;
    return Flink == Blink;
}

#endif
