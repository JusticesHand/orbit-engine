#ifndef GAME_COMPOSITETREE_COMPOSITE_H
#define GAME_COMPOSITETREE_COMPOSITE_H
#pragma once

#include "Node.h"

#include <vector>

namespace Orbit
{
	class CompositeNode : public Node
	{
	public:
		explicit CompositeNode(const std::string& name);

		virtual ~CompositeNode() = 0;

		CompositeNode(const CompositeNode&) = delete;
		CompositeNode& operator=(const CompositeNode&) = delete;

		void acceptVisitor(const Visitor* visitor) override;

		void destroy() override;

		void addChild(std::shared_ptr<Node> child);
		void removeChild(std::shared_ptr<Node> child);

		std::shared_ptr<Node> find(std::string name) override;
		std::shared_ptr<const Node> find(std::string name) const override;

	protected:
		std::shared_ptr<CompositeNode> getParent();
		std::shared_ptr<const CompositeNode> getParent() const;

	private:
		std::weak_ptr<CompositeNode> _parent;
		std::vector<std::shared_ptr<Node>> _children;

	};

	inline CompositeNode::~CompositeNode() { }
}

#endif //GAME_COMPOSITETREE_COMPOSITE_H
