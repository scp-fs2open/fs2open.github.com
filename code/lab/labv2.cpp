#include "lab/labv2.h"
#include "lab/labv2_internal.h"
#include "lab/manager/lab_manager.h"
#include "gamesequence/gamesequence.h"
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
struct LabSavedSettings {
	RTShadowQuality rt_shadow_quality;
	float rt_shadow_bias_min;
	float rt_shadow_bias_max;
	int rt_shadow_samples_override;
	float sun_angular_size_override;
	int rtao_samples;
	bool shadow_contact_hardening_enabled;
};

bool Lab_settings_saved = false;
LabSavedSettings Lab_saved_settings;
}

void lab_init() {
	gr_set_clear_color(0, 0, 0);

	Lab_saved_settings = {Rt_shadow_quality,
		Rt_shadow_bias_min,
		Rt_shadow_bias_max,
		Rt_shadow_samples_override,
		Sun_angular_size_override,
		Rtao_samples,
		Shadow_contact_hardening_enabled};
	Lab_settings_saved = true;
}

void lab_close(bool reset) {
	if (Lab_settings_saved) {
		Rt_shadow_quality = Lab_saved_settings.rt_shadow_quality;
		Rt_shadow_bias_min = Lab_saved_settings.rt_shadow_bias_min;
		Rt_shadow_bias_max = Lab_saved_settings.rt_shadow_bias_max;
		Rt_shadow_samples_override = Lab_saved_settings.rt_shadow_samples_override;
		Sun_angular_size_override = Lab_saved_settings.sun_angular_size_override;
		Rtao_samples = Lab_saved_settings.rtao_samples;
		Shadow_contact_hardening_enabled = Lab_saved_settings.shadow_contact_hardening_enabled;
		Lab_settings_saved = false;
	}

	if (reset) {
		LMGR.reset();
	}
	gameseq_post_event(GS_EVENT_PREVIOUS_STATE);
}

void lab_do_frame(float frametime) {
	getLabManager()->onFrame(frametime);
}

