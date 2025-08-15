#include "EffectHostObject.h"

#include "math/vecmat.h"
#include "object/object.h"
#include "weapon/weapon.h"

static inline WeaponState getWeaponStateOrInvalid(const object* objp) {
	switch(objp->type) {
		case OBJ_WEAPON:
			return Weapons[objp->instance].weapon_state;
		case OBJ_BEAM:
			return Beams[objp->instance].weapon_state;
		default:
			return WeaponState::INVALID;
	}
}

EffectHostObject::EffectHostObject(const object* objp, vec3d offset, matrix orientationOverride, bool orientationOverrideRelative) :
	EffectHost(orientationOverride, orientationOverrideRelative), m_offset(offset), m_objnum(OBJ_INDEX(objp)),
	m_objsig(objp->signature), m_weaponState(getWeaponStateOrInvalid(objp)) {}

std::pair<vec3d, matrix> EffectHostObject::getPositionAndOrientation(bool relativeToParent, float interp, const std::optional<vec3d>& tabled_offset) const {
	vec3d pos = m_offset;
	if (tabled_offset) {
		pos += *tabled_offset;
	}

	matrix orientation;
	if (!relativeToParent) {
		//Conversion into global space is required
		vec3d global_pos;
		vm_vec_linear_interpolate(&global_pos, &Objects[m_objnum].pos, &Objects[m_objnum].last_pos, interp);

		vm_vec_unrotate(&pos, &pos, &Objects[m_objnum].orient);
		pos += global_pos;

		//In relative mode, add the override orientation, otherwise just override
		orientation = m_orientationOverrideRelative ? m_orientationOverride * Objects[m_objnum].orient : m_orientationOverride;
	}
	else {
		//Since we're operating in local space, we can take the orientation override at face value if it's relative, but we need to convert it from global to local otherwise.
		//The position is already correct
		matrix global_orient_transpose;
		orientation = m_orientationOverrideRelative ? m_orientationOverride : m_orientationOverride * *vm_copy_transpose(&global_orient_transpose, &Objects[m_objnum].orient);
	}

	return { pos, orientation };
}

vec3d EffectHostObject::getVelocity() const {
	return Objects[m_objnum].phys_info.vel;
}

std::pair<int, int> EffectHostObject::getParentObjAndSig() const {
	return { m_objnum, m_objsig };
}

float EffectHostObject::getHostRadius() const {
	auto objp = &Objects[m_objnum];
	if (objp->type == OBJ_WEAPON) {
		auto wp = &Weapons[objp->instance];
		auto wip = &Weapon_info[wp->weapon_info_index];
		if (wip->render_type == WRT_LASER) {
			return objp->radius * wip->weapon_curves.get_output(weapon_info::WeaponCurveOutputs::LASER_RADIUS_MULT, *wp, &wp->modular_curves_instance);
		}
	}
	return objp->radius;
}

bool EffectHostObject::isValid() const {
	if (m_objnum < 0 || Objects[m_objnum].signature != m_objsig)
		return false;

	if (m_weaponState != WeaponState::INVALID) {
		if (Objects[m_objnum].type == OBJ_BEAM) {
			return Beams[Objects[m_objnum].instance].weapon_state == m_weaponState;
		}
		else if (Objects[m_objnum].type == OBJ_WEAPON) {
			return Weapons[Objects[m_objnum].instance].weapon_state == m_weaponState;
		}
	}
	return true;
}

void EffectHostObject::setupProcessing() {
	if (IS_VEC_NULL(&m_offset)) {
		object* obj = &Objects[m_objnum];

		if (obj->type == OBJ_WEAPON) {
			weapon* wp = &Weapons[obj->instance];
			weapon_info* wip = &Weapon_info[wp->weapon_info_index];

			if (wip->subtype == WP_MISSILE && wip->model_num >= 0) {
				// Now that we are here we know that this is a missile which has no offset set
				// The particles of a missile should be created at its thruster
				polymodel* pm = model_get(wip->model_num);

				if (pm->n_thrusters < 1) {
					return;
				}

				// Only use the first thruster, for multiple thrusters we need more sources
				auto thruster = &pm->thrusters[0];

				if (thruster->num_points < 1) {
					return;
				}

				// Only use the first point in the bank
				auto point = &thruster->points[0];

				model_local_to_global_point(&m_offset, &point->pnt, pm, thruster->submodel_num);
			}
		}
	}
}