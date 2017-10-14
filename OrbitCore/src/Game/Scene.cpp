/*! @file Game/Scene.cpp */

#include "Game/Scene.h"

using namespace Orbit;

const std::vector<std::shared_ptr<Model>>& Scene::getModels() const
{
	return _models;
}

void Scene::storeModel(std::shared_ptr<Model> model)
{
	_models.push_back(model);
}