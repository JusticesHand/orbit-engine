/// @file Game/Game.h

#ifndef GAME_GAME_H
#define GAME_GAME_H
#pragma once

#include <string>
#include <chrono>
#include <memory>
#include <stack>

namespace Orbit
{
	class Scene;
	class CompositeTree;
	class MainModule;
	class ModLibrary;
	class Input;

	class Game final
	{
	public:
		static Game& getInstance();

		~Game() = default;

		Game(const Game&) = delete;
		Game& operator=(const Game&) = delete;

		void initialize();
		void cleanup();

		void update(std::chrono::nanoseconds elaspedTime);
		
	private:
		Game();

		std::unique_ptr<Scene> _currentScene;
		std::unique_ptr<Scene> _nextScene;

		std::unique_ptr<CompositeTree> _tree;

		std::unique_ptr<MainModule> _mainModule;
		std::stack<std::unique_ptr<ModLibrary>> _modStack;

		void loadScene(std::unique_ptr<Scene> scene);

		void updateScene();
	};
}

#endif //GAME_GAME_H