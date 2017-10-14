/// @file Game/Scene.h

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

	class Scene
	{
	public:
		Scene() = default;
		virtual ~Scene() = 0;

		virtual void loadFactories() = 0;
		virtual void load(CompositeTree& tree) = 0;
		virtual void unload() = 0;

		template<typename T>
		std::shared_ptr<Node> createNode()
		{
			if (_factoryMap.find(typeid(T)) == _factoryMap.end())
				throw std::runtime_error("Could not create a node for this type in this scene!");

			return _factoryMap[typeid(T)]->create();
		}

		template<typename T>
		std::shared_ptr<Node> createNode(const std::string& name)
		{
			if (_factoryMap.find(typeid(T)) == _factoryMap.end())
				throw std::runtime_error("Could not create a node for this type in this scene!");

			return _factoryMap[typeid(T)]->create(name);
		}

		ORBIT_CORE_API const std::vector<std::shared_ptr<Model>>& getModels() const;

	protected:
		ORBIT_CORE_API void storeModel(std::shared_ptr<Model> model);

		template<typename T>
		void storeFactory(std::unique_ptr<NodeFactory> factory)
		{
			_factoryMap[typeid(T)] = std::move(factory);
		}

	private:
		std::unordered_map<std::type_index, std::unique_ptr<NodeFactory>> _factoryMap;
		std::vector<std::shared_ptr<Model>> _models;
	};

	inline Scene::~Scene() = default;
}

#endif //GAME_SCENE_H