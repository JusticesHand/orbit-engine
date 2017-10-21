/*! @file Input/Window.cpp */

#include "Input/Window.h"

#include "Render/Renderer.h"

#include <Input/Input.h>

using namespace Orbit;

Window::Window(int width, int height, const std::string& title, bool fullscreen)
	: _width(width), _height(height), _title(title), _fullscreen(fullscreen)
{
}

Window::~Window() = default;

Renderer* Window::getRenderer() const
{
	return _renderer.get();
}

Input* Window::getInput() const
{
	return _input.get();
}

void Window::getMousePosition(int& x, int& y)
{
	x = _x;
	y = _y;
}

void Window::setMousePosition(int x, int y)
{
	_x = x;
	_y = y;
}
