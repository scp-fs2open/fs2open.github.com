
#include "particle/util/ParticleProperties.h"
#include "particle/ParticleManager.h"

#include "bmpman/bmpman.h"
#include "math/curve.h"

namespace particle {
namespace util {
ParticleProperties::ParticleProperties() : m_radius(0.0f, 1.0f), m_lifetime(0.0f, 1.0f), m_length (0.0f), m_size_lifetime_curve (-1), m_vel_lifetime_curve (-1), m_rotation_type(RotationType::DEFAULT) {
}

void ParticleProperties::parse(bool nocreate) {
	if (internal::required_string_if_new("+Filename:", nocreate)) {
		m_bitmap_list = internal::parseAnimationList(true);
		m_bitmap_range = ::util::UniformRange<size_t>(0, m_bitmap_list.size() - 1);
	}

	if (optional_string("+Size:")) {
		m_radius = ::util::parseUniformRange<float>();
	} else if (optional_string("+Parent Size Factor:")) {
		m_radius = ::util::parseUniformRange<float>();
		m_parentScale = true;
	} else if (!nocreate) {
		Error(LOCATION, "Missing +Size or +Parent Size Factor");
	}

	if (optional_string("+Length:")) {
		m_length = ::util::parseUniformRange<float>();
	}

	if (optional_string("+Lifetime:")) {
		if (optional_string("<none>")) {
			// Use lifetime of effect
			m_hasLifetime = false;
		} else {
			m_hasLifetime = true;
			m_lifetime = ::util::parseUniformRange<float>();
		}
	} else if (optional_string("+Parent Lifetime Factor:")) {
		m_hasLifetime = true;
		m_parentLifetime = true;
		m_lifetime = ::util::parseUniformRange<float>();
	}

	if (optional_string("+Size over lifetime curve:")) {
		SCP_string buf;
		stuff_string(buf, F_NAME);
		m_size_lifetime_curve = curve_get_by_name(buf);

		if (m_size_lifetime_curve < 0) {
			Warning(LOCATION, "Could not find curve '%s'", buf.c_str());
		}
	}

	if (optional_string("+Velocity scalar over lifetime curve:")) {
		SCP_string buf;
		stuff_string(buf, F_NAME);
		m_vel_lifetime_curve = curve_get_by_name(buf);

		if (m_vel_lifetime_curve < 0) {
			Warning(LOCATION, "Could not find curve '%s'", buf.c_str());
		}
	}

	if (optional_string("+Rotation:")) {
		SCP_string buf;
		stuff_string(buf, F_NAME);
		if (!stricmp(buf.c_str(), "DEFAULT")) {
			m_rotation_type = RotationType::DEFAULT;
		} else if (!stricmp(buf.c_str(), "RANDOM")) {
			m_rotation_type = RotationType::RANDOM;
		} else if (!stricmp(buf.c_str(), "SCREEN_ALIGNED") || !stricmp(buf.c_str(), "SCREEN-ALIGNED") || !stricmp(buf.c_str(), "SCREEN ALIGNED")) {
			m_rotation_type = RotationType::SCREEN_ALIGNED;
		} else {
			// in the future we may want to support additional types, or even a specific angle, but that is TBD
			Warning(LOCATION, "Rotation Type %s not supported", buf.c_str());
		}
	}
}

int ParticleProperties::chooseBitmap()
{
	// failsafe check, though we should not have been able to get to this point
	Assertion(!m_bitmap_list.empty(), "No bitmaps found!");

	return m_bitmap_list[m_bitmap_range.next()];
}

void ParticleProperties::createParticle(particle_info& info) {
	info.optional_data = ParticleProperties::chooseBitmap();
	info.type = PARTICLE_BITMAP;
	if (m_parentScale)
		// if we were spawned by a particle, info.rad is the parent's radius and m_radius is a factor of that
		info.rad *= m_radius.next(); 
	else
		info.rad = m_radius.next();
	info.length = m_length.next();
	if (m_hasLifetime) {
		if (m_parentLifetime)
			// if we were spawned by a particle, info.lifetime is the parent's remaining liftime and m_lifetime is a factor of that
			info.lifetime *= m_lifetime.next();
		else
			info.lifetime = m_lifetime.next();
		info.lifetime_from_animation = false;
	}
	info.size_lifetime_curve = m_size_lifetime_curve;
	info.vel_lifetime_curve = m_vel_lifetime_curve;

	switch (m_rotation_type) {
		case RotationType::DEFAULT:
			info.use_angle = Randomize_particle_rotation;
			break;
		case RotationType::RANDOM:
			info.use_angle = true;
			break;
		case RotationType::SCREEN_ALIGNED:
			info.use_angle = false;
			break;
		default:
			UNREACHABLE("Rotation type not supported");
	}

	create(&info);
}

WeakParticlePtr ParticleProperties::createPersistentParticle(particle_info& info) {
	info.optional_data = ParticleProperties::chooseBitmap();
	info.type = PARTICLE_BITMAP;
	info.rad = m_radius.next();
	info.length = m_length.next();
	info.size_lifetime_curve = m_size_lifetime_curve;
	info.vel_lifetime_curve = m_vel_lifetime_curve;

	auto p = createPersistent(&info);

	if (m_hasLifetime && !p.expired()) {
		p.lock()->max_life = m_lifetime.next();
	}

	return p;
}

void ParticleProperties::pageIn() {

	for (int bitmap : m_bitmap_list) {
		bm_page_in_texture(bitmap);
	}

}
}
}
