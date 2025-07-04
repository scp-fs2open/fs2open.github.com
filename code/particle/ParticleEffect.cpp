#include <utility>

#include "particle/ParticleEffect.h"
#include "particle/ParticleManager.h"

#include "model/modelrender.h"
#include "render/3d.h"

#include <anl.h>

namespace particle {

ParticleEffect::ParticleEffect(SCP_string name)
	: m_name(std::move(name)),
	  m_duration(Duration::ONETIME),
	  m_rotation_type(RotationType::DEFAULT),
	  m_direction(ShapeDirection::ALIGNED),
	  m_velocity_directional_scaling(VelocityScaling::NONE),
	  m_affectedByDetail(false),
	  m_parentLifetime(false),
	  m_parentScale(false),
	  m_hasLifetime(false),
	  m_parent_local(false),
	  m_keep_anim_length_if_available(false),
	  m_vel_inherit_absolute(false),
	  m_vel_inherit_from_position_absolute(false),
	  m_reverseAnimation(false),
	  m_ignore_velocity_inherit_if_has_parent(false),
	  m_bitmap_list({}),
	  m_bitmap_range(::util::UniformRange<size_t>(0)),
	  m_delayRange(::util::UniformFloatRange(0.0f)),
	  m_durationRange(::util::UniformFloatRange(0.0f)),
	  m_particlesPerSecond(::util::UniformFloatRange(-1.f)),
	  m_particleNum(::util::UniformFloatRange(1.0f)),
	  m_radius(::util::UniformFloatRange(0.0f, 1.0f)),
	  m_lifetime(::util::UniformFloatRange(0.0f)),
	  m_length(::util::UniformFloatRange(0.0f)),
	  m_vel_inherit(::util::UniformFloatRange(0.0f)),
	  m_velocity_scaling(::util::UniformFloatRange(1.0f)),
	  m_velocity_noise_scaling(::util::UniformFloatRange(1.0f)),
	  m_position_noise_scaling(::util::UniformFloatRange(1.0f)),
	  m_vel_inherit_from_orientation(std::nullopt),
	  m_vel_inherit_from_position(std::nullopt),
	  m_velocityVolume(nullptr),
	  m_spawnVolume(nullptr),
	  m_velocityNoise(nullptr),
	  m_spawnNoise(nullptr),
	  m_manual_offset (std::nullopt),
	  m_manual_velocity_offset(std::nullopt),
	  m_particleTrail(ParticleEffectHandle::invalid()),
	  m_size_lifetime_curve(-1),
	  m_vel_lifetime_curve (-1),
	  m_particleChance(1.f),
	  m_distanceCulled(-1.f)
	{}

ParticleEffect::ParticleEffect(SCP_string name,
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
	int bitmap)
	: m_name(std::move(name)),
	  m_duration(duration),
	  m_rotation_type(RotationType::DEFAULT),
	  m_direction(direction),
	  m_velocity_directional_scaling(velocity_directional_scaling),
	  m_affectedByDetail(affectedByDetail),
	  m_parentLifetime(false),
	  m_parentScale(false),
	  m_hasLifetime(true),
	  m_parent_local(parentLocal),
	  m_keep_anim_length_if_available(!disregardAnimationLength),
	  m_vel_inherit_absolute(vel_inherit_absolute),
	  m_vel_inherit_from_position_absolute(velInheritFromPositionAbsolute),
	  m_reverseAnimation(reverseAnimation),
	  m_ignore_velocity_inherit_if_has_parent(ignoreVelocityInheritIfParented),
	  m_bitmap_list({bitmap}),
	  m_bitmap_range(::util::UniformRange<size_t>(0)),
	  m_delayRange(::util::UniformFloatRange(0.0f)),
	  m_durationRange(durationRange),
	  m_particlesPerSecond(particlesPerSecond),
	  m_particleNum(particleNum),
	  m_radius(radius),
	  m_lifetime(lifetime),
	  m_length(::util::UniformFloatRange(0.0f)),
	  m_vel_inherit(vel_inherit),
	  m_velocity_scaling(velocity_scaling),
	  m_velocity_noise_scaling(::util::UniformFloatRange(1.0f)),
	  m_position_noise_scaling(::util::UniformFloatRange(1.0f)),
	  m_vel_inherit_from_orientation(vel_inherit_from_orientation),
	  m_vel_inherit_from_position(vel_inherit_from_position),
	  m_velocityVolume(std::move(velocityVolume)),
	  m_spawnVolume(std::move(spawnVolume)),
	  m_velocityNoise(nullptr),
	  m_spawnNoise(nullptr),
	  m_manual_offset(offsetLocal),
	  m_manual_velocity_offset(velocityOffsetLocal),
	  m_particleTrail(particleTrail),
	  m_size_lifetime_curve(-1),
	  m_vel_lifetime_curve (-1),
	  m_particleChance(particleChance),
	  m_distanceCulled(distanceCulled) {}

float ParticleEffect::getApproximateVisualSize(const vec3d& pos) const {
	float distance_to_eye = vm_vec_dist(&Eye_position, &pos);

	return convert_distance_and_diameter_to_pixel_size(
		distance_to_eye,
		m_radius.avg() * 2.f,
		fl_degrees(g3_get_hfov(Eye_fov)),
		gr_screen.max_h);
}

float ParticleEffect::getCurrentFrequencyMult(decltype(modular_curves_definition)::input_type_t source) const {
	return m_modular_curves.get_output(ParticleEffect::ParticleCurvesOutput::PARTICLE_FREQ_MULT, source);
}

matrix ParticleEffect::getNewDirection(const matrix& hostOrientation, const std::optional<vec3d>& normal) const {
	switch (m_direction) {
		case ShapeDirection::ALIGNED:
			return hostOrientation;
		case ShapeDirection::HIT_NORMAL: {
			if (!normal) {
				return hostOrientation;
			}

			matrix normalOrient;
			vm_vector_2_matrix_norm(&normalOrient, &*normal);

			return normalOrient;
		}
		case ShapeDirection::REFLECTED: {
			if (!normal) {
				return hostOrientation;
			}

			// Compute reflect vector as R = V - 2*(V dot N)*N where N
			// is the normal and V is the incoming direction

			return hostOrientation * vm_matrix_new(1.f - 2.f * normal->xyz.x * normal->xyz.x, -2.f * normal->xyz.x * normal->xyz.y, -2.f * normal->xyz.x * normal->xyz.z,
								 -2.f * normal->xyz.x * normal->xyz.y, 1.f - 2.f * normal->xyz.y * normal->xyz.y, -2.f * normal->xyz.y * normal->xyz.z,
								 -2.f * normal->xyz.x * normal->xyz.z, -2.f * normal->xyz.y * normal->xyz.z, 1.f - 2.f * normal->xyz.z * normal->xyz.z);
		}
		case ShapeDirection::REVERSE: {
			return vm_matrix_new(hostOrientation.vec.rvec * -1.f, hostOrientation.vec.uvec * -1.f, hostOrientation.vec.fvec * -1.f);
		}
		default:
			Error(LOCATION, "Unhandled direction value!");
			return vmd_identity_matrix;
	}
}

void ParticleEffect::sampleNoise(vec3d& noiseTarget, const matrix* orientation, std::pair<anl::CKernel, anl::CInstructionIndex>& noise, decltype(modular_curves_definition)::input_type_t source, ParticleCurvesOutput noiseMult, ParticleCurvesOutput noiseTimeMult, ParticleCurvesOutput noiseSeed) const {
	auto& [kernel, instruction] = noise;
	anl::CNoiseExecutor executor(kernel);
	const auto& color = executor.evaluateColor(
		ParticleSource::getEffectRunningTime(std::forward_as_tuple(std::get<0>(source), std::get<1>(source)))
			* m_modular_curves.get_output(noiseTimeMult, source)
			, m_modular_curves.get_output(noiseSeed, source), instruction);

	vec3d noiseSampleLocal{{{ color.r, color.g, color.b }}};
	noiseSampleLocal *= m_modular_curves.get_output(noiseMult, source);

	vm_vec_unrotate(&noiseTarget, &noiseSampleLocal, orientation);
}

/*
 * In persistent mode (should only ever be used by scripting, really), this function returns pointers to the persistent particles
 * In non-persistent mode, this function returns the multiplier for the next spawn time. This is because the source cannot know about the curve evaluation that is required to get this factor
 *
 * */
template<bool isPersistent>
auto ParticleEffect::processSourceInternal(float interp, const ParticleSource& source, size_t effectNumber, const vec3d& velParent, int parent, int parent_sig, float parentLifetime, float parentRadius, float particle_percent) const {
	using persistentParticlesList = std::conditional_t<isPersistent, SCP_vector<WeakParticlePtr>, bool>;
	persistentParticlesList createdParticles;

	if constexpr (!isPersistent)
		SCP_UNUSED(createdParticles);

	if (m_affectedByDetail){
		if (Detail.num_particles > 0)
			particle_percent *= (0.5f + (0.25f * static_cast<float>(Detail.num_particles - 1)));
		else {
			//Will not emit on current detail settings, but may in the future.
			if constexpr (isPersistent)
				return createdParticles;
			else {
				const auto& [pos, hostOrientation] = source.m_host->getPositionAndOrientation(m_parent_local, interp, m_manual_offset);
				auto modularCurvesInput = std::forward_as_tuple(source, effectNumber, pos);
				return getCurrentFrequencyMult(modularCurvesInput);
			}
		}
	}

	const auto& [pos, hostOrientation] = source.m_host->getPositionAndOrientation(m_parent_local, interp, m_manual_offset);

	vec3d posGlobal = pos;
	if (m_parent_local && parent >= 0) {
		vm_vec_unrotate(&posGlobal, &posGlobal, &Objects[parent].orient);
		vm_vec_add2(&posGlobal, &Objects[parent].pos);
	}

	auto modularCurvesInput = std::forward_as_tuple(source, effectNumber, posGlobal);

	const auto& orientation = getNewDirection(hostOrientation, source.m_normal);

	if (m_distanceCulled > 0.f) {
		float min_dist = 125.0f;
		float dist = vm_vec_dist_quick(&pos, &Eye_position) / m_distanceCulled;
		if (dist > min_dist)
			particle_percent *= min_dist / dist;
	}

	particle_percent *= m_particleChance * m_modular_curves.get_output(ParticleCurvesOutput::PARTICLE_NUM_MULT, modularCurvesInput);
	float radiusMultiplier = m_modular_curves.get_output(ParticleCurvesOutput::RADIUS_MULT, modularCurvesInput);
	float lengthMultiplier = m_modular_curves.get_output(ParticleCurvesOutput::LENGTH_MULT, modularCurvesInput);
	float lifetimeMultiplier = m_modular_curves.get_output(ParticleCurvesOutput::LIFETIME_MULT, modularCurvesInput);
	float velocityVolumeMultiplier = m_modular_curves.get_output(ParticleCurvesOutput::VOLUME_VELOCITY_MULT, modularCurvesInput);
	float inheritVelocityMultiplier = m_modular_curves.get_output(ParticleCurvesOutput::INHERIT_VELOCITY_MULT, modularCurvesInput);
	float positionInheritVelocityMultiplier = m_modular_curves.get_output(ParticleCurvesOutput::POSITION_INHERIT_VELOCITY_MULT, modularCurvesInput);
	float orientationInheritVelocityMultiplier = m_modular_curves.get_output(ParticleCurvesOutput::ORIENTATION_INHERIT_VELOCITY_MULT, modularCurvesInput);

	vec3d velNoise = ZERO_VECTOR;
	if (m_velocityNoise != nullptr) {
		sampleNoise(velNoise, &orientation, *m_velocityNoise, modularCurvesInput, ParticleCurvesOutput::VELOCITY_NOISE_MULT, ParticleCurvesOutput::VELOCITY_NOISE_TIME_MULT, ParticleCurvesOutput::VELOCITY_NOISE_SEED);
		velNoise *= m_velocity_noise_scaling.next();
	}

	vec3d posNoise = ZERO_VECTOR;
	if (m_spawnNoise != nullptr) {
		sampleNoise(posNoise, &orientation, *m_spawnNoise, modularCurvesInput, ParticleCurvesOutput::SPAWN_POSITION_NOISE_MULT, ParticleCurvesOutput::SPAWN_POSITION_NOISE_TIME_MULT, ParticleCurvesOutput::SPAWN_POSITION_NOISE_SEED);
		posNoise *= m_position_noise_scaling.next();
	}

	float num = m_particleNum.next() * particle_percent;
	unsigned int num_spawn;

	if (num >= 1.f){
		num_spawn = static_cast<unsigned int>(num);
	}
	else {
		num_spawn = (num >= frand() ? 1 : 0);
	}

	for (uint i = 0; i < num_spawn; ++i) {
		float particleFraction = static_cast<float>(i) / static_cast<float>(num_spawn);

		particle_info info;

		info.reverse = m_reverseAnimation;
		info.pos = pos;
		info.vel = velParent;

		if (m_parent_local) {
			info.attached_objnum = parent;
			info.attached_sig = parent_sig;
		}
		else {
			info.attached_objnum = -1;
			info.attached_sig = -1;
		}

		if (m_vel_inherit_absolute)
			vm_vec_normalize_safe(&info.vel, true);

		info.vel *= (m_ignore_velocity_inherit_if_has_parent && parent >= 0) ? 0.f : m_vel_inherit.next() * inheritVelocityMultiplier;

		vec3d localVelocity = velNoise;
		vec3d localPos = posNoise;

		if (m_spawnVolume != nullptr) {
			localPos += m_spawnVolume->sampleRandomPoint(orientation, modularCurvesInput, particleFraction);
		}

		if (m_velocityVolume != nullptr) {
			localVelocity += m_velocityVolume->sampleRandomPoint(orientation, modularCurvesInput, particleFraction) * (m_velocity_scaling.next() * velocityVolumeMultiplier);
		}

		if (m_manual_velocity_offset.has_value()) {
			localVelocity += *m_manual_velocity_offset;
		}

		if (m_vel_inherit_from_orientation.has_value()) {
			localVelocity += orientation.vec.fvec * (m_vel_inherit_from_orientation->next() * orientationInheritVelocityMultiplier);
		}

		if (m_vel_inherit_from_position.has_value()) {
			vec3d velFromPos = localPos;
			if (m_vel_inherit_from_position_absolute)
				vm_vec_normalize_safe(&velFromPos);
			localVelocity += velFromPos * (m_vel_inherit_from_position->next() * positionInheritVelocityMultiplier);
		}

		if (m_velocity_directional_scaling != VelocityScaling::NONE) {
			// Scale the vector with a random localVelocity sample and also multiply that with cos(angle between
			// info.vel and sourceDir) That should produce good looking directions where the maximum localVelocity is
			// only achieved when the particle travels directly on the normal/reflect vector
			vec3d normalizedVelocity;
			vm_vec_copy_normalize_safe(&normalizedVelocity, &localVelocity);
			float dot = vm_vec_dot(&normalizedVelocity, &orientation.vec.fvec);
			vm_vec_scale(&localVelocity,
				m_velocity_directional_scaling == VelocityScaling::DOT ? dot : 1.f / std::max(0.001f, dot));
		}

		info.vel += localVelocity;
		info.pos += localPos + info.vel * (interp * f2fl(Frametime));

		info.bitmap = m_bitmap_list[m_bitmap_range.next()];

		if (m_parentScale)
			// if we were spawned by a particle, parentRadius is the parent's radius and m_radius is a factor of that
			info.rad = parentRadius * m_radius.next() * radiusMultiplier;
		else
			info.rad = m_radius.next() * radiusMultiplier;

		info.length = m_length.next() * lengthMultiplier;
		if (m_hasLifetime) {
			if (m_parentLifetime)
				// if we were spawned by a particle, parentLifetime is the parent's remaining liftime and m_lifetime is a factor of that
				info.lifetime = parentLifetime * m_lifetime.next() * lifetimeMultiplier;
			else
				info.lifetime = m_lifetime.next() * lifetimeMultiplier;
			info.lifetime_from_animation = m_keep_anim_length_if_available;
		}
		info.size_lifetime_curve = m_size_lifetime_curve;
		info.vel_lifetime_curve = m_vel_lifetime_curve;

		switch (m_rotation_type) {
			case RotationType::DEFAULT:
				info.use_angle = Randomize_particle_rotation;
				break;
			case RotationType::RANDOM:
				info.use_angle = true;
				break;
			case RotationType::SCREEN_ALIGNED:
				info.use_angle = false;
				break;
			default:
				UNREACHABLE("Rotation type not supported");
		}

		if (m_particleTrail.isValid()) {
			auto part = createPersistent(&info);

			if constexpr (isPersistent)
				createdParticles.push_back(part);

			// There are some possibilities where we can get a null pointer back. Those are very rare but we
			// still shouldn't crash in those circumstances.
			if (!part.expired()) {
				auto trailSource = ParticleManager::get()->createSource(m_particleTrail);
				trailSource->setHost(std::make_unique<EffectHostParticle>(std::move(part)));
				trailSource->finishCreation();
			}
		} else {
			if constexpr (isPersistent){
				auto part = createPersistent(&info);
				createdParticles.push_back(part);
			}
			else {
				// We don't have a trail so we don't need a persistent particle
				create(&info);
			}
		}
	}

	if constexpr (isPersistent)
		return createdParticles;
	else
		return getCurrentFrequencyMult(modularCurvesInput);
}

float ParticleEffect::processSource(float interp, const ParticleSource& source, size_t effectNumber, const vec3d& velParent, int parent, int parent_sig, float parentLifetime, float parentRadius, float particle_percent) const {
	return processSourceInternal<false>(interp, source, effectNumber, velParent, parent, parent_sig, parentLifetime, parentRadius, particle_percent);
}

SCP_vector<WeakParticlePtr> ParticleEffect::processSourcePersistent(float interp, const ParticleSource& source, size_t effectNumber, const vec3d& velParent, int parent, int parent_sig, float parentLifetime, float parentRadius, float particle_percent) const {
	return processSourceInternal<true>(interp, source, effectNumber, velParent, parent, parent_sig, parentLifetime, parentRadius, particle_percent);
}

void ParticleEffect::pageIn() {
	for (int bitmap : m_bitmap_list) {
		bm_page_in_texture(bitmap);
	}
}

std::pair<TIMESTAMP, TIMESTAMP> ParticleEffect::getEffectDuration() const {
	std::pair<TIMESTAMP, TIMESTAMP> timing;
	timing.first = _timestamp(fl2i(m_delayRange.next() * 1000.0f));
	if (m_duration == Duration::ALWAYS)
		timing.second = TIMESTAMP::never();
	else
		timing.second = timestamp_delta(timing.first, fl2i(m_durationRange.next() * 1000.0f));
	return timing;
}

float ParticleEffect::getNextSpawnDelay() const {
	return 1.0f / m_particlesPerSecond.next();
}

}