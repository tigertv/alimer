//
// Copyright (c) 2020 Amer Koleci and contributors.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//

#include "VulkanGPUDevice.h"
#include "Core/Utils.h"
#include "Diagnostics/Assert.h"
#include "Diagnostics/Log.h"
#include <EASTL/algorithm.h>
#include <EASTL/vector.h>

#define VMA_IMPLEMENTATION
#include <vk_mem_alloc.h>

#if ALIMER_WINDOWS || ALIMER_LINUX || ALIMER_OSX
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#endif

using namespace eastl;
namespace alimer
{
    bool hasLayers(const vector<const char*>& required, const vector<VkLayerProperties>& available)
    {
        for (auto layer : required)
        {
            bool found = false;
            for (auto& available_layer : available)
            {
                if (strcmp(available_layer.layerName, layer) == 0)
                {
                    found = true;
                    break;
                }
            }

            if (!found)
            {
                return false;
            }
        }

        return true;
    }

    vector<const char*> getOptimalValidationLayers(const vector<VkLayerProperties>& supported_instance_layers)
    {
        vector<vector<const char*>> validationLayerPriorityList =
        {
            // The preferred validation layer is "VK_LAYER_KHRONOS_validation"
            {"VK_LAYER_KHRONOS_validation"},

            // Otherwise we fallback to using the LunarG meta layer
            {"VK_LAYER_LUNARG_standard_validation"},

            // Otherwise we attempt to enable the individual layers that compose the LunarG meta layer since it doesn't exist
            {
                "VK_LAYER_GOOGLE_threading",
                "VK_LAYER_LUNARG_parameter_validation",
                "VK_LAYER_LUNARG_object_tracker",
                "VK_LAYER_LUNARG_core_validation",
                "VK_LAYER_GOOGLE_unique_objects",
            },

            // Otherwise as a last resort we fallback to attempting to enable the LunarG core layer
            {"VK_LAYER_LUNARG_core_validation"} };

        for (auto& validation_layers : validationLayerPriorityList)
        {
            if (hasLayers(validation_layers, supported_instance_layers))
            {
                return validation_layers;
            }

            ALIMER_LOGW("Couldn't enable validation layers (see log for error) - falling back");
        }

        // Else return nothing
        return {};
    }

    static VkPresentModeKHR ChooseSwapPresentMode(
        const vector<VkPresentModeKHR>& availablePresentModes, bool vsyncEnabled)
    {
        // Try to match the correct present mode to the vsync state.
        vector<VkPresentModeKHR> desiredModes;
        if (vsyncEnabled) desiredModes = { VK_PRESENT_MODE_FIFO_KHR, VK_PRESENT_MODE_FIFO_RELAXED_KHR };
        else desiredModes = { VK_PRESENT_MODE_IMMEDIATE_KHR, VK_PRESENT_MODE_MAILBOX_KHR };

        // Iterate over all available present mdoes and match to one of the desired ones.
        for (const auto& availablePresentMode : availablePresentModes)
        {
            for (auto mode : desiredModes)
            {
                if (availablePresentMode == mode)
                    return availablePresentMode;
            }
        }

        // If no match was found, return the first present mode or default to FIFO.
        if (availablePresentModes.size() > 0) return availablePresentModes[0];
        else return VK_PRESENT_MODE_FIFO_KHR;
    }

    static VKAPI_ATTR VkBool32 VKAPI_CALL vulkanDebugCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT           messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT                  messageType,
        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
        void* pUserData)
    {
        auto* context = static_cast<GraphicsDevice*>(pUserData);

        switch (messageSeverity)
        {
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
            if (messageType == VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT)
            {
                ALIMER_LOGE("[Vulkan]: Validation Error: %s", pCallbackData->pMessage);
                // context->notify_validation_error(pCallbackData->pMessage);
            }
            else
                ALIMER_LOGE("[Vulkan]: Other Error: %s", pCallbackData->pMessage);
            break;

        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
            if (messageType == VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT)
                ALIMER_LOGW("[Vulkan]: Validation Warning: %s", pCallbackData->pMessage);
            else
                ALIMER_LOGW("[Vulkan]: Other Warning: %s", pCallbackData->pMessage);
            break;

        default:
            return VK_FALSE;
        }

        bool log_object_names = false;
        for (uint32_t i = 0; i < pCallbackData->objectCount; i++)
        {
            auto* name = pCallbackData->pObjects[i].pObjectName;
            if (name)
            {
                log_object_names = true;
                break;
            }
        }

        if (log_object_names)
        {
            for (uint32_t i = 0; i < pCallbackData->objectCount; i++)
            {
                auto* name = pCallbackData->pObjects[i].pObjectName;
                ALIMER_LOGI("  Object #%u: %s", i, name ? name : "N/A");
            }
        }

        return VK_FALSE;
    }

    bool VulkanGPUDevice::IsAvailable()
    {
        static bool availableInitialized = false;
        static bool available = false;
        if (availableInitialized) {
            return available;
        }

        availableInitialized = true;
        VkResult result = volkInitialize();
        if (result != VK_SUCCESS)
        {
            ALIMER_LOGW("Failed to initialize volk, vulkan backend is not available.");
            return false;
        }

        available = true;
        return available;
    }
    
    VulkanGPUDevice::VulkanGPUDevice()
        : GPUDevice()
    {
        ALIMER_ASSERT(IsAvailable());
    }

    VulkanGPUDevice::~VulkanGPUDevice()
    {
        WaitIdle();
        BackendShutdown();
    }

    bool VulkanGPUDevice::BackendInit(const DeviceDesc& desc)
    {
        VkResult result = volkInitialize();
        if (result != VK_SUCCESS)
        {
            ALIMER_LOGERROR("Failed to initialize volk.");
            return false;
        }

        // Setup application info.
        vk_features.apiVersion = volkGetInstanceVersion();
        bool api_version_12 = false;
        bool api_version_11 = false;
        VkApplicationInfo appInfo{ VK_STRUCTURE_TYPE_APPLICATION_INFO };
        appInfo.pApplicationName = desc.applicationName;
        appInfo.applicationVersion = 0;
        appInfo.pEngineName = "Alimer";
        appInfo.engineVersion = 0;
        if (vk_features.apiVersion >= VK_API_VERSION_1_2) {
            appInfo.apiVersion = VK_API_VERSION_1_2;
            api_version_12 = true;
        }
        else if (vk_features.apiVersion >= VK_API_VERSION_1_1) {
            appInfo.apiVersion = VK_API_VERSION_1_1;
            api_version_11 = true;
        }
        else {
            appInfo.apiVersion = VK_MAKE_VERSION(1, 0, 55);
        }

        // Create instance first.
        {
            uint32_t extensionCount;
            VK_CHECK(vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr));

            vector<VkExtensionProperties> queriedExtensions(extensionCount);
            VK_CHECK(vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, queriedExtensions.data()));

            const auto hasExtension = [&](const char* name) -> bool {
                auto itr = find_if(begin(queriedExtensions), end(queriedExtensions), [name](const VkExtensionProperties& e) -> bool {
                    return strcmp(e.extensionName, name) == 0;
                    });
                return itr != end(queriedExtensions);
            };

            vector<const char*> instanceExtensions;
            // Try to enable headless surface extension if it exists
            if (desc.headless)
            {
                if (!hasExtension(VK_EXT_HEADLESS_SURFACE_EXTENSION_NAME))
                {
                    ALIMER_LOGW("%s is not available, disabling swapchain creation", VK_EXT_HEADLESS_SURFACE_EXTENSION_NAME);
                }
                else
                {
                    ALIMER_LOGI("%s is available, enabling it", VK_EXT_HEADLESS_SURFACE_EXTENSION_NAME);
                    instanceExtensions.push_back(VK_EXT_HEADLESS_SURFACE_EXTENSION_NAME);
                }
            }
            else
            {
                instanceExtensions.push_back(VK_KHR_SURFACE_EXTENSION_NAME);
                // Enable surface extensions depending on os
#if defined(VK_USE_PLATFORM_ANDROID_KHR)
                instanceExtensions.push_back(VK_KHR_ANDROID_SURFACE_EXTENSION_NAME);
#elif defined(VK_USE_PLATFORM_WIN32_KHR)
                instanceExtensions.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
#elif defined(_DIRECT2DISPLAY)
                instanceExtensions.push_back(VK_KHR_DISPLAY_EXTENSION_NAME);
#elif defined(VK_USE_PLATFORM_WAYLAND_KHR)
                instanceExtensions.push_back(VK_KHR_WAYLAND_SURFACE_EXTENSION_NAME);
#elif defined(VK_USE_PLATFORM_XCB_KHR)
                instanceExtensions.push_back(VK_KHR_XCB_SURFACE_EXTENSION_NAME);
#elif defined(VK_USE_PLATFORM_IOS_MVK)
                instanceExtensions.push_back(VK_MVK_IOS_SURFACE_EXTENSION_NAME);
#elif defined(VK_USE_PLATFORM_MACOS_MVK)
                // TODO: Support VK_EXT_metal_surface
                instanceExtensions.push_back(VK_MVK_MACOS_SURFACE_EXTENSION_NAME);
#endif

                if (hasExtension(VK_KHR_GET_SURFACE_CAPABILITIES_2_EXTENSION_NAME))
                {
                    instanceExtensions.push_back(VK_KHR_GET_SURFACE_CAPABILITIES_2_EXTENSION_NAME);
                    vk_features.surface_capabilities2 = true;
            }
            }

            if (hasExtension(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME))
            {
                vk_features.physical_device_properties2 = true;
                instanceExtensions.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
            }

            if (vk_features.physical_device_properties2
                && hasExtension(VK_KHR_EXTERNAL_MEMORY_CAPABILITIES_EXTENSION_NAME)
                && hasExtension(VK_KHR_EXTERNAL_SEMAPHORE_CAPABILITIES_EXTENSION_NAME))
            {
                instanceExtensions.push_back(VK_KHR_EXTERNAL_MEMORY_CAPABILITIES_EXTENSION_NAME);
                instanceExtensions.push_back(VK_KHR_EXTERNAL_SEMAPHORE_CAPABILITIES_EXTENSION_NAME);
                vk_features.external = true;
            }

            if (hasExtension(VK_EXT_DEBUG_UTILS_EXTENSION_NAME))
            {
                instanceExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
                vk_features.debug_utils = true;
            }

            vector<const char*> instanceLayers;

            if (desc.validation)
            {
                uint32_t layerCount;
                VK_CHECK(vkEnumerateInstanceLayerProperties(&layerCount, nullptr));

                vector<VkLayerProperties> queriedLayers(layerCount);
                VK_CHECK(vkEnumerateInstanceLayerProperties(&layerCount, queriedLayers.data()));

                instanceLayers = getOptimalValidationLayers(queriedLayers);
            }

            VkInstanceCreateInfo createInfo = { VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO };
            createInfo.pApplicationInfo = &appInfo;
            createInfo.enabledLayerCount = static_cast<uint32_t>(instanceLayers.size());
            createInfo.ppEnabledLayerNames = instanceLayers.data();
            createInfo.enabledExtensionCount = static_cast<uint32_t>(instanceExtensions.size());
            createInfo.ppEnabledExtensionNames = instanceExtensions.data();

            // Create the Vulkan instance
            VkResult result = vkCreateInstance(&createInfo, nullptr, &instance);

            if (result != VK_SUCCESS)
            {
                VK_THROW(result, "Could not create Vulkan instance");
            }

            volkLoadInstance(instance);

            if (desc.validation && vk_features.debug_utils)
            {
                VkDebugUtilsMessengerCreateInfoEXT debugMessengerCreateInfo = { VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT };
                debugMessengerCreateInfo.messageSeverity =
                    VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |
                    VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                    VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
                debugMessengerCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                    VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                    VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
                debugMessengerCreateInfo.pfnUserCallback = vulkanDebugCallback;
                debugMessengerCreateInfo.pUserData = this;
                VK_CHECK(
                    vkCreateDebugUtilsMessengerEXT(instance, &debugMessengerCreateInfo, nullptr, &debug_messenger)
                );
            }
        }

        // Enumerate physical device and create logical one.
        {
            uint32_t device_count;
            if (vkEnumeratePhysicalDevices(instance, &device_count, nullptr) != VK_SUCCESS) {
                return false;
            }

            vector<VkPhysicalDevice> physicalDevices(device_count);
            if (vkEnumeratePhysicalDevices(instance, &device_count, physicalDevices.data()) != VK_SUCCESS) {
                return false;
            }

            // Pick a suitable physical device based on user's preference.
            VkPhysicalDeviceProperties device_props;
            uint32_t best_device_score = 0;
            uint32_t best_device_index = -1;
            for (uint32_t i = 0; i < device_count; ++i)
            {
                vkGetPhysicalDeviceProperties(physicalDevices[i], &device_props);
                uint32_t score = 0U;

                if (device_props.apiVersion >= VK_API_VERSION_1_2) {
                    score += 10000u;
                }
                else if (device_props.apiVersion >= VK_API_VERSION_1_1) {
                    score += 5000u;
                }

                switch (device_props.deviceType) {
                case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
                    score += 100U;
                    if (desc.powerPreference == DevicePowerPreference::HighPerformance) {
                        score += 1000u;
                    }
                    break;
                case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
                    score += 90U;
                    if (desc.powerPreference == DevicePowerPreference::LowPower) {
                        score += 1000u;
                    }
                    break;
                case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:
                    score += 80u;
                    break;
                case VK_PHYSICAL_DEVICE_TYPE_CPU:
                    score += 70u;
                    break;
                default: score += 10u;
                }
                if (score > best_device_score) {
                    best_device_index = i;
                    best_device_score = score;
                }
            }
            if (best_device_index == -1) {
                return false;
            }

            physical_device = physicalDevices[best_device_index];
            vkGetPhysicalDeviceProperties(physical_device, &device_props);

            uint32_t queue_count;
            vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_count, nullptr);
            vector<VkQueueFamilyProperties> queue_props(queue_count);
            vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_count, queue_props.data());

            for (uint32_t i = 0; i < queue_count; i++)
            {
                VkBool32 supportPresent = true;
#if ALIMER_WINDOWS || ALIMER_LINUX || ALIMER_OSX
                supportPresent = glfwGetPhysicalDevicePresentationSupport(instance, physical_device, i);
#endif

                static const VkQueueFlags required = VK_QUEUE_COMPUTE_BIT | VK_QUEUE_GRAPHICS_BIT;
                if (supportPresent && ((queue_props[i].queueFlags & required) == required))
                {
                    graphicsQueueFamily = i;
                    break;
                }
            }

            for (uint32_t i = 0; i < queue_count; i++)
            {
                static const VkQueueFlags required = VK_QUEUE_COMPUTE_BIT;
                if (i != graphicsQueueFamily && (queue_props[i].queueFlags & required) == required)
                {
                    computeQueueFamily = i;
                    break;
                }
            }

            for (uint32_t i = 0; i < queue_count; i++)
            {
                static const VkQueueFlags required = VK_QUEUE_TRANSFER_BIT;
                if (i != graphicsQueueFamily
                    && i != computeQueueFamily
                    && (queue_props[i].queueFlags & required) == required)
                {
                    copyQueueFamily = i;
                    break;
                }
            }

            if (copyQueueFamily == VK_QUEUE_FAMILY_IGNORED)
            {
                for (uint32_t i = 0; i < queue_count; i++)
                {
                    static const VkQueueFlags required = VK_QUEUE_TRANSFER_BIT;
                    if (i != graphicsQueueFamily
                        && (queue_props[i].queueFlags & required) == required)
                    {
                        copyQueueFamily = i;
                        break;
                    }
                }
            }

            if (graphicsQueueFamily == VK_QUEUE_FAMILY_IGNORED)
                return false;

            uint32_t universal_queue_index = 1;
            const uint32_t graphicsQueueIndex = 0;
            uint32_t compute_queue_index = 0;
            uint32_t transfer_queue_index = 0;

            if (computeQueueFamily == VK_QUEUE_FAMILY_IGNORED)
            {
                computeQueueFamily = graphicsQueueFamily;
                compute_queue_index = eastl::min(queue_props[graphicsQueueFamily].queueCount - 1, universal_queue_index);
                universal_queue_index++;
            }

            if (copyQueueFamily == VK_QUEUE_FAMILY_IGNORED)
            {
                copyQueueFamily = graphicsQueueFamily;
                transfer_queue_index = eastl::min(queue_props[graphicsQueueFamily].queueCount - 1, universal_queue_index);
                universal_queue_index++;
            }
            else if (copyQueueFamily == computeQueueFamily)
            {
                transfer_queue_index = eastl::min(queue_props[computeQueueFamily].queueCount - 1, 1u);
            }

            static const float graphicsQueuePrio = 0.5f;
            static const float computeQueuePrio = 1.0f;
            static const float transferQueuePrio = 1.0f;
            float prio[3] = { graphicsQueuePrio, computeQueuePrio, transferQueuePrio };

            unsigned queue_family_count = 0;
            VkDeviceQueueCreateInfo queueCreateInfo[3] = {};
            queueCreateInfo[queue_family_count].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queueCreateInfo[queue_family_count].queueFamilyIndex = graphicsQueueFamily;
            queueCreateInfo[queue_family_count].queueCount = eastl::min(universal_queue_index, queue_props[graphicsQueueFamily].queueCount);
            queueCreateInfo[queue_family_count].pQueuePriorities = prio;
            queue_family_count++;

            if (computeQueueFamily != graphicsQueueFamily)
            {
                queueCreateInfo[queue_family_count].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
                queueCreateInfo[queue_family_count].queueFamilyIndex = computeQueueFamily;
                queueCreateInfo[queue_family_count].queueCount = eastl::min(copyQueueFamily == computeQueueFamily ? 2u : 1u,
                    queue_props[computeQueueFamily].queueCount);
                queueCreateInfo[queue_family_count].pQueuePriorities = prio + 1;
                queue_family_count++;
            }

            if (copyQueueFamily != graphicsQueueFamily
                && copyQueueFamily != computeQueueFamily)
            {
                queueCreateInfo[queue_family_count].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
                queueCreateInfo[queue_family_count].queueFamilyIndex = copyQueueFamily;
                queueCreateInfo[queue_family_count].queueCount = 1;
                queueCreateInfo[queue_family_count].pQueuePriorities = prio + 2;
                queue_family_count++;
            }

            /* Setup device extensions to enable. */
            uint32_t ext_count = 0;
            vkEnumerateDeviceExtensionProperties(physical_device, nullptr, &ext_count, nullptr);
            vector<VkExtensionProperties> queried_extensions(ext_count);
            if (ext_count)
                vkEnumerateDeviceExtensionProperties(physical_device, nullptr, &ext_count, queried_extensions.data());

            const auto hasDeviceExtension = [&](const char* name) -> bool {
                auto itr = find_if(begin(queried_extensions), end(queried_extensions), [name](const VkExtensionProperties& e) -> bool {
                    return strcmp(e.extensionName, name) == 0;
                    });
                return itr != end(queried_extensions);
            };

            vector<const char*> enabledDeviceExtensions;
            if (!desc.headless)
            {
                enabledDeviceExtensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
            }

            if (hasDeviceExtension(VK_KHR_GET_MEMORY_REQUIREMENTS_2_EXTENSION_NAME))
            {
                vk_features.get_memory_requirements2 = true;
                enabledDeviceExtensions.push_back(VK_KHR_GET_MEMORY_REQUIREMENTS_2_EXTENSION_NAME);
            }

            if (vk_features.get_memory_requirements2
                && hasDeviceExtension(VK_KHR_DEDICATED_ALLOCATION_EXTENSION_NAME))
            {
                vk_features.dedicated = true;
                enabledDeviceExtensions.push_back(VK_KHR_DEDICATED_ALLOCATION_EXTENSION_NAME);
            }

            if (device_props.apiVersion >= VK_API_VERSION_1_1
                || hasDeviceExtension(VK_KHR_BIND_MEMORY_2_EXTENSION_NAME))
            {
                vk_features.bind_memory2 = true;
                enabledDeviceExtensions.push_back(VK_KHR_BIND_MEMORY_2_EXTENSION_NAME);
            }

            if (device_props.apiVersion >= VK_API_VERSION_1_1
                || hasDeviceExtension(VK_EXT_MEMORY_BUDGET_EXTENSION_NAME))
            {
                vk_features.memory_budget = true;
                enabledDeviceExtensions.push_back(VK_EXT_MEMORY_BUDGET_EXTENSION_NAME);
            }

            VkDeviceCreateInfo deviceCreateInfo = { VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO };
            deviceCreateInfo.queueCreateInfoCount = queue_family_count;
            deviceCreateInfo.pQueueCreateInfos = queueCreateInfo;
            deviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(enabledDeviceExtensions.size());
            deviceCreateInfo.ppEnabledExtensionNames = enabledDeviceExtensions.data();
            deviceCreateInfo.enabledLayerCount = 0;
            deviceCreateInfo.ppEnabledLayerNames = nullptr;

            for (auto* enabled_extension : enabledDeviceExtensions)
            {
                ALIMER_LOGI("Enabling device extension: %s.", enabled_extension);
            }

            if (vkCreateDevice(physical_device, &deviceCreateInfo, nullptr, &device) != VK_SUCCESS)
            {
                return false;
            }

            vkGetDeviceQueue(device, graphicsQueueFamily, graphicsQueueIndex, &graphicsQueue);
            vkGetDeviceQueue(device, computeQueueFamily, compute_queue_index, &computeQueue);
            vkGetDeviceQueue(device, copyQueueFamily, transfer_queue_index, &copyQueue);
        }

        // Create VMA allocator.
        {
            VmaVulkanFunctions vma_vulkan_func{};
            vma_vulkan_func.vkGetPhysicalDeviceProperties = vkGetPhysicalDeviceProperties;
            vma_vulkan_func.vkGetPhysicalDeviceMemoryProperties = vkGetPhysicalDeviceMemoryProperties;
            vma_vulkan_func.vkAllocateMemory = vkAllocateMemory;
            vma_vulkan_func.vkFreeMemory = vkFreeMemory;
            vma_vulkan_func.vkMapMemory = vkMapMemory;
            vma_vulkan_func.vkUnmapMemory = vkUnmapMemory;
            vma_vulkan_func.vkFlushMappedMemoryRanges = vkFlushMappedMemoryRanges;
            vma_vulkan_func.vkInvalidateMappedMemoryRanges = vkInvalidateMappedMemoryRanges;
            vma_vulkan_func.vkBindBufferMemory = vkBindBufferMemory;
            vma_vulkan_func.vkBindImageMemory = vkBindImageMemory;
            vma_vulkan_func.vkGetBufferMemoryRequirements = vkGetBufferMemoryRequirements;
            vma_vulkan_func.vkGetImageMemoryRequirements = vkGetImageMemoryRequirements;
            vma_vulkan_func.vkCreateBuffer = vkCreateBuffer;
            vma_vulkan_func.vkDestroyBuffer = vkDestroyBuffer;
            vma_vulkan_func.vkCreateImage = vkCreateImage;
            vma_vulkan_func.vkDestroyImage = vkDestroyImage;
            vma_vulkan_func.vkCmdCopyBuffer = vkCmdCopyBuffer;

            VmaAllocatorCreateInfo allocator_info{};
            allocator_info.physicalDevice = physical_device;
            allocator_info.device = device;
            allocator_info.instance = instance;

            if (vk_features.get_memory_requirements2
                && vk_features.dedicated)
            {
                allocator_info.flags |= VMA_ALLOCATOR_CREATE_KHR_DEDICATED_ALLOCATION_BIT;
                vma_vulkan_func.vkGetBufferMemoryRequirements2KHR = vkGetBufferMemoryRequirements2KHR;
                vma_vulkan_func.vkGetImageMemoryRequirements2KHR = vkGetImageMemoryRequirements2KHR;
            }

            if (vk_features.bind_memory2)
            {
                allocator_info.flags |= VMA_ALLOCATOR_CREATE_KHR_BIND_MEMORY2_BIT;
                vma_vulkan_func.vkBindBufferMemory2KHR = vkBindBufferMemory2KHR;
                vma_vulkan_func.vkBindImageMemory2KHR = vkBindImageMemory2KHR;
            }

            if (vk_features.memory_budget)
            {
                allocator_info.flags |= VMA_ALLOCATOR_CREATE_EXT_MEMORY_BUDGET_BIT;
                vma_vulkan_func.vkGetPhysicalDeviceMemoryProperties2KHR = vkGetPhysicalDeviceMemoryProperties2KHR;
            }

            allocator_info.pVulkanFunctions = &vma_vulkan_func;

            result = vmaCreateAllocator(&allocator_info, &memoryAllocator);

            if (result != VK_SUCCESS)
            {
                VK_THROW(result, "Cannot create allocator");
                return false;
            }
        }

        return true;
    }

    void VulkanGPUDevice::BackendShutdown()
    {
        if (instance == VK_NULL_HANDLE) {
            return;
        }

        if (memoryAllocator != VK_NULL_HANDLE)
        {
            VmaStats stats;
            vmaCalculateStats(memoryAllocator, &stats);

            ALIMER_LOGI("Total device memory leaked: {} bytes.", stats.total.usedBytes);

            vmaDestroyAllocator(memoryAllocator);
        }

        if (device != VK_NULL_HANDLE) {
            vkDestroyDevice(device, nullptr);
        }

        if (debug_messenger != VK_NULL_HANDLE) {
            vkDestroyDebugUtilsMessengerEXT(instance, debug_messenger, nullptr);
            debug_messenger = VK_NULL_HANDLE;
        }

        vkDestroyInstance(instance, nullptr);
        instance = VK_NULL_HANDLE;
    }

    void VulkanGPUDevice::WaitIdle()
    {
        VK_CHECK(vkDeviceWaitIdle(device));
    }

    shared_ptr<SwapChain> VulkanGPUDevice::CreateSwapChain(void* nativeWindow, const SwapChainDescriptor& desc)
    {
        return nullptr;
        //return make_shared<VulkanSwapChain>(this, nativeWindow, desc);
    }

    GPUDevice* CreateVulkanGPUDevice()
    {
        return new VulkanGPUDevice();
    }
}