/*! @file Render/VulkanMemoryBuffer.cpp */

#include "Render/VulkanMemoryBuffer.h"

#include "Render/VulkanUtils.h"

using namespace Orbit;

VulkanMemoryBuffer::VulkanMemoryBuffer(
	const vk::PhysicalDevice& physicalDevice,
	const vk::Device& device,
	const std::vector<vk::DeviceSize>& blockSizes,
	vk::BufferCreateInfo createInfo,
	vk::MemoryPropertyFlags memFlags)
	: _device(device)
{
	for (const vk::DeviceSize& size : blockSizes)
		_totalSize += size;

	if (_totalSize == 0)
		return;

	createInfo.setSize(_totalSize);

	_buffer = _device.createBuffer(createInfo);

	vk::MemoryRequirements requirements = _device.getBufferMemoryRequirements(_buffer);
	uint32_t memoryTypeIndex = getMemoryTypeIndex(physicalDevice, requirements.memoryTypeBits, memFlags);

	vk::MemoryAllocateInfo memAllocInfo{ requirements.size, memoryTypeIndex };
	_memory = _device.allocateMemory(memAllocInfo);

	_device.bindBufferMemory(_buffer, _memory, 0);

	vk::DeviceSize offset = 0;
	_blocks.reserve(blockSizes.size());
	for (const vk::DeviceSize& size : blockSizes)
	{
		_blocks.push_back(Block(device, _memory, size, offset));
		offset += size;
	}
}

VulkanMemoryBuffer::VulkanMemoryBuffer(VulkanMemoryBuffer&& rhs)
	: _device(rhs._device),
	_buffer(rhs._buffer), 
	_memory(rhs._memory), 
	_totalSize(rhs._totalSize),
	_blocks(std::move(rhs._blocks))
{
	rhs._buffer = vk::Buffer();
	rhs._memory = vk::DeviceMemory();
}

VulkanMemoryBuffer& VulkanMemoryBuffer::operator=(VulkanMemoryBuffer&& rhs)
{
	_device = rhs._device;
	_buffer = rhs._buffer;
	_memory = rhs._memory;
	_totalSize = rhs._totalSize;
	_blocks = std::move(rhs._blocks);

	rhs._buffer = vk::Buffer();
	rhs._memory = vk::DeviceMemory();

	return *this;
}

VulkanMemoryBuffer::~VulkanMemoryBuffer()
{
	clear();
}

vk::CommandBuffer VulkanMemoryBuffer::transferToBuffer(VulkanMemoryBuffer& rhs, vk::CommandPool transferPool, vk::DeviceSize dstOffset)
{
	if (dstOffset + _totalSize > rhs._totalSize)
		throw std::runtime_error("Attempted to transfer buffers that do not match!");

	vk::CommandBufferAllocateInfo allocInfo{ transferPool, vk::CommandBufferLevel::ePrimary, 1 };
	vk::CommandBuffer cmdBuffer = _device.allocateCommandBuffers(allocInfo)[0];

	cmdBuffer.begin({ vk::CommandBufferUsageFlagBits::eOneTimeSubmit });

	vk::BufferCopy copyRegion{ 0, dstOffset, _totalSize };
	cmdBuffer.copyBuffer(_buffer, rhs._buffer, copyRegion);

	cmdBuffer.end();

	return cmdBuffer;
}

void VulkanMemoryBuffer::clear()
{
	if (_buffer)
		_device.destroyBuffer(_buffer);
	if (_memory)
		_device.freeMemory(_memory);

	_buffer = vk::Buffer();
	_memory = vk::DeviceMemory();

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

VulkanMemoryBuffer::Block::Block(
	vk::Device device,
	vk::DeviceMemory memory,
	vk::DeviceSize size,
	vk::DeviceSize offset)
	: _device(device), _memory(memory), _size(size), _offset(offset)
{
}

void VulkanMemoryBuffer::Block::copy(const void* data, vk::DeviceSize size)
{
#if defined(_DEBUG)
	if (size != _size)
		throw std::runtime_error("Tried to copy memory of mismatching sizes!");
#endif

	void* copyRegion = _device.mapMemory(_memory, _offset, size);
	memcpy(copyRegion, data, size);
	_device.unmapMemory(_memory);
}

vk::DeviceSize VulkanMemoryBuffer::Block::size() const
{
	return _size;
}

vk::DeviceSize VulkanMemoryBuffer::Block::offset() const
{
	return _offset;
}