#ifndef VULKAN_CORE_RHI_H
#define VULKAN_CORE_RHI_H

#include <MoltenVK/vk_mvk_moltenvk.h>
#include <vulkan/vulkan.h>

#include <string>
#include <vector>

#include "Resources.hpp"
#include "Utils.hpp"

struct QueueFamilyIndex
{
    int8_t graphicsQueueIndex;
    int8_t computeQueueIndex;
    int8_t transferQueueIndex;
    int8_t sparseBindingQueueIndex;
    int8_t protectedQueueIndex;
};

struct layerProperties
{
    VkLayerProperties properties;
    std::vector<VkExtensionProperties> instanceExtensions;
    std::vector<VkExtensionProperties> deviceExtensions;
};

class VulkanRHI
{
  public:
    VulkanRHI();
    ~VulkanRHI();

    void Init();
    void Init2();

#ifdef VK_USE_PLATFORM_METAL_EXT
    void Init(void *view);
#endif

  private:
    void initGlobalLayerProperties();
    void initInstanceExtensionNames();
    void initDeviceExtensionNames();
    void initInstance();
    void initEnumerateDevice();
    void initWindowSize();
    void initSwapchainExtension();
    void initDevice();
    void initCommandPool();
    void initCommandBuffer();
    void executeBeginCommandBuffer();
    void initDeviceQueue();
    void initSwapChain(VkImageUsageFlags usageFlags);
    void initDepthBuffer();
    void initUniformBuffer();
    void initDescriptorAndPipelineLayouts();
    void initRenderpass(bool includePath, bool clear = true,
                        VkImageLayout finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
                        VkImageLayout initialLayout = VK_IMAGE_LAYOUT_UNDEFINED);

    void initDeviceExtensionProperties(layerProperties &layer_props);
    void initGlobalExtensionProperties(layerProperties &layer_props);

    bool memoryTypeFromProperties(uint32_t typeBits, VkFlags requirements_mask, uint32_t *typeIndex);

#ifdef VK_USE_PLATFORM_METAL_EXT
    void destoryWindow();
#endif

  public:
#ifdef VK_USE_PLATFORM_METAL_EXT
    void *caMetalLayer;
#endif
    VkSurfaceKHR m_surface;

    char *m_appShortName;

    VkInstance m_inst;

    std::vector<const char *> m_instanceLayerNames;
    std::vector<layerProperties> m_instanceLayerProperties;
    std::vector<const char *> m_instanceExtensionNames;
    std::vector<VkExtensionProperties> m_instanceExtensionProperties;

    std::vector<const char *> m_deviceExtensionNames;
    std::vector<VkExtensionProperties> m_deviceExtensionProperties;

    uint32_t m_queueFamilyCount;
    std::vector<VkQueueFamilyProperties> m_queueProps;

    std::vector<VkPhysicalDevice> m_gpus;
    VkPhysicalDeviceMemoryProperties m_memoryProperties;
    VkPhysicalDeviceProperties m_gpuProps;

    uint32_t m_graphicsQueueFamilyIndex;
    uint32_t m_presentQueueFamilyIndex;
    VkQueue m_graphicsQueue;
    VkQueue m_presentQueue;
    VkDevice m_device;

    VkCommandPool m_cmdPool;
    VkCommandBuffer m_cmdBuffer;

    VkFormat m_format;
    int32_t mWidth;
    int32_t mHeight;

    VkSwapchainKHR m_swapChain;
    uint32_t m_swapChainImageCount;
    std::vector<SwapChainBuffer> m_swapChainBuffers;
    uint32_t m_currentSwapChainBuffer;

    ImageResource m_depthBuf;

    glm::mat4 mProjection;
    glm::mat4 mView;
    glm::mat4 mModel;
    glm::mat4 mClip;
    glm::mat4 mMVP;
    BufferResource mUniformData;

    std::vector<VkDescriptorSetLayout> mDescLayout;
    VkPipelineLayout mPipelineLayout;
};

#endif // VULKAN_CORE_RHI_H