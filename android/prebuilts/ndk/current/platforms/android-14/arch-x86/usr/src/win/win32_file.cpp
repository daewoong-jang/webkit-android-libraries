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

#include "win32_file.h"

#if defined(WIN32) || defined(_WINDOWS)

#include "fcntl.h"
#include "osf.h"
#include "sys/select.h"
#include <errno.h>
#include <map>
#include <mutex>
#include <vector>
#include <cutils/log.h>

#undef _get_osfhandle
#undef _open_osfhandle

class FileMap {
public:
    static const int NewEntry = -1;

    static FileMap& shared();

    bool has(int) const;

    int set(int, Win32File*);
    Win32File* get(int) const;
    void erase(int);

    void lock();
    void unlock();

private:
    FileMap() = default;

    int slot();

    mutable std::mutex m_lock;
    std::map<int, Win32File*> m_files;
};

FileMap& FileMap::shared()
{
    static FileMap fileMap;
    return fileMap;
}

int FileMap::slot()
{
    std::lock_guard<std::mutex> lock(m_lock);
    int fd = m_files.size();
    while (m_files.count(++fd)) { }
    return fd;
}

bool FileMap::has(int fd) const
{
    if (fd == NewEntry)
        return false;

    std::lock_guard<std::mutex> lock(m_lock);
    return m_files.count(fd);
}

int FileMap::set(int fd, Win32File* file)
{
    if (fd == NewEntry)
        fd = slot();

    std::lock_guard<std::mutex> lock(m_lock);
    assert(m_files.count(fd) == 0);
    m_files[fd] = file;
    return fd;
}

Win32File* FileMap::get(int fd) const
{
    if (fd == NewEntry)
        return nullptr;

    std::lock_guard<std::mutex> lock(m_lock);
    return m_files.at(fd);
}

void FileMap::erase(int fd)
{
    if (fd == NewEntry)
        return;

    std::lock_guard<std::mutex> lock(m_lock);
    m_files.erase(fd);
}

void FileMap::lock()
{
    m_lock.lock();
}

void FileMap::unlock()
{
    m_lock.unlock();
}

Win32File::Win32File(void* fh, Type type, int fd)
    : m_fd(FileMap::shared().set(fd, this))
    , m_type(type)
    , m_handle(fh)
    , m_flags(0)
{
}

Win32File::~Win32File()
{
    FileMap::shared().erase(m_fd);
}

Win32File* Win32File::of(int fd)
{
    return FileMap::shared().get(fd);
}

bool Win32File::is(Type type) const
{
    return type == m_type;
}

bool Win32File::isValid() const
{
    return m_handle && m_handle != INVALID_HANDLE_VALUE;
}

int Win32File::fd() const
{
    return m_fd;
}

Win32File::Type Win32File::type() const
{
    return m_type;
}

void* Win32File::handle() const
{
    return m_handle;
}

void Win32File::setHandle(void* handle, Type type)
{
    m_handle = handle;
    m_type = type;
}

static int handleError(int error)
{
    errno = error;
    return -1;
}

static HANDLE duplicateHandle(pid_t sourcePid, HANDLE handle, bool close)
{
    HANDLE sourceProcess = (sourcePid == -1) ? ::GetCurrentProcess() : ::OpenProcess(PROCESS_DUP_HANDLE, FALSE, sourcePid);
    if (sourcePid != -1 && sourceProcess == INVALID_HANDLE_VALUE)
        return INVALID_HANDLE_VALUE;

    DWORD options = DUPLICATE_SAME_ACCESS;
    if (close)
        options |= DUPLICATE_CLOSE_SOURCE;

    HANDLE newHandle;
    BOOL ok = ::DuplicateHandle(sourceProcess, handle, ::GetCurrentProcess(), &newHandle, 0, FALSE, options);
    if (!ok)
        return INVALID_HANDLE_VALUE;

    ::CloseHandle(sourceProcess);
    return newHandle;
}

using UniqueFileDescriptor = std::unique_ptr<int, std::function<void (int*)>>;

static UniqueFileDescriptor osfd(int fd)
{
    return UniqueFileDescriptor(new int(fd), [](int* fdptr) {
        if (*fdptr >= 1)
            _close(*fdptr);
        delete fdptr;
    });
}

static UniqueFileDescriptor osfd(Win32File* file)
{
    assert(file->is(Win32File::Type::File) && file->isValid());

    HANDLE handle = duplicateHandle(-1, file->handle(), false);
    if (handle == INVALID_HANDLE_VALUE)
        return osfd(-1);

    return osfd(_open_osfhandle((intptr_t)handle, _O_BINARY));
}

int Win32File::open(void* handle, Type type, int fd, pid_t sourcePid)
{
    HANDLE myHandle = INVALID_HANDLE_VALUE;
    if (type != Type::Unknown) {
        if (!handle || handle == INVALID_HANDLE_VALUE)
            return -1;

        myHandle = sourcePid == -1 ? handle : duplicateHandle(sourcePid, handle, true);
        if (myHandle == INVALID_HANDLE_VALUE)
            return -1;
    }

    Win32File* file = new Win32File(myHandle, type, fd);
    return file->fd();
}

int Win32File::open(const char * filename, int openFlag, int permissionMode)
{
    auto fd = osfd(_open(filename, openFlag, permissionMode));
    if (*fd == -1)
        return -1;

    HANDLE handle = (HANDLE)_get_osfhandle(*fd);
    if (handle == INVALID_HANDLE_VALUE)
        return -1;

    HANDLE myHandle = duplicateHandle(-1, handle, true);
    if (myHandle == INVALID_HANDLE_VALUE)
        return -1;

    Win32File* file = new Win32File(myHandle, Type::File, FileMap::NewEntry);
    return file->fd();
}

int Win32File::sopen(const char * filename, int openFlag, int shareFlag, int permissionMode)
{
    auto fd = osfd(_sopen(filename, openFlag, shareFlag, permissionMode));
    if (*fd == -1)
        return -1;

    HANDLE handle = (HANDLE)_get_osfhandle(*fd);
    if (handle == INVALID_HANDLE_VALUE)
        return -1;

    HANDLE myHandle = duplicateHandle(-1, handle, true);
    if (myHandle == INVALID_HANDLE_VALUE)
        return -1;

    Win32File* file = new Win32File(myHandle, Type::File, FileMap::NewEntry);
    return file->fd();
}

class WSAInitializer {
public:
    WSAInitializer() {
        memset(&m_wsaData, 0, sizeof(WSADATA));
        WORD versionRequired = MAKEWORD(2, 2);
        m_error = WSAStartup(versionRequired, &m_wsaData);
        if (m_error != 0) {
            ALOGE("WSAStartup failed with error: %d\n", WSAGetLastError());
        }
    }
    ~WSAInitializer() {
        if (m_error == 0)
            FORWARD_CALL(WSACLEANUP)();
    }

    int error() const { return m_error; }

private:
    WSADATA m_wsaData;
    int m_error;
};

static std::unique_ptr<WSAInitializer> lazyWSAInitializer;

static int WSAInitialize()
{
    if (!lazyWSAInitializer)
        lazyWSAInitializer = std::make_unique<WSAInitializer>();
    return lazyWSAInitializer->error();
}

static int handleWSALastError()
{
    switch (WSAGetLastError()) {
    case WSAEWOULDBLOCK:
        errno = EWOULDBLOCK;
        break;
    case WSAEINTR:
        errno = EINTR;
        break;
    case WSAEINVAL:
        errno = EINVAL;
        break;
    case WSAECONNRESET:
        errno = ECONNRESET;
        break;
    case WSAECONNABORTED:
        errno = ECONNABORTED;
        break;
    default:
        errno = EBADF;
        break;
    }

    return -1;
}

int Win32File::socket(int addressFamily, int type, int protocol)
{
    if (WSAInitialize() != 0)
        return -1;

    SOCKET sock = FORWARD_CALL(SOCKET)(addressFamily, type, protocol);
    if (sock == -1)
        return handleWSALastError();

    Win32File* file = new Win32File((void*)sock, Type::Socket, FileMap::NewEntry);
    return file->fd();
}

extern "C" int dumb_socketpair(SOCKET socks[2], int make_overlapped); // socketpair.c

int Win32File::socketpair(int* fds, int make_overlapped)
{
    if (WSAInitialize() != 0)
        return -1;

    SOCKET sockets[2];
    if (dumb_socketpair(sockets, make_overlapped) == SOCKET_ERROR)
        return handleWSALastError();

    fds[0] = Win32File::open((void*)sockets[0], Type::Socket, FileMap::NewEntry, -1);
    fds[1] = Win32File::open((void*)sockets[1], Type::Socket, FileMap::NewEntry, -1);
    return 0;
}

int Win32File::accept(struct sockaddr * addr, int * addrlen)
{
    if (!isValid())
        return handleError(EBADF);

    if (!is(Type::Socket))
        return handleError(EBADF);

    SOCKET sock = FORWARD_CALL(ACCEPT)((SOCKET)m_handle, addr, addrlen);
    if (sock == -1)
        return handleWSALastError();

    Win32File* file = new Win32File((void*)sock, Type::Socket, FileMap::NewEntry);
    return file->fd();
}

int Win32File::close()
{
    if (!isValid()) {
        delete this;
        return handleError(EBADF);
    }

    int result = 0;
    switch (m_type) {
    case Type::File:
    case Type::Map:
        if (!::CloseHandle(m_handle))
            result = handleError(EBADF);
        break;
    case Type::Socket:
        result = FORWARD_CALL(CLOSESOCKET)((SOCKET)m_handle);
        if (result < 0)
            result = handleWSALastError();
        break;
    }

    release();
    return result;
}

void* Win32File::release()
{
    void* handle = nullptr;
    std::swap(handle, m_handle);
    delete this;
    return handle;
}

long Win32File::tell()
{
    if (!isValid())
        return handleError(EBADF);

    return _tell(*osfd(this));
}

off_t Win32File::lseek(off_t offset, int whence)
{
    if (!isValid())
        return handleError(EBADF);

    return _lseek(*osfd(this), offset, whence);
}

int Win32File::read(void * buf, size_t nbyte, unsigned int flags)
{
    if (!isValid())
        return handleError(EBADF);

    int result = 0;
    DWORD bytesRead = 0;
    switch (m_type) {
    case Type::File:
        if (!::ReadFile(m_handle, buf, nbyte, &bytesRead, 0))
            return handleError(EBADF);
        result = bytesRead;
        break;
    case Type::Socket:
        result = FORWARD_CALL(RECV)((SOCKET)m_handle, (char*) buf, nbyte, flags);
        if (result < 0)
            return handleWSALastError();
        break;
    }

    return result;
}

int Win32File::write(const void * buf, size_t count, unsigned int flags)
{
    if (!isValid())
        return handleError(EBADF);

    int result = 0;
    DWORD bytesWritten = 0;
    switch (m_type) {
    case Type::File:
        if (!::WriteFile(m_handle, buf, count, &bytesWritten, 0))
            return handleError(EBADF);
        result = bytesWritten;
        break;
    case Type::Socket:
        result = FORWARD_CALL(SEND)((SOCKET)m_handle, (char*) buf, count, flags);
        if (result < 0)
            return handleWSALastError();
        break;
    }

    return result;
}

int Win32File::eof()
{
    if (!isValid())
        return handleError(EBADF);

    return _eof(*osfd(this));
}

int Win32File::dup(int newfd)
{
    if (!isValid())
        return handleError(EBADF);

    Win32File* oldFile = Win32File::of(newfd);
    if (oldFile)
        oldFile->close();

    HANDLE newHandle = duplicateHandle(-1, m_handle, false);
    if (newHandle == INVALID_HANDLE_VALUE)
        return handleError(EBADF);

    Win32File* file = new Win32File(newHandle, m_type, newfd);
    return file->fd();
}

int Win32File::stat(const char * filename, struct stat* out)
{
    _STATIC_ASSERT(sizeof(struct stat) == sizeof(struct _stat64i32));
    return _stat64i32(filename, (struct _stat64i32*)out);
}

int Win32File::fstat(int handle, struct stat* out)
{
    _STATIC_ASSERT(sizeof(struct stat) == sizeof(struct _stat64i32));
    Win32File* file = Win32File::of(handle);
    if (!file)
        return -1;

    return _fstat64i32(*osfd(file), (struct _stat64i32*)out);
}

// lstat() implementation based on https://github.com/git-for-windows/git/blob/master/compat/mingw.c
#define MAX_LONG_PATH 4096

static bool isPathSeparator(char c)
{
    return c == '/' || c == '\\';
}

static int hasValidDirectoryPrefix(char * path)
{
    int n = strlen(path);

    while (n > 0) {
        wchar_t c = path[--n];
        DWORD attributes;

        if (!isPathSeparator(c))
            continue;

        path[n] = L'\0';
        attributes = GetFileAttributesA(path);
        path[n] = c;
        if (attributes == FILE_ATTRIBUTE_DIRECTORY ||
            attributes == FILE_ATTRIBUTE_DEVICE)
            return 1;
        if (attributes == INVALID_FILE_ATTRIBUTES)
            switch (GetLastError()) {
            case ERROR_PATH_NOT_FOUND:
                continue;
            case ERROR_FILE_NOT_FOUND:
                return 1;
            }
        return 0;
    }
    return 1;
}

static int handleLstatLastError(char * filename)
{
    switch (GetLastError()) {
    case ERROR_ACCESS_DENIED:
    case ERROR_SHARING_VIOLATION:
    case ERROR_LOCK_VIOLATION:
    case ERROR_SHARING_BUFFER_EXCEEDED:
        errno = EACCES;
        break;
    case ERROR_BUFFER_OVERFLOW:
        errno = ENAMETOOLONG;
        break;
    case ERROR_NOT_ENOUGH_MEMORY:
        errno = ENOMEM;
        break;
    case ERROR_PATH_NOT_FOUND:
        if (!hasValidDirectoryPrefix(filename)) {
            errno = ENOTDIR;
            break;
        }
    default:
        errno = ENOENT;
        break;
    }
    return -1;
}

static int fileAttributeToMode(DWORD attr, DWORD tag)
{
    int fMode = S_IREAD;
    if ((attr & FILE_ATTRIBUTE_REPARSE_POINT) && tag == IO_REPARSE_TAG_SYMLINK)
        fMode |= _S_IFLNK;
    else if (attr & FILE_ATTRIBUTE_DIRECTORY)
        fMode |= S_IFDIR;
    else
        fMode |= S_IFREG;
    if (!(attr & FILE_ATTRIBUTE_READONLY))
        fMode |= S_IWRITE;
    return fMode;
}

static void fileTimeToTime(const FILETIME *ft, time_t* tt)
{
    long long filetime = ((long long)ft->dwHighDateTime << 32) + ft->dwLowDateTime;
    *tt = filetime - 116444736000000000LL;
}

int Win32File::lstat(const char * filename, struct stat* out)
{
    _STATIC_ASSERT(sizeof(struct stat) == sizeof(struct _stat64i32));
    int len = strlen(filename);
    std::vector<char> strvec;
    strvec.reserve(len + 1);
    strncpy(strvec.data(), filename, len);

    char* path = strvec.data();
    while (len && isPathSeparator(path[len - 1]))
        path[--len] = 0;
    if (!len)
        return handleError(ENOENT);

    WIN32_FILE_ATTRIBUTE_DATA attributes;
    WIN32_FIND_DATAA finddata;
    ZeroMemory(&finddata, sizeof(finddata));

    if (GetFileAttributesExA(path, GetFileExInfoStandard, &attributes)) {
        if (attributes.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT) {
            HANDLE fh = FindFirstFileA(path, &finddata);
            if (fh == INVALID_HANDLE_VALUE)
                return handleLstatLastError(path);
            FindClose(fh);
        }
        out->st_ino = 0;
        out->st_gid = 0;
        out->st_uid = 0;
        out->st_nlink = 1;
        out->st_mode = fileAttributeToMode(attributes.dwFileAttributes, finddata.dwReserved0);
        out->st_size = S_ISLNK(out->st_mode) ? MAX_LONG_PATH : attributes.nFileSizeLow;
        out->st_dev = out->st_rdev = 0;
        fileTimeToTime(&attributes.ftLastAccessTime, &out->st_atime);
        fileTimeToTime(&attributes.ftLastWriteTime, &out->st_mtime);
        fileTimeToTime(&attributes.ftCreationTime, &out->st_ctime);
        return 0;
    }

    return handleLstatLastError(path);
}

int Win32File::isatty()
{
    if (!isValid())
        return handleError(EBADF);

    return _isatty(*osfd(this));
}

int Win32File::fcntl(int command, int flags)
{
    if (!isValid())
        return handleError(EBADF);

    switch (command) {
    case F_GETFD:
        DWORD handleFlags;
        ::GetHandleInformation(m_handle, &handleFlags);
        return (handleFlags & HANDLE_FLAG_INHERIT) == 0 ? 0 : FD_CLOEXEC;
    case F_SETFD:
        if (flags & FD_CLOEXEC)
            ::SetHandleInformation(m_handle, HANDLE_FLAG_INHERIT, 0);
        else
            ::SetHandleInformation(m_handle, HANDLE_FLAG_INHERIT, HANDLE_FLAG_INHERIT);
        return 0;
    case F_GETFL:
        if (flags & O_NONBLOCK)
            return m_flags & O_NONBLOCK;
        return 0;
    case F_SETFL:
        if (flags & O_NONBLOCK && !(m_flags & O_NONBLOCK)) {
            u_long iMode = 1;
            if (FORWARD_CALL(IOCTLSOCKET)((SOCKET)m_handle, FIONBIO, &iMode) == SOCKET_ERROR)
                return handleWSALastError();
            m_flags |= O_NONBLOCK;
        }
        return 0;
    }

    errno = EBADF;
    return -1;
}

int Win32File::chsize(long size)
{
    if (!isValid())
        return handleError(EBADF);

    return _chsize(*osfd(this), size);
}

long Win32File::filelength()
{
    if (!isValid())
        return handleError(EBADF);

    u_long length = 0;
    switch (m_type) {
    case Type::File:
        return _filelength(*osfd(this));
    case Type::Socket:
        if (FORWARD_CALL(IOCTLSOCKET)((SOCKET)m_handle, FIONREAD, &length) == SOCKET_ERROR)
            return handleWSALastError();
        return length;
    }

    return -1;
}

int Win32File::locking(int lockMode, long numOfBytes)
{
    if (!isValid())
        return handleError(EBADF);

    return _locking(*osfd(this), lockMode, numOfBytes);
}

int Win32File::setmode(int mode)
{
    if (!isValid())
        return handleError(EBADF);

    return _setmode(*osfd(this), mode);
}

#endif
