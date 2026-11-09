#pragma once

#include "particle/EffectHost.h"
#include "object/object.h"

class EffectHostSubmodel : public EffectHost {

	vec3d m_offset;

	int m_objnum, m_objsig, m_submodel, m_modelnum;
public:
	EffectHostSubmodel(const object* objp, int submodel, vec3d offset, matrix orientationOverride = vmd_identity_matrix, bool orientationOverrideRelative = true);

	std::pair<vec3d, matrix> getPositionAndOrientation(bool relativeToParent, float interp, const std::optional<vec3d>& tabled_offset) const override;

	vec3d getVelocity() const override;

	effects::EffectAttachment getParentAttachment() const override;
	int getParentSubmodel() const override;

	float getHostRadius() const override;

	bool isValid() const override;
};