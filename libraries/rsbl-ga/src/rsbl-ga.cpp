// Copyright 2025 Robert Srinivasiah
// Licensed under the MIT License, see the LICENSE file for more info

#include <rsbl-ga.h>

namespace rsbl
{

// Forward declarations of backend-specific creation functions
namespace backend
{
Result<gaDevice*> createNullDevice(const gaDeviceCreateInfo& createInfo);
Result<gaDevice*> createDX12Device(const gaDeviceCreateInfo& createInfo);
Result<gaDevice*> createVulkanDevice(const gaDeviceCreateInfo& createInfo);
} // namespace backend

Result<gaDevice*> gaCreateDevice(const gaDeviceCreateInfo& createInfo)
{
	switch (createInfo.backend)
	{
	case gaBackend::Null:
		return backend::createNullDevice(createInfo);

	case gaBackend::DX12:
		return backend::createDX12Device(createInfo);

	case gaBackend::Vulkan:
		return backend::createVulkanDevice(createInfo);

	default:
		return "Unknown graphics backend";
	}
}

void gaDestroyDevice(gaDevice* device)
{
	if (device == nullptr)
	{
		return;
	}

	// Virtual destructor will call the appropriate backend-specific destructor
	delete device;
}

} // namespace rsbl
