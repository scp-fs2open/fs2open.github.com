#pragma once

#include "model/modelanimation.h"

namespace animation {

	//This segment handles multiple generic segments chained after one another
	class ModelAnimationSegmentSerial : public ModelAnimationSegment {
		std::vector<std::shared_ptr<ModelAnimationSegment>> m_segments;

		void recalculate(const submodel_instance* submodel_instance, const bsp_info* submodel, const ModelAnimationData<>& base) override;
		ModelAnimationData<true> calculateAnimation(const ModelAnimationData<>& base, const ModelAnimationData<>& lastState, float time) const override;
		void executeAnimation(const ModelAnimationData<>& state, float time) const override;

	public:
		void addSegment(std::shared_ptr<ModelAnimationSegment> segment);

	};

	//This segment handles multiple generic segments executing in parallel
	class ModelAnimationSegmentParallel : public ModelAnimationSegment {
		std::vector<std::shared_ptr<ModelAnimationSegment>> m_segments;

		void recalculate(const submodel_instance* submodel_instance, const bsp_info* submodel, const ModelAnimationData<>& base) override;
		ModelAnimationData<true> calculateAnimation(const ModelAnimationData<>& base, const ModelAnimationData<>& lastState, float time) const override;
		void executeAnimation(const ModelAnimationData<>& state, float time) const override;

	public:
		void addSegment(std::shared_ptr<ModelAnimationSegment> segment);

	};

	//This segment does nothing but serve as a placeholder taking up time, used primarily in serial segments
	class ModelAnimationSegmentWait : public ModelAnimationSegment {
		void recalculate(const submodel_instance* /*submodel_instance*/, const bsp_info* /*submodel*/, const ModelAnimationData<>& /*base*/) override { };
		ModelAnimationData<true> calculateAnimation(const ModelAnimationData<>& /*base*/, const ModelAnimationData<>& /*lastState*/, float /*time*/) const override { return {}; };
		void executeAnimation(const ModelAnimationData<>& /*state*/, float /*time*/) const override { };

	public:
		ModelAnimationSegmentWait(float time);

	};

	//This segment changes or sets a submodels orientation to a defined Pitch Heading and Bank angle.
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

	//This segment rotates a submodels orientation by a certain amount around its defined rotation axis
	class ModelAnimationSegmentSetAngle : public ModelAnimationSegment {
		float m_angle;
		matrix m_rot;

		void recalculate(const submodel_instance* /*submodel_instance*/, const bsp_info* submodel, const ModelAnimationData<>& /*base*/) override;
		ModelAnimationData<true> calculateAnimation(const ModelAnimationData<>& /*base*/, const ModelAnimationData<>& /*lastState*/, float /*time*/) const override;
		void executeAnimation(const ModelAnimationData<>& /*state*/, float /*time*/) const override { };

	public:
		ModelAnimationSegmentSetAngle(float angle);

	};

}