/*! @file Input/Window.cpp */

#include "Input/Window.h"

#include "Render/Renderer.h"

#include <Input/Input.h>

using namespace Orbit;

Window::Window(const glm::ivec2& size, const std::string& title, bool fullscreen)
	: _size(size), _title(title), _fullscreen(fullscreen), _input(std::make_unique<Input>())
{
}

Window::~Window() = default;

Renderer* Window::renderer() const
{
	return _renderer.get();
}

Input* Window::input() const
{
	return _input.get();
}

glm::ivec2 Window::size() const
{
	return _size;
}

glm::ivec2 Window::mousePosition() const
{
	return _mousePos;
}

void Window::setMousePosition(const glm::ivec2& pos)
{
	_mousePos = pos;
}
