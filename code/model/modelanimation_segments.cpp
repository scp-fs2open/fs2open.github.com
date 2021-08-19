#include "model/modelanimation_segments.h"

namespace animation {

	void ModelAnimationSegmentSerial::recalculate(const submodel_instance* submodel_instance, const bsp_info* submodel, const ModelAnimationData<>& base, int pmi_id) {
		ModelAnimationData<true> data = base;
		float& duration = m_duration[pmi_id];
		duration = 0.0f;

		for (const auto& segment : m_segments) {
			segment->recalculate(submodel_instance, submodel, data, pmi_id);

			duration += segment->getDuration(pmi_id);

			//To properly recalculate, we actually need to fully calculate the previous' segment's final delta
			data.applyDelta(segment->calculateAnimation(data, segment->getDuration(pmi_id), pmi_id));
		}
	}

	ModelAnimationData<true> ModelAnimationSegmentSerial::calculateAnimation(const ModelAnimationData<>& base, float time, int pmi_id) const {
		ModelAnimationData<true> delta;
		ModelAnimationData<> absoluteState = base;

		size_t animationCnt = 0;
		while (time > 0.0f && animationCnt < m_segments.size()) {
			float timeLocal = time;
			//Make sure that each segment actually stops at its end
			if (timeLocal > m_segments[animationCnt]->getDuration(pmi_id))
				timeLocal = m_segments[animationCnt]->getDuration(pmi_id);
			ModelAnimationData<true> deltaLocal = m_segments[animationCnt]->calculateAnimation(absoluteState, timeLocal, pmi_id);

			absoluteState.applyDelta(deltaLocal);
			delta.applyDelta(deltaLocal);

			time -= m_segments[animationCnt]->getDuration(pmi_id);
			animationCnt++;
		}

		return delta;
	}

	void ModelAnimationSegmentSerial::executeAnimation(const ModelAnimationData<>& state, float time, int pmi_id) const {
		for (const auto& segment : m_segments) {
			if (time < segment->getDuration(pmi_id)) {
				segment->executeAnimation(state, time, pmi_id);
				return;
			}
			time -= segment->getDuration(pmi_id);
		}
	}

	void ModelAnimationSegmentSerial::addSegment(std::shared_ptr<ModelAnimationSegment> segment) {
		m_segments.push_back(std::move(segment));
	}


	void ModelAnimationSegmentParallel::recalculate(const submodel_instance* submodel_instance, const bsp_info* submodel, const ModelAnimationData<>& base, int pmi_id) {
		float& duration = m_duration[pmi_id];
		duration = 0.0f;

		for (const auto& segment : m_segments) {
			segment->recalculate(submodel_instance, submodel, base, pmi_id);

			//recalculate total duration if necessary
			float newDur = segment->getDuration(pmi_id);
			duration = newDur > duration ? newDur : duration;
		}

	}

	ModelAnimationData<true> ModelAnimationSegmentParallel::calculateAnimation(const ModelAnimationData<>& base, float time, int pmi_id) const {
		ModelAnimationData<true> delta;

		for (const auto& segment : m_segments) {
			float timeLocal = time;
			//Make sure that no segment runs over its length
			if (timeLocal > segment->getDuration(pmi_id))
				timeLocal = segment->getDuration(pmi_id);
			
			ModelAnimationData<true> deltaLocal = segment->calculateAnimation(base, timeLocal, pmi_id);
			delta.applyDelta(deltaLocal);
		}

		return delta;
	}

	void ModelAnimationSegmentParallel::executeAnimation(const ModelAnimationData<>& state, float time, int pmi_id) const {
		for (const auto& segment : m_segments) {
			if(time < segment->getDuration(pmi_id))
				segment->executeAnimation(state, time, pmi_id);
		}
	}

	void ModelAnimationSegmentParallel::addSegment(std::shared_ptr<ModelAnimationSegment> segment) {
		m_segments.push_back(std::move(segment));
	}


	ModelAnimationSegmentWait::ModelAnimationSegmentWait(float time) : m_time(time) { }

	void ModelAnimationSegmentWait::recalculate(const submodel_instance* /*submodel_instance*/, const bsp_info* /*submodel*/, const ModelAnimationData<>& /*base*/, int pmi_id) { 
		m_duration[pmi_id] = m_time;
	};


	ModelAnimationSegmentSetPHB::ModelAnimationSegmentSetPHB(const angles& angle, bool isAngleRelative) :
		m_targetAngle(angle), m_isAngleRelative(isAngleRelative) { }

	void ModelAnimationSegmentSetPHB::recalculate(const submodel_instance* /*submodel_instance*/, const bsp_info* /*submodel*/, const ModelAnimationData<>& base, int /*pmi_id*/) {
		if (m_isAngleRelative) {
			vm_angles_2_matrix(&m_rot, &m_targetAngle);
		}
		else {
			//In Absolute mode we need to undo the previously applied rotation to make sure we actually end up at the target rotation despite having only a delta we output, as opposed to just overwriting the value
			matrix unrotate, target;
			vm_copy_transpose(&unrotate, &base.orientation);
			vm_angles_2_matrix(&target, &m_targetAngle);
			vm_matrix_x_matrix(&m_rot, &target, &unrotate);
		}
	}

	ModelAnimationData<true> ModelAnimationSegmentSetPHB::calculateAnimation(const ModelAnimationData<>& /*base*/, float /*time*/, int /*pmi_id*/) const {
		ModelAnimationData<true> data;
		data.orientation = m_rot;
		return data;
	}


	ModelAnimationSegmentSetAngle::ModelAnimationSegmentSetAngle(float angle) :
		m_angle(angle) { }

	void ModelAnimationSegmentSetAngle::recalculate(const submodel_instance* /*submodel_instance*/, const bsp_info* submodel, const ModelAnimationData<>& /*base*/, int /*pmi_id*/) {
		angles angs = vmd_zero_angles;

		switch (submodel->movement_axis_id)
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
			vm_quaternion_rotate(&m_rot, m_angle, &submodel->movement_axis);
			break;
		}
	}

	ModelAnimationData<true> ModelAnimationSegmentSetAngle::calculateAnimation(const ModelAnimationData<>& /*base*/, float /*time*/, int /*pmi_id*/) const {
		ModelAnimationData<true> data;
		data.orientation = m_rot;
		return data;
	}


	constexpr float angles::*pbh[] = { &angles::p, &angles::b, &angles::h };

	ModelAnimationSegmentRotation::ModelAnimationSegmentRotation(optional<angles> targetAngle, optional<angles> velocity, optional<float> time, optional<angles> acceleration, bool isAbsolute) :
		m_targetAngle(targetAngle), m_velocity(velocity), m_time(time), m_acceleration(acceleration), m_isAbsolute(isAbsolute) { }

	void ModelAnimationSegmentRotation::recalculate(const submodel_instance* /*submodel_instance*/, const bsp_info* submodel, const ModelAnimationData<>& base, int pmi_id) {
		Assertion(!(m_targetAngle.has() ^ m_velocity.has() ^ m_time.has()), "Tried to run over- or underdefined rotation. Define exactly two out of 'time', 'velocity', and 'angle'!");

		instance_data& instanceData = m_instances[pmi_id];

		if (m_targetAngle.has()) { //If we have an angle specified, use it.
			if (m_isAbsolute) {
				angles absoluteOffset{ 0,0,0 };
				vm_extract_angles_matrix_alternate(&absoluteOffset, &base.orientation);
				const angles& targetAngle = m_targetAngle;
				for(int i = 0; i < 3; i++)
					instanceData.m_actualTarget.*pbh[i] = targetAngle.*pbh[i] - absoluteOffset.*pbh[i];
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

				for (int i = 0; i < 3; i++) {
					a.*pbh[i] = copysignf(a.*pbh[i], v.*pbh[i]);
					at.*pbh[i] = fmaxf(v.*pbh[i] / a.*pbh[i], t / 2.0f);
				}
				instanceData.m_actualAccel = a;
				instanceData.m_accelTime = at;

				for (int i = 0; i < 3; i++)
					instanceData.m_actualTarget.*pbh[i] = 2.0f * v.*pbh[i] / a.*pbh[i] <= t ? (v.*pbh[i] * (a.*pbh[i] * t - v.*pbh[i])) / a.*pbh[i] : a.*pbh[i] * (t * t / 4.0f);

			}
			else { //Don't consider acceleration, assume instant velocity.
				for (int i = 0; i < 3; i++)
					instanceData.m_actualTarget.*pbh[i] = v.*pbh[i] * t;
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
				for (int i = 0; i < 3; i++)
					a.*pbh[i] = copysignf(a.*pbh[i], d.*pbh[i]);
				instanceData.m_actualAccel = a;

				angles at{ 0,0,0 };

				for (int i = 0; i < 3; i++) {
					float a_abs = fabsf(a.*pbh[i]);
					float radicant = a_abs * t * t - 4 * fabsf(d.*pbh[i]);
					if (radicant >= 0) {
						instanceData.m_actualVelocity.*pbh[i] = copysignf(0.5f * (a_abs * t - sqrtf(a_abs) * sqrtf(radicant)), d.*pbh[i]);
						at.*pbh[i] = 0.5f * (t - (sqrtf(radicant) / sqrtf(a_abs)));
					}
					else {
						instanceData.m_actualVelocity.*pbh[i] = a.*pbh[i] * t / 2.0f;
						at.*pbh[i] = t / 2.0f;
					}
				}

				instanceData.m_accelTime = at;
			}
			else { //Don't consider acceleration, assume instant velocity.
				for (int i = 0; i < 3; i++)
					instanceData.m_actualVelocity.*pbh[i] = d.*pbh[i] / t;
			}

		}

		if (m_time.has()) { //If we have time specified, use it.
			const float& time = m_time;
			m_duration[pmi_id] = time;

			angles actualTime{ 0,0,0 };
			for (int i = 0; i < 3; i++)
				actualTime.*pbh[i] = time;

			instanceData.m_actualTime = actualTime;

			if (time <= 0.0f) {
				Error(LOCATION, "Tried to rotate submodel %s in %.2f seconds. Rotation time must be positive and nonzero!", submodel->name, time);
			}
		}
		else { //Calc time
			float& duration = m_duration[pmi_id] = 0.0f;

			angles& v = instanceData.m_actualVelocity;
			const angles& d = instanceData.m_actualTarget;

			angles actualAccel{ 0,0,0 };
			angles accelTime{ 0,0,0 };
			angles actualTime{ 0,0,0 };

			for (int i = 0; i < 3; i++) {
				if (d.*pbh[i] != 0.0f) {
					if (v.*pbh[i] == 0.0f) {
						Warning(LOCATION, "Tried to rotate submodel %s by %.2f in axis %d, but velocity was 0! Rotating with velocity 1...", submodel->name, d.*pbh[i], i);
						v.*pbh[i] = 1;
					}

					v.*pbh[i] = copysignf(v.*pbh[i], d.*pbh[i]);

					float durationAxis = 0.0f;

					if (m_acceleration.has()) { //Consider acceleration to calculate the time
						//Assume equations from calc angles case, but solve for ta and t now, with the resulting ta <= t / 2.
						//t = v/a+d/v and ta = v/a
						//If thus d/v < v/a, it's t = 2*sqrt(d/a), and ta = 1/2*t -> this implies that the acceleration is too small to reach the target velocity within the specified distance.
						float a = copysignf(((angles)m_acceleration).*pbh[i], d.*pbh[i]);
						actualAccel.*pbh[i] = a;

						float va = v.*pbh[i] / a;
						float dv = d.*pbh[i] / v.*pbh[i];

						if (dv >= va) {
							durationAxis = va + dv;
							accelTime.*pbh[i] = va;
						}
						else {
							durationAxis = 2.0f * sqrtf(d.*pbh[i] / a);
							accelTime.*pbh[i] = durationAxis / 2.0f;
						}
					}
					else {
						durationAxis = d.*pbh[i] / v.*pbh[i];
					}

					duration = duration < durationAxis ? durationAxis : duration;
					actualTime.*pbh[i] = durationAxis;
				}
			}
			
			if (m_acceleration.has()) {
				instanceData.m_actualAccel = actualAccel;
				instanceData.m_accelTime = accelTime;
			}

			instanceData.m_actualTime = actualTime;
		}
	}

	ModelAnimationData<true> ModelAnimationSegmentRotation::calculateAnimation(const ModelAnimationData<>& /*base*/, float time, int pmi_id) const {
		const instance_data& instanceData = m_instances.at(pmi_id);

		angles currentRot{ 0,0,0 };

		if (instanceData.m_actualAccel.has()) {
			const angles& a = instanceData.m_actualAccel;
			const angles& at = instanceData.m_accelTime;
			const angles& v = instanceData.m_actualVelocity;
			const angles& t = instanceData.m_actualTime;

			for (int i = 0; i < 3; i++) {
				float acceltime1 = fminf(time, at.*pbh[i]);
				currentRot.*pbh[i] = 0.5f * a.*pbh[i] * acceltime1 * acceltime1;
				
				float lineartime = fminf(time - at.*pbh[i], t.*pbh[i] - 2.0f * at.*pbh[i]);
				if (lineartime > 0) {
					currentRot.*pbh[i] += v.*pbh[i] * lineartime;
				}

				float acceltime2 = fminf(time - (t.*pbh[i] - at.*pbh[i]), at.*pbh[i]);
				if (acceltime2 > 0) {
					//Cap this to 0, as it could get negative if it rotates "longer than its duration" (which happens when a different axis takes longer to rotate)
					currentRot.*pbh[i] += fminf(at.*pbh[i] * a.*pbh[i] * acceltime2 - 0.5f * a.*pbh[i] * acceltime2 * acceltime2, 0.0f);
				}
			}
		}
		else {
			for (int i = 0; i < 3; i++) // Linear Rotation
				currentRot.*pbh[i] = instanceData.m_actualVelocity.*pbh[i] * time;
		}

		for (int i = 0; i < 3; i++) { // Clamp rotation to actual target
			if (instanceData.m_actualTarget.*pbh[i] < 0) {
				currentRot.*pbh[i] = fmaxf(instanceData.m_actualTarget.*pbh[i], currentRot.*pbh[i]);
			}
			else {
				currentRot.*pbh[i] = fminf(instanceData.m_actualTarget.*pbh[i], currentRot.*pbh[i]);
			}
		}

		matrix orient;
		vm_angles_2_matrix(&orient, &currentRot);

		ModelAnimationData<true> delta;
		delta.orientation = orient;

		return delta;
	}
}