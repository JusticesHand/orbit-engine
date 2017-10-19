/*! @file Game/Mod.h */

#ifndef GAME_MOD_H
#define GAME_MOD_H
#pragma once

#include "Util.h"

namespace Orbit
{
	/*!
	@brief Base class defining a Mod for the Orbit engine. Basically a small wrapper for load/unload methods for
	dynamically loaded libraries.
	*/
	class Mod
	{
	public:
		/*!
		@brief The class's default constructor.
		*/
		Mod() = default;

		/*!
		@brief The class's destructor.
		*/
		virtual ~Mod() = 0;

		/*!
		@brief Load method for the mod. Called when the mod is first loaded in memory.
		*/
		virtual void load() = 0;
		
		/*!
		@brief Unload method for the mod. Called when the mod is unloaded (usually at the program's end).
		*/
		virtual void unload() = 0;
	};

	inline Mod::~Mod() = default;
}

#if !defined(ORBIT)

extern "C"
{
	/*!
	@brief Declaration for the Mod-getting signature of the module. Intended to be one-per-mod in assemblies.
	@return The mod instance.
	*/
	ORBIT_API Orbit::Mod* getMod();
}

#endif

#endif //GAME_MODULE_H