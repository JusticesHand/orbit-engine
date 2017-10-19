/*! @file Game/CompositeTree/CameraNode.cpp */

#include "Game/CompositeTree/CameraNode.h"

#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/gtc/matrix_transform.hpp>

using namespace Orbit;

CameraNode::CameraNode()
	: Node("CAMERA")
{
}

CameraNode::~CameraNode() = default;

CameraNode::CameraNode(CameraNode&& rhs)
	: Node(std::move(rhs))
{
	_direction = rhs._direction;
	_up = rhs._up;
}

CameraNode& CameraNode::operator=(CameraNode&& rhs)
{
	Node::operator=(std::move(rhs));

	_direction = rhs._direction;
	_up = rhs._up;

	return *this;
}

void CameraNode::acceptVisitor(Visitor* visitor)
{
}

std::shared_ptr<Node> CameraNode::clone() const
{
	throw std::runtime_error("Cloning a camera node is not allowed!");
}

void CameraNode::update(std::chrono::nanoseconds elapsedtime)
{
}

glm::mat4 CameraNode::getViewMatrix() const
{
	return glm::lookAt(_position, _position + _direction, _up);
}

glm::vec3 CameraNode::direction() const
{
	return _direction;
}

glm::vec3 CameraNode::up() const
{
	return _up;
}

void CameraNode::setDirection(const glm::vec3& direction)
{
	_direction = direction;
}

void CameraNode::setUp(const glm::vec3& up)
{
	_up = up;
}