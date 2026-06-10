#include "particle/EffectHost.h"

#include "freespace.h"
#include "globalincs/utility.h"
#include "particle/particle.h"

namespace effects {
vec3d attachment_local_pos_to_global(const EffectAttachment& attachment, const vec3d& local_pos, float interp) {
	return std::visit(overloads{
		[&local_pos](const std::monostate&) {
			return local_pos;
		},
		[&local_pos, interp](const effects::attachment_object& obj) {
			if (obj.objnum < 0)
				return local_pos;

			vec3d pos;
			vm_vec_unrotate(&pos, &local_pos, &Objects[obj.objnum].orient);

			vec3d parent_pos;
			vm_vec_linear_interpolate(&parent_pos, &Objects[obj.objnum].pos, &Objects[obj.objnum].last_pos, interp);
			pos += parent_pos;

			return pos;
		},
		[&local_pos, interp](const effects::attachment_particle& parent_part) {
			const auto& parent = parent_part.particle.lock();
			if (!parent)
				return local_pos;
			return local_pos + attachment_local_pos_to_global(parent->attachment, parent->pos, interp)
			       - attachment_local_vel_to_global(parent->attachment, parent->velocity) * interp * flFrametime;
		},
	}, attachment);
}

vec3d attachment_local_vel_to_global(const EffectAttachment& attachment, const vec3d& local_vel) {
	return std::visit(overloads{
		[&local_vel](const std::monostate&) {
			return local_vel;
		},
		[&local_vel](const effects::attachment_object& obj) {
			if (obj.objnum < 0)
				return local_vel;

			vec3d vel;
			vm_vec_unrotate(&vel, &local_vel, &Objects[obj.objnum].orient);
			return vel;
		},
		[&local_vel](const effects::attachment_particle& parent_part) {
			const auto& parent = parent_part.particle.lock();
			if (!parent)
				return local_vel;
			return local_vel + attachment_local_vel_to_global(parent->attachment, parent->velocity);
		},
	}, attachment);
}

vec3d attachment_local_last_pos_to_global(const EffectAttachment& attachment, const vec3d& last_pos) {
	return std::visit(overloads{
		[&last_pos](const std::monostate&) {
			return last_pos;
		},
		[&last_pos](const effects::attachment_object& obj) {
			vec3d pos = last_pos;
			if (obj.objnum >= 0) {
				vm_vec_unrotate(&pos, &pos, &Objects[obj.objnum].last_orient);
				pos += Objects[obj.objnum].last_pos;
			}
			return pos;
		},
		[&last_pos](const effects::attachment_particle& parent_part) {
			const auto& parent = parent_part.particle.lock();
			if (!parent)
				return last_pos;
			return last_pos + attachment_local_pos_to_global(parent->attachment, parent->pos, 0.0f)
			       - attachment_local_vel_to_global(parent->attachment, parent->velocity) * flFrametime;
		},
	}, attachment);
}

bool is_attachment_valid(const EffectAttachment& attachment) {
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
	}, attachment);
}

std::pair<vec3d, matrix> get_attachment_frame(const EffectAttachment& attachment, float interp) {
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
			return get_attachment_frame(parent->attachment, interp);
		},
	}, attachment);
}
}
