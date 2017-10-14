/*! @file Render/VulkanUtils.h */

#ifndef RENDER_VULKANUTILS_H
#define RENDER_VULKANUTILS_H
#pragma once

#include <cstdint>
#include <vulkan/vulkan.hpp>

#include <vector>

namespace Orbit
{
	struct VulkanQueueFamilies
	{
		uint32_t graphicsQueueFamily = -1;
		uint32_t presentQueueFamily = -1;
		uint32_t transferQueueFamily = -1;
	};

	VulkanQueueFamilies getQueueFamilies(vk::PhysicalDevice device, vk::SurfaceKHR surface);

	vk::ImageView createImageView(vk::Device device, vk::Image image, vk::Format format);

	uint32_t getMemoryTypeIndex(vk::PhysicalDevice device, uint32_t filter, vk::MemoryPropertyFlags flags);

	void getLayoutParameters(vk::ImageLayout layout, vk::PipelineStageFlags& stage, vk::AccessFlags& accessFlags);
}

#endif //RENDER_VULKANUTILS_H