/*! @file Render/VulkanImage.h */

#ifndef RENDER_VULKANIMAGE_H
#define RENDER_VULKANIMAGE_H
#pragma once

#include <memory>

#include <vulkan/vulkan.hpp>

namespace Orbit
{
	class VulkanBase;
	class VulkanBuffer;

	/*!
	@brief Wrapper class containing a vk::Image and its associated memory.
	As of right now, it is a one-to-one association - as in, each image object has its own memory.
	An image view object is created along the image.
	*/
	class VulkanImage final
	{
	public:
		/*!
		@brief Default constructor for the class. Leaves everything in a non-initialized (and unusable) state.
		*/
		VulkanImage(std::nullptr_t);

		/*!
		@brief Initializing constructor for the class. Creates and allocates resources with the objects in
		param.
		@param base The renderer's base.
		@param imageSizes The sizes to create image blocks with. All images within the object reside on a single memory.
		@param imageCreateInfo Create info struct for the image.
		@param memFlags The memory requirement flags for the image.
		*/
		explicit VulkanImage(
			std::shared_ptr<const VulkanBase> base,
			const std::vector<vk::Extent2D>& imageSizes,
			vk::ImageCreateInfo imageCreateInfo,
			vk::MemoryPropertyFlags memFlags);

		VulkanImage(const VulkanImage&) = delete;
		VulkanImage& operator=(const VulkanImage&) = delete;

		/*!
		@brief Move constructor for the class. Moves the created resources from rhs to this.
		@param rhs The image to take.
		*/
		VulkanImage(VulkanImage&& rhs);

		/*!
		@brief Move assignment operator for the class. Moves the created resources from rhs to this.
		@param rhs The image to take.
		@return A reference to this.
		*/
		VulkanImage& operator=(VulkanImage&& rhs);

		/*!
		@brief Destructor for the class. Frees all created resources.
		*/
		~VulkanImage();

		/*!
		@brief Frees up created/allocated resources. Essentially resets to a default state.
		*/
		void clear();

		/*!
		@brief Getter for the total size of the device memory.
		@return The total size of the device memory.
		*/
		vk::DeviceSize totalSize() const;

		/*!
		@brief Getter for the amount of images (blocks) in the object.
		@return The amount of images in the object.
		*/
		size_t imageCount() const;

		/*!
		@brief Transitions the layouts of all underlying images to the new layout.
		@param newLayout The new layout to adopt.
		@return A recorded command buffer, ready to submit.
		*/
		vk::CommandBuffer transitionLayouts(vk::ImageLayout newLayout);

		class Block final
		{
			/*! Only VulkanImage can access the constructor. */
			friend class VulkanImage;

		public:
			Block(const Block&) = delete;
			Block& operator=(const Block&) = delete;

			Block(Block&& rhs);
			Block& operator=(Block&& rhs);

			~Block();

			/*!
			@brief Getter for the image's size.
			@return The image's size.
			*/
			vk::DeviceSize size() const;

			/*!
			@brief Getter for the image extent.
			@return The image's extent.
			*/
			vk::Extent2D extent() const;

			/*!
			@brief Getter for the image layout.
			@return The image layout.
			*/
			vk::ImageLayout layout() const;

			/*!
			@brief Getter for the image's format.
			@return The image's format.
			*/
			vk::Format format() const;

			/*!
			@brief Getter for the image object.
			@return The image object.
			*/
			vk::Image image() const;

			/*!
			@brief Getter for the image view.
			@return The image view.
			*/
			vk::ImageView imageView() const;

			/*!
			@brief Getter for the sampler.
			@return The sampler.
			*/
			vk::Sampler sampler() const;

			/*!
			@brief Builds and records a command buffer that executes a layout transition.
			@param newLayout The new layout of the image.
			@param secondary Whether the returned command buffer should be a secondary buffer to be executed in another command.
			@return The recorded command buffer, ready to submit.
			*/
			vk::CommandBuffer transitionLayout(vk::ImageLayout newLayout, bool secondary = false);

			/*!
			@brief Builds and records a command buffer that copies data from the buffer in parameter to the image (block).
			@param buffer The source buffer from which to copy data.
			@param bufferOffset The offset into the source buffer from which to source data.
			@param secondary Whether the returned command buffer should be a secondary buffer to be executed in another command.
			@return The recorded command buffer, ready to submit.
			*/
			vk::CommandBuffer copy(const VulkanBuffer& buffer, vk::DeviceSize bufferOffset, bool secondary = false);

		private:
			Block(
				std::shared_ptr<const VulkanBase> base,
				const vk::Extent2D& extent,
				vk::ImageCreateInfo createInfo);

			/*!
			@brief Binds the image's memory to the memory in parameter with the offset.
			@param memory The memory to hold the image.
			@param offset The offset into this memory on which to hold the image.
			*/
			void bindMemory(const vk::DeviceMemory& memory, vk::DeviceSize offset);
			
			/*!
			@brief Getter for the memory requirements of the image.
			@return The memory requirements of the image.
			*/
			vk::MemoryRequirements memoryRequirements() const;

			/*! The renderer's base. */
			std::shared_ptr<const VulkanBase> _base;

			/*! The image object itself. */
			vk::Image _image;
			/*! The image view object. */
			vk::ImageView _imageView;

			/*! The image's extent. */
			vk::Extent2D _extent;

			/*! The image's sampler, if applicable. */
			vk::Sampler _sampler;

			/*! The image's current format. */
			vk::Format _format;
			/*! The image's current layout. Initially always vk::ImageLayout::eUndefined.*/
			vk::ImageLayout _layout = vk::ImageLayout::eUndefined;
		};

		/*!
		@brief Indexed getter for a block.
		@throw std::out_of_range Throws if the index is out of range - passed on by std::vector.
		@param index The index of the block to retrieve.
		@return The block.
		*/
		Block& getBlock(size_t i);

		/*! @copydoc VulkanImage::getBlock(size_t) */
		const Block& getBlock(size_t i) const;

		/*!
		@brief Indexing operator for getting a block. Simply calls VulkanMemoryBuffer::getBlock().
		@throw std::out_of_range Throws if the index is out of range - passed on by std::vector.
		@param index The index of the block to retrieve.
		@return The block.
		*/
		Block& operator[](size_t i);

		/*! @copydoc VulkanBuffer::operator[](size_t) */
		const Block& operator[](size_t i) const;

	private:
		/*! The renderer's base. */
		std::shared_ptr<const VulkanBase> _base;

		std::vector<Block> _blocks;

		/*! The underlying memory. */
		vk::DeviceMemory _memory;
	};
}

#endif //RENDER_VULKANIMAGE_H