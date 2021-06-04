#include "model/modelanimation_segments.h"

namespace animation {

	void ModelAnimationSegmentSerial::recalculate(const submodel_instance* ship_info, const ModelAnimationData<true>& base) {
		ModelAnimationData<true> data = base;
		for (size_t i = 0; i < m_segments.size(); i++) {
			m_segments[i]->recalculate(ship_info, data);
			data.applyDelta(m_segments[i]->calculateAnimation(data, base, m_segments[i]->getDuration()));
		}
	}

	ModelAnimationData<true> ModelAnimationSegmentSerial::calculateAnimation(const ModelAnimationData<>& base, const ModelAnimationData<>& lastState, float time) const {
		ModelAnimationData<true> delta;
		ModelAnimationData<> absoluteState = base;

		size_t animationCnt = 0;
		while (time > 0.0f && animationCnt < m_segments.size()) {
			float timeLocal = time;
			if (timeLocal > m_segments[animationCnt]->getDuration())
				timeLocal = m_segments[animationCnt]->getDuration();
			ModelAnimationData<true> deltaLocal = m_segments[animationCnt]->calculateAnimation(absoluteState, lastState, timeLocal);

			absoluteState.applyDelta(deltaLocal);
			delta.applyDelta(deltaLocal);

			animationCnt++;
			time -= m_segments[animationCnt]->getDuration();
		}

		return delta;
	}

	void ModelAnimationSegmentSerial::executeAnimation(const ModelAnimationData<>& state, float time) const {
		for (size_t i = 0; i < m_segments.size(); i++) {
			if (time < m_segments[i]->getDuration()) {
				m_segments[i]->executeAnimation(state, time);
				return;
			}
			time -= m_segments[i]->getDuration();
		}
	}

	void ModelAnimationSegmentSerial::addSegment(std::unique_ptr<ModelAnimationSegment> segment) {
		m_duration += segment->getDuration();
		m_segments.push_back(std::move(segment));
	}


	void ModelAnimationSegmentParallel::recalculate(const submodel_instance* ship_info, const ModelAnimationData<true>& base) {
		for (size_t i = 0; i < m_segments.size(); i++) {
			m_segments[i]->recalculate(ship_info, base);
		}
	}

	ModelAnimationData<true> ModelAnimationSegmentParallel::calculateAnimation(const ModelAnimationData<>& base, const ModelAnimationData<>& lastState, float time) const {
		ModelAnimationData<true> delta;

		for (size_t i = 0; i < m_segments.size(); i++) {
			float timeLocal = time;
			if (timeLocal > m_segments[i]->getDuration())
				timeLocal = m_segments[i]->getDuration();
			
			ModelAnimationData<true> deltaLocal = m_segments[i]->calculateAnimation(base, lastState, timeLocal);
			delta.applyDelta(deltaLocal);
		}

		return delta;
	}

	void ModelAnimationSegmentParallel::executeAnimation(const ModelAnimationData<>& state, float time) const {
		for (size_t i = 0; i < m_segments.size(); i++) {
			if(time < m_segments[i]->getDuration())
				m_segments[i]->executeAnimation(state, time);
		}
	}

	void ModelAnimationSegmentParallel::addSegment(std::unique_ptr<ModelAnimationSegment> segment) {
		float newDur = segment->getDuration();
		m_duration = newDur > m_duration ? newDur : m_duration;
		m_segments.push_back(std::move(segment));
	}


	ModelAnimationSegmentWait::ModelAnimationSegmentWait(float time) {
		m_duration = time;
	}


	ModelAnimationSegmentRotation::ModelAnimationSegmentRotation(optional<float> time, optional<vec3d> angle, optional<vec3d> velocity, optional<vec3d> acceleration, bool isAngleRelative) :
		m_time(time), m_angle(angle), m_velocity(velocity), m_acceleration(acceleration), m_isAngleRelative(isAngleRelative) { }

	void ModelAnimationSegmentRotation::recalculate(const submodel_instance* ship_info, const ModelAnimationData<true>& base) {
		m_vel = m_angle;
		vm_vec_scale2(&m_vel, 1.0f, m_time);

		/*optional<vec3d> relativeAngleActual;
		m_angle.if_filled([&relativeAngleActual, ship_info, &base, this](const vec3d& angle) -> void {
			if (m_isAngleRelative) {
				relativeAngleActual = angle;
			}
			else {
				float theta, actualAnimationTheta;
				vec3d shipAngle, actualAnimationAngle;
				vec3d target = angle;
				matrix animationRotation = base.orientation.has() ? base.orientation : matrix(IDENTITY_MATRIX);
				vm_matrix_to_rot_axis_and_angle(&Objects[ship_info->parent_objnum].orient, &theta, &shipAngle);
				vm_matrix_to_rot_axis_and_angle(&animationRotation, &actualAnimationTheta, &actualAnimationAngle);
				vm_vec_scale(&shipAngle, theta);
				vm_vec_scale_add2(&shipAngle, &actualAnimationAngle, actualAnimationTheta);
				vm_vec_sub2(&target, &shipAngle);

				relativeAngleActual = target;
			}
		});

		if (m_time.has()) {
			m_duration = m_time;
		}
		else {
			if (m_velocity.has() && relativeAngleActual.has()) {

			}
			else {
				Error("Rotation Animation Segment without defined time must have an angle and a velocity defined");
			}
		}*/
		//TODO: proper calculation of values with all alternatives
	}

	ModelAnimationData<true> ModelAnimationSegmentRotation::calculateAnimation(const ModelAnimationData<>& /*base*/, const ModelAnimationData<>& /*lastState*/, float time) const {
		ModelAnimationData<true> data;
		vec3d pos = m_vel;
		vm_vec_scale(&pos, time);
		data.position = pos;
		return data;
	}
}