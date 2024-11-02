#pragma once

#include "globalincs/pstypes.h"

#include <tl/optional.hpp>

class EffectHost {

protected:
	matrix m_orientationOverride;
	bool m_orientationOverrideRelative;

	EffectHost(matrix orientationOverride, bool orientationOverrideRelative) : m_orientationOverride(orientationOverride), m_orientationOverrideRelative(orientationOverrideRelative) {}

public:
	virtual ~EffectHost() = default;

	virtual std::pair<vec3d, matrix> getPositionAndOrientation(bool relativeToParent, float interp, const tl::optional<vec3d>& tabled_offset) = 0;

	virtual vec3d getVelocity() = 0;

	virtual std::pair<int, int> getParentObjAndSig() { return {-1, -1}; }

	virtual float getLifetime() { return -1.f; }

	virtual float getScale() { return 1.f; }

	virtual float getParticleMultiplier() { return 1.f; }

	virtual bool isValid() { return true; }

	virtual void setupProcessing() {}

};