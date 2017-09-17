#include <iostream>

#include "Input/Window.h"

using namespace Orbit;

int main(int argc, char* argv[])
{
	using namespace std::chrono;

	std::cout << "Hello World!" << std::endl;

	Window::createWindow(1280, 720, "Hello World");

	Window::getInstance().constrainTicking(true);
	Window::getInstance().setTargetTicksPerSecond(120);

	Window::getInstance().open();
	Window::getInstance().run([](nanoseconds time)
	{
		static nanoseconds accumulatedTime = nanoseconds::zero();
		accumulatedTime += time;
		if (accumulatedTime >= seconds(1))
		{
			accumulatedTime = nanoseconds::zero();
			std::cout << "A second has passed." << std::endl;
		}
	});

	return 0;
}