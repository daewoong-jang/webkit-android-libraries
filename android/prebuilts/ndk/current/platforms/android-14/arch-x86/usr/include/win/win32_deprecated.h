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
#ifndef _WIN32_DEPRECATED_H_
#define _WIN32_DEPRECATED_H_

_CRT_BEGIN_C_HEADER

#define O_CREAT    _O_CREAT
#define O_EXCL     _O_EXCL
#define S_IRUSR    _S_IREAD
#define S_IWUSR    _S_IWRITE
#define S_IRGRP    _S_IREAD
#define S_IWGRP    _S_IWRITE
#define S_IXUSR    0
#define S_IXGRP    0
#define F_OK       0

#define _S_IFLNK   0x5000 // Hack: Directory + Pipe = Link

#define S_ISDIR(m) (((m) & _S_IFMT) == _S_IFDIR)
#define S_ISLNK(m) (((m) & _S_IFLNK) == _S_IFLNK)
#define S_ISREG(m) (((m) & _S_IFMT) == _S_IFREG)

extern int __cdecl stat(char const* const _FileName, struct stat* const _Stat);

#undef  mkdir
#define mkdir(a, b)     _mkdir(a)

#define fdopen          _fdopen
#define fileno          _fileno
#define getcwd          _getcwd
#define stricmp         _stricmp
#define strnicmp        _strnicmp
#define unlink          _unlink
#define rmdir           _rmdir
#define wcsicmp         _wcsicmp

#define _open_osfhandle win32_open_osfhandle
#define _get_osfhandle  win32_get_osfhandle

_CRT_END_C_HEADER

#endif /* _WIN32_DEPRECATED_H_ */
