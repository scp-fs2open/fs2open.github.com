#include "EffectHostTurret.h"

#include "math/vecmat.h"
#include "model/model.h"
#include "ship/ship.h"

EffectHostTurret::EffectHostTurret(object* objp, int submodel, int fire_pnt, matrix orientationOverride, bool orientationOverrideRelative) :
	EffectHost(orientationOverride, orientationOverrideRelative), m_objnum(OBJ_INDEX(objp)), m_objsig(objp->signature), m_submodel(submodel), m_fire_pnt(fire_pnt) {}

std::pair<vec3d, matrix> EffectHostTurret::getPositionAndOrientation(bool relativeToParent, float interp, const std::optional<vec3d>& tabled_offset) const {
	ship& shipp = Ships[Objects[m_objnum].instance];
	const polymodel* pm = model_get(Ship_info[shipp.ship_info_index].model_num);
	const polymodel_instance* pmi = model_get_instance(shipp.model_instance_num);

	vec3d gvec, pos;
	model_subsystem *tp = ship_get_indexed_subsys(&shipp, pm->submodel[m_submodel].subsys_num)->system_info;
	vec3d* gun_pos = &tp->turret_firing_point[m_fire_pnt % tp->turret_num_firing_points];
	const matrix& gun_frame_of_reference = pm->submodel[m_submodel].frame_of_reference;

	matrix orientation;
	if (!relativeToParent) {
		//Conversion into global space is required
		vec3d global_pos;
		vm_vec_linear_interpolate(&global_pos, &Objects[m_objnum].pos, &Objects[m_objnum].last_pos, interp);
		//Possible future improvement would be to also interp the orientation here, but that may be expensive for little visual benefit.

		model_instance_local_to_global_point_dir(&pos, &gvec, gun_pos, &tp->turret_norm, pm, pmi, tp->turret_gun_sobj,
												 &Objects[m_objnum].orient, &global_pos);

		if (m_orientationOverrideRelative) {
			vm_vector_2_matrix_norm(&orientation, &gvec, &gun_frame_of_reference.vec.uvec);
			vm_matrix_x_matrix(&orientation, &orientation, &m_orientationOverride);
		}
		else {
			orientation = m_orientationOverride;
		}
	}
	else {
		//The position data here is in submodel-local space, but we need it in model-local space.
		model_instance_local_to_global_point_dir(&pos, &gvec, gun_pos, &tp->turret_norm, pm, pmi, tp->turret_gun_sobj);

		if (m_orientationOverrideRelative) {
			vm_vector_2_matrix_norm(&orientation, &gvec, &gun_frame_of_reference.vec.uvec);
			vm_matrix_x_matrix(&orientation, &orientation, &m_orientationOverride);
		}
		else {
			matrix global_orient_transpose;
			orientation = m_orientationOverride * *vm_copy_transpose(&global_orient_transpose, &Objects[m_objnum].orient);
		}
	}

	if (tabled_offset) {
		pos += gvec * tabled_offset->xyz.z;
	}

	return { pos, orientation };
}

vec3d EffectHostTurret::getVelocity() const {
	//Possible future improvement might be to calculate the actual submodel velocity, as the collision code does.
	//This'll make velocity-inherit particles behave correctly when on fast-moving submodels.
	//But that's a) expensive and b) different from prior behaviour.
	return Objects[m_objnum].phys_info.vel;
}

std::pair<int, int> EffectHostTurret::getParentObjAndSig() const {
	return { m_objnum, m_objsig };
}

float EffectHostTurret::getHostRadius() const {
	return Objects[m_objnum].radius;
}

bool EffectHostTurret::isValid() const {
	return m_objnum >= 0 && m_submodel >= 0 && Objects[m_objnum].signature == m_objsig && Objects[m_objnum].type == OBJ_SHIP &&
			model_get(Ship_info[Ships[Objects[m_objnum].instance].ship_info_index].model_num)->submodel[m_submodel].subsys_num >= 0;
}