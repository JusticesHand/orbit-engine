/*! @file Task/TaskRunner.cpp */

#include "Task/TaskRunner.h"

using namespace Orbit;

TaskRunner::~TaskRunner()
{
	joinAll();
}

void TaskRunner::run(
	size_t targetTPS,
	const std::function<bool()>& end, 
	const std::function<void()>& run)
{
	run_func(*this, targetTPS, end, run);
}

void TaskRunner::run(
	size_t targetTPS, 
	const std::function<bool()>& end,
	const std::function<void(std::chrono::nanoseconds)>& run)
{
	run_func_tick(*this, targetTPS, end, run);
}

void TaskRunner::runAsync(
	size_t targetTPS,
	const std::function<bool()>& end,
	const std::function<void()>& run)
{
	_threads.push_back(std::thread(run_func, std::cref(*this), targetTPS, end, run));
}

void TaskRunner::runAsync(
	size_t targetTPS,
	const std::function<bool()>& end,
	const std::function<void(std::chrono::nanoseconds)>& run)
{
	_threads.push_back(std::thread(run_func_tick, std::cref(*this), targetTPS, end, run));
}

void TaskRunner::joinAll()
{
	_shouldJoin = true;

	while (!_threads.empty())
	{
		std::thread thread = std::move(_threads.back());
		_threads.pop_back();

		thread.join();
	}
}

bool TaskRunner::shouldJoin() const
{
	return _shouldJoin;
}

void TaskRunner::run_func(
	const TaskRunner& parentRunner,
	size_t targetTPS,
	const std::function<bool()>& end,
	const std::function<void()>& run)
{
	using namespace std::chrono;

	const nanoseconds targetTime{ (seconds(1) / nanoseconds(1)) / targetTPS };
	steady_clock::time_point time = steady_clock::now();

	while (!(parentRunner.shouldJoin() || end()))
	{
		time = steady_clock::now();

		try
		{
			run();
		}
		catch (std::exception& ex)
		{
			// TODO: Properly log this
			throw;
		}
		catch (...)
		{
			// TODO: Properly log this
			throw;
		}

		steady_clock::time_point nextTime = time + targetTime;
		std::this_thread::sleep_until(nextTime);
	}
}

void TaskRunner::run_func_tick(
	const TaskRunner& parentRunner,
	size_t targetTPS,
	const std::function<bool()>& end,
	const std::function<void(std::chrono::nanoseconds)>& run)
{
	using namespace std::chrono;

	const nanoseconds targetTime{ (seconds(1) / nanoseconds(1)) / targetTPS };
	steady_clock::time_point lastTime = steady_clock::now();

	while (!(parentRunner.shouldJoin() || end()))
	{
		steady_clock::time_point currentTime = steady_clock::now();
		nanoseconds elapsedTime = currentTime - lastTime;
		lastTime = currentTime;

		run(elapsedTime);

		steady_clock::time_point nextTime = currentTime + targetTime;
		std::this_thread::sleep_until(nextTime);
	}
}