
#include "particle/util/ParticleProperties.h"
#include "particle/ParticleManager.h"

#include "bmpman/bmpman.h"

namespace particle {
namespace util {
ParticleProperties::ParticleProperties() : m_radius(0.0f, 1.0f), m_lifetime(0.0f, 1.0f), m_length (0.0f){
}

void ParticleProperties::parse(bool nocreate) {
	if (internal::required_string_if_new("+Filename:", nocreate)) {
		m_bitmap_list = internal::parseAnimationList(true);
		m_bitmap_range = ::util::UniformRange<size_t>(0, m_bitmap_list.size() - 1);
	}

	if (internal::required_string_if_new("+Size:", nocreate)) {
		m_radius = ::util::parseUniformRange<float>();
	}

	if (optional_string("+Length:")) {
		m_length = ::util::parseUniformRange<float>();
	}

	if (optional_string("+Lifetime:")) {
		if (optional_string("<none>")) {
			// Use lifetime of effect
			m_hasLifetime = false;
		}
		else {
			m_hasLifetime = true;
			m_lifetime = ::util::parseUniformRange<float>();
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
	info.rad = m_radius.next();
	info.length = m_length.next();
	if (m_hasLifetime) {
		info.lifetime = m_lifetime.next();
		info.lifetime_from_animation = false;
	}

	create(&info);
}

WeakParticlePtr ParticleProperties::createPersistentParticle(particle_info& info) {
	info.optional_data = ParticleProperties::chooseBitmap();
	info.type = PARTICLE_BITMAP;
	info.rad = m_radius.next();
	info.length = m_length.next();

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
