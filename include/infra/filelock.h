#pragma once

#include "infra/exception.h"
#include "infra/filesystem.h"

#ifdef _WIN32
#include <Windows.h>
#include <io.h>
#else
#include <cstdio>
#include <fcntl.h>
#endif

namespace inf {

//! A file lock, is a mutual exclusion utility similar to a mutex using a
//! file.
//! A file lock can't guarantee synchronization between threads of the same
//! process so just use file locks to synchronize threads from different processes.
class FileLock
{
#ifdef _WIN32
    static inline const char* OpenMode = "r";
#else
    static inline const char* OpenMode = "w";
#endif

public:
    FileLock() noexcept = default;

    //! Opens a file lock. Throws interprocess_exception if the file does not
    //! exist or there are no operating system resources.
    FileLock(const fs::path& path)
    : _file(path, OpenMode)
    {
        if (_file.get() == nullptr) {
            file::touch(path);
            _file = file::Handle(path, OpenMode);
            if (_file.get() == nullptr) {
                throw RuntimeError("Failed to obtain file handle: ec={} ({})", errno, strerror(errno));
            }
        }
    }

    //! Moves the ownership of "moved"'s file mapping object to *this.
    //! After the call, "moved" does not represent any file mapping object.
    //! Does not throw
    FileLock(FileLock&& moved) noexcept
    {
        std::swap(_file, moved._file);
    }

    //! Moves the ownership of "moved"'s file mapping to *this.
    //! After the call, "moved" does not represent any file mapping.
    //! Does not throw
    FileLock& operator=(FileLock&& moved) noexcept
    {
        unlock();
        std::swap(_file, moved._file);
        return *this;
    }

    //! Requires: The calling thread does not own the mutex.
    //!
    //! Effects: The calling thread tries to obtain exclusive ownership of the mutex,
    //!    and if another thread has exclusive, or sharable ownership of
    //!    the mutex, it waits until it can obtain the ownership.
    //! Throws: interprocess_exception on error.
    //!
    //! Note: A program may deadlock if the thread that has ownership calls
    //!    this function. If the implementation can detect the deadlock,
    //!    an exception could be thrown.
    void lock()
    {
#ifdef _WIN32
        static OVERLAPPED overlapped;
        std::memset(&overlapped, 0, sizeof(overlapped));
        const auto len = ((unsigned long)-1);
        auto handle    = reinterpret_cast<HANDLE>(_get_osfhandle(_fileno(_file.get())));

        if (LockFileEx(handle, LOCKFILE_EXCLUSIVE_LOCK, 0, len, len, &overlapped) == FALSE) {
            throw RuntimeError("Failed to lock file: ec={}", GetLastError());
        }
#else
        struct ::flock lock;
        lock.l_type   = F_WRLCK;
        lock.l_whence = SEEK_SET;
        lock.l_start  = 0;
        lock.l_len    = 0;

        auto fd = fileno(_file.get());
        if (fd == -1) {
            throw RuntimeError("Failed to obtain file handle: ec={} ({})", errno, strerror(errno));
        }

        if (-1 == ::fcntl(fd, F_SETLKW, &lock)) {
            throw RuntimeError("Failed to lock file: ec={} ({})", errno, strerror(errno));
        }
#endif
    }

    //! Requires: The calling thread does not own the mutex.
    //!
    //! Effects: The calling thread tries to acquire exclusive ownership of the mutex
    //!    without waiting. If no other thread has exclusive, or sharable
    //!    ownership of the mutex this succeeds.
    //! Returns: If it can acquire exclusive ownership immediately returns true.
    //!    If it has to wait, returns false.
    //! Throws: interprocess_exception on error.
    //!
    //! Note: A program may deadlock if the thread that has ownership calls
    //!    this function. If the implementation can detect the deadlock,
    //!    an exception could be thrown.
    bool try_lock()
    {
        bool acquired = true;
#ifdef _WIN32
        static OVERLAPPED overlapped;
        std::memset(&overlapped, 0, sizeof(overlapped));
        const auto len = ((unsigned long)-1);
        auto handle    = reinterpret_cast<HANDLE>(_get_osfhandle(_fileno(_file.get())));

        if (LockFileEx(handle, LOCKFILE_EXCLUSIVE_LOCK | LOCKFILE_FAIL_IMMEDIATELY, 0, len, len, &overlapped) == FALSE) {
            acquired = false;
            if (auto lastError = GetLastError(); lastError != ERROR_LOCK_VIOLATION) {
                throw RuntimeError("Failed to try lock file: ec={}", lastError);
            }
        }
#else
        struct ::flock lock;
        lock.l_type   = F_WRLCK;
        lock.l_whence = SEEK_SET;
        lock.l_start  = 0;
        lock.l_len    = 0;

        auto fd = fileno(_file.get());
        if (fd == -1) {
            throw RuntimeError("Failed to obtain file handle: ec={} ({})", errno, strerror(errno));
        }

        int ret = ::fcntl(fd, F_SETLK, &lock);
        if (ret == -1) {
            acquired = false;
            if (errno != EAGAIN && errno != EACCES) {
                throw RuntimeError("Failed to try lock file: ec={} ({})", errno, strerror(errno));
            }
        }
#endif
        return acquired;
    }

    //! Precondition: The thread must have exclusive ownership of the mutex.
    //! Effects: The calling thread releases the exclusive ownership of the mutex.
    //! Throws: An exception derived from interprocess_exception on error.
    void unlock()
    {
#ifdef _WIN32
        const auto len = ((unsigned long)-1);
        static OVERLAPPED overlapped;
        std::memset(&overlapped, 0, sizeof(overlapped));
        auto handle = reinterpret_cast<HANDLE>(_get_osfhandle(_fileno(_file.get())));

        if (UnlockFileEx(handle, 0, len, len, &overlapped) == FALSE) {
            throw RuntimeError("Failed to try lock file: ec={}", GetLastError());
        }
#else
        struct ::flock lock;
        lock.l_type   = F_UNLCK;
        lock.l_whence = SEEK_SET;
        lock.l_start  = 0;
        lock.l_len    = 0;

        auto fd = fileno(_file.get());
        if (fd == -1) {
            throw RuntimeError("Failed to obtain file handle: ec={} ({})", errno, strerror(errno));
        }

        if (-1 == ::fcntl(fd, F_SETLK, &lock)) {
            throw RuntimeError("Failed to try lock file: ec={} ({})", errno, strerror(errno));
        }
#endif
    }

private:
    file::Handle _file;
};
}
