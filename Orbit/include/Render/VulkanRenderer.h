/*! @file Render/VulkanRenderer.h */

#ifndef RENDER_VULKANRENDERER_H
#define RENDER_VULKANRENDERER_H
#pragma once

#if defined(RENDERER)
#error "The RENDERER macro was already defined elsewhere!"
#endif

/*! Define the RENDERER macro to the class of the used renderer, for easier preprocessor-defined compilation. */
#define RENDERER Orbit::VulkanRenderer

#include "VulkanUtils.h"

#include "VulkanGraphicsPipeline.h"
#include "VulkanModelRenderer.h"

#include "Render/Renderer.h"
#include <vulkan/vulkan.hpp>

namespace Orbit
{
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
		@param windowHandle A handle to the window on which to render.
		@param windowSize The size of the window.
		*/
		void init(void* windowHandle, const glm::ivec2& windowSize) override;

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
		@brief Cleans up everything in the object.
		*/
		void cleanup();

		/*!
		@brief Helper function to create a Vulkan Instance.
		@param windowHandle The handle of the window.
		@return A newly created Vulkan Instance.
		*/
		vk::Instance createInstance(void* windowHandle);

		/*!
		@brief Helper function to create a debug callback.
		@return A newly created Vulkan DebugReportCallback.
		*/
		vk::DebugReportCallbackEXT createDebugCallback();

		/*!
		@brief Helper function to create a Surface on which rendering operations are possible.
		@return A newly create Vulkan Surface.
		*/
		vk::SurfaceKHR createSurface(void* windowHandle);

		/*!
		@brief Helper function to pick out a physical device.
		@return The picked physical device.
		*/
		vk::PhysicalDevice pickPhysicalDevice();

		/*!
		@brief Helper function to create a Vulkan Device.
		@return The newly created Vulkan Device.
		*/
		vk::Device createDevice();

		/*!
		@brief Helper function to create a command pool for a queue family.
		@return The newly created Vulkan CommandPool.
		*/
		vk::CommandPool createCommandPool(int family);

		/*!
		@brief Helper function to destroy the debug callback.
		*/
		void destroyDebugCallback();

		/*! The instance used by the renderer. */
		vk::Instance _instance;
		/*! The debug callback used by the renderer. */
		vk::DebugReportCallbackEXT _debugCallback;

		/*! The surface that the renderer uses to render. */
		vk::SurfaceKHR _surface;

		/*! The physical device picked out by the renderer. */
		vk::PhysicalDevice _physicalDevice;
		/*! The logical device offering access to most Vulkan API functions. */
		vk::Device _device;

		/*! The graphics command queue. */
		vk::Queue _graphicsQueue;
		/*! The presentation command queue. */
		vk::Queue _presentQueue;
		/*! The transfer command queue. */
		vk::Queue _transferQueue;

		/*! The graphics command pool. */
		vk::CommandPool _graphicsCommandPool;
		/*! The transfer command pool. */
		vk::CommandPool _transferCommandPool;

		/*! Abstraction of the main graphics pipeline. */
		VulkanGraphicsPipeline _pipeline;
		/*! Abstraction of model rendering-related operations. */
		VulkanModelRenderer _modelRenderer;
	};
}

#endif //RENDER_VULKANRENDERER_H