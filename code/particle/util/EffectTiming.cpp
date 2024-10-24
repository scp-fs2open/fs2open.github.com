#include "EffectTiming.h"

namespace particle {
namespace util {


EffectTiming::EffectTiming() : m_duration(Duration::Onetime) {

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
			timing.m_durationRange = ::util::ParsedRandomFloatRange::parseRandomRange(0.0f);
		}
	}

	if (optional_string("+Delay:")) {
		if (timing.m_duration == Duration::Onetime) {
			error_display(0, "+Delay is not valid for one-time effects!");
		}
		else {
			timing.m_delayRange = ::util::ParsedRandomFloatRange::parseRandomRange(0.0f);
		}
	}

	if (optional_string("+Effects per second:")) {
		timing.m_particlesPerSecond = ::util::ParsedRandomFloatRange::parseRandomRange();
		if (timing.m_particlesPerSecond.min() < 0.001f) {
			error_display(0, "Invalid effects per second minimum %f. Setting was disabled.", timing.m_particlesPerSecond.min());
			timing.m_particlesPerSecond = ::util::UniformFloatRange(-1.f);
		}
		if (timing.m_particlesPerSecond.max() > 1000.0f) {
			error_display(0, "Effects per second maximum %f is above 1000. Delay between effects will be clamped to 1 millisecond.", timing.m_particlesPerSecond.max());
		}
	}

	return timing;
}
}
}
