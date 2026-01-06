// Copyright 2025 Robert Srinivasiah
// Licensed under the MIT License, see the LICENSE file for more info

#pragma once

#include <rsbl-result.h>

// TODO: check for async compute during device creation?

namespace rsbl
{

enum class gaBackend
{
    Null,   // No-op implementation for API validation
    DX12,   // DirectX 12
    Vulkan, // Vulkan
};

struct gaDeviceCreateInfo
{
    gaBackend backend = gaBackend::Null;
    bool enableValidation = false; // Enable debug/validation layers
    const char* appName = "rsbl Application";
    uint32 appVersion = 1;
};

struct gaDevice
{
    gaBackend backend;
    void* internalHandle; // Backend-specific device handle

    virtual ~gaDevice() = default;
};

struct gaSwapchainCreateInfo
{
    gaDevice* device;
    void* appHandle;    // Platform-specific application handle (HINSTANCE on Windows)
    void* windowHandle; // Platform-specific window handle (HWND on Windows)
    uint32 width;       // Window client width
    uint32 height;      // Window client height
};

struct gaSwapchain
{
    gaBackend backend;
    void* internalHandle; // Backend-specific swapchain handle

    virtual ~gaSwapchain() = default;
};

Result<gaDevice*> GaCreateDevice(const gaDeviceCreateInfo& createInfo);
void GaDestroyDevice(gaDevice* device);

Result<gaSwapchain*> GaCreateSwapchain(const gaSwapchainCreateInfo& createInfo);
void GaDestroySwapchain(gaSwapchain* swapchain);

} // namespace rsbl