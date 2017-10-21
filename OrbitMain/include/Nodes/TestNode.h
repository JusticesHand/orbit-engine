/*! @file Nodes/TestNode.h */

#ifndef ORBITMAIN_NODES_TESTNODE_H
#define ORBITMAIN_NODES_TESTNODE_H
#pragma once

#include <Game/CompositeTree/Node.h>

namespace OrbitMain
{
	/*!
	@brief First test node meant to test scene-wide input handling. Essentially serves as a trigger holder
	for the scene - as in, the node is never modified, but it outputs stuff to the console and handles scene
	changes as a result of inputs.
	*/
	class TestNode : public Orbit::Node
	{
	public:
		/*!
		@brief Initializes the node with the model in parameter, using the node's default name.
		@param model The model to assign to the node.
		*/
		explicit TestNode(const Orbit::Input& input, const std::shared_ptr<Orbit::Model>& model);

		/*!
		@brief Initializes the node with the name and model in parameter.
		@param name The name of the node, for lookup purposes.
		@param model The model to assign to the node.
		*/
		explicit TestNode(const Orbit::Input& input, const std::string& name, const std::shared_ptr<Orbit::Model>& model);

		/*!
		@brief Destructor for the class.
		*/
		virtual ~TestNode() = default;

		/*!
		@brief Move constructor for the class. Calls the base class's move constructor.
		@param rhs The right hand side of the operation.
		*/
		TestNode(TestNode&& rhs);

		/*!
		@brief Move assignment operator for the class. Calls the base class's move assignment operator.
		@param rhs The right hand side of the operation.
		@return A reference to this.
		*/
		TestNode& operator=(TestNode&& rhs);

		TestNode(const TestNode&) = delete;
		TestNode& operator=(const TestNode&) = delete;

		/*!
		@brief Simple implementation of the visitor design pattern.
		@param visitor The visitor to accept.
		*/
		void acceptVisitor(Orbit::Visitor* visitor) override;

		/*!
		@brief Returns a cloned instance of the node.
		@return A cloned instance of the node.
		*/
		std::shared_ptr<Orbit::Node> clone() const override;

		/*!
		@brief Update code for the node.
		@param elapsedTime The elapsed time, in nanoseconds.
		*/
		void update(std::chrono::nanoseconds elapsedTime) override;

	private:
		/*! Accumulated time for the node - it outputs something every second. */
		std::chrono::nanoseconds _accumulatedTime = std::chrono::nanoseconds::zero();
	};
}

#endif //ORBITMAIN_NODES_TESTNODE_H