#pragma once

#include "math/ik_solver.h"
#include "model/modelanimation.h"

namespace animation {

	//This segment handles multiple generic segments chained after one another
	class ModelAnimationSegmentSerial : public ModelAnimationSegment {
		//configureables:
	public:
		std::vector<std::shared_ptr<ModelAnimationSegment>> m_segments;

	private:
		void recalculate(ModelAnimationSubmodelBuffer& base, polymodel_instance* pmi) override;
		void calculateAnimation(ModelAnimationSubmodelBuffer& base, float time, int pmi_id) const override;
		void executeAnimation(const ModelAnimationSubmodelBuffer& state, float timeboundLower, float timeboundUpper, ModelAnimationDirection direction, int pmi_id) override;
		void exchangeSubmodelPointers(ModelAnimationSet& replaceWith) override;

	public:
		ModelAnimationSegment* copy() const override;
		static std::shared_ptr<ModelAnimationSegment> parser(ModelAnimationParseHelper* data);
		void addSegment(std::shared_ptr<ModelAnimationSegment> segment);

	};

	//This segment handles multiple generic segments executing in parallel
	class ModelAnimationSegmentParallel : public ModelAnimationSegment {
		//configureables:
	public:
		std::vector<std::shared_ptr<ModelAnimationSegment>> m_segments;

	private:
		void recalculate(ModelAnimationSubmodelBuffer& base, polymodel_instance* pmi) override;
		void calculateAnimation(ModelAnimationSubmodelBuffer& base, float time, int pmi_id) const override;
		void executeAnimation(const ModelAnimationSubmodelBuffer& state, float timeboundLower, float timeboundUpper, ModelAnimationDirection direction, int pmi_id) override;
		void exchangeSubmodelPointers(ModelAnimationSet& replaceWith) override;

	public:
		ModelAnimationSegment* copy() const override;
		static std::shared_ptr<ModelAnimationSegment> parser(ModelAnimationParseHelper* data);
		void addSegment(std::shared_ptr<ModelAnimationSegment> segment);

	};

	//This segment does nothing but serve as a placeholder taking up time, used primarily in serial segments
	class ModelAnimationSegmentWait : public ModelAnimationSegment {
		//configureables:
	public:
		float m_time;

	private:
		ModelAnimationSegment* copy() const override;
		void recalculate(ModelAnimationSubmodelBuffer& /*base*/, polymodel_instance* pmi) override;
		void calculateAnimation(ModelAnimationSubmodelBuffer& /*base*/, float /*time*/, int /*pmi_id*/) const override { };
		void executeAnimation(const ModelAnimationSubmodelBuffer& /*state*/, float /*timeboundLower*/, float /*timeboundUpper*/, ModelAnimationDirection /*direction*/, int /*pmi_id*/) override { };
		void exchangeSubmodelPointers(ModelAnimationSet& /*replaceWith*/) override { };

	public:
		static std::shared_ptr<ModelAnimationSegment> parser(ModelAnimationParseHelper* /*data*/);
		ModelAnimationSegmentWait(float time);

	};

	//This segment changes or sets a submodels orientation to a defined Pitch Heading and Bank angle.
	class ModelAnimationSegmentSetOrientation : public ModelAnimationSegment {
		struct instance_data {
			matrix rot;
		};

		//PMI ID -> Instance Data
		std::map<int, instance_data> m_instances;

		//configurables:
	public:
		std::shared_ptr<ModelAnimationSubmodel> m_submodel;
		tl::optional<angles> m_targetAngle;
		tl::optional<matrix> m_targetOrientation;
		bool m_isAngleRelative;

	private:
		ModelAnimationSegment* copy() const override;
		void recalculate(ModelAnimationSubmodelBuffer& base, polymodel_instance* pmi) override;
		void calculateAnimation(ModelAnimationSubmodelBuffer& base, float /*time*/, int pmi_id) const override;
		void executeAnimation(const ModelAnimationSubmodelBuffer& /*state*/, float /*timeboundLower*/, float /*timeboundUpper*/, ModelAnimationDirection /*direction*/, int /*pmi_id*/) override { };
		void exchangeSubmodelPointers(ModelAnimationSet& replaceWith) override;

	public:
		static std::shared_ptr<ModelAnimationSegment> parser(ModelAnimationParseHelper* data);
		ModelAnimationSegmentSetOrientation(std::shared_ptr<ModelAnimationSubmodel> submodel, const angles& angle, bool isAngleRelative);
		ModelAnimationSegmentSetOrientation(std::shared_ptr<ModelAnimationSubmodel> submodel, const matrix& orientation, bool isAngleRelative);
	};

	//This segment rotates a submodels orientation by a certain amount around its defined rotation axis
	class ModelAnimationSegmentSetAngle : public ModelAnimationSegment {
		//configurables:
	public:
		std::shared_ptr<ModelAnimationSubmodel> m_submodel;
		float m_angle;
		matrix m_rot;

	private:
		ModelAnimationSegment* copy() const override;
		void recalculate(ModelAnimationSubmodelBuffer& /*base*/, polymodel_instance* pmi) override;
		void calculateAnimation(ModelAnimationSubmodelBuffer& base, float /*time*/, int /*pmi_id*/) const override;
		void executeAnimation(const ModelAnimationSubmodelBuffer& /*state*/, float /*timeboundLower*/, float /*timeboundUpper*/, ModelAnimationDirection /*direction*/, int /*pmi_id*/) override { };
		void exchangeSubmodelPointers(ModelAnimationSet& replaceWith) override;

	public:
		static std::shared_ptr<ModelAnimationSegment> parser(ModelAnimationParseHelper* data);
		ModelAnimationSegmentSetAngle(std::shared_ptr<ModelAnimationSubmodel> submodel, float angle);

	};

	//This segment rotates a submodels orientation by a certain amount in PBH
	class ModelAnimationSegmentRotation : public ModelAnimationSegment {
		struct instance_data {
			angles m_actualVelocity;
			angles m_actualTarget; //Usually won't be needed, but if vel + angle is specified, not all angles necessarily end simultaneously.
			angles m_actualTime;
			tl::optional<angles> m_actualAccel;
			tl::optional<angles> m_accelTime;
		};

		//PMI ID -> Instance Data
		std::map<int, instance_data> m_instances;
		
		//configurables:
	public:
		std::shared_ptr<ModelAnimationSubmodel> m_submodel;
		tl::optional<angles> m_targetAngle;
		tl::optional<angles> m_velocity;
		tl::optional<float> m_time;
		tl::optional<angles> m_acceleration;
		bool m_isAbsolute;

	private:
		ModelAnimationSegment* copy() const override;
		void recalculate(ModelAnimationSubmodelBuffer& base, polymodel_instance* pmi) override;
		void calculateAnimation(ModelAnimationSubmodelBuffer& base, float time, int pmi_id) const override;
		void executeAnimation(const ModelAnimationSubmodelBuffer& /*state*/, float /*timeboundLower*/, float /*timeboundUpper*/, ModelAnimationDirection /*direction*/, int /*pmi_id*/) override { };
		void exchangeSubmodelPointers(ModelAnimationSet& replaceWith) override;

	public:
		static std::shared_ptr<ModelAnimationSegment> parser(ModelAnimationParseHelper* data);
		ModelAnimationSegmentRotation(std::shared_ptr<ModelAnimationSubmodel> submodel, tl::optional<angles> targetAngle, tl::optional<angles> velocity, tl::optional<float> time, tl::optional<angles> acceleration, bool isAbsolute = false);

	};

	
	//This segment rotates a submodels orientation by a certain amount in PBH
	class ModelAnimationSegmentAxisRotation : public ModelAnimationSegment {
		struct instance_data {
			float m_actualVelocity;
			float m_actualTarget;
			float m_actualTime;
			tl::optional<float> m_actualAccel;
			tl::optional<float> m_accelTime;
		};

		//PMI ID -> Instance Data
		std::map<int, instance_data> m_instances;

		//configurables:
	public:
		std::shared_ptr<ModelAnimationSubmodel> m_submodel;
		tl::optional<float> m_targetAngle;
		tl::optional<float> m_velocity;
		tl::optional<float> m_time;
		tl::optional<float> m_acceleration;
		vec3d m_axis;

	private:
		ModelAnimationSegment* copy() const override;
		void recalculate(ModelAnimationSubmodelBuffer& base, polymodel_instance* pmi) override;
		void calculateAnimation(ModelAnimationSubmodelBuffer& /*base*/, float time, int pmi_id) const override;
		void executeAnimation(const ModelAnimationSubmodelBuffer& /*state*/, float /*timeboundLower*/, float /*timeboundUpper*/, ModelAnimationDirection /*direction*/, int /*pmi_id*/) override { };
		void exchangeSubmodelPointers(ModelAnimationSet& replaceWith) override;

	public:
		static std::shared_ptr<ModelAnimationSegment> parser(ModelAnimationParseHelper* data);
		ModelAnimationSegmentAxisRotation(std::shared_ptr<ModelAnimationSubmodel> submodel, tl::optional<float> targetAngle, tl::optional<float> velocity, tl::optional<float> time, tl::optional<float> acceleration, const vec3d& axis);

	};

	class ModelAnimationSegmentTranslation : public ModelAnimationSegment {
		struct instance_data {
			vec3d m_actualVelocity;
			vec3d m_actualTarget; //Usually won't be needed, but if vel + angle is specified, not all angles necessarily end simultaneously.
			vec3d m_actualTime;
			tl::optional<vec3d> m_actualAccel;
			tl::optional<vec3d> m_accelTime;
			matrix m_rotationAtStart = vmd_identity_matrix;
		};

		//PMI ID -> Instance Data
		std::map<int, instance_data> m_instances;

		//configurables:
	public:
		std::shared_ptr<ModelAnimationSubmodel> m_submodel;
		tl::optional<vec3d> m_target;
		tl::optional<vec3d> m_velocity;
		tl::optional<float> m_time;
		tl::optional<vec3d> m_acceleration;
		enum class CoordinateSystem { COORDS_PARENT, COORDS_LOCAL_AT_START, COORDS_LOCAL_CURRENT } m_coordType;

	private:

		ModelAnimationSegment* copy() const override;
		void recalculate(ModelAnimationSubmodelBuffer& base, polymodel_instance* pmi) override;
		void calculateAnimation(ModelAnimationSubmodelBuffer& base, float time, int pmi_id) const override;
		void executeAnimation(const ModelAnimationSubmodelBuffer& /*state*/, float /*timeboundLower*/, float /*timeboundUpper*/, ModelAnimationDirection /*direction*/, int /*pmi_id*/) override { };
		void exchangeSubmodelPointers(ModelAnimationSet& replaceWith) override;
	public:
		static std::shared_ptr<ModelAnimationSegment> parser(ModelAnimationParseHelper* data);
		ModelAnimationSegmentTranslation(std::shared_ptr<ModelAnimationSubmodel> submodel, tl::optional<vec3d> target, tl::optional<vec3d> velocity, tl::optional<float> time, tl::optional<vec3d> acceleration, CoordinateSystem coordType = CoordinateSystem::COORDS_PARENT);

	};

	class ModelAnimationSegmentSoundDuring : public ModelAnimationSegment {
		std::shared_ptr<ModelAnimationSegment> m_segment;

		struct instance_data {
			sound_handle currentlyPlaying;
		};

		//PMI ID -> Instance Data
		std::map<int, instance_data> m_instances;

		//configurables:
	public:
		gamesnd_id m_start;
		gamesnd_id m_end;
		gamesnd_id m_during;
		bool m_flipIfReversed;
		
	private:
		ModelAnimationSegment* copy() const override;
		void recalculate(ModelAnimationSubmodelBuffer& base, polymodel_instance* pmi) override;
		void calculateAnimation(ModelAnimationSubmodelBuffer& base, float time, int pmi_id) const override;
		void executeAnimation(const ModelAnimationSubmodelBuffer& state, float timeboundLower, float timeboundUpper, ModelAnimationDirection direction, int pmi_id) override;
		void exchangeSubmodelPointers(ModelAnimationSet& replaceWith) override;

		void playStartSnd(int pmi_id);
		void playEndSnd(int pmi_id);

	public:
		static std::shared_ptr<ModelAnimationSegment> parser(ModelAnimationParseHelper* data);
		ModelAnimationSegmentSoundDuring(std::shared_ptr<ModelAnimationSegment> segment, gamesnd_id start, gamesnd_id end, gamesnd_id during, bool flipIfReversed = false);

	};
	
	class ModelAnimationSegmentIK : public ModelAnimationSegment {
		struct instance_data {
			
		};

		struct chainlink_data{
			std::shared_ptr<ModelAnimationSubmodel> submodel;
			std::shared_ptr<ik_constraint> constraint;
			std::shared_ptr<ModelAnimationSegmentRotation> animSegment;
		};
		
		//PMI ID -> Instance Data
		std::map<int, instance_data> m_instances;
	public:
		
		std::vector<chainlink_data> m_chain;
		std::shared_ptr<ModelAnimationSegmentParallel> m_segment;
		vec3d m_targetPosition;
		tl::optional<matrix> m_targetRotation;
	private:
		ModelAnimationSegment* copy() const override;
		void recalculate(ModelAnimationSubmodelBuffer& base, polymodel_instance* pmi) override;
		void calculateAnimation(ModelAnimationSubmodelBuffer& base, float time, int pmi_id) const override;
		void executeAnimation(const ModelAnimationSubmodelBuffer& /*state*/, float /*timeboundLower*/, float /*timeboundUpper*/, ModelAnimationDirection /*direction*/, int /*pmi_id*/) override { };
		void exchangeSubmodelPointers(ModelAnimationSet& replaceWith) override;
	public:
		static std::shared_ptr<ModelAnimationSegment> parser(ModelAnimationParseHelper* data);
		ModelAnimationSegmentIK(const vec3d& targetPosition, const tl::optional<matrix>& targetRotation);

	};

}