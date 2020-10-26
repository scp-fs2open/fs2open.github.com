
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
	if (m_bitmap_list.empty())
		return -1;

	int bitmap_index;
	int num_bitmaps = (int)m_bitmap_list.size();

	if (num_bitmaps == 1) {
		bitmap_index = 0;
	} else if (num_bitmaps > 1) {
		bitmap_index = m_bitmap_list[rand() % num_bitmaps];		
	} else {
		bitmap_index = -1;
	}

	return bitmap_index;
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
	
	int num_bitmaps = m_bitmap_list.size();
	if (num_bitmaps > 0 ) {
		for (auto i = 0; i < num_bitmaps; i++) {
			bm_page_in_aabitmap(m_bitmap_list[i], -1);
		}
	}

}
}
}
