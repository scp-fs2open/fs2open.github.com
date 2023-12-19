#pragma once

#include "globalincs/pstypes.h"
#include "globalincs/flagset.h"
#include "graphics/2d.h"
#include "lighting/lighting_profiles.h"
#include "camera/camera.h"
#include "cmdline/cmdline.h"
#include "options/renderer/ingame_options_camera.h"
#include "globalincs/systemvars.h"
#include "starfield/starfield.h"
#include "graphics/light.h"
#include "globalincs/alphacolors.h"

FLAG_LIST(OptRenderFlag){
	ModelRotationEnabled,
	ShowInsignia,
	ShowDamageLightning,
	MoveSubsystems,
	HidePostProcessing,
	NoDiffuseMap,
	NoGlowMap,
	NoSpecularMap,
	NoReflectMap,
	NoEnvMap,
	NoNormalMap,
	NoHeightMap,
	NoAOMap,
	NoMiscMap,
	NoGlowpoints,
	NoLighting,
	ShowWireframe,
	ShowFullDetail,
	ShowThrusters,
	ShowWeapons,
	ShowEmissiveLighting,
	ShowAfterburners,
	TimeStopped,

	NUM_VALUES
};

class OptRenderer {
  public:
	OptRenderer() {

		gr_set_clear_color(0, 0, 0);
	}

	void onFrame(float frametime);

	static void close() {
		Viewer_mode &= ~VM_FREECAMERA;
	}

private:
	flagset<OptRenderFlag> renderFlags;

};