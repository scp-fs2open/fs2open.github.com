#pragma once

#include "object/object.h"
#include "globalincs/pstypes.h"
#include "globalincs/utility.h"
#include "math/vecmat.h"

#include <optional>
#include <variant>

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

	struct EffectAttachment : public std::variant<std::monostate, attachment_object, attachment_particle> {
		using std::variant<std::monostate, attachment_object, attachment_particle>::variant;

		vec3d local_pos_to_global(const vec3d& local_pos, float interp = 0.0f) const;
		vec3d local_vel_to_global(const vec3d& local_vel) const;
		vec3d local_last_pos_to_global(const vec3d& last_pos) const;
		bool is_valid() const;
		std::pair<vec3d, matrix> get_frame(float interp = 0.0f) const;
		std::optional<attachment_object> extract_object() const;
		EffectAttachment resolve_true_parent() const;
	};
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

	virtual effects::EffectAttachment getParentAttachment() const { return {}; }
	virtual int getParentSubmodel() const { return -1; }

	virtual float getLifetime() const { return -1.f; }

	virtual float getScale() const { return 1.f; }

	virtual float getParticleMultiplier() const { return 1.f; }

	virtual float getHostRadius() const { return 0.f; }

	virtual bool isValid() const { return true; }

	virtual void setupProcessing() {}
};