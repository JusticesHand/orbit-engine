/*! @file Nodes/TestNode2.h */

#ifndef ORBITMAIN_NODES_TESTNODE2_H
#define ORBITMAIN_NODES_TESTNODE2_H
#pragma once

#include <Game/CompositeTree/Node.h>

namespace OrbitMain
{
	/*!
	@brief Second test node containing different behavioral code than the first one, but possibly the same model.
	Meant to test instancing code.
	*/
	class TestNode2 : public Orbit::Node
	{
	public:
		/*!
		@brief Initializes the node with the model in parameter, with the node's default name.
		@param input The input handler reference to use by the node.
		@param model The model to be adopted by the node.
		*/
		explicit TestNode2(const Orbit::Input& input, const std::shared_ptr<Orbit::Model>& model);

		/*!
		@brief Initializes the node with the name and model in parameter.
		@param input The input handler reference to use by the node.
		@param name The name of the node, for lookup purposes.
		@param model The model to be adopted by the node.
		*/
		explicit TestNode2(const Orbit::Input& input, const std::string& name, const std::shared_ptr<Orbit::Model>& model);

		/*!
		@brief Destructor for the class.
		*/
		virtual ~TestNode2() = default;

		/*!
		@brief Move constructor for the class. Calls the base class's move constructor.
		@param rhs The right hand side of the operation.
		*/
		TestNode2(TestNode2&& rhs);

		/*!
		@brief Move assignment operator for the class. Calls the base class's move assignment operator.
		@param rhs The right hand side of the operation.
		@return A reference to this.
		*/
		TestNode2& operator=(TestNode2&& rhs);

		TestNode2(const TestNode2&) = delete;
		TestNode2& operator=(const TestNode2&) = delete;

		/*!
		@brief Simple visitor design pattern implementation.
		@param visitor The visitor to accept.
		*/
		void acceptVisitor(Orbit::Visitor* visitor) override;

		/*!
		@brief Returns a cloned instance of the node.
		@return A cloned instance of the node.
		*/
		std::shared_ptr<Orbit::Node> clone() const override;

		/*!
		@brief Performs the update operation on the node for a tick.
		@param elapsedTime The elapsed time, in nanoseconds.
		*/
		void update(std::chrono::nanoseconds elapsedTime) override;
	};
}

#endif //ORBITMAIN_NODES_TESTNODE2_H