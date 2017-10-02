/*! @file Input/Input.cpp */

#include "Input/Input.h"

using namespace Orbit;

Input Input::_instance;

const Input& Input::getInput()
{
	return _instance;
}

bool Input::keyPressed(const Key& key) const
{
	return _keyStates[key.index()];
}

bool Input::mousePressed(const Mouse& button) const
{
	return _mouseStates[button.index()];
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

void Input::logKeyPress(const Key& key)
{
	_keyStates[key.index()] = true;
}

void Input::logKeyRelease(const Key& key)
{
	_keyStates[key.index()] = false;
}

void Input::logMousePress(const Mouse& button)
{
	_mouseStates[button.index()] = true;
}

void Input::logMouseRelease(const Mouse& button)
{
	_mouseStates[button.index()] = false;
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