
#include "particle/util/ParticleProperties.h"
#include "particle/ParticleManager.h"

#include "bmpman/bmpman.h"

namespace particle {
namespace util {
ParticleProperties::ParticleProperties() : m_radius(0.0f, 1.0f), m_lifetime(0.0f, 1.0f), m_length (0.0f){
}

void ParticleProperties::parse(bool nocreate) {
	if (internal::required_string_if_new("+Filename:", nocreate)) {
		m_bitmap = internal::parseAnimation(true);
	}

	if (optional_string("+Filenames:")) {
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
	// if list is empty that means the optional string to add multiple bitmaps was not used
	// thus choose the necessary bitmap specified "+Filename:"
	if (m_bitmap_list.empty())
		return m_bitmap;

	int bitmap_index = -1;
	int num_bitmaps = (int)m_bitmap_list.size();

	if (num_bitmaps > 0) {
		bitmap_index = m_bitmap_list[rand() % num_bitmaps];
		return bitmap_index;
	} else {
		return m_bitmap;
	}
	
}

void ParticleProperties::createParticle(particle_info& info) {
	m_bitmap = ParticleProperties::chooseBitmap();
	info.optional_data = m_bitmap;
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
	m_bitmap = ParticleProperties::chooseBitmap();
	info.optional_data = m_bitmap;
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
	if (m_bitmap >= 0) {
		bm_page_in_aabitmap(m_bitmap, -1);
	}
}
}
}
