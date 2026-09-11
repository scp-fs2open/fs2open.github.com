#include "volumetrics.h"

#include "bmpman/bmpman.h"
#include "mission/missionparse.h"
#include "model/model.h"
#include "parse/parselo.h"
#include "render/3d.h"

#include <anl.h>

#define OFFSET_R 2
#define OFFSET_G 1
#define OFFSET_B 0
#define OFFSET_A 3
#define COLOR_3D_ARRAY_POS(n, color, x, y, z) (z * n * n * 4 + y * n * 4 + x * 4 + OFFSET_##color)

volumetric_nebula::volumetric_nebula() { }

volumetric_nebula& volumetric_nebula::parse_volumetric_nebula() {
	//This expects that parse_volumetric_nebula was called in an if(optional_string("Volumetrics")) or something
	stuff_string(hullPof, F_PATHNAME);

	//General Settings
	required_string("+Position:");
	vec3d parsedPos;
	stuff_vec3d(&parsedPos);
	
	required_string("+Color:");
	int rgb[3];
	size_t number = stuff_int_list(rgb, 3);
	if (number != 3) {
		error_display(1, "Volumetric nebula color must be fully specified.");
		return *this;
	}
	const int parsedMainColorR = rgb[0];
	const int parsedMainColorG = rgb[1];
	const int parsedMainColorB = rgb[2];

	required_string("+Visibility Opacity:");
	float visibilityOpacity;
	stuff_float(&visibilityOpacity);

	required_string("+Visibility Distance:");
	float visibilityDistance;
	stuff_float(&visibilityDistance);

	std::optional<int> parsedSteps = std::nullopt;
	if (optional_string("+Steps:")) {
		int parsedValue;
		stuff_int(&parsedValue);
		parsedSteps = parsedValue;
	}

	std::optional<int> parsedResolution = std::nullopt;
	if (optional_string("+Resolution:")) {
		int parsedValue;
		stuff_int(&parsedValue);
		if (parsedValue > 8) {
			error_display(0, "Volumetric nebula resolution was set to %d. Maximum is 8.", parsedValue);
			parsedValue = 8;
		}
		parsedResolution = parsedValue;
	}

	std::optional<int> parsedOversampling = std::nullopt;
	if (optional_string("+Oversampling:")) {
		int parsedValue;
		stuff_int(&parsedValue);
		parsedOversampling = parsedValue;
	}

	std::optional<float> parsedSmoothing = std::nullopt;
	if (optional_string("+Smoothing:")) {
		float parsedValue;
		stuff_float(&parsedValue);
		parsedSmoothing = parsedValue;
	}

	//Lighting settings
	std::optional<float> parsedHenyey = std::nullopt;
	if (optional_string("+Heyney Greenstein Coefficient:")) {
		float parsedValue;
		stuff_float(&parsedValue);
		parsedHenyey = parsedValue;
	}

	std::optional<float> parsedSunFalloff = std::nullopt;
	if (optional_string("+Sun Falloff Factor:")) {
		float parsedValue;
		stuff_float(&parsedValue);
		parsedSunFalloff = parsedValue;
	}

	std::optional<int> parsedSunSteps = std::nullopt;
	if (optional_string("+Sun Steps:")) {
		int parsedValue;
		stuff_int(&parsedValue);
		parsedSunSteps = parsedValue;
	}

	//Emissive settings
	std::optional<float> parsedEmissiveSpread = std::nullopt;
	if (optional_string("+Emissive Light Spread:")) {
		float parsedValue;
		stuff_float(&parsedValue);
		parsedEmissiveSpread = parsedValue;
	}

	std::optional<float> parsedEmissiveIntensity = std::nullopt;
	if (optional_string("+Emissive Light Intensity:")) {
		float parsedValue;
		stuff_float(&parsedValue);
		parsedEmissiveIntensity = parsedValue;
	}

	std::optional<float> parsedEmissiveFalloff = std::nullopt;
	if (optional_string("+Emissive Light Falloff:")) {
		float parsedValue;
		stuff_float(&parsedValue);
		parsedEmissiveFalloff = parsedValue;
	}

	//Noise settings
	std::optional<float> parsedNoiseScaleBase = std::nullopt;
	std::optional<float> parsedNoiseScaleSub = std::nullopt;
	std::optional<int> parsedNoiseColorR = std::nullopt;
	std::optional<int> parsedNoiseColorG = std::nullopt;
	std::optional<int> parsedNoiseColorB = std::nullopt;
	std::optional<float> parsedNoiseIntensity = std::nullopt;
	std::optional<int> parsedNoiseResolution = std::nullopt;
	if (optional_string("+Noise:")) {
		noiseActive = true;

		required_string("+Scale:");
		float scale[2];
		number = stuff_float_list(scale, 2);
		if (number == 0) {
			error_display(1, "Volumetric nebula noise scale must have at least the base scale.");
			return *this;
		}
		else if (number == 1) {
			//Set smaller scale to about half, but with low-ish periodicity
			scale[1] = scale[0] * (14.0f / 25.0f);
		}
		parsedNoiseScaleBase = scale[0];
		parsedNoiseScaleSub = scale[1];

		required_string("+Color:");
		number = stuff_int_list(rgb, 3);
		if (number != 3) {
			error_display(1, "Volumetric nebula noise color must be fully specified.");
			return *this;
		}
		parsedNoiseColorR = rgb[0];
		parsedNoiseColorG = rgb[1];
		parsedNoiseColorB = rgb[2];

		if (optional_string("+Intensity:")) {
			float parsedValue;
			stuff_float(&parsedValue);
			parsedNoiseIntensity = parsedValue;
		}

		if (optional_string("+Function Base:")) {
			SCP_string func;
			stuff_string(func, F_RAW);
			noiseColorFunc1 = std::move(func);
		}

		if (optional_string("+Function Sub:")) {
			SCP_string func;
			stuff_string(func, F_RAW);
			noiseColorFunc2 = std::move(func);
		}

		if (optional_string("+Resolution:")) {
			int parsedValue;
			stuff_int(&parsedValue);
			if (parsedValue > 8) {
				error_display(0, "Volumetric nebula noise resolution was set to %d. Maximum is 8.", parsedValue);
				parsedValue = 8;
			}
			parsedNoiseResolution = parsedValue;
		}

	}

	auto color_to_unit = [](int value) { return static_cast<float>(std::clamp(value, 0, 255)) / 255.0f; };
	auto optional_color_to_unit = [&](const std::optional<int>& value) -> std::optional<float> {
		if (!value) {
			return std::nullopt;
		}
		return color_to_unit(*value);
	};

	set_runtime_params(
		parsedPos,
		parsedSteps,
		parsedSunSteps,
		visibilityDistance,
		visibilityOpacity,
		parsedEmissiveSpread,
		parsedEmissiveIntensity,
		parsedEmissiveFalloff,
		parsedHenyey,
		parsedSunFalloff,
		std::nullopt,
		parsedNoiseScaleBase,
		parsedNoiseScaleSub,
		parsedNoiseIntensity,
		color_to_unit(parsedMainColorR),
		color_to_unit(parsedMainColorG),
		color_to_unit(parsedMainColorB),
		optional_color_to_unit(parsedNoiseColorR),
		optional_color_to_unit(parsedNoiseColorG),
		optional_color_to_unit(parsedNoiseColorB),
		parsedResolution,
		parsedOversampling,
		parsedSmoothing,
		parsedNoiseResolution);

	return *this;
}

volumetric_nebula::~volumetric_nebula() {
	if (volumeBitmapHandle >= 0) {
		bm_release(volumeBitmapHandle);
	}
	if (noiseVolumeBitmapHandle >= 0) {
		bm_release(noiseVolumeBitmapHandle);
	}
}

const vec3d& volumetric_nebula::getPos() const {
	return pos;
}

const vec3d& volumetric_nebula::getSize() const {
	return size;
}

const SCP_string& volumetric_nebula::getHullPof() const {
	return hullPof;
}

const std::tuple<float, float, float>& volumetric_nebula::getNebulaColor() const {
	return nebulaColor;
}

int volumetric_nebula::getVolumeBitmapSmoothingSteps() const {
	return std::max(1, static_cast<int>(static_cast<float>(1 << (resolution + oversampling - 1)) * std::min(smoothing, 0.5f)));
}

bool volumetric_nebula::getEdgeSmoothing() const {
	return Detail.nebula_detail == MAX_DETAIL_VALUE || doEdgeSmoothing; //Only for highest setting, or when the lab has an override.
}

bool volumetric_nebula::getConfiguredEdgeSmoothing() const {
	return doEdgeSmoothing;
}

int volumetric_nebula::getSteps() const {
	if (Detail.nebula_detail == 0)
		return 8;

	//Minimal setting (if not hard-set to 8) is steps / 2, max settings is steps.  Ensure it doesn't drop below 8, 10, 12, 14, 16.
	return std::max(steps * (MAX_DETAIL_VALUE + 1) / (2 * (MAX_DETAIL_VALUE + 1) - Detail.nebula_detail), 8 + 2 * Detail.nebula_detail);
}

int volumetric_nebula::getConfiguredSteps() const {
	return steps;
}

int volumetric_nebula::getGlobalLightSteps() const {
	if (Detail.nebula_detail == 0)
		return 4;

	//Minimal setting (if not hard-set to 4) is globalLightSteps / 2, max settings is globalLightSteps. Ensure it doesn't drop below 4, 5, 6, 7, 8.
	return std::max(globalLightSteps * (MAX_DETAIL_VALUE + 1) / (2 * (MAX_DETAIL_VALUE + 1) - Detail.nebula_detail), 4 + Detail.nebula_detail);
}

int volumetric_nebula::getConfiguredGlobalLightSteps() const {
	return globalLightSteps;
}

int volumetric_nebula::getResolution() const
{
	return resolution;
}

int volumetric_nebula::getOversampling() const
{
	return oversampling;
}

int volumetric_nebula::getNoiseResolution() const
{
	return noiseResolution;
}

float volumetric_nebula::getSmoothing() const
{
	return smoothing;
}

float volumetric_nebula::getOpacityDistance() const {
	return opacityDistance;
}

float volumetric_nebula::getStepsize() const {
	return getOpacityDistance() / static_cast<float>(getSteps());
}

float volumetric_nebula::getAlphaLim() const {
	return alphaLim;
}

float volumetric_nebula::getEmissiveSpread() const {
	return emissiveSpread;
}

float volumetric_nebula::getEmissiveIntensity() const {
	return emissiveIntensity;
}

float volumetric_nebula::getEmissiveFalloff() const {
	return emissiveFalloff;
}

float volumetric_nebula::getHenyeyGreensteinCoeff() const {
	return henyeyGreensteinCoeff;
}

float volumetric_nebula::getGlobalLightDistanceFactor() const {
	return globalLightDistanceFactor;
}

float volumetric_nebula::getGlobalLightStepsize() const {
	return getOpacityDistance() * static_cast<float>(getVolumeBitmapSmoothingSteps()) / static_cast<float>(getGlobalLightSteps()) * getGlobalLightDistanceFactor();
}

bool volumetric_nebula::getNoiseActive() const {
	return noiseActive;
}

const std::tuple<float, float>& volumetric_nebula::getNoiseColorScale() const {
	return noiseScale;
}

const std::tuple<float, float, float>& volumetric_nebula::getNoiseColor() const {
	return noiseColor;
}

float volumetric_nebula::getNoiseColorIntensity() const {
	return noiseColorIntensity;
}

bool volumetric_nebula::isVolumeBitmapValid() const {
	return volumeBitmapHandle >= 0 && (!getNoiseActive() || noiseVolumeBitmapHandle >= 0);
}

static anl::CInstructionIndex getDefaultNoise(anl::CKernel& kernel, int seedOffset) {
	return kernel.translateDomain(
		kernel.bias(
			kernel.scaleDomain(kernel.valueBasis(kernel.constant(3), kernel.seed(seedOffset + 0)), kernel.constant(3)),
			kernel.scaleDomain(kernel.valueBasis(kernel.constant(3), kernel.seed(seedOffset + 1)), kernel.constant(8))),
		kernel.multiply(
			kernel.scaleDomain(kernel.simplexBasis(kernel.seed(seedOffset + 2)), kernel.constant(4)),
			kernel.constant(0.6)));
}

static anl::CInstructionIndex getCustomNoise(anl::CKernel& kernel, const SCP_string& expression) {
	anl::CExpressionBuilder builder(kernel);
	return builder.eval(expression);
}

static inline std::array<ivec3, 6> getNeighbors(const ivec3& pnt){
	return {
		ivec3{pnt.x+1, pnt.y, pnt.z},
		ivec3{pnt.x-1, pnt.y, pnt.z},
		ivec3{pnt.x, pnt.y+1, pnt.z},
		ivec3{pnt.x, pnt.y-1, pnt.z},
		ivec3{pnt.x, pnt.y, pnt.z+1},
		ivec3{pnt.x, pnt.y, pnt.z-1}
	};
}

//Nebula distance must be a lower bound to avoid errors, so subtract sqrt(2) in each dimension
static inline float getNebDistSquared(const ivec3& l, const ivec3& r, const vec3d& scale, bool lowerBound) {
	int dx = (l.x - r.x) * (l.x - r.x);
	int dy = (l.y - r.y) * (l.y - r.y);
	int dz = (l.z - r.z) * (l.z - r.z);

	if (lowerBound){
		dx -= 2;
		dy -= 2;
		dz -= 2;
	}

	return (dx < 0 ? 0 : dx) * scale.xyz.x * scale.xyz.x + (dy < 0 ? 0 : dy) * scale.xyz.y * scale.xyz.y + (dz < 0 ? 0 : dz) * scale.xyz.z * scale.xyz.z;
}

void volumetric_nebula::renderVolumeBitmap() {
	Assertion(!hullPof.empty(), "Volumetric Nebula was not properly configured. Did you call parse_volumetric_nebula()?");
	Assertion(!isVolumeBitmapValid(), "Volume bitmap was already rendered!");

	int n = 1 << resolution;
	int nSample = (n << (oversampling - 1)) + 1;
	auto volumeSampleCache = make_unique<bool[]>(nSample * nSample * nSample);

	int modelnum = model_load(hullPof.c_str(), nullptr, ErrorType::NONE);
	if (modelnum < 0) {
		Warning(LOCATION, "Could not load model '%s'.  Unable to render volume bitmap!", hullPof.c_str());
		return;
	}

	const polymodel* pm = model_get(modelnum);
	//Scale up by 2% to ensure that the 3d volume texture does not end on an axis aligned edge with full opacity.
	constexpr float scaleFactor = 1.02f;
	size = pm->maxs - pm->mins;
	size *= scaleFactor;

	bb_min = pos - (size * 0.5f);
	bb_max = pos + (size * 0.5f);

	mc_info mc;

	mc.model_num = modelnum;
	mc.orient = &vmd_identity_matrix;
	mc.pos = &vmd_zero_vector;
	mc.p1 = &vmd_zero_vector;

	mc.flags = MC_CHECK_MODEL | MC_COLLIDE_ALL | MC_CHECK_INVISIBLE_FACES;

	//Calculate minimum "bottom left" corner of scaled size box
	vec3d bl = pm->mins - (size * ((scaleFactor - 1.0f) / 2.0f / scaleFactor));

	//Go through sampling procedure to test where the nebula even is
	for (int x = 0; x < nSample; x++) {
		for (int y = 0; y < nSample; y++) {
			vec3d start = bl;
			start += vec3d{ {{static_cast<float>(x) * size.xyz.x / static_cast<float>(n << (oversampling - 1)),
							 static_cast<float>(y) * size.xyz.y / static_cast<float>(n << (oversampling - 1)),
							 0.0f }} };
			vec3d end = start;
			end.xyz.z += size.xyz.z;

			mc.p0 = &start;
			mc.p1 = &end;
			mc.hit_points_all.clear();
			mc.hit_submodels_all.clear();
			model_collide(&mc);

			//Annoying hack cause sometimes, if edges of polygons get too close to the ray, the collisions are missed / too many. At least find odd rays and fix those, since these are very visible
			while (mc.hit_points_all.size() % 2 != 0) {
				start += vec3d{ {{ size.xyz.x / static_cast<float>(n << (oversampling - 1)) * (Random::INV_F_MAX_VALUE * Random::next() - 0.5f),
								  size.xyz.y / static_cast<float>(n << (oversampling - 1)) * (Random::INV_F_MAX_VALUE * Random::next() - 0.5f), 0.0f }} };
				end += vec3d{ {{ size.xyz.x / static_cast<float>(n << (oversampling - 1)) * (Random::INV_F_MAX_VALUE * Random::next() - 0.5f),
								size.xyz.y / static_cast<float>(n << (oversampling - 1)) * (Random::INV_F_MAX_VALUE * Random::next() - 0.5f), 0.0f }} };
				mc.hit_points_all.clear();
				mc.hit_submodels_all.clear();
				model_collide(&mc);
			}

			SCP_multiset<int> collisionZIndices;
			for(const vec3d& hitpnt : mc.hit_points_all)
				collisionZIndices.emplace(static_cast<int>((hitpnt.xyz.z - bl.xyz.z) / size.xyz.z * static_cast<float>(n << (oversampling - 1))));

			size_t hitcnt = 0;
			auto hitpntit = collisionZIndices.cbegin();
			for (int z = 0; z < nSample; z++) {
				while (hitpntit != collisionZIndices.cend() && *hitpntit < z) {
					++hitpntit;
					++hitcnt;
				}
				volumeSampleCache[x * nSample * nSample + y * nSample + z] = hitcnt % 2 != 0;
			}
		}
	}

	model_unload(modelnum);

	//Sample the nebula values from the binary cubegrid.
	volumeBitmapData = make_unique<ubyte[]>(n * n * n * 4);
	int oversamplingCount = (1 << (oversampling - 1));

	int smoothing_steps = getVolumeBitmapSmoothingSteps();
	float oversamplingDivisor = 255.1f / (static_cast<float>(oversamplingCount + smoothing_steps) * static_cast<float>(oversamplingCount + smoothing_steps) * static_cast<float>(oversamplingCount + smoothing_steps));
	int smoothStart = smoothing_steps / 2;
	int smoothStop = (smoothing_steps / 2 + (1 & smoothing_steps));

	for (int x = 0; x < n; x++) {
		for (int y = 0; y < n; y++) {
			for (int z = 0; z < n; z++) {
				int sum = 0;
				for (int sx = x * oversamplingCount - smoothStart; sx < (x + 1) * oversamplingCount + smoothStop; sx++) {
					for (int sy = y * oversamplingCount - smoothStart; sy < (y + 1) * oversamplingCount + smoothStop; sy++) {
						for (int sz = z * oversamplingCount - smoothStart; sz < (z + 1) * oversamplingCount + smoothStop; sz++) {
							if (sx >= 0 && sx < nSample && sy >= 0 && sy < nSample && sz >= 0 && sz < nSample &&
								volumeSampleCache[sx * nSample * nSample + sy * nSample + sz])
								sum++;
						}
					}
				}

				volumeBitmapData[COLOR_3D_ARRAY_POS(n, A, x, y, z)] = static_cast<ubyte>(static_cast<float>(sum) * oversamplingDivisor);
			}
		}
	}

	// Test for edges in the nebula to compute the UDF
	auto volumeEdgeCache = make_unique<ivec3[]>(n * n * n);
	SCP_set<ivec3> udfBFS_checking, udfBFS_to_check;

	for (int x = 0; x < n; x++) {
		for (int y = 0; y < n; y++) {
			for (int z = 0; z < n; z++) {
				const ubyte& nebula_density = volumeBitmapData[COLOR_3D_ARRAY_POS(n, A, x, y, z)];

				//If we have neither full nor no nebula presence, it's an edge.
				if (nebula_density > 0 && nebula_density < 255) {
					udfBFS_to_check.emplace(ivec3{x, y, z});
					volumeEdgeCache[x * n * n + y * n + z] = ivec3{x, y, z};
				}
				else {
					bool found_edge = false;
					//it's possible that we get completely sharp edges. So test for that.
					for (const ivec3& neighbor : getNeighbors({x, y, z})){
						if (neighbor.x < 0 || neighbor.x >= n || neighbor.y < 0 || neighbor.y >= n || neighbor.z < 0 || neighbor.z >= n)
							continue;

						if (nebula_density != volumeBitmapData[COLOR_3D_ARRAY_POS(n, A, neighbor.x, neighbor.y, neighbor.z)]){
							found_edge = true;
							break;
						}
					}

					if (found_edge) {
						udfBFS_to_check.emplace(ivec3{x, y, z});
						volumeEdgeCache[x * n * n + y * n + z] = ivec3{x, y, z};
					}
					else
						volumeEdgeCache[x * n * n + y * n + z] = ivec3{-1, -1, -1};
				}
			}
		}
	}

	//BFS from the known nebula edges to find the distance to the closest edge
	while(!udfBFS_to_check.empty()){
		udfBFS_checking = udfBFS_to_check;
		udfBFS_to_check.clear();

		for (const ivec3& toCheck : udfBFS_checking){
			const ivec3& closestEdgeTile = volumeEdgeCache[toCheck.x * n * n + toCheck.y * n + toCheck.z];

			for (const ivec3& neighbor : getNeighbors(toCheck)) {
				if (neighbor.x < 0 || neighbor.x >= n || neighbor.y < 0 || neighbor.y >= n || neighbor.z < 0 || neighbor.z >= n)
					continue;

				ivec3& neighborClosestEdgeTile = volumeEdgeCache[neighbor.x * n * n + neighbor.y * n + neighbor.z];

				if (neighborClosestEdgeTile.x < 0 || getNebDistSquared(neighbor, closestEdgeTile, size, false) < getNebDistSquared(neighbor, neighborClosestEdgeTile, size, false)) {
					neighborClosestEdgeTile = closestEdgeTile;
					udfBFS_to_check.emplace(neighbor);
				}
			}
		}
	}

	//Compute the actual UDF from the BFS
	//scale is the maximal distance possible.
	udfScale = vm_vec_mag(&size);
	for (int x = 0; x < n; x++) {
		for (int y = 0; y < n; y++) {
			for (int z = 0; z < n; z++) {
				float dist = sqrtf(getNebDistSquared(ivec3{x, y, z}, volumeEdgeCache[x * n * n + y * n + z], size, true)) / static_cast<float>(n); //in meters
				volumeBitmapData[COLOR_3D_ARRAY_POS(n, R, x, y, z)] = static_cast<ubyte>(dist / udfScale * 255.0f); //UDF
				volumeBitmapData[COLOR_3D_ARRAY_POS(n, G, x, y, z)] = 0; // Reserved
				volumeBitmapData[COLOR_3D_ARRAY_POS(n, B, x, y, z)] = 0; // Reserved
			}
		}
	}

	volumeBitmapHandle = bm_create_3d(32, n, n, n, volumeBitmapData.get());

	if (!noiseActive)
		return;

	int nNoise = 1 << noiseResolution;
	noiseVolumeBitmapData = make_unique<ubyte[]>(nNoise * nNoise * nNoise * 4);

	anl::CKernel kernel;

	anl::CArray3Dd img(nNoise, nNoise, nNoise), img2(nNoise, nNoise, nNoise);
	anl::SMappingRanges ranges(
		0.0f, 1.0f,
		0.0f, 1.0f,
		0.0f, 1.0f);

	anl::CInstructionIndex wispyNoise = noiseColorFunc1 ? getCustomNoise(kernel, *noiseColorFunc1) : getDefaultNoise(kernel, 0);
	anl::CInstructionIndex wispyNoise2 = noiseColorFunc2 ? getCustomNoise(kernel, *noiseColorFunc2) : getDefaultNoise(kernel, 3);

	anl::map3D(anl::SEAMLESS_XYZ, img, kernel, ranges, wispyNoise);
	anl::map3D(anl::SEAMLESS_XYZ, img2, kernel, ranges, wispyNoise2);

	for (int x = 0; x < nNoise; x++) {
		for (int y = 0; y < nNoise; y++) {
			for (int z = 0; z < nNoise; z++) {
				const auto& noisePixel = img.get(x, y, z);
				const auto& noisePixel2 = img2.get(x, y, z);
				noiseVolumeBitmapData[COLOR_3D_ARRAY_POS(nNoise, R, x, y, z)] = static_cast<ubyte>(noisePixel * 255.0f); // R. Color noise 1, sampled at detail 1
				noiseVolumeBitmapData[COLOR_3D_ARRAY_POS(nNoise, G, x, y, z)] = static_cast<ubyte>(noisePixel2 * 255.0f); // G. Color noise 2, sampled at detail 2
				noiseVolumeBitmapData[COLOR_3D_ARRAY_POS(nNoise, B, x, y, z)] = 0; // B. Reserved for surface noise
				noiseVolumeBitmapData[COLOR_3D_ARRAY_POS(nNoise, A, x, y, z)] = 0; // A. Reserved for surface noise.
			}
		}
	}
	noiseVolumeBitmapHandle = bm_create_3d(32, nNoise, nNoise, nNoise, noiseVolumeBitmapData.get());
}

int volumetric_nebula::getVolumeBitmapHandle() const {
	Assertion(volumeBitmapHandle >= 0, "Tried to access volume bitmap without creating it!");
	return volumeBitmapHandle;
}

int volumetric_nebula::getNoiseVolumeBitmapHandle() const {
	Assertion(noiseVolumeBitmapHandle >= 0, "Tried to access noise volume bitmap without creating it!");
	return noiseVolumeBitmapHandle;
}

float volumetric_nebula::getUDFScale() const {
	return udfScale;
}

float volumetric_nebula::getAlphaToPos(const vec3d& pnt, float distance_mult) const {
	// This pretty much emulates the volumetric shader. This could be slow, so I hope it's not needed too often
	vec3d ray_direction;
	float maxTmin = 0;
	float minTmax = vm_vec_normalized_dir(&ray_direction, &pnt, &Eye_position);

	vec3d t1 = (bb_min - Eye_position) / ray_direction;
	vec3d t2 = (bb_max - Eye_position) / ray_direction;

	for (size_t i = 0; i < 3; i++) {
		std::pair<const float&, const float&> tmin_tmax = t1.a1d[i] < t2.a1d[i] ? 
			std::pair<const float&, const float&>{t1.a1d[i], t2.a1d[i]} : 
			std::pair<const float&, const float&>{t2.a1d[i], t1.a1d[i]};
		
		maxTmin = MAX(maxTmin, tmin_tmax.first);
		minTmax = MIN(minTmax, tmin_tmax.second);
	}

	float alpha = 1.0f;
	const float stepalpha = -(powf(getAlphaLim(), 1.0f / (getOpacityDistance() / getStepsize())) - 1.0f);
	const int n = 1 << resolution;
	for (float stept = maxTmin; stept < minTmax; stept += getStepsize()) {
		vec3d localpos = (Eye_position + (ray_direction * stept) - bb_min) / size * static_cast<float>(n);
		int x = static_cast<int>(localpos.xyz.x);
		int y = static_cast<int>(localpos.xyz.y);
		int z = static_cast<int>(localpos.xyz.z);
		CLAMP(x, 0, n - 1);
		CLAMP(y, 0, n - 1);
		CLAMP(z, 0, n - 1);
		alpha *= 1.0f - (stepalpha * (static_cast<float>(volumeBitmapData[COLOR_3D_ARRAY_POS(n, A, x, y, z)]) / 255.0f) / distance_mult);

		if (alpha <= alphaLim)
			break;
	}
	
	CLAMP(alpha, 0.0f, 1.0f);
	return alpha;
}

void volumetric_nebula::set_enabled(bool set_enabled){
	enabled = set_enabled;
}
bool volumetric_nebula::get_enabled() const {
	return enabled;
};

void volumetric_nebula::set_runtime_params(
	std::optional<vec3d> position,
	std::optional<int> renderSteps,
	std::optional<int> sunSteps,
	std::optional<float> visibilityDistance,
	std::optional<float> visibilityOpacity,
	std::optional<float> emissiveLightSpread,
	std::optional<float> emissiveLightIntensity,
	std::optional<float> emissiveLightFalloff,
	std::optional<float> henyeyGreenstein,
	std::optional<float> sunFalloffFactor,
	std::optional<bool> noiseIsActive,
	std::optional<float> noiseScaleBase,
	std::optional<float> noiseScaleSub,
	std::optional<float> noiseIntensity,
	std::optional<float> mainColorR,
	std::optional<float> mainColorG,
	std::optional<float> mainColorB,
	std::optional<float> noiseColorR,
	std::optional<float> noiseColorG,
	std::optional<float> noiseColorB,
	std::optional<int> renderResolution,
	std::optional<int> resolutionOversampling,
	std::optional<float> edgeSmoothingAmount,
	std::optional<int> noiseRes)
{
	auto clamp_int = [](int value, int minValue, int maxValue) { return std::clamp(value, minValue, maxValue); };
	auto clamp_float = [](float value, float minValue, float maxValue) {
		return std::clamp(value, minValue, maxValue);
	};
	auto clamp_unit = [&](float value) { return clamp_float(value, 0.0f, 1.0f); };

	if (position) {
		pos = *position;
	}
	if (renderSteps) {
		steps = clamp_int(*renderSteps, 1, 100);
	}
	if (sunSteps) {
		globalLightSteps = clamp_int(*sunSteps, 2, 16);
	}
	if (visibilityDistance) {
		opacityDistance = clamp_float(*visibilityDistance, 0.1f, 999999.0f);
	}
	if (visibilityOpacity) {
		alphaLim = clamp_float(*visibilityOpacity, 0.0001f, 1.0f);
	}
	if (renderResolution) {
		resolution = clamp_int(*renderResolution, 6, 8);
	}
	if (resolutionOversampling) {
		oversampling = clamp_int(*resolutionOversampling, 1, 3);
	}
	if (edgeSmoothingAmount) {
		smoothing = clamp_float(*edgeSmoothingAmount, 0.0f, 0.5f);
	}
	doEdgeSmoothing = smoothing > 0.0f;
	if (emissiveLightSpread) {
		emissiveSpread = clamp_float(*emissiveLightSpread, 0.0f, 5.0f);
	}
	if (emissiveLightIntensity) {
		emissiveIntensity = clamp_float(*emissiveLightIntensity, 0.0f, 100.0f);
	}
	if (emissiveLightFalloff) {
		emissiveFalloff = clamp_float(*emissiveLightFalloff, 0.01f, 10.0f);
	}
	if (henyeyGreenstein) {
		henyeyGreensteinCoeff = clamp_float(*henyeyGreenstein, -1.0f, 1.0f);
	}
	if (sunFalloffFactor) {
		globalLightDistanceFactor = clamp_float(*sunFalloffFactor, 0.001f, 100.0f);
	}

	if (mainColorR) {
		std::get<0>(nebulaColor) = clamp_unit(*mainColorR);
	}
	if (mainColorG) {
		std::get<1>(nebulaColor) = clamp_unit(*mainColorG);
	}
	if (mainColorB) {
		std::get<2>(nebulaColor) = clamp_unit(*mainColorB);
	}

	if (noiseIsActive) {
		if (*noiseIsActive) {
			if (noiseVolumeBitmapHandle >= 0) {
				noiseActive = true;
			}
		} else {
			noiseActive = false;
		}
	}

	if (!noiseActive) {
		return;
	}

	if (noiseScaleBase) {
		std::get<0>(noiseScale) = clamp_float(*noiseScaleBase, 0.01f, 1000.0f);
	}
	if (noiseScaleSub) {
		std::get<1>(noiseScale) = clamp_float(*noiseScaleSub, 0.01f, 1000.0f);
	}
	if (noiseIntensity) {
		noiseColorIntensity = clamp_float(*noiseIntensity, 0.1f, 100.0f);
	}
	if (noiseRes) {
		noiseResolution = clamp_int(*noiseRes, 4, 8);
	}
	if (noiseColorR) {
		std::get<0>(noiseColor) = clamp_unit(*noiseColorR);
	}
	if (noiseColorG) {
		std::get<1>(noiseColor) = clamp_unit(*noiseColorG);
	}
	if (noiseColorB) {
		std::get<2>(noiseColor) = clamp_unit(*noiseColorB);
	}
}

void volumetrics_level_close() {
	if (The_mission.volumetrics)
		The_mission.volumetrics.reset();
}