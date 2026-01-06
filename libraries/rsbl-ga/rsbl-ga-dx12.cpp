// Copyright 2025 Robert Srinivasiah
// Licensed under the MIT License, see the LICENSE file for more info

#include "rsbl-ga-backends.h"

#include <rsbl-log.h>
#include <rsbl-ptr.h>
#include <rsbl-dynamic-array.h>

#include <d3d12.h>
#include <dxgi1_6.h>

// TODO: check for DX 12.2 (Ray Tracing)

namespace rsbl
{
namespace backend
{

    struct DX12Device : public gaDevice
    {
        ID3D12Device* d3d12Device = nullptr;
        IDXGIFactory4* dxgiFactory = nullptr;
        IDXGIAdapter1* adapter = nullptr;
        DynamicArray<ID3D12CommandQueue*> commandQueues;

        DX12Device()
        {
            backend = gaBackend::DX12;
            internalHandle = nullptr;
        }

        ~DX12Device() override
        {
            RSBL_LOG_INFO("Destroying DX12 device...");

            // Release command queues first
            for (size_t i = 0; i < commandQueues.Size(); ++i)
            {
                if (commandQueues[i])
                {
                    RSBL_LOG_INFO("Releasing ID3D12CommandQueue: {}", static_cast<void*>(commandQueues[i]));
                    commandQueues[i]->Release();
                }
            }
            commandQueues.Clear();

            if (d3d12Device)
            {
                RSBL_LOG_INFO("Releasing ID3D12Device: {}", static_cast<void*>(d3d12Device));
                d3d12Device->Release();
                d3d12Device = nullptr;
            }

            if (adapter)
            {
                RSBL_LOG_INFO("Releasing DXGIAdapter: {}", static_cast<void*>(adapter));
                adapter->Release();
                adapter = nullptr;
            }

            if (dxgiFactory)
            {
                RSBL_LOG_INFO("Releasing DXGIFactory: {}", static_cast<void*>(dxgiFactory));
                dxgiFactory->Release();
                dxgiFactory = nullptr;
            }
        }
    };

    Result<gaDevice*> CreateDX12Device(const gaDeviceCreateInfo& createInfo)
    {
        RSBL_LOG_INFO("Creating DX12 device...");

        auto device = rsbl::UniquePtr(new DX12Device());

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
            return "Failed to create DXGI factory";
        }

        RSBL_LOG_INFO("DXGI factory created: {}", static_cast<void*>(device->dxgiFactory));

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
            if (SUCCEEDED(D3D12CreateDevice(
                    adapter, D3D_FEATURE_LEVEL_12_1, __uuidof(ID3D12Device), nullptr)))
            {
                device->adapter = adapter;
                break;
            }

            adapter->Release();
        }

        if (device->adapter == nullptr)
        {
            return "Failed to find suitable graphics adapter";
        }

        RSBL_LOG_INFO("Found suitable graphics adapter: {}", static_cast<void*>(device->adapter));

        // Create D3D12 device
        hr = D3D12CreateDevice(
            device->adapter, D3D_FEATURE_LEVEL_12_1, IID_PPV_ARGS(&device->d3d12Device));

        if (FAILED(hr))
        {
            return "Failed to create D3D12 device";
        }

        RSBL_LOG_INFO("D3D12 device created: {}", static_cast<void*>(device->d3d12Device));

        device->internalHandle = device->d3d12Device;

        // Create command queue
        D3D12_COMMAND_QUEUE_DESC queueDesc = {};
        queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
        queueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
        queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
        queueDesc.NodeMask = 0;

        ID3D12CommandQueue* commandQueue = nullptr;
        hr = device->d3d12Device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&commandQueue));
        if (FAILED(hr))
        {
            return "Failed to create command queue";
        }

        RSBL_LOG_INFO("Command queue created: {}", static_cast<void*>(commandQueue));
        device->commandQueues.PushBack(commandQueue);

        return device.Release();
    }

} // namespace backend
} // namespace rsbl
