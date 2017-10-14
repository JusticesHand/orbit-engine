/*! @file Render/VulkanImage.cpp */

#include "Render/VulkanImage.h"

#include "Render/VulkanUtils.h"

using namespace Orbit;

VulkanImage::VulkanImage(
	const vk::PhysicalDevice& physicalDevice,
	const vk::Device& device,
	vk::ImageCreateInfo imageCreateInfo,
	vk::MemoryPropertyFlags memFlags)
	: _device(device), _format(imageCreateInfo.format)
{
	_image = _device.createImage(imageCreateInfo);

	vk::MemoryRequirements requirements = _device.getImageMemoryRequirements(_image);
	uint32_t memoryTypeIndex = getMemoryTypeIndex(physicalDevice, requirements.memoryTypeBits, memFlags);

	vk::MemoryAllocateInfo allocInfo{ requirements.size, memoryTypeIndex };
	_memory = _device.allocateMemory(allocInfo);

	_device.bindImageMemory(_image, _memory, 0);

	_imageView = createImageView(_device, _image, _format);
}

VulkanImage::~VulkanImage()
{
	clear();
}

VulkanImage::VulkanImage(VulkanImage&& rhs)
	: _device(rhs._device),
	_image(rhs._image),
	_imageView(rhs._imageView),
	_memory(rhs._memory),
	_format(rhs._format),
	_layout(rhs._layout)
{
	rhs._image = vk::Image();
	rhs._imageView = vk::ImageView();
	rhs._memory = vk::DeviceMemory();
}

VulkanImage& VulkanImage::operator=(VulkanImage&& rhs)
{
	_device = rhs._device;
	_image = rhs._image;
	_imageView = rhs._imageView;
	_memory = rhs._memory;
	_format = rhs._format;
	_layout = rhs._layout;

	rhs._image = vk::Image();
	rhs._imageView = vk::ImageView();
	rhs._memory = vk::DeviceMemory();

	return *this;
}

vk::Format VulkanImage::format() const
{
	return _format;
}

vk::Image VulkanImage::image() const
{
	return _image;
}

vk::ImageView VulkanImage::imageView() const
{
	return _imageView;
}

void VulkanImage::clear()
{
	if (_imageView)
		_device.destroyImageView(_imageView);
	if (_image)
		_device.destroyImage(_image);
	if (_memory)
		_device.freeMemory(_memory);

	_imageView = vk::ImageView();
	_image = vk::Image();
	_memory = vk::DeviceMemory();
}

vk::CommandBuffer VulkanImage::transitionLayout(vk::ImageLayout newLayout, vk::CommandPool transferPool)
{
	vk::ImageSubresourceRange subresourceRange;
	subresourceRange
		.setAspectMask(vk::ImageAspectFlagBits::eColor)
		.setBaseMipLevel(0)
		.setLevelCount(1)
		.setBaseArrayLayer(0)
		.setLayerCount(1);

	if (newLayout == vk::ImageLayout::eDepthStencilAttachmentOptimal)
	{
		subresourceRange.setAspectMask(vk::ImageAspectFlagBits::eDepth);
		if (_format == vk::Format::eD32SfloatS8Uint || _format == vk::Format::eD24UnormS8Uint)
			subresourceRange.aspectMask |= vk::ImageAspectFlagBits::eStencil;
	}

	vk::PipelineStageFlags srcStage, dstStage;
	vk::AccessFlags srcAccessFlags, dstAccessFlags;
	getLayoutParameters(_layout, srcStage, srcAccessFlags);
	getLayoutParameters(newLayout, dstStage, dstAccessFlags);

	vk::ImageMemoryBarrier barrier;
	barrier
		.setOldLayout(_layout)
		.setNewLayout(newLayout)
		.setSrcQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
		.setDstQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
		.setImage(_image)
		.setSubresourceRange(subresourceRange)
		.setSrcAccessMask(srcAccessFlags)
		.setDstAccessMask(dstAccessFlags);

	vk::CommandBufferAllocateInfo allocInfo{ transferPool, vk::CommandBufferLevel::ePrimary, 1 };
	vk::CommandBuffer commandBuffer = _device.allocateCommandBuffers(allocInfo)[0];

	commandBuffer.begin({ vk::CommandBufferUsageFlagBits::eOneTimeSubmit });

	commandBuffer.pipelineBarrier(srcStage, dstStage, vk::DependencyFlags(), nullptr, nullptr, barrier);

	commandBuffer.end();

	return commandBuffer;
}