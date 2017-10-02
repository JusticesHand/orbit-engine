/*! @file OrbitMainModule.cpp */

#include "OrbitMainModule.h"

#include "Scenes/LoadScene.h"

#include <iostream>

using namespace OrbitMain;

void OrbitMainModule::load()
{
	// TODO
	std::cout << typeid(*this).name() << std::endl;
}

void OrbitMainModule::unload()
{
	// TODO
}

std::unique_ptr<Orbit::Scene> OrbitMainModule::getInitialScene()
{
	return std::make_unique<LoadScene>();
}

extern "C"
{
	ORBIT_API Orbit::MainModule* getMainModule()
	{
		return new OrbitMainModule();
	}
}