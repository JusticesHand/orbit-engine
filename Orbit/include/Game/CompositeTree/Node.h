#ifndef GAME_COMPOSITETREE_NODE_H
#define GAME_COMPOSITETREE_NODE_H
#pragma once

#include <memory>
#include <string>
#include <mutex>

namespace Orbit
{
	class Visitor;

	class Node : public std::enable_shared_from_this<Node>
	{
	public:
		explicit Node(const std::string& name);

		virtual ~Node() = 0;

		Node(Node&& rhs);
		Node& operator=(Node&& rhs);

		Node(const Node&) = delete;
		Node& operator=(const Node&) = delete;

		virtual void acceptVisitor(const Visitor* visitor) = 0;

		virtual std::shared_ptr<Node> clone() = 0;

		virtual void destroy();

		std::string getName() const;

		bool destroyed() const;

		virtual std::shared_ptr<Node> find(std::string name);
		virtual std::shared_ptr<const Node> find(std::string name) const;

	protected:
		void setDestroyed(bool value);
		mutable std::mutex _mutex;

		template<typename Derived>
		std::shared_ptr<Derived> shared_from_base()
		{
			static_assert(std::is_base_of_v<Node, Derived>, "Derived must derive from Node!");
			return std::static_pointer_cast<Derived>(shared_from_this());
		}

	private:
		bool _destroyed = false;
		std::string _name;
	};

	inline Node::~Node() { }
}

#endif //GAME_COMPOSITETREE_NODE_H
