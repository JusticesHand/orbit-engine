#include "Game/CompositeTree/CompositeNode.h"

using namespace Orbit;

CompositeNode::CompositeNode(const std::string& name)
	: Node(name)
{
}

void CompositeNode::acceptVisitor(const Visitor* visitor)
{
	for (std::shared_ptr<Node>& child : _children)
		child->acceptVisitor(visitor);
}

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

	while (!_children.empty())
	{
		std::shared_ptr<Node> child = _children.back();
		removeChild(child);
		child->destroy();
	}

	setDestroyed(true);
}

void CompositeNode::addChild(std::shared_ptr<Node> child)
{
	if (!child)
		throw std::runtime_error("Attempted to add a null child!");

	if (find(child->getName()))
		throw std::runtime_error("Child is already in children (or subchildren)!");

	_children.push_back(child);
}

void CompositeNode::removeChild(std::shared_ptr<Node> child)
{
	auto foundChild = std::find(_children.begin(), _children.end(), child);
	if (foundChild == _children.end())
		return;

	std::swap(*foundChild, _children.back());
	_children.pop_back();
}

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

std::shared_ptr<CompositeNode> CompositeNode::getParent()
{
	return _parent.lock();
}

std::shared_ptr<const CompositeNode> CompositeNode::getParent() const
{
	return std::const_pointer_cast<const CompositeNode>(_parent.lock());
}