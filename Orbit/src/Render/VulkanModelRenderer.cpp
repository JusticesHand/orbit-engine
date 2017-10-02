/*! @file Render/VulkanModelRenderer.cpp */

#include "Render/VulkanModelRenderer.h"
#include "Render/VulkanGraphicsPipeline.h"

using namespace Orbit;

void VulkanModelRenderer::init(vk::Device device, const VulkanQueueFamilies& families, const VulkanGraphicsPipeline* const pipeline)
{
	_device = device;
	_families = families;
	_pipeline = pipeline;

	_graphicsCommandPool = createCommandPool(families.graphicsQueueFamily);
	_transferCommandPool = createCommandPool(families.transferQueueFamily);

	_graphicsCommandBuffers = createGraphicsCommandBuffers();
	recordCommandBuffers();

	// Semaphore create info is currently empty.
	_imageSemaphore = _device.createSemaphore({});
	_renderSemaphore = _device.createSemaphore({});
}

void VulkanModelRenderer::recreateBuffers(const VulkanGraphicsPipeline* const newPipeline)
{
	_pipeline = newPipeline;

	_device.destroyCommandPool(_transferCommandPool);
	_device.destroyCommandPool(_graphicsCommandPool);

	_graphicsCommandBuffers.clear();

	_graphicsCommandPool = createCommandPool(_families.graphicsQueueFamily);
	_transferCommandPool = createCommandPool(_families.transferQueueFamily);

	_graphicsCommandBuffers = createGraphicsCommandBuffers();
	recordCommandBuffers();
}

void VulkanModelRenderer::prepareRender(const Model& model, const glm::mat4& transform)
{
	// TODO; stub
}

void VulkanModelRenderer::renderFrame(vk::Queue graphicsQueue, vk::Queue presentQueue) const
{
	vk::SwapchainKHR swapchain = _pipeline->getSwapchain();
	auto imageResult = _device.acquireNextImageKHR(swapchain, std::numeric_limits<uint64_t>::max(), _imageSemaphore, nullptr);
	
	if (imageResult.result != vk::Result::eSuccess)
		throw std::runtime_error("Could not acquire next image!");

	uint32_t imageIndex = imageResult.value;

	vk::PipelineStageFlags waitStages = vk::PipelineStageFlagBits::eColorAttachmentOutput;

	std::array<vk::SubmitInfo, 1> submitInfos;
	submitInfos[0]
		.setWaitSemaphoreCount(1)
		.setPWaitSemaphores(&_imageSemaphore)
		.setPWaitDstStageMask(&waitStages)
		.setCommandBufferCount(1)
		.setPCommandBuffers(&_graphicsCommandBuffers[imageIndex])
		.setSignalSemaphoreCount(1)
		.setPSignalSemaphores(&_renderSemaphore);

	graphicsQueue.submit(submitInfos, nullptr);

	vk::PresentInfoKHR presentInfo;
	presentInfo
		.setWaitSemaphoreCount(1)
		.setPWaitSemaphores(&_renderSemaphore)
		.setSwapchainCount(1)
		.setPSwapchains(&swapchain)
		.setPImageIndices(&imageIndex);

	presentQueue.presentKHR(presentInfo);
}

void VulkanModelRenderer::cleanup()
{
	_device.destroySemaphore(_renderSemaphore);
	_device.destroySemaphore(_imageSemaphore);

	_device.destroyCommandPool(_transferCommandPool);
	_device.destroyCommandPool(_graphicsCommandPool);
}

vk::CommandPool VulkanModelRenderer::createCommandPool(int family)
{
	vk::CommandPoolCreateInfo createInfo;
	createInfo
		.setQueueFamilyIndex(family)
		.setFlags(vk::CommandPoolCreateFlagBits::eTransient | vk::CommandPoolCreateFlagBits::eResetCommandBuffer);

	return _device.createCommandPool(createInfo);
}

std::vector<vk::CommandBuffer> VulkanModelRenderer::createGraphicsCommandBuffers()
{
	const std::vector<vk::Framebuffer>& framebuffers = _pipeline->getFramebuffers();

	vk::CommandBufferAllocateInfo allocInfo;
	allocInfo
		.setCommandPool(_graphicsCommandPool)
		.setLevel(vk::CommandBufferLevel::ePrimary)
		.setCommandBufferCount(static_cast<uint32_t>(framebuffers.size()));

	return _device.allocateCommandBuffers(allocInfo);
}

void VulkanModelRenderer::recordCommandBuffers()
{
	const std::vector<vk::Framebuffer>& framebuffers = _pipeline->getFramebuffers();
	for (size_t i = 0; i < _graphicsCommandBuffers.size(); i++)
	{
		vk::CommandBuffer& buffer = _graphicsCommandBuffers[i];

		// Implicitly build begin info with necessary flag.
		buffer.begin({ vk::CommandBufferUsageFlagBits::eSimultaneousUse });

		vk::ClearValue clearColor{ std::array<float, 4>{ 0.f, 0.f, 0.f, 0.f } };

		vk::Rect2D renderArea;
		renderArea
			.setOffset({ 0, 0 })
			.setExtent(_pipeline->getSwapExtent());

		vk::RenderPassBeginInfo renderPassBeginInfo;
		renderPassBeginInfo
			.setRenderPass(_pipeline->getRenderPass())
			.setFramebuffer(framebuffers[i])
			.setRenderArea(renderArea)
			.setClearValueCount(1)
			.setPClearValues(&clearColor);

		buffer.beginRenderPass(renderPassBeginInfo, vk::SubpassContents::eInline);

		buffer.bindPipeline(vk::PipelineBindPoint::eGraphics, _pipeline->getGraphicsPipeline());

		// TODO: Better drawing :)
		// Best practices seem to be creating secondary command buffers for each object and binding them
		// together in a single primary command buffer on the main thread.
		buffer.draw(3, 1, 0, 0);

		buffer.endRenderPass();

		buffer.end();
	}
}