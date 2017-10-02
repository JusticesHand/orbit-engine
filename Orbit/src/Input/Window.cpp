/// @file Input/Window.cpp

#include "Input/Window.h"
#include "Input/Input.h"

#include <Render/Renderer.h>

#if defined(USE_OPENGL)
#include "Render/OpenGLRenderer.h"
#elif defined(USE_DIRECTX)
#include "Render/DirectXRenderer.h"
#else
// The vulkan renderer, being cross-platform and blazing fast, is the safest choice.
#include "Render/VulkanRenderer.h"
#endif

#include <thread>

#include <GLFW/glfw3.h>

using namespace Orbit;

namespace
{
	Key getKey(int keyCode);
	Mouse getMouse(int buttonCode);
}

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

Window::Window(Window&& rhs)
	: _windowHandle(rhs._windowHandle),
	_width(rhs._width),
	_height(rhs._height),
	_title(rhs._title),
	_fullscreen(rhs._fullscreen),
	_running(rhs._running),
	_tickingConstrained(rhs._tickingConstrained),
	_targetTicksPerSecond(rhs._targetTicksPerSecond),
	_renderer(std::move(rhs._renderer))
{
	rhs._windowHandle = nullptr;
}

Window& Window::operator=(Window&& rhs)
{
	_windowHandle = rhs._windowHandle;
	_width = rhs._width;
	_height = rhs._height;
	_title = rhs._title;
	_fullscreen = rhs._fullscreen;
	_running = rhs._running;
	_tickingConstrained = rhs._tickingConstrained;
	_targetTicksPerSecond = rhs._targetTicksPerSecond;
	_renderer = std::move(rhs._renderer);

	rhs._windowHandle = nullptr;

	return *this;
}

/// The destructor for the class. Destroys the window handle is appropriate. Also terminates the GLFW
/// library (it will be be reinitiated when the next window is opened, if any).
Window::~Window()
{
	if (_windowHandle != nullptr)
	{
		glfwDestroyWindow(_windowHandle);
		glfwTerminate();
	}
}

/// Opens the window. Begins by initiating the GLFW library, sets the hints for the window creation and sets
/// callbacks. Finally creates the window.
void Window::open()
{
	if (!glfwInit())
		throw std::runtime_error("Could not initialize the GLFW library!");

	glfwSetErrorCallback([](int error, const char* desc) {
		std::cerr << "GLFW ERROR (" << error << "): " << desc << std::endl;
	});

	_renderer = std::make_unique<RENDERER>();

	glfwWindowHint(GLFW_CLIENT_API, static_cast<int>(_renderer->getAPI()));
	//glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

	_windowHandle = glfwCreateWindow(_width, _height, _title.c_str(), nullptr, nullptr);

	glfwSetWindowUserPointer(_windowHandle, this);

	int sizeX, sizeY;
	glfwGetWindowSize(_windowHandle, &sizeX, &sizeY);

	glm::ivec2 windowSize = { sizeX, sizeY };
	Input::_instance.setWindowSize(windowSize);

	_renderer->init(_windowHandle, windowSize);

	// Not-input callback setups.
	glfwSetWindowSizeCallback(_windowHandle, [](GLFWwindow* windowHandle, int sizeX, int sizeY) {
		std::cout << "RESIZED THE WINDOW" << std::endl;

		glm::ivec2 newSize{ sizeX, sizeY };

		Input::_instance.setWindowSize(newSize);
		Window::getInstance().getRenderer()->flagResize(newSize);
	});

	// Setup input.
	glfwSetKeyCallback(_windowHandle, [](GLFWwindow*, int keyCode, int, int action, int) {
		Key key = getKey(keyCode);

		if (action == GLFW_PRESS)
		{
			std::cout << "PRESSED " << key << std::endl;
			Input::_instance.logKeyPress(key);
		}
		else if (action == GLFW_RELEASE)
		{
			std::cout << "RELEASED " << key << std::endl;
			Input::_instance.logKeyRelease(key);
		}
	});

	glfwSetMouseButtonCallback(_windowHandle, [](GLFWwindow*, int buttonCode, int action, int) {
		Mouse button = getMouse(buttonCode);

		if (action == GLFW_PRESS)
			Input::_instance.logMousePress(button);
		else if (action == GLFW_RELEASE)
			Input::_instance.logMouseRelease(button);
	});

	glfwSetCursorPosCallback(_windowHandle, [](GLFWwindow*, double posX, double posY) {
		static int lastX = static_cast<int>(posX), lastY = static_cast<int>(posY);

		int deltaX = static_cast<int>(posX) - lastX;
		int deltaY = static_cast<int>(posY) - lastY;

		lastX = static_cast<int>(posX);
		lastY = static_cast<int>(posY);

		Input::_instance.accumulateMouseMovement({ deltaX, deltaY });
	});
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
	return _renderer.get();
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

namespace
{
	Key getKey(int keyCode)
	{
		switch (keyCode)
		{
		case GLFW_KEY_A: return Key::Code::A;
		case GLFW_KEY_B: return Key::Code::B;
		case GLFW_KEY_C: return Key::Code::C;
		case GLFW_KEY_D: return Key::Code::D;
		case GLFW_KEY_E: return Key::Code::E;
		case GLFW_KEY_F: return Key::Code::F;
		case GLFW_KEY_G: return Key::Code::G;
		case GLFW_KEY_H: return Key::Code::H;
		case GLFW_KEY_I: return Key::Code::I;
		case GLFW_KEY_J: return Key::Code::J;
		case GLFW_KEY_K: return Key::Code::K;
		case GLFW_KEY_L: return Key::Code::L;
		case GLFW_KEY_M: return Key::Code::M;
		case GLFW_KEY_N: return Key::Code::N;
		case GLFW_KEY_O: return Key::Code::O;
		case GLFW_KEY_P: return Key::Code::P;
		case GLFW_KEY_Q: return Key::Code::Q;
		case GLFW_KEY_R: return Key::Code::R;
		case GLFW_KEY_S: return Key::Code::S;
		case GLFW_KEY_T: return Key::Code::T;
		case GLFW_KEY_U: return Key::Code::U;
		case GLFW_KEY_V: return Key::Code::V;
		case GLFW_KEY_W: return Key::Code::W;
		case GLFW_KEY_X: return Key::Code::X;
		case GLFW_KEY_Y: return Key::Code::Y;
		case GLFW_KEY_Z: return Key::Code::Z;
		case GLFW_KEY_ESCAPE: return Key::Code::Esc;
		case GLFW_KEY_F1: return Key::Code::F1;
		case GLFW_KEY_F2: return Key::Code::F2;
		case GLFW_KEY_F3: return Key::Code::F3;
		case GLFW_KEY_F4: return Key::Code::F4;
		case GLFW_KEY_F5: return Key::Code::F5;
		case GLFW_KEY_F6: return Key::Code::F6;
		case GLFW_KEY_F7: return Key::Code::F7;
		case GLFW_KEY_F8: return Key::Code::F8;
		case GLFW_KEY_F9: return Key::Code::F9;
		case GLFW_KEY_F10: return Key::Code::F10;
		case GLFW_KEY_F11: return Key::Code::F11;
		case GLFW_KEY_F12: return Key::Code::F12;
		case GLFW_KEY_PRINT_SCREEN: return Key::Code::PrntScrn;
		case GLFW_KEY_SCROLL_LOCK: return Key::Code::ScrLk;
		case GLFW_KEY_INSERT: return Key::Code::Insert;
		case GLFW_KEY_HOME: return Key::Code::Home;
		case GLFW_KEY_PAGE_UP: return Key::Code::PgUp;
		case GLFW_KEY_PAGE_DOWN: return Key::Code::PgDown;
		case GLFW_KEY_DELETE: return Key::Code::Delete;
		case GLFW_KEY_END: return Key::Code::End;
		case GLFW_KEY_UP: return Key::Code::Up;
		case GLFW_KEY_DOWN: return Key::Code::Down;
		case GLFW_KEY_LEFT: return Key::Code::Left;
		case GLFW_KEY_RIGHT: return Key::Code::Right;
		case GLFW_KEY_GRAVE_ACCENT: return Key::Code::Tilde;
		case GLFW_KEY_1: return Key::Code::N1;
		case GLFW_KEY_2: return Key::Code::N2;
		case GLFW_KEY_3: return Key::Code::N3;
		case GLFW_KEY_4: return Key::Code::N4;
		case GLFW_KEY_5: return Key::Code::N5;
		case GLFW_KEY_6: return Key::Code::N6;
		case GLFW_KEY_7: return Key::Code::N7;
		case GLFW_KEY_8: return Key::Code::N8;
		case GLFW_KEY_9: return Key::Code::N9;
		case GLFW_KEY_0: return Key::Code::N0;
		case GLFW_KEY_MINUS: return Key::Code::Minus;
		case GLFW_KEY_EQUAL: return Key::Code::Equal;
		case GLFW_KEY_TAB: return Key::Code::Tab;
		case GLFW_KEY_CAPS_LOCK: return Key::Code::CapsLock;
		case GLFW_KEY_LEFT_SHIFT: return Key::Code::LShift;
		case GLFW_KEY_LEFT_CONTROL: return Key::Code::LCtrl;
		case GLFW_KEY_LEFT_SUPER: return Key::Code::LSuper;
		case GLFW_KEY_LEFT_ALT: return Key::Code::LAlt;
		case GLFW_KEY_RIGHT_SHIFT: return Key::Code::RShift;
		case GLFW_KEY_RIGHT_CONTROL: return Key::Code::RCtrl;
		case GLFW_KEY_RIGHT_ALT: return Key::Code::RAlt;
		case GLFW_KEY_RIGHT_SUPER: return Key::Code::RSuper;
		case GLFW_KEY_SPACE: return Key::Code::Space;
		case GLFW_KEY_BACKSPACE: return Key::Code::Backspace;
		case GLFW_KEY_ENTER: return Key::Code::Return;
		case GLFW_KEY_BACKSLASH: return Key::Code::Backslash;
		case GLFW_KEY_SLASH: return Key::Code::Slash;
		case GLFW_KEY_PERIOD: return Key::Code::Period;
		case GLFW_KEY_COMMA: return Key::Code::Comma;
		case GLFW_KEY_SEMICOLON: return Key::Code::Semicolon;
		case GLFW_KEY_APOSTROPHE: return Key::Code::Apostrophe;
		case GLFW_KEY_LEFT_BRACKET: return Key::Code::LeftBracket;
		case GLFW_KEY_RIGHT_BRACKET: return Key::Code::RightBracket;

		default: return Key::Code::None;
		}
	}

	Mouse getMouse(int buttonCode)
	{
		switch (buttonCode)
		{
		case GLFW_MOUSE_BUTTON_1: return Mouse::Code::Mouse1;
		case GLFW_MOUSE_BUTTON_2: return Mouse::Code::Mouse2;
		case GLFW_MOUSE_BUTTON_3: return Mouse::Code::Mouse3;
		case GLFW_MOUSE_BUTTON_4: return Mouse::Code::Mouse4;
		case GLFW_MOUSE_BUTTON_5: return Mouse::Code::Mouse5;

		default: return Mouse::Code::None;
		}
	}
}