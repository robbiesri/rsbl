// Copyright 2025 Robert Srinivasiah
// Licensed under the MIT License, see the LICENSE file for more info

#include "rsbl-ga-backends.h"

#include <rsbl-dynamic-array.h>
#include <rsbl-fixed-array.h>
#include <rsbl-log.h>
#include <rsbl-ptr.h>

#include <vulkan/vulkan.h>

#if defined(WIN32)
    #include <windows.h>

    #include <vulkan/vulkan_win32.h>
#endif

// TODO: Convert gaDeviceCreateInfo::appVersion to engineVersion
// TODO: Process to reasonably select device
// TODO: set up VkDebugUtilsMessengerEXT
// TODO: Query instance + device layers, check against requests
// TODO: do I really need graphicsQueueFamilyIndex stored in VulkanDevice?

namespace rsbl
{
namespace backend
{

    struct VulkanDevice : public gaDevice
    {
        VkInstance instance = VK_NULL_HANDLE;
        VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
        VkDevice logicalDevice = VK_NULL_HANDLE;
        VkDebugUtilsMessengerEXT debugMessenger = VK_NULL_HANDLE;

        uint32 graphicsQueueFamilyIndex = 0;

        VulkanDevice()
        {
            backend = gaBackend::Vulkan;
            internalHandle = nullptr;
        }

        ~VulkanDevice() override
        {
            RSBL_LOG_INFO("Destroying Vulkan device...");

            if (logicalDevice != VK_NULL_HANDLE)
            {
                RSBL_LOG_INFO("Destroying VkDevice: {}", static_cast<void*>(logicalDevice));
                vkDestroyDevice(logicalDevice, nullptr);
                logicalDevice = VK_NULL_HANDLE;
            }

            if (debugMessenger != VK_NULL_HANDLE)
            {
                auto vkDestroyDebugUtilsMessengerEXT =
                    reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(
                        vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT"));
                if (vkDestroyDebugUtilsMessengerEXT != nullptr)
                {
                    vkDestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
                }
                debugMessenger = VK_NULL_HANDLE;
            }

            if (instance != VK_NULL_HANDLE)
            {
                RSBL_LOG_INFO("Destroying VkInstance: {}", static_cast<void*>(instance));
                vkDestroyInstance(instance, nullptr);
                instance = VK_NULL_HANDLE;
            }
        }
    };

    struct VulkanSwapchain : public gaSwapchain
    {
        VkSurfaceKHR surface = VK_NULL_HANDLE;
        VkSwapchainKHR swapchain = VK_NULL_HANDLE;
        DynamicArray<VkImage> swapchainImages;
        DynamicArray<VkImageView> swapchainImageViews;
        VkDevice device = VK_NULL_HANDLE;
        VkInstance instance = VK_NULL_HANDLE;

        VulkanSwapchain()
        {
            backend = gaBackend::Vulkan;
            internalHandle = nullptr;
        }

        ~VulkanSwapchain() override
        {
            RSBL_LOG_INFO("Destroying Vulkan swapchain...");

            // Destroy image views
            for (size_t i = 0; i < swapchainImageViews.Size(); ++i)
            {
                if (swapchainImageViews[i] != VK_NULL_HANDLE)
                {
                    RSBL_LOG_INFO("Destroying VkImageView {}: {}",
                                  i,
                                  static_cast<void*>(swapchainImageViews[i]));
                    vkDestroyImageView(device, swapchainImageViews[i], nullptr);
                }
            }
            swapchainImageViews.Clear();
            swapchainImages.Clear();

            if (swapchain != VK_NULL_HANDLE)
            {
                RSBL_LOG_INFO("Destroying VkSwapchainKHR: {}", static_cast<void*>(swapchain));
                vkDestroySwapchainKHR(device, swapchain, nullptr);
                swapchain = VK_NULL_HANDLE;
            }

            if (surface != VK_NULL_HANDLE)
            {
                RSBL_LOG_INFO("Destroying VkSurfaceKHR: {}", static_cast<void*>(surface));
                vkDestroySurfaceKHR(instance, surface, nullptr);
                surface = VK_NULL_HANDLE;
            }
        }
    };

    Result<gaDevice*> CreateVulkanDevice(const gaDeviceCreateInfo& createInfo)
    {
        RSBL_LOG_INFO("Creating Vulkan device...");
        auto device = rsbl::UniquePtr(new VulkanDevice());

        // Application info
        VkApplicationInfo appInfo{};
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pApplicationName = createInfo.appName;
        appInfo.applicationVersion = createInfo.appVersion;
        appInfo.pEngineName = "rsbl";
        appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.apiVersion = VK_API_VERSION_1_3;

        // Instance extensions for surface support
        FixedArray instanceExtensions = {VK_KHR_SURFACE_EXTENSION_NAME,
                                         VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
                                         VK_KHR_GET_SURFACE_CAPABILITIES_2_EXTENSION_NAME};

        // Instance create info
        VkInstanceCreateInfo instanceCreateInfo{};
        instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        instanceCreateInfo.pApplicationInfo = &appInfo;
        instanceCreateInfo.enabledExtensionCount = static_cast<uint32>(instanceExtensions.Size());
        instanceCreateInfo.ppEnabledExtensionNames = instanceExtensions.Data();

        // Validation layers
        FixedArray validationLayers = {"VK_LAYER_KHRONOS_validation"};
        if (createInfo.enableValidation)
        {
            instanceCreateInfo.enabledLayerCount = static_cast<uint32>(validationLayers.Size());
            instanceCreateInfo.ppEnabledLayerNames = validationLayers.Data();
        }

        // Create instance
        VkResult result = vkCreateInstance(&instanceCreateInfo, nullptr, &device->instance);
        if (result != VK_SUCCESS)
        {
            return "Failed to create Vulkan instance";
        }

        RSBL_LOG_INFO("Vulkan instance created: {}", static_cast<void*>(device->instance));

        // Select physical device
        uint32 deviceCount = 0;
        vkEnumeratePhysicalDevices(device->instance, &deviceCount, nullptr);

        if (deviceCount == 0)
        {
            return "Failed to find GPUs with Vulkan support";
        }

        RSBL_LOG_INFO("Found {} GPUs with Vulkan support", deviceCount);

        rsbl::DynamicArray<VkPhysicalDevice> physicalDevices(deviceCount);
        vkEnumeratePhysicalDevices(device->instance, &deviceCount, physicalDevices.Data());

        // For now, just pick the first device
        device->physicalDevice = physicalDevices[0];

        // Find queue family
        uint32 queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(
            device->physicalDevice, &queueFamilyCount, nullptr);

        rsbl::DynamicArray<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(
            device->physicalDevice, &queueFamilyCount, queueFamilies.Data());

        uint32 graphicsQueueFamilyIndex = 0;
        bool foundGraphicsQueue = false;
        for (uint32 i = 0; i < queueFamilyCount; i++)
        {
            if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
            {
                graphicsQueueFamilyIndex = i;
                foundGraphicsQueue = true;
                break;
            }
        }

        if (!foundGraphicsQueue)
        {
            return "Failed to find graphics queue family";
        }

        device->graphicsQueueFamilyIndex = graphicsQueueFamilyIndex;
        RSBL_LOG_INFO("Found graphics queue family {}", graphicsQueueFamilyIndex);

        // Device extensions for swapchain support
        // These extensions are expected to be available in Vulkan 1.3
        FixedArray deviceExtensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME,
                                       VK_KHR_SWAPCHAIN_MUTABLE_FORMAT_EXTENSION_NAME,
                                       VK_KHR_IMAGE_FORMAT_LIST_EXTENSION_NAME};

        // Create logical device
        VkDeviceQueueCreateInfo queueCreateInfo{};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = graphicsQueueFamilyIndex;
        queueCreateInfo.queueCount = 1;
        float queuePriority = 1.0f;
        queueCreateInfo.pQueuePriorities = &queuePriority;

        VkPhysicalDeviceFeatures deviceFeatures{};

        VkDeviceCreateInfo deviceCreateInfo{};
        deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        deviceCreateInfo.pQueueCreateInfos = &queueCreateInfo;
        deviceCreateInfo.queueCreateInfoCount = 1;
        deviceCreateInfo.pEnabledFeatures = &deviceFeatures;
        deviceCreateInfo.enabledExtensionCount = static_cast<uint32>(deviceExtensions.Size());
        deviceCreateInfo.ppEnabledExtensionNames = deviceExtensions.Data();
        // deviceCreateInfo.flags = 0; // flags not needed currently, according to spec

        result = vkCreateDevice(
            device->physicalDevice, &deviceCreateInfo, nullptr, &device->logicalDevice);
        if (result != VK_SUCCESS)
        {
            return "Failed to create Vulkan logical device";
        }

        RSBL_LOG_INFO("Vulkan logical device created: {}",
                      static_cast<void*>(device->logicalDevice));

        device->internalHandle = device->logicalDevice;

        return device.Release();
    }

    Result<gaSwapchain*> CreateVulkanSwapchain(const gaSwapchainCreateInfo& createInfo)
    {
        RSBL_LOG_INFO("Creating Vulkan swapchain...");

        // Cast to Vulkan device
        auto vulkanDevice = static_cast<VulkanDevice*>(createInfo.device);

        // Decode platform handles
        HWND hwnd = static_cast<HWND>(createInfo.windowHandle);
        HINSTANCE hinstance = static_cast<HINSTANCE>(createInfo.appHandle);

        if (hwnd == nullptr)
        {
            return "Invalid window handle";
        }

        if (hinstance == nullptr)
        {
            return "Invalid application handle";
        }

        auto swapchain = rsbl::UniquePtr(new VulkanSwapchain());
        swapchain->device = vulkanDevice->logicalDevice;
        swapchain->instance = vulkanDevice->instance;

        // Create Win32 surface
        VkWin32SurfaceCreateInfoKHR surfaceCreateInfo{};
        surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
        surfaceCreateInfo.hwnd = hwnd;
        surfaceCreateInfo.hinstance = hinstance;

        VkResult result = vkCreateWin32SurfaceKHR(
            vulkanDevice->instance, &surfaceCreateInfo, nullptr, &swapchain->surface);
        if (result != VK_SUCCESS)
        {
            return "Failed to create Win32 surface";
        }

        RSBL_LOG_INFO("Win32 surface created: {}", static_cast<void*>(swapchain->surface));

        // Check if queue family supports presentation
        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(vulkanDevice->physicalDevice,
                                             vulkanDevice->graphicsQueueFamilyIndex,
                                             swapchain->surface,
                                             &presentSupport);

        if (!presentSupport)
        {
            return "Graphics queue family does not support presentation";
        }

        // Query surface capabilities using VK_KHR_get_surface_capabilities2
        VkPhysicalDeviceSurfaceInfo2KHR surfaceInfo{};
        surfaceInfo.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SURFACE_INFO_2_KHR;
        surfaceInfo.surface = swapchain->surface;

        VkSurfaceCapabilities2KHR capabilities2{};
        capabilities2.sType = VK_STRUCTURE_TYPE_SURFACE_CAPABILITIES_2_KHR;

        result = vkGetPhysicalDeviceSurfaceCapabilities2KHR(
            vulkanDevice->physicalDevice, &surfaceInfo, &capabilities2);
        if (result != VK_SUCCESS)
        {
            return "Failed to get surface capabilities";
        }

        VkSurfaceCapabilitiesKHR& capabilities = capabilities2.surfaceCapabilities;

        // Query surface formats using VK_KHR_get_surface_capabilities2
        uint32 formatCount;
        result = vkGetPhysicalDeviceSurfaceFormats2KHR(
            vulkanDevice->physicalDevice, &surfaceInfo, &formatCount, nullptr);
        if (result != VK_SUCCESS || formatCount == 0)
        {
            return "No surface formats available";
        }

        DynamicArray<VkSurfaceFormat2KHR> formats2(formatCount);
        for (uint32 i = 0; i < formatCount; ++i)
        {
            formats2[i].sType = VK_STRUCTURE_TYPE_SURFACE_FORMAT_2_KHR;
            formats2[i].pNext = nullptr;
        }

        result = vkGetPhysicalDeviceSurfaceFormats2KHR(
            vulkanDevice->physicalDevice, &surfaceInfo, &formatCount, formats2.Data());
        if (result != VK_SUCCESS)
        {
            return "Failed to get surface formats";
        }

        // Choose format (prefer B8G8R8A8_UNORM with SRGB_NONLINEAR)
        VkSurfaceFormatKHR surfaceFormat = formats2[0].surfaceFormat;
        for (uint32 i = 0; i < formatCount; ++i)
        {
            if (formats2[i].surfaceFormat.format == VK_FORMAT_B8G8R8A8_UNORM &&
                formats2[i].surfaceFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
            {
                surfaceFormat = formats2[i].surfaceFormat;
                break;
            }
        }

        RSBL_LOG_INFO("Selected surface format: {}", static_cast<int>(surfaceFormat.format));

        // Query present modes
        uint32 presentModeCount;
        vkGetPhysicalDeviceSurfacePresentModesKHR(
            vulkanDevice->physicalDevice, swapchain->surface, &presentModeCount, nullptr);

        if (presentModeCount == 0)
        {
            return "No present modes available";
        }

        DynamicArray<VkPresentModeKHR> presentModes(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(vulkanDevice->physicalDevice,
                                                  swapchain->surface,
                                                  &presentModeCount,
                                                  presentModes.Data());

        // Choose present mode (prefer MAILBOX, fallback to FIFO which is always available)
        VkPresentModeKHR presentMode = VK_PRESENT_MODE_FIFO_KHR;
        for (uint32 i = 0; i < presentModeCount; ++i)
        {
            if (presentModes[i] == VK_PRESENT_MODE_MAILBOX_KHR)
            {
                presentMode = VK_PRESENT_MODE_MAILBOX_KHR;
                break;
            }
        }

        RSBL_LOG_INFO("Selected present mode: {}", static_cast<int>(presentMode));

        // Determine swap extent
        VkExtent2D extent;
        if (capabilities.currentExtent.width != UINT32_MAX)
        {
            extent = capabilities.currentExtent;
        }
        else
        {
            extent.width = createInfo.width;
            extent.height = createInfo.height;

            // Clamp to supported range
            extent.width = extent.width < capabilities.minImageExtent.width
                               ? capabilities.minImageExtent.width
                               : (extent.width > capabilities.maxImageExtent.width
                                      ? capabilities.maxImageExtent.width
                                      : extent.width);
            extent.height = extent.height < capabilities.minImageExtent.height
                                ? capabilities.minImageExtent.height
                                : (extent.height > capabilities.maxImageExtent.height
                                       ? capabilities.maxImageExtent.height
                                       : extent.height);
        }

        // Determine image count (use min + 1, but don't exceed max)
        uint32 imageCount = capabilities.minImageCount + 1;
        if (capabilities.maxImageCount > 0 && imageCount > capabilities.maxImageCount)
        {
            imageCount = capabilities.maxImageCount;
        }

        RSBL_LOG_INFO(
            "Swapchain extent: {}x{}, image count: {}", extent.width, extent.height, imageCount);

        // Create swapchain
        FixedArray queueFamilyIndices = {vulkanDevice->graphicsQueueFamilyIndex};

        VkSwapchainCreateInfoKHR swapchainCreateInfo{};
        swapchainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        swapchainCreateInfo.surface = swapchain->surface;
        swapchainCreateInfo.minImageCount = imageCount;
        swapchainCreateInfo.imageFormat = surfaceFormat.format;
        swapchainCreateInfo.imageColorSpace = surfaceFormat.colorSpace;
        swapchainCreateInfo.imageExtent = extent;
        swapchainCreateInfo.imageArrayLayers = 1;
        swapchainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        swapchainCreateInfo.queueFamilyIndexCount = static_cast<uint32>(queueFamilyIndices.Size());
        swapchainCreateInfo.pQueueFamilyIndices = queueFamilyIndices.Data();
        swapchainCreateInfo.preTransform = capabilities.currentTransform;
        swapchainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        swapchainCreateInfo.presentMode = presentMode;
        swapchainCreateInfo.clipped = VK_TRUE;
        swapchainCreateInfo.oldSwapchain = VK_NULL_HANDLE;

        result = vkCreateSwapchainKHR(
            vulkanDevice->logicalDevice, &swapchainCreateInfo, nullptr, &swapchain->swapchain);
        if (result != VK_SUCCESS)
        {
            return "Failed to create swapchain";
        }

        RSBL_LOG_INFO("Swapchain created: {}", static_cast<void*>(swapchain->swapchain));

        swapchain->internalHandle = swapchain->swapchain;

        // Get swapchain images
        uint32 actualImageCount;
        vkGetSwapchainImagesKHR(
            vulkanDevice->logicalDevice, swapchain->swapchain, &actualImageCount, nullptr);

        swapchain->swapchainImages.Resize(actualImageCount);
        vkGetSwapchainImagesKHR(vulkanDevice->logicalDevice,
                                swapchain->swapchain,
                                &actualImageCount,
                                swapchain->swapchainImages.Data());

        RSBL_LOG_INFO("Retrieved {} swapchain images", actualImageCount);

        // Create image views
        swapchain->swapchainImageViews.Resize(actualImageCount);
        for (uint32 i = 0; i < actualImageCount; ++i)
        {
            VkImageViewCreateInfo imageViewCreateInfo{};
            imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            imageViewCreateInfo.image = swapchain->swapchainImages[i];
            imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
            imageViewCreateInfo.format = surfaceFormat.format;
            imageViewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
            imageViewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
            imageViewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
            imageViewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
            imageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
            imageViewCreateInfo.subresourceRange.levelCount = 1;
            imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
            imageViewCreateInfo.subresourceRange.layerCount = 1;

            result = vkCreateImageView(vulkanDevice->logicalDevice,
                                       &imageViewCreateInfo,
                                       nullptr,
                                       &swapchain->swapchainImageViews[i]);

            if (result != VK_SUCCESS)
            {
                return "Failed to create image view";
            }

            RSBL_LOG_INFO("Image view {} created: {}",
                          i,
                          static_cast<void*>(swapchain->swapchainImageViews[i]));
        }

        return swapchain.Release();
    }

} // namespace backend
} // namespace rsbl
