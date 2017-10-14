#include "Factories/TestNodeFactory.h"

#include "Nodes/TestNode.h"

using namespace Orbit;
using namespace OrbitMain;

TestNodeFactory::TestNodeFactory(std::shared_ptr<Model> testNodeModel)
	: _testNodeModel(testNodeModel)
{
}

std::shared_ptr<Node> TestNodeFactory::create() const
{
	return std::make_shared<TestNode>(_testNodeModel);
}

std::shared_ptr<Orbit::Node> TestNodeFactory::create(const std::string& name) const
{
	return std::make_shared<TestNode>(name, _testNodeModel);
}