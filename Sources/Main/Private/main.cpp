#define GLFW_INCLUDE_VULKAN

#include "spdlog/spdlog.h"
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <stdlib.h>
#include <vulkan/vulkan.h>

#include "VulkanRHI.hpp"

static void error_callback(int error, const char *description)
{
    fprintf(stderr, "Error: %s\n", description);
}

int main()
{
    GLFWwindow *window = nullptr;
    glfwSetErrorCallback(error_callback);

    if (!glfwInit())
        exit(EXIT_FAILURE);

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    window = glfwCreateWindow(640, 480, "Wave Simulation", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    VulkanRHI rhi;
    rhi.Init();
    VkResult res = glfwCreateWindowSurface(rhi.m_inst, window, nullptr, &rhi.m_surface);
    rhi.Init2();

    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
    }
    glfwTerminate();
    exit(EXIT_SUCCESS);
}
