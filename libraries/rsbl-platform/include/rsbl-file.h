// Copyright 2025 Robert Srinivasiah
// Licensed under the MIT License, see the LICENSE file for more info

#pragma once

#include <rsbl-int-types.h>
#include <rsbl-result.h>

namespace rsbl
{

enum class FileOpenMode : uint8
{
    // Read + Append = file required to exist
    // Write + ReadWrite = truncates file, meaning existence not necessary

    Read,
    Write,           // Truncates the file on opening for writes
    WriteAppend,     // Does not truncate the file, allowing you to modify it
    ReadWrite,       // Truncates the file on opening for writes, and allows read
    ReadWriteAppend, // Does not tructate file, modif + read
};

// TODO: worth considering returning something more complex instead of just handles
using FileHandle = uint64_t;

rsbl::Result<FileHandle> OpenFile(const char* path, FileOpenMode mode);
rsbl::Result<> CloseFile(FileHandle handle);
rsbl::Result<uint64> WriteFile(FileHandle handle, const void* buffer, uint64 num_bytes_to_write);
rsbl::Result<uint64> ReadFile(FileHandle handle, void* buffer, uint64 num_bytes_to_read);

// TODO: just support one entry point, default arg for offset?
rsbl::Result<uint64> ReadFile(FileHandle handle,
                              void* buffer,
                              uint64 num_bytes_to_read,
                              uint64 offset);

// convenience functions
rsbl::Result<uint64> OpenAndReadFile(const char* path, void* buffer, uint64 num_bytes_to_read);

} // namespace rsbl