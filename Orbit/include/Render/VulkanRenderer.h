/*! @file Render/VulkanRenderer.h */

#ifndef RENDER_VULKANRENDERER_H
#define RENDER_VULKANRENDERER_H
#pragma once

#if defined(RENDERER)
#error "The RENDERER macro was already defined elsewhere!"
#endif

#define RENDERER Orbit::VulkanRenderer

#include "VulkanUtils.h"

#include "VulkanGraphicsPipeline.h"
#include "VulkanModelRenderer.h"

#include "Render/Renderer.h"
#include <vulkan/vulkan.hpp>

namespace Orbit
{
	class VulkanRenderer final : public Renderer
	{
	public:
		VulkanRenderer() = default;
		virtual ~VulkanRenderer();

		void init(void* windowHandle, const glm::ivec2& windowSize) override;

		RendererAPI getAPI() const override;

		void flagResize(const glm::ivec2& newSize) override;

		void loadModels(const std::vector<ModelCountPair>& models) override;

		void setupViewProjection(const glm::mat4& view, const glm::mat4& projection) override;

		void queueRender(const std::vector<ModelTransformsPair>& modelTransforms) override;

		void renderFrame() override;

		void waitDeviceIdle() override;

	private:
		void cleanup();

		vk::Instance createInstance(void* windowHandle);
		vk::DebugReportCallbackEXT createDebugCallback();
		vk::SurfaceKHR createSurface(void* windowHandle);
		vk::PhysicalDevice pickPhysicalDevice();
		vk::Device createDevice(); 
		vk::CommandPool createCommandPool(int family);

		void destroyDebugCallback();

		vk::Instance _instance;
		vk::DebugReportCallbackEXT _debugCallback;

		vk::SurfaceKHR _surface;

		vk::PhysicalDevice _physicalDevice;
		vk::Device _device;

		vk::Queue _graphicsQueue;
		vk::Queue _presentQueue;
		vk::Queue _transferQueue;

		vk::CommandPool _graphicsCommandPool;
		vk::CommandPool _transferCommandPool;

		VulkanGraphicsPipeline _pipeline;
		VulkanModelRenderer _modelRenderer;
	};
}

#endif //RENDER_VULKANRENDERER_H