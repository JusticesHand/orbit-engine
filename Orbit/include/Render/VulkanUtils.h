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
	@brief Creates an image view from a device to an image with a format in parameter. Not wrapped in a class as
	it should be possible to create an ImageView from an already existing image.
	@param device The device to use to create the ImageView.
	@param image The image on which to bind the newly created ImageView.
	@param format The format to apply to the ImageView.
	@return The newly created ImageView.
	*/
	vk::ImageView createImageView(vk::Device device, vk::Image image, vk::Format format);
}

#endif //RENDER_VULKANUTILS_H