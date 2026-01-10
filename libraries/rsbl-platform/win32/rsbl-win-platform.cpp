// Copyright 2025 Robert Srinivasiah
// Licensed under the MIT License, see the LICENSE file for more info

#include "rsbl-platform.h"

#include <windows.h>

namespace rsbl
{

void* GetApplicationHandle()
{
    return GetModuleHandle(nullptr);
}

} // namespace rsbl
