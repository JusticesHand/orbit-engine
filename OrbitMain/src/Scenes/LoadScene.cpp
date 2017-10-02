/*! @file Scenes/LoadScene.cpp */

#include "Scenes/LoadScene.h"

#include "Nodes/TestNode.h"

#include <Game/CompositeTree/CompositeTree.h>

using namespace OrbitMain;

void LoadScene::registerFactories()
{
	// TODO
}

void LoadScene::load(Orbit::CompositeTree& tree)
{
	tree.addChild(std::make_shared<TestNode>());
}

void LoadScene::unload()
{
	// TODO
}