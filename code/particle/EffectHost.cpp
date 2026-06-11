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
			if (!parent)
				return local_pos;
			if (parent->parent_effect.getParticleEffect().m_parent_is_transitive)
				return parent->attachment.local_pos_to_global(local_pos, interp);
			else {
				const auto& [parent_pos, parent_orient] = get_frame(interp);
				vec3d pos;
				vm_vec_unrotate(&pos, &local_pos, &parent_orient);

				return pos + parent_pos;
			}
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
			if (!parent)
				return local_vel;
			if (parent->parent_effect.getParticleEffect().m_parent_is_transitive)
				return parent->attachment.local_vel_to_global(local_vel);
			else {
				const auto& [parent_pos, parent_orient] = get_frame();
				vec3d vel;
				vm_vec_unrotate(&vel, &local_vel, &parent_orient);

				return vel + parent->attachment.local_vel_to_global(parent->velocity);
			}
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
			if (!parent)
				return last_pos;
			if (parent->parent_effect.getParticleEffect().m_parent_is_transitive)
				return parent->attachment.local_last_pos_to_global(last_pos);
			else {
				const auto& [parent_pos, parent_orient] = get_frame();
				vec3d pos;
				vm_vec_unrotate(&pos, &last_pos, &parent_orient);

				return pos + parent_pos - parent->attachment.local_vel_to_global(parent->velocity) * flFrametime;
			}
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
			return !parent_part.particle.expired() && (parent_part.particle.lock()->parent_effect.getParticleEffect().m_parent_is_transitive && parent_part.particle.lock()->attachment.is_valid());
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
			if (!parent)
				return {ZERO_VECTOR, vmd_identity_matrix};
			if (parent->parent_effect.getParticleEffect().m_parent_is_transitive)
				return parent->attachment.get_frame(interp);
			else {
				auto [parent_pos, orient] = parent->attachment.get_frame(interp);

				vec3d parent_global_orient_local_velocity;
				vm_vec_unrotate(&parent_global_orient_local_velocity, &parent->velocity, &orient);

				return {parent_pos + parent->pos - parent_global_orient_local_velocity * flFrametime * interp, orient};
			}
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
		[](const attachment_particle& parent_part) -> std::optional<attachment_object>  {
			const auto& parent = parent_part.particle.lock();
			const auto& effect = parent->parent_effect.getParticleEffect();
			if (effect.m_parent_is_transitive)
				return parent->attachment.extract_object();
			else
				return std::nullopt;
		},
	}, *this);
}
}
