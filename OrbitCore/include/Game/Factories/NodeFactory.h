/*! @file Game/Factories/NodeFactory.h */

#ifndef GAME_FACTORIES_NODEFACTORY_H
#define GAME_FACTORIES_NODEFACTORY_H
#pragma once

#include <memory>
#include <string>

namespace Orbit
{
	class Node;

	/*!
	@brief Base class for implementation of the Factory design pattern for Node-derived classes.
	@see Orbit::Node
	*/
	class NodeFactory
	{
	public:
		/*!
		@brief Default constructor for the class.
		*/
		NodeFactory() = default;

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
	};
}

#endif //GAME_FACTORIES_NODEFACTORY_H