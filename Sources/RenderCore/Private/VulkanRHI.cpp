
#include "VulkanRHI.hpp"

#include <assert.h>
#include <iostream>
#include <vulkan/vulkan_metal.h>

#include "Utils.hpp"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "spdlog/spdlog.h"

VulkanRHI::VulkanRHI() : caMetalLayer(nullptr)
{
}

VulkanRHI::~VulkanRHI()
{
#ifdef VK_USE_PLATFORM_METAL_EXT
    destoryWindow();
#endif
}

void VulkanRHI::Init()
{
    initGlobalLayerProperties();
    initInstanceExtensionNames();
    initDeviceExtensionNames();
    initInstance();
    initEnumerateDevice();
    initWindowSize();
}

void VulkanRHI::Init2()
{
    initSwapchainExtension();
    initDevice();
    initCommandPool();
    initCommandBuffer();
    initSwapChain(VkImageUsageFlagBits::VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
    initDepthBuffer();
    initUniformBuffer();
    initDescriptorAndPipelineLayouts();
    initRenderpass(true);
}

#ifdef VK_USE_PLATFORM_METAL_EXT
void VulkanRHI::Init(void *view)
{
    caMetalLayer = view;
    Init();
}
#endif

void VulkanRHI::initGlobalLayerProperties()
{
    // LOG("initGlobalLayerProperties");
    spdlog::info("initGlobalLayerProperties");

    uint32_t instance_layer_count;
    std::vector<VkLayerProperties> vk_props;
    VkResult res;

    do
    {
        res = vkEnumerateInstanceLayerProperties(&instance_layer_count, nullptr);
        PANIC_IF_NOT_SUCCESS(res);

        if (instance_layer_count == 0)
        {
            return;
        }

        vk_props.resize(instance_layer_count);
        res = vkEnumerateInstanceLayerProperties(&instance_layer_count, vk_props.data());
    } while (res == VK_INCOMPLETE);

    for (auto prop : vk_props)
    {
        layerProperties layer_props;
        layer_props.properties = prop;
        initGlobalExtensionProperties(layer_props);
        m_instanceLayerProperties.push_back(layer_props);

        spdlog::info("LayerName: {}", layer_props.properties.layerName);
        // LOG(("LayerName: " + std::string(layer_props.properties.layerName)).c_str());
        for (auto ep : layer_props.instanceExtensions)
        {
            spdlog::info("instance extension: {}", ep.extensionName);
            // LOG(("instance extension:" + std::string(ep.extensionName)).c_str());
        }
        for (auto ep : layer_props.deviceExtensions)
        {
            spdlog::info("device extension: {}", ep.extensionName);
            // LOG(("device extension:" + std::string(ep.extensionName)).c_str());
        }
    }
}

void VulkanRHI::initGlobalExtensionProperties(layerProperties &layer_props)
{
    spdlog::info("initGlobalExtensionProperties");
    // LOG("initGlobalExtensionProperties");

    VkExtensionProperties *instance_extensions;
    uint32_t instance_extension_count;
    VkResult res;
    char *layer_name = nullptr;

    layer_name = layer_props.properties.layerName;
    do
    {
        res = vkEnumerateInstanceExtensionProperties(layer_name, &instance_extension_count, nullptr);
        PANIC_IF_NOT_SUCCESS(res);

        if (instance_extension_count == 0)
        {
            return;
        }

        layer_props.instanceExtensions.resize(instance_extension_count);
        instance_extensions = layer_props.instanceExtensions.data();

        res = vkEnumerateInstanceExtensionProperties(layer_name, &instance_extension_count, instance_extensions);
    } while (res == VK_INCOMPLETE);

    PANIC_IF_NOT_SUCCESS(res);
}

void VulkanRHI::initInstanceExtensionNames()
{
    // LOG("initInstanceExtensionNames");
    spdlog::info("initInstanceExtensionNames");

    m_instanceExtensionNames.push_back(VK_KHR_SURFACE_EXTENSION_NAME);
#ifdef VK_USE_PLATFORM_METAL_EXT
    m_instanceExtensionNames.push_back(VK_EXT_METAL_SURFACE_EXTENSION_NAME);
#endif
}

void VulkanRHI::initDeviceExtensionNames()
{
    spdlog::info("initDeviceExtensionNames");
    // LOG("initDeviceExtensionNames");
    m_deviceExtensionNames.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
}

void VulkanRHI::initDeviceExtensionProperties(layerProperties &layer_props)
{
    spdlog::info("initDeviceExtensionProperties");
    // LOG("initDeviceExtensionProperties");

    VkExtensionProperties *device_extensions;
    uint32_t device_extension_count;
    VkResult res;
    char *layer_name = nullptr;

    layer_name = layer_props.properties.layerName;

    do
    {
        res = vkEnumerateDeviceExtensionProperties(m_gpus[0], layer_name, &device_extension_count, nullptr);
        PANIC_IF_NOT_SUCCESS(res);

        if (device_extension_count == 0)
        {
            return;
        }

        layer_props.deviceExtensions.resize(device_extension_count);
        device_extensions = layer_props.deviceExtensions.data();
        res = vkEnumerateDeviceExtensionProperties(m_gpus[0], layer_name, &device_extension_count, device_extensions);
    } while (res == VK_INCOMPLETE);
}

void VulkanRHI::initInstance()
{
    spdlog::info("initInstance");
    // LOG("initInstance");

    // initialize the VkApplicationInfo structure
    VkApplicationInfo appInfo = {};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pNext = nullptr;
    appInfo.pApplicationName = m_appShortName;
    appInfo.applicationVersion = 1;
    appInfo.pEngineName = m_appShortName;
    appInfo.engineVersion = 1;
    appInfo.apiVersion = VK_API_VERSION_1_0;

    // initialize the VkInstanceCreateInfo structure
    VkInstanceCreateInfo instInfo = {};
    instInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instInfo.pNext = nullptr;
    instInfo.flags = 0;
    instInfo.pApplicationInfo = &appInfo;
    instInfo.enabledLayerCount = (uint32_t)(m_instanceLayerNames.size());
    instInfo.ppEnabledLayerNames = m_instanceLayerNames.size() ? m_instanceLayerNames.data() : NULL;
    instInfo.enabledExtensionCount = (uint32_t)(m_instanceExtensionNames.size());
    instInfo.ppEnabledExtensionNames = m_instanceExtensionNames.data();

    VkResult res = vkCreateInstance(&instInfo, nullptr, &m_inst);
    PANIC_IF_NOT_SUCCESS(res);
}

void VulkanRHI::initEnumerateDevice()
{
    spdlog::info("initEnumerateDevice");
    // LOG("initEnumerateDevice");

    VkResult res;
    uint32_t gpu_count;
    res = vkEnumeratePhysicalDevices(m_inst, &gpu_count, NULL);
    assert(gpu_count);

    m_gpus.resize(gpu_count);
    res = vkEnumeratePhysicalDevices(m_inst, &gpu_count, m_gpus.data());
    spdlog::info("gpu_count: {}", gpu_count);
    // LOG(("gpu_count:" + std::to_string(gpu_count)).c_str());

    vkGetPhysicalDeviceQueueFamilyProperties(m_gpus[0], &m_queueFamilyCount, nullptr);
    assert(m_queueFamilyCount);
    m_queueProps.resize(m_queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(m_gpus[0], &m_queueFamilyCount, m_queueProps.data());

    vkGetPhysicalDeviceMemoryProperties(m_gpus[0], &m_memoryProperties);
    vkGetPhysicalDeviceProperties(m_gpus[0], &m_gpuProps);
    spdlog::info("use gpu0: {}", m_gpuProps.deviceName);
    // LOG(("use gpu0: " + std::string(m_gpuProps.deviceName)).c_str());

    for (auto &layer_props : m_instanceLayerProperties)
    {
        initDeviceExtensionProperties(layer_props);
    }
}

void VulkanRHI::initWindowSize()
{
    spdlog::info("initWindowSize");
    // LOG("initWindowSize");

    mWidth = 500;
    mHeight = 500;
}

void VulkanRHI::initSwapchainExtension()
{
    spdlog::info("initSwapchainExtension");
    // LOG("initSwapchainExtension");

    VkResult res;
    // #ifdef VK_USE_PLATFORM_METAL_EXT
    //     VkMetalSurfaceCreateInfoEXT createInfo = {};
    //     createInfo.sType = VK_STRUCTURE_TYPE_METAL_SURFACE_CREATE_INFO_EXT;
    //     createInfo.pNext = nullptr;
    //     createInfo.flags = 0;
    //     createInfo.pLayer = caMetalLayer;
    //     res = vkCreateMetalSurfaceEXT(m_inst, &createInfo, nullptr, &m_surface);
    //     PANIC_IF_NOT_SUCCESS(res);
    // #endif

    // search graphics and present queue
    // VkBool32 *pSupportsPresent = (VkBool32 *)malloc(queueFamilyCount * sizeof(VkBool32));
    std::vector<VkBool32> supportsPresent;
    supportsPresent.resize(m_queueFamilyCount);
    for (uint32_t i = 0; i < m_queueFamilyCount; ++i)
    {
        vkGetPhysicalDeviceSurfaceSupportKHR(m_gpus[0], i, m_surface, &supportsPresent[i]);
    }

    m_graphicsQueueFamilyIndex = UINT32_MAX;
    m_presentQueueFamilyIndex = UINT32_MAX;
    for (uint32_t i = 0; i < m_queueFamilyCount; i++)
    {
        if ((m_queueProps[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0)
        {
            if (m_graphicsQueueFamilyIndex == UINT32_MAX)
                m_graphicsQueueFamilyIndex = i;
            if (supportsPresent[i] == VK_TRUE)
            {
                m_graphicsQueueFamilyIndex = i;
                m_presentQueueFamilyIndex = i;
                break;
            }
        }
    }

    if (m_presentQueueFamilyIndex == UINT32_MAX)
    {
        for (size_t i = 0; i < m_queueFamilyCount; i++)
        {
            if (supportsPresent[i] == VK_TRUE)
            {
                m_presentQueueFamilyIndex = (uint32_t)i;
                break;
            }
        }
    }
    // free(pSupportsPresent);
    supportsPresent.clear();

    if (m_graphicsQueueFamilyIndex == UINT32_MAX || m_presentQueueFamilyIndex == UINT32_MAX)
    {
        // spdlog::error("Could not find a queues for both graphics and present");
        PANIC("Could not find a queues for both graphics and present");
    }

    uint32_t formatCount;
    res = vkGetPhysicalDeviceSurfaceFormatsKHR(m_gpus[0], m_surface, &formatCount, nullptr);
    PANIC_IF_NOT_SUCCESS(res);
    std::vector<VkSurfaceFormatKHR> surfFormats;
    surfFormats.resize(formatCount);
    res = vkGetPhysicalDeviceSurfaceFormatsKHR(m_gpus[0], m_surface, &formatCount, surfFormats.data());
    PANIC_IF_NOT_SUCCESS(res);

    assert(formatCount > 0);
    m_format = surfFormats[0].format;
    for (size_t i = 0; i < formatCount; i++)
    {
        if (surfFormats[i].format == VK_FORMAT_B8G8R8A8_UNORM)
        {
            m_format = VK_FORMAT_B8G8R8A8_UNORM;
            break;
        }
    }
    surfFormats.clear();
}

void VulkanRHI::initDevice()
{
    spdlog::info("initDevice");
    // LOG("initDevice");

    VkResult res;

    VkDeviceQueueCreateInfo queueInfo = {};
    float queuePriorities[1] = {0.0};
    queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueInfo.pNext = nullptr;
    queueInfo.queueCount = 1;
    queueInfo.pQueuePriorities = queuePriorities;
    queueInfo.queueFamilyIndex = m_graphicsQueueFamilyIndex;

    VkDeviceCreateInfo deviceInfo = {};
    deviceInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceInfo.pNext = nullptr;
    deviceInfo.queueCreateInfoCount = 1;
    deviceInfo.pQueueCreateInfos = &queueInfo;
    deviceInfo.enabledExtensionCount = (uint32_t)(m_deviceExtensionNames.size());
    deviceInfo.ppEnabledExtensionNames = deviceInfo.enabledExtensionCount ? m_deviceExtensionNames.data() : nullptr;
    deviceInfo.pEnabledFeatures = nullptr;

    res = vkCreateDevice(m_gpus[0], &deviceInfo, nullptr, &m_device);
    PANIC_IF_NOT_SUCCESS(res);
}

void VulkanRHI::initCommandPool()
{
    spdlog::info("initCommandPool");
    // LOG("initCommandPool");

    VkResult res;
    VkCommandPoolCreateInfo cmdPoolInfo = {};
    cmdPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    cmdPoolInfo.pNext = nullptr;
    cmdPoolInfo.queueFamilyIndex = m_graphicsQueueFamilyIndex;
    cmdPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

    res = vkCreateCommandPool(m_device, &cmdPoolInfo, nullptr, &m_cmdPool);
    PANIC_IF_NOT_SUCCESS(res);
}

void VulkanRHI::initCommandBuffer()
{
    spdlog::info("initCommandBuffer");
    // LOG("initCommandBuffer");

    VkResult res;

    VkCommandBufferAllocateInfo cmdBufInfo = {};
    cmdBufInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    cmdBufInfo.pNext = nullptr;
    cmdBufInfo.commandPool = m_cmdPool;
    cmdBufInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    cmdBufInfo.commandBufferCount = 1;

    res = vkAllocateCommandBuffers(m_device, &cmdBufInfo, &m_cmdBuffer);
    PANIC_IF_NOT_SUCCESS(res);
}

void VulkanRHI::executeBeginCommandBuffer()
{
    VkResult res;
    VkCommandBufferBeginInfo cmdBufInfo = {};
    cmdBufInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    cmdBufInfo.pNext = nullptr;
    cmdBufInfo.flags = 0;
    cmdBufInfo.pInheritanceInfo = nullptr;

    res = vkBeginCommandBuffer(m_cmdBuffer, &cmdBufInfo);
    PANIC_IF_NOT_SUCCESS(res);
}

void VulkanRHI::initDeviceQueue()
{
    spdlog::info("initDeviceQueue");
    vkGetDeviceQueue(m_device, m_graphicsQueueFamilyIndex, 0, &m_graphicsQueue);
    if (m_graphicsQueueFamilyIndex == m_presentQueueFamilyIndex)
    {
        m_presentQueue = m_graphicsQueue;
    }
    else
    {
        vkGetDeviceQueue(m_device, m_presentQueueFamilyIndex, 0, &m_presentQueue);
    }
}

void VulkanRHI::initSwapChain(VkImageUsageFlags usageFlags)
{
    spdlog::info("initSwapChain");
    // LOG("initSwapChain");
    VkResult res;
    VkSurfaceCapabilitiesKHR surfCapabilities;
    res = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_gpus[0], m_surface, &surfCapabilities);
    PANIC_IF_NOT_SUCCESS(res);

    uint32_t presentModeCount;
    res = vkGetPhysicalDeviceSurfacePresentModesKHR(m_gpus[0], m_surface, &presentModeCount, nullptr);
    PANIC_IF_NOT_SUCCESS(res);

    if (presentModeCount == 0)
    {
        PANIC("presentModeCount == 0");
    }

    std::vector<VkPresentModeKHR> presentModes;
    presentModes.resize(presentModeCount);
    res = vkGetPhysicalDeviceSurfacePresentModesKHR(m_gpus[0], m_surface, &presentModeCount, presentModes.data());
    PANIC_IF_NOT_SUCCESS(res);

    VkExtent2D swapchainExtent;
    if (surfCapabilities.currentExtent.width == 0xFFFFFFFF)
    {
        swapchainExtent.width = mWidth;
        swapchainExtent.height = mHeight;
        if (swapchainExtent.width < surfCapabilities.minImageExtent.width)
        {
            swapchainExtent.width = surfCapabilities.minImageExtent.width;
        }
        else if (swapchainExtent.width > surfCapabilities.maxImageExtent.width)
        {
            swapchainExtent.width = surfCapabilities.maxImageExtent.width;
        }

        if (swapchainExtent.height < surfCapabilities.minImageExtent.height)
        {
            swapchainExtent.height = surfCapabilities.minImageExtent.height;
        }
        else if (swapchainExtent.height > surfCapabilities.maxImageExtent.height)
        {
            swapchainExtent.height = surfCapabilities.maxImageExtent.height;
        }
    }
    else
    {
        swapchainExtent = surfCapabilities.currentExtent;
    }

    VkPresentModeKHR swapchainPresentMode = VK_PRESENT_MODE_FIFO_KHR;
    uint32_t desiredNumberOfSwapChainImages = surfCapabilities.minImageCount;
    VkSurfaceTransformFlagBitsKHR preTransform;
    if (surfCapabilities.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR)
    {
        preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    }
    else
    {
        preTransform = surfCapabilities.currentTransform;
    }
    // Find a supported composite alpha mode - one of these is guaranteed to be set
    VkCompositeAlphaFlagBitsKHR compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    VkCompositeAlphaFlagBitsKHR compositeAlphaFlags[4] = {
        VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
        VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR,
        VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR,
        VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR,
    };
    for (uint32_t i = 0; i < sizeof(compositeAlphaFlags) / sizeof(compositeAlphaFlags[0]); i++)
    {
        if (surfCapabilities.supportedCompositeAlpha & compositeAlphaFlags[i])
        {
            compositeAlpha = compositeAlphaFlags[i];
            break;
        }
    }

    VkSwapchainCreateInfoKHR swapchainInfo = {};
    swapchainInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapchainInfo.pNext = nullptr;
    swapchainInfo.surface = m_surface;
    swapchainInfo.minImageCount = desiredNumberOfSwapChainImages;
    swapchainInfo.imageFormat = m_format;
    swapchainInfo.imageExtent.width = swapchainExtent.width;
    swapchainInfo.imageExtent.height = swapchainExtent.height;
    swapchainInfo.preTransform = preTransform;
    swapchainInfo.compositeAlpha = compositeAlpha;
    swapchainInfo.imageArrayLayers = 1;
    swapchainInfo.presentMode = swapchainPresentMode;
    swapchainInfo.oldSwapchain = VK_NULL_HANDLE;
#ifdef __ANDROID__
    swapchainInfo.clipped = true;
#else
    swapchainInfo.clipped = false;
#endif
    swapchainInfo.imageColorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR;
    swapchainInfo.imageUsage = usageFlags;
    swapchainInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    swapchainInfo.queueFamilyIndexCount = 0;
    swapchainInfo.pQueueFamilyIndices = nullptr;

    uint32_t queueFamilyIndices[2] = {
        (uint32_t)m_graphicsQueueFamilyIndex,
        (uint32_t)m_presentQueueFamilyIndex,
    };
    if (m_graphicsQueueFamilyIndex != m_presentQueueFamilyIndex)
    {
        swapchainInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        swapchainInfo.queueFamilyIndexCount = 2;
        swapchainInfo.pQueueFamilyIndices = queueFamilyIndices;
    }

    res = vkCreateSwapchainKHR(m_device, &swapchainInfo, nullptr, &m_swapChain);
    PANIC_IF_NOT_SUCCESS(res);

    res = vkGetSwapchainImagesKHR(m_device, m_swapChain, &m_swapChainImageCount, nullptr);
    PANIC_IF_NOT_SUCCESS(res);
    if (m_swapChainImageCount == 0)
    {
        PANIC("swapChainImageCount == 0");
    }

    std::vector<VkImage> swapChainImages;
    swapChainImages.resize(m_swapChainImageCount);
    res = vkGetSwapchainImagesKHR(m_device, m_swapChain, &m_swapChainImageCount, swapChainImages.data());
    PANIC_IF_NOT_SUCCESS(res);

    for (uint32_t i = 0; i < m_swapChainImageCount; i++)
    {
        SwapChainBuffer swapChainBuf;
        VkImageViewCreateInfo imageViewInfo = {};
        imageViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        imageViewInfo.pNext = nullptr;
        imageViewInfo.format = m_format;
        imageViewInfo.components.r = VK_COMPONENT_SWIZZLE_R;
        imageViewInfo.components.g = VK_COMPONENT_SWIZZLE_G;
        imageViewInfo.components.b = VK_COMPONENT_SWIZZLE_B;
        imageViewInfo.components.a = VK_COMPONENT_SWIZZLE_A;
        imageViewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        imageViewInfo.subresourceRange.baseMipLevel = 0;
        imageViewInfo.subresourceRange.levelCount = 1;
        imageViewInfo.subresourceRange.baseArrayLayer = 0;
        imageViewInfo.subresourceRange.layerCount = 1;
        imageViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        imageViewInfo.flags = 0;

        swapChainBuf.image = swapChainImages[i];
        imageViewInfo.image = swapChainBuf.image;

        res = vkCreateImageView(m_device, &imageViewInfo, nullptr, &swapChainBuf.view);
        PANIC_IF_NOT_SUCCESS(res);
        m_swapChainBuffers.push_back(swapChainBuf);
    }
    swapChainImages.clear();
    m_currentSwapChainBuffer = 0;

    if (!presentModes.empty())
    {
        presentModes.clear();
    }
}

void VulkanRHI::initDepthBuffer()
{
    spdlog::info("initDepthBuffer");
    // LOG("initDepthBuffer");

    VkResult res;
    bool pass;
    VkImageCreateInfo imageCreateInfo = {};
    VkFormatProperties props;

    if (m_depthBuf.format == VK_FORMAT_UNDEFINED)
    {
        spdlog::info("depth buffer format: {}, change to: {}", m_depthBuf.format, VK_FORMAT_D16_UNORM);
        m_depthBuf.format = VK_FORMAT_D16_UNORM;
    }

    const VkFormat depth_format = m_depthBuf.format;
    spdlog::info("depth buffer format: {}", m_depthBuf.format);
    vkGetPhysicalDeviceFormatProperties(m_gpus[0], m_depthBuf.format, &props);
    if (props.linearTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)
    {
        imageCreateInfo.tiling = VK_IMAGE_TILING_LINEAR;
    }
    else if (props.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)
    {
        imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    }
    else
    {
        /* Try other depth formats? */
        spdlog::error("depth_format {} unsupported", depth_format);
        // LOG(("depth_format " + std::to_string(depth_format) + " Unsupported").c_str());
        exit(-1);
    }

    imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageCreateInfo.pNext = NULL;
    imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
    imageCreateInfo.format = depth_format;
    imageCreateInfo.extent.width = mWidth;
    imageCreateInfo.extent.height = mHeight;
    imageCreateInfo.extent.depth = 1;
    imageCreateInfo.mipLevels = 1;
    imageCreateInfo.arrayLayers = 1;
    imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageCreateInfo.queueFamilyIndexCount = 0;
    imageCreateInfo.pQueueFamilyIndices = NULL;
    imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageCreateInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    imageCreateInfo.flags = 0;

    VkMemoryAllocateInfo memAllocInfo = {};
    memAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    memAllocInfo.pNext = NULL;
    memAllocInfo.allocationSize = 0;
    memAllocInfo.memoryTypeIndex = 0;

    VkImageViewCreateInfo viewCreateInfo = {};
    viewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewCreateInfo.pNext = NULL;
    viewCreateInfo.image = VK_NULL_HANDLE;
    viewCreateInfo.format = depth_format;
    viewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_R;
    viewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_G;
    viewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_B;
    viewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_A;
    viewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
    viewCreateInfo.subresourceRange.baseMipLevel = 0;
    viewCreateInfo.subresourceRange.levelCount = 1;
    viewCreateInfo.subresourceRange.baseArrayLayer = 0;
    viewCreateInfo.subresourceRange.layerCount = 1;
    viewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewCreateInfo.flags = 0;

    if (depth_format == VK_FORMAT_D16_UNORM_S8_UINT || depth_format == VK_FORMAT_D24_UNORM_S8_UINT ||
        depth_format == VK_FORMAT_D32_SFLOAT_S8_UINT)
    {
        viewCreateInfo.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
    }

    VkMemoryRequirements memReqs;

    /* Create image */
    res = vkCreateImage(m_device, &imageCreateInfo, NULL, &m_depthBuf.image);
    assert(res == VK_SUCCESS);

    vkGetImageMemoryRequirements(m_device, m_depthBuf.image, &memReqs);

    memAllocInfo.allocationSize = memReqs.size;
    /* Use the memory properties to determine the type of memory required */
    pass = memoryTypeFromProperties(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                                    &memAllocInfo.memoryTypeIndex);
    assert(pass);

    /* Allocate memory */
    res = vkAllocateMemory(m_device, &memAllocInfo, NULL, &m_depthBuf.mem);
    assert(res == VK_SUCCESS);

    /* Bind memory */
    res = vkBindImageMemory(m_device, m_depthBuf.image, m_depthBuf.mem, 0);
    assert(res == VK_SUCCESS);

    /* Create image view */
    viewCreateInfo.image = m_depthBuf.image;
    res = vkCreateImageView(m_device, &viewCreateInfo, NULL, &m_depthBuf.view);
    assert(res == VK_SUCCESS);
}

void VulkanRHI::initUniformBuffer()
{
    // LOG("initUniformBuffer");
    spdlog::info("initUniformBuffer");

    VkResult res;
    bool pass;

    float fov = glm::radians(45.0f);
    if (mWidth > mHeight)
    {
        fov *= static_cast<float>(mHeight) / static_cast<float>(mWidth);
    }
    mProjection = glm::perspective(fov, static_cast<float>(mHeight) / static_cast<float>(mWidth), 0.1f, 100.0f);
    mView = glm::lookAt(glm::vec3(-5, 3, -10), glm::vec3(0, 0, 0), glm::vec3(0, -1, 0));
    mModel = glm::mat4(1.0f);
    mClip = glm::mat4(
        1.0f, 0.0f, 0.0f, 0.0f, 
        0.0f, -1.0f, 0.0f, 0.0f, 
        0.0f, 0.0f, 0.5f, 0.0f, 
        0.0f, 0.0f, 0.5f, 1.0f);
    mMVP = mClip * mProjection * mView * mModel;

    VkBufferCreateInfo bufCreateInfo = {};
    bufCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufCreateInfo.pNext = nullptr;
    bufCreateInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    bufCreateInfo.size = sizeof(mMVP);
    bufCreateInfo.queueFamilyIndexCount = 0;
    bufCreateInfo.pQueueFamilyIndices = nullptr;
    bufCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    bufCreateInfo.flags = 0;
    res = vkCreateBuffer(m_device, &bufCreateInfo, nullptr, &mUniformData.buf);
    assert(res == VK_SUCCESS);

    VkMemoryRequirements memReqs;
    vkGetBufferMemoryRequirements(m_device, mUniformData.buf, &memReqs);

    VkMemoryAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.pNext = nullptr;
    allocInfo.memoryTypeIndex = 0;
    allocInfo.allocationSize = memReqs.size;
    pass = memoryTypeFromProperties(memReqs.memoryTypeBits,
                                    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                                    &allocInfo.memoryTypeIndex);
    assert(pass);

    res = vkAllocateMemory(m_device, &allocInfo, nullptr, &mUniformData.mem);
    assert(res == VK_SUCCESS);

    uint8_t *pData;
    res = vkMapMemory(m_device, mUniformData.mem, 0, memReqs.size, 0, (void **)&pData);
    assert(res == VK_SUCCESS);
    memcpy(pData, &mMVP, sizeof(mMVP));
    vkUnmapMemory(m_device, mUniformData.mem);

    res = vkBindBufferMemory(m_device, mUniformData.buf, mUniformData.mem, 0);
    assert(res == VK_SUCCESS);

    mUniformData.bufferInfo.buffer = mUniformData.buf;
    mUniformData.bufferInfo.offset = 0;
    mUniformData.bufferInfo.range = sizeof(mMVP);
}

void VulkanRHI::initDescriptorAndPipelineLayouts()
{
    // LOG("initDescriptorAndPipelineLayouts");
    spdlog::info("initDescriptorAndPipelineLayouts");
    VkDescriptorSetLayoutBinding layoutBindings[2];
    layoutBindings[0].binding = 0;
    layoutBindings[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    layoutBindings[0].descriptorCount = 1;
    layoutBindings[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    layoutBindings[0].pImmutableSamplers = nullptr;

    VkDescriptorSetLayoutCreateInfo descSetLayoutInfo = {};
    descSetLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    descSetLayoutInfo.pNext = nullptr;
    descSetLayoutInfo.flags = 0;
    descSetLayoutInfo.bindingCount = 1;
    descSetLayoutInfo.pBindings = layoutBindings;

    VkResult res;
    mDescLayout.resize(1);
    res = vkCreateDescriptorSetLayout(m_device, &descSetLayoutInfo, nullptr, mDescLayout.data());
    assert(res == VK_SUCCESS);

    VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = {};
    pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutCreateInfo.pNext = nullptr;
    pipelineLayoutCreateInfo.pushConstantRangeCount = 0;
    pipelineLayoutCreateInfo.pPushConstantRanges = nullptr;
    pipelineLayoutCreateInfo.setLayoutCount = 1;
    pipelineLayoutCreateInfo.pSetLayouts = mDescLayout.data();

    res = vkCreatePipelineLayout(m_device, &pipelineLayoutCreateInfo, nullptr, &mPipelineLayout);
    assert(res == VK_SUCCESS);
}

void VulkanRHI::initRenderpass(bool includePath,bool clear, VkImageLayout finalLayout, VkImageLayout initialLayout)
{
    // LOG("initRenderpass");
    spdlog::info("initRenderpass");
    assert(clear || (initialLayout != VK_IMAGE_LAYOUT_UNDEFINED));

    VkResult res;
    VkAttachmentDescription attachments[2];
    attachments[0].format = m_format;
    attachments[0].samples = VK_SAMPLE_COUNT_1_BIT;
    attachments[0].loadOp = clear ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_LOAD;
    attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachments[0].initialLayout = initialLayout;
    attachments[0].finalLayout = finalLayout;
    attachments[0].flags = 0;

    if (includePath) {
        attachments[1].format = m_depthBuf.format;
        attachments[1].samples = VK_SAMPLE_COUNT_1_BIT;
        attachments[1].loadOp = clear ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachments[1].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        attachments[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachments[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_STORE;
        attachments[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        attachments[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        attachments[1].flags = 0;
    }

    VkAttachmentReference colorRef = {};
    colorRef.attachment = 0;
    colorRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentReference depthRef = {};
    depthRef.attachment = 1;
    depthRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpassDesc = {};
    subpassDesc.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpassDesc.flags = 0;
    subpassDesc.inputAttachmentCount = 0;
    subpassDesc.pInputAttachments = nullptr;
    subpassDesc.colorAttachmentCount = 1;
    subpassDesc.pColorAttachments = &colorRef;
    subpassDesc.pResolveAttachments = nullptr;
    subpassDesc.pDepthStencilAttachment = includePath ? &depthRef : nullptr;
    subpassDesc.preserveAttachmentCount = 0;
    subpassDesc.pPreserveAttachments = nullptr;

    VkSubpassDependency subpassDep = {};
    subpassDep.srcSubpass = VK_SUBPASS_EXTERNAL;
    subpassDep.dstSubpass = 0;
    subpassDep.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    subpassDep.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    subpassDep.srcAccessMask = 0;
    subpassDep.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    subpassDep.dependencyFlags = 0;

    VkRenderPassCreateInfo renderPassCreateInfo = {};
    renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassCreateInfo.pNext = NULL;
    renderPassCreateInfo.attachmentCount = includePath ? 2 : 1;
    renderPassCreateInfo.pAttachments = attachments;
    renderPassCreateInfo.subpassCount = 1;
    renderPassCreateInfo.pSubpasses = &subpassDesc;
    renderPassCreateInfo.dependencyCount = 1;
    renderPassCreateInfo.pDependencies = &subpassDep;

    res = vkCreateRenderPass(m_device, &renderPassCreateInfo, NULL, &mRenderPass);
    assert(res == VK_SUCCESS);
}

bool VulkanRHI::memoryTypeFromProperties(uint32_t typeBits, VkFlags requirementsMask, uint32_t *typeIndex)
{
    // Search memtypes to find first index with those properties
    for (uint32_t i = 0; i < m_memoryProperties.memoryTypeCount; i++)
    {
        if ((typeBits & 1) == 1)
        {
            // Type is available, does it match user properties?
            if ((m_memoryProperties.memoryTypes[i].propertyFlags & requirementsMask) == requirementsMask)
            {
                *typeIndex = i;
                return true;
            }
        }
        typeBits >>= 1;
    }
    // No memory types matched, return failure
    return false;
}

#ifdef VK_USE_PLATFORM_METAL_EXT
void VulkanRHI::destoryWindow()
{
    LOG("destoryWindow");

    caMetalLayer = nullptr;
}
#endif
