/*
 * Copyright (C) 2013 NAVER Corp. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *  * Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include "sys/mman.h"

#if defined(WIN32) || defined(_WINDOWS)

#include "win/osf.h"
#include "win/win32_file.h"

#define OffsetType off_t

// Implementation derived from: https://github.com/witwall/mman-win32/blob/master/mman.c

#include <windows.h>
#include <errno.h>
#include <io.h>

#ifndef FILE_MAP_EXECUTE
#define FILE_MAP_EXECUTE    0x0020
#endif /* FILE_MAP_EXECUTE */

static int __map_mman_error(const DWORD err, const int deferr)
{
    if (err == 0)
        return 0;
    //TODO: implement
    return err;
}

static DWORD __map_mmap_prot_page(const int prot)
{
    DWORD protect = 0;
    
    if (prot == PROT_NONE)
        return protect;
        
    if ((prot & PROT_EXEC) != 0)
    {
        protect = ((prot & PROT_WRITE) != 0) ? 
                    PAGE_EXECUTE_READWRITE : PAGE_EXECUTE_READ;
    }
    else
    {
        protect = ((prot & PROT_WRITE) != 0) ?
                    PAGE_READWRITE : PAGE_READONLY;
    }
    
    return protect;
}

static DWORD __map_mmap_prot_file(const int prot)
{
    DWORD desiredAccess = 0;
    
    if (prot == PROT_NONE)
        return desiredAccess;
        
    if ((prot & PROT_READ) != 0)
        desiredAccess |= FILE_MAP_READ;
    if ((prot & PROT_WRITE) != 0)
        desiredAccess |= FILE_MAP_WRITE;
    if ((prot & PROT_EXEC) != 0)
        desiredAccess |= FILE_MAP_EXECUTE;
    
    return desiredAccess;
}

void* mmap(void *addr, size_t len, int prot, int flags, int fildes, OffsetType off)
{
    HANDLE fm, h;
    
    void * map = MAP_FAILED;
    
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4293)
#endif

    const DWORD dwFileOffsetLow = (sizeof(OffsetType) <= sizeof(DWORD)) ?
                    (DWORD)off : (DWORD)(off & 0xFFFFFFFFL);
    const DWORD dwFileOffsetHigh = (sizeof(OffsetType) <= sizeof(DWORD)) ?
                    (DWORD)0 : (DWORD)((off >> 32) & 0xFFFFFFFFL);
    const DWORD protect = __map_mmap_prot_page(prot);
    const DWORD desiredAccess = __map_mmap_prot_file(prot);

    const OffsetType maxSize = off + (OffsetType)len;

    const DWORD dwMaxSizeLow = (sizeof(OffsetType) <= sizeof(DWORD)) ?
                    (DWORD)maxSize : (DWORD)(maxSize & 0xFFFFFFFFL);
    const DWORD dwMaxSizeHigh = (sizeof(OffsetType) <= sizeof(DWORD)) ?
                    (DWORD)0 : (DWORD)((maxSize >> 32) & 0xFFFFFFFFL);

    Win32File* const file = Win32File::of(fildes);

#ifdef _MSC_VER
#pragma warning(pop)
#endif

    errno = 0;
    
    if (len == 0 
        /* Unsupported flag combinations */
        || (flags & MAP_FIXED) != 0
        /* Usupported protection combinations */
        || prot == PROT_EXEC)
    {
        errno = EINVAL;
        return MAP_FAILED;
    }

    if (file->is(Win32File::Type::Unknown))
        flags |= MAP_ANONYMOUS;

    h = ((flags & MAP_ANONYMOUS) == 0) ? 
                    (HANDLE)_get_osfhandle(fildes) : INVALID_HANDLE_VALUE;

    if ((flags & MAP_ANONYMOUS) == 0 && h == INVALID_HANDLE_VALUE)
    {
        errno = EBADF;
        return MAP_FAILED;
    }

    fm = file->is(Win32File::Type::Map) ? h : CreateFileMapping(h, NULL, protect, dwMaxSizeHigh, dwMaxSizeLow, NULL);

    if (fm == NULL)
    {
        errno = __map_mman_error(GetLastError(), EPERM);
        return MAP_FAILED;
    }
  
    map = MapViewOfFile(fm, desiredAccess, dwFileOffsetHigh, dwFileOffsetLow, len);

    if (file->is(Win32File::Type::Unknown))
        file->setHandle(fm, Win32File::Type::Map);
    else if (file->is(Win32File::Type::File))
        CloseHandle(fm);
  
    if (map == NULL)
    {
        errno = __map_mman_error(GetLastError(), EPERM);
        return MAP_FAILED;
    }

    return map;
}

int munmap(void *addr, size_t len)
{
    if (UnmapViewOfFile(addr))
        return 0;
        
    errno =  __map_mman_error(GetLastError(), EPERM);
    
    return -1;
}

int mprotect(void *addr, size_t len, int prot)
{
    DWORD newProtect = __map_mmap_prot_page(prot);
    DWORD oldProtect = 0;
    
    if (VirtualProtect(addr, len, newProtect, &oldProtect))
        return 0;
    
    errno =  __map_mman_error(GetLastError(), EPERM);
    
    return -1;
}

int msync(void *addr, size_t len, int flags)
{
    if (FlushViewOfFile(addr, len))
        return 0;
    
    errno =  __map_mman_error(GetLastError(), EPERM);
    
    return -1;
}

int mlock(const void *addr, size_t len)
{
    if (VirtualLock((LPVOID)addr, len))
        return 0;
        
    errno =  __map_mman_error(GetLastError(), EPERM);
    
    return -1;
}

int munlock(const void *addr, size_t len)
{
    if (VirtualUnlock((LPVOID)addr, len))
        return 0;
        
    errno =  __map_mman_error(GetLastError(), EPERM);
    
    return -1;
}

#endif
