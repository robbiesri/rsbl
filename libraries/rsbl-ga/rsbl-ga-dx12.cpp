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
        uint32 rtvDescriptorSize = 0;

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

    struct DX12Swapchain : public gaSwapchain
    {
        IDXGISwapChain3* dxgiSwapchain = nullptr;
        DynamicArray<ID3D12Resource*> renderTargets;
        ID3D12DescriptorHeap* rtvHeap = nullptr;

        DX12Swapchain()
        {
            backend = gaBackend::DX12;
            internalHandle = nullptr;
        }

        ~DX12Swapchain() override
        {
            RSBL_LOG_INFO("Destroying DX12 swapchain...");

            // Release render targets first
            for (size_t i = 0; i < renderTargets.Size(); ++i)
            {
                if (renderTargets[i])
                {
                    RSBL_LOG_INFO("Releasing render target {}: {}", i, static_cast<void*>(renderTargets[i]));
                    renderTargets[i]->Release();
                }
            }
            renderTargets.Clear();

            if (rtvHeap)
            {
                RSBL_LOG_INFO("Releasing RTV descriptor heap: {}", static_cast<void*>(rtvHeap));
                rtvHeap->Release();
                rtvHeap = nullptr;
            }

            if (dxgiSwapchain)
            {
                RSBL_LOG_INFO("Releasing IDXGISwapChain3: {}", static_cast<void*>(dxgiSwapchain));
                dxgiSwapchain->Release();
                dxgiSwapchain = nullptr;
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

        // Cache RTV descriptor size for later use
        device->rtvDescriptorSize =
            device->d3d12Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
        RSBL_LOG_INFO("RTV descriptor size: {}", device->rtvDescriptorSize);

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

    Result<gaSwapchain*> CreateDX12Swapchain(const gaSwapchainCreateInfo& createInfo)
    {
        RSBL_LOG_INFO("Creating DX12 swapchain...");

        // Cast to DX12 device
        auto dx12Device = static_cast<DX12Device*>(createInfo.device);
        if (dx12Device->commandQueues.Size() == 0)
        {
            return "No command queues available on device";
        }

        // Decode platform handles
        HWND hwnd = static_cast<HWND>(createInfo.windowHandle);
        if (hwnd == nullptr)
        {
            return "Invalid window handle";
        }

        auto swapchain = rsbl::UniquePtr(new DX12Swapchain());

        // Create swapchain description
        DXGI_SWAP_CHAIN_DESC1 swapchainDesc = {};
        swapchainDesc.Width = createInfo.width;
        swapchainDesc.Height = createInfo.height;
        swapchainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        swapchainDesc.Stereo = FALSE;
        swapchainDesc.SampleDesc.Count = 1;
        swapchainDesc.SampleDesc.Quality = 0;
        swapchainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        swapchainDesc.BufferCount = createInfo.bufferCount;
        swapchainDesc.Scaling = DXGI_SCALING_STRETCH;
        swapchainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
        swapchainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
        swapchainDesc.Flags = 0;

        // Create swapchain
        IDXGISwapChain1* tempSwapchain = nullptr;
        HRESULT hr = dx12Device->dxgiFactory->CreateSwapChainForHwnd(
            dx12Device->commandQueues[0], // Use first command queue
            hwnd,
            &swapchainDesc,
            nullptr, // Fullscreen desc
            nullptr, // Restrict output
            &tempSwapchain);

        if (FAILED(hr))
        {
            return "Failed to create swapchain";
        }

        // Query for IDXGISwapChain3 interface
        hr = tempSwapchain->QueryInterface(IID_PPV_ARGS(&swapchain->dxgiSwapchain));
        tempSwapchain->Release();

        if (FAILED(hr))
        {
            return "Failed to query IDXGISwapChain3 interface";
        }

        RSBL_LOG_INFO("Swapchain created: {}", static_cast<void*>(swapchain->dxgiSwapchain));

        swapchain->internalHandle = swapchain->dxgiSwapchain;

        // Create RTV descriptor heap
        D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
        rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
        rtvHeapDesc.NumDescriptors = swapchainDesc.BufferCount;
        rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
        rtvHeapDesc.NodeMask = 0;

        hr = dx12Device->d3d12Device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&swapchain->rtvHeap));
        if (FAILED(hr))
        {
            return "Failed to create RTV descriptor heap";
        }

        RSBL_LOG_INFO("RTV descriptor heap created: {}", static_cast<void*>(swapchain->rtvHeap));

        // Get render targets and create RTVs
        D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = swapchain->rtvHeap->GetCPUDescriptorHandleForHeapStart();

        for (UINT i = 0; i < swapchainDesc.BufferCount; ++i)
        {
            ID3D12Resource* renderTarget = nullptr;
            hr = swapchain->dxgiSwapchain->GetBuffer(i, IID_PPV_ARGS(&renderTarget));
            if (FAILED(hr))
            {
                return "Failed to get swapchain buffer";
            }

            dx12Device->d3d12Device->CreateRenderTargetView(renderTarget, nullptr, rtvHandle);
            swapchain->renderTargets.PushBack(renderTarget);

            RSBL_LOG_INFO("Render target {} created: {}", i, static_cast<void*>(renderTarget));

            rtvHandle.ptr += dx12Device->rtvDescriptorSize;
        }

        return swapchain.Release();
    }

} // namespace backend
} // namespace rsbl
