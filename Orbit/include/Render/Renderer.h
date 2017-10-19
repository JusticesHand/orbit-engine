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
	enum class RendererAPI : int;

	/*!
	@brief Renderer base class for the 
	*/
	class Renderer
	{
	public:
		Renderer() = default;
		virtual ~Renderer() = 0;

		virtual void init(void* windowHandle, const glm::ivec2& windowSize) = 0;

		virtual RendererAPI getAPI() const = 0;

		virtual void flagResize(const glm::ivec2& newSize) = 0;

		using ModelCountPair = std::pair<std::shared_ptr<Model>, size_t>;
		virtual void loadModels(const std::vector<ModelCountPair>& modelCounts) = 0;

		virtual void setupViewProjection(const glm::mat4& view, const glm::mat4& projection) = 0;

		using ModelTransformsPair = std::pair<std::shared_ptr<Model>, std::vector<glm::mat4>>;
		virtual void queueRender(const std::vector<ModelTransformsPair>& modelTransforms) = 0;

		virtual void renderFrame() = 0;

		virtual void waitDeviceIdle() = 0;
	};

	inline Renderer::~Renderer() = default;
}

#endif //RENDER_RENDERER_H