/*! @file Game/ModLibrary.h */

#ifndef GAME_MODLIBRARY_H
#define GAME_MODLIBRARY_H
#pragma once

#include <string>
#include <memory>

namespace Orbit
{
	class Mod;

	/*!
	@brief Abstraction of OS-specific dynamic library loading, where libraries are assumed to contain
	an API consumable as a Mod object.
	*/
	class ModLibrary final
	{
	public:
		/*!
		@brief Constructs an instance of the object using the library at name.
		@param name The name of the library, without extensions.
		*/
		ModLibrary(const std::string& name);

		/*!
		@brief Destructor for the class. Frees up library resources.
		*/
		~ModLibrary();

		/*!
		@brief Pointer dereferencing operator for the class. Returns a pointer to the underlying mod.
		@return A pointer to the underlying mod.
		*/
		Mod* operator->();

		/*! @copydoc ModLibrary::operator->() */
		const Mod* operator->() const;

		/*!
		@brief Getter for the library's mod.
		@return The library's mod.
		*/
		Mod* getMod();

		/*! @copydoc ModLibrary::getMod() */
		const Mod* getMod() const;

	private:
		/*! A handle to the OS-specific library. */
		void* _libraryHandle;
		/*! A unique, owning pointer to the library's created Mod object. */
		std::unique_ptr<Mod> _mod;
	};
}

#endif //GAME_MODLIBRARY_H