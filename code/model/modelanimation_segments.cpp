#include "model/modelanimation_segments.h"

#include "render/3d.h"

namespace animation {

	ModelAnimationSegment* ModelAnimationSegmentSerial::copy() const {
		auto newCopy = new ModelAnimationSegmentSerial();
		for (const auto& segment : m_segments) {
			newCopy->m_segments.push_back(std::shared_ptr<ModelAnimationSegment>(segment->copy()));
		}
		return newCopy;
	}

	void ModelAnimationSegmentSerial::recalculate(ModelAnimationSubmodelBuffer& base, ModelAnimationSubmodelBuffer& currentAnimDelta, polymodel_instance* pmi) {
		float& duration = m_duration[pmi->id];
		duration = 0.0f;

		for (const auto& segment : m_segments) {
			segment->recalculate(base, currentAnimDelta, pmi);

			duration += segment->getDuration(pmi->id);

			//To properly recalculate, we actually need to fully calculate the previous' segment's final delta
			segment->calculateAnimation(base, segment->getDuration(pmi->id), pmi->id);
			segment->calculateAnimation(currentAnimDelta, segment->getDuration(pmi->id), pmi->id);
		}
	}

	void ModelAnimationSegmentSerial::calculateAnimation(ModelAnimationSubmodelBuffer& base, float time, int pmi_id) const {
		size_t animationCnt = 0;
		while (time >= 0.0f && animationCnt < m_segments.size()) {
			float timeLocal = time;
			//Make sure that each segment actually stops at its end
			if (timeLocal > m_segments[animationCnt]->getDuration(pmi_id))
				timeLocal = m_segments[animationCnt]->getDuration(pmi_id);
			m_segments[animationCnt]->calculateAnimation(base, timeLocal, pmi_id);

			time -= m_segments[animationCnt]->getDuration(pmi_id);
			animationCnt++;
		}
	}

	void ModelAnimationSegmentSerial::executeAnimation(const ModelAnimationSubmodelBuffer& state, float timeboundLower, float timeboundUpper, ModelAnimationDirection direction, int pmi_id) {
		for (const auto& segment : m_segments) {
			if (timeboundLower < segment->getDuration(pmi_id)) {
				segment->executeAnimation(state, fmaxf(0.0f, timeboundLower), fminf(timeboundUpper, segment->getDuration(pmi_id)), direction, pmi_id);
			}

			timeboundLower -= segment->getDuration(pmi_id);
			timeboundUpper -= segment->getDuration(pmi_id);

			if (timeboundUpper < 0)
				return;
		}
	}

	void ModelAnimationSegmentSerial::exchangeSubmodelPointers(ModelAnimationSet& replaceWith) {
		for (const auto& segment : m_segments)
			segment->exchangeSubmodelPointers(replaceWith);
	}

	void ModelAnimationSegmentSerial::addSegment(std::shared_ptr<ModelAnimationSegment> segment) {
		m_segments.push_back(std::move(segment));
	}
	
	std::shared_ptr<ModelAnimationSegment> ModelAnimationSegmentSerial::parser(ModelAnimationParseHelper* data) {
		auto submodelOverride = ModelAnimationParseHelper::parseSubmodel();

		ignore_white_space();
		auto segment = std::shared_ptr<ModelAnimationSegmentSerial>(new ModelAnimationSegmentSerial());

		while (!optional_string("+End Segment")) {
			if (submodelOverride)
				data->parentSubmodel = submodelOverride;

			segment->addSegment(data->parseSegment());
			ignore_white_space();
		}

		return segment;
	}


	ModelAnimationSegment* ModelAnimationSegmentParallel::copy() const {
		auto newCopy = new ModelAnimationSegmentParallel();
		for (const auto& segment : m_segments) {
			newCopy->m_segments.push_back(std::shared_ptr<ModelAnimationSegment>(segment->copy()));
		}
		return newCopy;
	}

	void ModelAnimationSegmentParallel::recalculate(ModelAnimationSubmodelBuffer& base, ModelAnimationSubmodelBuffer& currentAnimDelta, polymodel_instance* pmi) {
		float& duration = m_duration[pmi->id];
		duration = 0.0f;

		for (const auto& segment : m_segments) {
			ModelAnimationSubmodelBuffer baseCopy = base;
			segment->recalculate(baseCopy, currentAnimDelta, pmi);

			//recalculate total duration if necessary
			float newDur = segment->getDuration(pmi->id);
			duration = newDur > duration ? newDur : duration;
		}

	}

	void ModelAnimationSegmentParallel::calculateAnimation(ModelAnimationSubmodelBuffer& base, float time, int pmi_id) const {
		for (const auto& segment : m_segments) {
			float timeLocal = time;
			//Make sure that no segment runs over its length
			if (timeLocal > segment->getDuration(pmi_id))
				timeLocal = segment->getDuration(pmi_id);
			
			segment->calculateAnimation(base, timeLocal, pmi_id);
		}
	}

	void ModelAnimationSegmentParallel::executeAnimation(const ModelAnimationSubmodelBuffer& state, float timeboundLower, float timeboundUpper, ModelAnimationDirection direction, int pmi_id) {
		for (const auto& segment : m_segments) {
			if (timeboundLower < segment->getDuration(pmi_id)) {
				segment->executeAnimation(state, timeboundLower, fminf(timeboundUpper, segment->getDuration(pmi_id)), direction, pmi_id);
			}
		}
	}

	void ModelAnimationSegmentParallel::exchangeSubmodelPointers(ModelAnimationSet& replaceWith) {
		for (const auto& segment : m_segments)
			segment->exchangeSubmodelPointers(replaceWith);
	}

	void ModelAnimationSegmentParallel::addSegment(std::shared_ptr<ModelAnimationSegment> segment) {
		m_segments.push_back(std::move(segment));
	}
	
	std::shared_ptr<ModelAnimationSegment> ModelAnimationSegmentParallel::parser(ModelAnimationParseHelper* data) {
		auto submodelOverride = ModelAnimationParseHelper::parseSubmodel();

		ignore_white_space();
		auto segment = std::shared_ptr<ModelAnimationSegmentParallel>(new ModelAnimationSegmentParallel());

		while (!optional_string("+End Segment")) {
			if (submodelOverride)
				data->parentSubmodel = submodelOverride;

			segment->addSegment(data->parseSegment());
			ignore_white_space();
		}

		return segment;
	}


	ModelAnimationSegmentWait::ModelAnimationSegmentWait(float time) : m_time(time) { }

	ModelAnimationSegment* ModelAnimationSegmentWait::copy() const {
		return new ModelAnimationSegmentWait(*this);
	}
	
	void ModelAnimationSegmentWait::recalculate(ModelAnimationSubmodelBuffer& /*base*/, ModelAnimationSubmodelBuffer& /*currentAnimDelta*/, polymodel_instance* pmi) {
		m_duration[pmi->id] = m_time;
	};
	
	std::shared_ptr<ModelAnimationSegment> ModelAnimationSegmentWait::parser(ModelAnimationParseHelper* /*data*/) {
		required_string("+Time:");
		float time = 0.0f;
		stuff_float(&time);
		auto segment = std::shared_ptr<ModelAnimationSegmentWait>(new ModelAnimationSegmentWait(time));

		return segment;
	}

	ModelAnimationSegmentSetOrientation::ModelAnimationSegmentSetOrientation(std::shared_ptr<ModelAnimationSubmodel> submodel, const angles& angle, ModelAnimationCoordinateRelation isAngleRelative) :
		m_submodel(std::move(submodel)), m_targetAngle(angle), m_relationType(isAngleRelative) { }
	ModelAnimationSegmentSetOrientation::ModelAnimationSegmentSetOrientation(std::shared_ptr<ModelAnimationSubmodel> submodel, const matrix& orientation, ModelAnimationCoordinateRelation isAngleRelative) :
			m_submodel(std::move(submodel)), m_targetOrientation(orientation), m_relationType(isAngleRelative) { }
		
	ModelAnimationSegment* ModelAnimationSegmentSetOrientation::copy() const {
		return new ModelAnimationSegmentSetOrientation(*this);
	}

	void ModelAnimationSegmentSetOrientation::recalculate(ModelAnimationSubmodelBuffer& base, ModelAnimationSubmodelBuffer& currentAnimDelta, polymodel_instance* pmi) {
		int pmi_id = pmi->id;
		if (m_relationType == ModelAnimationCoordinateRelation::RELATIVE_COORDS) {
			if (m_targetAngle) {
				vm_angles_2_matrix(&m_instances[pmi_id].rot, &(*m_targetAngle));
			}
			else if (m_targetOrientation) {
				m_instances[pmi_id].rot = *m_targetOrientation;
			}
		}
		else {
			//In Absolute mode we need to undo the previously applied rotation to make sure we actually end up at the target rotation despite having only a delta we output, as opposed to just overwriting the value
			matrix unrotate, target;
			const ModelAnimationData<>& submodel = (m_relationType == ModelAnimationCoordinateRelation::ABSOLUTE_COORDS ? base[m_submodel].data : currentAnimDelta[m_submodel].data);

			vm_copy_transpose(&unrotate, &submodel.orientation);

			if (m_targetAngle) {
				vm_angles_2_matrix(&target, &(*m_targetAngle));
			}
			else if (m_targetOrientation) {
				target = *m_targetOrientation;
			}

			vm_matrix_x_matrix(&m_instances[pmi_id].rot, &target, &unrotate);
		}

		m_duration[pmi_id] = 0.0f;
	}

	void ModelAnimationSegmentSetOrientation::calculateAnimation(ModelAnimationSubmodelBuffer& base, float /*time*/, int pmi_id) const {
		ModelAnimationData<true> data;
		data.orientation = m_instances.at(pmi_id).rot;

		base[m_submodel].data.applyDelta(data);
		base[m_submodel].modified = true;
	}

	void ModelAnimationSegmentSetOrientation::exchangeSubmodelPointers(ModelAnimationSet& replaceWith) {
		m_submodel = replaceWith.getSubmodel(m_submodel);
	}
	
	std::shared_ptr<ModelAnimationSegment> ModelAnimationSegmentSetOrientation::parser(ModelAnimationParseHelper* data) {
		angles angle;

		required_string("+Angle:");
		stuff_angles_deg_phb(&angle);
		ModelAnimationCoordinateRelation relationType = ModelAnimationParseHelper::parseCoordinateRelation();

		auto submodel = ModelAnimationParseHelper::parseSubmodel();

		if (!submodel) {
			if (data->parentSubmodel) 
				submodel = data->parentSubmodel;
			else
				error_display(1, "Set Orientation has no target submodel!");
		}

		auto segment = std::shared_ptr<ModelAnimationSegmentSetOrientation>(new ModelAnimationSegmentSetOrientation(submodel, angle, relationType));

		return segment;
	}


	ModelAnimationSegmentSetAngle::ModelAnimationSegmentSetAngle(std::shared_ptr<ModelAnimationSubmodel> submodel, float angle) :
		m_submodel(std::move(submodel)), m_angle(angle) { }

	ModelAnimationSegment* ModelAnimationSegmentSetAngle::copy() const {
		return new ModelAnimationSegmentSetAngle(*this);
	}

	void ModelAnimationSegmentSetAngle::recalculate(ModelAnimationSubmodelBuffer& /*base*/, ModelAnimationSubmodelBuffer& /*currentAnimDelta*/, polymodel_instance* pmi) {
		angles angs = vmd_zero_angles;
		auto submodel_info = m_submodel->findSubmodel(pmi).second;
		if (submodel_info == nullptr) {
			m_rot = vmd_identity_matrix;
			m_duration[pmi->id] = 0.0f;
			return;
		}

		switch (submodel_info->rotation_axis_id)
		{
			case MOVEMENT_AXIS_X:
			angs.p = m_angle;
			vm_angles_2_matrix(&m_rot, &angs);
			break;

		case MOVEMENT_AXIS_Y:
			angs.h = m_angle;
			vm_angles_2_matrix(&m_rot, &angs);
			break;

		case MOVEMENT_AXIS_Z:
			angs.b = m_angle;
			vm_angles_2_matrix(&m_rot, &angs);
			break;

		default:
			vm_quaternion_rotate(&m_rot, m_angle, &submodel_info->rotation_axis);
			break;
		}

		m_duration[pmi->id] = 0.0f;
	}

	void ModelAnimationSegmentSetAngle::calculateAnimation(ModelAnimationSubmodelBuffer& base, float /*time*/, int /*pmi_id*/) const {
		ModelAnimationData<true> data;
		data.orientation = m_rot;
		
		base[m_submodel].data.applyDelta(data);
		base[m_submodel].modified = true;
	}

	void ModelAnimationSegmentSetAngle::exchangeSubmodelPointers(ModelAnimationSet& replaceWith) {
		m_submodel = replaceWith.getSubmodel(m_submodel);
	}
	
	std::shared_ptr<ModelAnimationSegment> ModelAnimationSegmentSetAngle::parser(ModelAnimationParseHelper* data) {
		float angle;

		required_string("+Angle:");
		stuff_float(&angle);

		auto submodel = ModelAnimationParseHelper::parseSubmodel();

		if (!submodel) {
			if (data->parentSubmodel)
				submodel = data->parentSubmodel;
			else
				error_display(1, "Set Angle has no target submodel!");
		}

		auto segment = std::shared_ptr<ModelAnimationSegmentSetAngle>(new ModelAnimationSegmentSetAngle(submodel, fl_radians(angle)));

		return segment;
	}


	static constexpr float angles::*pbh[] = { &angles::p, &angles::b, &angles::h };

	ModelAnimationSegmentRotation::ModelAnimationSegmentRotation(std::shared_ptr<ModelAnimationSubmodel> submodel, tl::optional<angles> targetAngle, tl::optional<angles> velocity, tl::optional<float> time, tl::optional<angles> acceleration, ModelAnimationCoordinateRelation relationType) :
		m_submodel(std::move(submodel)), m_targetAngle(targetAngle), m_velocity(velocity), m_time(time), m_acceleration(acceleration), m_relationType(relationType) { }

	ModelAnimationSegment* ModelAnimationSegmentRotation::copy() const {
		return new ModelAnimationSegmentRotation(*this);
	}

	void ModelAnimationSegmentRotation::recalculate(ModelAnimationSubmodelBuffer& base, ModelAnimationSubmodelBuffer& currentAnimDelta, polymodel_instance* pmi) {
		Assertion(!(m_targetAngle.has_value() ^ m_velocity.has_value() ^ m_time.has_value()), "Tried to run over- or underdefined rotation. Define exactly two out of 'time', 'velocity', and 'angle'!");

		instance_data& instanceData = m_instances[pmi->id];
		auto submodel_info = m_submodel->findSubmodel(pmi).second;
		if (submodel_info == nullptr) {
			m_duration[pmi->id] = 0.0f;
			return;
		}

		if (m_targetAngle) { //If we have an angle specified, use it.
			if (m_relationType != ModelAnimationCoordinateRelation::RELATIVE_COORDS) {
				const ModelAnimationData<>& submodel = (m_relationType == ModelAnimationCoordinateRelation::ABSOLUTE_COORDS ? base[m_submodel].data : currentAnimDelta[m_submodel].data);

				matrix orientTransp, target, diff;
				const angles& targetAngle = *m_targetAngle;
				vm_copy_transpose(&orientTransp, &submodel.orientation);
				vm_angles_2_matrix(&target, &targetAngle);
				vm_matrix_x_matrix(&diff, &target, &orientTransp);
				vm_extract_angles_matrix_alternate(&instanceData.m_actualTarget, &diff);
			}
			else
				instanceData.m_actualTarget = *m_targetAngle;
		}
		else { //If we don't have an angle specified, calculate it. This implies we must have velocity and time.
			const angles& v = *m_velocity;
			const float& t = *m_time;

			if (m_acceleration) { //Consider acceleration to calculate the angle
				//Let the following equations define our accelerated and braked movement, under the assumption that 2 * ta <= t.
				//d : distance, v : max velocity, a : acceleration, t : total time, ta : time spent accelerating (and breaking)
				//v = a * ta
				//d = v * (t - 2 * ta) + 1/2 * 2 * a * ta^2
				//this simplifies to d = (v(a * t - v))/a and ta = v / a
				//if 2 * ta <= t does not hold, it's just d = 1/2 * 2 * a * (t/2)^2 -> this implies that the acceleration is too small to reach the target velocity within the specified time.
				angles a = *m_acceleration;
				angles at;

				for (float angles::* i : pbh) {
					a.*i = copysignf(a.*i, v.*i);
					at.*i = fmaxf(v.*i / a.*i, t / 2.0f);
				}
				instanceData.m_actualAccel = a;
				instanceData.m_accelTime = at;

				for (float angles::* i : pbh)
					instanceData.m_actualTarget.*i = 2.0f * v.*i / a.*i <= t ? (v.*i * (a.*i * t - v.*i)) / a.*i: a.*i * (t * t / 4.0f);

			}
			else { //Don't consider acceleration, assume instant velocity.
				for (float angles::* i : pbh)
					instanceData.m_actualTarget.*i = v.*i * t;
			}
		}

		if (m_velocity) { //If we have velocity specified, use it.
			instanceData.m_actualVelocity = *m_velocity;
		}
		else { //If we don't have velocity specified, calculate it. This implies we must have an angle and time.
			const float& t = *m_time;
			const angles& d = instanceData.m_actualTarget;

			if (m_acceleration) { //Consider acceleration to calculate the velocity
				//Assume equations from calc angles case, but solve for ta and v now, under the assumption that these roots have a real solution.
				//v = 1/2*(|a|*t-sqrt(|a|)*sqrt(|a|*t^2-4*|d|))*sign(d) and ta = 1/2*(t-(sqrt(|a|*t^2-4*|d|)/sqrt(|a|)))
				//If the roots don't have a real solution, it's v = a * t/2, and ta = 1/2*t -> this implies that the acceleration is too small to reach the target distance within the specified time.

				angles a = *m_acceleration;
				for (float angles::* i : pbh)
					a.*i = copysignf(a.*i, d.*i);
				instanceData.m_actualAccel = a;

				angles at{ 0,0,0 };

				for (float angles::* i : pbh) {
					float a_abs = fabsf(a.*i);
					float radicant = a_abs * t * t - 4 * fabsf(d.*i);
					if (radicant >= 0) {
						instanceData.m_actualVelocity.*i = copysignf(0.5f * (a_abs * t - sqrtf(a_abs) * sqrtf(radicant)), d.*i);
						at.*i = 0.5f * (t - (sqrtf(radicant) / sqrtf(a_abs)));
					}
					else {
						instanceData.m_actualVelocity.*i = a.*i * t / 2.0f;
						at.*i = t / 2.0f;
					}
				}

				instanceData.m_accelTime = at;
			}
			else { //Don't consider acceleration, assume instant velocity.
				for (float angles::* i : pbh)
					instanceData.m_actualVelocity.*i = d.*i / t;
			}

		}

		if (m_time) { //If we have time specified, use it.
			const float& time = *m_time;
			m_duration[pmi->id] = time;

			angles actualTime{ 0,0,0 };
			for (float angles::* i : pbh)
				actualTime.*i = time;

			instanceData.m_actualTime = actualTime;

			if (time <= 0.0f) {

				Error(LOCATION, "Tried to rotate submodel %s in %.2f seconds. Rotation time must be positive and nonzero!", submodel_info->name, time);
			}
		}
		else { //Calc time
			float& duration = m_duration[pmi->id] = 0.0f;

			angles& v = instanceData.m_actualVelocity;
			const angles& d = instanceData.m_actualTarget;

			angles actualAccel{ 0,0,0 };
			angles accelTime{ 0,0,0 };
			angles actualTime{ 0,0,0 };

			for (float angles::* i : pbh) {
				if (d.*i != 0.0f) {
					if (v.*i == 0.0f) {
						Warning(LOCATION, "Tried to rotate submodel %s by %.2f, but velocity was 0! Rotating with velocity 1...", submodel_info->name, d.*i);
						v.*i = 1;
					}

					v.*i = copysignf(v.*i, d.*i);

					float durationAxis = 0.0f;

					if (m_acceleration) { //Consider acceleration to calculate the time
						//Assume equations from calc angles case, but solve for ta and t now, with the resulting ta <= t / 2.
						//t = v/a+d/v and ta = v/a
						//If thus d/v < v/a, it's t = 2*sqrt(d/a), and ta = 1/2*t -> this implies that the acceleration is too small to reach the target velocity within the specified distance.
						float a = copysignf((*m_acceleration).*i, d.*i);
						actualAccel.*i = a;

						float va = v.*i / a;
						float dv = d.*i / v.*i;

						if (dv >= va) {
							durationAxis = va + dv;
							accelTime.*i = va;
						}
						else {
							durationAxis = 2.0f * sqrtf(d.*i / a);
							accelTime.*i = durationAxis / 2.0f;
						}
					}
					else {
						durationAxis = d.*i / v.*i;
					}

					duration = duration < durationAxis ? durationAxis : duration;
					actualTime.*i = durationAxis;
				}
			}
			
			if (m_acceleration) {
				instanceData.m_actualAccel = actualAccel;
				instanceData.m_accelTime = accelTime;
			}

			instanceData.m_actualTime = actualTime;
		}
	}

	void ModelAnimationSegmentRotation::calculateAnimation(ModelAnimationSubmodelBuffer& base, float time, int pmi_id) const {
		const instance_data& instanceData = m_instances.at(pmi_id);

		angles currentRot{ 0,0,0 };

		if (instanceData.m_actualAccel) {
			const angles& a = *instanceData.m_actualAccel;
			const angles& at = *instanceData.m_accelTime;
			const angles& v = instanceData.m_actualVelocity;
			const angles& t = instanceData.m_actualTime;

			for (float angles::* i : pbh) {
				float acceltime1 = fminf(time, at.*i);
				currentRot.*i = 0.5f * a.*i * acceltime1 * acceltime1;
				
				float lineartime = fminf(time - at.*i, t.*i - 2.0f * at.*i);
				if (lineartime > 0) {
					currentRot.*i += v.*i * lineartime;
				}

				float acceltime2 = fminf(time - (t.*i - at.*i), at.*i);
				if (acceltime2 > 0) {
					//Cap this to 0, as it could get negative if it rotates "longer than its duration" (which happens when a different axis takes longer to rotate)
					float accel2dist = at.*i * a.*i * acceltime2 - 0.5f * a.*i * acceltime2 * acceltime2;
					if(std::signbit(a.*i))
						currentRot.*i += fminf(accel2dist, 0.0f);
					else
						currentRot.*i += fmaxf(accel2dist, 0.0f);
				}
			}
		}
		else {
			for (float angles::* i : pbh)// Linear Rotation
				currentRot.*i = instanceData.m_actualVelocity.*i * time;
		}

		for (float angles::* i : pbh) { // Clamp rotation to actual target
			if (instanceData.m_actualTarget.*i < 0) {
				currentRot.*i = fmaxf(instanceData.m_actualTarget.*i, currentRot.*i);
			}
			else {
				currentRot.*i = fminf(instanceData.m_actualTarget.*i, currentRot.*i);
			}
		}

		matrix orient;
		vm_angles_2_matrix(&orient, &currentRot);

		ModelAnimationData<true> delta;
		delta.orientation = orient;

		base[m_submodel].data.applyDelta(delta);
		base[m_submodel].modified = true;
	}

	void ModelAnimationSegmentRotation::exchangeSubmodelPointers(ModelAnimationSet& replaceWith) {
		m_submodel = replaceWith.getSubmodel(m_submodel);
	}
	
	std::shared_ptr<ModelAnimationSegment> ModelAnimationSegmentRotation::parser(ModelAnimationParseHelper* data) {
		tl::optional<angles> angle, velocity, acceleration;
		tl::optional<float> time;
		ModelAnimationCoordinateRelation relationType = ModelAnimationCoordinateRelation::RELATIVE_COORDS;

		if (optional_string("+Angle:")) {
			angles parse;
			stuff_angles_deg_phb(&parse);
			angle = std::move(parse);
			relationType = ModelAnimationParseHelper::parseCoordinateRelation();
		}

		if (optional_string("+Velocity:")) {
			angles parse;
			stuff_angles_deg_phb(&parse);
			velocity = std::move(parse);
		}

		if (optional_string("+Time:")) {
			float parse;
			stuff_float(&parse);
			time = parse;
		}

		if (angle.has_value() ^ velocity.has_value() ^ time.has_value()) {
			error_display(1, "Rotation must have exactly two values out of angle, velocity and time specified!");
		}

		if (optional_string("+Acceleration:")) {
			angles parse;
			stuff_angles_deg_phb(&parse);
			acceleration = std::move(parse);
		}

		auto submodel = ModelAnimationParseHelper::parseSubmodel();
		if (!submodel) {
			if (data->parentSubmodel)
				submodel = data->parentSubmodel;
			else
				error_display(1, "Rotation has no target submodel!");
		}

		auto segment = std::shared_ptr<ModelAnimationSegmentRotation>(new ModelAnimationSegmentRotation(submodel, angle, velocity, time, acceleration, relationType));

		return segment;
	}


	ModelAnimationSegmentAxisRotation::ModelAnimationSegmentAxisRotation(std::shared_ptr<ModelAnimationSubmodel> submodel, tl::optional<float> targetAngle, tl::optional<float> velocity, tl::optional<float> time, tl::optional<float> acceleration, const vec3d& axis) :
			m_submodel(std::move(submodel)), m_targetAngle(targetAngle), m_velocity(velocity), m_time(time), m_acceleration(acceleration) {
		vm_vec_copy_normalize(&m_axis, &axis);
	}

	ModelAnimationSegment* ModelAnimationSegmentAxisRotation::copy() const {
		return new ModelAnimationSegmentAxisRotation(*this);
	}

	void ModelAnimationSegmentAxisRotation::recalculate(ModelAnimationSubmodelBuffer& /*base*/, ModelAnimationSubmodelBuffer& /*currentAnimDelta*/, polymodel_instance* pmi) {
		Assertion(!(m_targetAngle.has_value() ^ m_velocity.has_value() ^ m_time.has_value()), "Tried to run over- or underdefined rotation. Define exactly two out of 'time', 'velocity', and 'angle'!");

		instance_data& instanceData = m_instances[pmi->id];
		auto submodel_info = m_submodel->findSubmodel(pmi).second;
		if (submodel_info == nullptr) {
			m_duration[pmi->id] = 0.0f;
			return;
		}

		if (m_targetAngle) { //If we have an angle specified, use it.
			instanceData.m_actualTarget = *m_targetAngle;
		}
		else { //If we don't have an angle specified, calculate it. This implies we must have velocity and time.
			const float& v = *m_velocity;
			const float& t = *m_time;

			if (m_acceleration) { //Consider acceleration to calculate the angle
				//Let the following equations define our accelerated and braked movement, under the assumption that 2 * ta <= t.
				//d : distance, v : max velocity, a : acceleration, t : total time, ta : time spent accelerating (and breaking)
				//v = a * ta
				//d = v * (t - 2 * ta) + 1/2 * 2 * a * ta^2
				//this simplifies to d = (v(a * t - v))/a and ta = v / a
				//if 2 * ta <= t does not hold, it's just d = 1/2 * 2 * a * (t/2)^2 -> this implies that the acceleration is too small to reach the target velocity within the specified time.
				float a = *m_acceleration;
				a = copysignf(a, v);
				instanceData.m_actualAccel = a;
				instanceData.m_accelTime = fmaxf(v / a, t / 2.0f);
				
				instanceData.m_actualTarget = 2.0f * v / a <= t ? (v * (a * t - v)) / a: a * (t * t / 4.0f);

			}
			else { //Don't consider acceleration, assume instant velocity.
				instanceData.m_actualTarget = v * t;
			}
		}

		if (m_velocity) { //If we have velocity specified, use it.
			instanceData.m_actualVelocity = *m_velocity;
		}
		else { //If we don't have velocity specified, calculate it. This implies we must have an angle and time.
			const float& t = *m_time;
			const float& d = instanceData.m_actualTarget;

			if (m_acceleration) { //Consider acceleration to calculate the velocity
				//Assume equations from calc angles case, but solve for ta and v now, under the assumption that these roots have a real solution.
				//v = 1/2*(|a|*t-sqrt(|a|)*sqrt(|a|*t^2-4*|d|))*sign(d) and ta = 1/2*(t-(sqrt(|a|*t^2-4*|d|)/sqrt(|a|)))
				//If the roots don't have a real solution, it's v = a * t/2, and ta = 1/2*t -> this implies that the acceleration is too small to reach the target distance within the specified time.

				float a = *m_acceleration;
				float at;
				a = copysignf(a, d);
				instanceData.m_actualAccel = a;
				
				float a_abs = fabsf(a);
				float radicant = a_abs * t * t - 4 * fabsf(d);
				if (radicant >= 0) {
					instanceData.m_actualVelocity = copysignf(0.5f * (a_abs * t - sqrtf(a_abs) * sqrtf(radicant)), d);
					at = 0.5f * (t - (sqrtf(radicant) / sqrtf(a_abs)));
				}
				else {
					instanceData.m_actualVelocity = a * t / 2.0f;
					at = t / 2.0f;
				}
				
				instanceData.m_accelTime = at;
			}
			else { //Don't consider acceleration, assume instant velocity.
				instanceData.m_actualVelocity = d / t;
			}

		}

		if (m_time) { //If we have time specified, use it.
			const float& time = *m_time;
			m_duration[pmi->id] = time;

			instanceData.m_actualTime = time;

			if (time <= 0.0f) {
				Error(LOCATION, "Tried to rotate submodel %s in %.2f seconds. Rotation time must be positive and nonzero!", submodel_info->name, time);
			}
		}
		else { //Calc time
			float& duration = m_duration[pmi->id] = 0.0f;

			float& v = instanceData.m_actualVelocity;
			const float& d = instanceData.m_actualTarget;

			float actualAccel = 0;
			float accelTime = 0;
			
			if (d != 0.0f) {
				if (v == 0.0f) {
					Warning(LOCATION, "Tried to rotate submodel %s by %.2f, but velocity was 0! Rotating with velocity 1...", submodel_info->name, d);
					v = 1;
				}

				v = copysignf(v, d);

				if (m_acceleration) { //Consider acceleration to calculate the time
					//Assume equations from calc angles case, but solve for ta and t now, with the resulting ta <= t / 2.
					//t = v/a+d/v and ta = v/a
					//If thus d/v < v/a, it's t = 2*sqrt(d/a), and ta = 1/2*t -> this implies that the acceleration is too small to reach the target velocity within the specified distance.
					float a = copysignf(*m_acceleration, d);
					actualAccel = a;

					float va = v / a;
					float dv = d / v;

					if (dv >= va) {
						duration = va + dv;
						accelTime = va;
					}
					else {
						duration = 2.0f * sqrtf(d / a);
						accelTime = duration / 2.0f;
					}
				}
				else {
					duration = d / v;
				}
			}
			

			if (m_acceleration) {
				instanceData.m_actualAccel = actualAccel;
				instanceData.m_accelTime = accelTime;
			}

			instanceData.m_actualTime = duration;
		}
	}

	void ModelAnimationSegmentAxisRotation::calculateAnimation(ModelAnimationSubmodelBuffer& base, float time, int pmi_id) const {
		const instance_data& instanceData = m_instances.at(pmi_id);

		float currentRot = 0;

		if (instanceData.m_actualAccel) {
			const float& a = *instanceData.m_actualAccel;
			const float& at = *instanceData.m_accelTime;
			const float& v = instanceData.m_actualVelocity;
			const float& t = instanceData.m_actualTime;

			float acceltime1 = fminf(time, at);
			currentRot = 0.5f * a * acceltime1 * acceltime1;

			float lineartime = fminf(time - at, t - 2.0f * at);
			if (lineartime > 0) {
				currentRot += v * lineartime;
			}

			float acceltime2 = fminf(time - (t - at), at);
			if (acceltime2 > 0) {
				//Cap this to 0, as it could get negative if it rotates "longer than its duration" (which happens when a different axis takes longer to rotate)
				float accel2dist = at * a * acceltime2 - 0.5f * a * acceltime2 * acceltime2;
				if(std::signbit(a))
					currentRot += fminf(accel2dist, 0.0f);
				else
					currentRot += fmaxf(accel2dist, 0.0f);
			}
			
		}
		else {
			// Linear Rotation
			currentRot = instanceData.m_actualVelocity * time;
		}

		// Clamp rotation to actual target
		if (instanceData.m_actualTarget < 0) {
			currentRot = fmaxf(instanceData.m_actualTarget, currentRot);
		}
		else {
			currentRot = fminf(instanceData.m_actualTarget, currentRot);
		}
		
		matrix orient;
		vm_quaternion_rotate(&orient, currentRot, &m_axis);

		ModelAnimationData<true> delta;
		delta.orientation = orient;

		base[m_submodel].data.applyDelta(delta);
		base[m_submodel].modified = true;
	}

	void ModelAnimationSegmentAxisRotation::exchangeSubmodelPointers(ModelAnimationSet& replaceWith) {
		m_submodel = replaceWith.getSubmodel(m_submodel);
	}

	std::shared_ptr<ModelAnimationSegment> ModelAnimationSegmentAxisRotation::parser(ModelAnimationParseHelper* data) {
		tl::optional<float> angle, velocity, acceleration, time;
		vec3d axis;

		required_string("+Axis:");
		stuff_vec3d(&axis);
		
		if (optional_string("+Angle:")) {
			float parse;
			stuff_float(&parse);
			angle = fl_radians(parse);
		}

		if (optional_string("+Velocity:")) {
			float parse;
			stuff_float(&parse);
			velocity = fl_radians(parse);
		}

		if (optional_string("+Time:")) {
			float parse;
			stuff_float(&parse);
			time = parse;
		}

		if (angle.has_value() ^ velocity.has_value() ^ time.has_value()) {
			error_display(1, "Axis Rotation must have exactly two values out of angle, velocity and time specified!");
		}

		if (optional_string("+Acceleration:")) {
			float parse;
			stuff_float(&parse);
			acceleration = fl_radians(parse);
		}

		auto submodel = ModelAnimationParseHelper::parseSubmodel();
		if (!submodel) {
			if (data->parentSubmodel)
				submodel = data->parentSubmodel;
			else
				error_display(1, "Rotation has no target submodel!");
		}

		auto segment = std::shared_ptr<ModelAnimationSegmentAxisRotation>(new ModelAnimationSegmentAxisRotation(submodel, angle, velocity, time, acceleration, axis));

		return segment;
	}
	

	ModelAnimationSegmentTranslation::ModelAnimationSegmentTranslation(std::shared_ptr<ModelAnimationSubmodel> submodel, tl::optional<vec3d> target, tl::optional<vec3d> velocity, tl::optional<float> time, tl::optional<vec3d> acceleration, CoordinateSystem coordType, ModelAnimationCoordinateRelation relationType) :
		m_submodel(std::move(submodel)), m_target(target), m_velocity(velocity), m_time(time), m_acceleration(acceleration), m_coordType(coordType), m_relationType(relationType) { }

	ModelAnimationSegment* ModelAnimationSegmentTranslation::copy() const {
		return new ModelAnimationSegmentTranslation(*this);
	}

	void ModelAnimationSegmentTranslation::recalculate(ModelAnimationSubmodelBuffer& base, ModelAnimationSubmodelBuffer& currentAnimDelta, polymodel_instance* pmi) {
		Assertion(!(m_target.has_value() ^ m_velocity.has_value() ^ m_time.has_value()), "Tried to run over- or underdefined translation. Define exactly two out of 'time', 'velocity', and 'vector'!");

		instance_data& instanceData = m_instances[pmi->id];
		auto submodel_info = m_submodel->findSubmodel(pmi).second;
		if (submodel_info == nullptr) {
			m_duration[pmi->id] = 0.0f;
			return;
		}

		if (m_target) { //If we have an target specified, use it.
			if (m_relationType != ModelAnimationCoordinateRelation::RELATIVE_COORDS) {
				const ModelAnimationData<>& submodel = (m_relationType == ModelAnimationCoordinateRelation::ABSOLUTE_COORDS ? base[m_submodel].data : currentAnimDelta[m_submodel].data);
				instanceData.m_actualTarget = *m_target - submodel.position;
			}
			else
				instanceData.m_actualTarget = *m_target;
		}
		else { //If we don't have a target specified, calculate it. This implies we must have velocity and time.
			const vec3d& v = *m_velocity;
			const float& t = *m_time;

			if (m_acceleration) { //Consider acceleration to calculate the angle
				//Let the following equations define our accelerated and braked movement, under the assumption that 2 * ta <= t.
				//d : distance, v : max velocity, a : acceleration, t : total time, ta : time spent accelerating (and breaking)
				//v = a * ta
				//d = v * (t - 2 * ta) + 1/2 * 2 * a * ta^2
				//this simplifies to d = (v(a * t - v))/a and ta = v / a
				//if 2 * ta <= t does not hold, it's just d = 1/2 * 2 * a * (t/2)^2 -> this implies that the acceleration is too small to reach the target velocity within the specified time.
				vec3d a = *m_acceleration;
				vec3d at;

				for (size_t i = 0; i < 3; i++) {
					a.a1d[i] = copysignf(a.a1d[i], v.a1d[i]);
					at.a1d[i] = fmaxf(v.a1d[i] / a.a1d[i], t / 2.0f);
				}
				instanceData.m_actualAccel = a;
				instanceData.m_accelTime = at;

				for (size_t i = 0; i < 3; i++)
					instanceData.m_actualTarget.a1d[i] = 2.0f * v.a1d[i] / a.a1d[i] <= t ? (v.a1d[i] * (a.a1d[i] * t - v.a1d[i])) / a.a1d[i] : a.a1d[i] * (t * t / 4.0f);

			}
			else { //Don't consider acceleration, assume instant velocity.
				for (size_t i = 0; i < 3; i++)
					instanceData.m_actualTarget.a1d[i] = v.a1d[i] * t;
			}
		}

		if (m_velocity) { //If we have velocity specified, use it.
			instanceData.m_actualVelocity = *m_velocity;
		}
		else { //If we don't have velocity specified, calculate it. This implies we must have an angle and time.
			const float& t = *m_time;
			const vec3d& d = instanceData.m_actualTarget;

			if (m_acceleration) { //Consider acceleration to calculate the velocity
				//Assume equations from calc angles case, but solve for ta and v now, under the assumption that these roots have a real solution.
				//v = 1/2*(|a|*t-sqrt(|a|)*sqrt(|a|*t^2-4*|d|))*sign(d) and ta = 1/2*(t-(sqrt(|a|*t^2-4*|d|)/sqrt(|a|)))
				//If the roots don't have a real solution, it's v = a * t/2, and ta = 1/2*t -> this implies that the acceleration is too small to reach the target distance within the specified time.

				vec3d a = *m_acceleration;
				for (size_t i = 0; i < 3; i++)
					a.a1d[i] = copysignf(a.a1d[i], d.a1d[i]);
				instanceData.m_actualAccel = a;

				vec3d at{ {{ 0,0,0}} };

				for (size_t i = 0; i < 3; i++) {
					float a_abs = fabsf(a.a1d[i]);
					float radicant = a_abs * t * t - 4 * fabsf(d.a1d[i]);
					if (radicant >= 0) {
						instanceData.m_actualVelocity.a1d[i] = copysignf(0.5f * (a_abs * t - sqrtf(a_abs) * sqrtf(radicant)), d.a1d[i]);
						at.a1d[i] = 0.5f * (t - (sqrtf(radicant) / sqrtf(a_abs)));
					}
					else {
						instanceData.m_actualVelocity.a1d[i] = a.a1d[i] * t / 2.0f;
						at.a1d[i] = t / 2.0f;
					}
				}

				instanceData.m_accelTime = at;
			}
			else { //Don't consider acceleration, assume instant velocity.
				for (size_t i = 0; i < 3; i++)
					instanceData.m_actualVelocity.a1d[i] = d.a1d[i] / t;
			}

		}

		if (m_time) { //If we have time specified, use it.
			const float& time = *m_time;
			m_duration[pmi->id] = time;

			vec3d actualTime{ {{ 0,0,0 }} };
			for (float& timeAxis : actualTime.a1d)
				timeAxis = time;

			instanceData.m_actualTime = actualTime;

			if (time <= 0.0f) {

				Error(LOCATION, "Tried to translate submodel %s in %.2f seconds. Rotation time must be positive and nonzero!", submodel_info->name, time);
			}
		}
		else { //Calc time
			float& duration = m_duration[pmi->id] = 0.0f;

			vec3d& v = instanceData.m_actualVelocity;
			const vec3d& d = instanceData.m_actualTarget;

			vec3d actualAccel{ {{ 0,0,0 }} };
			vec3d accelTime{ {{ 0,0,0 }} };
			vec3d actualTime{ {{ 0,0,0 }} };

			for (size_t i = 0; i < 3; i++) {
				if (d.a1d[i] != 0.0f) {
					if (v.a1d[i] == 0.0f) {
						Warning(LOCATION, "Tried to translate submodel %s by %.2f, but velocity was 0! Rotating with velocity 1...", submodel_info->name, d.a1d[i]);
						v.a1d[i] = 1;
					}

					v.a1d[i] = copysignf(v.a1d[i], d.a1d[i]);

					float durationAxis = 0.0f;

					if (m_acceleration) { //Consider acceleration to calculate the time
						//Assume equations from calc angles case, but solve for ta and t now, with the resulting ta <= t / 2.
						//t = v/a+d/v and ta = v/a
						//If thus d/v < v/a, it's t = 2*sqrt(d/a), and ta = 1/2*t -> this implies that the acceleration is too small to reach the target velocity within the specified distance.
						float a = copysignf((*m_acceleration).a1d[i], d.a1d[i]);
						actualAccel.a1d[i] = a;

						float va = v.a1d[i] / a;
						float dv = d.a1d[i] / v.a1d[i];

						if (dv >= va) {
							durationAxis = va + dv;
							accelTime.a1d[i] = va;
						}
						else {
							durationAxis = 2.0f * sqrtf(d.a1d[i] / a);
							accelTime.a1d[i] = durationAxis / 2.0f;
						}
					}
					else {
						durationAxis = d.a1d[i] / v.a1d[i];
					}

					duration = duration < durationAxis ? durationAxis : duration;
					actualTime.a1d[i] = durationAxis;
				}
			}

			if (m_acceleration) {
				instanceData.m_actualAccel = actualAccel;
				instanceData.m_accelTime = accelTime;
			}

			instanceData.m_actualTime = actualTime;
		}

		if (CoordinateSystem::COORDS_LOCAL_AT_START == m_coordType) {
			instanceData.m_rotationAtStart = base[m_submodel].data.orientation;
		}
	}

	void ModelAnimationSegmentTranslation::calculateAnimation(ModelAnimationSubmodelBuffer& base, float time, int pmi_id) const {
		const instance_data& instanceData = m_instances.at(pmi_id);

		vec3d currentOffset{ {{ 0,0,0 }} };

		if (instanceData.m_actualAccel) {
			const vec3d& a = *instanceData.m_actualAccel;
			const vec3d& at = *instanceData.m_accelTime;
			const vec3d& v = instanceData.m_actualVelocity;
			const vec3d& t = instanceData.m_actualTime;

			for (size_t i = 0; i < 3; i++) {
				float acceltime1 = fminf(time, at.a1d[i]);
				currentOffset.a1d[i] = 0.5f * a.a1d[i] * acceltime1 * acceltime1;

				float lineartime = fminf(time - at.a1d[i], t.a1d[i] - 2.0f * at.a1d[i]);
				if (lineartime > 0) {
					currentOffset.a1d[i] += v.a1d[i] * lineartime;
				}

				float acceltime2 = fminf(time - (t.a1d[i] - at.a1d[i]), at.a1d[i]);
				if (acceltime2 > 0) {
					//Cap this to 0, as it could get negative if it translates "longer than its duration" (which happens when a different axis takes longer to translate)
					float accel2dist = at.a1d[i] * a.a1d[i] * acceltime2 - 0.5f * a.a1d[i] * acceltime2 * acceltime2;
					if (std::signbit(a.a1d[i]))
						currentOffset.a1d[i] += fminf(accel2dist, 0.0f);
					else
						currentOffset.a1d[i] += fmaxf(accel2dist, 0.0f);
				}
			}
		}
		else {
			for (size_t i = 0; i < 3; i++)// Linear translation
				currentOffset.a1d[i] = instanceData.m_actualVelocity.a1d[i] * time;
		}

		for (size_t i = 0; i < 3; i++) { // Clamp translation to actual target
			if (instanceData.m_actualTarget.a1d[i] < 0) {
				currentOffset.a1d[i] = fmaxf(instanceData.m_actualTarget.a1d[i], currentOffset.a1d[i]);
			}
			else {
				currentOffset.a1d[i] = fminf(instanceData.m_actualTarget.a1d[i], currentOffset.a1d[i]);
			}
		}

		vec3d finalOffset = currentOffset;
		auto& submodelInstance = base[m_submodel];

		switch (m_coordType) {
		case CoordinateSystem::COORDS_LOCAL_AT_START:
			//Use the local coordinates this submodel had at the start of this animation, if no other animations would be running. Useful for cases where your movement shouldn't be modified by anything else
			vm_vec_rotate(&finalOffset, &currentOffset, &instanceData.m_rotationAtStart);
			break;
		case CoordinateSystem::COORDS_LOCAL_CURRENT:
			//Use the local coordinates this submodel has right now, including all other animations running. Useful if you want your submodel to move visibly on it's own axes
			vm_vec_rotate(&finalOffset, &currentOffset, &submodelInstance.data.orientation);
			break;
		case CoordinateSystem::COORDS_PARENT:
		default:
			//Trivial case. Specified coordinates are assumed to be in relation to this submodel's coordinate system (dependent on its parent's rotation, but not on its own).
			//No rotation necessary.
			break;
		}

		ModelAnimationData<true> delta;
		delta.position = finalOffset;

		submodelInstance.data.applyDelta(delta);
		submodelInstance.modified = true;
	}

	void ModelAnimationSegmentTranslation::exchangeSubmodelPointers(ModelAnimationSet& replaceWith) {
		m_submodel = replaceWith.getSubmodel(m_submodel);
	}

	std::shared_ptr<ModelAnimationSegment> ModelAnimationSegmentTranslation::parser(ModelAnimationParseHelper* data) {
		tl::optional<vec3d> offset, velocity, acceleration;
		tl::optional<float> time;
		CoordinateSystem coordSystem = CoordinateSystem::COORDS_PARENT;
		ModelAnimationCoordinateRelation relationType = ModelAnimationCoordinateRelation::RELATIVE_COORDS;

		if (optional_string("+Vector:")) {
			vec3d parse;
			stuff_vec3d(&parse);
			offset = std::move(parse);
			relationType = ModelAnimationParseHelper::parseCoordinateRelation();
		}

		if (optional_string("+Velocity:")) {
			vec3d parse;
			stuff_vec3d(&parse);
			velocity = std::move(parse);
		}

		if (optional_string("+Time:")) {
			float parse;
			stuff_float(&parse);
			time = parse;
		}

		if (offset.has_value() ^ velocity.has_value() ^ time.has_value()) {
			error_display(1, "Translation must have exactly two values out of vector, velocity and time specified!");
		}

		if (optional_string("+Acceleration:")) {
			vec3d parse;
			stuff_vec3d(&parse);
			acceleration = std::move(parse);
		}

		if (optional_string("+Coordinate System:")) {
			switch (optional_string_one_of(3, "Parent", "Local at start", "Local current")) {
			case 0:
				coordSystem = CoordinateSystem::COORDS_PARENT;
				break;
			case 1:
				coordSystem = CoordinateSystem::COORDS_LOCAL_AT_START;
				break;
			case 2:
				coordSystem = CoordinateSystem::COORDS_LOCAL_CURRENT;
				break;
			default:
				error_display(0, "Unknown translation coordinate system! Assuming 'Parent'.");
				break;
			}
		}

		auto submodel = ModelAnimationParseHelper::parseSubmodel();
		if (!submodel) {
			if (data->parentSubmodel)
				submodel = data->parentSubmodel;
			else
				error_display(1, "Translation has no target submodel!");
		}

		auto segment = std::shared_ptr<ModelAnimationSegmentTranslation>(new ModelAnimationSegmentTranslation(submodel, offset, velocity, time, acceleration, coordSystem, relationType));

		return segment;
	}


	ModelAnimationSegmentSoundDuring::ModelAnimationSegmentSoundDuring(std::shared_ptr<ModelAnimationSegment> segment, gamesnd_id start, gamesnd_id end, gamesnd_id during, bool flipIfReversed, bool abortPlayingSounds, float radius, std::shared_ptr<ModelAnimationSubmodel> submodel, tl::optional<vec3d> position) :
		m_segment(std::move(segment)), m_submodel(submodel), m_position(std::move(position)), m_radius(radius), m_start(start), m_end(end), m_during(during), m_flipIfReversed(flipIfReversed), m_abortSoundIfRunning(abortPlayingSounds) { }

	ModelAnimationSegment* ModelAnimationSegmentSoundDuring::copy() const {
		auto newCopy = new ModelAnimationSegmentSoundDuring(*this);
		newCopy->m_segment = std::shared_ptr<ModelAnimationSegment>(newCopy->m_segment->copy());
		return newCopy;
	}

	void ModelAnimationSegmentSoundDuring::recalculate(ModelAnimationSubmodelBuffer& base, ModelAnimationSubmodelBuffer& currentAnimDelta, polymodel_instance* pmi) {
		m_segment->recalculate(base, currentAnimDelta, pmi);
		m_duration[pmi->id] = m_segment->getDuration(pmi->id);
	}

	void ModelAnimationSegmentSoundDuring::calculateAnimation(ModelAnimationSubmodelBuffer& base, float time, int pmi_id) const {
		m_segment->calculateAnimation(base, time, pmi_id);
	}

	void ModelAnimationSegmentSoundDuring::executeAnimation(const ModelAnimationSubmodelBuffer& state, float timeboundLower, float timeboundUpper, ModelAnimationDirection direction, int pmi_id) {
		polymodel_instance* pmi = model_get_instance(pmi_id);

		if (timeboundLower <= 0.0f && 0.0f <= timeboundUpper) {
			if (!m_flipIfReversed || direction == ModelAnimationDirection::FWD)
				playStartSnd(pmi);
			else
				playEndSnd(pmi);
		}

		if (0.0f < timeboundLower && timeboundUpper < m_duration.at(pmi_id)) {
			if (m_during.isValid() && (!m_instances[pmi_id].currentlyPlaying.isValid() || !snd_is_playing(m_instances[pmi_id].currentlyPlaying)))
				m_instances[pmi_id].currentlyPlaying = playSnd(pmi, m_during, true);
		}

		if (timeboundLower <= m_duration.at(pmi_id) && m_duration.at(pmi_id) <= timeboundUpper) {
			if (!m_flipIfReversed || direction == ModelAnimationDirection::FWD)
				playEndSnd(pmi);
			else
				playStartSnd(pmi);
		}
		m_segment->executeAnimation(state, timeboundLower, timeboundUpper, direction, pmi_id);
	}

	void ModelAnimationSegmentSoundDuring::exchangeSubmodelPointers(ModelAnimationSet& replaceWith) {
		m_segment->exchangeSubmodelPointers(replaceWith);
	}

	void ModelAnimationSegmentSoundDuring::playStartSnd(polymodel_instance* pmi) {
		if (m_abortSoundIfRunning && snd_is_playing(m_instances[pmi->id].currentlyPlaying))
			snd_stop(m_instances[pmi->id].currentlyPlaying);
		
		if(m_start.isValid())
			m_instances[pmi->id].currentlyPlaying = playSnd(pmi, m_start, false);
	}
	void ModelAnimationSegmentSoundDuring::playEndSnd(polymodel_instance* pmi) {
		if (m_abortSoundIfRunning && snd_is_playing(m_instances[pmi->id].currentlyPlaying))
			snd_stop(m_instances[pmi->id].currentlyPlaying);

		if (m_end.isValid()) 
			m_instances[pmi->id].currentlyPlaying = playSnd(pmi, m_end, false);
	}

	sound_handle ModelAnimationSegmentSoundDuring::playSnd(polymodel_instance* pmi, const gamesnd_id& sound, bool loop) {
		if (m_submodel != nullptr && pmi->objnum >= 0) {
			auto submodel = m_submodel->findSubmodel(pmi);
			if (submodel.first != nullptr) {
				const object& obj = Objects[pmi->objnum];

				vec3d pnt;
				model_instance_local_to_global_point(&pnt, m_position ? &(*m_position) : &vmd_zero_vector, pmi->id, (int)(submodel.first - pmi->submodel), &obj.orient, &obj.pos);

				return snd_play_3d(gamesnd_get_game_sound(sound), &pnt, &View_position, m_radius, nullptr, loop ? 1 : 0);
			}
		}

		if(loop)
			return snd_play_looping(gamesnd_get_game_sound(sound));
		else
			return snd_play(gamesnd_get_game_sound(sound));
	}
	
	std::shared_ptr<ModelAnimationSegment> ModelAnimationSegmentSoundDuring::parser(ModelAnimationParseHelper* data) {
		gamesnd_id start_sound;
		gamesnd_id loop_sound;
		gamesnd_id end_sound;
		float snd_rad = 0.0f;
		
		auto submodel = ModelAnimationParseHelper::parseSubmodel();
		if (!submodel) {
			if (data->parentSubmodel)
				submodel = data->parentSubmodel;
		}

		parse_game_sound("+Start:", &start_sound);
		parse_game_sound("+Loop:", &loop_sound);
		parse_game_sound("+End:", &end_sound);

		if (optional_string("+Radius:")) {
			stuff_float(&snd_rad);
		}

		tl::optional<vec3d> position = tl::nullopt;
		if (optional_string("+Position:")) {
			if (!submodel)
				error_display(1, "Supplied sound position for animation but no parent submodel. Cannot play sound as 3D without attachment to submodel.");

			vec3d parse;
			stuff_vec3d(&parse);
			position = std::move(parse);
		}

		bool flipIfReversed = optional_string("+Flip When Reversed");
		bool abortSoundIfRunning = !optional_string("+Don't Interrupt Playing Sounds");

		auto segment = std::shared_ptr<ModelAnimationSegmentSoundDuring>(new ModelAnimationSegmentSoundDuring(data->parseSegment(), start_sound, end_sound, loop_sound, flipIfReversed, abortSoundIfRunning, snd_rad, submodel, position));

		return segment;
	}


	ModelAnimationSegmentIK::ModelAnimationSegmentIK(const vec3d& targetPosition, const tl::optional<matrix>& targetRotation)
		: m_targetPosition(targetPosition), m_targetRotation(targetRotation) { }
	
	ModelAnimationSegment* ModelAnimationSegmentIK::copy() const {
		auto* copy = new ModelAnimationSegmentIK(*this);
		copy->m_segment = std::shared_ptr<ModelAnimationSegmentParallel>(static_cast<ModelAnimationSegmentParallel*>(m_segment->copy()));
		
		for(size_t i = 0; i < m_chain.size(); ++i){
			copy->m_chain[i].animSegment = std::static_pointer_cast<ModelAnimationSegmentRotation>(copy->m_segment->m_segments[i]);
		}
		
		return copy;
	};
	
	void ModelAnimationSegmentIK::recalculate(ModelAnimationSubmodelBuffer& base, ModelAnimationSubmodelBuffer& currentAnimDelta, polymodel_instance* pmi) {
		auto ik = std::unique_ptr<ik_solver>(new ik_solver_fabrik());

		polymodel* pm = model_get(pmi->model_num);
		bsp_info* lastSubmodel = nullptr;
		
		for(const auto& chainlink : m_chain) {
			bsp_info* submodel = chainlink.submodel->findSubmodel(pmi).second;
			if(lastSubmodel != nullptr) {
				//Validate chain for current POF
				if(&pm->submodel[submodel->parent] != lastSubmodel){
					Error(LOCATION, "Tried to perform IK on a non-continuous chain. Submodel %s must be an immediate child of the previous chain-link's submodel %s.", submodel->name, lastSubmodel->name);
				}
			}
			lastSubmodel = submodel;
			
			ik->addNode(submodel, chainlink.constraint.get());
		}
		
		ik->solve(m_targetPosition, &(*m_targetRotation));
		
		auto chainlink_it = m_chain.cbegin();
		for(const auto& solvedlink : *ik){
			angles converted;
			vm_extract_angles_matrix_alternate(&converted, &solvedlink.calculatedRot);
			chainlink_it++->animSegment->m_targetAngle = converted;
		}
		
		std::static_pointer_cast<ModelAnimationSegment>(m_segment)->recalculate(base, currentAnimDelta, pmi);
		m_duration[pmi->id] = m_segment->getDuration(pmi->id);
	};
	
	void ModelAnimationSegmentIK::calculateAnimation(ModelAnimationSubmodelBuffer& base, float time, int pmi_id) const {
		std::static_pointer_cast<ModelAnimationSegment>(m_segment)->calculateAnimation(base, time, pmi_id);
	};
	
	void ModelAnimationSegmentIK::exchangeSubmodelPointers(ModelAnimationSet& replaceWith) {
		std::static_pointer_cast<ModelAnimationSegment>(m_segment)->exchangeSubmodelPointers(replaceWith);

		for(auto& chainlink : m_chain)
			chainlink.submodel = replaceWith.getSubmodel(chainlink.submodel);
	};
	
	std::shared_ptr<ModelAnimationSegment> ModelAnimationSegmentIK::parser(ModelAnimationParseHelper* data) {		
		
		vec3d targetPosition;
		tl::optional<matrix> targetRotation;
		
		required_string("+Target Position:");
		stuff_vec3d(&targetPosition);
		
		if(optional_string("+Target Orientation:")){
			matrix targetRot;
			angles angle;
			stuff_angles_deg_phb(&angle);

			vm_angles_2_matrix(&targetRot, &angle);
			
			targetRotation = targetRot;
		}
		
		required_string("+Time:");
		float time;
		stuff_float(&time);
		
		auto segment = std::shared_ptr<ModelAnimationSegmentIK>(new ModelAnimationSegmentIK(targetPosition, targetRotation));
		auto parallel = std::shared_ptr<ModelAnimationSegmentParallel>(new ModelAnimationSegmentParallel());
		segment->m_segment = parallel;
		
		while(optional_string("$Chain Link:")){
			auto submodel = ModelAnimationParseHelper::parseSubmodel();
			if (!submodel) {
				if (data->parentSubmodel)
					submodel = data->parentSubmodel;
				else
					error_display(1, "IK chain link has no target submodel!");
			}

			tl::optional<angles> acceleration;
			if(optional_string("+Acceleration:")){
				angles accel;
				stuff_angles_deg_phb(&accel);
				acceleration = accel;
			}
			
			std::shared_ptr<ik_constraint> constraint;
			if(optional_string("+Constraint:")){
				int type = required_string_one_of(2, "Window", "Hinge");
				switch(type){
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
			}
			else
				constraint = std::shared_ptr<ik_constraint>(new ik_constraint());
			
			auto rotation = std::shared_ptr<ModelAnimationSegmentRotation>(new ModelAnimationSegmentRotation(submodel, tl::optional<angles>({0,0,0}), tl::optional<angles>(), time, acceleration, ModelAnimationCoordinateRelation::ABSOLUTE_COORDS));
			parallel->addSegment(rotation);
			segment->m_chain.push_back({submodel, constraint, rotation});
		}
		
		return segment;
	}
}
