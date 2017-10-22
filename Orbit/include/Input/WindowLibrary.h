/*! @file Input/WindowLibrary.h */

#ifndef INPUT_WINDOWLIBRARY_H
#define INPUT_WINDOWLIBRARY_H
#pragma once

#include <memory>
#include <string>

#include <glm/glm.hpp>

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
		@param size The window's desired size.
		@param title The window's desired title.
		@param fullscreen Whether the window should be created as fullscreen or not.
		*/
		virtual std::unique_ptr<Window> createWindow(const glm::ivec2& size, const std::string& title, bool fullscreen) = 0;
	};
}

#endif //INPUT_WINDOWLIBRARY_H