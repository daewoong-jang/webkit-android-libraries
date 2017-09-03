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

#include "win/win32_file.h"

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
    return Win32File::stat(_FileName, _Stat);
}

int __cdecl     fstat(int const _FileHandle, struct stat* const _Stat)
{
    return Win32File::fstat(_FileHandle, _Stat);
}

int __cdecl     lstat(char const* const _FileName, struct stat* const _Stat)
{
    return Win32File::lstat(_FileName, _Stat);
}

int __cdecl     chmod(_In_z_ const char * _Filename, int _AccessMode)
{
    return _chmod(_Filename, _AccessMode);
}

int __cdecl     chsize(_In_ int _FileHandle, _In_ long _Size)
{
    return Win32File::of(_FileHandle)->chsize(_Size);
}

int __cdecl     eof(_In_ int _FileHandle)
{
    return Win32File::of(_FileHandle)->eof();
}

long __cdecl    filelength(_In_ int _FileHandle)
{
    return Win32File::of(_FileHandle)->filelength();
}

int __cdecl     locking(_In_ int _FileHandle, _In_ int _LockMode, _In_ long _NumOfBytes)
{
    return Win32File::of(_FileHandle)->locking(_LockMode, _NumOfBytes);
}

char * __cdecl  mktemp(_Inout_z_ char * _TemplateName)
{
    return _mktemp(_TemplateName);
}

int __cdecl     setmode(_In_ int _FileHandle, _In_ int _Mode)
{
    return Win32File::of(_FileHandle)->setmode(_Mode);
}

int __cdecl     sopen(const char * _Filename, _In_ int _OpenFlag, _In_ int _ShareFlag, ...)
{
    va_list args;
    va_start(args, _ShareFlag);
    int _Mode = va_arg(args, int);
    int retval = Win32File::sopen(_Filename, _OpenFlag, _ShareFlag, _Mode);
    va_end(args);
    return retval;
}

long __cdecl    tell(_In_ int _FileHandle)
{
    return Win32File::of(_FileHandle)->tell();
}

int __cdecl     umask(_In_ int _Mode)
{
    return _umask(_Mode);
}

}

#endif
