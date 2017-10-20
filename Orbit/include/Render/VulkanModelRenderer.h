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
	class VulkanGraphicsPipeline;
	class Model;

	/*!
	@brief Abstraction of the rendering (commands) portion of the Vulkan Renderer.
	Handles actual rendering as well as memory transfers.
	*/
	class VulkanModelRenderer final
	{
	public:
		/*!
		@brief Default constructor for the class.
		*/
		VulkanModelRenderer() = default;

		/*!
		@brief Initializes the instance.
		@param physicalDevice The physical device on which to render.
		@param device The logical device allowing access to most of the API.
		@param pipeline A reference to the pipeline abstraction used by the renderer.
		@param graphicsCommandPool A reference to the graphics command pool.
		@param transferCommandPool A reference to the transfer command pool.
		*/
		void init(
			vk::PhysicalDevice physicalDevice,
			vk::Device device,
			const VulkanGraphicsPipeline& pipeline,
			const vk::CommandPool& graphicsCommandPool,
			const vk::CommandPool& transferCommandPool);

		/*!
		@brief Recreates the command buffers (and other relevant data).
		@param newPipeline The new pipeline abstraction object.
		*/
		void recreateBuffers(const VulkanGraphicsPipeline& newPipeline);

		/*!
		@brief Loads the models in parameter to device local memory, and calculates necessary offsets.
		@param models The models (and instance counts) to load.
		@param transferQueue The queue on which transfer commands are submitted.
		*/
		void loadModels(const std::vector<Renderer::ModelCountPair>& models, vk::Queue transferQueue);

		/*!
		@brief Sets up the viewProjection matrix in device memory.
		@param viewProjectionTransform The viewProjection transformation matrix.
		*/
		void setupViewProjection(const glm::mat4& viewProjectionTransform);

		/*!
		@brief Updates the transformations for the models, taking the model transforms in parameter.
		Note that the models must have the correct amount of instances when loaded with loadModels.
		@param modelTransforms The models and their respective transformations, assumed to be in instance order.
		*/
		void updateTransforms(const std::vector<Renderer::ModelTransformsPair>& modelTransforms);

		/*!
		@brief Renders a frame, submitting the commands to the queues in parameter.
		@param graphicsQueue The graphics queue on which to submit commands.
		@param presentQueue The present queue on which to submit commands.
		*/
		void renderFrame(vk::Queue graphicsQueue, vk::Queue presentQueue);

		/*!
		@brief Cleans up the Vulkan objects contained within.
		*/
		void cleanup();

	private:
		/*!
		@brief Creates the graphics command buffers from the framebuffers.
		@return The created command buffers.
		*/
		std::vector<vk::CommandBuffer> createGraphicsCommandBuffers();

		/*!
		@brief Records the (primary) command buffers.
		*/
		void recordCommandBuffers();

		/*!
		@brief Records the secondary buffers for a framebuffer.
		@param framebuffer The framebuffer whose state to keep.
		@return The recorded secondary buffers.
		*/
		std::vector<vk::CommandBuffer> recordSecondaryBuffers(const vk::Framebuffer& framebuffer);

		/*! The physical device used for rendering. */
		vk::PhysicalDevice _physicalDevice;
		/*! The logical device used for rendering. */
		vk::Device _device;
		/*! A pointer to the graphics pipeline used. */
		const VulkanGraphicsPipeline* _pipeline;

		/*! A pointer to the graphics command pool. */
		const vk::CommandPool* _graphicsCommandPool;
		/*! A pointer to the transfer command pool. */
		const vk::CommandPool* _transferCommandPool;

		/*!
		@brief Definition of model data, determining where in memory models (and its data) is located.
		*/
		struct ModelData
		{
			std::weak_ptr<Model> weakModel;
			size_t vertexIndex;
			size_t indicesIndex;

			size_t instanceIndex;
			size_t instanceCount;
		};

		/*! The collection of model data. */
		std::vector<ModelData> _modelData;

		/*! The (main) graphics command buffers. */
		std::vector<vk::CommandBuffer> _graphicsCommandBuffers;
		/*! A map of framebuffers to secondary command buffers (as the order of rendering isn't set in stone). */
		std::unordered_map<VkFramebuffer, std::vector<vk::CommandBuffer>> _secondaryBufferMap;

		/*! The main buffer, containing model and vertex data. Device-local. */
		VulkanMemoryBuffer _mainBuffer;
		/*! The buffer containing transformation data. Host coherent and visible, as it changes every frame. */
		VulkanMemoryBuffer _transformBuffer;
		/*! The buffer containing animation data. Nothing for now, as animation isn't implemented (yet). */
		//VulkanMemoryBuffer _animationBuffer;

		/*! Semaphore controlling access to an image availability. */
		vk::Semaphore _imageSemaphore;
		/*! Semaphore controlling access to render operations. */
		vk::Semaphore _renderSemaphore;
	};
}

#endif //RENDER_VULKANMODELRENDERER_H