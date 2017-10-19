/*! @file Game/MainModule.h */

#ifndef GAME_MAINMODULE_H
#define GAME_MAINMODULE_H
#pragma once

#include "Mod.h"
#include "Util.h"
#include <memory>

namespace Orbit
{
	class Scene;

	/*!
	@brief Defines the main module to be loaded by the Orbit engine. Essentially a glorified mod whose symbols aren't
	dynamically loaded application-side and is expected to exist when the application loads.
	*/
	class MainModule : public Mod
	{
	public:
		/*!
		@brief Default constructor for the class.
		*/
		MainModule() = default;

		/*!
		@brief Destructor for the class.
		*/
		virtual ~MainModule() = 0;

		/*!
		@brief Returns the module's initial scene.
		@return The module's initial scene.
		*/
		virtual std::unique_ptr<Scene> getInitialScene() = 0;
	};

	inline MainModule::~MainModule() = default;
}

extern "C"
{
	/*!
	@brief Returns the main module that will be used by the engine. It is to be implemented by the (single)
	library that contains the module.
	@return the main module.
	*/
	ORBIT_API Orbit::MainModule* getMainModule();
}

#endif //GAME_MAINMODULE_H