/*! @file TestNode2Factory.h */

#ifndef FACTORIES_TESTNODE2FACTORY_H
#define FACTORIES_TESTNODE2FACTORY_H
#pragma once

#include <Game/Factories/NodeFactory.h>

#include <Render/Model.h>

namespace OrbitMain
{
	class TestNode2Factory : public Orbit::NodeFactory
	{
	public:
		TestNode2Factory(std::shared_ptr<Orbit::Model> testNode2Model);

		std::shared_ptr<Orbit::Node> create() const override;
		std::shared_ptr<Orbit::Node> create(const std::string& name) const override;

	private:
		const std::shared_ptr<Orbit::Model> _testNode2Model;
	};
}

#endif //FACTORIES_TESTNODE2FACTORY_H