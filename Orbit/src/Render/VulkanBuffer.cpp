/*! @file Render/VulkanMemoryBuffer.cpp */

#include "Render/VulkanBuffer.h"
#include "Render/VulkanImage.h"

#include "Render/VulkanBase.h"

#include "Render/VulkanUtils.h"

#include <Util.h>

using namespace Orbit;

VulkanBuffer::VulkanBuffer(std::nullptr_t)
{
}

VulkanBuffer::VulkanBuffer(
	std::shared_ptr<const VulkanBase> base,
	const std::vector<vk::DeviceSize>& blockSizes,
	vk::BufferCreateInfo createInfo,
	vk::MemoryPropertyFlags memFlags)
	: _base(base)
{
	for (const vk::DeviceSize& size : blockSizes)
		_totalSize += size;

	if (_totalSize == 0)
		return;

	createInfo.setSize(_totalSize);

	_buffer = _base->device().createBuffer(createInfo);

	vk::MemoryRequirements requirements = _base->device().getBufferMemoryRequirements(_buffer);
	uint32_t memoryTypeIndex = _base->getMemoryTypeIndex(requirements.memoryTypeBits, memFlags);

	vk::MemoryAllocateInfo memAllocInfo = vk::MemoryAllocateInfo()
		.setAllocationSize(requirements.size)
		.setMemoryTypeIndex(memoryTypeIndex);
	
	_memory = _base->device().allocateMemory(memAllocInfo);

	_base->device().bindBufferMemory(_buffer, _memory, 0);

	vk::DeviceSize offset = 0;
	_blocks.reserve(blockSizes.size());
	for (const vk::DeviceSize& size : blockSizes)
	{
		_blocks.push_back(Block(_base, _memory, size, offset));
		offset += size;
	}
}

VulkanBuffer::VulkanBuffer(VulkanBuffer&& rhs)
	: _base(rhs._base),
	_buffer(rhs._buffer),
	_memory(rhs._memory), 
	_totalSize(rhs._totalSize),
	_blocks(std::move(rhs._blocks))
{
	rhs._base = nullptr;

	rhs._buffer = nullptr;
	rhs._memory = nullptr;
}

VulkanBuffer& VulkanBuffer::operator=(VulkanBuffer&& rhs)
{
	_base = rhs._base;
	_buffer = rhs._buffer;
	_memory = rhs._memory;
	_totalSize = rhs._totalSize;
	_blocks = std::move(rhs._blocks);

	rhs._base = nullptr;

	rhs._buffer = nullptr;
	rhs._memory = nullptr;

	return *this;
}

VulkanBuffer::~VulkanBuffer()
{
	clear();
}

vk::CommandBuffer VulkanBuffer::transferToBuffer(VulkanBuffer& rhs, vk::DeviceSize dstOffset)
{
	if (dstOffset + _totalSize > rhs._totalSize)
		throw std::runtime_error("Attempted to transfer buffers that do not match!");

	vk::CommandBufferAllocateInfo allocInfo = vk::CommandBufferAllocateInfo()
		.setCommandPool(_base->transferCommandPool())
		.setLevel(vk::CommandBufferLevel::ePrimary)
		.setCommandBufferCount(1);
	
	vk::CommandBuffer cmdBuffer = _base->device().allocateCommandBuffers(allocInfo)[0];

	cmdBuffer.begin(vk::CommandBufferBeginInfo()
		.setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit));

	vk::BufferCopy copyRegion = vk::BufferCopy()
		.setSrcOffset(0)
		.setDstOffset(dstOffset)
		.setSize(_totalSize);

	cmdBuffer.copyBuffer(_buffer, rhs._buffer, copyRegion);

	cmdBuffer.end();

	return cmdBuffer;
}

vk::CommandBuffer VulkanBuffer::transferToImage(VulkanImage& rhs)
{
	if (_totalSize == 0)
		return nullptr;

	if (_totalSize != rhs.totalSize())
		throw std::runtime_error("Attempted to transfer buffer to an image that cannot contain it!");

	vk::CommandBufferAllocateInfo allocInfo = vk::CommandBufferAllocateInfo()
		.setCommandPool(_base->transferCommandPool())
		.setLevel(vk::CommandBufferLevel::ePrimary)
		.setCommandBufferCount(1);

	vk::CommandBuffer cmdBuffer = _base->device().allocateCommandBuffers(allocInfo)[0];

	cmdBuffer.begin(vk::CommandBufferBeginInfo()
		.setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit));

	std::vector<vk::CommandBuffer> copyCommands;
	copyCommands.reserve(rhs.imageCount());

	vk::DeviceSize bufferOffset = 0;
	for (size_t i = 0; i < rhs.imageCount(); i++)
	{
		VulkanImage::Block& block = rhs[i];
		copyCommands.push_back(rhs[i].copy(*this, bufferOffset, true));
		bufferOffset += block.size();

		//vk::BufferImageCopy copyRegion = vk::BufferImageCopy()
		//	.setBufferOffset(bufferOffset)
		//	.setBufferRowLength(0)
		//	.setImageSubresource(vk::ImageSubresourceLayers()
		//		.setAspectMask(vk::ImageAspectFlagBits::eColor)
		//		.setMipLevel(0)
		//		.setBaseArrayLayer(0)
		//		.setLayerCount(1));

		

		//cmdBuffer.copyBufferToImage(_buffer, block.image(), block.layout(), copyRegion);
	}
	
	cmdBuffer.executeCommands(copyCommands);

	cmdBuffer.end();

	return cmdBuffer;
}

void VulkanBuffer::clear()
{
	if (!_base)
		return;

	_base->device().destroyBuffer(_buffer);
	_base->device().freeMemory(_memory);

	_buffer = nullptr;
	_memory = nullptr;

	_totalSize = 0;
	_blocks.clear();
}

vk::Buffer VulkanBuffer::buffer() const
{
	return _buffer;
}

VulkanBuffer::Block& VulkanBuffer::getBlock(size_t index)
{
	return _blocks[index];
}

const VulkanBuffer::Block& VulkanBuffer::getBlock(size_t index) const
{
	return _blocks[index];
}

VulkanBuffer::Block& VulkanBuffer::operator[](size_t index)
{
	return getBlock(index);
}

const VulkanBuffer::Block& VulkanBuffer::operator[](size_t index) const
{
	return getBlock(index);
}

VulkanBuffer::Block::Block(
	std::shared_ptr<const VulkanBase> base,
	vk::DeviceMemory memory,
	vk::DeviceSize size,
	vk::DeviceSize offset)
	: _base(base), _memory(memory), _size(size), _offset(offset)
{
}

void VulkanBuffer::Block::copy(const void* data, vk::DeviceSize size)
{
	ASSERT_DEBUG(size == _size, "Tried to copy memory of mismatching sizes!");

	void* copyRegion = _base->device().mapMemory(_memory, _offset, size);
	memcpy(copyRegion, data, size);
	_base->device().unmapMemory(_memory);
}

vk::DeviceSize VulkanBuffer::Block::size() const
{
	return _size;
}

vk::DeviceSize VulkanBuffer::Block::offset() const
{
	return _offset;
}