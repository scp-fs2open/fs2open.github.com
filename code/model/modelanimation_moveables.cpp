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
			
			anim->start(pmi, ModelAnimationDirection::FWD);
		}
		catch(const linb::bad_any_cast& e){
			Error(LOCATION, "Argument error trying to update orientation moveable: %s", e.what());
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
			error_display(1, "Could not create moveable! Moveable Orientation has no target submodel!");

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
			anim->m_animation->calculateAnimation(buffer, anim->getTime(pmi->id), pmi->id);
			
			anim->forceRecalculate(pmi);
			
			setOrientation.m_targetOrientation = buffer[rotation.m_submodel].data.orientation;
			
			rotation.m_targetAngle = angles{fl_radians((float)p), fl_radians((float)b), fl_radians((float)h)};

			anim->start(pmi, ModelAnimationDirection::FWD);
		}
		catch(const linb::bad_any_cast& e){
			Error(LOCATION, "Argument error trying to update rotation moveable: %s", e.what());
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
			error_display(1, "Could not create moveable! Moveable Rotation has no target submodel!");

		return std::shared_ptr<ModelAnimationMoveableRotation>(new ModelAnimationMoveableRotation(submodel, angle, velocity, acceleration));
	}


	ModelAnimationMoveableAxisRotation::ModelAnimationMoveableAxisRotation(std::shared_ptr<ModelAnimationSubmodel> submodel, const float& velocity, const optional<float>& acceleration, const vec3d& axis) :
			m_submodel(std::move(submodel)), m_velocity(velocity), m_acceleration(acceleration), m_axis(axis) { }

	void ModelAnimationMoveableAxisRotation::update(polymodel_instance* pmi, const std::vector<linb::any>& args) {
		if(args.empty()){
			Error(LOCATION,"Tried updating moveable axis rotation with too few (%d of 1) arguments!", (int) args.size());
			return;
		}

		auto& anim = m_instances[pmi->id].animation;

		try {
			auto ang = linb::any_cast<int>(args[0]);
			
			auto& sequence = *std::static_pointer_cast<ModelAnimationSegmentSerial>(anim->m_animation);
			auto& setOrientation = *std::static_pointer_cast<ModelAnimationSegmentSetOrientation>(sequence.m_segments[0]);
			auto& rotation = *std::static_pointer_cast<ModelAnimationSegmentAxisRotation>(sequence.m_segments[1]);

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

			float currentAngle;
			vm_closest_angle_to_matrix(&newOrient, &m_axis, &currentAngle);
			//CurrentAngle is 0 to 2Pi, convert to -pi to pi
			if (currentAngle > PI)
				currentAngle -= PI2;

			rotation.m_targetAngle = fl_radians((float)ang) - currentAngle;

			anim->start(pmi, ModelAnimationDirection::FWD);
		}
		catch(const linb::bad_any_cast& e){
			Error(LOCATION, "Argument error trying to update rotation moveable: %s", e.what());
		}
	}

	void ModelAnimationMoveableAxisRotation::initialize(ModelAnimationSet* parentSet, polymodel_instance* pmi) {
		auto& anim = m_instances[pmi->id].animation;

		anim = std::shared_ptr<ModelAnimation>(new ModelAnimation(false, false, true, parentSet));

		auto submodel = parentSet->getSubmodel(m_submodel);

		auto sequence = std::shared_ptr<ModelAnimationSegmentSerial>(new ModelAnimationSegmentSerial());
		sequence->addSegment(std::shared_ptr<ModelAnimationSegment>(new ModelAnimationSegmentSetOrientation(submodel, vmd_identity_matrix, true)));
		sequence->addSegment(std::shared_ptr<ModelAnimationSegment>(new ModelAnimationSegmentAxisRotation(submodel, 0, m_velocity, optional<float>(), m_acceleration, m_axis)));

		anim->setAnimation(sequence);

		anim->start(pmi,ModelAnimationDirection::FWD);
	}

	std::shared_ptr<ModelAnimationMoveable> ModelAnimationMoveableAxisRotation::parser() {
		required_string("+Axis:");
		vec3d axis;
		stuff_vec3d(&axis);

		required_string("+Velocity:");
		float velocity;
		stuff_float(&velocity);
		velocity = fl_radians(velocity);

		optional<float> acceleration;
		if (optional_string("+Acceleration:")) {
			float parse;
			stuff_float(&parse);
			acceleration = fl_radians(parse);
		}

		auto submodel = ModelAnimationParseHelper::parseSubmodel();
		if(submodel == nullptr)
			error_display(1, "Could not create moveable! Moveable Axis Rotation has no target submodel!");

		return std::shared_ptr<ModelAnimationMoveableAxisRotation>(new ModelAnimationMoveableAxisRotation(submodel, velocity, acceleration, axis));
	}


	ModelAnimationMoveableIK::ModelAnimationMoveableIK(std::vector<moveable_chainlink> chain, float time) :
		m_time(time), m_chain(std::move(chain)) { }

	void ModelAnimationMoveableIK::update(polymodel_instance* pmi, const std::vector<linb::any>& args) {
		if(args.size() < 3){
			Error(LOCATION,"Tried updating moveable IK with too few (%d of 3) arguments!", (int) args.size());
			return;
		}

		auto& anim = m_instances[pmi->id].animation;

		try {
			auto x = linb::any_cast<int>(args[0]), y = linb::any_cast<int>(args[1]), z = linb::any_cast<int>(args[2]);

			optional<matrix> orient;
			if(args.size() >= 6 && args[3].type() == typeid(int)) {
				auto p = linb::any_cast<int>(args[3]), h = linb::any_cast<int>(args[4]), b = linb::any_cast<int>(args[5]);
				
				matrix rot;
				angles ang{ fl_radians(p), fl_radians(b), fl_radians(h) };
				vm_angles_2_matrix(&rot, &ang);
				orient = rot;
			}
			
			auto& sequence = *std::static_pointer_cast<ModelAnimationSegmentSerial>(anim->m_animation);
			auto& setOrientationParallel = *std::static_pointer_cast<ModelAnimationSegmentParallel>(sequence.m_segments[0]);
			auto& ik = *std::static_pointer_cast<ModelAnimationSegmentIK>(sequence.m_segments[1]);

			ModelAnimationSubmodelBuffer buffer;
			for(const auto& segment : setOrientationParallel.m_segments){
				auto& orientation = *std::static_pointer_cast<ModelAnimationSegmentSetOrientation>(segment);
				buffer[orientation.m_submodel].data.orientation = IDENTITY_MATRIX;
			}
			
			//Will now only contain the delta of the IK
			anim->m_animation->calculateAnimation(buffer, anim->getTime(pmi->id), pmi->id);

			anim->forceRecalculate(pmi);

			for(const auto& segment : setOrientationParallel.m_segments){
				auto& orientation = *std::static_pointer_cast<ModelAnimationSegmentSetOrientation>(segment);
				orientation.m_targetOrientation = buffer[orientation.m_submodel].data.orientation;
			}

			ik.m_targetRotation = orient;
			ik.m_targetPosition = vec3d{{{x * 0.01f, y * 0.01f, z * 0.01f}}};

			anim->start(pmi, ModelAnimationDirection::FWD);
		}
		catch(const linb::bad_any_cast& e){
			Error(LOCATION, "Argument error trying to update rotation moveable: %s", e.what());
		}
	}
	
	void ModelAnimationMoveableIK::initialize(ModelAnimationSet* parentSet, polymodel_instance* pmi) {
		auto& anim = m_instances[pmi->id].animation;

		anim = std::shared_ptr<ModelAnimation>(new ModelAnimation(false, false, true, parentSet));

		auto sequence = std::shared_ptr<ModelAnimationSegmentSerial>(new ModelAnimationSegmentSerial());
		auto parallelSetOrient = std::shared_ptr<ModelAnimationSegmentParallel>(new ModelAnimationSegmentParallel());
		for(const auto& link : m_chain) {
			auto submodel = parentSet->getSubmodel(link.submodel);
			parallelSetOrient->addSegment(std::shared_ptr<ModelAnimationSegment>(new ModelAnimationSegmentSetOrientation(submodel, vmd_identity_matrix, true)));
		}
		sequence->addSegment(parallelSetOrient);
		
		auto parallelIK = std::shared_ptr<ModelAnimationSegmentParallel>(new ModelAnimationSegmentParallel());
		vec3d startPos = ZERO_VECTOR;
		
		for(size_t i = 1; i < m_chain.size(); ++i) {
			startPos += parentSet->getSubmodel(m_chain[i].submodel)->findSubmodel(pmi).second->offset;
		}
		
		auto segment = std::shared_ptr<ModelAnimationSegmentIK>(new ModelAnimationSegmentIK(startPos, optional<matrix>()));
		segment->m_segment = parallelIK;
		
		for(const auto& link : m_chain) {
			auto submodel = parentSet->getSubmodel(link.submodel);
			auto rotation = std::shared_ptr<ModelAnimationSegmentRotation>(new ModelAnimationSegmentRotation(submodel, optional<angles>({0,0,0}), optional<angles>(), m_time, link.acceleration, true));
			parallelIK->addSegment(rotation);
			segment->m_chain.push_back({submodel, link.constraint, rotation});
		}
		
		sequence->addSegment(segment);
		
		anim->setAnimation(sequence);

		anim->start(pmi,ModelAnimationDirection::FWD);
	}
	
	std::shared_ptr<ModelAnimationMoveable> ModelAnimationMoveableIK::parser() {
		required_string("+Time:");
		float time;
		stuff_float(&time);

		std::vector<moveable_chainlink> chain;

		while (optional_string("$Chain Link:")) {
			auto submodel = ModelAnimationParseHelper::parseSubmodel();

			optional<angles> acceleration;
			if (optional_string("+Acceleration:")) {
				angles accel;
				stuff_angles_deg_phb(&accel);
				acceleration = accel;
			}

			std::shared_ptr<ik_constraint> constraint;
			if (optional_string("+Constraint:")) {
				int type = required_string_one_of(2, "Window", "Hinge");
				switch (type) {
					case 0: { //Window
						angles window;
						required_string("Window");
						required_string("+Window Size:");
						stuff_angles_deg_phb(&window);
						constraint = std::shared_ptr<ik_constraint>(new ik_constraint_window(window));
						break;
					}
					case 1: { //Hinge
						vec3d axis;
						required_string("Hinge");
						required_string("+Axis:");
						stuff_vec3d(&axis);
						vm_vec_normalize(&axis);
						constraint = std::shared_ptr<ik_constraint>(new ik_constraint_hinge(axis));
						break;
					}
					default:
						UNREACHABLE("IK constraint of unknown type specified!");
						break;
				}
			} else
				constraint = std::shared_ptr<ik_constraint>(new ik_constraint());

			chain.push_back({submodel, constraint, acceleration});
		}
		
		return std::shared_ptr<ModelAnimationMoveable>(new ModelAnimationMoveableIK(chain, time));
	}
}
