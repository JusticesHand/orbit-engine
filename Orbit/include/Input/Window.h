/// @file Input/Window.h

#ifndef INPUT_WINDOW_H
#define INPUT_WINDOW_H
#pragma once

#include <string>
#include <chrono>
#include <memory>

struct GLFWwindow;

namespace Orbit
{
	class Renderer; /// Forward declaration for the Renderer class.
	class Input;

	/// A class encapsulating windowing operations and allows for running work load on the side.
	/// This class implements the Singleton pattern, meaning its one instance should be acquired using
	/// getInstance(). Furthermore, before acquiring an instance, it should be initialized to a correct state
	/// using createWindow().
	class Window final
	{
	public:
		/// Sets the window singleton to a window with the defined parameters.
		/// @param width the width of the window, in pixels.
		/// @param height the height of the window, in pixels.
		/// @param title the name of the window, which turns up in the task bar.
		/// @param fullscreen whether or not the window should be fullscreen on startup.
		static void createWindow(int width, int height, const std::string& title, bool fullscreen = false);

		/// Returns the one instance of this class, as per the Singleton design pattern.
		/// @return the single window instance.
		static Window& getInstance();

		/// Destroys the window. Also cleans up the GLFW library.
		~Window();

		/// Opens the window. This has the effect of acquiring resources to open the physical window and then
		/// opening it to display things to the user. This must be called before calling other member methods,
		/// as otherwise their behaviour will be undefined.
		void open();
		
		/// Closes the window. Useful in other threads where it is necessary to signal that the program should
		/// end. Care should be taken that other threads exit in a proper manner before calling this method,
		/// as it usually will mean that the main thread will exit (leaving the program with orphaned threads).
		void close();

		/// Changes the window's state from fullscreen to windowed.
		/// @param value true if fullscreen, false if windowed.
		void setFullscreen(bool value);

		/// Returns whether or not the window has been flagged as 'should close'.
		/// @return whether or not the window should close.
		bool shouldClose() const;

		/// Returns the renderer built alongside the window. The renderer allows graphics to be drawn on the
		/// Window surface. Renderers can be changed at compile-time. Returns a raw pointer as an
		/// std::observer_ptr type is unavailable.
		/// @return the renderer for the window.
		Renderer* getRenderer() const;

		/// Defines whether or not the updating loop should be capped in terms of calls per second.
		/// This allows fine-tuning performance requirements for the thread in which the window will run.
		/// Setting this will attempt to make the thread sleep until the next scheduled time (determined by
		/// the ticks per second, defined by setTargetTicksPerSecond()).
		/// @param value whether or not to constrain ticking.
		void constrainTicking(bool value);

		/// Determines the amount of ticks per second the window thread (when calling run()) should attempt to
		/// cap itself to allow more processing time for other tasks. Has no effect if ticking hasn't been
		/// constrained.
		/// @param targetTicksPerSecond the target amount of ticks every second, in Hz.
		void setTargetTicksPerSecond(unsigned int targetTicksPerSecond);

		/// A typedef simplifying the use of nanosecond-using function pointers.
		using TickingCallback = void(*)(std::chrono::nanoseconds elapsedTime);

		/// Takes control of the current thread and executes callbacks to the callback in parameter. To 
		/// control behaviour, use the contrainTicking() and setTargetTicksPerSecond() methods. Before 
		/// using the callback, handles all OS messages.
		/// @param tickingCallback the callback to run every tick.
		void run(TickingCallback tickingCallback);

	private:
		/// Default constructor for the class. Called once, to set up an invalid window as the singleton at
		/// the start of the program.
		Window() = default;

		/// Main constructor for the class. Called when createWindow() is called - it is only used to update
		/// the instance of the Singleton.
		Window(int width, int height, const std::string& title, bool fullscreen);

		Window(const Window&) = delete;
		Window& operator=(const Window&) = delete;

		Window(Window&& rhs);
		Window& operator=(Window&& rhs);

		GLFWwindow* _windowHandle = nullptr; ///< The handle of the window, using GLFW.

		int _width; ///< The width of the window.
		int _height; ///< The height of the window.

		bool _tickingConstrained = false; ///< Whether or not the window should attempt to tick as fast as possible.
		unsigned int _targetTicksPerSecond = 60; ///< The target amount of ticks per second.

		bool _fullscreen; ///< Whether or not the window is currently in fullscreen mode.
		std::string _title; ///< The title of the window.

		bool _running = false; ///< Whether or not the scene is currently running.

		std::unique_ptr<Renderer> _renderer;

		static Window _instance; ///< The instance of the window.
	};
}

#endif //INPUT_WINDOW_H
