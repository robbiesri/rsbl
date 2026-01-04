// Copyright 2025 Robert Srinivasiah
// Licensed under the MIT License, see the LICENSE file for more info

#include <rsbl-ga.h>

#ifdef _WIN32

#include <d3d12.h>
#include <dxgi1_6.h>

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")

namespace rsbl
{
namespace backend
{

struct DX12Device : public gaDevice
{
	ID3D12Device* d3d12Device = nullptr;
	IDXGIFactory4* dxgiFactory = nullptr;
	IDXGIAdapter1* adapter = nullptr;

	DX12Device()
	{
		backend = gaBackend::DX12;
		internalHandle = nullptr;
	}

	~DX12Device() override
	{
		if (d3d12Device)
		{
			d3d12Device->Release();
			d3d12Device = nullptr;
		}

		if (adapter)
		{
			adapter->Release();
			adapter = nullptr;
		}

		if (dxgiFactory)
		{
			dxgiFactory->Release();
			dxgiFactory = nullptr;
		}
	}
};

Result<gaDevice*> createDX12Device(const gaDeviceCreateInfo& createInfo)
{
	DX12Device* device = new DX12Device();

	// Enable debug layer if validation is requested
	if (createInfo.enableValidation)
	{
		ID3D12Debug* debugController = nullptr;
		if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
		{
			debugController->EnableDebugLayer();
			debugController->Release();
		}
	}

	// Create DXGI factory
	UINT dxgiFactoryFlags = 0;
	if (createInfo.enableValidation)
	{
		dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
	}

	HRESULT hr = CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&device->dxgiFactory));
	if (FAILED(hr))
	{
		delete device;
		return "Failed to create DXGI factory";
	}

	// Find hardware adapter
	IDXGIAdapter1* adapter = nullptr;
	for (UINT adapterIndex = 0;
		 SUCCEEDED(device->dxgiFactory->EnumAdapters1(adapterIndex, &adapter));
		 ++adapterIndex)
	{
		DXGI_ADAPTER_DESC1 desc;
		adapter->GetDesc1(&desc);

		// Skip software adapter
		if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
		{
			adapter->Release();
			continue;
		}

		// Check if adapter supports D3D12
		if (SUCCEEDED(D3D12CreateDevice(adapter, D3D_FEATURE_LEVEL_11_0, __uuidof(ID3D12Device), nullptr)))
		{
			device->adapter = adapter;
			break;
		}

		adapter->Release();
	}

	if (device->adapter == nullptr)
	{
		delete device;
		return "Failed to find suitable graphics adapter";
	}

	// Create D3D12 device
	hr = D3D12CreateDevice(
		device->adapter,
		D3D_FEATURE_LEVEL_11_0,
		IID_PPV_ARGS(&device->d3d12Device));

	if (FAILED(hr))
	{
		delete device;
		return "Failed to create D3D12 device";
	}

	device->internalHandle = device->d3d12Device;

	return device;
}

} // namespace backend
} // namespace rsbl

#else

namespace rsbl
{
namespace backend
{
Result<gaDevice*> createDX12Device(const gaDeviceCreateInfo& createInfo)
{
	return "DX12 is only supported on Windows";
}
} // namespace backend
} // namespace rsbl

#endif // _WIN32
