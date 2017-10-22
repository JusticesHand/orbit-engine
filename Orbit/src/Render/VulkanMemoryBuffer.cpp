/*! @file Render/VulkanMemoryBuffer.cpp */

#include "Render/VulkanMemoryBuffer.h"

#include "Render/VulkanBase.h"

#include "Render/VulkanUtils.h"

#include <Util.h>

using namespace Orbit;

VulkanMemoryBuffer::VulkanMemoryBuffer(std::nullptr_t)
{
}

VulkanMemoryBuffer::VulkanMemoryBuffer(
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

VulkanMemoryBuffer::VulkanMemoryBuffer(VulkanMemoryBuffer&& rhs)
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

VulkanMemoryBuffer& VulkanMemoryBuffer::operator=(VulkanMemoryBuffer&& rhs)
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

VulkanMemoryBuffer::~VulkanMemoryBuffer()
{
	clear();
}

vk::CommandBuffer VulkanMemoryBuffer::transferToBuffer(VulkanMemoryBuffer& rhs, vk::DeviceSize dstOffset)
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

void VulkanMemoryBuffer::clear()
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

vk::Buffer VulkanMemoryBuffer::buffer() const
{
	return _buffer;
}

VulkanMemoryBuffer::Block& VulkanMemoryBuffer::getBlock(size_t index)
{
	return _blocks[index];
}

const VulkanMemoryBuffer::Block& VulkanMemoryBuffer::getBlock(size_t index) const
{
	return _blocks[index];
}

VulkanMemoryBuffer::Block& VulkanMemoryBuffer::operator[](size_t index)
{
	return getBlock(index);
}

const VulkanMemoryBuffer::Block& VulkanMemoryBuffer::operator[](size_t index) const
{
	return getBlock(index);
}

VulkanMemoryBuffer::Block::Block(
	std::shared_ptr<const VulkanBase> base,
	vk::DeviceMemory memory,
	vk::DeviceSize size,
	vk::DeviceSize offset)
	: _base(base), _memory(memory), _size(size), _offset(offset)
{
}

void VulkanMemoryBuffer::Block::copy(const void* data, vk::DeviceSize size)
{
	ASSERT_DEBUG(size == _size, "Tried to copy memory of mismatching sizes!");

	void* copyRegion = _base->device().mapMemory(_memory, _offset, size);
	memcpy(copyRegion, data, size);
	_base->device().unmapMemory(_memory);
}

vk::DeviceSize VulkanMemoryBuffer::Block::size() const
{
	return _size;
}

vk::DeviceSize VulkanMemoryBuffer::Block::offset() const
{
	return _offset;
}