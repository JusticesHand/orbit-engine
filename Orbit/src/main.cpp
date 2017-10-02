/// @file main.cpp

#include <cstdlib>
#include <iostream>

#include "Input/Window.h"
#include "Render/Renderer.h"
#include "Game/Game.h"

using namespace Orbit;

int main(int argc, char* argv[])
{
	// TODO: Handle args, etc etc
	try
	{
		Game::getInstance().initialize();

		// TODO: Initialization with options.
		Window::createWindow(1280, 720, "Hello World");

		Window::getInstance().constrainTicking(true);
		Window::getInstance().setTargetTicksPerSecond(120);

		Window::getInstance().open();
		Window::getInstance().run([](std::chrono::nanoseconds time)
		{
			Game::getInstance().update(time);
			Window::getInstance().getRenderer()->renderFrame();
		});
	}
	catch (std::exception& ex)
	{
		// TODO: Logging.
		std::cerr << "Caught exception: " << ex.what() << std::endl;
		return EXIT_FAILURE;
	}
	catch (...)
	{
		std::cerr << "Caught unknown exception type!" << std::endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}