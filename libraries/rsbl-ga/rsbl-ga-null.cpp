// Copyright 2025 Robert Srinivasiah
// Licensed under the MIT License, see the LICENSE file for more info

#include "rsbl-ga-backends.h"

namespace rsbl
{
namespace backend
{

struct NullDevice : public gaDevice
{
	NullDevice()
	{
		backend = gaBackend::Null;
		internalHandle = nullptr;
	}

	~NullDevice() override
	{
		// No cleanup needed for null backend
	}
};

struct NullSwapchain : public gaSwapchain
{
	NullSwapchain()
	{
		backend = gaBackend::Null;
		internalHandle = nullptr;
	}

	~NullSwapchain() override
	{
		// No cleanup needed for null backend
	}
};

Result<gaDevice*> CreateNullDevice(const gaDeviceCreateInfo& createInfo)
{
	// Null backend always succeeds and validates API usage
	NullDevice* device = new NullDevice();
	return device;
}

Result<gaSwapchain*> CreateNullSwapchain(const gaSwapchainCreateInfo& createInfo)
{
	// Validate create info
	if (createInfo.width == 0)
	{
		return "Swapchain width must be greater than zero";
	}

	if (createInfo.height == 0)
	{
		return "Swapchain height must be greater than zero";
	}

	if (createInfo.bufferCount < 2 || createInfo.bufferCount > 4)
	{
		return "Swapchain buffer count must be between 2 and 4";
	}

	if (createInfo.appHandle == nullptr && createInfo.windowHandle == nullptr)
	{
		return "At least one of appHandle or windowHandle must be non-null";
	}

	// Null backend always succeeds and validates API usage
	NullSwapchain* swapchain = new NullSwapchain();
	return swapchain;
}

} // namespace backend
} // namespace rsbl
