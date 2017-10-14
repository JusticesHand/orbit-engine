/*! @file Render/VulkanRenderer.cpp */

#include "Render/VulkanRenderer.h"

#include <GLFW/glfw3.h>

#include <iostream>
#include <vector>
#include <set>

namespace Orbit
{
	enum class RendererAPI : int
	{
		VulkanAPI = GLFW_NO_API
	};

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

constexpr const static std::array<const char*, 1> VALIDATION_LAYERS = {
	"VK_LAYER_LUNARG_standard_validation"
};

constexpr const static std::array<const char*, 1> REQUIRED_EXTENSIONS = {
	VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

using namespace Orbit;

VulkanRenderer::~VulkanRenderer()
{
	cleanup();
}

void VulkanRenderer::init(void* windowHandle, const glm::ivec2& windowSize)
{
	_instance = createInstance(windowHandle);
	_debugCallback = createDebugCallback();
	_surface = createSurface(windowHandle);

	_physicalDevice = pickPhysicalDevice();
	_device = createDevice();

	VulkanQueueFamilies queueFamilies = getQueueFamilies(_physicalDevice, _surface);
	_graphicsQueue = _device.getQueue(queueFamilies.graphicsQueueFamily, 0);
	_presentQueue = _device.getQueue(queueFamilies.presentQueueFamily, 0);
	_transferQueue = _device.getQueue(queueFamilies.transferQueueFamily, 0);

	_graphicsCommandPool = createCommandPool(queueFamilies.graphicsQueueFamily);
	_transferCommandPool = createCommandPool(queueFamilies.transferQueueFamily);

	_pipeline.init(windowSize, _physicalDevice, _device, _surface, _transferCommandPool);
	_modelRenderer.init(_physicalDevice, _device, _pipeline, _graphicsCommandPool, _transferCommandPool);
}

RendererAPI VulkanRenderer::getAPI() const
{
	return RendererAPI::VulkanAPI;
}

void VulkanRenderer::flagResize(const glm::ivec2& newSize)
{
	_presentQueue.waitIdle();
	_pipeline.resize(newSize);
	_modelRenderer.recreateBuffers(&_pipeline);
}

void VulkanRenderer::loadModels(const std::vector<ModelCountPair>& models)
{
	_modelRenderer.loadModels(models, _transferQueue);
}

void VulkanRenderer::setupViewProjection(const glm::mat4& view, const glm::mat4& projection)
{
	// Flip the middle y coordinate to flip the matrix around (since vulkan is flipped on that coordinate vs OGL).
	glm::mat4 flippedProjection = projection;
	flippedProjection[1][1] *= -1;
	_modelRenderer.setupViewProjection(flippedProjection * view);
}

void VulkanRenderer::queueRender(const std::vector<ModelTransformsPair>& modelTransforms)
{
	_modelRenderer.updateTransforms(modelTransforms);
}

void VulkanRenderer::renderFrame()
{
	_modelRenderer.renderFrame(_graphicsQueue, _presentQueue);
}

void VulkanRenderer::waitDeviceIdle()
{
	_device.waitIdle();
}

void VulkanRenderer::cleanup()
{
	waitDeviceIdle();

	_modelRenderer.cleanup();
	_pipeline.cleanup();

	_device.destroyCommandPool(_transferCommandPool);
	_device.destroyCommandPool(_graphicsCommandPool);

	_device.destroy();
	
	_instance.destroySurfaceKHR(_surface);
	destroyDebugCallback();
	_instance.destroy();
}

vk::Instance VulkanRenderer::createInstance(void* windowHandle)
{
	GLFWwindow* window = reinterpret_cast<GLFWwindow*>(windowHandle);

	std::vector<const char*> requiredExtensions;

	if (VALIDATION_LAYERS.size() > 0)
		requiredExtensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);

	unsigned int glfwExtensionCount = 0;
	const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

	for (unsigned int i = 0; i < glfwExtensionCount; i++)
		requiredExtensions.push_back(glfwExtensions[i]);

	vk::ApplicationInfo applicationInfo;
	applicationInfo.setApiVersion(VK_API_VERSION_1_0)
		.setPApplicationName("Orbit Engine")
		.setApplicationVersion(VK_MAKE_VERSION(1, 0, 0))
		.setPEngineName("Orbit Engine")
		.setEngineVersion(VK_MAKE_VERSION(1, 0, 0));

	vk::InstanceCreateInfo createInfo;
	createInfo
		.setEnabledExtensionCount(static_cast<uint32_t>(requiredExtensions.size()))
		.setPpEnabledExtensionNames(requiredExtensions.data())
		.setPApplicationInfo(&applicationInfo)
		.setEnabledLayerCount(static_cast<uint32_t>(VALIDATION_LAYERS.size()))
		.setPpEnabledLayerNames(VALIDATION_LAYERS.data());
	
	return vk::createInstance(createInfo);
}

vk::DebugReportCallbackEXT VulkanRenderer::createDebugCallback()
{
	// Don't create if we don't have to, obviously.
	if (VALIDATION_LAYERS.size() == 0)
		return nullptr;

	// Precondition: have the vulkan instance created.
	if (!_instance)
		throw std::runtime_error("The vulkan instance was not initialized before trying to create a debug callback!");

	PFN_vkCreateDebugReportCallbackEXT func = 
		(PFN_vkCreateDebugReportCallbackEXT)_instance.getProcAddr("vkCreateDebugReportCallbackEXT");

	if (!func)
		throw std::runtime_error("Attempted to create a debug callback, but the extension is not present!");

	vk::DebugReportCallbackCreateInfoEXT createInfo;
	createInfo
		.setFlags(vk::DebugReportFlagBitsEXT::eError | vk::DebugReportFlagBitsEXT::eWarning | vk::DebugReportFlagBitsEXT::ePerformanceWarning)
		.setPfnCallback(debugCallbackFunc);

	VkDebugReportCallbackCreateInfoEXT cCreateInfo = static_cast<VkDebugReportCallbackCreateInfoEXT>(createInfo);

	VkDebugReportCallbackEXT debugReportCallback;
	if (VK_SUCCESS != func(static_cast<VkInstance>(_instance), &cCreateInfo, nullptr, &debugReportCallback))
		throw std::runtime_error("There was an error creating the debug report callback!");

	return vk::DebugReportCallbackEXT(debugReportCallback);
}

vk::SurfaceKHR VulkanRenderer::createSurface(void* windowHandle)
{
	// Precondition: have the vulkan instance created.
	if (!_instance)
		throw std::runtime_error("The vulkan instance was not initialized before trying to create a window surface!");

	GLFWwindow* window = reinterpret_cast<GLFWwindow*>(windowHandle);

	VkSurfaceKHR surface;
	if (VK_SUCCESS != glfwCreateWindowSurface(_instance, window, nullptr, &surface))
		throw std::runtime_error("Therre was an error creating the window surface!");

	return vk::SurfaceKHR(surface);
}

vk::PhysicalDevice VulkanRenderer::pickPhysicalDevice()
{
	// Precondition: have the vulkan instance created and the surface created.
	if (!_instance)
		throw std::runtime_error("The vulkan instance was not initialized before trying to pick a physical device!");
	if (!_surface)
		throw std::runtime_error("The surface was not created before trying to pick a physical device!");

	std::vector<vk::PhysicalDevice> systemDevices = _instance.enumeratePhysicalDevices();
	if (systemDevices.empty())
		throw std::runtime_error("Could not find a physical device that supports Vulkan!");

	vk::PhysicalDevice bestDevice;

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
		std::set<std::string> requiredExtensions(REQUIRED_EXTENSIONS.begin(), REQUIRED_EXTENSIONS.end());
		for (const vk::ExtensionProperties& extension : extensionProperties)
			requiredExtensions.erase(extension.extensionName);

		if (!requiredExtensions.empty())
			continue;

		// Now we know that we have the required extensions, but do we have the required swap chain support?
		std::vector<vk::SurfaceFormatKHR> formats = device.getSurfaceFormatsKHR(_surface);
		std::vector<vk::PresentModeKHR> presentModes = device.getSurfacePresentModesKHR(_surface);
		if (formats.empty() || presentModes.empty())
			continue;

		// Check for device queues - there should at least be graphics, present and transfer queues
		// (which might overlap, and that doesn't really matter).
		VulkanQueueFamilies queueFamilies = getQueueFamilies(device, _surface);
		if (queueFamilies.graphicsQueueFamily == -1 ||
			queueFamilies.presentQueueFamily == -1 ||
			queueFamilies.transferQueueFamily == -1)
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

vk::Device VulkanRenderer::createDevice()
{
	// Precondition: have a picked physical device.
	if (!_physicalDevice)
		throw std::runtime_error("The physical device was not chosen before attempting to create a logical device!");

	VulkanQueueFamilies queueFamilies = getQueueFamilies(_physicalDevice, _surface);
	std::set<uint32_t> uniqueQueues{ queueFamilies.graphicsQueueFamily, queueFamilies.presentQueueFamily, queueFamilies.transferQueueFamily };
	std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;
	queueCreateInfos.reserve(uniqueQueues.size());

	for (uint32_t queueFamily : uniqueQueues)
	{
		const float queuePriority = 1.0f;

		vk::DeviceQueueCreateInfo queueCreateInfo;
		queueCreateInfo
			.setQueueFamilyIndex(queueFamily)
			.setQueueCount(1)
			.setPQueuePriorities(&queuePriority);

		queueCreateInfos.push_back(queueCreateInfo);
	}

	vk::PhysicalDeviceFeatures deviceFeatures;

	vk::DeviceCreateInfo createInfo;
	createInfo
		.setQueueCreateInfoCount(static_cast<uint32_t>(queueCreateInfos.size()))
		.setPQueueCreateInfos(queueCreateInfos.data())
		.setPEnabledFeatures(&deviceFeatures)
		.setEnabledExtensionCount(static_cast<uint32_t>(REQUIRED_EXTENSIONS.size()))
		.setPpEnabledExtensionNames(REQUIRED_EXTENSIONS.data())
		.setEnabledLayerCount(static_cast<uint32_t>(VALIDATION_LAYERS.size()))
		.setPpEnabledLayerNames(VALIDATION_LAYERS.data());

	return _physicalDevice.createDevice(createInfo);
}

vk::CommandPool VulkanRenderer::createCommandPool(int family)
{
	vk::CommandPoolCreateInfo createInfo;
	createInfo
		.setQueueFamilyIndex(family)
		.setFlags(vk::CommandPoolCreateFlagBits::eTransient | vk::CommandPoolCreateFlagBits::eResetCommandBuffer);

	return _device.createCommandPool(createInfo);
}

void VulkanRenderer::destroyDebugCallback()
{
	if (!_debugCallback || !_instance)
		return;

	PFN_vkDestroyDebugReportCallbackEXT func = 
		(PFN_vkDestroyDebugReportCallbackEXT)_instance.getProcAddr("vkDestroyDebugReportCallbackEXT");

	if (func)
		func(static_cast<VkInstance>(_instance), static_cast<VkDebugReportCallbackEXT>(_debugCallback), nullptr);
}