#include "Game/CompositeTree/CompositeTree.h"

using namespace Orbit;

/// Constructor for the class. Sets the name of the node to 'ROOT_TREE_NODE'.
CompositeTree::CompositeTree()
	: CompositeNode("ROOT_TREE_NODE")
{ 
}

/// Class's destructor is the default implementation.
CompositeTree::~CompositeTree() = default;

/// Move constructor for the class. Calls CompositeNode's move constructor.
/// @param rhs the tree to move.
CompositeTree::CompositeTree(CompositeTree&& rhs)
	: CompositeNode(std::move(rhs))
{
}

/// Move assignment operator for the class. Calls CompositeNode's move assignment operator.
/// @param rhs the tree to move.
/// @return a reference to this.
CompositeTree& CompositeTree::operator=(CompositeTree&& rhs)
{
	CompositeNode::operator=(std::move(rhs));
	return *this;
}

/// Cloning operation for the tree. Returns a deep copy of the tree (as far the clone() methods allow).
/// @return a cloned node.
std::shared_ptr<Node> CompositeTree::clone() const
{
	std::shared_ptr<CompositeTree> newTree = std::make_shared<CompositeTree>();
	newTree->moveChildren(cloneChildren());
	return newTree;
}