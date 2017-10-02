/// @file Game/Scene.h

#ifndef GAME_SCENE_H
#define GAME_SCENE_H
#pragma once

namespace Orbit
{
	class CompositeTree;

	class Scene
	{
	public:
		Scene() = default;
		virtual ~Scene() = 0;

		virtual void registerFactories() = 0;
		virtual void load(CompositeTree& tree) = 0;
		virtual void unload() = 0;
	};

	inline Scene::~Scene() = default;
}

#endif //GAME_SCENE_H