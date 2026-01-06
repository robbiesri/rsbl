// Copyright 2025 Robert Srinivasiah
// Licensed under the MIT License, see the LICENSE file for more info

#include "rsbl-ga-backends.h"

#include <rsbl-dynamic-array.h>
#include <rsbl-log.h>
#include <rsbl-ptr.h>

#include <vulkan/vulkan.h>

// TODO: Convert gaDeviceCreateInfo::appVersion to engineVersion
// TODO: Process to reasonably select device
// TODO: set up VkDebugUtilsMessengerEXT
// TODO: Query instance + device layers, check against requests

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

        // Instance create info
        VkInstanceCreateInfo instanceCreateInfo{};
        instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        instanceCreateInfo.pApplicationInfo = &appInfo;

        // Validation layers
        const char* validationLayers[] = {"VK_LAYER_KHRONOS_validation"};
        if (createInfo.enableValidation)
        {
            instanceCreateInfo.enabledLayerCount = 1;
            instanceCreateInfo.ppEnabledLayerNames = validationLayers;
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

        RSBL_LOG_INFO("Found graphics queue family {}", graphicsQueueFamilyIndex);

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
        // TODO: Implement Vulkan swapchain creation
        return "Vulkan swapchain creation not yet implemented";
    }

} // namespace backend
} // namespace rsbl
