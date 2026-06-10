#pragma once

#include "object/object.h"
#include "globalincs/pstypes.h"
#include "globalincs/utility.h"
#include "math/vecmat.h"

#include <optional>

namespace particle {
	struct particle;
	typedef std::weak_ptr<particle> WeakParticlePtr;
}

namespace effects {
	struct attachment_object {
		int objnum = -1;
		int sig = -1;
	};
	struct attachment_particle {
		particle::WeakParticlePtr particle = particle::WeakParticlePtr();
	};
}
using EffectAttachment = std::variant<std::monostate, effects::attachment_object, effects::attachment_particle>;

namespace effects {

vec3d attachment_local_pos_to_global(const EffectAttachment& attachment, const vec3d& local_pos, float interp = 0.0f);

vec3d attachment_local_vel_to_global(const EffectAttachment& attachment, const vec3d& local_vel);

vec3d attachment_local_last_pos_to_global(const EffectAttachment& attachment, const vec3d& last_pos);

bool is_attachment_valid(const EffectAttachment& attachment);

std::pair<vec3d, matrix> get_attachment_frame(const EffectAttachment& attachment, float interp = 0.0f);

inline std::optional<effects::attachment_object> extract_attachment_object(const EffectAttachment& input) {
	return variant_get_optional<effects::attachment_object>(input);
}

}

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

	virtual EffectAttachment getParentAttachment() const { return {}; }
	virtual int getParentSubmodel() const { return -1; }

	virtual float getLifetime() const { return -1.f; }

	virtual float getScale() const { return 1.f; }

	virtual float getParticleMultiplier() const { return 1.f; }

	virtual float getHostRadius() const { return 0.f; }

	virtual bool isValid() const { return true; }

	virtual void setupProcessing() {}
};