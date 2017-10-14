/*! @file Scenes/LoadScene.h */

#ifndef ORBITMAIN_SCENE_LOADSCENE_H
#define ORBITMAIN_SCENE_LOADSCENE_H
#pragma once

#include <Game/Scene.h>

namespace OrbitMain
{
	class LoadScene final : public Orbit::Scene
	{
	public:
		LoadScene() = default;
		virtual ~LoadScene() = default;

		void loadFactories() override;
		void load(Orbit::CompositeTree& tree) override;
		void unload() override;
	};
}

#endif //ORBITMAIN_SCENE_LOADSCENE_H
