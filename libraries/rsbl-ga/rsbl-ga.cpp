// Copyright 2025 Robert Srinivasiah
// Licensed under the MIT License, see the LICENSE file for more info

#include "rsbl-ga-backends.h"

namespace rsbl
{

Result<gaDevice*> GaCreateDevice(const gaDeviceCreateInfo& createInfo)
{
    switch (createInfo.backend)
    {
    case gaBackend::Null:
        return backend::CreateNullDevice(createInfo);

    case gaBackend::DX12:
        return backend::CreateDX12Device(createInfo);

    case gaBackend::Vulkan:
        return backend::CreateVulkanDevice(createInfo);

    default:
        return "Unknown graphics backend";
    }
}

void GaDestroyDevice(gaDevice* device)
{
    if (device == nullptr)
    {
        return;
    }

    // Virtual destructor will call the appropriate backend-specific destructor
    delete device;
}

} // namespace rsbl
