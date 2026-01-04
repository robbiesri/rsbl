// Copyright 2025 Robert Srinivasiah
// Licensed under the MIT License, see the LICENSE file for more info

#include "rsbl-ga-backends.h"

namespace rsbl
{

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
