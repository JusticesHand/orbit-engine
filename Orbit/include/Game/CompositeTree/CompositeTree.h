#ifndef GAME_COMPOSITETREE_COMPOSITETREE_H
#define GAME_COMPOSITETREE_COMPOSITETREE_H
#pragma once

#include "CompositeNode.h"

namespace Orbit
{
	class CompositeTree : public CompositeNode
	{
	public:
		CompositeTree();
		
		virtual ~CompositeTree();

		CompositeTree(CompositeTree&& rhs);
		CompositeTree& operator=(CompositeTree&& rhs);

		CompositeTree(const CompositeTree&) = delete;
		CompositeTree& operator=(const CompositeTree&) = delete;
	};
}

#endif //GAME_COMPOSITETREE_COMPOSITETREE_H
