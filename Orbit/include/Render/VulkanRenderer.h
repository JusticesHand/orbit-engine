/*! @file Render/VulkanRenderer.h */

#ifndef RENDER_VULKANRENDERER_H
#define RENDER_VULKANRENDERER_H
#pragma once

#if defined(RENDERER)
#error "The RENDERER macro was already defined elsewhere!"
#endif

/*! Define the RENDERER macro to the class of the used renderer, for easier preprocessor-defined compilation. */
#define RENDERER Orbit::VulkanRenderer

#include "Renderer.h"
#include "VulkanGraphicsPipeline.h"
#include "VulkanMemoryBuffer.h"

#include <memory>

#include <vulkan/vulkan.hpp>

namespace Orbit
{
	class VulkanBase;

	/*!
	@brief Implementation of the Renderer virtual class, using Vulkan for rendering operations.
	Most of the work is done in the member classes Orbit::VulkanGraphicsPipeline and
	Orbit::VulkanModelRenderer - while the former handles pipeline creation, the latter handles
	command buffer creation and most memory/buffer allocations.
	*/
	class VulkanRenderer final : public Renderer
	{
	public:
		/*!
		@brief Default constructor for the class.
		*/
		VulkanRenderer() = default;

		/*!
		@brief Destructor for the class. Simply calls cleanup() to destroy all Vulkan objects.
		*/
		virtual ~VulkanRenderer();

		/*!
		@brief Initializes the renderer. Creates (and retrieves) base Vulkan objects and handles
		their lifetimes, before calling the members' initialization methods.
		@param window A pointer to the window that will accept the rendering.
		*/
		void init(const Window* window) override;

		/*!
		@brief Returns a value for the Renderer's API.
		@return The renderer's API.
		*/
		RendererAPI getAPI() const override;

		/*!
		@brief Flags the renderer for resize. Recreates the pipeline and command buffers to correspond
		to the new viewport/framebuffer sizes.
		@param newSize The new size of the window.
		*/
		void flagResize(const glm::ivec2& newSize) override;

		/*!
		@brief Loads the models into GPU-local memory.
		@param models The models to load into memory.
		*/
		void loadModels(const std::vector<ModelCountPair>& models) override;

		/*!
		@brief Sets up the viewProjection matrix for the current frame. Computes it together and writes
		it in device-local memory.
		@param view The view matrix.
		@param projection The projection matrix.
		*/
		void setupViewProjection(const glm::mat4& view, const glm::mat4& projection) override;

		/*!
		@brief Queues a render operation for the current frame with the updated model transformation data.
		@param modelTransforms The model and transformation data.
		*/
		void queueRender(const std::vector<ModelTransformsPair>& modelTransforms) override;

		/*!
		@brief Makes a frame be rendered.
		*/
		void renderFrame() override;

		/*!
		@brief Waits for the device to become idle, for synchronization purposes.
		*/
		void waitDeviceIdle() override;

	private:

		/*!
		@brief Definition of model data, determining where in memory models (and its data) is located.
		*/
		struct ModelData
		{
			/*! Weak pointer reference to the model. */
			std::weak_ptr<Model> weakModel;
			/*! Index of the vertices in the model buffer. */
			size_t vertexIndex;
			/*! Index of the indices in the model buffer. */
			size_t indicesIndex;

			/*! Index of the first instance of the model. */
			size_t instanceIndex;
			/*! Amount of instances of this model to render. */
			size_t instanceCount;
		};


		/*!
		@brief Helper function to record the primary command buffers. Also handles their creation.
		@param pipeline The pipeline to use for recording.
		@return The newly created and recorded command buffers.
		*/
		std::vector<vk::CommandBuffer> createPrimaryCommandBuffers(const VulkanGraphicsPipeline& pipeline);

		/*!
		@brief Helper function to record the primary command buffers. Optimizes creation by reusing the old command buffers.
		@param pipeline The pipeline to use for recording.
		@param oldBuffers The old command buffers, to optimize command buffer recording.
		@return The newly recorded command buffers.
		*/
		std::vector<vk::CommandBuffer> createPrimaryCommandBuffers(
			const VulkanGraphicsPipeline& pipeline,
			std::vector<vk::CommandBuffer>&& oldBuffers);

		/*!
		@brief Helper function to destroy all secondary command buffers at once.
		@param[in,out] secondaryBuffers The secondary buffers to destroy.
		*/
		void destroySecondaryBuffers(std::vector<std::vector<vk::CommandBuffer>>& secondaryBuffers);

		/*!
		@brief Helper function to record all secondary command buffers at once.
		@param modelData The model data to record.
		@param pipeline The pipeline to use for recording.
		@return The new collection of created secondary buffers.
		*/
		std::vector<std::vector<vk::CommandBuffer>> createAllSecondaryCommandBuffers(
			const std::vector<ModelData>& modelData,
			const VulkanGraphicsPipeline& pipeline);

		/*!
		@brief Helper function to record the secondary command buffers. 
		@param modelData The model data to record.
		@param pipeline The pipeline to use for recording.
		@param framebuffer The framebuffer to use for inheritance data.
		@return The created and recorded command buffers.
		*/
		std::vector<vk::CommandBuffer> createSecondaryCommandBuffers(
			const std::vector<ModelData>& modelData,
			const VulkanGraphicsPipeline& pipeline,
			const vk::Framebuffer& framebuffer);

		/*! Abstration of the base of the renderer. */
		std::shared_ptr<VulkanBase> _base = nullptr;

		/*! Abstraction of the main graphics pipeline. */
		std::shared_ptr<VulkanGraphicsPipeline> _pipeline = nullptr;

		/*! The collection of model data. */
		std::vector<ModelData> _modelData;

		/*! The main graphics command buffers, handling submitting secondary buffers. */
		std::vector<vk::CommandBuffer> _primaryGraphicsCommandBuffers;
		/*! The secondary graphics command buffers. */
		std::vector<std::vector<vk::CommandBuffer>> _secondaryGraphicsCommandBuffers;

		/*! Main buffer containing model vertex/index data. Device local. */
		VulkanMemoryBuffer _modelBuffer = nullptr;
		/*! Main buffer containing model instance transformations for the main pipeline. Host visible and coherent. */
		VulkanMemoryBuffer _transformBuffer = nullptr;
		/*! Buffer containing animation data for each instance of the models. */
		VulkanMemoryBuffer _animationBuffer = nullptr;

		/*! Semaphore controlling access to image availability. */
		vk::Semaphore _imageSemaphore;
		/*! Semaphore controlling access to render operations. */
		vk::Semaphore _renderSemaphore;
	};
}

#endif //RENDER_VULKANRENDERER_H