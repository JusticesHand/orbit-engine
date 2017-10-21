/*! @file Scenes/LoadScene.h */

#ifndef ORBITMAIN_SCENE_LOADSCENE_H
#define ORBITMAIN_SCENE_LOADSCENE_H
#pragma once

#include <Game/Scene.h>

namespace OrbitMain
{
	/*!
	@brief Scene representing a loading screen, to be rendered while the game is waiting for
	something to load.
	*/
	class LoadScene final : public Orbit::Scene
	{
	public:
		/*!
		@brief Default constructor for the class.
		*/
		LoadScene() = default;

		/*!
		@brief Destructor for the class.
		*/
		virtual ~LoadScene() = default;

		/*!
		@brief Loads in the scene's node factories.
		*/
		void loadFactories(const Orbit::Input& input) override;

		/*!
		@brief Places the scene's initial object states.
		@param tree The tree containing the objects.
		*/
		void load(Orbit::CompositeTree& tree) override;

		/*!
		@brief Unloads the scene's specific data.
		*/
		void unload() override;
	};
}

#endif //ORBITMAIN_SCENE_LOADSCENE_H
