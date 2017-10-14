/*! @file TestNodeFactory.h */

#ifndef FACTORIES_TESTNODEFACTORY_H
#define FACTORIES_TESTNODEFACTORY_H
#pragma once

#include <Game/Factories/NodeFactory.h>

#include <Render/Model.h>

namespace OrbitMain
{
	class TestNodeFactory : public Orbit::NodeFactory
	{
	public:
		TestNodeFactory(std::shared_ptr<Orbit::Model> testNodeModel);

		std::shared_ptr<Orbit::Node> create() const override;

		std::shared_ptr<Orbit::Node> create(const std::string& name) const override;

	private:
		const std::shared_ptr<Orbit::Model> _testNodeModel;
	};
}

#endif //FACTORIES_TESTNODEFACTORY_H