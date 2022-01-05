#include "model/modelanimation_moveables.h"
#include "model/modelanimation_segments.h"

namespace animation {

	ModelAnimationMoveableOrientation::ModelAnimationMoveableOrientation(std::shared_ptr<ModelAnimationSubmodel> submodel, const angles& defaultOrient) :
		m_submodel(submodel), m_defaultPosOrient(defaultOrient) { }

	void ModelAnimationMoveableOrientation::update(polymodel_instance* pmi, const std::vector<linb::any>& args) {
		if(args.size() < 3){
			Error(LOCATION,"");
			return;
		}

		auto& anim = m_instances[pmi->id].animation;

		try {
			auto p = linb::any_cast<int>(args[0]), h = linb::any_cast<int>(args[1]), b = linb::any_cast<int>(args[2]);

			anim->forceRecalculate(pmi);

			auto &setOrientation = *std::static_pointer_cast<ModelAnimationSegmentSetOrientation>(anim->m_animation);
			setOrientation.m_targetAngle = angles{fl_radians((float)p), fl_radians((float)b), fl_radians((float)h)};
		}
		catch(const linb::bad_any_cast& e){
			Error(LOCATION, "Failed to parse update for orientation moveable: %s", e.what());
		}
	}

	void ModelAnimationMoveableOrientation::initialize(ModelAnimationSet* parentSet, polymodel_instance* pmi) {
		auto& anim = m_instances[pmi->id].animation;

		anim = std::shared_ptr<ModelAnimation>(new ModelAnimation(false, false, true, parentSet));

		anim->setAnimation(std::shared_ptr<ModelAnimationSegment>(new ModelAnimationSegmentSetOrientation(parentSet->getSubmodel(m_submodel), m_defaultPosOrient, false)));

		anim->start(pmi,ModelAnimationDirection::FWD);
	}

	std::shared_ptr<ModelAnimationMoveable> ModelAnimationMoveableOrientation_parser() {
		required_string("+Angle:");
		angles angle;
		stuff_angles_deg_phb(&angle);

		auto submodel = ModelAnimationParseHelper::parseSubmodel();
		if(submodel == nullptr)
			error_display(1, "Orientation Moveable has no target submodel!");

		return std::shared_ptr<ModelAnimationMoveableOrientation>(new ModelAnimationMoveableOrientation(submodel, angle));
	}
	ModelAnimationParseHelper::Moveable ModelAnimationMoveableOrientation::reg("Orientation", &ModelAnimationMoveableOrientation_parser);

}