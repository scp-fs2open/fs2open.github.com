#include <utility>

#include "particle/ParticleEffect.h"
#include "particle/ParticleManager.h"

#include "render/3d.h"

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
	  m_velocity_scaling(::util::UniformFloatRange(0.0f)),
	  m_vel_inherit_from_orientation(std::nullopt),
	  m_vel_inherit_from_position(std::nullopt),
	  m_velocityVolume(nullptr),
	  m_spawnVolume(nullptr),
	  m_manual_offset (std::nullopt),
	  m_particleTrail(ParticleEffectHandle::invalid()),
	  m_size_lifetime_curve(-1),
	  m_vel_lifetime_curve (-1),
	  m_particleChance(1.f),
	  m_distanceCulled(-1.f)
	{}

ParticleEffect::ParticleEffect(SCP_string name,
	::util::ParsedRandomFloatRange particleNum,
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
	::util::ParsedRandomFloatRange lifetime,
	::util::ParsedRandomFloatRange radius,
	int bitmap)
	: m_name(std::move(name)),
	  m_duration(Duration::ONETIME),
	  m_rotation_type(RotationType::DEFAULT),
	  m_direction(direction),
	  m_velocity_directional_scaling(velocity_directional_scaling),
	  m_affectedByDetail(affectedByDetail),
	  m_parentLifetime(false),
	  m_parentScale(false),
	  m_hasLifetime(true),
	  m_parent_local(false),
	  m_keep_anim_length_if_available(!disregardAnimationLength),
	  m_vel_inherit_absolute(vel_inherit_absolute),
	  m_vel_inherit_from_position_absolute(false),
	  m_bitmap_list({bitmap}),
	  m_bitmap_range(::util::UniformRange<size_t>(0)),
	  m_delayRange(::util::UniformFloatRange(0.0f)),
	  m_durationRange(::util::UniformFloatRange(0.0f)),
	  m_particlesPerSecond(::util::UniformFloatRange(-1.f)),
	  m_particleNum(particleNum),
	  m_radius(radius),
	  m_lifetime(lifetime),
	  m_length(::util::UniformFloatRange(0.0f)),
	  m_vel_inherit(vel_inherit),
	  m_velocity_scaling(velocity_scaling),
	  m_vel_inherit_from_orientation(vel_inherit_from_orientation),
	  m_vel_inherit_from_position(vel_inherit_from_position),
	  m_velocityVolume(std::move(velocityVolume)),
	  m_spawnVolume(std::move(spawnVolume)),
	  m_manual_offset (std::nullopt),
	  m_particleTrail(particleTrail),
	  m_size_lifetime_curve(-1),
	  m_vel_lifetime_curve (-1),
	  m_particleChance(particleChance),
	  m_distanceCulled(distanceCulled) {}

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

			return vm_matrix_new(1.f - 2.f * normal->xyz.x * normal->xyz.x, -2.f * normal->xyz.x * normal->xyz.y, -2.f * normal->xyz.x * normal->xyz.z,
								 -2.f * normal->xyz.x * normal->xyz.y, 1.f - 2.f * normal->xyz.y * normal->xyz.y, -2.f * normal->xyz.y * normal->xyz.z,
								 -2.f * normal->xyz.x * normal->xyz.z, -2.f * normal->xyz.y * normal->xyz.z, 1.f - 2.f * normal->xyz.z * normal->xyz.z)
								 * hostOrientation;
		}
		case ShapeDirection::REVERSE: {
			return vm_matrix_new(hostOrientation.vec.rvec * -1.f, hostOrientation.vec.uvec * -1.f, hostOrientation.vec.fvec * -1.f);
		}
		default:
			Error(LOCATION, "Unhandled direction value!");
			return vmd_identity_matrix;
	}
}

void ParticleEffect::processSource(float interp, const ParticleSource& source, size_t effectNumber, const vec3d& vel, int parent, int parent_sig, float parentLifetime, float parentRadius, float particle_percent) const {
	const auto& [pos, hostOrientation] = source.m_host->getPositionAndOrientation(m_parent_local, interp, m_manual_offset);

	if (m_affectedByDetail){
		if (Detail.num_particles > 0)
			particle_percent *= (0.5f + (0.25f * static_cast<float>(Detail.num_particles - 1)));
		else
			return; //Will not emit on current detail settings, but may in the future.
	}

	if (m_distanceCulled > 0.f) {
		float min_dist = 125.0f;
		float dist = vm_vec_dist_quick(&pos, &Eye_position) / m_distanceCulled;
		if (dist > min_dist)
			particle_percent *= min_dist / dist;
	}

	const auto& orientation = getNewDirection(hostOrientation, source.m_normal);

	auto modularCurvesInput = std::forward_as_tuple(source, effectNumber);
	particle_percent *= m_particleChance * m_modular_curves.get_output(ParticleCurvesOutput::PARTICLE_NUM_MULT, modularCurvesInput);
	float radiusMultiplier = m_modular_curves.get_output(ParticleCurvesOutput::RADIUS_MULT, modularCurvesInput);
	float lengthMultiplier = m_modular_curves.get_output(ParticleCurvesOutput::LENGTH_MULT, modularCurvesInput);
	float lifetimeMultiplier = m_modular_curves.get_output(ParticleCurvesOutput::LIFETIME_MULT, modularCurvesInput);
	float velocityVolumeMultiplier = m_modular_curves.get_output(ParticleCurvesOutput::VOLUME_VELOCITY_MULT, modularCurvesInput);
	float inheritVelocityMultiplier = m_modular_curves.get_output(ParticleCurvesOutput::INHERIT_VELOCITY_MULT, modularCurvesInput);
	float positionInheritVelocityMultiplier = m_modular_curves.get_output(ParticleCurvesOutput::POSITION_INHERIT_VELOCITY_MULT, modularCurvesInput);
	float orientationInheritVelocityMultiplier = m_modular_curves.get_output(ParticleCurvesOutput::ORIENTATION_INHERIT_VELOCITY_MULT, modularCurvesInput);

	float num = m_particleNum.next() * particle_percent;
	unsigned int num_spawn;

	if (num >= 1.f){
		num_spawn = static_cast<unsigned int>(num);
	}
	else {
		num_spawn = (num >= frand() ? 1 : 0);
	}

	for (uint i = 0; i < num_spawn; ++i) {
		particle_info info;

		info.pos = pos;
		info.vel = vel;

		if (m_parent_local) {
			info.attached_objnum = parent;
			info.attached_sig = parent_sig;
		}
		else {
			info.attached_objnum = -1;
			info.attached_sig = -1;
		}

		if (m_vel_inherit_absolute)
			vm_vec_normalize_quick(&info.vel);

		info.vel *= m_vel_inherit.next() * inheritVelocityMultiplier;

		vec3d velocity = ZERO_VECTOR;
		vec3d localPos = ZERO_VECTOR;

		if (m_spawnVolume != nullptr) {
			localPos += m_spawnVolume->sampleRandomPoint(orientation, modularCurvesInput);
		}

		if (m_velocityVolume != nullptr) {
			velocity += m_velocityVolume->sampleRandomPoint(orientation, modularCurvesInput) * (m_velocity_scaling.next() * velocityVolumeMultiplier);
		}

		if (m_vel_inherit_from_orientation.has_value()) {
			velocity += orientation.vec.fvec * (m_vel_inherit_from_orientation->next() * orientationInheritVelocityMultiplier);
		}

		if (m_vel_inherit_from_position.has_value()) {
			vec3d velFromPos = localPos;
			if (m_vel_inherit_from_position_absolute)
				vm_vec_normalize_safe(&velFromPos);
			velocity += velFromPos * (m_vel_inherit_from_position->next() * positionInheritVelocityMultiplier);
		}

		if (m_velocity_directional_scaling != VelocityScaling::NONE) {
			// Scale the vector with a random velocity sample and also multiply that with cos(angle between
			// info.vel and sourceDir) That should produce good looking directions where the maximum velocity is
			// only achieved when the particle travels directly on the normal/reflect vector
			vec3d normalizedVelocity;
			vm_vec_copy_normalize_safe(&normalizedVelocity, &velocity);
			float dot = vm_vec_dot(&normalizedVelocity, &orientation.vec.fvec);
			vm_vec_scale(&velocity,
				m_velocity_directional_scaling == VelocityScaling::DOT ? dot : 1.f / std::max(0.001f, dot));
		}

		info.pos += localPos;
		info.vel += velocity;

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

			// There are some possibilities where we can get a null pointer back. Those are very rare but we
			// still shouldn't crash in those circumstances.
			if (!part.expired()) {
				auto trailSource = ParticleManager::get()->createSource(m_particleTrail);
				trailSource->setHost(std::make_unique<EffectHostParticle>(std::move(part)));
				trailSource->finishCreation();
			}
		} else {
			// We don't have a trail so we don't need a persistent particle
			create(&info);
		}
	}
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