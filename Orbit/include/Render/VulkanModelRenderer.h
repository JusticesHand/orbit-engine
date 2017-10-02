/*! @file Render/VulkanModelRenderer.h */

#ifndef RENDER_VULKANMODELRENDERER_H
#define RENDER_VULKANMODELRENDERER_H
#pragma once

#include <glm/glm.hpp>
#include <vulkan/vulkan.hpp>

#include "VulkanUtils.h"

namespace Orbit
{
	class Model;
	class VulkanGraphicsPipeline;

	class VulkanModelRenderer final
	{
	public:
		VulkanModelRenderer() = default;

		void init(vk::Device device, const VulkanQueueFamilies& families, const VulkanGraphicsPipeline* const pipeline);

		void recreateBuffers(const VulkanGraphicsPipeline* const newPipeline);

		void prepareRender(const Model& model, const glm::mat4& transform);

		void renderFrame(vk::Queue graphicsQueue, vk::Queue presentQueue) const;

		void cleanup();

	private:
		vk::CommandPool createCommandPool(int family);
		std::vector<vk::CommandBuffer> createGraphicsCommandBuffers();
		void recordCommandBuffers();

		vk::Device _device;
		VulkanQueueFamilies _families;
		const VulkanGraphicsPipeline* _pipeline;

		vk::CommandPool _graphicsCommandPool;
		vk::CommandPool _transferCommandPool;
		std::vector<vk::CommandBuffer> _graphicsCommandBuffers;

		vk::Semaphore _imageSemaphore;
		vk::Semaphore _renderSemaphore;
	};
}

#endif //RENDER_VULKANMODELRENDERER_H