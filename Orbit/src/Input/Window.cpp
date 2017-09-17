/// @file Input/Window.cpp

#include "Input/Window.h"
#include <thread>

#include <GLFW/glfw3.h>

using namespace Orbit;

Window Window::_instance;

static bool _windowCreated = false;

void Window::createWindow(int width, int height, const std::string& title, bool fullscreen)
{
	_instance = Window(width, height, title, fullscreen);
	_windowCreated = true;
}

Window& Window::getInstance()
{
	if (!_windowCreated)
		throw std::runtime_error("The window was not created before being accessed!");

	return _instance;
}

Window::Window(int width, int height, const std::string& title, bool fullscreen)
	: _width(width), _height(height), _title(title), _fullscreen(fullscreen)
{
}

Window::~Window()
{
	if (_windowHandle != nullptr)
		glfwDestroyWindow(_windowHandle);

	glfwTerminate();
}

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

void Window::close()
{
	if (_windowHandle == nullptr)
		throw std::runtime_error("Cannot close a window that was not opened!");

	setShouldClose(true);
}

void Window::setFullscreen(bool value)
{
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

void Window::setShouldClose(bool value)
{
	glfwSetWindowShouldClose(_windowHandle, value);
}

bool Window::shouldClose() const
{
	return glfwWindowShouldClose(_windowHandle);
}

Renderer* Window::getRenderer() const
{
	// TODO
	return nullptr;
}

void Window::constrainTicking(bool value)
{
	_tickingConstrained = value;
}

void Window::setTargetTicksPerSecond(int targetTicksPerSecond)
{
	_targetTicksPerSecond = targetTicksPerSecond;
}

void Window::run(TickingCallback callback)
{
	using namespace std::chrono;

	if (_windowHandle == nullptr)
		throw std::runtime_error("The window was not opened before running it!");

	if (callback == nullptr)
		throw std::runtime_error("Cannot run a window without a ticking callback!");

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