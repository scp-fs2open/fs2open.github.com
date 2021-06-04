#pragma once

#include "model/modelanimation.h"

namespace animation {

	class ModelAnimationSegmentSerial : public ModelAnimationSegment {
		std::vector<std::unique_ptr<ModelAnimationSegment>> m_segments;

		void recalculate(const submodel_instance* ship_info, const ModelAnimationData<true>& base) override;
		ModelAnimationData<true> calculateAnimation(const ModelAnimationData<>& base, const ModelAnimationData<>& lastState, float time) const override;
		void executeAnimation(const ModelAnimationData<>& state, float time) const override;

	public:
		void addSegment(std::unique_ptr<ModelAnimationSegment> segment);

	};

	class ModelAnimationSegmentParallel : public ModelAnimationSegment {
		std::vector<std::unique_ptr<ModelAnimationSegment>> m_segments;

		void recalculate(const submodel_instance* ship_info, const ModelAnimationData<true>& base) override;
		ModelAnimationData<true> calculateAnimation(const ModelAnimationData<>& base, const ModelAnimationData<>& lastState, float time) const override;
		void executeAnimation(const ModelAnimationData<>& state, float time) const override;

	public:
		void addSegment(std::unique_ptr<ModelAnimationSegment> segment);

	};

	class ModelAnimationSegmentWait : public ModelAnimationSegment {
		void recalculate(const submodel_instance* /*ship_info*/, const ModelAnimationData<true>& /*base*/) override { };
		ModelAnimationData<true> calculateAnimation(const ModelAnimationData<>& /*base*/, const ModelAnimationData<>& /*lastState*/, float /*time*/) const override { return ModelAnimationData<true>(); };
		void executeAnimation(const ModelAnimationData<>& /*state*/, float /*time*/) const override { };

	public:
		ModelAnimationSegmentWait(float time);

	};

	class ModelAnimationSegmentRotation : public ModelAnimationSegment {
		optional<float> m_time;
		optional<vec3d> m_angle;
		optional<vec3d> m_velocity;
		optional<vec3d> m_acceleration;
		bool m_isAngleRelative;

		vec3d m_vel;

		void recalculate(const submodel_instance* ship_info, const ModelAnimationData<true>& base) override;
		ModelAnimationData<true> calculateAnimation(const ModelAnimationData<>& /*base*/, const ModelAnimationData<>& /*lastState*/, float time) const override;
		void executeAnimation(const ModelAnimationData<>& /*state*/, float /*time*/) const override { };

	public:
		ModelAnimationSegmentRotation(optional<float> time, optional<vec3d> angle, optional<vec3d> velocity, optional<vec3d> acceleration, bool isAngleRelative);

	};

}