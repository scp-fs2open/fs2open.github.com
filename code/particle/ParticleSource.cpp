#include <math/bitarray.h>
#include <math/curve.h>
#include "freespace.h"
#include "particle/ParticleSource.h"
#include "weapon/weapon.h"
#include "ParticleSource.h"
#include "ship/ship.h"

namespace particle {
SourceOrigin::SourceOrigin() : m_originType(SourceOriginType::NONE),
							   m_weaponState(WeaponState::INVALID),
							   m_offset(vmd_zero_vector),
	                           m_velocity(vmd_zero_vector) {
}

void SourceOrigin::getGlobalPosition(vec3d* posOut, float interp, tl::optional<vec3d> tabled_offset) const {
	Assertion(posOut != nullptr, "Invalid vector pointer passed!");
	Assertion(m_originType != SourceOriginType::NONE, "Invalid origin type!");

	vec3d offset;
	switch (m_originType) {
		case SourceOriginType::OBJECT: {
			if (interp != 0.0f) {
				vm_vec_linear_interpolate(posOut, &m_origin.m_object.objp()->pos, &m_origin.m_object.objp()->last_pos, interp);
			} else {
				*posOut = m_origin.m_object.objp()->pos;
			}

			// we add whatever offset already exists to the tabled offset specified by the modder
			vec3d combined_offset = m_offset + tabled_offset.value_or(vmd_zero_vector);
			vm_vec_unrotate(&offset, &combined_offset, &m_origin.m_object.objp()->orient);
			break;
		}
		case SourceOriginType::SUBOBJECT: {
			// this interpolation only accounts for movement of the object as a whole, not local subobject movements
			// this could be made more accurate, but it would be a pain and as of now I am not considering it worth it
			if (interp != 0.0f) {
				vm_vec_linear_interpolate(posOut, &m_origin.m_object.objp()->pos, &m_origin.m_object.objp()->last_pos, interp);
			} else {
				*posOut = m_origin.m_object.objp()->pos;
			}

			// we add whatever offset already exists to the tabled offset specified by the modder
			vec3d combined_offset = m_offset + tabled_offset.value_or(vmd_zero_vector);
			model_instance_local_to_global_point(&offset, &combined_offset, Ships[m_origin.m_object.objp()->instance].model_instance_num, m_origin.m_subobject, &m_origin.m_object.objp()->orient, &vmd_zero_vector);

			break;
		}
		case SourceOriginType::TURRET: {
			ship* shipp = &Ships[m_origin.m_object.objp()->instance];

			polymodel_instance *pmi = model_get_instance(Ships[m_origin.m_object.objp()->instance].model_instance_num);
			polymodel *pm = model_get(pmi->model_num);
			ship_subsys* sss = ship_get_indexed_subsys(shipp, pm->submodel[m_origin.m_subobject].subsys_num);

			// this interpolation only accounts for movement of the object as a whole, not local subobject movements like turret rotation
			// this could be made more accurate, but it would be a pain and as of now I am not considering it worth it
			vec3d obj_pos;
			if (interp != 0.0f) {
				vm_vec_linear_interpolate(&obj_pos, &m_origin.m_object.objp()->pos, &m_origin.m_object.objp()->last_pos, interp);
			} else {
				obj_pos = m_origin.m_object.objp()->pos;
			}
			
			vec3d *gun_pos;
			vec3d gvec;
			model_subsystem *tp = sss->system_info;

			gun_pos = &tp->turret_firing_point[m_origin.m_fire_pos % tp->turret_num_firing_points];

			model_instance_local_to_global_point(posOut, gun_pos, pm, pmi, tp->turret_gun_sobj, &m_origin.m_object.objp()->orient, &obj_pos);
			model_instance_local_to_global_dir(&gvec, &tp->turret_norm, pm, pmi, tp->turret_gun_sobj, &m_origin.m_object.objp()->orient);
			
			vm_vec_copy_scale(&offset, &gvec, tabled_offset.value_or(vmd_zero_vector).xyz.z);

			break;
		}
		case SourceOriginType::PARTICLE: {
			auto part = m_origin.m_particle.lock();
			matrix m = vmd_identity_matrix;
			vec3d dir = part->velocity;

			if (interp != 0.0f) {
				float vel_scalar = 1.0f;
				if (part->vel_lifetime_curve >= 0) {
					vel_scalar = Curves[part->vel_lifetime_curve].GetValue(part->age / part->max_life);
				}
				vec3d pos_current = part->pos;
				vec3d pos_last = pos_current - (dir * vel_scalar * flFrametime);
				vm_vec_linear_interpolate(posOut, &pos_current, &pos_last, interp);
			} else {
				*posOut = part->pos;
			}

			vm_vec_normalize_safe(&dir);
			vm_vector_2_matrix_norm(&m, &dir);

			// we add whatever offset already exists to the tabled offset specified by the modder
			vec3d combined_offset = m_offset + tabled_offset.value_or(vmd_zero_vector);
			vm_vec_unrotate(&offset, &combined_offset, &m);

			break;
		}
		case SourceOriginType::VECTOR: {
			*posOut = m_origin.m_pos;

			matrix m = vmd_identity_matrix;
			vec3d dir = m_velocity;

			vm_vec_normalize_safe(&dir);
			vm_vector_2_matrix_norm(&m, &dir);

			// we add whatever offset already exists to the tabled offset specified by the modder
			vec3d combined_offset = m_offset + tabled_offset.value_or(vmd_zero_vector);
			vm_vec_unrotate(&offset, &combined_offset, &m);

			break;
		}
		case SourceOriginType::BEAM: {
			auto beam = &Beams[m_origin.m_object.objp()->instance];
			*posOut = beam->last_start;
			// weight the random points towards the start linearly
			// proportion along the beam the beam stopped, of its total potential length
			float dist_adjusted = vm_vec_dist(&beam->last_start, &beam->last_shot) / beam->range;
			// randomly sample from the weighted distribution, excluding points beyond the last_shot
			auto t = (1.0f - sqrtf(frand_range(powf(1.f - dist_adjusted, 2.0f), 1.0f)));

			matrix m = vmd_identity_matrix;
			vec3d dir;

			vm_vec_normalized_dir(&dir, &beam->last_shot, &beam->last_start);
			vm_vector_2_matrix_norm(&m, &dir);

			*posOut += (dir * beam->range) * t;

			// we add whatever offset already exists to the tabled offset specified by the modder
			vec3d combined_offset = m_offset + tabled_offset.value_or(vmd_zero_vector);
			vm_vec_unrotate(&offset, &combined_offset, &m);
			
			break;
		}
		default: {
			*posOut = vmd_zero_vector;
			offset = m_offset;
			break;
		}
	}

	vm_vec_add2(posOut, &offset);
}
void SourceOrigin::getHostOrientation(matrix* matOut, bool allow_relative) const {
	vec3d vec;
	switch (m_originType) {
	case SourceOriginType::OBJECT:
		if (allow_relative) {
			*matOut = vmd_identity_matrix;
		} else {
			*matOut = m_origin.m_object.objp()->orient;
		}
		break;
	case SourceOriginType::SUBOBJECT: {
		const ship& shipp = Ships[m_origin.m_object.objp()->instance];
		vec3d temp;
		model_instance_local_to_global_point_orient(&temp,
			matOut,
			&vmd_zero_vector,
			&vmd_identity_matrix,
			model_get(Ship_info[shipp.ship_info_index].model_num),
			model_get_instance(shipp.model_instance_num),
			m_origin.m_subobject,
			allow_relative ? &vmd_identity_matrix : &m_origin.m_object.objp()->orient,
			&vmd_zero_vector);
		break;
		}
	case SourceOriginType::TURRET: {
		ship* shipp = &Ships[m_origin.m_object.objp()->instance];

		polymodel_instance *pmi = model_get_instance(Ships[m_origin.m_object.objp()->instance].model_instance_num);
		polymodel *pm = model_get(pmi->model_num);
		ship_subsys* sss = ship_get_indexed_subsys(shipp, pm->submodel[m_origin.m_subobject].subsys_num);

		vec3d gvec;
		model_subsystem *tp = sss->system_info;

		if (allow_relative) {
			model_instance_local_to_global_dir(&gvec, &tp->turret_norm, pm, pmi, tp->turret_gun_sobj);
		} else {
			model_instance_local_to_global_dir(&gvec, &tp->turret_norm, pm, pmi, tp->turret_gun_sobj, &m_origin.m_object.objp()->orient);
		}

		vm_vector_2_matrix(matOut, &gvec);
		break;
	}
	case SourceOriginType::PARTICLE:
		vm_vector_2_matrix(matOut, &m_origin.m_particle.lock()->velocity, nullptr, nullptr);
		break;
	case SourceOriginType::BEAM:
		vec = vmd_zero_vector;
		vm_vec_normalized_dir(&vec, &Beams[m_origin.m_object.objp()->instance].last_shot, &Beams[m_origin.m_object.objp()->instance].last_start);
		vm_vector_2_matrix(matOut, &vec, nullptr, nullptr);
		break;
	case SourceOriginType::VECTOR:
		*matOut = m_origin.m_host_orientation;
		break;
	default:
		*matOut = vmd_identity_matrix;
		break;
	}
}

void SourceOrigin::applyToParticleInfo(particle_info& info, bool allow_relative, float interp, tl::optional<vec3d> tabled_offset) const {
	Assertion(m_originType != SourceOriginType::NONE, "Invalid origin type!");

	switch (m_originType) {
		case SourceOriginType::OBJECT: {
			if (allow_relative) {
				info.attached_objnum = m_origin.m_object.objnum;
				info.attached_sig = m_origin.m_object.sig;

				info.pos = m_offset + tabled_offset.value_or(vmd_zero_vector);
			} else {
				this->getGlobalPosition(&info.pos, interp, tabled_offset);
				info.attached_objnum = -1;
				info.attached_sig = -1;
			}
			break;
		}
		case SourceOriginType::SUBOBJECT: {
			if (allow_relative) {
				info.attached_objnum = m_origin.m_object.objnum;
				info.attached_sig = m_origin.m_object.sig;

				vec3d combined_offset = m_offset + tabled_offset.value_or(vmd_zero_vector);
				model_instance_local_to_global_point(&info.pos,
					&combined_offset,
					Ships[m_origin.m_object.objp()->instance].model_instance_num,
					m_origin.m_subobject);
			} else {
				this->getGlobalPosition(&info.pos, interp, tabled_offset);
				info.attached_objnum = -1;
				info.attached_sig = -1;
			}
			break;
		}
		case SourceOriginType::TURRET: {
			if (allow_relative) {
				info.attached_objnum = m_origin.m_object.objnum;
				info.attached_sig = m_origin.m_object.sig;

				ship* shipp = &Ships[m_origin.m_object.objp()->instance];
				polymodel_instance *pmi = model_get_instance(Ships[m_origin.m_object.objp()->instance].model_instance_num);
				polymodel *pm = model_get(pmi->model_num);
				ship_subsys* sss = ship_get_indexed_subsys(shipp, pm->submodel[m_origin.m_subobject].subsys_num);

				vec3d *gun_pos;
				vec3d gvec;
				model_subsystem *tp = sss->system_info;

				gun_pos = &tp->turret_firing_point[m_origin.m_fire_pos % tp->turret_num_firing_points];

				model_instance_local_to_global_point(&info.pos, gun_pos, pm, pmi, tp->turret_gun_sobj);

				model_instance_local_to_global_dir(&gvec, &tp->turret_norm, pm, pmi, tp->turret_gun_sobj);

				vm_vec_scale_add2(&info.pos, &gvec, tabled_offset.value_or(vmd_zero_vector).xyz.z);
			} else {
				this->getGlobalPosition(&info.pos, interp, tabled_offset);
				info.attached_objnum = -1;
				info.attached_sig = -1;
			}
			break;
		}
		case SourceOriginType::PARTICLE: {
			info.rad = getScale();
			info.lifetime = getLifetime();
			this->getGlobalPosition(&info.pos, interp, tabled_offset);
			info.attached_objnum = -1;
			info.attached_sig = -1;
			break;
		}
		case SourceOriginType::BEAM: // Intentional fall-through
		case SourceOriginType::VECTOR: // Intentional fall-through
		default: {
			this->getGlobalPosition(&info.pos, interp, tabled_offset);
			info.attached_objnum = -1;
			info.attached_sig = -1;
			break;
		}
	}

	info.vel = getVelocity();
}

vec3d SourceOrigin::getVelocity() const {
	switch (this->m_originType) {
		case SourceOriginType::TURRET:
		case SourceOriginType::SUBOBJECT:
		case SourceOriginType::OBJECT:
			return m_origin.m_object.objp()->phys_info.vel;
		case SourceOriginType::PARTICLE:
			return m_origin.m_particle.lock()->velocity;
		case SourceOriginType::BEAM: {
			beam* bm = &Beams[m_origin.m_object.objp()->instance];
			vec3d vel;
			vm_vec_normalized_dir(&vel, &bm->last_shot, &bm->last_start);
			vm_vec_scale(&vel, Weapon_info[bm->weapon_info_index].max_speed);
			return vel;
		}
		default:
			return m_velocity;
	}
}

float SourceOrigin::getLifetime() const {
	switch (this->m_originType) {
	case SourceOriginType::PARTICLE:
		return m_origin.m_particle.lock()->max_life - m_origin.m_particle.lock()->age;
	default:
		return -1.0f;
	}
}

float SourceOrigin::getScale() const {
	int idx = -1;
	switch (this->m_originType) {
	case SourceOriginType::PARTICLE:
		idx = m_origin.m_particle.lock()->size_lifetime_curve;
		if (idx >= 0)
			return m_origin.m_particle.lock()->radius * Curves[idx].GetValue(m_origin.m_particle.lock()->age / m_origin.m_particle.lock()->max_life);
		else
			return m_origin.m_particle.lock()->radius;
	default:
		return 1.0f;
	}
}

void SourceOrigin::setVelocity(const vec3d* vel) {
	Assertion(vel, "Invalid vector pointer passed!");
	if (!vel)
		return;

	m_velocity = *vel;
}

void SourceOrigin::setWeaponState(WeaponState state) {
	m_weaponState = state;
}

void SourceOrigin::moveTo(const vec3d* pos, const matrix* orientation) {
	Assertion(pos, "Invalid vector pointer passed!");

	m_originType = SourceOriginType::VECTOR;
	m_origin.m_host_orientation = *orientation;
	m_origin.m_pos = *pos;
}

void SourceOrigin::moveToBeam(const object* objp) {
	Assertion(objp, "Invalid object pointer passed!");

	m_originType = SourceOriginType::BEAM;
	m_origin.m_object = object_h(OBJ_INDEX(objp));
}

void SourceOrigin::moveToObject(const object* objp, const vec3d* offset) {
	Assertion(objp, "Invalid object pointer passed!");
	Assertion(offset, "Invalid vector pointer passed!");

	m_originType = SourceOriginType::OBJECT;
	m_origin.m_object = object_h(OBJ_INDEX(objp));

	m_offset = *offset;
}

void SourceOrigin::moveToSubobject(const object* objp, int subobject, const vec3d* offset)
{
	Assertion(objp, "Invalid object pointer passed!");
	Assertion(offset, "Invalid vector pointer passed!");

	m_originType = SourceOriginType::SUBOBJECT;
	m_origin.m_object = object_h(OBJ_INDEX(objp));
	m_origin.m_subobject = subobject;

	m_offset = *offset;
}

void SourceOrigin::moveToTurret(const object* objp, int subobject, int fire_pos)
{
	Assertion(objp, "Invalid object pointer passed!");

	m_originType = SourceOriginType::TURRET;
	m_origin.m_object = object_h(OBJ_INDEX(objp));
	m_origin.m_subobject = subobject;
	m_origin.m_fire_pos = fire_pos;

	m_offset = vmd_zero_vector;
}

void SourceOrigin::moveToParticle(const WeakParticlePtr& weakParticlePtr) {
	m_originType = SourceOriginType::PARTICLE;
	m_origin.m_particle = weakParticlePtr;
}

bool SourceOrigin::isValid() const {
	switch (m_originType) {
		case SourceOriginType::NONE:
			return false;
		case SourceOriginType::TURRET: {
			if (m_origin.m_object.objp()->type != OBJ_SHIP) {
				return false;
			}
			if (m_origin.m_subobject < 0) {
				return false;
			}
			ship* shipp = &Ships[m_origin.m_object.objp()->instance];
			polymodel* pm = model_get(Ship_info[shipp->ship_info_index].model_num);
			
			return pm->submodel[m_origin.m_subobject].subsys_num >= 0;
		}
		case SourceOriginType::SUBOBJECT:
			if (m_origin.m_subobject < 0) {
				return false;
			}
			// intentional fallthrough
		case SourceOriginType::OBJECT:
		case SourceOriginType::BEAM: {
			if (!m_origin.m_object.isValid()) {
				return false;
			}

			auto objp = m_origin.m_object.objp();

			if (objp->type != OBJ_WEAPON && objp->type != OBJ_BEAM) {
				// The following checks are only relevant for weapons
				return true;
			}

			if (m_weaponState == WeaponState::INVALID) {
				// If no state is specified, ignore it.
				return true;
			}

			// Make sure we stay in the same weapon state
			if (objp->type == OBJ_BEAM) {
				beam* bm = &Beams[objp->instance];
				return bm->weapon_state == m_weaponState;
			} else {
				weapon* wp = &Weapons[objp->instance];
				return wp->weapon_state == m_weaponState;
			}
		}
		case SourceOriginType::PARTICLE:
			return !m_origin.m_particle.expired();
		case SourceOriginType::VECTOR:
			return true;
	}

	return false;
}

SourceOrientation::SourceOrientation() : m_orientation(vmd_identity_matrix) {}

void SourceOrientation::setFromVector(const vec3d& vec, bool relative) {
	vec3d workVec = vec;

	vm_vec_normalize(&workVec);

	this->setFromNormalizedVector(workVec, relative);
}

void SourceOrientation::setFromNormalizedVector(const vec3d& vec, bool relative) {
	vm_vector_2_matrix_norm(&m_orientation, &vec);
	m_isRelative = relative;
}

void SourceOrientation::setFromMatrix(const matrix& mat, bool relative) {
	m_orientation = mat;
	m_isRelative = relative;
}

void SourceOrientation::setNormal(const vec3d& normal) {
	m_hasNormal = true;
	m_normal = normal;
}

vec3d SourceOrientation::getDirectionVector(const SourceOrigin* origin, bool allow_relative) const {
	if (!m_isRelative) {
		return m_orientation.vec.fvec;
	}

	matrix finalOrient;

	matrix hostOrient;

	origin->getHostOrientation(&hostOrient, allow_relative);

	vm_matrix_x_matrix(&finalOrient, &hostOrient, &m_orientation);

	return finalOrient.vec.fvec;
}

bool SourceOrientation::getNormal(vec3d* outNormal) const {
	Assert(outNormal != nullptr);

	*outNormal = m_normal;

	return m_hasNormal;
}

SourceTiming::SourceTiming() : m_creationTimestamp(timestamp(-1)), m_beginTimestamp(timestamp(-1)),
							   m_endTimestamp(timestamp(-1)) {}

void SourceTiming::setCreationTimestamp(int time) {
	m_creationTimestamp = time;
	m_nextCreation      = time;
}

void SourceTiming::setLifetime(int begin, int end) {
	m_beginTimestamp = begin;
	m_endTimestamp = end;
}

bool SourceTiming::isActive() const {
	if (!timestamp_valid(m_beginTimestamp) && !timestamp_valid(m_endTimestamp)) {
		// No valid timestamps => default is to be active
		return true;
	}

	if (!timestamp_valid(m_beginTimestamp) && timestamp_valid(m_endTimestamp)) {
		// Active until the end has elapsed
		return !timestamp_elapsed(m_endTimestamp);
	}

	if (timestamp_valid(m_beginTimestamp) && !timestamp_valid(m_endTimestamp)) {
		// If begin is valid, check if it already happened
		return timestamp_elapsed(m_beginTimestamp) != 0;
	}

	// Check if we are in the range [begin, end]
	return timestamp_elapsed(m_beginTimestamp) && !timestamp_elapsed(m_endTimestamp);
}

bool SourceTiming::isFinished() const {
	// If end isn't valid the timing is never finished
	if (!timestamp_valid(m_endTimestamp)) {
		return false;
	}

	return timestamp_elapsed(m_endTimestamp) != 0;
}

float SourceTiming::getLifeTimeProgress() const {
	// The progress can only be given when we have a valid time range
	if (!timestamp_valid(m_beginTimestamp) && !timestamp_valid(m_endTimestamp)) {
		return -1.f;
	}

	if (!timestamp_valid(m_beginTimestamp) && timestamp_valid(m_endTimestamp)) {
		return -1.f;
	}

	if (timestamp_valid(m_beginTimestamp) && !timestamp_valid(m_endTimestamp)) {
		return -1.f;
	}

	auto total = m_endTimestamp - m_beginTimestamp;
	auto done = timestamp() - m_beginTimestamp;

	return done / (float) total;
}
int SourceTiming::getNextCreationTime() const { return m_nextCreation; }
bool SourceTiming::nextCreationTimeExpired() const { return timestamp_elapsed(m_nextCreation); }
void SourceTiming::incrementNextCreationTime(int time_diff) { m_nextCreation += time_diff; }

ParticleSource::ParticleSource() : m_effect(nullptr), m_processingCount(0) {}

bool ParticleSource::isValid() const {
	if (m_timing.isFinished()) {
		return false;
	}

	if (m_effect == nullptr) {
		return false;
	}

	if (!m_origin.isValid()) {
		return false;
	}

	return true;
}

void ParticleSource::initializeThrusterOffset(weapon*  /*wp*/, weapon_info* wip) {
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

	model_local_to_global_point(&this->m_origin.m_offset, &point->pnt, pm, thruster->submodel_num,
						   &vmd_identity_matrix, &vmd_zero_vector);
}

void ParticleSource::finishCreation() {
	if (m_origin.m_originType == SourceOriginType::OBJECT) {
		if (IS_VEC_NULL(&m_origin.m_offset)) {
			object* obj = m_origin.m_origin.m_object.objp();

			if (obj->type == OBJ_WEAPON) {
				weapon* wp = &Weapons[obj->instance];
				weapon_info* wip = &Weapon_info[wp->weapon_info_index];

				if (wip->subtype == WP_MISSILE && wip->model_num >= 0) {
					// Now that we are here we know that this is a missile which has no offset set
					// The particles of a missile should be created at its thruster
					this->initializeThrusterOffset(wp, wip);
				}
			}
		}
	}
}

bool ParticleSource::process() {
	if (m_timing.isActive()) {
		auto result = this->m_effect->processSource(this);

		++m_processingCount;

		return result;
	}
	else {
		// If not active, try the next frame
		return true;
	}
}
}
