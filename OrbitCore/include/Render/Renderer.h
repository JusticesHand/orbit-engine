/*! @file Render/Renderer.h */

#ifndef RENDER_RENDERER_H
#define RENDER_RENDERER_H
#pragma once

#include <glm/glm.hpp>

namespace Orbit
{
	class Model;
	enum class RendererAPI : int;

	class Renderer
	{
	public:
		Renderer() = default;
		virtual ~Renderer() = 0;

		virtual void init(void* windowHandle, const glm::ivec2& windowSize) = 0;

		virtual RendererAPI getAPI() const = 0;

		virtual void flagResize(const glm::ivec2& newSize) = 0;

		virtual void queueRender(const Model& model, const glm::mat4& transform) = 0;

		virtual void renderFrame() const = 0;

		virtual void waitDeviceIdle() = 0;
	};

	inline Renderer::~Renderer() = default;
}

#endif //RENDER_RENDERER_H