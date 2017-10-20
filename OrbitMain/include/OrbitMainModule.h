/*! @file OrbitMainModule.h */

#ifndef ORBITMAINMODULE_H
#define ORBITMAINMODULE_H
#pragma once

#include "Game/MainModule.h"

namespace OrbitMain
{
	/*!
	@brief Definition of the MainModule for this assembly.
	*/
	class OrbitMainModule final : public Orbit::MainModule
	{
	public:
		/*!
		@brief Default constructor for the class.
		*/
		OrbitMainModule() = default;

		/*!
		@brief Destructor for the class.
		*/
		virtual ~OrbitMainModule() = default;

		/*!
		@brief Loads in the module's custom data.
		*/
		void load() override;

		/*!
		@brief Unloads the module's custom data.
		*/
		void unload() override;

		/*!
		@brief Initializes and returns the initial scene to be run by the engine.
		@return The initial scene used by the engine.
		*/
		std::unique_ptr<Orbit::Scene> getInitialScene() override;
	};
}

#endif //ORBITMAINMODULE_H