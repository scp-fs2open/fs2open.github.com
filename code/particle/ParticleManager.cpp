#include <algorithm>
#include <memory>

#include "particle/effects/OmniEffect.h"

#include "particle/ParticleManager.h"

#include "particle/ParticleSourceWrapper.h"

#include "bmpman/bmpman.h"
#include "globalincs/systemvars.h"
#include "tracing/tracing.h"

/**
 * @defgroup particleSystems Particle System
 */

namespace {
using namespace particle;

constexpr size_t effectTypeNamesMax = static_cast<size_t>(EffectType::MAX);
const char* effectTypeNames[effectTypeNamesMax] = {
	"Single",
	"Composite",
	"Cone",
	"Sphere",
	"Volume"
};


std::nullptr_t constructEffect(const SCP_string& name, EffectType type) {
	/*using namespace effects;
	// Use a unique_ptr to make sure memory is deallocated if an exception is thrown
	std::unique_ptr<ParticleEffect> effect;

	switch (type) {
		case EffectType::Single: {
			effect.reset(new SingleParticleEffect(name));
			effect->parseValues(false);
			break;
		}
		case EffectType::Composite: {
			 while (optional_string("+Child effect:")) {
				auto effectId = internal::parseEffectElement();
				if (effectId.isValid()) {
					//TODO
					ParticleEffectPtr effect = ParticleManager::get()->getEffect(effectId);

					if (effect->getType() == EffectType::Composite) {
						error_display(0,
									  "A composite effect cannot contain more composite effects! The effect has not been added.");
					}
					else {
						addEffect(effect);
					}
				}
			}
		}
		case EffectType::Cone: {
			if (internal::required_string_if_new("+Deviation:", nocreate)) {
				float deviation;
				stuff_float(&deviation);

				if (deviation < 0.001f) {
					error_display(0, "A standard deviation of %f is not valid. Must be greater than 0. Defaulting to 1.",
								  deviation);
					deviation = 1.0f;
				}

				m_normalDeviation = ::util::NormalFloatRange(0.f, fl_radians(deviation));
			}
		}
		case EffectType::Sphere: {
		}
		 m_particleProperties.parse(nocreate);

			m_shape.parse(nocreate);

			if (internal::required_string_if_new("+Velocity:", nocreate)) {
				m_velocity = ::util::ParsedRandomFloatRange::parseRandomRange();
			}

			if (internal::required_string_if_new("+Number:", nocreate)) {
				m_particleNum = ::util::ParsedRandomRange<uint>::parseRandomRange();
			}
			if (!nocreate) {
				m_particleChance = 1.0f;
			}
			if (optional_string("+Chance:")) {
				float chance;
				stuff_float(&chance);
				if (chance <= 0.0f) {
					error_display(0,
						"Particle %s tried to set +Chance: %f\nChances below 0 would result in no particles.",
						m_name.c_str(), chance);
				} else if (chance > 1.0f) {
					error_display(0,
						"Particle %s tried to set +Chance: %f\nChances above 1 are ignored, please use +Number: (min,max) "
						"to spawn multiple particles.", m_name.c_str(), chance);
					chance = 1.0f;
				}
				m_particleChance = chance;
			}
			m_particleRoll = ::util::UniformFloatRange(m_particleChance - 1.0f, m_particleChance);

			if (optional_string("+Direction:")) {
				char dirStr[NAME_LENGTH];
				stuff_string(dirStr, F_NAME, NAME_LENGTH);

				if (!stricmp(dirStr, "Incoming")) {
					m_direction = ConeDirection::Incoming;
				}
				else if (!stricmp(dirStr, "Normal")) {
					m_direction = ConeDirection::Normal;
				}
				else if (!stricmp(dirStr, "Reflected")) {
					m_direction = ConeDirection::Reflected;
				}
				else if (!stricmp(dirStr, "Reverse")) {
					m_direction = ConeDirection::Reverse;
				}
				else {
					error_display(0, "Unknown direction name '%s'!", dirStr);
				}
			}

			bool saw_deprecated_effect_location = false;
			if (optional_string("+Trail effect:")) {
				// This is the deprecated location since this introduces ambiguities in the parsing process
				m_particleTrail = internal::parseEffectElement();
				saw_deprecated_effect_location = true;
			}

			if (optional_string("+Parent Velocity Factor:")) {
				m_vel_inherit = ::util::ParsedRandomFloatRange::parseRandomRange();
			}

			m_timing = util::EffectTiming::parseTiming();

			if (optional_string("+Trail effect:")) {
				// This is the new and correct location. This might create duplicate effects but the warning should be clear
				// enough to avoid that
				if (saw_deprecated_effect_location) {
					error_display(0, "Found two trail effect options! Specifying '+Trail effect:' before '+Duration:' is "
									 "deprecated since that can cause issues with conflicting effect options.");
				}
				m_particleTrail = internal::parseEffectElement();
			}

		case EffectType::Volume: {
			m_particleProperties.parse(nocreate);

			if (internal::required_string_if_new("+Velocity:", nocreate)) {
				m_velocity = ::util::ParsedRandomFloatRange::parseRandomRange();
			}

			if (internal::required_string_if_new("+Number:", nocreate)) {
				m_particleNum = ::util::ParsedRandomRange<uint>::parseRandomRange();
			}

			if (!nocreate) {
				m_particleChance = 1.0f;
			}

			if (optional_string("+Chance:")) {
				float chance;
				stuff_float(&chance);
				if (chance <= 0.0f) {
					error_display(0,
						"Particle %s tried to set +Chance: %f\nChances below 0 would result in no particles.",
						m_name.c_str(),
						chance);
				} else if (chance >= 1.0f) {
					error_display(0,
						"Particle %s tried to set +Chance: %f\nChances above 1 are ignored, please use +Number: "
						"(min,max) to spawn multiple particles.",
						m_name.c_str(),
						chance);
					chance = 1.0f;
				}
				m_particleChance = chance;
			}
			m_particleRoll = ::util::UniformFloatRange(m_particleChance - 1.0f, m_particleChance);

			if (internal::required_string_if_new("+Volume radius:", nocreate)) {
				float radius;
				stuff_float(&radius);

				if (radius < 0.001f) {
					error_display(0, "A volume radius of %f is not valid. Must be greater than 0. Defaulting to 10.", radius);
					radius = 10.0f;
				}
				m_radius = radius;
			}

			if (optional_string("+Bias:")) {
				float bias;
				stuff_float(&bias);

				if (bias < 0.001f) {
					error_display(0, "A volume bias value of %f is not valid. Must be greater than 0.", bias);
					bias = 1.0f;
				}
				m_bias = bias;
			}

			if (optional_string("+Stretch:")) {
				float stretch;
				stuff_float(&stretch);

				if (stretch < 0.001f) {
					error_display(0, "A volume stretch value of %f is not valid. Must be greater than 0.", stretch);
					stretch = 1.0f;
				}
				m_stretch = stretch;
			}

			if (optional_string("+Parent Velocity Factor:")) {
				m_vel_inherit = ::util::ParsedRandomFloatRange::parseRandomRange();
			}

			m_timing = util::EffectTiming::parseTiming();
		}
		default: {
			Error(LOCATION, "Unimplemented effect type %d encountered! Get a coder!", static_cast<int>(type));
			throw std::runtime_error("Unimplemented effect type encountered!");
		}
	}

	return effect.release();*/
	return nullptr;
}

EffectType parseEffectType() {
	required_string("$Type:");

	SCP_string type;
	stuff_string(type, F_NAME);

	int i = string_lookup(type.c_str(), effectTypeNames, effectTypeNamesMax, "EffectType", true);
	if (i >= 0)
		return static_cast<EffectType>(i);
	else
		return EffectType::Invalid;
}

void parseCallback(const char* fileName) {
	using namespace particle;

	try {
		read_file_text(fileName, CF_TYPE_TABLES);

		reset_parse();

		required_string("#Particle Effects");

		while (optional_string("$Effect:")) {
			SCP_string name;
			stuff_string(name, F_NAME);

			auto type = parseEffectType();

			//TODO
			//ParticleManager::get()->addEffect(constructEffect(name, type));
		}

		required_string("#End");
	}
	catch (const parse::ParseException& e)
	{
		mprintf(("TABLES: Unable to parse '%s'!  Error message = %s.\n", fileName, e.what()));
		return;
	}
}

void parseConfigFiles() {
	parse_modular_table("*-part.tbm", parseCallback);
}
}

namespace particle {
std::unique_ptr<ParticleManager> ParticleManager::m_manager = nullptr;

ParticleManager::ParticleManager() {}

void ParticleManager::init() {
	Assertion(m_manager == nullptr, "ParticleManager was not properly shut down!");

	m_manager.reset(new ParticleManager());

	//Need to init the base graphics once here
	::particle::init();

	parseConfigFiles();
}

void ParticleManager::shutdown() {
	Assertion(m_manager != nullptr, "ParticleManager was not properly inited!");

	m_manager = nullptr;
}

ParticleSource* ParticleManager::createSource() {
	ParticleSource* source;

	// If we are currently in the onFrame function, adding stuff to the vector would invalidate the iterator currently in use
	if (m_processingSources) {
		m_deferredSourceAdding.emplace_back();

		source = &m_deferredSourceAdding.back();
	}
	else {
		m_sources.emplace_back();

		source = &m_sources.back();
	}

	return source;
}

ParticleEffectHandle ParticleManager::getEffectByName(const SCP_string& name)
{
	if (name.empty()) {
		// Don't allow empty names, it's a special case for effects that should not be referenced.
		return ParticleEffectHandle::invalid();
	}

	auto foundIterator = find_if(m_effects.begin(), m_effects.end(),
								 [&name](const SCP_vector<ParticleEffect>& vec) {
									 return !vec.empty() && lcase_equal(vec.front().getName(), name);
								 });

	if (foundIterator == m_effects.end()) {
		return ParticleEffectHandle::invalid();
	}

	return ParticleEffectHandle(distance(m_effects.begin(), foundIterator));
}

void ParticleManager::doFrame(float) {
	if (Is_standalone) {
		return;
	}

	TRACE_SCOPE(tracing::ProcessParticleEffects);

	m_processingSources = true;

	for (auto source = std::begin(m_sources); source != std::end(m_sources);) {
		if (!source->isValid() || !source->process()) {
			// if we're sitting on the very last source, popping-back will invalidate the iterator!
			if (std::next(source) == m_sources.end()) {
				m_sources.pop_back();
				break;
			}

			*source = std::move(m_sources.back());
			m_sources.pop_back();
			continue;
		}

		// source is only incremented here as elements would be skipped in
		// the case that a source needs to be removed
		++source;
	}

	m_processingSources = false;

	for (auto& source : m_deferredSourceAdding) {
		m_sources.push_back(source);
	}
	m_deferredSourceAdding.clear();
}

ParticleEffectHandle ParticleManager::addEffect(ParticleEffect&& effect)
{
	SCP_vector<ParticleEffect> effectList;
	effectList.emplace_back(std::move(effect));
	return addEffect(std::move(effectList));
}

ParticleEffectHandle ParticleManager::addEffect(SCP_vector<ParticleEffect>&& effect)
{
	// we don't need this on standalone so remove the effect and return something invalid
	if (Is_standalone) {
		return ParticleEffectHandle::invalid();
	}

	Assert(!effect.empty());

#ifndef NDEBUG
	if (!effect.front().getName().empty()) {
		// This check is a bit expensive and will only be used in debug
		auto index = getEffectByName(effect.front().getName());

		if (index.isValid()) {
			Warning(LOCATION, "Effect with name '%s' already exists!", effect.front().getName().c_str());
			return index;
		}
	}
#endif

	m_effects.emplace_back(std::move(effect));

	return ParticleEffectHandle(static_cast<ParticleEffectHandle::impl_type>(m_effects.size() - 1));
}

void ParticleManager::pageIn() {
	for (auto& effectList : m_effects) {
		for (auto& effect : effectList)
			effect.pageIn();
	}
}

ParticleSourceWrapper ParticleManager::createSource(ParticleEffectHandle index)
{
	ParticleSourceWrapper wrapper;

	ParticleSource* source = createSource();
	source->setEffect(index);
	//TODO sources are no longer initialized properly for compound effects
	m_effects[index.value()].front().initializeSource(*source);
	//eff->initializeSource(*source);

	wrapper = ParticleSourceWrapper(source);
	wrapper.setCreationTimestamp(timestamp());

	return wrapper;
}

void ParticleManager::clearSources() {
	m_sources.clear();
	m_deferredSourceAdding.clear();
}

namespace util {
ParticleEffectHandle parseEffect(const SCP_string& objectName)
{
	SCP_string name;
	stuff_string(name, F_NAME);

	auto idx = ParticleManager::get()->getEffectByName(name);

	if (!idx.isValid()) {
		if (objectName.empty()) {
			error_display(0, "Unknown particle effect name '%s' encountered!", name.c_str());
		} else {
			error_display(0, "Unknown particle effect name '%s' encountered while parsing '%s'!", name.c_str(),
						  objectName.c_str());
		}
	}

	return idx;
}
}

namespace internal {
ParticleEffectHandle parseEffectElement(const SCP_string& name)
{
	if (!optional_string("$New Effect")) {
		SCP_string newName;
		stuff_string(newName, F_NAME);

		auto index = ParticleManager::get()->getEffectByName(newName);

		if (!index.isValid()) {
			error_display(0, "Unknown particle effect name '%s' encountered!", newName.c_str());
		}

		return index;
	}

	EffectType forcedType = parseEffectType();

	auto effect = constructEffect(name, forcedType);

	return ParticleEffectHandle::invalid();//;ParticleManager::get()->addEffect(effect);
}

bool required_string_if_new(const char* token, bool no_create) {
	if (no_create) {
		return optional_string(token) == 1;
	}

	required_string(token);
	return true;
}

SCP_vector<int> parseAnimationList(bool critical) {

	SCP_vector<SCP_string> bitmap_strings;
	
	// check to see if we are parsing a single value or list
	ignore_white_space();
	if (*Mp == '(') {
		// list of names case
		stuff_string_list(bitmap_strings);
	}
	else {
		// single name case
		SCP_string name;
		stuff_string(name, F_FILESPEC);
		bitmap_strings.push_back(std::move(name));
	}
	
	SCP_vector<int> handles;

	for (auto const &name: bitmap_strings) {
		auto handle = bm_load_animation(name.c_str());
		if (handle >= 0) {
			handles.push_back(handle);
		}
		else {
			int level = critical ? 1 : 0;
			error_display(level, "Failed to load effect %s!", name.c_str());
		}
	}

	return handles;
}

}
}
