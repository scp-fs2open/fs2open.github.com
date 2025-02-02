#pragma once

#include "globalincs/pstypes.h"
#include "math/vecmat.h"

#include <memory>
#include <optional>

namespace fso {
	namespace fred {
		class CFred_mission_save;
	}
}

class volumetric_nebula {
	//Instance Settings
	
	//Type
	//The name of the POF file specifying the hull of the nebula. Can be convex. Can be multiple disjunct meshes. Each mesh must be watertight. Mesh polies may not intersect, however one mesh may be completely contained in another.
	SCP_string hullPof;
	//The position and size of the bounding box of the volumetrics.
	vec3d pos = ZERO_VECTOR, size = ZERO_VECTOR, bb_min = ZERO_VECTOR, bb_max = ZERO_VECTOR;
	//Main color
	std::tuple<float, float, float> nebulaColor = std::make_tuple(0.0f, 0.0f, 0.0f);

	//Quality
	//Whether or not edge smoothing is enabled. Disabling this causes jagged edges when looking at the nebula axis aligned, but it's expensive
	bool doEdgeSmoothing = false;
	//How many steps are used to nebulify the volume until the visibility is reached. In theory purely quality and can be changed without changing the aesthetics. Mostly FPS Cost
	int steps = 15;
	//Number of steps per nebula slice to test towards the sun. Mostly FPS Cost
	int globalLightSteps = 6;
	//Resolution of 3D texture as 2^n. 5 - 8 recommended. Mostly VRAM cost
	int resolution = 6;
	//Oversampling of 3D-Texture. Will quadruple loading computation time for each increment, but improves banding especially at lower resolutions. 1 - 3. Mostly Loading time cost.
	int oversampling = 2;
	//Resolution of Noise 3D-Texture as 2^n. 5 - 8 recommended. Mostly VRAM cost
	int noiseResolution = 5;

	//General Visibility
	//The distance in meters until the target translucity is reached
	float opacityDistance = 5.0f;
	//The target translucity. The nebula won't get more opaque than what is specified here. 0 is not possible.
	float alphaLim = 0.001f;

	//Emissive Light Fogging
	//How quickly the emissive will widen in the nebula. The larger the value, the wider will emissives be drawn even with only little nebula to obscure it.
	float emissiveSpread = 0.7f;
	//How intense emissives will be added. The higher, the brighter the emissives.
	float emissiveIntensity = 1.1f;
	//Correcting factor for emissive alpha. Values > 1 will darken the added emissive closer to the actual source, values < 1 will lighten the added emissive closer to the actual source.
	float emissiveFalloff = 1.5f;

	//Sun-based illumination
	//HG coefficient for backlit nebulae. (-1, 1), but should probably be in the range of (0.05, 0.75)
	float henyeyGreensteinCoeff = 0.2f;
	//Distance factor for global light vs nebula translucity. Values > 1 means the nebula is brighter than it ought to be when it's deeper, values < 0 means it's darker when its shallower
	float globalLightDistanceFactor = 1.0f;

	//Edge noise settings
	//Is the noise even active
	bool noiseActive = false;
	//The size of the noise's two levels, in meters. The fraction of the two levels should have a large denominator to avoid visible harmonics
	std::tuple<float, float> noiseScale = std::make_tuple(1.0f, 1.0f);
	//Noise functions. ANL's DSL for noise. Leave empty to use default noise. Default is representable by: translate(bias(scale(valueBasis(3,0),3),scale(valueBasis(3,1),8)),scale(simplexBasis(2),4)*0.6)
	std::optional<SCP_string> noiseColorFunc1 = std::nullopt, noiseColorFunc2 = std::nullopt;
	//Noise color
	std::tuple<float, float, float> noiseColor = std::make_tuple(0.0f, 0.0f, 0.0f);
	//Noise Intensity
	float noiseColorIntensity = 1.0f;

	//Instance Data
	int volumeBitmapHandle = -1;
	std::unique_ptr<ubyte[]> volumeBitmapData = nullptr;

	int noiseVolumeBitmapHandle = -1;
	std::unique_ptr<ubyte[]> noiseVolumeBitmapData = nullptr;

	float udfScale = 1.0f;

	bool enabled = true;

	//Friend things that are allowed to directly manipulate "current" volumetrics. Only FRED and the Lab. In all other cases, "sensibly constant" values behave properly RAII and stay constant afterwards.
	friend class LabUi; //Lab
	friend class CFred_mission_save; //FRED
	friend class volumetrics_dlg; //FRED
	friend class fso::fred::CFred_mission_save; //QtFRED
public:
	volumetric_nebula();
	~volumetric_nebula();

	volumetric_nebula& parse_volumetric_nebula();

	const vec3d& getPos() const;
	const vec3d& getSize() const;
	const std::tuple<float, float, float>& getNebulaColor() const;

	bool getEdgeSmoothing() const;
	int getSteps() const;
	int getGlobalLightSteps() const;

	float getOpacityDistance() const;
	float getStepsize() const;
	float getAlphaLim() const;

	float getEmissiveSpread() const;
	float getEmissiveIntensity() const;
	float getEmissiveFalloff() const;

	float getHenyeyGreensteinCoeff() const;
	float getGlobalLightDistanceFactor() const;
	float getGlobalLightStepsize() const;

	bool getNoiseActive() const;
	const std::tuple<float, float>& getNoiseColorScale() const;
	const std::tuple<float, float, float>& getNoiseColor() const;
	float getNoiseColorIntensity() const;

	bool isVolumeBitmapValid() const;
	void renderVolumeBitmap();
	int getVolumeBitmapHandle() const;
	int getNoiseVolumeBitmapHandle() const;
	float getUDFScale() const;

	float getAlphaToPos(const vec3d& pnt, float distance_mult) const;

	void set_enabled(bool set_enabled);
	bool get_enabled() const;
};

void volumetrics_level_close();