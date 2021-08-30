#ifndef RENDER_CORE_H
#define RENDER_CORE_H

#include <memory>

class VulkanRHI;

class RenderCore{
	public:
	RenderCore();
	RenderCore(RenderCore&);
	void Init(void *view);

	private:
	std::shared_ptr<VulkanRHI> rhi;
};

#endif