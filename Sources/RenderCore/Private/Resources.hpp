#ifndef VULKAN_CORE_RESOURCES_H
#define VULKAN_CORE_RESOURCES_H

#include <vulkan/vulkan_core.h>

struct ImageResource
{
    ImageResource() : format(VK_FORMAT_UNDEFINED)
    {
    }

    VkFormat format;

    VkImage image;
    VkDeviceMemory mem;
    VkImageView view;
};

struct BufferResource{
    VkBuffer buf;
    VkDeviceMemory mem;
    VkDescriptorBufferInfo bufferInfo;
};

#endif // VULKAN_CORE_RESOURCES_H