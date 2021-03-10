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
#include "globalincs/alphacolors.h"

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

	static void close() {
		Viewer_mode &= ~VM_FREECAMERA;
	}

	void useBackground(const SCP_string& mission_name);

	static void setAAMode(AntiAliasMode mode) {
		Gr_aa_mode = mode;

		Motion_debris_override = false;
	}

	void useNextTeamColorPreset() {
		auto color_itr = Team_Colors.find(currentTeamColor);

		if (color_itr == Team_Colors.begin()) {
			color_itr = --Team_Colors.end();
			currentTeamColor = color_itr->first;
		}
		else {
			--color_itr;
			currentTeamColor = color_itr->first;
		}
	}

	void usePreviousTeamColorPreset() {
		auto color_itr = Team_Colors.find(currentTeamColor);

		++color_itr;
		if (color_itr == Team_Colors.end())
			color_itr = Team_Colors.begin();
		currentTeamColor = color_itr->first;
	}

	void setTeamColor(SCP_string teamColor) {
		currentTeamColor = std::move(teamColor);
	}

	void resetView() {}

	void setRenderFlag(LabRenderFlag flag, bool value) { renderFlags.set(flag, value); }

	int setAmbientFactor(int factor) { 
		ambientFactor = factor; 
		Cmdline_ambient_factor = factor;
		gr_calculate_ambient_factor();

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

	void setTextureOverride(TextureOverride, bool) {};

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

	void renderHud(float);
	void renderModel(float frametime);

};