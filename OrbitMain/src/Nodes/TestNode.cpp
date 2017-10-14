/*! @file Nodes/TestNode.cpp */

#include "Nodes/TestNode.h"

#include <Game/CompositeTree/Visitor.h>
#include <Input/Input.h>

#include <iostream>

using namespace OrbitMain;
using namespace Orbit;

TestNode::TestNode(const std::shared_ptr<Model>& model)
	: TestNode("TestNode", model)
{ 
}

TestNode::TestNode(const std::string& name, const std::shared_ptr<Model>& model)
	: Node(name, model)
{
}

TestNode::TestNode(TestNode&& rhs)
	: Node(std::move(rhs))
{
}

TestNode& TestNode::operator=(TestNode&& rhs)
{
	Node::operator=(std::move(rhs));
	return *this;
}

void TestNode::acceptVisitor(Orbit::Visitor* visitor)
{
	visitor->visitElement(this);
}

std::shared_ptr<Orbit::Node> TestNode::clone() const
{
	return std::make_shared<TestNode>(getModel());
}

void TestNode::update(std::chrono::nanoseconds elapsedTime)
{
	using namespace std::chrono;

	_accumulatedTime += elapsedTime;
	if (_accumulatedTime >= seconds(1))
	{
		_accumulatedTime = nanoseconds::zero();
		std::cout << "A second has passed." << std::endl;
	}

	if (Input::getInput().keyPressed(Orbit::Key::Code::A))
	{
		std::cout << "Hi I pressed the A button" << std::endl;
	}

	if (Input::getInput().keyPressed("Fire"))
	{
		std::cout << "Pew pew - virtual fire button enabled" << std::endl;
	}

	glm::ivec2 mouseDelta = Input::getInput().mouseDelta();
	if (mouseDelta.x != 0 && mouseDelta.y != 0)
	{
		std::cout << "MOVED THE MOUSE: " << mouseDelta.x << "," << mouseDelta.y << std::endl;
	}
}