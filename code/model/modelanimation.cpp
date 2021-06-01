#include "model/modelanimation.h"

extern float flFrametime;

namespace animation {

	std::multimap<ship*, std::shared_ptr<ModelAnimation>> ModelAnimation::s_runningAnimations;

	ModelAnimationState ModelAnimation::play(float frametime, ship* ship) {

		switch (m_state) {
		case ModelAnimationState::UNTRIGGERED:
			//Stop other running animations on subsystems we care about. Store subsystems initial values as well.
			for (size_t i = 0; i < m_submodelAnimation.size(); i++) {
				for (auto animIter = s_runningAnimations.cbegin(); animIter != s_runningAnimations.cend(); animIter++) {
					//Don't stop this animation
					if (animIter->second == shared_from_this())
						continue;

					if (animIter->first != ship)
						continue;

					const auto& otherAnims = animIter->second->m_submodelAnimation;
					bool needStop = false;
					for (size_t j = 0; j < otherAnims.size(); j++) {
						if (otherAnims[j]->m_subsysStatic == m_submodelAnimation[i]->m_subsysStatic) {
							needStop = true;
							break;
						}
					}
					
					if (needStop) {
						animIter->second->stop(animIter->first);
					}
				}

				m_submodelAnimation[i]->saveCurrentAsBase(m_submodelAnimation[i]->findSubsys(ship));
			}

			m_state = ModelAnimationState::RUNNING_FWD;
		case ModelAnimationState::RUNNING_FWD:
			m_time += frametime;

			if (m_time > m_duration) {
				m_time = m_duration;
				m_state = ModelAnimationState::TRIGGERED;
			}

			for (size_t i = 0; i < m_submodelAnimation.size(); i++) {
				m_submodelAnimation[i]->play(m_time, m_submodelAnimation[i]->findSubsys(ship));
			}
			break;

		case ModelAnimationState::TRIGGERED:
			m_state = ModelAnimationState::RUNNING_RWD;
		case ModelAnimationState::RUNNING_RWD:
			m_time -= frametime;

			if (m_time < 0) {
				stop(ship);
				break;
			}

			for (size_t i = 0; i < m_submodelAnimation.size(); i++) {
				m_submodelAnimation[i]->play(m_time, m_submodelAnimation[i]->findSubsys(ship));
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
	
	void ModelAnimation::start(ship* ship) {
		s_runningAnimations.emplace(ship, shared_from_this());
		play(0, ship);
		cleanRunning();
	}

	void ModelAnimation::stop(ship* ship) {
		for (size_t i = 0; i < m_submodelAnimation.size(); i++) {
			m_submodelAnimation[i]->reset(m_submodelAnimation[i]->findSubsys(ship));
		}

		m_time = 0;
		m_state = ModelAnimationState::UNTRIGGERED;
	}

	void ModelAnimation::addSubsystemAnimation(std::unique_ptr<ModelAnimationSubsystem> animation) {
		m_submodelAnimation.push_back(std::move(animation));
	}

	void ModelAnimation::stepAnimations() {
		for (const auto& anim : s_runningAnimations) {
			anim.second->play(flFrametime, anim.first);
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


	ModelAnimationSubsystem::ModelAnimationSubsystem(model_subsystem* ssp, std::unique_ptr<ModelAnimationSegment> mainSegment) : m_subsysStatic(ssp), m_mainSegment(std::move(mainSegment)) { }

	void ModelAnimationSubsystem::play(float frametime, ship_subsys* subsys) {
		if (frametime > m_mainSegment->getDuration())
			frametime = m_mainSegment->getDuration();

		ModelAnimationData<> currentFrame = m_initialData;
		ModelAnimationData<true> delta = m_mainSegment->calculateAnimation(m_initialData, m_lastFrame, frametime);
		
		currentFrame.applyDelta(delta);

		m_mainSegment->executeAnimation(currentFrame, frametime);

		copyToSubsystem(currentFrame, subsys);
		m_lastFrame = currentFrame;
	}

	void ModelAnimationSubsystem::reset(ship_subsys* subsys) {
		copyToSubsystem(m_initialData, subsys);
	}

	void ModelAnimationSubsystem::copyToSubsystem(const ModelAnimationData<>& data, ship_subsys* subsys) {
		subsys->submodel_instance_1->canonical_orient = data.orientation;
		//TODO: Once translation is a thing
		//m_subsys->submodel_instance_1->offset = data.position;
	}

	void ModelAnimationSubsystem::saveCurrentAsBase(ship_subsys* subsys) {
		m_initialData.orientation = subsys->submodel_instance_1->canonical_orient;
		//TODO: Once translation is a thing
		//m_initialData.position = m_subsys->submodel_instance_1->offset;

		m_lastFrame = m_initialData;
		m_mainSegment->recalculate(subsys, m_initialData);
	}

	ship_subsys* ModelAnimationSubsystem::findSubsys(ship* ship) const {
		ship_subsys* subsys = nullptr;
		for (ship_subsys* pss = GET_FIRST(&ship->subsys_list); pss != END_OF_LIST(&ship->subsys_list); pss = GET_NEXT(pss)) {
			if (pss->system_info == m_subsysStatic) {
				subsys = pss;
				break;
			}
		}
		Assertion(subsys != nullptr, "Subsystem for animation not found in ship!");
		return subsys;
	}


	float ModelAnimationSegment::getDuration() const {
		return m_duration;
	}
}