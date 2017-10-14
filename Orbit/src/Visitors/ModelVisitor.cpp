/*! @file Visitors/ModelVisitor.cpp */

#include "Visitors/ModelVisitor.h"

#include <Game/CompositeTree/Node.h>

using namespace Orbit;

void ModelVisitor::visitElement(Node* node)
{
	if (!node->hasModel())
		return;

	commitModel(node->getModel(), node->modelMatrix());
}

bool ModelVisitor::modelCountsChanged() const
{
	std::vector<Renderer::ModelCountPair> counts = modelCounts();
	if (_oldModelCounts.size() != counts.size())
		return true;

	for (size_t i = 0; i < _oldModelCounts.size(); i++)
	{
		const Renderer::ModelCountPair& oldModelCount = _oldModelCounts[i];
		const Renderer::ModelCountPair& newModelCount = counts[i];

		if (oldModelCount.first != newModelCount.first)
			return true;

		if (oldModelCount.second != newModelCount.second)
			return true;
	}

	return false;
}

void ModelVisitor::flushModelCounts()
{
	_oldModelCounts = modelCounts();
	_retrievedTreeState.clear();
}

std::vector<Renderer::ModelCountPair> ModelVisitor::modelCounts() const
{
	std::vector<Renderer::ModelCountPair> modelCounts;
	modelCounts.reserve(_retrievedTreeState.size());

	for (const Renderer::ModelTransformsPair& pair : _retrievedTreeState)
		modelCounts.push_back(std::make_pair(pair.first, pair.second.size()));

	return modelCounts;
}

const std::vector<Renderer::ModelTransformsPair>& ModelVisitor::treeState() const
{
	return _retrievedTreeState;
}

void ModelVisitor::commitModel(std::shared_ptr<Model> model, const glm::mat4& transform)
{
	std::vector<Renderer::ModelTransformsPair>::iterator found = std::find_if(_retrievedTreeState.begin(), _retrievedTreeState.end(),
		[&model] (const Renderer::ModelTransformsPair& pair) {
		return model == pair.first;
	});

	if (found == _retrievedTreeState.end())
	{
		_retrievedTreeState.push_back(std::make_pair(model, std::vector<glm::mat4>{ transform }));
		return;
	}
	
	found->second.push_back(transform);
}