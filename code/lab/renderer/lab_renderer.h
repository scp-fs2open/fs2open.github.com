#pragma once

#include "globalincs/pstypes.h"
#include "globalincs/flagset.h"
#include "graphics/2d.h"
#include "lighting/lighting_profiles.h"
#include "camera/camera.h"
#include "cmdline/cmdline.h"
#include "lab/renderer/lab_cameras.h"
#include "globalincs/systemvars.h"
#include "starfield/starfield.h"
#include "graphics/light.h"
#include "globalincs/alphacolors.h"

FLAG_LIST(LabRenderFlag) {
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


struct gfx_options {
	int bloom_level;
	float ambient_factor;
	float light_factor;
	float emissive_factor;
	float exposure_level;
	piecewise_power_curve_values ppcv;
	AntiAliasMode aa_mode;
	TonemapperAlgorithm tonemapper;
};

constexpr auto LAB_MISSION_NONE_STRING = "None";
constexpr auto LAB_TEAM_COLOR_NONE = "<none>";

class LabRenderer {
public:
	LabRenderer() {
		bloomLevel = gr_bloom_intensity();
		textureQuality = TextureQuality::Maximum;
		cameraDistance = 100.0f;
		currentTeamColor = LAB_TEAM_COLOR_NONE;
		useBackground(LAB_MISSION_NONE_STRING);

		labCamera.reset(new OrbitCamera());

		Viewer_mode |= VM_FREECAMERA;

		Motion_debris_override = true;
		Num_stars = 0;

		gr_set_clear_color(0, 0, 0);
	}

	void onFrame(float frametime);

	static void close() {
		Viewer_mode &= ~VM_FREECAMERA;
	}

	void useBackground(const SCP_string& mission_name);

	SCP_string currentMissionBackground;


	static void setAAMode(AntiAliasMode mode) {
		Gr_aa_mode = mode;

		Motion_debris_override = false;
	}

	static void setTonemapper(TonemapperAlgorithm mode) {
		lighting_profile::lab_set_tonemapper(mode);
	}

	void useNextTeamColorPreset() {
		if (!Team_Colors.empty()) {
			auto color_itr = Team_Colors.find(currentTeamColor);

			if (color_itr == Team_Colors.begin()) {
				color_itr = --Team_Colors.end();
				currentTeamColor = color_itr->first;
			} else {
				--color_itr;
				currentTeamColor = color_itr->first;
			}
		}
	}

	void usePreviousTeamColorPreset() {
		if (!Team_Colors.empty()) {
			auto color_itr = Team_Colors.find(currentTeamColor);

			++color_itr;
			if (color_itr == Team_Colors.end())
				color_itr = Team_Colors.begin();
			currentTeamColor = color_itr->first;
		}
	}

	void setTeamColor(SCP_string teamColor) {
		currentTeamColor = std::move(teamColor);
	}

	void resetView() {}

	void setRenderFlag(LabRenderFlag flag, bool value) { renderFlags.set(flag, value); }

	static float setAmbientFactor(float factor) { 
		lighting_profile::lab_set_ambient(factor);
		return factor; 
	}

	static float setLightFactor(float factor) {
		lighting_profile::lab_set_light(factor);
		return factor; 
	}

	static float setEmissiveFactor(float factor) { 
		lighting_profile::lab_set_emissive(factor);
		return factor; 
	}

	int setBloomLevel(int level) { 
		bloomLevel = level; 
		gr_set_bloom_intensity(level);
		return level; 
	}

	float setExposureLevel(float level) {
		exposureLevel = level;
		lighting_profile::lab_set_exposure(level);
		return level;
	}
	
	static void setPPCValues(piecewise_power_curve_values ppcv) {
		lighting_profile::lab_set_ppc(ppcv);
	}

	void setTextureQuality(TextureQuality quality) { textureQuality = quality; }
	TextureQuality getTextureQuality()
	{
		return textureQuality;
	}

	void setTextureOverride(TextureOverride, bool) {};

	void resetTextureOverride() {};

	void resetGraphicsSettings(gfx_options settings);

	std::unique_ptr<LabCamera> &getCurrentCamera();
	void setCurrentCamera(std::unique_ptr<LabCamera> &newcam);

private:
	flagset<LabRenderFlag> renderFlags;
	int bloomLevel;
	float exposureLevel;
	TextureQuality textureQuality;
	SCP_string currentTeamColor;

	std::unique_ptr<LabCamera> labCamera;

	float cameraDistance;

	void renderHud(float);
	void renderModel(float frametime);

};