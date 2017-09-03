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

#include "dirent.h"

#if defined(WIN32) || defined(_WINDOWS)

#include <windows.h>
#include <errno.h>

struct DIR {
    intptr_t findhandle;
    struct _finddata_t finddata;
    struct dirent d;
    char overflow[MAX_PATH - sizeof(dirent::d_name)];
    char name[MAX_PATH];
};

static DIR* allocDir()
{
    DIR* dir = new DIR;
    dir->findhandle = -1;
    memset(&dir->d, 0, sizeof(dirent));
    dir->name[0] = 0;
    return dir;
}

static void freeDir(DIR* dir)
{
    delete dir;
}

DIR* opendir(const char* filename)
{
    errno = EINVAL;

    if (!filename || *filename == 0)
        return 0;

    DIR* dir = allocDir();
    size_t base_length = strlen(filename);
    const char *all = /* search pattern must end with suitable wildcard */
        strchr("/\\", filename[base_length - 1]) ? "*" : "/*";

    strcpy(dir->name, filename);
    strcat(dir->name, all);

    if ((dir->findhandle = _findfirst(dir->name, &dir->finddata)) == -1) {
        freeDir(dir);
        return 0;
    }

    errno = 0;
    return dir;
}

dirent* readdir(DIR* dir)
{
    errno = EBADF;

    if (!dir || dir->findhandle == -1)
        return 0;

    bool ok = false;
    while ((!dir->d.d_name || _findnext(dir->findhandle, &dir->finddata) != -1) && !ok) {
        HANDLE hfile = CreateFileA(dir->finddata.name, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
        if (hfile == INVALID_HANDLE_VALUE)
            continue;
        BY_HANDLE_FILE_INFORMATION info;
        if (GetFileInformationByHandle(hfile, &info) == FALSE)
            continue;
        dir->d.d_ino = (info.nFileIndexHigh << sizeof(DWORD)) | info.nFileIndexLow;
        dir->d.d_off = -1;
        dir->d.d_reclen = GetFullPathNameA(dir->finddata.name, MAX_PATH, dir->d.d_name, NULL);
        if (info.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
            dir->d.d_type = DT_DIR;
        else if (info.dwFileAttributes & FILE_ATTRIBUTE_NORMAL)
            dir->d.d_type = DT_REG;
        else if (info.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT)
            dir->d.d_type = DT_LNK;
        errno = 0;
        return &dir->d;
    }

    return 0;
}

int closedir(DIR* dir)
{
    if (!dir || dir->findhandle == -1)
        return 0;

    int retval = _findclose(dir->findhandle);
    freeDir(dir);

    if (retval == -1)
        errno = EBADF;

    return retval;
}

#endif
