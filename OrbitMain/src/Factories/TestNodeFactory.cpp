#include "Factories/TestNodeFactory.h"

#include "Nodes/TestNode.h"

using namespace OrbitMain;

TestNodeFactory::TestNodeFactory(const Orbit::Input& input, std::shared_ptr<Orbit::Model> testNodeModel)
	: NodeFactory(input), _testNodeModel(testNodeModel)
{
}

std::shared_ptr<Orbit::Node> TestNodeFactory::create() const
{
	return std::make_shared<TestNode>(_input, _testNodeModel);
}

std::shared_ptr<Orbit::Node> TestNodeFactory::create(const std::string& name) const
{
	return std::make_shared<TestNode>(_input, name, _testNodeModel);
}