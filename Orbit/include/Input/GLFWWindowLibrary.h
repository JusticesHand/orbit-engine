/*! @file Input/GLFWWindowLibrary.h */

#ifndef INPUT_GLFWWINDOWLIBRARY_H
#define INPUT_GLFWWINDOWLIBRARY_H
#pragma once

#include "WindowLibrary.h"

#if defined(WINDOWLIB)
#error "WINDOWLIB was already defined elsewhere! Only one window library can be defined."
#endif

/*! Definition of the window library type. */
#define WINDOWLIB Orbit::GLFWWindowLibrary

namespace Orbit
{
	/*!
	@brief Implementation of the WindowLibrary class, using the GLFW windowing library.
	*/
	class GLFWWindowLibrary final : public WindowLibrary
	{
	public:
		/*!
		@brief Default constructor for the class. Initializes the library.
		*/
		explicit GLFWWindowLibrary();

		/*!
		@brief Destructor for the class. Deinitializes the library.
		*/
		virtual ~GLFWWindowLibrary();

		/*!
		@brief Returns a new instance of Orbit::Window, as an instance of Orbit::GLFWWindow.
		@param size The desired size of the window.
		@param title The desired title of the window.
		@param fullscreen Whether or not the window should be created full screen.
		*/
		std::unique_ptr<Window> createWindow(const glm::ivec2& size, const std::string& title, bool fullscreen) override;
	};
}

#endif //INPUT_GLFWWINDOWLIBRARY_H