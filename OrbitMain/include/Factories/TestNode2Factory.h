/*! @file TestNode2Factory.h */

#ifndef FACTORIES_TESTNODE2FACTORY_H
#define FACTORIES_TESTNODE2FACTORY_H
#pragma once

#include <Game/Factories/NodeFactory.h>

#include <Render/Model.h>

namespace OrbitMain
{
	/*!
	@brief Implementation of the factorydesign pattern, outputting instances of OrbitMain::TestNode2.
	*/
	class TestNode2Factory : public Orbit::NodeFactory
	{
	public:
		/*!
		@brief Constructs the factory with the model in parameter.
		@param input The input handler to be used in the nodes.
		@param testNode2Model The model to be used in instances of TestNode2s.
		*/
		TestNode2Factory(const Orbit::Input& input, std::shared_ptr<Orbit::Model> testNode2Model);

		/*!
		@brief Creates an instance of OrbitMain::TestNode2.
		@return An instance of OrbitMain::TestNode2.
		*/
		std::shared_ptr<Orbit::Node> create() const override;

		/*!
		@brief Creates an instance of OrbitMain::TestNode2 with the name in parameter.
		@param name the name to apply to the node.
		@return An instance of OrbitMain::TestNode2.
		*/
		std::shared_ptr<Orbit::Node> create(const std::string& name) const override;

	private:
		/*! The model used by instances of TestNode2. */
		const std::shared_ptr<Orbit::Model> _testNode2Model;
	};
}

#endif //FACTORIES_TESTNODE2FACTORY_H