/*! @file Game/CompositeTree/CompositeTree.cpp */

#include "Game/CompositeTree/CompositeTree.h"

using namespace Orbit;

CompositeTree::CompositeTree()
	: CompositeNode( "ROOT_TREE_NODE")
{
}

CompositeTree::CompositeTree(CompositeTree&& rhs)
	: CompositeNode(std::move(rhs))
{
}

CompositeTree& CompositeTree::operator=(CompositeTree&& rhs)
{
	CompositeNode::operator=(std::move(rhs));
	return *this;
}

std::shared_ptr<Node> CompositeTree::clone() const
{
	std::shared_ptr<CompositeTree> newTree = std::make_shared<CompositeTree>();
	newTree->moveChildren(cloneChildren());
	return newTree;
}

std::shared_ptr<CameraNode> CompositeTree::getCamera()
{
	return std::dynamic_pointer_cast<CameraNode>(find("CAMERA"));
}

std::shared_ptr<const CameraNode> CompositeTree::getCamera() const
{
	return std::dynamic_pointer_cast<const CameraNode>(find("CAMERA"));
}