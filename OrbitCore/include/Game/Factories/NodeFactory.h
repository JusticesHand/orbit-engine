/*! @file Game/Factories/NodeFactory.h */

#ifndef GAME_FACTORIES_NODEFACTORY_H
#define GAME_FACTORIES_NODEFACTORY_H
#pragma once

#include <memory>
#include <string>

#include "Util.h"

namespace Orbit
{
	class Node;
	class Input;

	/*!
	@brief Base class for implementation of the Factory design pattern for Node-derived classes.
	@see Orbit::Node
	*/
	class NodeFactory
	{
	public:
		/*!
		@brief Constructor for the class. Sets the reference to the input handler for future construction.
		*/
		ORBIT_CORE_API NodeFactory(const Input& input);

		/*!
		@brief Creates a new node instance.
		@return A new node instance.
		*/
		virtual std::shared_ptr<Node> create() const = 0;

		/*!
		@brief Creates a new node instance, passing along the node's name.
		@param name The name of the newly created node.
		@return A new node instance with the name in param.*/
		virtual std::shared_ptr<Node> create(const std::string& name) const = 0;

	protected:
		/*! A reference to the input handler for node creation. */
		const Input& _input;
	};
}

#endif //GAME_FACTORIES_NODEFACTORY_H