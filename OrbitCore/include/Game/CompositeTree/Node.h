///	@file Game/CompositeTree/Node.h

#ifndef GAME_COMPOSITETREE_NODE_H
#define GAME_COMPOSITETREE_NODE_H
#pragma once

#include <memory>
#include <string>
#include <chrono>
#include <mutex>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include "Util.h"

namespace Orbit
{
	class Model;
	class Visitor;

	///	A simple abstract node class to use with higher-level Composite and Visitor models,
	///	with obligatory Visitor support.
	class Node : public std::enable_shared_from_this<Node>
	{
	public:
		///	The class's only constructor.
		///	@param name the name to apply to the node to be able to search for it later.
		ORBIT_CORE_API explicit Node(const std::string& name);

		ORBIT_CORE_API explicit Node(const std::string& name, const std::shared_ptr<Model>& model);

		///	Destructor for this (abstract) class. Uses the default destructor.
		virtual ~Node() = 0;

		///	Move constructor for the class. Required by derived classes.
		///	@param rhs the node to move.
		ORBIT_CORE_API Node(Node&& rhs);
		ORBIT_CORE_API Node& operator=(Node&& rhs);

		Node(const Node&) = delete;
		Node& operator=(const Node&) = delete;

		///	A virtual method to implement the visitor pattern on an object.
		///	@param visitor a pointer to the used visitor.
		ORBIT_CORE_API virtual void acceptVisitor(Visitor* visitor);

		///	An abstract method to clone a node (as nodes are not copy-constructible).
		///	@return the cloned node.
		virtual std::shared_ptr<Node> clone() const = 0;

		virtual void update(std::chrono::nanoseconds elapsedTime) = 0;

		///	A virtual method to destroy a node. By default, only sets the destroyed property
		///	to true.
		ORBIT_CORE_API virtual void destroy();

		///	Gets the name of the node.
		///	@return the name of the node.
		ORBIT_CORE_API std::string getName() const;

		///	Gets whether or not the node has been destroyed.
		///	@see destroy()
		///	@return whether or not the node is destroyed.
		ORBIT_CORE_API bool destroyed() const;

		///	Returns a node with the name in parameter if found.
		///	@param name the name of the node to be found.
		///	@return the found node, or nullptr if not found.
		ORBIT_CORE_API virtual std::shared_ptr<Node> find(std::string name);

		///	Returns a node with the name in parameter if found.
		///	@param name the name of the node to be found.
		///	@return the found node, or nullptr if not found.
		ORBIT_CORE_API virtual std::shared_ptr<const Node> find(std::string name) const;

		ORBIT_CORE_API glm::vec3 position() const;

		ORBIT_CORE_API glm::quat rotation() const;

		ORBIT_CORE_API float scale() const;

		ORBIT_CORE_API glm::mat4 modelMatrix() const;

		ORBIT_CORE_API void setPosition(const glm::vec3& newPos);

		ORBIT_CORE_API void setRotation(const glm::quat& newRot);

		ORBIT_CORE_API void setScale(float newScale);

		ORBIT_CORE_API bool hasModel() const;

		ORBIT_CORE_API std::shared_ptr<Model> getModel() const;

	protected:
		///	Sets a value to the destroyed property. Preferred way to set it.
		///	@param value the value to set.
		ORBIT_CORE_API void setDestroyed(bool value);

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

		glm::vec3 _position;
		glm::quat _rotation;
		float _scale = 1.f;

	private:
		bool _destroyed = false; ///< Whether or not the object has been destroyed.
		std::string _name; ///< the name of the object, for searching purposes.
		std::shared_ptr<Model> _model;
	};

	inline Node::~Node() = default;
}

#endif //GAME_COMPOSITETREE_NODE_H
