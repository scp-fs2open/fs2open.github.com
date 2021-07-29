#include "model/modelanimation.h"
#include "model/modelanimation_segments.h"
#include "ship/ship.h"

extern float flFrametime;

namespace animation {

	std::multimap<int, std::shared_ptr<ModelAnimation>> ModelAnimation::s_runningAnimations;

	ModelAnimationState ModelAnimation::play(float frametime, polymodel_instance* pmi) {

		float& timeCurrentAnim = m_time[pmi->id];

		switch (m_state[pmi->id]) {
		case ModelAnimationState::UNTRIGGERED:
			//We have a new animation starting up in this phase. Put it in the list of running animations to track and step it later
			s_runningAnimations.emplace(pmi->id, shared_from_this());
			m_duration = 0.0f;
			//Stop other running animations on subsystems we care about. Store subsystems initial values as well.
			for (const auto& animation : m_submodelAnimation) {
				//We need to make sure that we have the submodel index cached before we check if other animations have the same index
				animation->findSubmodel(pmi);

				auto animIterRange = s_runningAnimations.equal_range(pmi->id);
				for (auto animIter = animIterRange.first; animIter != animIterRange.second; animIter++) {
					//Don't stop this animation, even though it (obviously) runs on the same submodels
					if (animIter->second == shared_from_this())
						continue;

					const auto& otherAnims = animIter->second->m_submodelAnimation;
					bool needStop = false;

					//See if we can find a submodel that must be reset. In case the other submodel has not been cached, it cannot have been running, so it is fine to miss these here
					for (const auto& otherAnim : otherAnims) {
						if (otherAnim->m_submodel == animation->m_submodel) {
							needStop = true;
							break;
						}
					}
					
					if (needStop) {
						animIter->second->stop(model_get_instance(animIter->first), false);
					}
				}

				//Store the submodels current data as the base for this animation and calculate this animations parameters
				float duration = animation->saveCurrentAsBase(pmi);

				//Update this animations duration based on all the submodel animations (note that these could differ in multiple executions of the same animation, for example in case of absolute angles)
				if (duration > m_duration)
					m_duration = duration;
			}

			m_state[pmi->id] = ModelAnimationState::RUNNING_FWD;

			/* fall-thru */
		case ModelAnimationState::RUNNING_FWD:
			timeCurrentAnim += frametime;

			//Cap time if needed
			if (timeCurrentAnim > m_duration) {
				timeCurrentAnim = m_duration;
				m_state[pmi->id] = ModelAnimationState::COMPLETED;
			}

			for (const auto& animation : m_submodelAnimation) {
				animation->play(timeCurrentAnim, pmi);
			}
			break;

		case ModelAnimationState::COMPLETED:
			//This means someone requested to start once we were complete, so start moving backwards.
			m_state[pmi->id] = ModelAnimationState::RUNNING_RWD;
			/* fall-thru */
		case ModelAnimationState::RUNNING_RWD:
			timeCurrentAnim -= frametime;

			//Cap time at 0, but don't clean up the animations here since this function is called in a loop over the running animations, and cleaning now would invalidate the iterator
			if (timeCurrentAnim < 0) {
				stop(pmi, false);
				break;
			}

			for (const auto& animation : m_submodelAnimation) {
				animation->play(timeCurrentAnim, pmi);
			}

			break;
		}

		return m_state[pmi->id];
	}

	void ModelAnimation::cleanRunning() {
		auto removeIt = s_runningAnimations.cbegin();
		while (removeIt != s_runningAnimations.cend()) {
			if (removeIt->second->m_state[removeIt->first] == ModelAnimationState::UNTRIGGERED) {
				removeIt = s_runningAnimations.erase(removeIt);
			}
			else
				removeIt++;
		}
	}
	
	void ModelAnimation::start(polymodel_instance* pmi, bool reverse) {
		if (reverse) {
			switch (m_state[pmi->id]) {
			case ModelAnimationState::RUNNING_RWD:
			case ModelAnimationState::UNTRIGGERED:
				//Cannot reverse-start if it's already running rwd or fully untriggered
				return;
			case ModelAnimationState::RUNNING_FWD:
				//Just pretend we were going in the right direction
				m_state[pmi->id] = ModelAnimationState::RUNNING_RWD;
				break;
			case ModelAnimationState::COMPLETED:
				//Nothing special to do. Expected case
				break;
			}
		}
		else {
			switch (m_state[pmi->id]) {
			case ModelAnimationState::RUNNING_FWD:
			case ModelAnimationState::COMPLETED:
				//Cannot start if it's already running fwd or fully triggered
				return;
			case ModelAnimationState::RUNNING_RWD:
				//Just pretend we were going in the right direction
				m_state[pmi->id] = ModelAnimationState::RUNNING_FWD;
				break;
			case ModelAnimationState::UNTRIGGERED:
				//Nothing special to do. Expected case
				break;
			}
		}
		
		//Play them once without changing their set time, this will make sure they stop other conflicting animations and set up properly.
		play(0, pmi);
		//In case this stopped some other animations, we need to remove them from the playing buffer
		cleanRunning();
	}

	void ModelAnimation::stop(polymodel_instance* pmi, bool cleanup) {
		for (const auto& animation : m_submodelAnimation) {
			animation->reset(pmi);
		}

		m_time[pmi->id] = 0;
		m_state[pmi->id] = ModelAnimationState::UNTRIGGERED;

		if (cleanup)
			cleanRunning();
	}

	void ModelAnimation::addSubsystemAnimation(std::unique_ptr<ModelAnimationSubmodel> animation) {
		m_submodelAnimation.push_back(std::move(animation));
	}

	void ModelAnimation::stepAnimations(float frametime) {
		for (const auto& anim : s_runningAnimations) {
			switch (anim.second->m_state[anim.first]) {
			case ModelAnimationState::RUNNING_FWD:
			case ModelAnimationState::RUNNING_RWD:
				anim.second->play(frametime, model_get_instance(anim.first));
				break;
			case ModelAnimationState::COMPLETED:
				//Fully triggered. Keep in buffer in case some other animation starts on that submodel, but don't play without manual starting
				break;
			case ModelAnimationState::UNTRIGGERED:
				UNREACHABLE("An untriggered animation should not be in the runningAnimations buffer");
				break;
			}

		}

		//Clear Animations that might have completed this frame.
		cleanRunning();
	}

	void ModelAnimation::clearAnimations() {
		for (const auto& anim : s_runningAnimations) {
			anim.second->stop(model_get_instance(anim.first), false);
		}

		s_runningAnimations.clear();
	}

	void ModelAnimation::parseLegacyAnimationTable(model_subsystem* sp, ship_info* sip) {
		//the only thing initial animation type needs is the angle, 
		//so to save space lets just make everything optional in this case

		//Most of these properties were read and allowed, but never actually used for anything.
		//Hence, still allow them in tables, but now just skip reading them

		angles angle;
		bool isRelative;

		if (optional_string("+delay:"))
			skip_token();

		if (optional_string("+reverse_delay:"))
			skip_token();

		if (optional_string("+absolute_angle:")) {
			vec3d anglesDeg;
			stuff_vec3d(&anglesDeg);

			angle.p = fl_radians(anglesDeg.xyz.x);
			angle.h = fl_radians(anglesDeg.xyz.y);
			angle.b = fl_radians(anglesDeg.xyz.z);

			isRelative = false;
		}
		else {
			vec3d anglesDeg;
			required_string("+relative_angle:");

			stuff_vec3d(&anglesDeg);

			angle.p = fl_radians(anglesDeg.xyz.x);
			angle.h = fl_radians(anglesDeg.xyz.y);
			angle.b = fl_radians(anglesDeg.xyz.z);

			isRelative = true;
		}

		if (optional_string("+velocity:"))
			skip_token();

		if (optional_string("+acceleration:"))
			skip_token();

		if (optional_string("+time:"))
			skip_token();

		std::shared_ptr<ModelAnimation> anim = std::shared_ptr<ModelAnimation>(new ModelAnimation());

		char namelower[MAX_NAME_LEN];
		strncpy(namelower, sp->subobj_name, MAX_NAME_LEN);
		strlwr(namelower);
		//since sp->type is not set without reading the pof, we need to infer it by subsystem name (which works, since the same name is used to match the submodels name, which is used to match the type in pof parsing)
		//sadly, we also need to check for engine and radar, since these take precedent (as in, an engineturret is an engine before a turret type)
		if (!strstr(namelower, "engine") && !strstr(namelower, "radar") && strstr(namelower, "turret")) {
			auto rotBase = std::unique_ptr<ModelAnimationSegmentSetAngle>(new ModelAnimationSegmentSetAngle(angle.h));
			auto subsysBase = std::unique_ptr<ModelAnimationSubmodel>(new ModelAnimationSubmodel(sp->subobj_name, false, sip, std::move(rotBase)));
			anim->addSubsystemAnimation(std::move(subsysBase));

			auto rotBarrel = std::unique_ptr<ModelAnimationSegmentSetAngle>(new ModelAnimationSegmentSetAngle(angle.p));
			auto subsysBarrel = std::unique_ptr<ModelAnimationSubmodel>(new ModelAnimationSubmodel(sp->subobj_name, true, sip, std::move(rotBarrel)));
			anim->addSubsystemAnimation(std::move(subsysBarrel));
		}
		else {
			auto rot = std::unique_ptr<ModelAnimationSegmentSetPHB>(new ModelAnimationSegmentSetPHB(angle, isRelative));
			auto subsys = std::unique_ptr<ModelAnimationSubmodel>(new ModelAnimationSubmodel(sp->subobj_name, std::move(rot)));
			anim->addSubsystemAnimation(std::move(subsys));
		}

		sip->animations.emplace(anim, "", animation::ModelAnimationTriggerType::Initial);
	}

	/*std::shared_ptr<ModelAnimation> ModelAnimation::parseAnimationTable() {
		//TODO. This is not the function to parse the legacy table, that is still part of modelread.cpp
		//This will eventually be the function to parse the new animation table
		return std::make_shared<ModelAnimation>(nullptr);
	}*/


	ModelAnimationSubmodel::ModelAnimationSubmodel(SCP_string submodelName, std::unique_ptr<ModelAnimationSegment> mainSegment) : m_name(std::move(submodelName)), m_mainSegment(std::move(mainSegment)) { }

	ModelAnimationSubmodel::ModelAnimationSubmodel(SCP_string subsystemName, bool findBarrel, ship_info* sip, std::unique_ptr<ModelAnimationSegment> mainSegment) : m_name(std::move(subsystemName)), m_findBarrel(findBarrel), m_sip(sip), m_mainSegment(std::move(mainSegment)) { }

	void ModelAnimationSubmodel::play(float frametime, polymodel_instance* pmi) {
		auto dataIt = m_initialData.find(pmi->id);
		auto lastDataIt = m_lastFrame.find(pmi->id);

		//Specified submodel not found. Don't play
		if (dataIt == m_initialData.end() || lastDataIt == m_lastFrame.end())
			return;

		//Cap the frametime at this submodels duration
		if (frametime > m_mainSegment->getDuration())
			frametime = m_mainSegment->getDuration();

		ModelAnimationData<> currentFrame = dataIt->second;
		//Calculate the submodels data (or its delta from the data stored at the animation's start)
		ModelAnimationData<true> delta = m_mainSegment->calculateAnimation(currentFrame, lastDataIt->second, frametime);
		
		currentFrame.applyDelta(delta);

		//Execute stuff of the animation that doesn't modify this delta (stuff like sounds / particles)
		m_mainSegment->executeAnimation(currentFrame, frametime);

		//Actually apply the result to the submodel
		copyToSubmodel(currentFrame, pmi);
		m_lastFrame[pmi->id] = currentFrame;
	}

	void ModelAnimationSubmodel::reset(polymodel_instance* pmi) {
		auto dataIt = m_initialData.find(pmi->id);
		if (dataIt == m_initialData.end())
			return;

		//Since resetting the submodel animation also has to happen for the submodel itself, copy its state at the animation's start back to the submodel
		copyToSubmodel(dataIt->second, pmi);
	}

	void ModelAnimationSubmodel::copyToSubmodel(const ModelAnimationData<>& data, polymodel_instance* pmi) {
		submodel_instance* submodel = findSubmodel(pmi).first;
		if (!submodel)
			return;

		submodel->canonical_orient = data.orientation;
		//TODO: Once translation is a thing
		//m_subsys->submodel_instance_1->offset = data.position;
	}

	float ModelAnimationSubmodel::saveCurrentAsBase(polymodel_instance* pmi) {
		auto submodel = findSubmodel(pmi);
		if (!submodel.first || !submodel.second)
			return 0;

		ModelAnimationData<>& data = m_initialData[pmi->id];
		data.orientation = submodel.first->canonical_orient;
		//TODO: Once translation is a thing
		//data.position = m_subsys->submodel_instance_1->offset;
		
		m_lastFrame[pmi->id] = data;
		m_mainSegment->recalculate(submodel.first, submodel.second, data);
		return m_mainSegment->getDuration();
	}

	std::pair<submodel_instance*, bsp_info*> ModelAnimationSubmodel::findSubmodel(polymodel_instance* pmi) {
		//Ship was deleted
		if(pmi->id < 0)
			return { nullptr, nullptr };

		int submodelNumber = -1;

		polymodel* pm = model_get(pmi->model_num);

		//Do we have a submodel number already cached?
		if (m_submodel.has())
			submodelNumber = m_submodel;
		//Do we know if we were told to find the barrel submodel or not? This implies we have a subsystem name, not a submodel name
		else if (m_findBarrel.has()) {

			if(m_sip == nullptr)
				return { nullptr, nullptr };

			for (int i = 0; i < m_sip->n_subsystems; i++) {
				if (!subsystem_stricmp(m_sip->subsystems[i].subobj_name, m_name.c_str())) {
					if ((bool)m_findBarrel) {
						//Check if the barrel subobj is a dedicated existing subobj or just the base turret.
						if (m_sip->subsystems[i].turret_gun_sobj != m_sip->subsystems[i].subobj_num)
							submodelNumber = m_sip->subsystems[i].turret_gun_sobj;
						else
							submodelNumber = -1;
					}
					else
						submodelNumber = m_sip->subsystems[i].subobj_num;
					break;
				}
			}

			m_submodel = submodelNumber;
		}
		//We seem to have a submodel name
		else {
			for (int i = 0; i < pm->n_models; i++) {
				if (!subsystem_stricmp(pm->submodel[i].name, m_name.c_str())) {
					submodelNumber = i;
					break;
				}
			}

			m_submodel = submodelNumber;
		}

		//If the model does not exist, return null. The system is expected to just silently tolerate this,
		//as this needs to be the case to be able to handle turrets whose barrels may or may not exist, depending on the pof which is not loaded at table parse time
		if (submodelNumber < 0 || submodelNumber >= pm->n_models)
			return { nullptr, nullptr };

		return { &pmi->submodel[submodelNumber], &pm->submodel[submodelNumber] };
	}


	float ModelAnimationSegment::getDuration() const {
		return m_duration;
	}


	int ModelAnimationSet::SUBTYPE_DEFAULT = ANIMATION_SUBTYPE_ALL;

	void ModelAnimationSet::emplace(const std::shared_ptr<ModelAnimation>& animation, const SCP_string& name, ModelAnimationTriggerType type, int subtype) {
		animationSet[{type, subtype}].emplace(name, animation);
	}


	void anim_set_initial_states(ship* shipp) {
		ship_info* sip = &Ship_info[shipp->ship_info_index];

		const auto& initialAnims = sip->animations.animationSet[{animation::ModelAnimationTriggerType::Initial, animation::ModelAnimationSet::SUBTYPE_DEFAULT}];

		for (const auto& anim : initialAnims) {
			anim.second->start(model_get_instance(shipp->model_instance_num), false);
		}
	}
}