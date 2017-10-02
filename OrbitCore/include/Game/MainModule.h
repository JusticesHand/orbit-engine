#ifndef GAME_MAINMODULE_H
#define GAME_MAINMODULE_H
#pragma once

#include "Mod.h"
#include <memory>

namespace Orbit
{
	class Scene;

	class MainModule : public Mod
	{
	public:
		MainModule() = default;
		virtual ~MainModule() = 0;

		virtual std::unique_ptr<Scene> getInitialScene() = 0;
	};

	inline MainModule::~MainModule() = default;
}

#if defined(ORBIT)
#define ORBIT_API __declspec(dllimport)
#else
#define ORBIT_API __declspec(dllexport)
#endif

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