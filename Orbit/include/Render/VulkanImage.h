/*! @file Render/VulkanImage.h */

#ifndef RENDER_VULKANIMAGE_H
#define RENDER_VULKANIMAGE_H
#pragma once

#include <vulkan/vulkan.hpp>

namespace Orbit
{
	class VulkanImage final
	{
	public:
		VulkanImage() = default;
		explicit VulkanImage(
			const vk::PhysicalDevice& physicalDevice,
			const vk::Device& device,
			vk::ImageCreateInfo imageCreateInfo,
			vk::MemoryPropertyFlags memFlags);
		~VulkanImage();

		VulkanImage(const VulkanImage&) = delete;
		VulkanImage& operator=(const VulkanImage&) = delete;

		VulkanImage(VulkanImage&& rhs);
		VulkanImage& operator=(VulkanImage&& rhs);

		vk::Format format() const;

		vk::Image image() const;
		vk::ImageView imageView() const;

		void clear();

		vk::CommandBuffer transitionLayout(vk::ImageLayout newLayout, vk::CommandPool transferPool);

	private:
		vk::Device _device;

		vk::Image _image;
		vk::ImageView _imageView;
		vk::DeviceMemory _memory;

		vk::Format _format;
		vk::ImageLayout _layout = vk::ImageLayout::eUndefined;
	};
}

#endif //RENDER_VULKANIMAGE_H