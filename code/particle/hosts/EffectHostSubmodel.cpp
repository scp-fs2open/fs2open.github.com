#include "EffectHostSubmodel.h"

#include "math/vecmat.h"
#include "model/model.h"
#include "object/object.h"
#include "ship/ship.h"

//Vector hosts can never have a parent, so it'll always return global space
std::pair<vec3d, matrix> EffectHostSubmodel::getPositionAndOrientation(bool relativeToParent, float interp, const tl::optional<vec3d>& tabled_offset) {
	vec3d pos = m_offset;
	if (tabled_offset) {
		pos += *tabled_offset;
	}

	const ship& shipp = Ships[Objects[m_objnum].instance];
	const polymodel* pm = model_get(Ship_info[shipp.ship_info_index].model_num);
	const polymodel_instance* pmi = model_get_instance(shipp.model_instance_num);

	matrix orientation;
	if (!relativeToParent) {
		//Conversion into global space is required
		vec3d global_pos;
		vm_vec_linear_interpolate(&global_pos, &Objects[m_objnum].pos, &Objects[m_objnum].last_pos, interp);
		//Possible future improvement would be to also interp the orientation here, but that may be expensive for little visual benefit.

		if (m_orientationOverrideRelative) {
			model_instance_local_to_global_point_orient(&pos, &orientation, &pos, &m_orientationOverride,
														pm, pmi, m_submodel, &Objects[m_objnum].orient, &global_pos);
		}
		else {
			model_instance_local_to_global_point(&pos, &pos, pm, pmi, m_submodel, &Objects[m_objnum].orient, &global_pos);
			orientation = m_orientationOverride;
		}
	}
	else {
		//The position data here is in submodel-local space, but we need it in model-local space. The orientation here is either global or also submodel-local

		if (m_orientationOverrideRelative) {
			model_instance_local_to_global_point_orient(&pos, &orientation, &pos, &m_orientationOverride, pm, pmi, m_submodel);
		}
		else {
			model_instance_local_to_global_point(&pos, &pos, pm, pmi, m_submodel);

			matrix global_orient_transpose;
			orientation = m_orientationOverride * *vm_copy_transpose(&global_orient_transpose, &Objects[m_objnum].orient);
		}
	}

	return { pos, orientation };
}

vec3d EffectHostSubmodel::getVelocity() {
	//Possible future improvement might be to calculate the actual submodel velocity, as the collision code does.
	//This'll make velocity-inherit particles behave correctly when on fast-moving submodels.
	//But that's a) expensive and b) different from prior behaviour.
	return Objects[m_objnum].phys_info.vel;
}

std::pair<int, int> EffectHostSubmodel::getParentObjAndSig() {
	return { m_objnum, m_objsig };
}

bool EffectHostSubmodel::isValid() {
	return m_objnum >= 0 && m_submodel >= 0 && Objects[m_objnum].signature == m_objsig && Objects[m_objnum].type == OBJ_SHIP;
}