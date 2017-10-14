/*! @file Nodes/TestNode2.h */

#ifndef ORBITMAIN_NODES_TESTNODE2_H
#define ORBITMAIN_NODES_TESTNODE2_H
#pragma once

#include <Game/CompositeTree/Node.h>

namespace OrbitMain
{
	class TestNode2 : public Orbit::Node
	{
	public:
		explicit TestNode2(const std::shared_ptr<Orbit::Model>& model);
		explicit TestNode2(const std::string& name, const std::shared_ptr<Orbit::Model>& model);
		virtual ~TestNode2() = default;

		TestNode2(TestNode2&& rhs);
		TestNode2& operator=(TestNode2&& rhs);

		TestNode2(const TestNode2&) = delete;
		TestNode2& operator=(const TestNode2&) = delete;

		void acceptVisitor(Orbit::Visitor* visitor) override;

		std::shared_ptr<Orbit::Node> clone() const override;

		void update(std::chrono::nanoseconds elapsedTime) override;
	};
}

#endif //ORBITMAIN_NODES_TESTNODE2_H