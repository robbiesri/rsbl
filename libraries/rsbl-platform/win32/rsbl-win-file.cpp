// Copyright 2025 Robert Srinivasiah
// Licensed under the MIT License, see the LICENSE file for more info

#include "rsbl-file.h"

#include <windows.h>

namespace rsbl
{

Result<FileHandle> OpenFile(const char* path, FileOpenMode mode)
{
    DWORD desiredAccess = 0;
    DWORD creationDisposition = 0;

    switch (mode)
    {
    case FileOpenMode::Read:
        desiredAccess = GENERIC_READ;
        creationDisposition = OPEN_EXISTING;
        break;
    case FileOpenMode::Write:
        desiredAccess = GENERIC_WRITE;
        creationDisposition = CREATE_ALWAYS; // Truncates
        break;
    case FileOpenMode::WriteAppend:
        desiredAccess = GENERIC_WRITE;
        creationDisposition = OPEN_ALWAYS; // Does not truncate
        break;
    case FileOpenMode::ReadWrite:
        desiredAccess = GENERIC_READ | GENERIC_WRITE;
        creationDisposition = CREATE_ALWAYS; // Truncates
        break;
    case FileOpenMode::ReadWriteAppend:
        desiredAccess = GENERIC_READ | GENERIC_WRITE;
        creationDisposition = OPEN_ALWAYS; // Does not truncate
        break;
    }

    HANDLE handle = CreateFileA(path,
                                desiredAccess,
                                FILE_SHARE_READ, // Allow other processes to read
                                nullptr,         // Default security
                                creationDisposition,
                                FILE_ATTRIBUTE_NORMAL,
                                nullptr); // No template file

    if (handle == INVALID_HANDLE_VALUE)
    {
        return "Failed to open file";
    }

    // return Result<FileHandle>(reinterpret_cast<FileHandle>(handle));
    return reinterpret_cast<FileHandle>(handle);
}

Result<> CloseFile(FileHandle handle)
{
    HANDLE win_handle = reinterpret_cast<HANDLE>(handle);

    if (!::CloseHandle(win_handle))
    {
        return "Failed to close file";
    }

    return ResultCode::Success;
}

Result<uint64> WriteFile(FileHandle handle, const void* buffer, uint64 num_bytes_to_write)
{
    HANDLE winHandle = reinterpret_cast<HANDLE>(handle);
    DWORD bytesWritten = 0;

    // Windows API uses DWORD (32-bit) for write size
    if (num_bytes_to_write > MAXDWORD)
    {
        return "Write size exceeds maximum supported by Windows API";
    }

    BOOL success = ::WriteFile(winHandle,
                               buffer,
                               static_cast<DWORD>(num_bytes_to_write),
                               &bytesWritten,
                               nullptr); // Not using overlapped I/O

    if (!success)
    {
        return "Failed to write to file";
    }

    return Result<uint64>(static_cast<uint64>(bytesWritten));
}

Result<uint64> ReadFile(FileHandle handle, void* buffer, uint64 num_bytes_to_read, uint64 offset)
{
    HANDLE win_handle = reinterpret_cast<HANDLE>(handle);

    // Interestingly, Spectre mitigation cares if this is range checked (> 0)
    // I might just enable this branch unconditionally because why not? This isn't really a 'fast'
    // API.
    if (offset != 0)
    {
        // Set file pointer to the specified offset
        LARGE_INTEGER liOffset;
        liOffset.QuadPart = static_cast<LONGLONG>(offset);

        if (!SetFilePointerEx(win_handle, liOffset, nullptr, FILE_BEGIN))
        {
            return "Failed to seek to offset";
        }
    }

    DWORD bytesRead = 0;

    // Windows API uses DWORD (32-bit) for read size, so we need to handle large reads
    if (num_bytes_to_read > MAXDWORD)
    {
        return "Read size exceeds maximum supported by Windows API";
    }

    const BOOL success = ::ReadFile(win_handle,
                                    buffer,
                                    static_cast<DWORD>(num_bytes_to_read),
                                    &bytesRead,
                                    nullptr); // Not using overlapped I/O

    if (!success)
    {
        return "Failed to read from file";
    }

    return static_cast<uint64>(bytesRead);
}

Result<uint64> OpenAndReadFile(const char* path, void* buffer, uint64 num_bytes_to_read)
{
    auto openResult = rsbl::OpenFile(path, FileOpenMode::Read);
    if (openResult.Code() != ResultCode::Success)
    {
        return "Failed to open file for reading";
    }

    FileHandle handle = openResult.Value();

    auto readResult = rsbl::ReadFile(handle, buffer, num_bytes_to_read);

    // Always try to close, even if read failed
    auto closeResult = rsbl::CloseFile(handle);

    // Return read result if successful, otherwise return close error or read error
    if (readResult.Code() != ResultCode::Success)
    {
        return readResult;
    }

    if (closeResult.Code() != ResultCode::Success)
    {
        return "Read succeeded but failed to close file";
    }

    return readResult;
}

} // namespace rsbl
