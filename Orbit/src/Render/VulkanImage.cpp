/*! @file Render/VulkanImage.cpp */

#include "Render/VulkanImage.h"

#include "Render/VulkanBase.h"

#include "Render/VulkanUtils.h"

using namespace Orbit;

VulkanImage::VulkanImage(std::nullptr_t)
{
}

VulkanImage::VulkanImage(
	std::shared_ptr<const VulkanBase> base,
	vk::ImageCreateInfo imageCreateInfo,
	vk::MemoryPropertyFlags memFlags)
	: _base(base), _format(imageCreateInfo.format)
{
	_image = _base->device().createImage(imageCreateInfo);

	vk::MemoryRequirements requirements = _base->device().getImageMemoryRequirements(_image);
	uint32_t memoryTypeIndex = _base->getMemoryTypeIndex(requirements.memoryTypeBits, memFlags);

	vk::MemoryAllocateInfo allocInfo{ requirements.size, memoryTypeIndex };
	_memory = _base->device().allocateMemory(allocInfo);

	_base->device().bindImageMemory(_image, _memory, 0);

	_imageView = createImageView(_base->device(), _image, _format);
}

VulkanImage::~VulkanImage()
{
	clear();
}

VulkanImage::VulkanImage(VulkanImage&& rhs)
	: _base(rhs._base),
	_image(rhs._image),
	_imageView(rhs._imageView),
	_memory(rhs._memory),
	_format(rhs._format),
	_layout(rhs._layout)
{
	rhs._base = nullptr;
	rhs._image = nullptr;
	rhs._imageView = nullptr;
	rhs._memory = nullptr;
}

VulkanImage& VulkanImage::operator=(VulkanImage&& rhs)
{
	_base = rhs._base;;
	_image = rhs._image;
	_imageView = rhs._imageView;
	_memory = rhs._memory;
	_format = rhs._format;
	_layout = rhs._layout;

	rhs._base = nullptr;
	rhs._image = nullptr;
	rhs._imageView = nullptr;
	rhs._memory = nullptr;

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
	if (!_base)
		return;

	_base->device().destroyImageView(_imageView);
	_base->device().destroyImage(_image);
	_base->device().freeMemory(_memory);

	_imageView = nullptr;
	_image = nullptr;
	_memory = nullptr;
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
	_base->getLayoutParameters(_layout, srcStage, srcAccessFlags);
	_base->getLayoutParameters(newLayout, dstStage, dstAccessFlags);

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
	vk::CommandBuffer commandBuffer = _base->device().allocateCommandBuffers(allocInfo)[0];

	commandBuffer.begin({ vk::CommandBufferUsageFlagBits::eOneTimeSubmit });

	commandBuffer.pipelineBarrier(srcStage, dstStage, vk::DependencyFlags(), nullptr, nullptr, barrier);

	commandBuffer.end();

	return commandBuffer;
}