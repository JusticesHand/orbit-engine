/*! @file TestNodeFactory.h */

#ifndef FACTORIES_TESTNODEFACTORY_H
#define FACTORIES_TESTNODEFACTORY_H
#pragma once

#include <Game/Factories/NodeFactory.h>

#include <Render/Model.h>

namespace OrbitMain
{
	/*!
	@brief Implementation of the factory design pattern, outputting instances of OrbitMain::TestNode.
	*/
	class TestNodeFactory : public Orbit::NodeFactory
	{
	public:
		/*!
		@brief Constructs the factory with the model in parameter.
		@param input A reference to the input handler used by all the nodes.
		@param testNodeModel The model to be used in instances of TestNodes.
		*/
		TestNodeFactory(const Orbit::Input& input, std::shared_ptr<Orbit::Model> testNodeModel);

		/*!
		@brief Creates an instance of OrbitMain::TestNode.
		@return An instance of OrbitMain::TestNode.
		*/
		std::shared_ptr<Orbit::Node> create() const override;

		/*!
		@brief Creates an instance of OrbitMain::TestNode with the name in parameter.
		@param name The name to apply to the node.
		@return An instance of OrbitMain::TestNode.
		*/
		std::shared_ptr<Orbit::Node> create(const std::string& name) const override;

	private:
		/*! The model used by instances of TestNode. */
		const std::shared_ptr<Orbit::Model> _testNodeModel;
	};
}

#endif //FACTORIES_TESTNODEFACTORY_H