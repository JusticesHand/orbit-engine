/*! @file Game/Game.h */

#ifndef GAME_GAME_H
#define GAME_GAME_H
#pragma once

#include <string>
#include <chrono>
#include <memory>
#include <stack>

#include "Visitors/ModelVisitor.h"
#include <Render/Projection.h>

namespace Orbit
{
	class Scene;
	class CompositeTree;
	class MainModule;
	class ModLibrary;
	class Input;

	/*!
	@brief Class wrapping up general game-related logic, like scene handling, projection, composite tree state,
	as well as general update. Implements the Singleton pattern.
	*/
	class Game final
	{
	public:
		/*! 
		@brief Returns the game's single instance.
		@return The game's single instance.
		*/
		static Game& getInstance();

		/*!
		@brief Destructor for the class.
		*/
		~Game() = default;

		Game(const Game&) = delete;
		Game& operator=(const Game&) = delete;

		/*!
		@brief Initializes the game. Retrieves the main module's initial scene and handles transactions.
		*/
		void initialize();
		
		/*!
		@brief Cleans up the game.
		*/
		void cleanup();

		/*!
		@brief Passes on the elapsed time to the inner tree to generate a game tick.
		@param elapsedTime The elapsed time since the last tick.
		*/
		void update(std::chrono::nanoseconds elapsedTime);
		
	private:
		/*!
		@brief Default constructor for the class. Sets most members to default values.
		*/
		Game();

		/*! Sets the next scene to the scene in parameter. It will be loaded on the next frame. */
		void loadScene(std::unique_ptr<Scene> scene);

		/*! Handles scene changing transactions. */
		void updateScene();

		/*! The scene currently running in the game. Should never be nullptr except during initialization. */
		std::unique_ptr<Scene> _currentScene;
		/*! The scene that should be switched on the next (or current) frame. Should mostly be nullptr, except when a transition is necessary. */
		std::unique_ptr<Scene> _nextScene;

		/*! The composite tree containing the game's nodes. */
		std::unique_ptr<CompositeTree> _tree;

		/*! The main module for the game. */
		std::unique_ptr<MainModule> _mainModule;
		/*! The stack of mods running within the game. */
		std::stack<std::unique_ptr<ModLibrary>> _modStack;

		/*! The game's projection. */
		Projection _projection{ Projection::Type::Perspective };

		/*! The game's visitor, retrieving model state data. */
		ModelVisitor _visitor;
	};
}

#endif //GAME_GAME_H