/*! @file Render/VulkanModelRenderer.cpp */

#include "Render/VulkanModelRenderer.h"
#include "Render/VulkanGraphicsPipeline.h"

#include <Render/Model.h>

#include <set>

using namespace Orbit;

void VulkanModelRenderer::init(
	vk::PhysicalDevice physicalDevice, 
	vk::Device device,
	const VulkanGraphicsPipeline& pipeline,
	const vk::CommandPool& graphicsCommandPool,
	const vk::CommandPool& transferCommandPool)
{
	_physicalDevice = physicalDevice;
	_device = device;
	_pipeline = &pipeline;

	_graphicsCommandPool = &graphicsCommandPool;
	_transferCommandPool = &transferCommandPool;

	_graphicsCommandBuffers = createGraphicsCommandBuffers();
	recordCommandBuffers();

	// Semaphore create info is currently empty.
	_imageSemaphore = _device.createSemaphore({});
	_renderSemaphore = _device.createSemaphore({});
}

void VulkanModelRenderer::recreateBuffers(const VulkanGraphicsPipeline* const newPipeline)
{
	_pipeline = newPipeline;

	if (!_graphicsCommandBuffers.empty())
		_device.freeCommandBuffers(*_graphicsCommandPool, _graphicsCommandBuffers);
	_graphicsCommandBuffers.clear();

	_graphicsCommandBuffers = createGraphicsCommandBuffers();
	recordCommandBuffers();
}

void VulkanModelRenderer::loadModels(const std::vector<Renderer::ModelCountPair>& models, vk::Queue transferQueue)
{
	_device.waitIdle();

	for (auto framebufferToCommandBufferPair : _secondaryBufferMap)
		if (!framebufferToCommandBufferPair.second.empty())
			_device.freeCommandBuffers(*_graphicsCommandPool, framebufferToCommandBufferPair.second);
	_secondaryBufferMap.clear();

	_modelData.clear();
	_mainBuffer.clear();
	_transformBuffer.clear();
	// TODO: _animationBuffer.clear();

	std::vector<vk::DeviceSize> modelDataBlocks;
	modelDataBlocks.reserve(2 * models.size());

	for (const Renderer::ModelCountPair& modelCountPair : models)
	{
		modelDataBlocks.push_back(static_cast<vk::DeviceSize>(modelCountPair.first->getVertices().size() * Vertex::size()));
		modelDataBlocks.push_back(static_cast<vk::DeviceSize>(modelCountPair.first->getIndices().size() * sizeof(uint32_t)));
	}

	vk::BufferCreateInfo createInfo;
	createInfo
		.setUsage(vk::BufferUsageFlagBits::eTransferSrc)
		.setSharingMode(vk::SharingMode::eExclusive);

	VulkanMemoryBuffer vertexStagingBuffer{
		_physicalDevice,
		_device,
		modelDataBlocks,
		createInfo,
		vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent
	};

	// Create transform buffer (sizes used later on)
	std::vector<vk::DeviceSize> transformBlockSizes = { static_cast<vk::DeviceSize>(sizeof(glm::mat4)) };

	_modelData.reserve(models.size());
	size_t index = 0;
	size_t instanceIndex = 0;
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

		vertexStagingBuffer.getBlock(modelData.vertexIndex).copy(
			model->getVertices().data(),
			model->getVertices().size() * Vertex::size());

		vertexStagingBuffer.getBlock(modelData.indicesIndex).copy(
			model->getIndices().data(),
			model->getIndices().size() * sizeof(uint32_t));
	}

	// Create transform buffer. Still host coherent and cohesive, since it's going to be overwritten every frame anyways.
	createInfo.setUsage(
		vk::BufferUsageFlagBits::eUniformBuffer | 
		vk::BufferUsageFlagBits::eIndirectBuffer | 
		vk::BufferUsageFlagBits::eTransferDst);

	_transformBuffer = VulkanMemoryBuffer{
		_physicalDevice,
		_device,
		transformBlockSizes,
		createInfo,
		vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent
	};

	// Update the descriptor set to point to our new buffer.
	vk::DescriptorSet descriptorSet = _pipeline->getDescriptorSet();
	vk::DescriptorBufferInfo bufferInfo{ _transformBuffer.buffer(), 0Ui64, static_cast<vk::DeviceSize>(sizeof(glm::mat4)) };
	vk::WriteDescriptorSet descriptorWrite;
	descriptorWrite
		.setDstSet(descriptorSet)
		.setDstBinding(0)
		.setDstArrayElement(0)
		.setDescriptorType(vk::DescriptorType::eUniformBuffer)
		.setDescriptorCount(1)
		.setPBufferInfo(&bufferInfo);

	_device.updateDescriptorSets(descriptorWrite, nullptr);

	createInfo.setUsage(
		vk::BufferUsageFlagBits::eVertexBuffer |
		vk::BufferUsageFlagBits::eIndexBuffer |
		vk::BufferUsageFlagBits::eTransferDst);

	_mainBuffer = VulkanMemoryBuffer{
		_physicalDevice,
		_device,
		modelDataBlocks,
		createInfo,
		vk::MemoryPropertyFlagBits::eDeviceLocal
	};

	vk::CommandBuffer vertexTransferBuffer = vertexStagingBuffer.transferToBuffer(_mainBuffer, *_transferCommandPool);

	//vk::Buffer textureStagingBuffer;
	//vk::DeviceMemory textureStagingBufferMemory;
	//vk::CommandBuffer textureTransferBuffer = buildTextureTransferCommand(models, textureStagingBuffer, textureStagingBufferMemory);

	std::array<vk::CommandBuffer, 1> commands = { vertexTransferBuffer };
	//std::array<vk::CommandBuffer, 2> commands = { vertexTransferBuffer, textureTransferBuffer };
	
	vk::SubmitInfo submit;
	submit
		.setCommandBufferCount(static_cast<uint32_t>(commands.size()))
		.setPCommandBuffers(commands.data());

	// Create the fence that will be used to synchronize transfer operations.
	vk::Fence transferFence = _device.createFence({}); // Initially the fence is created in the unsignalled state

	transferQueue.submit(submit, transferFence);

	_device.waitForFences(transferFence, VK_TRUE, std::numeric_limits<uint64_t>::max());

	_device.destroyFence(transferFence);

	vertexStagingBuffer.clear();
	//_device.destroyBuffer(textureStagingBuffer);
	//_device.freeMemory(textureStagingBufferMemory);

	recordCommandBuffers();
}

void VulkanModelRenderer::setupViewProjection(const glm::mat4& viewProjectionTransform)
{
	_transformBuffer.getBlock(0).copy(&viewProjectionTransform, static_cast<vk::DeviceSize>(sizeof(glm::mat4)));
}

void VulkanModelRenderer::updateTransforms(const std::vector<Renderer::ModelTransformsPair>& modelTransforms)
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

void VulkanModelRenderer::renderFrame(vk::Queue graphicsQueue, vk::Queue presentQueue)
{
	_device.waitIdle();

	vk::SwapchainKHR swapchain = _pipeline->getSwapchain();
	auto imageResult = _device.acquireNextImageKHR(swapchain, std::numeric_limits<uint64_t>::max(), _imageSemaphore, nullptr);
	
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
		.setPCommandBuffers(&_graphicsCommandBuffers[imageIndex])
		.setSignalSemaphoreCount(1)
		.setPSignalSemaphores(&_renderSemaphore);

	graphicsQueue.submit(submitInfos, nullptr);

	vk::PresentInfoKHR presentInfo;
	presentInfo
		.setWaitSemaphoreCount(1)
		.setPWaitSemaphores(&_renderSemaphore)
		.setSwapchainCount(1)
		.setPSwapchains(&swapchain)
		.setPImageIndices(&imageIndex);

	presentQueue.presentKHR(presentInfo);
}

void VulkanModelRenderer::cleanup()
{
	_device.destroySemaphore(_renderSemaphore);
	_device.destroySemaphore(_imageSemaphore);

	_mainBuffer.clear();
	_transformBuffer.clear();
	_animationBuffer.clear();
}

std::vector<vk::CommandBuffer> VulkanModelRenderer::createGraphicsCommandBuffers()
{
	const std::vector<vk::Framebuffer>& framebuffers = _pipeline->getFramebuffers();

	vk::CommandBufferAllocateInfo allocInfo;
	allocInfo
		.setCommandPool(*_graphicsCommandPool)
		.setLevel(vk::CommandBufferLevel::ePrimary)
		.setCommandBufferCount(static_cast<uint32_t>(framebuffers.size()));

	return _device.allocateCommandBuffers(allocInfo);
}

void VulkanModelRenderer::recordCommandBuffers()
{
	const std::vector<vk::Framebuffer>& framebuffers = _pipeline->getFramebuffers();

	for (size_t i = 0; i < _graphicsCommandBuffers.size(); i++)
	{
		const vk::Framebuffer& framebuffer = framebuffers[i];

		// Construct all secondary buffers for this framebuffer.
		_secondaryBufferMap[framebuffer] = recordSecondaryBuffers(framebuffer);

		vk::CommandBuffer& primaryBuffer = _graphicsCommandBuffers[i];

		// Implicitly build begin info with necessary flag.
		primaryBuffer.begin({ vk::CommandBufferUsageFlagBits::eSimultaneousUse });

		std::array<vk::ClearValue, 2> clearValues;
		clearValues[0].setColor(std::array<float, 4>{ 0.f, 0.f, 0.f, 0.f });
		clearValues[1].setDepthStencil({ 1.f, 0 });

		vk::Rect2D renderArea;
		renderArea
			.setOffset({ 0, 0 })
			.setExtent(_pipeline->getSwapExtent());

		vk::RenderPassBeginInfo renderPassBeginInfo;
		renderPassBeginInfo
			.setRenderPass(_pipeline->getRenderPass())
			.setFramebuffer(framebuffer)
			.setRenderArea(renderArea)
			.setClearValueCount(static_cast<uint32_t>(clearValues.size()))
			.setPClearValues(clearValues.data());

		primaryBuffer.beginRenderPass(renderPassBeginInfo, vk::SubpassContents::eSecondaryCommandBuffers);

		std::vector<vk::CommandBuffer> secondaryBuffers = _secondaryBufferMap.at(framebuffer);
		if (!secondaryBuffers.empty())
		{
			// Execute the stored secondary command buffers. These handle the actual model rendering operations.
			primaryBuffer.executeCommands(secondaryBuffers);
		}

		primaryBuffer.endRenderPass();

		primaryBuffer.end();
	}
}

std::vector<vk::CommandBuffer> VulkanModelRenderer::recordSecondaryBuffers(const vk::Framebuffer& framebuffer)
{
	if (_modelData.empty())
		return std::vector<vk::CommandBuffer>();

	vk::CommandBufferAllocateInfo allocInfo;
	allocInfo
		.setCommandBufferCount(static_cast<uint32_t>(_modelData.size()))
		.setCommandPool(*_graphicsCommandPool)
		.setLevel(vk::CommandBufferLevel::eSecondary);

	std::vector<vk::CommandBuffer> commandBuffers = _device.allocateCommandBuffers(allocInfo);

	for (size_t i = 0; i < commandBuffers.size(); i++)
	{
		vk::CommandBuffer& commandBuffer = commandBuffers[i];
		const ModelData& modelData = _modelData[i];

		std::shared_ptr<Model> model = modelData.weakModel.lock();
		if (!model)
			throw std::runtime_error("Attempted to render an unloaded model!");

		vk::CommandBufferInheritanceInfo inheritanceInfo{ _pipeline->getRenderPass(), 0, framebuffer };

		vk::CommandBufferBeginInfo beginInfo{
			vk::CommandBufferUsageFlagBits::eSimultaneousUse | 
			vk::CommandBufferUsageFlagBits::eRenderPassContinue,
			&inheritanceInfo };

		commandBuffer.begin(beginInfo);

		commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, _pipeline->getGraphicsPipeline());

		commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, _pipeline->getPipelineLayout(), 0U, _pipeline->getDescriptorSet(), nullptr);

		commandBuffer.bindVertexBuffers(0, _mainBuffer.buffer(), _mainBuffer.getBlock(modelData.vertexIndex).offset());
		commandBuffer.bindVertexBuffers(1, _transformBuffer.buffer(), _transformBuffer.getBlock(modelData.instanceIndex + 1).offset());
		commandBuffer.bindIndexBuffer(_mainBuffer.buffer(), _mainBuffer.getBlock(modelData.indicesIndex).offset(), vk::IndexType::eUint32);

		

		// TODO: Bind transform, animation data here

		commandBuffer.drawIndexed(
			static_cast<uint32_t>(model->getIndices().size()),
			static_cast<uint32_t>(modelData.instanceCount),
			0,
			0,
			0);

		commandBuffer.end();
	}

	return commandBuffers;
}