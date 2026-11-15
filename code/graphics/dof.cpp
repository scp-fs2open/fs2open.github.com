#include "graphics/dof.h"

#include <algorithm>
#include <cmath>
#include <variant>

#include "graphics/2d.h"
#include "localization/localize.h"
#include "options/Option.h"
#include "options/OptionsManager.h"
#include "render/3d.h"

namespace graphics {

static bool Dof_enabled = false;
static int Dof_max_blur = 16;
static int Dof_near_dist = 50;

// coverity[GLOBAL_INIT_ORDER] -- safe; OptionBuilder::finish() uses Meyers singleton
static auto DofOption = options::OptionBuilder<bool>("Graphics.DepthOfField",
                     std::pair<const char*, int>{"Depth of field", -1},
                     std::pair<const char*, int>{"Blurs what is very near the camera, for example the hull of a ship that you fly past. "
                                                 "Everything farther away and your cockpit stay sharp (requires post-processing, OpenGL only)", -1})
                     .category(std::make_pair("Graphics", 1825))
                     .level(options::ExpertLevel::Advanced)
                     .default_val(false)
                     .bind_to(&Dof_enabled)
                     .importance(59)
                     .finish();

// coverity[GLOBAL_INIT_ORDER] -- safe; OptionBuilder::finish() uses Meyers singleton
static auto DofMaxBlurOption = options::OptionBuilder<int>("Graphics.DepthOfFieldMaxBlur",
                     std::pair<const char*, int>{"Depth of field max blur", -1},
                     std::pair<const char*, int>{"Largest blur radius, in pixels at 1080 lines and the default field of view. "
                                                 "The blur grows with the resolution and when the view zooms in", -1})
                     .category(std::make_pair("Graphics", 1825))
                     .level(options::ExpertLevel::Advanced)
                     .range(1, 32)
                     .default_val(16)
                     .bind_to(&Dof_max_blur)
                     .importance(58)
                     .flags({options::OptionFlags::RangeTypeInteger})
                     .finish();

// coverity[GLOBAL_INIT_ORDER] -- safe; OptionBuilder::finish() uses Meyers singleton
static auto DofDistanceOption = options::OptionBuilder<int>("Graphics.DepthOfFieldDistance",
                     std::pair<const char*, int>{"Depth of field distance", -1},
                     std::pair<const char*, int>{"Objects nearer than this distance, in metres, start to blur. "
                                                 "At half this distance, the blur is at its largest", -1})
                     .category(std::make_pair("Graphics", 1825))
                     .level(options::ExpertLevel::Advanced)
                     .range(5, 150)
                     .default_val(50)
                     .bind_to(&Dof_near_dist)
                     .importance(57)
                     .flags({options::OptionFlags::RangeTypeInteger})
                     .finish();

float dof_coc(float z, float near_dist, float max_blur)
{
	return std::clamp(near_dist / std::max(z, 1.0e-3f) - 1.0f, 0.0f, 1.0f) * max_blur;
}

// The vertical field of view of the default view: g3_set_fov() multiplies the zoom (DEFAULT_FOV, 0.75)
// by PROJ_FOV_FACTOR.
constexpr float DOF_REFERENCE_FOV = 0.75f * PROJ_FOV_FACTOR;
constexpr float DOF_REFERENCE_LINES = 1080.0f;

float dof_pixel_scale(float screen_h, float fov_y)
{
	return (screen_h / tanf(fov_y * 0.5f)) / (DOF_REFERENCE_LINES / tanf(DOF_REFERENCE_FOV * 0.5f));
}

int dof_samples(float radius)
{
	const float taps = 96.0f * (radius / 16.0f) * (radius / 16.0f);
	return static_cast<int>(std::clamp(taps, 32.0f, static_cast<float>(DOF_MAX_SAMPLES)));
}

bool dof_get_frame(dof_frame* out)
{
	if (!Dof_enabled) {
		return false;
	}
	// Limit: an asymmetric (VR) field of view uses the default field of view for the scale.
	const float fov_y = std::holds_alternative<float>(Proj_fov) ? std::get<float>(Proj_fov) : DOF_REFERENCE_FOV;
	out->near_dist = static_cast<float>(Dof_near_dist);
	out->max_blur = static_cast<float>(Dof_max_blur) * dof_pixel_scale(static_cast<float>(gr_screen.max_h), fov_y);
	return true;
}

void dof_set_requirements()
{
	const SCP_vector<std::shared_ptr<const options::OptionBase>> dof_options = {DofOption, DofMaxBlurOption,
		DofDistanceOption};
	for (const auto& option : dof_options) {
		options::OptionsManager::instance()->setRequirement(option->getConfigKey(), "opengl",
			XSTR("Needs the OpenGL renderer", -1));
	}
}

} // namespace graphics
