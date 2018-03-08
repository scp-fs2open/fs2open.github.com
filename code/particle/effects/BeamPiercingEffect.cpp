

#include "particle/effects/BeamPiercingEffect.h"
#include "particle/ParticleSource.h"

#include "bmpman/bmpman.h"
#include "parse/parselo.h"

namespace particle {
namespace effects {
bool BeamPiercingEffect::processSource(const ParticleSource* source) {
	particle_info info;
	memset(&info, 0, sizeof(info));

	source->getOrigin()->applyToParticleInfo(info);

	if (m_effectBitmap >= 0) {
		info.type = PARTICLE_BITMAP_PERSISTENT;
		info.optional_data = m_effectBitmap;
	}
	else {
		info.type = PARTICLE_SMOKE;
	}

	info.rad = m_radius * frand_range(0.5f, 2.0f);

	vec3d fvec = source->getOrientation()->getDirectionVector(source->getOrigin());

	float base_v, back_v;
	vec3d rnd_vec;

	vm_vec_rand_vec_quick(&rnd_vec);

	if (m_velocity != 0.0f) {
		base_v = m_velocity;
	} else {
		base_v = m_radius;
	}

	if (m_backVelocity != 0.0f) {
		back_v = m_backVelocity;
	} else {
		back_v = base_v * (-0.2f);
	}

	vm_vec_copy_scale(&info.vel, &fvec, base_v * frand_range(1.0f, 2.0f));
	vm_vec_scale_add2(&info.vel, &rnd_vec, base_v * m_variance);

	// Create the primary piercing particle
	create(&info);

	vm_vec_copy_scale(&info.vel, &fvec, back_v * frand_range(1.0f, 2.0f));
	vm_vec_scale_add2(&info.vel, &rnd_vec, back_v * m_variance);

	// Create the splash particle
	create(&info);

	return false;
}

void BeamPiercingEffect::parseValues(bool) {
	error_display(1, "Parsing not implemented for this effect because I'm lazy...");
}

void BeamPiercingEffect::pageIn() {
	bm_page_in_texture(m_effectBitmap);
}

void BeamPiercingEffect::setValues(int bitmapIndex, float radius, float velocity, float back_velocity, float variance) {
	m_effectBitmap = bitmapIndex;
	m_radius = radius;
	m_velocity = velocity;
	m_backVelocity = back_velocity;
	m_variance = variance;
}
}
}
