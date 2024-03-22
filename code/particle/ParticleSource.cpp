#include <math/bitarray.h>
#include <math/curve.h>
#include "particle/ParticleSource.h"
#include "weapon/weapon.h"
#include "ParticleSource.h"

namespace particle {
SourceOrigin::SourceOrigin() : m_originType(SourceOriginType::NONE),
							   m_weaponState(WeaponState::INVALID),
							   m_offset(vmd_zero_vector),
	                           m_velocity(vmd_zero_vector) {
}

void SourceOrigin::getGlobalPosition(vec3d* posOut) const {
	Assertion(posOut != nullptr, "Invalid vector pointer passed!");
	Assertion(m_originType != SourceOriginType::NONE, "Invalid origin type!");

	vec3d offset;
	switch (m_originType) {
		case SourceOriginType::OBJECT: {
			*posOut = m_origin.m_object.objp->pos;
			vm_vec_unrotate(&offset, &m_offset, &m_origin.m_object.objp->orient);
			break;
		}
		case SourceOriginType::PARTICLE: {
			*posOut = m_origin.m_particle.lock()->pos;

			matrix m = vmd_identity_matrix;
			vec3d dir = m_origin.m_particle.lock()->velocity;

			vm_vec_normalize_safe(&dir);
			vm_vector_2_matrix_norm(&m, &dir);

			vm_vec_unrotate(&offset, &m_offset, &m);

			break;
		}
		case SourceOriginType::VECTOR: {
			*posOut = m_origin.m_pos;
			offset = m_offset;
			break;
		}
		case SourceOriginType::BEAM: {
			auto beam = &Beams[m_origin.m_object.objp->instance];
			*posOut = beam->last_start;
			// weight the random points towards the start linearly
			// proportion along the beam the beam stopped, of its total potential length
			float dist_adjusted = vm_vec_dist(&beam->last_start, &beam->last_shot) / beam->range;
			// randomly sample from the weighted distribution, excluding points beyond the last_shot
			auto t = (1.0f - sqrtf(frand_range(powf(1.f - dist_adjusted, 2.0f), 1.0f)));
			vec3d dir;
			vm_vec_normalized_dir(&dir, &beam->last_shot, &beam->last_start);
			*posOut += (dir * beam->range) * t;
			offset = m_offset;
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
void SourceOrigin::getHostOrientation(matrix* matOut) const {
	vec3d vec;
	switch (m_originType) {
	case SourceOriginType::OBJECT:
		*matOut = m_origin.m_object.objp->orient;
		break;
	case SourceOriginType::PARTICLE:
		vm_vector_2_matrix(matOut, &m_origin.m_particle.lock()->velocity, nullptr, nullptr);
		break;
	case SourceOriginType::BEAM:
		vec = vmd_zero_vector;
		vm_vec_normalized_dir(&vec, &Beams[m_origin.m_object.objp->instance].last_shot, &Beams[m_origin.m_object.objp->instance].last_start);
		vm_vector_2_matrix(matOut, &vec, nullptr, nullptr);
		break;
	case SourceOriginType::VECTOR: // Intentional fall-through, plain vectors have no orientation
	default:
		*matOut = vmd_identity_matrix;
		break;
	}
}

void SourceOrigin::applyToParticleInfo(particle_info& info, bool allow_relative) const {
	Assertion(m_originType != SourceOriginType::NONE, "Invalid origin type!");

	switch (m_originType) {
		case SourceOriginType::OBJECT: {
			if (allow_relative) {
				info.attached_objnum = static_cast<int>(OBJ_INDEX(m_origin.m_object.objp));
				info.attached_sig = m_origin.m_object.objp->signature;

				info.pos = m_offset;
			} else {
				this->getGlobalPosition(&info.pos);
				info.attached_objnum = -1;
				info.attached_sig = -1;
			}
			break;
		}
		case SourceOriginType::PARTICLE: {
			info.rad = getScale();
			info.lifetime = getLifetime();
			this->getGlobalPosition(&info.pos);
			info.attached_objnum = -1;
			info.attached_sig = -1;
			break;
		}
		case SourceOriginType::BEAM: // Intentional fall-through
		case SourceOriginType::VECTOR: // Intentional fall-through
		default: {
			this->getGlobalPosition(&info.pos);
			info.attached_objnum = -1;
			info.attached_sig = -1;
			break;
		}
	}

	info.vel = getVelocity();
}

vec3d SourceOrigin::getVelocity() const {
	switch (this->m_originType) {
		case SourceOriginType::OBJECT:
			return m_origin.m_object.objp->phys_info.vel;
		case SourceOriginType::PARTICLE:
			return m_origin.m_particle.lock()->velocity;
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

void SourceOrigin::setVelocity(vec3d* vel) {
	Assertion(vel, "Invalid vector pointer passed!");
	if (!vel)
		return;

	m_velocity = *vel;
}

void SourceOrigin::setWeaponState(WeaponState state) {
	m_weaponState = state;
}

void SourceOrigin::moveTo(vec3d* pos) {
	Assertion(pos, "Invalid vector pointer passed!");

	m_originType = SourceOriginType::VECTOR;
	m_origin.m_pos = *pos;
}

void SourceOrigin::moveToBeam(object* objp) {
	Assertion(objp, "Invalid object pointer passed!");

	m_originType = SourceOriginType::BEAM;
	m_origin.m_object = object_h(objp);
}

void SourceOrigin::moveToObject(object* objp, vec3d* offset) {
	Assertion(objp, "Invalid object pointer passed!");
	Assertion(offset, "Invalid vector pointer passed!");

	m_originType = SourceOriginType::OBJECT;
	m_origin.m_object = object_h(objp);

	m_offset = *offset;
}

void SourceOrigin::moveToParticle(const WeakParticlePtr& weakParticlePtr) {
	m_originType = SourceOriginType::PARTICLE;
	m_origin.m_particle = weakParticlePtr;
}

bool SourceOrigin::isValid() const {
	switch (m_originType) {
		case SourceOriginType::NONE:
			return false;
		case SourceOriginType::OBJECT:
		case SourceOriginType::BEAM: {
			if (!m_origin.m_object.isValid()) {
				return false;
			}

			auto objp = m_origin.m_object.objp;

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

vec3d SourceOrientation::getDirectionVector(const SourceOrigin* origin) const {
	if (!m_isRelative) {
		return m_orientation.vec.fvec;
	}

	matrix finalOrient;

	matrix hostOrient;
	origin->getHostOrientation(&hostOrient);

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
			object* obj = m_origin.m_origin.m_object.objp;

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
