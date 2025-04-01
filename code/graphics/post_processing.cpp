
#include "post_processing.h"

#include "graphics/grinternal.h"
#include "options/Option.h"
#include "parse/parselo.h"
#include "ship/ship.h"
#include "starfield/supernova.h"

namespace graphics {
namespace {

PostEffectUniformType mapUniformNameToType(const SCP_string& uniform_name)
{
	if (!stricmp(uniform_name.c_str(), "noise_amount")) {
		return PostEffectUniformType::NoiseAmount;
	} else if (!stricmp(uniform_name.c_str(), "saturation")) {
		return PostEffectUniformType::Saturation;
	} else if (!stricmp(uniform_name.c_str(), "brightness")) {
		return PostEffectUniformType::Brightness;
	} else if (!stricmp(uniform_name.c_str(), "contrast")) {
		return PostEffectUniformType::Contrast;
	} else if (!stricmp(uniform_name.c_str(), "film_grain")) {
		return PostEffectUniformType::FilmGrain;
	} else if (!stricmp(uniform_name.c_str(), "tv_stripes")) {
		return PostEffectUniformType::TvStripes;
	} else if (!stricmp(uniform_name.c_str(), "cutoff")) {
		return PostEffectUniformType::Cutoff;
	} else if (!stricmp(uniform_name.c_str(), "dither")) {
		return PostEffectUniformType::Dither;
	} else if (!stricmp(uniform_name.c_str(), "tint")) {
		return PostEffectUniformType::Tint;
	} else {
		error_display(0, "Unknown uniform name '%s'!", uniform_name.c_str());
		return PostEffectUniformType::Invalid;
	}
}


// used by In-Game Options menu
bool Post_processing_enable_lightshafts = true;

void parse_lightshafts_func()
{
	bool enabled;
	stuff_boolean(&enabled);
	Post_processing_enable_lightshafts = enabled;
}

// used by In-Game Options menu
bool Post_processing_enable_sunglare = true;

void parse_sunglare_func()
{
	bool enabled;
	stuff_boolean(&enabled);
	Post_processing_enable_sunglare = enabled;
}

auto LightshaftsOption = options::OptionBuilder<bool>("Graphics.Lightshafts",
                     std::pair<const char*, int>{"Lightshafts", 1724},
                     std::pair<const char*, int>{"Enables or disables lightshafts (requires post-processing)", 1725})
                     .category(std::make_pair("Graphics", 1825))
                     .default_func([]() { return Post_processing_enable_lightshafts;})
                     .level(options::ExpertLevel::Advanced)
                     .bind_to(&Post_processing_enable_lightshafts)
                     .importance(60)
                     .parser(parse_lightshafts_func)
                     .finish();

auto SunglareOption = options::OptionBuilder<bool>("Graphics.Sunglare",
					std::pair<const char*, int>{"Sunglare", 1880},
					std::pair<const char*, int>{"Enables or disables glare from suns", 1881})
			.category(std::make_pair("Graphics", 1825))
			.default_func([]() { return Post_processing_enable_sunglare;})
			.level(options::ExpertLevel::Advanced)
			.bind_to(&Post_processing_enable_sunglare)
			.importance(61)
			.parser(parse_sunglare_func)
			.finish();

int Post_processing_bloom_intensity = 25; // using default value of Cmdline_bloom_intensity

void parse_bloom_intensity_func()
{
	int value;
	stuff_int(&value);
	CLAMP(value, 0, 200);
	Post_processing_bloom_intensity = value;
}

static auto BloomIntensityOption __UNUSED = options::OptionBuilder<int>("Graphics.BloomIntensity",
                     std::pair<const char*, int>{"Bloom intensity", 1701},
                     std::pair<const char*, int>{"Sets the bloom intensity (requires post-processing)", 1702})
                     .category(std::make_pair("Graphics", 1825))
                     .range(0, 200)
                     .level(options::ExpertLevel::Advanced)
                     .default_func([](){return Post_processing_bloom_intensity;})
                     .bind_to(&Post_processing_bloom_intensity)
                     .importance(55)
                     .flags({options::OptionFlags::RangeTypeInteger})
                     .parser(parse_bloom_intensity_func)
                     .finish();
} // namespace

bool PostProcessingManager::parse_table()
{

	bool warned = false;

	try {
		if (cf_exists_full("post_processing.tbl", CF_TYPE_TABLES))
			read_file_text("post_processing.tbl", CF_TYPE_TABLES);
		else
			read_file_text_from_default(defaults_get_file("post_processing.tbl"));

		reset_parse();

		if (optional_string("#Effects")) {
			while (!required_string_one_of(3, "$Name:", "#Ship Effects", "#End")) {
				post_effect_t eff;

				required_string("$Name:");
				stuff_string(eff.name, F_NAME);

				required_string("$Uniform:");

				SCP_string tbuf;
				stuff_string(tbuf, F_NAME);
				eff.uniform_type = mapUniformNameToType(tbuf);

				required_string("$Define:");
				stuff_string(eff.define_name, F_NAME);

				required_string("$AlwaysOn:");
				stuff_boolean(&eff.always_on);

				required_string("$Default:");
				stuff_float(&eff.default_intensity);
				eff.intensity = eff.default_intensity;

				required_string("$Div:");
				stuff_float(&eff.div);

				required_string("$Add:");
				stuff_float(&eff.add);

				if (optional_string("$RGB:")) {
					stuff_vec3d(&eff.rgb);
				}

				// Post_effects index is used for flag checks, so we can't have more than 32
				if (m_postEffects.size() < 32) {
					m_postEffects.push_back(eff);
				} else if (!warned) {
					mprintf(("WARNING: post_processing.tbl can only have a max of 32 effects! Ignoring extra...\n"));
					warned = true;
				}
			}
		}

		// Built-in per-ship effects
		ship_effect se1;
		strcpy_s(se1.name, "FS1 Ship select");
		se1.shader_effect = 0;
		se1.disables_rendering = false;
		se1.invert_timer = false;
		Ship_effects.push_back(se1);

		if (optional_string("#Ship Effects")) {
			while (!required_string_one_of(3, "$Name:", "#Light Shafts", "#End")) {
				ship_effect se;
				char tbuf[NAME_LENGTH] = {0};

				required_string("$Name:");
				stuff_string(tbuf, F_NAME, NAME_LENGTH);
				strcpy_s(se.name, tbuf);

				required_string("$Shader Effect:");
				stuff_int(&se.shader_effect);

				required_string("$Disables Rendering:");
				stuff_boolean(&se.disables_rendering);

				required_string("$Invert timer:");
				stuff_boolean(&se.invert_timer);

				Ship_effects.push_back(se);
			}
		}

		if (optional_string("#Light Shafts")) {
			required_string("$AlwaysOn:");
			stuff_boolean(&m_lightshaftParams.on);
			required_string("$Density:");
			stuff_float(&m_lightshaftParams.density);
			required_string("$Falloff:");
			stuff_float(&m_lightshaftParams.falloff);
			required_string("$Weight:");
			stuff_float(&m_lightshaftParams.weight);
			required_string("$Intensity:");
			stuff_float(&m_lightshaftParams.intensity);
			required_string("$Sample Number:");
			stuff_int(&m_lightshaftParams.samplenum);

			m_lightshaftParams.cpintensity = m_lightshaftParams.weight;

			float falloff = m_lightshaftParams.falloff;
			for (int i = 1; i < m_lightshaftParams.samplenum; ++i)
			{
				m_lightshaftParams.cpintensity += m_lightshaftParams.weight * falloff;
				falloff *= m_lightshaftParams.falloff;	// this replaces pow(falloff, i)
			}

			m_lightshaftParams.cpintensity *= m_lightshaftParams.intensity;
		}

		required_string("#End");

		return true;
	} catch (const parse::ParseException& e) {
		mprintf(("Unable to parse 'post_processing.tbl'!  Error message = %s.\n", e.what()));
		return false;
	}
}

void PostProcessingManager::clear()
{
	m_postEffects.clear();
	m_lightshaftParams = lightshaft_parameters();
}

const SCP_vector<graphics::post_effect_t>& PostProcessingManager::getPostEffects() const { return m_postEffects; }
SCP_vector<graphics::post_effect_t>& PostProcessingManager::getPostEffects() { return m_postEffects; }

const lightshaft_parameters& PostProcessingManager::getLightshaftParams() const { return m_lightshaftParams; }
lightshaft_parameters& PostProcessingManager::getLightshaftParams() { return m_lightshaftParams; }

bool PostProcessingManager::bloomShadersOk() const
{
	return m_bloomShadersOk;
}

void PostProcessingManager::setBloomShadersOk(bool ok)
{
	m_bloomShadersOk = ok;
}
} // namespace graphics

bool gr_lightshafts_enabled()
{
	if (gr_screen.mode == GR_STUB) {
		return false;
	}

	// supernova glare should switch to legacy lightshafts
	if (supernova_stage() >= SUPERNOVA_STAGE::CLOSE) {
		return false;
	}

	if (!graphics::Post_processing_manager->getLightshaftParams().on) {
		return false;
	}

	return graphics::LightshaftsOption->getValue();
}

bool gr_sunglare_enabled()
{
	if (gr_screen.mode == GR_STUB) {
		return false;
	}

	// supernova glare gets to override this and actually display glare
	if (supernova_stage() >= SUPERNOVA_STAGE::CLOSE) {
		return true;
	}

	return graphics::SunglareOption->getValue();
}

int gr_bloom_intensity()
{
	if (gr_screen.mode == GR_STUB) {
		return 0;
	}

	if (graphics::Post_processing_manager == nullptr || !graphics::Post_processing_manager->bloomShadersOk()) {
		return 0;
	}

	return graphics::BloomIntensityOption->getValue();
}

void gr_set_bloom_intensity(int intensity)
{
	if (gr_screen.mode == GR_STUB) {
		return;
	}

	graphics::Post_processing_bloom_intensity = intensity;
	options::OptionsManager::instance()->set_ingame_range_option("Graphics.BloomIntensity", intensity);
}
