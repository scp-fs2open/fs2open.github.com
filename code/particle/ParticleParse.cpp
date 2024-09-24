#include "particle/ParticleManager.h"
#include "particle/ParticleSourceWrapper.h"
#include "particle/effects/OmniEffect.h"
#include "particle/volumes/ConeVolume.h"
#include "particle/volumes/SpheroidVolume.h"

namespace particle {

	//
	// ------------ INDIVIDUAL FIELD PARSERS ------------
	//

	struct ParticleParse {
		static void parseParticleProperties(ParticleEffect &effect) {
			//TODO Split up ParticleProperties
			effect.m_particleProperties.parse(false);
		}

		static void parseNumber(ParticleEffect &effect) {
			if (internal::required_string_if_new("+Number:", false)) {
				effect.m_particleNum = ::util::ParsedRandomFloatRange::parseRandomRange();
			}
		}

		static void parseDirection(ParticleEffect &effect) {
			if (optional_string("+Direction:")) {
				SCP_string dirStr;
				stuff_string(dirStr, F_NAME);

				if (!stricmp(dirStr.c_str(), "Incoming") || !stricmp(dirStr.c_str(), "Aligned")) {
					effect.m_direction = ParticleEffect::ShapeDirection::ALIGNED;
				} else if (!stricmp(dirStr.c_str(), "Normal") || !stricmp(dirStr.c_str(), "HitNormal")) {
					effect.m_direction = ParticleEffect::ShapeDirection::HIT_NORMAL;
				} else if (!stricmp(dirStr.c_str(), "Reflected")) {
					effect.m_direction = ParticleEffect::ShapeDirection::REFLECTED;
				} else if (!stricmp(dirStr.c_str(), "Reverse")) {
					effect.m_direction = ParticleEffect::ShapeDirection::REVERSE;
				} else {
					error_display(0, "Unknown direction name '%s'!", dirStr.c_str());
				}
			}
		}

		static void parseVelocityInherit(ParticleEffect &effect) {
			if (optional_string("+Parent Velocity Factor:")) {
				effect.m_vel_inherit = ::util::ParsedRandomFloatRange::parseRandomRange();
			}
		}

		static void parseVelocityVolume(ParticleEffect &effect) {
			//TODO
		}

		static void parseVelocityVolumeScale(ParticleEffect &effect) {
			if (internal::required_string_if_new("+Velocity:", false)) {
				effect.m_velocity_scaling = ::util::ParsedRandomFloatRange::parseRandomRange();
			}
		}

		static void parseTiming(ParticleEffect &effect) {
			//TODO integrate timing
			effect.m_timing = util::EffectTiming::parseTiming();
		}


		//
		// ------------ MODERN TABLES CODE ------------
		//

		static ParticleEffect constructModernEffect(const SCP_string& name) {
			//TODO
			return ParticleEffect(name);
		}

		//
		// ------------ LEGACY TABLES CODE ------------
		//

		enum class ParticleEffectLegacyType: int8_t {
			Invalid = -1,
			Single,
			Composite,
			Cone,
			Sphere,
			Volume,

			MAX,
		};
		static constexpr size_t legacyEffectTypeNamesMax = static_cast<size_t>(ParticleEffectLegacyType::MAX);
		static std::array<const char*, legacyEffectTypeNamesMax> legacyEffectTypeNames;

		static ParticleEffectLegacyType parseLegacyEffectType() {
			if(!optional_string("$Type:"))
				return ParticleEffectLegacyType::Invalid;

			SCP_string type;
			stuff_string(type, F_NAME);

			int i = string_lookup(type.c_str(), legacyEffectTypeNames, legacyEffectTypeNamesMax, "ParticleEffectLegacyType", true);
			if (i >= 0)
				return static_cast<ParticleEffectLegacyType>(i);
			else
				return ParticleEffectLegacyType::Invalid;
		}

		static SCP_vector<ParticleEffect> parseLegacyCompositeElement(const SCP_string &name) {
			if (!optional_string("$New Effect")) {
				SCP_string newName;
				stuff_string(newName, F_NAME);

				auto index = ParticleManager::get()->getEffectByName(newName);

				if (!index.isValid()) {
					error_display(0, "Unknown particle effect name '%s' encountered!", newName.c_str());
				}

				return ParticleManager::get()->getEffect(index);
			}

			ParticleEffectLegacyType type = parseLegacyEffectType();
			if (type == ParticleEffectLegacyType::Invalid) {
				error_display(0, "Legacy inline particle effect only accepts inline sub-effects with a legacy type!");
			}

			return constructLegacyEffect(name, type);
		}

		static ParticleEffectHandle parseLegacyCompositeElementByID(const SCP_string &name) {
			if (!optional_string("$New Effect")) {
				SCP_string newName;
				stuff_string(newName, F_NAME);

				auto index = ParticleManager::get()->getEffectByName(newName);

				if (!index.isValid()) {
					error_display(0, "Unknown particle effect name '%s' encountered!", newName.c_str());
				}

				return index;
			}

			ParticleEffectLegacyType type = parseLegacyEffectType();
			if (type == ParticleEffectLegacyType::Invalid) {
				error_display(0, "Legacy inline particle effect only accepts inline sub-effects with a legacy type!");
			}

			return ParticleManager::get()->addEffect(constructLegacyEffect(name, type));
		}

		static void parseLegacyChance(ParticleEffect& effect) {
			if (optional_string("+Chance:")) {
				float chance;
				stuff_float(&chance);
				if (chance <= 0.0f) {
					error_display(0, "Particle %s tried to set +Chance: %f\nChances below 0 would result in no particles.", effect.m_name.c_str(), chance);
				} else if (chance > 1.0f) {
					error_display(0, "Particle %s tried to set +Chance: %f\nChances above 1 are ignored, please use +Number: (min,max) to spawn multiple particles.", effect.m_name.c_str(), chance);
					chance = 1.0f;
				}
				effect.m_particleChance = chance;
			}
		}

		static SCP_vector<ParticleEffect> constructLegacyEffect(const SCP_string& name, ParticleEffectLegacyType type) {
			SCP_vector<ParticleEffect> result;
			result.emplace_back(name);
			ParticleEffect& effect = result.back();

			switch (type) {
				case ParticleEffectLegacyType::Single:
					parseParticleProperties(effect);
					parseVelocityInherit(effect);
					parseTiming(effect);
					break;
				case ParticleEffectLegacyType::Composite: {
					result.pop_back();
					while (optional_string("+Child effect:")) {
						const auto& child_effects = parseLegacyCompositeElement("");
						result.insert(result.end(), std::make_move_iterator(child_effects.begin()), std::make_move_iterator(child_effects.end()));
					}
					if (result.empty()) {
						error_display(0, "Composite effect %s must have at least one child effect!", name.c_str());
					}
					break;
				}
				case ParticleEffectLegacyType::Cone: {
					parseParticleProperties(effect);

					if (required_string("+Deviation:")) {
						float deviation;
						stuff_float(&deviation);

						if (deviation < 0.001f) {
							error_display(0, "A standard deviation of %f is not valid. Must be greater than 0. Defaulting to 1.", deviation);
							deviation = 1.0f;
						}

						effect.m_velocityVolume = std::make_shared<ConeVolume>(::util::BoundedNormalFloatRange(::util::BoundedNormalDistribution::param_type{ std::normal_distribution<float>::param_type(0.f, fl_radians(deviation)), -PI, PI }), 1.f);
					}

					parseVelocityVolumeScale(effect);
					parseNumber(effect);
					parseLegacyChance(effect);
					parseDirection(effect);

					bool saw_deprecated_effect_location = false;
					if (optional_string("+Trail effect:")) {
						// This is the deprecated location since this introduces ambiguities in the parsing process
						effect.m_particleTrail = parseLegacyCompositeElementByID("");
						saw_deprecated_effect_location = true;
					}

					parseVelocityInherit(effect);
					parseTiming(effect);

					if (optional_string("+Trail effect:")) {
						// This is the new and correct location. This might create duplicate effects but the warning should be clear
						// enough to avoid that
						if (saw_deprecated_effect_location) {
							error_display(0, "Found two trail effect options! Specifying '+Trail effect:' before '+Duration:' is "
											 "deprecated since that can cause issues with conflicting effect options.");
						}
						effect.m_particleTrail = parseLegacyCompositeElementByID("");
					}
					break;
				}
				case ParticleEffectLegacyType::Sphere: {
					parseParticleProperties(effect);

					effect.m_velocityVolume = make_shared<SpheroidVolume>(1.f, 1.f, 1.f);

					parseVelocityVolumeScale(effect);
					parseNumber(effect);
					parseLegacyChance(effect);
					parseDirection(effect);

					bool saw_deprecated_effect_location = false;
					if (optional_string("+Trail effect:")) {
						// This is the deprecated location since this introduces ambiguities in the parsing process
						effect.m_particleTrail = parseLegacyCompositeElementByID("");
						saw_deprecated_effect_location = true;
					}

					parseVelocityInherit(effect);
					parseTiming(effect);

					if (optional_string("+Trail effect:")) {
						// This is the new and correct location. This might create duplicate effects but the warning should be clear
						// enough to avoid that
						if (saw_deprecated_effect_location) {
							error_display(0, "Found two trail effect options! Specifying '+Trail effect:' before '+Duration:' is "
											 "deprecated since that can cause issues with conflicting effect options.");
						}
						effect.m_particleTrail = parseLegacyCompositeElementByID("");
					}
					break;
				}
				case ParticleEffectLegacyType::Volume: {
					parseParticleProperties(effect);

					effect.m_vel_inherit_from_position_absolute = true;
					//This is, unfortunately, not semantically identical to the other velocity, so this is handled manually
					if (required_string("+Velocity:")) {
						effect.m_vel_inherit_from_position.emplace(::util::ParsedRandomFloatRange::parseRandomRange());
					}

					parseNumber(effect);
					parseLegacyChance(effect);

					float radius = 10.f;
					if (required_string("+Volume radius:")) {
						stuff_float(&radius);

						if (radius < 0.001f) {
							error_display(0, "A volume radius of %f is not valid. Must be greater than 0. Defaulting to 10.", radius);
							radius = 10.0f;
						}
					}

					float bias = 1.f;
					if (optional_string("+Bias:")) {
						stuff_float(&bias);

						if (bias < 0.001f) {
							error_display(0, "A volume bias value of %f is not valid. Must be greater than 0.", bias);
							bias = 1.0f;
						}
					}

					float stretch = 1.f;
					if (optional_string("+Stretch:")) {
						stuff_float(&stretch);

						if (stretch < 0.001f) {
							error_display(0, "A volume stretch value of %f is not valid. Must be greater than 0.", stretch);
							stretch = 1.0f;
						}
					}

					effect.m_spawnVolume = make_shared<SpheroidVolume>(bias, stretch, radius);

					parseVelocityInherit(effect);
					parseTiming(effect);
					break;
				}
				default: {
					Error(LOCATION, "Unimplemented effect type %d encountered! Get a coder!", static_cast<int>(type));
					throw std::runtime_error("Unimplemented effect type encountered!");
				}
			}

			return result;
		}

		//
		// ------------ MAIN CALLBACK ------------
		//

		static void parseCallback(const char* fileName) {
			using namespace particle;

			try {
				read_file_text(fileName, CF_TYPE_TABLES);

				reset_parse();

				required_string("#Particle Effects");

				while (optional_string("$Effect:")) {
					SCP_string name;
					stuff_string(name, F_NAME);

					auto type = parseLegacyEffectType();

					if (type == ParticleEffectLegacyType::Invalid)
						ParticleManager::get()->addEffect(constructModernEffect(name));
					else
						ParticleManager::get()->addEffect(constructLegacyEffect(name, type));
				}

				required_string("#End");
			}
			catch (const parse::ParseException& e)
			{
				mprintf(("TABLES: Unable to parse '%s'!  Error message = %s.\n", fileName, e.what()));
				return;
			}
		}
	};

	void ParticleManager::parseConfigFiles() {
		parse_modular_table("*-part.tbm", ParticleParse::parseCallback);
	}

	std::array<const char*, ParticleParse::legacyEffectTypeNamesMax> ParticleParse::legacyEffectTypeNames = {
		"Single",
		"Composite",
		"Cone",
		"Sphere",
		"Volume"
	};
}