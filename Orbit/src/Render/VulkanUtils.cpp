/*! @file Render/VulkanUtils.cpp */

#include "Render/VulkanUtils.h"

using namespace Orbit;

VulkanQueueFamilies Orbit::getQueueFamilies(vk::PhysicalDevice device, vk::SurfaceKHR surface)
{
	VulkanQueueFamilies families;

	std::vector<vk::QueueFamilyProperties> queueFamilyProperties = device.getQueueFamilyProperties();
	for (size_t i = 0; i < queueFamilyProperties.size(); i++)
	{
		const vk::QueueFamilyProperties& queueFamilyProperty = queueFamilyProperties[i];

		if (queueFamilyProperty.queueCount == 0)
			continue;

		if (queueFamilyProperty.queueFlags & vk::QueueFlagBits::eGraphics)
			families.graphicsQueueFamily = i;

		if (device.getSurfaceSupportKHR(i, surface))
			families.presentQueueFamily = i;

		if (queueFamilyProperty.queueFlags & vk::QueueFlagBits::eTransfer)
			families.transferQueueFamily = i;

		if (families.graphicsQueueFamily != -1 && families.presentQueueFamily != -1 && families.transferQueueFamily != -1)
			break;
	}

	return families;
}

vk::ImageView Orbit::createImageView(vk::Device device, vk::Image image, vk::Format format)
{
	vk::ComponentMapping componentMapping;
	componentMapping
		.setR(vk::ComponentSwizzle::eIdentity)
		.setG(vk::ComponentSwizzle::eIdentity)
		.setB(vk::ComponentSwizzle::eIdentity)
		.setA(vk::ComponentSwizzle::eIdentity);

	vk::ImageSubresourceRange subresourceRange;
	subresourceRange
		.setAspectMask(vk::ImageAspectFlagBits::eColor)
		.setBaseMipLevel(0)
		.setLevelCount(1)
		.setBaseArrayLayer(0)
		.setLayerCount(1);

	vk::ImageViewCreateInfo createInfo;
	createInfo
		.setImage(image)
		.setViewType(vk::ImageViewType::e2D)
		.setFormat(format)
		.setComponents(componentMapping)
		.setSubresourceRange(subresourceRange);

	return device.createImageView(createInfo);
}