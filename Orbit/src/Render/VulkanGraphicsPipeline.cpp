/*! @file Render/VulkanGraphicsPipeline.cpp */

#include "Render/VulkanGraphicsPipeline.h"

#include "Render/VulkanUtils.h"

#include <Render/Model.h>

#include <Util.h>

using namespace Orbit;

VulkanGraphicsPipeline::VulkanGraphicsPipeline(std::nullptr_t)
{
}

VulkanGraphicsPipeline::VulkanGraphicsPipeline(std::shared_ptr<const VulkanBase> base, const glm::ivec2& size)
	: _base(base)
{
	_surfaceFormat = chooseSurfaceFormat(_base->physicalDevice(), _base->surface());
	_presentMode = choosePresentMode(_base->physicalDevice(), _base->surface());
	_swapExtent = chooseExtent(_base->physicalDevice(), _base->surface(), size);

	_swapchain = createSwapchain(
		_base->physicalDevice(), 
		_base->device(), 
		_base->surface(), 
		_surfaceFormat,
		_swapExtent, 
		_presentMode,
		_base->indices());

	_swapchainImages = _base->device().getSwapchainImagesKHR(_swapchain);

	_swapchainImageViews.reserve(_swapchainImages.size());
	for (const vk::Image& image : _swapchainImages)
		_swapchainImageViews.push_back(createImageView(_base->device(), image, _surfaceFormat.format));

	vk::ImageCreateInfo depthImageCreateInfo;
	depthImageCreateInfo
		.setImageType(vk::ImageType::e2D)
		.setExtent({ _swapExtent.width, _swapExtent.height, 1 })
		.setMipLevels(1)
		.setArrayLayers(1)
		.setFormat(vk::Format::eD32Sfloat)
		.setTiling(vk::ImageTiling::eOptimal)
		.setInitialLayout(vk::ImageLayout::eUndefined)
		.setUsage(vk::ImageUsageFlagBits::eDepthStencilAttachment)
		.setSamples(vk::SampleCountFlagBits::e1)
		.setSharingMode(vk::SharingMode::eExclusive);

	_depthImage = VulkanImage{
		_base,
		{ _swapExtent },
		depthImageCreateInfo,
		vk::MemoryPropertyFlagBits::eDeviceLocal
	};

	vk::Fence transitionFence = _base->device().createFence({});
	vk::CommandBuffer transitionBuffer = _depthImage[0].transitionLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal);

	vk::SubmitInfo submit = vk::SubmitInfo()
		.setCommandBufferCount(1)
		.setPCommandBuffers(&transitionBuffer);

	_base->transferQueue().submit(submit, transitionFence);
	
	_renderPass = createRenderPass(_base->device(), _surfaceFormat, _depthImage);
	_descriptorSetLayout = createDescriptorSetLayout(_base->device());
	_pipelineLayout = createPipelineLayout(_base->device(), _descriptorSetLayout);
	_graphicsPipeline = createGraphicsPipeline(_base->device(), _swapExtent, _pipelineLayout, _renderPass);
	_framebuffers = createFramebuffers(_base->device(), _swapchainImageViews, _depthImage, _renderPass, _swapExtent);

	_base->device().waitForFences(transitionFence, VK_TRUE, std::numeric_limits<uint64_t>::max());
	_base->device().destroyFence(transitionFence);
}

VulkanGraphicsPipeline::VulkanGraphicsPipeline(VulkanGraphicsPipeline&& rhs)
	: _base(rhs._base),
	_surfaceFormat(rhs._surfaceFormat),
	_presentMode(rhs._presentMode),
	_swapExtent(rhs._swapExtent),
	_swapchain(rhs._swapchain),
	_swapchainImages(std::move(rhs._swapchainImages)),
	_swapchainImageViews(std::move(rhs._swapchainImageViews)),
	_framebuffers(std::move(rhs._framebuffers)),
	_renderPass(rhs._renderPass),
	_pipelineLayout(rhs._pipelineLayout),
	_descriptorSetLayout(rhs._descriptorSetLayout),
	_descriptorPool(rhs._descriptorPool),
	_graphicsPipeline(rhs._graphicsPipeline),
	_depthImage(std::move(rhs._depthImage))
{
	rhs._base = nullptr;
	rhs._surfaceFormat = vk::SurfaceFormatKHR();
	rhs._presentMode = vk::PresentModeKHR();
	rhs._swapExtent = vk::Extent2D();
	rhs._renderPass = nullptr;
	rhs._pipelineLayout = nullptr;
	rhs._descriptorSetLayout = nullptr;
	rhs._descriptorPool = nullptr;
	rhs._graphicsPipeline = nullptr;
}

VulkanGraphicsPipeline& VulkanGraphicsPipeline::operator=(VulkanGraphicsPipeline&& rhs)
{
	_base = rhs._base;
	_surfaceFormat=rhs._surfaceFormat;
	_presentMode = rhs._presentMode;
	_swapExtent = rhs._swapExtent;
	_swapchain = rhs._swapchain;
	_swapchainImages = std::move(rhs._swapchainImages);
	_swapchainImageViews = std::move(rhs._swapchainImageViews);
	_framebuffers = std::move(rhs._framebuffers);
	_renderPass = rhs._renderPass;
	_pipelineLayout = rhs._pipelineLayout;
	_descriptorSetLayout = rhs._descriptorSetLayout;
	_descriptorPool = rhs._descriptorPool;
	_graphicsPipeline = rhs._graphicsPipeline;
	_depthImage = std::move(rhs._depthImage);

	rhs._base = nullptr;
	rhs._surfaceFormat = vk::SurfaceFormatKHR();
	rhs._presentMode = vk::PresentModeKHR();
	rhs._swapExtent = vk::Extent2D();
	rhs._renderPass = nullptr;
	rhs._pipelineLayout = nullptr;
	rhs._descriptorSetLayout = nullptr;
	rhs._descriptorPool = nullptr;
	rhs._graphicsPipeline = nullptr;
	
	return *this;
}

VulkanGraphicsPipeline::~VulkanGraphicsPipeline()
{
	if (!_base)
		return;

	for (vk::Framebuffer& framebuffer : _framebuffers)
		_base->device().destroyFramebuffer(framebuffer);
	_framebuffers.clear();

	_depthImage.clear();
	_base->device().destroyPipeline(_graphicsPipeline);
	if (_descriptorPool)
		_base->device().destroyDescriptorPool(_descriptorPool);
	_base->device().destroyDescriptorSetLayout(_descriptorSetLayout);
	_base->device().destroyPipelineLayout(_pipelineLayout);
	_base->device().destroyRenderPass(_renderPass);

	for (vk::ImageView& imageView : _swapchainImageViews)
		_base->device().destroyImageView(imageView);
	_swapchainImageViews.clear();

	_base->device().destroySwapchainKHR(_swapchain);
}

void VulkanGraphicsPipeline::resize(const glm::ivec2& newSize)
{
	// Precondition: have completed initialization.
	if (!_base)
		throw std::runtime_error("Attempted to resize the graphics pipeline before initialzation!");

	_swapExtent = chooseExtent(_base->physicalDevice(), _base->surface(), newSize);

	vk::SwapchainKHR newSwapchain = createSwapchain(
		_base->physicalDevice(),
		_base->device(),
		_base->surface(),
		_surfaceFormat,
		_swapExtent,
		_presentMode,
		_base->indices(),
		_swapchain);

	_base->device().destroySwapchainKHR(_swapchain);

	for (vk::Framebuffer& framebuffer : _framebuffers)
		_base->device().destroyFramebuffer(framebuffer);
	_framebuffers.clear();

	for (vk::ImageView& imageView : _swapchainImageViews)
		_base->device().destroyImageView(imageView);
	_swapchainImageViews.clear();

	_swapchainImages.clear();

	_swapchain = newSwapchain;
	_swapchainImages = _base->device().getSwapchainImagesKHR(_swapchain);
	_swapchainImageViews.reserve(_swapchainImages.size());
	for (const vk::Image& image : _swapchainImages)
		_swapchainImageViews.push_back(createImageView(_base->device(), image, _surfaceFormat.format));

	vk::Pipeline newPipeline = createGraphicsPipeline(_base->device(), _swapExtent, _pipelineLayout, _renderPass, _graphicsPipeline);
	_base->device().destroyPipeline(_graphicsPipeline);
	_graphicsPipeline = newPipeline;
	_framebuffers = createFramebuffers(_base->device(), _swapchainImageViews, _depthImage, _renderPass, _swapExtent);
}

const std::vector<vk::Framebuffer>& VulkanGraphicsPipeline::framebuffers() const
{
	return _framebuffers;
}

vk::Extent2D VulkanGraphicsPipeline::swapExtent() const
{
	return _swapExtent;
}

vk::RenderPass VulkanGraphicsPipeline::renderPass() const
{
	return _renderPass;
}

void VulkanGraphicsPipeline::updateDescriptorPool(uint32_t maxSets)
{
	if (_descriptorPool)
		_base->device().destroyDescriptorPool(_descriptorPool);

	_descriptorPool = createDescriptorPool(_base->device(), maxSets);
}

vk::DescriptorSet VulkanGraphicsPipeline::allocateDescriptorSet() const
{
	return createDescriptorSet(_base->device(), _descriptorPool, _descriptorSetLayout);
}

vk::PipelineLayout VulkanGraphicsPipeline::pipelineLayout() const
{
	return _pipelineLayout;
}

vk::Pipeline VulkanGraphicsPipeline::graphicsPipeline() const
{
	return _graphicsPipeline;
}

vk::SwapchainKHR VulkanGraphicsPipeline::swapchain() const
{
	return _swapchain;
}

vk::SurfaceFormatKHR VulkanGraphicsPipeline::chooseSurfaceFormat(const vk::PhysicalDevice& physicalDevice, const vk::SurfaceKHR& surface)
{
	std::vector<vk::SurfaceFormatKHR> formats = physicalDevice.getSurfaceFormatsKHR(surface);

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

vk::PresentModeKHR VulkanGraphicsPipeline::choosePresentMode(const vk::PhysicalDevice& physicalDevice, const vk::SurfaceKHR& surface)
{
	std::vector<vk::PresentModeKHR> presentModes = physicalDevice.getSurfacePresentModesKHR(surface);

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

vk::Extent2D VulkanGraphicsPipeline::chooseExtent(
	const vk::PhysicalDevice& physicalDevice,
	const vk::SurfaceKHR& surface, 
	const glm::ivec2& size)
{
	vk::SurfaceCapabilitiesKHR capabilities = physicalDevice.getSurfaceCapabilitiesKHR(surface);

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

vk::SwapchainKHR VulkanGraphicsPipeline::createSwapchain(
	const vk::PhysicalDevice& physicalDevice, 
	const vk::Device& device,
	const vk::SurfaceKHR& surface,
	const vk::SurfaceFormatKHR& surfaceFormat,
	const vk::Extent2D& swapExtent,
	const vk::PresentModeKHR& presentMode,
	const VulkanBase::QueueFamilyIndices& indices,
	vk::SwapchainKHR oldSwapchain)
{
	vk::SurfaceCapabilitiesKHR capabilities = physicalDevice.getSurfaceCapabilitiesKHR(surface);

	uint32_t imageCount = capabilities.minImageCount + 1;
	if (capabilities.maxImageCount > 0)
		imageCount = std::min(imageCount, capabilities.maxImageCount);

	// TODO: deferred rendering - set swapchain as transferDst and render to intermediate image instead
	vk::SwapchainCreateInfoKHR createInfo = vk::SwapchainCreateInfoKHR()
		.setSurface(surface)
		.setMinImageCount(imageCount)
		.setImageFormat(surfaceFormat.format)
		.setImageColorSpace(surfaceFormat.colorSpace)
		.setImageExtent(swapExtent)
		.setImageArrayLayers(1)
		.setImageUsage(vk::ImageUsageFlagBits::eColorAttachment)
		.setImageSharingMode(vk::SharingMode::eExclusive)
		.setPreTransform(capabilities.currentTransform)
		.setCompositeAlpha(vk::CompositeAlphaFlagBitsKHR::eOpaque)
		.setPresentMode(presentMode)
		.setClipped(VK_TRUE)
		.setOldSwapchain(oldSwapchain);

	// Ensure we can still render even if we have two different queues for graphics/presentation
	if (indices.graphicsQueueFamily != indices.presentQueueFamily)
	{
		std::array<uint32_t, 2> queueFamilyIndices = {
			static_cast<uint32_t>(indices.graphicsQueueFamily),
			static_cast<uint32_t>(indices.presentQueueFamily)
		};

		createInfo.setImageSharingMode(vk::SharingMode::eConcurrent)
			.setQueueFamilyIndexCount(static_cast<uint32_t>(queueFamilyIndices.size()))
			.setPQueueFamilyIndices(queueFamilyIndices.data());
	}

	return device.createSwapchainKHR(createInfo);
}

vk::RenderPass VulkanGraphicsPipeline::createRenderPass(
	const vk::Device& device,
	const vk::SurfaceFormatKHR& surfaceFormat,
	const VulkanImage& depthImage)
{
	std::array<vk::AttachmentDescription, 2> attachments = {
		vk::AttachmentDescription()
			.setFormat(surfaceFormat.format)
			.setSamples(vk::SampleCountFlagBits::e1)
			.setLoadOp(vk::AttachmentLoadOp::eClear)
			.setStoreOp(vk::AttachmentStoreOp::eStore)
			.setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
			.setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
			.setInitialLayout(vk::ImageLayout::eUndefined)
			.setFinalLayout(vk::ImageLayout::ePresentSrcKHR),

		vk::AttachmentDescription()
			.setFormat(depthImage[0].format())
			.setSamples(vk::SampleCountFlagBits::e1)
			.setLoadOp(vk::AttachmentLoadOp::eClear)
			.setStoreOp(vk::AttachmentStoreOp::eDontCare)
			.setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
			.setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
			.setInitialLayout(vk::ImageLayout::eUndefined)
			.setFinalLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal)
	};

	vk::AttachmentReference colorAttachmentRef = vk::AttachmentReference()
		.setAttachment(0)
		.setLayout(vk::ImageLayout::eColorAttachmentOptimal);

	vk::AttachmentReference depthAttachmentRef = vk::AttachmentReference()
		.setAttachment(1)
		.setLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal);

	vk::SubpassDependency dependency = vk::SubpassDependency()
		.setSrcSubpass(VK_SUBPASS_EXTERNAL)
		.setDstSubpass(0)
		.setSrcStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
		.setDstStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
		.setDstAccessMask(vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite);

	vk::SubpassDescription subpass = vk::SubpassDescription()
		.setPipelineBindPoint(vk::PipelineBindPoint::eGraphics)
		.setColorAttachmentCount(1)
		.setPColorAttachments(&colorAttachmentRef)
		.setPDepthStencilAttachment(&depthAttachmentRef);

	vk::RenderPassCreateInfo createInfo = vk::RenderPassCreateInfo()
		.setAttachmentCount(static_cast<uint32_t>(attachments.size()))
		.setPAttachments(attachments.data())
		.setDependencyCount(1)
		.setPDependencies(&dependency)
		.setSubpassCount(1)
		.setPSubpasses(&subpass);

	return device.createRenderPass(createInfo);
}

vk::PipelineLayout VulkanGraphicsPipeline::createPipelineLayout(
	const vk::Device& device, 
	const vk::DescriptorSetLayout& descriptorSetLayout)
{
	vk::PipelineLayoutCreateInfo pipelineLayoutCreateInfo = vk::PipelineLayoutCreateInfo()
		.setSetLayoutCount(1)
		.setPSetLayouts(&descriptorSetLayout)
		.setPushConstantRangeCount(0)
		.setPPushConstantRanges(nullptr);

	return device.createPipelineLayout(pipelineLayoutCreateInfo);
}

vk::DescriptorSetLayout VulkanGraphicsPipeline::createDescriptorSetLayout(const vk::Device& device)
{
	std::array<vk::DescriptorSetLayoutBinding, 2> layoutBindings = {
		vk::DescriptorSetLayoutBinding()
			.setBinding(0)
			.setDescriptorType(vk::DescriptorType::eUniformBuffer)
			.setDescriptorCount(1)
			.setStageFlags(vk::ShaderStageFlagBits::eVertex),

		vk::DescriptorSetLayoutBinding()
			.setBinding(1)
			.setDescriptorType(vk::DescriptorType::eCombinedImageSampler)
			.setDescriptorCount(1)
			.setStageFlags(vk::ShaderStageFlagBits::eFragment)
	};

	vk::DescriptorSetLayoutCreateInfo createInfo = vk::DescriptorSetLayoutCreateInfo()
		.setBindingCount(static_cast<uint32_t>(layoutBindings.size()))
		.setPBindings(layoutBindings.data());

	return device.createDescriptorSetLayout(createInfo);
}

vk::DescriptorPool VulkanGraphicsPipeline::createDescriptorPool(const vk::Device& device, uint32_t maxSets)
{
	std::array<vk::DescriptorPoolSize, 2> sizes = {
		vk::DescriptorPoolSize()
			.setDescriptorCount(maxSets)
			.setType(vk::DescriptorType::eUniformBuffer),

		vk::DescriptorPoolSize()
			.setDescriptorCount(maxSets)
			.setType(vk::DescriptorType::eCombinedImageSampler)
	};

	vk::DescriptorPoolCreateInfo createInfo = vk::DescriptorPoolCreateInfo()
		.setPoolSizeCount(static_cast<uint32_t>(sizes.size()))
		.setPPoolSizes(sizes.data())
		.setMaxSets(maxSets);

	return device.createDescriptorPool(createInfo);
}

vk::DescriptorSet VulkanGraphicsPipeline::createDescriptorSet(
	const vk::Device& device,
	const vk::DescriptorPool& descriptorPool,
	const vk::DescriptorSetLayout& descriptorSetLayout)
{
	vk::DescriptorSetAllocateInfo allocInfo = vk::DescriptorSetAllocateInfo()
		.setDescriptorPool(descriptorPool)
		.setDescriptorSetCount(1)
		.setPSetLayouts(&descriptorSetLayout);

	return device.allocateDescriptorSets(allocInfo)[0];
}

vk::Pipeline VulkanGraphicsPipeline::createGraphicsPipeline(
	const vk::Device& device,
	const vk::Extent2D& swapExtent,
	const vk::PipelineLayout& pipelineLayout,
	const vk::RenderPass& renderPass,
	vk::Pipeline oldPipeline)
{
	std::vector<char> vertexShaderCode = loadFile("Shaders/vert.spv");
	std::vector<char> fragmentShaderCode = loadFile("Shaders/frag.spv");

	vk::ShaderModuleCreateInfo shaderCreateInfo = vk::ShaderModuleCreateInfo()
		.setCodeSize(vertexShaderCode.size())
		.setPCode(reinterpret_cast<const uint32_t*>(vertexShaderCode.data()));

	vk::ShaderModule vertexShaderModule = device.createShaderModule(shaderCreateInfo);

	shaderCreateInfo
		.setCodeSize(fragmentShaderCode.size())
		.setPCode(reinterpret_cast<const uint32_t*>(fragmentShaderCode.data()));

	vk::ShaderModule fragmentShaderModule = device.createShaderModule(shaderCreateInfo);

	// TODO: Set shader entry name depending on the quality settings instead of "main".
	std::array<vk::PipelineShaderStageCreateInfo, 2> shaderStageCreateInfos = {
		vk::PipelineShaderStageCreateInfo()
			.setStage(vk::ShaderStageFlagBits::eVertex)
			.setModule(vertexShaderModule)
			.setPName("main"),

		vk::PipelineShaderStageCreateInfo()
			.setStage(vk::ShaderStageFlagBits::eFragment)
			.setModule(fragmentShaderModule)
			.setPName("main")
	};

	std::array<vk::VertexInputBindingDescription, 2> vertexInputBindingDescriptions = {
		vk::VertexInputBindingDescription()
			.setBinding(0)
			.setInputRate(vk::VertexInputRate::eVertex)
			.setStride(static_cast<uint32_t>(Vertex::size())),

		vk::VertexInputBindingDescription()
			.setBinding(1)
			.setInputRate(vk::VertexInputRate::eInstance)
			.setStride(static_cast<uint32_t>(sizeof(glm::mat4)))
	};

	std::array<vk::VertexInputAttributeDescription, 8> vertexInputAttributes = {
		vk::VertexInputAttributeDescription()
			.setBinding(0)
			.setLocation(0)
			.setFormat(vk::Format::eR32G32B32Sfloat)
			.setOffset(static_cast<uint32_t>(Vertex::posOffset())),

		vk::VertexInputAttributeDescription()
			.setBinding(0)
			.setLocation(1)
			.setFormat(vk::Format::eR32G32Sfloat)
			.setOffset(static_cast<uint32_t>(Vertex::uvOffset())),

		vk::VertexInputAttributeDescription()
			.setBinding(0)
			.setLocation(2)
			.setFormat(vk::Format::eR32G32B32Sfloat)
			.setOffset(static_cast<uint32_t>(Vertex::normalOffset())),

		vk::VertexInputAttributeDescription()
			.setBinding(0)
			.setLocation(3)
			.setFormat(vk::Format::eR32G32B32A32Sfloat)
			.setOffset(static_cast<uint32_t>(Vertex::colorOffset())),

		// A mat4 input variable in glsl is considered to be four column vectors that take locations i, i+1, i+2 and i+3.
		vk::VertexInputAttributeDescription()
			.setBinding(1)
			.setLocation(4)
			.setFormat(vk::Format::eR32G32B32A32Sfloat)
			.setOffset(0U),

		vk::VertexInputAttributeDescription()
			.setBinding(1)
			.setLocation(5)
			.setFormat(vk::Format::eR32G32B32A32Sfloat)
			.setOffset(static_cast<uint32_t>(sizeof(glm::vec4))),

		vk::VertexInputAttributeDescription()
			.setBinding(1)
			.setLocation(6)
			.setFormat(vk::Format::eR32G32B32A32Sfloat)
			.setOffset(2 * static_cast<uint32_t>(sizeof(glm::vec4))),

		vk::VertexInputAttributeDescription()
			.setBinding(1)
			.setLocation(7)
			.setFormat(vk::Format::eR32G32B32A32Sfloat)
			.setOffset(3 * static_cast<uint32_t>(sizeof(glm::vec4)))
	};
	
	vk::PipelineVertexInputStateCreateInfo vertexInputInfo = vk::PipelineVertexInputStateCreateInfo()
		.setVertexBindingDescriptionCount(static_cast<uint32_t>(vertexInputBindingDescriptions.size()))
		.setPVertexBindingDescriptions(vertexInputBindingDescriptions.data())
		.setVertexAttributeDescriptionCount(static_cast<uint32_t>(vertexInputAttributes.size()))
		.setPVertexAttributeDescriptions(vertexInputAttributes.data());

	vk::PipelineInputAssemblyStateCreateInfo inputAssembly = vk::PipelineInputAssemblyStateCreateInfo()
		.setTopology(vk::PrimitiveTopology::eTriangleList)
		.setPrimitiveRestartEnable(VK_FALSE);

	vk::Viewport viewport = vk::Viewport()
		.setX(0.f)
		.setY(0.f)
		.setWidth(static_cast<float>(swapExtent.width))
		.setHeight(static_cast<float>(swapExtent.height))
		.setMinDepth(0.f)
		.setMaxDepth(1.f);

	vk::Rect2D scissor = vk::Rect2D()
		.setOffset({ 0, 0 })
		.setExtent(swapExtent);

	vk::PipelineViewportStateCreateInfo viewportState = vk::PipelineViewportStateCreateInfo()
		.setViewportCount(1)
		.setPViewports(&viewport)
		.setScissorCount(1)
		.setPScissors(&scissor);

	vk::PipelineRasterizationStateCreateInfo rasterizer = vk::PipelineRasterizationStateCreateInfo()
		.setDepthClampEnable(VK_FALSE)
		.setRasterizerDiscardEnable(VK_FALSE)
		.setPolygonMode(vk::PolygonMode::eFill)
		.setLineWidth(1.f)
		.setCullMode(vk::CullModeFlagBits::eBack)
		.setFrontFace(vk::FrontFace::eCounterClockwise)
		.setDepthBiasEnable(VK_FALSE);

	vk::PipelineMultisampleStateCreateInfo multisampling = vk::PipelineMultisampleStateCreateInfo()
		.setSampleShadingEnable(VK_FALSE)
		.setRasterizationSamples(vk::SampleCountFlagBits::e1);

	vk::PipelineDepthStencilStateCreateInfo depthStencil = vk::PipelineDepthStencilStateCreateInfo()
		.setDepthTestEnable(VK_TRUE)
		.setDepthWriteEnable(VK_TRUE)
		.setDepthCompareOp(vk::CompareOp::eLess);

	vk::PipelineColorBlendAttachmentState colorBlendAttachment = vk::PipelineColorBlendAttachmentState()
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

	vk::PipelineColorBlendStateCreateInfo colorBlending = vk::PipelineColorBlendStateCreateInfo()
		.setLogicOpEnable(VK_FALSE)
		.setAttachmentCount(1)
		.setPAttachments(&colorBlendAttachment);

	std::array<vk::DynamicState, 1> dynamicStates = { vk::DynamicState::eViewport };

	vk::PipelineDynamicStateCreateInfo dynamicState = vk::PipelineDynamicStateCreateInfo()
		.setDynamicStateCount(static_cast<uint32_t>(dynamicStates.size()))
		.setPDynamicStates(dynamicStates.data());

	vk::GraphicsPipelineCreateInfo createInfo = vk::GraphicsPipelineCreateInfo()
		.setStageCount(static_cast<uint32_t>(shaderStageCreateInfos.size()))
		.setPStages(shaderStageCreateInfos.data())
		.setPVertexInputState(&vertexInputInfo)
		.setPInputAssemblyState(&inputAssembly)
		.setPViewportState(&viewportState)
		.setPRasterizationState(&rasterizer)
		.setPMultisampleState(&multisampling)
		.setPDepthStencilState(&depthStencil)
		.setPColorBlendState(&colorBlending)
		.setPDynamicState(nullptr)
		.setLayout(pipelineLayout)
		.setRenderPass(renderPass)
		.setSubpass(0)
		.setBasePipelineIndex(-1)
		.setBasePipelineHandle(oldPipeline);

	// TODO: Look into the vk::PipelineCache to speed up pipeline creation
	vk::Pipeline graphicsPipeline = device.createGraphicsPipeline(nullptr, createInfo);

	device.destroyShaderModule(fragmentShaderModule);
	device.destroyShaderModule(vertexShaderModule);

	return graphicsPipeline;
}

std::vector<vk::Framebuffer> VulkanGraphicsPipeline::createFramebuffers(
	const vk::Device& device,
	const std::vector<vk::ImageView>& swapchainImageViews,
	const VulkanImage& depthImage,
	const vk::RenderPass& renderPass,
	const vk::Extent2D& swapExtent)
{
	std::vector<vk::Framebuffer> framebuffers;
	framebuffers.reserve(swapchainImageViews.size());

	for (const vk::ImageView& imageView : swapchainImageViews)
	{
		std::array<vk::ImageView, 2> attachments = { imageView, depthImage[0].imageView() };

		vk::FramebufferCreateInfo createInfo = vk::FramebufferCreateInfo()
			.setRenderPass(renderPass)
			.setAttachmentCount(static_cast<uint32_t>(attachments.size()))
			.setPAttachments(attachments.data())
			.setWidth(swapExtent.width)
			.setHeight(swapExtent.height)
			.setLayers(1);

		framebuffers.push_back(device.createFramebuffer(createInfo));
	}

	return std::move(framebuffers);
}