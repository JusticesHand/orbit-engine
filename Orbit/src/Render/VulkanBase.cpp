/*! @file Render/VulkanBase.cpp */

#include "Render/VulkanBase.h"

#include "Input/Window.h"

#include <iostream>

#if defined(USE_WIN32)
#error Win32Window not implemented yet!
#elif defined(USE_XWINDOW)
#error XWindow is not implemented yet!
#elif defined(USE_WAYLAND)
#error WaylandWindow is not implemented yet!
#else
#include <GLFW/glfw3.h>
#endif

namespace
{
	constexpr bool UseValidation = true;
	constexpr std::array<const char*, 1> RequiredDeviceExtensions{ VK_KHR_SWAPCHAIN_EXTENSION_NAME };
	constexpr std::array<const char*, 1> ValidationLayers{ "VK_LAYER_LUNARG_standard_validation" };

	/*!
	@brief Debug callback function to be run by Vulkan in case of errors.
	@param flags The flags of the debug error.
	@param objType The type of the object that triggered the error.
	@param obj The handle of the object that triggered the error.
	@param location The location of the object that triggered the error.
	@param code The error code.
	@param layerPrefix The prefix of the layer that triggered the error.
	@param msg The actual message of the error.
	@param userData User data for the error.
	@return Whether the error should abort the call or not. For the same behaviour as without
	debugging layers enabled, it should return VK_FALSE (which it does).
	*/
	VKAPI_ATTR VkBool32 VKAPI_CALL debugCallbackFunc(
		VkDebugReportFlagsEXT flags,
		VkDebugReportObjectTypeEXT objType,
		uint64_t obj,
		size_t location,
		int32_t code,
		const char* layerPrefix,
		const char* msg,
		void* userData)
	{
		std::cerr << "validation layer (" << layerPrefix << "): " << msg << std::endl;

		return VK_FALSE;
	}
}

using namespace Orbit;

bool VulkanBase::QueueFamilyIndices::completed() const
{
	return transferQueueFamily != std::numeric_limits<uint32_t>::max() &&
		graphicsQueueFamily != std::numeric_limits<uint32_t>::max() &&
		presentQueueFamily != std::numeric_limits<uint32_t>::max();
}

std::set<uint32_t> VulkanBase::QueueFamilyIndices::uniqueQueues() const
{
	return std::set<uint32_t>{ transferQueueFamily, graphicsQueueFamily, presentQueueFamily };
}

VulkanBase::VulkanBase(std::nullptr_t)
{
}

VulkanBase::VulkanBase(const Window* window)
{
	std::vector<const char*> extensions;
	std::vector<const char*> layers;

	if (UseValidation)// if constexpr
	{
		extensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
		layers.insert(layers.end(), ValidationLayers.begin(), ValidationLayers.end());
	}

#if defined(USE_WIN32)
#error Win32Window not implemented yet
#elif defined(USE_XWINDOW)
#error XWindow not implemented yet
#elif defined(USE_WAYLAND)
#error WaylandWindow not implemented yet
#else
	uint32_t glfwExtensionCount = 0;
	const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

	for (uint32_t i = 0; i < glfwExtensionCount; i++)
		extensions.push_back(glfwExtensions[i]);
#endif

	_instance = createInstance(extensions, layers);
	_debugCallback = createDebugReportCallback(_instance);
	_surface = createSurface(window->handle(), _instance);
	_physicalDevice = pickPhysicalDevice(_instance, _surface);

	_indices = retrieveQueueFamilyIndices(_physicalDevice, _surface);

	_device = createDevice(_physicalDevice, _indices);

	_transferCommandPool = createCommandPool(_device, _indices.transferQueueFamily);
	_graphicsCommandPool = createCommandPool(_device, _indices.graphicsQueueFamily);
}

VulkanBase::VulkanBase(VulkanBase&& rhs)
	: _instance(rhs._instance),
	_debugCallback(rhs._debugCallback),
	_surface(rhs._surface),
	_physicalDevice(rhs._physicalDevice),
	_device(rhs._device),
	_indices(rhs._indices),
	_transferCommandPool(rhs._transferCommandPool),
	_graphicsCommandPool(rhs._graphicsCommandPool)
{
	rhs._instance = nullptr;
	rhs._debugCallback = nullptr;
	rhs._surface = nullptr;
	rhs._physicalDevice = nullptr;
	rhs._device = nullptr;
	rhs._transferCommandPool = nullptr;
	rhs._graphicsCommandPool = nullptr;
}

VulkanBase& VulkanBase::operator=(VulkanBase&& rhs)
{
	_instance = rhs._instance;
	_debugCallback = rhs._debugCallback;
	_surface = rhs._surface;
	_physicalDevice = rhs._physicalDevice;
	_device = rhs._device;
	_indices = rhs._indices;
	_transferCommandPool = rhs._transferCommandPool;
	_graphicsCommandPool = rhs._graphicsCommandPool;

	rhs._instance = nullptr;
	rhs._debugCallback = nullptr;
	rhs._surface = nullptr;
	rhs._physicalDevice = nullptr;
	rhs._device = nullptr;
	rhs._transferCommandPool = nullptr;
	rhs._graphicsCommandPool = nullptr;

	return *this;
}

VulkanBase::~VulkanBase()
{
	if (_graphicsCommandPool)
		_device.destroyCommandPool(_graphicsCommandPool);

	if (_transferCommandPool)
		_device.destroyCommandPool(_transferCommandPool);

	if (_device)
		_device.destroy();

	if (_surface)
		_instance.destroySurfaceKHR(_surface);

	if (_debugCallback)
		destroyDebugCallback(_instance, _debugCallback);

	if (_instance)
		_instance.destroy();
}

vk::Instance VulkanBase::instance() const
{
	return _instance;
}

vk::SurfaceKHR VulkanBase::surface() const
{
	return _surface;
}

vk::PhysicalDevice VulkanBase::physicalDevice() const
{
	return _physicalDevice;
}

vk::Device VulkanBase::device() const
{
	return _device;
}

vk::Queue VulkanBase::transferQueue() const
{
	return _device.getQueue(_indices.transferQueueFamily, 0);
}

vk::Queue VulkanBase::graphicsQueue() const
{
	return _device.getQueue(_indices.graphicsQueueFamily, 0);
}

vk::Queue VulkanBase::presentQueue() const
{
	return _device.getQueue(_indices.presentQueueFamily, 0);
}

vk::CommandPool VulkanBase::transferCommandPool() const
{
	return _transferCommandPool;
}

vk::CommandPool VulkanBase::graphicsCommandPool() const
{
	return _graphicsCommandPool;
}

VulkanBase::QueueFamilyIndices VulkanBase::indices() const
{
	return _indices;
}

uint32_t VulkanBase::getMemoryTypeIndex(uint32_t filter, vk::MemoryPropertyFlags flags) const
{
	vk::PhysicalDeviceMemoryProperties memoryProperties = _physicalDevice.getMemoryProperties();

	for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; i++)
	{
		bool memorySuitable = static_cast<bool>(filter & (1 << i));
		bool hasCorrectProperties = (memoryProperties.memoryTypes[i].propertyFlags & flags) == flags;
		if (memorySuitable && hasCorrectProperties)
			return i;
	}

	return std::numeric_limits<uint32_t>::max();
}

void VulkanBase::getLayoutParameters(vk::ImageLayout layout, vk::PipelineStageFlags& stage, vk::AccessFlags& accessFlags)
{
	if (layout == vk::ImageLayout::eUndefined)
	{
		stage = vk::PipelineStageFlagBits::eTopOfPipe;
		accessFlags = vk::AccessFlags();
	}
	else if (layout == vk::ImageLayout::eTransferDstOptimal)
	{
		stage = vk::PipelineStageFlagBits::eTransfer;
		accessFlags = vk::AccessFlagBits::eTransferWrite;
	}
	else if (layout == vk::ImageLayout::eShaderReadOnlyOptimal)
	{
		stage = vk::PipelineStageFlagBits::eFragmentShader;
		accessFlags = vk::AccessFlagBits::eShaderRead;
	}
	else if (layout == vk::ImageLayout::eDepthStencilAttachmentOptimal)
	{
		stage = vk::PipelineStageFlagBits::eEarlyFragmentTests;
		accessFlags = vk::AccessFlagBits::eDepthStencilAttachmentRead | vk::AccessFlagBits::eDepthStencilAttachmentWrite;
	}
	else
	{
		throw std::runtime_error("Could not get layout parameters for a layout!");
	}
}

vk::Instance VulkanBase::createInstance(const std::vector<const char*>& extensions, const std::vector<const char*>& layers)
{
	vk::ApplicationInfo appInfo = vk::ApplicationInfo()
		.setApiVersion(VK_API_VERSION_1_0)
		.setPApplicationName("Orbit Engine")
		.setApplicationVersion(VK_MAKE_VERSION(1, 0, 0))
		.setPEngineName("Orbit Engine")
		.setEngineVersion(VK_MAKE_VERSION(1, 0, 0));

	vk::InstanceCreateInfo createInfo = vk::InstanceCreateInfo()
		.setEnabledExtensionCount(static_cast<uint32_t>(extensions.size()))
		.setPpEnabledExtensionNames(extensions.data())
		.setEnabledLayerCount(static_cast<uint32_t>(layers.size()))
		.setPpEnabledLayerNames(layers.data())
		.setPApplicationInfo(&appInfo);

	return vk::createInstance(createInfo);
}

vk::DebugReportCallbackEXT VulkanBase::createDebugReportCallback(const vk::Instance& instance)
{
	if (!UseValidation)//if constexpr
		return nullptr;

	PFN_vkCreateDebugReportCallbackEXT func =
		(PFN_vkCreateDebugReportCallbackEXT)_instance.getProcAddr("vkCreateDebugReportCallbackEXT");

	if (!func)
		throw std::runtime_error("Attempted to create a debug callback, but the extension is not present!");

	vk::DebugReportCallbackCreateInfoEXT createInfo = vk::DebugReportCallbackCreateInfoEXT()
		.setFlags(vk::DebugReportFlagBitsEXT::eError | vk::DebugReportFlagBitsEXT::eWarning | vk::DebugReportFlagBitsEXT::ePerformanceWarning)
		.setPfnCallback(debugCallbackFunc);

	VkDebugReportCallbackCreateInfoEXT cCreateInfo = static_cast<VkDebugReportCallbackCreateInfoEXT>(createInfo);

	VkDebugReportCallbackEXT debugReportCallback = nullptr;
	if (VK_SUCCESS != func(static_cast<VkInstance>(_instance), &cCreateInfo, nullptr, &debugReportCallback))
		throw std::runtime_error("There was an error createing the debug report callback!");

	return debugReportCallback;
}

vk::SurfaceKHR VulkanBase::createSurface(void* windowHandle, const vk::Instance& instance)
{
	VkSurfaceKHR surface = nullptr;

#if defined(USE_WIN32)
#error Win32Window not implemented yet
#elif defined(USE_XWINDOW)
#error XWindow not implemented yet
#elif defined(USE_WAYLAND)
#error WaylandWindow not implemented yet
#else
	glfwCreateWindowSurface(
		static_cast<VkInstance>(_instance),
		static_cast<GLFWwindow*>(windowHandle),
		nullptr,
		&surface);
#endif

	return surface;
}

vk::PhysicalDevice VulkanBase::pickPhysicalDevice(const vk::Instance& instance, const vk::SurfaceKHR& surface)
{
	std::vector<vk::PhysicalDevice> systemDevices = instance.enumeratePhysicalDevices();
	if (systemDevices.empty())
		throw std::runtime_error("Could not find a physical device that supports Vulkan!");

	vk::PhysicalDevice bestDevice = nullptr;

	for (const vk::PhysicalDevice& device : systemDevices)
	{
		vk::PhysicalDeviceProperties deviceProperties = device.getProperties();
		vk::PhysicalDeviceFeatures deviceFeatures = device.getFeatures();
		std::vector<vk::ExtensionProperties> extensionProperties = device.enumerateDeviceExtensionProperties();

		// TODO: More checks here to get a more suitable device if applicable as the renderer becomes
		// more complex.

		// Check whether or not the device can actually render on our surface, which is pretty important
		// considering we're attempting to do some rendering.
		// Applying negative logic here saves simplifies code.
		std::set<std::string> requiredExtensions{ VK_KHR_SWAPCHAIN_EXTENSION_NAME };
		for (const vk::ExtensionProperties& extension : extensionProperties)
			requiredExtensions.erase(extension.extensionName);

		if (!requiredExtensions.empty())
			continue;

		// Now we know that we have the required extensions, but do we have the required swap chain support?
		std::vector<vk::SurfaceFormatKHR> formats = device.getSurfaceFormatsKHR(surface);
		std::vector<vk::PresentModeKHR> presentModes = device.getSurfacePresentModesKHR(surface);
		if (formats.empty() || presentModes.empty())
			continue;

		// Check for device queues - there should at least be graphics, present and transfer queues
		// (which might overlap, and that doesn't really matter).
		QueueFamilyIndices queueFamilies = retrieveQueueFamilyIndices(device, surface);
		if (!queueFamilies.completed())
			continue;

		// Always prefer discrete GPUs over integrated (or virtual).
		if (deviceProperties.deviceType == vk::PhysicalDeviceType::eDiscreteGpu)
			return device;

		if (deviceProperties.deviceType == vk::PhysicalDeviceType::eIntegratedGpu ||
			deviceProperties.deviceType == vk::PhysicalDeviceType::eVirtualGpu)
			bestDevice = device;
	}

	if (!bestDevice)
		throw std::runtime_error("Could not choose a suitable physical device that support Vulkan!");

	return bestDevice;
}

vk::Device VulkanBase::createDevice(const vk::PhysicalDevice& device, const QueueFamilyIndices& queueFamilies)
{
	std::set<uint32_t> uniqueQueues{ queueFamilies.uniqueQueues() };
	std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;
	queueCreateInfos.reserve(uniqueQueues.size());

	for (uint32_t queueFamily : uniqueQueues)
	{
		const float queuePriority = 1.f;

		queueCreateInfos.push_back(vk::DeviceQueueCreateInfo()
			.setQueueFamilyIndex(queueFamily)
			.setQueueCount(1)
			.setPQueuePriorities(&queuePriority));
	}

	vk::PhysicalDeviceFeatures deviceFeatures;

	std::vector<const char*> validationLayers;
	if (UseValidation)//if constexpr
		validationLayers.insert(validationLayers.end(), ValidationLayers.begin(), ValidationLayers.end());

	vk::DeviceCreateInfo createInfo{
		vk::DeviceCreateFlags(),
		static_cast<uint32_t>(queueCreateInfos.size()), queueCreateInfos.data(),
		static_cast<uint32_t>(validationLayers.size()), validationLayers.data(),
		static_cast<uint32_t>(RequiredDeviceExtensions.size()), RequiredDeviceExtensions.data(),
		&deviceFeatures
	};

	return device.createDevice(createInfo);
}

vk::CommandPool VulkanBase::createCommandPool(const vk::Device& device, uint32_t queueFamilyIndex)
{
	vk::CommandPoolCreateInfo createInfo = vk::CommandPoolCreateInfo()
		.setFlags(vk::CommandPoolCreateFlagBits::eTransient | vk::CommandPoolCreateFlagBits::eResetCommandBuffer)
		.setQueueFamilyIndex(queueFamilyIndex);

	return device.createCommandPool(createInfo);
}

void VulkanBase::destroyDebugCallback(const vk::Instance& instance, vk::DebugReportCallbackEXT& callback)
{
	PFN_vkDestroyDebugReportCallbackEXT func =
		(PFN_vkDestroyDebugReportCallbackEXT)instance.getProcAddr("vkDestroyDebugReportCallbackEXT");

	if (!func)
		throw std::runtime_error("Could not find the function to destroy a debug callback!");

	func(static_cast<VkInstance>(_instance), static_cast<VkDebugReportCallbackEXT>(callback), nullptr);
	callback = nullptr;
}

VulkanBase::QueueFamilyIndices VulkanBase::retrieveQueueFamilyIndices(const vk::PhysicalDevice& physicalDevice, const vk::SurfaceKHR& surface)
{
	QueueFamilyIndices families;

	std::vector<vk::QueueFamilyProperties> queueFamilyProperties = physicalDevice.getQueueFamilyProperties();
	for (size_t i = 0; i < queueFamilyProperties.size(); i++)
	{
		const vk::QueueFamilyProperties& queueFamilyProperty = queueFamilyProperties[i];

		if (queueFamilyProperty.queueCount == 0)
			continue;

		uint32_t familyIndex = static_cast<uint32_t>(i);

		if (queueFamilyProperty.queueFlags & vk::QueueFlagBits::eGraphics)
			families.graphicsQueueFamily = familyIndex;

		if (physicalDevice.getSurfaceSupportKHR(familyIndex, surface))
			families.presentQueueFamily = familyIndex;

		if (queueFamilyProperty.queueFlags & vk::QueueFlagBits::eTransfer)
			families.transferQueueFamily = familyIndex;

		if (families.completed())
			break;
	}

	return families;
}