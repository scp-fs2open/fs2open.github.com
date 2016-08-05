#include "EffectTiming.h"

namespace particle {
namespace util {


EffectTiming::EffectTiming() : m_duration(Duration::Onetime) {

}

void EffectTiming::applyToSource(ParticleSource* source) {
	if (m_duration == Duration::Range || m_duration == Duration::Always) {
		int duration = -1;
		int delay = static_cast<int>(m_delayRange.next() * 1000.f);

		// Make the timestamp invalid if there is no delay
		if (delay == 0) {
			delay = -1;
		}

		if (m_duration == Duration::Range) {
			duration = static_cast<int>(m_durationRange.next() * 1000.f);
		}

		if (delay != -1 && duration != -1) {
			// Need to adjust the duration because setLifetime expects a timestamp for the end
			duration = delay + duration;
		}

		source->getTiming()->setLifetime(timestamp(delay), timestamp(duration));
	}
}


bool EffectTiming::continueProcessing(const ParticleSource* source) {
	switch (m_duration) {
		case Duration::Onetime:
			if (source->getProcessingCount() > 0) {
				// Only process on the first frame
				return false;
			}
			break;
		default:
			// Everything else is handled elsewhere
			return true;
	}

	return true;
}

EffectTiming EffectTiming::parseTiming() {
	EffectTiming timing;

	if (optional_string("+Duration:")) {
		if (optional_string("Onetime")) {
			timing.m_duration = Duration::Onetime;
		}
		else if (optional_string("Always")) {
			timing.m_duration = Duration::Always;
		}
		else {
			timing.m_duration = Duration::Range;
			timing.m_durationRange = util::parseUniformRange<float>(false);
		}
	}

	if (optional_string("+Delay:")) {
		if (timing.m_duration == Duration::Onetime) {
			error_display(0, "+Delay is not valid for one-time effects!");
		}
		else {
			timing.m_delayRange = util::parseUniformRange<float>(false);
		}
	}

	return timing;
}
}
}
