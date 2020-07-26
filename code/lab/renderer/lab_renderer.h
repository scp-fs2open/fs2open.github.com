#pragma once

#include "globalincs/pstypes.h"
#include "globalincs/flagset.h"
#include "graphics/2d.h"
#include "camera/camera.h"
#include "cmdline/cmdline.h"
#include "lab/renderer/lab_cameras.h"
#include "globalincs/systemvars.h"
#include "starfield/starfield.h"
#include "graphics/light.h"

FLAG_LIST(LabRenderFlag) {
	ModelRotationEnabled,
	ShowInsignia,
	ShowDamageLightning,
	RotateSubsystems,
	HidePostProcessing,
	NoDiffuseMap,
	NoGlowMap,
	NoSpecularMap,
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

enum class TextureQuality {
	Minimum = 0,
	Low,
	Medium,
	High,
	Maximum
};

enum class TextureChannel {
	DiffuseRed,
	DiffuseGreen,
	DiffuseBlue,
	GlowRed,
	GlowGreen,
	GlowBlue,
	SpecularRed,
	SpecularGreen,
	SpecularBlue,
	SpecularGloss
};

enum class TextureOverride {
	Diffuse,
	Glow,
	Specular
};

class LabRenderer {
public:
	LabRenderer(LabCamera* cam) {
		bloomLevel = Cmdline_bloom_intensity;
		ambientFactor = Cmdline_ambient_factor;
		directionalFactor = static_light_factor;
		textureQuality = TextureQuality::Maximum;
		cameraDistance = 100.0f;
		currentTeamColor = "<none>";
		useBackground("None");

		labCamera = cam;

		Viewer_mode |= VM_FREECAMERA;

		Motion_debris_override = true;
		Num_stars = 0;

		gr_set_clear_color(0, 0, 0);
	}

	void onFrame(float frametime);

	void close() {
		Viewer_mode &= ~VM_FREECAMERA;
	}

	void useBackground(SCP_string mission_name);

	void setAAMode(AntiAliasMode mode) {
		Gr_aa_mode = mode;

		Motion_debris_override = false;
	}

	void useNextTeamColorPreset() {}

	void usePreviousTeamColorPreset() {}

	void resetView() {}

	void setRenderFlag(LabRenderFlag flag, bool value) { renderFlags.set(flag, value); }

	int setAmbientFactor(int factor) { 
		ambientFactor = factor; 
		Cmdline_ambient_factor = factor;
		return factor; 
	}

	float setDirectionalFactor(float factor) { 
		directionalFactor = factor; 
		static_light_factor = factor;
		return factor; 
	}

	int setBloomLevel(int level) { 
		bloomLevel = level; 
		Cmdline_bloom_intensity = level;
		return level; 
	}

	void setTextureQuality(TextureQuality quality) { textureQuality = quality; }

	void setTextureOverride(TextureOverride texture, bool value) {};

	void setTextureChannelValue(TextureChannel channel, float value) {};

	void resetTextureOverride() {};

	LabCamera* getCurrentCamera();
	void setCurrentCamera(LabCamera* newcam);

private:
	flagset<LabRenderFlag> renderFlags;
	int ambientFactor;
	float directionalFactor;
	int bloomLevel;
	TextureQuality textureQuality;
	SCP_string currentTeamColor;
	SCP_string currentMissionBackground;

	LabCamera* labCamera;

	float cameraDistance;

	void renderHud(float frametime);
	void renderModel(float frametime);

};