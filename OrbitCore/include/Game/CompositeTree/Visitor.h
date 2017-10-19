/*! @file Game/CompositeTree/Visitor.h */

#ifndef GAME_COMPOSITETREE_VISITOR_H
#define GAME_COMPOSITETREE_VISITOR_H
#pragma once

#include "Util.h"

namespace Orbit
{
	class Node;

	/*!
	@brief Base class implementing the Visitor design pattern.
	*/
	class Visitor
	{
	public:
		/*!
		@brief Default constructor for the class.
		*/
		Visitor() = default;
		
		/*!
		@brief Default destructor for the class.
		*/
		virtual ~Visitor() = 0;

		/*!
		@brief Main chunk of the visitor pattern.
		@param node The node to visit.
		*/
		virtual void visitElement(Node* node) = 0;
	};

	inline Visitor::~Visitor() = default;
}

#endif //GAME_COMPOSITETREE_VISITOR_H