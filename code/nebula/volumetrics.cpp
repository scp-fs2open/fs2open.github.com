#include "volumetrics.h"

#include "bmpman/bmpman.h"
#include "model/model.h"

#include <anl.h>

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
	return doEdgeSmoothing;
}

int volumetric_nebula::getSteps() const {
	return steps; //potentially adjust for graphics settings in F2 menu
}

int volumetric_nebula::getGlobalLightSteps() const {
	return globalLightSteps; //potentially adjust for graphics settings in F2 menu
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

float volumetric_nebula::getNoiseNear() const {
	return noiseNear;
}

float volumetric_nebula::getNoiseFar() const {
	return noiseFar;
}

float volumetric_nebula::getNoiseScale() const {
	return noiseScale;
}

bool volumetric_nebula::isVolumeBitmapValid() const {
	return volumeBitmapHandle >= 0/* && noiseVolumeBitmapHandle >= 0*/;
}

void volumetric_nebula::renderVolumeBitmap(float r, float g, float b) {
	Assertion(!isVolumeBitmapValid(), "Volume bitmap was already rendered!");

	int n = 1 << resolution;
	int nSample = (n << (oversampling - 1)) + 1;
	auto volumeSampleCache = make_unique<bool[]>(nSample * nSample * nSample);

	int modelnum = model_load(hullPof.c_str(), 0, nullptr);

	const polymodel* pm = model_get(modelnum);
	//Scale up by 2% to ensure that the 3d volume texture does not end on an axis aligned edge with full opacity
	constexpr float scaleFactor = 1.02f;
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
	for (size_t x = 0; x < n; x++) {
		for (size_t y = 0; y < n; y++) {
			for (size_t z = 0; z < n; z++) {
				volumeBitmapData[x * n * n * 4 + y * n * 4 + z * 4] = static_cast<ubyte>(b * 255.0f);
				volumeBitmapData[x * n * n * 4 + y * n * 4 + z * 4 + 1] = static_cast<ubyte>(g * 255.0f);
				volumeBitmapData[x * n * n * 4 + y * n * 4 + z * 4 + 2] = static_cast<ubyte>(r * 255.0f);
				
				float sum = 0.0f;
				for (size_t sx = x * oversampling; sx <= (x + 1) * oversampling; sx++) {
					for (size_t sy = y * oversampling; sy <= (y + 1) * oversampling; sy++) {
						for (size_t sz = z * oversampling; sz <= (z + 1) * oversampling; sz++) {
							if (volumeSampleCache[sx * nSample * nSample + sy * nSample + sz])
								sum += 1.0f;
						}
					}
				}
				
				volumeBitmapData[x * n * n * 4 + y * n * 4 + z * 4 + 3] = static_cast<ubyte>(sum * 255.0f / static_cast<float>((1 << oversampling) * (1 << oversampling) * (1 << oversampling)));
			}
		}
	}

	volumeBitmapHandle = bm_create_3d(32, n, n, n, volumeBitmapData.get());


	int nNoise = 1 << noiseResolution;
	noiseVolumeBitmapData = make_unique<ubyte[]>(nNoise * nNoise * nNoise * 4);

	anl::CKernel kernel;

	anl::CArray3Dd img(nNoise, nNoise, nNoise);
	anl::SMappingRanges ranges(
		0.0f, 1.0f,
		0.0f, 1.0f,
		0.0f, 1.0f);
	
	anl::CInstructionIndex instruction = kernel.valueBasis(kernel.constant(3), kernel.seed(0));
	anl::map3D(anl::SEAMLESS_XYZ, img, kernel, ranges, instruction);

	for (int x = 0; x < nNoise; x++) {
		for (int y = 0; y < nNoise; y++) {
			for (int z = 0; z < nNoise; z++) {
				const auto& noisePixel = img.get(x, y, z);
				noiseVolumeBitmapData[x * n * n * 4 + y * n * 4 + z * 4] = static_cast<ubyte>(noisePixel * 255.0f); // B
				noiseVolumeBitmapData[x * n * n * 4 + y * n * 4 + z * 4 + 1] = static_cast<ubyte>(noisePixel * 255.0f); // G
				noiseVolumeBitmapData[x * n * n * 4 + y * n * 4 + z * 4 + 2] = static_cast<ubyte>(noisePixel * 255.0f); // R
				noiseVolumeBitmapData[x * n * n * 4 + y * n * 4 + z * 4 + 3] = 0xFFU;
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
