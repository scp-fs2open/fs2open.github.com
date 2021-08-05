#include "model/modelanimation_segments.h"

namespace animation {

	void ModelAnimationSegmentSerial::recalculate(const submodel_instance* submodel_instance, const bsp_info* submodel, const ModelAnimationData<>& base) {
		ModelAnimationData<true> data = base;
		for (const auto& segment : m_segments) {
			segment->recalculate(submodel_instance, submodel, data);
			//To properly recalculate, we actually need to fully calculate the previous' segment's final delta
			data.applyDelta(segment->calculateAnimation(data, base, segment->getDuration()));
		}
	}

	ModelAnimationData<true> ModelAnimationSegmentSerial::calculateAnimation(const ModelAnimationData<>& base, const ModelAnimationData<>& lastState, float time) const {
		ModelAnimationData<true> delta;
		ModelAnimationData<> absoluteState = base;

		size_t animationCnt = 0;
		while (time > 0.0f && animationCnt < m_segments.size()) {
			float timeLocal = time;
			//Make sure that each segment actually stops at its end
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
		for (const auto& segment : m_segments) {
			if (time < segment->getDuration()) {
				segment->executeAnimation(state, time);
				return;
			}
			time -= segment->getDuration();
		}
	}

	void ModelAnimationSegmentSerial::addSegment(std::shared_ptr<ModelAnimationSegment> segment) {
		m_duration += segment->getDuration();
		m_segments.push_back(std::move(segment));
	}


	void ModelAnimationSegmentParallel::recalculate(const submodel_instance* submodel_instance, const bsp_info* submodel, const ModelAnimationData<>& base) {
		for (const auto& segment : m_segments) {
			segment->recalculate(submodel_instance, submodel, base);
		}
	}

	ModelAnimationData<true> ModelAnimationSegmentParallel::calculateAnimation(const ModelAnimationData<>& base, const ModelAnimationData<>& lastState, float time) const {
		ModelAnimationData<true> delta;

		for (const auto& segment : m_segments) {
			float timeLocal = time;
			//Make sure that no segment runs over its length
			if (timeLocal > segment->getDuration())
				timeLocal = segment->getDuration();
			
			ModelAnimationData<true> deltaLocal = segment->calculateAnimation(base, lastState, timeLocal);
			delta.applyDelta(deltaLocal);
		}

		return delta;
	}

	void ModelAnimationSegmentParallel::executeAnimation(const ModelAnimationData<>& state, float time) const {
		for (const auto& segment : m_segments) {
			if(time < segment->getDuration())
				segment->executeAnimation(state, time);
		}
	}

	void ModelAnimationSegmentParallel::addSegment(std::shared_ptr<ModelAnimationSegment> segment) {
		float newDur = segment->getDuration();
		//recalculate total duration if necessary
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
			//In Absolute mode we need to undo the previously applied rotation to make sure we actually end up at the target rotation despite having only a delta we output, as opposed to just overwriting the value
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

	void ModelAnimationSegmentSetAngle::recalculate(const submodel_instance* /*submodel_instance*/, const bsp_info* submodel, const ModelAnimationData<>& /*base*/) {
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