/*! @file Input/Input.h */

#ifndef INPUT_INPUT_H
#define INPUT_INPUT_H
#pragma once

#include <array>
#include <atomic>
#include <mutex>

#include <glm/glm.hpp>

#include "Key.h"
#include "Util.h"

namespace Orbit
{
	/*!
	@brief The main input class, giving read (and minimal write) access to the state of input for the engine.
	Implements the Singleton pattern as the input states must be consistant between modules.
	*/
	class Input final
	{
		// Only the window can set the input states.
		friend class Window;

		// The game class synchronizes mouse movement so nodes have a usable mouse delta.
		friend class Game;

	public:
		/*!
		@brief Returns the single Input instance.
		@return The single Input instance.
		*/
		ORBIT_CORE_API static Input& getInput();

		/*!
		@brief Returns true if the Key in parameter is pressed.
		@param key The key whose state is polled.
		@return The state of the key in parameter.
		*/
		ORBIT_CORE_API bool keyPressed(const Key& key) const;

		/*!
		@brief Returns true if the virtual key in parameter is pressed.
		@param virtualKeyName The name of the virtual key pressed.
		@return The state of the key in parameter, or false if the key name doesn't correspond to any key.
		*/
		ORBIT_CORE_API bool keyPressed(const const_str& virtualKeyName) const;

		/*!
		@brief Returns the mouse delta for the current update cycle (i.e. since the last time lockMouseMovement() was called).
		@return The mouse delta.
		*/
		ORBIT_CORE_API glm::ivec2 mouseDelta() const;

		/*!
		@brief Returns the size of the window.
		@return The size of the window.
		*/
		ORBIT_CORE_API glm::ivec2 windowSize() const;

		/*!
		@brief Registers a virtual key name to a key type.
		Calling this function on an already registered name binds the new key to the name - the old key is lost.
		@param keyName The name of the key.
		@param key The key on which to bind it.
		*/
		ORBIT_CORE_API void registerVirtualKey(const const_str& keyName, const Key& key);

	private:
		/*!
		@brief Default constructor for the class.
		*/
		Input() = default;

		/*!
		@brief Logs a key press. Sets the flag of the key.
		@param key The key that was pressed.
		*/
		ORBIT_CORE_API void logKeyPress(const Key& key);

		/*!
		@brief Logs a key release. Clears the flag of the key.
		@param key The key that was released.
		*/
		ORBIT_CORE_API void logKeyRelease(const Key& key);

		/*!
		@brief Accumulates the amount of movement with the saved mouse delta.
		@param amount The amount accumulated.
		*/
		ORBIT_CORE_API void accumulateMouseMovement(const glm::ivec2& amount);

		/*!
		@brief Switches the accumulated mouse delta to the saved value that will be returned by mouseDelta().
		Clears the accumulated amount.
		*/
		ORBIT_CORE_API void lockMouseMovement();

		/*!
		@brief Sets the window size member.
		@param newWindowSize The new window size.
		*/
		ORBIT_CORE_API void setWindowSize(const glm::ivec2& newWindowSize);

		/*! A bi-directional map that maps keys to their virtual names. */
		bimap<const_str, Key> _virtualKeyMap;
		/*! An array of key states. */
		std::array<std::atomic<bool>, Key::count()> _keyStates;

		/*! The size of the window. */
		glm::ivec2 _windowSize;

		/*! The singleton's instance. */
		static Input _instance;

		/*! A mutex to handle synchronization. */
		mutable std::mutex _mutex;
		/*! The accumulated mouse delta. */
		glm::ivec2 _mouseDelta;
		/*! The locked mouse delta, returned by the mouseDelta() function. */
		glm::ivec2 _lockedMouseDelta;
	};
}

#endif //INPUT_INPUT_H