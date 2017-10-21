/*! @file Input/GLFWWindow.h */

#ifndef INPUT_GLFWWINDOW_H
#define INPUT_GLFWWINDOW_H
#pragma once

#include "Window.h"

namespace Orbit
{
	class GLFWWindow final : public Window
	{
	public:
		/*!
		@brief Constructor for the class. Only sets the property values for the class (by calling the base constructor),
		as initialization is to be done when calling the GLFWWindow::open() method.
		@param width The requested width of the window.
		@param height The requested height of the window.
		@param title The requested title of the window.
		@param fullscreen Whether or not the window should be fullscreen.
		*/
		explicit GLFWWindow(int width, int height, const std::string& title, bool fullscreen);

		/*!
		@brief Ensures that the window is properly cleaned up.
		*/
		virtual ~GLFWWindow();

		/*! @copydoc Window::open() */
		void open() override;

		/*! @copydoc Window::close() */
		void close() override;

		/*! @copydoc Window::setFullscreen(bool) */
		void setFullscreen(bool value) override;

		/*! @copydoc Window::shouldClose() const */
		bool shouldClose() const override;

		/*! @copydoc Window::handleMessages() */
		void handleMessages() override;

	private:
		/*! Handle for the window. */
		void* _windowHandle = nullptr;
	};
}

#endif //INPUT_GLFWWINDOW_H