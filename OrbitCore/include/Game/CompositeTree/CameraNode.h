/*! @file Game/CompositeTree/CameraNode.h */

#ifndef GAME_COMPOSITETREE_CAMERANODE_H
#define GAME_COMPOSITETREE_CAMERANODE_H
#pragma once

#include "Node.h"
#include "Util.h"

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>

namespace Orbit
{
	/*!
	@brief Specialization of the Node class to offer 3D camera functionality, such as direction and up and computation
	of view matrices.
	*/
	class CameraNode final : public Node
	{
	public:
		/*!
		@brief Constructor for a CameraNode.
		*/
		ORBIT_CORE_API explicit CameraNode();

		/*!
		@brief Destructor for a CameraNode.
		*/
		ORBIT_CORE_API virtual ~CameraNode() = default;
		
		CameraNode(const CameraNode&) = delete;
		CameraNode& operator=(const CameraNode&) = delete;

		/*!
		@brief Move constructor for a CameraNode.
		@see Orbit::Node::Node(Node&&)
		@param rhs The node to move.
		*/
		ORBIT_CORE_API CameraNode(CameraNode&& rhs);

		/*!
		@brief Move assignment operator for a CameraNode. Calls the base class's assignment operator.
		@see Orbit::Node::operator=(CameraNode&&)
		@param rhs The node to move.
		@return A reference to this.
		*/
		ORBIT_CORE_API CameraNode& operator=(CameraNode&& rhs);

		/*!
		@brief Implements the visitor design pattern for the node.
		@param visitor The visitor to accept.
		*/
		ORBIT_CORE_API void acceptVisitor(Visitor* visitor) override;

		/*!
		@brief Throws an exception, as cloning a camera is not meant to be allowed.
		@throw std::runtime_error Throws every time.
		*/
		ORBIT_CORE_API std::shared_ptr<Node> clone() const override;

		/*!
		@brief Does nothing.
		@param elapsedTime The elapsed time since the last update cycle.
		*/
		ORBIT_CORE_API void update(std::chrono::nanoseconds elapsedTime) override;

		/*!
		@brief Computes the view matrix taking the position, direction and up direction in consideration. Note that the view
		matrix is assumed to be used in an OpenGL context, where the coordinate system's origin is in the bottom-left.
		@note Behaviour is undefined if the camera's up direction and facing directions are linear combinations of one another.
		@return The view matrix computed with the camera's parameters.
		*/
		ORBIT_CORE_API glm::mat4 getViewMatrix() const;

		/*!
		@brief Getter for the camera's direction.
		@return The camera's direction.
		*/
		ORBIT_CORE_API glm::vec3 direction() const;

		/*!
		@brief Getter for the camera's up direction.
		@return The camera's up direction.
		*/
		ORBIT_CORE_API glm::vec3 up() const;

		/*!
		@brief Setter for the camera's direction.
		@param direction The new direction for the camera.
		*/
		ORBIT_CORE_API void setDirection(const glm::vec3& direction);

		/*!
		@brief Setter for the camera's up position.
		@param up The new up direction for the camera.
		*/
		ORBIT_CORE_API void setUp(const glm::vec3& up);

	private:
		/*! The direction of the camera. */
		glm::vec3 _direction{ 0, 0, -1 };
		/*! The up direction of the camera. */
		glm::vec3 _up{ 0, 1, 0 };
	};
}

#endif //GAME_COMPOSITETREE_CAMERANODE_H