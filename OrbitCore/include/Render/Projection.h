/*! @file Render/Projection.h */

#ifndef RENDER_PROJECTION_H
#define RENDER_PROJECTION_H
#pragma once

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>

#include "Util.h"

namespace Orbit
{
	/*!
	@brief Class wrapping different 3D projection types. As it is meant to be "simple", it merges all of them into a single
	API with functions that do nothing unless the type of the projection is chosen correctly, essentially turning it into a
	(albeit simple) state machine.
	*/
	class Projection
	{
	public:
		/*! Simple enumeration showing the allowed types for a projection. */
		enum class Type	{ Perspective, Orthogonal };

		/*!
		@brief Constructor for the class. Takes the inital type of the projection in parameter.
		@param type The initial type of the projection.
		*/
		ORBIT_CORE_API explicit Projection(Type type);

		/*!
		@brief Destructor for the class.
		*/
		ORBIT_CORE_API ~Projection() = default;

		/*!
		@brief Getter for the computed projection matrix.
		@return The computed projection matrix.
		*/
		ORBIT_CORE_API glm::mat4 getMatrix() const;

		/*!
		@brief Updates the object's state to output a (possibly) different type of projection.
		@param newType The new type of the projection.
		*/
		ORBIT_CORE_API void switchType(Type newType);

		/*!
		@brief Getter for the zNear property of the class.
		@return The zNear property of the class.
		*/
		ORBIT_CORE_API float zNear() const;

		/*!
		@brief Getter for the zFar property of the class.
		@return The zFar property of the class.
		*/
		ORBIT_CORE_API float zFar() const;

		/*!
		@brief Getter for the FoV property of the class.
		@return The FoV property of the class.
		*/
		ORBIT_CORE_API float fov() const;

		/*!
		@brief Getter for the aspect ratio property of the class.
		@return The aspect ratio property of the class.
		*/
		ORBIT_CORE_API float aspectRatio() const;
		
		/*!
		@brief Getter for the left bound property of the class.
		@return The left bound property for the class.
		*/
		ORBIT_CORE_API float left() const;

		/*!
		@brief Getter for the right bound property of the class.
		@return The right bound property for the class.
		*/
		ORBIT_CORE_API float right() const;

		/*!
		@brief Getter for the bottom bound property of the class.
		@return The bottom bound property of the class.
		*/
		ORBIT_CORE_API float bottom() const;

		/*!
		@brief Getter for the top bound property of the class.
		@return The top bound property of the class.
		*/
		ORBIT_CORE_API float top() const;

		/*!
		@brief Setter for the zNear property of the class.
		@param zNear The updated zNear value.
		*/
		ORBIT_CORE_API void setZNear(float zNear);

		/*!
		@brief Setter for the zFar property of the class.
		@param zFar The updated zFar value.
		*/
		ORBIT_CORE_API void setZFar(float zFar);

		/*!
		@brief Setter for the FoV property of the class.
		@param angle The new field of view angle, in radians.
		*/
		ORBIT_CORE_API void setFoV(float angle);

		/*!
		@brief Setter for the aspect ratio property of the class.
		@param aspectRatio The new aspect ratio value, taken as width/height.
		*/
		ORBIT_CORE_API void setAspectRatio(float aspectRatio);

		/*!
		@brief Setter for the left bound property of the class.
		@param left The new left boundary value.
		*/
		ORBIT_CORE_API void setLeft(float left);

		/*!
		@brief Setter for the right bound property of the class.
		@param right The new right boundary value.
		*/
		ORBIT_CORE_API void setRight(float right);

		/*!
		@brief Setter for the bottom bound property of the class.
		@param bottom The new bottom boundary value.
		*/
		ORBIT_CORE_API void setBottom(float bottom);

		/*!
		@brief Setter for the top bound property of the class.
		@param top The new top boundary value.
		*/
		ORBIT_CORE_API void setTop(float top);

	private:
		/*! The type of projection applied. */
		Type _type;

		/*! The zNear property. */
		float _zNear = 0.f;
		/*! The zFar property. */
		float _zFar = 1.f;

		/*! The field of view, used only in Type::Perspective. */
		float _fov = 45.f;
		/*! The aspect ratio, used only in Type::Perspective. */
		float _aspectRatio = 1.6f;

		/*! The left bound, used only in Type::Orthogonal. */
		float _left = -1.f;
		/*! The right bound, used only in Type::Orthogonal. */
		float _right = 1.f;
		/*! The bottom bound, used only in Type::Orthogonal. */
		float _bottom = -1.f;
		/*! The top bound, used only in Type::Orthogonal. */
		float _top = 1.f;
	};
}

#endif //RENDER_PROJECTION_H