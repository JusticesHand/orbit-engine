/*! @file Game/ModLibrary.h */

#ifndef GAME_MODLIBRARY_H
#define GAME_MODLIBRARY_H
#pragma once

#include <string>
#include <memory>

namespace Orbit
{
	class Mod;

	class ModLibrary final
	{
	public:
		ModLibrary(const std::string& name);
		~ModLibrary();

		Mod* operator->();
		const Mod* operator->() const;

		Mod* getMod();
		const Mod* getMod() const;

	private:
		void* _libraryHandle;
		std::unique_ptr<Mod> _mod;
	};
}

#endif //GAME_MODLIBRARY_H