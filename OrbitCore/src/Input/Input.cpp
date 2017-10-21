/*! @file Input/Input.cpp */

#include "Input/Input.h"

using namespace Orbit;

bool Input::keyPressed(const Key& key) const
{
	return _keyStates[key.index()];
}

bool Input::keyPressed(const const_str& virtualKeyName) const
{
	std::pair<Key, bool> found = _virtualKeyMap.find(virtualKeyName);
	if (!found.second)
		return false;

	return _keyStates[found.first.index()];
}

glm::ivec2 Input::mouseDelta() const
{
	std::lock_guard<std::mutex> guard(_mutex);
	return _lockedMouseDelta;
}

glm::ivec2 Input::windowSize() const
{
	return _windowSize;
}

void Input::registerVirtualKey(const const_str& keyName, const Key& key)
{
	_virtualKeyMap[keyName] = key;
}

void Input::logKeyPress(const Key& key)
{
	_keyStates[key.index()] = true;
}

void Input::logKeyRelease(const Key& key)
{
	_keyStates[key.index()] = false;
}

void Input::accumulateMouseMovement(const glm::ivec2& amount)
{
	_mouseDelta += amount;
}

void Input::lockMouseMovement()
{
	std::lock_guard<std::mutex> guard(_mutex);
	_lockedMouseDelta = _mouseDelta;
	_mouseDelta = {};
}

void Input::setWindowSize(const glm::ivec2& newWindowSize)
{
	_windowSize = newWindowSize;
}