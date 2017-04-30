
#include "particle/ParticleSource.h"
#include "particle/effects/ParticleEmitterEffect.h"
#include "bmpman/bmpman.h"
#include "parse/parselo.h"

namespace particle {
namespace effects {
ParticleEmitterEffect::ParticleEmitterEffect() : ParticleEffect("") {
	memset(&m_emitter, 0, sizeof(m_emitter));
}

bool ParticleEmitterEffect::processSource(const ParticleSource* source) {
	particle_emitter emitter = m_emitter;
	source->getOrigin()->getGlobalPosition(&emitter.pos);
	emitter.normal = source->getOrientation()->getDirectionVector();

	emit(&emitter, PARTICLE_BITMAP, m_particleBitmap, m_range);

	return false;
}

void ParticleEmitterEffect::parseValues(bool) {
	error_display(1, "Parsing not implemented for this effect because I'm lazy...");
}

void ParticleEmitterEffect::pageIn() {
	bm_page_in_texture(m_particleBitmap);
}

void ParticleEmitterEffect::setValues(const particle_emitter& emitter, int bitmap, float range) {
	Assert(bm_is_valid(bitmap));

	m_emitter = emitter;
	m_particleBitmap = bitmap;
	m_range = range;
}
}
}
