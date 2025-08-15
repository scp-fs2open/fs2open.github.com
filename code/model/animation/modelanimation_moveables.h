#pragma once

#include "math/ik_solver.h"
#include "model/animation/modelanimation.h"

namespace animation {

	class ModelAnimationMoveableOrientation : public ModelAnimationMoveable {
		std::shared_ptr<ModelAnimationSubmodel> m_submodel;
		angles m_defaultPosOrient;

	public:
		static std::shared_ptr<ModelAnimationMoveable> parser();
		ModelAnimationMoveableOrientation(std::shared_ptr<ModelAnimationSubmodel> submodel, const angles& defaultOrient);

		void update(polymodel_instance* pmi, const SCP_vector<std::any>& args) override;
		void initialize(ModelAnimationSet* parentSet, polymodel_instance* pmi) override;
	};

	class ModelAnimationMoveableRotation : public ModelAnimationMoveable {
		std::shared_ptr<ModelAnimationSubmodel> m_submodel;
		angles m_defaultPosOrient;
		angles m_velocity;
		std::optional<angles> m_acceleration;

	public:
		static std::shared_ptr<ModelAnimationMoveable> parser();
		ModelAnimationMoveableRotation(std::shared_ptr<ModelAnimationSubmodel> submodel, const angles& defaultOrient, const angles& velocity, const std::optional<angles>& acceleration);

		void update(polymodel_instance* pmi, const SCP_vector<std::any>& args) override;
		void initialize(ModelAnimationSet* parentSet, polymodel_instance* pmi) override;
	};

	class ModelAnimationMoveableTranslation : public ModelAnimationMoveable {
		std::shared_ptr<ModelAnimationSubmodel> m_submodel;
		vec3d m_defaultOffset;
		vec3d m_velocity;
		std::optional<vec3d> m_acceleration;

	  public:
		static std::shared_ptr<ModelAnimationMoveable> parser();
		ModelAnimationMoveableTranslation(std::shared_ptr<ModelAnimationSubmodel> submodel, const vec3d& defaultOffset, const vec3d& velocity, const std::optional<vec3d>& acceleration);

		void update(polymodel_instance* pmi, const SCP_vector<std::any>& args) override;
		void initialize(ModelAnimationSet* parentSet, polymodel_instance* pmi) override;
	};

	class ModelAnimationMoveableAxisRotation : public ModelAnimationMoveable {
		std::shared_ptr<ModelAnimationSubmodel> m_submodel;
		float m_velocity;
		std::optional<float> m_acceleration;
		vec3d m_axis;

	public:
		static std::shared_ptr<ModelAnimationMoveable> parser();
		ModelAnimationMoveableAxisRotation(std::shared_ptr<ModelAnimationSubmodel> submodel, const float& velocity, const std::optional<float>& acceleration, const vec3d& axis);

		void update(polymodel_instance* pmi, const SCP_vector<std::any>& args) override;
		void initialize(ModelAnimationSet* parentSet, polymodel_instance* pmi) override;
	};

	class ModelAnimationMoveableIK : public ModelAnimationMoveable {
		float m_time;
		
		struct moveable_chainlink {
			std::shared_ptr<ModelAnimationSubmodel> submodel;
			std::shared_ptr<ik_constraint> constraint;
			std::optional<angles> acceleration;
		};
		
		std::vector<moveable_chainlink> m_chain;
		
	public:
		static std::shared_ptr<ModelAnimationMoveable> parser();
		ModelAnimationMoveableIK(std::vector<moveable_chainlink> chain, float time);

		void update(polymodel_instance* pmi, const SCP_vector<std::any>& args) override;
		void initialize(ModelAnimationSet* parentSet, polymodel_instance* pmi) override;
	};
	
}