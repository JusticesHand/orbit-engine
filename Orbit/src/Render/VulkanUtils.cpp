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

		uint32_t familyIndex = static_cast<uint32_t>(i);

		if (queueFamilyProperty.queueFlags & vk::QueueFlagBits::eGraphics)
			families.graphicsQueueFamily = familyIndex;

		if (device.getSurfaceSupportKHR(familyIndex, surface))
			families.presentQueueFamily = familyIndex;

		if (queueFamilyProperty.queueFlags & vk::QueueFlagBits::eTransfer)
			families.transferQueueFamily = familyIndex;

		constexpr const uint32_t max = std::numeric_limits<uint32_t>::max();

		if (families.graphicsQueueFamily != max && families.presentQueueFamily != max && families.transferQueueFamily != max)
			break;
	}

	return families;
}

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

uint32_t Orbit::getMemoryTypeIndex(vk::PhysicalDevice device, uint32_t filter, vk::MemoryPropertyFlags flags)
{
	vk::PhysicalDeviceMemoryProperties memoryProperties = device.getMemoryProperties();

	for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; i++)
	{
		bool memorySuitable = static_cast<bool>(filter & (1 << i));
		bool hasCorrectProperties = (memoryProperties.memoryTypes[i].propertyFlags & flags) == flags;
		if (memorySuitable && hasCorrectProperties)
			return i;
	}

	return std::numeric_limits<uint32_t>::max();
}

void Orbit::getLayoutParameters(vk::ImageLayout layout, vk::PipelineStageFlags& stage, vk::AccessFlags& accessFlags)
{
	if (layout == vk::ImageLayout::eUndefined)
	{
		stage = vk::PipelineStageFlagBits::eTopOfPipe;
		accessFlags = vk::AccessFlags();
	}
	else if (layout == vk::ImageLayout::eTransferDstOptimal)
	{
		stage = vk::PipelineStageFlagBits::eTransfer;
		accessFlags = vk::AccessFlagBits::eTransferWrite;
	}
	else if (layout == vk::ImageLayout::eShaderReadOnlyOptimal)
	{
		stage = vk::PipelineStageFlagBits::eFragmentShader;
		accessFlags = vk::AccessFlagBits::eShaderRead;
	}
	else if (layout == vk::ImageLayout::eDepthStencilAttachmentOptimal)
	{
		stage = vk::PipelineStageFlagBits::eEarlyFragmentTests;
		accessFlags = vk::AccessFlagBits::eDepthStencilAttachmentRead | vk::AccessFlagBits::eDepthStencilAttachmentWrite;
	}
	else
	{
		throw std::runtime_error("Could not get layout parameters for a layout!");
	}
}