///	@file Game/CompositeTree/CompositeNode.h

#ifndef GAME_COMPOSITETREE_COMPOSITENODE_H
#define GAME_COMPOSITETREE_COMPOSITENODE_H
#pragma once

#include "Node.h"

#include "Util.h"

#include <vector>

namespace Orbit
{
	///	An abstract specialization of the Node class to add Composite functionality - that is, the
	///	ability to have child nodes.
	class CompositeNode : public Node
	{
	public:
		///	The class's only constructor.
		///	@param name the name to apply to the node to be able to search for it later.
		ORBIT_CORE_API explicit CompositeNode(const std::string& name);

		///	Destructor for this (abstract) class. Uses the default destructor.
		virtual ~CompositeNode() = 0;

		///	Move constructor for the class. Moves the children as to not have multiple node ownerships.
		///	@param rhs the node to move.
		ORBIT_CORE_API CompositeNode(CompositeNode&& rhs);

		///	Move assignment operator for the class.
		///	Moves the children as to not have multiple node ownerships.
		///	@param rhs the node to move.
		///	@return a reference to this.
		ORBIT_CORE_API CompositeNode& operator=(CompositeNode&& rhs);

		CompositeNode(const CompositeNode&) = delete;
		CompositeNode& operator=(const CompositeNode&) = delete;

		///	A method to implement the visitor pattern on this composite object.
		///	Redirects the visitor to all children.
		///	@param visitor a pointer to the used visitor.
		ORBIT_CORE_API void acceptVisitor(const Visitor* visitor) override;

		///	A method to destroy a node. Destroys child nodes.
		ORBIT_CORE_API void destroy() override;

		ORBIT_CORE_API virtual void update(std::chrono::nanoseconds elapsedTime);

		///	Adds a child to this node's children. Throws an exception if the parameter is null or the child
		///	already exists in the subtree.
		///	@param child the child to add.
		ORBIT_CORE_API void addChild(std::shared_ptr<Node> child);

		///	Removes a child at the first level. If this child is not found, nothing is done.
		///	@param child the child to remove.
		ORBIT_CORE_API void removeChild(std::shared_ptr<Node> child);

		/// Clears all children from the composite node.
		ORBIT_CORE_API void clearChildren();

		///	Returns a node with the name in parameter if found. Searches in the child nodes.
		///	@param name the name of the node to be found.
		///	@return the found node, or nullptr if not found.
		ORBIT_CORE_API std::shared_ptr<Node> find(std::string name) override;

		///	Returns a node with the name in parameter if found. Searches in the child nodes.
		///	@param name the name of the node to be found.
		///	@return the found node, or nullptr if not found.
		ORBIT_CORE_API std::shared_ptr<const Node> find(std::string name) const override;

	protected:
		///	Returns a locked version of the parent.
		///	@return the node's parent, or nullptr if none.
		ORBIT_CORE_API std::shared_ptr<CompositeNode> getParent();

		///	Returns a locked version of the parent.
		///	@return the node's parent, or nullptr if none.
		ORBIT_CORE_API std::shared_ptr<const CompositeNode> getParent() const;

		/// Returns a vector of cloned children. Useful when overriding the clone() 
		/// method in concrete classes.
		/// @return a deep copy (using clone()) of the child nodes.
		ORBIT_CORE_API std::vector<std::shared_ptr<Node>> cloneChildren() const;

		/// Moves the children in parameter to the children in the tree. Essentially destroys the children
		/// in the tree in order to set the new children.
		/// @param children the children to move.
		ORBIT_CORE_API void moveChildren(std::vector<std::shared_ptr<Node>>&& children);

	private:
		std::weak_ptr<CompositeNode> _parent; ///< A weak reference to the node's parent.
		std::vector<std::shared_ptr<Node>> _children; ///< A list of children owned by the node.

	};

	inline CompositeNode::~CompositeNode() = default;
}

#endif //GAME_COMPOSITETREE_COMPOSITENODE_H
