/*!	@file Game/CompositeTree/CompositeNode.h */

#ifndef GAME_COMPOSITETREE_COMPOSITENODE_H
#define GAME_COMPOSITETREE_COMPOSITENODE_H
#pragma once

#include "Node.h"

#include "Util.h"

#include <vector>

namespace Orbit
{
	/*!
	@brief An abstract specialization of the Node class to add Composite functionality - that is, the
	ability to have child nodes.
	*/
	class CompositeNode : public Node
	{
	public:
		/*!
		@brief The class's only constructor.
		@param name the name to apply to the node to be able to search for it later.
		*/
		ORBIT_CORE_API explicit CompositeNode(const std::string& name);

		/*!
		@brief Destructor for this (abstract) class.
		*/
		virtual ~CompositeNode() = 0;

		/*!
		@brief Move constructor for the class. Moves the children as to not have multiple node ownerships.
		@param rhs the node to move.
		*/
		ORBIT_CORE_API CompositeNode(CompositeNode&& rhs);

		/*!
		@brief Move assignment operator for the class. Moves the children as to not have multiple node ownerships.
		@param rhs the node to move.
		@return A reference to this.
		*/
		ORBIT_CORE_API CompositeNode& operator=(CompositeNode&& rhs);

		CompositeNode(const CompositeNode&) = delete;
		CompositeNode& operator=(const CompositeNode&) = delete;

		/*!
		@brief Implementation of the visitor pattern for a composite node. Passes the visitor on to children.
		@param visitor The visitor to accept.
		*/
		ORBIT_CORE_API void acceptVisitor(Visitor* visitor) override;

		/*! 
		@brief A method to destroy a node. Recursively destroys child nodes.
		*/
		ORBIT_CORE_API void destroy() override;

		/*!
		@brief Updates the node. Calls the update method for child nodes.
		@param elapsedTime The elapsed time since the last update cycle.
		*/
		ORBIT_CORE_API virtual void update(std::chrono::nanoseconds elapsedTime);

		/*!
		@brief Adds a child to this node's children.
		@throw std::runtime_error Throws if the child in param is nullptr.
		@throw std::runtime_error Throws if the child in param is already in the child hierarchy.
		@param child the child to add.
		*/
		ORBIT_CORE_API void addChild(std::shared_ptr<Node> child);

		/*!
		@brief Removes a child at the first level. If this child is not found, nothing is done.
		@param child the child to remove.
		*/
		ORBIT_CORE_API void removeChild(std::shared_ptr<Node> child);

		/*!
		@brief Clears all children from the composite node.
		*/
		ORBIT_CORE_API void clearChildren();

		/*!
		@brief Returns a node with the name in parameter if found. Searches recursively in the child nodes.
		@param name the name of the node to be found.
		@return the found node, or nullptr if not found.
		*/
		ORBIT_CORE_API std::shared_ptr<Node> find(std::string name) override;

		/*!
		@copydoc Orbit::CompositeNode::find(std::string)
		*/
		ORBIT_CORE_API std::shared_ptr<const Node> find(std::string name) const override;

	protected:
		/*!
		@brief Returns a locked version of the parent.
		@return the node's parent, or nullptr if none.
		*/
		ORBIT_CORE_API std::shared_ptr<CompositeNode> getParent();

		/*!
		@copydoc Orbit::CompositeNode::getParent()
		*/
		ORBIT_CORE_API std::shared_ptr<const CompositeNode> getParent() const;

		/*!
		@brief Returns a vector of cloned children. Useful when overriding the clone()
		method in concrete classes.
		@return a deep copy (using clone()) of the child nodes.
		*/
		ORBIT_CORE_API std::vector<std::shared_ptr<Node>> cloneChildren() const;

		/*!
		@brief Moves the children in parameter to the children in the tree. Essentially destroys the children
		in the tree in order to set the new children.
		@param children the children to move.
		*/
		ORBIT_CORE_API void moveChildren(std::vector<std::shared_ptr<Node>>&& children);

	private:
		/*! A weak reference to the node's parent. */
		std::weak_ptr<CompositeNode> _parent;
		/*! A list of the children owned by the node. */
		std::vector<std::shared_ptr<Node>> _children;

	};

	inline CompositeNode::~CompositeNode() = default;
}

#endif //GAME_COMPOSITETREE_COMPOSITENODE_H
