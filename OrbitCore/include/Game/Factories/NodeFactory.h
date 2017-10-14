/*! @file Game/Factories/NodeFactory.h */

#ifndef GAME_FACTORIES_NODEFACTORY_H
#define GAME_FACTORIES_NODEFACTORY_H
#pragma once

#include <memory>
#include <string>

namespace Orbit
{
	class Node;

	class NodeFactory
	{
	public:
		NodeFactory() = default;

		virtual std::shared_ptr<Node> create() const = 0;
		virtual std::shared_ptr<Node> create(const std::string& name) const = 0;
	};
}

#endif //GAME_FACTORIES_NODEFACTORY_H