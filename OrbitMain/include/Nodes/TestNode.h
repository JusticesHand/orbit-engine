/*! @file Nodes/TestNode.h */

#ifndef ORBITMAIN_NODES_TESTNODE_H
#define ORBITMAIN_NODES_TESTNODE_H
#pragma once

#include <Game/CompositeTree/Node.h>

namespace OrbitMain
{
	class TestNode : public Orbit::Node
	{
	public:
		explicit TestNode();
		virtual ~TestNode() = default;

		TestNode(TestNode&& rhs);
		TestNode& operator=(TestNode&& rhs);

		TestNode(const TestNode&) = delete;
		TestNode& operator=(const TestNode&) = delete;

		void acceptVisitor(const Orbit::Visitor* visitor) override;

		std::shared_ptr<Orbit::Node> clone() const override;

		void update(std::chrono::nanoseconds elapsedTime) override;

	private:
		std::chrono::nanoseconds _accumulatedTime = std::chrono::nanoseconds::zero();
	};
}

#endif //ORBITMAIN_NODES_TESTNODE_H