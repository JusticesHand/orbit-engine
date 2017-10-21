/*! @file Game/Scene.h */

#ifndef GAME_SCENE_H
#define GAME_SCENE_H
#pragma once

#include "Render/Model.h"
#include "Util.h"

#include "Factories/NodeFactory.h"

#include <memory>
#include <string>
#include <type_traits>
#include <typeindex>
#include <unordered_map>

namespace Orbit
{
	class CompositeTree;
	class Node;

	/*!
	@brief Base class for scene logic. Handles loading of the scene through loadFactories (which load node-creating
	factories) and load (which instances the scene's initial tree state through those same factories).
	Custom data is meant to be loaded in the loadFactories() call and unloaded with the given unload() method.
	*/
	class Scene
	{
	public:
		/*!
		@brief Destructor for the class.
		*/
		virtual ~Scene() = default;

		/*!
		@brief Loads the factories necessary for future node creation. Factories must be registered using the storeFactory
		to be useable.
		@see Orbit::Scene::storeFactory(std::unique_ptr<Factory>)
		*/
		virtual void loadFactories(const Input& input) = 0;

		/*!
		@brief Loads the initial composite tree state. Creates the necessary nodes in the tree, using the loaded factories.
		@see Orbit::Scene::loadFactories()
		@param tree The tree in which to load up nodes.
		*/
		virtual void load(CompositeTree& tree) = 0;

		/*!
		@brief Unloads custom data from the scene, such as external models and file descriptors.
		*/
		virtual void unload() = 0;

		/*!
		@brief Creates a node of type T from the scene's factories.
		@tparam T The type of node to create.
		@return The newly created node.
		*/
		template<typename T>
		std::shared_ptr<Node> createNode()
		{
			if (_factoryMap.find(typeid(T)) == _factoryMap.end())
				throw std::runtime_error("Could not create a node for this type in this scene!");

			return _factoryMap[typeid(T)]->create();
		}

		/*!
		@brief Creates a node of type T from the scene's factories with the specified name.
		@tparam T The type ofnode to create.
		@return The newly created node.
		*/
		template<typename T>
		std::shared_ptr<Node> createNode(const std::string& name)
		{
			if (_factoryMap.find(typeid(T)) == _factoryMap.end())
				throw std::runtime_error("Could not create a node for this type in this scene!");

			return _factoryMap[typeid(T)]->create(name);
		}

	protected:
		/*!
		@brief Stores the factory in parameter to the scene's active factories, to enable simple Node creation with createNode.
		@see Orbit::Scene::createNode<T>()
		@tparam T The type of node registered with the factory.
		@param factory The factory to store.
		*/
		template<typename T>
		void storeFactory(std::unique_ptr<NodeFactory> factory)
		{
			static_assert(std::is_base_of_v<Node, T>, "Cannot store a factory for something that is not a node!");
			_factoryMap[typeid(T)] = std::move(factory);
		}

	private:
		/*! The scene's map of factories. */
		std::unordered_map<std::type_index, std::unique_ptr<NodeFactory>> _factoryMap;
	};
}

#endif //GAME_SCENE_H