/*! @file Input/Window.cpp */

#include "Input/Window.h"
#include "Input/Input.h"

#include "Render/Renderer.h"

#if defined(USE_OPENGL)
#include "Render/OpenGLRenderer.h"
#elif defined(USE_DIRECTX)
#include "Render/DirectXRenderer.h"
#else
// The vulkan renderer, being cross-platform and blazing fast (as well as most maintained here), is the safest choice.
#include "Render/VulkanRenderer.h"
#endif

#include <thread>

#include <GLFW/glfw3.h>

using namespace Orbit;

namespace
{
	Key getKey(int keyCode);
	Key getMouse(int keyCode);
}

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

Window::~Window()
{
	if (_windowHandle != nullptr)
	{
		glfwDestroyWindow(_windowHandle);
		glfwTerminate();
	}
}

void Window::open()
{
	if (!glfwInit())
		throw std::runtime_error("Could not initialize the GLFW library!");

	glfwSetErrorCallback([](int error, const char* desc) {
		std::cerr << "GLFW ERROR (" << error << "): " << desc << std::endl;
	});

	_renderer = std::make_unique<RENDERER>();

	glfwWindowHint(GLFW_CLIENT_API, static_cast<int>(_renderer->getAPI()));
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

	_windowHandle = glfwCreateWindow(_width, _height, _title.c_str(), nullptr, nullptr);

	glfwSetWindowUserPointer(_windowHandle, this);

	int sizeX, sizeY;
	glfwGetWindowSize(_windowHandle, &sizeX, &sizeY);

	glm::ivec2 windowSize = { sizeX, sizeY };
	Input::getInput().setWindowSize(windowSize);

	_renderer->init(_windowHandle, windowSize);

	// Not-input callback setups.
	glfwSetWindowSizeCallback(_windowHandle, [](GLFWwindow* windowHandle, int sizeX, int sizeY) {
		glm::ivec2 newSize{ sizeX, sizeY };

		Input::getInput().setWindowSize(newSize);
		Window::getInstance().getRenderer()->flagResize(newSize);
	});

	// Setup input.
	glfwSetKeyCallback(_windowHandle, [](GLFWwindow*, int keyCode, int, int action, int) {
		Key key = getKey(keyCode);

		if (action == GLFW_PRESS)
			Input::getInput().logKeyPress(key);
		else if (action == GLFW_RELEASE)
			Input::getInput().logKeyRelease(key);
	});

	glfwSetMouseButtonCallback(_windowHandle, [](GLFWwindow*, int buttonCode, int action, int) {
		Key button = getMouse(buttonCode);

		if (action == GLFW_PRESS)
			Input::getInput().logKeyPress(button);
		else if (action == GLFW_RELEASE)
			Input::getInput().logKeyPress(button);
	});

	glfwSetCursorPosCallback(_windowHandle, [](GLFWwindow*, double posX, double posY) {
		static int lastX = static_cast<int>(posX), lastY = static_cast<int>(posY);

		int deltaX = static_cast<int>(posX) - lastX;
		int deltaY = static_cast<int>(posY) - lastY;

		lastX = static_cast<int>(posX);
		lastY = static_cast<int>(posY);

		Input::getInput().accumulateMouseMovement({ deltaX, deltaY });
	});
}

void Window::close()
{
	if (_windowHandle == nullptr)
		throw std::runtime_error("Cannot close a window that was not opened!");

	glfwSetWindowShouldClose(_windowHandle, true);
}

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

bool Window::shouldClose() const
{
	return glfwWindowShouldClose(_windowHandle);
}

Renderer* Window::getRenderer() const
{
	return _renderer.get();
}

void Window::constrainTicking(bool value)
{
	_tickingConstrained = value;
}

void Window::setTargetTicksPerSecond(unsigned int targetTicksPerSecond)
{
	_targetTicksPerSecond = targetTicksPerSecond;
}

void Window::run(TickingCallback tickingCallback)
{
	using namespace std::chrono;

	if (_running)
		throw std::runtime_error("The window is already running!");

	if (_windowHandle == nullptr)
		throw std::runtime_error("The window was not opened before running it!");

	if (tickingCallback == nullptr)
		throw std::runtime_error("Cannot run a window without a ticking callback!");

	_running = true;

	high_resolution_clock::time_point lastTime = high_resolution_clock::now();
	while (!shouldClose())
	{
		glfwPollEvents();

		high_resolution_clock::time_point currentTime = high_resolution_clock::now();
		nanoseconds elapsedTime = currentTime - lastTime;

		tickingCallback(elapsedTime);

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

	Key getMouse(int buttonCode)
	{
		switch (buttonCode)
		{
		case GLFW_MOUSE_BUTTON_1: return Key::Code::Mouse1;
		case GLFW_MOUSE_BUTTON_2: return Key::Code::Mouse2;
		case GLFW_MOUSE_BUTTON_3: return Key::Code::Mouse3;
		case GLFW_MOUSE_BUTTON_4: return Key::Code::Mouse4;
		case GLFW_MOUSE_BUTTON_5: return Key::Code::Mouse5;

		default: return Key::Code::None;
		}
	}
}