/*! @file Nodes/TestNode2.cpp */

#include "Nodes/TestNode2.h"

#include <Game/CompositeTree/Visitor.h>
#include <Input/Input.h>

using namespace OrbitMain;

TestNode2::TestNode2(const std::shared_ptr<Orbit::Model>& model)
	: TestNode2("TestNode2", model)
{
}

TestNode2::TestNode2(const std::string& name, const std::shared_ptr<Orbit::Model>& model)
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

void TestNode2::acceptVisitor(Orbit::Visitor* visitor)
{
	visitor->visitElement(this);
}

std::shared_ptr<Orbit::Node> TestNode2::clone() const
{
	return std::make_shared<TestNode2>(getModel());
}

void TestNode2::update(std::chrono::nanoseconds elapsedTime)
{
	if (Orbit::Input::getInput().keyPressed(Orbit::Key::Code::Up))
		_position.z += 0.01f;

	if (Orbit::Input::getInput().keyPressed(Orbit::Key::Code::Down))
		_position.z -= 0.01f;

	if (Orbit::Input::getInput().keyPressed(Orbit::Key::Code::Left))
		_position.y -= 0.01f;

	if (Orbit::Input::getInput().keyPressed(Orbit::Key::Code::Right))
		_position.y += 0.01f;
}