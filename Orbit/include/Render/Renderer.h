/*! @file Render/Renderer.h */

#ifndef RENDER_RENDERER_H
#define RENDER_RENDERER_H
#pragma once

#include <glm/glm.hpp>
#include <memory>
#include <vector>

namespace Orbit
{
	class Model;

	/*!
	@brief Definition of the available API types in the system. Note that some aren't available for some platforms
	(read: DirectX is Windows-only).
	*/
	enum class RendererAPI : int
	{
		Vulkan, DirectX, OpenGL
	};

	/*!
	@brief Renderer base class for the engine. Abstracts rendering operations in a usable base class
	for general rendering requirements.
	*/
	class Renderer
	{
	public:
		/*!
		@brief Default constructor for the class.
		*/
		Renderer() = default;

		/*!
		@brief Destructor for the class. Pure virtual as the class itself is pure virtual.
		*/
		virtual ~Renderer() = 0;

		/*!
		@brief Function intended to initialize the renderer.
		@param windowHandle The window's handle.
		@param windowSize The window's size.
		*/
		virtual void init(void* windowHandle, const glm::ivec2& windowSize) = 0;

		/*!
		@brief Returns the renderer's API type. Useful when the framework requests an API type to set
		itself up (see GLFW).
		@return The API used by the renderer.
		*/
		virtual RendererAPI getAPI() const = 0;

		/*!
		@brief Flags that the renderer has to resize. Whether or not it does so immediately is up to
		implementation.
		@param newSize The new size of the window.
		*/
		virtual void flagResize(const glm::ivec2& newSize) = 0;

		/*! Pair of models and their counts in the composite tree. */
		using ModelCountPair = std::pair<std::shared_ptr<Model>, size_t>;

		/*!
		@brief Loads the models in parameter. Uses ModelCountPair in order to allow the renderer any
		optimizations in memory (like reserving buffers for transformations, for instance).
		@param modelCounts A collection containing models and their counts.
		*/
		virtual void loadModels(const std::vector<ModelCountPair>& modelCounts) = 0;

		/*!
		@brief Sets up the view/projection matrices in the renderer. Ideally, information would be
		passed along to device memory in order to be used in rendering.
		@param view The view matrix.
		@param projection The projection matrix.
		*/
		virtual void setupViewProjection(const glm::mat4& view, const glm::mat4& projection) = 0;

		/*! Pair of models and their transforms. Useful when passing along transform data for models. */
		using ModelTransformsPair = std::pair<std::shared_ptr<Model>, std::vector<glm::mat4>>;

		/*!
		@brief Prepares a frame to be rendered, with the models in parameter taking the transforms in parameter.
		@param modelTransforms The models and their transformations.
		*/
		virtual void queueRender(const std::vector<ModelTransformsPair>& modelTransforms) = 0;

		/*!
		@brief Actually renders a frame. Whether or not this really happens (think OpenGL 1.2 immediate mode) is
		up to implementation.
		*/
		virtual void renderFrame() = 0;

		/*!
		@brief Waits for the rendering device to be idle. Serves as high-level synchronization.
		*/
		virtual void waitDeviceIdle() = 0;
	};

	inline Renderer::~Renderer() = default;
}

#endif //RENDER_RENDERER_H