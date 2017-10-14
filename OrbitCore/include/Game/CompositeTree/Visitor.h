/*! @file Game/CompositeTree/Visitor.h */

#ifndef GAME_COMPOSITETREE_VISITOR_H
#define GAME_COMPOSITETREE_VISITOR_H
#pragma once

#include "Util.h"

namespace Orbit
{
	class Node;

	class Visitor
	{
	public:
		Visitor() = default;
		virtual ~Visitor() = 0;

		virtual void visitElement(Node* node) = 0;
	};

	inline Visitor::~Visitor() = default;
}

#endif //GAME_COMPOSITETREE_VISITOR_H