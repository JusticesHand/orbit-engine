/*! @file Render/VulkanUtils.h */

#ifndef RENDER_VULKANUTILS_H
#define RENDER_VULKANUTILS_H
#pragma once

#include <cstdint>
#include <vulkan/vulkan.hpp>

#include <vector>

namespace Orbit
{
	/*!
	@brief Structure containing information about Vulkan queue families.
	Values at std::numeric_limits<uint32_t>::max() are considered to be invalid.
	Values of queue families can be the same.
	*/
	struct VulkanQueueFamilies
	{
		/*! The index of the graphics queue family. */
		uint32_t graphicsQueueFamily = -1;
		/*! The index of the present queue family. */
		uint32_t presentQueueFamily = -1;
		/*! The index of the transfer queue family. */
		uint32_t transferQueueFamily = -1;
	};

	/*!
	@brief Retrieves the queue families appropriate for a surface from a physical device.
	@param device The physical device from which to retrieve queues.
	@param surface The surface on which rendering is intended.
	@return The structure containing queue family information.
	*/
	VulkanQueueFamilies getQueueFamilies(vk::PhysicalDevice device, vk::SurfaceKHR surface);

	/*!
	@brief Creates an image view from a device to an image with a format in parameter. Not wrapped in a class as
	it should be possible to create an ImageView from an already existing image.
	@param device The device to use to create the ImageView.
	@param image The image on which to bind the newly created ImageView.
	@param format The format to apply to the ImageView.
	@return The newly created ImageView.
	*/
	vk::ImageView createImageView(vk::Device device, vk::Image image, vk::Format format);

	/*!
	@brief Retrieves a memory type index from a filter, usually returned by querying memory requirements for buffers
	or images.
	@param device The physical device that will accept the memory.
	@param filter The filter containing the available memory types.
	@param flags The flags that the memory type should satisfy.
	@return The found index of the most appropriate memory type.
	*/
	uint32_t getMemoryTypeIndex(vk::PhysicalDevice device, uint32_t filter, vk::MemoryPropertyFlags flags);

	/*!
	@brief Retrieves PipelineStageFlags and AccessFlags from an ImageLayout. Useful when doing layout transitions
	as this information is required for both the old and new layouts.
	@param[in] layout The layout to poll.
	@param[out] stage The retrieved pipeline stage flags.
	@param[out] accessFlags The retrieved access flags.
	*/
	void getLayoutParameters(vk::ImageLayout layout, vk::PipelineStageFlags& stage, vk::AccessFlags& accessFlags);
}

#endif //RENDER_VULKANUTILS_H