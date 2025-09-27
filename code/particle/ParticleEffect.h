#ifndef PARTICLE_EFFECT_H
#define PARTICLE_EFFECT_H
#pragma once

#include "globalincs/pstypes.h"
#include "globalincs/systemvars.h"
#include "particle/ParticleVolume.h"
#include "particle/ParticleSource.h"
#include "utils/RandomRange.h"
#include "utils/id.h"
#include "utils/modular_curves.h"
#include "graphics/2d.h"

#include "object/objectshield.h"
#include "ship/ship.h"
#include "object/object_instance.h"
#include "hud/hudets.h"

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

	enum class ParticleLifetimeCurvesOutput : uint8_t {
		VELOCITY_MULT,
		RADIUS_MULT,
		LENGTH_MULT,
		ANIM_STATE,
		LIGHT_RADIUS_MULT,
		LIGHT_SOURCE_RADIUS_MULT,
		LIGHT_INTENSITY_MULT,
		LIGHT_R_MULT,
		LIGHT_G_MULT,
		LIGHT_B_MULT,
		LIGHT_CONE_ANGLE_MULT,
		LIGHT_CONE_INNER_ANGLE_MULT,

		NUM_VALUES
	};

	struct LightInformation {
		float light_radius;
		float source_radius;
		float intensity;
		float r, g, b;
		float cone_angle, cone_inner_angle;

		enum class LightSourceMode : uint8_t {
			POINT,
			AS_PARTICLE,
			TO_LAST_POS,
			CONE
		} light_source_mode;

		constexpr LightInformation() : light_radius(0.f), source_radius(0.f), intensity(0.f), r(0.f), g(0.f), b(0.f), cone_angle(0.f), cone_inner_angle(0.f), light_source_mode(LightSourceMode::POINT) {}
	};

 private:
	friend struct ParticleParse;
	friend class ParticleManager;
	friend int ::parse_weapon(int, bool, const char*);
	friend ParticleEffectHandle scripting::api::getLegacyScriptingParticleEffect(int bitmap, bool reversed);
	friend bool move_particle(float frametime, particle* part);

	SCP_string m_name; //!< The name of this effect

	ParticleSubeffectHandle m_self;

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

	std::optional<LightInformation> m_light_source;

	ParticleEffectHandle m_particleTrail;

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

	float getApproximatePixelSize(const vec3d& pos) const;

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
		std::pair {"Effects Running", modular_curves_math_input<
		    modular_curves_submember_input<&ParticleSource::m_effect_is_running, &decltype(ParticleSource::m_effect_is_running)::count>,
			modular_curves_submember_input<&ParticleSource::getEffect, &SCP_vector<ParticleEffect>::size>,
			ModularCurvesMathOperators::division>{}},
		std::pair {"Total Particle Count", modular_curves_global_submember_input<get_particle_count>{}},
		std::pair {"Particle Usage Score", modular_curves_math_input<
		    modular_curves_global_submember_input<get_particle_count>,
			modular_curves_global_submember_input<Detail, &detail_levels::num_particles>,
			ModularCurvesMathOperators::division>{}},
		std::pair {"Nebula Usage Score", modular_curves_math_input<
		    modular_curves_global_submember_input<get_particle_count>,
			modular_curves_global_submember_input<Detail, &detail_levels::nebula_detail>,
			ModularCurvesMathOperators::division>{}},
		std::pair {"Host Object Hitpoints", modular_curves_submember_input<&ParticleSource::m_host, &EffectHost::getParentObjAndSig, 0, &Objects, &object::hull_strength>{}},
		std::pair {"Host Ship Hitpoints Fraction", modular_curves_math_input<
		    modular_curves_submember_input<&ParticleSource::m_host, &EffectHost::getParentObjAndSig, 0, &Objects, &object::hull_strength>,
			modular_curves_submember_input<&ParticleSource::m_host, &EffectHost::getParentObjAndSig, 0, &Objects, &obj_get_instance_maybe<OBJ_SHIP>, &ship::ship_max_hull_strength>,
			ModularCurvesMathOperators::division>{}},
		std::pair {"Host Object Shield", modular_curves_submember_input<&ParticleSource::m_host, &EffectHost::getParentObjAndSig, 0, &Objects, &shield_get_strength>{}},
		std::pair {"Host Ship Shield Fraction", modular_curves_math_input<
		    modular_curves_submember_input<&ParticleSource::m_host, &EffectHost::getParentObjAndSig, 0, &Objects, &shield_get_strength>,
			modular_curves_submember_input<&ParticleSource::m_host, &EffectHost::getParentObjAndSig, 0, &Objects, &obj_get_instance_maybe<OBJ_SHIP>, &ship::ship_max_shield_strength>,
			ModularCurvesMathOperators::division>{}},
		std::pair {"Host Ship AB Fuel Left", modular_curves_submember_input<&ParticleSource::m_host, &EffectHost::getParentObjAndSig, 0, &Objects, &obj_get_instance_maybe<OBJ_SHIP>, &ship::afterburner_fuel>{}},
		std::pair {"Host Ship Countermeasures Left", modular_curves_submember_input<&ParticleSource::m_host, &EffectHost::getParentObjAndSig, 0, &Objects, &obj_get_instance_maybe<OBJ_SHIP>, &ship::cmeasure_count>{}},
		std::pair {"Host Ship Weapon Energy Left", modular_curves_submember_input<&ParticleSource::m_host, &EffectHost::getParentObjAndSig, 0, &Objects, &obj_get_instance_maybe<OBJ_SHIP>, &ship::weapon_energy>{}},
		std::pair {"Host Ship ETS Engines", modular_curves_math_input<modular_curves_submember_input<&ParticleSource::m_host, &EffectHost::getParentObjAndSig, 0, &Objects, &obj_get_instance_maybe<OBJ_SHIP>, &ship::engine_recharge_index>, modular_curves_global_submember_input<MAX_ENERGY_INDEX>, ModularCurvesMathOperators::division>{}},
		std::pair {"Host Ship ETS Shields", modular_curves_math_input<modular_curves_submember_input<&ParticleSource::m_host, &EffectHost::getParentObjAndSig, 0, &Objects, &obj_get_instance_maybe<OBJ_SHIP>, &ship::shield_recharge_index>, modular_curves_global_submember_input<MAX_ENERGY_INDEX>, ModularCurvesMathOperators::division>{}},
		std::pair {"Host Ship ETS Weapons", modular_curves_math_input<modular_curves_submember_input<&ParticleSource::m_host, &EffectHost::getParentObjAndSig, 0, &Objects, &obj_get_instance_maybe<OBJ_SHIP>, &ship::weapon_recharge_index>, modular_curves_global_submember_input<MAX_ENERGY_INDEX>, ModularCurvesMathOperators::division>{}},
		std::pair {"Host Ship EMP Intensity", modular_curves_submember_input<&ParticleSource::m_host, &EffectHost::getParentObjAndSig, 0, &Objects, &obj_get_instance_maybe<OBJ_SHIP>, &ship::emp_intensity>{}},
		std::pair {"Host Ship Time Until Explosion", modular_curves_submember_input<&ParticleSource::m_host, &EffectHost::getParentObjAndSig, 0, &Objects, &obj_get_instance_maybe<OBJ_SHIP>, &ship::final_death_time, static_cast<int (*)(int)>(&timestamp_until)>{}})
	.derive_modular_curves_input_only_subset<size_t>( //Effect Number
		std::pair {"Spawntime Left", modular_curves_functional_full_input<&ParticleSource::getEffectRemainingTime>{}},
		std::pair {"Time Running", modular_curves_functional_full_input<&ParticleSource::getEffectRunningTime>{}})
	.derive_modular_curves_input_only_subset<vec3d>( //Sampled spawn position
		std::pair {"Pixel Size At Emitter", modular_curves_functional_full_input<&ParticleSource::getEffectPixelSize>{}},
		std::pair {"Apparent Size At Emitter", modular_curves_math_input<
			modular_curves_functional_full_input<&ParticleSource::getEffectPixelSize>,
			modular_curves_global_submember_input<gr_screen, &screen::max_w>,
			ModularCurvesMathOperators::division>{}}
		);

	constexpr static auto modular_curves_lifetime_definition = make_modular_curve_definition<particle, ParticleLifetimeCurvesOutput>(
		std::array {
			std::pair {"Radius", ParticleLifetimeCurvesOutput::RADIUS_MULT},
			std::pair {"Velocity", ParticleLifetimeCurvesOutput::VELOCITY_MULT},
			std::pair {"Radius Mult", ParticleLifetimeCurvesOutput::RADIUS_MULT}, // Modern Naming Alias
			std::pair {"Velocity Mult", ParticleLifetimeCurvesOutput::VELOCITY_MULT}, // Modern Naming Alias
			std::pair {"Length Mult", ParticleLifetimeCurvesOutput::LENGTH_MULT},
			std::pair {"Anim State", ParticleLifetimeCurvesOutput::ANIM_STATE},
			std::pair {"Light Radius Mult", ParticleLifetimeCurvesOutput::LIGHT_RADIUS_MULT},
			std::pair {"Light Source Radius Mult", ParticleLifetimeCurvesOutput::LIGHT_SOURCE_RADIUS_MULT},
			std::pair {"Light Intensity Mult", ParticleLifetimeCurvesOutput::LIGHT_INTENSITY_MULT},
			std::pair {"Light R Mult", ParticleLifetimeCurvesOutput::LIGHT_R_MULT},
			std::pair {"Light G Mult", ParticleLifetimeCurvesOutput::LIGHT_G_MULT},
			std::pair {"Light B Mult", ParticleLifetimeCurvesOutput::LIGHT_B_MULT},
			std::pair {"Light Cone Angle Mult", ParticleLifetimeCurvesOutput::LIGHT_CONE_ANGLE_MULT},
			std::pair {"Light Cone Inner Angle Mult", ParticleLifetimeCurvesOutput::LIGHT_CONE_INNER_ANGLE_MULT},
		},
		//Should you ever need to access something from the effect as a modular curve input:
		//std::pair {"", modular_curves_submember_input<&particle::parent_effect, &ParticleSubeffectHandle::getParticleEffect, &ParticleEffect::>{}}
		std::pair {"Age", modular_curves_submember_input<&particle::age>{}},
		std::pair {"Lifetime", modular_curves_math_input<
		     modular_curves_submember_input<&particle::age>,
			 modular_curves_submember_input<&particle::max_life>,
			 ModularCurvesMathOperators::division>{}},
		std::pair {"Radius", modular_curves_submember_input<&particle::radius>{}},
		std::pair {"Velocity", modular_curves_submember_input<&particle::velocity, &vm_vec_mag_quick>{}},
		std::pair {"Parent Object Hitpoints", modular_curves_submember_input<&particle::attached_objnum, &Objects, &object::hull_strength>{}},
		std::pair {"Parent Ship Hitpoints Fraction", modular_curves_math_input<
			modular_curves_submember_input<&particle::attached_objnum, &Objects, &object::hull_strength>,
			modular_curves_submember_input<&particle::attached_objnum, &Objects, &obj_get_instance_maybe<OBJ_SHIP>, &ship::ship_max_hull_strength>,
			ModularCurvesMathOperators::division>{}},
		std::pair {"Parent Object Shield", modular_curves_submember_input<&particle::attached_objnum, &Objects, &shield_get_strength>{}},
		std::pair {"Parent Ship Shield Fraction", modular_curves_math_input<
			modular_curves_submember_input<&particle::attached_objnum, &Objects, &shield_get_strength>,
			modular_curves_submember_input<&particle::attached_objnum, &Objects, &obj_get_instance_maybe<OBJ_SHIP>, &ship::ship_max_shield_strength>,
			ModularCurvesMathOperators::division>{}},
		std::pair {"Parent Ship AB Fuel Left", modular_curves_submember_input<&particle::attached_objnum, &Objects, &obj_get_instance_maybe<OBJ_SHIP>, &ship::afterburner_fuel>{}},
		std::pair {"Parent Ship Countermeasures Left", modular_curves_submember_input<&particle::attached_objnum, &Objects, &obj_get_instance_maybe<OBJ_SHIP>, &ship::cmeasure_count>{}},
		std::pair {"Parent Ship Weapon Energy Left", modular_curves_submember_input<&particle::attached_objnum, &Objects, &obj_get_instance_maybe<OBJ_SHIP>, &ship::weapon_energy>{}},
		std::pair {"Parent Ship ETS Engines", modular_curves_math_input<modular_curves_submember_input<&particle::attached_objnum, &Objects, &obj_get_instance_maybe<OBJ_SHIP>, &ship::engine_recharge_index>, modular_curves_global_submember_input<MAX_ENERGY_INDEX>, ModularCurvesMathOperators::division>{}},
		std::pair {"Parent Ship ETS Shields", modular_curves_math_input<modular_curves_submember_input<&particle::attached_objnum, &Objects, &obj_get_instance_maybe<OBJ_SHIP>, &ship::shield_recharge_index>, modular_curves_global_submember_input<MAX_ENERGY_INDEX>, ModularCurvesMathOperators::division>{}},
		std::pair {"Parent Ship ETS Weapons", modular_curves_math_input<modular_curves_submember_input<&particle::attached_objnum, &Objects, &obj_get_instance_maybe<OBJ_SHIP>, &ship::weapon_recharge_index>, modular_curves_global_submember_input<MAX_ENERGY_INDEX>, ModularCurvesMathOperators::division>{}},
		std::pair {"Parent Ship EMP Intensity",	modular_curves_submember_input<&particle::attached_objnum, &Objects, &obj_get_instance_maybe<OBJ_SHIP>, &ship::emp_intensity>{}},
		std::pair {"Parent Ship Time Until Explosion", modular_curves_submember_input<&particle::attached_objnum, &Objects, &obj_get_instance_maybe<OBJ_SHIP>, &ship::final_death_time, static_cast<int (*)(int)>(&timestamp_until)>{}})
	.derive_modular_curves_input_only_subset<float>(
		std::pair {"Post-Curves Velocity", modular_curves_self_input{}}
		);

	MODULAR_CURVE_SET(m_modular_curves, modular_curves_definition);
	MODULAR_CURVE_SET(m_lifetime_curves, modular_curves_lifetime_definition);

  private:
	float getCurrentFrequencyMult(decltype(modular_curves_definition)::input_type_t source) const;
	void sampleNoise(vec3d& noiseTarget, const matrix* orientation, std::pair<anl::CKernel, anl::CInstructionIndex>& noise, decltype(modular_curves_definition)::input_type_t source, ParticleCurvesOutput noiseMult, ParticleCurvesOutput noiseTimeMult, ParticleCurvesOutput noiseSeed) const;
};

}


#endif // PARTICLE_EFFECT_H

