/*! @file Render/VulkanMemoryBuffer.h */

#ifndef RENDER_VULKANMEMORYBUFFER_H
#define RENDER_VULKANMEMORYBUFFER_H
#pragma once

#include <cstdint>
#include <memory>
#include <vector>

#include <vulkan/vulkan.hpp>

namespace Orbit
{
	class VulkanBase;
	class VulkanImage;

	/*!
	@brief Wrapper class containing a vk::Buffer and vk::DeviceMemory object.
	Handles memory allocation through a simple block framework - block sizes are supplied and their access
	(read/write) is allowed through the getBlock() method.
	*/
	class VulkanMemoryBuffer final
	{
	public:
		/*!
		@brief Default constructor for the class. Does not allocate any memory - serves as a blank slate to be overriden.
		*/
		VulkanMemoryBuffer(std::nullptr_t);

		/*!
		@brief Constructs a VulkanMemoryBuffer object with the objects in parameter.
		@param base The base of the Vulkan renderer.
		@param blockSizes A collection of sizes for the blocks. The constructor computes the total size from these.
		@param createInfo The create info used to create the buffer. Its size property is set by the contructor.
		@param memFlags The flags that the memory should possess.
		*/
		explicit VulkanMemoryBuffer(
			std::shared_ptr<const VulkanBase> base,
			const std::vector<vk::DeviceSize>& blockSizes,
			vk::BufferCreateInfo createInfo,
			vk::MemoryPropertyFlags memFlags);

		VulkanMemoryBuffer(const VulkanMemoryBuffer&) = delete;
		VulkanMemoryBuffer& operator=(const VulkanMemoryBuffer& rhs) = delete;

		/*!
		@brief Move constructor for a VulkanMemoryBuffer. Transfers the owned objects.
		@param rhs The right hand side of the operation.
		*/
		VulkanMemoryBuffer(VulkanMemoryBuffer&& rhs);

		/*!
		@brief Move assignment operator for a VulkanMemoryBuffer. Transfers the owned objects.
		@param rhs The right hand side of the operation.
		@return A reference to this.
		*/
		VulkanMemoryBuffer& operator=(VulkanMemoryBuffer&& rhs);

		/*!
		@brief Destructor for the class. Ensures that the inner buffer and memory are correctly destroyed.
		*/
		~VulkanMemoryBuffer();

		/*!
		@brief Helper function to generate a command buffer that handles transfers to another buffer.
		@param rhs The buffer to be the destination of the data.
		@param dstOffset The offset into the destination memory.
		@return A recorded command buffer describing the transfer operation, ready to be submitted.
		*/
		vk::CommandBuffer transferToBuffer(VulkanMemoryBuffer& rhs, vk::DeviceSize dstOffset = 0Ui64);

		/*!
		@brief Helper function to generate a command buffer that handles transfers to an image.
		@param rhs The image to be the destination of the data.
		@return A recorded command buffer describing the transfer operation, ready to be submitted.
		*/
		vk::CommandBuffer transferToImage(VulkanImage& rhs);

		/*!
		@brief Destroys the Buffer and DeviceMemory members, essentially recreating a blank slate.
		*/
		void clear();

		/*!
		@brief Getter for the interior buffer.
		@return The inner vk::Buffer.
		*/
		vk::Buffer buffer() const;

		/*!
		@brief Block class describing a block of memory in a buffer.
		*/
		class Block final
		{
			/*! Only allow a VulkanMemoryBuffer access to the constructor. */
			friend class VulkanMemoryBuffer;

		public:
			/*!
			@brief Copy the data in parameter to the underlying memory (using memcpy) with the size in parameter.
			@throw std::runtime_error Throws if the size parameter is different from the Block's size.
			@param data The data source to copy.
			@param size The size of the data to copy.
			*/
			void copy(const void* data, vk::DeviceSize size);

			/*!
			@brief Getter for the size property of the class.
			@return The size property of the class.
			*/
			vk::DeviceSize size() const;

			/*!
			@brief Getter for the offset property of the class.
			@return The offset property of the class.
			*/
			vk::DeviceSize offset() const;

		private:
			/*!
			@brief Constructor for the class. Constructs the Block object with the parameters.
			@param base The base of the Vulkan renderer.
			@param memory The memory on which the block is based.
			@param size The size of the memory block.
			@param offset The offset of the memory block.
			*/
			Block(
				std::shared_ptr<const VulkanBase> base,
				vk::DeviceMemory memory,
				vk::DeviceSize size,
				vk::DeviceSize offset);

			/*! The Vulkan base. */
			std::shared_ptr<const VulkanBase> _base;
			/*! The actual underlying memory containing the Block. */
			vk::DeviceMemory _memory;
			/*! The size of the block's memory. */
			vk::DeviceSize _size;
			/*! The offset of the block's memory in the underlying memory. */
			vk::DeviceSize _offset;
		};

		/*!
		@brief Indexed getter for a block.
		@throw std::out_of_range Throws if the index is out of range - passed on by std::vector.
		@param index The index of the block to retrieve.
		@return The block.
		*/
		Block& getBlock(size_t index);

		/*!
		@brief Indexing operator for getting a block. Simply calls VulkanMemoryBuffer::getBlock().
		@throw std::out_of_range Throws if the index is out of range - passed on by std::vector.
		@param index The index of the block to retrieve.
		@return The block.
		*/
		Block& operator[](size_t index);

	private:
		/*! The renderer's base. */
		std::shared_ptr<const VulkanBase> _base;
		/*! Created and owned buffer. */
		vk::Buffer _buffer;
		/*! Created and owned memory. */
		vk::DeviceMemory _memory;

		/*! The total size of the DeviceMemory. */
		vk::DeviceSize _totalSize = 0;
		/*! The blocks making up the memory scheme. */
		std::vector<Block> _blocks;
	};
}

#endif //RENDER_VULKANMEMORYBUFFER_H