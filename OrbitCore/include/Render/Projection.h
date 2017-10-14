/*! @file Render/Projection.h */

#ifndef RENDER_PROJECTION_H
#define RENDER_PROJECTION_H
#pragma once

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>

#include "Util.h"

namespace Orbit
{
	class Projection
	{
	public:
		enum class Type	{ Perspective, Orthogonal };

		ORBIT_CORE_API explicit Projection(Type type);
		ORBIT_CORE_API ~Projection() = default;

		ORBIT_CORE_API glm::mat4 getMatrix() const;

		ORBIT_CORE_API void switchType(Type newType);

		ORBIT_CORE_API float zNear() const;
		ORBIT_CORE_API float zFar() const;

		ORBIT_CORE_API float fov() const;
		ORBIT_CORE_API float aspectRatio() const;
		
		ORBIT_CORE_API float left() const;
		ORBIT_CORE_API float right() const;
		ORBIT_CORE_API float bottom() const;
		ORBIT_CORE_API float top() const;

		ORBIT_CORE_API void setZNear(float zNear);
		ORBIT_CORE_API void setZFar(float zFar);

		ORBIT_CORE_API void setFoV(float angle);
		ORBIT_CORE_API void setAspectRatio(float aspectRatio);

		ORBIT_CORE_API void setLeft(float left);
		ORBIT_CORE_API void setRight(float right);
		ORBIT_CORE_API void setBottom(float bottom);
		ORBIT_CORE_API void setTop(float top);

	private:
		Type _type;

		float _zNear = 0.f;
		float _zFar = 1.f;

		float _fov = 45.f;
		float _aspectRatio = 1.6f;

		float _left = -1.f;
		float _right = 1.f;
		float _bottom = -1.f;
		float _top = 1.f;
	};
}

#endif //RENDER_PROJECTION_H