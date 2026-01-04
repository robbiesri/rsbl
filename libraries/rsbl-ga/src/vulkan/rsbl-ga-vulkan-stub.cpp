// Copyright 2025 Robert Srinivasiah
// Licensed under the MIT License, see the LICENSE file for more info

#include <rsbl-ga.h>

namespace rsbl
{
namespace backend
{

Result<gaDevice*> createVulkanDevice(const gaDeviceCreateInfo& createInfo)
{
	return "Vulkan backend is not available. Install Vulkan SDK and reconfigure CMake";
}

} // namespace backend
} // namespace rsbl
