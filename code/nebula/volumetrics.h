#pragma once

#include "globalincs/pstypes.h"
#include "math/vecmat.h"

#include <memory>

class volumetric_nebula {
	//Instance Settings
	
	//Type
	//The name of the POF file specifying the hull of the nebula. Can be convex. Can be multiple disjunct meshes. Each mesh must be watertight. Mesh polies may not intersect, however one mesh may be completely contained in another.
	SCP_string hullPof = "neb.pof";
	//The position and size of the bounding box of the volumetrics.
	vec3d pos = ZERO_VECTOR, size = ZERO_VECTOR;

	//Quality
	//Whether or not edge smoothing is enabled. Disabling this causes jagged edges when looking at the nebula axis aligned, but it's expensive
	bool doEdgeSmoothing = false;
	//How many steps are used to nebulify the volume until the visibility is reached. In theory purely quality and can be changed without changing the aesthetics. Mostly FPS Cost
	int steps = 7;
	//Number of steps per nebula slice to test towards the sun. Mostly FPS Cost
	int globalLightSteps = 4;
	//Resolution of 3D texture as 2^n. 5 - 8 recommended. Mostly VRAM cost
	int resolution = 6;
	//Oversampling of 3D-Texture. Will quadruple loading computation time for each increment, but improves banding especially at lower resolutions. 1 - 3. Mostly Loading time cost.
	int oversampling = 2;
	//Resolution of Noise 3D-Texture as 2^n. 5 - 8 recommended. Mostly VRAM cost
	int noiseResolution = 6;

	//General Visibility
	//The distance in meters until the target translucity is reached
	float visibility = 5.0f;
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

	//Edge noise settings
	//The near distance at which the edge noise starts to fade off
	float noiseNear = 5.0f;
	//The far distance at which the edge noise is gone
	float noiseFar = 15.0f;
	//The detail of the noise's three levels, >= 3
	std::tuple<float, float, float> noiseDetail = { 5.0f, 15.0f, 25.0f };
	//The scale of the edge noise, specified in the size of the noise cube
	float noiseScale = 45.0f;

	//Instance Data
	int volumeBitmapHandle = -1;
	std::unique_ptr<ubyte[]> volumeBitmapData = nullptr;

	int noiseVolumeBitmapHandle = -1;
	std::unique_ptr<ubyte[]> noiseVolumeBitmapData = nullptr;

public:
	volumetric_nebula() = default;
	~volumetric_nebula();

	const vec3d& getPos() const;
	const vec3d& getSize() const;

	bool getEdgeSmoothing() const;
	int getSteps() const;
	int getGlobalLightSteps() const;

	float getVisibility() const;
	float getAlphaLim() const;

	float getEmissiveSpread() const;
	float getEmissiveIntensity() const;
	float getEmissiveFalloff() const;

	float getHenyeyGreensteinCoeff() const;
	float getGlobalLightDistanceFactor() const;

	float getNoiseNear() const;
	float getNoiseFar() const;
	float getNoiseScale() const;

	bool isVolumeBitmapValid() const;
	void renderVolumeBitmap(float r, float g, float b);
	int getVolumeBitmapHandle() const;
	int getNoiseVolumeBitmapHandle() const;
};

