#include "Factories/TestNode2Factory.h"

#include "Nodes/TestNode2.h"

using namespace OrbitMain;

TestNode2Factory::TestNode2Factory(const Orbit::Input& input, std::shared_ptr<Orbit::Model> testNode2Model)
	: NodeFactory(input), _testNode2Model(testNode2Model)
{
}

std::shared_ptr<Orbit::Node> TestNode2Factory::create() const
{
	return std::make_shared<TestNode2>(_input, _testNode2Model);
}

std::shared_ptr<Orbit::Node> TestNode2Factory::create(const std::string& name) const
{
	return std::make_shared<TestNode2>(_input, name, _testNode2Model);
}