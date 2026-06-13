#include "particle/EffectHost.h"

#include "freespace.h"
#include "ParticleEffect.h"
#include "globalincs/utility.h"
#include "particle/particle.h"

namespace effects {
vec3d EffectAttachment::local_pos_to_global(const vec3d& local_pos, float interp) const {
	return std::visit(overloads {
		[&local_pos](const std::monostate&) {
			return local_pos;
		},
		[&local_pos, interp](const attachment_object& obj) {
			if (obj.objnum < 0)
				return local_pos;

			vec3d pos;
			vm_vec_unrotate(&pos, &local_pos, &Objects[obj.objnum].orient);

			vec3d parent_pos;
			vm_vec_linear_interpolate(&parent_pos, &Objects[obj.objnum].pos, &Objects[obj.objnum].last_pos, interp);
			pos += parent_pos;

			return pos;
		},
		[&local_pos, interp, this](const attachment_particle& parent_part) {
			const auto& parent = parent_part.particle.lock();
			Assertion(!parent->parent_effect.getParticleEffect().m_parent_is_transitive, "Encountered live transitive parent in effect attachment.");
			if (!parent)
				return local_pos;

			const auto& [parent_pos, parent_orient] = get_frame(interp);
			vec3d pos;
			vm_vec_unrotate(&pos, &local_pos, &parent_orient);

			return pos + parent_pos;
		},
	}, *this);
}

vec3d EffectAttachment::global_to_local(const vec3d& global_pos) const {
	return std::visit(overloads {
		[&global_pos](const std::monostate&) {
			return global_pos;
		},
		[&global_pos](const attachment_object& obj) {
			if (obj.objnum < 0)
				return global_pos;

			vec3d pos = global_pos - Objects[obj.objnum].pos;
			vm_vec_rotate(&pos, &pos, &Objects[obj.objnum].orient);

			return pos;
		},
		[&global_pos, this](const attachment_particle& parent_part) {
			const auto& parent = parent_part.particle.lock();
			Assertion(!parent->parent_effect.getParticleEffect().m_parent_is_transitive, "Encountered live transitive parent in effect attachment.");
			if (!parent)
				return global_pos;

			const auto& [parent_pos, parent_orient] = get_frame();
			vec3d pos = global_pos - parent_pos;
			vm_vec_rotate(&pos, &pos, &parent_orient);

			return pos;
		},
	}, *this);
}

vec3d EffectAttachment::local_vel_to_global(const vec3d& local_vel) const {
	return std::visit(overloads {
		[&local_vel](const std::monostate&) {
			return local_vel;
		},
		[&local_vel](const attachment_object& obj) {
			if (obj.objnum < 0)
				return local_vel;

			vec3d vel;
			vm_vec_unrotate(&vel, &local_vel, &Objects[obj.objnum].orient);
			return vel;
		},
		[&local_vel, this](const attachment_particle& parent_part) {
			const auto& parent = parent_part.particle.lock();
			Assertion(!parent->parent_effect.getParticleEffect().m_parent_is_transitive, "Encountered live transitive parent in effect attachment.");
			if (!parent)
				return local_vel;

			const auto& [parent_pos, parent_orient] = get_frame();
			vec3d vel;
			vm_vec_unrotate(&vel, &local_vel, &parent_orient);

			return vel + parent->attachment.local_vel_to_global(parent->velocity);
		},
	}, *this);
}

vec3d EffectAttachment::global_vel_to_local(const vec3d& global_vel) const {
	return std::visit(overloads {
		[&global_vel](const std::monostate&) {
			return global_vel;
		},
		[&global_vel](const attachment_object& obj) {
			if (obj.objnum < 0)
				return global_vel;

			vec3d vel;
			vm_vec_rotate(&vel, &global_vel, &Objects[obj.objnum].orient);
			return vel;
		},
		[&global_vel, this](const attachment_particle& parent_part) {
			const auto& parent = parent_part.particle.lock();
			Assertion(!parent->parent_effect.getParticleEffect().m_parent_is_transitive, "Encountered live transitive parent in effect attachment.");
			if (!parent)
				return global_vel;

			const auto& [parent_pos, parent_orient] = get_frame();
			vec3d vel;
			vm_vec_rotate(&vel, &global_vel, &parent_orient);

			return vel + parent->attachment.global_vel_to_local(parent->velocity);
		},
	}, *this);
}

vec3d EffectAttachment::local_last_pos_to_global(const vec3d& last_pos) const {
	return std::visit(overloads{
		[&last_pos](const std::monostate&) {
			return last_pos;
		},
		[&last_pos](const attachment_object& obj) {
			vec3d pos = last_pos;
			if (obj.objnum >= 0) {
				vm_vec_unrotate(&pos, &pos, &Objects[obj.objnum].last_orient);
				pos += Objects[obj.objnum].last_pos;
			}
			return pos;
		},
		[&last_pos, this](const attachment_particle& parent_part) {
			const auto& parent = parent_part.particle.lock();
			Assertion(!parent->parent_effect.getParticleEffect().m_parent_is_transitive, "Encountered live transitive parent in effect attachment.");
			if (!parent)
				return last_pos;

			const auto& [parent_pos, parent_orient] = get_frame();
			vec3d pos;
			vm_vec_unrotate(&pos, &last_pos, &parent_orient);

			float vel_scalar = parent->parent_effect.getParticleEffect().m_lifetime_curves.get_output(particle::ParticleEffect::ParticleLifetimeCurvesOutput::VELOCITY_MULT, std::forward_as_tuple(*parent, vm_vec_mag_quick(&parent->velocity)));
			return pos + parent_pos - parent->attachment.local_vel_to_global(parent->velocity) * flFrametime * vel_scalar;
		},
	}, *this);
}

bool EffectAttachment::is_valid() const {
	return std::visit(overloads{
		[](const std::monostate&) {
			return true;
		},
		[](const effects::attachment_object& obj) {
			return obj.objnum >= 0 && obj.objnum < MAX_OBJECTS && Objects[obj.objnum].signature == obj.sig;
		},
		[](const effects::attachment_particle& parent_part) {
			return !parent_part.particle.expired();
		},
	}, *this);
}

std::pair<vec3d, matrix> EffectAttachment::get_frame(float interp) const {
	return std::visit(overloads{
		[](const std::monostate&) -> std::pair<vec3d, matrix> {
			return {ZERO_VECTOR, vmd_identity_matrix};
		},
		[interp](const effects::attachment_object& obj) -> std::pair<vec3d, matrix> {
			if (obj.objnum < 0)
				return {ZERO_VECTOR, vmd_identity_matrix};
			vec3d pos;
			vm_vec_linear_interpolate(&pos, &Objects[obj.objnum].pos, &Objects[obj.objnum].last_pos, interp);
			return {pos, Objects[obj.objnum].orient};
		},
		[interp](const effects::attachment_particle& parent_part) -> std::pair<vec3d, matrix> {
			const auto& parent = parent_part.particle.lock();
			Assertion(!parent->parent_effect.getParticleEffect().m_parent_is_transitive, "Encountered live transitive parent in effect attachment.");
			if (!parent)
				return {ZERO_VECTOR, vmd_identity_matrix};

			auto [parent_pos, orient] = parent->attachment.get_frame(interp);

			vec3d parent_global_orient_local_velocity;
			vm_vec_unrotate(&parent_global_orient_local_velocity, &parent->velocity, &orient);
			float vel_scalar = parent->parent_effect.getParticleEffect().m_lifetime_curves.get_output(particle::ParticleEffect::ParticleLifetimeCurvesOutput::VELOCITY_MULT, std::forward_as_tuple(*parent, vm_vec_mag_quick(&parent->velocity)));

			vec3d pos_rotated;
			vm_vec_unrotate(&pos_rotated, &parent->pos, &orient);

			return {parent_pos + pos_rotated - parent_global_orient_local_velocity * flFrametime * interp * vel_scalar, orient};
		},
	}, *this);
}

std::optional<attachment_object> EffectAttachment::extract_object() const {
	return std::visit(overloads{
		[](const std::monostate&) -> std::optional<attachment_object>  {
			return std::nullopt;
		},
		[](const attachment_object& obj) -> std::optional<attachment_object>  {
			return obj;
		},
		[](const attachment_particle&) -> std::optional<attachment_object>  {
			return std::nullopt;
		},
	}, *this);
}

EffectAttachment EffectAttachment::resolve_true_parent() const {
	return std::visit(overloads{
		[](const std::monostate&) -> EffectAttachment  {
			return std::monostate{};
		},
		[](const attachment_object& obj) -> EffectAttachment  {
			return obj;
		},
		[](const attachment_particle& parent_part) -> EffectAttachment  {
			const auto& parent = parent_part.particle.lock();
			const auto& effect = parent->parent_effect.getParticleEffect();
			if (effect.m_parent_is_transitive)
				return parent->attachment.resolve_true_parent();
			else
				return parent_part;
		},
	}, *this);
}
}
