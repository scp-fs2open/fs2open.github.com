
#include "particle/util/ParticleProperties.h"
#include "particle/ParticleManager.h"

#include "bmpman/bmpman.h"

namespace particle {
namespace util {
ParticleProperties::ParticleProperties() : m_radius(0.0f, 1.0f), m_lifetime(0.0f, 1.0f) {
}

void ParticleProperties::parse(bool nocreate) {
	if (internal::required_string_if_new("+Filename:", nocreate)) {
		m_bitmap = internal::parseAnimation(true);
	}

	if (internal::required_string_if_new("+Size:", nocreate)) {
		m_radius = util::parseUniformRange<float>();
	}

	if (optional_string("+Lifetime:")) {
		if (optional_string("<none>")) {
			// Use lifetime of effect
			m_hasLifetime = false;
		}
		else {
			m_hasLifetime = true;
			m_lifetime = util::parseUniformRange<float>();
		}
	}
}

WeakParticlePtr ParticleProperties::createParticle(particle_info& info) {
	info.optional_data = m_bitmap;
	info.type = PARTICLE_BITMAP;
	info.rad = m_radius.next();

	auto p = create(&info);

	if (m_hasLifetime && !p.expired()) {
		p.lock()->max_life = m_lifetime.next();
	}

	return p;
}

void ParticleProperties::pageIn() {
	if (m_bitmap >= 0) {
		bm_page_in_aabitmap(m_bitmap);
	}
}
}
}
