#include "EffectTiming.h"

namespace particle {
namespace util {


EffectTiming::EffectTiming() : m_duration(Duration::Onetime) {

}

void EffectTiming::applyToSource(ParticleSource* source) const
{
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

bool EffectTiming::continueProcessing(const ParticleSource* source) const
{
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

bool EffectTiming::shouldCreateEffect(ParticleSource* source, EffectTiming::TimingState& localState) const
{
	if (m_particlesPerSecond < 0.0f) {
		// If this is not specified then on every frame we will create exactly one effect
		if (localState.initial) {
			localState.initial = false;
			return true;
		} else {
			return false;
		}
	}

	// We have a valid particles per second value so we use the next creation timestamp in the particle source for
	// deciding if we can create an effect


	if (source->getTiming()->nextCreationTimeExpired())
	{
		// Invert this so we can compute the time difference between effect creations
		auto secondsPerParticle = 1.0f / m_particlesPerSecond;
		auto time_diff_ms = fl2i(secondsPerParticle * 1000.f);
		source->getTiming()->incrementNextCreationTime(time_diff_ms);

		return true;
	}

	return false;
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
			timing.m_durationRange = ::util::parseUniformRange<float>(0.0f);
		}
	}

	if (optional_string("+Delay:")) {
		if (timing.m_duration == Duration::Onetime) {
			error_display(0, "+Delay is not valid for one-time effects!");
		}
		else {
			timing.m_delayRange = ::util::parseUniformRange<float>(0.0f);
		}
	}

	if (optional_string("+Effects per second:")) {
		stuff_float(&timing.m_particlesPerSecond);
		if (timing.m_particlesPerSecond < 0.001f) {
			error_display(0, "Invalid effect per second value %f. Setting was disabled.", timing.m_particlesPerSecond);
			timing.m_particlesPerSecond = -1.f;
		}
	}

	return timing;
}
}
}
