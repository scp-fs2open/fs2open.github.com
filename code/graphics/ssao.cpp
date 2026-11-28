#include "graphics/ssao.h"

#include "bmpman/bmpman.h"
#include "graphics/2d.h"
#include "graphics/matrix.h"
#include "graphics/openxr.h"
#include "options/Option.h"

int Ssao_samples = 0;
ssao_settings Ssao_settings;

// coverity[GLOBAL_INIT_ORDER] -- safe; OptionBuilder::finish() uses Meyers singleton
auto SsaoQualityOption = options::OptionBuilder<int>("Graphics.SsaoQuality",
                     std::pair<const char*, int>{"Screen-Space Ambient Occlusion", -1},
                     std::pair<const char*, int>{"Ambient occlusion from the screen contents (CORTAO)", -1})
                     .enumerator([]() -> SCP_vector<int> {
                         if (ssao_supported()) {
                             return {0, 4, 8, 16};
                         }
                         return {0};
                     })
                     .display(options::MapValueDisplay<int>({
                         {0, {"Off", -1}},
                         {4, {"Low", -1}},
                         {8, {"Medium", -1}},
                         {16, {"High", -1}}
                     }))
                     .bind_to(&Ssao_samples)
                     .flags({options::OptionFlags::ForceMultiValueSelection})
                     .level(options::ExpertLevel::Advanced)
                     .category(std::make_pair("Graphics", 1825))
                     .default_func([]() { return 0; })
                     .importance(70)
                     .finish();

bool ssao_supported()
{
	return gr_is_capable(gr_capability::CAPABILITY_SCREEN_SPACE_AO);
}

// CORTAO rebuilds the view positions with a perspective projection over the whole scene target.
static bool ssao_view_supported()
{
	// An orthographic projection, for example the lab camera, has a1d[15] == 1. A perspective projection has 0.
	if (gr_projection_matrix.a1d[15] != 0.0f) {
		return false;
	}
	// In a stereo frame, the engine sets the clip to the swap chain image of the eye.
	if (openxr_in_stereo_frame()) {
		return true;
	}
	// A script can draw the scene into a part of the screen (gr.setClip, then mn.renderFrame).
	return gr_screen.offset_x == 0 && gr_screen.offset_y == 0 && gr_screen.clip_width >= gr_screen.max_w
		&& gr_screen.clip_height >= gr_screen.max_h;
}

bool ssao_enabled()
{
	return Ssao_samples > 0 && ssao_supported() && ssao_view_supported();
}

int ssao_sample_count()
{
	// The shader has variants only for these counts. Round down, but never below 4.
	if (Ssao_samples >= 16) {
		return 16;
	}
	if (Ssao_samples >= 8) {
		return 8;
	}
	return 4;
}

SsaoFilterMode ssao_filter_mode(bool temporalFilterActive)
{
	if (Ssao_settings.filter != SsaoFilterMode::Auto) {
		return Ssao_settings.filter;
	}
	// The CORTAO filter only prepares the noise for a temporal filter. Without one, the larger
	// filter must remove more of the noise.
	return temporalFilterActive ? SsaoFilterMode::Small : SsaoFilterMode::Large;
}

bool ssao_animate_noise(bool temporalFilterActive)
{
	switch (Ssao_settings.noise) {
	case SsaoNoiseMode::Constant:
		return false;
	case SsaoNoiseMode::Animated:
		return true;
	case SsaoNoiseMode::Auto:
	default:
		return temporalFilterActive;
	}
}

bool ssao_bent_normals_active()
{
	return Ssao_settings.bent_normals && ENVMAP > 0;
}

void ssao_remove_unsupported_options()
{
	if (!ssao_supported()) {
		options::OptionsManager::instance()->removeOption(SsaoQualityOption);
	}
}
