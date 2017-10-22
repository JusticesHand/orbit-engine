/*! @file Render/VulkanBase.h */

#ifndef RENDER_VULKANBASE_H
#define RENDER_VULKANBASE_H
#pragma once

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

#include <vulkan/vulkan.hpp>

#include <set>

namespace Orbit
{
	class Window;

	/*!
	@brief Serves as a container and helper class for Vulkan-based operations.
	Offers helper functions to access the API in a more fluent manner, as well as serve as initialization for
	the main sections of the API (instance, physical device, device, surface, etc).
	*/
	class VulkanBase final
	{
	public:
		/*!
		@brief Struct simplifying searching for queue families.
		*/
		struct QueueFamilyIndices
		{
			/*! The index of the transfer queue family. */
			uint32_t transferQueueFamily = std::numeric_limits<uint32_t>::max();
			/*! The index of the graphics queue family. */
			uint32_t graphicsQueueFamily = std::numeric_limits<uint32_t>::max();
			/*! The index of the present queue family. */
			uint32_t presentQueueFamily = std::numeric_limits<uint32_t>::max();

			/*!
			@brief Returns whether or not all queue families are accounted for.
			@return Whether or not all queue families are accounted for.
			*/
			bool completed() const;

			/*!
			@brief Returns a set of unique queues, as to not duplicate queues (since a queue family can
			correspond to different queue family types).
			@return A set of unique queue families.
			*/
			std::set<uint32_t> uniqueQueues() const;
		};

		/*!
		@brief Constructor that initializes the class in a simple (but mostly undefined) state.
		*/
		VulkanBase(std::nullptr_t);

		/*!
		@brief Constructor that handles initialization of the base Vulkan state.
		@param window The renderer's parent window.
		*/
		explicit VulkanBase(const Window* window);

		VulkanBase(const VulkanBase&) = delete;
		VulkanBase& operator=(const VulkanBase&) = delete;

		/*!
		@brief Move constructor for the class. Copy semantics don't make sense for this kind of renderer,
		as everything copied would be handles. Furthermore, to do a deep copy you'd simply redo the whole
		initialization process anyways.
		@param rhs The right hand side of the operation.
		*/
		VulkanBase(VulkanBase&& rhs);

		/*!
		@brief Move assignment operator for the class. Copy semantics don't make sense for this kind of renderer,
		as everything copied would be handles. Furthermore, to do a deep copy you'd simply redo the whole
		initialization process anyways.
		@param rhs The right hand side of the operation.
		@return A reference to this.
		*/
		VulkanBase& operator=(VulkanBase&& rhs);

		/*!
		@brief Cleans up the created objects, if applicable.
		*/
		~VulkanBase();

		/*!
		@brief Getter for the class's instance.
		@return The class's instance.
		*/
		vk::Instance instance() const;

		/*!
		@brief Getter for the class's surface.
		@return The class's surface.
		*/
		vk::SurfaceKHR surface() const;

		/*!
		@brief Getter for the class's physical device.
		@return The class's physical device.
		*/
		vk::PhysicalDevice physicalDevice() const;

		/*!
		@brief Getter for the class's device.
		@return The class's device.
		*/
		vk::Device device() const;

		/*!
		@brief Getter for the class's transfer queue.
		@return The class's transfer queue.
		*/
		vk::Queue transferQueue() const;

		/*!
		@brief Getter for the class's graphics queue.
		@return The class's graphics queue.
		*/
		vk::Queue graphicsQueue() const;

		/*!
		@brief Getter for the class's presentation queue.
		@return The class's presentation queue.
		*/
		vk::Queue presentQueue() const;

		/*!
		@brief Getter for the class's transfer command pool.
		@return The class's transfer command pool.
		*/
		vk::CommandPool transferCommandPool() const;

		/*!
		@brief Getter for the class's graphics command pool.
		@return The class's graphics command pool.
		*/
		vk::CommandPool graphicsCommandPool() const;

		/*!
		@brief Helper function, returns the queue family indices for the base's physical device and surface.
		@return The queue family indices for the base's physical device and surface.
		*/
		QueueFamilyIndices indices() const;

		/*!
		@brief Helper function, returns a proper memory type index from a filter.
		@param filter The filter containing the available memory types.
		@param flags The flags that the memory should satisfy.
		@return The found index of the most appropriate memory type.
		*/
		uint32_t getMemoryTypeIndex(uint32_t filter, vk::MemoryPropertyFlags flags) const;

		/*!
		@brief Helper function to retrieve stage and access flags necessary for layout transitions.
		@param[in] layout The layout to poll.
		@param[out] stage The retrieved pipeline stage flags.
		@param[out] accessFlags The retrieved access flags.
		*/
		static void getLayoutParameters(vk::ImageLayout layout, vk::PipelineStageFlags& stage, vk::AccessFlags& accessFlags);

	private:
		/*!
		@brief Helper function to create a Vulkan instance.
		@param extensions A reference to the extensions that the instance should support.
		@param layers The layers that are enabled for the instance.
		@return A newly created Vulkan instance.
		*/
		vk::Instance createInstance(const std::vector<const char*>& extensions, const std::vector<const char*>& layers);

		/*!
		@brief Helper function to create a debug callback.
		@param instance The instance to use to create the debug callback.
		@return A newly created Vulkan DebugReportCallback.
		*/
		vk::DebugReportCallbackEXT createDebugReportCallback(const vk::Instance& instance);

		/*!
		@brief Helper function to create a Surface on which rendering operations are possible.
		@param windowHandle The handle of the window to use.
		@param instance The instance to use to create the surface.
		@return A newly created Vulkan Surface.
		*/
		vk::SurfaceKHR createSurface(void* windowHandle, const vk::Instance& instance);

		/*!
		@brief Helper function to pick a physical device for use with the surface.
		@param instance The instance to use to pick the device.
		@param surface The surface on which to base selection criteria.
		@return The picked physical device.
		*/
		vk::PhysicalDevice pickPhysicalDevice(const vk::Instance& instance, const vk::SurfaceKHR& surface);

		/*!
		@brief Helper function to create a logical device.
		@param device The physical device on which to base the device on.
		@param queueFamilies The queue families used by the device.
		@return The newly created logical device.
		*/
		vk::Device createDevice(const vk::PhysicalDevice& device, const QueueFamilyIndices& queueFamilies);

		/*!
		@brief Helper function to create a command pool.
		@param device The device to use to create the command pool.
		@param queueFamilyIndex The queue family index for which to create the command pool.
		@return The newly created command pool.
		*/
		vk::CommandPool createCommandPool(const vk::Device& device, uint32_t queueFamilyIndex);

		/*!
		@brief Helper function to destroy a debug callback.
		@param[in] instance The instance to use to destroy the debug callback.
		@param[in,out] callback The debug callback to destroy.
		*/
		void destroyDebugCallback(const vk::Instance& instance, vk::DebugReportCallbackEXT& callback);

		/*!
		@brief Retrieves the queue family indices for the physical device in parameter applied to the surface in parameter.
		@param physicalDevice The physical device to poll.
		@param surface The surface to poll.
		@return The computed QueueFamilyIndices.
		*/
		QueueFamilyIndices retrieveQueueFamilyIndices(const vk::PhysicalDevice& physicalDevice, const vk::SurfaceKHR& surface);

		/*! The instance to use for operations. */
		vk::Instance _instance;
		/*! The debug callback used for debugging. */
		vk::DebugReportCallbackEXT _debugCallback;
		/*! The surface on which things will be rendered. */
		vk::SurfaceKHR _surface;
		/*! The picked physical device. */
		vk::PhysicalDevice _physicalDevice;
		/*! The used logical device. */
		vk::Device _device;

		/*! The saved queue family indices, for retrieval and queue getting. */
		QueueFamilyIndices _indices;

		/*! The transfer command pool. */
		vk::CommandPool _transferCommandPool;
		/*! The graphics command pool. */
		vk::CommandPool _graphicsCommandPool;
	};
}

#endif //RENDER_VULKANBASE_H