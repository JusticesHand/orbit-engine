/*! @file Render/VulkanMemoryBuffer.h */

#ifndef RENDER_VULKANMEMORYBUFFER_H
#define RENDER_VULKANMEMORYBUFFER_H
#pragma once

#include <cstdint>
#include <vulkan/vulkan.hpp>

#include <vector>

#include "Render/VulkanImage.h"

namespace Orbit
{
	class VulkanMemoryBuffer final
	{
	public:
		VulkanMemoryBuffer() = default;

		explicit VulkanMemoryBuffer(
			const vk::PhysicalDevice& physicalDevice,
			const vk::Device& device,
			const std::vector<vk::DeviceSize>& blockSizes,
			vk::BufferCreateInfo createInfo,
			vk::MemoryPropertyFlags memFlags);

		VulkanMemoryBuffer(const VulkanMemoryBuffer&) = delete;
		VulkanMemoryBuffer& operator=(const VulkanMemoryBuffer& rhs) = delete;

		VulkanMemoryBuffer(VulkanMemoryBuffer&& rhs);
		VulkanMemoryBuffer& operator=(VulkanMemoryBuffer&& rhs);

		~VulkanMemoryBuffer();

		vk::CommandBuffer transferToBuffer(VulkanMemoryBuffer& rhs, vk::CommandPool transferPool, vk::DeviceSize dstOffset = 0Ui64);
		vk::CommandBuffer transferToImage(VulkanImage& rhs, vk::CommandPool transferPool);

		void clear();

		vk::Buffer buffer() const;

		class Block final
		{
			friend class VulkanMemoryBuffer;

		public:
			/*!
			* @brief Precondition is that size here is equal to the size of the memory at creation.
			*/
			void copy(const void* data, vk::DeviceSize size);

			vk::DeviceSize size() const;
			vk::DeviceSize offset() const;

		private:
			Block(
				vk::Device device,
				vk::DeviceMemory memory,
				vk::DeviceSize size,
				vk::DeviceSize offset);

			vk::Device _device;
			vk::DeviceMemory _memory;
			vk::DeviceSize _size;
			vk::DeviceSize _offset;
		};

		Block& getBlock(size_t index);

	private:
		vk::Device _device;
		vk::Buffer _buffer;
		vk::DeviceMemory _memory;

		vk::DeviceSize _totalSize = 0;
		std::vector<Block> _blocks;
	};
}

#endif //RENDER_VULKANMEMORYBUFFER_H