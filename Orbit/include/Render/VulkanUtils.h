/*! @file Render/VulkanUtils.h */

#ifndef RENDER_VULKANUTILS_H
#define RENDER_VULKANUTILS_H
#pragma once

#include <vulkan/vulkan.hpp>

namespace Orbit
{
	struct VulkanQueueFamilies
	{
		int graphicsQueueFamily = -1;
		int presentQueueFamily = -1;
		int transferQueueFamily = -1;
	};

	VulkanQueueFamilies getQueueFamilies(vk::PhysicalDevice device, vk::SurfaceKHR surface);

	vk::ImageView createImageView(vk::Device device, vk::Image image, vk::Format format);
}

#endif //RENDER_VULKANUTILS_H