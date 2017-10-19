/*! @file Visitors/ModelVisitor.h */

#ifndef VISITORS_MODELVISITOR_H
#define VISITORS_MODELVISITOR_H
#pragma once

#include <Game/CompositeTree/Visitor.h>

#include "Render/Renderer.h"

#include <memory>

namespace Orbit
{
	class Model;

	class ModelVisitor final : public Visitor
	{
	public:
		ModelVisitor() = default;
		virtual ~ModelVisitor() = default;

		void visitElement(Node* node) override;

		bool modelCountsChanged() const;

		void flushModelCounts();

		std::vector<Renderer::ModelCountPair> modelCounts() const;
		const std::vector<Renderer::ModelTransformsPair>& treeState() const;

	private:
		void commitModel(std::shared_ptr<Model> model, const glm::mat4& transform);

		std::vector<Renderer::ModelCountPair> _oldModelCounts;
		std::vector<Renderer::ModelTransformsPair> _retrievedTreeState;
	};
}

#endif //VISITORS_MODELVISITOR_H
