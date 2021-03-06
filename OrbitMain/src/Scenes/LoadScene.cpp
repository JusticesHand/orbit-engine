/*! @file Scenes/LoadScene.cpp */

#include "Scenes/LoadScene.h"

#include "Nodes/TestNode.h"
#include "Nodes/TestNode2.h"

#include "Factories/TestNodeFactory.h"
#include "Factories/TestNode2Factory.h"

#include <Render/Texture.h>

#include <Game/CompositeTree/CompositeTree.h>

#include <glm/gtc/matrix_transform.hpp>

using namespace OrbitMain;

void LoadScene::loadFactories(const Orbit::Input& input)
{
	std::shared_ptr<Orbit::Texture> texture = std::make_shared<Orbit::Texture>("Resources/Hello.png");

	std::shared_ptr<Orbit::Model> model1 = std::make_shared<Orbit::Model>(std::vector<Orbit::Vertex>{
		{{-0.5, -0.5, 0}, {0, 1}, {0, 0, 0}, {1, 0, 0, 1}},
		{{0.5, -0.5, 0}, {1, 1}, {0, 0, 0}, {0, 1, 0, 1}},
		{{0.5, 0.5, 0}, {1, 0}, {0, 0, 0}, {0, 0, 1, 1}},

		{{0.5, 0.5, 0}, {1, 0}, {0, 0, 0}, {0, 0, 1, 1}},
		{{-0.5, 0.5, 0}, {0, 0}, {0, 0, 0}, {1, 1, 1, 1}},
		{{-0.5, -0.5, 0}, {0, 1}, {0, 0, 0}, {1, 0, 0, 1}}
		}, texture);

	std::shared_ptr<Orbit::Texture> texture2 = std::make_shared<Orbit::Texture>("Resources/Hi.png");

	std::shared_ptr<Orbit::Model> model2 = std::make_shared<Orbit::Model>(std::vector<Orbit::Vertex>{
		{ {-0.5, -0.5, 0}, { 0, 1 }, { 0, 0, 0 }, { 1, 0, 0, 1 }},
		{ { 0.5, -0.5, 0 },{ 1, 1 },{ 0, 0, 0 },{ 0, 1, 0, 1 } },
		{ { 0.5, 0.5, 0.5 },{ 1, 0 },{ 0, 0, 0 },{ 0, 0, 1, 1 } },

		{ { 0.5, 0.5, 0.5 },{ 1, 0 },{ 0, 0, 0 },{ 0, 0, 1, 1 } },
		{ { -0.5, 0.5, 0 },{ 0, 0 },{ 0, 0, 0 },{ 1, 1, 1, 1 } },
		{ { -0.5, -0.5, 0 },{ 0, 1 },{ 0, 0, 0 },{ 1, 0, 0, 1 } }
	}, texture2);

	storeFactory<TestNode>(std::make_unique<TestNodeFactory>(input, model1));
	storeFactory<TestNode2>(std::make_unique<TestNode2Factory>(input, model2));
}

void LoadScene::load(Orbit::CompositeTree& tree)
{
	std::shared_ptr<Orbit::CameraNode> camera = std::make_shared<Orbit::CameraNode>();

	camera->setPosition({ 2, 2, 2 });
	camera->setDirection({ -1, -1, -1 });
	camera->setUp({ 0, 0, 1 });

	tree.addChild(camera);

	tree.addChild(createNode<TestNode>("First"));

	std::shared_ptr<Orbit::Node> secondChild = createNode<TestNode2>("Second");
	secondChild->setPosition({ 0, 0, -0.5 });
	secondChild->setRotation(glm::angleAxis(glm::radians(180.f), glm::vec3{ 0, 0, 1 }));
	tree.addChild(secondChild);
}

void LoadScene::unload()
{
}