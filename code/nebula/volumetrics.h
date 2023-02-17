#pragma once

#include "globalincs/pstypes.h"

#include <memory>

class volumetric_nebula {
	//Instance Settings
	//Type
	SCP_string hullPof = "neb.pof";
	vec3d pos = {{0, 0, 0}}, size = {{60, 60, 60}};

	//Quality
	//How many steps are used to nebulify the volume until the visibility is reached. In theory purely quality and can be changed without changing the aesthetics. Mostly FPS Cost
	int steps = 15;
	//Number of steps per nebula slice to test towards the sun. Mostly FPS Cost
	int globalLightSteps = 6;
	//Resolution of 3D texture as 2^n. 5 - 8 recommended. Mostly VRAM cost
	int resolution = 6;
	//Oversampling of 3D-Texture. Will octuple loading computation time for each value, but improves banding especially at lower resolutions. 1 - 3. Mostly Loading time cost.
	int oversampling = 2;

	//General Visibility
	//The distance in meters until the target translucity is reached
	float visibility = 7.5f;
	//The target translucity. The nebula won't get more opaque than what is specified here. 0 is not possible.
	float alphaLim = 0.001f;

	//Emissive Light Fogging
	//How quickly the emissive will widen in the nebula. The larger the value, the wider will emissives be drawn even with only little nebula to obscure it.
	float emissiveSpread = 1.0f;
	//How intense emissives will be added. The higher, the brighter the emissives.
	float emissiveIntensity = 0.2f;
	//Correcting factor for emissive alpha. Values > 1 will darken the added emissive closer to the actual source, values < 1 will lighten the added emissive closer to the actual source.
	float emissiveFalloff = 1.33f;

	//Sun-based illumination
	//HG coefficient for backlit nebulae. (-1, 1), but should probably be in the range of (0.05, 0.75)
	float henyeyGreensteinCoeff = 0.2f;
	//Distance factor for global light vs nebula translucity. Values > 1 means the nebula is brighter than it ought to be when it's deeper, values < 0 means it's darker when its shallower
	float globalLightDistanceFactor = 1.0f;

	//Instance Data

	int volumeBitmapHandle = -1;
	std::unique_ptr<ubyte[]> volumeBitmapData = nullptr;

public:
	volumetric_nebula() = default;
	~volumetric_nebula();

	int getSteps() const;
	int getGlobalLightSteps() const;

	float getVisibility() const;
	float getAlphaLim() const;

	float getEmissiveSpread() const;
	float getEmissiveIntensity() const;
	float getEmissiveFalloff() const;

	float getHenyeyGreensteinCoeff() const;
	float getGlobalLightDistanceFactor() const;

	bool isVolumeBitmapValid() const;
	void renderVolumeBitmap(float r, float g, float b);
	int getVolumeBitmapHandle() const;
};
