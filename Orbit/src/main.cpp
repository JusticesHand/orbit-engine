/*! @file main.cpp */

#include <cstdlib>
#include <iostream>
#include <memory>

#include "Input/WindowLibrary.h"
#include "Input/Window.h"
#include "Render/Renderer.h"
#include "Game/Game.h"

#include "Task/TaskRunner.h"

#if defined(USE_WIN32)
#include "Input/Win32WindowLibrary.h"
#elif defined(USE_XWINDOW)
#error XWindowLibrary is not implemented yet!
#elif defined(USE_WAYLAND)
#error WaylandWindowLibrary is not implemented yet!
#else
#include "Input/GLFWWindowLibrary.h"
#endif

using namespace Orbit;

/*!
@brief Main function of the program. Same signature as all other main functions.
@param argc The amount of arguments.
@param argv The argument strings.
*/
int main(int argc, char* argv[])
{
	// TODO: Handle args, etc etc
	try
	{
		std::unique_ptr<WindowLibrary> windowLib = std::make_unique<WINDOWLIB>();

		// TODO: Initialization with options.
		std::unique_ptr<Window> window = windowLib->createWindow(1280, 720, "Hello World", false);

		window->open();

		TaskRunner runner;
		Game game{ *window, runner };

		game.initialize();

		// Note that the game's shouldClose() function is directly linked to the window's.
		// TODO: Instead of doing rendering on main thread, check if -server is in parameters. Then pipe console commands
		// to the game's hypothetical command pipeline.
		runner.run(120, [&window]() {
			return window->shouldClose();
		},
		[&window]() {
			window->handleMessages();
			window->getRenderer()->renderFrame();
		});

		runner.joinAll();
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