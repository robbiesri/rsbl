// Copyright 2025 Robert Srinivasiah
// Licensed under the MIT License, see the LICENSE file for more info

#include "rsbl-ga-backends.h"

namespace rsbl
{
namespace backend
{

Result<gaDevice*> CreateDX12Device(const gaDeviceCreateInfo& createInfo)
{
	return "DX12 backend is not available. Build with MSVC to enable DX12 support";
}

Result<gaSwapchain*> CreateDX12Swapchain(const gaSwapchainCreateInfo& createInfo)
{
	return "DX12 backend is not available. Build with MSVC to enable DX12 support";
}

} // namespace backend
} // namespace rsbl
