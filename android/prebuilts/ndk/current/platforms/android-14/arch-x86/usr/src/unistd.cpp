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

#include "sys/select.h"
#include "sys/socket.h"
#include "win/win32_file.h"

#include <errno.h>

pid_t gettid(void)
{
    return ::GetCurrentThreadId();
}

int access(const char * pathname, int mode)
{
    return _access(pathname, mode);
}

int link(const char * path1, const char * path2)
{
    return ::CreateHardLinkA(path2, path1, NULL) != 0 ? 0 : -1;
}

int pipe(int * pipefd)
{
    return socketpair(AF_UNIX, SOCK_STREAM, 0, pipefd);
}

int symlink(const char * target, const char * linkpath)
{
    DWORD attributes = GetFileAttributesA(target);
    if (attributes == INVALID_FILE_ATTRIBUTES) {
        errno = ENOENT;
        return -1;
    }

    if (!CreateSymbolicLinkA(linkpath, target, (attributes & FILE_ATTRIBUTE_DIRECTORY) ? SYMBOLIC_LINK_FLAG_DIRECTORY : 0)) {
        errno = EACCES;
        return -1;
    }

    return 0;
}

int close(int fildes)
{
    return (fildes > 0 && Win32File::of(fildes)) ? Win32File::of(fildes)->close() : EBADF;
}

off_t lseek(int fildes, off_t offset, int whence)
{
    return Win32File::of(fildes)->lseek(offset, whence);
}

int read(int fildes, void * buf, size_t nbyte)
{
    return Win32File::of(fildes)->read(buf, nbyte, 0);
}

int write(int fildes, const void * buf, size_t count)
{
    return Win32File::of(fildes)->write(buf, count, 0);
}

int dup(int fildes)
{
    return (fildes > 0) ? Win32File::of(fildes)->dup() : -1;
}

int dup2(int oldfd, int newfd)
{
    return (oldfd > 0) ? Win32File::of(oldfd)->dup(newfd) : -1;
}

int ioctl(int fd, int request, ssize_t* va)
{
    return ioctlsocket(fd, request, (u_long FAR *)va);
}

int ftruncate(int fildes, off_t length)
{
    return Win32File::of(fildes)->chsize(length);
}

unsigned sleep(unsigned seconds)
{
    Sleep(seconds * 1000);
    return 0;
}

int  gethostname(char * name, size_t len)
{
    return FORWARD_CALL(GETHOSTNAME)(name, len);
}

long getpagesize(void)
{
    static long g_pagesize = 0;
    if (!g_pagesize) {
        SYSTEM_INFO system_info;
        GetSystemInfo(&system_info);
        g_pagesize = system_info.dwPageSize;
    }
    return g_pagesize;
}

int isatty(int fildes)
{
    return Win32File::of(fildes)->isatty();
}

#endif
