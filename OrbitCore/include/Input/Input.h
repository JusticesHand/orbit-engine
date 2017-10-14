/*! @file Input/Input.h */

#ifndef INPUT_INPUT_H
#define INPUT_INPUT_H
#pragma once

#include <array>
#include <atomic>
#include <mutex>
#include <unordered_map>

#include <glm/glm.hpp>

#include "Key.h"
#include "Util.h"

namespace Orbit
{
	class Input final
	{
		// Only the window can set the input states.
		friend class Window;

		// The game class synchronizes mouse movement so nodes have a usable mouse delta.
		friend class Game;

	public:
		ORBIT_CORE_API static Input& getInput();

		ORBIT_CORE_API bool keyPressed(const Key& key) const;

		ORBIT_CORE_API bool keyPressed(const const_str& virtualKeyName) const;

		ORBIT_CORE_API glm::ivec2 mouseDelta() const;

		ORBIT_CORE_API glm::ivec2 windowSize() const;

		ORBIT_CORE_API void registerVirtualKey(const const_str& keyName, const Key& key);

	private:
		Input() = default;

		ORBIT_CORE_API void logKeyPress(const Key& key);
		ORBIT_CORE_API void logKeyRelease(const Key& key);

		ORBIT_CORE_API void accumulateMouseMovement(const glm::ivec2& amount);
		ORBIT_CORE_API void lockMouseMovement();

		ORBIT_CORE_API void setWindowSize(const glm::ivec2& newWindowSize);

		bimap<const_str, Key> _virtualKeyMap;
		std::array<std::atomic<bool>, Key::count()> _keyStates;

		glm::ivec2 _windowSize;

		ORBIT_CORE_API static Input _instance;

		mutable std::mutex _mutex;
		glm::ivec2 _mouseDelta;
		glm::ivec2 _lockedMouseDelta;
	};
}

#endif //INPUT_INPUT_H