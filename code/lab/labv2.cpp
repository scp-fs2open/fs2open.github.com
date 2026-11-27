#include "lab/labv2.h"
#include "lab/labv2_internal.h"
#include "lab/manager/lab_manager.h"
#include "gamesequence/gamesequence.h"
#include "graphics/gtao.h"
#include "graphics/rtao.h"
#include "graphics/shadows.h"
#include "starfield/starfield.h"

static std::unique_ptr<LabManager> LMGR;

const std::unique_ptr<LabManager> &getLabManager() {
	if (LMGR == nullptr) {
		LMGR.reset(new LabManager());
	}

	return LMGR;
}

namespace {
// The lab changes these settings directly, so we save them when the lab opens and restore them when it closes.
// Each LabRenderer setter that writes a game global must have a field here.
struct LabSavedSettings {
	ShadowRenderMethod shadow_render_method;
	RTShadowQuality rt_shadow_quality;
	int max_rt_shadow_lights;
	int max_rt_shadow_local_lights;
	float rt_shadow_bias_min;
	float rt_shadow_bias_max;
	int rt_shadow_samples_override;
	float sun_angular_size_override;
	int rtao_samples;
	bool shadow_contact_hardening_enabled;
	int gtao_quality;
	float gtao_radius_scale;
	float gtao_intensity;
};

bool Lab_settings_saved = false;
LabSavedSettings Lab_saved_settings;
}

void lab_init() {
	gr_set_clear_color(0, 0, 0);

	Lab_saved_settings = {Shadow_render_method,
		Rt_shadow_quality,
		Max_rt_shadow_lights,
		Max_rt_shadow_local_lights,
		Rt_shadow_bias_min,
		Rt_shadow_bias_max,
		Rt_shadow_samples_override,
		Sun_angular_size_override,
		Rtao_samples,
		Shadow_contact_hardening_enabled,
		Gtao_quality,
		Gtao_radius_scale,
		Gtao_intensity};
	Lab_settings_saved = true;
}

void lab_close(bool reset) {
	if (Lab_settings_saved) {
		Shadow_render_method = Lab_saved_settings.shadow_render_method;
		Rt_shadow_quality = Lab_saved_settings.rt_shadow_quality;
		Max_rt_shadow_lights = Lab_saved_settings.max_rt_shadow_lights;
		Max_rt_shadow_local_lights = Lab_saved_settings.max_rt_shadow_local_lights;
		Rt_shadow_bias_min = Lab_saved_settings.rt_shadow_bias_min;
		Rt_shadow_bias_max = Lab_saved_settings.rt_shadow_bias_max;
		Rt_shadow_samples_override = Lab_saved_settings.rt_shadow_samples_override;
		Sun_angular_size_override = Lab_saved_settings.sun_angular_size_override;
		Rtao_samples = Lab_saved_settings.rtao_samples;
		Shadow_contact_hardening_enabled = Lab_saved_settings.shadow_contact_hardening_enabled;
		Gtao_quality = Lab_saved_settings.gtao_quality;
		Gtao_radius_scale = Lab_saved_settings.gtao_radius_scale;
		Gtao_intensity = Lab_saved_settings.gtao_intensity;
		Lab_settings_saved = false;
	}
	// A lab-only debug view; the lab render flags are not reset elsewhere, so the game would keep it.
	Gtao_show_buffer = false;

	if (reset) {
		LMGR.reset();
	}
	gameseq_post_event(GS_EVENT_PREVIOUS_STATE);
}

void lab_do_frame(float frametime) {
	getLabManager()->onFrame(frametime);
}

