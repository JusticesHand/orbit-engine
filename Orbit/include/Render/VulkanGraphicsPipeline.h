/*! @file Render/VulkanGraphicsPipeline.h */

#ifndef RENDER_VULKANGRAPHICSPIPELINE_H
#define RENDER_VULKANGRAPHICSPIPELINE_H
#pragma once

#include <glm/glm.hpp>
#include <vulkan/vulkan.hpp>

#include "VulkanImage.h"

#include "VulkanUtils.h"

namespace Orbit
{
	/*!
	@brief Abstraction of the rendering pipeline portion of Vulkan. Creates the pipeline itself,
	along with the render pass, pipeline layout, descriptor sets, etc.
	*/
	class VulkanGraphicsPipeline final
	{
	public:
		/*!
		@brief Constructor for the class. Leaves everything in an undefined state.
		*/
		VulkanGraphicsPipeline() = default;

		/*!
		@brief Initializes the object with the parameters.
		@param size The size on which to base the swapchain's extent.
		@param physicalDevice The physical device hosting the pipeline.
		@param device The logical device used to create elements.
		@param surface The surface that will accept the rendering.
		@param transferPool The command pool to create transfer buffers.
		*/
		void init(
			const glm::ivec2& size,
			vk::PhysicalDevice physicalDevice, 
			vk::Device device, 
			vk::SurfaceKHR surface, 
			vk::CommandPool transferPool);

		/*!
		@brief Resizes the graphics pipeline by destroying everything requiring knowledge
		of the framebuffer sizes.
		@param newSize The new size of the window.
		*/
		void resize(const glm::ivec2& newSize);

		/*!
		@brief Getter for the framebuffers of the pipeline.
		@return The framebuffers of the pipeline.
		*/
		const std::vector<vk::Framebuffer>& getFramebuffers() const;

		/*!
		@brief Getter for the swapchain's extent.
		@return The swapchain's extent.
		*/
		vk::Extent2D getSwapExtent() const;

		/*!
		@brief Getter for the pipeline's render pass.
		@return The pipeline's render pass.
		*/
		vk::RenderPass getRenderPass() const;

		/*!
		@brief Getter for the pipeline's descriptor set.
		@return The pipeline's descriptor set.
		*/
		vk::DescriptorSet getDescriptorSet() const;

		/*!
		@brief Getter for the pipeline's layout.
		@return The pipeline's layout.
		*/
		vk::PipelineLayout getPipelineLayout() const;

		/*!
		@brief Getter for the pipeline itself.
		@return The pipeline.
		*/
		vk::Pipeline getGraphicsPipeline() const;

		/*!
		@brief Getter for the pipeline's swapchain.
		@return The swapchain.
		*/
		vk::SwapchainKHR getSwapchain() const;

		/*!
		@brief Frees objects created by the pipeline.
		*/
		void cleanup();

	private:
		/*! The physical device containing the pipeline. */
		vk::PhysicalDevice _physicalDevice;
		/*! The logical device used to create the pipeline. */
		vk::Device _device;
		/*! The surface the pipeline uses to render. */
		vk::SurfaceKHR _surface;
		
		/*!
		@brief Helper function to choose the surface format.
		@return The chosen surface format.
		*/
		vk::SurfaceFormatKHR chooseSurfaceFormat();

		/*!
		@brief Helper function to choose the present mode.
		@return The chosen present mode.
		*/
		vk::PresentModeKHR choosePresentMode();

		/*!
		@brief Helper function to choose the extent.
		@param size The window's size.
		@return The chosen extent.
		*/
		vk::Extent2D chooseExtent(const glm::ivec2& size);

		/*!
		@brief Creates the swapchain. Optionally takes the old swapchain to possibly optimize creation.
		@param oldSwapchain The old swapchain, or empty to ignore.
		@return The newly created swapchain.
		*/
		vk::SwapchainKHR createSwapchain(vk::SwapchainKHR oldSwapchain = nullptr);

		/*!
		@brief Helper function to create the render pass.
		@return The created render pass.
		*/
		vk::RenderPass createRenderPass();

		/*!
		@brief Helper function to create the pipeline layout.
		@return The created pipeline layout.
		*/
		vk::PipelineLayout createPipelineLayout();

		/*!
		@brief Helper function to create the descriptor set layout.
		@return The created descriptor set layout.
		*/
		vk::DescriptorSetLayout createDescriptorSetLayout();

		/*!
		@brief Helper function to create the descriptor pool.
		@return The created descriptor pool.
		*/
		vk::DescriptorPool createDescriptorPool();

		/*!
		@brief Helper function to create the descriptor set.
		@return The created descriptor set.
		*/
		vk::DescriptorSet createDescriptorSet();

		/*!
		@brief Helper function to create the actual graphics pipeline. Takes an optional parameter of the
		old pipeline to hopefully optimize pipeline creation.
		@param oldPipeline The old pipeline, or empty if creating the first one.
		@return The created pipeline.
		*/
		vk::Pipeline createGraphicsPipeline(vk::Pipeline oldPipeline = nullptr);

		/*!
		@brief Helper function to create the framebuffers.
		@return The created framebuffers.
		*/
		std::vector<vk::Framebuffer> createFramebuffers();

		/*! The chosen surface format. */
		vk::SurfaceFormatKHR _surfaceFormat;
		/*! The chosen present mode. */
		vk::PresentModeKHR _presentMode;
		/*! The chosen swap extent. */
		vk::Extent2D _swapExtent;

		/*! The pipeline's swapchain. */
		vk::SwapchainKHR _swapchain;
		/*! The swapchain's images. */
		std::vector<vk::Image> _swapchainImages;
		/*! The swapchain's imageviews. */
		std::vector<vk::ImageView> _swapchainImageViews;
		/*! The swapchain's framebuffers. */
		std::vector<vk::Framebuffer> _framebuffers;

		/*! The pipeline's render pass. */
		vk::RenderPass _renderPass;
		/*! The pipeline's layout. */
		vk::PipelineLayout _pipelineLayout;
		/*! The pipeline's descriptor set layout. */
		vk::DescriptorSetLayout _descriptorSetLayout;
		/*! The pipeline's descriptor pool. */
		vk::DescriptorPool _descriptorPool;
		/*! The pipeline's descriptor set. */
		vk::DescriptorSet _descriptorSet;
		/*! The actual graphics pipeline. */
		vk::Pipeline _graphicsPipeline;

		/*! The image containing depth information. */
		VulkanImage _depthImage;
	};
}

#endif //RENDER_VULKANGRAPHICSPIPELINE_H