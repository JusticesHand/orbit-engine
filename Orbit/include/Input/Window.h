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
	class Renderer;

	class Window final
	{
	public:
		static void createWindow(int width, int height, const std::string& title, bool fullscreen = false);
		static Window& getInstance();

		~Window();

		void open();
		void close();

		void setFullscreen(bool value);

		void setShouldClose(bool value);
		bool shouldClose() const;

		Renderer* getRenderer() const;

		void constrainTicking(bool value);
		void setTargetTicksPerSecond(int targetTicksPerSecond);

		using TickingCallback = void(*)(std::chrono::nanoseconds elapsedTime);
		void run(TickingCallback tickingCallback);

	private:
		Window() = default;
		Window(int width, int height, const std::string& title, bool fullscreen);

		Window(const Window&) = default;
		Window& operator=(const Window&) = default;

		GLFWwindow* _windowHandle = nullptr;

		int _width;
		int _height;

		bool _tickingConstrained = false;
		int _targetTicksPerSecond = 60;

		bool _fullscreen;
		std::string _title;

		static Window _instance;
	};
}

#endif //INPUT_WINDOW_H
