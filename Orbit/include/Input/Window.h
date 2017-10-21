/*! @file Input/Window.h */

#ifndef INPUT_WINDOW_H
#define INPUT_WINDOW_H
#pragma once

#include <string>
#include <memory>

namespace Orbit
{
	class Input;
	class Renderer;

	/*!
	@brief A class encapsulating windowing operations. Specializations handle the actual work.
	*/
	class Window
	{
	public:
		/*!
		@brief Constructor for the class. Simply sets the properties of the class to those in parameter -
		all the actual work is to be done by specializations.
		@param width The requested width of the window.
		@param height The requested height of the window.
		@param title The requested title of the window.
		@param fullscreen Whether or not the window should be fullscreen.
		*/
		explicit Window(int width, int height, const std::string& title, bool fullscreen);

		/*!
		@brief Destructor for the class.
		*/
		virtual ~Window();

		/*!
		@brief Opens the window. This has the effect of acquiring resources to open the physical window and then
		opening it to display things to the user. This must be called before calling other member methods,
		as otherwise their behaviour will be undefined.
		*/
		virtual void open() = 0;
		
		/*!
		@brief Closes the window. Useful in other threads where it is necessary to signal that the program should
		end. Care should be taken that other threads exit in a proper manner before calling this method,
		as it usually will mean that the main thread will exit (leaving the program with orphaned threads).
		*/
		virtual void close() = 0;

		/*!
		@brief Changes the window's state from fullscreen to windowed.
		@param value true if fullscreen, false if windowed.
		*/
		virtual void setFullscreen(bool value) = 0;

		/*!
		@brief Returns whether or not the window has been flagged as 'should close'.
		@return Whether or not the window should close.
		*/
		virtual bool shouldClose() const = 0;

		/*!
		@brief Handles the OS-specific messages pending for the window. This (usually) triggers callbacks that 
		*/
		virtual void handleMessages() = 0;

		/*!
		@brief Returns the renderer built alongside the window. The renderer allows graphics to be drawn on the
		Window surface. Renderers can be changed at compile-time. Returns a raw pointer as an std::observer_ptr
		type is unavailable in the vc++ in VS Community 2017.
		@return The renderer for the window.
		*/
		virtual Renderer* getRenderer() const;

		/*!
		@brief Returns the input handler built alongside the window. Ideally this input handler would be created
		alongside the window and be registered to use its callbacks (otherwise it doesn't work at all and it would
		be quite sad). Since std::observer_ptr isn't available in the vc++ version in VS Community 2017, raw pointers
		will have to do.
		@return The input handler for the window.
		*/
		virtual Input* getInput() const;

		/*!
		@brief Getter for the mouse position. Useful in callbacks.
		@param[out] x The x position of the mouse.
		@param[out] y The y position of the mouse.
		*/
		void getMousePosition(int& x, int& y);

		/*!
		@brief Setter for the mouse position. Useful in callbacks.
		@param x The x position of the mouse.
		@param y The y position of the mouse.
		*/
		void setMousePosition(int x, int y);

	protected:
		/*! The width of the window. */
		int _width;
		/*! The height of the window. */
		int _height;

		/*! The title of the window. */
		std::string _title;
		/*! Whether or not the window is currently in fullscreen mode. */
		bool _fullscreen;

		/*! A unique pointer to the renderer associated with the window. */
		std::unique_ptr<Renderer> _renderer;

		/*! A unique pointer to the window's input. */
		std::unique_ptr<Input> _input;

		/*! Positions for the mouse. */
		int _x = 0, _y = 0;
	};
}

#endif //INPUT_WINDOW_H
