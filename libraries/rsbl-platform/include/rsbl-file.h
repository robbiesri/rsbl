// Copyright 2025 Robert Srinivasiah
// Licensed under the MIT License, see the LICENSE file for more info

#pragma once

#include <rsbl-int-types.h>

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

} // namespace rsbl