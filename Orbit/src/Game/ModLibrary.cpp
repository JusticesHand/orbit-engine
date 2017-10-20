/*! @file Game/ModLibrary.cpp */

#include "Game/ModLibrary.h"

#include <Game/Mod.h>

using namespace Orbit;

Mod* ModLibrary::operator->()
{
	return getMod();
}

const Mod* ModLibrary::operator->() const
{
	return getMod();
}

Mod* ModLibrary::getMod()
{
	return _mod.get();
}

const Mod* ModLibrary::getMod() const
{
	return _mod.get();
}

/*! Simple using statement to simplify callback code. */
using GetModFunc = Mod*(*)();

#if defined(_WIN32)

#include <Windows.h>

ModLibrary::ModLibrary(const std::string& name)
	: _libraryHandle(nullptr), _mod(nullptr)
{
	std::string libraryName = name + ".dll";

	_libraryHandle = LoadLibrary(libraryName.c_str());
	if (!_libraryHandle)
		throw std::runtime_error("Could not load library " + libraryName + "!");

	GetModFunc modFunc = (GetModFunc)GetProcAddress((HMODULE)_libraryHandle, "getMod");
	if (!modFunc)
		throw std::runtime_error("Library " + libraryName + " is malformatted!");

	_mod = std::unique_ptr<Mod>(modFunc());
}

ModLibrary::~ModLibrary()
{
	_mod.reset();
	FreeLibrary((HMODULE)_libraryHandle);
}

#elif defined(__linux__)
#error "Linux library support hasn't been implemented yet!"
#else
#error "Dynamic library loading hasn't been defined for this system!"
#endif
