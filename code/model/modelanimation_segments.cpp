#include "model/modelanimation_segments.h"

namespace animation {

	ModelAnimationSegment* ModelAnimationSegmentSerial::copy() const {
		ModelAnimationSegmentSerial* newCopy = new ModelAnimationSegmentSerial();
		for (const auto& segment : m_segments) {
			newCopy->m_segments.push_back(std::shared_ptr<ModelAnimationSegment>(segment->copy()));
		}
		return newCopy;
	}

	void ModelAnimationSegmentSerial::recalculate(ModelAnimationSubmodelBuffer& base, polymodel_instance* pmi) {
		float& duration = m_duration[pmi->id];
		duration = 0.0f;

		for (const auto& segment : m_segments) {
			segment->recalculate(base, pmi);

			duration += segment->getDuration(pmi->id);

			//To properly recalculate, we actually need to fully calculate the previous' segment's final delta
			segment->calculateAnimation(base, segment->getDuration(pmi->id), pmi->id);
		}
	}

	void ModelAnimationSegmentSerial::calculateAnimation(ModelAnimationSubmodelBuffer& base, float time, int pmi_id) const {

		size_t animationCnt = 0;
		while (time > 0.0f && animationCnt < m_segments.size()) {
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

	void ModelAnimationSegmentSerial::exchangeSubmodelPointers(const std::map<std::shared_ptr<ModelAnimationSubmodel>, std::shared_ptr<ModelAnimationSubmodel>>& exchangeMap) {
		for (const auto& segment : m_segments)
			segment->exchangeSubmodelPointers(exchangeMap);
	}

	void ModelAnimationSegmentSerial::addSegment(std::shared_ptr<ModelAnimationSegment> segment) {
		m_segments.push_back(std::move(segment));
	}


	ModelAnimationSegment* ModelAnimationSegmentParallel::copy() const {
		ModelAnimationSegmentParallel* newCopy = new ModelAnimationSegmentParallel();
		for (const auto& segment : m_segments) {
			newCopy->m_segments.push_back(std::shared_ptr<ModelAnimationSegment>(segment->copy()));
		}
		return newCopy;
	}

	void ModelAnimationSegmentParallel::recalculate(ModelAnimationSubmodelBuffer& base, polymodel_instance* pmi) {
		float& duration = m_duration[pmi->id];
		duration = 0.0f;

		for (const auto& segment : m_segments) {
			ModelAnimationSubmodelBuffer baseCopy = base;
			segment->recalculate(baseCopy, pmi);

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

	void ModelAnimationSegmentParallel::exchangeSubmodelPointers(const std::map<std::shared_ptr<ModelAnimationSubmodel>, std::shared_ptr<ModelAnimationSubmodel>>& exchangeMap) {
		for (const auto& segment : m_segments)
			segment->exchangeSubmodelPointers(exchangeMap);
	}

	void ModelAnimationSegmentParallel::addSegment(std::shared_ptr<ModelAnimationSegment> segment) {
		m_segments.push_back(std::move(segment));
	}


	ModelAnimationSegmentWait::ModelAnimationSegmentWait(float time) : m_time(time) { }

	ModelAnimationSegment* ModelAnimationSegmentWait::copy() const {
		return new ModelAnimationSegmentWait(*this);
	}
	
	void ModelAnimationSegmentWait::recalculate(ModelAnimationSubmodelBuffer& /*base*/, polymodel_instance* pmi) {
		m_duration[pmi->id] = m_time;
	};


	ModelAnimationSegmentSetPHB::ModelAnimationSegmentSetPHB(std::shared_ptr<ModelAnimationSubmodel> submodel, const angles& angle, bool isAngleRelative) :
		m_submodel(submodel), m_targetAngle(angle), m_isAngleRelative(isAngleRelative) { }

	ModelAnimationSegment* ModelAnimationSegmentSetPHB::copy() const {
		return new ModelAnimationSegmentSetPHB(*this);
	}

	void ModelAnimationSegmentSetPHB::recalculate(ModelAnimationSubmodelBuffer& base, polymodel_instance* pmi) {
		int pmi_id = pmi->id;
		if (m_isAngleRelative) {
			vm_angles_2_matrix(&m_instances[pmi_id].rot, &m_targetAngle);
		}
		else {
			//In Absolute mode we need to undo the previously applied rotation to make sure we actually end up at the target rotation despite having only a delta we output, as opposed to just overwriting the value
			matrix unrotate, target;
			const ModelAnimationData<>& submodel = base[m_submodel].first;
			base[m_submodel].second = true;

			vm_copy_transpose(&unrotate, &submodel.orientation);
			vm_angles_2_matrix(&target, &m_targetAngle);
			vm_matrix_x_matrix(&m_instances[pmi_id].rot, &target, &unrotate);
		}

		m_duration[pmi_id] = 0.0f;
	}

	void ModelAnimationSegmentSetPHB::calculateAnimation(ModelAnimationSubmodelBuffer& base, float /*time*/, int pmi_id) const {
		ModelAnimationData<true> data;
		data.orientation = m_instances.at(pmi_id).rot;

		base[m_submodel].first.applyDelta(data);
	}

	void ModelAnimationSegmentSetPHB::exchangeSubmodelPointers(const std::map<std::shared_ptr<ModelAnimationSubmodel>, std::shared_ptr<ModelAnimationSubmodel>>& exchangeMap) {
		m_submodel = exchangeMap.at(m_submodel);
	}


	ModelAnimationSegmentSetAngle::ModelAnimationSegmentSetAngle(std::shared_ptr<ModelAnimationSubmodel> submodel, float angle) :
		m_submodel(submodel), m_angle(angle) { }

	ModelAnimationSegment* ModelAnimationSegmentSetAngle::copy() const {
		return new ModelAnimationSegmentSetAngle(*this);
	}

	void ModelAnimationSegmentSetAngle::recalculate(ModelAnimationSubmodelBuffer& /*base*/, polymodel_instance* pmi) {
		angles angs = vmd_zero_angles;
		auto submodel_info = m_submodel->findSubmodel(pmi).second;
		switch (submodel_info->movement_axis_id)
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
			vm_quaternion_rotate(&m_rot, m_angle, &submodel_info->movement_axis);
			break;
		}

		m_duration[pmi->id] = 0.0f;
	}

	void ModelAnimationSegmentSetAngle::calculateAnimation(ModelAnimationSubmodelBuffer& base, float /*time*/, int /*pmi_id*/) const {
		ModelAnimationData<true> data;
		data.orientation = m_rot;
		
		base[m_submodel].first.applyDelta(data);
		base[m_submodel].second = true;
	}

	void ModelAnimationSegmentSetAngle::exchangeSubmodelPointers(const std::map<std::shared_ptr<ModelAnimationSubmodel>, std::shared_ptr<ModelAnimationSubmodel>>& exchangeMap) {
		m_submodel = exchangeMap.at(m_submodel);
	}

	constexpr float angles::*pbh[] = { &angles::p, &angles::b, &angles::h };

	ModelAnimationSegmentRotation::ModelAnimationSegmentRotation(std::shared_ptr<ModelAnimationSubmodel> submodel, optional<angles> targetAngle, optional<angles> velocity, optional<float> time, optional<angles> acceleration, bool isAbsolute) :
		m_submodel(submodel), m_targetAngle(targetAngle), m_velocity(velocity), m_time(time), m_acceleration(acceleration), m_isAbsolute(isAbsolute) { }

	ModelAnimationSegment* ModelAnimationSegmentRotation::copy() const {
		return new ModelAnimationSegmentRotation(*this);
	}

	void ModelAnimationSegmentRotation::recalculate(ModelAnimationSubmodelBuffer& base, polymodel_instance* pmi) {
		Assertion(!(m_targetAngle.has() ^ m_velocity.has() ^ m_time.has()), "Tried to run over- or underdefined rotation. Define exactly two out of 'time', 'velocity', and 'angle'!");

		instance_data& instanceData = m_instances[pmi->id];
		auto submodel_info = m_submodel->findSubmodel(pmi).second;

		if (m_targetAngle.has()) { //If we have an angle specified, use it.
			if (m_isAbsolute) {
				const ModelAnimationData<>& submodel = base[m_submodel].first;
				base[m_submodel].second = true;

				angles absoluteOffset{ 0,0,0 };
				vm_extract_angles_matrix_alternate(&absoluteOffset, &submodel.orientation);
				const angles& targetAngle = m_targetAngle;
				for(float angles::* i : pbh)
					instanceData.m_actualTarget.*i = targetAngle.*i - absoluteOffset.*i;
			}
			else
				instanceData.m_actualTarget = m_targetAngle;
		}
		else { //If we don't have an angle specified, calculate it. This implies we must have velocity and time.
			const angles& v = m_velocity;
			const float& t = m_time;

			if (m_acceleration.has()) { //Consider acceleration to calculate the angle
				//Let the following equations define our accelerated and braked movement, under the assumption that 2 * ta <= t.
				//d : distance, v : max velocity, a : acceleration, t : total time, ta : time spent accelerating (and breaking)
				//v = a * ta
				//d = v * (t - 2 * ta) + 1/2 * 2 * a * ta^2
				//this simplifies to d = (v(a * t - v))/a and ta = v / a
				//if 2 * ta <= t does not hold, it's just d = 1/2 * 2 * a * (t/2)^2 -> this implies that the acceleration is too small to reach the target velocity within the specified time.
				angles a = m_acceleration;
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

		if (m_velocity.has()) { //If we have velocity specified, use it.
			instanceData.m_actualVelocity = m_velocity;
		}
		else { //If we don't have velocity specified, calculate it. This implies we must have an angle and time.
			const float& t = m_time;
			const angles& d = instanceData.m_actualTarget;

			if (m_acceleration.has()) { //Consider acceleration to calculate the velocity
				//Assume equations from calc angles case, but solve for ta and v now, under the assumption that these roots have a real solution.
				//v = 1/2*(|a|*t-sqrt(|a|)*sqrt(|a|*t^2-4*|d|))*sign(d) and ta = 1/2*(t-(sqrt(|a|*t^2-4*|d|)/sqrt(|a|)))
				//If the roots don't have a real solution, it's v = a * t/2, and ta = 1/2*t -> this implies that the acceleration is too small to reach the target distance within the specified time.

				angles a = m_acceleration;
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

		if (m_time.has()) { //If we have time specified, use it.
			const float& time = m_time;
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

					if (m_acceleration.has()) { //Consider acceleration to calculate the time
						//Assume equations from calc angles case, but solve for ta and t now, with the resulting ta <= t / 2.
						//t = v/a+d/v and ta = v/a
						//If thus d/v < v/a, it's t = 2*sqrt(d/a), and ta = 1/2*t -> this implies that the acceleration is too small to reach the target velocity within the specified distance.
						float a = copysignf(((angles)m_acceleration).*i, d.*i);
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
			
			if (m_acceleration.has()) {
				instanceData.m_actualAccel = actualAccel;
				instanceData.m_accelTime = accelTime;
			}

			instanceData.m_actualTime = actualTime;
		}
	}

	void ModelAnimationSegmentRotation::calculateAnimation(ModelAnimationSubmodelBuffer& base, float time, int pmi_id) const {
		const instance_data& instanceData = m_instances.at(pmi_id);

		angles currentRot{ 0,0,0 };

		if (instanceData.m_actualAccel.has()) {
			const angles& a = instanceData.m_actualAccel;
			const angles& at = instanceData.m_accelTime;
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

		base[m_submodel].first.applyDelta(delta);
		base[m_submodel].second = true;
	}

	void ModelAnimationSegmentRotation::exchangeSubmodelPointers(const std::map<std::shared_ptr<ModelAnimationSubmodel>, std::shared_ptr<ModelAnimationSubmodel>>& exchangeMap) {
		m_submodel = exchangeMap.at(m_submodel);
	}


	ModelAnimationSegmentSoundDuring::ModelAnimationSegmentSoundDuring(std::shared_ptr<ModelAnimationSegment> segment, gamesnd_id start, gamesnd_id end, gamesnd_id during, bool flipIfReversed) :
		m_segment(std::move(segment)), m_start(start), m_end(end), m_during(during), m_flipIfReversed(flipIfReversed) { }

	ModelAnimationSegment* ModelAnimationSegmentSoundDuring::copy() const {
		auto newCopy = new ModelAnimationSegmentSoundDuring(*this);
		newCopy->m_segment = std::shared_ptr<ModelAnimationSegment>(newCopy->m_segment->copy());
		return newCopy;
	}

	void ModelAnimationSegmentSoundDuring::recalculate(ModelAnimationSubmodelBuffer& base, polymodel_instance* pmi) {
		m_segment->recalculate(base, pmi);
		m_duration[pmi->id] = m_segment->getDuration(pmi->id);
	}

	void ModelAnimationSegmentSoundDuring::calculateAnimation(ModelAnimationSubmodelBuffer& base, float time, int pmi_id) const {
		m_segment->calculateAnimation(base, time, pmi_id);
	}

	void ModelAnimationSegmentSoundDuring::executeAnimation(const ModelAnimationSubmodelBuffer& state, float timeboundLower, float timeboundUpper, ModelAnimationDirection direction, int pmi_id) {
		if (timeboundLower <= 0.0f && 0.0f <= timeboundUpper) {
			if (!m_flipIfReversed || direction == ModelAnimationDirection::FWD)
				playStartSnd(pmi_id);
			else
				playEndSnd(pmi_id);
		}

		if (0.0f < timeboundLower && timeboundUpper < m_duration.at(pmi_id)) {
			if (m_during.isValid() && (!m_instances[pmi_id].currentlyPlaying.isValid() || !snd_is_playing(m_instances[pmi_id].currentlyPlaying)))
				m_instances[pmi_id].currentlyPlaying = snd_play_looping(gamesnd_get_game_sound(m_during));
		}

		if (timeboundLower <= m_duration.at(pmi_id) && m_duration.at(pmi_id) <= timeboundUpper) {
			if (!m_flipIfReversed || direction == ModelAnimationDirection::FWD)
				playEndSnd(pmi_id);
			else
				playStartSnd(pmi_id);
		}
		m_segment->executeAnimation(state, timeboundLower, timeboundUpper, direction, pmi_id);
	}

	void ModelAnimationSegmentSoundDuring::exchangeSubmodelPointers(const std::map<std::shared_ptr<ModelAnimationSubmodel>, std::shared_ptr<ModelAnimationSubmodel>>& exchangeMap) {
		m_segment->exchangeSubmodelPointers(exchangeMap);
	}

	void ModelAnimationSegmentSoundDuring::playStartSnd(int pmi_id) {
		if(m_start.isValid())
			m_instances[pmi_id].currentlyPlaying = snd_play(gamesnd_get_game_sound(m_start));
	}
	void ModelAnimationSegmentSoundDuring::playEndSnd(int pmi_id) {
		if (snd_is_playing(m_instances[pmi_id].currentlyPlaying))
			snd_stop(m_instances[pmi_id].currentlyPlaying);

		if (m_end.isValid()) 
			m_instances[pmi_id].currentlyPlaying = snd_play(gamesnd_get_game_sound(m_end));
	}
}