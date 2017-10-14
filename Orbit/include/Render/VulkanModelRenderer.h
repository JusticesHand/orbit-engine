/*! @file Render/VulkanModelRenderer.h */

#ifndef RENDER_VULKANMODELRENDERER_H
#define RENDER_VULKANMODELRENDERER_H
#pragma once

#include <Render/Model.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <vulkan/vulkan.hpp>

#include <vector>
#include <unordered_map>

#include "VulkanUtils.h"

#include "Render/Renderer.h"

#include "Render/VulkanMemoryBuffer.h"

namespace Orbit
{
	class Model;
	class VulkanGraphicsPipeline;

	class VulkanModelRenderer final
	{
	public:
		VulkanModelRenderer() = default;

		void init(
			vk::PhysicalDevice physicalDevice,
			vk::Device device,
			const VulkanGraphicsPipeline& pipeline,
			const vk::CommandPool& graphicsCommandPool,
			const vk::CommandPool& transferCommandPool);

		void recreateBuffers(const VulkanGraphicsPipeline* const newPipeline);

		void loadModels(const std::vector<Renderer::ModelCountPair>& models, vk::Queue transferQueue);

		void setupViewProjection(const glm::mat4& viewProjectionTransform);

		void updateTransforms(const std::vector<Renderer::ModelTransformsPair>& modelTransforms);

		void renderFrame(vk::Queue graphicsQueue, vk::Queue presentQueue);

		void cleanup();

	private:
		std::vector<vk::CommandBuffer> createGraphicsCommandBuffers();
		void recordCommandBuffers();
		std::vector<vk::CommandBuffer> recordSecondaryBuffers(const vk::Framebuffer& framebuffer);

		vk::PhysicalDevice _physicalDevice;
		vk::Device _device;
		const VulkanGraphicsPipeline* _pipeline;

		const vk::CommandPool* _graphicsCommandPool;
		const vk::CommandPool* _transferCommandPool;

		struct ModelData
		{
			std::weak_ptr<Model> weakModel;
			size_t vertexIndex;
			size_t indicesIndex;

			size_t instanceIndex;
			size_t instanceCount;
		};

		std::vector<ModelData> _modelData;

		std::vector<vk::CommandBuffer> _graphicsCommandBuffers;
		std::unordered_map<VkFramebuffer, std::vector<vk::CommandBuffer>> _secondaryBufferMap;

		VulkanMemoryBuffer _mainBuffer;
		VulkanMemoryBuffer _transformBuffer;
		VulkanMemoryBuffer _animationBuffer;

		vk::Semaphore _imageSemaphore;
		vk::Semaphore _renderSemaphore;
	};
}

#endif //RENDER_VULKANMODELRENDERER_H