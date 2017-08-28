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

#include "unistd.h"

#if defined(WIN32) || defined(_WINDOWS)

extern "C" {

int __cdecl     getpid(void)
{
    return ::GetCurrentProcessId();
}

char *  __cdecl strdup(_In_opt_z_ const char * _Src)
{
    return _strdup(_Src);
}

int __cdecl     stat(char const* const _FileName, struct stat* const _Stat)
{
    _STATIC_ASSERT(sizeof(struct stat) == sizeof(struct _stat64i32));
    DWORD attr = GetFileAttributesA(_FileName);
    if (attr ==  INVALID_FILE_ATTRIBUTES)
        return -1;
    int retval = _stat64i32(_FileName, (struct _stat64i32*)_Stat);
    if (attr & FILE_ATTRIBUTE_REPARSE_POINT)
        _Stat->st_mode = _S_IFLNK;
    return retval;
}

}

#endif
