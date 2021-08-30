#include "RenderCore.hpp"
#include "VulkanRHI.hpp"

RenderCore::RenderCore() : rhi(std::make_shared<VulkanRHI>())
{
}

RenderCore::RenderCore(RenderCore &other) : rhi(other.rhi)
{
}

void RenderCore::Init(void *view)
{
    rhi->Init(view);
}
