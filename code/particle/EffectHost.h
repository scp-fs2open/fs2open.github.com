#pragma once

#include "object/object.h"
#include "globalincs/pstypes.h"
#include "math/vecmat.h"

#include <optional>

class EffectHost {

protected:
	matrix m_orientationOverride;
	bool m_orientationOverrideRelative;

	EffectHost(matrix orientationOverride, bool orientationOverrideRelative) : m_orientationOverride(orientationOverride), m_orientationOverrideRelative(orientationOverrideRelative) {}

public:
	virtual ~EffectHost() = default;

	virtual std::pair<vec3d, matrix> getPositionAndOrientation(bool relativeToParent, float interp, const std::optional<vec3d>& tabled_offset) const = 0;

	virtual vec3d getVelocity() const = 0;

	virtual float getVelocityMagnitude() const {
		vec3d velocity = getVelocity();
		return vm_vec_mag_quick(&velocity);
	}

	virtual std::pair<int, int> getParentObjAndSig() const { return {-1, -1}; }

	virtual float getLifetime() const { return -1.f; }

	virtual float getScale() const { return 1.f; }

	virtual float getParticleMultiplier() const { return 1.f; }

	virtual float getHostRadius() const { return 0.f; }

	virtual bool isValid() const { return true; }

	virtual void setupProcessing() {}
};