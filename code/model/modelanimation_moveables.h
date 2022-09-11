#pragma once

#include "math/ik_solver.h"
#include "model/modelanimation.h"

namespace animation {

	class ModelAnimationMoveableOrientation : public ModelAnimationMoveable {
		std::shared_ptr<ModelAnimationSubmodel> m_submodel;
		angles m_defaultPosOrient;

	public:
		static std::shared_ptr<ModelAnimationMoveable> parser();
		ModelAnimationMoveableOrientation(std::shared_ptr<ModelAnimationSubmodel> submodel, const angles& defaultOrient);

		void update(polymodel_instance* pmi, const std::vector<linb::any>& args) override;
		void initialize(ModelAnimationSet* parentSet, polymodel_instance* pmi) override;
	};

	class ModelAnimationMoveableRotation : public ModelAnimationMoveable {
		std::shared_ptr<ModelAnimationSubmodel> m_submodel;
		angles m_defaultPosOrient;
		angles m_velocity;
		tl::optional<angles> m_acceleration;

	public:
		static std::shared_ptr<ModelAnimationMoveable> parser();
		ModelAnimationMoveableRotation(std::shared_ptr<ModelAnimationSubmodel> submodel, const angles& defaultOrient, const angles& velocity, const tl::optional<angles>& acceleration);

		void update(polymodel_instance* pmi, const std::vector<linb::any>& args) override;
		void initialize(ModelAnimationSet* parentSet, polymodel_instance* pmi) override;
	};

	class ModelAnimationMoveableAxisRotation : public ModelAnimationMoveable {
		std::shared_ptr<ModelAnimationSubmodel> m_submodel;
		float m_velocity;
		tl::optional<float> m_acceleration;
		vec3d m_axis;

	public:
		static std::shared_ptr<ModelAnimationMoveable> parser();
		ModelAnimationMoveableAxisRotation(std::shared_ptr<ModelAnimationSubmodel> submodel, const float& velocity, const tl::optional<float>& acceleration, const vec3d& axis);

		void update(polymodel_instance* pmi, const std::vector<linb::any>& args) override;
		void initialize(ModelAnimationSet* parentSet, polymodel_instance* pmi) override;
	};

	class ModelAnimationMoveableIK : public ModelAnimationMoveable {
		float m_time;
		
		struct moveable_chainlink {
			std::shared_ptr<ModelAnimationSubmodel> submodel;
			std::shared_ptr<ik_constraint> constraint;
			tl::optional<angles> acceleration;
		};
		
		std::vector<moveable_chainlink> m_chain;
		
	public:
		static std::shared_ptr<ModelAnimationMoveable> parser();
		ModelAnimationMoveableIK(std::vector<moveable_chainlink> chain, float time);

		void update(polymodel_instance* pmi, const std::vector<linb::any>& args) override;
		void initialize(ModelAnimationSet* parentSet, polymodel_instance* pmi) override;
	};
	
}