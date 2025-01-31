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
	stuff_vec3d(&pos);
	
	required_string("+Color:");
	int rgb[3];
	size_t number = stuff_int_list(rgb, 3);
	if (number != 3) {
		error_display(1, "Volumetric nebula color must be fully specified.");
		return *this;
	}
	nebulaColor = std::make_tuple(static_cast<float>(rgb[0]) / 255.0f, static_cast<float>(rgb[1]) / 255.0f , static_cast<float>(rgb[2]) / 255.0f);

	required_string("+Visibility Opacity:");
	stuff_float(&alphaLim);

	required_string("+Visibility Distance:");
	stuff_float(&opacityDistance);

	if(optional_string("+Steps:")) {
		stuff_int(&steps);
	}

	if (optional_string("+Resolution:")) {
		stuff_int(&resolution);
		if (resolution > 8) {
			error_display(0, "Volumetric nebula resolution was set to %d. Maximum is 8.", resolution);
			resolution = 8;
		}
	}

	if (optional_string("+Oversampling:")) {
		stuff_int(&oversampling);
	}

	//Lighting settings
	if (optional_string("+Heyney Greenstein Coefficient:")) {
		stuff_float(&henyeyGreensteinCoeff);
	}

	if (optional_string("+Sun Falloff Factor:")) {
		stuff_float(&globalLightDistanceFactor);
	}

	if (optional_string("+Sun Steps:")) {
		stuff_int(&globalLightSteps);
	}

	//Emissive settings
	if (optional_string("+Emissive Light Spread:")) {
		stuff_float(&emissiveSpread);
	}

	if (optional_string("+Emissive Light Intensity:")) {
		stuff_float(&emissiveIntensity);
	}

	if (optional_string("+Emissive Light Falloff:")) {
		stuff_float(&emissiveFalloff);
	}

	//Noise settings
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
		noiseScale = std::make_tuple(scale[0], scale[1]);

		required_string("+Color:");
		number = stuff_int_list(rgb, 3);
		if (number != 3) {
			error_display(1, "Volumetric nebula noise color must be fully specified.");
			return *this;
		}
		noiseColor = std::make_tuple(static_cast<float>(rgb[0]) / 255.0f, static_cast<float>(rgb[1]) / 255.0f , static_cast<float>(rgb[2]) / 255.0f);

		if (optional_string("+Intensity:")) {
			stuff_float(&noiseColorIntensity);
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
			stuff_int(&noiseResolution);
			if (noiseResolution > 8) {
				error_display(0, "Volumetric nebula noise resolution was set to %d. Maximum is 8.", noiseResolution);
				noiseResolution = 8;
			}
		}
	}

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

const std::tuple<float, float, float>& volumetric_nebula::getNebulaColor() const {
	return nebulaColor;
}

bool volumetric_nebula::getEdgeSmoothing() const {
	return Detail.nebula_detail == MAX_DETAIL_VALUE || doEdgeSmoothing; //Only for highest setting, or when the lab has an override.
}

int volumetric_nebula::getSteps() const {
	if (Detail.nebula_detail == 0)
		return 8;

	//Minimal setting (if not hard-set to 8) is steps / 2, max settings is steps.  Ensure it doesn't drop below 8, 10, 12, 14, 16.
	return std::max(steps * (MAX_DETAIL_VALUE + 1) / (2 * (MAX_DETAIL_VALUE + 1) - Detail.nebula_detail), 8 + 2 * Detail.nebula_detail);
}

int volumetric_nebula::getGlobalLightSteps() const {
	if (Detail.nebula_detail == 0)
		return 4;

	//Minimal setting (if not hard-set to 4) is globalLightSteps / 2, max settings is globalLightSteps. Ensure it doesn't drop below 4, 5, 6, 7, 8.
	return std::max(globalLightSteps * (MAX_DETAIL_VALUE + 1) / (2 * (MAX_DETAIL_VALUE + 1) - Detail.nebula_detail), 4 + Detail.nebula_detail);
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
	return getOpacityDistance() / static_cast<float>(getGlobalLightSteps()) * getGlobalLightDistanceFactor();
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
	int oversamplingCount = (1 << (oversampling - 1)) + 1;
	float oversamplingDivisor = 255.1f / static_cast<float>(oversamplingCount);
	for (int x = 0; x < n; x++) {
		for (int y = 0; y < n; y++) {
			for (int z = 0; z < n; z++) {
				int sum = 0;
				for (int sx = x * oversampling; sx <= (x + 1) * oversampling; sx++) {
					for (int sy = y * oversampling; sy <= (y + 1) * oversampling; sy++) {
						for (int sz = z * oversampling; sz <= (z + 1) * oversampling; sz++) {
							if (volumeSampleCache[sx * nSample * nSample + sy * nSample + sz])
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

void volumetrics_level_close() {
	if (The_mission.volumetrics)
		The_mission.volumetrics.reset();
}