#include "model/modelanimation.h"

extern float flFrametime;

namespace animation {

	std::multimap<ship*, std::shared_ptr<ModelAnimation>> ModelAnimation::s_runningAnimations;

	ModelAnimationState ModelAnimation::play(float frametime, ship* ship) {

		switch (m_state) {
		case ModelAnimationState::UNTRIGGERED:
			s_runningAnimations.emplace(ship, shared_from_this());
			//Stop other running animations on subsystems we care about. Store subsystems initial values as well.
			for (const auto& animation : m_submodelAnimation) {
				auto animIterRange = s_runningAnimations.equal_range(ship);
				for (auto animIter = animIterRange.first; animIter != animIterRange.second; animIter++) {
					//Don't stop this animation
					if (animIter->second == shared_from_this())
						continue;

					const auto& otherAnims = animIter->second->m_submodelAnimation;
					bool needStop = false;
					for (const auto& otherAnim : otherAnims) {
						if (otherAnim->m_submodel == animation->m_submodel) {
							needStop = true;
							break;
						}
					}
					
					if (needStop) {
						animIter->second->stop(animIter->first);
					}
				}

				animation->saveCurrentAsBase(ship);
			}

			m_state = ModelAnimationState::RUNNING_FWD;
		case ModelAnimationState::RUNNING_FWD:
			m_time += frametime;

			if (m_time > m_duration) {
				m_time = m_duration;
				m_state = ModelAnimationState::COMPLETED;
			}

			for (const auto& animation : m_submodelAnimation) {
				animation->play(m_time, ship);
			}
			break;

		case ModelAnimationState::COMPLETED:
			m_state = ModelAnimationState::RUNNING_RWD;
		case ModelAnimationState::RUNNING_RWD:
			m_time -= frametime;

			if (m_time < 0) {
				stop(ship);
				break;
			}

			for (const auto& animation : m_submodelAnimation) {
				animation->play(m_time, ship);
			}

			break;
		}

		return m_state;
	}

	void ModelAnimation::cleanRunning() {
		auto removeIt = s_runningAnimations.cbegin();
		while (removeIt != s_runningAnimations.cend()) {
			if (removeIt->second->m_state == ModelAnimationState::UNTRIGGERED) {
				removeIt = s_runningAnimations.erase(removeIt);
			}
			else
				removeIt++;
		}
	}
	
	void ModelAnimation::start(ship* ship, bool reverse) {
		if (reverse) {
			switch (m_state) {
			case ModelAnimationState::RUNNING_RWD:
			case ModelAnimationState::UNTRIGGERED:
				//Cannot reverse-start if it's already running rwd or fully untriggered
				return;
			case ModelAnimationState::RUNNING_FWD:
				//Just pretend we were going in the right direction
				m_state = ModelAnimationState::RUNNING_RWD;
				break;
			case ModelAnimationState::COMPLETED:
				//Nothing special to do. Expected case
				break;
			}
		}
		else {
			switch (m_state) {
			case ModelAnimationState::RUNNING_FWD:
			case ModelAnimationState::COMPLETED:
				//Cannot start if it's already running fwd or fully triggered
				return;
			case ModelAnimationState::RUNNING_RWD:
				//Just pretend we were going in the right direction
				m_state = ModelAnimationState::RUNNING_FWD;
				break;
			case ModelAnimationState::UNTRIGGERED:
				//Nothing special to do. Expected case
				break;
			}
		}
		
		play(0, ship);
		//In case this stopped some other animations, we need to remove them from the playing buffer
		cleanRunning();
	}

	void ModelAnimation::stop(ship* ship) {
		for (const auto& animation : m_submodelAnimation) {
			animation->reset(ship);
		}

		m_time = 0;
		m_state = ModelAnimationState::UNTRIGGERED;
	}

	void ModelAnimation::addSubsystemAnimation(std::unique_ptr<ModelAnimationSubsystem> animation) {
		m_submodelAnimation.push_back(std::move(animation));
	}

	void ModelAnimation::stepAnimations() {
		for (const auto& anim : s_runningAnimations) {
			switch (anim.second->m_state) {
			case ModelAnimationState::RUNNING_FWD:
			case ModelAnimationState::RUNNING_RWD:
				anim.second->play(flFrametime, anim.first);
				break;
			case ModelAnimationState::COMPLETED:
				//Fully triggered. Keep in buffer in case some other animation starts on that submodel, but don't play without manual starting
				break;
			case ModelAnimationState::UNTRIGGERED:
				UNREACHABLE("An untriggered animation should not be in the runningAnimations buffer");
				break;
			}

		}

		cleanRunning();
	}

	void ModelAnimation::clearAnimations() {
		for (const auto& anim : s_runningAnimations) {
			anim.second->stop(anim.first);
		}

		s_runningAnimations.clear();
	}

	/*std::shared_ptr<ModelAnimation> ModelAnimation::parseAnimationTable() {
		//TODO. This is not the function to parse the legacy table, that is still part of modelread.cpp
		return std::make_shared<ModelAnimation>(nullptr);
	}*/


	ModelAnimationSubsystem::ModelAnimationSubsystem(const SCP_string& submodelName, std::unique_ptr<ModelAnimationSegment> mainSegment) : m_submodelName(submodelName), m_mainSegment(std::move(mainSegment)) { }

	void ModelAnimationSubsystem::play(float frametime, ship* ship) {
		if (frametime > m_mainSegment->getDuration())
			frametime = m_mainSegment->getDuration();

		auto dataIt = m_initialData.find(ship);
		Assertion(dataIt != m_initialData.end(), "Tried to play animation of ship that had no data for a running animation");
		auto lastDataIt = m_lastFrame.find(ship);
		Assertion(lastDataIt != m_lastFrame.end(), "Tried to play animation of ship that had no data for a running animation");

		ModelAnimationData<> currentFrame = dataIt->second;
		ModelAnimationData<true> delta = m_mainSegment->calculateAnimation(currentFrame, lastDataIt->second, frametime);
		
		currentFrame.applyDelta(delta);

		m_mainSegment->executeAnimation(currentFrame, frametime);

		copyToSubsystem(currentFrame, ship);
		m_lastFrame[ship] = currentFrame;
	}

	void ModelAnimationSubsystem::reset(ship* ship) {
		auto dataIt = m_initialData.find(ship);
		Assertion(dataIt != m_initialData.end(), "Tried to reset animation of ship that had no data for a running animation");
		copyToSubsystem(dataIt->second, ship);
	}

	void ModelAnimationSubsystem::copyToSubsystem(const ModelAnimationData<>& data, ship* ship) {
		submodel_instance* submodel = findSubmodel(ship);

		submodel->canonical_orient = data.orientation;
		//TODO: Once translation is a thing
		//m_subsys->submodel_instance_1->offset = data.position;
	}

	void ModelAnimationSubsystem::saveCurrentAsBase(ship* ship) {
		submodel_instance* submodel = findSubmodel(ship);

		m_lastFrame[ship] = m_initialData[ship];
		ModelAnimationData<>& data = m_initialData[ship];
		data.orientation = submodel->canonical_orient;
		//TODO: Once translation is a thing
		//data.position = m_subsys->submodel_instance_1->offset;
		
		m_mainSegment->recalculate(submodel, data);
	}

	submodel_instance* ModelAnimationSubsystem::findSubmodel(ship* ship) {
		int submodelNumber = -1;

		if (m_submodel.has())
			submodelNumber = m_submodel;
		else {
			polymodel* pm = model_get(Ship_info[ship->ship_info_index].model_num);
			for (int i = 0; i < pm->n_models; i++) {
				if (!subsystem_stricmp(pm->submodel[i].name, m_submodelName.c_str())) {
					m_submodel = i;
					submodelNumber = i;
					break;
				}
			}
		}

		Assertion(submodelNumber != -1, "Submodel for animation not found in ship!");
		polymodel_instance* pmi = model_get_instance(ship->model_instance_num);
		return &pmi->submodel[submodelNumber];
	}


	float ModelAnimationSegment::getDuration() const {
		return m_duration;
	}
}