/*! @file Task/TaskRunner.h */

#ifndef TASK_TASKRUNNER_H
#define TASK_TASKRUNNER_H
#pragma once

#include <atomic>
#include <chrono>
#include <thread>
#include <functional>
#include <vector>

namespace Orbit
{
	/*!
	@brief Helper class to run multiple threads of stuff at the same time in an easy-to-use format.
	*/
	class TaskRunner final
	{
	public:
		/*!
		@brief Ensures that all the threads close up correctly.
		*/
		~TaskRunner();

		/*!
		@brief Runs the function defined by run in the current thread, taking control of the thread.
		@param targetTPS The target amount of ticks per second.
		@param end The end condition, taken as a function execution.
		@param run The function to actually run on every tick.
		*/
		void run(
			size_t targetTPS,
			const std::function<bool()>& end,
			const std::function<void()>& run);

		/*!
		@brief Runs the function defined by run in the current thread, taking control of the thread, passing time.
		@param targetTPS The target amount of ticks per second.
		@param end The end condition, taken as a function execution.
		@param run The function to actually run on every tick.
		*/
		void run(
			size_t targetTPS,
			const std::function<bool()>& end, 
			const std::function<void(std::chrono::nanoseconds)>& run);

		/*!
		@brief Runs the function defined by run on a separate thread.
		@param targetTPS The target amount of ticks per second.
		@param end The end condition, taken as a function execution.
		@param run The function to actually run on every tick.
		*/
		void runAsync(
			size_t targetTPS, 
			const std::function<bool()>& end,
			const std::function<void()>& run);

		/*!
		@brief Runs the function defined by run on a separate thread, passing time.
		@param targetTPS the target amount of ticks per second.
		@param end The end condition, taken as a function execution.
		@param run The function to actually run on every tick.
		*/
		void runAsync(
			size_t targetTPS, 
			const std::function<bool()>& end,
			const std::function<void(std::chrono::nanoseconds)>& run);

		/*!
		@brief Signals all thread that they should join, and therefore end execution.
		*/
		void joinAll();

		/*!
		@brief Returns the value of the shouldJoin property.
		@return The value of the shouldJoin property.
		*/
		bool shouldJoin() const;

	private:
		/*!
		@brief Actual implementation of the function that is run either on the current thread (bare) or on another thread
		(wrapped by an std::thread).
		@param parentRunner A reference to the parent TaskRunner object, to check if it should join.
		@param targetTPS The target amount of ticks per second for the function. Sleeps otherwise.
		@param end The end condition, taken as a function execution.
		@param run The function to actually run on every tick.
		*/
		static void run_func(
			const TaskRunner& parentRunner,
			size_t targetTPS, 
			const std::function<bool()>& end, 
			const std::function<void()>& run);

		/*! @copydoc TaskRunner:run_func() */
		static void run_func_tick(
			const TaskRunner& parentRunner,
			size_t targetTPS,
			const std::function<bool()>& end,
			const std::function<void(std::chrono::nanoseconds)>& run);

		/*! Whether or not every task in the runner should join. */
		std::atomic<bool> _shouldJoin = false;

		/*! Collection of threads owned by the class. */
		std::vector<std::thread> _threads;
	};
}

#endif //TASK_TASKRUNNER_H