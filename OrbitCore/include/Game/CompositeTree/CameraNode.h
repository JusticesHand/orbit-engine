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
	class CameraNode final : public Node
	{
	public:
		ORBIT_CORE_API explicit CameraNode();
		ORBIT_CORE_API virtual ~CameraNode();
		
		CameraNode(const CameraNode&) = delete;
		CameraNode& operator=(const CameraNode&) = delete;

		ORBIT_CORE_API CameraNode(CameraNode&& rhs);
		ORBIT_CORE_API CameraNode& operator=(CameraNode&& rhs);

		ORBIT_CORE_API void acceptVisitor(Visitor* visitor) override;

		ORBIT_CORE_API std::shared_ptr<Node> clone() const override;

		ORBIT_CORE_API void update(std::chrono::nanoseconds elapsedtime) override;

		ORBIT_CORE_API glm::mat4 getViewMatrix() const;

		ORBIT_CORE_API glm::vec3 direction() const;
		ORBIT_CORE_API glm::vec3 up() const;

		ORBIT_CORE_API void setDirection(const glm::vec3& direction);
		ORBIT_CORE_API void setUp(const glm::vec3& up);

	private:
		glm::vec3 _direction{ 0, 0, -1 };
		glm::vec3 _up{ 0, 1, 0 };
	};
}

#endif //GAME_COMPOSITETREE_CAMERANODE_H