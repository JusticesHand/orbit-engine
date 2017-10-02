/*! @file Render/VulkanGraphicsPipeline.cpp */

#include "Render/VulkanGraphicsPipeline.h"

#include <Util.h>

using namespace Orbit;

void VulkanGraphicsPipeline::init(
	const glm::ivec2& size,
	vk::PhysicalDevice physicalDevice, 
	vk::Device device, 
	vk::SurfaceKHR surface, 
	VulkanQueueFamilies families)
{
	_physicalDevice = physicalDevice;
	_device = device;
	_surface = surface;

	_surfaceFormat = chooseSurfaceFormat();
	_presentMode = choosePresentMode();
	_swapExtent = chooseExtent(size);

	_swapchain = createSwapchain();

	_swapchainImages = _device.getSwapchainImagesKHR(_swapchain);

	_swapchainImageViews.reserve(_swapchainImages.size());
	for (const vk::Image& image : _swapchainImages)
		_swapchainImageViews.push_back(createImageView(_device, image, _surfaceFormat.format));

	_renderPass = createRenderPass();
	_pipelineLayout = createPipelineLayout();
	_graphicsPipeline = createGraphicsPipeline();
	_framebuffers = createFramebuffers();
}

void VulkanGraphicsPipeline::resize(const glm::ivec2& newSize)
{
	// Precondition: have completed initialization.
	if (!_physicalDevice || !_device || !_surface)
		throw std::runtime_error("Attempted to resize the graphics pipeline before initialzation!");

	_swapExtent = chooseExtent(newSize);

	vk::SwapchainKHR newSwapchain = createSwapchain(_swapchain);

	_device.destroySwapchainKHR(_swapchain);

	for (vk::Framebuffer& framebuffer : _framebuffers)
		_device.destroyFramebuffer(framebuffer);
	_framebuffers.clear();

	for (vk::ImageView& imageView : _swapchainImageViews)
		_device.destroyImageView(imageView);
	_swapchainImageViews.clear();

	_swapchainImages.clear();

	_swapchain = newSwapchain;
	_swapchainImages = _device.getSwapchainImagesKHR(_swapchain);
	_swapchainImageViews.reserve(_swapchainImages.size());
	for (const vk::Image& image : _swapchainImages)
		_swapchainImageViews.push_back(createImageView(_device, image, _surfaceFormat.format));

	vk::Pipeline newPipeline = createGraphicsPipeline(_graphicsPipeline);
	_device.destroyPipeline(_graphicsPipeline);
	_graphicsPipeline = newPipeline;
	_framebuffers = createFramebuffers();
}

const std::vector<vk::Framebuffer>& VulkanGraphicsPipeline::getFramebuffers() const
{
	return _framebuffers;
}

vk::Extent2D VulkanGraphicsPipeline::getSwapExtent() const
{
	return _swapExtent;
}

vk::RenderPass VulkanGraphicsPipeline::getRenderPass() const
{
	return _renderPass;
}

vk::Pipeline VulkanGraphicsPipeline::getGraphicsPipeline() const
{
	return _graphicsPipeline;
}

vk::SwapchainKHR VulkanGraphicsPipeline::getSwapchain() const
{
	return _swapchain;
}

void VulkanGraphicsPipeline::cleanup()
{
	for (vk::Framebuffer& framebuffer : _framebuffers)
		_device.destroyFramebuffer(framebuffer);
	_framebuffers.clear();

	_device.destroyPipeline(_graphicsPipeline);
	_device.destroyPipelineLayout(_pipelineLayout);
	_device.destroyRenderPass(_renderPass);

	for (vk::ImageView& imageView : _swapchainImageViews)
		_device.destroyImageView(imageView);
	_swapchainImageViews.clear();

	_device.destroySwapchainKHR(_swapchain);
}

vk::SurfaceFormatKHR VulkanGraphicsPipeline::chooseSurfaceFormat()
{
	// Precondition: have done the first few lines of initialization.
	if (!_physicalDevice || !_device || !_surface)
		throw std::runtime_error("Attempted to choose a surface format before initialization!");

	std::vector<vk::SurfaceFormatKHR> formats = _physicalDevice.getSurfaceFormatsKHR(_surface);

	// Check if there is no preferred format.
	if (formats.size() == 1 && formats[0].format == vk::Format::eUndefined)
		return vk::SurfaceFormatKHR{ vk::Format::eB8G8R8A8Unorm, vk::ColorSpaceKHR::eSrgbNonlinear };

	// Check for the preferred format, if available.
	for (const vk::SurfaceFormatKHR format : formats)
		if (format.format == vk::Format::eB8G8R8A8Unorm && format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear)
			return format;

	// Return the first format, as at this point it doesn't really matter.
	return formats[0];
}

vk::PresentModeKHR VulkanGraphicsPipeline::choosePresentMode()
{
	// Precondition: have done the first few lines of initialization.
	if (!_physicalDevice || !_device || !_surface)
		throw std::runtime_error("Attempted to choose a present mode before initialization!");

	std::vector<vk::PresentModeKHR> presentModes = _physicalDevice.getSurfacePresentModesKHR(_surface);

	// Only FIFO is guaranteed to be there.
	vk::PresentModeKHR presentMode = vk::PresentModeKHR::eFifo;

	// Prefer mailbox for triple buffering. Immediate has better driver support in general, so prefer that
	// to FIFO.
	for (const vk::PresentModeKHR& mode : presentModes)
	{
		if (mode == vk::PresentModeKHR::eMailbox)
			return mode;
		else if (mode == vk::PresentModeKHR::eImmediate)
			presentMode = mode;
	}

	return presentMode;
}

vk::Extent2D VulkanGraphicsPipeline::chooseExtent(const glm::ivec2& size)
{
	// Precondition: have done the first few lines of initialization.
	if (!_physicalDevice || !_device || !_surface)
		throw std::runtime_error("Attempted to choose an extent before initialization!");

	vk::SurfaceCapabilitiesKHR capabilities = _physicalDevice.getSurfaceCapabilitiesKHR(_surface);

	if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
	{
		vk::Extent2D extent = capabilities.currentExtent;

		if (extent.width == 0)
			extent.width = 1;
		if (extent.height == 0)
			extent.height = 1;

		return extent;
	}

	vk::Extent2D extent = { static_cast<uint32_t>(size.x), static_cast<uint32_t>(size.y) };
	extent.width = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, extent.width));
	extent.height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, extent.height));

	if (extent.width == 0)
		extent.width = 1;
	if (extent.height == 0)
		extent.height = 1;

	return extent;
}

vk::SwapchainKHR VulkanGraphicsPipeline::createSwapchain(vk::SwapchainKHR oldSwapchain)
{
	// Precondition: have done the first few lines of initialization.
	if (!_physicalDevice || !_device || !_surface)
		throw std::runtime_error("Attempted to create a swapchain before initialization!");

	vk::SurfaceCapabilitiesKHR capabilities = _physicalDevice.getSurfaceCapabilitiesKHR(_surface);

	uint32_t imageCount = capabilities.minImageCount + 1;
	if (capabilities.maxImageCount > 0)
		imageCount = std::min(imageCount, capabilities.maxImageCount);

	// TODO: deferred rendering - set swapchain as transferDst and render to intermediate image instead
	vk::SwapchainCreateInfoKHR createInfo;
	createInfo
		.setSurface(_surface)
		.setMinImageCount(imageCount)
		.setImageFormat(_surfaceFormat.format)
		.setImageColorSpace(_surfaceFormat.colorSpace)
		.setImageExtent(_swapExtent)
		.setImageArrayLayers(1)
		.setImageUsage(vk::ImageUsageFlagBits::eColorAttachment)
		.setImageSharingMode(vk::SharingMode::eExclusive)
		.setPreTransform(capabilities.currentTransform)
		.setCompositeAlpha(vk::CompositeAlphaFlagBitsKHR::eOpaque)
		.setPresentMode(_presentMode)
		.setClipped(VK_TRUE)
		.setOldSwapchain(oldSwapchain);

	VulkanQueueFamilies families = getQueueFamilies(_physicalDevice, _surface);

	// Ensure we can still render even if we have two different queues for graphics/presentation
	if (families.graphicsQueueFamily != families.presentQueueFamily)
	{
		std::array<uint32_t, 2> queueFamilyIndices = {
			static_cast<uint32_t>(families.graphicsQueueFamily),
			static_cast<uint32_t>(families.presentQueueFamily)
		};

		createInfo.setImageSharingMode(vk::SharingMode::eConcurrent)
			.setQueueFamilyIndexCount(static_cast<uint32_t>(queueFamilyIndices.size()))
			.setPQueueFamilyIndices(queueFamilyIndices.data());
	}

	return _device.createSwapchainKHR(createInfo);
}

vk::RenderPass VulkanGraphicsPipeline::createRenderPass()
{
	vk::AttachmentDescription colorAttachment;
	colorAttachment
		.setFormat(_surfaceFormat.format)
		.setSamples(vk::SampleCountFlagBits::e1)
		.setLoadOp(vk::AttachmentLoadOp::eClear)
		.setStoreOp(vk::AttachmentStoreOp::eStore)
		.setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
		.setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
		.setInitialLayout(vk::ImageLayout::eUndefined)
		.setFinalLayout(vk::ImageLayout::ePresentSrcKHR);

	vk::AttachmentReference colorAttachmentRef;
	colorAttachmentRef
		.setAttachment(0)
		.setLayout(vk::ImageLayout::eColorAttachmentOptimal);

	vk::SubpassDependency dependency;
	dependency
		.setSrcSubpass(VK_SUBPASS_EXTERNAL)
		.setDstSubpass(0)
		.setSrcStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
		.setDstStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
		.setDstAccessMask(vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite);

	vk::SubpassDescription subpass;
	subpass
		.setPipelineBindPoint(vk::PipelineBindPoint::eGraphics)
		.setColorAttachmentCount(1)
		.setPColorAttachments(&colorAttachmentRef);

	vk::RenderPassCreateInfo createInfo;
	createInfo
		.setAttachmentCount(1)
		.setPAttachments(&colorAttachment)
		.setDependencyCount(1)
		.setPDependencies(&dependency)
		.setSubpassCount(1)
		.setPSubpasses(&subpass);

	return _device.createRenderPass(createInfo);
}

vk::PipelineLayout VulkanGraphicsPipeline::createPipelineLayout()
{
	vk::PipelineLayoutCreateInfo pipelineLayoutCreateInfo;
	pipelineLayoutCreateInfo
		.setSetLayoutCount(0)
		.setPSetLayouts(nullptr)
		.setPushConstantRangeCount(0)
		.setPPushConstantRanges(nullptr);

	return _device.createPipelineLayout(pipelineLayoutCreateInfo);
}

vk::Pipeline VulkanGraphicsPipeline::createGraphicsPipeline(vk::Pipeline oldPipeline)
{
	std::vector<char> vertexShaderCode = loadFile("Shaders/vert.spv");
	std::vector<char> fragmentShaderCode = loadFile("Shaders/frag.spv");

	vk::ShaderModuleCreateInfo shaderCreateInfo;
	shaderCreateInfo
		.setCodeSize(vertexShaderCode.size())
		.setPCode(reinterpret_cast<const uint32_t*>(vertexShaderCode.data()));

	vk::ShaderModule vertexShaderModule = _device.createShaderModule(shaderCreateInfo);

	shaderCreateInfo
		.setCodeSize(fragmentShaderCode.size())
		.setPCode(reinterpret_cast<const uint32_t*>(fragmentShaderCode.data()));

	vk::ShaderModule fragmentShaderModule = _device.createShaderModule(shaderCreateInfo);

	// TODO: Set shader entry name depending on the quality settings instead of "main".
	std::array<vk::PipelineShaderStageCreateInfo, 2> shaderStageCreateInfos;

	shaderStageCreateInfos[0]
		.setStage(vk::ShaderStageFlagBits::eVertex)
		.setModule(vertexShaderModule)
		.setPName("main");

	shaderStageCreateInfos[1]
		.setStage(vk::ShaderStageFlagBits::eFragment)
		.setModule(fragmentShaderModule)
		.setPName("main");

	vk::PipelineVertexInputStateCreateInfo vertexInputInfo;
	vertexInputInfo
		.setVertexBindingDescriptionCount(0)
		.setVertexAttributeDescriptionCount(0);

	vk::PipelineInputAssemblyStateCreateInfo inputAssembly;
	inputAssembly
		.setTopology(vk::PrimitiveTopology::eTriangleList)
		.setPrimitiveRestartEnable(VK_FALSE);

	vk::Viewport viewport;
	viewport
		.setX(0.f)
		.setY(0.f)
		.setWidth(static_cast<float>(_swapExtent.width))
		.setHeight(static_cast<float>(_swapExtent.height))
		.setMinDepth(0.f)
		.setMaxDepth(1.f);

	vk::Rect2D scissor;
	scissor.setOffset({ 0, 0 }).setExtent(_swapExtent);

	vk::PipelineViewportStateCreateInfo viewportState;
	viewportState
		.setViewportCount(1)
		.setPViewports(&viewport)
		.setScissorCount(1)
		.setPScissors(&scissor);

	vk::PipelineRasterizationStateCreateInfo rasterizer;
	rasterizer
		.setDepthClampEnable(VK_FALSE)
		.setRasterizerDiscardEnable(VK_FALSE)
		.setPolygonMode(vk::PolygonMode::eFill)
		.setLineWidth(1.f)
		.setCullMode(vk::CullModeFlagBits::eBack)
		.setFrontFace(vk::FrontFace::eClockwise)
		.setDepthBiasEnable(VK_FALSE);

	vk::PipelineMultisampleStateCreateInfo multisampling;
	multisampling
		.setSampleShadingEnable(VK_FALSE)
		.setRasterizationSamples(vk::SampleCountFlagBits::e1);

	// TODO
	vk::PipelineDepthStencilStateCreateInfo depthStencil;

	vk::PipelineColorBlendAttachmentState colorBlendAttachment;
	colorBlendAttachment
		.setColorWriteMask(vk::ColorComponentFlagBits::eR |
			vk::ColorComponentFlagBits::eG |
			vk::ColorComponentFlagBits::eB |
			vk::ColorComponentFlagBits::eA)
		.setBlendEnable(VK_TRUE)
		.setSrcColorBlendFactor(vk::BlendFactor::eSrcAlpha)
		.setDstColorBlendFactor(vk::BlendFactor::eOneMinusSrcAlpha)
		.setColorBlendOp(vk::BlendOp::eAdd)
		.setSrcAlphaBlendFactor(vk::BlendFactor::eOne)
		.setDstAlphaBlendFactor(vk::BlendFactor::eZero)
		.setAlphaBlendOp(vk::BlendOp::eAdd);

	vk::PipelineColorBlendStateCreateInfo colorBlending;
	colorBlending
		.setLogicOpEnable(VK_FALSE)
		.setAttachmentCount(1)
		.setPAttachments(&colorBlendAttachment);

	std::array<vk::DynamicState, 1> dynamicStates = { vk::DynamicState::eViewport };

	vk::PipelineDynamicStateCreateInfo dynamicState;
	dynamicState
		.setDynamicStateCount(static_cast<uint32_t>(dynamicStates.size()))
		.setPDynamicStates(dynamicStates.data());

	vk::GraphicsPipelineCreateInfo createInfo;
	createInfo
		.setStageCount(static_cast<uint32_t>(shaderStageCreateInfos.size()))
		.setPStages(shaderStageCreateInfos.data())
		.setPVertexInputState(&vertexInputInfo)
		.setPInputAssemblyState(&inputAssembly)
		.setPViewportState(&viewportState)
		.setPRasterizationState(&rasterizer)
		.setPMultisampleState(&multisampling)
		.setPDepthStencilState(nullptr)	// TODO
		.setPColorBlendState(&colorBlending)
		.setPDynamicState(nullptr)	// TODO
		.setLayout(_pipelineLayout)
		.setRenderPass(_renderPass)
		.setSubpass(0)
		.setBasePipelineIndex(-1)
		.setBasePipelineHandle(oldPipeline);

	// TODO: Look into the vk::PipelineCache to speed up pipeline creation
	vk::Pipeline graphicsPipeline = _device.createGraphicsPipeline(nullptr, createInfo);

	_device.destroyShaderModule(fragmentShaderModule);
	_device.destroyShaderModule(vertexShaderModule);

	return graphicsPipeline;
}

std::vector<vk::Framebuffer> VulkanGraphicsPipeline::createFramebuffers()
{
	std::vector<vk::Framebuffer> framebuffers;
	framebuffers.reserve(_swapchainImageViews.size());

	for (const vk::ImageView& imageView : _swapchainImageViews)
	{
		std::array<vk::ImageView, 1> attachments = { imageView };

		vk::FramebufferCreateInfo createInfo;
		createInfo
			.setRenderPass(_renderPass)
			.setAttachmentCount(static_cast<uint32_t>(attachments.size()))
			.setPAttachments(attachments.data())
			.setWidth(_swapExtent.width)
			.setHeight(_swapExtent.height)
			.setLayers(1);

		framebuffers.push_back(_device.createFramebuffer(createInfo));
	}

	return std::move(framebuffers);
}