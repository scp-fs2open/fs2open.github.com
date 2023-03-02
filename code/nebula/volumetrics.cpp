#include "volumetrics.h"

#include "bmpman/bmpman.h"
#include "mission/missionparse.h"
#include "model/model.h"
#include "parse/parselo.h"

#include <anl.h>

#define OFFSET_R 2
#define OFFSET_G 1
#define OFFSET_B 0
#define OFFSET_A 3
#define COLOR_3D_ARRAY_POS(n, color, x, y, z) (z * n * n * 4 + y * n * 4 + x * 4 + OFFSET_##color)

volumetric_nebula::volumetric_nebula() {
	//This expects that the constructor was called in an if(optional_string("Volumetrics")) or something
	stuff_string(hullPof, F_PATHNAME);

	//General Settings
	required_string("+Position:");
	stuff_vec3d(&pos);
	
	required_string("+Color:");
	int rgb[3];
	size_t number = stuff_int_list(rgb, 3);
	if (number != 3) {
		error_display(1, "Volumetric nebula color must be fully specified.");
		return;
	}
	nebulaColor = { static_cast<float>(rgb[0]) / 255.0f, static_cast<float>(rgb[1]) / 255.0f , static_cast<float>(rgb[2]) / 255.0f };

	required_string("+Visibility Opacity:");
	stuff_float(&alphaLim);

	required_string("+Visibility Distance:");
	stuff_float(&visibility);

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
			return;
		}
		else if (number == 1) {
			//Set smaller scale to about half, but with low-ish periodicity
			scale[1] = scale[0] * (14.0f / 25.0f);
		}
		noiseScale = { scale[0], scale[1] };

		required_string("+Color:");
		number = stuff_int_list(rgb, 3);
		if (number != 3) {
			error_display(1, "Volumetric nebula noise color must be fully specified.");
			return;
		}
		noiseColor = { static_cast<float>(rgb[0]) / 255.0f, static_cast<float>(rgb[1]) / 255.0f , static_cast<float>(rgb[2]) / 255.0f };

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

bool volumetric_nebula::getEdgeSmoothing() const {
	return doEdgeSmoothing; //potentially adjust for graphics settings in F2 menu. Only for highest setting.
}

int volumetric_nebula::getSteps() const {
	return steps; //potentially adjust for graphics settings in F2 menu. 8 to 16 for normal nebulae.
}

int volumetric_nebula::getGlobalLightSteps() const {
	return globalLightSteps; //potentially adjust for graphics settings in F2 menu. 4 to 8 for normal nebulae.
}

float volumetric_nebula::getVisibility() const {
	return visibility;
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

bool volumetric_nebula::getNoiseActive() const {
	return noiseActive;
}

const std::tuple<float, float>& volumetric_nebula::getNoiseColorScale() const {
	return noiseScale;
}

const std::tuple<float, float, float>& volumetric_nebula::getNoiseColor() const {
	return noiseColor;
}

bool volumetric_nebula::isVolumeBitmapValid() const {
	return volumeBitmapHandle >= 0/* && noiseVolumeBitmapHandle >= 0*/;
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

void volumetric_nebula::renderVolumeBitmap() {
	Assertion(!isVolumeBitmapValid(), "Volume bitmap was already rendered!");

	int n = 1 << resolution;
	int nSample = (n << (oversampling - 1)) + 1;
	auto volumeSampleCache = make_unique<bool[]>(nSample * nSample * nSample);

	int modelnum = model_load(hullPof.c_str(), 0, nullptr);

	const polymodel* pm = model_get(modelnum);
	//Scale up by 5% to ensure that the 3d volume texture does not end on an axis aligned edge with full opacity. Alternatively, set texture to not repeat but rather to clip
	constexpr float scaleFactor = 1.05f;
	size = pm->maxs - pm->mins;
	size *= scaleFactor;

	mc_info mc;

	mc.model_num = modelnum;
	mc.orient = &vmd_identity_matrix;
	mc.pos = &vmd_zero_vector;
	mc.p1 = &vmd_zero_vector;
	//mc.radius = 0.1f;

	mc.flags = MC_CHECK_MODEL /* | MC_CHECK_SPHERELINE*/ | MC_COLLIDE_ALL | MC_CHECK_INVISIBLE_FACES;

	//Calculate minimum "bottom left" corner of scaled size box
	vec3d bl = pm->mins - (size * ((scaleFactor - 1.0f) / 2.0f / scaleFactor));

	for (size_t x = 0; x < nSample; x++) {
		for (size_t y = 0; y < nSample; y++) {
			vec3d start = bl;
			start += vec3d{ {static_cast<float>(x) * size.xyz.x / static_cast<float>(n << (oversampling - 1)),
							 static_cast<float>(y) * size.xyz.y / static_cast<float>(n << (oversampling - 1)),
							 0.0f } };
			vec3d end = start;
			end.xyz.z += size.xyz.z;

			mc.p0 = &start;
			mc.p1 = &end;
			mc.hit_points_all.clear();
			mc.hit_submodels_all.clear();
			model_collide(&mc);

			//Annoying hack cause sometimes, if edges of polygons get too close to the ray, the collisions are missed / too many. At least find odd rays and fix those, since these are very visible
			while (mc.hit_points_all.size() % 2 != 0) {
				start += vec3d{ { size.xyz.x / static_cast<float>(n << (oversampling - 1)) * (Random::INV_F_MAX_VALUE * Random::next() - 0.5f),
								  size.xyz.y / static_cast<float>(n << (oversampling - 1)) * (Random::INV_F_MAX_VALUE * Random::next() - 0.5f), 0.0f } };
				end += vec3d{ { size.xyz.x / static_cast<float>(n << (oversampling - 1)) * (Random::INV_F_MAX_VALUE * Random::next() - 0.5f),
								size.xyz.y / static_cast<float>(n << (oversampling - 1)) * (Random::INV_F_MAX_VALUE * Random::next() - 0.5f), 0.0f } };
				mc.hit_points_all.clear();
				mc.hit_submodels_all.clear();
				model_collide(&mc);
			}

			SCP_multiset<size_t> collisionZIndices;
			for(const vec3d& hitpnt : mc.hit_points_all)
				collisionZIndices.emplace(static_cast<size_t>((hitpnt.xyz.z - bl.xyz.z) / size.xyz.z * static_cast<float>(n << (oversampling - 1))));

			size_t hitcnt = 0;
			auto hitpntit = collisionZIndices.cbegin();
			for (size_t z = 0; z < nSample; z++) {
				while (hitpntit != collisionZIndices.cend() && *hitpntit < z) {
					++hitpntit;
					++hitcnt;
				}
				volumeSampleCache[x * nSample * nSample + y * nSample + z] = hitcnt % 2 != 0;
			}
		}
	}

	model_unload(modelnum);

	volumeBitmapData = make_unique<ubyte[]>(n * n * n * 4);
	float oversamplingDivisor = 255.0f / static_cast<float>((1 << (oversampling - 1)) + 1);
	for (size_t x = 0; x < n; x++) {
		for (size_t y = 0; y < n; y++) {
			for (size_t z = 0; z < n; z++) {
				volumeBitmapData[COLOR_3D_ARRAY_POS(n, R, x, y, z)] = static_cast<ubyte>(std::get<0>(nebulaColor) * 255.0f);
				volumeBitmapData[COLOR_3D_ARRAY_POS(n, G, x, y, z)] = static_cast<ubyte>(std::get<1>(nebulaColor) * 255.0f);
				volumeBitmapData[COLOR_3D_ARRAY_POS(n, B, x, y, z)] = static_cast<ubyte>(std::get<2>(nebulaColor) * 255.0f);
				
				float sum = 0.0f;
				for (size_t sx = x * oversampling; sx <= (x + 1) * oversampling; sx++) {
					for (size_t sy = y * oversampling; sy <= (y + 1) * oversampling; sy++) {
						for (size_t sz = z * oversampling; sz <= (z + 1) * oversampling; sz++) {
							if (volumeSampleCache[sx * nSample * nSample + sy * nSample + sz])
								sum += 1.0f;
						}
					}
				}
				
				volumeBitmapData[COLOR_3D_ARRAY_POS(n, A, x, y, z)] = static_cast<ubyte>(sum * oversamplingDivisor);
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

float volumetric_nebula::getAlphaToPos(const vec3d& pnt, float distance_mult) const {
	return 1.0f;
}

void volumetrics_level_close() {
	if (The_mission.volumetrics)
		The_mission.volumetrics.reset();
}