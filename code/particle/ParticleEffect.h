#ifndef PARTICLE_EFFECT_H
#define PARTICLE_EFFECT_H
#pragma once

#include "globalincs/pstypes.h"
#include "particle/ParticleVolume.h"
#include "particle/ParticleSource.h"
#include "utils/RandomRange.h"
#include "utils/id.h"
#include "utils/modular_curves.h"

#include <optional>

class EffectHost;

//Due to parsing shenanigans in weapons, this needs a forward-declare here
int parse_weapon(int, bool, const char*);
namespace scripting::api {
	particle::ParticleEffectHandle getLegacyScriptingParticleEffect(int bitmap, bool reversed);
}

namespace anl {
	class CKernel;
	class CInstructionIndex;
}

namespace particle {

/**
 * @brief Defines a particle effect
 *
 * A particle effect contains all data necessary for spawning the particles from a particle source. A source can't
 * store effect specific data so a particle effect has to be state-less. All additions to particles should be done
 * within this class. This allows all features to be available for all particles.
 *
 * @ingroup particleSystems
 */
class ParticleEffect {
public:
	enum class Duration : uint8_t {
		ONETIME, //!< The effect is active exactly once
		RANGE, //!< The effect is active within a specific time range
		ALWAYS //!< The effect is always active
	};

	enum class ShapeDirection : uint8_t {
		ALIGNED,
		HIT_NORMAL,
		REFLECTED,
		REVERSE
	};

	enum class VelocityScaling : uint8_t {
		NONE,
		DOT,
		DOT_INVERSE
	};

	enum class RotationType : uint8_t {
		DEFAULT,
		RANDOM,
		SCREEN_ALIGNED
	};

	enum class ParticleCurvesOutput : uint8_t {
		PARTICLE_NUM_MULT,
		PARTICLE_FREQ_MULT,
		RADIUS_MULT,
		LENGTH_MULT,
		LIFETIME_MULT,
		VOLUME_VELOCITY_MULT,
		INHERIT_VELOCITY_MULT,
		POSITION_INHERIT_VELOCITY_MULT,
		ORIENTATION_INHERIT_VELOCITY_MULT,
		VELOCITY_NOISE_MULT,
		VELOCITY_NOISE_TIME_MULT,
		VELOCITY_NOISE_SEED,
		SPAWN_POSITION_NOISE_MULT,
		SPAWN_POSITION_NOISE_TIME_MULT,
		SPAWN_POSITION_NOISE_SEED,

		NUM_VALUES
	};

 private:
	friend struct ParticleParse;

	friend int ::parse_weapon(int, bool, const char*);

	friend ParticleEffectHandle scripting::api::getLegacyScriptingParticleEffect(int bitmap, bool reversed);

	SCP_string m_name; //!< The name of this effect

	Duration m_duration;
	RotationType m_rotation_type;
	ShapeDirection m_direction;
	VelocityScaling m_velocity_directional_scaling;

	bool m_affectedByDetail; //Kinda deprecated. Only used by the oldest of legacy effects.
	bool m_parentLifetime;
	bool m_parentScale;
	bool m_hasLifetime;
	bool m_parent_local;
	bool m_keep_anim_length_if_available;
	bool m_vel_inherit_absolute;
	bool m_vel_inherit_from_position_absolute;
	bool m_reverseAnimation;
	bool m_ignore_velocity_inherit_if_has_parent;

	SCP_vector<int> m_bitmap_list;
	::util::UniformRange<size_t> m_bitmap_range;

	::util::ParsedRandomFloatRange m_delayRange;
	::util::ParsedRandomFloatRange m_durationRange;
	::util::ParsedRandomFloatRange m_particlesPerSecond;
	::util::ParsedRandomFloatRange m_particleNum;
	::util::ParsedRandomFloatRange m_radius;
	::util::ParsedRandomFloatRange m_lifetime;
	::util::ParsedRandomFloatRange m_length;
	::util::ParsedRandomFloatRange m_vel_inherit;
	::util::ParsedRandomFloatRange m_velocity_scaling;
	::util::ParsedRandomFloatRange m_velocity_noise_scaling;
	::util::ParsedRandomFloatRange m_position_noise_scaling;

	std::optional<::util::ParsedRandomFloatRange> m_vel_inherit_from_orientation;
	std::optional<::util::ParsedRandomFloatRange> m_vel_inherit_from_position;

	std::shared_ptr<::particle::ParticleVolume> m_velocityVolume;
	std::shared_ptr<::particle::ParticleVolume> m_spawnVolume;

	std::shared_ptr<std::pair<anl::CKernel, anl::CInstructionIndex>> m_velocityNoise;
	std::shared_ptr<std::pair<anl::CKernel, anl::CInstructionIndex>> m_spawnNoise;

	std::optional<vec3d> m_manual_offset;
	std::optional<vec3d> m_manual_velocity_offset;

	ParticleEffectHandle m_particleTrail;

	int m_size_lifetime_curve; //This is a curve of the particle, not of the particle effect, as such, it should not be part of the curve set
	int m_vel_lifetime_curve; //This is a curve of the particle, not of the particle effect, as such, it should not be part of the curve set

	float m_particleChance; //Deprecated. Use particle num random ranges instead.
	float m_distanceCulled; //Kinda deprecated. Only used by the oldest of legacy effects.

	matrix getNewDirection(const matrix& hostOrientation, const std::optional<vec3d>& normal) const;

	template<bool isPersistent>
	auto processSourceInternal(float interp, const ParticleSource& source, size_t effectNumber, const vec3d& velParent, int parent, int parent_sig, float parentLifetime, float parentRadius, float particle_percent) const;
  public:
	/**
	 * @brief Initializes the base ParticleEffect
	 * @param name The name this effect should have
	 */
	explicit ParticleEffect(SCP_string name);

	// Use this to recreate deprecated legacy effects from in-engine code.
	// All cases like this should already be handled. You should thus never add new uses of this constructor.
	// This constructor will initialize certain non-parsed values differently to handle legacy effects correctly.
	// Parsing the deprecated -part.tbm effects uses the simple constructor + parseLegacy() instead!
	explicit ParticleEffect(SCP_string name,
							::util::ParsedRandomFloatRange particleNum,
							Duration duration,
							::util::ParsedRandomFloatRange durationRange,
							::util::ParsedRandomFloatRange particlesPerSecond,
							ShapeDirection direction,
							::util::ParsedRandomFloatRange vel_inherit,
							bool vel_inherit_absolute,
							std::shared_ptr<::particle::ParticleVolume> velocityVolume,
							::util::ParsedRandomFloatRange velocity_scaling,
							VelocityScaling velocity_directional_scaling,
							std::optional<::util::ParsedRandomFloatRange> vel_inherit_from_orientation,
							std::optional<::util::ParsedRandomFloatRange> vel_inherit_from_position,
							std::shared_ptr<::particle::ParticleVolume> spawnVolume,
							ParticleEffectHandle particleTrail,
							float particleChance,
							bool affectedByDetail,
							float distanceCulled,
							bool disregardAnimationLength,
							bool reverseAnimation,
							bool parentLocal,
							bool ignoreVelocityInheritIfParented,
							bool velInheritFromPositionAbsolute,
							std::optional<vec3d> velocityOffsetLocal,
							std::optional<vec3d> offsetLocal,
							::util::ParsedRandomFloatRange lifetime,
							::util::ParsedRandomFloatRange radius,
							int bitmap
	);

	float processSource(float interp, const ParticleSource& host, size_t effectNumber, const vec3d& vel, int parent, int parent_sig, float parentLifetime, float parentRadius, float particle_percent) const;
	SCP_vector<WeakParticlePtr> processSourcePersistent(float interp, const ParticleSource& host, size_t effectNumber, const vec3d& vel, int parent, int parent_sig, float parentLifetime, float parentRadius, float particle_percent) const;

	void pageIn();

	const SCP_string& getName() const { return m_name; }

	std::pair<TIMESTAMP, TIMESTAMP> getEffectDuration() const;

	float getNextSpawnDelay() const;

	bool isOnetime() const { return m_duration == Duration::ONETIME; }

	float getApproximateVisualSize(const vec3d& pos) const;

	constexpr static auto modular_curves_definition = make_modular_curve_definition<ParticleSource, ParticleCurvesOutput>(
		std::array {
			std::pair {"Particle Number Mult", ParticleCurvesOutput::PARTICLE_NUM_MULT},
			std::pair {"Particle Frequency Mult", ParticleCurvesOutput::PARTICLE_FREQ_MULT},
			std::pair {"Radius Mult", ParticleCurvesOutput::RADIUS_MULT},
			std::pair {"Length Mult", ParticleCurvesOutput::LENGTH_MULT},
			std::pair {"Lifetime Mult", ParticleCurvesOutput::LIFETIME_MULT},
			std::pair {"Velocity Volume Mult", ParticleCurvesOutput::VOLUME_VELOCITY_MULT},
			std::pair {"Velocity Inherit Mult", ParticleCurvesOutput::INHERIT_VELOCITY_MULT},
			std::pair {"Velocity Position Inherit Mult", ParticleCurvesOutput::POSITION_INHERIT_VELOCITY_MULT},
			std::pair {"Velocity Orientation Inherit Mult", ParticleCurvesOutput::ORIENTATION_INHERIT_VELOCITY_MULT},
			std::pair {"Velocity Noise Mult", ParticleCurvesOutput::VELOCITY_NOISE_MULT},
			std::pair {"Velocity Noise Time Mult", ParticleCurvesOutput::VELOCITY_NOISE_TIME_MULT},
			std::pair {"Velocity Noise Seed", ParticleCurvesOutput::VELOCITY_NOISE_SEED},
			std::pair {"Spawn Position Noise Mult", ParticleCurvesOutput::SPAWN_POSITION_NOISE_MULT},
			std::pair {"Spawn Position Noise Time Mult", ParticleCurvesOutput::SPAWN_POSITION_NOISE_TIME_MULT},
			std::pair {"Spawn Position Noise Seed", ParticleCurvesOutput::SPAWN_POSITION_NOISE_SEED}
		},
		std::pair {"Trigger Radius", modular_curves_submember_input<&ParticleSource::m_triggerRadius>{}},
		std::pair {"Trigger Velocity", modular_curves_submember_input<&ParticleSource::m_triggerVelocity>{}},
		std::pair {"Host Radius", modular_curves_submember_input<&ParticleSource::m_host, &EffectHost::getHostRadius>{}},
		std::pair {"Host Velocity", modular_curves_submember_input<&ParticleSource::m_host, &EffectHost::getVelocityMagnitude>{}},
		//TODO Long term, this should have access to a lot of interesting host properties, especially also those that change during gameplay like current hitpoints
		std::pair {"Effects Running", modular_curves_math_input<
		    modular_curves_submember_input<&ParticleSource::m_effect_is_running, &decltype(ParticleSource::m_effect_is_running)::count>,
			modular_curves_submember_input<&ParticleSource::getEffect, &SCP_vector<ParticleEffect>::size>,
			ModularCurvesMathOperators::division>{}})
	.derive_modular_curves_input_only_subset<size_t>( //Effect Number
		std::pair {"Spawntime Left", modular_curves_functional_full_input<&ParticleSource::getEffectRemainingTime>{}},
		std::pair {"Time Running", modular_curves_functional_full_input<&ParticleSource::getEffectRunningTime>{}})
	.derive_modular_curves_input_only_subset<vec3d>( //Sampled spawn position
		std::pair {"Apparent Visual Size At Emitter", modular_curves_functional_full_input<&ParticleSource::getEffectVisualSize>{}}
		);

	MODULAR_CURVE_SET(m_modular_curves, modular_curves_definition);

  private:
	float getCurrentFrequencyMult(decltype(modular_curves_definition)::input_type_t source) const;
	void sampleNoise(vec3d& noiseTarget, const matrix* orientation, std::pair<anl::CKernel, anl::CInstructionIndex>& noise, decltype(modular_curves_definition)::input_type_t source, ParticleCurvesOutput noiseMult, ParticleCurvesOutput noiseTimeMult, ParticleCurvesOutput noiseSeed) const;
};

}


#endif // PARTICLE_EFFECT_H

