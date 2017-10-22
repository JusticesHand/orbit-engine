/*! @file Render/VulkanRenderer.cpp */

#include "Render/VulkanRenderer.h"

#include "Render/VulkanBase.h"
#include "Render/VulkanGraphicsPipeline.h"

#include <GLFW/glfw3.h>

#include <iostream>
#include <vector>
#include <set>

#include <Render/Model.h>

#include "Input/Window.h"

using namespace Orbit;

VulkanRenderer::~VulkanRenderer()
{
	if (!_base)
		return;

	if (_base->device())
		waitDeviceIdle();

	//Buffer/stuff clearing
	_base->device().destroySemaphore(_renderSemaphore);
	_base->device().destroySemaphore(_imageSemaphore);

	_modelBuffer.clear();
	_transformBuffer.clear();
	_animationBuffer.clear();

	// Note that command buffers are destroyed upon command pool destruction when destroying the base.

	_pipeline = nullptr;
	_base = nullptr;
}

void VulkanRenderer::init(const Window* window)
{
	_base = std::make_shared<VulkanBase>(window);
	_pipeline = std::make_shared<VulkanGraphicsPipeline>(_base, window->size());
	
	_secondaryGraphicsCommandBuffers.resize(_pipeline->framebuffers().size());
	_primaryGraphicsCommandBuffers = createPrimaryCommandBuffers(
		_base->device(),
		_base->graphicsCommandPool(),
		*_pipeline,
		_secondaryGraphicsCommandBuffers);

	_renderSemaphore = _base->device().createSemaphore({});
	_imageSemaphore = _base->device().createSemaphore({});
}

RendererAPI VulkanRenderer::getAPI() const
{
	return RendererAPI::Vulkan;
}

void VulkanRenderer::flagResize(const glm::ivec2& newSize)
{
	_base->presentQueue().waitIdle();
	_pipeline->resize(newSize);
	
	destroySecondaryBuffers(_base->device(), _base->graphicsCommandPool(), _secondaryGraphicsCommandBuffers);
	createAllSecondaryCommandBuffers(
		_base->device(),
		_base->graphicsCommandPool(),
		_modelBuffer,
		_transformBuffer,
		_modelData, 
		*_pipeline);

	if (!_primaryGraphicsCommandBuffers.empty())
		_base->device().freeCommandBuffers(_base->graphicsCommandPool(), _primaryGraphicsCommandBuffers);
	_primaryGraphicsCommandBuffers.clear();

	_primaryGraphicsCommandBuffers = createPrimaryCommandBuffers(
		_base->device(),
		_base->graphicsCommandPool(),
		*_pipeline,
		_secondaryGraphicsCommandBuffers);
}

void VulkanRenderer::loadModels(const std::vector<ModelCountPair>& models)
{
	waitDeviceIdle();

	_modelData.clear();
	_modelBuffer.clear();
	_transformBuffer.clear();
	//_animationBuffer.clear();

	std::vector<vk::DeviceSize> modelDataBlocks;
	modelDataBlocks.reserve(2 * models.size());

	// Create transform buffer (sizes used later on)
	std::vector<vk::DeviceSize> transformBlockSizes = { static_cast<vk::DeviceSize>(sizeof(glm::mat4)) };

	_modelData.reserve(models.size());
	size_t index = 0;
	size_t instanceIndex = 0;

	for (const Renderer::ModelCountPair& modelCountPair : models)
	{
		modelDataBlocks.push_back(static_cast<vk::DeviceSize>(modelCountPair.first->getVertices().size() * Vertex::size()));
		modelDataBlocks.push_back(static_cast<vk::DeviceSize>(modelCountPair.first->getIndices().size() * sizeof(uint32_t)));
	}

	vk::BufferCreateInfo createInfo = vk::BufferCreateInfo()
		.setUsage(vk::BufferUsageFlagBits::eTransferSrc)
		.setSharingMode(vk::SharingMode::eExclusive);

	VulkanMemoryBuffer vertexStagingBuffer{
		_base,
		modelDataBlocks,
		createInfo,
		vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent
	};

	for (const Renderer::ModelCountPair& modelCountPair : models)
	{
		std::shared_ptr<Model> model = modelCountPair.first;

		ModelData modelData;
		modelData.weakModel = modelCountPair.first;
		modelData.vertexIndex = index++;
		modelData.indicesIndex = index++;

		modelData.instanceCount = modelCountPair.second;
		modelData.instanceIndex = instanceIndex++;

		_modelData.push_back(modelData);

		transformBlockSizes.push_back(static_cast<vk::DeviceSize>(modelData.instanceCount * sizeof(glm::mat4)));

		vertexStagingBuffer[modelData.vertexIndex].copy(
			model->getVertices().data(),
			model->getVertices().size() * Vertex::size());

		vertexStagingBuffer[modelData.indicesIndex].copy(
			model->getIndices().data(),
			model->getIndices().size() * sizeof(uint32_t));
	}

	// Create transform buffer. Still host coherent and cohesive, since it's going to be overwritten every frame anyways.
	createInfo.setUsage(
		vk::BufferUsageFlagBits::eUniformBuffer |
		vk::BufferUsageFlagBits::eIndirectBuffer |
		vk::BufferUsageFlagBits::eTransferDst);

	_transformBuffer = VulkanMemoryBuffer{
		_base,
		transformBlockSizes,
		createInfo,
		vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent
	};

	// Update the descriptor set to point to our new buffer.
	vk::DescriptorSet descriptorSet = _pipeline->descriptorSet();
	vk::DescriptorBufferInfo bufferInfo{ _transformBuffer.buffer(), 0Ui64, static_cast<vk::DeviceSize>(sizeof(glm::mat4)) };
	vk::WriteDescriptorSet descriptorWrite;
	descriptorWrite
		.setDstSet(descriptorSet)
		.setDstBinding(0)
		.setDstArrayElement(0)
		.setDescriptorType(vk::DescriptorType::eUniformBuffer)
		.setDescriptorCount(1)
		.setPBufferInfo(&bufferInfo);

	_base->device().updateDescriptorSets(descriptorWrite, nullptr);

	createInfo.setUsage(
		vk::BufferUsageFlagBits::eVertexBuffer |
		vk::BufferUsageFlagBits::eIndexBuffer |
		vk::BufferUsageFlagBits::eTransferDst);

	_modelBuffer = VulkanMemoryBuffer{
		_base,
		modelDataBlocks,
		createInfo,
		vk::MemoryPropertyFlagBits::eDeviceLocal
	};

	vk::CommandBuffer vertexTransferBuffer = vertexStagingBuffer.transferToBuffer(_modelBuffer);

	//vk::Buffer textureStagingBuffer;
	//vk::DeviceMemory textureStagingBufferMemory;
	//vk::CommandBuffer textureTransferBuffer = buildTextureTransferCommand(models, textureStagingBuffer, textureStagingBufferMemory);

	auto commands = make_array<vk::CommandBuffer>(vertexTransferBuffer); //, textureTransferBuffer

	vk::SubmitInfo submit;
	submit
		.setCommandBufferCount(static_cast<uint32_t>(commands.size()))
		.setPCommandBuffers(commands.data());

	// Create the fence that will be used to synchronize transfer operations.
	vk::Fence transferFence = _base->device().createFence({}); // Initially the fence is created in the unsignalled state

	_base->transferQueue().submit(submit, transferFence);

	_base->device().waitForFences(transferFence, VK_TRUE, std::numeric_limits<uint64_t>::max());

	_base->device().destroyFence(transferFence);

	vertexStagingBuffer.clear();
	//_device.destroyBuffer(textureStagingBuffer);
	//_device.freeMemory(textureStagingBufferMemory);

	destroySecondaryBuffers(
		_base->device(),
		_base->graphicsCommandPool(),
		_secondaryGraphicsCommandBuffers);

	_secondaryGraphicsCommandBuffers = createAllSecondaryCommandBuffers(
		_base->device(),
		_base->graphicsCommandPool(),
		_modelBuffer,
		_transformBuffer,
		_modelData, 
		*_pipeline);

	_primaryGraphicsCommandBuffers = createPrimaryCommandBuffers(
		_base->device(),
		_base->graphicsCommandPool(),
		*_pipeline, 
		_secondaryGraphicsCommandBuffers,
		std::move(_primaryGraphicsCommandBuffers));
}

void VulkanRenderer::setupViewProjection(const glm::mat4& view, const glm::mat4& projection)
{
	// Flip the middle y coordinate to flip the matrix around (since vulkan is flipped on that coordinate vs OGL).
	glm::mat4 flippedProjection = projection;
	flippedProjection[1][1] *= -1;
	glm::mat4 viewProjection = flippedProjection * view;
	_transformBuffer[0].copy(&viewProjection, static_cast<vk::DeviceSize>(sizeof(glm::mat4)));
}

void VulkanRenderer::queueRender(const std::vector<ModelTransformsPair>& modelTransforms)
{
	// _modelData (loaded by loadModel()) is assumed to be in the same order as these transforms. Same goes for instances.
	// They are intended to be traversed by visitors in the tree - as the tree's general state is assumed to not change without
	// re-recording command buffers (and rebuilding GPU state) this is a non-issue.
	// This changes every frame.

	if (modelTransforms.size() != _modelData.size())
		throw std::runtime_error("Renderer is in a weird state!");

	for (size_t i = 0; i < modelTransforms.size(); i++)
	{
		const Renderer::ModelTransformsPair& modelTransform = modelTransforms[i];
		const ModelData& modelData = _modelData[i];

		_transformBuffer.getBlock(i + 1).copy(
			modelTransform.second.data(),
			static_cast<vk::DeviceSize>(modelTransform.second.size() * sizeof(glm::mat4)));
	}
}

void VulkanRenderer::renderFrame()
{
	waitDeviceIdle();

	vk::SwapchainKHR swapchain = _pipeline->swapchain();
	auto imageResult = _base->device().acquireNextImageKHR(swapchain, std::numeric_limits<uint64_t>::max(), _imageSemaphore, nullptr);

	if (imageResult.result != vk::Result::eSuccess)
		throw std::runtime_error("Could not acquire next image!");

	uint32_t imageIndex = imageResult.value;

	vk::PipelineStageFlags waitStages = vk::PipelineStageFlagBits::eColorAttachmentOutput;

	std::array<vk::SubmitInfo, 1> submitInfos;
	submitInfos[0]
		.setWaitSemaphoreCount(1)
		.setPWaitSemaphores(&_imageSemaphore)
		.setPWaitDstStageMask(&waitStages)
		.setCommandBufferCount(1)
		.setPCommandBuffers(&_primaryGraphicsCommandBuffers[imageIndex])
		.setSignalSemaphoreCount(1)
		.setPSignalSemaphores(&_renderSemaphore);

	_base->graphicsQueue().submit(submitInfos, nullptr);

	vk::PresentInfoKHR presentInfo;
	presentInfo
		.setWaitSemaphoreCount(1)
		.setPWaitSemaphores(&_renderSemaphore)
		.setSwapchainCount(1)
		.setPSwapchains(&swapchain)
		.setPImageIndices(&imageIndex);

	_base->presentQueue().presentKHR(presentInfo);
}

void VulkanRenderer::waitDeviceIdle()
{
	_base->device().waitIdle();
}

std::vector<vk::CommandBuffer> VulkanRenderer::createPrimaryCommandBuffers(
	const vk::Device& device,
	const vk::CommandPool& commandPool,
	const VulkanGraphicsPipeline& pipeline,
	const std::vector<std::vector<vk::CommandBuffer>>& secondaryCommandBuffersCollection)
{
	vk::CommandBufferAllocateInfo allocInfo = vk::CommandBufferAllocateInfo()
		.setCommandPool(commandPool)
		.setLevel(vk::CommandBufferLevel::ePrimary)
		.setCommandBufferCount(static_cast<uint32_t>(pipeline.framebuffers().size()));

	return createPrimaryCommandBuffers(
		device, 
		commandPool, 
		pipeline, 
		secondaryCommandBuffersCollection,
		device.allocateCommandBuffers(allocInfo));
}

std::vector<vk::CommandBuffer> VulkanRenderer::createPrimaryCommandBuffers(
	const vk::Device& device,
	const vk::CommandPool& commandPool,
	const VulkanGraphicsPipeline& pipeline,
	const std::vector<std::vector<vk::CommandBuffer>>& secondaryCommandBuffersCollection,
	std::vector<vk::CommandBuffer>&& oldBuffers)
{
	std::vector<vk::CommandBuffer> commandBuffers = std::move(oldBuffers);
	const std::vector<vk::Framebuffer>& framebuffers = pipeline.framebuffers();

	if (commandBuffers.size() != framebuffers.size())
		throw std::runtime_error("Renderer is in a weird state!");

	for (size_t i = 0; i < commandBuffers.size(); i++)
	{
		vk::CommandBuffer& commandBuffer = commandBuffers[i];
		const vk::Framebuffer& framebuffer = framebuffers[i];
		const std::vector<vk::CommandBuffer>& secondaryCommandBuffers = secondaryCommandBuffersCollection[i];

		commandBuffer.begin(vk::CommandBufferBeginInfo()
			.setFlags(vk::CommandBufferUsageFlagBits::eSimultaneousUse));

		std::array<vk::ClearValue, 2> clearValues = {
			vk::ClearValue().setColor(std::array<float, 4>{ 0.f, 0.f, 0.f, 0.f }),
			vk::ClearValue().setDepthStencil(vk::ClearDepthStencilValue{ 1.f, 0 })
		};

		vk::Rect2D renderArea = vk::Rect2D()
			.setOffset(vk::Offset2D{ 0, 0 })
			.setExtent(pipeline.swapExtent());

		vk::RenderPassBeginInfo renderPassBeginInfo = vk::RenderPassBeginInfo()
			.setRenderPass(pipeline.renderPass())
			.setFramebuffer(framebuffer)
			.setRenderArea(renderArea)
			.setClearValueCount(static_cast<uint32_t>(clearValues.size()))
			.setPClearValues(clearValues.data());

		commandBuffer.beginRenderPass(renderPassBeginInfo, vk::SubpassContents::eSecondaryCommandBuffers);

		if (!secondaryCommandBuffers.empty())
			commandBuffer.executeCommands(secondaryCommandBuffers);

		commandBuffer.endRenderPass();
		commandBuffer.end();
	}

	return commandBuffers;
}

void VulkanRenderer::destroySecondaryBuffers(
	const vk::Device& device,
	const vk::CommandPool& commandPool,
	std::vector<std::vector<vk::CommandBuffer>>& secondaryBuffers)
{
	for (std::vector<vk::CommandBuffer>& secondaryBuffer : secondaryBuffers)
		if (!secondaryBuffer.empty())
			device.freeCommandBuffers(commandPool, secondaryBuffer);
	secondaryBuffers.clear();
}

std::vector<std::vector<vk::CommandBuffer>> VulkanRenderer::createAllSecondaryCommandBuffers(
	const vk::Device& device,
	const vk::CommandPool& commandPool,
	const VulkanMemoryBuffer& modelBuffer,
	const VulkanMemoryBuffer& transformBuffer,
	const std::vector<ModelData>& allModelData,
	const VulkanGraphicsPipeline& pipeline)
{
	std::vector<std::vector<vk::CommandBuffer>> secondaryCommandBuffers;

	const std::vector<vk::Framebuffer>& framebuffers = pipeline.framebuffers();
	secondaryCommandBuffers.reserve(framebuffers.size());
	for (const vk::Framebuffer& framebuffer : framebuffers)
		secondaryCommandBuffers.push_back(createSecondaryCommandBuffers(
			device,
			commandPool,
			modelBuffer,
			transformBuffer,
			allModelData, 
			pipeline, 
			framebuffer));

	return secondaryCommandBuffers;
}

std::vector<vk::CommandBuffer> VulkanRenderer::createSecondaryCommandBuffers(
	const vk::Device& device,
	const vk::CommandPool& commandPool,
	const VulkanMemoryBuffer& modelBuffer,
	const VulkanMemoryBuffer& transformBuffer,
	const std::vector<ModelData>& allModelData,
	const VulkanGraphicsPipeline& pipeline,
	const vk::Framebuffer& framebuffer)
{
	if (allModelData.empty())
		return std::vector<vk::CommandBuffer>();

	vk::CommandBufferAllocateInfo allocInfo = vk::CommandBufferAllocateInfo()
		.setCommandBufferCount(static_cast<uint32_t>(allModelData.size()))
		.setCommandPool(commandPool)
		.setLevel(vk::CommandBufferLevel::eSecondary);

	std::vector<vk::CommandBuffer> secondaryBuffers = device.allocateCommandBuffers(allocInfo);

	for (size_t i = 0; i < secondaryBuffers.size(); i++)
	{
		vk::CommandBuffer& secondaryBuffer = secondaryBuffers[i];
		const ModelData& modelData = allModelData[i];

		std::shared_ptr<Model> model = modelData.weakModel.lock();
		if (!model)
			throw std::runtime_error("Attempted to render an unloaded model!");

		vk::CommandBufferInheritanceInfo inheritanceInfo = vk::CommandBufferInheritanceInfo()
			.setRenderPass(pipeline.renderPass())
			.setSubpass(0)
			.setFramebuffer(framebuffer);

		secondaryBuffer.begin(vk::CommandBufferBeginInfo()
			.setFlags(vk::CommandBufferUsageFlagBits::eSimultaneousUse | vk::CommandBufferUsageFlagBits::eRenderPassContinue)
			.setPInheritanceInfo(&inheritanceInfo));

		secondaryBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline.graphicsPipeline());
		secondaryBuffer.bindDescriptorSets(
			vk::PipelineBindPoint::eGraphics, 
			pipeline.pipelineLayout(),
			0U, 
			pipeline.descriptorSet(),
			nullptr);

		// TODO: Add animation data to buffers and offsets (and shaders, and descriptor sets, etc etc)
		std::array<vk::Buffer, 2> buffers = {
			modelBuffer.buffer(),
			transformBuffer.buffer()
		};

		std::array<vk::DeviceSize, 2> offsets = {
			modelBuffer[modelData.vertexIndex].offset(),
			transformBuffer[modelData.instanceIndex + 1].offset()
		};

		secondaryBuffer.bindVertexBuffers(0, buffers, offsets);
		secondaryBuffer.bindIndexBuffer(modelBuffer.buffer(), modelBuffer[modelData.indicesIndex].offset(), vk::IndexType::eUint32);

		secondaryBuffer.drawIndexed(
			static_cast<uint32_t>(model->getIndices().size()),
			static_cast<uint32_t>(modelData.instanceCount),
			0,
			0,
			0);

		secondaryBuffer.end();
	}

	return secondaryBuffers;
}