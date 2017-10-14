///	@file Game/CompositeTree/Node.cpp

#include "Game/CompositeTree/Node.h"

#include "Game/CompositeTree/Visitor.h"

#include "Render/Model.h"

#include <glm/gtc/matrix_transform.hpp>

using namespace Orbit;

///	Simply constructs a node with the name in parameter. Other members are constructed with default values.
///	@param name the name to apply to the node to be able to search for it later.
Node::Node(const std::string& name)
	: Node(name, nullptr)
{
}

Node::Node(const std::string& name, const std::shared_ptr<Model>& model)
	: _name(name), _model(model)
{
}

///	Move constructor - moves the name from rhs (making it undefined). Useful as a base for derived class
///	move operations.
Node::Node(Node&& rhs)
	: _name(std::move(rhs._name)), _model(rhs._model)
{
}

Node& Node::operator=(Node&& rhs)
{
	_destroyed = rhs._destroyed;
	_name = std::move(rhs._name);
	_model = rhs._model;
	return *this;
}

void Node::acceptVisitor(Visitor* visitor)
{
	visitor->visitElement(this);
}

///	Destroys the node. Base implementation simply flags the node as destroyed - derived classes are meant to
///	override this behaviour to implement their own destruction, should it be more complex. By flagging this,
///	it ensures that the destruction operation is logically executed only once (actual object destruction is
///	executed separately to help speed up heap access operations, as they are temporally constrained).
void Node::destroy()
{
	setDestroyed(true);
}

///	Returns the name of the node. Useful for derived classes who need the node's name for algorithm work.
std::string Node::getName() const
{
	return _name;
}

///	Returns the flag of whether or not the node is destroyed. This flag is synchronized using the node's
///	unique mutex object.
bool Node::destroyed() const
{
	
	std::lock_guard<std::mutex> lock(_mutex);
	return _destroyed;
}

///	Sets the flag of whether or not the node is destroyed. This flag is synchronized using the node's
///	unique mutex object.
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

///	Returns a node that is found with the name in parameter. For complex nodes (such as composites), it
///	theorically searches in children. In this base case, however, only the current node is checked.
std::shared_ptr<Node> Node::find(std::string name)
{
	if (_name == name)
		return shared_from_this();

	return nullptr;
}

///	Same as Node::find(std::string), except with proper constness to use with const objects.
///	Note that the returned pointer is a shared_ptr to a const instance.
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