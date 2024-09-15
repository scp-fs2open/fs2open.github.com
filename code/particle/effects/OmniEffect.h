#pragma once

#include "globalincs/pstypes.h"
#include "particle/ParticleEffect.h"
#include "particle/ParticleManager.h"
#include "particle/ParticleVolume.h"
#include "particle/util/ParticleProperties.h"
#include "particle/util/EffectTiming.h"
#include "utils/RandomRange.h"

namespace particle {
namespace effects {
/**
 * @ingroup particleEffects
 */
class OmniParticleEffect: public ParticleEffect {
public:
	enum class ShapeDirection {
		Aligned,
		HitNormal,
		Reflected,
		Reverse
	};

	enum class VelocityScaling {
		NONE,
		DOT,
		DOT_INVERSE
	};
private:

	util::EffectTiming m_timing;

	util::ParticleProperties m_particleProperties;

	::util::ParsedRandomFloatRange m_vel_inherit;

	::util::ParsedRandomFloatRange m_particleNum;

	ShapeDirection m_direction = ShapeDirection::Aligned;

	::util::ParsedRandomFloatRange m_velocity;

	VelocityScaling m_velocity_scaling;

	ParticleEffectHandle m_particleTrail = ParticleEffectHandle::invalid();

	tl::optional<::util::ParsedRandomFloatRange> m_vel_inherit_from_position;

	std::unique_ptr<::particle::ParticleVolume> m_spawnVolume;

	std::unique_ptr<::particle::ParticleVolume> m_velocityVolume;

	//Bad legacy flags. Get rid off, or at least don't expose in new table.
	float m_particleChance;

	bool m_affectedByDetail;

	float m_distanceCulled = -1.f;

	vec3d getNewDirection(const ParticleSource* source) const;

public:

	explicit OmniParticleEffect(const SCP_string& name);

	bool processSource(ParticleSource* source) override;

	void parseValues(bool nocreate) override;

	void pageIn() override;

	void initializeSource(ParticleSource& source) override;

	EffectType getType() const override { return EffectType::Single; }

};
}
}