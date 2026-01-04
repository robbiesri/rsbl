// Copyright 2025 Robert Srinivasiah
// Licensed under the MIT License, see the LICENSE file for more info

#pragma once

#include <rsbl-ga.h>

// Internal header for backend implementations
// This ensures type safety between the dispatcher and backend implementations

namespace rsbl
{
namespace backend
{

Result<gaDevice*> createNullDevice(const gaDeviceCreateInfo& createInfo);
Result<gaDevice*> createDX12Device(const gaDeviceCreateInfo& createInfo);
Result<gaDevice*> createVulkanDevice(const gaDeviceCreateInfo& createInfo);

} // namespace backend
} // namespace rsbl
