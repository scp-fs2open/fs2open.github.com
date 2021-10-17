#include "model/modelanimation.h"
#include "model/modelanimation_segments.h"
#include "network/multi.h"
#include "network/multimsgs.h"
#include "ship/ship.h"

extern float flFrametime;

namespace animation {

	flag_def_list_new<animation::Animation_Flags> Animation_flags[] = {
		{ "auto reverse",				animation::Animation_Flags::Auto_Reverse,						true, false },
		{ "reset at completion",		animation::Animation_Flags::Reset_at_completion,		        true, false },
		{ "loop",						animation::Animation_Flags::Loop,						        true, false },
	};

	const size_t Num_animation_flags = sizeof(Animation_flags) / sizeof(flag_def_list_new<animation::Animation_Flags>);

	std::map<int, std::pair<const ModelAnimationSet*, std::list<std::shared_ptr<ModelAnimation>>>> ModelAnimationSet::s_runningAnimations;
	std::vector<std::shared_ptr<ModelAnimation>> ModelAnimation::s_animationById;

	ModelAnimation::ModelAnimation(bool isInitialType) : m_set(nullptr), m_isInitialType(isInitialType), id(s_animationById.size()) { }

	std::shared_ptr<ModelAnimation> ModelAnimation::createAnimation(bool isInitialType) {
		auto ptr = std::shared_ptr<ModelAnimation>(new ModelAnimation(isInitialType));
		s_animationById.push_back(ptr);
		return ptr;
	}

	void ModelAnimation::setAnimation(std::shared_ptr<ModelAnimationSegment> animation) {
		m_animation = animation;
	}

	ModelAnimationState ModelAnimation::play(float frametime, polymodel_instance* pmi, ModelAnimationSubmodelBuffer& applyBuffer, bool applyOnly) {
		instance_data& instanceData = m_instances[pmi->id];
		
		if (applyOnly) {
			m_animation->calculateAnimation(applyBuffer, instanceData.time, pmi->id);

			return instanceData.state;
		}

		float prevTime = instanceData.time;

		switch (instanceData.state) {
		case ModelAnimationState::UNTRIGGERED:
			//We have a new animation starting up in this phase. Put it in the list of running animations to track and step it later
			if (!m_isInitialType) {
				auto& animEntry = ModelAnimationSet::s_runningAnimations[pmi->id];
				animEntry.first = m_set;
				animEntry.second.push_back(shared_from_this());
			}

			//Store the submodels current data as the base for this animation and calculate this animations parameters
			m_animation->recalculate(applyBuffer, pmi);

			instanceData.duration = m_animation->getDuration(pmi->id);
			instanceData.state = ModelAnimationState::RUNNING_FWD;

			/* fall-thru */
		case ModelAnimationState::RUNNING_FWD:
			instanceData.time += frametime;

			//Cap time if needed
			if (instanceData.time > instanceData.duration) {
				instanceData.time = instanceData.duration;
				if (m_flags[Animation_Flags::Auto_Reverse])
					//Reverse
					instanceData.state = ModelAnimationState::RUNNING_RWD;
				else if (m_flags[Animation_Flags::Loop]) {
					if (m_flags[Animation_Flags::Reset_at_completion]) {
						//Loop from start
						instanceData.time = 0;
					}
					else {
						//Loop back
						instanceData.state = ModelAnimationState::RUNNING_RWD;
					}
				}
				else if (m_flags[Animation_Flags::Reset_at_completion])
					//Stop animation at start, not end
					stop(pmi, false);
				else
					//Normal execution, stop aniamtion at end
					instanceData.state = ModelAnimationState::COMPLETED;
			}

			m_animation->calculateAnimation(applyBuffer, instanceData.time, pmi->id);
			m_animation->executeAnimation(applyBuffer, prevTime, instanceData.time, ModelAnimationDirection::FWD, pmi->id);
			break;

		case ModelAnimationState::COMPLETED:
			//This means someone requested to start once we were complete, so start moving backwards.
			instanceData.state = ModelAnimationState::RUNNING_RWD;
			/* fall-thru */
		case ModelAnimationState::RUNNING_RWD:
			instanceData.time -= frametime;

			//Cap time at 0, but don't clean up the animations here since this function is called in a loop over the running animations, and cleaning now would invalidate the iterator
			if (instanceData.time < 0) {
				if (m_flags[Animation_Flags::Loop]) {
					if (m_flags[Animation_Flags::Reset_at_completion]) {
						//Loop from end. This happens when a Loop + Reset at completion animation is started in reverse.
						instanceData.time = instanceData.duration;
					}
					else {
						//Loop back
						instanceData.time = 0;
						instanceData.state = ModelAnimationState::RUNNING_FWD;
					}
				}
				else
					stop(pmi, false);
				break;
			}

			m_animation->calculateAnimation(applyBuffer, instanceData.time, pmi->id);
			m_animation->executeAnimation(applyBuffer, instanceData.time, prevTime, ModelAnimationDirection::RWD, pmi->id);
			break;
		case ModelAnimationState::PAUSED:
			//Shouldn't happen. Paused Animations are only allowed to be apply only.
			UNREACHABLE("Tried to play a paused animation without starting it. Get a coder.");
			break;
		}
		
		return instanceData.state;
	}
	
	void ModelAnimation::start(polymodel_instance* pmi, ModelAnimationDirection direction, bool force, bool instant, bool pause, const float* multiOverrideTime) {
		instance_data& instanceData = m_instances[pmi->id];

		if (multiOverrideTime == nullptr && (Game_mode & GM_MULTIPLAYER)) {
			//We are in multiplayer. Send animation to server to start. Server starts animation online, and sends start request back (which'll have multiOverride == true).
			//If we _are_ the server, also just start the animation

			send_animation_triggered_packet((int)id, pmi->id, direction, force, instant, pause);

			if(MULTIPLAYER_CLIENT)
				return;
		}

		if (pause) {
			if(instanceData.state != ModelAnimationState::UNTRIGGERED && instanceData.state != ModelAnimationState::COMPLETED)
				instanceData.state = ModelAnimationState::PAUSED;
			return;
		}

		float timeOffset = multiOverrideTime != nullptr ? *multiOverrideTime : 0.0f;

		if (direction == ModelAnimationDirection::RWD) {
			switch (instanceData.state) {
			case ModelAnimationState::RUNNING_RWD:
			case ModelAnimationState::UNTRIGGERED:
				//Cannot reverse-start if it's already running rwd or fully untriggered

				//If forced, reset and play anyways
				if (force) {
					//This needs to recalculate first, so start it as if it were going forwards, and then set it's time and continue on
					if (instanceData.state == ModelAnimationState::UNTRIGGERED)
						start(pmi, ModelAnimationDirection::FWD, true);
					instanceData.time = instant ? 0 : instanceData.duration - timeOffset;
					instanceData.state = ModelAnimationState::RUNNING_RWD;
					break;
				}

				return;
			case ModelAnimationState::RUNNING_FWD:
			case ModelAnimationState::PAUSED:
				//Just pretend we were going in the right direction
				instanceData.time -= timeOffset;

				if (force)
					instanceData.time = instant ? 0 : instanceData.duration - timeOffset;
				instanceData.state = ModelAnimationState::RUNNING_RWD;
				break;
			case ModelAnimationState::COMPLETED:
				//Nothing special to do. Expected case
				instanceData.time -= timeOffset;

				break;
			}
		}
		else {
			switch (instanceData.state) {
			case ModelAnimationState::RUNNING_FWD:
			case ModelAnimationState::COMPLETED:
				//Cannot start if it's already running fwd or fully triggered

				//If forced, reset and play anyways
				if (force) {
					instanceData.time = instant ? instanceData.duration : 0 + timeOffset;
					instanceData.state = ModelAnimationState::RUNNING_FWD;
					break;
				}

				return;
			case ModelAnimationState::RUNNING_RWD:
			case ModelAnimationState::PAUSED:
				//Just pretend we were going in the right direction
				instanceData.time += timeOffset;

				if (force)
					instanceData.time = instant ? instanceData.duration : 0 + timeOffset;
				instanceData.state = ModelAnimationState::RUNNING_FWD;
				break;
			case ModelAnimationState::UNTRIGGERED:
				//Nothing special to do. Expected case
				instanceData.time += timeOffset;

				break;
			}
		}

		//Make sure to recalculate the animation here, as otherwise we cannot inquire about things like length after starting.
		//Don't apply just yet if it's a non-initial type, as there might be other animations this'd need to depend upon
		ModelAnimationSubmodelBuffer applyBuffer;
		m_set->initializeSubmodelBuffer(pmi, applyBuffer);
		play(0, pmi, applyBuffer);
		//Since initial types never get stepped, they need to be manually applied here once.
		if (m_isInitialType) {
			ModelAnimationSet::apply(pmi, applyBuffer);

			//Save the things modified by initial animations as actual baseline
			for (const auto& initialModified : applyBuffer) {
				if (!initialModified.second.second)
					continue;

				initialModified.first->saveCurrentAsBase(pmi);
			}
		}
	}

	void ModelAnimation::stop(polymodel_instance* pmi, bool cleanup) {
		//If inaccuracies occur, this needs to reset again, but only if no other animation is running on this submodel
		/*for (const auto& animation : m_submodelAnimation) {
			animation->reset(pmi);
		}*/

		if (!pmi)
			return;

		instance_data& instanceData = m_instances[pmi->id];
		instanceData.time = 0.0f;
		instanceData.state = ModelAnimationState::UNTRIGGERED;

		if (cleanup)
			ModelAnimationSet::cleanRunning();
	}

	void ModelAnimation::stepAnimations(float frametime, polymodel_instance* pmi) {
		auto animListIt = ModelAnimationSet::s_runningAnimations.find(pmi->id);

		if (animListIt == ModelAnimationSet::s_runningAnimations.end())
			return;

		const auto& animList = animListIt->second;

		ModelAnimationSubmodelBuffer applyBuffer;
		animList.first->initializeSubmodelBuffer(pmi, applyBuffer);

		for (const auto& anim : animList.second) {
			switch (anim->m_instances[pmi->id].state) {
			case ModelAnimationState::RUNNING_FWD:
			case ModelAnimationState::RUNNING_RWD:
				anim->play(frametime, pmi, applyBuffer);
				break;
			case ModelAnimationState::COMPLETED:
			case ModelAnimationState::PAUSED:
				anim->play(frametime, pmi, applyBuffer, true);
				//Currently not moving. Keep in buffer in case some other animation starts on that submodel, but don't play without manual starting
				break;
			case ModelAnimationState::UNTRIGGERED:
				UNREACHABLE("An untriggered animation should not be in the runningAnimations buffer");
				break;
			}

		}

		ModelAnimationSet::apply(pmi, applyBuffer);

		//Clear Animations that might have completed this frame.
		ModelAnimationSet::cleanRunning();
	}

	ModelAnimationSubmodel::ModelAnimationSubmodel(SCP_string submodelName) : m_name(std::move(submodelName)) { }

	ModelAnimationSubmodel* ModelAnimationSubmodel::copy() const {
		return new ModelAnimationSubmodel(*this);
	}

	const ModelAnimationData<>& ModelAnimationSubmodel::getInitialData(polymodel_instance* pmi) {
		auto dataIt = m_initialData.find({ pmi->id });
		if (dataIt == m_initialData.end()) {
			saveCurrentAsBase(pmi);
		}

		return m_initialData.at(pmi->id);
	}

	void ModelAnimationSubmodel::reset(polymodel_instance* pmi) {
		if(!m_submodel.has())
			findSubmodel(pmi);

		auto dataIt = m_initialData.find({ pmi->id });
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

	void ModelAnimationSubmodel::saveCurrentAsBase(polymodel_instance* pmi) {
		auto submodel = findSubmodel(pmi);
		ModelAnimationData<>& data = m_initialData[{ pmi->id }];

		if (!submodel.first || !submodel.second) 
			return;

		data.orientation = submodel.first->canonical_orient;
		//TODO: Once translation is a thing
		//data.position = m_subsys->submodel_instance_1->offset;
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

	ModelAnimationSubmodelTurret::ModelAnimationSubmodelTurret(SCP_string subsystemName, bool findBarrel, SCP_string SIPname) : ModelAnimationSubmodel(std::move(subsystemName)), m_SIPname(std::move(SIPname)), m_findBarrel(findBarrel) {
		is_turret = true;
	}

	ModelAnimationSubmodel* ModelAnimationSubmodelTurret::copy() const {
		return new ModelAnimationSubmodelTurret(*this);
	}

	void ModelAnimationSubmodelTurret::renameSIP(const SCP_string& newSIPname) {
		m_SIPname = newSIPname;
	}

	std::pair<submodel_instance*, bsp_info*> ModelAnimationSubmodelTurret::findSubmodel(polymodel_instance* pmi) {
		//Ship was deleted
		if (pmi->id < 0)
			return { nullptr, nullptr };

		int submodelNumber = -1;

		polymodel* pm = model_get(pmi->model_num);

		//Do we have a submodel number already cached?
		if (m_submodel.has())
			submodelNumber = m_submodel;
		//Do we know if we were told to find the barrel submodel or not? This implies we have a subsystem name, not a submodel name
		else {

			int sip_index = ship_info_lookup(m_SIPname.c_str());
			if (sip_index < 0)
				return { nullptr, nullptr };
			ship_info* sip = &Ship_info[sip_index];

			for (int i = 0; i < sip->n_subsystems; i++) {
				if (!subsystem_stricmp(sip->subsystems[i].subobj_name, m_name.c_str())) {
					if ((bool)m_findBarrel) {
						//Check if the barrel subobj is a dedicated existing subobj or just the base turret.
						if (sip->subsystems[i].turret_gun_sobj != sip->subsystems[i].subobj_num)
							submodelNumber = sip->subsystems[i].turret_gun_sobj;
						else
							submodelNumber = -1;
					}
					else 
						submodelNumber = sip->subsystems[i].subobj_num;
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

	//To handle Multipart-turrets properly, they need a modified version that sets the actual angle, not just the orientation matrix.
	void ModelAnimationSubmodelTurret::copyToSubmodel(const ModelAnimationData<>& data, polymodel_instance* pmi) {
		submodel_instance* submodel = findSubmodel(pmi).first;

		if (!submodel)
			return;

		submodel->canonical_orient = data.orientation;

		float angle;
		vec3d axis;
		vm_matrix_to_rot_axis_and_angle(&submodel->canonical_orient, &angle, &axis);
		while (angle > PI2)
			angle -= PI2;
		while (angle < 0.0f)
			angle += PI2;

		submodel->cur_angle = angle;
		submodel->turret_idle_angle = angle;
	}


	float ModelAnimationSegment::getDuration(int pmi_id) const {
		return m_duration.at(pmi_id);
	}


	int ModelAnimationSet::SUBTYPE_DEFAULT = ANIMATION_SUBTYPE_ALL;

	ModelAnimationSet::ModelAnimationSet(SCP_string SIPname) : m_SIPname(SIPname) { };

	ModelAnimationSet::ModelAnimationSet(const ModelAnimationSet& other) {
		operator=(other);
	}

	ModelAnimationSet& ModelAnimationSet::operator=(ModelAnimationSet&& other) {
		std::swap(m_submodels, other.m_submodels);
		std::swap(m_animationSet, other.m_animationSet);
		std::swap(m_SIPname, other.m_SIPname);

		for (const auto& animationTypes : m_animationSet) {
			for (const auto& animations : animationTypes.second) {
				for(const auto& animation : animations.second)
					animation->m_set = this;
			}
		}

		return *this;
	}

	ModelAnimationSet& ModelAnimationSet::operator=(const ModelAnimationSet& other) {
		m_SIPname = other.m_SIPname;

		for (const auto& submodel : other.m_submodels) {
			auto newSubmodel = std::shared_ptr<ModelAnimationSubmodel>(submodel->copy());
			newSubmodel->m_submodel = optional<int>();
			m_submodels.push_back(newSubmodel);
		}

		for (const auto& animationTypes : other.m_animationSet) {
			auto& newAnimations = m_animationSet[animationTypes.first];
			for (const auto& oldAnimations : animationTypes.second) {
				for (const auto& oldAnimation : oldAnimations.second) {
					std::shared_ptr<ModelAnimation> newAnimation = std::shared_ptr<ModelAnimation>(new ModelAnimation(*oldAnimation));
					newAnimation->m_animation = std::shared_ptr<ModelAnimationSegment>(newAnimation->m_animation->copy());

					newAnimation->m_animation->exchangeSubmodelPointers(*this);
					newAnimation->m_set = this;

					newAnimations[oldAnimations.first].push_back(newAnimation);
				}
			}
		}

		return *this;
	}

	void ModelAnimationSet::emplace(const std::shared_ptr<ModelAnimation>& animation, const SCP_string& name, ModelAnimationTriggerType type, int subtype) {
		auto newAnim = std::shared_ptr<ModelAnimation>(new ModelAnimation(*animation));
		newAnim->m_set = this;
		newAnim->m_animation = std::shared_ptr<ModelAnimationSegment>(animation->m_animation->copy());
		newAnim->m_animation->exchangeSubmodelPointers(*this);
		m_animationSet[{type, subtype}][name].push_back(newAnim);
	}

	void ModelAnimationSet::changeShipName(const SCP_string& name) {
		m_SIPname = name;
		for (const auto& submodel : m_submodels) {
			submodel->renameSIP(name);
		}
	}

	void ModelAnimationSet::cleanRunning() {
		auto removeIt = s_runningAnimations.begin();
		while (removeIt != s_runningAnimations.end()) {
			auto animIt = removeIt->second.second.cbegin();
			while (animIt != removeIt->second.second.cend()) {
				if ((*animIt)->m_instances[removeIt->first].state == ModelAnimationState::UNTRIGGERED) {
					animIt = removeIt->second.second.erase(animIt);
				}
				else
					animIt++;
			}

			if (removeIt->second.second.empty()) {
				removeIt = s_runningAnimations.erase(removeIt);
			}
			else
				removeIt++;
		}
	}

	void ModelAnimationSet::stopAnimations(polymodel_instance* pmi) {
		if (pmi != nullptr) {
			for (const auto& anim : s_runningAnimations[pmi->id].second) {
				anim->stop(pmi, false);
			}
			s_runningAnimations.erase(pmi->id);
		}
		else {
			for (const auto& animList : s_runningAnimations) {
				for (const auto& anim : animList.second.second) {
					anim->stop(model_get_instance(animList.first), false);
				}
			}

			s_runningAnimations.clear();
		}
	}

	void ModelAnimationSet::clearShipData(polymodel_instance* pmi) {
		for (const auto& submodel : m_submodels) {
			submodel->m_initialData.erase(pmi->id);
		}
	}

	void ModelAnimationSet::apply(polymodel_instance* pmi, const ModelAnimationSubmodelBuffer& applyBuffer) {
		for (const auto& toApply : applyBuffer) {
			toApply.first->copyToSubmodel(toApply.second.first, pmi);
		}
	}

	void ModelAnimationSet::initializeSubmodelBuffer(polymodel_instance* pmi, ModelAnimationSubmodelBuffer& applyBuffer) const {
		for (const auto& submodel : m_submodels) {
			ModelAnimationData<> base = submodel->getInitialData(pmi);
			applyBuffer[submodel].first = base;
		}
	}

	bool ModelAnimationSet::start(polymodel_instance* pmi, ModelAnimationTriggerType type, const SCP_string& name, ModelAnimationDirection direction, bool forced, bool instant, bool pause, int subtype) const {
		if (pmi == nullptr)
			return false;

		bool started = false;
		auto animations = m_animationSet.find({ type, subtype });
		if (animations != m_animationSet.end()) {
			auto namedAnimations = animations->second.find(name);
			if (namedAnimations != animations->second.end()) {
				for (const auto& namedAnimation : namedAnimations->second) {
					namedAnimation->start(pmi, direction, forced, instant, pause);
					started = true;
				}
			}
		}

		//Only search for default anims again if these weren't looked for in the first place
		if (subtype == SUBTYPE_DEFAULT)
			return started;

		animations = m_animationSet.find({ type, SUBTYPE_DEFAULT });
		if (animations != m_animationSet.end()) {
			auto namedAnimations = animations->second.find(name);
			if (namedAnimations != animations->second.end()) {
				for (const auto& namedAnimation : namedAnimations->second) {
					namedAnimation->start(pmi, direction, forced, instant, pause);
					started = true;
				}
			}
		}
		return started;
	}

	bool ModelAnimationSet::startAll(polymodel_instance* pmi, ModelAnimationTriggerType type, ModelAnimationDirection direction, bool forced, bool instant, bool pause, int subtype, bool strict) const {
		if (pmi == nullptr)
			return false;

		bool started = false;
		auto animations = m_animationSet.find({ type, subtype });
		if (animations != m_animationSet.end()) {
			for (auto& namedAnimations : animations->second) {
				for (const auto& namedAnimation : namedAnimations.second) {
					namedAnimation->start(pmi, direction, forced, instant, pause);
					started = true;
				}
			}
		}

		//Only search for default anims again if these weren't looked for in the first place
		if (strict || subtype == SUBTYPE_DEFAULT)
			return started;

		animations = m_animationSet.find({ type, SUBTYPE_DEFAULT });
		if (animations != m_animationSet.end()) {
			for (auto& namedAnimations : animations->second) {
				for (const auto& namedAnimation : namedAnimations.second) {
					namedAnimation->start(pmi, direction, forced, instant, pause);
					started = true;
				}
			}
		}
		return started;
	}

	//Yes why of course does this need special handling...
	bool ModelAnimationSet::startDockBayDoors(polymodel_instance* pmi, ModelAnimationDirection direction, bool forced, bool instant, bool pause, int subtype) const {
		if (pmi == nullptr)
			return false;

		bool started = false;
		subtype++;

		for (const auto& animList : m_animationSet) {
			if (animList.first.first != ModelAnimationTriggerType::DockBayDoor)
				continue;

			if (animList.first.second != ModelAnimationSet::SUBTYPE_DEFAULT) {
				//Trigger on all but x type
				if (animList.first.second < 0 && animList.first.second == subtype)
					continue;

				//Trigger only on x type. For the record, animsubtype 0 cannot happen here.
				if (animList.first.second > 0 && animList.first.second != subtype)
					continue;
			}
			for (auto& namedAnimations : animList.second) {
				for (const auto& namedAnimation : namedAnimations.second) {
					namedAnimation->start(pmi, direction, forced, instant, pause);
					started = true;
				}
			}
		}
		return started;
	}

	int ModelAnimationSet::getTimeDockBayDoors(polymodel_instance* pmi, int subtype) const {
		float duration = 0.0f;
		subtype++;

		for (const auto& animList : m_animationSet) {
			if (animList.first.first != ModelAnimationTriggerType::DockBayDoor)
				continue;

			if (animList.first.second != ModelAnimationSet::SUBTYPE_DEFAULT) {
				//Trigger on all but x type
				if (animList.first.second < 0 && animList.first.second == subtype)
					continue;

				//Trigger only on x type. For the record, animsubtype 0 cannot happen here.
				if (animList.first.second > 0 && animList.first.second != subtype)
					continue;
			}
			for (auto& namedAnimations : animList.second) {
				for (const auto& namedAnimation : namedAnimations.second) {
					if (namedAnimation->m_instances[pmi->id].state == ModelAnimationState::UNTRIGGERED)
						continue;

					float localDur = namedAnimation->m_instances[pmi->id].duration;
					duration = duration < localDur ? localDur : duration;
				}
			}
		}

		return (int) (duration * 1000);
	}

	int ModelAnimationSet::getTime(polymodel_instance* pmi, ModelAnimationTriggerType type, const SCP_string& name, int subtype) const {
		float duration = 0.0f;

		auto animations = m_animationSet.find({ type, subtype });
		if (animations != m_animationSet.end()) {
			auto namedAnimations = animations->second.find(name);
			if (namedAnimations != animations->second.end()){
				for (const auto& namedAnimation : namedAnimations->second) {
					if (namedAnimation->m_instances[pmi->id].state != ModelAnimationState::UNTRIGGERED) {
						float localDur = namedAnimation->m_instances[pmi->id].duration;
						duration = duration < localDur ? localDur : duration;
					}
				}
			}
		}

		//Only search for default anims again if these weren't looked for in the first place
		if (subtype == SUBTYPE_DEFAULT)
			return (int) (duration * 1000);

		animations = m_animationSet.find({ type, SUBTYPE_DEFAULT });
		if (animations != m_animationSet.end()) {
			auto namedAnimations = animations->second.find(name);
			if (namedAnimations != animations->second.end()) {
				for (const auto& namedAnimation : namedAnimations->second) {
					if (namedAnimation->m_instances[pmi->id].state != ModelAnimationState::UNTRIGGERED) {
						float localDur = namedAnimation->m_instances[pmi->id].duration;
						duration = duration < localDur ? localDur : duration;
					}
				}
			}
		}

		return (int) (duration * 1000);
	}

	int ModelAnimationSet::getTimeAll(polymodel_instance* pmi, ModelAnimationTriggerType type, int subtype, bool strict) const {
		float duration = 0.0f;
		auto animations = m_animationSet.find({ type, subtype });
		if (animations != m_animationSet.end()) {
			for (const auto& namedAnimations : animations->second) {
				for (const auto& namedAnimation : namedAnimations.second) {
					if (namedAnimation->m_instances[pmi->id].state == ModelAnimationState::UNTRIGGERED)
						continue;
					float localDur = namedAnimation->m_instances[pmi->id].duration;
					duration = duration < localDur ? localDur : duration;
				}
			}
		}

		//Only search for default anims again if these weren't looked for in the first place
		if (strict || subtype == SUBTYPE_DEFAULT)
			return (int) (duration * 1000);

		animations = m_animationSet.find({ type, SUBTYPE_DEFAULT });
		if (animations != m_animationSet.end()) {
			for (const auto& namedAnimations : animations->second) {
				for (const auto& namedAnimation : namedAnimations.second) {
					if (namedAnimation->m_instances[pmi->id].state == ModelAnimationState::UNTRIGGERED)
						continue;
					float localDur = namedAnimation->m_instances[pmi->id].duration;
					duration = duration < localDur ? localDur : duration;
				}
			}
		}

		return (int) (duration * 1000);
	}

	bool ModelAnimationSet::isEmpty() const {
		for (const auto& animSet : m_animationSet) {
			if (!animSet.second.empty())
				return false;
		}

		return true;
	};

	std::shared_ptr<ModelAnimationSubmodel> ModelAnimationSet::getSubmodel(SCP_string submodelName) {
		for (const auto& submodel : m_submodels) {
			if (!submodel->is_turret && submodel->m_name == submodelName)
				return submodel;
		}

		auto submodel = std::shared_ptr<ModelAnimationSubmodel>(new ModelAnimationSubmodel(submodelName));
		m_submodels.push_back(submodel);
		return submodel;
	}

	std::shared_ptr<ModelAnimationSubmodel> ModelAnimationSet::getSubmodel(SCP_string submodelName, SCP_string SIP_name, bool findBarrel) {
		for (const auto& submodel : m_submodels) {
			if (submodel->is_turret && submodel->m_name == submodelName) {
				auto submodelTurret = ((ModelAnimationSubmodelTurret*)submodel.get());
				if (submodelTurret->m_SIPname == SIP_name && submodelTurret->m_findBarrel == findBarrel)
					return submodel;
			}
		}

		auto submodel = std::shared_ptr<ModelAnimationSubmodelTurret>(new ModelAnimationSubmodelTurret(submodelName, findBarrel, SIP_name));
		m_submodels.push_back(submodel);
		return submodel;
	}

	std::shared_ptr<ModelAnimationSubmodel> ModelAnimationSet::getSubmodel(const std::shared_ptr<ModelAnimationSubmodel> other) {
		if (other->is_turret) {
			const ModelAnimationSubmodelTurret* const turret = (const ModelAnimationSubmodelTurret*)other.get();
			return getSubmodel(turret->m_name, m_SIPname, turret->m_findBarrel);
		}
		else
			return getSubmodel(other->m_name);
	}


	void anim_set_initial_states(ship* shipp) {
		ship_info* sip = &Ship_info[shipp->ship_info_index];
		sip->animations.clearShipData(model_get_instance(shipp->model_instance_num));
		sip->animations.startAll(model_get_instance(shipp->model_instance_num), animation::ModelAnimationTriggerType::Initial, ModelAnimationDirection::FWD, true, true);
	}

	const std::map<ModelAnimationTriggerType, std::pair<const char*, bool>> Animation_types = {
	{ModelAnimationTriggerType::Initial, {"initial", false}},
	{ModelAnimationTriggerType::Docking_Stage1, {"docking-stage-1", false}},
	{ModelAnimationTriggerType::Docking_Stage2, {"docking-stage-2", false}},
	{ModelAnimationTriggerType::Docking_Stage3, {"docking-stage-3", false}},
	{ModelAnimationTriggerType::Docked, {"docked", false}},
	{ModelAnimationTriggerType::PrimaryBank, {"primary-bank", false}},
	{ModelAnimationTriggerType::SecondaryBank, {"secondary-bank", false}},
	{ModelAnimationTriggerType::DockBayDoor, {"fighterbay", false}},
	{ModelAnimationTriggerType::Afterburner, {"afterburner", false}},
	{ModelAnimationTriggerType::TurretFiring, {"turret-firing", false}},
	{ModelAnimationTriggerType::Scripted, {"scripted", false}},
	{ModelAnimationTriggerType::TurretFired, {"turret-fired", true}},
	{ModelAnimationTriggerType::PrimaryFired,   {"primary-fired", true}},
	{ModelAnimationTriggerType::SecondaryFired, {"secondary-fired", true}},
	{ModelAnimationTriggerType::WeaponLaunched, {"weapon-launched", false}} //Atypical case. While no reverse trigger occurs, it is also guaranteed to not trigger more than once per model, hence non-resetting animations are fine here.
	};

	ModelAnimationTriggerType anim_match_type(const char* p)
	{
		// standard match
		for (const auto& entry : Animation_types) {
			if (!strnicmp(p, entry.second.first, strlen(entry.second.first)))
				return entry.first;
		}

		// Goober5000 - misspelling
		if (!strnicmp(p, "inital", 6) || !strnicmp(p, "\"inital\"", 8)) {
			Warning(LOCATION, "Spelling error in table file.  Please change \"inital\" to \"initial\".");
			return ModelAnimationTriggerType::Initial;
		}

		// Goober5000 - deprecation
		if (!strnicmp(p, "docking", 7) || !strnicmp(p, "\"docking\"", 9)) {
			auto docking_string = Animation_types.find(ModelAnimationTriggerType::Docking_Stage2)->second.first;
			mprintf(("The \"docking\" animation type name is deprecated.  Specify \"%s\" instead.\n", docking_string));
			return ModelAnimationTriggerType::Docking_Stage2;
		}
		else if (!strnicmp(p, "primary_bank", 12) || !strnicmp(p, "\"primary_bank\"", 14)) {
			auto pbank_string = Animation_types.find(ModelAnimationTriggerType::PrimaryBank)->second.first;
			mprintf(("The \"primary_bank\" animation type name is deprecated.  Specify \"%s\" instead.\n", pbank_string));
			return ModelAnimationTriggerType::PrimaryBank;
		}
		else if (!strnicmp(p, "secondary_bank", 14) || !strnicmp(p, "\"secondary_bank\"", 16)) {
			auto sbank_string = Animation_types.find(ModelAnimationTriggerType::SecondaryBank)->second.first;
			mprintf(("The \"secondary_bank\" animation type name is deprecated.  Specify \"%s\" instead.\n", sbank_string));
			return ModelAnimationTriggerType::SecondaryBank;
		}
		else if (!strnicmp(p, "door", 4) || !strnicmp(p, "\"door\"", 6)) {
			auto docking_string = Animation_types.find(ModelAnimationTriggerType::DockBayDoor)->second.first;
			mprintf(("The \"door\" animation type name is deprecated.  Specify \"%s\" instead.\n", docking_string));
			return ModelAnimationTriggerType::DockBayDoor;
		}
		else if (!strnicmp(p, "turret firing", 13) || !strnicmp(p, "\"turret firing\"", 15)) {
			auto turret_string = Animation_types.find(ModelAnimationTriggerType::TurretFiring)->second.first;
			mprintf(("The \"turret firing\" animation type name is deprecated.  Specify \"%s\" instead.\n", turret_string));
			return ModelAnimationTriggerType::TurretFiring;
		}

		// Goober5000 - with quotes
		for (const auto& entry : Animation_types) {
			char quoted_name[NAME_LENGTH + 2];
			strcpy(quoted_name, "\"");
			strcat(quoted_name, entry.second.first);
			strcat(quoted_name, "\"");

			if (!strnicmp(p, quoted_name, strlen(quoted_name))) {
				mprintf(("Old usage warning: Please remove quotes from animation type %s.\n", quoted_name));
				return entry.first;
			}
		}

		return ModelAnimationTriggerType::None;
	}

	SCP_string anim_name_from_subsys(model_subsystem* ss) {
		char namelower[MAX_NAME_LEN];
		strncpy(namelower, ss->subobj_name, MAX_NAME_LEN);
		strlwr(namelower);
		return namelower;
	}


	//Parsing functions

	std::map<SCP_string, ModelAnimationParseHelper::ModelAnimationSegmentParser> ModelAnimationParseHelper::s_segmentParsers;
	std::map<SCP_string, ModelAnimationParseHelper::ParsedModelAnimation> ModelAnimationParseHelper::s_animationsById;

	std::shared_ptr<ModelAnimationSegment> ModelAnimationParseHelper::parseSegment() {
		ignore_white_space();
		char segment_type[NAME_LENGTH];
		stuff_string(segment_type, F_NAME, NAME_LENGTH);

		auto segment_parser = s_segmentParsers.find(segment_type);
		if (segment_parser != s_segmentParsers.end())
			return segment_parser->second(this);
		else {
			error_display(1, "Unknown segment type % s in animation with ID % s!", segment_type, m_animationName.c_str());
			return nullptr;
		}
	}

	std::shared_ptr<ModelAnimationSubmodel> ModelAnimationParseHelper::parseSubmodel() {
		int turretStatus = optional_string_one_of(3, "+Submodel:", "+Turret Base:", "+Turret Arm:");

		if (turretStatus == -1)
			return nullptr;

		char name[NAME_LENGTH];
		stuff_string(name, F_NAME, NAME_LENGTH);

		switch (turretStatus) {
		case 0: //Submodel
			return std::shared_ptr<ModelAnimationSubmodel>(new ModelAnimationSubmodel(name));
		case 1: //Turret Base
			return std::shared_ptr<ModelAnimationSubmodel>(new ModelAnimationSubmodelTurret(name, false, ""));
		case 2: //Turret Arm
			return std::shared_ptr<ModelAnimationSubmodel>(new ModelAnimationSubmodelTurret(name, true, ""));
		default: //Not specified
			return nullptr;
		}

	}

	void ModelAnimationParseHelper::parseSingleAnimation() {
		auto animation = std::shared_ptr<ModelAnimation>(ModelAnimation::createAnimation());

		ModelAnimationParseHelper helper;
		required_string("$Name:");
		char animID[NAME_LENGTH];
		stuff_string(animID, F_NAME, NAME_LENGTH);
		helper.m_animationName = animID;

		if (s_animationsById.count(helper.m_animationName))
			error_display(1, "Animation with name %s already exists!", animID);

		required_string("$Type:");
		char atype[NAME_LENGTH];
		stuff_string(atype, F_NAME, NAME_LENGTH);
		ModelAnimationTriggerType type = anim_match_type(atype);

		int subtype = ModelAnimationSet::SUBTYPE_DEFAULT;
		SCP_string name = "";

		if (optional_string("+Triggered By:")) {
			switch (type) {
			case ModelAnimationTriggerType::Docking_Stage1:
			case ModelAnimationTriggerType::Docking_Stage2:
			case ModelAnimationTriggerType::Docking_Stage3:
			case ModelAnimationTriggerType::Docked:
				//The index of the dock port
			case ModelAnimationTriggerType::PrimaryBank:
			case ModelAnimationTriggerType::SecondaryBank:
			case ModelAnimationTriggerType::PrimaryFired:
			case ModelAnimationTriggerType::SecondaryFired:
				//The index of the bank
				stuff_int(&subtype);

				break;
			case ModelAnimationTriggerType::DockBayDoor: {
				//Index of the dock bay door, if NOT before, then all but this door
				bool flip = optional_string("NOT");
				stuff_int(&subtype);
				subtype++;
				if (flip)
					subtype *= -1;

				break;
			}
			case ModelAnimationTriggerType::Scripted:
				//More accurate name of scripted animation
			case ModelAnimationTriggerType::TurretFired:
			case ModelAnimationTriggerType::TurretFiring: {
				//Name of the turret subsys that needs to be firing
				char parsedname[NAME_LENGTH];
				stuff_string(parsedname, F_NAME, NAME_LENGTH);
				name = parsedname;
				break;
			}

			case ModelAnimationTriggerType::Initial:
			case ModelAnimationTriggerType::Afterburner:
			case ModelAnimationTriggerType::WeaponLaunched:
			default:
				error_display(0, "Animation trigger type %s does not use any trigger specification!", atype);
				//These shouldn't have further specifications. Ignore
			}
		}

		if (optional_string("$Flags:")) {
			SCP_vector<SCP_string> unparsed;
			parse_string_flag_list(animation->m_flags, Animation_flags, Num_animation_flags, &unparsed);
		}

		if (Animation_types.find(type)->second.second) {
			//This is a type where the code does not reset the animation. The animation is expected to auto-reset in one way or another.
			//If it doesn't, nothing will crash, but the result is most likely unintended.
			if (!animation->m_flags[Animation_Flags::Auto_Reverse, Animation_Flags::Reset_at_completion]) {
				error_display(0, "Animation trigger type %s expects an auto-reset flag to be set.", atype);
			}

			if (animation->m_flags[Animation_Flags::Loop]) {
				//Looping animations for these trigger types are probably unintended as well, but rare cases could exist, hence no explicit warning.
				mprintf(("Animation %s with trigger type %s has an unexpected loop flag.", helper.m_animationName.c_str(), atype));
			}
		}

		animation->setAnimation(helper.parseSegment());

		s_animationsById.emplace(helper.m_animationName, ParsedModelAnimation { animation, type, name, subtype });
	}

	void ModelAnimationParseHelper::parseTableFile(const char* filename) {
		try {
			read_file_text(filename, CF_TYPE_TABLES);
			reset_parse();

			required_string("#Animations");
			ignore_white_space();

			while (!optional_string("#End")) {
				parseSingleAnimation();
				ignore_white_space();
			}
		}
		catch (const parse::ParseException& e) {
			mprintf(("TABLES: Unable to parse '%s'!  Error message = %s.\n", filename, e.what()));
			return;
		}

	}


	void ModelAnimationParseHelper::parseTables() {
		s_animationsById.clear();
		parse_modular_table("*-anim.tbm", parseTableFile, CF_TYPE_TABLES);
	}
	
	void ModelAnimationParseHelper::parseAnimsetInfo(ModelAnimationSet& set, ship_info* sip) {
		if(sip != nullptr)
			set.changeShipName(sip->name);
		
		SCP_vector<SCP_string> requestedAnimations;
		stuff_string_list(requestedAnimations);

		for (const SCP_string& request : requestedAnimations) {
			auto animIt = s_animationsById.find(request);
			if (animIt != s_animationsById.end()) {
				const ParsedModelAnimation& foundAnim = animIt->second;
				set.emplace(foundAnim.anim, foundAnim.name, foundAnim.type, foundAnim.subtype);
			}
			else {
				error_display(0, "Animation with name %s not found!", request.c_str());
			}
		}
	}


	void ModelAnimationParseHelper::parseLegacyAnimationTable(model_subsystem* sp, ship_info* sip) {
		//the only thing initial animation type needs is the angle, 
		//so to save space lets just make everything optional in this case

		//Most of these properties were read and allowed, but never actually used for anything.
		//Hence, still allow them in tables, but now just skip reading them

		required_string("$type:");
		char atype[NAME_LENGTH];
		stuff_string(atype, F_NAME, NAME_LENGTH);
		animation::ModelAnimationTriggerType type = anim_match_type(atype);
		int subtype = ModelAnimationSet::SUBTYPE_DEFAULT;
		char sub_name[NAME_LENGTH];

		if (optional_string("+sub_type:")) {
			stuff_int(&subtype);

			if (type == ModelAnimationTriggerType::DockBayDoor) {
				if (subtype == 0) {
					//Apparently the legacy code treated a 0 here like subtype default (this is incorrect in a lot of places).
					subtype = ModelAnimationSet::SUBTYPE_DEFAULT;
				}
				else {
					//Increase by 1, so that -x != x is true for all possible values
					subtype += subtype < 0 ? -1 : 1;
				}

			}
		}

		if (optional_string("+sub_name:")) {
			stuff_string(sub_name, F_NAME, NAME_LENGTH);
		}
		else {
			strcpy_s(sub_name, "<none>");
		}

		if (type == animation::ModelAnimationTriggerType::Initial) {
			angles angle;
			bool isRelative;

			if (optional_string("+delay:"))
				skip_token();

			if (optional_string("+reverse_delay:"))
				skip_token();

			if (optional_string("+absolute_angle:")) {
				stuff_angles_deg_phb(&angle);

				isRelative = false;
			}
			else {
				required_string("+relative_angle:");

				stuff_angles_deg_phb(&angle);

				isRelative = true;
			}

			if (optional_string("+velocity:"))
				skip_token();

			if (optional_string("+acceleration:"))
				skip_token();

			if (optional_string("+time:"))
				skip_token();

			std::shared_ptr<ModelAnimation> anim = ModelAnimation::createAnimation(true);

			char namelower[MAX_NAME_LEN];
			strncpy(namelower, sp->subobj_name, MAX_NAME_LEN);
			strlwr(namelower);
			//since sp->type is not set without reading the pof, we need to infer it by subsystem name (which works, since the same name is used to match the submodels name, which is used to match the type in pof parsing)
			//sadly, we also need to check for engine and radar, since these take precedent (as in, an engineturret is an engine before a turret type)
			if (!strstr(namelower, "engine") && !strstr(namelower, "radar") && strstr(namelower, "turret")) {
				auto subsysBase = sip->animations.getSubmodel(sp->subobj_name, sip->name, false);
				auto rotBase = std::shared_ptr<ModelAnimationSegmentSetAngle>(new ModelAnimationSegmentSetAngle(subsysBase, angle.h));

				auto subsysBarrel = sip->animations.getSubmodel(sp->subobj_name, sip->name, true);
				auto rotBarrel = std::shared_ptr<ModelAnimationSegmentSetAngle>(new ModelAnimationSegmentSetAngle(subsysBarrel, angle.p));

				auto rot = std::shared_ptr<ModelAnimationSegmentParallel>(new ModelAnimationSegmentParallel());
				rot->addSegment(std::move(rotBase));
				rot->addSegment(std::move(rotBarrel));

				anim->setAnimation(std::move(rot));
			}
			else {
				auto subsys = sip->animations.getSubmodel(sp->subobj_name);
				auto rot = std::shared_ptr<ModelAnimationSegmentSetPHB>(new ModelAnimationSegmentSetPHB(subsys, angle, isRelative));
				anim->setAnimation(std::move(rot));
			}

			//Initial Animations in legacy style will continue to be fully supported and allowed, given the frequency of these (especially for turrets) and the fact that these are more intuitive to be directly in the subsystem section of the ship table, as these are closer to representing a property of the subsystem rather than an animation.
			//Hence, there will not be any warning displayed if the legacy table is used for these. -Lafiel 
			sip->animations.emplace(anim, anim_name_from_subsys(sp), animation::ModelAnimationTriggerType::Initial);
		}
		else {
			std::shared_ptr<ModelAnimation> anim = ModelAnimation::createAnimation();
			auto subsys = sip->animations.getSubmodel(sp->subobj_name);

			if (type == ModelAnimationTriggerType::TurretFired) {
				//Turret fireds won't get reset by code, so make them auto-resetting
				anim->m_flags += Animation_Flags::Reset_at_completion;
			}

			auto mainSegment = std::shared_ptr<ModelAnimationSegmentSerial>(new ModelAnimationSegmentSerial());

			if (optional_string("+delay:")) {
				int delayByMs;
				stuff_int(&delayByMs);
				auto delay = std::shared_ptr<ModelAnimationSegmentWait>(new ModelAnimationSegmentWait(((float)delayByMs) * 0.001f));
				mainSegment->addSegment(delay);
			}

			int delayByMsReverse = -1;
			if (optional_string("+reverse_delay:")) {
				stuff_int(&delayByMsReverse);
			}

			angles target{ 0,0,0 };
			bool absolute = false;

			if (optional_string("+absolute_angle:")) {
				absolute = true;

				stuff_angles_deg_phb(&target);
			}
			else {
				absolute = false;

				required_string("+relative_angle:");
				stuff_angles_deg_phb(&target);
			}

			angles velocity{ 0,0,0 };
			required_string("+velocity:");
			stuff_angles_deg_phb(&velocity);

			optional<angles> acceleration;

			if (optional_string("+acceleration:")) {
				angles accel{ 0,0,0 };
				stuff_angles_deg_phb(&accel);

				bool allZero = accel.p == 0 && accel.b == 0 && accel.h == 0;

				if (!allZero) {
					acceleration = accel;
				}
			}

			if (optional_string("+time:")) {
				skip_token();

				//Time is ignored if acceleration is set in legacy animations. Even if it isn't set, time seems to only affect metadata and not the actual animation.
				//Hence, throw time away, and let the segment handle calculating how long it actually takes
			}

			auto rotation = std::shared_ptr<ModelAnimationSegmentRotation>(new ModelAnimationSegmentRotation(subsys, target, velocity, optional<float>(), acceleration, absolute));

			if (optional_string("$Sound:")) {
				gamesnd_id start_sound;
				gamesnd_id loop_sound;
				gamesnd_id end_sound;
				float snd_rad;

				parse_game_sound("+Start:", &start_sound);

				parse_game_sound("+Loop:", &loop_sound);

				parse_game_sound("+End:", &end_sound);

				required_string("+Radius:");
				stuff_float(&snd_rad);

				auto sound = std::shared_ptr<ModelAnimationSegmentSoundDuring>(new ModelAnimationSegmentSoundDuring(rotation, start_sound, end_sound, loop_sound, true));
				mainSegment->addSegment(sound);
			}
			else
				mainSegment->addSegment(rotation);

			if (delayByMsReverse != -1) {
				auto delay = std::shared_ptr<ModelAnimationSegmentWait>(new ModelAnimationSegmentWait(((float)delayByMsReverse) * 0.001f));
				mainSegment->addSegment(delay);
			}

			anim->setAnimation(mainSegment);

			//TODO maybe handle sub_name? Not documented in Wiki, maybe no one actually uses it...
			sip->animations.emplace(anim, anim_name_from_subsys(sp), type, subtype);

			mprintf(("Specified deprecated non-initial type animation on subsystem %s of ship class %s. Consider using *-anim.tbm's instead.", sp->subobj_name, sip->name));
		}
	}
}