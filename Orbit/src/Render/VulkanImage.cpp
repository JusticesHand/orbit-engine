/*! @file Render/VulkanImage.cpp */

#include "Render/VulkanImage.h"
#include "Render/VulkanBuffer.h"

#include "Render/VulkanBase.h"

#include "Render/VulkanUtils.h"

#include <Util.h>

using namespace Orbit;

VulkanImage::VulkanImage(std::nullptr_t)
{
}

VulkanImage::VulkanImage(
	std::shared_ptr<const VulkanBase> base,
	const std::vector<vk::Extent2D>& imageSizes,
	vk::ImageCreateInfo imageCreateInfo,
	vk::MemoryPropertyFlags memFlags)
	: _base(base)
{
	_blocks.reserve(imageSizes.size());

	for (const vk::Extent2D& imageSize : imageSizes)
	{
		_blocks.push_back(Block{
			base,
			imageSize,
			imageCreateInfo
		});
	}

	vk::DeviceSize memorySize = 0;
	std::vector<vk::DeviceSize> memoryOffsets;
	memoryOffsets.reserve(_blocks.size());
	uint32_t lastBits = std::numeric_limits<uint32_t>::max();
	uint32_t memoryTypeIndex = std::numeric_limits<uint32_t>::max();

	for (const Block& block : _blocks)
	{
		vk::MemoryRequirements requirements = block.memoryRequirements();
		
		memoryOffsets.push_back(memorySize);
		memorySize += requirements.size;

		if (memoryTypeIndex == std::numeric_limits<uint32_t>::max())
		{
			lastBits = requirements.memoryTypeBits;
			memoryTypeIndex = _base->getMemoryTypeIndex(requirements.memoryTypeBits, memFlags);
		}

		ASSERT_DEBUG(lastBits == requirements.memoryTypeBits,
			"Got a weird error where required memory type bits are different for the same image type!");
	}

	vk::MemoryAllocateInfo allocInfo = vk::MemoryAllocateInfo()
		.setAllocationSize(memorySize)
		.setMemoryTypeIndex(memoryTypeIndex);

	_memory = _base->device().allocateMemory(allocInfo);

	for (size_t i = 0; i < memoryOffsets.size(); i++)
		_blocks[i].bindMemory(_memory, memoryOffsets[i]);
}

VulkanImage::~VulkanImage()
{
	clear();
}

VulkanImage::VulkanImage(VulkanImage&& rhs)
	: _base(rhs._base),
	_blocks(std::move(rhs._blocks)),
	_memory(rhs._memory)
{
	rhs._base = nullptr;
	rhs._memory = nullptr;
}

VulkanImage& VulkanImage::operator=(VulkanImage&& rhs)
{
	_base = rhs._base;;
	_blocks = std::move(rhs._blocks);
	_memory = rhs._memory;

	rhs._base = nullptr;
	rhs._memory = nullptr;

	return *this;
}

void VulkanImage::clear()
{
	if (!_base)
		return;

	_blocks.clear();

	_base->device().freeMemory(_memory);

	_memory = nullptr;
}

vk::DeviceSize VulkanImage::totalSize() const
{
	vk::DeviceSize size = 0;
	for (const Block& block : _blocks)
		size += block.size();

	return size;
}

size_t VulkanImage::imageCount() const
{
	return _blocks.size();
}

vk::CommandBuffer VulkanImage::transitionLayouts(vk::ImageLayout newLayout)
{
	std::vector<vk::CommandBuffer> secondaryBuffers;
	secondaryBuffers.reserve(_blocks.size());

	for (Block& block : _blocks)
		secondaryBuffers.push_back(block.transitionLayout(newLayout, true));

	vk::CommandBufferAllocateInfo allocInfo = vk::CommandBufferAllocateInfo()
		.setCommandBufferCount(1)
		.setCommandPool(_base->transferCommandPool())
		.setLevel(vk::CommandBufferLevel::ePrimary);

	vk::CommandBuffer cmdBuffer = _base->device().allocateCommandBuffers(allocInfo)[0];

	cmdBuffer.begin(vk::CommandBufferBeginInfo()
		.setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit));

	cmdBuffer.executeCommands(secondaryBuffers);

	cmdBuffer.end();

	return cmdBuffer;
}

VulkanImage::Block& VulkanImage::getBlock(size_t i)
{
	return _blocks[i];
}

const VulkanImage::Block& VulkanImage::getBlock(size_t i) const
{
	return _blocks[i];
}

VulkanImage::Block& VulkanImage::operator[](size_t i)
{
	return getBlock(i);
}

const VulkanImage::Block& VulkanImage::operator[](size_t i) const
{
	return getBlock(i);
}

vk::DeviceSize VulkanImage::Block::size() const
{
	return _extent.width * _extent.height * 4 * sizeof(uint8_t);
}

vk::Extent2D VulkanImage::Block::extent() const
{
	return _extent;
}

vk::ImageLayout VulkanImage::Block::layout() const
{
	return _layout;
}

vk::Format VulkanImage::Block::format() const
{
	return _format;
}

vk::Image VulkanImage::Block::image() const
{
	return _image;
}

vk::ImageView VulkanImage::Block::imageView() const
{
	return _imageView;
}

vk::Sampler VulkanImage::Block::sampler() const
{
	return _sampler;
}

vk::CommandBuffer VulkanImage::Block::transitionLayout(vk::ImageLayout newLayout, bool secondary)
{
	vk::ImageSubresourceRange subresourceRange = vk::ImageSubresourceRange()
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

	vk::ImageMemoryBarrier barrier = vk::ImageMemoryBarrier()
		.setOldLayout(_layout)
		.setNewLayout(newLayout)
		.setSrcQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
		.setDstQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
		.setImage(_image)
		.setSubresourceRange(subresourceRange)
		.setSrcAccessMask(srcAccessFlags)
		.setDstAccessMask(dstAccessFlags);

	vk::CommandBufferAllocateInfo allocInfo = vk::CommandBufferAllocateInfo()
		.setCommandBufferCount(1)
		.setCommandPool(_base->transferCommandPool())
		.setLevel(secondary ? vk::CommandBufferLevel::eSecondary : vk::CommandBufferLevel::ePrimary);

	vk::CommandBufferInheritanceInfo inheritanceInfo;
	vk::CommandBufferInheritanceInfo* pInheritanceInfo = nullptr;
	if (secondary)
	{
		allocInfo.setLevel(vk::CommandBufferLevel::eSecondary);
		pInheritanceInfo = &inheritanceInfo;
	}

	vk::CommandBuffer commandBuffer = _base->device().allocateCommandBuffers(allocInfo)[0];

	commandBuffer.begin(vk::CommandBufferBeginInfo()
		.setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit)
		.setPInheritanceInfo(pInheritanceInfo));

	commandBuffer.pipelineBarrier(srcStage, dstStage, vk::DependencyFlags(), nullptr, nullptr, barrier);

	commandBuffer.end();

	return commandBuffer;
}

vk::CommandBuffer VulkanImage::Block::copy(const VulkanBuffer& buffer, vk::DeviceSize bufferOffset, bool secondary)
{
	// Necessary to make two subresource ranges, as source could be depth/stencil image type
	vk::ImageSubresourceRange transferSubresourceRange = vk::ImageSubresourceRange()
		.setAspectMask(vk::ImageAspectFlagBits::eColor)
		.setBaseMipLevel(0)
		.setLevelCount(1)
		.setBaseArrayLayer(0)
		.setLayerCount(1);

	vk::PipelineStageFlags stageMask, transferStageMask;
	vk::AccessFlags accessFlags, transferAccessFlags;
	_base->getLayoutParameters(_layout, stageMask, accessFlags);
	_base->getLayoutParameters(vk::ImageLayout::eTransferDstOptimal, transferStageMask, transferAccessFlags);

	vk::ImageMemoryBarrier transferBarrier = vk::ImageMemoryBarrier()
		.setOldLayout(_layout)
		.setNewLayout(vk::ImageLayout::eTransferDstOptimal)
		.setSrcQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
		.setDstQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
		.setImage(_image)
		.setSubresourceRange(transferSubresourceRange)
		.setSrcAccessMask(accessFlags)
		.setDstAccessMask(transferAccessFlags);

	vk::BufferImageCopy copyRegion = vk::BufferImageCopy()
		.setBufferOffset(bufferOffset)
		.setBufferRowLength(0)
		.setImageOffset(vk::Offset3D{ 0, 0, 0 })
		.setImageExtent(vk::Extent3D{ _extent.width, _extent.height, 1 })
		.setImageSubresource(vk::ImageSubresourceLayers()
			.setAspectMask(vk::ImageAspectFlagBits::eColor)
			.setMipLevel(0)
			.setBaseArrayLayer(0)
			.setLayerCount(1));

	vk::CommandBufferAllocateInfo allocInfo = vk::CommandBufferAllocateInfo()
		.setCommandBufferCount(1)
		.setCommandPool(_base->transferCommandPool())
		.setLevel(vk::CommandBufferLevel::ePrimary);

	vk::CommandBufferInheritanceInfo inheritanceInfo;
	vk::CommandBufferInheritanceInfo* pInheritanceInfo = nullptr;
	if (secondary)
	{
		allocInfo.setLevel(vk::CommandBufferLevel::eSecondary);
		pInheritanceInfo = &inheritanceInfo;
	}

	vk::CommandBuffer commandBuffer = _base->device().allocateCommandBuffers(allocInfo)[0];

	commandBuffer.begin(vk::CommandBufferBeginInfo()
		.setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit)
		.setPInheritanceInfo(pInheritanceInfo));

	commandBuffer.pipelineBarrier(stageMask, transferStageMask, vk::DependencyFlags(), nullptr, nullptr, transferBarrier);

	commandBuffer.copyBufferToImage(buffer.buffer(), _image, vk::ImageLayout::eTransferDstOptimal, copyRegion);

	commandBuffer.end();

	return commandBuffer;
}

VulkanImage::Block::Block(std::shared_ptr<const VulkanBase> base, const vk::Extent2D& extent, vk::ImageCreateInfo createInfo)
	: _base(base), _extent(extent), _format(createInfo.format)
{
	createInfo.setExtent(vk::Extent3D{ extent.width, extent.height, 1 });

	_image = _base->device().createImage(createInfo);

	if (_format != vk::Format::eD32SfloatS8Uint && _format != vk::Format::eD24UnormS8Uint)
	{
		// TODO: mipmapping, essentially
		vk::SamplerCreateInfo createInfo = vk::SamplerCreateInfo()
			.setMagFilter(vk::Filter::eLinear)
			.setMinFilter(vk::Filter::eLinear)
			.setAddressModeU(vk::SamplerAddressMode::eClampToEdge)
			.setAddressModeV(vk::SamplerAddressMode::eClampToEdge)
			.setAddressModeW(vk::SamplerAddressMode::eClampToEdge)
			.setAnisotropyEnable(VK_TRUE)
			.setMaxAnisotropy(16)
			.setBorderColor(vk::BorderColor::eIntOpaqueBlack)
			.setUnnormalizedCoordinates(VK_FALSE)
			.setCompareEnable(VK_FALSE)
			.setCompareOp(vk::CompareOp::eAlways)
			.setMipmapMode(vk::SamplerMipmapMode::eLinear)
			.setMipLodBias(0.f)
			.setMinLod(0.f)
			.setMaxLod(0.f);

		_sampler = _base->device().createSampler(createInfo);
	}
}

VulkanImage::Block::~Block()
{
	if (!_base)
		return;

	_base->device().destroyImage(_image);
	_base->device().destroyImageView(_imageView);
	if (_sampler)
		_base->device().destroySampler(_sampler);
}

VulkanImage::Block::Block(Block&& rhs)
	: _base(rhs._base),
	_image(rhs._image),
	_imageView(rhs._imageView),
	_sampler(rhs._sampler),
	_extent(rhs._extent),
	_format(rhs._format),
	_layout(rhs._layout)
{
	rhs._base = nullptr;
	rhs._image = nullptr;
	rhs._imageView = nullptr;
	rhs._sampler = nullptr;
	rhs._extent = vk::Extent2D();
	rhs._format = vk::Format();
	rhs._layout = vk::ImageLayout::eUndefined;
}

VulkanImage::Block& VulkanImage::Block::operator=(Block&& rhs)
{
	_base = rhs._base;
	_image = rhs._image;
	_imageView = rhs._imageView;
	_sampler = rhs._sampler;
	_extent = rhs._extent;
	_format = rhs._format;
	_layout = rhs._layout;

	rhs._base = nullptr;
	rhs._image = nullptr;
	rhs._imageView = nullptr;
	rhs._sampler = nullptr;
	rhs._extent = vk::Extent2D();
	rhs._format = vk::Format();
	rhs._layout = vk::ImageLayout::eUndefined;

	return *this;
}

void VulkanImage::Block::bindMemory(const vk::DeviceMemory& memory, vk::DeviceSize offset)
{
	_base->device().bindImageMemory(_image, memory, offset);

	_imageView = createImageView(_base->device(), _image, _format);
}

vk::MemoryRequirements VulkanImage::Block::memoryRequirements() const
{
	return _base->device().getImageMemoryRequirements(_image);
}