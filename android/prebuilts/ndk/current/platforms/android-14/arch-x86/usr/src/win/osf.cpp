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

#include "osf.h"

#if defined(WIN32) || defined(_WINDOWS)

#include "win32_file.h"

int         win32_open_osfhandle(intptr_t _OSFileHandle, int _Flags, int _FileHandle)
{
    return Win32File::open((void*)_OSFileHandle, Win32File::Type::File, _FileHandle, -1);
}

int         win32_open_osfhandle_with_type(intptr_t _OSFileHandle, int _Flags, int _FileHandle, _In_ int _FileType)
{
    return Win32File::open((void*)_OSFileHandle, (Win32File::Type)_FileType, _FileHandle, -1);
}

intptr_t    win32_release_osfhandle(int _FileHandle)
{
    return (intptr_t)Win32File::of(_FileHandle)->release();
}

intptr_t    win32_get_osfhandle(_In_ int _FileHandle)
{
    return (intptr_t)Win32File::of(_FileHandle)->handle();
}

#endif
