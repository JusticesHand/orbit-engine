///	@file Game/CompositeTree/Node.h

#ifndef GAME_COMPOSITETREE_NODE_H
#define GAME_COMPOSITETREE_NODE_H
#pragma once

#include <memory>
#include <string>
#include <mutex>

namespace Orbit
{
	class Visitor;

	///	A simple abstract node class to use with higher-level Composite and Visitor models,
	///	with obligatory Visitor support.
	class Node : public std::enable_shared_from_this<Node>
	{
	public:
		///	The class's only constructor.
		///	@param name the name to apply to the node to be able to search for it later.
		explicit Node(const std::string& name);

		///	Destructor for this (abstract) class. Uses the default destructor.
		virtual ~Node() = 0;

		///	Move constructor for the class. Required by derived classes.
		///	@param rhs the node to move.
		Node(Node&& rhs);

		///	Move assignment operator for the class. Required by derived classes.
		///	@param rhs the node to move.
		///	@return a reference to this.
		Node& operator=(Node&& rhs);

		Node(const Node&) = delete;
		Node& operator=(const Node&) = delete;

		///	An abstract method to implement the visitor pattern on an object.
		///	@param visitor a pointer to the used visitor.
		virtual void acceptVisitor(const Visitor* visitor) = 0;

		///	An abstract method to clone a node (as nodes are not copy-constructible).
		///	@return the cloned node.
		virtual std::shared_ptr<Node> clone() const = 0;

		///	A virtual method to destroy a node. By default, only sets the destroyed property
		///	to true.
		virtual void destroy();

		///	Gets the name of the node.
		///	@return the name of the node.
		std::string getName() const;

		///	Gets whether or not the node has been destroyed.
		///	@see destroy()
		///	@return whether or not the node is destroyed.
		bool destroyed() const;

		///	Returns a node with the name in parameter if found.
		///	@param name the name of the node to be found.
		///	@return the found node, or nullptr if not found.
		virtual std::shared_ptr<Node> find(std::string name);

		///	Returns a node with the name in parameter if found.
		///	@param name the name of the node to be found.
		///	@return the found node, or nullptr if not found.
		virtual std::shared_ptr<const Node> find(std::string name) const;

	protected:
		///	Sets a value to the destroyed property. Preferred way to set it.
		///	@param value the value to set.
		void setDestroyed(bool value);

		mutable std::mutex _mutex; ///< Mutex to be used to control access to this object.

		///	Simple extension method to add functionality to shared_from_this() functionality.
		///	Handles casting to derived classes of shared_from_this(), simplifying class hierarchy.
		///	@return a std::shared_ptr<Derived> pointing to this.
		template<typename Derived>
		std::shared_ptr<Derived> shared_from_base()
		{
			static_assert(std::is_base_of_v<Node, Derived>, "Derived must derive from Node!");
			return std::static_pointer_cast<Derived>(shared_from_this());
		}

	private:
		bool _destroyed = false; ///< Whether or not the object has been destroyed.
		std::string _name; ///< the name of the object, for searching purposes.
	};

	inline Node::~Node() = default;
}

#endif //GAME_COMPOSITETREE_NODE_H
