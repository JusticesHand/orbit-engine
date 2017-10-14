/*! @file Scenes/LoadScene.cpp */

#include "Scenes/LoadScene.h"

#include "Nodes/TestNode.h"
#include "Nodes/TestNode2.h"

#include "Factories/TestNodeFactory.h"
#include "Factories/TestNode2Factory.h"

#include <Game/CompositeTree/CompositeTree.h>

#include <glm/gtc/matrix_transform.hpp>

using namespace Orbit;
using namespace OrbitMain;

void LoadScene::loadFactories()
{
	std::shared_ptr<Model> model1 = std::make_shared<Model>(std::vector<Vertex>{
		{{-0.5, -0.5, 0}, {0, 0}, {0, 0, 0}, {1, 0, 0, 1}},
		{{0.5, -0.5, 0}, {0, 0}, {0, 0, 0}, {0, 1, 0, 1}},
		{{0.5, 0.5, 0}, {0, 0}, {0, 0, 0}, {0, 0, 1, 1}},

		{{0.5, 0.5, 0}, {0, 0}, {0, 0, 0}, {0, 0, 1, 1}},
		{{-0.5, 0.5, 0}, {0, 0}, {0, 0, 0}, {1, 1, 1, 1}},
		{{-0.5, -0.5, 0}, {0, 0}, {0, 0, 0}, {1, 0, 0, 1}}
	});

	storeModel(model1);
	storeFactory<TestNode>(std::make_unique<TestNodeFactory>(model1));
	storeFactory<TestNode2>(std::make_unique<TestNode2Factory>(model1));
}

void LoadScene::load(Orbit::CompositeTree& tree)
{
	std::shared_ptr<CameraNode> camera = std::make_shared<CameraNode>();

	camera->setPosition({2, 2, 2});
	camera->setDirection({ -1, -1, -1 });
	camera->setUp({ 0, 0, 1 });

	tree.addChild(camera);

	tree.addChild(createNode<TestNode>("First"));

	std::shared_ptr<Node> secondChild = createNode<TestNode2>("Second");
	secondChild->setPosition({ 0, 0, -0.5 });
	secondChild->setRotation(glm::angleAxis(glm::radians(180.f), glm::vec3{ 0, 0, 1 }));
	tree.addChild(secondChild);
}

void LoadScene::unload()
{
}