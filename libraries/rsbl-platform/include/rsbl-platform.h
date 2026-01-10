// Copyright 2025 Robert Srinivasiah
// Licensed under the MIT License, see the LICENSE file for more info

#pragma once

namespace rsbl
{

// Get platform-specific application handle
// On Windows: Returns HINSTANCE from GetModuleHandle(nullptr)
// On other platforms: Returns nullptr
void* GetApplicationHandle();

} // namespace rsbl
