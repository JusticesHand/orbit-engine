#include "Game/CompositeTree/CompositeTree.h"

using namespace Orbit;

CompositeTree::CompositeTree()
	: CompositeNode("ROOT_TREE_NODE")
{ 
}

CompositeTree::~CompositeTree() = default;

CompositeTree::CompositeTree(CompositeTree&& rhs)
	: CompositeNode(std::move(rhs))
{
}

CompositeTree& CompositeTree::operator=(CompositeTree&& rhs)
{
	CompositeNode::operator=(std::move(rhs));
	return *this;
}