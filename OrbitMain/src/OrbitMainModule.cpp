/*! @file OrbitMainModule.cpp */

#include "OrbitMainModule.h"

#include "Scenes/LoadScene.h"

#include <iostream>

using namespace OrbitMain;

void OrbitMainModule::load()
{
}

void OrbitMainModule::unload()
{
}

std::unique_ptr<Orbit::Scene> OrbitMainModule::getInitialScene()
{
	return std::make_unique<LoadScene>();
}

extern "C"
{
	/*!
	@brief Main export function of the assembly. Returns an instance of the MainModule.
	@return An instance of the main module.
	*/
	ORBIT_API Orbit::MainModule* getMainModule()
	{
		return new OrbitMainModule();
	}
}