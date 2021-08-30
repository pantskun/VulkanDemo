#ifndef VULKAN_CORE_UTILS_H
#define VULKAN_CORE_UTILS_H

#include <string>
#include <vulkan/vulkan.h>

std::string GetQueueFlagString(VkQueueFlags flag);

std::string GetVkResultString(VkResult res);

#define PANIC_IF_NOT_SUCCESS(res)              \
    if (res != VK_SUCCESS)                     \
    {                                          \
        PANIC(GetVkResultString(res).c_str()); \
    }

struct SwapChainBuffer
{
    VkImage image;
    VkImageView view;
};

#define LOG(s) printf("Log: %s [file: %s, line: %d]\n", s, __FILE__, __LINE__);
#define WARN(s) printf("Warn: %s [file: %s, line: %d]\n", s, __FILE__, __LINE__);
#define ERROR(s) printf("Error: %s [file: %s, line: %d]\n", s, __FILE__, __LINE__);
#define PANIC(s)                                                       \
    printf("Panic: %s [file: %s, line: %d]\n", s, __FILE__, __LINE__); \
    exit(EXIT_FAILURE);

#endif // VULKAN_CORE_UTILS_H