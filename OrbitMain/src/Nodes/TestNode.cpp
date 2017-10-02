/*! @file Nodes/TestNode.cpp */

#include "Nodes/TestNode.h"

#include "Input/Input.h"

#include <iostream>

using namespace OrbitMain;

TestNode::TestNode()
	: Node("TestNode")
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

void TestNode::acceptVisitor(const Orbit::Visitor* visitor)
{
	// TODO
	//visitor->visitElement(this);
}

std::shared_ptr<Orbit::Node> TestNode::clone() const
{
	return std::make_shared<TestNode>();
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

	if (Orbit::Input::getInput().keyPressed(Orbit::Key::Code::A))
	{
		std::cout << "Hi I pressed the A button" << std::endl;
	}

	glm::ivec2 mouseDelta = Orbit::Input::getInput().mouseDelta();
	if (mouseDelta.x != 0 && mouseDelta.y != 0)
	{
		std::cout << "MOVED THE MOUSE: " << mouseDelta.x << "," << mouseDelta.y << std::endl;
	}
}