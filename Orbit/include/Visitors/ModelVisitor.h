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

	/*!
	@brief Simple visitor implementation to retrieve models from the composite tree.
	*/
	class ModelVisitor final : public Visitor
	{
	public:
		/*!
		@brief Default constructor for the class.
		*/
		ModelVisitor() = default;

		/*!
		@brief Destructor for the class.
		*/
		virtual ~ModelVisitor() = default;

		/*!
		@brief Simple element visiting, to retrieve a node's model. Since all nodes can have a model,
		there is no overloading of the visitElement method.
		@param node The node to visit.
		*/
		void visitElement(Node* node) override;

		/*!
		@brief Returns whether or not the models (and counts of models) have changed since the last update.
		If true, it indicates that the rendering tree's state is different that on the last iteration and
		that an update to graphics memory is required (depending on the renderer used).
		@return If the composite tree's state has changed.
		*/
		bool modelCountsChanged() const;

		/*!
		@brief Flushes the model counts from the last iteration to the "saved" state. The in-progress
		model states are saved and will be tested against by modelCountsChanged.
		*/
		void flushModelCounts();

		/*!
		@brief Computes the collection of ModelCountPairs from the inner transforms map for reloading purposes.
		@return A collection of ModelCountPairs.
		*/
		std::vector<Renderer::ModelCountPair> modelCounts() const;

		/*!
		@brief Returns a reference to the interior collection of models and transforms, effectively translating
		the state of the composite tree.
		@return A reference to the interior collection of models and transforms.
		*/
		const std::vector<Renderer::ModelTransformsPair>& treeState() const;

	private:
		/*!
		@brief Commits a model and a transform to the tree state. Handles insertions in the collection and
		updating of already existing values.
		@param model The model to insert/update.
		@param transform The new transform to add.
		*/
		void commitModel(std::shared_ptr<Model> model, const glm::mat4& transform);

		/*! The model counts. Updated when Orbit::ModelVisitor::flushModelCounts() is called. */
		std::vector<Renderer::ModelCountPair> _oldModelCounts;
		/*! The current retrieved state of the composite tree. */
		std::vector<Renderer::ModelTransformsPair> _retrievedTreeState;
	};
}

#endif //VISITORS_MODELVISITOR_H
