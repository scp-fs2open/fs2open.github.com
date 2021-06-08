#pragma once

#include "model/modelanimation.h"

namespace animation {

	class ModelAnimationSegmentSerial : public ModelAnimationSegment {
		std::vector<std::unique_ptr<ModelAnimationSegment>> m_segments;

		void recalculate(const submodel_instance* submodel_instance, const bsp_info* submodel, const ModelAnimationData<>& base) override;
		ModelAnimationData<true> calculateAnimation(const ModelAnimationData<>& base, const ModelAnimationData<>& lastState, float time) const override;
		void executeAnimation(const ModelAnimationData<>& state, float time) const override;

	public:
		void addSegment(std::unique_ptr<ModelAnimationSegment> segment);

	};

	class ModelAnimationSegmentParallel : public ModelAnimationSegment {
		std::vector<std::unique_ptr<ModelAnimationSegment>> m_segments;

		void recalculate(const submodel_instance* submodel_instance, const bsp_info* submodel, const ModelAnimationData<>& base) override;
		ModelAnimationData<true> calculateAnimation(const ModelAnimationData<>& base, const ModelAnimationData<>& lastState, float time) const override;
		void executeAnimation(const ModelAnimationData<>& state, float time) const override;

	public:
		void addSegment(std::unique_ptr<ModelAnimationSegment> segment);

	};

	class ModelAnimationSegmentWait : public ModelAnimationSegment {
		void recalculate(const submodel_instance* /*submodel_instance*/, const bsp_info* /*submodel*/, const ModelAnimationData<>& /*base*/) override { };
		ModelAnimationData<true> calculateAnimation(const ModelAnimationData<>& /*base*/, const ModelAnimationData<>& /*lastState*/, float /*time*/) const override { return ModelAnimationData<true>(); };
		void executeAnimation(const ModelAnimationData<>& /*state*/, float /*time*/) const override { };

	public:
		ModelAnimationSegmentWait(float time);

	};

	class ModelAnimationSegmentSetPHB : public ModelAnimationSegment {
		angles m_targetAngle;
		bool m_isAngleRelative;
		matrix m_rot;

		void recalculate(const submodel_instance* /*submodel_instance*/, const bsp_info* /*submodel*/, const ModelAnimationData<>& base) override;
		ModelAnimationData<true> calculateAnimation(const ModelAnimationData<>& /*base*/, const ModelAnimationData<>& /*lastState*/, float /*time*/) const override;
		void executeAnimation(const ModelAnimationData<>& /*state*/, float /*time*/) const override { };

	public:
		ModelAnimationSegmentSetPHB(const angles& angle, bool isAngleRelative);

	};

	class ModelAnimationSegmentSetAngle : public ModelAnimationSegment {
		float m_angle;
		matrix m_rot;

		void recalculate(const submodel_instance* /*submodel_instance*/, const bsp_info* submodel, const ModelAnimationData<>& base) override;
		ModelAnimationData<true> calculateAnimation(const ModelAnimationData<>& /*base*/, const ModelAnimationData<>& /*lastState*/, float /*time*/) const override;
		void executeAnimation(const ModelAnimationData<>& /*state*/, float /*time*/) const override { };

	public:
		ModelAnimationSegmentSetAngle(float angle);

	};

}