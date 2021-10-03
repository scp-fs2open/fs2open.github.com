#pragma once

#include "model/modelanimation.h"

namespace animation {

	//This segment handles multiple generic segments chained after one another
	class ModelAnimationSegmentSerial : public ModelAnimationSegment {
		std::vector<std::shared_ptr<ModelAnimationSegment>> m_segments;

		void recalculate(const submodel_instance* submodel_instance, const bsp_info* submodel, const ModelAnimationData<>& base, int pmi_id) override;
		ModelAnimationData<true> calculateAnimation(const ModelAnimationData<>& base, float time, int pmi_id) const override;
		void executeAnimation(const ModelAnimationData<>& state, float timeboundLower, float timeboundUpper, ModelAnimationDirection direction, int pmi_id) override;

	public:
		void addSegment(std::shared_ptr<ModelAnimationSegment> segment);

	};

	//This segment handles multiple generic segments executing in parallel
	class ModelAnimationSegmentParallel : public ModelAnimationSegment {
		std::vector<std::shared_ptr<ModelAnimationSegment>> m_segments;

		void recalculate(const submodel_instance* submodel_instance, const bsp_info* submodel, const ModelAnimationData<>& base, int pmi_id) override;
		ModelAnimationData<true> calculateAnimation(const ModelAnimationData<>& base, float time, int pmi_id) const override;
		void executeAnimation(const ModelAnimationData<>& state, float timeboundLower, float timeboundUpper, ModelAnimationDirection direction, int pmi_id) override;

	public:
		void addSegment(std::shared_ptr<ModelAnimationSegment> segment);

	};

	//This segment does nothing but serve as a placeholder taking up time, used primarily in serial segments
	class ModelAnimationSegmentWait : public ModelAnimationSegment {
		float m_time;

		void recalculate(const submodel_instance* /*submodel_instance*/, const bsp_info* /*submodel*/, const ModelAnimationData<>& /*base*/, int pmi_id) override;
		ModelAnimationData<true> calculateAnimation(const ModelAnimationData<>& /*base*/, float /*time*/, int /*pmi_id*/) const override { return {}; };
		void executeAnimation(const ModelAnimationData<>& /*state*/, float /*timeboundLower*/, float /*timeboundUpper*/, ModelAnimationDirection /*direction*/, int /*pmi_id*/) override { };

	public:
		ModelAnimationSegmentWait(float time);

	};

	//This segment changes or sets a submodels orientation to a defined Pitch Heading and Bank angle.
	class ModelAnimationSegmentSetPHB : public ModelAnimationSegment {
		struct instance_data {
			matrix rot;
		};

		//PMI ID -> Instance Data
		std::map<int, instance_data> m_instances;

		angles m_targetAngle;
		bool m_isAngleRelative;

		void recalculate(const submodel_instance* /*submodel_instance*/, const bsp_info* /*submodel*/, const ModelAnimationData<>& base, int pmi_id) override;
		ModelAnimationData<true> calculateAnimation(const ModelAnimationData<>& /*base*/, float /*time*/, int pmi_id) const override;
		void executeAnimation(const ModelAnimationData<>& /*state*/, float /*timeboundLower*/, float /*timeboundUpper*/, ModelAnimationDirection /*direction*/, int /*pmi_id*/) override { };

	public:
		ModelAnimationSegmentSetPHB(const angles& angle, bool isAngleRelative);

	};

	//This segment rotates a submodels orientation by a certain amount around its defined rotation axis
	class ModelAnimationSegmentSetAngle : public ModelAnimationSegment {
		float m_angle;
		matrix m_rot;

		void recalculate(const submodel_instance* /*submodel_instance*/, const bsp_info* submodel, const ModelAnimationData<>& /*base*/, int pmi_id) override;
		ModelAnimationData<true> calculateAnimation(const ModelAnimationData<>& /*base*/, float /*time*/, int /*pmi_id*/) const override;
		void executeAnimation(const ModelAnimationData<>& /*state*/, float /*timeboundLower*/, float /*timeboundUpper*/, ModelAnimationDirection /*direction*/, int /*pmi_id*/) override { };

	public:
		ModelAnimationSegmentSetAngle(float angle);

	};

	//This segment rotates a submodels orientation by a certain amount in PBH
	class ModelAnimationSegmentRotation : public ModelAnimationSegment {
		struct instance_data {
			angles m_actualVelocity;
			angles m_actualTarget; //Usually won't be needed, but if vel + angle is specified, not all angles necessarily end simultaneously.
			angles m_actualTime;
			optional<angles> m_actualAccel;
			optional<angles> m_accelTime;
		};

		//PMI ID -> Instance Data
		std::map<int, instance_data> m_instances;

		optional<angles> m_targetAngle;
		optional<angles> m_velocity;
		optional<float> m_time;
		optional<angles> m_acceleration;
		bool m_isAbsolute;

		void recalculate(const submodel_instance* /*submodel_instance*/, const bsp_info* submodel, const ModelAnimationData<>& base, int pmi_id) override;
		ModelAnimationData<true> calculateAnimation(const ModelAnimationData<>& /*base*/, float time, int pmi_id) const override;
		void executeAnimation(const ModelAnimationData<>& /*state*/, float /*timeboundLower*/, float /*timeboundUpper*/, ModelAnimationDirection /*direction*/, int /*pmi_id*/) override { };

	public:
		ModelAnimationSegmentRotation(optional<angles> targetAngle, optional<angles> velocity, optional<float> time, optional<angles> acceleration, bool isAbsolute = false);

	};

	class ModelAnimationSegmentSoundDuring : public ModelAnimationSegment {
		std::shared_ptr<ModelAnimationSegment> m_segment;

		struct instance_data {
			sound_handle currentlyPlaying;
		};

		//PMI ID -> Instance Data
		std::map<int, instance_data> m_instances;

		gamesnd_id m_start;
		gamesnd_id m_end;
		gamesnd_id m_during;
		bool m_flipIfReversed;

		void recalculate(const submodel_instance* submodel_instance, const bsp_info* submodel, const ModelAnimationData<>& base, int pmi_id) override;
		ModelAnimationData<true> calculateAnimation(const ModelAnimationData<>& base, float time, int pmi_id) const override;
		void executeAnimation(const ModelAnimationData<>& state, float timeboundLower, float timeboundUpper, ModelAnimationDirection direction, int pmi_id) override;

		void playStartSnd(int pmi_id);
		void playEndSnd(int pmi_id);

	public:
		ModelAnimationSegmentSoundDuring(std::shared_ptr<ModelAnimationSegment> segment, gamesnd_id start, gamesnd_id end, gamesnd_id during, bool flipIfReversed = false);

	};

}