/*! @file Render/Projection.cpp */

#include "Render/Projection.h"

#include <glm/gtc/matrix_transform.hpp>

using namespace Orbit;

Projection::Projection(Type type)
	: _type(type)
{
}

glm::mat4 Projection::getMatrix() const
{
	switch (_type)
	{
	case Type::Perspective:
		return glm::perspective(_fov, _aspectRatio, _zNear, _zFar);
	case Type::Orthogonal:
		return glm::ortho(_left, _right, _bottom, _top, _zNear, _zFar);
	default:
		throw std::runtime_error("Projection type is invalid!");
	}
}

void Projection::switchType(Type newType)
{
	_type = newType;
}

float Projection::zNear() const
{
	return _zNear;
}

float Projection::zFar() const
{
	return _zFar;
}

float Projection::fov() const
{
	return _fov;
}

float Projection::aspectRatio() const
{
	return _aspectRatio;
}

float Projection::left() const
{
	return _left;
}

float Projection::right() const
{
	return _right;
}

float Projection::bottom() const
{
	return _bottom;
}

float Projection::top() const
{
	return _top;
}

void Projection::setZNear(float zNear)
{
	_zNear = zNear;
}

void Projection::setZFar(float zFar)
{
	_zFar = zFar;
}

void Projection::setFoV(float angle)
{
	_fov = angle;
}

void Projection::setAspectRatio(float aspectRatio)
{
	_aspectRatio = aspectRatio;
}

void Projection::setLeft(float left)
{
	_left = left;
}

void Projection::setRight(float right)
{
	_right = right;
}

void Projection::setBottom(float bottom)
{
	_bottom = bottom;
}

void Projection::setTop(float top)
{
	_top = top;
}