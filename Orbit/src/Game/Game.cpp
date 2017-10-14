/// @file Game/Game.cpp

#include "Game/Game.h"

#include "Game/ModLibrary.h"

#include <Input/Window.h>
#include <Render/Renderer.h>

#include <Input/Input.h>

#include <Game/Scene.h>
#include <Game/Mod.h>
#include <Game/MainModule.h>

#include <Game/CompositeTree/CompositeTree.h>

#include <json.hpp>

#include <fstream>
#include <iostream>

using namespace Orbit;

Game::Game()
	: _currentScene(nullptr), _nextScene(nullptr), _tree(std::make_unique<CompositeTree>())
{ }

Game& Game::getInstance()
{
	static Game instance;
	return instance;
}

void Game::initialize()
{
	// Load up the main module of the game - getMainModule() is loaded at runtime (linked at compile time)
	_mainModule = std::unique_ptr<MainModule>(getMainModule());
	_mainModule->load();

	// TEMP
	glm::ivec2 windowSize = Input::getInput().windowSize();
	_projection.setFoV(glm::radians(45.f));
	_projection.setAspectRatio(windowSize.x / static_cast<float>(windowSize.y));
	_projection.setZNear(0.1f);
	_projection.setZFar(10.0f);

	// Load up all the mods
	std::ifstream moduleFile("mods.json");
	nlohmann::json j;
	moduleFile >> j;

	auto modNames = j["mods"];
	for (std::string modName : modNames)
	{
		std::unique_ptr<ModLibrary> modLib = std::make_unique<ModLibrary>("Mods\\" + modName + "\\" + modName);
		(*modLib)->load();
		_modStack.push(std::move(modLib));
	}

	// Load the initial scene
	std::unique_ptr<Scene> initialScene = _mainModule->getInitialScene();
	if (!initialScene)
		throw std::runtime_error("Main module did not contain an initial scene!");

	loadScene(std::move(initialScene));

	// Temporary test: register Spacebar to "Fire".
	Input::getInput().registerVirtualKey("Fire", Key::Code::Space);
}

void Game::cleanup()
{
	while (!_modStack.empty())
	{
		std::unique_ptr<ModLibrary> modLib = std::move(_modStack.top());
		_modStack.pop();

		(*modLib)->unload();
	}

	_mainModule->unload();
	_mainModule = nullptr;
}

void Game::update(std::chrono::nanoseconds elapsedTime)
{
	// Lock the mouse movement for the current frame.
	Input::_instance.lockMouseMovement();

	updateScene();

	_tree->update(elapsedTime);

	_tree->acceptVisitor(&_visitor);
	if (_visitor.modelCountsChanged())
		Window::getInstance().getRenderer()->loadModels(_visitor.modelCounts());

	if (_tree->getCamera() == nullptr)
		throw std::runtime_error("Scene has no camera to render!");

	glm::mat4 view = _tree->getCamera()->getViewMatrix();
	glm::mat4 projection = _projection.getMatrix();
	Window::getInstance().getRenderer()->setupViewProjection(view, projection);
	Window::getInstance().getRenderer()->queueRender(_visitor.treeState());

	_visitor.flushModelCounts();
}

void Game::loadScene(std::unique_ptr<Scene> scene)
{
	_nextScene = std::move(scene);
}

void Game::updateScene()
{
	if (!_nextScene)
		return;

	if (_currentScene)
		_currentScene->unload();

	_tree->clearChildren();

	_currentScene = std::move(_nextScene);
	_nextScene = nullptr;

	_currentScene->loadFactories();
	_currentScene->load(*_tree);
}