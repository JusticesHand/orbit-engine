///	@file Game/CompositeTree/Node.cpp

#include "Game/CompositeTree/Node.h"

using namespace Orbit;

///	Simply constructs a node with the name in parameter. Other members are constructed with default values.
///	@param name the name to apply to the node to be able to search for it later.
Node::Node(const std::string& name)
	: _name(name)
{
}

///	Move constructor - moves the name from rhs (making it undefined). Useful as a base for derived class
///	move operations.
Node::Node(Node&& rhs)
	: _name(std::move(rhs._name))
{
}

///	Move assignment operator - as with the move constructor, moves the name from rhs (making it undefined).
///	Useful as a base for derived class move operations.
Node& Node::operator=(Node&& rhs)
{
	_name = std::move(rhs._name);
	return *this;
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