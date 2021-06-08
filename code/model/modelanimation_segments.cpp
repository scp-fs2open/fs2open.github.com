#include "model/modelanimation_segments.h"

namespace animation {

	void ModelAnimationSegmentSerial::recalculate(const submodel_instance* submodel_instance, const bsp_info* submodel, const ModelAnimationData<>& base) {
		ModelAnimationData<true> data = base;
		for (size_t i = 0; i < m_segments.size(); i++) {
			m_segments[i]->recalculate(submodel_instance, submodel, data);
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


	void ModelAnimationSegmentParallel::recalculate(const submodel_instance* submodel_instance, const bsp_info* submodel, const ModelAnimationData<>& base) {
		for (size_t i = 0; i < m_segments.size(); i++) {
			m_segments[i]->recalculate(submodel_instance, submodel, base);
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


	ModelAnimationSegmentSetPHB::ModelAnimationSegmentSetPHB(const angles& angle, bool isAngleRelative) :
		m_targetAngle(angle), m_isAngleRelative(isAngleRelative) { }

	void ModelAnimationSegmentSetPHB::recalculate(const submodel_instance* /*submodel_instance*/, const bsp_info* /*submodel*/, const ModelAnimationData<>& base) {
		if (m_isAngleRelative) {
			vm_angles_2_matrix(&m_rot, &m_targetAngle);
		}
		else {
			matrix unrotate, target;
			vm_copy_transpose(&unrotate, &base.orientation);
			vm_angles_2_matrix(&target, &m_targetAngle);
			vm_matrix_x_matrix(&m_rot, &target, &unrotate);
		}
	}

	ModelAnimationData<true> ModelAnimationSegmentSetPHB::calculateAnimation(const ModelAnimationData<>& /*base*/, const ModelAnimationData<>& /*lastState*/, float /*time*/) const {
		ModelAnimationData<true> data;
		data.orientation = m_rot;
		return data;
	}


	ModelAnimationSegmentSetAngle::ModelAnimationSegmentSetAngle(float angle) :
		m_angle(angle) { }

	void ModelAnimationSegmentSetAngle::recalculate(const submodel_instance* /*submodel_instance*/, const bsp_info* submodel, const ModelAnimationData<>& base) {
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

	ModelAnimationData<true> ModelAnimationSegmentSetAngle::calculateAnimation(const ModelAnimationData<>& /*base*/, const ModelAnimationData<>& /*lastState*/, float /*time*/) const {
		ModelAnimationData<true> data;
		data.orientation = m_rot;
		return data;
	}
}