/// @file Game/Mod.h

#ifndef GAME_MOD_H
#define GAME_MOD_H
#pragma once

namespace Orbit
{
	class Mod
	{
	public:
		Mod() = default;
		virtual ~Mod() = 0;

		virtual void load() = 0;
		virtual void unload() = 0;
	};

	inline Mod::~Mod() = default;
}

#if !defined(ORBIT)
#define ORBIT_API __declspec(dllexport)

#include <memory>

extern "C"
{
	ORBIT_API Orbit::Mod* getMod();
}

#endif

#endif //GAME_MODULE_H