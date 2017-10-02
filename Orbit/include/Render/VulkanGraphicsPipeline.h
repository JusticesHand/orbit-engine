/*! @file Render/VulkanGraphicsPipeline.h */

#ifndef RENDER_VULKANGRAPHICSPIPELINE_H
#define RENDER_VULKANGRAPHICSPIPELINE_H
#pragma once

#include <glm/glm.hpp>
#include <vulkan/vulkan.hpp>

#include "VulkanUtils.h"

namespace Orbit
{
	class VulkanGraphicsPipeline final
	{
	public:
		VulkanGraphicsPipeline() = default;

		void init(
			const glm::ivec2& size,
			vk::PhysicalDevice physicalDevice, 
			vk::Device device, 
			vk::SurfaceKHR surface, 
			VulkanQueueFamilies families);

		void resize(const glm::ivec2& newSize);

		const std::vector<vk::Framebuffer>& getFramebuffers() const;

		vk::Extent2D getSwapExtent() const;

		vk::RenderPass getRenderPass() const;

		vk::Pipeline getGraphicsPipeline() const;

		vk::SwapchainKHR getSwapchain() const;

		void cleanup();

	private:
		vk::PhysicalDevice _physicalDevice;
		vk::Device _device;
		vk::SurfaceKHR _surface;
		
		vk::SurfaceFormatKHR chooseSurfaceFormat();
		vk::PresentModeKHR choosePresentMode();
		vk::Extent2D chooseExtent(const glm::ivec2& size);

		vk::SwapchainKHR createSwapchain(vk::SwapchainKHR oldSwapchain = nullptr);

		vk::RenderPass createRenderPass();
		vk::PipelineLayout createPipelineLayout();
		vk::Pipeline createGraphicsPipeline(vk::Pipeline oldPipeline = nullptr);
		std::vector<vk::Framebuffer> createFramebuffers();

		vk::SurfaceFormatKHR _surfaceFormat;
		vk::PresentModeKHR _presentMode;
		vk::Extent2D _swapExtent;

		vk::SwapchainKHR _swapchain;
		std::vector<vk::Image> _swapchainImages;
		std::vector<vk::ImageView> _swapchainImageViews;
		std::vector<vk::Framebuffer> _framebuffers;

		vk::RenderPass _renderPass;
		vk::PipelineLayout _pipelineLayout;
		vk::Pipeline _graphicsPipeline;
	};
}

#endif //RENDER_VULKANGRAPHICSPIPELINE_H