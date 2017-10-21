/*! @file Input/WindowLibrary.h */

#ifndef INPUT_WINDOWLIBRARY_H
#define INPUT_WINDOWLIBRARY_H
#pragma once

#include <memory>
#include <string>

namespace Orbit
{
	class Window;

	/*!
	@brief Class abstracting the windowing library itself. Used mainly as a factory-like object
	to create window instances without having a strong link between the library and window.
	*/
	class WindowLibrary
	{
	public:
		/*!
		@brief Serves as the factory method for the class. Returns a new instance of a window with the elements in parameter.
		@param width The window's desired width.
		@param height The window's desired height.
		@param title The window's desired title.
		@param fullscreen Whether the window should be created as fullscreen or not.
		*/
		virtual std::unique_ptr<Window> createWindow(int width, int height, const std::string& title, bool fullscreen) = 0;
	};
}

#endif //INPUT_WINDOWLIBRARY_H