/*! @file Render/VulkanGraphicsPipeline.h */

#ifndef RENDER_VULKANGRAPHICSPIPELINE_H
#define RENDER_VULKANGRAPHICSPIPELINE_H
#pragma once

#include <glm/glm.hpp>
#include <vulkan/vulkan.hpp>

#include "VulkanBase.h"
#include "VulkanImage.h"

namespace Orbit
{
	class VulkanBase;

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
		VulkanGraphicsPipeline(std::nullptr_t);

		/*!
		@brief Constructor building the pipeline itself.
		@param base The renderer's base.
		@param size The size of the extent.
		*/
		explicit VulkanGraphicsPipeline(std::shared_ptr<const VulkanBase> base, const glm::ivec2& size);

		VulkanGraphicsPipeline(const VulkanGraphicsPipeline&) = delete;
		VulkanGraphicsPipeline& operator=(const VulkanGraphicsPipeline&) = delete;

		/*!
		@brief Move constructor for the class. Invalidates all of rhs while moving its objects to this.
		@param rhs The graphics pipeline to move.
		*/
		VulkanGraphicsPipeline(VulkanGraphicsPipeline&& rhs);

		/*!
		@brief Move assignment operator for the class. Invalidates all of rhs while moving its objets to this.
		@param rhs The graphics pipeline to move.
		@return A reference to this.
		*/
		VulkanGraphicsPipeline& operator=(VulkanGraphicsPipeline&& rhs);

		/*!
		@brief Destructor for the class. Destroys any created objects, if applicable.
		*/
		~VulkanGraphicsPipeline();

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
		const std::vector<vk::Framebuffer>& framebuffers() const;

		/*!
		@brief Getter for the swapchain's extent.
		@return The swapchain's extent.
		*/
		vk::Extent2D swapExtent() const;

		/*!
		@brief Getter for the pipeline's render pass.
		@return The pipeline's render pass.
		*/
		vk::RenderPass renderPass() const;

		/*!
		@brief Updates the descriptor pool within to allow the new maximum number of allocated descriptor sets.
		Note that this will destroy the currently used descriptor sets.
		@param maxSets The new maximum amount of sets the descriptor pool should use.
		*/
		void updateDescriptorPool(uint32_t maxSets);

		/*!
		@brief Getter for a newly created descriptor set, allocated from the descriptor pool.
		@return The newly allocated descriptor set.
		*/
		vk::DescriptorSet allocateDescriptorSet() const;

		/*!
		@brief Getter for the pipeline's layout.
		@return The pipeline's layout.
		*/
		vk::PipelineLayout pipelineLayout() const;

		/*!
		@brief Getter for the pipeline itself.
		@return The pipeline.
		*/
		vk::Pipeline graphicsPipeline() const;

		/*!
		@brief Getter for the pipeline's swapchain.
		@return The swapchain.
		*/
		vk::SwapchainKHR swapchain() const;

	private:
		/*!
		@brief Helper function to choose the surface format.
		@param physicalDevice The physical device to poll.
		@param surface The surface on which to render.
		@return The chosen surface format.
		*/
		static vk::SurfaceFormatKHR chooseSurfaceFormat(
			const vk::PhysicalDevice& physicalDevice, 
			const vk::SurfaceKHR& surface);

		/*!
		@brief Helper function to choose the present mode.
		@param physicalDevice The physical device to poll.
		@param surface The surface on which to render.
		@return The chosen present mode.
		*/
		static vk::PresentModeKHR choosePresentMode(
			const vk::PhysicalDevice& physicalDevice,
			const vk::SurfaceKHR& surface);

		/*!
		@brief Helper function to choose the extent.
		@param physicalDevice The physical device to poll.
		@param surface The surface on which to render.
		@param size The window's size.
		@return The chosen extent.
		*/
		static vk::Extent2D chooseExtent(
			const vk::PhysicalDevice& physicalDevice, 
			const vk::SurfaceKHR& surface,
			const glm::ivec2& size);

		/*!
		@brief Creates the swapchain. Optionally takes the old swapchain to possibly optimize creation.
		@param physicalDevice The physical device to poll.
		@param device The device used for allocations.
		@param surface The surface on which to render.
		@param surfaceFormat The format of the surface.
		@param swapExtent The extent of the swap volume.
		@param presentMode The present mode to be used by the swapchain.
		@param indices Queue family indices polled by the program.
		@param oldSwapchain The old swapchain, or empty to ignore.
		@return The newly created swapchain.
		*/
		static vk::SwapchainKHR createSwapchain(
			const vk::PhysicalDevice& physicalDevice,
			const vk::Device& device,
			const vk::SurfaceKHR& surface,
			const vk::SurfaceFormatKHR& surfaceFormat,
			const vk::Extent2D& swapExtent,
			const vk::PresentModeKHR& presentMode,
			const VulkanBase::QueueFamilyIndices& indices,
			vk::SwapchainKHR oldSwapchain = nullptr);

		/*!
		@brief Helper function to create the render pass.
		@param device The device used for allocations.
		@param surfaceFormat The format of the surface.
		@param depthImage The depth image.
		@return The created render pass.
		*/
		static vk::RenderPass createRenderPass(
			const vk::Device& device,
			const vk::SurfaceFormatKHR& surfaceFormat,
			const VulkanImage& depthImage);

		/*!
		@brief Helper function to create the pipeline layout.
		@param device The device used for allocations.
		@param descriptorSetLayout The layout of descriptor sets.
		@return The created pipeline layout.
		*/
		static vk::PipelineLayout createPipelineLayout(
			const vk::Device& device, 
			const vk::DescriptorSetLayout& descriptorSetLayout);

		/*!
		@brief Helper function to create the descriptor set layout.
		@param device The device used for allocations.
		@return The created descriptor set layout.
		*/
		static vk::DescriptorSetLayout createDescriptorSetLayout(const vk::Device& device);

		/*!
		@brief Helper function to create the descriptor pool.
		@param device The device used for allocations.
		@param maxSets The maximum amount of sets allocated from the pool.
		@return The created descriptor pool.
		*/
		static vk::DescriptorPool createDescriptorPool(const vk::Device& device, uint32_t maxSets);

		/*!
		@brief Helper function to create the descriptor set.
		@param device The device used for allocations.
		@param descriptorPool The descriptor pool to allocate descriptor sets.
		@param descriptorSetLayout The layout of descriptor sets.
		@return The created descriptor set.
		*/
		static vk::DescriptorSet createDescriptorSet(
			const vk::Device& device,
			const vk::DescriptorPool& descriptorPool,
			const vk::DescriptorSetLayout& descriptorSetLayout);

		/*!
		@brief Helper function to create the actual graphics pipeline. Takes an optional parameter of the
		old pipeline to hopefully optimize pipeline creation.
		@param device The device used for allocations.
		@param swapExtent The extent of the swap volume.
		@param pipelineLayout The pipeline layout.
		@param renderPass The renderpass used by the pipeline.
		@param oldPipeline The old pipeline, or empty if creating the first one.
		@return The created pipeline.
		*/
		static vk::Pipeline createGraphicsPipeline(
			const vk::Device& device,
			const vk::Extent2D& swapExtent,
			const vk::PipelineLayout& pipelineLayout,
			const vk::RenderPass& renderPass,
			vk::Pipeline oldPipeline = nullptr);

		/*!
		@brief Helper function to create the framebuffers.
		@param device The device used for allocations.
		@param swapchainImageViews The collection of image views for the swapchain images.
		@param depthImage The depth image.
		@param renderPass The renderpass used by the pipeline.
		@param swapExtent The extent of the swap volume.
		@return The created framebuffers.
		*/
		static std::vector<vk::Framebuffer> createFramebuffers(
			const vk::Device& device,
			const std::vector<vk::ImageView>& swapchainImageViews,
			const VulkanImage& depthImage,
			const vk::RenderPass& renderPass,
			const vk::Extent2D& swapExtent);

		/*! The renderer's base. */
		std::shared_ptr<const VulkanBase> _base;

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
		/*! The actual graphics pipeline. */
		vk::Pipeline _graphicsPipeline;

		/*! The image containing depth information. */
		VulkanImage _depthImage = nullptr;
	};
}

#endif //RENDER_VULKANGRAPHICSPIPELINE_H