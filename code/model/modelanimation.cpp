#include "model/modelanimation.h"
#include "model/modelanimation_segments.h"
#include "ship/ship.h"

extern float flFrametime;

namespace animation {

	std::map<int, std::list<std::shared_ptr<ModelAnimation>>> ModelAnimation::s_runningAnimations;

	std::map<std::pair<int, int>, ModelAnimationData<>> ModelAnimationSubmodel::s_initialData;

	ModelAnimation::ModelAnimation(bool isInitialType) : m_isInitialType(isInitialType) { }

	ModelAnimationState ModelAnimation::play(float frametime, polymodel_instance* pmi, std::map<ModelAnimationSubmodel*, ModelAnimationData<true>>* applyBuffer) {
		instance_data& instanceData = m_instances[pmi->id];

		switch (instanceData.state) {
		case ModelAnimationState::UNTRIGGERED:
			//We have a new animation starting up in this phase. Put it in the list of running animations to track and step it later
			if(!m_isInitialType)
				s_runningAnimations[pmi->id].push_back(shared_from_this());

			instanceData.duration = 0.0f;
			//Stop other running animations on subsystems we care about. Store subsystems initial values as well.
			for (const auto& animation : m_submodelAnimation) {
				//We need to make sure that we have the submodel index cached before we check if other animations have the same index
				animation->findSubmodel(pmi);

				bool haveInitial = false;

				auto animIterList = s_runningAnimations[pmi->id];
				for (auto animIter : animIterList) {
					//Don't stop this animation, even though it (obviously) runs on the same submodels
					if (animIter == shared_from_this())
						continue;

					const auto& otherAnims = animIter->m_submodelAnimation;
	
					//See if we can find a submodel that must be reset. In case the other submodel has not been cached, it cannot have been running, so it is fine to miss these here
					for (const auto& otherAnim : otherAnims) {
						if (otherAnim->m_submodel == animation->m_submodel) {
							haveInitial = true;
							break;
						}
					}
					if (haveInitial)
						break;
				}

				if (!haveInitial) {
					animation->saveCurrentAsBase(pmi);
				}

				//Store the submodels current data as the base for this animation and calculate this animations parameters
				float duration = animation->recalculate(pmi);

				//Update this animations duration based on all the submodel animations (note that these could differ in multiple executions of the same animation, for example in case of absolute angles)
				if (duration > instanceData.duration)
					instanceData.duration = duration;
			}

			instanceData.state = ModelAnimationState::RUNNING_FWD;

			/* fall-thru */
		case ModelAnimationState::RUNNING_FWD:
			instanceData.time += frametime;

			//Cap time if needed
			if (instanceData.time > instanceData.duration) {
				instanceData.time = instanceData.duration;
				instanceData.state = ModelAnimationState::COMPLETED;
			}

			for (const auto& animation : m_submodelAnimation) {
				ModelAnimationData<> base = animation->s_initialData.at({ pmi->id, animation->m_submodel });
				ModelAnimationData<true>& previousDelta = (*applyBuffer)[animation.get()];

				base.applyDelta(previousDelta);
				ModelAnimationData<true> delta = animation->play(instanceData.time, pmi, base);
				previousDelta.applyDelta(delta);
			}
			break;

		case ModelAnimationState::COMPLETED:
			//This means someone requested to start once we were complete, so start moving backwards.
			instanceData.state = ModelAnimationState::RUNNING_RWD;
			/* fall-thru */
		case ModelAnimationState::RUNNING_RWD:
			instanceData.time -= frametime;

			//Cap time at 0, but don't clean up the animations here since this function is called in a loop over the running animations, and cleaning now would invalidate the iterator
			if (instanceData.time < 0) {
				stop(pmi, false);
				break;
			}

			for (const auto& animation : m_submodelAnimation) {
				ModelAnimationData<> base = animation->s_initialData.at({ pmi->id, animation->m_submodel });
				ModelAnimationData<true>& previousDelta = (*applyBuffer)[animation.get()];

				base.applyDelta(previousDelta);
				ModelAnimationData<true> delta = animation->play(instanceData.time, pmi, base);
				previousDelta.applyDelta(delta);
			}

			break;
		}

		return instanceData.state;
	}

	void ModelAnimation::apply(polymodel_instance* pmi, std::map<ModelAnimationSubmodel*, ModelAnimationData<true>>* applyBuffer) {
		for (const auto& toApply : *applyBuffer) {
			ModelAnimationData<> base = ModelAnimationSubmodel::s_initialData.at({ pmi->id, toApply.first->m_submodel });

			base.applyDelta(toApply.second);

			toApply.first->copyToSubmodel(base, pmi);
		}
	}

	void ModelAnimation::cleanRunning() {
		auto removeIt = s_runningAnimations.begin();
		while (removeIt != s_runningAnimations.end()) {
			auto animIt = removeIt->second.cbegin();
			while (animIt != removeIt->second.cend()) {
				if ((*animIt)->m_instances[removeIt->first].state == ModelAnimationState::UNTRIGGERED) {
					animIt = removeIt->second.erase(animIt);
				}
				else
					animIt++;
			}

			if (removeIt->second.empty()) {
				removeIt = s_runningAnimations.erase(removeIt);
			}
			else
				removeIt++;
		}
	}
	
	void ModelAnimation::start(polymodel_instance* pmi, bool reverse, bool force, bool instant) {
		instance_data& instanceData = m_instances[pmi->id];

		if (reverse) {
			switch (instanceData.state) {
			case ModelAnimationState::RUNNING_RWD:
			case ModelAnimationState::UNTRIGGERED:
				//Cannot reverse-start if it's already running rwd or fully untriggered

				//If forced, reset and play anyways
				if (force) {
					//This needs to recalculate first, so start it as if it were going forwards, and then set it's time and continue on
					if (instanceData.state == ModelAnimationState::UNTRIGGERED)
						start(pmi, false, true);
					instanceData.time = instant ? 0 : instanceData.duration;
					instanceData.state = ModelAnimationState::RUNNING_RWD;
					break;
				}

				return;
			case ModelAnimationState::RUNNING_FWD:
				//Just pretend we were going in the right direction
				if (force)
					instanceData.time = instant ? 0 : instanceData.duration;
				instanceData.state = ModelAnimationState::RUNNING_RWD;
				break;
			case ModelAnimationState::COMPLETED:
				//Nothing special to do. Expected case
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
					instanceData.time = instant ? instanceData.duration : 0;
					instanceData.state = ModelAnimationState::RUNNING_FWD;
					break;
				}

				return;
			case ModelAnimationState::RUNNING_RWD:
				//Just pretend we were going in the right direction
				if (force)
					instanceData.time = instant ? instanceData.duration : 0;
				instanceData.state = ModelAnimationState::RUNNING_FWD;
				break;
			case ModelAnimationState::UNTRIGGERED:
				//Nothing special to do. Expected case
				break;
			}
		}

		//Make sure to recalculate the animation here, as otherwise we cannot inquire about things like length after starting.
		//Don't apply just yet if it's a non-initial type, as there might be other animations this'd need to depend upon
		std::map<ModelAnimationSubmodel*, ModelAnimationData<true>> applyBuffer;
		play(0, pmi, &applyBuffer);
		//Since initial types never get stepped, they need to be manually applied here once.
		if (m_isInitialType) {
			apply(pmi, &applyBuffer);
		}
	}

	void ModelAnimation::stop(polymodel_instance* pmi, bool cleanup) {
		//If inaccuracies occur, this needs to reset again, but only if no other animation is running on this submodel
		/*for (const auto& animation : m_submodelAnimation) {
			animation->reset(pmi);
		}*/

		instance_data& instanceData = m_instances[pmi->id];
		instanceData.time = 0.0f;
		instanceData.state = ModelAnimationState::UNTRIGGERED;

		if (cleanup)
			cleanRunning();
	}

	void ModelAnimation::addSubmodelAnimation(std::shared_ptr<ModelAnimationSubmodel> animation) {
		m_submodelAnimation.push_back(std::move(animation));
	}

	void ModelAnimation::stepAnimations(float frametime) {
		for (const auto& animList : s_runningAnimations) {
			std::map<ModelAnimationSubmodel*, ModelAnimationData<true>> applyBuffer;

			for (const auto& anim : animList.second) {
				switch (anim->m_instances[animList.first].state) {
				case ModelAnimationState::RUNNING_FWD:
				case ModelAnimationState::RUNNING_RWD:
					anim->play(frametime, model_get_instance(animList.first), &applyBuffer);
					break;
				case ModelAnimationState::COMPLETED:
					//Fully triggered. Keep in buffer in case some other animation starts on that submodel, but don't play without manual starting
					break;
				case ModelAnimationState::UNTRIGGERED:
					UNREACHABLE("An untriggered animation should not be in the runningAnimations buffer");
					break;
				}

			}

			apply(model_get_instance(animList.first), &applyBuffer);
		}

		//Clear Animations that might have completed this frame.
		cleanRunning();
	}

	void ModelAnimation::clearAnimations() {
		for (const auto& animList : s_runningAnimations) {
			for (const auto& anim : animList.second) {
				anim->stop(model_get_instance(animList.first), false);
			}
		}

		s_runningAnimations.clear();
	}

	void ModelAnimation::parseLegacyAnimationTable(model_subsystem* sp, ship_info* sip) {
		//the only thing initial animation type needs is the angle, 
		//so to save space lets just make everything optional in this case

		//Most of these properties were read and allowed, but never actually used for anything.
		//Hence, still allow them in tables, but now just skip reading them

		required_string("$type:");
		char atype[NAME_LENGTH];
		stuff_string(atype, F_NAME, NAME_LENGTH);
		AnimationTriggerType type = anim_match_type(atype);
		int subtype = ModelAnimationSet::SUBTYPE_DEFAULT;
		char sub_name[NAME_LENGTH];

		if (optional_string("+sub_type:")) {
			stuff_int(&subtype);
		}

		if (optional_string("+sub_name:")) {
			stuff_string(sub_name, F_NAME, NAME_LENGTH);
		}
		else {
			strcpy_s(sub_name, "<none>");
		}

		if (type == AnimationTriggerType::Initial) {
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

			std::shared_ptr<ModelAnimation> anim = std::shared_ptr<ModelAnimation>(new ModelAnimation(true));

			char namelower[MAX_NAME_LEN];
			strncpy(namelower, sp->subobj_name, MAX_NAME_LEN);
			strlwr(namelower);
			//since sp->type is not set without reading the pof, we need to infer it by subsystem name (which works, since the same name is used to match the submodels name, which is used to match the type in pof parsing)
			//sadly, we also need to check for engine and radar, since these take precedent (as in, an engineturret is an engine before a turret type)
			if (!strstr(namelower, "engine") && !strstr(namelower, "radar") && strstr(namelower, "turret")) {
				auto rotBase = std::shared_ptr<ModelAnimationSegmentSetAngle>(new ModelAnimationSegmentSetAngle(angle.h));
				auto subsysBase = std::shared_ptr<ModelAnimationSubmodelTurret>(new ModelAnimationSubmodelTurret(sp->subobj_name, false, sip->name, std::move(rotBase)));
				anim->addSubmodelAnimation(std::move(subsysBase));

				auto rotBarrel = std::shared_ptr<ModelAnimationSegmentSetAngle>(new ModelAnimationSegmentSetAngle(angle.p));
				auto subsysBarrel = std::shared_ptr<ModelAnimationSubmodelTurret>(new ModelAnimationSubmodelTurret(sp->subobj_name, true, sip->name, std::move(rotBarrel)));
				anim->addSubmodelAnimation(std::move(subsysBarrel));
			}
			else {
				auto rot = std::shared_ptr<ModelAnimationSegmentSetPHB>(new ModelAnimationSegmentSetPHB(angle, isRelative));
				auto subsys = std::shared_ptr<ModelAnimationSubmodel>(new ModelAnimationSubmodel(sp->subobj_name, std::move(rot)));
				anim->addSubmodelAnimation(std::move(subsys));
			}

			sip->animations.emplace(anim, anim_name_from_subsys(sp), animation::ModelAnimationTriggerType::Initial);
		}
		else {
			std::shared_ptr<ModelAnimation> anim = std::shared_ptr<ModelAnimation>(new ModelAnimation());
			
			auto mainSegment = std::shared_ptr<ModelAnimationSegmentSerial>(new ModelAnimationSegmentSerial());
			auto moveSegment = std::shared_ptr<ModelAnimationSegmentParallel>(new ModelAnimationSegmentParallel());

			if (optional_string("+delay:")) {
				int delayByMs;
				stuff_int(&delayByMs);
				auto delay = std::shared_ptr<ModelAnimationSegmentWait>(new ModelAnimationSegmentWait(((float)delayByMs) * 0.001f));
				mainSegment->addSegment(delay);
			}

			mainSegment->addSegment(moveSegment);

			if (optional_string("+reverse_delay:")) {
				int delayByMs;
				stuff_int(&delayByMs);
				auto delay = std::shared_ptr<ModelAnimationSegmentWait>(new ModelAnimationSegmentWait(((float)delayByMs) * 0.001f));
				mainSegment->addSegment(delay);
			}

			angles target{ 0,0,0 };
			bool absolute = false;

			if (optional_string("+absolute_angle:")) {
				absolute = true;
				vec3d angle{ 0,0,0 };

				stuff_vec3d(&angle);

				target.p = fl_radians(angle.xyz.x);
				target.b = fl_radians(angle.xyz.y);
				target.h = fl_radians(angle.xyz.z);
			}
			else {
				absolute = false;
				vec3d angle{ 0,0,0 };

				required_string("+relative_angle:");
				stuff_vec3d(&angle);

				target.p = fl_radians(angle.xyz.x);
				target.b = fl_radians(angle.xyz.y);
				target.h = fl_radians(angle.xyz.z);
			}

			angles velocity{ 0,0,0 };
			{
				vec3d vel{ 0,0,0 };
				required_string("+velocity:");
				stuff_vec3d(&vel);
				velocity.p = fl_radians(vel.xyz.x);
				velocity.b = fl_radians(vel.xyz.y);
				velocity.h = fl_radians(vel.xyz.z);
			}

			optional<angles> acceleration;

			if (optional_string("+acceleration:")) {
				angles accel{ 0,0,0 };
				vec3d accelVec{ 0,0,0 };
				stuff_vec3d(&accelVec);
				accel.p = fl_radians(accelVec.xyz.x);
				accel.b = fl_radians(accelVec.xyz.y);
				accel.h = fl_radians(accelVec.xyz.z);
				acceleration = accel;
			}

			if (optional_string("+time:")) {
				skip_token();

				//Time is ignored if acceleration is set in legacy animations. Even if it isn't set, time seems to only affect metadata and not the actual animation.
				//Hence, throw time away, and let the segment handle calculating how long it actually takes
			}

			auto rotation = std::shared_ptr<ModelAnimationSegmentRotation>(new ModelAnimationSegmentRotation(target, velocity, optional<float>(), acceleration, absolute));
			moveSegment->addSegment(rotation);

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

				//TODO Add sound segment
			}

			auto subsys = std::shared_ptr<ModelAnimationSubmodel>(new ModelAnimationSubmodel(sp->subobj_name, std::move(mainSegment)));
			anim->addSubmodelAnimation(subsys);

			//TODO maybe handle sub_name? Not documented in Wiki, maybe no one actually uses it...
			sip->animations.emplace(anim, anim_name_from_subsys(sp), type, subtype);
		}
	}

	/*std::shared_ptr<ModelAnimation> ModelAnimation::parseAnimationTable() {
		//TODO. This is not the function to parse the legacy table, that is still part of modelread.cpp
		//This will eventually be the function to parse the new animation table
		return std::make_shared<ModelAnimation>(nullptr);
	}*/


	ModelAnimationSubmodel::ModelAnimationSubmodel(SCP_string submodelName, std::shared_ptr<ModelAnimationSegment> mainSegment) : m_name(std::move(submodelName)), m_mainSegment(std::move(mainSegment)) { }

	ModelAnimationSubmodel* ModelAnimationSubmodel::copy(const SCP_string& /*newSIPname*/) {
		return new ModelAnimationSubmodel(*this);
	}

	ModelAnimationData<true> ModelAnimationSubmodel::play(float frametime, polymodel_instance* pmi, ModelAnimationData<> base) {
		if (!m_submodel.has())
			findSubmodel(pmi);

		auto dataIt = s_initialData.find({ pmi->id, m_submodel });

		//Specified submodel not found. Don't play
		if (dataIt == s_initialData.end())
			return ModelAnimationData<true>();

		//Cap the frametime at this submodels duration
		if (frametime > m_mainSegment->getDuration(pmi->id))
			frametime = m_mainSegment->getDuration(pmi->id);

		//Calculate the submodels data (or its delta from the data stored at the animation's start)
		ModelAnimationData<true> delta = m_mainSegment->calculateAnimation(base, frametime, pmi->id);
		
		base.applyDelta(delta);

		//Execute stuff of the animation that doesn't modify this delta (stuff like sounds / particles)
		m_mainSegment->executeAnimation(base, frametime, pmi->id);

		return delta;
	}

	void ModelAnimationSubmodel::reset(polymodel_instance* pmi) {
		if(!m_submodel.has())
			findSubmodel(pmi);

		auto dataIt = s_initialData.find({ pmi->id, m_submodel });
		if (dataIt == s_initialData.end())
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

	float ModelAnimationSubmodel::recalculate(polymodel_instance* pmi) {
		auto submodel = findSubmodel(pmi);
		if (!submodel.first || !submodel.second)
			return 0;

		ModelAnimationData<>& data = s_initialData[{ pmi->id, m_submodel }];

		m_mainSegment->recalculate(submodel.first, submodel.second, data, pmi->id);
		return m_mainSegment->getDuration(pmi->id);
	}

	void ModelAnimationSubmodel::saveCurrentAsBase(polymodel_instance* pmi) {
		auto submodel = findSubmodel(pmi);
		if (!submodel.first || !submodel.second)
			return;

		ModelAnimationData<>& data = s_initialData[{ pmi->id, m_submodel }];
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

	ModelAnimationSubmodelTurret::ModelAnimationSubmodelTurret(SCP_string subsystemName, bool findBarrel, SCP_string SIPname, std::shared_ptr<ModelAnimationSegment> mainSegment) : ModelAnimationSubmodel(std::move(subsystemName), std::move(mainSegment)), m_SIPname(std::move(SIPname)), m_findBarrel(findBarrel) { }

	ModelAnimationSubmodel* ModelAnimationSubmodelTurret::copy(const SCP_string& newSIPname) {
		auto anim = new ModelAnimationSubmodelTurret(*this);
		anim->m_SIPname = newSIPname;
		return anim;
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

	void ModelAnimationSet::emplace(const std::shared_ptr<ModelAnimation>& animation, const SCP_string& name, ModelAnimationTriggerType type, int subtype) {
		animationSet[{type, subtype}].emplace(name, animation);
	}

	void ModelAnimationSet::changeShipName(const SCP_string& name) {
		decltype(animationSet) newAnimationSet;

		for (const auto& animationTypes : animationSet) {
			auto& newAnimations = newAnimationSet[animationTypes.first];
			for (const auto& oldAnimation : animationTypes.second) {
				std::shared_ptr<ModelAnimation> newAnimation = std::shared_ptr<ModelAnimation>(new ModelAnimation(oldAnimation.second->m_isInitialType));
				for (const auto& submodelAnims : oldAnimation.second->m_submodelAnimation) {
					std::shared_ptr<ModelAnimationSubmodel> animSubmodel = std::shared_ptr<ModelAnimationSubmodel>(submodelAnims->copy(name));
					newAnimation->addSubmodelAnimation(std::move(animSubmodel));
				}
				newAnimations.emplace(oldAnimation.first, newAnimation);
			}
		}

		animationSet = newAnimationSet;
	}

	bool ModelAnimationSet::start(polymodel_instance* pmi, ModelAnimationTriggerType type, SCP_string name, bool reverse, bool forced, bool instant, int subtype) {
		bool started = false;
		auto animations = animationSet.find({ type, subtype });
		if (animations != animationSet.end()) {
			auto namedAnimation = animations->second.find(name);
			if (namedAnimation != animations->second.end()) {
				namedAnimation->second->start(pmi, reverse, forced, instant);
				started = true;
			}
		}

		//Only search for default anims again if these weren't looked for in the first place
		if (subtype == SUBTYPE_DEFAULT)
			return started;

		animations = animationSet.find({ type, SUBTYPE_DEFAULT });
		if (animations != animationSet.end()) {
			auto namedAnimation = animations->second.find(name);
			if (namedAnimation != animations->second.end()) {
				namedAnimation->second->start(pmi, reverse, forced, instant);
				started = true;
			}
		}
		return started;
	}

	bool ModelAnimationSet::startAll(polymodel_instance* pmi, ModelAnimationTriggerType type, bool reverse, bool forced, bool instant, int subtype, bool strict) {
		bool started = false;
		auto animations = animationSet.find({ type, subtype });
		if (animations != animationSet.end()) {
			for (auto& namedAnimation : animations->second) {
				namedAnimation.second->start(pmi, reverse, forced, instant);
				started = true;
			}
		}

		//Only search for default anims again if these weren't looked for in the first place
		if (strict || subtype == SUBTYPE_DEFAULT)
			return started;

		animations = animationSet.find({ type, SUBTYPE_DEFAULT });
		if (animations != animationSet.end()) {
			for (auto& namedAnimation : animations->second) {
				namedAnimation.second->start(pmi, reverse, forced, instant);
				started = true;
			}
		}
		return started;
	}

	int ModelAnimationSet::getTime(polymodel_instance* pmi, ModelAnimationTriggerType type, SCP_string name, int subtype) {
		float duration = 0.0f;

		auto animations = animationSet.find({ type, subtype });
		if (animations != animationSet.end()) {
			auto namedAnimation = animations->second.find(name);
			if (namedAnimation != animations->second.end() && namedAnimation->second->m_instances[pmi->id].state != ModelAnimationState::UNTRIGGERED) {
				duration = namedAnimation->second->m_instances[pmi->id].duration;
			}
		}

		//Only search for default anims again if these weren't looked for in the first place
		if (subtype == SUBTYPE_DEFAULT)
			return (int) (duration * 1000);

		animations = animationSet.find({ type, SUBTYPE_DEFAULT });
		if (animations != animationSet.end()) {
			auto namedAnimation = animations->second.find(name);
			if (namedAnimation != animations->second.end() && namedAnimation->second->m_instances[pmi->id].state != ModelAnimationState::UNTRIGGERED) {
				float localDur = namedAnimation->second->m_instances[pmi->id].duration;
				duration = duration < localDur ? localDur : duration;
			}
		}

		return (int) (duration * 1000);
	}

	int ModelAnimationSet::getTimeAll(polymodel_instance* pmi, ModelAnimationTriggerType type, int subtype, bool strict) {
		float duration = 0.0f;

		auto animations = animationSet.find({ type, subtype });
		if (animations != animationSet.end()) {
			for (const auto& namedAnimation : animations->second) {
				if (namedAnimation.second->m_instances[pmi->id].state == ModelAnimationState::UNTRIGGERED)
					continue;
				float localDur = namedAnimation.second->m_instances[pmi->id].duration;
				duration = duration < localDur ? localDur : duration;
			}
		}

		//Only search for default anims again if these weren't looked for in the first place
		if (strict || subtype == SUBTYPE_DEFAULT)
			return (int) (duration * 1000);

		animations = animationSet.find({ type, SUBTYPE_DEFAULT });
		if (animations != animationSet.end()) {
			for (const auto& namedAnimation : animations->second) {
				if (namedAnimation.second->m_instances[pmi->id].state == ModelAnimationState::UNTRIGGERED)
					continue;
				float localDur = namedAnimation.second->m_instances[pmi->id].duration;
				duration = duration < localDur ? localDur : duration;
			}
		}

		return (int) (duration * 1000);
	}


	void anim_set_initial_states(ship* shipp) {
		ship_info* sip = &Ship_info[shipp->ship_info_index];

		const auto& initialAnims = sip->animations.animationSet[{animation::ModelAnimationTriggerType::Initial, animation::ModelAnimationSet::SUBTYPE_DEFAULT}];

		for (const auto& anim : initialAnims) {
			anim.second->start(model_get_instance(shipp->model_instance_num), false, true);
		}
	}

	const std::map<ModelAnimationTriggerType, const char*> Animation_type_names = {
	{ModelAnimationTriggerType::Initial, "initial"},
	{ModelAnimationTriggerType::Docking_Stage1, "docking-stage-1"},
	{ModelAnimationTriggerType::Docking_Stage2, "docking-stage-2"},
	{ModelAnimationTriggerType::Docking_Stage3, "docking-stage-3"},
	{ModelAnimationTriggerType::Docked, "docked"},
	{ModelAnimationTriggerType::PrimaryBank, "primary-bank"},
	{ModelAnimationTriggerType::SecondaryBank, "secondary-bank"},
	{ModelAnimationTriggerType::DockBayDoor, "fighterbay"},
	{ModelAnimationTriggerType::Afterburner, "afterburner"},
	{ModelAnimationTriggerType::TurretFiring, "turret-firing"},
	{ModelAnimationTriggerType::Scripted, "scripted"},
	{ModelAnimationTriggerType::TurretFired, "turret-fired"}
	};

	ModelAnimationTriggerType anim_match_type(const char* p)
	{
		// standard match
		for (const auto& entry : Animation_type_names) {
			if (!strnicmp(p, entry.second, strlen(entry.second)))
				return entry.first;
		}

		// Goober5000 - misspelling
		if (!strnicmp(p, "inital", 6) || !strnicmp(p, "\"inital\"", 8)) {
			Warning(LOCATION, "Spelling error in table file.  Please change \"inital\" to \"initial\".");
			return ModelAnimationTriggerType::Initial;
		}

		// Goober5000 - deprecation
		if (!strnicmp(p, "docking", 7) || !strnicmp(p, "\"docking\"", 9)) {
			auto docking_string = Animation_type_names.find(ModelAnimationTriggerType::Docking_Stage2)->second;
			mprintf(("The \"docking\" animation type name is deprecated.  Specify \"%s\" instead.\n", docking_string));
			return ModelAnimationTriggerType::Docking_Stage2;
		}
		else if (!strnicmp(p, "primary_bank", 12) || !strnicmp(p, "\"primary_bank\"", 14)) {
			auto pbank_string = Animation_type_names.find(ModelAnimationTriggerType::PrimaryBank)->second;
			mprintf(("The \"primary_bank\" animation type name is deprecated.  Specify \"%s\" instead.\n", pbank_string));
			return ModelAnimationTriggerType::PrimaryBank;
		}
		else if (!strnicmp(p, "secondary_bank", 14) || !strnicmp(p, "\"secondary_bank\"", 16)) {
			auto sbank_string = Animation_type_names.find(ModelAnimationTriggerType::SecondaryBank)->second;
			mprintf(("The \"secondary_bank\" animation type name is deprecated.  Specify \"%s\" instead.\n", sbank_string));
			return ModelAnimationTriggerType::SecondaryBank;
		}
		else if (!strnicmp(p, "door", 4) || !strnicmp(p, "\"door\"", 6)) {
			auto docking_string = Animation_type_names.find(ModelAnimationTriggerType::DockBayDoor)->second;
			mprintf(("The \"door\" animation type name is deprecated.  Specify \"%s\" instead.\n", docking_string));
			return ModelAnimationTriggerType::DockBayDoor;
		}
		else if (!strnicmp(p, "turret firing", 13) || !strnicmp(p, "\"turret firing\"", 15)) {
			auto turret_string = Animation_type_names.find(ModelAnimationTriggerType::TurretFiring)->second;
			mprintf(("The \"turret firing\" animation type name is deprecated.  Specify \"%s\" instead.\n", turret_string));
			return ModelAnimationTriggerType::TurretFiring;
		}

		// Goober5000 - with quotes
		for (const auto& entry : Animation_type_names) {
			char quoted_name[NAME_LENGTH + 2];
			strcpy(quoted_name, "\"");
			strcat(quoted_name, entry.second);
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
}