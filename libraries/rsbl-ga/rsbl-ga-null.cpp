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

Result<gaDevice*> CreateNullDevice(const gaDeviceCreateInfo& createInfo)
{
	// Null backend always succeeds and validates API usage
	NullDevice* device = new NullDevice();
	return device;
}

} // namespace backend
} // namespace rsbl
