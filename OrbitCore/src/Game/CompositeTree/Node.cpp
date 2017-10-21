/*! @file Game/CompositeTree/Node.cpp */

#include "Game/CompositeTree/Node.h"

#include "Game/CompositeTree/Visitor.h"

#include "Render/Model.h"

#include <glm/gtc/matrix_transform.hpp>

using namespace Orbit;

Node::Node(const std::string& name, const std::shared_ptr<Model>& model)
	: _name(name), _model(model)
{
}

Node::Node(const Input& input, const std::string& name, const std::shared_ptr<Model>& model)
	: _input(&input), _name(name), _model(model)
{
}

Node::Node(Node&& rhs)
	: _input(rhs._input), _name(std::move(rhs._name)), _model(rhs._model)
{
}

Node& Node::operator=(Node&& rhs)
{
	_input = rhs._input;
	_destroyed = rhs._destroyed;
	_name = std::move(rhs._name);
	_model = rhs._model;
	return *this;
}

void Node::acceptVisitor(Visitor* visitor)
{
	visitor->visitElement(this);
}

void Node::destroy()
{
	setDestroyed(true);
}

std::string Node::getName() const
{
	return _name;
}

bool Node::destroyed() const
{
	
	std::lock_guard<std::mutex> lock(_mutex);
	return _destroyed;
}

void Node::setDestroyed(bool value)
{
	std::lock_guard<std::mutex> lock(_mutex);
	_destroyed = value;
}

bool Node::hasModel() const
{
	return _model != nullptr;
}

std::shared_ptr<Model> Node::getModel() const
{
	return _model;
}

std::shared_ptr<Node> Node::find(std::string name)
{
	if (_name == name)
		return shared_from_this();

	return nullptr;
}

std::shared_ptr<const Node> Node::find(std::string name) const
{
	if (_name == name)
		return shared_from_this();

	return nullptr;
}

glm::vec3 Node::position() const
{
	return _position;
}

glm::quat Node::rotation() const
{
	return _rotation;
}

float Node::scale() const
{
	return _scale;
}

glm::mat4 Node::modelMatrix() const
{
	glm::mat4 result = glm::translate(glm::mat4(), _position);
	result *= glm::mat4_cast(_rotation);
	return glm::scale(result, glm::vec3(_scale));
}

void Node::setPosition(const glm::vec3& newPos)
{
	_position = newPos;
}

void Node::setRotation(const glm::quat& newRot)
{
	_rotation = newRot;
}

void Node::setScale(float newScale)
{
	_scale = newScale;
}

const Input& Node::getInput() const
{
	if (_input == nullptr)
		throw std::runtime_error("Attempted to get input for a node whose input was uninitialized!");

	return *_input;
}