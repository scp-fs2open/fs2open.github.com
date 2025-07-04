#include "particle/ParticleManager.h"
#include "particle/ParticleEffect.h"
#include "particle/volumes/ConeVolume.h"
#include "particle/volumes/PointVolume.h"
#include "particle/volumes/RingVolume.h"
#include "particle/volumes/SpheroidVolume.h"

#include <anl.h>

namespace particle {

	//
	// ------------ INDIVIDUAL FIELD PARSERS ------------
	//

	struct ParticleParse {
		static void parseBitmaps(ParticleEffect &effect) {
			if (internal::required_string_if_new("+Filename:", false)) {
				effect.m_bitmap_list = internal::parseAnimationList(true);
				effect.m_bitmap_range = ::util::UniformRange<size_t>(0, effect.m_bitmap_list.size() - 1);
			}
		}

		static void parseBitmapReversed(ParticleEffect &effect) {
			if (optional_string("+Animation Reversed:")) {
				stuff_boolean(&effect.m_reverseAnimation);
			}
		}

		template<bool modern = true> static void parseRadius(ParticleEffect &effect) {
			if (optional_string(modern ? "+Radius:" : "+Size:")) {
				effect.m_radius = ::util::ParsedRandomFloatRange::parseRandomRange();
			} else if (optional_string(modern ? "+Parent Radius Factor:" : "+Parent Size Factor:")) {
				effect.m_radius = ::util::ParsedRandomFloatRange::parseRandomRange();
				effect.m_parentScale = true;
			} else {
				error_display(1, modern ? "Missing +Radius or +Parent Radius Factor" : "Missing +Size or +Parent Size Factor");
			}
		}

		static void parseLength(ParticleEffect &effect) {
			if (optional_string("+Length:")) {
				effect.m_length = ::util::ParsedRandomFloatRange::parseRandomRange();
			}
		}

		static void parseLifetime(ParticleEffect &effect) {
			if (optional_string("+Lifetime:")) {
				if (optional_string("<none>")) {
					// Use lifetime of effect
					effect.m_hasLifetime = false;
				} else {
					effect.m_hasLifetime = true;
					effect.m_lifetime = ::util::ParsedRandomFloatRange::parseRandomRange();
				}
			} else if (optional_string("+Parent Lifetime Factor:")) {
				effect.m_hasLifetime = true;
				effect.m_parentLifetime = true;
				effect.m_lifetime = ::util::ParsedRandomFloatRange::parseRandomRange();
			}
		}

		template<bool modern = true> static void parseRotationType(ParticleEffect &effect) {
			if (optional_string(modern ? "+Bitmap Alignment:" : "+Rotation:")) {
				char buf[NAME_LENGTH];
				stuff_string(buf, F_NAME, NAME_LENGTH);
				if (!stricmp(buf, "DEFAULT")) {
					effect.m_rotation_type = ParticleEffect::RotationType::DEFAULT;
				} else if (!stricmp(buf, "RANDOM")) {
					effect.m_rotation_type = ParticleEffect::RotationType::RANDOM;
				} else if (!stricmp(buf, "SCREEN_ALIGNED") || !stricmp(buf, "SCREEN-ALIGNED") || !stricmp(buf, "SCREEN ALIGNED")) {
					effect.m_rotation_type = ParticleEffect::RotationType::SCREEN_ALIGNED;
				} else {
					// in the future we may want to support additional types, or even a specific angle, but that is TBD
					error_display(0, "Rotation Type %s not supported", buf);
				}
			}
		}

		template<bool modern = true> static void parseOffset(ParticleEffect& effect) {
			if (optional_string(modern ? "+Position Offset:" : "+Offset:")) {
				stuff_vec3d(&effect.m_manual_offset.emplace());
			}

			if (optional_string("+Velocity Offset:")) {
				stuff_vec3d(&effect.m_manual_velocity_offset.emplace());
			}
		}

		static void parseParentLocal(ParticleEffect& effect) {
			if (optional_string("+Remain local to parent:")) {
				stuff_boolean(&effect.m_parent_local);
			}
		}

		template<bool modern = true> static void parseParticleNumber(ParticleEffect &effect) {
			if (internal::required_string_if_new(modern ? "+Particle Count Per Spawn:" : "+Number:", false)) {
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

		static std::shared_ptr<ParticleVolume> parseVolume() {

			int type = required_string_one_of(4, "Spheroid", "Cone", "Ring", "Point"); //... and future volumes
			std::shared_ptr<ParticleVolume> volume;

			switch (type) {
				case 0:
					required_string("Spheroid"); //Because required string one of, for some reason, does not advance parsing.
					volume = std::make_shared<SpheroidVolume>();
					break;
				case 1:
					required_string("Cone");
					volume = std::make_shared<ConeVolume>();
					break;
				case 2:
					required_string("Ring");
					volume = std::make_shared<RingVolume>();
					break;
				case 3:
					required_string("Point");
					volume = std::make_shared<PointVolume>();
					break;
				default:
					UNREACHABLE("Invalid volume type specified!");
			}
			volume->parse();
			return volume;
		}

		template<bool modern = true> static void parseVelocityInherit(ParticleEffect &effect) {
			if (optional_string(modern ? "+Velocity Inherit:" : "+Parent Velocity Factor:")) {
				effect.m_vel_inherit = ::util::ParsedRandomFloatRange::parseRandomRange();
			}
			else if (optional_string("+Absolute Velocity Inherit:")) {
				effect.m_vel_inherit = ::util::ParsedRandomFloatRange::parseRandomRange();
				effect.m_vel_inherit_absolute = true;
			}

			if (optional_string("+Ignore Velocity Inherit If Parented:")) {
				stuff_boolean(&effect.m_ignore_velocity_inherit_if_has_parent);
			}
		}

		static void parseVelocityVolume(ParticleEffect &effect) {
			if (optional_string("+Velocity Volume:")) {
				effect.m_velocityVolume = parseVolume();
			}
		}

		static void parseVelocityNoise(ParticleEffect &effect) {
			if (optional_string("+Velocity Noise:")) {
				SCP_string func;
				stuff_string(func, F_RAW);
				anl::CKernel kernel;
				anl::CExpressionBuilder builder(kernel);
				anl::CInstructionIndex instruction = builder.eval(func);
				effect.m_velocityNoise = std::make_shared<std::pair<anl::CKernel, anl::CInstructionIndex>>(std::move(kernel), std::move(instruction));
			}
			if (optional_string("+Velocity Noise Scale:")) {
				effect.m_velocity_noise_scaling = ::util::ParsedRandomFloatRange::parseRandomRange();
			}
		}

		template<bool modern = true> static void parseVelocityVolumeScale(ParticleEffect &effect) {
			if constexpr (modern) {
				if (optional_string("+Velocity Volume Scale:")) {
					effect.m_velocity_scaling = ::util::ParsedRandomFloatRange::parseRandomRange();
				}
			}
			else {
				if (internal::required_string_if_new("+Velocity:", false)) {
					effect.m_velocity_scaling = ::util::ParsedRandomFloatRange::parseRandomRange();
				}
			}
		}

		static void parseVelocityDirectionScale(ParticleEffect &effect) {
			if (optional_string("+Velocity Direction Scale:")) {
				SCP_string dirStr;
				stuff_string(dirStr, F_NAME);

				if (!stricmp(dirStr.c_str(), "None") || !stricmp(dirStr.c_str(), "Normal")) {
					effect.m_velocity_directional_scaling = ParticleEffect::VelocityScaling::NONE;
				} else if (!stricmp(dirStr.c_str(), "Dot")) {
					effect.m_velocity_directional_scaling = ParticleEffect::VelocityScaling::DOT;
				} else if (!stricmp(dirStr.c_str(), "Inverse Dot")) {
					effect.m_velocity_directional_scaling = ParticleEffect::VelocityScaling::DOT_INVERSE;
				} else {
					error_display(0, "Unknown velocity direction scaling name '%s'!", dirStr.c_str());
				}
			}
		}

		static void parsePositionVolume(ParticleEffect &effect) {
			if (optional_string("+Spawn Position Volume:")) {
				effect.m_spawnVolume = parseVolume();
			}
		}

		static void parsePositionNoise(ParticleEffect &effect) {
			if (optional_string("+Spawn Position Noise:")) {
				SCP_string func;
				stuff_string(func, F_RAW);
				anl::CKernel kernel;
				anl::CExpressionBuilder builder(kernel);
				anl::CInstructionIndex instruction = builder.eval(func);
				effect.m_spawnNoise = std::make_shared<std::pair<anl::CKernel, anl::CInstructionIndex>>(std::move(kernel), std::move(instruction));
			}
			if (optional_string("+Spawn Position Noise Scale:")) {
				effect.m_position_noise_scaling = ::util::ParsedRandomFloatRange::parseRandomRange();
			}
		}

		static void parseVelocityInheritFromPosition(ParticleEffect &effect) {
			if (optional_string("+Velocity From Position:")) {
				effect.m_vel_inherit_from_position.emplace(::util::ParsedRandomFloatRange::parseRandomRange());
			}
			else if (optional_string("+Absolute Velocity From Position:")) {
				effect.m_vel_inherit_from_position.emplace(::util::ParsedRandomFloatRange::parseRandomRange());
				effect.m_vel_inherit_from_position_absolute = true;
			}
		}

		static void parseVelocityInheritFromOrientation(ParticleEffect &effect) {
			if (optional_string("+Velocity From Orientation:")) {
				effect.m_vel_inherit_from_orientation.emplace(::util::ParsedRandomFloatRange::parseRandomRange());
			}
		}

		template<bool modern = true> static void parseTiming(ParticleEffect &effect) {
			if (optional_string("+Duration:")) {
				if (optional_string("ONETIME")) {
					effect.m_duration = ParticleEffect::Duration::ONETIME;
				}
				else if (optional_string("Always")) {
					effect.m_duration = ParticleEffect::Duration::ALWAYS;
				}
				else {
					effect.m_duration = ParticleEffect::Duration::RANGE;
					effect.m_durationRange = ::util::ParsedRandomFloatRange::parseRandomRange(0.0f);
				}
			}

			if (optional_string("+Delay:")) {
				effect.m_delayRange = ::util::ParsedRandomFloatRange::parseRandomRange(0.0f);
			}

			if (optional_string(modern ? "+Spawns per Second:" : "+Effects per second:")) {
				effect.m_particlesPerSecond = ::util::ParsedRandomFloatRange::parseRandomRange();
				if (effect.m_particlesPerSecond.min() < 0.001f) {
					error_display(0, "Invalid effects per second minimum %f. Setting was disabled.", effect.m_particlesPerSecond.min());
					effect.m_particlesPerSecond = ::util::UniformFloatRange(-1.f);
				}
				if (effect.m_particlesPerSecond.max() > 1000.0f) {
					error_display(0, "Effects per second maximum %f is above 1000. Delay between effects will be clamped to 1 millisecond.", effect.m_particlesPerSecond.max());
				}
			}
		}

		static void parseModularCurvesLifetime(ParticleEffect& effect) {
			//TODO The following loop behaves as a true subset of how parsing will work once the particle modular curve set is implemented.
			//As such, once that's added the loop can be replaced with a modular_curve_set.parse without worry about breaking tables.
			while (optional_string("$Particle Lifetime Curve:")) {
				required_string("+Input: Lifetime");

				required_string("+Output:");
				int output = required_string_one_of(2, "Radius", "Velocity");
				//The required string part enforces this to be either 0 or 1
				required_string(output == 0 ? "Radius" : "Velocity");
				int& curve = output == 0 ? effect.m_size_lifetime_curve : effect.m_vel_lifetime_curve;

				required_string_either("+Curve Name:", "+Curve:", true);
				curve = curve_parse(" Unknown curve requested for modular curves!");
			}
		}

		static void parseModularCurvesSource(ParticleEffect& effect) {
			effect.m_modular_curves.parse("$Particle Source Curve:");
		}

		//
		// ------------ MODERN TABLES CODE ------------
		//

		static void parseModernTrail(ParticleEffect& effect, bool top_layer) {
			if (optional_string("$Trail Effect:")) {
				SCP_string name;
				stuff_string(name, F_NAME);

				effect.m_particleTrail = ParticleManager::get()->getEffectByName(name);
			}
			else if (top_layer && optional_string("$Trail Inline Effect:")) {
				SCP_string name;
				stuff_string(name, F_NAME);

				SCP_vector<ParticleEffect> trail {constructModernEffect(name, false)};
				while (optional_string("$Continue Trail Effect:"))
					trail.emplace_back(constructModernEffect(name));

				effect.m_particleTrail = ParticleManager::get()->addEffect(std::move(trail));
			}
		}

		static ParticleEffect constructModernEffect(const SCP_string& name, bool top_layer = true) {
			ParticleEffect effect(name);

			//Particle Settings
			parseBitmaps(effect);
			parseBitmapReversed(effect);
			parseRotationType(effect);
			parseRadius(effect);
			parseLength(effect);
			parseLifetime(effect);
			parseParentLocal(effect);

			//Spawner Settings
			parseTiming(effect);
			parseParticleNumber(effect);
			parseDirection(effect);
			parseOffset(effect);
			parsePositionVolume(effect);
			parsePositionNoise(effect);
			parseVelocityVolume(effect);
			parseVelocityVolumeScale(effect);
			parseVelocityNoise(effect);
			parseVelocityDirectionScale(effect);
			parseVelocityInheritFromPosition(effect);
			parseVelocityInheritFromOrientation(effect);
			parseVelocityInherit(effect);

			//Curves
			parseModularCurvesLifetime(effect);
			parseModularCurvesSource(effect);

			parseModernTrail(effect, top_layer);

			return effect;
		}

		//
		// ------------ LEGACY TABLES CODE ------------
		//

		static void parseSizeLifetimeCurve(ParticleEffect &effect) {
			if (optional_string("+Size over lifetime curve:")) {
				effect.m_size_lifetime_curve = curve_parse("");
			}
		}

		static void parseVelocityLifetimeCurve(ParticleEffect &effect) {
			if (optional_string("+Velocity scalar over lifetime curve:")) {
				effect.m_vel_lifetime_curve = curve_parse("");
			}
		}

		static void parseParticleProperties(ParticleEffect &effect) {
			//Emulates parsing in the legacy order, analogous to the old particle properties
			parseBitmaps(effect);
			parseRadius<false>(effect);
			parseLength(effect);
			parseLifetime(effect);
			parseSizeLifetimeCurve(effect);
			parseVelocityLifetimeCurve(effect);
			parseRotationType<false>(effect);
			parseOffset<false>(effect);
			parseParentLocal(effect);
		}

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

				auto effect = ParticleManager::get()->getEffect(index);
				if (!effect.empty()) {
					effect.front().m_name = name;
				}

				return effect;
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
					parseVelocityInherit<false>(effect);
					parseTiming<false>(effect);
					break;
				case ParticleEffectLegacyType::Composite: {
					bool first = true;
					result.pop_back();
					while (optional_string("+Child effect:")) {
						const auto& child_effects = parseLegacyCompositeElement(first ? name : "");
						result.insert(result.end(), std::make_move_iterator(child_effects.begin()), std::make_move_iterator(child_effects.end()));
						first = false;
					}
					if (result.empty()) {
						error_display(0, "Composite effect %s must have at least one child effect!", name.c_str());
					}
					break;
				}
				case ParticleEffectLegacyType::Cone: {
					parseParticleProperties(effect);
					effect.m_velocity_directional_scaling = ParticleEffect::VelocityScaling::DOT;

					if (required_string("+Deviation:")) {
						float deviation;
						stuff_float(&deviation);

						if (deviation < 0.001f) {
							error_display(0, "A standard deviation of %f is not valid. Must be greater than 0. Defaulting to 1.", deviation);
							deviation = 1.0f;
						}

						effect.m_velocityVolume = std::make_shared<ConeVolume>(::util::BoundedNormalFloatRange(::util::BoundedNormalDistribution::param_type{ std::normal_distribution<float>::param_type(0.f, fl_radians(deviation)), -PI, PI }), 1.f);
					}

					parseVelocityVolumeScale<false>(effect);
					parseParticleNumber<false>(effect);
					parseLegacyChance(effect);
					parseDirection(effect);

					bool saw_deprecated_effect_location = false;
					if (optional_string("+Trail effect:")) {
						// This is the deprecated location since this introduces ambiguities in the parsing process
						effect.m_particleTrail = parseLegacyCompositeElementByID("");
						saw_deprecated_effect_location = true;
					}

					parseVelocityInherit<false>(effect);
					parseTiming<false>(effect);

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

					parseVelocityVolumeScale<false>(effect);
					parseParticleNumber<false>(effect);
					parseLegacyChance(effect);
					parseDirection(effect);

					bool saw_deprecated_effect_location = false;
					if (optional_string("+Trail effect:")) {
						// This is the deprecated location since this introduces ambiguities in the parsing process
						effect.m_particleTrail = parseLegacyCompositeElementByID("");
						saw_deprecated_effect_location = true;
					}

					parseVelocityInherit<false>(effect);
					parseTiming<false>(effect);

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

					parseParticleNumber<false>(effect);
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

					parseVelocityInherit<false>(effect);
					parseTiming<false>(effect);
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

					if (type == ParticleEffectLegacyType::Invalid) {
						SCP_vector<ParticleEffect> effect {constructModernEffect(name)};
						while (optional_string("$Continue Effect:"))
							effect.emplace_back(constructModernEffect(name));
						ParticleManager::get()->addEffect(std::move(effect));
					}
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