#include "EffectHostBeam.h"

#include "math/vecmat.h"
#include "weapon/weapon.h"

#include "freespace.h"

EffectHostBeam::EffectHostBeam(object* objp, matrix orientationOverride, bool orientationOverrideRelative) :
	EffectHost(orientationOverride, orientationOverrideRelative), m_objnum(OBJ_INDEX(objp)),
	m_objsig(objp->signature), m_weaponState(Beams[objp->instance].weapon_state) {}

//Beam hosts can never have a parent, so it'll always return global space
std::pair<vec3d, matrix> EffectHostBeam::getPositionAndOrientation(bool /*relativeToParent*/, float /*interp*/, const std::optional<vec3d>& tabled_offset) const {
	const beam& bm = Beams[m_objnum];
	vec3d pos = bm.last_start;

	// weight the random points towards the start linearly
	// proportion along the beam the beam stopped, of its total potential length
	float dist_adjusted = vm_vec_dist(&bm.last_start, &bm.last_shot) / bm.range;
	// randomly sample from the weighted distribution, excluding points beyond the last_shot
	auto t = (1.0f - sqrtf(frand_range(powf(1.f - dist_adjusted, 2.0f), 1.0f)));

	matrix orientation;
	vec3d dir;

	vm_vec_normalized_dir(&dir, &bm.last_shot, &bm.last_start);
	vm_vector_2_matrix_norm(&orientation, &dir);

	pos += (dir * bm.range) * t;

	if (tabled_offset){
		pos += dir * tabled_offset->xyz.z;
	}

	//As there's no sensible uvec in this particle orientation, relative override orientation is not that sensible. Nonetheless, allow it for compatibility, or future orientation-aware particles
	orientation = m_orientationOverrideRelative ? m_orientationOverride * orientation : m_orientationOverride;

	return { pos, orientation };
}

vec3d EffectHostBeam::getVelocity() const {
	const beam& bm = Beams[Objects[m_objnum].instance];
	vec3d vel;
	vm_vec_normalized_dir(&vel, &bm.last_shot, &bm.last_start);
	vm_vec_scale(&vel, Weapon_info[bm.weapon_info_index].max_speed);
	return vel;
}

float EffectHostBeam::getParticleMultiplier() const {
	//beam particle numbers are per km
	object* b_obj = &Objects[m_objnum];
	return vm_vec_dist(&Beams[b_obj->instance].last_start, &Beams[b_obj->instance].last_shot) / 1000.0f;
}

float EffectHostBeam::getHostRadius() const {
	return Beams[Objects[m_objnum].instance].beam_light_width;
}

bool EffectHostBeam::isValid() const {
	return m_objnum >= 0 && Objects[m_objnum].signature == m_objsig && Objects[m_objnum].type == OBJ_BEAM &&
		(m_weaponState == WeaponState::INVALID || Beams[Objects[m_objnum].instance].weapon_state == m_weaponState);
}