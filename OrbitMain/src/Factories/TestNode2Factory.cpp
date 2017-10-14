#include "Factories/TestNode2Factory.h"

#include "Nodes/TestNode2.h"

using namespace Orbit;
using namespace OrbitMain;

TestNode2Factory::TestNode2Factory(std::shared_ptr<Model> testNode2Model)
	: _testNode2Model(testNode2Model)
{
}

std::shared_ptr<Node> TestNode2Factory::create() const
{
	return std::make_shared<TestNode2>(_testNode2Model);
}

std::shared_ptr<Orbit::Node> TestNode2Factory::create(const std::string& name) const
{
	return std::make_shared<TestNode2>(name, _testNode2Model);
}