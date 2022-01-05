#pragma once

#include "model/modelanimation.h"

namespace animation {

	class ModelAnimationMoveableOrientation : public ModelAnimationMoveable {
		static ModelAnimationParseHelper::Moveable reg;

		std::shared_ptr<ModelAnimationSubmodel> m_submodel;
		angles m_defaultPosOrient;
	public:
		ModelAnimationMoveableOrientation(std::shared_ptr<ModelAnimationSubmodel> submodel, const angles& defaultOrient);

		void update(polymodel_instance* pmi, const std::vector<linb::any>& args) override;
		void initialize(ModelAnimationSet* parentSet, polymodel_instance* pmi) override;
	};

}