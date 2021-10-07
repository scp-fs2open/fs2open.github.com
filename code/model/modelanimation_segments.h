#pragma once

#include "model/modelanimation.h"

namespace animation {

	//This segment handles multiple generic segments chained after one another
	class ModelAnimationSegmentSerial : public ModelAnimationSegment {
		static ModelAnimationParseHelper::Registrar reg;
		std::vector<std::shared_ptr<ModelAnimationSegment>> m_segments;

		virtual ModelAnimationSegment* copy() const override;
		void recalculate(ModelAnimationSubmodelBuffer& base, polymodel_instance* pmi) override;
		void calculateAnimation(ModelAnimationSubmodelBuffer& base, float time, int pmi_id) const override;
		void executeAnimation(const ModelAnimationSubmodelBuffer& state, float timeboundLower, float timeboundUpper, ModelAnimationDirection direction, int pmi_id) override;
		void exchangeSubmodelPointers(ModelAnimationSet& replaceWith) override;

		static std::shared_ptr<ModelAnimationSegment> parser(ModelAnimationParseHelper* data);
	public:
		void addSegment(std::shared_ptr<ModelAnimationSegment> segment);

	};

	//This segment handles multiple generic segments executing in parallel
	class ModelAnimationSegmentParallel : public ModelAnimationSegment {
		static ModelAnimationParseHelper::Registrar reg;
		std::vector<std::shared_ptr<ModelAnimationSegment>> m_segments;

		virtual ModelAnimationSegment* copy() const override;
		void recalculate(ModelAnimationSubmodelBuffer& base, polymodel_instance* pmi) override;
		void calculateAnimation(ModelAnimationSubmodelBuffer& base, float time, int pmi_id) const override;
		void executeAnimation(const ModelAnimationSubmodelBuffer& state, float timeboundLower, float timeboundUpper, ModelAnimationDirection direction, int pmi_id) override;
		void exchangeSubmodelPointers(ModelAnimationSet& replaceWith) override;

		static std::shared_ptr<ModelAnimationSegment> parser(ModelAnimationParseHelper* data);
	public:
		void addSegment(std::shared_ptr<ModelAnimationSegment> segment);

	};

	//This segment does nothing but serve as a placeholder taking up time, used primarily in serial segments
	class ModelAnimationSegmentWait : public ModelAnimationSegment {
		static ModelAnimationParseHelper::Registrar reg;
		float m_time;

		virtual ModelAnimationSegment* copy() const override;
		void recalculate(ModelAnimationSubmodelBuffer& /*base*/, polymodel_instance* pmi) override;
		void calculateAnimation(ModelAnimationSubmodelBuffer& /*base*/, float /*time*/, int /*pmi_id*/) const override { };
		void executeAnimation(const ModelAnimationSubmodelBuffer& /*state*/, float /*timeboundLower*/, float /*timeboundUpper*/, ModelAnimationDirection /*direction*/, int /*pmi_id*/) override { };
		void exchangeSubmodelPointers(ModelAnimationSet& replaceWith) override { };

		static std::shared_ptr<ModelAnimationSegment> parser(ModelAnimationParseHelper* data);
	public:
		ModelAnimationSegmentWait(float time);

	};

	//This segment changes or sets a submodels orientation to a defined Pitch Heading and Bank angle.
	class ModelAnimationSegmentSetPHB : public ModelAnimationSegment {
		static ModelAnimationParseHelper::Registrar reg;
		struct instance_data {
			matrix rot;
		};

		//PMI ID -> Instance Data
		std::map<int, instance_data> m_instances;

		std::shared_ptr<ModelAnimationSubmodel> m_submodel;

		angles m_targetAngle;
		bool m_isAngleRelative;

		virtual ModelAnimationSegment* copy() const override;
		void recalculate(ModelAnimationSubmodelBuffer& base, polymodel_instance* pmi) override;
		void calculateAnimation(ModelAnimationSubmodelBuffer& base, float /*time*/, int pmi_id) const override;
		void executeAnimation(const ModelAnimationSubmodelBuffer& /*state*/, float /*timeboundLower*/, float /*timeboundUpper*/, ModelAnimationDirection /*direction*/, int /*pmi_id*/) override { };
		void exchangeSubmodelPointers(ModelAnimationSet& replaceWith) override;

		static std::shared_ptr<ModelAnimationSegment> parser(ModelAnimationParseHelper* data);
	public:
		ModelAnimationSegmentSetPHB(std::shared_ptr<ModelAnimationSubmodel> submodel, const angles& angle, bool isAngleRelative);

	};

	//This segment rotates a submodels orientation by a certain amount around its defined rotation axis
	class ModelAnimationSegmentSetAngle : public ModelAnimationSegment {
		static ModelAnimationParseHelper::Registrar reg;
		std::shared_ptr<ModelAnimationSubmodel> m_submodel;

		float m_angle;
		matrix m_rot;

		virtual ModelAnimationSegment* copy() const override;
		void recalculate(ModelAnimationSubmodelBuffer& /*base*/, polymodel_instance* pmi) override;
		void calculateAnimation(ModelAnimationSubmodelBuffer& base, float /*time*/, int /*pmi_id*/) const override;
		void executeAnimation(const ModelAnimationSubmodelBuffer& /*state*/, float /*timeboundLower*/, float /*timeboundUpper*/, ModelAnimationDirection /*direction*/, int /*pmi_id*/) override { };
		void exchangeSubmodelPointers(ModelAnimationSet& replaceWith) override;

		static std::shared_ptr<ModelAnimationSegment> parser(ModelAnimationParseHelper* data);
	public:
		ModelAnimationSegmentSetAngle(std::shared_ptr<ModelAnimationSubmodel> submodel, float angle);

	};

	//This segment rotates a submodels orientation by a certain amount in PBH
	class ModelAnimationSegmentRotation : public ModelAnimationSegment {
		static ModelAnimationParseHelper::Registrar reg;
		struct instance_data {
			angles m_actualVelocity;
			angles m_actualTarget; //Usually won't be needed, but if vel + angle is specified, not all angles necessarily end simultaneously.
			angles m_actualTime;
			optional<angles> m_actualAccel;
			optional<angles> m_accelTime;
		};

		//PMI ID -> Instance Data
		std::map<int, instance_data> m_instances;

		std::shared_ptr<ModelAnimationSubmodel> m_submodel;

		optional<angles> m_targetAngle;
		optional<angles> m_velocity;
		optional<float> m_time;
		optional<angles> m_acceleration;
		bool m_isAbsolute;

		virtual ModelAnimationSegment* copy() const override;
		void recalculate(ModelAnimationSubmodelBuffer& base, polymodel_instance* pmi) override;
		void calculateAnimation(ModelAnimationSubmodelBuffer& base, float time, int pmi_id) const override;
		void executeAnimation(const ModelAnimationSubmodelBuffer& /*state*/, float /*timeboundLower*/, float /*timeboundUpper*/, ModelAnimationDirection /*direction*/, int /*pmi_id*/) override { };
		void exchangeSubmodelPointers(ModelAnimationSet& replaceWith) override;

		static std::shared_ptr<ModelAnimationSegment> parser(ModelAnimationParseHelper* data);
	public:
		ModelAnimationSegmentRotation(std::shared_ptr<ModelAnimationSubmodel> submodel, optional<angles> targetAngle, optional<angles> velocity, optional<float> time, optional<angles> acceleration, bool isAbsolute = false);

	};

	class ModelAnimationSegmentTranslation : public ModelAnimationSegment {
		static ModelAnimationParseHelper::Registrar reg;
		struct instance_data {
			vec3d m_actualVelocity;
			vec3d m_actualTarget; //Usually won't be needed, but if vel + angle is specified, not all angles necessarily end simultaneously.
			vec3d m_actualTime;
			optional<vec3d> m_actualAccel;
			optional<vec3d> m_accelTime;
			matrix m_rotationAtStart = vmd_identity_matrix;
		};

		//PMI ID -> Instance Data
		std::map<int, instance_data> m_instances;

		std::shared_ptr<ModelAnimationSubmodel> m_submodel;

		optional<vec3d> m_target;
		optional<vec3d> m_velocity;
		optional<float> m_time;
		optional<vec3d> m_acceleration;
		enum class CoordinateSystem { COORDS_PARENT, COORDS_LOCAL_AT_START, COORDS_LOCAL_CURRENT } m_coordType;

		virtual ModelAnimationSegment* copy() const override;
		void recalculate(ModelAnimationSubmodelBuffer& base, polymodel_instance* pmi) override;
		void calculateAnimation(ModelAnimationSubmodelBuffer& base, float time, int pmi_id) const override;
		void executeAnimation(const ModelAnimationSubmodelBuffer& /*state*/, float /*timeboundLower*/, float /*timeboundUpper*/, ModelAnimationDirection /*direction*/, int /*pmi_id*/) override { };
		void exchangeSubmodelPointers(ModelAnimationSet& replaceWith) override;

		static std::shared_ptr<ModelAnimationSegment> parser(ModelAnimationParseHelper* data);
	public:
		ModelAnimationSegmentTranslation(std::shared_ptr<ModelAnimationSubmodel> submodel, optional<vec3d> target, optional<vec3d> velocity, optional<float> time, optional<vec3d> acceleration, CoordinateSystem coordType = CoordinateSystem::COORDS_PARENT);

	};

	class ModelAnimationSegmentSoundDuring : public ModelAnimationSegment {
		static ModelAnimationParseHelper::Registrar reg;
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

		virtual ModelAnimationSegment* copy() const override;
		void recalculate(ModelAnimationSubmodelBuffer& base, polymodel_instance* pmi) override;
		void calculateAnimation(ModelAnimationSubmodelBuffer& base, float time, int pmi_id) const override;
		void executeAnimation(const ModelAnimationSubmodelBuffer& state, float timeboundLower, float timeboundUpper, ModelAnimationDirection direction, int pmi_id) override;
		void exchangeSubmodelPointers(ModelAnimationSet& replaceWith) override;

		void playStartSnd(int pmi_id);
		void playEndSnd(int pmi_id);

		static std::shared_ptr<ModelAnimationSegment> parser(ModelAnimationParseHelper* data);
	public:
		ModelAnimationSegmentSoundDuring(std::shared_ptr<ModelAnimationSegment> segment, gamesnd_id start, gamesnd_id end, gamesnd_id during, bool flipIfReversed = false);

	};

}