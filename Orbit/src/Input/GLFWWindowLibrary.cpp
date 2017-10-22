/*! @file Input/GLFWWindowLibrary.cpp */

#include "Input/GLFWWindowLibrary.h"

#include "Input/GLFWWindow.h"

#include <GLFW/glfw3.h>

using namespace Orbit;

GLFWWindowLibrary::GLFWWindowLibrary()
{
	if (!glfwInit())
		throw std::runtime_error("Could not initialize GLFW!");
}

GLFWWindowLibrary::~GLFWWindowLibrary()
{
	glfwTerminate();
}

std::unique_ptr<Window> GLFWWindowLibrary::createWindow(const glm::ivec2& size, const std::string& title, bool fullscreen)
{
	return std::make_unique<GLFWWindow>(size, title, fullscreen);
}