/*! @file Input/Win32WindowLibrary.h */

#ifndef INPUT_WIN32WINDOWLIBRARY_H
#define INPUT_WIN32WINDOWLIBRARY_H
#pragma once

#if defined(_WIN32)

#include "WindowLibrary.h"

namespace Orbit
{
	/*!
	@brief Implementation of the WindowLibrary class, using Win32 code.
	As it is Win32, it doesn't need any library initializations and therefore has only default
	constructor/destructors.
	*/
	class Win32WindowLibrary final : public WindowLibrary
	{
	public:
		/*!
		@brief Returns a new instance of Orbit::Window, as an instance of Orbit::Win32Window.
		@param width The desired width of the window.
		@param height The desired height of the window.
		@param title The desired title of the window.
		@param fullscreen Whether or not the window should be created full screen.
		*/
		std::unique_ptr<Window> createWindow(int width, int height, std::string title, bool fullscreen) override;
	};
}

#endif

#endif //INPUT_WIN32WINDOWLIBRARY_H