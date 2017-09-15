///	@file Game/CompositeTree/CompositeTree.h

#ifndef GAME_COMPOSITETREE_COMPOSITETREE_H
#define GAME_COMPOSITETREE_COMPOSITETREE_H
#pragma once

#include "CompositeNode.h"

namespace Orbit
{
	/// A concrete specialization of the composite tree structure. Simulates a root node and gives access
	/// to tree members (via visitor methods).
	class CompositeTree : public CompositeNode
	{
	public:
		/// The class's constructor.
		CompositeTree();
		
		/// The class's destructor.
		virtual ~CompositeTree();

		/// Move constructor for the class. Moves rhs's child nodes and invalidates rhs.
		/// @param rhs the tree to move.
		CompositeTree(CompositeTree&& rhs);
		/// Move assignment operator for the class. Moves rhs's child nodes and invalidates rhs.
		/// @param rhs the tree to move.
		/// @return a reference to this.
		CompositeTree& operator=(CompositeTree&& rhs);

		CompositeTree(const CompositeTree&) = delete;
		CompositeTree& operator=(const CompositeTree&) = delete;

		/// Cloning operation for the tree. Returns a deep copy of the tree, calling the clone method
		/// on every child node.
		/// @return a copy of this tree.
		std::shared_ptr<Node> clone() const override;
	};
}

#endif //GAME_COMPOSITETREE_COMPOSITETREE_H
