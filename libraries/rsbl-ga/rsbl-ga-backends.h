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

    Result<gaDevice*> CreateNullDevice(const gaDeviceCreateInfo& createInfo);
    Result<gaDevice*> CreateDX12Device(const gaDeviceCreateInfo& createInfo);
    Result<gaDevice*> CreateVulkanDevice(const gaDeviceCreateInfo& createInfo);

    Result<gaSwapchain*> CreateNullSwapchain(const gaSwapchainCreateInfo& createInfo);
    Result<gaSwapchain*> CreateDX12Swapchain(const gaSwapchainCreateInfo& createInfo);
    Result<gaSwapchain*> CreateVulkanSwapchain(const gaSwapchainCreateInfo& createInfo);

} // namespace backend
} // namespace rsbl
