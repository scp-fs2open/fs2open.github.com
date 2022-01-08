#include "model/modelanimation_moveables.h"
#include "model/modelanimation_segments.h"

namespace animation {

	ModelAnimationMoveableOrientation::ModelAnimationMoveableOrientation(std::shared_ptr<ModelAnimationSubmodel> submodel, const angles& defaultOrient) :
		m_submodel(std::move(submodel)), m_defaultPosOrient(defaultOrient) { }

	void ModelAnimationMoveableOrientation::update(polymodel_instance* pmi, const std::vector<linb::any>& args) {
		if(args.size() < 3){
			Error(LOCATION,"Tried updating moveable orientation with too few (%d of 3) arguments!", (int) args.size());
			return;
		}

		auto& anim = m_instances[pmi->id].animation;

		try {
			auto p = linb::any_cast<int>(args[0]), h = linb::any_cast<int>(args[1]), b = linb::any_cast<int>(args[2]);

			anim->forceRecalculate(pmi);

			auto& setOrientation = *std::static_pointer_cast<ModelAnimationSegmentSetOrientation>(anim->m_animation);
			setOrientation.m_targetAngle = angles{fl_radians((float)p), fl_radians((float)b), fl_radians((float)h)};
		}
		catch(const linb::bad_any_cast& e){
			Error(LOCATION, "Failed to parse update for orientation moveable: %s", e.what());
		}
	}

	void ModelAnimationMoveableOrientation::initialize(ModelAnimationSet* parentSet, polymodel_instance* pmi) {
		auto& anim = m_instances[pmi->id].animation;

		anim = std::shared_ptr<ModelAnimation>(new ModelAnimation(false, false, true, parentSet));

		anim->setAnimation(std::shared_ptr<ModelAnimationSegment>(new ModelAnimationSegmentSetOrientation(parentSet->getSubmodel(m_submodel), m_defaultPosOrient, true)));

		anim->start(pmi,ModelAnimationDirection::FWD);
	}
	
	std::shared_ptr<ModelAnimationMoveable> ModelAnimationMoveableOrientation::parser() {
		required_string("+Angle:");
		angles angle;
		stuff_angles_deg_phb(&angle);

		auto submodel = ModelAnimationParseHelper::parseSubmodel();
		if(submodel == nullptr)
			error_display(1, "Orientation Moveable has no target submodel!");

		return std::shared_ptr<ModelAnimationMoveableOrientation>(new ModelAnimationMoveableOrientation(submodel, angle));
	}


	ModelAnimationMoveableRotation::ModelAnimationMoveableRotation(std::shared_ptr<ModelAnimationSubmodel> submodel, const angles& defaultOrient, const angles& velocity, const optional<angles>& acceleration) :
			m_submodel(std::move(submodel)), m_defaultPosOrient(defaultOrient), m_velocity(velocity), m_acceleration(acceleration) { }

	void ModelAnimationMoveableRotation::update(polymodel_instance* pmi, const std::vector<linb::any>& args) {
		if(args.size() < 3){
			Error(LOCATION,"Tried updating moveable rotation with too few (%d of 3) arguments!", (int) args.size());
			return;
		}

		auto& anim = m_instances[pmi->id].animation;

		try {
			auto p = linb::any_cast<int>(args[0]), h = linb::any_cast<int>(args[1]), b = linb::any_cast<int>(args[2]);


			auto& sequence = *std::static_pointer_cast<ModelAnimationSegmentSerial>(anim->m_animation);
			auto& setOrientation = *std::static_pointer_cast<ModelAnimationSegmentSetOrientation>(sequence.m_segments[0]);
			auto& rotation = *std::static_pointer_cast<ModelAnimationSegmentRotation>(sequence.m_segments[1]);
			
			ModelAnimationSubmodelBuffer buffer;
			buffer[rotation.m_submodel].data.orientation = IDENTITY_MATRIX;
			//Will now only contain the delta of the rotation
			sequence.m_segments[1]->calculateAnimation(buffer, anim->getTime(pmi->id), pmi->id);
			
			anim->forceRecalculate(pmi);
			
			matrix startOrient = setOrientation.m_targetOrientation;
			matrix newOrient;
			//Apply rotation to old default state
			vm_matrix_x_matrix(&newOrient, &startOrient, &buffer[rotation.m_submodel].data.orientation);
			setOrientation.m_targetOrientation = newOrient;
			
			rotation.m_targetAngle = angles{fl_radians((float)p), fl_radians((float)b), fl_radians((float)h)};
		}
		catch(const linb::bad_any_cast& e){
			Error(LOCATION, "Failed to parse update for rotation moveable: %s", e.what());
		}
	}

	void ModelAnimationMoveableRotation::initialize(ModelAnimationSet* parentSet, polymodel_instance* pmi) {
		auto& anim = m_instances[pmi->id].animation;

		anim = std::shared_ptr<ModelAnimation>(new ModelAnimation(false, false, true, parentSet));

		matrix orient;
		vm_angles_2_matrix(&orient, &m_defaultPosOrient);
		
		auto submodel = parentSet->getSubmodel(m_submodel);
		
		auto sequence = std::shared_ptr<ModelAnimationSegmentSerial>(new ModelAnimationSegmentSerial());
		sequence->addSegment(std::shared_ptr<ModelAnimationSegment>(new ModelAnimationSegmentSetOrientation(submodel, orient, true)));
		sequence->addSegment(std::shared_ptr<ModelAnimationSegment>(new ModelAnimationSegmentRotation(submodel, m_defaultPosOrient, m_velocity, optional<float>(), m_acceleration, true)));
		
		anim->setAnimation(sequence);

		anim->start(pmi,ModelAnimationDirection::FWD);
	}
	
	std::shared_ptr<ModelAnimationMoveable> ModelAnimationMoveableRotation::parser() {
		required_string("+Angle:");
		angles angle;
		stuff_angles_deg_phb(&angle);

		required_string("+Velocity:");
		angles velocity;
		stuff_angles_deg_phb(&velocity);

		optional<angles> acceleration;
		if (optional_string("+Acceleration:")) {
			angles parse;
			stuff_angles_deg_phb(&parse);
			acceleration = parse;
		}

		auto submodel = ModelAnimationParseHelper::parseSubmodel();
		if(submodel == nullptr)
			error_display(1, "Orientation Rotation has no target submodel!");

		return std::shared_ptr<ModelAnimationMoveableRotation>(new ModelAnimationMoveableRotation(submodel, angle, velocity, acceleration));
	}

}