/*! @file Render/VulkanImage.h */

#ifndef RENDER_VULKANIMAGE_H
#define RENDER_VULKANIMAGE_H
#pragma once

#include <memory>

#include <vulkan/vulkan.hpp>

namespace Orbit
{
	class VulkanBase;

	/*!
	@brief Wrapper class containing a vk::Image and its associated memory.
	As of right now, it is a one-to-one association - as in, each image object has its own memory.
	An image view object is created along the image.
	*/
	class VulkanImage final
	{
	public:
		/*!
		@brief Default constructor for the class. Leaves everything in a non-initialized (and unusable) state.
		*/
		VulkanImage(std::nullptr_t);

		/*!
		@brief Initializing constructor for the class. Creates and allocates resources with the objects in
		param.
		@param base The renderer's base.
		@param imageCreateInfo Create info struct for the image.
		@param memFlags The memory requirement flags for the image.
		*/
		explicit VulkanImage(
			std::shared_ptr<const VulkanBase> base,
			vk::ImageCreateInfo imageCreateInfo,
			vk::MemoryPropertyFlags memFlags);

		VulkanImage(const VulkanImage&) = delete;
		VulkanImage& operator=(const VulkanImage&) = delete;

		/*!
		@brief Move constructor for the class. Moves the created resources from rhs to this.
		@param rhs The image to take.
		*/
		VulkanImage(VulkanImage&& rhs);

		/*!
		@brief Move assignment operator for the class. Moves the created resources from rhs to this.
		@param rhs The image to take.
		@return A reference to this.
		*/
		VulkanImage& operator=(VulkanImage&& rhs);

		/*!
		@brief Destructor for the class. Frees all created resources.
		*/
		~VulkanImage();

		/*!
		@brief Getter for the image's format.
		@return The image's format.
		*/
		vk::Format format() const;

		/*!
		@brief Getter for the image object.
		@return The image object.
		*/
		vk::Image image() const;

		/*!
		@brief Getter for the image view.
		@return The image view.
		*/
		vk::ImageView imageView() const;

		/*!
		@brief Frees up created/allocated resources. Essentially resets to a default state.
		*/
		void clear();

		/*!
		@brief Builds and records a command buffer that executes a layout transition.
		@param newLayout The new layout of the image.
		@param transferPool The command pool from which commands are built.
		@return The recorded command buffer, ready to submit.
		*/
		vk::CommandBuffer transitionLayout(vk::ImageLayout newLayout, vk::CommandPool transferPool);

	private:
		/*! The renderer's base. */
		std::shared_ptr<const VulkanBase> _base;

		/*! The image object itself. */
		vk::Image _image;
		/*! The image view object. */
		vk::ImageView _imageView;
		/*! The underlying memory. */
		vk::DeviceMemory _memory;

		/*! The image's current format. */
		vk::Format _format;
		/*! The image's current layout. Initially always vk::ImageLayout::eUndefined.*/
		vk::ImageLayout _layout = vk::ImageLayout::eUndefined;
	};
}

#endif //RENDER_VULKANIMAGE_H