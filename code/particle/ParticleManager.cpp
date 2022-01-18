#include <algorithm>
#include <memory>

#include "particle/ParticleManager.h"

#include "particle/effects/SingleParticleEffect.h"
#include "particle/effects/CompositeEffect.h"
#include "particle/effects/VolumeEffect.h"

#include "particle/effects/ConeShape.h"
#include "particle/effects/SphereShape.h"
#include "particle/effects/GenericShapeEffect.h"

#include "bmpman/bmpman.h"
#include "globalincs/systemvars.h"
#include "tracing/tracing.h"

/**
 * @defgroup particleSystems Particle System
 */

namespace {
using namespace particle;

const char* effectTypeNames[static_cast<int64_t>(EffectType::MAX)] = {
	"Single",
	"Composite",
	"Cone",
	"Sphere",
	"Volume"
};

const char* getEffectTypeName(EffectType type) {
	Assertion(static_cast<int64_t>(type) >= static_cast<int64_t>(EffectType::Single)
				  && static_cast<int64_t>(type) < static_cast<int64_t>(EffectType::MAX),
			  "Invalid effect type specified!");

	return effectTypeNames[static_cast<int64_t>(type)];
}

ParticleEffectPtr constructEffect(const SCP_string& name, EffectType type) {
	using namespace effects;
	// Use an unique_ptr to make sure memory is deallocated if an exception is thrown
	std::unique_ptr<ParticleEffect> effect;

	switch (type) {
		case EffectType::Single: {
			effect.reset(new SingleParticleEffect(name));
			effect->parseValues(false);
			break;
		}
		case EffectType::Composite: {
			effect.reset(new CompositeEffect(name));
			effect->parseValues(false);
			break;
		}
		case EffectType::Cone: {
			effect.reset(new GenericShapeEffect<ConeShape>(name));
			effect->parseValues(false);
			break;
		}
		case EffectType::Sphere: {
			effect.reset(new GenericShapeEffect<SphereShape>(name));
			effect->parseValues(false);
			break;
		}
		case EffectType::Volume: {
			effect.reset(new VolumeEffect(name));
			effect->parseValues(false);
			break;
		}
		default: {
			Error(LOCATION, "Unimplemented effect type %d encountered! Get a coder!", static_cast<int>(type));
			throw std::runtime_error("Unimplemented effect type encountered!");
		}
	}

	return effect.release();
}

EffectType parseEffectType() {
	required_string("$Type:");

	SCP_string type;
	stuff_string(type, F_NAME);

	for (size_t i = 0; i < static_cast<size_t>(EffectType::MAX); ++i) {
		if (!stricmp(type.c_str(), effectTypeNames[i])) {
			return static_cast<EffectType>(i);
		}
	}

	error_display(0, "Unknown effect type '%s'!", type.c_str());
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

			ParticleManager::get()->addEffect(constructEffect(name, type));
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

void ParticleManager::init() {
	Assertion(m_manager == nullptr, "ParticleManager was not properly shut down!");

	m_manager.reset(new ParticleManager());

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
								 [&name](const std::shared_ptr<ParticleEffect>& ptr) {
									 return !stricmp(ptr->getName().c_str(), name.c_str());
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

ParticleEffectHandle ParticleManager::addEffect(ParticleEffectPtr effect)
{
	// we don't need this on standalone so remove the effect and return something invalid
	if (Is_standalone) {
		delete effect;

		return ParticleEffectHandle::invalid();
	}

	Assertion(effect, "Invalid effect pointer passed!");

#ifndef NDEBUG
	if (!effect->getName().empty()) {
		// This check is a bit expensive and will only be used in debug
		auto index = getEffectByName(effect->getName());

		if (index.isValid()) {
			Warning(LOCATION, "Effect with name '%s' already exists!", effect->getName().c_str());
			return index;
		}
	}
#endif

	m_effects.push_back(std::shared_ptr<ParticleEffect>(effect));

	return ParticleEffectHandle(static_cast<ParticleEffectHandle::impl_type>(m_effects.size() - 1));
}

void ParticleManager::pageIn() {
	for (auto& effect : m_effects) {
		effect->pageIn();
	}
}

ParticleSourceWrapper ParticleManager::createSource(ParticleEffectHandle index)
{
	ParticleEffectPtr eff = this->getEffect(index);
	ParticleSourceWrapper wrapper;

	if (eff->getType() == EffectType::Composite) {
		SCP_vector<ParticleSource*> sources;
		auto composite = static_cast<effects::CompositeEffect*>(eff);
		auto& childEffects = composite->getEffects();

		// UGH, HACK! To implement the source wrapper we need constant pointers to all sources.
		// To ensure this we reserve the number of sources we will need (current sources + sources being created)
		if (m_processingSources) {
			// If we are already in our onFrame, we need to apply the hack to the right vector though
			m_deferredSourceAdding.reserve(m_sources.size() + childEffects.size());
		} else {
			m_sources.reserve(m_sources.size() + childEffects.size());
		}

		for (auto& effect : childEffects) {
			ParticleSource* source = createSource();
			source->setEffect(effect);
			effect->initializeSource(*source);

			sources.push_back(source);
		}

		wrapper = ParticleSourceWrapper(std::move(sources));
	}
	else {
		ParticleSource* source = createSource();
		source->setEffect(eff);
		eff->initializeSource(*source);

		wrapper = ParticleSourceWrapper(source);
	}

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
ParticleEffectHandle parseEffectElement(EffectType forcedType, const SCP_string& name)
{
	if (!optional_string("$New Effect")) {
		SCP_string newName;
		stuff_string(newName, F_NAME);

		auto index = ParticleManager::get()->getEffectByName(newName);

		if (!index.isValid()) {
			error_display(0, "Unknown particle effect name '%s' encountered!", newName.c_str());
		}
		if (forcedType != EffectType::Invalid) {
			// Validate the effect type
			auto effect = ParticleManager::get()->getEffect(index);

			if (effect->getType() != forcedType) {
				error_display(0, "Particle effect '%s' has the wrong effect type! Expected %s but was %s!",
							  newName.c_str(), getEffectTypeName(forcedType), getEffectTypeName(effect->getType()));
			}
		}

		return index;
	}

	if (forcedType == EffectType::Invalid) {
		forcedType = parseEffectType();
	}

	auto effect = constructEffect(name, forcedType);

	return ParticleManager::get()->addEffect(effect);
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
		bitmap_strings.push_back(name);
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
