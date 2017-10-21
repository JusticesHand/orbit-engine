/*! @file Game/CompositeTree/Node.h */

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
	class Input;
	class Visitor;

	/*!
	@brief A simple abstract node class to use with higher-level Composite and Visitor models,
	with obligatory Visitor support.
	*/
	class Node : public std::enable_shared_from_this<Node>
	{
	public:
		/*!
		@brief The class's constructor, for a node where input is not required.
		@param name The name applied to the node to enable named searching.
		@param model The model to apply to the node.
		*/
		ORBIT_CORE_API explicit Node(const std::string& name, const std::shared_ptr<Model>& model = nullptr);

		/*!
		@brief The class's constructor, for a node where input is needed.
		@param input A reference to the input handler of the game.
		@param name The name applied to the node to enable named searching.
		@param model The model to apply to the node.
		*/
		ORBIT_CORE_API explicit Node(const Input& input, const std::string& name, const std::shared_ptr<Model>& model = nullptr);

		/*!
		@brief Move constructor for the class. Required by derived classes.
		@param rhs The node to move.
		*/
		ORBIT_CORE_API Node(Node&& rhs);

		ORBIT_CORE_API virtual ~Node() = default;

		/*!
		@brief Move assignment operator for the class. Required by derived classes.
		@param rhs The node to move.
		@return A reference to this.
		*/
		ORBIT_CORE_API Node& operator=(Node&& rhs);

		Node(const Node&) = delete;
		Node& operator=(const Node&) = delete;

		/*!
		@brief A virtual method to implement the visitor pattern on an object.
		@param visitor A pointer to the used visitor.
		*/
		ORBIT_CORE_API virtual void acceptVisitor(Visitor* visitor);

		/*!
		@brief An abstract method to clone a node (as nodes are not copy-constructible).
		@return The cloned node.
		*/
		virtual std::shared_ptr<Node> clone() const = 0;

		/*!
		@brief An abstract method to enable updating on a node, based around a cycle.
		@param elapsedTime The elapsed time since the last update cycle.
		*/
		virtual void update(std::chrono::nanoseconds elapsedTime) = 0;

		/*!
		@brief A virtual method to destroy a node. By default, only sets the destroyed property
		to true.
		*/
		ORBIT_CORE_API virtual void destroy();

		/*!
		@brief Getter for the node's name.
		@return The name of the node.
		*/
		ORBIT_CORE_API std::string getName() const;

		/*!
		@brief Getter for the node's destroyed property.
		@see destroy()
		@return whether or not the node is destroyed.
		*/
		ORBIT_CORE_API bool destroyed() const;

		/*!
		@brief Searches for a node with the name in parameter.
		@param name The name of the node to be found.
		@return The found node, or nullptr if not found.
		*/
		ORBIT_CORE_API virtual std::shared_ptr<Node> find(std::string name);

		/*!
		@copydoc Orbit::Node::find(std::string)
		*/
		ORBIT_CORE_API virtual std::shared_ptr<const Node> find(std::string name) const;

		/*!
		@brief Getter for the node's position.
		@return The node's position.
		*/
		ORBIT_CORE_API glm::vec3 position() const;

		/*!
		@brief Getter for the node's rotation.
		@return The node's rotation.
		*/
		ORBIT_CORE_API glm::quat rotation() const;

		/*!
		@brief Getter for the node's scale.
		@return The node's scale.
		*/
		ORBIT_CORE_API float scale() const;

		/*!
		@brief Computes the model matrix for this node, using the node's position, rotation and scale properties.
		@return The node's model matrix.
		*/
		ORBIT_CORE_API glm::mat4 modelMatrix() const;

		/*!
		@brief Setter for the node's position.
		@param newPos The node's new position.
		*/
		ORBIT_CORE_API void setPosition(const glm::vec3& newPos);

		/*!
		@brief Setter for the node's rotation.
		@param newRot The node's new rotation.
		*/
		ORBIT_CORE_API void setRotation(const glm::quat& newRot);

		/*!
		@brief Setter for the node's scale.
		@param newScale The node's new scale.
		*/
		ORBIT_CORE_API void setScale(float newScale);

		/*!
		@brief Checks if the node has a model assigned.
		@return Whether the node has a model or not.
		*/
		ORBIT_CORE_API bool hasModel() const;

		/*!
		@brief Returns the node's model.
		@return The node's model.
		*/
		ORBIT_CORE_API std::shared_ptr<Model> getModel() const;

	protected:
		/*!
		@brief Sets a value to the destroyed property. Preferred way to set it.
		@param value the value to set.
		*/
		ORBIT_CORE_API void setDestroyed(bool value);

		/*!
		@brief Getter for a reference to the node's input handler, passed along during construction.
		@return A reference to the node's input handler.
		*/
		ORBIT_CORE_API const Input& getInput() const;

		/*! Mutex to be used to control access to this object. */
		mutable std::mutex _mutex;

		/*!
		@brief Simple extension method to add functionality to shared_from_this() functionality.
		Handles casting to derived classes of shared_from_this(), simplifying class hierarchy.
		@tparam Derived The type of the derived class to cast to. Must satisfy std::is_base_of<Node, Derived>::value.
		@return a std::shared_ptr<Derived> pointing to this.
		*/
		template<typename Derived>
		std::shared_ptr<Derived> shared_from_base()
		{
			static_assert(std::is_base_of_v<Node, Derived>, "Derived must derive from Node!");
			return std::static_pointer_cast<Derived>(shared_from_this());
		}

		/*! The node's position. */
		glm::vec3 _position;
		/*! The node's rotation. */
		glm::quat _rotation;
		/*! The node's scale. */
		float _scale = 1.f;

	private:
		/*! The status of the node's destruction. */
		bool _destroyed = false;
		/*! The node's input handler pointer, allowing nullptr and copy semantics. */
		const Input* _input = nullptr;
		/*! The node's name. */
		std::string _name;
		/*! The node's model. */
		std::shared_ptr<Model> _model;
	};
}

#endif //GAME_COMPOSITETREE_NODE_H
