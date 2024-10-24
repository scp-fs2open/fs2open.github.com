#include "particle/effects/OmniEffect.h"

#include "particle/ParticleManager.h"

#include "particle/ParticleSourceWrapper.h"

#include "weapon/beam.h"
#include "render/3d.h"

namespace particle {

ParticleEffect::ParticleEffect(const SCP_string& name)
	: ParticleEffectLegacy(name),
	  m_timing(),
	  m_particleProperties(),
	  m_particleNum(::util::UniformFloatRange(1.0f)),
	  m_direction(ShapeDirection::ALIGNED),
	  m_vel_inherit(),
	  m_vel_inherit_absolute(false),
	  m_velocityVolume(nullptr),
	  m_velocity_scaling(),
	  m_velocity_directional_scaling(VelocityScaling::NONE),
	  m_vel_inherit_from_orientation(tl::nullopt),
	  m_vel_inherit_from_position(tl::nullopt),
	  m_vel_inherit_from_position_absolute(false),
	  m_spawnVolume(nullptr),
	  m_particleTrail(ParticleEffectHandle::invalid()),
	  m_particleChance(1.f),
	  m_affectedByDetail(false),
	  m_distanceCulled(-1.f)
	{}

ParticleEffect::ParticleEffect(const SCP_string& name,
	::util::ParsedRandomFloatRange particleNum,
	ShapeDirection direction,
	::util::ParsedRandomFloatRange vel_inherit,
	bool vel_inherit_absolute,
	std::shared_ptr<::particle::ParticleVolume> velocityVolume,
	::util::ParsedRandomFloatRange velocity_scaling,
	VelocityScaling velocity_directional_scaling,
	tl::optional<::util::ParsedRandomFloatRange> vel_inherit_from_orientation,
	tl::optional<::util::ParsedRandomFloatRange> vel_inherit_from_position,
	std::shared_ptr<::particle::ParticleVolume> spawnVolume,
	ParticleEffectHandle particleTrail,
	float particleChance,
	bool affectedByDetail,
	float distanceCulled,
	::util::ParsedRandomFloatRange lifetime,
	::util::ParsedRandomFloatRange radius,
	int bitmap)
	: ParticleEffectLegacy(name),
	  m_timing(),
	  m_particleProperties(),
	  m_particleNum(std::move(particleNum)),
	  m_direction(direction),
	  m_vel_inherit(std::move(vel_inherit)),
	  m_vel_inherit_absolute(vel_inherit_absolute),
	  m_velocityVolume(std::move(velocityVolume)),
	  m_velocity_scaling(std::move(velocity_scaling)),
	  m_velocity_directional_scaling(velocity_directional_scaling),
	  m_vel_inherit_from_orientation(std::move(vel_inherit_from_orientation)),
	  m_vel_inherit_from_position(std::move(vel_inherit_from_position)),
	  m_vel_inherit_from_position_absolute(false),
	  m_spawnVolume(std::move(spawnVolume)),
	  m_particleTrail(std::move(particleTrail)),
	  m_particleChance(particleChance),
	  m_affectedByDetail(affectedByDetail),
	  m_distanceCulled(distanceCulled) {
	m_particleProperties.m_lifetime = std::move(lifetime);
	m_particleProperties.m_hasLifetime = true;
	m_particleProperties.m_keep_anim_length_if_available = true;
	m_particleProperties.m_radius = std::move(radius);
	m_particleProperties.m_bitmap_list = {bitmap};
	m_particleProperties.m_bitmap_range = ::util::UniformRange<size_t>(0);
}

//This MUST be refactored into ParticleSourceHost
	vec3d ParticleEffect::getNewDirection(const ParticleSource* source) const {
		switch (m_direction) {
			case ShapeDirection::ALIGNED:
				return source->getOrientation()->getDirectionVector(source->getOrigin(), m_particleProperties.m_parent_local);
			case ShapeDirection::HIT_NORMAL: {
				vec3d normal;
				if (!source->getOrientation()->getNormal(&normal)) {
					return source->getOrientation()->getDirectionVector(source->getOrigin(), m_particleProperties.m_parent_local);
				}

				return normal;
			}
			case ShapeDirection::REFLECTED: {
				vec3d out = source->getOrientation()->getDirectionVector(source->getOrigin(), m_particleProperties.m_parent_local);
				vec3d normal;
				if (!source->getOrientation()->getNormal(&normal)) {
					return out;
				}

				// Compute reflect vector as R = V - 2*(V dot N)*N where N
				// is the normal and V is the incoming direction

				auto dot = 2 * vm_vec_dot(&out, &normal);

				vm_vec_scale(&normal, dot);
				vm_vec_sub(&out, &out, &normal);

				return out;
			}
			case ShapeDirection::REVERSE: {
				vec3d out = source->getOrientation()->getDirectionVector(source->getOrigin(), m_particleProperties.m_parent_local);
				vm_vec_scale(&out, -1.0f);
				return out;
			}
			default:
				Error(LOCATION, "Unhandled direction value!");
				return vmd_zero_vector;
		}
	}

void ParticleEffect::processSource(ParticleSource* source, float interp) const {
	float particle_percent = m_particleChance;

	if (m_affectedByDetail){
		if (Detail.num_particles > 0)
			particle_percent *= (0.5f + (0.25f * static_cast<float>(Detail.num_particles - 1)));
		else
			return; //Will not emit on current detail settings, but may in the future.
	}

	vec3d pe_pos; //TODO actually calc this here and not once per particle in applytoparticleinfo...
	//TODO In general, move _as much_ as possible from here to the source

	if (m_distanceCulled > 0.f) {
		float min_dist = 125.0f;
		float dist = 0.0f;//vm_vec_dist_quick(&pe_pos, &Eye_position) / m_distanceCulled;
		if (dist > min_dist)
			particle_percent *= min_dist / dist;
	}

	vec3d sourceDir = getNewDirection(source);
	matrix sourceDirMatrix;
	vm_vector_2_matrix(&sourceDirMatrix, &sourceDir, nullptr, nullptr);

	if (source->getOrigin()->getType() == SourceOriginType::BEAM) {
		// beam particle numbers are per km
		object* b_obj = source->getOrigin()->getObjectHost();
		float dist = vm_vec_dist(&Beams[b_obj->instance].last_start, &Beams[b_obj->instance].last_shot) / 1000.0f;
		particle_percent *= dist;
	}

	//Start of per-spawn stuff

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

		source->getOrigin()->applyToParticleInfo(info, m_particleProperties.m_parent_local, interp, m_particleProperties.m_manual_offset);

		if (m_vel_inherit_absolute)
			vm_vec_normalize_quick(&info.vel);

		info.vel *= m_vel_inherit.next();

		vec3d velocity = ZERO_VECTOR;
		vec3d localPos = ZERO_VECTOR;

		if (m_spawnVolume != nullptr) {
			localPos += m_spawnVolume->sampleRandomPoint(sourceDirMatrix);
		}

		if (m_velocityVolume != nullptr) {
			velocity += m_velocityVolume->sampleRandomPoint(sourceDirMatrix) * m_velocity_scaling.next();
		}

		if (m_vel_inherit_from_orientation.has_value()) {
			velocity += sourceDir * m_vel_inherit_from_orientation->next();
		}

		if (m_vel_inherit_from_position.has_value()) {
			vec3d velFromPos = localPos;
			if (m_vel_inherit_from_position_absolute)
				vm_vec_normalize_safe(&velFromPos);
			velocity += velFromPos * m_vel_inherit_from_position->next();
		}

		if (m_velocity_directional_scaling != VelocityScaling::NONE) {
			// Scale the vector with a random velocity sample and also multiply that with cos(angle between
			// info.vel and sourceDir) That should produce good looking directions where the maximum velocity is
			// only achieved when the particle travels directly on the normal/reflect vector
			vec3d normalizedVelocity;
			vm_vec_copy_normalize(&normalizedVelocity, &velocity);
			float dot = vm_vec_dot(&normalizedVelocity, &sourceDir);
			vm_vec_scale(&velocity,
				m_velocity_directional_scaling == VelocityScaling::DOT ? dot : 1.f / std::max(0.001f, dot));
		}

		info.pos += localPos;
		info.vel += velocity;

		if (m_particleTrail.isValid()) {
			auto part = m_particleProperties.createPersistentParticle(info);

			// There are some possibilities where we can get a null pointer back. Those are very rare but we
			// still shouldn't crash in those circumstances.
			if (!part.expired()) {
				auto trailSource = ParticleManager::get()->createSource(m_particleTrail);
				trailSource.moveToParticle(part);

				trailSource.finish();
			}
		} else {
			// We don't have a trail so we don't need a persistent particle
			m_particleProperties.createParticle(info);
		}
	}
}

void ParticleEffect::parseValues(bool nocreate) {
	//TODO
};

void ParticleEffect::pageIn() {
	m_particleProperties.pageIn();
}

}