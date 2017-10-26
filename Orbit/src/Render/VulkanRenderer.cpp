/*! @file Render/VulkanRenderer.cpp */

#include "Render/VulkanRenderer.h"

#include "Render/VulkanBase.h"
#include "Render/VulkanGraphicsPipeline.h"

#include <GLFW/glfw3.h>

#include <iostream>
#include <list>
#include <vector>
#include <set>

#include <Render/Model.h>
#include <Render/Texture.h>

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
	_textureImage.clear();

	// Note that command buffers are destroyed upon command pool destruction when destroying the base.

	_pipeline = nullptr;
	_base = nullptr;
}

void VulkanRenderer::init(const Window* window)
{
	_base = std::make_shared<VulkanBase>(window);
	_pipeline = std::make_shared<VulkanGraphicsPipeline>(_base, window->size());
	
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

	std::vector<vk::DeviceSize> textureDataBlocks;
	std::vector<vk::Extent2D> textureExtents;

	// Create transform buffer (sizes used later on)
	std::vector<vk::DeviceSize> transformBlockSizes = { static_cast<vk::DeviceSize>(sizeof(glm::mat4)) };

	_modelData.reserve(models.size());
	for (const Renderer::ModelCountPair& modelCountPair : models)
	{
		modelDataBlocks.push_back(static_cast<vk::DeviceSize>(modelCountPair.first->getVertices().size() * Vertex::size()));
		modelDataBlocks.push_back(static_cast<vk::DeviceSize>(modelCountPair.first->getIndices().size() * sizeof(uint32_t)));

		if (std::shared_ptr<const Texture> texture = modelCountPair.first->getTexture())
		{
			textureDataBlocks.push_back(texture->data().size());
			textureExtents.push_back(vk::Extent2D{
				static_cast<uint32_t>(texture->size().x),
				static_cast<uint32_t>(texture->size().y)
			});
		}
			
	}

	vk::BufferCreateInfo createInfo = vk::BufferCreateInfo()
		.setUsage(vk::BufferUsageFlagBits::eTransferSrc)
		.setSharingMode(vk::SharingMode::eExclusive);

	VulkanBuffer vertexStagingBuffer{
		_base,
		modelDataBlocks,
		createInfo,
		vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent
	};

	VulkanBuffer textureStagingBuffer{
		_base,
		textureDataBlocks,
		createInfo,
		vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostVisible
	};

	// Update the descriptor pool to 1: dealloc old descriptor sets and 2: allow new allocation of just enough descriptor sets.
	_pipeline->updateDescriptorPool(static_cast<uint32_t>(models.size()));

	size_t modelIndex = 0;
	size_t textureIndex = 0;
	size_t instanceIndex = 0;
	for (const Renderer::ModelCountPair& modelCountPair : models)
	{
		std::shared_ptr<Model> model = modelCountPair.first;

		ModelData modelData;
		modelData.weakModel = modelCountPair.first;
		modelData.vertexIndex = modelIndex++;
		modelData.indicesIndex = modelIndex++;

		modelData.descriptorSet = _pipeline->allocateDescriptorSet();

		if (model->getTexture() != nullptr)
			modelData.textureIndex = textureIndex++;

		modelData.instanceCount = modelCountPair.second;
		modelData.instanceIndex = instanceIndex++;

		_modelData.push_back(modelData);

		transformBlockSizes.push_back(static_cast<vk::DeviceSize>(modelData.instanceCount * sizeof(glm::mat4)));

		vertexStagingBuffer[modelData.vertexIndex].copy(
			model->getVertices().data(),
			static_cast<vk::DeviceSize>(model->getVertices().size() * Vertex::size()));

		vertexStagingBuffer[modelData.indicesIndex].copy(
			model->getIndices().data(),
			static_cast<vk::DeviceSize>(model->getIndices().size() * sizeof(uint32_t)));

		if (modelData.textureIndex != std::numeric_limits<size_t>::max())
		{
			textureStagingBuffer[modelData.textureIndex].copy(
				model->getTexture()->data().data(),
				static_cast<vk::DeviceSize>(model->getTexture()->data().size() * sizeof(uint8_t)));
		}
	}

	// Create transform buffer. Still host coherent and cohesive, since it's going to be overwritten every frame anyways.
	createInfo.setUsage(
		vk::BufferUsageFlagBits::eUniformBuffer |
		vk::BufferUsageFlagBits::eIndirectBuffer |
		vk::BufferUsageFlagBits::eTransferDst);

	_transformBuffer = VulkanBuffer{
		_base,
		transformBlockSizes,
		createInfo,
		vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent
	};

	createInfo.setUsage(
		vk::BufferUsageFlagBits::eVertexBuffer |
		vk::BufferUsageFlagBits::eIndexBuffer |
		vk::BufferUsageFlagBits::eTransferDst);

	_modelBuffer = VulkanBuffer{
		_base,
		modelDataBlocks,
		createInfo,
		vk::MemoryPropertyFlagBits::eDeviceLocal
	};

	vk::ImageCreateInfo imageCreateInfo = vk::ImageCreateInfo()
		.setImageType(vk::ImageType::e2D)
		.setMipLevels(1)
		.setArrayLayers(1)
		.setFormat(vk::Format::eR8G8B8A8Unorm)
		.setTiling(vk::ImageTiling::eOptimal)
		.setInitialLayout(vk::ImageLayout::eUndefined)
		.setUsage(vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled)
		.setSharingMode(vk::SharingMode::eExclusive)
		.setSamples(vk::SampleCountFlagBits::e1);

	_textureImage = VulkanImage{
		_base,
		textureExtents,
		imageCreateInfo,
		vk::MemoryPropertyFlagBits::eDeviceLocal
	};

	// Update the descriptor sets to point to our new buffer (and texture).
	// Buffer info for the viewProjection transform...
	vk::DescriptorBufferInfo bufferInfo = vk::DescriptorBufferInfo()
		.setBuffer(_transformBuffer.buffer())
		.setOffset(0Ui64)
		.setRange(static_cast<vk::DeviceSize>(sizeof(glm::mat4)));

	
	std::vector<vk::WriteDescriptorSet> descriptorWrites;

	// Need to declare image infos here as to not have memory overriden when used in a loop (since everything uses pointers).
	// List ensures no resizing i.e. pointers don't change up.
	std::list<vk::DescriptorImageInfo> imageInfos;
	for (ModelData& modelData : _modelData)
	{
		vk::DescriptorSet& descriptorSet = modelData.descriptorSet;

		descriptorWrites.push_back(vk::WriteDescriptorSet()
			.setDstSet(descriptorSet)
			.setDstBinding(0)
			.setDstArrayElement(0)
			.setDescriptorType(vk::DescriptorType::eUniformBuffer)
			.setDescriptorCount(1)
			.setPBufferInfo(&bufferInfo));

		if (modelData.textureIndex != std::numeric_limits<size_t>::max())
		{
			VulkanImage::Block& imageBlock = _textureImage[modelData.textureIndex];

			imageInfos.push_back(vk::DescriptorImageInfo()
				.setImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal)
				.setImageView(imageBlock.imageView())
				.setSampler(imageBlock.sampler()));

			descriptorWrites.push_back(vk::WriteDescriptorSet()
				.setDstSet(descriptorSet)
				.setDstBinding(1)
				.setDstArrayElement(0)
				.setDescriptorType(vk::DescriptorType::eCombinedImageSampler)
				.setDescriptorCount(1)
				.setPImageInfo(&imageInfos.back()));
		}
	}

	_base->device().updateDescriptorSets(descriptorWrites, nullptr);

	vk::CommandBuffer vertexTransferBuffer = vertexStagingBuffer.transferToBuffer(_modelBuffer);
	vk::CommandBuffer textureTransferBuffer = textureStagingBuffer.transferToImage(_textureImage);
	vk::CommandBuffer textureTransitionBuffer = _textureImage.transitionLayouts(vk::ImageLayout::eShaderReadOnlyOptimal);

	std::vector<vk::CommandBuffer> commands;
	if (vertexTransferBuffer)
		commands.push_back(vertexTransferBuffer);
	if (textureTransferBuffer)
	{
		commands.push_back(textureTransferBuffer);
		commands.push_back(textureTransitionBuffer);
	}

	vk::SubmitInfo submit;
	submit
		.setCommandBufferCount(static_cast<uint32_t>(commands.size()))
		.setPCommandBuffers(commands.data());

	// Create the fence that will be used to synchronize transfer operations.
	vk::Fence transferFence = _base->device().createFence({});

	_base->transferQueue().submit(submit, transferFence);

	_base->device().waitForFences(transferFence, VK_TRUE, std::numeric_limits<uint64_t>::max());
	_base->device().destroyFence(transferFence);

	vertexStagingBuffer.clear();
	textureStagingBuffer.clear();

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
	if (_primaryGraphicsCommandBuffers.empty())
		return;

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
		return createPrimaryCommandBuffers(device, commandPool, pipeline, secondaryCommandBuffersCollection);

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
	const VulkanBuffer& modelBuffer,
	const VulkanBuffer& transformBuffer,
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
	const VulkanBuffer& modelBuffer,
	const VulkanBuffer& transformBuffer,
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
			modelData.descriptorSet,
			nullptr);

		// TODO: Add animation data to buffers and offsets (and shaders, and descriptor sets, etc etc)
		std::array<vk::Buffer, 2> buffers = {
			modelBuffer.buffer(),
			transformBuffer.buffer()
		};

		std::array<vk::DeviceSize, 2> offsets = {
			modelBuffer[modelData.vertexIndex].offset(),
			transformBuffer[modelData.instanceIndex + 1].offset()	// Add 1 since block 0 is the uniform buffer
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