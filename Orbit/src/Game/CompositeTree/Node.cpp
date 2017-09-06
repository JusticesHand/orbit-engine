#include "Game/CompositeTree/Node.h"

using namespace Orbit;

Node::Node(const std::string& name)
	: _name(name)
{
}

Node::Node(Node&& rhs)
	: _name(std::move(rhs._name))
{
}

Node& Node::operator=(Node&& rhs)
{
	_name = std::move(rhs._name);
	return *this;
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