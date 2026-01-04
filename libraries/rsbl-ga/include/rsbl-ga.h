// Copyright 2025 Robert Srinivasiah
// Licensed under the MIT License, see the LICENSE file for more info

#pragma once

#include <rsbl-result.h>

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
	u32 appVersion = 1;
};

struct gaDevice
{
	gaBackend backend;
	void* internalHandle; // Backend-specific device handle

	virtual ~gaDevice() = default;
};

Result<gaDevice*> gaCreateDevice(const gaDeviceCreateInfo& createInfo);
void gaDestroyDevice(gaDevice* device);

} // namespace rsbl