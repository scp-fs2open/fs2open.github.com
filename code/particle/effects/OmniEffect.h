#pragma once

#include "globalincs/pstypes.h"
#include "particle/ParticleEffect.h"
#include "particle/ParticleVolume.h"
#include "particle/util/ParticleProperties.h"
#include "particle/util/EffectTiming.h"
#include "utils/RandomRange.h"

namespace particle {
/**
 * @ingroup particleEffects
 */
class ParticleEffect : public ParticleEffectLegacy {
public:
	enum class ShapeDirection {
		ALIGNED,
		HIT_NORMAL,
		REFLECTED,
		REVERSE
	};

	enum class VelocityScaling {
		NONE,
		DOT,
		DOT_INVERSE
	};
private:

	util::EffectTiming m_timing;

	util::ParticleProperties m_particleProperties;

	::util::ParsedRandomFloatRange m_particleNum;

	ShapeDirection m_direction;

	::util::ParsedRandomFloatRange m_vel_inherit;

	bool m_vel_inherit_absolute;

	std::unique_ptr<::particle::ParticleVolume> m_velocityVolume;

	::util::ParsedRandomFloatRange m_velocity_scaling;

	VelocityScaling m_velocity_directional_scaling;

	tl::optional<::util::ParsedRandomFloatRange> m_vel_inherit_from_position;

	std::unique_ptr<::particle::ParticleVolume> m_spawnVolume;

	ParticleEffectHandle m_particleTrail;

	//Bad legacy flags. Get rid off, or at least don't expose in new table.
	float m_particleChance;

	bool m_affectedByDetail;

	float m_distanceCulled;

	vec3d getNewDirection(const ParticleSource* source) const;

public:

	explicit ParticleEffect(const SCP_string& name);

	// Use this to recreate deprecated legacy effects from in-engine code.
	// Parsing the deprecated -part.tbm effects uses the simple constructor + parseLegacy() instead!
	explicit ParticleEffect(const SCP_string& name,
								::util::ParsedRandomFloatRange particleNum,
								ShapeDirection direction,
								::util::ParsedRandomFloatRange vel_inherit,
								bool vel_inherit_absolute,
								std::unique_ptr<::particle::ParticleVolume> velocityVolume,
								::util::ParsedRandomFloatRange velocity_scaling,
								VelocityScaling velocity_directional_scaling,
								tl::optional<::util::ParsedRandomFloatRange> vel_inherit_from_position,
								std::unique_ptr<::particle::ParticleVolume> spawnVolume,
								ParticleEffectHandle particleTrail,
								float particleChance,
								bool affectedByDetail,
								float distanceCulled,
								::util::ParsedRandomFloatRange lifetime,
								::util::ParsedRandomFloatRange radius,
								int bitmap
		);

	bool processSource(ParticleSource* source) const override;

	void parseValues(bool nocreate) override;

	void pageIn() override;

	void initializeSource(ParticleSource& source) override;

	EffectType getType() const override { return EffectType::Single; }

};
}