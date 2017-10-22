/*! @file Render/VulkanUtils.cpp */

#include "Render/VulkanUtils.h"

using namespace Orbit;

vk::ImageView Orbit::createImageView(vk::Device device, vk::Image image, vk::Format format)
{
	vk::ImageSubresourceRange subresourceRange;
	subresourceRange
		.setAspectMask(vk::ImageAspectFlagBits::eColor)
		.setBaseMipLevel(0)
		.setLevelCount(1)
		.setBaseArrayLayer(0)
		.setLayerCount(1);

	if (format == vk::Format::eD32Sfloat)
		subresourceRange.setAspectMask(vk::ImageAspectFlagBits::eDepth);

	vk::ImageViewCreateInfo createInfo;
	createInfo
		.setImage(image)
		.setViewType(vk::ImageViewType::e2D)
		.setFormat(format)
		.setSubresourceRange(subresourceRange);

	return device.createImageView(createInfo);
}