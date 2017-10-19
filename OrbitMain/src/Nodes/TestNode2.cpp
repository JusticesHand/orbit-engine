/*! @file Nodes/TestNode2.cpp */

#include "Nodes/TestNode2.h"

#include <Game/CompositeTree/Visitor.h>
#include <Input/Input.h>

using namespace Orbit;
using namespace OrbitMain;

TestNode2::TestNode2(const std::shared_ptr<Model>& model)
	: TestNode2("TestNode2", model)
{
}

TestNode2::TestNode2(const std::string& name, const std::shared_ptr<Model>& model)
	: Node(name, model)
{
}

TestNode2::TestNode2(TestNode2&& rhs)
	: Node(std::move(rhs))
{
}

TestNode2& TestNode2::operator=(TestNode2&& rhs)
{
	Node::operator=(std::move(rhs));
	return *this;
}

void TestNode2::acceptVisitor(Visitor* visitor)
{
	visitor->visitElement(this);
}

std::shared_ptr<Node> TestNode2::clone() const
{
	return std::make_shared<TestNode2>(getModel());
}

void TestNode2::update(std::chrono::nanoseconds elapsedTime)
{
	if (Input::getInput().keyPressed(Key::Code::Up))
		_position.z += 0.01f;

	if (Input::getInput().keyPressed(Key::Code::Down))
		_position.z -= 0.01f;

	if (Input::getInput().keyPressed(Key::Code::Left))
		_position.y -= 0.01f;

	if (Input::getInput().keyPressed(Key::Code::Right))
		_position.y += 0.01f;
}