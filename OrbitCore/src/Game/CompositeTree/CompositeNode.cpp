///	@file Game/CompositeTree/CompositeNode.cpp

#include "Game/CompositeTree/CompositeNode.h"

using namespace Orbit;

/// The class's constructor. Simply calls the base Node constructor with the parameter.
///	@param name the name to apply to the node to be able to search for it later.
CompositeNode::CompositeNode(const std::string& name)
	: Node(name)
{
}

///	Move constructor. Moves the base node, along with the parent and children.
///	@param rhs the node to move.
CompositeNode::CompositeNode(CompositeNode&& rhs)
	: Node(std::move(rhs)), _parent(std::move(rhs._parent)), _children(std::move(rhs._children))
{
}

/// Move assignment operator. Moves the base node, along with the parents and children.
/// @param rhs the node to move.
CompositeNode& CompositeNode::operator=(CompositeNode&& rhs)
{
	Node::operator=(std::move(rhs));
	_parent = std::move(rhs._parent);
	_children = std::move(rhs._children);
	return *this;
}

/// Implements the visitor pattern for the class. Simply iterates through children and makes them accept the
/// visitor. It's meant to be called as a final operation of visitor accepting for child classes.
/// @param visitor a pointer to the used visitor.
void CompositeNode::acceptVisitor(const Visitor* visitor)
{
	for (std::shared_ptr<Node>& child : _children)
		child->acceptVisitor(visitor);
}

///	A method to destroy the node. Ensures the node is orphaned and the children destroyed (and orphaned) 
///	as well.
void CompositeNode::destroy()
{
	if (destroyed())
		return;

	std::shared_ptr<CompositeNode> parent = getParent();
	if (parent)
	{
		parent->removeChild(shared_from_this());
		parent = nullptr;
	}

	clearChildren();
	setDestroyed(true);
}

void CompositeNode::update(std::chrono::nanoseconds elapsedTime)
{
	for (std::shared_ptr<Node>& child : _children)
		if (!child->destroyed())
			child->update(elapsedTime);
}

///	Adds a child to this node's children. Throws an exception if the parameter is null or the child
///	already exists in the subtree.
///	@param child the child to add.
void CompositeNode::addChild(std::shared_ptr<Node> child)
{
	if (!child)
		throw std::runtime_error("Attempted to add a null child!");

	if (find(child->getName()))
		throw std::runtime_error("Child is already in children (or subchildren)!");

	_children.push_back(child);
}

///	Removes a child at the first level. If this child is not found, nothing is done.
///	@param child the child to remove.
void CompositeNode::removeChild(std::shared_ptr<Node> child)
{
	auto foundChild = std::find(_children.begin(), _children.end(), child);
	if (foundChild == _children.end())
		return;

	std::swap(*foundChild, _children.back());
	_children.pop_back();
}

/// Removes all children in the tree. Essentially clears the children vector.
void CompositeNode::clearChildren()
{
	_children.clear();
}

///	Returns a node with the name in parameter if found. Searches in the child nodes.
///	@param name the name of the node to be found.
///	@return the found node, or nullptr if not found.
std::shared_ptr<Node> CompositeNode::find(std::string name)
{
	if (getName() == name)
		return shared_from_this();

	for (std::shared_ptr<Node> child : _children)
	{
		std::shared_ptr<Node> found = child->find(name);
		if (found)
			return found;
	}

	return nullptr;
}

///	Returns a node with the name in parameter if found. Searches in the child nodes.
///	@param name the name of the node to be found.
///	@return the found node, or nullptr if not found.
std::shared_ptr<const Node> CompositeNode::find(std::string name) const
{
	if (getName() == name)
		return shared_from_this();

	for (std::shared_ptr<Node> child : _children)
	{
		std::shared_ptr<Node> found = child->find(name);
		if (found)
			return found;
	}

	return nullptr;
}

/// Returns a vector of cloned children, with each child having its clone method called.
/// Ideally returns a deep copy of child nodes (though the actual nodes returned depend entirely on the
/// clone() implementation).
/// @return a vector of cloned children.
std::vector<std::shared_ptr<Node>> CompositeNode::cloneChildren() const
{
	std::vector<std::shared_ptr<Node>> clonedChildren;

	for (const std::shared_ptr<Node>& child : _children)
		clonedChildren.push_back(child->clone());

	return clonedChildren;
}

///	Returns a locked version of the parent.
///	@return the node's parent, or nullptr if none.
std::shared_ptr<CompositeNode> CompositeNode::getParent()
{
	return _parent.lock();
}

///	Returns a locked version of the parent.
///	@return the node's parent, or nullptr if none.
std::shared_ptr<const CompositeNode> CompositeNode::getParent() const
{
	return std::const_pointer_cast<const CompositeNode>(_parent.lock());
}

/// Moves the children in parameter to be this node's children. Essentially destroys the children
/// in the tree in order to set the new children.
/// @param children the children to move.
void CompositeNode::moveChildren(std::vector<std::shared_ptr<Node>>&& children)
{
	_children = std::move(children);
}