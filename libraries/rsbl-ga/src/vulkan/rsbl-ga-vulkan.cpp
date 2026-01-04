// Copyright 2025 Robert Srinivasiah
// Licensed under the MIT License, see the LICENSE file for more info

#include "../rsbl-ga-backends.h"
#include <vulkan/vulkan.h>

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
		if (logicalDevice != VK_NULL_HANDLE)
		{
			vkDestroyDevice(logicalDevice, nullptr);
			logicalDevice = VK_NULL_HANDLE;
		}

		if (debugMessenger != VK_NULL_HANDLE)
		{
			auto vkDestroyDebugUtilsMessengerEXT = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
				instance,
				"vkDestroyDebugUtilsMessengerEXT");
			if (vkDestroyDebugUtilsMessengerEXT != nullptr)
			{
				vkDestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
			}
			debugMessenger = VK_NULL_HANDLE;
		}

		if (instance != VK_NULL_HANDLE)
		{
			vkDestroyInstance(instance, nullptr);
			instance = VK_NULL_HANDLE;
		}
	}
};

Result<gaDevice*> createVulkanDevice(const gaDeviceCreateInfo& createInfo)
{
	VulkanDevice* device = new VulkanDevice();

	// Application info
	VkApplicationInfo appInfo{};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = createInfo.appName;
	appInfo.applicationVersion = createInfo.appVersion;
	appInfo.pEngineName = "rsbl";
	appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.apiVersion = VK_API_VERSION_1_0;

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
		delete device;
		return "Failed to create Vulkan instance";
	}

	// Select physical device
	u32 deviceCount = 0;
	vkEnumeratePhysicalDevices(device->instance, &deviceCount, nullptr);

	if (deviceCount == 0)
	{
		delete device;
		return "Failed to find GPUs with Vulkan support";
	}

	VkPhysicalDevice* physicalDevices = new VkPhysicalDevice[deviceCount];
	vkEnumeratePhysicalDevices(device->instance, &deviceCount, physicalDevices);

	// For now, just pick the first device
	device->physicalDevice = physicalDevices[0];
	delete[] physicalDevices;

	// Find queue family
	u32 queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(device->physicalDevice, &queueFamilyCount, nullptr);

	VkQueueFamilyProperties* queueFamilies = new VkQueueFamilyProperties[queueFamilyCount];
	vkGetPhysicalDeviceQueueFamilyProperties(device->physicalDevice, &queueFamilyCount, queueFamilies);

	u32 graphicsQueueFamilyIndex = 0;
	bool foundGraphicsQueue = false;
	for (u32 i = 0; i < queueFamilyCount; i++)
	{
		if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
		{
			graphicsQueueFamilyIndex = i;
			foundGraphicsQueue = true;
			break;
		}
	}
	delete[] queueFamilies;

	if (!foundGraphicsQueue)
	{
		delete device;
		return "Failed to find graphics queue family";
	}

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

	if (createInfo.enableValidation)
	{
		deviceCreateInfo.enabledLayerCount = 1;
		deviceCreateInfo.ppEnabledLayerNames = validationLayers;
	}

	result = vkCreateDevice(device->physicalDevice, &deviceCreateInfo, nullptr, &device->logicalDevice);
	if (result != VK_SUCCESS)
	{
		delete device;
		return "Failed to create Vulkan logical device";
	}

	device->internalHandle = device->logicalDevice;

	return device;
}

} // namespace backend
} // namespace rsbl
