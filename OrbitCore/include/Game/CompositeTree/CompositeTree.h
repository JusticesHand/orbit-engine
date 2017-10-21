/*! @file Game/CompositeTree/CompositeTree.h */

#ifndef GAME_COMPOSITETREE_COMPOSITETREE_H
#define GAME_COMPOSITETREE_COMPOSITETREE_H
#pragma once

#include "CompositeNode.h"
#include "CameraNode.h"

#include "Util.h"

namespace Orbit
{
	/*!
	@brief A concrete specialization of the composite tree structure. Simulates a root node and gives access
	to tree members (via visitor methods).
	*/
	class CompositeTree final : public CompositeNode
	{
	public:
		/*!
		@brief The class's constructor.
		@param input A reference to the game's input.
		*/
		ORBIT_CORE_API CompositeTree();
		
		/*!
		@brief The class's destructor.
		*/
		ORBIT_CORE_API virtual ~CompositeTree() = default;

		/*!
		@brief Move constructor for the class. Moves rhs's child nodes and invalidates rhs.
		@param rhs the tree to move.
		*/
		ORBIT_CORE_API CompositeTree(CompositeTree&& rhs);

		/*!
		@brief Move assignment operator for the class. Moves rhs's child nodes and invalidates rhs.
		@param rhs The tree to move.
		@return A reference to this.
		*/
		ORBIT_CORE_API CompositeTree& operator=(CompositeTree&& rhs);

		CompositeTree(const CompositeTree&) = delete;
		CompositeTree& operator=(const CompositeTree&) = delete;

		/*!
		@brief Cloning operation for the tree. Returns a deep copy of the tree, calling the clone method
		on every child node.
		@return a copy of this tree.
		*/
		ORBIT_CORE_API std::shared_ptr<Node> clone() const override;

		/*!
		@brief Gets a camera present in the tree's hierarchy, if any.
		@see Orbit::CameraNode
		@return The tree's camera, or nullptr if not found.
		*/
		ORBIT_CORE_API std::shared_ptr<CameraNode> getCamera();

		/*!
		@copydoc Orbit::CompositeTree::getCamera()
		*/
		ORBIT_CORE_API std::shared_ptr<const CameraNode> getCamera() const;
	};
}

#endif //GAME_COMPOSITETREE_COMPOSITETREE_H
