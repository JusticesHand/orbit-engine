/// @file Input/Window.cpp

#include "Input/Window.h"
#include <thread>

#include <GLFW/glfw3.h>

using namespace Orbit;

/// The implementation of the window's static instance.
Window Window::_instance;

/// Whether or not the window has been created already using createWindow(). This is useful as getInstance()
/// will fail if createWindow() hasn't been called once (and the window is in an invalid state). Ensures that
/// the program will fail fast and not silently.
static bool _windowCreated = false;

/// Essentially sets the window singleton's instance by calling the appropriate (private) constructor.
/// Also sets the flag indicating that the window was created at least once.
/// @param width the width of the window.
/// @param height the height of the window.
/// @param title the title of the window
/// @param fullscreen whether or no the window should be created in fullscreen.
void Window::createWindow(int width, int height, const std::string& title, bool fullscreen)
{
	_instance = Window(width, height, title, fullscreen);
	_windowCreated = true;
}

/// Returns the window's instance, per the Singleton design pattern. Fails fast if createWindow() hasn't been
/// called at least once.
/// @return the window single instance.
Window& Window::getInstance()
{
	if (!_windowCreated)
		throw std::runtime_error("The window was not created before being accessed!");

	return _instance;
}

/// The primary constructor for the window. Only sets the class's members - resource acquisition is done in
/// open() as to have a valid this pointer (and to simplify window show/hide code).
/// @param width the width of the window.
/// @param height the height of the window.
/// @param title the title of the window.
/// @param fullscreen whether or not the window should be created fullscreen.
Window::Window(int width, int height, const std::string& title, bool fullscreen)
	: _width(width), _height(height), _title(title), _fullscreen(fullscreen)
{
}

/// The destructor for the class. Destroys the window handle is appropriate. Also terminates the GLFW
/// library (it will be be reinitiated when the next window is opened, if any).
Window::~Window()
{
	if (_windowHandle != nullptr)
		glfwDestroyWindow(_windowHandle);

	glfwTerminate();
}

/// Opens the window. Begins by initiating the GLFW library, sets the hints for the window creation and sets
/// callbacks. Finally creates the window.
void Window::open()
{
	if (!glfwInit())
		throw std::runtime_error("Could not initialize the GLFW library!");

	// TODO: Instead of GLFW_NO_API, infer API from the chosen renderer type
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

	_windowHandle = glfwCreateWindow(_width, _height, _title.c_str(), nullptr, nullptr);
	glfwSetWindowUserPointer(_windowHandle, this);
}

/// "Closes" the window. Essentially sets the 'shouldClose' flag so the window is closed on the next run
/// loop. Throws an exception if the window wasn't opened.
void Window::close()
{
	if (_windowHandle == nullptr)
		throw std::runtime_error("Cannot close a window that was not opened!");

	glfwSetWindowShouldClose(_windowHandle, true);
}

/// Sets the window to a fullscreen (or windowed, depending on the parameter) mode. Throws an exception if
/// the window wasn't opened.
/// @param value the value of the fullscreen flag (true for fullscreen, false for windowed).
void Window::setFullscreen(bool value)
{
	if (_windowHandle == nullptr)
		throw std::runtime_error("Cannot change a state for a window that was not opened!");

	_fullscreen = value;

	if (_fullscreen)
	{
		// TODO: Instead of getting the primary monitor, make it customizable
		GLFWmonitor* monitor = glfwGetPrimaryMonitor();
		glfwSetWindowMonitor(_windowHandle, monitor, 0, 0, _width, _height, GLFW_DONT_CARE);
	}
	else
	{
		glfwSetWindowMonitor(_windowHandle, nullptr, 0, 0, _width, _height, GLFW_DONT_CARE);
	}
}

/// Returns whether or not the window should close. Simple syntactic sugar over the GLFW C API.
/// @return whether or not the window should close.
bool Window::shouldClose() const
{
	return glfwWindowShouldClose(_windowHandle);
}

/// Returns the renderer that was built with the window. Renderers are set at compile-time. Returns a raw
/// pointer as std::observer_ptr is not available in VS2017 as of this commit.
/// @return the renderer for the window.
Renderer* Window::getRenderer() const
{
	// TODO
	return nullptr;
}

/// Flags whether or not the window ticks its main loop at fastest speed or if it allows time for the
/// thread to sleep.
/// @param value the value of the flag.
void Window::constrainTicking(bool value)
{
	_tickingConstrained = value;
}

/// Determines the ideal amount of ticks each second for the main loop. Has no effect if constrainTicking has
/// not been called with true as parameter.
/// @param targetTicksPerSecond the target amount of ticks each second.
void Window::setTargetTicksPerSecond(unsigned int targetTicksPerSecond)
{
	_targetTicksPerSecond = targetTicksPerSecond;
}

/// Contains the main loop of the window. Ensures the window is correctly initialized and then launches
/// the main loop, calling the callback with the elapsed time between loops on every loop.
/// This throws if the window is already running - it is intended to only be run once on the main loop.
/// It also throws if the window has not been opened and if the callback is null.
/// @param callback the callback to call every loop.
void Window::run(TickingCallback callback)
{
	using namespace std::chrono;

	if (_running)
		throw std::runtime_error("The window is already running!");

	if (_windowHandle == nullptr)
		throw std::runtime_error("The window was not opened before running it!");

	if (callback == nullptr)
		throw std::runtime_error("Cannot run a window without a ticking callback!");

	_running = true;

	high_resolution_clock::time_point lastTime = high_resolution_clock::now();
	while (!shouldClose())
	{
		glfwPollEvents();

		high_resolution_clock::time_point currentTime = high_resolution_clock::now();
		nanoseconds elapsedTime = currentTime - lastTime;

		callback(elapsedTime);

		lastTime = currentTime;

		if (!_tickingConstrained)
			continue;

		high_resolution_clock::time_point postCurrentTime = high_resolution_clock::now();
		nanoseconds processingTime = postCurrentTime - currentTime;
		nanoseconds timePerCycle = nanoseconds(static_cast<long long>(1e9 / _targetTicksPerSecond));
		milliseconds waitTime = duration_cast<milliseconds>(timePerCycle - processingTime);

		high_resolution_clock::time_point waitPoint = postCurrentTime + waitTime;

		if (waitTime.count() > 0)
			std::this_thread::sleep_until(waitPoint);
	}
}