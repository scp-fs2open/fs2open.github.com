#include "graphics/gtao.h"

#include "localization/localize.h"
#include "options/Option.h"
#include "options/OptionsManager.h"

#include <algorithm>

int Gtao_quality = 0;
float Gtao_radius_scale = 1.0f;
float Gtao_intensity = 1.5f;

// coverity[GLOBAL_INIT_ORDER] -- safe; OptionBuilder::finish() uses Meyers singleton
static auto GtaoQualityOption __UNUSED = options::OptionBuilder<int>("Graphics.SsaoQuality",
                     std::pair<const char*, int>{"Screen-Space Ambient Occlusion", -1},
                     std::pair<const char*, int>{"Darkens the ambient light in creases and gaps of ships, from what is on screen. OpenGL renderer with deferred lighting only; the Vulkan renderer offers Raytraced Ambient Occlusion instead, on ray tracing hardware", -1})
                     .values({{0, {"Off", -1}}, {1, {"Low", -1}}, {2, {"Medium", -1}}, {3, {"High", -1}}})
                     .bind_to(&Gtao_quality)
                     .flags({options::OptionFlags::ForceMultiValueSelection})
                     .level(options::ExpertLevel::Advanced)
                     .category(std::make_pair("Graphics", 1825))
                     .default_func([]() { return 0; })
                     .importance(70)
                     .finish();

// coverity[GLOBAL_INIT_ORDER] -- safe; OptionBuilder::finish() uses Meyers singleton
static auto GtaoRadiusOption __UNUSED = options::OptionBuilder<float>("Graphics.SsaoRadius",
                     std::pair<const char*, int>{"Screen-Space Ambient Occlusion Radius", -1},
                     std::pair<const char*, int>{"Multiplier on the size of the area that screen-space ambient occlusion looks at", -1})
                     .range(0.25f, 3.0f)
                     .bind_to(&Gtao_radius_scale)
                     .level(options::ExpertLevel::Advanced)
                     .category(std::make_pair("Graphics", 1825))
                     .default_func([]() { return 1.0f; })
                     .importance(69)
                     .finish();

// coverity[GLOBAL_INIT_ORDER] -- safe; OptionBuilder::finish() uses Meyers singleton
static auto GtaoIntensityOption __UNUSED = options::OptionBuilder<float>("Graphics.SsaoIntensity",
                     std::pair<const char*, int>{"Screen-Space Ambient Occlusion Intensity", -1},
                     std::pair<const char*, int>{"How dark screen-space ambient occlusion makes the ambient light", -1})
                     .range(0.5f, 4.0f)
                     .bind_to(&Gtao_intensity)
                     .level(options::ExpertLevel::Advanced)
                     .category(std::make_pair("Graphics", 1825))
                     .default_func([]() { return 1.5f; })
                     .importance(68)
                     .finish();

gtao_sample_counts gtao_samples(int quality)
{
	// The shader loops over these counts, so clamp a level from a hand-edited config file.
	static const gtao_sample_counts levels[] = {{0, 0}, {2, 4}, {3, 6}, {4, 8}};
	return levels[std::clamp(quality, 0, 3)];
}

// A fixed world-space radius cannot serve both a 12 m fighter and a 1.6 km capital ship:
// a radius that suits the fighter is under one pixel on the far ship, and one that suits
// the ship puts a halo around the fighter. So the world radius grows with view depth:
//
//   world_radius = 0.03 * radius_scale * depth
//
// Projected to the screen, depth cancels out, and the radius in pixels is
//
//   pixels = 0.03 * radius_scale * proj11 * screen_height / 2
//
// The shader gets only this pixel radius and derives the world radius of each pixel from it.
// The clamp keeps the cost bounded (upper limit) and keeps at least one sample away from
// the center texel (lower limit). The upper limit is a fraction of the screen height so that
// the look does not change with the resolution.
float gtao_pixel_radius(float radius_scale, float proj11, int screen_height)
{
	constexpr float radius_per_depth = 0.03f;
	const float pixels = radius_per_depth * radius_scale * proj11 * 0.5f * static_cast<float>(screen_height);
	return std::clamp(pixels, 2.0f, static_cast<float>(screen_height) / 8.0f);
}

// The options menu greys the options out by the same rule, see gtao_set_option_requirements().
static const char* const Gtao_requirement = "opengl";

bool gtao_supported(GraphicsAPI api)
{
	return gr_option_requirement_met(Gtao_requirement, api, false);  // the requirement is "opengl", which needs no ray tracing
}

bool gtao_enabled()
{
	// Check the renderer here: an option with an unmet requirement keeps the player's
	// stored value, so Gtao_quality can be above 0 on a renderer without the pass.
	return Gtao_quality > 0 && gtao_supported(gr_screen.mode);
}

void gtao_set_option_requirements()
{
	auto* mgr = options::OptionsManager::instance();
	const SCP_string explanation = XSTR("Only the OpenGL renderer has screen-space ambient occlusion.", -1);
	for (auto key : {"Graphics.SsaoQuality", "Graphics.SsaoRadius", "Graphics.SsaoIntensity"}) {
		mgr->setRequirement(key, Gtao_requirement, explanation);
	}
}
